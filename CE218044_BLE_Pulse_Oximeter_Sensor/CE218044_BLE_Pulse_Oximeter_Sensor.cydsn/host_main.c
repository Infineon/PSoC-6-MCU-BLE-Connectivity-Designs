/*******************************************************************************
* File Name: host_main.c
*
* Version 1.0
*
* Description:
*  This code example BLE example project demonstrates how to configure and use 
*  Cypress's BLE component APIs and application layer callback for
*  Pulse Oximeter Profile (PLXP). 
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


#include <project.h>
#include "common.h"
#include "user_interface.h"
#include "plxs.h"
#include "bas.h"
#include "bmss.h"
#include "cts.h"


/* Global Variables */
static cy_stc_ble_conn_handle_t appConnHandle;
static volatile uint32_t        mainTimer  = 1u;

static cy_en_app_adv_state_t    appAdvState;

/* Private Function Prototypes */
static void LowPowerImplementation(void);
static uint8_t GetAdvConfigurationIdx(void);


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
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
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

    switch(event)
    {
        /**********************************************************
        *                       Generic and HCI Events
        ***********************************************************/
        case CY_BLE_EVT_STACK_ON: /* This event received when component is started */
            DBG_PRINTF("CY_BLE_EVT_STACK_ON, StartAdvertisement \r\n");
            
            /* Set state of advertising mode */
            appAdvState = App_GetCountOfBondedDevices() ? APP_ADV_STATE_BONDED_WHITELIST_ENABLE : APP_ADV_STATE_NON_BONDED;
            
            /* Enter into discoverable mode so that remote can find it. */
            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, GetAdvConfigurationIdx());
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAPP_StartAdvertisement API Error: ");
                PrintApiResult(apiResult);
            }
            
            /* Generates the security keys */
            apiResult = Cy_BLE_GAP_GenerateKeys(&keyInfo);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAP_GenerateKeys API Error: ");
                PrintApiResult(apiResult);
            }
            
            /* Display Bond list */
            App_DisplayBondList();
            
            /* Enable SW2 when stack started */
            SW2_ClearInt();
            SW2_EnableInt();
            break;
        
        case CY_BLE_EVT_TIMEOUT:
            DBG_PRINTF("CY_BLE_EVT_TIMEOUT \r\n");
            break;
            
        case CY_BLE_EVT_HARDWARE_ERROR:    /* This event indicates that some internal HW error has occurred. */
            DBG_PRINTF("Hardware Error \r\n");
            IndicateError();
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
                DBG_PRINTF("Cy_BLE_GAP_GetBdAddress API Error: ");
                PrintApiResult(apiResult);
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
            
        case CY_BLE_EVT_ADD_DEVICE_TO_WHITE_LIST_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_ADD_DEVICE_TO_WHITE_LIST_COMPLETE \r\n");
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
            
        case CY_BLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
            DBG_PRINTF("CY_BLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT \r\n");
            break;
            
        case CY_BLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO:
            DBG_PRINTF("CY_BLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO: "
                       "bdHandle=%x, security=%x, bonding=%x, ekeySize=%x, err=%x \r\n", 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bdHandle, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).security, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bonding, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).ekeySize, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).authErr);
            break;
        
        case CY_BLE_EVT_GAP_AUTH_COMPLETE:
            {
                cy_stc_ble_gap_peer_addr_info_t peerAddrInfo;
                cy_stc_ble_gap_auth_info_t *authInfo;
                
                authInfo = (cy_stc_ble_gap_auth_info_t *)eventParam;
                peerAddrInfo.bdHandle = authInfo->bdHandle;

                DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_COMPLETE: security: 0x%x, bonding:"\
                           " 0x%x, ekeySize: 0x%x, authErr 0x%x \r\n", authInfo->security, authInfo->bonding, 
                                                                       authInfo->ekeySize, authInfo->authErr);
               
                /* Add peer to white list */   
                Cy_BLE_GAP_GetPeerBdAddr(&peerAddrInfo);   
                apiResult = Cy_BLE_AddDeviceToWhiteList(&peerAddrInfo.bdAddr);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_AddDeviceToWhiteList Error: 0x%x \r\n", apiResult);
                }
            }
            break;
            
       case CY_BLE_EVT_GAP_AUTH_FAILED:
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_FAILED: bdHandle=%x, authErr=%x\r\n", 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).bdHandle, 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).authErr);
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_FAILED, reason: ");
            App_ShowAuthError(((cy_stc_ble_gap_auth_info_t *)eventParam)->authErr);
            break;
            
        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            DBG_PRINTF("CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP, state: %x\r\n", Cy_BLE_GetAdvertisementState());

            if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED)
            {
                if(appAdvState == APP_ADV_STATE_BONDED_WHITELIST_ENABLE)
                {        
                    appAdvState = APP_ADV_STATE_BONDED_WHITELIST_DISABLEE;
                    
                    /* Enter into discoverable mode so that remote can find it. */
                    apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, GetAdvConfigurationIdx());
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_GAPP_StartAdvertisement API Error: ");
                        PrintApiResult(apiResult);
                    }  
                    
                }else
                {
                    /* Fast and slow advertising period complete, go to low power  
                     * mode (Hibernate) and wait for an external
                     * user event to wake up the device again */
                    UpdateLedState();     
                    Cy_BLE_Stop();  
                }
            }
            break;
            
        case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_CONNECTED: connIntv = %d ms \r\n", 
                       ((cy_stc_ble_gap_connected_param_t *)eventParam)->connIntv * 5u /4u); /* in milliseconds / 1.25ms */
                                            
            /* Update Security Keys */
            if(App_IsDeviceInBondList((*(cy_stc_ble_gap_connected_param_t *)eventParam).bdHandle) == 0u)
            {
                keyInfo.SecKeyParam.bdHandle = (*(cy_stc_ble_gap_connected_param_t *)eventParam).bdHandle;
                apiResult = Cy_BLE_GAP_SetSecurityKeys(&keyInfo);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAP_SetSecurityKeys API Error: 0x%x \r\n", apiResult);
                }     
            }         
               
            break;
             
        case CY_BLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP:
            DBG_PRINTF("CY_BLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP, result = %d\r\n", 
                (*(cy_stc_ble_l2cap_conn_update_rsp_param_t *)eventParam).result);
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
            
            /* Set state of advertising mode */
            appAdvState = App_GetCountOfBondedDevices() ? APP_ADV_STATE_BONDED_WHITELIST_ENABLE : APP_ADV_STATE_NON_BONDED;
            
            /* Put the device into discoverable mode so that a remote can find it */
            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, GetAdvConfigurationIdx());
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("StartAdvertisement API Error: "); 
                PrintApiResult(apiResult);
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
            break;
            
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            DBG_PRINTF("CY_BLE_EVT_GATT_DISCONNECT_IND: %x, %x \r\n", 
                        (*(cy_stc_ble_conn_handle_t *)eventParam).attId, 
                        (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle);
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
            /* Triggered on server side when client sends read request and when
            * characteristic has CY_BLE_GATT_DB_ATTR_CHAR_VAL_RD_EVENT property set.
            * This event could be ignored by application unless it need to response
            * by error response which needs to be set in gattErrorCode field of
            * event parameter. */
            DBG_PRINTF("CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ: handle: %x \r\n", 
                ((cy_stc_ble_gatts_char_val_read_req_t *)eventParam)->attrHandle);
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
* Function Name: GetAdvConfigurationIdx(void)
********************************************************************************
* Summary:
*  This function returns proper advertisement configuration index according to
*  appAdvState
*
* Return:
*  uint8_t - Advertisement configuration index.
*
*******************************************************************************/
static uint8_t GetAdvConfigurationIdx(void)
{
    uint8_t index;
    switch(appAdvState)
    {
        /* In non-bonded mode, the design shall advertise in fast advertisement mode 
            for 30 seconds & fall back to slow advertisement for 150 seconds after timeout. 
            Advertising configuration: CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX */ 
        case APP_ADV_STATE_NON_BONDED:
            index = CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX;
            break;
            
        /* When in bonded mode, enable bonded devices whitelist for the first 10 seconds of
            advertisement. Advertising configuration: CY_BLE_PERIPHERAL_CONFIGURATION_1_INDEX */                                         
        case APP_ADV_STATE_BONDED_WHITELIST_ENABLE:
            index = CY_BLE_PERIPHERAL_CONFIGURATION_1_INDEX;
            break;
        
        /* When in bonded mode and if not connected in the first 10 seconds, disable whitelist and 
            advertise in discoverable undirected mode for the next 20 seconds. Switch over to 150 seconds 
            of slow advertisement. Advertising configuration: CY_BLE_PERIPHERAL_CONFIGURATION_2_INDEX */ 
        case APP_ADV_STATE_BONDED_WHITELIST_DISABLEE:
            index = CY_BLE_PERIPHERAL_CONFIGURATION_2_INDEX;
            break;
        
        default:
            index = CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX;
    }

    return(index);
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
    if((UART_DEB_IS_TX_COMPLETE() != 0u) &&
       (App_IsAuthReq() == false))
    {            
        /* Entering into the Deep Sleep */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }
}


/*******************************************************************************
* Function Name: Timer_Interrupt
********************************************************************************
*
* Summary:
*   Handles the MCWDT. 
*   MCWDT timer is used to time simulations and LED blinking.
*
*******************************************************************************/
void Timer_Interrupt(void)
{
    /* Update LED State */
    UpdateLedState();

    /* Indicate that timer is raised to the main loop */
    mainTimer++;

    /* Press and hold the mechanical button (SW2) during 4 seconds to clear the bond list. */
    App_RemoveDevicesFromBondListBySW2Press(SW2_PRESS_TIME_DEL_BOND_LIST);
         
    /* Clear MCWDT Interrupt */
    MCWDT_ClearInterrupt(CY_MCWDT_CTR0);
}


/*******************************************************************************
* Function Name: HostMain()
********************************************************************************
* Summary:
*  Main function for the project.
*
*******************************************************************************/
int HostMain(void)
{    
    cy_en_ble_api_result_t apiResult;
    
    /* Initialize wakeup pin for Hibernate */
    Cy_SysPm_SetHibWakeupSource(CY_SYSPM_HIBPIN1_LOW);
    
    /* Initialize LEDs */
    DisableAllLeds();
    
    /* Initialization the user interface: LEDs, SW2, ect.  */
    InitUserInterface();
      
    /* Initialize Timer */
    Cy_SysInt_Init(&Timer_Int_cfg, Timer_Interrupt);
    NVIC_EnableIRQ(Timer_Int_cfg.intrSrc);   
    MCWDT_SetInterruptMask(CY_MCWDT_CTR0);
    MCWDT_Start(); 
    
    /* Initialize Debug UART */ 
    UART_START();
    DBG_PRINTF("BLE Pulse Oximeter Sensor Project \r\n");
       
    /* Start BLE component and register generic event handler */
    apiResult = Cy_BLE_Start(AppCallBack);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_Start API Error: ");
        PrintApiResult(apiResult);
    }
   
    /* Print stack version */
    PrintStackVersion();
        
    /* Initialize BLE Services */
    PlxsInit();
    BmsInit();
    BasInit();
    CtsInit();
    
    /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    while(1)
    {
        /* Cy_BLE_ProcessEvents() allows BLE stack to process pending events */
        Cy_BLE_ProcessEvents();
        
        /* To achieve low power in the device */
        LowPowerImplementation();
               
        /* Periodically simulate PLXS (SpO2, Pr, PRI) and measure a battery level 
        *  and send results to the Client
        */        
        if(mainTimer != 0u)
        {        
            /* PLXS Simulation   */
            PlxsSimulateMeasurement(appConnHandle);

            /* BAS Simulation   */
            BasSimulateBattery(appConnHandle);
            
            /* Reset timer value each 1 second */
            mainTimer = 0u;
        }                 
        
        /* Wait for connection established with Central device */
        if(Cy_BLE_GetConnectionState(appConnHandle) >= CY_BLE_CONN_STATE_CONNECTED)
        {
            /* PLXS RACP processing */
            PlxsRacpProcess(appConnHandle);   
        }
        else
        {
            /* Process BMS actions when device is disconnected */
            BmsProcess();        
            
            /* Remove devices from the bond list. Should be done when no active connections */
            if(App_IsRemoveBondListFlag() == true)
            {
                skipRunSpotChekPocedure = true;
                App_RemoveDevicesFromBondList();
            }  
        }
        
        /* Passkey entry */
        if(App_IsAuthReq())
        {
            App_AuthReplay(appConnHandle);
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
