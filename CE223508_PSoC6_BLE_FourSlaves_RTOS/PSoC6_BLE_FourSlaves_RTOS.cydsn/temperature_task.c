/******************************************************************************
* File Name: temperature_task.c
*
* Version: 1.0
*
* Description: This file contains the task that handles temperature sensing
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
* This file contains the task that handles temperature sensing
*******************************************************************************/
#include "temperature_task.h"
#include <math.h>
#include "ble_task.h"
#include "uart_debug.h"
#include "timers.h"

/* ADC channels used to measure reference and thermistor voltages */
#define REFERENCE_CHANNEL   (uint32_t)(0x00u)
#define THERMISTOR_CHANNEL  (uint32_t)(0x01u)

/* Reference resistor in series with the thermistor is of 10 KOhm */
#define R_REFERENCE         (float)(10000)

/* See the thermistor (NCP18XH103F03RB) data sheet for more details. */
/* Beta constant of this thermistor is 3380 Kelvin. */
#define B_CONSTANT          (float)(3380) 
/**
 * Resistance of the thermistor is 10K at 25 degrees C (from data sheet)
 * Therefore R0 = 10000 Ohm, and T0 = 298.15 Kelvin, which gives
 * R_INFINITY = R0 e^(-B_CONSTANT / T0) = 0.1192855 
 */
#define R_INFINITY          (float)(0.1192855)

/* Zero Kelvin in degree C */
#define ABSOLUTE_ZERO       (float)(-273.15)

/* Temperature scan interval */
#define SCAN_INTERVAL               pdMS_TO_TICKS(500u)
/* Idle interval used when no temperature scan required */
#define STATUS_IDLE_INTERVAL   (portMAX_DELAY)

/* Handle for the Queue that contains Temperature Task commands */
QueueHandle_t temperatureCommandQ; 

/* Timer handle for scan interval timer */
TimerHandle_t xScanTimer;

bool tempTicklessIdleReadiness = false;

/**
 * These static functions are used by the Temperature Task. These are not 
 * available outside this file. See the respective function definitions for 
 * more details 
 */
static void ADC_Isr(void);
static void ScanTimerCallback(TimerHandle_t xTimer);

