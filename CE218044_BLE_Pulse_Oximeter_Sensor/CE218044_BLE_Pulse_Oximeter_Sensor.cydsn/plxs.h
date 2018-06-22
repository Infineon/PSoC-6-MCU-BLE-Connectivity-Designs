/*******************************************************************************
* File Name: plxs.h
*
* Version 1.0
*
* Description:
*  The code header for the PLXS service.
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


#include "common.h"

#if !defined(PLXS_H) 
#define PLXS_H


/***************************************
*        Constant definitions
***************************************/                                        
#define SUPPORT                         (1u)
#define NOT_SUPPORT                     (0u)
                                        
#define PLXS_MAX_PDU_SIZE               (20u)
#define PLXS_RACP_BD_SIZE               (30u)                   /* 30 - minimal by spec */
                                                                         
/* Simulations defines */  
#define PLXS_SIM_TIMEOUT                (2u)
#define PLXS_SIM_SCMT_MT_PERIOD_COUNT   (35u)                   /* Counts of simulated SCMT records */
    
/* SpO2  range >= 95% */    
#define PLXS_SIM_MIN_SPO2               (95u)    
#define PLXS_SIM_MAX_SPO2               (100u)
                                        
/* Pr in range 50 - 100% */     
#define PLXS_SIM_MIN_PR                 (50u)    
#define PLXS_SIM_MAX_PR                 (100u)
                                        
/* PAI in range .... */         
#define PLXS_SIM_MIN_PAI                (1000u)                 /* 10%: 1000 * EXP_10M2 (10^-2) */    
#define PLXS_SIM_MAX_PAI                (2000u)                 /* 20%: 2000 * EXP_10M2 (10^-2) */    
                                                                               
#define PLXS_SIM_STEP_SPO2              (1u)  
#define PLXS_SIM_STEP_PR                (10u)
#define PLXS_SIM_STEP_PAI               (15u)                   /* step: 0.15 = 15 * EXP_10M2 (10^-2) */   
    
#define EXP_10M1                        (-1)                    /* exp: 10^-1 (0.1)  */    
#define EXP_10M2                        (-2)                    /* exp: 10^-2 (0.01) */       
   
/* Special Short Float Value */   
#define SFLOAT_NAN  (0x07ffu)                                   /* not a number */
#define SFLOAT_NRES (0x0800u)                                   /* not at this resolution */
#define SFLOAT_PINF (0x07feu)                                   /* + infinity */
#define SFLOAT_NINF (0x0802u)                                   /* - infinity */
#define SFLOAT_RSRV (0x0801u)                                   /* reserved for future use */
 
/* Defined sizes which using in pack/unpack procedures */
    
#define SIZE_8BIT                       (1u)
#define SIZE_16BIT                      (2u)
#define SIZE_24BIT                      (3u)
#define SIZE_TIMESTAMP                  (7u)
    
/***************************************
*            Data Types 
***************************************/
        
/* Timestamp structure  */   
typedef struct
{
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hours;
    uint8_t  minutes;
    uint8_t  seconds;
}cy_stc_ble_date_time_t;

/* SFLOAT structure  */
typedef struct
{
    #if defined(__ARMCC_VERSION)
        #pragma anon_unions
    #endif /* defined(__ARMCC_VERSION) */
    
    union 
    {
        int16_t value;
        struct /* Mantissa / Exponent */
        {    
            int16_t m                   :12;   /* Mantissa (12 bits) */  
            int16_t exp                 :4;    /* Exponent (4 bits) */ 
        };
    };
}sfloat_t;


/* 
*    The PLX structures, which represent PLX characteristic
*/

/* Structure of "Device and Sensor Status " bit fields  */
typedef struct /*  (0 - False, 1 - True) */
{            
    uint32_t EDU                        : 1;    /* Extended Display Update Ongoing */
    uint32_t EMD                        : 1;    /* Equipment Malfunction Detected */
    uint32_t SPID                       : 1;    /* Signal Processing Irregularity Detected */
    uint32_t ISD                        : 1;    /* Inadequate Signal Detected */
    uint32_t PSD                        : 1;    /* Poor Signal Detected */
    uint32_t LPD                        : 1;    /* Low Perfusion Detected */
    uint32_t ESD                        : 1;    /* Erratic Signal Detected */
    uint32_t NSD                        : 1;    /* Nonpulsatile Signal Detected */
    uint32_t QPD                        : 1;    /* Questionable Pulse Detected */
    uint32_t SA                         : 1;    /* Signal Analysis Ongoing */
    uint32_t SID                        : 1;    /* Sensor Interface Detected */
    uint32_t SUTU                       : 1;    /* Sensor Unconnected to User */
    uint32_t USC                        : 1;    /* Unknown Sensor Connected */
    uint32_t SD                         : 1;    /* Sensor Displaced */        
    uint32_t SM                         : 1;    /* Sensor Malfunctioning */        
    uint32_t SDISC                      : 1;    /* Sensor Disconnected */         
    uint32_t RESERVED                   : 9;    /* Reserved for future use  */        
}device_sensor_status_bits_t;

