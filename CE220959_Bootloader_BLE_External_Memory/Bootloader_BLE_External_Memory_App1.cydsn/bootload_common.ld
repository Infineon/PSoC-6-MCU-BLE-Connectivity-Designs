/***************************************************************************//**
* \file bootload_common.ld
* \version 2.10
*
* This is a common part of all GCC linker scripts for Bootloader SDK applications.
* \note It contains the default content, so your project may need to modify it.
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/*
* Memory regions, for each application and MCU core.
*/
MEMORY
{
    flash_app0_core0  (rx)  : ORIGIN = 0x10000000, LENGTH = 0x10000
    flash_app0_core1  (rx)  : ORIGIN = 0x10010000, LENGTH = 0x30000
    flash_app1_core0  (rx)  : ORIGIN = 0x10040000, LENGTH = 0x32000
    flash_app1_core1  (rx)  : ORIGIN = 0x10072000, LENGTH = 0x02000

    flash_storage     (rw)  : ORIGIN = 0x100C0000, LENGTH = 0x1000
    flash_boot_meta   (rw)  : ORIGIN = 0x100FFA00, LENGTH = 0x400

    sflash_user_data  (rx)  : ORIGIN = 0x16000800, LENGTH = 0x800
    sflash_nar        (rx)  : ORIGIN = 0x16001A00, LENGTH = 0x200
    sflash_public_key (rx)  : ORIGIN = 0x16005A00, LENGTH = 0xC00
    sflash_toc_2      (rx)  : ORIGIN = 0x16007C00, LENGTH = 0x400

    efuse             (r)   : ORIGIN = 0x90700000, LENGTH = 0x100000

    ram_common        (rwx) : ORIGIN = 0x08000000, LENGTH = 0x0100

    /* note: all the ram_appX_core0 regions has to be 0x100 aligned */
    /* and the ram_appX_core1 regions has to be 0x400 aligned       */
    /* as they contain Interrupt Vector Table Remapped at the start */
    ram_app0_core0    (rwx) : ORIGIN = 0x08000100, LENGTH = 0x1F00
    ram_app0_core1    (rwx) : ORIGIN = 0x08002000, LENGTH = 0x8000

    ram_app1_core0    (rwx) : ORIGIN = 0x08000100, LENGTH = 0x1FF00
    ram_app1_core1    (rwx) : ORIGIN = 0x08020000, LENGTH = 0x20000

    em_eeprom         (rx)  : ORIGIN = 0x14000000, LENGTH = 0x8000
    xip               (rx)  : ORIGIN = 0x18000000, LENGTH = 0x08000000
}

/* Regions parameters */
/* Flash */
__cy_memory_0_start    = 0x10000000;
__cy_memory_0_length   = 0x00100000;
__cy_memory_0_row_size = 0x200;

/* Emulated EEPROM Flash area */
__cy_memory_1_start    = 0x14000000;
__cy_memory_1_length   = 0x8000;
__cy_memory_1_row_size = 0x200;

/* Supervisory Flash */
__cy_memory_2_start    = 0x16000000;
__cy_memory_2_length   = 0x8000;
__cy_memory_2_row_size = 0x200;

/* XIP */
__cy_memory_3_start    = 0x18000000;
__cy_memory_3_length   = 0x08000000;
__cy_memory_3_row_size = 0x200;

/* eFuse */
__cy_memory_4_start    = 0x90700000;
__cy_memory_4_length   = 0x100000;
__cy_memory_4_row_size = 1;

/* The Bootloader SDK metadata limits */
__cy_boot_metadata_addr = ORIGIN(flash_boot_meta);
__cy_boot_metadata_length = __cy_memory_0_row_size;

/* The Product ID, used by CyMCUElfTool to generate a bootloading file */
__cy_product_id = 0x01020304;

/* The checksum type used by CyMCUElfTool to generate a bootloading file */
__cy_checksum_type = 0x00;

/* Used by the Bootloader SDK application to set the metadata */
__cy_app0_verify_start = ORIGIN(flash_app0_core0);
__cy_app0_verify_length = LENGTH(flash_app0_core0) + LENGTH(flash_app0_core1) - __cy_boot_signature_size;
__cy_app1_verify_start = ORIGIN(flash_app1_core0);
__cy_app1_verify_length = LENGTH(flash_app1_core0) + LENGTH(flash_app1_core1) - __cy_boot_signature_size;

/*
* The size of the application signature.
* E.g. 4 for CRC-32,
*     32 for SHA256,
*    256 for RSA 2048.
*/
__cy_boot_signature_size = 4;
