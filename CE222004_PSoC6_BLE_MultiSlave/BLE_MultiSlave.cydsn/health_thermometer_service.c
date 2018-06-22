/*******************************************************************************
* File Name: health_thermometer_service.c
*
* Version: 1.0
*
* Description:
*  This file contains Health Thermometer Service related functions.
* 
*******************************************************************************
* Copyright (2017-2018), Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress’s integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress 
* reserves the right to make changes to the Software without notice. Cypress 
* does not assume any liability arising out of the application or use of the 
* Software or any product or circuit described in the Software. Cypress does 
* not authorize its products for use in any products where a malfunction or 
* failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such 
* system or application assumes all risk of such use and in doing so agrees to 
* indemnify Cypress against all liability.
*******************************************************************************/
#include "health_thermometer_service.h"
#include "temperature.h"
#include "math.h"

/* Macros used for indexing */
#define HTS_TEMP_VAL_INDEX_0                (1u)
#define HTS_TEMP_VAL_INDEX_1                (2u)
#define HTS_TEMP_VAL_INDEX_2                (3u)
#define HTS_TEMP_VAL_INDEX_3                (4u)

/*******************************************************************************
* Function Name: HtsGetTemperature
********************************************************************************
*
* Summary:
*  This function calculates the temperature and convert it into proper format
*
* Parameters:
*  temperatureArray : Array that holds temperature value
*
* Return:
*  None
*
*******************************************************************************/
void HtsGetTemperature(uint8_t* temperatureArray)
{
    /* Temporary array to hold Health Thermometer characteristic information */
    float temperature;
    temperature_data_t tempData;
    
    /* Read the temperature value */
    temperature = GetTemperature();
                
    /* Convert from IEEE-754 single precision floating point format to
       IEEE-11073 FLOAT, which is required for the health thermometer
       charactsristics */
    tempData.temperatureValue = (int32_t)(roundf(temperature*100));
    tempData.temperatureArray[3] = (uint8_t)(-2);
    
    /* Read Health Thermometer Characteristic from GATT DB */
    if(Cy_BLE_HTSS_GetCharacteristicValue(CY_BLE_HTS_TEMP_MEASURE, HTS_DATA_LEN, temperatureArray) == CY_BLE_SUCCESS)
    { 
        /* Update temperature in the characteristic */
        temperatureArray[HTS_TEMP_VAL_INDEX_0] = tempData.temperatureArray[0];
        temperatureArray[HTS_TEMP_VAL_INDEX_1] = tempData.temperatureArray[1];
        temperatureArray[HTS_TEMP_VAL_INDEX_2] = tempData.temperatureArray[2];
        temperatureArray[HTS_TEMP_VAL_INDEX_3] = tempData.temperatureArray[3];
    }
}

/*******************************************************************************
* Function Name: HtsSendIndication
********************************************************************************
*
* Summary:
*  This function sends HTS indication to central device 
*
* Parameters:
*  appConnHandle    : Connection handle
*  temperatureArray : Array that holds temperature data
*
* Return:
*  None
*
*******************************************************************************/
void HtsSendIndication(cy_stc_ble_conn_handle_t appConnHandle, uint8_t* temperatureArray)
{
    Cy_BLE_HTSS_SendIndication(appConnHandle, CY_BLE_HTS_TEMP_MEASURE, HTS_DATA_LEN, temperatureArray);
}
    
/* [] END OF FILE */
