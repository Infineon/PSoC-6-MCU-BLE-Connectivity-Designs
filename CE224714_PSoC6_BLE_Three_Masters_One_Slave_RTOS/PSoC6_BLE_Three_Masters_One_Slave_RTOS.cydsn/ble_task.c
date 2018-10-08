/******************************************************************************
* File Name: ble_task.c
*
* Version: 1.0
*
* Description: This file contains the task that initializes BLE Host and 
*  handles different BLE events.
*
* Related Document: CE224714_PSoC6_BLE_Three_Masters_One_Slave_RTOS.pdf
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
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

/* Include header */
#include "ble_app_config.h"
#include "ble_task.h"
#include "status_led_task.h"
#include "uart_task.h"
#include "uart_debug.h"

/* Queue Handle for BLE command */
QueueHandle_t bleCommandQ;

#define MAX_BUFFER_LENGTH   (64u)   /* Buffer size for display messages */
#define BAS_SERVICE_INDEX   (0u)    /* BAS service index */

/* Variable for storing connection information */
static app_stc_connection_info_t    appConnInfo;
/* Variable for storing scanned device information */
static app_stc_scan_device_list_t   appScanDevInfo;
/* Flag used for tracking device with BAS notification enabled */ 
static uint8_t basNotificationFlag = 0u;

/** 
 * These static functions are used by the BLE Task. These are not available 
 * outside this file. See the respective function definitions for more 
 * details. 
 */
static void BleControllerInterruptEventHandler(void);
static void StackEventHandler(uint32_t eventType, void *eventParam);
static void BasCallback(uint32_t event, void *eventParam);

static void Ble_AddDevice(cy_stc_ble_gap_connected_param_t *connParam);
static void Ble_ConnectDevice(uint32_t devId);
static void Ble_DisconnectDevice(uint32_t devId);
static void Ble_RemoveDevice(uint8_t bdHandle);
static void Ble_DisplayConnectedDevice(void);
static void Ble_DisplayScannedDevice(void);
static bool Ble_AddDeviceInScanList(uint8_t* peerBdAddr,\
    uint8_t peerAddrType);
static bool Ble_IsDeviceInScanList(uint8_t* peerBdAddr);
static bool Ble_IsServiceUuidPresent(\
    cy_stc_ble_gapc_adv_report_param_t *eventParam, uint16_t uuid);

static void UpdateBatteryLevel(uint8_t bdHandle, uint8_t* value);

/* These functions are used for peripheral */
static void Ble_Peripheral_StartAdvertisement(void);
static void Ble_Peripheral_StopAdvertisement(void);
static void Ble_Peripheral_SendNotification(void);
static void Ble_Peripheral_AddDevice(uint8_t bdHandle);

/* These functions are used for central */
static void Ble_Central_StartScan(void);
static void Ble_Central_StopScan(void);
static void Ble_Central_AddDevice(uint8_t bdHandle);

/* Display the menu */ 
static void PrintMenu(void);

