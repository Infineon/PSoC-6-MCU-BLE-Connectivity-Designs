/*******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.0
*
* Description: This file provides the source code for the Bootloader (App1)
*              running on the Core M4 (Core1).
*
*******************************************************************************
* Related Document: CE220960.pdf
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
*                      CY5677 CySmart USB Dongle
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
*******************************************************************************/

#include "project.h"
#include "debug.h"

/* This is placed in the checksum section to validate the application */
CY_SECTION(".cy_app_signature") __USED static const uint32_t cy_bootload_appSignature;

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Main function of App1 Core1. Runs the bootloader if the BLE component is
*  configured in dual core mode or single core on CM4. Else goes into deep sleep.
*
* Parameters:
*  None
* 
* Return:
*  None
*
*******************************************************************************/
int main(void)
{
    /* enable global interrupts */
    __enable_irq();
    
#if(CY_BLE_CONFIG_HOST_CORE == CY_BLE_CORE_CORTEX_M4)   
    /* Run Bootloader */
    BootloaderMain();
#else
    for(;;)
    {
        /* To achieve low power in the device */
    	Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }  
#endif /* (CY_BLE_HOST_CORE == CY_CPU_CORTEX_M0P) */
}

#if (CY_BLE_CONFIG_HOST_CORE == CY_BLE_CORE_CORTEX_M4)
#if defined(__ARMCC_VERSION)
/* Variable used to return to the bootloadable when initializing bootloader RAM */
CY_SECTION(".cy_boot_noinit") __attribute__((zero_init)) __USED static uint32_t CyReturnToBootloaddableAddress;

/* RAM Initialization routine for MDK Compiler */
extern void __main(void);

/*******************************************************************************
* Function Name: CyReturnToBootloaddable
********************************************************************************
*
* Summary:
*  ASM function which branches to appAddr. Used to return to the bootloadable.
*
* Parameters:
*  appAddr: Address to branch to.
*
*******************************************************************************/
__asm static void CyReturnToBootloaddable(uint32 appAddr)
{
    BX  R0
    ALIGN
}

/*******************************************************************************
* Function Name: StackRAMInit
********************************************************************************
*
* Summary:
*  Function called by the bootloadable (App2) to initialize the Stack's RAM
*  Stores return address in CyReturnToBootloaddableAddress to return
*  after the RAM has been initialized.
*
* Parameters:
*  ReturnAddress: Address to return to from the bootloadable.
* 
* Return:
*  None
*
*******************************************************************************/
__USED void StackRAMInit(uint32_t ReturnAddress)
{
    /* Store address to return to after initializing RAM */
    CyReturnToBootloaddableAddress = ReturnAddress;
    /* Initialize RAM */
    __main();
}

/*******************************************************************************
* Function Name: _platform_pre_stackheap_init
********************************************************************************
*
* Summary:
*  Weak function from ARM Lib. This function is called after initializing the
*  RAM and before initializing the Stack and Heap.
*  Function checks if there is an address stored in CyReturnToBootloaddableAddress
*  and branches to this address if there is using CyReturnToBootloaddable.
*  This can only happen if the function BootloaderRAMInit was called.
*
* Parameters:
*  None
* 
* Return:
*  None
*
*******************************************************************************/
void _platform_pre_stackheap_init(void)
{
    /* Check if it is required to return to the bootloadable */
    if(CyReturnToBootloaddableAddress != 0)
    {
        CyReturnToBootloaddable(CyReturnToBootloaddableAddress);
    }
    return;
}
#elif defined (__ICCARM__)
    
/* RAM Initialization routine for IAR Compiler */
extern void __iar_data_init3(void);
    
/*******************************************************************************
* Function Name: StackRAMInit
********************************************************************************
*
* Summary:
*  Function called by the bootloadable (App2) to initialize the Stack's RAM.
*
* Parameters:
*  None
* 
* Return:
*  None
*
*******************************************************************************/
__root void StackRAMInit(void)
{
    /* Initialize RAM */
    __iar_data_init3();
    return;
}
    
#endif /* __ARMCC_VERSION || __ICCARM__ */
#endif /* (CY_BLE_CONFIG_HOST_CORE == CY_BLE_CORE_CORTEX_M4) */

/* [] END OF FILE */
