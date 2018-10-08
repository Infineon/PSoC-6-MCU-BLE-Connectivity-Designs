/*******************************************************************************
* File Name: host_main.c
*
* Version 1.0
*
* Description:
*  Simple BLE example project that demonstrates how to configure and use 
*  Cypress's BLE component APIs and application layer callback. Device 
*  Information service is used as an example to demonstrate configuring 
*  BLE service characteristics in the BLE component.
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
#include "user_interface.h"

static cy_stc_ble_timer_info_t  timerParam = { .timeout = ADV_TIMER_TIMEOUT };        
static volatile uint32_t        mainTimer  = 1u;

/*******************************************************************************
* Function Name: AppCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE Component.
*
* Parameters:
*   event - the event code
*   eventParam - the event parameters
*
*******************************************************************************/
void AppCallBack(uint32 event, void *eventParam)
{
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
        /* Mandatory events to be handled by Find Me Target design */
        case CY_BLE_EVT_STACK_ON:
            /* Enter into discoverable mode so that remote can find it */
            (void) Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
           
            /* Generates the security keys */
            (void) Cy_BLE_GAP_GenerateKeys(&keyInfo);
            
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:                             
            /* Start BLE advertisement for 30 seconds and update link status on LEDs */
            Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            break;

        case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
            /* BLE link is established */
            keyInfo.SecKeyParam.bdHandle = (*(cy_stc_ble_gap_connected_param_t *)eventParam).bdHandle;
            Cy_BLE_GAP_SetSecurityKeys(&keyInfo);
            UpdateLedState();   
            break;

        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED)
            {   
                /* Fast and slow advertising period complete, go to low power  
                 * mode (Hibernate) and wait for an external
                 * user event to wake up the device again */
                UpdateLedState();   
                Cy_BLE_Stop();             
            }
            break;

        /* Other events that are generated by the BLE Component that
         * are not required for functioning of this design */
        /**********************************************************
        *                       General Events
        ***********************************************************/
        case CY_BLE_EVT_TIMEOUT: 
            if((((cy_stc_ble_timeout_param_t *)eventParam)->reasonCode == CY_BLE_GENERIC_APP_TO) && 
               (((cy_stc_ble_timeout_param_t *)eventParam)->timerHandle == timerParam.timerHandle))
            {
                /* Update Led State */
                UpdateLedState();
                
                /* Indicate that timer is raised to the main loop */
                mainTimer++;
            }
            break;
            
        case CY_BLE_EVT_HARDWARE_ERROR:    /* This event indicates that some internal HW error has occurred */
            break;

        case CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE:
            /* Hibernate */
            UpdateLedState();
            Cy_SysPm_Hibernate();
            break;
            
        /**********************************************************
        *                       GAP Events
        ***********************************************************/
        case CY_BLE_EVT_GAP_AUTH_REQ:
            if(cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].security == 
                (CY_BLE_GAP_SEC_MODE_1 | CY_BLE_GAP_SEC_LEVEL_1))
            {
                cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].authErr =
                    CY_BLE_GAP_AUTH_ERROR_PAIRING_NOT_SUPPORTED;
            }    
            
            cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].bdHandle = 
                ((cy_stc_ble_gap_auth_info_t *)eventParam)->bdHandle;

            if(Cy_BLE_GAPP_AuthReqReply(&cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]) != 
                    CY_BLE_SUCCESS)
            {
                Cy_BLE_GAP_RemoveOldestDeviceFromBondedList();
                Cy_BLE_GAPP_AuthReqReply(&cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);
            }  
            break;
            
        case CY_BLE_EVT_GAP_PASSKEY_ENTRY_REQUEST:
            break;

        case CY_BLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
            break;

        case CY_BLE_EVT_GAP_AUTH_COMPLETE:
            break;
            
        case CY_BLE_EVT_GAP_AUTH_FAILED:
            break;

        case CY_BLE_EVT_GAP_ENCRYPT_CHANGE:
            break;
            
        case CY_BLE_EVT_GAP_KEYS_GEN_COMPLETE:
            keyInfo.SecKeyParam = (*(cy_stc_ble_gap_sec_key_param_t *)eventParam);
            Cy_BLE_GAP_SetIdAddress(&cy_ble_deviceAddress);
            break;

        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CY_BLE_EVT_GATT_CONNECT_IND:
            break;

        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            break;

        case CY_BLE_EVT_GATTS_XCNHG_MTU_REQ:
            break;

        case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
            break;

        /**********************************************************
        *                       Other Events
        ***********************************************************/
        case CY_BLE_EVT_PENDING_FLASH_WRITE:
            break;

        default:
            break;
    }
}

/*******************************************************************************
* Function Name: LowPowerImplementation()
********************************************************************************
*
* Summary:
*   Implements low power in the project.
*
* Theory:
*  The function tries to enter deep sleep as much as possible - whenever the 
*  BLE is idle.
*
*******************************************************************************/
void LowPowerImplementation(void)
{
    /* Entering into the Deep Sleep */
    Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
}

/*******************************************************************************
* Function Name: DisUpdateFirmWareRevision
********************************************************************************
*
* Summary:
*   Updates the Firmware Revision characteristic with BLE Stack version.
*
*******************************************************************************/
static void DisUpdateFirmWareRevision(void)
{
    cy_stc_ble_stack_lib_version_t stackVersion;
    uint8_t fwRev[9u] = "0.0.0.000";
    
    if(Cy_BLE_GetStackLibraryVersion(&stackVersion) == CY_BLE_SUCCESS)
    {
        /* Transform numbers to ASCII string */
        fwRev[0u] = stackVersion.majorVersion + '0'; 
        fwRev[2u] = stackVersion.minorVersion + '0';
        fwRev[4u] = stackVersion.patch + '0';
        fwRev[6u] = (stackVersion.buildNumber / 100u) + '0';
        stackVersion.buildNumber %= 100u; 
        fwRev[7u] = (stackVersion.buildNumber / 10u) + '0';
        fwRev[8u] = (stackVersion.buildNumber % 10u) + '0';
    }
    
    Cy_BLE_DISS_SetCharacteristicValue(CY_BLE_DIS_FIRMWARE_REV, sizeof(fwRev), fwRev);
}


/*******************************************************************************
* Function Name: HostMain
********************************************************************************
*
* Summary:
*   Main function for the project.
*
* Theory:
*   The function starts BLE.
*   This function processes all BLE events and also implements the low power 
*   functionality.
*
*******************************************************************************/
int HostMain(void)
{   
    const char8 serialNumber[] = "123456"; 
    
    /* Initialization the user interface: LEDs, SW2, etc.  */
    InitUserInterface();

    /* Start BLE component and register generic event handler */
    Cy_BLE_Start(AppCallBack);
     
    /* Set Serial Number string not initialized in GUI */
    Cy_BLE_DISS_SetCharacteristicValue(CY_BLE_DIS_SERIAL_NUMBER, sizeof(serialNumber), (uint8_t *)serialNumber);
    
    /* Updates the Firmware Revision characteristic with BLE Stack version */
    DisUpdateFirmWareRevision();
    
    /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    while(1)
    {
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
    }
}

    
/* [] END OF FILE */
