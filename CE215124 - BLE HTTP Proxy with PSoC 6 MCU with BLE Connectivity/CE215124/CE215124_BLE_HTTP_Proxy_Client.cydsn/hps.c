/*******************************************************************************
* File Name: hps.c
*
* Version: 1.0
*
* Description:
*  This file contains routines related to HTTP Proxy Service.
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
#include "hps.h"


/***************************************
*        Global Variables
***************************************/
static http_send_request_state_t    reqState   = STATE_FILL_URI;
static http_read_response_state_t   respState  = STATE_READ_ENTITY_BODY;
static cy_en_ble_hps_http_request_t hpsRequest;
static cy_en_ble_hps_http_request_t hpsLedState;
static hps_ntf_data_t               ntfStatusCode;
static bool                         isHpsWritePending = false;
static bool                         isHpsReadPending  = false;
static bool                         isSendRequestInitiated  = false;
static bool                         isReadResponseInitiated = false;
static bool                         isConfigConfirmed       = false;
static uint8_t reqHttpUri[HPS_HTTP_REQ_URI_SIZE] = "http://www.cypress.com/led";
static uint8_t reqHttpHeader[HPS_HTTP_GET_HEADER_SIZE] = 
                "GET /led.html HTTP/1.1\r\nHost: www.cypress.com\r\nAccept-Language: en-us\r\nConnection: Keep-Alive";
static uint8_t ledOnHttpBody[HPS_HTTP_LED_ON_SIZE] = "parameter=On";
static uint8_t ledOffHttpBody[HPS_HTTP_LED_OFF_SIZE] = "parameter=Off";


/*******************************************************************************
* Function Name: HpsInit
********************************************************************************
*
* Summary:
*   Initializes the HPS service.
*
*******************************************************************************/
void HpsInit(void)
{
    /* Register service specific callback function */
    Cy_BLE_HPS_RegisterAttrCallback(HpsCallBack);
    
    isConfigConfirmed = false;
}


