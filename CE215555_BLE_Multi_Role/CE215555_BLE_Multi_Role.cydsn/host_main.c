/*******************************************************************************
* File Name: host_main.c
*
* Version 1.0
*
* Description:
*  This code example demonstrates capability of PSoC 6 BLE device to be in 
*  all the GAP roles (Central, Peripheral, Observer, and Broadcaster).
*
* Note:
*  Cypress’s Energy harvesting Beacon format
*  Refer to: 002-00297, section 8.5 "BLE Beacon Format",
*  http://www.cypress.com/file/187321/download
*
*  AltBeacon format 
*  Refer to: section "AltBeacon Protocol Format" of
*  https://github.com/AltBeacon/spec
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
*  S6SAE101A00SA1002 Solar-Powered IoT Device Kit - operates as "BLE Beacon"
*
******************************************************************************
* Copyright (2017), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the
* materials described herein. Cypress does not assume any liability arising out
* of the application or use of any product or circuit described herein. Cypress
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*****************************************************************************/

#include <project.h>
#include "common.h"
#include "user_interface.h"
#include "hrss.h"
#include "bas.h"

/* Global Variables */
static app_stc_connection_info_t    appConnInfo;
static app_stc_scan_device_list_t   appScanDevInfo = { .count = 0u };
 
static volatile uint32_t            sleepTimer    = 0u;
static volatile uint32_t            mainTimer     = 1u;

/* Private Function Prototypes */
static void ObserveBeacons(cy_stc_ble_gapc_adv_report_param_t *eventParam);
static uint32_t CheckAdvPacketForServiceUuid(cy_stc_ble_gapc_adv_report_param_t *eventParam, uint16_t uuid);
static uint32_t IsDeviceInBondList(uint32_t bdHandle);


