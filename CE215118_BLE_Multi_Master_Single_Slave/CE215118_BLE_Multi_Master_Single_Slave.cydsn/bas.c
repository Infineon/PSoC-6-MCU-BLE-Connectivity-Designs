/*******************************************************************************
* File Name: bas.c
*
* Version: 1.0
*
* Description:
*  This file contains the code for the BAS.
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

#include "bas.h"

   
uint16_t batterySimulationNotify = DISABLED;


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

    /* Read CCCD configurations from flash */
    batterySimulationNotify = DISABLED;

#if ((BAS_SIMULATE_ENABLE != 0) && (CY_BLE_BONDING_REQUIREMENT == CY_BLE_BONDING_YES))
    {
        cy_en_ble_api_result_t apiResult;
        uint16_t cccdValue;
        uint32_t i;

        for(i = 0u; i < CY_BLE_CONN_COUNT; i++)
        {
            apiResult = Cy_BLE_BASS_GetCharacteristicDescriptor(cy_ble_connHandle[i], BAS_SERVICE_SIMULATE,
                                                                CY_BLE_BAS_BATTERY_LEVEL, CY_BLE_BAS_BATTERY_LEVEL_CCCD,
                                                                CY_BLE_CCCD_LEN, (uint8_t*)&cccdValue);
            if((apiResult == CY_BLE_SUCCESS) && (cccdValue != 0u))
            {
                batterySimulationNotify |= ENABLED << i;
            }
        }
    }
#endif /* (BAS_SIMULATE_ENABLE != 0) */
}

/*******************************************************************************
* Function Name: BasCallBack()
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service specific events from 
*   the Battery Service.
*
* Parameters:
*  event - The event code.
*  *eventParam - The event parameters.
*
********************************************************************************/
void BasCallBack(uint32_t event, void *eventParam)
{
    app_stc_connection_info_t *appConnInfoPtr = GetAppConnInfoPtr();
    uint8_t locServiceIndex;
    uint32_t i;
    
    locServiceIndex = ((cy_stc_ble_bas_char_value_t *)eventParam)->serviceIndex;
    DBG_PRINTF("BAS event: %lx, \r\n", event);
    
    switch(event)
    {
        case CY_BLE_EVT_BASS_NOTIFICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_BASS_NOTIFICATION_ENABLED: %x \r\n", locServiceIndex);
            batterySimulationNotify |= ENABLED << locServiceIndex;
            break;
                
        case CY_BLE_EVT_BASS_NOTIFICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_BASS_NOTIFICATION_DISABLED: %x \r\n", locServiceIndex);
            batterySimulationNotify &= (uint16_t)(~(ENABLED << locServiceIndex));
            break;
            
        case CY_BLE_EVT_BASC_NOTIFICATION:
            DBG_PRINTF("CY_BLE_EVT_BASC_NOTIFICATION: Battery Level: %d [attId:%x, bdHandle:%x]\r\n", 
                                                    *((cy_stc_ble_bas_char_value_t *)eventParam)->value->val,
                                                     ((cy_stc_ble_bas_char_value_t *)eventParam)->connHandle.attId,
                                                     ((cy_stc_ble_bas_char_value_t *)eventParam)->connHandle.bdHandle);
            
            for(i = 0u; i < CY_BLE_MAX_CENTRAL_CONN_NUM; i++ )
            {
                if(appConnInfoPtr->central[i].connHandle.bdHandle == 
                    ((cy_stc_ble_bas_char_value_t *)eventParam)->connHandle.bdHandle)
                {
                    appConnInfoPtr->central[i].battaryLevel = *((cy_stc_ble_bas_char_value_t *)eventParam)->value->val;
                    appConnInfoPtr->central[i].isNewNotification = true;
                }     
            }
            break;
            
        case CY_BLE_EVT_BASC_READ_CHAR_RESPONSE:
            DBG_PRINTF("Battery Level: %x \r\n", *((cy_stc_ble_bas_char_value_t *)eventParam)->value->val);
            break;
            
        case CY_BLE_EVT_BASC_READ_DESCR_RESPONSE:
            DBG_PRINTF("Read Descriptor: \r\n");
            break;
            
        case CY_BLE_EVT_BASC_WRITE_DESCR_RESPONSE:
            DBG_PRINTF("Write Descriptor Confirm \r\n");
            break;
            
        default:
            DBG_PRINTF("Other event\r\n");
            break;
    }
}

/*******************************************************************************
* Function Name: ProcessingBatteryData
********************************************************************************
*
* Summary:
*   The Custom function to simulate a Battery voltage.
*
*******************************************************************************/
void ProcessingBatteryData(void)
{
    cy_en_ble_api_result_t apiResult;
    app_stc_connection_info_t *appConnInfoPtr = GetAppConnInfoPtr();
    uint32_t i;
           
    for(i = 0u; i < CY_BLE_MAX_CENTRAL_CONN_NUM; i++)
    {
        if((Cy_BLE_GetConnectionState(appConnInfoPtr->central[i].connHandle) >= CY_BLE_CONN_STATE_CONNECTED) &&
           (appConnInfoPtr->central[i].isNewNotification == true) &&
           (Cy_BLE_GetConnectionState(appConnInfoPtr->peripheral[0u].connHandle) == CY_BLE_CONN_STATE_CONNECTED) &&
           (Cy_BLE_GATT_GetBusyStatus(appConnInfoPtr->peripheral[0u].connHandle.attId) == CY_BLE_STACK_STATE_FREE))
        {
            if((batterySimulationNotify & (ENABLED << i)) != 0u)
            {
                /* Update the Battery level characteristic value and send notification */
                apiResult = Cy_BLE_BASS_SendNotification(appConnInfoPtr->peripheral[0u].connHandle, i, 
                                                            CY_BLE_BAS_BATTERY_LEVEL, sizeof(uint8_t), 
                                                            &appConnInfoPtr->central[i].battaryLevel);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_BASS_SendNotification API Error: 0x%x [connHandle: 0x%x, 0x%x] \r\n", apiResult,
                                appConnInfoPtr->peripheral[0u].connHandle.attId,
                                appConnInfoPtr->peripheral[0u].connHandle.bdHandle);
                    batterySimulationNotify = DISABLED;
                }
            }
            else
            {
                /* Update the Battery level characteristic value */
                apiResult = Cy_BLE_BASS_SetCharacteristicValue(i, CY_BLE_BAS_BATTERY_LEVEL, sizeof(uint8_t), 
                                                                             &appConnInfoPtr->central[i].battaryLevel);   
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_BASS_SetCharacteristicValue API Error: 0x%x \r\n", apiResult);
                    batterySimulationNotify = DISABLED;
                }
            }
            
            appConnInfoPtr->central[i].isNewNotification = false;
        }
    }
}


/* [] END OF FILE */
