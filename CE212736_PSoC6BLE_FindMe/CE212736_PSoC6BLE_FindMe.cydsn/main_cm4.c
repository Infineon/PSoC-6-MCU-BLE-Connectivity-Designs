/*******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.20
*
* Description:
*   This is source code for the PSoC 6 MCU with BLE Find Me code example.
*
* Note:
*
* Owners:
*   snvn@cypress.com
*
* Related Documents:
*   AN210781 - Getting Started with PSoC 6 MCU with 
*              Bluetooth Low Energy BLE) Connectivity
*   CE212736 - PSoC 6 MCU with Bluetooth Low Energy 
*              (BLE) Connectivity - Find Me
*
* Hardware Dependency:
*  1. PSoC 6 MCU with BLE device
*  2. CY8CKIT-062-BLE Pioneer Kit
*
* Code Tested With:
*  1. PSoC Creator 4.2
*   a. ARM GCC
*   b. ARM MDK
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>
#include "BLEFindMe.h"

/*******************************************************************************
* Function Name: main()
********************************************************************************
* Summary:
*  Main function for the project.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Theory:
*  The main function initializes the PSoC 6 BLE device and runs a BLE process.
*
*******************************************************************************/
int main(void)
{
    /* Initialize BLE */
    BleFindMe_Init();

    __enable_irq(); /* Enable global interrupts. */    
    
    for(;;) 
    {   
        BleFindMe_Process();
	}   
} 

/* [] END OF FILE */
