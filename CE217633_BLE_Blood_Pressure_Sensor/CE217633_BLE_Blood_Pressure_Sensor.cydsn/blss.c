/*******************************************************************************
* File Name: blss.c
*
* Version 1.0
*
* Description:
*  This file contains BLS service related code.
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


#include "common.h"
#include "blss.h"

/* Static global variables */
static uint8_t blsSim;       /* Blood Pressure Measurement simulation counter */

/* Blood Pressure Measurement values */
static cy_stc_ble_bls_bpm_t blsBpm[] =
{
    {
        CY_BLE_BLS_BPM_FLG_TSP | CY_BLE_BLS_BPM_FLG_PRT | CY_BLE_BLS_BPM_FLG_UID | CY_BLE_BLS_BPM_FLG_MST,
        0x008au /* Systolic 138.0 mmHg */,
        0x004fu /* Diastolic 79.0 mmHg */,
        0x0050u /* MAP 80.0 mmHg */,
        {2014u, 9u, 8u, 13u, 20u, 45u},
        0xf321u /* 80.1 */,
        1u,
        CY_BLE_BLS_BPM_MST_BMD
    }
};

/* Intermediate Cuff Pressure values */
static cy_stc_ble_bls_bpm_t blsIcp[] =
{
    {   CY_BLE_BLS_BPM_FLG_TSP | CY_BLE_BLS_BPM_FLG_PRT | CY_BLE_BLS_BPM_FLG_UID | CY_BLE_BLS_BPM_FLG_MST,
        0x0091u /* 145.0 mmHg */,
        0x07ffu /* NaN */,
        0x07ffu /* NaN */,
        {2014u, 9u, 8u, 13u, 20u, 40u},
        0xf33au /* 82.6 */,
        1u,
        CY_BLE_BLS_BPM_MST_BMD
    }
};

/*******************************************************************************
* Function Name: BlsCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service specific events from
*   Blood Pressure Service.
*
* Parameters:
*  event - the event code
*  *eventParam - the event parameters
*
********************************************************************************/
void BlsCallBack(uint32_t event, void* eventParam)
{
    if(0u != eventParam)
    {
        /* This dummy operation is to avoid warning about unused eventParam */
    }
    
    DBG_PRINTF("BLS event: %lx, ", event);

    switch(event)
    {
        case CY_BLE_EVT_BLSS_NOTIFICATION_ENABLED:
            DBG_PRINTF("Intermediate Cuff Pressure Notification is Enabled \r\n");
            blsSim = 0u;
            break;

        case CY_BLE_EVT_BLSS_NOTIFICATION_DISABLED:
            DBG_PRINTF("Intermediate Cuff Pressure Notification is Disabled \r\n");
            break;

        case CY_BLE_EVT_BLSS_INDICATION_ENABLED:
            DBG_PRINTF("Blood Pressure Measurement Indication is Enabled \r\n");
            blsSim = 0u;
            break;

        case CY_BLE_EVT_BLSS_INDICATION_DISABLED:
            DBG_PRINTF("Blood Pressure Measurement Indication is Disabled \r\n");
            break;

        case CY_BLE_EVT_BLSS_INDICATION_CONFIRMED:
            DBG_PRINTF("Blood Pressure Measurement Indication is Confirmed \r\n");
            break;

        default:
            DBG_PRINTF("Unknown BLS event: %lx \r\n", event);
            break;
    }
}

/*******************************************************************************
* Function Name: BlsInit
********************************************************************************
*
* Summary:
*   Initializes the blood pressure service.
*
*******************************************************************************/
void BlsInit(void)
{
    /* Register service specific callback function */
    Cy_BLE_BLS_RegisterAttrCallback(BlsCallBack);
}

