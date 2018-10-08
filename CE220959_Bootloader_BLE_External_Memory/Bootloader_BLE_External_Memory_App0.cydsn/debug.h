/***************************************************************************//**
* \file debug.h
*
* \version 1.0
*
*  Contains the function prototypes and constants for LED status notification.
*
********************************************************************************
* Copyright 2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <stdio.h>
#include "project.h"

#define ENABLED                     (1u)
#define DISABLED                    (0u)

/***************************************
* Conditional Compilation Parameters
***************************************/
#define DEBUG_LED_ENABLED           ENABLED

/***************************************
*           API Constants
***************************************/
#define LED_ON                          (0u)
#define LED_OFF                         (1u)
#define RGB_LED_MIN_VOLTAGE_MV          (2700u)
#define ADV_TIMER_TIMEOUT               (1u)

/***************************************
*        External Function Prototypes
***************************************/
#if (DEBUG_LED_ENABLED)

void InitLED(void);
void HibernateLED(void);
void BlinkLED(void);
void ConnectedLED(void);
void WarningLED(void);

#else

#define InitLED()
#define HibernateLED()
#define BlinkLED()
#define ConnectedLED()
#define WarningLED()
    
#endif

/* [] END OF FILE */
