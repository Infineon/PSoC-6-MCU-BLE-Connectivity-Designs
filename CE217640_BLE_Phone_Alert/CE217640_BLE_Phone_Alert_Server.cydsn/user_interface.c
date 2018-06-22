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
#include "passs.h"


static uint32_t   flag;
static uint8_t    button = 0u;
static uint32_t   buttonSW2Timer = 0u;


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
      
    /* Initialize SW2 interrupt */
    Cy_SysInt_Init(&SW2_Int_cfg, SW2_Int);
    NVIC_EnableIRQ(SW2_Int_cfg.intrSrc);   
    SW2_EnableInt();
    
    /* Configure the SysTick timer to generate interrupt every 1 ms
    * and start its operation.
    */
    Cy_SysTick_Init(CY_SYSTICK_CLOCK_SOURCE_CLK_LF, 32u); 
    Cy_SysTick_SetCallback(0u, Timer_Interrupt); 
}


/*******************************************************************************
* Function Name: UpdateLedState
********************************************************************************
*
* Summary:
*  Handles indications LEDs.
*
*******************************************************************************/
void UpdateLedState(void)
{
#if(CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV)
    if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_SCANNING)
    {
        /* In scanning state, turn off disconnect and connect indication LEDs */
        Disconnect_LED_Write(LED_OFF);
        Connect_LED_Write(LED_OFF);
        
        /* Blink advertising indication LED */
        Scanning_LED_INV();
    }
    else if(Cy_BLE_GetConnectionState(appConnHandle) == CY_BLE_CONN_STATE_DISCONNECTED)
    {
        /* If in disconnected state, turn on disconnect indication LED and turn
        * off scanning and connect LEDs.
        */
        Disconnect_LED_Write(LED_ON);
        Connect_LED_Write(LED_OFF);
        Scanning_LED_Write(LED_OFF);  
    }
    else 
    {
        /* In connected state, turn off disconnect indication and advertising 
        * indication LEDs. 
        */
        
        Disconnect_LED_Write(LED_OFF);
        Connect_LED_Write(LED_ON);
        Scanning_LED_Write(LED_OFF);       
    }
#else
    if(Cy_BLE_GetScanState() == CY_BLE_SCAN_STATE_SCANNING)
    {
        /* Blink advertising indication LED */
        LED5_INV();
    }
    else if(Cy_BLE_GetConnectionState(appConnHandle) == CY_BLE_CONN_STATE_DISCONNECTED)
    {
        /* If in disconnected state, turn on disconnect indication LED */
        LED5_Write(LED_ON);
    }
    else 
    {
        /* In connected state, turn off disconnect indication LED */
        LED5_Write(LED_OFF);
    }
#endif /* #if(CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV) */    
}


/*******************************************************************************
* Function Name: Timer_Interrupt
********************************************************************************
*
* Summary:
*  Handles the Interrupt Service Routine for the sys tick timer.
*
*******************************************************************************/
void Timer_Interrupt(void)
{
    /* Increment timer for recognition 1 press / 2 presses per second */
    if(buttonSW2Timer != TIMER_STOP)
    {    
        buttonSW2Timer ++;
        if(buttonSW2Timer > SW2_POLLING_TIME)
        {
            /* Set flag for button polling */
            flag |= SW2_READY;
            buttonSW2Timer = TIMER_STOP;
        }        
    }
}


