/*******************************************************************************
* File Name: hrss.h
*
* Version 1.0
*
* Description:
*  HRS related code header.
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(HRSS_H)
#define HRSS_H

#include "common.h"

/***************************************
*        Constant definitions
***************************************/
/* Definitions for Heart Rate Measurement characteristic */
#define CY_BLE_HRS_HRM_HRVAL16           (0x01u)
#define CY_BLE_HRS_HRM_SC_SPRT           (0x04u)
#define CY_BLE_HRS_HRM_SC_STAT           (0x02u)
#define CY_BLE_HRS_HRM_ENEXP             (0x08u)
#define CY_BLE_HRS_HRM_RRINT             (0x10u)

#define CY_BLE_HRS_HRM_CHAR_LEN          (20u)        /* for default 23-byte MTU */
/* RR-Interval buffer size = ((char size)-(flags: 1 byte)-(Heart Rate: min 1 byte))/(RR-Interval size: 2 bytes) */
#define CY_BLE_HRS_HRM_RRSIZE            ((CY_BLE_HRS_HRM_CHAR_LEN - 2u) / 2u)
#define CY_BLE_ENERGY_EXPENDED_MAX_VALUE (0xFFFFu)   /* kilo Joules */
#define CY_BLE_HRS_RRCNT_OL              (0x80u)

/* Energy expended is typically only included in the Heart Rate Measurement characteristic
*  once every 10 measurements at a regular interval.
*/
#define HRS_TIMEOUT                     (2u)        /* Counts of 0.5 second */
#define ENERGY_EXPECTED_TIMEOUT         (10u)       /* 10 seconds */
#define SENSOR_CONTACT_TIMEOUT          (3u)        /* 3 seconds */

#define SIM_HEART_RATE_MIN              (60u)       /* Minimum simulated heart rate measurement */
#define SIM_HEART_RATE_MAX              (300u)      /* Maximum simulated heart rate measurement */
#define SIM_HEART_RATE_INCREMENT        (12u)       /* Value by which the heart rate is incremented */
#define SIM_ENERGY_EXPENDED_INCREMENT   (20u)       /* Value by which the Energy is incremented */
#define SENSOR_CONTACT_DETECTED         (0x01u)


/***************************************
*            Data Types
***************************************/
/* Heart Rate Measurement characteristic data structure type */
typedef struct
{
    uint8_t flags;
    uint16_t heartRateValue;
    uint16_t energyExpendedValue;
    uint16_t rrInterval[CY_BLE_HRS_HRM_RRSIZE];
}cy_stc_ble_hrs_hrm_t;

/* Body Sensor Location characteristic value type */
typedef enum
{
    CY_BLE_HRS_BODY_SENSOR_LOCATION_OTHER,
    CY_BLE_HRS_BODY_SENSOR_LOCATION_CHEST,
    CY_BLE_HRS_BODY_SENSOR_LOCATION_WRIST,
    CY_BLE_HRS_BODY_SENSOR_LOCATION_FINGER,
    CY_BLE_HRS_BODY_SENSOR_LOCATION_HAND,
    CY_BLE_HRS_BODY_SENSOR_LOCATION_EAR_LOBE,
    CY_BLE_HRS_BODY_SENSOR_LOCATION_FOOT
}cy_en_ble_hrs_bsl_t;


/***************************************
*        Function Prototypes
***************************************/

void HrsCallBack(uint32_t event, void* eventParam);
void HrsSimulateHeartRate(cy_stc_ble_conn_handle_t connHandle);
void HrsInit(void);
void HrssSetEnergyExpended(uint16_t energyExpended);
void HrssAddRrInterval(uint16_t rrIntervalValue);
void HrssSendHeartRateNtf(cy_stc_ble_conn_handle_t connHandle);
void HrssSetBodySensorLocation(cy_en_ble_hrs_bsl_t location);


/* Functions generated in macros */

/*******************************************************************************
* Function Name: Cy_BLE_HRSS_SetHeartRate
********************************************************************************
*
* Summary:
*  Sets Heart Rate value into the Heart Rate Measurument characteristic.
*
* Parameters:
*  uint16_t heartRate - Heart Rate value to be set.
*
*******************************************************************************/
#define HrssSetHeartRate(heartRate)\
            (hrsHeartRate.heartRateValue = (heartRate))


/*******************************************************************************
* Function Name: Cy_BLE_HRSS_SensorContactSupportEnable
********************************************************************************
*
* Summary:
*  Sets the Sensor Contact Feature Support bit.
*
*******************************************************************************/
#define HrssSensorContactSupportEnable()\
            (hrsHeartRate.flags |= CY_BLE_HRS_HRM_SC_SPRT)


/*******************************************************************************
* Function Name: Cy_BLE_HRSS_SensorContactSupportDisable
********************************************************************************
*
* Summary:
*  Clears both Sensor Contact Feature bits.
*
*******************************************************************************/
#define HrssSensorContactSupportDisable()\
            (hrsHeartRate.flags &= (uint8_t) ~(CY_BLE_HRS_HRM_SC_SPRT | CY_BLE_HRS_HRM_SC_STAT))


/*******************************************************************************
* Function Name: Cy_BLE_HRSS_SensorContactIsDetected
********************************************************************************
*
* Summary:
*  Sets the Sensor Contact Status bit.
*
*******************************************************************************/
#define HrssSensorContactIsDetected()\
            (hrsHeartRate.flags |= CY_BLE_HRS_HRM_SC_SPRT | CY_BLE_HRS_HRM_SC_STAT)


/*******************************************************************************
* Function Name: Cy_BLE_HRSS_SensorContactIsUndetected
********************************************************************************
*
* Summary:
*  Clears the Sensor Contact Status bit.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
#define HrssSensorContactIsUndetected()\
            (hrsHeartRate.flags &= (uint8_t) ~CY_BLE_HRS_HRM_SC_STAT)


/*******************************************************************************
* Function Name: Cy_BLE_HRSS_IsRrIntervalBufferFull
********************************************************************************
*
* Summary:
*  Checks if the RR-Interval buffer is full.
*
* Return:
*  bool: TRUE  - if the buffer is full,
*        FALSE - otherwise.
*
*******************************************************************************/
#define HrssIsRrIntervalBufferFull()\
            (hrssRrIntCnt >= CY_BLE_HRS_HRM_RRSIZE)

/*******************************************************************************
* Function Name: Cy_BLE_HRSS_AreThereRrIntervals
********************************************************************************
*
* Summary:
*  Checks if there are any RR-Intervals in the buffer.
*
* Return:
*  bool: TRUE  - if there is at least one RR-Interval,
*        FALSE - otherwise.
*
*******************************************************************************/
#define HrssAreThereRrIntervals()\
            ((hrsHeartRate.flags & CY_BLE_HRS_HRM_RRINT) != 0u)


/***************************************
*      External data references
***************************************/
/* Heart Rate Measurement characteristic data structure */
extern cy_stc_ble_hrs_hrm_t hrsHeartRate;
extern uint8_t hrssRrIntPtr;
extern uint8_t hrssRrIntCnt;


#endif /* HRSS_H */

/* [] END OF FILE */
