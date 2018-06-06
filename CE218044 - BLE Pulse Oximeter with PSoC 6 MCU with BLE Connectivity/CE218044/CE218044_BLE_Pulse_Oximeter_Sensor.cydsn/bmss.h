/*******************************************************************************
* File Name: bmss.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants available for BMS.
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

#if !defined(BMSS_H)
#define BMSS_H

#include <project.h>

   
/***************************************
*        Constant definitions
***************************************/
#define CY_BLE_BMS_BMFT_SIZE        (3u)   
#define CY_BLE_BMS_AUTH_CODE_SIZE   (6u)
    
/***************************************
*            Data Types
***************************************/
/* The BMS Features characteristic stucture defines */
typedef struct
{
    /* Supported Features */
    union 
    {
        uint32_t value;
        struct  /* "Supported Features" bitfields (0 - NO SUPPORT, 1 - SUPPORT) */
        {                  
            bool DELC_ALL                : 1;    /* Delete Bond of current connection (BR/EDR and LE) supported */
            bool DELC_ALL_AUTH           : 1;    /* Authorization Code required for feature above */
            bool DELC_BR                 : 1;    /* Delete bond of current connection (BR/EDR transport only) supported */
            bool DELC_BR_AUTH            : 1;    /* Authorization Code required for feature above */
            bool DELC_LE                 : 1;    /* Delete bond of current connection (LE transport only) supported */
            bool DELC_LE_AUTH            : 1;    /* Authorization Code required for feature above */
            
            bool REMA_ALL                : 1;    /* Remove all bonds on server (BR/EDR and LE) supported */
            bool REMA_ALL_AUTH           : 1;    /* Authorization Code required for feature above */
            bool REMA_BR                 : 1;    /* Remove all bonds on server (BR/EDR transport only) supported */
            bool REMA_BR_AUTH            : 1;    /* Authorization Code required for feature above */
            bool REMA_LE                 : 1;    /* Remove all bonds on server (LE transport only) supported */
            bool REMA_LE_AUTH            : 1;    /* Authorization Code required for feature above */
            
            bool REMAB_ALL               : 1;    /* Remove all but the active bond on server (BR/EDR and LE) supported */
            bool REMAB_ALL_AUTH          : 1;    /* Authorization Code required for feature above */
            bool REMAB_BR                : 1;    /* Remove all but the active bond on server (BR/EDR transport only) supported */
            bool REMAB_BR_AUTH           : 1;    /* Authorization Code required for feature above */
            bool REMAB_LE                : 1;    /* Remove all but the active bond on server (LE transport only) supported */
            bool REMAB_LE_AUTH           : 1;    /* Authorization Code required for feature above */
            
            bool IDENTS                  : 1;    /* Identify yourself supported */
            bool FEATEXT                 : 1;    /* Feature Extension */
            bool RFFU                    : 1;    /* Reserved for future use */      
        }bit;
    }features; 
}cy_stc_ble_bmss_bmft_char_t;
    
    
/***************************************
*      API function prototypes
***************************************/
void BmsInit(void);
void BmsCallBack(uint32_t event, void* eventParam);
void BmsProcess(void);
bool BmsUnPackData(cy_en_ble_bms_char_index_t charIdx, uint8_t inDataSize, uint8_t *inData, void *outData);
                       
#endif /* BMSS_H  */

/* [] END OF FILE */
