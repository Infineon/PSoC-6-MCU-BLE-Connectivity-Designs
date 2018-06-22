/*******************************************************************************
* File Name: main_cm0p.c
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

#include "project.h"

int main(void)
{
    cy_en_ble_api_result_t          apiResult;
    
    __enable_irq(); /* Enable global interrupts. */
    
    /* Unfreeze IO if device is waking up from hibernate */
    if(Cy_SysPm_GetIoFreezeStatus())
    {
        Cy_SysPm_IoUnfreeze();
    }
    
    /* Start the Controller portion of BLE. Host runs on the CM4 */
    apiResult = Cy_BLE_Start(NULL);
    
    if(apiResult == CY_BLE_SUCCESS)
    {
        /* Enable CM4 only if BLE Controller started successfully. 
        *  CY_CORTEX_M4_APPL_ADDR must be updated if CM4 memory layout 
        *  is changed. */
        Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR); 
    }
    else
    {
        /* Halt CPU */
        CY_ASSERT(0u != 0u);
    }
   
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    for(;;)
    {
        /* Place your application code here. */
        /* Put CM0p to deep sleep */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
        
        /* Cy_Ble_ProcessEvents() allows BLE stack to process pending events */
        /* The BLE Controller automatically wakes up host if required */
        Cy_BLE_ProcessEvents();        
    }
}

/* [] END OF FILE */