/*******************************************************************************
* Function Name: BlsInd
********************************************************************************
*
* Summary:
*   Sends the Blood Pressure Measurement indication.
*
* Parameters:
*   connHandle: The connection handle
*   num - number of record to indicate.
*
*******************************************************************************/
void BlsInd(cy_stc_ble_conn_handle_t connHandle, uint8_t num)
{
    cy_en_ble_api_result_t apiResult;
    uint16_t cccd;
        
    apiResult = Cy_BLE_BLSS_GetCharacteristicDescriptor(connHandle, CY_BLE_BLS_BPM, CY_BLE_BLS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
              
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_BLSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
    }
    else if(cccd == CY_BLE_CCCD_INDICATION) 
    {
        uint8_t pdu[sizeof(cy_stc_ble_bls_bpm_t)];
        uint8_t ptr;

        /* flags, Systolic, Diastolic and Mean Arterial Pressure fields always go first */
        pdu[0u] = blsBpm[num].flags;
        pdu[1u] = CY_LO8(blsBpm[num].sys);
        pdu[2u] = CY_HI8(blsBpm[num].sys);
        pdu[3u] = CY_LO8(blsBpm[num].dia);
        pdu[4u] = CY_HI8(blsBpm[num].dia);
        pdu[5u] = CY_LO8(blsBpm[num].map);
        pdu[6u] = CY_HI8(blsBpm[num].map);
        
        /* if the Time Stamp Present flag is set */
        if(0u != (blsBpm[num].flags & CY_BLE_BLS_BPM_FLG_TSP))
        {
            /* set the full 7-bytes Time Stamp value */
            pdu[7u] = CY_LO8(blsBpm[num].time.year);
            pdu[8u] = CY_HI8(blsBpm[num].time.year);
            pdu[9u] = blsBpm[num].time.month;
            pdu[10u] = blsBpm[num].time.day;
            pdu[11u] = blsBpm[num].time.hours;
            pdu[12u] = blsBpm[num].time.minutes;
            pdu[13u] = blsBpm[num].time.seconds;

            /* the next data will be located at 14th byte */
            ptr = 14u;
        }
        else
        {
            /* the next data will be located at 7th byte */
            ptr = 7u;   
        }

        if(0u != (blsBpm[num].flags & CY_BLE_BLS_BPM_FLG_PRT))
        {
            pdu[ptr] = CY_LO8(blsBpm[num].prt);
            pdu[ptr + 1u] = CY_HI8(blsBpm[num].prt);
            ptr += 2u;
        }

        if(0u != (blsBpm[num].flags & CY_BLE_BLS_BPM_FLG_UID))
        {
            pdu[ptr] = blsBpm[num].uid;
            ptr += 1u;
        }

        if(0u != (blsBpm[num].flags & CY_BLE_BLS_BPM_FLG_MST))
        {
            pdu[ptr] = CY_LO8(blsBpm[num].mst);
            pdu[ptr + 1u] = CY_HI8(blsBpm[num].mst);
            ptr += 2u;
        }
        
        do
        {
            Cy_BLE_ProcessEvents();
        }
        while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);
        
        if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
        {
            if((apiResult = Cy_BLE_BLSS_SendIndication(connHandle, CY_BLE_BLS_BPM, ptr, pdu)) != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_BLSS_SendIndication API Error: 0x%x \r\n", apiResult );
            }
            else
            {
                DBG_PRINTF("Blood Pressure Ind  sys:%d mmHg, dia:%d mmHg\r\n", blsBpm[num].sys, blsBpm[num].dia);
            }
        }
    }
}


