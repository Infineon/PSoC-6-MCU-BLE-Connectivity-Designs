/*******************************************************************************
* File Name: debug.h
*
* Version: 1.00
*
* Description:
*  Contains the function prototypes and constants available to the code example
*  for debugging purposes.
*
*******************************************************************************
* Copyright (2017-2018), Cypress Semiconductor Corporation. All rights reserved.
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
/* Include guard */
#ifndef DEBUG_H
#define DEBUG_H

/* Header file includes */
#include <project.h>
#include <stdio.h>  

#define DISABLED    0
#define ENABLED     (!DISABLED)

/***************************************
* Conditional Compilation Parameters
***************************************/
#define DEBUG_UART_ENABLED      DISABLED

/***************************************
*        External Function Prototypes
***************************************/
void ShowError(void);

/***************************************
*        Macros
***************************************/
#if DEBUG_UART_ENABLED

    __STATIC_INLINE void UART_DEBUG_START(void)              
    {
        (void) Cy_SCB_UART_Init(UART_DEBUG_HW, &UART_DEBUG_config, &UART_DEBUG_context);
        Cy_SCB_UART_Enable(UART_DEBUG_HW);
    }

    #define DEBUG_PRINTF(...)               (printf(__VA_ARGS__))

    #define UART_DEBUG_PUT_CHAR(...)        while(1UL != Cy_SCB_UART_Put(UART_DEBUG_HW, __VA_ARGS__))

    __STATIC_INLINE char8 UART_DEBUG_GET_CHAR(void)
    {
        uint32 rec;
        Cy_SCB_UART_Receive(UART_DEBUG_HW, &rec, 0x01, &UART_DEBUG_context);
        return((rec == CY_SCB_UART_RX_NO_DATA) ? 0u : (char8)(rec & 0xff));
    }
    #define UART_DEBUG_GET_TX_BUFF_SIZE(...)  (Cy_SCB_GetNumInTxFifo(UART_DEBUG_SCB__HW) + Cy_SCB_GetTxSrValid(UART_DEBUG_SCB__HW))

    #define DEBUG_WAIT_UART_TX_COMPLETE()     while(UART_DEBUG_GET_TX_BUFF_SIZE() != 0);

#else
    #define UART_DEBUG_START() 

    #define DEBUG_PRINTF(...)

    #define UART_DEBUG_PUT_CHAR(...)

    #define UART_DEBUG_GET_CHAR(...)              (0u)

    #ifndef UART_DEBUG_GET_TX_FIFO_SR_VALID
        #define UART_DEBUG_GET_TX_FIFO_SR_VALID   (0u)
    #endif

    #define UART_DEBUG_GET_TX_BUFF_SIZE(...)      (0u)

    #define DEBUG_WAIT_UART_TX_COMPLETE()

#endif /* (DEBUG_UART_ENABLED ) */    
#endif /* DEBUG_H */

/* [] END OF FILE */
