/*******************************************************************************
* File Name: BLEFindMe.c
*
* Version: 1.20
*
* Description:
*  This file contains BLE related functions.
* 
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "BLEFindMe.h"

/*******************************************************************************
* Variables to keep track of BLE connection handle
*******************************************************************************/
cy_stc_ble_conn_handle_t    appConnHandle;

/* IAS alert level value. This value is picked up in the main execution loop
   for driving the alert (Blue) LED. */
uint8 alertLevel = 0;

/*******************************************************************************
*        Function prototypes
*******************************************************************************/
void StackEventHandler(uint32 event, void* eventParam);
void IasEventHandler(uint32 event, void* eventParam);
void EnterLowPowerMode(void);

/*******************************************************************************
* Function Name: MCWDT_Interrupt_Handler
*******************************************************************************/
void MCWDT_Interrupt_Handler(void)
{
    /* Clear the MCWDT peripheral interrupt */
    Cy_MCWDT_ClearInterrupt(MCWDT_HW, CY_MCWDT_CTR0);
    /* Clear the CM4 NVIC pending interrupt for MCWDT */
    NVIC_ClearPendingIRQ(MCWDT_isr_cfg.intrSrc);
    
    /* If mild alert is received, toggle the Alert LED */
    if (alertLevel == CY_BLE_MILD_ALERT)
    {
        Cy_GPIO_Inv(Alert_LED_0_PORT, Alert_LED_0_NUM);
    }
}

/*******************************************************************************
* Function Name: BleFindMe_Init()
********************************************************************************
*
* Summary:
*   This function initializes the BLE and UART components for use in the code
*   code example.
*
* Parameters:
*  None
*
* Return:
*   None
*
*******************************************************************************/
void BleFindMe_Init(void)
{
    cy_en_ble_api_result_t          apiResult;
    cy_stc_ble_stack_lib_version_t  stackVersion;  
       
    /* Configure switch SW2 as hibernate wake up source */
    Cy_SysPm_SetHibWakeupSource(CY_SYSPM_HIBPIN1_LOW);        
    
    /* Start the UART debug port */
    UART_DEBUG_START();
    DEBUG_PRINTF("\r\n\nPSoC 6 MCU with BLE Find Me Code Example \r\n");    
    
     /* Start Host of BLE Component and register generic event handler */
    apiResult = Cy_BLE_Start(StackEventHandler);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        /* BLE stack initialization failed, check configuration, 
           notify error and halt CPU in debug mode */
        DEBUG_PRINTF("Cy_BLE_Start API Error: %x \r\n", apiResult);
        ShowError();    
    }
    else
    {
        DEBUG_PRINTF("Cy_BLE_Start API Success: %x \r\n", apiResult);
    }
    
	apiResult = Cy_BLE_GetStackLibraryVersion(&stackVersion);
    
    if(apiResult != CY_BLE_SUCCESS)
    {
        DEBUG_PRINTF("Cy_BLE_GetStackLibraryVersion API Error: 0x%2.2x \r\n", apiResult);
        ShowError();  
    }
    else
    {
        DEBUG_PRINTF("Stack Version: %d.%d.%d.%d \r\n", stackVersion.majorVersion, 
            stackVersion.minorVersion, stackVersion.patch, stackVersion.buildNumber);
    }
    
    /* Register IAS event handler */
    Cy_BLE_IAS_RegisterAttrCallback(IasEventHandler); 
    
    /* Enable 4 Hz free-running MCWDT counter 0*/
    /* MCWDT_config structure is defined by the MCWDT_PDL component based on
       parameters entered in the customizer */
    Cy_MCWDT_Init(MCWDT_HW, &MCWDT_config);
    Cy_MCWDT_Enable(MCWDT_HW, CY_MCWDT_CTR0, 93 /* 2 LFCLK cycles */);
    /* Unmask the MCWDT counter 0 peripheral interrupt */
    Cy_MCWDT_SetInterruptMask(MCWDT_HW, CY_MCWDT_CTR0);
 
    /* Configure ISR connected to MCWDT interrupt signal*/
    /* MCWDT_isr_cfg structure is defined by the SYSINT_PDL component based on
       parameters entered in the customizer. */
    Cy_SysInt_Init(&MCWDT_isr_cfg, &MCWDT_Interrupt_Handler);
    /* Clear CM4 NVIC pending interrupt for MCWDT */
    NVIC_ClearPendingIRQ(MCWDT_isr_cfg.intrSrc);
    /* Enable CM4 NVIC MCWDT interrupt */
    NVIC_EnableIRQ(MCWDT_isr_cfg.intrSrc);
    
}

