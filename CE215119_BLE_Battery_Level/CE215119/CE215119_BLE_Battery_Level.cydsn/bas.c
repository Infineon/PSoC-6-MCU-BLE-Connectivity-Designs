/*******************************************************************************
* File Name: bas.c
*
* Version: 1.0
*
* Description:
* This file contains the code for the BAS.
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
#include "bas.h"

/* Static global variables */
static uint8_t batteryLevel = SIM_BATTERY_MIN;

/*******************************************************************************
* Function Name: BasInit
********************************************************************************
*
* Summary:
*   Initializes the battery service.
*
*******************************************************************************/
void BasInit(void)
{
    /* Register service specific callback function */
    Cy_BLE_BAS_RegisterAttrCallback(BasCallBack);

}

/*******************************************************************************
* Function Name: BasCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service-specific events from
*   Battery Service.
*
* Parameters:
*   event      - the event code
*   eventParam - the event parameters
*
********************************************************************************/
void BasCallBack(uint32_t event, void *eventParam)
{
    uint8_t locServiceIndex;

    locServiceIndex = ((cy_stc_ble_bas_char_value_t*)eventParam)->serviceIndex;
    DBG_PRINTF("BAS event: %lx, ", event);

    switch(event)
    {
        case CY_BLE_EVT_BASS_NOTIFICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_BASS_NOTIFICATION_ENABLED %x %x: serviceIndex=%x \r\n",
                       ((cy_stc_ble_bas_char_value_t*)eventParam)->connHandle.attId,
                       ((cy_stc_ble_bas_char_value_t*)eventParam)->connHandle.bdHandle,
                       locServiceIndex);
            break;

        case CY_BLE_EVT_BASS_NOTIFICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_BASS_NOTIFICATION_DISABLED %x %x: serviceIndex=%x \r\n",
                       ((cy_stc_ble_bas_char_value_t*)eventParam)->connHandle.attId,
                       ((cy_stc_ble_bas_char_value_t*)eventParam)->connHandle.bdHandle,
                       locServiceIndex);
            break;

        case CY_BLE_EVT_BASC_NOTIFICATION:
            break;

        case CY_BLE_EVT_BASC_READ_CHAR_RESPONSE:
            break;

        case CY_BLE_EVT_BASC_READ_DESCR_RESPONSE:
            break;

        case CY_BLE_EVT_BASC_WRITE_DESCR_RESPONSE:
            break;

        default:
            DBG_PRINTF("Other event\r\n");
            break;
    }
}


/*******************************************************************************
* Function Name: BasSimulateBattery
********************************************************************************
*
* Summary:
*   The custom function to simulate Battery Voltage.
*
* Parameters:
*  connHandle: The connection handle
*
*******************************************************************************/
void BasSimulateBattery(cy_stc_ble_conn_handle_t connHandle)
{
    static uint32_t batteryTimer = BATTERY_TIMEOUT;
    cy_en_ble_api_result_t apiResult;
    uint16_t cccd;
    
    if(--batteryTimer == 0u)
    {
        batteryTimer = BATTERY_TIMEOUT;

        /* Battery Level simulation */
        batteryLevel += SIM_BATTERY_INCREMENT;
        if(batteryLevel > SIM_BATTERY_MAX)
        {
            batteryLevel = SIM_BATTERY_MIN;
        }

        (void) Cy_BLE_BASS_GetCharacteristicDescriptor(connHandle, CY_BLE_BAS_BATTERY_LEVEL, CY_BLE_BAS_BATTERY_LEVEL, CY_BLE_BAS_BATTERY_LEVEL_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
        
        if(cccd == CY_BLE_CCCD_NOTIFICATION) 
        {
            do
            {
                Cy_BLE_ProcessEvents();
            }
            while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);
            
            if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
            {
                {
                    /* Update Battery Level characteristic value and send Notification */
                    apiResult = Cy_BLE_BASS_SendNotification(connHandle, CY_BLE_BAS_BATTERY_LEVEL, CY_BLE_BAS_BATTERY_LEVEL,
                                                             sizeof(batteryLevel), &batteryLevel);
                    if(apiResult != CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("Cy_BLE_BASS_SendNotification API Error: 0x%x \r\n", apiResult);
                    }
                    
                }
            }
        }    
            
        /* Update Battery Level characteristic value */
        apiResult = Cy_BLE_BASS_SetCharacteristicValue(CY_BLE_BAS_BATTERY_LEVEL, CY_BLE_BAS_BATTERY_LEVEL, 
                                                       sizeof(batteryLevel), &batteryLevel);
        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("Cy_BLE_BASS_SetCharacteristicValue API Error: 0x%x \r\n", apiResult);
        }
        else
        {
            DBG_PRINTF("SimulBatteryLevelUpdate: %d \r\n", batteryLevel);
        }
        
        Cy_BLE_ProcessEvents();
    }
}


/*******************************************************************************
* Function Name: BasGetBatteryLevel
********************************************************************************
*
* Summary:
*   This function returns simulated battery voltage value.
*
* Returns:
*  uint8_t - battery voltage value.
*******************************************************************************/
uint8_t BasGetBatteryLevel(void)
{
    return(batteryLevel);
}


/* [] END OF FILE */
