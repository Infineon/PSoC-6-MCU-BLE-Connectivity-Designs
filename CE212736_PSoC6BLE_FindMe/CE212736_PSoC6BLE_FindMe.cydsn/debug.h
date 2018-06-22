/*******************************************************************************
* File Name: debug.h
*
* Version: 1.20
*
* Description:
*  Contains the function prototypes and constants available to the code example
*  for debugging purposes.
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef DEBUG_H
    
    #define DEBUG_H
    
    #include <project.h>
    #include <stdio.h>  
    #include "LED.h"
    
    #define DISABLED                    0
    #define ENABLED                     (!DISABLED)
    
    /***************************************
    * Conditional Compilation Parameters
    ***************************************/
    #define DEBUG_UART_ENABLED          DISABLED

    /***************************************
    *        External Function Prototypes
    ***************************************/
    void ShowError(void);

    /***************************************
    *        Macros
    ***************************************/
    #if (DEBUG_UART_ENABLED == ENABLED)
        
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
        
    #endif /* (DEBUG_UART_ENABLED == ENABLED) */
    
#endif

/* [] END OF FILE */