/*******************************************************************************
* Function Name: BleFindMe_Process()
********************************************************************************
*
* Summary:
*   This function processes the BLE events and configures the device to enter
*   low power mode as required.
*
* Parameters:
*  None
*
* Return:
*   None
*
*******************************************************************************/
void BleFindMe_Process(void)
{      
    /* The call to EnterLowPowerMode also causes the device to enter hibernate
       mode if the BLE stack is shutdown */
    EnterLowPowerMode();
    
    /* Cy_Ble_ProcessEvents() allows BLE stack to process pending events */
    Cy_BLE_ProcessEvents();
    
    /* Update Alert Level value on the Blue LED */
    switch(alertLevel)
    {        
        case CY_BLE_NO_ALERT:
            /* Disable MCWDT interrupt at NVIC */        
            NVIC_DisableIRQ(MCWDT_isr_cfg.intrSrc);
            /* Turn the Blue LED OFF in case of no alert */
            Cy_GPIO_Write(Alert_LED_0_PORT, Alert_LED_0_NUM, LED_OFF);
            break;

        /* Use the MCWDT to blink the Blue LED in case of mild alert */
        case CY_BLE_MILD_ALERT:
            /* Enable MCWDT interrupt at NVIC */               
            NVIC_EnableIRQ(MCWDT_isr_cfg.intrSrc);
            /* The MCWDT interrupt handler will take care of LED blinking */
            break;

        case CY_BLE_HIGH_ALERT:
            /* Disable MCWDT interrupt at NVIC */
            NVIC_DisableIRQ(MCWDT_isr_cfg.intrSrc);
            /* Turn the Blue LED ON in case of high alert */
            Cy_GPIO_Write(Alert_LED_0_PORT, Alert_LED_0_NUM, LED_ON);
            break;
            
        /* Do nothing in all other cases */        
        default:
            break;
    }      
}

