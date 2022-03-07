/*
 * BACnet Server Example C++
 * ----------------------------------------------------------------------------
 * BACnetServerExample.cpp
 * 
 * In this CAS BACnet Stack example, we create a BACnet IP server with various
 * objects and properties from an example database.
 *
 * More information https://github.com/chipkin/BACnetServerExampleCPP
 * 
 * This file contains the 'main' function. Program execution begins and ends there.
 * 
 * Created by: Steven Smethurst
 */

#include "CASBACnetStackAdapter.h"
// !!!!!! This file is part of the CAS BACnet Stack. Please contact Chipkin for more information.
// !!!!!! https://github.com/chipkin/BACnetServerExampleCPP/issues/8

#include "CASBACnetStackExampleConstants.h"
#include "CASBACnetStackExampleDatabase.h"
#include "CIBuildSettings.h"

// Helpers 
#include "SimpleUDP.h"
#include "ChipkinEndianness.h"
#include "ChipkinConvert.h"
#include "ChipkinUtilities.h"

#include <iostream>
#ifndef __GNUC__ // Windows
	#include <conio.h> // _kbhit
#else // Linux 
	#include <sys/ioctl.h>
	#include <termios.h>
	bool _kbhit() {
		termios term;
		tcgetattr(0, &term);
		termios term2 = term;
		term2.c_lflag &= ~ICANON;
		tcsetattr(0, TCSANOW, &term2);
		int byteswaiting;
		ioctl(0, FIONREAD, &byteswaiting);
		tcsetattr(0, TCSANOW, &term);
		return byteswaiting > 0;
	}
	#include <termios.h>
	#include <unistd.h>
	#include <fcntl.h>
	void Sleep(int milliseconds) {
		usleep(milliseconds * 1000);
	}

#endif // __GNUC__

// Globals
// =======================================
CSimpleUDP g_udp; // UDP resource
ExampleDatabase g_database; // The example database that stores current values.

// Constants
// =======================================
const std::string APPLICATION_VERSION = "0.0.16";  // See CHANGELOG.md for a full list of changes.
const uint32_t MAX_RENDER_BUFFER_LENGTH = 1024 * 20;


// Callback Functions to Register to the DLL
// Message Functions
uint16_t CallbackReceiveMessage(uint8_t* message, const uint16_t maxMessageLength, uint8_t* receivedConnectionString, const uint8_t maxConnectionStringLength, uint8_t* receivedConnectionStringLength, uint8_t* networkType);
uint16_t CallbackSendMessage(const uint8_t* message, const uint16_t messageLength, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, bool broadcast);

// System Functions
time_t CallbackGetSystemTime();
bool CallbackSetSystemTime(const uint32_t deviceInstance, const uint8_t year, const uint8_t month, const uint8_t day, const uint8_t weekday, const uint8_t hour, const uint8_t minute, const uint8_t second, const uint8_t hundrethSeconds);

// Get Property Functions
bool CallbackGetPropertyBitString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, bool* value, uint32_t* valueElementCount, const uint32_t maxElementCount, const bool useArrayIndex, const uint32_t propertyArrayIndex);
bool CallbackGetPropertyBool(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, bool* value, const bool useArrayIndex, const uint32_t propertyArrayIndex);
bool CallbackGetPropertyCharString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, char* value, uint32_t* valueElementCount, const uint32_t maxElementCount, uint8_t* encodingType, const bool useArrayIndex, const uint32_t propertyArrayIndex);
bool CallbackGetPropertyDate(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, uint8_t* year, uint8_t* month, uint8_t* day, uint8_t* weekday, const bool useArrayIndex, const uint32_t propertyArrayIndex);
bool CallbackGetPropertyDouble(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, double* value, const bool useArrayIndex, const uint32_t propertyArrayIndex);
bool CallbackGetPropertyEnum(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, uint32_t* value, const bool useArrayIndex, const uint32_t propertyArrayIndex);
bool CallbackGetPropertyOctetString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, uint8_t* value, uint32_t* valueElementCount, const uint32_t maxElementCount, const bool useArrayIndex, const uint32_t propertyArrayIndex);
bool CallbackGetPropertyInt(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, int32_t* value, const bool useArrayIndex, const uint32_t propertyArrayIndex);
bool CallbackGetPropertyReal(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, float* value, const bool useArrayIndex, const uint32_t propertyArrayIndex);
bool CallbackGetPropertyTime(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, uint8_t* hour, uint8_t* minute, uint8_t* second, uint8_t* hundrethSeconds, const bool useArrayIndex, const uint32_t propertyArrayIndex);
bool CallbackGetPropertyUInt(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, uint32_t* value, const bool useArrayIndex, const uint32_t propertyArrayIndex);

// Set Property Functions
bool CallbackSetPropertyBitString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool* value, const uint32_t length, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode);
bool CallbackSetPropertyBool(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode);
bool CallbackSetPropertyCharString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const char* value, const uint32_t length, const uint8_t encodingType, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode);
bool CallbackSetPropertyDate(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint8_t year, const uint8_t month, const uint8_t day, const uint8_t weekday, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode);
bool CallbackSetPropertyDouble(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const double value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode);
bool CallbackSetPropertyEnum(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint32_t value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode);
bool CallbackSetPropertyNull(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode);
bool CallbackSetPropertyOctetString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint8_t* value, const uint32_t length, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode);
bool CallbackSetPropertyInt(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const int32_t value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode);
bool CallbackSetPropertyReal(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const float value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode);
bool CallbackSetPropertyTime(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint8_t hour, const uint8_t minute, const uint8_t second, const uint8_t hundrethSeconds, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode);
bool CallbackSetPropertyUInt(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint32_t value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode);
bool CallbackCreateObject(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance);
bool CallbackDeleteObject(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance);

bool CallbackReinitializeDevice(const uint32_t deviceInstance, const uint32_t reinitializedState, const char* password, const uint32_t passwordLength, uint32_t* errorCode);
bool CallbackDeviceCommunicationControl(const uint32_t deviceInstance, const uint8_t enableDisable, const char* password, const uint8_t passwordLength, const bool useTimeDuration, const uint16_t timeDuration, uint32_t* errorCode);
bool HookTextMessage(const uint32_t sourceDeviceIdentifier, const bool useMessageClass, const uint32_t messageClassUnsigned, const char* messageClassString, const uint32_t messageClassStringLength, const uint8_t messagePriority, const char* message, const uint32_t messageLength, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t sourceNetwork, const uint8_t* sourceAddress, const uint8_t sourceAddressLength, uint16_t* errorClass, uint16_t* errorCode);

// Helper functions 
bool DoUserInput();
bool GetObjectName(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, char* value, uint32_t* valueElementCount, const uint32_t maxElementCount);

// Debug Message Function
void CallbackLogDebugMessage(const char* message, const uint16_t messageLength, const uint8_t messageType);

