/*******************************************************************************
* File Name: host_main.c
*
* Version: 1.0
*
* Description:
*  This example project demonstrates following new Bluetooth 4.2 features:
*  1. Data Length Extension (DLE) 
*  2. LE Secure Connection (SC)
*  3. Link Layer Privacy (LL Privacy) 
*
* Related Document:
*  BLUETOOTH SPECIFICATION Version 5.0
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
*  CY5677 CySmart BLE 4.2 USB Dongle 
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
#include <common.h>
#include <custom.h>
#include <BLE.h>
#include "cyfitter_sysint_cfg.h"

    
/*******************************************************************************
* Enumerations
*******************************************************************************/
typedef enum
{
    ADVERTISEMENT_PUBLIC,
    ADVERTISEMENT_PRIVATE
} ADVERTISEMENT_STATE;

typedef enum
{
    AUTHENTICATION_NOT_CONNECTED,
    AUTHENTICATION_WAITING_FOR_PASSKEY,
    AUTHENTICATION_PASSKEY_ENTERED,
    AUTHENTICATION_PASSKEY_YES_NO,    
    AUTHENTICATION_COMPLETE_BONDING_REQD,
    AUTHENTICATION_READ_CENTRAL_ADDRESS_RESOLUTION,
    AUTHENTICATION_READING_CAR,
    AUTHENTICATION_UPDATE_CONN_PARAM,
    AUTHENTICATION_BONDING_COMPLETE,
    AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT,
    AUTHENTICATION_BONDING_REMOVE_GO_AHEAD
} AUTHENTICATION_STATE;


/*******************************************************************************
* Global variables
*******************************************************************************/
ADVERTISEMENT_STATE advState = ADVERTISEMENT_PUBLIC;
AUTHENTICATION_STATE authState = AUTHENTICATION_NOT_CONNECTED;
cy_stc_ble_gap_peer_addr_info_t bondedDeviceInfo[CY_BLE_MAX_BONDED_DEVICES];
cy_stc_ble_gap_bonded_device_list_info_t bondedDeviceList =
{
    .bdHandleAddrList = bondedDeviceInfo
};

cy_stc_ble_conn_handle_t appConnHandle;
cy_stc_ble_gap_auth_info_t peerAuthInfo;
cy_stc_ble_gap_bd_addr_t privateAddress;
uint8_t centralAddressResolutionValue;
cy_stc_ble_resolving_device_info_t rpaInfo;
uint8_t setBdAddrComplete;

uint32_t passkeyValue;
uint32_t passkeyPow;
uint8_t passkeyCounter;

volatile uint32_t mainTimer = 0u;
volatile uint32_t disableLowPowerModeFlag = 0u;
uint16_t connIntv;


/*******************************************************************************
* Function definitions
*******************************************************************************/
void DisplayBondList(void);
uint8_t IsDeviceInBondList(uint8_t bdHandle);
static void AdvertisePrivately(cy_stc_ble_peer_id_addr_t *peerBdAddr);


/*******************************************************************************
* Function Name: AdvertisePublicly()
********************************************************************************
* Summary:
* Starts an advertisement with a public address.
*
* Theory:
* The function starts BLE advertisements with a public address.
*
*******************************************************************************/
static void AdvertisePublicly(void)
{
    uint8_t counter;
    setBdAddrComplete = 0u;
    /* Set the original public address as the device address */
    if(Cy_BLE_IsDeviceAddressValid(cy_ble_sflashDeviceAddress) != 0u)
    {
        memcpy(&cy_ble_deviceAddress, cy_ble_sflashDeviceAddress, sizeof(cy_stc_ble_gap_bd_addr_t));
    }
    Cy_BLE_GAP_SetBdAddress(&cy_ble_deviceAddress);
    
    while(setBdAddrComplete == 0u)
    {
        Cy_BLE_ProcessEvents();
    }
    
    Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
    DBG_PRINTF("Advertising with a public address: ");
    for(counter = CY_BLE_GAP_BD_ADDR_SIZE; counter > 0; counter--)
    {
        DBG_PRINTF("%2.2x ", cy_ble_deviceAddress.bdAddr[counter - 1]);
    }    
    DBG_PRINTF("\r\n");
#if(CY_BLE_LL_PRIVACY_FEATURE_ENABLED)
    /* Disable LL Privacy */
    Cy_BLE_SetAddressResolutionEnable(0u);
#endif  /* CY_BLE_LL_PRIVACY_FEATURE_ENABLED */
}


/*******************************************************************************
* Function Name: AdvertisePrivately()
********************************************************************************
* Summary:
* Starts an advertisement with a random resolvable-private address.
*
* Parameters:
* peerBdAddr - Peer BD address for directed advertising.
*
* Theory:
* The function removes the Device name from the advertisement packet which was
* configured in the BLE component wizard, and starts advertisement with a 
* random resolvable-private address.
*
*******************************************************************************/
static void AdvertisePrivately(cy_stc_ble_peer_id_addr_t *peerBdAddr)
{
    cy_en_ble_api_result_t apiResult;

#if(CY_BLE_LL_PRIVACY_FEATURE_ENABLED)
    /* Enable LL Privacy */
    Cy_BLE_SetAddressResolutionEnable(1u);
    Cy_BLE_ProcessEvents();

    if((centralAddressResolutionValue != 0u) && ((customConfiguration.mode & CY_BLE_CUSTOM_MODE_DIRECTED_ADV) != 0u))
    {   /* Set directed advertising mode */
        cy_ble_discoveryModeInfo[CY_BLE_PERIPHERAL_CONFIGURATION_1_INDEX].advParam->advType = 
            CY_BLE_GAPP_CONNECTABLE_LOW_DC_DIRECTED_ADV;
    }
    else
    {   /* Set undirected advertising mode */
        cy_ble_discoveryModeInfo[CY_BLE_PERIPHERAL_CONFIGURATION_1_INDEX].advParam->advType = 
            CY_BLE_GAPP_CONNECTABLE_UNDIRECTED_ADV;
    }
    /* Set peer address for direct advertising */
    if((peerBdAddr->type == CY_BLE_GAP_ADDR_TYPE_PUBLIC) || (peerBdAddr->type == CY_BLE_GAP_ADDR_TYPE_RANDOM))
    {
        memcpy(cy_ble_discoveryModeInfo[CY_BLE_PERIPHERAL_CONFIGURATION_1_INDEX].advParam->directAddr, 
                                        peerBdAddr->bdAddr, CY_BLE_GAP_BD_ADDR_SIZE);
        cy_ble_discoveryModeInfo[CY_BLE_PERIPHERAL_CONFIGURATION_1_INDEX].advParam->directAddrType = peerBdAddr->type;
    }    
#else
    /* Generate a new resolvable private address */
    cy_stc_ble_gap_bd_addr_info_t bdAddresInfo =
    {
        .addrType = CY_BLE_GAP_RANDOM_PRIV_RESOLVABLE_ADDR,
    };
    memcpy(bdAddresInfo.irkInfo, rpaInfo.localIrk, CY_BLE_GAP_SMP_IRK_SIZE);
    setBdAddrComplete = 0u;
    apiResult = Cy_BLE_GAP_GenerateBdAddress(&bdAddresInfo);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_GAP_GenerateBdAddress API Error: 0x%x \r\n", apiResult);
    }
    /* Generated address is informed and set through 'CY_BLE_EVT_GAP_DEVICE_ADDR_GEN_COMPLETE' event */
    
    while(setBdAddrComplete == 0u)
    {
        Cy_BLE_ProcessEvents();
    }
    
    /* peerBdAddr is not used for legacy pairing */
    (void)peerBdAddr;
    
