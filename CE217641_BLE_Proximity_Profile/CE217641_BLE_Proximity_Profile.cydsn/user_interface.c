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
#include "lls.h"

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
    Cy_SysInt_Init(&SW2_Int_cfg, SW2_Interrupt);
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
void UpdateLedState(uint8_t llsAlertTOCounter)
{

#if(CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV)
    if((Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING) &&
       (LlsGetAlertLevel() == CY_BLE_NO_ALERT))
    {
        /* In advertising state, turn off disconnect indication LED */
        Disconnect_LED_Write(LED_OFF);
        Alert_LED_Write(LED_OFF);
        
        /* Blink advertising indication LED */
        Advertising_LED_INV();     
    }
    else if(Cy_BLE_GetNumOfActiveConn() == 0u)
    {
        /* If in disconnected state, turn on disconnect indication LED and turn
        * off Advertising LED.
        */
        
        Advertising_LED_Write(LED_OFF);
              
        /* If "Mild Alert" is set by the Client then blink alert LED */
        if((LlsGetAlertLevel() == CY_BLE_MILD_ALERT) && (llsAlertTOCounter != ALERT_TIMEOUT))
        {
            Disconnect_LED_Write(LED_OFF);
            Alert_LED_INV();
        }
        /* For "High Alert" turn alert LED on */
        else if((LlsGetAlertLevel() == CY_BLE_HIGH_ALERT) && (llsAlertTOCounter != ALERT_TIMEOUT))
        {
            Disconnect_LED_Write(LED_OFF);
            Alert_LED_Write(LED_ON);
        }
        /* In case of "No Alert" turn alert LED off */
        else
        {
            Disconnect_LED_Write(LED_ON);
            Alert_LED_Write(LED_OFF);
        }
    }
    else 
    {
        /* In connected state, turn off disconnect indication and advertising 
        * indication LEDs. 
        */
        Disconnect_LED_Write(LED_OFF);
        Advertising_LED_Write(LED_OFF);
        Alert_LED_Write(LED_OFF);
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
/* [] END OF FILE */
