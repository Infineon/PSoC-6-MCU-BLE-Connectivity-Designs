/*******************************************************************************
* File Name: bas.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants for BAS.
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"

/***************************************
*          Constants
***************************************/

#define BATTERY_TIMEOUT             (3u)       /* Counts depend on connection parameters */
#define BAS_SIMULATE_ENABLE         (1u)
#define SIM_BATTERY_MIN             (2u)        /* Minimum simulated battery level measurement */
#define SIM_BATTERY_MAX             (20u)       /* Maximum simulated battery level measurement */
#define SIM_BATTERY_INCREMENT       (1u)        /* Value by which the battery level is incremented */                             
#define LOW_BATTERY_LIMIT           (10)        /* Low level limit in percent to switch on LED */
#define LOW_BATTERY_LIMIT           (10)        /* Low level limit in percent to switch on LED */   
#define BAS_SERVICE_SIMULATE        (0u)        /* BAS service for simulation */

/***************************************
*       Function Prototypes
***************************************/
void BasInit(void);
void BasCallBack(uint32_t event, void *eventParam);
void ProcessingBatteryData(void);


/***************************************
* External data references
***************************************/
extern uint16_t batterySimulationNotify;



/* [] END OF FILE */
