/*******************************************************************************
* File Name: plxs.c
*
* Version 1.0
*
* Description:
*  This file contains the code for the PLXS service.
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


#include "common.h"
#include "plxs.h"
#include "math.h"

    
/* PLXS Characteristics data structures */
static cy_stc_ble_plxs_scmt_char_t plxsSpotCheckMeasChar;
static cy_stc_ble_plxs_ctmt_char_t plxsContMeasChar;
static cy_stc_ble_plxs_feat_char_t plxsFeaturesChar;
static cy_stc_ble_plxs_racp_char_t plxsRacpChar;

/* RACP records storage */
static volatile cy_stc_ble_plxs_racp_t plxsRacpOpr = {  .storage.count = 0u,  .state = APP_RACP_STATE_IDLE };

/* RACP records storage in flash */
CY_SECTION(".cy_em_eeprom") CY_ALIGN(CY_FLASH_SIZEOF_ROW) 
    const cy_stc_ble_plxs_racp_storage_t  racpFlashStorage = { .count = 0u };
    
static volatile bool extAbortFlag;
static volatile bool indCnfFlag;
static volatile uint32_t startSpotCheckTimer = 0u;    
static volatile bool startSpotCheckFlag = false;    

    
/*******************************************************************************
* Function Name: PlxsCallBack
********************************************************************************
*
* Summary:
*  PLX Service callback
*
*  event      - The event code.
*  eventParam - The pointer to event data.
*
*******************************************************************************/    
void PlxsCallBack(uint32_t event, void* eventParam)
{
    switch(event)
    {
        /** PLXS Server - Indication for Pulse Oximeter Service Characteristic was enabled. The parameter of this event
        is a structure of cy_stc_ble_plxs_char_value_t type. */
        case CY_BLE_EVT_PLXSS_INDICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_PLXSS_INDICATION_ENABLED: \r\n");
            break;
            
        /** PLXS Server - Indication for Pulse Oximeter Service Characteristic was disabled. The parameter of this event 
            is a structure of the cy_stc_ble_gls_char_value_t type.*/
        case CY_BLE_EVT_PLXSS_INDICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_PLXSS_INDICATION_DISABLED: \r\n");
            break;
            
        /** PLXS Server - Pulse Oximeter Service Characteristic Indication was confirmed. The parameter of this event
            is a structure of the cy_stc_ble_plxs_char_value_t type. */
        case CY_BLE_EVT_PLXSS_INDICATION_CONFIRMED:     
            indCnfFlag = true;
            break;
            
        /** PLXS Server - Notifications for Pulse Oximeter Service Characteristic was enabled. The parameter of this 
            event is a structure of the cy_stc_ble_plxs_char_value_t type. */
        case CY_BLE_EVT_PLXSS_NOTIFICATION_ENABLED:
            DBG_PRINTF("CY_BLE_EVT_PLXSS_NOTIFICATION_ENABLED: \r\n");
            break;
            
        /** PLXS Server - Notifications for Pulse Oximeter Service Characteristic were disabled. The parameter of this 
            event is a structure of the cy_stc_ble_plxs_char_value_t type.  */
        case CY_BLE_EVT_PLXSS_NOTIFICATION_DISABLED:
            DBG_PRINTF("CY_BLE_EVT_PLXSS_NOTIFICATION_DISABLED: \r\n");
            break;
            
        /** PLXS Server - Write Request for Pulse Oximeter Service  was received. The parameter of this event is a 
            structure of cy_stc_ble_plxs_char_value_t type.  */    
        case CY_BLE_EVT_PLXSS_WRITE_CHAR:
            DBG_PRINTF("CY_BLE_EVT_PLXSS_WRITE_CHAR: \r\n");
            
            if(((cy_stc_ble_plxs_char_value_t *)eventParam)->charIndex == CY_BLE_PLXS_RACP)
            {
                
                /* Save received RACP packets in plxsRacpChar structure */
                PlxsUnPackData(CY_BLE_PLXS_RACP,((cy_stc_ble_plxs_char_value_t *)eventParam)->value->len,
                                                ((cy_stc_ble_plxs_char_value_t *)eventParam)->value->val, &plxsRacpChar);
                
                /* Indicate that we have RACP event for processing */
                plxsRacpOpr.state = APP_RACP_STATE_RECEIVED_OPCODE; 
                
                if(plxsRacpChar.opCode == CY_BLE_PLXS_RACP_OPC_ABORT_OPN)
                {
                    extAbortFlag = true;
                    DBG_PRINTF("ABORT operation: extAbortFlag = true \r\n");
                }
            } 
            break;
            
        /** PLXS Client - Pulse Oximeter Service Characteristic Indication was received. The parameter of this event
            is a structure of the cy_stc_ble_plxs_char_value_t type. */
        case CY_BLE_EVT_PLXSC_INDICATION:
            DBG_PRINTF("CY_BLE_EVT_PLXSC_INDICATION: \r\n");
            
            if(((cy_stc_ble_plxs_char_value_t *)eventParam)->charIndex == CY_BLE_PLXS_SCMT)
            {
                /* Save received SCMT packet in plxsSpotCheckMeasChar structure */
                PlxsUnPackData(CY_BLE_PLXS_SCMT, ((cy_stc_ble_plxs_char_value_t *)eventParam)->value->len,
                                                    ((cy_stc_ble_plxs_char_value_t *)eventParam)->value->val, 
                                                    &plxsSpotCheckMeasChar);
            }
            
            if(((cy_stc_ble_plxs_char_value_t *)eventParam)->charIndex == CY_BLE_PLXS_RACP)
            {
                /* Save received RACP packets in plxsRacpChar structure */
                PlxsUnPackData(CY_BLE_PLXS_RACP, ((cy_stc_ble_plxs_char_value_t *)eventParam)->value->len,
                               ((cy_stc_ble_plxs_char_value_t *)eventParam)->value->val, &plxsRacpChar);                
            }
            break;
            
        /** PLXS Client - Pulse Oximeter Service Characteristic Notification was received. The parameter of this event
            is a structure of the cy_stc_ble_plxs_char_value_t type. */
        case CY_BLE_EVT_PLXSC_NOTIFICATION:
            DBG_PRINTF("CY_BLE_EVT_PLXSC_NOTIFICATION: \r\n");
           
            if(((cy_stc_ble_plxs_char_value_t *)eventParam)->charIndex == CY_BLE_PLXS_CTMT)
            {
                /* Save received CTMT packets in plxsContMeasChar structure */
                PlxsUnPackData(CY_BLE_PLXS_CTMT, ((cy_stc_ble_plxs_char_value_t *)eventParam)->value->len,
                               ((cy_stc_ble_plxs_char_value_t *)eventParam)->value->val, &plxsContMeasChar);
            }
            break;
            
        /** PLXS Client - Read Response for Read Request of Pulse Oximeter Service Characteristic value. The parameter 
            of this event is a structure of the cy_stc_ble_plxs_char_value_t type. */
        case CY_BLE_EVT_PLXSC_READ_CHAR_RESPONSE:
            DBG_PRINTF("CY_BLE_EVT_PLXSC_READ_CHAR_RESPONSE: \r\n");
            if(((cy_stc_ble_plxs_char_value_t *)eventParam)->charIndex == CY_BLE_PLXS_FEAT)
            {
                /* Save received FEAT packets in plxsFeaturesChar structure */
                PlxsUnPackData(CY_BLE_PLXS_FEAT, ((cy_stc_ble_plxs_char_value_t *)eventParam)->value->len,
                               ((cy_stc_ble_plxs_char_value_t *)eventParam)->value->val, &plxsFeaturesChar);        
            }
            break;
            
        /** PLXS Client - Write Response for Write Request of Pulse Oximeter Service Characteristic value. The 
            parameter of this event is a structure of the cy_stc_ble_plxs_char_value_t type. */
        case CY_BLE_EVT_PLXSC_WRITE_CHAR_RESPONSE:
            DBG_PRINTF("CY_BLE_EVT_PLXSC_WRITE_CHAR_RESPONSE: \r\n");
            break;
            
        /** PLXS Client - Read Response for Read Request of Pulse Oximeter Service Characteristic Descriptor Read 
            request. The parameter of this event is a structure of the cy_stc_ble_plxs_descr_value_t type.  */
        case CY_BLE_EVT_PLXSC_READ_DESCR_RESPONSE:
            DBG_PRINTF("CY_BLE_EVT_PLXSC_READ_DESCR_RESPONSE: \r\n");
            break;
            
        /** PLXS Client - Write Response for Write Request of Pulse Oximeter Service Characteristic Configuration
            Descriptor value. The parameter of this event is a structure of the cy_stc_ble_plxs_descr_value_t type.  */
        case CY_BLE_EVT_PLXSC_WRITE_DESCR_RESPONSE:
            DBG_PRINTF("CY_BLE_EVT_PLXSC_WRITE_DESCR_RESPONSE: \r\n");
            break;
            
        case CY_BLE_EVT_PLXSC_TIMEOUT:
            DBG_PRINTF("CY_BLE_EVT_PLXSC_TIMEOUT: %x, %x \r\n", 
                                                ((cy_stc_ble_plxs_char_value_t *)eventParam)->connHandle.attId, 
                                                ((cy_stc_ble_plxs_char_value_t *)eventParam)->connHandle.bdHandle);
            break;
            
        default:
            DBG_PRINTF("Unknown PLXS event: %lx \r\n", event);
            break;
    }
}

