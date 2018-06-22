/*******************************************************************************
* File Name: rscs.c
*
* Version: 1.0
*
* Description:
*  This file contains routines related to Running Speed and Cadence Service.
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
#include "rscs.h"
#include "user_interface.h"

/***************************************
*        Global Variables
***************************************/
static uint8_t                   profile = WALKING;
static uint8_t                   rscIndicationPending = false;
static uint8_t                   rcsOpCode    = RSC_SC_CP_INVALID_OP_CODE;
static uint8_t                   rcsRespValue = RSC_SC_CP_INVALID_OP_CODE;
static uint32_t                  totalDistanceCm;

/* Sensor locations supported by the device */
static uint8_t                 rscSensors[RSC_SENSORS_NUMBER];

/* This variable contains profile simulation data */
static rsc_rsc_measurement_t   rscMeasurement;

/* This variable contains the value of RSC Feature Characteristic */
static uint16_t                rscFeature;

/* Contains the min. and max. cadence ranges for Running and Walking profiles */
static cadence_max_min_t       cadenceRanges[NUM_PROFILES] = {
    {WALKING_INST_CADENCE_MAX, WALKING_INST_CADENCE_MIN},
    {RUNNING_INST_CADENCE_MAX, RUNNING_INST_CADENCE_MIN}};

/* Contains the min. and max. stride length ranges for Running and Walking 
* profiles.
*/
static stride_len_max_min_t    strideLenRanges[NUM_PROFILES] = {
    {WALKING_INST_STRIDE_LENGTH_MAX, WALKING_INST_STRIDE_LENGTH_MIN},
    {RUNNING_INST_STRIDE_LENGTH_MAX, RUNNING_INST_STRIDE_LENGTH_MIN}};

