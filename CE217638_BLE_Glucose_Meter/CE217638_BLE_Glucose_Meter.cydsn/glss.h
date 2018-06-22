/*******************************************************************************
* File Name: glss.h
*
* Version 1.0
*
* Description:
*  GLS service related code header.
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

#if !defined(GLSS_H)
#define GLSS_H

#include <project.h>

#define CY_BLE_GLS_REC_NUM           (11u) /* Number of records */
#define CY_BLE_GLS_REC_STAT_OK       (0u)
#define CY_BLE_GLS_REC_STAT_DELETED  (1u)

/* Glucose Measurement characteristic "Flags" bit field flags */
#define CY_BLE_GLS_GLMT_FLG_TOP (0x01u) /* Time Offset Present */
#define CY_BLE_GLS_GLMT_FLG_GLC (0x02u) /* Glucose Concentration, Type and Sample Location Present */
#define CY_BLE_GLS_GLMT_FLG_GCU (0x04u) /* Glucose Concentration Units, 1 = mol/L, 0 = kg/L */
#define CY_BLE_GLS_GLMT_FLG_SSA (0x08u) /* Sensor Status Annunciation Present */
#define CY_BLE_GLS_GLMT_FLG_CIF (0x10u) /* Context Information Follows */

/* Glucose Measurement characteristic "Sensor Status Annunciation" bit field flags */
#define CY_BLE_GLS_GLMT_SSA_BTL      (0x0001)  /* Device battery low at time of measurement */
#define CY_BLE_GLS_GLMT_SSA_SMF      (0x0002)  /* Sensor malfunction or faulting at time of measurement */
#define CY_BLE_GLS_GLMT_SSA_INSFNT   (0x0004)  /* Sample size for blood or control solution insufficient at time of measurement */
#define CY_BLE_GLS_GLMT_SSA_STRERR   (0x0008)  /* Strip insertion error */
#define CY_BLE_GLS_GLMT_SSA_STRTYP   (0x0010)  /* Strip type incorrect for device */
#define CY_BLE_GLS_GLMT_SSA_SRH      (0x0020)  /* Sensor result higher than the device can process */
#define CY_BLE_GLS_GLMT_SSA_SRL      (0x0040)  /* Sensor result lower than the device can process */
#define CY_BLE_GLS_GLMT_SSA_STH      (0x0080)  /* Sensor temperature too high for valid test/result at time of measurement */
#define CY_BLE_GLS_GLMT_SSA_STL      (0x0100)  /* Sensor temperature too low for valid test/result at time of measurement */
#define CY_BLE_GLS_GLMT_SSA_SRI      (0x0200)  /* Sensor read interrupted because strip was pulled too soon at time of measurement */
#define CY_BLE_GLS_GLMT_SSA_FLT      (0x0400)  /* General device fault has occurred in the sensor */
#define CY_BLE_GLS_GLMT_SSA_TMFLT    (0x0800)  /* Time fault has occurred in the sensor and time may be inaccurate */

#define CY_BLE_GLS_GLMT_TYPE_MASK    (0x0Fu) /* "Type" mask in the "tnsl" field */
#define CY_BLE_GLS_GLMT_SL_MASK      (0xF0u) /* "Sample Location" mask in the "tnsl" field */
#define CY_BLE_GLS_GLMT_SLNUM        (4u)    /* "Sample Location" offset in the "tnsl" field */

/* Glucose Measurement Context characteristic "Flags" bit field flags */
#define CY_BLE_GLS_GLMC_FLG_CH       (0x01u) /* Carbohydrate ID And Carbohydrate Present */
#define CY_BLE_GLS_GLMC_FLG_MEAL     (0x02u) /* Meal Present */
#define CY_BLE_GLS_GLMC_FLG_TH       (0x04u) /* Tester-Health Present */
#define CY_BLE_GLS_GLMC_FLG_EXER     (0x08u) /* Exercise Duration And Exercise Intensity Present */
#define CY_BLE_GLS_GLMC_FLG_MEDC     (0x10u) /* Medication ID And Medication Present */
#define CY_BLE_GLS_GLMC_FLG_MVU      (0x20u) /* Medication Value Units 1 = liters, 0 = kilograms */
#define CY_BLE_GLS_GLMC_FLG_HBA1C    (0x40u) /* HbA1c Present */
#define CY_BLE_GLS_GLMC_FLG_EXFLAG   (0x80u) /* Extended Flags Present */

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
    CY_BLE_GLS_GLMT_TYPE_CWB = 1u,   /* Capillary Whole blood */
    CY_BLE_GLS_GLMT_TYPE_CPL,        /* Capillary Plasma */
    CY_BLE_GLS_GLMT_TYPE_VWB,        /* Venous Whole blood */
    CY_BLE_GLS_GLMT_TYPE_VPL,        /* Venous Plasma */
    CY_BLE_GLS_GLMT_TYPE_AWB,        /* Arterial Whole blood */
    CY_BLE_GLS_GLMT_TYPE_APL,        /* Arterial Plasma */
    CY_BLE_GLS_GLMT_TYPE_UWB,        /* Undetermined Whole blood */
    CY_BLE_GLS_GLMT_TYPE_UPL,        /* Undetermined Plasma */
    CY_BLE_GLS_GLMT_TYPE_ISF,        /* Interstitial Fluid */
    CY_BLE_GLS_GLMT_TYPE_CS          /* Control Solution */
}cy_stc_ble_gls_glmt_type_t;

