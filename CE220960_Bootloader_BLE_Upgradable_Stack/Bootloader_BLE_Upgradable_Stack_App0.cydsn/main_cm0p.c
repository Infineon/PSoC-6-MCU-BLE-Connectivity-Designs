/*******************************************************************************
* File Name: main_cm0p.c
*
* Version: 1.0
*
* Description: This file provides the source code for the Launcher (App0)
*              running on the Core M0 (Core0).
*              App0 Core0 firmware does the following:
*              - Checks if the copy flag is set.
*                If set, copies the stack upgrade from the temporal location
*                into the correct region (App1).
*              - Else check if App2 or App1 is valid, and launches it if it is.
*              - If no valid App is present then halt. (Should never happen)
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
#include <string.h>

#if defined (__GNUC__) || defined (__ARMCC_VERSION)
/* Flag which signals a Stack update is available and must be copied */
CY_SECTION(".cy_boot_copy") static const volatile uint8_t cy_bootload_copyFlag = 0;
#elif defined (__ICCARM__)
CY_SECTION(".cy_boot_copy") __root static const uint8_t cy_bootload_copyFlag = 0;
#else
 #error "Compiler not supported."
#endif /* __GNUC__ || __ARMCC_VERSION || __ICCARM__ */

#if defined(__ARMCC_VERSION)
/* Variable used to return to the bootloadable when initializing bootloader RAM */
CY_SECTION(".cy_boot_noinit") __USED static uint32_t CyReturnToBootloaddableAddress;
#endif /* __ARMCC_VERSION */

/* Local function declarations */
static bool IsButtonPressed(uint16_t timeoutInMilis);
static cy_en_bootload_status_t CopyRow(uint32_t dest, uint32_t src, uint32_t rowSize, cy_stc_bootload_params_t * params);
static cy_en_bootload_status_t HandleMetadata(cy_stc_bootload_params_t *params);

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Main function of App0 Core0. Checks copy flag, copies stack update to App1 if set
*  else start App1 or App2 depending on if the application is valid and the status
*  of the user button.
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
    /* Allocate buffer */
    cy_stc_bootload_params_t bootParams;
    CY_ALIGN(4) uint8_t buffer[CY_BOOTLOAD_SIZEOF_DATA_BUFFER];
    bootParams.dataBuffer = &buffer[0];
    
    /* Stores which application will be launched */
    uint8_t validApp = 0u;
    
    /* Enable global interrupts. */
    __enable_irq(); 
    
    /* Enable CM4 Core */
    Cy_SysEnableCM4( (uint32_t)&__cy_app_core1_start_addr );
    
    /* Check metadata, restore with metadata copy row if invalid  */
    HandleMetadata(&bootParams);
    
    /* Check if copy flag has been set */
    if(cy_bootload_copyFlag != 0)
    {
        /* Application was already verified by the bootloader. Only needs to be copied */
        cy_en_bootload_status_t status;
        uint32_t srcAddress;
        uint32_t srcLength;
        uint32_t copyLength;
        uint32_t destAddress;
        /*
        * Stack update temporal address is stored at vApp3 metadata.
        * Stack update destination address and size is stored at vApp4 metadata.
        */
        Cy_Bootload_GetAppMetadata(3u, &srcAddress, NULL);
        Cy_Bootload_GetAppMetadata(4u, &destAddress, &srcLength);
        /* Make sure App signature is copied */
        copyLength = srcLength + CY_BOOTLOAD_SIGNATURE_SIZE;

        /* Turn LED purple, signaling launcher is copying the app */
        Cy_GPIO_Write(PIN_LED_RED_PORT, PIN_LED_RED_NUM, 0u);
        Cy_GPIO_Write(PIN_LED_BLUE_PORT, PIN_LED_BLUE_NUM, 0u);        
        
        /* Copy Stack update to proper location */
        status = Cy_Bootload_CopyApp(destAddress, srcAddress, copyLength, CY_FLASH_SIZEOF_ROW, &bootParams);
        
        if(status == CY_BOOTLOAD_SUCCESS)
        {
            /* 
            *  Copy operation was successful, now update metadata.
            *  If a power loss occurs during metadata update, the
            *  metadata copy row is used as a back-up.
            */
            status = Cy_Bootload_SetAppMetadata(1u, destAddress, srcLength, &bootParams);
            if(status == CY_BOOTLOAD_SUCCESS)
            {
                /* Clear the Copy Flag as the stack was successfully updated */
                Cy_Flash_EraseRow((uint32_t) &cy_bootload_copyFlag);
                
                /* Update metadata copy row */
                HandleMetadata(&bootParams);
                    
                /* Schedule an App switch to the bootloader */
                validApp = 1u;
            }
        }
        /*
        * Else: copy operation failed, handle error here. In this
        * code example, we do nothing, resulting in a breakpoint.
        */
    }
    else
    {
        /* 
        *  Validate App1. App2 requires a valid App1 because of code sharing.
        *  Modify this code if App2 can work independently from App1.
        */
        if((Cy_Bootload_ValidateApp(1u, NULL) == CY_BOOTLOAD_SUCCESS))
        {
            /* Check if button is pressed, validate App2 if it is not */
            if(!IsButtonPressed(1000u))
            {
                if((Cy_Bootload_ValidateApp(2u, NULL) == CY_BOOTLOAD_SUCCESS))
                {
                    validApp = 2u;
                }
                else
                {
                    /* App2 is not valid. Launch App1. */
                    validApp = 1u;
                }
            }
            else
            {
                validApp = 1u;
            }
        }
        /* Else: No valid App. */
    }

    if(validApp != 0u)
    {
        /*
        * Clear reset reason because Cy_Bootload_ExecuteApp() performs
        * a software reset.
        * Without clearing two reset reasons would be present.
        */
        do
        {
            Cy_SysLib_ClearResetReason();
        }while(Cy_SysLib_GetResetReason() != 0);
        /* Never returns */
        Cy_Bootload_ExecuteApp(validApp);
    }
    /* If no Bootloadable application is valid */
    else
    {
        /* Turn LED yellow, signaling no valid app is present. */
        Cy_GPIO_Write(PIN_LED_RED_PORT, PIN_LED_RED_NUM, 0u);
        Cy_GPIO_Write(PIN_LED_GREEN_PORT, PIN_LED_GREEN_NUM, 0u);
        Cy_GPIO_Write(PIN_LED_BLUE_PORT, PIN_LED_BLUE_NUM, 1u);
        
        /* Halt System */
        Cy_SysLib_Halt(0x00u);
    }
}

