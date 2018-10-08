/*******************************************************************************
* File Name: main_cm0p.c
*
* Version: 1.0
*
* Description: This file provides the source code for the Bootloader (App0)
*              running on the Core M0+ (Core0).
*              App0 Core0 firmware does the following:
*              - Starts App0 Core1 firmware.
*              - Switches to the Bootloadable (App1) on reset
*                if it was scheduled.
*
* Related Document: Code example CE220959.pdf
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

#include "bootloader/cy_bootload.h"
#include "project.h"

#if CY_BOOTLOAD_OPT_CRYPTO_HW != 0
    #include "cy_crypto_config.h"
    cy_stc_crypto_server_context_t cryptoServerContext;
#endif

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Main function of App0 Core0. Unfreezes IO and sets up the user button (SW2)
*  as the hibernate wakeup source. Afterwards initializes Core1 (CM4) and goes 
*  into deep sleep.
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
    /* Unfreeze IO after Hibernate */
    if(Cy_SysPm_GetIoFreezeStatus())
    {
        Cy_SysPm_IoUnfreeze();
    }
    /* Set SW2 as hibernate wakeup pin */
    Cy_SysPm_SetHibWakeupSource(CY_SYSPM_HIBPIN1_LOW);
    
    /* enable global interrupts */
    __enable_irq();
    
#if CY_BOOTLOAD_OPT_CRYPTO_HW != 0
    /* Start the Crypto Server */
    Cy_Crypto_Server_Start(&cryptoConfig, &cryptoServerContext);
#endif

    /* Enable CM4 with the CM4 start address defined in the
       Bootloader SDK linker script */
    Cy_SysEnableCM4( (uint32_t)(&__cy_app_core1_start_addr) );

    for (;;)
    {
        /* Go into Deep Sleep */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }
}

/*******************************************************************************
* Function Name: Cy_OnResetUser
********************************************************************************
*
* Summary:
*  This function is called at the start of Reset_Handler(). It is a weak function
*  that may be redefined by user code.
*  Bootloader SDK requires it to call Cy_Bootload_OnResetApp0().
*  Checks if an App switch has been scheduled and transfers control to it.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Cy_OnResetUser(void)
{
    Cy_Bootload_OnResetApp0();
}

/* [] END OF FILE */
