/*******************************************************************************
* File Name: cgmss.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project.
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CGMSS_H)
#define CGMSS_H

#include "common.h"
   
typedef uint16_t sfloat;
#define SFLOAT_NAN  (0x07ffu) /* not a number */
#define SFLOAT_NRES (0x0800u) /* not at this resolution */
#define SFLOAT_PINF (0x07feu) /* + infinity */
#define SFLOAT_NINF (0x0802u) /* - infinity */
#define SFLOAT_RSRV (0x0801u) /* reserved for future use */

#define REC_STATUS_OK      (0u)
#define REC_STATUS_DELETED (1u)
#define REC_NUM            (3u)

#define CGMS_FLAG_SOCP (0x01u)
#define CGMS_FLAG_RACP (0x02u)

#define CY_BLE_CGMS_CRC_SEED (0xFFFFu) /* CRC-CCITT initial seed value */
#define CY_BLE_CGMS_CRC_POLY (0x8408u) /* CRC-CCITT polynomial is D16+D12+D5+1 in reverse order */

#define CY_BLE_CGMS_SSTM_SIZE    (9u)
#define CY_BLE_CGMS_CRC_SIZE     (2u)
#define CY_BLE_SIM_ALERT_LEVEL   (5u)

/* CGM Measurement characteristic "Flags" bit field flags */
#define CY_BLE_CGMS_GLMT_FLG_TI (0x01u) /* CGM Trend Information Present */
#define CY_BLE_CGMS_GLMT_FLG_QA (0x02u) /* CGM Quality Present */
#define CY_BLE_CGMS_GLMT_FLG_WG (0x20u) /* Sensor Status Annunciation Field, Warning-Octet present */
#define CY_BLE_CGMS_GLMT_FLG_CT (0x40u) /* Sensor Status Annunciation Field, Cal/Temp-Octet present */
#define CY_BLE_CGMS_GLMT_FLG_ST (0x80u) /* Sensor Status Annunciation Field, Status-Octet present */

/* CGM Measurement characteristic "Sensor Status Annunciation" bit field flags */
#define CY_BLE_CGMS_GLMT_SSA_SS      (0x00000001u)  /* Session Stopped */
#define CY_BLE_CGMS_GLMT_SSA_BL      (0x00000002u)  /* Device Battery Low */
#define CY_BLE_CGMS_GLMT_SSA_TE      (0x00000004u)  /* Sensor type incorrect for device */
#define CY_BLE_CGMS_GLMT_SSA_SM      (0x00000008u)  /* Sensor malfunction */
#define CY_BLE_CGMS_GLMT_SSA_DA      (0x00000010u)  /* Device Specific Alert */
#define CY_BLE_CGMS_GLMT_SSA_DF      (0x00000020u)  /* General device fault has occurred in the sensor */
#define CY_BLE_CGMS_GLMT_SSA_WGM     (0x0000003Fu)  /* Warning-Octet mask */
    
#define CY_BLE_CGMS_GLMT_SSA_TS      (0x00000100u)  /* Time synchronization between sensor and collector required */
#define CY_BLE_CGMS_GLMT_SSA_CN      (0x00000200u)  /* Calibration not allowed */
#define CY_BLE_CGMS_GLMT_SSA_CR      (0x00000400u)  /* Calibration recommended */
#define CY_BLE_CGMS_GLMT_SSA_CL      (0x00000800u)  /* Calibration required */
#define CY_BLE_CGMS_GLMT_SSA_TH      (0x00001000u)  /* Sensor Temperature too high for valid test/result at time of measurement */
#define CY_BLE_CGMS_GLMT_SSA_TL      (0x00002000u)  /* Sensor temperature too low for valid test/result at time of measurement */
#define CY_BLE_CGMS_GLMT_SSA_CTM     (0x00003F00u)  /* Cal/Temp-Octet mask */
#define CY_BLE_CGMS_GLMT_SSA_CTS     (8u)           /* Cal/Temp-Octet shift */
    
