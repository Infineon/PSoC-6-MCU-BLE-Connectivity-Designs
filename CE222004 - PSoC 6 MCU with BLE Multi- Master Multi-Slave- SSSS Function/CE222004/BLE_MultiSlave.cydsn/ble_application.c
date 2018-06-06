/*******************************************************************************
* File Name: ble_application.c
*
* Version: 1.10
*
* Description:
*  This file contains BLE related functions.
* 
*******************************************************************************
* Copyright (2017-2018), Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress’s integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress 
* reserves the right to make changes to the Software without notice. Cypress 
* does not assume any liability arising out of the application or use of the 
* Software or any product or circuit described in the Software. Cypress does 
* not authorize its products for use in any products where a malfunction or 
* failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such 
* system or application assumes all risk of such use and in doing so agrees to 
* indemnify Cypress against all liability.
*******************************************************************************/
/* Header file includes */
#include "ble_application.h"
#include "custom_notification.h"
#include "custom_service.h"
#include "rgb_led_service.h"
#include "health_thermometer_service.h"
#include "temperature.h"
#include "led.h"
#include "debug.h"

/* Macros */
#define TIME_OUT        (120u)      /* 250msec * 120 = 30sec                    */
                                    /* MCWDT will generate interrupt at every   */
                                    /* 250msec.                                 */
                                    
/* Flag variables to handle different events */
bool writeReqPending    = false;
bool mcwdtInterruptFlag = false;

/* Variable used to maintain connection information */
cy_stc_ble_conn_handle_t appConnHandle[CY_BLE_CONN_COUNT];

/* Variable to store different event parameters */
cy_stc_ble_gatts_write_cmd_req_param_t *writeReqParameter;

/* Variable to check connection timeout */
uint8_t timeOutVar;

/* Variable used to manage HTS */
bool htsIndication = false;

/* Variable used to maintain how many devices requested for HTS indication */
static uint8_t htsRequestCount = 0;

/*****************************************************/
/*              Function prototypes                  */
/*****************************************************/

/* Health Thermometer Service */
void CallBackHts(uint32 event, void *eventParam);

/* BLE Stack Event Handler  */
void StackEventHandler(uint32 event, void* eventParam);

/* MCWDT interrupt service routine */
void MCWDT_Interrupt_Handler(void);

/* Initialize connection database */
void ResetConnState(void);

/* Checks is system ready for hibernate mode */
inline bool IsReadyForHibernate(void);

/* Puts CM4 into deep sleep mode */
void EnterLowPowerMode(void);

/*******************************************************************************
* Function Name: InitMcwdt
********************************************************************************
*
* Summary:
*  Function that initializes the MCWDT To generate interrupt every 250msec.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void InitMcwdt(void)
{
    /* Enable 4 Hz free-running MCWDT_Timer_4Hz counter 0*/
    /* MCWDT_Timer_4Hz_config structure is defined by the MCWDT_PDL component based on
       parameters entered in the customizer */
    Cy_MCWDT_Init(MCWDT_Timer_4Hz_HW, &MCWDT_Timer_4Hz_config);
    Cy_MCWDT_Enable(MCWDT_Timer_4Hz_HW, CY_MCWDT_CTR0, 93 /* 2 LFCLK cycles */);
    /* Unmask the MCWDT counter 0 peripheral interrupt */
    Cy_MCWDT_SetInterruptMask(MCWDT_Timer_4Hz_HW, CY_MCWDT_CTR0);
 
    /* Configure ISR connected to MCWDT_Timer_4Hz interrupt signal*/
    /* MCWDT_Timer_4Hz_isr_cfg structure is defined by the SYSINT_PDL component based on
       parameters entered in the customizer. */
    Cy_SysInt_Init(&MCWDT_isr_cfg, &MCWDT_Interrupt_Handler);
    /* Clear CM4 NVIC pending interrupt for MCWDT */
    NVIC_ClearPendingIRQ(MCWDT_isr_cfg.intrSrc);
    /* Enable CM4 NVIC MCWDT interrupt */
    NVIC_EnableIRQ(MCWDT_isr_cfg.intrSrc);
}

