/*******************************************************************************
* File Name: blss.h
*
* Version 1.0
*
* Description:
*  BLS service related code header.
*
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


#include "common.h"   
       
#define SIM_BLS_DLT     (10u)
#define SIM_UNIT_MAX    (59u)  /* seconds in minute */
#define SIM_ICF_MAX     (200u)
#define SIM_ICF_MSK     (0x07)
#define SIM_BPM_SYS_MIN (100u)
#define SIM_BPM_DIA_MIN (60u)
#define SIM_BPM_MSK     (0x38)
#define BLS_TIMEOUT     (2u)

/* Blood Pressure Measurement characteristic "Flags" bit field flags */
#define CY_BLE_BLS_BPM_FLG_BPU (0x01u)        /* Blood Pressure Units 0 = mmHg, 1 = kPa */
#define CY_BLE_BLS_BPM_FLG_TSP (0x02u)        /* Time Stamp */
#define CY_BLE_BLS_BPM_FLG_PRT (0x04u)        /* Pulse Rate */
#define CY_BLE_BLS_BPM_FLG_UID (0x08u)        /* User ID */
#define CY_BLE_BLS_BPM_FLG_MST (0x10u)        /* Measurement Status */

/* Blood Pressure Measurement characteristic "Measurement Status" bit field flags */
#define CY_BLE_BLS_BPM_MST_BMD      (0x0001)  /* Body Movement Detection */
#define CY_BLE_BLS_BPM_MST_CFD      (0x0002)  /* Cuff Fit Detection */
#define CY_BLE_BLS_BPM_MST_IPD      (0x0004)  /* Irregular Pulse Detection */
#define CY_BLE_BLS_BPM_MST_PRW      (0x0000)  /* Pulse Rate Range Detection: Pulse rate is within the range */
#define CY_BLE_BLS_BPM_MST_PRH      (0x0008)  /* Pulse Rate Range Detection: Pulse rate exceeds upper limit */
#define CY_BLE_BLS_BPM_MST_PRL      (0x0010)  /* Pulse Rate Range Detection: Pulse rate is less than lower limit */
#define CY_BLE_BLS_BPM_MST_PRM      (0x0018)  /* Pulse Rate Range Detection Mask */
#define CY_BLE_BLS_BPM_MST_MPD      (0x0020)  /* Measurement Position Detection */


typedef struct
{
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hours;
    uint8_t  minutes;
    uint8_t  seconds;
}cy_stc_ble_date_time_t;

typedef enum
{
    CY_BLE_TIME_EQUAL,
    CY_BLE_TIME_LESS,
    CY_BLE_TIME_GREAT
}cy_en_ble_date_time_comp_t;

typedef uint16_t sfloat; /* Placeholder for SFLOAT */

typedef struct
{
    uint8_t  flags;                 /* Flag */
    sfloat   sys;                   /* Systolic */
    sfloat   dia;                   /* Diastolic */
    sfloat   map;                   /* Mean Arterial Pressure */
    cy_stc_ble_date_time_t time;    /* Time Stamp */
    sfloat   prt;                   /* Pulse Rate */
    uint8_t  uid;                   /* User ID */
    uint16_t mst;                   /* Measurement Status */
}cy_stc_ble_bls_bpm_t;

/* Blood Pressure Feature characteristic value type */
typedef enum
{
    CY_BLE_BLS_BPF_BMD = 0x0001u,    /* Body Movement Detection Support */
    CY_BLE_BLS_BPF_CFD = 0x0002u,    /* Cuff Fit Detection Support */
    CY_BLE_BLS_BPF_IPD = 0x0004u,    /* Irregular Pulse Detection Support */
    CY_BLE_BLS_BPF_PRD = 0x0008u,    /* Pulse Rate Range Detection Support */
    CY_BLE_BLS_BPF_MPD = 0x0010u,    /* Measurement Position Detection Support */
    CY_BLE_BLS_BPF_MBS = 0x0020u,    /* Multiple Bond Support */
} cy_en_ble_bls_bpf_t;


/***************************************
*      Function prototypes
***************************************/
void BlsInit(void);
void BlsCallBack(uint32_t event, void* eventParam);
void BlsInd(cy_stc_ble_conn_handle_t connHandle, uint8_t num);
void BlsNtf(cy_stc_ble_conn_handle_t connHandle, uint8_t num);
void BlsSimulateBloodPressure(cy_stc_ble_conn_handle_t connHandle);

/* [] END OF FILE */
