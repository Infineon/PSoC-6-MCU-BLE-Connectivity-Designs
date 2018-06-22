/*******************************************************************************
* File Name: cts.c
*
* Version: 1.0
*
* Description:
*  This file contains CTS callback handler function.
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
#include "common.h"
#include "cts.h"
#include "user_interface.h"


/***************************************
*        Global Variables
***************************************/
static cy_stc_ble_cts_current_time_t currentTime;
static cy_stc_ble_cts_local_time_info_t localTimeInfo;
static cy_stc_ble_cts_reference_time_info_t referenceTime;


/*******************************************************************************
* Function Name: CtsCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE Component,
*   which are specific to Current Time Service.
*
* Parameters:  
*   event:       Event for Current Time Service.
*   eventParams: Event param for Current Time Service.
*
*******************************************************************************/
void CtsCallBack(uint32_t event, void *eventParam)
{
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_cts_char_value_t *timeAttribute;
    cy_stc_ble_cts_char_value_t *ntfValParam;
    
    /* This is a CTS specific event triggered by the BLE component */
    switch(event)
    {
    /* Only operation that is required for RTC is reading the time. We are not enabling notifications
    * right now.
    */
    case CY_BLE_EVT_CTSC_READ_CHAR_RESPONSE:
    
        timeAttribute = (cy_stc_ble_cts_char_value_t *) eventParam;
        
        DBG_PRINTF("CTS characteristic read response received\r\n");
        
        if(timeAttribute->charIndex == CY_BLE_CTS_CURRENT_TIME)
        {
            /* Copy the received current time from the time server to local data structure */
            memcpy(&currentTime, timeAttribute->value->val, timeAttribute->value->len);
            CtsTimeUpdate(&currentTime);
        }
        else if(timeAttribute->charIndex == CY_BLE_CTS_LOCAL_TIME_INFO)
        {
            /* Copy the received local time info from the time server to local data structure */
            memcpy(&localTimeInfo, timeAttribute->value->val, timeAttribute->value->len);
            
            DBG_PRINTF("Local time:\r\nTime Zone : %d\r\nDST : %d\r\n", localTimeInfo.timeZone, localTimeInfo.dst);
        }
        else if(timeAttribute->charIndex == CY_BLE_CTS_REFERENCE_TIME_INFO)
        {
            /* Copy the received reference time from the time server to local data structure */
            memcpy(&referenceTime, timeAttribute->value->val, timeAttribute->value->len);
            
            DBG_PRINTF("Reference time:\r\nTime source : %d\r\nTime Accuracy : %d\r\nDays since update : %d\r\nHours since update : %d\r\n",
                    referenceTime.timeSource, referenceTime.timeAccuracy, referenceTime.daysSinceUpdate, 
                    referenceTime.hoursSinseUpdate);
        }
        else
        {
            DBG_PRINTF("Unknown CTS characteristic \r\n");
        }
        break;
        
    case CY_BLE_EVT_CTSC_NOTIFICATION:
        
        ntfValParam = (cy_stc_ble_cts_char_value_t *) eventParam;
        
        DBG_PRINTF("Current Time Characteristic notification received\r\n");
        /* Copy the current time received from the time server to local data structure  */
        memcpy(&currentTime, ntfValParam->value->val, ntfValParam->value->len);
        CtsTimeUpdate(&currentTime);
        break;
            
    case CY_BLE_EVT_CTSC_WRITE_DESCR_RESPONSE:
        DBG_PRINTF("CTS Current Time CCCD was written successfully\r\n");
        /* Send Read request for Current Time characteristic to the Time Server */
        apiResult = Cy_BLE_CTSC_GetCharacteristicValue(appConnHandle, CY_BLE_CTS_CURRENT_TIME);
        DBG_PRINTF("Get Current Time char value, apiResult: 0x%x \r\n", apiResult);
        break;

    /***************************************
    *        CTS Server events
    ***************************************/
    case CY_BLE_EVT_CTSS_NOTIFICATION_ENABLED:
        break;
    case CY_BLE_EVT_CTSS_NOTIFICATION_DISABLED:
        break;

    default:
        DBG_PRINTF("Unknown CTS event received\r\n");
        break;
    }
}


/*******************************************************************************
* Function Name: CtsInit
********************************************************************************
*
* Summary:
*   Initializes the CTS application global variables.
*
*******************************************************************************/
void CtsInit(void)
{
    /* Register the event handler for CTS specific events */
    Cy_BLE_CTS_RegisterAttrCallback(CtsCallBack);
}


/*******************************************************************************
* Function Name: CtsPrintCurrentTime
********************************************************************************
*
* Summary:
*   Displays the current time information on the terminal.
*
*******************************************************************************/
void CtsPrintCurrentTime(void)
{
    cy_stc_rtc_config_t curTimeAndDate;
    
    Cy_RTC_GetDateAndTime(&curTimeAndDate);
    
    DBG_PRINTF("Current time: %2.2ld:%2.2ld:%2.2ld ", curTimeAndDate.hour, curTimeAndDate.min, curTimeAndDate.sec);
    
    switch(curTimeAndDate.dayOfWeek)
    {
    case CY_RTC_MONDAY:
        DBG_PRINTF("Mon ");
        break;
    case CY_RTC_TUESDAY:
        DBG_PRINTF("Tue ");
        break;
    case CY_RTC_WEDNESDAY:
        DBG_PRINTF("Wed ");
        break;
    case CY_RTC_THURSDAY:
        DBG_PRINTF("Thu");
        break;
    case CY_RTC_FRIDAY:
        DBG_PRINTF("Fri ");
        break;
    case CY_RTC_SATURDAY:
        DBG_PRINTF("Sat ");
        break;
     case CY_RTC_SUNDAY:
        DBG_PRINTF("Sun ");
        break;
    default:
        DBG_PRINTF("Unknown day of week ");
        break;
    }
    curTimeAndDate.year += CY_RTC_TWO_THOUSAND_YEARS;
    DBG_PRINTF(" %2.2ld.%2.2ld.%4.4ld\r\n", curTimeAndDate.month, curTimeAndDate.date, curTimeAndDate.year);
}


/*******************************************************************************
* Function Name: CtsTimeUpdate
********************************************************************************
*
* Summary:
*  Implements the function for a Real Time Clock syncronization.
*
* Parameters:
*   time - pointer to the time structure
*
*******************************************************************************/
void CtsTimeUpdate(cy_stc_ble_cts_current_time_t *time)
{
    cy_en_rtc_status_t rtcStatus;
    
    DBG_PRINTF("Server time: %2.2d:%2.2d:%2.2d ", time->hours, time->minutes, time->seconds);
    DBG_PRINTF(" %2.2d.%2.2d.%4.4ld\r\n", time->month, time->day, (uint32_t)time->yearHigh << 8u | time->yearLow);
    
    DBG_PRINTF("Update RTC time\r\n");
    rtcStatus = Cy_RTC_SetDateAndTimeDirect(time->seconds, time->minutes, time->hours,
        time->day, time->month, 
        ((uint32_t)time->yearHigh << 8u | time->yearLow) - CY_RTC_TWO_THOUSAND_YEARS);
    DBG_PRINTF("Cy_RTC_SetDateAndTimeDirect API status: %x \r\n", rtcStatus);
}

/* [] END OF FILE */