/*******************************************************************************
* Function Name: BleApplicationInit
********************************************************************************
*
* Summary:
*  Function that performs all required initialization for this project, that 
*  includes
*  - initialization of the BLE component with a custom event handler
*  - initialization of the PWMs
*  - initialization of the ADC
*  - initialization of the MCWDT
*  - initialization of the UART
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void BleApplicationInit(void)
{
    cy_en_ble_api_result_t          apiResult;
    cy_stc_ble_stack_lib_version_t  stackVersion;  
    
    InitRgb();
    InitTemperature();
    InitMcwdt();    
    
    /* Start the UART debug port */
    UART_DEBUG_START();
    
    /* Initialize connection database */
    ResetConnState();
        
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    DEBUG_PRINTF("\x1b[2J\x1b[;H");
    DEBUG_PRINTF("CE222004- PSoC 6 MCU with BLE Connectivity Multi-Master Multi-Slave: SSSS Function \r\n\n");    
    
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
    /* Register the HTS specific callback handler */
    apiResult = Cy_BLE_HTS_RegisterAttrCallback(CallBackHts);
    if (apiResult != CY_BLE_SUCCESS )
    {
        DEBUG_PRINTF("BLE HTS registration failed! \r\n");
    }
    else
    {
        DEBUG_PRINTF("BLE HTS registration completed \r\n");
    }
}

/*******************************************************************************
* Function Name: BleProcess
********************************************************************************
* Summary:
*  Function that continuously process the BLE events and handles custom BLE 
*  services
*
* Parameters:
*  None
*
* Return:
*  Void
*
*******************************************************************************/
void BleProcess(void)
{
    cy_en_ble_api_result_t apiResult;
    uint8_t i;
    
    /* Temporary array to hold Health Thermometer characteristic information */
    uint8 temperatureArray[HTS_DATA_LEN];
    
    /* Enter low power mode to save power */
    EnterLowPowerMode();
        
    /* Cy_Ble_ProcessEvents() allows BLE stack to process pending events */
    Cy_BLE_ProcessEvents();
       
    /* Start advertisement */
    if((Cy_BLE_GetState() == CY_BLE_STATE_ON) && \
       (Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED) && \
       (Cy_BLE_GetNumOfActiveConn() < CY_BLE_CONN_COUNT))
    {
        apiResult = Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
        if(apiResult != CY_BLE_SUCCESS)
	    {
            DEBUG_PRINTF("Start Advertisement API Error: ​%d\r\n", apiResult);
            ShowError();
            /* Execution does not continue beyond this point */
	    }
	    else
	    {
            DEBUG_PRINTF("Start Advertisement API Success: ​%d\r\n", apiResult);    
        }
    }
    
    /* BLE is connected */
	if(0 != Cy_BLE_GetNumOfActiveConn())
	{ 
        if(writeReqPending)
        {
            /* Reset flag */
            writeReqPending = false;
            
            /* Write request for 128-bit custom service*/
            if(CY_BLE_CUSTOM_SERVICE_CUSTOM_CHARACTERISTIC_CHAR_HANDLE == \
                                writeReqParameter->handleValPair.attrHandle)
            {
                /* Handle write request for Custom Service */
                HandleCustomServiceWriteRequest(writeReqParameter);
                
                /* Update variable for notification */
                customNotification[CUSTOM_NOTIFICATION_DEVICE_INDEX] = \
                    CY_BLE_CONN_COUNT - writeReqParameter->connHandle.attId;
                customNotification[CUSTOM_NOTIFICATION_SERVICE_INDEX] = \
                    NOTIFICATION_CUSTOM_SERVICE;
            }
            
            /* write request to change the color of RGB LED */
            if (RGB_CCCD_HANDLE == writeReqParameter->handleValPair.attrHandle ||
                CY_BLE_RGB_LED_RGB_LED_CONTROL_CHAR_HANDLE == \
                    writeReqParameter->handleValPair.attrHandle)
            {
                /* Handle write request for RGB LED Service */
                HandleWriteRequestforRgb(writeReqParameter);
                
                /* Update variable for notification */
                customNotification[CUSTOM_NOTIFICATION_DEVICE_INDEX] = \
                    CY_BLE_CONN_COUNT - writeReqParameter->connHandle.attId;
                customNotification[CUSTOM_NOTIFICATION_SERVICE_INDEX] = \
                    NOTIFICATION_RGB_SERVICE;
            }
                        
            /* Send the response to the write request received. */
            Cy_BLE_GATTS_WriteRsp(writeReqParameter->connHandle);
            
            /* Notify all the connected devices */
            for(i = 0; i < CY_BLE_CONN_COUNT; i++)
            {
                if(Cy_BLE_GetConnectionState(appConnHandle[i]) == CY_BLE_CONN_STATE_CONNECTED)
                {
                    SendCustomNotification(appConnHandle[i]);
                }
            }
        }
        
        /* Connected device requested for temperature data */
        if(htsIndication)
        {
            /* Get temprature information */ 
            HtsGetTemperature(temperatureArray);

            /* Send indication to the central */
            for(i = 0; i < CY_BLE_CONN_COUNT;i++)
            {
                if(Cy_BLE_GetConnectionState(appConnHandle[i]) == CY_BLE_CONN_STATE_CONNECTED)
                {
                    HtsSendIndication(appConnHandle[i], temperatureArray);
                }
            }
        }
	}
    
    /* Service MCWDT_Timer_4Hz interrupt */
    if(mcwdtInterruptFlag)
    {
        /* Reset interrupt flag */
        mcwdtInterruptFlag = false;
        if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING)
        {
            /* Toggle orange LED */
            InvertOrange;
        }
        else
        {
            /* Turn off orange LED */
            TurnOffOrange;
        }
        
        /* If no device is connected after advertising 30 sec go to hibernate mode */
        if(0 == Cy_BLE_GetNumOfActiveConn())
        {
            timeOutVar++;
            if(IsReadyForHibernate())
            {
                Cy_BLE_Stop();
                
                /* Turn on red LED */
                TurnOnRed;
                TurnOffOrange;
                DEBUG_PRINTF("Entering hibernate mode \r\n"); 
                DEBUG_WAIT_UART_TX_COMPLETE();
                
                Cy_SysPm_Hibernate();
                /* Code execution will not reach here */
                /* Device wakes up from hibernate and performs reset sequence 
                   when the reset switch or SW2 switch on the kit is pressed */
            }
        }
        else
        {
            timeOutVar = 0;
        }
    }
}

