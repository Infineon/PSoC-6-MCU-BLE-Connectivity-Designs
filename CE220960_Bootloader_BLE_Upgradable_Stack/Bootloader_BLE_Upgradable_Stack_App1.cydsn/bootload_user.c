/***************************************************************************//**
* \file bootload_user.c
* \version 2.10
* 
* This file provides the custom API for a firmware application with 
* Bootloader SDK.
* - Cy_Bootload_ReadData (address, length, ctl, params) - to read  the NVM block 
* - Cy_Bootalod_WriteData(address, length, ctl, params) - to write the NVM block
*
* - Cy_Bootload_TransportStart() to start a communication interface
* - Cy_Bootload_TransportStop () to stop  a communication interface
* - Cy_Bootload_TransportReset() to reset a communication interface
* - Cy_Bootload_TransportRead (buffer, size, count, timeout)
* - Cy_Bootload_TransportWrite(buffer, size, count, timeout)
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <string.h>
#include "transport_ble.h"
#include "syslib/cy_syslib.h"
#include "flash/cy_flash.h"
#include "bootloader/cy_bootload.h"


static uint32_t IsMultipleOf(uint32_t value, uint32_t multiple);
static void GetStartEndAddress(uint32_t appId, uint32_t *startAddress, uint32_t *endAddress);


/*******************************************************************************
* Function Name: IsMultipleOf
****************************************************************************//**
*
* This internal function check if value parameter is a multiple of parameter 
* multiple
*
* \param value      value that will be checked
* \param multiple   value with which value is checked 
*
* \return 1 - value is multiple of parameter multiple, else 0  
*
*******************************************************************************/
static uint32_t IsMultipleOf(uint32_t value, uint32_t multiple)
{
    return ( ((value % multiple) == 0u)? 1ul : 0ul);
}


/*******************************************************************************
* Function Name: GetStartEndAddress
****************************************************************************//**
*
* This internal function returns start and end address of application
*
* \param appId          The application number
* \param startAddress   The pointer to a variable where an application start
*                       address is stored
* \param endAddress     The pointer to a variable where a size of application 
*                       area is stored.
*
*******************************************************************************/
static void GetStartEndAddress(uint32_t appId, uint32_t *startAddress, uint32_t *endAddress)
{
    uint32_t verifyStart;
    uint32_t verifySize;

    (void)Cy_Bootload_GetAppMetadata(appId, &verifyStart, &verifySize);

#if (CY_BOOTLOAD_APP_FORMAT == CY_BOOTLOAD_SIMPLIFIED_APP)
    *startAddress = verifyStart - CY_BOOTLOAD_SIGNATURE_SIZE; 
    *endAddress = verifyStart + verifySize;
#else
    *startAddress = verifyStart; 
    *endAddress = verifyStart + verifySize + CY_BOOTLOAD_SIGNATURE_SIZE;
#endif
}


