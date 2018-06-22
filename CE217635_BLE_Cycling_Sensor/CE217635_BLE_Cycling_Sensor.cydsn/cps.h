/*******************************************************************************
* File Name: cps.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project.
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"


/***************************************
*      Data Struct Definition 
***************************************/

typedef struct
{
    uint16_t flags;                       /* Mandatory */
    int16_t instantaneousPower;           /* Mandatory, Unit is in watts with a resolution of 1 */
    uint32_t accumulatedTorque;           /* Unit is in newton meters with a resolution of 1/32, send only low 2 bytes */
    uint32_t cumulativeWheelRevolutions;  /* When present, these fields are always present as a pair */
    uint16_t lastWheelEventTime;          /* Unit is in seconds with a resolution of 1/2048 */
    uint32_t accumulatedEnergy;           /* Unit is in kilojoules with a resolution of 1, send only low 2 bytes */
}__PACKED cy_stc_ble_cps_power_measure_t;

typedef struct
{
    uint8_t flags;                        /* Mandatory */
    uint16_t cumulativeCrankRevolutions;
    uint16_t lastCrankEventTime;          /* Unit is in seconds with a resolution of 1/1024 */
}__PACKED cy_stc_ble_cps_power_vector_t;


/***************************************
*          Constants
***************************************/
#define CPS_CP_RESP_LENGTH                          (0u)
#define CPS_CP_RESP_OP_CODES                        (1u)
#define CPS_CP_RESP_REQUEST_OP_CODE                 (2u)
#define CPS_CP_RESP_VALUE                           (3u)
#define CPS_CP_RESP_PARAMETER                       (4u)

#define CPS_POWER_MEASURE_DATA_MAX_SIZE             (35u)
#define CPS_POWER_VECTOR_DATA_MAX_SIZE              (12u)

#define CPS_SIM_TORQUE_INIT                         (0xFDC0u)   /* Start value for rollover simulation */
#define CPS_SIM_TORQUE_INCREMENT                    (32u*10u)   /* Value by which the torque is incremented - 10 Nm */

#define CPS_WHEEL_EVENT_TIME_PER_SEC                (2048u)     /* Unit is in seconds with a resolution of 1/2048 */
#define CPS_SIM_WHEEL_EVENT_TIME_INIT               (63000u)    /* Start value for rollover simulation */
#define CPS_SIM_WHEEL_EVENT_TIME_INCREMENT          (2048u)     /* Value by which the torque is incremented - 1 sec */
#define CPS_SEC_IN_HOUR                             (3600u)     /* To convert speed to km per hour */

#define CPS_SIM_CUMULATIVE_WHEEL_REVOLUTION_INIT      (1000u)   /* Start value for Cumulative Wheel Revolution */
#define CPS_SIM_CUMULATIVE_WHEEL_REVOLUTION_INCREMENT (8u)      /* Value by which the torque is incremented - 1 sec */

#define CPS_WHEEL_CIRCUMFERENCE                     (0.0021f)   /* km */

#define CPS_SIM_ACCUMULATED_ENERGY_INIT             (65532u)    /* Start value for Accumulated Energy Value kJ */
#define CPS_SIM_ACCUMULATED_ENERGY_INCREMENT        (2u)        /* Value by which the energy is incremented - 2 kJ */

#define CPS_SIM_POWER_INIT                          (200u)      /* Start value for instantaneous power in W */ 

#define CPS_CRANK_EVENT_TIME_PER_SEC                (1024u)     /* Unit is in seconds with a resolution of 1/1024 */
#define CPS_SIM_CRANK_EVENT_TIME_INIT               (9300u)     /* Start value last crank event time */
#define CPS_SIM_CRANK_EVENT_TIME_INCREMENT          (1024u)     /* Value by which the torque is incremented - 1 sec */

#define CPS_SIM_CUMULATIVE_CRANK_REVOLUTION_INIT      (65470u)  /* Start value for Cumulative Crank Revolution */
#define CPS_SIM_CUMULATIVE_CRANK_REVOLUTION_INCREMENT (60u)     /* Value by which the torque is incremented  */

#define CPS_ADVERT_INTERVAL_NONCON                  (0x0640u)   /**< Advertising interval in 625 us units, i.e. 1 sec. */
/***************************************
*       Function Prototypes
***************************************/
void CpsCallback(uint32_t event, void *eventParam);
void CpsInit(void);
void CpsSimulateCyclingPower(cy_stc_ble_conn_handle_t connHandle);

/* [] END OF FILE */
