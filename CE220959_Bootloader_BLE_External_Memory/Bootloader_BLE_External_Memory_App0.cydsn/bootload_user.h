/***************************************************************************//**
* \file bootload_user.h
* \version 2.10
*
* This file provides declarations that can be modified by the user but
* are used by the Bootloader SDK.
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(BOOTLOAD_USER_H)
#define BOOTLOAD_USER_H

#include <stdint.h>
#include "flash/cy_flash.h"
    

/**
* \addtogroup group_bootload_macro_config
* \{
*/

#if defined(CY_DOXYGEN) || !defined(CY_PSOC_CREATOR_USED)
    /**
    * The silicon ID for the chip value used by the Bootloader SDK.
    * If set to ZERO Bootloader Host, is supposed to ignore it.
    * The Enter bootloader command uses this to respond to a Bootloader Host.
    */
    #define CY_BOOTLOAD_SILICON_ID      (0u)
    /**
    * Silicon Revision for the chip.
    * This parameter is an addition to \ref CY_BOOTLOAD_SILICON_ID.
    */
    #define CY_BOOTLOAD_SILICON_REV     (0u)
#else
    #include "cy_device_headers.h" /* for CY_SILICON_ID            */
    #include <cyfitter.h>          /* for CYDEV_CHIP_REVISION_USED */

    #define CY_BOOTLOAD_SILICON_ID  CY_SILICON_ID
    #define CY_BOOTLOAD_SILICON_REV CYDEV_CHIP_REVISION_USED
#endif /* defined CY_DOXYGEN*/

/** The size of a buffer to hold bootloader commands */
/* 16 bytes is a maximum overhead of a bootloader packet and additional data for the Program Data command */
#define CY_BOOTLOAD_SIZEOF_CMD_BUFFER  (CY_FLASH_SIZEOF_ROW + 16u)

/** The size of a buffer to hold an NVM row of data to write or verify */
#define CY_BOOTLOAD_SIZEOF_DATA_BUFFER (CY_FLASH_SIZEOF_ROW + 16u)

/**
* Set to non-zero for the Bootloader SDK Program Data command to check
* if the Golden image is going to be overwritten while bootloading.
*/
#define CY_BOOTLOAD_OPT_GOLDEN_IMAGE    (0)

/**
* List of Golden Image Application IDs.
* Here "Golden Image Application" means an application that cannot be changed with
* CommandProgramData()
*
* Usage. Define the list of Golden Image Application IDs without enclosing
* parenthesis, e.g.
* \code #define CY_BOOTLOAD_GOLDEN_IMAGE_IDS()     0u, 1u, 3u \endcode
* later it is used in cy_bootload.c file:
* \code uint8_t goldenImages[] = { CY_BOOTLOAD_GOLDEN_IMAGE_IDS() }; \endcode
*/
#define CY_BOOTLOAD_GOLDEN_IMAGE_IDS()  0u

/**
* The number of applications in the metadata,
* for 512 bytes in a Flash row - 63 is the maximum possible value,
* because 4 bytes are reserved for the entire metadata CRC.
* 
* The smallest metadata size if CY_BOOTLOAD_MAX_APPS * 8 (bytes per one app) + 4 (bytes for CRC-32C)
*/
#define CY_BOOTLOAD_MAX_APPS            (3u)


/** A non-zero value enables the Verify Data bootloader command  */
#define CY_BOOTLOAD_OPT_VERIFY_DATA     (1)

/** A non-zero value enables the Erase Data bootloader command   */
#define CY_BOOTLOAD_OPT_ERASE_DATA      (1)

/** A non-zero value enables the Verify App bootloader command   */
#define CY_BOOTLOAD_OPT_VERIFY_APP      (1)

