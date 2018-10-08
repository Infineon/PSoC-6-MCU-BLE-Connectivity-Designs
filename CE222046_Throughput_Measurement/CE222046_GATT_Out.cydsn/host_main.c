/******************************************************************************
* File Name: host_main.c
*
* Version: 1.10
*
* Description: This is the source code for BLE GATT Server with custom 
*              throughput service. The BLE sends notification data to the BLE
*              GATT client device which is used by the client for BLE 
*              throughput measurement. 
*
* Related Document: CE222046_Throughput_Measurement.pdf
*
* Hardware Dependency: See CE222046_Throughput_Measurement.pdf
*
*******************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress’s integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress 
* reserves the right to make changes to the Software without notice. Cypress 
* does not assume any liability arising out of the application or use of the 
* Software or any product or circuit described in the Software. Cypress does 
* not authorize its products for use in any products where a malfunction or 
* failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such 
* system or application assumes all risk of such use and in doing so agrees to 
* indemnify Cypress against all liability.
*******************************************************************************/

#include "project.h"
#include "stdio_user.h"
#include <stdio.h>


#if CY_BLE_HOST_CORE   
/******************************************************************************
* Macros
*******************************************************************************/
#define ENABLE      (1u)
#define DISABLE     (0u)

#define DEBUG_BLE_ENABLE    DISABLE

#if DEBUG_BLE_ENABLE
#define DEBUG_BLE       printf
#else
#define DEBUG_BLE(...)
#endif

#define MAX_MTU_SIZE                (512)
#define DEFAULT_MTU_SIZE            (23) 
#define NOTIFICATION_PKT_SIZE       (495)
#define SUCCESS                     (0u)
#define TARGET_BDADDR       {{0xFF, 0xBB, 0xAA, 0x50, 0xA0, 0x00}, 0}
#define CUSTOM_SERV0_CHAR0_DESC0_HANDLE     cy_ble_customConfig.customs[0]\
                                        .customServInfo[0].customServCharDesc[0] 
#define CUSTOM_SERV0_CHAR0_HANDLE           cy_ble_customConfig.customs[0]\
                                        .customServInfo[0].customServCharHandle 

/*******************************************************************************
* Variables
*******************************************************************************/
bool charNotificationEnabled = false;
uint8 buffer[NOTIFICATION_PKT_SIZE];
uint16 negotiatedMtu = DEFAULT_MTU_SIZE;
cy_stc_ble_gap_bd_addr_t    local_addr = TARGET_BDADDR;
cy_stc_ble_conn_handle_t appConnHandle;
cy_stc_ble_gatts_handle_value_ntf_t notificationPacket;

/*******************************************************************************
*        Function Prototypes
*******************************************************************************/
void Ble_Init(void);
void StackEventHandler(uint32 event, void* eventParam);
void SendNotification(void);

/*******************************************************************************
* Function Name: HostMain()
********************************************************************************
* Summary:
*  Main function for the BLE Host.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Theory:
*  The function starts BLE and UART components.
*  This function processes all BLE events
*
*******************************************************************************/
int HostMain(void)
{
    __enable_irq(); /* Enable global interrupts. */
    
    UART_Start();
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    
    printf("*****************CE222046: PSoC 6 MCU BLE Throughput Measurement *******"\
               "**********\r\n");
    printf("Role : Server (GATT OUT)\r\n");
    
    printf("*****************************************************************"\
               "*****************\r\n\n");
    /* Initialize BLE */
    Ble_Init();
    
    /* Variable 'buffer' holds dummy notification data to be send for 
       BLE throughput measurement */
    for(uint16_t i=0; i < NOTIFICATION_PKT_SIZE; i++)
    {
        buffer[i]=i; 
    }
    for(;;)
    {
        /* Cy_Ble_ProcessEvents() allows BLE stack to process pending events */
        Cy_BLE_ProcessEvents();
        
        if(charNotificationEnabled == true)
        {
            if(Cy_BLE_GATT_GetBusyStatus(appConnHandle.attId) ==\
               CY_BLE_STACK_STATE_FREE)
            {
                /* Send notification data to the GATT Client*/
                SendNotification();
            }
        }
    }
}

