/*******************************************************************************
* File Name: hps.c
*
* Version: 1.0
*
* Description:
*  This file contains routines related to HTTP Proxy Service.
*
* 
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"
#include "user_interface.h"
#include "hps.h"


/***************************************
*        Global Variables
***************************************/
static hps_ntf_data_t    hpsNotificationData;
static uint8_t           fullReqInfo;
static uint8_t           supportedHttpUri[HPS_HTTP_SUPPORTED_URI_SIZE] = "http://www.cypress.com/led";
static uint8_t           postHttpBody1[HPS_HTTP_POST_BODY1_SIZE] = "parameter=On";
static uint8_t           postHttpBody2[HPS_HTTP_POST_BODY2_SIZE] = "parameter=Off";
static uint8_t           responseHttpHeader[HPS_HTTP_RESPONSE_HEADER_SIZE] = 
"Server: Apache/2.2.14 (Win32)\r\nContent-Type: text/html; charset=iso-8859-1\r\nConnection: Closed";
static uint8_t           ledOffHttpBody[HPS_HTTP_LED_OFF_BODY_SIZE] = 
"<html>\r\n<head>\r\n<title>LED State page</title>\r\n</head>\r\n<body>\r\n<p>LED is off.</p>\r\n</body>\r\n</html>";
static uint8_t           ledOnHttpBody[HPS_HTTP_LED_ON_BODY_SIZE] = 
"<html>\r\n<head>\r\n<title>LED State page</title>\r\n</head>\r\n<body>\r\n<p>LED is on.</p>\r\n</body>\r\n</html>";
static uint8_t           otherErrorHttpBody[HPS_HTTP_OTHER_ERROR_BODY_SIZE] = 
"<html>\r\n<head>\r\n<title>Error</title>\r\n</head>\r\n<body>\r\n<h1>An error occurred</h1>\r\n<p>See Status code for more information.</p>\r\n</body>\r\n</html>";

