/*******************************************************************************
* File Name: host_main.c
*
* Version: 1.0
*
* Description:
*  This project demonstrates the operation of the Heart Rate Profile
*  in Collector (Central) role.
*
* Related Document:
*  HEART RATE PROFILE SPECIFICATION v1.0
*  HEART RATE SERVICE SPECIFICATION v1.0
*
* Note:
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
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


#include "project.h"
#include "common.h"
#include "user_interface.h"
#include "basc.h"
#include "hrsc.h"
    
    
/* Global Variables */
static cy_stc_ble_conn_handle_t   appConnHandle;
static app_stc_scan_device_list_t appScanDevInfo;
static volatile uint32_t          appState = APP_STATE_IDLE ;

static volatile uint32_t          mainTimer  = 1u;
static cy_stc_ble_timer_info_t    timerParam = { .timeout = ADV_TIMER_TIMEOUT };
      

/* Private Function Prototypes */
static bool CheckAdvPacketForServiceUuid(cy_stc_ble_gapc_adv_report_param_t *eventParam, uint16_t uuid);


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
static bool CheckAdvPacketForServiceUuid(cy_stc_ble_gapc_adv_report_param_t *eventParam, uint16_t uuid)
{
    bool servicePresent = false; 
    uint32_t advIndex = 0u;
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
* Function Name: AppCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE Component.
*
* Parameters:
*  event      - The event code.
*  eventParam - The event parameters.
*
*******************************************************************************/
void AppCallBack(uint32_t event, void *eventParam)
{   
    cy_en_ble_api_result_t apiResult;
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
    uint32_t i;
  
    switch(event)
    {
        case CY_BLE_EVT_STACK_ON:
            DBG_PRINTF("CY_BLE_EVT_STACK_ON, StartAdvertisement \r\n");  
            
            /* Starts the Scanning process */
            apiResult = Cy_BLE_GAPC_StartScan(CY_BLE_SCANNING_FAST, CY_BLE_CENTRAL_CONFIGURATION_0_INDEX);
            if(apiResult != CY_BLE_SUCCESS)
            {   
                DBG_PRINTF("Cy_BLE_GAPC_StartScan API Error: 0x%x \r\n", apiResult);
            }
                        
            /* Generates the security keys */
            apiResult = Cy_BLE_GAP_GenerateKeys(&keyInfo);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAP_GenerateKeys API Error: 0x%x \r\n", apiResult);
            }
                    
            /* Display Bond list */
            App_DisplayBondList();
            
            DBG_PRINTF("\r\nWhen you see a scan report from the device you wish to connect to - press 'c'.\r\n\r\n");
            break;
            
        case CY_BLE_EVT_TIMEOUT: /* 0x01 -> GAP limited discoverable mode timeout. */
                                 /* 0x02 -> GAP pairing process timeout. */
                                 /* 0x03 -> GATT response timeout. */
            if((((cy_stc_ble_timeout_param_t *)eventParam)->reasonCode == CY_BLE_GENERIC_APP_TO) && 
               (((cy_stc_ble_timeout_param_t *)eventParam)->timerHandle == timerParam.timerHandle))
            {
                /* Update Led State */
                UpdateLedState();
                
                /* Indicate that timer is raised to the main loop */
                mainTimer++;
                
                /* Press and hold the mechanical button (SW2) during 4 seconds to clear the bond list. */
                App_RemoveDevicesFromBondListBySW2Press(SW2_PRESS_TIME_DEL_BOND_LIST);     
            }
            else
            {
                DBG_PRINTF("CY_BLE_EVT_TIMEOUT: %d \r\n", (((cy_stc_ble_timeout_param_t *)eventParam)->reasonCode));
            }       
            break;

        case CY_BLE_EVT_HARDWARE_ERROR:    /* This event indicates that some internal HW error has occurred. */
            DBG_PRINTF("CY_BLE_EVT_HARDWARE_ERROR \r\n");
            ShowError();
            break;
            
        /* This event will be triggered by host stack if BLE stack is busy or not busy.
         *  Parameter corresponding to this event will be the state of BLE stack.
         *  BLE stack busy = CY_BLE_STACK_STATE_BUSY,
         *  BLE stack not busy = CY_BLE_STACK_STATE_FREE 
         */
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
            UART_DEB_WAIT_TX_COMPLETE();
            UpdateLedState();
            /* Hibernate */
            Cy_SysPm_Hibernate();
            break;   

        /**********************************************************
        *                       GAP Events
        ***********************************************************/

        case CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
        {
            cy_stc_ble_gapc_adv_report_param_t  *advReport = (cy_stc_ble_gapc_adv_report_param_t *)eventParam;
            
            bool printHeader  = false;
            bool isNewAddress = true;
                            
            if((appScanDevInfo.pauseScanProgress == false) && 
               (advReport->eventType == CY_BLE_GAPC_CONN_UNDIRECTED_ADV))
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
                DBG_PRINTF("Scan complete! \r\n \r\n");
                
                if(appScanDevInfo.connReq)
                {
                    
                    apiResult = Cy_BLE_GAPC_ConnectDevice(&appScanDevInfo.address[appScanDevInfo.connReq - 1u], 
                                                           CY_BLE_CENTRAL_CONFIGURATION_0_INDEX);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("ConnectDevice API Error: 0x%x ", apiResult);
                    }
                    else
                    {
                        DBG_PRINTF("Connecting to the device ");
                        /* Print the devices Addr of the detected Client */
                        DBG_PRINTF("%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x \r\n",
                                                    appScanDevInfo.address[appScanDevInfo.connReq - 1u].bdAddr[5u], 
                                                    appScanDevInfo.address[appScanDevInfo.connReq - 1u].bdAddr[4u],
                                                    appScanDevInfo.address[appScanDevInfo.connReq - 1u].bdAddr[3u], 
                                                    appScanDevInfo.address[appScanDevInfo.connReq - 1u].bdAddr[2u],
                                                    appScanDevInfo.address[appScanDevInfo.connReq - 1u].bdAddr[1u], 
                                                    appScanDevInfo.address[appScanDevInfo.connReq - 1u].bdAddr[0u]); 
                    } 
                    appScanDevInfo.connReq = 0u;
                }
                else
                {
                    if(Cy_BLE_GetConnectionState(appConnHandle) < CY_BLE_CONN_STATE_CONNECTED)
                    {
                        /* Set the device to the Hibernate mode. */
                        Cy_BLE_Stop();
                        UpdateLedState();        
                    }
                }
            }
            else
            {
                DBG_PRINTF("GAPC_START_SCANNING\r\n");
            }
            break;
        
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

        case CY_BLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO:
            DBG_PRINTF("CY_BLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO:"
                       " bdHandle=%x, security=%x, bonding=%x, ekeySize=%x, err=%x \r\n", 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bdHandle, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).security, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bonding, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).ekeySize, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).authErr);
            break;
            
        case CY_BLE_EVT_GAP_PASSKEY_ENTRY_REQUEST:
            DBG_PRINTF("CY_BLE_EVT_GAP_PASSKEY_ENTRY_REQUEST\r\n");
            DBG_PRINTF("Please enter the passkey displayed on the peer device:\r\n");
            App_SetAuthIoCap(CY_BLE_GAP_IOCAP_KEYBOARD_ONLY);
            break;
            
        case CY_BLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
            DBG_PRINTF("CY_BLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST: %6.6ld\r\n", *(uint32_t *)eventParam);
            App_SetAuthIoCap(CY_BLE_GAP_IOCAP_DISPLAY_ONLY);
            break;
            
        case CY_BLE_EVT_GAP_NUMERIC_COMPARISON_REQUEST:
            DBG_PRINTF("Compare this passkey with the one displayed in your peer device and press 'y' or 'n':"
                       " %6.6lu \r\n", *(uint32_t *)eventParam);
            App_SetAuthIoCap(CY_BLE_GAP_IOCAP_DISPLAY_YESNO);
            break;    
            
        case CY_BLE_EVT_GAP_AUTH_FAILED:
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_FAILED, reason: ");
            App_ShowAuthError(((cy_stc_ble_gap_auth_info_t *)eventParam)->authErr);
            break;

        case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_CONNECTED: connIntv = %d ms \r\n",          /* in milliseconds / 1.25ms */
                        ((cy_stc_ble_gap_connected_param_t *)eventParam)->connIntv * 5u /4u);
                       
            keyInfo.SecKeyParam.bdHandle = (*(cy_stc_ble_gap_connected_param_t *)eventParam).bdHandle;     
            if(App_IsDeviceInBondList(keyInfo.SecKeyParam.bdHandle) == 0u)
            {
                apiResult = Cy_BLE_GAP_SetSecurityKeys(&keyInfo);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAP_SetSecurityKeys API Error:");
                    PrintApiResult(apiResult);
                }
            } 
            
            /* Send an authorization request */
            cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].bdHandle = 
                (*(cy_stc_ble_gap_connected_param_t *)eventParam).bdHandle;   
            apiResult = Cy_BLE_GAP_AuthReq(&cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAP_AuthReq API Error:  ");
                PrintApiResult(apiResult);
            }
            break;

        case CY_BLE_EVT_GAP_KEYS_GEN_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GAP_KEYS_GEN_COMPLETE \r\n");
            keyInfo.SecKeyParam = (*(cy_stc_ble_gap_sec_key_param_t *)eventParam);
            Cy_BLE_GAP_SetIdAddress(&cy_ble_deviceAddress);
            break;
            
        case CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE: connIntv = %d ms \r\n", /* in milliseconds / 1.25ms */
                        ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->connIntv * 5u /4u);
            break;
            
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_DISCONNECTED: bdHandle=%x, reason=%x, status=%x\r\n",
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).bdHandle, 
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).reason, 
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).status);
            break;

        case CY_BLE_EVT_GAP_AUTH_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_COMPLETE: security:%x, bonding:%x, ekeySize:%x, authErr %x \r\n",
                        ((cy_stc_ble_gap_auth_info_t *)eventParam)->security,
                        ((cy_stc_ble_gap_auth_info_t *)eventParam)->bonding, 
                        ((cy_stc_ble_gap_auth_info_t *)eventParam)->ekeySize, 
                        ((cy_stc_ble_gap_auth_info_t *)eventParam)->authErr);
                        
            if(((cy_stc_ble_gap_auth_info_t *)eventParam)->authErr == 0u)
            {         
                /* Starts the Server Discovery process */
                DBG_PRINTF("StartDiscovery \r\n");
                apiResult = Cy_BLE_GATTC_StartDiscovery(appConnHandle);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("StartDiscovery API Error: %x \r\n", apiResult);
                }
            }
            break;

        case CY_BLE_EVT_GAP_ENCRYPT_CHANGE:
            DBG_PRINTF("CY_BLE_EVT_GAP_ENCRYPT_CHANGE: %d \r\n", *(uint8_t *)eventParam);
            break;
            
        case CY_BLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
            DBG_PRINTF("CY_BLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT \r\n");
            break;

        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        
        case CY_BLE_EVT_GATT_CONNECT_IND:
            appConnHandle = *(cy_stc_ble_conn_handle_t *)eventParam;
            DBG_PRINTF("CY_BLE_EVT_GATT_CONNECT_IND: %x, %x \r\n", 
                (*(cy_stc_ble_conn_handle_t *)eventParam).attId, 
                (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle);
            
            /* Stops the Scanning process after connection */
            Cy_BLE_GAPC_StopScan();
            appState = APP_STATE_IDLE;
            break;
            
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            DBG_PRINTF("CY_BLE_EVT_GATT_DISCONNECT_IND: %x, %x \r\n", 
                (*(cy_stc_ble_conn_handle_t *)eventParam).attId, 
                (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle);
                        
            /* Starts the Scanning process after disconect */
            Cy_BLE_GAPC_StartScan(CY_BLE_SCANNING_FAST, CY_BLE_CENTRAL_CONFIGURATION_0_INDEX);
            appState = APP_STATE_IDLE;
            break;
    
        case CY_BLE_EVT_GATTC_XCHNG_MTU_RSP:
            DBG_PRINTF("CY_BLE_EVT_GATTC_XCHNG_MTU_RSP: \r\n");
            DBG_PRINTF("MTU is set to %d Bytes\r\n", (*(cy_stc_ble_gatt_xchg_mtu_param_t *)eventParam).mtu);
            break;
            
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            DBG_PRINTF("CY_BLE_EVT_GATTS_WRITE_REQ attr handle: %4.4x , value: ",
                        ((cy_stc_ble_gatts_write_cmd_req_param_t *)eventParam)->handleValPair.attrHandle);
            
            for(i = 0; i < ((cy_stc_ble_gatts_write_cmd_req_param_t *)eventParam)->handleValPair.value.len; i++)
            {
                DBG_PRINTF("%2.2x ", ((cy_stc_ble_gatts_write_cmd_req_param_t *)eventParam)->handleValPair.value.val[i]);
            }
            DBG_PRINTF("\r\n");
            break;
            
        case CY_BLE_EVT_GATTS_XCNHG_MTU_REQ:
            DBG_PRINTF("CY_BLE_EVT_GATTS_XCNHG_MTU_REQ \r\n");
            break;
            
        case CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF:
            DBG_PRINTF("CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF \r\n");
            break;
            
        case CY_BLE_EVT_GATTS_PREP_WRITE_REQ:
            DBG_PRINTF("CY_BLE_EVT_GATTS_PREP_WRITE_REQ \r\n");
            break;
        
        case CY_BLE_EVT_GATTS_EXEC_WRITE_REQ:
            DBG_PRINTF("CY_BLE_EVT_GATTS_EXEC_WRITE_REQ \r\n");
            break;

        /**********************************************************
        *                  GATT Service Events 
        ***********************************************************/
            
        case CY_BLE_EVT_GATTS_INDICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_GATTS_INDICATION_ENABLED \r\n");
            break;
        
        case CY_BLE_EVT_GATTS_INDICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_GATTS_INDICATION_DISABLED \r\n");
            break;
            
        case CY_BLE_EVT_GATTC_INDICATION:
            DBG_PRINTF("CY_BLE_EVT_GATTS_INDICATION \r\n");
            break;

        case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
            /* Triggered on server side when client sends read request and when
            * characteristic has CY_BLE_GATT_DB_ATTR_CHAR_VAL_RD_EVENT property set.
            * This event could be ignored by application unless it need to response
            * by error response which needs to be set in gattErrorCode field of
            * event parameter. */
            DBG_PRINTF("CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ: handle: %x \r\n", 
                        ((cy_stc_ble_gatts_char_val_read_req_t *)eventParam)->attrHandle);
            break;

        case CY_BLE_EVT_GATTC_DISCOVERY_COMPLETE:
        {
            DBG_PRINTF("Discovery complete.\r\n");
            uint8_t discIdx = Cy_BLE_GetDiscoveryIdx(appConnHandle);  
            
            for(i = 0u; i < CY_BLE_SRVI_COUNT; i++)
            {
                DBG_PRINTF("Service with UUID 0x%x has range from 0x%x to 0x%x\r\n", 
                               cy_ble_serverInfo[discIdx][i].uuid,
                               cy_ble_serverInfo[discIdx][i].range.startHandle,
                               cy_ble_serverInfo[discIdx][i].range.endHandle);
            }

            DBG_PRINTF("Heart Rate Characteristic\r\n");
            for(i = 0u; i < CY_BLE_HRS_CHAR_COUNT; i++)
            {
                switch(i)
                {
                    case CY_BLE_HRS_HRM:
                        DBG_PRINTF("CY_BLE_HRS_HRM\t");
                        break;
                    case CY_BLE_HRS_BSL:
                        DBG_PRINTF("CY_BLE_HRS_BSL\t");
                        break;
                    case CY_BLE_HRS_CPT:
                        DBG_PRINTF("CY_BLE_HRS_CPT\t");
                        break; 
                    default:
                        DBG_PRINTF("Unknown\t\t");
                        break;
                        
                }
                DBG_PRINTF(" - Handle: 0x%4.4x. \r\n", cy_ble_hrsc[discIdx].charInfo[i].valueHandle);
            }
            if(cy_ble_hrsc[discIdx].hrmCccdHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                DBG_PRINTF(" Heart Rate Measurement CCCD. Handle: 0x%4.4x\r\n", cy_ble_hrsc[discIdx].hrmCccdHandle);
            }
            else
            {
                DBG_PRINTF(" No Descriptor\r\n");
            }     

            appState = APP_STATE_READ_HRS_BSL;
            
            break;
        }
           
        case CY_BLE_EVT_GATTC_DISC_SKIPPED_SERVICE:
            DBG_PRINTF("CY_BLE_EVT_GATTC_DISC_SKIPPED_SERVICE \r\n ");
            break;
            
        case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
            DBG_PRINTF("CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF \r\n");
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
* Function Name: HostMain
********************************************************************************
*
* Summary:
*  The main function of the project.
*
*******************************************************************************/
int HostMain()
{
    cy_en_ble_api_result_t apiResult;
    char8 command;
        
    /* Initialization the user interface: LEDs, SW2, ect.  */
    InitUserInterface();
    
    /* Initialize Debug UART */
    UART_START();
    DBG_PRINTF("BLE Heart Rate Collector Example Project\r\n");

    /* Start BLE component and register generic event handler */
    apiResult = Cy_BLE_Start(AppCallBack);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_Start API Error: 0x%x \r\n", apiResult);
    }

    /* Print stack version */
    PrintStackVersion();
    
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
        
        if((Cy_BLE_GetConnectionState(appConnHandle) >= CY_BLE_CONN_STATE_CONNECTED) && 
           ((appState & APP_STATE_WAITING_RESPONSE) == false))
        {
            switch(appState & 0x0fff)
            {        
                /* Send request to read the body sensor location char. */
                case APP_STATE_READ_HRS_BSL:
                
                    if((appState & APP_STATE_OPERATION_COMPLETE) == 0u)
                    {
                        apiResult = HrscReadBodySensorLocation();

                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("HrscReadBodySensorLocation API Error: ");
                            PrintApiResult(apiResult);
                        }
                        else
                        {
                            DBG_PRINTF("Body Sensor Location Read Request is sent \r\n");
                            appState |= APP_STATE_WAITING_RESPONSE;
                        }   
                    }
                    else
                    {
                        /* Previous operation is completed, go to next  */   
                        appState = APP_STATE_READ_HRS_CCCD;
                    }
                    break;
                
                /* Send request to read CCCD for characteristic */
                case APP_STATE_READ_HRS_CCCD:
                    if((appState & APP_STATE_OPERATION_COMPLETE) == 0u)
                    {
                        apiResult = Cy_BLE_HRSC_GetCharacteristicDescriptor(appConnHandle, CY_BLE_HRS_HRM, 
                                                                            CY_BLE_HRS_HRM_CCCD);
                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("Cy_BLE_HRSC_GetCharacteristicDescriptor API Error: ");
                            PrintApiResult(apiResult);
                        }
                        else
                        {
                            DBG_PRINTF("HRM CCCD Read Request is sent \r\n");
                            appState |= APP_STATE_WAITING_RESPONSE;
                        } 
                    }
                    else
                    {
                        /* Previous operation is completed, go to next  */   
                        appState = APP_STATE_READ_BAS_CCCD;
                    }
                    break;
                    
                /* Send request to read CCCD for characteristic*/
                case APP_STATE_READ_BAS_CCCD:
                    if((appState & APP_STATE_OPERATION_COMPLETE) == 0u)
                    {
                        apiResult = Cy_BLE_BASC_GetCharacteristicDescriptor(appConnHandle, 0, CY_BLE_BAS_BATTERY_LEVEL,
                                                                            CY_BLE_BAS_BATTERY_LEVEL_CCCD);
                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("Cy_BLE_BASC_GetCharacteristicDescriptor API Error: ");
                            PrintApiResult(apiResult);
                        }
                        else
                        {
                            DBG_PRINTF("BAS CCCD Read Request is sent \r\n");
                            appState |= APP_STATE_WAITING_RESPONSE;
                        }
                        
                    }
                    else
                    {
                        /* Previous operation is completed, go to next  */   
                        appState = APP_STATE_ENABLING_CCCD; 
                    }
                    break;
                
                /* Send request to enable notifications for HRS and BAS */    
                case APP_STATE_ENABLING_CCCD:
                {    
                    uint16_t config = CY_BLE_CCCD_NOTIFICATION;                   
                    if(hrsNotification != CY_BLE_CCCD_NOTIFICATION)
                    {   
                        apiResult = Cy_BLE_HRSC_SetCharacteristicDescriptor(appConnHandle, CY_BLE_HRS_HRM,
                                                                             CY_BLE_HRS_HRM_CCCD , CY_BLE_CCCD_LEN,
                                                                             (uint8_t *)&config);
                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("HrscConfigHeartRateNtf API Error: ");
                            PrintApiResult(apiResult);
                        }
                        else
                        {
                            DBG_PRINTF("HRM CCCD Write Request is sent \r\n");
                            appState |= APP_STATE_WAITING_RESPONSE;
                        }
                    }
                    else if(basNotification != CY_BLE_CCCD_NOTIFICATION)
                    {
                        apiResult = Cy_BLE_BASC_SetCharacteristicDescriptor(appConnHandle, 0, CY_BLE_BAS_BATTERY_LEVEL,
                                                                             CY_BLE_BAS_BATTERY_LEVEL_CCCD, 
                                                                             CY_BLE_CCCD_LEN,(uint8_t *)&config);
                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("Cy_BLE_BASC_SetCharacteristicDescriptor API Error: ");
                            PrintApiResult(apiResult);
                        }
                        else
                        {
                            DBG_PRINTF("Battery Level CCCD Write Request is sent \r\n");
                            appState |= APP_STATE_WAITING_RESPONSE;
                        }
                    }
                    else
                    {
                        appState = APP_STATE_IDLE;
                    }
                }   
                break;  
            }
            
            /* Clear complete state */
            appState &= ~APP_STATE_OPERATION_COMPLETE;
        }                
 
        /* Restart 1 second timer */
        if(mainTimer != 0u)
        {
            mainTimer = 0u;
            Cy_BLE_StartTimer(&timerParam);
        }

        /* Passkey entry */
        if(App_IsAuthReq())
        {
            App_AuthReplay(appConnHandle);
        }
        
        /* Remove devices from the bond list. Should be done when no active connections */
        if((Cy_BLE_GetNumOfActiveConn() == 0u) && (App_IsRemoveBondListFlag() == true))
        {
            App_RemoveDevicesFromBondList();
        }
        
        /* Process command from debug terminal */
        command = UART_DEB_GET_CHAR();
        if(command != UART_DEB_NO_DATA)
        {            
            ProcessUartCommands(command);
        }
        
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



/*******************************************************************************
* Function Name: GetAppConnInfoPtr
********************************************************************************
*
* Summary:
*  Returns a pointer to appConnInfo structure
*
*******************************************************************************/
cy_stc_ble_conn_handle_t* GetAppConnHandlePtr(void)
{ 
    return(&appConnHandle);
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
* Function Name: GetAppState
********************************************************************************
*
* Summary:
*  Returns the state of the application state machine
*
*******************************************************************************/
uint32_t GetAppState(void)
{ 
    return(appState);
}

/*******************************************************************************
* Function Name: SetAppState
********************************************************************************
*
* Summary:
*  Set the state of the application state machine
*
*******************************************************************************/
void SetAppState(uint32_t state)
{ 
    appState = state;
}

/* [] END OF FILE */

