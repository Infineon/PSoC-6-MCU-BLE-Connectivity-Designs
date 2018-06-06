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
#include "hrsc.h"

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
    if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_SCANNING)
    {
        /* In scanning state, turn off disconnect indication LED */
        Disconnect_LED_Write(LED_OFF);
        
        /* Blink scaning indication LED. */
        Scanning_LED_INV();
    }
    else if(Cy_BLE_GetNumOfActiveConn() == 0u)
    {
        /* If in disconnected state, turn on disconnect indication LED and turn
        * off Scanning LED.
        */
        Disconnect_LED_Write(LED_ON);
        Scanning_LED_Write(LED_OFF);
    }
    else
    {
        /* In connected state, turn off disconnect indication and scanning 
        * indication LEDs. 
        */
        Disconnect_LED_Write(LED_OFF);
        Scanning_LED_Write(LED_OFF);
    }             
#else
    if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_SCANNING)
    {
        /* Blink scanning indication LED. */
        LED5_INV();
    }
    else  if(Cy_BLE_GetNumOfActiveConn() == 0u)
    {    
        LED5_Write(LED_ON);
    }
    else
    {
        LED5_Write(LED_OFF);
    }

#endif /* #if(CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV) */    
   
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
    cy_stc_ble_conn_handle_t *appConnHandlePtr    = GetAppConnHandlePtr();
    app_stc_scan_device_list_t *appScanDevInfoPtr = GetAppScanDevInfoPtr();
    
    switch(command)
    {                   
        case 'c': /* Connect to the detected device */  
            if(appScanDevInfoPtr->count != 0u) 
            {
                if(Cy_BLE_GetConnectionState(*appConnHandlePtr) < CY_BLE_CONN_STATE_CONNECTED)
                {
                    uint32_t i;
                    
                    /* Print a list of scanned devices from the peerAddr array */
                    DBG_PRINTF("Detected device: \r\n");
                    for(i = 0u; i < appScanDevInfoPtr->count; i++)
                    {
                       /* Print the devices Addr of the detected Client */
                        DBG_PRINTF("Device %ld: %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x \r\n", i+1,
                                    appScanDevInfoPtr->address[i].bdAddr[5u], appScanDevInfoPtr->address[i].bdAddr[4u], 
                                    appScanDevInfoPtr->address[i].bdAddr[3u], appScanDevInfoPtr->address[i].bdAddr[2u],
                                    appScanDevInfoPtr->address[i].bdAddr[1u], appScanDevInfoPtr->address[i].bdAddr[0u]);         
                    }
                    
                    /* Select a device for connection */  
                    DBG_PRINTF("select device for connection:  (1..%lx):\r\n", appScanDevInfoPtr->count);        
                    while((command = UART_DEB_GET_CHAR()) == UART_DEB_NO_DATA)
                    {
                        appScanDevInfoPtr->pauseScanProgress = true;
                        Cy_BLE_ProcessEvents();   
                    }
                    
                    if((command > '0') && (command <= (appScanDevInfoPtr->count + '0')))
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
                        DBG_PRINTF(" Wrong digit \r\n");
                        break;
                    }
                    
                    appScanDevInfoPtr->pauseScanProgress = false;
                    
                }
                else
                {
                    DBG_PRINTF("You are already connected to peer device \r\n");
                }
            }
            else
            {
                DBG_PRINTF("Connecting list is empty. Press 's' to start scan \r\n");
            }     
            break;
            
        case 'r': /* Remove bonding info */
            /* Set flag and goto the disconect procedure */
            App_SetRemoveBondListFlag(); 
            
        case 'd': /* Disconnect from peer device */
            if(Cy_BLE_GetNumOfActiveConn() != 0u)
            {
                cy_stc_ble_gap_disconnect_info_t disconnectParam =
                {
                    .bdHandle = appConnHandlePtr->bdHandle,
                    .reason = CY_BLE_HCI_ERROR_OTHER_END_TERMINATED_USER
                };
              
                apiResult = Cy_BLE_GAP_Disconnect(&disconnectParam); 
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Disconnect device API Error: %x \r\n", apiResult);
                }
                else
                {
                    DBG_PRINTF("Disconnect device API Success \r\n");
                }
            }
            break;
        
        default:
            DBG_PRINTF("Unsupported command\r\n");
            break;
    }
}

/* [] END OF FILE */
