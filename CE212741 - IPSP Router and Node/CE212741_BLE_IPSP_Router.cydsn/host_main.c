/*******************************************************************************
* File Name: host_main.c
*
* Version: 1.0
*
*  This example demonstrates how to setup an IPv6 communication infrastructure 
*  between two devices over a BLE transport using L2CAP channel. Creation 
*  and transmission of IPv6 packets over BLE is not part of this example.
*
*  Router sends generated packets with different content to Node in the loop 
*  and validate them with the afterwards received data packet. Node simply wraps
*  received data coming from the Node, back to the Router.
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

    
#include "common.h"
#include "BLE.h"
#include <stdbool.h>


uint16_t                                    connIntv;                    /* in milliseconds / 1.25ms */
bool                                        l2capConnected = false;
cy_stc_ble_l2cap_cbfc_conn_cnf_param_t      l2capParameters;
cy_stc_ble_conn_handle_t                    appConnHandle;
cy_stc_ble_gap_bd_addr_t                    peerAddr[CY_BLE_MAX_ADV_DEVICES];
volatile uint32_t                           mainTimer = 1u;
uint8_t                                     advDevices = 0u;
uint8_t                                     deviceN = 0u;
uint8_t                                     state = STATE_INIT;
static uint16_t                             ipv6LoopbackBuffer[L2CAP_MAX_LEN/2];
uint8_t                                     custom_command = 0u;
cy_stc_ble_timer_info_t                     timerParam = { .timeout = TIMER_TIMEOUT };   


/****************************************************************************** 
* Function Name: CheckAdvPacketForServiceUuid
*******************************************************************************
*
* Summary:
*  This function parses advertising packet and returns nonzero value 
*  when the packet contains service UUID equal to input parameter.
*
* Parameters:
*  eventParam: the pointer to a data structure specified by the event.
*  uuid: 16-bit UUID of the service.
*
* Return:
*  Nonzero value when the advertising packet contains service UUID.
*
******************************************************************************/
uint32_t CheckAdvPacketForServiceUuid(cy_stc_ble_gapc_adv_report_param_t *eventParam, uint16_t uuid)
{
    uint32_t servicePresent = NO; 
    uint32_t advIndex = 0u;
    uint32_t i;
    
    do
    {
        /* Find complete or incomplete Service UUID list field type. */
        if((eventParam->data[advIndex + 1u] == (uint8)CY_BLE_GAP_ADV_INCOMPL_16UUID) || 
           (eventParam->data[advIndex + 1u] == (uint8)CY_BLE_GAP_ADV_COMPL_16UUID))
        {
            /* Compare uuid values with input parameter */
            for(i = 0u; (i < (eventParam->data[advIndex] - 1u)) && (servicePresent == NO); i += sizeof(uint16))
            {
                if(Cy_BLE_Get16ByPtr(&eventParam->data[advIndex + 2u + i]) == uuid)
                {
                    servicePresent = 1u;
                }
            }
        }
        advIndex += eventParam->data[advIndex] + 1u;
    }while((advIndex < eventParam->dataLen) && (servicePresent == NO));    
    
    return(servicePresent);
}


