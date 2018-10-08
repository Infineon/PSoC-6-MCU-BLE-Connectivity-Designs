/*******************************************************************************
* File Name: ipss.c
*
* Version: 1.0
*
* Description:
*  This file contains IPS service related code.
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
#include "ipss.h"

static bool updateAdvScanDataCompeteEventFlag = false;
static bool switchAdvModeFlag = false;


/******************************************************************************
* Function Name: IpsInit
*******************************************************************************
*
* Summary:
*    Registers the IPS CallBack.
*
******************************************************************************/
void IpsInit(void)
{
    Cy_BLE_IPS_RegisterAttrCallback(IpsCallBack);
}

/*******************************************************************************
* Function Name: IpsCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service-specific events from
*   the Indoor Positioning Service.
*
* Parameters:
*  event       - the event code
*  *eventParam - the event parameters
*
********************************************************************************/
void IpsCallBack(uint32_t event, void* eventParam)
{
    uint8_t i;
    
    if(0u != eventParam)
    {
        /* This dummy operation is to avoid warning about unused eventParam */
    }

    switch(event)
    {
        case CY_BLE_EVT_IPSS_WRITE_CHAR:
            
            DBG_PRINTF("Write characteristic ");
            
            IpsPrintCharName(((cy_stc_ble_ips_char_value_t *)eventParam)->charIndex);
             
            DBG_PRINTF(". Value - "); 
            if(((cy_stc_ble_ips_char_value_t *)eventParam)->charIndex == CY_BLE_IPS_LOCATION_NAME)
            {
                for(i = 0; i < ((cy_stc_ble_ips_char_value_t*)eventParam)->value->len; i++)
                {
                    if(((cy_stc_ble_ips_char_value_t*)eventParam)->value->val[i] != 0u)
                    {
                       DBG_PRINTF("%2.2x",((cy_stc_ble_ips_char_value_t*)eventParam)->value->val[i]);
                    }
                }
            }
            else
            {   
                uint32_t locVal = 0; 
                for(i = 0; i < ((cy_stc_ble_ips_char_value_t*)eventParam)->value->len; i++)
                {
                    DBG_PRINTF("%2.2x ", ((cy_stc_ble_ips_char_value_t*)eventParam)->value->val[i]);
                    locVal <<= 8;
                    locVal += ((cy_stc_ble_ips_char_value_t*)eventParam)->value->val[((cy_stc_ble_ips_char_value_t*)eventParam)->value->len-i-1];
                }
                DBG_PRINTF("(%ld).",locVal);
                
            }
            DBG_PRINTF("   Len - %d.\r\n", ((cy_stc_ble_ips_char_value_t*)eventParam)->value->len);
            break;
           
        default:
            DBG_PRINTF("Unknown IPS event: %lx \r\n", event);
            break;
    }
}

/*******************************************************************************
* Function Name: IpsSimulateCoordinates
********************************************************************************
*
* Summary:
*   Function simulates Latitude and Longitude. 
*
*******************************************************************************/
void IpsSimulateCoordinates(void)
{
    cy_en_ble_api_result_t apiResult;
    static uint8_t simulatingMode = MODE_LATITUDE_INC;
    int32_t latitudeValue;
    int32_t longitudeValue;
    uint32_t timeout;
    
    apiResult = Cy_BLE_IPSS_GetCharacteristicValue(CY_BLE_IPS_LATITUDE, sizeof(latitudeValue),(uint8_t*)&latitudeValue);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("\r\nCy_BLE_IPSS_GetCharacteristicValue API Error: ");
        PrintApiResult(apiResult);
    }
    
    apiResult = Cy_BLE_IPSS_GetCharacteristicValue(CY_BLE_IPS_LONGITUDE, sizeof(longitudeValue),(uint8_t*)&longitudeValue);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("\r\nCy_BLE_IPSS_GetCharacteristicValue API Error: ");
        PrintApiResult(apiResult);
    }
    
    switch(simulatingMode)
    {
        case MODE_LATITUDE_INC:
            latitudeValue += 100;
            if ( latitudeValue - CYUA_LATITUDE == 2000u)
            {   
                simulatingMode = MODE_LONGITUDE_INC;
            }
            break;
            
        case MODE_LONGITUDE_INC:
            longitudeValue += 100;
            if ( longitudeValue - CYUA_LONGITUDE == 2000u)
            {   
                simulatingMode = MODE_LATITUDE_DEC;
            }
            break;
            
        case MODE_LATITUDE_DEC:
            latitudeValue -= 100;
            if ( latitudeValue - CYUA_LATITUDE == 0u)
            {   
                simulatingMode = MODE_LONGITUDE_DEC;
            }
            break;
            
        case MODE_LONGITUDE_DEC:
            longitudeValue -= 100;
            if ( longitudeValue - CYUA_LONGITUDE == 0u)
            {   
                simulatingMode = MODE_LATITUDE_INC;
            }
            break;
            
        default:
            break;
    
    }
    
    /* See Indoor Positioning Specification */
    DBG_PRINTF("Latitude - %f\t Longitude - %f\r\n",  
        (double)latitudeValue * 90.0 / 2147483648.0,
        (double)longitudeValue * 180.0 / 2147483648.0);
    
    
    apiResult = Cy_BLE_IPSS_SetCharacteristicValue(CY_BLE_IPS_LATITUDE, sizeof(latitudeValue),(uint8_t*)&latitudeValue);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("\r\nCy_BLE_IPSS_SetCharacteristicValue API Error: ");
        PrintApiResult(apiResult);
    }
    
    /* Wait while Cy_BLE_IPSS_SetCharacteristicValue (updating Adv data) finish  */
    updateAdvScanDataCompeteEventFlag = false;
    timeout = 0xFFFF;
    do
    {
        Cy_BLE_ProcessEvents();
    }
    while((--timeout != 0u) && (updateAdvScanDataCompeteEventFlag == false));
    
    apiResult = Cy_BLE_IPSS_SetCharacteristicValue(CY_BLE_IPS_LONGITUDE, sizeof(longitudeValue),(uint8_t*)&longitudeValue);
    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("\r\nCy_BLE_IPSS_SetCharacteristicValue API Error: ");
        PrintApiResult(apiResult);
    }
    
    /* Wait while Cy_BLE_IPSS_SetCharacteristicValue (updating Adv data) finish  */
    updateAdvScanDataCompeteEventFlag = false;
    timeout = 0xFFFF;
    do
    {
        Cy_BLE_ProcessEvents();
         
    }
    while((--timeout != 0u) && (updateAdvScanDataCompeteEventFlag == false));
}

