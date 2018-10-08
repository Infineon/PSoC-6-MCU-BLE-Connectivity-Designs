/***************************************************************************//**
* \file bootload_cm0p.ld
* \version 2.10
*
* The linker file for the GNU C compiler.
* Used for Bootloader SDK core0 firmware projects.
*
* \note The linker files included with the PDL template projects must be generic
* and handle all common use cases. Your project may not use every section
* defined in the linker files. In that case, you may see warnings during the
* build process. In your project, simply comment out or remove the
* relevant code in the linker file.
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)
ENTRY(Reset_Handler)


/*
* Forces symbol to be added to the output file.
* Otherwise linkes may remove it if founds that it is not used in the project.
* This command has the same effect as the -u command-line option.
*/
EXTERN(Reset_Handler)


/* Bootloader SDK specific: includes common configuration */
INCLUDE bootload_common.ld

/*
* Bootloader SDK specific: aliases regions, so the rest of code does not use
* application specific memory region names
*/
REGION_ALIAS("flash",       flash_app2_core0);
REGION_ALIAS("flash_core1", flash_app2_core1);
REGION_ALIAS("ram",           ram_app2_core0);

/* Bootloader SDK specific: sets an app Id */
__cy_app_id = 2;

/*
* Bootloader SDK specific: sets a start address of the Core1 application image,
* more specifically an address of the Core1 interrupt vector table.
* CM0+ uses this information to launch Core1.
*/
__cy_app_core1_start_addr = ORIGIN(flash_core1); /* used to start Core1 from Core0 */

/* Bootloader SDK specific */
/* CyMCUElfTool uses these ELF symbols to generate an application signature */
__cy_app_verify_start  = ORIGIN(flash);
__cy_app_verify_length = LENGTH(flash) + LENGTH(flash_core1) - __cy_boot_signature_size;


/* Library configurations */
GROUP(libgcc.a libc.a libm.a libnosys.a)

/* The linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions FLASH and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines the symbols which can be used by code without a definition:
 *   __exidx_start
 *   __exidx_end
 *   __copy_table_start__
 *   __copy_table_end__
 *   __zero_table_start__
 *   __zero_table_end__
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 *   __Vectors_End
 *   __Vectors_Size
 *
 * For the Bootloder SDK, the following additional symbols are defined:
 *   __cy_app_id
 *   __cy_product_id
 *   __cy_checksum_type
 *   __cy_app_core1_start_addr
 *   __cy_boot_metadata_addr
 *   __cy_boot_metadata_length
 */


