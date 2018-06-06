/*******************************************************************************
* File Name: hrsc.c
*
* Version 1.0
*
* Description:
*  This file contains HRS service related code.
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"
#include "hrsc.h"

uint8_t hrsEeReset = 0;
uint16_t hrsNotification = 0;

/* Heart Rate Measurement characteristic data structure */
cy_stc_ble_hrs_hrm_t hrsHeartRate;

/* Heart Rate Service callback */
void HeartRateCallBack(uint32_t event, void* eventParam)
{
    cy_en_ble_api_result_t apiResult;
    uint16_t rrInt;
    uint16_t i;

    switch(event)
    {
        case CY_BLE_EVT_HRSC_READ_CHAR_RESPONSE:
            DBG_PRINTF("Body Sensor Location: ");
            switch(*(((cy_stc_ble_hrs_char_value_t*)eventParam)->value->val))
            {
                case CY_BLE_HRS_BODY_SENSOR_LOCATION_OTHER:
                    DBG_PRINTF("OTHER (0) \r\n");
                    break;

                case CY_BLE_HRS_BODY_SENSOR_LOCATION_CHEST:
                    DBG_PRINTF("CHEST (1) \r\n");
                    break;

                case CY_BLE_HRS_BODY_SENSOR_LOCATION_WRIST:
                    DBG_PRINTF("WRIST (2) \r\n");
                    break;

                default:
                    DBG_PRINTF("default \r\n");
                    break;
            }
            
            SetAppState(GetAppState() | APP_STATE_OPERATION_COMPLETE);
            SetAppState(GetAppState() & ~APP_STATE_WAITING_RESPONSE);
            
            break;


        case CY_BLE_EVT_HRSC_WRITE_DESCR_RESPONSE:
            if((GetAppState() & APP_STATE_ENABLING_CCCD) != 0u)
            {
                DBG_PRINTF("Heart Rate Measurement Notification is Enabled \r\n");
                hrsNotification = CY_BLE_CCCD_NOTIFICATION;
                
            }
            else
            {
                DBG_PRINTF("Heart Rate Measurement Notification is Disabled \r\n");
                hrsNotification = CY_BLE_CCCD_DEFAULT;
            }
       
            SetAppState(GetAppState() | APP_STATE_OPERATION_COMPLETE);
            SetAppState(GetAppState() & ~APP_STATE_WAITING_RESPONSE);
            break;

        case CY_BLE_EVT_HRSC_NOTIFICATION:
            DBG_PRINTF("Heart Rate Notification: ");
            
            HrscUnPackHrm(((cy_stc_ble_hrs_char_value_t*)eventParam)->value);

            if(!HrscIsSensorContactSupported() || HrscIsSensorContactDetected())
            {
                DBG_PRINTF("Heart Rate: %d    ", HrscGetHeartRate());
                DBG_PRINTF("EnergyExpended: %d", HrscGetEnergyExpended());

                for(i = 0; i < CY_BLE_HRS_HRM_RRSIZE; i++)
                {
                    rrInt = HrscGetRRInterval(i);
                    if(0u != rrInt)
                    {
                        DBG_PRINTF("    RR-Interval %d: %d", i, HrscGetRRInterval(i));
                    }
                }

                DBG_PRINTF("\r\n");
            }
            else
            {
                DBG_PRINTF("Sensor Contact is supported but not detected \r\n");
            }

            if((hrsEeReset == true) && (30u > HrscGetEnergyExpended()))
            {
                hrsEeReset = 0u;
            }
            else if((0u == hrsEeReset) && (300u < HrscGetEnergyExpended()))
            {
                hrsEeReset = 1u;

                apiResult = HrscResetEnergyExpendedCounter(((cy_stc_ble_hrs_char_value_t*)eventParam)->connHandle);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("HrscResetEnergyExpendedCounter API Error: ");
                    PrintApiResult(apiResult);
                }
                else
                {
                    DBG_PRINTF("Heart Rate Control Point Write Request is sent \r\n");
                }
            }
             
            break;

        case CY_BLE_EVT_HRSC_READ_DESCR_RESPONSE:
            DBG_PRINTF("HRM CCCD Read Response: %4.4x \r\n", 
                        Cy_BLE_Get16ByPtr(((cy_stc_ble_hrs_descr_value_t*)eventParam)->value->val));
            
            hrsNotification = Cy_BLE_Get16ByPtr(((cy_stc_ble_hrs_descr_value_t*)eventParam)->value->val);
            
            SetAppState(GetAppState() | APP_STATE_OPERATION_COMPLETE);
            SetAppState(GetAppState() & ~APP_STATE_WAITING_RESPONSE);
            break;

        case CY_BLE_EVT_HRSC_WRITE_CHAR_RESPONSE:
            DBG_PRINTF("CPT Write Response: energy expended counter is reset \r\n");
            break;

        default:
            DBG_PRINTF("OTHER HRS event: %lx \r\n", event);
            break;
    }
}

/*******************************************************************************
* Function Name: HrsInit
********************************************************************************
*
* Summary:
*  Initializes the Heart Rate application global variables.
*
*******************************************************************************/
void HrsInit(void)
{
    uint16_t i;

    hrsHeartRate.flags = 0u;
    hrsHeartRate.heartRateValue = 0u;
    hrsHeartRate.energyExpendedValue = 0u;

    for(i = 0; i < CY_BLE_HRS_HRM_RRSIZE; i++)
    {
        hrsHeartRate.rrInterval[i] = 0u;
    }
    
    Cy_BLE_HRS_RegisterAttrCallback(HeartRateCallBack);
}

