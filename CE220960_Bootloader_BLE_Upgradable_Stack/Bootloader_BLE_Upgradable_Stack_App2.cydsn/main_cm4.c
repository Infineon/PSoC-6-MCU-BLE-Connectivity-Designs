/*******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.0
*
* Description: This file provides the source code for the Bootloadable (App2)
*              BLE HID Keyboard running on the Core M4 (Core1).
*              App1 Core1 firmware does the following:
*              - Runs the BLE HID Keyboard code example with shared stack from App1.
*
* Related Document: Code example CE220960.pdf
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

#include "common.h"

/* This is placed in signature section to validate application */
CY_SECTION(".cy_app_signature") __USED const uint32_t cy_bootload_appSignature; 


/*******************************************************************************
* Function Name: main()
********************************************************************************
*
* Summary:
*  Main function for the project.
*
*
*******************************************************************************/
int main()
{
    /* Enable global interrupts. */
    __enable_irq();

#if(CY_BLE_CONFIG_HOST_CORE == CY_BLE_CORE_CORTEX_M4)   
#if(!CY_BLE_STACK_MODE_IPC)
    /* Interrupt vector must be manually set in profile only mode */
    Cy_SysInt_SetVector(BLE_bless_isr_cfg.intrSrc, &Cy_BLE_BlessInterrupt);    
#endif /* (CY_BLE_STACK_MODE_IPC) */
    /* Run Host main */
    HostMain();
    
#else
    for(;;)
    {
        /* To achieve low power in the device */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }
#endif /* (CY_BLE_HOST_CORE == CY_CPU_CORTEX_M4) */
}

#if (CY_BLE_CONFIG_HOST_CORE == CY_BLE_CORE_CORTEX_M4)
#if defined (__ARMCC_VERSION)
extern void StackRAMInit(uint32_t);
extern void Reset_Handler(void);
/* Variable used to return to the bootloadable when initializing bootloader RAM */
extern uint32_t CyReturnToBootloaddableAddress;
#elif defined (__ICCARM__)
extern void StackRAMInit(void);
#endif /* __ARMCC_VERSION || __ICCARM__ */

#if defined (__ARMCC_VERSION) || (__ICCARM__)
/*******************************************************************************
* Function Name: Cy_OnResetUser()
********************************************************************************
*
* Summary:
*  This function is called at the start of Reset_Handler(). It is a weak function
*  that may be redefined by user code.
*  This function calls BootloaderRAMInit located in the bootloader to initialize
*  the stack's RAM.
*
*******************************************************************************/
void Cy_OnResetUser(void)
{
    #if defined (__ARMCC_VERSION)
    if(CyReturnToBootloaddableAddress == 0)
    {
        StackRAMInit((uint32_t) Reset_Handler);
    }
    CyReturnToBootloaddableAddress = 0;
    #elif defined (__ICCARM__)
    StackRAMInit();
    #endif /* __ARMCC_VERSION || __ICCARM__ */
}
#endif /* __ARMCC_VERSION || __ICCARM__ */
#endif /* (CY_BLE_CONFIG_HOST_CORE == CY_BLE_CORE_CORTEX_M4) */

/* [] END OF FILE */
