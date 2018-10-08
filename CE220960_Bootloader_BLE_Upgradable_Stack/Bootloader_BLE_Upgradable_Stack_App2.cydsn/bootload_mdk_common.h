/*******************************************************************************
* \file bootload_mdk_common.h
* \version 2.10
* 
* This file provides only macro definitions to use for
* project configuration.
* They may be used in both scatter files and source code files.
* 
********************************************************************************
* \copyright
* Copyright 2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
    
#ifndef BOOTLOAD_MDK_COMMON_H_
#define BOOTLOAD_MDK_COMMON_H_

/* The following #define is used to rearrange the RAM and FLASH memory depending on */
/* the core running the BLE Component. (BLE_CM0, BLE_CM4, BLE_DUAL) */
#define BLE_CM4
    
/* Bootloader SDK parameters */
/* The user application may either update them or leave the defaults if they fit */
#define __CY_BOOT_METADATA_ADDR         0x100FFA00
#define __CY_BOOT_METADATA_LENGTH       0x200
#define __CY_PRODUCT_ID                 0x01020304
#define __CY_CHECKSUM_TYPE              0x00

/*
* The size of the section .cy_app_signature.
* 1,2, or 4 for a checksum
* 4 for CRC-32
* 20 for SHA1
* 32 for SHA256
* 256 for RSASSA-PKCS1-v1.5 with the 2048 bit RSA key.
*
* SHA1 must be used.
*/
#define __CY_BOOT_SIGNATURE_SIZE        4

/* For the MDK linker script, defines TOC parameters */
/* Update per device series to be in the last Flash row */
#define CY_TOC_START                    0x16007C00
#define CY_TOC_SIZE                     0x400


/* Memory region ranges per core and app */
#define CY_APP0_CORE0_FLASH_ADDR        0x10000000
#define CY_APP0_CORE0_FLASH_LENGTH      0x03800

#define CY_APP0_CORE1_FLASH_ADDR        0x10003800
#define CY_APP0_CORE1_FLASH_LENGTH      0x02800

/* FLASH partitioning depends on which core runs the BLE Component */
#ifdef BLE_CM4

#define CY_APP1_CORE0_FLASH_ADDR        0x10006000
#define CY_APP1_CORE0_FLASH_LENGTH      0x03800

#define CY_APP1_CORE1_FLASH_ADDR        0x10009800
#define CY_APP1_CORE1_FLASH_LENGTH      0x2F000

#define CY_APP2_CORE0_FLASH_ADDR        0x10040000
#define CY_APP2_CORE0_FLASH_LENGTH      0x03400
    
#define CY_APP2_CORE1_FLASH_ADDR        0x10043400
#define CY_APP2_CORE1_FLASH_LENGTH      0x08000

#elif defined BLE_CM0

#define CY_APP1_CORE0_FLASH_ADDR        0x10006000
#define CY_APP1_CORE0_FLASH_LENGTH      0x30000

#define CY_APP1_CORE1_FLASH_ADDR        0x10036000
#define CY_APP1_CORE1_FLASH_LENGTH      0x02400

#define CY_APP2_CORE0_FLASH_ADDR        0x10040000
#define CY_APP2_CORE0_FLASH_LENGTH      0x09400
    
#define CY_APP2_CORE1_FLASH_ADDR        0x10049400
#define CY_APP2_CORE1_FLASH_LENGTH      0x02400

#elif defined BLE_DUAL

#define CY_APP1_CORE0_FLASH_ADDR        0x10006000
#define CY_APP1_CORE0_FLASH_LENGTH      0x1B000

#define CY_APP1_CORE1_FLASH_ADDR        0x10021000
#define CY_APP1_CORE1_FLASH_LENGTH      0x1A000

#define CY_APP2_CORE0_FLASH_ADDR        0x10040000
#define CY_APP2_CORE0_FLASH_LENGTH      0x04000
    
#define CY_APP2_CORE1_FLASH_ADDR        0x10044000
#define CY_APP2_CORE1_FLASH_LENGTH      0x08000

#endif

/* Bootloader SDK metadata address range in Flash */
#define CY_BOOT_META_FLASH_ADDR         0x100FFA00
#define CY_BOOT_META_FLASH_LENGTH       0x200

/* Flash row to store the copy flag variable */
#define CY_BOOT_COPY_FLASH_ADDR         0x100FF800
#define CY_BOOT_COPY_FLASH_LENGTH       0x200