typedef enum
{
    CY_BLE_GLS_GLMT_SL_FR = 1u,      /* Finger */
    CY_BLE_GLS_GLMT_SL_AST,          /* Alternate Site Test */
    CY_BLE_GLS_GLMT_SL_EL,           /* Earlobe */
    CY_BLE_GLS_GLMT_SL_CS,           /* Control Solution */
    CY_BLE_GLS_GLMT_SL_NAV = 0x0Fu   /* Sample Location value not available */
}cy_en_ble_gls_glmt_sl_t;

typedef uint16_t sfloat;

typedef struct
{
    uint8_t  flags;
    uint16_t seqNum;
    cy_stc_ble_date_time_t baseTime;
    int16_t  timeOffset;  /* in minutes */
    sfloat gluConc;     /* Glucose Concentration, units depend on CY_BLE_GLS_GLMT_FLG_GLCU flag */
    uint8_t  tnsl;        /* "Type" & "Sample Location" two nibble-type fields in one byte,
                        *  "Type" is less significant.
                        */
    uint16_t ssa;         /* Sensor Status Annunciation */
}cy_stc_ble_gls_glmt_t;

#define CY_BLE_GLS_GLMC_FLG_CBID (0x01u) /* Carbohydrate ID And Carbohydrate Present */
#define CY_BLE_GLS_GLMC_FLG_MEAL (0x02u) /* Meal Present */
#define CY_BLE_GLS_GLMC_FLG_TNH  (0x04u) /* Tester-Health Present */
#define CY_BLE_GLS_GLMC_FLG_EXR  (0x08u) /* Exercise Duration And Exercise Intensity Present */
#define CY_BLE_GLS_GLMC_FLG_MED  (0x10u) /* Medication ID And Medication Present */
#define CY_BLE_GLS_GLMC_FLG_MDU  (0x20u) /* Medication Value Units 0 = kilograms, 1 = liters */
#define CY_BLE_GLS_GLMC_FLG_A1C  (0x40u) /* HbA1c Present */
#define CY_BLE_GLS_GLMC_FLG_EXT  (0x80u) /* Extended Flags Present */

#define CY_BLE_GLS_GLMC_TESTER_MASK  (0x0Fu) /* "Tester" mask in the "tnh" field */
#define CY_BLE_GLS_GLMC_HEALTH_MASK  (0xF0u) /* "Health" mask in the "tnh" field */
#define CY_BLE_GLS_GLMC_HEALTHNUM (4u)    /* "Health" offset in the "tnh" field */

typedef enum
{
    CY_BLE_GLS_GLMC_CBID_BREAKFAST = 1u, /* Breakfast */
    CY_BLE_GLS_GLMC_CBID_LUNCH,          /* Lunch */
    CY_BLE_GLS_GLMC_CBID_DINNER,         /* Dinner */
    CY_BLE_GLS_GLMC_CBID_SNACK,          /* Snack */
    CY_BLE_GLS_GLMC_CBID_DRINK,          /* Drink */
    CY_BLE_GLS_GLMC_CBID_SUPPER,         /* Supper */
    CY_BLE_GLS_GLMC_CBID_BRUNCH          /* Brunch */
}cy_en_ble_gls_glmc_cbid_t;

typedef enum
{
    CY_BLE_GLS_GLMC_MEAL_PRE = 1u,   /* Preprandial (before meal) */
    CY_BLE_GLS_GLMC_MEAL_POST,       /* Postprandial (after meal) */
    CY_BLE_GLS_GLMC_MEAL_FAST,       /* Fasting */
    CY_BLE_GLS_GLMC_MEAL_CAS,        /* Casual (snacks, drinks, etc.) */
    CY_BLE_GLS_GLMC_MEAL_BED         /* Bedtime */
}cy_en_ble_gls_glmc_meal_t;

