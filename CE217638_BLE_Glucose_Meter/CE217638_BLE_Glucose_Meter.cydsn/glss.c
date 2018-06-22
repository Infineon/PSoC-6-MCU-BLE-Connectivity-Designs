/*******************************************************************************
* File Name: glss.c
*
* Version 1.0
*
* Description:
*  This file contains GLS service related code.
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
#include "glss.h"

/* Global variables */
uint8_t racpCommand  = 0u;
uint8_t racpOpCode   = 0u;
uint8_t racpOperator = 0u;
uint8_t racpFilterType;
uint16_t seqNum1;
uint16_t seqNum2;
cy_stc_ble_date_time_t userFacingTime1;
cy_stc_ble_date_time_t userFacingTime2;
uint8_t racpInd[4u];
uint8_t recCnt = 1u;


/* Record Status array */
uint8_t recStat[CY_BLE_GLS_REC_NUM] =
{
    CY_BLE_GLS_REC_STAT_OK,
    CY_BLE_GLS_REC_STAT_OK,
    CY_BLE_GLS_REC_STAT_OK,
    CY_BLE_GLS_REC_STAT_OK,
    CY_BLE_GLS_REC_STAT_OK,
    CY_BLE_GLS_REC_STAT_OK,
    CY_BLE_GLS_REC_STAT_OK,
    CY_BLE_GLS_REC_STAT_OK,
    CY_BLE_GLS_REC_STAT_OK,
    CY_BLE_GLS_REC_STAT_OK,
    CY_BLE_GLS_REC_STAT_OK
};


/* Glucose Measurement records */
cy_stc_ble_gls_glmt_t glsGlucose[CY_BLE_GLS_REC_NUM] =
{
    {CY_BLE_GLS_GLMT_FLG_TOP | CY_BLE_GLS_GLMT_FLG_SSA,
        0u, {2014u, 7u, 27, 20u, 30u, 40u}, 0, 0xb032u /* 50 mg/dL */,
        (CY_BLE_GLS_GLMT_TYPE_CWB | (CY_BLE_GLS_GLMT_SL_FR << CY_BLE_GLS_GLMT_SLNUM)), CY_BLE_GLS_GLMT_SSA_BTL},
    {CY_BLE_GLS_GLMT_FLG_TOP | CY_BLE_GLS_GLMT_FLG_GLC | CY_BLE_GLS_GLMT_FLG_GCU | CY_BLE_GLS_GLMT_FLG_CIF,
        1u, {2014u, 7u, 27, 20u, 30u, 40u}, 1, 0xd032u /* 50 mmol/L (50*10^-3 mol/L)*/,
        (CY_BLE_GLS_GLMT_TYPE_CWB | (CY_BLE_GLS_GLMT_SL_FR << CY_BLE_GLS_GLMT_SLNUM)), 0u},
    {CY_BLE_GLS_GLMT_FLG_TOP | CY_BLE_GLS_GLMT_FLG_GLC | CY_BLE_GLS_GLMT_FLG_SSA,
        2u, {2014u, 7u, 27, 20u, 30u, 40u}, 2, 0xb032u /* 50 mg/dL (50*10^-5 kg/L)*/,
        (CY_BLE_GLS_GLMT_TYPE_CWB | (CY_BLE_GLS_GLMT_SL_FR << CY_BLE_GLS_GLMT_SLNUM)), CY_BLE_GLS_GLMT_SSA_BTL},
    {CY_BLE_GLS_GLMT_FLG_TOP | CY_BLE_GLS_GLMT_FLG_SSA,
        3u, {2014u, 7u, 27, 20u, 30u, 40u}, 60, 0xb032u /* 50 mg/dL */,
        (CY_BLE_GLS_GLMT_TYPE_CWB | (CY_BLE_GLS_GLMT_SL_FR << CY_BLE_GLS_GLMT_SLNUM)), CY_BLE_GLS_GLMT_SSA_BTL},
    {CY_BLE_GLS_GLMT_FLG_TOP | CY_BLE_GLS_GLMT_FLG_GLC | CY_BLE_GLS_GLMT_FLG_GCU | CY_BLE_GLS_GLMT_FLG_CIF,
        4u, {2014u, 7u, 27, 20u, 30u, 40u}, 60, 0xd032u /* 50 mmol/L (50*10^-3 mol/L)*/,
        (CY_BLE_GLS_GLMT_TYPE_CWB | (CY_BLE_GLS_GLMT_SL_FR << CY_BLE_GLS_GLMT_SLNUM)), 0u},
    {CY_BLE_GLS_GLMT_FLG_TOP | CY_BLE_GLS_GLMT_FLG_GLC | CY_BLE_GLS_GLMT_FLG_SSA,
        5u, {2014u, 7u, 27, 20u, 30u, 40u}, 59, 0xb032u /* 50 mg/dL (50*10^-5 kg/L)*/,
        (CY_BLE_GLS_GLMT_TYPE_CWB | (CY_BLE_GLS_GLMT_SL_FR << CY_BLE_GLS_GLMT_SLNUM)), CY_BLE_GLS_GLMT_SSA_BTL},
    {CY_BLE_GLS_GLMT_FLG_TOP | CY_BLE_GLS_GLMT_FLG_SSA,
        6u, {2014u, 7u, 27, 20u, 30u, 40u}, -60, 0xb032u /* 50 mg/dL */,
        (CY_BLE_GLS_GLMT_TYPE_CWB | (CY_BLE_GLS_GLMT_SL_FR << CY_BLE_GLS_GLMT_SLNUM)), CY_BLE_GLS_GLMT_SSA_BTL},
    {CY_BLE_GLS_GLMT_FLG_TOP | CY_BLE_GLS_GLMT_FLG_GLC | CY_BLE_GLS_GLMT_FLG_GCU | CY_BLE_GLS_GLMT_FLG_CIF,
        7u, {2014u, 7u, 27, 20u, 30u, 40u}, -60, 0xd032u /* 50 mmol/L (50*10^-3 mol/L)*/,
        (CY_BLE_GLS_GLMT_TYPE_CWB | (CY_BLE_GLS_GLMT_SL_FR << CY_BLE_GLS_GLMT_SLNUM)), 0u},
    {CY_BLE_GLS_GLMT_FLG_TOP | CY_BLE_GLS_GLMT_FLG_GLC | CY_BLE_GLS_GLMT_FLG_SSA,
        8u, {2014u, 7u, 27, 20u, 30u, 40u}, -58, 0xb032u /* 50 mg/dL (50*10^-5 kg/L)*/,
        (CY_BLE_GLS_GLMT_TYPE_CWB | (CY_BLE_GLS_GLMT_SL_FR << CY_BLE_GLS_GLMT_SLNUM)), CY_BLE_GLS_GLMT_SSA_BTL},
    {CY_BLE_GLS_GLMT_FLG_TOP | CY_BLE_GLS_GLMT_FLG_GLC | CY_BLE_GLS_GLMT_FLG_SSA,
        9u, {2014u, 7u, 27, 20u, 32u, 45u}, 10u, 0xb037u /* 55 mg/dL (50*10^-5 kg/L)*/,
        (CY_BLE_GLS_GLMT_TYPE_CWB | (CY_BLE_GLS_GLMT_SL_FR << CY_BLE_GLS_GLMT_SLNUM)), CY_BLE_GLS_GLMT_SSA_BTL},
    {CY_BLE_GLS_GLMT_FLG_TOP | CY_BLE_GLS_GLMT_FLG_GLC | CY_BLE_GLS_GLMT_FLG_SSA | CY_BLE_GLS_GLMT_FLG_CIF,
        10u, {2014u, 7u, 27, 20u, 33u, 46u}, 11u, 0xb032u /* 50 mg/dL (50*10^-5 kg/L)*/,
        (CY_BLE_GLS_GLMT_TYPE_CWB | (CY_BLE_GLS_GLMT_SL_FR << CY_BLE_GLS_GLMT_SLNUM)), CY_BLE_GLS_GLMT_SSA_BTL}
};