/*******************************************************************************
* Function Name: void Task_Ble(void *pvParameters)
********************************************************************************
* Summary:
*  Task that processes the BLE state and events, and then commands other tasks 
*  to take an action based on the current BLE state and data received over 
*  bleComaandQ queue.
*
* Parameters:
*  void *pvParameters : Task parameter defined during task creation (unused)                            
*
* Return:
*  None
*
*******************************************************************************/
void Task_Ble(void *pvParameters)
{
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    /* Variable used to store the return values of BLE APIs */
    cy_en_ble_api_result_t bleApiResult;
    
    /* Variable that stores BLE commands that need to be processed */
    ble_command_t bleCommand;
    
    bool connectPending = false;
    bool disconnectPending = false;
    
    /* Remove warning for unused parameter */
    (void)pvParameters;
    /* Display the menu */
    PrintMenu();
    /* Start the BLE component and register the stack event handler */
    bleApiResult = Cy_BLE_Start(StackEventHandler);
    
    /* Check if the operation was successful */
    if(bleApiResult == CY_BLE_SUCCESS)
    {
        Task_DebugPrintf("Success  : BLE - Stack initialization", 0u);
    
        /* Register the application Host callback */
        Cy_BLE_RegisterAppHostCallback(BleControllerInterruptEventHandler);
        
        /* Register service specific callback function */
        Cy_BLE_BAS_RegisterAttrCallback(BasCallback);
        
        /* Process BLE events to turn the stack on */
        Cy_BLE_ProcessEvents();
    }
    else
    {
        Task_DebugPrintf("Failure! : BLE  - Stack initialization. Error Code:",
                         bleApiResult);
    }
    
    /* Repeatedly running part of the task */
    for(;;)
    {
        /* Block until a BLE command has been received over bleCommandQ */
        rtosApiResult = xQueueReceive(bleCommandQ, &bleCommand, portMAX_DELAY);
        
        /* Command has been received from bleCommandQ */
        if(rtosApiResult == pdTRUE)
        {
            switch(bleCommand.command)
            {
                /*~~~~~~~~~~~~~~ Command to process BLE events ~~~~~~~~~~~~~~*/
                case PROCESS_BLE_EVENTS:
                {
                    /**
                     * Process event callback to handle BLE events. The events
                     * and associated actions taken by this application are 
                     * inside the 'StackEventHandler' routine. 
                     * Note that Cortex M4 only handles the BLE host portion of
                     * the stack, while Cortex M0+ handles the BLE controller 
                     * portion 
                     */
                    Cy_BLE_ProcessEvents();
                    break;
                }
                /*~~~~~~~~~~~~~~ Command to start scan ~~~~~~~~~~~~~~~~~~~~~~*/
                case START_SCAN:
                {
                    Ble_Central_StartScan();
                    break;
                }
                /*~~~~~~~~~~~~~~ Command to stop scan ~~~~~~~~~~~~~~~~~~~~~~~*/
                case STOP_SCAN:
                {
                    Ble_Central_StopScan();
                    break;
                }
                /*~~~~~~~~~~~~~~ Command to start advertisement ~~~~~~~~~~~~~*/
                case START_ADVERTISEMENT:
                {
                    Ble_Peripheral_StartAdvertisement();
                    break;
                }
                /*~~~~~~~~~~~~~~ Command to stop advertisement ~~~~~~~~~~~~~~*/
                case STOP_ADVERTISEMENT:
                {
                    Ble_Peripheral_StopAdvertisement();             
                    break;
                }
                /*~~~~~~~~~~~~~~ Command to connect to peer device ~~~~~~~~~~*/
                case CONNECT:
                {
                    uint32_t userInput;
                    /* Check if connection is pending */
                    if(connectPending)
                    {
                        connectPending = false;
                        userInput = bleCommand.data - '0';
                        /* Check the user input */
                        if((userInput > 0) && (userInput <= appScanDevInfo.count))
                        {
                            /* Connect to peer device */
                            Ble_ConnectDevice(userInput);
                        }
                        else
                        {
                            Task_Printf("\rError    : Invalid input ", 0u);
                        }
                    }
                    else
                    {
                        if(appScanDevInfo.count > 0)
                        {
                            connectPending = true;
                            Ble_DisplayScannedDevice();
                        }
                        else
                        {
                            Task_Printf("\rInfo     : BLE - No device present "\
                                "with Battery Alert Service", 0u);
                        }
                    }
                    break;
                }
                
                /*~~~~~~~~~~~~~~ Command to disconnect to peer device ~~~~~~~*/
                case DISCONNECT:
                {
                    uint32_t userInput;
                    uint32_t connDeviceCount =  Cy_BLE_GetNumOfActiveConn();
                    /* Check if disconnect is pending */
                    if(disconnectPending)
                    {
                        disconnectPending = false;
                        userInput = bleCommand.data - '0';
                        /* Check the user input */
                        if(((userInput > 0) && (userInput <= appConnInfo.centralCnt)) || \
                        ((userInput > BLE_MAX_CENTRAL_CONN_COUNT) &&\
                        (userInput <= BLE_MAX_CONN_COUNT)))
                        {
                            Ble_DisconnectDevice((uint8_t)userInput);
                        }
                        else
                        {
                            Task_Printf("\rError   : Invalid input \r\n", 0u);
                        }
                    }
                    else
                    {
                        if(connDeviceCount > 0)
                        {
                            disconnectPending = true;
                            Task_Printf("Select device for disconnect...", 0u);
                            /* Print a list of connected devices */
                            Ble_DisplayConnectedDevice();
                        }
                        else
                        {
                            Task_Printf("Info     : No device connected", 0u);
                        }
                    }
                    break;
                }
                
                /*~~~~~~~~~~ Command to display all the connected devices ~~~~*/
                case DISPLAY_CONNECTED_DEVICE:
                {
                    Task_DebugPrintf("Info     : Display connected device", 0);
                    Ble_DisplayConnectedDevice();
                    break;
                }
                
                /*~~~~~~~~~~~ Command to scan notification ~~~~~~~~~~~~~~~~~~*/
                case SEND_NOTIFICATION:
                {
                    Ble_Peripheral_SendNotification();
                    break;
                }
                
                /*~~~~~~~~~~~~~~~ Invalid BLE command ~~~~~~~~~~~~~~~~~~~~~~~*/
                default:
                {
                    Task_DebugPrintf("Warning! : BLE - Invalid command", 0u);
                    break;
                }
            }
        }
        /**
         * Task has timed out and received no commands during an interval of 
         *  portMAXDELAY ticks 
         */
        else
        {
            Task_DebugPrintf("Warning! : BLE - Task Timed out ", 0u);
        }
    }   
}

