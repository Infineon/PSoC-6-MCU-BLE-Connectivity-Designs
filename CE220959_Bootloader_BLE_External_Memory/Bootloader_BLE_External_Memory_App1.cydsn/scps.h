/*******************************************************************************
* File Name: scps.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants available for SCPS
*
********************************************************************************
* Copyright 2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>


/***************************************
*       Function Prototypes
***************************************/
void ScpsInit(void);
void ScpsCallBack (uint32_t event, void *eventParam);
void ScpsSendReqUpdateConnParam(cy_stc_ble_conn_handle_t connHandle);

/* [] END OF FILE */
