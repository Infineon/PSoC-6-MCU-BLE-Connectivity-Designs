/*******************************************************************************
* File Name: hrss.c
*
* Version 1.0
*
* Description:
*  This file contains the code for the HRS service.
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

    
uint16_t energyExpended = 0u;

/* Heart Rate Measurement characteristic data structure */
cy_stc_ble_hrs_hrm_t hrsHeartRate;

/* The pointer and counter for the RR-Interval data */
uint8_t hrssRrIntPtr;
uint8_t hrssRrIntCnt;


/* Heart Rate Service callback */
void HeartRateCallBack(uint32_t event, void* eventParam)
{
    app_stc_connection_info_t *appConnInfoPtr = GetAppConnInfoPtr();
    uint32_t i;
    uint16_t rrInt;
    
    (void)eventParam;

    switch(event)
    {
        case CY_BLE_EVT_HRSS_NOTIFICATION_ENABLED:
            DBG_PRINTF("Heart Rate Measurement Notification is Enabled \r\n");
            appConnInfoPtr->central.isNotificationEn = true;
            break;

        case CY_BLE_EVT_HRSS_NOTIFICATION_DISABLED:
            DBG_PRINTF("Heart Rate Measurement Notification is Disabled \r\n");
            appConnInfoPtr->central.isNotificationEn = false;
            break;

        /* This event is received when the Client writes a control point characteristic value */
        case CY_BLE_EVT_HRSS_ENERGY_EXPENDED_RESET:
            energyExpended = 0u;
            HrssSetEnergyExpended(energyExpended);
            DBG_PRINTF("Energy Expended Reset \r\n");
            break;

            
        /* HRS Client - Heart Rate Measurement Characteristic Notification was received. The parameter of this event
        is a structure of the cy_stc_ble_hrs_char_value_t type. */
        case CY_BLE_EVT_HRSC_NOTIFICATION:
            DBG_PRINTF("CY_BLE_EVT_HRSC_NOTIFICATION: ");
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
                        DBG_PRINTF("    RR-Interval %ld: %d", i, HrscGetRRInterval(i));
                    }
                }

                DBG_PRINTF("\r\n");
            }
            else
            {
                DBG_PRINTF("Sensor Contact is supported but not detected \r\n");
            }
            
            (void)memcpy((void *) &appConnInfoPtr->central.hrsHrmCharValue,
                         (void *)((cy_stc_ble_hrs_char_value_t *)eventParam)->value->val,
                                 ((cy_stc_ble_hrs_char_value_t *)eventParam)->value->len);
            
            appConnInfoPtr->central.hrsHrmCharLen     = ((cy_stc_ble_hrs_char_value_t *)eventParam)->value->len;   
            appConnInfoPtr->central.isNewNotification = true;
            break;
        
        /* HRS Client - Read response for Read request of HRS 
            Service Characteristic value. The parameter of this event
            is a structure of the cy_stc_ble_hrs_char_value_t type.
        */
        case CY_BLE_EVT_HRSC_READ_CHAR_RESPONSE:
            DBG_PRINTF("CY_BLE_EVT_HRSC_READ_CHAR_RESPONSE \r\n");
            break;
        
        /* HRS Client - Write response for Write request of HRS 
            Service Characteristic value. The parameter of this event
            is a structure of the cy_stc_ble_hrs_char_value_t type.
        */
        case CY_BLE_EVT_HRSC_WRITE_CHAR_RESPONSE:
            DBG_PRINTF("CY_BLE_EVT_HRSC_WRITE_CHAR_RESPONSE \r\n");
            break;
        
        /* HRS Client - Read response for Read request of HRS
            Service Characteristic Descriptor Read request. The 
            parameter of this event is a structure of
            the cy_stc_ble_hrs_char_value_t type.
        */
        case CY_BLE_EVT_HRSC_READ_DESCR_RESPONSE:
            DBG_PRINTF("CY_BLE_EVT_HRSC_READ_DESCR_RESPONSE \r\n");
            break;
        
        /* HRS Client - Write response for Write request of HRS
            Service Characteristic Configuration Descriptor value.
            The parameter of this event is a structure of 
            the cy_stc_ble_hrs_char_value_t type.
        */
        case CY_BLE_EVT_HRSC_WRITE_DESCR_RESPONSE:  
            DBG_PRINTF("CY_BLE_EVT_HRSC_WRITE_DESCR_RESPONSE \r\n");
            break;
            
        default:
            DBG_PRINTF("Unknown HRS event: %lx \r\n", event);
            break;
    }
}