/*******************************************************************************
* Function Name: HpsCallBack
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component
*  specific to HTTP Proxy Service.
*
* Parameters:
*  event - An HPS event.
*  eventParams - The data structure specific to an event received.
*
*******************************************************************************/
void HpsCallBack(uint32_t event, void *eventParam)
{
    uint8_t i;
    cy_stc_ble_hps_char_value_t * charValPtr;

    switch(event)
    {

    /****************************************
    *    HTTP Proxy Service Events
    ****************************************/

    /** HPS Server - Notification for HTTP Proxy Service Characteristic
        was enabled. The parameter of this event is a structure 
        of the cy_stc_ble_hps_char_value_t type.
    */
    case CY_BLE_EVT_HPSS_NOTIFICATION_ENABLED:
        DBG_PRINTF("CY_BLE_EVT_HPSS_NOTIFICATION_ENABLED\r\n");
        break;

    /** HPS Server - Notification for HTTP Proxy Service Characteristic
        was disabled. The parameter of this event is a structure 
        of the cy_stc_ble_hps_char_value_t type.
    */
    case CY_BLE_EVT_HPSS_NOTIFICATION_DISABLED:
        DBG_PRINTF("CY_BLE_EVT_HPSS_NOTIFICATION_DISABLED\r\n");
        break;

    /** HPS Server - Write Request for HTTP Proxy Service
        Characteristic was received. The parameter of this event is a structure of
        the cy_stc_ble_hps_char_value_t type.
    */    
    case CY_BLE_EVT_HPSS_WRITE_CHAR:
        DBG_PRINTF("CY_BLE_EVT_HPSS_CHAR_WRITE\r\n");
        break;

    /****************************************************
    *               HPS Client events
    * These events are not active for the Server device
    * operation. They are added as a template for the
    * Client mode operation to ease the user experience
    * with HTTP Proxy Service.
    ****************************************************/
    /** HPS Client - HTTP Proxy Service Characteristic
        Notification was received. The parameter of this event
        is a structure of the cy_stc_ble_hps_char_value_t type.
    */
    case CY_BLE_EVT_HPSC_NOTIFICATION:
        DBG_PRINTF("CY_BLE_EVT_HPSC_NOTIFICATION\t\t");
        charValPtr = (cy_stc_ble_hps_char_value_t *) eventParam;
        HpsPrintCharacteristic(charValPtr->charIndex);
        ntfStatusCode.statusCode = (((uint16_t) charValPtr->value->val[1u]) << 8u) |
                                    ((uint16_t) charValPtr->value->val[0u]);
        ntfStatusCode.dataStatus = charValPtr->value->val[2u];
        DBG_PRINTF("\r\n\t\t\t\t\tStatus code: %d ", ntfStatusCode.statusCode);
        HpsPrintHttpStatus(ntfStatusCode.statusCode);
        DBG_PRINTF("\r\n");
        isSendRequestInitiated = false;
        isReadResponseInitiated = true;
        respState = STATE_READ_HTTP_HEADERS;
        break;

    /** HPS Client - a read response for the read request of HTTP Proxy
        Service Characteristic value. The parameter of this event
        is a structure of the cy_stc_ble_hps_char_value_t type.
    */
    case CY_BLE_EVT_HPSC_READ_CHAR_RESPONSE:
        DBG_PRINTF("CY_BLE_EVT_HPSC_READ_CHAR_RESPONSE\t");
        charValPtr = (cy_stc_ble_hps_char_value_t *) eventParam;
        HpsPrintCharacteristic(charValPtr->charIndex);
        DBG_PRINTF("\r\nChar data:");
        for(i = 0u; i < charValPtr->value->len; i++)
        {
            DBG_PRINTF("%c", charValPtr->value->val[i]);
        }
        DBG_PRINTF("\r\n");
        
        if(isHpsReadPending == true)
        {
            if(charValPtr->charIndex == CY_BLE_HPS_HTTP_ENTITY_BODY)
            {
                isReadResponseInitiated = false;
                if((hpsRequest == CY_BLE_HPS_HTTP_GET) && (ntfStatusCode.statusCode == HPS_HTTP_OK))
                {
                    DBG_PRINTF("\r\nPress '1' to send POST request to turn on the LED on the peer device or ");
                    DBG_PRINTF("press '0' to send POST request to turn off the LED on the peer device.\r\n");
                }
            }
            else
            {
                /* Received read response for burst of HTTP response. Update internal
                * variables to process the next burst of response.
                */
                respState++;
            }
            isHpsReadPending = false;
        }
        break;

    /** HPS Client - a read response for the read request of the HTTP Proxy
        Service Characteristic descriptor read request. The 
        parameter of this event is a structure of
        the cy_stc_ble_hps_descr_value_t type.
    */
    case CY_BLE_EVT_HPSC_READ_DESCR_RESPONSE:
        DBG_PRINTF("CY_BLE_EVT_HPSC_READ_DESCR_RESPONSE\r\n");
        break;
        
    /** HPS Client - a write response for the write request of the HPS 
        Service Characteristic value. The parameter of this event
        is a structure of the cy_stc_ble_hps_char_value_t type.
    */
    case CY_BLE_EVT_HPSC_WRITE_CHAR_RESPONSE:
        DBG_PRINTF("CY_BLE_EVT_HPSC_WRITE_CHAR_RESPONSE\t");
        charValPtr = (cy_stc_ble_hps_char_value_t *) eventParam;
        HpsPrintCharacteristic(charValPtr->charIndex);
        DBG_PRINTF("\r\n");

        if(isHpsWritePending == true)
        {
            /* Received write response for burst of HTTP request. Update internal 
            * variables to process the next burst of request.
            */
            isHpsWritePending = false;
            reqState++;
        }
        break;

    /** HPS Client - a write response for the write request of the HTTP Proxy
        Service Characteristic Configuration descriptor value.
        The parameter of this event is a structure of 
        the cy_stc_ble_hps_descr_value_t type.
    */
    case CY_BLE_EVT_HPSC_WRITE_DESCR_RESPONSE:
        DBG_PRINTF("CY_BLE_EVT_HPSC_WRITE_DESCR_RESPONSE\r\n");
        isConfigConfirmed = true;
        break;

    default:
        DBG_PRINTF("Unrecognised HPS event.\r\n");
        break;
    }
}


