/*******************************************************************************
* File Name: bmss.c
*
* Version 1.0
*
* Description:
*  This file contains the Bond Management Service related code.
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
#include "bmss.h"

    
static cy_stc_ble_gap_peer_addr_info_t bondedDeviceInfo[CY_BLE_GAP_MAX_BONDED_DEVICE];
static cy_stc_ble_gap_bonded_device_list_info_t bdList =
{
    .bdHandleAddrList = bondedDeviceInfo
};

static cy_stc_ble_gap_peer_addr_info_t peerAddrInfo;
                    
static uint8_t bmcp = 0u;

static cy_stc_ble_bmss_bmft_char_t bmsFeatureChar;

/******************************************************************************
* Function Name: BmsInit
*******************************************************************************
*
* Summary:
*   Initializes the battery service.
*
******************************************************************************/
void BmsInit(void)
{
    uint8_t pduData[CY_BLE_BMS_BMFT_SIZE];
    uint8_t charSize;
       
    /* Register service specific callback function */
    Cy_BLE_BMS_RegisterAttrCallback(BmsCallBack);
    
    /* Init global variables */
    charSize = CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_MAX_LEN(cy_ble_bmsConfig.bmss->charInfo[CY_BLE_BMS_BMFT].charHandle);
    Cy_BLE_BMSS_GetCharacteristicValue(CY_BLE_BMS_BMFT, charSize, pduData);
    BmsUnPackData(CY_BLE_BMS_BMFT, charSize, pduData, &bmsFeatureChar);
        
}


