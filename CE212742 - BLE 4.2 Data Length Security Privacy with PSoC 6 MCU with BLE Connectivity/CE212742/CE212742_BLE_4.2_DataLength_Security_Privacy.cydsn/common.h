/*******************************************************************************
* File Name: common.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project.
*
********************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined (_COMMON_H)
#define _COMMON_H
    
#include <project.h>
#include <stdio.h>

#if defined (__GNUC__)
    /* Add an explicit reference to the floating point printf library */
    /* to allow the usage of floating point conversion specifiers. */
    /* This is not linked in by default with the newlib-nano library. */
    asm (".global _printf_float");
#endif

#define ENABLED                     (1u)
#define DISABLED                    (0u)

/***************************************
* Conditional Compilation Parameters
***************************************/
#define DEBUG_UART_ENABLED              ENABLED
#define DEBUG_UART_SHOW_BOND_LIST       ENABLED    


/***************************************
*        API Constants
***************************************/
#define LED_ON                          (0u)
#define LED_OFF                         (1u)
#define RGB_LED_MIN_VOLTAGE_MV          (2700u)

#define CY_BLE_PVT_ADDRESS_TIMEOUT      (900u)      /* It is recommended to set the timer to 15 minutes */
#define CY_BLE_TIMER_TIMEOUT            (10u)       /* 10 sec */

#define CY_BLE_CONN_INTRV_TO_MS         (5.0f / 4.0f)
    
#define CY_BLE_NOTIFICATION_HEADER_LEN  (0x03u)

#define CY_BLE_GAP_KEY_FLAG_LII         (0x02u)

#define PACKET_LENGTH_REDUCE            (0x05u)

#define ADV_TIMER_TIMEOUT               (1u)              /* Сounts in s */

/***************************************
* Function Prototypes
***************************************/
int HostMain(void);
void UpdateLedState(void);

/* Function Prototypes from debug.c */
void ShowValue(cy_stc_ble_gatt_value_t *value);
char HexToAscii(uint8_t value, uint8_t nibble);
void Set32ByPtr(uint8_t ptr[], uint32_t value);
void PrintStackVersion(void);
void PrintApiResult(cy_en_ble_api_result_t apiResult);
void ShowError(void);

/***************************************
*        Macros
***************************************/
#if (DEBUG_UART_ENABLED == ENABLED)
    #define DBG_PRINTF(...)                 (printf(__VA_ARGS__))
    #define UART_DEB_PUT_CHAR(...)           while(1UL != UART_DEB_Put(__VA_ARGS__))
    #define UART_DEB_GET_CHAR(...)          (UART_DEB_Get())
    #define UART_DEB_IS_TX_COMPLETE(...)    (UART_DEB_IsTxComplete())
    #define UART_DEB_WAIT_TX_COMPLETE(...)   while(UART_DEB_IS_TX_COMPLETE() == 0) ;    
    #define UART_DEB_SCB_CLEAR_RX_FIFO(...) (Cy_SCB_ClearRxFifo(UART_DEB_SCB__HW))
    #define UART_START(...)                 (UART_DEB_Start(__VA_ARGS__))
#else
    #define DBG_PRINTF(...)
    #define UART_DEB_PUT_CHAR(...)
    #define UART_DEB_GET_CHAR(...)          (0u)
    #define UART_DEB_IS_TX_COMPLETE(...)    (1u)
    #define UART_DEB_WAIT_TX_COMPLETE(...)  (0u)
    #define UART_DEB_SCB_CLEAR_RX_FIFO(...) (0u)
    #define UART_START(...)
#endif /* (DEBUG_UART_ENABLED == ENABLED) */

#define UART_DEB_NO_DATA                (char8) CY_SCB_UART_RX_NO_DATA


/* Set Disconnect LED  */
#define Disconnect_LED_Write(value) \
                do{       \
                    Cy_GPIO_Write(Disconnect_LED_0_PORT, Disconnect_LED_0_NUM, value); \
                }while(0)
        
/* Set Advertise LED  */
#define Advertising_LED_Write(value) \
                do{       \
                    Cy_GPIO_Write(Advertising_LED_0_PORT, Advertising_LED_0_NUM, value); \
                }while(0)

#define Advertising_LED_INV()     Cy_GPIO_Inv(Advertising_LED_0_PORT, Advertising_LED_0_NUM) 

/* Set Simulation LED  */
#define Simulation_LED_Write(value) \
                do{       \
                    Cy_GPIO_Write(Simulation_LED_0_PORT, Simulation_LED_0_NUM, value); \
                }while(0)
 
/* Set LED5  */
#define LED5_Write(value) \
                do{       \
                    Cy_GPIO_Write(Disconnect_LED_0_PORT, Disconnect_LED_0_NUM, value); \
                }while(0)

#define LED5_INV()     Cy_GPIO_Inv(Disconnect_LED_0_PORT, Disconnect_LED_0_NUM)        


/* Read SW2 pin */
#define SW2_Read()        Cy_GPIO_Read(SW2_0_PORT, SW2_0_NUM)

/* Leds operations */
#define EnableAllLeds() \
                do{       \
                    Disconnect_LED_Write(LED_ON);\
                    Advertising_LED_Write(LED_ON);\
                    Simulation_LED_Write(LED_ON);\
                }while(0)

#define DisableAllLeds() \
                do{       \
                    Disconnect_LED_Write(LED_OFF);\
                    Advertising_LED_Write(LED_OFF);\
                    Simulation_LED_Write(LED_OFF);\
                }while(0)

/***************************************
* External data references
***************************************/
extern cy_stc_ble_conn_handle_t appConnHandle;
extern volatile uint32_t totalByteCounter;
#endif

/* [] END OF FILE */
