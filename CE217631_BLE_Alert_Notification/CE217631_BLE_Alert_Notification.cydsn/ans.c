/*******************************************************************************
* File Name: ans.c
*
* Version: 1.0
*
* Description:
*  This file contains ANS service related code.
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

#include <project.h>
#include <stdio.h>
#include "ans.h"
#include "common.h"

static uint16_t ntfCatIdFlag;
static uint8_t  newAletCount[MAX_CATEGORIES];
static uint8_t curAnsCatId = ALL_CATEGORIES;

/*******************************************************************************
* Function Name: AnsSendNotification
********************************************************************************
*
* Summary:
*  This function sends ANS Notification
* 
* Parameters:  
*   charIndex   - index of Alert Notification Characteristic Category
*   attrSize    - size of attribute value
*   attrValue   - attribute value
*
*******************************************************************************/
cy_en_ble_api_result_t AnsSendNotification(cy_en_ble_ans_char_index_t charIndex, uint8_t attrSize, uint8_t *attrValue)
{
    cy_en_ble_api_result_t result;
    
    if(ntfCatIdFlag & ((uint16_t) (1u << attrValue[0u])))
    {
        result = Cy_BLE_ANSS_SendNotification(appConnHandle, charIndex, attrSize, attrValue);
    }
    else
    {
        result = CY_BLE_ERROR_INVALID_OPERATION;
    }
    
    return(result);
}


