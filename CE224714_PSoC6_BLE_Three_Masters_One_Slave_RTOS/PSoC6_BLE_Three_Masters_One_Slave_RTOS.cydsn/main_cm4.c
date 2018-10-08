/******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.0
*
* Description: This project demonstrates how to configure the PSoC® 6 MCU with
*              Bluetooth Low Energy (BLE) Connectivity device in simultaneous 
*              Multiple Master and Single Slave modes of operation. The Multi
*              Master Single Slave project utilizes 3 BLE Central connections
*              and 1 Peripheral connection. The Central is configured as a BAS 
*              GATT client to communicate with a peer BAS GATT Server.
*
* Related Document: CE224714_PSoC6_BLE_Three_Masters_One_Slave_RTOS.pdf
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
#include "project.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "ble_task.h"
#include "uart_task.h"
#include "status_led_task.h"

/** 
 * Priorities of user tasks in this project - spaced at intervals of 5 for 
 * the ease of further modification and addition of new tasks. 
 * Larger number indicates higher priority.
 */
#define TASK_BLE_PRIORITY           (10)
#define TASK_STATUS_LED_PRIORITY    (5u)
#define TASK_UART_PRIORITY          (tskIDLE_PRIORITY + 1u)

/* Stack sizes of user tasks in this project */
#define TASK_BLE_STACK_SIZE         (1024u)
#define TASK_STATUS_LED_STACK_SIZE  (configMINIMAL_STACK_SIZE)
#define TASK_UART_STACK_SIZE        (configMINIMAL_STACK_SIZE)

/* Queue lengths of message queues used in this project */
#define BLE_COMMAND_QUEUE_LEN           (10u)
#define STATUS_LED_COMMAND_QUEUE_LEN    (1u)
#define DEBUG_QUEUE_LEN                 (32u)

int main(void)
{
    __enable_irq(); /* Enable global interrupts. */
    
    /* Create queues */
    bleCommandQ     = xQueueCreate(BLE_COMMAND_QUEUE_LEN, sizeof(ble_command_t));
    debugMessageQ   = xQueueCreate(DEBUG_QUEUE_LEN, sizeof(debug_print_data_t));
    statusLedDataQ  = xQueueCreate(STATUS_LED_COMMAND_QUEUE_LEN, sizeof(status_led_data_t));
    
    /** 
     * Create the user Tasks. 
     * See the respective Task definition for more details of these tasks 
     */ 
    xTaskCreate(Task_Ble, "BLE Task", TASK_BLE_STACK_SIZE,
                NULL, TASK_BLE_PRIORITY, NULL);
    xTaskCreate(Task_Debug, "UART Task", TASK_UART_STACK_SIZE, NULL,
                TASK_UART_PRIORITY, NULL);
    xTaskCreate(Task_StatusLed, "Status LED Task", TASK_STATUS_LED_STACK_SIZE,
                NULL, TASK_STATUS_LED_PRIORITY, NULL);
    
    /* Start the RTOS scheduler. This function should never return */
    vTaskStartScheduler();
    
    /* Should never get here! */ 
    printf("Error!   : RTOS - scheduler crashed \r\n");
    
    /* Halt the CPU if scheduler exits */
    CY_ASSERT(0);

    for(;;)
    {
        /* Place your application code here. */
    }
}

/*******************************************************************************
* Function Name: void vApplicationStackOverflowHook(TaskHandle_t *pxTask, 
*                                                   signed char *pcTaskName)
********************************************************************************
*
* Summary:
*  This function is called when a stack overflow has been detected by the RTOS
*    
* Parameters:
*  TaskHandle_t  : Handle to the task
*  signed char   : Name of the task
*
* Return:
*  None
*
*******************************************************************************/
void vApplicationStackOverflowHook(TaskHandle_t *pxTask, 
                                   signed char *pcTaskName)
{
    /* Remove warning for unused parameters */
    (void)pxTask;
    
    /** 
     * Print the error message with task name if debug is enabled in 
     * uart_debug.h file 
     */
    printf("Error!   : RTOS - stack overflow in %s \r\n", pcTaskName);
    
    /* Halt the CPU */
    CY_ASSERT(0);
}

/*******************************************************************************
* Function Name: void vApplicationMallocFailedHook(void)
********************************************************************************
*
* Summary:
*  This function is called when a memory allocation operation by the RTOS
*  has failed
*    
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void vApplicationMallocFailedHook(void)
{
    /* Print the error message if debug is enabled in uart_debug.h file */
    printf("Error!   : RTOS - Memory allocation failed \r\n");
    
    /* Halt the CPU */
    CY_ASSERT(0);
}

/* [] END OF FILE */
