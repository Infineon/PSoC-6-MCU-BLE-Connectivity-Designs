/*******************************************************************************
* File Name: host_main.c
*
* Version: 1.0
*
* Description:
*  This is source code for the BLE Alert Notification code example.
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include <project.h>
#include <stdio.h>
#include <math.h>
#include "common.h"
#include "user_interface.h"
    
/***************************************
*        Global Variables
***************************************/
cy_stc_ble_conn_handle_t        appConnHandle;

static volatile uint32_t        mainTimer        = 1u;
static cy_stc_ble_timer_info_t  timerParam = { .timeout = TIMER_TIMEOUT };  

static uint32_t                 appConnRole = 0xFF;
static volatile bool            skipScanProgress = false;
static cy_stc_ble_gap_bd_addr_t peerAddr[CY_BLE_MAX_ADV_DEVICES];
static uint32_t                 peerCnt;
static uint32_t                 peerForConn;
static bool                     connReq = false;

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
uint32_t CheckAdvPacketForServiceUuid(cy_stc_ble_gapc_adv_report_param_t *eventParam, uint16_t uuid)
{
    uint32_t servicePresent = NO; 
    uint32_t advIndex = 0u;
    uint32_t i;
    
    do
    {
        /* Find complete or incomplete Service UUID list field type */
        if((eventParam->data[advIndex + 1u] == (uint8_t)CY_BLE_GAP_ADV_INCOMPL_16UUID) || 
           (eventParam->data[advIndex + 1u] == (uint8_t)CY_BLE_GAP_ADV_COMPL_16UUID))
        {
            /* Compare uuid values with input parameter */
            for(i = 0u; (i < (eventParam->data[advIndex] - 1u)) && (servicePresent == NO); i += sizeof(uint16_t))
            {
                if(Cy_BLE_Get16ByPtr(&eventParam->data[advIndex + 2u + i]) == uuid)
                {
                    servicePresent = YES;
                }
            }
        }
        advIndex += eventParam->data[advIndex] + 1u;
    }while((advIndex < eventParam->dataLen) && (servicePresent == NO));    
    
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
*  event - The event code.
*  eventParam - The event parameters.
*
*******************************************************************************/
void AppCallBack(uint32_t event, void *eventParam)
{   
    uint16_t connIntv;   /* in milliseconds / 1.25ms */
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
            DBG_PRINTF("CY_BLE_EVT_STACK_ON\r\n");  

            /* Generates the security keys */
            apiResult = Cy_BLE_GAP_GenerateKeys(&keyInfo);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAP_GenerateKeys API Error: 0x%x \r\n", apiResult);
            }
                    
            /* Display Bond list */
            App_DisplayBondList();

            break;
            
        case CY_BLE_EVT_TIMEOUT: /* 0x01 -> GAP limited discoverable mode timeout */
                                 /* 0x02 -> GAP pairing process timeout */
                                 /* 0x03 -> GATT response timeout */
            if((((cy_stc_ble_timeout_param_t *)eventParam)->reasonCode == CY_BLE_GENERIC_APP_TO) && 
               (((cy_stc_ble_timeout_param_t *)eventParam)->timerHandle == timerParam.timerHandle))
            {
                /* Update Led State */
                UpdateLedState();
                
                /* Indicate that timer is raised to the main loop */
                mainTimer++;
                
                /* Press and hold the mechanical button (SW2) during 4 seconds to clear the bond list */
                App_RemoveDevicesFromBondListBySW2Press(SW2_PRESS_TIME_DEL_BOND_LIST);   
                
            }
            else
            {
                DBG_PRINTF("CY_BLE_EVT_TIMEOUT: %d \r\n", (((cy_stc_ble_timeout_param_t *)eventParam)->reasonCode));
            }       
            break;

        case CY_BLE_EVT_HARDWARE_ERROR:    /* This event indicates that some internal HW error has occurred */
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
                bool isNewAddr = true;
                bool addToList = false;
                if((advReport->eventType == 0u) && (skipScanProgress == false))
                {   
                    /* Filter and add to the connect list only nodes that advertise HRS in ADV payload */
                    if(CheckAdvPacketForServiceUuid(advReport, CY_BLE_UUID_ALERT_NOTIFICATION_SERVICE) == YES)
                    {      
                        addToList = true;
                        
                        /* Compare device address with already logged one */
                        for(i = 0u; i < CY_BLE_MAX_ADV_DEVICES; i++)
                        {
                            if(!(memcmp(&peerAddr[i].bdAddr, advReport->peerBdAddr, CY_BLE_GAP_BD_ADDR_SIZE)))
                            {
                                isNewAddr = false;
                            }
                        }
                        
                        /* Add device address to peerAddr */
                        if((isNewAddr == true) && (peerCnt < CY_BLE_MAX_ADV_DEVICES))
                        {
                            memcpy(&peerAddr[peerCnt].bdAddr, advReport->peerBdAddr, CY_BLE_GAP_BD_ADDR_SIZE); 
                            peerAddr[peerCnt].type = advReport->peerAddrType;
                            peerCnt++;  
                        }
                    }

                    /* Display scanned device ADV data */
                    if((addToList == true) && (isNewAddr == true))
                    {
                        DBG_PRINTF("\r\n-----------------------------------------------------------------------------\r\n");
                        DBG_PRINTF("uuid: ALERT NOTIFICATION SERVICE - YES, added to the connect list  \r\n");
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
                    if((addToList == true) && (isNewAddr == true))
                    {            
                        DBG_PRINTF("-----------------------------------------------------------------------------\r\n\r\n");
                    } 
                }
            }
            break;
                    
        case CY_BLE_EVT_GAPC_SCAN_START_STOP:
            DBG_PRINTF("CY_BLE_EVT_GAPC_SCAN_START_STOP\r\n");
            if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_STOPPED)
            {
                DBG_PRINTF("Scan complete! \r\n \r\n");
                
                if(connReq == true)
                {
                    connReq = false;                    
                    apiResult = Cy_BLE_GAPC_ConnectDevice(&peerAddr[peerForConn], CY_BLE_CENTRAL_CONFIGURATION_0_INDEX);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("ConnectDevice API Error: 0x%x ", apiResult);
                    }
                    else
                    {
                        DBG_PRINTF("Connecting to the device ");
                        /* Print the devices Addr of the detected Client */
                        DBG_PRINTF("%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x \r\n",
                                    peerAddr[peerForConn].bdAddr[5u], peerAddr[peerForConn].bdAddr[4u],
                                    peerAddr[peerForConn].bdAddr[3u], peerAddr[peerForConn].bdAddr[2u],
                                    peerAddr[peerForConn].bdAddr[1u], peerAddr[peerForConn].bdAddr[0u]); 
                    } 
                }
            }
            else
            {
                DBG_PRINTF("GAPC_START_SCANNING\r\n");
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
                  
            /* Check if connected as Central or Peripheral */
            if(((cy_stc_ble_gap_connected_param_t *)eventParam)->role == CY_BLE_GAP_LL_ROLE_SLAVE)
            {
                DBG_PRINTF("Connected as Peripheral (slave role)\r\n");
                appConnRole = CY_BLE_GAP_LL_ROLE_SLAVE;
            }
            else
            {
                DBG_PRINTF("Connected as Central (master role) \r\n");
                appConnRole = CY_BLE_GAP_LL_ROLE_MASTER;
            }
            
            /* Set security keys for new device which is not already bonded */
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
            connIntv = ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->connIntv * CY_BLE_CONN_INTRV_TO_MS;
            DBG_PRINTF("CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE: connIntv = %d ms \r\n", connIntv);
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
            
            /* Starts the Server Discovery process */
            if(appConnRole == CY_BLE_GAP_LL_ROLE_SLAVE)
            {
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
            
        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            DBG_PRINTF("CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP, state: %x\r\n", Cy_BLE_GetAdvertisementState());

            if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED)
            {
                DBG_PRINTF("Advertisement is disabled \r\n");
                /* Fast and slow advertising period complete, go to low power  
                 * mode (Hibernate) and wait for an external
                 * user event to wake up the device again */
                UpdateLedState();
                Cy_BLE_Stop();      
            }
            else
            {
                DBG_PRINTF("Advertisement is enabled \r\n");
            }
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
            break;
            
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            DBG_PRINTF("CY_BLE_EVT_GATT_DISCONNECT_IND: %x, %x \r\n", 
                        (*(cy_stc_ble_conn_handle_t *)eventParam).attId, (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle);
            
            AnsSetCurAnsCatId(ALL_CATEGORIES);
            
            if(appConnRole == CY_BLE_GAP_LL_ROLE_MASTER)
            {
                /* Starts the Scanning process after disconect */
                apiResult = Cy_BLE_GAPC_StartScan(CY_BLE_SCANNING_FAST, CY_BLE_CENTRAL_CONFIGURATION_0_INDEX);                   
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("StartScan API Error: %d \r\n", apiResult);
                }
            }
            else
            {
                /* Starts the Advertising process after disconect */
                apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAPP_StartAdvertisement API Error: %d \r\n", apiResult);
                }
            }
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
        }
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
* Function Name: MakeWordFromBytePtr
********************************************************************************
*
* Summary:
*   Converts array of two bytes, transferred over the BLE, to 16-bit word.
*
*******************************************************************************/
void MakeWordFromBytePtr(uint8_t bytePtr[], uint16_t *wordPtr)
{
    *wordPtr = (((uint16_t) bytePtr[1u]) << 8u) | ((uint16_t) bytePtr[0u]);
}


