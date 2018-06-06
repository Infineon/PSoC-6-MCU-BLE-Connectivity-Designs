/*******************************************************************************
* File Name: lnss.c
*
* Version 1.0
*
* Description:
*  This file contains the Location and Navigation Service related code.
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
#include "lnss.h"


static uint8_t cp[7u];
static uint8_t lnsFlag  = 0u;
static uint8_t lnsTimer = 0u;
static uint8_t lnsCount = 0u;

static cy_stc_ble_lns_ls_t ls;
static cy_stc_ble_lns_nv_t nv;
static cy_stc_ble_date_time_t time = {2015u, 5u, 21u, 14u, 14u, 41u};

/* Location and Speed characteristic data */
const cy_stc_ble_lns_ls_t cls[LNS_COUNT] =
{
    {
        CY_BLE_LNS_LS_FLG_LC |
        CY_BLE_LNS_LS_FLG_EL |
        CY_BLE_LNS_LS_FLG_UTC, /* Flags */
        138,       /* Instantaneous Speed */
        14u,       /* Total Distance */
        498090300, /* Location - Latitude */ 
        240413960, /* Location - Longitude */
        34409,     /* Elevation */ 
        13500,     /* Heading */
        1,         /* Rolling Time */
        {2015u, 5u, 21u, 14u, 14u, 41u}
    },
    {
        CY_BLE_LNS_LS_FLG_IS |
        CY_BLE_LNS_LS_FLG_TD |
        CY_BLE_LNS_LS_FLG_LC |
        CY_BLE_LNS_LS_FLG_HD |
        CY_BLE_LNS_LS_FLG_RT, /* Flags */
        137,       /* Instantaneous Speed */
        14u,       /* Total Distance */
        498090200, /* Location - Latitude */
        240414060, /* Location - Longitude */
        34420,     /* Elevation */
        22500,     /* Heading */
        2,         /* Rolling Time */
        {2015u, 5u, 21u, 14u, 14u, 42u}
    },
    {
        CY_BLE_LNS_LS_FLG_IS |
        CY_BLE_LNS_LS_FLG_TD |
        CY_BLE_LNS_LS_FLG_LC |
        CY_BLE_LNS_LS_FLG_EL |
        CY_BLE_LNS_LS_FLG_RT, /* Flags */
        139,       /* Instantaneous Speed */
        14u,       /* Total Distance */
        498090100, /* Location - Latitude */
        240413960, /* Location - Longitude */
        34431,     /* Elevation */
        31500,     /* Heading */
        3,         /* Rolling Time */
        {2015u, 5u, 21u, 14u, 14u, 43u}
    },
    {
        CY_BLE_LNS_LS_FLG_TD |
        CY_BLE_LNS_LS_FLG_LC |
        CY_BLE_LNS_LS_FLG_EL |
        CY_BLE_LNS_LS_FLG_HD |
        CY_BLE_LNS_LS_FLG_RT, /* Flags */
        141,       /* Instantaneous Speed */
        14u,       /* Total Distance */
        498090200, /* Location - Latitude */
        240413860, /* Location - Longitude */
        34420,     /* Elevation */
        4500,      /* Heading */
        4,         /* Rolling Time */
        {2015u, 5u, 21u, 14u, 14u, 44u}
    }
};