/*******************************************************************************
* Function Name: StackEventHandler()
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE Component.
*
* Parameters:
*  uint32 event:      event from the BLE component
*  void* eventParam:  parameters related to the event
*
* Return:
*   None
*
*******************************************************************************/
void StackEventHandler(uint32 event, void* eventParam)
{
    cy_en_ble_api_result_t      apiResult;
    uint8 i;
    
    switch (event)
	{
        /* There are some events generated by the BLE component
        *  that are not required for this code example. */
        
        /**********************************************************
        *                       General Events
        ***********************************************************/
		/* This event is received when the BLE stack is started */
        case CY_BLE_EVT_STACK_ON:       
            DEBUG_PRINTF("CY_BLE_EVT_STACK_ON, Start Advertisement \r\n");    
            
            /* Enter into discoverable mode so that remote device can search it */
            apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);          
            if(apiResult != CY_BLE_SUCCESS)
            {
                DEBUG_PRINTF("Start Advertisement API Error: %d \r\n", apiResult);      
                ShowError();
                /* Execution does not continue beyond this point */
            }
            else
            {
                DEBUG_PRINTF("Start Advertisement API Success: %d \r\n", apiResult);
                Cy_GPIO_Write(Advertising_LED_0_PORT, Advertising_LED_0_NUM, LED_ON);
                Cy_GPIO_Write(Disconnect_LED_0_PORT, Disconnect_LED_0_NUM, LED_OFF);
                alertLevel = CY_BLE_NO_ALERT;                
            }
            
            /* Get address of the device */           
            apiResult = Cy_BLE_GAP_GetBdAddress();
            if(apiResult != CY_BLE_SUCCESS)
            {   
                DEBUG_PRINTF("Cy_BLE_GAP_GetBdAddress API Error: %d \r\n", apiResult);
            }
            else
            {
                DEBUG_PRINTF("Cy_BLE_GAP_GetBdAddress API Success: %d \r\n", apiResult);
            }

            break;
            
        /* This event is received when there is a timeout */
        case CY_BLE_EVT_TIMEOUT:
            DEBUG_PRINTF("CY_BLE_EVT_TIMEOUT \r\n"); 
            break;
            
        /* This event indicates that some internal HW error has occurred */    
		case CY_BLE_EVT_HARDWARE_ERROR:    
            DEBUG_PRINTF("Hardware Error \r\n");
            ShowError();
			break;
            
        /*  This event will be triggered by host stack if BLE stack is busy or 
         *  not busy. Parameter corresponding to this event will be the state 
    	 *  of BLE stack.
         *  BLE stack busy = CYBLE_STACK_STATE_BUSY,
    	 *  BLE stack not busy = CYBLE_STACK_STATE_FREE 
         */
    	case CY_BLE_EVT_STACK_BUSY_STATUS:
            DEBUG_PRINTF("CY_BLE_EVT_STACK_BUSY_STATUS: %x\r\n", *(uint8 *)eventParam);
            break;
            
        /* This event indicates completion of Set LE event mask */
        case CY_BLE_EVT_LE_SET_EVENT_MASK_COMPLETE:
            DEBUG_PRINTF("CY_BLE_EVT_LE_SET_EVENT_MASK_COMPLETE \r\n");
            break;
            
        /* This event indicates set device address command completed */
        case CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE:
            DEBUG_PRINTF("CY_BLE_EVT_SET_DEVICE_ADDR_COMPLETE \r\n");
            break;
            
        /* This event indicates get device address command completed
           successfully */
        case CY_BLE_EVT_GET_DEVICE_ADDR_COMPLETE:
            DEBUG_PRINTF("CY_BLE_EVT_GET_DEVICE_ADDR_COMPLETE: ");
            for(i = CY_BLE_GAP_BD_ADDR_SIZE; i > 0u; i--)
            {
                DEBUG_PRINTF("%2.2x", ((cy_stc_ble_bd_addrs_t *)((cy_stc_ble_events_param_generic_t *)eventParam)->eventParams)->publicBdAddr[i-1]);
            }
            DEBUG_PRINTF("\r\n");
            
            break;
         
        /* This event indicates set Tx Power command completed */
        case CY_BLE_EVT_SET_TX_PWR_COMPLETE:
            DEBUG_PRINTF("CY_BLE_EVT_SET_TX_PWR_COMPLETE \r\n");
            break;
            
        /* This event indicates that stack shutdown is complete */
        case CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE:
            DEBUG_PRINTF("CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE \r\n");
            
            DEBUG_PRINTF("Entering hibernate mode \r\n"); 
            DEBUG_WAIT_UART_TX_COMPLETE();
            Cy_SysPm_Hibernate();
            /* Code execution will not reach here */
            /* Device wakes up from hibernate and performs reset sequence 
               when the reset switch or SW2 switch on the kit is pressed */            
            break;
                        
        /**********************************************************
        *                       GAP Events
        ***********************************************************/
       
        /* This event indicates peripheral device has started/stopped
           advertising */
        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            DEBUG_PRINTF("CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP: ");
            
            if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
            {
                DEBUG_PRINTF("Advertisement started \r\n");
                Cy_GPIO_Write(Advertising_LED_0_PORT, Advertising_LED_0_NUM, LED_ON);
                Cy_GPIO_Write(Disconnect_LED_0_PORT, Disconnect_LED_0_NUM, LED_OFF);
            }
            else if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED)
            {
                DEBUG_PRINTF("Advertisement stopped \r\n");  
                Cy_GPIO_Write(Advertising_LED_0_PORT, Advertising_LED_0_NUM, LED_OFF);
                Cy_GPIO_Write(Disconnect_LED_0_PORT, Disconnect_LED_0_NUM, LED_ON);
                
                /* Advertisement event timed out before connection, shutdown BLE 
                * stack to enter hibernate mode and wait for device reset event
                * or SW2 press to wake up the device */          
                Cy_BLE_Stop();             
            }           
            break;
            
        /* This event is generated at the GAP Peripheral end after connection 
           is completed with peer Central device */
        case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
            DEBUG_PRINTF("CY_BLE_EVT_GAP_DEVICE_CONNECTED \r\n");
            break;
           
        /* This event is generated when disconnected from remote device or 
           failed to establish connection */
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:   
            if(Cy_BLE_GetConnectionState(appConnHandle) == CY_BLE_CONN_STATE_DISCONNECTED)
            {
                DEBUG_PRINTF("CY_BLE_EVT_GAP_DEVICE_DISCONNECTED %d\r\n", CY_BLE_CONN_STATE_DISCONNECTED);
                alertLevel = CY_BLE_NO_ALERT;
                
                Cy_GPIO_Write(Advertising_LED_0_PORT, Advertising_LED_0_NUM, LED_OFF);
                Cy_GPIO_Write(Disconnect_LED_0_PORT, Disconnect_LED_0_NUM, LED_ON);   
              
                /* Enter into discoverable mode so that remote device can search it */
                apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);          
                if(apiResult != CY_BLE_SUCCESS)
                {
                    DEBUG_PRINTF("Start Advertisement API Error: %d \r\n", apiResult);      
                    ShowError();
                    /* Execution does not continue beyond this point */
                }
                else
                {
                    DEBUG_PRINTF("Start Advertisement API Success: %d \r\n", apiResult);
                    Cy_GPIO_Write(Advertising_LED_0_PORT, Advertising_LED_0_NUM, LED_ON);
                    Cy_GPIO_Write(Disconnect_LED_0_PORT, Disconnect_LED_0_NUM, LED_OFF);
                }
            
            }
            break;
            
        /* This event is generated at the GAP Central and the peripheral end 
           after connection parameter update is requested from the host to 
           the controller */
        case CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
            DEBUG_PRINTF("CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE \r\n");
            break;

        /* This event is triggered instead of 'CY_BLE_EVT_GAP_DEVICE_CONNECTED', 
           if Link Layer Privacy is enabled in component customizer */
        case CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE:
            
            /* BLE link is established */
            /* This event will be triggered since link layer privacy is enabled */
            DEBUG_PRINTF("CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE \r\n");
            if(Cy_BLE_GetState() == CY_BLE_STATE_ON)
            {           
                Cy_GPIO_Write(Advertising_LED_0_PORT, Advertising_LED_0_NUM, LED_OFF);
                Cy_GPIO_Write(Disconnect_LED_0_PORT, Disconnect_LED_0_NUM, LED_OFF);
            }
            break;    
                     
        /**********************************************************
        *                       GATT Events
        ***********************************************************/
            
        /* This event is generated at the GAP Peripheral end after connection 
           is completed with peer Central device */
       case CY_BLE_EVT_GATT_CONNECT_IND:
            appConnHandle = *(cy_stc_ble_conn_handle_t *)eventParam;
            DEBUG_PRINTF("CY_BLE_EVT_GATT_CONNECT_IND: %x, %x \r\n", 
                        (*(cy_stc_ble_conn_handle_t *)eventParam).attId, 
                        (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle);
            break;
            
        /* This event is generated at the GAP Peripheral end after 
           disconnection */
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            DEBUG_PRINTF("CY_BLE_EVT_GATT_DISCONNECT_IND \r\n");
            Cy_GPIO_Write(Disconnect_LED_0_PORT, Disconnect_LED_0_NUM, LED_ON);
            break;
         
        /* This event is triggered when 'GATT MTU Exchange Request' 
           received from GATT client device */
        case CY_BLE_EVT_GATTS_XCNHG_MTU_REQ:
            DEBUG_PRINTF("CY_BLE_EVT_GATTS_XCNHG_MTU_REQ \r\n");
            break;
        
        /* This event is triggered when a read received from GATT 
           client device */
        case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
            DEBUG_PRINTF("CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ \r\n");
            break;

        /**********************************************************
        *                       Other Events
        ***********************************************************/
        default:
            DEBUG_PRINTF("Other event: %lx \r\n", (unsigned long) event);
			break;
	}
}

