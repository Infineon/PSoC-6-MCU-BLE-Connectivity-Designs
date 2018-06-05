/*******************************************************************************
* File Name: host_main.c
*
* Version: 1.0
*
* Description:
* This project is the pair to the multiple BLE Battery Level (BAS GATT server)
* projects to demonstrate the Multiple Master and Single Slave mode simultaneous
* operation. The Multi Master Single Slave project utilizes 3 BLE Central
* connections and 1 Peripheral connection. The Central is configured as a BAS GATT
* client to communicate with a peer BAS GATT Server (use the existing BAS 
* example project or an application to simulate a BAS GATT Server as a peer device).
* The Peripheral instantiates 3 instances of the BAS GATT Server. These state 
* corresponds to the Battery state of Peripherals to which the device is connected.
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
#include "user_interface.h"
#include "bas.h"


static app_stc_connection_info_t    appConnInfo;
static app_stc_scan_device_list_t   appScanDevInfo;

static volatile uint32_t            mainTimer     = 1u;
static volatile uint32_t            sleepTimer    = 0u;

/* Private Function Prototypes */
static uint32_t CheckAdvPacketForServiceUuid(cy_stc_ble_gapc_adv_report_param_t *eventParam, uint16_t uuid);
static uint8_t GetFreePerephiralIndex(void);
static uint8_t GetFreeCentralIndex(void);

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
*
*******************************************************************************/
void AppCallBack(uint32_t event, void* eventParam)
{
    cy_en_ble_api_result_t          apiResult;  
    cy_stc_ble_conn_handle_t        connHandle;
    uint32_t                        discIdx;
    uint32_t                        i;

    switch (event)
    {
        /**********************************************************
        *                       General Events
        ***********************************************************/
        case CY_BLE_EVT_STACK_ON: /* This event is received when the component is started */
            DBG_PRINTF("CY_BLE_EVT_STACK_ON, StartAdvertisement \r\n");
            
            /* Enter into discoverable mode so that remote can find it. */
            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAPP_StartAdvertisement API Error: 0x%x \r\n", apiResult);
            }
            
            /* Print the operation menu */
            PrintMenu();          
            break;
            
        case CY_BLE_EVT_TIMEOUT:
            DBG_PRINTF("CY_BLE_EVT_TIMEOUT Error \r\n");
            break;
            
        case CY_BLE_EVT_HARDWARE_ERROR:    /* This event indicates that an internal HW error occurred. */
            DBG_PRINTF("Hardware Error \r\n");
            ShowError();
            break;
        
        /* This event will be triggered by the host stack in both cases: the BLE stack is busy or not busy.
         *  The parameter that corresponds to this event will be the state of the BLE stack.
         *  The BLE stack is busy = CY_BLE_STACK_STATE_BUSY,
         *  The BLE stack is not busy = CY_BLE_STACK_STATE_FREE.
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
            DBG_PRINTF("Hibernate\r\n");
            UART_DEB_WAIT_TX_COMPLETE();
            /* Hibernate */
            Cy_SysPm_Hibernate();   
            break;
            
        /**********************************************************
        *                       GAP Events
        ***********************************************************/
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
        
        case CY_BLE_EVT_GAP_AUTH_FAILED:
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_FAILED: bdHandle=%x, authErr=%x\r\n", 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bdHandle, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).authErr);
            break;
            
        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            DBG_PRINTF("CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP, state: %d \r\n", Cy_BLE_GetAdvertisementState());
            break;
            
        case CY_BLE_EVT_GAPC_SCAN_START_STOP:
            DBG_PRINTF("CY_BLE_EVT_GAPC_SCAN_START_STOP\r\n");
            if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_STOPPED)
            {
                DBG_PRINTF("Scan complete! \r\n \r\n");
                
            }
            else
            {
                DBG_PRINTF("GAPC_START_SCANNING\r\n");
            }
            break;
        
        case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
        {
            bool sentGapAuthReqFlag = false;        
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_CONNECTED: status = %x,  connIntv = %d ms \r\n",
                        ((cy_stc_ble_gap_connected_param_t *)eventParam)->status,
                        ((cy_stc_ble_gap_connected_param_t *)eventParam)->connIntv * 5u /4u); /* in milliseconds / 1.25ms */

            if(((cy_stc_ble_gap_connected_param_t *)eventParam)->status == 0u)
            {
                connHandle = Cy_BLE_GetConnHandleByBdHandle(((cy_stc_ble_gap_connected_param_t *)eventParam)->bdHandle);
                
                /* Check if connected as Central or Peripheral */
                if(((cy_stc_ble_gap_connected_param_t *)eventParam)->role == CY_BLE_GAP_LL_ROLE_SLAVE)
                {
                    /* Connected as Peripheral (Slave role) */    
                    DBG_PRINTF("Connected as Peripheral (slave role) %x \r\n",appConnInfo.peripheralCnt);
                    if((appConnInfo.peripheralCnt < CY_BLE_MAX_PEREPHIRAL_CONN_NUM) &&
                       (GetFreePerephiralIndex() != INVALID_INDEX))
                    {
                        appConnInfo.peripheralCnt++;
                        appConnInfo.peripheral[GetFreePerephiralIndex()].connHandle = connHandle;
                    }
                     
                    /* Start advertising if we can connect more Slave peers */            
                    if(appConnInfo.peripheralCnt < CY_BLE_MAX_PEREPHIRAL_CONN_NUM)
                    {
                        apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, 
                                                                   CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("StartAdvertisement API Error: 0x%x \r\n", apiResult);
                            ShowError();
                        }
                    }
                }
                else
                {
                    /* Connected as Central (Master role) */    
                    if((appConnInfo.centralCnt < CY_BLE_MAX_CENTRAL_CONN_NUM) && (GetFreeCentralIndex() != INVALID_INDEX))
                    {
                        DBG_PRINTF("connected as Central connHandle %x\r\n",connHandle.bdHandle);
                        appConnInfo.centralCnt++;
                        appConnInfo.central[GetFreeCentralIndex()].connHandle = connHandle;
                        sentGapAuthReqFlag = true;
                    }                
                }
                if(sentGapAuthReqFlag == true)
                {
                    DBG_PRINTF("Send an authorization request: bdHandler 0x%x \r\n", 
                                    ((cy_stc_ble_gap_connected_param_t *)eventParam)->bdHandle);
                    /* Send an authorization request */
                    cy_ble_authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].bdHandle = 
                        ((cy_stc_ble_gap_connected_param_t *)eventParam)->bdHandle;
                        
                    apiResult = Cy_BLE_GAP_AuthReq(&cy_ble_authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_GAP_AuthReq API Error: 0x%x \r\n", apiResult);
                    }
                } 
            }
           
        } 
        break;
        
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_DISCONNECTED: bdHandle=%x, reason=%x, status=%x\r\n",
               (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).bdHandle, 
               (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).reason, 
               (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).status);
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

            /* Start discovery only for the Central connection */
            for(i = 0; i < CY_BLE_MAX_CENTRAL_CONN_NUM; i++)
            {
                uint8_t bdHandle = ((cy_stc_ble_gap_auth_info_t *)eventParam)->bdHandle;                
                if(appConnInfo.central[i].connHandle.bdHandle == bdHandle) 
                {
                    DBG_PRINTF("Cy_BLE_GATTC_StartDiscovery \r\n");
                    apiResult = Cy_BLE_GATTC_StartDiscovery(appConnInfo.central[i].connHandle);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("StartDiscovery API Error: 0x%x \r\n", apiResult);
                    }
                    break;
                }
            }
            break;

        case CY_BLE_EVT_GAP_ENCRYPT_CHANGE:
            DBG_PRINTF("CY_BLE_EVT_GAP_ENCRYPT_CHANGE: %x \r\n", *(uint8_t *)eventParam);
            break;
            
        case CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
        {
            cy_stc_ble_gapc_adv_report_param_t  *advReport = (cy_stc_ble_gapc_adv_report_param_t *)eventParam;
            
            bool printHeader  = false;
            bool isNewAddress = true;
                            
            if((appScanDevInfo.pauseScanProgress == false) && (Cy_BLE_IsPeerConnected(advReport->peerBdAddr) == false) &&
               (advReport->eventType == CY_BLE_GAPC_CONN_UNDIRECTED_ADV))
            {
                /* Filter and add to the connect list only nodes that advertise HRS in ADV payload */
                if((CheckAdvPacketForServiceUuid(advReport, CY_BLE_UUID_BAS_SERVICE) == true) &&
                   (appScanDevInfo.count < CY_BLE_MAX_SCAN_DEVICES))
                {
                    /* Detected devices are stored in appScanDevInfo.address[] array. User can review this list 
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
                    DBG_PRINTF("uuid: BAS SERVICE - YES, added to the connect list  \r\n");
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
                    
        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CY_BLE_EVT_GATT_CONNECT_IND:         
            DBG_PRINTF("CY_BLE_EVT_GATT_CONNECT_IND: attId=%x, bdHandle=%x \r\n", 
                (*(cy_stc_ble_conn_handle_t *)eventParam).attId, 
                (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle);
            break;
                
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            {
                cy_stc_ble_conn_handle_t *connHandle;        
                uint32_t isCentral;
                
                DBG_PRINTF("CY_BLE_EVT_GATT_DISCONNECT_IND: attId=%x, bdHandle=%x \r\n", 
                                (*(cy_stc_ble_conn_handle_t *)eventParam).attId, 
                                (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle);
                
                for(i = 0u; i < CY_BLE_MAX_CONN_NUM; i++)
                {
                    isCentral = (i < CY_BLE_MAX_CENTRAL_CONN_NUM) ? true : false;
                    
                    connHandle = (isCentral == true) ? &appConnInfo.central[i].connHandle : 
                                                      &appConnInfo.peripheral[i - CY_BLE_MAX_CENTRAL_CONN_NUM].connHandle; 
                   
                    if(connHandle->bdHandle == (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle)
                    {
                        connHandle->bdHandle = CY_BLE_INVALID_CONN_HANDLE_VALUE;
                        connHandle->attId    = CY_BLE_INVALID_CONN_HANDLE_VALUE;
                        
                        if(isCentral == true)
                        {
                            appConnInfo.centralCnt--;
                        }
                        else
                        {
                            appConnInfo.peripheralCnt--;
                            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST,
                                                                     CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
                            if(apiResult != CY_BLE_SUCCESS)
                            {
                                DBG_PRINTF("StartAdvertisement API Error: 0x%x \r\n", apiResult);
                                ShowError();
                            }
                        }
                        break;
                    }
                }
            }
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
        
        case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
            /* Triggered on the Server's side when the Client sends a Read request and when    
            * the characteristic has the CY_BLE_GATT_DB_ATTR_CHAR_VAL_RD_EVENT property set.
            * This event can be ignored by the application except when an error response                
            * is needed and it must be set in the gattErrorCode field 
            * of the event parameter. */
            
            DBG_PRINTF("CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ %x %x: handle: %x \r\n", 
                ((cy_stc_ble_gatts_char_val_read_req_t *)eventParam)->connHandle.attId,
                ((cy_stc_ble_gatts_char_val_read_req_t *)eventParam)->connHandle.bdHandle,
                ((cy_stc_ble_gatts_char_val_read_req_t *)eventParam)->attrHandle);
            break;
                
        case CY_BLE_EVT_GATTC_DISCOVERY_COMPLETE:
            {
                uint32_t basCnt = 0u;
                uint32_t basServSel = 0u;
                uint16_t cccd = CY_BLE_CCCD_NOTIFICATION;
                
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
                    
                    if((cy_ble_serverInfo[discIdx][i].uuid == CY_BLE_UUID_BAS_SERVICE) && 
                       (cy_ble_serverInfo[discIdx][i].range.startHandle != 0u))
                    {
                        basCnt++;   
                    }       
                }
                
                /* Check if Bas services is available */
                basServSel = ((CLIENT_BAS_SERVICE_IDX + 1u) <= basCnt)? CLIENT_BAS_SERVICE_IDX : 0u;

                /* Enable notification for the connected BAS Client */
                DBG_PRINTF("Enable notifications for BAS service [%lu]:  \r\n", basServSel);

                apiResult = Cy_BLE_BASC_SetCharacteristicDescriptor(*(cy_stc_ble_conn_handle_t *)eventParam, basServSel, 
                                                                    CY_BLE_BAS_BATTERY_LEVEL, 
                                                                    CY_BLE_BAS_BATTERY_LEVEL_CCCD,
                                                                    sizeof(cccd), (uint8 *)&cccd);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_BASC_SetCharacteristicDescriptor() failed. Error code: %d \r\n", apiResult);
                }
                else
                {
                    DBG_PRINTF("Cy_BLE_BASC_SetCharacteristicDescriptor() successful. \r\n");
                }
                DBG_PRINTF("\r\n");
            }
            break;
            
        
        case CY_BLE_EVT_GATTC_DISC_SKIPPED_SERVICE:
            DBG_PRINTF("CY_BLE_EVT_GATTC_DISC_SKIPPED_SERVICE: ");
           
            switch((*(cy_stc_ble_disc_srv_info_t *)eventParam).uuidFormat)
            {
                case CY_BLE_GATT_16_BIT_UUID_FORMAT:
                    DBG_PRINTF("16_BIT_UUID: %x", (*(cy_stc_ble_disc_srv_info_t *)eventParam).srvcInfo->uuid.uuid16);
                    break;
                
                case CY_BLE_GATT_128_BIT_UUID_FORMAT:
                    DBG_PRINTF("128_BIT_UUID:");
                    {
                        uint32_t i;
                        for(i=0; i < CY_BLE_GATT_128_BIT_UUID_SIZE; i++)
                        {
                            DBG_PRINTF("%x ", (*(cy_stc_ble_disc_srv_info_t *)eventParam).srvcInfo->uuid.uuid128.value[i]);
                        }
                    }                   
                    break;
            }
            DBG_PRINTF("\r\n");
            break;
           
        case CY_BLE_EVT_GATTC_ERROR_RSP:
            DBG_PRINTF("opCode: %x,  errorCode: %x,\r\n", 
                            ((cy_stc_ble_gatt_err_param_t *)eventParam)->errInfo.opCode,
                            ((cy_stc_ble_gatt_err_param_t *)eventParam)->errInfo.errorCode);
            break;
            
        case CY_BLE_EVT_GATTC_READ_RSP:
            DBG_PRINTF("CY_BLE_EVT_GATTC_READ_RSP: ");
            for(i = 0; i < ((cy_stc_ble_gattc_read_rsp_param_t *)eventParam)->value.len; i++)
            {
                DBG_PRINTF("%2.2x ", ((cy_stc_ble_gattc_read_rsp_param_t *)eventParam)->value.val[i]);
            }
            DBG_PRINTF("\r\n");
            break;

        case CY_BLE_EVT_GATTC_WRITE_RSP:
            DBG_PRINTF("CY_BLE_EVT_GATTC_WRITE_RSP: ");
            DBG_PRINTF("bdHandle: 0x%2.2x\r\n", ((cy_stc_ble_conn_handle_t *)eventParam)->bdHandle);
            break;

        case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
            DBG_PRINTF("CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF\r\n");
            break;
            
        case CY_BLE_EVT_GATTC_HANDLE_VALUE_IND:
            DBG_PRINTF("CY_BLE_EVT_GATTC_HANDLE_VALUE_IND\r\n");
            break;
            
        case CY_BLE_EVT_GATTC_EXEC_WRITE_RSP:
            DBG_PRINTF("CY_BLE_EVT_GATTC_EXEC_WRITE_RSP\r\n");
            break;
        
        case CY_BLE_EVT_GATTS_INDICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_GATTS_INDICATION_ENABLED\r\n");
            break;
            
        case CY_BLE_EVT_GATTS_INDICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_GATTS_INDICATION_DISABLED\r\n");
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
            /* Inform the application that a flash Write is pending. The Stack internal data 
            * structures are modified and must be stored in the flash using
            * Cy_BLE_StoreBondingData() */
            DBG_PRINTF("CY_BLE_EVT_PENDING_FLASH_WRITE\r\n");
            break;
            
        case CY_BLE_EVT_LE_PING_AUTH_TIMEOUT:
            DBG_PRINTF("CY_BLE_EVT_LE_PING_AUTH_TIMEOUT\r\n");
            break;
            
        default:
            DBG_PRINTF("Other event: 0x%lx \r\n", event);
            break;
    }
}


