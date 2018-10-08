/******************************************************************************
* File Name: task_uart.h
*
* Version: 1.0
*
* Description: This file contains the macros that are used for UART task
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
/* ****************************************************************************
* You should set up a serial port terminal emulator such as Tera Term, or 
* HyperTerminal with these settings to view the debug information :
*
*   Baud rate     :  115200
*   Data size     :  8-bit
*   Parity        :  None
*   Stop          :  1-bit
*   Flow Control  :  None  
*******************************************************************************/

/* Include guard */
#include "project.h"    
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Prints string */
#define Printf(...) printf(__VA_ARGS__)

/* Prints a constant string and an optional error code from a separate task */
#define Task_Printf(constString, errorCode) (SendToDebugPrintTask(	\
                                                (char*)constString,	\
                                                (uint32_t)errorCode))


/* Datatype used for debug queue */
typedef struct
{
    char* stringPointer;
    uint32_t errorCode;
}   debug_print_data_t;

/* Queue handle for debug message Queue */
extern QueueHandle_t debugMessageQ;

/* Task that performs thread safe debug message printing */
void Task_Debug(void *pvParameters);
   
/* Inline function that sends messages to the debug Queue */
void inline static SendToDebugPrintTask(char* stringPtr, uint32_t errCode)
{
    debug_print_data_t printData = {.stringPointer = stringPtr, 
                                    .errorCode     = errCode};
    xQueueSend(debugMessageQ, &printData, 0u);
}

/* [] END OF FILE */
