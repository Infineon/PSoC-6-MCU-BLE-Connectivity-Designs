/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This is source code for the CY_BLE Proximity Profile example project. In this
*  example project the CY_BLE component is configured for Proximity Reporter
*  profile role.
*
********************************************************************************
* Copyright 2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"


/***************************************
*          API Constants
***************************************/
#define DISCONNECTED                        (0u)
#define ADVERTISING                         (1u)
#define CONNECTED                           (2u)

#define ALERT_TO                            (20u)
#define BLINK_DELAY                         (90u)
#define ALERT_BLINK_DELAY                   (20u)


/***************************************
*        Global Variables
***************************************/
cy_stc_ble_conn_handle_t  connectionHandle;
uint8_t                ledState = LED_OFF;
uint8_t                advLedState = LED_OFF;
uint8_t                alertLedState = LED_OFF;
uint16_t               alertBlinkDelayCount;
uint16_t               advBlinkDelayCount;
uint8_t                displayAlertMessage = YES;
uint8_t                buttonState = BUTTON_IS_NOT_PRESSED;


/*******************************************************************************
* Function Name: AppCallBack
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the CY_BLE Component.
*
* Parameters:
*  uint8_t event:       Event from the CY_BLE component.
*  void* eventParams: A structure instance for corresponding event type. The
*                     list of events structure is described in the component
*                     datasheet.
*
* Return:
*  None
*
*******************************************************************************/
void AppCallBack(uint32_t event, void *eventParam)
{
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_gap_bd_addr_t localAddr;
    uint8_t i;

    switch(event)
    {
    /**********************************************************
    *                       General Events
    ***********************************************************/
    case CY_BLE_EVT_STACK_ON: /* This event is received when the component is Started */
        DBG_PRINTF("Bluetooth On. Device address is: ");
        localAddr.type = 0u;
        Cy_BLE_GetDeviceAddress(&localAddr);
        for(i = CY_BLE_GAP_BD_ADDR_SIZE; i > 0u; i--)
        {
            DBG_PRINTF("%2.2x", localAddr.bdAddr[i-1]);
        }
        DBG_PRINTF("\r\n");

        /* Enter discoverable mode so that the remote Client could find the device. */
        apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST);

        if(apiResult != CY_BLE_ERROR_OK)
        {
            DBG_PRINTF("StartAdvertisement API Error: %d \r\n", apiResult);
        }
        break;

    case CY_BLE_EVT_TIMEOUT:
        /* Possible timeout event parameter values:
        * CY_BLE_GAP_ADV_MODE_TO -> GAP limited discoverable mode timeout;
        * CY_BLE_GAP_AUTH_TO -> GAP pairing process timeout.
        */
        if(CY_BLE_GAP_ADV_MODE_TO == *(uint8_t *) eventParam)
        {
            DBG_PRINTF("Advertisement timeout occurred. Advertisement will be disabled.\r\n");
        }
        else
        {
            DBG_PRINTF("Timeout occurred.\r\n");
        }
        break;

    case CY_BLE_EVT_HARDWARE_ERROR:    /* This event indicates that some internal HW error has occurred. */
        DBG_PRINTF("Hardware Error \r\n");
        break;

    case CY_BLE_EVT_HCI_STATUS:
        DBG_PRINTF("HCI Error. Error code is %x.\r\n", *(uint8_t *) eventParam);
        break;

    /**********************************************************
    *                       GAP Events
    ***********************************************************/
    case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
        if(Cy_BLE_GetState() == CY_BLE_STATE_DISCONNECTED)
        {   
            /* Fast and slow advertising period complete, go to low power  
             * mode (Hibernate mode) and wait for an external
             * user event to wake up the device again */
            DBG_PRINTF("Hibernate \r\n");
            Disconnect_LED_Write(LED_ON);
            Advertising_LED_Write(LED_OFF);
            Alert_LED_Write(LED_OFF);
        #if (DEBUG_UART_ENABLED == ENABLED)
            while((UART_DEB_SpiUartGetTxBufferSize() + UART_DEB_GET_TX_FIFO_SR_VALID) != 0);
        #endif /* (DEBUG_UART_ENABLED == ENABLED) */
            SW2_ClearInterrupt();
            SW2_Interrupt_ClearPending();
            SW2_Interrupt_Start();
            CySysPmHibernate();
        }
        break;

    case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
        DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_CONNECTED: %d \r\n", connectionHandle.bdHandle);
        break;

    case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
        DBG_PRINTF("CY_BLE_EVT_GAP_DEVICE_DISCONNECTED\r\n");
        /* Enter discoverable mode so that remote Client could find the device. */
        apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST);
        connectionHandle.bdHandle = 0u;
        if(apiResult != CY_BLE_ERROR_OK)
        {
            DBG_PRINTF("StartAdvertisement API Error: %d\r\n", apiResult);
        }
        break;

    /**********************************************************
    *                       GATT Events
    ***********************************************************/
    case CY_BLE_EVT_GATT_CONNECT_IND:
        /* GATT connection was established */
        connectionHandle = *(cy_stc_ble_conn_handle_t *) eventParam;
        DBG_PRINTF("CY_BLE_EVT_GATT_CONNECT_IND: %x\r\n", connectionHandle.attId);
        break;

    case CY_BLE_EVT_GATT_DISCONNECT_IND:
        /* GATT connection was disabled */
        DBG_PRINTF("CY_BLE_EVT_GATT_DISCONNECT_IND:\r\n");
        connectionHandle.attId = 0u;

        /* Get the LLS Alert Level from the GATT database */
        (void) Cy_BLE_LLSS_GetCharacteristicValue(CY_BLE_LLS_ALERT_LEVEL, CY_BLE_LLS_ALERT_LEVEL_SIZE, &alertLevel);
        break;

    case CY_BLE_EVT_GATTS_XCNHG_MTU_REQ:
        break;

    case CY_BLE_EVT_GATTS_INDICATION_ENABLED:
        break;

    case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
        /* Triggered on server side when client sends read request and when
        * characteristic has CY_BLE_GATT_DB_ATTR_CHAR_VAL_RD_EVENT property set.
        * This event could be ignored by application unless it need to response
        * by error response which needs to be set in gattErrorCode field of
        * event parameter. */
        DBG_PRINTF("CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ: handle: %x \r\n", 
            ((cy_stc_ble_gatts_char_val_read_req_t *)eventParam)->attrHandle);
        break;

    /**********************************************************
    *                       Other Events
    ***********************************************************/
    case CY_BLE_EVT_PENDING_FLASH_WRITE:
        /* Inform application that flash write is pending. Stack internal data 
        * structures are modified and require to be stored in Flash using 
        * Cy_BLE_StoreBondingData() */
        DBG_PRINTF("CY_BLE_EVT_PENDING_FLASH_WRITE\r\n");
        break;

    default:
        DBG_PRINTF("OTHER event: %lx \r\n", event);
        break;
    }
}