/*******************************************************************************
* Function Name: HostMain
********************************************************************************
*
* Summary:
*   Main function.
*
*******************************************************************************/
int HostMain(void)
{
    char8 command;
    uint8_t config[2u];
    uint8_t charVal[7u];
    uint8_t bCount;
    cy_en_ble_api_result_t apiResult;
        
    /* Initialization the user interface: LEDs, SW2, etc.  */
    InitUserInterface();  
    
    /* Initialize Debug UART */
    UART_START();
    DBG_PRINTF("BLE Alert Notification Example \r\n");

    /* Start BLE component and register generic event handler */
    apiResult = Cy_BLE_Start(AppCallBack);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_Start API Error: 0x%x \r\n", apiResult);
    }

    /* Print stack version */
    PrintStackVersion();
    
    /* Initialize BLE Services */
    /* Register the event handler for ANS specific events */
    Cy_BLE_ANS_RegisterAttrCallback(AnsServiceAppEventHandler);


    /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    while(1)
    {
        /* Cy_BLE_ProcessEvents() allows BLE stack to process pending events */
        Cy_BLE_ProcessEvents();
        
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
        
        /* Processing general commands */
        if(command != UART_DEB_NO_DATA)
        {
            switch(command)
            {
                case 'i':  /* Information about available commands */
                    DBG_PRINTF("Available commands:\r\n");
                    DBG_PRINTF(" \'q\' - Start advertising.\r\n");
                    DBG_PRINTF(" \'a\' - Start scanning.\r\n");
                    DBG_PRINTF(" \'c\' - Send a connect request to a peer device.\r\n");
                    DBG_PRINTF(" \'d\' - Send a disconnect request to a peer device.\r\n");
                    DBG_PRINTF(" \'f\' - Unbond all the devices.\r\n");
                    
                    if(appConnRole == CY_BLE_GAP_LL_ROLE_SLAVE)
                    {       
                        DBG_PRINTF("\r\nClient commands:\r\n");
                        DBG_PRINTF(" \'n\' - Turn off the LED of the New Alert Category that was notified previously. \r\n");
                   
                        DBG_PRINTF(" \'e\' - Send a request with the immediate notification command for the New Alert \r\n");
                        DBG_PRINTF("        Characteristic with the Category ID set to Email.\r\n");
                        
                        DBG_PRINTF(" \'m\' - Send a request with the immediate notification command for the New Alert \r\n");
                        DBG_PRINTF("        Characteristic with the Category ID set to Missed call.\r\n");
                        
                        DBG_PRINTF(" \'s\' - Send a request with the immediate notification command for the New Alert \r\n");
                        DBG_PRINTF("        Characteristic with the Category ID set to SMS/MMS.\r\n");
                        
                        DBG_PRINTF(" \'r\' - Read the Supported New Alert Category Characteristic. This command is required for the Client to\r\n");
                        DBG_PRINTF("        configure local supported categories setting prior sending a notification request to the Server.\r\n");
                                     
                        DBG_PRINTF(" \'t\' - Send the Enable New Alert Notification command to the Alert Notification Control Point \r\n");
                        DBG_PRINTF("        Characteristic. The category ID is set to All categories.\r\n");
                        
                        DBG_PRINTF(" \'o\' - Send the Disable New Alert Notification command to the Alert Notification Control Point \r\n");
                        DBG_PRINTF("        Characteristic. The category ID is set to All categories.\r\n");
                        
                        DBG_PRINTF(" \'0\' - Enable notifications for the New Alert Characteristic.\r\n");
                        DBG_PRINTF(" \'1\' - Disable notifications for the New Alert Characteristic.\r\n");
                    }
                    else if(appConnRole == CY_BLE_GAP_LL_ROLE_MASTER)
                    {
                        DBG_PRINTF("\r\nServer commands:\r\n");
                        DBG_PRINTF(" \'7\' - Send the Missed call notification.\r\n");
                        DBG_PRINTF(" \'8\' - Send the Email notification.\r\n");
                        DBG_PRINTF(" \'9\' - Send the SMS notification.\r\n");
                    }
                    break;
                                
                case 'c': /* Connect to the peer device */
                {
                    if(peerCnt != 0u) 
                    {
                        if(Cy_BLE_GetConnectionState(appConnHandle) < CY_BLE_CONN_STATE_CONNECTED)
                        {
                            uint32_t i;
                            
                            /* Print a list of scanned devices from the peerAddr array */
                            DBG_PRINTF("Detected device: \r\n");
                            for(i = 0u; i < peerCnt; i++)
                            {
                               /* Print the devices Addr of the detected Client */
                                DBG_PRINTF("Device %ld: %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x \r\n", i+1,
                                                peerAddr[i].bdAddr[5u], peerAddr[i].bdAddr[4u], 
                                                peerAddr[i].bdAddr[3u], peerAddr[i].bdAddr[2u],
                                                peerAddr[i].bdAddr[1u], peerAddr[i].bdAddr[0u]);         
                            }
                            
                            skipScanProgress = true;
                            
                            /* Select a device for connection */  
                            DBG_PRINTF("select device for connection:  (1..%lx):\r\n", peerCnt);        
                            while((command = UART_DEB_GET_CHAR()) == UART_DEB_NO_DATA)
                            {
                                Cy_BLE_ProcessEvents();
                            }
                            
                            if((command > '0') && (command <= (peerCnt + '0')))
                            {
                                /* Save index of selected device for connection */
                                peerForConn = (uint8_t)(command - '0') - 1;
                                
                                /* Set connection request flag */
                                connReq = true;
                                
                                /* Stop scanning before connection */
                                if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_SCANNING)
                                {
                                    Cy_BLE_GAPC_StopScan();
                                }
                            }
                            else
                            {
                                DBG_PRINTF(" Wrong digit \r\n");
                                break;
                            }
                            skipScanProgress = false;
                        }
                        else
                        {
                            DBG_PRINTF("You are already connected to peer device \r\n");
                        }
                    }
                    else
                    {
                        DBG_PRINTF("Connecting list is empty. Press 'a' to start scan \r\n");
                    }
                }
                break;
                          
                case 'f':                   /* Unbond all devices: set flag and go to disconnect section */ 
                    App_SetRemoveBondListFlag();
                
                case 'd':                   /* Disconnect */
                    if(Cy_BLE_GetNumOfActiveConn() != 0u)
                    {
                        cy_stc_ble_gap_disconnect_info_t disconnectInfoParam =
                        {
                            .bdHandle = appConnHandle.bdHandle,
                            .reason = CY_BLE_HCI_ERROR_OTHER_END_TERMINATED_USER
                        };
                        apiResult = Cy_BLE_GAP_Disconnect(&disconnectInfoParam); 

                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("Disconnect Device API Error: %x.\r\n", apiResult);
                        }
                    } 
                    break;
                    
                case 'a':                   /* Start Scan */
                    apiResult = Cy_BLE_GAPC_StartScan(CY_BLE_SCANNING_FAST, CY_BLE_CENTRAL_CONFIGURATION_0_INDEX);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_GAPC_StartScan API Error: %x \r\n", apiResult);
                    }
                    break;
                    
                case 'q':                   /* Start advertisement */
                    apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_GAPP_StartAdvertisement API Error: %d \r\n", apiResult);
                    }
                    break;   
                    
                default:
                    DBG_PRINTF("Unsupported command.\r\n");
                    break;
            }
        }
        
        /* Processing Client commands */
        if((command != UART_DEB_NO_DATA) && (appConnRole == CY_BLE_GAP_LL_ROLE_SLAVE) && 
           (Cy_BLE_GetConnectionState(appConnHandle) != CY_BLE_CONN_STATE_DISCONNECTED))
        {
            switch(command)
            {
            case 'n':
                AnsSetCurAnsCatId(ALL_CATEGORIES);  /* Reset New Alert Category */
                break;

            
            case 'e':
                charVal[ANCP_COMMAND_ID_INDEX] = CY_BLE_ANS_IMM_NEW_ALERT_NTF;
                charVal[ANCP_CATEGORY_ID_INDEX] = CY_BLE_ANS_CAT_ID_EMAIL;

                apiResult =
                    Cy_BLE_ANSC_SetCharacteristicValue(appConnHandle, CY_BLE_ANS_ALERT_NTF_CONTROL_POINT, 2u, charVal);

                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_ANSC_SetCharacteristicValue(CY_BLE_ANS_ALERT_NTF_CONTROL_POINT) routine Error: %d \r\n",
                                apiResult);
                }

                break;

            case 'm':
                charVal[ANCP_COMMAND_ID_INDEX] = CY_BLE_ANS_IMM_NEW_ALERT_NTF;
                charVal[ANCP_CATEGORY_ID_INDEX] = CY_BLE_ANS_CAT_ID_MISSED_CALL;

                apiResult =
                    Cy_BLE_ANSC_SetCharacteristicValue(appConnHandle, CY_BLE_ANS_ALERT_NTF_CONTROL_POINT, 2u, charVal);

                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_ANSC_SetCharacteristicValue(CY_BLE_ANS_ALERT_NTF_CONTROL_POINT) routine Error: %d \r\n",
                                apiResult);
                }
                else
                {
                    DBG_PRINTF("Cy_BLE_ANSC_SetCharacteristicValue(CY_BLE_ANS_ALERT_NTF_CONTROL_POINT) routine Success \r\n");
                }
                break;

            case 's':
                charVal[ANCP_COMMAND_ID_INDEX] = CY_BLE_ANS_IMM_NEW_ALERT_NTF;
                charVal[ANCP_CATEGORY_ID_INDEX] = CY_BLE_ANS_CAT_ID_SMS_MMS;

                apiResult =
                    Cy_BLE_ANSC_SetCharacteristicValue(appConnHandle, CY_BLE_ANS_ALERT_NTF_CONTROL_POINT, 2u, charVal);

                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_ANSC_SetCharacteristicValue(CY_BLE_ANS_ALERT_NTF_CONTROL_POINT) routine Error: %d \r\n",
                                apiResult);
                }
                else
                {
                    DBG_PRINTF("Cy_BLE_ANSC_SetCharacteristicValue(CY_BLE_ANS_ALERT_NTF_CONTROL_POINT) routine Success \r\n");
                }
                break;

            case 'r':

                apiResult = Cy_BLE_ANSC_GetCharacteristicValue(appConnHandle, CY_BLE_ANS_SUPPORTED_NEW_ALERT_CAT);

                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_ANSC_GetCharacteristicValue(CY_BLE_ANS_SUPPORTED_NEW_ALERT_CAT) routine Error: %d \r\n",
                                apiResult);
                }
                else
                {
                    DBG_PRINTF("Cy_BLE_ANSC_GetCharacteristicValue(CY_BLE_ANS_SUPPORTED_NEW_ALERT_CAT) routine Success \r\n");
                }
                break;

            case 't':
                charVal[ANCP_COMMAND_ID_INDEX] = CY_BLE_ANS_EN_NEW_ALERT_NTF;
                charVal[ANCP_CATEGORY_ID_INDEX] = CY_BLE_ANS_CAT_ID_ALL;

                apiResult =
                    Cy_BLE_ANSC_SetCharacteristicValue(appConnHandle, CY_BLE_ANS_ALERT_NTF_CONTROL_POINT, 2u, charVal);

                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_ANSC_SetCharacteristicValue(CY_BLE_ANS_ALERT_NTF_CONTROL_POINT) routine Error: %d \r\n",
                                apiResult);
                }
                else
                {
                    DBG_PRINTF("Cy_BLE_ANSC_SetCharacteristicValue(CY_BLE_ANS_ALERT_NTF_CONTROL_POINT) routine Success \r\n");
                }
                break;

            case 'o':
                charVal[ANCP_COMMAND_ID_INDEX] = CY_BLE_ANS_DIS_NEW_ALERT_NTF;
                charVal[ANCP_CATEGORY_ID_INDEX] = CY_BLE_ANS_CAT_ID_ALL;

                apiResult =
                    Cy_BLE_ANSC_SetCharacteristicValue(appConnHandle, CY_BLE_ANS_ALERT_NTF_CONTROL_POINT, 2u, charVal);

                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_ANSC_SetCharacteristicValue(CY_BLE_ANS_ALERT_NTF_CONTROL_POINT) routine Error: %d \r\n",
                        apiResult);
                }
                else
                {
                    DBG_PRINTF("Cy_BLE_ANSC_SetCharacteristicValue(CY_BLE_ANS_ALERT_NTF_CONTROL_POINT) routine Success \r\n");
                }
                break;

            case '0':     /* Set Notification enable bit in Client Characteristic Configuration
                          * Descriptor of New Alert Characteristic.
                          */
                config[0u] = 0x01u;
                config[1u] = 0x00u;

                apiResult = Cy_BLE_ANSC_SetCharacteristicDescriptor(appConnHandle,
                                                                    CY_BLE_ANS_NEW_ALERT,
                                                                    CY_BLE_ANS_CCCD,
                                                                    CY_BLE_CCCD_LEN,
                                                                    config);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_ANSC_SetCharacteristicDescriptor routine Error: %d \r\n", apiResult);
                }
                else
                {
                    DBG_PRINTF("Cy_BLE_ANSC_SetCharacteristicDescriptor routine Success \r\n");
                }
                break;

            case '1':     /* Set Notification enable bit in Client Characteristic Configuration
                          * Descriptor of New Alert Characteristic.
                          */

                    config[0u] = 0x00u;
                    config[1u] = 0x00u;

                    apiResult = Cy_BLE_ANSC_SetCharacteristicDescriptor(appConnHandle,
                                                         CY_BLE_ANS_NEW_ALERT,
                                                         CY_BLE_ANS_CCCD,
                                                         CY_BLE_CCCD_LEN,
                                                         config);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_ANSC_SetCharacteristicDescriptor routine Error: %d \r\n", apiResult);
                    }
                    else
                    {
                        DBG_PRINTF("Cy_BLE_ANSC_SetCharacteristicDescriptor routine Success \r\n");
                    }
                break;   
            
            default:
                DBG_PRINTF("Unsupported command.\r\n");
                break;

            }
        }
        
        /* Processing Server commands */
        if((command != UART_DEB_NO_DATA) && (appConnRole == CY_BLE_GAP_LL_ROLE_MASTER) && 
           (Cy_BLE_GetConnectionState(appConnHandle) != CY_BLE_CONN_STATE_DISCONNECTED))
        {
            switch(command)
            {
            case '7':    /* Missed Call Notification */
                
                bCount = 0;
                charVal[bCount++] = CY_BLE_ANS_CAT_ID_MISSED_CALL;
                charVal[bCount++] = 1u;

                AnsSetNewAletCount(CY_BLE_ANS_CAT_ID_MISSED_CALL, charVal[1u]);
                
                apiResult = AnsSendNotification(CY_BLE_ANS_NEW_ALERT, bCount, charVal);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("AnsSendNotification API error: 0x%x \r\n", apiResult);
                }
                break;
                
            case '8':    /* Email Notification */
                
                bCount = 0;
                charVal[bCount++] = CY_BLE_ANS_CAT_ID_EMAIL;
                charVal[bCount++] = 5u;
                charVal[bCount++] = 'H';
                charVal[bCount++] = 'e';
                charVal[bCount++] = 'l';
                charVal[bCount++] = 'l';
                charVal[bCount++] = 'o';

                AnsSetNewAletCount(CY_BLE_ANS_CAT_ID_EMAIL, charVal[1u]);

                apiResult = AnsSendNotification(CY_BLE_ANS_NEW_ALERT, bCount, charVal);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("AnsSendNotification API error: 0x%x \r\n", apiResult);
                }
                break;
                
            case '9':    /* SMS Notification */
                
                bCount = 0;
                charVal[bCount++] = CY_BLE_ANS_CAT_ID_SMS_MMS;
                charVal[bCount++] = 2u;
                charVal[bCount++] = ':';
                charVal[bCount++] = ')';

                AnsSetNewAletCount(CY_BLE_ANS_CAT_ID_SMS_MMS, charVal[1u]);
                
                apiResult = AnsSendNotification(CY_BLE_ANS_NEW_ALERT, bCount, charVal);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("AnsSendNotification API error: 0x%x \r\n", apiResult);
                }
                break;

            default:
                DBG_PRINTF("Unsupported command.\r\n");
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