/*******************************************************************************
* Function Name: BlsNtf
********************************************************************************
*
* Summary:
*   Sends the BLS Intermediate Cuff Pressure notification.
*
* Parameters:
*   connHandle: The connection handle
*   num: number of record to notify.
*
*******************************************************************************/
void BlsNtf(cy_stc_ble_conn_handle_t connHandle, uint8_t num)
{
    cy_en_ble_api_result_t apiResult;
    uint16_t cccd;
        
    apiResult = Cy_BLE_BLSS_GetCharacteristicDescriptor(connHandle, CY_BLE_BLS_ICP, CY_BLE_BLS_CCCD, CY_BLE_CCCD_LEN, 
                                                   (uint8_t*)&cccd);
                        
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_BLSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
    }
    else if(cccd == CY_BLE_CCCD_NOTIFICATION) 
    {
        uint8_t pdu[sizeof(cy_stc_ble_bls_bpm_t)];
        uint8_t ptr;
        
        /* flags, Systolic, Diastolic and Mean Arterial Pressure fields always go first */
        pdu[0u] = blsIcp[num].flags;
        pdu[1u] = CY_LO8(blsIcp[num].sys);
        pdu[2u] = CY_HI8(blsIcp[num].sys);
        pdu[3u] = CY_LO8(blsIcp[num].dia);
        pdu[4u] = CY_HI8(blsIcp[num].dia);
        pdu[5u] = CY_LO8(blsIcp[num].map);
        pdu[6u] = CY_HI8(blsIcp[num].map);

        /* if the Time Stamp Present flag is set */
        if(0u != (blsIcp[num].flags & CY_BLE_BLS_BPM_FLG_TSP))
        {
            /* set the full 7-bytes Time Stamp value */
            pdu[7u] = CY_LO8(blsIcp[num].time.year);
            pdu[8u] = CY_HI8(blsIcp[num].time.year);
            pdu[9u] = blsIcp[num].time.month;
            pdu[10u] = blsIcp[num].time.day;
            pdu[11u] = blsIcp[num].time.hours;
            pdu[12u] = blsIcp[num].time.minutes;
            pdu[13u] = blsIcp[num].time.seconds;

            /* the next data will be located at 14th byte */
            ptr = 14u;
        }
        else
        {
            /* the next data will be located at 7th byte */
            ptr = 7u;   
        }

        if(0u != (blsIcp[num].flags & CY_BLE_BLS_BPM_FLG_PRT))
        {
            pdu[ptr] = CY_LO8(blsIcp[num].prt);
            pdu[ptr + 1u] = CY_HI8(blsIcp[num].prt);
            ptr += 2u;
        }

        if(0u != (blsIcp[num].flags & CY_BLE_BLS_BPM_FLG_UID))
        {
            pdu[ptr] = blsIcp[num].uid;
            ptr += 1u;
        }

        if(0u != (blsIcp[num].flags & CY_BLE_BLS_BPM_FLG_MST))
        {
            pdu[ptr] = CY_LO8(blsIcp[num].mst);
            pdu[ptr + 1u] = CY_HI8(blsIcp[num].mst);
            ptr += 2u;
        }

        do
        {
            Cy_BLE_ProcessEvents();
        }
        while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);
        
        if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
        {
            if((apiResult = Cy_BLE_BLSS_SendNotification(connHandle, CY_BLE_BLS_ICP, ptr, pdu)) != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_BLSS_SendNotification API Error: ");
                PrintApiResult(apiResult);
            }
            else
            {
                DBG_PRINTF("Intermediate Cuff Pressure Ntf: %d mmHg\r\n", blsIcp[num].sys);
            }
        }
    }
}

/*******************************************************************************
* Function Name:  BlsSimulateBloodPressure
********************************************************************************
*
* Summary:
*   Simulate Blood Pressure.
*
* Parameters:
*   connHandle: The connection handle
*
*******************************************************************************/
void BlsSimulateBloodPressure(cy_stc_ble_conn_handle_t connHandle)
{
    static uint32_t blsTimer = BLS_TIMEOUT;
    
    if(--blsTimer == 0u) 
    {
        blsTimer = BLS_TIMEOUT;
    
        blsSim++;
        if(blsSim > SIM_UNIT_MAX)
        {
            blsSim = 0;
        }
        
        blsIcp[0u].sys = SIM_ICF_MAX - SIM_BLS_DLT * (blsSim & SIM_ICF_MSK);
        blsIcp[0u].time.seconds = blsSim;
        BlsNtf(connHandle, 0u);
        
        blsBpm[0u].sys = SIM_BPM_SYS_MIN + (blsSim & SIM_BPM_MSK);
        blsBpm[0u].dia = SIM_BPM_DIA_MIN + (blsSim & SIM_BPM_MSK);
        blsBpm[0u].time.seconds = blsSim;
        BlsInd(connHandle, 0u);
    }
}


/* [] END OF FILE */
