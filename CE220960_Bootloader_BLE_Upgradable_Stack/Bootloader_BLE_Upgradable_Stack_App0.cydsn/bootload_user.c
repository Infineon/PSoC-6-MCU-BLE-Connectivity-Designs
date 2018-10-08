/***************************************************************************//**
* \file bootload_user.c
* \version 2.10
* 
* This file provides the custom API for a firmware application with 
* Bootloader SDK.
* - Cy_Bootload_ReadData (address, length, ctl, params) - to read  the NVM block 
* - Cy_Bootalod_WriteData(address, length, ctl, params) - to write the NVM block
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <string.h>
#include "syslib/cy_syslib.h"
#include "flash/cy_flash.h"
#include "bootloader/cy_bootload.h"


/*
* The Bootloader SDK metadata initial value is placed here
* Note: the number of elements equal to the number of the app multiplies by 2 
*       because of the two fields per app plus one element for the CRC-32C field.
*/
CY_SECTION(".cy_boot_metadata") __USED
static const uint32_t cy_bootload_metadata[CY_FLASH_SIZEOF_ROW / sizeof(uint32_t)] =
{
    CY_BOOTLOAD_APP0_VERIFY_START, CY_BOOTLOAD_APP0_VERIFY_LENGTH, /* The App0 base address and length */
    CY_BOOTLOAD_APP1_VERIFY_START, CY_BOOTLOAD_APP1_VERIFY_LENGTH, /* The App1 base address and length */
    CY_BOOTLOAD_APP2_VERIFY_START, CY_BOOTLOAD_APP2_VERIFY_LENGTH, /* The App2 base address and length */
                                                                   /* Initialize vApps with dummy data */
    0xFFFFFFFF, 0x00000000,                                        /* vApp3 stores the temporal stack update location */
    0xFFFFFFFF, 0x00000000,                                        /* vApp4 stores stack update destination */
    0u                                                             /* The rest does not matter     */
};


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

    uint32_t app = Cy_Bootload_GetRunningApp();
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
    
    /* Check if the address is inside the valid range */
    if ( ( (minUFlashAddress <= address) && (address < maxUFlashAddress) ) 
      || ( (minEmEepromAddress <= address) && (address < maxEmEepromAddress) )  )
    {   /* Do nothing, this is an allowed memory range to bootload to */
    }
    else
    {
        status = CY_BOOTLOAD_ERROR_ADDRESS;   
    }
    
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
    const uint32_t minUFlashAddress = CY_FLASH_BASE + CY_BOOTLOAD_APP0_VERIFY_LENGTH;
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

    /* Check if the address is inside the valid range */
    if ( ( (minUFlashAddress <= address) && (address < maxUFlashAddress) ) 
      || ( (minEmEepromAddress <= address) && (address < maxEmEepromAddress) )  )
    {   /* Do nothing, this is an allowed memory range to bootload to */
    }
    else
    {
        status = CY_BOOTLOAD_ERROR_ADDRESS;   
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

/* [] END OF FILE */