/*******************************************************************************
* Function Name: Cy_Bootload_WriteData
****************************************************************************//**
*
* This function documentation is part of the Bootloader SDK API, see the 
* cy_bootload.h file or Bootloader SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_bootload_status_t Cy_Bootload_WriteData (uint32_t address, uint32_t length, uint32_t ctl, 
                                               cy_stc_bootload_params_t *params)
{
    /* User Flash Limits */
    /* Note that App0 is out of range */
    const uint32_t minUFlashAddress = CY_FLASH_BASE + CY_BOOTLOAD_APP0_VERIFY_LENGTH + CY_BOOTLOAD_SIGNATURE_SIZE;
    const uint32_t maxUFlashAddress = CY_FLASH_BASE + CY_FLASH_SIZE;
    /* EM_EEPROM Limits*/
    const uint32_t minEmEepromAddress = CY_EM_EEPROM_BASE;
    const uint32_t maxEmEepromAddress = CY_EM_EEPROM_BASE + CY_EM_EEPROM_SIZE;
    
    cy_en_bootload_status_t status = CY_BOOTLOAD_SUCCESS;

    uint32_t app = 1u;
    uint32_t startAddress;
    uint32_t endAddress;
    
    GetStartEndAddress(app, &startAddress, &endAddress);

    /* Check if the address  and length are valid 
     * Note Length = 0 is valid for erase command */
    if ( (IsMultipleOf(address, CY_FLASH_SIZEOF_ROW) == 0u) || 
         ( (length != CY_FLASH_SIZEOF_ROW) && ( (ctl & CY_BOOTLOAD_IOCTL_ERASE) == 0u) ) )
    {
        status = CY_BOOTLOAD_ERROR_LENGTH;   
    }

    if(status == CY_BOOTLOAD_SUCCESS)
    {
        /* Check for writes into metadata */
        if ( (((uint32_t)&__cy_boot_metadata_addr <= address) && 
            (((uint32_t)&__cy_boot_metadata_addr + (uint32_t)&__cy_boot_metadata_length) > address)) )
        {
            /* Check if we are receiving a stack update. */
            const uint8_t METADATA_BYTES_PER_APP = 8u;
            const uint8_t METADATA_APP_LENGTH_OFFSET = 4u;
            const uint8_t CRC_CHECKSUM_LENGTH = 4u;
            uint32_t vApp3Base, vApp3Length, getvApp3Base, getvApp3Length;
            
            Cy_Bootload_GetAppMetadata(3u, &getvApp3Base, &getvApp3Length);
            
            memcpy(&vApp3Base, &params->dataBuffer[(3 * METADATA_BYTES_PER_APP)], sizeof(uint32_t));
            memcpy(&vApp3Length, &params->dataBuffer[(3 * METADATA_BYTES_PER_APP) + METADATA_APP_LENGTH_OFFSET],
                   sizeof(uint32_t));
            
            if( (vApp3Base == 0xFFFFFFFF) && (vApp3Length == 0x00000000) )
            {
                /* Allow writing dummy data to vApp3 */
            }
            else if( (vApp3Base != getvApp3Base) || (vApp3Length != getvApp3Length) )
            {
                /* 
                * Metadata is being updated because of a Stack update. Modify metadata so that the
                * Virtual App3 points to the temporal location and virtual App4 points to the 
                * destination after being copied.
                */                
                uint32_t temporalLocation;
                uint32_t crc;
                /* Copy metadata information from vApp3 to vApp4 */
                memcpy( &params->dataBuffer[(4 * METADATA_BYTES_PER_APP)], 
                        &params->dataBuffer[(3 * METADATA_BYTES_PER_APP)], METADATA_BYTES_PER_APP);

                /* 
                *  Set temporal location as App2 start address. If another address is desired
                *  to store the stack temporarilly. Remove the following line and set the
                *  desired address to temporalLocation variable.
                */
                Cy_Bootload_GetAppMetadata(2u, &temporalLocation, NULL);
                
                memcpy( &params->dataBuffer[(3 * METADATA_BYTES_PER_APP)], &temporalLocation, sizeof(uint32_t));
                
                /* Calculate new CRC of metadata row */
                crc = Cy_Bootload_DataChecksum(params->dataBuffer, 
                        (uint32_t)&__cy_boot_metadata_length - CRC_CHECKSUM_LENGTH, params);
                memcpy( &params->dataBuffer[(uint32_t)&__cy_boot_metadata_length - CRC_CHECKSUM_LENGTH], 
                        &crc, sizeof(uint32_t));
            }
        }
        else
        {
            if(params->appId == 3u)
            {
                /* 
                * If AppID is 3, we are receiving a Stack update, 
                * move address to temporal memory space for storing. 
                */
                uint32_t temporalLocation;
                uint32_t updateBaseAddress;
                Cy_Bootload_GetAppMetadata(3u, &temporalLocation, NULL);
                Cy_Bootload_GetAppMetadata(4u, &updateBaseAddress, NULL);
                address = (address - updateBaseAddress) + temporalLocation;
            }
            
            /* Check if the address is inside the valid range */
            if ( ( (minUFlashAddress <= address) && (address < maxUFlashAddress) ) 
              || ( (minEmEepromAddress <= address) && (address < maxEmEepromAddress) )  )
            {   
                /* Check if address is inside the bootloader memory space */
                uint32_t BootloaderBaseAddress;
                uint32_t BootloaderEndAddress;
                Cy_Bootload_GetAppMetadata(1u, &BootloaderBaseAddress, &BootloaderEndAddress);
                BootloaderEndAddress = BootloaderBaseAddress + BootloaderEndAddress + CY_BOOTLOAD_SIGNATURE_SIZE;
                if ( (BootloaderBaseAddress <= address) && ( address < BootloaderEndAddress ) )
                {
                    /* It is forbidden to overwrite the currently running application */
                    status = CY_BOOTLOAD_ERROR_ADDRESS;
                }
                /* Else: Do nothing, this is an allowed memory range to bootload to */
            }
            else
            {
                status = CY_BOOTLOAD_ERROR_ADDRESS;   
            }
        }
    }

    /* Refuse to write to a row within a range of the current application */ 
    if ( (startAddress <= address) && (address < endAddress) )
    {   /* It is forbidden to overwrite the currently running application */
        status = CY_BOOTLOAD_ERROR_ADDRESS;
    }
