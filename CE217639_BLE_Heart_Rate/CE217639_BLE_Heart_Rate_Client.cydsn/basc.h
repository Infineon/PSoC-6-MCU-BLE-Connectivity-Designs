/*******************************************************************************
* File Name: basc.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project.
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"


/***************************************
*       Function Prototypes
***************************************/
void BasInit(void);
void BasCallBack(uint32_t event, void *eventParam);


/***************************************
* External data references
***************************************/
extern uint16_t basNotification;


/* [] END OF FILE */