/*******************************************************************************
* Function Name: static void StackEventHandler(uint32_t event, void *eventParam)
********************************************************************************
* Summary:
*  Call back event function to handle various events from the BLE stack. Note 
*  that Cortex M4 only handles the BLE host portion of the stack, while 
*  Cortex M0+ handles the BLE controller portion. 
*
* Parameters:
*  event        :	BLE event occurred
*  eventParam   :	Pointer to the value of event specific parameters
*
* Return:
*  None
*
*******************************************************************************/
static void StackEventHandler(uint32_t eventType, void *eventParam)
{
    /* Variable used to store the return values of BLE APIs */
    cy_en_ble_api_result_t      bleApiResult;
    
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    /* Take an action based on the current event */
    switch (eventType)
    {
        /* This event is indicates BLE stack is on */  
        case CY_BLE_EVT_STACK_ON:
        {
            Task_DebugPrintf("Info     : BLE - Stack on", 0u);
            break;
        }

        /* This event is generated when there is a timeout */
        case CY_BLE_EVT_TIMEOUT:
        {
            cy_stc_ble_timeout_param_t* timeoutParam = \
                (cy_stc_ble_timeout_param_t*) eventParam;
            switch(timeoutParam->reasonCode)
            {
                case CY_BLE_GAP_ADV_TO:
                {
                    Task_DebugPrintf("Info     : BLE - Advertisement timeout", 0u);
                    break;
                }
                case CY_BLE_GAP_SCAN_TO:
                {
                    Task_DebugPrintf("Info     : BLE - Scan timeout", 0u);
                    break;
                }
                case CY_BLE_GATT_RSP_TO:
                {
                    Task_DebugPrintf("Info     : BLE - GATT response timeout", 0u);
                    break;
                }
                case CY_BLE_GENERIC_APP_TO:
                {
                    Task_DebugPrintf("Info     : BLE - generic application timeout", 0u);
                    break;
                }
            }
            Task_DebugPrintf("Info     : BLE - Timeout", 0u);
            break;
        }
        
        /* This event is indicates stack shutdown complete */
        case CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE:
        {
            Task_DebugPrintf("Info     : BLE - Stack shutdown complete", 0u);
            break;
        }
        
        /*~~~~~~~~~~~~~~~~~~~~~~ GAP Central ~~~~~~~~~~~~~~~~~~~~~~~~~*/
        
        /* GAP device connected */
        case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
        {
            Ble_AddDevice((cy_stc_ble_gap_connected_param_t *)eventParam);
            break;
        }
        
        /* Gap device connected */
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
        {
            Task_DebugPrintf("Info     : BLE - GAP device disconnected. bdHandle: ",
                ((cy_stc_ble_gap_disconnect_param_t*)eventParam)->bdHandle);
            break;
        }
        
        /* Indicates that the Central device has started/stopped scanning */
        case CY_BLE_EVT_GAPC_SCAN_START_STOP:
        {
            status_led_data_t statusLedData = {.orangeLed = LED_NO_CHANGE};
           
            if (Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_SCANNING)
            {
                statusLedData.redLed = LED_TOGGLE_EN;
                Task_DebugPrintf("Info     : BLE - Scanning started", 0u);
            }
            else
            {
                statusLedData.redLed = LED_TURN_OFF;
                Task_DebugPrintf("Info     : BLE - Scan complete!", 0u);
            }
            
            rtosApiResult = xQueueSend(statusLedDataQ, &statusLedData, 0u);
            /* Check if the operation has been successful */
            if(rtosApiResult != pdTRUE)
            {
                Task_DebugPrintf("Failure! : BLE - Sending data to Status LED"\
                                 "queue", 0u);   
            }
            break;
        }
        
        /* Indicates that the Peripheral device has started/stopped advertisement */
        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
        {
            status_led_data_t statusLedData = {.redLed = LED_NO_CHANGE};
            if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
            {
                statusLedData.orangeLed = LED_TOGGLE_EN;
                Task_DebugPrintf("Info     : BLE - Advertisement started ", 0u);
            }
            else
            {
                statusLedData.orangeLed = LED_TURN_OFF;
                Task_DebugPrintf("Info     : BLE - Advertisement stopped ", 0u);
            }            
            rtosApiResult = xQueueSend(statusLedDataQ, &statusLedData, 0u);
            /* Check if the operation has been successful */
            if(rtosApiResult != pdTRUE)
            {
                Task_DebugPrintf("Failure! : BLE - Sending data to Status LED"\
                                 "queue", 0u);   
            }
            break;
        }
        
        /* This event is triggered every time a device is discovered */
        case CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
        {
            /* Variable used to store scanned device information */
            static char printBuffer[BLE_MAX_SCAN_DEVICES][MAX_BUFFER_LENGTH];
            static uint8_t devCount = 0;
            
            cy_stc_ble_gapc_adv_report_param_t advReport = \
                *(cy_stc_ble_gapc_adv_report_param_t*)eventParam;
            
            if((Cy_BLE_IsPeerConnected(advReport.peerBdAddr) == false) && \
               (advReport.eventType == CY_BLE_GAPC_CONN_UNDIRECTED_ADV) && \
               (Ble_IsServiceUuidPresent(&advReport, CY_BLE_UUID_BAS_SERVICE) == true) &&\
               (appScanDevInfo.count < BLE_MAX_SCAN_DEVICES))
            {
                /* Display device address */
                if(Ble_AddDeviceInScanList(advReport.peerBdAddr, advReport.peerAddrType))
                {
                    sprintf(printBuffer[devCount], "\rInfo     : Device with BAS found" \
                    "- %2.2X%2.2X%2.2X%2.2X%2.2X%2.2X", \
                    advReport.peerBdAddr[5], advReport.peerBdAddr[4], \
                    advReport.peerBdAddr[3], advReport.peerBdAddr[2], \
                    advReport.peerBdAddr[1], advReport.peerBdAddr[0]);
                    
                    Task_Printf(printBuffer[devCount], 0u);
                    devCount = devCount % BLE_MAX_SCAN_DEVICES;
                }
            }
            break;
        }
        
        /* GAP authentication request received */
        case CY_BLE_EVT_GAP_AUTH_REQ:
        {
            if(cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].security == 
                (CY_BLE_GAP_SEC_MODE_1 | CY_BLE_GAP_SEC_LEVEL_1))
            {
                cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].authErr = 
                    CY_BLE_GAP_AUTH_ERROR_PAIRING_NOT_SUPPORTED;
            }    
            cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].bdHandle = 
                ((cy_stc_ble_gap_auth_info_t *)eventParam)->bdHandle;
                
            bleApiResult = Cy_BLE_GAPP_AuthReqReply(\
                &cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);            
            if(bleApiResult != CY_BLE_SUCCESS)
            {
                Cy_BLE_GAP_RemoveOldestDeviceFromBondedList();
                bleApiResult = Cy_BLE_GAPP_AuthReqReply(\
                    &cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);            
                if(bleApiResult != CY_BLE_SUCCESS)
                {
                    Task_DebugPrintf("Failure!  : BLE - Send pairing response", \
                    	bleApiResult);
                }
            }
            break;
        }
        
        /* GAP authentication failed */
        case CY_BLE_EVT_GAP_AUTH_FAILED:
        {
            Task_DebugPrintf("Info     : BLE - GAP authentication failed", 
                (*(cy_stc_ble_gap_auth_info_t *)eventParam).authErr);
            break;
        } 
        
        /* SMP has completed pairing feature exchange */
        case CY_BLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO:
        {
            Task_DebugPrintf("Info     : BLE - SMP feature exchange complete", 0u);
            break;
        }
        
        /* Indicates that the GAP authentication process complete */
        case CY_BLE_EVT_GAP_AUTH_COMPLETE:
        {
            Task_DebugPrintf("Info     : BLE - GAP authentication complete", 0u);
            
            cy_stc_ble_conn_handle_t connHandle =  Cy_BLE_GetConnHandleByBdHandle( \
                ((cy_stc_ble_gap_auth_info_t*)eventParam)->bdHandle);
            
            if(Cy_BLE_GetDeviceRole(&connHandle) == CY_BLE_GAP_LL_ROLE_MASTER )
            {
                bleApiResult = Cy_BLE_GATTC_StartDiscovery(connHandle);
                if(bleApiResult != CY_BLE_SUCCESS)
                {
                    Task_DebugPrintf("Failure!  : BLE - Start Discovery API", 0u);
                }
            }
            else
            {
                Task_Printf("Info     : BLE - Connection complete", 0u);
            }
            break;
        }
        
        /* Indicates that the encryption change event for an active connection */
        case CY_BLE_EVT_GAP_ENCRYPT_CHANGE:
        {
            Task_DebugPrintf("Info     : BLE - encryption change event for" \
                "an active connection", 0u);
            break;
        }
        
        /*~~~~~~~~~~~~~~~~~~~~~ GATT Events ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        
        /* The discovery of a remote device completed successfully */
        case CY_BLE_EVT_GATTC_DISCOVERY_COMPLETE:
        {
            cy_stc_ble_conn_handle_t connHandle = 
            	*(cy_stc_ble_conn_handle_t *)eventParam;
            
            uint16_t cccd_attValue = CY_BLE_CCCD_NOTIFICATION;
            
            Task_DebugPrintf("Info     : BLE - Discovery complete", 0u);
            
            /* Enable notification for the connected BAS Client */
            Task_DebugPrintf("Info     : BLE - Enable notifications for BAS service", 0u);
            
            bleApiResult = Cy_BLE_BASC_SetCharacteristicDescriptor(connHandle, \
                            BAS_SERVICE_INDEX, CY_BLE_BAS_BATTERY_LEVEL, \
                            CY_BLE_BAS_BATTERY_LEVEL_CCCD, sizeof(cccd_attValue), \
                            (uint8 *)&cccd_attValue);
            
            if(bleApiResult != CY_BLE_SUCCESS)
            {
                
                Task_DebugPrintf("Failure!  : BLE - BASC set characteristic descriptor API ", \
                    bleApiResult);
            }
            
            Task_Printf("Info     : BLE - Connection complete", 0u);
            break;
        }
        
        /* Read response received from GATT Server device*/
        case CY_BLE_EVT_GATTC_READ_RSP:
        {
            Task_DebugPrintf("Info     : BLE - Read response received from GATT Server"
                "device, bdHandle ", ((cy_stc_ble_conn_handle_t *)eventParam)->bdHandle);
            break;
        }
        
        /* Read response received from GATT Server device*/
        case CY_BLE_EVT_GATTC_WRITE_RSP:
        {
            Task_DebugPrintf("Info     : BLE - Write response received from GATT Server"
                "device, bdHandle ", ((cy_stc_ble_conn_handle_t *)eventParam)->bdHandle);
            break;
        }
        
        /* */
        case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
        {
            Task_DebugPrintf("Info     : BLE - GATT read request", 0u);
            break;
        }
        
        /* GATT device connected */
        case CY_BLE_EVT_GATT_CONNECT_IND:
        {
            Task_DebugPrintf("Info     : BLE - GATT device connected", 0u);
            break;
        }
        
        /* GATT device disconnected */
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
        {
            Task_DebugPrintf("Info     : BLE - GATT device disconnected", 0u); 
            Ble_RemoveDevice(((cy_stc_ble_conn_handle_t*)eventParam)->bdHandle);
            break;
        }
        
        /* GATT MTU Exchange Request is received from GATT Client device */
        case CY_BLE_EVT_GATTS_XCNHG_MTU_REQ:
        {
            Task_DebugPrintf("Failure! : BLE - GATT MTU Exchange Request received" \
                "from GATT Client device", 0u);
            break;
        }
        
        /* Other BLE events */
        default:
        {
            Task_DebugPrintf("Info     : BLE Event ", eventType);
            break;
        }
    }
}