/*******************************************************************************
* Function Name: RscServiceAppEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component,
*  which are specific to Running Speed and Cadence Service.
*
* Parameters:  
*  uint8_t event:       Running Speed and Cadence Service event.
*  void* eventParams:   Data structure specific to event received.
*
*******************************************************************************/
void RscServiceAppEventHandler(uint32_t event, void *eventParam)
{
    uint8_t i;
    cy_stc_ble_rscs_char_value_t *wrReqParam;
    
    switch(event)
    {
    /***************************************
    *        RSCS Server events
    ***************************************/
    case CY_BLE_EVT_RSCSS_NOTIFICATION_ENABLED:
        DBG_PRINTF("Notifications for RSC Measurement Characteristic are enabled\r\n");
        break;
        
    case CY_BLE_EVT_RSCSS_NOTIFICATION_DISABLED:
        DBG_PRINTF("Notifications for RSC Measurement Characteristic are disabled\r\n");
        break;

    case CY_BLE_EVT_RSCSS_INDICATION_ENABLED:
        DBG_PRINTF("Indications for SC Control point Characteristic are enabled\r\n");
        break;
        
    case CY_BLE_EVT_RSCSS_INDICATION_DISABLED:
        DBG_PRINTF("Indications for SC Control point Characteristic are disabled\r\n");
        break;

    case CY_BLE_EVT_RSCSS_INDICATION_CONFIRMATION:
        DBG_PRINTF("Indication Confirmation for SC Control point was received\r\n");
        break;
    
    case CY_BLE_EVT_RSCSS_WRITE_CHAR:
        
        wrReqParam = (cy_stc_ble_rscs_char_value_t *) eventParam;
        DBG_PRINTF("Write to SC Control Point Characteristic occurred\r\n");
        DBG_PRINTF("Data length: %d\r\n", wrReqParam->value->len);
        DBG_PRINTF("Received data:");

        for(i = 0u; i < wrReqParam->value->len; i++)
        {
             DBG_PRINTF(" %x", wrReqParam->value->val[i]);
        }
         DBG_PRINTF("\r\n");
        
        rcsOpCode = wrReqParam->value->val[RSC_SC_CP_OP_CODE_IDX];

        switch(rcsOpCode)
        {
        case CY_BLE_RSCS_SET_CUMMULATIVE_VALUE:
            
            /* Validate command length */
            if(wrReqParam->value->len == RSC_SET_CUMMULATIVE_VALUE_LEN)
            {
                if(0u != (rscFeature && RSC_FEATURE_INST_STRIDE_SUPPORTED))
                {
                    totalDistanceCm = ((wrReqParam->value->val[RSC_SC_CUM_VAL_BYTE3_IDX] << THREE_BYTES_SHIFT) |
                                        (wrReqParam->value->val[RSC_SC_CUM_VAL_BYTE2_IDX] << TWO_BYTES_SHIFT) |
                                        (wrReqParam->value->val[RSC_SC_CUM_VAL_BYTE1_IDX] << ONE_BYTE_SHIFT) |
                                        wrReqParam->value->val[RSC_SC_CUM_VAL_BYTE0_IDX]) * RSCS_CM_TO_DM_VALUE;

                    DBG_PRINTF("Set cumulative value command was received.\r\n");
                    rscMeasurement.totalDistance *= RSCS_CM_TO_DM_VALUE;
                    rcsRespValue = CY_BLE_RSCS_ERR_SUCCESS;
                }
                else
                {
                    DBG_PRINTF("The procedure is not supported.\r\n");
                    rcsRespValue = CY_BLE_RSCS_ERR_OP_CODE_NOT_SUPPORTED;
                }
            }
            else
            {
                rcsRespValue = CY_BLE_RSCS_ERR_OPERATION_FAILED;
            }
            break;

        case CY_BLE_RSCS_START_SENSOR_CALIBRATION:
            DBG_PRINTF("Start Sensor calibration command was received.\r\n");
            rcsRespValue = CY_BLE_RSCS_ERR_OP_CODE_NOT_SUPPORTED;
            rcsOpCode = CY_BLE_RSCS_START_SENSOR_CALIBRATION;
            break;

        case CY_BLE_RSCS_UPDATE_SENSOR_LOCATION:
            
            /* Validate command length */
            if(wrReqParam->value->len == RSC_UPDATE_SENSOR_LOCATION_LEN)
            {
                if(0u != (rscFeature & RSC_FEATURE_MULTIPLE_SENSOR_LOC_PRESENT))
                {
                    DBG_PRINTF("Update sensor location command was received.\r\n");
                    
                    /* Check if the requested sensor location is supported */
                    if(RscIsSensorLocationSupported(wrReqParam->value->val[RSC_SC_SENSOR_LOC_IDX]) == true)
                    {
                        DBG_PRINTF("New Sensor location was set.\r\n");
                        
                        /* Set requested sensor location */
                        Cy_BLE_RSCSS_SetCharacteristicValue(CY_BLE_RSCS_SENSOR_LOCATION, 1u, 
                            &wrReqParam->value->val[RSC_SC_SENSOR_LOC_IDX]);
                        rcsRespValue = CY_BLE_RSCS_ERR_SUCCESS;
                    }
                    else
                    {
                        DBG_PRINTF("The requested sensor location is not supported.\r\n");
                        
                        /* Invalid sensor location is received */
                        rcsRespValue = CY_BLE_RSCS_ERR_INVALID_PARAMETER;
                    }
                }
                else
                {
                    DBG_PRINTF("The procedure is not supported.\r\n");
                    rcsRespValue = CY_BLE_RSCS_ERR_OP_CODE_NOT_SUPPORTED;
                }
            }
            else
            {
                rcsRespValue = CY_BLE_RSCS_ERR_OPERATION_FAILED;
            }
            break;

        case CY_BLE_RSCS_REQ_SUPPORTED_SENSOR_LOCATION:
            
            /* Validate command length */
            if(wrReqParam->value->len == RSC_REQ_SUPPORTED_SENSOR_LOCATION_LEN)
            {
                if(0u != (rscFeature & RSC_FEATURE_MULTIPLE_SENSOR_LOC_PRESENT))
                {
                    DBG_PRINTF("Supported sensor location command was received.\r\n");
                    rcsRespValue = CY_BLE_RSCS_ERR_SUCCESS;
                }
                else
                {
                    DBG_PRINTF("The procedure is not supported.\r\n");
                    rcsRespValue = CY_BLE_RSCS_ERR_OP_CODE_NOT_SUPPORTED;
                }
            }
            else
            {
                rcsRespValue = CY_BLE_RSCS_ERR_OPERATION_FAILED;
            }
            break;

        default:
            DBG_PRINTF("Unsupported command.\r\n");
            break;
        }

        /* Set the flag to sent indication from main() */
        rscIndicationPending = true;
        break;

    /***************************************
    *        RSCS Client events
    ***************************************/
    case CY_BLE_EVT_RSCSC_NOTIFICATION:
        break;
    case CY_BLE_EVT_RSCSC_INDICATION:
        break;
    case CY_BLE_EVT_RSCSC_READ_CHAR_RESPONSE:
        break;
    case CY_BLE_EVT_RSCSC_WRITE_CHAR_RESPONSE:
        break;
    case CY_BLE_EVT_RSCSC_READ_DESCR_RESPONSE:
        break;
    case CY_BLE_EVT_RSCSC_WRITE_DESCR_RESPONSE:
        break;

    default:
        DBG_PRINTF("Unrecognized RSCS event.\r\n");
        break;
    }
}


