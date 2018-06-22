/*******************************************************************************
* File Name: ess.c
*
* Version: 1.0
*
* Description:
*  This file contains routines related to Environmental Sensing Service.
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
#include "ess.h"

/***************************************
*        Global Variables
***************************************/

static uint16_t indicationValue;
static uint16_t essChangeIndex = 0u;

static const uint8_t defaultSensorCond[NUMBER_OF_TRIGGERS] =
    {CY_BLE_ESS_TRIG_NO_LESS_THEN_TIME_INTERVAL, CY_BLE_ESS_TRIG_WHEN_CHANGED,CY_BLE_ESS_TRIG_TRIGGER_INACTIVE};

extern cy_stc_ble_ess_characteristic_data_t humidity;
extern cy_stc_ble_ess_characteristic_data_t windSpeed[SIZE_2_BYTES];


/*******************************************************************************
* Function Name: EssInit
********************************************************************************
*
* Summary: This function initialized parameters for Environmental Sensing Service.
*
*******************************************************************************/
void EssInit(void)
{
    
    /* Set initial Change Index value */
    Cy_BLE_ESSS_SetChangeIndex(essChangeIndex, 0u);

    windSpeed[0].EssChrIndex = CY_BLE_ESS_TRUE_WIND_SPEED;
    windSpeed[0].chrInstance = CHARACTERISTIC_INSTANCE_1;
    EssInitCharacteristic(&windSpeed[0]);
    
    windSpeed[1].EssChrIndex = CY_BLE_ESS_TRUE_WIND_SPEED;
    windSpeed[1].chrInstance = CHARACTERISTIC_INSTANCE_2;
    EssInitCharacteristic(&windSpeed[1]);
    
    humidity.EssChrIndex = CY_BLE_ESS_HUMIDITY;
    humidity.chrInstance = CHARACTERISTIC_INSTANCE_1;
    EssInitCharacteristic(&humidity);
}

