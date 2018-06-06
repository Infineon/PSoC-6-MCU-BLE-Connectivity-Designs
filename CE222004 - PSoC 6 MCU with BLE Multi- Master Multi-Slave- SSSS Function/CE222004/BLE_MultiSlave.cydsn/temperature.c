/******************************************************************************
* File Name: temperature.c
*
* Version: 1.00
*
* Description: This file contains functions that handle the initialization
*              of ADCs and temperature calculation.
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
*
******************************************************************************
* Copyright (2017-2018), Cypress Semiconductor Corporation.
******************************************************************************
* This software, including source code, documentation and related materials
* ("Software") is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and 
* foreign), United States copyright laws and international treaty provisions. 
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the 
* Cypress source code and derivative works for the sole purpose of creating 
* custom software in support of licensee product, such licensee product to be
* used only in conjunction with Cypress's integrated circuit as specified in the
* applicable agreement. Any reproduction, modification, translation, compilation,
* or representation of this Software except as specified above is prohibited 
* without the express written permission of Cypress.
* 
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes to the Software without notice. 
* Cypress does not assume any liability arising out of the application or use
* of Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use as critical components in any products 
* where a malfunction or failure may reasonably be expected to result in 
* significant injury or death ("ACTIVE Risk Product"). By including Cypress's 
* product in a ACTIVE Risk Product, the manufacturer of such system or application
* assumes all risk of such use and in doing so indemnifies Cypress against all
* liability. Use of this Software may be limited by and subject to the applicable
* Cypress software license agreement.
*****************************************************************************/
#include <math.h>
#include "temperature.h"

/* Refernce resistor in series with the thermistor is of 10 KOhm */
#define R_REFERENCE     (float)(10000)

/* Beta constant of this thermistor is 3380 Kelvin. See the thermistor
   (NCP18XH103F03RB) datasheet for more details. */
#define B_CONSTANT      (float)(3380)

/* Resistance of the thermistor is 10K at 25 degrees C (from datasheet)
   Therefore R0 = 10000 Ohm, and T0 = 298.15 Kelvin, which gives
   R_INFINITY = R0 e^(-B_CONSTANT / T0) = 0.1192855 */
#define R_INFINITY      (float)(0.1192855)

/* Zero Kelvin in degree C */
#define ABSOLUTE_ZERO   (float)(-273.15)

/*******************************************************************************
* Function Name: InitTemperature
********************************************************************************
* Summary:
*  This functions intializes the ADC and put it to sleep so that entering low
*  power modes won't affect the ADC configuration.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void InitTemperature(void)
{
    ADC_Start();
    ADC_Sleep();
}

/*******************************************************************************
* Function Name: GetTemperature
********************************************************************************
* Summary:
*  This functions calculates the thermistor resistance and the corresponding 
*  temperature value.
*
* Parameters:
*  None
*
* Return:
*  float : temperature value
*
*******************************************************************************/
float GetTemperature(void)
{   
    /* Variables used to store ADC counts, thermistor resistance and
       the temperature */
    int16_t countThermistor, countReference;
    float rThermistor, temperature;
    
    /* Set the GPIO that drives the thermistor circuit */
    Cy_GPIO_Set(THER_VDD_0_PORT, THER_VDD_0_NUM);
    
    /* Wake up the ADC and start conversion */
    ADC_Wakeup();
    ADC_StartConvert();
    ADC_IsEndConversion(CY_SAR_WAIT_FOR_RESULT);
    
    /* Read the ADC count values */
    countReference  = ADC_GetResult16(REF_CHANNEL);
    countThermistor = ADC_GetResult16(THER_CHANNEL);
   
    /* Put the ADC to sleep so that entering low power modes won't affect
       the ADC configuration */
    ADC_Sleep();
    
    /* Clear the GPIO that drives the thermistor circuit, to save power */
    Cy_GPIO_Clr(THER_VDD_0_PORT, THER_VDD_0_NUM);
    
    /* Calculate the thermistor resistance and the corresponding temperature */
    rThermistor = (R_REFERENCE * countThermistor) / countReference;    
    temperature = (B_CONSTANT / (logf(rThermistor / R_INFINITY))) + ABSOLUTE_ZERO;
    
    /* Return the temperature value */
    return temperature;
}

/* [] END OF FILE */
