/*******************************************************************************
* File Name: host_main.c
*
* Version: 1.0
*
* Description:
*  This is source code for the BLE Weight Scale code example.
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
#include "common.h"
#include "user_interface.h"
#include "bcs.h"
#include "uds.h"
#include "wss.h"
    
/* Global Variables */
cy_stc_ble_conn_handle_t        appConnHandle;
volatile uint32_t               mainTimer  = 1u;
cy_stc_ble_timer_info_t         timerParam = { .timeout = ADV_TIMER_TIMEOUT };        
uint8_t                         userIndex  = 0u;

/* Private Function Prototypes */
static void LowPowerImplementation(void);


/*******************************************************************************
* Function Name: AppCallBack
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component.
*
* Parameters:
*   event:       Event from the BLE component.
*   eventParams: A structure instance for corresponding event type. The
*                list of event structure is described in the component
*                datasheet.
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
            DBG_PRINTF("Cy_BLE_GAP_GenerateKeys API Error: %d \r\n", apiResult);
        }
        
        /* Enter into discoverable mode so that remote can find it */
        apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("Cy_BLE_GAPP_StartAdvertisement API Error: %d \r\n", apiResult);
        }
        
        /* Display Bond list */
        App_DisplayBondList();
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
            if((((cy_stc_ble_timeout_param_t *)eventParam)->reasonCode == CY_BLE_GENERIC_APP_TO) && 
               (((cy_stc_ble_timeout_param_t *)eventParam)->timerHandle == timerParam.timerHandle))
            {
                UpdateLedState();
                mainTimer++;
                
                /* Press and hold the mechanical button (SW2) during 4 seconds to clear the bond list */
                App_RemoveDevicesFromBondListBySW2Press(SW2_PRESS_TIME_DEL_BOND_LIST);     
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
            DBG_PRINTF("%2.2x", ((cy_stc_ble_bd_addrs_t *)((cy_stc_ble_events_param_generic_t *)eventParam)->eventParams)->publicBdAddr[i-1]);
        }
        DBG_PRINTF("\r\n");
        break;
        
    case CY_BLE_EVT_SET_TX_PWR_COMPLETE:
        DBG_PRINTF("CY_BLE_EVT_SET_TX_PWR_COMPLETE \r\n");
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
        break;

    case CY_BLE_EVT_GAP_AUTH_FAILED:
        DBG_PRINTF("CY_BLE_EVT_GAP_AUTH_FAILED: %x \r\n", ((cy_stc_ble_gap_auth_info_t *)eventParam)->authErr);
        break;

    case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
        DBG_PRINTF("CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP, status: %x, state: %x\r\n", 
            *((uint8_t *)eventParam), Cy_BLE_GetAdvertisementState());

        if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED)
        {   
            /* Fast and slow advertising period complete, go to low-power  
             * mode (Hibernate) and wait for external
             * user event to wake device up again */
            Cy_BLE_Stop();
        }
        break;
        
    case CY_BLE_EVT_GAPP_UPDATE_ADV_SCAN_DATA_COMPLETE:            
        DBG_PRINTF("CY_BLE_EVT_GAPP_UPDATE_ADV_SCAN_DATA_COMPLETE, status: %x\r\n", *((uint8_t *)eventParam));
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
                DBG_PRINTF("Cy_BLE_GAP_SetSecurityKeys API Error: 0x%x \r\n", apiResult);
            }
        }
        
        /* Send authentication request to peer device */
        cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].bdHandle = appConnHandle.bdHandle;
        apiResult = Cy_BLE_GAP_AuthReq(&cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);
        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("Cy_BLE_GAP_AuthReq API Error: %d \r\n", apiResult);
        }
        break;

    case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
        DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_DISCONNECTED: bdHandle=%x, reason=%x, status=%x\r\n",
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).bdHandle, 
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).reason, 
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).status);
        
        /* Enter discoverable mode so that remote Client could find device. */
        apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("StartAdvertisement API Error: %d \r\n", apiResult);
        }
        break;

    case CY_BLE_EVT_GAP_ENCRYPT_CHANGE:
        DBG_PRINTF("ENCRYPT_CHANGE: %x \r\n", 
            ((cy_stc_ble_gap_encrypt_change_param_t *)
             ((cy_stc_ble_events_param_generic_t *)eventParam)->eventParams)->encryption);
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
        DBG_PRINTF("CY_BLE_EVT_GATT_DISCONNECT_IND \r\n");
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
        
    case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
        DBG_PRINTF("CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ, attHandle: %d \r\n", 
            ((cy_stc_ble_gatts_char_val_read_req_t *)eventParam)->attrHandle);
        break;
        
    case CY_BLE_EVT_GATTS_INDICATION_ENABLED:
        DBG_PRINTF("CY_BLE_EVT_GATTS_INDICATION_ENABLED \r\n");
        break;
    
    case CY_BLE_EVT_GATTS_INDICATION_DISABLED:
        DBG_PRINTF("CY_BLE_EVT_GATTS_INDICATION_DISABLED \r\n");
        break;
        
    case CY_BLE_EVT_GATTC_INDICATION:
        DBG_PRINTF("CY_BLE_EVT_GATTS_INDICATION \r\n");
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
* Function Name: LowPowerImplementation
********************************************************************************
*
* Summary:
*   Implements low power in the project.
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
* Function Name: SimulateWeightMeasurement
********************************************************************************
*
* Summary:
*   Simulates measurements from the weight sensor. Updates weight in both WSS 
*   and UDS records.
*
*******************************************************************************/
void SimulateWeightMeasurement(void)
{
    /* Check if new height or weight are entered by user */
    if(isUserWeightReceived == true)
    {
        weightMeasurement[userIndex].weightKg = udsUserRecord[userIndex].weight;
    }
    if(isUserHeightReceived == true)
    {
        weightMeasurement[userIndex].heightM = udsUserRecord[userIndex].height;
    }
    
    if(weightMeasurement[userIndex].weightKg < SI_MAX_WEIGHT)
    {
        weightMeasurement[userIndex].weightKg = weightMeasurement[userIndex].weightKg + 100u;
    }
    else
    {
        weightMeasurement[userIndex].weightKg = SI_MIN_WEIGHT;
    }

    /* Calculate Body Mass index */
    weightMeasurement[userIndex].bmi = (((weightMeasurement[userIndex].weightKg * SI_WEIGHT_RESOLUTION * 
                                       PERCENT_MODIFIER) /
                                       ((weightMeasurement[userIndex].heightM * weightMeasurement[userIndex].heightM) /
                                       (CM_TO_M_DIVIDER * CM_TO_M_DIVIDER * HEIGHT_METER_MODIFIER))) /
                                       (HEIGHT_METER_MODIFIER)) /
                                       (SI_WEIGHT_RESOLUTION_DIVIDER/10u);

    /* Set flag that there is a data for indication */
    isWssIndicationPending = true;

    DBG_PRINTF("New measurements from weight sensor\r\n");
    DBG_PRINTF("    New weight: %d.%2.2d kg", CONVERT_TO_KILOGRAMS_INT(weightMeasurement[userIndex].weightKg),
                                                   CONVERT_TO_KILOGRAMS_REM(weightMeasurement[userIndex].weightKg));
    DBG_PRINTF(" BMI: %d.%2.1d %% \r\n", (weightMeasurement[userIndex].bmi / 10u), 
                                                   (weightMeasurement[userIndex].bmi % 10u));

    /* Update weight in UDS */
    UdsSetWeight(weightMeasurement[userIndex].weightKg);
}


