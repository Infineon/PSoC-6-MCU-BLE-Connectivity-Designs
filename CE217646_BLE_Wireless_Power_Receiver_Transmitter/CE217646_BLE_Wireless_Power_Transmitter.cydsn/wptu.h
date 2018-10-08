/*******************************************************************************
* File Name: wptu.h
*
*  Version: 1.0
*
* Description:
*  Contains the function prototypes, constants and structure definitions 
*  available to the example project.
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


#if !defined(CY_BLE_WPTU_H)
#define CY_BLE_WPTU_H

    
#include <project.h>
#include "common.h"


/***************************************
*      Data Struct Definition 
***************************************/

typedef struct
{
    uint8_t enables;                      /* PTU turn on, PTU on indication etc. */
    uint8_t permission;                   /* PRU is permitted in PTU. */
    uint8_t timeSet;                      /* PTU sets up time. Units: ms */
    uint8_t rfu1;                         /* Undefined */
    uint8_t rfu2;                         /* Undefined */
}__PACKED cy_stc_ble_pru_control_t;

typedef struct
{
    uint8_t flags;                        /* Defines which fields are valid. */
    uint8_t ptuPower;                     /* Power of PTU. */
    uint8_t ptuMaxSourceImpedance;        /* Maximum source impedance of the PTU */
    uint8_t ptuMaxLoadResistance;         /* Maximum load resistance of the PTU */             
    uint8_t rfu1[2u];                     /* Undefined */
    uint8_t ptuClass;                     /* PTU class 1-5 */
    uint8_t hardwareRev;                  /* Revision of the PTU HW */
    uint8_t firmwareRev;                  /* Revision of the PTU SW */
    uint8_t protocolRev;                  /* A4WP Supported Revision */
    uint8_t ptuDevNumber;                 /* Max Number of Devices */
    uint8_t rfu2[6u];                     /* Undefined */
}__PACKED cy_stc_ble_ptu_static_par_t;


typedef struct
{
    uint8_t flags;                        /* Defines which optional fields are populated */
    uint8_t protocolRev;                  /* A4WP Supported Revision */
    uint8_t rfu1;                         /* Undefined */
    uint8_t pruCategory;                  /* Category of PRU */
    uint8_t pruInformation;               /* Capabilities of PRU (bit field) */             
    uint8_t hardwareRev;                  /* Revision of the PTU HW */
    uint8_t firmwareRev;                  /* Revision of the PTU SW */
    uint8_t pRectMax;                     /* PRECT_MAX of the PRU, units: mW*100 */
    uint16_t vRectMinStatic;              /* VRECT_MIN (static, first estimate), units: mV */
    uint16_t vRectHighStatic;             /* VRECT_HIGH (static, first estimate), units: mV */
    uint16_t vRectSet;                    /* VRECT_SET, units: mV */
    uint16_t deltaR1;                     /* Delta R1 caused by PRU, units: 0.01 ohms */
    uint8_t rfu2[4u];                      /* Undefined */
}__PACKED cy_stc_ble_pru_static_par_t;

/* PRU Dynamic Parameter Characteristic structure */
typedef struct
{
    uint8_t flags;                        /* Defines which optional fields are populated */
    uint16_t vRect;                       /* DC voltage at the output of the rectifier, units: mV  */
    uint16_t iRect;                       /* DC current at the output of the rectifier, units: mA  */
    uint16_t vOut;                        /* Voltage at charge/battery port, units: mV  */
    uint16_t iOut;                        /* Current at charge/battery port, units: mA  */
    uint8_t temperature;                  /* Temperature of PRU, units: Deg C from -40C  */
    uint16_t vRectMinDyn;                 /* The current dynamic minimum rectifier voltage desired, units: mV */
    uint16_t vRectSetDyn;                 /* Desired VRECT (dynamic value), units: mV */
    uint16_t vRectHighDyn;                /* The current dynamic maximum rectifier voltage desired, units: mV */
    uint8_t alert;                        /* Warnings, units: Bit field */
    uint8_t rfu[3u];                      /* Undefined */
}__PACKED cy_stc_ble_pru_dynamic_par_t;

/* PRU Advertising service data structure */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t wptsServiceHandle;  /* WPTS GATT Primary Service Handle */
    uint8_t rssi;                                    /* PRU RSSI Parameters  */
    uint8_t flags;                                   /* ADV Flags  */
}__PACKED cy_stc_ble_pru_adv_service_data_t;

typedef struct
{
    cy_stc_ble_gap_bd_addr_t peerAddr;
    cy_stc_ble_pru_adv_service_data_t peerAdvServData;
}cy_stc_ble_peer_device_info_t;

/***************************************
*          Constants
***************************************/

#define PRU_ALERT_SIMULATION_DISABLE                        (0u)
#define PRU_ALERT_NOTIFICATION_ENABLE                       (1u)
#define PRU_ALERT_INDICATION_ENABLE                         (2u)

#define PRU_CONTROL_ENABLES_ENABLE_PRU_OUTPUT               (0x80u)
#define PRU_CONTROL_ENABLES_ENABLE_CHARGE_INDICATOR         (0x40u)
#define PRU_CONTROL_ENABLES_ADJUST_POWER_MASK               (0x30u)
#define PRU_CONTROL_ENABLES_ADJUST_POWER_MAX                (0x00u)
#define PRU_CONTROL_ENABLES_ADJUST_POWER_66                 (0x10u)
#define PRU_CONTROL_ENABLES_ADJUST_POWER_33                 (0x20u)
#define PRU_CONTROL_ENABLES_ADJUST_POWER_2_5                (0x30u)

