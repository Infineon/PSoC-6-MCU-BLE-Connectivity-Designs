/*******************************************************************************
* File Name: wpru.c
*
* Version: 1.0
*
* Description:
*  This file contains WPT callback handler function and code for Power 
*  Receiver Unit.
*
* Hardware Dependency:
*  CY8CKIT-062 BLE
* 
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"
#include "wpru.h"


/*******************************************************************************
* Global variables
*******************************************************************************/
static uint16_t wPowerSimulation;
cy_stc_ble_pru_dynamic_par_t pruDynamicParameter;
cy_stc_ble_pru_control_t pruControl;

static const uint16_t maxSourceImpedance[] = {50u, 60u, 70u, 80u, 90u, 100u, 110, 120u, 130u, 140u, 150u,
                                              175u, 200u, 225u, 250u, 275u, 300u, 350u, 375u};

static uint8_t  alertIndicationConfirmation = PRU_ALERT_INDICATION_CONFIRMED;
static bool chargingEnable = false;


/*******************************************************************************
* Function Name: WptsInit()
********************************************************************************
*
* Summary:
*   Initializes the WPT service.
*
*******************************************************************************/
void WptsInit(void)
{
    cy_en_ble_api_result_t apiResult;

    wPowerSimulation = PRU_ALERT_SIMULATION_DISABLE;

    /* Register service-specific callback function */
    Cy_BLE_WPTS_RegisterAttrCallback(WptsCallBack);

    /* Read PRU Dynamic Parameter characteristic default value */
    apiResult = Cy_BLE_WPTSS_GetCharacteristicValue(CY_BLE_WPTS_PRU_DYNAMIC_PAR, sizeof(pruDynamicParameter), 
        (uint8_t *)&pruDynamicParameter);
    if((apiResult != CY_BLE_SUCCESS))
    {
        DBG_PRINTF("Cy_BLE_WPTSS_GetCharacteristicValue API Error: %x \r\n", apiResult);
    }
}


/*******************************************************************************
* Function Name: WptsGetPower
********************************************************************************
*
* Summary:
*   This function converts value in decimal to power in milliwatts.
*
* Parameters:
*  Value in decimal.
*
* Return:
*  Power in milliWatts.
*
*******************************************************************************/
uint32_t WptsGetPower(uint8_t value)
{
    uint8_t range = 0u;
    uint32_t step = PTU_STATIC_PAR_POWER_STEP;
    uint32_t power = 0u;
    while(range < value)
    {
        if((value - range) > PTU_STATIC_PAR_POWER_RANGE)
        {
            power += step * PTU_STATIC_PAR_POWER_RANGE; 
        }
        else
        {
            power += step * (value - range); 
        }
        range += PTU_STATIC_PAR_POWER_RANGE;
        if(range < PTU_STATIC_PAR_POWER_HI_RANGE)
        {
            step += PTU_STATIC_PAR_POWER_STEP;
        }
        else
        {
            step = PTU_STATIC_PAR_POWER_HI_STEP;
        }
    }
    return(power);
}


