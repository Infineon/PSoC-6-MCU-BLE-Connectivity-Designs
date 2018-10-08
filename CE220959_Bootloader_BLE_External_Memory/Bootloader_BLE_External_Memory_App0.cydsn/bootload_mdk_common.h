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
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
    
#ifndef BOOTLOAD_MDK_COMMON_H_
#define BOOTLOAD_MDK_COMMON_H_

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
#define CY_APP0_CORE0_FLASH_LENGTH      0x10000

#define CY_APP0_CORE1_FLASH_ADDR        0x10010000
#define CY_APP0_CORE1_FLASH_LENGTH      0x30000

#define CY_APP1_CORE0_FLASH_ADDR        0x10040000
#define CY_APP1_CORE0_FLASH_LENGTH      0x32000

#define CY_APP1_CORE1_FLASH_ADDR        0x10072000
#define CY_APP1_CORE1_FLASH_LENGTH      0x02000

/* Bootloader SDK metadata address range in Flash */
#define CY_BOOT_META_FLASH_ADDR         0x100FFA00
#define CY_BOOT_META_FLASH_LENGTH       0x200

/* Application ranges in emulated EEPROM */
#define CY_APP0_CORE0_EM_EEPROM_ADDR    0x14000000
#define CY_APP0_CORE0_EM_EEPROM_LENGTH  0x8000

#define CY_APP0_CORE1_EM_EEPROM_ADDR    CY_APP0_CORE0_EM_EEPROM_ADDR
#define CY_APP0_CORE1_EM_EEPROM_LENGTH  0x8000

#define CY_APP1_CORE0_EM_EEPROM_ADDR    0x14000000
#define CY_APP1_CORE0_EM_EEPROM_LENGTH  0x8000

#define CY_APP1_CORE1_EM_EEPROM_ADDR    CY_APP1_CORE0_EM_EEPROM_ADDR
#define CY_APP1_CORE1_EM_EEPROM_LENGTH  0x8000

/* Application ranges in SMIF XIP */
#define CY_APP0_CORE0_SMIF_ADDR         0x18000000
#define CY_APP0_CORE0_SMIF_LENGTH       0x00000000

#define CY_APP0_CORE1_SMIF_ADDR         (CY_APP0_CORE0_SMIF_ADDR + CY_APP0_CORE0_SMIF_LENGTH)
#define CY_APP0_CORE1_SMIF_LENGTH       0x00000000

#define CY_APP1_CORE0_SMIF_ADDR         0x14000200
#define CY_APP1_CORE0_SMIF_LENGTH       0x00000000

#define CY_APP1_CORE1_SMIF_ADDR         (CY_APP1_CORE0_SMIF_ADDR + CY_APP1_CORE0_SMIF_LENGTH)
#define CY_APP1_CORE1_SMIF_LENGTH       0x00000000

/* Application ranges in RAM */
#define CY_APP_RAM_COMMON_ADDR          0x08000000
#define CY_APP_RAM_COMMON_LENGTH        0x00000100

/* note: all the CY_APPX_CORE0_RAM regions has to be 0x100 aligned */
/* and the CY_APPX_CORE1_RAM regions has to be 0x400 aligned       */
/* as they contain Interrupt Vector Table Remapped at the start */

#define CY_APP0_CORE0_RAM_ADDR          0x08000100
#define CY_APP0_CORE0_RAM_LENGTH        0x00001F00

#define CY_APP0_CORE1_RAM_ADDR          (CY_APP0_CORE0_RAM_ADDR + CY_APP0_CORE0_RAM_LENGTH)
#define CY_APP0_CORE1_RAM_LENGTH        0x00008000

#define CY_APP1_CORE0_RAM_ADDR          CY_APP0_CORE0_RAM_ADDR
#define CY_APP1_CORE0_RAM_LENGTH        0x0001FF00

#define CY_APP1_CORE1_RAM_ADDR          (CY_APP1_CORE0_RAM_ADDR + CY_APP1_CORE0_RAM_LENGTH)
#define CY_APP1_CORE1_RAM_LENGTH        0x00020000


#endif /* BOOTLOAD_MDK_COMMON_H_ */


/* [] END OF FILE */