/*******************************************************************************
* Function Name: SW_Interrupt
********************************************************************************
*
* Summary:
*   Handles the mechanical button press.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(ButtonPressInt)
{
    buttonState = BUTTON_IS_PRESSED;

    SW2_ClearInterrupt();
}


/*******************************************************************************
* Function Name: WDT_Start
********************************************************************************
*
* Summary:
*  Configures and starts Watchdog timer to trigger an interrupt every second.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void WDT_Start(void)
{
    /* Unlock the WDT registers for modification */
    CySysWdtUnlock();
    /* Setup ISR callback */
    WDT_Interrupt_StartEx(&Timer_Interrupt);
    /* Write the mode to generate interrupt on match */
    CySysWdtWriteMode(WDT_COUNTER, CY_SYS_WDT_MODE_INT);
    /* Configure the WDT counter clear on a match setting */
    CySysWdtWriteClearOnMatch(WDT_COUNTER, WDT_COUNTER_ENABLE);
    /* Configure the WDT counter match comparison value */
    CySysWdtWriteMatch(WDT_COUNTER, WDT_COUNT_PERIOD);
    /* Reset WDT counter */
    CySysWdtResetCounters(WDT_COUNTER);
    /* Enable the specified WDT counter */
    CySysWdtEnable(WDT_COUNTER_MASK);
    /* Lock out configuration changes to the Watchdog timer registers */
    CySysWdtLock();
}


