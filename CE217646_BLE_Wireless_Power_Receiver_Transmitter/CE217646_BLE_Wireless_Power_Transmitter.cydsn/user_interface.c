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
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
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
    Cy_SysPm_SetHibernateWakeupSource(CY_SYSPM_HIBERNATE_PIN1_LOW);
    
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
    if(Cy_BLE_GetNumOfActiveConn() > 0u)
    {
        /* In connected state, turn off scanning indication LEDs */            
        Scanning_LED_Write(LED_OFF);
    }
    else if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_SCANNING)
    {
        /* Blink scaning indication LED */
        Scanning_LED_INV();
    }
    else
    {
        /* If in disconnected state, turn on Scanning LED */
        Scanning_LED_Write(LED_ON);
    }
}


/*******************************************************************************
* Function Name: ProcessUartCommands
********************************************************************************
*
* Summary:
*   Processes UART user commands
*
*******************************************************************************/
void ProcessUartCommands(char8 command)
{
    cy_en_ble_api_result_t apiResult;
    uint16_t alertCCCD = 0u;
    uint32_t i;
    
    switch(command)
    { 
    case '1':                   /* Enable Notification */
        alertCCCD |= CY_BLE_CCCD_NOTIFICATION;
        for(i = 0u; i < CY_BLE_CONN_COUNT; i++)
        {  
            if(Cy_BLE_GetConnectionState(cy_ble_connHandle[i]) == CY_BLE_CONN_STATE_CLIENT_DISCOVERED) 
            {
                while(appConnInfo.central[i].requestResponce != false)
                {
                    Cy_BLE_ProcessEvents();
                }
                apiResult = Cy_BLE_WPTSC_SetCharacteristicDescriptor(cy_ble_connHandle[i], 
                    CY_BLE_WPTS_PRU_ALERT, CY_BLE_WPTS_CCCD, sizeof(alertCCCD), (uint8_t *)&alertCCCD);
                DBG_PRINTF("Device %lu-->Enable Alert Notification, apiResult: %x \r\n", 
                            (unsigned long)i, apiResult);
                if(apiResult == CY_BLE_SUCCESS)
                {
                    appConnInfo.central[i].requestResponce = true;
                }
            }
        }
        break;
        
    case '2':                   /* Enable Indication */
        alertCCCD |= CY_BLE_CCCD_INDICATION;
        for(i = 0u; i < CY_BLE_CONN_COUNT; i++)
        {  
            if(Cy_BLE_GetConnectionState(cy_ble_connHandle[i]) == CY_BLE_CONN_STATE_CLIENT_DISCOVERED) 
            {
                while(appConnInfo.central[i].requestResponce != false)
                {
                    Cy_BLE_ProcessEvents();
                }
                apiResult = Cy_BLE_WPTSC_SetCharacteristicDescriptor(cy_ble_connHandle[i], 
                    CY_BLE_WPTS_PRU_ALERT, CY_BLE_WPTS_CCCD, sizeof(alertCCCD), (uint8_t *)&alertCCCD);
                DBG_PRINTF("Device %lu-->Enable Alert Indication, apiResult: %x \r\n", 
                            (unsigned long)i, apiResult);
                if(apiResult == CY_BLE_SUCCESS)
                {
                    appConnInfo.central[i].requestResponce = true;
                }
            }
        }
        break;
        
    case '3':  /* Disable Notification and Indication */
        alertCCCD = 0u;
        for(i = 0u; i < CY_BLE_CONN_COUNT; i++)
        {  
            if(Cy_BLE_GetConnectionState(cy_ble_connHandle[i]) == CY_BLE_CONN_STATE_CLIENT_DISCOVERED) 
            {
                while(appConnInfo.central[i].requestResponce != false)
                {
                    Cy_BLE_ProcessEvents();
                }
                apiResult = Cy_BLE_WPTSC_SetCharacteristicDescriptor(cy_ble_connHandle[i], CY_BLE_WPTS_PRU_ALERT,
                                                                     CY_BLE_WPTS_CCCD, sizeof(alertCCCD),
                                                                     (uint8_t *)&alertCCCD);
                DBG_PRINTF("Device %lu-->Disable Alert Notification and Indication, apiResult: %x \r\n", 
                            (unsigned long)i, apiResult);
                if(apiResult == CY_BLE_SUCCESS)
                {
                    appConnInfo.central[i].requestResponce = true;
                }
            }
        }
        break;
        
    case '4': /* Send Read request for PRU Static Parameter characteristic */ 
        for(i = 0u; i < CY_BLE_CONN_COUNT; i++)
        {  
            if(Cy_BLE_GetConnectionState(cy_ble_connHandle[i]) == CY_BLE_CONN_STATE_CLIENT_DISCOVERED) 
            {
                while(appConnInfo.central[i].requestResponce != false)
                {
                    Cy_BLE_ProcessEvents();
                }
                apiResult = Cy_BLE_WPTSC_GetCharacteristicValue(cy_ble_connHandle[i], CY_BLE_WPTS_PRU_STATIC_PAR);
                DBG_PRINTF("Device %lu-->Get PRU Static Parameter char value, apiResult: %x \r\n", 
                            (unsigned long)i, apiResult);
                if(apiResult == CY_BLE_SUCCESS)
                {
                    appConnInfo.central[i].requestResponce = true;
                }
            }
        }
        break;
        
    case '5': /* Send Read request for PRU Dynamic Parameter characteristic */ 
        for(i = 0u; i < CY_BLE_CONN_COUNT; i++)
        {  
            if(Cy_BLE_GetConnectionState(cy_ble_connHandle[i]) == CY_BLE_CONN_STATE_CLIENT_DISCOVERED) 
            {
                while(appConnInfo.central[i].requestResponce != false)
                {
                    Cy_BLE_ProcessEvents();
                }
                apiResult = Cy_BLE_WPTSC_GetCharacteristicValue(cy_ble_connHandle[i], CY_BLE_WPTS_PRU_DYNAMIC_PAR);
                DBG_PRINTF("Device %lu-->Get PRU Dynamic Parameter char value, apiResult: %x \r\n", 
                            (unsigned long)i, apiResult);
                if(apiResult == CY_BLE_SUCCESS)
                {
                    appConnInfo.central[i].requestResponce = true;
                }
            }
        }
        break;
        
    case '6': /* Enable Charging */
        for(i = 0u; i < CY_BLE_CONN_COUNT; i++)
        {  
            if(Cy_BLE_GetConnectionState(cy_ble_connHandle[i]) == CY_BLE_CONN_STATE_CLIENT_DISCOVERED) 
            {
                while(appConnInfo.central[i].requestResponce != false)
                {
                    Cy_BLE_ProcessEvents();
                }
                appConnInfo.central[i].pruControl.enables = PRU_CONTROL_ENABLES_ENABLE_CHARGE_INDICATOR;
                apiResult = Cy_BLE_WPTSC_SetCharacteristicValue(cy_ble_connHandle[i], CY_BLE_WPTS_PRU_CONTROL,
                    sizeof(appConnInfo.central[i].pruControl), (uint8_t *)&appConnInfo.central[i].pruControl);
                DBG_PRINTF("Device %lu-->Set PRU Control char (enable charging), apiResult: %x \r\n", 
                            (unsigned long)i, apiResult);
                if(apiResult == CY_BLE_SUCCESS)
                {
                    appConnInfo.central[i].requestResponce = true;
                    appConnInfo.central[i].pruCharging = true;
                }
            }
        }
        break;
        
    case '7':  /* Disable Charging */
        for(i = 0u; i < CY_BLE_CONN_COUNT; i++)
        {  
            if(Cy_BLE_GetConnectionState(cy_ble_connHandle[i]) == CY_BLE_CONN_STATE_CLIENT_DISCOVERED) 
            {
                while(appConnInfo.central[i].requestResponce != false)
                {
                    Cy_BLE_ProcessEvents();
                }
                
                appConnInfo.central[i].pruControl.enables &= ~PRU_CONTROL_ENABLES_ENABLE_CHARGE_INDICATOR;
                apiResult = Cy_BLE_WPTSC_SetCharacteristicValue(cy_ble_connHandle[i], CY_BLE_WPTS_PRU_CONTROL,
                    sizeof(appConnInfo.central[i].pruControl), (uint8_t *)&appConnInfo.central[i].pruControl);
                DBG_PRINTF("Device %lu-->Set PRU Control char (disable charging), apiResult: %x \r\n", 
                            (unsigned long)i, apiResult);
                if(apiResult == CY_BLE_SUCCESS)
                {
                    appConnInfo.central[i].requestResponce = true;
                    appConnInfo.central[i].pruCharging = false;
                }
            }
        }
        break;
        
    case '8':
        DBG_PRINTF("Enable sequential read of PRU Dynamic Parameter characteristic\r\n");
        for(i = 0u; i < CY_BLE_CONN_COUNT; i++)
        {  
            appConnInfo.central[i].readingDynChar = 1u;
        }
        break;
        
    case '9':
        DBG_PRINTF("Disable sequential read of PRU Dynamic Parameter characteristic\r\n");
        for(i = 0u; i < CY_BLE_CONN_COUNT; i++)
        {  
            appConnInfo.central[i].readingDynChar = 0u;
        }
        break;
    
    case 'c': /* Connect to the detected device */  
        if(appScanDevInfo.count != 0u) 
        {
            if(Cy_BLE_GetNumOfActiveConn() < CY_BLE_CONN_COUNT)
            {
                uint32_t i;
                
                /* Print a list of scanned devices from the peerAddr array */
                DBG_PRINTF("Detected device: \r\n");
                for(i = 0u; i < appScanDevInfo.count; i++)
                {
                   /* Print the devices Addr of the detected Client */
                    DBG_PRINTF("Device %ld: %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x \r\n", i+1,
                                    appScanDevInfo.address[i].bdAddr[5u], appScanDevInfo.address[i].bdAddr[4u], 
                                    appScanDevInfo.address[i].bdAddr[3u], appScanDevInfo.address[i].bdAddr[2u],
                                    appScanDevInfo.address[i].bdAddr[1u], appScanDevInfo.address[i].bdAddr[0u]);         
                }
                
                /* Select a device for connection */  
                DBG_PRINTF("select device for connection:  (1..%lx):\r\n", appScanDevInfo.count);        
                while((command = UART_DEB_GET_CHAR()) == UART_DEB_NO_DATA)
                {
                    appScanDevInfo.pauseScanProgress = true;
                    Cy_BLE_ProcessEvents();   
                }
                
                if((command > '0') && (command <= (appScanDevInfo.count + '0')))
                {
                    /* Save index of selected device for connection */
                    appScanDevInfo.connReq = (uint8_t)(command - '0');
                   
                    /* Stop scanning before connection */
                    if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_SCANNING)
                    {
                        Cy_BLE_GAPC_StopScan();
                    }
                }
                else
                {
                    DBG_PRINTF(" Wrong digit \r\n");
                    break;
                }
                
                appScanDevInfo.pauseScanProgress = false;
                
            }
            else
            {
                DBG_PRINTF("You are already connected to all peer device \r\n");
            }
        }
        else
        {
            DBG_PRINTF("Connecting list is empty. Press 's' to start scan \r\n");
        }     
        break;   
        
    case 'd': /* Disconnect all connected devices */  
        if(Cy_BLE_GetNumOfActiveConn() < CY_BLE_CONN_COUNT)
        {
            for(i = 0u; i < CY_BLE_CONN_COUNT; i++)
            {
                if(Cy_BLE_GetConnectionState(cy_ble_connHandle[i]) >= CY_BLE_CONN_STATE_CONNECTED)
                {
                    cy_stc_ble_gap_disconnect_info_t disconnectParam =
                    {
                        .bdHandle = cy_ble_connHandle[i].bdHandle,
                        .reason = CY_BLE_HCI_ERROR_OTHER_END_TERMINATED_USER
                    };
                    apiResult = Cy_BLE_GAP_Disconnect(&disconnectParam); 
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Disconnect device API Error: %x [attId = %x]\r\n", apiResult,
                                    cy_ble_connHandle[i].attId);
                    }
                    else
                    {
                        DBG_PRINTF("Disconnect device API Success [attId = %x]\r\n", cy_ble_connHandle[i].attId);
                    }             
                }
            }
        }
        else
        {
            DBG_PRINTF("There is no connected device \r\n");
        }     
        break;    
        
    case 'h':  /* Help menu */
        DBG_PRINTF("\r\n");
        DBG_PRINTF("Available commands:\r\n");
        DBG_PRINTF(" \'h\' - Help menu.\r\n");
        DBG_PRINTF(" \'1\' - Enable notifications for Alert characteristic.\r\n");
        DBG_PRINTF(" \'2\' - Enable indications for Alert characteristic.\r\n");
        DBG_PRINTF(" \'3\' - Disable notifications and indication for Alert characteristic.\r\n");
        DBG_PRINTF(" \'4\' - Send Read request for PRU Static Parameter characteristic.\r\n");
        DBG_PRINTF(" \'5\' - Send Read request for PRU Dynamic Parameter characteristic.\r\n");
        DBG_PRINTF(" \'6\' - Send Enable Charging command to PRU control characteristic.\r\n");
        DBG_PRINTF(" \'7\' - Send Disable Charging command to PRU control characteristic.\r\n");
        DBG_PRINTF(" \'8\' - Enable sequential read of PRU Dynamic Parameter characteristic.\r\n");
        DBG_PRINTF(" \'9\' - Disable sequential read of PRU Dynamic Parameter characteristic.\r\n");
        DBG_PRINTF(" \'c\' - Send connect request to peer device.\r\n");
        DBG_PRINTF(" \'d\' - Send disconnect request to all connected peer devices.\r\n");
        break;
    
    default:
        break;
    }
}

/* [] END OF FILE */
