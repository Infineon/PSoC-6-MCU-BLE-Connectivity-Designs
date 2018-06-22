/******************************************************************************
* File Name: main_cm0p.c
*
* Version: 1.00
*
* Description: This file contains the Cortex-M0+ BLE controller initialization 
*  code and enable the CM4 core.
*
* Related Document: CE222004_PSoC6_BLE_MultiSlave.pdf
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
*
******************************************************************************
* Copyright (2017-2018), Cypress Semiconductor Corporation.
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
/******************************************************************************
* Cortex-M0+ handles the controller portion of BLE. For the host functions, 
* see main_cm4.c
*******************************************************************************/
/* Header file includes */
#include <project.h>

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  System entrance point. This function enables the Cortex-M4 and continuously 
*  processes BLE controller events
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_en_ble_api_result_t          apiResult;
    
    __enable_irq(); /* Enable global interrupts. */
    /* Unfreeze IO if device is waking up from hibernate */
    if(Cy_SysPm_GetIoFreezeStatus())
    {
        Cy_SysPm_IoUnfreeze();
    }
    
    /* Start the Controller portion of BLE. Host runs on the CM4 */
    apiResult = Cy_BLE_Start(NULL);
    
    if(apiResult == CY_BLE_SUCCESS)
    {
        /* Enable CM4 only if BLE Controller started successfully. 
        *  CY_CORTEX_M4_APPL_ADDR must be updated if CM4 memory layout 
        *  is changed. */
        Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR); 
    }
    else
    {
        /* Halt CPU */
        CY_ASSERT(0u != 0u);
    }
   
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    for(;;)
    {
        /* Place your application code here. */
        /* Put CM0p to deep sleep */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
        
        /* Cy_Ble_ProcessEvents() allows BLE stack to process pending events */
        /* The BLE Controller automatically wakes up host if required */
        Cy_BLE_ProcessEvents();        
    }
}

/* [] END OF FILE */
