/*******************************************************************************
* File Name: cgmss.c
*
* Version 1.0
*
* Description:
*  This file contains the Continuous Glucose Monitoring Service related code.
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
*
*
********************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cgmss.h"
    
static uint8_t cgmsFlag = 0u;
static uint8_t recCnt = 1u;
static uint8_t racpOpCode = 0u;
static uint8_t racpOperator = 0u;
static uint8_t racpOperand[3u];

static cy_en_ble_cgms_socp_opc_t socpOpCode;
static uint8_t socp[13];
static uint8_t socpLength = 0u;
static sfloat socpOpd = 0u;
static sfloat* socpOpdPtr = &socpOpd;
static uint8_t commInterval = 5u;
static uint16_t socpRecNum = 1u;
static cy_stc_ble_cgms_alrt_t alrt = {  CY_BLE_SIM_ALERT_LEVEL,
                                        CY_BLE_SIM_ALERT_LEVEL,
                                        CY_BLE_SIM_ALERT_LEVEL,
                                        CY_BLE_SIM_ALERT_LEVEL,
                                        CY_BLE_SIM_ALERT_LEVEL,
                                        CY_BLE_SIM_ALERT_LEVEL};

static uint8_t attr[24u];
static uint8_t length = 0u;
static cy_stc_ble_cgms_cgft_t cgft;
static cy_stc_ble_cgms_sstm_t sstm;
static const cy_en_ble_time_zone_t timeZone[CY_BLE_TIME_ZONE_VAL_NUM] =
{
    CY_BLE_TIME_ZONE_M1200, /* UTC-12:00 */
    CY_BLE_TIME_ZONE_M1100, /* UTC-11:00 */
    CY_BLE_TIME_ZONE_M1000, /* UTC-10:00 */
    CY_BLE_TIME_ZONE_M0930, /* UTC-9:30 */
    CY_BLE_TIME_ZONE_M0900, /* UTC-9:00 */
    CY_BLE_TIME_ZONE_M0800, /* UTC-8:00 */
    CY_BLE_TIME_ZONE_M0700, /* UTC-7:00 */
    CY_BLE_TIME_ZONE_M0600, /* UTC-6:00 */
    CY_BLE_TIME_ZONE_M0500, /* UTC-5:00 */
    CY_BLE_TIME_ZONE_M0430, /* UTC-4:30 */
    CY_BLE_TIME_ZONE_M0400, /* UTC-4:00 */
    CY_BLE_TIME_ZONE_M0330, /* UTC-3:30 */
    CY_BLE_TIME_ZONE_M0300, /* UTC-3:00 */
    CY_BLE_TIME_ZONE_M0200, /* UTC-2:00 */
    CY_BLE_TIME_ZONE_M0100, /* UTC-1:00 */
    CY_BLE_TIME_ZONE_ZERO,  /* UTC+0:00 */
    CY_BLE_TIME_ZONE_P0100, /* UTC+1:00 */
    CY_BLE_TIME_ZONE_P0200, /* UTC+2:00 */
    CY_BLE_TIME_ZONE_P0300, /* UTC+3:00 */
    CY_BLE_TIME_ZONE_P0330, /* UTC+3:30 */
    CY_BLE_TIME_ZONE_P0400, /* UTC+4:00 */
    CY_BLE_TIME_ZONE_P0430, /* UTC+4:30 */
    CY_BLE_TIME_ZONE_P0500, /* UTC+5:00 */
    CY_BLE_TIME_ZONE_P0530, /* UTC+5:30 */
    CY_BLE_TIME_ZONE_P0545, /* UTC+5:45 */
    CY_BLE_TIME_ZONE_P0600, /* UTC+6:00 */
    CY_BLE_TIME_ZONE_P0630, /* UTC+6:30 */
    CY_BLE_TIME_ZONE_P0700, /* UTC+7:00 */
    CY_BLE_TIME_ZONE_P0800, /* UTC+8:00 */
    CY_BLE_TIME_ZONE_P0845, /* UTC+8:45 */
    CY_BLE_TIME_ZONE_P0900, /* UTC+9:00 */
    CY_BLE_TIME_ZONE_P0930, /* UTC+9:30 */
    CY_BLE_TIME_ZONE_P1000, /* UTC+10:00 */
    CY_BLE_TIME_ZONE_P1030, /* UTC+10:30 */
    CY_BLE_TIME_ZONE_P1100, /* UTC+11:00 */
    CY_BLE_TIME_ZONE_P1130, /* UTC+11:30 */
    CY_BLE_TIME_ZONE_P1200, /* UTC+12:00 */
    CY_BLE_TIME_ZONE_P1245, /* UTC+12:45 */
    CY_BLE_TIME_ZONE_P1300, /* UTC+13:00 */
    CY_BLE_TIME_ZONE_P1400  /* UTC+14:00 */
};

static uint8_t recStatus[REC_NUM] = {REC_STATUS_OK, REC_STATUS_OK, REC_STATUS_OK};

static cy_stc_ble_cgms_cgmt_t cgmt[REC_NUM] =
{
    {   CY_BLE_CGMS_GLMT_FLG_TI | 
        CY_BLE_CGMS_GLMT_FLG_QA | 
        CY_BLE_CGMS_GLMT_FLG_WG | 
        CY_BLE_CGMS_GLMT_FLG_CT |
        CY_BLE_CGMS_GLMT_FLG_ST, /* flags */
        0xb032u /* 50 mg/dL */, /* CGM Glucose Concentration */
        1u, /* timeOffset */
        CY_BLE_CGMS_GLMT_SSA_BL | CY_BLE_CGMS_GLMT_SSA_CR | CY_BLE_CGMS_GLMT_SSA_RL, /* Sensor Status Annunciation */
        0u, /* CGM Trend Information */
        0u, /* CGM Quality */
    },
    {   CY_BLE_CGMS_GLMT_FLG_QA | 
        CY_BLE_CGMS_GLMT_FLG_WG | 
        CY_BLE_CGMS_GLMT_FLG_CT |
        CY_BLE_CGMS_GLMT_FLG_ST, /* flags */
        0xb032u /* 50 mg/dL */, /* CGM Glucose Concentration */
        2u, /* timeOffset */
        CY_BLE_CGMS_GLMT_SSA_BL | CY_BLE_CGMS_GLMT_SSA_CR | CY_BLE_CGMS_GLMT_SSA_RL, /* Sensor Status Annunciation */
        0u, /* CGM Trend Information */
        0u, /* CGM Quality */
    },
    {   CY_BLE_CGMS_GLMT_FLG_WG | 
        CY_BLE_CGMS_GLMT_FLG_CT |
        CY_BLE_CGMS_GLMT_FLG_ST, /* flags */
        0xb032u /* 50 mg/dL */, /* CGM Glucose Concentration */
        3u, /* timeOffset */
        CY_BLE_CGMS_GLMT_SSA_BL | CY_BLE_CGMS_GLMT_SSA_CR | CY_BLE_CGMS_GLMT_SSA_RL, /* Sensor Status Annunciation */
        0u, /* CGM Trend Information */
        0u, /* CGM Quality */
    }
};

