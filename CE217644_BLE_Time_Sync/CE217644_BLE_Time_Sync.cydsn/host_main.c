/*******************************************************************************
* File Name: host_main.c
*
* Version 1.0
*
* Description:
*  This is source code for the BLE Time Sync example project.
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


#include "common.h"
#include "cts.h"
#include "ndcs.h"
#include "rtus.h"
#include "user_interface.h"

/* Global Variables */
cy_stc_ble_conn_handle_t                appConnHandle;
volatile uint32_t                       mainTimer  = 1u;
cy_stc_ble_timer_info_t                 timerParam = { .timeout = TIMER_TIMEOUT };

/* Private Function Prototypes */
static void LowPowerImplementation(void);
static uint32_t IsDeviceInBondList(uint32_t bdHandle);

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
    cy_stc_ble_gap_peer_addr_info_t bondedDeviceInfo[CY_BLE_GAP_MAX_BONDED_DEVICE];
    cy_stc_ble_gap_bonded_device_list_info_t bondedDeviceList =
    {
        .bdHandleAddrList = bondedDeviceInfo
    };
    uint32_t deviceIsDetected = 0u;
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
                    deviceIsDetected = 1u;
                }
            } while(deviceCount != 0u);
        }
    }
    return(deviceIsDetected);
}


