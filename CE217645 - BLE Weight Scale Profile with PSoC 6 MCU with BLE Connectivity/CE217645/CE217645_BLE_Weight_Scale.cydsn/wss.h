/*******************************************************************************
* File Name: wss.h
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants used by Weight Scale Service.
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>


/***************************************
*       Data Struct Definition
***************************************/
/* WSS Measurement value type */
typedef struct
{
    uint8_t flags;
    uint16_t weightKg;
    uint16_t weightLb;
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minutes;
    uint8_t  seconds;
    uint8_t  userId;
    uint16_t bmi;
    uint16_t heightM;
    uint16_t heightIn;
}__PACKED wss_measurement_value_t;


/***************************************
*          Constants
***************************************/
#define WSS_MEASUREMENT_UNITS_IMPERIAL                  (0x01u)
#define WSS_MEASUREMENT_TIME_STAMP_PRESENT              (0x02u)
#define WSS_USER_ID_PRESENT                             (0x04u)
#define WSS_BMI_AND_HEIGHT_PRESENT                      (0x08u)

#define WSS_FLAGS_COUNT                                 (4u)
#define WSS_WEIGHT_FEATURE_CHAR_SIZE                    (4u)
#define WSS_WS_MEASUREMENT_MAX_DATA_SIZE                (19u)

#define WSS_WEIGHT_FEATURE_BYTE1                        (0u)
#define WSS_WEIGHT_FEATURE_BYTE2                        (1u)
#define WSS_WEIGHT_FEATURE_BYTE3                        (2u)
#define WSS_WEIGHT_FEATURE_BYTE4                        (3u)

#define WSS_DEFAULT_USER_ID                             (0u)

/* Return constants */
#define WSS_RET_SUCCESS                                 (0u)
#define WSS_RET_FAILURE                                 (1u)

#define WSS_SENSOR_TIMER_PERIOD                         (7u)


/***************************************
*        Function Prototypes
***************************************/
void WssInit(void);
void WssCallBack(uint32_t event, void *eventParam);
uint8_t WssPackIndicationData(uint8_t *pData, uint8_t *length, wss_measurement_value_t *wMeasurement);


/***************************************
* External data references
***************************************/
extern bool                      isWssIndicationEnabled;
extern bool                      isWssIndicationPending;
extern wss_measurement_value_t   weightMeasurement[];
extern uint8_t                   wssIndData[WSS_WS_MEASUREMENT_MAX_DATA_SIZE];


/* [] END OF FILE */
