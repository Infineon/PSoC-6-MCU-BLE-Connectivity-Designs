/*******************************************************************************
* File Name: ias.c
*
* Description:
*  This file contains Immediate Alert Service callback handler function.
* 
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "project.h"
#include "ias.h"


/* IAS alert level value */
volatile uint8_t alertLevel = 0;

/******************************************************************************
* Function Name: IasInit
*******************************************************************************
*
* Summary:
*   Registers the IAS CallBack.
*
******************************************************************************/
void IasInit(void)
{
    Cy_BLE_IAS_RegisterAttrCallback(IasEventHandler);
}


/*******************************************************************************
* Function Name: IasEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component,
*  which are specific to Immediate Alert Service.
*
* Parameters:
*   event:       Write Command event from the BLE component.
*   eventParams: A structure instance of CY_BLE_GATT_HANDLE_VALUE_PAIR_T type.
*
*******************************************************************************/
void IasEventHandler(uint32 event, void *eventParam)
{
    (void) eventParam;
    uint8_t alert;
    
    /* Alert Level Characteristic write event */
    if(event == CY_BLE_EVT_IASS_WRITE_CHAR_CMD)
    {
        /* Read the updated Alert Level value from the GATT database */
        Cy_BLE_IASS_GetCharacteristicValue(CY_BLE_IAS_ALERT_LEVEL, sizeof(alert), &alert);
        alertLevel = alert;
    }   
}


/* [] END OF FILE */
