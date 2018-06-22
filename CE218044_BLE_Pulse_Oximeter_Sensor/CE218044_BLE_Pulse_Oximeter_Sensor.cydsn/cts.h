/*******************************************************************************
* File Name: cts.h
*
*  Version: 1.0
*
* Description:
*  Contains the function prototypes and constants used by Current Time Service.
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>


/***************************************
*        API Constants
***************************************/
/* Day of week values */
#define MONDAY                        (1u)
#define TUESDAY                       (2u)
#define WEDNESDAY                     (3u)
#define THURSDAY                      (4u)
#define FRIDAY                        (5u)
#define SATURDAY                      (6u)
#define SUNDAY                        (7u)

/* Initial current time values */
#define INIT_YEAR_HIGH                (0x07u)
#define INIT_YEAR_LOW                 (0x6Cu)
#define INIT_MONTH                    (1u)
#define INIT_DAY                      (1u)
#define INIT_HOURS                    (0u)
#define INIT_MINUTES                  (0u)
#define INIT_SECONDS                  (0u)

#define TEN                           (10u)

/***************************************
*        Function Prototypes
***************************************/
void CtsInit(void);
void CtsAppEventHandler(uint32_t event, void * eventParam);
void PrintCurrentTime(void);
void TimeUpdate(void);

/* [] END OF FILE */
