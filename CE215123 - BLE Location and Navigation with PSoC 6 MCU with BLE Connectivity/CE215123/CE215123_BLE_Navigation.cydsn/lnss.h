/*******************************************************************************
* File Name: lnss.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project.
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(LNSS_H)
#define LNSS_H

#include "common.h"

#define NV_EN               (0x04u)
#define INV_PAR             (0x08u)
#define LNS_COUNT           (0x0004u)
#define LNS_TIME            (2u)            /* Counts of 0.5 second */
    
/* L&N Feature characteristic flags */
#define CY_BLE_LNS_FT_IS     (0x00000001u)   /* Instantaneous Speed Supported */
#define CY_BLE_LNS_FT_TD     (0x00000002u)   /* Total Distance Supported */
#define CY_BLE_LNS_FT_LC     (0x00000004u)   /* Location Supported */
#define CY_BLE_LNS_FT_EL     (0x00000008u)   /* Elevation Supported */
#define CY_BLE_LNS_FT_HD     (0x00000010u)   /* Heading Supported */
#define CY_BLE_LNS_FT_RT     (0x00000020u)   /* Rolling Time Supported */
#define CY_BLE_LNS_FT_UTC    (0x00000040u)   /* UTC Time Supported */
#define CY_BLE_LNS_FT_RD     (0x00000080u)   /* Remaining Distance Supported */
#define CY_BLE_LNS_FT_RVD    (0x00000100u)   /* Remaining Vertical Distance Supported */
#define CY_BLE_LNS_FT_ETA    (0x00000200u)   /* Estimated Time of Arrival Supported */
#define CY_BLE_LNS_FT_NBS    (0x00000400u)   /* Number of Beacons in Solution Supported */
#define CY_BLE_LNS_FT_NBV    (0x00000800u)   /* Number of Beacons in View Supported */
#define CY_BLE_LNS_FT_TFF    (0x00001000u)   /* Time to First Fix Supported */
#define CY_BLE_LNS_FT_EHE    (0x00002000u)   /* Estimated Horizontal Position Error Supported */
#define CY_BLE_LNS_FT_EVE    (0x00004000u)   /* Estimated Vertical Position Error Supported */
#define CY_BLE_LNS_FT_HDP    (0x00008000u)   /* Horizontal Dilution of Precision Supported */
#define CY_BLE_LNS_FT_VDP    (0x00010000u)   /* Vertical Dilution of Precision Supported */
#define CY_BLE_LNS_FT_LSM    (0x00020000u)   /* Location and Speed Characteristic Content Masking Supported */
#define CY_BLE_LNS_FT_FRS    (0x00040000u)   /* Fix Rate Setting Supported */
#define CY_BLE_LNS_FT_ES     (0x00080000u)   /* Elevation Setting Supported */
#define CY_BLE_LNS_FT_PS     (0x00100000u)   /* Position Status Supported */

/* Location and Speed characteristic "flags" */
#define CY_BLE_LNS_LS_FLG_IS     (0x0001u)   /* Instantaneous Speed Present */
#define CY_BLE_LNS_LS_FLG_TD     (0x0002u)   /* Total Distance Present */
#define CY_BLE_LNS_LS_FLG_LC     (0x0004u)   /* Location Present */
#define CY_BLE_LNS_LS_FLG_EL     (0x0008u)   /* Elevation Present */
#define CY_BLE_LNS_LS_FLG_HD     (0x0010u)   /* Heading Present */
#define CY_BLE_LNS_LS_FLG_RT     (0x0020u)   /* Rolling Time Present */
#define CY_BLE_LNS_LS_FLG_UTC    (0x0040u)   /* UTC Time Present */
#define CY_BLE_LNS_LS_FLG_PS     (0x0180u)   /* Position Status mask */
#define CY_BLE_LNS_LS_FLG_PS_NP  (0x0000u)   /* Position Status: No Position */
#define CY_BLE_LNS_LS_FLG_PS_OK  (0x0080u)   /* Position Status: Position Ok */
#define CY_BLE_LNS_LS_FLG_PS_EP  (0x0100u)   /* Position Status: Estimated Position */
#define CY_BLE_LNS_LS_FLG_PS_LK  (0x0180u)   /* Position Status: Last Known Position */
#define CY_BLE_LNS_LS_FLG_SDF    (0x0200u)   /* Speed and Distance format */
#define CY_BLE_LNS_LS_FLG_ES     (0x0400u)   /* Elevation Source */
#define CY_BLE_LNS_LS_FLG_HS     (0x0800u)   /* Heading Source */

typedef struct
{
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hours;
    uint8_t  minutes;
    uint8_t  seconds;
}cy_stc_ble_date_time_t;

typedef struct
{
    uint16_t flags;
    uint16_t instSpd;     /* Instantaneous Speed */
    uint32_t totalDst;    /* Total Distance */
    int32_t  latitude;    /* Location - Latitude */
    int32_t  longitude;   /* Location - Longitude */
    int32_t  elevation;   /* Elevation */
    uint16_t heading;     /* Heading */
    uint8_t  rollTime;    /* Rolling Time */
    cy_stc_ble_date_time_t utcTime; /* UTC Time */
}cy_stc_ble_lns_ls_t;