/*******************************************************************************
* Function Name: EssCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE Component
*   specific to Environmental Sensing Service.
*
* Parameters:
*   event: An ESS event.
*   eventParams: The data structure specific to an event received.
*
*******************************************************************************/
void EssCallBack(uint32_t event, void *eventParam)
{
    cy_stc_ble_ess_char_value_t * charValPtr;
    cy_stc_ble_ess_descr_value_t * descrValPtr;

    switch(event)
    {
        /****************************************************
        *              ESS Server events
        ****************************************************/
        
        /* ESS Server - Notifications for Environmental Sensing Service
         * Characteristics were enabled. The parameter of this event is a structure of
         * the cy_stc_ble_ess_char_value_t type.
         */
        case CY_BLE_EVT_ESSS_NOTIFICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_ESSS_NOTIFICATION_ENABLED\r\n");
            charValPtr = (cy_stc_ble_ess_char_value_t *) eventParam;
            DBG_PRINTF("Char instance: %d \r\n.",charValPtr->charInstance + 1u);
            break;
            
        /* ESS Server - Notifications for Environmental Sensing Service
         * Characteristics were disabled. The parameter of this event is a structure of
         * the cy_stc_ble_ess_char_value_t type.
         */
        case CY_BLE_EVT_ESSS_NOTIFICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_ESSS_NOTIFICATION_DISABLED\r\n");
            charValPtr = (cy_stc_ble_ess_char_value_t *) eventParam;
            DBG_PRINTF("Char instance: %d \r\n",charValPtr->charInstance + 1u);
            break;

        /* ESS Server - The indication for Descriptor Value Changed Characteristic
         * was enabled. The parameter of this event is a structure of
         * the cy_stc_ble_ess_char_value_t type.
         */
        case CY_BLE_EVT_ESSS_INDICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_ESSS_INDICATION_ENABLED\r\n");
            charValPtr = (cy_stc_ble_ess_char_value_t *) eventParam;
            DBG_PRINTF("\r\n");
            break;

        /* ESS Server - The indication for Descriptor Value Changed Characteristic
         * was disabled. The parameter of this event is a structure of
         * the cy_stc_ble_ess_char_value_t type.
         */
        case CY_BLE_EVT_ESSS_INDICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_ESSS_INDICATION_DISABLED\r\n");
            charValPtr = (cy_stc_ble_ess_char_value_t *) eventParam;
            DBG_PRINTF("\r\n");
            break;

        /* ESS Server - The indication for Descriptor Value Changed Characteristic
         * was confirmed. The parameter of this event is a structure of 
         * the cy_stc_ble_ess_char_value_t type.
         */
        case CY_BLE_EVT_ESSS_INDICATION_CONFIRMATION:
            DBG_PRINTF("CY_BLE_EVT_ESSS_INDICATION_CONFIRMATION\r\n");
            break;

        /* ESS Server - A write request for Environmental Sensing Service
         * Characteristic was received. The parameter of this event is a structure of
         * the cy_stc_ble_ess_char_value_t type.
         */    
        case CY_BLE_EVT_ESSS_WRITE_CHAR:
            DBG_PRINTF("CY_BLE_EVT_ESSS_CHAR_WRITE\r\n");
            break;

        /* ESS Server - A write request for Environmental Sensing Service
         * characteristic descriptor was received. The parameter of this event is a structure of
         * the cy_stc_ble_ess_descr_value_t type. This event is generated only when a write for
         * CY_BLE_ESS_CHAR_USER_DESCRIPTION_DESCR, CY_BLE_ESS_ES_TRIGGER_SETTINGS_DESCR or
         * CY_BLE_ESS_ES_CONFIG_DESCR has occurred.
         */    
        case CY_BLE_EVT_ESSS_DESCR_WRITE:
            DBG_PRINTF("CY_BLE_EVT_ESSS_DESCR_WRITE\r\n");
            descrValPtr = (cy_stc_ble_ess_descr_value_t *) eventParam;
            DBG_PRINTF(" %s Char instance: %d", EssCharIndexToText(descrValPtr->charIndex), descrValPtr->charInstance + 1u);
            DBG_PRINTF(" Descriptor: %d. ", descrValPtr->descrIndex);
            EssHandleDescriptorWriteOp(descrValPtr);
            break;

        /****************************************************
        *               ESS Client events
        * These events are not active for the Server device
        * operation. They are added as a template for the
        * Client mode operation to ease the user experience
        * with the Environmental Sensing Service.
        ****************************************************/
        /* ESS Client - Environmental Sensing Service Characteristic
        * Notification was received. The parameter of this event is a structure of
        * cy_stc_ble_ess_char_value_t type.
        */
        case CY_BLE_EVT_ESSC_NOTIFICATION:
            /* Add code to handle notification from the Server */
            break;

        /* ESS Client - Environmental Sensing Service Characteristic
        * Indication was received. The parameter of this event is a structure of 
        * cy_stc_ble_ess_char_value_t type.
        */
        case CY_BLE_EVT_ESSC_INDICATION:
            /* Add code to handle indication from the Server */
            break;

        /* ESS Client - Read Response for Read Request of Environmental Sensing 
        * Service Characteristic value. The parameter of this event is a structure of
        * cy_stc_ble_ess_char_value_t type.
        */
        case CY_BLE_EVT_ESSC_READ_CHAR_RESPONSE:
            /* Add code to handle read characteristic response from the Server */
            break;

        /* ESS Client - Write Response for Write Request of Environmental Sensing 
        * Service Characteristic value. The parameter of this event is a structure of
        * cy_stc_ble_ess_char_value_t type.
        */
        case CY_BLE_EVT_ESSC_WRITE_CHAR_RESPONSE:
            /* Add code to handle write characteristic response from the Server */
            break;

        /* ESS Client - Read Response for Read Request of Environmental Sensing
        * Service Characteristic Descriptor Read request. The parameter of this event
        * is a structure of cy_stc_ble_ess_descr_value_t type.
        */
        case CY_BLE_EVT_ESSC_READ_DESCR_RESPONSE:
            /* Add code to handle read characteristic descriptor response from the Server */
            break;

        /* ESS Client - Write Response for Write Request of Environmental Sensing
        * Service Characteristic Configuration Descriptor value. The parameter of
        * this event is a structure of  cy_stc_ble_ess_descr_value_t type.
        */
        case CY_BLE_EVT_ESSC_WRITE_DESCR_RESPONSE:
            /* Add code to handle write characteristic descriptor response from the Server
            */
            break;

        default:
            DBG_PRINTF("Unrecognized ESSS event.\r\n");
            break;
    }
}

