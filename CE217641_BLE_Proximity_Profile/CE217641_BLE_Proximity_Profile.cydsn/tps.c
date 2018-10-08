/*******************************************************************************
* File Name: tps.c
*
*  Version 1.0
*
* Description:
*  This file contains Tx Power Service callback handler function.
* 
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <common.h>
#include "tps.h"
#include "user_interface.h"

/***************************************
*        Global Variables
***************************************/
bool      isTpsNotificationPending = true;

/*******************************************************************************
* Function Name: TpsInit
********************************************************************************
*
* Summary: This function initialized parameters for TPS.
*
*******************************************************************************/
void TpsInit(void)
{
    /* Register the event handler for TPS specific events */
    Cy_BLE_TPS_RegisterAttrCallback(TpsServiceAppEventHandler);
}


/*******************************************************************************
* Function Name: TpsInitTxPower
********************************************************************************
*
* Summary: This function initialized tx power to +4 dBm for connection and
*          advertisement channels.
*
*******************************************************************************/
void TpsInitTxPower(void)
{
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_tx_pwr_lvl_info_t txPower = { .pwrConfigParam.bdHandle = 0xffu };

    /* Set Tx power level for connection channels to +4 dBm */
    txPower.pwrConfigParam.bleSsChId = CY_BLE_LL_CONN_CH_TYPE;
    txPower.blePwrLevel = CY_BLE_LL_PWR_LVL_MAX;
    apiResult = Cy_BLE_SetTxPowerLevel(&txPower);
    if(apiResult == CY_BLE_SUCCESS)
    {
        int8_t intTxPowerLevel;
        
        /* Convert power level to numeric int8_t value */
        intTxPowerLevel = TpsConvertTxPowerlevelToInt8(txPower.blePwrLevel);

        /* Set Tx power level for advertisement channels to +4 dBm */
        txPower.pwrConfigParam.bleSsChId = CY_BLE_LL_ADV_CH_TYPE;
        txPower.blePwrLevel = CY_BLE_LL_PWR_LVL_MAX;
        apiResult = Cy_BLE_SetTxPowerLevel(&txPower);
        
        if(apiResult == CY_BLE_SUCCESS)
        {
            /* Write the new Tx power level value to the GATT database */
            apiResult = Cy_BLE_TPSS_SetCharacteristicValue(CY_BLE_TPS_TX_POWER_LEVEL,
                                                           CY_BLE_TPS_TX_POWER_LEVEL_SIZE,
                                                           &intTxPowerLevel);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_TPSS_SetCharacteristicValue() error.\r\n");
            }
            
            /* Display new Tx Power Level value */
            DBG_PRINTF("Tx power level is set to %d dBm\r\n", intTxPowerLevel);
        }
        else
        {
            DBG_PRINTF("Cy_BLE_SetTxPowerLevel(CY_BLE_LL_ADV_CH_TYPE) error.\r\n"); 
        }
    }
    else 
    {
        DBG_PRINTF("Cy_BLE_SetTxPowerLevel(CY_BLE_LL_CONN_CH_TYPE) error: %x.\r\n", apiResult); 
    }
}


/*******************************************************************************
* Function Name: TpsServiceAppEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component,
*  which are specific to Tx Power Service.
*
* Parameters:
*  event      - The event code.
*  eventParam - The event parameters.
*
*******************************************************************************/
void TpsServiceAppEventHandler(uint32_t event, void *eventParam)
{
    switch(event)
    {
    /***************************************
    *        TPS Server events
    ***************************************/
    case CY_BLE_EVT_TPSS_NOTIFICATION_ENABLED:
        DBG_PRINTF("TPS notification enabled\r\n");
        break;
        
    case CY_BLE_EVT_TPSS_NOTIFICATION_DISABLED:
        DBG_PRINTF("TPS notification disabled\r\n");
        break;

    /***************************************
    *        TPS Client events
    ***************************************/
    case CY_BLE_EVT_TPSC_NOTIFICATION:
        break;
        
    case CY_BLE_EVT_TPSC_READ_CHAR_RESPONSE:
        break;
        
    case CY_BLE_EVT_TPSC_READ_DESCR_RESPONSE:
        break;
        
    case CY_BLE_EVT_TPSC_WRITE_DESCR_RESPONSE:
        break;
        
    default:
        break;
    }
}

/*******************************************************************************
* Function Name: TpsConvertTxPowerlevelToInt8
********************************************************************************
*
* Summary:
*  Converts Tx Power Level from cy_en_ble_bless_pwr_lvl_t to int8_t.
*
*******************************************************************************/
int TpsConvertTxPowerlevelToInt8(cy_en_ble_bless_pwr_lvl_t pwrLevel)
{
    int8_t intPwrLevel = 3;

    switch (pwrLevel)
    {
    case CY_BLE_LL_PWR_LVL_NEG_20_DBM:
        intPwrLevel = -20;
        break;
    case CY_BLE_LL_PWR_LVL_NEG_16_DBM:
        intPwrLevel = -16;
        break;
    case CY_BLE_LL_PWR_LVL_NEG_12_DBM:
        intPwrLevel = -12;
        break;
    case CY_BLE_LL_PWR_LVL_NEG_6_DBM:
        intPwrLevel = -6;
        break;
    case CY_BLE_LL_PWR_LVL_0_DBM:
        intPwrLevel = 0;
        break;
    case CY_BLE_LL_PWR_LVL_MAX:
        intPwrLevel = 4;
        break;
    default:
        break;
    }

    return(intPwrLevel);
}


/*******************************************************************************
* Function Name: TpsDecreaseTxPowerLevelValue
********************************************************************************
*
* Summary:
*  Decreases the Tx Power level by one scale lower.
*
*******************************************************************************/
void TpsDecreaseTxPowerLevelValue(cy_en_ble_bless_pwr_lvl_t * pwrLevel)
{
    isTpsNotificationPending = true;
    
    switch (*pwrLevel)
    {
    case CY_BLE_LL_PWR_LVL_NEG_20_DBM:
        *pwrLevel = CY_BLE_LL_PWR_LVL_NEG_20_DBM;
        isTpsNotificationPending = false;
        break;
    case CY_BLE_LL_PWR_LVL_NEG_16_DBM:
        *pwrLevel = CY_BLE_LL_PWR_LVL_NEG_20_DBM;
        break;
    case CY_BLE_LL_PWR_LVL_NEG_12_DBM:
        *pwrLevel = CY_BLE_LL_PWR_LVL_NEG_16_DBM;
        break;
    case CY_BLE_LL_PWR_LVL_NEG_6_DBM:
        *pwrLevel = CY_BLE_LL_PWR_LVL_NEG_12_DBM;
        break;
    case CY_BLE_LL_PWR_LVL_0_DBM:
        *pwrLevel = CY_BLE_LL_PWR_LVL_NEG_6_DBM;
        break;
    case CY_BLE_LL_PWR_LVL_MAX:
        *pwrLevel = CY_BLE_LL_PWR_LVL_0_DBM;

    default:
        break;
    }
}


/*******************************************************************************
* Function Name: TpsIsNotificationPending()
********************************************************************************
* Summary:
*  Get value of remove bond list flag
*
* Return:
*   true  - TPS notification pending flag is set
*   false - TPS notification pending flag is clear
*
*******************************************************************************/
bool TpsIsNotificationPending(void)
{
    return ((isTpsNotificationPending == true) ? true : false);
}

/* [] END OF FILE */
