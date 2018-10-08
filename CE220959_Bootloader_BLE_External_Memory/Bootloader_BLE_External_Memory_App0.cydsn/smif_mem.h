/******************************************************************************
* File Name: smif_mem.h
*
* Version: 1.0
*
* This header file contains the defines for the routines to access SMIF memory.
*
* Related Document: CE220959.pdf
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer kit
*
******************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.
******************************************************************************
* This software, including source code, documentation and related materials
* ("Software") is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and 
* foreign), United States copyright laws and international treaty provisions. 
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the 
* Cypress source code and derivative works for the sole purpose of creating 
* custom software in support of licensee product, such licensee product to be
* used only in conjunction with Cypress's integrated circuit as specified in the
* applicable agreement. Any reproduction, modification, translation, compilation,
* or representation of this Software except as specified above is prohibited 
* without the express written permission of Cypress.
* 
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes to the Software without notice. 
* Cypress does not assume any liability arising out of the application or use
* of Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use as critical components in any products 
* where a malfunction or failure may reasonably be expected to result in 
* significant injury or death ("ACTIVE Risk Product"). By including Cypress's 
* product in a ACTIVE Risk Product, the manufacturer of such system or application
* assumes all risk of such use and in doing so indemnifies Cypress against all
* liability. Use of this Software may be limited by and subject to the applicable
* Cypress software license agreement.
*****************************************************************************/

#ifndef __SMIF_MEM_H
#define __SMIF_MEM_H

#include <cy_smif_memconfig.h>
    
/*******************************************************************************
*            Function Prototypes
*******************************************************************************/
void configureSMIF(SMIF_Type *base, cy_stc_smif_context_t *context); /* Initializes SMIF component */

void WriteMemory(
                    uint8_t txBuffer[], 	
                    uint32_t txSize, 	
                    uint32_t address);    	/* Program memory in the quad mode */
void ReadMemory(	
                    uint8_t rxBuffer[], 	
                    uint32_t rxSize, 	
                    uint32_t address);  	/* Read data from memory in the quad mode */

void SwitchSMIFMemory(void);                /* Switch to XIP mode */

void SwitchSMIFNormal(void);                /* Switch to Normal mode */

void EraseSMIFChip(void);                   /* Bulk erase the chip */

void EraseSMIFSector(uint32_t Address);     /* Erase a sector */

/*******************************************************************************
*            Constants
*******************************************************************************/

#define TIMEOUT_1_MS        (1000ul)  /* 1 ms timeout for all blocking functions */

#endif /*__SMIF_MEM_H*/
    
/* [] END OF FILE */

