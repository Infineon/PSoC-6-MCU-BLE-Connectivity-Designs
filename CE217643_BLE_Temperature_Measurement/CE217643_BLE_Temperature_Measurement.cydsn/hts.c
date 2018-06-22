/*******************************************************************************
* File Name: hts.c
*
* Version: 1.0
*
* Description:
*  This file contains routines related to HTS.
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
* 
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"
#include "hts.h"
#include "user_interface.h"

static uint32_t temperatureTimer = 1u;

static uint16_t initialMeasurementInterval = 10;
static bool     measure = true; 

/* Byte order: 
*   Flags
*   Temperature in float format (MSB 8 bits - exponent, LSB 24 bits - mantissa) 
*/
static uint8_t temp_data[HTS_TEMP_DATA_MIN_SIZE] = {0u, 0u, 0u, 0u, 0u};


/*******************************************************************************
* Function Name: HtsInit
********************************************************************************
*
* Summary:
*   Initializes the HTS service.
*
*******************************************************************************/
void HtsInit(void)
{
   
    /* Register service specific callback function */
    Cy_BLE_HTS_RegisterAttrCallback(HtsCallBack);
}


/*******************************************************************************
* Function Name: HtsCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service specific events from 
*   Health Thermometer Service.
*
* Parameters:
*   event - the event code
*   eventParam - the event parameters
*
*******************************************************************************/
void HtsCallBack(uint32_t event, void *eventParam)
{
    uint8_t locCharIndex;
    locCharIndex = ((cy_stc_ble_hts_char_value_t *)eventParam)->charIndex;
    DBG_PRINTF("HTS event: %lx, ", event);

    switch(event)
    {
        case CY_BLE_EVT_HTSS_NOTIFICATION_ENABLED:
            break;
        case CY_BLE_EVT_HTSS_NOTIFICATION_DISABLED:
            break;
        case CY_BLE_EVT_HTSS_INDICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_HTSS_INDICATION_ENABLED: char: %x\r\n", locCharIndex);
            if(locCharIndex == CY_BLE_HTS_TEMP_MEASURE)
            {
                measure = true;
                temp_data[0] = 0u;
            }
            break;
        case CY_BLE_EVT_HTSS_INDICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_HTSS_INDICATION_DISABLED: char: %x\r\n", locCharIndex);
            break;
        case CY_BLE_EVT_HTSS_INDICATION_CONFIRMED:
            DBG_PRINTF("CY_BLE_EVT_HTSS_INDICATION_CONFIRMED\r\n");
            break;
        case CY_BLE_EVT_HTSS_WRITE_CHAR:
            DBG_PRINTF("CY_BLE_EVT_HTSS_CHAR_WRITE: %x ", locCharIndex);
            ShowValue(((cy_stc_ble_hts_char_value_t *)eventParam)->value);
            if(locCharIndex == CY_BLE_HTS_MEASURE_INTERVAL)
            {
                initialMeasurementInterval = Cy_BLE_Get16ByPtr(((cy_stc_ble_hts_char_value_t *)eventParam)->value->val);
                
                /* Store new interval value */
                if(initialMeasurementInterval != 0u)
                {
                    /* Reset the measurement interval and start the measurement process */
                    temperatureTimer = 1u;
                }
            }
            break;
        case CY_BLE_EVT_HTSC_NOTIFICATION:
            break;
        case CY_BLE_EVT_HTSC_INDICATION:
            break;
        case CY_BLE_EVT_HTSC_READ_CHAR_RESPONSE:
            break;
        case CY_BLE_EVT_HTSC_WRITE_CHAR_RESPONSE:
            break;
        case CY_BLE_EVT_HTSC_READ_DESCR_RESPONSE:
            break;
        case CY_BLE_EVT_HTSC_WRITE_DESCR_RESPONSE:
            break;
        default:
            DBG_PRINTF("Not supported event\r\n");
            break;
    }
}


/*******************************************************************************
* Function Name: HtsMeasureTemperature
********************************************************************************
*
* Summary:
*   This function measures the die temperature and sends it to the client.
*
* Parameters:
*   connHandle - the connection handle
*
*******************************************************************************/
void HtsMeasureTemperature(cy_stc_ble_conn_handle_t connHandle)
{
    int32_t temperatureValue;
    static int32_t temperatureCelsius;
    cy_en_ble_api_result_t apiResult;
    uint16_t cccd;
    
    /* Send data */
    apiResult = Cy_BLE_HTSS_GetCharacteristicDescriptor(connHandle, CY_BLE_HTS_TEMP_MEASURE, CY_BLE_HTS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
          
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_HTSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
    }
    else if(cccd == CY_BLE_CCCD_INDICATION) 
    {
        /* Do not measure temperature when 0 interval is set */
        if((initialMeasurementInterval != 0u) && (--temperatureTimer == 0u)) 
        {
            temperatureTimer = initialMeasurementInterval;
            
            /* Measure Die temperature only first time, later simulate */
            if(measure == true)
            {
                measure = false;                
                /* Measure die temperature when Cy_BLE_GetTemperature() function will be implemented */
                
                /* Convert the ADC output to degrees Celsius */
                temperatureCelsius = SIM_TEMPERATURE_MIN;

            }
            else /* Temperature Simulation */
            {
                temperatureCelsius += SIM_TEMPERATURE_INCREMENT;
                if(temperatureCelsius > SIM_TEMPERATURE_MAX)
                {
                    temperatureCelsius = SIM_TEMPERATURE_MIN; 
                }
            }
            
            /* Convert Celsius to Fahrenheit if required */
            if((temp_data[0] & CY_BLE_HTS_MEAS_FLAG_TEMP_UNITS_BIT) != 0u)
            {
                temperatureValue = 32 + ((temperatureCelsius * 9) / 5);
            }
            else
            {
                temperatureValue = temperatureCelsius;
            }
            
            /* Convert int32_t to the IEEE-11073 FLOAT-Type.
            *  It is defined as a 32-bit value with a 24-bit mantissa and an 8-bit exponent.
            *  For the integer temperature, 24-bit value has been used with mantissa equal to 0 */
            temperatureValue &= 0x00FFFFFF;

            /* Copy temperature to array */
            Set32ByPtr(temp_data + 1u, (uint32)temperatureValue);
            
            do
            {
                Cy_BLE_ProcessEvents();
            }
            while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);
            
            if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
            {
                /* Send temperature to client */
                apiResult = Cy_BLE_HTSS_SendIndication(connHandle, CY_BLE_HTS_TEMP_MEASURE, 
                                                        sizeof(temp_data), temp_data);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_HTSS_SendIndication API Error: %x \r\n", apiResult);
                }
                else
                {
                    DBG_PRINTF("MeasureTemperature: %d %c  ", (int16_t)temperatureValue,
                    (((temp_data[0] & CY_BLE_HTS_MEAS_FLAG_TEMP_UNITS_BIT) != 0u) ? 'F' : 'C'));
                }
            }

            /* Toggle the temperature unit flag on each temperature update */
            temp_data[0] ^= CY_BLE_HTS_MEAS_FLAG_TEMP_UNITS_BIT;
        }
    }
}

/* [] END OF FILE */

