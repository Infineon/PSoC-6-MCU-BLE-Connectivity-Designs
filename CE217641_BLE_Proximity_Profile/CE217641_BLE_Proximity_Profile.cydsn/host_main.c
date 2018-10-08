/*******************************************************************************
* File Name: host_main.c
*
* Version 1.0
*
* Description:
*  This is source code for the Proximity Profile example project. In this
*  example project the BLE component is configured for Proximity Reporter
*  profile role.
*
* Note:
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
#include "lls.h"
#include "tps.h"
#include "user_interface.h"

/* Global Variables */
static cy_stc_ble_conn_handle_t            appConnHandle;
static volatile uint32_t                   mainTimer  = 1u;
static cy_stc_ble_timer_info_t             timerParam = { .timeout = ADV_TIMER_TIMEOUT };
 
static volatile cy_en_ble_bless_pwr_lvl_t  getPwrLevelValue;
static volatile bool                       getTxPowerLevelFlag;

static bool                                isButtonPressed     = false;
static bool                                displayAlertMessage = false;
static uint8_t                             llsAlertTOCounter;

/* Private Function Prototypes */
static void LowPowerImplementation(void);

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
            
            /* Enter into discoverable mode so that remote can find it */
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
            break;
            
        case CY_BLE_EVT_TIMEOUT: /* 0x01 -> GAP limited discoverable mode timeout */
                                 /* 0x02 -> GAP pairing process timeout */
                                 /* 0x03 -> GATT response timeout */
            if((((cy_stc_ble_timeout_param_t *)eventParam)->reasonCode == CY_BLE_GENERIC_APP_TO) && 
               (((cy_stc_ble_timeout_param_t *)eventParam)->timerHandle == timerParam.timerHandle))
            {
                /* Update Led State */
                UpdateLedState(llsAlertTOCounter);                
                if(Cy_BLE_GetNumOfActiveConn() == 0u)
                {
                    if(displayAlertMessage == true)
                    {   
                        if((LlsGetAlertLevel() == CY_BLE_MILD_ALERT) && (llsAlertTOCounter != ALERT_TIMEOUT))
                        {
                            DBG_PRINTF("Device started alerting with \"Mild Alert\"\r\n");
                        }
                        else if((LlsGetAlertLevel() == CY_BLE_HIGH_ALERT) && (llsAlertTOCounter != ALERT_TIMEOUT))
                        {
                            DBG_PRINTF("Device started alerting with \"High Alert\"\r\n");
                        }
                        
                        displayAlertMessage = false;
                    }
                } 
                
                /* Indicate that timer is raised to the main loop */
                mainTimer++;
                
                if(Cy_BLE_GetNumOfActiveConn() == 0u)
                {
                    if((llsAlertTOCounter != ALERT_TIMEOUT) && (LlsGetAlertLevel() != CY_BLE_NO_ALERT))
                    {
                        /* Update alert timeout */
                        llsAlertTOCounter++;
                    }
                    else
                    {
                        LlsSetAlertLevel(CY_BLE_NO_ALERT);
                        
                        /* Clear alert timeout */
                        llsAlertTOCounter = 0u;
                    }
                }
            }
            else
            {
                DBG_PRINTF("CY_BLE_EVT_TIMEOUT: %d \r\n", *(uint8_t *)eventParam);
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
        
        case CY_BLE_EVT_GET_TX_PWR_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GET_TX_PWR_COMPLETE  \r\n"); 
            getPwrLevelValue = ((cy_stc_ble_tx_pwr_lvl_info_t *)((cy_stc_ble_events_param_generic_t *)eventParam)->eventParams)->blePwrLevel;
            getTxPowerLevelFlag = true;
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
            UpdateLedState(llsAlertTOCounter);
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
            break;
            
	    case CY_BLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
            DBG_PRINTF("CY_BLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST: %6.6ld\r\n", *(uint32_t *)eventParam);
            break;
            
        case CY_BLE_EVT_GAP_NUMERIC_COMPARISON_REQUEST:
            DBG_PRINTF("Compare this passkey with the one displayed in your peer device and press 'y' or 'n':"
                       " %6.6lu \r\n", *(uint32_t *)eventParam);
            break;    
            
        case CY_BLE_EVT_GAP_AUTH_FAILED:
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_FAILED, reason: %x \r\n", ((cy_stc_ble_gap_auth_info_t *)eventParam)->authErr);
            
            break;

        case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_CONNECTED: connIntv = %d ms \r\n",          /* in milliseconds / 1.25ms */
                        ((cy_stc_ble_gap_connected_param_t *)eventParam)->connIntv * CY_BLE_CONN_INTRV_TO_MS);
        
            keyInfo.SecKeyParam.bdHandle = (*(cy_stc_ble_gap_connected_param_t *)eventParam).bdHandle;     
            apiResult = Cy_BLE_GAP_SetSecurityKeys(&keyInfo);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAP_SetSecurityKeys API Error: 0x%x \r\n", apiResult);
            }
            break;

        case CY_BLE_EVT_GAP_KEYS_GEN_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GAP_KEYS_GEN_COMPLETE \r\n");
            keyInfo.SecKeyParam = (*(cy_stc_ble_gap_sec_key_param_t *)eventParam);
            Cy_BLE_GAP_SetIdAddress(&cy_ble_deviceAddress);
            
            /* Init MAX Power level  */
            TpsInitTxPower();
            break;
            
        case CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE: connIntv = %d ms \r\n", /* in milliseconds / 1.25ms */
                        ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->connIntv * CY_BLE_CONN_INTRV_TO_MS);
            break;
            
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_DISCONNECTED: bdHandle=%x, reason=%x, status=%x\r\n",
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).bdHandle, 
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).reason, 
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).status);
          
            /* Put device to discoverable mode so that remote can find it */
            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAPP_StartAdvertisement API Error: 0x%x \r\n", apiResult);
            }
            break;

        case CY_BLE_EVT_GAP_AUTH_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_COMPLETE: security:%x, bonding:%x, ekeySize:%x, authErr %x \r\n",
                        ((cy_stc_ble_gap_auth_info_t *)eventParam)->security,
                        ((cy_stc_ble_gap_auth_info_t *)eventParam)->bonding, 
                        ((cy_stc_ble_gap_auth_info_t *)eventParam)->ekeySize, 
                        ((cy_stc_ble_gap_auth_info_t *)eventParam)->authErr);
            break;

        case CY_BLE_EVT_GAP_ENCRYPT_CHANGE:
            DBG_PRINTF("CY_BLE_EVT_GAP_ENCRYPT_CHANGE: %d \r\n", *(uint8_t *)eventParam);
            break;
            
        case CY_BLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
            DBG_PRINTF("CY_BLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT \r\n");
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
           
            displayAlertMessage = true;            
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
            DBG_PRINTF("Other event: 0x%lx \r\n", event);;
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
* Function Name: SW2_Interrupt
********************************************************************************
*
* Summary:
*   Handles the mechanical button press (SW2).
*
*******************************************************************************/
void SW2_Interrupt(void)
{
    /* In connected state - indicate that button was pressed. Check timer
     * value to cut false button presses.
     */
    if (Cy_GPIO_GetInterruptStatusMasked(SW2_0_PORT, SW2_0_NUM) == 1u)
    { 
        if(Cy_BLE_GetConnectionState(appConnHandle) >= CY_BLE_CONN_STATE_CONNECTED)
        {
            isButtonPressed = true;
        }
    
        SW2_CleanInt();
    } 
}


