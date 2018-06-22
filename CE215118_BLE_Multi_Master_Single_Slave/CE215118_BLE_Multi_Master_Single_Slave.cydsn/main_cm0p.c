/*******************************************************************************
* File Name: main_cm0p.c
*
* Version: 1.0
*
* Description:
*  This is the source code CM0p core for the BLE project (for dual cores MPN).
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
*
******************************************************************************
* Copyright (2017), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the
* materials described herein. Cypress does not assume any liability arising out
* of the application or use of any product or circuit described herein. Cypress
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*****************************************************************************/


#include "project.h"
#include "common.h"


/*******************************************************************************
* Function Name: main()
********************************************************************************
*
* Summary:
*  Main function for the project.
*
*
*******************************************************************************/
int main(void)
{    

    /* Unfreeze IO after Hibernate */
    if(Cy_SysPm_GetIoFreezeStatus())
    {
        Cy_SysPm_IoUnfreeze();
    }

    /* Enable global interrupts. */
    __enable_irq(); 

#if(CY_BLE_CONFIG_HOST_CORE == CY_BLE_CORE_CORTEX_M0P)   
    
    /* Enable CM4.  CY_CORTEX_M4_APPL_ADDR must be updated if CM4 memory layout is changed. */
    Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR); 
    
    /* Run Host main */
    HostMain();

#else
    
    #if(CY_BLE_STACK_MODE_IPC)
        /* Start BLE Controller for dual core mode */
        Cy_BLE_Start(NULL);
    #endif /* (CY_BLE_STACK_MODE_IPC)*/
    
    /* Enable CM4.  CY_CORTEX_M4_APPL_ADDR must be updated if CM4 memory layout is changed. */
    Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR); 
    
    for(;;)
    {
       
    #if(CY_BLE_STACK_MODE_IPC)
        /* Process BLE events continuously for controller in dual core mode */
        Cy_BLE_ProcessEvents();
        
        /* To achieve low power in the device */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT); 
    #endif /* CY_BLE_STACK_MODE_IPC */
    }
    
#endif /* (CY_BLE_CONFIG_HOST_CORE == CY_BLE_CORE_CORTEX_M0P) */  
}


/* [] END OF FILE */