#endif /* !CY_BLE_LL_PRIVACY_FEATURE_ENABLED */

    #if(CY_BLE_LL_PRIVACY_FEATURE_ENABLED)
        /* Controller generates Resolvable Private Address based on the local IRK from resolving list.*/
        cy_ble_discoveryParam[CY_BLE_PERIPHERAL_CONFIGURATION_1_INDEX].ownAddrType = CY_BLE_GAP_ADDR_TYPE_PUBLIC_RPA;
    #else 
        cy_ble_discoveryParam[CY_BLE_PERIPHERAL_CONFIGURATION_1_INDEX].ownAddrType = CY_BLE_GAP_ADDR_TYPE_RANDOM;
    #endif /* CY_BLE_LL_PRIVACY_FEATURE_ENABLED */
    
    apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_1_INDEX);

    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_GAPP_StartAdvertisement API Error: 0x%x \r\n", apiResult);
    }
    /* Get a new resolvable device address */
    #if(CY_BLE_LL_PRIVACY_FEATURE_ENABLED)
    {
        uint8_t counter;
        
        apiResult = Cy_BLE_GetLocalResolvableAddress(peerBdAddr);        
        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("Cy_BLE_GAP_ReadLocalResolvableAddress API Error: 0x%x \r\n", apiResult);
        }
        if((centralAddressResolutionValue != 0u) && ((customConfiguration.mode & CY_BLE_CUSTOM_MODE_DIRECTED_ADV) != 0u))
        {
            DBG_PRINTF("Directed Advertising to: ");
            
            for(counter = CY_BLE_GAP_BD_ADDR_SIZE; counter > 0u; counter--)
            {
                DBG_PRINTF("%2.2x ", peerBdAddr->bdAddr[counter - 1u]);
            }
        }
    }
    #endif /* CY_BLE_LL_PRIVACY_FEATURE_ENABLED */
    
    DBG_PRINTF("Advertising with a new private address \r\n"); 
}


