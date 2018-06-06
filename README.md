## Table of Contents

* [Introduction](#introduction)
* [Navigating the Repository](#navigating-the-repository)
* [Required Tools](#required-tools)
* [Code Examples List](#code-examples-list)
* [References](#references)

# Introduction
This repository contains examples and demos for PSoC 6 MCU family of devices, a single chip solution for the emerging IoT devices. PSoC 6 MCU bridges the gap between expensive, power hungry application processors and low‑performance microcontrollers (MCUs). The ultra‑low‑power, dual-core architecture of PSoC 6 MCU offers the processing performance needed by IoT devices, eliminating the tradeoffs between power and performance.

Cypress provides a wealth of data at [www.cypress.com](http://www.cypress.com/) to help you select the right PSoC device and effectively integrate it into your design. Visit our [PSoC 6 MCU](http://www.cypress.com/products/32-bit-arm-cortex-m4-psoc-6) webpage to explore more about PSoC 6 MCU family of device.
Feel free to explore through the code example source files and let us innovate together!

# Navigating the Repository

The examples in this repository demonstrates PSoC 6 MCU with BLE Connectivity based Bluetooth Low Energy (BLE) connectivity feature. These code examples show how to integrate the complex BLE protocol into your design effortlessly.
If you are new to developing projects with PSoC 6 MCU, we recommend you to refer the [PSoC 6 Getting Started GitHub](https://github.com/cypresssemiconductorco/PSoC-6-MCU-Getting-Started) page which can help you familiarize with device features and guides you to create a simple PSoC 6 design with PSoC Creator IDE. For other block specific design please visit the following GitHub Pages:

For block level examples please refer to the following GitHub repositories:

#### 1. [Analog Designs]
#### 2. [Digital Designs]
#### 3. [Audio Designs]
#### 4. [Device Related Designs]
#### 5. [System-Level Designs]
#### 6. [PSoC 6 Pioneer Kit Designs]

You can use these block level examples to guide you through the development of a system-level design using PSoC 6 MCU. All the code examples in this repository comes with well documented design guidelines to help you understand the design and how to develop it. The code examples and their associated documentation are in the Code Example folder in the repository.

# Required Tools

## Software
### Integrated Development Environment (IDE)
To use the code examples in this repository, please download and install
[PSoC Creator](http://www.cypress.com/products/psoc-creator)

## Hardware
### PSoC 6 MCU Development Kits
* [CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit](http://www.cypress.com/documentation/development-kitsboards/psoc-6-ble-pioneer-kit).

* [CY8CKIT-062 PSoC 6 WiFi-BT Pioneer Kit](http://www.cypress.com/documentation/development-kitsboards/psoc-6-wifi-bt-pioneer-kit). 

**Note** Please refer to the code example documentation for selecting the appropriate kit for testing the project

## Code Example List
#### 1. CE212736 - PSoC 6 MCU with BLuetooth Low Energy (BLE) - Find Me
This code example demonstrates the implementation of a simple BLE Immediate Alert Service (IAS)-based Find Me profile using PSoC® 6 MCU with BLE Connectivity.
#### 2. CE212741 - IPSP Router and Node with PSoC 6 MCU with BLE Connectivity
This example demonstrates how to set up the IPv6 communication infrastructure between two devices (CY8CKIT-062 PSoC 6 BLE Pioneer Kits) over a BLE transport using the L2CAP channel. Creation and transmission of IPv6 packets over the BLE is not part of this example.
The example consists of two projects: IPSP Router (GAP Central) and IPSP Node (GAP Peripheral). The router sends generated packets with different content to the node in a loop and validates them with the data packet received afterward. The node wraps the received data coming from the router back to the router.
#### 3. CE215118 – BLE Multi-Master Single Slave with PSoC 6 MCU with BLE Connectivity
This example demonstrates how to configure the PSoC® 6 MCU with Bluetooth Low Energy (BLE) Connectivity device in
simultaneous Multiple Master and Single Slave modes of operation. The Multi-Master Single Slave project uses three BLE Central connections and one Peripheral connection.
#### 4. CE212742 – BLE 4.2 Data Length Security Privacy with PSoC 6 MCU with BLE Connectivity
This code example demonstrates the new BLE 4.2 and 5.0 features of the PSoC® Creator™ BLE Component.
#### 5. CE215119 - BLE Battery Level with PSoC 6 MCU with BLE Connectivity
This example demonstrates the operation of Bluetooth Low Energy (BLE) Battery Service (BAS) using the BLE_PDL Component. The design implements a Custom Profile in a GATT server and generic attribute profile (GAP) peripheral roles with the Battery and Device Information services. The Battery Service is used for software simulation of the battery level. The simulated battery level value is continuously changed from 2 to 20 percent.
#### 6. CE215121 - BLE HID Keyboard with PSoC 6 MCU with BLE Connectivity
This example demonstrates the implementation of the Bluetooth Low Energy (BLE) HID over GATT Profile where the device operates as a HID keyboard. The design demonstrates the core functionality of the BLE Component configured as a HID Device (GATT Server). It simulates keyboard press in Boot and Protocol modes. Also, the design demonstrates how to handle a suspend event from the central device and enter Low-Power mode when suspended.
#### 7. CE215122 - BLE HID Mouse with PSoC 6 MCU with BLE Connectivity
This example demonstrates the implementation of the Bluetooth Low Energy (BLE) HID over GATT Profile where the device operates as a HID Mouse. The design demonstrates the core functionality of the BLE Component configured as a HID Device (GATT Server). It simulates the mouse movement and button click in Boot and Protocol modes. Also, the design demonstrates how to handle a suspend event from the central device and enter Low-Power mode when suspended.
#### 8. CE215123 - BLE Location and Navigation with PSoC 6 MCU with BLE Connectivity
This example project demonstrates the Location and Navigation Pod application workflow. The design demonstrates the core functionality of the Bluetooth Low Energy (BLE) Component configured as a BLE Location and Navigation Service (LNS) device in the GATT Server role. The application uses a BLE Location and Navigation Profile to report location and navigation information to a Client. Also, the Location and Navigation Pod application uses the Battery Service to notify the battery level and the Device Information Service to assert the Device Name and so on.
#### 9. CE215124 - BLE HTTP Proxy with PSoC 6 MCU with BLE Connectivity
This example demonstrates the HTTP Proxy Client and Server operation of the Bluetooth Low Energy (BLE) PSoC Creator™ Component. The HTTP Proxy Server and HTTP Proxy Client projects are used as a pair to demonstrate the BLE HTTP Proxy Service (HPS) operation. The HTTP Proxy Server uses one instance of the HTTP Proxy Service to simulate an HTTP Server on the BLE device. Also, the HTTP Proxy Server operates with other devices that implement the HTTP Proxy Client Role. To conserve power, the device switches to Deep Sleep mode between the BLE connection intervals.
#### 10. CE215555 - BLE Multi Role with PSoC 6 MCU with BLE Connectivity
This example demonstrates the capability of the PSoC 6 BLE device to be in all Generic Access Profile (GAP) roles (Central,
Peripheral, Observer, and Broadcaster) simultaneously.
#### 11. CE217639 - BLE Heart Rate with PSoC 6 MCU with BLE Connectivity 
This example demonstrates the Heart Rate Client and Server operation of the Bluetooth Low Energy (BLE) PSoC Creator™ Component.
The Heart Rate Server and Heart Rate Client projects are used in a pair to demonstrate the Heart Rate Service (HRS) operation. The Heart Rate Server project demonstrates the BLE workflow procedures such as advertising, connecting, simulating, and notifying Heart Rate data and Battery Level. To conserve power, the device switches to Deep Sleep mode between the BLE connection intervals.
The Heart Rate Client project receives Heart Rate data from any BLE-enabled Heart Rate Sensor and indicates that data on any terminal software via a UART.
#### 12. CE217645 - BLE Weight Scale Profile with PSoC 6 MCU with BLE Connectivity
This example demonstrates how to configure and use BLE Component API functions and application layer callback for the BLE Weight Scale application. The design demonstrates the Weight Scale Profile operation of the BLE Component. The Weight Scale Sensor uses one instance of the Weight Scale Service (WSS), Body Composition Service (BCS), User Data Service (UDS), and Device Information Service (DIS) to simulate weight measurements for up to four registered users. The Weight Scale Sensor operates with other devices that implement the Weight Scale Collector Profile.
#### 13. CE218044 - BLE Pulse Oximeter with PSoC 6 MCU with BLE Connectivity
This example demonstrates how to configure and use the Bluetooth Low Energy (BLE) Component APIs and application layer
callbacks for the Pulse Oximeter Profile (PLXP). This example demonstrates the core functionality of the BLE Component configured as a Pulse Oximeter Service (PLXS) device (GATT Server). The example simulates the PLX Spot-check Measurement and PLX Continuous Measurement
characteristics. To conserve power, the device switches to Deep Sleep mode between the BLE connection intervals.
Additionally, this project implements the following services as per the Pulse Oximeter Profile specification: BMS, BAS, DIS,
and CTS. 
#### 14. CE222004 – PSoC 6 MCU with BLE Multi-Master Multi-Slave: SSSS Function
This example demonstrates the implementation of multi-slave functionality of the PSoC® 6 MCU with BLE Connectivity (PSoC 6 BLE) device. The example shows the connectivity between the PSoC 6 BLE, acting as a Peripheral and GATT Server, and four BLE enabled devices (a personal computer running the CySmart BLE Host Emulation tool or mobile device running the CySmart mobile application) acting as a Central and GATT Client

## References
#### 1. PSoC 6 MCU
PSoC 6 bridges the gap between expensive, power hungry application processors and low‑performance microcontrollers (MCUs). The ultra‑low‑power PSoC 6 MCU architecture offers the processing performance needed by IoT devices, eliminating the tradeoffs between power and performance. The PSoC 6 MCU contains a dual‑core architecture, with both cores on a single chip. It has an Arm® Cortex®‑M4 for high‑performance tasks, and an Arm® Cortex®‑M0+ for low-power tasks, and with security built-in, your IoT system is protected.
To learn more on the device, please visit our [PSoC 6 MCU](http://www.cypress.com/products/32-bit-arm-cortex-m4-psoc-6) webpage.

####  2. PSoC 6 MCU Learning resource list
##### 2.1 PSoC 6 MCU Datasheets
Device datasheets list the features and electrical specifications of PSoC 6 families of devices: [PSoC 6 MCU Datasheets](http://www.cypress.com/search/all?f%5B0%5D=meta_type%3Atechnical_documents&f%5B1%5D=resource_meta_type%3A575&f%5B2%5D=field_related_products%3A114026)
##### 2.2 PSoC 6 MCU Application Notes
Application notes are available on the Cypress website to assist you with designing your PSoC application: [A list of PSoC 6 MCU ANs](http://www.cypress.com/psoc6an)
##### 2.3 PSoC 6 MCU Component Datasheets
PSoC Creator utilizes "components" as interfaces to functional Hardware (HW). Each component in PSoC Creator has an associated datasheet that describes the functionality, APIs, and electrical specifications for the HW. You can access component datasheets in PSoC Creator by right-clicking a component on the schematic page or by going through the component library listing. You can also access component datasheets from the Cypress website: [PSoC 6 Component Datasheets](http://www.cypress.com/documentation/component-datasheets)
##### 2.4 PSoC 6 MCU Technical Reference Manuals (TRM)
The TRM provides detailed descriptions of the internal architecture of PSoC 6 devices:[PSoC 6 MCU TRMs](http://www.cypress.com/psoc6trm)

## FAQ

### Technical Support
If you have any queries or questions, our technical support team would be happy to assist you. You can create a support request at [This link](https://secure.cypress.com/myaccount/?id=25&techSupport=1), or if you are in the United States, you can talk to our technical support team by calling our toll-free number +1-800-541-4736 and then selecting option 8 at the IVR prompt. 

You can also use the following support resources if you need quick assistance: 
##### Self-help: [Technical Support](http://www.cypress.com/support)
##### Local Sales office locations: [Sales Office](http://www.cypress.com/about-us/sales-offices)