/*******************************************************************************
* Function Name: IasEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component,
*  which are specific to Immediate Alert Service.
*
* Parameters:
*  uint32 event:      event from the BLE component
*  void* eventParams: parameters related to the event
*
* Return:
*  None
*
*******************************************************************************/
void IasEventHandler(uint32 event, void *eventParam)
{
    /* Alert Level Characteristic write event */
    if(event == CY_BLE_EVT_IASS_WRITE_CHAR_CMD)
    {
        /* Read the updated Alert Level value from the GATT database */
        Cy_BLE_IASS_GetCharacteristicValue(CY_BLE_IAS_ALERT_LEVEL, 
            sizeof(alertLevel), &alertLevel);
    }

    /* To remove unused parameter warning */
    eventParam = eventParam;
}

/*******************************************************************************
* Function Name: EnterLowPowerMode()
********************************************************************************
* Summary:
*  Configures the device to enter low power mode.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Theory:
*  The function configures the device to enter deep sleep - whenever the 
*  BLE is idle and the UART transmission/reception is not happening. 
*
*  In case of disconnection, the function configures the device to 
*  enter hibernate mode.
*
*******************************************************************************/
void EnterLowPowerMode(void)
{
    DEBUG_PRINTF("Entering deep sleep mode \r\n");     
    DEBUG_WAIT_UART_TX_COMPLETE();
    
    /* Configure deep sleep mode to wake up on interrupt */
    Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);

    DEBUG_PRINTF("Exiting deep sleep mode \r\n"); 
    DEBUG_WAIT_UART_TX_COMPLETE();          
}
/* [] END OF FILE */
