/*******************************************************************************
* File Name: user_interface.c
*
* Version: 1.0
*
* Description:
*  This file contains user interface related source.
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "user_interface.h"
#include "ipss.h"


/*******************************************************************************
* Function Name: void InitUserInterface(void)
********************************************************************************
*
* Summary:
*   Initialization the user interface: LEDs, SW2, etc. 
*
*******************************************************************************/
void InitUserInterface(void)
{
    /* Initialize wakeup pin for Hibernate */
    Cy_SysPm_SetHibernateWakeupSource(CY_SYSPM_HIBERNATE_PIN1_LOW);
    
    /* Initialize LEDs */
    DisableAllLeds();
    
    /* Configure SW2 */
    Cy_SysInt_Init(&SW2_Int_cfg, GlobalSignal_InterruptHandler);
    NVIC_EnableIRQ(SW2_Int_cfg.intrSrc);  
    SW2_EnableInt();
}


/*******************************************************************************
* Function Name: UpdateLedState
********************************************************************************
*
* Summary:
*  This function updates LED status based on current BLE state.
*
*******************************************************************************/
void UpdateLedState(void)
{
#if(CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV)
    if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
    {
        /* In advertising state, turn off disconnect indication LED */
        Disconnect_LED_Write(LED_OFF);

        /* Blink advertising indication LED */
        if(cy_ble_advIndex == CY_BLE_PERIPHERAL_CONFIGURATION_1_INDEX) 
        {
            Simulation_LED_Write(LED_OFF);
            Advertising_LED_INV();
        }
        else
        {
            Advertising_LED_Write(LED_OFF);
            Simulation_LED_INV();
        }
    }
    else
    {   if(Cy_BLE_GetNumOfActiveConn() == 0u)
        {
            /* If in disconnected state, turn on disconnect indication LED and turn
            * off Advertising LED.
            */
            Disconnect_LED_Write(LED_ON);
        }
        else 
        {
            /* In connected state, turn off disconnect indication and advertising 
            * indication LEDs. 
            */
            Disconnect_LED_Write(LED_OFF);
        }
        Advertising_LED_Write(LED_OFF);
        Simulation_LED_Write(LED_OFF);
    }
#else
    if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
    {
        /* Blink advertising indication LED */
        LED5_INV();
    }
    else if(Cy_BLE_GetNumOfActiveConn() == 0u)
    {
        /* If in disconnected state, turn on disconnect indication LED */
        LED5_Write(LED_ON);
    }
    else 
    {
        /* In connected state, turn off disconnect indication LED */
        LED5_Write(LED_OFF);
    }
#endif /* #if(CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV) */   

}


/*******************************************************************************
* Function Name: GlobalSignal_InterruptHandler
********************************************************************************
*
* Summary:
*   Handles the mechanical button press (SW2) 
*
*******************************************************************************/
void GlobalSignal_InterruptHandler(void)
{
    /* Process of SW2 interrupt */
    if(Cy_GPIO_GetInterruptStatusMasked(SW2_0_PORT, SW2_0_NUM) == 1u)                                                   
    { 
        if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
        {
            /* Set flag to indicate about switch advertisement operation */
            IpsSetSwitchAdvModeFlag(true);
            
            /* Stop Advertisement */
            Cy_BLE_GAPP_StopAdvertisement();
        }    
        
        /* Clear the triggered pin interrupt */   
        SW2_ClearInt();
    } 
}


/* [] END OF FILE */
