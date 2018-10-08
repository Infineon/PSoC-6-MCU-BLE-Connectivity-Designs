/******************************************************************************
* File Name: status_led.c
*
* Version: 1.0
*
* Description: This file contains the task that that controls the status LEDs
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
* This file contains the tasks that that control the status LEDs
*******************************************************************************/

/* Header file includes */
#include "status_led_task.h"
#include "uart_debug.h"
#include "uart_debug.h"
#include "task.h"  
#include "timers.h" 

/**
 * LED refresh interval of 500ms is used when continuously toggling a 
 * status LED 
 */
#define STATUS_LED_TOGGLE_INTERVAL (pdMS_TO_TICKS(500u))
/* LED refresh interval of 4s is used for blinking a status LED once */
#define STATUS_LED_BLINK_INTERVAL  (pdMS_TO_TICKS(4000u))
/* Idle interval is used for static LED states or when no LEDs are ON */
#define STATUS_LED_IDLE_INTERVAL   (portMAX_DELAY)

/* Queue handle used for commands to Task_StatusLed */
QueueHandle_t statusLedDataQ;

/* Timer handles used to control LED blink / toggle intervals */
TimerHandle_t xTimer_StatusLedRed;
TimerHandle_t xTimer_StatusLedOrange;

/**
 * Functions that start and control the timers used for LED blink / toggle
 * intervals. 
 */
static void StatusLedRedTimerStart(void);
static void StatusLedOrangeTimerStart(void);
static void StatusLedRedTimerUpdate(TickType_t period);
static void StatusLedOrangeTimerUpdate(TickType_t period);

