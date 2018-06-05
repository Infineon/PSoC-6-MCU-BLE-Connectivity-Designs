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
*   Initialization the user interface: LEDs, SW2, ect. 
*
*******************************************************************************/
void InitUserInterface(void)
{
    /* Initialize wakeup pin for Hibernate */
    Cy_SysPm_SetHibWakeupSource(CY_SYSPM_HIBPIN1_LOW);
    
    /* Initialize LEDs */
    DisableAllLeds();
    
    /* Initialize Timer */
    Cy_SysInt_Init(&Timer_Int_cfg, Timer_Interrupt);
    NVIC_EnableIRQ(Timer_Int_cfg.intrSrc);   
    MCWDT_SetInterruptMask(CY_MCWDT_CTR0);
    MCWDT_Start(); 
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
        Connected_LED_Write(LED_OFF);
        Advertising_LED_INV();
    }
    else if((Cy_BLE_GetState() == CY_BLE_STATE_INITIALIZING) || (Cy_BLE_GetState() == CY_BLE_STATE_STOPPED) ||
            (Cy_BLE_GetNumOfActiveConn() == 0u))
    {   
        Disconnect_LED_Write(LED_ON);
        Connected_LED_Write(LED_OFF);
        Advertising_LED_Write(LED_OFF);
    }
    else
    {   
        Disconnect_LED_Write(LED_OFF);
        Connected_LED_Write(LED_ON);
        Advertising_LED_Write(LED_OFF);
    }
#else
    if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
    {
        /* Blink advertising indication LED. */
        LED5_INV();
    }
    else if(Cy_BLE_GetNumOfActiveConn() == 0u)
    {
        /* If in disconnected state, turn on disconnect indication LED 
        */
        LED5_Write(LED_ON);
    }
    else 
    {
        /* In connected state
        */
        LED5_Write(LED_OFF);
    }
#endif /* #if(CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV) */    
}


/*******************************************************************************
* Function Name: PrintMenu
********************************************************************************
*
* Display the menu.
*
*******************************************************************************/
void PrintMenu(void)
{
    DBG_PRINTF("\r\nPlease select operations:\r\n"); 
    DBG_PRINTF("'1' -- stop scanning \r\n"); 
    DBG_PRINTF("'2' -- stop advertising \r\n");  
    DBG_PRINTF("'s' -- start scanning for BLE devices\r\n"); 
    DBG_PRINTF("'c' -- send connect request to peer device\r\n"); 
    DBG_PRINTF("'d' -- send disconnect request to peer device\r\n"); 
    DBG_PRINTF("'p' -- print list of connected devices \r\n\r\n"); 
}
 

