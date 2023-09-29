# Change Log

## Version 0.0.x

### 0.0.21.x (2023-Sep-29)

- Updated preprocessor macro to include ENABLE_DATA_LINK_LAYER_IPV4
- Updated Windows SDK version [Issues/35](https://github.com/chipkin/BACnetServerExampleCPP/issues/35)

### 0.0.20.x (2023-Sep-13)

- Calculate the number of states in a Multi-State by the number of elements in the stateText vector [Issues/31](https://github.com/chipkin/BACnetServerExampleCPP/issues/31)
- Updated device's application-software-version (12) property to reflect the application version [Issues/32](https://github.com/chipkin/BACnetServerExampleCPP/issues/32)
- Changed the RELINQUISH_DEFAULT of 1 for multiStateOutput instead of the CAS BACnet Stack's default of 0 [Issues/33](https://github.com/chipkin/BACnetServerExampleCPP/issues/33)

### 0.0.19.x (2022-Sep-02)

- Added example of a ProprietaryProperty that is an array of Real Primitives

### 0.0.18.x (2022-Aug-11)

- Updated CAS BACnet Stack to version 4.1.5

### 0.0.17.x (2022-Jun-13)

- Added example of Writing to a ProprietaryProperty
- Updated CAS BACnet Stack to version v3.30.9

### 0.0.16.x (2022-Mar-07)

- Added read-only DateTime proprietary property to analog input
- Updated ReinitializeDevice password check errorCode
- Updated CAS BACnet Stack to version v3.29.0.0

### 0.0.15.x (2022-Jan-28)

- Added DateTimeValue to example
- Updated CAS BACnet Stack to version v3.27.0.0

### 0.0.14.x (2022-Jan-20)

- Added LogDebugMessage callback to example
- Updated CAS BACnet Stack to version v3.26.0.0

### 0.0.13.x (2022-Jan-14)

- Added BroadcastDistributionTable to example

### 0.0.12.x (2021-Dec-08)

- Added CallbackDeviceCommunicationControl to example

### 0.0.11.x (2021-Jul-22)

- Fixed issue where broadcast address would be set incorrectly for 0 subnet addresses (eg. 192.168.0.x)

### 0.0.10.x (2021-Jul-15)

- Updated to CAS BACnet Stack Version 3.24.6
- Added support for confirmed and unconfirmed text message requests
- Added UnconfirmedTextMessageRequest broadcast of stack version on application startup

### 0.0.9.x (2020-Nov-05)

- Updated to CAS BACnet Stack Version 3.21.0
- Decode incoming packets as JSON instead of XML

### 0.0.8.x (2020-Nov-05)

- Updated to CAS BACnet Stack Version 3.20.0
- Added ability to set the device instance from the command line.

### 0.0.7.x (2020-Sep-24)

- Updated to CAS BACnet Stack Version 3.19.0.0

### 0.0.6.x (2020-Jul-3)

- Fixed CallbackGetPropertyEnum to retrieve system status from database for COV notifications

### 0.0.5.x (2020-May-21)

- Added ReinitializeDevice and Writable Network Port Object properties to example

### 0.0.4.x (2020-Apr-16)

- Added TrendLogMultiple to example

### 0.0.3.x (2020-Feb-12)

- Added Linux make files and linux CI job.

### 0.0.2.x (2020-Feb-11)

- Added CI build for Windows release

### 0.0.1.x (2019-Aug-22)

- Inital release.