#define CY_BLE_CGMS_GLMT_SSA_RL      (0x00010000u)  /* Sensor result lower than the Patient Low level */
#define CY_BLE_CGMS_GLMT_SSA_RH      (0x00020000u)  /* Sensor result higher than the Patient High level */
#define CY_BLE_CGMS_GLMT_SSA_HL      (0x00040000u)  /* Sensor result lower than the Hypo level */
#define CY_BLE_CGMS_GLMT_SSA_HH      (0x00080000u)  /* Sensor result higher than the Hyper level */
#define CY_BLE_CGMS_GLMT_SSA_SD      (0x00100000u)  /* Sensor Rate of Decrease exceeded */
#define CY_BLE_CGMS_GLMT_SSA_SI      (0x00200000u)  /* Sensor Rate of Increase exceeded */
#define CY_BLE_CGMS_GLMT_SSA_DL      (0x00400000u)  /* Sensor result lower than the device can process */
#define CY_BLE_CGMS_GLMT_SSA_DH      (0x00800000u)  /* Sensor result higher than the device can process */
#define CY_BLE_CGMS_GLMT_SSA_STM     (0x00FF0000u)  /* Status-Octet mask */
#define CY_BLE_CGMS_GLMT_SSA_STS     (16u)          /* Status-Octet shift */

/* Glucose Measurement Context characteristic "Flags" bit field flags */
#define CY_BLE_CGMS_CGMT_FLG_CH       (0x01u) /* Carbohydrate ID And Carbohydrate Present */
#define CY_BLE_CGMS_GLMT_FLG_MEAL     (0x02u) /* Meal Present */
#define CY_BLE_CGMS_GLMT_FLG_TH       (0x04u) /* Tester-Health Present */
#define CY_BLE_CGMS_GLMT_FLG_EXER     (0x08u) /* Exercise Duration And Exercise Intensity Present */
#define CY_BLE_CGMS_GLMT_FLG_MEDC     (0x10u) /* Medication ID And Medication Present */
#define CY_BLE_CGMS_GLMT_FLG_MVU      (0x20u) /* Medication Value Units 1 = liters, 0 = kilograms */
#define CY_BLE_CGMS_GLMT_FLG_HBA1C    (0x40u) /* HbA1c Present */
#define CY_BLE_CGMS_GLMT_FLG_EXFLAG   (0x80u) /* Extended Flags Present */

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



typedef enum
{
    CY_BLE_CGMS_GLMT_TYPE_CWB = 1u,   /* Capillary Whole blood */
    CY_BLE_CGMS_GLMT_TYPE_CPL,        /* Capillary Plasma */
    CY_BLE_CGMS_GLMT_TYPE_VWB,        /* Venous Whole blood */
    CY_BLE_CGMS_GLMT_TYPE_VPL,        /* Venous Plasma */
    CY_BLE_CGMS_GLMT_TYPE_AWB,        /* Arterial Whole blood */
    CY_BLE_CGMS_GLMT_TYPE_APL,        /* Arterial Plasma */
    CY_BLE_CGMS_GLMT_TYPE_UWB,        /* Undetermined Whole blood */
    CY_BLE_CGMS_GLMT_TYPE_UPL,        /* Undetermined Plasma */
    CY_BLE_CGMS_GLMT_TYPE_ISF,        /* Interstitial Fluid */
    CY_BLE_CGMS_GLMT_TYPE_CS          /* Control Solution */
}cy_stc_ble_cgms_cgmt_type_t;

typedef enum
{
    CY_BLE_CGMS_GLMT_SL_FR = 1u,      /* Finger */
    CY_BLE_CGMS_GLMT_SL_AST,          /* Alternate Site Test */
    CY_BLE_CGMS_GLMT_SL_EL,           /* Earlobe */
    CY_BLE_CGMS_GLMT_SL_CS,           /* Control Solution */
    CY_BLE_CGMS_GLMT_SL_NAV = 0x0Fu   /* Sample Location value not available */
}cy_en_ble_cgms_glmt_sl_t;

typedef struct
{
    uint8_t  flags;
    sfloat gluConc;     /* CGM Glucose Concentration */
    uint16_t timeOffset;  /* in minutes */
    uint32_t ssa;         /* Sensor Status Annunciation */
    sfloat trend;       /* CGM Trend Information */
    sfloat quality;     /* CGM Quality */
}cy_stc_ble_cgms_cgmt_t;