/*******************************************************************************
* Function Name: HrsInit
********************************************************************************
*
* Summary:
*   Initializes the Heart Rate application global variables.
*
*******************************************************************************/

void HrsInit(void)
{
    Cy_BLE_HRS_RegisterAttrCallback(HeartRateCallBack);

    hrsHeartRate.flags = 0u;
    hrsHeartRate.heartRateValue = 0u;
    hrsHeartRate.energyExpendedValue = 0u;

    memset(hrsHeartRate.rrInterval, 0, CY_BLE_HRS_HRM_RRSIZE);
}

/***************************************
*        Functions
***************************************/


/*******************************************************************************
* Function Name: HrssSetBodySensorLocation
********************************************************************************
*
* Summary:
*   Sets a Body Sensor Location characteristic value.
*
* Parameters:
*   cy_en_ble_hrs_bsl_t location:  The Body Sensor Location characteristic value.
*
* Return:
*   cy_en_ble_api_result_t: An API result will state if the API succeeded
*                           (CY_BLE_SUCCESS)
*                           or failed with error codes.
*
*******************************************************************************/
void HrssSetBodySensorLocation(cy_en_ble_hrs_bsl_t location)
{    
    if(Cy_BLE_HRSS_SetCharacteristicValue(CY_BLE_HRS_BSL, CY_BLE_HRS_BSL_CHAR_LEN, (uint8_t*)&location) != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("HrssSetBodySensorLocation API Error: ");
    }
    else
    {
        DBG_PRINTF("Body Sensor Location is set successfully: %d \r\n", location);
    }
}

/*******************************************************************************
* Function Name: HrssSetEnergyExpended
********************************************************************************
*
* Summary:
*   Sets an Energy Expended value into the Heart Rate Measurement characteristic
*
* Parameters:
*   energyExpended - The Energy Expended value to be set.
*
*******************************************************************************/
void HrssSetEnergyExpended(uint16_t energyExpended)
{
    hrsHeartRate.energyExpendedValue = energyExpended;
    hrsHeartRate.flags |= CY_BLE_HRS_HRM_ENEXP;
}

/*******************************************************************************
* Function Name: HrssGetHrmDescriptor
********************************************************************************
*
* Summary:
*   Gets the Heart Rate Measurement characteristic
*   Configuration Descriptor value.
*
* Return:
*   uint16_t The value of the Heart Rate Measurement characteristic
*   configuration descriptor.
*
*******************************************************************************/
uint16_t HrssGetHrmDescriptor(cy_stc_ble_conn_handle_t connHandle)
{
    uint8_t val[CY_BLE_CCCD_LEN];
    uint16_t retVal;

    if(CY_BLE_SUCCESS == Cy_BLE_HRSS_GetCharacteristicDescriptor(connHandle, CY_BLE_HRS_HRM,
        CY_BLE_HRS_HRM_CCCD, CY_BLE_CCCD_LEN, val))
    {
        /* Use the length as a temporary variable with the suitable type to return */
        retVal = Cy_BLE_Get16ByPtr(val);
    }
    else
    {
        retVal = 0u;
    }

    return (retVal);
}


