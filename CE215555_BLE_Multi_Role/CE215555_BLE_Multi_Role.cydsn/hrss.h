/*******************************************************************************
* File Name: hrss.h
*
* Version 1.0
*
* Description:
*  The code header for the HRS service.
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>

#if !defined(HRSS_H)
#define HRSS_H


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

void HrsInit(void);
void HrsCallBack(uint32_t event, void* eventParam);
void HrsSimulateHeartRate(cy_stc_ble_conn_handle_t connHandle);
void HrsBridge(void);

void HrssSetEnergyExpended(uint16_t energyExpended);
void HrssAddRrInterval(uint16_t rrIntervalValue);
void HrssSendHeartRateNtf(cy_stc_ble_conn_handle_t connHandle);
void HrssSetBodySensorLocation(cy_en_ble_hrs_bsl_t location);
uint16_t HrssGetHrmDescriptor(cy_stc_ble_conn_handle_t connHandle);
void HrscUnPackHrm(cy_stc_ble_gatt_value_t* value);
uint16_t HrscGetRRInterval(uint8_t rrIntervalNumber);

/* Functions generated in macros */

/*******************************************************************************
* Function Name: HrscGetHeartRate
********************************************************************************
*
* Summary:
*   Gets the Heart Rate value from the
*   Heart Rate Measurement characteristic structure
*
* Return:
*   uint16_t The heartRate.
*
*******************************************************************************/
#define HrscGetHeartRate()\
            (hrsHeartRate.heartRateValue)


/*******************************************************************************
* Function Name: HrscGetEnergyExpended
********************************************************************************
*
* Summary:
*   Gets the Energy Expended value from the
*   Heart Rate Measurement characteristic structure.
*
* Return:
*   uint16_t The energyExpended value.
*
*******************************************************************************/
#define HrscGetEnergyExpended()\
            (hrsHeartRate.energyExpendedValue)


/*******************************************************************************
* Function Name: HrscIsSensorContactSupported
********************************************************************************
*
* Summary:
*  Checks if the Sensor Contact feature is supported.
*
* Return:
*  bool: TRUE  - if the Sensor Contact feature is supported,
*        FALSE - otherwise.
*
*******************************************************************************/
#define HrscIsSensorContactSupported()\
            (0u != (hrsHeartRate.flags & CY_BLE_HRS_HRM_SC_SPRT))


/*******************************************************************************
* Function Name: HrscIsSensorContactDetected
********************************************************************************
*
* Summary:
*  Checks if the Sensor Contact detected.
*
* Return:
*  bool: TRUE  - if the Sensor Contact feature is supported and
*                the sensor contact is detected,
*        FALSE - otherwise.
*
*******************************************************************************/
#define HrscIsSensorContactDetected()\
            (HrscIsSensorContactSupported() &&\
                (0u != (hrsHeartRate.flags & CY_BLE_HRS_HRM_SC_STAT)))


/*******************************************************************************
* Function Name: HrscReadBodySensorLocation
********************************************************************************
*
* Summary:
*   Initiates the Read Characteristic Request for Heart Rate Service Body Sensor
*   Location characteristic
*
* Parameters:
*   cy_stc_ble_conn_handle_t cy_ble_connHandle: The connection handle which
*                           consists of the device ID and ATT connection ID.
*
* Return:
*   uint16_t: An API result will state if the API succeeded
*           (CY_BLE_SUCCESS) or failed with error codes:
*   CY_BLE_SUCCESS - The request is sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER - Validation of the input parameter failed.
*
*******************************************************************************/
#define HrscReadBodySensorLocation()\
            (Cy_BLE_HRSC_GetCharacteristicValue(cy_ble_connHandle, CY_BLE_HRS_BSL))

/*******************************************************************************
* Function Name: HrscGetBodySensorLocation
********************************************************************************
*
* Summary:
*   Gets the Body Sensor Location characteristic value from the internal
*   variable.
*
* Return:
*   cy_en_ble_hrs_bsl_t The Body Sensor Location value.
*
*******************************************************************************/
#define HrscGetBodySensorLocation() (cy_ble_hrscBodySensorLocation)


/*******************************************************************************
* Function Name: HrssSetHeartRate
********************************************************************************
*
* Summary:
*   Sets Heart Rate value into the Heart Rate Measurement characteristic.
*
* Parameters:
*   uint16_t heartRate - The Heart Rate value to be set.
*
*******************************************************************************/
#define HrssSetHeartRate(heartRate)\
            (hrsHeartRate.heartRateValue = (heartRate))


/*******************************************************************************
* Function Name: HrssSensorContactSupportEnable
********************************************************************************
*
* Summary:
*   Sets the Sensor Contact Feature Support bit.
*
*******************************************************************************/
#define HrssSensorContactSupportEnable()\
            (hrsHeartRate.flags |= CY_BLE_HRS_HRM_SC_SPRT)


/*******************************************************************************
* Function Name: HrssSensorContactSupportDisable
********************************************************************************
*
* Summary:
*   Clears both Sensor Contact Feature bits.
*
*******************************************************************************/
#define HrssSensorContactSupportDisable()\
            (hrsHeartRate.flags &= (uint8_t) ~(CY_BLE_HRS_HRM_SC_SPRT | CY_BLE_HRS_HRM_SC_STAT))


/*******************************************************************************
* Function Name: HrssSensorContactIsDetected
********************************************************************************
*
* Summary:
*   Sets the Sensor Contact status bit.
*
*******************************************************************************/
#define HrssSensorContactIsDetected()\
            (hrsHeartRate.flags |= CY_BLE_HRS_HRM_SC_SPRT | CY_BLE_HRS_HRM_SC_STAT)


/*******************************************************************************
* Function Name: HrssSensorContactIsUndetected
********************************************************************************
*
* Summary:
*   Clears the Sensor Contact status bit.
*
*******************************************************************************/
#define HrssSensorContactIsUndetected()\
            (hrsHeartRate.flags &= (uint8_t) ~CY_BLE_HRS_HRM_SC_STAT)


/*******************************************************************************
* Function Name: HrssIsRrIntervalBufferFull
********************************************************************************
*
* Summary:
*   Checks if the RR-Interval buffer is full.
*
* Return:
*   bool: TRUE  - if the buffer is full,
*         FALSE - otherwise.
*
*******************************************************************************/
#define HrssIsRrIntervalBufferFull()\
            (hrssRrIntCnt >= CY_BLE_HRS_HRM_RRSIZE)

/*******************************************************************************
* Function Name: HrssAreThereRrIntervals
********************************************************************************
*
* Summary:
*   Checks if there are any RR-Intervals in the buffer.
*
* Return:
*   bool: TRUE  - if there is at least one RR-Interval,
*         FALSE - otherwise.
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