/*******************************************************************************
* Function Name: AppCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE Component.
*
* Parameters:
*  event      - the event code
*  eventParam - the event parameters
*
*******************************************************************************/
void AppCallBack(uint32_t event, void* eventParam)
{
    cy_en_ble_api_result_t apiResult;
    uint16_t    cccdValue;
    uint32_t    i;
    uint32_t    discIdx;
        
    static cy_stc_ble_gap_sec_key_info_t keyInfo =
    {
        .localKeysFlag    = CY_BLE_GAP_SMP_INIT_ENC_KEY_DIST | 
                            CY_BLE_GAP_SMP_INIT_IRK_KEY_DIST | 
                            CY_BLE_GAP_SMP_INIT_CSRK_KEY_DIST,
        .exchangeKeysFlag = CY_BLE_GAP_SMP_INIT_ENC_KEY_DIST | 
                            CY_BLE_GAP_SMP_INIT_IRK_KEY_DIST | 
                            CY_BLE_GAP_SMP_INIT_CSRK_KEY_DIST |
                            CY_BLE_GAP_SMP_RESP_ENC_KEY_DIST |
                            CY_BLE_GAP_SMP_RESP_IRK_KEY_DIST |
                            CY_BLE_GAP_SMP_RESP_CSRK_KEY_DIST,
    };
    
    switch(event)
    {
        case CY_BLE_EVT_STACK_ON:
            DBG_PRINTF("CY_BLE_EVT_STACK_ON, StartAdvertisement \r\n");
            
            /* Enter into discoverable mode so that remote can find it. */
            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAPP_StartAdvertisement API Error: 0x%x \r\n", apiResult);
            }
                                    
            /* Generates the security keys */
            apiResult = Cy_BLE_GAP_GenerateKeys(&keyInfo);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAP_GenerateKeys API Error: 0x%x \r\n", apiResult);
            } 
            
            /* Print the operation menu */
            PrintMenu();
            break;

        case CY_BLE_EVT_TIMEOUT:
            DBG_PRINTF("CY_BLE_EVT_TIMEOUT \r\n");
            break;
            
        case CY_BLE_EVT_HARDWARE_ERROR:    /* This event indicates that some internal HW error has occurred. */
            DBG_PRINTF("CY_BLE_EVT_HARDWARE_ERROR \r\n");
            break;
            
        case CY_BLE_EVT_STACK_BUSY_STATUS:
            DBG_PRINTF("CY_BLE_EVT_STACK_BUSY_STATUS: %x\r\n", *(uint8_t *)eventParam);
            break;
            
        case CY_BLE_EVT_SET_TX_PWR_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_SET_TX_PWR_COMPLETE \r\n");
            break;
            
        case CY_BLE_EVT_LE_SET_EVENT_MASK_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_LE_SET_EVENT_MASK_COMPLETE \r\n");
            break;
            
        case CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE \r\n");
            
            /* Reads the BD device address from BLE Controller's memory */
            apiResult = Cy_BLE_GAP_GetBdAddress();
            if(apiResult != CY_BLE_SUCCESS)
            {   
                DBG_PRINTF("Cy_BLE_GAP_GetBdAddress API Error: 0x%x \r\n", apiResult);
            }
            break;
            
        case CY_BLE_EVT_GET_DEVICE_ADDR_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GET_DEVICE_ADDR_COMPLETE: ");
            for(i = CY_BLE_GAP_BD_ADDR_SIZE; i > 0u; i--)
            {
                DBG_PRINTF("%2.2x", ((cy_stc_ble_bd_addrs_t *)
                                    ((cy_stc_ble_events_param_generic_t *)eventParam)->eventParams)->publicBdAddr[i-1]);
            }
            DBG_PRINTF("\r\n");
            break;
            
        case CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE \r\n");
            DBG_PRINTF("Hibernate \r\n");
            UpdateLedState();
            UART_DEB_WAIT_TX_COMPLETE();
            /* Hibernate */
            Cy_SysPm_Hibernate();       
            break;      
            
        /**********************************************************
        *                       GAP Events
        ***********************************************************/

        case CY_BLE_EVT_GAP_AUTH_REQ:
            /* This event is received by Peripheral and Central devices. When it is received by a peripheral, 
             * that peripheral must Call Cy_BLE_GAPP_AuthReqReply() to reply to the authentication request
             * from Central. */
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_REQ: bdHandle=%x, security=%x, bonding=%x, ekeySize=%x, err=%x \r\n", 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bdHandle, (*(cy_stc_ble_gap_auth_info_t *)eventParam).security, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bonding, (*(cy_stc_ble_gap_auth_info_t *)eventParam).ekeySize, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).authErr);
            
            if(cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].security == 
                (CY_BLE_GAP_SEC_MODE_1 | CY_BLE_GAP_SEC_LEVEL_1))
            {
                cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].authErr = 
                    CY_BLE_GAP_AUTH_ERROR_PAIRING_NOT_SUPPORTED;
            }    
            
            cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].bdHandle = 
                ((cy_stc_ble_gap_auth_info_t *)eventParam)->bdHandle;

            /* Pass security information for authentication in reply to an authentication request 
             * from the master device */
            apiResult = Cy_BLE_GAPP_AuthReqReply(&cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);            
            if(apiResult != CY_BLE_SUCCESS)
            {
                Cy_BLE_GAP_RemoveOldestDeviceFromBondedList();
                apiResult = Cy_BLE_GAPP_AuthReqReply(&cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);            
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAPP_AuthReqReply API Error: 0x%x \r\n", apiResult);
                }
            }
            break;

        case CY_BLE_EVT_GAP_AUTH_FAILED:
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_FAILED: bdHandle=%x, authErr=%x\r\n", 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bdHandle, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).authErr);
            break;

        case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_CONNECTED: connIntv = %d ms \r\n", 
                       ((cy_stc_ble_gap_connected_param_t *)eventParam)->connIntv * 5u /4u); /* in milliseconds / 1.25ms */

            if(((cy_stc_ble_gap_connected_param_t *)eventParam)->status == 0u)
            {
                /* Set security keys for new device which is not already bonded */
                if(IsDeviceInBondList((*(cy_stc_ble_gap_connected_param_t *)eventParam).bdHandle) == 0u)
                {
                    keyInfo.SecKeyParam.bdHandle = (*(cy_stc_ble_gap_connected_param_t *)eventParam).bdHandle;
                    apiResult = Cy_BLE_GAP_SetSecurityKeys(&keyInfo);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_GAP_SetSecurityKeys API Error: 0x%x \r\n", apiResult);
                    }
                }  
                            
                /* Check if connected as Central or Peripheral */
                if(((cy_stc_ble_gap_connected_param_t *)eventParam)->role == CY_BLE_GAP_LL_ROLE_SLAVE)
                {
                    /* Connected as Peripheral (slave role) */    
                    appConnInfo.peripheral.connHandle = 
                        Cy_BLE_GetConnHandleByBdHandle(((cy_stc_ble_gap_connected_param_t *)eventParam)->bdHandle);  
                }
                else
                {
                    /* Connected as Central (master role) */    
                    appConnInfo.central.connHandle = 
                        Cy_BLE_GetConnHandleByBdHandle(((cy_stc_ble_gap_connected_param_t *)eventParam)->bdHandle);
                }
                            
                cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].bdHandle = 
                    ((cy_stc_ble_gap_connected_param_t *)eventParam)->bdHandle;
                
                apiResult = Cy_BLE_GAP_AuthReq(&cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAP_AuthReq API Error: 0x%x \r\n", apiResult);
                } 
            }
            break;
        
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_DISCONNECTED: bdHandle=%x, reason=%x, status=%x\r\n",
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).bdHandle, 
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).reason, 
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).status);
            break;

        case CY_BLE_EVT_GAP_KEYS_GEN_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GAP_KEYS_GEN_COMPLETE \r\n");
            keyInfo.SecKeyParam = (*(cy_stc_ble_gap_sec_key_param_t *)eventParam);
            Cy_BLE_GAP_SetIdAddress(&cy_ble_deviceAddress);
            break;
            
        case CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GAPC_CONNECTION_UPDATE_COMPLETE \r\n");
            break;

        case CY_BLE_EVT_GAP_AUTH_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_COMPLETE: security:%x, bonding:%x, ekeySize:%x, authErr %x \r\n",
                        ((cy_stc_ble_gap_auth_info_t *)eventParam)->security,
                        ((cy_stc_ble_gap_auth_info_t *)eventParam)->bonding, 
                        ((cy_stc_ble_gap_auth_info_t *)eventParam)->ekeySize, 
                        ((cy_stc_ble_gap_auth_info_t *)eventParam)->authErr);
            
            /* Start Discovery only for connection when device in Central role */
            if(appConnInfo.central.connHandle.bdHandle == ((cy_stc_ble_gap_auth_info_t *)eventParam)->bdHandle)
            {         
                DBG_PRINTF("\r\nCy_BLE_GATTC_StartDiscovery \r\n");
                apiResult = Cy_BLE_GATTC_StartDiscovery(appConnInfo.central.connHandle);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("StartDiscovery API Error: 0x%x \r\n", apiResult);
                }

            }
            break;
            
        case CY_BLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
            DBG_PRINTF("CY_BLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT \r\n");
            break;
            
        case CY_BLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO:
            DBG_PRINTF("CY_BLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO:" 
                       " bdHandle=%x, security=%x, bonding=%x, ekeySize=%x, err=%x \r\n", 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bdHandle, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).security, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bonding, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).ekeySize, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).authErr);
            break;
            
        case CY_BLE_EVT_GAP_ENCRYPT_CHANGE:
            DBG_PRINTF("CY_BLE_EVT_GAP_ENCRYPT_CHANGE: %d \r\n", *(uint8_t *)eventParam);
            break;
            
        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            DBG_PRINTF("CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP, state: %x\r\n", Cy_BLE_GetAdvertisementState());
            break;
            
        case CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
            {
                cy_stc_ble_gapc_adv_report_param_t  *advReport = (cy_stc_ble_gapc_adv_report_param_t *)eventParam;
                
                bool printHeader  = false;
                bool isNewAddress = true;
                
                if((advReport->eventType == CY_BLE_GAPC_NON_CONN_UNDIRECTED_ADV) &&
                   (cy_ble_scanIndex == CY_BLE_OBSERVER_CONFIGURATION_0_INDEX))
                {
                    ObserveBeacons(advReport);
                }
                
                if((appScanDevInfo.pauseScanProgress == false) && 
                   (advReport->eventType == CY_BLE_GAPC_CONN_UNDIRECTED_ADV) &&
                   (cy_ble_scanIndex != CY_BLE_OBSERVER_CONFIGURATION_0_INDEX)) /* skip UNDIRECTED_ADV in the observer mode */
                {
                    /* Filter and add to the connect list only nodes that advertise HRS in ADV payload */
                    if((CheckAdvPacketForServiceUuid(advReport, CY_BLE_UUID_HEART_RATE_SERVICE) == true) &&
                       (appScanDevInfo.count < CY_BLE_MAX_SCAN_DEVICES))
                    {
                        /* Detected devices are stored in peerAddr[] array. User can review this list 
                         * by pressing 'c' button and selecting a device for connection 
                         */
                        for(i = 0u; i < CY_BLE_MAX_SCAN_DEVICES; i++)
                        {
                            if(!(memcmp(&appScanDevInfo.address[i].bdAddr, advReport->peerBdAddr, CY_BLE_GAP_BD_ADDR_SIZE)))
                            {
                                isNewAddress = false;
                            }
                        }
                        
                        if(isNewAddress == true)
                        {
                            memcpy(&appScanDevInfo.address[appScanDevInfo.count].bdAddr, advReport->peerBdAddr,
                                                                                 CY_BLE_GAP_BD_ADDR_SIZE); 
                            appScanDevInfo.address[appScanDevInfo.count].type = advReport->peerAddrType;
                            appScanDevInfo.count++;
                            printHeader = true;
                        }
                    }
                    
                    /* Display scanned device ADV data */
                    if((printHeader == true) && (isNewAddress == true))
                    {
                        DBG_PRINTF("\r\n-------------------------------------------------------------------------\r\n");
                        DBG_PRINTF("uuid: HEART RATE SERVICE - YES, added to the connect list  \r\n");
                    } 
                    DBG_PRINTF("ADV type: 0x%x address: ", advReport->eventType );
                    for(i = CY_BLE_GAP_BD_ADDR_SIZE; i > 0u; i--)
                    {
                        DBG_PRINTF("%2.2x", advReport->peerBdAddr[i-1]);
                    }
                    DBG_PRINTF(", rssi - %d dBm, data - ", advReport->rssi);
                    for( i = 0; i < advReport->dataLen; i++)
                    {
                        DBG_PRINTF("%2.2x ", advReport->data[i]);
                    }
                    DBG_PRINTF("\r\n");
                    if((printHeader == true) && (isNewAddress == true))
                    {            
                        DBG_PRINTF("-------------------------------------------------------------------------\r\n\r\n");
                    }  
                }  
            }
            break; 
        
        case CY_BLE_EVT_GAPC_SCAN_START_STOP:
            DBG_PRINTF("CY_BLE_EVT_GAPC_SCAN_START_STOP\r\n");
            if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_STOPPED)
            {
                DBG_PRINTF("Scan complete \r\n");
                if(appScanDevInfo.connReq)
                {
                    Connect(&appScanDevInfo.address[appScanDevInfo.connReq - 1u]);
                    appScanDevInfo.connReq = 0u;
                }
            }
            else
            {
                DBG_PRINTF("GAPC_START_SCANNING\r\n");
            }
            break; 
            
        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CY_BLE_EVT_GATTC_ERROR_RSP:
            DBG_PRINTF("opCode: %x,  errorCode: %x,\r\n", ((cy_stc_ble_gatt_err_param_t *)eventParam)->errInfo.opCode,
                                                          ((cy_stc_ble_gatt_err_param_t *)eventParam)->errInfo.errorCode);
            break;

        case CY_BLE_EVT_GATT_CONNECT_IND:
            DBG_PRINTF("CY_BLE_EVT_GATT_CONNECT_IND: attId %x, bdHandle %x \r\n", 
                      ((cy_stc_ble_conn_handle_t *)eventParam)->attId, ((cy_stc_ble_conn_handle_t *)eventParam)->bdHandle);
            
            /* Initializes CCCD for the BLE service */ 
            BasInitCccd();
            break;

        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            DBG_PRINTF("CY_BLE_EVT_GATT_DISCONNECT_IND: attId=%x, bdHandle=%x \r\n", 
                (*(cy_stc_ble_conn_handle_t *)eventParam).attId, 
                (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle);
            
            if(appConnInfo.central.connHandle.bdHandle == (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle)
            {
                DBG_PRINTF("Disconnect central \r\n");
                appConnInfo.central.connHandle.bdHandle = CY_BLE_INVALID_CONN_HANDLE_VALUE;
                appConnInfo.central.connHandle.attId    = CY_BLE_INVALID_CONN_HANDLE_VALUE;
                
                if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_STOPPED)
                {
                    apiResult = Cy_BLE_GAPC_StartScan(CY_BLE_SCANNING_FAST, CY_BLE_CENTRAL_CONFIGURATION_0_INDEX);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_GAPC_StartScan API Error: 0x%x \r\n", apiResult);
                    }
                    else
                    {
                        DBG_PRINTF("Cy_BLE_GAPC_StartScan API Success \r\n");
                    }
                }
            }
            
            if(appConnInfo.peripheral.connHandle.bdHandle == (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle)
            {
                DBG_PRINTF("Disconnect peripheral \r\n");
                appConnInfo.peripheral.connHandle.bdHandle = CY_BLE_INVALID_CONN_HANDLE_VALUE;
                appConnInfo.peripheral.connHandle.attId    = CY_BLE_INVALID_CONN_HANDLE_VALUE;
                
                if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED)
                {
                    /* Put the device into discoverable mode so that remote can find it. */
                    apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_GAPP_StartAdvertisement API Error: 0x%x \r\n", apiResult);
                    }
                } 
            }
            
            break;
            
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            DBG_PRINTF("CY_BLE_EVT_GATT_WRITE_REQ: %x = ",
                       ((cy_stc_ble_gatt_write_param_t *)eventParam)->handleValPair.attrHandle);
            for(i = 0; i < ((cy_stc_ble_gatt_write_param_t *)eventParam)->handleValPair.value.len; i++)
            {
                DBG_PRINTF("%2.2x ", ((cy_stc_ble_gatt_write_param_t *)eventParam)->handleValPair.value.val[i]);
            }
            DBG_PRINTF("\r\n");
            break;

        case CY_BLE_EVT_GATTS_XCNHG_MTU_REQ:
        { 
            cy_stc_ble_gatt_xchg_mtu_param_t mtu = 
            {
                .connHandle = ((cy_stc_ble_gatt_xchg_mtu_param_t *)eventParam)->connHandle
            };
            Cy_BLE_GATT_GetMtuSize(&mtu);
            DBG_PRINTF("CY_BLE_EVT_GATTS_XCNHG_MTU_REQ %x, %x, final mtu= %d \r\n",  
                        mtu.connHandle.attId, mtu.connHandle.bdHandle, mtu.mtu);
        }
        break;

        case CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF:
            DBG_PRINTF("CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF \r\n");
            break;

        case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
            /* Triggered on a server side when the client sends a read request and when
            * the characteristic has CY_BLE_GATT_DB_ATTR_CHAR_VAL_RD_EVENT property set.
            * This event could be ignored by the application unless it needs to response
            * by error response which needs to be set in gattErrorCode field of
            * the event parameter. */
            DBG_PRINTF("CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ: handle: %x \r\n", 
                ((cy_stc_ble_gatts_char_val_read_req_t *)eventParam)->attrHandle);
            break;

        case CY_BLE_EVT_GATTC_DISCOVERY_COMPLETE:
            DBG_PRINTF("Discovery complete: attId=%x, bdHandle=%x \r\n", 
                (*(cy_stc_ble_conn_handle_t *)eventParam).attId, 
                (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle);
            
            discIdx = Cy_BLE_GetDiscoveryIdx(*(cy_stc_ble_conn_handle_t *)eventParam);
            
            for(i = 0u; i < CY_BLE_SRVI_COUNT; i++)
            {
                DBG_PRINTF("  service with UUID 0x%x has range from 0x%x to 0x%x\r\n", 
                                                                       cy_ble_serverInfo[discIdx][i].uuid,
                                                                       cy_ble_serverInfo[discIdx][i].range.startHandle,
                                                                       cy_ble_serverInfo[discIdx][i].range.endHandle);
            }

            /* Enable notifications for connected HRS device */
            DBG_PRINTF("\r\nEnable notifications \r\n");
            cccdValue = CY_BLE_CCCD_NOTIFICATION;
            apiResult = Cy_BLE_HRSC_SetCharacteristicDescriptor(appConnInfo.central.connHandle, CY_BLE_HRS_HRM,
                                                                CY_BLE_HRS_HRM_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccdValue);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_HRSC_SetCharacteristicDescriptor() failed. Error code: %d \r\n\r\n", apiResult);
            }
            else
            {
                DBG_PRINTF("Cy_BLE_HRSC_SetCharacteristicDescriptor() successful. \r\n\r\n");
            }
            break;   
            
        case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
            break;
            
        /**********************************************************
        *                       L2CAP Events
        ***********************************************************/
        case CY_BLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ:
            DBG_PRINTF("CY_BLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ \r\n");
            break;

        /**********************************************************
        *                       Other Events
        ***********************************************************/
        case CY_BLE_EVT_PENDING_FLASH_WRITE:
            /* Inform application that flash write is pending. Stack internal data 
            * structures are modified and require to be stored in Flash using 
            * Cy_BLE_StoreBondingData() */
            DBG_PRINTF("CY_BLE_EVT_PENDING_FLASH_WRITE\r\n");
            break;

        default:
            DBG_PRINTF("Other event: 0x%lx \r\n", event);
            break;
    }
}
 
