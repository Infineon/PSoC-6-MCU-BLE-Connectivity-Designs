/*******************************************************************************
* File Name: cscss.c
*
* Version: 1.0
*
* Description:
*  This file contains routines related to Cycling Speed and Cadence Service.
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

#include "cscs.h"


/***************************************
*        Global Variables
***************************************/
static uint8_t                cscFlags;
static uint32_t               wheelRev = CSC_WHEEL_REV_INIT_VAL;
static uint32_t               lastWheelEvTime = CSC_WHEEL_EV_TIME_VAL;
static uint32_t               crankRev = 0u;
static uint32_t               lastCrankEvTime = CSC_CRANK_EV_TIME_VAL;
static uint16_t               cscSpeed;
static uint16_t               cscCadenceRpm;
static uint8_t                speedCPresponse = 0u;

/* Stores the sensor locations supported by the device */
static uint8_t                sensorLocations[NUM_SUPPORTED_SENSORS] = {TOP_OF_SHOE, IN_SHOE, LEFT_CRANK, RIGHT_CRANK};

/* Data buffer for Control Point response. 
   First byte contains the length of response 
*/
static uint8_t                scCPResponse[SC_CP_CHAR_LENGTH_4BYTES + 1u + NUM_SUPPORTED_SENSORS];


/*******************************************************************************
* Function Name: CscServiceAppEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component,
*  which are specific to Cycling Speed and Cadence Service.
*
* Parameters:  
*  event:           CSCS event.
*  *eventParams:    Data structure specific to event received.
*
*******************************************************************************/
void CscsCallback(uint32_t event, void *eventParam)
{
    uint8_t i;
    uint8_t sensorLocSupported = 0u;
    cy_stc_ble_cscs_char_value_t *wrReqParam;

    switch(event)
    {
    /* CSCS Server - Notifications for Cycling Speed and Cadence Service
        Characteristic was enabled. The parameter of this event is a structure of
        cy_stc_ble_cscs_char_value_t type.
    */
    case CY_BLE_EVT_CSCSS_NOTIFICATION_ENABLED:
        DBG_PRINTF("Notifications for CSC Measurement Characteristic are enabled\r\n");
        break;

    /* CSCS Server - Notifications for Cycling Speed and Cadence Service
        Characteristic was disabled. The parameter of this event is a structure  of
        cy_stc_ble_cscs_char_value_t type
    */
    case CY_BLE_EVT_CSCSS_NOTIFICATION_DISABLED:
        DBG_PRINTF("Notifications for CSC Measurement Characteristic are disabled\r\n");
        break;

    /* CSCS Server - Indication for Cycling Speed and Cadence Service Characteristic
        was enabled. The parameter of this event is a structure of
        cy_stc_ble_cscs_char_value_t type
    */
    case CY_BLE_EVT_CSCSS_INDICATION_ENABLED:
        DBG_PRINTF("Indications for SC Control Point Characteristic are enabled\r\n");
        break;

    /* CSCS Server - Indication for Cycling Speed and Cadence Service Characteristic
        was disabled. The parameter of this event is a structure of
        cy_stc_ble_cscs_char_value_t type
    */
    case CY_BLE_EVT_CSCSS_INDICATION_DISABLED:
        DBG_PRINTF("Indications for SC Control Point Characteristic are disabled\r\n");
        break;

    /* CSCS Server - Cycling Speed and Cadence Service Characteristic
        Indication was confirmed. The parameter of this event is a structure of 
        cy_stc_ble_cscs_char_value_t type
    */
    case CY_BLE_EVT_CSCSS_INDICATION_CONFIRMATION:
        DBG_PRINTF("Confirmation of SC Control Point Characteristic indication received\r\n");
        break;
    
    /* CSCS Server - Write Request for Cycling Speed and Cadence Service
        Characteristic was received. The parameter of this event is a structure of
        cy_stc_ble_cscs_char_value_t type.
    */
    case CY_BLE_EVT_CSCSS_WRITE_CHAR:

        wrReqParam = (cy_stc_ble_cscs_char_value_t *) eventParam;
        DBG_PRINTF("Write to SC Control Point Characteristic occurred. ");
        DBG_PRINTF("Data length: %d. ", wrReqParam->value->len);
        DBG_PRINTF("Received data:");
        
        for(i = 0u; i < wrReqParam->value->len; i++)
        {
             DBG_PRINTF(" %x", wrReqParam->value->val[i]);
        }
         DBG_PRINTF("\r\n");

        /* Form the general response packet */
        scCPResponse[CSCS_SC_CP_RESP_LEN_IDX] = SC_CP_CHAR_LENGTH_3BYTES;
        scCPResponse[CSCS_SC_CP_RESP_OP_CODE_IDX] = CY_BLE_CSCS_RESPONSE_CODE;
        scCPResponse[CSCS_SC_CP_REQ_OP_CODE_IDX] = wrReqParam->value->val[CSCS_SC_CP_OP_CODE_IDX];
        scCPResponse[CSCS_SC_CP_RESP_VALUE_IDX] = CY_BLE_CSCS_ERR_SUCCESS;

        /* Handle the received op code */
        switch(wrReqParam->value->val[CSCS_SC_CP_OP_CODE_IDX])
        {
            case CY_BLE_CSCS_SET_CUMMULATIVE_VALUE:
                if(wrReqParam->value->len == CSCS_SET_CUMMULATIVE_VALUE_LEN)
                {
                    wheelRev = (wrReqParam->value->val[CSCS_SC_CUM_VAL_BYTE0_IDX + 3u] << THREE_BYTES_SHIFT) |
                                    (wrReqParam->value->val[CSCS_SC_CUM_VAL_BYTE0_IDX + 2u] << TWO_BYTES_SHIFT) |
                                    (wrReqParam->value->val[CSCS_SC_CUM_VAL_BYTE0_IDX + 1u] << ONE_BYTE_SHIFT) |
                                    wrReqParam->value->val[CSCS_SC_CUM_VAL_BYTE0_IDX];
                    
                    if(0ul == wheelRev)
                    {
                        DBG_PRINTF("Set cumulative value to zero.\r\n");
                    }
                    else
                    {
                        DBG_PRINTF("Set cumulative value to 0x%4.4x%4.4x.\r\n", CY_HI16(wheelRev), CY_LO16(wheelRev));
                    }
                }
                else
                {
                    /* Length is invalid, indicate failure */
                    scCPResponse[CSCS_SC_CP_RESP_VALUE_IDX] = CY_BLE_CSCS_ERR_OPERATION_FAILED;
                }
                break;

            case CY_BLE_CSCS_START_SENSOR_CALIBRATION:
                DBG_PRINTF("Start Sensor calibration command received. ");
                if(wrReqParam->value->len == CSCS_START_SENSOR_CALIBRATION_LEN)
                {
                    DBG_PRINTF("This command is not supported in this example project.\r\n");
                    /* The Start Sensor Calibration command is not supported in the example */
                    scCPResponse[CSCS_SC_CP_RESP_VALUE_IDX] = CY_BLE_CSCS_ERR_OP_CODE_NOT_SUPPORTED;
                }
                else
                {
                    /* Length is invalid, indicate failure */
                    scCPResponse[CSCS_SC_CP_RESP_VALUE_IDX] = CY_BLE_CSCS_ERR_OPERATION_FAILED;
                }
                break;

            case CY_BLE_CSCS_UPDATE_SENSOR_LOCATION:
                DBG_PRINTF("Update Sensor Location \r\n");

                if(wrReqParam->value->len == CSCS_UPDATE_SENSOR_LOCATION_LEN)
                {
                    /* Update to the location of the Sensor with the value sent as parameter to this op code. 
                    */
                    for(i = 0u; i < NUM_SUPPORTED_SENSORS; i++)
                    {
                        if(sensorLocations[i] == wrReqParam->value->val[CY_BLE_CSCS_SENSOR_LOC_IDX])
                        {
                            sensorLocSupported = 1u;
                        }
                    }

                    if(0u != sensorLocSupported)
                    {
                        (void) Cy_BLE_CSCSS_SetCharacteristicValue(CY_BLE_CSCS_SENSOR_LOCATION, 
                                                                 sizeof(uint8_t),
                                                                 &wrReqParam->value->val[CY_BLE_CSCS_SENSOR_LOC_IDX]);
                        DBG_PRINTF("Set sensor location operation completed successfully. \r\n");
                    }
                    else
                    {
                        scCPResponse[CSCS_SC_CP_RESP_VALUE_IDX] = CY_BLE_CSCS_ERR_INVALID_PARAMETER;
                        DBG_PRINTF("Unsupported sensor location.\r\n");
                    }
                }
                else
                {
                    /* Length is invalid, indicate failure */
                    scCPResponse[CSCS_SC_CP_RESP_VALUE_IDX] = CY_BLE_CSCS_ERR_OPERATION_FAILED;
                }
                break;

            case CY_BLE_CSCS_REQ_SUPPORTED_SENSOR_LOCATION:
                DBG_PRINTF("Request Supported Sensor Locations \r\n");

                if(wrReqParam->value->len == CSCS_REQ_SUPPORTED_SENSOR_LOCATION_LEN)
                {
                    /* Request a list of supported locations where the Sensor can be attached */
                    scCPResponse[CSCS_SC_CP_RESP_LEN_IDX] += NUM_SUPPORTED_SENSORS;
                    
                    for(i = 0u; i < NUM_SUPPORTED_SENSORS; i++)
                    {
                        scCPResponse[CSCS_SC_CP_RESP_PARAMETER_IDX + i] = sensorLocations[i];
                    }
                }
                else
                {
                    /* Length is invalid, indicate failure */
                    scCPResponse[CSCS_SC_CP_RESP_VALUE_IDX] = CY_BLE_CSCS_ERR_OPERATION_FAILED;
                }
                break;

            default:
                DBG_PRINTF("Unsupported command.\r\n");
                scCPResponse[CSCS_SC_CP_RESP_VALUE_IDX] = CY_BLE_CSCS_ERR_OP_CODE_NOT_SUPPORTED;
                break;
        }

        /* Set the flag about active response pending */
        speedCPresponse = 1u;
        break;


    /* CSCS Client - Cycling Speed and Cadence Service Characteristic
        Notification was received. The parameter of this event is a structure of
        cy_stc_ble_cscs_char_value_t type
    */
    case CY_BLE_EVT_CSCSC_NOTIFICATION:
        break;

    /* CSCS Client - Cycling Speed and Cadence Service Characteristic
        Indication was received. The parameter of this event is a structure of 
        cy_stc_ble_cscs_char_value_t type
    */
    case CY_BLE_EVT_CSCSC_INDICATION:
        break;

    /* CSCS Client - Read Response for Read Request of Cycling Speed and Cadence 
        Service Characteristic value. The parameter of this event is a structure of
        cy_stc_ble_cscs_char_value_t type
    */
    case CY_BLE_EVT_CSCSC_READ_CHAR_RESPONSE:
        break;

    /* CSCS Client - Write Response for Write Request of Cycling Speed and Cadence 
        Service Characteristic value. The parameter of this event is a structure of
        cy_stc_ble_cscs_char_value_t type
    */
    case CY_BLE_EVT_CSCSC_WRITE_CHAR_RESPONSE:
        break;

    /* CSCS Client - Read Response for Read Request of Cycling Speed and Cadence
        Service Characteristic Descriptor Read request. The parameter of this event
        is a structure of cy_stc_ble_cscs_descr_value_t type
    */
    case CY_BLE_EVT_CSCSC_READ_DESCR_RESPONSE:
        break;

    /* CSCS Client - Write Response for Write Request of Cycling Speed and Cadence
        Service Characteristic Configuration Descriptor value. The parameter of
        this event is a structure of  cy_stc_ble_cscs_descr_value_t type
    */
    case CY_BLE_EVT_CSCSC_WRITE_DESCR_RESPONSE:
        break;

    default:
        DBG_PRINTF("Unrecognized CSCS event.\r\n");
        break;
    }
}


