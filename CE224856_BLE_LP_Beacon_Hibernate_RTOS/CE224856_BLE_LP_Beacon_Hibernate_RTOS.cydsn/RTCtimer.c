/*******************************************************************************
* File Name: BLEFindMe.c
*
* Version: 1.20
*
* Description: This file contains RTC related functions.
*
* Related Document: CE224856_BLE_Low_Power_Beacon_with_Hibernate_RTOS.pdf
* 
*******************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress’s integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death (“High Risk Product”). By
* including Cypress’s product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability. 
*******************************************************************************/

#include "hibernate_config.h"
#include "RTCtimer.h"

/* Macro used wait 3 LFCLK cycles for RTC operations */
#define WAIT_3LFCLKS    (92u)

/* Default time written to RTC at power-up or reset */
#define TIME_AT_RESET           (00u),   /* Seconds    */\
                                (00u),   /* Minutes    */\
                                (00u),   /* Hours      */\
                                (01u),   /* Date       */\
                                (01u),   /* Month      */\
                                (18u)    /* Year 20xx  */

/* Structure that enables alarm interrupts at 2 second intervals, i.e when the 
   seconds field updates to "02" */                                
cy_stc_rtc_alarm_t const alarm = 
{
    .sec            =   HIBERNATE_TIME_SEC,
    .secEn          =   CY_RTC_ALARM_ENABLE,
    .min            =   HIBERNATE_TIME_MIN,
    .minEn          =   CY_RTC_ALARM_DISABLE,
    .hour           =   HIBERNATE_TIME_HR,
    .hourEn         =   CY_RTC_ALARM_DISABLE,
    .dayOfWeek      =   01u,
    .dayOfWeekEn    =   CY_RTC_ALARM_DISABLE,
    .date           =   01u,
    .dateEn         =   CY_RTC_ALARM_DISABLE,
    .month          =   01u,
    .monthEn        =   CY_RTC_ALARM_DISABLE,
    .almEn          =   CY_RTC_ALARM_ENABLE
};


void RTCtimer_Init()
{
    /* Initialize the RTC */
    Cy_RTC_Init(&RTC_config);
    
    /* Initialize the RTC interrupt*/
    Cy_SysInt_Init(&RTC_RTC_IRQ_cfg, &RTC_Interrupt);
    
    /* Clear any pending interrupts */
    Cy_RTC_ClearInterrupt(CY_RTC_INTR_ALARM1);
    NVIC_ClearPendingIRQ(RTC_RTC_IRQ_cfg.intrSrc);
    
    /*Configures the source (Alarm1) that trigger the interrupts */
    Cy_RTC_SetInterruptMask(CY_RTC_INTR_ALARM1);

    /* Set the default date and time */
    Cy_RTC_SetDateAndTimeDirect(TIME_AT_RESET);
    
    /* Wait for 3 LFCLK cycles before the next RTC operation */
    Cy_SysLib_DelayUs(WAIT_3LFCLKS);
     
    /* Set alarm that generates interrupts at 2 second intervals */
    Cy_RTC_SetAlarmDateAndTime(&alarm, CY_RTC_ALARM_1);
}

/* [] END OF FILE */
