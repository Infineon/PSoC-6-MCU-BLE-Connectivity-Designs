/******************************************************************************
* File Name: cy_smif_memconfig.c
*
* Version: 1.0
*
* Description: Provides a definitions of the SMIF driver memory configuration.
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

#include "cy_smif_memconfig.h"

cy_stc_smif_mem_cmd_t S25FL512S_0_readCmd =
{
    /**< 8 bit command. 1 x I/O read command */
    .command = 0xECU,
    /**< Width of command transfer */
    .cmdWidth = CY_SMIF_WIDTH_SINGLE,
    /**< Width of address transfer */
    .addrWidth = CY_SMIF_WIDTH_QUAD,
    /**< 8 bit mode byte. This value is 0xFFFFFFFF when there is no mode present */
    .mode = 0x01U,
    /**< Width of mode command transfer */
    .modeWidth = CY_SMIF_WIDTH_QUAD,
    /**< Number of dummy cycles. A value of zero suggest no dummy cycles */
    .dummyCycles = 4U,
    /**< Width of data transfer */
    .dataWidth = CY_SMIF_WIDTH_QUAD
};

cy_stc_smif_mem_cmd_t S25FL512S_0_writeEnCmd =
{
    /**< 8 bit command. 1 x I/O read command */
    .command = 0x06U,
    /**< Width of command transfer */
    .cmdWidth = CY_SMIF_WIDTH_SINGLE,
    /**< Width of address transfer */
    .addrWidth = CY_SMIF_WIDTH_SINGLE,
    /**< 8 bit mode byte. This value is 0xFFFFFFFF when there is no mode present */
    .mode = 0xFFFFFFFFU,
    /**< Width of mode command transfer */
    .modeWidth = CY_SMIF_WIDTH_SINGLE,
    /**< Number of dummy cycles. A value of zero suggest no dummy cycles */
    .dummyCycles = 0U,
    /**< Width of data transfer */
    .dataWidth = CY_SMIF_WIDTH_SINGLE
};

cy_stc_smif_mem_cmd_t S25FL512S_0_writeDisCmd =
{
    /**< 8 bit command. 1 x I/O read command */
    .command = 0x04U,
    /**< Width of command transfer */
    .cmdWidth = CY_SMIF_WIDTH_SINGLE,
    /**< Width of address transfer */
    .addrWidth = CY_SMIF_WIDTH_SINGLE,
    /**< 8 bit mode byte. This value is 0xFFFFFFFF when there is no mode present */
    .mode = 0xFFFFFFFFU,
    /**< Width of mode command transfer */
    .modeWidth = CY_SMIF_WIDTH_SINGLE,
    /**< Number of dummy cycles. A value of zero suggest no dummy cycles */
    .dummyCycles = 0U,
    /**< Width of data transfer */
    .dataWidth = CY_SMIF_WIDTH_SINGLE
};

cy_stc_smif_mem_cmd_t S25FL512S_0_eraseCmd =
{
    /**< 8 bit command. 1 x I/O read command */
    .command = 0xDCU,
    /**< Width of command transfer */
    .cmdWidth = CY_SMIF_WIDTH_SINGLE,
    /**< Width of address transfer */
    .addrWidth = CY_SMIF_WIDTH_SINGLE,
    /**< 8 bit mode byte. This value is 0xFFFFFFFF when there is no mode present */
    .mode = 0xFFFFFFFFU,
    /**< Width of mode command transfer */
    .modeWidth = CY_SMIF_WIDTH_SINGLE,
    /**< Number of dummy cycles. A value of zero suggest no dummy cycles */
    .dummyCycles = 0U,
    /**< Width of data transfer */
    .dataWidth = CY_SMIF_WIDTH_SINGLE
};

