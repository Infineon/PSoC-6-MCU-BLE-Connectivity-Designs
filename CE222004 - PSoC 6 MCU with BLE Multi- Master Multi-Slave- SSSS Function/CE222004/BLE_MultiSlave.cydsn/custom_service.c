/*******************************************************************************
* File Name: custom_service.c
*
* Version: 1.0
*
* Description:
*  This file contains custom service related functions.
* 
*******************************************************************************
* Copyright (2017-2018), Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress’s integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress 
* reserves the right to make changes to the Software without notice. Cypress 
* does not assume any liability arising out of the application or use of the 
* Software or any product or circuit described in the Software. Cypress does 
* not authorize its products for use in any products where a malfunction or 
* failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such 
* system or application assumes all risk of such use and in doing so agrees to 
* indemnify Cypress against all liability.
*******************************************************************************/
#include "custom_service.h"

/* Array to store the Custom Service Data */
uint8_t customServiceData[CUSTOM_SERVICE_LEN];

/*******************************************************************************
* Function Name: HandleCustomServiceWriteRequest
********************************************************************************
* Summary:
*  This functions handles the 'write request' event for the 128-bit Custom service
*
* Parameters:
*  writeRequest : pointer to the write request parameters from the central        
*
* Return:
*  void
*
*******************************************************************************/
void HandleCustomServiceWriteRequest(cy_stc_ble_gatts_write_cmd_req_param_t *writeRequest)
{
    uint8_t i;
    cy_stc_ble_conn_handle_t appConnHandle = writeRequest->connHandle;
    
    for(i = 0; i<CUSTOM_SERVICE_LEN; i++)
    {
        customServiceData[i] = writeRequest->handleValPair.value.val[i];
    }
    
    /* Local variable that stores custom service data parameters */
    cy_stc_ble_gatt_handle_value_pair_t  customHandle = 
    {
        .attrHandle = CY_BLE_CUSTOM_SERVICE_CUSTOM_CHARACTERISTIC_CHAR_HANDLE,
        .value.val  = customServiceData,
        .value.len  = CUSTOM_SERVICE_LEN 
    };
    
    /* Local variable that stores custom service attribute value */
    cy_stc_ble_gatts_db_attr_val_info_t  userAttributeValue = 
    {
        .handleValuePair = customHandle,
        .offset = CY_BLE_CCCD_DEFAULT,
        .connHandle = appConnHandle,
        .flags = CY_BLE_GATT_DB_LOCALLY_INITIATED
    };
    
    /* Send updated Custom Service control handle as an attribute to the central device, 
       so that the central reads the new user data */
    Cy_BLE_GATTS_WriteAttributeValue(&userAttributeValue);
}

/* [] END OF FILE */