/*******************************************************************************
* Function Name: AppCallBack()
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE Component.
*
* Parameters:
*   event      - the event code
*   eventParam - the event parameters
*
*******************************************************************************/
void AppCallBack(uint32_t event, void *eventParam)
{
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_gap_auth_info_t *authInfo;
    uint16_t i;

    static cy_stc_ble_gap_sec_key_info_t keyInfo =
    {
        .localKeysFlag = CY_BLE_GAP_SMP_INIT_ENC_KEY_DIST | 
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
        /**********************************************************
        *                       Generic and HCI Events
        ***********************************************************/
        case CY_BLE_EVT_STACK_ON: /* This event received when component is started */

            DBG_PRINTF("CY_BLE_EVT_STACK_ON \r\n");
            apiResult = Cy_BLE_GAP_GenerateKeys(&keyInfo);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAP_GenerateKeys API Error: 0x%x \r\n", apiResult);
            }
            
            /* Enter into discoverable mode so that remote can find it */
            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAPP_StartAdvertisement API Error: 0x%x \r\n", apiResult);
            }    
            break;

        case CY_BLE_EVT_TIMEOUT:
            switch(*(cy_en_ble_to_reason_code_t *)eventParam)
            {
                case CY_BLE_GAP_ADV_TO:
                    /* Advertisement time set by application has expired */
                    DBG_PRINTF("CY_BLE_GAP_ADV_MODE_TO.\r\n");
                    break;
                case CY_BLE_GAP_SCAN_TO:
                    /* Scan time set by application has expired */
                    DBG_PRINTF("CY_BLE_GAP_SCAN_TO.\r\n");
                    break;
                case CY_BLE_GATT_RSP_TO:
                    /* GATT procedure timeout */
                    DBG_PRINTF("CY_BLE_GATT_RSP_TO\r\n");
                    break;
                case CY_BLE_GENERIC_APP_TO:
                    /* Generic timeout */
                    if(((cy_stc_ble_timeout_param_t *)eventParam)->timerHandle == timerParam.timerHandle)
                    {
                        UpdateLedState();
                        mainTimer++;
                    }
                    break;
                default:    /* Not existing timeout reason */
                    DBG_PRINTF("CY_BLE_EVT_TIMEOUT: %x\r\n", *(uint8_t *)eventParam);
                    break;
            }
            break;
            
        case CY_BLE_EVT_STACK_BUSY_STATUS:
            break;
            
        case CY_BLE_EVT_PENDING_FLASH_WRITE:
            DBG_PRINTF("CY_BLE_EVT_PENDING_FLASH_WRITE\r\n");
            break;
            
        case CY_BLE_EVT_HARDWARE_ERROR:    /* This event indicates that some internal HW error has occurred */
            DBG_PRINTF("CY_BLE_EVT_HARDWARE_ERROR \r\n");
            break;
            
        case CY_BLE_EVT_LE_SET_EVENT_MASK_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_LE_SET_EVENT_MASK_COMPLETE \r\n");
            break;

        case CY_BLE_EVT_SET_SUGGESTED_DATA_LENGTH_COMPLETE:            
            DBG_PRINTF("CY_BLE_EVT_SET_SUGGESTED_DATA_LENGTH_COMPLETE \r\n");
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
            DBG_PRINTF("CY_BLE_EVT_GET_DEVICE_ADDR_COMPLETE: public:");
            for(i = CY_BLE_GAP_BD_ADDR_SIZE; i > 0u; i--)
            {
                DBG_PRINTF("%2.2x", ((cy_stc_ble_bd_addrs_t *)
                                     ((cy_stc_ble_events_param_generic_t *)eventParam)->eventParams)->publicBdAddr[i-1]);
            }
            DBG_PRINTF("\r\n");
            break;
            
        case CY_BLE_EVT_SET_TX_PWR_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_SET_TX_PWR_COMPLETE \r\n");
            break;
            
        case CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE \r\n");
            DBG_PRINTF("Hibernate\r\n");
            UpdateLedState();
            UART_DEB_WAIT_TX_COMPLETE();
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
                DBG_PRINTF("Cy_BLE_GAPP_AuthReqReply API Error: 0x%x, call Cy_BLE_GAP_RemoveOldestDeviceFromBondedList\r\n", apiResult);
                apiResult = Cy_BLE_GAP_RemoveOldestDeviceFromBondedList();
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAP_RemoveOldestDeviceFromBondedList API Error: 0x%x \r\n", apiResult);
                }
                else
                {
                    apiResult = Cy_BLE_GAPP_AuthReqReply(&cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);            
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_GAPP_AuthReqReply API Error: 0x%x \r\n", apiResult);
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
            
            apiResult = Cy_BLE_GATTC_StartDiscovery(appConnHandle);
            DBG_PRINTF("Start Discovery \r\n");
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("StartDiscovery API Error: 0x%x \r\n", apiResult);
            }
            break;

        case CY_BLE_EVT_GAP_AUTH_FAILED:
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_FAILED: %x \r\n", ((cy_stc_ble_gap_auth_info_t *)eventParam)->authErr);
            break;

        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            DBG_PRINTF("CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP, state: %x\r\n", Cy_BLE_GetAdvertisementState());

            if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED)
            {
                /* Fast and slow advertising period complete, go to low power  
                 * mode (Hibernate mode) and wait for an external
                 * user event to wake up the device again */
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
            if(IsDeviceInBondList((*(cy_stc_ble_gap_connected_param_t *)eventParam).bdHandle) == 0u)
            {
                keyInfo.SecKeyParam.bdHandle = (*(cy_stc_ble_gap_connected_param_t *)eventParam).bdHandle;
                apiResult = Cy_BLE_GAP_SetSecurityKeys(&keyInfo);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAP_SetSecurityKeys API Error: 0x%x \r\n", apiResult);
                }
            }  

            /* Send authentication request to peer device */
            cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].bdHandle = appConnHandle.bdHandle;
            apiResult = Cy_BLE_GAP_AuthReq(&cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAP_AuthReq API Error: 0x%x \r\n", apiResult);
            }
            break;

        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_DISCONNECTED\r\n");
            
            /* Enter discoverable mode so that remote Client could find device */
            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("StartAdvertisement API Error: 0x%x \r\n", apiResult);
            }
            break;

        case CY_BLE_EVT_GAP_ENCRYPT_CHANGE:
            DBG_PRINTF("ENCRYPT_CHANGE: %x \r\n", 
                ((cy_stc_ble_gap_encrypt_change_param_t *)((cy_stc_ble_events_param_generic_t *)eventParam)->
                                                                                                eventParams)->encryption);
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
                DBG_PRINTF("Cy_BLE_GAP_SetIdAddress API Error: 0x%x \r\n", apiResult);
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
            DBG_PRINTF("CY_BLE_EVT_GATT_DISCONNECT_IND \r\n");
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
            DBG_PRINTF("CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ, attHandle: %d \r\n", 
                ((cy_stc_ble_gatts_char_val_read_req_t *)eventParam)->attrHandle);
            break;
            
        case CY_BLE_EVT_GATTC_DISCOVERY_COMPLETE:
            {
                uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*(cy_stc_ble_conn_handle_t *)eventParam);
                
                DBG_PRINTF("\r\n");
                DBG_PRINTF("Discovery complete: attId=%x, bdHandle=%x \r\n", 
                    (*(cy_stc_ble_conn_handle_t *)eventParam).attId, 
                    (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle);
                
                DBG_PRINTF("Discovered services: \r\n");
                for(i = 0u; i < CY_BLE_SRVI_COUNT; i++)
                {
                    DBG_PRINTF("Service with UUID 0x%x has handle range from 0x%x to 0x%x\r\n",
                           cy_ble_serverInfo[discIdx][i].uuid,
                           cy_ble_serverInfo[discIdx][i].range.startHandle,
                           cy_ble_serverInfo[discIdx][i].range.endHandle);
                }
                
                if(cy_ble_serverInfo[discIdx][CY_BLE_SRVI_CTS].range.startHandle != 0u)
                {
                    /* Enable Notification for Current Time Characteristic */
                    uint16_t timeCCCD = CY_BLE_CCCD_NOTIFICATION;
                    apiResult = Cy_BLE_CTSC_SetCharacteristicDescriptor(appConnHandle,
                                                                        CY_BLE_CTS_CURRENT_TIME,
                                                                        CY_BLE_CTS_CURRENT_TIME_CCCD,
                                                                        CY_BLE_CCCD_LEN,
                                                                        (uint8_t *)&timeCCCD);
                    DBG_PRINTF("Enable Current Time Notification, apiResult: %x \r\n", apiResult);
                }
                else
                {
                    DBG_PRINTF("There are no Current Time service in the connected device\r\n");
                }
                break;
            }
        case CY_BLE_EVT_GATTC_ERROR_RSP:
            DBG_PRINTF("CY_BLE_EVT_GATTC_ERROR_RSP, opCode: %x,  errorCode: %x,\r\n", 
                ((cy_stc_ble_gatt_err_param_t *)eventParam)->errInfo.opCode,
                ((cy_stc_ble_gatt_err_param_t *)eventParam)->errInfo.errorCode);
            break;
        
        case CY_BLE_EVT_GATTC_DISC_SKIPPED_SERVICE:
            DBG_PRINTF("CY_BLE_EVT_GATTC_DISC_SKIPPED_SERVICE\r\n");
            break;
       
    /**********************************************************
    *                       Other Events
    ***********************************************************/
        default:
            DBG_PRINTF("Other event: 0x%lx \r\n", event);
            break;
    }
}