/*******************************************************************************
* Function Name: void Task_Temperature(void *pvParameters)   
********************************************************************************
* Summary:
*  Task that reads temperature data from thermistor circuit  
*
* Parameters:
*  void *pvParameters : Task parameter defined during task creation (unused)                            
*
* Return:
*  None
*
*******************************************************************************/
void Task_Temperature(void* pvParameters)
{
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
        
    /* Variable that stores command received */
    temperature_command_t temperatureCommand;
        
    /* Variables that stores ADC count values */
    int16_t countThermistor, countReference;
    
    /* Variable that stores temperature */
    float temperature;
    
    /* Variable that stores thermistor resistance */
    float rThermistor;
    
    /* Remove warning for unused parameter */
    (void)pvParameters;
    
    xScanTimer = xTimerCreate("Temperature Scan Timer", STATUS_IDLE_INTERVAL, \
                 pdTRUE, NULL, ScanTimerCallback);
    xTimerStart(xScanTimer, 0u);
    
    /* Initialize the ADC  */
    ADC_StartEx(ADC_Isr);
    ADC_IRQ_Enable();
    ADC_StartConvert();
       
    for(;;)
    {
        /* Block until a BLE command has been received over temperatureCommandQ */
        rtosApiResult = xQueueReceive(temperatureCommandQ, &temperatureCommand,
                        portMAX_DELAY);
        
        /* Command has been received from temperatureCommandQ */
        if(rtosApiResult)
        {
            switch(temperatureCommand)
            {
                /*~~~~~~~~~~~~~~ Command to enable scan ~~~~~~~~~~~~~~~~~~~~~~*/
                case ENABLE_SCAN:
                {
                    xTimerChangePeriod(xScanTimer, SCAN_INTERVAL, 0u);
                    break;
                }
                /*~~~~~~~~~~~~~~ Command to stop scan ~~~~~~~~~~~~~~~~~~~~~~*/
                case DISABLE_SCAN:
                {
                    xTimerChangePeriod(xScanTimer, STATUS_IDLE_INTERVAL, 0u);
                    break;
                }
                
                /*~~~~~~~~~~~~~~ Command to start scan ~~~~~~~~~~~~~~~~~~~~~~*/
                case START_SCAN:
                {
                    tempTicklessIdleReadiness = false;
                    
                    /* Set the GPIO that drives the thermistor circuit */
                    Cy_GPIO_Set(THER_VDD_0_PORT,THER_VDD_0_NUM);
                    
                    /* Wake up the ADC and start conversion */
                    ADC_Wakeup();
                    ADC_StartConvert();
                    break;
                }
                
                /*~~~~~~~~~~~~~~ Command to process ADC data  ~~~~~~~~~~~~~~~~~*/
                case PROCESS_TEMP:
                {
                    /* Read the ADC count values */
                    countReference  = ADC_GetResult16(REFERENCE_CHANNEL);
                    countThermistor = ADC_GetResult16(THERMISTOR_CHANNEL);
           
                    /**
                     * Put the ADC to sleep so that entering low power modes 
                     * won't affect the ADC operation 
                     */
                    ADC_Sleep();
                    
                    /**
                     * Clear the GPIO that drives the thermistor circuit, to 
                     * save power. 
                     */
                    Cy_GPIO_Clr(THER_VDD_0_PORT, THER_VDD_0_NUM);
                    
                    /**
                     * Calculate the thermistor resistance and the corresponding
                     * temperature 
                     */
                    rThermistor = (R_REFERENCE*countThermistor)/countReference;    
                    temperature = (B_CONSTANT/(logf(rThermistor/R_INFINITY)))+
                                                                ABSOLUTE_ZERO;
                    
                    /* Send updated temperature value to all connected devices */
                    ble_commandAndData_t commandAndData =
                    {
                        .command = SEND_TEMP_INDICATION,
                        .data    = (void*)&temperature
                    };
                    rtosApiResult = xQueueSend(bleCommandDataQ, &commandAndData, 0);
                    /* Check if the operation has been successful */
                    if(!rtosApiResult)
                    {
                        Task_DebugPrintf("Failure! : Temperature - Failed to send BLE command",
                            rtosApiResult); 
                    }
                    
                    tempTicklessIdleReadiness = true;
                    
                    break;
                }
                
                /*~~~~~~~~~~~~~~ Unknown command ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
                default:
                {   
                    Task_DebugPrintf("Info     : Temperature - Unknown BLE command", 0u);
                    break;
                }
            }
        }
        /**
         * Task has timed out and received no commands during an interval of 
         * portMAXDELAY ticks
         */
        else
        {
            Task_DebugPrintf("Warning! : Temperature - Task Timed out ", 0u);
        }
        
    }
}

/*******************************************************************************
* Function Name: static void ADC_Isr(void)
********************************************************************************
* Summary:
*  Interrupt service routine of the Scanning SAR ADC
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void ADC_Isr(void)
{
    /* Notify the Task_Ble to restart advertisement */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    /* Variable that stored interrupt status */
    uint32_t intr_status;

    /* Read interrupt status register */
    intr_status = Cy_SAR_GetInterruptStatus(ADC_SAR__HW);
    
     /* Clear handled interrupt */
    Cy_SAR_ClearInterrupt(ADC_SAR__HW, intr_status);
    
    /* Read interrupt status register to ensure write completed due to 
       buffered writes. */
    (void)Cy_SAR_GetInterruptStatus(ADC_SAR__HW);
    
    /* Send command to process temperatue data */
    temperature_command_t command = PROCESS_TEMP;
    xQueueSendFromISR(temperatureCommandQ, &command, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken );
}

/*******************************************************************************
* Function Name: static void ScanTimerCallback(TimerHandle_t xTimer)                     
********************************************************************************
* Summary:
*  This function starts the temperature scan
*
* Parameters:
*  TimerHandle_t xTimer : Current timer value (unused)
*
* Return:
*  None
*
*******************************************************************************/
static void ScanTimerCallback(TimerHandle_t xTimer)
{
    /* Remove warning for unused parameter */
    (void)xTimer;
    
    /* Send command to start temperature scan */
    temperature_command_t command = START_SCAN;
    xQueueSend(temperatureCommandQ, &command, 0u);
}

/*******************************************************************************
* Function Name: bool Task_Temp_Tickless_Idle_Readiness(void)
********************************************************************************
* Summary:
*  Function that returns the Tickless Idle readiness of Task_Temperature
*
* Parameters:
*  None                            
*
* Return:
*  bool : Tickless Idle readiness 
*
*******************************************************************************/
bool Task_Temp_Tickless_Idle_Readiness(void)
{
    return tempTicklessIdleReadiness;
}

/* [] END OF FILE */
