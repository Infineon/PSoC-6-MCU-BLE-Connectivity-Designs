/******************************************************************************
* File Name: rgb_led_task.h
*
* Version: 1.0
*
* Description: This file is the public interface of rgb_led.c source file 
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
* This file contains the declaration of tasks and functions used for RGB LED
* color and intensity control
*******************************************************************************/

/* Include guard */
#ifndef RGB_LED_TASK_H
#define RGB_LED_TASK_H

/* Header file includes */
#include "project.h"
#include "FreeRTOS.h"
#include "queue.h"    

/* Handle for the Queue that contains RGB LED data */    
extern QueueHandle_t rgbLedDataQ;

/* Task_RgbLed updates the RGB LED color and intensity */
void Task_RgbLed(void *pvParameters);

/* Function that returns the Tickless Idle readiness of Task_RgbLed */
bool Task_RgbLed_Tickless_Idle_Readiness(void);

#endif /* RGB_LED_TASK_H */
/* [] END OF FILE */
