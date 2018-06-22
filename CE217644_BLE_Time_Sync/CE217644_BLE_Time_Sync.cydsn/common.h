/*******************************************************************************
* File Name: common.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants for the example
*  project.
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef COMMON_H
#define COMMON_H
    
#include <project.h>
#include <stdio.h>
       
/***************************************
* Conditional Compilation Parameters
***************************************/
#define ENABLED                     (1u)
#define DISABLED                    (0u)

#define DEBUG_UART_ENABLED          ENABLED
  
#define TIMER_TIMEOUT               (1u)
#define CY_BLE_CONN_INTRV_TO_MS     (5u / 4u)

    
/***************************************
*      API Function Prototypes
***************************************/
int HostMain(void);
void UpdateLedState(void);

/* Function Prototypes from debug.c */
void ShowValue(cy_stc_ble_gatt_value_t *value);
char HexToAscii(uint8_t value, uint8_t nibble);
void Set32ByPtr(uint8_t ptr[], uint32_t value);
void PrintStackVersion(void);
void PrintApiResult(cy_en_ble_api_result_t apiResult);

/***************************************
*        Macros
***************************************/
#if (DEBUG_UART_ENABLED == ENABLED)
    #define UART_DEB_NO_DATA                (char8) CY_SCB_UART_RX_NO_DATA
    #define DBG_PRINTF(...)                 (printf(__VA_ARGS__))
    #define UART_DEB_PUT_CHAR(...)           while(1UL != UART_DEB_Put(__VA_ARGS__))
    #define UART_DEB_GET_CHAR(...)          (UART_DEB_Get())
    #define UART_DEB_IS_TX_COMPLETE(...)    (UART_DEB_IsTxComplete())
    #define UART_DEB_WAIT_TX_COMPLETE(...)   while(UART_DEB_IS_TX_COMPLETE() == false);    
    #define UART_DEB_SCB_CLEAR_RX_FIFO(...) (Cy_SCB_ClearRxFifo(UART_DEB_SCB__HW))
    #define UART_START(...)                 (UART_DEB_Start(__VA_ARGS__))
#else
    #define UART_DEB_NO_DATA                (0u)
    #define DBG_PRINTF(...)
    #define UART_DEB_PUT_CHAR(...)
    #define UART_DEB_GET_CHAR(...)          (0u)
    #define UART_DEB_IS_TX_COMPLETE(...)    (true)
    #define UART_DEB_WAIT_TX_COMPLETE(...)  
    #define UART_DEB_SCB_CLEAR_RX_FIFO(...) 
    #define UART_START(...)
#endif /* (DEBUG_UART_ENABLED == ENABLED) */

/***************************************
*      External data references
***************************************/
extern cy_stc_ble_conn_handle_t     appConnHandle;

#endif /* COMMON_H */

/* [] END OF FILE */