void CgmsPrintCharName(cy_en_ble_cgms_char_index_t CgmsCharIndex)
{
    switch(CgmsCharIndex)
    {
        case CY_BLE_CGMS_CGMT:                    
            DBG_PRINTF("CGM Measurement ");
            break;
        
        case CY_BLE_CGMS_CGFT:                    
            DBG_PRINTF("CGM Feature ");
            break;
            
        case CY_BLE_CGMS_CGST:
            DBG_PRINTF("CGM Status ");
            break;
            
        case CY_BLE_CGMS_SSTM:
            DBG_PRINTF("CGM Session Start Time ");
            break;
            
        case CY_BLE_CGMS_SRTM:
            DBG_PRINTF("CGM Session Run Time ");
            break;
            
        case CY_BLE_CGMS_RACP:
            DBG_PRINTF("RACP ");
            break;
            
        case CY_BLE_CGMS_SOCP:
            DBG_PRINTF("SOCP ");
            break;
            
        default:
            DBG_PRINTF("Unknown CGM");
            break;
    }
    
    DBG_PRINTF("characteristic ");
}

/******************************************************************************
* Function Name: CgmsCrc
*******************************************************************************
*
* Summary:
*   Calculates a 16-bit CRC value with seed 0xFFFF and polynomial D16+D12+D5+1.
*
* Parameters:
*   length: The length of the data.
*   dataPtr: The pointer to the data.
*
* Return:
*   A uint16_t CRC value.
*
******************************************************************************/
uint16_t CgmsCrc(uint8_t length, uint8_t *dataPtr)
{
    uint8_t  bit, byte;
    uint16_t data, crc = CY_BLE_CGMS_CRC_SEED;
    
    for(byte = 0; byte < length; byte++)
    {
        data = (uint16_t) dataPtr[byte];
        for(bit = 0u; bit < 8u; bit++)
        {
            if(0u != ((crc ^ data) & 0x0001u))
            {
                crc = (crc >> 1u) ^ CY_BLE_CGMS_CRC_POLY;
            }
            else
            {
                crc >>= 1u;
            }
            
            data >>= 1u;
        }
    }

    return(crc);
}


/******************************************************************************
* Function Name: CgmsCrcLength
*******************************************************************************
*
* Summary:
*   Adds the CRC value to the end of the attribute payload 
*   and returns the actual length of the attribute
*   taking into account whether the CRC calculation feature is active.
*
*  Uses the above declared CgmsCrc();
*
* Parameters:
*   length: The length of the attribute payload without CRC.
*   dataPtr: The pointer to the attribute payload.
*
* Return:
*  A uint8_t length of the attribute, within CRC if the CRC feature is active.
*
******************************************************************************/
uint8_t CgmsCrcLength(uint8_t length, uint8_t* dataPtr)
{
    if(0u != (cgft.feature & CY_BLE_CGMS_CGFT_FTR_EC))
    {
        Cy_BLE_Set16ByPtr(&dataPtr[length], CgmsCrc(length, dataPtr));
        length += 2;
    }
    
    return(length);
}


/******************************************************************************
* Function Name: CgmsCrcCheck
*******************************************************************************
*
* Summary:
*   Checks the CRC value in the received attribute payload
*   taking into account whether the CRC calculation feature is active.
*
*   Uses the above declared CgmsCrc();
*
* Parameters:
*   attrSize:  The length of the attribute payload without CRC.
*   gattValue: The pointer to the GATT value.
*
* Return:
*   cy_en_ble_gatt_err_code_t result of the CRC checking if the CRC feature is active.
*   Following are the possible error codes:
*    CY_BLE_GATT_ERR_NONE               CRC is correct.
*    CY_BLE_GATT_ERR_INVALID_CRC        CRC doesn't match.
*    CY_BLE_GATT_ERR_MISSING_CRC        CRC is absent.
*
******************************************************************************/
cy_en_ble_gatt_err_code_t CgmsCrcCheck(uint16_t attrSize, cy_stc_ble_gatt_value_t *gattValue)
{
    cy_en_ble_gatt_err_code_t gattError = CY_BLE_GATT_ERR_NONE;
    
    if(0u != (CY_BLE_CGMS_CGFT_FTR_EC & cgft.feature))
    {
        DBG_PRINTF("Check CRC: ");
        
        if((attrSize + CY_BLE_CGMS_CRC_SIZE) == gattValue->len)
        {   
            if(Cy_BLE_Get16ByPtr(&gattValue->val[attrSize]) != 
                CgmsCrc(attrSize, gattValue->val))
            {
                gattError = CY_BLE_GATT_ERR_INVALID_CRC;
                DBG_PRINTF("invalid CRC\r\n");
            }
            else
            {
                DBG_PRINTF("ok\r\n");
            }
        }
        else
        {
            gattError = CY_BLE_GATT_ERR_MISSING_CRC;
            DBG_PRINTF("missing CRC\r\n");
        }
    }
    
    return(gattError);
}