cy_stc_smif_mem_cmd_t S25FL512S_0_chipEraseCmd =
{
    /**< 8 bit command. 1 x I/O read command */
    .command = 0x60U,
    /**< Width of command transfer */
    .cmdWidth = CY_SMIF_WIDTH_SINGLE,
    /**< Width of address transfer */
    .addrWidth = CY_SMIF_WIDTH_SINGLE,
    /**< 8 bit mode byte. This value is 0xFFFFFFFF when there is no mode present */
    .mode = 0xFFFFFFFFU,
    /**< Width of mode command transfer */
    .modeWidth = CY_SMIF_WIDTH_SINGLE,
    /**< Number of dummy cycles. A value of zero suggest no dummy cycles */
    .dummyCycles = 0U,
    /**< Width of data transfer */
    .dataWidth = CY_SMIF_WIDTH_SINGLE
};

cy_stc_smif_mem_cmd_t S25FL512S_0_programCmd =
{
    /**< 8 bit command. 1 x I/O read command */
    .command = 0x34U,
    /**< Width of command transfer */
    .cmdWidth = CY_SMIF_WIDTH_SINGLE,
    /**< Width of address transfer */
    .addrWidth = CY_SMIF_WIDTH_SINGLE,
    /**< 8 bit mode byte. This value is 0xFFFFFFFF when there is no mode present */
    .mode = 0xFFFFFFFFU,
    /**< Width of mode command transfer */
    .modeWidth = CY_SMIF_WIDTH_QUAD,
    /**< Number of dummy cycles. A value of zero suggest no dummy cycles */
    .dummyCycles = 0U,
    /**< Width of data transfer */
    .dataWidth = CY_SMIF_WIDTH_QUAD
};

cy_stc_smif_mem_cmd_t S25FL512S_0_readStsRegQeCmd =
{
    /**< 8 bit command. 1 x I/O read command */
    .command = 0x35U,
    /**< Width of command transfer */
    .cmdWidth = CY_SMIF_WIDTH_SINGLE,
    /**< Width of address transfer */
    .addrWidth = CY_SMIF_WIDTH_SINGLE,
    /**< 8 bit mode byte. This value is 0xFFFFFFFF when there is no mode present */
    .mode = 0xFFFFFFFFU,
    /**< Width of mode command transfer */
    .modeWidth = CY_SMIF_WIDTH_SINGLE,
    /**< Number of dummy cycles. A value of zero suggest no dummy cycles */
    .dummyCycles = 0U,
    /**< Width of data transfer */
    .dataWidth = CY_SMIF_WIDTH_SINGLE
};

cy_stc_smif_mem_cmd_t S25FL512S_0_readStsRegWipCmd =
{
    /**< 8 bit command. 1 x I/O read command */
    .command = 0x05U,
    /**< Width of command transfer */
    .cmdWidth = CY_SMIF_WIDTH_SINGLE,
    /**< Width of address transfer */
    .addrWidth = CY_SMIF_WIDTH_SINGLE,
    /**< 8 bit mode byte. This value is 0xFFFFFFFF when there is no mode present */
    .mode = 0xFFFFFFFFU,
    /**< Width of mode command transfer */
    .modeWidth = CY_SMIF_WIDTH_SINGLE,
    /**< Number of dummy cycles. A value of zero suggest no dummy cycles */
    .dummyCycles = 0U,
    /**< Width of data transfer */
    .dataWidth = CY_SMIF_WIDTH_SINGLE
};

cy_stc_smif_mem_cmd_t S25FL512S_0_writeStsRegQeCmd =
{
    /**< 8 bit command. 1 x I/O read command */
    .command = 0x01U,
    /**< Width of command transfer */
    .cmdWidth = CY_SMIF_WIDTH_SINGLE,
    /**< Width of address transfer */
    .addrWidth = CY_SMIF_WIDTH_SINGLE,
    /**< 8 bit mode byte. This value is 0xFFFFFFFF when there is no mode present */
    .mode = 0xFFFFFFFFU,
    /**< Width of mode command transfer */
    .modeWidth = CY_SMIF_WIDTH_SINGLE,
    /**< Number of dummy cycles. A value of zero suggest no dummy cycles */
    .dummyCycles = 0U,
    /**< Width of data transfer */
    .dataWidth = CY_SMIF_WIDTH_SINGLE
};

