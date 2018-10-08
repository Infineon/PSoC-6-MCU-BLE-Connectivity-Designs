/*******************************************************************************
* File Name: ancsc.c
*
* Version 1.0
*
* Description:
*  This file contains the Apple Notification Center Service related code.
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

#include "ancsc.h"
#include "common.h"
#include "user_interface.h"

    
/* Global variables */
uint8_t ancsFlag = 0u; /* ANCS specific flags */
uint8_t nsCnt = 0u; /* Count of the notifications */
cy_stc_ble_ancs_ns_t ns[CY_BLE_ANCS_NS_CNT]; /* array of the received notifications */
cy_stc_ble_ancs_cp_t cp; /* Control point value */
cy_stc_ble_ancs_ds_t ds; /* Data source value */


/*******************************************************************************
* Function Name: AncsPrintCharName
********************************************************************************
*
* Summary:
*   Decodes and prints the Apple Notification Center service characteristic name.
*
* Parameters:
*   charIndex - ANCS characteristic index.
*
********************************************************************************/
void AncsPrintCharName(cy_en_ble_ancs_char_index_t charIndex)
{
    switch(charIndex)
    {
        case CY_BLE_ANCS_NS:                    
            DBG_PRINTF("Notification Source");
            break;
        
        case CY_BLE_ANCS_CP:                    
            DBG_PRINTF("Control Point");
            break;
            
        case CY_BLE_ANCS_DS:
            DBG_PRINTF("Data Source");
            break;
            
        default:
            DBG_PRINTF("Unknown ANCS");
            break;
    }
    
    DBG_PRINTF(" characteristic ");
}


/*******************************************************************************
* Function Name: AncsNextAct
********************************************************************************
*
* Summary:
*   Initiates the next action processing.
*
*******************************************************************************/
void AncsNextAct(void)
{
    DBG_PRINTF("\r\n\n");
    
    switch(cp.attId)
    {
        case CY_BLE_ANCS_CP_ATT_ID_TTL: /* If previous Attribute ID is "Title" */
            switch(cp.ctgId)
            {
                case CY_BLE_ANCS_NS_CAT_ID_EML: /* If Category ID is "Email" */
                    cp.attId = CY_BLE_ANCS_CP_ATT_ID_SBT; /* Next Attribute ID is "Subtitle" */
                    ancsFlag |= CY_BLE_ANCS_FLG_CMD; /* Trig control point command write */
                    break;
                
                case CY_BLE_ANCS_NS_CAT_ID_SOC: /* If Category ID is "Social" */
                    cp.attId = CY_BLE_ANCS_CP_ATT_ID_MSG; /* Next Attribute ID is "Message" */
                    ancsFlag |= CY_BLE_ANCS_FLG_CMD; /* Trig control point command write */
                    break;
                
                case CY_BLE_ANCS_NS_CAT_ID_INC: /* If Category ID is "Incoming Call" */
                    if(((ns[nsCnt].evtFlg & CY_BLE_ANCS_NS_FLG_PA) != 0u) &&
                       ((ns[nsCnt].evtFlg & CY_BLE_ANCS_NS_FLG_NA) != 0u))
                    { /* If "Positive Action" and "Negative Action" flags are set */
                        DBG_PRINTF("User Action: Accept  (two pressing SW2 per second)\r\n");
                        DBG_PRINTF("             Decline (one pressing SW2 per second)? : \r\n");
                        ancsFlag |= CY_BLE_ANCS_FLG_ACT; /* Trig polling/waiting for user action */
                    }
                    break;
                
                default:
                    break;
            }
            break;
        
        case CY_BLE_ANCS_CP_ATT_ID_SBT: /* If previous Attribute ID is "Subtitle" */
            if(cp.ctgId == CY_BLE_ANCS_NS_CAT_ID_EML) /* If Category ID is "Email" */
            {
                cp.attId = CY_BLE_ANCS_CP_ATT_ID_MSG; /* Next Attribute ID is "Message" */
                ancsFlag |= CY_BLE_ANCS_FLG_CMD; /* Trig control point command write */
            }
            break;
            
        case CY_BLE_ANCS_CP_ATT_ID_MSG: /* If previous Attribute ID is "Message" */
            switch(cp.ctgId)
            {
                case CY_BLE_ANCS_NS_CAT_ID_EML: /* If Category ID is "Email" */
                    ancsFlag &= (uint8_t)~CY_BLE_ANCS_FLG_NTF; /* Stop to process current Notification */
                    break;
                
                case CY_BLE_ANCS_NS_CAT_ID_SOC: /* If Category ID is "social" */
                    if((ns[nsCnt].evtFlg & CY_BLE_ANCS_NS_FLG_NA) != 0u)
                    { /* If "negative action" flag is set */
                        DBG_PRINTF("User Action: Decline (one pressing SW2 per second)? : ");
                        ancsFlag |= CY_BLE_ANCS_FLG_ACT; /* Trig polling/waiting for user action */
                    }
                    break;
                
                default:
                    break;
            }
            break;
            
        default:
            ancsFlag &= (uint8_t)~CY_BLE_ANCS_FLG_NTF; /* Stop to process current Notification */
            break;
    }
}


