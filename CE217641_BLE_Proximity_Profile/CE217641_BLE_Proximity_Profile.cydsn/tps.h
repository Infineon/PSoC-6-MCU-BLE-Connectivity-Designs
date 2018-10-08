/*******************************************************************************
* File Name: tps.h
*
*  Version 1.0
*
* Description:
*  This file contains routines related for the Tx Power Service
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include "common.h"

/***************************************
*          API Constants
***************************************/
#define TX_POWER_LEVEL_MAX                  (0)
#define TX_POWER_LEVEL_MIN                  (-18)


/***************************************
*       Function Prototypes
***************************************/
void TpsInit(void);
void TpsInitTxPower(void);
void TpsServiceAppEventHandler(uint32_t event, void * eventParam);
int TpsConvertTxPowerlevelToInt8(cy_en_ble_bless_pwr_lvl_t pwrLevel);
void TpsDecreaseTxPowerLevelValue(cy_en_ble_bless_pwr_lvl_t * pwrLevel);
bool TpsIsNotificationPending(void);



/* [] END OF FILE */