/*******************************************************************************
* Function Name: RscInit
********************************************************************************
*
* Summary:
*  Performs InitRscs  initialization.
*
*******************************************************************************/
void RscInit(void)
{
    uint8_t buff[RSC_RSC_MEASUREMENT_CHAR_SIZE];
    
    /* Register the event handler for RSCS specific events */
    Cy_BLE_RSCS_RegisterAttrCallback(RscServiceAppEventHandler);

    
    if(Cy_BLE_RSCSS_GetCharacteristicValue(CY_BLE_RSCS_RSC_MEASUREMENT, RSC_RSC_MEASUREMENT_CHAR_SIZE,
                                          (uint8_t *) &rscMeasurement) != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Failed to read the RSC Measurement value.\r\n");
    }

    totalDistanceCm = rscMeasurement.totalDistance * RSCS_CM_TO_DM_VALUE;
    
    /* Get the RSC Feature */
    /* Get the characteristic into the buffer */
    if((Cy_BLE_RSCSS_GetCharacteristicValue(CY_BLE_RSCS_RSC_FEATURE, RSC_RSC_FEATURE_SIZE, buff)) == CY_BLE_SUCCESS)
    {
        /* Fill in the rsc_rsc_measurement_t structure */
        rscFeature = (uint16_t) ((((uint16_t) buff[1u]) << ONE_BYTE_SHIFT) | ((uint16_t) buff[0u]));
    }
    else
    {
        DBG_PRINTF("Failed to read the RSC Feature value.\r\n");
    }
    
    /* Set supported sensor locations */
    rscSensors[RSC_SENSOR1_IDX] = RSC_SENSOR_LOC_IN_SHOE;
    rscSensors[RSC_SENSOR2_IDX] = RSC_SENSOR_LOC_HIP;
}


/*******************************************************************************
* Function Name: RscSimulateProfile
********************************************************************************
*
* Summary:
*  Simulates the Running Speed and Cadence profile. When this function is called, 
*  it is assumed that the complete stride has occurred and it is the time to
*  update the speed and the total distance values.
*
*******************************************************************************/
void RscSimulateProfile(void)
{
    /* Update total distance */
    totalDistanceCm += rscMeasurement.instStridelen;
    rscMeasurement.totalDistance = totalDistanceCm / RSCS_CM_TO_DM_VALUE;

    /* Calculate speed in m/s with resolution of 1/256 of second */
    rscMeasurement.instSpeed = (((uint16_t)(2 * rscMeasurement.instCadence * rscMeasurement.instStridelen)) << 8u) /
                                           (RSCS_SEC_TO_MIN_VALUE * RSCS_CM_TO_M_VALUE);
}


/*******************************************************************************
* Function Name: RscIsSensorLocationSupported
********************************************************************************
*
* Summary:
*  Verifies if the requested sensor location is supported by the device.
*
* Parameters:  
*  sensorLocation: Sensor location.
*
* Return: 
*  Status:
*   true - the sensor is supported;
*   false - sensor is not supported.
*
*******************************************************************************/
bool RscIsSensorLocationSupported(uint8_t sensorLocation)
{
    uint8_t i;
    uint8_t result = false;
    
    for(i = 0u; i < RSC_SENSORS_NUMBER; i++)
    {
        if(rscSensors[i] == sensorLocation)
        {
            result = true;
        }
    }
    return(result);
}