/*******************************************************************************
* Function Name: AncsCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service-specific events for
*   Apple Notification Center Service.
*
* Parameters:
*  event - The event code.
*  eventParam - The pointer to the event data.
*
********************************************************************************/
void AncsCallBack(uint32_t event, void* eventParam)
{   
    uint16_t i;
    cy_en_ble_api_result_t apiResult;
    
    switch(event)
    {
        case CY_BLE_EVT_ANCSC_NOTIFICATION:  
        #if(DEBUG_UART_FULL == ENABLED)
            DBG_PRINTF("CY_BLE_EVT_ANCSC_NOTIFICATION \r\n");
            ShowValue(((cy_stc_ble_ancs_char_value_t *)eventParam)->value);
        #endif /* if(DEBUG_UART_FULL == ENABLED) */
            switch(((cy_stc_ble_ancs_char_value_t *)eventParam)->charIndex)
            {
                case CY_BLE_ANCS_NS:
                    {
                        uint8_t locFlag = 0u;
                        
                        /* Unpack the Notification ID */
                        uint32_t locNtfUid = ((uint32)((cy_stc_ble_ancs_char_value_t *)eventParam)->value->val[4u]) +
                                         ((uint32)((cy_stc_ble_ancs_char_value_t *)eventParam)->value->val[5u] << 8u) +
                                         ((uint32)((cy_stc_ble_ancs_char_value_t *)eventParam)->value->val[6u] << 16u) +
                                         ((uint32)((cy_stc_ble_ancs_char_value_t *)eventParam)->value->val[7u] << 24u);
                                      
                        if((ancsFlag & CY_BLE_ANCS_FLG_ACT) != 0u)
                        {
                            /* Discard pending User Action and current notification processing */
                            ancsFlag &= (uint8_t)~(CY_BLE_ANCS_FLG_ACT | CY_BLE_ANCS_FLG_NTF);
                            DBG_PRINTF("User Action is discarded \r\n");
                        }
                        
                        /* Check if this Notification ID is already stored */
                        for(i = 0u; i < nsCnt; i++)
                        {
                            if(locNtfUid == ns[i].ntfUid)
                            {
                                locFlag = 1u;
                            }
                        }
                        
                        if(locFlag == 0u) /* If it is "new" Notification */
                        {   
                            /* Unpack all Notification Source characteristic fields */
                            ns[nsCnt].evtId = (cy_en_ble_ancs_ns_evt_id_t)((cy_stc_ble_ancs_char_value_t *)eventParam)->value->val[0u];
                            ns[nsCnt].evtFlg = ((cy_stc_ble_ancs_char_value_t *)eventParam)->value->val[1u];
                            ns[nsCnt].ctgId = (cy_en_ble_ancs_ns_cat_id_t)((cy_stc_ble_ancs_char_value_t *)eventParam)->value->val[2u];
                            ns[nsCnt].ctgCnt = ((cy_stc_ble_ancs_char_value_t *)eventParam)->value->val[3u];
                            ns[nsCnt].ntfUid = locNtfUid;
                            
                            /* Print all details of Notification */
                            DBG_PRINTF("\r\nEventID: ");
                            switch(ns[nsCnt].evtId)
                            {
                                case CY_BLE_ANCS_NS_EVT_ID_ADD:
                                    DBG_PRINTF("Notification Added\r\n");
                                    break;
                                
                                case CY_BLE_ANCS_NS_EVT_ID_MOD:
                                    DBG_PRINTF("Notification Modified\r\n");
                                    break;
                                
                                case CY_BLE_ANCS_NS_EVT_ID_REM:
                                    DBG_PRINTF("Notification Removed\r\n");
                                    break;
                                
                                default:
                                    DBG_PRINTF("Unsupported\r\n");
                                    break;
                            }
                            
                            DBG_PRINTF("EventFlags: ");
                            if((ns[nsCnt].evtFlg & CY_BLE_ANCS_NS_FLG_SL) != 0u)
                            {
                                DBG_PRINTF("Silent, ");
                            }
                            
                            if((ns[nsCnt].evtFlg & CY_BLE_ANCS_NS_FLG_IM) != 0u)
                            {
                                DBG_PRINTF("Important, ");
                            }
                            
                            if((ns[nsCnt].evtFlg & CY_BLE_ANCS_NS_FLG_PE) != 0u)
                            {
                                DBG_PRINTF("Pre-existing, ");
                            }
                            
                            if((ns[nsCnt].evtFlg & CY_BLE_ANCS_NS_FLG_PA) != 0u)
                            {
                                DBG_PRINTF("Positive Action, ");
                            }
                            
                            if((ns[nsCnt].evtFlg & CY_BLE_ANCS_NS_FLG_NA) != 0u)
                            {
                                DBG_PRINTF("Negative Action, ");
                            }
                            DBG_PRINTF("\r\n");
                            
                            DBG_PRINTF("CategoryCount: %d\r\n", ns[nsCnt].ctgCnt);
                            DBG_PRINTF("NotificationUID: 0x%8.8lx\r\n", ns[nsCnt].ntfUid);
                            
                            DBG_PRINTF("CategoryID: ");
                            switch(ns[nsCnt].ctgId)
                            {
                                case CY_BLE_ANCS_NS_CAT_ID_OTH: /* If Category ID is "Other" */
                                    DBG_PRINTF("Other\r\n");
                                    break;
                                    
                                case CY_BLE_ANCS_NS_CAT_ID_INC: /* If Category ID is "Incoming Call" */
                                    DBG_PRINTF("Incoming Call\r\n");
                                    if(ns[nsCnt].evtId == CY_BLE_ANCS_NS_EVT_ID_ADD) /* If Event ID is "Notification Added" */
                                    { 
                                        if(nsCnt < (CY_BLE_ANCS_NS_CNT - 1)) /* If notification queue is not full */
                                        {
                                            nsCnt++; /* Increment notification counter */
                                        }
                                    }
                                    break;
                                
                                case CY_BLE_ANCS_NS_CAT_ID_MIS: /* If Category ID is "Missed Call" */
                                    DBG_PRINTF("Missed Call\r\n");
                                    break;
                                
                                case CY_BLE_ANCS_NS_CAT_ID_VML: /* If Category ID is "Voice mail" */
                                    DBG_PRINTF("Voicemail\r\n");
                                    break;
                                
                                case CY_BLE_ANCS_NS_CAT_ID_SOC: /* If Category ID is "Social" */
                                    DBG_PRINTF("Social\r\n");
                                    if(ns[nsCnt].evtId == CY_BLE_ANCS_NS_EVT_ID_ADD) /* If Event ID is "Notification Added" */
                                    {
                                        if(nsCnt < (CY_BLE_ANCS_NS_CNT - 1)) /* If notification queue is not full */
                                        {
                                            nsCnt++; /* Increment notification counter */
                                        }
                                    }
                                    break;
                                
                                case CY_BLE_ANCS_NS_CAT_ID_SCH: /* If the Category ID is "Schedule" */
                                    DBG_PRINTF("Schedule\r\n");
                                    break;
                                
                                case CY_BLE_ANCS_NS_CAT_ID_EML: /* If Category ID is "Email" */
                                    DBG_PRINTF("Email\r\n");
                                    if(ns[nsCnt].evtId == CY_BLE_ANCS_NS_EVT_ID_ADD) /* If Event ID is "Notification Added" */
                                    {
                                        if(nsCnt < (CY_BLE_ANCS_NS_CNT - 1)) /* If notification queue is not full */
                                        {
                                            nsCnt++; /* Increment notification counter */
                                        }
                                    }
                                    break;
                                
                                case CY_BLE_ANCS_NS_CAT_ID_NWS: /* If Category ID is "News" */
                                    DBG_PRINTF("News\r\n");
                                    break;
                                
                                case CY_BLE_ANCS_NS_CAT_ID_HNF: /* If Category ID is "Health and Fitness" */
                                    DBG_PRINTF("Health and Fitness\r\n");
                                    break;
                                
                                case CY_BLE_ANCS_NS_CAT_ID_BNF: /* If Category ID is "Business and Finance" */
                                    DBG_PRINTF("Business and Finance\r\n");
                                    break;
                                
                                case CY_BLE_ANCS_NS_CAT_ID_LOC: /* If Category ID is "Location" */
                                    DBG_PRINTF("Location\r\n");
                                    break;
                                
                                case CY_BLE_ANCS_NS_CAT_ID_ENT: /* If Category ID is "Entertainment" */
                                    DBG_PRINTF("Entertainment\r\n");
                                    break;
                                
                                default:
                                    DBG_PRINTF("Unsupported\r\n");
                                    break;
                            }
                        }
                    }
                    break;
            
                case CY_BLE_ANCS_DS:
                    {
                        uint8_t locLength = ((cy_stc_ble_ancs_char_value_t *)eventParam)->value->len;
                        uint8_t* locData = ((cy_stc_ble_ancs_char_value_t *)eventParam)->value->val;
                        
                        if((ancsFlag & CY_BLE_ANCS_FLG_STR) != 0u) /* If there is unfinished string */
                        {
                            for(i = 0u; i < locLength; i++)
                            {
                                UART_DEB_PUT_CHAR(locData[i]); /* Print data */
                            }
                            
                            ds.currLength += (uint16_t)locLength; /* Update current length */
                            
                            if(ds.currLength == ds.length) /* If data is complete */
                            {
                                ancsFlag &= (uint8_t)~CY_BLE_ANCS_FLG_STR; /* Finish the string */
                                AncsNextAct(); /* Perform next action */
                            }
                        }
                        else 
                        {
                            locLength -= 8u; /* Take into account only data length */
                            
                            ds.cmdId = (cy_en_ble_ancs_cp_cmd_id_t)locData[0u]; /* Unpack Command ID */
                            /* Unpack Notification UID */
                            ds.ntfUid = ((uint32)locData[1u]) +
                                        ((uint32)locData[2u] << 8u) +
                                        ((uint32)locData[3u] << 16u) +
                                        ((uint32)locData[4u] << 24u);
                            ds.attId = (cy_en_ble_ancs_cp_att_id_t)locData[5u]; /* Unpack Attribute ID */
                             
                            Cy_BLE_Set16ByPtr((uint8_t*)&ds.length,
                            Cy_BLE_Get16ByPtr(&locData[6u])); /* Unpack data length */
                            
                            ds.data = &locData[8u]; /* Extract data pointer */
                            
                            if((ds.cmdId == CY_BLE_ANCS_CP_CMD_ID_GNA) && (memcmp(&ds.ntfUid, &cp.ntfUid, 4u) == 0))
                            { /* If Command ID is "Get Notification Attributes" and Notification UID matches */
                                switch(ds.attId)
                                {
                                    case CY_BLE_ANCS_CP_ATT_ID_TTL: /* If Attribute ID is "Title" */
                                        switch(cp.ctgId)
                                        {
                                            case CY_BLE_ANCS_NS_CAT_ID_EML: /* If Category ID is "Email" */
                                                DBG_PRINTF("\r\nEmail from: \n");
                                                break;
                                                
                                            case CY_BLE_ANCS_NS_CAT_ID_INC: /* If Category ID is "Incoming Call" */
                                                DBG_PRINTF("\r\nIncoming Call from:  \n");
                                                break;
                                                
                                            case CY_BLE_ANCS_NS_CAT_ID_SOC: /* If Category ID is "Social" */
                                                DBG_PRINTF("\r\nApp: \n");
                                                break;
                                                
                                            default:
                                                DBG_PRINTF("\r\nTitle: \n");
                                                break;
                                        }
                                        break;
                                        
                                    case CY_BLE_ANCS_CP_ATT_ID_SBT: /* If Attribute ID is "Subtitle" */
                                        DBG_PRINTF("Subject: \n");
                                        break;
                                        
                                    case CY_BLE_ANCS_CP_ATT_ID_MSG: /* If Attribute ID is "Message" */
                                        DBG_PRINTF("Message: \n");
                                        break;
                                        
                                    default:
                                        break;
                                }
                                
                                switch(ds.attId)
                                {
                                    case CY_BLE_ANCS_CP_ATT_ID_TTL: /* If Attribute ID is "Title" */
                                    case CY_BLE_ANCS_CP_ATT_ID_SBT: /* If Attribute ID is "Subtitle" */
                                    case CY_BLE_ANCS_CP_ATT_ID_MSG: /* If Attribute ID is "Message" */
                                        for(i = 0u; i < locLength; i++)
                                        {
                                            UART_DEB_PUT_CHAR(ds.data[i]); /* Print data */
                                        }
                                        
                                        if(ds.length > locLength)
                                        { /* Indicate that data is not full, 
                                            waiting for next notification with rest of data */
                                            ancsFlag |= CY_BLE_ANCS_FLG_STR; 
                                            ds.currLength = locLength; /* Initialize current length */
                                        }
                                        else
                                        { /* Otherwise perform next action */
                                            AncsNextAct();
                                        }
                                        break;
                                    
                                    default:
                                        break;
                                }
                            }   
                        }
                    }
                    break;
                    
                default:
                    break;
            }
            break;
        
        case CY_BLE_EVT_ANCSC_WRITE_CHAR_RESPONSE:
            ancsFlag &= (uint8_t)~CY_BLE_ANCS_FLG_RSP;
            break;
                
        case CY_BLE_EVT_ANCSC_WRITE_DESCR_RESPONSE:
            if(((cy_stc_ble_ancs_char_value_t *)eventParam)->charIndex == CY_BLE_ANCS_NS) /* If NS notification is enabled */
            {
                uint16_t cccd;
                /* Enable Data Source notification */
                cccd = CY_BLE_CCCD_NOTIFICATION;
                apiResult = Cy_BLE_ANCSC_SetCharacteristicDescriptor(appConnHandle, CY_BLE_ANCS_DS, CY_BLE_ANCS_CCCD, 
                    CY_BLE_CCCD_LEN, (uint8_t *)&cccd);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_ANCSC_SetCharacteristicDescriptor(DS) API Error: ");
                    PrintApiResult(apiResult);
                }
                else
                {
                    AncsPrintCharName(CY_BLE_ANCS_DS);
                    DBG_PRINTF("CCCD write request: 0x%2.2x\r\n", cccd);
                }
            }
            break;
            
        case CY_BLE_EVT_ANCSC_ERROR_RESPONSE:
            ancsFlag &= (uint8_t)~CY_BLE_ANCS_FLG_RSP;
            break;
        
        default: /* Print unknown event number */
            DBG_PRINTF("Unknown ANCS event: 0x%lx\r\n", event);
            break;
    }
}