/*******************************************************************************
* Function Name: PlxsInit
********************************************************************************
*
* Summary:
*  Initializes the PLX application global variables.
*
*******************************************************************************/
void PlxsInit(void)
{
    uint8_t pduData[PLXS_MAX_PDU_SIZE];
    uint8_t charSize; 
    
    /* Register Plxs callback */
    Cy_BLE_PLXS_RegisterAttrCallback(PlxsCallBack);
    
    /* Sync configured in Customizer characteristics with global structures plxsSpotCheckMeasChar, 
       plxsContMeasChar, etc */
    charSize = CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_MAX_LEN(cy_ble_plxsConfig.plxss->charInfo[CY_BLE_PLXS_SCMT].charHandle);
    Cy_BLE_PLXSS_GetCharacteristicValue(CY_BLE_PLXS_SCMT, charSize, pduData);
    PlxsUnPackData(CY_BLE_PLXS_SCMT, charSize, pduData, &plxsSpotCheckMeasChar);
    
    charSize = CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_MAX_LEN(cy_ble_plxsConfig.plxss->charInfo[CY_BLE_PLXS_CTMT].charHandle);
    Cy_BLE_PLXSS_GetCharacteristicValue(CY_BLE_PLXS_CTMT, charSize, pduData);
    PlxsUnPackData(CY_BLE_PLXS_CTMT, charSize, pduData, &plxsContMeasChar);
    
    charSize = CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_MAX_LEN(cy_ble_plxsConfig.plxss->charInfo[CY_BLE_PLXS_FEAT].charHandle);
    Cy_BLE_PLXSS_GetCharacteristicValue(CY_BLE_PLXS_FEAT, charSize, pduData);
    PlxsUnPackData(CY_BLE_PLXS_FEAT, charSize, pduData, &plxsFeaturesChar);

    charSize = CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_MAX_LEN(cy_ble_plxsConfig.plxss->charInfo[CY_BLE_PLXS_RACP].charHandle);
    Cy_BLE_PLXSS_GetCharacteristicValue(CY_BLE_PLXS_RACP, PLXS_MAX_PDU_SIZE, pduData);
    PlxsUnPackData(CY_BLE_PLXS_RACP, charSize, pduData, &plxsRacpChar);
        
    /* Restore plxsRacpOpr.storage from flash (racpFlashStorage) */
    (void)memcpy((uint8_t*)&plxsRacpOpr.storage, &racpFlashStorage, sizeof(plxsRacpOpr.storage));
   
    /* Print stored Racp records */
    PlxsRacpStoragePrintRecords();
      
}

/***************************************
*        Functions
***************************************/

/*******************************************************************************
* Function Name: PlxsRacpStoragePushRecord
********************************************************************************
*
* Summary:
*   Push record to RACP storage.
*   If the maximum storage capacity is reached, this procedure overwrites the oldest 
*   stored measurements first when acquiring new measurements.
* 
* Parameters:
*   *data  - pointer to PLX Spot-check Measurement characteristic data
*
*******************************************************************************/
void PlxsRacpStoragePushRecord(cy_stc_ble_plxs_scmt_char_t *data)
{    
    /* Store PLX Spot-check Measurement characteristic data */
    if(plxsRacpOpr.storage.count == 0u)
    {
        plxsRacpOpr.storage.head = 0u;
        plxsRacpOpr.storage.tail = 0u;
        plxsRacpOpr.storage.records[plxsRacpOpr.storage.head] = *data;      
    }
    else 
    {
        /* Modify Head pointer */
        plxsRacpOpr.storage.head = (plxsRacpOpr.storage.head + 1u) < PLXS_RACP_BD_SIZE ? 
                                   (plxsRacpOpr.storage.head + 1u) : 0u;
        
        /* Store data */
        plxsRacpOpr.storage.records[plxsRacpOpr.storage.head] = *data;  
    }
    
    /* Modify tail pointer */
    if((plxsRacpOpr.storage.count != 0u) && (plxsRacpOpr.storage.head ==  plxsRacpOpr.storage.tail))
    {
         plxsRacpOpr.storage.tail = (plxsRacpOpr.storage.tail + 1u) < PLXS_RACP_BD_SIZE ? 
                                    (plxsRacpOpr.storage.tail + 1u) : 0u;
    }      
    /* Modify Count  */
    plxsRacpOpr.storage.count = (plxsRacpOpr.storage.count + 1u) < PLXS_RACP_BD_SIZE ? 
                                (plxsRacpOpr.storage.count + 1u) : PLXS_RACP_BD_SIZE;
    
    /* Store plxsRacpOpr.storage to flash (racpFlashStorage) */
    cy_stc_ble_app_flash_param_t appFlashParam =
    {
        .buffLen      = sizeof(plxsRacpOpr.storage),
        .destAddr     = (uint8_t *) &racpFlashStorage,
        .srcBuff      = (uint8_t *) &plxsRacpOpr.storage
    };
    
    Cy_BLE_StoreAppData(&appFlashParam);
    
    DBG_PRINTF("INFO: The PLX Spot-check Measurement record was added to RACP storage \r\n");      
}


/*******************************************************************************
* Function Name: PlxsRacpStoragePopRecord
********************************************************************************
*
* Summary:
*    Pop the stored record from RACP storage. This procedure returns (via input parameter retData) 
*    oldest data first followed by the next oldest data (in first-in, first-out order) 
* Parameters:
*   *retData  - pointer to PLX Spot-check Measurement characteristic data
*
* Return:
*    0u - storage is empty, 
*    1u - storage has stored records
*
*******************************************************************************/
uint32_t PlxsRacpStoragePopRecord(cy_stc_ble_plxs_scmt_char_t *retData)
{
    uint32_t retVal = 0u;
        
    if(plxsRacpOpr.storage.count != 0u)
    {   
        /* Return Data */
        *retData = plxsRacpOpr.storage.records[plxsRacpOpr.storage.tail];
    
        /* Modify tail pointer */
        plxsRacpOpr.storage.tail = (plxsRacpOpr.storage.tail + 1u) < PLXS_RACP_BD_SIZE ? 
                                                                    (plxsRacpOpr.storage.tail + 1u) : 0u;
        /* Modify Count */
        plxsRacpOpr.storage.count--;
        retVal = 1u;
    }
    
    /* Store plxsRacpOpr.storage to flash (racpFlashStorage) */
    cy_stc_ble_app_flash_param_t appFlashParam =
    {
        .buffLen    = sizeof(plxsRacpOpr.storage),
        .destAddr   = (uint8_t *) &racpFlashStorage,
        .srcBuff    = (uint8_t *) &plxsRacpOpr.storage
    };
    Cy_BLE_StoreAppData(&appFlashParam);
    
    return (retVal);
}

/*******************************************************************************
* Function Name: PlxsRacpStorageDeleteAll
********************************************************************************
*
* Summary:
*    Delete all stored records
*
*******************************************************************************/
void PlxsRacpStorageDeleteAll(void)
{
    /* Delete all stored records by reset record counter */
    plxsRacpOpr.storage.count = 0u;
     
    /* Store plxsRacpOpr.storage.count to flash (racpFlashStorage) */
    cy_stc_ble_app_flash_param_t appFlashParam =
    {
        .buffLen    = sizeof(plxsRacpOpr.storage.count),
        .destAddr   = (uint8_t *) &racpFlashStorage.count,
        .srcBuff    = (uint8_t *) &plxsRacpOpr.storage.count
    };
    Cy_BLE_StoreAppData(&appFlashParam);
}