/*******************************************************************************
* Function Name: void Task_StatusLed(void *pvParameters)
********************************************************************************
* Summary:
*  Task that controls the Orange and Red status LEDs  
*
* Parameters:
*  void *pvParameters : Task parameter defined during task creation (unused)                            
*
* Return:
*  None
*
*******************************************************************************/
void Task_StatusLed(void *pvParameters)
{ 
    /* Variable that stores the data recieved received over queue */
    status_led_data_t statusLedData;

    /* Variables used to detect changes to the LED states */
    status_led_command_t static redLedState    = LED_TURN_OFF;
    status_led_command_t static orangeLedState = LED_TURN_OFF;
    
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    /* Remove warning for unused parameter */
    (void)pvParameters ;
    
    /* Start the timers that control LED blink / toggle intervals */
    StatusLedRedTimerStart();
    StatusLedOrangeTimerStart();
    
    /* Repeatedly running part of the task */
    for(;;)
    {
        /* Block until a command has been received over statusLedDataQ */
        rtosApiResult = xQueueReceive(statusLedDataQ, &statusLedData,
                         portMAX_DELAY);
        /* Command has been received from statusLedDataQ */
        if(rtosApiResult == pdTRUE)
        {
            /* Take an action based on the command received for Orange LED */
            switch(statusLedData.orangeLed)
            {
                /* No change to the LED state */
                case LED_NO_CHANGE:
                    break;
                /* Turn ON the LED */
                case LED_TURN_ON:
                    Cy_GPIO_Clr(Pin_LED_Orange_PORT, Pin_LED_Orange_NUM);
                    /* Set the timer to idle mode */
                    StatusLedOrangeTimerUpdate(STATUS_LED_IDLE_INTERVAL);
                    orangeLedState = LED_TURN_ON;
                    break;
                /* Turn OFF the LED */    
                case LED_TURN_OFF:
                    Cy_GPIO_Set(Pin_LED_Orange_PORT, Pin_LED_Orange_NUM);
                    /* Set the timer to idle mode */
                    StatusLedOrangeTimerUpdate(STATUS_LED_IDLE_INTERVAL);
                    orangeLedState = LED_TURN_OFF;
                    break;
                /* Continuously toggle the LED */
                case LED_TOGGLE_EN:
                    /* Update the timer period to toggle interval */
                    StatusLedOrangeTimerUpdate(STATUS_LED_TOGGLE_INTERVAL);
                    orangeLedState = LED_TOGGLE_EN;
                    break;
                /* Blink the LED once */    
                case LED_BLINK_ONCE:
                    Cy_GPIO_Clr(Pin_LED_Orange_PORT, Pin_LED_Orange_NUM);
                    /* Update the timer period to blink interval */
                    StatusLedOrangeTimerUpdate(STATUS_LED_BLINK_INTERVAL);
                    orangeLedState = LED_BLINK_ONCE;
                    break;
                /* Refresh the LED based on the current state */     
                case LED_TIMER_EXPIRED:
                    if(orangeLedState == LED_TOGGLE_EN)
                    {
                        /* Toggle the LED */
                        Cy_GPIO_Inv(Pin_LED_Orange_PORT,
                                    Pin_LED_Orange_NUM);
                    }
                    else if (orangeLedState == LED_BLINK_ONCE)
                    {
                        /* Turn off the LED */
                        Cy_GPIO_Set(Pin_LED_Orange_PORT,
                                    Pin_LED_Orange_NUM);
                        /* Set the timer to idle mode */
                        StatusLedOrangeTimerUpdate(STATUS_LED_IDLE_INTERVAL);
                        orangeLedState = LED_TURN_OFF;
                    }
                    else
                    {
                        Task_DebugPrintf("Error!   : Status LED - refresh "\
                                         "command for Orange LED during invalid"\
                                         "state", 0u);   
                    }
                    break;
                /* Invalid command received */    
                default:
                    Task_DebugPrintf("Error!   : Status LED - Invalid  command"\
                                     " for Red LED received. Error Code:",
                                      statusLedData.orangeLed);
                    break;
            }
            
            /* Take an action based on the command received for Red LED */
            switch(statusLedData.redLed)
            {
                /* No change to the LED state */
                case LED_NO_CHANGE:
                    break;
                /* Turn ON the LED */
                case LED_TURN_ON:
                    Cy_GPIO_Clr(Pin_LED_Red_PORT,Pin_LED_Red_NUM);
                    /* Set the timer to idle mode */
                    StatusLedRedTimerUpdate(STATUS_LED_IDLE_INTERVAL);
                    redLedState = LED_TURN_ON;
                    break;
                /* Turn OFF the LED */        
                case LED_TURN_OFF:
                    Cy_GPIO_Set(Pin_LED_Red_PORT,Pin_LED_Red_NUM);
                    /* Set the timer to idle mode */
                    StatusLedRedTimerUpdate(STATUS_LED_IDLE_INTERVAL);
                    redLedState = LED_TURN_OFF;
                    break;
                /* Continuously toggle the LED */
                case LED_TOGGLE_EN:
                    /* Update the timer period to toggle interval */
                    StatusLedRedTimerUpdate(STATUS_LED_TOGGLE_INTERVAL);
                    redLedState = LED_TOGGLE_EN;
                    break;
                /* Blink the LED once */        
                case LED_BLINK_ONCE:
                    Cy_GPIO_Clr(Pin_LED_Red_PORT,Pin_LED_Red_NUM);
                    /* Update the timer period to blink interval */
                    StatusLedRedTimerUpdate(STATUS_LED_BLINK_INTERVAL);
                    redLedState = LED_BLINK_ONCE;
                    break;
                /* Refresh the LED based on the current state */         
                case LED_TIMER_EXPIRED:
                    if(redLedState == LED_TOGGLE_EN)
                    {
                        /* Toggle the LED */
                        Cy_GPIO_Inv(Pin_LED_Red_PORT,
                                    Pin_LED_Red_NUM);
                    }
                    else if (redLedState == LED_BLINK_ONCE)
                    {
                        /* Turn off the LED */
                        Cy_GPIO_Set(Pin_LED_Red_PORT,
                                    Pin_LED_Red_NUM);
                        /* Set the timer to idle mode */
                        StatusLedRedTimerUpdate(STATUS_LED_IDLE_INTERVAL);                        
                        redLedState = LED_TURN_OFF;
                    }
                    else
                    {
                         Task_DebugPrintf("Error!   : Status LED - refresh "\
                                          "command for Red LED during"\
                                          "invalid state",0u);   
                    }
                    break;
                /* Invalid command received */      
                default:
                    Task_DebugPrintf("Error!   : Status LED - Invalid command"
                                     "for Red LED received. Error Code:",
                                      statusLedData.orangeLed);
                    break;
            }            
        }
        /* Task has timed out and received no commands during an interval of 
           portMAXDELAY ticks */
        else
        {
            Task_DebugPrintf("Warning! : Status LED - Task Timed out ", 0u);   
        }
    }
}

/*******************************************************************************
* Function Name: static void StatusLedOrangeTimerCallback(TimerHandle_t xTimer)                          
********************************************************************************
* Summary:
*  This function is called when the Orange LED Timer expires
*
* Parameters:
*  TimerHandle_t xTimer :  Current timer value (unused)
*
* Return:
*  None
*
*******************************************************************************/
static void StatusLedOrangeTimerCallback(TimerHandle_t xTimer)
{
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    /* Remove warning for unused parameter */
    (void)xTimer;
    
    /* Send command to refresh the LED state */
    status_led_data_t ledRefreshData = 
    {   .redLed     = LED_NO_CHANGE,
        .orangeLed  = LED_TIMER_EXPIRED 
    };
    rtosApiResult = xQueueSend(statusLedDataQ, &ledRefreshData,0u);
    
    /* Check if the operation has been successful */
    if(rtosApiResult != pdTRUE)
    {
        Task_DebugPrintf("Failure! : Status LED - Sending data to Status LED "\
                         "queue",0u);      
    }
}