/*******************************************************************************
* Function Name: SW2_Int
********************************************************************************
*
* Summary:
*   Handles the SW2 button press.
*
*******************************************************************************/
void SW2_Int(void)
{
    if (Cy_GPIO_GetInterruptStatusMasked(SW2_0_PORT, SW2_0_NUM) == 1u)
    { 
        if(buttonSW2Timer == TIMER_STOP)
        {   
            button = SW2_ONE_PRESSING;  
            buttonSW2Timer = TIMER_START;
        } 
        if (buttonSW2Timer > SW2_DEBOUNCING_TIME)
        {   
            button = SW2_TWO_PRESSING;           
        }
        SW2_ClearInt();
    }   
}
/*******************************************************************************
* Function Name: SW2_buttonPolling
********************************************************************************
*
* Summary:
*   Polling the SW2 button state.
*
*******************************************************************************/
char SW2_buttonPolling(void)
{
    char command = 0u;
    
    if((flag & SW2_READY) != 0u)
    {
        switch(button)
        {
            case SW2_ONE_PRESSING:
                command = 'a';
                break;
            
            case SW2_TWO_PRESSING:
                command = 'z';
                break;
            
            default:
                break;        
        }
        flag &= ~SW2_READY;
    }
    return command;
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

    DBG_PRINTF("command: %c \r\n", command);
    switch(command)
    {
        case 'a':                   /* alert status notify */
            alertStatus++;
            alertStatus &= 0x03;
            DBG_PRINTF("Alert Status characteristic: %x -", alertStatus);
            switch(alertStatus)
            {
                case ALERT_STATUS_NO_ALERT:
                    DBG_PRINTF(" NO ALERT\r\n");
                    break;
                
                case ALERT_STATUS_RINGER:
                    DBG_PRINTF(" RINGER\r\n");
                    break;

                case ALERT_STATUS_VIBRATE:
                    DBG_PRINTF(" VIBRATE\r\n");
                    break;
                
                case ALERT_STATUS_VIBRATE_RINGER:
                    DBG_PRINTF(" VIBRATE + RINGER\r\n");
                    break;
                    
                default:  
                    break;
            }
            SetAS();
            break;
                                
        case 'z':/* Ringer setting */
            switch(ringerSetting)
            {
                case CY_BLE_PASS_RS_SILENT:
                    ringerSetting = CY_BLE_PASS_RS_NORMAL;
                    break;
                
                default:
                    ringerSetting = CY_BLE_PASS_RS_SILENT;
                    break;
            }
            
            SetRS();
            break;
            
        case 'c': /* Connect to the peer device */
        {
            if(peerCnt != 0u) 
            {
                if(Cy_BLE_GetConnectionState(appConnHandle) < CY_BLE_CONN_STATE_CONNECTED)
                {
                    uint32_t i;
                    
                    /* Print a list of scanned devices from the peerAddr array */
                    DBG_PRINTF("Detected device: \r\n");
                    for(i = 0u; i < peerCnt; i++)
                    {
                       /* Print the devices Addr of the detected Client */
                        DBG_PRINTF("Device %ld: %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x \r\n", i+1,
                                        peerAddr[i].bdAddr[5u], peerAddr[i].bdAddr[4u], 
                                        peerAddr[i].bdAddr[3u], peerAddr[i].bdAddr[2u],
                                        peerAddr[i].bdAddr[1u], peerAddr[i].bdAddr[0u]);         
                    }
                    
                    /* Select a device for connection */  
                    DBG_PRINTF("select device for connection:  (1..%lx):\r\n", peerCnt);        
                    while((command = UART_DEB_GET_CHAR()) == UART_DEB_NO_DATA)
                    {
                        Cy_BLE_ProcessEvents();
                    }
                    
                    if((command > '0') && (command <= (peerCnt + '0')))
                    {
                        apiResult = Cy_BLE_GAPC_ConnectDevice(&peerAddr[(uint8_t)(command - '0') - 1], CY_BLE_CENTRAL_CONFIGURATION_0_INDEX);
                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("ConnectDevice API Error: 0x%x ", apiResult);
                        }
                        else
                        {
                            DBG_PRINTF("Connecting to the device ");
                            /* Print the devices Addr of the detected Client */
                            DBG_PRINTF(": %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x \r\n",
                                            peerAddr[(uint8_t)(command - '0') - 1].bdAddr[5u], 
                                            peerAddr[(uint8_t)(command - '0') - 1].bdAddr[4u], 
                                            peerAddr[(uint8_t)(command - '0') - 1].bdAddr[3u],
                                            peerAddr[(uint8_t)(command - '0') - 1].bdAddr[2u],
                                            peerAddr[(uint8_t)(command - '0') - 1].bdAddr[1u],
                                            peerAddr[(uint8_t)(command - '0') - 1].bdAddr[0u]); 
                        } 
                    }
                    else
                    {
                        DBG_PRINTF(" Wrong digit \r\n");
                        break;
                    }
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
        }
        break;
            
        case 'r': /* Remove bonding info */
            App_SetRemoveBondListFlag();  /* Set flag and goto the disconect procedure */
            
        case 'd': /* Disconnect from peer device */
        {
            if(Cy_BLE_GetConnectionState(appConnHandle) >= CY_BLE_CONN_STATE_CONNECTED)
            {
                cy_stc_ble_gap_disconnect_info_t disconnectParam =
                {
                    .bdHandle = appConnHandle.bdHandle,
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
            else
            {
                DBG_PRINTF("Device is disconnected \r\n");          
            }
        }
        break;
        
        case 'p':
            if(Cy_BLE_GetConnectionState(appConnHandle) >= CY_BLE_CONN_STATE_CONNECTED)
            {
                /* Send an authorization request */
                cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX].bdHandle = 
                    appConnHandle.bdHandle;   
                apiResult = Cy_BLE_GAP_AuthReq(&cy_ble_configPtr->authInfo[CY_BLE_SECURITY_CONFIGURATION_0_INDEX]);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAP_AuthReq API Error:  ");
                    PrintApiResult(apiResult);
                }
            }
            else
            {
                DBG_PRINTF("Please connect to peer device before pairing \r\n");       
            }
            break;
            
        case 'h':  /* Information about available commands */
            DisplayHelpInfo();   
            break;
        
        default:
            break;
    }    
}


/*******************************************************************************
* Function Name: DisplayHelpInfo();
********************************************************************************
*
* Summary:
*   Prints the list of terminal comands used in this code example.
*
*******************************************************************************/
void DisplayHelpInfo(void)
{
    DBG_PRINTF("\r\nPASS specified commands:\r\n");
    DBG_PRINTF(" a - Alert Status Notification\r\n");
    DBG_PRINTF(" z - Ringer Setting Notification\r\n");
    
    DBG_PRINTF("\r\nGeneral commands :\r\n");
    DBG_PRINTF(" h - Help screen             \r\n");
    DBG_PRINTF(" s - Start scanning          c - Initiate connection\r\n");
    DBG_PRINTF(" d - Disconnection           r - Unbounding all devices\r\n");
    DBG_PRINTF(" p - Pair  \r\n");
}

/* [] END OF FILE */
