/*******************************************************************************
* File Name: rgb_led_service.c
*
* Version: 1.0
*
* Description:
*  This file contains RGB LED Service related functions.
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
#include "rgb_led_service.h"

/* Array to store the present RGB LED control data. The four bytes of the array 
   represent {Red, Green, Blue, Intensity} respectively */
uint8_t rgbData[RGB_DATA_LEN];

/* Variable used to store RGB notification status */
uint8_t rgbNotificationStatus = CCCD_NOTIFY_DISABLED;

/*******************************************************************************
* Function Name: UpdateCccdStatusInGattDb
********************************************************************************
* Summary:
*  This function updates the notification status (lower byte of CCCD array) of
*  a characteristic in GATT DB with the provided parameters
*
* Parameters:
*  cccdHandle   :	CCCD handle of the service
*  appConnHandle:   Connection handle
*  value        :   Notification status. Valid values are CCCD_NOTIFY_DISABLED and
*                   CCCD_NOTIFY_ENABLED
*
* Return:
*  void
*
*******************************************************************************/
void UpdateCccdStatusInGattDb(cy_ble_gatt_db_attr_handle_t cccdHandle, \
    cy_stc_ble_conn_handle_t appConnHandle, uint8_t value)
{
    /* Local variable to store the current CCCD value */
    uint8_t cccdValue[CY_BLE_CCCD_LEN];
    
    /* Load the notification status to the CCCD array */
    cccdValue[CCCD_INDEX_0] = value;
    cccdValue[CCCD_INDEX_1] = CY_BLE_CCCD_DEFAULT;
        
    /* Local variable that stores notification data parameters */
    cy_stc_ble_gatt_handle_value_pair_t  cccdValuePair = 
    {
        .attrHandle = cccdHandle,
        .value.len = CY_BLE_CCCD_LEN,
        .value.val = cccdValue
    };
    
    /* Local variable that stores attribute value */
    cy_stc_ble_gatts_db_attr_val_info_t  cccdAttributeHandle=
    {
        .connHandle = appConnHandle,
        .handleValuePair = cccdValuePair,
        .offset = CY_BLE_CCCD_DEFAULT,
    };
    
    /* Extract flag value from the connection handle - TO BE FIXED*/
    if(appConnHandle.bdHandle == 0u)
    {
        cccdAttributeHandle.flags = CY_BLE_GATT_DB_LOCALLY_INITIATED;
    }
    else
    {
        cccdAttributeHandle.flags = CY_BLE_GATT_DB_PEER_INITIATED;
    }
    
    /* Update the CCCD attribute value per the input parameters */
    Cy_BLE_GATTS_WriteAttributeValueCCCD(&cccdAttributeHandle);
}

/*******************************************************************************
* Function Name: SendRgbNotification
********************************************************************************
* Summary:
*  Send RGB LED data as BLE Notifications. This function updates
*  the notification handle with data and triggers the BLE component to send
*  notification
*
* Parameters:
*  appConnHandle:	Connection handle
*
* Return:
*  void
*
*******************************************************************************/
void SendRgbNotification(cy_stc_ble_conn_handle_t appConnHandle)
{
    /* Make sure that stack is not busy, then send the notification. Note that 
       the number of buffers in the BLE stack that holds the application data 
       payload are limited, and there are chances that notification might drop 
       a packet if the BLE stack buffers are not available. This error condition
       is not handled in this example project */
    if (Cy_BLE_GATT_GetBusyStatus(appConnHandle.attId) 
                                                     == CY_BLE_STACK_STATE_FREE)
    {
        /* Local variable that stores RGB LED notification data parameters */
        cy_stc_ble_gatts_handle_value_ntf_t rgbNotificationHandle = 
        {
            .connHandle = appConnHandle,
            .handleValPair.attrHandle = CY_BLE_RGB_LED_RGB_LED_CONTROL_CHAR_HANDLE,
            .handleValPair.value.val = rgbData,
            .handleValPair.value.len = RGB_DATA_LEN
        };

        /* Send the updated handle as part of attribute for notifications */
        Cy_BLE_GATTS_Notification(&rgbNotificationHandle);
    }
}