/* Opcode of Record Access Control Point characteristic value type */
typedef enum
{
    CY_BLE_LNS_CP_OPC_SCV = 1u,  /* Set Cumulative Value (Parameter: Cumulative Value) */
    CY_BLE_LNS_CP_OPC_MLS,       /* Mask Location and Speed Characteristic Content (Parameter: Content Mask) */
    CY_BLE_LNS_CP_OPC_NC,        /* Navigation Control (Parameter: Control value) */
    CY_BLE_LNS_CP_OPC_NRS,       /* Request Number of Routes (Parameter: None) */
    CY_BLE_LNS_CP_OPC_RNM,       /* Request Name of Route (Parameter: Route Number) */
    CY_BLE_LNS_CP_OPC_SR,        /* Select Route (Parameter: Route Number) */
    CY_BLE_LNS_CP_OPC_SFR,       /* Set Fix Rate (Parameter: Fix Rate) */
    CY_BLE_LNS_CP_OPC_SE,        /* Set Elevation (Parameter: Elevation) */
    CY_BLE_LNS_CP_OPC_RC = 32u,  /* Response Code */
} cy_en_ble_lns_cp_opc_t;

/* Navigation Control OpCode parameter value type */
typedef enum
{
    CY_BLE_LNS_CP_OPC_NC_STOP,   /* Stop Notification of Navigation characteristic. Stop Navigation */
    CY_BLE_LNS_CP_OPC_NC_START,  /* Start Notification of Navigation characteristic. 
                                 * Start Navigation to first waypoint on route 
                                 */
    CY_BLE_LNS_CP_OPC_NC_PAUSE,  /* Stop Notification of Navigation characteristic. 
                                 * Pause Navigation keeping next waypoint on route 
                                 * in the memory for continuing navigation later 
                                 */
    CY_BLE_LNS_CP_OPC_NC_RESUME, /* Start Notification of Navigation characteristic. 
                                 * Continue Navigation from point where navigation 
                                 * was paused to next waypoint on route 
                                 */
    CY_BLE_LNS_CP_OPC_NC_SKIP,   /* Notification of Navigation characteristic not affected. 
                                 * Skip Way point: disregard next waypoint and continue navigation 
                                 * to waypoint following next waypoint on route
                                 */
    CY_BLE_LNS_CP_OPC_NC_NEAR    /* Start Notification of Navigation characteristic. 
                                 * Select Nearest Waypoint on Route: measure distance to all
                                 * waypoints on route, and start navigation to closest or 
                                 * optimal waypoint on route (left to implementation) and 
                                 * from there to waypoints following next waypoint along route 
                                 */
} cy_en_ble_lns_cp_opc_nc_t;

typedef enum
{
    CY_BLE_LNS_CP_RSP_SUCCESS = 1u,  /* Response for successful operation */
    CY_BLE_LNS_CP_RSP_UNSPRT_OPC,    /* Response if unsupported Op Code is received */
    CY_BLE_LNS_CP_RSP_INV_PAR,       /* Response if Parameter received does not meet requirements
                                     * of service or is outside of supported range of Sensor.
                                     */
    CY_BLE_LNS_CP_RSP_FAIL           /* Operation Failed */
} cy_en_ble_lns_cp_rsp_t;

/* Navigation characteristic "flags" */
#define CY_BLE_LNS_NV_FLG_RD     (0x0001u)   /* Remaining Distance Present */
#define CY_BLE_LNS_NV_FLG_RVD    (0x0002u)   /* Remaining Vertical Distance Present */
#define CY_BLE_LNS_NV_FLG_EAT    (0x0004u)   /* Estimated Time of Arrival Present */
#define CY_BLE_LNS_NV_FLG_PS     (0x0018u)   /* Position Status mask */
#define CY_BLE_LNS_NV_FLG_PS_NP  (0x0000u)   /* Position Status: No Position */
#define CY_BLE_LNS_NV_FLG_PS_OK  (0x0008u)   /* Position Status: Position Ok */
#define CY_BLE_LNS_NV_FLG_PS_EP  (0x0010u)   /* Position Status: Estimated Position */
#define CY_BLE_LNS_NV_FLG_PS_LK  (0x0018u)   /* Position Status: Last Known Position */
#define CY_BLE_LNS_NV_FLG_RT     (0x0020u)   /* Heading Source */
#define CY_BLE_LNS_NV_FLG_UTC    (0x0040u)   /* Navigation Indicator Type */
#define CY_BLE_LNS_NV_FLG_SDF    (0x0080u)   /* Waypoint Reached */
#define CY_BLE_LNS_NV_FLG_ES     (0x0100u)   /* Destination Reached */

typedef struct
{
    uint16_t flags;
    uint16_t bearing;     /* Bearing */
    uint16_t heading;     /* Heading */
    uint32_t rDst;        /* Remaining Distance */
    int32_t  rvDst;       /* Remaining Vertical Distance */
    cy_stc_ble_date_time_t eaTime; /* Estimated Time of Arrival */
}cy_stc_ble_lns_nv_t;


/***************************************
*      Function prototypes
***************************************/
void LnsInit(void);
void LnsCallBack(uint32_t event, void* eventParam);
void LnsProcess(cy_stc_ble_conn_handle_t connHandle);
void LnsNtf(cy_stc_ble_conn_handle_t connHandle);
uint8_t LnsGetFlag(void);

#endif /* LNSS_H  */

/* [] END OF FILE */