#define CY_BLE_CGMS_CGFT_FTR_CL (0x00000001u) /* Calibration Supported */
#define CY_BLE_CGMS_CGFT_FTR_PT (0x00000002u) /* Patient High/Low Alerts supported */
#define CY_BLE_CGMS_CGFT_FTR_HO (0x00000004u) /* Hypo Alerts supported */
#define CY_BLE_CGMS_CGFT_FTR_HR (0x00000008u) /* Hyper Alerts supported */
#define CY_BLE_CGMS_CGFT_FTR_ID (0x00000010u) /* Rate of Increase/Decrease Alerts supported */
#define CY_BLE_CGMS_CGFT_FTR_DS (0x00000020u) /* Device Specific Alert supported */
#define CY_BLE_CGMS_CGFT_FTR_SM (0x00000040u) /* Sensor Malfunction Detection supported */
#define CY_BLE_CGMS_CGFT_FTR_ST (0x00000080u) /* Sensor Temperature High-Low Detection supported */

#define CY_BLE_CGMS_CGFT_FTR_HL (0x00000100u) /* Sensor Result High-Low Detection supported */
#define CY_BLE_CGMS_CGFT_FTR_BL (0x00000200u) /* Low Battery Detection supported */
#define CY_BLE_CGMS_CGFT_FTR_TE (0x00000400u) /* Sensor Type Error Detection supported */
#define CY_BLE_CGMS_CGFT_FTR_DF (0x00000800u) /* General Device Fault supported */
#define CY_BLE_CGMS_CGFT_FTR_EC (0x00001000u) /* E2E-CRC supported */
#define CY_BLE_CGMS_CGFT_FTR_MB (0x00002000u) /* Multiple Bond supported */
#define CY_BLE_CGMS_CGFT_FTR_MS (0x00004000u) /* Multiple Sessions supported */
#define CY_BLE_CGMS_CGFT_FTR_TI (0x00008000u) /* CGM Trend Information supported */

#define CY_BLE_CGMS_CGFT_FTR_QA (0x00010000u) /* CGM Quality supported */

#define CY_BLE_CGMS_CGFT_TYPE_MASK    (0x0Fu) /* "Type" mask in the "tnsl" field */
#define CY_BLE_CGMS_CGFT_SL_MASK      (0xF0u) /* "Sample Location" mask in the "tnsl" field */
#define CY_BLE_CGMS_CGFT_SLNUM     (4u)    /* "Sample Location" offset in the "tnsl" field */


/* CGM Feature characteristic structure */
typedef struct
{
    uint32_t feature;     /* CGM Feature, actual size - 24 bit */
    uint8_t  type;        /* CGM Type, actual size - 4 bit */
    uint8_t  sampLoc;     /* CGM Sample Location, actual size - 4 bit */
}cy_stc_ble_cgms_cgft_t;

#define CY_BLE_SSTM_LEN (11u)

