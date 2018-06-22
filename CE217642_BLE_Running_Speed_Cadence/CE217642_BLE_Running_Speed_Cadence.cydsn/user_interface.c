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
#include "common.h"
#include "rscs.h"

/* Global Variables */
static bool     buttonSW2Status = BUTTON_IS_NOT_PRESSED;


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
        Advertising_LED_INV();
        
        Running_LED_Write(LED_OFF);
    }
    else if(Cy_BLE_GetNumOfActiveConn() == 0u)
    {
        /* If in disconnected state, turn on disconnect indication LED and turn
        * off Advertising LED.
        */
        Disconnect_LED_Write(LED_ON);
        Advertising_LED_Write(LED_OFF);
        Running_LED_Write(LED_OFF);
    }
    else 
    {
        /* In connected state, turn off disconnect indication and advertising 
        * indication LEDs. 
        */
        Disconnect_LED_Write(LED_OFF);
        Advertising_LED_Write(LED_OFF);
        
        if(RscGetProfileMode() == WALKING)
        {   /* Walking */
            Running_LED_Write(LED_OFF);
        }
        else
        {   /* Running */
            Running_LED_Write(LED_ON);
        }
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
        ButtonSW2SetStatus(BUTTON_IS_PRESSED);
        
        /* Clear the triggered pin interrupt */   
        SW2_ClearInt();
    } 
}


/*******************************************************************************
* Function Name: ButtonSW2GetStatus
********************************************************************************
*
* Summary:
*   This function returns SW2 button status:
*    - BUTTON_IS_PRESSED (true)       - SW2 button was pressed
*    - BUTTON_IS_NOT_PRESSED (false)  - SW2 button was not pressed            
*
* Returns:
*  bool - SW2 button status.
*
*******************************************************************************/
bool ButtonSW2GetStatus(void)
{
    return buttonSW2Status;
}


/*******************************************************************************
* Function Name: ButtonSW2SetStatus
********************************************************************************
*
* Summary:
*   This function sets SW2 button status:
*    - BUTTON_IS_PRESSED (true)       - SW2 button was pressed
*    - BUTTON_IS_NOT_PRESSED (false)  - SW2 button was not pressed            
*
* Parameter :
*   Status - SW2 button status.
*
*******************************************************************************/
void ButtonSW2SetStatus(bool Status)
{
    buttonSW2Status = Status;
}    

/* [] END OF FILE */