/*******************************************************************************
* Function Name: HrssAddRrInterval
********************************************************************************
*
* Summary:
*   Adds the next RR-Interval into the
*   Heart Rate Measurement characteristic structure.
*
* Parameters:
*   rrIntervalValue: The RR-Interval value to be set.
*
*******************************************************************************/
void HrssAddRrInterval(uint16_t rrIntervalValue)
{
    hrsHeartRate.flags |= CY_BLE_HRS_HRM_RRINT;

    if(hrssRrIntPtr == CY_BLE_HRS_HRM_RRSIZE - 1)
    {
        hrssRrIntPtr = 0;
    }
    else
    {
        hrssRrIntPtr++;
    }

    if(hrssRrIntCnt <= CY_BLE_HRS_HRM_RRSIZE)
    {
        hrssRrIntCnt++;
    }

    hrsHeartRate.rrInterval[hrssRrIntPtr] = rrIntervalValue;
}


/*******************************************************************************
* Function Name: HrssSendHeartRateNtf
********************************************************************************
*
* Summary:
*   Packs the Heart Rate Measurement characteristic structure into the
*   uint8_t array prior to sending it to the collector. Also, clears the
*   CY_BLE_HRS_HRM_HRVAL16, CY_BLE_HRS_HRM_ENEXP and CY_BLE_HRS_HRM_RRINT flags.
*
* Parameters:
*   attHandle:  The pointer to the handle which consists of the device ID and ATT
*               connection ID.
*
* Return:
*   cy_en_ble_api_result_t: An API result will state if the API succeeded
*                      (CY_BLE_SUCCESS)
*                      or failed with error codes.
*
*******************************************************************************/
void HrssSendHeartRateNtf(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    uint16_t cccd;
        
    (void) Cy_BLE_HRSS_GetCharacteristicDescriptor(connHandle, CY_BLE_HRS_HRM, CY_BLE_HRS_HRM_CCCD, CY_BLE_CCCD_LEN, 
                                                  (uint8_t*)&cccd);
                                                        
    if(cccd == CY_BLE_CCCD_NOTIFICATION)
    {  
        uint8_t pdu[CY_BLE_HRS_HRM_CHAR_LEN];
        uint8_t nextPtr;
        uint8_t length;
        uint8_t rrInt;
        
        /* The Flags field is always the first byte */
        pdu[0u] = hrsHeartRate.flags & (uint8_t) ~CY_BLE_HRS_HRM_HRVAL16;

        /* If the Heart Rate value exceeds one byte, ... */
        if(hrsHeartRate.heartRateValue > 0x00FFu)
        {
            /* ... then set a CY_BLE_HRS_HRM_HRVAL16 flag */
            pdu[0u] |= CY_BLE_HRS_HRM_HRVAL16;
            /* and set a full 2-byte Heart Rate value */
            Cy_BLE_Set16ByPtr(&pdu[1u], hrsHeartRate.heartRateValue);
            /* The next data will be located beginning from the 3rd byte */
            nextPtr = 3u;
        }
        else
        {
            /* Else leave the CY_BLE_HRS_HRM_HRVAL16 flag remains being cleared */
            /* and set only the LSB of the  Heart Rate value */
            pdu[1u] = (uint8_t) hrsHeartRate.heartRateValue;
            /* The next data will be located beginning from the 2nd byte */
            nextPtr = 2u;
        }

        /* If an Energy Expended flag is set ... */
        if(0u != (hrsHeartRate.flags & CY_BLE_HRS_HRM_ENEXP))
        {
            /* ... clear the CY_BLE_HRS_HRM_ENEXP flag */
            hrsHeartRate.flags &= (uint8_t) ~CY_BLE_HRS_HRM_ENEXP;
            /* and set a 2-byte Energy Expended value */
            Cy_BLE_Set16ByPtr(&pdu[nextPtr], hrsHeartRate.energyExpendedValue);
            /* add 2 bytes: the Energy Expended value is uint16_t */
            nextPtr += 2u;
        }

        if(HrssAreThereRrIntervals())
        {
            /* Calculate the actual length of pdu: the RR-interval block length should be an even number */
            length = ((CY_BLE_HRS_HRM_CHAR_LEN - nextPtr) & ~0x01) + nextPtr;

            rrInt = hrssRrIntPtr;

            while(nextPtr < length)
            {
                /* Increment the rrInterval array pointer in the RR-Interval buffer size loop */
                rrInt++;
                if(rrInt >= CY_BLE_HRS_HRM_RRSIZE)
                {
                    rrInt = 0;
                }

                if(hrsHeartRate.rrInterval[rrInt] != 0)
                {
                    /* Copy the non-zero RR-Interval into pdu */
                    Cy_BLE_Set16ByPtr(&pdu[nextPtr], hrsHeartRate.rrInterval[rrInt]);
                    /* Clear the current RR-Interval in the buffer */
                    hrsHeartRate.rrInterval[rrInt] = 0;
                    /* Add 2 bytes: the RR-Interval value is uint16_t */
                    nextPtr += 2u;
                }

                if(rrInt == hrssRrIntPtr)
                {
                    hrsHeartRate.flags &= (uint8_t) ~CY_BLE_HRS_HRM_RRINT;
                    break;
                }
            }

            hrssRrIntCnt = 0;
        }

        do
        {
            Cy_BLE_ProcessEvents();
        }
        while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);

        apiResult = Cy_BLE_HRSS_SendNotification(connHandle, CY_BLE_HRS_HRM, nextPtr, pdu);
        
        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("Cy_BLE_HRSS_SendNotification API Error: 0x%x\r\n", apiResult);
        }
        else
        {
            DBG_PRINTF("Simulated Heart Rate = %d \r\n", hrsHeartRate.heartRateValue);
        }
    }
}