/* Navigation characteristic data */
const cy_stc_ble_lns_nv_t cnv[LNS_COUNT] =
{
    {
        CY_BLE_LNS_NV_FLG_RD | CY_BLE_LNS_NV_FLG_RVD | CY_BLE_LNS_NV_FLG_EAT,    /* Flags */
        9000,      /* Bearing */
        13500,     /* Heading */
        14u,       /* Remaining Distance */
        11,        /* Remaining Vertical Distance */
        {2015u, 5u, 21u, 14u, 14u, 41u}
    },
    {
        CY_BLE_LNS_NV_FLG_RD | CY_BLE_LNS_NV_FLG_RVD | CY_BLE_LNS_NV_FLG_EAT,    /* Flags */
        9000,      /* Bearing */
        22500,     /* Heading */
        14u,       /* Remaining Distance */
        11,        /* Remaining Vertical Distance */
        {2015u, 5u, 21u, 14u, 14u, 42u}
    },
    {
        CY_BLE_LNS_NV_FLG_RD | CY_BLE_LNS_NV_FLG_RVD | CY_BLE_LNS_NV_FLG_EAT,    /* Flags */
        9000,      /* Bearing */
        31500,     /* Heading */
        14u,       /* Remaining Distance */
        -11,       /* Remaining Vertical Distance */
        {2015u, 5u, 21u, 14u, 14u, 43u}
    },
    {
        CY_BLE_LNS_NV_FLG_RD | CY_BLE_LNS_NV_FLG_RVD | CY_BLE_LNS_NV_FLG_EAT,    /* Flags */
        9000,      /* Bearing */
        4500,      /* Heading */
        14u,       /* Remaining Distance */
        -11,       /* Remaining Vertical Distance */
        {2015u, 5u, 21u, 14u, 14u, 44u}
    }
};

/*******************************************************************************
* Function Name: LnsInit
********************************************************************************
*
* Summary:
*   Initializes the Location and Navigation Service.
*
*******************************************************************************/
void LnsInit(void)
{
    cp[0u] = CY_BLE_LNS_CP_OPC_RC;
    Cy_BLE_LNS_RegisterAttrCallback(LnsCallBack);
}