/*******************************************************************************
* Function Name: static void BasCallback(uint32_t event, void *eventParam)
********************************************************************************
* Summary:
*  Call back event function to handle various BAS events from the BLE stack.
*
* Parameters:
*  event        :	BLE event occurred
*  eventParam   :	Pointer to the value of event specific parameters
*
* Return:
*  None
*
*******************************************************************************/
static void BasCallback(uint32_t event, void *eventParam)
{
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    /* Variable used to store the BAS event parameter */
    cy_stc_ble_bas_char_value_t* basCharValue = 
        (cy_stc_ble_bas_char_value_t*)eventParam;
    
    switch(event)
    {
        /* This event indicates notification for Battery Level Characteristic
           was enabled */
        case CY_BLE_EVT_BASS_NOTIFICATION_ENABLED:
        {
            basNotificationFlag |= (1 << basCharValue->serviceIndex);
            
            Task_DebugPrintf("Info     : BLE - BAS notification enabled for" \
                "device", (basCharValue->serviceIndex + 1));
            break;
        }
        /* This event indicates notification for Battery Level Characteristic 
           was disabled */
        case CY_BLE_EVT_BASS_NOTIFICATION_DISABLED:
        {
            basNotificationFlag &= ~(1 << basCharValue->serviceIndex);
            
            Task_DebugPrintf("Info     : BLE - BAS notification disabled"
                "for device", (basCharValue->serviceIndex + 1));
            break;
        }
        /* This event indicates Battery Level Characteristic notification
           was received */
        case CY_BLE_EVT_BASC_NOTIFICATION:
        {
            /* Update the battery level locally */
            
            UpdateBatteryLevel(basCharValue->connHandle.bdHandle, 
                basCharValue->value->val);
            
            /* Send BLE command for sending notification */
            ble_command_t bleCommand = {.command = SEND_NOTIFICATION};
            rtosApiResult = xQueueSend(bleCommandQ, &bleCommand, 0);
            /* Check if the operation has been successful */
            if(rtosApiResult != pdTRUE)
            {
                Task_DebugPrintf("Failure!  : BLE - Send notification command", \
                    rtosApiResult);
            }
            break;
        }
        /* This event indicates read response for Battery Level Characteristic
           Value received */
        case CY_BLE_EVT_BASC_READ_CHAR_RESPONSE:
        {
            Task_DebugPrintf("Info     : BLE - BAS characteristic read response", 0u);
            break;
        }
        /* This event indicates read response for Battery Level Descriptor 
           Value received */
        case CY_BLE_EVT_BASC_READ_DESCR_RESPONSE:
        {
            Task_DebugPrintf("Info     : BLE - BAS descriptor read request response", 0u);
            break;
        }
        /* This event indicates write response for Battery Level Descriptor
           Value received */
        case CY_BLE_EVT_BASC_WRITE_DESCR_RESPONSE:
        {
            Task_DebugPrintf("Info     : BLE - BAS descriptor write request response", 0u);
            break;
        }
        /* Other BAS events */
        default:
        {
            break;
        }
    }
}