/*******************************************************************************
* Function Name: RscNotifications
********************************************************************************
*
* Summary:
*  Handles Notifications to the Client device.
*
* Parameters:
*   connHandle - The connection handler.
*
*******************************************************************************/
void RscNotifications(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    uint16_t instSpeed;
    uint16_t cccd;

    apiResult = Cy_BLE_RSCSS_GetCharacteristicDescriptor(connHandle, CY_BLE_RSCS_RSC_MEASUREMENT, CY_BLE_RSCS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
              
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_RSCSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
    }
    else if(cccd == CY_BLE_CCCD_NOTIFICATION)
    {
        do
        {
            Cy_BLE_ProcessEvents();
        }
        while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);
        
        if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
        {
            /* Send notification to the peer Client */
            apiResult = Cy_BLE_RSCSS_SendNotification(connHandle, CY_BLE_RSCS_RSC_MEASUREMENT,
                                                      RSC_RSC_MEASUREMENT_CHAR_SIZE, (uint8_t *) &rscMeasurement);

            /* Update the debug info if notification is sent successfully */
            if(apiResult == CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Notification is sent! \r\n");
                DBG_PRINTF("Cadence: %d, ", rscMeasurement.instCadence);

                /* Calculate the instantaneous speed in m/h */
                instSpeed = ((rscMeasurement.instSpeed * RSCS_SEC_TO_HOUR_VALUE) >> 8u) / 10u;
                (void)instSpeed;
                
                /* Display instantaneous speed in km/h */
                DBG_PRINTF("Speed: %d.%2.2d km/h, ", instSpeed/RSCS_10M_TO_KM_VALUE, instSpeed%RSCS_10M_TO_KM_VALUE);
                DBG_PRINTF("Stride length: %d cm, ", rscMeasurement.instStridelen);
                DBG_PRINTF("Total distance: %ld m, ", (rscMeasurement.totalDistance) / RSCS_DM_TO_M_VALUE);

                if(WALKING == profile)
                {
                    DBG_PRINTF("Status: Walking \r\n");
                }
                else
                {
                    DBG_PRINTF("Status: Running \r\n");
                }
            }
            else
            {
                /* Print info about occurred errors */
                if(CY_BLE_ERROR_INVALID_PARAMETER == apiResult)
                {
                    DBG_PRINTF("Cy_BLE_RSCSS_SendNotification() resulted with CY_BLE_ERROR_INVALID_PARAMETER\r\n");
                }
                else if(CY_BLE_ERROR_NTF_DISABLED == apiResult)
                {
                    DBG_PRINTF("Cy_BLE_RSCSS_SendNotification() resulted with CY_BLE_ERROR_NTF_DISABLED\r\n");
                }
                else
                {
                    DBG_PRINTF("Cy_BLE_RSCSS_SendNotification() resulted with CY_BLE_ERROR_INVALID_STATE\r\n");
                }
            }
        }
    }
}