/*******************************************************************************
* Function Name: AncsAct
********************************************************************************
*
* Summary:
*   Initiates the ANCS action.
*
* Parameters:
*   actId - The ANCS action identifier.
*
*******************************************************************************/
void AncsAct(cy_en_ble_ancs_cp_act_id_t actId)
{
    cp.ntfUid = ns[nsCnt].ntfUid; /* Initialize control point Notification UID */
    cp.ctgId = ns[nsCnt].ctgId; /* Initialize control point Category ID */
    cp.cmdId = CY_BLE_ANCS_CP_CMD_ID_PNA; /* Set control point Command ID to "Perform Notification Action" */
    cp.actId = actId; /* Initialize control point Action ID */
    ancsFlag |= CY_BLE_ANCS_FLG_CMD; /* Trig control point command write */
    ancsFlag &= (uint8_t)~(CY_BLE_ANCS_FLG_ACT | CY_BLE_ANCS_FLG_NTF); /* Clear Action and Notification flags */
    Ringing_LED_Write(LED_OFF); /* Turn off ringing LED */
    if(actId == CY_BLE_ANCS_CP_ACT_ID_POS)
    { /* If Positive action */
        DBG_PRINTF("Accepted.\r\n");
    }
    else
    { /* Else Negative action */
        DBG_PRINTF("Declined.\r\n");
    }
}


