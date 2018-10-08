/***************************************************************************//**
* \file debug.h
*
* \version 1.0
*
*  Contains the function prototypes and constants for the UART debugging
*  and LED status notification.
*
********************************************************************************
* Copyright 2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <stdio.h>
#include "project.h"

#define ENABLED                     (1u)
#define DISABLED                    (0u)

/***************************************
* Conditional Compilation Parameters
***************************************/
#define DEBUG_UART_ENABLED          ENABLED
#define DEBUG_LED_ENABLED           ENABLED

/***************************************
*           API Constants
***************************************/
#define LED_ON                          (0u)
#define LED_OFF                         (1u)
#define RGB_LED_MIN_VOLTAGE_MV          (2700u)
#define ADV_TIMER_TIMEOUT               (1u)

/***************************************
*        External Function Prototypes
***************************************/
void BootloaderMain(void);

#if (DEBUG_LED_ENABLED)

void InitLED(void);
void HibernateLED(void);
void BlinkLED(void);
void ConnectedLED(void);

#else

#define InitLED()
#define HibernateLED()
#define BlinkLED()
#define ConnectedLED()
    
#endif
/***************************************
*        Macros
***************************************/
#if (DEBUG_UART_ENABLED == ENABLED)
    #define DBG_PRINTF(...)                 (printf(__VA_ARGS__))
    #define UART_DEB_PUT_CHAR(ch)           while(0UL == UART_DEB_Put(ch))

    __STATIC_INLINE char8 UART_DEB_GET_CHAR(void)
    {
        uint32 rec;
        rec = UART_DEB_Get();
        return((rec == CY_SCB_UART_RX_NO_DATA) ? 0u : (char8)(rec & 0xff));
    }
    
    #define UART_DEB_GET_TX_BUFF_SIZE()  ( UART_DEB_GetNumInTxFifo() )
    #define UART_START()                 ( UART_DEB_Start() )
#else
    #define DBG_PRINTF(...)
    #define UART_DEB_PUT_CHAR(ch)
    #define UART_DEB_GET_CHAR(ch)          (0u)
    #ifndef UART_DEB_GET_TX_FIFO_SR_VALID
        #define UART_DEB_GET_TX_FIFO_SR_VALID   (0u)
    #endif
    #define UART_DEB_GET_TX_BUFF_SIZE(...)      (0u)
    #define UART_START()
#endif /* (DEBUG_UART_ENABLED == ENABLED) */

/* [] END OF FILE */
