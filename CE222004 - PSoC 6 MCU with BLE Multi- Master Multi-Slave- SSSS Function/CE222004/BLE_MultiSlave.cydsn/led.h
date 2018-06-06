/******************************************************************************
* File Name: led.h
*
* Version: 1.00
*
* Description: This file is the public interface of led.c source file 
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
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
#ifndef LED_H
#define LED_H

/* Header file includes */
#include <project.h>
    
/* For more details on the data format of RGB LED required for Cypress's Custom 
   RGB LED profile, see Cypress RGB LED profile specification available at:
    
   http://www.cypress.com/documentation/software-and-drivers/cypresss-custom-ble-profiles-and-services
*/
    
/* Read and write length of the RGB LED data */
#define RGB_DATA_LEN    (uint8_t) (0x04u)

/* Macros used to control the individual status LEDs */   
#define TurnOnOrange    Cy_GPIO_Write(Pin_LED_Orange_0_PORT,\
                        Pin_LED_Orange_0_NUM, 0)
#define TurnOnRed       Cy_GPIO_Write(Pin_LED_Red_0_PORT,\
                        Pin_LED_Red_0_NUM, 0)
#define TurnOffOrange   Cy_GPIO_Write(Pin_LED_Orange_0_PORT,\
                        Pin_LED_Orange_0_NUM, 1)
#define TurnOffRed      Cy_GPIO_Write(Pin_LED_Red_0_PORT,\
                        Pin_LED_Red_0_NUM, 1)
#define InvertOrange    Cy_GPIO_Inv(Pin_LED_Orange_0_PORT,\
                        Pin_LED_Orange_0_NUM)

/* Respective indexes of the color coordinates stored in the 4-byte RGB LED control
   characteristic */
#define RED_INDEX       (uint8_t) (0x00u)
#define GREEN_INDEX     (uint8_t) (0x01u)
#define BLUE_INDEX      (uint8_t) (0x02u)
#define INTENSITY_INDEX (uint8_t) (0x03u)

/* TCPWM compare value for the LED OFF state */
#define RGB_LED_OFF     (uint16_t) (0x0000u)

/***************************************
*        External Function Prototypes
***************************************/
    
/* Function that controls the color of the RGB LED */
void SetColorRgb(uint8_t* colorInfo);

/* Function that initialize RGB LEDs */
void InitRgb(void);
#endif/* LED_H */

/* [] END OF FILE */
