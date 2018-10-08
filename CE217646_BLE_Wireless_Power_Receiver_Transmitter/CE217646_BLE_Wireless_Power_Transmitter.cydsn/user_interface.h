/*******************************************************************************
* File Name: user_interface.h
*
* Version: 1.0
*
* Description:
*  Contains the function prototypes and constants for user interface related source.
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


#include "common.h"


/***************************************
*           API Constants
***************************************/
#define LED_ON                          (0u)
#define LED_OFF                         (1u)
#define RGB_LED_MIN_VOLTAGE_MV          (2700u)
#define SW2_PRESS_TIME_DEL_BOND_LIST    (4u)

/***************************************
*    Function Prototypes
***************************************/
void InitUserInterface(void);
void UpdateLedState(void);
void ProcessUartCommands(char8 command);

/***************************************
*        Macros
***************************************/

/* Set Scanning LED  */
#define Scanning_LED_Write(value)                                           \
    do {                                                                    \
        Cy_GPIO_Write(Scanning_LED_0_PORT, Scanning_LED_0_NUM, value);      \
    } while(0u)

#define Scanning_LED_INV()    Cy_GPIO_Inv(Scanning_LED_0_PORT, Scanning_LED_0_NUM)


/* Read SW2 pin */
#define SW2_Read()    Cy_GPIO_Read(SW2_0_PORT, SW2_0_NUM)

/* Leds operations */
#define EnableAllLeds()                \
    do {                               \
        Scanning_LED_Write(LED_ON);    \
    } while(0u)

#define DisableAllLeds()               \
    do{                                \
        Scanning_LED_Write(LED_OFF);   \
    } while(0u)

/* [] END OF FILE */
