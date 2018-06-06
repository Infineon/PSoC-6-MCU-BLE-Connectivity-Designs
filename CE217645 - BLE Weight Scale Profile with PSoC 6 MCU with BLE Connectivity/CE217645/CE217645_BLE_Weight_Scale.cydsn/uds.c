/*******************************************************************************
* File Name: uds.c
*
* Version: 1.0
*
* Description:
*  This file contains routines related to User Data Service.
* 
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"
#include "uds.h"


/***************************************
*        Global Variables
***************************************/
bool                      isUdsIndicationEnabled   = false;
bool                      isUdsNotificationEnabled = false;
bool                      isUdsIndicationPending   = false;
bool                      isUdsNotificationPending = false;
uds_user_record_t         udsUserRecordDef;
uds_user_record_t         udsUserRecord[MAX_USERS];
uint8_t                   ucpResp[UDS_CP_RESPONSE_MAX_SIZE];
uint8_t                   udsIndDataSize;
uint8_t                   isUserRegistered[MAX_USERS];

bool                      isUserHeightReceived = false;
bool                      isUserWeightReceived = false;

/* Stores number of registered users. Maximum is 4. */
uint8_t                   udsRegisteredUserCount;
bool                      udsAccessDenied = true;


/*******************************************************************************
* Function Name: UdsCallBack
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component
*  specific to User Data Service.
*
* Parameters:
*  event -          A UDS event.
*  eventParams - The data structure specific to an event received.
*
*******************************************************************************/
void UdsCallBack(uint32_t event, void *eventParam)
{
    uint8_t i;
    cy_stc_ble_uds_char_value_t *udsCharValPtr;
    
    switch(event)
    {

    /****************************************************
    *              UDS Server events
    ****************************************************/
    /** UDS Server - Indication for User Data Service Characteristic
        was enabled. The parameter of this event is a structure 
        of the cy_stc_ble_uds_char_value_t type.
    */
    case CY_BLE_EVT_UDSS_INDICATION_ENABLED:
        DBG_PRINTF("CY_BLE_EVT_UDSS_INDICATION_ENABLED\r\n");
        isUdsIndicationEnabled = true;
        break;

    /** UDS Server - Indication for User Data Service Characteristic
        was disabled. The parameter of this event is a structure 
        of the cy_stc_ble_uds_char_value_t type.
    */
    case CY_BLE_EVT_UDSS_INDICATION_DISABLED:
        DBG_PRINTF("CY_BLE_EVT_UDSS_INDICATION_DISABLED\r\n");
        isUdsIndicationEnabled = false;
        break;

    /** UDS Server - User Data Service Characteristic
        Indication was confirmed. The parameter of this event
        is a structure of the cy_stc_ble_uds_char_value_t type.
    */
    case CY_BLE_EVT_UDSS_INDICATION_CONFIRMED:
        DBG_PRINTF("CY_BLE_EVT_WSSS_INDICATION_CONFIRMED\r\n");
        break;

    /** UDS Server - Notifications for User Data Service Characteristic
        were enabled. The parameter of this event is a structure of the
        cy_stc_ble_uds_char_value_t type.
    */
    case CY_BLE_EVT_UDSS_NOTIFICATION_ENABLED:
        DBG_PRINTF("CY_BLE_EVT_WSSS_INDICATION_CONFIRMED\r\n");
        isUdsNotificationEnabled = true;
        break;

    /** UDS Server - Notifications for User Data Service Characteristic
        were disabled. The parameter of this event is a structure 
        of the cy_stc_ble_uds_char_value_t type.
    */
    case CY_BLE_EVT_UDSS_NOTIFICATION_DISABLED:
        DBG_PRINTF("CY_BLE_EVT_WSSS_INDICATION_CONFIRMED\r\n");
        isUdsNotificationEnabled = false;
        break;

   /** UDS Server - Read Request for User Data Service Characteristic 
        was received. The parameter of this event is a structure
        of the cy_stc_ble_uds_char_value_t type.
    */    
    case CY_BLE_EVT_UDSS_READ_CHAR:
        DBG_PRINTF("CY_BLE_EVT_UDSS_READ_CHAR\r\n");

        udsCharValPtr = (cy_stc_ble_uds_char_value_t *) eventParam;

        /* If user didn't provide correct Consent for current user index,
        * then no characteristic can be read.
        */
        if((udsAccessDenied == true) && (udsCharValPtr->charIndex != CY_BLE_UDS_UIX) && 
            (udsCharValPtr->charIndex != CY_BLE_UDS_DCI))
        {
            udsCharValPtr->gattErrorCode = CY_BLE_GATT_ERR_USER_DATA_ACCESS_NOT_PERMITTED;
        }
        DBG_PRINTF("OK\r\n");
        break;
        
    /** UDS Server - Write Request for User Data Service Characteristic 
        was received. The parameter of this event is a structure
        of the cy_stc_ble_uds_char_value_t type.
    */    
    case CY_BLE_EVT_UDSS_WRITE_CHAR:
        DBG_PRINTF("CY_BLE_EVT_UDSS_WRITE_CHAR\r\n");

        udsCharValPtr = (cy_stc_ble_uds_char_value_t *) eventParam;
        DBG_PRINTF("Char index: %d\r\n", udsCharValPtr->charIndex);

        if(udsCharValPtr->charIndex != CY_BLE_UDS_UCP)
        {
            if(userIndex != UDS_UNKNOWN_USER)
            {
                if((udsAccessDenied == true) && (udsCharValPtr->charIndex != CY_BLE_UDS_DCI))
                {
                    DBG_PRINTF("Characteristic value wasn't written: ");
                    DBG_PRINTF("Access denied. No consent provided.\n\r");
                    udsCharValPtr->gattErrorCode = CY_BLE_GATT_ERR_USER_DATA_ACCESS_NOT_PERMITTED;
                }
                else
                {
                    DBG_PRINTF("Characteristic value: ");

                    for(i = 0u; i < udsCharValPtr->value->len; i++)
                    {
                        DBG_PRINTF("%2.2x", udsCharValPtr->value->val[i]);
                    }
                    DBG_PRINTF("\n\r");

                    UdsUpdateUserRecord(udsCharValPtr->charIndex, udsCharValPtr->value);
                    
                    /* Need to update Database Change Increment characteristic */
                    UdsUpdateDatabaseChangeIncrement();
                }
            }
            else
            {
                DBG_PRINTF("Characteristic value wasn't written: ");

                if(udsAccessDenied != true)
                {
                    DBG_PRINTF("Access denied. No consent provided.\n\r");
                }
                else
                {
                    DBG_PRINTF("No user registered.\n\r");
                }

                udsCharValPtr->gattErrorCode = CY_BLE_GATT_ERR_USER_DATA_ACCESS_NOT_PERMITTED;
            }
        }
        else
        {
            UdsHandleCpResponse(udsCharValPtr->value->val);
        }
        break;

    default:
        DBG_PRINTF("Unrecognized UDS event.\r\n");
        break;
    }
}