/*******************************************************************************
* Function Name: RscIndications
********************************************************************************
*
* Summary:
*  Handles SC Control Point indications to the Client device. With this 
*  indication the Client receives a response for the previously send SC Control
*  Point Procedure. Refer to the Running Speed and Cadence Profile spec. for 
*  details on the SC Control Point Procedures.
* 
* Parameters:
*   connHandle - The connection handler.
*
*******************************************************************************/
void RscIndications(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    uint8_t i;
    uint8_t size = RSC_SC_CP_SIZE;
    uint8_t buff[RSC_SC_CP_SIZE + RSC_SENSORS_NUMBER];
    uint16_t cccd;

    apiResult = Cy_BLE_RSCSS_GetCharacteristicDescriptor(connHandle, CY_BLE_RSCS_SC_CONTROL_POINT, CY_BLE_RSCS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
              
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_RSCSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
    }
    else if((cccd == CY_BLE_CCCD_INDICATION) && (rscIndicationPending == true))
    {
        /* Handle the received SC Control Point Op Code */
        switch(rcsOpCode)
        {
            case CY_BLE_RSCS_REQ_SUPPORTED_SENSOR_LOCATION:
                buff[RSC_SC_CP_RESP_VAL_IDX] = rcsRespValue;
                if(rcsRespValue == CY_BLE_RSCS_ERR_SUCCESS)
                {
                    for(i = 0u; i < RSC_SENSORS_NUMBER; i++)
                    {
                        buff[RSC_SC_CP_RESP_PARAM_IDX + i] = rscSensors[i];
                    }

                    /* Update the size with number of supported sensors */
                    size += RSC_SENSORS_NUMBER;
                }
            case CY_BLE_RSCS_START_SENSOR_CALIBRATION:
            case CY_BLE_RSCS_SET_CUMMULATIVE_VALUE:
            case CY_BLE_RSCS_UPDATE_SENSOR_LOCATION:
                buff[RSC_SC_CP_RESP_VAL_IDX] = rcsRespValue;
                break;

            default:
                buff[RSC_SC_CP_RESP_VAL_IDX] = CY_BLE_RSCS_ERR_OP_CODE_NOT_SUPPORTED;
                break;
        }

        buff[RSC_SC_CP_RESP_OP_CODE_IDX] = CY_BLE_RSCS_RESPONSE_CODE;
        buff[RSC_SC_CP_REQ_OP_CODE_IDX] = rcsOpCode;
        
        do
        {
            Cy_BLE_ProcessEvents();
        }
        while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);
        
        if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
        {
            apiResult = Cy_BLE_RSCSS_SendIndication(connHandle, CY_BLE_RSCS_SC_CONTROL_POINT, size, buff);

            if(apiResult == CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_RSCSS_SendIndication() succeeded\r\n");
            }
            else
            {
                DBG_PRINTF("Cy_BLE_RSCSS_SendIndication() resulted with an error. Error code: %x\r\n", apiResult);
            }
        }  
        
        /* Clear the Op Code and the indication pending flag */
        rcsOpCode = RSC_SC_CP_INVALID_OP_CODE;
        rscIndicationPending = false;
    }
}


/*******************************************************************************
* Function Name: RscUpdatePace
********************************************************************************
*
* Summary:
*  Simulates "Running" or "Walking" profile.
*
*******************************************************************************/
void RscUpdatePace(void)
{
    /* Update stride length */
    if(rscMeasurement.instStridelen <= strideLenRanges[profile].max)
    {
        rscMeasurement.instStridelen++;
    }
    else
    {
        rscMeasurement.instStridelen = strideLenRanges[profile].min;
    }
    
    /* .. and cadence */
    if(rscMeasurement.instCadence <= cadenceRanges[profile].max)
    {
        rscMeasurement.instCadence++;
    }
    else
    {
        rscMeasurement.instCadence = cadenceRanges[profile].min;
    }
}


/*******************************************************************************
* Function Name: RscSwitchProfileMode
********************************************************************************
*
* Summary:
*   Switch between the "Running" and "Walking" profiles.
*
*******************************************************************************/
void RscSwitchProfileMode(void)
{
    if(Cy_BLE_GetNumOfActiveConn() != 0u)
    {
        if(profile == WALKING)
        {
            /* Update device with running simulation data */
            profile = RUNNING;
            rscMeasurement.flags |= RSC_FLAGS_WALK_RUN_STATUS_MASK;
            DBG_PRINTF("\r\nUpdate device with running simulation data \r\n");
        }
        else
        {
            /* Update device with walking simulation data */
            profile = WALKING;
            rscMeasurement.flags &= ~RSC_FLAGS_WALK_RUN_STATUS_MASK;
            DBG_PRINTF("\r\nUpdate device with walking simulation data \r\n");
        }
        
        rscMeasurement.instStridelen = strideLenRanges[profile].min;
        rscMeasurement.instCadence = cadenceRanges[profile].min;
    }    
}

        
/*******************************************************************************
* Function Name: RscGetProfileMode
********************************************************************************
*
* Summary:
*   This function returns mode of Rsc profile.
*
* Returns:
*  uint8_t - Rsc profile mode.
*******************************************************************************/
uint8_t RscGetProfileMode(void)
{
    return(profile);
}

/* [] END OF FILE */
