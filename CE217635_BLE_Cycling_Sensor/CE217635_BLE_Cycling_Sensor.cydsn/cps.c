/*******************************************************************************
* File Name: cps.c
*
* Version: 1.0
*
* Description:
*  This file contains CPS callback handler function.
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

#include "cps.h"

static cy_stc_ble_cps_power_measure_t powerMeasure;
static cy_stc_ble_cps_power_vector_t powerVector;

/* Data buffer for Control Point response. 
   First byte contains the length of response 
*/
static uint8_t powerCPData[CY_BLE_GATT_DEFAULT_MTU - 2u] = {3u, CY_BLE_CPS_CP_OC_RC, CY_BLE_CPS_CP_OC_SCV, CY_BLE_CPS_CP_RC_SUCCESS};


/*******************************************************************************
* Function Name: CpsCallBack()
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service specific events from 
*   Cycling Power Service.
*
* Parameters:
*  event - the event code
*  *eventParam - the event parameters
*
*******************************************************************************/
void CpsCallback(uint32_t event, void *eventParam)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    uint8_t i;
    uint8_t locCharIndex;
    locCharIndex = ((cy_stc_ble_cps_char_value_t *)eventParam)->charIndex;
    DBG_PRINTF("CPS event: %lx, ", event);

    switch(event)
    {
        /* CPS Server - Notifications for Cycling Power Service Characteristic
        was enabled. The parameter of this event is a structure of
        cy_stc_ble_cps_char_value_t type.
        */
        case CY_BLE_EVT_CPSS_NOTIFICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_CPSS_NOTIFICATION_ENABLED: char: %x\r\n", locCharIndex);
            break;
        
        /* CPS Server - Notifications for Cycling Power Service Characteristic
            was disabled. The parameter of this event is a structure 
            of cy_stc_ble_cps_char_value_t type
        */
        case CY_BLE_EVT_CPSS_NOTIFICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_CPSS_NOTIFICATION_DISABLED: char: %x\r\n", locCharIndex);
            break;
        
        /* CPS Server - Indication for Cycling Power Service Characteristic
            was enabled. The parameter of this event is a structure 
            of cy_stc_ble_cps_char_value_t type
        */
        case CY_BLE_EVT_CPSS_INDICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_CPSS_INDICATION_ENABLED: char: %x\r\n", locCharIndex);
            break;
        
        /* CPS Server - Indication for Cycling Power Service Characteristic
            was disabled. The parameter of this event is a structure 
            of cy_stc_ble_cps_char_value_t type
        */
        case CY_BLE_EVT_CPSS_INDICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_CPSS_INDICATION_DISABLED: char: %x\r\n", locCharIndex);
            break;
        
        /* CPS Server - Cycling Power Service Characteristic
            Indication was confirmed. The parameter of this event
            is a structure of cy_stc_ble_cps_char_value_t type
        */
        case CY_BLE_EVT_CPSS_INDICATION_CONFIRMED:
            DBG_PRINTF("CY_BLE_EVT_CPSS_INDICATION_CONFIRMED: char: %x\r\n", locCharIndex);
            break;
        
        /* CPS Server - Broadcast for Cycling Power Service Characteristic
            was enabled. The parameter of this event
            is a structure of cy_stc_ble_cps_char_value_t type
        */
        case CY_BLE_EVT_CPSS_BROADCAST_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_CPSS_BROADCAST_ENABLED: char: %x\r\n", locCharIndex);
            break;
        
        /* CPS Server - Broadcast for Cycling Power Service Characteristic
            was disabled. The parameter of this event
            is a structure of cy_stc_ble_cps_char_value_t type
        */
        case CY_BLE_EVT_CPSS_BROADCAST_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_CPSS_BROADCAST_DISABLED: char: %x\r\n", locCharIndex);
            Cy_BLE_CPSS_StopBroadcast();
            DBG_PRINTF("Stop Broadcast \r\n");
            break;
        
        /* CPS Server - Write Request for Cycling Power Service Characteristic 
            was received. The parameter of this event is a structure
            of cy_stc_ble_cps_char_value_t type.
        */
        case CY_BLE_EVT_CPSS_WRITE_CHAR:
            DBG_PRINTF("CY_BLE_EVT_CPSS_CHAR_WRITE: %x ", locCharIndex);
            ShowValue(((cy_stc_ble_cps_char_value_t *)eventParam)->value);
            if(locCharIndex == CY_BLE_CPS_POWER_CP)
            {
                uint8_t cpOpCode;
                cpOpCode = ((cy_stc_ble_cps_char_value_t *)eventParam)->value->val[0];
                /* Prepare general response */
                powerCPData[CPS_CP_RESP_LENGTH] = 3u; /* Length of response */
                powerCPData[CPS_CP_RESP_OP_CODES] = CY_BLE_CPS_CP_OC_RC;
                powerCPData[CPS_CP_RESP_REQUEST_OP_CODE] = cpOpCode;
                powerCPData[CPS_CP_RESP_VALUE] = CY_BLE_CPS_CP_RC_SUCCESS;
            
                DBG_PRINTF("CP Opcode %x: ", cpOpCode);
                switch(cpOpCode)
                {
                    case CY_BLE_CPS_CP_OC_SCV:
                        DBG_PRINTF("Set Cumulative Value \r\n");
                        /* Initiate the procedure to set a cumulative value. The new value is sent as parameter 
                           following op code (parameter defined per service). The response to this control point is 
                           Op Code 0x20 followed by the appropriate Response Value. */
                        if(((cy_stc_ble_cps_char_value_t *)eventParam)->value->len == (sizeof(uint32) + 1u))
                        {
                            powerMeasure.cumulativeWheelRevolutions = *(uint32_t *)&((cy_stc_ble_cps_char_value_t *)eventParam)->value->val[1];
                        }
                        else
                        {
                            powerCPData[CPS_CP_RESP_VALUE] = CY_BLE_CPS_CP_RC_INVALID_PARAMETER;                            
                        }
                        break;
                    case CY_BLE_CPS_CP_OC_USL:      
                        DBG_PRINTF("Update Sensor Location \r\n");
                        /* Update to the location of the Sensor with the value sent as parameter to this op code. 
                           The response to this control point is Op Code 0x20 followed by the appropriate Response 
                           Value. */
                        if(((cy_stc_ble_cps_char_value_t *)eventParam)->value->val[1] < CY_BLE_CPS_SL_COUNT)
                        {
                            apiResult = Cy_BLE_CPSS_SetCharacteristicValue(CY_BLE_CPS_SENSOR_LOCATION, sizeof(uint8_t), &((cy_stc_ble_cps_char_value_t *)eventParam)->value->val[1]);
                        }
                        else
                        {
                            powerCPData[CPS_CP_RESP_VALUE] = CY_BLE_CPS_CP_RC_INVALID_PARAMETER;                            
                        }
                        DBG_PRINTF("Cy_BLE_CPSS_SetCharacteristicValue SENSOR_LOCATION, API result: %x \r\n", apiResult);
                        break;
                    case CY_BLE_CPS_CP_OC_RSSL: 
                        DBG_PRINTF("Request Supported Sensor Locations \r\n");
                        /* Request a list of supported locations where the Sensor can be attached. The response to this
                           control point is Op Code 0x20 followed by the appropriate Response Value, including a list
                           of supported Sensor locations in the Response Parameter. */
                        powerCPData[CPS_CP_RESP_LENGTH] += CY_BLE_CPS_SL_COUNT; /* Length of response */
                        for(i = 0; i < CY_BLE_CPS_SL_COUNT; i++)
                        {
                            powerCPData[CPS_CP_RESP_PARAMETER + i] = i;
                        }
                        
                        break;
                    case CY_BLE_CPS_CP_OC_SCRL: 
                        DBG_PRINTF("Set Crank Length \r\n");
                        /* Initiate the procedure to set the crank length value to Sensor. The new value is sent as a 
                           parameter with preceding Op Code 0x04 operand. The response to this control point is Op Code
                           0x20 followed by the appropriate Response Value. */
                        if(((cy_stc_ble_cps_char_value_t *)eventParam)->value->len == (sizeof(uint16_t) + 1u))
                        {
                            cy_ble_cpssAdjustment.crankLength =  *(uint16_t *)&((cy_stc_ble_cps_char_value_t *)eventParam)->value->val[1];
                        }
                        else
                        {
                            powerCPData[CPS_CP_RESP_VALUE] = CY_BLE_CPS_CP_RC_INVALID_PARAMETER;                            
                        }
                        break;
                    case CY_BLE_CPS_CP_OC_RCRL:
                        DBG_PRINTF(" Request Crank Length \r\n");
                        powerCPData[CPS_CP_RESP_LENGTH] += sizeof(cy_ble_cpssAdjustment.crankLength); /* Length of response */
                        Cy_BLE_Set16ByPtr(powerCPData + CPS_CP_RESP_PARAMETER, cy_ble_cpssAdjustment.crankLength);
                        break;
                    case CY_BLE_CPS_CP_OC_SCHL:
                        DBG_PRINTF("Set Chain Length \r\n");
                        if(((cy_stc_ble_cps_char_value_t *)eventParam)->value->len == (sizeof(uint16_t) + 1u))
                        {
                            cy_ble_cpssAdjustment.chainLength =  *(uint16_t *)&((cy_stc_ble_cps_char_value_t *)eventParam)->value->val[1];
                        }
                        else
                        {
                            powerCPData[CPS_CP_RESP_VALUE] = CY_BLE_CPS_CP_RC_INVALID_PARAMETER;                            
                        }
                        break;
                    case CY_BLE_CPS_CP_OC_RCHL:
                        DBG_PRINTF("Request Chain Length \r\n");
                        powerCPData[CPS_CP_RESP_LENGTH] += sizeof(cy_ble_cpssAdjustment.chainLength); /* Length of response */
                        Cy_BLE_Set16ByPtr(powerCPData + CPS_CP_RESP_PARAMETER, cy_ble_cpssAdjustment.chainLength);
                        break;
                    case CY_BLE_CPS_CP_OC_SCHW:
                        DBG_PRINTF("Set Chain Weight \r\n");
                        if(((cy_stc_ble_cps_char_value_t *)eventParam)->value->len == (sizeof(uint16_t) + 1u))
                        {
                            cy_ble_cpssAdjustment.chainWeight =  *(uint16_t *)&((cy_stc_ble_cps_char_value_t *)eventParam)->value->val[1];
                        }
                        else
                        {
                            powerCPData[CPS_CP_RESP_VALUE] = CY_BLE_CPS_CP_RC_INVALID_PARAMETER;                            
                        }
                        break;
                    case CY_BLE_CPS_CP_OC_RCHW:
                        DBG_PRINTF("Request Chain Weight \r\n");
                        powerCPData[CPS_CP_RESP_LENGTH] += sizeof(cy_ble_cpssAdjustment.chainWeight); /* Length of response */
                        Cy_BLE_Set16ByPtr(powerCPData + CPS_CP_RESP_PARAMETER, cy_ble_cpssAdjustment.chainWeight);
                        break;
                    case CY_BLE_CPS_CP_OC_SSL:
                        DBG_PRINTF("Set Span Length \r\n");
                        if(((cy_stc_ble_cps_char_value_t *)eventParam)->value->len == (sizeof(uint16_t) + 1u))
                        {
                            cy_ble_cpssAdjustment.spanLength =  *(uint16_t *)&((cy_stc_ble_cps_char_value_t *)eventParam)->value->val[1];
                        }
                        else
                        {
                            powerCPData[CPS_CP_RESP_VALUE] = CY_BLE_CPS_CP_RC_INVALID_PARAMETER;                            
                        }
                        break;
                    case CY_BLE_CPS_CP_OC_RSL:
                        DBG_PRINTF(" Request Span Length \r\n");
                        powerCPData[CPS_CP_RESP_LENGTH] += sizeof(cy_ble_cpssAdjustment.spanLength); /* Length of response */
                        Cy_BLE_Set16ByPtr(powerCPData + CPS_CP_RESP_PARAMETER, cy_ble_cpssAdjustment.spanLength);
                        break;
                    case CY_BLE_CPS_CP_OC_SOC:
                        DBG_PRINTF("Start Offset Compensation \r\n");
                        powerCPData[CPS_CP_RESP_LENGTH] += sizeof(cy_ble_cpssAdjustment.offsetCompensation); /* Length of response */
                        Cy_BLE_Set16ByPtr(powerCPData + CPS_CP_RESP_PARAMETER, cy_ble_cpssAdjustment.offsetCompensation);
                        break;
                    case CY_BLE_CPS_CP_OC_MCPMCC:
                        DBG_PRINTF("Mask Cycling Power Measurement Characteristic Content \r\n");
                        { 
                            uint16_t mask = *(uint16_t *)&((cy_stc_ble_cps_char_value_t *)eventParam)->value->val[1];
                            if((mask & CY_BLE_CPS_CP_ENERGY_RESERVED) != 0u)
                            {
                                powerCPData[CPS_CP_RESP_VALUE] = CY_BLE_CPS_CP_RC_INVALID_PARAMETER;                            
                            }

                            if((mask & CY_BLE_CPS_CP_PEDAL_PRESENT_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CY_BLE_CPS_CPM_PEDAL_PRESENT_BIT;
                            }
                            if((mask & CY_BLE_CPS_CP_TORQUE_PRESENT_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CY_BLE_CPS_CPM_TORQUE_PRESENT_BIT;
                            }
                            if((mask & CY_BLE_CPS_CP_WHEEL_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CY_BLE_CPS_CPM_WHEEL_BIT;
                            }
                            if((mask & CY_BLE_CPS_CP_CRANK_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CY_BLE_CPS_CPM_CRANK_BIT;
                            }
                            if((mask & CY_BLE_CPS_CP_MAGNITUDES_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CY_BLE_CPS_CPM_FORCE_MAGNITUDES_BIT;
                                powerMeasure.flags &= ~CY_BLE_CPS_CPM_TORQUE_MAGNITUDES_BIT;
                            }
                            if((mask & CY_BLE_CPS_CP_ANGLES_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CY_BLE_CPS_CPM_ANGLES_BIT;
                            }
                            if((mask & CY_BLE_CPS_CP_TOP_DEAD_SPOT_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CY_BLE_CPS_CPM_TOP_DEAD_SPOT_BIT;
                            }
                            if((mask & CY_BLE_CPS_CP_BOTTOM_DEAD_SPOT_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CY_BLE_CPS_CPM_BOTTOM_DEAD_SPOT_BIT;
                            }
                            if((mask & CY_BLE_CPS_CP_ENERGY_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CY_BLE_CPS_CPM_ENERGY_BIT;
                            }
                        }
                        break;
                    case CY_BLE_CPS_CP_OC_RSR: 
                        DBG_PRINTF("Request Sampling Rate \r\n");
                        powerCPData[CPS_CP_RESP_LENGTH] += sizeof(cy_ble_cpssAdjustment.samplingRate); /* Length of response */
                        powerCPData[CPS_CP_RESP_PARAMETER] = cy_ble_cpssAdjustment.samplingRate;
                        break;
                    case CY_BLE_CPS_CP_OC_RFCD:
                        DBG_PRINTF("Request Factory Calibration Date \r\n");
                        powerCPData[CPS_CP_RESP_LENGTH] += sizeof(cy_ble_cpssAdjustment.factoryCalibrationDate); /* Length of response */
                        memcpy(powerCPData + CPS_CP_RESP_PARAMETER, &cy_ble_cpssAdjustment.factoryCalibrationDate, 
                               sizeof(cy_ble_cpssAdjustment.factoryCalibrationDate));
                        break;
                    case CY_BLE_CPS_CP_OC_RC:
                        DBG_PRINTF("Response Code \r\n");
                        break;
                    default:
                        DBG_PRINTF("Op Code Not supported \r\n");
                        powerCPData[CPS_CP_RESP_VALUE] = CY_BLE_CPS_CP_RC_NOT_SUPPORTED;
                        break;
                }
            }
            break;
        
        /* CPS Client - Cycling Power Service Characteristic
            Notification was received. The parameter of this event
            is a structure of cy_stc_ble_cps_char_value_t type
        */
        case CY_BLE_EVT_CPSC_NOTIFICATION:
            break;
        
        /* CPS Client - Cycling Power Service Characteristic
            Indication was received. The parameter of this event
            is a structure of cy_stc_ble_cps_char_value_t type
        */
        case CY_BLE_EVT_CPSC_INDICATION:
            break;
        
        /* CPS Client - Read Response for Read Request of Cycling Power Service
            Characteristic value. The parameter of this event
            is a structure of cy_stc_ble_cps_char_value_t type
        */
        case CY_BLE_EVT_CPSC_READ_CHAR_RESPONSE:
            break;

        /* CPS Client - Write Response for Write Request of Cycling Power Service
            Characteristic value. The parameter of this event
            is a structure of cy_stc_ble_cps_char_value_t type
        */
        case CY_BLE_EVT_CPSC_WRITE_CHAR_RESPONSE:
            break;
        
        /* CPS Client - Read Response for Read Request of Cycling Power
            Service Characteristic Descriptor Read request. The 
            parameter of this event is a structure of
            cy_stc_ble_cps_descr_value_t type
        */
        case CY_BLE_EVT_CPSC_READ_DESCR_RESPONSE:
            break;
        
        /* CPS Client - Write Response for Write Request of Cycling Power
            Service Characteristic Configuration Descriptor value.
            The parameter of this event is a structure of 
            cy_stc_ble_cps_descr_value_t type
        */
        case CY_BLE_EVT_CPSC_WRITE_DESCR_RESPONSE:
            break;

        /* CPS Client - This event is triggered every time a device receive
            non-connectable undirected advertising event.
            The parameter of this event is a structure of 
            cy_stc_ble_cps_char_value_t type
        */
        case CY_BLE_EVT_CPSC_SCAN_PROGRESS_RESULT:
            break;
            
        default:
            DBG_PRINTF("Not supported event\r\n");
            break;
    }
}


/*******************************************************************************
* Function Name: CpsInit()
********************************************************************************
*
* Summary:
*   Initializes the SCP service.
*
*******************************************************************************/
void CpsInit(void)
{
    cy_en_ble_api_result_t apiResult;
    cy_en_ble_cps_sl_value_t sensorLocation = CY_BLE_CPS_SL_TOP_OF_SHOE;
    
    /* Register service specific callback function */
    Cy_BLE_CPS_RegisterAttrCallback(&CpsCallback);
    
    /* Read flags of Power Measure characteristic default value */
    apiResult = Cy_BLE_CPSS_GetCharacteristicValue(CY_BLE_CPS_POWER_MEASURE, sizeof(powerMeasure.flags), 
        (uint8_t *)&powerMeasure);
    if((apiResult != CY_BLE_SUCCESS))
    {
        DBG_PRINTF("Cy_BLE_CPSS_GetCharacteristicValue API Error: %x \r\n", apiResult);
    }
    
    /* Read flags of Power Vector characteristic default value */
    apiResult = Cy_BLE_CPSS_GetCharacteristicValue(CY_BLE_CPS_POWER_VECTOR, sizeof(powerVector.flags), 
        (uint8_t *)&powerVector);
    if((apiResult != CY_BLE_SUCCESS))
    {
        DBG_PRINTF("Cy_BLE_CPSS_GetCharacteristicValue API Error: %x \r\n", apiResult);
    }

    /* Set default values which might be placed in EEPROM by application */
    cy_ble_cpssAdjustment.factoryCalibrationDate.year = 2016;
    cy_ble_cpssAdjustment.factoryCalibrationDate.day = 12;
    cy_ble_cpssAdjustment.factoryCalibrationDate.month = 12;
    
    powerMeasure.instantaneousPower = CPS_SIM_POWER_INIT; 
    powerMeasure.accumulatedTorque = CPS_SIM_TORQUE_INIT;
    powerMeasure.cumulativeWheelRevolutions = CPS_SIM_CUMULATIVE_WHEEL_REVOLUTION_INIT;
    powerMeasure.lastWheelEventTime = CPS_SIM_WHEEL_EVENT_TIME_INIT;
    powerMeasure.accumulatedEnergy = CPS_SIM_ACCUMULATED_ENERGY_INIT;
    
    powerVector.cumulativeCrankRevolutions += CPS_SIM_CUMULATIVE_CRANK_REVOLUTION_INIT;
    powerVector.lastCrankEventTime += CPS_SIM_CRANK_EVENT_TIME_INIT;
    
    Cy_BLE_CPSS_SetCharacteristicValue(CY_BLE_CPS_SENSOR_LOCATION, sizeof(uint8_t), (uint8_t *)&sensorLocation);
}


/*******************************************************************************
* Function Name: SimulateCyclingPower()
********************************************************************************
*
* Summary:
*   This function measures the die temperature and sends it to the client.
*
*******************************************************************************/
void CpsSimulateCyclingPower(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    uint8_t powerMeasureData[CY_BLE_GATT_DEFAULT_MTU - 3];
    uint8_t length = 0;
    uint16_t cccd = CY_BLE_CCCD_DEFAULT;
    uint16_t sccd;
    
    /* Prepare data array */
    Cy_BLE_Set16ByPtr(powerMeasureData, (CY_BLE_CPS_CPM_TORQUE_PRESENT_BIT |
                                         CY_BLE_CPS_CPM_TORQUE_SOURCE_BIT |
                                         CY_BLE_CPS_CPM_WHEEL_BIT |
                                         CY_BLE_CPS_CPM_ENERGY_BIT) & powerMeasure.flags);
    length += sizeof(powerMeasure.flags);
    Cy_BLE_Set16ByPtr(powerMeasureData + length, powerMeasure.instantaneousPower);
    length += sizeof(powerMeasure.instantaneousPower);
    if((CY_BLE_CPS_CPM_TORQUE_PRESENT_BIT & powerMeasure.flags) != 0u)
    {
        Cy_BLE_Set16ByPtr(powerMeasureData + length, powerMeasure.accumulatedTorque);
        length += sizeof(uint16_t);
        Cy_BLE_Set16ByPtr(powerMeasureData + length, (uint16_t)powerMeasure.cumulativeWheelRevolutions);
        Cy_BLE_Set16ByPtr(powerMeasureData + length+2, (uint16_t)(powerMeasure.cumulativeWheelRevolutions >> 16u));
        length += sizeof(powerMeasure.cumulativeWheelRevolutions);
        Cy_BLE_Set16ByPtr(powerMeasureData + length, powerMeasure.lastWheelEventTime);
        length += sizeof(powerMeasure.lastWheelEventTime);
    }
    if((CY_BLE_CPS_CPM_ENERGY_BIT & powerMeasure.flags) != 0u)
    {
        Cy_BLE_Set16ByPtr(powerMeasureData + length, powerMeasure.accumulatedEnergy);
        length += sizeof(uint16_t);
    }
        
    /* Send data */
    apiResult = Cy_BLE_CPSS_GetCharacteristicDescriptor(connHandle, CY_BLE_CPS_POWER_MEASURE, CY_BLE_CPS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
          
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_CPSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
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
            apiResult = Cy_BLE_CPSS_SendNotification(connHandle, CY_BLE_CPS_POWER_MEASURE, length, powerMeasureData);
            DBG_PRINTF("CpssSendNotification POWER_MEASURE, Power: %d W, ", powerMeasure.instantaneousPower);
            DBG_PRINTF("Torque: %ld, Wheel Revolution: %ld, Time: %d s, Speed: %3.2f km/h, ", 
                powerMeasure.accumulatedTorque / 32u,
                powerMeasure.cumulativeWheelRevolutions,
                powerMeasure.lastWheelEventTime / CPS_WHEEL_EVENT_TIME_PER_SEC,
                (float)CPS_SIM_CUMULATIVE_WHEEL_REVOLUTION_INCREMENT * CPS_WHEEL_CIRCUMFERENCE /
                CPS_SIM_WHEEL_EVENT_TIME_INCREMENT * CPS_WHEEL_EVENT_TIME_PER_SEC * CPS_SEC_IN_HOUR);
            DBG_PRINTF("Energy: %ld kJ \r\n", powerMeasure.accumulatedEnergy);
            
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("CpssSendNotification API Error: %x \r\n", apiResult);
            }
        }
    }
    
    apiResult = Cy_BLE_CPSS_GetCharacteristicDescriptor(connHandle, CY_BLE_CPS_POWER_MEASURE, CY_BLE_CPS_SCCD, CY_BLE_CCCD_LEN, (uint8_t*)&sccd);
          
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_CPSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
    }
    else if(sccd == CY_BLE_SCCD_BROADCAST) 
    {
        do
        {
            Cy_BLE_ProcessEvents();
        }
        while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);
        
        if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
        {    
            apiResult = Cy_BLE_CPSS_StartBroadcast(CPS_ADVERT_INTERVAL_NONCON, length, powerMeasureData);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_CPSS_StartBroadcast, API result: %x \r\n", apiResult);
            }
        }
    }
    
    apiResult = Cy_BLE_CPSS_GetCharacteristicDescriptor(connHandle, CY_BLE_CPS_POWER_VECTOR, CY_BLE_CPS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
          
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_CPSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
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
            uint8_t powerVectorData[CPS_POWER_VECTOR_DATA_MAX_SIZE];
            length = 0;
            
            Cy_BLE_Set16ByPtr(powerVectorData,  powerVector.flags);
            length += sizeof(powerVector.flags);
            Cy_BLE_Set16ByPtr(powerVectorData + length, powerVector.cumulativeCrankRevolutions);
            length += sizeof(powerVector.cumulativeCrankRevolutions);
            Cy_BLE_Set16ByPtr(powerVectorData + length, powerVector.lastCrankEventTime);
            length += sizeof(powerVector.lastCrankEventTime);

            apiResult = Cy_BLE_CPSS_SendNotification(connHandle, CY_BLE_CPS_POWER_VECTOR, length , powerVectorData);
            DBG_PRINTF("CpssSendNotification POWER_VECTOR, Crank Revolution: %d W, ", powerVector.cumulativeCrankRevolutions);
            DBG_PRINTF("Time: %d s, Cadence: %d rpm \r\n", powerVector.lastCrankEventTime / CPS_CRANK_EVENT_TIME_PER_SEC,
                CPS_SIM_CUMULATIVE_CRANK_REVOLUTION_INCREMENT / (CPS_SIM_CRANK_EVENT_TIME_INCREMENT / CPS_CRANK_EVENT_TIME_PER_SEC));
            
            if((apiResult != CY_BLE_SUCCESS))
            {
                DBG_PRINTF("CpssSendNotification API Error: %x \r\n", apiResult);
            }
        }
    }
    
    apiResult = Cy_BLE_CPSS_GetCharacteristicDescriptor(connHandle, CY_BLE_CPS_POWER_CP, CY_BLE_CPS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
          
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_CPSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
    }
    else if(cccd == CY_BLE_CCCD_INDICATION) 
    {
        do
        {
            Cy_BLE_ProcessEvents();
        }
        while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);
        
        if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
        {
            apiResult = Cy_BLE_CPSS_SendIndication(connHandle, CY_BLE_CPS_POWER_CP, 
                                                 powerCPData[CPS_CP_RESP_LENGTH], powerCPData + 1u);
            DBG_PRINTF("Cy_BLE_CPSS_SendIndication POWER_CP, API result: %x \r\n", apiResult);
        }
    }
    
    /* Simulate data */
    powerMeasure.instantaneousPower++;
    powerMeasure.accumulatedTorque += CPS_SIM_TORQUE_INCREMENT;
    powerMeasure.cumulativeWheelRevolutions += CPS_SIM_CUMULATIVE_WHEEL_REVOLUTION_INCREMENT;
    powerMeasure.lastWheelEventTime += CPS_SIM_WHEEL_EVENT_TIME_INCREMENT;
    powerMeasure.accumulatedEnergy += CPS_SIM_ACCUMULATED_ENERGY_INCREMENT;
    
    powerVector.cumulativeCrankRevolutions += CPS_SIM_CUMULATIVE_CRANK_REVOLUTION_INCREMENT;
    powerVector.lastCrankEventTime += CPS_SIM_CRANK_EVENT_TIME_INCREMENT;

}

/* [] END OF FILE */