/*******************************************************************************
* Function Name: HrsSimulateHeartRate
********************************************************************************
*
* Summary:
*   Simulates all constituents of the Heart Rate Measurement characteristic.
*
*******************************************************************************/
void HrsSimulateHeartRate(cy_stc_ble_conn_handle_t connHandle)
{
    static uint32_t hrsTimer = HRS_TIMEOUT;

    if(--hrsTimer == 0u)  
    {
        static uint32_t sensorContactTimer    = SENSOR_CONTACT_TIMEOUT;
        static uint32_t energyExpendedTimer   = ENERGY_EXPECTED_TIMEOUT;
        static uint16_t heartRate             = SIM_HEART_RATE_MIN;
        static uint8_t  sensorContact         = 0u;
        
        uint16_t rrInterval;
        uint8_t  rrIntCnt;
        
        hrsTimer = HRS_TIMEOUT;

        /* Heart Rate simulation */
        heartRate += SIM_HEART_RATE_INCREMENT;
        if(heartRate > SIM_HEART_RATE_MAX)
        {
            heartRate = SIM_HEART_RATE_MIN;
        }

        HrssSetHeartRate(heartRate);
        
        /* RR-Interval calculation */    
        /* rrInterval = 60 000 mSec (1 min) / heartRate */
        rrInterval = 60000u / heartRate;
        
        /* count of RR-intervals per one second (1000 mSec) */
        rrIntCnt = (uint8_t)(1000 / rrInterval);

        while(rrIntCnt > 0u)
        {
            if(!HrssIsRrIntervalBufferFull())
            {
                HrssAddRrInterval(rrInterval);
            }
            rrInterval++;
            rrIntCnt--;
        }
        
        /* Energy Expended simulation */
        if(--energyExpendedTimer == 0)
        {
            energyExpendedTimer = ENERGY_EXPECTED_TIMEOUT;

            HrssSetEnergyExpended(energyExpended);

            if(energyExpended < (CY_BLE_ENERGY_EXPENDED_MAX_VALUE - SIM_ENERGY_EXPENDED_INCREMENT))
            {
                energyExpended += SIM_ENERGY_EXPENDED_INCREMENT;
            }
        }
        
        /* Simulate rare sensor disconnection */
        if(--sensorContactTimer == 0)
        {
            sensorContactTimer = SENSOR_CONTACT_TIMEOUT;

            sensorContact ^= SENSOR_CONTACT_DETECTED;
            if(0u != (sensorContact & SENSOR_CONTACT_DETECTED))
            {
                HrssSensorContactIsDetected();
            }
            else
            {
                HrssSensorContactIsUndetected();
            }
        }

        HrssSendHeartRateNtf(connHandle);
    }
    
}

