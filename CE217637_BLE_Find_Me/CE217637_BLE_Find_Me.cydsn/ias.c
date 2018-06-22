/*******************************************************************************
* File Name: ias.c
*
*  Version: 1.0
*
* Description:
*  This file contains Immediate Alert Service callback handler function.
* 
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>
#include "ias.h"

/* IAS alert level value */
static uint8_t alertLevel = NO_ALERT;

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
*   event - the event code
*   *eventParam - the event parameters
*
*******************************************************************************/
void IasEventHandler(uint32_t event, void *eventParam)
{    
    /* Alert Level Characteristic write event */
    if(event == CY_BLE_EVT_IASS_WRITE_CHAR_CMD)
    {
        alertLevel = *((cy_stc_ble_ias_char_value_t *)eventParam)->value->val;
    }   
}


/*******************************************************************************
* Function Name: IasSetAlertLevel
********************************************************************************
*
* Summary:
*   This function sets alert level value.
*
* Paramerets:
*  alertLevelValue - Value of alert level.
*******************************************************************************/
void IasSetAlertLevel(uint8_t alertLevelValue)
{
    alertLevel = alertLevelValue;
}


/*******************************************************************************
* Function Name: IasGetAlertLevel
********************************************************************************
*
* Summary:
*   This function returns alert level value.
*
* Returns:
*  uint8_t - alert level value.
*******************************************************************************/
uint8_t IasGetAlertLevel(void)
{
    return(alertLevel);
}
/* [] END OF FILE */