/*******************************************************************************
* Function Name: WptsCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE service.
*
* Parameters:
*  event - the event code
*  *eventParam - the event parameters
*
* Return:
*  None.
*
*******************************************************************************/
void WptsCallBack(uint32_t event, void* eventParam)
{
    uint8_t locCharIndex;
    
    locCharIndex = ((cy_stc_ble_wpts_char_value_t *)eventParam)->charIndex;
    
    switch(event)
    {
        /* WPTS Server - Notifications for Wireless Power Transfer Service Characteristic
            were enabled. The parameter of this event is a structure of
            cy_stc_ble_wpts_char_value_t type.
        */
        case CY_BLE_EVT_WPTSS_NOTIFICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_WPTS_NOTIFICATION_ENABLED: char: %x\r\n", locCharIndex);
            if(locCharIndex == CY_BLE_WPTS_PRU_ALERT)
            {
                wPowerSimulation |= PRU_ALERT_NOTIFICATION_ENABLE;
            }
            break;
            
        /* WPTS Server - Notifications for Wireless Power Transfer Service Characteristic
            were disabled. The parameter of this event is a structure 
            of cy_stc_ble_wpts_char_value_t type.
        */
        case CY_BLE_EVT_WPTSS_NOTIFICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_WPTS_NOTIFICATION_DISABLED: char: %x\r\n", locCharIndex);
            if(locCharIndex == CY_BLE_WPTS_PRU_ALERT)
            {
                wPowerSimulation &= ~PRU_ALERT_NOTIFICATION_ENABLE;
            }
            break;
            
        /* WPTS Server - Indication for Wireless Power Transfer Service Characteristic
            was enabled. The parameter of this event is a structure 
            of cy_stc_ble_wpts_char_value_t type.
        */
        case CY_BLE_EVT_WPTSS_INDICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_WPTS_INDICATION_ENABLED: char: %x\r\n", locCharIndex);
            if(locCharIndex == CY_BLE_WPTS_PRU_ALERT)
            {
                wPowerSimulation |= PRU_ALERT_INDICATION_ENABLE;
            }
            break;
            
        /* WPTS Server - Indication for Wireless Power Transfer Service Characteristic
            was disabled. The parameter of this event is a structure 
            of cy_stc_ble_wpts_char_value_t type.
        */
        case CY_BLE_EVT_WPTSS_INDICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_WPTS_INDICATION_DISABLED: char: %x\r\n", locCharIndex);
            if(locCharIndex == CY_BLE_WPTS_PRU_ALERT)
            {
                wPowerSimulation &= ~PRU_ALERT_INDICATION_ENABLE;
            }
            break;
            
        /* WPTS Server - Wireless Power Transfer Service Characteristic
            Indication was confirmed. The parameter of this event
            is a structure of cy_stc_ble_wpts_char_value_t type.
        */
        case CY_BLE_EVT_WPTSS_INDICATION_CONFIRMED:
            DBG_PRINTF("CY_BLE_EVT_WPTS_INDICATION_CONFIRMED\r\n");
            alertIndicationConfirmation = PRU_ALERT_INDICATION_CONFIRMED;
            break;
            
        /* WPTS Server - Write Request for Wireless Power Transfer Service Characteristic 
            was received. The parameter of this event is a structure
            of cy_stc_ble_wpts_char_value_t type.
        */    
        case CY_BLE_EVT_WPTSS_WRITE_CHAR:
            if(locCharIndex ==CY_BLE_WPTS_PRU_CONTROL)
            {
                pruControl = *(cy_stc_ble_pru_control_t *)(((cy_stc_ble_wpts_char_value_t *)eventParam)->value->val);
                DBG_PRINTF("PRU_CONTROL: enables: %x, Output - ", pruControl.enables);
                if((pruControl.enables & PRU_CONTROL_ENABLES_ENABLE_PRU_OUTPUT) != 0u)
                {
                    DBG_PRINTF("ENABLE ,");
                }
                else
                {
                    DBG_PRINTF("DISABLE,");
                }
                DBG_PRINTF("Charge indicator -");
                if((pruControl.enables & PRU_CONTROL_ENABLES_ENABLE_CHARGE_INDICATOR) != 0u)
                {
                    DBG_PRINTF("ENABLE ,");
                    pruDynamicParameter.alert &= (uint8_t)~(PRU_ALERT_CHARGE_COMPLETE | PRU_ALERT_OVER_MASK);
                    (void)Cy_BLE_WPTSS_SetCharacteristicValue(CY_BLE_WPTS_PRU_DYNAMIC_PAR, sizeof(pruDynamicParameter), 
                        (uint8_t *)&pruDynamicParameter);
                    chargingEnable = true;
                }
                else
                {
                    DBG_PRINTF("DISABLE,");
                    chargingEnable = false;
                }
                DBG_PRINTF("Power:");
                switch(pruControl.enables & PRU_CONTROL_ENABLES_ADJUST_POWER_MASK)
                {
                    case PRU_CONTROL_ENABLES_ADJUST_POWER_MAX: 
                        DBG_PRINTF("Max");
                        break;
                    case PRU_CONTROL_ENABLES_ADJUST_POWER_66:
                        DBG_PRINTF("66 percent");
                        break;
                    case PRU_CONTROL_ENABLES_ADJUST_POWER_33:
                        DBG_PRINTF("33 percent");
                        break;
                    case PRU_CONTROL_ENABLES_ADJUST_POWER_2_5:
                        DBG_PRINTF("2.5W");
                        break;
                }                    
                DBG_PRINTF(" permission: %x, ", pruControl.permission);
                if((pruControl.permission & PRU_CONTROL_PERMISSION_DENIED_FLAG) != 0u)
                {
                    DBG_PRINTF("Denied ,");
                }
                else if((pruControl.permission & PRU_CONTROL_PERMISSION_PERMITTED_WITH_WARNING) != 0u)
                {
                    DBG_PRINTF("Permitted with warning,");
                }
                else
                {
                    DBG_PRINTF("Permitted,");
                }
                DBG_PRINTF("timeSet %d ms\r\n", pruControl.timeSet * PRU_CONTROL_TIME_SET_STEP_MS);
            }
            else if(locCharIndex == CY_BLE_WPTS_PTU_STATIC_PAR)
            {
                cy_stc_ble_ptu_static_par_t ptuStaticPar;
                ptuStaticPar = *(cy_stc_ble_ptu_static_par_t *)(((cy_stc_ble_wpts_char_value_t *)eventParam)->value->val);
                DBG_PRINTF("PTU_STATIC_PARAMETER: flags: %x, power: %.1f watts, ", 
                    ptuStaticPar.flags, WptsGetPower(ptuStaticPar.ptuPower)/1000.0);
                if(((ptuStaticPar.flags & PTU_STATIC_PAR_FLAGS_MAX_IMPEDANCE_EN) != 0u) &&
                   (ptuStaticPar.ptuMaxSourceImpedance <= PTU_STATIC_PAR_MAX_SOURCE_IMPEDANCE_MAX))
                {
                    DBG_PRINTF("maxSourceImpedance: %d ohms, ", maxSourceImpedance[ptuStaticPar.ptuMaxSourceImpedance]);
                }
                if((ptuStaticPar.flags & PTU_STATIC_PAR_FLAGS_MAX_RESISTANCE_EN) != 0u)
                {
                    DBG_PRINTF("maxLoadResistance: %d ohms, ", Wpts_GetLoadResistance(ptuStaticPar.ptuMaxLoadResistance));
                }
                DBG_PRINTF("supported devices number: %d , ", ptuStaticPar.ptuDevNumber + PTU_STATIC_PAR_NUMBER_OF_DEVICES_OFFSET);
                DBG_PRINTF("class: Class%d , ", ptuStaticPar.ptuClass + PTU_STATIC_PAR_CLASS_OFFSET);
                DBG_PRINTF("hardware rev: %d, firmware rev: %d, protocol rev: %d\r\n", 
                    ptuStaticPar.hardwareRev, ptuStaticPar.firmwareRev, ptuStaticPar.protocolRev);
            }
            else
            {
                DBG_PRINTF("CY_BLE_EVT_WPTS_CHAR_WRITE: char: %x, value: ", locCharIndex);
                ShowValue(((cy_stc_ble_wpts_char_value_t *)eventParam)->value);
            }
            break;
            
        /* WPTS Client - Wireless Power Transfer Service Characteristic
            Indication was received. The parameter of this event
            is a structure of cy_stc_ble_wpts_char_value_t type.
        */
        case CY_BLE_EVT_WPTSC_INDICATION:
            break;
            
        /* WPTS Client - Read Response for Read Request of Wireless Power Transfer 
            Service Characteristic value. The parameter of this event
            is a structure of cy_stc_ble_wpts_char_value_t type.
        */
        case CY_BLE_EVT_WPTSC_READ_CHAR_RESPONSE:
            break;
            
        /* WPTS Client - Read Response for Read Request of Wireless Power Transfer
            Service Characteristic Descriptor Read request. The 
            parameter of this event is a structure of
            cy_stc_ble_wpts_descr_value_t type.
        */
        case CY_BLE_EVT_WPTSC_READ_DESCR_RESPONSE:
            break;
            
        /* WPTS Client - Write Response for Write Request of Wireless Power Transfer
            Service Characteristic Configuration Descriptor value.
            The parameter of this event is a structure of 
            cy_stc_ble_wpts_descr_value_t type.
        */
        case CY_BLE_EVT_WPTSC_WRITE_DESCR_RESPONSE:
            break;
        default:
            DBG_PRINTF("WPTS OTHER event: %lx \r\n", event);
            break;
    }
}


