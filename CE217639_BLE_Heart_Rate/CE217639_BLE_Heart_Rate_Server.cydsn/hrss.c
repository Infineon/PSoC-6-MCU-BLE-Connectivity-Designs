/*******************************************************************************
* File Name: hrss.c
*
* Version 1.0
*
* Description:
*  This file contains HRS service related code.
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"
#include "hrss.h"

uint16_t energyExpended = 0u;

/* Heart Rate Measurement characteristic data structure */
cy_stc_ble_hrs_hrm_t hrsHeartRate;

/* Pointer and counter for RR-Interval data */
uint8_t hrssRrIntPtr;
uint8_t hrssRrIntCnt;

/* Heart Rate Service callback */
void HrsCallBack(uint32_t event, void* eventParam)
{
    if(0u != eventParam)
    {
        /* This dummy operation is to avoid warning about unused eventParam */
    }

    switch(event)
    {
        case CY_BLE_EVT_HRSS_NOTIFICATION_ENABLED:
            DBG_PRINTF("Heart Rate Measurement Notification is Enabled \r\n");
            break;

        case CY_BLE_EVT_HRSS_NOTIFICATION_DISABLED:
            DBG_PRINTF("Heart Rate Measurement Notification is Disabled \r\n");
            break;

        /* This event is received when client writes Control Point characteristic value */
        case CY_BLE_EVT_HRSS_ENERGY_EXPENDED_RESET:
            energyExpended = 0u;
            HrssSetEnergyExpended(energyExpended);
            DBG_PRINTF("Energy Expended Reset \r\n");
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
*  Initializes the Heart Rate callBack and application global variables.
*
*******************************************************************************/
void HrsInit(void)
{
    uint32_t i;
    
    Cy_BLE_HRS_RegisterAttrCallback(HrsCallBack);

    hrsHeartRate.flags = 0u;
    hrsHeartRate.heartRateValue = 0u;
    hrsHeartRate.energyExpendedValue = 0u;

    for(i = 0; i < CY_BLE_HRS_HRM_RRSIZE; i++)
    {
        hrsHeartRate.rrInterval[i] = 0u;
    }
}


/*******************************************************************************
* Function Name: HrssSetBodySensorLocation
********************************************************************************
*
* Summary:
*  Set the Body Sensor Location characteristic value.
*
* Parameters:
*  cy_en_ble_hrs_bsl_t location:  Body Sensor Location characteristic value.
* Return:
*  cy_en_ble_api_result_t: API result will state if API succeeded
*                      (CY_BLE_SUCCESS)
*                      or failed with error codes.
*
*******************************************************************************/
void HrssSetBodySensorLocation(cy_en_ble_hrs_bsl_t location)
{
    cy_en_ble_api_result_t apiResult;
    
    apiResult = Cy_BLE_HRSS_SetCharacteristicValue(CY_BLE_HRS_BSL, CY_BLE_HRS_BSL_CHAR_LEN, (uint8_t*)&location);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("HrssSetBodySensorLocation API Error: ");
        PrintApiResult(apiResult);
    }
    else
    {
        DBG_PRINTF("Body Sensor Location is set successfully: %d \r\n", location);
    }
}

/*******************************************************************************
* Function Name: Cy_BLE_HRSS_SetEnergyExpended
********************************************************************************
*
* Summary:
*  Sets Energy Expended value into the Heart Rate Measurument characteristic
*
* Parameters:
*  uint16_t energyExpended - Energy Expended value to be set.
*
*******************************************************************************/
void HrssSetEnergyExpended(uint16_t energyExpended)
{
    hrsHeartRate.energyExpendedValue = energyExpended;
    hrsHeartRate.flags |= CY_BLE_HRS_HRM_ENEXP;
}


/*******************************************************************************
* Function Name: HrssAddRrInterval
********************************************************************************
*
* Summary:
*  Adds the next RR-Interval into the
*  Heart Rate Measurument characteristic structure.
*
* Parameters:
*  uint16_t rrIntervalValue: RR-Interval value to be set.
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
*  Packs the Heart Rate Measurement characteristic structure into the
*  uint8_t array prior to sending it to the collector. Also clears the
*  CY_BLE_HRS_HRM_HRVAL16, CY_BLE_HRS_HRM_ENEXP and CY_BLE_HRS_HRM_RRINT flags.
*
* Parameters:
*  connHandle: The connection handle
*
* Return:
*  cy_en_ble_api_result_t: API result will state if API succeeded
*                      (CY_BLE_SUCCESS)
*                      or failed with error codes.
*
*******************************************************************************/
void HrssSendHeartRateNtf(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    uint16_t cccd;
        
    (void) Cy_BLE_HRSS_GetCharacteristicDescriptor(connHandle, CY_BLE_HRS_HRM, CY_BLE_HRS_HRM_CCCD, CY_BLE_CCCD_LEN,
                                                   (uint8_t *) &cccd);                                                        
    if(cccd == CY_BLE_CCCD_NOTIFICATION)
    { 
        uint8_t pdu[CY_BLE_HRS_HRM_CHAR_LEN];
        uint8_t nextPtr;
        uint8_t length;
        uint8_t rrInt;
        
        /* Flags field is always the first byte */
        pdu[0u] = hrsHeartRate.flags & (uint8_t) ~CY_BLE_HRS_HRM_HRVAL16;

        /* If the Heart Rate value exceeds one byte */
        if(hrsHeartRate.heartRateValue > 0x00FFu)
        {
            /* then set the CY_BLE_HRS_HRM_HRVAL16 flag */
            pdu[0u] |= CY_BLE_HRS_HRM_HRVAL16;
            /* and set the full 2-bytes Heart Rate value */
            Cy_BLE_Set16ByPtr(&pdu[1u], hrsHeartRate.heartRateValue);
            /* The next data will be located beginning from 3rd byte */
            nextPtr = 3u;
        }
        else
        {
            /* Else leave the CY_BLE_HRS_HRM_HRVAL16 flag remains being cleared */
            /* and set only LSB of the  Heart Rate value */
            pdu[1u] = (uint8_t) hrsHeartRate.heartRateValue;
            /* The next data will be located beginning from 2nd byte */
            nextPtr = 2u;
        }

        /* If the Energy Expended flag is set */
        if(0u != (hrsHeartRate.flags & CY_BLE_HRS_HRM_ENEXP))
        {
            /* clear the CY_BLE_HRS_HRM_ENEXP flag */
            hrsHeartRate.flags &= (uint8_t) ~CY_BLE_HRS_HRM_ENEXP;
            /* and set the 2-bytes Energy Expended value */
            Cy_BLE_Set16ByPtr(&pdu[nextPtr], hrsHeartRate.energyExpendedValue);
            /* add 2 bytes: Energy Expended value is uint16_t */
            nextPtr += 2u;
        }

        if(HrssAreThereRrIntervals())
        {
            /* Calculate the actual length of pdu: the RR-interval block length should be an even number */
            length = ((CY_BLE_HRS_HRM_CHAR_LEN - nextPtr) & ~0x01) + nextPtr;

            rrInt = hrssRrIntPtr;

            while(nextPtr < length)
            {
                /* Increment the rrInterval array pointer in RR-Interval buffer size loop */
                rrInt++;
                if(rrInt >= CY_BLE_HRS_HRM_RRSIZE)
                {
                    rrInt = 0;
                }

                if(hrsHeartRate.rrInterval[rrInt] != 0)
                {
                    /* Copy the non-zero RR-Interval into the pdu */
                    Cy_BLE_Set16ByPtr(&pdu[nextPtr], hrsHeartRate.rrInterval[rrInt]);
                    /* Clear the current RR-Interval in the buffer */
                    hrsHeartRate.rrInterval[rrInt] = 0;
                    /* Add 2 bytes: RR-Interval value is uint16_t */
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

        if(Cy_BLE_GetConnectionState(connHandle) == CY_BLE_CONN_STATE_CONNECTED) 
        {
            apiResult = Cy_BLE_HRSS_SendNotification(connHandle, CY_BLE_HRS_HRM, nextPtr, pdu);
            
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("HrssSendHeartRateNtf API Error: ");
                PrintApiResult(apiResult);
            }
            else
            {
                DBG_PRINTF("Heart Rate Notification is sent successfully, Heart Rate = %d \r\n", hrsHeartRate.heartRateValue);
            }
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
* Parameters:
*  connHandle: The connection handle
*
*******************************************************************************/
void HrsSimulateHeartRate(cy_stc_ble_conn_handle_t connHandle)
{
    static uint8_t hrsTimer = HRS_TIMEOUT;
    
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

/* [] END OF FILE */