/*******************************************************************************
* Function Name:  HpsIsConfigConfirmed(void)
********************************************************************************
*
* Summary:
*  Returns isConfigConfirmed status
*
*******************************************************************************/
bool HpsIsConfigConfirmed(void)
{
    return(isConfigConfirmed);
}

/*******************************************************************************
* Function Name: HpsPrintCharacteristic
********************************************************************************
*
* Summary:
*  Prints the characteristic name on the UART terminal.
*
* Parameters:  
*  charIndex: Characteristic index.
*
*******************************************************************************/
void HpsPrintCharacteristic(cy_en_ble_hps_char_index_t charIndex)
{
    DBG_PRINTF("Characteristic: ");    
    
    switch(charIndex)
    {
    case CY_BLE_HPS_URI:
        DBG_PRINTF("CY_BLE_HPS_URI\t\t\t");
        break;
    case CY_BLE_HPS_HTTP_HEADERS:
        DBG_PRINTF("CY_BLE_HPS_HTTP_HEADERS\t\t");
        break;
    case CY_BLE_HPS_HTTP_ENTITY_BODY:
        DBG_PRINTF("CY_BLE_HPS_HTTP_ENTITY_BODY\t");
        break;
    case CY_BLE_HPS_HTTP_CP:
        DBG_PRINTF("CY_BLE_HPS_HTTP_CP \t\t");
        break;
    case CY_BLE_HPS_HTTP_STATUS_CODE:
        DBG_PRINTF("CY_BLE_HPS_HTTP_STATUS_CODE\t");
        break;
    case CY_BLE_HPS_HTTPS_SECURITY:
        DBG_PRINTF("CY_BLE_HPS_HTTPS_SECURITY\t");
        break;
    default:
        DBG_PRINTF("Unknown\t\t\t");
        break;
    }
}


/*******************************************************************************
* Function Name: HpsSendRequest
********************************************************************************
*
* Summary:
*  Initiates an HTTP request to HPS Server.
*
* Parameters:  
*  request:  An HPS request.
*  reqParam: The request parameter. Used only for a POST request (0 - turn off LED on 
*            HPS server, 0 - turn on LED on HPS server)
*
*******************************************************************************/
void HpsSendRequest(cy_en_ble_hps_http_request_t request, uint8_t reqParam)
{
    if(isSendRequestInitiated == false)
    {
        hpsRequest = request;
        isSendRequestInitiated = true;
        hpsLedState = (cy_en_ble_hps_http_request_t)reqParam;
        reqState = STATE_FILL_URI;
    }
    else
    {
        DBG_PRINTF("Can't send the request as there is an active request processing.\r\n");
    }
}