#if CY_BOOTLOAD_OPT_GOLDEN_IMAGE
    if (status == CY_BOOTLOAD_SUCCESS)
    {
        uint8_t goldenImages[] = { CY_BOOTLOAD_GOLDEN_IMAGE_IDS() };
        uint32_t count = sizeof(goldenImages) / sizeof(goldenImages[0]);
        uint32_t idx;
        for (idx = 0u; idx < count; ++idx)
        {
            app = goldenImages[idx];
            GetStartEndAddress(app, &startAddress, &endAddress);

            if ( (startAddress <= address) && (address < endAddress) )
            {
                status = Cy_Bootload_ValidateApp(app, params);
                status = (status == CY_BOOTLOAD_SUCCESS) ? CY_BOOTLOAD_ERROR_ADDRESS : CY_BOOTLOAD_SUCCESS;
                break;
            }
        }
    } 
#endif /* #if CY_BOOTLOAD_OPT_GOLDEN_IMAGE != 0 */     
    
    if (status == CY_BOOTLOAD_SUCCESS)
    {
        if ((ctl & CY_BOOTLOAD_IOCTL_ERASE) != 0u)
        {
            (void) memset(params->dataBuffer, 0, CY_FLASH_SIZEOF_ROW);
        }
        cy_en_flashdrv_status_t fstatus =  Cy_Flash_WriteRow(address, (uint32_t*)params->dataBuffer);
        status = (fstatus == CY_FLASH_DRV_SUCCESS) ? CY_BOOTLOAD_SUCCESS : CY_BOOTLOAD_ERROR_DATA;
    }
    return (status);
}