/* Time Zone */
typedef enum
{
    CY_BLE_TIME_ZONE_M1200 = -48, /* UTC-12:00 */
    CY_BLE_TIME_ZONE_M1100 = -44, /* UTC-11:00 */
    CY_BLE_TIME_ZONE_M1000 = -40, /* UTC-10:00 */
    CY_BLE_TIME_ZONE_M0930 = -38, /* UTC-9:30 */
    CY_BLE_TIME_ZONE_M0900 = -36, /* UTC-9:00 */
    CY_BLE_TIME_ZONE_M0800 = -32, /* UTC-8:00 */
    CY_BLE_TIME_ZONE_M0700 = -28, /* UTC-7:00 */
    CY_BLE_TIME_ZONE_M0600 = -24, /* UTC-6:00 */
    CY_BLE_TIME_ZONE_M0500 = -20, /* UTC-5:00 */
    CY_BLE_TIME_ZONE_M0430 = -18, /* UTC-4:30 */
    CY_BLE_TIME_ZONE_M0400 = -16, /* UTC-4:00 */
    CY_BLE_TIME_ZONE_M0330 = -14, /* UTC-3:30 */
    CY_BLE_TIME_ZONE_M0300 = -12, /* UTC-3:00 */
    CY_BLE_TIME_ZONE_M0200 = -8,  /* UTC-2:00 */
    CY_BLE_TIME_ZONE_M0100 = -4,  /* UTC-1:00 */
    CY_BLE_TIME_ZONE_ZERO  = 0,   /* UTC+0:00 */
    CY_BLE_TIME_ZONE_P0100 = 4,   /* UTC+1:00 */
    CY_BLE_TIME_ZONE_P0200 = 8,   /* UTC+2:00 */
    CY_BLE_TIME_ZONE_P0300 = 12,  /* UTC+3:00 */
    CY_BLE_TIME_ZONE_P0330 = 14,  /* UTC+3:30 */
    CY_BLE_TIME_ZONE_P0400 = 16,  /* UTC+4:00 */
    CY_BLE_TIME_ZONE_P0430 = 18,  /* UTC+4:30 */
    CY_BLE_TIME_ZONE_P0500 = 20,  /* UTC+5:00 */
    CY_BLE_TIME_ZONE_P0530 = 22,  /* UTC+5:30 */
    CY_BLE_TIME_ZONE_P0545 = 23,  /* UTC+5:45 */
    CY_BLE_TIME_ZONE_P0600 = 24,  /* UTC+6:00 */
    CY_BLE_TIME_ZONE_P0630 = 26,  /* UTC+6:30 */
    CY_BLE_TIME_ZONE_P0700 = 28,  /* UTC+7:00 */
    CY_BLE_TIME_ZONE_P0800 = 32,  /* UTC+8:00 */
    CY_BLE_TIME_ZONE_P0845 = 35,  /* UTC+8:45 */
    CY_BLE_TIME_ZONE_P0900 = 36,  /* UTC+9:00 */
    CY_BLE_TIME_ZONE_P0930 = 38,  /* UTC+9:30 */
    CY_BLE_TIME_ZONE_P1000 = 40,  /* UTC+10:00 */
    CY_BLE_TIME_ZONE_P1030 = 42,  /* UTC+10:30 */
    CY_BLE_TIME_ZONE_P1100 = 44,  /* UTC+11:00 */
    CY_BLE_TIME_ZONE_P1130 = 46,  /* UTC+11:30 */
    CY_BLE_TIME_ZONE_P1200 = 48,  /* UTC+12:00 */
    CY_BLE_TIME_ZONE_P1245 = 51,  /* UTC+12:45 */
    CY_BLE_TIME_ZONE_P1300 = 52,  /* UTC+13:00 */
    CY_BLE_TIME_ZONE_P1400 = 56   /* UTC+14:00 */
}cy_en_ble_time_zone_t;

#define CY_BLE_TIME_ZONE_VAL_NUM (40u)


/* DST Offset */
typedef enum
{
    CY_BLE_DSTOFFSET_ST = 0u,   /* Standard Time */
    CY_BLE_DSTOFFSET_HF = 2u,   /* Half An Hour Daylight Time (+0.5h) */
    CY_BLE_DSTOFFSET_DT = 4u,   /* Daylight Time (+1h) */
    CY_BLE_DSTOFFSET_DD = 8u,   /* Double Daylight Time (+2h) */
}cy_en_ble_dstoffset_t;

/* CGM Session Start Time characteristic structure */
typedef struct
{
    cy_stc_ble_date_time_t sst;       /* Session Start Time */
    cy_en_ble_time_zone_t timeZone;  /* Time Zone */
    cy_en_ble_dstoffset_t dstOffset; /* DST Offset */
}cy_stc_ble_cgms_sstm_t;