/*******************************************************************************
* Function Name: PrintConnDevList
********************************************************************************
*
* Summary:
*   Prints a list of connected devices.
*
*******************************************************************************/
void PrintConnDevList(void)
{    
    app_stc_connection_info_t *appConnInfoPtr = GetAppConnInfoPtr();
    cy_stc_ble_conn_handle_t connHandle;
    uint32_t i;
    uint32_t num = 1u;
    bool isCentral = false;
    
    if(appConnInfoPtr != NULL)
    {
        DBG_PRINTF("\r\n---------------------------------------------------------- \r\n");
        DBG_PRINTF("Connected devices list: \r\n");

        /* print client/server info  */
        for(i = 0u; i < CY_BLE_MAX_CONN_NUM; i++)
        {
            isCentral = (i < CY_BLE_MAX_CENTRAL_CONN_NUM) ? true : false;
            
            connHandle = (isCentral == true) ? appConnInfoPtr->central[i].connHandle : 
                                               appConnInfoPtr->peripheral[i - CY_BLE_MAX_CENTRAL_CONN_NUM].connHandle;

            if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
            {
                /* Get BdAddress from bdHandle of the connected device */
                cy_stc_ble_gap_peer_addr_info_t param = {.bdHandle = connHandle.bdHandle};
                Cy_BLE_GAP_GetPeerBdAddr(&param);
                  
                PRINT_DEVICE_INFO(num++, param.bdAddr, connHandle, (isCentral == true) ? "CLIENT" : "SERVER");                
            }
        }

        DBG_PRINTF("---------------------------------------------------------- \r\n");
    }
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
    
    uint32_t i;
    
    switch(command)
    {
        case 's':  /* Start scanning */
        {
            if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_STOPPED)
            {
                apiResult = Cy_BLE_GAPC_StartScan(CY_BLE_SCANNING_FAST, 0u);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAPC_StartScan API Error: 0x%x \r\n", apiResult);
                }
                else
                {
                    DBG_PRINTF("Cy_BLE_GAPC_StartScan API Success \r\n");
                }
            }
            else
            {
                DBG_PRINTF("Scanning is started \r\n");
            }
            break;
        }
        
        case 'c': /* Connect to the detected device */
        {
            if(appScanDevInfoPtr->count != 0u) 
            {      
                /* Print a list of scanned devices from the peerAddr array */
                DBG_PRINTF("Detected device: \r\n");
                for(i = 0u; i < appScanDevInfoPtr->count; i++)
                {
                   /* Print the devices Addr of the detected Client */
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
                }
                if((command > '0') && (command <= (appScanDevInfoPtr->count + '0')))
                {
                    DBG_PRINTF("%c \r\n",command); /* print number */
                    if(Cy_BLE_IsPeerConnected(appScanDevInfoPtr->address[(uint8_t)(command - '0') - 1u].bdAddr) == false)
                    {
                        /* Save index of selected device for connection */
                        appScanDevInfoPtr->connReq = (uint8_t)(command - '0');
                        
                        /* Stop scanning before connection */
                        if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_SCANNING)
                        {
                            Cy_BLE_GAPC_StopScan();
                        }
                    }
                    else
                    {
                        DBG_PRINTF("You are already connected to selected peer \r\n");
                    } 
                }
                else
                {
                    DBG_PRINTF("Wrong digit \r\n");
                    break;
                }
            }
            else
            {
                DBG_PRINTF("Connecting list is empty. Press 's' to start scan \r\n");
            }
            break;
        }     
        
        case 'd': /* Disconnect the device */
        {
            uint32_t num = 1u;
            uint8_t bdHandle = CY_BLE_INVALID_CONN_HANDLE_VALUE;
            
            /* Print a list of connected devices */
            PrintConnDevList();
            
            DBG_PRINTF("select device for disconnect:  (1..%x):\r\n", Cy_BLE_GetNumOfActiveConn());        
            while((command = UART_DEB_GET_CHAR()) == UART_DEB_NO_DATA)
            {
                Cy_BLE_ProcessEvents();
            }
            
            if((command > '0') && (command <= Cy_BLE_GetNumOfActiveConn() + '0'))
            {
                cy_stc_ble_conn_handle_t connHandle;
                bool isCentral;
                
                DBG_PRINTF("%c \r\n",command); /* print number */

                /* Find bdHandle by the selected device number */
                for(i = 0u; i < CY_BLE_MAX_CONN_NUM; i++)
                {
                    isCentral = (i < CY_BLE_MAX_CENTRAL_CONN_NUM) ? true : false;
                    connHandle = (isCentral == true) ? appConnInfoPtr->central[i].connHandle : 
                                                       appConnInfoPtr->peripheral[i - CY_BLE_MAX_CENTRAL_CONN_NUM].connHandle;

                    if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
                    {
                        if(num++ == (uint8_t)(command - '0'))
                        {
                            bdHandle = connHandle.bdHandle;
                            break;
                        }
                    }
                }
                
                /* Perform the disconnect operation */
                if(bdHandle != CY_BLE_INVALID_CONN_HANDLE_VALUE)
                {
                    Disconnect(connHandle);
                }       
            }
            else
            {
                DBG_PRINTF(" Wrong digit \r\n");
            }
            break;
        }
        
        case '1': /* Stop scanning */
            if(Cy_BLE_GetScanState() != CY_BLE_SCAN_STATE_STOPPED)
            {
                apiResult = Cy_BLE_GAPC_StopScan();
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAPC_StopScan API Error: 0x%x \r\n", apiResult);
                }
            }
            break;
        
        case '2': /* Stop advertising */
            if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
            {
                Cy_BLE_GAPP_StopAdvertisement();
            }
            break;
            
        case '3':
            /* Enter into discoverable mode so that remote can find it. */
            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GAPP_StartAdvertisement API Error: 0x%x \r\n", apiResult);
            }
            break;
    
        case 'p': /* Print a list of connected devices */
            PrintConnDevList();
            break;
            
        case 'h':
            PrintMenu();
            break;
            
        default:
            DBG_PRINTF("Unsupported command\r\n");
            break;
    }
    
}

/* [] END OF FILE */
