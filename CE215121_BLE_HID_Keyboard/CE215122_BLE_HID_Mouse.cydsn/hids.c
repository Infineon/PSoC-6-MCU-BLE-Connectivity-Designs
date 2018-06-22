/*******************************************************************************
* File Name: hids.c
*
* Version: 1.0
*
* Description:
*  This file contains the code for the HIDS.
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
#include "user_interface.h"
#include "hids.h"


/* Static global variables */
static uint8_t  protocol = CY_BLE_HIDS_PROTOCOL_MODE_REPORT;   /* Boot or Report protocol mode */
static uint8_t  suspend  = CY_BLE_HIDS_CP_EXIT_SUSPEND;        /* Suspend to enter into deep sleep mode */


/*******************************************************************************
* Function Name: HidsInit
********************************************************************************
*
* Summary:
*   Initializes the HID service.
*
*******************************************************************************/
void HidsInit(void)
{    
    /* Register service specific callback function */
    Cy_BLE_HIDS_RegisterAttrCallback(HidsCallBack);
}


/*******************************************************************************
* Function Name: HidsCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service specific events from 
*   HID Service.
*
* Parameters:
*  event       - the event code
*  *eventParam - the event parameters
*
********************************************************************************/
void HidsCallBack(uint32_t event, void *eventParam)
{
    cy_stc_ble_hids_char_value_t *locEventParam = (cy_stc_ble_hids_char_value_t *)eventParam;

    DBG_PRINTF("HIDS event: %lx, ", event);

    switch(event)
    {
        case CY_BLE_EVT_HIDSS_NOTIFICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_HIDSS_NOTIFICATION_ENABLED %x %x: serv=%x, char=%x\r\n", 
                locEventParam->connHandle.attId,
                locEventParam->connHandle.bdHandle,
                locEventParam->serviceIndex,
                locEventParam->charIndex);
            break;
            
        case CY_BLE_EVT_HIDSS_NOTIFICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_HIDSS_NOTIFICATION_DISABLED %x %x: serv=%x, char=%x\r\n", 
                locEventParam->connHandle.attId,
                locEventParam->connHandle.bdHandle,
                locEventParam->serviceIndex,
                locEventParam->charIndex);
            break;
            
        case CY_BLE_EVT_HIDSS_BOOT_MODE_ENTER:
            DBG_PRINTF("CY_BLE_EVT_HIDSS_BOOT_MODE_ENTER \r\n");
            protocol = CY_BLE_HIDS_PROTOCOL_MODE_BOOT;
            break;
            
        case CY_BLE_EVT_HIDSS_REPORT_MODE_ENTER:
            DBG_PRINTF("CY_BLE_EVT_HIDSS_REPORT_MODE_ENTER \r\n");
            protocol = CY_BLE_HIDS_PROTOCOL_MODE_REPORT;
            break;
            
        case CY_BLE_EVT_HIDSS_SUSPEND:
            DBG_PRINTF("CY_BLE_EVT_HIDSS_SUSPEND \r\n");
            suspend = CY_BLE_HIDS_CP_SUSPEND;
        #if (DEBUG_UART_ENABLED == ENABLED)
            /* Reduce power consumption, power down logic that is not required to wake up the system */
            UART_DEB_Disable();
        #endif /* (DEBUG_UART_ENABLED == ENABLED) */
            break;
        
        case CY_BLE_EVT_HIDSS_EXIT_SUSPEND:
        #if (DEBUG_UART_ENABLED == ENABLED)
            /* Power up all circuitry previously shut down */
            UART_DEB_Start();
        #endif /* (DEBUG_UART_ENABLED == ENABLED) */
            DBG_PRINTF("CY_BLE_EVT_HIDSS_EXIT_SUSPEND \r\n");
            suspend = CY_BLE_HIDS_CP_EXIT_SUSPEND;
            break;
            
        case CY_BLE_EVT_HIDSS_REPORT_WRITE_CHAR:
            DBG_PRINTF("CY_BLE_EVT_HIDSS_REPORT_WRITE_CHAR %x %x: serv=%x, char=%x, value=", 
                locEventParam->connHandle.attId,
                locEventParam->connHandle.bdHandle,
                locEventParam->serviceIndex,
                locEventParam->charIndex);
                ShowValue(locEventParam->value);
            break;
                
        case CY_BLE_EVT_HIDSC_NOTIFICATION:
            break;
                
        case CY_BLE_EVT_HIDSC_READ_CHAR_RESPONSE:
            break;
                
        case CY_BLE_EVT_HIDSC_WRITE_CHAR_RESPONSE:
            break;
                
        case CY_BLE_EVT_HIDSC_READ_DESCR_RESPONSE:
            break;
                
        case CY_BLE_EVT_HIDSC_WRITE_DESCR_RESPONSE:           
            break;
                
        default:
            DBG_PRINTF("Not supported event\r\n");
            break;
    }
}


