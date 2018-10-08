/*******************************************************************************
* File Name: host_main.c
*
* Version: 1.10
*
* Description: This is the source code for BLE GATT Client. The client receives
*              the notification data sent by the GATT server. The BLE throughput 
*              is calculated every 10 seconds and displayed on a UART terminal
*              emulator.
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

#include <project.h>
#include "stdio_user.h"
#include <stdio.h>

#if CY_BLE_HOST_CORE

/*******************************************************************************
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


#define SUCCESS                     (  0u)
#define MAX_MTU_SIZE                (512u)
#define DEFAULT_MTU_SIZE            ( 23u)
    
#define GATT_MTU                    (MAX_MTU_SIZE)
              

    
/* The configurable GAP parameters */
#define TARGET_BDADDR   {{0xFF, 0xBB, 0xAA, 0x50, 0xA0, 0x00}, 0}
    
/* The configurable GATT parameters */
#define GATT_CCD_HANDLE             (28u) 
#define GATT_CHAR_HANDLE            (27u) 
#define CY_BLE_CCCD_LEN             (0x02u) 

/*******************************************************************************
* Variables
*******************************************************************************/
cy_en_ble_api_result_t              apiResult = CY_BLE_SUCCESS;
cy_stc_ble_gap_bd_addr_t            local_addr;
cy_stc_ble_gap_bd_addr_t            remote_addr = TARGET_BDADDR;

/* GAP Related Information */
bool    isConnected = false;
uint8   bdHandle;
bool    targetAddrFound = false;

/* GATT Related Information */
/* Char handle for which notification will be received. */
const uint16  char_handle = GATT_CHAR_HANDLE; 

/* CCD handle to enable notification. */
const uint16  desc_handle = GATT_CCD_HANDLE; 

/* CCD value to enable notification*/
uint8   desc_val[CY_BLE_CCCD_LEN] = {0x01, 0x00}; 

cy_stc_ble_conn_handle_t     conn_handle;

/* Number of notification received */
uint32 notify_count; 

/*******************************************************************************
*        Function Prototypes
*******************************************************************************/
void Ble_Init(void);
void StartScan(void);
void InitiateConnection(void);
void EnableNotification(void);
void StackEventHandler(uint32 event, void* eventParam);
void BleAssert(void);
void Disconnect(cy_stc_ble_conn_handle_t connHandle);
void MyTimerIsr(void);
void StartScan(void);