/******************************************************************************
* Function Name: BmsCallBack
*******************************************************************************
*
* Summary:
*   A CallBack function to handle BMS specific events.
*
*  Parameters:
*   event      - the event code
*   eventParam - the event parameters
*
******************************************************************************/
void BmsCallBack(uint32_t event, void* eventParam)
{   
    cy_en_ble_api_result_t apiResult;
    uint32_t i;
    char8 *authCode = "0abcdef";

    switch(event)
    {
        case CY_BLE_EVT_BMSS_WRITE_CHAR:
        {
            bool opCodeFlag;
            bool authCodeFlag = false;
            
            cy_en_ble_gatt_err_code_t bmsGattError = CY_BLE_GATT_ERR_NONE;
            
            DBG_PRINTF("BMCP write request: ");
            for(i = 0; i < ((cy_stc_ble_bms_char_value_t*)eventParam)->value->len; i++)
            {
                DBG_PRINTF("%2.2x ", ((cy_stc_ble_bms_char_value_t*)eventParam)->value->val[i]);
            }
            DBG_PRINTF("\r\n");
            
            bmcp = ((cy_stc_ble_bms_char_value_t*)eventParam)->value->val[0];
            
            switch(bmcp)
            {
                case CY_BLE_BMS_BMCP_OPC_RD:
                    DBG_PRINTF("OpCode: Delete bond of requesting device\r\n");
                    opCodeFlag = bmsFeatureChar.features.bit.DELC_LE;
                    authCodeFlag = bmsFeatureChar.features.bit.DELC_LE_AUTH;
                    break;
                    
                case CY_BLE_BMS_BMCP_OPC_AB:
                    DBG_PRINTF("OpCode: Delete all bonds on server\r\n");
                    opCodeFlag = bmsFeatureChar.features.bit.REMA_LE;
                    authCodeFlag = bmsFeatureChar.features.bit.REMA_LE_AUTH;
                    break;
                        
                case CY_BLE_BMS_BMCP_OPC_BA:
                    DBG_PRINTF("OpCode: Delete all but the active bond on server\r\n");
                    opCodeFlag = bmsFeatureChar.features.bit.REMAB_LE;
                    authCodeFlag = bmsFeatureChar.features.bit.REMAB_LE_AUTH;
                    break;
                
                default:
                    DBG_PRINTF("unsupported BMCP OpCode: 0x%x \r\n", 
                                    ((cy_stc_ble_bms_char_value_t*)eventParam)->value->val[0]);
                    bmsGattError = CY_BLE_GATT_ERR_OP_CODE_NOT_SUPPORTED;
                    break;
            }
                    
            /* Check if opCode is supported */
            if(bmsGattError == CY_BLE_GATT_ERR_NONE)
            {
                bmsGattError = (opCodeFlag == true) ? bmsGattError : CY_BLE_GATT_ERR_OP_CODE_NOT_SUPPORTED;
            }
            
            /* Check authorization code */
            if((bmsGattError == CY_BLE_GATT_ERR_NONE) && (authCodeFlag == true))
            {
                if(((cy_stc_ble_bms_char_value_t*)eventParam)->value->len == (CY_BLE_BMS_AUTH_CODE_SIZE + 1))
                {
                    for(i = 1u; i < ((cy_stc_ble_bms_char_value_t*)eventParam)->value->len; i++)
                    {
                        if(((cy_stc_ble_bms_char_value_t*)eventParam)->value->val[i] != authCode[i])
                        {
                            bmsGattError = CY_BLE_GATT_ERR_INSUFFICIENT_AUTHORIZATION;
                            break;
                        }
                    }
                }
                else
                {
                    bmsGattError = CY_BLE_GATT_ERR_INSUFFICIENT_AUTHORIZATION;
                }     
            }
            
            /* Get peer bd address for current connection */
            if(bmsGattError == CY_BLE_GATT_ERR_NONE)
            {                                       
                peerAddrInfo.bdHandle = ((cy_stc_ble_bms_char_value_t*)eventParam)->connHandle.bdHandle;
                apiResult = Cy_BLE_GAP_GetPeerBdAddr(&peerAddrInfo);
                  DBG_PRINTF("Cy_BLE_GAP_GetPeerBdAddr API result: %x \r\n", apiResult);

                apiResult = Cy_BLE_GAP_GetBondList(&bdList);
                DBG_PRINTF("Cy_BLE_GAP_GetBondedDevicesList API result: %x [%x ] \r\n", apiResult,bdList.noOfDevices);           
                
                for(i = 0u; i < bdList.noOfDevices; i++)
                {
                    DBG_PRINTF("%lx. address: %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x  \r\n", i,
                        bdList.bdHandleAddrList[i].bdAddr.bdAddr[5u], bdList.bdHandleAddrList[i].bdAddr.bdAddr[4u],
                        bdList.bdHandleAddrList[i].bdAddr.bdAddr[3u], bdList.bdHandleAddrList[i].bdAddr.bdAddr[2u],
                        bdList.bdHandleAddrList[i].bdAddr.bdAddr[1u], bdList.bdHandleAddrList[i].bdAddr.bdAddr[0u]);
                }
            }
            else
            {
                bmcp = 0u;   
            }
        
            DBG_PRINTF("GATT error: ");
            switch(bmsGattError)
            {
                case CY_BLE_GATT_ERR_NONE:
                    DBG_PRINTF("Ok\r\n");
                    break;
                
                case CY_BLE_GATT_ERR_OP_CODE_NOT_SUPPORTED:
                    DBG_PRINTF("Op Code Not Supported\r\n");
                    break;
                    
                case CY_BLE_GATT_ERR_INSUFFICIENT_AUTHORIZATION:
                    DBG_PRINTF("Insufficient Authorization\r\n");
                    break;
                
                default:
                    DBG_PRINTF("unexpected 0x%2.2x\r\n", bmsGattError);
                    break;
            }
        
            ((cy_stc_ble_bms_char_value_t*)eventParam)->gattErrorCode = bmsGattError;
        }
        break;
            
        default:
            DBG_PRINTF("unknown BMS event: 0x%lx \r\n", event);
            break;
    }
}

