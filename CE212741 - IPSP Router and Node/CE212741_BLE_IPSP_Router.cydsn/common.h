/*******************************************************************************
* File Name: common.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project.
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>
#include <stdio.h>

#define ENABLED                     (1u)
#define DISABLED                    (0u)

/***************************************
* Conditional Compilation Parameters
***************************************/
#define DEBUG_UART_ENABLED          ENABLED
#define DEBUG_UART_FULL             DISABLED
/* Add code for multicounter support */
#define WDT_CODE                    DISABLED 

/***************************************
*           API Constants
***************************************/
#define CY_BLE_MAX_ADV_DEVICES       10u

#define LED_ON                      (0u)
#define LED_OFF                     (1u)

#define LED_TIMEOUT                 (10u)              /* Сounts in hundreds of ms */

#define STATE_INIT                  (0u)
#define STATE_CONNECTING            (1u)
#define STATE_DISCONNECTED          (2u)
#define STATE_CONNECTED             (3u)

#define TIMER_TIMEOUT               (1u)

#define YES                         (1u)
#define NO                          (0u) 

/* IPSP defines */
#define LE_DATA_CREDITS_IPSP        (1000u)
/* Set the watermark to half of the total credits to be used */
#define LE_WATER_MARK_IPSP          (LE_DATA_CREDITS_IPSP/2u)

#define L2CAP_MAX_LEN               (CY_BLE_L2CAP_MTU - 2u)
#define SCAN_TIMER_TIMEOUT          (1u)              /* Сounts in s */

/***************************************
*        External Function Prototypes
***************************************/
int HostMain(void);
void UpdateLedState(void);

/* Function Prototypes from debug.c */
void ShowValue(cy_stc_ble_gatt_value_t *value);
char HexToAscii(uint8_t value, uint8_t nibble);
void Set32ByPtr(uint8_t ptr[], uint32_t value);
void PrintStackVersion(void);
void PrintApiResult(cy_en_ble_api_result_t apiResult);
void ShowError(void);

/***************************************
*        Macros
***************************************/
#if (DEBUG_UART_ENABLED == ENABLED)
    #define DBG_PRINTF(...)                 (printf(__VA_ARGS__))
    #define UART_DEB_PUT_CHAR(...)           while(1UL != UART_DEB_Put(__VA_ARGS__))
    #define UART_DEB_GET_CHAR(...)          (UART_DEB_Get())
    #define UART_DEB_IS_TX_COMPLETE(...)    (UART_DEB_IsTxComplete())
    #define UART_DEB_WAIT_TX_COMPLETE(...)   while(UART_DEB_IS_TX_COMPLETE() == 0) ;    
    #define UART_DEB_SCB_CLEAR_RX_FIFO(...) (Cy_SCB_ClearRxFifo(UART_DEB_SCB__HW))
    #define UART_START(...)                 (UART_DEB_Start(__VA_ARGS__))
#else
    #define DBG_PRINTF(...)
    #define UART_DEB_PUT_CHAR(...)
    #define UART_DEB_GET_CHAR(...)          (0u)
    #define UART_DEB_IS_TX_COMPLETE(...)    (1u)
    #define UART_DEB_WAIT_TX_COMPLETE(...)  (0u)
    #define UART_DEB_SCB_CLEAR_RX_FIFO(...) (0u)
    #define UART_START(...)
#endif /* (DEBUG_UART_ENABLED == ENABLED) */

#define UART_DEB_NO_DATA                (char8) CY_SCB_UART_RX_NO_DATA


/* Set Scanning LED  */
#define Scanning_LED_Write(value) \
                do{       \
                    Cy_GPIO_Write(Scanning_LED_0_PORT, Scanning_LED_0_NUM, value); \
                }while(0)
#define Scanning_LED_INV()     Cy_GPIO_Inv(Scanning_LED_0_PORT, Scanning_LED_0_NUM) 

/* Read SW2 pin */
#define SW2_Read()        Cy_GPIO_Read(SW2_0_PORT, SW2_0_NUM)
#define SW2_EnableInt() \
                do{       \
                    Cy_GPIO_ClearInterrupt(SW2_0_PORT, SW2_0_NUM); \
                    Cy_GPIO_SetInterruptMask(SW2_0_PORT, SW2_0_NUM, 0x01u);\
                }while(0)
#define SW2_DisableInt() \
                do{       \
                    Cy_GPIO_SetInterruptMask(SW2_0_PORT, SW2_0_NUM, 0x00u);\
                    Cy_GPIO_ClearInterrupt(SW2_0_PORT, SW2_0_NUM);\
                }while(0)

/* Leds operations */
#define EnableAllLeds() \
                do{       \
                    Scanning_LED_Write(LED_ON);\
                }while(0)

#define DisableAllLeds() \
                do{       \
                    Scanning_LED_Write(LED_OFF);\
                }while(0)

/***************************************
* External data references
***************************************/
extern cy_stc_ble_conn_handle_t appConnHandle;        

/* [] END OF FILE */