/*******************************************************************************
* Function Name: EssInitCharacteristic
********************************************************************************
*
* Summary:
*   Initializes the Environmental Sensing Service for single characteristic.
*
* Parameters:  
*   sensorPtr: A pointer to the sensor characteristic structure.
*
*******************************************************************************/
void EssInitCharacteristic(cy_stc_ble_ess_characteristic_data_t *sensorPtr)
{
    uint8_t i;
    uint8_t buff[SIZE_2_BYTES];
    ess_measurement_value_t esMeasurementDescrVal;
    
    /* localConnHandle is used for reading ESSS Descriptor (not CCCD descriptor). 
     * So, this parameter can be skipped ( = 0) in Cy_BLE_ESSS_GetCharacteristicDescriptor() 
     */
    cy_stc_ble_conn_handle_t localConnHandle = { .attId = 0u };
    
    /* A temporary store for trigger settings descriptor values. The trigger settings
     * descriptor value have a variable length, refer to section 3.1.2.2 of ESS spec.
     * For this example the max. length is 4 bytes.
     */
    uint8_t esTrigSettingsVal[SIZE_4_BYTES];

    
    switch(sensorPtr->EssChrIndex)
    {
        case CY_BLE_ESS_TRUE_WIND_SPEED :
            sensorPtr->value = INIT_WIND_SPEED;

            switch(sensorPtr->chrInstance)
            {   
                case CHARACTERISTIC_INSTANCE_1:
                    sensorPtr->valueMax = WIND_SPEED_MAX1;
                    sensorPtr->valueMin = WIND_SPEED_MIN1;
                    sensorPtr->valueUpdateStep = WIND_UPDATE_STEP_1;
                    break;
                
                case CHARACTERISTIC_INSTANCE_2:
                    sensorPtr->valueMax = WIND_SPEED_MAX2;
                    sensorPtr->valueMin = WIND_SPEED_MIN2;
                    sensorPtr->valueUpdateStep = WIND_UPDATE_STEP_2;                    
                    break;
                
                default :   /* Message */
                    DBG_PRINTF("Characteristic instance is incorrect. Characteristic instance: %d \r\n.",sensorPtr->chrInstance);
                    break;
            }
            break;
            
        case CY_BLE_ESS_HUMIDITY :
            sensorPtr->value = INIT_HUMIDITY;
            sensorPtr->valueMax = HUMIDITY_MAX;
            sensorPtr->valueMin = HUMIDITY_MIN;
            sensorPtr->valueUpdateStep = HUMIDITY_UPDATE_STEP;
            break;
        
        default : 
            DBG_PRINTF("Characteristic index is incorrect. Characteristic index: %d \r\n.",sensorPtr->EssChrIndex);
            break;    
    }
    
    sensorPtr->sensorNewDataReady = false;
    sensorPtr->isMeasurementPeriodElapsed = false;
    sensorPtr->prevValue = sensorPtr->value; 
   
    /* Register event handler for ESS specific events */
    Cy_BLE_ESS_RegisterAttrCallback(EssCallBack);

    /* Set initial value for parameter */
    Cy_BLE_Set16ByPtr(buff, sensorPtr->value);
    (void) Cy_BLE_ESSS_SetCharacteristicValue(sensorPtr->EssChrIndex, sensorPtr->chrInstance, SIZE_2_BYTES, buff);

    /* Get initial values for ES Configuration Descriptor of parameter*/
    (void) Cy_BLE_ESSS_GetCharacteristicDescriptor(localConnHandle, sensorPtr->EssChrIndex, sensorPtr->chrInstance,
                                                    CY_BLE_ESS_ES_CONFIG_DESCR, SIZE_1_BYTE, &sensorPtr->esConfig);
     
    for(i = 0u; i < NUMBER_OF_TRIGGERS; i++)
    {
        if (CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE == 
            Cy_BLE_ESSS_GetCharacteristicDescriptor(localConnHandle, sensorPtr->EssChrIndex, sensorPtr->chrInstance,
                                                     (cy_en_ble_ess_descr_index_t)(i + CY_BLE_ESS_ES_TRIGGER_SETTINGS_DESCR1),
                                                     SIZE_4_BYTES, esTrigSettingsVal))
        {   /* No settings for this trigger */
            sensorPtr->valueCond[i] = CY_BLE_ESS_TRIG_TRIGGER_INACTIVE;
            sensorPtr->cmpValue[i] = 0;
        } 
        else 
        {   /* Trigger is active */
            sensorPtr->valueCond[i] = esTrigSettingsVal[TRIG_CONDITION_OFFSET];
            if ( sensorPtr->valueCond[i] > CY_BLE_ESS_LAST_TRIG_CONDITION )
            { 
                sensorPtr->valueCond[i] = defaultSensorCond[i];
            }

            /* Pack compare value to buffer to be set to GATT database */
            GetUint24(&sensorPtr->cmpValue[i], &esTrigSettingsVal[TRIG_OPERAND_OFFSET]);
                
            if( ( sensorPtr->valueCond[i] == CY_BLE_ESS_TRIG_USE_FIXED_TIME_INTERVAL)
            ||  ( sensorPtr->valueCond[i] == CY_BLE_ESS_TRIG_NO_LESS_THEN_TIME_INTERVAL) )
            {    /* Pack notification timeout value to buffer to be set to GATT database */
                GetUint24(&sensorPtr->ntfTimeoutVal, &esTrigSettingsVal[TRIG_OPERAND_OFFSET]);
            }
            else
            {
                sensorPtr->ntfTimeoutVal = NTF_INIT_TIMEOUT_VAL;
            }
        }
    }    

    /* Get the value of ES measurement Descriptor */
    (void) Cy_BLE_ESSS_GetCharacteristicDescriptor(localConnHandle, sensorPtr->EssChrIndex, sensorPtr->chrInstance,
                                                    CY_BLE_ESS_ES_MEASUREMENT_DESCR, sizeof(esMeasurementDescrVal), 
                                                    (uint8_t *) &esMeasurementDescrVal);
    
    /* Store the measurement period value into uint32_t for easy access to it */
    GetUint24(&sensorPtr->measurementPeriod, &esMeasurementDescrVal.measurementPeriod[0u]);
    
    /* Store the update interval value into uint32_t for easy access to it */
    GetUint24(&sensorPtr->updateIntervalValue, &esMeasurementDescrVal.updateInterval[0u]);
    
    DBG_PRINTF("\r\n* The initialized Characteristic - %s instance #%d\r\n", 
                EssCharIndexToText(sensorPtr->EssChrIndex),sensorPtr->chrInstance+1); 
    DBG_PRINTF("* Value of imitated parameter          - %d\r\n", sensorPtr->value); 
    DBG_PRINTF("* Maximum value of imitated parameter  - %d\r\n", sensorPtr->valueMax); 
    DBG_PRINTF("* Minimum value of imitated parameter  - %d\r\n", sensorPtr->valueMin); 
    DBG_PRINTF("* Step of imitated parameter changing  - %d\r\n", sensorPtr->valueUpdateStep); 
    DBG_PRINTF("* Value of ES Configuration descriptor - %s\r\n", 
                (sensorPtr->esConfig == CY_BLE_ESS_CONF_BOOLEAN_AND) ? "AND" : "OR"); 
    
    DBG_PRINTF("* Notification timeout value           - %ld\r\n", sensorPtr->ntfTimeoutVal); 
    
    for(i = 0u; i < NUMBER_OF_TRIGGERS; i++)
    {    
        DBG_PRINTF("* Value condition, Comparison value #%d - %d, %ld\r\n", i, 
                    sensorPtr->valueCond[i],sensorPtr->cmpValue[i]); 
    }
    
    DBG_PRINTF("* Measurement period in seconds        - %ld\r\n", sensorPtr->measurementPeriod); 
    DBG_PRINTF("* Update Interval in seconds           - %ld\r\n\n", sensorPtr->updateIntervalValue); 
}


