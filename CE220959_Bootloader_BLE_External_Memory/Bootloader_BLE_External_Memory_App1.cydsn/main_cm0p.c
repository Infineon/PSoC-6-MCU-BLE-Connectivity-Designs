/*******************************************************************************
* File Name: main_cm0p.c
*
* Version: 1.0
*
* Description: This file provides the source code for the Bootloadable (App1)
*              BLE HID Keyboard running on the Core M0+ (Core0).
*              App1 Core0 firmware does the following:
*              - Starts App1 Core1 firmware.
*              - Hosts the complete BLE component.
*
* Related Document: Code example CE220959.pdf
*                   Code example CE215121.pdf
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
*
******************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.
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

#include "bootloader/cy_bootload.h"
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
    
    /* Enable CM4 with the CM4 start address defined in the
       Bootloader SDK linker script */
    Cy_SysEnableCM4( (uint32_t)(&__cy_app_core1_start_addr) );
    
    /* Run Host main */
    HostMain();

#else
    
    #if(CY_BLE_STACK_MODE_IPC)
        /* Start BLE Controller for dual core mode */
        Cy_BLE_Start(NULL);
    #endif /* (CY_BLE_STACK_MODE_IPC)*/
    
    /* Enable CM4 with the CM4 start address defined in the
       Bootloader SDK linker script */
    Cy_SysEnableCM4( (uint32_t)(&__cy_app_core1_start_addr) ); 
    
    for(;;)
    {
       
    #if(CY_BLE_STACK_MODE_IPC)
        /* Process BLE events continuously for controller in dual core mode */
        Cy_BLE_ProcessEvents();
        
        /* To achieve low power in the device */
    #endif /* CY_BLE_STACK_MODE_IPC */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT); 
    }
    
#endif /* (CY_BLE_HOST_CORE == CY_CPU_CORTEX_M0P) */  
}


/* [] END OF FILE */