int main(int argc, char** argv)
{
	// Print the application version information 
	std::cout << "CAS BACnet Stack Server Example v" << APPLICATION_VERSION << "." << CIBUILDNUMBER << std::endl; 
	std::cout << "https://github.com/chipkin/BACnetServerExampleCPP" << std::endl << std::endl;

	// Check to see if they defined the device.instance via the command arguments.
	if (argc >= 2) {		
		g_database.device.instance = atoi(argv[1]); 
		std::cout << "FYI: Device instance= " << g_database.device.instance << std::endl;
	}
	else {
		std::cout << "FYI: Default to use device instance= " << g_database.device.instance << std::endl;
	}


	// 1. Load the CAS BACnet stack functions
	// ---------------------------------------------------------------------------
	std::cout << "FYI: Loading CAS BACnet Stack functions... "; 
	if (!LoadBACnetFunctions()) {
		std::cerr << "Failed to load the functions from the DLL" << std::endl;
		return 0;
	}
	std::cout << "OK" << std::endl;
	std::cout << "FYI: CAS BACnet Stack version: " << fpGetAPIMajorVersion() << "." << fpGetAPIMinorVersion() << "." << fpGetAPIPatchVersion() << "." << fpGetAPIBuildVersion() << std::endl;

	// 2. Connect the UDP resource to the BACnet Port
	// ---------------------------------------------------------------------------
	std::cout << "FYI: Connecting UDP Resource to port=["<< g_database.networkPort.BACnetIPUDPPort << "]... ";
	if (!g_udp.Connect(g_database.networkPort.BACnetIPUDPPort)) {
		std::cerr << "Failed to connect to UDP Resource" << std::endl ;
		std::cerr << "Press any key to exit the application..." << std::endl;
		(void) getchar();
		return -1;
	}
	std::cout << "OK, Connected to port" << std::endl;


	// 3. Setup the callbacks
	// ---------------------------------------------------------------------------
	std::cout << "FYI: Registering the Callback Functions with the CAS BACnet Stack" << std::endl;

	// Message Callback Functions
	fpRegisterCallbackReceiveMessage(CallbackReceiveMessage);
	fpRegisterCallbackSendMessage(CallbackSendMessage);

	// System Time Callback Functions
	fpRegisterCallbackGetSystemTime(CallbackGetSystemTime);
	fpRegisterCallbackSetSystemTime(CallbackSetSystemTime);

	// Get Property Callback Functions
	fpRegisterCallbackGetPropertyBitString(CallbackGetPropertyBitString);
	fpRegisterCallbackGetPropertyBool(CallbackGetPropertyBool);
	fpRegisterCallbackGetPropertyCharacterString(CallbackGetPropertyCharString);
	fpRegisterCallbackGetPropertyDate(CallbackGetPropertyDate);
	fpRegisterCallbackGetPropertyDouble(CallbackGetPropertyDouble);
	fpRegisterCallbackGetPropertyEnumerated(CallbackGetPropertyEnum);
	fpRegisterCallbackGetPropertyOctetString(CallbackGetPropertyOctetString);
	fpRegisterCallbackGetPropertySignedInteger(CallbackGetPropertyInt);
	fpRegisterCallbackGetPropertyReal(CallbackGetPropertyReal);
	fpRegisterCallbackGetPropertyTime(CallbackGetPropertyTime);
	fpRegisterCallbackGetPropertyUnsignedInteger(CallbackGetPropertyUInt);

	// Set Property Callback Functions
	fpRegisterCallbackSetPropertyBitString(CallbackSetPropertyBitString);
	fpRegisterCallbackSetPropertyBool(CallbackSetPropertyBool);
	fpRegisterCallbackSetPropertyCharacterString(CallbackSetPropertyCharString);
	fpRegisterCallbackSetPropertyDate(CallbackSetPropertyDate);
	fpRegisterCallbackSetPropertyDouble(CallbackSetPropertyDouble);
	fpRegisterCallbackSetPropertyEnumerated(CallbackSetPropertyEnum);
	fpRegisterCallbackSetPropertyNull(CallbackSetPropertyNull);
	fpRegisterCallbackSetPropertyOctetString(CallbackSetPropertyOctetString);
	fpRegisterCallbackSetPropertySignedInteger(CallbackSetPropertyInt);
	fpRegisterCallbackSetPropertyReal(CallbackSetPropertyReal);
	fpRegisterCallbackSetPropertyTime(CallbackSetPropertyTime);
	fpRegisterCallbackSetPropertyUnsignedInteger(CallbackSetPropertyUInt);

	// Object creation
	fpRegisterCallbackCreateObject(CallbackCreateObject);
	fpRegisterCallbackDeleteObject(CallbackDeleteObject);

	// Remote Device Management
	fpRegisterCallbackReinitializeDevice(CallbackReinitializeDevice);
	fpRegisterCallbackDeviceCommunicationControl(CallbackDeviceCommunicationControl);
	fpRegisterHookTextMessage(HookTextMessage);

	// Get Debug Message Function
	fpRegisterCallbackLogDebugMessage(CallbackLogDebugMessage);
	

	// 4. Setup the BACnet device
	// ---------------------------------------------------------------------------

	std::cout << "Setting up server device. device.instance=[" << g_database.device.instance << "]" << std::endl; 

	// Create the Device
	if (!fpAddDevice(g_database.device.instance)) {
		std::cerr << "Failed to add Device." << std::endl;
		return false;
	}
	std::cout << "Created Device." << std::endl; 


	// Enable the services that this device supports
	// Some services are mandatory for BACnet devices and are already enabled.
	// These are: Read Property, Who Is, Who Has
	//
	// Any other services need to be enabled as below.

	std::cout << "Enabling IAm... ";
	if (!fpSetServiceEnabled(g_database.device.instance, CASBACnetStackExampleConstants::SERVICE_I_AM, true)) {
		std::cerr << "Failed to enabled the IAm" << std::endl;
		return -1;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling ReadPropertyMultiple... "; 
	if (!fpSetServiceEnabled(g_database.device.instance, CASBACnetStackExampleConstants::SERVICE_READ_PROPERTY_MULTIPLE, true)) {
		std::cerr << "Failed to enabled the ReadPropertyMultiple" << std::endl;
		return -1;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling WriteProperty... ";
	if (!fpSetServiceEnabled(g_database.device.instance, CASBACnetStackExampleConstants::SERVICE_WRITE_PROPERTY, true)) {
		std::cerr << "Failed to enable the WriteProperty service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling WritePropertyMultiple... "; 
	if (!fpSetServiceEnabled(g_database.device.instance, CASBACnetStackExampleConstants::SERVICE_WRITE_PROPERTY_MULTIPLE, true)) {
		std::cerr << "Failed to enable the WritePropertyMultiple service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;	

	std::cout << "Enabling TimeSynchronization... ";
	if (!fpSetServiceEnabled(g_database.device.instance, CASBACnetStackExampleConstants::SERVICE_TIME_SYNCHRONIZATION, true)) {
		std::cerr << "Failed to enable the TimeSynchronization service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;	

	std::cout << "Enabling UTCTimeSynchronization... ";
	if (!fpSetServiceEnabled(g_database.device.instance, CASBACnetStackExampleConstants::SERVICE_UTC_TIME_SYNCHRONIZATION, true)) {
		std::cerr << "Failed to enable the UTCTimeSynchronization service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;
	
	std::cout << "Enabling SubscribeCOV... ";
	if (!fpSetServiceEnabled(g_database.device.instance, CASBACnetStackExampleConstants::SERVICE_SUBSCRIBE_COV, true)) {
		std::cerr << "Failed to enable the SubscribeCOV service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling SubscribeCOVProperty... ";
	if (!fpSetServiceEnabled(g_database.device.instance, CASBACnetStackExampleConstants::SERVICE_SUBSCRIBE_COV_PROPERTY, true)) {
		std::cerr << "Failed to enable the SubscribeCOVProperty service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;
	
	std::cout << "Enabling CreateObject... ";
	if (!fpSetServiceEnabled(g_database.device.instance, CASBACnetStackExampleConstants::SERVICE_CREATE_OBJECT, true)) {
		std::cerr << "Failed to enable the CreateObject service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;
	
	std::cout << "Enabling DeleteObject... ";
	if (!fpSetServiceEnabled(g_database.device.instance, CASBACnetStackExampleConstants::SERVICE_DELETE_OBJECT, true)) {
		std::cerr << "Failed to enable the DeleteObject service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;
	
	std::cout << "Enabling ReadRange... ";
	if (!fpSetServiceEnabled(g_database.device.instance, CASBACnetStackExampleConstants::SERVICE_READ_RANGE, true)) {
		std::cerr << "Failed to enable the ReadRange service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling ReinitializeDevice...";
	if (!fpSetServiceEnabled(g_database.device.instance, CASBACnetStackExampleConstants::SERVICE_REINITIALIZE_DEVICE, true)) {
		std::cerr << "Failed to enable the ReinitializeDevice service";
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling DeviceCommunicationControl...";
	if (!fpSetServiceEnabled(g_database.device.instance, CASBACnetStackExampleConstants::SERVICE_DEVICE_COMMUNICATION_CONTROL, true)) {
		std::cerr << "Failed to enable the DeviceCommunicationControl service";
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling UnconfirmedTextMessage...";
	if (!fpSetServiceEnabled(g_database.device.instance, CASBACnetStackExampleConstants::SERVICE_UNCONFIRMED_TEXT_MESSAGE, true)) {
		std::cerr << "Failed to enable the UnconfirmedTextMessage service";
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling ConfirmedTextMessage...";
	if (!fpSetServiceEnabled(g_database.device.instance, CASBACnetStackExampleConstants::SERVICE_CONFIRMED_TEXT_MESSAGE, true)) {
		std::cerr << "Failed to enable the ConfirmedTextMessage service";
		return false;
	}
	std::cout << "OK" << std::endl;


	// Enable Optional Device Properties
	if (!fpSetPropertyEnabled(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE, g_database.device.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_DESCRIPTION, true)) {
		std::cerr << "Failed to enable the description property for Device" << std::endl;
		return false;
	}

	// Update Writable Device Properties
	// UTC Offset
	if (!fpSetPropertyWritable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE, g_database.device.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_UTC_OFFSET, true)) {
		std::cerr << "Failed to make the UTC Offset property writable for Device" << std::endl ; 
		return false;
	}
	// Description
	if (!fpSetPropertyWritable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE, g_database.device.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_DESCRIPTION, true)) {
		std::cerr << "Failed to make the description property writable for Device" << std::endl ; 
		return false;
	}
	// Object Name
	if (!fpSetPropertyWritable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE, g_database.device.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, true)) {
		std::cerr << "Failed to make the object name property writable for Device" << std::endl ; 
		return false;
	}

	// Make some object creatable (optional)
	if (!fpSetObjectTypeSupported(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, true)) {
		std::cerr << "Failed to make Analog Values as supported object types in Device" << std::endl ; 
		return false;
	}
	if (!fpSetObjectTypeCreatable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, true)) {
		std::cerr << "Failed to make Analog Values as creatable object types in Device" << std::endl ; 
		return false;
	}


	// Add Objects
	// ---------------------------------------
	// AnalogInput (AO) 
	std::cout << "Adding AnalogInput. analogInput.instance=[" << g_database.analogInput.instance << "]... ";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_database.analogInput.instance)) {
		std::cerr << "Failed to add AnalogInput" << std::endl;
		return -1;
	}
	// Enable ProprietaryProperty for an object 
	// These properties are not part of the BACnet spec 
	fpSetProprietaryProperty(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_database.analogInput.instance, 512 + 1, false, false, CASBACnetStackExampleConstants::DATA_TYPE_CHARACTER_STRING, false, false, false);
	fpSetProprietaryProperty(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_database.analogInput.instance, 512 + 2, true, false, CASBACnetStackExampleConstants::DATA_TYPE_CHARACTER_STRING, false, false, false);
	fpSetProprietaryProperty(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_database.analogInput.instance, 512 + 3, true, true, CASBACnetStackExampleConstants::DATA_TYPE_CHARACTER_STRING, false, false, false);
	fpSetProprietaryProperty(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_database.analogInput.instance, 512 + 4, false, false, CASBACnetStackExampleConstants::DATA_TYPE_DATETIME, false, false, false);

	// Set the Present value to subscribable 
	fpSetPropertySubscribable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_database.analogInput.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	fpSetPropertyWritable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_database.analogInput.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_COV_INCURMENT, true);

	// Enable the description, and Reliability property 
	fpSetPropertyByObjectTypeEnabled(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_DESCRIPTION, true);
	fpSetPropertyByObjectTypeEnabled(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_RELIABILITY, true);

	// Enable a specific property to be subscribable for COVProperty 
	fpSetPropertySubscribable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_database.analogInput.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_RELIABILITY, true);
	std::cout << "OK" << std::endl;

	// AnalogOutput (AO) 
	std::cout << "Added AnalogOutput. analogOutput.instance=[" << g_database.analogOutput.instance << "]... ";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT, g_database.analogOutput.instance)) {
		std::cerr << "Failed to add AnalogOutput" << std::endl;
		return -1;
	}
	fpSetPropertyByObjectTypeEnabled(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_MIN_PRES_VALUE, true);
	fpSetPropertyByObjectTypeEnabled(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_MAX_PRES_VALUE, true);
	std::cout << "OK" << std::endl;

	// AnalogValue (AV) 
	std::cout << "Added AnalogValue. analogValue.instance=[" << g_database.analogValue.instance << "]... ";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, g_database.analogValue.instance)) {
		std::cerr << "Failed to add AnalogValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, g_database.analogValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	fpSetPropertySubscribable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, g_database.analogValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// BinaryInput (BI)
	std::cout << "Adding BinaryInput. binaryInput.instance=[" << g_database.binaryInput.instance << "]... ";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_INPUT, g_database.binaryInput.instance)) {
		std::cerr << "Failed to add BinaryInput" << std::endl;
		return -1;
	}
	std::cout << "OK" << std::endl;

	// BinaryOutput (BO)
	std::cout << "Added BinaryOutput. binaryOutput.instance=[" << g_database.binaryOutput.instance << "]... ";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_OUTPUT, g_database.binaryOutput.instance)) {
		std::cerr << "Failed to add BinaryOutput" << std::endl;
		return -1;
	}
	std::cout << "OK" << std::endl;

	// BinaryValue (BV)
	std::cout << "Added BinaryValue. binaryValue.instance=[" << g_database.binaryValue.instance << "]... ";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_VALUE, g_database.binaryValue.instance)) {
		std::cerr << "Failed to add BinaryValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_VALUE, g_database.binaryValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// MultiStateInput (MSI) 
	std::cout << "Added MultiStateInput. multiStateInput.instance=[" << g_database.multiStateInput.instance << "]... ";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT, g_database.multiStateInput.instance)) {
		std::cerr << "Failed to add MultiStateInput" << std::endl;
		return -1;
	}
	fpSetPropertyByObjectTypeEnabled(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_STATE_TEXT, true);
	std::cout << "OK" << std::endl;

	// MultiStateOutput (MSO)
	std::cout << "Added MultiStateOutput. multiStateOutput.instance=[" << g_database.multiStateOutput.instance << "]... ";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT, g_database.multiStateOutput.instance)) {
		std::cerr << "Failed to add MultiStateOutput" << std::endl;
		return -1;
	}
	std::cout << "OK" << std::endl;

	// MultiStateValue (MSV)
	std::cout << "Added MultiStateValue. multiStateValue.instance=[" << g_database.multiStateValue.instance << "]... ";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE, g_database.multiStateValue.instance)) {
		std::cerr << "Failed to add MultiStateValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE, g_database.multiStateValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// BitstringValue (BSV)
	std::cout << "Added BitstringValue. bitstringValue.instance=[" << g_database.bitstringValue.instance << "]...";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE, g_database.bitstringValue.instance)) {
		std::cerr << "Failed to add BitstringValue" << std::endl;
		return -1;
	}
	fpSetPropertyEnabled(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE, g_database.bitstringValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_BIT_TEXT, true);
	fpSetPropertyWritable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE, g_database.bitstringValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// characterStringValue (CSV)
	std::cout << "Added characterStringValue. characterStringValue.instance=[" << g_database.characterStringValue.instance << "]...";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_CHARACTERSTRING_VALUE, g_database.characterStringValue.instance)) {
		std::cerr << "Failed to add characterStringValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_CHARACTERSTRING_VALUE, g_database.characterStringValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// DateValue (DV)
	std::cout << "Added DateValue. dateValue.instance=[" << g_database.dateValue.instance << "]... ";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_DATE_VALUE, g_database.dateValue.instance)) {
		std::cerr << "Failed to add DateValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_DATE_VALUE, g_database.dateValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// IntegerValue (IV)
	std::cout << "Added IntegerValue. integerValue.instance=[" << g_database.integerValue.instance << "]... ";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_INTEGER_VALUE, g_database.integerValue.instance)) {
		std::cerr << "Failed to add IntegerValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_INTEGER_VALUE, g_database.integerValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// LargeAnalogValue (LAV)
	std::cout << "Added LargeAnalogValue. largeAnalogValue.instance=[" << g_database.largeAnalogValue.instance << "]... ";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_LARGE_ANALOG_VALUE, g_database.largeAnalogValue.instance)) {
		std::cerr << "Failed to add LargeAnalogValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_LARGE_ANALOG_VALUE, g_database.largeAnalogValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// octetStringValue (OSV)
	std::cout << "Added octetStringValue. octetStringValue.instance=[" << g_database.octetStringValue.instance << "]... ";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_OCTETSTRING_VALUE, g_database.octetStringValue.instance)) {
		std::cerr << "Failed to add octetStringValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_OCTETSTRING_VALUE, g_database.octetStringValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// PositiveIntegerValue (PIV)
	std::cout << "Added PositiveIntegerValue. positiveIntegerValue.instance=[" << g_database.positiveIntegerValue.instance << "]... ";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_POSITIVE_INTEGER_VALUE, g_database.positiveIntegerValue.instance)) {
		std::cerr << "Failed to add PositiveIntegerValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_POSITIVE_INTEGER_VALUE, g_database.positiveIntegerValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// TimeValue (TV)
	std::cout << "Added TimeValue. timeValue.instance=[" << g_database.timeValue.instance << "]... ";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_TIME_VALUE, g_database.timeValue.instance)) {
		std::cerr << "Failed to add TimeValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_TIME_VALUE, g_database.timeValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// Add Trend Log Object
	std::cout << "Added TrendLog. trendLog.instance=[" << g_database.trendLog.instance << "]... ";
	if (!fpAddTrendLogObject(g_database.device.instance, g_database.trendLog.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_database.analogInput.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, CASBACnetStackExampleConstants::MAX_TREND_LOG_MAX_BUFFER_SIZE, false, 0)) {
		std::cerr << "Failed to add TrendLog" << std::endl;
		return -1;
	}

	// Setup TrendLog Object
	if (!fpSetTrendLogTypeToPolled(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_TREND_LOG, g_database.trendLog.instance, true, false, 3000)) {
		std::cerr << "Failed to setup TrendLog to poll every 30 seconds";
		return -1;
	}
	std::cout << "OK" << std::endl;

	// Add Trend Log Multiple Object
	std::cout << "Added TrendLogMultiple. trendLogMultiple.instance=[" << g_database.trendLogMultiple.instance << "]... ";
	if (!fpAddTrendLogMultipleObject(g_database.device.instance, g_database.trendLogMultiple.instance, CASBACnetStackExampleConstants::MAX_TREND_LOG_MAX_BUFFER_SIZE)) {
		std::cerr << "Failed to add TrendLogMultiple" << std::endl;
		return -1;
	}

	// Setup TrendLogMultiple Object
	if (!fpAddLoggedObjectToTrendLogMultiple(g_database.device.instance, g_database.trendLogMultiple.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_database.analogInput.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0, false, 0)) {
		std::cerr << "Failed to add AnalogInput to be logged by TrendLogMultiple" << std::endl;
		return -1;
	}
	if (!fpAddLoggedObjectToTrendLogMultiple(g_database.device.instance, g_database.trendLogMultiple.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_INPUT, g_database.binaryInput.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0, false, 0)) {
		std::cerr << "Failed to add BinaryInput to be logged by TrendLogMultiple" << std::endl;
		return -1;
	}
	if (!fpSetTrendLogTypeToPolled(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_TREND_LOG_MULTIPLE, g_database.trendLogMultiple.instance, true, false, 3000)) {
		std::cerr << "Failed to setup TrendLogMultiple to poll every 30 seconds";
		return -1;
	}
	std::cout << "OK" << std::endl;

	// Add the Network Port Object
	std::cout << "Added NetworkPort. networkPort.instance=[" << g_database.networkPort.instance << "]... ";
	if (!fpAddNetworkPortObject(g_database.device.instance, g_database.networkPort.instance, CASBACnetStackExampleConstants::NETWORK_TYPE_IPV4, CASBACnetStackExampleConstants::PROTOCOL_LEVEL_BACNET_APPLICATION, CASBACnetStackExampleConstants::NETWORK_PORT_LOWEST_PROTOCOL_LAYER)) {
		std::cerr << "Failed to add NetworkPort" << std::endl;
		return -1;
	}
	fpSetPropertyEnabled(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT, g_database.networkPort.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_BBMD_ACCEPT_FD_REGISTRATIONS, true);
	fpSetPropertyEnabled(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT, g_database.networkPort.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_BBMD_BROADCAST_DISTRIBUTION_TABLE, true);
	fpSetPropertyEnabled(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT, g_database.networkPort.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_BBMD_FOREIGN_DEVICE_TABLE, true);

	uint8_t ipPortConcat[6];
	memcpy(ipPortConcat, g_database.networkPort.IPAddress, 4);
	ipPortConcat[4] = g_database.networkPort.BACnetIPUDPPort / 256;
	ipPortConcat[5] = g_database.networkPort.BACnetIPUDPPort % 256;
	fpAddBDTEntry(ipPortConcat, 6, g_database.networkPort.IPSubnetMask, 4);		// First BDT Entry must be server device


	std::cout << "OK" << std::endl;

	// Add the DateTimeValue Object
	std::cout << "Added DateTimeValue. dateTimeValue.instance=[" << g_database.dateTimeValue.instance << "]... ";
	if (!fpAddObject(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_DATETIME_VALUE, g_database.dateTimeValue.instance)) {
		std::cerr << "Failed to add DateTimeValue" << std::endl;
		return -1;
	}
	
	std::cout << "OK" << std::endl;

	// 5. Send I-Am of this device
	// ---------------------------------------------------------------------------
	// To be a good citizen on a BACnet network. We should announce ourself when we start up. 
	std::cout << "FYI: Sending I-AM broadcast" << std::endl;
	uint8_t connectionString[6]; //= { 0xC0, 0xA8, 0x01, 0xFF, 0xBA, 0xC0 };
	memcpy(connectionString, g_database.networkPort.BroadcastIPAddress, 4);
	connectionString[4] = g_database.networkPort.BACnetIPUDPPort / 256;
	connectionString[5] = g_database.networkPort.BACnetIPUDPPort % 256;

	if (!fpSendIAm(g_database.device.instance, connectionString, 6, CASBACnetStackExampleConstants::NETWORK_TYPE_IP, true, 65535, NULL, 0)) {
		std::cerr << "Unable to send IAm broadcast" << std::endl ; 
		return false;
	}

	// Broadcast BACnet stack version to the network via UnconfirmedTextMessage
	char stackVersionInfo[50];
	sprintf(stackVersionInfo, "CAS BACnet Stack v%u.%u.%u.%u", fpGetAPIMajorVersion(), fpGetAPIMinorVersion(), fpGetAPIPatchVersion(), fpGetAPIBuildVersion());
	if (!fpSendUnconfirmedTextMessage(g_database.device.instance, false, 0, NULL, 0, 0, stackVersionInfo, strlen(stackVersionInfo), connectionString, 6, CASBACnetStackExampleConstants::NETWORK_TYPE_IP, true, 65535, NULL, 0)) {
		std::cerr << "Unable to send UnconfirmedTextMessage broadcast" << std::endl;
		return false;
	}

	// 6. Start the main loop
	// ---------------------------------------------------------------------------
	std::cout << "FYI: Entering main loop..." << std::endl ;
	for (;;) {
		// Call the DLLs loop function which checks for messages and processes them.
		fpLoop();

		// Handle any user input.
		// Note: User input in this example is used for the following:
		//		i - increment the analog-input value. Used to test cov
		//		h - Display options
		//		q - Quit
		if (!DoUserInput()) {
			// User press 'q' to quit the example application.
			break;
		}

		// Update values in the example database
		g_database.Loop();

		// Call Sleep to give some time back to the system
		Sleep(0); // Windows 
	}

	// All done. 
	return 0;
}

