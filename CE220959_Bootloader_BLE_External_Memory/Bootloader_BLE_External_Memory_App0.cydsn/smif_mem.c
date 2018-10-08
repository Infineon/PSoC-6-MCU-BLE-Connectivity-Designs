/******************************************************************************
* File Name: smif_mem.c
*
* Version: 1.0
*
* Description: Functions in this file implement routines to access SMIF memory
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

#include "smif_mem.h"
#include "project.h"

/* Macro to wait until a next operation can be issued */
#define WaitMemBusy(Hardware, Context)  while(Cy_SMIF_Memslot_IsBusy(Hardware, (cy_stc_smif_mem_config_t*)smifMemConfigs[0], Context)){}

/* Local functions */
void SetSMIFPointers(SMIF_Type *base, cy_stc_smif_context_t *context); /* Sets local pointers */

/* Pointers must be initialized before using component, otherwise a fault will occur */
static SMIF_Type* SMIFHardware;
static cy_stc_smif_context_t* SMIFcontext;

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
*
* This function processes unrecoverable errors such as UART component 
* initialization error or SMIF initialization error etc. In case of such error 
* the system will stay in the infinite loop of this function.
*
* \param
*  None
*
* \return
*  None
*
*******************************************************************************/
void handle_error(void)
{
     /* Disable all interrupts */
    __disable_irq();
	
    /* Handle SMIF Error */
    while(1u) 
    {}
}

/*******************************************************************************
* Function Name: RxCmpltCallback
********************************************************************************
*
*   Callback function for the SMIF interrupt. Receives events.
*
* \param
*  uint32_t event: Event received from SMIF interrupt
*
* \return
*  None
*
*******************************************************************************/
void RxCmpltCallback (uint32_t event)
{
    if(0u == event)
    {
        /*The process event is 0*/
    }
}

/*******************************************************************************
* Function Name: configureSMIF
********************************************************************************
*
* Summary:
*  This function initializes the SMIF component, sets up the interrupt,
*  enables the SMIF cache, and sets up global pointers within shared RAM
*  to allow use of the SMIF component between both cores.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void configureSMIF(SMIF_Type *base, cy_stc_smif_context_t *context)
{
    /* Configure the SMIF component. */
    Cy_SMIF_Init(base , &SMIF_config, TIMEOUT_1_MS, context);

    /* Configure the interrupt sources. */
    Cy_SMIF_SetInterruptMask(base , SMIF_SMIF_INTR_MASK);
    
    /* Configure the trigger levels. */
    Cy_SMIF_SetTxFifoTriggerLevel(base , SMIF_TX_FIFO_TRIGEER_LEVEL);
    Cy_SMIF_SetRxFifoTriggerLevel(base , SMIF_RX_FIFO_TRIGEER_LEVEL);
    
    /* Configure the SMIF interrupt */
    Cy_SysInt_Init(&SMIF_SMIF_IRQ_cfg, &SMIF_Interrupt);
    
    /* Enable the fast and slow caches with pre-fetching */
    (void)Cy_SMIF_CacheEnable(base, CY_SMIF_CACHE_BOTH);
    (void)Cy_SMIF_CachePrefetchingEnable(base, CY_SMIF_CACHE_BOTH);
    
    /* Enables the SMIF interrupt */
    NVIC_EnableIRQ(SMIF_SMIF_IRQ_cfg.intrSrc);

    /* Configure the SMIF XIP registers */
    Cy_SMIF_Memslot_Init(base,(cy_stc_smif_block_config_t*) &smifBlockConfig, context);
    
    /* Starts the SMIF component */
    Cy_SMIF_Enable(base, context);
    
    /* Sets global pointers */
    SetSMIFPointers(base, context);
        
    /* Enable Quad Mode */
    WaitMemBusy(SMIFHardware, SMIFcontext);

    Cy_SMIF_Memslot_QuadEnable(base, (cy_stc_smif_mem_config_t*)smifMemConfigs[0], context);
    
    WaitMemBusy(SMIFHardware, SMIFcontext);
    
    return;
}


/*******************************************************************************
* Function Name: WriteMemory
********************************************************************************
*
* This function writes data to the external memory in the quad mode. 
* The function uses the Quad Page Program. 
*
* \param txBuffer
* Holds the address of the data to be sent.
*
* \param txSize
* The size of the data.
*
* \param Address 
* The address to write data to.
* 
* \return
* None
*******************************************************************************/
void WriteMemory(uint8_t txBuffer[],uint32_t txSize,uint32_t Address)
{
    cy_en_smif_status_t smif_status;
    
    /* Reverse address byte order */
    Address = __REV(Address);
    
    /* Wait until memory is available */
    WaitMemBusy(SMIFHardware, SMIFcontext);
    
    /* Send Write Enable to external memory */	
    smif_status = Cy_SMIF_Memslot_CmdWriteEnable(SMIFHardware, smifMemConfigs[0], SMIFcontext);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        handle_error();
    }
    
    WaitMemBusy(SMIFHardware, SMIFcontext);
    
	/* Quad Page Program command */       
    smif_status = Cy_SMIF_Memslot_CmdProgram(SMIFHardware, smifMemConfigs[0], (uint8_t*)&Address, txBuffer, txSize, &RxCmpltCallback, SMIFcontext);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        handle_error();
    }
}

