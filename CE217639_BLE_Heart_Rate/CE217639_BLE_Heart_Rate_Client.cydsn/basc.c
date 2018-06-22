/*******************************************************************************
* File Name: basc.c
*
* Version 1.0
*
* Description:
*  This file contains BAS callback handler function.
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

#include "common.h"
#include "basc.h"

uint16_t basNotification = 0u;

/*******************************************************************************
* Function Name: BasInit()
********************************************************************************
*
* Summary:
*   Initializes the battery service.
*
*******************************************************************************/
void BasInit(void)
{
    Cy_BLE_BAS_RegisterAttrCallback(BasCallBack);
}


/*******************************************************************************
* Function Name: BasCallBack()
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service specific events from 
*   Battery Service.
*
* Parameters:
*  event - the event code
*  *eventParam - the event parameters
*
* Return:
*  None.
*
*******************************************************************************/
void BasCallBack(uint32_t event, void* eventParam)
{
    uint8_t batteryLevel;
    
    DBG_PRINTF("BAS event: %lx, ", event);

    switch(event)
    {
        case CY_BLE_EVT_BASC_WRITE_DESCR_RESPONSE:
            if((GetAppState() & APP_STATE_ENABLING_CCCD) != 0u)
            {
                DBG_PRINTF("Battery Level Notification is Enabled  \r\n");
                basNotification = CY_BLE_CCCD_NOTIFICATION;
                
            }
            else
            {
                DBG_PRINTF("Battery Level Notification is Disabled  \r\n");
                basNotification = CY_BLE_CCCD_DEFAULT;
            }
       
            SetAppState(GetAppState() | APP_STATE_OPERATION_COMPLETE);
            SetAppState(GetAppState() & ~APP_STATE_WAITING_RESPONSE);

            break;

        case CY_BLE_EVT_BASC_NOTIFICATION:
            batteryLevel = *((cy_stc_ble_bas_char_value_t*)eventParam)->value->val;
            (void)batteryLevel;
            DBG_PRINTF("Battery Level Notification: %d  \r\n", batteryLevel);       
            break;

        case CY_BLE_EVT_BASC_READ_CHAR_RESPONSE:
            DBG_PRINTF("BAS CHAR Read Response: %d  \r\n", *((cy_stc_ble_bas_char_value_t*)eventParam)->value->val);
            break;

        case CY_BLE_EVT_BASC_READ_DESCR_RESPONSE:
            DBG_PRINTF("BAS descriptor read rsp: %4.4x  \r\n", 
                          Cy_BLE_Get16ByPtr(((cy_stc_ble_bas_descr_value_t *)eventParam)->value->val));
            
            basNotification = Cy_BLE_Get16ByPtr(((cy_stc_ble_bas_descr_value_t *)eventParam)->value->val);
            
            SetAppState(GetAppState() | APP_STATE_OPERATION_COMPLETE);
            SetAppState(GetAppState() & ~APP_STATE_WAITING_RESPONSE);
            
            break;

        default:
            DBG_PRINTF("Not supported event\r\n");
            break;
    }
}


/* [] END OF FILE */