/*******************************************************************************
* Function Name: IsDeviceInBondList()
********************************************************************************
* Summary:
*  This function returns nonzero value when bdHandle exists in bond list
*
*******************************************************************************/
static uint32_t IsDeviceInBondList(uint32_t bdHandle)
{
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_gap_peer_addr_info_t bondedDeviceInfo[CY_BLE_MAX_BONDED_DEVICES];
    cy_stc_ble_gap_bonded_device_list_info_t bondedDeviceList =
    {
        .bdHandleAddrList = bondedDeviceInfo
    };
    bool deviceIsDetected = false;
    uint32_t deviceCount;
    
    /* Find out whether the device has bonding information stored already or not */
    apiResult = Cy_BLE_GAP_GetBondList(&bondedDeviceList);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_GAP_GetBondedDevicesList API Error: 0x%x \r\n", apiResult);
    }
    else
    {
        deviceCount = bondedDeviceList.noOfDevices;

        if(deviceCount != 0u)
        {
            do
            {
                deviceCount--;
                if(bdHandle == bondedDeviceList.bdHandleAddrList[deviceCount].bdHandle)
                {
                    deviceIsDetected = true;
                }
            } while(deviceCount != 0u);
        }
    }
    return(deviceIsDetected);
}


/****************************************************************************** 
* Function Name: CheckAdvPacketForServiceUuid
*******************************************************************************
*
* Summary:
*   This function parses advertising packet and returns nonzero value 
*   when the packet contains service UUID equal to input parameter.
*
* Parameters:
*   eventParam: the pointer to a data structure specified by the event.
*   uuid: 16-bit UUID of the service.
*
* Return:
*   Nonzero value when the advertising packet contains service UUID.
*
******************************************************************************/
static uint32_t CheckAdvPacketForServiceUuid(cy_stc_ble_gapc_adv_report_param_t *eventParam, uint16_t uuid)
{
    bool servicePresent = false; 
    uint32_t advIndex   = 0u;
    uint32_t i;
    
    do
    {
        /* Find complete or incomplete Service UUID list field type. */
        if((eventParam->data[advIndex + 1u] == (uint8_t)CY_BLE_GAP_ADV_INCOMPL_16UUID) || 
           (eventParam->data[advIndex + 1u] == (uint8_t)CY_BLE_GAP_ADV_COMPL_16UUID))
        {
            /* Compare uuid values with input parameter */
            for(i = 0u; (i < (eventParam->data[advIndex] - 1u)) && (servicePresent == false); i += sizeof(uint16_t))
            {
                if(Cy_BLE_Get16ByPtr(&eventParam->data[advIndex + 2u + i]) == uuid)
                {
                    servicePresent = true;
                }
            }
        }
        advIndex += eventParam->data[advIndex] + 1u;
    }while((advIndex < eventParam->dataLen) && (servicePresent == false));    
    
    return(servicePresent);
}


