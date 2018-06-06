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
#include <hrss.h>


/***************************************
* Conditional Compilation Parameters
***************************************/
#define ENABLED                     (1u)
#define DISABLED                    (0u)
    
#define DEBUG_UART_ENABLED          ENABLED
    
#define PRINT_DEVICE_INFO(NUM, BD_ADRES, CONN_HANDLE, ROLE)                                                          \
                          DBG_PRINTF("%x. address: %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x bdHandle:%x attId:%x %s \r\n", NUM,\
                                                     BD_ADRES.bdAddr[5u], BD_ADRES.bdAddr[4u], BD_ADRES.bdAddr[3u],  \
                                                     BD_ADRES.bdAddr[2u], BD_ADRES.bdAddr[1u], BD_ADRES.bdAddr[0u],  \
                                                     CONN_HANDLE.bdHandle, CONN_HANDLE.attId, ROLE);

#define TIMER_TIMEOUT               (1u)
#define TIMER_SLEEP_MODE            (10u)
#define CY_BLE_MAX_SCAN_DEVICES     (9u)


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

typedef struct
{    
    /* Peer device handle */
    cy_stc_ble_conn_handle_t  connHandle;
    
    /* HRS Heart Rate Measurement characteristic data */
    uint8_t                   hrsHrmCharValue[CY_BLE_HRS_HRM_CHAR_LEN];
    uint8_t                   hrsHrmCharLen;
    
    /* New notification flag */
    bool                      isNewNotification;
    /* Notification enable flag */
    bool                      isNotificationEn;
    
} app_stc_central_info_t;

typedef struct
{    
    /* Peer device handle */
    cy_stc_ble_conn_handle_t  connHandle;
    
} app_stc_peripheral_info_t;

typedef struct
{    
    /* Client info */
    app_stc_central_info_t    central;
    
    /* Server info */
    app_stc_peripheral_info_t peripheral;
        
} app_stc_connection_info_t;
    
    
/***************************************
*      API Function Prototypes
***************************************/
int HostMain(void);
void Connect(cy_stc_ble_gap_bd_addr_t *peerBdAddr);
void Disconnect(cy_stc_ble_conn_handle_t connHandle);

app_stc_scan_device_list_t* GetAppScanDevInfoPtr(void);
app_stc_connection_info_t* GetAppConnInfoPtr(void);

/* Function Prototypes from debug.c */
void ShowValue(cy_stc_ble_gatt_value_t *value);
char HexToAscii(uint8_t value, uint8_t nibble);
void Set32ByPtr(uint8_t ptr[], uint32_t value);
void PrintStackVersion(void);
void PrintApiResult(cy_en_ble_api_result_t apiResult);
void ShowError(void);


/***************************************
* Beacon Data Types / Defines 
***************************************/
#define CY_BLE_BEACON_UUID_LEN                  (0x10)
#define CY_BLE_BEACON_DEVICE_TYPE               (0x02)

/* Beacon data offsets (general) */
#define ADDR_DATA_LEN_OFFSET                    (0u)                                        /* Data length offset */
#define SIZE_DATA_LEN                           (1u)                                        /* Size */

#define ADDR_DATA_TYPE_OFFSET                   (ADDR_DATA_LEN_OFFSET + SIZE_DATA_LEN)      /* Data type offset */
#define SIZE_DATA_TYPE                          (1u)                                        /* Size */

/* AltBeacon data offsets */
#define ALTB_ADDR_COID_OFFSET                   (ADDR_DATA_TYPE_OFFSET + SIZE_DATA_TYPE)    /* COID/MFG ID offset */
#define ALTB_SIZE_COID                          (2u)                                        /* length of COID in bytes */

#define ALTB_ADDR_BCODE_OFFSET                  (ALTB_ADDR_COID_OFFSET + ALTB_SIZE_COID)    /* AltBeacon code offset */
#define ALTB_SIZE_BCODE                         (2u)

#define ALTB_ADDR_UUID_OFFSET                   (ALTB_ADDR_BCODE_OFFSET + ALTB_SIZE_BCODE)  /* UUID offset */
#define ALTB_SIZE_UUID                          (20u)                                       /* length of UUID in bytes */

#define ALTB_ADDR_RSSI_OFFSET                   (ALTB_ADDR_UUID_OFFSET + ALTB_SIZE_UUID)    /* RSSI offset */
#define ALTB_SIZE_RSSI                          (1u)

/* Cypress Beacon data offsets */
#define CYB_ADDR_COID_OFFSET                   (ADDR_DATA_TYPE_OFFSET + SIZE_DATA_TYPE)     /* COID/MFG ID offset */
#define CYB_SIZE_COID                          (2u)                                         /* length of COID in bytes */

#define CYB_ADDR_DEV_TYPE_OFFSET               (CYB_ADDR_COID_OFFSET + CYB_SIZE_COID)       /* device type offset */
#define CYB_SIZE_DEV_TYPE                      (1u)
#define CYB_VALUE_DEV_TYPE                     (0x02u)

#define CYB_ADDR_LEN3_OFFSET                   (CYB_ADDR_DEV_TYPE_OFFSET + CYB_SIZE_DEV_TYPE)   /* device type offset */
#define CYB_SIZE_LEN3                          (1u)
#define CYB_VALUE_LEN3                         (0x15u)

#define CYB_ADDR_UUID_OFFSET                   (CYB_ADDR_LEN3_OFFSET + CYB_SIZE_LEN3)       /* UUID offset */
#define CYB_SIZE_UUID                          (16u)                                        /* length of UUID in bytes */

#define CYB_ADDR_MAJOR_OFFSET                  (CYB_ADDR_UUID_OFFSET + CYB_SIZE_UUID)       /* MAJOR offset: 25 */
#define CYB_SIZE_MAJOR                         (2u)                                         /* length of MAJOR in bytes */

#define CYB_ADDR_MINOR_OFFSET                  (CYB_ADDR_MAJOR_OFFSET + CYB_SIZE_MAJOR)     /* MINOR offset: 27 */
#define CYB_SIZE_MINOR                         (2u)                                         /* length of MAJOR in bytes */

#define CYB_ADDR_RSSI_OFFSET                   (CYB_ADDR_MINOR_OFFSET + CYB_SIZE_MINOR)     /* RSSI offset: 29 */
#define CYB_SIZE_RSSI                          (1u)                                         /* length of RSSI in bytes */


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