/*******************************************************************************
* Function Name: static void BleControllerInterruptEventHandler(void)
********************************************************************************
* Summary:
*  Call back event function to handle interrupts from BLE Controller
*  (Cortex M0+)
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void BleControllerInterruptEventHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken;
    
    /* Send command to process BLE events  */
    ble_command_t bleCommand = {.command = PROCESS_BLE_EVENTS};
    xQueueSendToFrontFromISR(bleCommandQ, &bleCommand, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken );
}

/*******************************************************************************
* Function Name: static void Ble_DisplayScannedDevice(void)
********************************************************************************
* Summary:
*  Display all the scanned devices with BAS service
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void Ble_DisplayScannedDevice(void)
{    
    /* Local variables */
    uint8_t i;
    uint8_t devCount = 0;
    
    /* String buffer used for storing display messages */
    char8 printBuffer[BLE_MAX_SCAN_DEVICES][MAX_BUFFER_LENGTH];
    
    if(appScanDevInfo.count > 0)
    {
        Task_Printf("\rSelect device to connect 1...", appScanDevInfo.count);
        for(i = 0; i < appScanDevInfo.count; i++)
        {
            sprintf(printBuffer[devCount], \
            "\rDevice %u : %2.2X%2.2X%2.2X%2.2X%2.2X%2.2X %s", (i+1), \
            appScanDevInfo.address[i].bdAddr[5u], appScanDevInfo.address[i].bdAddr[4u],\
            appScanDevInfo.address[i].bdAddr[3u], appScanDevInfo.address[i].bdAddr[2u],\
            appScanDevInfo.address[i].bdAddr[1u], appScanDevInfo.address[i].bdAddr[0u],\
            (Cy_BLE_IsPeerConnected(appScanDevInfo.address[i].bdAddr)?"[CONNECTED]": ""));
            Task_Printf(printBuffer[devCount++], 0u);
        }
    }
    else
    {
        Task_Printf("\rInfo     : No device found with Battery Alert Service",0u);
    }
}

/*******************************************************************************
* Function Name: static void Ble_DisplayConnectedDevice(void)
********************************************************************************
* Summary:
*  Display all the connected devices
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void Ble_DisplayConnectedDevice(void)
{
    /* Variable used for storing device address */
    cy_stc_ble_gap_peer_addr_info_t peerAddress;
        
    /* Local variable */
    uint8_t i;
    uint8_t devCount = 0;
    
    /* String buffer used for storing display messages */
    char8 printBuffer[BLE_MAX_CONN_COUNT][MAX_BUFFER_LENGTH];
    
    for(i = 0; i < appConnInfo.centralCnt; i++)
    {
        if(Cy_BLE_GetConnectionState(appConnInfo.central[i].connHandle) 
            >= CY_BLE_CONN_STATE_CONNECTED)
        {
            peerAddress.bdHandle = appConnInfo.central[i].connHandle.bdHandle;
            
            Cy_BLE_GAP_GetPeerBdAddr(&peerAddress);
            
            sprintf(printBuffer[devCount], "\rDevice %u : " \
            "%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X [PERIPHERAL]", (i+1), \
            peerAddress.bdAddr.bdAddr[5], peerAddress.bdAddr.bdAddr[4],\
            peerAddress.bdAddr.bdAddr[3], peerAddress.bdAddr.bdAddr[2],\
            peerAddress.bdAddr.bdAddr[1], peerAddress.bdAddr.bdAddr[0]);
            
            Task_Printf(printBuffer[devCount++], 0u);
        }
    }
    for(i = 0; i < appConnInfo.peripheralCnt; i++)
    {
        if(Cy_BLE_GetConnectionState(appConnInfo.peripheral[i].connHandle)
            >= CY_BLE_CONN_STATE_CONNECTED)
        {
            peerAddress.bdHandle = appConnInfo.peripheral[i].connHandle.bdHandle;
            
            Cy_BLE_GAP_GetPeerBdAddr(&peerAddress);
            sprintf(printBuffer[devCount], "\rDevice %u : " \
            "%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X [CENTRAL]", \
            (i + BLE_MAX_CENTRAL_CONN_COUNT + 1),
            peerAddress.bdAddr.bdAddr[5], peerAddress.bdAddr.bdAddr[4],
            peerAddress.bdAddr.bdAddr[3], peerAddress.bdAddr.bdAddr[2],
            peerAddress.bdAddr.bdAddr[1], peerAddress.bdAddr.bdAddr[0]);
            
            Task_Printf(printBuffer[devCount++], 0u);
        }
    }
    if((appConnInfo.centralCnt == 0) && (appConnInfo.peripheralCnt == 0))
    {
        Task_Printf("No connected device", 0u);
    }
}