/*******************************************************************************
* Function Name: UdsInit
********************************************************************************
*
* Summary:
*  Initializes the variables related to User Data Service's Characteristics 
*  with values from the BLE component customizer's GUI. 
*
*******************************************************************************/
void UdsInit(void)
{
    uint8_t i;
    cy_en_ble_api_result_t apiResult;
    uint8_t rdData[UDS_LAST_NAME_LENGTH]; 
    
    /* Register event handler for UDS specific events */
    Cy_BLE_UDS_RegisterAttrCallback(UdsCallBack);

    /* The initial value of the User Index characteristic is ignored in
    * this example. The users' index starts from 0 and goes up to 3.
    */
    userIndex = UDS_DEFAULT_USER;
    
    isUserRegistered[UDS_DEFAULT_USER] = true;

    udsUserRecordDef.consent = UDS_DEFAULT_CONSENT;
    
    /* Read initial value of First Name Characteristic */
    apiResult = Cy_BLE_UDSS_GetCharacteristicValue(CY_BLE_UDS_FNM, UDS_FIRST_NAME_LENGTH, rdData);

    if(apiResult == CY_BLE_SUCCESS)
    {
        DBG_PRINTF("First Name Characteristic was read successfully \r\n");

        memcpy(udsUserRecordDef.firstName, rdData, UDS_FIRST_NAME_LENGTH);
        
        for(i = UDS_DEFAULT_USER; i < MAX_USERS; i++)
        {
            memcpy(udsUserRecord[i].firstName, rdData, UDS_FIRST_NAME_LENGTH); 
        }
    }
    else
    {
        DBG_PRINTF("Error while reading First Name Characteristic. Error code: %d \r\n", apiResult);
    }

    /* Read initial value of Last Name Characteristic */
    apiResult = Cy_BLE_UDSS_GetCharacteristicValue(CY_BLE_UDS_LNM, UDS_LAST_NAME_LENGTH, rdData);

    if(apiResult == CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Last Name Characteristic was read successfully \r\n");

        memcpy(udsUserRecordDef.lastName, rdData, UDS_LAST_NAME_LENGTH);
        
        for(i = UDS_DEFAULT_USER; i < MAX_USERS; i++)
        {
            memcpy(udsUserRecord[i].lastName, rdData, UDS_LAST_NAME_LENGTH);  
        }
    }
    else
    {
        DBG_PRINTF("Error while reading Last Name Characteristic. Error code: %d \r\n", apiResult);
    }

    /* Read initial value of Age Characteristic */
    apiResult = Cy_BLE_UDSS_GetCharacteristicValue(CY_BLE_UDS_AGE, 1u, &udsUserRecordDef.age);

    if(apiResult == CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Age Characteristic was read successfully \r\n");
        
        /* Fill the Age Characteristic value for all users with value from
        * component customizer.
        */
        for(i = UDS_DEFAULT_USER; i < MAX_USERS; i++)
        {
            udsUserRecord[i].age = udsUserRecordDef.age;
        }
    }
    else
    {
        DBG_PRINTF("Error while reading Age Characteristic. Error code: %d \r\n", apiResult);
    }

    /* Read initial value of Gender Characteristic */
    apiResult = Cy_BLE_UDSS_GetCharacteristicValue(CY_BLE_UDS_GND, 1u, &udsUserRecordDef.gender);

    if(apiResult == CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Gender Characteristic was read successfully \r\n");
        
        /* Fill Gender Characteristic value for all users with value from
        * component customizer.
        */
        for(i = UDS_DEFAULT_USER; i < MAX_USERS; i++)
        {
            udsUserRecord[i].gender = udsUserRecordDef.gender;
        }
    }
    else
    {
        DBG_PRINTF("Error while reading Gender Characteristic. Error code: %d \r\n", apiResult);
    }

    /* Read initial value of Weight Characteristic */
    apiResult = Cy_BLE_UDSS_GetCharacteristicValue(CY_BLE_UDS_WGT, 2u, rdData);

    if(apiResult == CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Weight Characteristic was read successfully \r\n");
        
        udsUserRecordDef.weight = PACK_U16(rdData[0u], rdData[1u]);

        /* Fill Weight Characteristic value for all users with value from
        * component customizer.
        */
        for(i = UDS_DEFAULT_USER; i < MAX_USERS; i++)
        {
            udsUserRecord[i].weight = udsUserRecordDef.weight;
        }
    }
    else
    {
        DBG_PRINTF("Error while reading Weight Characteristic. Error code: %d \r\n", apiResult);
    }

    /* Read initial value of Height Characteristic */
    apiResult = Cy_BLE_UDSS_GetCharacteristicValue(CY_BLE_UDS_HGT, 2u, (uint8_t *) &udsUserRecordDef.height);

    if(apiResult == CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Height Characteristic was read successfully \r\n");
        
        /* Fill Height Characteristic value for all users with value from
        * component customizer.
        */
        for(i = UDS_DEFAULT_USER; i < MAX_USERS; i++)
        {
            udsUserRecord[i].height = udsUserRecordDef.height;
        }
    }
    else
    {
        DBG_PRINTF("Error while reading Height Characteristic. Error code: %d \r\n", apiResult);
    }

    /* Read initial value of Database Change Increment Characteristic */
    apiResult =
        Cy_BLE_UDSS_GetCharacteristicValue(CY_BLE_UDS_DCI, 4u, (uint8_t *) &udsUserRecordDef.dbChangeIncrement);

    if(apiResult == CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Database Change Increment Characteristic was read successfully \r\n");
        
        /* Fill Database Change Increment Characteristic value for all users with
        * value from the component customizer.
        */
        for(i = UDS_DEFAULT_USER; i < MAX_USERS; i++)
        {
            udsUserRecord[i].dbChangeIncrement = udsUserRecordDef.dbChangeIncrement;
        }
    }
    else
    {
        DBG_PRINTF("Error while reading Database Change Increment Characteristic. Error code: %d \r\n", apiResult);
    }
    
    udsRegisteredUserCount = 1u;
}