#define PRU_CONTROL_PERMISSION_PERMITTED                    (0x00u)
#define PRU_CONTROL_PERMISSION_PERMITTED_WITH_WARNING       (0x01u)
#define PRU_CONTROL_PERMISSION_DENIED_FLAG                  (0x80u)
#define PRU_CONTROL_PERMISSION_DENIED_LIMITED_POWER         (0x81u)
#define PRU_CONTROL_PERMISSION_DENIED_LIMITED_POWER         (0x81u)
#define PRU_CONTROL_PERMISSION_DENIED_LIMITED_DEVICES       (0x82u)
#define PRU_CONTROL_PERMISSION_DENIED_LIMITED_CLASS         (0x83u)

#define PRU_CONTROL_TIME_SET_STEP_MS                        (10u)
#define PRU_CONTROL_TIME_SET_STEP_MIN                       (1u)
#define PRU_CONTROL_TIME_SET_STEP_MAX                       (8u)

#define PTU_STATIC_PAR_FLAGS_MAX_IMPEDANCE_EN               (0x80u)
#define PTU_STATIC_PAR_FLAGS_MAX_RESISTANCE_EN              (0x40u)

#define PTU_STATIC_PAR_POWER_RANGE                          (20u)
#define PTU_STATIC_PAR_POWER_STEP                           (100u)
#define PTU_STATIC_PAR_POWER_HI_RANGE                       (120u)
#define PTU_STATIC_PAR_POWER_HI_STEP                        (1000u)

#define PTU_STATIC_PAR_MAX_SOURCE_IMPEDANCE_SHIFT           (3u)

#define PTU_STATIC_PAR_MAX_LOAD_RESISTANCE_SHIFT            (3u)
#define PTU_STATIC_PAR_MAX_LOAD_RESISTANCE_OFFSET           (1u)
#define PTU_STATIC_PAR_MAX_LOAD_RESISTANCE_STEP             (5u)

#define PTU_STATIC_PAR_NUMBER_OF_DEVICES_OFFSET             (1u)

#define PTU_STATIC_PAR_CLASS_OFFSET                         (1u)

#define PRU_STATIC_PAR_FLAGS_ENABLE_DELTA_R1                (0x80u)
#define PRU_STATIC_PAR_PREACT_MAX_MULT                      (100u)      /* The value is in increments of 100 mW */
#define PRU_STATIC_PAR_DELTA_R1_MULT                        (0.01f)      /* The value is in increments of 0.01 ohms */

#define PRU_DYNAMIC_PAR_FLAGS_VOUT_EN                       (0x80u)
#define PRU_DYNAMIC_PAR_FLAGS_IOUT_EN                       (0x40u)
#define PRU_DYNAMIC_PAR_FLAGS_TEMPERATURE_EN                (0x20u)
#define PRU_DYNAMIC_PAR_FLAGS_VREACT_MIN_EN                 (0x10u)
#define PRU_DYNAMIC_PAR_FLAGS_VREACT_SET_EN                 (0x08u)
#define PRU_DYNAMIC_PAR_FLAGS_VREACT_HIGH_EN                (0x04u)

#define PRU_DYNAMIC_PAR_TEMPERATURE_OFFSET                  (40)        /* -40 degree */

#define PRU_ALERT_OVER_VOLTAGE                              (0x80)
#define PRU_ALERT_OVER_CURRENT                              (0x40)
#define PRU_ALERT_OVER_TEMP                                 (0x20)
#define PRU_ALERT_SELF_PROTECTION                           (0x10)
#define PRU_ALERT_CHARGE_COMPLETE                           (0x08)
#define PRU_ALERT_WIRED_CHARGER_DETECT                      (0x04)
#define PRU_ALERT_DYNAMIC_CHARGE_PORT                       (0x02)
#define PRU_ALERT_MODE_TRANSITION_MASK                      (0x03)
#define PRU_ALERT_MODE_TRANSITION_2S                        (0x01)
#define PRU_ALERT_MODE_TRANSITION_3S                        (0x02)
#define PRU_ALERT_MODE_TRANSITION_6S                        (0x03)

#define PRU_ADV_SERV_DATA_LEN                               (0x07)
#define PRU_ADV_SERV_DATA_TYPE_OFFSET                       (0x01)
#define PRU_ADV_SERV_DATA_SERV_OFFSET                       (0x02)
#define PRU_ADV_SERV_DATA_HANDLE_OFFSET                     (0x04)
#define PRU_ADV_SERV_DATA_RSSI_OFFSET                       (0x06)
#define PRU_ADV_SERV_DATA_FLAGS_OFFSET                      (0x07)

#define PRU_STATIC_PAR_POWER_DEF                            (84u)       /* Class 4, 22W (value = 84) */
#define PRU_STATIC_PAR_CLASS_DEF                            (4u)       
#define PRU_STATIC_PAR_IMPEDANCE_DEF                        (5u)        /* 100 ohms (value = 5)*/  
#define PRU_STATIC_PAR_RESISTANCE_DEF                       (9u)        /* 50 ohms (value = 9)*/ 
#define PRU_STATIC_PAR_HARDW_REV_DEF                        (62u)      
#define PRU_STATIC_PAR_FW_REV_DEF                           (32u)      
#define PRU_STATIC_PAR_PROTOCOL_REV_DEF                     (0u)      

/***************************************
*       Function Prototypes
***************************************/
void WptsInit(void);
void WptsCallBack(uint32_t event, void *eventParam);
uint32_t WptsScanProcessEventHandler(cy_stc_ble_gapc_adv_report_param_t *eventParam, 
                                     cy_stc_ble_pru_adv_service_data_t *serviceData);


#endif /* CY_BLE_WPTU_H  */


/* [] END OF FILE */