/*******************************************************************************
* Function Name: static void StatusLedOrangeTimerStart(void)                 
********************************************************************************
* Summary:
*  This function starts the timer that provides timing to Orange LED 
*  toggle / blink
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void StatusLedOrangeTimerStart(void)
{
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    /* Create an RTOS timer */
    xTimer_StatusLedOrange =  xTimerCreate ("Orange Status LED Timer",
                                            STATUS_LED_IDLE_INTERVAL, pdTRUE, 
                                            NULL, StatusLedOrangeTimerCallback); 
    
    /* Make sure that timer handle is valid */
    if (xTimer_StatusLedOrange != NULL)
    {
        /* Start the timer */
        rtosApiResult = xTimerStart(xTimer_StatusLedOrange, 0u);
        
        /* Check if the operation has been successful */
        if(rtosApiResult  != pdPASS)
        {
            Task_DebugPrintf("Failure! : Status LED  - Orange LED Timer "\
                             "initialization", 0u);    
        }
    }
    else
    {
        Task_DebugPrintf("Failure! : Status LED  - Orange LED Timer creation",
                          0u); 
    }  
}

/*******************************************************************************
* Function Name: static void StatusLedOrangeTimerUpdate(TickType_t period)                
********************************************************************************
* Summary:
*  This function updates the timer period per the parameter
*
* Parameters:
*  TickType_t period :  Period of the timer in ticks
*
* Return:
*  None
*
*******************************************************************************/
static void StatusLedOrangeTimerUpdate(TickType_t period)
{
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    /* Change the timer period */
    rtosApiResult = xTimerChangePeriod(xTimer_StatusLedOrange, period, 0u);
    
    /* Check if the operation has been successful */
    if(rtosApiResult != pdPASS)
    {
        Task_DebugPrintf("Failure! : Status LED - Orange LED Timer update ",
                          0u);   
    }
}

/*******************************************************************************
* Function Name: static void StatusLedRedTimerCallback(TimerHandle_t xTimer)                          
********************************************************************************
* Summary:
*  This function is called when the Red LED Timer expires
*
* Parameters:
*  TimerHandle_t xTimer :  Current timer value (unused)
*
* Return:
*  None
*
*******************************************************************************/
static void StatusLedRedTimerCallback(TimerHandle_t xTimer)
{
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    /* Remove warning for unused parameter */
    (void)xTimer;
    
    /* Send command to refresh the LED state */
    status_led_data_t ledRefreshData = 
    {   .redLed     = LED_TIMER_EXPIRED,
        .orangeLed  = LED_NO_CHANGE 
    };
    rtosApiResult = xQueueSend(statusLedDataQ, &ledRefreshData,0u);
    
    /* Check if the operation has been successful */
    if(rtosApiResult != pdTRUE)
    {
        Task_DebugPrintf("Failure! : Status LED - Sending data to Status LED "\
                         "queue",0u);    
    }
}

/*******************************************************************************
* Function Name: static void StatusLedRedTimerStart(void)                   
********************************************************************************
* Summary:
*  This function starts the timer that provides timing to Red LED toggle / blink
*
* Parameters:
*  void
*
* Return:
*  None
*
*******************************************************************************/
static void StatusLedRedTimerStart(void)
{   
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    /* Create an RTOS timer */
    xTimer_StatusLedRed    =  xTimerCreate ("Red Status LED Timer",
                                            STATUS_LED_IDLE_INTERVAL, pdTRUE,
                                            NULL, StatusLedRedTimerCallback);
    
    /* Make sure that timer handle is valid */
    if (xTimer_StatusLedRed != NULL)
    {
        /* Start the timer */
        rtosApiResult = xTimerStart(xTimer_StatusLedRed, 0u);
        
        /* Check if the operation has been successful */
        if(rtosApiResult != pdPASS)
        {
            Task_DebugPrintf("Failure! : Status LED  - Red LED Timer "\
                             "initialization", 0u);    
        }
    }
    else
    {
        Task_DebugPrintf("Failure! : Status LED  - Red LED Timer creation", 0u); 
    }
}

/*******************************************************************************
* Function Name: static void StatusLedRedTimerUpdate(TickType_t period)                 
********************************************************************************
* Summary:
*  This function updates the timer period per the parameter
*
* Parameters:
*  TickType_t period :  Period of the timer in ticks
*
* Return:
*  None
*
*******************************************************************************/
static void StatusLedRedTimerUpdate(TickType_t period)
{
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    /* Change the timer period */
    rtosApiResult = xTimerChangePeriod(xTimer_StatusLedRed, period, 0u);
    
    /* Check if the operation has been successful */
    if(rtosApiResult != pdPASS)
    {
        Task_DebugPrintf("Failure! : Status LED - Red LED Timer update ", 0u);   
    }
}

/* [] END OF FILE */