/*******************************************************************************
* Function Name: StackEventHandler()
********************************************************************************
* Summary:
* Event handler function for the BLE events processing.
*
* Parameters:
* uint32_t eventCode: The event to be processed
* void * eventParam: Pointer to hold the additional information associated 
*                    with an event
*
* Return:
* None
*
* Theory:
* The function is responsible for handling the events generated by the stack.
* The function initiates a pairing request upon connection. 
* In addition to handling general events for BLE advertisement, connection, 
* and disconnection, this function enables/disables notifications for the custom
* characteristic.
*
* Side Effects:
* None
*
*******************************************************************************/
void StackEventHandler(uint32_t event, void * eventParam)
{
    cy_stc_ble_gap_sec_key_param_t * peerKeys;
    uint8_t counter;
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
    
    /* Common events for advertisement with public or private address */
    switch(event)
    {
        case CY_BLE_EVT_STACK_ON:
            DBG_PRINTF("CY_BLE_EVT_STACK_ON \r\n");            /* Stack initialized */
            #if(CY_BLE_BONDING_REQUIREMENT == CY_BLE_BONDING_YES)
            {
                uint8_t deviceCount;
                
                /* Find out whether the device has bonded information stored already or not */
                apiResult = Cy_BLE_GAP_GetBondList(&bondedDeviceList);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAP_GetBondList API Error: 0x%x \r\n", apiResult);
                }

                deviceCount = bondedDeviceList.noOfDevices;
                if(deviceCount != 0u)
                {
                    /* Start advertisement with private address */
                    advState = ADVERTISEMENT_PRIVATE;
                    
                    /* Generate new private address automatically */
                    Cy_BLE_SetResolvablePvtAddressTimeOut(CY_BLE_PVT_ADDRESS_TIMEOUT);
                }
                else
                {
                    /* Start advertisement with public address */
                    advState = ADVERTISEMENT_PUBLIC;
                }
                authState = AUTHENTICATION_NOT_CONNECTED;
            }
            #endif /* CY_BLE_BONDING_REQUIREMENT == CY_BLE_BONDING_YES */
            
            /* Generates the security keys */
            apiResult = Cy_BLE_GAP_GenerateKeys(&keyInfo);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAP_GenerateKeys API Error: 0x%x \r\n", apiResult);
            }               
            break;
            
        case CY_BLE_EVT_TIMEOUT: 
            switch(*(cy_en_ble_to_reason_code_t *)eventParam)
            {
                case CY_BLE_GAP_ADV_TO:
                    DBG_PRINTF("CY_BLE_GAP_ADV_MODE_TO\r\n");
                    break;
                case CY_BLE_GAP_SCAN_TO:
                    DBG_PRINTF("CY_BLE_GAP_SCAN_TO\r\n");
                    break;
                case CY_BLE_GATT_RSP_TO:
                    DBG_PRINTF("CY_BLE_GATT_RSP_TO\r\n");
                    break;
                case CY_BLE_GENERIC_APP_TO:
                    DBG_PRINTF("CY_BLE_GENERIC_APP_TO\r\n");
                    break;
                default:    /* Not existing timeout reason */
                    DBG_PRINTF("%x\r\n", *(uint8_t *)eventParam);
                    break;
            }
            break;
            
        case CY_BLE_EVT_HARDWARE_ERROR:    /* This event indicates that some internal HW error has occurred. */
            DBG_PRINTF("CY_BLE_EVT_HARDWARE_ERROR \r\n");
            break;
            
        case CY_BLE_EVT_STACK_BUSY_STATUS:
            break;
            
        case CY_BLE_EVT_PENDING_FLASH_WRITE:
            DBG_PRINTF("CY_BLE_EVT_PENDING_FLASH_WRITE\r\n");
            break;
            
        case CY_BLE_EVT_LE_SET_EVENT_MASK_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_LE_SET_EVENT_MASK_COMPLETE \r\n");
            break;
            
        case CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE \r\n");
            
            apiResult = Cy_BLE_GAP_GetBdAddress();
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAP_GetBdAddress API Error: 0x%x \r\n", apiResult);
            }
            break;
            
        case CY_BLE_EVT_GET_DEVICE_ADDR_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GET_DEVICE_ADDR_COMPLETE: public:");
            for(counter = CY_BLE_GAP_BD_ADDR_SIZE; counter > 0u; counter--)
            {
                DBG_PRINTF("%2.2x", ((cy_stc_ble_bd_addrs_t *)((cy_stc_ble_events_param_generic_t *)eventParam)->
                                                                 eventParams)->publicBdAddr[counter-1]);
            }
            DBG_PRINTF(", private:");
            for(counter = CY_BLE_GAP_BD_ADDR_SIZE; counter > 0u; counter--)
            {
                DBG_PRINTF("%2.2x", ((cy_stc_ble_bd_addrs_t *)((cy_stc_ble_events_param_generic_t *)eventParam)->
                                                                 eventParams)->privateBdAddr[counter-1]);
            }
            DBG_PRINTF("\r\n");
            setBdAddrComplete = 1u;
            break;
            
        case CY_BLE_EVT_GET_LOCAL_RPA_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GET_LOCAL_RPA_COMPLETE: ");
            for(counter = CY_BLE_GAP_BD_ADDR_SIZE; counter > 0u; counter--)
            {
                DBG_PRINTF("%2.2x", ((uint8_t *)
                                    ((cy_stc_ble_events_param_generic_t *)eventParam)->eventParams)[counter-1]);
            }
            DBG_PRINTF("\r\n");
            break;
            
        case CY_BLE_EVT_SET_RPA_TO_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_SET_RPA_TO_COMPLETE \r\n");
            break;
            
        case CY_BLE_EVT_SET_RPA_ENABLE_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_SET_RPA_ENABLE_COMPLETE \r\n");
            break;
            
        case CY_BLE_EVT_SET_HOST_CHANNEL_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_SET_HOST_CHANNEL_COMPLETE \r\n");
            break;
            
        case CY_BLE_EVT_ADD_DEVICE_TO_RPA_LIST_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_ADD_DEVICE_TO_RPA_LIST_COMPLETE \r\n");
            break;
            
        case CY_BLE_EVT_REMOVE_DEVICE_FROM_RPA_LIST_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_REMOVE_DEVICE_FROM_RPA_LIST_COMPLETE \r\n");
            break;
            
        case CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE \r\n");
            break;
            
        case CY_BLE_EVT_SET_SUGGESTED_DATA_LENGTH_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_SET_SUGGESTED_DATA_LENGTH_COMPLETE \r\n");
            break;
            
        case CY_BLE_EVT_SET_TX_PWR_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_SET_TX_PWR_COMPLETE \r\n");
            break;
            
        case CY_BLE_EVT_SET_DEFAULT_PHY_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_SET_DEFAULT_PHY_COMPLETE ");
            DBG_PRINTF("status=%x \r\n",     ((cy_stc_ble_events_param_generic_t *)eventParam)->status);
            break;
            
        case CY_BLE_EVT_SET_PHY_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_SET_PHY_COMPLETE ");
            DBG_PRINTF("status=%x \r\n",     ((cy_stc_ble_events_param_generic_t *)eventParam)->status);
            break;
            
        case CY_BLE_EVT_PHY_UPDATE_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_PHY_UPDATE_COMPLETE: ");
            DBG_PRINTF("status=%x, ",     
                        ((cy_stc_ble_events_param_generic_t *)eventParam)->status);
            DBG_PRINTF("bdHandle=%x, ",  ((cy_stc_ble_phy_param_t *)
                                         ((cy_stc_ble_events_param_generic_t *)eventParam)->eventParams)->bdHandle);
            DBG_PRINTF("rxPhyMask=%x, ", ((cy_stc_ble_phy_param_t *)
                                         ((cy_stc_ble_events_param_generic_t *)eventParam)->eventParams)->rxPhyMask);
            DBG_PRINTF("txPhyMask=%x, ", ((cy_stc_ble_phy_param_t *)
                                         ((cy_stc_ble_events_param_generic_t *)eventParam)->eventParams)->txPhyMask);
            DBG_PRINTF("\r\n");
            break;
            
        /**********************************************************
        *                       GAP Events
        ***********************************************************/
        case CY_BLE_EVT_GAP_DEVICE_ADDR_GEN_COMPLETE:
            privateAddress = *(cy_stc_ble_bd_addr_t *) eventParam;
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_ADDR_GEN_COMPLETE: type: %x, addr: ", privateAddress.type);
            for(counter = CY_BLE_GAP_BD_ADDR_SIZE; counter > 0u; counter--)
            {
                DBG_PRINTF("%2.2x", privateAddress.bdAddr[counter-1]);
            }
            DBG_PRINTF("\r\n");
            
            /* Set a new device address */
            apiResult = Cy_BLE_GAP_SetBdAddress(&privateAddress);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAP_SetBdAddress API Error: 0x%x \r\n", apiResult);
            }
            break;
            
        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            DBG_PRINTF("CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP, state: %d \r\n", Cy_BLE_GetAdvertisementState());
            /* If advertisement timed out and we were not connected 
             * yet, restart advertisement.
             */
            if(authState == AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT)
            {
                DBG_PRINTF("Advertisement stopped.\r\n");
                authState = AUTHENTICATION_BONDING_REMOVE_GO_AHEAD;
                UpdateLedState();            
            }
            break;       
            
    #if(CY_BLE_LL_PRIVACY_FEATURE_ENABLED)
        case CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE:
        {
            cy_stc_ble_gap_enhance_conn_complete_param_t *enhConnParameters;
            uint8_t connectedBdHandle;
            
            enhConnParameters = (cy_stc_ble_gap_enhance_conn_complete_param_t *)eventParam;
            DBG_PRINTF("CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE: %x, %x(%.2f ms), %x \r\n",
                enhConnParameters->status,
                enhConnParameters->connIntv,
                (float)enhConnParameters->connIntv * CY_BLE_CONN_INTRV_TO_MS,
                enhConnParameters->connLatency);
            connIntv = enhConnParameters->connIntv;
            connectedBdHandle = enhConnParameters->bdHandle;
    #else    
        case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
        {
            uint8_t connectedBdHandle;
            
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_CONNECTED: %x, %x(%.2f ms), %x, %x \r\n",   
                ((cy_stc_ble_gap_connected_param_t *)eventParam)->status,
                ((cy_stc_ble_gap_connected_param_t *)eventParam)->connIntv,
                (float)((cy_stc_ble_gap_connected_param_t *)eventParam)->connIntv * CY_BLE_CONN_INTRV_TO_MS,
                ((cy_stc_ble_gap_connected_param_t *)eventParam)->connLatency,
                ((cy_stc_ble_gap_connected_param_t *)eventParam)->supervisionTO);
            connIntv = ((cy_stc_ble_gap_connected_param_t *)eventParam)->connIntv;
            connectedBdHandle = ((cy_stc_ble_gap_connected_param_t *)eventParam)->bdHandle;
    #endif  /* CY_BLE_LL_PRIVACY_FEATURE_ENABLED */
            /* Continue connection case without a break */   
    
            /* Initiate pairing process */
            if((cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].security & CY_BLE_GAP_SEC_LEVEL_MASK) > 
                CY_BLE_GAP_SEC_LEVEL_1)
            {
                cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].bdHandle = appConnHandle.bdHandle;
                apiResult = Cy_BLE_GAP_AuthReq(&cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF(" Cy_BLE_GAP_AuthReq API Error: 0x%x \r\n", apiResult);
                }
            }
            else
            {
                authState = AUTHENTICATION_UPDATE_CONN_PARAM;
            }
            UpdateLedState();
            
            /* Set security keys for new device which is not already bonded */
            if(IsDeviceInBondList(connectedBdHandle) == 0u)
            {
                keyInfo.SecKeyParam.bdHandle = connectedBdHandle;
                apiResult = Cy_BLE_GAP_SetSecurityKeys(&keyInfo);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAP_SetSecurityKeys API Error: 0x%x \r\n", apiResult);
                }
            }   
        }
            break;
            
        case CY_BLE_EVT_GAP_KEYS_GEN_COMPLETE:
            DBG_PRINTF("CY_BLE_EVT_GAP_KEYS_GEN_COMPLETE \r\n");
            keyInfo.SecKeyParam = (*(cy_stc_ble_gap_sec_key_param_t *)eventParam);
            apiResult = Cy_BLE_GAP_SetIdAddress(&cy_ble_deviceAddress);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAP_SetIdAddress API Error: 0x%x \r\n", apiResult);
            }
            
            #if(CY_BLE_CONFIG_ENABLE_PHY_UPDATE != 0u)
            {
                const cy_stc_ble_set_suggested_phy_info_t phyInfo =
                {
                    .allPhyMask = CY_BLE_PHY_NO_PREF_MASK_NONE,
                    .txPhyMask = CY_BLE_PHY_MASK_LE_2M,
                    .rxPhyMask = CY_BLE_PHY_MASK_LE_2M
                };
                apiResult = Cy_BLE_SetDefaultPhy(&phyInfo);            
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_SetDefaultPhy API Error: 0x%x \r\n", apiResult);
                }
            }
            #endif  /* (CY_BLE_CONFIG_ENABLE_PHY_UPDATE != 0u) */          
            break;
            
        case CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:   
            DBG_PRINTF("CY_BLE_EVT_GAPC_CONNECTION_UPDATE_COMPLETE: %x, %x(%.2f ms), %x, %x \r\n",
                ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->status,
                ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->connIntv,
                (float)((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->connIntv * 5 / 4,
                ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->connLatency,
                ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->supervisionTO);
            connIntv = ((cy_stc_ble_gap_conn_param_updated_in_controller_t *)eventParam)->connIntv;
            break;
            
        case CY_BLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP:
            DBG_PRINTF("CY_BLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP: %s\r\n", 
                *(uint16_t *)eventParam == 0u ? "Accepted" : "Rejected");
            break;    
            
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
            DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_DISCONNECTED: bdHandle=%x, reason=%x, status=%x\r\n",
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).bdHandle, 
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).reason, 
                (*(cy_stc_ble_gap_disconnect_param_t *)eventParam).status);
            totalByteCounter = 0u;
            UpdateLedState();       
            /* Disconnected but bonding is not complete; restart advertisement. */
            if(authState == AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT)
            {
                authState = AUTHENTICATION_BONDING_REMOVE_GO_AHEAD;
            }
            else
            {
                authState = AUTHENTICATION_NOT_CONNECTED;
            }
            break;
            
        case CY_BLE_EVT_GAP_AUTH_REQ:
            peerAuthInfo = *(cy_stc_ble_gap_auth_info_t *)eventParam;
            DBG_PRINTF("EVT_AUTH_REQ: security=%x, bonding=%x, ekeySize=%x, err=%x \r\n", 
                peerAuthInfo.security, 
                peerAuthInfo.bonding, 
                peerAuthInfo.ekeySize, 
                peerAuthInfo.authErr);
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
                DBG_PRINTF("Cy_BLE_GAPP_AuthReqReply API Error:"
                           " 0x%x, call Cy_BLE_GAP_RemoveOldestDeviceFromBondedList\r\n", apiResult);
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
            DBG_PRINTF("Enter the passkey shown in the peer device: ");
            authState = AUTHENTICATION_WAITING_FOR_PASSKEY;
            passkeyValue = 0u;
            passkeyPow = 100000u;
            passkeyCounter = 0u;
            if(((peerAuthInfo.security & CY_BLE_GAP_SEC_LEVEL_MASK) == CY_BLE_GAP_SEC_LEVEL_4) &&
               ((peerAuthInfo.pairingProperties & CY_BLE_GAP_SMP_SC_PAIR_PROP_KP_MASK) != 0u))
            {
                cy_stc_ble_gap_sc_kp_notif_info_t scKpNotifParam =
                {
                    .bdHandle = appConnHandle.bdHandle,
                    .notificationType = CY_BLE_GAP_PASSKEY_ENTRY_STARTED
                };
                apiResult = Cy_BLE_GAP_AuthSendKeyPress(&scKpNotifParam);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAP_AuthSendKeyPress API Error: 0x%x \r\n", apiResult);
                }
            }
            break;
            
    #if(CY_BLE_SECURE_CONN_FEATURE_ENABLED)
        case CY_BLE_EVT_GAP_NUMERIC_COMPARISON_REQUEST:
            DBG_PRINTF("Compare this passkey with displayed in your peer device and press 'y' or 'n': %6.6ld \r\n", 
                *(uint32_t *)eventParam);
            authState = AUTHENTICATION_PASSKEY_YES_NO;
            break;
            
        case CY_BLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO:
            peerAuthInfo = *(cy_stc_ble_gap_auth_info_t *)eventParam;
            DBG_PRINTF("CY_BLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO: security:%x, bonding:%x, ekeySize:%x, authErr %x \r\n", 
                peerAuthInfo.security, 
                peerAuthInfo.bonding, 
                peerAuthInfo.ekeySize, 
                peerAuthInfo.authErr);
            break;
    #endif /* CY_BLE_SECURE_CONN_FEATURE_ENABLED */

        case CY_BLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
            DBG_PRINTF("Enter this passkey in your peer device: %6.6ld \r\n", *(uint32_t *)eventParam);
            break;
            
        case CY_BLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
            DBG_PRINTF("CY_BLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT \r\n");
            /* Copy the IRK key to a buffer; to be used for creating a 
             * private address.
             */
            peerKeys = (cy_stc_ble_gap_sec_key_param_t *) eventParam;
            memcpy(rpaInfo.peerIrk, peerKeys->irkInfo, CY_BLE_GAP_SMP_IRK_SIZE);
            break;
            
        case CY_BLE_EVT_GAP_AUTH_COMPLETE:
            /* Authentication complete; initiate bonding */
            peerAuthInfo = *(cy_stc_ble_gap_auth_info_t *)eventParam;
            DBG_PRINTF("AUTH_COMPLETE: security:%x, bonding:%x, ekeySize:%x, authErr %x \r\n", 
                peerAuthInfo.security, 
                peerAuthInfo.bonding, 
                peerAuthInfo.ekeySize, 
                peerAuthInfo.authErr);
            /* Check that peer device supports bonding */
            if(peerAuthInfo.bonding != 0u)
            {
                authState = AUTHENTICATION_COMPLETE_BONDING_REQD;
                #if(CY_BLE_LL_PRIVACY_FEATURE_ENABLED)
                {
                    cy_stc_ble_gap_peer_addr_info_t peerAddrInfo =
                    {
                        .bdHandle = appConnHandle.bdHandle
                    };
                    
                    memcpy(rpaInfo.localIrk, keyInfo.SecKeyParam.irkInfo, CY_BLE_GAP_SMP_IRK_SIZE);
                    
                    apiResult = Cy_BLE_GAP_GetPeerBdAddr(&peerAddrInfo);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_GAP_GetPeerBdAddr API Error: 0x%x \r\n", apiResult);
                    }
                    else
                    {
                        memcpy(rpaInfo.bdAddr, peerAddrInfo.bdAddr.bdAddr, CY_BLE_GAP_BD_ADDR_SIZE);
                        rpaInfo.type = peerAddrInfo.bdAddr.type;
                    }
                    
                    /* Add device to resolving list */
                    apiResult = Cy_BLE_AddDeviceToResolvingList(&rpaInfo);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_AddDeviceToResolvingList API Error: 0x%x \r\n", apiResult);
                    }
                    /* Generate new private address automatically */
                    apiResult = Cy_BLE_SetResolvablePvtAddressTimeOut(CY_BLE_PVT_ADDRESS_TIMEOUT); 
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_SetResolvablePvtAddressTimeOut API Error: 0x%x \r\n", apiResult);
                    }
                }
                #endif /* CY_BLE_LL_PRIVACY_FEATURE_ENABLED */
            }
            else /* Use public advertisement if bonding is not supported by peer device */
            {
                advState = ADVERTISEMENT_PUBLIC;
                authState = AUTHENTICATION_UPDATE_CONN_PARAM;                
            }
            break;
            
        case CY_BLE_EVT_GAP_AUTH_FAILED:
            DBG_PRINTF("EVT_GAP_AUTH_FAILED, reason: ");
            switch(*(cy_en_ble_gap_auth_failed_reason_t *)eventParam)
            {
                case CY_BLE_GAP_AUTH_ERROR_CONFIRM_VALUE_NOT_MATCH:
                    DBG_PRINTF("CONFIRM_VALUE_NOT_MATCH\r\n");
                    break;
                    
                case CY_BLE_GAP_AUTH_ERROR_INSUFFICIENT_ENCRYPTION_KEY_SIZE:
                    DBG_PRINTF("INSUFFICIENT_ENCRYPTION_KEY_SIZE\r\n");
                    break;
                
                case CY_BLE_GAP_AUTH_ERROR_UNSPECIFIED_REASON:
                    DBG_PRINTF("UNSPECIFIED_REASON\r\n");
                    break;
                    
                case CY_BLE_GAP_AUTH_ERROR_AUTHENTICATION_TIMEOUT:
                    DBG_PRINTF("AUTHENTICATION_TIMEOUT\r\n");
                    break;
                    
                default:
                    DBG_PRINTF("0x%x  \r\n", *(cy_en_ble_gap_auth_failed_reason_t *)eventParam);
                    break;
            }
            break;
            
        case CY_BLE_EVT_GAP_ENCRYPT_CHANGE:
            DBG_PRINTF("ENCRYPT_CHANGE: %x \r\n", *(uint8_t *)eventParam);
            break;
            
    #if(CY_BLE_DLE_FEATURE_ENABLED)
        case CY_BLE_EVT_DATA_LENGTH_CHANGE:
        {
            cy_stc_ble_data_length_change_event_param_t *connDataLen;
            
            connDataLen = (cy_stc_ble_data_length_change_event_param_t *)eventParam;
            DBG_PRINTF("CY_BLE_EVT_GAP_DATA_LENGTH_CHANGE: maxTx: %d , maxRx: %d \r\n", 
                connDataLen->connMaxTxOctets, connDataLen->connMaxRxOctets);
            (void)connDataLen;  /* Suppress a warning for disabled UART mode */
        }
            break;
        case CY_BLE_EVT_GAP_KEYPRESS_NOTIFICATION:
            DBG_PRINTF("CY_BLE_EVT_GAP_KEYPRESS_NOTIFICATION %d \r\n", *(uint8_t *) eventParam);
            break;
    #endif /* CY_BLE_DLE_FEATURE_ENABLED */
    
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

         /* Events handled in custom service */
        case CY_BLE_EVT_GATTS_WRITE_REQ:
        case CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF:
        case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
            CustomCallBack(event, eventParam);
            break;
            
        case CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP:
            {
                cy_stc_ble_gattc_grp_attr_data_list_t *locAttrData;
                
                locAttrData = &(*(cy_stc_ble_gattc_read_by_grp_rsp_param_t *)eventParam).attrData;
                
                DBG_PRINTF("EVT_GATT_READ_BY_TYPE_RSP len=%x, value: ", locAttrData->attrLen);
                for(counter = 0u; counter < locAttrData->attrLen; counter++)
                { 
                    DBG_PRINTF("%2.2x ",*(uint8_t *)(locAttrData->attrValue + counter));
                }
                DBG_PRINTF("\r\n");

                if(authState == AUTHENTICATION_READING_CAR)
                {
                    if(locAttrData->attrLen > CY_BLE_DB_ATTR_HANDLE_LEN)
                    {
                        centralAddressResolutionValue = *(uint8_t *)(locAttrData->attrValue + CY_BLE_DB_ATTR_HANDLE_LEN);
                        /* Central Address Resolution characteristic is supported by peer device */
                        DBG_PRINTF("Central Address Resolution value = %x \r\n", centralAddressResolutionValue);
                    }
                    authState = AUTHENTICATION_UPDATE_CONN_PARAM;
                }
            }
            break;
            
        case CY_BLE_EVT_GATTC_ERROR_RSP:
            if(authState == AUTHENTICATION_READING_CAR)
            {
                /* Central Address Resolution is not supported by peer device */
                DBG_PRINTF("Central Address Resolution characteristic is absent \r\n");
                authState = AUTHENTICATION_UPDATE_CONN_PARAM;
            }
            break;
            
        case CY_BLE_EVT_GATTS_INDICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_GATTS_INDICATION_ENABLED \r\n");
            break;
            
        case CY_BLE_EVT_GATTS_INDICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_GATTS_INDICATION_DISABLED \r\n");
            dataSendEnabled = 0;   /* hold the transfer and the write responce to send */
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
#if(CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV)
    if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
    {        
        Disconnect_LED_Write(LED_OFF);
        Simulation_LED_Write(LED_OFF);
        Advertising_LED_INV();
    }
    else if(Cy_BLE_GetState() == CY_BLE_STATE_INITIALIZING || Cy_BLE_GetState() == CY_BLE_STATE_STOPPED)
    {   
        Advertising_LED_Write(LED_OFF);
        Disconnect_LED_Write(LED_OFF);
        Simulation_LED_Write(LED_OFF);
    }
    else if(Cy_BLE_GetNumOfActiveConn() == 0u)
    {   
        Advertising_LED_Write(LED_OFF);
        Disconnect_LED_Write(LED_ON);
        Simulation_LED_Write(LED_OFF);
    }