// Helper Functions

// Handle any user input.
// Note: User input in this example is used for the following:
//		i - increment the analog-input value. Used to test COV
//		r - Toggle Analog Input Reliability
//		f - Send Register Foreign Device message
//		h - Display options
//		q - Quit
bool DoUserInput()
{
	// Check to see if the user hit any key
	if (!_kbhit()) {
		// No keys have been hit
		return true;
	}

	// Extract the letter that the user hit and convert it to lower case
	char action = tolower(getchar());

	// Handle the action 
	switch (action) {
		// Quit
	case 'q': {
		return false;
	}
	case 'b': {
		// Add BDT Entry
		// Ask for BBMD Address, Port, and Mask
		std::string bbmdIpStr, bbmdPortStr, bbmdIpMaskStr;
		std::cout << "\nEnter BBMD IP Address (Format: WWW.XXX.YYY.ZZZ) (Enter empty string to use default value [192.168.1.208]):";
		std::cin >> bbmdIpStr;
		std::cout << "Enter BBMD IP Port (Enter [N] to use default value [47808])";
		std::cin >> bbmdPortStr;
		std::cout << "Enter BBMD IP Mask (Format: WWW.XXX.YYY.ZZZ) (Enter [N] to use default value [255.255.255.0]):";
		std::cin >> bbmdIpMaskStr;
		uint8_t bbmdIpAddress[6] = {192, 168, 1, 208, 0xBA, 0xC0};
		uint8_t bbmdIpMask[4] = {255, 255, 255, 0};
		uint8_t periodIndex;
		if (bbmdIpStr != "N" && bbmdIpStr != "n") {
			for (uint8_t i = 0; i < 3; i++) {
				periodIndex = bbmdIpStr.find(".");
				bbmdIpAddress[i] = std::atoi(bbmdIpStr.substr(0, periodIndex).c_str());
				bbmdIpStr = bbmdIpStr.substr(periodIndex + 1);
			}
			bbmdIpAddress[3] = std::atoi(bbmdIpStr.c_str());
		}
		if (bbmdPortStr != "N" && bbmdPortStr != "n") {
			bbmdIpAddress[4] = std::atoi(bbmdPortStr.c_str()) / 256;
			bbmdIpAddress[5] = std::atoi(bbmdPortStr.c_str()) % 256;
		}
		if (bbmdIpMaskStr != "N" && bbmdIpMaskStr != "n") {
			for (uint8_t i = 0; i < 3; i++) {
				periodIndex = bbmdIpMaskStr.find(".");
				bbmdIpMask[i] = std::atoi(bbmdIpMaskStr.substr(0, periodIndex).c_str());
				bbmdIpMaskStr = bbmdIpMaskStr.substr(periodIndex + 1);
			}
			bbmdIpMask[3] = std::atoi(bbmdIpMaskStr.c_str());
		}
			
		fpAddBDTEntry(bbmdIpAddress, 6, bbmdIpMask, 4);
		fpSetBBMD(g_database.device.instance, g_database.networkPort.instance);
		break;
	}
	case 'i': {
		// Increment the Analog Value
		g_database.analogValue.presentValue += 1.1f;
		std::cout << "Incrementing Analog Value to " << g_database.analogValue.presentValue << std::endl;

		// Notify the stack that this data point was updated so the stack can check for logic
		// that may need to run on the data.  Example: check if COV (change of value) occurred.
		if (fpValueUpdated != NULL) {
			fpValueUpdated(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, g_database.analogValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE);
		}
		break;
	}
	case 'r': {
		// Toggle the Analog Input reliability status
		// no-fault-detected (0), unreliable-other (7)
		if (g_database.analogInput.reliability == 0) {
			g_database.analogInput.reliability = 7; // unreliable-other (7)
		}
		else {
			g_database.analogInput.reliability = 0; //no-fault-detected (0)
		}
		std::cout << "Toggle the Analog Input reliability status to " << g_database.analogInput.reliability << std::endl;

		// Notify the stack that this data point was updated so the stack can check for logic
		// that may need to run on the data. Example: Check if COVProperty (change of value) occurred.
		if (fpValueUpdated != NULL) {
			fpValueUpdated(g_database.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_database.analogInput.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_RELIABILITY);
		}
		break;
	}
	case 'f': {
		// Send Foreign Device Registration
		
		uint8_t connectionString[6];
		memcpy(connectionString, g_database.networkPort.FdBbmdAddressHostIp, 4);
		connectionString[4] = g_database.networkPort.FdBbmdAddressPort / 256;
		connectionString[5] = g_database.networkPort.FdBbmdAddressPort % 256;
		std::cout << "Sending Register Foreign Device to" << g_database.networkPort.FdBbmdAddressHostIp[0] << "." << 
			g_database.networkPort.FdBbmdAddressHostIp[1] << "." << g_database.networkPort.FdBbmdAddressHostIp[2] << "." <<
			g_database.networkPort.FdBbmdAddressHostIp[3] << ":" << g_database.networkPort.FdBbmdAddressPort << std::endl;

		if (!fpSendRegisterForeignDevice(g_database.networkPort.FdSubscriptionLifetime, connectionString, 6)) {
			std::cout << "Error - failed to send Register Foreign Device" << std::endl;
		}
		break;
	}
	case 'm': {
		// Send text message
		uint8_t connectionString[6];
		memcpy(connectionString, g_database.networkPort.FdBbmdAddressHostIp, 4);
		connectionString[4] = g_database.networkPort.FdBbmdAddressPort / 256;
		connectionString[5] = g_database.networkPort.FdBbmdAddressPort % 256;

		uint8_t message[24] = "This is a test message.";
		std::cout << "Sending text message to" << g_database.networkPort.FdBbmdAddressHostIp[0] << "." << 
			g_database.networkPort.FdBbmdAddressHostIp[1] << "." << g_database.networkPort.FdBbmdAddressHostIp[2] << "." <<
			g_database.networkPort.FdBbmdAddressHostIp[3] << ":" << g_database.networkPort.FdBbmdAddressPort << std::endl;

		if (!CallbackSendMessage(message, 24, connectionString, 6, CASBACnetStackExampleConstants::NETWORK_TYPE_BACNET_IP, true)) {
			std::cout << "Error - failed to send Text Message" << std::endl;
		}
		break;
	}

	case 'h':
	default: {
		// Print the Help
		std::cout << std::endl << std::endl; 
		// Print the application version information 
		std::cout << "CAS BACnet Stack Server Example v" << APPLICATION_VERSION << "." << CIBUILDNUMBER << std::endl;
		std::cout << "https://github.com/chipkin/BACnetServerExampleCPP" << std::endl << std::endl;

		std::cout << "Help:" << std::endl;
		std::cout << "b - Add (B)roadcast Distribution Table entry" << std::endl;
		std::cout << "i - (i)ncrement Analog Value: " << g_database.analogValue.instance << " by 1.1" << std::endl;
		std::cout << "r - Toggle the Analog Input: 0 (r)eliability status" << std::endl;
		std::cout << "f - Send Register (foreign) device message" << std::endl;
		// std::cout << "d - (d)ebug" << std::endl;
		std::cout << "h - (h)elp" << std::endl;
		std::cout << "m - Send text (m)essage" << std::endl;
		std::cout << "q - (q)uit" << std::endl;
		std::cout << std::endl;
		break;
	}
	}

	return true;
}

