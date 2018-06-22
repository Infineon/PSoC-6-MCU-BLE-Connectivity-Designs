/*******************************************************************************
* File Name: hts.h
*
* Version 1.0
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

#include <project.h>


/***************************************
*          Constants
***************************************/

#define SIM_TEMPERATURE_MIN         (15)        /* Minimum simulated temperature measurement */
#define SIM_TEMPERATURE_MAX         (40)        /* Maximum simulated temperature measurement */
#define SIM_TEMPERATURE_INCREMENT   (1)         /* Value by which the temperature is incremented */                             

#define HTS_TEMP_DATA_MIN_SIZE      (5u)


/***************************************
*       Function Prototypes
***************************************/
void HtsCallBack(uint32_t event, void *eventParam);
void HtsMeasureTemperature(cy_stc_ble_conn_handle_t connHandle);
void HtsInit(void);

/* [] END OF FILE */
