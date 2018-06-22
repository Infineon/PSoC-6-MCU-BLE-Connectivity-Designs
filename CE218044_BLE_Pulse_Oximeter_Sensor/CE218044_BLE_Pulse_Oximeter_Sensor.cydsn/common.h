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
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef COMMON_H
#define COMMON_H
    
#include <project.h>
#include <stdio.h>
#include "math.h"

    
/***************************************
* Conditional Compilation Parameters
***************************************/
#define ENABLED                             (1u)
#define DISABLED                            (0u)
#define DEBUG_UART_ENABLED                  ENABLED
#define FULL_DEBUG_UART_ENABLED             ENABLED                           

#define CY_BLE_CONN_INTRV_TO_MS             (5 / 4)
  

/* BAS service defines */
#define BATTERY_TIMEOUT                     (30u)             /* Counts depend on connection parameters */    
    
/* Initial Time and Date definitions */
#define RTC_PDL_INITIAL_DATE_SEC            (0u)
#define RTC_PDL_INITIAL_DATE_MIN            (0u)
#define RTC_PDL_INITIAL_DATE_HOUR           (16u)
#define RTC_PDL_INITIAL_DATE_HOUR_MODE       RTC_24_HOURS
#define RTC_PDL_INITIAL_DATE_DOW             RTC_MONDAY
#define RTC_PDL_INITIAL_DATE_DOM            (7u)
#define RTC_PDL_INITIAL_DATE_MONTH          (11u)
#define RTC_PDL_INITIAL_DATE_YEAR           (16u)
#define RTC_TRYES_TO_INIT                   (500u)    
    
    
/***************************************
*      API Function Prototypes
***************************************/
/* Function Prototypes from host_main.c */
int HostMain(void);

/* Function Prototypes from debug.c */
void ShowValue(cy_stc_ble_gatt_value_t *value);
char HexToAscii(uint8_t value, uint8_t nibble);
void Set32ByPtr(uint8_t ptr[], uint32_t value);
void PrintStackVersion(void);
void PrintApiResult(cy_en_ble_api_result_t apiResult);
void IndicateError(void);

/* Function prototypes from bond.c */
void App_DisplayBondList(void);
void App_RemoveDevicesFromBondListBySW2Press(uint32_t seconds);
void App_RemoveDevicesFromBondList(void);
void App_SetRemoveBondListFlag(void);
bool App_IsRemoveBondListFlag(void);
bool App_IsDeviceInBondList(uint32_t bdHandle);
uint32_t App_GetCountOfBondedDevices(void);

/* Function prototypes from auth.c */
void App_AuthReplay(cy_stc_ble_conn_handle_t connHandle);
bool App_IsAuthReq(void);
void App_SetAuthIoCap(uint32_t value);
void App_ShowAuthError(cy_en_ble_gap_auth_failed_reason_t authErr);

/***************************************
* Data Types
***************************************/
typedef enum
{
    APP_ADV_STATE_NON_BONDED,               /* When in non-bonded mode, the design shall advertise in fast
                                                advertisement mode for 30 seconds & fall back to slow advertisement
                                                for 150 seconds after timeout.
                                                Advertising configuration: CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX */ 

    APP_ADV_STATE_BONDED_WHITELIST_ENABLE,  /* When in bonded mode, enable bonded devices whitelist for the 
                                                first 10 seconds of advertisement.
                                                Advertising configuration: CY_BLE_PERIPHERAL_CONFIGURATION_1_INDEX */ 
                                            
    

    APP_ADV_STATE_BONDED_WHITELIST_DISABLEE /* When in bonded mode and if not connected in the first 10 seconds, 
                                                disable whitelist and advertise in discoverable undirected mode for the 
                                                next 20 seconds. Switch over to 150 seconds of slow advertisement.
                                                Advertising configuration: CY_BLE_PERIPHERAL_CONFIGURATION_2_INDEX */ 

}cy_en_app_adv_state_t;

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

#endif /* COMMON_H */

/* [] END OF FILE */
