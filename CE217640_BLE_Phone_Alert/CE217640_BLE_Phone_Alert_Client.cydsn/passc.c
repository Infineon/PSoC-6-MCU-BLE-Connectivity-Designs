/*******************************************************************************
* File Name: passc.c
*
* Version 1.0
*
* Description:
*  This file contains Phone Alert Status service related code.
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

#include <project.h>
#include "passc.h"

/*******************************************************************************
* Global variables
*******************************************************************************/
static uint8_t passFlag;
static uint16_t dscr;
static cy_en_ble_pass_char_index_t charIndex;
static uint8_t alertStatus = 0u;
static cy_en_ble_pass_rs_t ringerSetting = CY_BLE_PASS_RS_SILENT;
static cy_en_ble_pass_cp_t controlPoint = CY_BLE_PASS_CP_SILENT;


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
*
********************************************************************************/
void PassCallBack(uint32_t event, void* eventParam)
{   
    switch(event)
    {
        case CY_BLE_EVT_PASSC_NOTIFICATION:
            if(((cy_stc_ble_pass_char_value_t*)eventParam)->charIndex == CY_BLE_PASS_AS)
            {
                alertStatus = ((cy_stc_ble_pass_char_value_t*)eventParam)->value->val[0];
                DBG_PRINTF("Alert Status");
            }
            else
            {
                ringerSetting = (cy_en_ble_pass_rs_t)((cy_stc_ble_pass_char_value_t*)eventParam)->value->val[0];
                DBG_PRINTF("Ringer Setting");
            }
  
            DBG_PRINTF(" notification: %x -", ((cy_stc_ble_pass_char_value_t*)eventParam)->value->val[0]);
            if(((cy_stc_ble_pass_char_value_t*)eventParam)->charIndex == CY_BLE_PASS_AS)
            {
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
            else
            {   switch(ringerSetting)
                {
                case CY_BLE_PASS_RS_SILENT:
                    DBG_PRINTF(" SILENT\r\n");
                    break;
                
                case CY_BLE_PASS_RS_NORMAL:
                    DBG_PRINTF(" NORMAL\r\n");
                    break;
            
                default:  
                    break;
                }
            }
            break;
        
        case CY_BLE_EVT_PASSC_READ_CHAR_RESPONSE:
            if(((cy_stc_ble_pass_char_value_t*)eventParam)->charIndex == CY_BLE_PASS_AS)
            {
                alertStatus = ((cy_stc_ble_pass_char_value_t*)eventParam)->value->val[0];
                passFlag |= FLAG_RD;
                charIndex = CY_BLE_PASS_RS;
                DBG_PRINTF("Alert Status");
            }
            else
            {
                ringerSetting = (cy_en_ble_pass_rs_t)((cy_stc_ble_pass_char_value_t*)eventParam)->value->val[0];
                passFlag |= FLAG_DSCR;
                dscr = CY_BLE_CCCD_NOTIFICATION;
                charIndex = CY_BLE_PASS_AS;
                DBG_PRINTF("Ringer Setting");
            }
                        
            DBG_PRINTF(" read response: %2.2x \r\n", ((cy_stc_ble_pass_char_value_t*)eventParam)->value->val[0]);
            break;
            
        case CY_BLE_EVT_PASSC_READ_DESCR_RESPONSE:
            if(((cy_stc_ble_pass_char_value_t*)eventParam)->charIndex == CY_BLE_PASS_AS)
            {
                DBG_PRINTF("Alert Status");
            }
            else
            {
                DBG_PRINTF("Ringer Setting");
            }
            DBG_PRINTF(" read descriptor response: ");
            DBG_PRINTF("%x \r\n", ((cy_stc_ble_pass_char_value_t*)eventParam)->value->val[0]);
            break;
            
        case CY_BLE_EVT_PASSC_WRITE_DESCR_RESPONSE:
            if(((cy_stc_ble_pass_char_value_t*)eventParam)->charIndex == CY_BLE_PASS_AS)
            {
                passFlag |= FLAG_DSCR;
                dscr = CY_BLE_CCCD_NOTIFICATION;
                charIndex = CY_BLE_PASS_RS;
                DBG_PRINTF("Alert Status");
            }
            else
            {
                DBG_PRINTF("Ringer Setting");
            }
            DBG_PRINTF(" write descriptor response \r\n");
            break;

        default:
            DBG_PRINTF("Unknown PASS event: %lx \r\n", event);
            break;
    }
}


/******************************************************************************
* Function Name: PassInit
*******************************************************************************
*
* Summary:
*    Registers the PASS CallBack.
*
******************************************************************************/
void PassInit(void)
{
    Cy_BLE_PASS_RegisterAttrCallback(PassCallBack);
    passFlag = 0u;
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
    cy_en_ble_api_result_t apiResult;
    
    if((passFlag & FLAG_RD) != 0u)
    {
        passFlag &= (uint8) ~FLAG_RD;
        
        do
        {
            Cy_BLE_ProcessEvents();
        }
        while(Cy_BLE_GATT_GetBusyStatus(appConnHandle.attId) == CY_BLE_STACK_STATE_BUSY);
        
        apiResult = Cy_BLE_PASSC_GetCharacteristicValue(appConnHandle, charIndex);
        
        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("Cy_BLE_PASSC_GetCharacteristicValue API Error: ");
            PrintApiResult(apiResult);
        }
        else
        {
            if(charIndex == CY_BLE_PASS_AS)
            {
                DBG_PRINTF("Alert Status ");
            }
            else
            {
                DBG_PRINTF("Ringer Setting ");
            }
            
            DBG_PRINTF("characteristic Read Request is sent \r\n");
        }
    }
 
    if((passFlag & FLAG_DSCR) != 0u)
    {
        passFlag &= (uint8) ~FLAG_DSCR;
        
        do
        {
            Cy_BLE_ProcessEvents();
        }
        while(Cy_BLE_GATT_GetBusyStatus(appConnHandle.attId) == CY_BLE_STACK_STATE_BUSY);
            
        apiResult = Cy_BLE_PASSC_SetCharacteristicDescriptor(appConnHandle, charIndex, CY_BLE_PASS_CCCD, CY_BLE_CCCD_LEN, (uint8*)&dscr);
        
        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("Cy_BLE_PASSC_SetCharacteristicDescriptor API Error: ");
            PrintApiResult(apiResult);
        }
        else
        {
            if(charIndex == CY_BLE_PASS_AS )
            {
                DBG_PRINTF("Alert Status notification");
            }
            else
            {
                DBG_PRINTF("Ringer Setting notification");
            }
            
            if(dscr == CY_BLE_CCCD_NOTIFICATION)
            {
                DBG_PRINTF(" enable request is sent \r\n");
            }
            else
            {
                DBG_PRINTF(" disable request is sent \r\n");
            }
        }   
    }
    
    if((passFlag & FLAG_WR) != 0u)
    {
        passFlag &= (uint8) ~FLAG_WR;
         
        do
        {
            Cy_BLE_ProcessEvents();
        }
        while(Cy_BLE_GATT_GetBusyStatus(appConnHandle.attId) == CY_BLE_STACK_STATE_BUSY);
        
        apiResult = Cy_BLE_PASSC_SetCharacteristicValue(appConnHandle, CY_BLE_PASS_CP, 1u, (uint8*)&controlPoint);
        
        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("Cy_BLE_PASSC_SetCharacteristicValue API Error: ");
            PrintApiResult(apiResult);
        }
        else
        {
            DBG_PRINTF("Control Point write request is sent: ");
            switch(controlPoint)
            {
                case CY_BLE_PASS_CP_SILENT:
                    DBG_PRINTF("Silent Mode\r\n");
                    break;
                
                case CY_BLE_PASS_CP_MUTE:
                    DBG_PRINTF("Mute Once\r\n");
                    break;
                
                case CY_BLE_PASS_CP_CANCEL:
                    DBG_PRINTF("Normal Mode\r\n");
                    break;
                
                default:
                    break;
            }
        }
    }
}