/*******************************************************************************
* Function Name: Timer_Interrupt
********************************************************************************
*
* Summary:
*  Handles the Interrupt Service Routine for the WDT timer.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(Timer_Interrupt)
{
    if(CySysWdtGetInterruptSource() & WDT_INTERRUPT_SOURCE)
    {
        if(Cy_BLE_GetState() != CY_BLE_STATE_CONNECTED)
        {
            if((llsAlertTOCounter != ALERT_TO) && (alertLevel != CY_BLE_NO_ALERT))
            {
                /* Update alert timeout */
                llsAlertTOCounter++;
            }
            else
            {
                alertLevel = CY_BLE_NO_ALERT;
                
                /* Clear alert timeout */
                llsAlertTOCounter = 0u;
            }
        }
        /* Clear interrupt request */
        CySysWdtClearInterrupt(WDT_INTERRUPT_SOURCE);
    }
}


/*******************************************************************************
* Function Name: HandleLeds
********************************************************************************
*
* Summary:
*  Handles indications on Advertising, Disconnection and Alert LEDs. Also it
*  implements timeout after which Alert LED is cleared if it was previously set.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void HandleLeds(void)
{
    uint8_t advertisingIsOn = 0u;
    
    /* If in disconnected state ... */
    if(Cy_BLE_GetState() == CY_BLE_STATE_DISCONNECTED)
    {
        /* ... turn on disconnect indication LED and turn off rest of the LEDs. */
        Disconnect_LED_Write(LED_ON);
        Advertising_LED_Write(LED_OFF);
    }
    /* In advertising state ... */
    else if(Cy_BLE_GetState() == CY_BLE_STATE_ADVERTISING)
    {
        /* ... turn off disconnect indication and alert LEDs ... */
        Disconnect_LED_Write(LED_OFF);

        advertisingIsOn++;
    }
    /* In connected State ... */
    else
    {
        /* ... turn off all LEDs. */
        Disconnect_LED_Write(LED_OFF);
        Advertising_LED_Write(LED_OFF);
        Alert_LED_Write(LED_OFF);
    }

    if(Cy_BLE_GetState() != CY_BLE_STATE_CONNECTED)
    {
        /* If "Mild Alert" is set by the Client then blink alert LED */
        if((CY_BLE_MILD_ALERT == alertLevel) && (llsAlertTOCounter != ALERT_TO))
        {
            if(displayAlertMessage == YES)
            {
                DBG_PRINTF("Device started alerting with \"Mild Alert\"\r\n");
                displayAlertMessage = NO;
            }

            /* Switch state of alert LED to make blinking */
            if(alertBlinkDelayCount == ALERT_BLINK_DELAY)
            {
                if(alertLedState == LED_OFF)
                {
                    alertLedState = LED_ON;
                }
                else
                {
                    alertLedState = LED_OFF;
                }
                alertBlinkDelayCount = 0u;
            }
            alertBlinkDelayCount++;
        }
        /* For "High Alert" turn alert LED on */
        else if((alertLevel == CY_BLE_HIGH_ALERT) && (llsAlertTOCounter != ALERT_TO))
        {
            if(displayAlertMessage == YES)
            {
                DBG_PRINTF("Device started alerting with \"High Alert\"\r\n");
                displayAlertMessage = NO;
            }

            alertLedState = LED_ON;
        }
        /* In case of "No Alert" turn alert LED off */
        else
        {
            displayAlertMessage = YES;

            alertLedState = LED_OFF;
        }

        if((advertisingIsOn != 0u) && (llsAlertTOCounter == 0u))
        {
            /* ... and blink advertisement indication LED. */
            if(advBlinkDelayCount == BLINK_DELAY)
            {
                if(advLedState == LED_OFF)
                {
                    advLedState = LED_ON;
                }
                else
                {
                    advLedState = LED_OFF;
                }
                advBlinkDelayCount = 0u;
            }

            advBlinkDelayCount++;

            Advertising_LED_Write(advLedState);

        }
        else
        {
            Advertising_LED_Write(LED_OFF);
        }
        
        Alert_LED_Write(alertLedState);
    }
}


