/*******************************************************************************
* File Name: lls.h
*
*  Version 1.0
*
* Description:
*  This file contains routines related for the Link Loss Service.
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include "common.h"

/***************************************
*       Function Prototypes
***************************************/
void LlsInit(void);
void LlsServiceAppEventHandler(uint32_t event, void *eventParam);
uint8_t LlsGetAlertLevel(void);
void LlsSetAlertLevel(uint8_t alertLevelValue);


/* [] END OF FILE */