/*******************************************************************************
* Function Name: PassGetRingerSetting
********************************************************************************
*
* Summary:
*   This function returns Ringer Setting value.
*
* Returns:
*  cy_en_ble_pass_rs_t - Ringer Setting value.
*
*******************************************************************************/
cy_en_ble_pass_rs_t PassGetRingerSetting(void)
{
    return ringerSetting;
}


/*******************************************************************************
* Function Name: PassSetControlPoint
********************************************************************************
*
* Summary:
*   This function sets Control Point value.
*
* Parameters:  
*   controlPointValue - new value of Control Point.
*
*******************************************************************************/
void PassSetControlPoint(cy_en_ble_pass_cp_t controlPointValue)
{
    controlPoint = controlPointValue;
}


/*******************************************************************************
* Function Name: PassGetAlertStatus
********************************************************************************
*
* Summary:
*   This function returns Alert Status value.
*
* Returns:
*  uint8_t - Alert Status value.
*
*******************************************************************************/
uint8_t PassGetAlertStatus(void)
{
    return alertStatus;
}


/*******************************************************************************
* Function Name: PassSetCharIndex
********************************************************************************
*
* Summary:
*   This function set charIndex value.
*
* Parameters:  
*   charIndexValue - new value of charIndex.
*
*******************************************************************************/
void PassSetCharIndex(cy_en_ble_pass_char_index_t charIndexValue)
{
   charIndex = charIndexValue;
}

/*******************************************************************************
* Function Name: PassGetPassFlag
********************************************************************************
*
* Summary:
*   This function returns passFlag value.
*
* Returns:
*  uint8_t - passFlag value.
*
*******************************************************************************/
uint8_t PassGetPassFlag(void)
{
    return passFlag;
}


/*******************************************************************************
* Function Name: PassSetPassFlag
********************************************************************************
*
* Summary:
*   This function set passFlag value.
*
* Parameters:  
*   uint8_t - new value of passFlag.
*
*******************************************************************************/
void PassSetPassFlag(uint8_t passFlagValue)
{
   passFlag = passFlagValue;
}


/* [] END OF FILE */