/*******************************************************************************
* Function Name: AncsProcess
********************************************************************************
*
* Summary:
*   Processes the Ansc functionality in the main loop.
*
*******************************************************************************/
void AncsProcess(void)
{
    cy_en_ble_api_result_t apiResult;

    if(Cy_BLE_GATT_GetBusyStatus(appConnHandle.attId) != CY_BLE_STACK_STATE_BUSY)
    {
        if((ancsFlag & CY_BLE_ANCS_FLG_ACT) != 0u)
        {
            /* Button polling */
            if((button & SW2_READY) != 0u)
            {
                switch(button & SW2_PRESS_MASK)
                {
                    case SW2_ONE_PRESSING:
                        if((ns[nsCnt].evtFlg & CY_BLE_ANCS_NS_FLG_NA) != 0u)
                        {
                            AncsAct(CY_BLE_ANCS_CP_ACT_ID_NEG); /* Trig negative action */
                        }
                        break;
                        
                    case SW2_TWO_PRESSING:
                        if((ns[nsCnt].evtFlg & CY_BLE_ANCS_NS_FLG_PA) != 0u)
                        {
                            AncsAct(CY_BLE_ANCS_CP_ACT_ID_POS); /* Trig positive action */
                        }
                        break;
                        
                    default:
                        break;
                }
                
                button = SW2_ZERO_PRESSING;
            }
        }
        else if((nsCnt > 0u) && ((ancsFlag & CY_BLE_ANCS_FLG_NTF) == 0u))
        { /* If there are pending Notifications and current Notification is already processed */
            nsCnt--; /* Decrement Notification counter */
            cp.ntfUid = ns[nsCnt].ntfUid; /* Initialize control point Notification UID */
            cp.ctgId = ns[nsCnt].ctgId; /* Initialize control point Category ID */
            cp.cmdId = CY_BLE_ANCS_CP_CMD_ID_GNA; /* Set control point Command ID to "Get Notification Attributes" */
            cp.attId = CY_BLE_ANCS_CP_ATT_ID_TTL; /* Set control point Attribute ID to "Title" */
            ancsFlag |= CY_BLE_ANCS_FLG_NTF | CY_BLE_ANCS_FLG_CMD; /* Trig next Notification process */
        }
        else if((ancsFlag & CY_BLE_ANCS_FLG_START) != 0u)
        { 
            /* Enable Source characteristic notification */
            uint16_t cccd = CY_BLE_CCCD_NOTIFICATION;
            
            apiResult = Cy_BLE_ANCSC_SetCharacteristicDescriptor(appConnHandle, CY_BLE_ANCS_NS, CY_BLE_ANCS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_ANCSC_SetCharacteristicDescriptor(NS) API Error: ");
                PrintApiResult(apiResult);
            }
            else
            {
                AncsPrintCharName(CY_BLE_ANCS_NS);
                DBG_PRINTF("CCCD write request: 0x%2.2x\r\n", cccd);
            }
            ancsFlag &= (uint8_t)~CY_BLE_ANCS_FLG_START;
        }
        else
        {
        }
        
        if(((ancsFlag & CY_BLE_ANCS_FLG_CMD) != 0u) && ((ancsFlag & CY_BLE_ANCS_FLG_RSP) == 0u))
        { /* If there is pending command */
            uint8_t attr[8u];
            uint8_t length = 0u;
            uint8_t i;
            
            ancsFlag &= (uint8_t)~CY_BLE_ANCS_FLG_CMD;
            
            attr[0u] = cp.cmdId; /* Pack Command ID */
            for(i = 0u; i < 4u; i++)
            {
                attr[i + 1u] = (uint8_t)((cp.ntfUid >> (i << 3u)) & 0x000000FFu);
            } /* Pack Notification UID */
            
            switch(cp.cmdId)
            {
                case CY_BLE_ANCS_CP_CMD_ID_GNA: /* If Command ID is "Get Notification Attributes" */
                    attr[5u] = (uint8_t)cp.attId; /* Pack Attribute ID */
                    Cy_BLE_Set16ByPtr(&attr[6u], CY_BLE_ANCS_MAX_STR_LENGTH); /* Pack maximum string length */
                    length = 8u; /* Length of this command */
                    break;
                
                case CY_BLE_ANCS_CP_CMD_ID_PNA: /* If Command ID is "Perform Notification Action" */
                    attr[5u] = (uint8_t)cp.actId; /* Pack Action ID */
                    length = 6u; /* Length of this command */
                    break;
                
                default:
                    break;
            }
            
            if(length != 0u)
            { /* If there is packed command */
                (void) Cy_BLE_ANCSC_SetCharacteristicValue(appConnHandle, CY_BLE_ANCS_CP, length, attr);
                ancsFlag |= CY_BLE_ANCS_FLG_RSP;
            }
        }
    }
}


/* [] END OF FILE */
