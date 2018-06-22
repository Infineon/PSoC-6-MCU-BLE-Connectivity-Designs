/*******************************************************************************
* File Name: common.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants for the example
*  project.
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(MAIN_H)
#define MAIN_H

#include <project.h>
    
/***************************************
*        API Constants
***************************************/

#define ADV_TIMER_TIMEOUT       (1u)             /* */
    
/***************************************
*        Function Prototypes
***************************************/
int HostMain(void);
void AppCallBack(uint32_t event, void* eventParam);
void LowPowerImplementation(void);

#endif /* MAIN_H */

/* [] END OF FILE */
