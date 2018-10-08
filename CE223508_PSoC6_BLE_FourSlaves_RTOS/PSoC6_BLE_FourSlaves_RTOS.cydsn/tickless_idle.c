/******************************************************************************
* File Name: tickless_idle.c
*
* Version: 1.00
*
* Description: This file contains functions and RTOS hooks used for
*              Tickless idle mode
*
* Related Document: CE223508_PSoC6_BLE_FourSlaves_RTOS.pdf
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
*
******************************************************************************
* Copyright (2017), Cypress Semiconductor Corporation.
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
/*******************************************************************************
* This file contains functions and RTOS hooks used for Tickless idle mode
*******************************************************************************/

/* Header file includes */
#include "tickless_idle.h"
#include "FreeRTOS.h"
#include "task.h"

/* Macro used to detect if the expected idle time is of portMAX_DELAY */
#define PORT_MAX_DETECT         (uint32_t)(portMAX_DELAY & 0xFFFF0000u)

/* Recommended wait times for MCWDT functions. See the MCWDT PDL documentation 
   for details */
#define MCWDT_RESET_API_WAIT    (uint32_t)(62u)
#define MCWDT_COMMON_API_WAIT   (uint32_t)(93u)

/* Macros used to convert RTOS ticks to MCWDT counts and vice versa */
#define TicksToMcwdtCount(x)    (((x*CYDEV_CLK_LFCLK__HZ)/configTICK_RATE_HZ)-1)
#define McwdtCountToTicks(x)    (((x*configTICK_RATE_HZ)/CYDEV_CLK_LFCLK__HZ)+1)

/* Function for checking the tickless idle entry readiness of the system */
tickless_readiness_function_t   IsReadyForTicklessIdle;

/* Flag used to detect a MCWDT match event */
bool static mcwdtMatchEvent = false;

/*******************************************************************************
* Function Name: void vApplicationSleep(TickType_t xExpectedIdleTime)
********************************************************************************
* Summary:
*  This function is called by portSUPPRESS_TICKS_AND_SLEEP() when FreeRTOS
*  is ready to enter Tickless idle mode.
*
* Parameters:
*  xExpectedIdleTime :	Expected time in ticks until a task gets unblocked
*
* Return:
*  void
*
*******************************************************************************/
void vApplicationSleep(TickType_t xExpectedIdleTime)
{
    /* Local variables used to store the system status and MCWDT values */
    bool     deepSleepEntry = false;
    bool     portMaxDelayDetected = false;
    uint16_t counterMatchValue = UINT16_MAX;
    uint16_t counterCurrentValue;

    /* Stop the SysTick Timer that is generating the RTOS tick interrupt */
    Cy_SysTick_Disable();
    
    /* Check if the user tasks are ready to enter Tickless Idle  */
    if (IsReadyForTicklessIdle())
    {       
        /* PortMaxDelay detected */
        if(xExpectedIdleTime > PORT_MAX_DETECT)
        {
            portMaxDelayDetected = true;
        }
        /* Check if the expected wait time is below the maximum count value
           possible by MCWDT Counter0 */
        else if(xExpectedIdleTime < (TickType_t)McwdtCountToTicks(UINT16_MAX))
        {
            /* Calculate MCWDT Counter0 value corresponding to the given
               expected idle time */
            counterMatchValue = (uint16_t)TicksToMcwdtCount(xExpectedIdleTime); 
        }
        
        /* Indicate Deep Sleep entry readiness */
        deepSleepEntry = true;
    }

    /* Ensure it is still OK to enter the sleep mode. */
    if(eTaskConfirmSleepModeStatus() == eAbortSleep )
    {
        /* A task has been moved out of the blocked state since this function 
        has been called , or a context switch is being held pending. Do not 
        enter Tickless Idle. Restart the RTOS tick counter instead */ 
        Cy_SysTick_Enable();
    }
    else
    {     
        /* OK to enter Deep Sleep mode */
        if (deepSleepEntry == true)
        {
            /* If portMaxDelay is detected, do not use any timed-wake up. 
               In this case, an external interrupt such as GPIO interrupt 
               should be used to wake up from Tickless Idle */
            if(!portMaxDelayDetected)
            {
                /* Set up the MCWDT Counter0 for the required wakeup time */
                mcwdtMatchEvent = false;
                Cy_MCWDT_ResetCounters(MCWDT_HW, CY_MCWDT_CTR0,
                                       MCWDT_RESET_API_WAIT);
                Cy_MCWDT_SetMatch(MCWDT_HW, CY_MCWDT_COUNTER0, 
                                  counterMatchValue, MCWDT_COMMON_API_WAIT);
                Cy_MCWDT_Enable(MCWDT_HW, CY_MCWDT_CTR0, MCWDT_COMMON_API_WAIT);
            }
            
            /* Enter Deep Sleep mode */
            Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
            
            if(!portMaxDelayDetected)
            {
                /* MCWDT match event has occurred */
                if (mcwdtMatchEvent)
                {
                    counterCurrentValue = counterMatchValue;
                }
                /* Other source of wakeup */
                else
                {
                    /* Disable MCWDT and retrieve count value */
                    Cy_MCWDT_Disable(MCWDT_HW, CY_MCWDT_CTR0,
                                     MCWDT_COMMON_API_WAIT);
                    counterCurrentValue = (Cy_MCWDT_GetCount
                                           (MCWDT_HW,CY_MCWDT_COUNTER0));
                }
                
                 /* Correct the RTOS Kernel's tick count to account for the time
                    spent in the Deep Sleep mode */
                vTaskStepTick((TickType_t)McwdtCountToTicks
                                          (counterCurrentValue));
            }
            
            /* Restart the RTOS tick counter */
            Cy_SysTick_Enable();
        }
        /* Tasks are not ready for Deep Sleep */
        else
        {
            /* Restart the RTOS tick counter */
            Cy_SysTick_Enable();
            /* Enter Sleep mode and wait for next tick to re-check if the 
               tasks are OK to enter Deep Sleep */
            Cy_SysPm_Sleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
        }      
    }
}