// Callback used by the BACnet Stack to check if there is a message to process
uint16_t CallbackReceiveMessage(uint8_t* message, const uint16_t maxMessageLength, uint8_t* receivedConnectionString, const uint8_t maxConnectionStringLength, uint8_t* receivedConnectionStringLength, uint8_t* networkType)
{
	// Check parameters
	if (message == NULL || maxMessageLength == 0) {
		std::cerr << "Invalid input buffer" << std::endl;
		return 0;
	}
	if (receivedConnectionString == NULL || maxConnectionStringLength == 0) {
		std::cerr << "Invalid connection string buffer" << std::endl;
		return 0;
	}
	if (maxConnectionStringLength < 6) {
		std::cerr << "Not enough space for a UDP connection string" << std::endl;
		return 0;
	}

	char ipAddress[32];
	uint16_t port = 0;

	// Attempt to read bytes
	int bytesRead = g_udp.GetMessage(message, maxMessageLength, ipAddress, &port);
	if (bytesRead > 0) {
		ChipkinCommon::CEndianness::ToBigEndian(&port, sizeof(uint16_t));
		std::cout << std::endl <<  "FYI: Received message from [" << ipAddress << ":" << port << "], length [" << bytesRead << "]" << std::endl;

		// Convert the IP Address to the connection string
		if (!ChipkinCommon::ChipkinConvert::IPAddressToBytes(ipAddress, receivedConnectionString, maxConnectionStringLength)) {
			std::cerr << "Failed to convert the ip address into a connectionString" << std::endl;
			return 0;
		}
		receivedConnectionString[4] = port / 256;
		receivedConnectionString[5] = port % 256;

		*receivedConnectionStringLength = 6;
		*networkType = CASBACnetStackExampleConstants::NETWORK_TYPE_IP;

		/*
		// Process the message as XML
		static char xmlRenderBuffer[MAX_RENDER_BUFFER_LENGTH ]; 
		if (fpDecodeAsXML((char*)message, bytesRead, xmlRenderBuffer, MAX_RENDER_BUFFER_LENGTH ) > 0) {
			std::cout << "---------------------" << std::endl;				
			std::cout << xmlRenderBuffer << std::endl; 
			std::cout << "---------------------" << std::endl;
			memset(xmlRenderBuffer, 0, MAX_RENDER_BUFFER_LENGTH );
		}
		*/
		
		// Process the message as JSON
		static char jsonRenderBuffer[MAX_RENDER_BUFFER_LENGTH];
		if (fpDecodeAsJSON((char*)message, bytesRead, jsonRenderBuffer, MAX_RENDER_BUFFER_LENGTH) > 0) {
			std::cout << "---------------------" << std::endl;
			std::cout << jsonRenderBuffer << std::endl;
			std::cout << "---------------------" << std::endl;
			memset(jsonRenderBuffer, 0, MAX_RENDER_BUFFER_LENGTH);
		}
	}

	return bytesRead;
}

// Callback used by the BACnet Stack to send a BACnet message
uint16_t CallbackSendMessage(const uint8_t* message, const uint16_t messageLength, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, bool broadcast)
{
	std::cout << "CallbackSendMessage" << std::endl;

	if (message == NULL || messageLength == 0) {
		std::cout << "Nothing to send" << std::endl;
		return 0;
	}
	if (connectionString == NULL || connectionStringLength == 0) {
		std::cout << "No connection string" << std::endl;
		return 0;
	}

	// Verify Network Type
	if (networkType != CASBACnetStackExampleConstants::NETWORK_TYPE_IP) {
		std::cout << "Message for different network" << std::endl;
		return 0;
	}

	// Prepare the IP Address
	char ipAddress[32];
	if (broadcast) {
		snprintf( ipAddress, 32, "%hhu.%hhu.%hhu.%hhu",
			connectionString[0] | ~g_database.networkPort.IPSubnetMask[0],
			connectionString[1] | ~g_database.networkPort.IPSubnetMask[1],
			connectionString[2] | ~g_database.networkPort.IPSubnetMask[2],
			connectionString[3] | ~g_database.networkPort.IPSubnetMask[3]);
	}
	else {
		snprintf( ipAddress, 32, "%u.%u.%u.%u", connectionString[0], connectionString[1], connectionString[2], connectionString[3]);
	}

	// Get the port
	uint16_t port = 0;
	port += connectionString[4] * 256;
	port += connectionString[5];

	std::cout << std::endl << "FYI: Sending message to [" << ipAddress << ":"<< port <<"] length [" << messageLength << "]" << std::endl;

	// Send the message
	if (!g_udp.SendMessage(ipAddress, port, (unsigned char*)message, messageLength)) {
		std::cout << "Failed to send message" << std::endl;
		return 0;
	}

	/*
	// Get the XML rendered version of the just sent message
	static char xmlRenderBuffer[MAX_RENDER_BUFFER_LENGTH];
	if (fpDecodeAsXML((char*)message, messageLength, xmlRenderBuffer, MAX_RENDER_BUFFER_LENGTH) > 0) {
		std::cout << xmlRenderBuffer << std::endl;
		memset(xmlRenderBuffer, 0, MAX_RENDER_BUFFER_LENGTH);
	}
	*/
	
	// Get the JSON rendered version of the just sent message
	static char jsonRenderBuffer[MAX_RENDER_BUFFER_LENGTH];
	if (fpDecodeAsJSON((char*)message, messageLength, jsonRenderBuffer, MAX_RENDER_BUFFER_LENGTH) > 0) {
		std::cout << "---------------------" << std::endl;
		std::cout << jsonRenderBuffer << std::endl;
		std::cout << "---------------------" << std::endl;
		memset(jsonRenderBuffer, 0, MAX_RENDER_BUFFER_LENGTH);
	}

	return messageLength;
}

