/*******************************************************************************
* File Name: hrsc.h
*
* Version 1.0
*
* Description:
*  HRS service related code header.
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

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

#define CY_BLE_HRS_HRM_NTF_ENABLE        CY_BLE_CCCD_NOTIFICATION
#define CY_BLE_HRS_HRM_NTF_DISABLE       (0x0000u)


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

void HeartRateCallBack(uint32_t event, void* eventParam);
void HrsInit(void);
cy_en_ble_api_result_t HrscConfigHeartRateNtf(cy_stc_ble_conn_handle_t connHandle, uint16_t configuration);
void HrscUnPackHrm(cy_stc_ble_gatt_value_t* value);
uint16_t HrscGetRRInterval(uint8_t rrIntervalNumber);
cy_en_ble_api_result_t HrscResetEnergyExpendedCounter(cy_stc_ble_conn_handle_t connHandle);


/* Functions generated in macros */


/*******************************************************************************
* Function Name: Cy_BLE_HRSC_GetHeartRate
********************************************************************************
*
* Summary:
*   Gets the Heart Rate value from the
*   Heart Rate Measurument characteristic structure
*
* Return:
*   uint16_t heartRate.
*
*******************************************************************************/
#define HrscGetHeartRate()\
            (hrsHeartRate.heartRateValue)


/*******************************************************************************
* Function Name: Cy_BLE_HRSC_GetEnergyExpended
********************************************************************************
*
* Summary:
*   Gets the Energy Expended value from the
*   Heart Rate Measurument characteristic structure.
*
* Parameters:
*   None
*
* Return:
*   uint16_t energyExpended value.
*
*******************************************************************************/
#define HrscGetEnergyExpended()\
            (hrsHeartRate.energyExpendedValue)


/*******************************************************************************
* Function Name: Cy_BLE_HRSC_IsSensorContactSupported
********************************************************************************
*
* Summary:
*  Checks if the Sensor Contact feature is supported.
*
* Return:
*  bool: TRUE  - if the Sensor Contact feature is supported.
*        FALSE - otherwise.
*
*******************************************************************************/
#define HrscIsSensorContactSupported()\
            (0u != (hrsHeartRate.flags & CY_BLE_HRS_HRM_SC_SPRT))


/*******************************************************************************
* Function Name: Cy_BLE_HRSC_IsSensorContactDetected
********************************************************************************
*
* Summary:
*  Checks if the Sensor Contact detected.
*
* Return:
*  bool: TRUE  - if the Sensor Contact feature is supported and
*                the sensor contact is detected.
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
*   cy_stc_ble_conn_handle_t cy_ble_connHandle: connection handle which
*                           consists of device ID and ATT connection ID.
*
* Return:
*   uint16_t: API result will state if API succeeded
*           (CY_BLE_SUCCESS) or failed with error codes:
*   CY_BLE_SUCCESS - The request is sent successfully;
*   CY_BLE_ERROR_INVALID_PARAMETER - Validation of input parameter failed.
*******************************************************************************/
#define HrscReadBodySensorLocation()\
            (Cy_BLE_HRSC_GetCharacteristicValue(appConnHandle, CY_BLE_HRS_BSL))

/*******************************************************************************
* Function Name: HrscGetBodySensorLocation
********************************************************************************
*
* Summary:
*   Gets the Body Sensor Location characteristic value from the internal
*   variable.
*
* Return:
*   cy_en_ble_hrs_bsl_t Body Sensor Location value.
*
*******************************************************************************/
#define HrscGetBodySensorLocation() (cy_ble_hrscBodySensorLocation)


/***************************************
*      External data references
***************************************/
extern cy_stc_ble_hrs_hrm_t hrsHeartRate;     
extern uint16_t hrsNotification;


/* [] END OF FILE */
