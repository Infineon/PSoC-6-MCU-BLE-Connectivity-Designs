/*******************************************************************************
* File Name: bas.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants available for BAS.
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
#ifndef BATTERY_TIMEOUT
    #define BATTERY_TIMEOUT    (3u)                 /* Battery simulation timeout */
#endif /* ifndef BATTERY_TIMEOUT */

#ifndef BAS_SIMULATE_ENABLE                         /* Enable simulations */
    #define BAS_SIMULATE_ENABLE    (1u)
#endif /* ifndef BAS_SIMULATE_ENABLE */

#ifndef SIM_BATTERY_MIN
    #define SIM_BATTERY_MIN    (2u)                 /* Minimum simulated battery level measurement */
#endif /* ifndef SIM_BATTERY_MIN */

#ifndef SIM_BATTERY_MAX
    #define SIM_BATTERY_MAX    (20u)                /* Maximum simulated battery level measurement */
#endif /* ifndef SIM_BATTERY_MAX */

#ifndef SIM_BATTERY_INCREMENT
    #define SIM_BATTERY_INCREMENT    (1u)           /* Value by which the battery level is incremented */
#endif /* ifndef SIM_BATTERY_INCREMENT */

#ifndef LOW_BATTERY_LIMIT
    #define LOW_BATTERY_LIMIT    (10)               /* Low level limit in percent to switch on LED */
#endif /* ifndef LOW_BATTERY_LIMIT */

#ifndef BAS_SERVICE_SIMULATE
    #define BAS_SERVICE_SIMULATE    (0u)            /* BAS service for simulation */
#endif /* ifndef BAS_SERVICE_SIMULATE */


/***************************************
*       Function Prototypes
***************************************/
void BasInit(void);
void BasCallBack(uint32_t event, void *eventParam);
void BasSimulateBattery(cy_stc_ble_conn_handle_t connHandle);
uint8_t BasGetBatteryLevel(void);


/* [] END OF FILE */