/*******************************************************************************
* Function Name: HostMain
********************************************************************************
*
* Summary:
*  The main function for the project.
*
* Theory:
*  The function starts the BLE and UART components.
*  This function processes all BLE events and also implements the low-power 
*  functionality.
*
*******************************************************************************/
int HostMain(void)
{ 
    cy_en_ble_api_result_t apiResult;
    
    /* Initialization the user interface: LEDs, SW2, etc.  */
    InitUserInterface();
        
    /* Initialize Debug UART */
	UART_START();
    DBG_PRINTF("\r\nBLE Proximity Profile Example\r\n");
    
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
    TpsInit();
	LlsInit();

    /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    while(1u)
    {        
        cy_stc_ble_tx_pwr_lvl_info_t txPower;
        
        /* Cy_BLE_ProcessEvents() allows BLE stack to process pending events */
        Cy_BLE_ProcessEvents();
        
        /* To achieve low power */
        LowPowerImplementation();
        
        /* Restart 1s timer */
        if(mainTimer != 0u)
        {
            mainTimer = 0u;
            Cy_BLE_StartTimer(&timerParam); 
        } 
        
        /* Decrease Tx power level of BLE radio if button is pressed */
        if((Cy_BLE_GetConnectionState(appConnHandle) >= CY_BLE_CONN_STATE_CONNECTED) && (isButtonPressed == true))
        {
            cy_en_ble_bless_pwr_lvl_t tempPwrLeve;
            
            /* Specify connection channels for reading Tx power level */
            txPower.pwrConfigParam.bdHandle  = appConnHandle.bdHandle;
            txPower.pwrConfigParam.bleSsChId = CY_BLE_LL_CONN_CH_TYPE;
        
            /* Get current Tx power level */
            getTxPowerLevelFlag = false;
            Cy_BLE_GetTxPowerLevel(&txPower.pwrConfigParam);
            
            /* Wait while Cy_BLE_GetTxPowerLevel complite */
            do{
                Cy_BLE_ProcessEvents();
            }
            while(getTxPowerLevelFlag == false);
           
            /* Decrease the Tx power level by one scale */
            tempPwrLeve = getPwrLevelValue;
            TpsDecreaseTxPowerLevelValue(&tempPwrLeve);
            txPower.blePwrLevel = tempPwrLeve;
           
            /* Set the new Tx power level */
            apiResult = Cy_BLE_SetTxPowerLevel(&txPower);

            if(apiResult == CY_BLE_SUCCESS)
            {
                int8_t intTxPowerLevel;
                /* Convert power level to numeric int8_t value */
                intTxPowerLevel = TpsConvertTxPowerlevelToInt8(getPwrLevelValue);

                /* Specify advertisement channels for reading Tx power level */
                txPower.pwrConfigParam.bleSsChId = CY_BLE_LL_ADV_CH_TYPE;

                /* Get current Tx power level */
                getTxPowerLevelFlag = false;
                Cy_BLE_GetTxPowerLevel(&txPower.pwrConfigParam);
                
                /* Wait while Cy_BLE_GetTxPowerLevel complite */
                do{
                    Cy_BLE_ProcessEvents();
                }
                while(getTxPowerLevelFlag == false);
                
                /* Decrease the Tx power level by one scale for the advertisement channels */
                tempPwrLeve = getPwrLevelValue;
                TpsDecreaseTxPowerLevelValue(&tempPwrLeve);
                txPower.blePwrLevel = tempPwrLeve;

                /* Set the new Tx power level for advertisement channels */
                apiResult = Cy_BLE_SetTxPowerLevel(&txPower);

                /* Write the new Tx power level value to the GATT database */
                apiResult = Cy_BLE_TPSS_SetCharacteristicValue(CY_BLE_TPS_TX_POWER_LEVEL,
                                                               CY_BLE_TPS_TX_POWER_LEVEL_SIZE,
                                                               &intTxPowerLevel);

                if(apiResult == CY_BLE_SUCCESS) 
                {
                    /* Display new Tx Power Level value */
                    DBG_PRINTF("Tx power level is set to %d dBm\r\n", intTxPowerLevel);

                    if (TpsIsNotificationPending() == true)
                    {
                        uint16_t cccd;
                        
                        apiResult = Cy_BLE_TPSS_GetCharacteristicDescriptor(appConnHandle, CY_BLE_TPS_TX_POWER_LEVEL, CY_BLE_TPS_CCCD, CY_BLE_CCCD_LEN, 
                                                       (uint8_t*)&cccd);
                            
                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("Cy_BLE_TPSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
                        }
                        else if(cccd == CY_BLE_CCCD_NOTIFICATION) 
                        {
                            if(Cy_BLE_GetConnectionState(appConnHandle) >= CY_BLE_CONN_STATE_CONNECTED)
                            {   
                                /* Send notification to the Client */
                                apiResult = Cy_BLE_TPSS_SendNotification(appConnHandle, CY_BLE_TPS_TX_POWER_LEVEL, 
                                                                         CY_BLE_TPS_TX_POWER_LEVEL_SIZE, &intTxPowerLevel);

                                if(apiResult == CY_BLE_SUCCESS)
                                {
                                    DBG_PRINTF("New Tx power level value was notified to the Client\r\n");
                                }
                                else
                                {
                                    DBG_PRINTF("Failed to send notification to the Client\r\n");
                                }
                            }
                        }
                    }
                }
            }
            /* Reset button state */
            isButtonPressed = false;    
        }
    }
}


/* [] END OF FILE */