// Callback used by the BACnet Stack to get the current time
time_t CallbackGetSystemTime()
{
	return time(0) - g_database.device.currentTimeOffset;
}

// Callback used by the BACnet Stack to set the current time
bool CallbackSetSystemTime(const uint32_t deviceInstance, const uint8_t year, const uint8_t month, const uint8_t day, const uint8_t weekday, const uint8_t hour, const uint8_t minute, const uint8_t second, const uint8_t hundrethSeconds)
{

	// Calculate the time_t based on the passed in parameters
	struct tm timeInfo;
	timeInfo.tm_year = year;
	timeInfo.tm_mon = month - 1;
	timeInfo.tm_mday = day;
	timeInfo.tm_wday = weekday == 7 ? 0 : weekday;
	timeInfo.tm_hour = hour;
	timeInfo.tm_min = minute;
	timeInfo.tm_sec = second;

	// Calculate and store the current time offset
	g_database.device.currentTimeOffset = time(0) - mktime(&timeInfo);
	return true;
}

// Callback used by the BACnet Stack to get Bitstring property values from the user
bool CallbackGetPropertyBitString(uint32_t deviceInstance, uint16_t objectType, uint32_t objectInstance, uint32_t propertyIdentifier, bool* value, uint32_t* valueElementCount, uint32_t maxElementCount, bool useArrayIndex, uint32_t propertyArrayIndex)
{
	// Example of Bitstring Value Object Present Value property
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE && objectInstance == g_database.bitstringValue.instance) {
			if (g_database.bitstringValue.presentValue.size() > maxElementCount) {
				return false;
			}
			else {
				uint32_t valueArrayLength = 0;
				for (std::vector<bool>::const_iterator itr = g_database.bitstringValue.presentValue.begin(); itr != g_database.bitstringValue.presentValue.end(); itr++) {
					*(value + valueArrayLength) = (*itr);
				}

				*valueElementCount = valueArrayLength;
				return true;
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to get Boolean property values from the user
bool CallbackGetPropertyBool(uint32_t deviceInstance, uint16_t objectType, uint32_t objectInstance, uint32_t propertyIdentifier, bool* value, bool useArrayIndex, uint32_t propertyArrayIndex)
{
	// Example of Priority array Null handling
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRIORITY_ARRAY) {
		if (useArrayIndex) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT && objectInstance == g_database.analogOutput.instance) {
				if (propertyArrayIndex <= CASBACnetStackExampleConstants::MAX_BACNET_PRIORITY) {
					*value = g_database.analogOutput.priorityArrayNulls[propertyArrayIndex - 1];
					return true;
				}
				else {
					return false; // property array index out of range
				}
			}
			else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_OUTPUT && objectInstance == g_database.binaryOutput.instance) {
				if (propertyArrayIndex <= CASBACnetStackExampleConstants::MAX_BACNET_PRIORITY) {
					*value = g_database.binaryOutput.priorityArrayNulls[propertyArrayIndex - 1];
					return true;
				}
				else {
					return false; // property array index out of range
				}
			}
			else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT && objectInstance == g_database.multiStateOutput.instance) {
				if (propertyArrayIndex <= CASBACnetStackExampleConstants::MAX_BACNET_PRIORITY) {
					*value = g_database.multiStateOutput.priorityArrayNulls[propertyArrayIndex - 1];
					return true;
				}
				else {
					return false; // property array index out of range
				}
			}
		}
		else {
			return false;
		}
	}
	// Example of Device Day Light Savings Status property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_DAY_LIGHT_SAVINGS_STATUS) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && objectInstance == g_database.device.instance) {
			time_t currentTime = time(0);
			struct tm* timeinfo = localtime(&currentTime);
			*value = (timeinfo->tm_isdst != 0);
			return true;
		}
	}
	// Network Port Object - Changes Pending property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_CHANGES_PENDING) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_database.networkPort.instance) {
			*value = g_database.networkPort.ChangesPending;
			return true;
		}
	}
	return false;
}

// Callback used by the BACnet Stack to get Character String property values from the user
bool CallbackGetPropertyCharString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, char* value, uint32_t* valueElementCount, const uint32_t maxElementCount, uint8_t* encodingType, const bool useArrayIndex, const uint32_t propertyArrayIndex)
{
	// Example of Object Name property
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME) {
		return GetObjectName(deviceInstance, objectType, objectInstance, value, valueElementCount, maxElementCount);
	}
	// Example of Device Desription
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_DESCRIPTION) {
		if (g_database.device.description.size() <= maxElementCount) {
			memcpy(value, g_database.device.description.c_str(), g_database.device.description.size());
			*valueElementCount = (uint32_t)g_database.device.description.size();
			return true;
		}
		return false;
	}
	// Example of Character String Value Object Present Value property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_CHARACTERSTRING_VALUE && objectInstance == g_database.characterStringValue.instance) {
			if (g_database.characterStringValue.presentValue.size() <= maxElementCount) {
				memcpy(value, g_database.characterStringValue.presentValue.c_str(), g_database.characterStringValue.presentValue.size());
				*valueElementCount = (uint32_t)g_database.characterStringValue.presentValue.size();
				return true;
			}
		}
		return false;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE && propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_BIT_TEXT && objectInstance == g_database.bitstringValue.instance && useArrayIndex) {
		if (propertyArrayIndex <= g_database.bitstringValue.presentValue.size()) {
			memcpy(value, g_database.bitstringValue.bitText[propertyArrayIndex - 1].c_str(), g_database.bitstringValue.bitText[propertyArrayIndex - 1].size());
			*valueElementCount = (uint32_t)g_database.bitstringValue.bitText[propertyArrayIndex - 1].size();
			return true;
		}
		return false;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_DESCRIPTION && objectInstance == g_database.analogInput.instance) {
		if (g_database.analogInput.description.size() <= maxElementCount) {
			memcpy(value, g_database.analogInput.description.c_str(), g_database.analogInput.description.size());
			*valueElementCount = (uint32_t)g_database.analogInput.description.size();
			return true;
		}
		return false;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && propertyIdentifier == 512 + 1 && objectInstance == g_database.analogInput.instance)
	{
		*valueElementCount = snprintf( value, maxElementCount, "Example custom property 512 + 1");
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && propertyIdentifier == 512 + 2 && objectInstance == g_database.analogInput.instance)
	{
		*valueElementCount = snprintf(value, maxElementCount, "Example custom property 512 + 2");
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && propertyIdentifier == 512 + 3 && objectInstance == g_database.analogInput.instance)
	{
		*valueElementCount = snprintf(value, maxElementCount, "Example custom property 512 + 3");
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT && propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_STATE_TEXT && objectInstance == g_database.multiStateInput.instance) {
		if (useArrayIndex && propertyArrayIndex > 0 && propertyArrayIndex <= g_database.multiStateInput.numberOfStates) {
			// 0 is number of dates. 
			*valueElementCount = snprintf(value, maxElementCount, g_database.multiStateInput.stateText[propertyArrayIndex-1].c_str());
			return true;
		}
	}
	return false;
}

// Callback used by the BACnet Stack to get Date property values from the user
bool CallbackGetPropertyDate(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, uint8_t* year, uint8_t* month, uint8_t* day, uint8_t* weekday, const bool useArrayIndex, const uint32_t propertyArrayIndex)
{
	// Example of getting Date Value Object Present Value property
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DATE_VALUE && objectInstance == g_database.dateValue.instance) {
			*year = g_database.dateValue.presentValueYear;
			*month = g_database.dateValue.presentValueMonth;
			*day = g_database.dateValue.presentValueDay;
			*weekday = g_database.dateValue.presentValueWeekday;
			return true;
		}
	}
	// Example of getting Device Local Date property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_LOCAL_DATE) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && objectInstance == g_database.device.instance) {
			time_t adjustedTime = time(0) - g_database.device.currentTimeOffset;
			struct tm* timeInfo = localtime(&adjustedTime);
			*year = timeInfo->tm_year;
			*month = timeInfo->tm_mon + 1;
			*day = timeInfo->tm_mday;
			*weekday = timeInfo->tm_wday == 0 ? 7 : timeInfo->tm_wday;
			return true;
		}
	}
	// Example of getting DateTime Value Object Present Value property
	if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DATETIME_VALUE && objectInstance == 60) {
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			*year = g_database.dateTimeValue.presentValueYear;
			*month = g_database.dateTimeValue.presentValueMonth;
			*day = g_database.dateTimeValue.presentValueDay;
			*weekday = g_database.dateTimeValue.presentValueWeekDay;
			return true;
			}
	}
	// Example of getting Analog Input object Date Time Proprietary property
	if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_database.analogInput.instance) {
		if (propertyIdentifier == 512 + 4) {
			*year = g_database.analogInput.proprietaryYear;
			*month = g_database.analogInput.proprietaryMonth;
			*day = g_database.analogInput.proprietaryDay;
			*weekday = g_database.analogInput.proprietaryWeekDay;
			return true;
		}
	}

	return false;
}

// Callback used by the BACnet Stack to get Dboule property values from the user
bool CallbackGetPropertyDouble(uint32_t deviceInstance, uint16_t objectType, uint32_t objectInstance, uint32_t propertyIdentifier, double* value, bool useArrayIndex, uint32_t propertyArrayIndex)
{
	// Example of Large Analg Value Object Present Value property
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_LARGE_ANALOG_VALUE && objectInstance == g_database.largeAnalogValue.instance) {
			*value = g_database.largeAnalogValue.presentValue;
			return true;
		}
	}
	return false;
}

// Callback used by the BACnet Stack to get Enumerated property values from the user
bool CallbackGetPropertyEnum(uint32_t deviceInstance, uint16_t objectType, uint32_t objectInstance, uint32_t propertyIdentifier, uint32_t* value, bool useArrayIndex, uint32_t propertyArrayIndex)
{
	std::cout << "CallbackGetPropertyEnum deviceInstance=" << deviceInstance << ", objectType=" << objectType << ", objectInstance=" << objectInstance << ", propertyIdentifier=" << propertyIdentifier << ", useArrayIndex=" << useArrayIndex << ", propertyArrayIndex=" << propertyArrayIndex << std::endl; 

	// Example of Binary Input / Value Object Present Value property
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_INPUT && objectInstance == g_database.binaryInput.instance) {
			*value = g_database.binaryInput.presentValue;
			return true;
		}
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_VALUE && objectInstance == g_database.binaryValue.instance) {
			*value = g_database.binaryValue.presentValue;
			return true;
		}
	}
	// Example of Binary Output Priority Array property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRIORITY_ARRAY) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_OUTPUT && objectInstance == g_database.binaryOutput.instance) {
			if (useArrayIndex) {
				if (propertyArrayIndex <= CASBACnetStackExampleConstants::MAX_BACNET_PRIORITY) {
					*value = g_database.binaryOutput.priorityArrayValues[propertyArrayIndex - 1];
					return true;
				}
				else {
					return false; // property array index out of range
				}
			}
			else {
				return false;
			}
		}
	}
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_RELIABILITY) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_database.analogInput.instance) {
			*value = g_database.analogInput.reliability; 
			return true;
		}
	}
	// Network Port Object - FdBbmdAddress Host Type
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_FD_BBMD_ADDRESS) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_database.networkPort.instance) {
			*value = g_database.networkPort.FdBbmdAddressHostType;
			return true;
		}
	}
	
	// Debug for customer 
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_SYSTEM_STATUS &&
		objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE)
	{
		std::cout << "Debug: Device:System Status" << std::endl;
		*value = g_database.device.systemStatus;
		return true;
	}


	// We could not answer this request. 
	return false;
}

