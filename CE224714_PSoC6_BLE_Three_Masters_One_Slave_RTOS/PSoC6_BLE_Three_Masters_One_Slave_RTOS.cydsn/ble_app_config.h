/******************************************************************************
* File Name: ble_app_config.h
*
* Version: 1.0
*
* Description: This file is contains macro and data structures used by BLE Task. 
*
* Related Document: CE224714_PSoC6_BLE_Three_Masters_One_Slave_RTOS.pdf
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
*
*******************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation. All rights reserved.
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
#ifndef BLE_APPLICATION_H
#define BLE_APPLICATION_H

#include "project.h"    

#define BLE_MAX_SCAN_DEVICES            (16u)
#define BLE_MAX_PERIPHERAL_CONN_COUNT   (1u)    /* Slave  */    
#define BLE_MAX_CENTRAL_CONN_COUNT      (3u)    /* Master */
#define BLE_MAX_CONN_COUNT              (BLE_MAX_PERIPHERAL_CONN_COUNT + \
                                         BLE_MAX_CENTRAL_CONN_COUNT)        

#define IS_CONNECTED(x)     (Cy_BLE_GetConnectionState(x) >= \
                             CY_BLE_CONN_STATE_CONNECTED)
#define IS_DISCONNECTED(x)  (Cy_BLE_GetConnectionState(x) == \
                             CY_BLE_CONN_STATE_DISCONNECTED)

/* Central connection information */
typedef struct
{    
    /* Peer device handle */
    cy_stc_ble_conn_handle_t   connHandle;
    
    /* Battery level data */
    uint8_t                    battaryLevel;
    
    /* New notification flag */
    uint8_t                    isNewNotification;
    
} app_stc_central_info_t;

/* Peripheral connection information */
typedef struct
{    
    /* Peer device handle */
    cy_stc_ble_conn_handle_t   connHandle;
    
} app_stc_peripheral_info_t;

/* Connection information */
typedef struct
{    
    /* Client Info */
    app_stc_central_info_t     central[BLE_MAX_CENTRAL_CONN_COUNT];
    
    /* Server Info */
    app_stc_peripheral_info_t  peripheral[BLE_MAX_PERIPHERAL_CONN_COUNT];
    
    uint8_t                    centralCnt;
    uint8_t                    peripheralCnt;
    
} app_stc_connection_info_t;

/* Scan device information */
typedef struct
{
    /* Array for store peer device address */
    cy_stc_ble_gap_bd_addr_t  address[BLE_MAX_SCAN_DEVICES];
    
    /* Number of peer device in list */
    uint32_t                  count;    
    
    /* Connection request, include number of device for connection */
    uint32_t                  connReq;   
    
    volatile bool             pauseScanProgress;
} app_stc_scan_device_list_t;

#endif /* BLE_APPLICATION_H */

/* [] END OF FILE */