/*******************************************************************************
* Function Name: PlxsRacpStoragePrintRecords
********************************************************************************
*
* Summary:
*   Prints stored RACP records (from plxsRacpOpr.storage).
*
*******************************************************************************/
void PlxsRacpStoragePrintRecords()
{    
    uint32_t i;
    uint32_t idx;  

    if(plxsRacpOpr.storage.count)
    {
        DBG_PRINTF("\r\nINFO: RACP storage:  \r\n");     
        for(i = 0u ; i < plxsRacpOpr.storage.count; i++)
        { 
            idx = (plxsRacpOpr.storage.tail + i) < PLXS_RACP_BD_SIZE ? (plxsRacpOpr.storage.tail + i) : 
                                                    (plxsRacpOpr.storage.tail + i) - PLXS_RACP_BD_SIZE ;
                                                                
            DBG_PRINTF("RACP storage record(s): %2lu. [Date: %.2d-%.2d-%.2d  Time: %.2d:%.2d:%.2d ]" \
                       "spO2: %.2f PR: %.2f PI: %.2f \r\n", i + 1u,
                plxsRacpOpr.storage.records[idx].timestamp.day, plxsRacpOpr.storage.records[idx].timestamp.month,
                plxsRacpOpr.storage.records[idx].timestamp.year,
                plxsRacpOpr.storage.records[idx].timestamp.hours,  plxsRacpOpr.storage.records[idx].timestamp.minutes,
                plxsRacpOpr.storage.records[idx].timestamp.seconds,
                plxsRacpOpr.storage.records[idx].spO2.m * pow(10.0, plxsRacpOpr.storage.records[idx].spO2.exp),
                plxsRacpOpr.storage.records[idx].Pr.m * pow(10.0, plxsRacpOpr.storage.records[idx].Pr.exp),
                plxsRacpOpr.storage.records[idx].pulseAmpIndex.m *
                    pow(10.0, plxsRacpOpr.storage.records[idx].pulseAmpIndex.exp));
        }
    }
    else
    {
        DBG_PRINTF("\r\nINFO: RACP storage is empty. \r\n");     
    } 
    DBG_PRINTF("\r\n");     
}


/*******************************************************************************
* Function Name: PlxsRacpProcess
********************************************************************************
*
* Summary:
*  Processes the Record Access Control Point (RACP) requests. 
*  RACP OpCode: "Report stored records", "Report number of stored records"
*               "Delete stored records", " Delete stored records" and 
*               "Abort operation" operations.
*
*******************************************************************************/
void PlxsRacpProcess(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    uint8_t pduData[PLXS_MAX_PDU_SIZE];   
    uint8_t pduSize;
    
    if(plxsRacpOpr.state == APP_RACP_STATE_RECEIVED_OPCODE)
    {       
        
        DBG_PRINTF("INFO: PlxsRacpProcess: %d %d \r\n", plxsRacpChar.opCode, plxsRacpChar.operator);
        
        /* Check if operator is supported/invalid */
        if(((plxsRacpChar.opCode == CY_BLE_PLXS_RACP_OPC_ABORT_OPN) && 
            (plxsRacpChar.operator != CY_BLE_PLXS_RACP_OPR_NULL)) ||
           ((plxsRacpChar.opCode != CY_BLE_PLXS_RACP_OPC_ABORT_OPN) && 
            (plxsRacpChar.operator != CY_BLE_PLXS_RACP_OPR_ALL))) 
        {
            plxsRacpOpr.state = (plxsRacpChar.operator == CY_BLE_PLXS_RACP_OPR_NULL) ? APP_RACP_STATE_ERROR_INV_OPR :
                                                                                       APP_RACP_STATE_ERROR_UNSPRT_OPR;
        }
        
        /* PLXS does not support any operands, so generate error if it comes */
        if(plxsRacpChar.operandIsPresent == true) 
        {
            plxsRacpOpr.state = APP_RACP_STATE_ERROR_UNSPRT_OPD;
        }
        
        /* Check if opCode is supported */
        if((plxsRacpChar.opCode < CY_BLE_PLXS_RACP_OPC_REPORT_REC) || 
           (plxsRacpChar.opCode > CY_BLE_PLXS_RACP_OPC_RSP_CODE))
        {
            plxsRacpOpr.state = APP_RACP_STATE_ERROR_UNSPRT_OPC;
        }
        
        /* --------------------------- Handle RACP Op Codes ---------------------------------------------------------*/
        if(plxsRacpOpr.state == APP_RACP_STATE_RECEIVED_OPCODE)
        {
            switch(plxsRacpChar.opCode)
            {
                /* Report stored records (Operator: Value from Operator Table) */  
                case CY_BLE_PLXS_RACP_OPC_REPORT_REC:     
                    
                    /* Check if we have stored any records */
                    if(plxsRacpOpr.storage.count == 0u)
                    {
                        plxsRacpChar.opCode                = CY_BLE_PLXS_RACP_OPC_RSP_CODE;
                        plxsRacpChar.operand.rsp.reqOpCode = CY_BLE_PLXS_RACP_OPC_REPORT_REC;
                        plxsRacpChar.operand.rsp.rspCode   = CY_BLE_PLXS_RACP_RSP_NO_REC;
                        
                        DBG_PRINTF("INFO: RACP_OPC_REPORT_REC: storage is empty \r\n");
                    }
                    else
                    { 
                        uint32_t i;
                        /* Send all stored records */
                        extAbortFlag = false;              /* reset extAbortFlag */
                        
                        DBG_PRINTF("INFO: RACP_OPC_REPORT_REC: read %ld records from storage: \r\n",
                                    plxsRacpOpr.storage.count);
                        
                        while((plxsRacpOpr.storage.count) && (extAbortFlag == false))
                        {            
                            /* Read stored PLX Spot-check measurement characteristic from RACP storage */
                            cy_stc_ble_plxs_scmt_char_t spotCheckRecordFromRacpStorage;
                            
                            /* Get oldest record from storage  */                  
                            PlxsRacpStoragePopRecord(&spotCheckRecordFromRacpStorage);  
                            
                            DBG_PRINTF("Stored data: [ Date: %ld-%ld-%ld  Time: %ld:%ld:%ld ]" \
                                       " spO2: %.2f PR: %.2f PI: %.2f \r\n",
                             (uint32_t)spotCheckRecordFromRacpStorage.timestamp.day,
                             (uint32_t)spotCheckRecordFromRacpStorage.timestamp.month,
                             (uint32_t)spotCheckRecordFromRacpStorage.timestamp.year,
                             (uint32_t)spotCheckRecordFromRacpStorage.timestamp.hours,
                             (uint32_t)spotCheckRecordFromRacpStorage.timestamp.minutes,
                             (uint32_t)spotCheckRecordFromRacpStorage.timestamp.seconds,
                             spotCheckRecordFromRacpStorage.spO2.m * pow(10.0, spotCheckRecordFromRacpStorage.spO2.exp),
                             spotCheckRecordFromRacpStorage.Pr.m * pow(10.0, spotCheckRecordFromRacpStorage.Pr.exp),
                             spotCheckRecordFromRacpStorage.pulseAmpIndex.m * 
                                 pow(10.0, spotCheckRecordFromRacpStorage.pulseAmpIndex.exp));
                            
                            /* Send Indication of PLX Spot-check measurement characteristic */
                            pduSize = PlxsPackData(CY_BLE_PLXS_SCMT, &spotCheckRecordFromRacpStorage, pduData);
                            
                            /* Print pduData */
                            DBG_PRINTF("PDU data: ");
                            for(i = 0u; i < pduSize; i++)
                            {
                                DBG_PRINTF("0x%2.2x ", pduData[i]);
                            }
                            DBG_PRINTF("\r\n");
                                
                            apiResult = Cy_BLE_PLXSS_SendIndication(connHandle, CY_BLE_PLXS_SCMT,  pduSize, pduData);  
                            if(apiResult != CY_BLE_SUCCESS)
                            {
                                DBG_PRINTF("Cy_BLE_PLXSS_SendIndication() failed. API Error: 0x%x \r\n", apiResult);
                            }
                            
                            indCnfFlag = false;
     
                            /* Wait while Indication finish send */
                            do
                            {
                                Cy_BLE_ProcessEvents();
                            }
                            while(indCnfFlag == false);
                        }
                        
                        /* Response packet */
                        plxsRacpChar.opCode                = CY_BLE_PLXS_RACP_OPC_RSP_CODE;
                        plxsRacpChar.operand.rsp.rspCode   = CY_BLE_PLXS_RACP_RSP_SUCCESS;
                        plxsRacpChar.operand.rsp.reqOpCode = (extAbortFlag == false) ? CY_BLE_PLXS_RACP_OPC_REPORT_REC :
                                                                                       CY_BLE_PLXS_RACP_OPC_ABORT_OPN;
                    }
                    break;
                    
                /* Report number of stored records (Operator: Value from Operator Table) */
                case CY_BLE_PLXS_RACP_OPC_REPORT_NUM_REC:  
                    /* Response packet */
                    plxsRacpChar.opCode = CY_BLE_PLXS_RACP_OPC_NUM_REC_RSP;
                    plxsRacpChar.operand.value = plxsRacpOpr.storage.count;
                    
                    DBG_PRINTF("INFO: RACP_OPC_REPORT_NUM_REC: stored data [%ld] \r\n", plxsRacpOpr.storage.count);
                    break;    
                  
                /* Delete stored records (Operator: Value from Operator Table) */
                case CY_BLE_PLXS_RACP_OPC_DELETE_REC:     
  
                    /* Delete all stored records by reset record counter */
                    plxsRacpOpr.storage.count = 0u;
                    
                    /* Response packet */
                    plxsRacpChar.opCode = CY_BLE_PLXS_RACP_OPC_RSP_CODE;
                    plxsRacpChar.operand.rsp.reqOpCode = CY_BLE_PLXS_RACP_OPC_DELETE_REC;
                    plxsRacpChar.operand.rsp.rspCode   = CY_BLE_PLXS_RACP_RSP_SUCCESS;
                    
                    DBG_PRINTF("INFO: RACP_OPC_DELETE_REC: remove all stored data \r\n");
                    break;
                    
                /* Abort operation (Operator: Null 'value of 0x00 from Operator Table') */
                case CY_BLE_PLXS_RACP_OPC_ABORT_OPN:       

                    /* Response packet */
                    plxsRacpChar.opCode = CY_BLE_PLXS_RACP_OPC_RSP_CODE;
                    plxsRacpChar.operand.rsp.reqOpCode = CY_BLE_PLXS_RACP_OPC_ABORT_OPN;
                    plxsRacpChar.operand.rsp.rspCode   = CY_BLE_PLXS_RACP_RSP_SUCCESS;
                    
                    DBG_PRINTF("INFO: RACP_OPC_ABORT_OPN \r\n");
                    break;

                default:
                    /* Receive not supported Op Code */
                    plxsRacpOpr.state = APP_RACP_STATE_ERROR_UNSPRT_OPC;
                    break;   
            }
        }
        /* --------------------------- Handle RACP Specific Errors --------------------------------------------------*/
        /* Op Code Not Supported */
        if(plxsRacpOpr.state == APP_RACP_STATE_ERROR_UNSPRT_OPC)
        {
            plxsRacpChar.operand.rsp.reqOpCode = plxsRacpChar.opCode;
            plxsRacpChar.operand.rsp.rspCode   = CY_BLE_PLXS_RACP_RSP_UNSPRT_OPC;
            plxsRacpChar.opCode                = CY_BLE_PLXS_RACP_OPC_RSP_CODE;
        }
        /* Operator Not Supported */
        else if(plxsRacpOpr.state == APP_RACP_STATE_ERROR_UNSPRT_OPR)
        {
            plxsRacpChar.operand.rsp.reqOpCode = plxsRacpChar.opCode;
            plxsRacpChar.operand.rsp.rspCode   = CY_BLE_PLXS_RACP_RSP_UNSPRT_OPR;
            plxsRacpChar.opCode                = CY_BLE_PLXS_RACP_OPC_RSP_CODE;
        }
        /* Invalid Operator  */
        else if(plxsRacpOpr.state == APP_RACP_STATE_ERROR_INV_OPR)
        {
            plxsRacpChar.operand.rsp.reqOpCode = plxsRacpChar.opCode;
            plxsRacpChar.operand.rsp.rspCode   = CY_BLE_PLXS_RACP_RSP_INV_OPR;
            plxsRacpChar.opCode                = CY_BLE_PLXS_RACP_OPC_RSP_CODE;
        }
        /* Operand Not Supported */
        else if(plxsRacpOpr.state == APP_RACP_STATE_ERROR_UNSPRT_OPD)
        {
            plxsRacpChar.operand.rsp.reqOpCode = plxsRacpChar.opCode;
            plxsRacpChar.operand.rsp.rspCode   = CY_BLE_PLXS_RACP_RSP_UNSPRT_OPD;
            plxsRacpChar.opCode                = CY_BLE_PLXS_RACP_OPC_RSP_CODE;
        }
        /* Invalid Operator  */
        else if(plxsRacpOpr.state == APP_RACP_STATE_ERROR_INV_OPD)
        {
            plxsRacpChar.operand.rsp.reqOpCode = plxsRacpChar.opCode;
            plxsRacpChar.operand.rsp.rspCode   = CY_BLE_PLXS_RACP_RSP_INV_OPD;
            plxsRacpChar.opCode                = CY_BLE_PLXS_RACP_OPC_RSP_CODE;
        }
        else
        {
            /* Final empty else is required by the coding standard */   
        }
        /* ----------------------------------------------------------------------------------------------------------*/
        
        /* Send RACP indication with response */
        DBG_PRINTF("INFO: plxsRacpOpr.state: %d \r\n", plxsRacpOpr.state);
        if(plxsRacpOpr.state != APP_RACP_STATE_IDLE)
        {
            plxsRacpChar.operator = CY_BLE_PLXS_RACP_OPR_NULL;               /* operator for response is always NULL */
            
            pduSize = PlxsPackData(CY_BLE_PLXS_RACP, &plxsRacpChar, pduData);
            apiResult = Cy_BLE_PLXSS_SendIndication(connHandle, CY_BLE_PLXS_RACP, pduSize, pduData);  
            
            plxsRacpOpr.state = APP_RACP_STATE_IDLE; 
        }
    }
}