/*******************************************************************************
* Function Name: static void Ble_ConnectDevice(uint32_t deviceId)
********************************************************************************
* Summary:
*  Send a connection request to the remote device
*
* Parameters: 
*  uint8_t deviceId: Device id 
*
* Return:
*  None
*
*******************************************************************************/
static void Ble_ConnectDevice(uint32_t deviceId)
{
    /* Variable used to store the return values of BLE APIs */
    cy_en_ble_api_result_t bleApiResult;
        
    /* Variable used for buffer */
    static char8 printBuffer[32];
    
    /* Check if the device is already connected */
    if(Cy_BLE_IsPeerConnected(appScanDevInfo.address[deviceId - 1].bdAddr))
    {
        Task_Printf("\rInfo     : You are already connected to device :", 
            deviceId);
    }
    else
    {
        /* Stop scanning before connection */
        Ble_Central_StopScan();
        
        /* Send connect request */
        bleApiResult = Cy_BLE_GAPC_ConnectDevice(&appScanDevInfo.address[--deviceId],\
            CY_BLE_CENTRAL_CONFIGURATION_0_INDEX);
        if(bleApiResult != CY_BLE_SUCCESS)
        {
            Task_DebugPrintf("Failure!  : BLE - Connection request ", bleApiResult);
        }
        else
        {
            sprintf(printBuffer, "\rConnecting with - %2.2X%2.2X%2.2X%2.2X%2.2X%2.2X", \
                appScanDevInfo.address[deviceId].bdAddr[5u],\
                appScanDevInfo.address[deviceId].bdAddr[4u],\
                appScanDevInfo.address[deviceId].bdAddr[3u],\
                appScanDevInfo.address[deviceId].bdAddr[2u],\
                appScanDevInfo.address[deviceId].bdAddr[1u],\
                appScanDevInfo.address[deviceId].bdAddr[0u]);
            
            Task_Printf(printBuffer, 0u);
        }
        Cy_BLE_ProcessEvents();
    }
}

/*******************************************************************************
* Function Name: static void Ble_DisconnectDevice(uint32_t deviceId)
********************************************************************************
*
* Summary:
*  Disconnects a peer device.
*
* Parameters:
*  uint8_t deviceId : Device ID
*
* Return:
*  None
*
*******************************************************************************/
static void Ble_DisconnectDevice(uint32_t deviceId)
{
    /* Variable used to store the return values of BLE APIs */
    cy_en_ble_api_result_t bleApiResult;
    /* Variable used to store disconnect information */
    cy_stc_ble_gap_disconnect_info_t param =
    {
        .reason   = CY_BLE_HCI_ERROR_OTHER_END_TERMINATED_USER
    };
    
    if(deviceId <= BLE_MAX_CENTRAL_CONN_COUNT)
    {
        param.bdHandle = appConnInfo.central[deviceId-1].connHandle.bdHandle;
    }
    else
    {
        param.bdHandle = \
        appConnInfo.peripheral[deviceId - 1 - BLE_MAX_CENTRAL_CONN_COUNT].connHandle.bdHandle;
    }
    /* Terminate connection with specified peer device */
    bleApiResult = Cy_BLE_GAP_Disconnect(&param);
    if(bleApiResult != CY_BLE_SUCCESS)
    {
        Task_DebugPrintf("Failure! : BLE - Disconnect API ", bleApiResult);
    }
    Cy_BLE_ProcessEvents();
}

/*******************************************************************************
* Function Name: static void Ble_Central_StartScan(void)
********************************************************************************
*
* Summary:
*  This function is used for discovering GAP peripheral devices with BAS that are
*  available for connection.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void Ble_Central_StartScan(void)
{
    /* Variable used to store the return values of BLE APIs */
    cy_en_ble_api_result_t bleApiResult;
    if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_STOPPED)
    {
        bleApiResult = Cy_BLE_GAPC_StartScan(CY_BLE_SCANNING_FAST, 0u);
        if(bleApiResult != CY_BLE_SUCCESS)
        {
            Task_DebugPrintf("Failure!  : BLE - Failed to start scan : \r\n", \
                bleApiResult);
        }
        Cy_BLE_ProcessEvents();
    }
}

/*******************************************************************************
* Function Name: static void Ble_Central_StopScan(void)
********************************************************************************
*
* Summary:
*  This function is used to stop scanning.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void Ble_Central_StopScan(void)
{
    /* Variable used to store the return values of BLE APIs */
    cy_en_ble_api_result_t bleApiResult;
    if(Cy_BLE_GetScanState() != CY_BLE_SCAN_STATE_STOPPED)
    {
        bleApiResult = Cy_BLE_GAPC_StopScan();
        if(bleApiResult != CY_BLE_SUCCESS)
        {
            Task_DebugPrintf("Failure!  : BLE - Stop scan", bleApiResult);
        }
        Cy_BLE_ProcessEvents();
    }
}

/*******************************************************************************
* Function Name: static void Ble_Peripheral_StartAdvertisement(void)
********************************************************************************
*
* Summary:
*  This function is used to start the advertisement.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void Ble_Peripheral_StartAdvertisement(void)
{
    /* Variable used to store the return values of BLE APIs */
    cy_en_ble_api_result_t bleApiResult;
    /* Enter into discoverable mode so that remote can find it */
    if((Cy_BLE_GetAdvertisementState() != CY_BLE_ADV_STATE_ADVERTISING) && \
        (appConnInfo.peripheralCnt < BLE_MAX_PERIPHERAL_CONN_COUNT))
    {
        bleApiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, \
            CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
        if(bleApiResult != CY_BLE_SUCCESS)
        {
            Task_DebugPrintf("Failure!  : BLE - start advertisement", \
                bleApiResult);
        }
        Cy_BLE_ProcessEvents();
    }
}

/*******************************************************************************
* Function Name: static void Ble_Peripheral_StopAdvertisement(void)
********************************************************************************
*
* Summary:
*  This function is used to stop the advertisement.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void Ble_Peripheral_StopAdvertisement(void)
{
    /* Variable used to store the return values of BLE APIs */
    cy_en_ble_api_result_t bleApiResult;
    if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
    {
        bleApiResult = Cy_BLE_GAPP_StopAdvertisement();
        if(bleApiResult != CY_BLE_SUCCESS)
        {
            Task_DebugPrintf("Failure!  : BLE - Stop advertisement", \
                bleApiResult);
        }
        Cy_BLE_ProcessEvents();
    }
}

