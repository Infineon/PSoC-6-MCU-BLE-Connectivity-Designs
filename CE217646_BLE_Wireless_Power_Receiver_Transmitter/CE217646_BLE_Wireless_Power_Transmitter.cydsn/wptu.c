/*******************************************************************************
* File Name: wptu.c
*
* Version: 1.0
*
* Description:
*  This file contains WPTS callback handler function and code for Power 
*  Transmitter Unit.
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
#include <stdbool.h>

static cy_stc_ble_ptu_static_par_t ptuStaticPar;


/*******************************************************************************
* Function Name: WptsInit()
********************************************************************************
*
* Summary:
*   Initializes the WPTS service.
*
*******************************************************************************/
void WptsInit(void)
{
   
    /* Register service specific callback function */
    Cy_BLE_WPTS_RegisterAttrCallback(WptsCallBack);
    
    /* Init PTU static parameters */
    ptuStaticPar.flags = PTU_STATIC_PAR_FLAGS_MAX_IMPEDANCE_EN | PTU_STATIC_PAR_FLAGS_MAX_RESISTANCE_EN;
    ptuStaticPar.ptuPower = PRU_STATIC_PAR_POWER_DEF;    
    ptuStaticPar.ptuClass = PRU_STATIC_PAR_CLASS_DEF - PTU_STATIC_PAR_CLASS_OFFSET;
    ptuStaticPar.ptuMaxSourceImpedance = PRU_STATIC_PAR_IMPEDANCE_DEF << PTU_STATIC_PAR_MAX_SOURCE_IMPEDANCE_SHIFT;
    ptuStaticPar.ptuMaxLoadResistance = PRU_STATIC_PAR_RESISTANCE_DEF << PTU_STATIC_PAR_MAX_LOAD_RESISTANCE_SHIFT; 
    ptuStaticPar.hardwareRev = PRU_STATIC_PAR_HARDW_REV_DEF;
    ptuStaticPar.firmwareRev = PRU_STATIC_PAR_FW_REV_DEF;
    ptuStaticPar.protocolRev = PRU_STATIC_PAR_PROTOCOL_REV_DEF;
    ptuStaticPar.ptuDevNumber = 1u - PTU_STATIC_PAR_NUMBER_OF_DEVICES_OFFSET;
    
}