/*******************************************************************************
* Function Name: UdsHandleCpResponse
********************************************************************************
*
* Summary:
*  Handles sending the response for UDS User Control Point.
*
* Parameters:  
*  charValue: UDS User Control Point Characteristic value written by the Client.
*
*******************************************************************************/
void UdsHandleCpResponse(uint8_t *charValue)
{
    uint8_t i;
    uint8_t byteCount = 0u;
    cy_en_ble_api_result_t apiResult;

    ucpResp[byteCount++] = UDS_CP_RESPONSE;

    if(charValue != NULL)
    {
        switch (charValue[0u])
        {
        /* Handler for Register New User command */
        case UDS_CP_REGISTER_NEW_USER:
            if(udsRegisteredUserCount < MAX_USERS)
            {
                for(i = 0u; i < MAX_USERS; i++)
                {
                    if(isUserRegistered[i] == false)
                    {
                        /* Increment user records count */
                        udsRegisteredUserCount++;
                        isUserRegistered[i] = true;
                        userIndex = i;
                        break;
                    }
                }

                /* Clear access denied flag */
                udsAccessDenied = false;

                /* Fill in received consent for new user */
                udsUserRecord[userIndex].consent =
                    (((uint16_t) charValue[UDS_CP_PARAM_BYTE2_IDX]) << 8u) | ((uint16_t) charValue[UDS_CP_PARAM_BYTE1_IDX]);
                        
                DBG_PRINTF("New user registered. User ID: %d. Consent: 0x%4.4x\r\n", userIndex,
                            udsUserRecord[udsRegisteredUserCount - 1u].consent);

                /* Form response packet */
                ucpResp[byteCount++] = UDS_CP_REGISTER_NEW_USER;
                ucpResp[byteCount++] = UDS_CP_RESP_VALUE_SUCCESS;
                ucpResp[byteCount++] = userIndex;

                /* Set active user's ID to User ID Characteristics */
                apiResult = Cy_BLE_UDSS_SetCharacteristicValue(CY_BLE_UDS_UIX, 1u, &userIndex);

                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_UDSS_SetCharacteristicValue() API Error: %x \r\n", apiResult);
                }
                else
                {
                    DBG_PRINTF("Cy_BLE_UDSS_SetCharacteristicValue() Success \r\n");
                }
            }
            else
            {
                ucpResp[byteCount++] = charValue[0u];
                ucpResp[byteCount++] = UDS_CP_RESP_OPERATION_FAILED;
                DBG_PRINTF("Too many user records\r\n");
            }
            break;
        case UDS_CP_CONSENT:
            ucpResp[byteCount++] = UDS_CP_CONSENT;
            if(charValue[UDS_CP_PARAM_BYTE1_IDX] < MAX_USERS)
            {
                if(isUserRegistered[charValue[UDS_CP_PARAM_BYTE1_IDX]] == true)
                {
                    if(udsUserRecord[charValue[UDS_CP_PARAM_BYTE1_IDX]].consent ==
                        ((uint16_t)((((uint16_t) charValue[UDS_CP_PARAM_BYTE3_IDX]) << 8u) |
                            ((uint16_t) charValue[UDS_CP_PARAM_BYTE2_IDX]))))
                    {
                        ucpResp[byteCount++] = UDS_CP_RESP_VALUE_SUCCESS;

                        /* Clear access denied flag */
                        udsAccessDenied = false;

                        DBG_PRINTF("Access allowed for: %s, %s (Index - %d).\r\n", 
                                udsUserRecord[userIndex].firstName,
                                udsUserRecord[userIndex].lastName,
                                userIndex);
                        UdsLoadUserDataToDb(charValue[UDS_CP_PARAM_BYTE1_IDX]);
                    }
                    else
                    {
                        ucpResp[byteCount++] = UDS_CP_RESP_USER_NOT_AUTHORIZED;
                        DBG_PRINTF("Wrong consent\r\n");
                    }
                }
                else
                {
                    DBG_PRINTF("User with index - %d is not registered.\r\n", charValue[UDS_CP_PARAM_BYTE1_IDX]);
                    ucpResp[byteCount++] = UDS_CP_RESP_OPERATION_FAILED;
                }
            }
            else
            {
                DBG_PRINTF("Invalid user index - %d.\r\n", charValue[UDS_CP_PARAM_BYTE1_IDX]);
                ucpResp[byteCount++] = UDS_CP_RESP_INVALID_PARAMETER;
            }
            break;
        case UDS_CP_DELETE_USER_DATA:
            if(udsRegisteredUserCount == 0u)
            {
                ucpResp[byteCount++] = UDS_CP_DELETE_USER_DATA;
                ucpResp[byteCount++] = UDS_CP_RESP_OPERATION_FAILED;
                DBG_PRINTF("Failed to delete user. No registered users\r\n");
            }
            else
            {
                ucpResp[byteCount++] = UDS_CP_DELETE_USER_DATA;
                ucpResp[byteCount++] = UDS_CP_RESP_VALUE_SUCCESS;

                DBG_PRINTF("User record for: %s, %s (Index - %d) is deleted.\r\n", 
                        udsUserRecord[userIndex].firstName,
                        udsUserRecord[userIndex].lastName,
                        userIndex);

                isUserRegistered[userIndex] = false;

                /* Decrement user records count */
                udsRegisteredUserCount--;
                userIndex = UdsFindRegisteredUserIndex();

                if(userIndex != UDS_UNKNOWN_USER)
                {
                    /* Load data for the user whith userIndex-1 */
                    UdsLoadUserDataToDb(userIndex);
                }

                udsAccessDenied = true;
            }
            break;
        default:
            ucpResp[byteCount++] = charValue[0u];
            ucpResp[byteCount++] = UDS_CP_RESP_OP_CODE_NOT_SUPPORTED;
            DBG_PRINTF("Unknown UCP Op Code\r\n");
            break;
        }

        udsIndDataSize = byteCount;

        /* Set flag that response is ready to be indicated */
        isUdsIndicationPending = true;
    }
}