static bool              hpsConnLedState = LED_OFF;
static bool              hpsIsRequestProcessingPending = false;

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
*  event       - An HPS event.
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

    /*  HPS Server - Notification for HTTP Proxy Service Characteristic
        was enabled. The parameter of this event is a structure 
        of the cy_stc_ble_hps_char_value_t type.
    */
    case CY_BLE_EVT_HPSS_NOTIFICATION_ENABLED:
        DBG_PRINTF("CY_BLE_EVT_HPSS_NOTIFICATION_ENABLED\r\n");
        break;

    /*  HPS Server - Notification for HTTP Proxy Service Characteristic
        was disabled. The parameter of this event is a structure 
        of the cy_stc_ble_hps_char_value_t type.
    */
    case CY_BLE_EVT_HPSS_NOTIFICATION_DISABLED:
        DBG_PRINTF("CY_BLE_EVT_HPSS_NOTIFICATION_DISABLED\r\n");
        break;

    /*  HPS Server - A write request for HTTP Proxy Service
        Characteristic was received. The parameter of this event is a structure of
        the cy_stc_ble_hps_char_value_t type.
    */    
    case CY_BLE_EVT_HPSS_WRITE_CHAR:
        DBG_PRINTF("CY_BLE_EVT_HPSS_CHAR_WRITE\t");
        charValPtr = (cy_stc_ble_hps_char_value_t *) eventParam;
        HpsPrintCharacteristic(charValPtr->charIndex);
        DBG_PRINTF("\r\n");
        switch (charValPtr->charIndex)
        {
            case CY_BLE_HPS_URI:
            case CY_BLE_HPS_HTTP_HEADERS:
            case CY_BLE_HPS_HTTP_ENTITY_BODY:

                /* HPS Spec 3.4.1.: To verify "Invalid request" response generation.
                * It should be generated when any of CY_BLE_HPS_URI, CY_BLE_HPS_HTTP_HEADERS
                * and CY_BLE_HPS_HTTP_ENTITY_BODY are not set prior the HTTP request.
                */
                fullReqInfo |= 1u << charValPtr->charIndex;
                DBG_PRINTF("Char data:");
                for(i = 0u; i < charValPtr->value->len; i++)
                {
                    DBG_PRINTF("%c", charValPtr->value->val[i]);
                }
                DBG_PRINTF("\r\n\r\n");
                break;
                
            case CY_BLE_HPS_HTTP_CP:
                /* Display request type */
                switch (charValPtr->value->val[0u])
                {
                case CY_BLE_HPS_HTTP_GET:
                    DBG_PRINTF("HTTP GET request\r\n");
                    break;
                case CY_BLE_HPS_HTTP_HEAD:
                    DBG_PRINTF("HTTP HEAD request\r\n");
                    break;
                case CY_BLE_HPS_HTTP_POST:
                    DBG_PRINTF("HTTP POST request\r\n");
                    break;
                case CY_BLE_HPS_HTTP_PUT:
                    DBG_PRINTF("HTTP PUT request\r\n");
                    break;
                case CY_BLE_HPS_HTTP_DELETE:
                    DBG_PRINTF("HTTP DELETE request\r\n");
                    break;
                case CY_BLE_HPS_HTTPS_GET:
                    DBG_PRINTF("HTTPS GET request\r\n");
                    break;
                case CY_BLE_HPS_HTTPS_HEAD:
                    DBG_PRINTF("HTTPS HEAD request\r\n");
                    break;
                case CY_BLE_HPS_HTTPS_POST:
                    DBG_PRINTF("HTTPS POST request\r\n");
                    break;
                case CY_BLE_HPS_HTTPS_PUT:
                    DBG_PRINTF("HTTPS PUT request\r\n");
                    break;
                case CY_BLE_HPS_HTTPS_DELETE:
                    DBG_PRINTF("HTTPS DELETE request\r\n");
                    break;
                case CY_BLE_HPS_HTTP_REQ_CANCEL:
                    DBG_PRINTF("HTTP request Cancel \r\n");
                    break;
                default:
                    DBG_PRINTF("Unknown request \r\n");
                    break;
                }
                
                /* 'fullReqInfo' contains a bit field that contains info if
                * CY_BLE_HPS_URI, CY_BLE_HPS_HTTP_HEADERS and CY_BLE_HPS_HTTP_ENTITY_BODY
                * were set prior to receiving an HPS request.
                */
                if((fullReqInfo && HPS_HTTP_UHB_MASK) == HPS_HTTP_UHB_MASK)
                {
                    HpsHandleHttpRequest((cy_en_ble_hps_http_request_t) charValPtr->value->val[0u]);
                }
                else
                {
                    charValPtr->gattErrorCode = CY_BLE_GATTS_ERR_HPS_INVALID_REQUEST;
                }
                break;
            default:
                DBG_PRINTF("Unknown characteristic \r\n");
                break;
        }
        break;

    /****************************************************
    *               HPS Client events
    * These events are not active for the Server device
    * operation. They are added as a template for the
    * Client mode operation to ease the user experience
    * with HTTP Proxy Service.
    ****************************************************/
    /*  HPS Client - HTTP Proxy Service Characteristic
        Notification was received. The parameter of this event
        is a structure of the cy_stc_ble_hps_char_value_t type.
    */
    case CY_BLE_EVT_HPSC_NOTIFICATION:
        DBG_PRINTF("CY_BLE_EVT_HPSC_NOTIFICATION\r\n");
        break;

    /*  HPS Client - A read response for the read request of the HTTP Proxy 
        Service Characteristic value. The parameter of this event
        is a structure of the cy_stc_ble_hps_char_value_t type.
    */
    case CY_BLE_EVT_HPSC_READ_CHAR_RESPONSE:
        DBG_PRINTF("CY_BLE_EVT_HPSC_READ_CHAR_RESPONSE\r\n");
        break;

    /*  HPS Client - A read response for read request of the HTTP Proxy
        Service Characteristic descriptor read request. The 
        parameter of this event is a structure of
        the cy_stc_ble_hps_descr_value_t type.
    */
    case CY_BLE_EVT_HPSC_READ_DESCR_RESPONSE:
        DBG_PRINTF("CY_BLE_EVT_HPSC_READ_DESCR_RESPONSE\r\n");
        break;

    /*  HPS Client - A write response for the write request of the HTTP Proxy
        Service Characteristic Configuration descriptor value.
        The parameter of this event is a structure of 
        the cy_stc_ble_hps_descr_value_t type.
    */
    case CY_BLE_EVT_HPSC_WRITE_DESCR_RESPONSE:
        DBG_PRINTF("CY_BLE_EVT_HPSC_WRITE_DESCR_RESPONSE\r\n");
        break;

    default:
        DBG_PRINTF("Unrecognized HPS event.\r\n");
        break;
    }
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
        DBG_PRINTF("CY_BLE_HPS_URI ");
        break;
    case CY_BLE_HPS_HTTP_HEADERS:
        DBG_PRINTF("CY_BLE_HPS_HTTP_HEADERS ");
        break;
    case CY_BLE_HPS_HTTP_ENTITY_BODY:
        DBG_PRINTF("CY_BLE_HPS_HTTP_ENTITY_BODY ");
        break;
    case CY_BLE_HPS_HTTP_CP:
        DBG_PRINTF("CY_BLE_HPS_HTTP_CP ");
        break;
    case CY_BLE_HPS_HTTP_STATUS_CODE:
        DBG_PRINTF("CY_BLE_HPS_HTTP_STATUS_CODE ");
        break;
    case CY_BLE_HPS_HTTPS_SECURITY:
        DBG_PRINTF("CY_BLE_HPS_HTTPS_SECURITY ");
        break;
    default:
        DBG_PRINTF("Unknown");
        break;
    }
}