/*******************************************************************************
* Function Name: LnsCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service-specific events for
*   Location and Navigation Service.
*
* Parameters:
*  event      - The event code.
*  eventParam - The pointer to event data.
*
********************************************************************************/
void LnsCallBack(uint32_t event, void* eventParam)
{
    uint32_t i;
    
    switch(event)
    {
        case CY_BLE_EVT_LNSS_WRITE_CHAR:
            DBG_PRINTF("CP is written: ");
            for(i = 0; i < ((cy_stc_ble_lns_char_value_t *)eventParam)->value->len; i++)
            {
                DBG_PRINTF("%2.2x ", ((cy_stc_ble_lns_char_value_t *)eventParam)->value->val[i]);
            }
            DBG_PRINTF("\r\n");
            
            cp[2u] = CY_BLE_LNS_CP_RSP_SUCCESS;
            
            DBG_PRINTF("Opcode: ");
            cp[1u] = ((cy_stc_ble_lns_char_value_t *)eventParam)->value->val[0];
            switch(cp[1u])
            {
                case CY_BLE_LNS_CP_OPC_SCV:
                    DBG_PRINTF("Set Cumulative Value \r\n");
                    if(4u == ((cy_stc_ble_lns_char_value_t *)eventParam)->value->len)
                    {
                        ls.totalDst = (uint32)((cy_stc_ble_lns_char_value_t *)eventParam)->value->val[1];
                        ls.totalDst |= (uint32)((cy_stc_ble_lns_char_value_t *)eventParam)->value->val[2] << 8;
                        ls.totalDst |= (uint32)((cy_stc_ble_lns_char_value_t *)eventParam)->value->val[3] << 16;
                    }
                    else
                    {
                        cp[2u] = CY_BLE_LNS_CP_RSP_INV_PAR;
                        DBG_PRINTF("Invalid Parameter \r\n");
                    }
                    break;
                    
                case CY_BLE_LNS_CP_OPC_MLS:
                    DBG_PRINTF("Mask Location and Speed \r\n");
                    break;
                    
                case CY_BLE_LNS_CP_OPC_NC:
                    DBG_PRINTF("Navigation Control\r\nParameter: ");
                    switch(((cy_stc_ble_lns_char_value_t *)eventParam)->value->val[1])
                    {
                        case CY_BLE_LNS_CP_OPC_NC_STOP:
                            DBG_PRINTF("Stop navigation \r\n");
                            lnsFlag &= ~NV_EN;
                            break;
                            
                        case CY_BLE_LNS_CP_OPC_NC_START:
                            DBG_PRINTF("Start navigation \r\n");
                            lnsFlag |= NV_EN;
                            break;
                            
                        case CY_BLE_LNS_CP_OPC_NC_PAUSE:
                            DBG_PRINTF("Pause navigation \r\n");
                            lnsFlag &= ~NV_EN;
                            break;  
                            
                        case CY_BLE_LNS_CP_OPC_NC_RESUME:
                            DBG_PRINTF("Resume navigation \r\n");
                            lnsFlag |= NV_EN;
                            break;
                            
                        case CY_BLE_LNS_CP_OPC_NC_SKIP:
                            DBG_PRINTF("Skip waypoint \r\n");
                            break;
                            
                        case CY_BLE_LNS_CP_OPC_NC_NEAR:
                            DBG_PRINTF("Start navigation to nearest waypoint \r\n");
                            lnsFlag |= NV_EN;
                            break;
                            
                        default:
                            DBG_PRINTF("Unknown \r\n");
                            break;
                    }       
                    break;
                    
                case CY_BLE_LNS_CP_OPC_NRS:
                    DBG_PRINTF("Request Number of Routes \r\n");
                    break;
                    
                case CY_BLE_LNS_CP_OPC_RNM:
                    DBG_PRINTF("Request Name of Route \r\n");
                    break;
                    
                case CY_BLE_LNS_CP_OPC_SR:
                    DBG_PRINTF("Select Route \r\n");
                    break;
                    
                case CY_BLE_LNS_CP_OPC_SFR:
                    DBG_PRINTF("Set Fix Rate \r\n");
                    break;
                    
                case CY_BLE_LNS_CP_OPC_SE:
                    DBG_PRINTF("Set Elevation \r\n");
                    ls.elevation = (uint32)((cy_stc_ble_lns_char_value_t *)eventParam)->value->val[1];
                    ls.elevation |= (uint32)((cy_stc_ble_lns_char_value_t *)eventParam)->value->val[2] << 8;
                    ls.elevation |= (uint32)((cy_stc_ble_lns_char_value_t *)eventParam)->value->val[3] << 16;
                    break;
                    
                default:
                    DBG_PRINTF("Unknown \r\n");
                    break;
            }
            break;
        
        case CY_BLE_EVT_LNSS_NOTIFICATION_ENABLED:          
            switch(((cy_stc_ble_lns_char_value_t *)eventParam)->charIndex)
            {
                case CY_BLE_LNS_LS:
                    DBG_PRINTF("Location and Speed Notification is Enabled \r\n");
                    break;
                    
                case CY_BLE_LNS_NV:
                    DBG_PRINTF("Navigation Notification is Enabled \r\n");
                    break;
                
                default:
                break;
            }
            break;
                
        case CY_BLE_EVT_LNSS_NOTIFICATION_DISABLED:
            switch(((cy_stc_ble_lns_char_value_t *)eventParam)->charIndex)
            {
                case CY_BLE_LNS_LS:
                    DBG_PRINTF("Location and Speed Notification is Disabled \r\n");
                    break;
           
                case CY_BLE_LNS_NV:
                    DBG_PRINTF("Navigation Notification is Disabled \r\n");
                    break;
                
                default:
                break;
            }
            break;
            
        case CY_BLE_EVT_LNSS_INDICATION_ENABLED:
            DBG_PRINTF("LN Control Point Indication is Enabled \r\n");
            break;
                
        case CY_BLE_EVT_LNSS_INDICATION_DISABLED:
            DBG_PRINTF("LN Control Point Indication is Disabled \r\n");
            break;
            
        case CY_BLE_EVT_LNSS_INDICATION_CONFIRMED:
            DBG_PRINTF("LN Control Point Indication is Confirmed \r\n");
            break;

        default:
            DBG_PRINTF("unknown LNS event: %lx \r\n", event);
            break;
    }
}