#else
    if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
    {
        /* Blink advertising indication LED. */
        LED5_INV();
    }
    else if(Cy_BLE_GetNumOfActiveConn() == 0u)
    {
        /* If in disconnected state, turn on disconnect indication LED */
        LED5_Write(LED_ON);
    }
    else 
    {
        /* In connected state */
        LED5_Write(LED_OFF);
    }
#endif /* #if(CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV) */    
}


/*******************************************************************************
* Function Name: DisplayBondList()
********************************************************************************
* Summary:
*  This function displays bonding list and updates Resolved list.
*
*******************************************************************************/
void DisplayBondList(void)
{
    uint8_t deviceCount;
    cy_en_ble_api_result_t apiResult;
    
    /* Find out whether the device has bonded information stored already or not */
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
            uint8_t counter;
            
            DBG_PRINTF("Bond list: \r\n");
            do
            {
                DBG_PRINTF("%d. ", deviceCount);
                deviceCount--;
                
                if(bondedDeviceList.bdHandleAddrList[deviceCount].bdAddr.type == CY_BLE_GAP_ADDR_TYPE_RANDOM)
                {
                    DBG_PRINTF("Peer Random Address:");
                }
                else
                {
                    DBG_PRINTF("Peer Public Address:");
                }
                for(counter = CY_BLE_GAP_BD_ADDR_SIZE; counter > 0u; counter--)
                {
                    DBG_PRINTF(" %2.2x", bondedDeviceList.bdHandleAddrList[deviceCount].bdAddr.bdAddr[counter - 1u]);
                }
                DBG_PRINTF(", bdHandle: %x, ", bondedDeviceList.bdHandleAddrList[deviceCount].bdHandle);

                /* Get the IRK for currently processed device */ 
                {
                    cy_stc_ble_gap_sec_key_info_t secKeyInfo =
                    {
                        .SecKeyParam.bdHandle = bondedDeviceList.bdHandleAddrList[deviceCount].bdHandle
                    };
                    apiResult = Cy_BLE_GAP_GetPeerDevSecurityKeyInfo(&secKeyInfo);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_GAP_GetPeerDevSecurityKeyInfo API Error: 0x%x \r\n", apiResult);
                    }
                    
                    /* rpaInfo.peerIrk is used by Cy_BLE_GAP_GenerateBdAddress() */
                    memcpy(rpaInfo.peerIrk, secKeyInfo.SecKeyParam.irkInfo, CY_BLE_GAP_SMP_IRK_SIZE);
                
                    DBG_PRINTF("peer IRK:");
                    for(counter = 0u; counter < CY_BLE_GAP_SMP_IRK_SIZE; counter++)
                    {
                        DBG_PRINTF(" %2.2x",rpaInfo.peerIrk[counter]);
                    }
        
                    /* Get the local keys */
                    apiResult = Cy_BLE_GAP_GetLocalDevSecurityKeyInfo(&secKeyInfo);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_GAP_GetLocalDevSecurityKeyInfo API Error: 0x%x \r\n", apiResult);
                    }
        
                    memcpy(rpaInfo.localIrk, secKeyInfo.SecKeyParam.irkInfo, CY_BLE_GAP_SMP_IRK_SIZE);
                    DBG_PRINTF(", local IRK:");
                    for(counter = 0u; counter < CY_BLE_GAP_SMP_IRK_SIZE; counter++)
                    {
                        DBG_PRINTF(" %2.2x",rpaInfo.localIrk[counter]);
                    }
                #if(DEBUG_UART_SHOW_BOND_LIST == ENABLED)
                    /* Show other keys */
                    DBG_PRINTF("\r\n Long Term Key:");
                    for(counter = 0u; counter < CY_BLE_GAP_SMP_LTK_SIZE; counter++)
                    {
                        DBG_PRINTF(" %2.2x", secKeyInfo.SecKeyParam.ltkInfo[counter]); 
                    }
                    DBG_PRINTF("\r\n Encrypted Diversifier and Random Number:");
                    for(counter = 0u; counter < CY_BLE_GAP_SMP_MID_INFO_SIZE; counter++)
                    {
                        DBG_PRINTF(" %2.2x",secKeyInfo.SecKeyParam.midInfo[counter]);
                    }
                    DBG_PRINTF("\r\n Public device/Static Random address type:");
                    for(counter = 0u; counter < CY_BLE_GAP_SMP_IDADDR_DATA_SIZE; counter++)
                    {
                        DBG_PRINTF(" %2.2x", secKeyInfo.SecKeyParam.idAddrInfo[counter]);
                    }
                    DBG_PRINTF("\r\n Connection Signature Resolving Key:");
                    for(counter = 0u; counter < CY_BLE_GAP_SMP_CSRK_SIZE; counter++)
                    {
                        DBG_PRINTF(" %2.2x", secKeyInfo.SecKeyParam.csrkInfo[counter]);
                    }
                #endif /* DEBUG_UART_SHOW_BOND_LIST == ENABLED */
                }
                DBG_PRINTF("\r\n");
            } while(deviceCount != 0u);
        }
        else
        {
            DBG_PRINTF("Bond list is empty. \r\n");
        }
        #if(CY_BLE_LL_PRIVACY_FEATURE_ENABLED && (DEBUG_UART_SHOW_BOND_LIST == ENABLED))
        {
            cy_stc_ble_resolving_list_retention_t const *resolvingDeviceList;
            uint8_t counter;
            
            apiResult = Cy_BLE_GetResolvingList(&resolvingDeviceList);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAP_ReadResolvingList API Error: 0x%x \r\n", apiResult);
            }
            else
            {
                deviceCount = resolvingDeviceList->size;

                if(deviceCount != 0u)
                {
                    DBG_PRINTF("Resolving list: \r\n");
                    do
                    {
                        DBG_PRINTF("%d. ", deviceCount);
                        deviceCount--;
                        
                        if(resolvingDeviceList->resolvingList[deviceCount].type == CY_BLE_GAP_ADDR_TYPE_RANDOM)
                        {
                            DBG_PRINTF("Peer Random Address:");
                        }
                        else
                        {
                            DBG_PRINTF("Peer Public Address:");
                        }
                        for(counter = CY_BLE_GAP_BD_ADDR_SIZE; counter > 0u; counter--)
                        {
                            DBG_PRINTF(" %2.2x", resolvingDeviceList->resolvingList[deviceCount].bdAddr[counter - 1u]);
                        }
                        
                        DBG_PRINTF(", peer IRK:");
                        for(counter = 0u; counter < CY_BLE_IRK_SIZE; counter++)
                        {
                            DBG_PRINTF(" %2.2x", resolvingDeviceList->resolvingList[deviceCount].peerIrk[counter]);
                        }

                        DBG_PRINTF(", local IRK:");
                        for(counter = 0u; counter < CY_BLE_GAP_SMP_IRK_SIZE; counter++)
                        {
                            DBG_PRINTF(" %2.2x",resolvingDeviceList->resolvingList[deviceCount].localIrk[counter]);
                        }
                        DBG_PRINTF("\r\n");
                    } while(deviceCount != 0u);
                }
            }
        }
        #endif /* CY_BLE_LL_PRIVACY_FEATURE_ENABLED */
    }
}