/*******************************************************************************
* Function Name: UpdateRgb
********************************************************************************
* Summary:
*  Receive the new RGB data and update the read characteristic handle so that the
*  next read from the BLE central device gives the current RGB color and intensity 
*  data. This function also calls SetColorRgb() function to set the color of the
*  RGB LED per the current value
*
* Parameters:
*  appConnHandle    :   Connection handle
*
* Return:
*  void
*
*******************************************************************************/
void UpdateRgb(cy_stc_ble_conn_handle_t appConnHandle)
{
    /* Local variable that stores RGB control data parameters */
    cy_stc_ble_gatt_handle_value_pair_t  rgbHandle = 
    {
        .attrHandle = CY_BLE_RGB_LED_RGB_LED_CONTROL_CHAR_HANDLE,
        .value.val  = rgbData,
        .value.len  = RGB_DATA_LEN 
    };
    
    /* Local variable that stores RGB attribute value */
    cy_stc_ble_gatts_db_attr_val_info_t  rgbAttributeValue = 
    {
        .handleValuePair = rgbHandle,
        .offset = CY_BLE_CCCD_DEFAULT,
        .connHandle = appConnHandle,
        .flags = CY_BLE_GATT_DB_LOCALLY_INITIATED
    };

    /* Send updated RGB control handle as an attribute to the central device, 
       so that the central reads the new RGB color data */
    Cy_BLE_GATTS_WriteAttributeValue(&rgbAttributeValue);
    
    /* Set the color of the RGB LED to match the current values */
    SetColorRgb(rgbData);
}

/*******************************************************************************
* Function Name: HandleWriteRequestforRgb
********************************************************************************
* Summary:
*  This functions handles the 'write request' event for the RGB LED service
*
* Parameters:
*  writeRequest : pointer to the write request parameters from the central        
*
* Return:
*  void
*
*******************************************************************************/
void HandleWriteRequestforRgb(cy_stc_ble_gatts_write_cmd_req_param_t *writeRequest)
{
    /* Check the validity and then extract the write value sent by the Client 
       for RGB LED CCCD */
    if (writeRequest->handleValPair.attrHandle == RGB_CCCD_HANDLE && 
        ((writeRequest->handleValPair.value.val[RGB_CCCD_INDEX] 
            == CCCD_NOTIFY_DISABLED)||
        (writeRequest->handleValPair.value.val[RGB_CCCD_INDEX]
            == CCCD_NOTIFY_ENABLED)))
    {
        rgbNotificationStatus = writeRequest->
                                    handleValPair.value.val[RGB_CCCD_INDEX] ;
        
         /* Update the corresponding CCCD value in GATT DB */
        UpdateCccdStatusInGattDb(RGB_CCCD_HANDLE, writeRequest->connHandle, rgbNotificationStatus);
    }
    
    /* Check if the notification is enabled for RGB LED service */
    if (rgbNotificationStatus == CCCD_NOTIFY_ENABLED)
    {
        /* Update the RGB LED Notification attribute with new color
           coordinates */
        SendRgbNotification(writeRequest->connHandle);
    }
    
    /* Check if the returned handle is matching to RGB LED Control Write
       Attribute and extract the RGB data*/
    if (writeRequest->handleValPair.attrHandle == 
            CY_BLE_RGB_LED_RGB_LED_CONTROL_CHAR_HANDLE)
    {
        /* Extract the write value sent by the Client for RGB LED Color 
           characteristic */
        rgbData[RED_INDEX] = 
                writeRequest->handleValPair.value.val[RED_INDEX];
        rgbData[GREEN_INDEX] = 
                writeRequest->handleValPair.value.val[GREEN_INDEX];
        rgbData[BLUE_INDEX] = 
                writeRequest->handleValPair.value.val[BLUE_INDEX];
        rgbData[INTENSITY_INDEX] = 
                writeRequest->handleValPair.value.val[INTENSITY_INDEX];

        /* Update the the attribute for RGB LED read characteristics and
           set the color of the LED per the received value */
        UpdateRgb(writeRequest->connHandle);
    }
}

/* [] END OF FILE */
