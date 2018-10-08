/******************************************************************************
* File Name: eddystone_config.h
*
* Version: 1.10
*
* Description: This file contains macros used for Eddystone packet 
*              configuration. 
*
* Related Document: CE224856_BLE_Low_Power_Beacon_with_Hibernate_RTOS.pdf
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
/******************************************************************************
* This file contains macros used for Eddystone packet configuration. For details
* of Eddystone, see:
*
* https://developers.google.com/beacons/eddystone
*
* For details of the core Eddystone frame types (URL, UID and TLM), see the 
* following documentation:
*
* https://github.com/google/eddystone/tree/master/eddystone-url
* https://github.com/google/eddystone/tree/master/eddystone-uid
* https://github.com/google/eddystone/tree/master/eddystone-tlm
*
********************************************************************************/

/* Include Guard */
#ifndef EDDYSTONE_CONFIG_H
#define EDDYSTONE_CONFIG_H
    
/*~~~~~~~~~~~~~~~~~~~~~~ IMPLEMENTATION SETTINGS ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    
/* Use this macro value to select an Eddystone implementation (URL/UID).
   Valid options are  EDDYSTONE_URL and EDDYSTONE_UID */
#define EDDYSTONE_IMPLEMENTATION        EDDYSTONE_URL

/* UID/URL frame timeout in seconds */
#define APP_UID_URL_TIMEOUT             (0x0Au)
    
/* TLM frame timeout in seconds */
#define APP_TLM_TIMEOUT                 (0x01u)
 
/*~~~~~~~~~~~~~~~~~~~~~~ GENERIC PACKET SETTINGS ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    
/* Service Solicitation data for all packet types */
#define SERVICE_SOLICITATION            {0x03u, /* Length */ \
                                         0x03u, /* Number of Service UUIDs */\
                                         0xAAu, /* LSB - Eddystone service */\
                                         0xFEu} /* MSB - Eddystone service */\

/* Eddystone packet index of the Solicitation data */    
#define SOLICITATION_INDEX              (0x03u)
    
/*~~~~~~~~~~~~~~~~~~~~~~ UID PACKET SETTINGS ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* Service data for the UID frame */    
#define UID_SERVICE_DATA                {0x17u, /* Length */\
                                         0x16u, /* Service Data */\
                                         0xAAu, /* LSB - Eddystone Service */\
                                         0xFEu, /* MSB - Eddystone Service */\
                                         0x00u, /* Signifies Eddystone UID */\
                                         0xF2u} /* Ranging data: -18dB*/
/* Note: Per Eddystone specification, the ranging data should be set by measuring 
   the actual output of the beacon from 1 meter away and then adding 41dBm to 
   the measured value */    

/* Eddystone packet index of the UID service data */       
#define UID_SERVICE_INDEX               (0x07u)
                                        
/* SHA-1 hash of the FQDN (cypress.com) is calculated and its first 10 bytes
   are placed here as the Name-space IDU, with MSB coming first */
#define NAME_SPACE_ID                   {0xCBu, /* NID[0] */\
                                         0x6Fu, /* NID[1] */\
                                         0x15u, /* NID[2] */\
                                         0xCEu, /* NID[3] */\
                                         0x20u, /* NID[4] */\
                                         0x2Au, /* NID[5] */\
                                         0xCEu, /* NID[6] */\
                                         0x15u, /* NID[7] */\
                                         0x6Fu, /* NID[8] */\
                                         0xCBu} /* NID[9] */  

/* Eddystone packet index of the name space ID */ 
#define NAME_SPACE_INDEX                (0x0Du)

/* Instance ID - you can put a randomly generated value here */                                        
#define INSTANCE_ID                     {0x01u, /* BID[0] */\
                                         0x01u, /* BID[1] */\
                                         0x01u, /* BID[2] */\
                                         0x01u, /* BID[3] */\
                                         0x01u, /* BID[4] */\
                                         0x01u} /* BID[5] */

/* Eddystone packet index of the instance ID */     
#define INSTANCE_INDEX                  (0x17u)


/* Data written to reserved Eddystone fields */    
#define RESERVED_FIELD_VALUE            (0x00u)
 
