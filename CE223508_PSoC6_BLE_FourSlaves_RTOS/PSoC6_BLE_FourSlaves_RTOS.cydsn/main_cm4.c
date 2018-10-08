/******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.0
*
* Description: This project demonstrates connectivity between the PSoC 6 BLE and 
*              CySmart BLE host Emulation tool or mobile device running the CySmart
*              mobile application, and also demonstrates four independent slaves 
*              that talks to four independent masters.
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
#include "project.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "ble_task.h"
#include "rgb_led_task.h"
#include "status_led_task.h"
#include "temperature_task.h"
#include "uart_debug.h"
#include "tickless_idle.h"

/** 
 * Priorities of user tasks in this project - spaced at intervals of 5 for 
 * the ease of further modification and addition of new tasks. 
 * Larger number indicates higher priority.
 */
#define TASK_BLE_PRIORITY           (20u)
#define TASK_RGB_LED_PRIORITY       (15u)
#define TASK_TEMPERATURE_PRIORITY   (10u)
#define TASK_STATUS_LED_PRIORITY    (5u)

/* Stack sizes of user tasks in this project */
#define TASK_BLE_STACK_SIZE         (1024u)
#define TASK_TEMPERATURE_STACK_SIZE (256u)
#define TASK_RGB_LED_STACK_SIZE     (configMINIMAL_STACK_SIZE)
#define TASK_STATUS_LED_STACK_SIZE  (configMINIMAL_STACK_SIZE)

/* Queue lengths of message queues used in this project */
#define BLE_COMMAND_QUEUE_LEN           (10u)
#define RGB_LED_QUEUE_LEN               (1u)
#define STATUS_LED_QUEUE_LEN            (5u)
#define TEMPERATURE_COMMAND_QUEUE_LEN   (5u)

/* Function that returns the Tickless Idle readiness of the system */
bool IsSystemReadyForTicklessIdle(void);

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  System entrance point. This function sets up user tasks and then starts 
*  the RTOS scheduler. 
*
* Parameters:
*  None
*
* Return:
*  int
*
*******************************************************************************/
int main()
{   
    /* Configure switch SW2 as hibernate wake up source */
    Cy_SysPm_SetHibWakeupSource(CY_SYSPM_HIBPIN1_LOW);
    
    /** 
     * Create the queues. 
     * See the respective data-types for details of queue contents. 
     */
    bleCommandDataQ = xQueueCreate(BLE_COMMAND_QUEUE_LEN,
                                        sizeof(ble_commandAndData_t));
    statusLedDataQ  = xQueueCreate(STATUS_LED_QUEUE_LEN,
                                        sizeof(status_led_data_t));
    rgbLedDataQ     = xQueueCreate(RGB_LED_QUEUE_LEN,
                                        sizeof(uint32_t));
    temperatureCommandQ = xQueueCreate(TEMPERATURE_COMMAND_QUEUE_LEN,
                                        sizeof(temperature_command_t));  
         
    /** 
     * Create the user Tasks. 
     * See the respective Task definition for more details of these tasks 
     */       
    xTaskCreate(Task_Ble, "BLE Task", TASK_BLE_STACK_SIZE,
                NULL, TASK_BLE_PRIORITY, NULL);
    xTaskCreate(Task_RgbLed, "RGB LED Task", TASK_RGB_LED_STACK_SIZE,
                NULL, TASK_RGB_LED_PRIORITY, NULL);
    xTaskCreate(Task_StatusLed, "Status LED Task", TASK_STATUS_LED_STACK_SIZE,
                NULL, TASK_STATUS_LED_PRIORITY, NULL);
    xTaskCreate(Task_Temperature, "Temperature Task", TASK_TEMPERATURE_STACK_SIZE,
                NULL, TASK_TEMPERATURE_PRIORITY, NULL);
    
    /** 
     * Initialize thread-safe debug message printing. 
     * See uart_debug.h header file to enable / disable this feature 
     */
    Task_DebugInit();
    
    /* Initialize Tickless Idle mode */                    
    TicklessIdleInit(IsSystemReadyForTicklessIdle);
    
    /* Start the RTOS scheduler. This function should never return */
    vTaskStartScheduler();
    
    /* Should never get here! */ 
    DebugPrintf("Error!   : RTOS - scheduler crashed \r\n");
    
    /* Halt the CPU if scheduler exits */
    CY_ASSERT(0);
    
    for(;;)
    {
    }	
}


/*******************************************************************************
* Function Name: bool IsSystemReadyForTicklessIdle(void)
********************************************************************************
*
* Summary:
*  This function returns the Tickless Idle readiness of the system.
*    
* Parameters:
*  None
*
* Return:
*  bool: Tickless Idle readiness of the system.
*
*******************************************************************************/
bool IsSystemReadyForTicklessIdle(void)
{
    return  ((Task_Ble_Tickless_Idle_Readiness())      &&
             (Task_RgbLed_Tickless_Idle_Readiness())   &&
             (Task_Temp_Tickless_Idle_Readiness())     &&
             (Task_Debug_Tickless_Idle_Readiness()));
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
    DebugPrintf("Error!   : RTOS - stack overflow in %s \r\n", pcTaskName);
    
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
    DebugPrintf("Error!   : RTOS - Memory allocation failed \r\n");
    
    /* Halt the CPU */
    CY_ASSERT(0);
}

/* [] END OF FILE */
