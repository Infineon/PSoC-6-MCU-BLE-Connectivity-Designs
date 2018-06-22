/*******************************************************************************
* File Name: cscs.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants used by Cycling Speed and 
*  Cadence Service.
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"


/***************************************
*          Constants
***************************************/
#define CSC_WHEEL_REV_DATA_PRESENT              (0x01)
#define CSC_CRANK_REV_DATA_PRESENT              (0x02)

/* SC CP Characteristic constants */
#define SC_CP_CHAR_LENGTH_3BYTES                (3u)
#define SC_CP_CHAR_LENGTH_4BYTES                (4u)

/* CSC Measurement Characteristic's fields indexes */
#define CSC_CHAR_FLAGS_IDX                     (0u)
#define CSC_CHAR_CUMULATIVE_WHEEL_REV_IDX      (1u)
#define CSC_CHAR_LAST_WHEEL_EV_TIME_IDX        (5u)
#define CSC_CHAR_CUMULATIVE_CRANK_REV_IDX      (7u)
#define CSC_CHAR_LAST_CRANK_EV_TIME_IDX        (9u)

/* Time values */
#define CSC_TIME_PER_SEC                        (1024u) 
#define CSC_CRANK_EV_TIME_VAL                   (CSC_TIME_PER_SEC)
#define CSC_WHEEL_EV_TIME_VAL                   (CSC_TIME_PER_SEC)

/* Crank and wheel revolution values */
#define CSC_CRANK_REV_VAL                       (0x02u)
#define CSC_WHEEL_REV_VAL                       (0x05u)
#define CSC_WHEEL_REV_INIT_VAL                  (0x000077FFul)

/* SC Control Point Characteristic Procedure Request fields indexes */
#define CSCS_SC_CP_OP_CODE_IDX                  (0u)
#define CSCS_SC_CUM_VAL_BYTE0_IDX               (1u)
#define CSCS_SC_CUM_VAL_BYTE1_IDX               (2u)
#define CSCS_SC_CUM_VAL_BYTE2_IDX               (3u)
#define CSCS_SC_CUM_VAL_BYTE3_IDX               (4u)
#define CY_BLE_CSCS_SENSOR_LOC_IDX               (1u)

/* CSC Characteristic sizes */
#define CSC_MEASUREMENT_CHAR_SIZE               (11u)
#define CSC_FEATURE_SIZE                        (2u)
#define CSC_SC_CP_SIZE                          (3u)

/* SC Control Point Characteristic Procedure Response fields indexes */
#define CSCS_SC_CP_RESP_LEN_IDX                 (0u)
#define CSCS_SC_CP_RESP_OP_CODE_IDX             (1u)
#define CSCS_SC_CP_REQ_OP_CODE_IDX              (2u)
#define CSCS_SC_CP_RESP_VALUE_IDX               (3u)
#define CSCS_SC_CP_RESP_PARAMETER_IDX           (4u)

/* Lengths of SC Control Point Characteristic Procedures */
#define CSCS_SET_CUMMULATIVE_VALUE_LEN          (5u)
#define CSCS_START_SENSOR_CALIBRATION_LEN       (1u)
#define CSCS_UPDATE_SENSOR_LOCATION_LEN         (2u)
#define CSCS_REQ_SUPPORTED_SENSOR_LOCATION_LEN  (1u)

/* Speed calculation coefficients */
#define WHEEEL_CIRCUMFERENCE_CM                 (210u)
#define MS_TO_KMH_COEFITIENT                    (36u)
#define INT_DIVIDER                             (10u)
#define WHEEL_TIME_EVENT_UNIT                   (1024u)

/* Supported sensor locations */
#define TOP_OF_SHOE                             (1u)
#define IN_SHOE                                 (2u)
#define LEFT_CRANK                              (5u)
#define RIGHT_CRANK                             (6u)

/* Number of sensorsors supported by the example */
#define NUM_SUPPORTED_SENSORS                   (4u)

#define ONE_BYTE_SHIFT                          (8u)
#define TWO_BYTES_SHIFT                         (16u)
#define THREE_BYTES_SHIFT                       (24u)


/***************************************
*       Function Prototypes
***************************************/
void CscsCallback(uint32_t event, void *eventParam);
void CscsInit(void);
void CscsSimulateCyclingSpeed(cy_stc_ble_conn_handle_t connHandle);


/* [] END OF FILE */
