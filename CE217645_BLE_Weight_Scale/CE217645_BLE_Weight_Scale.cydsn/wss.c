/*******************************************************************************
* File Name: wss.c
*
* Version: 1.0
*
* Description:
*  This file contains routines related to Weight Scale Service.
* 
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"
#include "wss.h"


/***************************************
*        Global Variables
***************************************/
bool                        isWssIndicationEnabled = false;
bool                        isWssIndicationPending = false;
wss_measurement_value_t     weightMeasurement[MAX_USERS];
wss_measurement_value_t     weightMeasurementDef;
uint8_t                     wssIndData[WSS_WS_MEASUREMENT_MAX_DATA_SIZE];
uint32_t                    wssFeature;


/*******************************************************************************
* Function Name: WssCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE Component
*   specific to Weight Scale Service.
*
* Parameters:
*   event - A WSS event.
*   eventParams - The data structure specific to an event received.
*
*******************************************************************************/
void WssCallBack(uint32_t event, void *eventParam)
{
    switch(event)
    {

    /****************************************************
    *              WSS Server events
    ****************************************************/
    /* WSS Server - Indication for Weight Scale Service Characteristic
        was enabled. Parameter of this event is structure 
        of cy_stc_ble_wss_char_value_t type.
    */
    case CY_BLE_EVT_WSSS_INDICATION_ENABLED:
        DBG_PRINTF("CY_BLE_EVT_WSSS_INDICATION_ENABLED\r\n");
        isWssIndicationEnabled = true;
        break;

    /* WSS Server - Indication for Weight Scale Service Characteristic
        was disabled. Parameter of this event is structure 
        of cy_stc_ble_wss_char_value_t type.
    */
    case CY_BLE_EVT_WSSS_INDICATION_DISABLED:
        DBG_PRINTF("CY_BLE_EVT_WSSS_INDICATION_DISABLED\r\n");
        isWssIndicationEnabled = false;
        break;

    /* WSS Server - Weight Scale Service Characteristic
        Indication was confirmed. Parameter of this event
        is structure of cy_stc_ble_wss_char_value_t type.
    */
    case CY_BLE_EVT_WSSS_INDICATION_CONFIRMED:
        DBG_PRINTF("CY_BLE_EVT_WSSS_INDICATION_CONFIRMED\r\n");
        break;

    /****************************************************
    *               WSS Client events
    * These events are not active for the Server device
    * operation. They are added as a template for the
    * Client mode operation to ease the user experience
    * with Weight Scale Service.
    ****************************************************/
    /* WSS Client - Weight Scale Service Characteristic
        Indication was received. Parameter of this event
        is structure of cy_stc_ble_wss_char_value_t type.
    */
    case CY_BLE_EVT_WSSC_INDICATION:
        break;

    /* WSS Client - Read Response for Read Request of Weight Scale 
        Service Characteristic value. Parameter of this event
        is structure of cy_stc_ble_wss_char_value_t type.
    */
    case CY_BLE_EVT_WSSC_READ_CHAR_RESPONSE:
        break;

    /* WSS Client - Read Response for Read Request of Weight Scale
        Service Characteristic Descriptor Read request. 
        Parameter of this event is structure of
        cy_stc_ble_wss_descr_value_t type.
    */
    case CY_BLE_EVT_WSSC_READ_DESCR_RESPONSE:
        break;

    /* WSS Client - Write Response for Write Request of Weight Scale
        Service Characteristic Configuration Descriptor value.
        Parameter of this event is structure of 
        cy_stc_ble_wss_descr_value_t type.
    */
    case CY_BLE_EVT_WSSC_WRITE_DESCR_RESPONSE:
        break;

    default:
        DBG_PRINTF("Unrecognized WSS event.\r\n");
        break;
    }
}


