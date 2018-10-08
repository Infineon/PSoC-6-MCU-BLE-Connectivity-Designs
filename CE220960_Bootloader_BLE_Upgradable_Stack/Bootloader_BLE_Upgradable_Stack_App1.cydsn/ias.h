/*******************************************************************************
* File Name: ias.h
*
* Description:
*  Contains the function prototypes and references for the Immediate Alert
*  Service of the Bluetooth Component.
*
********************************************************************************
* Copyright 2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "project.h"


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
