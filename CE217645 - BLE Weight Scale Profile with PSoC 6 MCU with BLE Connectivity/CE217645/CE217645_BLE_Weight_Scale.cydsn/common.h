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
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#ifndef COMMON_H
#define COMMON_H
    
#include <project.h>
#include <stdio.h>
#include "syslib/cy_syslib.h"

/***************************************
* Conditional Compilation Parameters
***************************************/
#define ENABLED                             (1u)
#define DISABLED                            (0u)
#define DEBUG_UART_ENABLED                  ENABLED


/***************************************
*           API Constants
***************************************/
#define CY_BLE_CONN_INTRV_TO_MS             (5 / 4)

#define ADV_TIMER_TIMEOUT                   (1u)              /* Сounts in s */
#define SW2_PRESS_TIME_DEL_BOND_LIST        (0x04u)

/* Constants for buttonState */
#define BUTTON_IS_PRESSED                   (1u)
#define BUTTON_IS_NOT_PRESSED               (0u)

#define BUTTON_PRESS_DELAY                  (1u)

#define BYTE_SHIFT                          (8u)
#define TWO_BYTES_SHIFT                     (16u)
#define THREE_BYTES_SHIFT                   (24u)

#define BYTE_0                              (0u)
#define BYTE_1                              (1u)
#define BYTE_2                              (2u)
#define BYTE_3                              (3u)

#define SI_WEIGHT_RESOLUTION                (5u)
#define SI_WEIGHT_RESOLUTION_DIVIDER        (1000u)
#define SI_HEIGHT_RESOLUTION                (1u)
#define SI_HEIGHT_RESOLUTION_DIVIDER        (1000u)

/* Maximum weight of example project simulation */
#define SI_MAX_WEIGHT                       (16000u)
#define SI_MIN_WEIGHT                       (14000u)

#define IMP_WEIGHT_RESOLUTION               (1u)
#define IMP_WEIGHT_RESOLUTION_DIVIDER       (1u)
#define IMP_HEIGHT_RESOLUTION               (1u)
#define IMP_HEIGHT_RESOLUTION_DIVIDER       (1u)

#define CM_TO_M_DIVIDER                     (100u)
#define HEIGHT_METER_MODIFIER               (10u)

#define PERCENT_MODIFIER                    (100u)

/* Maximum users for this example project */
#define MAX_USERS                           (4u)


/***************************************
*        External Function Prototypes
***************************************/
int HostMain(void);
bool ButtonSW2GetStatus(void);
void ButtonSW2SetStatus(bool Status);

/* Function Prototypes from debug.c */
void PrintStackVersion(void);
void PrintApiResult(cy_en_ble_api_result_t apiResult);

/* Function prototypes from bond.c */
void App_DisplayBondList(void);
void App_RemoveDevicesFromBondListBySW2Press(uint32_t seconds);
void App_RemoveDevicesFromBondList(void);
void App_SetRemoveBondListFlag(void);
bool App_IsRemoveBondListFlag(void);
bool App_IsDeviceInBondList(uint32_t bdHandle);
uint32_t App_GetCountOfBondedDevices(void);

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

#define PACK_U16(loByte, hiByte)            ((uint16_t) (((uint16_t)loByte) | (((uint16_t) hiByte) << BYTE_SHIFT)))

#define CONVERT_TO_KILOGRAMS_INT(weight)    (((weight) * SI_WEIGHT_RESOLUTION)/SI_WEIGHT_RESOLUTION_DIVIDER)
#define CONVERT_TO_KILOGRAMS_REM(weight)    ((((weight) * SI_WEIGHT_RESOLUTION * 100u) /\
                                                SI_WEIGHT_RESOLUTION_DIVIDER) % 100u)

/***************************************
* External data references
***************************************/
extern cy_stc_ble_conn_handle_t     appConnHandle;
extern uint8_t userIndex;

#endif /* COMMON_H */

/* [] END OF FILE */
