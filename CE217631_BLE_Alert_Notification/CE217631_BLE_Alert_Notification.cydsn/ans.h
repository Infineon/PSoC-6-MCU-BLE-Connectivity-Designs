/*******************************************************************************
* File Name: ans.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants used by Alert Notification
*  Service.
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(ANS_H)
#define ANS_H

#include <project.h>
    
#if CY_BLE_HOST_CORE    
    
/***************************************
*        API Constants
***************************************/
/* Alert Notification Control Point Characteristic indexes */
#define ANCP_COMMAND_ID_INDEX               (0u)
#define ANCP_CATEGORY_ID_INDEX              (1u)

/* Supported Client Categories */
#define ANS_CLIENT_SUPPORTED_CATEGORIES     ((((uint16_t) 1u) << CY_BLE_ANS_CAT_ID_EMAIL) | \
                                            (((uint16_t) 1u) << CY_BLE_ANS_CAT_ID_MISSED_CALL) | \
                                            (((uint16_t) 1u) << CY_BLE_ANS_CAT_ID_SMS_MMS))

#define NOTIFICATION_TIMER_VALUE            (5000u)

/* Alert Notification Control Point Characteristic indexes */
#define ANCP_COMMAND_ID_INDEX               (0u)
#define ANCP_CATEGORY_ID_INDEX              (1u)

/* Supported New Alert Category Characteristic indexes */
#define SNAC_CATEGORY_ID_BM0_INDEX          (0u)
#define SNAC_CATEGORY_ID_BM1_INDEX          (1u)

/* Supported Unread Alert Category Characteristic indexes */
#define SUAC_CATEGORY_ID_BM0_INDEX          (SNAC_CATEGORY_ID_BM0_INDEX)
#define SUAC_CATEGORY_ID_BM1_INDEX          (SNAC_CATEGORY_ID_BM1_INDEX)

/* New Alert Characteristic indexes */
#define NA_CATEGORY_ID_INDEX                (0u)
#define NA_NUM_ALERTS_INDEX                 (1u)

/* New Alert Characteristic indexes */
#define UAS_CATEGORY_ID_INDEX               (NA_CATEGORY_ID_INDEX)
#define UAS_NUM_ALERTS_INDEX                (NA_NUM_ALERTS_INDEX)

#define ALL_CATEGORIES                      (0xFFu)
#define MAX_CATEGORIES                      (10u)

#define CY_BLE_MAX_ADV_DEVICES              (10u)
#define APP_MAX_SUPPOTED_SERVICES           (1u)

#define ANS_CATEGORY_NTF_LENGHT             (2u)

/***************************************
*        Function Prototypes
***************************************/

void AnsServiceAppEventHandler(uint32_t event, void * eventParam);
cy_en_ble_api_result_t AnsSendNotification(cy_en_ble_ans_char_index_t charIndex, uint8_t attrSize, uint8_t *attrValue);
void AnsSetNewAletCount(uint8_t ansCatId, uint8_t countValue);
uint8_t AnsGetCurAnsCatId(void);
void AnsSetCurAnsCatId(uint8_t curAnsCatIdValue);

#endif /* CY_BLE_HOST_CORE */

#endif /* ANS_H */

/* [] END OF FILE */