/*******************************************************************************
* Function Name: LowPowerImplementation()
********************************************************************************
* Summary:
* Implements low power in the project.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The function tries to enter deep sleep as much as possible - whenever the 
* BLE is idle and the UART transmission/reception is not happening. At all other
* times, the function tries to enter CPU sleep.
*
*******************************************************************************/
static void LowPowerImplementation(void)
{
    cy_en_ble_lp_mode_t bleMode;
    uint8_t interruptStatus;
    
    /* For advertising and connected states, implement deep sleep 
     * functionality to achieve low power in the system. For more details
     * on the low power implementation, refer to the Low Power Application 
     * Note.
     */
    if((Cy_BLE_GetState() == CY_BLE_STATE_ADVERTISING) || 
       (Cy_BLE_GetState() == CY_BLE_STATE_CONNECTED))
    {
        /* Request BLE subsystem to enter into Deep-Sleep mode between connection and advertising intervals */
        bleMode = Cy_BLE_EnterLPM(CY_BLE_BLESS_DEEPSLEEP);
        /* Disable global interrupts */
        interruptStatus = CyEnterCriticalSection();
        /* When BLE subsystem has been put into Deep-Sleep mode */
        if(bleMode == CY_BLE_BLESS_DEEPSLEEP)
        {
            /* And it is still there or ECO is on */
            if((Cy_BLE_GetBleSsState() == CY_BLE_BLESS_STATE_ECO_ON) || 
               (Cy_BLE_GetBleSsState() == CY_BLE_BLESS_STATE_DEEPSLEEP))
            {
            #if (DEBUG_UART_ENABLED == ENABLED)
                /* Put the CPU into the Deep-Sleep mode when all debug information has been sent */
                if((UART_DEB_SpiUartGetTxBufferSize() + UART_DEB_GET_TX_FIFO_SR_VALID) == 0u)
                {
                    CySysPmDeepSleep();
                }
                else /* Put the CPU into Sleep mode and let SCB to continue sending debug data */
                {
                    CySysPmSleep();
                }
            #else
                CySysPmDeepSleep();
            #endif /* (DEBUG_UART_ENABLED == ENABLED) */
            }
        }
        else /* When BLE subsystem has been put into Sleep mode or is active */
        {
            /* And hardware doesn't finish Tx/Rx opeation - put the CPU into Sleep mode */
            if(Cy_BLE_GetBleSsState() != CY_BLE_BLESS_STATE_EVENT_CLOSE)
            {
                CySysPmSleep();
            }
        }
        /* Enable global interrupt */
        CyExitCriticalSection(interruptStatus);
    }
}