/*******************************************************************************
* Function Name: AppCallback
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE Component.
*
* Parameters:
*  event - the event code
*  eventParam - the event parameters
*
* Theory:
* The function is responsible for handling the events generated by the stack.
* It first starts scanning once the stack is initialized. 
* Upon scanning timeout this function enters Hibernate mode.
*
* IPSP protocol multiplexer for L2CAP is registered and the initial Receive 
* Credit Low Mark for Based Flow Control mode is set after CY_BLE_EVT_STACK_ON
* event.
* When GAP connection is established, after CY_BLE_EVT_GATT_CONNECT_IND event, 
* Router automatically initiates an L2CAP LE credit based connection with a PSM
* set to LE_PSM_IPSP.
* Use '1' command to generate and send first Data packet to Node through IPSP 
* channel. Sent data will be compared with the received data in the response packet 
* after CY_BLE_EVT_L2CAP_CBFC_DATA_READ event. When no failure is observed, a new 
* packet will be generated and sent to Node. Otherwise, a transfer will be stopped
* and "Wraparound failed" message will indicate a failure.
*
*******************************************************************************/
void AppCallback(uint32_t event, void* eventParam)
{
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_gapc_adv_report_param_t *advReport;
    uint8_t newDevice = 0u;
    uint16_t i;
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
    
    switch (event)
    {
        /**********************************************************
        *                       General Events
        ***********************************************************/
        case CY_BLE_EVT_STACK_ON: /* This event received when component is Started */
           {
                DBG_PRINTF("CY_BLE_EVT_STACK_ON, StartAdvertisement \r\n");
                /* Register IPSP protocol multiplexer to L2CAP and 
                *  set the initial Receive Credit Low Mark for Based Flow Control mode 
                */
                cy_stc_ble_l2cap_cbfc_psm_info_t l2capCbfcPsmParam =
                {
                    .creditLwm = LE_WATER_MARK_IPSP,
                    .l2capPsm  = CY_BLE_L2CAP_PSM_LE_PSM_IPSP,
                };
                apiResult = Cy_BLE_L2CAP_CbfcRegisterPsm(&l2capCbfcPsmParam);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_L2CAP_CbfcRegisterPsm API Error: 0x%x \r\n", apiResult);
                }
                
                /* Start Limited Discovery */
                apiResult = Cy_BLE_GAPC_StartScan(CY_BLE_SCANNING_FAST, 0u);                   
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("StartScan API Error: 0x%x \r\n", apiResult);
                }
                else
                {   
                    DBG_PRINTF("Bluetooth On, StartScan with addr: ");
      
                }

                /* Generates the security keys */
                apiResult = Cy_BLE_GAP_GenerateKeys(&keyInfo);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAP_GenerateKeys API Error: 0x%x \r\n", apiResult);
                }
                
                break; 
            }
                
        case CY_BLE_EVT_TIMEOUT: 
            if((((cy_stc_ble_timeout_param_t *)eventParam)->reasonCode == CY_BLE_GENERIC_APP_TO) && 
               (((cy_stc_ble_timeout_param_t *)eventParam)->timerHandle == timerParam.timerHandle))
            {
                /* Update Led State */
                UpdateLedState();
                
                /* Indicate that timer is raised to the main loop */
                mainTimer++;
            }
            else
            {
                DBG_PRINTF("CY_BLE_EVT_TIMEOUT: %x \r\n", *(cy_en_ble_to_reason_code_t *)eventParam);
            }
            break;
            
        case CY_BLE_EVT_HARDWARE_ERROR:    /* This event indicates that some internal HW error has occurred. */
            DBG_PRINTF("Hardware Error: 0x%x \r\n", *(uint8_t *)eventParam);
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
            /* Hibernate */
            Cy_SysPm_Hibernate();
            break;
            
            
        /**********************************************************
        *                       GAP Events
        ***********************************************************/
        /* This event provides the remote device lists during discovery process. */
        case CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
            advReport = (cy_stc_ble_gapc_adv_report_param_t *)eventParam;
            /* Filter and connect only to nodes that advertise IPSS in ADV payload */
            if(CheckAdvPacketForServiceUuid(advReport, CY_BLE_UUID_INTERNET_PROTOCOL_SUPPORT_SERVICE) != 0u)
            {
                DBG_PRINTF("Advertisement report: eventType = %x, peerAddrType - %x, ", 
                    advReport->eventType, advReport->peerAddrType);
                DBG_PRINTF("peerBdAddr - ");
                for(newDevice = 1u, i = 0u; i < advDevices; i++)
                {
                    /* Compare device address with already logged one */
                    if((memcmp(peerAddr[i].bdAddr, advReport->peerBdAddr, CY_BLE_GAP_BD_ADDR_SIZE) == 0))
                    {
                        DBG_PRINTF("%x: ",i);
                        newDevice = 0u;
                        break;
                    }
                }
                if(newDevice != 0u)
                {
                    if(advDevices < CY_BLE_MAX_ADV_DEVICES)
                    {
                        memcpy(peerAddr[advDevices].bdAddr, advReport->peerBdAddr, CY_BLE_GAP_BD_ADDR_SIZE);
                        peerAddr[advDevices].type = advReport->peerAddrType;
                        DBG_PRINTF("%x: ",advDevices);
                        advDevices++;
                    }
                }
                for(i = CY_BLE_GAP_BD_ADDR_SIZE; i > 0u; i--)
                {
                    DBG_PRINTF("%2.2x", advReport->peerBdAddr[i-1]);
                }
                DBG_PRINTF(", rssi - %d dBm", advReport->rssi);
            #if(DEBUG_UART_FULL)  
                DBG_PRINTF(", data - ");
                for( i = 0; i < advReport->dataLen; i++)
                {
                    DBG_PRINTF("%2.2x ", advReport->data[i]);
                }
            #endif /* DEBUG_UART_FULL */
                DBG_PRINTF("\r\n");
            }
            break;
            
        case CY_BLE_EVT_GAPC_SCAN_START_STOP:
            DBG_PRINTF("CY_BLE_EVT_GAPC_SCAN_START_STOP, state: %x\r\n", Cy_BLE_GetScanState());
            if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_STOPPED)
            {
                if(state == STATE_CONNECTING)
                {
                    DBG_PRINTF("GAPC_END_SCANNING\r\n");
                    /* Connect to selected device */
                    apiResult = Cy_BLE_GAPC_ConnectDevice(&peerAddr[deviceN], 0u);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("ConnectDevice API Error: 0x%x \r\n", apiResult);
                    }
                }
                else
                {
                    /* Fast scanning period complete,
                     * go to low power mode (Hibernate) and wait for an external
                     * user event to wake up the device again */
                    UpdateLedState();
                    Cy_BLE_Stop();
                }
            }
            break;
            
        case CY_BLE_EVT_GAP_AUTH_REQ:
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_REQ: bdHandle=%x, security=%x, bonding=%x, ekeySize=%x, err=%x \r\n", 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bdHandle, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).security, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bonding, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).ekeySize, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).authErr);
            
            if(cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].security == 
                (CY_BLE_GAP_SEC_MODE_1 | CY_BLE_GAP_SEC_LEVEL_1))
            {
                cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].authErr = 
                    CY_BLE_GAP_AUTH_ERROR_PAIRING_NOT_SUPPORTED;
            }    
            
            cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].bdHandle = 
                ((cy_stc_ble_gap_auth_info_t *)eventParam)->bdHandle;

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
            
        case CY_BLE_EVT_GAP_PASSKEY_ENTRY_REQUEST:
            DBG_PRINTF("CY_BLE_EVT_PASSKEY_ENTRY_REQUEST press 'p' to enter passkey \r\n");
            break;
            
        case CY_BLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
            DBG_PRINTF("CY_BLE_EVT_PASSKEY_DISPLAY_REQUEST %6.6ld \r\n", *(uint32_t *)eventParam);
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
            
        case CY_BLE_EVT_GAP_AUTH_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_COMPLETE: bdHandle=%x, security=%x, bonding=%x, ekeySize=%x, err=%x \r\n", 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bdHandle, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).security, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bonding, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).ekeySize, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).authErr);
            break;
            
        case CY_BLE_EVT_GAP_AUTH_FAILED:
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_FAILED: bdHandle=%x, authErr=%x\r\n", 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bdHandle, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).authErr);
            break;
            
        case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
            connIntv = ((cy_stc_ble_gap_connected_param_t *)eventParam)->connIntv * 5u /4u;
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_CONNECTED: connIntv = %d ms %d \r\n", connIntv, 
                        ((cy_stc_ble_gap_connected_param_t *)eventParam)->status);
            keyInfo.SecKeyParam.bdHandle = (*(cy_stc_ble_gap_connected_param_t *)eventParam).bdHandle;
            apiResult = Cy_BLE_GAP_SetSecurityKeys(&keyInfo);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAP_SetSecurityKeys API Error: 0x%x \r\n", apiResult);
            }
            break;
            
        case CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
            connIntv = ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->connIntv * 5u /4u;
            DBG_PRINTF("CY_BLE_EVT_GAPC_CONNECTION_UPDATE_COMPLETE: %x, %x, %x, %x \r\n", 
                ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->status,
                ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->connIntv,
                ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->connLatency,
                ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->supervisionTO
            );
            break;
            
        case CY_BLE_EVT_GAP_KEYS_GEN_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GAP_KEYS_GEN_COMPLETE \r\n");
            keyInfo.SecKeyParam = (*(cy_stc_ble_gap_sec_key_param_t *)eventParam);
            Cy_BLE_GAP_SetIdAddress(&cy_ble_deviceAddress);
            break;
            
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_DISCONNECTED: bdHandle=%x, reason=%x, status=%x\r\n",
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).bdHandle, 
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).reason, 
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).status);
            
            apiResult = Cy_BLE_GAPC_StartScan(CY_BLE_SCANNING_FAST, 0u);                   /* Start Limited Discovery */
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("StartScan API Error: 0x%x \r\n", apiResult);
            }
            break;
            
        case CY_BLE_EVT_GAP_ENCRYPT_CHANGE:
            DBG_PRINTF("CY_BLE_EVT_GAP_ENCRYPT_CHANGE: %x \r\n", *(uint8_t *)eventParam);
            break;
            
        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CY_BLE_EVT_GATT_CONNECT_IND:
            appConnHandle = *(cy_stc_ble_conn_handle_t *)eventParam;
            DBG_PRINTF("CY_BLE_EVT_GATT_CONNECT_IND: %x, %x \r\n", 
                (*(cy_stc_ble_conn_handle_t *)eventParam).attId, 
                (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle);
            
            /* Send an L2CAP LE credit based connection request with a PSM set to LE_PSM_IPSP.
             * Once the peer responds, CY_BLE_EVT_L2CAP_CBFC_CONN_CNF 
             * event will come up on this device.
             */
            
            {
                cy_stc_ble_l2cap_cbfc_connection_info_t bfcConnParam =
                {
                    .mtu = CY_BLE_L2CAP_MTU,
                    .mps = CY_BLE_L2CAP_MPS,
                    .credit = LE_DATA_CREDITS_IPSP
                };
                
                /* L2CAP Channel parameters for the local device */
                cy_stc_ble_l2cap_cbfc_conn_req_info_t cbfcConnParam =
                {
                    .bdHandle = appConnHandle.bdHandle,
                    .connParam = bfcConnParam,
                    .localPsm = CY_BLE_L2CAP_PSM_LE_PSM_IPSP,
                    .remotePsm = CY_BLE_L2CAP_PSM_LE_PSM_IPSP
                 
                };
                
                apiResult = Cy_BLE_L2CAP_CbfcConnectReq(&cbfcConnParam);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_L2CAP_CbfcConnectReq API Error: 0x%x \r\n", apiResult);
                }
                else
                {
                    DBG_PRINTF("L2CAP channel connection request sent. \r\n");
                }
            }
            break;
            
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
           DBG_PRINTF("CY_BLE_EVT_GATT_DISCONNECT_IND: %x, %x \r\n", 
                (*(cy_stc_ble_conn_handle_t *)eventParam).attId, 
                (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle);
            break;
        
        case CY_BLE_EVT_GATTC_ERROR_RSP:
            DBG_PRINTF("GATT_ERROR_RSP: opcode: %x,  handle: %x,  errorcode: %x \r\n",
                ((cy_stc_ble_gatt_err_param_t *)eventParam)->errInfo.opCode,
                ((cy_stc_ble_gatt_err_param_t *)eventParam)->errInfo.attrHandle,
                ((cy_stc_ble_gatt_err_param_t *)eventParam)->errInfo.errorCode);
            break;
            
        /**********************************************************
        *                       L2CAP Events
        ***********************************************************/
        case CY_BLE_EVT_L2CAP_CBFC_CONN_CNF:
            l2capParameters = *((cy_stc_ble_l2cap_cbfc_conn_cnf_param_t *)eventParam);
            DBG_PRINTF("CY_BLE_EVT_L2CAP_CBFC_CONN_CNF: bdHandle=%d, lCid=%d, responce=%d", 
                l2capParameters.bdHandle,
                l2capParameters.lCid,
                l2capParameters.response);
            
            DBG_PRINTF(", connParam: mtu=%d, mps=%d, credit=%d\r\n", 
                l2capParameters.connParam.mtu,
                l2capParameters.connParam.mps,
                l2capParameters.connParam.credit);
            l2capConnected = true;
            break;

        case CY_BLE_EVT_L2CAP_CBFC_DISCONN_IND:
            DBG_PRINTF("CY_BLE_EVT_L2CAP_CBFC_DISCONN_IND: %d \r\n", *(uint16_t *)eventParam);
            l2capConnected = false;
            break;

        /* Following two events are required, to receive data */        
        case CY_BLE_EVT_L2CAP_CBFC_DATA_READ:
            {
                cy_stc_ble_l2cap_cbfc_rx_param_t *rxDataParam = (cy_stc_ble_l2cap_cbfc_rx_param_t *)eventParam;
                DBG_PRINTF("<- EVT_L2CAP_CBFC_DATA_READ: lCid=%d, result=%d, len=%d", 
                    rxDataParam->lCid,
                    rxDataParam->result,
                    rxDataParam->rxDataLength);
            #if(DEBUG_UART_FULL)  
                DBG_PRINTF(", data:");
                for(i = 0; i < rxDataParam->rxDataLength; i++)
                {
                    DBG_PRINTF("%2.2x", rxDataParam->rxData[i]);
                }
            #endif /* DEBUG_UART_FULL */
                DBG_PRINTF("\r\n");
                /* Data is received from Node, validate the content */
                if(memcmp(((uint8_t *)ipv6LoopbackBuffer), rxDataParam->rxData, L2CAP_MAX_LEN) != 0u)
                {
                    DBG_PRINTF("Wraparound failed \r\n");
                }
                else
                {
                    /* Send new Data packet to Node through IPSP channel  */
                    custom_command = '1';
                }
            }
            break;

        case CY_BLE_EVT_L2CAP_CBFC_RX_CREDIT_IND:
            {
                cy_stc_ble_l2cap_cbfc_low_rx_credit_param_t *rxCreditParam = 
                    (cy_stc_ble_l2cap_cbfc_low_rx_credit_param_t *)eventParam;
                cy_stc_ble_l2cap_cbfc_credit_info_t l2capCbfcCreditParam =
                {
                    .credit   = LE_DATA_CREDITS_IPSP,
                    .localCid = rxCreditParam->lCid
                };
                
                DBG_PRINTF("CY_BLE_EVT_L2CAP_CBFC_RX_CREDIT_IND: lCid=%d, credit=%d \r\n", 
                    rxCreditParam->lCid,
                    rxCreditParam->credit);

                /* This event informs that receive credits reached low mark. 
                 * If the device expects more data to receive, send more credits back to the peer device.
                 */
                apiResult = Cy_BLE_L2CAP_CbfcSendFlowControlCredit(&l2capCbfcCreditParam);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_L2CAP_CbfcSendFlowControlCredit API Error: 0x%x \r\n", apiResult);
                }
            }
            break;

        /* Following events are required, to send data */
        case CY_BLE_EVT_L2CAP_CBFC_TX_CREDIT_IND:
            DBG_PRINTF("CY_BLE_EVT_L2CAP_CBFC_TX_CREDIT_IND: lCid=%d, result=%d, credit=%d \r\n", 
                ((cy_stc_ble_l2cap_cbfc_low_tx_credit_param_t *)eventParam)->lCid,
                ((cy_stc_ble_l2cap_cbfc_low_tx_credit_param_t *)eventParam)->result,
                ((cy_stc_ble_l2cap_cbfc_low_tx_credit_param_t *)eventParam)->credit);
            break;
        
        case CY_BLE_EVT_L2CAP_CBFC_DATA_WRITE_IND:
            #if(DEBUG_UART_FULL)
            {
                cy_ble_l2cap_cbfc_data_write_param_t *writeDataParam = (cy_ble_l2cap_cbfc_data_write_param_t*)eventParam;
                DBG_PRINTF("CY_BLE_EVT_L2CAP_CBFC_DATA_WRITE_IND: lCid=%d \r\n", writeDataParam->lCid);
            }
            #endif /* DEBUG_UART_FULL */
            break;
            
        /**********************************************************
        *                       Discovery Events 
        ***********************************************************/
        case CY_BLE_EVT_GATTC_DISCOVERY_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_SERVER_DISCOVERY_COMPLETE \r\n");
            DBG_PRINTF("GATT %x-%x Char: %x, cccd: %x, \r\n", 
                cy_ble_serverInfo[Cy_BLE_GetDiscoveryIdx(*(cy_stc_ble_conn_handle_t *)eventParam)][CY_BLE_SRVI_GATT].
                    range.startHandle,
                cy_ble_serverInfo[Cy_BLE_GetDiscoveryIdx(*(cy_stc_ble_conn_handle_t *)eventParam)][CY_BLE_SRVI_GATT].
                    range.endHandle,
                cy_ble_discovery[Cy_BLE_GetDiscoveryIdx(*(cy_stc_ble_conn_handle_t *)eventParam)].
                    gattc.serviceChanged.valueHandle,
                cy_ble_discovery[Cy_BLE_GetDiscoveryIdx(*(cy_stc_ble_conn_handle_t *)eventParam)].
                    gattc.cccdHandle);

            DBG_PRINTF("\r\nIPSP %x-%x: ", 
                cy_ble_serverInfo[Cy_BLE_GetDiscoveryIdx(*(cy_stc_ble_conn_handle_t *)eventParam)][CY_BLE_SRVI_IPSS].
                    range.startHandle,
                cy_ble_serverInfo[Cy_BLE_GetDiscoveryIdx(*(cy_stc_ble_conn_handle_t *)eventParam)][CY_BLE_SRVI_IPSS].
                    range.endHandle);
  
            DBG_PRINTF("\r\n");
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
* Function Name: UpdateLedState
********************************************************************************
*
* Summary:
*  This function updates LED status based on current BLE state.
*
*******************************************************************************/
void UpdateLedState(void)
{

    if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_SCANNING)
    {
        Scanning_LED_INV();
    }
    else if(Cy_BLE_GetConnectionState(appConnHandle) == CY_BLE_CONN_STATE_DISCONNECTED)
    {   
        Scanning_LED_Write(LED_OFF);
    }
    else if(Cy_BLE_GetConnectionState(appConnHandle) >= CY_BLE_CONN_STATE_CONNECTED)
    {
        Scanning_LED_Write(LED_ON);
    }
    
}


