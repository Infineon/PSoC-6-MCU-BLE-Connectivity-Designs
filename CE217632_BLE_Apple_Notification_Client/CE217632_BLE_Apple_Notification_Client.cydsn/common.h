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
* Copyright (2018), Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>
#include <stdio.h>


/***************************************
* Conditional Compilation Parameters
***************************************/
#define ENABLED                      (1u)
#define DISABLED                     (0u)

#define DEBUG_UART_ENABLED           ENABLED
#define DEBUG_UART_FULL              DISABLED
#define CONN_COUNT                   (4u)             /* up to CY_BLE_CONN_COUNT */


/***************************************
*           API Constants
***************************************/
#define CY_BLE_CONN_INTRV_TO_MS      (5 / 4)

#define ADV_TIMER_TIMEOUT            (1u)             /* Сounts in s */

#define SW2_ZERO_PRESSING            (0u)    
#define SW2_ONE_PRESSING             (1u)
#define SW2_TWO_PRESSING             (2u)
#define SW2_PRESS_MASK               (0x7fu)
#define SW2_READY                    (0x80u)

#define TIMER_STOP                   (0u)    
#define TIMER_START                  (1u)
#define SW2_DEBOUNCING_TIME          (100u)          /* Counts in ms */
#define SW2_POLLING_TIME             (500u)          /* Counts in ms */


/***************************************
*        External Function Prototypes
***************************************/
/* Function Prototypes from host_main.c */
int HostMain(void);
void UpdateLedState(void);

/* Function Prototypes from debug.c */
void ShowValue(cy_stc_ble_gatt_value_t *value);
char HexToAscii(uint8_t value, uint8_t nibble);
void Set32ByPtr(uint8_t ptr[], uint32_t value);
void PrintStackVersion(void);
void PrintApiResult(cy_en_ble_api_result_t apiResult);
void ShowError(void);

/* Function prototypes from bond.c */
void App_DisplayBondList(void);
void App_RemoveDevicesFromBondListBySW2Press(uint32_t seconds);
void App_RemoveDevicesFromBondList(void);
void App_SetRemoveBondListFlag(void);
bool App_IsRemoveBondListFlag(void);
bool App_IsDeviceInBondList(uint32_t bdHandle);
uint32_t App_GetCountOfBondedDevices(void);


/***************************************
*        Macros
***************************************/
#if (DEBUG_UART_ENABLED == ENABLED)
    #define UART_DEB_NO_DATA                (char8) CY_SCB_UART_RX_NO_DATA
    #define DBG_PRINTF(...)                 (printf(__VA_ARGS__))
    #define UART_DEB_PUT_CHAR(...)           while(1UL != UART_DEB_Put(__VA_ARGS__))
    #define UART_DEB_GET_CHAR(...)          (UART_DEB_Get())
    #define UART_DEB_IS_TX_COMPLETE(...)    (UART_DEB_IsTxComplete())
    #define UART_DEB_WAIT_TX_COMPLETE(...)   while(UART_DEB_IS_TX_COMPLETE() == false);    
    #define UART_DEB_SCB_CLEAR_RX_FIFO(...) (Cy_SCB_ClearRxFifo(UART_DEB_SCB__HW))
    #define UART_START(...)                 (UART_DEB_Start(__VA_ARGS__))
#else
    #define UART_DEB_NO_DATA                (0u)
    #define DBG_PRINTF(...)
    #define UART_DEB_PUT_CHAR(...)
    #define UART_DEB_GET_CHAR(...)          (0u)
    #define UART_DEB_IS_TX_COMPLETE(...)    (true)
    #define UART_DEB_WAIT_TX_COMPLETE(...)  
    #define UART_DEB_SCB_CLEAR_RX_FIFO(...) 
    #define UART_START(...)
#endif /* (DEBUG_UART_ENABLED == ENABLED) */

/***************************************
* External data references
***************************************/
/* extern from host_main.c */
extern cy_stc_ble_conn_handle_t  appConnHandle;
extern volatile uint8_t button;

/* [] END OF FILE */