/*******************************************************************************
* Function Name: EssHandleDescriptorWriteOp()
********************************************************************************
*
* Summary:
*   Parses the data received with CY_BLE_EVT_ESSS_DESCR_WRITE and appropriately
*   updates the internal variables used for Environmental Profile simulation.
*
* Parameters:  
*   descrValPtr: A pointer to the data received with CY_BLE_EVT_ESSS_DESCR_WRITE
*                event.
*
*******************************************************************************/
void EssHandleDescriptorWriteOp(cy_stc_ble_ess_descr_value_t *descrValPtr)
{
    uint32_t i;
    
    switch (descrValPtr->descrIndex)
    { 
        /* This case is for three following conditions:
        *      CY_BLE_ESS_ES_TRIGGER_SETTINGS_DESCR1,
        *      CY_BLE_ESS_ES_TRIGGER_SETTINGS_DESCR2,        
        *      CY_BLE_ESS_ES_TRIGGER_SETTINGS_DESCR3.
        */
        case CY_BLE_ESS_ES_TRIGGER_SETTINGS_DESCR1:
        case CY_BLE_ESS_ES_TRIGGER_SETTINGS_DESCR2:        
        case CY_BLE_ESS_ES_TRIGGER_SETTINGS_DESCR3:         
        
            switch(descrValPtr->charIndex)
            {   
                case CY_BLE_ESS_TRUE_WIND_SPEED :
                    windSpeed[descrValPtr->charInstance].valueCond[descrValPtr->descrIndex-CY_BLE_ESS_ES_TRIGGER_SETTINGS_DESCR1]
                                            = descrValPtr->value->val[0u];
                    
                    windSpeed[descrValPtr->charInstance].cmpValue[descrValPtr->descrIndex-CY_BLE_ESS_ES_TRIGGER_SETTINGS_DESCR1]
                                            =   ((uint32) ((descrValPtr->value->val[3u] << SHIFT_16_BITS) |
                                                (descrValPtr->value->val[2u] << SHIFT_8_BITS) | 
                                                descrValPtr->value->val[1u]));
                                                    
                    if( (descrValPtr->value->val[0u] == CY_BLE_ESS_TRIG_USE_FIXED_TIME_INTERVAL) ||  
                        (descrValPtr->value->val[0u] == CY_BLE_ESS_TRIG_NO_LESS_THEN_TIME_INTERVAL) )
                    {   /* Update notification timer variables with new value */
                        windSpeed[descrValPtr->charInstance].ntfTimeoutVal = 
                         windSpeed[descrValPtr->charInstance].cmpValue[descrValPtr->descrIndex-CY_BLE_ESS_ES_TRIGGER_SETTINGS_DESCR1];                   
                    }
                    
                    break;
                
                case CY_BLE_ESS_HUMIDITY :
                    humidity.valueCond[descrValPtr->descrIndex-CY_BLE_ESS_ES_TRIGGER_SETTINGS_DESCR1]
                                            = descrValPtr->value->val[0u];
                    
                    humidity.cmpValue[descrValPtr->descrIndex-CY_BLE_ESS_ES_TRIGGER_SETTINGS_DESCR1] =   
                        ((uint32) ((descrValPtr->value->val[3u] << SHIFT_16_BITS) | 
                        (descrValPtr->value->val[2u] << SHIFT_8_BITS) |  descrValPtr->value->val[1u]));
                                                    
                    if( (descrValPtr->value->val[0u] == CY_BLE_ESS_TRIG_USE_FIXED_TIME_INTERVAL) ||  
                        (descrValPtr->value->val[0u] == CY_BLE_ESS_TRIG_NO_LESS_THEN_TIME_INTERVAL) )
                    {   /* Update notification timer variables with new value */
                        humidity.ntfTimeoutVal = humidity.cmpValue[descrValPtr->descrIndex-CY_BLE_ESS_ES_TRIGGER_SETTINGS_DESCR1];                   
                    }
                    
                    break;
                                                
                default : 
                    DBG_PRINTF("Characteristic index is incorrect. Characteristic index: %d \r\n.",descrValPtr->charIndex);
                    break;         
        
            }
        
            DBG_PRINTF("Received value is: ");
            for(i = 0u; i < descrValPtr->value->len; i++)
            {
                DBG_PRINTF("0x%2.2x ", descrValPtr->value->val[i]);
            }
            DBG_PRINTF("\r\n");
            SetIndicationPendingFlag(true);
            indicationValue = CY_BLE_ESS_VALUE_CHANGE_SOURCE_CLIENT | CY_BLE_ESS_VALUE_CHANGE_ES_TRIGGER;
            break;
        
        case CY_BLE_ESS_ES_CONFIG_DESCR: 
            switch(descrValPtr->charIndex)
            {   
                case CY_BLE_ESS_TRUE_WIND_SPEED :
                    windSpeed[descrValPtr->charInstance].esConfig = descrValPtr->value->val[0u];
                    break;
                
                case CY_BLE_ESS_HUMIDITY :
                    humidity.esConfig = descrValPtr->value->val[0u];
                    break;
                
                default : 
                    DBG_PRINTF("Characteristic index is incorrect. Characteristic index: %d \r\n.",descrValPtr->charIndex);
                    break;
            }
                    
            DBG_PRINTF("Received value is: 0x%2.2x\r\n", descrValPtr->value->val[0u]);
            SetIndicationPendingFlag(true);
            indicationValue = CY_BLE_ESS_VALUE_CHANGE_SOURCE_CLIENT | CY_BLE_ESS_VALUE_CHANGE_ES_CONFIG;
            break;
            
        case CY_BLE_ESS_CHAR_USER_DESCRIPTION_DESCR:
            DBG_PRINTF("Write to CY_BLE_ESS_CHAR_USER_DESCRIPTION_DESCR has occurred. Value is: \"");
            for(i = 0u; i < descrValPtr->value->len; i++)
            {
                DBG_PRINTF("%c", descrValPtr->value->val[i]);
            }
            DBG_PRINTF("\"\r\n");
            SetIndicationPendingFlag(true);
            indicationValue = CY_BLE_ESS_VALUE_CHANGE_SOURCE_CLIENT | CY_BLE_ESS_VALUE_CHANGE_USER_DESCRIPTION;
            break;
        default:
            break;
    }
}