/*******************************************************************************
* Function Name: HostMain
********************************************************************************
* 
* Summary:
*  Main function for the project.
*
* Theory:
*  The function starts BLE and UART components.
*  This function processes all BLE events and also implements the low power 
*  functionality.
*
*******************************************************************************/
int HostMain(void)
{
    cy_en_ble_api_result_t apiResult;
    char8 command;
        
    /* Initialize wakeup pin for Hibernate */
    Cy_SysPm_SetHibWakeupSource(CY_SYSPM_HIBPIN1_LOW);
    
    /* Initialize LEDs */
    DisableAllLeds();
        
    /* Initialize Debug UART */
    UART_START();
    DBG_PRINTF("BLE IPSP Router Example Project \r\n");
    
    /* Start BLE component and register generic event handler */
    Cy_BLE_Start(AppCallback);    
    
    /* Print stack version */
    PrintStackVersion();
    
    /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    while(1u)
    {
        /* Cy_BLE_ProcessEvents() allows BLE stack to process pending events */
        Cy_BLE_ProcessEvents();

        /* Restart timer */
        if(mainTimer != 0u)
        {
            mainTimer = 0u;
            Cy_BLE_StartTimer(&timerParam); 
        }      
        
        if(((command = UART_DEB_GET_CHAR()) != UART_DEB_NO_DATA) || ((custom_command != 0) && 
            (cy_ble_busyStatus[appConnHandle.attId] == 0u)))
        {
            if(custom_command != 0u)
            {
                command = custom_command;
                custom_command = 0u;
            }
            switch(command)
            {
                case 'c':                   /* Send connect request to selected peer device.  */
                    Cy_BLE_GAPC_StopScan(); 
                    state = STATE_CONNECTING;
                    break;
                    
                case 'v':                   /* Cancel connection request. */
                    apiResult = Cy_BLE_GAPC_CancelDeviceConnection();
                    DBG_PRINTF("Cy_BLE_GAPC_CancelDeviceConnection: %x\r\n" , apiResult);
                    break;
                    
                case 'd':                   /* Send disconnect request to peer device. */
                    {
                        cy_stc_ble_gap_disconnect_info_t disconnectInfoParam =
                        {
                            .bdHandle = appConnHandle.bdHandle,
                            .reason = CY_BLE_HCI_ERROR_OTHER_END_TERMINATED_USER
                        };
                        apiResult = Cy_BLE_GAP_Disconnect(&disconnectInfoParam); 
                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("DisconnectDevice API Error: 0x%x \r\n", apiResult);
                        }
                        state = STATE_DISCONNECTED;
                    }
                    break;
                    
                case 's':                   /* Start discovery procedure. */
                    apiResult = Cy_BLE_GATTC_StartDiscovery(appConnHandle);
                    DBG_PRINTF("StartDiscovery \r\n");
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("StartDiscovery API Error: 0x%x \r\n", apiResult);
                    }
                    break;
                    
                case 'z':                   /* Select specific peer device.  */
                    DBG_PRINTF("Select Device:\n"); 
                    while((command = UART_DEB_GET_CHAR()) == UART_DEB_NO_DATA);
                    if((command >= '0') && (command <= '9'))
                    {
                        deviceN = (uint8)(command - '0');
                        DBG_PRINTF("%c\n",command); /* print number */
                    }
                    else
                    {
                        DBG_PRINTF(" Wrong digit \r\n");
                        break;
                    }
                    break;
                    
                    /**********************************************************
                    *               L2Cap Commands (WrapAround)
                    ***********************************************************/
                case '1':                   /* Send Data packet to node through IPSP channel */
                    {
                        static uint16_t counter = 0;
                        static uint16_t repeats = 0;
                        uint16_t i;
                        cy_stc_ble_l2cap_cbfc_tx_data_info_t l2capCbfcTxDataParam;
                        
                        DBG_PRINTF("-> Cy_BLE_L2CAP_ChannelDataWrite #%d \r\n", repeats++);
                        (void)repeats;
                    #if(DEBUG_UART_FULL)  
                        DBG_PRINTF(", Data:");
                    #endif /* DEBUG_UART_FULL */
                        /* Fill output buffer by counter */
                        for(i = 0u; i < L2CAP_MAX_LEN / 2u; i++)
                        {
                            ipv6LoopbackBuffer[i] = counter++;
                        #if(DEBUG_UART_FULL)  
                            DBG_PRINTF("%4.4x", ipv6LoopbackBuffer[i]);
                        #endif /* DEBUG_UART_FULL */
                        }
                        l2capCbfcTxDataParam.buffer = (uint8_t *)ipv6LoopbackBuffer;
                        l2capCbfcTxDataParam.bufferLength = L2CAP_MAX_LEN; 
                        l2capCbfcTxDataParam.localCid = l2capParameters.lCid;
                        apiResult = Cy_BLE_L2CAP_ChannelDataWrite(&l2capCbfcTxDataParam);
                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("Cy_BLE_L2CAP_ChannelDataWrite API Error: 0x%x \r\n", apiResult);
                        }
                    }
                    break;
                    
                case 'h':                   /* Help menu */
                    DBG_PRINTF("\r\n");
                    DBG_PRINTF("Available commands:\r\n");
                    DBG_PRINTF(" \'h\' - Help menu.\r\n");
                    DBG_PRINTF(" \'z\' + 'Number' - Select peer device.\r\n");
                    DBG_PRINTF(" \'c\' - Send connect request to peer device.\r\n");
                    DBG_PRINTF(" \'d\' - Send disconnect request to peer device.\r\n");
                    DBG_PRINTF(" \'v\' - Cancel connection request.\r\n");
                    DBG_PRINTF(" \'s\' - Start discovery procedure.\r\n");
                    DBG_PRINTF(" \'1\' - Send Data packet to Node through IPSP channel.\r\n");
                    break;
            }
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


/* [] END OF FILE */