/* Eddystone packet indexes of the reserved fields */     
#define RESERVED_INDEX_0                (0x1Du)
#define RESERVED_INDEX_1                (0x1Eu)  
    
/* Total size of a UID frame */    
#define UID_PACKET_SIZE                 (0x1Fu)      
    
/*~~~~~~~~~~~~~~~~~~~~~~ URL PACKET SETTINGS ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* URL frame data packet for http://www.cypress.com  */    
#define URL_PACKET_DATA                 {0x0Eu, /* Length */\
                                         0x16u, /* Service Data */\
                                         0xAAu, /* LSB - Eddystone Service */\
                                         0xFEu, /* MSB - Eddystone Service */\
                                         0x10u, /* Signifies Eddystone URL */\
                                         0xF2u, /* Ranging data: -18dB*/\
                                         0x00u, /* URL scheme prefix- http://www. */\
                                         0x63u, /* Encoded URL - 'c' */\
                                         0x79u, /* Encoded URL - 'y' */\
                                         0x70u, /* Encoded URL - 'p' */\
                                         0x72u, /* Encoded URL - 'r' */\
                                         0x65u, /* Encoded URL - 'e' */\
                                         0x73u, /* Encoded URL - 's' */\
                                         0x73u, /* Encoded URL - 's' */\
                                         0x00u} /* Expansion - .com */

/* Eddystone packet index of the URL data */     
#define URL_INDEX                       (0x07u)

/* Total size of a URL frame */     
#define URL_PACKET_SIZE                 (0x16u)  
    
/*~~~~~~~~~~~~~~~~~~~~~~ TLM PACKET SETTINGS ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define TLM_SERVICE_DATA                {0x11u, /* Length */\
                                         0x16u, /* Service Data */\
                                         0xAAu, /* LSB - Eddystone Service */\
                                         0xFEu, /* MSB - Eddystone Service */\
                                         0x20u, /* Signifies Eddystone TLM */\
                                         0x00u} /* TLM version */\
    
/* Eddystone packet index of the TLM service data */                                         
#define TLM_SERVICE_INDEX               (0x07u)
                                        
/* Battery voltage in mV (1 mV per bit). Set at "N/A" */ 
#define BATTERY_VOLTAGE_MSB             (0x00u) 
#define BATTERY_VOLTAGE_LSB             (0x00u) 

/* Eddystone packet indexes of battery data */     
#define BATTERY_MSB_INDEX               (0x0Du) 
#define BATTERY_LSB_INDEX               (0x0Eu)     

/* Eddystone packet indexes of temperature data */       
#define TEMPERATURE_MSB_INDEX           (0x0Fu) 
#define TEMPERATURE_LSB_INDEX           (0x10u) 

/* Beacon temperature in Celsius (8.8 fixed point notation).
   Set at 25 degree Celsius */
#define BEACON_TEMPERATURE_MSB          (0x19u)     
#define BEACON_TEMPERATURE_LSB          (0x00u)     

/* Eddystone packet indexes of packet count information */       
#define PACKET_COUNT_INDEX_0            (0x11u)
#define PACKET_COUNT_INDEX_1            (0x12u)
#define PACKET_COUNT_INDEX_2            (0x13u)
#define PACKET_COUNT_INDEX_3            (0x14u)

/* Eddystone packet indexes of time information */        
#define SECONDS_INDEX_0                 (0x15u)
#define SECONDS_INDEX_1                 (0x16u)
#define SECONDS_INDEX_2                 (0x17u)
#define SECONDS_INDEX_3                 (0x18u)

/* Macros used to swap the endian of 4-byte data used in Eddystone 
   TLM packets */        
#define TLM_4B_ENDIAN_SWAP_0            (0x03u)
#define TLM_4B_ENDIAN_SWAP_1            (0x02u)
#define TLM_4B_ENDIAN_SWAP_2            (0x01u)
#define TLM_4B_ENDIAN_SWAP_3            (0x00u)
    
/* Total size of a TLM frame */     
#define TLM_PACKET_SIZE                 (0x19u)  

#endif    
/* [] END OF FILE */