/*******************************************************************************
* Function Name: HrsBridge
********************************************************************************
*
* Summary:
*   The HRS Bridge between the Client and Server.
*
*******************************************************************************/
void HrsBridge(void)
{
    cy_en_ble_api_result_t apiResult;
    app_stc_connection_info_t *appConnInfoPtr = GetAppConnInfoPtr();
    
    
    /* Send notification */
    apiResult = Cy_BLE_HRSS_SendNotification(appConnInfoPtr->peripheral.connHandle, CY_BLE_HRS_HRM, 
                                             appConnInfoPtr->central.hrsHrmCharLen, 
                                             appConnInfoPtr->central.hrsHrmCharValue);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_HRSS_SendNotification API Error: 0x%x \r\n", apiResult);
    }
    else
    {
        DBG_PRINTF("Cy_BLE_HRSS_SendNotification\r\n");
    }
    
    /* Reset flag that we have new notification */
    appConnInfoPtr->central.isNewNotification = false;
}


/*******************************************************************************
* Function Name: HrscUnPackHrm
********************************************************************************
*
* Summary:
*   Parse the HRS notification packet.
*
* Parameters:
*   value
*
*******************************************************************************/
void HrscUnPackHrm(cy_stc_ble_gatt_value_t* value)
{
    uint8_t nextPtr;
    uint8_t rrInt;
    uint8_t i;
    uint8_t * pdu;

    pdu = value->val;

    /* The Flags field is always the first byte */
    hrsHeartRate.flags = pdu[0u];

    if((hrsHeartRate.flags & CY_BLE_HRS_HRM_HRVAL16) != 0u)
    {
        /* Heart Rate Measurement Value is also located beginning from the first byte */
        hrsHeartRate.heartRateValue =
            Cy_BLE_Get16ByPtr(&pdu[1u]);
        /* the next data will be located beginning from the 3rd byte */
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
        hrsHeartRate.energyExpendedValue =
            Cy_BLE_Get16ByPtr(&pdu[nextPtr]);
        /* Add 2 bytes: the Energy Expended field is uint16_t */
        nextPtr += 2u;
    }

    if((hrsHeartRate.flags & CY_BLE_HRS_HRM_RRINT) != 0u)
    {
        /* Calculate how many RR-Intervals are in this pdu ... */
        rrInt = (uint8_t)(((uint8_t)(value->len) - nextPtr) >> 1);

        for(i = 0u; i < CY_BLE_HRS_HRM_RRSIZE; i++)
        {
            if(i < rrInt) /* while there are RR-Interval values in this pdu */
            {
                hrsHeartRate.rrInterval[i] =
                    Cy_BLE_Get16ByPtr(&pdu[nextPtr]);
                /* Add 2 bytes: the RR-Interval field is uint16_t */
                nextPtr += 2u;
            }
            else /* Fill the rest of the R-Interval buffer with zeros */
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
*   Heart Rate Measurement characteristic structure.
*
* Parameters:
*   rrIntervalNumber - The number of the desired RR-Intervals.
*
* Return:
*   uint16_t The rrInterval.
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
        retVal = 0u; /* If the Energy Expended field is not present, then return 0 */
    }

    return (retVal);
}


/* [] END OF FILE */
