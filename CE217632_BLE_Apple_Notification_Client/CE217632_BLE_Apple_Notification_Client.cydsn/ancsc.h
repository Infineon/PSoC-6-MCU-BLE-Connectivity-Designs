/*******************************************************************************
* File Name: ancsc.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project.
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>

#if !defined(ANCSC_H)
#define ANCSC_H

    
#define CY_BLE_ANCS_MAX_STR_LENGTH (300u)
    
#define CY_BLE_ANCS_FLG_START      (0x01u) /* Start ANCS operation */
#define CY_BLE_ANCS_FLG_CMD        (0x02u) /* Command */
#define CY_BLE_ANCS_FLG_NTF        (0x04u) /* Notification */
#define CY_BLE_ANCS_FLG_STR        (0x08u) /* String */
#define CY_BLE_ANCS_FLG_ACT        (0x10u) /* Action */
#define CY_BLE_ANCS_FLG_RSP        (0x20u) /* Response */

#define CY_BLE_ANCS_NS_CNT         (10u)
    
#define CY_BLE_ANCS_NS_FLG_SL      (0x01u) /* Silent */
#define CY_BLE_ANCS_NS_FLG_IM      (0x02u) /* Important */
#define CY_BLE_ANCS_NS_FLG_PE      (0x04u) /* Pre-existing */
#define CY_BLE_ANCS_NS_FLG_PA      (0x08u) /* Positive Action */
#define CY_BLE_ANCS_NS_FLG_NA      (0x10u) /* Negative Action */
    
typedef enum
{
    CY_BLE_ANCS_NS_EVT_ID_ADD,             /* Notification Added */
    CY_BLE_ANCS_NS_EVT_ID_MOD,             /* Notification Modified */
    CY_BLE_ANCS_NS_EVT_ID_REM              /* Notification Removed */
}cy_en_ble_ancs_ns_evt_id_t;

typedef enum
{
    CY_BLE_ANCS_NS_CAT_ID_OTH,             /* Other */
    CY_BLE_ANCS_NS_CAT_ID_INC,             /* Incoming Call */
    CY_BLE_ANCS_NS_CAT_ID_MIS,             /* Missed Call */
    CY_BLE_ANCS_NS_CAT_ID_VML,             /* Voice mail */
    CY_BLE_ANCS_NS_CAT_ID_SOC,             /* Social */
    CY_BLE_ANCS_NS_CAT_ID_SCH,             /* Schedule */
    CY_BLE_ANCS_NS_CAT_ID_EML,             /* Email */
    CY_BLE_ANCS_NS_CAT_ID_NWS,             /* News */
    CY_BLE_ANCS_NS_CAT_ID_HNF,             /* Health and Fitness */
    CY_BLE_ANCS_NS_CAT_ID_BNF,             /* Business and Finance */
    CY_BLE_ANCS_NS_CAT_ID_LOC,             /* Location */
    CY_BLE_ANCS_NS_CAT_ID_ENT              /* Entertainment */
}cy_en_ble_ancs_ns_cat_id_t;

typedef enum
{
    CY_BLE_ANCS_CP_CMD_ID_GNA,              /* Get Notification Attributes */
    CY_BLE_ANCS_CP_CMD_ID_GAA,              /* Get App Attributes */
    CY_BLE_ANCS_CP_CMD_ID_PNA               /* Perform Notification Action */
}cy_en_ble_ancs_cp_cmd_id_t;

typedef enum
{
    CY_BLE_ANCS_CP_ACT_ID_POS,              /* Positive */
    CY_BLE_ANCS_CP_ACT_ID_NEG               /* Negative */
}cy_en_ble_ancs_cp_act_id_t;

typedef enum
{
    CY_BLE_ANCS_CP_ATT_ID_AID,              /* App identifier */
    CY_BLE_ANCS_CP_ATT_ID_TTL,              /* Title */
    CY_BLE_ANCS_CP_ATT_ID_SBT,              /* Subtitle */
    CY_BLE_ANCS_CP_ATT_ID_MSG,              /* Message */
    CY_BLE_ANCS_CP_ATT_ID_MSZ,              /* Message size */
    CY_BLE_ANCS_CP_ATT_ID_DAT,              /* Date */
    CY_BLE_ANCS_CP_ATT_ID_PAL,              /* Positive action label */
    CY_BLE_ANCS_CP_ATT_ID_NAL               /* Negative action label */
} cy_en_ble_ancs_cp_att_id_t;

typedef struct
{
    cy_en_ble_ancs_ns_evt_id_t evtId;       /* Event ID */
    uint8_t                  evtFlg;        /* Event Flags */
    cy_en_ble_ancs_ns_cat_id_t ctgId;       /* Category ID */
    uint8_t                  ctgCnt;        /* Category Count */
    uint32_t                 ntfUid;        /* Notification UID */
}cy_stc_ble_ancs_ns_t;                      /* Notification Source characteristic structure */

typedef struct
{
    cy_en_ble_ancs_cp_cmd_id_t cmdId;       /* Command ID */
    uint32_t                 ntfUid;        /* Notification UID */
    uint32_t                 appId;         /* App Identifier */
    cy_en_ble_ancs_cp_att_id_t attId;       /* Attribute ID */
    cy_en_ble_ancs_cp_act_id_t actId;       /* Action ID */
    cy_en_ble_ancs_ns_cat_id_t ctgId;       /* Auxiliary field to store Category ID */
}cy_stc_ble_ancs_cp_t;                      /* Control Point characteristic structure */
                                            
typedef struct                              
{                                           
    cy_en_ble_ancs_cp_cmd_id_t cmdId;       /* Command ID */
    uint32_t                 ntfUid;        /* Notification UID */
    cy_en_ble_ancs_cp_att_id_t attId;       /* Attribute ID */
    uint16_t                 length;        /* Length of data */
    uint8_t*                   data;        /* Pointer to data buffer */
    uint16_t                 currLength;    /* Auxiliary field to store current length */
}cy_stc_ble_ancs_ds_t;                      /* Data Source characteristic structure */


/***************************************
*      External Function Prototypes
***************************************/

void AncsPrintCharName(cy_en_ble_ancs_char_index_t charIndex);
void AncsCallBack(uint32_t event, void* eventParam);
void AncsProcess(void);


/***************************************
*      External data references
***************************************/

extern uint8_t ancsFlag;


#endif /* ANCSC_H */

/* [] END OF FILE */