/**
* A non-zero value enables the Send Data bootloader command.
* If the "Send Data" bootloader command is enabled, \c packetBuffer and \c dataBuffer
* must be non-overlapping.
*
* Else, \c dataBuffer must be inside \c packetBuffer with an offset of
* \c CY_BOOTLOAD_PACKET_DATA_IDX, typically 4 bytes. \n
* <code>params->dataBuffer = &packetBuffer[4];</code> \n
* \note that \c packetBuffer in this case must be 4 bytes aligned, as
* \c dataBuffer is required to be 4 bytes aligned.
*/
#define CY_BOOTLOAD_OPT_SEND_DATA       (1)

/** A non-zero value enables the Get Metadata bootloader command */
#define CY_BOOTLOAD_OPT_GET_METADATA    (1)

/** A non-zero value enables the Set EI Vector bootloader command */
#define CY_BOOTLOAD_OPT_SET_EIVECTOR    (0)

/**
* A non-zero value allows writing metadata
* with the Set App Metadata bootloader command.
*/
#define CY_BOOTLOAD_METADATA_WRITABLE   (1)

/** Non-zero value enables the usage of hardware Crypto API */
#define CY_BOOTLOAD_OPT_CRYPTO_HW       (0)

/** A non-zero value enables the usage of CRC-16 for bootloader packet verification */
#define CY_BOOTLOAD_OPT_PACKET_CRC      (0)

/** Set the default application-format-possible values defined in \ref group_bootload_macro_app_type */
#define CY_BOOTLOAD_APP_FORMAT          (CY_BOOTLOAD_BASIC_APP)

/** Set the default secure application-verification-type possible values 
 * defined in \ref group_bootload_macro_ver_type */
#define CY_BOOTLOAD_SEC_APP_VERIFY_TYPE  (CY_BOOTLOAD_VERIFY_FAST)

/** \} group_bootload_macro_config */

#if !defined(CY_DOXYGEN)
    #if defined(__GNUC__) || defined(__ICCARM__)
        /*
        * These variables are defined in the linker scripts, the values of their addresses define
        * corresponding applications start address and length.
        */
        extern uint8_t __cy_app0_verify_start;
        extern uint8_t __cy_app0_verify_length;
        extern uint8_t __cy_app1_verify_start;
        extern uint8_t __cy_app1_verify_length;
        extern uint8_t __cy_boot_signature_size;

        #define CY_BOOTLOAD_APP0_VERIFY_START       ( (uint32_t)&__cy_app0_verify_start )
        #define CY_BOOTLOAD_APP0_VERIFY_LENGTH      ( (uint32_t)&__cy_app0_verify_length )
        #define CY_BOOTLOAD_APP1_VERIFY_START       ( (uint32_t)&__cy_app1_verify_start )
        #define CY_BOOTLOAD_APP1_VERIFY_LENGTH      ( (uint32_t)&__cy_app1_verify_length )
        #define CY_BOOTLOAD_SIGNATURE_SIZE          ( (uint32_t)&__cy_boot_signature_size )

    #elif defined(__ARMCC_VERSION)
        #include "bootload_mdk_common.h"
        
        #define CY_BOOTLOAD_APP0_VERIFY_START       ( CY_APP0_CORE0_FLASH_ADDR )
        #define CY_BOOTLOAD_APP0_VERIFY_LENGTH      ( CY_APP0_CORE0_FLASH_LENGTH + CY_APP0_CORE1_FLASH_LENGTH \
                                                    - __CY_BOOT_SIGNATURE_SIZE)
        #define CY_BOOTLOAD_APP1_VERIFY_START       ( CY_APP1_CORE0_FLASH_ADDR )
        #define CY_BOOTLOAD_APP1_VERIFY_LENGTH      ( CY_APP1_CORE0_FLASH_LENGTH + CY_APP1_CORE1_FLASH_LENGTH \
                                                    - __CY_BOOT_SIGNATURE_SIZE)
        #define CY_BOOTLOAD_SIGNATURE_SIZE          __CY_BOOT_SIGNATURE_SIZE

    #else
        #error "Not implemented for this compiler"
    #endif /* defined(__GNUC__) || defined(__ICCARM__) */
#endif /* !defined(CY_DOXYGEN) */


#endif /* !defined(BOOTLOAD_USER_H) */


/* [] END OF FILE */