/*******************************************************************************
* Function Name: Connect
********************************************************************************
*
* Summary:
*   Connect to peer device
*
* Parameters:
*   peerAddrType: Type of the peer address
*   peerBdAddr: Pointer to the peer address array
*
*******************************************************************************/
void Connect(cy_stc_ble_gap_bd_addr_t *peerBdAddr)
{
    cy_en_ble_api_result_t apiResult;
    
    apiResult = Cy_BLE_GAPC_ConnectDevice(peerBdAddr, CY_BLE_CENTRAL_CONFIGURATION_0_INDEX);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("ConnectDevice API Error: 0x%x ", apiResult);
    }
    else
    {
        DBG_PRINTF("Connecting to the device ");
    } 
    DBG_PRINTF("(address  - %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x) \r\n", peerBdAddr->bdAddr[5u], peerBdAddr->bdAddr[4u],
                                                                   peerBdAddr->bdAddr[3u], peerBdAddr->bdAddr[2u], 
                                                                   peerBdAddr->bdAddr[1u], peerBdAddr->bdAddr[0u]);         
}


/*******************************************************************************
* Function Name: Disconnect
********************************************************************************
*
* Summary:
*   Disconnect a peer device.
*
* Parameters:
*   connHandle: connection handle of a peer device
*
*******************************************************************************/
void Disconnect(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_gap_disconnect_info_t param =
    {
        .bdHandle = connHandle.bdHandle,
        .reason   = CY_BLE_HCI_ERROR_OTHER_END_TERMINATED_USER
    };
    
    DBG_PRINTF("Cy_BLE_GAP_Disconnect param: bdHandle:%x, reason:%x \r\n", param.bdHandle, param.reason);
    apiResult = Cy_BLE_GAP_Disconnect(&param);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_GAP_Disconnect API Error: 0x%x \r\n", apiResult);
    }
    else
    {
        DBG_PRINTF("Cy_BLE_GAP_Disconnect API Success \r\n");
    } 
}