typedef enum
{
    CY_BLE_GLS_GLMC_TESTER_SELF = 1u,    /* Self */
    CY_BLE_GLS_GLMC_TESTER_HCP,          /* Health Care Professional */
    CY_BLE_GLS_GLMC_TESTER_LAB,          /* Lab test */
    CY_BLE_GLS_GLMC_TESTER_NAV = 0x0Fu   /* Tester value not available */
}cy_stc_ble_gls_glmc_tester_t;

typedef enum
{
    CY_BLE_GLS_GLMC_HEALTH_MIN = 1u,     /* Minor health issues */
    CY_BLE_GLS_GLMC_HEALTH_MAJ,          /* Major health issues */
    CY_BLE_GLS_GLMC_HEALTH_DM,           /* During menses */
    CY_BLE_GLS_GLMC_HEALTH_US,           /* Under stress */
    CY_BLE_GLS_GLMC_HEALTH_NO,           /* No health issues */
    CY_BLE_GLS_GLMC_HEALTH_NAV = 0x0Fu   /* Health value not available */
}cy_en_ble_gls_glmc_health_t;

typedef enum
{
    CY_BLE_GLS_GLMC_MEDID_RAI = 1u, /* Rapid acting insulin */
    CY_BLE_GLS_GLMC_MEDID_SAI,      /* Short acting insulin */
    CY_BLE_GLS_GLMC_MEDID_IAI,      /* Intermediate acting insulin */
    CY_BLE_GLS_GLMC_MEDID_LAI,      /* Long acting insulin */
    CY_BLE_GLS_GLMC_MEDID_PMI,      /* Pre-mixed insulin */
}cy_en_ble_gls_glmc_medid_t;

typedef struct
{
    uint8_t  flags;
    uint16_t seqNum;
    uint8_t  exFlags;
    uint8_t  cbId;    /* Carbohydrate ID */
    sfloat cbhdr;   /* Carbohydrate */
    uint8_t  meal;
    uint8_t  tnh;     /* "Tester" & "Health" two nibble-type fields in one byte,
                     * "Tester" is less significant.
                     */
    uint16_t exDur;   /* Exercise Duration (in seconds) */
    uint8_t  exInt;   /* Exercise Intensity (in percents) */
    uint8_t  medId;   /* Medication ID */
    sfloat medic;   /* Medication */
    sfloat hba1c;   /* HbA1c (glycated hemoglobin) */
}cy_stc_ble_gls_glmc_t;

/* Glucose Feature characteristic value type */
typedef enum
{
    CY_BLE_GLS_GLFT_LOW_BATTERY      = 0x0001u,   /* Low Battery Detection During Measurement Supported */
    CY_BLE_GLS_GLFT_SNS_MLFNCTN      = 0x0002u,   /* Sensor Malfunction Detection Supported */
    CY_BLE_GLS_GLFT_SNS_SMPL_SZ      = 0x0004u,   /* Sensor Sample Size Supported */
    CY_BLE_GLS_GLFT_SNS_STRP_INS_ERR = 0x0008u,   /* Sensor Strip Insertion Error Detection Supported */
    CY_BLE_GLS_GLFT_SNS_STRP_TYP_ERR = 0x0010u,   /* Sensor Strip Type Error Detection Supported */
    CY_BLE_GLS_GLFT_SNS_RSLT_HI_LO   = 0x0020u,   /* Sensor Result High-Low Detection Supported */
    CY_BLE_GLS_GLFT_SNS_TEMP_HI_LO   = 0x0040u,   /* Sensor Temperature High-Low Detection Supported */
    CY_BLE_GLS_GLFT_SNS_READ_INT     = 0x0080u,   /* Sensor Read Interrupt Detection Supported */
    CY_BLE_GLS_GLFT_GNRL_DEV_FAULT   = 0x0100u,   /* General Device Fault Supported */
    CY_BLE_GLS_GLFT_TIME_FAULT       = 0x0200u,   /* Time Fault Supported */
    CY_BLE_GLS_GLFT_MLTP_BOND        = 0x0400u    /* Multiple Bond Supported */
} cy_en_ble_gls_glft_t;

/* Opcode of the Record Access Control Point characteristic value type */
typedef enum
{
    CY_BLE_GLS_RACP_OPC_RESERVED = 0u,   /* Reserved for future use (Operator: N/A) */
    CY_BLE_GLS_RACP_OPC_REPORT_REC,      /* Report stored records (Operator: Value from Operator Table) */
    CY_BLE_GLS_RACP_OPC_DELETE_REC,      /* Delete stored records (Operator: Value from Operator Table) */
    CY_BLE_GLS_RACP_OPC_ABORT_OPN,       /* Abort operation (Operator: Null 'value of 0x00 from Operator Table') */
    CY_BLE_GLS_RACP_OPC_REPORT_NUM_REC,  /* Report number of stored records (Operator: Value from Operator Table) */
    CY_BLE_GLS_RACP_OPC_NUM_REC_RSP,     /* Number of stored records response (Operator: Null 'value of 0x00 from Operator Table') */
    CY_BLE_GLS_RACP_OPC_RSP_CODE,        /* Response Code (Operator: Null 'value of 0x00 from Operator Table') */

} cy_en_ble_gls_racp_opc_t;