/*******************************************************************************
* Function Name: HpsHandleHttpRequest
********************************************************************************
*
* Summary:
*  Handles HTTP requests. 
*
* Parameters:
*  request: An HTTP request received from the server.
*
*******************************************************************************/
void HpsHandleHttpRequest(cy_en_ble_hps_http_request_t request)
{
    cy_en_ble_api_result_t apiResult;

    switch (request)
    {
    case CY_BLE_HPS_HTTP_GET:
    case CY_BLE_HPS_HTTP_POST:
        /* Process valid requests */
        HpsProcessRequest(request);
        break;
    case CY_BLE_HPS_HTTP_HEAD:
    case CY_BLE_HPS_HTTP_PUT:
    case CY_BLE_HPS_HTTP_DELETE:
    case CY_BLE_HPS_HTTPS_GET:
    case CY_BLE_HPS_HTTPS_HEAD:
    case CY_BLE_HPS_HTTPS_POST:
    case CY_BLE_HPS_HTTPS_PUT:
    case CY_BLE_HPS_HTTPS_DELETE:
    case CY_BLE_HPS_HTTP_REQ_CANCEL:
    default:
        hpsNotificationData.dataStatus = CY_BLE_HPS_HTTP_HEADERS_RECEIVED | CY_BLE_HPS_HTTP_BODY_RECEIVED;;
        hpsNotificationData.statusCode = HPS_HTTP_BAD_REQUEST;

        /* Fill HTTP Headers Characteristic */
        apiResult = Cy_BLE_HPSS_SetCharacteristicValue(CY_BLE_HPS_HTTP_HEADERS, 
                                                        HPS_HTTP_RESPONSE_HEADER_SIZE,
                                                        responseHttpHeader);

        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("Can't set HTTP Headers Characteristic. Error code: %x\r\n", apiResult);
        }

        /* Fill HTTP Entity Body Characteristic */
        apiResult = Cy_BLE_HPSS_SetCharacteristicValue(CY_BLE_HPS_HTTP_ENTITY_BODY, 
                                                         HPS_HTTP_OTHER_ERROR_BODY_SIZE,
                                                         otherErrorHttpBody);

        if(apiResult != CY_BLE_SUCCESS)
        {
            DBG_PRINTF("Can't set HTTP Entity Body Characteristic. Error code: %x\r\n", apiResult);
        }
        break;
    }

    /* Set flag so that notification can be sent from main loop */
    hpsIsRequestProcessingPending = true;
}