/*******************************************************************************
* Function Name: HostMain
********************************************************************************
*
* Summary:
*  The main function of the project. The function performs the following 
*  actions:
*   1. Performs initialization of the BLE and ISR components. It also registers
*      all the service's callback functions for the BLE component.
*   2. Invokes handling of the BLE related events.
*   3. Simulates the Weight Scale device operation.
*   4. Performs handling of the low-power modes for the device.
*
*******************************************************************************/
int HostMain(void)
{
    cy_en_ble_api_result_t apiResult;
    uint8_t length;
    uint8_t wssIndData[WSS_WS_MEASUREMENT_MAX_DATA_SIZE];
    uint8_t advUserIndex = UDS_UNKNOWN_USER;
    
    /* FW timer that defines period of weight scale sensor update simulation */
    uint16_t wssSensorUpdateTimer = WSS_SENSOR_TIMER_PERIOD;
    
    /* Initialization the user interface: LEDs, SW2, etc.  */
    InitUserInterface();
            
    UART_DEB_Start();
    DBG_PRINTF("BLE Weight Scale code example \r\n");
    
    /* Start BLE component and register generic event handler */
    apiResult = Cy_BLE_Start(AppCallBack);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_Start API Error: %d \r\n", apiResult);
    }

    /* Initialize BLE services */
    BcsInit();
    WssInit();
    UdsInit();
    
    /* Print stack version */
    PrintStackVersion();

    /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    while(1)
    {
        /* Process all pending BLE events in the stack */
        Cy_BLE_ProcessEvents();

        /* To achieve low power */
        LowPowerImplementation();

        /* Restart 1 second timer */
        if(mainTimer != 0u)
        {
            mainTimer = 0u;
            Cy_BLE_StartTimer(&timerParam);
            if(wssSensorUpdateTimer > 0u)
            {
                wssSensorUpdateTimer--;
            }
        }
        
        if ( ButtonSW2GetStatus() == BUTTON_IS_PRESSED )
        {
            UdsUserChange();
            ButtonSW2SetStatus(BUTTON_IS_NOT_PRESSED);
        }
        
        if(wssSensorUpdateTimer == 0u)
        {
            wssSensorUpdateTimer = WSS_SENSOR_TIMER_PERIOD;

            if(userIndex != UDS_UNKNOWN_USER)
            {
                SimulateWeightMeasurement();
            }
            else
            {
                DBG_PRINTF("No user Registered. Register a new user to start getting weight measurements.\r\n");
            }
            if(advUserIndex != userIndex)
            {
                /* Update advertisement data */
                apiResult = Cy_BLE_WSS_SetAdUserId(sizeof(userIndex), &userIndex, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_WSS_SetAdUserId() API Error: %x \r\n", apiResult);
                }
                else
                {
                    advUserIndex = userIndex;
                }
            }
        }
        
        /* In connection state, check if there is data that
         * should be sent to remote Client.
         */
        if(Cy_BLE_GetConnectionState(appConnHandle) >= CY_BLE_CONN_STATE_CONNECTED)
        {
            /* Handling WSS indications */
            if((isWssIndicationEnabled == true) && (isWssIndicationPending == true))
            {
                length = WSS_WS_MEASUREMENT_MAX_DATA_SIZE;

                if(WssPackIndicationData(wssIndData, &length, &weightMeasurement[userIndex]) == WSS_RET_SUCCESS)
                {
                    if(udsAccessDenied != true)
                    {
                        apiResult =
                            Cy_BLE_WSSS_SendIndication(appConnHandle, CY_BLE_WSS_WEIGHT_MEASUREMENT, length, wssIndData);
                    
                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("Cy_BLE_WSSS_SendIndication() API Error: %x \r\n", apiResult);
                        }
                    }
                    else
                    {
                        DBG_PRINTF("Indication wasn't sent. Please, provide correct consent.\r\n");
                    }
                }
                else
                {
                    DBG_PRINTF("Data packing failed\r\n");
                }
                isWssIndicationPending = false;
            }
            else if(isWssIndicationPending == true)
            {
                DBG_PRINTF("Indication wasn't sent. Indications are disabled.\r\n");
                isWssIndicationPending = false;
            }
            
            /* Handling UDS indications */
            if((isUdsIndicationPending == true) && (isUdsIndicationEnabled == true))
            {
                apiResult = Cy_BLE_UDSS_SendIndication(appConnHandle, CY_BLE_UDS_UCP, udsIndDataSize, ucpResp);
            
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_UDSS_SendIndication() API Error: %x \r\n", apiResult);
                }
                isUdsIndicationPending = false;
            }

            /* Handling UDS notifications */
            if((isUdsNotificationPending == true) && (isUdsNotificationEnabled == true))
            {
                apiResult = Cy_BLE_UDSS_SendNotification(appConnHandle, 
                    CY_BLE_UDS_DCI, UDS_NOTIFICATION_SIZE, (uint8_t *) &udsUserRecord[userIndex].dbChangeIncrement);
            
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_UDSS_SendNotification() API Error: %x \r\n", apiResult);
                }
                isUdsNotificationPending = false;
            }
        }
        
       /* Execute remove device from Bond list if it was required */
        if((Cy_BLE_GetNumOfActiveConn() == 0u) && (App_IsRemoveBondListFlag() == true))
        {
            App_RemoveDevicesFromBondList();
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