/*******************************************************************************
* Function Name: IsDeviceInBondList()
********************************************************************************
* Summary:
*  This function returns nonzero value when bdHandle exists in bond list
*
*******************************************************************************/
uint8_t IsDeviceInBondList(uint8_t bdHandle)
{
    uint8_t deviceCount;
    cy_en_ble_api_result_t apiResult;
    uint8_t deviceIsDetected = 0u;

    /* Find out whether the device has bonded information stored already or not */
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
* Function Name: PrintMenu
********************************************************************************
*
* Print Menu
*
*******************************************************************************/
static void DisplayMenu(void)
{
    DBG_PRINTF("\r\nSelect operation:\r\n"); 
    DBG_PRINTF("\'r' -- Remove the bond. \r\n"); 
    DBG_PRINTF("\'s' -- Display bond list. \r\n\r\n"); 
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
       (authState != AUTHENTICATION_PASSKEY_YES_NO) && 
       (authState != AUTHENTICATION_WAITING_FOR_PASSKEY) &&
       (disableLowPowerModeFlag == 0))
    {
        cy_en_clkpath_in_sources_t clockSource;

        clockSource = Cy_SysClk_ClkPathGetSource(0u);
        
        if(clockSource == CY_SYSCLK_CLKPATH_IN_ALTHF)
        {
            /* Disable FLL */
            Cy_SysClk_FllDisable();
            
            /* BLE ECO is not available during BLESS deep sleep mode, change CPU clock source to IMO */
            Cy_SysClk_ClkPathSetSource(0u, CY_SYSCLK_CLKPATH_IN_IMO);
        }
        
        /* Entering into the Deep Sleep */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
        
        if(clockSource == CY_SYSCLK_CLKPATH_IN_ALTHF)
        {
            /* Check the status of BLESS is active */
            while(Cy_BLE_StackGetBleSsState() == CY_BLE_BLESS_STATE_DEEPSLEEP);
            /* Return clock source to BLE ECO */
            Cy_SysClk_ClkPathSetSource(0u, CY_SYSCLK_CLKPATH_IN_ALTHF);
            
            /* Enable FLL */
            Cy_SysClk_FllEnable(0);
        
            /* wait on FLL to be locked */
            while(Cy_SysClk_FllLocked());
        }
        
        if(disableLowPowerModeFlag  != 0u)
        {
            DBG_PRINTF("INFO: Low Power Mode is DISABLED \r\n");
            /* Display main operation menu */
            DisplayMenu();
        }
    }
}