/*******************************************************************************
* Function Name: WptsCallBack()
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service specific events from 
*   Wireless Power Transfer Service.
*
* Parameters:
*  event       - the event code
*  *eventParam - the event parameters
*
*******************************************************************************/
void WptsCallBack (uint32_t event, void *eventParam)
{
    cy_stc_ble_wpts_char_value_t *eventPar = (cy_stc_ble_wpts_char_value_t *)eventParam;
    cy_stc_ble_wpts_descr_value_t *descrValue = (cy_stc_ble_wpts_descr_value_t *)eventParam;
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    
    switch(event)
    {
        case CY_BLE_EVT_WPTSC_NOTIFICATION:
            DBG_PRINTF("Device %d<--CY_BLE_EVT_WPTSC_NOTIFICATION charIndex= #%d Value= ", eventPar->connHandle.attId, 
                eventPar->charIndex);
            ShowValue(eventPar->value);
            break;
            
        case CY_BLE_EVT_WPTSC_INDICATION:
            DBG_PRINTF("Device %d<--CY_BLE_EVT_WPTSC_INDICATION charIndex= #%d Value= ", eventPar->connHandle.attId,
                eventPar->charIndex);
            ShowValue(eventPar->value);
            break;
            
        case CY_BLE_EVT_WPTSC_WRITE_CHAR_RESPONSE:
            appConnInfo.central[eventPar->connHandle.attId].requestResponce = false;
            DBG_PRINTF("Device %d<--CY_BLE_EVT_WPTSC_WRITE_CHAR_RESPONSE: charIndex =%x \r\n",
                eventPar->connHandle.attId,
                eventPar->charIndex);
            if(eventPar->charIndex == CY_BLE_WPTS_PTU_STATIC_PAR)
            {
                /* Enable Notification */
                uint16_t cccd = CY_BLE_CCCD_NOTIFICATION;
                apiResult = Cy_BLE_WPTSC_SetCharacteristicDescriptor(eventPar->connHandle, CY_BLE_WPTS_PRU_ALERT,  
                                                                     CY_BLE_WPTS_CCCD, sizeof(cccd), 
                                                                    (uint8_t *)&cccd);
                DBG_PRINTF("Device %d-->Enable Alert Notification, apiResult: %x \r\n", eventPar->connHandle.attId, apiResult);
                if(apiResult == CY_BLE_SUCCESS)
                {
                    appConnInfo.central[eventPar->connHandle.attId].requestResponce = true;
                }
            }
            break;
            
        case CY_BLE_EVT_WPTSC_READ_CHAR_RESPONSE:
            appConnInfo.central[eventPar->connHandle.attId].requestResponce = false;
            
            /* Parse static characteristic value */
            if(eventPar->charIndex == CY_BLE_WPTS_PRU_STATIC_PAR)
            {
                cy_stc_ble_pru_static_par_t pruStaticPar;
                
                pruStaticPar = *(cy_stc_ble_pru_static_par_t *)(((cy_stc_ble_wpts_char_value_t *)eventParam)->value->val);
                appConnInfo.central[eventPar->connHandle.attId].pruStaticPar = pruStaticPar;
                
                DBG_PRINTF("Device %d<--PRU_STATIC_PARAMETER: flags: %x, protocol rev: %d, ", eventPar->connHandle.attId, 
                    pruStaticPar.flags, pruStaticPar.protocolRev);
                DBG_PRINTF("category: Category %d , ", pruStaticPar.pruCategory);
                DBG_PRINTF("information: %x , ", pruStaticPar.pruInformation);
                
                DBG_PRINTF("hardware rev: %d , firmware rev: %d ", pruStaticPar.hardwareRev, pruStaticPar.firmwareRev);
                DBG_PRINTF("Prect_max: %d mW, ", pruStaticPar.pRectMax * PRU_STATIC_PAR_PREACT_MAX_MULT);
                DBG_PRINTF("Vrect_min_static: %d mV, ", pruStaticPar.vRectMinStatic);
                DBG_PRINTF("Vrect_high_static: %d mV, ", pruStaticPar.vRectHighStatic);
                DBG_PRINTF("Vrect_set: %d mV, ", pruStaticPar.vRectSet);
                if((pruStaticPar.flags & PRU_STATIC_PAR_FLAGS_ENABLE_DELTA_R1) != 0u)
                {
                    DBG_PRINTF("deltaR1: %.2f ohms", (float)pruStaticPar.deltaR1 * PRU_STATIC_PAR_DELTA_R1_MULT);
                }
                DBG_PRINTF("\r\n");
                
                /* Write PTU Static Par */
                apiResult = Cy_BLE_WPTSC_SetCharacteristicValue(eventPar->connHandle, CY_BLE_WPTS_PTU_STATIC_PAR, 
                    sizeof(ptuStaticPar), (uint8_t *)&ptuStaticPar);
                DBG_PRINTF("Device %d-->Set PTU Static Parameter char value, apiResult: %x \r\n", 
                    eventPar->connHandle.attId, apiResult);
                if(apiResult == CY_BLE_SUCCESS)
                {
                    appConnInfo.central[eventPar->connHandle.attId].requestResponce = true;
                }
            }
            else if(eventPar->charIndex == CY_BLE_WPTS_PRU_DYNAMIC_PAR)
            {
                cy_stc_ble_pru_dynamic_par_t *pruDynamicPar;
                pruDynamicPar = (cy_stc_ble_pru_dynamic_par_t *)(((cy_stc_ble_wpts_char_value_t *)eventParam)->value->val);
                appConnInfo.central[eventPar->connHandle.attId].pruDynamicPar = *pruDynamicPar;
                
            #if(DEBUG_UART_EXTENDED)  
                DBG_PRINTF("Device %d<--PRU_DYNAMIC_PARAMETER: flags: %x,", eventPar->connHandle.attId, pruDynamicPar->flags);
                DBG_PRINTF("Vrect: %d mV, ", pruDynamicPar->vRect);
                DBG_PRINTF("Irect: %d mA, ", pruDynamicPar->iRect);
                if((pruDynamicPar->flags & PRU_DYNAMIC_PAR_FLAGS_VOUT_EN) != 0u)
                {
                    DBG_PRINTF("Vout: %d mV, ", pruDynamicPar->vOut);
                }
                if((pruDynamicPar->flags & PRU_DYNAMIC_PAR_FLAGS_IOUT_EN) != 0u)
                {
                    DBG_PRINTF("Iout: %d mA, ", pruDynamicPar->iOut);
                }
                if((pruDynamicPar->flags & PRU_DYNAMIC_PAR_FLAGS_TEMPERATURE_EN) != 0u)
                {
                    DBG_PRINTF("Temp: %d C, ", pruDynamicPar->temperature - PRU_DYNAMIC_PAR_TEMPERATURE_OFFSET);
                }
                if((pruDynamicPar->flags & PRU_DYNAMIC_PAR_FLAGS_VREACT_MIN_EN) != 0u)
                {
                    DBG_PRINTF("\r\n  VrectMinDyn: %d mV, ", pruDynamicPar->vRectMinDyn);
                }
                if((pruDynamicPar->flags & PRU_DYNAMIC_PAR_FLAGS_VREACT_SET_EN) != 0u)
                {
                    DBG_PRINTF("VrectSetDyn: %d mV, ", pruDynamicPar->vRectSetDyn);
                }
                if((pruDynamicPar->flags & PRU_DYNAMIC_PAR_FLAGS_VREACT_HIGH_EN) != 0u)
                {
                    DBG_PRINTF("VrectHighDyn: %d mV, ", pruDynamicPar->vRectHighDyn);
                }
                DBG_PRINTF("Alert: %x.\r\n", pruDynamicPar->alert);
            #endif /* (DEBUG_UART_EXTENDED) */
                if(appConnInfo.central[eventPar->connHandle.attId].pruState == PEER_DEVICE_STATE_ADDED)
                {
                    appConnInfo.central[eventPar->connHandle.attId].pruState = PEER_DEVICE_STATE_CONFIGURED;
                #if(DEBUG_UART_EXTENDED)  
                    DBG_PRINTF("Device %d-->: Configured\r\n", eventPar->connHandle.attId);
                #endif /* DEBUG_UART_EXTENDED */
                
                    /* Send Connection Parameter Update Request to Peripheral */
                    cy_stc_ble_gap_conn_update_param_info_t connUpdateParam =
                    {
                        .connIntvMin = SLOW_CONN_INTERVAL,
                        .connIntvMax = SLOW_CONN_INTERVAL,
                        .connLatency = cy_ble_configPtr->gapcScanParams[0].gapcConnectionSlaveLatency,
                        .supervisionTO = cy_ble_configPtr->gapcScanParams[0].gapcConnectionTimeOut,
                        .bdHandle = eventPar->connHandle.bdHandle
                    };
                    
                    apiResult = Cy_BLE_GAPC_ConnectionParamUpdateRequest(&connUpdateParam);
                    DBG_PRINTF("Device %d-->Send Connection Parameter Update Request to Peripheral, apiResult: %x \r\n", 
                        eventPar->connHandle.attId, apiResult);
                }
                
                /* Analyse PRU static characteristic and send charge enable command when charging is required */
                if(appConnInfo.central[eventPar->connHandle.attId].pruCharging == false)
                {
                    if((pruDynamicPar->alert & PRU_ALERT_CHARGE_COMPLETE) == 0u)
                    {
                        /* Charge enable */
                        appConnInfo.central[eventPar->connHandle.attId].pruControl.enables = 
                            PRU_CONTROL_ENABLES_ENABLE_CHARGE_INDICATOR;
                        apiResult = Cy_BLE_WPTSC_SetCharacteristicValue(eventPar->connHandle, CY_BLE_WPTS_PRU_CONTROL,
                            sizeof(appConnInfo.central[eventPar->connHandle.attId].pruControl), 
                            (uint8_t *)&appConnInfo.central[eventPar->connHandle.attId].pruControl);
                        DBG_PRINTF("Device %d-->Set PRU Control char (enable charging), apiResult: %x \r\n", 
                             eventPar->connHandle.attId, apiResult);
                        if(apiResult == CY_BLE_SUCCESS)
                        {
                            appConnInfo.central[eventPar->connHandle.attId].requestResponce = true;
                            appConnInfo.central[eventPar->connHandle.attId].pruCharging = true;
                        }
                    }
                }
                else
                {
                    if(((pruDynamicPar->alert & PRU_ALERT_OVER_VOLTAGE) != 0u) ||
                       ((pruDynamicPar->alert & PRU_ALERT_OVER_CURRENT) != 0u) ||
                       ((pruDynamicPar->alert & PRU_ALERT_OVER_TEMP) != 0u))
                    {   /* System error */
                    }
                    else if((pruDynamicPar->alert & PRU_ALERT_CHARGE_COMPLETE) != 0u)
                    {   /* Charge complete */
                        appConnInfo.central[eventPar->connHandle.attId].pruCharging = false;
                    }
                    else /* Self protection */
                    {
                    }
                }
            }
            else
            {
                DBG_PRINTF("Device %d<--CY_BLE_EVT_WPTSC_READ_CHAR_RESPONSE: charIndex =%x, ", 
                            eventPar->connHandle.attId, eventPar->charIndex);
                DBG_PRINTF(" value_len = %x, value = ", eventPar->value->len);
                ShowValue(eventPar->value);
            }
            break;
            
        case CY_BLE_EVT_WPTSC_READ_DESCR_RESPONSE:
            appConnInfo.central[eventPar->connHandle.attId].requestResponce = false;
            (void)*descrValue;
            DBG_PRINTF("Device %d<--CY_BLE_EVT_WPTSC_READ_DESCR_RESPONSE: charIndex =%x, ", eventPar->connHandle.attId, 
                descrValue->charIndex);
            DBG_PRINTF(" descrIndex =%x, value_len = %x, value = ", descrValue->descrIndex, descrValue->value->len);
            ShowValue(eventPar->value);
            break;
            
        case CY_BLE_EVT_WPTSC_WRITE_DESCR_RESPONSE:
            appConnInfo.central[eventPar->connHandle.attId].requestResponce = false;
            DBG_PRINTF("Device %d<--CY_BLE_EVT_WPTSC_WRITE_DESCR_RESPONSE charIndex =%x \r\n", eventPar->connHandle.attId, 
                eventPar->charIndex);
            if(eventPar->charIndex == CY_BLE_WPTS_PRU_ALERT)
            {
                /* Enable reading of PRU dynamic characteristic */
                 appConnInfo.central[eventPar->connHandle.attId].readingDynChar = 1u;
            }
            break;
            
        default:
            DBG_PRINTF("WPTS OTHER event: %lx \r\n", event);
            break;
    }
}