/*******************************************************************************
* Function Name: ObserveBeacons
********************************************************************************
*
* This function parses advertising packet (beacons) and prints beacons data:
* RAW DATA, COMPANY ID, UUID, RSSI, etc
*
* Parameters:
*   eventParam: Pointer to the peer address array
*
*******************************************************************************/
static void ObserveBeacons(cy_stc_ble_gapc_adv_report_param_t *eventParam)
{    
    uint32_t i;
    uint32_t beaconIdx  = 0u;
    bool beaconDetected = false;
    
    /* Print raw data */
    DBG_PRINTF("CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT: ADDRESS: %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x, RSSI: %d dBm, data: ", 
                eventParam->peerBdAddr[5u], eventParam->peerBdAddr[4u], eventParam->peerBdAddr[3u],
                eventParam->peerBdAddr[2u], eventParam->peerBdAddr[1u], eventParam->peerBdAddr[0u], 
                eventParam->rssi);  
    for(i = 0; i < eventParam->dataLen; i++)
    {
        DBG_PRINTF("%2.2x ", eventParam->data[i]);    
    }
    DBG_PRINTF(" \r\n");
    
    /* Find Manufacture specific data in advertising packet */
    for(i = 0u; (i < eventParam->dataLen) && (beaconDetected == false); )
    {
        if(eventParam->data[i + 1u] == CY_BLE_GAP_ADV_MANUFACTURER_SPECIFIC_DATA)
        {
            beaconDetected = true;
            beaconIdx = i;
        }
        i += eventParam->data[i] + 1u;
    }
    if(beaconDetected == true)
    {
        /* Check AltBeacon 
        --------------------------------------------------------------------------------------------------------------
        |           |           |               |               |                            |           |           |
        | 1 Byte    |  1 Byte   |   2 Bytes     |  2 Bytes      |   20 Bytes                 | 1 Byte    |  1 Byte   |
        | AD Length |  AD Type  |   MFG ID      |  Beacon Code  |   Beacon ID                | Ref RSSI  |  MFG RSVD |
        |  [0x1B]   |  [0xFF]   |               |   [0xBEAC]    |                            |           |           |
        --------------------------------------------------------------------------------------------------------------   
        Refer to: section "AltBeacon Protocol Format" of https://github.com/AltBeacon/spec
        
        */
        if((eventParam->dataLen >= (ALTB_ADDR_RSSI_OFFSET + ALTB_SIZE_RSSI)) &&
           (eventParam->data[beaconIdx + ADDR_DATA_TYPE_OFFSET] == CY_BLE_GAP_ADV_MANUFACTURER_SPECIFIC_DATA) &&      /* data type   */
           (eventParam->data[beaconIdx + ADDR_DATA_LEN_OFFSET] >= 0x1B) &&                                            /* length      */
           (eventParam->data[beaconIdx + ALTB_ADDR_BCODE_OFFSET] == 0xBE) &&                                          /* beacon code */
           (eventParam->data[beaconIdx + ALTB_ADDR_BCODE_OFFSET + 1u] == 0xAC))
        {
            DBG_PRINTF("[ALT BEACON] COMPANY ID: 0x%2.2x, RSSI_1M: %d, UUID: ", 
                        Cy_BLE_Get16ByPtr(&eventParam->data[beaconIdx + ALTB_ADDR_COID_OFFSET]), 
                        (int8_t) eventParam->data[beaconIdx + ALTB_ADDR_RSSI_OFFSET]);     
            for(i = 0; i < ALTB_SIZE_UUID; i++)
            {
                DBG_PRINTF("%2.2x ", eventParam->data[beaconIdx + ALTB_ADDR_UUID_OFFSET + i]);
            } 
            DBG_PRINTF("\r\n\r\n"); 
        }    
        
        /* Check Cypress Beacon 
        ----------------------------------------------------------------------------------------------------------------
        |           |          |             |          |           |                   |          |          |        |
        |   1 Byte  |  1 Byte  |   2 Bytes   |  1 Byte  |  1 Byte   |   16 Bytes        | 2 Bytes  | 2 Bytes  | 1 Byte |
        | AD Length | AD Type  | COMPANY ID  | Dev Type |   LEN_3   |   UUID            | MFG ID   | MFG ID   |  RSSI  |
        |   [0x1A]  | [0xFF]   |             |  [0x02]  |  [0x15]   |                   |          |          |        |
        ----------------------------------------------------------------------------------------------------------------
        Refer to: 002-00297, section 8.5 "BLE Beacon Format", http://www.cypress.com/file/187321/download
        */
        else if((eventParam->dataLen >= (ALTB_ADDR_RSSI_OFFSET + ALTB_SIZE_RSSI)) &&
                (eventParam->data[beaconIdx + ADDR_DATA_TYPE_OFFSET] == CY_BLE_GAP_ADV_MANUFACTURER_SPECIFIC_DATA) && /* data type   */
                (eventParam->data[beaconIdx + ADDR_DATA_LEN_OFFSET] >= 0x1A) &&                                       /* length      */
                (eventParam->data[beaconIdx + CYB_ADDR_DEV_TYPE_OFFSET] == CYB_VALUE_DEV_TYPE) &&                     /* device type */
                (eventParam->data[beaconIdx + CYB_ADDR_LEN3_OFFSET] == CYB_VALUE_LEN3))                               /* length 3    */  
        {
            DBG_PRINTF("[CYPRESS BEACON] COMPANY ID: 0x%2.2x, RSSI_1M: %d, UUID: ", 
                        Cy_BLE_Get16ByPtr(&eventParam->data[beaconIdx + CYB_ADDR_COID_OFFSET]),
                        (int8_t) eventParam->data[beaconIdx + CYB_ADDR_RSSI_OFFSET]);     
            for(i = 0; i < CYB_SIZE_UUID; i++)
            {
                DBG_PRINTF("%2.2x ", eventParam->data[beaconIdx + CYB_ADDR_UUID_OFFSET + i]);
            } 
            DBG_PRINTF("\r\n\r\n"); 
        }
        else
        {
            DBG_PRINTF("Unknown Beacon \r\n\r\n");
        }
    }
}

