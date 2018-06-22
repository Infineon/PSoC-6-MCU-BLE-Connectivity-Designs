/*******************************************************************************
* File Name: passs.h
*
* Version 1.0
*
* Description:
*  PASS service related code header.
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

#if !defined(PASSS_H)
#define PASSS_H

#include <project.h>
#include "common.h"


/*******************************************************************************
*        API Constants
*******************************************************************************/

    
#define ALERT_STATUS_NO_ALERT           (0x00u)
#define ALERT_STATUS_RINGER             (0x01u)
#define ALERT_STATUS_VIBRATE            (0x02u)
#define ALERT_STATUS_VIBRATE_RINGER     (0x03u)
    
#define FLAG_AS                         (0x01u) /* Alert Status change */
#define FLAG_RS                         (0x02u) /* Ringer Setting change */

/***************************************
*        Function Prototypes
***************************************/
void  PassInit(void);
void  SetAS(void);
void  SetRS(void);
void  PassProcess(void);
void  PassCallBack(uint32_t event, void* eventParam);


/***************************************
*      External variables
***************************************/
extern uint8_t alertStatus;
extern cy_en_ble_pass_rs_t ringerSetting;


#endif /* PASSS_H  */

/* [] END OF FILE */
