/*******************************************************************************
* File Name: scps.c
*
* Version: 1.0
*
* Description:
*  This file contains the code for the SCPS.
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
* 
********************************************************************************
* Copyright 2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"
#include "scps.h"

/* Static global variables */
static uint16_t scanInterval       = 0u;
static uint16_t scanWindow         = 0u;
static bool requestScanRefresh     = true;

/*******************************************************************************
* Function Name: ScpsInit()
********************************************************************************
*
* Summary:
*   Initializes the SCPS Service.
*
*******************************************************************************/
void ScpsInit(void)
{   
    /* Register service specific callback function */
    Cy_BLE_SCPS_RegisterAttrCallback(ScpsCallBack);
    
    requestScanRefresh = true;
}


/*******************************************************************************
* Function Name: ScpsCallBack()
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service specific events from 
*   SCPS Service.
*
* Parameters:
*  event      - the event code
*  eventParam - the event parameters
*
********************************************************************************/
void ScpsCallBack (uint32_t event, void *eventParam)
{
    DBG_PRINTF("SCPS event: %lx, ", event);
    switch(event)
    {
        case CY_BLE_EVT_SCPSS_NOTIFICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_SCPSS_NOTIFICATION_ENABLED %x %x \r\n",
                ((cy_stc_ble_scps_char_value_t *)eventParam)->connHandle.attId,
                ((cy_stc_ble_scps_char_value_t *)eventParam)->connHandle.bdHandle); 
            break;
            
        case CY_BLE_EVT_SCPSS_NOTIFICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_SCPSS_NOTIFICATION_DISABLED %x %x \r\n",
                ((cy_stc_ble_scps_char_value_t *)eventParam)->connHandle.attId,
                ((cy_stc_ble_scps_char_value_t *)eventParam)->connHandle.bdHandle);
            break;
            
        case CY_BLE_EVT_SCPSS_SCAN_INT_WIN_WRITE_CHAR:
            scanInterval = Cy_BLE_Get16ByPtr(((cy_stc_ble_scps_char_value_t *)eventParam)->value->val);
            scanWindow   = Cy_BLE_Get16ByPtr(((cy_stc_ble_scps_char_value_t *)eventParam)->value->val + 
                            sizeof(scanInterval));
            DBG_PRINTF("CY_BLE_EVT_SCPSS_SCAN_INT_WIN_WRITE_CHAR scanInterval: %x, scanWindow: %x \r\n", 
                        scanInterval, scanWindow);
            break;
            
        case CY_BLE_EVT_SCPSC_NOTIFICATION:
            break;
            
        case CY_BLE_EVT_SCPSC_READ_DESCR_RESPONSE:
            break;
            
        case CY_BLE_EVT_SCPSC_WRITE_DESCR_RESPONSE:
            break;
            
        default:
            DBG_PRINTF("Not supported event\r\n");
            break;
    }
}


/*******************************************************************************
* Function Name: ScpsSendReqUpdateConnParam()
********************************************************************************
*
* Summary:
*   Send notification to request update connection parameters
*
* Parameters:
*   connHandle: The connection handle
*
********************************************************************************/
void ScpsSendReqUpdateConnParam(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    uint8_t refresh = CY_BLE_SCAN_REFRESH_ENABLED;
    uint16 cccd = CY_BLE_CCCD_DEFAULT;

    if(requestScanRefresh == true)
    {
        apiResult = Cy_BLE_SCPSS_GetCharacteristicDescriptor(connHandle, CY_BLE_SCPS_SCAN_REFRESH,
                                                             CY_BLE_SCPS_SCAN_REFRESH_CCCD, CY_BLE_CCCD_LEN,
                                                             (uint8_t *)&cccd);
        
        if((apiResult == CY_BLE_SUCCESS) && (cccd == CY_BLE_CCCD_NOTIFICATION))
        {
            /* Send notification to request update connection parameters */
            apiResult = Cy_BLE_SCPSS_SendNotification(connHandle, CY_BLE_SCPS_SCAN_REFRESH, sizeof(refresh), &refresh);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_SCPSS_SendNotification API Error: 0x%x \r\n", apiResult);
            }
            else
            {
                requestScanRefresh = false; 
            }
        }
        
        /* Cy_BLE_ProcessEvents() allows BLE stack to process pending events */
        Cy_BLE_ProcessEvents();
    }
}


/* [] END OF FILE */