/* Opcode of Record Access Control Point characteristic value type */
typedef enum
{
    CY_BLE_CGMS_RACP_OPC_NA       = 0u,   /* Reserved for future use (Operator:N/A) */
    CY_BLE_CGMS_RACP_OPC_REPORT_REC,      /* Report stored records (Operator: Value from Operator Table) */
    CY_BLE_CGMS_RACP_OPC_DELETE_REC,      /* Delete stored records (Operator: Value from Operator Table) */
    CY_BLE_CGMS_RACP_OPC_ABORT_OPN,       /* Abort operation (Operator: Null 'value of 0x00 from Operator Table') */
    CY_BLE_CGMS_RACP_OPC_REPORT_NUM_REC,  /* Report number of stored records (Operator: Value from Operator Table) */
    CY_BLE_CGMS_RACP_OPC_NUM_REC_RSP,     /* Number of stored records response (Operator: Null 'value of 0x00 from Operator Table') */
    CY_BLE_CGMS_RACP_OPC_RSP_CODE,        /* Response Code (Operator: Null 'value of 0x00 from Operator Table') */
} cy_en_ble_cgms_racp_opc_t;

/* Operator of Record Access Control Point characteristic value type */
typedef enum
{
    CY_BLE_CGMS_RACP_OPR_NULL = 0u,  /* Null */
    CY_BLE_CGMS_RACP_OPR_ALL,        /* All records */
    CY_BLE_CGMS_RACP_OPR_LESS,       /* Less than or equal to */
    CY_BLE_CGMS_RACP_OPR_GREAT,      /* Greater than or equal to */
    CY_BLE_CGMS_RACP_OPR_WITHIN,     /* Within range of (inclusive) */
    CY_BLE_CGMS_RACP_OPR_FIRST,      /* First record(i.e. oldest record) */
    CY_BLE_CGMS_RACP_OPR_LAST        /* Last record (i.e. most recent record) */
} cy_en_ble_cgms_racp_opr_t;

/* Operand of Record Access Control Point characteristic value type */
typedef enum
{
    CY_BLE_CGMS_RACP_OPD_NA = 0u,     /* N/A */
    CY_BLE_CGMS_RACP_OPD_1,           /* Filter parameters (as appropriate to Operator and Service) */
    CY_BLE_CGMS_RACP_OPD_2,           /* Filter parameters (as appropriate to Operator and Service) */
    CY_BLE_CGMS_RACP_OPD_NO_INCL,     /* Not included */
    CY_BLE_CGMS_RACP_OPD_4,           /* Filter parameters (as appropriate to Operator and Service) */
    CY_BLE_CGMS_RACP_OPD_NUM_REC,     /* Number of Records (Field size defined per service) */
    CY_BLE_CGMS_RACP_OPD_RSP,         /* Request Op Code, Response Code Value */
    
} cy_en_ble_cgms_racp_opd_t;

/* Operand Response Code Values of Record Access Control Point characteristic value type */
typedef enum
{
    CY_BLE_CGMS_RACP_RSP_NA = 0u,     /* N/A */
    CY_BLE_CGMS_RACP_RSP_SUCCESS,     /* Normal response for successful operation */
    CY_BLE_CGMS_RACP_RSP_UNSPRT_OPC,  /* Normal response if unsupported Op Code is received */
    CY_BLE_CGMS_RACP_RSP_INV_OPR,     /* Normal response if Operator received does not meet 
                                     * requirements of service (e.g. Null was expected) 
                                     */
    CY_BLE_CGMS_RACP_RSP_UNSPRT_OPR,  /* Normal response if unsupported Operator is received */
    CY_BLE_CGMS_RACP_RSP_INV_OPD,     /* Normal response if Operand received does not meet 
                                     * requirements of service 
                                     */
    CY_BLE_CGMS_RACP_RSP_NO_REC,      /* Normal response if request to report stored records
                                     * or request to delete stored records resulted in no
                                     * records meeting criterion. 
                                     */
    CY_BLE_CGMS_RACP_RSP_UNSUCCESS,   /* Normal response if request for Abort cannot be completed */
    CY_BLE_CGMS_RACP_RSP_NO_COMPL,    /* Normal response if request for Abort cannot be completed */
    CY_BLE_CGMS_RACP_RSP_UNSPRT_OPD   /* Normal response if unsupported Operand is received */
    
} cy_en_ble_cgms_racp_rsp_t;