/*******************************************************************************
* Function Name: StackEventHandler
********************************************************************************
* Summary:
*  Call back event function to handle various events from the BLE stack
*
* Parameters:
*  event            :	event returned
*  eventParameter   :	link to value of the events returned
*
* Return:
*  void
*
*******************************************************************************/
void StackEventHandler(uint32 event, void* eventParam)
{
    /* Local variable */
    cy_en_ble_api_result_t apiResult;
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
            break;
        
        /* This event is received when Central device sends a Write command
           on an Attribute */
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            //DEBUG_PRINTF("CY_BLE_EVT_GATTS_WRITE_REQ \r\n");
            /* Read the write request parameter */
            writeReqParameter = (cy_stc_ble_gatts_write_cmd_req_param_t *) eventParam;
            DEBUG_PRINTF("WR vals = %d %d %d %d \r\n", writeReqParameter->handleValPair.value.val[0], writeReqParameter->handleValPair.value.val[1], writeReqParameter->handleValPair.value.val[2], writeReqParameter->handleValPair.value.val[3]);
            /* Set flag to process write request*/
            writeReqPending = true;
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
            }
            else if(Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_STOPPED)
            {
                DEBUG_PRINTF("Advertisement stopped \r\n");
                
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
            DEBUG_PRINTF("CY_BLE_EVT_GAP_DEVICE_DISCONNECTED\r\n");
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
            DEBUG_PRINTF("CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE \r\n");
            break;
        /**********************************************************
        *                       GATT Events
        ***********************************************************/
            
        /* This event is generated at the GAP Peripheral end after connection 
           is completed with peer Central device */
       case CY_BLE_EVT_GATT_CONNECT_IND:
            DEBUG_PRINTF("CY_BLE_EVT_GATT_CONNECT_IND: %x, %x \r\n", 
                        (*(cy_stc_ble_conn_handle_t *)eventParam).attId, 
                        (*(cy_stc_ble_conn_handle_t *)eventParam).bdHandle);
            DEBUG_PRINTF("Connected device: %u\r\n", Cy_BLE_GetNumOfActiveConn());
            /* Add connected device into database */
            appConnHandle[(*(cy_stc_ble_conn_handle_t *)eventParam).attId] = 
                                        *(cy_stc_ble_conn_handle_t *)eventParam;            
            break;
            
        /* This event is generated at the GAP Peripheral end after 
           disconnection */
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            DEBUG_PRINTF("CY_BLE_EVT_GATT_DISCONNECT_IND \r\n");
            
            /* Remove disconnected device from database*/
            appConnHandle[(*(cy_stc_ble_conn_handle_t *)eventParam).attId].attId = \
                                                    CY_BLE_INVALID_CONN_HANDLE_VALUE ;

            appConnHandle[(*(cy_stc_ble_conn_handle_t *)eventParam).attId].bdHandle = \
                                                    CY_BLE_INVALID_CONN_HANDLE_VALUE ;
            DEBUG_PRINTF("Connected device: %u\r\n", Cy_BLE_GetNumOfActiveConn());
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
* Function Name: CallBackHts
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component,
*  which are specific to Health Thermometer Service.
*
* Parameters:  
*  event:       Event for Health Thermometer Service.
*  eventParams: Event parameter for Health Thermometer Service.
*
* Return: 
*  None
*
*******************************************************************************/
void CallBackHts(uint32 event, void *eventParam)
{     
    switch(event)
    {
        /* This event is received when indication are enabled by the central */
        case CY_BLE_EVT_HTSS_INDICATION_ENABLED:
            printf ("HTS indication enabled \r\n");
            
            /* Tracking how many devices requested for HTS */
            htsRequestCount++;
            
            /* Set the htsIndication flag */
            htsIndication = true;
            break;

        /* This event is received when indication are enabled by the central */
        case CY_BLE_EVT_HTSS_INDICATION_DISABLED:
            printf ("HTS indication disabled \r\n");
            
            if(--htsRequestCount == 0)
            {
                /* Reset the htsIndiciation flag */
                htsIndication = false;
            }
            break;

        default:
          /* Error handling */
            break;
    }
    /* To avoid compiler warning */
    (void)eventParam;
}

/*******************************************************************************
* Function Name: ResetConnState
********************************************************************************
* Summary:
*  Reset all connection database
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ResetConnState(void)
{
    uint8_t i;
    for(i = 0; i < CY_BLE_CONN_COUNT; i++)
    {
        appConnHandle[i].attId      = CY_BLE_INVALID_CONN_HANDLE_VALUE;
        appConnHandle[i].bdHandle   = CY_BLE_INVALID_CONN_HANDLE_VALUE;
    }
}

/*******************************************************************************
* Function Name: MCWDT_Interrupt_Handler
********************************************************************************
* Summary:
*  Interrupt service routine for the MCWDT interrupt
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void MCWDT_Interrupt_Handler(void)
{
    /* Clear the MCWDT peripheral interrupt */
    Cy_MCWDT_ClearInterrupt(MCWDT_Timer_4Hz_HW, CY_MCWDT_CTR0);
    
    /* Set interrupt flag */
    mcwdtInterruptFlag = true;
    
    /* Clear the CM4 NVIC pending interrupt for MCWDT */
    NVIC_ClearPendingIRQ(MCWDT_isr_cfg.intrSrc);
}

/*******************************************************************************
* Function Name: IsReadyForHibernate
********************************************************************************
*
* Summary:
*  Evaluates the status of the system and enters hibernate mode if the conditions
*  permit
*
* Parameters:
*  bool : is device is ready for hibernate mode
*
* Return:
*  None
*
**********************************************************************************/
inline bool IsReadyForHibernate(void)
{
    return (timeOutVar >= TIME_OUT);
}

/*******************************************************************************
* Function Name: IsReadyForHibernate
********************************************************************************
*
* Summary: 
*  This function puts CM4 into deep sleep mode.
*  
*
* Parameters:
*  None
*
* Return:
*  None
*
**********************************************************************************/
void EnterLowPowerMode(void)
{    
    DEBUG_WAIT_UART_TX_COMPLETE();
    
    if(Cy_BLE_GetNumOfActiveConn() == 0) /* No device connected */
    {
        /* Configure deep sleep mode to wake up on interrupt */
        Cy_SysPm_DeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }
    else
    {
        /* Configure sleep mode to wake up on interrupt */
        Cy_SysPm_Sleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }
}

/* [] END OF FILE */