/* Application ranges in emulated EEPROM */
#define CY_APP0_CORE0_EM_EEPROM_ADDR    0x14000000
#define CY_APP0_CORE0_EM_EEPROM_LENGTH  0x00000000

#define CY_APP0_CORE1_EM_EEPROM_ADDR    (CY_APP0_CORE0_EM_EEPROM_ADDR + CY_APP0_CORE0_EM_EEPROM_LENGTH)
#define CY_APP0_CORE1_EM_EEPROM_LENGTH  0x0000

#define CY_APP1_CORE0_EM_EEPROM_ADDR    0x14000000
#define CY_APP1_CORE0_EM_EEPROM_LENGTH  0x00000000

#define CY_APP1_CORE1_EM_EEPROM_ADDR    (CY_APP1_CORE0_EM_EEPROM_ADDR + CY_APP1_CORE0_EM_EEPROM_LENGTH)
#define CY_APP1_CORE1_EM_EEPROM_LENGTH  0x00000000

#define CY_APP2_CORE0_EM_EEPROM_ADDR    0x14000000
#define CY_APP2_CORE0_EM_EEPROM_LENGTH  0x00008000

#define CY_APP2_CORE1_EM_EEPROM_ADDR    CY_APP2_CORE0_EM_EEPROM_ADDR
#define CY_APP2_CORE1_EM_EEPROM_LENGTH  0x00008000

/* Application ranges in SMIF XIP */
#define CY_APP0_CORE0_SMIF_ADDR         0x18000000
#define CY_APP0_CORE0_SMIF_LENGTH       0x00000000

#define CY_APP0_CORE1_SMIF_ADDR         (CY_APP0_CORE0_SMIF_ADDR + CY_APP0_CORE0_SMIF_LENGTH)
#define CY_APP0_CORE1_SMIF_LENGTH       0x00000000

#define CY_APP1_CORE0_SMIF_ADDR         0x18000000
#define CY_APP1_CORE0_SMIF_LENGTH       0x00000000

#define CY_APP1_CORE1_SMIF_ADDR         (CY_APP1_CORE0_SMIF_ADDR + CY_APP1_CORE0_SMIF_LENGTH)
#define CY_APP1_CORE1_SMIF_LENGTH       0x00000000

#define CY_APP2_CORE0_SMIF_ADDR         0x18000000
#define CY_APP2_CORE0_SMIF_LENGTH       0x00000000

#define CY_APP2_CORE1_SMIF_ADDR         (CY_APP2_CORE0_SMIF_ADDR + CY_APP2_CORE0_SMIF_LENGTH)
#define CY_APP2_CORE1_SMIF_LENGTH       0x00000000

/* Application ranges in RAM */
#define CY_APP_RAM_COMMON_ADDR          0x08000000
#define CY_APP_RAM_COMMON_LENGTH        0x00000100

/* note: all the CY_APPX_CORE0_RAM regions has to be 0x100 aligned */
/* and the CY_APPX_CORE1_RAM regions has to be 0x400 aligned       */
/* as they contain Interrupt Vector Table Remapped at the start */

#define CY_APP0_CORE0_RAM_ADDR          0x08000100
#define CY_APP0_CORE0_RAM_LENGTH        0x00001F00

#define CY_APP0_CORE1_RAM_ADDR          (CY_APP0_CORE0_RAM_ADDR + CY_APP0_CORE0_RAM_LENGTH)
#define CY_APP0_CORE1_RAM_LENGTH        0x00002000

/* The BLE Stack RAM data must be reserved and reinitialized by App2 */
/* In order to save RAM, the STACK and HEAP of App1 are placed somewhere */
/* where they can be overwritten. Some common configuration variables must be */
/* updated by App2, these are placed in a specific section for this purpose */
    
#ifdef BLE_CM4

#define CY_APP1_CORE1_CONFIG_RAM_ADDR   (CY_APP_RAM_COMMON_ADDR + CY_APP_RAM_COMMON_LENGTH)
#define CY_APP1_CORE1_CONFIG_RAM_LENGTH 0x00000300

#define CY_APP1_CORE1_RAM_ADDR          (CY_APP1_CORE1_CONFIG_RAM_ADDR + CY_APP1_CORE1_CONFIG_RAM_LENGTH)
#define CY_APP1_CORE1_RAM_LENGTH        0x00002600

