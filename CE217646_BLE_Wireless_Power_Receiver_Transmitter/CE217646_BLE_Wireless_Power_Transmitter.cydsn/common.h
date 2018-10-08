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
#include <stdbool.h>
#include "BLE.h"
#include "UART_DEB.h"
#include "wptu.h"

/***************************************
* Conditional Compilation Parameters
***************************************/
#define ENABLED                         (1u)
#define DISABLED                        (0u)
    
#define DEBUG_UART_ENABLED              ENABLED
#define DEBUG_UART_EXTENDED             DISABLED

/***************************************
*           API Constants
***************************************/
#define TIMER_TIMEOUT                   (1u)                /* in sec */
#define CONNECT_TIMEOUT                 (5u)                /* in sec */
#define STATUS_TIMEOUT                  (3u)                /* in sec */
#define SLOW_CONN_INTERVAL              (250u * 4u / 5u)    /* 250 ms */

#define CY_BLE_CONN_INTRV_TO_MS         (5u / 4u)

#define PEER_DEVICE_STATE_NONE          (0u)
#define PEER_DEVICE_STATE_ADDED         (1u)
#define PEER_DEVICE_STATE_CONFIGURED    (2u)

#define INVALID_INDEX                   (0xff) 

#define CY_BLE_MAX_SCAN_DEVICES         (0x9)
    
/***************************************
*        External Function Prototypes
***************************************/
int HostMain(void);

/* Function Prototypes from debug.c */
void ShowValue(cy_stc_ble_gatt_value_t *value);
char HexToAscii(uint8_t value, uint8_t nibble);
void Set32ByPtr(uint8_t ptr[], uint32_t value);
void PrintStackVersion(void);
void PrintApiResult(cy_en_ble_api_result_t apiResult);
void ShowError(void);

/***************************************
*       Data Type
***************************************/
typedef struct
{    
    cy_stc_ble_pru_control_t     pruControl;
    cy_stc_ble_pru_static_par_t  pruStaticPar;
    cy_stc_ble_pru_dynamic_par_t pruDynamicPar;
    uint8_t                      pruState;
    uint8_t                      pruCharging;
    bool                         requestResponce;
    uint8_t                      readingDynChar;
} app_stc_central_info_t;

typedef struct
{    
    /* Client Info */
    app_stc_central_info_t       central[CY_BLE_CONFIG_CONN_COUNT];
} app_stc_connection_info_t;

typedef struct
{
    /* Array for store peer device address */
    cy_stc_ble_gap_bd_addr_t  address[CY_BLE_MAX_SCAN_DEVICES];
    
    cy_stc_ble_pru_adv_service_data_t peerAdvServData[CY_BLE_MAX_SCAN_DEVICES];
    
    /* Number of peer device in list */
    uint32_t                  count;    
    
    /* Connection request, include number of device for connection */
    uint32_t                  connReq;   
    
    volatile bool             pauseScanProgress;
} app_stc_scan_device_list_t;

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

/*******************************************************************************
* Macro: AddressCompare
********************************************************************************
*
* Summary:
*  This macro returns true if addresses are same.
*
* Parameters:
*  uint8_t* address1:  Address 1 among the two addresses to be compared
*  uint8_t* address2:  Address 2 among the two addresses to be compared

* Return:
*  uint8_t:  TRUE   (if addresses are same)
*            FALSE  (if addresses are not same)
*
*******************************************************************************/
#define AddressCompare(address1, address2) \
    (memcmp(address1, address2, CY_BLE_GAP_BD_ADDR_SIZE) == 0u)


/***************************************
* External data references
***************************************/
extern app_stc_scan_device_list_t appScanDevInfo;
extern app_stc_connection_info_t  appConnInfo;
#endif /* COMMON_H */

/* [] END OF FILE */
