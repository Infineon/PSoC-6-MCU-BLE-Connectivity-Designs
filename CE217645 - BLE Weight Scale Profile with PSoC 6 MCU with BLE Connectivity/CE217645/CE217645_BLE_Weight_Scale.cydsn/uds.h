/*******************************************************************************
* File Name: uds.h
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants used by User Data Service.
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>

    
/***************************************
*          Constants
***************************************/
#define UDS_FIRST_NAME_LENGTH                       (30u)
#define UDS_LAST_NAME_LENGTH                        (50u)

#define UDS_GENDER_MALE                             (0u)
#define UDS_GENDER_FEMALE                           (1u)
#define UDS_GENDER_UNSPECIFIED                      (2u)

/* UCP Characteristic Value indexes */
#define UDS_CP_OP_CODE_IDX                          (0u)
#define UDS_CP_PARAM_BYTE1_IDX                      (1u)
#define UDS_CP_PARAM_BYTE2_IDX                      (2u)
#define UDS_CP_PARAM_BYTE3_IDX                      (3u)
#define UDS_CP_PARAM_BYTE4_IDX                      (4u)
#define UDS_CP_PARAM_BYTE5_IDX                      (5u)
#define UDS_CP_PARAM_BYTE6_IDX                      (6u)
#define UDS_CP_REQ_OP_CODE_IDX                      (UDS_CP_PARAM_BYTE1_IDX)
#define UDS_CP_RESP_VALUE_IDX                       (UDS_CP_PARAM_BYTE2_IDX)
#define UDS_CP_RESP_PARAM_IDX                       (UDS_CP_PARAM_BYTE3_IDX)

/* UCP Characteristic Response values */
#define UDS_CP_RESP_VALUE_SUCCESS                   (1u)
#define UDS_CP_RESP_OP_CODE_NOT_SUPPORTED           (2u)
#define UDS_CP_RESP_INVALID_PARAMETER               (3u)
#define UDS_CP_RESP_OPERATION_FAILED                (4u)
#define UDS_CP_RESP_USER_NOT_AUTHORIZED             (5u)

#define UDS_CP_REGISTER_NEW_USER                    (1u)
#define UDS_CP_CONSENT                              (2u)
#define UDS_CP_DELETE_USER_DATA                     (3u)
#define UDS_CP_RESPONSE                             (0x20u)

#define UDS_CP_RESPONSE_MAX_SIZE                    (20u)

#define UDS_CP_RESP_CODE_IDX                        (0u)

#define UDS_UNKNOWN_USER                            (0xFFu)
#define UDS_DEFAULT_USER                            (0x00u)
#define UDS_DEFAULT_CONSENT                         (0x0000u)

#define UDS_NOTIFICATION_SIZE                       (0x04u)


/***************************************
*       Data Struct Definition
***************************************/
/* UDS Measurement value type */
typedef struct
{
    uint8_t firstName[UDS_FIRST_NAME_LENGTH];
    uint8_t lastName[UDS_LAST_NAME_LENGTH];
    uint8_t age;
    uint8_t gender;
    uint16_t weight;
    uint16_t height;
    uint32_t dbChangeIncrement;
    uint16_t consent;
}__PACKED uds_user_record_t;


/***************************************
*        Function Prototypes
***************************************/
void UdsInit(void);
void UdsCallBack(uint32_t event, void *eventParam);
void UdsHandleCpResponse(uint8_t *charValue);
void UdsLoadUserDataToDb(uint8_t uIdx);
void UdsUpdateUserRecord(cy_en_ble_uds_char_index_t charIndex, cy_stc_ble_gatt_value_t *charValue);
void UdsSetWeight(uint16_t weight);
void UdsSetHeight(uint16_t height);
void UdsUpdateDatabaseChangeIncrement(void);
uint8_t UdsFindRegisteredUserIndex(void);
void UdsUserChange(void);


/***************************************
* External data references
***************************************/
extern uds_user_record_t         udsUserRecord[];
extern uint8_t                   ucpResp[UDS_CP_RESPONSE_MAX_SIZE];
extern uint8_t                   udsIndDataSize;
extern uint8_t                   userIndex;
extern uint8_t                   udsRegisteredUserCount;
extern bool                      udsAccessDenied;
extern bool                      isUdsIndicationEnabled;
extern bool                      isUdsNotificationEnabled;
extern bool                      isUdsIndicationPending;
extern bool                      isUdsNotificationPending;
extern bool                      isUserHeightReceived;
extern bool                      isUserWeightReceived;

    
/* [] END OF FILE */
