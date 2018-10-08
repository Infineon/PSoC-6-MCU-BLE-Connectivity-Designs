/*******************************************************************************
* File Name: main_cm0p.c
*
* Version: 1.10
*
* Description:
*  This is the source code for CM0+ core for the dual core BLE project.
*
* Related Document: CE222046_Throughput_Measurement.pdf
*
* Hardware Dependency: See CE222046_Throughput_Measurement.pdf
*
*******************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress’s integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress 
* reserves the right to make changes to the Software without notice. Cypress 
* does not assume any liability arising out of the application or use of the 
* Software or any product or circuit described in the Software. Cypress does 
* not authorize its products for use in any products where a malfunction or 
* failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such 
* system or application assumes all risk of such use and in doing so agrees to 
* indemnify Cypress against all liability.
*******************************************************************************/

#include "project.h"

/*******************************************************************************
* Function Name: main()
********************************************************************************
* Summary:
*  Main function for the CM0+ core.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Theory:
*  The function starts BLE Controller and processes all BLE events.
*
*******************************************************************************/
extern int HostMain(void);
int main(void)
{
    cy_en_ble_api_result_t apiResult;
    __enable_irq(); /* Enable global interrupts. */
    
    #if(CY_BLE_CONFIG_HOST_CORE == CY_BLE_CORE_CORTEX_M0P)   
    
    /* Run Host main */
    HostMain();

    #else
        /* Start BLE Controller for dual core mode */
	    apiResult = Cy_BLE_Start(NULL);
        
        if(apiResult == CY_BLE_SUCCESS)
        {
            /* Enable CM4 only if BLE Controller started successfully.
             * Enable CM4. CY_CORTEX_M4_APPL_ADDR must be updated if CM4 memory 
             * layout is changed. */
            Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR);
        }
        
        for(;;)
        {
            /* Process BLE events continuously for controller in dual core mode */
		    Cy_BLE_ProcessEvents();
        }
    
    #endif /* (CY_BLE_CONFIG_HOST_CORE == CY_BLE_CORE_CORTEX_M0P) */
    
}
/* [] END OF FILE */
