/*******************************************************************************
* File Name: debug.c
*
* Version: 1.0
*
* Description:
*  This file contains functions for LED status notification.
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
* 
********************************************************************************
* Copyright 2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "debug.h"

#if (DEBUG_LED_ENABLED == ENABLED)

void InitLED(void)
{
    #if CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV
    Cy_GPIO_SetDrivemode(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM, CY_GPIO_DM_STRONG_IN_OFF);
    Cy_GPIO_Write(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM, LED_OFF);  
    Cy_GPIO_SetDrivemode(PIN_LED_GREEN_0_PORT, PIN_LED_GREEN_0_NUM, CY_GPIO_DM_STRONG_IN_OFF);
    Cy_GPIO_Write(PIN_LED_GREEN_0_PORT, PIN_LED_GREEN_0_NUM, LED_OFF);    
    Cy_GPIO_SetDrivemode(PIN_LED_BLUE_0_PORT, PIN_LED_BLUE_0_NUM, CY_GPIO_DM_STRONG_IN_OFF);
    Cy_GPIO_Write(PIN_LED_BLUE_0_PORT, PIN_LED_BLUE_0_NUM, LED_OFF);
    #else
    Cy_GPIO_SetDrivemode(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM, CY_GPIO_DM_STRONG_IN_OFF); 
    Cy_GPIO_Write(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM, LED_OFF);    
    #endif  
}

void HibernateLED(void)
{
    #if CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV
    Cy_GPIO_Write(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM, LED_ON);    
    Cy_GPIO_Write(PIN_LED_GREEN_0_PORT, PIN_LED_GREEN_0_NUM, LED_OFF);    
    Cy_GPIO_Write(PIN_LED_BLUE_0_PORT, PIN_LED_BLUE_0_NUM, LED_OFF);
    #else
    Cy_GPIO_Write(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM, LED_ON);    
    #endif
}

void BlinkLED(void)
{
    #if CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV
    Cy_GPIO_Inv(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM);    
    Cy_GPIO_Inv(PIN_LED_GREEN_0_PORT, PIN_LED_GREEN_0_NUM);    
    Cy_GPIO_Inv(PIN_LED_BLUE_0_PORT, PIN_LED_BLUE_0_NUM);
    #else
    Cy_GPIO_Inv(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM);    
    #endif  
}

void ConnectedLED(void)
{
    #if CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV
    Cy_GPIO_Write(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM, LED_OFF);    
    Cy_GPIO_Write(PIN_LED_GREEN_0_PORT, PIN_LED_GREEN_0_NUM, LED_OFF);    
    Cy_GPIO_Write(PIN_LED_BLUE_0_PORT, PIN_LED_BLUE_0_NUM, LED_OFF);
    #else
    Cy_GPIO_Write(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM, LED_OFF);    
    #endif    
}

void WarningLED(void)
{
    Cy_GPIO_Inv(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM);
}

#endif /* DEBUG_LED == ENABLED */

/* [] END OF FILE */
