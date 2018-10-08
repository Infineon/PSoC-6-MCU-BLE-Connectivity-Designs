/*******************************************************************************
* \file bootload_mdk_symbols.c
* \version 2.10
* 
* This file provides symbols to add to an ELF file required by
* CyMCUElfTool to generate correct HEX and CYACD2 files.
* 
********************************************************************************
* \copyright
* Copyright 2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "bootload_mdk_common.h"

/*******************************************************************************
* Function Name: cy_Bootload_mdkAsmDummy
********************************************************************************
* This function provides ELF file symbols through
* the inline assembly.
* The inline assembly in the *.c file is chosen, because it allows using
* #include <mdk_linker_common.h> where the user configuration is updated.
*
* Note that this function does not have code, so no additional memory
* is allocated for it.
*******************************************************************************/
__asm void cy_Bootload_mdkAsmDummy(void)
{
    EXPORT __cy_boot_metadata_addr  
    EXPORT __cy_boot_metadata_length
    
    EXPORT __cy_app_core1_start_addr
    
    EXPORT __cy_product_id
    EXPORT __cy_checksum_type
    EXPORT __cy_app_id
    
    EXPORT __cy_app_verify_start
    EXPORT __cy_app_verify_length
    
/* Used by all Bootloader SDK applications to switch to another app */
__cy_boot_metadata_addr     EQU __cpp(__CY_BOOT_METADATA_ADDR)
/* Used by CyMCUElfTool to update Bootloader SDK metadata with CRC-32C */
__cy_boot_metadata_length   EQU __cpp(__CY_BOOT_METADATA_LENGTH)

/* Used by CM0+ to start CM4 core in the Bootloader SDK applications. */
/* Make sure the correct app no. is entered here */
__cy_app_core1_start_addr   EQU __cpp(CY_APP1_CORE1_FLASH_ADDR)

/* Used by CyMCUElfTool to generate ProductID */
__cy_product_id             EQU __cpp(__CY_PRODUCT_ID)
/* Used by CyMCUElfTool to generate ChecksumType */
__cy_checksum_type          EQU __cpp(__CY_CHECKSUM_TYPE)
/* Application number (ID) */
__cy_app_id                 EQU 3

/* CyMCUElfTool uses these to generate an application signature */
/* The size of the default signature (CRC-32C) is 4 bytes */
__cy_app_verify_start     EQU __cpp(CY_APP1_CORE0_FLASH_ADDR)
__cy_app_verify_length    EQU __cpp(CY_APP1_CORE0_FLASH_LENGTH + CY_APP1_CORE1_FLASH_LENGTH - __CY_BOOT_SIGNATURE_SIZE)
}


/* [] END OF FILE */
