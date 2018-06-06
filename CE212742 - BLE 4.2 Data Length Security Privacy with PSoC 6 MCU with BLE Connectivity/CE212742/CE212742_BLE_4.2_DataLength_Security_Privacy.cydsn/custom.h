/*******************************************************************************
* File Name: custom.h
*
*  Version: 1.0
*
* Description:
*  Contains the function prototypes, constants and structure definitions 
*  available to the example project.
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CUSTOM_H)
#define CUSTOM_H

#include <project.h>

/***************************************
*      Data Struct Definition 
***************************************/
typedef struct
{
    uint8_t mode;                         /* bit0: General or Limited TX operation
                                             bit1: Undirected of Directed advertising */
    uint32_t dataLength;                  /* Data Length for limited mode */
}__PACKED cy_stc_ble_custom_config_char_t;


/***************************************
*          Constants
***************************************/
#define CY_BLE_CUSTOM_MODE_LIMITED                          (1u)
#define CY_BLE_CUSTOM_MODE_DIRECTED_ADV                     (2u)


/***************************************
*       Function Prototypes
***************************************/
int HostMain(void);
void CustomInit(void);
void CustomCallBack(uint32_t event, void *eventParam);
void CustomSendData(void);


/***************************************
*       Macro
***************************************/

/***************************************
* External data references
***************************************/
extern uint16_t dataSendEnabled;
extern uint16_t dataSendCccd;
extern uint8_t dataSendConfirmed;
extern cy_stc_ble_custom_config_char_t customConfiguration;
extern uint16_t dataSendReduceLength;

#endif /* CUSTOM_H  */


/* [] END OF FILE */