/*******************************************************************************
* Function Name: UdsLoadUserDataToDb
********************************************************************************
*
* Summary:
*  Loads all the data related to the user identified by "userIndex" into the GATT 
*  database.
*
* Parameters:  
*  userIndex - The UDS User Control Point Characteristic value written by the Client.
*
*******************************************************************************/
void UdsLoadUserDataToDb(uint8_t uIdx)
{
    uint8_t buff[4u];
    cy_en_ble_api_result_t apiResult;

    userIndex = uIdx;

    /* Set all User's characteristic into GATT DB */
    apiResult = Cy_BLE_UDSS_SetCharacteristicValue(CY_BLE_UDS_UIX, 1u, &userIndex);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Set User index - API Error: %x\r\n", apiResult);
    }
    
    apiResult = Cy_BLE_UDSS_SetCharacteristicValue(CY_BLE_UDS_AGE, 1u, &udsUserRecord[userIndex].age);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Set Age - API Error: %x\r\n", apiResult);
    }
    
    apiResult = Cy_BLE_UDSS_SetCharacteristicValue(CY_BLE_UDS_GND, 1u, &udsUserRecord[userIndex].gender);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Set Gender - API Error: %x\r\n", apiResult);
    }
    
    apiResult =
        Cy_BLE_UDSS_SetCharacteristicValue(CY_BLE_UDS_FNM, UDS_FIRST_NAME_LENGTH, udsUserRecord[userIndex].firstName);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Set First Name - Error (Error Code: %x)\r\n", apiResult);
    }
    
    apiResult = Cy_BLE_UDSS_SetCharacteristicValue(CY_BLE_UDS_LNM, UDS_LAST_NAME_LENGTH, udsUserRecord[userIndex].lastName);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Set Last Name - API Error: %x\r\n", apiResult);
    }
    
    buff[0u] = CY_LO8(udsUserRecord[userIndex].height);
    buff[1u] = CY_HI8(udsUserRecord[userIndex].height);
    apiResult = Cy_BLE_UDSS_SetCharacteristicValue(CY_BLE_UDS_HGT, 2u, buff);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Set Height - API Error: %x\r\n", apiResult);
    }

    buff[0u] = CY_LO8(udsUserRecord[userIndex].weight);
    buff[1u] = CY_HI8(udsUserRecord[userIndex].weight);
    apiResult = Cy_BLE_UDSS_SetCharacteristicValue(CY_BLE_UDS_WGT, 2u, buff);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Set Weight - API Error: %x\r\n", apiResult);
    }

    buff[0u] = CY_LO8(CY_LO16(udsUserRecord[userIndex].dbChangeIncrement));
    buff[1u] = CY_HI8(CY_LO16(udsUserRecord[userIndex].dbChangeIncrement));
    buff[2u] = CY_LO8(CY_HI8(udsUserRecord[userIndex].dbChangeIncrement));
    buff[3u] = CY_HI8(CY_HI8(udsUserRecord[userIndex].dbChangeIncrement));
    apiResult =
        Cy_BLE_UDSS_SetCharacteristicValue(CY_BLE_UDS_DCI, 4u, buff);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Set Database Change Increment - API Error: %x\r\n", apiResult);
    }
}


