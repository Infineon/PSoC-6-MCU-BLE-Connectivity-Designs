/******************************************************************************
* File Name: cy_smif_memconfig.h
*
* Version: 1.0
*
* Description: Provides a declarations of the SMIF driver memory configuration.
*
* Related Document: CE220959.pdf
*                   See also CE220823
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

#ifndef CY_SMIF_MEMCONFIG_H
#define CY_SMIF_MEMCONFIG_H
#include "smif/cy_smif_memslot.h"

#define CY_SMIF_DEVICE_NUM 1

extern cy_stc_smif_mem_cmd_t S25FL512S_0_readCmd;
extern cy_stc_smif_mem_cmd_t S25FL512S_0_writeEnCmd;
extern cy_stc_smif_mem_cmd_t S25FL512S_0_writeDisCmd;
extern cy_stc_smif_mem_cmd_t S25FL512S_0_eraseCmd;
extern cy_stc_smif_mem_cmd_t S25FL512S_0_chipEraseCmd;
extern cy_stc_smif_mem_cmd_t S25FL512S_0_programCmd;
extern cy_stc_smif_mem_cmd_t S25FL512S_0_readStsRegQeCmd;
extern cy_stc_smif_mem_cmd_t S25FL512S_0_readStsRegWipCmd;
extern cy_stc_smif_mem_cmd_t S25FL512S_0_writeStsRegQeCmd;

extern cy_stc_smif_mem_device_cfg_t deviceCfg_S25FL512S_0;

extern const cy_stc_smif_mem_config_t S25FL512S_0;

extern const cy_stc_smif_mem_config_t* smifMemConfigs[CY_SMIF_DEVICE_NUM];

extern const cy_stc_smif_block_config_t smifBlockConfig;


#endif /*CY_SMIF_MEMCONFIG_H*/

