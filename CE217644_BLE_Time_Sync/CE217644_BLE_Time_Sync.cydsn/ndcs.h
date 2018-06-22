/*******************************************************************************
* File Name: ndcs.h
*
*  Version: 1.0
*
* Description:
*  Contains the function prototypes and constants used by Next DST Change 
*  Service.
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>


/***************************************
*        Function Prototypes
***************************************/
void NdcsInit(void);
void NdcsCallBack(uint32_t event, void * eventParam);


/* [] END OF FILE */