/*******************************************************************************
* Function Name: WptsSetAlert()
********************************************************************************
*
* Summary:
*   This function sends Notification and/or Indication of Alert characteristic 
*   if it is enabled by client and stores Alert value in PRU Dynamic Parameter
*   characteristic. 
*
* Parameters:
*   uint8_t alert value. Combination of the following defines:
*       PRU_ALERT_OVER_VOLTAGE
*       PRU_ALERT_OVER_CURRENT
*       PRU_ALERT_OVER_TEMP
*       PRU_ALERT_SELF_PROTECTION
*       PRU_ALERT_CHARGE_COMPLETE
*       PRU_ALERT_WIRED_CHARGER_DETECT
*
* Return:
*   None
*
*******************************************************************************/
void WptsSetAlert(uint8_t alert)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    pruDynamicParameter.alert |= alert;
        
    if(wPowerSimulation & PRU_ALERT_NOTIFICATION_ENABLE)
    {
        apiResult = Cy_BLE_WPTSS_SendNotification(appConnHandle, CY_BLE_WPTS_PRU_ALERT, 
            sizeof(pruDynamicParameter.alert), &pruDynamicParameter.alert);
        DBG_PRINTF("WptssSendNotification Alert: %x  ", alert);
        if((apiResult != CY_BLE_SUCCESS))
        {
            DBG_PRINTF("WptssSendNotification API Error: %x \r\n", apiResult);
        }
    }
    if((wPowerSimulation & PRU_ALERT_INDICATION_ENABLE ) && 
        (alertIndicationConfirmation == PRU_ALERT_INDICATION_CONFIRMED))
    {
        apiResult = Cy_BLE_WPTSS_SendIndication(appConnHandle, CY_BLE_WPTS_PRU_ALERT, 
            sizeof(pruDynamicParameter.alert), &pruDynamicParameter.alert);
        DBG_PRINTF("WptssSendIndication Alert: %x ", alert);
        if((apiResult != CY_BLE_SUCCESS))
        {
            DBG_PRINTF("WptssSendIndication API Error: %x \r\n", apiResult);
        }
        else
        {
            alertIndicationConfirmation = PRU_ALERT_INDICATION_NOT_CONFIRMED;
        }
    }
}


