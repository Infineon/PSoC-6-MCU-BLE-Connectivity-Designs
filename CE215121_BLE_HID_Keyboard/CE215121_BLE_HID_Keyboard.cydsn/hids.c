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
#include "hids.h"

/* Static global variables */
static bool     capsLockPress = false;   
static bool     capsLockLed   = false;

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
            if(CY_BLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX == locEventParam->serviceIndex)
            {
                /* Write request to Keyboard Output Report characteristic. 
                *  Handle Boot and Report protocol. 
                */
                if( ((CY_BLE_HIDS_PROTOCOL_MODE_REPORT == protocol) && 
                     (CY_BLE_HUMAN_INTERFACE_DEVICE_REPORT_OUT == locEventParam->charIndex)) ||
                     ((CY_BLE_HIDS_PROTOCOL_MODE_BOOT == protocol) && 
                     (CY_BLE_HIDS_BOOT_KYBRD_OUT_REP == locEventParam->charIndex)) )
                {
                    if( (CAPS_LOCK_LED & locEventParam->value->val[0u]) != 0u)
                    {
                        capsLockLed = true;
                    }
                    else
                    {
                        capsLockLed = false;
                    }
                }
            }
            DBG_PRINTF("CY_BLE_EVT_HIDSS_REPORT_WRITE_CHAR: serv=%x, char=%x, value=", 
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
* Function Name: HidsSimulateKeyboard
********************************************************************************
*
* Summary:
*   The custom function to simulate CapsLock key pressing
*
* Parameters:
*  connHandle: The connection handle
*
*******************************************************************************/
void HidsSimulateKeyboard(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    static uint32_t keyboardTimer = KEYBOARD_TIMEOUT;
    static uint8_t keyboard_data[KEYBOARD_DATA_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0};
    static uint8_t simKey; 
    bool keyboardSimulation = false;
    uint32_t i;
    uint16_t cccd = CY_BLE_CCCD_DEFAULT;
    
    if(capsLockPress == true)
    {
        keyboard_data[2u] = CAPS_LOCK;              /* Set up keyboard data */
        keyboardTimer = 1u;                         /* Clear Simulation timer to send data */
        capsLockPress = false;
    }
    
    /* Read CCCD configurations */
    apiResult = Cy_BLE_HIDSS_GetCharacteristicDescriptor(connHandle, CY_BLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
                                                         CY_BLE_HUMAN_INTERFACE_DEVICE_REPORT_IN, CY_BLE_HIDS_REPORT_CCCD,
                                                         CY_BLE_CCCD_LEN, (uint8_t *)&cccd);
    if((apiResult == CY_BLE_SUCCESS) && (cccd != 0u))
    {
        keyboardSimulation = true;
    }
    
    apiResult = Cy_BLE_HIDSS_GetCharacteristicDescriptor(connHandle, CY_BLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
                                                         CY_BLE_HIDS_BOOT_KYBRD_IN_REP, CY_BLE_HIDS_REPORT_CCCD, 
                                                         CY_BLE_CCCD_LEN, (uint8_t *)&cccd);
    if((apiResult == CY_BLE_SUCCESS) && (cccd != 0u))
    {
        keyboardSimulation = true;
    }
    
    
    if((Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_FREE) && 
       (--keyboardTimer == 0u) && (keyboardSimulation == true))
    //(--keyboardTimer == 0u) && ((keyboardSimulation & (ENABLED << connHandle.attId)) != 0u))
    {
        keyboardTimer = KEYBOARD_TIMEOUT;
    
        simKey++;
        if(simKey > SIM_KEY_MAX)
        {
            simKey = SIM_KEY_MIN; 
        }
        keyboard_data[3u] = simKey;              
        
        apiResult = Cy_BLE_HIDSS_GetCharacteristicValue(CY_BLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
                                                        CY_BLE_HIDS_PROTOCOL_MODE, sizeof(protocol), &protocol);
        if(apiResult == CY_BLE_SUCCESS)
        {
            DBG_PRINTF("HID notification: ");
            for(i = 0; i < KEYBOARD_DATA_SIZE; i++)
            {
                DBG_PRINTF("%2.2x,", keyboard_data[i]);
            }
            DBG_PRINTF("\r\n");
            
            if(protocol == CY_BLE_HIDS_PROTOCOL_MODE_BOOT)
            {
                apiResult = Cy_BLE_HIDSS_SendNotification(connHandle, CY_BLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX,
                                                          CY_BLE_HIDS_BOOT_KYBRD_IN_REP, KEYBOARD_DATA_SIZE,
                                                          keyboard_data);
            }
            else
            {
                apiResult = Cy_BLE_HIDSS_SendNotification(connHandle, CY_BLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
                                                          CY_BLE_HUMAN_INTERFACE_DEVICE_REPORT_IN, KEYBOARD_DATA_SIZE, 
                                                          keyboard_data);
            }
            
            if(apiResult == CY_BLE_SUCCESS)
            {
                keyboard_data[2u] = 0u;                       /* Set up keyboard data */
                keyboard_data[3u] = 0u;                       /* Set up keyboard data */
                if(protocol == CY_BLE_HIDS_PROTOCOL_MODE_BOOT)
                {
                    apiResult = Cy_BLE_HIDSS_SendNotification(connHandle, CY_BLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX,
                                                              CY_BLE_HIDS_BOOT_KYBRD_IN_REP, KEYBOARD_DATA_SIZE,
                                                              keyboard_data);
                }
                else
                {
                    apiResult = Cy_BLE_HIDSS_SendNotification(connHandle, CY_BLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
                                                              CY_BLE_HUMAN_INTERFACE_DEVICE_REPORT_IN, KEYBOARD_DATA_SIZE,
                                                              keyboard_data);
                }
            }
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("HID notification API Error: 0x%x \r\n", apiResult);
            }
        }
        Cy_BLE_ProcessEvents();
    }
    
    /* Reset keyboard timer */
    keyboardTimer = (keyboardTimer == 0u) ? KEYBOARD_TIMEOUT : keyboardTimer;
}


/*******************************************************************************
* Function Name: HidsIsCapsLock
********************************************************************************
*
* Summary:
*   This function returns CapsLock status
*
* Returns:
*  true  - CapsLock on
*  false - CapsLock off
*
*******************************************************************************/
bool HidsIsCapsLock(void)
{
    return((capsLockLed == true) ? true : false);   
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
* Function Name: HidsSetCapsLockPressState
********************************************************************************
*
* Summary:
*   This function inform that CapsLock pressed . 
*
*******************************************************************************/
void HidsSetCapsLockPress(void)
{
    capsLockPress = true;
}



/* [] END OF FILE */
