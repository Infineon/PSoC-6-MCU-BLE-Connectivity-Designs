/*******************************************************************************
* File Name: ipss.h
*
*  Version: 1.0
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project.
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(IPSS_H)
#define IPSS_H

#include <project.h>
#include <stdio.h>

/***************************************
*           API Constants
***************************************/
   
#define MODE_LATITUDE_INC   (0u)
#define MODE_LATITUDE_DEC   (1u)
#define MODE_LONGITUDE_INC  (2u)
#define MODE_LONGITUDE_DEC  (3u)
    
/* Coordinates of Cypress Ukrainian office location */ 
    
#define CYUA_LATITUDE       (1188484261u) 
#define CYUA_LONGITUDE      (286826267u)

/***************************************
*      Function prototypes
***************************************/

void IpsInit(void);
void IpsSimulateCoordinates(void);
void IpsStartAdvertisement(void);
void IpsCallBack(uint32_t event, void* eventParam);
void IpsPrintCharName(cy_en_ble_ips_char_index_t charIndex);
void IpsSetUpdateAdvDataCompeteEventFlag(void);
bool IpsGetSwitchAdvModeFlag(void);
void IpsSetSwitchAdvModeFlag(bool switchAdvModeFlagValue);

#endif /* IPSS_H  */

/* [] END OF FILE */
