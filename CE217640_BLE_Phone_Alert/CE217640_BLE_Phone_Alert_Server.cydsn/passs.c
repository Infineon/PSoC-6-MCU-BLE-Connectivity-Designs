/*******************************************************************************
* File Name: passs.c
*
* Version: 1.0
*
* Description:
*  This is source code for the Phone Alert Status Server example project.
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


#include "passs.h"

uint8_t alertStatus = 0u;
uint8_t passFlag    = 0u;
cy_en_ble_pass_rs_t ringerSetting = CY_BLE_PASS_RS_SILENT;
cy_en_ble_pass_cp_t controlPoint  = CY_BLE_PASS_CP_SILENT;


/*******************************************************************************
* Function Name: PassCallBack()
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service specific events from
*   Phone Alert Status service.
*
* Parameters:
*  event       - the event code
*  *eventParam - the event parameters
*
********************************************************************************/
void PassCallBack(uint32_t event, void* eventParam)
{
    if(eventParam != 0u)
    {
        /* This dummy operation is to avoid warning about unused eventParam */   
    }
    
    switch(event)
    {
        case CY_BLE_EVT_PASSS_NOTIFICATION_ENABLED:
            if(((cy_stc_ble_pass_char_value_t *)eventParam)->charIndex == CY_BLE_PASS_AS)
            {
                DBG_PRINTF("Alert Status Notification is Enabled \r\n");
            }
            else
            {
                DBG_PRINTF("Ringer Setting Notification is Enabled \r\n");
            }
            break;
                
        case CY_BLE_EVT_PASSS_NOTIFICATION_DISABLED:
            if(((cy_stc_ble_pass_char_value_t *)eventParam)->charIndex == CY_BLE_PASS_AS)
            {
                DBG_PRINTF("Alert Status Notification is Disabled \r\n");
            }
            else
            {
                DBG_PRINTF("Ringer Setting Notification is Disabled \r\n");
            }
            break;
            
        case CY_BLE_EVT_PASSS_WRITE_CHAR:
            controlPoint = (cy_en_ble_pass_cp_t)((cy_stc_ble_pass_char_value_t *)eventParam)->value->val[0];
            DBG_PRINTF("CP is written: %x \r\n", controlPoint);
            switch(controlPoint)
            {
                case CY_BLE_PASS_CP_SILENT:
                    ringerSetting = CY_BLE_PASS_RS_SILENT;
                    DBG_PRINTF("Ringer Setting: Silent \r\n");
                    passFlag |= FLAG_RS;
                    break;
                
                case CY_BLE_PASS_CP_MUTE:
                    if((alertStatus & CY_BLE_PASS_AS_RINGER) != 0u)
                    {
                        alertStatus &= ~CY_BLE_PASS_AS_RINGER;
                        DBG_PRINTF("Ringer Setting: Silent \r\n");
                        passFlag |= FLAG_AS;
                    }
                    else
                    {
                        DBG_PRINTF("Ringer is not active so 'Mute Once' command has no effect \r\n");
                    }
                    break;
                
                case CY_BLE_PASS_CP_CANCEL:
                    ringerSetting = CY_BLE_PASS_RS_NORMAL;
                    DBG_PRINTF("Ringer Setting: Cancel Silent Mode \r\n");
                    passFlag |= FLAG_RS;
                    break;
                
                default:
                    break;
            }
            break;
       
        default:
            DBG_PRINTF("Unknown PASS event: %lx \r\n", event);
            break;
    }
}

/*******************************************************************************
* Function Name: PassInit()
********************************************************************************
*
* Summary:
*   Initializes the Phone Alert Status Service.
*
*******************************************************************************/
void PassInit(void)
{
    /* Register the event handler for PASS specific events */
    Cy_BLE_PASS_RegisterAttrCallback(PassCallBack);
}


/*******************************************************************************
* Function Name: SetAS();
********************************************************************************
*
* Summary:
*   Sends notification for Alert Status characteristic.
*
*******************************************************************************/
void SetAS(void)
{
    cy_en_ble_api_result_t apiResult;
    
    apiResult = Cy_BLE_PASSS_SetCharacteristicValue(CY_BLE_PASS_AS, 1u, &alertStatus);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_PASSS_SetCharacteristicValue API Error: ");
        PrintApiResult(apiResult);
    }
    else
    {
        do
        {
            Cy_BLE_ProcessEvents();
        }
        while(Cy_BLE_GATT_GetBusyStatus(appConnHandle.attId) == CY_BLE_STACK_STATE_BUSY);
    
        if(Cy_BLE_GetConnectionState(appConnHandle) >= CY_BLE_CONN_STATE_CONNECTED)
        {
            apiResult = Cy_BLE_PASSS_SendNotification(appConnHandle, CY_BLE_PASS_AS, 1u, &alertStatus);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_PASSS_SendNotification API Error: ");
                PrintApiResult(apiResult);
            }
            else
            {
                DBG_PRINTF("Alert Status characteristic is notified: %x -", alertStatus);
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
            }
        }
    }
}

/*******************************************************************************
* Function Name: SetRS();
********************************************************************************
*
* Summary:
*   Sends notification for Ringer Setting characteristic.
*
*******************************************************************************/
void SetRS(void)
{
    cy_en_ble_api_result_t apiResult;
    apiResult = Cy_BLE_PASSS_SetCharacteristicValue(CY_BLE_PASS_RS, 1u, (uint8*)&ringerSetting);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_PASSS_SetCharacteristicValue API Error: ");
        PrintApiResult(apiResult);
    }
    else
    {
        do
        {
            Cy_BLE_ProcessEvents();
        }
        while(Cy_BLE_GATT_GetBusyStatus(appConnHandle.attId) == CY_BLE_STACK_STATE_BUSY);
        
        if(Cy_BLE_GetConnectionState(appConnHandle) >= CY_BLE_CONN_STATE_CONNECTED)
        {
            apiResult = Cy_BLE_PASSS_SendNotification(appConnHandle, CY_BLE_PASS_RS, 1u, (uint8*)&ringerSetting);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_PASSS_SendNotification API Error: ");
                PrintApiResult(apiResult);
            }
            else
            {
                DBG_PRINTF("Ringer Setting characteristic is notified: %x - %s\r\n", ringerSetting,
                    ((ringerSetting == CY_BLE_PASS_RS_SILENT) ? "SILENT" : "NORMAL"));
            }
        }
    }
}

/*******************************************************************************
* Function Name: PassProcess
********************************************************************************
*
* Summary:
*   Handles of PASS Process.
*
*******************************************************************************/
void PassProcess(void)
{
    if((passFlag & FLAG_RS) != 0u)
    {
        passFlag &= ~FLAG_RS;
        
        SetRS();
    }
    
    if((passFlag & FLAG_AS) != 0u)
    {
        passFlag &= ~FLAG_AS;
        
        SetAS();
    }
}

/* [] END OF FILE */
