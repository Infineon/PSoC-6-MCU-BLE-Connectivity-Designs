/******************************************************************************
* File Name: rgb_led.c
*
* Version: 1.0
*
* Description: This file contains the task that that control the RGB LED
*
* Related Document: CE223508_PSoC6_BLE_FourSlaves_RTOS.pdf
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
*
*******************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation. All rights reserved.
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
/******************************************************************************
* This file contains the tasks that that control the RGB LED color and intensity
*******************************************************************************/

/* Header file includes */
#include "rgb_led_task.h"
#include "uart_debug.h"
#include "uart_debug.h"
#include "task.h"  
#include "timers.h" 

/**
 * Macros used to scale the RGB color component values so that they fall within 
 * the linear range of Pseudo Random PWM compare values 
 */

/**
 * Pseudo Random PWMs that drive Red and Green LEDs are configured as 16 bit 
 * and the Pseudo Random PWM that drives the Blue LED is configured as 32 bit 
 * so that they have direct HSIOM connections to the RGB LED pins on the 
 * Pioneer kit. Therefore, two different scaling values are used 
 */
#define COLOR_COMPONENT_SCALER_RG  (uint8_t) (0x01u)
#define COLOR_COMPONENT_SCALER_B   (uint8_t) (0x10u)
/* Threshold below which RGB color mixing is skipped */
#define LED_NO_COLOR_THRESHOLD     (uint8_t) (0x04u)

/**
 * Macros that extract Red, Green, Blue and the intensity information bytes 
 * from a 32-bit (true color) word in RGBA color space format. 
 * Intensity value is stored in the Alpha-channel byte  
 */
#define GET_RED(x)       (uint8_t)((x)        & 0x000000FFu)
#define GET_GREEN(x)     (uint8_t)((x >> 8u)  & 0x000000FFu)
#define GET_BLUE(x)      (uint8_t)((x >> 16u) & 0x000000FFu)
#define GET_INTENSITY(x) (uint8_t)((x >> 24u) & 0x000000FFu)

/* Handle for the Queue that contains RGB LED data */   
QueueHandle_t rgbLedDataQ;

/*******************************************************************************
* Function Name: void Task_RgbLed (void *pvParameters)
********************************************************************************
* Summary:
*  Task that controls the color and intensity of the RGB LED   
*
* Parameters:
*  void *pvParameters : Task parameter defined during task creation (unused)                            
*
* Return:
*  void
*
*******************************************************************************/
void Task_RgbLed (void *pvParameters)
{ 
    /**
     * Variable that stores a 32-bit (true color) word in RGBA color space 
     * format. Intensity value is stored in the Alpha-channel byte. 
     */
    uint32_t colorAndIntensity;
    
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    /* Remove warning for unused parameter */
    (void)pvParameters ;

    /* Repeatedly running part of the task */
    for(;;)
    {
        /* Block until RGB data has been received over rgbLedDataQ */
        rtosApiResult = xQueueReceive(rgbLedDataQ, &colorAndIntensity,
                        portMAX_DELAY);
        
        /* RGB data has been received from rgbLedDataQ */
        if(rtosApiResult == pdTRUE)
        {
            /**
             * If the Intensity value sent by the client is below the set  
             * threshold or if the individual color values of Red, Green and  
             * Blue components are below the threshold, turn off all the LEDs 
             */
            if ((GET_INTENSITY(colorAndIntensity) < LED_NO_COLOR_THRESHOLD)||
               ((GET_BLUE(colorAndIntensity)      < LED_NO_COLOR_THRESHOLD)&&
                (GET_GREEN(colorAndIntensity)     < LED_NO_COLOR_THRESHOLD)&&
                (GET_RED(colorAndIntensity)       < LED_NO_COLOR_THRESHOLD)))
            {           
                PWMPR_Red_Disable();
                PWMPR_Green_Disable();
                PWMPR_Blue_Disable();
            }
            /* Intensity and the color component values are above threshold */
            else
            {
                /**
                 * Check if the one of the TCPWMs are running. If not, they may
                 * not have been initialized or lost their configuration values
                 * during a power mode transition 
                 */
                if (((Cy_TCPWM_PWM_GetStatus(PWMPR_Red_TCPWM__HW,
                        PWMPR_Red_TCPWM__CNT_IDX))
                        &TCPWM_CNT_STATUS_RUNNING_Msk) == 0u)
                {
                    /* Reconfigure and restart TCPWMs used for RGB LED control */
                    PWMPR_Red_Start();
                    PWMPR_Green_Start();
                    PWMPR_Blue_Start();
                }

                /**
                 * Update the compare values of the TCPWMs according to the color
                 * and intensity values received 
                 */
                Cy_TCPWM_Counter_SetCompare0(PWMPR_Red_TCPWM__HW, 
                                             PWMPR_Red_TCPWM__CNT_IDX, 
                                             ((GET_RED(colorAndIntensity) * 
                                             GET_INTENSITY(colorAndIntensity)) 
                                             >> COLOR_COMPONENT_SCALER_RG));
                Cy_TCPWM_Counter_SetCompare0(PWMPR_Green_TCPWM__HW, 
                                             PWMPR_Green_TCPWM__CNT_IDX,
                                             ((GET_GREEN(colorAndIntensity) *
                                             GET_INTENSITY(colorAndIntensity))
                                             >> COLOR_COMPONENT_SCALER_RG));
                Cy_TCPWM_Counter_SetCompare0(PWMPR_Blue_TCPWM__HW, 
                                             PWMPR_Blue_TCPWM__CNT_IDX,
                                             ((GET_BLUE(colorAndIntensity) * 
                                             GET_INTENSITY(colorAndIntensity)) 
                                             << COLOR_COMPONENT_SCALER_B ));
            }
        }
        /**
         * Task has timed out and received no RGB data during an interval of
         * portMAXDELAY ticks 
         */
        else
        {
            Task_DebugPrintf("Warning! : RGB - Task Timed out ", 0u);   
        }
    }  
}

/*******************************************************************************
* Function Name: Task_RgbLed_Tickless_Idle_Readiness(void)
********************************************************************************
* Summary:
*  This function returns the Tickless Idle readiness of Task_RgbLed  
*
* Parameters:
*  None                          
*
* Return:
*  bool: Tickless Idle readiness of Task_RgbLed
*
*******************************************************************************/
bool Task_RgbLed_Tickless_Idle_Readiness(void)  
{
    /* Return "false" if one of TCPWMs are running, "true" otherwise */
    return ((Cy_TCPWM_PWM_GetStatus(PWMPR_Red_TCPWM__HW, PWMPR_Red_TCPWM__CNT_IDX)
             &TCPWM_CNT_STATUS_RUNNING_Msk) == 0u) ? true : false;
};

/* [] END OF FILE */
