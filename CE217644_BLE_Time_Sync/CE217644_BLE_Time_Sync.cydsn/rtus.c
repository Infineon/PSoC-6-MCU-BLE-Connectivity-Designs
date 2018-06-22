/*******************************************************************************
* File Name: rtus.c
*
* Version: 1.0
*
* Description:
*  This file contains RTUS callback handler function.
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
#include <stdio.h>
#include "common.h"
#include "rtus.h"
#include "user_interface.h"


/*******************************************************************************
* Function Name: RtusCallBack
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component,
*  which are specific to Reference Time Update Service.
*
* Parameters:  
*  event      - The event code.
*  eventParam - The pointer to event data.
*
*******************************************************************************/
void RtusCallBack(uint32_t event, void *eventParam)
{
    uint8_t i;
    cy_stc_ble_rtus_char_value_t *charValue;
    
    switch(event)
    {
    /***************************************
    *        RTUS Client events
    ***************************************/
    case CY_BLE_EVT_RTUSC_READ_CHAR_RESPONSE:
        charValue = (cy_stc_ble_rtus_char_value_t *) eventParam;
        
        DBG_PRINTF("Read Characteristic response is received!\r\n");
        DBG_PRINTF("Data length: %x, ", charValue->value->len);
        DBG_PRINTF("Data: ");

        for(i = 0u; i < charValue->value->len; i++)
        {
            DBG_PRINTF("%x ", charValue->value->val[i]);
        }
        DBG_PRINTF("\r\n");
        break;

    /***************************************
    *        RTUS Server events
    ***************************************/
    case CY_BLE_EVT_RTUSS_WRITE_CHAR_CMD:
        break;
    
    default:
        DBG_PRINTF("Unrecognized RTUS event.\r\n");
        break;
    }
}

/*******************************************************************************
* Function Name: RtusInit
********************************************************************************
*
* Summary:
*  Initializes the RTUS application global variables.
*
*******************************************************************************/
void RtusInit(void)
{
    /* Register the event handler for RTUS specific events */
    Cy_BLE_RTUS_RegisterAttrCallback(RtusCallBack);
}

    
/* [] END OF FILE */
