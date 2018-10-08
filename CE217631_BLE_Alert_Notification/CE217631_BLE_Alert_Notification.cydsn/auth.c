/*******************************************************************************
* File Name: auth.c
*
* Version: 1.0
*
* Description:
*  This file contains Authentication API
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

#include "common.h"
#include "user_interface.h"

/* Flag indicates what IOCAP request came */
static uint32_t appAuthIoCap = CY_BLE_GAP_IOCAP_NOINPUT_NOOUTPUT;

/*
    appAuthIoCap is set from the events of AppCallBack in host_main.c:

                      Event                    |          Value         
    -----------------------------------------------------------------------------
     CY_BLE_EVT_GAP_PASSKEY_ENTRY_REQUEST      | CY_BLE_GAP_IOCAP_KEYBOARD_ONLY
     CY_BLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST    | CY_BLE_GAP_IOCAP_DISPLAY_ONLY
     CY_BLE_EVT_GAP_NUMERIC_COMPARISON_REQUEST | CY_BLE_GAP_IOCAP_DISPLAY_YESNO

*/


/*******************************************************************************
* Function Name: AuthReplay
********************************************************************************
*
* Summary:
*   Provides mechanism to enter PIN when IOCAP request came
*
* Parameters:
*   connHandle - connection handler
*
*******************************************************************************/
void App_AuthReplay(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    char8 command = 0;

    switch(appAuthIoCap)
    {
        /* Platform supports a numeric keyboard that can input the numbers '0' through '9' 
        and a confirmation key(s) for  'yes' and 'no'. */
        case CY_BLE_GAP_IOCAP_KEYBOARD_ONLY:
        {
            uint32_t passkey = 0u;
            uint32_t pow10 = 100000ul;
            uint32_t i;
                
            DBG_PRINTF("Enter 6 digit passkey:\r\n");
            
            /* Clean RX buffer before enter new passkey */
            UART_DEB_SCB_CLEAR_RX_FIFO();
            
            for(i = 0u; i < CY_BLE_GAP_USER_PASSKEY_SIZE; i++)
            {
                while((command = UART_DEB_GET_CHAR()) == UART_DEB_NO_DATA)
                {
                    Cy_BLE_ProcessEvents();
                }
            
                if((command >= '0') && (command <= '9'))
                {
                    passkey += (uint32)(command - '0') * pow10;
                    pow10 /= 10u;
                    DBG_PRINTF("%c\n", command);
                }
                else
                {
                    DBG_PRINTF(" Wrong digit\r\n");
                    break;
                }
            }
        
            if(i == CY_BLE_GAP_USER_PASSKEY_SIZE)
            {
                cy_stc_ble_gap_auth_pk_info_t authPkParam =
                {
                    .bdHandle = connHandle.bdHandle,
                    .passkey  = passkey,
                    .accept   = CY_BLE_GAP_ACCEPT_PASSKEY_REQ
                };
                apiResult = Cy_BLE_GAP_AuthPassKeyReply(&authPkParam);

                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF(" Cy_BLE_GAP_AuthPassKeyReply API Error: ");
                    PrintApiResult(apiResult);
                }
            }
            
            appAuthIoCap = CY_BLE_GAP_IOCAP_NOINPUT_NOOUTPUT;
            break;
        }
        
        /* The device has a mechanism whereby the user can indicate 'yes' or 'no'.*/    
        case CY_BLE_GAP_IOCAP_DISPLAY_YESNO:
        {
            uint8_t var = CY_BLE_GAP_REJECT_PASSKEY_REQ;
            uint8_t sw2Status = 1u;
            
            while(((command = UART_DEB_GET_CHAR()) == UART_DEB_NO_DATA) && (sw2Status = SW2_Read()) == 1u) 
            {
                Cy_BLE_ProcessEvents();
            }
            
            if(sw2Status == 0u)
            {
                command = 'y'; 
            }
            
            switch(command)
            {
                case 'y':
                    DBG_PRINTF(" Accept the displayed passkey\r\n");
                    var = CY_BLE_GAP_ACCEPT_PASSKEY_REQ;   
                    break;
                
                case 'n':
                    DBG_PRINTF(" Reject displayed passkey\r\n");
                    var = CY_BLE_GAP_REJECT_PASSKEY_REQ;
                    break; 
            }
            
            if((command == 'y') || (command == 'n'))
            {
                cy_stc_ble_gap_auth_pk_info_t authPkParam =
                {
                    .bdHandle = connHandle.bdHandle,
                    .passkey  = 0u,
                    .accept   = var
                };   
                apiResult = Cy_BLE_GAP_AuthPassKeyReply(&authPkParam);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GAP_AuthPassKeyReply API Error: 0x%x \r\n", apiResult);
                }
                
                appAuthIoCap = CY_BLE_GAP_IOCAP_NOINPUT_NOOUTPUT;
            }
            break;
        }
        
        case CY_BLE_GAP_IOCAP_NOINPUT_NOOUTPUT:
            break;
        
        /* Platform supports only a mechanism to display or convey only 6 digit number to user.*/
        case CY_BLE_GAP_IOCAP_DISPLAY_ONLY:
            break;
            
        default:
            DBG_PRINTF("Unsupported IO capability  \n\r ");
    }
}