typedef struct
{
    sfloat hal;       /* High Alert Level */
    sfloat lal;       /* Low Alert Level */
    sfloat hpo;       /* Hypo Alert Level */
    sfloat hpr;       /* Hyper Alert Level */
    sfloat dec;       /* Rate of Decrease Alert Level */
    sfloat inc;       /* Rate of Increase Alert Level */
}cy_stc_ble_cgms_alrt_t;

/* Op Code - Response Codes of CGM Specific Ops Control Point characteristic value type */
typedef enum
{
    CY_BLE_CGMS_SOCP_RSP_RSRV       = 0u, /* Reserved for Future Use. N/A. */
    CY_BLE_CGMS_SOCP_RSP_SUCCESS    = 1u, /* Success. Normal response for successful operation. . */
    CY_BLE_CGMS_SOCP_RSP_UNSPRT_OPC = 2u, /* Op Code not supported. Normal response if unsupported Op Code is received.. */
    CY_BLE_CGMS_SOCP_RSP_INV_OPD    = 3u, /* Invalid Operand. Normal response if Operand received does not meet requirements of service.. */
    CY_BLE_CGMS_SOCP_RSP_NO_COMPL   = 4u, /* Procedure not completed. Normal response if unable to complete procedure for any reason.. */
    CY_BLE_CGMS_SOCP_RSP_POOR       = 5u  /* Parameter out of range. Normal response if Operand received does not meet range requirements . */
} cy_en_ble_cgms_socp_rsp_t;

/* Op Code of CGM Specific Ops Control Point characteristic value type */
typedef enum
{
    CY_BLE_CGMS_SOCP_OPC_NA   = 0u, /* Reserved for future use. N/A. */
    CY_BLE_CGMS_SOCP_OPC_SINT = 1u, /* Set CGM Communication Interval. Response to this control point is Response Code (Op Code 0x0F). */
    CY_BLE_CGMS_SOCP_OPC_GINT = 2u, /* Get CGM Communication Interval. Normal response to this control point is Op Code 0x03. For error conditions, response is defined in Op Code - Response Codes field. */
    CY_BLE_CGMS_SOCP_OPC_RINT = 3u, /* CGM Communication Interval response. This is normal response to Op Code 0x02. */
    CY_BLE_CGMS_SOCP_OPC_SGCV = 4u, /* Set Glucose Calibration Value. Response to this control point is defined in Op Code - Response Codes field. */
    CY_BLE_CGMS_SOCP_OPC_GGCV = 5u, /* Get Glucose Calibration Value. Normal response to this control point is Op Code 0x06. for error conditions, response is defined in Op Code - Response Codes field. */
    CY_BLE_CGMS_SOCP_OPC_RGCV = 6u, /* Glucose Calibration Value response. Glucose Calibration Value response. */
    CY_BLE_CGMS_SOCP_OPC_SHAL = 7u, /* Set Patient High Alert Level. Response to this control point is defined in Op Code - Response Codes field. */
    CY_BLE_CGMS_SOCP_OPC_GHAL = 8u, /* Get Patient High Alert Level. Normal response to this control point is Op Code 0x09. For error conditions, response is defined in Op Code - Response Codes field. */
    CY_BLE_CGMS_SOCP_OPC_RHAL = 9u, /* Patient High Alert Level Response. This is normal response to Op Code 0x08. */
    CY_BLE_CGMS_SOCP_OPC_SLAL = 10u, /* Set Patient Low Alert Level. Response to this control point is defined in Op Code - Response Codes field. */
    CY_BLE_CGMS_SOCP_OPC_GLAL = 11u, /* Get Patient Low Alert Level. Normal response to this control point is Op Code 0x0C. Response is defined in Op Code - Response Codes field. */
    CY_BLE_CGMS_SOCP_OPC_RLAL = 12u, /* Patient Low Alert Level Response. This is normal response to Op Code 0x0B. */
    CY_BLE_CGMS_SOCP_OPC_SHPO = 13u, /* Set Hypo Alert Level. Response to this control point is defined in Op Code - Response Codes field. */
    CY_BLE_CGMS_SOCP_OPC_GHPO = 14u, /* Get Hypo Alert Level. Normal response to this control point is Op Code 0x0F. Response is defined in Op Code - Response Codes field. */
    CY_BLE_CGMS_SOCP_OPC_RHPO = 15u, /* Hypo Alert Level Response. This is normal response to Op Code 0x0E. */
    CY_BLE_CGMS_SOCP_OPC_SHPR = 16u, /* Set Hyper Alert Level. Response to this control point is defined in Op Code - Response Codes field. */
    CY_BLE_CGMS_SOCP_OPC_GHPR = 17u, /* Get Hyper Alert Level. Normal response to this control point is Op Code 0x12. Response is defined in Op Code - Response Codes field. */
    CY_BLE_CGMS_SOCP_OPC_RHPR = 18u, /* Hyper Alert Level Response. This is normal response to Op Code 0x11. */
    CY_BLE_CGMS_SOCP_OPC_SDEC = 19u, /* Set Rate of Decrease Alert Level. Response to this control point is defined in Op Code - Response Codes field. */
    CY_BLE_CGMS_SOCP_OPC_GDEC = 20u, /* Get Rate of Decrease Alert Level. Normal response to this control point is Op Code 0x15. For error conditions, response is Response Code. */
    CY_BLE_CGMS_SOCP_OPC_RDEC = 21u, /* Rate of Decrease Alert Level Response. This is normal response to Op Code 0x14. */
    CY_BLE_CGMS_SOCP_OPC_SINC = 22u, /* Set Rate of Increase Alert Level. Response to this control point is defined in Op Code - Response Codes field. */
    CY_BLE_CGMS_SOCP_OPC_GINC = 23u, /* Get Rate of Increase Alert Level. Normal response to this control point is Op Code 0x18. For error conditions, response is Response Code. */
    CY_BLE_CGMS_SOCP_OPC_RINC = 24u, /* Rate of Increase Alert Level Response. This is normal response to Op Code 0x17. */
    CY_BLE_CGMS_SOCP_OPC_RDSA = 25u, /* Reset Device Specific Alert. Response to this control point is defined in Op Code - Response Codes field. */
    CY_BLE_CGMS_SOCP_OPC_STSN = 26u, /* Start the Session. Response to this control point is defined in Op Code - Response Codes field. */
    CY_BLE_CGMS_SOCP_OPC_SPSN = 27u, /* Stop the Session. Response to this control point is defined in Op Code - Response Codes field. */
    CY_BLE_CGMS_SOCP_OPC_RSPC = 28u  /* Response Code. see Op Code - Response Codes field. */
} cy_en_ble_cgms_socp_opc_t;