// Callback used by the BACnet Stack to get OctetString property values from the user
bool CallbackGetPropertyOctetString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, uint8_t* value, uint32_t* valueElementCount, const uint32_t maxElementCount, const bool useArrayIndex, const uint32_t propertyArrayIndex)
{
	// Example of Octet String Value Object Present Value property
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_OCTETSTRING_VALUE && objectInstance == g_database.octetStringValue.instance) {
			if (g_database.octetStringValue.presentValue.size() > maxElementCount) {
				return false;
			}
			else {

				uint32_t valueLength = 0;
				for (std::vector<uint8_t>::const_iterator itr = g_database.octetStringValue.presentValue.begin(); itr != g_database.octetStringValue.presentValue.end(); itr++) {
					*(value + valueLength) = (*itr);
					valueLength++;
				}
				*valueElementCount = valueLength;

				// memcpy(value, g_database.octetStringValuePresentValue, g_database.octetStringValue.presentValue.size());
				// *valueElementCount = g_database.octetStringValue.presentValue.size();
				return true;
			}
		}
	}
	// Example of Network Port Object IP Address property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_IP_ADDRESS) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_database.networkPort.instance) {
			memcpy(value, g_database.networkPort.IPAddress, g_database.networkPort.IPAddressLength);
			*valueElementCount = g_database.networkPort.IPAddressLength;
			return true;
		}
	}
	// Example of Network Port Object IP Default Gateway property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_IP_DEFAULT_GATEWAY) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_database.networkPort.instance) {
			memcpy(value, g_database.networkPort.IPDefaultGateway, g_database.networkPort.IPDefaultGatewayLength);
			*valueElementCount = g_database.networkPort.IPDefaultGatewayLength;
			return true;
		}
	}
	// Example of Network Port Object IP Subnet Mask property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_IP_SUBNET_MASK) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_database.networkPort.instance) {
			memcpy(value, g_database.networkPort.IPSubnetMask, g_database.networkPort.IPSubnetMaskLength);
			*valueElementCount = g_database.networkPort.IPSubnetMaskLength;
			return true;
		}
	}
	// Example of Network Port Object IP DNS Server property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_IP_DNS_SERVER) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_database.networkPort.instance) {
			// The IP DNS Server property is an array of DNS Server addresses
			if (useArrayIndex) {
				if (propertyArrayIndex != 0 && propertyArrayIndex <= g_database.networkPort.IPDNSServers.size()) {
					memcpy(value, g_database.networkPort.IPDNSServers[propertyArrayIndex - 1], g_database.networkPort.IPDNSServerLength);
					*valueElementCount = g_database.networkPort.IPDNSServerLength;
					return true;
				}
			}
		}
	}
	// Network Port Object FdBbmdAddress Host (as IP Address)
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_FD_BBMD_ADDRESS) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_database.networkPort.instance) {
			if (useArrayIndex && propertyArrayIndex == CASBACnetStackExampleConstants::HOST_TYPE_IPADDRESS) {
				memcpy(value, g_database.networkPort.FdBbmdAddressHostIp, 4);
				*valueElementCount = 4;
				return true;
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to get Integer property values from the user
bool CallbackGetPropertyInt(uint32_t deviceInstance, uint16_t objectType, uint32_t objectInstance, uint32_t propertyIdentifier, int32_t* value, bool useArrayIndex, uint32_t propertyArrayIndex)
{
	// Example of Integer Value Object Present Value property
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_INTEGER_VALUE && objectInstance == g_database.integerValue.instance) {
			*value = g_database.integerValue.presentValue;
			return true;
		}
	}
	// Example of Device UTC Offset property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_UTC_OFFSET) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && objectInstance == g_database.device.instance) {
			*value = g_database.device.UTCOffset;
			return true;
		}
	}
	return false;
}

// Callback used by the BACnet Stack to get Real property values from the user
bool CallbackGetPropertyReal(uint32_t deviceInstance, uint16_t objectType, uint32_t objectInstance, uint32_t propertyIdentifier, float* value, bool useArrayIndex, uint32_t propertyArrayIndex)
{

	// Example of Analog Input / Value Object Present Value property
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_database.analogInput.instance) {
			*value = g_database.analogInput.presentValue;
			return true;
		}
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && objectInstance == g_database.analogValue.instance) {
			*value = g_database.analogValue.presentValue;
			return true;
		}
		// Check if this is for a created analog value
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && g_database.CreatedAnalogValueData.count(objectInstance) > 0) {
			*value = g_database.CreatedAnalogValueData[objectInstance].value;
			return true;
		}
	}
	// Example of Analog Output Priority Array property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRIORITY_ARRAY) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT && objectInstance == g_database.analogOutput.instance) {
			if (useArrayIndex) {
				if (propertyArrayIndex <= CASBACnetStackExampleConstants::MAX_BACNET_PRIORITY) {
					*value = g_database.analogOutput.priorityArrayValues[propertyArrayIndex - 1];
					return true;
				}
				else {
					return false; // property array index out of range
				}
			}
			else {
				return false;
			}
		}
	}
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_COV_INCURMENT && objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_database.analogInput.instance) {
		*value = g_database.analogInput.covIncurment;
		return true;
	}
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_MAX_PRES_VALUE && objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && objectInstance == g_database.analogValue.instance) {
		*value = g_database.analogValue.maxPresValue;
		return true;
	}
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_MIN_PRES_VALUE && objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && objectInstance == g_database.analogValue.instance) {
		*value = g_database.analogValue.minPresValue;
		return true;
	}

	return false;
}

// Callback used by the BACnet Stack to get Time property values from the user
bool CallbackGetPropertyTime(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, uint8_t* hour, uint8_t* minute, uint8_t* second, uint8_t* hundrethSeconds, const bool useArrayIndex, const uint32_t propertyArrayIndex)
{
	// Example of getting Time Value Object Present Value property
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_TIME_VALUE && objectInstance == g_database.timeValue.instance) {
			*hour = g_database.timeValue.presentValueHour;
			*minute = g_database.timeValue.presentValueMinute;
			*second = g_database.timeValue.presentValueSecond;
			*hundrethSeconds = g_database.timeValue.presentValueHundrethSecond;
			return true;
		}
	}
	// Example of getting Device Local Time property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_LOCAL_TIME) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && objectInstance == g_database.device.instance) {
			time_t adjustedTime = time(0) - g_database.device.currentTimeOffset;
			struct tm* timeInfo = localtime(&adjustedTime);
			*hour = timeInfo->tm_hour;
			*minute = timeInfo->tm_min;
			*second = timeInfo->tm_sec;
			*hundrethSeconds = 0;
			return true;
		}
	}
	// Example of getting DateTime Value Object Present Value property
	if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DATETIME_VALUE && objectInstance == 60) {
			if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			*hour = g_database.dateTimeValue.presentValueHour;
			*minute = g_database.dateTimeValue.presentValueMinute;
			*second = g_database.dateTimeValue.presentValueSecond;
			*hundrethSeconds = g_database.dateTimeValue.presentValueHundredthSeconds;
			return true;
			}
	}
	// Example of getting Analog Input object DateTime Proprietary property
	if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_database.analogInput.instance) {
		if (propertyIdentifier == 512 + 4) {
			*hour = g_database.analogInput.proprietaryHour;
			*minute = g_database.analogInput.proprietaryMinute;
			*second = g_database.analogInput.proprietarySecond;
			*hundrethSeconds = g_database.analogInput.proprietaryHundredthSeconds;
			return true;
		}
	}

	return false;
}