/****************************************************************************** 
* Function Name: static bool Ble_IsServiceUuidPresent(
*    cy_stc_ble_gapc_adv_report_param_t *eventParam, uint16_t uuid)
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
*   true: when the advertising packet contains service UUID.
*
******************************************************************************/
static bool Ble_IsServiceUuidPresent(
    cy_stc_ble_gapc_adv_report_param_t *eventParam, uint16_t uuid)
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
            /* Compare UUID values with input parameter */
            for(i = 0u; (i < (eventParam->data[advIndex] - 1u)) && \
                (servicePresent == false); i += sizeof(uint16_t))
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

/****************************************************************************** 
* Function Name: static bool Ble_AddDeviceInScanList(
*                   uint8_t* peerBdAddr, uint8_t peerAddrType)
*******************************************************************************
*
* Summary:
*   This function add new device in scanned device list
*
* Parameters:
*   uint8_t* peerBdAddr : Public device address
*   uint8_t peerAddrType: BD address type of the device advertising
*
* Return:
*   true: if device is added successfully in list
*
******************************************************************************/
static bool Ble_AddDeviceInScanList(uint8_t* peerBdAddr, uint8_t peerAddrType)
{
    bool newDeviceAdded = false;
    if(!Ble_IsDeviceInScanList(peerBdAddr))
    {
        newDeviceAdded = true;
        memcpy(&appScanDevInfo.address[appScanDevInfo.count].bdAddr,\
                        peerBdAddr, CY_BLE_GAP_BD_ADDR_SIZE); 
        appScanDevInfo.address[appScanDevInfo.count].type = peerAddrType;
        appScanDevInfo.count++;
    }
    return newDeviceAdded;
}

/****************************************************************************** 
* Function Name: static void Ble_Peripheral_SendNotification(void)
*******************************************************************************
*
* Summary:
*   This function send BAS notification 
*
* Parameters:
*   None
*
* Return:
*   None
*
******************************************************************************/
static void Ble_Peripheral_SendNotification(void)
{
    uint32_t i;
    for(i = 0; i < BLE_MAX_CENTRAL_CONN_COUNT; i++ )
    {
        if((IS_CONNECTED(appConnInfo.central[i].connHandle)) && \
        (IS_CONNECTED(appConnInfo.peripheral[0].connHandle)) && \
        (appConnInfo.central[i].isNewNotification)           && \
        (basNotificationFlag & (1 << i)))
        {
            appConnInfo.central[i].isNewNotification = false;
            /* Update the Battery level characteristic value and send notification */
            Cy_BLE_BASS_SendNotification(
                appConnInfo.peripheral[0u].connHandle, i,
                CY_BLE_BAS_BATTERY_LEVEL, sizeof(uint8_t),
                &appConnInfo.central[i].battaryLevel);
        }
    }
}

/****************************************************************************** 
* Function Name: static void Ble_Peripheral_AddDevice(uint8_t bdHandle)
*******************************************************************************
*
* Summary:
*   This function add new device in connected peripheral list
*
* Parameters:
*   uint8_t bdHandle : Device BD Handle
*
* Return:
*   None
*
******************************************************************************/
static void Ble_Peripheral_AddDevice(uint8_t bdHandle)
{
    uint32_t i;
    for (i = 0; i < BLE_MAX_PERIPHERAL_CONN_COUNT; i++)
    {
        if(IS_DISCONNECTED(appConnInfo.peripheral[i].connHandle))
        {   
            appConnInfo.peripheral[i].connHandle = 
            	Cy_BLE_GetConnHandleByBdHandle(bdHandle);
            appConnInfo.peripheralCnt++;
            break;
        }
    }
}

/****************************************************************************** 
* Function Name: static void Ble_Central_AddDevice(uint8_t bdHandle)
*******************************************************************************
*
* Summary:
*   This function add new device in connected central list
*
* Parameters:
*   uint8_t bdHandle : Public device BD Handle
*
* Return:
*   None
*
******************************************************************************/
static void Ble_Central_AddDevice(uint8_t bdHandle)
{
    uint32_t i;
    for(i = 0; i< BLE_MAX_CENTRAL_CONN_COUNT; i++)
    {
        if(IS_DISCONNECTED(appConnInfo.central[i].connHandle))
        {
            /* Send an authorization request */
            cy_ble_authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].bdHandle = bdHandle;
            Cy_BLE_GAP_AuthReq(&cy_ble_authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);
            
            appConnInfo.central[i].connHandle = Cy_BLE_GetConnHandleByBdHandle(bdHandle);
            
            appConnInfo.centralCnt++;
            break;
        }
    }
}

/****************************************************************************** 
* Function Name: static void Ble_AddDevice(
*                               cy_stc_ble_gap_connected_param_t *connParam)
*******************************************************************************
*
* Summary:
*   This function adds device in database.
*
* Parameters:
*   cy_stc_ble_gap_connected_param_t *connParam
*
* Return:
*   None
*
******************************************************************************/
static void Ble_AddDevice(cy_stc_ble_gap_connected_param_t *connParam)
{
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    /* Check if connected as Central or Peripheral */
    if(connParam->role == CY_BLE_GAP_LL_ROLE_SLAVE)
    {
        /* Connected as Peripheral (Slave role) */
        Task_DebugPrintf("Info     : BLE - GAP device connected as peripheral", 0u);
        if(appConnInfo.peripheralCnt < BLE_MAX_PERIPHERAL_CONN_COUNT)
        {
            Ble_Peripheral_AddDevice(connParam->bdHandle);
        }
        
        /* Turn on the Orange LED to indicate connection */
        status_led_data_t statusLedData = 
        {   .orangeLed = LED_TURN_ON,
            .redLed    = LED_NO_CHANGE,
        };
        rtosApiResult = xQueueSend(statusLedDataQ, &statusLedData, 0u);
        /* Check if the operation has been successful */
        if(rtosApiResult != pdTRUE)
        {
            Task_DebugPrintf("Failure! : BLE - Sending data to Status LED queue", 0u);   
        }
    }
    /* Device is connected as central */
    else
    {
        if(appConnInfo.centralCnt < BLE_MAX_CENTRAL_CONN_COUNT)
        {
            Ble_Central_AddDevice(connParam->bdHandle);
        }
        
        if(appConnInfo.centralCnt == BLE_MAX_CENTRAL_CONN_COUNT)
        {
            /* Turn on the Red LED to indicate maximum number allowed 
               GATT client devices are connected */
            status_led_data_t statusLedData = 
            {   .orangeLed = LED_NO_CHANGE,
                .redLed    = LED_TURN_ON,
            };
            rtosApiResult = xQueueSend(statusLedDataQ, &statusLedData, 0u);
            /* Check if the operation has been successful */
            if(rtosApiResult != pdTRUE)
            {
                Task_DebugPrintf("Failure! : BLE - Sending data to Status LED queue", 0u);   
            }
        }
    }
}

