/*******************************************************************************
* File Name: user_interface.c
*
* Version: 1.0
*
* Description:
*  This file contains user interface related source.
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "user_interface.h"


/*******************************************************************************
* Function Name: void InitUserInterface(void)
********************************************************************************
*
* Summary:
*   Initialization the user interface: LEDs, SW2, etc. 
*
*******************************************************************************/
void InitUserInterface(void)
{
    /* Initialize wakeup pin for Hibernate */
    Cy_SysPm_SetHibWakeupSource(CY_SYSPM_HIBPIN1_LOW);
    
    /* Initialize LEDs */
    DisableAllLeds();
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
    if((Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING) &&
                   (cy_ble_advIndex == CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX))
    {        
        Disconnect_LED_Write(LED_OFF);
        Simulation_LED_Write(LED_OFF);
        Advertising_LED_INV();
    }
    else if((Cy_BLE_GetState() == CY_BLE_STATE_INITIALIZING) ||
            (Cy_BLE_GetState() == CY_BLE_STATE_STOPPED) ||
            (Cy_BLE_GetNumOfActiveConn() == 0u))
    {   
        Advertising_LED_Write(LED_OFF);
        Disconnect_LED_Write(LED_ON);
        Simulation_LED_Write(LED_OFF);
    }
    else if(Cy_BLE_GetNumOfActiveConn() != 0u)
    {   
        app_stc_connection_info_t *appConnInfoPtr = GetAppConnInfoPtr();
    
        Advertising_LED_Write(LED_OFF);
        Disconnect_LED_Write(LED_OFF);
        
        if((Cy_BLE_GetConnectionState(appConnInfoPtr->central.connHandle) >= CY_BLE_CONN_STATE_CONNECTED) &&
           (appConnInfoPtr->central.isNotificationEn == true))
        {
            Simulation_LED_INV();
        }
        else
        {
            Simulation_LED_Write(LED_OFF);
        }
    }
#else
    if((Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING) &&
                   (cy_ble_advIndex == CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX))
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
* Function Name: PrintConnDevList
********************************************************************************
*
* Summary:
*   Prints list of connected devices.
*
* Return:
*  Number of connected devices
*
*******************************************************************************/
uint8_t PrintConnDevList(void)
{    
    app_stc_connection_info_t *appConnInfoPtr = GetAppConnInfoPtr();
    uint8_t num = 1u;

    DBG_PRINTF("\r\n---------------------------------------------------------- \r\n");
    DBG_PRINTF("Connected devices list: \r\n");

    /* Print Client info */
    if(Cy_BLE_GetConnectionState(appConnInfoPtr->central.connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
    {
        /* Get BdAddress from bdHandle of connected device */
        cy_stc_ble_gap_peer_addr_info_t param = {.bdHandle = appConnInfoPtr->central.connHandle.bdHandle};
        Cy_BLE_GAP_GetPeerBdAddr(&param);
          
        PRINT_DEVICE_INFO(num++, param.bdAddr, appConnInfoPtr->central.connHandle, "CLIENT" );                
    }
    
    /* Print Server info */
    if(Cy_BLE_GetConnectionState(appConnInfoPtr->peripheral.connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
    {
        /* Get BdAddress from bdHandle of connected device */
        cy_stc_ble_gap_peer_addr_info_t param = {.bdHandle = appConnInfoPtr->peripheral.connHandle.bdHandle};
        Cy_BLE_GAP_GetPeerBdAddr(&param);
          
        PRINT_DEVICE_INFO(num, param.bdAddr, appConnInfoPtr->peripheral.connHandle, "SERVER" );                
    }
    
    DBG_PRINTF("---------------------------------------------------------- \r\n");
    return num;
}


/*******************************************************************************
* Function Name: PrintMenu
********************************************************************************
*
* Summary:
*   Print Menu
*
*******************************************************************************/
void PrintMenu(void)
{
    DBG_PRINTF("\r\nSelect operation:\r\n"); 
    DBG_PRINTF("'a' -- start connectable advertisement as HRM Server \r\n"); 
    DBG_PRINTF("'s' -- start scanning for the HRM sensor \r\n");
    DBG_PRINTF("'c' -- send connect request to peer device (HRM sensor). \r\n");
    DBG_PRINTF("'b' -- start broadcasting as Eddystone-URL Beacon \r\n"); 
    DBG_PRINTF("'o' -- start observer role \r\n"); 
    DBG_PRINTF("'d' -- send disconnect request to peer device \r\n"); 
    DBG_PRINTF("'p' -- print list of connected devices \r\n");
    DBG_PRINTF(" -------------------------- \r\n");
    DBG_PRINTF("'1' -- stop scanning \r\n"); 
    DBG_PRINTF("'2' -- stop advertising \r\n\r\n");  
}


/*******************************************************************************
* Function Name: ProcessUartCommands
********************************************************************************
*
* Summary:
*   Process UART user commands
*
*******************************************************************************/
void ProcessUartCommands(char8 command)
{
    cy_en_ble_api_result_t apiResult;
    app_stc_connection_info_t  *appConnInfoPtr    = GetAppConnInfoPtr();
    app_stc_scan_device_list_t *appScanDevInfoPtr = GetAppScanDevInfoPtr();
    uint32_t deviceN;
    uint32_t i;
    
    switch(command)
    {     
        case 'a':  /* Start advertising as HRM Server */  
        case 'b':  /* Start broadcasting as Eddystone-URL Beacon */  
        {
            uint8_t advParamIdx = CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX;
            
            if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
            {
                Cy_BLE_GAPP_StopAdvertisement();
                
                /* Wait stop advertising */
                while(Cy_BLE_GetAdvertisementState() != CY_BLE_ADV_STATE_STOPPED)
                {
                   Cy_BLE_ProcessEvents();     
                }
            }
           
            if(command == 'a')
            {
                DBG_PRINTF("\r\nStart advertising as HRM Server \r\n");
                advParamIdx = CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX;
                
                if(Cy_BLE_GetConnectionState(appConnInfoPtr->peripheral.connHandle) == CY_BLE_CONN_STATE_CONNECTED)
                {
                    DBG_PRINTF("HRM Server is connected, please perform disconnection before start Advertising \r\n");
                }
            }
            
            if(command == 'b')
            {
                /* Eddystone format details: https://developers.google.com/beacons/eddystone */
                DBG_PRINTF("\r\nStart broadcasting as Eddystone-URL Beacon with link to: http://www.cypress.com\r\n");
                advParamIdx = CY_BLE_BROADCASTER_CONFIGURATION_0_INDEX;
            }

            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, advParamIdx);   
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("StartAdvertisement API Error: 0x%x \r\n", apiResult);
            }    
            break;   
        }   
        
        case 'o':  /* Start scanning in observer role */  
        case 's':  /* Start scanning for the HRM sensor */
        {
            uint8_t scanParamIdx = CY_BLE_CENTRAL_CONFIGURATION_0_INDEX;
            
            if(Cy_BLE_GetScanState() != CY_BLE_SCAN_STATE_STOPPED)
            {
                apiResult = Cy_BLE_GAPC_StopScan();
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAPC_StopScan API Error: 0x%x \r\n", apiResult);
                }
                Cy_BLE_ProcessEvents();
            }
            
            if(command == 's')
            {  
                DBG_PRINTF("\r\nStart scanning for the HRM sensor \r\n");
                scanParamIdx = CY_BLE_CENTRAL_CONFIGURATION_0_INDEX;
            }
            
            if(command == 'o')
            {
                DBG_PRINTF("\r\nStart scanning in Observer mode \r\n");
                scanParamIdx = CY_BLE_OBSERVER_CONFIGURATION_0_INDEX;
            }
            
            apiResult = Cy_BLE_GAPC_StartScan(CY_BLE_SCANNING_FAST, scanParamIdx);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAPC_StartScan API Error: 0x%x \r\n", apiResult);
            }
            else
            {
                DBG_PRINTF("Cy_BLE_GAPC_StartScan API Success \r\n");
            }
            break;
            
        }
        
        case 'c': /* Connect to the detected device */
        {
            if(appScanDevInfoPtr->count != 0u) 
            {
                /* Print a list of scanned devices from peerAddr array */
                DBG_PRINTF("Detected device: \r\n");
                for(i = 0u; i < appScanDevInfoPtr->count; i++)
                {
                   /* print devices Addr of detected client */
                    DBG_PRINTF("Device %ld: %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x ", i+1,
                        appScanDevInfoPtr->address[i].bdAddr[5u], appScanDevInfoPtr->address[i].bdAddr[4u], 
                        appScanDevInfoPtr->address[i].bdAddr[3u], appScanDevInfoPtr->address[i].bdAddr[2u],
                        appScanDevInfoPtr->address[i].bdAddr[1u], appScanDevInfoPtr->address[i].bdAddr[0u]);
                
                    if(Cy_BLE_IsPeerConnected(appScanDevInfoPtr->address[i].bdAddr) == true)
                    {
                        DBG_PRINTF("[CONNECTED] \r\n");
                    }
                    else
                    {
                        DBG_PRINTF("\r\n");
                    }
                }
                
                /* Select a device for connection */  
                DBG_PRINTF("select device for connection:  (1..%lx):\r\n", appScanDevInfoPtr->count);        
                while((command = UART_DEB_GET_CHAR()) == UART_DEB_NO_DATA)
                {
                    Cy_BLE_ProcessEvents();
                    appScanDevInfoPtr->pauseScanProgress = true;
                }
                
                if((command > '0') && (command <= (appScanDevInfoPtr->count + '0')))
                {
                    deviceN = (uint8_t)(command - '0');
                    DBG_PRINTF("%c \r\n",command); /* print number */
                    
                    if(Cy_BLE_IsPeerConnected(appScanDevInfoPtr->address[deviceN - 1u].bdAddr) == false)
                    {
                        /* Save index of selected device for connection */
                        appScanDevInfoPtr->connReq = deviceN;
                        
                        if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_SCANNING)
                        {
                            /* Stop scanning before connection  */
                            Cy_BLE_GAPC_StopScan();
                        }
                        else
                        {
                            Connect(&appScanDevInfoPtr->address[appScanDevInfoPtr->connReq - 1u]);
                            appScanDevInfoPtr->connReq = 0u;
                        }

                    }
                    else
                    {
                        DBG_PRINTF("You are already connected to selected peer \r\n");
                    } 
                }
                else
                {
                    DBG_PRINTF(" Wrong digit \r\n");
                    break;
                }
                
                appScanDevInfoPtr->pauseScanProgress = false;
                
            }
            else
            {
                DBG_PRINTF("Connecting list is empty. Press 's' to start scan \r\n");
            }
            break;
        }        
        
        case '1': /* Stop scanning */
        {
            if(Cy_BLE_GetScanState() != CY_BLE_SCAN_STATE_STOPPED)
            {
                apiResult = Cy_BLE_GAPC_StopScan();
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAPC_StopScan API Error: 0x%x \r\n", apiResult);
                }
            }
            break;
        }
        
        case '2': /* Stop advertising */
        {
            if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
            {
                Cy_BLE_GAPP_StopAdvertisement();
                
                /* wait stop advertising */
                while(Cy_BLE_GetAdvertisementState() != CY_BLE_ADV_STATE_STOPPED)
                {
                   Cy_BLE_ProcessEvents();     
                }        
            }
            break;
        }
        
        case 'p': /* Print list of connected devices */
            (void)PrintConnDevList();
            break;
        
        case 'd': /* Disconnect device */
        {
            uint32_t num = 1u;
            cy_stc_ble_conn_handle_t connHandle;
            uint32_t peerDevNum;
            
            /* Print list of connected devices */
            peerDevNum = PrintConnDevList();
            
            DBG_PRINTF("Select a device for disconnection: (1..%x):\r\n", Cy_BLE_GetNumOfActiveConn());        
            while((command = UART_DEB_GET_CHAR()) == UART_DEB_NO_DATA)
            {
                Cy_BLE_ProcessEvents();   
            }
            
            if((command > '0') && (command <= (peerDevNum + '0')))
            {
                DBG_PRINTF("%c \r\n",command); /* print number */
                deviceN = (uint8_t)(command - '0');

                /* Find ConnHandle by selected device number */
                if((Cy_BLE_GetConnectionState(appConnInfoPtr->central.connHandle) >= CY_BLE_CONN_STATE_CONNECTED) &&
                   (deviceN == num++)) 
                {
                    connHandle = appConnInfoPtr->central.connHandle;
                }
                
                if((Cy_BLE_GetConnectionState(appConnInfoPtr->peripheral.connHandle) >= CY_BLE_CONN_STATE_CONNECTED) &&
                   (deviceN == num++))
                {
                    connHandle = appConnInfoPtr->peripheral.connHandle;
                }
                
                DBG_PRINTF("Selected device for disconnection: %ld \r\n", deviceN);                                
                
                /* Perform disconnect operation */
                Disconnect(connHandle);
            }
            else
            {
                DBG_PRINTF(" Wrong digit \r\n");
            }
            break;
        }
        
        case 'h':
            PrintMenu();
            break;
            
        default:
            DBG_PRINTF("Unsupported command\r\n");
            break;
    }
}

/* [] END OF FILE */