/* Operator of the Record Access Control Point characteristic value type */
typedef enum
{
    CY_BLE_GLS_RACP_OPR_NULL = 0u,  /* Null */
    CY_BLE_GLS_RACP_OPR_ALL,        /* All records */
    CY_BLE_GLS_RACP_OPR_LESS,       /* Less than or equal to */
    CY_BLE_GLS_RACP_OPR_GREAT,      /* Greater than or equal to */
    CY_BLE_GLS_RACP_OPR_WITHIN,     /* Within range of (inclusive) */
    CY_BLE_GLS_RACP_OPR_FIRST,      /* First record (i.e. oldest record) */
    CY_BLE_GLS_RACP_OPR_LAST        /* Last record (i.e. most recent record) */
} cy_en_ble_gls_racp_opr_t;

/* Operand of the Record Access Control Point characteristic value type */
typedef enum
{
    CY_BLE_GLS_RACP_OPD_NA = 0u,     /* N/A */
    CY_BLE_GLS_RACP_OPD_1,           /* Filter parameters (as appropriate to Operator and Service) */
    CY_BLE_GLS_RACP_OPD_2,           /* Filter parameters (as appropriate to Operator and Service) */
    CY_BLE_GLS_RACP_OPD_NO_INCL,     /* Not included */
    CY_BLE_GLS_RACP_OPD_4,           /* Filter parameters (as appropriate to Operator and Service) */
    CY_BLE_GLS_RACP_OPD_NUM_REC,     /* Number of Records (Field size defined per service) */
    CY_BLE_GLS_RACP_OPD_RSP,         /* Request Op Code, Response Code Value */

} cy_en_ble_gls_racp_opd_t;

/* Operand Response Code Values of the Record Access Control Point characteristic value type */
typedef enum
{
    CY_BLE_GLS_RACP_RSP_NA = 0u,     /* N/A */
    CY_BLE_GLS_RACP_RSP_SUCCESS,     /* Normal response for successful operation */
    CY_BLE_GLS_RACP_RSP_UNSPRT_OPC,  /* Normal response if unsupported Op Code is received */
    CY_BLE_GLS_RACP_RSP_INV_OPR,     /* Normal response if Operator received does not meet the
                                     * requirements of the service (e.g. Null was expected)
                                     */
    CY_BLE_GLS_RACP_RSP_UNSPRT_OPR,  /* Normal response if unsupported Operator is received */
    CY_BLE_GLS_RACP_RSP_INV_OPD,     /* Normal response if Operand received does not meet the
                                     * requirements of the service
                                     */
    CY_BLE_GLS_RACP_RSP_NO_REC,      /* Normal response if request to report stored records
                                     * or request to delete stored records resulted in no
                                     * records meeting criteria.
                                     */
    CY_BLE_GLS_RACP_RSP_UNSUCCESS,   /* Normal response if request for Abort cannot be completed */
    CY_BLE_GLS_RACP_RSP_NO_COMPL,    /* Normal response if request for Abort cannot be completed */
    CY_BLE_GLS_RACP_RSP_UNSPRT_OPD   /* Normal response if unsupported Operand is received */

} cy_en_ble_gls_racp_rsp_t;



/***************************************
*      Function prototypes
***************************************/
void GlsInit(void);
void GlsCallBack(uint32_t event, void* eventParam);
void GlsProcess(cy_stc_ble_conn_handle_t connHandle);

/* Internal functions */
void GlsNtf(cy_stc_ble_conn_handle_t connHandle, uint8_t num);
void GlsInd(cy_stc_ble_conn_handle_t connHandle);
cy_en_ble_date_time_comp_t TimeComparison(cy_stc_ble_date_time_t glmtTime, cy_stc_ble_date_time_t userFacingTime);
void OpCodeOperation(cy_stc_ble_conn_handle_t connHandle, uint8_t i);

/***************************************
*      External data references
***************************************/

extern cy_stc_ble_gls_glmt_t glsGlucose[CY_BLE_GLS_REC_NUM];
extern cy_stc_ble_gls_glmc_t glsGluCont[CY_BLE_GLS_REC_NUM];


#endif /* GLSS_H  */

/* [] END OF FILE */
