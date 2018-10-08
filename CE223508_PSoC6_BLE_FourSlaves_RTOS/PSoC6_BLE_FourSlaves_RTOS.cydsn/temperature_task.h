/******************************************************************************
* File Name: temperature_task.h
*
* Version: 1.0
*
* Description: This file is the public interface of temperature_task.h source file
*
* Related Document: CE223508_PSoC6_BLE_FourSlaves_RTOS.pdf
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
*
*******************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation. All rights reserved.
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
/******************************************************************************
* This file contains the declaration of tasks and data-types used for temperature
* scan
*******************************************************************************/
#ifndef TEMPERATURE_TASK_H
#define TEMPERATURE_TASK_H

#include "project.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
    
/**
 * Declaration of Health Thermometer Service (HTS) specific macros.GATT
 * specifications of the Health Thermometer  service can be found at:
 * https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.service.health_thermometer.xml 
 */
/* Total size of the Health Thermometer Service characteristic array */
#define HTS_CHARACTERISTIC_SIZE     (uint8_t) (5u)
/* Size of the temperature value in the HTS characteristic array */
#define HTS_TEMPERATURE_DATA_SIZE   (uint8_t) (4u)
/* Index of the temperature value in the HTS characteristic array */
#define HTS_TEMPERATURE_DATA_INDEX  (uint8_t) (1u)

/**
 * Macros used to convert from IEEE-754 single precision floating 
 * point format to IEEE-11073 FLOAT with two decimal digits of precision 
 */
#define IEEE_11073_MANTISSA_SCALER  (uint8_t) (100u)
#define IEEE_11073_EXPONENT_VALUE   (int8_t)  (-2)
#define IEEE_11073_EXPONENT_INDEX   (uint8_t) (3u)    

/**
 * Data-type used to store temperature as an IEEE-11073 FLOAT value as well as
 *  access it as an array of bytes for BLE operations 
 */
typedef union 
{
    int32_t temeratureValue;
    int8_t  temperatureArray[HTS_TEMPERATURE_DATA_SIZE];
} temperature_data_t;

/* List if commands */
typedef enum 
{
    ENABLE_SCAN,
    DISABLE_SCAN,
    START_SCAN,
    PROCESS_TEMP,
} temperature_command_t;

/* Handle for the Queue that contains Temperature Task commands */
extern QueueHandle_t temperatureCommandQ;

/* Task_Temperature takes care of the BLE thermometer module in this code example */
void Task_Temperature(void* pvParameters);

/* Function that returns the Tickless Idle readiness of Task_Temperature */
bool Task_Temp_Tickless_Idle_Readiness(void);

#endif /* TEMPERATURE_TASK_H */

/* [] END OF FILE */
