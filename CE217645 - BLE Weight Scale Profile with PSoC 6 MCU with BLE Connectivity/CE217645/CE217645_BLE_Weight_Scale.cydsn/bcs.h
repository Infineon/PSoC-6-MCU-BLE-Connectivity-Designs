/*******************************************************************************
* File Name: bcs.h
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants used by Body Composition 
*  Service.
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "BLE.h"


/***************************************
*       Data Struct Definition
***************************************/
/* BCS Measurement value type */
typedef struct
{
    /* The flags have been included to support future extensions of the BCS
    * measurement descriptor.
    */
    uint16_t flags;
    uint16_t bodyFatPercentage;
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minutes;
    uint8_t  seconds;
    uint8_t  userId;
    uint16_t basalMetabolism;
    uint16_t musclePercentage;
    uint16_t muscleMassKg;
    uint16_t muscleMassLb;
    uint16_t fatFreeMassKg;
    uint16_t fatFreeMassLb;
    uint16_t softLeanMassKg;
    uint16_t softLeanMassLb;
    uint16_t bodyWatherMassKg;
    uint16_t bodyWatherMassLb;
    uint16_t impedance;
    uint16_t weightKg;
    uint16_t weightLb;
    uint16_t heightM;
    uint16_t heightIn;
}__PACKED bcs_measurement_value_t;


/***************************************
*          Constants
***************************************/
#define BCS_BC_MEASUREMENT_MAX_DATA_SIZE           (42u)
#define BCS_BC_FEATURE_CHAR_SIZE                   (4u)
#define BCS_FLAGS_COUNT                            (13u)
#define BCS_RET_SUCCESS                            (0u)
#define BCS_RET_FAILURE                            (1u)

/* Body Composition Feature constants */
#define BCS_TIME_STAMP_SUPPORTED                   (0x00000001ul)
#define BCS_MULTIPLE_USERS_SUPPORTED               (0x00000002ul)
#define BCS_BASAL_METABOLISM_SUPPORTED             (0x00000004ul)
#define BCS_MUSCLE_PERCENTAGE_SUPPORTED            (0x00000008ul)
#define BCS_MUSCLE_MASS_SUPPORTED                  (0x00000010ul)
#define BCS_FAT_FREE_MASS_SUPPORTED                (0x00000020ul)
#define BCS_SOFT_LEAN_MASS_SUPPORTED               (0x00000040ul)
#define BCS_BODY_WATER_MASS_SUPPORTED              (0x00000080ul)
#define BCS_IMPENDANCE_SUPPORTED                   (0x00000100ul)
#define BCS_WEIGHT_SUPPORTED                       (0x00000200ul)
#define BCS_HEIGHT_SUPPORTED                       (0x00000400ul)
#define BCS_WEIGHT_MASK                            (0x00007800ul)
#define BCS_HEIGHT_MASK                            (0x00028000ul)

#define BCS_MULTIPLE_USERS_SHIFT                   (1u)
#define BCS_BASAL_METABOLISM_SHIFT                 (2u)
#define BCS_MUSCLE_PERCENTAGE_SHIFT                (3u)
#define BCS_MUSCLE_MASS_SHIFT                      (4u)
#define BCS_FAT_FREE_MASS_SHIFT                    (5u)
#define BCS_SOFT_LEAN_MASS_SHIFT                   (6u)
#define BCS_BODY_WATER_MASS_SHIFT                  (7u)
#define BCS_IMPENDANCE_SHIFT                       (8u)
#define BCS_WEIGHT_SHIFT                           (9u)
#define BCS_HEIGHT_SHIFT                           (10u)
#define BCS_WEIGHT_MASK_SHIFT                      (11u)
#define BCS_HEIGHT_MASK_SHIFT                      (15u)

/* Body Composition Measurement Flags constants */
#define BCS_MEASUREMENT_UNITS_IMPERIAL             (0x0001u)
#define BCS_TIME_STAMP_PRESENT                     (0x0002u)
#define BCS_USER_ID_PRESENT                        (0x0004u)
#define BCS_BASAL_METABOLISM_PRESENT               (0x0008u)
#define BCS_MUSCLE_PERCENTAGE_PRESENT              (0x0010u)
#define BCS_MUSCLE_MASS_PRESENT                    (0x0020u)
#define BCS_FAT_FREE_MASS_PRESENT                  (0x0040u)
#define BCS_SOFT_LEAN_MASS_PRESENT                 (0x0080u)
#define BCS_BODY_WATER_MASS_PRESENT                (0x0100u)
#define BCS_IMPEDANCE_PRESENT                      (0x0200u)
#define BCS_WEIGHT_PRESENT                         (0x0400u)
#define BCS_HEIGHT_PRESENT                         (0x0800u)
#define BCS_MULTIPLE_PACKET_MEASUREMENT            (0x1000u)


/***************************************
*        Function Prototypes
***************************************/
void BcsInit(void);
void BcsCallBack(uint32_t event, void *eventParam);
uint8_t BcsPackIndicationData(uint8_t *pData, uint8_t *length, bcs_measurement_value_t *bMeasurement);


/***************************************
* External data references
***************************************/
extern bool                       isBcsIndicationEnabled;
extern bcs_measurement_value_t    bodyMeasurement[];
extern uint8_t                    indData[BCS_BC_MEASUREMENT_MAX_DATA_SIZE];
extern uint32_t                   bcsFeature;


/* [] END OF FILE */