/*******************************************************************************
* Function Name: AnsServiceAppEventHandler
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE Component,
*   which are specific to Alert Notification Service.
*
* Parameters:
*   event:       ANS event.
*   eventParams: Data structure specific to event.
*
*******************************************************************************/
void AnsServiceAppEventHandler(uint32_t event, void *eventParam)
{
    uint8_t i;
    uint16_t rdCategories;
    uint8_t locCatId;
    uint8_t charVal[2u];
    cy_stc_ble_ans_char_value_t *rdCharVal;
    cy_stc_ble_ans_char_value_t *wrReqParam;
    cy_en_ble_api_result_t apiResult;
    

    switch(event)
    {
    /***************************************
    *        ANS Client events
    ***************************************/
    case CY_BLE_EVT_ANSC_READ_CHAR_RESPONSE:

        rdCharVal = (cy_stc_ble_ans_char_value_t *) eventParam;

        /* Get value of Server's supported categories */
        MakeWordFromBytePtr(rdCharVal->value->val, &rdCategories);

        if(CY_BLE_ANS_SUPPORTED_NEW_ALERT_CAT == rdCharVal->charIndex)
        {
            DBG_PRINTF("Supported New Alert Category Characteristic read response\r\n");
        }
        else
        {
            DBG_PRINTF("Unread Alert Category Characteristic read response \r\n");
        }

        DBG_PRINTF("Read Characteristic value is:\r\n");
        DBG_PRINTF("Category ID Bit Mask 0: %x\r\n", rdCharVal->value->val[0u]);
        DBG_PRINTF("Category ID Bit Mask 1: %x\r\n", rdCharVal->value->val[1u]);
        break;

    case CY_BLE_EVT_ANSC_WRITE_CHAR_RESPONSE:
        /* Confirmation of successful characteristic write operation */
        DBG_PRINTF("The Alert Notification Control Point Characteristic was written successfully.\r\n");
        break;

    case CY_BLE_EVT_ANSC_WRITE_DESCR_RESPONSE:
        /* Confirmation of successful characteristic descriptor write operation */
        DBG_PRINTF("The ANS Characteristic's Descriptor was written successfully.\r\n");
        break;

    case CY_BLE_EVT_ANSC_NOTIFICATION:

        rdCharVal = (cy_stc_ble_ans_char_value_t *) eventParam;

        if(rdCharVal->charIndex == CY_BLE_ANS_NEW_ALERT)
        {
            DBG_PRINTF("\r\nThe New Alert Characteristic notification received.\r\n");
        }
        else
        {
            DBG_PRINTF("\r\nThe Unread Alert Status Characteristic notification received.\r\n");
        }

        /* Verify notified data if it fits to categories Client supports */
        if((CY_BLE_ANS_CAT_ID_INSTANT_MESSAGE >= rdCharVal->value->val[0u]) &&
            ((((uint16_t) 1u) << rdCharVal->value->val[0u]) & ANS_CLIENT_SUPPORTED_CATEGORIES))
        {
            /* Print information about received value and turn on the corresponding LED.
            * Red LED = Missed call, Green LED = Email, Blue LED = SMS/MMS.
            */
            DBG_PRINTF("The notified value is:\r\n");

            switch(rdCharVal->value->val[0u])
            {
                case CY_BLE_ANS_CAT_ID_EMAIL:
                    DBG_PRINTF("The Category ID - Email.\r\n");
                    curAnsCatId = CY_BLE_ANS_CAT_ID_EMAIL;
                    break;
                    
                case CY_BLE_ANS_CAT_ID_MISSED_CALL:
                    DBG_PRINTF("The Category ID - Missed Call.\r\n");
                    curAnsCatId = CY_BLE_ANS_CAT_ID_MISSED_CALL;
                    break;
                    
                case CY_BLE_ANS_CAT_ID_SMS_MMS:
                    DBG_PRINTF("The Category ID - SMS/MMS.\r\n");
                    curAnsCatId = CY_BLE_ANS_CAT_ID_SMS_MMS;
                    break;
                default:
                    DBG_PRINTF("Unsupported Category ID - 0x%X\r\n",rdCharVal->value->val[0u]);
                    break;
            }
            UpdateLedState();

            DBG_PRINTF("The number of alerts: %x.\r\n", rdCharVal->value->val[1u]);

            if(rdCharVal->value->len > 2u)
            {
                DBG_PRINTF("Text: \"");

                for(i = 2u; i < rdCharVal->value->len; i++)
                {
                    DBG_PRINTF("%c", rdCharVal->value->val[i]);
                }
                DBG_PRINTF("\".\r\n");
            }
        }
        else
        {
            DBG_PRINTF("Notified category is not supported.\r\n");
        }
        break;

    case CY_BLE_EVT_ANSC_READ_DESCR_RESPONSE:
        /* Not used in this example */
        break;

        
    /***************************************
    *        ANS Server events
    ***************************************/
    case CY_BLE_EVT_ANSS_NOTIFICATION_ENABLED:
        wrReqParam = (cy_stc_ble_ans_char_value_t *)eventParam;
        
        if(CY_BLE_ANS_NEW_ALERT == wrReqParam->charIndex)
        {
            DBG_PRINTF("The Notifications for New Alert Characteristic are enabled\r\n");
        }
        else
        {
            DBG_PRINTF("The Notifications for Unread Alert Status Characteristic are enabled\r\n");
        }
        break;
        
    case CY_BLE_EVT_ANSS_NOTIFICATION_DISABLED:
        wrReqParam = (cy_stc_ble_ans_char_value_t *)eventParam;
        
        if(CY_BLE_ANS_NEW_ALERT == wrReqParam->charIndex)
        {
            DBG_PRINTF("The Notifications for New Alert Characteristic are disabled\r\n");
        }
        else
        {
            DBG_PRINTF("The Notifications for Unread Alert Status Characteristic are disabled\r\n");
        }
        break;
        
    case CY_BLE_EVT_ANSS_WRITE_CHAR:
        wrReqParam = (cy_stc_ble_ans_char_value_t *) eventParam;
        
        DBG_PRINTF("\r\nWrite to Alert Notification Control Point Characteristic occurred\r\n");     
        locCatId = wrReqParam->value->val[ANCP_CATEGORY_ID_INDEX];
        
            
        switch(wrReqParam->value->val[ANCP_COMMAND_ID_INDEX])
        {

            /* For simplification enabling and disabling for the Supported New Category
            * and Supported Unread Category are handled together. 
            */
            case CY_BLE_ANS_EN_NEW_ALERT_NTF:
            case CY_BLE_ANS_EN_UNREAD_ALERT_STATUS_NTF:
                if(locCatId <= CY_BLE_ANS_CAT_ID_INSTANT_MESSAGE)
                {
                    ntfCatIdFlag = (ALL_CATEGORIES & ntfCatIdFlag) | (1u << locCatId);
                }
                else if(locCatId == CY_BLE_ANS_CAT_ID_ALL)
                {
                    /* Enable notification for all categories */
                    ntfCatIdFlag = ALL_CATEGORIES; 
                }
                else
                {
                    DBG_PRINTF("Improper Category ID received: 0x%X \r\n", locCatId);
                }
                break;
                
            case CY_BLE_ANS_DIS_NEW_ALERT_NTF:
            case CY_BLE_ANS_DIS_UNREAD_ALERT_STATUS_NTF:
                if(locCatId <= CY_BLE_ANS_CAT_ID_INSTANT_MESSAGE)
                {
                    ntfCatIdFlag &= (~(1u << locCatId));
                }
                else if(locCatId == CY_BLE_ANS_CAT_ID_ALL)
                {
                    /* Disable notification for all categories */
                    ntfCatIdFlag = 0u;
                }
                else
                {
                    DBG_PRINTF("Improper Category ID received: 0x%X \r\n", locCatId);
                }
                break;
                
            case CY_BLE_ANS_IMM_NEW_ALERT_NTF:
                if(locCatId <= CY_BLE_ANS_CAT_ID_INSTANT_MESSAGE)
                {
                    /* Send notification for specific alert category if notifications are enabled */
                    if((ntfCatIdFlag & (1u << locCatId)) != 0u)
                    {
                        charVal[NA_CATEGORY_ID_INDEX] = locCatId;
                        charVal[NA_NUM_ALERTS_INDEX]  = newAletCount[locCatId];
                        apiResult = AnsSendNotification(CY_BLE_ANS_NEW_ALERT, ANS_CATEGORY_NTF_LENGHT, charVal);
                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("AnsSendNotification API error: 0x%x \r\n", apiResult);
                        }
                    }
                }
                else if(locCatId == ALL_CATEGORIES)
                {
                    /* Send notification for all enabled categories */
                    for(i = 0u; i < MAX_CATEGORIES; i++)
                    {
                        if((ntfCatIdFlag & (1u << i)) != 0u)
                        {
                            charVal[NA_CATEGORY_ID_INDEX] = i;
                            charVal[NA_NUM_ALERTS_INDEX]  = newAletCount[i];
                            apiResult = AnsSendNotification(CY_BLE_ANS_NEW_ALERT, ANS_CATEGORY_NTF_LENGHT, charVal);
                            if(apiResult != CY_BLE_SUCCESS)
                            {
                                DBG_PRINTF("AnsSendNotification API error: 0x%x \r\n", apiResult);
                            }
                        }
                    }
                }
                else
                {
                    DBG_PRINTF("Improper Category ID received: 0x%X \r\n", locCatId);
                }
                break;
                
            case CY_BLE_ANS_IMM_UNREAD_ALERT_STATUS_NTF:
                if(locCatId <= CY_BLE_ANS_CAT_ID_INSTANT_MESSAGE)
                {
                    /* Send notification for specific alert category if notifications are enabled */
                    if((ntfCatIdFlag & (1u << locCatId)) != 0u)
                    {
                        charVal[NA_CATEGORY_ID_INDEX] = locCatId;
                        charVal[NA_NUM_ALERTS_INDEX]  = newAletCount[locCatId];
                        apiResult = AnsSendNotification(CY_BLE_ANS_UNREAD_ALERT_STATUS, ANS_CATEGORY_NTF_LENGHT, charVal);
                        if(apiResult != CY_BLE_SUCCESS)
                        {
                            DBG_PRINTF("AnsSendNotification API error: 0x%x \r\n", apiResult);
                        }
                    }
                }
                else if(locCatId == ALL_CATEGORIES)
                {
                    /* Send notification for all enabled categories */
                    for(i = 0u; i < MAX_CATEGORIES; i++)
                    {
                        if((ntfCatIdFlag & (1u << i)) != 0u)
                        {
                            charVal[NA_CATEGORY_ID_INDEX] = i;
                            charVal[NA_NUM_ALERTS_INDEX]  = newAletCount[i];
                            apiResult = AnsSendNotification(CY_BLE_ANS_UNREAD_ALERT_STATUS, ANS_CATEGORY_NTF_LENGHT, charVal);
                            if(apiResult != CY_BLE_SUCCESS)
                            {
                                DBG_PRINTF("AnsSendNotification API error: 0x%x \r\n", apiResult);
                            }
                        }
                    }
                }
                else
                {
                    DBG_PRINTF("Improper Category ID received: 0x%X \r\n", locCatId);
                }
                break;
            
            default:
                DBG_PRINTF("Unsupported Command ID: 0x%X\r\n", wrReqParam->value->val[ANCP_COMMAND_ID_INDEX]);
                break;
        } 
        break;

    default:
        DBG_PRINTF("Other event: 0x%lX \r\n", event);
        break;
    }
}


/*******************************************************************************
* Function Name: AnsSetNewAletCount
********************************************************************************
*
* Summary:
*   This function sets newAletCount value.
*
* Parameters:  
*   ansCatId   - index of Alert Notification Characteristic Category
*   countValue - new value of newAletCount
*
*******************************************************************************/
void AnsSetNewAletCount(uint8_t ansCatId, uint8_t countValue)
{
    newAletCount[ansCatId] = countValue;
}


/*******************************************************************************
* Function Name: AnsGetCurAnsCatId
********************************************************************************
*
* Summary:
*   This function returns curAnsCatId value.
*
* Returns:
*  uint8_t - curAnsCatId value.
*
*******************************************************************************/
uint8_t AnsGetCurAnsCatId(void)
{
    return(curAnsCatId);
}


/*******************************************************************************
* Function Name: AnsSetCurAnsCatId
********************************************************************************
*
* Summary:
*   This function sets curAnsCatId value.
*
* Parameters:  
*   curAnsCatIdValue - new value of curAnsCatId
*
*******************************************************************************/
void AnsSetCurAnsCatId(uint8_t curAnsCatIdValue)
{
    curAnsCatId = curAnsCatIdValue;
}
/* [] END OF FILE */