/******************************************************************************
* Function Name: BmsProcess
*******************************************************************************
*
* Summary:
*   Processes the BM control point requests. 
*
******************************************************************************/
void BmsProcess(void)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t i;
    
    if((bmcp != 0u) && (Cy_BLE_GetNumOfActiveConn() == 0u))
    {
        switch(bmcp)
        {
            /* Delete bond of requesting device. */ 
            case CY_BLE_BMS_BMCP_OPC_RD:
            
                DBG_PRINTF(" peerAddr: %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x  \r\n",
                                            peerAddrInfo.bdAddr.bdAddr[5u], peerAddrInfo.bdAddr.bdAddr[4u],
                                            peerAddrInfo.bdAddr.bdAddr[3u], peerAddrInfo.bdAddr.bdAddr[2u],
                                            peerAddrInfo.bdAddr.bdAddr[1u], peerAddrInfo.bdAddr.bdAddr[0u]);
                
                apiResult = Cy_BLE_GAP_RemoveDeviceFromBondList(&peerAddrInfo.bdAddr);
                DBG_PRINTF("Cy_BLE_GAP_RemoveDeviceFromBondList API result: 0x%x \r\n ",apiResult);

                apiResult = Cy_BLE_GAP_GetBondList(&bdList);
                    DBG_PRINTF("Cy_BLE_GAP_GetBondedDevicesList API result: %x [%x ] \r\n", apiResult,bdList.noOfDevices);
                break;
            
            /* Delete all bonds on server. */
            case CY_BLE_BMS_BMCP_OPC_AB:
                {
                    cy_stc_ble_gap_bd_addr_t bdAddr = { .type = 0x0u };
                    apiResult = Cy_BLE_GAP_RemoveDeviceFromBondList(&bdAddr);
                    DBG_PRINTF("Cy_BLE_GAP_RemoveDeviceFromBondList API result: 0x%x \r\n ",apiResult);
                        
                }  
                break;
            
            /* Delete all but the active bond on server. */    
            case CY_BLE_BMS_BMCP_OPC_BA:
                for(i = 0u; i < bdList.noOfDevices; i++)
                {
                    if(memcmp(bdList.bdHandleAddrList[i].bdAddr.bdAddr, 
                                        peerAddrInfo.bdAddr.bdAddr, CY_BLE_GAP_BD_ADDR_SIZE) != 0u)
                    {
                        apiResult = Cy_BLE_GAP_RemoveDeviceFromBondList(&bdList.bdHandleAddrList[i].bdAddr);
                        DBG_PRINTF("Cy_BLE_GAP_RemoveDeviceFromBondList API result: 0x%x \r\n ",apiResult);
                    }
                }
                break;
                
            default:
                break;
        }
        
        bmcp = 0u;
        
        apiResult = Cy_BLE_GAP_GetBondList(&bdList);
                    DBG_PRINTF("Cy_BLE_GAP_GetBondedDevicesList API result: %x [%x ] \r\n", apiResult,bdList.noOfDevices);
        for(i = 0u; i < bdList.noOfDevices; i++)
        {
             DBG_PRINTF("%lx. address: %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x  \r\n", i,
                            bdList.bdHandleAddrList[i].bdAddr.bdAddr[5u], bdList.bdHandleAddrList[i].bdAddr.bdAddr[4u],
                            bdList.bdHandleAddrList[i].bdAddr.bdAddr[3u], bdList.bdHandleAddrList[i].bdAddr.bdAddr[2u],
                            bdList.bdHandleAddrList[i].bdAddr.bdAddr[1u], bdList.bdHandleAddrList[i].bdAddr.bdAddr[0u]);
        }
        
    }
}


/*******************************************************************************
* Function Name: BmsUnPackData
********************************************************************************
*
* Summary:
*   Unpacks received PDU array to application BM characteristics structure.
*
* Parameters:
*   charIdx     - index of BM characteristic(defined in cy_en_ble_bms_char_index_t enum)
*   inDataSize  - size of input data
*   *inData     - pointer on PDU array
*   *outData    - pointer on BM characteristics structure
*
* Return:
*  1u - correct input data, 0u - incorrect input data
*******************************************************************************/
bool BmsUnPackData(cy_en_ble_bms_char_index_t charIdx, uint8_t inDataSize, uint8_t *inData, void *outData)
{
    uint32_t pduSize = 0u;
    uint32_t value;
    uint32_t calcDataSize;
    bool retValue = true;     /* 1u - correct input data; 0u - incorrect input data; */
    
    /* declaration pointers on characteristics */
    cy_stc_ble_bmss_bmft_char_t *bmsFeaturesCharPtr      = (cy_stc_ble_bmss_bmft_char_t *)outData;
    
    /* Unpack the BMS Features characteristic */
    if(charIdx == CY_BLE_BMS_BMFT)
    {
        calcDataSize = CY_BLE_BMS_BMFT_SIZE; /* 3 bytes of mandatory field */        
       
        /* Check size of mandatory part of packet and unpack */
        if(inDataSize >= calcDataSize)
        {
            /* Unpack "BMS Features" field (24 bit) */
            value = (uint32)Cy_BLE_Get24ByPtr(&inData[pduSize]);
            memcpy(&bmsFeaturesCharPtr->features.value, &value, CY_BLE_BMS_BMFT_SIZE);
            pduSize += CY_BLE_BMS_BMFT_SIZE;     
        }
        else
        {
            retValue = false; /* incorrect packet size */      
        }
    }
    return (retValue);
}

    
/* [] END OF FILE */