/*******************************************************************************
* Function Name: Ble_Init()
********************************************************************************
*
* Summary:
*   This function initializes BLE.
*
* Return:
*   None
*
*******************************************************************************/
void Ble_Init(void)
{
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_stack_lib_version_t stackVersion;
    
    /* Start Host of BLE Component and register generic event handler */
    apiResult = Cy_BLE_Start(StackEventHandler);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        DEBUG_BLE("Cy_BLE_Start API Error: %x \r\n", apiResult);
    }
    else
    {
        DEBUG_BLE("BLE Stack Initialized...\r\n");
    }
    
    apiResult = Cy_BLE_GetStackLibraryVersion(&stackVersion);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        DEBUG_BLE("Cy_BLE_GetStackLibraryVersion API Error: 0x%2.2x \r\n",\
                  apiResult);
    }
    else
    {
        DEBUG_BLE("Stack Version: %d.%d.%d.%d \r\n", stackVersion.majorVersion, 
        stackVersion.minorVersion, stackVersion.patch, stackVersion.buildNumber);
    }
    
}

/*******************************************************************************
* Function Name: StackEventHandler()
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE Component.
*
*  event - the event code
*  *eventParam - the event parameters
*
* Return:
*   None
*
*******************************************************************************/
void StackEventHandler(uint32 event, void* eventParam)
{
    cy_en_ble_api_result_t apiResult;
    
    switch(event)
    {
         /* There are some events generated by the BLE component
        *  that are not required for this code example. */
        
        /**********************************************************
        *                       General Events
        ***********************************************************/
        /* This event is received when the BLE component is Started */
        case CY_BLE_EVT_STACK_ON:
        {
            DEBUG_BLE("CY_BLE_EVT_STACK_ON, Start Advertisement \r\n");    
            
            apiResult = Cy_BLE_GAP_SetBdAddress((cy_stc_ble_gap_bd_addr_t  *)
                                               &local_addr);
            
            if(apiResult != CY_BLE_SUCCESS)
            {   
                DEBUG_BLE("Cy_BLE_GAP_SetBdAddress API Error: %d \r\n",
                    apiResult);
            }
                        
             DEBUG_BLE("CUSTOM_SERV0_CHAR0_DESC0_HANDLE (%d)"
                "\r\nCUSTOM_SERV0_CHAR0_HANDLE(%d)\r\n",
                CUSTOM_SERV0_CHAR0_DESC0_HANDLE, CUSTOM_SERV0_CHAR0_HANDLE);
            
            /* Enter into discoverable mode so that remote device can search it */
            Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST,
                                   CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            
            break;
        }    
        /* This event is received when there is a timeout. */
        case CY_BLE_EVT_TIMEOUT:
        {
			DEBUG_BLE("CY_BLE_EVT_TIMEOUT \r\n"); 
            
            break;
		}   
        /* This event indicates that some internal HW error has occurred. */    
		case CY_BLE_EVT_HARDWARE_ERROR:    
        {
			DEBUG_BLE("Hardware Error \r\n");
            
			break;
		}   
        /*  This event will be triggered by host stack if BLE stack is busy or 
         *  not busy. Parameter corresponding to this event will be the state 
    	 *  of BLE stack.
         *  BLE stack busy = CY_BLE_STACK_STATE_BUSY,
    	 *  BLE stack not busy = CY_BLE_STACK_STATE_FREE 
         */
    	case CY_BLE_EVT_STACK_BUSY_STATUS:
        {
			DEBUG_BLE("CY_BLE_EVT_STACK_BUSY_STATUS: %x\r\n", *(uint8 *)eventParam);
            
			break;
		}
        /* This event indicates completion of Set LE event mask. */
        case CY_BLE_EVT_LE_SET_EVENT_MASK_COMPLETE:
        {
			DEBUG_BLE("CY_BLE_EVT_LE_SET_EVENT_MASK_COMPLETE \r\n");
            
            break;
		}            
        /* This event indicates set device address command completed. */
        case CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE:
        {
            DEBUG_BLE("CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE \r\n");
            
            cy_stc_ble_events_param_generic_t *param = \
			(cy_stc_ble_events_param_generic_t *) eventParam;
        
            if( param->status != SUCCESS)
            {
                DEBUG_BLE("Failed to Set Local BDAddress [Status 0x%02X]\r\n",\
                    param->status);
               
            }
            else
            {
                DEBUG_BLE("Local Address Set successfully \r\n");
                DEBUG_BLE("BdAddress set to: %02X:%02X:%02X:%02X:%02X:%02X \r\n",\
                    local_addr.bdAddr[5],local_addr.bdAddr[4], local_addr.bdAddr[3],\
                    local_addr.bdAddr[2], local_addr.bdAddr[1], local_addr.bdAddr[0]);  
                          
                Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST,\
				CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            }
            
            break;
        }
            
        /* This event indicates get device address command completed
           successfully */
        case CY_BLE_EVT_GET_DEVICE_ADDR_COMPLETE:
        {
			DEBUG_BLE("CY_BLE_EVT_GET_DEVICE_ADDR_COMPLETE: ");
            DEBUG_BLE("\n\rAdvertising with Address: ");
            for(uint8_t i = CY_BLE_GAP_BD_ADDR_SIZE; i > 0u; i--)
            {
                DEBUG_BLE("%2.2X ", ((cy_stc_ble_bd_addrs_t *)\
				((cy_stc_ble_events_param_generic_t *)eventParam)->eventParams)\
				  ->publicBdAddr[i-1]);
            }
            DEBUG_BLE("\r\n");
            
            break;
		}
        /* This event indicates set Tx Power command completed. */
        case CY_BLE_EVT_SET_TX_PWR_COMPLETE:
        {
			DEBUG_BLE("CY_BLE_EVT_SET_TX_PWR_COMPLETE \r\n");
            
            break;
		}                       
            
        /**********************************************************
        *                       GAP Events
        ***********************************************************/
       
        /* This event indicates peripheral device has started/stopped
           advertising. */
        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
        {
            DEBUG_BLE("CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP: ");
            if(Cy_BLE_GetConnectionState(appConnHandle) == CY_BLE_CONN_STATE_DISCONNECTED)
            {
                DEBUG_BLE(" <Restart ADV> \r\n");
                printf("Advertising...\r\n\n");
                Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST,\
                               				   CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
                charNotificationEnabled = false;
                
                /* Start the PWM Component to blink the LED_Advertisement*/
                PWM_Start();
            }
           
            break;
        }
            
        /* This event is triggered instead of 'CY_BLE_EVT_GAP_DEVICE_CONNECTED', 
        if Link Layer Privacy is enabled in component customizer. */
        case CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE:
        {
            /* BLE link is established */
            /* This event will be triggered since link layer privacy is enabled */
            DEBUG_BLE("CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE \r\n");
            
            cy_stc_ble_gap_enhance_conn_complete_param_t *param = \
            (cy_stc_ble_gap_enhance_conn_complete_param_t *)eventParam; 
            
            printf("Connected to Device ");

            printf("%02X:%02X:%02X:%02X:%02X:%02X\r\n\n",param->peerBdAddr[5],\
                    param->peerBdAddr[4], param->peerBdAddr[3], param->peerBdAddr[2],\
                    param->peerBdAddr[1], param->peerBdAddr[0]);
         
            PWM_Disable();
            Cy_GPIO_Write(LED_ConnectStatus_0_PORT, LED_ConnectStatus_0_NUM, 0);
            DEBUG_BLE("\r\nBDhandle : 0x%02X\r\n", param->bdHandle);
                
            break;
        }
        
        /* This event is triggered when there is a change to either the maximum Payload 
        length or the maximum transmission time of Data Channel PDUs in either direction */
        case CY_BLE_EVT_DATA_LENGTH_CHANGE:
        {
            DEBUG_BLE("CY_BLE_EVT_DATA_LENGTH_CHANGE \r\n");
            cy_stc_ble_set_phy_info_t phyParam;
            
            /* Configure the BLE Component for 2Mbps data rate */
            phyParam.bdHandle = appConnHandle.bdHandle;
            phyParam.allPhyMask = CY_BLE_PHY_NO_PREF_MASK_NONE;
            phyParam.phyOption = 0;
            phyParam.rxPhyMask = CY_BLE_PHY_MASK_LE_2M;
            phyParam.txPhyMask = CY_BLE_PHY_MASK_LE_2M;
            
            Cy_BLE_EnablePhyUpdateFeature();
            apiResult = Cy_BLE_SetPhy(&phyParam);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DEBUG_BLE("Failed to set PHY..[bdHandle 0x%02X] : 0x%4x\r\n",
                    phyParam.bdHandle, apiResult);
            }
            else
            {
                DEBUG_BLE("Setting PHY.[bdHandle 0x%02X] \r\n", phyParam.bdHandle);
            }
            
            break;
        }
        
        /* This event is generated at the GAP Peripheral end after connection 
           is completed with peer Central device. */
        case CY_BLE_EVT_GAP_DEVICE_CONNECTED: 
		{
			DEBUG_BLE("CY_BLE_EVT_GAP_DEVICE_CONNECTED \r\n");
            charNotificationEnabled = false;
                      
            break;
		}            
        /* This event is generated when disconnected from remote device or 
           failed to establish connection. */
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:   
		{
			if(Cy_BLE_GetConnectionState(appConnHandle) == CY_BLE_CONN_STATE_DISCONNECTED)
            {
                DEBUG_BLE("CY_BLE_EVT_GAP_DEVICE_DISCONNECTED %d\r\n",\
                    CY_BLE_CONN_STATE_DISCONNECTED);
            }
            
            /* Device disconnected; restart advertisement */
            negotiatedMtu = DEFAULT_MTU_SIZE;
            Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST,\
                CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            
            break;
		}            
        /* This event is generated at the GAP Central and the peripheral end 
           after connection parameter update is requested from the host to 
           the controller. */
        case CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
		{
			DEBUG_BLE("CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE \r\n");
            
            break;
		}
        /* This event indicates completion of the Cy_BLE_SetPhy API*/
		case CY_BLE_EVT_SET_PHY_COMPLETE:
        {
            DEBUG_BLE("Updating the Phy.....\r\n");
            cy_stc_ble_events_param_generic_t * param =\
            (cy_stc_ble_events_param_generic_t *) eventParam;
            if(param->status ==SUCCESS)
            {
                DEBUG_BLE("SET PHY updated to 2 Mbps\r\n");
                Cy_BLE_GetPhy(appConnHandle.bdHandle);
            }
            else
            {
                DEBUG_BLE("SET PHY Could not update to 2 Mbps\r\n");
                Cy_BLE_GetPhy(appConnHandle.bdHandle);
            }
            
            break;
        }
        /* This event indicates completion of the Cy_BLE_GetPhy API */
        case CY_BLE_EVT_GET_PHY_COMPLETE:
        {
            /* To remove unused parameter warning when UART debug is disabled */
            #if (DEBUG_BLE == ENABLE)
            cy_stc_ble_events_param_generic_t *param =\
            cy_stc_ble_events_param_generic_t *)eventParam;
            cy_stc_ble_phy_param_t *phyparam = NULL;
                      
            if(param->status == SUCCESS)
            {
                phyparam = (cy_stc_ble_phy_param_t *)param->eventParams;
                DEBUG_BLE("RxPhy Mask : 0x%02X\r\nTxPhy Mask : 0x%02X\r\n", \
                    phyparam->rxPhyMask, phyparam->txPhyMask);            
            }
            #endif
            
            break;

        }
        /* This event indicates that the controller has changed the transmitter
           PHY or receiver PHY in use */
        case CY_BLE_EVT_PHY_UPDATE_COMPLETE:
        {
            DEBUG_BLE("UPDATE PHY parameters\r\n");
             /* To remove unused parameter warning when UART debug is disabled */
            #if (DEBUG_BLE == ENABLE)
            cy_stc_ble_events_param_generic_t *param =\
           (cy_stc_ble_events_param_generic_t *)eventParam;
            cy_stc_ble_phy_param_t *phyparam = NULL;
            if(param->status == SUCCESS)
            {
                phyparam = (cy_stc_ble_phy_param_t *)param->eventParams;
                DEBUG_BLE("RxPhy Mask : 0x%02X\r\nTxPhy Mask : 0x%02X\r\n",\
                    phyparam->rxPhyMask, phyparam->txPhyMask);            
            }
            #endif
            
            break;
        }		
            
        /**********************************************************
        *                       GATT Events
        ***********************************************************/
            
        /* This event is generated at the GAP Peripheral end after connection 
           is completed with peer Central device. */
        case CY_BLE_EVT_GATT_CONNECT_IND:
        {
            appConnHandle = *(cy_stc_ble_conn_handle_t *)eventParam;
            DEBUG_BLE("CY_BLE_EVT_GATT_CONNECT_IND: %x, %x \r\n", 
                        appConnHandle.attId, 
                        appConnHandle.bdHandle);
            
            /* Update Notification packet with the data. */
            notificationPacket.connHandle = appConnHandle;
            notificationPacket.handleValPair.attrHandle = CUSTOM_SERV0_CHAR0_HANDLE;
            notificationPacket.handleValPair.value.val = buffer;
            notificationPacket.handleValPair.value.len = NOTIFICATION_PKT_SIZE;
            
            Cy_BLE_GetPhy(appConnHandle.bdHandle);                   
           
            break;
        }    
        /* This event is generated at the GAP Peripheral end after disconnection. */
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
        {
            DEBUG_BLE("CY_BLE_EVT_GATT_DISCONNECT_IND \r\n");
            if(appConnHandle.bdHandle == (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle)
            {
                printf("Disconnected. \r\n\n");
                appConnHandle.bdHandle = CY_BLE_INVALID_CONN_HANDLE_VALUE;
                appConnHandle.attId    = CY_BLE_INVALID_CONN_HANDLE_VALUE;
            }
            Cy_GPIO_Write(LED_ConnectStatus_0_PORT, LED_ConnectStatus_0_NUM, 1);
            
            break;
        }
        /* This event is triggered when 'GATT MTU Exchange Request' 
           received from GATT client device. */
        case CY_BLE_EVT_GATTS_XCNHG_MTU_REQ:
        {
            negotiatedMtu = (((cy_stc_ble_gatt_xchg_mtu_param_t *)eventParam)->mtu < CY_BLE_GATT_MTU) ?
                            ((cy_stc_ble_gatt_xchg_mtu_param_t *)eventParam)->mtu : CY_BLE_GATT_MTU;
            DEBUG_BLE("CY_BLE_EVT_GATTS_XCNHG_MTU_REQ negotiated = %d\r\n", negotiatedMtu);
            
            break;
        }   
        /* This event is triggered when there is a write request from the 
           Client device */
        case CY_BLE_EVT_GATTS_WRITE_REQ:
        {
            DEBUG_BLE("CY_BLE_EVT_GATTS_WRITE_REQ \r\n");  
            cy_stc_ble_gatt_write_param_t *write_req_param = \
            (cy_stc_ble_gatt_write_param_t *)eventParam;
            cy_stc_ble_gatts_db_attr_val_info_t attr_param;
                        
            DEBUG_BLE("Received GATT Write Request [bdHandle %02X]\r\n",\
                write_req_param->connHandle.bdHandle);
            attr_param.connHandle.bdHandle = write_req_param->connHandle.bdHandle;
            attr_param.connHandle.attId = write_req_param->connHandle.attId;
            attr_param.flags = CY_BLE_GATT_DB_PEER_INITIATED;
            attr_param.handleValuePair = write_req_param->handleValPair;
            attr_param.offset = 0;
            
            DEBUG_BLE("write_req_param->handleValPair.attrHandle = %d",\
                write_req_param->handleValPair.attrHandle);
            DEBUG_BLE(" -> Should be = CUSTOM_SERV0_CHAR0_DESC0_HANDLE (%d)"\
                "\r\n-> Should be = CUSTOM_SERV0_CHAR0_HANDLE(%d)\r\n",\
                CUSTOM_SERV0_CHAR0_DESC0_HANDLE, CUSTOM_SERV0_CHAR0_HANDLE);
                  
           
			if(write_req_param->handleValPair.attrHandle == (CUSTOM_SERV0_CHAR0_DESC0_HANDLE))
            {
                if(Cy_BLE_GATTS_WriteRsp(write_req_param->connHandle) != CY_BLE_SUCCESS)
                {
                    DEBUG_BLE("Failed to send write response \r\n");
                }
                else
                {
                    DEBUG_BLE("GATT write response sent \r\n");
                    charNotificationEnabled = attr_param.handleValuePair.value.val[0];
                    DEBUG_BLE("charNotificationEnabled = %d\r\n", charNotificationEnabled);
                    printf("Notification Enabled.\r\n\n");
                    notificationPacket.connHandle = appConnHandle;
                    notificationPacket.handleValPair.attrHandle = CUSTOM_SERV0_CHAR0_HANDLE;
                    notificationPacket.handleValPair.value.val = buffer;
                    notificationPacket.handleValPair.value.len = NOTIFICATION_PKT_SIZE;
                }
            }
            break;
        }
        case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
        {
            DEBUG_BLE("CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ \r\n");  
            break;
        }
        /**********************************************************
        *                       Other Events
        ***********************************************************/
        default:
            DEBUG_BLE("OTHER event: %lx \r\n", (unsigned long) event);
			break;
        
    }
}

/*******************************************************************************
* Function Name: SendNotification()
********************************************************************************
* Summary:
* Sends notification data to the GATT Client
*
* Parameters:
* None
*
* Return:
* None
*
*******************************************************************************/
void SendNotification(void)
{
    cy_en_ble_api_result_t apiResult;
    apiResult = Cy_BLE_GATTS_Notification(&notificationPacket);
    if(apiResult == CY_BLE_ERROR_INVALID_PARAMETER)
    {
        DEBUG_BLE("Couldn't send notification. [CY_BLE_ERROR_INVALID_PARAMETER]\r\n");
    }
    else if(apiResult != CY_BLE_SUCCESS)
    {
        DEBUG_BLE("Attrhandle = 0x%4X  Cy_BLE_GATTS_Notification API Error:"\
            "0x%2.2x \r\n", notificationPacket.handleValPair.attrHandle, apiResult);
    }
    
}

#endif /* CY_BLE_HOST_CORE */

/* [] END OF FILE */