/* Structure of "Measurement Status" bit fields */
typedef struct  /* (0 - False, 1 - True) */
{            
    uint16_t RESERVED                   : 5;    /* Reserved for future use  */
    uint16_t MEAS                       : 1;    /* Measurement Ongoing */
    uint16_t EED                        : 1;    /* Early Estimated Data */
    uint16_t VDATA                      : 1;    /* Validated Data */
    uint16_t FQDATA                     : 1;    /* Fully Qualified Data */
    uint16_t DFMS                       : 1;    /* Data from Measurement Storage */
    uint16_t DFDEMO                     : 1;    /* Data for Demonstration */
    uint16_t DFTEST                     : 1;    /* Data for Testing */
    uint16_t CALIB                      : 1;    /* Calibration Ongoing */
    uint16_t MUN                        : 1;    /* Measurement Unavailable */
    uint16_t QMD                        : 1;    /* Questionable Measurement Detected */
    uint16_t IMD                        : 1;    /* Invalid Measurement Detected */          
}meas_status_bits_t;

/* The PLX Spot-check Measurement characteristic structure defines */ 
typedef struct
{
    /* Flags */
    union 
    {
        uint8_t value;
        struct /* "Flags" bit fields (0 - False, 1 - True) */
        {            
            uint8_t TMSF                : 1;    /* Timestamp field is present */
            uint8_t MSF                 : 1;    /* Measurement Status Field Present */
            uint8_t DSSF                : 1;    /* Device and Sensor Status Field Present */
            uint8_t PAIF                : 1;    /* Pulse Amplitude Index field is present */
            uint8_t NO_DEVCLK           : 1;    /* Device Clock is Not Set */
            uint8_t RESERVED            : 3;    /* Reserved for future use */
        }bit;
    }flags;
    
    /* SpO2PR-Spot-check - SpO2 */
    sfloat_t spO2;
    
    /* SpO2PR-Spot-check - Pr */
    sfloat_t Pr;
    
    /* Timestamp */
    cy_stc_ble_date_time_t timestamp;
    
    /* Measurement Status */
    union 
    {
        uint16_t value;
        meas_status_bits_t bit;
    }measStatus; 
        
    /* Device and Sensor Status */
    union 
    {
        uint32_t value;
        device_sensor_status_bits_t bit;
    }dsStatus;
    
    /* Pulse Amplitude Index */
    sfloat_t pulseAmpIndex;
    
}cy_stc_ble_plxs_scmt_char_t;

/* The PLX Continuous Measurement characteristic structure defines */
typedef struct
{
    /* Flags */
    union 
    {
        uint8_t value;
        struct /* "Flags" bit fields (0 - False, 1 - True) */
        {            
            uint8_t FAST                : 1;    /* SpO2PR–Fast field is present */
            uint8_t SLOW                : 1;    /* SpO2PR-Slow field is present */
            uint8_t MSF                 : 1;    /* Measurement Status field is present */
            uint8_t DSSF                : 1;    /* Device and Sensor Status field is present */
            uint8_t PAIF                : 1;    /* Pulse Amplitude Index field is present */
            uint8_t RESERVED            : 3;    /* Reserved for future use */
        }bit;
    }flags; 
    
    /* SpO2PR-Normal - SpO2 */
    sfloat_t normalSpO2;
    
    /* SpO2PR-Normal - PR */
    sfloat_t normalPr;
    
    /* SpO2PR-Fast - SpO2 */
    sfloat_t fastSpO2;
    
    /* SpO2PR-Fast - PR */
    sfloat_t fastPr;
    
    /* SpO2PR-Slow - SpO2 */
    sfloat_t slowSpO2;
    
    /* SpO2PR-Slow - PR */
    sfloat_t slowPr;
    
    /* Measurement Status */
    union 
    {
        uint16_t value;
        meas_status_bits_t bit;
    }measStatus; 
        
    /* Device and Sensor Status */
    union
    {
        uint32_t value;
        device_sensor_status_bits_t bit;
    }dsStatus;
    
    /* Pulse Amplitude Index */
    sfloat_t pulseAmpIndex;       
    
}cy_stc_ble_plxs_ctmt_char_t;

