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
#include "ancsc.h"

static volatile uint32_t buttonSW2Timer = 0u;
volatile uint8_t         button = 0u;


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
    
    /* Initialize SW2 interrupt */
    Cy_SysInt_Init(&SW2_Int_cfg, SW2_Int);
    NVIC_EnableIRQ(SW2_Int_cfg.intrSrc);   
    SW2_EnableInt();
        
    /* Configure the SysTick timer to generate interrupt every 1 ms
    * and start its operation.
    */
    Cy_SysTick_Init(CY_SYSTICK_CLOCK_SOURCE_CLK_LF, 32u); 
    Cy_SysTick_SetCallback(0u, Timer_Interrupt);
}


/*******************************************************************************
* Function Name: UpdateLedState
********************************************************************************
*
* Summary:
*  Handles indications on LEDs.
*
*******************************************************************************/
void UpdateLedState(void)
{
#if(CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV)    
    if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
    {
        /* In advertising state, turn off disconnect indication LED */
        Disconnect_LED_Write(LED_OFF);
        Ringing_LED_Write(LED_OFF);

        /* Blink advertising indication LED */
        Advertising_LED_INV();
    }
    else if(Cy_BLE_GetNumOfActiveConn() == 0u)
    {
        /* If in disconnected state, turn on disconnect indication LED and turn
        * off Advertising LED.
        */
        Disconnect_LED_Write(LED_ON);
        Advertising_LED_Write(LED_OFF);
        Ringing_LED_Write(LED_OFF);
    }
    else if(Cy_BLE_GetConnectionState(appConnHandle) >= CY_BLE_CONN_STATE_CONNECTED)
    {
        /* In connected state, turn off disconnect indication and advertising 
        * indication LEDs. 
        */
        Disconnect_LED_Write(LED_OFF);
        Advertising_LED_Write(LED_OFF);
        if((ancsFlag & CY_BLE_ANCS_FLG_ACT) != 0u)
        {
            Ringing_LED_INV();        /* There is pending user action during incoming call ringing */
        }
        else
        {
            Ringing_LED_Write(LED_OFF);
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
* Function Name: Timer_Interrupt
********************************************************************************
*
* Summary:
*  Handles the Interrupt Service Routine for the sys tick timer.
*
*******************************************************************************/
void Timer_Interrupt(void)
{
    /* Increment timer for recognition 1 press / 2 presses per second */
    if(buttonSW2Timer != TIMER_STOP)
    {   
        buttonSW2Timer ++;
    
        if(buttonSW2Timer > SW2_POLLING_TIME)
        {
            /* Set flag for button polling */
            button |= SW2_READY;
            buttonSW2Timer = TIMER_STOP;
        }        
    }
}


/*******************************************************************************
* Function Name: SW2_Int
********************************************************************************
*
* Summary:
*   Handles the SW2 button press.
*
*******************************************************************************/
void SW2_Int(void)
{
    if (Cy_GPIO_GetInterruptStatusMasked(SW2_0_PORT, SW2_0_NUM) == 1u)
    { 
        if(((ancsFlag & CY_BLE_ANCS_FLG_ACT) != 0u) && ((button & SW2_READY) == 0u))
        {
            if(buttonSW2Timer == TIMER_STOP)
            {   
                button = SW2_ONE_PRESSING;
                buttonSW2Timer = TIMER_START;
            } 
            if (buttonSW2Timer > SW2_DEBOUNCING_TIME)
            {   
                button = SW2_TWO_PRESSING;
            }
        }
        SW2_ClearInt();
    }   
}


/* [] END OF FILE */