/****************************************************************************** 
* Function Name: WptsScanProcessEventHandler
*******************************************************************************
* 
* Summary:
*  This function handles CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT event for WPTS.
* 
* Parameters:
*  *eventParam: the pointer to a data structure specified by the event.
*  *serviceData: output parameter with service data
* 
* Return:
*  Non zero value when the advertising packet contains WPTS service data.
* 
******************************************************************************/
uint32_t WptsScanProcessEventHandler(cy_stc_ble_gapc_adv_report_param_t *eventParam, 
                                     cy_stc_ble_pru_adv_service_data_t *serviceData)
{
    uint32_t servicePresent = 0u; 
    uint32_t advIndex = 0u;
    
    do
    {
        /* Find Service Data AD type with WPT service UUID */
        if((eventParam->data[advIndex] >= PRU_ADV_SERV_DATA_LEN) &&
           (eventParam->data[advIndex + PRU_ADV_SERV_DATA_TYPE_OFFSET] == (uint8_t)CY_BLE_GAP_ADV_SRVC_DATA_16UUID) &&
           (Cy_BLE_Get16ByPtr(&eventParam->data[advIndex + PRU_ADV_SERV_DATA_SERV_OFFSET]) == 
                CY_BLE_UUID_WIRELESS_POWER_TRANSFER_SERVICE)
           )
        {
            serviceData->wptsServiceHandle = Cy_BLE_Get16ByPtr(&eventParam->data[advIndex + PRU_ADV_SERV_DATA_HANDLE_OFFSET]);
            serviceData->rssi = eventParam->data[advIndex + PRU_ADV_SERV_DATA_RSSI_OFFSET];
            serviceData->flags = eventParam->data[advIndex + PRU_ADV_SERV_DATA_FLAGS_OFFSET];
            servicePresent = 1u;
            break;
        }
        advIndex += eventParam->data[advIndex] + 1u;
    }while(advIndex < eventParam->dataLen);    
    
    return(servicePresent);
}

/* [] END OF FILE */