/*******************************************************************************
* Function Name: Connect
********************************************************************************
*
* Summary:
*   Connects to a peer device.
*
* Parameters:
*  \param peerAddrType: The type of the peer address.
*  \param peerBdAddr: The pointer to the peer address array
*
*******************************************************************************/
void Connect(uint8_t peerAddrType, uint8_t *peerBdAddr)
{
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_gap_bd_addr_t bdAddr = {.type = peerAddrType };    
    if(appConnInfo.centralCnt >= CY_BLE_MAX_CENTRAL_CONN_NUM)
    {
        DBG_PRINTF("MAX peers(%x) are connected \r\n", CY_BLE_MAX_CENTRAL_CONN_NUM);
    }
    else
    {
        memcpy(&bdAddr.bdAddr, peerBdAddr, CY_BLE_GAP_BD_ADDR_SIZE); 
        apiResult = Cy_BLE_GAPC_ConnectDevice(&bdAddr, CY_BLE_CENTRAL_CONFIGURATION_0_INDEX);
        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("ConnectDevice API Error: 0x%x ", apiResult);
        }
        else
        {
            DBG_PRINTF("Connecting to the device ");
        } 
        DBG_PRINTF("(address  - %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x) \r\n", bdAddr.bdAddr[5u], bdAddr.bdAddr[4u],
                                                                       bdAddr.bdAddr[3u], bdAddr.bdAddr[2u], 
                                                                       bdAddr.bdAddr[1u], bdAddr.bdAddr[0u]);         
    }
}