/*******************************************************************************
* Function Name: HpsHandleRequests
********************************************************************************
*
* Summary:
*  Handles sending an HTTP requests in bursts where an URI, HTTP
*  Headers, HTTP Entity Body and HTTP Control Point Characteristics are written.
*
*******************************************************************************/
void HpsHandleRequests(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_MAX;

    /* If there was request HPS initiated from the user and there is no
    * active write process, then write a next characteristic required 
    * by the HPS spec to create a complete HTTP request.
    */
    if((isSendRequestInitiated == true) && (isHpsWritePending == false))
    {   
        switch(reqState)
        {
        case STATE_FILL_URI:
            apiResult =
                Cy_BLE_HPSC_SetCharacteristicValue(connHandle, CY_BLE_HPS_URI, HPS_HTTP_REQ_URI_SIZE, reqHttpUri);

            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_HPSC_SetCharacteristicValue(CY_BLE_HPS_URI) failure. Error code: %d \r\n", apiResult);
            }
            break;
            
        case STATE_FILL_HTTP_HEADERS:
            apiResult =
                Cy_BLE_HPSC_SetCharacteristicValue(connHandle,
                                                     CY_BLE_HPS_HTTP_HEADERS,
                                                     HPS_HTTP_GET_HEADER_SIZE,
                                                     reqHttpHeader);
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_HPSC_SetCharacteristicValue(CY_BLE_HPS_HTTP_HEADERS) failure. Error code: %d \r\n",
                           apiResult);
            }
            break;
            
        case STATE_FILL_ENTITY_BODY:
            if(hpsLedState == 0u)
            {
                apiResult =  Cy_BLE_HPSC_SetCharacteristicValue(connHandle,
                                                                 CY_BLE_HPS_HTTP_ENTITY_BODY,
                                                                 HPS_HTTP_LED_OFF_SIZE,
                                                                 ledOffHttpBody);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF(
                        "Cy_BLE_HPSC_SetCharacteristicValue(CY_BLE_HPS_HTTP_ENTITY_BODY) failure. Error code: %d \r\n",
                        apiResult);
                }
            }
            else
            {
                apiResult = Cy_BLE_HPSC_SetCharacteristicValue(connHandle,
                                                                CY_BLE_HPS_HTTP_ENTITY_BODY,
                                                                HPS_HTTP_LED_ON_SIZE,
                                                                ledOnHttpBody);
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DBG_PRINTF(
                        "Cy_BLE_HPSC_SetCharacteristicValue(CY_BLE_HPS_HTTP_ENTITY_BODY) failure. Error code: %d \r\n",
                        apiResult);
                }
            }
            break;
            
        case STATE_SEND_REQUEST:
            apiResult =
                Cy_BLE_HPSC_SetCharacteristicValue(connHandle, CY_BLE_HPS_HTTP_CP, 1u, (uint8_t *) &hpsRequest);
                    
            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_HPSC_SetCharacteristicValue(CY_BLE_HPS_HTTP_CP) failure. Error code: %d \r\n",
                           apiResult);
            }
            break;
            
        default:
            /* Invalid state */
            break;
        }
        if(apiResult == CY_BLE_SUCCESS)
        {
            /* Indicate that write process is pending */
            isHpsWritePending = true;
        }
    }
}


/*******************************************************************************
* Function Name: HpsReadResponse
********************************************************************************
*
* Summary:
*  Reads the HPS response (HTTP headers and HTTP entity Body) from a peer device.
*
*******************************************************************************/
void HpsReadResponse(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;

    /* If there was HPS read initiated from the user and there is no
    * active read process, then read a next characteristic.
    */
    if((isReadResponseInitiated == true) && (isHpsReadPending == false))
    {
        switch(respState)
        {
        case STATE_READ_HTTP_HEADERS:
            apiResult = Cy_BLE_HPSC_GetCharacteristicValue(connHandle, CY_BLE_HPS_HTTP_HEADERS);

            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_HPSC_GetCharacteristicValue(CY_BLE_HPS_HTTP_HEADERS) failure. Error code: %d \r\n",
                    apiResult);
            }
            break;

        case STATE_READ_ENTITY_BODY:
            apiResult = Cy_BLE_HPSC_GetCharacteristicValue(connHandle, CY_BLE_HPS_HTTP_ENTITY_BODY);

            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Cy_BLE_HPSC_GetCharacteristicValue(CY_BLE_HPS_HTTP_ENTITY_BODY) failure. Error code: %d \r\n",
                    apiResult);
            }
            break;

        default:
            /* Invalid state */
            break;
        }

        /* Indicate that read process is pending */
        isHpsReadPending = true;
    }
}


/*******************************************************************************
* Function Name: HpsPrintHttpStatus
********************************************************************************
*
* Summary:
*  Prints a group name of a received HTTP Status Code.
*
* Parameters:  
*  statusCode: The status code of HTTP request execution.
*
*******************************************************************************/
void HpsPrintHttpStatus(uint16_t statusCode)
{
    switch(statusCode / 100u)
    {
    case HPS_HTTP_STATUS_INFORMAL:
        DBG_PRINTF(" \"Informal\" ");
        break;
    case HPS_HTTP_STATUS_SUCCESS:
        DBG_PRINTF(" \"Success\" ");
        break;
    case HPS_HTTP_STATUS_REDIRECTION:
        DBG_PRINTF(" \"Redirection\" ");
        break;
    case HPS_HTTP_STATUS_CLIENT_ERROR:
        DBG_PRINTF(" \"Client Error\" ");
        break;
    case HPS_HTTP_STATUS_SERVER_ERROR:
        DBG_PRINTF(" \"Server Error\" ");
        break;
    default:
        /* Invalid group */
        break;
    }
}


/* [] END OF FILE */