/*******************************************************************************
* Function Name: HpsSendHpsNotification
********************************************************************************
*
* Summary:
*  Sends the notification to the remote Client.
*
*******************************************************************************/
void HpsSendNotification(cy_stc_ble_conn_handle_t connHandle)
{
    cy_en_ble_api_result_t apiResult;
    uint16_t cccd = CY_BLE_CCCD_DEFAULT;
    uint8_t pData[3u];
    uint8_t currLength = 0u;
    
    
    pData[currLength++] = CY_LO8(hpsNotificationData.statusCode);
    pData[currLength++] = CY_HI8(hpsNotificationData.statusCode);
    pData[currLength] = hpsNotificationData.dataStatus;

    apiResult = Cy_BLE_HPSS_GetCharacteristicDescriptor(connHandle, CY_BLE_HPS_HTTP_STATUS_CODE, 
                                                        CY_BLE_HPS_CCCD, CY_BLE_CCCD_LEN, (uint8_t*)&cccd);
   
    if((apiResult == CY_BLE_SUCCESS) && (cccd != 0u))
    {
        apiResult = Cy_BLE_HPSS_SendNotification(connHandle, CY_BLE_HPS_HTTP_STATUS_CODE, HPS_HTTP_NOTIFICATION_SIZE,
                                                 pData);

        if(apiResult == CY_BLE_SUCCESS)
        {
            DBG_PRINTF("Notification was sent successfully \r\n");
        }
        else
        {
            DBG_PRINTF("Notification API Error: %d \r\n", apiResult);
        }
    }
    else
    {
        DBG_PRINTF("Notification wasn't sent as notifications are disabled \r\n");
    }
    
    hpsIsRequestProcessingPending = false;
}


