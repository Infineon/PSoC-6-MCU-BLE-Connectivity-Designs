/*******************************************************************************
* File Name: host_main.c
*
* Version: 1.0
*
* Description:
*  This is source code for the BLE Apple Notification Center code example.
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
* 
******************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.
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
#include "ancsc.h"
#include "common.h"
#include <stdbool.h>
#include "systick\cy_systick.h"
#include "user_interface.h"


/* Global Variables */
cy_stc_ble_conn_handle_t        appConnHandle;
static volatile uint32_t        mainTimer  = 1u;
static cy_stc_ble_timer_info_t  timerParam = { .timeout = ADV_TIMER_TIMEOUT };



/*******************************************************************************
* Function Name: AppCallBack
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component.
*
* Parameters:
*  uint8_t event:     Event from the BLE component.
*  void* eventParams: A structure instance for corresponding event type. The
*                     list of event structure is described in the component
*                     datasheet.
*
*******************************************************************************/
void AppCallBack(uint32_t event, void *eventParam)
{
    uint16_t i;
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_gap_auth_info_t *authInfo;
    
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
    uint8_t discIdx = Cy_BLE_GetDiscoveryIdx(appConnHandle);

    switch(event)
    {
        /**********************************************************
        *                       Generic and HCI Events
        ***********************************************************/
        case CY_BLE_EVT_STACK_ON: /* This event received when component is started */
            DBG_PRINTF("CY_BLE_EVT_STACK_ON, StartAdvertisement \r\n");  

            /* Start advertisement */
            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAPP_StartAdvertisement API Error: %d \r\n", apiResult);
            }
                    
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
        case CY_BLE_EVT_GAP_AUTH_REQ:
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_REQ: security=0x%x, bonding=0x%x, ekeySize=0x%x, err=0x%x \r\n",
                    (*(cy_stc_ble_gap_auth_info_t *)eventParam).security,
                    (*(cy_stc_ble_gap_auth_info_t *)eventParam).bonding,
                    (*(cy_stc_ble_gap_auth_info_t *)eventParam).ekeySize,
                    (*(cy_stc_ble_gap_auth_info_t *)eventParam).authErr);
            if(cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].security == (CY_BLE_GAP_SEC_MODE_1 | CY_BLE_GAP_SEC_LEVEL_1))
            {
                cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].authErr = CY_BLE_GAP_AUTH_ERROR_PAIRING_NOT_SUPPORTED;
            }    
            
            cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].bdHandle = ((cy_stc_ble_gap_auth_info_t *)eventParam)->bdHandle;

            apiResult = Cy_BLE_GAPP_AuthReqReply(&cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);            
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAPP_AuthReqReply API Error: %d, call Cy_BLE_GAP_RemoveOldestDeviceFromBondedList\r\n", apiResult);
                apiResult = Cy_BLE_GAP_RemoveOldestDeviceFromBondedList();
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAP_RemoveOldestDeviceFromBondedList API Error: %d \r\n", apiResult);
                }
                else
                {
                    apiResult = Cy_BLE_GAPP_AuthReqReply(&cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);            
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_GAPP_AuthReqReply API Error: %d \r\n", apiResult);
                    }
                }
            }
            break;
            
        case CY_BLE_EVT_GAP_PASSKEY_ENTRY_REQUEST:
            DBG_PRINTF("CY_BLE_EVT_GAP_PASSKEY_ENTRY_REQUEST press 'p' to enter passkey.\r\n");
            break;

        case CY_BLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
            DBG_PRINTF("CY_BLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST. Passkey is: %6.6ld.\r\n", *(uint32_t *)eventParam);
            DBG_PRINTF("Please enter the passkey on your Server device.\r\n");
            break;

        case CY_BLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO:
            DBG_PRINTF("CY_BLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO: security:%x, bonding:%x, ekeySize:%x, authErr %x \r\n", 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).security, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bonding, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).ekeySize, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).authErr);
            break;
            
        case CY_BLE_EVT_GAP_AUTH_COMPLETE:
            authInfo = (cy_stc_ble_gap_auth_info_t *)eventParam;
            (void)authInfo;
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_COMPLETE: security: 0x%x, bonding: 0x%x, ekeySize: 0x%x, authErr 0x%x \r\n",
                                    authInfo->security, authInfo->bonding, authInfo->ekeySize, authInfo->authErr);
            
            DBG_PRINTF("StartDiscovery \r\n");
            apiResult = Cy_BLE_GATTC_StartDiscovery(appConnHandle);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("StartDiscovery API Error: %d \r\n", apiResult);
            }
            break;

        case CY_BLE_EVT_GAP_AUTH_FAILED:
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_FAILED: %x \r\n", ((cy_stc_ble_gap_auth_info_t *)eventParam)->authErr);
            break;

        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            DBG_PRINTF("CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP, state: %d \r\n", Cy_BLE_GetAdvertisementState());
            
            if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED)
            {   
                /* Fast and slow advertising period complete, go to low-power  
                 * mode (Hibernate) and wait for external
                 * user event to wake device up again */
                Cy_BLE_Stop();
            }
            break;
            
        case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_CONNECTED: %x, %x(%.2f ms), %x, %x \r\n",   
                ((cy_stc_ble_gap_connected_param_t *)eventParam)->status,
                ((cy_stc_ble_gap_connected_param_t *)eventParam)->connIntv,
                (float)((cy_stc_ble_gap_connected_param_t *)eventParam)->connIntv * CY_BLE_CONN_INTRV_TO_MS,
                ((cy_stc_ble_gap_connected_param_t *)eventParam)->connLatency,
                ((cy_stc_ble_gap_connected_param_t *)eventParam)->supervisionTO);
            
            /* Set security keys for new device which is not already bonded */
            if(App_IsDeviceInBondList((*(cy_stc_ble_gap_connected_param_t *)eventParam).bdHandle) == 0u)
            {
                keyInfo.SecKeyParam.bdHandle = (*(cy_stc_ble_gap_connected_param_t *)eventParam).bdHandle;
                apiResult = Cy_BLE_GAP_SetSecurityKeys(&keyInfo);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAP_SetSecurityKeys API Error: %d \r\n", apiResult);
                }
            }
            
            ancsFlag = 0u;
            break;

        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_DISCONNECTED\r\n");
            
            /* Enter discoverable mode so that remote Client could find device */
            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("StartAdvertisement API Error: %d \r\n", apiResult);
            }
            break;

        case CY_BLE_EVT_GAP_ENCRYPT_CHANGE:
            DBG_PRINTF("ENCRYPT_CHANGE: %x \r\n", 
                ((cy_stc_ble_gap_encrypt_change_param_t *)((cy_stc_ble_events_param_generic_t *)eventParam)->eventParams)->encryption);
            break;

        case CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GAPC_CONNECTION_UPDATE_COMPLETE: %x, %x(%.2f ms), %x, %x \r\n",
                ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->status,
                ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->connIntv,
                (float)((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->connIntv * CY_BLE_CONN_INTRV_TO_MS,
                ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->connLatency,
                ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->supervisionTO);
            break;

        case CY_BLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
            DBG_PRINTF("CY_BLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT\r\n");
            break;
            
        case CY_BLE_EVT_GAP_KEYS_GEN_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GAP_KEYS_GEN_COMPLETE \r\n");
            keyInfo.SecKeyParam = (*(cy_stc_ble_gap_sec_key_param_t *)eventParam);
            apiResult = Cy_BLE_GAP_SetIdAddress(&cy_ble_deviceAddress);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAP_SetIdAddress API Error: %d \r\n", apiResult);
            }
            break;
            
        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CY_BLE_EVT_GATT_CONNECT_IND:
            appConnHandle = *(cy_stc_ble_conn_handle_t *)eventParam;
            DBG_PRINTF("CY_BLE_EVT_GATT_CONNECT_IND: %x, %x \r\n", appConnHandle.attId, appConnHandle.bdHandle);
            break;

        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            DBG_PRINTF("CY_BLE_EVT_GATT_DISCONNECT_IND: \r\n");
            appConnHandle.attId = 0u;
            appConnHandle.bdHandle = 0u;
            break;

        case CY_BLE_EVT_GATTS_XCNHG_MTU_REQ:
            { 
                cy_stc_ble_gatt_xchg_mtu_param_t mtu;
                Cy_BLE_GATT_GetMtuSize(&mtu);
                DBG_PRINTF("CY_BLE_EVT_GATTS_XCNHG_MTU_REQ, final mtu= %d \r\n", mtu.mtu);
            }
            break;
            
        case CY_BLE_EVT_GATTC_XCHNG_MTU_RSP:
            DBG_PRINTF("CY_BLE_EVT_GATTC_XCHNG_MTU_RSP\r\n");
            break;

        case CY_BLE_EVT_GATTC_DISCOVERY_COMPLETE:
            DBG_PRINTF("The discovery is complete.\r\n");
            DBG_PRINTF("The discovered services: \r\n");
            for(i = 0u; i < CY_BLE_SRVI_COUNT; i++)
            {
                DBG_PRINTF("Service with UUID 0x%x has handle range from 0x%x to 0x%x\r\n",
                       cy_ble_serverInfo[discIdx][i].uuid,
                       cy_ble_serverInfo[discIdx][i].range.startHandle,
                       cy_ble_serverInfo[discIdx][i].range.endHandle);
                if(cy_ble_serverInfo[discIdx][i].uuid == 0x0000u)
                {
                    if(cy_ble_serverInfo[discIdx][i].range.startHandle < cy_ble_serverInfo[discIdx][i].range.endHandle)
                    {
                        DBG_PRINTF("The peer device supports ANS \r\n");
                        ancsFlag |= CY_BLE_ANCS_FLG_START;
                    }
                    else
                    {
                        DBG_PRINTF("The peer device doesn't support ANS \r\n");
                    }
                }
            }
        break;

        case CY_BLE_EVT_GATTC_ERROR_RSP:
            DBG_PRINTF("CY_BLE_EVT_GATTC_ERROR_RSP: opcode: %x,  handle: %x,  errorcode: %x, ",
                    ((cy_stc_ble_gatt_err_param_t *)eventParam)->errInfo.opCode,
                    ((cy_stc_ble_gatt_err_param_t *)eventParam)->errInfo.attrHandle,
                    ((cy_stc_ble_gatt_err_param_t *)eventParam)->errInfo.errorCode);
            
            DBG_PRINTF("opcode: ");
            switch(((cy_stc_ble_gatt_err_param_t *)eventParam)->errInfo.opCode)
            {
                case CY_BLE_GATT_FIND_INFO_REQ:
                    DBG_PRINTF("FIND_INFO_REQ");
                    break;

                case CY_BLE_GATT_READ_BY_TYPE_REQ:
                    DBG_PRINTF("READ_BY_TYPE_REQ");
                    break;

                case CY_BLE_GATT_READ_BY_GROUP_REQ:
                    DBG_PRINTF("READ_BY_GROUP_REQ");
                    break;
                    
                case CY_BLE_GATT_WRITE_REQ:
                    DBG_PRINTF("WRITE_REQ");
                    break;
                    
                case CY_BLE_GATT_READ_REQ:
                    DBG_PRINTF("READ_REQ");
                    break;
                    
                case CY_BLE_GATT_EXECUTE_WRITE_REQ:
                    DBG_PRINTF("EXECUTE_WRITE_REQ");
                    break;

                default:
                    break;
            }
            DBG_PRINTF("errorcode: ");
            switch(((cy_stc_ble_gatt_err_param_t *)eventParam)->errInfo.errorCode)
            {
                case CY_BLE_GATT_ERR_ATTRIBUTE_NOT_FOUND:
                    DBG_PRINTF("ATTRIBUTE_NOT_FOUND");
                    break;

                case CY_BLE_GATT_ERR_READ_NOT_PERMITTED:
                    DBG_PRINTF("READ_NOT_PERMITTED");
                    break;
                    
                case CY_BLE_GATT_ERR_INSUFFICIENT_AUTHENTICATION:
                    DBG_PRINTF("INSUFFICIENT_AUTHENTICATION");
                    break;
                    
                case CY_BLE_GATT_ERR_INSUFFICIENT_ENCRYPTION:
                    DBG_PRINTF("INSUFFICIENT_ENCRYPTION");
                    break;
                    
                case CY_BLE_GATT_ERR_INVALID_ATTRIBUTE_LEN:
                    DBG_PRINTF("INVALID_ATTRIBUTE_LEN");
                    break;

                default:
                    break;
            }
            DBG_PRINTF("\r\n");
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
            
        case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
            DBG_PRINTF("CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ, attHandle: %d \r\n", 
                ((cy_stc_ble_gatts_char_val_read_req_t *)eventParam)->attrHandle);
            break;
        
        case CY_BLE_EVT_GATTS_INDICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_GATTS_INDICATION_DISABLED \r\n");
            break;
            
        case CY_BLE_EVT_GATTS_INDICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_GATTS_INDICATION_ENABLED \r\n");
            break;
            
        case CY_BLE_EVT_GATTC_DISC_SKIPPED_SERVICE:
            DBG_PRINTF("CY_BLE_EVT_GATTC_DISC_SKIPPED_SERVICE \r\n");
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
*  Main function.
*
*******************************************************************************/
int HostMain(void)
{
    cy_en_ble_api_result_t apiResult;
    
    /* Initialization the user interface: LEDs, SW2, etc.  */
    InitUserInterface();  
    
    /* Initialize Debug UART */
    UART_START();     
    DBG_PRINTF("BLE Apple Notification Center Example Project \r\n");
   
    /* Start BLE component and register generic event handler */
    apiResult = Cy_BLE_Start(AppCallBack);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_Start API Error: 0x%x \r\n", apiResult);
    }

    /* Print stack version */
    PrintStackVersion();
    
    /* Initialize BLE Services */
    /* Register the event handler for ANCS specific events */
    Cy_BLE_ANCS_RegisterAttrCallback(AncsCallBack);
   

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
        
        /* Wait for connection established with Central device */
        if(Cy_BLE_GetConnectionState(appConnHandle) >= CY_BLE_CONN_STATE_CONNECTED)
        {   
            /* Process ANCS tasks */
            AncsProcess();
        }
        else
        {
           /* Execute remove device from Bond list if it was required */
            if((Cy_BLE_GetNumOfActiveConn() == 0u) && (App_IsRemoveBondListFlag() == true))
            {
                App_RemoveDevicesFromBondList();
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

