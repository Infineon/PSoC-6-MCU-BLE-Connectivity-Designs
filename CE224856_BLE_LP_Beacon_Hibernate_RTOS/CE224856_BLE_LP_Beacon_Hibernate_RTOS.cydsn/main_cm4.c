/******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.00
*
* Description: This code example demonstrates a Bluetooth Low Energy (BLE) beacon 
* that goes into Hibernate mode once the specified active time has elapsed.
*
* Related Document: CE224856_BLE_Low_Power_Beacon_with_Hibernate_RTOS.pdf
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
* This code example implements the Eddystone Beacon (RTOS) code example and 
* in addition saves power by entering Hibernate after a specified interval of time. 
*
* This project broadcasts the URL frames by default, with interleaved TLM 
* frames. To change the Eddystone packet settings, see the header file
* eddystone_config.h
*
* The system enters the hibernate mode after the specified interval of time.
* The system then wakes up from hibernate mode once the RTC alarm has elapsed.
* ALternatively the system can wake up by a switch press.
* The active and hibernaet periods can be specified in "hibernate_config.h".
*
* This code example uses FreeRTOS. For documentation and API references of 
* FreeRTOS, visit : https://www.freertos.org 
*
*******************************************************************************/

/* Header file includes */
#include "project.h"
#include "limits.h"
#include "FreeRTOS.h"
#include "ble_task.h"
#include "status_led_task.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h" 

/* Priorities of user tasks in this project */
#define TASK_BLE_PRIORITY           (15u)
#define TASK_STATUS_LED_PRIORITY    (10u)

/* Stack sizes of user tasks in this project */
#define TASK_BLE_STACK_SIZE         (1024u)
#define TASK_STATUS_LED_STACK_SIZE  (configMINIMAL_STACK_SIZE)

/* Queue lengths of message queues used in this project */
#define STATUS_LED_QUEUE_LEN       (5u)

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  System entrance point. This function enables interrupts and then calls the
*  the function that sets up user tasks and then starts RTOS scheduler. 
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main()
{   
     /* Create the queues and semaphores. See the respective data-types for 
       details of queue contents */
    bleSemaphore    = xQueueCreateCountingSemaphore(ULONG_MAX,0u);
    statusLedDataQ  = xQueueCreate(STATUS_LED_QUEUE_LEN, sizeof(status_led_data_t));
 
    /* Create the user Tasks. See the respective Task definition for more
       details of these tasks */       
    xTaskCreate(Task_Ble, "BLE Task", TASK_BLE_STACK_SIZE,
                NULL, TASK_BLE_PRIORITY, NULL);
    xTaskCreate(Task_StatusLed, "Status LED Task", TASK_STATUS_LED_STACK_SIZE,
                NULL, TASK_STATUS_LED_PRIORITY, NULL);

    
    /* Start the RTOS scheduler. This function should never return */
    vTaskStartScheduler();
    
    /* Should never get here! */ 
    /* Halt the CPU if scheduler exits */
    CY_ASSERT(0);
    
    for(;;)
    {
    }	
}

/*******************************************************************************
* Function Name: void vApplicationIdleHook(void)
********************************************************************************
*
* Summary:
*  This function is called when the RTOS in idle mode
*    
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void vApplicationIdleHook(void)
{
    /* Enter sleep-mode */
    Cy_SysPm_Sleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
}

/*******************************************************************************
* Function Name: void vApplicationStackOverflowHook(TaskHandle_t *pxTask, 
                                                    signed char *pcTaskName)
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
    (void)pcTaskName;
    
    /* Print the error message with task name if debug is enabled in 
       uart_debug.h file */
    
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
    /* Halt the CPU */
    CY_ASSERT(0);
}


/* [] END OF FILE */