/*******************************************************************************
* Function Name: GetAppConnInfoPtr
********************************************************************************
*
* Summary:
*  Returns a pointer to appConnInfo structure
*
*******************************************************************************/
app_stc_connection_info_t* GetAppConnInfoPtr(void)
{ 
    return(&appConnInfo);
}


/*******************************************************************************
* Function Name: GetAppScanDevInfoPtr
********************************************************************************
*
* Summary:
*  Returns a pointer to appScanDevInfo structure
*
*******************************************************************************/
app_stc_scan_device_list_t* GetAppScanDevInfoPtr(void)
{ 
    return(&appScanDevInfo);
}


/*******************************************************************************
* Function Name: Timer_Interrupt
********************************************************************************
*
* Summary:
*  Handles the Interrupt Service Routine for the MCWDT timer.
*
*******************************************************************************/
void Timer_Interrupt(void)
{
    /* Update Led State */
    UpdateLedState();
    
    /* Indicate that timer is raised to the main loop */
    mainTimer++;
    sleepTimer++;
   
    MCWDT_ClearInterrupt(CY_MCWDT_CTR0);
}


/*******************************************************************************
* Function Name: Hibernate
********************************************************************************
*
* Summary:
*   Set device in Hibernate mode
*
*******************************************************************************/
void Hibernate(void)
{
    if((Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED) && 
       (Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_STOPPED) &&
       (Cy_BLE_GetNumOfActiveConn() == 0u))
    {
        if(sleepTimer == TIMER_SLEEP_MODE)
        {
            Cy_BLE_Stop();
            UpdateLedState();
        }
    }
    else
    {
        sleepTimer = 0u; 
    }
}