/* Glucose Measurement Context records */
cy_stc_ble_gls_glmc_t glsGluCont[CY_BLE_GLS_REC_NUM] =
{
    {CY_BLE_GLS_GLMC_FLG_EXT, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
    {CY_BLE_GLS_GLMC_FLG_CBID | CY_BLE_GLS_GLMC_FLG_MEAL | CY_BLE_GLS_GLMC_FLG_TNH |
        CY_BLE_GLS_GLMC_FLG_EXR | CY_BLE_GLS_GLMC_FLG_MED | CY_BLE_GLS_GLMC_FLG_A1C | CY_BLE_GLS_GLMC_FLG_EXT,
        1u, 0u, CY_BLE_GLS_GLMC_CBID_DRINK, 0xd032u /* 50 gram (50*10^-3 kg)*/,
        CY_BLE_GLS_GLMC_MEAL_FAST, CY_BLE_GLS_GLMC_TESTER_LAB | (CY_BLE_GLS_GLMC_HEALTH_US << CY_BLE_GLS_GLMC_HEALTHNUM),
        780u /* 13 min */, 78u /* 78% */,
        CY_BLE_GLS_GLMC_MEDID_IAI, 0xa032u /* 50 mgram (50*10^-6 kg)*/,
        0x0032u /* 50% */},
    {CY_BLE_GLS_GLMC_FLG_EXT, 2u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
    {CY_BLE_GLS_GLMC_FLG_EXT, 3u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
    {CY_BLE_GLS_GLMC_FLG_CBID | CY_BLE_GLS_GLMC_FLG_MEAL | CY_BLE_GLS_GLMC_FLG_TNH |
        CY_BLE_GLS_GLMC_FLG_EXR | CY_BLE_GLS_GLMC_FLG_MED | CY_BLE_GLS_GLMC_FLG_A1C | CY_BLE_GLS_GLMC_FLG_EXT,
        4u, 0u, CY_BLE_GLS_GLMC_CBID_DRINK, 0xd032u /* 50 gram (50*10^-3 kg)*/,
        CY_BLE_GLS_GLMC_MEAL_FAST, CY_BLE_GLS_GLMC_TESTER_LAB | (CY_BLE_GLS_GLMC_HEALTH_US << CY_BLE_GLS_GLMC_HEALTHNUM),
        780u /* 13 min */, 78u /* 78% */,
        CY_BLE_GLS_GLMC_MEDID_IAI, 0xa032u /* 50 mgram (50*10^-6 kg)*/,
        0x0032u /* 50% */},
    {CY_BLE_GLS_GLMC_FLG_EXT, 5u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
    {CY_BLE_GLS_GLMC_FLG_EXT, 6u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
    {CY_BLE_GLS_GLMC_FLG_CBID | CY_BLE_GLS_GLMC_FLG_MEAL | CY_BLE_GLS_GLMC_FLG_TNH |
        CY_BLE_GLS_GLMC_FLG_EXR | CY_BLE_GLS_GLMC_FLG_MED | CY_BLE_GLS_GLMC_FLG_A1C | CY_BLE_GLS_GLMC_FLG_EXT,
        7u, 0u, CY_BLE_GLS_GLMC_CBID_DRINK, 0xd032u /* 50 gram (50*10^-3 kg)*/,
        CY_BLE_GLS_GLMC_MEAL_FAST, CY_BLE_GLS_GLMC_TESTER_LAB | (CY_BLE_GLS_GLMC_HEALTH_US << CY_BLE_GLS_GLMC_HEALTHNUM),
        780u /* 13 min */, 78u /* 78% */,
        CY_BLE_GLS_GLMC_MEDID_IAI, 0xa032u /* 50 mgram (50*10^-6 kg)*/,
        0x0032u /* 50% */},
    {CY_BLE_GLS_GLMC_FLG_EXT, 8u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
    {CY_BLE_GLS_GLMC_FLG_EXT, 9u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u},
    {CY_BLE_GLS_GLMC_FLG_CBID | CY_BLE_GLS_GLMC_FLG_MEAL | CY_BLE_GLS_GLMC_FLG_TNH |
        CY_BLE_GLS_GLMC_FLG_EXR | CY_BLE_GLS_GLMC_FLG_MED | CY_BLE_GLS_GLMC_FLG_A1C | CY_BLE_GLS_GLMC_FLG_EXT,
        10u, 0u, CY_BLE_GLS_GLMC_CBID_DRINK, 0xd032u /* 50 gram (50*10^-3 kg)*/,
        CY_BLE_GLS_GLMC_MEAL_FAST, CY_BLE_GLS_GLMC_TESTER_LAB | (CY_BLE_GLS_GLMC_HEALTH_US << CY_BLE_GLS_GLMC_HEALTHNUM),
        780u /* 13 min */, 78u /* 78% */,
        CY_BLE_GLS_GLMC_MEDID_IAI, 0xa032u /* 50 mgram (50*10^-6 kg)*/,
        0x0032u /* 50% */}
};


/*******************************************************************************
* Function Name: Operand
********************************************************************************
*
* Summary:
*   Prints the Filter Type and Filter Parameters of the RACP Operand.
*
* Parameters:
*   val - RACP value.
*
*******************************************************************************/
void Operand(uint8_t* val)
{
    DBG_PRINTF("Filter type: \r\n");
    racpFilterType = val[2u];
    if(CY_BLE_GLS_RACP_OPD_1 == racpFilterType)
    {
        DBG_PRINTF("Sequence number: ");
        seqNum1 = Cy_BLE_Get16ByPtr(&val[3u]);
        DBG_PRINTF("%x", seqNum1);
        if(CY_BLE_GLS_RACP_OPR_WITHIN == racpOperator)
        {
            seqNum2 = Cy_BLE_Get16ByPtr(&val[5u]);
            DBG_PRINTF(", %x", seqNum2);
        }
        DBG_PRINTF("\r\n");
    }
    else if(CY_BLE_GLS_RACP_OPD_1 == racpFilterType)
    {
        DBG_PRINTF("User Facing Time \r\n");
        userFacingTime1.year    = Cy_BLE_Get16ByPtr(&val[3u]);
        userFacingTime1.month   = val[5u];
        userFacingTime1.day     = val[6u];
        userFacingTime1.hours   = val[7u];
        userFacingTime1.minutes = val[8u];
        userFacingTime1.seconds = val[9u];

        DBG_PRINTF(" %d a.d. %d/%d %d:%d:%d \r\n", userFacingTime1.year,
                                                   userFacingTime1.month,
                                                   userFacingTime1.day,
                                                   userFacingTime1.hours,
                                                   userFacingTime1.minutes,
                                                   userFacingTime1.seconds);

        if(CY_BLE_GLS_RACP_OPR_WITHIN == racpOperator)
        {
            userFacingTime2.year    = Cy_BLE_Get16ByPtr(&val[10u]);
            userFacingTime2.month   = val[12u];
            userFacingTime2.day     = val[13u];
            userFacingTime2.hours   = val[14u];
            userFacingTime2.minutes = val[15u];
            userFacingTime2.seconds = val[16u];

            DBG_PRINTF(" %d a.d. %d/%d %d:%d:%d \r\n", userFacingTime2.year,
                                                   userFacingTime2.month,
                                                   userFacingTime2.day,
                                                   userFacingTime2.hours,
                                                   userFacingTime2.minutes,
                                                   userFacingTime2.seconds);
        }
    }
    else
    {
        DBG_PRINTF("Unknown \r\n");
    }
}


/******************************************************************************
* Function Name: GlsInit
*******************************************************************************
*
* Summary:
*   Registers the GLS CallBack.
*
******************************************************************************/
void GlsInit(void)
{
    Cy_BLE_GLS_RegisterAttrCallback(GlsCallBack);
}


/*******************************************************************************
* Function Name: GlsCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service-specific events from
*   Glucose Service.
*
* Parameters:
*   event     - the event code
*   eventParam - the event parameters
*
********************************************************************************/
void GlsCallBack(uint32_t event, void* eventParam)
{
    uint8_t i;
    switch(event)
    {
        case CY_BLE_EVT_GLSS_WRITE_CHAR:
            DBG_PRINTF("RACP is written: ");
            for(i = 0; i < ((cy_stc_ble_gls_char_value_t *)eventParam)->value->len; i++)
            {
                DBG_PRINTF("%2.2x ", ((cy_stc_ble_gls_char_value_t *)eventParam)->value->val[i]);
            }
            DBG_PRINTF("\r\n");

            if((0u == racpCommand) || (CY_BLE_GLS_RACP_OPC_ABORT_OPN == ((cy_stc_ble_gls_char_value_t *)eventParam)->value->val[0]))
            {
                racpCommand = 1u;
                DBG_PRINTF("Opcode: ");
                racpOpCode = ((cy_stc_ble_gls_char_value_t *)eventParam)->value->val[0];
                switch(racpOpCode)
                {
                    case CY_BLE_GLS_RACP_OPC_REPORT_REC:
                        DBG_PRINTF("Report stored records \r\n");
                        break;

                    case CY_BLE_GLS_RACP_OPC_DELETE_REC:
                        DBG_PRINTF("Delete stored records \r\n");
                        break;

                    case CY_BLE_GLS_RACP_OPC_ABORT_OPN:
                        DBG_PRINTF("Abort operation \r\n");
                        break;

                    case CY_BLE_GLS_RACP_OPC_REPORT_NUM_REC:
                        DBG_PRINTF("Report number of stored records \r\n");
                        break;

                    default:
                        DBG_PRINTF("Unknown \r\n");
                        break;
                }

                DBG_PRINTF("Operator: ");
                racpOperator = ((cy_stc_ble_gls_char_value_t *)eventParam)->value->val[1];

                switch(racpOperator)
                {
                    case CY_BLE_GLS_RACP_OPR_NULL:
                        DBG_PRINTF("Null \r\n");
                        break;

                    case CY_BLE_GLS_RACP_OPR_LAST:
                        DBG_PRINTF("Last record \r\n");
                        if(((cy_stc_ble_gls_char_value_t *)eventParam)->value->len > 2)
                        {
                            racpCommand = 2u; /* invalid operand */
                        }
                        break;

                    case CY_BLE_GLS_RACP_OPR_FIRST:
                        DBG_PRINTF("First record \r\n");
                        if(((cy_stc_ble_gls_char_value_t *)eventParam)->value->len > 2)
                        {
                            racpCommand = 2u; /* invalid operand */
                        }
                        break;

                    case CY_BLE_GLS_RACP_OPR_ALL:
                        DBG_PRINTF("All records \r\n");
                        if(((cy_stc_ble_gls_char_value_t *)eventParam)->value->len > 2)
                        {
                            racpCommand = 2u; /* invalid operand */
                        }
                        break;

                    case CY_BLE_GLS_RACP_OPR_LESS:
                        DBG_PRINTF("Less than or equal to \r\n");
                        Operand(((cy_stc_ble_gls_char_value_t *)eventParam)->value->val);
                        break;


                    case CY_BLE_GLS_RACP_OPR_GREAT:
                        DBG_PRINTF("Greater than or equal to \r\n");
                        Operand(((cy_stc_ble_gls_char_value_t *)eventParam)->value->val);
                        break;

                    case CY_BLE_GLS_RACP_OPR_WITHIN:
                        DBG_PRINTF("Within range of (inclusive) \r\n");
                        Operand(((cy_stc_ble_gls_char_value_t *)eventParam)->value->val);
                        break;

                    default:
                        DBG_PRINTF("Unknown \r\n");
                        break;
                }
            }
            else
            {
                DBG_PRINTF("new request is not accepted because of current request is still being processed \r\n");
            }

            break;

        case CY_BLE_EVT_GLSS_NOTIFICATION_ENABLED:
            if(CY_BLE_GLS_GLMT == ((cy_stc_ble_gls_char_value_t*)eventParam)->charIndex)
            {
                DBG_PRINTF("Glucose Measurement");
            }
            else if(CY_BLE_GLS_GLMC == ((cy_stc_ble_gls_char_value_t*)eventParam)->charIndex)
            {
                DBG_PRINTF("Glucose Measurement Context");
            }
            else
            {
                DBG_PRINTF("Other GLS");
            }
            DBG_PRINTF(" characteristic notification is enabled \r\n");
            break;

        case CY_BLE_EVT_GLSS_NOTIFICATION_DISABLED:
            if(CY_BLE_GLS_GLMT == ((cy_stc_ble_gls_char_value_t*)eventParam)->charIndex)
            {
                DBG_PRINTF("Glucose Measurement");
            }
            else if(CY_BLE_GLS_GLMC == ((cy_stc_ble_gls_char_value_t*)eventParam)->charIndex)
            {
                DBG_PRINTF("Glucose Measurement Context");
            }
            else
            {
                DBG_PRINTF("Other GLS");
            }
            DBG_PRINTF(" characteristic notification is disabled \r\n");
            break;

        case CY_BLE_EVT_GLSS_INDICATION_ENABLED:
            if(CY_BLE_GLS_RACP == ((cy_stc_ble_gls_char_value_t*)eventParam)->charIndex)
            {
                DBG_PRINTF("RACP");
            }
            else
            {
                DBG_PRINTF("Other GLS");
            }
            DBG_PRINTF(" characteristic indication is enabled \r\n");
            break;

        case CY_BLE_EVT_GLSS_INDICATION_DISABLED:
            if(CY_BLE_GLS_RACP == ((cy_stc_ble_gls_char_value_t*)eventParam)->charIndex)
            {
                DBG_PRINTF("RACP");
            }
            else
            {
                DBG_PRINTF("Other GLS");
            }
            DBG_PRINTF(" characteristic indication is disabled \r\n");
            break;

        case CY_BLE_EVT_GLSS_INDICATION_CONFIRMED:
            if(CY_BLE_GLS_RACP == ((cy_stc_ble_gls_char_value_t*)eventParam)->charIndex)
            {
                DBG_PRINTF("RACP");
            }
            else
            {
                DBG_PRINTF("Other GLS");
            }
            DBG_PRINTF(" characteristic indication is confirmed \r\n");
            break;

        default:
            DBG_PRINTF("Unknown GLS event: %lx \r\n", event);
            break;
    }
}


/*******************************************************************************
* Function Name: GlsNtf
********************************************************************************
*
* Summary:
*   Sends the Glucose Measurement notification and the Glucose Measurement
*   Context notification (if the appropriate bit is set).
*
* Parameters:
*   num - number of record to notify.
*
*******************************************************************************/
void GlsNtf(cy_stc_ble_conn_handle_t connHandle, uint8_t num)
{
    cy_en_ble_api_result_t apiResult;
    uint8_t pdu[sizeof(cy_stc_ble_gls_glmt_t)]; /* GLMC size is also 17 bytes */
    uint8_t ptr;
    uint16_t cccd;
    
    apiResult = Cy_BLE_GLSS_GetCharacteristicDescriptor(connHandle, CY_BLE_GLS_GLMT, CY_BLE_GLS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
              
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_GLSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
    }
    else if(cccd == CY_BLE_CCCD_NOTIFICATION) 
    {
        /* flags field is always the first byte */
        pdu[0u] = glsGlucose[num].flags;

        /* Sequence number is always the second and third bytes */
        Cy_BLE_Set16ByPtr(&pdu[1u], glsGlucose[num].seqNum);

        /* Base Time consumes the next seven bytes */
        (void)memcpy(&pdu[3u], &glsGlucose[num].baseTime, sizeof(glsGlucose[num].baseTime));

        ptr = 10u; /* 3 + 7 (size of baseTime) */

        /* if the Time Offset Present flag is set */
        if(0u != (glsGlucose[num].flags & CY_BLE_GLS_GLMT_FLG_TOP))
        {
            /* and set the full 2-bytes Time Offset value */
            (void)memcpy(&pdu[ptr], &glsGlucose[num].timeOffset, sizeof(glsGlucose[num].timeOffset));
            /* the next data will be located beginning from 3rd byte */
            ptr += sizeof(glsGlucose[num].timeOffset);
        }


        if(0u != (glsGlucose[num].flags & CY_BLE_GLS_GLMT_FLG_GLC))
        {
            (void)memcpy(&pdu[ptr], &glsGlucose[num].gluConc, sizeof(glsGlucose[num].gluConc));
            ptr += sizeof(glsGlucose[num].gluConc);
            pdu[ptr] = glsGlucose[num].tnsl;
            ptr += 1u; /* uint8_t tnsl */
        }

        if(0u != (glsGlucose[num].flags & CY_BLE_GLS_GLMT_FLG_SSA))
        {
            (void)memcpy(&pdu[ptr], &glsGlucose[num].ssa, sizeof(glsGlucose[num].ssa));
            ptr += 2u; /* uint16_t ssa */
        }
        
        do
        {
            Cy_BLE_ProcessEvents();
        }
        while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);

        if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
        {
            if((apiResult = Cy_BLE_GLSS_SendNotification(connHandle, CY_BLE_GLS_GLMT, ptr, pdu)) != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GLSS_SendNotification API Error: ");
                PrintApiResult(apiResult);
            }
            else
            {
                DBG_PRINTF("Glucose Ntf: %d \r\n", glsGlucose[num].seqNum);
                racpInd[3] = CY_BLE_GLS_RACP_RSP_SUCCESS;
            }
        }
    }
    
    apiResult = Cy_BLE_GLSS_GetCharacteristicDescriptor(connHandle, CY_BLE_GLS_GLMC, CY_BLE_GLS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
              
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_GLSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
    }
    else if(cccd == CY_BLE_CCCD_NOTIFICATION) 
    {   
        if(0u != (glsGlucose[num].flags & CY_BLE_GLS_GLMT_FLG_CIF))
        {
            /* flags field is always the first byte */
            pdu[0u] = glsGluCont[num].flags;

            /* Sequence number is always the second and third bytes */
            Cy_BLE_Set16ByPtr(&pdu[1u], glsGluCont[num].seqNum);

            /* if the Time Offset Present flag is set */
            if(0u != (glsGluCont[num].flags & CY_BLE_GLS_GLMC_FLG_EXT))
            {
                /* and set the 1-byte Extended Flags */
                pdu[3u] = glsGluCont[num].exFlags;
                
                /* the next data will be located beginning from 3rd byte */
                ptr = 4u;
            }
            else
            {
                ptr = 3u; /* otherwise the next data will be located beginning from 2nd byte */
            }

            if(0u != (glsGluCont[num].flags & CY_BLE_GLS_GLMC_FLG_CBID))
            {
                pdu[ptr] = glsGluCont[num].cbId;
                ptr += 1u; /* uint8_t cbId */
                (void)memcpy(&pdu[ptr], &glsGluCont[num].cbhdr, sizeof(glsGluCont[num].cbhdr));
                ptr += sizeof(glsGluCont[num].cbhdr);
            }

            if(0u != (glsGluCont[num].flags & CY_BLE_GLS_GLMC_FLG_MEAL))
            {
                pdu[ptr] = glsGluCont[num].meal;
                ptr += 1u; /* uint8_t meal */
            }

            if(0u != (glsGluCont[num].flags & CY_BLE_GLS_GLMC_FLG_TNH))
            {
                pdu[ptr] = glsGluCont[num].tnh;
                ptr += 1u; /* uint8_t tnh */
            }

            if(0u != (glsGluCont[num].flags & CY_BLE_GLS_GLMC_FLG_EXR))
            {
                Cy_BLE_Set16ByPtr(&pdu[ptr], glsGluCont[num].exDur);
                ptr += 2u; /* uint16_t exDur */
                pdu[ptr] = glsGluCont[num].exInt;
                ptr += 1u; /* uint8_t exInt */
            }

            if(0u != (glsGluCont[num].flags & CY_BLE_GLS_GLMC_FLG_MED))
            {
                pdu[ptr] = glsGluCont[num].medId;
                ptr += 1u; /* uint8_t medId */
                (void)memcpy(&pdu[ptr], &glsGluCont[num].medic, sizeof(glsGluCont[num].medic));
                ptr += sizeof(glsGluCont[num].medic);
            }

            if(0u != (glsGluCont[num].flags & CY_BLE_GLS_GLMC_FLG_A1C))
            {
                (void)memcpy(&pdu[ptr], &glsGluCont[num].hba1c, sizeof(glsGluCont[num].hba1c));
                ptr += sizeof(glsGluCont[num].hba1c);
            }
            
            do
            {
                Cy_BLE_ProcessEvents();
            }
            while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);
         
            if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
            {
                if((apiResult = Cy_BLE_GLSS_SendNotification(connHandle, CY_BLE_GLS_GLMC, ptr, pdu)) != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_GLSS_SendNotification API Error: ");
                    PrintApiResult(apiResult);
                }
                else
                {
                    DBG_PRINTF("Glucose Context Ntf: %d \r\n", glsGlucose[num].seqNum);
                }
            }
        }
    }
}


/*******************************************************************************
* Function Name: GlsInd
********************************************************************************
*
* Summary:
*   Sends the GLS RACP indication with the status of the current request.
*
*******************************************************************************/
void GlsInd(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    racpInd[1] = CY_BLE_GLS_RACP_OPR_NULL;
    uint16_t cccd;
    
    apiResult = Cy_BLE_GLSS_GetCharacteristicDescriptor(connHandle, CY_BLE_GLS_RACP, CY_BLE_GLS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
              
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Cy_BLE_GLSS_GetCharacteristicDescriptor API Error: 0x%x \r\n", apiResult);
    }
    else if(cccd == CY_BLE_CCCD_INDICATION) 
    {
        if(CY_BLE_GLS_RACP_OPC_REPORT_NUM_REC == racpOpCode)
        {
            racpInd[0] = CY_BLE_GLS_RACP_OPC_NUM_REC_RSP;
            racpInd[2] = recCnt;
            racpInd[3] = 0;
        }
        else
        {
            racpInd[0] = CY_BLE_GLS_RACP_OPC_RSP_CODE;
            racpInd[2] = racpOpCode;
        }
        
        do
        {
            Cy_BLE_ProcessEvents();
        }
        while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);

        if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
        {
            if((apiResult = Cy_BLE_GLSS_SendIndication(connHandle, CY_BLE_GLS_RACP, 4, racpInd)) != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_GLSS_SendIndication API Error: ");
                PrintApiResult(apiResult);
            }
            else
            {
                DBG_PRINTF("RACP Ind: %d %d %d %d \r\n", racpInd[0], racpInd[1], racpInd[2], racpInd[3]);
            }
        }
    }    
}


/*******************************************************************************
* Function Name: TimeComparison
********************************************************************************
*
* Summary:
*   Compares the user facing time with time specified in
*   Glucose measurement record.
*
* Parameters:
*   glmtTime - Time specified in Glucose measurement record.
*   userFacingTime - User facing time.
*
* Return:
*   cy_en_ble_date_time_comp_t retVal - result of time comparison.
*
*******************************************************************************/
cy_en_ble_date_time_comp_t TimeComparison(cy_stc_ble_date_time_t glmtTime, cy_stc_ble_date_time_t userFacingTime)
{
    cy_en_ble_date_time_comp_t retVal = CY_BLE_TIME_LESS;

    if(glmtTime.year == userFacingTime.year)
    {
        if(glmtTime.month == userFacingTime.month)
        {
            if(glmtTime.day == userFacingTime.day)
            {
                if(glmtTime.hours == userFacingTime.hours)
                {
                    if(glmtTime.minutes == userFacingTime.minutes)
                    {
                        if(glmtTime.seconds == userFacingTime.seconds)
                        {
                            retVal = CY_BLE_TIME_EQUAL;
                        }
                        else if(glmtTime.seconds > userFacingTime.seconds)
                        {
                            retVal = CY_BLE_TIME_GREAT;
                        }
                        else
                        {
                            /* CY_BLE_TIME_LESS */
                        }
                    }
                    else if(glmtTime.minutes > userFacingTime.minutes)
                    {
                        retVal = CY_BLE_TIME_GREAT;
                    }
                    else
                    {
                        /* CY_BLE_TIME_LESS */
                    }
                }
                else if(glmtTime.hours > userFacingTime.hours)
                {
                    retVal = CY_BLE_TIME_GREAT;
                }
                else
                {
                    /* CY_BLE_TIME_LESS */
                }
            }
            else if(glmtTime.day > userFacingTime.day)
            {
                retVal = CY_BLE_TIME_GREAT;
            }
            else
            {
                /* CY_BLE_TIME_LESS */
            }
        }
        else if(glmtTime.month > userFacingTime.month)
        {
            retVal = CY_BLE_TIME_GREAT;
        }
        else
        {
            /* CY_BLE_TIME_LESS */
        }
    }
    else if(glmtTime.year > userFacingTime.year)
    {
        retVal = CY_BLE_TIME_GREAT;
    }
    else
    {
        /* CY_BLE_TIME_LESS */
    }

    return (retVal);
}

/*******************************************************************************
* Function Name: OpCodeOperation
********************************************************************************
*
* Summary:
*   Doing operation in accordance to opcode and status or current record.
*
* Parameters:
*   i - index of record.
*
*******************************************************************************/
void OpCodeOperation(cy_stc_ble_conn_handle_t connHandle, uint8_t i)
{
    if(CY_BLE_GLS_REC_STAT_OK == recStat[i])
    {
        switch(racpOpCode)
        {
            case CY_BLE_GLS_RACP_OPC_REPORT_REC:
                GlsNtf(connHandle, i);
                break;

            case CY_BLE_GLS_RACP_OPC_DELETE_REC:
                recStat[i] = CY_BLE_GLS_REC_STAT_DELETED;
                racpInd[3] = CY_BLE_GLS_RACP_RSP_SUCCESS;
                break;

            case CY_BLE_GLS_RACP_OPC_REPORT_NUM_REC:
                recCnt++;
                break;

            default:
                racpInd[3] = CY_BLE_GLS_RACP_RSP_UNSPRT_OPC;
                break;
        }
    }
}

/*******************************************************************************
* Function Name: GlsProcess
********************************************************************************
*
* Summary:
*   Processes the GLS RACP request.
*
* Parameters:
*   connHandle: The connection handle
*
*******************************************************************************/
void GlsProcess(cy_stc_ble_conn_handle_t connHandle)
{
    if(0u != racpCommand)
    {
        uint8_t i;
        Cy_SysLib_Delay(7);
        Cy_BLE_ProcessEvents();

        recCnt = 0u;
        racpInd[3] = CY_BLE_GLS_RACP_RSP_NO_REC;

        switch(racpOperator)
        {
            case CY_BLE_GLS_RACP_OPR_NULL:
                switch(racpOpCode)
                {
                    case CY_BLE_GLS_RACP_OPC_ABORT_OPN:
                        racpInd[3] = CY_BLE_GLS_RACP_RSP_SUCCESS;
                        break;

                    default:
                        racpInd[3] = CY_BLE_GLS_RACP_RSP_INV_OPR;
                        break;
                }

                GlsInd(connHandle);
                break;

            case CY_BLE_GLS_RACP_OPR_LAST:
                if(2u == racpCommand)
                {
                    racpInd[3] = CY_BLE_GLS_RACP_RSP_INV_OPD;
                }
                else
                {
                    OpCodeOperation(connHandle, CY_BLE_GLS_REC_NUM - 1);
                }

                GlsInd(connHandle);
                break;

            case CY_BLE_GLS_RACP_OPR_FIRST:
                if(2u == racpCommand)
                {
                    racpInd[3] = CY_BLE_GLS_RACP_RSP_INV_OPD;
                }
                else
                {
                    OpCodeOperation(connHandle, 0);
                }

                GlsInd(connHandle);
                break;

            case CY_BLE_GLS_RACP_OPR_ALL:
                if(2u == racpCommand)
                {
                    racpInd[3] = CY_BLE_GLS_RACP_RSP_INV_OPD;
                }
                else
                {
                    for(i = 0; i < CY_BLE_GLS_REC_NUM; i++)
                    {
                        OpCodeOperation(connHandle, i);
                    }
                }

                GlsInd(connHandle);
                break;

            case CY_BLE_GLS_RACP_OPR_LESS:
                if(CY_BLE_GLS_RACP_OPD_1 == racpFilterType)
                {
                    if(CY_BLE_GLS_REC_NUM > seqNum1)
                    {
                        for(i = 0; i <= seqNum1; i++)
                        {
                            OpCodeOperation(connHandle, i);
                        }
                    }
                }
                else if(CY_BLE_GLS_RACP_OPD_2 == racpFilterType)
                {
                    if(CY_BLE_TIME_GREAT == TimeComparison(glsGlucose[CY_BLE_GLS_REC_NUM - 1].baseTime, userFacingTime1))
                    {
                        for(i = 0; i < CY_BLE_GLS_REC_NUM; i++)
                        {
                            if(CY_BLE_TIME_GREAT != TimeComparison(glsGlucose[i].baseTime, userFacingTime1))
                            {
                                OpCodeOperation(connHandle, i);
                            }
                        }
                    }
                }
                else
                {
                    racpInd[3] = CY_BLE_GLS_RACP_RSP_UNSPRT_OPD;
                }
                GlsInd(connHandle);
                break;

            case CY_BLE_GLS_RACP_OPR_GREAT:
                if(CY_BLE_GLS_RACP_OPD_1 == racpFilterType)
                {
                    if(CY_BLE_GLS_REC_NUM > seqNum1)
                    {
                        for(i = seqNum1; i < CY_BLE_GLS_REC_NUM; i++)
                        {
                            OpCodeOperation(connHandle, i);
                        }
                    }
                }
                else if(CY_BLE_GLS_RACP_OPD_2 == racpFilterType)
                {
                    if(CY_BLE_TIME_GREAT == TimeComparison(glsGlucose[CY_BLE_GLS_REC_NUM - 1].baseTime, userFacingTime1))
                    {
                        for(i = 0; i < CY_BLE_GLS_REC_NUM; i++)
                        {
                            if(CY_BLE_TIME_LESS != TimeComparison(glsGlucose[i].baseTime, userFacingTime1))
                            {
                                OpCodeOperation(connHandle, i);
                            }
                        }
                    }
                }
                else
                {
                    racpInd[3] = CY_BLE_GLS_RACP_RSP_UNSPRT_OPD;
                }

                GlsInd(connHandle);
                break;

            case CY_BLE_GLS_RACP_OPR_WITHIN:
                if(CY_BLE_GLS_RACP_OPD_1 == racpFilterType)
                {
                    if(seqNum2 >= CY_BLE_GLS_REC_NUM)
                    {
                        seqNum2 = CY_BLE_GLS_REC_NUM - 1;
                    }

                    if(seqNum1 > seqNum2)
                    {
                        racpInd[3] = CY_BLE_GLS_RACP_RSP_INV_OPD;
                    }
                    else
                    {
                        for(i = seqNum1; i <= seqNum2; i++)
                        {
                            OpCodeOperation(connHandle, i);
                        }
                    }
                }
                else if(CY_BLE_GLS_RACP_OPD_2 == racpFilterType)
                {
                    if(CY_BLE_TIME_GREAT != TimeComparison(glsGlucose[CY_BLE_GLS_REC_NUM - 1].baseTime, userFacingTime2))
                    {
                        userFacingTime2 = glsGlucose[CY_BLE_GLS_REC_NUM - 1].baseTime;
                    }

                    if(CY_BLE_TIME_GREAT == TimeComparison(userFacingTime1, userFacingTime2))
                    {
                        racpInd[3] = CY_BLE_GLS_RACP_RSP_INV_OPD;
                    }
                    else
                    {
                        for(i = 0; i < CY_BLE_GLS_REC_NUM; i++)
                        {
                            if((CY_BLE_TIME_LESS != TimeComparison(glsGlucose[i].baseTime, userFacingTime1)) &&
                               (CY_BLE_TIME_GREAT != TimeComparison(glsGlucose[i].baseTime, userFacingTime2)))
                            {
                                OpCodeOperation(connHandle, i);
                            }
                        }
                    }
                }
                else
                {
                    racpInd[3] = CY_BLE_GLS_RACP_RSP_UNSPRT_OPD;
                }

                GlsInd(connHandle);
                break;

            default:
                racpInd[3] = CY_BLE_GLS_RACP_RSP_UNSPRT_OPR;
                GlsInd(connHandle);
                break;         
        }
        
        /* Cy_BLE_ProcessEvents() allows BLE stack to process pending events */
        Cy_BLE_ProcessEvents();

        racpCommand = 0u;
    }
}

    
/* [] END OF FILE */