/*******************************************************************************
* Function Name: App_IsAuthReq
********************************************************************************
*
* Summary:
*   Returns true if IOCAP request came
*
* Return:
*   true  - IOCAP flag is set
*   false - IOCAP flag is clear
*
*******************************************************************************/
bool App_IsAuthReq(void)
{
    return((appAuthIoCap == CY_BLE_GAP_IOCAP_NOINPUT_NOOUTPUT) ? false : true );   
}


/*******************************************************************************
* Function Name: App_SetAuthIoCap
********************************************************************************
*
* Summary:
*   Set appAuthIoCap flag when IOCAP request came
*
* Parameters:
*   value - new value of appAuthIoCap flag.
*
*******************************************************************************/
void App_SetAuthIoCap(uint32_t value)
{
    appAuthIoCap = value;   
}


/*******************************************************************************
* Function Name: ShowAuthError
********************************************************************************
*
* Summary:
*   Displays the Authentication Error
*
* Parameters:
*   authErr - Authentication Failed Error Codes
*
*******************************************************************************/
void App_ShowAuthError(cy_en_ble_gap_auth_failed_reason_t authErr)
{
    switch(authErr)
    {
        case CY_BLE_GAP_AUTH_ERROR_NONE:
            DBG_PRINTF("NONE\r\n");
            break;

        case CY_BLE_GAP_AUTH_ERROR_PASSKEY_ENTRY_FAILED:
            DBG_PRINTF("PASSKEY_ENTRY_FAILED\r\n");
            break;

        case CY_BLE_GAP_AUTH_ERROR_OOB_DATA_NOT_AVAILABLE:
            DBG_PRINTF("OOB_DATA_NOT_AVAILABLE\r\n");
            break;

        case CY_BLE_GAP_AUTH_ERROR_AUTHENTICATION_REQ_NOT_MET:
            DBG_PRINTF("AUTHENTICATION_REQ_NOT_MET\r\n");
            break;

        case CY_BLE_GAP_AUTH_ERROR_CONFIRM_VALUE_NOT_MATCH:
            DBG_PRINTF("CONFIRM_VALUE_NOT_MATCH\r\n");
            break;

        case CY_BLE_GAP_AUTH_ERROR_PAIRING_NOT_SUPPORTED:
            DBG_PRINTF("PAIRING_NOT_SUPPORTED\r\n");
            break;

        case CY_BLE_GAP_AUTH_ERROR_INSUFFICIENT_ENCRYPTION_KEY_SIZE:
            DBG_PRINTF("INSUFFICIENT_ENCRYPTION_KEY_SIZE\r\n");
            break;

        case CY_BLE_GAP_AUTH_ERROR_COMMAND_NOT_SUPPORTED:
            DBG_PRINTF("COMMAND_NOT_SUPPORTED\r\n");
            break;

        case CY_BLE_GAP_AUTH_ERROR_UNSPECIFIED_REASON:
            DBG_PRINTF("UNSPECIFIED_REASON\r\n");
            break;

        case CY_BLE_GAP_AUTH_ERROR_REPEATED_ATTEMPTS:
            DBG_PRINTF("REPEATED_ATTEMPTS\r\n");
            break;

        case CY_BLE_GAP_AUTH_ERROR_INVALID_PARAMETERS:
            DBG_PRINTF("INVALID_PARAMETERS\r\n");
            break;

        case CY_BLE_GAP_AUTH_ERROR_AUTHENTICATION_TIMEOUT:
            DBG_PRINTF("AUTHENTICATION_TIMEOUT\r\n");
            break;
          
        case CY_BLE_GAP_AUTH_ERROR_LINK_DISCONNECTED:
            DBG_PRINTF("LINK_DISCONNECTED\r\n");
            break;
  
        default:
            DBG_PRINTF("Unknown error - 0x%x\r\n",authErr);
            break;
    }
}

/* [] END OF FILE */