/*******************************************************************************
* Function Name: LnsNtf
********************************************************************************
*
* Summary:
*   Sends the configured (allowed) notifications 
*   of the Location and Navigation Service.
*
*******************************************************************************/
void LnsNtf(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    uint16_t cccd;
    uint32_t i;
    
    lnsTimer++;
    if(lnsTimer >= LNS_TIME)
    {
        lnsTimer = 0u;
        lnsCount++;
        if(lnsCount >= LNS_COUNT)
        {
            lnsCount = 0u;
        }
        
        ls = cls[lnsCount];
        ls.rollTime++;
        ls.utcTime = time;
        
        
        /* Read CCCD for Location and Speed characteristic */
        (void) Cy_BLE_LNSS_GetCharacteristicDescriptor(connHandle, CY_BLE_LNS_LS, CY_BLE_LNS_CCCD,
                                                        CY_BLE_CCCD_LEN, (uint8_t*)&cccd);

        if(cccd == CY_BLE_CCCD_NOTIFICATION)
        {
            uint8_t pdu[sizeof(cy_stc_ble_lns_ls_t)];
            uint8_t ptr;
            
            /* Flags field is always first two bytes */
            Cy_BLE_Set16ByPtr(&pdu[0u], ls.flags);

            /* Next possible value is at 2nd byte */
            ptr = 2u;
                
            /* If Instantaneous Speed Present flag is set */
            if(0u != (ls.flags & CY_BLE_LNS_LS_FLG_IS))
            {
                /* And set 2-byte Instantaneous Speed value */
                Cy_BLE_Set16ByPtr(&pdu[ptr], ls.instSpd);
                /* Next data will be located beginning from 3rd byte */
                ptr += sizeof(ls.instSpd);
            }
            
            if(0u != (ls.flags & CY_BLE_LNS_LS_FLG_TD))
            {
                pdu[ptr] = (uint8_t) (ls.totalDst & 0x000000FFu);
                pdu[ptr + 1] = (uint8_t) ((ls.totalDst & 0x0000FF00u) >> 8u);
                pdu[ptr + 2] = (uint8_t) ((ls.totalDst & 0x00FF0000u) >> 16u);
                ptr += 3; /* Total Distance size is actually 3 bytes */
            }

            if(0u != (ls.flags & CY_BLE_LNS_LS_FLG_LC))
            {
                pdu[ptr] = (uint8_t) (ls.latitude & 0x000000FFu);
                pdu[ptr + 1] = (uint8_t) ((ls.latitude & 0x0000FF00u) >> 8u);
                pdu[ptr + 2] = (uint8_t) ((ls.latitude & 0x00FF0000u) >> 16u);
                pdu[ptr + 3] = (uint8_t) ((ls.latitude & 0xFF000000u) >> 24u);
                ptr += sizeof(ls.latitude);
                pdu[ptr] = (uint8_t) (ls.longitude & 0x000000FFu);
                pdu[ptr + 1] = (uint8_t) ((ls.longitude & 0x0000FF00u) >> 8u);
                pdu[ptr + 2] = (uint8_t) ((ls.longitude & 0x00FF0000u) >> 16u);
                pdu[ptr + 3] = (uint8_t) ((ls.longitude & 0xFF000000u) >> 24u);
                ptr += sizeof(ls.longitude);
            }
            
            if(0u != (ls.flags & CY_BLE_LNS_LS_FLG_EL))
            {
                pdu[ptr] = (uint8_t) (ls.elevation & 0x000000FFu);
                pdu[ptr + 1] = (uint8_t) ((ls.elevation & 0x0000FF00u) >> 8u);
                pdu[ptr + 2] = (uint8_t) ((ls.elevation & 0x00FF0000u) >> 16u);
                ptr += 3; /* Elevation size is actually 3 bytes */
            }
            
            if(0u != (ls.flags & CY_BLE_LNS_LS_FLG_HD))
            {
                Cy_BLE_Set16ByPtr(&pdu[ptr], ls.heading);
                ptr += sizeof(ls.heading);
            }
            
            if(0u != (ls.flags & CY_BLE_LNS_LS_FLG_RT))
            {
                ls.rollTime++;
                pdu[ptr] = ls.rollTime;
                ptr += sizeof(ls.rollTime);
            }
            
            if(0u != (ls.flags & CY_BLE_LNS_LS_FLG_UTC))
            {
                Cy_BLE_Set16ByPtr(&pdu[ptr], ls.utcTime.year);
                pdu[ptr + 2u] = ls.utcTime.month;
                pdu[ptr + 3u] = ls.utcTime.day;
                pdu[ptr + 4u] = ls.utcTime.hours;
                pdu[ptr + 5u] = ls.utcTime.minutes;
                pdu[ptr + 6u] = ls.utcTime.seconds;
                ptr += 7u;
            }
            
            do
            {
                Cy_BLE_ProcessEvents();
            }
            while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);

            if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
            {
                apiResult = Cy_BLE_LNSS_SendNotification(connHandle, CY_BLE_LNS_LS, ptr, pdu);
                
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_LNSS_SendNotification API (L&S) Error: ");
                    PrintApiResult(apiResult);
                }
                else
                {
                    DBG_PRINTF("L&S Ntf: ");
                    for(i = 0; i < ptr; i++)
                    {
                        DBG_PRINTF("%2.2x ", pdu[i]);
                    }
                    DBG_PRINTF("\r\n");
                }
            }
        }
        
        time.seconds++;
        if(time.seconds >= 60u)
        {
            time.seconds = 0u;
            time.minutes++;
            if(time.minutes >= 60u)
            {
                time.minutes = 0u;
                time.hours++;
                if(time.hours >= 24u)
                {
                    time.hours = 0u;
                    time.day++;
                    /* Omit further data process because it is not critical for simulation example */
                }
            }
        }
        
        nv = cnv[lnsCount];
        nv.eaTime = time;
        
        
        apiResult = Cy_BLE_LNSS_GetCharacteristicDescriptor(connHandle, CY_BLE_LNS_NV, CY_BLE_LNS_CCCD,
                                                            CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
        
        if(((lnsFlag & NV_EN) ==  NV_EN) && (cccd == CY_BLE_CCCD_NOTIFICATION))
        {
            uint8_t pdu[sizeof(cy_stc_ble_lns_nv_t)];
            uint8_t ptr;
            
            /* Flags field is always first two bytes */
            Cy_BLE_Set16ByPtr(&pdu[0u], nv.flags);
            
            /* Bearing mandatory field is next */
            Cy_BLE_Set16ByPtr(&pdu[2u], nv.bearing);
            
            /* Heading mandatory field is next */
            Cy_BLE_Set16ByPtr(&pdu[4u], nv.heading);

            /* Next possible value is at 6th byte */
            ptr = 6u;
                
            /* If Remaining Distance Present flag is set */
            if(0u != (nv.flags & CY_BLE_LNS_NV_FLG_RD))
            {
                /* And set 2-byte Instantaneous Speed value */
                pdu[ptr] = (uint8_t) (nv.rDst & 0x000000FFu);
                pdu[ptr + 1] = (uint8_t) ((nv.rDst & 0x0000FF00u) >> 8u);
                pdu[ptr + 2] = (uint8_t) ((nv.rDst & 0x00FF0000u) >> 16u);
                ptr += 3; /* Remaining Distance size is actually 3 bytes */
            }
            
            if(0u != (nv.flags & CY_BLE_LNS_NV_FLG_RVD))
            {
                pdu[ptr] = (uint8_t) (nv.rvDst & 0x000000FFu);
                pdu[ptr + 1] = (uint8_t) ((nv.rvDst & 0x0000FF00u) >> 8u);
                pdu[ptr + 2] = (uint8_t) ((nv.rvDst & 0x00FF0000u) >> 16u);
                ptr += 3; /* Remaining Vertical Distance size is actually 3 bytes */
            }

            if(0u != (nv.flags & CY_BLE_LNS_NV_FLG_EAT))
            {
                Cy_BLE_Set16ByPtr(&pdu[ptr], nv.eaTime.year);
                pdu[ptr + 2u] = nv.eaTime.month;
                pdu[ptr + 3u] = nv.eaTime.day;
                pdu[ptr + 4u] = nv.eaTime.hours;
                pdu[ptr + 5u] = nv.eaTime.minutes;
                pdu[ptr + 6u] = nv.eaTime.seconds;
                ptr += 7u;
            }
            
            do
            {
                Cy_BLE_ProcessEvents();
            }
            while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);

            if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
            {
                apiResult = Cy_BLE_LNSS_SendNotification(connHandle, CY_BLE_LNS_NV, ptr, pdu);
                
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_LNSS_SendNotification API (Navigation) Error: ");
                    PrintApiResult(apiResult);
                }
                else
                {
                    DBG_PRINTF("Navigation Ntf: ");
                    for(i = 0; i < ptr; i++)
                    {
                        DBG_PRINTF("%2.2x ", pdu[i]);
                    }
                    DBG_PRINTF("\r\n");
                }
            }
        }
    }
}


