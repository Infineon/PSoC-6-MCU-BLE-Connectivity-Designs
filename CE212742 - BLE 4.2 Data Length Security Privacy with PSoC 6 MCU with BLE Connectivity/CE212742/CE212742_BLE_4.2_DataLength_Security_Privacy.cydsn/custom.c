/*******************************************************************************
* File Name: custom.c
*
* Version: 1.0
*
* Description:
*  This file contains custom callback handler function and code for custom 
*  service.
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
#include "custom.h"


/*******************************************************************************
* Global variables
*******************************************************************************/
uint16_t dataSendCccd = 0u;
uint16_t dataSendEnabled = 0u;
uint8_t dataSendConfirmed;
cy_stc_ble_custom_config_char_t customConfiguration;
uint8_t notificationBuffer[CY_BLE_GATT_MTU];
volatile uint32_t totalByteCounter = 0u;
uint16_t dataSendReduceLength = 0u;


/***************************************
* External data references
***************************************/
extern volatile uint32_t mainTimer;


/*******************************************************************************
* Function Name: CustomCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE service.
*
* Parameters:
*  event - the event code
*  *eventParam - the event parameters
*
* Return:
*  None.
*
*******************************************************************************/
void CustomCallBack(uint32_t event, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr;
    
    switch(event)
    {
         /* Check if the Client Characteristic Configuration Descriptor is
         *  written to. Notification is enabled or disabled accordingly.
         */
        case CY_BLE_EVT_GATTS_WRITE_REQ:
        {
            cy_stc_ble_gatt_write_param_t *writeParam = (cy_stc_ble_gatt_write_param_t *)eventParam;
            /* Store data in database */
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo =
            {
                .handleValuePair = writeParam->handleValPair,
                .connHandle      = writeParam->connHandle,
                .offset          = 0u,
                .flags           = CY_BLE_GATT_DB_PEER_INITIATED
                
            };
            
            DBG_PRINTF("CY_BLE_EVT_GATTS_WRITE_REQ ");
            
            /* Store value to database */
            gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
            
            if(CY_BLE_GATT_ERR_NONE == gattErr)
            {
                Cy_BLE_GATTS_WriteRsp(appConnHandle);
                if(writeParam->handleValPair.attrHandle == 
                   CY_BLE_THROUGHPUT_SERVICE_TX_DATA_CHARACTERISTIC_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
                {
                #if(CY_BLE_BONDING_REQUIREMENT == CY_BLE_BONDING_YES)
                    /* Set flag to store bonding data to flash */
                    if(cy_ble_peerBonding[writeParam->connHandle.attId] == CY_BLE_GAP_BONDING)
                    {
                        cy_ble_pendingFlashWrite |= CY_BLE_PENDING_CCCD_FLASH_WRITE_BIT;
                    }
                #endif /* (CY_BLE_BONDING_REQUIREMENT == CY_BLE_BONDING_YES) */
                    totalByteCounter = 0u;
                    
                    dataSendCccd = Cy_BLE_Get16ByPtr(writeParam->handleValPair.value.val);
                    if(dataSendCccd != 0u)
                    {
                        dataSendConfirmed = 0;
                        MCWDT_ResetCounters(CY_MCWDT_CTR0, 100u);
                        mainTimer = 0u;                
                        DBG_PRINTF("Start data transmitting\r\n");
                        MCWDT_ResetCounters(CY_MCWDT_CTR2, 100u);
                    }
                    else
                    {
                        DBG_PRINTF("Stop data transmitting\r\n");
                    }
                    dataSendEnabled = dataSendCccd;
                }
                else if(writeParam->handleValPair.attrHandle == 
                        CY_BLE_THROUGHPUT_SERVICE_CONFIGURATION_CHARACTERISTIC_CHAR_HANDLE)
                {
                    memcpy(&customConfiguration, writeParam->handleValPair.value.val, (writeParam->handleValPair.value.len 
                     < sizeof(customConfiguration) ? writeParam->handleValPair.value.len : sizeof(customConfiguration)));
                    DBG_PRINTF("config mode=%d, len=%ld \r\n", customConfiguration.mode, customConfiguration.dataLength);
                }
                else
                {
                    DBG_PRINTF("\r\n");
                }
            }
            else
            {
                cy_stc_ble_gatt_err_param_t err_param;
                
                err_param.errInfo.opCode = CY_BLE_GATT_WRITE_REQ;
                err_param.errInfo.attrHandle = writeParam->handleValPair.attrHandle;
                err_param.errInfo.errorCode = gattErr;
                err_param.connHandle = writeParam->connHandle;
                
                /* Send Error Response */
                (void)Cy_BLE_GATTS_ErrorRsp(&err_param);
                DBG_PRINTF("Write Attribute Error %d\r\n", gattErr);
            }
            break;
        }
        case CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF:
            DBG_PRINTF("CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF\r\n"); 
            dataSendConfirmed = 1;
            break;
            
        case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
            DBG_PRINTF("CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ, attHandle: %d \r\n", 
                ((cy_stc_ble_gatts_char_val_read_req_t *)eventParam)->attrHandle);
            break;

        default:
            DBG_PRINTF("OTHER event: %lx \r\n", event);
            break;
    }
}


/*******************************************************************************
* Function Name: CustomSendData()
********************************************************************************
* Summary:
* This function creates a notification packet and sends it.
*
* Theory:
* The function creates a notification packet for the outgoing data custom 
* characteristic and sends it over BLE. The amount of data sent for the 
* characteristic is equal to (negotiated MTU size - 3) bytes.
* In the Limited mode there is no indication when the last packet is delivered 
* to a peer device, therefore, the last packet is sent by Indication to receive
* a confirmation of delivery of all amount of data.
*
*******************************************************************************/
void CustomSendData(void)
{
    /* Create a new notification packet */
    cy_en_ble_api_result_t apiResult;
    static uint32_t packetN = 0u;
    static uint8_t led = LED_OFF;
    cy_stc_ble_gatts_handle_value_ntf_t notificationPacket =
    {
        /* Fill all fields of write request structure ... */
        .handleValPair.attrHandle = CY_BLE_THROUGHPUT_SERVICE_TX_DATA_CHARACTERISTIC_CHAR_HANDLE,
        .handleValPair.value.val  = notificationBuffer,
        .handleValPair.value.len  = CY_BLE_NOTIFICATION_HEADER_LEN,
        .connHandle               = appConnHandle
    };
    cy_stc_ble_gatt_xchg_mtu_param_t mtuSize = 
    {
        .connHandle = appConnHandle
    };
    
    /* Update Notification packet with the data.
     * Maximum data which can be sent is equal to (Negotiated MTU - 3) bytes.
     */
    Set32ByPtr(notificationBuffer, packetN);
    Cy_BLE_GATT_GetMtuSize(&mtuSize);
    mtuSize.mtu -= dataSendReduceLength;
    notificationPacket.handleValPair.value.len = mtuSize.mtu - CY_BLE_NOTIFICATION_HEADER_LEN;
    if((customConfiguration.mode == CY_BLE_CUSTOM_MODE_LIMITED) &&
       ((totalByteCounter + notificationPacket.handleValPair.value.len) >= customConfiguration.dataLength))
    {
        /* Limit the length of the last packet */
        notificationPacket.handleValPair.value.len = customConfiguration.dataLength - totalByteCounter;
        /* Send last packet by indication to get response */
        apiResult = Cy_BLE_GATTS_Indication(&notificationPacket);
        if(apiResult == CY_BLE_SUCCESS)
        {
            /* Stop transmission */
            dataSendEnabled = 0;
        }
    }
    else
    {
        /* Report data to BLE component */
        apiResult = Cy_BLE_GATTS_Notification(&notificationPacket);
    }
    if(apiResult == CY_BLE_SUCCESS)
    {
        
        /* Increment packet number, placed on the first two bytes in the packet */
        packetN++;
        totalByteCounter += notificationPacket.handleValPair.value.len;
        led ^= LED_OFF;
    #if(CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV)
        Simulation_LED_Write(led);
    #endif /* (CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV) */
    }
}


/*******************************************************************************
* Function Name: CustomInit()
********************************************************************************
*
* Summary:
*   Initializes a custom service.
*
*******************************************************************************/
void CustomInit(void)
{
    uint32_t counter;
    
    /* Read CCCD configurations from flash */
    cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo = 
    {
        .handleValuePair.attrHandle = CY_BLE_THROUGHPUT_SERVICE_TX_DATA_CHARACTERISTIC_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE,
        .handleValuePair.value.len  = sizeof(dataSendEnabled),
        .handleValuePair.value.val  = (uint8_t *)&dataSendCccd,
        .flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED
    };
    
    if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
    {
        DBG_PRINTF("Cy_BLE_GATTS_ReadAttributeValue API Error \r\n");
    }

    dataSendConfirmed = 0;
    dataSendEnabled = dataSendCccd;
    
    /* Initialize entire buffer; the amount of data send will 
     * depend on the peer device's MTU */
    for(counter = 0; counter < CY_BLE_GATT_MTU; counter++)
    {
        notificationBuffer[counter] = counter;
    }

    /* Get configuration characteristic value from GATT database */
    dbAttrValInfo.handleValuePair.attrHandle = CY_BLE_THROUGHPUT_SERVICE_CONFIGURATION_CHARACTERISTIC_CHAR_HANDLE;
    dbAttrValInfo.handleValuePair.value.len = sizeof(customConfiguration);
    dbAttrValInfo.handleValuePair.value.val = (uint8_t *)&customConfiguration;

    if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
    {
        DBG_PRINTF("Cy_BLE_GATTS_ReadAttributeValue API Error\r\n");
    }
    
}


/* [] END OF FILE */