/*******************************************************************************
* Function Name: UdsUpdateUserRecord
********************************************************************************
*
* Summary:
*  Performs an update of the Characteristic value identified by the "charIndex"
*  in the user records array. The "userIndex" contains the index of an active
*  user record set.
* 
*  The data is stored in the same byte order as in the GATT database.
*
* Parameters:  
*  charIndex - The characteristic index.
*  charValue - The characteristic value to be stored.
*
*******************************************************************************/
void UdsUpdateUserRecord(cy_en_ble_uds_char_index_t charIndex, cy_stc_ble_gatt_value_t *charValue)
{
    if(charValue != NULL)
    {
        switch (charIndex)
        {
        case CY_BLE_UDS_FNM:
            memcpy(udsUserRecord[userIndex].firstName, charValue->val, charValue->len); 
            break;
        case CY_BLE_UDS_LNM:
            memcpy(udsUserRecord[userIndex].lastName, charValue->val, charValue->len);
            break;
        case CY_BLE_UDS_AGE:
            udsUserRecord[userIndex].age = charValue->val[0u];
            break;
        case CY_BLE_UDS_GND:
            udsUserRecord[userIndex].gender = charValue->val[0u];
            break;
        case CY_BLE_UDS_WGT:
            memcpy((void *) &udsUserRecord[userIndex].weight, charValue->val, charValue->len);
            isUserWeightReceived = true;
            break;
        case CY_BLE_UDS_HGT:
            memcpy((void *) &udsUserRecord[userIndex].height, charValue->val, charValue->len);
            isUserHeightReceived = true;
            break;
        case CY_BLE_UDS_DCI:
            memcpy((void *) &udsUserRecord[userIndex].dbChangeIncrement, charValue->val, charValue->len);
            break;
        default:
            break;
        }
    }
}