#define CY_BLE_CGMS_SOCP_OPC_SN (1u) /* Length of the Start/Stop Session SOCP commands. */
#define CY_BLE_CGMS_SOCP_OPC_IN (2u) /* Length of the Set CGM Communication Interval SOCP command. */
#define CY_BLE_CGMS_SOCP_OPC_AL (3u) /* Length of the Set Alert Level (Patient High/Low, Hypo, Hyper, Rate of Decrease/Increase) SOCP commands. */
#define CY_BLE_CGMS_SOCP_OPC_CV (11u) /* Length of the Set Glucose Calibration Value SOCP command. */


/***************************************
*      API function prototypes
***************************************/
uint16_t CgmsCrc(uint8_t length, uint8_t *dataPtr);
uint8_t CgmsCrcLength(uint8_t length, uint8_t* dataPtr);
cy_en_ble_gatt_err_code_t CgmsCrcCheck(uint16_t attrSize, cy_stc_ble_gatt_value_t *gattValue);
void CgmsPrintCharName(cy_en_ble_cgms_char_index_t charIndex);
void CgmsInit(void);
void CgmsCallBack(uint32_t event, void* eventParam);
void CgmsProcess(cy_stc_ble_conn_handle_t connHandle);
void CgmsSendCgmtNtf(cy_stc_ble_conn_handle_t connHandle, cy_stc_ble_cgms_cgmt_t cgmt);
void CgmsRacpOpCodeProcess(cy_stc_ble_conn_handle_t connHandle, uint8_t recNum);

#endif /* CGMSS_H  */

/* [] END OF FILE */