/*******************************************************************************
* Function Name: ReadMemory
********************************************************************************
*
* This function reads data from the external memory in the quad mode. 
* The function sends the Quad I/O Read command. 
*
* \param rxBuffer
* Holds the address of where the data will be stored.
*
* \param rxSize
* The size of the data.
*
* \param Address 
* The address from where data will be read.
*
* \return
* None
*******************************************************************************/
void ReadMemory(uint8_t rxBuffer[], uint32_t rxSize, uint32_t Address)
{   
    cy_en_smif_status_t smif_status;

    /* Reverse address byte order */
    Address = __REV(Address);

    /* Wait until memory is available */    
    WaitMemBusy(SMIFHardware, SMIFcontext);
    
	/* The read command */    
    smif_status = Cy_SMIF_Memslot_CmdRead(SMIFHardware, smifMemConfigs[0], (uint8_t*)&Address, rxBuffer, rxSize, &RxCmpltCallback, SMIFcontext);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        handle_error();
    }
    
    /* Wait until data has been read */
    WaitMemBusy(SMIFHardware, SMIFcontext);
}

/*******************************************************************************
* Function Name: SwitchSMIFMemory
********************************************************************************
*
* This function switches the SMIF device from normal to memory mode. 
* Maps the external memory into the XIP memory region of the PSoC device.
* SMIF Device must be already initialized before calling this function.
* 
* \param
*  None
*
* \return
*  None
*******************************************************************************/
void SwitchSMIFMemory(void)
{
    /* SMIF must be already running */
    Cy_SMIF_SetMode(SMIFHardware, CY_SMIF_MEMORY);
    
    Cy_SMIF_CacheInvalidate(SMIFHardware, CY_SMIF_CACHE_BOTH);
}

/*******************************************************************************
* Function Name: SwitchSMIFNormal
********************************************************************************
*
* This function switches the SMIF device from memory to normal mode. 
* 
* \param
*  None
*
* \return
*  None
*******************************************************************************/
void SwitchSMIFNormal(void)
{
    /* SMIF must be already running */
    Cy_SMIF_SetMode(SMIFHardware, CY_SMIF_NORMAL);
    
    Cy_SMIF_CacheInvalidate(SMIFHardware, CY_SMIF_CACHE_BOTH);    
}

/*******************************************************************************
* Function Name: EraseSMIFChip
********************************************************************************
*
* This function erases the complete external memory. This is a blocking function.
* 
* \param
*  None
*
* \return
*  None
*******************************************************************************/
void EraseSMIFChip(void)
{
    cy_en_smif_status_t smif_status;
    
    smif_status = Cy_SMIF_Memslot_CmdWriteEnable(SMIFHardware, smifMemConfigs[0], SMIFcontext);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        handle_error();
    }
    
    smif_status = Cy_SMIF_Memslot_CmdChipErase(SMIFHardware, smifMemConfigs[0], SMIFcontext);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        handle_error();
    }
    
    WaitMemBusy(SMIFHardware, SMIFcontext);
}

/*******************************************************************************
* Function Name: EraseSMIFSector
********************************************************************************
*
* This function erases the sector where the passed address is located. 
*
* \param Address
* Address to be deleted (Including sector where address is located).
* 
* \return
*  None
*******************************************************************************/
void EraseSMIFSector(uint32_t Address)
{
    cy_en_smif_status_t smif_status;
    
    /* Reverse address byte order */
    Address = __REV(Address);
    
    smif_status = Cy_SMIF_Memslot_CmdWriteEnable(SMIFHardware, smifMemConfigs[0], SMIFcontext);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        handle_error();
    }
    
    smif_status = Cy_SMIF_Memslot_CmdSectorErase(SMIFHardware, (cy_stc_smif_mem_config_t*)smifMemConfigs[0], (uint8_t*)&Address, SMIFcontext);
    if(smif_status!=CY_SMIF_SUCCESS)
    {
        handle_error();
    }
    
    WaitMemBusy(SMIFHardware, SMIFcontext);
}

/*******************************************************************************
* Function Name: SetSMIFPointers
********************************************************************************
*
* Saves pointers of the SMIF configuration to common ram. Allowing the other
* core access to them.
*
* \param base
*  pointer to the SMIF hardware.
*
* \param context
* pointer to the SMIF context configuration.
*
* \return
*  None
*******************************************************************************/
void SetSMIFPointers(SMIF_Type *base, cy_stc_smif_context_t *context)
{
    if((base != NULL) && (context != NULL))
    {
        SMIFHardware = base;
        SMIFcontext = context;
    }
}

/* [] END OF FILE */

