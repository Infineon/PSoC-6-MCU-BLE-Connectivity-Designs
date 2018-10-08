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
#include "smif_mem.h"


/*
* The Bootloader SDK metadata initial value is placed here
* Note: the number of elements equal to the number of the app multiplies by 2 
*       because of the two fields per app plus one element for the CRC-32C field.
*/
CY_SECTION(".cy_boot_metadata") __USED
static const uint32_t cy_bootload_metadata[CY_FLASH_SIZEOF_ROW / sizeof(uint32_t)] =
{
    CY_BOOTLOAD_APP0_VERIFY_START, CY_BOOTLOAD_APP0_VERIFY_LENGTH, /* The App0 base address and length       */
    CY_BOOTLOAD_APP1_VERIFY_START, CY_BOOTLOAD_APP1_VERIFY_LENGTH, /* The App1 base address and length       */
    CY_BOOTLOAD_APP1_VERIFY_START, CY_BOOTLOAD_APP1_VERIFY_LENGTH, /* Temporary App1 base address and length */
    0u                                                             /* The rest does not matter               */
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
* Function Name: EraseExternalApp
****************************************************************************//**
*
* Erases the sectors of the external memory, where the application is stored.
* 
* /params
* None
*
*******************************************************************************/
cy_en_bootload_status_t EraseExternalApp(void)
{
    /* Get Sector Size from External Memory Configuration */
    uint32_t SectorSize = deviceCfg_S25FL512S_0.eraseSize;
    uint32_t appSize;
    
    /* Start address is assumed to be start of external memory */
    cy_en_bootload_status_t status = Cy_Bootload_GetAppMetadata(2u, NULL, &appSize);
    
    if(status == CY_BOOTLOAD_SUCCESS)
    {
        if(appSize > 0)
        {
            appSize += CY_BOOTLOAD_SIGNATURE_SIZE;
            /* Convert appSize into sectors */
            appSize = (appSize - 1) / SectorSize;
            for(uint16_t i = 0; i <= appSize; i++)
            {
                EraseSMIFSector(SectorSize * i);
            }
        }
        else
        {
            status = CY_BOOTLOAD_ERROR_ADDRESS;
        }
    }
    return status;
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
    const uint32_t minUFlashAddress = CY_FLASH_BASE + CY_BOOTLOAD_APP0_VERIFY_LENGTH;
    const uint32_t maxUFlashAddress = CY_FLASH_BASE + CY_FLASH_SIZE;
    /* EM_EEPROM Limits*/
    const uint32_t minEmEepromAddress = CY_EM_EEPROM_BASE;
    const uint32_t maxEmEepromAddress = CY_EM_EEPROM_BASE + CY_EM_EEPROM_SIZE;
    /* XIP Limits */
    const uint32_t minXIPAddress = CY_XIP_BASE;
    const uint32_t maxXIPAddress = CY_XIP_BASE + CY_XIP_SIZE;
    
    cy_en_bootload_status_t status = CY_BOOTLOAD_SUCCESS;
    
    uint32_t app = Cy_Bootload_GetRunningApp();
    uint32_t startAddress;
    uint32_t endAddress;
    
    GetStartEndAddress(app, &startAddress, &endAddress);
    
    /* 
    * Only shift address to the external memory if the address to write
    * is outside App0 and metadata section, in User Flash, and AppID == 2.
    */
    if(params->appId == 2u)
    {
        /* If address is in valid user flash and outside of metadata */
        if ( (minUFlashAddress <= address) && (address < maxUFlashAddress)
            && ( ((uint32_t)&__cy_boot_metadata_addr > address) || 
            (((uint32_t)&__cy_boot_metadata_addr + (uint32_t)&__cy_boot_metadata_length) < address)) )
        {
             uint32_t startAddress;
            /* Use the updated metadata address for start address */
            status = Cy_Bootload_GetAppMetadata(2u, &startAddress, NULL);
            if(status == CY_BOOTLOAD_SUCCESS)
            {
                address = (address - startAddress) + minXIPAddress;
            }
            else
            {
                status = CY_BOOTLOAD_ERROR_ADDRESS;
            }
        }
    }

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

    if (status == CY_BOOTLOAD_SUCCESS)
    {   
        /* Check if address is inside a valid range */
        if ( ( (minUFlashAddress <= address) && (address < maxUFlashAddress) ) 
          || ( (minEmEepromAddress <= address) && (address < maxEmEepromAddress) )  )
        {
            if ((ctl & CY_BOOTLOAD_IOCTL_ERASE) != 0u)
            {
                (void) memset(params->dataBuffer, 0, CY_FLASH_SIZEOF_ROW);
            }
            cy_en_flashdrv_status_t fstatus =  Cy_Flash_WriteRow(address, (uint32_t*)params->dataBuffer);
            status = (fstatus == CY_FLASH_DRV_SUCCESS) ? CY_BOOTLOAD_SUCCESS : CY_BOOTLOAD_ERROR_DATA;
        }
        else if ( (minXIPAddress <= address) && (address < maxXIPAddress) )
        {
            /*
            * Check if address to write is the beginning of a new App
            * If it is, then delete the application before writing.
            */
            if(address == minXIPAddress)
            {
                status = EraseExternalApp();
            }
            if(status == CY_BOOTLOAD_SUCCESS)
            {
                WriteMemory(params->dataBuffer, length, address - minXIPAddress);
            }
        }
        else
        {
            status = CY_BOOTLOAD_ERROR_ADDRESS;
        }
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
    /* XIP Limits */
    const uint32_t minXIPAddress = CY_XIP_BASE;
    const uint32_t maxXIPAddress = CY_XIP_BASE + CY_XIP_SIZE;
    
    cy_en_bootload_status_t status = CY_BOOTLOAD_SUCCESS;

    /*
    * Only shift address to the external memory if the address to write
    * is outside App0 and metadata section, in User Flash, and AppID == 2.
    */
    /* 
    * Only shift address to the external memory if the address to write
    * is outside App0 and metadata section, in User Flash, and AppID == 2.
    */
    if(params->appId == 2u)
    {
        /* If address is in valid user flash and outside of metadata */
        if ( (minUFlashAddress <= address) && (address < maxUFlashAddress)
            && ( ((uint32_t)&__cy_boot_metadata_addr > address) || 
            (((uint32_t)&__cy_boot_metadata_addr + (uint32_t)&__cy_boot_metadata_length) < address)) )
        {
             uint32_t startAddress;
            /* Use the updated metadata address for start address */
            status = Cy_Bootload_GetAppMetadata(2u, &startAddress, NULL);
            if(status == CY_BOOTLOAD_SUCCESS)
            {
                address = (address - startAddress) + minXIPAddress;
            }
            else
            {
                status = CY_BOOTLOAD_ERROR_ADDRESS;
            }
        }
    }

    /* Check if length is valid */
    if ( (IsMultipleOf(length, CY_FLASH_SIZEOF_ROW) == 0u) && ((ctl & CY_BOOTLOAD_IOCTL_ERASE) == 0u) )
    {
        status = CY_BOOTLOAD_ERROR_LENGTH;   
    }
    
    if(status == CY_BOOTLOAD_SUCCESS)
    {
        /* Check if address is inside a valid range */
        if ( ( (minUFlashAddress <= address) && (address < maxUFlashAddress) ) 
          || ( (minEmEepromAddress <= address) && (address < maxEmEepromAddress) )  )
        {
            /* Read or Compare */
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
        /* Check if address is in the external memory */
        else if ((minXIPAddress <= address) && (address < maxXIPAddress))
    	{
    		if ((ctl & CY_BOOTLOAD_IOCTL_COMPARE) == 0u)
    		{
    		    ReadMemory(params->dataBuffer, length, address - minXIPAddress);
    		}
    		else
    		{
    		    uint8_t buffer[CY_BOOTLOAD_SIZEOF_DATA_BUFFER];
    		    ReadMemory(&buffer[0], length, address - minXIPAddress);
    		    status = ( memcmp(params->dataBuffer, &buffer[0], length) == 0 )
    			     ? CY_BOOTLOAD_SUCCESS : CY_BOOTLOAD_ERROR_VERIFY;
    		}
	    }
        else
        {
            status = CY_BOOTLOAD_ERROR_ADDRESS;   
        }
    }
    return (status);
}


/*******************************************************************************
* Function Name: Cy_Bootload_ValidateApp
****************************************************************************//**
*
* Modified implementation of this weak function. Adds the proper functions
* to read from the external memory.
*
* \note It is assumed appId is valid application number.
*
* \param appId      An application number of the application to be validated.
*
* \param params     A pointer to a bootloader parameters structure.
*                   See \ref cy_stc_bootload_params_t .
* \returns
* - \ref CY_BOOTLOAD_SUCCESS if application is valid.
* - \ref CY_BOOTLOAD_ERROR_VERIFY if application in invalid.
*
*******************************************************************************/
cy_en_bootload_status_t Cy_Bootload_ValidateApp(uint32_t appId, cy_stc_bootload_params_t *params)
{
    const uint32_t minXIPAddress = CY_XIP_BASE;
    const uint32_t maxXIPAddress = CY_XIP_BASE + CY_XIP_SIZE;
    uint32_t appStartAddress;
    uint32_t appSize;
    
    (void)params;
    
    CY_ASSERT(appId < CY_BOOTLOAD_MAX_APPS);
    
    cy_en_bootload_status_t status = Cy_Bootload_GetAppMetadata(appId, &appStartAddress, &appSize);
    
    /* If verifying external, start address is the start address of the external memory */
    if(appId == 2u)
    {
        appStartAddress = minXIPAddress;
        /* 
        * Switch to XIP mode to enable direct access and avoid reimplementing the crc algorithm.
        * If memory mode is unavailable, the crc algorithm must be implemented to work in chunks.
        */
        SwitchSMIFMemory();
    }
    
    if (status == CY_BOOTLOAD_SUCCESS)
    {
        /* Calculate CRC */
        uint32_t appCrc = Cy_Bootload_DataChecksum((uint8_t *)appStartAddress, appSize, params);
        uint32_t appFooterAddress = appStartAddress + appSize;

        status = (*(uint32_t*)appFooterAddress == appCrc) ? CY_BOOTLOAD_SUCCESS : CY_BOOTLOAD_ERROR_VERIFY;
        
        if((minXIPAddress <= appFooterAddress) && (appFooterAddress < maxXIPAddress))
        {
            /* Return to normal SMIF mode, to use regular read/write functions */
            SwitchSMIFNormal();
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