/*******************************************************************************
* Function Name: main()
********************************************************************************
* Summary:
*  Main function for the project.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Theory:
*  The function starts BLE and UART components.
*  This function process all BLE events and also implements the low power 
*  functionality.
*
*******************************************************************************/
int main()
{
    cy_en_ble_api_result_t apiResult;
    cy_ble_bless_pwr_in_db_t txPower;
    int8_t intTxPowerLevel;

    CyGlobalIntEnable;

    /* Turn off all of the LEDs */
    Disconnect_LED_Write(LED_OFF);
    Advertising_LED_Write(LED_OFF);
    Alert_LED_Write(LED_OFF);

    /* Start CY_BLE component and register generic event handler */
    Cy_BLE_Start(AppCallBack);

    SW2_Interrupt_StartEx(&ButtonPressInt);

    /* Register the event handler for LLS specific events */
    Cy_BLE_LLS_RegisterAttrCallback(LlsServiceAppEventHandler);

    /* Register the event handler for TPS specific events */
    Cy_BLE_TPS_RegisterAttrCallback(TpsServiceAppEventHandler);

    WDT_Start();
    
#if (DEBUG_UART_ENABLED == ENABLED)
    UART_DEB_Start();
#endif /* (DEBUG_UART_ENABLED == ENABLED) */
    
    /* Set Tx power level for connection channels to +3 dBm */
    txPower.bleSsChId = CY_BLE_LL_CONN_CH_TYPE;
    txPower.blePwrLevelInDbm = CY_BLE_LL_PWR_LVL_3_DBM;
    apiResult = Cy_BLE_SetTxPowerLevel(&txPower);

    if(apiResult == CY_BLE_ERROR_OK)
    {
        /* Convert power level to numeric int8_t value */
        intTxPowerLevel = ConvertTxPowerlevelToInt8(txPower.blePwrLevelInDbm);

        /* Set Tx power level for advertisement channels to +3 dBm */
        txPower.bleSsChId = CY_BLE_LL_ADV_CH_TYPE;
        txPower.blePwrLevelInDbm = CY_BLE_LL_PWR_LVL_3_DBM;
        apiResult = Cy_BLE_SetTxPowerLevel(&txPower);
        
        if(apiResult == CY_BLE_ERROR_OK)
        {
            /* Write the new Tx power level value to the GATT database */
            apiResult = Cy_BLE_TPSS_SetCharacteristicValue(CY_BLE_TPS_TX_POWER_LEVEL,
                                                         CY_BLE_TPS_TX_POWER_LEVEL_SIZE,
                                                         &intTxPowerLevel);

            if(apiResult != CY_BLE_ERROR_OK)
            {
                DBG_PRINTF("Cy_BLE_TPSS_SetCharacteristicValue() error.\r\n");
            }
            
            /* Display new Tx Power Level value */
            DBG_PRINTF("Tx power level is set to %d dBm\r\n", intTxPowerLevel);
        }
    }

    while(1)
    {
        /* Cy_BLE_ProcessEvents() allows BLE stack to process pending events */
        Cy_BLE_ProcessEvents();

        /* To achieve low power in the device */
        LowPowerImplementation();

        if((Cy_BLE_GetState() != CY_BLE_STATE_CONNECTED) && (Cy_BLE_GetState() != CY_BLE_STATE_ADVERTISING))
        {
            if(buttonState == BUTTON_IS_PRESSED)
            {
                /* Start advertisement */
                if(CY_BLE_ERROR_OK == Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST))
                {
                    DBG_PRINTF("Device has entered Limited Discovery mode \r\n");
                }

                /* Reset button state */
                buttonState = BUTTON_IS_NOT_PRESSED;
            }
        }
        else
        {
            /* Decrease Tx power level of BLE radio if button is pressed */
            if(buttonState == BUTTON_IS_PRESSED)
            {
                /* Specify connection channels for reading Tx power level */
                txPower.bleSsChId = CY_BLE_LL_CONN_CH_TYPE;

                /* Get current Tx power level */
                Cy_BLE_GetTxPowerLevel(&txPower);

                /* Decrease the Tx power level by one scale */
                DecreaseTxPowerLevelValue(&txPower.blePwrLevelInDbm);

                /* Set the new Tx power level */
                apiResult = Cy_BLE_SetTxPowerLevel(&txPower);

                if(apiResult == CY_BLE_ERROR_OK)
                {
                    /* Convert power level to numeric int8_t value */
                    intTxPowerLevel = ConvertTxPowerlevelToInt8(txPower.blePwrLevelInDbm);

                    /* Specify advertisement channels for reading Tx power level */
                    txPower.bleSsChId = CY_BLE_LL_ADV_CH_TYPE;

                    /* Get current Tx power level */
                    Cy_BLE_GetTxPowerLevel(&txPower);

                    /* Decrease the Tx power level by one scale for the advertisement channels */
                    DecreaseTxPowerLevelValue(&txPower.blePwrLevelInDbm);

                    /* Set the new Tx power level for advertisement channels */
                    apiResult = Cy_BLE_SetTxPowerLevel(&txPower);

                    /* Write the new Tx power level value to the GATT database */
                    apiResult = Cy_BLE_TPSS_SetCharacteristicValue(CY_BLE_TPS_TX_POWER_LEVEL,
                                                            CY_BLE_TPS_TX_POWER_LEVEL_SIZE,
                                                            &intTxPowerLevel);

                    if(apiResult == CY_BLE_ERROR_OK)
                    {
                        /* Display new Tx Power Level value */
                        DBG_PRINTF("Tx power level is set to %d dBm\r\n", intTxPowerLevel);

                        if((YES == isTpsNotificationEnabled) && (YES == isTpsNotificationPending)) 
                        {
                            /* Send notification to the Client */
                            apiResult = Cy_BLE_TPSS_SendNotification(connectionHandle, CY_BLE_TPS_TX_POWER_LEVEL, 
                                                                   CY_BLE_TPS_TX_POWER_LEVEL_SIZE, &intTxPowerLevel);

                            if(apiResult == CY_BLE_ERROR_OK)
                            {
                                DBG_PRINTF("New Tx power level value was notified to the Client\r\n");
                            }
                            else
                            {
                                DBG_PRINTF("Failed to send notification to the Client\r\n");
                            }
                        }
                    }
                }
                /* Reset button state */
                buttonState = BUTTON_IS_NOT_PRESSED;
            }
        }

        HandleLeds();
    }
}


/* [] END OF FILE */