/*******************************************************************************
* Function Name: Disconnect
********************************************************************************
*
* Summary:
*   Disconnects a peer device.
*
* Parameters:
*   \param connHandle: The connection handle of a peer device.
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
    
    DBG_PRINTF("Cy_BLE_GAP_Disconnect param: bdHandle:%x, reason:%x \r\n", param.bdHandle, 
                                                                         param.reason);
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
* Function Name: GetFreeCentralIndex
********************************************************************************
*
* Summary:
*  Returns a free index of a central info structure.
*
*******************************************************************************/
static uint8_t GetFreeCentralIndex(void)
{ 
    uint32_t freeIndex = INVALID_INDEX;
    uint32_t i;
    
    for(i = 0u; i < CY_BLE_MAX_CENTRAL_CONN_NUM; i++)
    {
        if(Cy_BLE_GetConnectionState(appConnInfo.central[i].connHandle) == CY_BLE_CONN_STATE_DISCONNECTED)
        {
            freeIndex = i;
            break;
        }
    }
    return(freeIndex);
}


/*******************************************************************************
* Function Name: GetFreePerephiralIndex
********************************************************************************
*
* Summary:
*   Returns a free index of a peripheral info structure.
*
*******************************************************************************/
static uint8_t GetFreePerephiralIndex(void)
{ 
    uint32_t freeIndex = INVALID_INDEX;
    uint32_t i;
    
    for(i = 0u; i < CY_BLE_MAX_PEREPHIRAL_CONN_NUM; i++)
    {
        if(Cy_BLE_GetConnectionState(appConnInfo.peripheral[i].connHandle) == CY_BLE_CONN_STATE_DISCONNECTED)
        {
            freeIndex = i;
            break;
        }
    }
    return(freeIndex);
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
*   Sets the device to the Hibernate mode.
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
            DBG_PRINTF("Hibernate... \r\n");
            Cy_BLE_Stop();
            sleepTimer = 0u;
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
* Summary:
*  The main function for the project.
*
* Theory:
*  The function starts the BLE and UART components.
*  This function processes all BLE events and also implements the low power 
*  functionality.
*
*******************************************************************************/
int HostMain(void)
{
    cy_en_ble_api_result_t   apiResult;
    char8                    command;
    
    bool                     isCentral = false;
    uint32_t                 i;
    
    /* Initialize connHandle of the appConnInfo structure for Central and Peripheral  */
    for(i = 0u; i < CY_BLE_MAX_CONN_NUM; i++)
    {  
        cy_stc_ble_conn_handle_t *connHandlePtr;
        isCentral = (i < CY_BLE_MAX_CENTRAL_CONN_NUM) ? true : false;
        connHandlePtr = (isCentral == true) ? &appConnInfo.central[i].connHandle : 
                                              &appConnInfo.peripheral[i - CY_BLE_MAX_CENTRAL_CONN_NUM].connHandle; 
            connHandlePtr->bdHandle = CY_BLE_INVALID_CONN_HANDLE_VALUE;
            connHandlePtr->attId    = CY_BLE_INVALID_CONN_HANDLE_VALUE;
    }
    
    /* Initialization the user interface: LEDs, SW2, ect.  */
    InitUserInterface();
    
    /* Initialize Debug UART */
    UART_START();
    DBG_PRINTF("BLE Multi Master Single Slave Example \r\n");
             
    /* Start the BLE component and register the generic event handler */
    apiResult = Cy_BLE_Start(AppCallBack);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_Start API Error: 0x%x \r\n", apiResult);
    }

    /* Print stack version */
    PrintStackVersion();

    /* Initialize BLE Services */
    BasInit();
     
    /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    while(1u) 
    {   
        /* Cy_BLE_ProcessEvents() allows the BLE stack to process pending events */
        Cy_BLE_ProcessEvents();
        
        /* Set device in Hibernate mode if advertisement and scan are stopped and sleepTimer counted to TIMER_SLEEP_MODE */
        Hibernate();
        
        /* Restart the timer */
        if(mainTimer != 0u)
        {
            mainTimer = 0u;
        }  
        
        /* Process Battery Data from peer Servers and sending to the Client */   
        ProcessingBatteryData();
                        
        /* Process command from debug terminal */
        if((command = UART_DEB_GET_CHAR()) != UART_DEB_NO_DATA) 
        {
            ProcessUartCommands(command);     
        }
        
        /* Connect to peer device */
        if((appScanDevInfo.connReq != 0u) && (Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_STOPPED))
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
                appScanDevInfo.connReq = 0;
            } 
        }
    }  
}

    
/* [] END OF FILE */