/*******************************************************************************
* Function Name: WssInit
********************************************************************************
*
* Summary:
*   Initializes the variables related to the Weight Scale Service's 
*   characteristics with values from the BLE component customizer's GUI. 
*
*******************************************************************************/
void WssInit(void)
{
    uint8_t count = 0u;
    uint8_t i;
    cy_en_ble_api_result_t apiResult;
    uint8_t rdData[WSS_WS_MEASUREMENT_MAX_DATA_SIZE]; 
    
    /* Register event handler for WSS specific events */
    Cy_BLE_WSS_RegisterAttrCallback(WssCallBack);
    
    /* Read initial values of Weight Feature Characteristic */
    apiResult =
        Cy_BLE_WSSS_GetCharacteristicValue(CY_BLE_WSS_WEIGHT_SCALE_FEATURE, WSS_WEIGHT_FEATURE_CHAR_SIZE, rdData);

    if(apiResult == CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Weight Feature Characteristic was read successfully \r\n");

        /* Get Weight Feature and store it as uint32_t */
        wssFeature = (uint32) ( ((uint32) rdData[BYTE_0]) |
                                (((uint32) rdData[BYTE_1]) << BYTE_SHIFT) |
                                (((uint32) rdData[BYTE_2]) << TWO_BYTES_SHIFT) |
                                (((uint32) rdData[BYTE_3]) << THREE_BYTES_SHIFT));
    }
    else
    {
        DBG_PRINTF("Error while reading Weight Feature Characteristic. Error code: %d \r\n", apiResult);
    }

    /* Read initial values of Weight Measurement Characteristic */
    apiResult =
        Cy_BLE_WSSS_GetCharacteristicValue(CY_BLE_WSS_WEIGHT_MEASUREMENT, WSS_WS_MEASUREMENT_MAX_DATA_SIZE, rdData);

    if(apiResult == CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Weight Measurement Characteristic was read successfully \r\n");

        /* Fill data into Weight Measurement Characteristic into the
        * structure for easier access to Characteristic fields.
        */
        weightMeasurementDef.flags = rdData[count++];
        weightMeasurementDef.weightKg = PACK_U16(rdData[count], rdData[count + 1u]);
        count+=2u;
        weightMeasurementDef.weightLb = PACK_U16(rdData[count], rdData[count + 1u]);
        count+=2u;
        weightMeasurementDef.year = PACK_U16(rdData[count], rdData[count + 1u]);
        count+=2u;
        weightMeasurementDef.month = rdData[count++];
        weightMeasurementDef.day = rdData[count++];
        weightMeasurementDef.hour = rdData[count++];
        weightMeasurementDef.minutes = rdData[count++];
        weightMeasurementDef.seconds = rdData[count++];
        weightMeasurementDef.userId = rdData[count++];
        weightMeasurementDef.bmi = PACK_U16(rdData[count], rdData[count + 1u]);
        count+=2u;
        weightMeasurementDef.heightM = PACK_U16(rdData[count], rdData[count + 1u]);
        count+=2u;
        weightMeasurementDef.heightIn = PACK_U16(rdData[count], rdData[count + 1u]);
        count+=2u;
        
        for(i = 0u; i < MAX_USERS; i++)
        {
            memcpy(&weightMeasurement[i], &weightMeasurementDef, sizeof(weightMeasurementDef));
        }
    }
    else
    {
        DBG_PRINTF("Error while reading Weight Measurement Characteristic. Error code: %d \r\n", apiResult);
    }
}


/*******************************************************************************
* Function Name: WssPackIndicationData
********************************************************************************
*
* Summary:
*   Packs the Weight Measurement Characteristic value data to be indicated.
*
* Parameters:
*   pData -      The pointer to the buffer where indication data will be stored.
*   length -     The length of a buffer. After the function execution this parameter
*                will contain the actual length of bytes written to the buffer.
*   bMeasurement - The pointer to the Weight Measurement Characteristic value.
*
* Return
*   WSS_RET_SUCCESS - Success.
*   WSS_RET_FAILURE - Failure. The buffer allocated for indication data is too
*                     small.
*
*******************************************************************************/
uint8_t WssPackIndicationData(uint8_t *pData, uint8_t *length, wss_measurement_value_t *wMeasurement)
{
    uint8_t i;
    uint8_t result = WSS_RET_FAILURE;
    uint16_t flagMask;
    uint8_t currLength = 0u;

    if(*length != 0u)
    {
        pData[currLength++] = CY_LO8(wMeasurement->flags);
        
        if((wMeasurement->flags & WSS_MEASUREMENT_UNITS_IMPERIAL) != 0u)
        {
            pData[currLength++] = CY_LO8(wMeasurement->weightLb);
            pData[currLength++] = CY_HI8(wMeasurement->weightLb);
        }
        else
        {
            pData[currLength++] = CY_LO8(wMeasurement->weightKg);
            pData[currLength++] = CY_HI8(wMeasurement->weightKg);
        }

        for(i = 1u; i < WSS_FLAGS_COUNT; i++)
        {
            /* 'flagMask' contains mask for bit in the wMeasurement->flags.
            * Used for parsing flags and adding respective data to indication. 
            */
            flagMask = 1u << i;
            
            switch(wMeasurement->flags & flagMask)
            {
            /* Add time stamp data */
            case WSS_MEASUREMENT_TIME_STAMP_PRESENT:
                pData[currLength++] = CY_LO8(wMeasurement->year);
                pData[currLength++] = CY_HI8(wMeasurement->year);
                pData[currLength++] = wMeasurement->month;
                pData[currLength++] = wMeasurement->day;
                pData[currLength++] = wMeasurement->hour;
                pData[currLength++] = wMeasurement->minutes;
                pData[currLength++] = wMeasurement->seconds;
                break;
                
            /* Add user ID data */
            case WSS_USER_ID_PRESENT:
                pData[currLength++] = wMeasurement->userId;
                break;
                
            /* Add BMI and height data */
            case WSS_BMI_AND_HEIGHT_PRESENT:
                pData[currLength++] = CY_LO8(wMeasurement->bmi);
                pData[currLength++] = CY_HI8(wMeasurement->bmi);
                    
                if((wMeasurement->flags & WSS_MEASUREMENT_UNITS_IMPERIAL) != 0u)
                {
                    pData[currLength++] = CY_LO8(wMeasurement->heightIn);
                    pData[currLength++] = CY_HI8(wMeasurement->heightIn);
                }
                else
                {
                    pData[currLength++] = CY_LO8(wMeasurement->heightM);
                    pData[currLength++] = CY_HI8(wMeasurement->heightM);
                }
                break;
                
            /* Feature is disabled */
            default:
                break;
            }
        }

        if(currLength <= *length)
        {
            *length = currLength;
            result = WSS_RET_SUCCESS;
        }
    }

    return(result);
}


/* [] END OF FILE */