/*******************************************************************************
* Function Name: void IsrMcwdt(void)
********************************************************************************
*
* Summary:
*  Interrupt service routine for the MCWDT interrupt
*    
*  Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void IsrMcwdt(void)
{
    /* Clear the MCWDT Counter0 interrupt */
    NVIC_ClearPendingIRQ(MCWDT_isr_cfg.intrSrc); 
    Cy_MCWDT_ClearInterrupt(MCWDT_HW,CY_MCWDT_CTR0);
    /* Disable MCWDT Counter0. It will be re-enabled by vApplicationSleep
       when required */
    Cy_MCWDT_Disable(MCWDT_HW, CY_MCWDT_CTR0, MCWDT_COMMON_API_WAIT);
    
    /* Set the flag to indicate a match event */
    mcwdtMatchEvent = true;
}

/*******************************************************************************
* Function Name: void TicklessIdleInit(void)
********************************************************************************
*
* Summary:
*  Initializes the components used by Tickless Idle
*    
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void TicklessIdleInit(tickless_readiness_function_t TicklessIdleReadiness)
{
    IsReadyForTicklessIdle = TicklessIdleReadiness;
    
    /* Initialize the MCDWT */
    Cy_MCWDT_Init(MCWDT_HW, &MCWDT_config);
    /* Enable the interrupts for MCDWT Counter 0 */
    Cy_MCWDT_SetInterruptMask(MCWDT_HW,CY_MCWDT_CTR0);
    
    /* Initialize and enable MCWDT interrupt */
    Cy_SysInt_Init(&MCWDT_isr_cfg,IsrMcwdt);
    NVIC_ClearPendingIRQ(MCWDT_isr_cfg.intrSrc);
    NVIC_EnableIRQ((IRQn_Type)MCWDT_isr_cfg.intrSrc);
}

/* [] END OF FILE */
