/******************************************************************************
* File Name: rgb_led_service.h
*
* Version: 1.00
*
* Description: This file contains macros and the declaration of functions provided 
*              by the rgb_led_service.c file 
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
/* Include guard */
#ifndef RGB_LED_SERVICE_H 
#define RGB_LED_SERVICE_H 

/* Header file includes */
#include "project.h"
#include "led.h"
    
/* Macros used for RGB Service */
/* Redefinition of long CCCD handles and indexes for better readability */
#define RGB_CCCD_HANDLE         \
(CY_BLE_RGB_LED_RGB_LED_CONTROL_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
#define RGB_CCCD_INDEX          \
(CY_BLE_RGB_LED_RGB_LED_CONTROL_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_INDEX)

/* Bit mask for the notification bit in CCCD (Client Characteristic Configuration 
   Descriptor), which is written by the client device */
#define CCCD_NOTIFY_ENABLED     (uint8_t) (0x01u)
#define CCCD_NOTIFY_DISABLED    (uint8_t) (0x00u)

/* Indexes of a two-byte CCCD array */
#define CCCD_INDEX_0            (uint8_t) (0x00u)
#define CCCD_INDEX_1            (uint8_t) (0x01u)

/* Variable used for notification status */
extern uint8_t rgbNotificationStatus;

/* Variable used to store RGB Service data */
extern uint8_t rgbData[RGB_DATA_LEN];

/***************************************
*        Function Prototypes
***************************************/
void HandleWriteRequestforRgb(cy_stc_ble_gatts_write_cmd_req_param_t*);
void UpdateCccdStatusInGattDb(cy_ble_gatt_db_attr_handle_t , \
                                            cy_stc_ble_conn_handle_t , uint8_t);
void SendRgbNotification(cy_stc_ble_conn_handle_t );
void UpdateRgb(cy_stc_ble_conn_handle_t);

#endif

/* [] END OF FILE */