/*******************************************************************************
* Function Name: HpsProcessRequest
********************************************************************************
*
* Summary:
*  Performs processing of an HTTP request.
*
* Parameters:
*  request: An HTTP request received from the server.
*
*******************************************************************************/
void HpsProcessRequest(cy_en_ble_hps_http_request_t request)
{
    cy_en_ble_api_result_t apiResult;
    uint16_t bodyLen;
    uint8_t *chraDataPtr = NULL;
    uint8_t tmpBuff[512u];

    /* Fill HTTP Headers Characteristic */
    apiResult = Cy_BLE_HPSS_SetCharacteristicValue(CY_BLE_HPS_HTTP_HEADERS, 
                                                    HPS_HTTP_RESPONSE_HEADER_SIZE,
                                                    responseHttpHeader);

    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Can't set HTTP Headers Characteristic. Error code: %x\r\n", apiResult);
    }

    hpsNotificationData.dataStatus = CY_BLE_HPS_HTTP_HEADERS_RECEIVED | CY_BLE_HPS_HTTP_BODY_RECEIVED;

    if(HpsIsHttpUriValid() == true)
    {
        /* For POST request parameter sent in HTTP Entity Body Characteristic,
        *  needs to be validated.
        */
        if(request == CY_BLE_HPS_HTTP_POST)
        {
            /* Get HTTP Entity Body Characteristic value */
            apiResult = Cy_BLE_HPSS_GetCharacteristicValue(CY_BLE_HPS_HTTP_ENTITY_BODY, 
                                                        HPS_HTTP_BODY_SIZE,
                                                        tmpBuff);

            if(apiResult != CY_BLE_SUCCESS)
            {
                DBG_PRINTF("Can't Read HTTP Entity Body Characteristic. Error code: %x\r\n", apiResult);
                
                /* Read HTTP Entity Body Characteristic failed. Send error
                * status to remote HPS Client.
                */
                hpsNotificationData.statusCode = HPS_HTTP_INTERNAL_SERVER_ERROR;
            }
            else
            {
                if(strncmp((const char *) postHttpBody1, (const char *) tmpBuff, HPS_HTTP_POST_BODY1_SIZE) == 0)
                {
                    hpsConnLedState = LED_ON;
                    hpsNotificationData.statusCode = HPS_HTTP_OK;
                    DBG_PRINTF("Blue LED was turned on!\r\n");
                }
                else
                if(strncmp((const char *) postHttpBody2, (const char *) tmpBuff, HPS_HTTP_POST_BODY2_SIZE) == 0)
                {
                    hpsConnLedState = LED_OFF;
                    hpsNotificationData.statusCode = HPS_HTTP_OK;
                    DBG_PRINTF("Blue LED was turned off!\r\n");
                }
                else
                {
                    /* Send error to remote HPS Client */
                    hpsNotificationData.statusCode = HPS_HTTP_NOT_ACCEPTABLE;
                }
            }
        }
        else /* HTTP GET request */
        {
            hpsNotificationData.dataStatus = CY_BLE_HPS_HTTP_HEADERS_RECEIVED | CY_BLE_HPS_HTTP_BODY_RECEIVED;
            hpsNotificationData.statusCode = HPS_HTTP_OK;
        }

        if(hpsNotificationData.statusCode == HPS_HTTP_OK)
        {
            /* Check state of connect LED and report its state via HTTP Entity Body
            * Characteristic
            */
            if(hpsConnLedState == LED_ON)
            {
                chraDataPtr = ledOnHttpBody;
                bodyLen = sizeof(ledOnHttpBody);
            }
            else
            {
                chraDataPtr = ledOffHttpBody;
                bodyLen = sizeof(ledOffHttpBody);
            }
        }
        else
        {
            chraDataPtr = otherErrorHttpBody;
            bodyLen = sizeof(otherErrorHttpBody);
        }
    }
    else
    {
        chraDataPtr = otherErrorHttpBody;
        bodyLen = sizeof(otherErrorHttpBody);

        hpsNotificationData.statusCode = HPS_HTTP_REQUESTED_HOST_UNAVAILABLE;
    }

    /* Fill HTTP Entity Body Characteristic */
    apiResult = Cy_BLE_HPSS_SetCharacteristicValue(CY_BLE_HPS_HTTP_ENTITY_BODY, 
                                                     bodyLen,
                                                     chraDataPtr);

    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Can't set HTTP Entity Body Characteristic. Error code: %x\r\n", apiResult);
    }
}


/*******************************************************************************
* Function Name: HpsIsHttpUriValid
********************************************************************************
*
* Summary:
*  Validates URI of the HTTP request and returns "YES" if the URI is
*  valid. Otherwise returns "NO".
*
* Return:
*  false - URI is valid (accepted by the server).
*  true  - URI is invalid.
*
*******************************************************************************/
bool HpsIsHttpUriValid(void)
{
    cy_en_ble_api_result_t apiResult;
    bool result = false;
    uint8_t tmpBuff[512u];

    apiResult = Cy_BLE_HPSS_GetCharacteristicValue(CY_BLE_HPS_URI, HPS_HTTP_URI_SIZE, tmpBuff);

    if(apiResult != CY_BLE_SUCCESS)
    {
        DBG_PRINTF("Can't read URI Characteristic. Error code: %x\r\n", apiResult);
    }
    else
    {
        if(0 == strncmp((const char *) supportedHttpUri, (const char *) tmpBuff, sizeof(supportedHttpUri)))
        {
            result = true;
        }
    }
    
    return (result);
}


/*******************************************************************************
* Function Name: HpsIsisRequestProcessingPending
********************************************************************************
*
* Summary:
*  Returns true if we have pending request which need processing
*
*******************************************************************************/
bool HpsIsRequestProcessingPending(void)
{
    return(hpsIsRequestProcessingPending);    
}

/*******************************************************************************
* Function Name: HpsGetConnLedState
********************************************************************************
*
* Summary:
*  Returns HPS connection LED state
*
*******************************************************************************/
uint8_t HpsGetConnLedState(void)
{
    return(hpsConnLedState);    
}


/* [] END OF FILE */