// Callback used by the BACnet Stack to get Unsigned Integer property values from the user
bool CallbackGetPropertyUInt(uint32_t deviceInstance, uint16_t objectType, uint32_t objectInstance, uint32_t propertyIdentifier, uint32_t* value, bool useArrayIndex, uint32_t propertyArrayIndex)
{
	// Example of Positive Integer Value Object and Multi-State Input / Value Objects Present Value property
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_POSITIVE_INTEGER_VALUE && objectInstance == g_database.positiveIntegerValue.instance) {
			*value = g_database.positiveIntegerValue.presentValue;
			return true;
		}
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT && objectInstance == g_database.multiStateInput.instance) {
			*value = g_database.multiStateInput.presentValue;
			return true;
		}
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE && objectInstance == g_database.multiStateValue.instance) {
			*value = g_database.multiStateValue.presentValue;
			return true;
		}
	}
	// Example of Multi-State Output Priority Array property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRIORITY_ARRAY) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT && objectInstance == g_database.multiStateOutput.instance) {
			if (useArrayIndex) {
				if (propertyArrayIndex <= CASBACnetStackExampleConstants::MAX_BACNET_PRIORITY) {
					*value = g_database.multiStateOutput.priorityArrayValues[propertyArrayIndex - 1];
					return true;
				}
				else {
					return false; // property array index out of range
				}
			}
			else {
				return false;
			}
		}
	}
	// Example of Network Port Object BACnet IP UDP Port property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_BACNET_IP_UDP_PORT) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_database.networkPort.instance) {
			*value = g_database.networkPort.BACnetIPUDPPort;
			return true;
		}
	}
	// Example of Network Port Object IP DNS Server Array Size property
	// Any properties that are an array must have an entry here for the array size.
	// The array size is provided only if the useArrayIndex parameter is set to true and the propertyArrayIndex is zero.
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_IP_DNS_SERVER) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_database.networkPort.instance) {
			if (useArrayIndex && propertyArrayIndex == 0) {
				*value = (uint32_t)g_database.networkPort.IPDNSServers.size();
				return true;
			}
		}
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE && propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_BIT_TEXT) {
		if (objectInstance == g_database.bitstringValue.instance && useArrayIndex && propertyArrayIndex == 0) {
			*value = (uint32_t)g_database.bitstringValue.presentValue.size();
			return true;
		}
	}
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_NUMBER_OF_STATES) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT && objectInstance == g_database.multiStateInput.instance) {
			*value = g_database.multiStateInput.numberOfStates;
			return true;
		}
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT && objectInstance == g_database.multiStateOutput.instance) {
			*value = g_database.multiStateOutput.numberOfStates;
			return true;
		}
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE && objectInstance == g_database.multiStateValue.instance) {
			*value = g_database.multiStateValue.numberOfStates;
			return true;
		}
	}
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_STATE_TEXT) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT && objectInstance == g_database.multiStateInput.instance) {
			*value = g_database.multiStateInput.numberOfStates;
			return true;
		}
	}
	// Network Port Object FdBbmdAddress Port
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_FD_BBMD_ADDRESS) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_database.networkPort.instance) {
			if (useArrayIndex && propertyArrayIndex == CASBACnetStackExampleConstants::FD_BBMD_ADDRESS_PORT) {
				// Check for index 2, which is looking for the fdBbmdAddress port portion
				*value = g_database.networkPort.FdBbmdAddressPort;
				return true;
			}
		}
	}
	// Network Port Object FdSubscriptionLifetime
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_FD_SUBSCRIPTION_LIFETIME) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_database.networkPort.instance) {
			*value = g_database.networkPort.FdSubscriptionLifetime;
			return true;
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set Bitstring property values to the user
bool CallbackSetPropertyBitString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool* value, const uint32_t length, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode)
{
	if (deviceInstance == g_database.device.instance) {
		// Example of writing to Bitstring Value Object Present Value property
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE && objectInstance == g_database.bitstringValue.instance) {
				if (length > g_database.bitstringValue.presentValue.size()) {
					*errorCode = CASBACnetStackExampleConstants::ERROR_NO_SPACE_TO_WRITE_PROPERTY;
					return false;
				}
				else {
					g_database.bitstringValue.Resize(length);
					for (uint32_t offset = 0; offset < length; offset++) {
						g_database.bitstringValue.SetPresentValue(offset, *(value + offset));
					}
					return true;
				}
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set Boolean property values to the user
bool CallbackSetPropertyBool(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode)
{
	return false;
}

// Callback used by the BACnet Stack to set Charstring property values to the user
bool CallbackSetPropertyCharString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const char* value, const uint32_t length, const uint8_t encodingType, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode)
{
	if (deviceInstance == g_database.device.instance) {
		// Example of setting Charstring Value Object Present Value property
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_CHARACTERSTRING_VALUE && objectInstance == g_database.characterStringValue.instance) {
				g_database.characterStringValue.presentValue = std::string(value, length);
				return true;
			}
			return false;
		}
		// Example of setting description property of the Device
		else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_DESCRIPTION) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && objectInstance == g_database.device.instance) {
				g_database.device.description = std::string(value, length);
				return true;
			}
			return false;
		}
		// Example of setting object name property of the Device
		else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && objectInstance == g_database.device.instance) {
				g_database.device.objectName = std::string(value, length);
				return true;
			}
			return false;
		}
		// Check if trying to set the object name of an analog value that was created.
		// Used in initializing objects
		else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME && objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && g_database.CreatedAnalogValueData.count(objectInstance) > 0) {
			g_database.CreatedAnalogValueData[objectInstance].name = std::string(value, length);
			return true;
		}
	}

	return false;
}

// Callback used by the BACnet Stack to set Date property values to the user
bool CallbackSetPropertyDate(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint8_t year, const uint8_t month, const uint8_t day, const uint8_t weekday, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode)
{
	if (deviceInstance == g_database.device.instance) {
		// Example of setting Date Value Present Value property
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DATE_VALUE && objectInstance == g_database.dateValue.instance) {
				g_database.dateValue.presentValueYear = year;
				g_database.dateValue.presentValueMonth = month;
				g_database.dateValue.presentValueDay = day;
				g_database.dateValue.presentValueWeekday = weekday;
				return true;
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set Double property values to the user
bool CallbackSetPropertyDouble(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const double value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode)
{
	if (deviceInstance == g_database.device.instance) {
		// Example of setting Large Analog Value Object Present Value property
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_LARGE_ANALOG_VALUE && objectInstance == g_database.largeAnalogValue.instance) {
				g_database.largeAnalogValue.presentValue = value;
				return true;
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set Enumerated property values to the user
bool CallbackSetPropertyEnum(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint32_t value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode)
{
	if (deviceInstance == g_database.device.instance) {
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			// Example of setting Binary Value Present Value property
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_VALUE && objectInstance == g_database.binaryValue.instance) {
				g_database.binaryValue.presentValue = value;
				return true;
			}
			// Example of setting Binary Output Present Value / Priority Array property
			else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_OUTPUT && objectInstance == g_database.binaryOutput.instance) {
				g_database.binaryOutput.priorityArrayValues[priority - 1] = value;
				g_database.binaryOutput.priorityArrayNulls[priority - 1] = false;
				return true;
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set NULL property values to the user
// 
// This is commonly used when a BACnet client 'reliqunishes' a value in a object that has a priority array. The client sends a 
// WriteProperty message with a value of "NULL" to the present value with a priority. When the CAS BACnet Stack receives this 
// message, it will call the CallbackSetPropertyNull callback function with the write priorty.
bool CallbackSetPropertyNull(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode)
{
	if (deviceInstance == g_database.device.instance) {
		// Examples of setting Analog, Binary, and Multi-State Outputs Present Value / Priority Array property to Null
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT && objectInstance == g_database.analogOutput.instance) {
				g_database.analogOutput.priorityArrayValues[priority - 1] = 0.0f;
				g_database.analogOutput.priorityArrayNulls[priority - 1] = true;
				return true;
			}
			else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_OUTPUT && objectInstance == g_database.binaryOutput.instance) {
				g_database.binaryOutput.priorityArrayValues[priority - 1] = 0;
				g_database.binaryOutput.priorityArrayNulls[priority - 1] = true;
				return true;
			}
			else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT && objectInstance == g_database.multiStateOutput.instance) {
				g_database.multiStateOutput.priorityArrayValues[priority - 1] = 0;
				g_database.multiStateOutput.priorityArrayNulls[priority - 1] = true;
				return true;
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set OctetString property values to the user
bool CallbackSetPropertyOctetString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint8_t* value, const uint32_t length, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode)
{
	if (deviceInstance == g_database.device.instance) {
		// Example of setting Octet String Value Object Present Value property
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_OCTETSTRING_VALUE && objectInstance == g_database.octetStringValue.instance) {
				if (length > g_database.octetStringValue.presentValue.size()) {
					*errorCode = CASBACnetStackExampleConstants::ERROR_NO_SPACE_TO_WRITE_PROPERTY;
					return false;
				}
				else {
					g_database.octetStringValue.presentValue.resize(length);
					for (uint32_t offset = 0; offset < length; offset++) {
						g_database.octetStringValue.presentValue[offset] = *(value + offset);
					}
					return true;
				}
			}
		}

		// Example of setting FdBbmdAddress Host IP
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_FD_BBMD_ADDRESS) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_database.networkPort.instance) {
				if (useArrayIndex && propertyArrayIndex == CASBACnetStackExampleConstants::FD_BBMD_ADDRESS_HOST) {
					if (length > 4) {
						*errorCode = CASBACnetStackExampleConstants::ERROR_VALUE_OUT_OF_RANGE;
						return false;
					}
					if (memcmp(g_database.networkPort.FdBbmdAddressHostIp, value, length) == 0) {
						// No change, return true
						return true;
					}
					else {
						// Store new value and set changes pending to true
						memcpy(g_database.networkPort.FdBbmdAddressHostIp, value, length);
						g_database.networkPort.ChangesPending = true;
						return true;
					}
				}
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set Integer property values to the user
bool CallbackSetPropertyInt(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const int32_t value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode)
{
	if (deviceInstance == g_database.device.instance) {
		// Example of setting Integer Value Object Present Value property
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_INTEGER_VALUE && objectInstance == g_database.integerValue.instance) {
				g_database.integerValue.presentValue = value;
				return true;
			}
		}
		// Example of setting Device UTC Offset property
		else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_UTC_OFFSET) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && objectInstance == g_database.device.instance) {
				if (value < -1440 || value > 1440) {
					*errorCode = CASBACnetStackExampleConstants::ERROR_VALUE_OUT_OF_RANGE;
					return false;
				}
				else {
					g_database.device.UTCOffset = value;
					return true;
				}
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set Real property values to the user
bool CallbackSetPropertyReal(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const float value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode)
{
	if (deviceInstance != g_database.device.instance) {
		return false; // Not this device.
	}
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
		// Example of setting Analog Value Present Value Property
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && objectInstance == g_database.analogValue.instance) {
			if (value < g_database.analogValue.minPresValue || value > g_database.analogValue.maxPresValue) {
				*errorCode = CASBACnetStackExampleConstants::ERROR_VALUE_OUT_OF_RANGE;
				return false;
			}
			g_database.analogValue.presentValue = value;
			return true;
		}
		// Example of setting Analog Output Present Value / Priority Array property
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT && objectInstance == g_database.analogOutput.instance) {
			g_database.analogOutput.priorityArrayValues[priority - 1] = value;
			g_database.analogOutput.priorityArrayNulls[priority - 1] = false;
			return true;
		}
		// Check if setting present value of a create analog value
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && g_database.CreatedAnalogValueData.count(objectInstance) > 0) {
			g_database.CreatedAnalogValueData[objectInstance].value = value;
			return true;
		}
	}
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_COV_INCURMENT) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_database.analogInput.instance) {
			g_database.analogInput.covIncurment = value;
			return true;
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set Time property values to the user
bool CallbackSetPropertyTime(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint8_t hour, const uint8_t minute, const uint8_t second, const uint8_t hundrethSeconds, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode)
{
	if (deviceInstance == g_database.device.instance) {
		// Example of setting Time Value Object Present Value property
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_TIME_VALUE && objectInstance == g_database.timeValue.instance) {
				g_database.timeValue.presentValueHour = hour;
				g_database.timeValue.presentValueMinute = minute;
				g_database.timeValue.presentValueSecond = second;
				g_database.timeValue.presentValueHundrethSecond = hundrethSeconds;
				return true;
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set Date property values to the user
bool CallbackSetPropertyUInt(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint32_t value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, unsigned int* errorCode)
{
	if (deviceInstance == g_database.device.instance) {
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			// Example of setting Positive Integer Value Object Present Value property
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_POSITIVE_INTEGER_VALUE && objectInstance == g_database.positiveIntegerValue.instance) {
				g_database.positiveIntegerValue.presentValue = value;
				return true;
			}
			// Example of setting Multi-State Value Object Present Value property
			else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE && objectInstance == g_database.multiStateValue.instance) {
				g_database.multiStateValue.presentValue = value;
				return true;
			}
			// Example of setting Multi-State Output Present Value / Priority Array property
			else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT && objectInstance == g_database.multiStateOutput.instance) {
				g_database.multiStateOutput.priorityArrayValues[priority - 1] = value;
				g_database.multiStateOutput.priorityArrayNulls[priority - 1] = false;
				return true;
			}
		}
		// Network Port Object FdBbmdAddress Port
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_FD_BBMD_ADDRESS) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_database.networkPort.instance) {
				if (useArrayIndex && propertyArrayIndex == CASBACnetStackExampleConstants::FD_BBMD_ADDRESS_PORT) {
					g_database.networkPort.FdBbmdAddressPort = value;
					g_database.networkPort.ChangesPending = true;
					return true;
				}
			}
		}
		// Network Port Object FdSubscriptionLifetime
		else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_FD_SUBSCRIPTION_LIFETIME) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_database.networkPort.instance) {
				g_database.networkPort.FdSubscriptionLifetime = value;
				g_database.networkPort.ChangesPending = true;
				return true;
			}
		}
	}

	return false;
}

