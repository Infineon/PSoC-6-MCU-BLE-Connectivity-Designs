/*******************************************************************************
* File Name: ias.h
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project.
*
********************************************************************************
* Copyright 2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>


/***************************************
*          API Constants
***************************************/
#define NO_ALERT                (0u)
#define MILD_ALERT              (1u)
#define HIGH_ALERT              (2u)

#define LED_TOGGLE_TIMEOUT      (100u)


/***************************************
*       Function Prototypes
***************************************/
void IasInit(void);
void IasEventHandler(uint32_t event, void *eventParam);


/***************************************
* External data references
***************************************/
extern volatile uint8_t alertLevel;


/* [] END OF FILE */
