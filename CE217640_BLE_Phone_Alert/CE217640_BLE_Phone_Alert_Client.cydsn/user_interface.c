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
#include "passc.h"


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
    if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
        {
            /* In advertising state, turn off disconnect indication LED */
            Disconnect_LED_Write(LED_OFF);
            Simulation_LED_Write(LED_OFF);
            /* Blink scaning indication LED */
            Advertising_LED_INV();
        }
        else if(Cy_BLE_GetNumOfActiveConn() == 0u)
        {
            /* If in disconnected state, turn on disconnect indication LED and turn
            * off advertising LED.
            */
            Disconnect_LED_Write(LED_ON);
            Simulation_LED_Write(LED_OFF);
            Advertising_LED_Write(LED_OFF);
        }
        else
        {
            /* In connected state, turn off disconnect indication and advertising 
            * indication LEDs. 
            */
            Disconnect_LED_Write(LED_OFF);
            
            if(PassGetAlertStatus() == 0u)
            {
                if(PassGetRingerSetting() == CY_BLE_PASS_RS_NORMAL)
                {
                    Simulation_LED_Write(LED_ON);  /* On Blue LED */
                    Advertising_LED_Write(LED_OFF);
                }
                else /* silent */
                {
                    Simulation_LED_Write(LED_OFF); 
                    Advertising_LED_Write(LED_ON);  /* On Green LED */
                }
            }
                
            if(((PassGetAlertStatus() & CY_BLE_PASS_AS_VIBRATE) != 0u) || 
               ((PassGetAlertStatus() & (CY_BLE_PASS_AS_RINGER | CY_BLE_PASS_AS_VIBRATE)) == 
                                        (CY_BLE_PASS_AS_RINGER | CY_BLE_PASS_AS_VIBRATE)))
            {
                Simulation_LED_INV(); /* Blink Blue LED */
                Advertising_LED_Write(LED_OFF);
            }
            else if((PassGetAlertStatus() & CY_BLE_PASS_AS_RINGER) != 0u)
            {
                Advertising_LED_INV(); /* Blink Green LED */
                Simulation_LED_Write(LED_OFF);
            }
        }             
#else
    if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
    {
        /* Blink advertising indication LED */
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
        if((PassGetAlertStatus() & CY_BLE_PASS_AS_RINGER) != 0u)
        {
            PassSetControlPoint(CY_BLE_PASS_CP_MUTE);
            PassSetPassFlag(PassGetPassFlag() | FLAG_WR);
            
        }
        else if((PassGetAlertStatus() & CY_BLE_PASS_AS_VIBRATE) != 0u)
        {
            if(PassGetRingerSetting() == CY_BLE_PASS_RS_NORMAL)
            {
                PassSetControlPoint(CY_BLE_PASS_CP_SILENT);
            }
            else /* silent */
            {
                PassSetControlPoint(CY_BLE_PASS_CP_CANCEL);
            }
            PassSetPassFlag(PassGetPassFlag() | FLAG_WR);
        }
        SW2_ClearInt();
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

    DBG_PRINTF("command: %c \r\n", command);
    
    switch(command)
    {  
        case 'r': /* Remove bonding info */
            App_SetRemoveBondListFlag(); /* Set flag and goto the disconect procedure */
            
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
    DBG_PRINTF("\r\nGeneral commands :\r\n");
    DBG_PRINTF(" h - Help screen            d - Disconnection\r\n");
    DBG_PRINTF(" p - Pair                   r - Unbounding all devices \r\n");
    DBG_PRINTF("\r\n");
}

/* [] END OF FILE */