/****************************************************************************** 
* Function Name: static void Ble_RemoveDevice(uint8_t bdHandle)
*******************************************************************************
*
* Summary:
*   This function removes device from the list of connected device
*
* Parameters:
*   uint8_t bdHandle : BD Handle of the peer device
*
* Return:
*   None
*
******************************************************************************/
static void Ble_RemoveDevice(uint8_t bdHandle)
{
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    uint32_t i;
    for(i = 0; i < BLE_MAX_CENTRAL_CONN_COUNT; i++)
    {
        if(appConnInfo.central[i].connHandle.bdHandle == bdHandle)
        {
            appConnInfo.central[i].connHandle.bdHandle = \
                CY_BLE_INVALID_CONN_HANDLE_VALUE;
            appConnInfo.central[i].connHandle.attId = \
                CY_BLE_INVALID_CONN_HANDLE_VALUE;
            appConnInfo.centralCnt--;
            break;
        }
    }
     for(i = 0; i < BLE_MAX_PERIPHERAL_CONN_COUNT; i++)
    {
        if(appConnInfo.peripheral[i].connHandle.bdHandle == bdHandle)
        {
            appConnInfo.peripheral[i].connHandle.bdHandle = \
                CY_BLE_INVALID_CONN_HANDLE_VALUE;
            appConnInfo.peripheral[i].connHandle.attId = \
                CY_BLE_INVALID_CONN_HANDLE_VALUE;
            appConnInfo.peripheralCnt--;
            break;
        }
    }
    
    /* Turn on the Orange LED to indicate connection */
    status_led_data_t statusLedData = 
    {   .orangeLed = LED_NO_CHANGE,
        .redLed    = LED_NO_CHANGE,
    };
    
    if(appConnInfo.centralCnt != BLE_MAX_CENTRAL_CONN_COUNT)
    {
        statusLedData.orangeLed = LED_TURN_OFF;
    }
    if(appConnInfo.peripheralCnt != BLE_MAX_PERIPHERAL_CONN_COUNT)
    {
        statusLedData.redLed = LED_TURN_OFF;
    }
    rtosApiResult = xQueueSend(statusLedDataQ, &statusLedData, 0u);
    /* Check if the operation has been successful */
    if(rtosApiResult != pdTRUE)
    {
        Task_DebugPrintf("Failure! : BLE - Sending data to Status LED" \
                         "queue", 0u);   
    }
}

/****************************************************************************** 
* Function Name: static void UpdateBatteryLevel(uint8_t bdHandle, uint8_t* value)
*******************************************************************************
*
* Summary:
*   This function removes device from the list of connected device
*
* Parameters:
*   uint8_t bdHandle : BD Handle of the peer device
*   uint8_t* value : Battery level value 
*
* Return:
*   None
*
******************************************************************************/
static void UpdateBatteryLevel(uint8_t bdHandle, uint8_t* value)
{
    uint32_t i;
    for(i = 0; i < BLE_MAX_CENTRAL_CONN_COUNT; i++)
    {
        if(appConnInfo.central[i].connHandle.bdHandle == bdHandle)
        {
            appConnInfo.central[i].battaryLevel = *value;
            appConnInfo.central[i].isNewNotification = true;
        }
    }
}

/****************************************************************************** 
* Function Name: static bool IsDeviceInScanList(uint8_t* peerBdAddr)
*******************************************************************************
*
* Summary:
*   This function returns if the device is present in scanned device list 
*
* Parameters:
*   uint8_t* peerBdAddr : Device address
*
* Return:
*   true : if device is in scanned device list
*
******************************************************************************/
static bool Ble_IsDeviceInScanList(uint8_t* peerBdAddr)
{
    bool isDevicePresent = false;
    uint32_t i;
    for(i = 0u; i < BLE_MAX_SCAN_DEVICES; i++)
    {
        if(!(memcmp(&appScanDevInfo.address[i].bdAddr, peerBdAddr, 
            CY_BLE_GAP_BD_ADDR_SIZE)))
        {
            isDevicePresent = true;
        }
    } 
    return(isDevicePresent);
}

/*******************************************************************************
* Function Name: static void PrintMenu(void)
********************************************************************************
* Summary: 
*  Display the menu.
*
* Parameters:
*  None
*
* Return:
*  None
*******************************************************************************/
static void PrintMenu(void)
{
    printf("\r\nPlease select operations:\r\n"); 
    printf("'s' - start scanning for BLE devices\r\n");
    printf("'x' - stop scanning\r\n");
    printf("'a' - start advertisement\r\n");
    printf("'z' - stop advertisement\r\n");
    printf("'c' - send connect request to peer device\r\n"); 
    printf("'d' - send disconnect request to peer device\r\n");
    printf("'p' - display connected devices\r\n\n");
}

/* [] END OF FILE */