/*******************************************************************************
* Function Name: LnsProcess
********************************************************************************
*
* Summary:
*   Processes the LNS functionality in the main loop.
*
*******************************************************************************/
void LnsProcess(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    uint16 cccd;
    uint32_t i;
    
    if(cp[2u] != 0u)
    {
        uint8_t cpSize = 3u;
        
        switch(cp[1u])
        {
            case CY_BLE_LNS_CP_OPC_SCV:
                break;
                
            case CY_BLE_LNS_CP_OPC_MLS:
                break;
                
            case CY_BLE_LNS_CP_OPC_NC:       
                break;
                
            case CY_BLE_LNS_CP_OPC_NRS:
                cp[3u] = CY_LO8(LNS_COUNT);
                cp[4u] = CY_HI8(LNS_COUNT);
                cpSize = 5u;
                break;
                
            case CY_BLE_LNS_CP_OPC_RNM:
                cp[3u] = 'N';
                cp[4u] = 'A';
                cp[5u] = 'M';
                cp[6u] = 'E';
                cpSize = 7u;
                break;
                
            case CY_BLE_LNS_CP_OPC_SR:
                break;
                
            case CY_BLE_LNS_CP_OPC_SFR:
                break;
                
            case CY_BLE_LNS_CP_OPC_SE:
                break;
                
            default:
                cp[2u] = CY_BLE_LNS_CP_RSP_UNSPRT_OPC;
                break;
        }
    
        /* Send CP response if indicator is enabled */
        
        /* Read CCCD for Location and Navigation Control Point characteristic */
        (void)Cy_BLE_LNSS_GetCharacteristicDescriptor(connHandle, CY_BLE_LNS_CP, CY_BLE_LNS_CCCD,
                                                                CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
        if(cccd == CY_BLE_CCCD_INDICATION)
        {
            do
            {
                Cy_BLE_ProcessEvents();
            }
            while(Cy_BLE_GATT_GetBusyStatus(connHandle.attId) == CY_BLE_STACK_STATE_BUSY);

            if(Cy_BLE_GetConnectionState(connHandle) >= CY_BLE_CONN_STATE_CONNECTED)
            {   
                if((apiResult = Cy_BLE_LNSS_SendIndication(connHandle, CY_BLE_LNS_CP, cpSize, cp)) != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF("Cy_BLE_LNSS_SendIndication API Error: 0x%x \r\n", apiResult);
                }
                else
                {
                    DBG_PRINTF("CP Ind:");
                    for(i = 0; i < cpSize; i++)
                    {
                        DBG_PRINTF(" %2.2x", cp[i]);
                    }
                    DBG_PRINTF("\r\n");
                }
            }
        }
        else 
        {
            DBG_PRINTF("CP Indication is disabled \r\n");
        }
        cp[2u] = 0u;
    }
}
    

/*******************************************************************************
* Function Name:  LnsGetFlag
********************************************************************************
*
* Summary:
*   Returns Lns flag
*
*******************************************************************************/
uint8_t LnsGetFlag(void)
{
    return(lnsFlag);   
}

/* [] END OF FILE */