/*******************************************************************************
* Function Name: EssSimulateProfile
********************************************************************************
*
* Summary:
*   Simulates a wind measurement based on the time periods specified in the ES 
*   Measurement descriptor. 
*
* Parameters:  
*   sensorPtr: A pointer to the sensor characteristic structure.
*   mainTimer: Value of simulation time
*
*******************************************************************************/
void EssSimulateProfile(cy_stc_ble_ess_characteristic_data_t *sensorPtr, uint32_t mainTimer)
{
    /* Following two condition checks are required for measurement period simulation */
    if((sensorPtr->isMeasurementPeriodElapsed == false) && (sensorPtr->measurementPeriod <= mainTimer))
    {
        sensorPtr->isMeasurementPeriodElapsed = true;
        DBG_PRINTF("Measurement Period for %s sensor#%d (%d s) has elapsed.\r\n",
            EssCharIndexToText(sensorPtr->EssChrIndex), sensorPtr->chrInstance + 1u, 
             CY_LO16(sensorPtr->measurementPeriod));
    }

    /* The first sensor simulates an increase in the wind speed by 1.2 m/s each
    * 15 seconds until it reaches the maximum of 80 m/s. Then the wind speed
    * falls down to the minimum of 10 m/s, and then again it is
    * increased by 1.2 m/s each 15 seconds.
    * The second sensor simulates an increase in the wind speed by 0.7 m/s each
    * 20 seconds until, it reaches the maximum of ~90 m/s. After that the speed 
    * is not updated any more holding the maximum wind speed.
    */
    if(sensorPtr->isMeasurementPeriodElapsed == true)
    {
        if(sensorPtr->updateIntervalTimer == 0u)
        {   
            sensorPtr->prevValue = sensorPtr->value;
            
            if(sensorPtr->valueMax > sensorPtr->value)
            {
                sensorPtr->value += sensorPtr->valueUpdateStep;
            }
            else if(sensorPtr->chrInstance == CHARACTERISTIC_INSTANCE_1)
            {
                sensorPtr->value = sensorPtr->valueMin;
            }
            else
            {
                /*  Value of CHARACTERISTIC_INSTANCE_2 is not changed */
            }
            sensorPtr->updateIntervalTimer = sensorPtr->updateIntervalValue;
            sensorPtr->sensorNewDataReady = true;
            /* Updated Change Index value as new data is available */
            essChangeIndex++;
            Cy_BLE_ESSS_SetChangeIndex(essChangeIndex, 0u);
            
            DBG_PRINTF("Update Interval for %s sensor#%d (%d s) has elapsed.\r\n",
                       EssCharIndexToText(sensorPtr->EssChrIndex),sensorPtr->chrInstance + 1u,
                        CY_LO16(sensorPtr->updateIntervalTimer));
        }
    }
}


