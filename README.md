# BACnet Server Example C++

A basic BACnet IP server example written in C++ using the [CAS BACnet Stack](https://store.chipkin.com/services/stacks/bacnet-stack). Includes various sample BACnet objects and services.

## Supported CAS BACnet Stack Version

This example project uses version 4.1.5 of the CAS BACnet Stack

## Releases

Build versions of this example can be downloaded from the releases page:

[https://github.com/chipkin/BACnetServerExampleCPP/releases](https://github.com/chipkin/BACnetServerExampleCPP/releases)

## Installation

Download the latest release zip file on the releases page.

## Usage

Run the executable included in the zip file.

Pre-configured with the following example BACnet device and objects:

- **Device**: 389999 (Device Rainbow)
  - analog_input: 0  (AnalogInput Bronze)
  - analog_output: 1  (AnalogOutput Chartreuse)
  - analog_value: 2  (AnalogValue Diamond)
  - binary_input: 3  (BinaryInput Emerald)
  - binary_output: 4  (BinaryOutput Fuchsia)
  - binary_value: 5  (BinaryValue Gold)
  - multi_state_input: 13  (MultiStateInput Hot Pink)
  - multi_state_output: 14  (MultiStateOutput Indigo)
  - multi_state_value: 19  (MultiStateValue Kiwi)
  - trend_log: 20  (TrendLog Lilac)
  - trend_log_multiple: 27  (TrendLogMultiple Magenta)
  - bitstring_value: 39  (BitstringValue Nickel)
  - characterstring_value: 40  (CharacterstringValue Onyx)
  - data_value: 42  (DateValue Purple)
  - integer_value: 45  (IntegerValue Quartz)
  - large_analog_value: 46  (LargeAnalogValue Red)
  - octetstring_value: 47  (OctetstringValue Silver)
  - positive_integer_value: 48  (PositiveIntegerValue Turquoise)
  - time_value: 50  (TimeValue Umber)
  - NetworkPort: 56  (NetworkPort Vermilion)
  - dateTimeValue: 60 (DateTimeValue White)

The following keyboard commands can be issued in the server window:

- **b**: Add (B)roadcast Distribution Table entry
- **i**: (i)ncrement Analog Value: 2 by 1.1
- **r**: Toggle the Analog Input: 0 (r)eliability status
- **f**: Send Register (foreign) device message
- **h**: (h)elp
- **m**: Send text (m)essage
- **q**: (q)uit

## Command arguments

The first argument is the device instance. If no arguments are defined then the default device instance.

## Build

A [Visual studio 2019](https://visualstudio.microsoft.com/downloads/) project is included with this project. This project is also auto built using [Gitlab CI](https://docs.gitlab.com/ee/ci/) on every commit.

The [CAS BACnet Stack submodule](https://github.com/chipkin/BACnetServerExampleCPP/issues/8) is required for compilation.

For the example server to run properly, please enable all object types and features of the CAS BACNet Stack. For more details, please reference `Enabling optional functionality` and `Compiling example projects` sections of the *Quick Start Guide* (please contact Chipkin for this document).

## Example Output

```txt
CAS BACnet Stack Server Example v0.0.15.0
https://github.com/chipkin/BACnetServerExampleCPP

FYI: Default to use device instance= 389999
FYI: Loading CAS BACnet Stack functions... OK

FYI: CAS BACnet Stack version: 3.27.0.0
FYI: Connecting UDP Resource to port=[47808]... OK, Connected to port
FYI: Registering the Callback Functions with the CAS BACnet Stack
Setting up server device. device.instance=[389999]
Created Device.
Enabling IAm... OK
Enabling ReadPropertyMultiple... OK
Enabling WriteProperty... OK
...
Adding AnalogInput. analogInput.instance=[0]... OK
...
Added TrendLogMultiple. trendLogMultiple.instance=[27]... OK
Added NetworkPort. networkPort.instance=[56]... OK
FYI: Sending I-AM broadcast
...
FYI: Entering main loop...
...
FYI: Received message from [10.9.2.159:47808], length [25]
...
```