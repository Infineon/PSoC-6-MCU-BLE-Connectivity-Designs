/*******************************************************************************
* File Name: ndcs.c
*
* Version: 1.0
*
* Description:
*  This file contains NDCS callback handler function.
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
#include "ndcs.h"
#include "user_interface.h"


/*******************************************************************************
* Function Name: NdcsCallBack
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component,
*  which are specific to Next DST Change Service.
*
* Parameters:  
*  event      - the event code
*  eventParam - the event parameters
*
*******************************************************************************/
void NdcsCallBack(uint32_t event, void *eventParam)
{
    uint8_t i;
    cy_stc_ble_ndcs_char_value_t *charValue;
    
    switch(event)
    {
    case CY_BLE_EVT_NDCSC_READ_CHAR_RESPONSE:
        charValue = (cy_stc_ble_ndcs_char_value_t *) eventParam;
        
        DBG_PRINTF("Read Characteristic response is received!\r\n");
        DBG_PRINTF("Data length: %x, ", charValue->value->len);
        DBG_PRINTF("Data: ");

        for(i = 0u; i < charValue->value->len; i++)
        {
            DBG_PRINTF("%d ", charValue->value->val[i]);
        }
        DBG_PRINTF("\r\n");
        break;
        
    default:
        DBG_PRINTF("Unrecognized NDCS event.\r\n");
        break;
    }
}


/*******************************************************************************
* Function Name: NdcsInit
********************************************************************************
*
* Summary:
*  Initializes the NDCS application global variables.
*
*
*******************************************************************************/
void NdcsInit(void)
{
    /* Register the event handler for NDCS specific events */
    Cy_BLE_NDCS_RegisterAttrCallback(NdcsCallBack);
}


/* [] END OF FILE */