/*******************************************************************************
* Function Name: PlxsPackData
********************************************************************************
*
* Summary:
*  Packs app PLX characteristics structure to PDU array before using in BLE API.
*
* Parameters:
*  charIdx   - index of PLX characteristic (defined in cy_en_ble_plxs_char_index_t enum)
*  *inData   - pointer on PLX characteristic structure
*  *outData  - pointer on PDU array
*
* Return:
*  uint8_t   - PDU size.
*
*******************************************************************************/
uint8_t PlxsPackData(cy_en_ble_plxs_char_index_t charIdx, void *inData, uint8_t *outData)
{
    uint32_t pduSize = 0u;
    uint32_t value;    
    
    /* Clear output buffer */
    (void)memset(outData, 0u, PLXS_MAX_PDU_SIZE);
    
    switch(charIdx)
    {
        case CY_BLE_PLXS_SCMT:  /* Pack PLX Spot-check Measurement characteristic  */
        {
            /* Declaration pointer to PLX Spot-check Measurement characteristic */
            cy_stc_ble_plxs_scmt_char_t *plxsSpotCheckMeasCharPtr = (cy_stc_ble_plxs_scmt_char_t *)inData;
    
            /* Pack "Flags" field (8 bits) */
            outData[pduSize] = plxsSpotCheckMeasCharPtr->flags.value;
            pduSize += SIZE_8BIT;
            
            /* Pack "SpO2PR-Spot-check - SpO2" field (16 bits) */
            value = (uint32)Cy_BLE_Get16ByPtr((uint8_t *)&plxsSpotCheckMeasCharPtr->spO2);
            memcpy(&outData[pduSize], &value, SIZE_16BIT);
            pduSize += SIZE_16BIT;
            
            /* Pack "SpO2PR-Spot-check - PR" field (16 bits) */
            value = (uint32)Cy_BLE_Get16ByPtr((uint8_t *)&plxsSpotCheckMeasCharPtr->Pr);
            memcpy(&outData[pduSize], &value, SIZE_16BIT);
            pduSize += SIZE_16BIT;
            
            /* Pack "Timestamp" field (7 bytes) if "Timestamp field is present" bit is present in "Flags" field */
            if(plxsSpotCheckMeasCharPtr->flags.bit.TMSF == SUPPORT)
            {
                cy_stc_ble_date_time_t tempTime = plxsSpotCheckMeasCharPtr->timestamp;
                tempTime.year = Cy_BLE_Get16ByPtr((uint8_t *)&tempTime.year);
                memcpy(&outData[pduSize], &tempTime, SIZE_TIMESTAMP);         
                pduSize += SIZE_TIMESTAMP;     
            }
            
            /* Pack "Measurement Status" field (16 bits)
               if "Measurement Status Field Present" bit is present in "Flags" field */
            if(plxsSpotCheckMeasCharPtr->flags.bit.MSF == SUPPORT)
            {
                value = (uint32)Cy_BLE_Get16ByPtr((uint8_t *)&plxsSpotCheckMeasCharPtr->measStatus.value);
                memcpy(&outData[pduSize], &value, SIZE_16BIT);
                pduSize += SIZE_16BIT;
            }
            
            /* Pack "Device and Sensor Status" field (24 bits)
               if "Device and Sensor Status Field Present" bit is present in "Flags" field */
            if(plxsSpotCheckMeasCharPtr->flags.bit.DSSF == SUPPORT)
            {
                value = (uint32)Cy_BLE_Get24ByPtr((uint8_t *)&plxsSpotCheckMeasCharPtr->dsStatus.value);
                memcpy(&outData[pduSize], &value, SIZE_24BIT);
                pduSize += SIZE_24BIT;
            }
            
            /* Pack "Pulse Amplitude Index" field (16 bits)
               if "Pulse Amplitude Index field is present" bit is present in "Flags" field */
            if(plxsSpotCheckMeasCharPtr->flags.bit.PAIF == SUPPORT)
            {
                value = (uint32)Cy_BLE_Get16ByPtr((uint8_t *)&plxsSpotCheckMeasCharPtr->pulseAmpIndex);
                memcpy(&outData[pduSize], &value, SIZE_16BIT);
                pduSize += SIZE_16BIT;
            }                 
            break;
        }
        
        case CY_BLE_PLXS_CTMT:  /* Pack PLX Continuous Measurement characteristic  */
        {
            /* Declaration pointer to PLX Continuous Measurement characteristic */
            cy_stc_ble_plxs_ctmt_char_t *plxsContMeasCharPtr = (cy_stc_ble_plxs_ctmt_char_t *)inData;
            
            /* Pack "Flags" field (8 bit) */
            outData[pduSize] = plxsContMeasCharPtr->flags.value;
            pduSize += SIZE_8BIT;
            
            /* Pack "SpO2PR-Normal - SpO2" field (16 bits) */
            value = (uint32)Cy_BLE_Get16ByPtr((uint8_t *)&plxsContMeasCharPtr->normalSpO2);
            memcpy(&outData[pduSize], &value, SIZE_16BIT);
            pduSize += SIZE_16BIT;
            
            /* Pack "SSpO2PR-Normal - PR" field (16 bits) */
            value = (uint32)Cy_BLE_Get16ByPtr((uint8_t *)&plxsContMeasCharPtr->normalPr);
            memcpy(&outData[pduSize], &value, SIZE_16BIT);
            pduSize += SIZE_16BIT;
            
            /* Pack "SpO2PR-Fast - SpO2" and "SpO2PR-Fast - PR" fields (16/16 bits) if "SpO2PR–Fast field" bit is
               present in "Flags" field */
            if(plxsContMeasCharPtr->flags.bit.FAST == SUPPORT)
            {
                value = (uint32)Cy_BLE_Get16ByPtr((uint8_t *)&plxsContMeasCharPtr->fastSpO2);
                memcpy(&outData[pduSize], &value, SIZE_16BIT);
                pduSize += SIZE_16BIT;
                
                value = (uint32)Cy_BLE_Get16ByPtr((uint8_t *)&plxsContMeasCharPtr->fastPr);
                memcpy(&outData[pduSize], &value, SIZE_16BIT);
                pduSize += SIZE_16BIT;
            }
            
            /* Pack "SpO2PR-Slow - SpO2" and "SpO2PR-Slow - PR" fields (16/16 bits) if "SpO2PR–Fast field" bit is
               present in "Flags" field */
            if(plxsContMeasCharPtr->flags.bit.SLOW == SUPPORT)
            {
                value = (uint32)Cy_BLE_Get16ByPtr((uint8_t *)&plxsContMeasCharPtr->slowSpO2);
                memcpy(&outData[pduSize], &value, SIZE_16BIT);
                pduSize += SIZE_16BIT;
                
                value = (uint32)Cy_BLE_Get16ByPtr((uint8_t *)&plxsContMeasCharPtr->slowPr);
                memcpy(&outData[pduSize], &value, SIZE_16BIT);
                pduSize += SIZE_16BIT;
            }
            
            /* Pack "Measurement Status" field (16 bits) if "Measurement Status Field Present" bit is
               present in "Flags" field */
            if(plxsContMeasCharPtr->flags.bit.MSF == SUPPORT)
            {
                value = (uint32)Cy_BLE_Get16ByPtr((uint8_t *)&plxsContMeasCharPtr->measStatus.value);
                memcpy(&outData[pduSize], &value, SIZE_16BIT);
                pduSize += SIZE_16BIT;
            }
            
            /* Pack "Device and Sensor Status" field (24 bits) if "Device and Sensor Status Field Present" bit is present 
               in "Flags" field */
            if(plxsContMeasCharPtr->flags.bit.DSSF == SUPPORT)
            {
                value = (uint32)Cy_BLE_Get24ByPtr((uint8_t *)&plxsContMeasCharPtr->dsStatus.value);
                memcpy(&outData[pduSize], &value, SIZE_24BIT);
                pduSize += SIZE_24BIT;
            }
            
            /* Pack "Pulse Amplitude Index" field (16 bits) if "Pulse Amplitude Index field is present" bit is
               present in "Flags" field */
            if(plxsContMeasCharPtr->flags.bit.PAIF == SUPPORT)
            {
                value = (uint32)Cy_BLE_Get16ByPtr((uint8_t *)&plxsContMeasCharPtr->pulseAmpIndex);
                memcpy(&outData[pduSize], &value, SIZE_16BIT);
                pduSize += SIZE_16BIT;
            }
            break;
        }
        
        case CY_BLE_PLXS_FEAT:  /* Pack PLX Features characteristic */
        {
            /* Declaration pointer to PLX Continuous Features characteristic */
            cy_stc_ble_plxs_feat_char_t *plxsFeaturesCharPtr = (cy_stc_ble_plxs_feat_char_t *)inData;
            
            /* Pack "Supported Features" field (16 bits) */
            value = (uint32)Cy_BLE_Get16ByPtr((uint8_t *)&plxsFeaturesCharPtr->supportedFeatures.value);
            memcpy(&outData[pduSize], &value, SIZE_16BIT);
            pduSize += SIZE_16BIT;
            
            /* Pack "Measurement Status Support" field (16 bits) if "Measurement Status support" bit is
               present in "Supported Features field" */
            if(plxsFeaturesCharPtr->supportedFeatures.bit.MEAS == SUPPORT)
            {
                value = (uint32)Cy_BLE_Get16ByPtr((uint8_t *)&plxsFeaturesCharPtr->measStatusSupport.value);
                memcpy(&outData[pduSize], &value, SIZE_16BIT);
                pduSize += SIZE_16BIT;
            }
            
            /* Pack "Device and Sensor Status Support" field (24 bits) if "Device and Sensor Status support" bit is 
               present in "Supported Features field" */
            if(plxsFeaturesCharPtr->supportedFeatures.bit.DSS == SUPPORT)
            {
                value = (uint32)Cy_BLE_Get24ByPtr((uint8_t *)&plxsFeaturesCharPtr->dsStatusSupport.value);
                memcpy(&outData[pduSize], &value, SIZE_24BIT);
                pduSize += SIZE_24BIT;
            }
            break;
        } 
        
        case CY_BLE_PLXS_RACP:  /* Pack PLX Record Access Control Point characteristic */
        {
            /* Declaration pointer to PLX Record Access Control Point characteristic */
            cy_stc_ble_plxs_racp_char_t *plxsRacpCharPtr = (cy_stc_ble_plxs_racp_char_t *)inData;
            
            outData[pduSize] = (uint8_t)plxsRacpCharPtr->opCode;
            pduSize += SIZE_8BIT;
            
            outData[pduSize] = (uint8_t)plxsRacpCharPtr->operator;
            pduSize += SIZE_8BIT;
            
            /*  Operand used only in opCode == CY_BLE_PLXS_RACP_OPC_NUM_REC_RSP or  CY_BLE_PLXS_RACP_OPC_RSP_CODE */
            if((plxsRacpCharPtr->opCode == CY_BLE_PLXS_RACP_OPC_NUM_REC_RSP) ||
               (plxsRacpCharPtr->opCode == CY_BLE_PLXS_RACP_OPC_RSP_CODE))
            {
                value = (uint32)Cy_BLE_Get16ByPtr((uint8_t *)&plxsRacpCharPtr->operand.value);
                memcpy(&outData[pduSize], &value, SIZE_16BIT);
                pduSize += SIZE_16BIT;
            }
            break;
        }
        
        default:
            break;  
    }
    
    return(pduSize);    
}