/*******************************************************************************
* Function Name: WptsSimulateWirelessTransfer()
********************************************************************************
*
* Summary:
*   A custom function for service simulation. It generates Alert notification 
*   with charge complete status after timeout period. 
*
* Parameters:
*   uint8_t step: in mV to change Vrect value
*
* Return:
*   None
*
*******************************************************************************/
void WptsSimulateWirelessTransfer(uint8_t step)
{
    cy_en_ble_api_result_t apiResult;
    
    /* Simulate Vrect changing */
    if(PRU_CHARGING)
    {
        pruDynamicParameter.vRect += step;
    }
    else
    {
        if(pruDynamicParameter.vRect > step)
        {
            pruDynamicParameter.vRect -= step;
        }
    }
    
    if(pruDynamicParameter.vRect > pruDynamicParameter.vRectHighDyn)
    {
        if (Cy_BLE_GATT_GetBusyStatus(appConnHandle.attId) == CY_BLE_STACK_STATE_FREE)
        {   
            WptsSetAlert(PRU_ALERT_CHARGE_COMPLETE);
        }
    }
    if(pruDynamicParameter.vRect < pruDynamicParameter.vRectMinDyn)
    {
        pruDynamicParameter.alert = 0u;
    }
    /* Write PRU Dynamic Parameter characteristic value */
    apiResult = Cy_BLE_WPTSS_SetCharacteristicValue(CY_BLE_WPTS_PRU_DYNAMIC_PAR, sizeof(pruDynamicParameter), 
                                                    (uint8_t *)&pruDynamicParameter);
    if((apiResult != CY_BLE_SUCCESS))
    {
        DBG_PRINTF("Cy_BLE_WPTSS_SetCharacteristicValue API Error: %x \r\n", apiResult);
    }
    else
    {
        DBG_PRINTF("Simul Vrect: %d mV \r\n", pruDynamicParameter.vRect);
    }
    
}


/*******************************************************************************
* Function Name: WptsIsChargingEnable
********************************************************************************
*
* Summary:
*   This function return true if charging is enabled. 
*
*******************************************************************************/
bool WptsIsChargingEnable(void)
{
    return chargingEnable;
}

/* [] END OF FILE */