/*******************************************************************************
* Function Name: HostMain()
********************************************************************************
* Summary:
*  Main function for the BLE host.
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
    printf("Role : Client (GATT IN)\r\n");
    /* Configure the Timer to generate an interrupt every 10 seconds */
    Timer_Start();
    Cy_SysInt_Init(&TimerInterrupt_cfg, MyTimerIsr);
    NVIC_EnableIRQ(TimerInterrupt_cfg.intrSrc);
    NVIC_ClearPendingIRQ(TimerInterrupt_cfg.intrSrc);
    
    /* Initialize BLE */
    Ble_Init();
    
    for(;;) 
    {   
        /* Cy_BLE_ProcessEvents() allows BLE stack to process pending events */
        Cy_BLE_ProcessEvents();
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
    cy_en_ble_api_result_t          apiResult;
    cy_stc_ble_stack_lib_version_t  stackVersion;    
    
    apiResult = Cy_BLE_GetStackLibraryVersion(&stackVersion);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        DEBUG_BLE("Cy_BLE_GetStackLibraryVersion API Error: 0x%2.2x \r\n",\
                   apiResult);
    }
    else
    {
        DEBUG_BLE("Stack Library Version: Major Version-%u, Minor Version-%u,"\
        "Patch No-%u, Build No-%u\r\n", stackVersion.majorVersion,\
        stackVersion.minorVersion, stackVersion.patch, stackVersion.buildNumber);
    }
    
    printf("*****************************************************************"\
               "*****************\r\n\n");
    
     /* Start BLE component and register generic event handler */
    apiResult = Cy_BLE_Start(StackEventHandler);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        /* BLE stack initialization failed, check configuration 
           and halt CPU in debug mode */
        DEBUG_BLE("Cy_BLE_Start API Error: %x \r\n", apiResult);
        BleAssert();    
    }
    else
    {
        DEBUG_BLE("BLE Stack Initialized..\r\n");
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
void StackEventHandler(uint32 event, void *eventParam)
{
    cy_en_ble_api_result_t      apiResult;

    uint8 bdAddrIndex;
    
    cy_stc_ble_gapc_adv_report_param_t * advReport;
    
    uint8_t scanState = Cy_BLE_GetScanState();
    switch (event)
	{
        /* There are other events generated by the BLE component
        *  that are not required for this code example. */
        
        /**********************************************************
        *                       General Events
        ***********************************************************/
		/* This event is received when the BLE component is Started */
        case CY_BLE_EVT_STACK_ON:
        {
            DEBUG_BLE("CY_BLE_EVT_STACK_ON \r\n");    
            
            /* Stack initialized; ready for scan */
            
            /* Start scanning for the target device.
             * Triggers events: CY_BLE_EVT_GAPC_SCAN_START_STOP */
            StartScan();
            
            break;
        }
        
        /* This event indicates get device address command completed
           successfully */
        case CY_BLE_EVT_GET_DEVICE_ADDR_COMPLETE:
        {
            DEBUG_BLE("CY_BLE_EVT_GET_DEVICE_ADDR_COMPLETE\n\r");
            
            break;
        }
        
        /* This event indicates set device address command completed. */
        case CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE:
        {
            DEBUG_BLE("CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE \r\n");
            
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
            DEBUG_BLE("CY_BLE_EVT_HARDWARE_ERROR \r\n");
            
			break;
        }
        
        /*  This event will be triggered by host stack if BLE stack is busy
            or has become free */
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
        
        /* This event indicates set Tx Power command completed. */
        case CY_BLE_EVT_SET_TX_PWR_COMPLETE:
        {
            DEBUG_BLE("CY_BLE_EVT_SET_TX_PWR_COMPLETE \r\n");
            
            break;
        }

            
        /**********************************************************
        *                     GAP Central Events
        ***********************************************************/
        /* This event is triggered every time a device is discovered */   
        case CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
        {
            DEBUG_BLE("CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT\r\n");
            
            /* A new device listed in the scan report */
            advReport = (cy_stc_ble_gapc_adv_report_param_t *)eventParam;
            
            /* Process only for Advertisement packets, not on scan response packets */
            if(advReport->eventType != CY_BLE_GAPC_SCAN_RSP)
            {
                /* Set address 'found' status temporarily */
                targetAddrFound = true;
                
                for(bdAddrIndex = 0; bdAddrIndex < CY_BLE_BD_ADDR_SIZE; bdAddrIndex++)
                {
                    /* Set targetAddrFound status to false if BD address bytes don't match */
                    if (advReport->peerBdAddr[bdAddrIndex] != remote_addr.bdAddr[bdAddrIndex]) 
                    {
                        targetAddrFound = false;
                    }
                }

                if (targetAddrFound)
                {
                    printf("Found target device with address: ");
                    printf("%02X:%02X:%02X:%02X:%02X:%02X\r\n\n",advReport->peerBdAddr[5],\
                    advReport->peerBdAddr[4], advReport->peerBdAddr[3], advReport->peerBdAddr[2],\
                    advReport->peerBdAddr[1], advReport->peerBdAddr[0]);
                    Cy_BLE_GAPC_StopScan();
                }
            }
            
            break;
        }
        
        /* This event is triggered when the central device has started/stopped scanning */
        case CY_BLE_EVT_GAPC_SCAN_START_STOP:
        {
            DEBUG_BLE("CY_BLE_EVT_GAPC_SCAN_START_STOP, scanstate = ");
            switch(scanState)
            {
                case CY_BLE_SCAN_STATE_STOPPED:
                    DEBUG_BLE("CY_BLE_SCAN_STATE_STOPPED\r\n");
                    break;
                
                case CY_BLE_SCAN_STATE_SCAN_INITIATED:
                    DEBUG_BLE("CY_BLE_SCAN_STATE_SCAN_INITIATED\r\n");
                    break;
                    
                case CY_BLE_SCAN_STATE_SCANNING:
                    DEBUG_BLE("CY_BLE_SCAN_STATE_SCANNING\r\n");
                    break;
                    
                case CY_BLE_SCAN_STATE_STOP_INITIATED:
                    DEBUG_BLE("CY_BLE_SCAN_STATE_STOP_INITIATED\r\n");
                    break;
            }
            
            if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_STOPPED)
            {
                if(targetAddrFound == true)
                {
                    /* Scan stopped manually; do not restart scan */
                    targetAddrFound = false;
                    printf("Scan stopped as device was found. Initiating Connection...\r\n\n");
                    InitiateConnection();
                }
                else
                {
                    /* Scanning timed out; Restart scan */
                    printf("Timed Out. Restarting scan...\r\n\n");
                    StartScan();
                }
            }
            break;
        }    
        /* This event is triggered when there is a change to either the maximum Payload 
        length or the maximum transmission time of Data Channel PDUs in either direction */
        case CY_BLE_EVT_DATA_LENGTH_CHANGE:
        {
            DEBUG_BLE("CY_BLE_EVT_DATA_LENGTH_CHANGE\r\n");
            
            cy_stc_ble_set_phy_info_t phyParam;

            phyParam.bdHandle = conn_handle.bdHandle;
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
        /* This event indicates completion of the Cy_BLE_SetPhy API*/
        case CY_BLE_EVT_SET_PHY_COMPLETE:
        {
            DEBUG_BLE("Updating the Phy.....\r\n");
            cy_stc_ble_events_param_generic_t * param = (cy_stc_ble_events_param_generic_t *) eventParam;
            if(param->status ==SUCCESS)
            {
                DEBUG_BLE("SET PHY updated to 2 Mbps\r\n");
                Cy_BLE_GetPhy(conn_handle.bdHandle);
            }
            else
            {
                DEBUG_BLE("SET PHY Could not update to 2 Mbps\r\n");
                Cy_BLE_GetPhy(conn_handle.bdHandle);
            }
            
            break;
        }
        /* This event indicates completion of the Cy_BLE_GetPhy API*/
        case CY_BLE_EVT_GET_PHY_COMPLETE:
        {
             /* To remove unused parameter warning when UART debug is disabled*/
            #if (DEBUG_BLE == ENABLE)
            DEBUG_BLE("GET PHY parameters\r\n");
            cy_stc_ble_events_param_generic_t *param =(cy_stc_ble_events_param_generic_t *)eventParam;
            cy_stc_ble_phy_param_t *phyparam = NULL;
            if(param->status == SUCCESS)
            {
                phyparam = (cy_stc_ble_phy_param_t *)param->eventParams;
                DEBUG_BLE("RxPhy Mask : 0x%02X\r\nTxPhy Mask : 0x%02X\r\n", phyparam->rxPhyMask, phyparam->txPhyMask);            
            }
            #endif
            break;

        }
        /* This event indicates that the controller has changed the transmitter
           PHY or receiver PHY in use */
        case CY_BLE_EVT_PHY_UPDATE_COMPLETE:
        {
            /* To remove unused parameter warning when UART debug is disabled*/
            #if (DEBUG_BLE == ENABLE)
            DEBUG_BLE("UPDATE PHY parameters\r\n");
            cy_stc_ble_events_param_generic_t *param =(cy_stc_ble_events_param_generic_t *)eventParam;
            cy_stc_ble_phy_param_t *phyparam = NULL;
            if(param->status == SUCCESS)
            {
                phyparam = (cy_stc_ble_phy_param_t *)param->eventParams;
                DEBUG_BLE("RxPhy Mask : 0x%02X\r\nTxPhy Mask : 0x%02X\r\n", phyparam->rxPhyMask, phyparam->txPhyMask);            
            }
            #endif
            break;
        }
        
        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        /* This event is generated at the GAP Peripheral end after a connection is completed
           with a peer Central device. For a GAP Central device, this event is generated as
           in acknowledgment of receiving this event successfully by the BLE Controller.*/    
        case CY_BLE_EVT_GATT_CONNECT_IND:
        {
            /* Connected as Central (master role) */  
            conn_handle = *(cy_stc_ble_conn_handle_t *)eventParam;
            
            DEBUG_BLE("CY_BLE_EVT_GATT_CONNECT_IND: attId=%X, bdHandle=%X \r\n", 
                        conn_handle.attId, 
                        conn_handle.bdHandle);
            printf("Connected to Device\r\n\n");
            
            PWM_Disable();
            Cy_GPIO_Write(LED_ConnectStatus_0_PORT, LED_ConnectStatus_0_NUM, 0); 
                  
            /* Initiate an MTU exchange request */
            cy_stc_ble_gatt_xchg_mtu_param_t mtuParam = {conn_handle, 512};
            apiResult = Cy_BLE_GATTC_ExchangeMtuReq(&mtuParam);
            
            if(apiResult != CY_BLE_SUCCESS)
            {
                DEBUG_BLE("Cy_BLE_GATTC_ExchangeMtuReq API Error: %xd \r\n", apiResult);
            }
                
            break;
        }
        /* This event indicates GATT MTU Exchange response from the server device. */
        case CY_BLE_EVT_GATTC_XCHNG_MTU_RSP:
        {
            /* To remove unused parameter warning when UART debug is disabled */
            #if (DEBUG_BLE == ENABLE)
            cy_stc_ble_gatt_xchg_mtu_param_t *resp = (cy_stc_ble_gatt_xchg_mtu_param_t *)eventParam;
            DEBUG_BLE("CY_BLE_EVT_GATTC_XCHNG_MTU_RSP [bdHandle 0x%02X MTU %hu]\r\n", 
                   resp->connHandle.bdHandle, resp->mtu);
            #endif
            
            /* Enable notifications on the characteristic to get data from the Server. */
            EnableNotification();

            break;
        }
        
        /* This event is generated at the GAP Peripheral end after 
           disconnection. */
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
        {
            DEBUG_BLE("CY_BLE_EVT_GATT_DISCONNECT_IND \r\n");
            if(conn_handle.bdHandle == (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle)
            {
                printf("Disconnected.\r\n\n");
                conn_handle.bdHandle = CY_BLE_INVALID_CONN_HANDLE_VALUE;
                conn_handle.attId    = CY_BLE_INVALID_CONN_HANDLE_VALUE;
            }
            Cy_GPIO_Write(LED_ConnectStatus_0_PORT, LED_ConnectStatus_0_NUM, 1); 
            StartScan();
            break;
        }
         
        /* This event is triggered when 'GATT MTU Exchange Request' 
           received from GATT client device. */
        case CY_BLE_EVT_GATTS_XCNHG_MTU_REQ:
        {
            DEBUG_BLE("CY_BLE_EVT_GATTS_XCNHG_MTU_REQ \r\n");
            break;
        }
        
        /* This event indicates a Write response from the server device. */
        case CY_BLE_EVT_GATTC_WRITE_RSP:
        {
            /* Write response for the CCCD write; this means that the
             * notifications are now enabled. */
            DEBUG_BLE("CY_BLE_EVT_GATTC_WRITE_RSP, bdHandle = %X\r\n",\
                     (*(cy_stc_ble_conn_handle_t *) eventParam).bdHandle);
            DEBUG_BLE("Notifications are enabled\r\n");
            break;
        }
        /* This event is triggered when notification data is received from the 
           server device. */    
        case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
        {
            cy_stc_ble_gattc_handle_value_ntf_param_t *param \
             = (cy_stc_ble_gattc_handle_value_ntf_param_t *)eventParam;
            notify_count += param->handleValPair.value.len;
            break;
        }
        /* The event is received by the Client when the Server cannot perform 
           the requested operation and sends out an error response. */
        case CY_BLE_EVT_GATTC_ERROR_RSP:
        {
            DEBUG_BLE("CY_BLE_EVT_GATTC_ERROR_RSP \r\n");
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
* Function Name: StartScan()
********************************************************************************
* Summary:
* Starts a scan on the Central device.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The function starts BLE scan and resets scan related variables.
* As soon as the discovery operation starts, CY_BLE_EVT_GAPC_SCAN_START_STOP
* event is generated. The CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT event is
* generated when a GAP peripheral device is located.
*
* Side Effects:
* None
*
*******************************************************************************/
void StartScan(void)
{
    printf("Scanning for GAP Peripheral with address: %02X:%02X:%02X:%02X:%02X:%02X\r\n\n",
            remote_addr.bdAddr[5], remote_addr.bdAddr[4], remote_addr.bdAddr[3],
            remote_addr.bdAddr[2], remote_addr.bdAddr[1], remote_addr.bdAddr[0]);
    Cy_BLE_GAPC_StartScan(CY_BLE_SCANNING_FAST,\
                          CY_BLE_CENTRAL_CONFIGURATION_0_INDEX);
    PWM_Start();
}

/*******************************************************************************
* Function Name: InitiateConnection()
********************************************************************************
* Summary:
* Initiates a connection with a GAP peripheral.
*
* Parameters:
* None
*
* Return:
* None
*
*******************************************************************************/
void InitiateConnection(void)
{
    
    /* Initiate Connection */
    apiResult = Cy_BLE_GAPC_ConnectDevice(&remote_addr, \
                                         CY_BLE_CENTRAL_CONFIGURATION_0_INDEX);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        DEBUG_BLE("Failed to initiate connection \r\n");
        BleAssert();
    }
    else
    {
        DEBUG_BLE("Initiated GAP connection..\r\n");
    }
}

/*******************************************************************************
* Function Name: EnableNotification()
********************************************************************************
* Summary:
* Enables notification on the GATT Server
*
* Parameters:
* None
*
* Return:
* None
*
*******************************************************************************/
void EnableNotification(void)
{
    cy_stc_ble_gattc_write_req_t req;
                
    desc_val[0] = 1;
    
    req.connHandle = conn_handle;
    req.handleValPair.attrHandle = desc_handle;
    req.handleValPair.value.val = desc_val;
    req.handleValPair.value.len = CY_BLE_CCCD_LEN;
    apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&req);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DEBUG_BLE("Failed to enable notification [bdhandle 0x%02X]: \r\n",\
            req.connHandle.bdHandle);
        BleAssert();
    }
    else
    {
        DEBUG_BLE("Enabling notification [bdhandle 0x%02X]\r\n",\
                   req.connHandle.bdHandle);
    }
    
}

/*******************************************************************************
* Function Name: BleAssert()
********************************************************************************
* Summary:
*
* Parameters:
* None
*
* Return:
* None
*
*******************************************************************************/
void BleAssert(void)
{
    CY_ASSERT(0u != 0u);   
}

/*******************************************************************************
* Function Name: MyTimerIsr()
********************************************************************************
* Summary:
* Interrupt service routine for the timer block.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The ISR is fired when the timer reaches terminal count. In the ISR, the total
* number of bytes received until now is cached locally (to avoid any potential 
* race conditions) and then the throughput is calculated from it.
* Since the ISR fires 10 seconds after the timer is started, the total number
* of bytes is divided by 10. The bytes are converted to kilobytes by dividing 
* by 1024. It is then converted to kilo bits by multiplying the result by 8.
*
* Side Effects:
* None
*
*******************************************************************************/
void MyTimerIsr(void)
{
    uint32_t throughput;

    throughput = (notify_count>>7) /10 ;
    if (Cy_BLE_GetConnectionState(conn_handle)== CY_BLE_CONN_STATE_CONNECTED)
    {
        printf("Throughput is: %lu kbps. \r\n", (unsigned long)throughput);
    }
    notify_count = 0;
    Timer_ClearInterrupt(CY_TCPWM_INT_ON_TC);
    NVIC_ClearPendingIRQ(TimerInterrupt_cfg.intrSrc);
}

#endif /* CY_BLE_HOST_CORE */

/* [] END OF FILE */
