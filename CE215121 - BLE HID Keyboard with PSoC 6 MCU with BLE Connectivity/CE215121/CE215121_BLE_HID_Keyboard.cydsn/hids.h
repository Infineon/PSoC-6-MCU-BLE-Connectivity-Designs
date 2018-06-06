/*******************************************************************************
* File Name: hids.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants available for HIDS.
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>


/***************************************
*          Constants
***************************************/

#define KEYBOARD_TIMEOUT            (1u)        /* Сounts in sec */

/* Keyboard scan codes for notification defined in section 
*  10 Keyboard/Keypad Page of HID Usage Tables spec ver 1.12 
*/
#define SIM_KEY_MIN                 (4u)        /* Minimum simulated key 'a' */
#define SIM_KEY_MAX                 (40u)       /* Maximum simulated key '0' */
#define KEYBOARD_JITTER_SIZE        (0u)
#define NUM_LOCK                    (0x53u)
#define CAPS_LOCK                   (0x39u)
#define SCROLL_LOCK                 (0x47u)

/* LED codes received from HOST through output report 
*  Defined in section 11 LED Page of HID Usage Tables spec ver 1.12 
*/
#define NUM_LOCK_LED                (0x01u)
#define CAPS_LOCK_LED               (0x02u)
#define SCROLL_LOCK_LED             (0x04u)
#define KEYBOARD_DATA_SIZE          (8u)


/***************************************
*       Function Prototypes
***************************************/
void HidsInit(void);
void HidsInitCccd(void);
void HidsCallBack(uint32_t event, void *eventParam);
bool HidsIsCapsLock(void);
void HidsSetCapsLockPress(void);
uint8_t HidsGetSuspendState(void);
void HidsSimulateKeyboard(cy_stc_ble_conn_handle_t connHandle);


/* [] END OF FILE */