/* The PLX Features characteristic structure defines */
typedef struct
{
    /* Supported Features */
    union 
    {
        uint16_t value;
        struct  /* "Supported Features" bit fields (0 - NOT SUPPORT, 1 - SUPPORT) */
        {                  
            uint16_t MEAS               : 1;    /* Measurement Status support is present */
            uint16_t DSS                : 1;    /* Device and Sensor Status support is present */
            uint16_t MSSC               : 1;    /* Measurement Storage for Spot-check measurements is supported */
            uint16_t TMSF               : 1;    /* Timestamp for Spot-check measurements is supported */
            uint16_t FAST               : 1;    /* SpO2PR-Fast metric is supported */
            uint16_t SLOW               : 1;    /* SpO2PR-Slow metric is supported */
            uint16_t PAI                : 1;    /* Pulse Amplitude Index field is supported */
            uint16_t MBS                : 1;    /* Multiple Bonds Supported */   
            uint16_t RESERVED           : 8;    /* Reserved for future use */
        }bit;
    }supportedFeatures; 
    
    /* Measurement Status Support */
    union 
    {
        uint16_t value;
        meas_status_bits_t bit;
    }measStatusSupport; 
        
    /* Device and Sensor Status */
    union 
    {
        uint32_t value;
        device_sensor_status_bits_t bit;
    }dsStatusSupport;  
    
}cy_stc_ble_plxs_feat_char_t;

/* The PLX Record Access Control Point characteristic structure defines */
typedef struct
{
    cy_en_ble_plxs_racp_opc_t opCode;
    cy_en_ble_plxs_racp_opr_t operator;
    
    union
    {
        uint16_t value;                                 
        struct
        {
            cy_en_ble_plxs_racp_opc_t  reqOpCode;
            cy_en_ble_plxs_racp_rsp_t  rspCode;
        }rsp;

    }operand;
    
    bool operandIsPresent;                              /* set True when operand byte came */
    
}cy_stc_ble_plxs_racp_char_t;


/* 
*    The RACP Operations structures / enums 
*/

/* The application RACP States */
typedef enum
{
    APP_RACP_STATE_IDLE             = 0u,
    APP_RACP_STATE_RECEIVED_OPCODE  = 1u,
    APP_RACP_STATE_SEND_INDICATION  = 2u,
    APP_RACP_STATE_ERROR_UNSPRT_OPC = 3u,
    APP_RACP_STATE_ERROR_UNSPRT_OPR = 4u,
    APP_RACP_STATE_ERROR_UNSPRT_OPD = 5u,
    APP_RACP_STATE_ERROR_INV_OPR    = 6u,
    APP_RACP_STATE_ERROR_INV_OPD    = 7u   
}cy_en_ble_plxs_racp_app_state_t;

/* The application RACP Storage */
typedef struct
{
    uint32_t                     count;                             /* Count of stored records */
    cy_stc_ble_plxs_scmt_char_t  records[PLXS_RACP_BD_SIZE];        /* Storage */
    uint32_t                     head;                              /* Pointer to head */
    uint32_t                     tail;                              /* Pointer to tail */
}cy_stc_ble_plxs_racp_storage_t;

/* The RACP application operation structure */
typedef struct
{
    cy_en_ble_plxs_racp_app_state_t state;                          /* State using for processing RACP Op codes */
    cy_stc_ble_plxs_racp_storage_t  storage;                        /* Storage RACP records */        
}cy_stc_ble_plxs_racp_t;

  

/***************************************
*        Function Prototypes
***************************************/
void PlxsInit(void);
void PlxsCallBack(uint32_t event, void* eventParam);
void PlxsSimulateMeasurement(cy_stc_ble_conn_handle_t connHandle);
void PlxsStartSpotCheckMeasurement(void);
bool PlxsIsSpotCheckMeasurement(void);

/* RACP support procedures */
void PlxsRacpProcess(cy_stc_ble_conn_handle_t connHandle);
void PlxsRacpStoragePrintRecords(void);
void PlxsRacpStorageDeleteAll(void);
void PlxsRacpStoragePushRecord(cy_stc_ble_plxs_scmt_char_t *data);
uint32_t PlxsRacpStoragePopRecord(cy_stc_ble_plxs_scmt_char_t *retData);

/* Functions for Pack/UnPack characteristics structure */
uint8_t PlxsPackData(cy_en_ble_plxs_char_index_t charIdx, void *inData, uint8_t *outData);
uint32_t PlxsUnPackData(cy_en_ble_plxs_char_index_t charIdx,uint8_t inLen, uint8_t *inData, void *outData);

/* Helper functions */
uint32_t Cy_BLE_Get32ByPtr(const uint8_t ptr[]);

#endif /* PLXS_H */


/* [] END OF FILE */