SECTIONS
{
    /* Bootloader SDK specific */
    /* the noinit section, used across all the applications */
    .cy_boot_noinit (NOLOAD) :
    {
        KEEP(*(.cy_boot_noinit));
    } > ram_common

    /* The last byte of the section is used for AppId to be shared between all the applications */
    .cy_boot_noinit.appId ORIGIN(ram_common) + LENGTH(ram_common) - 1 (NOLOAD) :
    {
        KEEP(*(.cy_boot_noinit.appId));
    } > ram_common
    
    /* App0 uses it to initialize Bootloader SDK metadata, in the bootload_user.c file */
    .cy_boot_metadata :
    {
        KEEP(*(.cy_boot_metadata))
    } > flash_boot_meta

    .cy_app_header :
    {
        KEEP(*(.cy_app_header))
    } > flash

    .text :
    {
        . = ALIGN(4);
        __Vectors = . ;
        KEEP(*(.vectors))
        . = ALIGN(4);
        __Vectors_End = .;
        __Vectors_Size = __Vectors_End - __Vectors;
        __end__ = .;

        . = ALIGN(4);
        *(.text*)

        KEEP(*(.init))
        KEEP(*(.fini))

        /* .ctors */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)

        /* .dtors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)

        /* Read-only code (constants). */
        *(.rodata .rodata.* .constdata .constdata.* .conststring .conststring.*)

        KEEP(*(.eh_frame*))
    } > flash


    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > flash

    __exidx_start = .;

    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > flash
    __exidx_end = .;


    /* 
    *  The following lines are only to be used when the BLE stack is running
    *  exclusively on the CM4 core. If the BLE stack runs in dual core mode
    *  or exclusively on the CM0p core, comment this section out.
    */
    
    /* ---------- BLE Stack CM4 Start ---------- */
    
    .copy.table :
    {
        . = ALIGN(4);
        __copy_table_start__ = .;

        LONG (__Vectors)
        LONG (__ram_vectors_start__)
        LONG (__Vectors_End - __Vectors)

        LONG (__etext)
        LONG (__data_start__)
        LONG (__data_end__ - __data_start__)

        __copy_table_end__ = .;
    } > flash


    .zero.table :
    {
        . = ALIGN(4);
        __zero_table_start__ = .;
        LONG (__bss_start__)
        LONG (__bss_end__ - __bss_start__)
        __zero_table_end__ = .;
    } > flash

    __etext =  . ;


    .ramVectors (NOLOAD) : ALIGN(8)
    {
        __ram_vectors_start__ = .;
        KEEP(*(.ram_vectors))
        __ram_vectors_end__   = .;
    } > ram


    .data __ram_vectors_end__ : AT (__etext)
    {
        __data_start__ = .;

        *(vtable)
        *(.data*)

        . = ALIGN(4);
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);


        . = ALIGN(4);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        KEEP(*(.jcr*))
        . = ALIGN(4);

        KEEP(*(.cy_ramfunc*))
        . = ALIGN(4);

        __data_end__ = .;

    } > ram

    /* ---------- BLE Stack CM4 End ---------- */

    /*
    *  The following lines are only to be used when the BLE stack is running
    *  exclusively on the CM0p core or in dual core mode. If the BLE stack runs
    *  exclusively on the CM4 core, comment this section out.
    */

    /* ---------- BLE Stack Dual/CM0p Start ---------- 

    .copy.table :
    {
        . = ALIGN(4);
        __copy_table_start__ = .;

        LONG (__Vectors)
        LONG (__ram_vectors_start__)
        LONG (__Vectors_End - __Vectors)

        LONG (__ble_core0_data_at_flash)
        LONG (__ble_core0_data_start__)
        LONG (__ble_core0_data_end__ - __ble_core0_data_start__)

        LONG (__etext)
        LONG (__common_config_start__)
        LONG (__common_config_end__ - __common_config_start__)

        LONG (__etext + (__common_config_end__ - __common_config_start__))
        LONG (__data_start__)
        LONG (__data_end__ - __data_start__)

        __copy_table_end__ = .;
    } > flash


    .zero.table :
    {
        . = ALIGN(4);
        __zero_table_start__ = .;
        
        LONG (__bss_start__)
        LONG (__bss_end__ - __bss_start__)
        
        LONG (__ble_core0_bss_start__)
        LONG (__ble_core0_bss_end__ - __ble_core0_bss_start__)
        
        __zero_table_end__ = .;
    } > flash

    __etext =  . ;

    .ramVectors (NOLOAD) : ALIGN(8)
    {
        __ram_vectors_start__ = .;
        KEEP(*(.ram_vectors))
        __ram_vectors_end__   = .;
    } > ram

    .cy_boot_ble_data : AT (__etext)
    {
        __common_config_start__ = .;
        *system_psoc63_cm0plus.o(.data*)
        *cy_ipc_sema.o(.data*)
        *cy_ipc_config.o(.data*)
        *cy_ipc_pipe.o(.data*)
        __common_config_end__ = .;
    } > ram_ble_core0
    
    .cy_boot_ble_bss ABSOLUTE(__ble_core0_bss_start__) (NOLOAD):
    {
        *system_psoc63_cm0plus.o(.bss*)
        *cy_ipc_sema.o(.bss*)
        *cy_ipc_config.o(.bss*)
        *cy_ipc_pipe.o(.bss*)
    } > ram_ble_core0

    .data __ram_vectors_end__ : AT (__etext + (__common_config_end__ - __common_config_start__))
    {
        __data_start__ = .;

        *(vtable)
        *(.data*)

        . = ALIGN(4);
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);


        . = ALIGN(4);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        KEEP(*(.jcr*))
        . = ALIGN(4);

        KEEP(*(.cy_ramfunc*))
        . = ALIGN(4);

        __data_end__ = .;

    } > ram
    
    /* ---------- BLE Stack Dual/CM0p End ---------- */

    /* Place variables in the section that should not be initialized during the
    *  device startup.
    */
    .noinit (NOLOAD) : ALIGN(8)
    {
      KEEP(*(.noinit))
    } > ram


    /* The uninitialized global or static variables are placed in this section.
    *
    * The NOLOAD attribute tells linker that the .bss section does not consume
    * any space in the image. The NOLOAD attribute changes the .bss type to
    * NOBITS, and that  makes the linker: A) not allocate the section in memory;
    * B) put information to clear the section with all zeros during application
    * loading.
    *
    * Without the NOLOAD attribute, the .bss section might get the PROGBITS type.
    * This  makes the linker: A) allocate the zeroed section in memory; B) copy
    * this section to RAM during application loading.
    */
    .bss (NOLOAD):
    {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > ram


    .heap (NOLOAD):
    {
        __HeapBase = .;
        __end__ = .;
        end = __end__;
        KEEP(*(.heap*))
        __HeapLimit = .;
    } > ram


    /* The .stack_dummy section doesn't contain any symbols. It is only
     * used for the linker to calculate the size of the stack sections, and assign
     * values to the stack symbols later */
    .stack_dummy (NOLOAD):
    {
        KEEP(*(.stack*))
    } > ram


    /* Set the stack top to the end of RAM, and stack limit move down by
     * the size of the stack_dummy section */
    __StackTop = ORIGIN(ram) + LENGTH(ram);
    __StackLimit = __StackTop - SIZEOF(.stack_dummy);
    PROVIDE(__stack = __StackTop);

    /* Check if data + heap + stack exceeds RAM limit */
    ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")


    /* Emulated EEPROM Flash area */
    .cy_em_eeprom :
    {
        KEEP(*(.cy_em_eeprom))
    } > em_eeprom


    /* Supervisory Flash: User data */
    .cy_sflash_user_data :
    {
        KEEP(*(.cy_sflash_user_data))
    } > sflash_user_data


    /* Supervisory Flash: Normal Access Restrictions (NAR) */
    .cy_sflash_nar :
    {
        KEEP(*(.cy_sflash_nar))
    } > sflash_nar


    /* Supervisory Flash: Public Key */
    .cy_sflash_public_key :
    {
        KEEP(*(.cy_sflash_public_key))
    } > sflash_public_key


    /* Supervisory Flash: Table of Content # 2 */
    .cy_toc_part2 :
    {
        KEEP(*(.cy_toc_part2))
    } > sflash_toc_2


    /* Places the code in the Execute in Place (XIP) section. See the smif driver
    *  documentation for details.
    */
    .cy_xip :
    {
        KEEP(*(.cy_xip))
    } > xip


    /* eFuse */
    .cy_efuse :
    {
        KEEP(*(.cy_efuse))
    } > efuse


    /* These sections are used for additional metadata (silicon revision,
    *  Silicon/JTAG ID, etc.) storage.
    */
    .cymeta         0x90500000 : { KEEP(*(.cymeta)) } :NONE
}


/* EOF */