/*******************************************************************************
* Function Name: PlxsUnPackData
********************************************************************************
*
* Summary:
*   Unpacks received PDU array to application PLX characteristics structure.
*
* Parameters:
*   charIdx     - index of PLX characteristic(defined in cy_en_ble_plxs_char_index_t enum)
*   inDataSize  - size of input data
*   inData      - pointer on PDU array
*   outData     - pointer on PLX characteristics structure
*
* Return:
*  1u - correct input data, 0u - incorrect input data
*
*******************************************************************************/
uint32_t PlxsUnPackData(cy_en_ble_plxs_char_index_t charIdx, uint8_t inDataSize, uint8_t *inData, void *outData)
{
    uint32_t pduSize = 0u;
    uint32_t value;
    uint32_t calcDataSize;
    uint32_t retValue = 0u;     /* 1u - correct input data; 0u - incorrect input data; */
    
    /* Declaration pointers on characteristics */
    cy_stc_ble_plxs_scmt_char_t *plxsSpotCheckMeasCharPtr = (cy_stc_ble_plxs_scmt_char_t *)outData;
    cy_stc_ble_plxs_ctmt_char_t *plxsContMeasCharPtr      = (cy_stc_ble_plxs_ctmt_char_t *)outData;
    cy_stc_ble_plxs_feat_char_t *plxsFeaturesCharPtr      = (cy_stc_ble_plxs_feat_char_t *)outData;
    cy_stc_ble_plxs_racp_char_t *plxsRacpCharPtr          = (cy_stc_ble_plxs_racp_char_t *)outData;
    
    switch(charIdx)
    {
        case CY_BLE_PLXS_SCMT:  /* Unpack PLX Spot-check Measurement characteristic */
        {
            calcDataSize = SIZE_8BIT + SIZE_16BIT + SIZE_16BIT; /* Size of "Flags" + "SpO2" + "Pr" fields (mandatory)*/        
           
            /* Check size of mandatory part of packet and unpack */
            if(inDataSize >= calcDataSize)
            {
                /* Unpack "Flags" field (8 bits) */
                plxsSpotCheckMeasCharPtr->flags.value = inData[pduSize];
                pduSize += SIZE_8BIT;
                
                /* Unpack "SpO2PR-Spot-check - SpO2" field (16 bits) */
                value = (uint32)Cy_BLE_Get16ByPtr(&inData[pduSize]);
                memcpy(&plxsSpotCheckMeasCharPtr->spO2, &value, SIZE_16BIT);
                pduSize += SIZE_16BIT;
                
                /* Unpack "SpO2PR-Spot-check - PR" field (16 bits) */
                value = (uint32)Cy_BLE_Get16ByPtr(&inData[pduSize]);
                memcpy(&plxsSpotCheckMeasCharPtr->Pr, &value, SIZE_16BIT);
                pduSize += SIZE_16BIT;
                
                /* Check size of optional part of packet */
                if(plxsSpotCheckMeasCharPtr->flags.bit.TMSF == SUPPORT){ calcDataSize += SIZE_TIMESTAMP; }
                if(plxsSpotCheckMeasCharPtr->flags.bit.MSF  == SUPPORT){ calcDataSize += SIZE_16BIT;     }
                if(plxsSpotCheckMeasCharPtr->flags.bit.DSSF == SUPPORT){ calcDataSize += SIZE_24BIT;     }
                if(plxsSpotCheckMeasCharPtr->flags.bit.PAIF == SUPPORT){ calcDataSize += SIZE_16BIT;     }
                retValue = (inDataSize < calcDataSize) ? 0u : 1u;                
            }
            
            if(retValue)
            {    
                /* Unpack "Timestamp" field (7 bytes) if "Timestamp field is present" bit is present in "Flags" field */
                if(plxsSpotCheckMeasCharPtr->flags.bit.TMSF == SUPPORT) 
                {
                    memcpy(&plxsSpotCheckMeasCharPtr->timestamp, &inData[pduSize], SIZE_TIMESTAMP);
                    plxsSpotCheckMeasCharPtr->timestamp.year = 
                        Cy_BLE_Get16ByPtr((uint8_t *)&plxsSpotCheckMeasCharPtr->timestamp.year);
                    pduSize += SIZE_TIMESTAMP;     
                }
                
                /* Unpack "Measurement Status" field (16 bits) if "Measurement Status Field Present" bit is present in 
                   "Flags" field */
                if(plxsSpotCheckMeasCharPtr->flags.bit.MSF == SUPPORT) 
                {
                    value = (uint32)Cy_BLE_Get16ByPtr(&inData[pduSize]);
                    memcpy(&plxsSpotCheckMeasCharPtr->measStatus.value, &value, SIZE_16BIT);
                    pduSize += SIZE_16BIT;
                }
                
                /* Unpack "Device and Sensor Status" field (24 bits) if "Device and Sensor Status Field Present" bit is
                   present in "Flags" field */
                if(plxsSpotCheckMeasCharPtr->flags.bit.DSSF == SUPPORT)
                {
                    value = (uint32)Cy_BLE_Get24ByPtr(&inData[pduSize]);
                    memcpy(&plxsSpotCheckMeasCharPtr->dsStatus.value, &value, SIZE_24BIT);
                    pduSize += SIZE_24BIT;
                }
                
                /* Unpack "Pulse Amplitude Index" field (16 bits) if "Pulse Amplitude Index field is present" bit is 
                   present in "Flags" field */
                if(plxsSpotCheckMeasCharPtr->flags.bit.PAIF == SUPPORT)
                {
                    value = (uint32)Cy_BLE_Get16ByPtr(&inData[pduSize]);
                    memcpy(&plxsSpotCheckMeasCharPtr->pulseAmpIndex, &value, SIZE_16BIT);
                    pduSize += SIZE_16BIT;
                } 
            }
            break;
        }
        
        case CY_BLE_PLXS_CTMT:  /* Unpack PLX Continuous Measurement characteristic */
        {
            calcDataSize = SIZE_8BIT + SIZE_16BIT + SIZE_16BIT; /* Size of "Flags", "Normal SpO2", "Normal Pr"
                                                                   fields (mandatory) */
                      
            /* Check size of mandatory part of packet and unpack */
            if(inDataSize >= calcDataSize)
            {
                /* Unpack "Flags" field (8 bits) */
                plxsContMeasCharPtr->flags.value = inData[pduSize];
                pduSize += SIZE_8BIT;
                
                /* Unpack "SpO2PR-Normal - SpO2" field (16 bits) */
                value = (uint32)Cy_BLE_Get16ByPtr(&inData[pduSize]);
                memcpy(&plxsContMeasCharPtr->normalSpO2, &value, SIZE_16BIT);
                pduSize += SIZE_16BIT;
                
                /* Unpack "SpO2PR-Normal - PR" field (16 bits) */
                value = (uint32)Cy_BLE_Get16ByPtr(&inData[pduSize]);
                memcpy(&plxsContMeasCharPtr->normalPr, &value, SIZE_16BIT);
                pduSize += SIZE_16BIT;
                
                /* Check size of optional part of packet */
                if(plxsContMeasCharPtr->flags.bit.FAST == SUPPORT) { calcDataSize += (SIZE_16BIT + SIZE_16BIT); }
                if(plxsContMeasCharPtr->flags.bit.SLOW == SUPPORT) { calcDataSize += (SIZE_16BIT + SIZE_16BIT); }
                if(plxsContMeasCharPtr->flags.bit.MSF  == SUPPORT) { calcDataSize += SIZE_16BIT;                }
                if(plxsContMeasCharPtr->flags.bit.DSSF == SUPPORT) { calcDataSize += SIZE_24BIT;                }
                if(plxsContMeasCharPtr->flags.bit.PAIF == SUPPORT) { calcDataSize += SIZE_16BIT;                }
                retValue = (inDataSize < calcDataSize) ? 0u : 1u;    
            }            
            if(retValue)
            {   
                /* Unpack "SpO2PR-Fast - SpO2" and "SpO2PR-Fast - PR" fields (16/16 bits) if "SpO2PR–Fast field" bit is 
                   present in "Flags" field */
                if(plxsContMeasCharPtr->flags.bit.FAST == SUPPORT)
                {
                    value = (uint32)Cy_BLE_Get16ByPtr(&inData[pduSize]);
                    memcpy(&plxsContMeasCharPtr->fastSpO2, &value, SIZE_16BIT);
                    pduSize += SIZE_16BIT;
                    
                    value = (uint32)Cy_BLE_Get16ByPtr(&inData[pduSize]);
                    memcpy(&plxsContMeasCharPtr->fastPr, &value, SIZE_16BIT);
                    pduSize += SIZE_16BIT;
                }

                /* Unpack "SpO2PR-Slow - SpO2" and "SpO2PR-Fast - PR" fields (16/16 bits) if "SpO2PR–Fast field" bit is 
                   present in "Flags" field */
                if(plxsContMeasCharPtr->flags.bit.SLOW == SUPPORT)
                {
                    value = (uint32)Cy_BLE_Get16ByPtr(&inData[pduSize]);
                    memcpy(&plxsContMeasCharPtr->slowSpO2, &value, SIZE_16BIT);
                    pduSize += SIZE_16BIT;
                    
                    value = (uint32)Cy_BLE_Get16ByPtr(&inData[pduSize]);
                    memcpy(&plxsContMeasCharPtr->slowPr, &value, SIZE_16BIT);
                    pduSize += SIZE_16BIT;
                }         
                
                /* Unpack "Measurement Status" field (16 bits) if "Measurement Status Field Present" bit is present in 
                   "Flags" field */
                if(plxsContMeasCharPtr->flags.bit.MSF == SUPPORT)
                {
                    value = (uint32)Cy_BLE_Get16ByPtr(&inData[pduSize]);
                    memcpy(&plxsContMeasCharPtr->measStatus.value, &value, SIZE_16BIT);
                    pduSize += SIZE_16BIT;
                }
                
                /* Unpack "Device and Sensor Status" field (24 bits) if "Device and Sensor Status Field Present" bit is 
                   present in "Flags" field */
                if(plxsContMeasCharPtr->flags.bit.DSSF == SUPPORT)
                {
                    value = (uint32)Cy_BLE_Get24ByPtr(&inData[pduSize]);
                    memcpy(&plxsContMeasCharPtr->dsStatus.value, &value, SIZE_24BIT);
                    pduSize += SIZE_24BIT;
                }
                
                /* Unpack "Pulse Amplitude Index" field (16 bits) if "Pulse Amplitude Index field is present" bit is 
                   present in the "Flags" field */
                if(plxsContMeasCharPtr->flags.bit.PAIF == SUPPORT)
                {
                    value = (uint32)Cy_BLE_Get16ByPtr(&inData[pduSize]);
                    memcpy(&plxsContMeasCharPtr->pulseAmpIndex, &value, SIZE_16BIT);
                    pduSize += SIZE_16BIT;
                }     
            }
            break;   
        }
        
        case CY_BLE_PLXS_FEAT:  /* Unpack PLX Features characteristic */
        {   
            calcDataSize = SIZE_16BIT; /* Size of "Supported Features" field */    
            
            /* Check size of mandatory part of packet and unpack */
            if(inDataSize >= calcDataSize)
            {
                /* Unpack "Supported Features" field (16 bits) */
                value = (uint32)Cy_BLE_Get16ByPtr(&inData[pduSize]);
                memcpy(&plxsFeaturesCharPtr->supportedFeatures.value, &value, SIZE_16BIT);
                pduSize += SIZE_16BIT;
                
                /* Check size of optional part of packet */
                if(plxsFeaturesCharPtr->supportedFeatures.bit.MEAS == SUPPORT) { calcDataSize += SIZE_16BIT; }
                if(plxsFeaturesCharPtr->supportedFeatures.bit.DSS  == SUPPORT) { calcDataSize += SIZE_24BIT; }
                retValue = (inDataSize < calcDataSize) ? 0u : 1u;    
            }
            if(retValue)
            {    
                /* Unpack "Measurement Status Support" field (16 bits) if "Measurement Status support" bit is present
                   in "Supported Features field" */
                if(plxsFeaturesCharPtr->supportedFeatures.bit.MEAS == SUPPORT)
                {
                    value = (uint32)Cy_BLE_Get16ByPtr(&inData[pduSize]);
                    memcpy(&plxsFeaturesCharPtr->measStatusSupport.value, &value, SIZE_16BIT);
                    pduSize += SIZE_16BIT;
                }
                
                /* Unpack "Device and Sensor Status Support" field (24 bits) if "Device and Sensor Status support"
                   bit is present in "Supported Features field" */
                if(plxsFeaturesCharPtr->supportedFeatures.bit.DSS == SUPPORT)
                {
                    value = (uint32)Cy_BLE_Get24ByPtr(&inData[pduSize]);
                    memcpy(&plxsFeaturesCharPtr->dsStatusSupport.value, &value, SIZE_24BIT);
                    pduSize += SIZE_24BIT;
                }
            }
            break;
        }
        
        case CY_BLE_PLXS_RACP:  /* Unpack PLX Record Access Control Point characteristic */
        {
            /* Check size of mandatory part of packet and unpack */
            if(inDataSize >= (SIZE_8BIT + SIZE_8BIT)) /* Size of mandatory fields of RACP: opCode(8 bit) & operator(8 bit) */
            {
                retValue = 1u; /* correct input data */
            }
            
            if(retValue)
            {
                plxsRacpCharPtr->opCode = (cy_en_ble_plxs_racp_opc_t)inData[pduSize];
                pduSize += SIZE_8BIT;
                
                plxsRacpCharPtr->operator = (cy_en_ble_plxs_racp_opr_t)inData[pduSize];
                pduSize += SIZE_8BIT;
                
                /* Check if operand byte came */
                plxsRacpCharPtr->operandIsPresent = ((inDataSize - pduSize) == 0u) ? false : true;
                
                if(inDataSize >= (pduSize + SIZE_16BIT))
                {
                    value = (uint32)Cy_BLE_Get16ByPtr((uint8_t *)&inData[pduSize]);
                    memcpy(&plxsRacpCharPtr->operand.value, &value, SIZE_16BIT);
                    pduSize += SIZE_16BIT;
                }
                else
                {
                    plxsRacpCharPtr->operand.value = 0x0000;
                }
            }
            break;  
        }
        
        default:
            break;  
    }
    return (retValue);
}