#define CY_APP1_CORE0_RAM_ADDR          (CY_APP1_CORE1_RAM_ADDR + CY_APP1_CORE1_RAM_LENGTH)
#define CY_APP1_CORE0_RAM_LENGTH        0x00002000

#define CY_APP1_CORE1_STACKHEAP_ADDR    (CY_APP1_CORE0_RAM_ADDR + CY_APP1_CORE0_RAM_LENGTH)
#define CY_APP1_CORE1_STACKHEAP_LENGTH  0x00005000
    
#define CY_APP2_RAM_START               (CY_APP1_CORE1_RAM_ADDR + CY_APP1_CORE1_RAM_LENGTH)

#elif defined BLE_CM0

#define CY_APP1_CORE0_CONFIG_RAM_ADDR   (CY_APP_RAM_COMMON_ADDR + CY_APP_RAM_COMMON_LENGTH)
#define CY_APP1_CORE0_CONFIG_RAM_LENGTH 0x00000300

#define CY_APP1_CORE0_RAM_ADDR          (CY_APP1_CORE0_CONFIG_RAM_ADDR + CY_APP1_CORE0_CONFIG_RAM_LENGTH)
#define CY_APP1_CORE0_RAM_LENGTH        0x00002400

#define CY_APP1_CORE1_RAM_ADDR          (CY_APP1_CORE0_RAM_ADDR + CY_APP1_CORE0_RAM_LENGTH)
#define CY_APP1_CORE1_RAM_LENGTH        0x00002000

#define CY_APP1_CORE0_STACKHEAP_ADDR    (CY_APP1_CORE1_RAM_ADDR + CY_APP1_CORE1_RAM_LENGTH)
#define CY_APP1_CORE0_STACKHEAP_LENGTH  0x00005000

#define CY_APP2_RAM_START               (CY_APP1_CORE0_RAM_ADDR + CY_APP1_CORE0_RAM_LENGTH)
    
#elif defined BLE_DUAL

#define CY_APP1_CORE0_CONFIG_RAM_ADDR   (CY_APP_RAM_COMMON_ADDR + CY_APP_RAM_COMMON_LENGTH)
#define CY_APP1_CORE0_CONFIG_RAM_LENGTH 0x00000300

#define CY_APP1_CORE1_CONFIG_RAM_ADDR   (CY_APP1_CORE0_CONFIG_RAM_ADDR + CY_APP1_CORE0_CONFIG_RAM_LENGTH)
#define CY_APP1_CORE1_CONFIG_RAM_LENGTH 0x00000300

#define CY_APP1_CORE0_RAM_ADDR          (CY_APP1_CORE1_CONFIG_RAM_ADDR + CY_APP1_CORE1_CONFIG_RAM_LENGTH)
#define CY_APP1_CORE0_RAM_LENGTH        0x00001500

#define CY_APP1_CORE1_RAM_ADDR          (CY_APP1_CORE0_RAM_ADDR + CY_APP1_CORE0_RAM_LENGTH)
#define CY_APP1_CORE1_RAM_LENGTH        0x00001C00

#define CY_APP1_CORE0_STACKHEAP_ADDR    (CY_APP1_CORE1_RAM_ADDR + CY_APP1_CORE1_RAM_LENGTH)
#define CY_APP1_CORE0_STACKHEAP_LENGTH  0x00003500

#define CY_APP1_CORE1_STACKHEAP_ADDR    (CY_APP1_CORE0_STACKHEAP_ADDR + CY_APP1_CORE0_STACKHEAP_LENGTH)
#define CY_APP1_CORE1_STACKHEAP_LENGTH  0x00005000

#define CY_APP2_RAM_START               (CY_APP1_CORE1_RAM_ADDR + CY_APP1_CORE1_RAM_LENGTH)

#endif /* BLE_CM0 || BLE_CM4 || BLE_DUAL */

#define CY_APP2_CORE0_RAM_ADDR          CY_APP2_RAM_START
#define CY_APP2_CORE0_RAM_LENGTH        (0x08020000 - CY_APP2_RAM_START)

#define CY_APP2_CORE1_RAM_ADDR          (CY_APP2_CORE0_RAM_ADDR + CY_APP2_CORE0_RAM_LENGTH)
#define CY_APP2_CORE1_RAM_LENGTH        0x00027800

#endif /* BOOTLOAD_MDK_COMMON_H_ */


/* [] END OF FILE */