/*******************************************************************************
* Function Name: HrscConfigHeartRateNtf
********************************************************************************
*
* Summary:
*   Modifies the Heart Rate Measurument characteristic configuration descriptor.
*
* Parameters:
*  cy_stc_ble_conn_handle_t connHandle: Connection handle
*  uint16_t configuration:   Notification flag, can be either:
*                            CY_BLE_HRS_HRM_NTF_ENABLE or
*                            CY_BLE_HRS_HRM_NTF_DISABLE
*
* Return:
*   cy_en_ble_api_result_t: API result will state if API succeeded
*                               (CY_BLE_SUCCESS) or failed with error codes:
*   CY_BLE_SUCCESS - The request is sent successfully;
*   CY_BLE_ERROR_INVALID_PARAMETER - Validation of input parameter is failed.
*******************************************************************************/
cy_en_ble_api_result_t HrscConfigHeartRateNtf(cy_stc_ble_conn_handle_t connHandle, uint16_t configuration)
{
    uint8_t config[CY_BLE_CCCD_LEN];

    configuration &= CY_BLE_CCCD_NOTIFICATION;

    Cy_BLE_Set16ByPtr(config, configuration);

    return (Cy_BLE_HRSC_SetCharacteristicDescriptor(connHandle, CY_BLE_HRS_HRM,
                        CY_BLE_HRS_HRM_CCCD , CY_BLE_CCCD_LEN, config));
}


/*******************************************************************************
* Function Name: HrscUnPackHrm
********************************************************************************
*
* Summary:
*   unpack input Hrs packer and store in the hrsHeartRate structure
*
* Parameters:
*  uint16_t cy_stc_ble_gatt_value_t value:  input Hrs packet
*                   
*******************************************************************************/
void HrscUnPackHrm(cy_stc_ble_gatt_value_t* value)
{
    uint8_t nextPtr;
    uint8_t rrInt;
    uint8_t i;
    uint8_t *pdu;

    pdu = value->val;

    /* flags field is always the first byte */
    hrsHeartRate.flags = pdu[0u];

    if((hrsHeartRate.flags & CY_BLE_HRS_HRM_HRVAL16) != 0u)
    {
        /* Heart Rate Measurement Value is also located beginning from the first byte */
        hrsHeartRate.heartRateValue = Cy_BLE_Get16ByPtr(&pdu[1u]);
        /* the next data will be located beginning from 3rd byte */
        nextPtr = 3u;
    }
    else
    {
        /* Heart Rate Measurement Value is also located in the first byte */
        hrsHeartRate.heartRateValue = (uint16_t) pdu[1u];
        /* the next data will be located beginning from the 2nd byte */
        nextPtr = 2u;
    }

    if((hrsHeartRate.flags & CY_BLE_HRS_HRM_ENEXP) != 0u)
    {
        hrsHeartRate.energyExpendedValue = Cy_BLE_Get16ByPtr(&pdu[nextPtr]);
        /* add 2 bytes: Energy Expended field is uint16_t */
        nextPtr += 2u;
    }

    if((hrsHeartRate.flags & CY_BLE_HRS_HRM_RRINT) != 0u)
    {
        /* Calculate how many RR-Intervals are in this pdu */
        rrInt = (uint8_t)(((uint8_t)(value->len) - nextPtr) >> 1);

        for(i = 0u; i < CY_BLE_HRS_HRM_RRSIZE; i++)
        {
            if(i < rrInt) /* While there are RR-Interval values in this pdu */
            {
                hrsHeartRate.rrInterval[i] = Cy_BLE_Get16ByPtr(&pdu[nextPtr]);          
                /* add 2 bytes: RR-Interval field is uint16_t */
                nextPtr += 2u;
            }
            else /* Fill fhe rest of RR-Interval buffer with zeros */
            {
                hrsHeartRate.rrInterval[i] = 0u;
            }
        }
    }
}


/*******************************************************************************
* Function Name: HrscGetRRInterval
********************************************************************************
*
* Summary:
*   Gets the specified RR-Interval from the
*   Heart Rate Measurument characteristic structure
*
* Parameters:
*   uint8_t rrIntervalNumber - number of desired RR-Intervals
*
* Return:
*   uint16_t rrInterval.
*
*******************************************************************************/
uint16_t HrscGetRRInterval(uint8_t rrIntervalNumber)
{
    uint16_t retVal;

    if((hrsHeartRate.flags & CY_BLE_HRS_HRM_RRINT) != 0u)
    {
        retVal = hrsHeartRate.rrInterval[rrIntervalNumber];
    }
    else
    {
        retVal = 0u; /* If Energy Expended field is not present then return 0 */
    }

    return (retVal);
}

/*******************************************************************************
* Function Name: HrscResetEnergyExpendedCounter
********************************************************************************
*
* Summary:
*   Writes "1" into the Heart Rate Control Point characteristic
*       on the peripheral device, that is to reset the Energy Expended counter
* Parameters:
*  cy_stc_ble_conn_handle_t connHandle: Connection handle
*
* Return:
*   cy_en_ble_api_result_t: API result will state if API succeeded
*                               (CY_BLE_SUCCESS) or
*                               failed with error codes:
*   CY_BLE_SUCCESS - The request is sent successfully;
*   CY_BLE_ERROR_INVALID_PARAMETER - Validation of input parameter is failed.
*******************************************************************************/
cy_en_ble_api_result_t HrscResetEnergyExpendedCounter(cy_stc_ble_conn_handle_t connHandle)
{
    uint8_t value = CY_BLE_HRS_RESET_ENERGY_EXPENDED;

    return (Cy_BLE_HRSC_SetCharacteristicValue(connHandle, CY_BLE_HRS_CPT, CY_BLE_HRS_CPT_CHAR_LEN, &value));
}



/* [] END OF FILE */