/*******************************************************************************
* Function Name: EssHandleNotificaion
********************************************************************************
*
* Summary:
*   Sends a notification about a descriptor change to the Client.
*
* Parameters:
*   connHandle: The connection handle
*   sensorPtr:  A pointer to the sensor characteristic structure.
*
*******************************************************************************/
void EssHandleNotificaion(cy_stc_ble_conn_handle_t connHandle, cy_stc_ble_ess_characteristic_data_t *sensorPtr)
{
    cy_en_ble_api_result_t apiResult;
    uint8_t tmpBuff[CY_BLE_ESS_2BYTES_LENGTH];
    uint16_t cccd;

    apiResult = Cy_BLE_ESSS_GetCharacteristicDescriptor(connHandle, sensorPtr->EssChrIndex, sensorPtr->chrInstance, CY_BLE_ESS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
        
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_ESSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
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
                
            /* Pack data to BLE compatible format ... */
            Cy_BLE_Set16ByPtr(tmpBuff, sensorPtr->value);

            /* ... and send it */
            apiResult = Cy_BLE_ESSS_SendNotification(connHandle, sensorPtr->EssChrIndex, sensorPtr->chrInstance,
                                                      SIZE_2_BYTES, tmpBuff);

            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Send notification is failed: %d \r\n", apiResult);
            }
            else
            {   DBG_PRINTF("Notification for %s #%d was sent successfully. ",
                            EssCharIndexToText(sensorPtr->EssChrIndex), sensorPtr->chrInstance + 1u);
                DBG_PRINTF("Notified value is: %d.%d %s.\r\n", 
                    sensorPtr->value/100u, sensorPtr->value%100u,
                    (sensorPtr->EssChrIndex == CY_BLE_ESS_TRUE_WIND_SPEED) ? "m/s" : "%%");
                
                sensorPtr->sensorNewDataReady = false;
                sensorPtr->ntfTimer = sensorPtr->ntfTimeoutVal;
            }
        }
    }    
}