/*******************************************************************************
* Function Name: PlxsSimulateMeasurement()
********************************************************************************
*
* Summary:
*   The function simulates PLXS data.
*
*******************************************************************************/
void PlxsSimulateMeasurement(cy_stc_ble_conn_handle_t connHandle)
{  
    cy_en_ble_api_result_t apiResult;
    cy_stc_rtc_config_t dateTime;   
    uint16_t cccd;    
    uint8_t pduSize;
    uint8_t pduData[PLXS_MAX_PDU_SIZE];
    
    static sfloat_t spO2          = { .m = PLXS_SIM_MIN_SPO2 };
    static sfloat_t pulseRate     = { .m = PLXS_SIM_MIN_PR   };
    static sfloat_t pulseAmpIndex = { .m = PLXS_SIM_MIN_PAI  };
    static uint32_t timerSimPlxs = 0u;       
    
    if( ((Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED) || (startSpotCheckFlag == true)) &&
        ((timerSimPlxs++ >= PLXS_SIM_TIMEOUT) && (plxsRacpOpr.state == APP_RACP_STATE_IDLE)))
    {
        timerSimPlxs = 1u;  /* reset plxs timer */
        
        /*********************************************************************
        * Simulate SpO2(%), Pr(bbm) and PAI(%)
        ********************************************************************/

        /* Calculate values */
        /* Increase only mantissa (full range 0 - 100, resolution 1), exponent = 0 */
        spO2.m = (spO2.m + PLXS_SIM_STEP_SPO2) < PLXS_SIM_MAX_SPO2 ? 
                                                                (spO2.m + PLXS_SIM_STEP_SPO2) : PLXS_SIM_MIN_SPO2;
        spO2.exp = 0u;

        /* Increase  mantissa (full range 0 - 360, resolution 1), exponent = 0 */
        pulseRate.m = (pulseRate.m + PLXS_SIM_STEP_PR) < PLXS_SIM_MAX_PR ?
                                                                (pulseRate.m + PLXS_SIM_STEP_PR) : PLXS_SIM_MIN_PR;
        pulseRate.exp = 0u;

        /* Increase  mantissa (full range 0.01 - 20), exponent = 10^-2  */
        pulseAmpIndex.m = (pulseAmpIndex.m + PLXS_SIM_STEP_PAI) < PLXS_SIM_MAX_PAI ? 
                                                          (pulseAmpIndex.m + PLXS_SIM_STEP_PAI): PLXS_SIM_MIN_PAI;
        pulseAmpIndex.exp = EXP_10M2;   /* exp: 10^-2 */
        
        /*********************************************************************
        * Update The PLX Spot-Check Measurement characteristic 
        ********************************************************************/
        
        /* Update SpO2 */
        plxsSpotCheckMeasChar.spO2 = spO2;
        
        /* Update Pr */
        plxsSpotCheckMeasChar.Pr = pulseRate; 
        
        /* Update Pulse Amp Index */
        if((plxsFeaturesChar.supportedFeatures.bit.PAI == SUPPORT) && (plxsSpotCheckMeasChar.flags.bit.PAIF == true))
        {
            plxsSpotCheckMeasChar.pulseAmpIndex = pulseAmpIndex; 
        }
        
        /* Update timestamp */
        if((plxsFeaturesChar.supportedFeatures.bit.TMSF == SUPPORT) && (plxsSpotCheckMeasChar.flags.bit.TMSF == true))
        {
            /* Get Date and Time from RTC */
            Cy_RTC_GetDateAndTime(&dateTime);
    
            /* Update timestamp date */
            plxsSpotCheckMeasChar.timestamp.year  = (uint16_t)dateTime.year;
            plxsSpotCheckMeasChar.timestamp.month = (uint8_t)dateTime.month;
            plxsSpotCheckMeasChar.timestamp.day   = (uint8_t)dateTime.dayOfWeek;
            
            /* Update timestamp time */
            plxsSpotCheckMeasChar.timestamp.hours   = (uint8_t)dateTime.hour;
            plxsSpotCheckMeasChar.timestamp.minutes = (uint8_t)dateTime.min;
            plxsSpotCheckMeasChar.timestamp.seconds = (uint8_t)dateTime.sec;
        }
        
        /* Set Flag "Fully Qualified Data" */
        if((plxsFeaturesChar.supportedFeatures.bit.MEAS == SUPPORT) && 
           (plxsFeaturesChar.measStatusSupport.bit.FQDATA == SUPPORT) && (plxsSpotCheckMeasChar.flags.bit.MSF == true))
        {
            plxsSpotCheckMeasChar.measStatus.bit.FQDATA = true;
        }
             
        /*********************************************************************
        * Update the PLX Continuous Measurement characteristic
        ********************************************************************/
        
        /* Update SpO2/Pr */
        plxsContMeasChar.normalSpO2 = spO2;
        plxsContMeasChar.normalPr = pulseRate; 
        
        /* Update FAST SpO2/Pr */        
        if((plxsFeaturesChar.supportedFeatures.bit.FAST == SUPPORT) && (plxsContMeasChar.flags.bit.FAST == true))
        {
            plxsContMeasChar.fastSpO2 = spO2;
            plxsContMeasChar.fastPr   = pulseRate; 
        }
        
        /* Update SLOW SpO2/Pr */
        if((plxsFeaturesChar.supportedFeatures.bit.SLOW == SUPPORT) && (plxsContMeasChar.flags.bit.SLOW == true))
        { 
            plxsContMeasChar.slowSpO2 = spO2;
            plxsContMeasChar.fastPr   = pulseRate;  
        }
        
        /* Update Pulse Amp Index */
        if((plxsFeaturesChar.supportedFeatures.bit.PAI == SUPPORT) && (plxsContMeasChar.flags.bit.PAIF == true))
        {
            plxsContMeasChar.pulseAmpIndex = pulseAmpIndex; 
        }
        
        /* Set Flag "Fully Qualified Data" */
        if((plxsFeaturesChar.supportedFeatures.bit.MEAS == SUPPORT) && 
           (plxsFeaturesChar.measStatusSupport.bit.FQDATA == SUPPORT) && (plxsContMeasChar.flags.bit.MSF == true))
        {
            plxsContMeasChar.measStatus.bit.FQDATA = true;
        }
          
        /*********************************************************************
        * Display simulated data
        ********************************************************************/
        DBG_PRINTF("Simulated data: [ Date: %.2ld-%.2ld-%.2ld  Time: %.2ld:%.2ld:%.2ld ]"\
                   " spO2: %.2f PR: %.2f PI: %.2f \r\n",
                    dateTime.dayOfWeek, dateTime.month, dateTime.year, dateTime.hour, dateTime.min, dateTime.sec,
                    spO2.m * pow(10.0, spO2.exp), pulseRate.m * pow(10.0, pulseRate.exp),
                    pulseAmpIndex.m * pow(10.0, pulseAmpIndex.exp));
        
        
        /*********************************************************************
        * Sent Notifications / Indications
        ********************************************************************/
        
        /* Sent Notifications of The PLX Continuous Measurement (CTMT) characteristic */
        if((Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED) && (Cy_BLE_IsDevicePaired(&connHandle) == true))
        {
            
            /* Check if the notification is enabled */
            (void) Cy_BLE_PLXSS_GetCharacteristicDescriptor(connHandle, CY_BLE_PLXS_CTMT, CY_BLE_PLXS_CCCD,
                                                            CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
            if(cccd == CY_BLE_CCCD_NOTIFICATION)
            {
                pduSize = PlxsPackData(CY_BLE_PLXS_CTMT, &plxsContMeasChar, pduData);
                apiResult = Cy_BLE_PLXSS_SendNotification(connHandle, CY_BLE_PLXS_CTMT, pduSize, pduData);  
                if(apiResult == CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("INFO: the Continuous Measurement characteristic was notified successfully \r\n");
                }
                else
                {
                    DBG_PRINTF("Cy_BLE_PLXSS_SendNotification() failed. API Error: 0x%x \r\n", apiResult);
                }
            }
        }
        
        /* Sent Indications of the PLX Spot-check Measurement (SCMT) characteristic */
        if(startSpotCheckFlag == true)
        {
            bool storeRacpFlag = true;
            
            if(startSpotCheckTimer == 0u)
            {
                DBG_PRINTF("INFO: start the Spot-check procedure \r\n");
            }
            
            startSpotCheckTimer++;
            
            if((Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED) && (Cy_BLE_IsDevicePaired(&connHandle) == true))
            {
                /* Send PLX Spot-check Measurement characteristic indication */
                (void) Cy_BLE_PLXSS_GetCharacteristicDescriptor(connHandle, CY_BLE_PLXS_SCMT,
                                                                CY_BLE_PLXS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
                if(cccd == CY_BLE_CCCD_INDICATION)
                {
                    pduSize = PlxsPackData(CY_BLE_PLXS_SCMT, &plxsSpotCheckMeasChar, pduData);
                    apiResult = Cy_BLE_PLXSS_SendIndication(connHandle, CY_BLE_PLXS_SCMT, pduSize, pduData);  
                    
                    if(apiResult == CY_BLE_SUCCESS)
                    {
                        DBG_PRINTF("INFO: the PLX Spot-check Measurement characteristic was indicated successfully \r\n");
                        storeRacpFlag = false;   
                    }
                    else
                    {
                        DBG_PRINTF("Cy_BLE_PLXSS_SendIndication() failed. API Error: 0x%x \r\n", apiResult);  
                    }
                }
            }
            
            /* Store the Spot-check measurement record to PACP storage because we have the Spot-check measurement 
                session, but in some reason do not have a connection / or indication was not enabled */
            if(storeRacpFlag == true)
            {   
                PlxsRacpStoragePushRecord(&plxsSpotCheckMeasChar);
            }

            /* Stop generates of Spot-check events */
            if(startSpotCheckTimer >= PLXS_SIM_SCMT_MT_PERIOD_COUNT)
            {
                DBG_PRINTF("INFO: finish the Spot-check procedure \r\n");
                startSpotCheckFlag = false;    
                startSpotCheckTimer = 0u;
            }
            
            DBG_PRINTF("\r\n");  
        }
    }
}

/*******************************************************************************
* Function Name: PlxsStartSpotCheckMeasurement()
********************************************************************************
*
* Summary:
*   The function initiates the spot check measurement
*
*******************************************************************************/
void PlxsStartSpotCheckMeasurement(void)
{
    startSpotCheckFlag = true;
}


/*******************************************************************************
* Function Name: PlxsIsSpotCheckMeasurement()
********************************************************************************
*
* Summary:
*   The function returns the spot-check measurement status
* 
* Return:
*  true  - spot-check measurment is running, 
*  false - spot-check measurment is not running
*
*******************************************************************************/
bool PlxsIsSpotCheckMeasurement(void)
{
    return(startSpotCheckFlag);
}


/* Helper Procedures */

uint32_t Cy_BLE_Get32ByPtr(const uint8_t ptr[])
{
    return (((uint32) ptr[0u]) | ((uint32)(((uint32) ptr[1u]) << 8u)) | ((uint32)((uint32) ptr[2u]) << 16u));
}

    
/* [] END OF FILE */