/*******************************************************************************
* Function Name: SimulateMouse
********************************************************************************
*
* Summary:
*   The custom function to simulate mouse moving in a square clockwise position.
*
* Parameters:
*  connHandle: The connection handle
*
*******************************************************************************/
void HidsSimulateMouse(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    uint8_t abMouseData[MOUSE_DATA_LEN] = {0, 0, 0};  /* Mouse packet array */
    static uint8_t boxLoop = 0;                       /* Box loop counter */
    static uint8_t leftButtonPress = 0;               /* Left Button */
    static uint8_t dirState = 0;                      /* Mouse direction state */
    uint16_t cccd = CY_BLE_CCCD_DEFAULT;
    int8_t bXInc = 0;                                 /* X-Step Size */
    int8_t bYInc = 0;                                 /* Y-Step Size */
    bool mouseSimulation = false;
    
    /* Init Left Button state */
    leftButtonPress = (SW2_Read() == 0u) ? 1u : 0u;

    boxLoop++;
    if(boxLoop > BOX_SIZE)              /* Change mouse direction every 32 packets */
    {
        boxLoop = 0;
        dirState++;                     /* Advance box state */
        dirState &= POSMASK;
    }

    switch(dirState)                    /* Determine current direction state */
    {
        case MOUSE_DOWN:                /* Down */
            bXInc = 0;
            bYInc = CURSOR_STEP;
            break;
            
        case MOUSE_LEFT:                /* Left */
            bXInc = -CURSOR_STEP;
            bYInc = 0;
            break;
            
        case MOUSE_UP:                  /* Up */
            bXInc = 0;
            bYInc = -CURSOR_STEP;
            break;
            
        case MOUSE_RIGHT:               /* Right */
            bXInc = CURSOR_STEP;
            bYInc = 0;
            break;
    }

    abMouseData[1u] = bXInc;                                /* Load the packet array */
    abMouseData[2u] = bYInc;
    abMouseData[0u] = (leftButtonPress ? MOUSE_LB : 0u);    /* Set up Left Button state */
    DBG_PRINTF("HID notification %x %x %x", abMouseData[0],abMouseData[1],abMouseData[2]);
    
    /* Read CCCD configurations */
    apiResult = Cy_BLE_HIDSS_GetCharacteristicDescriptor(connHandle, CY_BLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
                                                         CY_BLE_HUMAN_INTERFACE_DEVICE_REPORT_IN, CY_BLE_HIDS_REPORT_CCCD,
                                                         CY_BLE_CCCD_LEN, (uint8_t *)&cccd);
    if((apiResult == CY_BLE_SUCCESS) && (cccd != 0u))
    {
        mouseSimulation = true;
    }
    
    apiResult = Cy_BLE_HIDSS_GetCharacteristicDescriptor(connHandle, CY_BLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
                                                         CY_BLE_HIDS_BOOT_KYBRD_IN_REP, CY_BLE_HIDS_REPORT_CCCD, 
                                                         CY_BLE_CCCD_LEN, (uint8_t *)&cccd);
    if((apiResult == CY_BLE_SUCCESS) && (cccd != 0u))
    {
        mouseSimulation = true;
    }
    
    if((mouseSimulation ==  true) &&(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_FREE))
    {
        if(protocol == CY_BLE_HIDS_PROTOCOL_MODE_BOOT)
        {
            apiResult = Cy_BLE_HIDSS_SendNotification(connHandle, CY_BLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
                                                      CY_BLE_HIDS_BOOT_MOUSE_IN_REP, MOUSE_DATA_LEN, abMouseData);
        }
        else
        {
            apiResult = Cy_BLE_HIDSS_SendNotification(connHandle, CY_BLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
                                                      CY_BLE_HUMAN_INTERFACE_DEVICE_REPORT_IN, MOUSE_DATA_LEN, abMouseData);
        }
        
        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("Cy_BLE_HIDSS_SendNotification API Error: 0x%x \r\n", apiResult);
        }
        else
        {
            DBG_PRINTF(", attId:%d", connHandle.attId);
        }
    }
    
    DBG_PRINTF("\r\n");
}


/*******************************************************************************
* Function Name: HidsGetSuspendState
********************************************************************************
*
* Summary:
*   This function returns HIDS suspend state
*
* Returns:
*  CY_BLE_HIDS_CP_SUSPEND      - HID Host is entering the Suspend State
*  CY_BLE_HIDS_CP_EXIT_SUSPEND - HID Host is exiting the Suspend State
*
*******************************************************************************/
uint8_t HidsGetSuspendState(void)
{
    return(suspend);   
}


/*******************************************************************************
* Function Name: HidsIsSimulationActive
********************************************************************************
*
* Summary:
*   This function returns simulation state active/not active.
*
*******************************************************************************/
bool HidsIsSimulationActive(void)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t i;
    uint16_t cccd;
    bool ret = false;
    
    for(i = 0u; (i < CY_BLE_CONN_COUNT) && (ret == false); i++)
    {
        cccd = CY_BLE_CCCD_DEFAULT;
        apiResult = Cy_BLE_HIDSS_GetCharacteristicDescriptor(cy_ble_connHandle[i], CY_BLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
                                                             CY_BLE_HUMAN_INTERFACE_DEVICE_REPORT_IN, CY_BLE_HIDS_REPORT_CCCD,
                                                             CY_BLE_CCCD_LEN, (uint8_t *)&cccd);
        if((apiResult == CY_BLE_SUCCESS) && (cccd != 0u))
        {
            ret = true;
        }
        
        cccd = CY_BLE_CCCD_DEFAULT;
        apiResult = Cy_BLE_HIDSS_GetCharacteristicDescriptor(cy_ble_connHandle[i], CY_BLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
                                                             CY_BLE_HIDS_BOOT_KYBRD_IN_REP, CY_BLE_HIDS_REPORT_CCCD, 
                                                             CY_BLE_CCCD_LEN, (uint8_t *)&cccd);
        if((apiResult == CY_BLE_SUCCESS) && (cccd != 0u))
        {
            ret = true;
        }
    }
    return(ret);   
}

/* [] END OF FILE */