/*******************************************************************************
* Function Name: TimerInterrupt
********************************************************************************
*
* Summary:
*  Handles the Interrupt Service Routine for the MCWDT timer.
*
*******************************************************************************/
void TimerInterrupt(void)
{
    /* Blink LED to indicate that device advertises */
    UpdateLedState();
    if(Cy_BLE_GetNumOfActiveConn() > 0u)
    {
        if(dataSendEnabled != 0u)
        {
            /* Indicate that timer is raised to the main loop */
            mainTimer++;
        }
    }
    MCWDT_ClearInterrupt(CY_MCWDT_CTR0);
}


/*******************************************************************************
* Function Name: WakeupInt
********************************************************************************
*
* Summary:
*   Handles all port interrupt for wakeup from deepsleep to receive data by UART.
*
*******************************************************************************/
void WakeupInterrupt(void)
{
    if (Cy_GPIO_GetInterruptStatusMasked(UART_RX_0_PORT, UART_RX_0_NUM) == 1u)  
    { 
        if((disableLowPowerModeFlag == 0u) &&
           (authState != AUTHENTICATION_PASSKEY_YES_NO) && 
           (authState != AUTHENTICATION_WAITING_FOR_PASSKEY))
        {
            
            /* Set flag to disable LPM */
            disableLowPowerModeFlag  = 1u;
            /* Wait 1ms and clean UART to skip false start bit detection */
            Cy_SysLib_Delay(1);   
            UART_DEB_SCB_CLEAR_RX_FIFO();
        }
        
        /* Clears the triggered pin interrupt. */   
        Cy_GPIO_ClearInterrupt(UART_RX_0_PORT, UART_RX_0_NUM);
    }   
}