/*******************************************************************************
* Function Name: EssHandleNtfConditions
********************************************************************************
*
* Summary:
*   Verifies if the conditions captured in ES Trigger descriptors and ES 
*   Configuration descriptor were met and returns a final verdict to inform if
*   notifications are allowed to be sent.
*
* Parameters:  
*   sensorPtr: A pointer to the sensor characteristic structure.
*
* Return: 
*  The status identifying if all conditions for sending a notification for
*  True Speed #1 were met:
*    YES - All conditions were met.
*    NO - Not all conditions were met.
*
*******************************************************************************/
uint8_t EssHandleNtfConditions(cy_stc_ble_ess_characteristic_data_t *sensorPtr)
{
    uint8_t result = false;
    uint8_t isCondTrue[NUMBER_OF_TRIGGERS]     = {false, false, false};
    uint8_t isCondInactive[NUMBER_OF_TRIGGERS] = {false, false, false};
    uint8_t i;

    if(sensorPtr->sensorNewDataReady == true)
    {
        for (i = 0; i < NUMBER_OF_TRIGGERS; i++)
        { 
            switch (sensorPtr->valueCond[i])
            {
                case CY_BLE_ESS_TRIG_USE_FIXED_TIME_INTERVAL:  /* FIXED_TIME work same as NO_LESS_THEN_TIME_INTERVAL  */ 
                case CY_BLE_ESS_TRIG_NO_LESS_THEN_TIME_INTERVAL:
                    if((sensorPtr->ntfTimer == 0u) && (sensorPtr->sensorNewDataReady == true))
                    {
                        isCondTrue[i] = true;
                    }
                    break;
                    
                case CY_BLE_ESS_TRIG_WHEN_CHANGED:
                    if(sensorPtr->value != sensorPtr->prevValue)
                    {
                        isCondTrue[i] = true;
                    }
                    break;
                    
                case CY_BLE_ESS_TRIG_WHILE_LESS_THAN:
                    if(sensorPtr->value < sensorPtr->cmpValue[i])
                    {
                        isCondTrue[i] = true;
                    }
                    break;
                    
                case CY_BLE_ESS_TRIG_WHILE_LESS_OR_EQUAL:
                    if(sensorPtr->value <= sensorPtr->cmpValue[i])
                    {
                        isCondTrue[i] = true;
                    }
                    break;
                    
                case CY_BLE_ESS_TRIG_WHILE_GREATER_THAN:
                    if(sensorPtr->value > sensorPtr->cmpValue[i])
                    {
                        isCondTrue[i] = true;
                    }
                    break;
                    
                case CY_BLE_ESS_TRIG_WHILE_GREATER_OR_EQUAL:
                    if(sensorPtr->value >= sensorPtr->cmpValue[i])
                    {
                        isCondTrue[i] = true;
                    }
                    break;
                    
                case CY_BLE_ESS_TRIG_WHILE_EQUAL_TO:
                    if(sensorPtr->value == sensorPtr->cmpValue[i])
                    {
                        isCondTrue[i] = true;
                    }
                    break;
                    
                case CY_BLE_ESS_TRIG_WHILE_EQUAL_NOT_TO:
                    if(sensorPtr->value != sensorPtr->cmpValue[i])
                    {
                        isCondTrue[i] = true;
                    }
                    break;
                
                default:
                    isCondInactive[i] = true;
                    break;    
            }
        }
    }
    
    /* All condition inactive or not present */
    if ( (isCondInactive[0] == true) && (isCondInactive[1] == true) && (isCondInactive[2] == true) )
    { 
        result = true;
    }
    else   /* Condition connected with boolean AND */
    {   if( sensorPtr->esConfig == CY_BLE_ESS_CONF_BOOLEAN_AND )
        {   
           if( ((isCondInactive[0] == true) || (isCondTrue[0] == true)) && 
               ((isCondInactive[1] == true) || (isCondTrue[1] == true)) && 
               ((isCondInactive[2] == true) || (isCondTrue[2] == true)) ) 
            {
                result = true;
            }
        }
        else 
        {   /* Condition connected with boolean OR */
            if(  (isCondTrue[0] == true) || (isCondTrue[1] == true) || (isCondTrue[2] == true) ) 
            {
                result = true;
            }
        }
    }

    return(result);
}

/*******************************************************************************
* Function Name: EssHandleIndication
********************************************************************************
*
* Summary:
*  Sends indication about a descriptor change to the Client.
*
* Parameters:  
*  connHandle: The connection handle
*
*******************************************************************************/
void EssHandleIndication(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    ess_descr_val_change_value_t essValChanged;
    uint16_t cccd;

    apiResult = Cy_BLE_ESSS_GetCharacteristicDescriptor(connHandle, CY_BLE_ESS_DESCRIPTOR_VALUE_CHANGED, 0u, CY_BLE_ESS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
        
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_ESSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
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
            /* Pack data to BLE compatible format ... */
            Cy_BLE_Set16ByPtr(&essValChanged.flags[0u], indicationValue);
            Cy_BLE_Set16ByPtr(&essValChanged.uuid[0u], CY_BLE_UUID_CHAR_TRUE_WIND_SPEED);
            
            /* ... and send it */
            apiResult = Cy_BLE_ESSS_SendIndication(connHandle, CY_BLE_ESS_DESCRIPTOR_VALUE_CHANGED, 0u, SIZE_4_BYTES,
                                                 (uint8_t *)&essValChanged);

            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Send indication is failed: %d \r\n", apiResult);
            }
            else
            {   DBG_PRINTF("Indication for ES Configuration Descriptor was sent successfully.\r\n");
                DBG_PRINTF("Indicated value is: %02X %02X %02X %02X\r\n",
                    essValChanged.uuid[1], essValChanged.uuid[0], essValChanged.flags[1], essValChanged.flags[0]);
            }
        }
    }
}


/*******************************************************************************
* Function Name: GetUint24
********************************************************************************
*
* Summary:
*  Extracts three bytes value from the buffer send over the BLE radio and stores 
*  in uint32_t variable.
*
* Parameters:  
*  u24Ptr[]: Three byte buffer that holds uint24 value. The buffer holds the 
*            lowest uint24 byte in the first element.
*  u32:     Output parameter to store uint24 value.
*
*******************************************************************************/
void GetUint24(uint32_t *u32, uint8_t u24Ptr[])
{
    *u32 = ((uint32) ((uint32) u24Ptr[0u]) | ((uint32) (((uint32) u24Ptr[1u]) << 8u)) |
                                               ((uint32) (((uint32) u24Ptr[2u]) << 16u)));
}