// Gets the object name based on the provided parameters
bool GetObjectName(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, char* value, uint32_t* valueElementCount, const uint32_t maxElementCount)
{
	size_t stringSize = 0;
	if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && objectInstance == g_database.device.instance) {
		stringSize = g_database.device.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance <<" ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.device.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t)stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_database.analogInput.instance) {
		stringSize = g_database.analogInput.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.analogInput.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == 389 ) {
		std::string name = "This is an example of the name";
		stringSize = name.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, name.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && objectInstance == g_database.analogValue.instance) {
		stringSize = g_database.analogValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.analogValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_INPUT && objectInstance == g_database.binaryInput.instance) {
		stringSize = g_database.binaryInput.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.binaryInput.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_OUTPUT && objectInstance == g_database.binaryOutput.instance) {
		stringSize = g_database.binaryOutput.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.binaryOutput.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_VALUE && objectInstance == g_database.binaryValue.instance) {
		stringSize = g_database.binaryValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.binaryValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT && objectInstance == g_database.multiStateInput.instance) {
		stringSize = g_database.multiStateInput.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.multiStateInput.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT && objectInstance == g_database.multiStateOutput.instance) {
		stringSize = g_database.multiStateOutput.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.multiStateOutput.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE && objectInstance == g_database.multiStateValue.instance) {
		stringSize = g_database.multiStateValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.multiStateValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_TREND_LOG && objectInstance == g_database.trendLog.instance) {
		stringSize = g_database.trendLog.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.trendLog.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_TREND_LOG_MULTIPLE && objectInstance == g_database.trendLogMultiple.instance) {
	stringSize = g_database.trendLogMultiple.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << "]" << std::endl;
			return false;
		}
		memcpy(value, g_database.trendLogMultiple.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t)stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE && objectInstance == g_database.bitstringValue.instance) {
		stringSize = g_database.bitstringValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.bitstringValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_CHARACTERSTRING_VALUE && objectInstance == g_database.characterStringValue.instance) {
		stringSize = g_database.characterStringValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.characterStringValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DATE_VALUE && objectInstance == g_database.dateValue.instance) {
		stringSize = g_database.dateValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.dateValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_INTEGER_VALUE && objectInstance == g_database.integerValue.instance) {
		stringSize = g_database.integerValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance <<" ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.integerValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t)stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_LARGE_ANALOG_VALUE && objectInstance == g_database.largeAnalogValue.instance) {
		stringSize = g_database.largeAnalogValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance <<" ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.largeAnalogValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_OCTETSTRING_VALUE && objectInstance == g_database.octetStringValue.instance) {
		stringSize = g_database.octetStringValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance <<" ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.octetStringValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_POSITIVE_INTEGER_VALUE && objectInstance == g_database.positiveIntegerValue.instance) {
		stringSize = g_database.positiveIntegerValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance <<" ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.positiveIntegerValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_TIME_VALUE && objectInstance == g_database.timeValue.instance) {
		stringSize = g_database.timeValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance <<" ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.timeValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_database.networkPort.instance) {
		stringSize = g_database.networkPort.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance <<" ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.networkPort.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DATETIME_VALUE && objectInstance == g_database.dateTimeValue.instance) {
		stringSize = g_database.dateTimeValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance <<" ]" << std::endl;
			return false;
		}
		memcpy(value, g_database.dateTimeValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else {
		// Check if the value is an Analog Value and check if it was a created object
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && g_database.CreatedAnalogValueData.count(objectInstance) > 0) {
			std::string objectName = g_database.CreatedAnalogValueData[objectInstance].name;
			memcpy(value, objectName.c_str(), objectName.size());
			*valueElementCount = (uint32_t)objectName.size();
			return true;
		}

		return false;
	}
}

bool CallbackCreateObject(
	const uint32_t deviceInstance,
	const uint16_t objectType,
	const uint32_t objectInstance)
{
	// This callback is called when this BACnet Server device receives a CreateObject message
	// In this callback, you can allocate memory to store properties that you would store
	// For example, present-value and object name

	// In this example, we will only allow Analog-Values to be enabled
	// See the SetupBACnetDeviceFunction on how this is handled
	if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE) {
		g_database.CreatedAnalogValueData.insert(std::pair<uint32_t, CreatedAnalogValue>(objectInstance, CreatedAnalogValue()));
		g_database.CreatedAnalogValueData[objectInstance].name = std::string("AnalogValue_") + ChipkinCommon::ChipkinConvert::ToString(objectInstance);
		return true;
	}
	return false;
}

bool CallbackDeleteObject(
	const uint32_t deviceInstance,
	const uint16_t objectType,
	const uint32_t objectInstance)
{
	// This callback is called when this BACnet Server device receives a DeleteObject message
	// In this callbcak, you can clean up any memory that was allocated when the object was
	// initially created.

	if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE) {
		if (g_database.CreatedAnalogValueData.count(objectInstance) > 0) {
			g_database.CreatedAnalogValueData.erase(objectInstance);
			return true;
		}
	}
	return false;
}

bool CallbackReinitializeDevice(const uint32_t deviceInstance, const uint32_t reinitializedState, const char* password, const uint32_t passwordLength, uint32_t* errorCode) {
	// This callback is called when this BACnet Server device receives a ReinitializeDevice message
	// In this callback, you will handle the reinitializedState.
	// If reinitializedState = ACTIVATE_CHANGES (7) then you will apply any network port changes and store the values in non-volatile memory
	// If reinitializedState = WARM_START(1) then you will apply any network port changes, store the values in non-volatile memory, and restart the device.

	// Before handling the reinitializedState, first check the password.
	// If your device does not require a password, then ignore any password passed in.
	// Otherwise, validate the password.
	//		If password invalid, or a password is required, but no password was provided: set errorCode to PasswordInvalid (26)
	// In this example, a password of 12345 is required.
	
	// Check if missing
	if (password == NULL || passwordLength == 0) {
		*errorCode = CASBACnetStackExampleConstants::ERROR_PASSWORD_FAILURE;
		return false;
	}

	// Check if correct
	if (strcmp(password, "12345") != 0) {
		*errorCode = CASBACnetStackExampleConstants::ERROR_PASSWORD_FAILURE;
		return false;
	}

	// In this example, only the NetworkPort Object FdBbmdAddress and FdSubscriptionLifetime properties are writable and need to be
	// stored in non-volatile memory.  For the purpose of this example, we will not storing these values in non-volaitle memory.

	// 1. Store values that must be stored in non-volatile memory (i.e. must survive a reboot).

	// 2. Apply any Network Port values that have been written to. 
	// If any validation on the Network Port values failes, set errorCode to INVALID_CONFIGURATION_DATA (46)

	// 3. Set Network Port ChangesPending property to false

	// 4. Handle ReinitializedState. If ACTIVATE_CHANGES, no other action, return true.
	//								 If WARM_START, prepare device for reboot, return true. and reboot.  
	// NOTE: Must return true first before rebooting so the stack sends the SimpleAck.
	if (reinitializedState == CASBACnetStackExampleConstants::REINITIALIZED_STATE_ACTIVATE_CHANGES) {
		g_database.networkPort.ChangesPending = false;
		return true;
	}
	else if (reinitializedState == CASBACnetStackExampleConstants::REINITIALIZED_STATE_WARM_START) {
		// Flag for reboot and handle reboot after stack responds with SimpleAck.
		g_database.networkPort.ChangesPending = false;
		return true;
	}
	else {
		// All other states are not supported in this example.
		*errorCode = CASBACnetStackExampleConstants::ERROR_OPTIONAL_FUNCTIONALITY_NOT_SUPPORTED;
		return false;
	}
}

bool CallbackDeviceCommunicationControl(const uint32_t deviceInstance, const uint8_t enableDisable, const char* password, const uint8_t passwordLength, const bool useTimeDuration, const uint16_t timeDuration, uint32_t* errorCode) {
	// This callback is called when this BACnet Server device receives a DeviceCommunicationControl message
	// In this callback, you will handle the password. All other parameters are purely for logging to know
	// what parameters the DeviceCommunicationControl request had.
	
	// To handle the password:
	// If your device does not require a password, then ignore any password passed in.
	// Otherwise, validate the password.
	//		If password invalid: set errorCode to PasswordInvalid (26)
	//		If password is required, but no password was provided: set errorCode to MissingRequiredParameter (16)
	// In this example, a password of 12345 is required.
	if (password == NULL || passwordLength == 0) {
		*errorCode = CASBACnetStackExampleConstants::ERROR_MISSING_REQUIRED_PARAMETER;
		return false;
	}
	if (strcmp(password, "12345") != 0) {
		*errorCode = CASBACnetStackExampleConstants::ERROR_PASSWORD_FAILURE;
		return false;
	}

	// Must return true to allow for the DeviceCommunicationControl logic to continue
	return true;
}

void CallbackLogDebugMessage(const char* message, const uint16_t messageLength, const uint8_t messageType) {
	// This callback is called when the CAS BACnet Stack logs an error or info message
	// In this callback, you will be able to access this debug message. This callback is optional.
	std::cout << std::string(message, messageLength) << std::endl;
	return;
}

bool HookTextMessage(const uint32_t sourceDeviceIdentifier, const bool useMessageClass, const uint32_t messageClassUnsigned, const char* messageClassString, const uint32_t messageClassStringLength, const uint8_t messagePriority, const char* message, const uint32_t messageLength, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t sourceNetwork, const uint8_t* sourceAddress, const uint8_t sourceAddressLength, uint16_t* errorClass, uint16_t* errorCode) {
	// Configured to respond to Client example Confirmed Text Message Requests
	uint32_t expectedSourceDeviceIdentifier = 389002;
	uint32_t expectedMessageClass = 5;
	uint8_t expectedMessagePriority = 0; // normal

	// Check that this device is configured to do some logic using the text message
	if (sourceDeviceIdentifier == expectedSourceDeviceIdentifier &&
		messageClassUnsigned == expectedMessageClass &&
		messagePriority == expectedMessagePriority) {

		// Perform some logic using the message
		std::cout << std::endl << "Received text message request meant for us to perform some logic: " << message << std::endl;

		// Device is configured to handle the confirmed text message, response is Result(+) or simpleAck
		return true;
	}

	// This device is not configured to handle the text message, response is Result(-)
	// Ignored for Unconfirmed Text Message Requests

	// Create an error
	uint16_t deviceErrorClass = 0;
	uint16_t notConfiguredErrorCode = 132;

	*errorClass = deviceErrorClass;
	*errorCode = notConfiguredErrorCode;
	return false;
}