/*******************************************************************************
* Function Name: HostMain()
********************************************************************************
* Summary:
* The top level application function.
*
* Theory:
* Initializes the BLE and UART components and then processes BLE events regularly.
* Also decides whether advertisement with public or private address is to be 
* done. Plus it maintains the bonding aspect - whether to add a device to bond
* list or to remove it.
* Initializes a buffer from which data is sent out over BLE.
* Sends characteristic notification packets over BLE.
*
* For more information on privacy feature, refer to Bluetooth 4.2 Specification, 
* Volume 3, Part C, Section 10.7.
*
*******************************************************************************/
int HostMain(void)
{
    char8 command;
    uint8_t customCommand = 0u;
    uint8_t button = 0u;
    cy_en_ble_api_result_t apiResult;
    static uint8_t limitedStarted = 0u;
    cy_stc_ble_gap_auth_pk_info_t authPkParam;
    
    /* Initialize LEDs */
    DisableAllLeds();
    
    /* Initialize Wakeup */
    Cy_SysInt_Init(&SW2_Int_cfg, WakeupInterrupt);
    NVIC_EnableIRQ(SW2_Int_cfg.intrSrc);   
        
    /* Initialize Debug UART */
    UART_START();
    DBG_PRINTF("BLE 4.2 DataLength Security Privacy Code Example\r\n");
    
    /* Start BLE component and register generic event handler */
    apiResult = Cy_BLE_Start(StackEventHandler);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_Start API Error: 0x%x \r\n", apiResult);
    }
    /* Initialize custom service */
    CustomInit();
    
    /* Print stack version */
    PrintStackVersion();
    
    /* Initialize Timer */
    Cy_SysInt_Init(&Timer_Int_cfg, TimerInterrupt);
    NVIC_EnableIRQ(Timer_Int_cfg.intrSrc);   
    MCWDT_SetInterruptMask(CY_MCWDT_CTR0);
    MCWDT_Start();
    
    /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    while(1)
    {
        
        /* Process pending BLE events */
        Cy_BLE_ProcessEvents();
        
        /* Start Advertising */
        if((Cy_BLE_GetState() == CY_BLE_STATE_ON) && (Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED) &&
           (Cy_BLE_GetNumOfActiveConn() == 0u) && (authState != AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT) &&
           (authState != AUTHENTICATION_BONDING_REMOVE_GO_AHEAD))
        {
            if(advState == ADVERTISEMENT_PUBLIC)
            {
                AdvertisePublicly();
            }
            else    /* ADVERTISEMENT_PRIVATE */
            {
                cy_stc_ble_peer_id_addr_t peerBdAddr;

                /* Display Bond list and optionaly Resolving list */
                DisplayBondList();
                
                peerBdAddr.type = bondedDeviceList.bdHandleAddrList[0u].bdAddr.type;
                memcpy(peerBdAddr.bdAddr, bondedDeviceList.bdHandleAddrList[0u].bdAddr.bdAddr, CY_BLE_BD_ADDR_SIZE);
                
                AdvertisePrivately(&peerBdAddr);
            }
        }
        
        /* To achieve low power in the device */
        LowPowerImplementation();
    
    #if(CY_BLE_BONDING_REQUIREMENT == CY_BLE_BONDING_YES)
        /* Store bonding data to flash only when all debug information has been sent */    
        if(cy_ble_pendingFlashWrite != 0u) 
        {   
            apiResult = Cy_BLE_StoreBondingData();    
            DBG_PRINTF("Store bonding data, status: %x, pending: %x \r\n", apiResult, cy_ble_pendingFlashWrite);
        }
    #endif /* CY_BLE_BONDING_REQUIREMENT == CY_BLE_BONDING_YES */
        
        /*******************************************************************
        *  Periodically calculate transmit baud rate
        *******************************************************************/        
        if((((customConfiguration.mode & CY_BLE_CUSTOM_MODE_LIMITED) == 0u) && (mainTimer >= CY_BLE_TIMER_TIMEOUT)) ||
           (((customConfiguration.mode & CY_BLE_CUSTOM_MODE_LIMITED) != 0u) && (dataSendConfirmed != 0u)))
        {
            uint64 cachedTotalByteCounter = totalByteCounter;
            uint32_t throughput;
            cy_stc_ble_gatt_xchg_mtu_param_t mtuSize = 
            {
                .connHandle = appConnHandle
            };
            Cy_BLE_GATT_GetMtuSize(&mtuSize);
            
            totalByteCounter = 0;
            if((customConfiguration.mode & CY_BLE_CUSTOM_MODE_LIMITED) == 0u)
            {
                /* Total bytes transmitted in 10 seconds converted to kilobits per second */
                throughput = (cachedTotalByteCounter >> 7u) / 10u;
                MCWDT_ResetCounters(CY_MCWDT_CTR0, 100u);
                mainTimer = 0u;                
            }
            else
            {
                uint32_t time;
                time = MCWDT_GetCount(CY_MCWDT_COUNTER2); /* Timer counts of 32768Hz crystal */
                
                dataSendConfirmed = 0u;
                
                /* Total transmitted bytes converted to kilobits per second */
                throughput = cachedTotalByteCounter * 8u * 1000000u / 32768 / time;
            }
            DBG_PRINTF("Throughput is: %ld kbps, packet length = %d\r\n", throughput, mtuSize.mtu - dataSendReduceLength);
            (void)throughput; /* Suppress a warning for disabled UART mode */
            limitedStarted = 0u;
            /* Change packet length */
            dataSendReduceLength += PACKET_LENGTH_REDUCE;
            
            if(dataSendReduceLength > mtuSize.mtu)
            {
                dataSendReduceLength = 0u;
            }

        }
        /***********************************************************************
        * Handle SW2 press. 
        ***********************************************************************/
        if((SW2_Read() == 0u) && (button != 0u))
        {
            if(SW2_Read() == 0u) 
            {
                /* Change private advertising address */
                if((Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING) && 
                   (advState == ADVERTISEMENT_PRIVATE))
                {
                    /* Stop advertisement */
                     Cy_BLE_GAPP_StopAdvertisement();
                }
                
                if(Cy_BLE_GetConnectionState(appConnHandle) == CY_BLE_CONN_STATE_CONNECTED)
                {
                    /* Accept displayed passkey */
                    if(authState == AUTHENTICATION_PASSKEY_YES_NO)
                    {
                        customCommand = 'y';
                    }
                    /* Restart data transmission in one shot mode */
                    if((dataSendEnabled == 0u) && (dataSendCccd != 0u) && (limitedStarted == 0u))
                    {
                        dataSendConfirmed = 0;
                        mainTimer = 0u;                
                        DBG_PRINTF("Start data transmitting\r\n");
                        MCWDT_ResetCounters(CY_MCWDT_CTR0, 100u);
                        MCWDT_ResetCounters(CY_MCWDT_CTR2, 100u);
                        dataSendEnabled = dataSendCccd;
                        limitedStarted = 1u;
                    }
                }
            }
        }
        button = SW2_Read();

        if((authState != AUTHENTICATION_PASSKEY_YES_NO) && (authState != AUTHENTICATION_WAITING_FOR_PASSKEY))
        {
            /* See if the user pressed 'r' button to remove the bond. */
            command = UART_DEB_GET_CHAR();
            
            if(command != UART_DEB_NO_DATA)
            {
                switch(command)
                {
                    case 'r':                 
                        /* User wants the bond to be removed */
                        DBG_PRINTF("Remove the bond. \r\n");
                        if(Cy_BLE_GetConnectionState(appConnHandle) == CY_BLE_CONN_STATE_CONNECTED)
                        {
                            cy_stc_ble_gap_disconnect_info_t param =
                            {
                                .bdHandle = appConnHandle.bdHandle,
                                .reason = CY_BLE_HCI_ERROR_OTHER_END_TERMINATED_USER                           
                            };
                                /* Disconnect */
                            authState = AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT;
                            Cy_BLE_GAP_Disconnect(&param);
                        }
                        else if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
                        {
                            /* Stop advertisement */
                            authState = AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT;
                            Cy_BLE_GAPP_StopAdvertisement();
                        }
                        else
                        {
                            authState = AUTHENTICATION_BONDING_REMOVE_GO_AHEAD;
                        }
                        break;
                    case 's': 
                        DisplayBondList();
                        break;
                }
                disableLowPowerModeFlag = 0u;
                DBG_PRINTF("INFO: Low Power Mode is ENABLED \r\n");
            }
        }
        
        switch(authState)
        {
            case AUTHENTICATION_PASSKEY_YES_NO:
                if(((command = UART_DEB_GET_CHAR()) != UART_DEB_NO_DATA) || (customCommand != 0u))
                {
                    if((command == UART_DEB_NO_DATA) && (customCommand != 0u))
                    {
                        command = customCommand;
                        customCommand = 0u;
                    }
                    if(command == 'y')
                    {
                        DBG_PRINTF("Accept displayed passkey \r\n");
                        authPkParam.bdHandle = appConnHandle.bdHandle;
                        authPkParam.passkey = 0u;
                        authPkParam.accept = CY_BLE_GAP_ACCEPT_PASSKEY_REQ;
                        apiResult = Cy_BLE_GAP_AuthPassKeyReply(&authPkParam);
                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("Cy_BLE_GAP_AuthPassKeyReply API Error: 0x%x \r\n", apiResult);
                        }
                        authState = AUTHENTICATION_PASSKEY_ENTERED;

                    }
                    else if(command == 'n')
                    {
                        DBG_PRINTF("Reject displayed passkey \r\n");
                        authPkParam.bdHandle = appConnHandle.bdHandle;
                        authPkParam.passkey = 0u;
                        authPkParam.accept = CY_BLE_GAP_REJECT_PASSKEY_REQ;
                        apiResult = Cy_BLE_GAP_AuthPassKeyReply(&authPkParam);
                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("Cy_BLE_GAP_AuthPassKeyReply API Error: 0x%x \r\n", apiResult);
                        }
                       authState = AUTHENTICATION_PASSKEY_ENTERED;
                    }
                    else
                    {
                        DBG_PRINTF("Wrong key entered. Press 'y' or 'n'\r\n");
                    }
                }
            break;
                
            case AUTHENTICATION_WAITING_FOR_PASSKEY:
                /* Get 6 digit number from UART terminal */
                if((command = UART_DEB_GET_CHAR()) != UART_DEB_NO_DATA)
                {
                    /* Accept the digits that are in between the range '0' and '9' */
                    if((command >= '0') && (command <= '9'))  
                    {
                        passkeyValue += (uint32)(command - '0') * passkeyPow;
                        passkeyPow /= 10u;
                        passkeyCounter++;
                        UART_DEB_PUT_CHAR(command); 
                    #if(CY_BLE_SECURE_CONN_FEATURE_ENABLED)
                        /* Send Key press notification */
                        if(((peerAuthInfo.security & CY_BLE_GAP_SEC_LEVEL_MASK) == CY_BLE_GAP_SEC_LEVEL_4) &&
                           ((peerAuthInfo.pairingProperties & CY_BLE_GAP_SMP_SC_PAIR_PROP_KP_MASK) != 0u))
                        {
                            cy_stc_ble_gap_sc_kp_notif_info_t pfInfo =
                            {
                                .bdHandle = appConnHandle.bdHandle,
                                .notificationType = CY_BLE_GAP_PASSKEY_DIGIT_ENTERED
                            };
                            apiResult = Cy_BLE_GAP_AuthSendKeyPress(&pfInfo);
                            if(apiResult != CY_BLE_SUCCESS)
                            {
                                DBG_PRINTF("Cy_BLE_GAP_AuthSendKeyPress API Error: 0x%x \r\n", apiResult);
                            }
                        }
                    #endif /* CY_BLE_SECURE_CONN_FEATURE_ENABLED */
                    }
                    else  /* If entered digit is not in between the range '0' and '9' */
                    {
                        DBG_PRINTF("Wrong key entered. \r\n");
                    }

                    if(passkeyCounter == CY_BLE_GAP_USER_PASSKEY_SIZE)
                    {
                        /* Send Pass key Response to create an Authenticated Link */
                        if(((peerAuthInfo.security & CY_BLE_GAP_SEC_LEVEL_MASK) == CY_BLE_GAP_SEC_LEVEL_4) &&
                           ((peerAuthInfo.pairingProperties & CY_BLE_GAP_SMP_SC_PAIR_PROP_KP_MASK) != 0u))
                        {
                            cy_stc_ble_gap_sc_kp_notif_info_t scKpNotifParam =
                            {
                                .bdHandle = appConnHandle.bdHandle,
                                .notificationType = CY_BLE_GAP_PASSKEY_ENTRY_COMPLETED
                            };
                            apiResult = Cy_BLE_GAP_AuthSendKeyPress(&scKpNotifParam);
                            if(apiResult != CY_BLE_SUCCESS)
                            {
                                DBG_PRINTF("Cy_BLE_GAP_AuthSendKeyPress API Error: 0x%x \r\n", apiResult);
                            }
                        }
                        authPkParam.bdHandle = appConnHandle.bdHandle;
                        authPkParam.passkey = passkeyValue;
                        authPkParam.accept = CY_BLE_GAP_ACCEPT_PASSKEY_REQ;
                        apiResult = Cy_BLE_GAP_AuthPassKeyReply(&authPkParam);
                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("Cy_BLE_GAP_AuthPassKeyReply API Error: 0x%x \r\n", apiResult);
                        }
                        authState = AUTHENTICATION_PASSKEY_ENTERED;
                    }
                }
                break;
        
                
            case AUTHENTICATION_COMPLETE_BONDING_REQD:
                
                authState = AUTHENTICATION_READ_CENTRAL_ADDRESS_RESOLUTION;
                DBG_PRINTF("Bonding complete. Press 'r' at any time to remove this bond. \r\n");
                
                /* Ready for private advertisement the next time */
                advState = ADVERTISEMENT_PRIVATE;
                break;
                
                
            case AUTHENTICATION_BONDING_COMPLETE:
                
                /* Send data via characteristic notifications */
                if((dataSendEnabled != 0u) && (Cy_BLE_GetConnectionState(appConnHandle) == CY_BLE_CONN_STATE_CONNECTED))
                {
                    /* Send new data only when the previous data has gone out, 
                     * which is indicated by Stack being Free.
                     */
                    if(Cy_BLE_GATT_GetBusyStatus(appConnHandle.attId) == CY_BLE_STACK_STATE_FREE)
                    {
                        CustomSendData();
                    }
                }
                break;

                
            case AUTHENTICATION_BONDING_REMOVE_GO_AHEAD:
            #if(CY_BLE_BONDING_REQUIREMENT == CY_BLE_BONDING_YES)
                {
                    cy_stc_ble_gap_bd_addr_t clearAllDevices = {{0u,0u,0u,0u,0u,0u},0u};
                    
                    /* Remove all bonded devices in the list */
                    apiResult = Cy_BLE_GAP_RemoveBondedDevice(&clearAllDevices);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_GAP_RemoveBondedDevice API Error: 0x%x \r\n", apiResult);
                    }
                    else
                    {
                        #if(CY_BLE_LL_PRIVACY_FEATURE_ENABLED)
                        {
                            cy_stc_ble_peer_id_addr_t emptyAddr = {.type = 0u};
                            
                            /* Remove all bonded devices in the resolving list */
                            apiResult = Cy_BLE_RemoveDeviceFromResolvingList(&emptyAddr);
                            if(apiResult != CY_BLE_SUCCESS)
                            {
                                DBG_PRINTF("Cy_BLE_RemoveDeviceFromResolvingList API Error: 0x%x \r\n", apiResult);
                            }
                            else
                            {
                                /* Start advertising with a public address */
                                advState = ADVERTISEMENT_PUBLIC;
                                authState = AUTHENTICATION_NOT_CONNECTED;
                            }
                        }
                        #endif  /* CY_BLE_LL_PRIVACY_FEATURE_ENABLED */
                    }

                    /* Process pending BLE events */
                    Cy_BLE_ProcessEvents();

                    DBG_PRINTF("Clear the list of bonded devices. \r\n");
                    
                }
            #else
                /* Start advertising with a public address */
                advState = ADVERTISEMENT_PUBLIC;
                authState = AUTHENTICATION_NOT_CONNECTED;
            #endif  /* CY_BLE_BONDING_REQUIREMENT == CY_BLE_BONDING_YES */

                break;
                
            case AUTHENTICATION_READ_CENTRAL_ADDRESS_RESOLUTION:
            {
                cy_stc_ble_gattc_read_by_type_req_t readByTypeReqParam =
                {
                    .range.startHandle = CY_BLE_GATT_ATTR_HANDLE_START_RANGE,
                    .range.endHandle = CY_BLE_GATT_ATTR_HANDLE_END_RANGE,
                    .uuid.uuid16 = CY_BLE_UUID_CHAR_CENTRAL_ADDRESS_RESOLUTION,
                    .uuidFormat = CY_BLE_GATT_16_BIT_UUID_FORMAT,
                    .connHandle = appConnHandle
                };
                centralAddressResolutionValue = 0u;
                apiResult = Cy_BLE_GATTC_ReadUsingCharacteristicUuid(&readByTypeReqParam);
                if(apiResult == CY_BLE_SUCCESS)
                {
                    authState = AUTHENTICATION_READING_CAR;
                }
                else
                {
                    DBG_PRINTF("Cy_BLE_GATTC_ReadUsingCharacteristicUuid API Error: 0x%x \r\n", apiResult);
                    authState = AUTHENTICATION_UPDATE_CONN_PARAM;
                }
            }
                break;
            
            case AUTHENTICATION_UPDATE_CONN_PARAM:
            {
                /* Read Peripheral Preferred Connection Parameters Characteristic value from database */ 
                uint8_t prefConnParamCharValue[CY_BLE_PPCPC_LEN];
                cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo = 
                {
                    .handleValuePair.attrHandle = cy_ble_configPtr->gaps->prefConnParamCharHandle,
                    .handleValuePair.value.len  = sizeof(prefConnParamCharValue),
                    .handleValuePair.value.val  = prefConnParamCharValue,
                    .flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED
                };
                
                if((Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE) &&
                   (connIntv > Cy_BLE_Get16ByPtr(prefConnParamCharValue + CY_BLE_PPCPC_MIN_CONN_INTV_OFFSET)))
                {
                    /* Send Connection Parameter Update Request to Client */
                    cy_stc_ble_gap_conn_update_param_info_t connUpdateParam =
                    {
                        .connIntvMin = Cy_BLE_Get16ByPtr(prefConnParamCharValue + CY_BLE_PPCPC_MIN_CONN_INTV_OFFSET),
                        .connIntvMax = Cy_BLE_Get16ByPtr(prefConnParamCharValue + CY_BLE_PPCPC_MAX_CONN_INTV_OFFSET),
                        .connLatency = Cy_BLE_Get16ByPtr(prefConnParamCharValue + CY_BLE_PPCPC_SLAVE_LATENCY_OFFSET),
                        .supervisionTO = Cy_BLE_Get16ByPtr(prefConnParamCharValue + CY_BLE_PPCPC_SUP_TIMEOUT_OFFSET),
                        .bdHandle = appConnHandle.bdHandle
                    };
                    
                    apiResult = Cy_BLE_L2CAP_LeConnectionParamUpdateRequest(&connUpdateParam);
                    if(apiResult == CY_BLE_SUCCESS)
                    {
                        authState = AUTHENTICATION_BONDING_COMPLETE;
                    }
                    else
                    {
                        DBG_PRINTF("Cy_BLE_L2CAP_LeConnectionParamUpdateRequest API Error: 0x%x \r\n", apiResult);
                    }
                }   
                else
                {
                    authState = AUTHENTICATION_BONDING_COMPLETE;
                }
            }
                break;
            default:    /* Not supported state */
                break;
        }
        
    }
}


/* [] END OF FILE */