/*******************************************************************************
* Function Name: UdsSetWeight
********************************************************************************
*
* Summary:
*  Sets weight for the currently active user in both GATT database and 
*  user records array. 
*
* Parameters:  
*  weight - The weight to be set.
*
*******************************************************************************/
void UdsSetWeight(uint16_t weight)
{
    uint8_t buff[2u];
    cy_en_ble_api_result_t apiResult;

    udsUserRecord[userIndex].weight = weight;

    buff[0u] = CY_LO8(weight);
    buff[1u] = CY_HI8(weight);

    apiResult = Cy_BLE_UDSS_SetCharacteristicValue(CY_BLE_UDS_WGT, 2u, buff);

    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Set Weight - API Error: %x\r\n", apiResult);
    }

    /* Need to update Database Change Increment characteristic */
    UdsUpdateDatabaseChangeIncrement();
}


/*******************************************************************************
* Function Name: UdsSetHeight
********************************************************************************
*
* Summary:
*  Sets height for the currently active user in both GATT database and 
*  user records array. 
*
* Parameters:  
*  height - The height to be set.
*
*******************************************************************************/
void UdsSetHeight(uint16_t height)
{
    uint8_t buff[2u];
    cy_en_ble_api_result_t apiResult;

    udsUserRecord[userIndex].height = height;

    buff[0u] = CY_LO8(height);
    buff[1u] = CY_HI8(height);

    apiResult = Cy_BLE_UDSS_SetCharacteristicValue(CY_BLE_UDS_WGT, 2u, (uint8_t *) &buff);

    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Set Height - API Error: %x\r\n", apiResult);
    }

    /* Need to update Database Change Increment characteristic */
    UdsUpdateDatabaseChangeIncrement();
}


