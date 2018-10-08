/*******************************************************************************
* File Name: debug.c
*
* Version: 1.0
*
* Description:
*  This file contains functions for printf functionality 
*  and LED status notification.
*
* Hardware Dependency:
*  CY8CKIT-062 PSoC6 BLE Pioneer Kit
* 
********************************************************************************
* Copyright 2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "debug.h"

#if (DEBUG_UART_ENABLED == ENABLED)

#if defined(__ARMCC_VERSION)
    
/* For MDK/RVDS compiler revise fputc function for printf functionality */
struct __FILE 
{
    int handle;  
};

enum 
{
    STDIN_HANDLE,
    STDOUT_HANDLE,
    STDERR_HANDLE
};

FILE __stdin = {STDIN_HANDLE};
FILE __stdout = {STDOUT_HANDLE};
FILE __stderr = {STDERR_HANDLE};

int fputc(int ch, FILE *file) 
{
    int ret = EOF;

    switch( file->handle )
    {
        case STDOUT_HANDLE:
            UART_DEB_PUT_CHAR(ch);
            ret = ch ;
            break ;

        case STDERR_HANDLE:
            ret = ch ;
            break ;

        default:
            file = file;
            break ;
    }
    return ret ;
}

#elif defined (__ICCARM__)      /* IAR */

/* For IAR compiler revise __write() function for printf functionality */
size_t __write(int handle, const unsigned char * buffer, size_t size)
{
    size_t nChars = 0;
    (void) handle;

    if (buffer == 0)
    {
        /*
         * This means that we should flush internal buffers.  Since we
         * don't we just return.  (Remember, "handle" == -1 means that all
         * handles should be flushed.)
         */
        return (0);
    }

    for (/* Empty */; size != 0; --size)
    {
        UART_DEB_PUT_CHAR(*buffer);
        ++buffer;
        ++nChars;
    }

    return (nChars);
}

#else  /* (__GNUC__)  GCC */

/* For GCC compiler revise _write() function for printf functionality */
int _write(int file, char *ptr, int len)
{
    int i;
    file = file;
    for (i = 0; i < len; i++)
    {
        UART_DEB_PUT_CHAR(*ptr);
        ++ptr;
    }
    return len;
}


#endif  /* (__ARMCC_VERSION) */   

#endif /* DEBUG_UART_ENABLED == ENABLED */

#if (DEBUG_LED_ENABLED == ENABLED)

void InitLED(void)
{
    #if CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV
    Cy_GPIO_SetDrivemode(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM, CY_GPIO_DM_STRONG_IN_OFF);
    Cy_GPIO_Write(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM, LED_OFF);  
    Cy_GPIO_SetDrivemode(PIN_LED_GREEN_0_PORT, PIN_LED_GREEN_0_NUM, CY_GPIO_DM_STRONG_IN_OFF);
    Cy_GPIO_Write(PIN_LED_GREEN_0_PORT, PIN_LED_GREEN_0_NUM, LED_OFF);    
    Cy_GPIO_SetDrivemode(PIN_LED_BLUE_0_PORT, PIN_LED_BLUE_0_NUM, CY_GPIO_DM_STRONG_IN_OFF);
    Cy_GPIO_Write(PIN_LED_BLUE_0_PORT, PIN_LED_BLUE_0_NUM, LED_OFF);
    #else
    Cy_GPIO_SetDrivemode(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM, CY_GPIO_DM_STRONG_IN_OFF); 
    Cy_GPIO_Write(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM, LED_OFF);    
    #endif  
}

void HibernateLED(void)
{
    #if CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV
    Cy_GPIO_Write(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM, LED_ON);    
    Cy_GPIO_Write(PIN_LED_GREEN_0_PORT, PIN_LED_GREEN_0_NUM, LED_OFF);    
    Cy_GPIO_Write(PIN_LED_BLUE_0_PORT, PIN_LED_BLUE_0_NUM, LED_OFF);
    #else
    Cy_GPIO_Write(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM, LED_ON);    
    #endif
}

void BlinkLED(void)
{
    #if CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV
    Cy_GPIO_Inv(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM);    
    Cy_GPIO_Inv(PIN_LED_GREEN_0_PORT, PIN_LED_GREEN_0_NUM);    
    Cy_GPIO_Inv(PIN_LED_BLUE_0_PORT, PIN_LED_BLUE_0_NUM);
    #else
    Cy_GPIO_Inv(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM);    
    #endif  
}

void ConnectedLED(void)
{
    #if CYDEV_VDDD_MV >= RGB_LED_MIN_VOLTAGE_MV
    Cy_GPIO_Write(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM, LED_OFF);    
    Cy_GPIO_Write(PIN_LED_GREEN_0_PORT, PIN_LED_GREEN_0_NUM, LED_OFF);    
    Cy_GPIO_Write(PIN_LED_BLUE_0_PORT, PIN_LED_BLUE_0_NUM, LED_OFF);
    #else
    Cy_GPIO_Write(PIN_LED_RED_0_PORT, PIN_LED_RED_0_NUM, LED_OFF);    
    #endif    
}

#endif /* DEBUG_LED == ENABLED */

/* [] END OF FILE */
