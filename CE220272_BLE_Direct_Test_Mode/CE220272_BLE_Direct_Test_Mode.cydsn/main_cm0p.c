/*****************************************************************************
* File Name: main_cm0p.c
*
* Version: 1.20
*
* Description: This code example demonstrates Direct Test Mode (DTM) over the
* Host Controller Interface (HCI) using PSoC 6 BLE.
*
* Related Document: CE220272_BLE_Direct_Test_Mode.pdf
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
*
*******************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation. All rights reserved.
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
#include "project.h"

/* BLE Callback Event Handler Function */
void GenericEventHandler(uint32 event, void * eventparam);

/*******************************************************************************
* Function Name: int main()
********************************************************************************
* Summary:
* Main function that initializes the system and continuously process the BLE
* events.     
*
* Parameters:
*  None
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    __enable_irq(); /* Enable global interrupts. */
        
    /*Start the BLE Component*/
    Cy_BLE_Start(GenericEventHandler);

    for(;;)
    {
        Cy_BLE_ProcessEvents();
        
    }
}

/*******************************************************************************
* Function Name: GenericEventHandler
********************************************************************************
* Summary:
*        Call back event function to handle various events from BLE stack. Not 
* used for HCI mode as BLE stack handles all events.
*
* Parameters:
*  event:		event returned
*  eventParam:	link to value of the events returned
*
* Return:
*  void
*
*******************************************************************************/
void GenericEventHandler(uint32 event, void * eventparam)
{
	switch(event)
	{
		/* No application event is generated in HCI Mode. All commands
		* are processed by BLE stack */

		default:
			/* To prevent compiler warning */
			eventparam = eventparam;
		break;
	}
}

/* [] END OF FILE */