/*******************************************************************************
* Function Name: UdsUpdateDatabaseChangeIncrement
********************************************************************************
*
* Summary:
*  Updates the Database Change increment Characteristic for the active user in both
*  GATT database and user records array.
*
*******************************************************************************/
void UdsUpdateDatabaseChangeIncrement(void)
{
    cy_en_ble_api_result_t apiResult;
    uint8_t buff[4u];

    udsUserRecord[userIndex].dbChangeIncrement += 1u;

    buff[0u] = CY_LO8(CY_LO16(udsUserRecord[userIndex].dbChangeIncrement));
    buff[1u] = CY_HI8(CY_LO16(udsUserRecord[userIndex].dbChangeIncrement));
    buff[2u] = CY_LO8(CY_HI8(udsUserRecord[userIndex].dbChangeIncrement));
    buff[3u] = CY_HI8(CY_HI8(udsUserRecord[userIndex].dbChangeIncrement));

    apiResult = Cy_BLE_UDSS_SetCharacteristicValue(CY_BLE_UDS_DCI, 4u, buff);

    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Set Database Change Increment - API Error: %x\r\n", apiResult);
    }

    isUdsNotificationPending = true;
}


/*******************************************************************************
* Function Name: UdsFindRegisteredUserIndex
********************************************************************************
*
* Summary:
*  Returns the index of registered user. The search starts from the 
*  (MAX_USERS - 1u), where MAX_USERS = 4, and goes down to 0. If no registered
*  users were found the value of 0xFF will be returned.
*
* Return:
*  UDS_UNKNOWN_USER - No registered users found, 1-4 - index of registered user.
*
*******************************************************************************/
uint8_t UdsFindRegisteredUserIndex(void)
{
    uint8_t index;

    for(index = (MAX_USERS - 1u); index > 0u; index--)
    {
        if(isUserRegistered[index] == true)
        {
            break;
        }
    }

    if(index == 0u)
    {
        /* If there are no registered users, set user index to "Unknown user" */
        index = UDS_UNKNOWN_USER;
    }

    return(index);
}

/*******************************************************************************
* Function Name: UdsUserChange
********************************************************************************
*
* Summary:
*  Changes current user.
*
*******************************************************************************/
void UdsUserChange(void)
{
    if(Cy_BLE_GetConnectionState(appConnHandle) >= CY_BLE_CONN_STATE_CONNECTED)
    {
        uint8_t userIndexLoc = userIndex;
        
        if((udsRegisteredUserCount - 1u) > userIndexLoc)
        {
            userIndexLoc++;
        }
        else
        {
            userIndexLoc = 0u;
        }
        if(userIndexLoc != userIndex)
        {
            userIndex = userIndexLoc;
            UdsLoadUserDataToDb(userIndex);
            udsAccessDenied = true;
            DBG_PRINTF("User changed to %s, %s (User Index: %d) \r\n", udsUserRecord[userIndex].firstName, 
                                                                       udsUserRecord[userIndex].lastName,
                                                                       userIndex);
        }
    }
}
/* [] END OF FILE */