/*******************************************************************************
* Function Name: HostMain
********************************************************************************
*
* Summary:
*   Main function for the project.
*
* Theory:
*   The function starts BLE and UART components.
*   This function processes all BLE events and also implements the low power 
*   functionality.
*
*******************************************************************************/
int HostMain(void)
{
    cy_en_ble_api_result_t apiResult;
    char8 command;
    
    /* Initialize the user interface: LEDs, SW2, ect.  */
    InitUserInterface();
    
    /* Initialize Timer */
    Cy_SysInt_Init(&Timer_Int_cfg, Timer_Interrupt);
    NVIC_EnableIRQ(Timer_Int_cfg.intrSrc);   
    MCWDT_SetInterruptMask(CY_MCWDT_CTR0);
    MCWDT_Start(); 
    
    /* Initialize Debug UART */
    UART_START();
    DBG_PRINTF("BLE Multi Role Example\r\n");
    
    /* Start BLE component and register generic event handler */
    apiResult = Cy_BLE_Start(AppCallBack);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_Start API Error: 0x%x", apiResult);
    }
    
    /* Initialize BLE Services */
    BasInit();
    HrsInit();
    
    /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    while(1)
    {
        /* Cy_BLE_ProcessEvents() allows BLE stack to process pending events */
        Cy_BLE_ProcessEvents();
        
        /* Wait for connection established with Master/Central device (CySmart, HRS collector) */
        if((Cy_BLE_GetConnectionState(appConnInfo.peripheral.connHandle) >= CY_BLE_CONN_STATE_CONNECTED) &&
            (Cy_BLE_IsDevicePaired(&appConnInfo.peripheral.connHandle) == true) && (mainTimer == 1u))
        {
            if((Cy_BLE_GetConnectionState(appConnInfo.central.connHandle) >= CY_BLE_CONN_STATE_CONNECTED) &&
               (appConnInfo.central.isNotificationEn == true))
            {
               /* HRS Bridge Mode
                *                              PSoC6 BLE Device master/slave (current CE)       
                *                                 ++++++++++++++++++++++++++++++++++       
                *   CE217639(Server)             +                                  +                CE217639(Client)
                *     --------                   +                                  +                  -----------
                *    |        |                  +  ---------          -----------  +                 |           |         
                *    |  HRS   |  Heart Rate      + |         |  HRS   |           | +  Heart Rate     |    HRS    |
                *    | Server | -------------->  + | Central | =====> | Peripheral| + --------------> | Collector |
                *    |        |  Notifications   + |         | Bridge |           | +  Notifications  | (Central) |
                *    |        |                  +  ---------          -----------  +                 |           | 
                *     --------                   +                                  +                  ----------- 
                *                                +                                  +
                *                                 ++++++++++++++++++++++++++++++++++     
                *   
                */
                if(appConnInfo.central.isNewNotification == true)
                {
                    HrsBridge(); 
                }
            }    
            else
            {   
                /* HRS Simulation Mode  */
                
                /* Starts simulate Heart Rate data if we don't connect to HRS Server */
                HrsSimulateHeartRate(appConnInfo.peripheral.connHandle);
            }    
            
            /* Periodically simulate Battery level charging */
            BasSimulateBattery(appConnInfo.peripheral.connHandle);
        }
        
        /* Reset 1 second timer */
        if(mainTimer)
        {
            mainTimer = 0u;
        }
        
        /* Process command from debug terminal */
        if((command = UART_DEB_GET_CHAR()) != UART_DEB_NO_DATA) 
        {
            ProcessUartCommands(command); 
        }

        /* Set device in Hibernate mode if advertisement and scan are stopped and 
           sleepTimer counted to 10 seconds ( refer to TIMER_SLEEP_MODE) */
        Hibernate();    
        
    #if(CY_BLE_BONDING_REQUIREMENT == CY_BLE_BONDING_YES)
        /* Store bonding data to flash only when all debug information has been sent */    
        if(cy_ble_pendingFlashWrite != 0u) 
        {   
            apiResult = Cy_BLE_StoreBondingData();    
            DBG_PRINTF("Store bonding data, status: %x, pending: %x \r\n", apiResult, cy_ble_pendingFlashWrite);
        }
    #endif /* CY_BLE_BONDING_REQUIREMENT == CY_BLE_BONDING_YES */
    }
}


/* [] END OF FILE */
