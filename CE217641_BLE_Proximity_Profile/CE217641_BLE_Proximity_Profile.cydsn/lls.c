/*******************************************************************************
* File Name: lls.c
*
*  Version 1.0
*
* Description:
*  This file contains Link Loss Service callback handler function.
* 
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "lls.h"
#include "user_interface.h"

/***************************************
*        Global Variables
***************************************/

static uint8_t         alertLevel;

/*******************************************************************************
* Function Name: LlsInit
********************************************************************************
*
* Summary: This function initialized parameters for LLS.
*
*******************************************************************************/
void LlsInit(void)
{
    /* Register the event handler for LLS specific events */
    Cy_BLE_LLS_RegisterAttrCallback(LlsServiceAppEventHandler);
}

/*******************************************************************************
* Function Name: LlsServiceAppEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component,
*  which are specific to Link Loss Service.
*
* Parameters:
*  event      - The event code.
*  eventParam - The event parameters.

*
*******************************************************************************/
void LlsServiceAppEventHandler(uint32_t event, void *eventParam)
{
    cy_stc_ble_lls_char_value_t * llsWrReqValueParam;
    
    /* There is only one event present in Link Loss Service for Server GATT role.
    *  'CY_BLE_EVT_LLSS_WRITE_CHAR_REQ' - write request to set new alert level. 
    */
    if (event == CY_BLE_EVT_LLSS_WRITE_CHAR_REQ)
    {
        DBG_PRINTF("Write LLS Alert Level request received \r\n");

        llsWrReqValueParam = (cy_stc_ble_lls_char_value_t *) eventParam;

        if(llsWrReqValueParam->value->len == CY_BLE_LLS_ALERT_LEVEL_SIZE)
        {
            alertLevel = llsWrReqValueParam->value->val[0u];

            switch (alertLevel)
            {
            case CY_BLE_NO_ALERT:
                DBG_PRINTF("Alert Level for LLS is set to \"No Alert\" \r\n");
                break;

            case CY_BLE_MILD_ALERT:
                DBG_PRINTF("Alert Level for LLS is set to \"Mild Alert\" \r\n");
                break;

            case CY_BLE_HIGH_ALERT:
                DBG_PRINTF("Alert Level for LLS is set to \"High Alert\" \r\n");
                break;

            default:
                DBG_PRINTF("Incorrect value of Alert Level for LLS was received\r\n");
                break;
            }
        }
    }
}

/*******************************************************************************
* Function Name: LlsGetAlertLevel
********************************************************************************
*
* Summary:
*   This function returns alert level value.
*
* Returns:
*  uint8_t - alert level value.
*
*******************************************************************************/
uint8_t LlsGetAlertLevel(void)
{
    return(alertLevel);
}

/*******************************************************************************
* Function Name: LlsSetAlertLevel
********************************************************************************
*
* Summary:
*   This function set alert level value.
*
* Parameters:
*   alertLevelValue - alert level value.
*
*******************************************************************************/
void LlsSetAlertLevel(uint8_t alertLevelValue)
{
    alertLevel = alertLevelValue;
}


/* [] END OF FILE */
