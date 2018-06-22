/*******************************************************************************
* File Name: cts.c
*
* Version: 1.0
*
* Description:
*  This file contains the CTS related code.
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
* 
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>
#include <stdio.h>
#include "common.h"
#include "cts.h"


/***************************************
*        Global Variables
***************************************/
/* Initial Time equals to 00:00:00 am of 1.01.1900 */
cy_stc_ble_cts_current_time_t currentTime = {
            INIT_YEAR_LOW,
            INIT_YEAR_HIGH,
            INIT_MONTH,
            INIT_DAY,
            INIT_HOURS,
            INIT_MINUTES,
            INIT_SECONDS,
};
cy_stc_ble_cts_local_time_info_t localTime;
cy_stc_ble_cts_reference_time_info_t referenceTime;


/*******************************************************************************
* Function Name: CtsAppEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component,
*  which are specific to Current Time Service.
*
* Parameters:  
*  event:       Event for Current Time Service.
*  eventParams: Event param for Current Time Service.
*
*******************************************************************************/
void CtsAppEventHandler(uint32_t event, void *eventParam)
{
    cy_stc_ble_cts_char_value_t *timeAttribute;
    cy_stc_ble_gatt_value_t *timeValue;
    
    /* This is a CTS specific event triggered by the BLE component */
    switch(event)
    {

    /***************************************
    *        CTS Server events
    ***************************************/
    case CY_BLE_EVT_CTSS_WRITE_CHAR: 
        DBG_PRINTF("CY_BLE_EVT_CTSS_WRITE_CHAR\r\n");
        
        timeAttribute = (cy_stc_ble_cts_char_value_t *) eventParam;
      
        if(timeAttribute->charIndex == CY_BLE_CTS_CURRENT_TIME)
        {
            cy_en_rtc_status_t rtcStatus;
            cy_stc_rtc_config_t dateTime;
            uint32_t millennium;
         
            timeValue = timeAttribute->value;  
            
            DBG_PRINTF("timeValue->len %x [%x])\r\n",timeValue->len, sizeof(cy_stc_ble_cts_char_value_t));
            if(timeValue->len > sizeof(cy_stc_ble_cts_char_value_t))
            {  
                
                memcpy(&currentTime, timeValue->val, timeValue->len);
                               
                /* Get millenium part */
                millennium = (((uint16_t)currentTime.yearHigh)<< 8u | currentTime.yearLow) / 1000u;
                millennium *= 1000u;
                
                /* Set time */
                dateTime.hour = currentTime.hours;
                dateTime.min  = currentTime.minutes;
                dateTime.sec  = currentTime.seconds;
               
                /* Set date */
                dateTime.year      = (((uint16_t)currentTime.yearHigh) << 8u | currentTime.yearLow) - millennium;
                dateTime.month     = currentTime.month;
                dateTime.date      = currentTime.day;
                dateTime.dayOfWeek = currentTime.dayOfWeek;
                dateTime.hrFormat  = CY_RTC_12_HOURS;

                /* Update the date/time */
                rtcStatus = Cy_RTC_SetDateAndTime(&dateTime);
                if(rtcStatus == CY_RTC_SUCCESS)
                {
                    DBG_PRINTF("Data/time updated ");
                    PrintCurrentTime();
                }
                else
                {
                    DBG_PRINTF("rtcStatus %x \r\n", rtcStatus);
                }
            }
            else
            {
                DBG_PRINTF("Error: incorrect the length of input packet\r\n"); 
            }
         
        }
        
        break;    
        
    case CY_BLE_EVT_CTSS_NOTIFICATION_ENABLED:
        DBG_PRINTF("CY_BLE_EVT_CTSS_NOTIFICATION_ENABLED\r\n");
        
        break;
    case CY_BLE_EVT_CTSS_NOTIFICATION_DISABLED:
        DBG_PRINTF("CY_BLE_EVT_CTSS_NOTIFICATION_DISABLED\r\n");
        
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
*  Initializes the CTS application global variables.
*
*******************************************************************************/
void CtsInit(void)
{
    /* Register the event handler for CTS specific events */
    Cy_BLE_CTS_RegisterAttrCallback(CtsAppEventHandler);
}


/*******************************************************************************
* Function Name: PrintCurrentTime
********************************************************************************
*
* Summary:
*  Displays the current time information on the terminal.
*
*******************************************************************************/
void PrintCurrentTime(void)
{
    DBG_PRINTF("Current time: ");
    
    if(TEN > currentTime.hours)
    {
        DBG_PRINTF("0%d:", currentTime.hours);
    }
    else
    {
        DBG_PRINTF("%d:", currentTime.hours);
    }
    
    if(TEN > currentTime.minutes)
    {
        DBG_PRINTF("0%d:", currentTime.minutes);
    }
    else
    {
        DBG_PRINTF("%d:", currentTime.minutes);
    }
    
    if(TEN > currentTime.seconds)
    {
        DBG_PRINTF("0%d: ", currentTime.seconds);
    }
    else
    {
        DBG_PRINTF("%d: ", currentTime.seconds);
    }
    
    switch(currentTime.dayOfWeek)
    {
    case MONDAY:
        DBG_PRINTF("Mon ");
        break;
    case TUESDAY:
        DBG_PRINTF("Tue ");
        break;
    case WEDNESDAY:
        DBG_PRINTF("Wed ");
        break;
    case THURSDAY:
        DBG_PRINTF("Thu");
        break;
    case FRIDAY:
        DBG_PRINTF("Fri ");
        break;
    case SATURDAY:
        DBG_PRINTF("Sat ");
        break;
     case SUNDAY:
        DBG_PRINTF("Sun ");
        break;
    default:
        DBG_PRINTF("Unknown day of week ");
        break;
    }
    DBG_PRINTF(" %d.%d.%d\r\n", currentTime.month, currentTime.day, 
                            ((uint16_t)currentTime.yearHigh)<< 8u | currentTime.yearLow);
}

/*******************************************************************************
* Function Name: TimeUpdate
********************************************************************************
*
* Summary:
*  Implements the function of a Real Time Clock. Every second it updates the 
*  current time.
*  The example project doesn't update days month and years after time
*  syncronisation. For the simulation only seconds minutes and hour are handled.
*
*******************************************************************************/
void TimeUpdate(void)
{
    /* 'needUpdate' will be used to update hours */
    static bool needUpdate = false;
    
    /* Update seconds */
    if(CY_BLE_CTS_SECONDS_MAX == currentTime.seconds)
    {
        currentTime.seconds = 0u;
    }
    else
    {
        currentTime.seconds++;
    }
    
    /* Update minutes */
    if(currentTime.seconds == 0u)
    {
        if(CY_BLE_CTS_MINUTES_MAX == currentTime.minutes)
        {
            currentTime.minutes = 0u;
            needUpdate = true;
        }
        else
        {
            currentTime.minutes++;
        }
    }

    /* Update hours */
    if(needUpdate == true)
    {
        if(CY_BLE_CTS_HOURS_MAX == currentTime.hours)
        {
            currentTime.hours = 0u;
        }
        else
        {
            currentTime.hours++;
        }
        needUpdate = false;
    }
}


/* [] END OF FILE */
