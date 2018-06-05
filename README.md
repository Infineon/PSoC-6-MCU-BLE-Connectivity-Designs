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