/*******************************************************************************
* Function Name: CscsInit()
********************************************************************************
*
* Summary:
*   Initializes the Cycling Speed and Cadence Service.
*
*******************************************************************************/
void CscsInit(void)
{
    uint8_t buff[CSC_MEASUREMENT_CHAR_SIZE];

    /* Register service specific callback function */
    Cy_BLE_CSCS_RegisterAttrCallback(CscsCallback);
    
    /* Get initial value of CSC Measurement Characteristic */
    if(Cy_BLE_CSCSS_GetCharacteristicValue(CY_BLE_CSCS_CSC_MEASUREMENT, CSC_MEASUREMENT_CHAR_SIZE, buff) !=
            CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Failed to read the CSC Measurement value.\r\n");
    }

    /* Set initial CSC Characteristic flags as per values set in the customizer */
    cscFlags = buff[0u];
}


/*******************************************************************************
* Function Name: SimulateCyclingSpeed
********************************************************************************
*
* Summary:
*  Simulates Cycling speed data and send them to the Client device.
*
*******************************************************************************/
void CscsSimulateCyclingSpeed(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    uint8_t csValue[CSC_MEASUREMENT_CHAR_SIZE];
    uint8_t len = 0u;
    uint16_t cccd;
    
    /*  Updates CSC Measurement Characteristic data */
    wheelRev += CSC_WHEEL_REV_VAL;
    lastWheelEvTime += CSC_WHEEL_EV_TIME_VAL;
    crankRev += CSC_CRANK_REV_VAL;
    lastCrankEvTime += CSC_CRANK_EV_TIME_VAL;

    apiResult = Cy_BLE_CSCSS_GetCharacteristicDescriptor(connHandle, CY_BLE_CSCS_CSC_MEASUREMENT, CY_BLE_CSCS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
          
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_CSCSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
    }
    else if(cccd == CY_BLE_CCCD_NOTIFICATION) 
    {
        /* Pack SCS Measurement Characteristic data */
        csValue[len++] = cscFlags;
        
        if(0u != (cscFlags & CSC_WHEEL_REV_DATA_PRESENT))
        {
            csValue[len++] = CY_LO8(CY_LO16(wheelRev));
            csValue[len++] = CY_HI8(CY_LO16(wheelRev));
            csValue[len++] = CY_LO8(CY_HI16(wheelRev));
            csValue[len++] = CY_HI8(CY_HI16(wheelRev));

            csValue[len++] = CY_LO8(lastWheelEvTime);
            csValue[len++] = CY_HI8(lastWheelEvTime);

            /* Calculate instantaneous speed */
            cscSpeed = ((((((uint32)WHEEEL_CIRCUMFERENCE_CM) * (CSC_WHEEL_REV_VAL))
                        * ((uint32) WHEEL_TIME_EVENT_UNIT)) / ((uint32) CSC_WHEEL_EV_TIME_VAL))
                            * ((uint32) MS_TO_KMH_COEFITIENT)) / ((uint32) INT_DIVIDER);
        }

        if(0u != (cscFlags & CSC_CRANK_REV_DATA_PRESENT))
        {
            csValue[len++] = CY_LO8(crankRev);
            csValue[len++] = CY_HI8(crankRev);

            csValue[len++] = CY_LO8(lastCrankEvTime);
            csValue[len++] = CY_HI8(lastCrankEvTime);

            cscCadenceRpm = (((uint32) CSC_CRANK_REV_VAL) * ((uint32) WHEEL_TIME_EVENT_UNIT) * 60u) /
                               ((uint32) CSC_CRANK_EV_TIME_VAL);
        }
        
        do
        {
            Cy_BLE_ProcessEvents();
        }
        while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);
    
        if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
        {
            /* Send Characteristic value to peer device */
            apiResult = Cy_BLE_CSCSS_SendNotification(connHandle, CY_BLE_CSCS_CSC_MEASUREMENT, len, csValue);

            if(CY_BLE_SUCCESS == apiResult)
            {
                DBG_PRINTF("CscssSendNotification, ");
                DBG_PRINTF("Wheel Revolution: %ld, ", wheelRev);
                DBG_PRINTF("Wheel Time: %ld s, ", lastWheelEvTime / CSC_TIME_PER_SEC);
                DBG_PRINTF("Crank Revolution: %ld, ", crankRev);
                DBG_PRINTF("Crank Time: %ld s, ", lastCrankEvTime / CSC_TIME_PER_SEC);
                DBG_PRINTF("Speed: %d.%2.2d km/h, ", cscSpeed/100u, cscSpeed%100u);
                DBG_PRINTF("Cadence: %d rpm\r\n", cscCadenceRpm);
            }
            else
            {
                DBG_PRINTF("Error #%x while sending CSC Measurement Characteristic notification\r\n", apiResult);
            }
        }
    }
    
    apiResult = Cy_BLE_CSCSS_GetCharacteristicDescriptor(connHandle, CY_BLE_CSCS_SC_CONTROL_POINT, CY_BLE_CSCS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
          
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_CSCSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
    }
    else if(cccd == CY_BLE_CCCD_INDICATION) 
    {
        do
        {
            Cy_BLE_ProcessEvents();
        }
        while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);
    
        if((Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED) && 
           (speedCPresponse != 0u))
        {
            apiResult = Cy_BLE_CSCSS_SendIndication(connHandle, CY_BLE_CSCS_SC_CONTROL_POINT, 
                                                     scCPResponse[CSCS_SC_CP_RESP_LEN_IDX], scCPResponse + 1u);
            DBG_PRINTF("Cy_BLE_CSCS_SendIndication(CY_BLE_CSCS_SC_CONTROL_POINT), API result: %x \r\n", apiResult);
            speedCPresponse = 0u;
        }
    }    
}


/* [] END OF FILE */