/*******************************************************************************
* Function Name: IsButtonPressed
********************************************************************************
*  Checks if button is pressed for a 'timeoutInMilis' time.
*
* Params:
*   timeout: Amount of time to check if button was pressed. Broken into
*            20 miliseconds steps.
* Returns:
*  true if button is pressed for specified amount.
*  false otherwise.
*******************************************************************************/
static bool IsButtonPressed(uint16_t timeoutInMilis)
{
    uint16_t buttonTime = 0;
    bool buttonPressed = false;
    timeoutInMilis /= 20;
    while(Cy_GPIO_Read(PIN_SW2_PORT, PIN_SW2_NUM) == 0u)
    {
        Cy_SysLib_Delay(20u);
        if(++buttonTime == timeoutInMilis)
        {
            /* time has passed */
            buttonPressed = true;
            break;
        }
    }
    return buttonPressed;
}

/*******************************************************************************
* Function Name: CopyRow
********************************************************************************
* Copies data from a "src" address to a flash row with the address "dest".
* If "src" data is the same as "dest" data then no copy is needed.
*
* Parameters:
*  dest     Destination address. Has to be an address of the start of flash row.
*  src      Source address. Has to be properly aligned.
*  rowSize  Size of flash row.
*
* Returns:
*  CY_BOOTLAOD_SUCCESS if operation is successful.
*  Error code in a case of failure.
*******************************************************************************/
static cy_en_bootload_status_t CopyRow(uint32_t dest, uint32_t src, uint32_t rowSize, cy_stc_bootload_params_t * params)
{
    cy_en_bootload_status_t status;
    
    /* Save params->dataBuffer value */
    uint8_t *buffer = params->dataBuffer;

    /* Compare "dest" and "src" content */
    params->dataBuffer = (uint8_t *)src;
    status = Cy_Bootload_ReadData(dest, rowSize, CY_BOOTLOAD_IOCTL_COMPARE, params);
    
    /* Restore params->dataBuffer */
    params->dataBuffer = buffer;

    /* If "dest" differs from "src" then copy "src" to "dest" */
    if (status != CY_BOOTLOAD_SUCCESS)
    {
        (void) memcpy((void *) params->dataBuffer, (const void*)src, rowSize);
        status = Cy_Bootload_WriteData(dest, rowSize, CY_BOOTLOAD_IOCTL_WRITE, params);
    }
    /* Restore params->dataBuffer */
    params->dataBuffer = buffer;
    
    return (status);
}

/*******************************************************************************
* Function Name: HandleMetadata
********************************************************************************
* The goal of this function is to make Bootloader SDK metadata (MD) valid.
* The following algorithm is used (in C-like pseudocode):
* ---
* if (isValid(MD) == true)
* {   if (MDC != MD)
*         MDC = MD;
* } else
* {   if(isValid(MDC) )
*         MD = MDC;
* }
* ---
* Here MD is metadata flash row, MDC is flash row with metadata copy,
* INITIAL_VALUE is known initial value.
*
* In this code example MDC is placed in the next flash row after the MD
* This is only done if metadata is writeable when bootloading.
*
* Parameters:
*  params   A pointer to a Bootloader SDK parameters structure.
*
* Returns:
* - CY_BOOTLOAD_SUCCESS when finished normally.
* - Any other status code on error.
*******************************************************************************/
static cy_en_bootload_status_t HandleMetadata(cy_stc_bootload_params_t *params)
{
    const uint32_t MD     = (uint32_t)(&__cy_boot_metadata_addr   ); /* MD address  */
    const uint32_t mdSize = (uint32_t)(&__cy_boot_metadata_length ); /* MD size, assumed to be one flash row */
    const uint32_t MDC    = MD + mdSize;                             /* MDC address */

    cy_en_bootload_status_t status = CY_BOOTLOAD_SUCCESS;
    
    status = Cy_Bootload_ValidateMetadata(MD, params);
    if (status == CY_BOOTLOAD_SUCCESS)
    {
        /* Checks if MDC equals to DC, if no then copies MD to MDC */
        status = CopyRow(MDC, MD, mdSize, params);
    }
    else
    {
        status = Cy_Bootload_ValidateMetadata(MDC, params);
        if (status == CY_BOOTLOAD_SUCCESS)
        {
            /* Copy MDC to MD */
            status = CopyRow(MD, MDC, mdSize, params);
        }
    }
    return (status);
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
    #if defined(__ARMCC_VERSION)
    /* 
    * At Reset, this variable is set to 0 to avoid jumping 
    * to an invalid address when starting App1.
    */
    CyReturnToBootloaddableAddress = 0u;
    #endif /* __ARMCC_VERSION */
    
    Cy_Bootload_OnResetApp0();
}

/* [] END OF FILE */