/*******************************************************************************
* Function Name: Cy_Bootload_ReadData
****************************************************************************//**
*
* This function documentation is part of the Bootloader SDK API, see the 
* cy_bootload.h file or Bootloader SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_bootload_status_t Cy_Bootload_ReadData (uint32_t address, uint32_t length, uint32_t ctl, 
                                              cy_stc_bootload_params_t *params)
{
    /* User Flash Limits */
    /* Note that App0 is out of range */
    const uint32_t minUFlashAddress = CY_FLASH_BASE + CY_BOOTLOAD_APP0_VERIFY_LENGTH + CY_BOOTLOAD_SIGNATURE_SIZE;
    const uint32_t maxUFlashAddress = CY_FLASH_BASE + CY_FLASH_SIZE;
    /* EM_EEPROM Limits*/
    const uint32_t minEmEepromAddress = CY_EM_EEPROM_BASE;
    const uint32_t maxEmEepromAddress = CY_EM_EEPROM_BASE + CY_EM_EEPROM_SIZE;
    
    cy_en_bootload_status_t status = CY_BOOTLOAD_SUCCESS;

    /* Check if the length is valid */
    if (IsMultipleOf(length, CY_FLASH_SIZEOF_ROW) == 0u) 
    {
        status = CY_BOOTLOAD_ERROR_LENGTH;   
    }
    
    if(status == CY_BOOTLOAD_SUCCESS)
    {
        /* 
        * If AppID is 3, we are receiving a Stack update, 
        * move address to App2 memory space for reading. 
        */
        if(params->appId == 3u)
        {
            /* If address is in valid user flash and outside of metadata */
            if ( (minUFlashAddress <= address) && (address < maxUFlashAddress)
                && ( ((uint32_t)&__cy_boot_metadata_addr > address) || 
                (((uint32_t)&__cy_boot_metadata_addr + (uint32_t)&__cy_boot_metadata_length) < address)) )
            {
                /* Receiving stack update data, move address for comparing */
                uint32_t temporalLocation;
                uint32_t updateBaseAddress;
                Cy_Bootload_GetAppMetadata(3u, &temporalLocation, NULL);
                Cy_Bootload_GetAppMetadata(4u, &updateBaseAddress, NULL);
                address = (address - updateBaseAddress) + temporalLocation;
            }
        }
        /* Check if the address is inside the valid range */
        if ( ( (minUFlashAddress <= address) && (address < maxUFlashAddress) ) 
          || ( (minEmEepromAddress <= address) && (address < maxEmEepromAddress) )  )
        {   /* Do nothing, this is an allowed memory range to bootload to */
        }
        else
        {
            status = CY_BOOTLOAD_ERROR_ADDRESS;   
        }
    }

    /* Read or Compare */
    if (status == CY_BOOTLOAD_SUCCESS)
    {
        if ((ctl & CY_BOOTLOAD_IOCTL_COMPARE) == 0u)
        {
            (void) memcpy(params->dataBuffer, (const void *)address, length);
            status = CY_BOOTLOAD_SUCCESS;
        }
        else
        {
            status = ( memcmp(params->dataBuffer, (const void *)address, length) == 0 )
                     ? CY_BOOTLOAD_SUCCESS : CY_BOOTLOAD_ERROR_VERIFY;
        }
    }
    return (status);
}


/*******************************************************************************
* Function Name: Cy_Bootload_TransportRead
****************************************************************************//**
*
* This function documentation is part of the Bootloader SDK API, see the 
* cy_bootload.h file or Bootloader SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_bootload_status_t Cy_Bootload_TransportRead (uint8_t *buffer, uint32_t size, uint32_t *count, uint32_t timeout)
{
    return (CyBLE_CyBtldrCommRead(buffer, size, count, timeout));
}

/*******************************************************************************
* Function Name: Cy_Bootload_TransportWrite
****************************************************************************//**
*
* This function documentation is part of the Bootloader SDK API, see the 
* cy_bootload.h file or Bootloader SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_bootload_status_t Cy_Bootload_TransportWrite(uint8_t *buffer, uint32_t size, uint32_t *count, uint32_t timeout)
{
    return (CyBLE_CyBtldrCommWrite(buffer, size, count, timeout));
}

/*******************************************************************************
* Function Name: Cy_Bootload_TransportReset
****************************************************************************//**
*
* This function documentation is part of the Bootloader SDK API, see the 
* cy_bootload.h file or Bootloader SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_Bootload_TransportReset(void)
{
    CyBLE_CyBtldrCommReset();
}

/*******************************************************************************
* Function Name: Cy_Bootload_TransportStart
****************************************************************************//**
*
* This function documentation is part of the Bootloader SDK API, see the 
* cy_bootload.h file or Bootloader SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_Bootload_TransportStart(void)
{
    CyBLE_CyBtldrCommStart();
}

/*******************************************************************************
* Function Name: Cy_Bootload_TransportStop
****************************************************************************//**
*
* This function documentation is part of the Bootloader SDK API, see the 
* cy_bootload.h file or Bootloader SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_Bootload_TransportStop(void)
{
    CyBLE_CyBtldrCommStop();
}


/* [] END OF FILE */
