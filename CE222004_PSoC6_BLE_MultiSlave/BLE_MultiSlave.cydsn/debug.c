/*******************************************************************************
* File Name: debug.c
*
* Version: 1.00
*
* Description:
*  This file contains functions for debug and UART functionality.
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
                    UART_DEBUG_PUT_CHAR(ch);
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
                UART_DEBUG_PUT_CHAR(*buffer, 0x01);
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
                UART_DEBUG_PUT_CHAR(*ptr);
                ++ptr;
            }
            return len;
        }

    #endif  /* (__ARMCC_VERSION) */   

#endif /* DEBUG_UART_ENABLED == ENABLED */

/*******************************************************************************
* Function Name: ShowError()
********************************************************************************
*
* Summary:
*   Shows error condition: Turn ON all leds - white colour will indicate error.
*
*******************************************************************************/
void ShowError(void)
{
    DEBUG_PRINTF("Press Reset switch (SW1) to reset the device\n\r");
    /* Halt CPU */
    CY_ASSERT(0u != 0u);
}

/* [] END OF FILE */