/******************************************************************************
* Function Name: CgmsInit
*******************************************************************************
*
* Summary:
*   Does the initialization of the CGM Service.
*   Registers CGMS CallBack and reads the initial CGM feature characteristic.
*
******************************************************************************/
void CgmsInit(void)
{
    cy_en_ble_api_result_t apiResult;
    uint8_t acgft[6u];
    
    Cy_BLE_CGMS_RegisterAttrCallback(CgmsCallBack);
    
    apiResult = Cy_BLE_CGMSS_GetCharacteristicValue(CY_BLE_CGMS_CGFT, 6u, acgft);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_CGMSS_GetCharacteristicValue API Error %x: ", apiResult);
    }
    else
    {
        cgft.feature = (uint32) acgft[0];
        cgft.feature |= (uint32) (acgft[1] << 8u);
        cgft.feature |= (uint32) (acgft[2] << 16u);
        cgft.type = acgft[3] & CY_BLE_CGMS_CGFT_TYPE_MASK;
        cgft.sampLoc = (acgft[3] & CY_BLE_CGMS_CGFT_SL_MASK) >> CY_BLE_CGMS_CGFT_SLNUM;
    }
}


/******************************************************************************
* Function Name: CgmsCallBack
*******************************************************************************
*
* Summary:
*   A CallBack function to handle CGMS specific events.
*
******************************************************************************/
void CgmsCallBack(uint32_t event, void* eventParam)
{   
    cy_en_ble_api_result_t apiResult;
    uint32_t i;
    
    switch(event)
    {
        case CY_BLE_EVT_CGMSS_WRITE_CHAR:
            {
                uint16_t attrSize = 0u;
                cy_en_ble_gatt_err_code_t cgmsGattError = CY_BLE_GATT_ERR_NONE;
                
                CgmsPrintCharName(((cy_stc_ble_cgms_char_value_t *)eventParam)->charIndex);
                DBG_PRINTF("is written: ");
                for(i = 0; i < ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->len; i++)
                {
                    DBG_PRINTF("%2.2x ", ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[i]);
                }
                DBG_PRINTF("\r\n");
            
                switch(((cy_stc_ble_cgms_char_value_t *)eventParam)->charIndex)
                {
                    case CY_BLE_CGMS_SSTM:
                        cgmsGattError = CgmsCrcCheck(CY_BLE_CGMS_SSTM_SIZE, ((cy_stc_ble_cgms_char_value_t *)eventParam)->value);
                        
                        sstm.sst.year = Cy_BLE_Get16ByPtr(&((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[0u]);
                        sstm.sst.month = ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[2u];
                        sstm.sst.day = ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[3u];
                        sstm.sst.hours = ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[4u];
                        sstm.sst.minutes = ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[5u];
                        sstm.sst.seconds = ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[6u];
                        sstm.timeZone = (cy_en_ble_time_zone_t)((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[7u];
                        sstm.dstOffset = (cy_en_ble_dstoffset_t)((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[8u];
                        
                        cgmsGattError = CY_BLE_GATT_ERR_INVALID_PDU;
                        for(i = 0; i < CY_BLE_TIME_ZONE_VAL_NUM; i++)
                        {
                            if(timeZone[i] == sstm.timeZone)
                            {
                                cgmsGattError = CY_BLE_GATT_ERR_NONE;
                            }
                        }
                        
                        if(cgmsGattError != CY_BLE_GATT_ERR_NONE)
                        {
                            DBG_PRINTF("invalid PDU\r\n");   
                        }
                        break;
                    
                    case CY_BLE_CGMS_RACP:                    
                        cgmsFlag |= CGMS_FLAG_RACP;
                        
                        DBG_PRINTF("Opcode: ");
                        racpOpCode = ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[0];
                        switch(racpOpCode)
                        {
                            case CY_BLE_CGMS_RACP_OPC_REPORT_REC:
                                DBG_PRINTF("Report stored records \r\n");
                                break;
                                
                            case CY_BLE_CGMS_RACP_OPC_DELETE_REC:
                                DBG_PRINTF("Delete stored records \r\n");
                                break;
                                
                            case CY_BLE_CGMS_RACP_OPC_ABORT_OPN:
                                DBG_PRINTF("Abort operation \r\n");
                                break;
                                
                            case CY_BLE_CGMS_RACP_OPC_REPORT_NUM_REC:
                                DBG_PRINTF("Report number of stored records \r\n");
                                break;
                                
                            default:
                                DBG_PRINTF("Unknown \r\n");
                                break;
                        }
                        
                        DBG_PRINTF("Operator: ");
                        racpOperator = ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[1];
                        attr[3u] = CY_BLE_CGMS_RACP_RSP_SUCCESS;
                        switch(racpOperator)
                        {
                            case CY_BLE_CGMS_RACP_OPR_NULL:
                                DBG_PRINTF("Null \r\n");
                                break;
                            
                            case CY_BLE_CGMS_RACP_OPR_LAST:
                                DBG_PRINTF("Last record \r\n");
                                break;
                                
                            case CY_BLE_CGMS_RACP_OPR_FIRST:
                                DBG_PRINTF("First record \r\n");
                                break;
                                
                            case CY_BLE_CGMS_RACP_OPR_ALL:
                                DBG_PRINTF("All records \r\n");
                                if(((cy_stc_ble_cgms_char_value_t *)eventParam)->value->len > 2u)
                                {
                                    attr[3u] = CY_BLE_CGMS_RACP_RSP_INV_OPD;
                                }
                                break;
                            
                            case CY_BLE_CGMS_RACP_OPR_LESS:
                                DBG_PRINTF("Less than or equal to \r\n");
                                DBG_PRINTF("Operand: ");
                                if(((cy_stc_ble_cgms_char_value_t *)eventParam)->value->len == 5u)
                                {
                                    racpOperand[0] = ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[2];
                                    if(CY_BLE_CGMS_RACP_OPD_1 == racpOperand[0u])
                                    {
                                        racpOperand[1] = ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[3];
                                        DBG_PRINTF("Time Offset: %x \r\n", racpOperand[1u]);
                                    }
                                    else
                                    {
                                        DBG_PRINTF("Unknown \r\n");
                                    }
                                }
                                else
                                {
                                    attr[3u] = CY_BLE_CGMS_RACP_RSP_INV_OPD;
                                    DBG_PRINTF("Invalid \r\n");
                                }
                                break;
                                
                            case CY_BLE_CGMS_RACP_OPR_GREAT:
                                DBG_PRINTF("Greater than or equal to \r\n");
                                DBG_PRINTF("Operand: ");
                                if(((cy_stc_ble_cgms_char_value_t *)eventParam)->value->len == 5u)
                                {
                                    racpOperand[0u] = ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[2u];
                                    if(CY_BLE_CGMS_RACP_OPD_1 == racpOperand[0u])
                                    {
                                        racpOperand[1u] = ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[3u];
                                        DBG_PRINTF("Time Offset: %x \r\n", racpOperand[1u]);
                                    }
                                    else
                                    {
                                        DBG_PRINTF("Unknown \r\n");
                                    }
                                }
                                else
                                {
                                    attr[3u] = CY_BLE_CGMS_RACP_RSP_INV_OPD;
                                    DBG_PRINTF("Invalid \r\n");
                                }
                                break;
                            
                            case CY_BLE_CGMS_RACP_OPR_WITHIN:
                                DBG_PRINTF("Within range of (inclusive) \r\n");
                                DBG_PRINTF("Operand: ");
                                if(((cy_stc_ble_cgms_char_value_t *)eventParam)->value->len == 7u)
                                {
                                    racpOperand[0] = ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[2u];
                                    if(CY_BLE_CGMS_RACP_OPD_1 == racpOperand[0u])
                                    {
                                        racpOperand[1] = ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[3u];
                                        racpOperand[2] = ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[5u];
                                        DBG_PRINTF("Time Offsets: %x, %x \r\n", racpOperand[1u], racpOperand[2u]);
                                    }
                                    else
                                    {
                                        DBG_PRINTF("Unknown \r\n");
                                    }
                                }
                                else
                                {
                                    attr[3u] = CY_BLE_CGMS_RACP_RSP_INV_OPD;
                                    DBG_PRINTF("Invalid \r\n");
                                }
                                break;
                        
                            default:
                                DBG_PRINTF("Unknown \r\n");
                                break;
                        }
                        break;
                        
                    case CY_BLE_CGMS_SOCP:
                        
                        socpOpCode = (cy_en_ble_cgms_socp_opc_t)((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[0];
                        socp[2u] = CY_BLE_CGMS_SOCP_RSP_SUCCESS;
                        
                        switch(socpOpCode)
                        {
                            case CY_BLE_CGMS_SOCP_OPC_STSN:
                            case CY_BLE_CGMS_SOCP_OPC_SPSN:
                                attrSize = CY_BLE_CGMS_SOCP_OPC_SN;
                                break;
                            
                            case CY_BLE_CGMS_SOCP_OPC_SINT:
                                attrSize = CY_BLE_CGMS_SOCP_OPC_IN;
                                break;
                            
                            case CY_BLE_CGMS_SOCP_OPC_SHAL:
                            case CY_BLE_CGMS_SOCP_OPC_SLAL:
                            case CY_BLE_CGMS_SOCP_OPC_SHPO:
                            case CY_BLE_CGMS_SOCP_OPC_SHPR:
                            case CY_BLE_CGMS_SOCP_OPC_SDEC:
                            case CY_BLE_CGMS_SOCP_OPC_SINC:
                                attrSize = CY_BLE_CGMS_SOCP_OPC_AL;
                                break;
                            
                            case CY_BLE_CGMS_SOCP_OPC_SGCV:
                                attrSize = CY_BLE_CGMS_SOCP_OPC_CV;
                                break;
                            
                            default:
                                socp[2u] = CY_BLE_CGMS_SOCP_RSP_UNSPRT_OPC;
                                break;
                        }
                        
                        if(socp[2u] == CY_BLE_CGMS_SOCP_RSP_SUCCESS)
                        {
                            cgmsGattError = CgmsCrcCheck(attrSize, ((cy_stc_ble_cgms_char_value_t *)eventParam)->value);
                        }
                                                
                        if(cgmsGattError == CY_BLE_GATT_ERR_NONE)
                        {
                            cgmsFlag |= CGMS_FLAG_SOCP;
                            DBG_PRINTF("Opcode: ");
                            
                            switch(socpOpCode)
                            {
                                case CY_BLE_CGMS_SOCP_OPC_GINT:
                                    DBG_PRINTF("Get CGM Communication Interval\r\n");
                                    DBG_PRINTF("Response: Communication Interval: 0x%2.2x\r\n", commInterval);
                                    socp[0u] = CY_BLE_CGMS_SOCP_OPC_RINT;
                                    socp[1u] = commInterval;
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_SINT:
                                    DBG_PRINTF("Set CGM Communication Interval\r\n");
                                    commInterval = ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[1];
                                    DBG_PRINTF("Operand: 0x%2.2x \r\n", commInterval);
                                    socp[1u] = CY_BLE_CGMS_SOCP_OPC_SINT;
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_SGCV:
                                    DBG_PRINTF("Set Glucose Calibration Value\r\n"); 
                                    socp[1u] = CY_BLE_CGMS_SOCP_OPC_SGCV;
                                    DBG_PRINTF("Operand:\r\n Glucose Concentration 0x%4.4x\r\n",
                                        Cy_BLE_Get16ByPtr(&((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[1]));
                                    DBG_PRINTF(" Calibration Time 0x%4.4x\r\n",
                                        Cy_BLE_Get16ByPtr(&((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[3]));
                                    DBG_PRINTF(" Sample Location 0x%2.2x\r\n", ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[5]);
                                    DBG_PRINTF(" Next Calibration 0x%4.4x\r\n",
                                        Cy_BLE_Get16ByPtr(&((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[6]));
                                    DBG_PRINTF(" Calibration Data Record Number 0x%4.4x\r\n",
                                        Cy_BLE_Get16ByPtr(&((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[8]));
                                    DBG_PRINTF(" Calibration Status 0x%2.2x\r\n", ((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[10]);
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_GGCV:
                                    DBG_PRINTF("Get Glucose Calibration Value\r\n");
                                    DBG_PRINTF("Operand: Calibration Data Record Number: 0x%4.4x \r\n",
                                            Cy_BLE_Get16ByPtr(&((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[1]));
                                    if(0xfffeu == Cy_BLE_Get16ByPtr(&((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[1]))
                                    {
                                        socp[0u] = CY_BLE_CGMS_SOCP_OPC_RSPC;
                                        socp[1u] = CY_BLE_CGMS_SOCP_OPC_GGCV;
                                        socp[2u] = CY_BLE_CGMS_SOCP_RSP_POOR;
                                        socpLength = 3u;
                                    }
                                    else
                                    {
                                    
                                        if(0xffffu == Cy_BLE_Get16ByPtr(&((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[1]))
                                        {
                                            socpRecNum++;
                                        }
                                        else
                                        {
                                            socpRecNum = Cy_BLE_Get16ByPtr(&((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[1]);
                                        }
                                        
                                        socp[0u] = CY_BLE_CGMS_SOCP_OPC_RGCV;
                                        DBG_PRINTF("Response:\r\n Glucose Concentration 0x004e(78mg/dL)\r\n");
                                        Cy_BLE_Set16ByPtr(&socp[1u], 0x004eu);
                                        DBG_PRINTF(" Calibration Time 0x0005 (5 minutes)\r\n");
                                        Cy_BLE_Set16ByPtr(&socp[3u], 0x0005u);
                                        DBG_PRINTF(" Sample Location 0x06\r\n");
                                        socp[5] = 0x06u;
                                        DBG_PRINTF(" Next Calibration 0x0005 (5 minutes)\r\n");
                                        Cy_BLE_Set16ByPtr(&socp[6u], 0x0005u);
                                        DBG_PRINTF(" Calibration Data Record Number 0x%4.4x \r\n", socpRecNum);
                                        Cy_BLE_Set16ByPtr(&socp[8u], socpRecNum);
                                        DBG_PRINTF(" Calibration Status 0x00\r\n");
                                        socp[10u] = 0x00u;
                                        socpLength = 11u;
                                    }
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_GHAL:
                                    DBG_PRINTF("Get Patient High Alert Level\r\n"); 
                                    DBG_PRINTF("Response: Patient High Alert Level: 0x%4.4x\r\n", alrt.hal);
                                    socp[0u] = CY_BLE_CGMS_SOCP_OPC_RHAL;
                                    Cy_BLE_Set16ByPtr(&socp[1u], alrt.hal);
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_SHAL:
                                    DBG_PRINTF("Set Patient High Alert Level\r\n");
                                    socpOpd = Cy_BLE_Get16ByPtr(&((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[1]);
                                    socpOpdPtr = &alrt.hal;
                                    DBG_PRINTF("Operand: Patient High Alert Level: 0x%4.4x \r\n", socpOpd);
                                    socp[1u] = CY_BLE_CGMS_SOCP_OPC_SHAL;
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_GLAL:
                                    DBG_PRINTF("Get Patient Low Alert Level\r\n"); 
                                    DBG_PRINTF("Response: Patient High Alert Level: 0x%4.4x\r\n", alrt.lal);
                                    socp[0u] = CY_BLE_CGMS_SOCP_OPC_RLAL;
                                    Cy_BLE_Set16ByPtr(&socp[1u], alrt.lal);
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_SLAL:
                                    DBG_PRINTF("Set Patient Low Alert Level\r\n");
                                    socpOpd = Cy_BLE_Get16ByPtr(&((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[1]);
                                    socpOpdPtr = &alrt.lal;
                                    DBG_PRINTF("Operand: Patient Low Alert Level: 0x%4.4x \r\n", socpOpd);
                                    socp[1u] = CY_BLE_CGMS_SOCP_OPC_SLAL;
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_GHPO:
                                    DBG_PRINTF("Get Hypo Alert Level\r\n"); 
                                    DBG_PRINTF("Response: Hypo Alert Level: 0x%4.4x\r\n", alrt.hpo);
                                    socp[0u] = CY_BLE_CGMS_SOCP_OPC_RHPO;
                                    Cy_BLE_Set16ByPtr(&socp[1u], alrt.hpo);
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_SHPO:
                                    DBG_PRINTF("Set Hypo Alert Level\r\n");
                                    socpOpd = Cy_BLE_Get16ByPtr(&((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[1]);
                                    socpOpdPtr = &alrt.hpo;
                                    DBG_PRINTF("Operand: Hypo Alert Level: 0x%4.4x \r\n", socpOpd);
                                    socp[1u] = CY_BLE_CGMS_SOCP_OPC_SHPO;
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_GHPR:
                                    DBG_PRINTF("Get Hyper Alert Level\r\n"); 
                                    DBG_PRINTF("Response: Hyper Alert Level: 0x%4.4x\r\n", alrt.hpr);
                                    socp[0u] = CY_BLE_CGMS_SOCP_OPC_RHPR;
                                    Cy_BLE_Set16ByPtr(&socp[1u], alrt.hpr);
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_SHPR:
                                    DBG_PRINTF("Set Hyper Alert Level\r\n");
                                    socpOpd = Cy_BLE_Get16ByPtr(&((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[1]);
                                    socpOpdPtr = &alrt.hpr;
                                    DBG_PRINTF("Operand: Hyper Alert Level: 0x%4.4x \r\n", socpOpd);
                                    socp[1u] = CY_BLE_CGMS_SOCP_OPC_SHPR;
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_GDEC:
                                    DBG_PRINTF("Get Rate of Decrease Alert Level\r\n"); 
                                    DBG_PRINTF("Response: Rate of Decrease Alert Level: 0x%4.4x\r\n", alrt.dec);
                                    socp[0u] = CY_BLE_CGMS_SOCP_OPC_RDEC;
                                    Cy_BLE_Set16ByPtr(&socp[1u], alrt.dec);
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_SDEC:
                                    DBG_PRINTF("Set Rate of Decrease Alert Level\r\n");
                                    socpOpd = Cy_BLE_Get16ByPtr(&((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[1]);
                                    socpOpdPtr = &alrt.dec;
                                    DBG_PRINTF("Operand: Rate of Decrease Alert Level: 0x%4.4x \r\n", socpOpd);
                                    socp[1u] = CY_BLE_CGMS_SOCP_OPC_SDEC;
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_GINC:
                                    DBG_PRINTF("Get Rate of Increase Alert Level\r\n"); 
                                    DBG_PRINTF("Response: Rate of Increase Alert Level: 0x%4.4x\r\n", alrt.inc);
                                    socp[0u] = CY_BLE_CGMS_SOCP_OPC_RINC;
                                    Cy_BLE_Set16ByPtr(&socp[1u], alrt.inc);
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_SINC:
                                    DBG_PRINTF("Set Rate of Increase Alert Level\r\n");
                                    socpOpd = Cy_BLE_Get16ByPtr(&((cy_stc_ble_cgms_char_value_t *)eventParam)->value->val[1]);
                                    socpOpdPtr = &alrt.inc;
                                    DBG_PRINTF("Operand: Rate of Increase Alert Level: 0x%4.4x \r\n", socpOpd);
                                    socp[1u] = CY_BLE_CGMS_SOCP_OPC_SINC;
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_RDSA:
                                    DBG_PRINTF("Reset Device Specific Alert\r\n"); 
                                    socp[1u] = CY_BLE_CGMS_SOCP_OPC_RDSA;
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_STSN:
                                    DBG_PRINTF("Start the Session\r\n");
                                    apiResult = Cy_BLE_CGMSS_GetCharacteristicValue(CY_BLE_CGMS_CGST, 7u, attr);
                                    if(apiResult != CY_BLE_SUCCESS)
                                    {
                                        DBG_PRINTF("Cy_BLE_CGMSS_GetCharacteristicValue API Error: 0x%x", apiResult);
                                    }
                                    
                                    if(0u != (attr[2] & (uint8_t) CY_BLE_CGMS_GLMT_SSA_SS))
                                    {
                                        attr[2u] &= (uint8_t) ~CY_BLE_CGMS_GLMT_SSA_SS;
                                    
                                        apiResult = Cy_BLE_CGMSS_SetCharacteristicValue(CY_BLE_CGMS_CGST, CgmsCrcLength(5u, attr), attr);
                                        if(apiResult != CY_BLE_SUCCESS)
                                        {
                                            DBG_PRINTF("Cy_BLE_CGMSS_GetCharacteristicValue API Error: 0x%x", apiResult);
                                        }
                                    }
                                    else
                                    {
                                        socp[2u] = CY_BLE_CGMS_SOCP_RSP_NO_COMPL;
                                    }
                                    socp[1u] = CY_BLE_CGMS_SOCP_OPC_STSN;
                                    break;
                                    
                                case CY_BLE_CGMS_SOCP_OPC_SPSN:
                                    DBG_PRINTF("Stop the Session\r\n");
                                    apiResult = Cy_BLE_CGMSS_GetCharacteristicValue(CY_BLE_CGMS_CGST, 7u, attr);
                                    if(apiResult != CY_BLE_SUCCESS)
                                    {
                                        DBG_PRINTF("Cy_BLE_CGMSS_GetCharacteristicValue API Error: 0x%x", apiResult);
                                    }
                                    
                                    attr[2u] |= (uint8_t) CY_BLE_CGMS_GLMT_SSA_SS;
                                    
                                    apiResult = Cy_BLE_CGMSS_SetCharacteristicValue(CY_BLE_CGMS_CGST, CgmsCrcLength(5u, attr), attr);
                                    if(apiResult != CY_BLE_SUCCESS)
                                    {
                                        DBG_PRINTF("Cy_BLE_CGMSS_GetCharacteristicValue API Error: 0x%x", apiResult);
                                    }
                                    
                                    socp[1u] = CY_BLE_CGMS_SOCP_OPC_SPSN;
                                    break;
                                    
                                default:
                                    DBG_PRINTF("Not Supported \r\n");
                                    break;
                            }
                        }
                        break;
                        
                    default:
                        break;
                }
                
                ((cy_stc_ble_cgms_char_value_t*)eventParam)->gattErrorCode = cgmsGattError;
            }
            break;
        
        case CY_BLE_EVT_CGMSS_NOTIFICATION_ENABLED:
            DBG_PRINTF("Glucose Notification is Enabled \r\n");
            break;
                
        case CY_BLE_EVT_CGMSS_NOTIFICATION_DISABLED:
            DBG_PRINTF("Glucose Notification is Disabled \r\n");
            break;
            
        case CY_BLE_EVT_CGMSS_INDICATION_ENABLED:
            DBG_PRINTF("RACP Indication is Enabled \r\n");
            break;
                
        case CY_BLE_EVT_CGMSS_INDICATION_DISABLED:
            DBG_PRINTF("RACP Indication is Disabled \r\n");
            break;
            
        case CY_BLE_EVT_CGMSS_INDICATION_CONFIRMED:
            DBG_PRINTF("RACP Indication is Confirmed \r\n");
            break;

        default:
            DBG_PRINTF("Unknown CGMS event: %lx \r\n", event);
            break;
    }
}


/******************************************************************************
* Function Name: CgmsSendCgmtNtf
*******************************************************************************
*
* Summary:
*   Packs the payload and sends a notification
*   of the CGM Measurement characteristic.
*
*   Uses the above declared CgmsCrc();
*
* Parameters:
*   cgmt - The CGM Measurement characteristic value structure.
*
******************************************************************************/
void CgmsSendCgmtNtf(cy_stc_ble_conn_handle_t connHandle, cy_stc_ble_cgms_cgmt_t cgmt)
{
    cy_en_ble_api_result_t apiResult;
    uint8_t pdu[sizeof(cy_stc_ble_cgms_cgmt_t)];
    uint8_t ptr, b;
    uint16_t cccd;
    
    /* "Flags" octet goes second */
    pdu[1u] = cgmt.flags;
    
    /* CGM Glucose Concentration is third and forth bytes */
    Cy_BLE_Set16ByPtr(&pdu[2u], cgmt.gluConc);
    
    /* Time Offset consumes next two bytes */
    Cy_BLE_Set16ByPtr(&pdu[4u], cgmt.timeOffset);
    
    ptr = 6u; /* size of size + flags + gluConc + timeOffset) */
        
    /* if "Warning-Octet present" flag is set */
    if(0u != (cgmt.flags & CY_BLE_CGMS_GLMT_FLG_WG))
    {
        /* set the next byte of CGM Measurement characteristic value */
        pdu[ptr] = cgmt.ssa & CY_BLE_CGMS_GLMT_SSA_WGM;
        ptr++;
    }
    
    if(0u != (cgmt.flags & CY_BLE_CGMS_GLMT_FLG_CT))
    {
        pdu[ptr] = (cgmt.ssa & CY_BLE_CGMS_GLMT_SSA_CTM) >> CY_BLE_CGMS_GLMT_SSA_CTS;
        ptr++;
    }
    
    if(0u != (cgmt.flags & CY_BLE_CGMS_GLMT_FLG_ST))
    {
        pdu[ptr] = (cgmt.ssa & CY_BLE_CGMS_GLMT_SSA_STM) >> CY_BLE_CGMS_GLMT_SSA_STS;
        ptr++;
    }
    
    if((0u != (cgft.feature & CY_BLE_CGMS_CGFT_FTR_TI)) && 
            (0u != (cgmt.flags & CY_BLE_CGMS_GLMT_FLG_TI)))
    {
        Cy_BLE_Set16ByPtr(&pdu[ptr], cgmt.trend);
        ptr += 2;
    }
    
    if((0u != (cgft.feature & CY_BLE_CGMS_CGFT_FTR_QA)) && 
            (0u != (cgmt.flags & CY_BLE_CGMS_GLMT_FLG_QA)))
    {
        Cy_BLE_Set16ByPtr(&pdu[ptr], cgmt.quality);
        ptr += 2;
    }
    
    if(0u != (cgft.feature & CY_BLE_CGMS_CGFT_FTR_EC))
    {
        /* Size includes crc field */
        pdu[0u] = ptr + 2;
        Cy_BLE_Set16ByPtr(&pdu[ptr], CgmsCrc(ptr, pdu));
        ptr = pdu[0u];
    }
    else
    {
        pdu[0u] = ptr;
    }
       
    apiResult = Cy_BLE_CGMSS_GetCharacteristicDescriptor(connHandle, CY_BLE_CGMS_CGMT, CY_BLE_CGMS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_CGMSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
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
            apiResult = Cy_BLE_CGMSS_SendNotification(connHandle, CY_BLE_CGMS_CGMT, ptr, pdu);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("CgmsSendCgmtNtf API Error: 0x%x", apiResult);     
            }
            else
            {
                DBG_PRINTF("Cgmt Ntf: ");
                for(b = 0; b < ptr; b++)
                {
                    DBG_PRINTF("%2.2x ", pdu[b]);
                }
                DBG_PRINTF("\r\n");
            }
        }
    }
}


/******************************************************************************
* Function Name: CgmsRacpOpCodeProcess
*******************************************************************************
*
* Summary:
*   Processes the CGM record depending on RACP OpCode.
*
* Parameters:
*   recNum: the number of the CGM record.
*
******************************************************************************/
void CgmsRacpOpCodeProcess(cy_stc_ble_conn_handle_t connHandle, uint8_t recNum)
{
    attr[3u] = CY_BLE_CGMS_RACP_RSP_SUCCESS;
    
    switch(racpOpCode)
    {
        case CY_BLE_CGMS_RACP_OPC_REPORT_REC:
            CgmsSendCgmtNtf(connHandle, cgmt[recNum]);
            break;
            
        case CY_BLE_CGMS_RACP_OPC_REPORT_NUM_REC:
            recCnt++;
            break;
            
        case CY_BLE_CGMS_RACP_OPC_DELETE_REC:
            recStatus[recNum] = REC_STATUS_DELETED;
            break;
            
        default:
            attr[3u] = CY_BLE_CGMS_RACP_RSP_UNSPRT_OPC;
            break;
    }
}


/******************************************************************************
* Function Name: CgmsProcess
*******************************************************************************
*
* Summary:
*   Processes the CGM control points requests.
*
******************************************************************************/
void CgmsProcess(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t i;
    uint32_t recNum;
    uint16_t cccd;
            
    if((cgmsFlag & CGMS_FLAG_RACP) != 0u)
    {
        switch(racpOperator)
        {
            case CY_BLE_CGMS_RACP_OPR_NULL:
                switch(racpOpCode)
                {
                    case CY_BLE_CGMS_RACP_OPC_REPORT_REC:
                    case CY_BLE_CGMS_RACP_OPC_DELETE_REC:
                    case CY_BLE_CGMS_RACP_OPC_REPORT_NUM_REC:
                        attr[3u] = CY_BLE_CGMS_RACP_RSP_INV_OPR;
                        break;
                        
                    default:
                        break;
                }
                break;
            
            case CY_BLE_CGMS_RACP_OPR_LAST:
                if(CY_BLE_CGMS_RACP_RSP_SUCCESS == attr[3u])
                {
                    CgmsRacpOpCodeProcess(connHandle, REC_NUM - 1u);
                }
                break;
                
            case CY_BLE_CGMS_RACP_OPR_FIRST:
                if(CY_BLE_CGMS_RACP_RSP_SUCCESS == attr[3u])
                {
                    CgmsRacpOpCodeProcess(connHandle, 0u);
                }
                break;
                
            case CY_BLE_CGMS_RACP_OPR_ALL:
                if(CY_BLE_CGMS_RACP_RSP_SUCCESS == attr[3u])
                {
                    attr[3u] = CY_BLE_CGMS_RACP_RSP_NO_REC;
                    recCnt = 0u;
                    for(recNum = 0u; recNum < REC_NUM; recNum++)
                    {  
                        if(REC_STATUS_OK == recStatus[recNum])
                        {
                            CgmsRacpOpCodeProcess(connHandle, recNum);
                        } 
                    }
                }
                break;
                
            case CY_BLE_CGMS_RACP_OPR_LESS:
                if(CY_BLE_CGMS_RACP_RSP_SUCCESS == attr[3u])
                {
                    attr[3] = CY_BLE_CGMS_RACP_RSP_NO_REC;
                    
                    if(CY_BLE_CGMS_RACP_OPD_1 == racpOperand[0u])
                    {
                        recCnt = 0u;
                        for(recNum = 0; recNum < REC_NUM; recNum++)
                        {
                            if((cgmt[recNum].timeOffset <= racpOperand[1u]) && (REC_STATUS_OK == recStatus[recNum]))
                            {
                                CgmsRacpOpCodeProcess(connHandle, recNum);
                            }    
                        }
                    }
                    else
                    {
                        attr[3u] = CY_BLE_CGMS_RACP_RSP_UNSPRT_OPD;
                    }
                }
                break;
                
            case CY_BLE_CGMS_RACP_OPR_GREAT:
                if(CY_BLE_CGMS_RACP_RSP_SUCCESS == attr[3u])
                {
                    attr[3u] = CY_BLE_CGMS_RACP_RSP_NO_REC;
                    
                    if(CY_BLE_CGMS_RACP_OPD_1 == racpOperand[0u])
                    {
                        recCnt = 0u;
                        for(recNum = 0; recNum < REC_NUM; recNum++)
                        {
                            if((cgmt[recNum].timeOffset >= racpOperand[1u]) && (REC_STATUS_OK == recStatus[recNum]))
                            {
                                CgmsRacpOpCodeProcess(connHandle, recNum);
                            }    
                        }
                    }
                    else
                    {
                        attr[3u] = CY_BLE_CGMS_RACP_RSP_UNSPRT_OPD;
                    }
                }
                break;
                
            case CY_BLE_CGMS_RACP_OPR_WITHIN:
                if(CY_BLE_CGMS_RACP_RSP_SUCCESS == attr[3u])
                {
                    attr[3u] = CY_BLE_CGMS_RACP_RSP_NO_REC;
                    
                    if(CY_BLE_CGMS_RACP_OPD_1 == racpOperand[0u])
                    {
                        if(racpOperand[1] > racpOperand[2u])
                        {
                            attr[3u] = CY_BLE_CGMS_RACP_RSP_INV_OPD;   
                        }
                        else
                        {
                            for(recNum = 0; recNum < REC_NUM; recNum++)
                            {
                                if((cgmt[recNum].timeOffset >= racpOperand[1u]) &&
                                    (cgmt[recNum].timeOffset <= racpOperand[2u]) &&
                                        (REC_STATUS_OK == recStatus[recNum]))
                                {
                                    CgmsRacpOpCodeProcess(connHandle, recNum);    
                                }
                            }
                        }
                    }
                    else
                    {
                        attr[3u] = CY_BLE_CGMS_RACP_RSP_UNSPRT_OPD;
                    }
                }
                break;
                
            default:
                attr[3u] = CY_BLE_CGMS_RACP_RSP_UNSPRT_OPR;
                break;
        }
        
        attr[1u] = CY_BLE_CGMS_RACP_OPR_NULL;
        length = 4u;
        
        if(CY_BLE_CGMS_RACP_OPC_REPORT_NUM_REC == racpOpCode)
        {
            attr[0u] = CY_BLE_CGMS_RACP_OPC_NUM_REC_RSP;
            attr[2u] = recCnt;
            attr[3u] = 0u;
        }
        else
        {
            attr[0u] = CY_BLE_CGMS_RACP_OPC_RSP_CODE;
            attr[2u] = racpOpCode;
        }
        
        apiResult = Cy_BLE_CGMSS_GetCharacteristicDescriptor(connHandle, CY_BLE_CGMS_RACP, CY_BLE_CGMS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
              
        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("Cy_BLE_CGMSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
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
                apiResult = Cy_BLE_CGMSS_SendIndication(connHandle, CY_BLE_CGMS_RACP, length, attr);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_CGMSS_SendIndication API Error: 0x%x", apiResult);
                }
                else
                {
                    DBG_PRINTF("RACP Ind: ");
                    for(i = 0; i < length; i++)
                    {
                        DBG_PRINTF("%2.2x ", attr[i]);
                    }
                    DBG_PRINTF("\r\n");
                }
            }
        }
        
        cgmsFlag &= (uint8_t) ~CGMS_FLAG_RACP;
    }
    
    if(0u != (cgmsFlag & CGMS_FLAG_SOCP))
    {
        switch(socpOpCode)
        {
            case CY_BLE_CGMS_SOCP_OPC_GINT:
                socpLength = 2u;
                break;
            
            case CY_BLE_CGMS_SOCP_OPC_RDSA:
            case CY_BLE_CGMS_SOCP_OPC_SPSN:
                socpOpd = 0u;    
                socpOpdPtr = &socpOpd;
            case CY_BLE_CGMS_SOCP_OPC_SINT:
            case CY_BLE_CGMS_SOCP_OPC_SGCV:
            case CY_BLE_CGMS_SOCP_OPC_SHAL:
            case CY_BLE_CGMS_SOCP_OPC_SLAL:
            case CY_BLE_CGMS_SOCP_OPC_SHPO:
            case CY_BLE_CGMS_SOCP_OPC_SHPR:
            case CY_BLE_CGMS_SOCP_OPC_SDEC:
            case CY_BLE_CGMS_SOCP_OPC_SINC:
                
                switch(socpOpd)
                {
                    case SFLOAT_NAN:
                    case SFLOAT_NRES:
                    case SFLOAT_PINF:
                    case SFLOAT_NINF:
                    case SFLOAT_RSRV:
                        socp[2u] = CY_BLE_CGMS_SOCP_RSP_POOR;
                        break;
                        
                    default:
                        *socpOpdPtr = socpOpd;
                        break;
                }
            case CY_BLE_CGMS_SOCP_OPC_STSN:
                socp[0u] = CY_BLE_CGMS_SOCP_OPC_RSPC;
            case CY_BLE_CGMS_SOCP_OPC_GHAL:
            case CY_BLE_CGMS_SOCP_OPC_GLAL:
            case CY_BLE_CGMS_SOCP_OPC_GHPO:
            case CY_BLE_CGMS_SOCP_OPC_GHPR:
            case CY_BLE_CGMS_SOCP_OPC_GDEC:
            case CY_BLE_CGMS_SOCP_OPC_GINC:
                socpLength = 3u;
                break;
            
            default:
                break;
        }
        
        apiResult = Cy_BLE_CGMSS_GetCharacteristicDescriptor(connHandle, CY_BLE_CGMS_SOCP, CY_BLE_CGMS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
              
        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("Cy_BLE_CGMSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
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
                socpLength = CgmsCrcLength(socpLength, socp);
                apiResult = Cy_BLE_CGMSS_SendIndication(connHandle, CY_BLE_CGMS_SOCP, socpLength, socp);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_CGMSS_SendIndication API Error: 0x%x", apiResult);
                }
                else
                {
                    DBG_PRINTF("SOCP Ind: ");
                    for(i = 0; i < socpLength; i++)
                    {
                        DBG_PRINTF("%2.2x ", socp[i]);
                    }
                    DBG_PRINTF("\r\n");
                }
            }
        }    
        
        cgmsFlag &= (uint8_t) ~CGMS_FLAG_SOCP;
    }
}


/* [] END OF FILE */
