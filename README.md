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
#### 1. [Analog Designs](https://github.com/cypresssemiconductorco/PSoC-6-MCU-Analog-Designs)
#### 2. [Digital Designs](https://github.com/cypresssemiconductorco/PSoC-6-MCU-Digital-Designs)
#### 3. [Audio Designs](https://github.com/cypresssemiconductorco/PSoC-6-MCU-Audio-Designs)
#### 4. [Device Related Designs](https://github.com/cypresssemiconductorco/PSoC-6-MCU-Device-Related-Design)
#### 5. [System-Level Designs](https://github.com/cypresssemiconductorco/PSoC-6-MCU-System-Level-Designs)
#### 6. [PSoC 6 Pioneer Kit Designs](https://github.com/cypresssemiconductorco/PSoC-6-MCU-Pioneer-Kits)
#### 7. [PSoC 6 MCU based RTOS Designs](https://github.com/cypresssemiconductorco/PSoC-6-MCU-RTOS-Based-Design)

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

# Code Examples List
#### 1. CE212736 - PSoC 6 MCU with BLE Connectivity - Find ME
This code example demonstrates the implementation of a simple BLE Immediate Alert Service (IAS)-based Find Me profile using PSoC 6 MCU with BLE Connectivity.
#### 2. CE212741 - IPSP Router and Node
This example demonstrates the operation of the Internet Protocol Support Profile (IPSP) with the Bluetooth Low Energy (BLE_PDL) Component.
#### 3. CE212742 - BLE 4.2 Data Length Security Privacy with PSoC 6 MCU with BLE Connectivity
This code example demonstrates the new BLE 4.2 and 5.0 features of the PSoC® Creator BLE Component.
#### 4. CE215118 - BLE_Multi Master Single Slave
This example demonstrates how to configure the PSoC 6 MCU with Bluetooth Low Energy (BLE) Connectivity device in
simultaneous Multiple Master and Single Slave modes of operation.
#### 5. CE215119 - BLE Battery Level
This example demonstrates the operation of Bluetooth Low Energy (BLE) Battery Service (BAS) using the BLE_PDL
Component.
#### 6. CE215120 - BLE Device Information Service
This example project demonstrates how to configure and use the Bluetooth Low Energy (BLE) Component APIs and an
application layer callback.
#### 7. CE215121 - BLE HID Keyboard
This example demonstrates the implementation of the Bluetooth Low Energy (BLE) HID over GATT Profile where the device operates as a HID keyboard.
#### 8. CE215122 - BLE HID Mouse
This example demonstrates the implementation of the Bluetooth Low Energy (BLE) HID over GATT Profile where the device operates as a HID Mouse.
#### 9. CE215123 - BLE Navigation
The design demonstrates the core functionality of the Bluetooth Low Energy (BLE) Component configured as a BLE Location and Navigation Service (LNS) device
in the GATT Server role. The application uses a BLE Location and Navigation Profile to report location and navigation information to a Client. Also, the Location
and Navigation Pod application uses the Battery Service to notify the battery level and the Device Information Service to assert the Device Name and so on.
#### 10. CE215124 - BLE HTTP Proxy
This example demonstrates the HTTP Proxy Client and Server operation of the Bluetooth Low Energy (BLE) PSoC Creator Component.
#### 11. CE215555 - BLE Multi Role
This example demonstrates the capability of the PSoC 6 BLE device to be in all Generic Access Profile (GAP) roles (Central,
Peripheral, Observer, and Broadcaster) simultaneously.
#### 12. CE217631 - BLE Alert Notification
This example project demonstrates the Alert Notification Client operation of the BLE PSoC Creator Component. The Alert
Notification Client uses the BLE Alert Notification Profile with one instance of the Alert Notification Service to receive information
about Email, missed call, and SMS/MMS alerts from the Alert Notification Server. The device remains in Sleep mode between
the BLE connection intervals.
#### 13. CE217632 BLE Apple Notification Client
The design demonstrates the core functionality of the BLE Component configured as a BLE Apple Notification Service (ANCS) device in the
GATT Client role. The application uses the BLE Apple Notification Center Service in the GATT Client mode to communicate with a BLE Apple 
Notification Center Server (iPhone, iPod, and so on).
#### 14. CE217633 - BLE Blood Pressure Sensor
This example project demonstrates the BLE Blood Pressure Sensor application workflow. The Blood Pressure Sensor
application uses the BLE Blood Pressure profile to report blood pressure measurement records to a client. Also, the Blood
Pressure Sensor application uses the Battery Service to notify the Battery Level and the Device Information services to assert
the Device Name and so on.
#### 15. CE217634 - BLE Continuous Glucose Monitoring Sensor
This example project demonstrates the Bluetooth Low Energy (BLE) Continuous Glucose Monitoring Sensor application workflow.
#### 16. CE217635 BLE Cycling Sensor
This example demonstrates the Cycling Speed and Cadence Service (CSCS) and Cycling Power Service (CPS) with
PSoC 6 MCU with Bluetooth Low Energy (BLE) Connectivity.
#### 17. CE217636 - BLE Environmental Sensing
This example project demonstrates the Environmental Sensing Profile operation of the BLE PSoC Creator Component.
The Environmental Sensor uses the Environmental Sensing Profile with one instance of Environmental Sensing and 
Device Information Services to simulate measuring the wind speed. The Environmental Sensor operates with other 
devices that implement the Environmental Collector Profile. The device switches to Deep Sleep mode between BLE 
connection intervals. The BLE Component supports PSoC 6 BLE.
#### 18. CE217637 - BLE Find Me
This example project demonstrates the Find Me Profile operation of the BLE Component. The Find Me Target uses the Find Me
Profile with one instance of the Immediate Alert Service to display the alerts if the Client has configured the device for alerting.
The Find Me Target operates with other devices that implement the Find Me Locator Profile. The device switches to Deep Sleep
mode between BLE connection intervals.
#### 19. CE217638 - BLE Glucose Meter
This example project demonstrates the BLE Glucose Meter application workflow. The Glucose Meter application uses the BLE Glucose 
Profile to report glucose measurement records to a Client. Also, the Glucose Meter application uses the Battery Service to notify 
the Battery Level and the Device Information Services to assert the Device Name, etc.
#### 20. CE217639 - BLE Heart Rate
This example demonstrates the Heart Rate Client and Server operation of the Bluetooth Low Energy (BLE) PSoC Creator Component.
#### 21. CE217640 - BLE Phone Alert
This code example demonstrates the current Ringer mode of the Phone Alert Server (a phone or CY8CKIT-062 PSoC 6 BLE Pioneer Kit)
and the Ringer and Vibrate states on the user interface LEDs of the Phone Alert Client on the CY8CKIT-062 PSoC 6 BLE Pioneer Kit).
#### 22. CE217641 - BLE Proximity Profile
This example project demonstrates the Proximity operation of the BLE PSoC Creator Component. The Proximity Reporter
uses the BLE Proximity Profile with one instance of the Link Loss Service and one instance of the Tx Power Service to display
alerts on the device if connection to the client is lost. The Proximity Reporter operates with other devices which implement the
Proximity Monitor Profile role. The device uses Limited Discovery mode during which it is visible to the BLE clients. The device
remains in Deep Sleep mode between the BLE connection intervals.
#### 23. CE217642 - BLE Running Speed Cadence
The design demonstrates the core functionality of the BLE Component configured as the Running Speed and Cadence Sensor (RSCS)
device in the GATT Server role. The device simulates running/walking data measurements and sends them over the BLE Running Speed
and Cadence Service
#### 24. CE217643 - BLE Temperature Measurement 
This example demonstrates the Health Thermometer Profile operation of the BLE Component. The device simulates
thermometer readings and sends it over to the BLE Health Thermometer Service. It also simulates a battery level value and
sends it over to the BLE Battery Service.
#### 25. CE217644 - BLE Time Sync
This example demonstrates the Current Time Service (CTS) in GATT Client and GAP Peripheral role.
#### 26. CE217645 - BLE Weight Scale
The design demonstrates the Weight Scale Profile operation of the BLE Component. The Weight Scale Sensor uses one instance of the
Weight Scale Service (WSS), Body Composition Service (BCS), User Data Service (UDS), and Device Information Service (DIS) to simulate
weight measurements for up to four registered users. The Weight Scale Sensor operates with other devices that implement the Weight Scale 
Collector Profile.
#### 27. CE217646 - BLE Wireless Power Receiver Transmitter
This example demonstrates the Wireless Power Transfer service in the Power Transmitter Unit and Power Receiver Unit role.
#### 28. CE217647 - BLE Indoor Positioning
This Bluetooth Low Energy (BLE) example project demonstrates how to create an indoor navigation system using the BLE broadcasting
mode that can be configured over a GATT connection.
#### 29. CE218044 - BLE Pulse Oximeter Sensor
This example demonstrates how to configure and use the Bluetooth Low Energy (BLE) Component APIs and application layer
callbacks for the Pulse Oximeter Profile (PLXP).
#### 30. CE220272 - BLE Direct Test Mode
This code example demonstrates Direct Test Mode (DTM) over the Host Controller Interface (HCI) using PSoC 6 MCU with
Bluetooth Low Energy (BLE) Connectivity.
#### 31. CE220959 - Bootloader BLE External Memory
This example demonstrates over the air (OTA) bootloading with a PSoC® 6 MCU with Bluetooth Low Energy (BLE) connectivity using an 
external memory. The application is downloaded into the external memory, verified, and afterwards copied into the internal flash 
memory for execution.
#### 32. CE220960 - Bootloader BLE Upgradable Stack
This example demonstrates over the air (OTA) bootloading with a PSoC® 6 MCU with Bluetooth Low Energy (BLE) connectivity. 
The BLE stack code is shared between applications to reduce flash usage. The bootloader may download updates to the BLE stack or to the application.
#### 33. CE222046 - BLE Throughput Measurement
This code example demonstrates how to maximize the BLE throughput on PSoC® 6 MCU with Bluetooth Low Energy (BLE)
Connectivity device.
#### 34. CE223508 - PSoC6 BLE Four Slaves RTOS
This example demonstrates the implementation of multi-slave functionality of the PSoC 6 MCU with BLE Connectivity (PSoC 6
BLE) device.
#### 35. CE224714 - PSoC6 BLE Three Masters One Slave RTOS
This example demonstrates how to configure the PSoC 6 MCU with Bluetooth Low Energy Connectivity (PSoC 6 BLE) device
in simultaneous Multiple Master and Single Slave modes of operation.
#### 36. CE224856 - BLE LP Beacon Hibernate RTOS
This code example demonstrates a Bluetooth Low Energy (BLE) beacon that goes into Hibernate mode once the specified active time has elapsed.

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
Need support for your design and development questions? Check out the [Cypress Developer Community 3.0](https://community.cypress.com/welcome).  

Interact with technical experts in the embedded design community and receive answers verified by Cypress' very best applications engineers. You'll also have access to robust technical documentation, active conversation threads, and rich multimedia content. 

You can also use the following support resources if you need quick assistance:
##### Self-help: [Technical Support](http://www.cypress.com/support)
##### Local Sales office locations: [Sales Office](http://www.cypress.com/about-us/sales-offices)
