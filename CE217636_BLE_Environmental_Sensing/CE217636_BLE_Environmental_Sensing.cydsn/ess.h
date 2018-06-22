/*******************************************************************************
* File Name: ess.h
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants used by Environmental 
*  Sensing Service.
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
#define CHARACTERISTIC_INSTANCE_1           (0u)
#define CHARACTERISTIC_INSTANCE_2           (1u)

#define TRIG_SETTINGS_DESCR_1               (0u)
#define TRIG_SETTINGS_DESCR_2               (1u)
#define TRIG_SETTINGS_DESCR_3               (2u)

#define NUMBER_OF_TRIGGERS                  (3u)
#define CY_BLE_ESS_LAST_TRIG_CONDITION      (0x09u)

#define TRUE_SPEED_VLUE_LENGTH              (2u)
#define INIT_WIND_SPEED                     (1500u)

#define HUMIDITY_VLUE_LENGTH                (2u)
#define INIT_HUMIDITY                       (200u)

#define NTF_INIT_TIMEOUT_VAL                (10u)

/* Characteristic/Descriptor sizes */
#define SIZE_1_BYTE                         (1u)
#define SIZE_2_BYTES                        (2u)
#define SIZE_3_BYTES                        (3u)
#define SIZE_4_BYTES                        (4u)
#define SIZE_5_BYTES                        (5u)

/* Wind Speed simulation data */
#define WIND_SPEED_MAX1                     (8000u)
#define WIND_SPEED_MIN1                     (1000u)
#define WIND_SPEED_MAX2                     (9000u)
#define WIND_SPEED_MIN2                     (700u)
#define WIND_UPDATE_STEP_1                  (120u)
#define WIND_UPDATE_STEP_2                  (20u)

/* Humidity simulation data */
#define HUMIDITY_MAX                        (9900u)
#define HUMIDITY_MIN                        (200u)
#define HUMIDITY_UPDATE_STEP                (140u)

/* Shift constants */
#define SHIFT_8_BITS                        (8u)
#define SHIFT_16_BITS                       (16u)

/* ES Trigger settings characteristic value offsets */
#define TRIG_CONDITION_OFFSET               (0u)
#define TRIG_OPERAND_OFFSET                 (1u)

#define MEASURE_UNCERTAINTY_NOT_AVAILABLE   (0xFFu)

/***************************************
* Data Struct Definition
***************************************/
/* ESS Measurement descriptor value type */
typedef struct
{
    /* The flags have been included to support future extensions of the ESS
    * measurement descriptor.
    */
    uint16_t flags;

    /* This field specifies the averaging operation or type of the sampling
    * function applying to the value of the ESS characteristic.
    */
    uint8_t samplingFunction;

    /* This field specifies the averaging time span, accumulation time,
    * or a measurement period in seconds over which the measurement is taken.
    */
    uint8_t measurementPeriod[CY_BLE_ESS_3BYTES_LENGTH];

    /* The value of the ESS characteristic shall be internally refreshed
    * by the Server at the frequency indicated in the Internal Update Interval
    * field.
    */
    uint8_t updateInterval[CY_BLE_ESS_3BYTES_LENGTH];

    /* This field specifies the intended application for which the ESS 
    * characteristic is designed to be used (refer to Environmental Sensing
    * Service spec for info).
    */
    uint8_t application;

    /* This field shows the measurement uncertainty of the data provided over
    * the supported range in the value of the ESS characteristic.
    */
    uint8_t measurementUncertanity;
} /* CY_BLE_CYPACKED_ATTR */ ess_measurement_value_t;

/* ESS Descriptor Value Changed Characteristic  value type */
typedef struct
{    
    /* Descriptor Value Changed Characteristic flag as per ESS spec. */
    uint8_t flags[CY_BLE_ESS_2BYTES_LENGTH];

    /* UUID for the affected ESS Characteristic */
    uint8_t uuid[CY_BLE_ESS_2BYTES_LENGTH];
} /* CY_BLE_CYPACKED_ATTR */ ess_descr_val_change_value_t;

   
/* Contains data for imitation of sensor */
typedef struct
{
    /* ESS characteristic Index*/ 
    cy_en_ble_ess_char_index_t   EssChrIndex;
    
    /* Number of Characteristic instance */
    uint8_t   chrInstance;
    
    /* Value of imitated parameter */
    uint16_t  value;
    
    /* Previous value of imitated parameter */
    uint16_t  prevValue;
    
    /* Maximum value of imitated parameter */
    uint16_t  valueMax;
    
    /* Minimum value of imitated parameter */
    uint16_t  valueMin;
    
    /* Step of imitated parameter changing */
    uint16_t  valueUpdateStep;
    
    /*value of ES Configuration descriptor */
    uint8_t   esConfig;
    
    /* Notification timeout value */
    uint32_t  ntfTimeoutVal;
    
    /* Notification timer value */
    uint32_t  ntfTimer;
    
    /* Value condition */
    uint8_t   valueCond[NUMBER_OF_TRIGGERS];

    /* Comparison value for parameter */
    uint32_t  cmpValue[NUMBER_OF_TRIGGERS];    

    uint8_t   sensorNewDataReady;
    
    uint8_t   isMeasurementPeriodElapsed;
    
    /* Measurement period in seconds. */
    uint32_t  measurementPeriod;
    
    /* Update Interval in seconds. */
    uint32_t  updateIntervalValue;
    
    /* Update Interval Timer. */
    uint32_t  updateIntervalTimer;
    
} cy_stc_ble_ess_characteristic_data_t;

/***************************************
*        Function Prototypes
***************************************/
void EssInit(void);
void EssInitCharacteristic(cy_stc_ble_ess_characteristic_data_t *sensorPtr);
void EssHandleIndication(cy_stc_ble_conn_handle_t connHandle);
void EssHandleNotificaion(cy_stc_ble_conn_handle_t connHandle, cy_stc_ble_ess_characteristic_data_t *sensorPtr);
void EssHandleDescriptorWriteOp(cy_stc_ble_ess_descr_value_t *descrValPtr);
uint8_t HandleNtfConditions(cy_stc_ble_conn_handle_t connHandle, cy_stc_ble_ess_characteristic_data_t *sensorPtr);
void EssSimulateProfile(cy_stc_ble_ess_characteristic_data_t *sensorPtr, uint32_t mainTimer);
void EssChkNtfAndSendData(cy_stc_ble_conn_handle_t connHandle, cy_stc_ble_ess_characteristic_data_t *sensorPtr);
void EssCallBack(uint32_t event, void *eventParam);
void GetUint24(uint32_t *u32, uint8_t u24Ptr[]);
char *EssCharIndexToText(cy_en_ble_ess_char_index_t EssChrIndex);
void EssSetIndicationValue(uint16_t localIndicationValue);


/* [] END OF FILE */
