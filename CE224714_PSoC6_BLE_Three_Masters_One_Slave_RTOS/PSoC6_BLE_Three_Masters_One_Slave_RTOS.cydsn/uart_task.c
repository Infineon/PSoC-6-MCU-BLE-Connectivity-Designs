/******************************************************************************
* File Name: uart_task.c
*
* Version: 1.0
*
* Description: This file contains the task that provides thread-safe UART 
*              interface.
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
/* Include Header */
#include "uart_task.h"
#include "stdio.h"
#include "stdio_user.h"
#include "ble_task.h"
#include "uart_debug.h"

#define UART_NO_DATA    (char8) CY_SCB_UART_RX_NO_DATA

/* Queue handle for debug message Queue */
QueueHandle_t debugMessageQ;

/** 
 * This static functions are used by the UART Task. This function is not available 
 * outside this file. See the respective function definition for more details. 
 */
static void ProcessUartCommands(char8 command);

/* Global variables */
static bool connectPending = false;
static bool disconnectPending = false;

/*******************************************************************************
* Function Name: void Task_Debug(void *pvParameters)
********************************************************************************
* Summary:
*  Task that prints UART messages and process user input.
*
* Parameters:
*  void *pvParameters : Task parameter defined during task creation (unused)                            
*
* Return:
*  void
*
*******************************************************************************/    
void Task_Debug(void *pvParameters)
{
    /* Variable that stores the data to be printed */
    debug_print_data_t dataToPrint;
    
   /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    /* Remove warning for unused parameter */
    (void) pvParameters;
    
    /* Variable used to store user command*/
    char8 command;
        
    /* Repeatedly running part of the task */    
    for(;;)
    {
        /* Block until a message to printed has been received over debugMessageQ */
        rtosApiResult = xQueueReceive(debugMessageQ, &dataToPrint, 0u);
        
        /* Message has been received from debugMessageQ */
        if(rtosApiResult == pdTRUE)
        {
            /* If the error code is not 0, print message string along with the
               error code (as 32-bit hexadecimal data) */
            if(dataToPrint.errorCode != 0u)
            {
                printf("%s %"PRIX32" \r\n", dataToPrint.stringPointer,
                            dataToPrint.errorCode);
            }
            /* Otherwise, print the message string only */
            else
            {
                printf("%s \r\n", dataToPrint.stringPointer);
            }
        }
        else
        {
            command = UART_Get();
            if(command != UART_NO_DATA)
            {
                 ProcessUartCommands(command);
            }
        }
    }
}


/*******************************************************************************
* Function Name: ProcessUartCommands
********************************************************************************
*
* Summary:
*   Process UART user commands
*
* Parameters:
*  char8 command
*
* Return:
*  None
*
*******************************************************************************/
void ProcessUartCommands(char8 command)
{
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    ble_command_t bleCommand;
    bool validBleCommand = false;
    
    if(connectPending)
    {
        connectPending = false;
        bleCommand.command = CONNECT;
        bleCommand.data = (uint32_t)command;
        validBleCommand  = true;
    }
    else if(disconnectPending)
    {
        disconnectPending = false;
        bleCommand.command = DISCONNECT;
        bleCommand.data = (uint32_t)command;
        validBleCommand  = true;
    }
    else
    {
        switch(command)
        {
            case 's':
            {
                bleCommand.command = START_SCAN;
                validBleCommand  = true;
                printf("\rCommand  : Start Scan\r\n");
                break;
            }
            case 'x':
            {
                bleCommand.command = STOP_SCAN;
                validBleCommand  = true;
                printf("\rCommand  : Stop Scan\r\n");
                break;
            }
            case 'a':
            {
                bleCommand.command = START_ADVERTISEMENT;
                validBleCommand  = true;
                printf("\rCommand  : Start advertisement\r\n");
                break;
            }
            
            case 'z':
            {
                bleCommand.command = STOP_ADVERTISEMENT;
                validBleCommand  = true;
                printf("\rCommand  : Stop advertisement\r\n");
                break;
            }
            case 'c':
            {
                bleCommand.command = CONNECT;
                connectPending = true;
                validBleCommand  = true;
                printf("\rCommand  : Connect\r\n");
                break;
            }
            case 'd':
            {
                bleCommand.command = DISCONNECT;
                disconnectPending = true;
                validBleCommand  = true;
                printf("\rCommand  : Disconnect\r\n");
                break;
            }
            case 'p':
            {
                bleCommand.command = DISPLAY_CONNECTED_DEVICE;
                validBleCommand  = true;
                printf("\rCommand  : Display connected device\r\n");
                break;
            }
            default:
            {
                /* unsupported command */
                break;
            }
        }
    }
    /* If valid BLE command then send it to BLE task */
    if(validBleCommand)
    {
        /* Send BLE command */
        rtosApiResult = xQueueSend(bleCommandQ, &bleCommand, 0);
        if(rtosApiResult != pdTRUE)
        {
            DebugPrintf("Failure!  : UART - Send command to BLE\r\n");
        }
    }  
}

/* [] END OF FILE */