/*******************************************************************************
* Function Name: LowPowerImplementation()
********************************************************************************
* Summary:
* Implements low power in the project.
*
* Theory:
* The function tries to enter deep sleep as much as possible - whenever the 
* BLE is idle and the UART transmission/reception is not happening. 
*
*******************************************************************************/
static void LowPowerImplementation(void)
{
    if(UART_DEB_IS_TX_COMPLETE() == true)
    {
        /* Entering into the Deep Sleep */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
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
    
    /* Initialization the user interface: LEDs, SW2, etc.  */
    InitUserInterface();
    
    /* Initialize Debug UART */
    UART_START();
    DBG_PRINTF("BLE Time Sync code example\r\n");

    /* Start BLE component and register generic event handler */
    apiResult = Cy_BLE_Start(AppCallBack);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_Start API Error: 0x%x \r\n", apiResult);
    }
    /* Print stack version */
    PrintStackVersion();
    
    /* Initialize BLE Services */
    CtsInit();
    NdcsInit();
    RtusInit();
    
    /* Start Real Time Clock */
    RTC_Start();
    
    /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    while(1)
    {
        /* Cy_BLE_ProcessEvents() allows BLE stack to process pending events */
        Cy_BLE_ProcessEvents();
        
        /* To achieve low power */
        LowPowerImplementation();

        /* Restart timer */
        if(mainTimer != 0u)
        {
            mainTimer = 0u;
            Cy_BLE_StartTimer(&timerParam);
            
            /* Read current time from RTC and print it */
            CtsPrintCurrentTime();                
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
