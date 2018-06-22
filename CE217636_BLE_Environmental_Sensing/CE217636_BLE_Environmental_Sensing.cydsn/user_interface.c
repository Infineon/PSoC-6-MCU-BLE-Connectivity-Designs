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
void UpdateLedState(void)
{
#if(CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV)
    if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
    {
        /* In advertising state, turn off disconnect indication LED */
        Disconnect_LED_Write(LED_OFF);

        /* Blink advertising indication LED */
        Advertising_LED_INV();
        
        Connected_LED_Write(LED_OFF);
    }
    else if(Cy_BLE_GetNumOfActiveConn() == 0u)
    {
        /* If in disconnected state, turn on disconnect indication LED and turn
        * off Advertising LED.
        */
        Disconnect_LED_Write(LED_ON);
        Advertising_LED_Write(LED_OFF);
        Connected_LED_Write(LED_OFF);
    }
    else 
    {
        /* In connected state, turn off disconnect indication and advertising 
        * indication LEDs. 
        */
        Disconnect_LED_Write(LED_OFF);
        Advertising_LED_Write(LED_OFF);
        Connected_LED_Write(LED_ON);
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
* Function Name: HandleButtonPress
********************************************************************************
*
* Summary:
*  Changes the value of ES Configuration Descriptor in the GATT database.
*
* Parameters:  
*   sensorPtr: A pointer to the sensor characteristic structure.
*
*******************************************************************************/
void HandleButtonPress(cy_stc_ble_ess_characteristic_data_t *sensorPtr)
{
    /* localConnHandle is used for reading ESSS Descriptor (not CCCD descriptor). 
     * So, this parameter can be skipped ( = 0) in Cy_BLE_ESSS_SetCharacteristicDescriptor() 
     */
    cy_stc_ble_conn_handle_t localConnHandle = { .attId = 0u };
    
    /* Change value of ES Configuration descriptor ... */
    if(sensorPtr->esConfig == CY_BLE_ESS_CONF_BOOLEAN_AND)
    {
        sensorPtr->esConfig = CY_BLE_ESS_CONF_BOOLEAN_OR;
    }
    else
    {
        sensorPtr->esConfig= CY_BLE_ESS_CONF_BOOLEAN_AND;
    }

    /* ... and set it to GATT database */
    (void) Cy_BLE_ESSS_SetCharacteristicDescriptor(localConnHandle, sensorPtr->EssChrIndex, sensorPtr->chrInstance,
                                                    CY_BLE_ESS_ES_CONFIG_DESCR, SIZE_1_BYTE, &sensorPtr->esConfig);
}

/* [] END OF FILE */
