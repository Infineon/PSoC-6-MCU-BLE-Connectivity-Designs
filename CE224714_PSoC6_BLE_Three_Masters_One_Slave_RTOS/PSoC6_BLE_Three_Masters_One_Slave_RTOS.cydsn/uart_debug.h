/******************************************************************************
* File Name: uart_debug.c
*
* Version: 1.0
*
* Description: This file contains the macros that are used for UART debug
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
/******************************************************************************
* This file contains the macros that are used for UART based debug. Note that 
* enabling debug reduces performance and power efficiency. It also increases 
* the code size.
*
* If the debug is enabled, you should set up a serial port terminal emulator 
* such as Tera Term, or HyperTerminal with these settings to view the debug
* information :
*
*   Baud rate     :  115200
*   Data size     :  8-bit
*   Parity        :  None
*   Stop          :  1-bit
*   Flow Control  :  None  
*******************************************************************************/
#ifndef UART_DEBUG_H
#define UART_DEBUG_H
/**
 * (true) enables UART based debug and (false) disables it. Note that 
 * enabling debug reduces performance, power efficiency and increases 
 * code size. Make sure to enable the UART_DEBUG component in TopDesign 
 * schematic if debug is enabled 
 */    
#define UART_DEBUG_ENABLE    (false)

/* If UART Debug is enabled */
#if (UART_DEBUG_ENABLE)
    
#define DebugPrintf(...)    printf(__VA_ARGS__)
    
/*  RTOS thread-safe macros for printing debug information from RTOS Tasks  */
/* Prints a constant string and an optional error code from a separate task */
#define Task_DebugPrintf(constString, errorCode) (SendToDebugPrintTask(	\
                                                (char*)constString,		\
                                                (uint32_t)errorCode))
#else
#define DebugPrintf(...) 
#define Task_DebugPrintf(...) 
#endif

#endif /* UART_DEBUG_H */

/* [] END OF FILE */