/*******************************************************************************
* Function Name: EssChkNtfAndSendData
********************************************************************************
*
* Summary:
*   Check if there are notifications for parameter value and send them.
*
* Parameters:  
*   connHandle: The connection handle
*   sensorPtr:  A pointer to the sensor characteristic structure.
*
*******************************************************************************/
void EssChkNtfAndSendData(cy_stc_ble_conn_handle_t connHandle, cy_stc_ble_ess_characteristic_data_t *sensorPtr)
{
    if(EssHandleNtfConditions(sensorPtr) == true)
    {
        EssHandleNotificaion(connHandle, sensorPtr);
        Cy_BLE_ProcessEvents();
    }
}

/*******************************************************************************
* Function Name: EssCharIndexToText
********************************************************************************
*
* Summary:
*   Convert a ESS Characteristic Index to the character string.
*
* Parameters:  
*   EssChrIndex : ESS Characteristic Index
*
* Return: 
*   A pointer to the character string contained a name of ESS Characteristic.
*
*******************************************************************************/
char *EssCharIndexToText(cy_en_ble_ess_char_index_t EssChrIndex)
{   
    char *ptrToMsg;
    
    switch (EssChrIndex)
    {   
        case CY_BLE_ESS_DESCRIPTOR_VALUE_CHANGED :   
            ptrToMsg = (char *)"Descriptor Value Changed"; 
            break;
            
        case CY_BLE_ESS_APPARENT_WIND_DIR :          
            ptrToMsg = (char *)"Apparent Wind Direction";
            break;
            
        case CY_BLE_ESS_APPARENT_WIND_SPEED :        
            ptrToMsg = (char *)"Apparent Wind Speed";
            break;
            
        case CY_BLE_ESS_DEW_POINT :                  
            ptrToMsg = (char *)"Dew Point";
            break;
            
        case CY_BLE_ESS_ELEVATION :                  
            ptrToMsg = (char *)"Elevation";     
            break;
            
        case CY_BLE_ESS_GUST_FACTOR :                
            ptrToMsg = (char *)"Gust Factor";     
            break;
            
        case CY_BLE_ESS_HEAT_INDEX :                 
            ptrToMsg = (char *)"Heat Index";                  
            break;
            
        case CY_BLE_ESS_HUMIDITY :                   
            ptrToMsg = (char *)"Humidity";                   
            break;
            
        case CY_BLE_ESS_IRRADIANCE :                 
            ptrToMsg = (char *)"Irradiance";                     
            break;
            
        case CY_BLE_ESS_POLLEN_CONCENTRATION :       
            ptrToMsg = (char *)"Pollen Concentration";
            break;
            
        case CY_BLE_ESS_RAINFALL :                  
            ptrToMsg = (char *)"Rainfall";
            break;
            
        case CY_BLE_ESS_PRESSURE :                   
            ptrToMsg = (char *)"Pressure";
            break;
            
        case CY_BLE_ESS_TEMPERATURE :                
            ptrToMsg = (char *)"Temperature";
            break;
            
        case CY_BLE_ESS_TRUE_WIND_DIR :              
            ptrToMsg = (char *)"True Wind Direction";
            break;
            
        case CY_BLE_ESS_TRUE_WIND_SPEED :           
            ptrToMsg = (char *)"True Wind Speed";
            break;
            
        case CY_BLE_ESS_UV_INDEX :                   
            ptrToMsg = (char *)"UV Index";
            break;
            
        case CY_BLE_ESS_WIND_CHILL :                 
            ptrToMsg = (char *)"Wind Chill";
            break;
            
        case CY_BLE_ESS_BAROMETRIC_PRESSURE_TREND :  
            ptrToMsg = (char *)"Barometric Pressure";
            break;
            
        case CY_BLE_ESS_MAGNETIC_DECLINATION :       
            ptrToMsg = (char *)"Magnetic Declination";
            break;
            
        case CY_BLE_ESS_MAGNETIC_FLUX_DENSITY_2D :   
            ptrToMsg = (char *)"Magnetic Flux Density 2D";
            break;
            
        case CY_BLE_ESS_MAGNETIC_FLUX_DENSITY_3D :   
            ptrToMsg = (char *)"Magnetic Flux Density 3D";
            break;
            
        case CY_BLE_ESS_CHAR_COUNT :                 
            ptrToMsg = (char *)"Total count of ESS";

        default :
            DBG_PRINTF("EssChrIndex is failed: %d \r\n", EssChrIndex);
            ptrToMsg = (char *)"NONE";
            break;
    }
    
    return ptrToMsg;
}

/*******************************************************************************
* Function Name: EssSetIndicationValue
********************************************************************************
*
* Summary:
*   Convert a ESS Characteristic Index to the character string.
*
* Parameters:  
*   EssChrIndex : ESS Characteristic Index
*
* Return: 
*   A pointer to the character string contained a name of ESS Characteristic.
*
*******************************************************************************/
void EssSetIndicationValue(uint16_t localIndicationValue)
{
    indicationValue = localIndicationValue;
}


/* [] END OF FILE */
