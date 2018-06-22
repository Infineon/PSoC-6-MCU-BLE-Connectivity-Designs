/*******************************************************************************
* File Name: passc.h
*
* Version 1.0
*
* Description:
*  Phone Alert Status service related code header.
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(PASSC_H)
#define PASSC_H
    
#include <project.h>
#include "common.h"
    
/*******************************************************************************
* Enumerations
*******************************************************************************/
   
#define LED_BLINK_RATE              (3u)
#define BTN_TMR_MASK                (0x01u)

#define FLAG_DSCR                   (0x01u) /* set descriptor */
#define FLAG_RD                     (0x02u) /* read request */
#define FLAG_WR                     (0x04u) /* write request */
#define FLAG_DISCOVERY_COMPLETE     (0x08u) /* discovery complete */

#define ALERT_STATUS_NO_ALERT       (0x00u)
#define ALERT_STATUS_RINGER         (0x01u)
#define ALERT_STATUS_VIBRATE        (0x02u)
#define ALERT_STATUS_VIBRATE_RINGER (0x03u)

    
/***************************************
*      API Function Prototypes
***************************************/
void                PassCallBack(uint32_t event, void* eventParam);
void                PassInit(void);
void                PassProcess(void);
cy_en_ble_pass_rs_t PassGetRingerSetting(void);
void                PassSetControlPoint(cy_en_ble_pass_cp_t controlPointValue);
uint8_t             PassGetAlertStatus(void);
void                PassSetCharIndex(cy_en_ble_pass_char_index_t charIndexValue);
uint8_t             PassGetPassFlag(void);
void                PassSetPassFlag(uint8_t passFlagValue);

#endif /* PASSC_H  */

/* [] END OF FILE */
