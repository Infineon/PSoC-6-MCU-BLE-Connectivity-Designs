/*******************************************************************************
* File Name: common.h
*
*  Version: 1.0
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

#ifndef COMMON_H
#define COMMON_H
    
#include <project.h>
#include <stdio.h>


/***************************************
* Conditional Compilation Parameters
***************************************/
#define ENABLED                             (1u)
#define DISABLED                            (0u)

#define DEBUG_UART_ENABLED                  (ENABLED)

/***************************************
*        API Constants
***************************************/
#define CY_BLE_CONN_INTRV_TO_MS             (5 / 4)
#define ADV_TIMER_TIMEOUT                   (1u)
#define CY_BLE_MAX_SCAN_DEVICES             (9u)  

    
/***************************************
* Data Types
***************************************/
typedef struct
{
    /* Array for store peer device address */
    cy_stc_ble_gap_bd_addr_t  address[CY_BLE_MAX_SCAN_DEVICES];
    
    /* Number of peer device in list */
    uint32_t                  count;    
    
    /* Connection request, include number of device for connection */
    uint32_t                  connReq;   
    
    volatile bool             pauseScanProgress;
} app_stc_scan_device_list_t;


/***************************************
*        Function Prototypes
***************************************/
/* Function Prototypes from host_main.c */
int HostMain(void);
void AppCallBack(uint32_t event, void * eventParam);

uint32_t GetAppState(void);
void SetAppState(uint32_t state);
app_stc_scan_device_list_t* GetAppScanDevInfoPtr(void);
cy_stc_ble_conn_handle_t* GetAppConnHandlePtr(void);

/* Function Prototypes from debug.c */
void ShowValue(cy_stc_ble_gatt_value_t *value);
char HexToAscii(uint8_t value, uint8_t nibble);
void Set32ByPtr(uint8_t ptr[], uint32_t value);
void PrintStackVersion(void);
void PrintApiResult(cy_en_ble_api_result_t apiResult);
void ShowError(void);

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

/* Application state flag bits */
#define APP_STATE_IDLE                         (0u)
#define APP_STATE_WAITING_RESPONSE             (1u << 15u)
#define APP_STATE_OPERATION_COMPLETE           (1u << 14u)
      
#define APP_STATE_READ_HRS_CCCD                (1u << 1u)
#define APP_STATE_READ_BAS_CCCD                (1u << 2u)
#define APP_STATE_ENABLING_CCCD                (1u << 3u)
#define APP_STATE_READ_HRS_BSL                 (1u << 4u)

#endif /* COMMON_H */

/* [] END OF FILE */
