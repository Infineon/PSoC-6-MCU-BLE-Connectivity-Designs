/******************************************************************************
* File Name: led.c
*
* Version: 1.00
*
* Description: This file contains functions that handle the initialization and
*              control of the RGB LED and the status LEDs
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
/* Header file includes */
#include<project.h>
#include"led.h"
#include "stdio.h"

/* Macros used to scale the RGB color component values so that they fall within
   the linear range of Pseudo Random PWM compare values */
/* Pseudo Random PWMs that drive Red and Green LEDs are configured as 16 bit 
   and the Pseudo Random PWM that drives the Blue LED is configured as 32 bit 
   so that they have direct HSIOM connections to the RGB LED pins on the
   Pioneer kit. Therefore, two different scaling values are used */
#define COLOR_COMPONENT_SCALER_RG  (uint8_t) (0x01u)
#define COLOR_COMPONENT_SCALER_B   (uint8_t) (0x10u)
/* Threshold below which RGB color mixing is skipped */
#define LED_NO_COLOR_THRESHOLD     (uint8_t) (0x04u)

/*******************************************************************************
* Function Name: SetColorRgb
********************************************************************************
* Summary:
*  Changes the RGB LED color to the specified value
*
* Parameters:
*  uint8_t* colorInfo : pointer to a 4-byte array holding the color information
*
* Return:
*  void
*
*******************************************************************************/
void SetColorRgb(uint8_t* colorInfo)
{
    /* Local variables to calculate the color components from the received RGB
       data*/
    uint32_t    redComponent;
    uint32_t    greenComponent;
    uint32_t    blueComponent;

    /* If the Intensity value sent by the client is below a set threshold, then 
	   turn off the LEDs */
    if (colorInfo[INTENSITY_INDEX] < LED_NO_COLOR_THRESHOLD)
    {
        PWMPR_Red_Disable();
        PWMPR_Green_Disable();
        PWMPR_Blue_Disable();
    }
    
    else
    {
        /* If the individual color values of Red, Green and Blue components are 
		   less than the predefined threshold, then turn off the LEDs */
        if ((colorInfo[RED_INDEX] < LED_NO_COLOR_THRESHOLD) &&
            (colorInfo[GREEN_INDEX] < LED_NO_COLOR_THRESHOLD) &&
            (colorInfo[BLUE_INDEX] < LED_NO_COLOR_THRESHOLD))
        {
            PWMPR_Red_Disable();
            PWMPR_Green_Disable();
            PWMPR_Blue_Disable();
        }
        else
        {
            /* Check if the one of the TCPWMs are running. If not, they may not 
               have been initialized or lost their configuration values after a 
    		   low power mode transition, since TCPWM registers are not retained
    		   during system Deep-Sleep power mode */
            if (((Cy_TCPWM_PWM_GetStatus(PWMPR_Red_TCPWM__HW, 
                PWMPR_Red_TCPWM__CNT_IDX)) & TCPWM_CNT_STATUS_RUNNING_Msk) == 0u)
            {
                /* Reconfigure and restart TCPWMs used for RGB LED control */
                InitRgb();
                printf("Re-Init\r\n");
            }
            
            /* Calculate the intensity of the Red, Green and Blue components 
        	   from the received 4-byte data */
            redComponent    = (colorInfo[RED_INDEX] * colorInfo[INTENSITY_INDEX]) >> COLOR_COMPONENT_SCALER_RG;
            greenComponent  = (colorInfo[GREEN_INDEX] * colorInfo[INTENSITY_INDEX]) >> COLOR_COMPONENT_SCALER_RG;
            blueComponent   = (colorInfo[BLUE_INDEX] * colorInfo[INTENSITY_INDEX]) << COLOR_COMPONENT_SCALER_B;

            /* Update the compare value of the PWMs for color control */
            Cy_TCPWM_Counter_SetCompare0(PWMPR_Red_TCPWM__HW,
                                   PWMPR_Red_TCPWM__CNT_IDX, redComponent);
            Cy_TCPWM_Counter_SetCompare0(PWMPR_Green_TCPWM__HW,
                                   PWMPR_Green_TCPWM__CNT_IDX, greenComponent);
            Cy_TCPWM_Counter_SetCompare0(PWMPR_Blue_TCPWM__HW,
                                   PWMPR_Blue_TCPWM__CNT_IDX, blueComponent);
        }
    }
}

/*******************************************************************************
* Function Name: void InitRgb(void)
********************************************************************************
* Summary:
*  Initializes the RGB LED by starting the PWM components
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void InitRgb(void)
{
    /* Start the PWM components for LED control*/
    PWMPR_Red_Start();
    PWMPR_Green_Start();
    PWMPR_Blue_Start();
}

/* [] END OF FILE */