/*******************************************************************************
* Function Name: IpsStartAdvertisement
********************************************************************************
*
* Summary:
*   Function initiates the advertisement procedure. 
*
*******************************************************************************/
void IpsStartAdvertisement(void)
{
    /*  
     *  Advertisement configurations:
     *   - CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX   - CONNECTABLE ADVERTISEMENT
     *   - CY_BLE_PERIPHERAL_CONFIGURATION_1_INDEX   - NON CONNECTABLE ADVERTISEMENT
     */
    cy_en_ble_api_result_t apiResult;
    
    if(cy_ble_advIndex == CY_BLE_PERIPHERAL_CONFIGURATION_1_INDEX) 
    {
        /* Start Advertise in connectable mode: CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX*/
        apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, 
                                                   CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("Cy_BLE_GAPP_StartAdvertisement API Error: 0x%x \r\n", apiResult);
        }

        DBG_PRINTF("Connectable mode.\r\n");
    }
    else
    {
        /* Start Advertise in non connectable mode: CY_BLE_PERIPHERAL_CONFIGURATION_1_INDEX*/
        apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST,
                                                   CY_BLE_PERIPHERAL_CONFIGURATION_1_INDEX);
        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("Cy_BLE_GAPP_StartAdvertisement API Error: 0x%x \r\n", apiResult);
        }
        DBG_PRINTF("Non connectable mode.   \r\n");
    }
    
    switchAdvModeFlag = false;
}
/*******************************************************************************
* Function Name: IpsPrintCharName
********************************************************************************
*
* Summary:
*   Function prints the name of Indoor Positioning Service characteristic.
*
* Parameters:  
*   charIndex - index of Indoor Positioning characteristic
*
*******************************************************************************/
void IpsPrintCharName(cy_en_ble_ips_char_index_t charIndex)
{
    switch(charIndex)
    {
        case CY_BLE_IPS_INDOOR_POSITINING_CONFIG:
            DBG_PRINTF("Ind Pos Config");
            break;

        case CY_BLE_IPS_LATITUDE:
            DBG_PRINTF("Latitude");                /**< WGS84 North coordinate of the device. */
            break;            
            
        case CY_BLE_IPS_LONGITUDE:
            DBG_PRINTF("Longitude");               /**< WGS84 East coordinate of the device. */
            break;       
            
        case CY_BLE_IPS_LOCAL_NORTH_COORDINATE:
            DBG_PRINTF("North Coordinate");         /**< North coordinate of the device using local coordinate system. */
            break;    
            
        case CY_BLE_IPS_LOCAL_EAST_COORDINATE:
            DBG_PRINTF("East Coordinate");          /**< East coordinate of the device using local coordinate system. */
            break;  
            
        case CY_BLE_IPS_FLOOR_NUMBER:
            DBG_PRINTF("Floor Number");             /**< Describes in which floor the device is installed in. */
            break;      
            
        case CY_BLE_IPS_ALTITUDE:
            DBG_PRINTF("Altitude");                 /**< Altitude of the device. */
            break;   
            
        case CY_BLE_IPS_UNCERTAINTY:
            DBG_PRINTF("Uncertainty");              /**< Uncertainty of the location information the device exposes. */
            break; 
            
        case CY_BLE_IPS_LOCATION_NAME:
            DBG_PRINTF("Location Name");            /**< Name of the location the device is installed in. */
            break;                    
            
        default:
            DBG_PRINTF("Unknown IPS");
            break;
    }
}


/*******************************************************************************
* Function Name: IpsSetUpdateAdvDataCompeteEventFlag
********************************************************************************
*
* Summary:
*   This function sets updateAdvScanDataCompeteEventFlag value.
*
*******************************************************************************/
void IpsSetUpdateAdvDataCompeteEventFlag(void)
{
    updateAdvScanDataCompeteEventFlag = true;
}


/*******************************************************************************
* Function Name: IpsGetSwitchAdvModeFlag
********************************************************************************
*
* Summary:
*   This function returns switchAdvModeFlag value.
*
* Returns:
*  bool - switchAdvModeFlag value.
*
*******************************************************************************/
bool IpsGetSwitchAdvModeFlag(void)
{
    return(switchAdvModeFlag);
}


/*******************************************************************************
* Function Name: IpsSetSwitchAdvModeFlag
********************************************************************************
*
* Summary:
*   This function sets switchAdvModeFlag value.
*
* Parameters:  
*   switchAdvModeFlagValue - new value of switchAdvModeFlag
*
*******************************************************************************/
void IpsSetSwitchAdvModeFlag(bool switchAdvModeFlagValue)
{
    switchAdvModeFlag = switchAdvModeFlagValue;
}

/* [] END OF FILE */