cy_stc_smif_mem_device_cfg_t deviceCfg_S25FL512S_0 =
{
    /**< This specifies the number of address bytes used by the memory slave device */
    .numOfAddrBytes = 0x04U,
    /**< Size of the memory */
    .memSize = 0x4000000U,
    /**< This specifies the read command */
    .readCmd = &S25FL512S_0_readCmd,
    /**< This specifies the write enable command */
    .writeEnCmd = &S25FL512S_0_writeEnCmd,
    /**< This specifies the write disable command */
    .writeDisCmd = &S25FL512S_0_writeDisCmd,
    /**< This specifies the erase command */
    .eraseCmd = &S25FL512S_0_eraseCmd,
    /**< This specifies the sector size of each erase */
    .eraseSize = 0x0040000U,
    /**< This specifies the chip erase command */
    .chipEraseCmd = &S25FL512S_0_chipEraseCmd,
    /**< This specifies the program command */
    .programCmd = &S25FL512S_0_programCmd,
    /**< This specifies the page size for programming */
    .programSize = 0x0000200U,
    /**< This specifies the command to read the QE-containing status register */
    .readStsRegQeCmd = &S25FL512S_0_readStsRegQeCmd,
    /**< This specifies the command to read the WIP-containing status register */
    .readStsRegWipCmd = &S25FL512S_0_readStsRegWipCmd,
    /**< This specifies the command to write into the QE-containing status register */
    .writeStsRegQeCmd = &S25FL512S_0_writeStsRegQeCmd,
    /**< Mask for the status register */
    .stsRegBusyMask = 0x01U,
    /**< Mask for the status register */
    .stsRegQuadEnableMask = 0x02U,
    /**< Max time for erase type 1 cycle time in ms */
    .eraseTime = 520U,
    /**< Max time for chip erase cycle time in ms */
    .chipEraseTime = 134000U,
    /**< Max time for page program cycle time in us */
    .programTime = 340U
};

const cy_stc_smif_mem_config_t S25FL512S_SlaveSlot_0 =
{
    /**< Determines the slot number where the memory device is placed */
    .slaveSelect = CY_SMIF_SLAVE_SELECT_0,
    /**< Flags */
    .flags = CY_SMIF_FLAG_MEMORY_MAPPED | CY_SMIF_FLAG_WR_EN,
    /**< Data line selection options for a slave device */
    .dataSelect = CY_SMIF_DATA_SEL0,
    /**< The base address the memory slave is mapped to in the PSoC memory map.
    Valid when memory mapped mode is enabled */
    .baseAddress = 0x18000000U,
    /**< The size allocated in the PSoC memory map, for the memory slave device.
    The size is allocated from the base address Valid when memory mapped mode is enabled */
    .memMappedSize = 0x4000000U,
    /**< Is this memory device one of the devices in a dual quad SPI configuration.
    Valid when memory mapped mode is enabled */
    .dualQuadSlots = 0,
    /**< Configuration of the device */
    .deviceCfg = &deviceCfg_S25FL512S_0
};

const cy_stc_smif_mem_config_t* smifMemConfigs[] = {
   &S25FL512S_SlaveSlot_0
};

const cy_stc_smif_block_config_t smifBlockConfig =
{
    /* Number of SMIF memories defined  */
    .memCount = CY_SMIF_DEVICE_NUM,
    /* pointer to the array of memory config structures of size memCount */
    .memConfig = (cy_stc_smif_mem_config_t**)smifMemConfigs,
    /* Version of the SMIF driver */
    .majorVersion = CY_SMIF_DRV_VERSION_MAJOR,
    /* version of the SMIF Driver */
    .minorVersion = CY_SMIF_DRV_VERSION_MINOR
};



