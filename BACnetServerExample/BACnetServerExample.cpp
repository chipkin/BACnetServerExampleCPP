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
ExampleDatabase g_exampleDatabase; // The example database that stores current values.
bool g_bbmdEnabled; // Flag for whether bbmd was enabled or not.  Users can enable bbmd by pressing 'b' after the application has started.
bool g_warmStart; // Flag for when warm start reinitialization is requested.
time_t g_warmStartTimer; // Timer used for delaying the warm start.

// Constants
// =======================================
const std::string APPLICATION_VERSION = "0.0.25";  // See CHANGELOG.md for a full list of changes.
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
bool CallbackSetPropertyBitString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool* value, const uint32_t length, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode);
bool CallbackSetPropertyBool(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode);
bool CallbackSetPropertyCharString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const char* value, const uint32_t length, const uint8_t encodingType, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode);
bool CallbackSetPropertyDate(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint8_t year, const uint8_t month, const uint8_t day, const uint8_t weekday, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode);
bool CallbackSetPropertyDouble(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const double value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode);
bool CallbackSetPropertyEnum(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint32_t value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode);
bool CallbackSetPropertyNull(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode);
bool CallbackSetPropertyObjectIdentifier(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint16_t valueObjectType, const uint32_t valueObjectInstance, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode);
bool CallbackSetPropertyOctetString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint8_t* value, const uint32_t length, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode);
bool CallbackSetPropertyInt(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const int32_t value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode);
bool CallbackSetPropertyReal(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const float value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode);
bool CallbackSetPropertyTime(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint8_t hour, const uint8_t minute, const uint8_t second, const uint8_t hundrethSeconds, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode);
bool CallbackSetPropertyUInt(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint32_t value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode);
bool CallbackCreateObject(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance);
bool CallbackDeleteObject(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance);

bool CallbackReinitializeDevice(const uint32_t deviceInstance, const uint32_t reinitializedState, const char* password, const uint32_t passwordLength, uint32_t* errorCode);
bool CallbackDeviceCommunicationControl(const uint32_t deviceInstance, const uint8_t enableDisable, const char* password, const uint8_t passwordLength, const bool useTimeDuration, const uint16_t timeDuration, uint32_t* errorCode);
bool HookTextMessage(const uint32_t sourceDeviceIdentifier, const bool useMessageClass, const uint32_t messageClassUnsigned, const char* messageClassString, const uint32_t messageClassStringLength, const uint8_t messagePriority, const char* message, const uint32_t messageLength, const uint8_t* connectionString, const uint8_t connectionStringLength, const uint8_t networkType, const uint16_t sourceNetwork, const uint8_t* sourceAddress, const uint8_t sourceAddressLength, uint16_t* errorClass, uint16_t* errorCode);

// Helper functions 
void RegisterCallbacks();
bool SetupDevice();
bool SendIAm(uint8_t* connectionString, uint8_t connectionStringLength);
void WarmStart();
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
		g_exampleDatabase.device.instance = atoi(argv[1]); 
		std::cout << "FYI: Device instance= " << g_exampleDatabase.device.instance << std::endl;
	}
	else {
		std::cout << "FYI: Default to use device instance= " << g_exampleDatabase.device.instance << std::endl;
	}

	// Initialize global flags
	g_bbmdEnabled = false;
	g_warmStart = false;
	g_warmStartTimer = time(0);

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
	std::cout << "FYI: Connecting UDP Resource to port=["<< g_exampleDatabase.networkPort.BACnetIPUDPPort << "]... ";
	if (!g_udp.Connect(g_exampleDatabase.networkPort.BACnetIPUDPPort)) {
		std::cerr << "Failed to connect to UDP Resource" << std::endl ;
		std::cerr << "Press any key to exit the application..." << std::endl;
		(void) getchar();
		return -1;
	}
	std::cout << "OK, Connected to port" << std::endl;


	// 3. Setup the callbacks
	// ---------------------------------------------------------------------------
	RegisterCallbacks();

	// 4. Setup the BACnet device
	// ---------------------------------------------------------------------------
	if (!SetupDevice()) {
		return false;
	}

	// 5. Send I-Am of this device
	// ---------------------------------------------------------------------------
	// To be a good citizen on a BACnet network. We should announce ourself when we start up. 
	uint8_t connectionString[6];
	if (!SendIAm(connectionString, 6)) {
		return false;
	}

	// Broadcast BACnet stack version to the network via UnconfirmedTextMessage
	char stackVersionInfo[50];
	sprintf(stackVersionInfo, "CAS BACnet Stack v%u.%u.%u.%u", fpGetAPIMajorVersion(), fpGetAPIMinorVersion(), fpGetAPIPatchVersion(), fpGetAPIBuildVersion());
	if (!fpSendUnconfirmedTextMessage(g_exampleDatabase.device.instance, false, 0, NULL, 0, 0, stackVersionInfo, strlen(stackVersionInfo), connectionString, 6, CASBACnetStackExampleConstants::NETWORK_TYPE_IP, true, 65535, NULL, 0)) {
		std::cerr << "Unable to send UnconfirmedTextMessage broadcast" << std::endl;
		return false;
	}

	
	// 6. Start the main loop
	// ---------------------------------------------------------------------------
	std::cout << "FYI: Entering main loop..." << std::endl ;
	for (;;) {

		// Starts warm start reinitialization when requested (after 3 seconds).
		if (g_warmStart && g_warmStartTimer + 3 < time(0)) {
			WarmStart();
		}

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
		g_exampleDatabase.Loop();

		// Call Sleep to give some time back to the system
		Sleep(0); // Windows 
	}

	// All done. 
	return 0;
}

// Helper Functions

// Registers all the necessary callbacks in this function.
void RegisterCallbacks() {
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
	fpRegisterCallbackSetPropertyObjectIdentifier(CallbackSetPropertyObjectIdentifier);
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
}

// Sets up the device on the BACnet Stack. 
// Add all the functionality for enabling required services and adding objects and properties in this function.
bool SetupDevice() {
	std::cout << "Setting up server device. device.instance=[" << g_exampleDatabase.device.instance << "]" << std::endl;

	// Create the Device
	if (!fpAddDevice(g_exampleDatabase.device.instance)) {
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
	if (!fpSetServiceEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::SERVICE_I_AM, true)) {
		std::cerr << "Failed to enabled the IAm" << std::endl;
		return -1;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling ReadPropertyMultiple... ";
	if (!fpSetServiceEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::SERVICE_READ_PROPERTY_MULTIPLE, true)) {
		std::cerr << "Failed to enabled the ReadPropertyMultiple" << std::endl;
		return -1;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling WriteProperty... ";
	if (!fpSetServiceEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::SERVICE_WRITE_PROPERTY, true)) {
		std::cerr << "Failed to enable the WriteProperty service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling WritePropertyMultiple... ";
	if (!fpSetServiceEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::SERVICE_WRITE_PROPERTY_MULTIPLE, true)) {
		std::cerr << "Failed to enable the WritePropertyMultiple service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling TimeSynchronization... ";
	if (!fpSetServiceEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::SERVICE_TIME_SYNCHRONIZATION, true)) {
		std::cerr << "Failed to enable the TimeSynchronization service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling UTCTimeSynchronization... ";
	if (!fpSetServiceEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::SERVICE_UTC_TIME_SYNCHRONIZATION, true)) {
		std::cerr << "Failed to enable the UTCTimeSynchronization service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling SubscribeCOV... ";
	if (!fpSetServiceEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::SERVICE_SUBSCRIBE_COV, true)) {
		std::cerr << "Failed to enable the SubscribeCOV service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling SubscribeCOVProperty... ";
	if (!fpSetServiceEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::SERVICE_SUBSCRIBE_COV_PROPERTY, true)) {
		std::cerr << "Failed to enable the SubscribeCOVProperty service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling CreateObject... ";
	if (!fpSetServiceEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::SERVICE_CREATE_OBJECT, true)) {
		std::cerr << "Failed to enable the CreateObject service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling DeleteObject... ";
	if (!fpSetServiceEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::SERVICE_DELETE_OBJECT, true)) {
		std::cerr << "Failed to enable the DeleteObject service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling ReadRange... ";
	if (!fpSetServiceEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::SERVICE_READ_RANGE, true)) {
		std::cerr << "Failed to enable the ReadRange service" << std::endl;
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling ReinitializeDevice...";
	if (!fpSetServiceEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::SERVICE_REINITIALIZE_DEVICE, true)) {
		std::cerr << "Failed to enable the ReinitializeDevice service";
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling DeviceCommunicationControl...";
	if (!fpSetServiceEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::SERVICE_DEVICE_COMMUNICATION_CONTROL, true)) {
		std::cerr << "Failed to enable the DeviceCommunicationControl service";
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling UnconfirmedTextMessage...";
	if (!fpSetServiceEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::SERVICE_UNCONFIRMED_TEXT_MESSAGE, true)) {
		std::cerr << "Failed to enable the UnconfirmedTextMessage service";
		return false;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Enabling ConfirmedTextMessage...";
	if (!fpSetServiceEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::SERVICE_CONFIRMED_TEXT_MESSAGE, true)) {
		std::cerr << "Failed to enable the ConfirmedTextMessage service";
		return false;
	}
	std::cout << "OK" << std::endl;


	// Enable Optional Device Properties
	if (!fpSetPropertyEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE, g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_DESCRIPTION, true)) {
		std::cerr << "Failed to enable the description property for Device" << std::endl;
		return false;
	}

	// Update Writable Device Properties
	// UTC Offset
	if (!fpSetPropertyWritable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE, g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_UTC_OFFSET, true)) {
		std::cerr << "Failed to make the UTC Offset property writable for Device" << std::endl;
		return false;
	}
	// Description
	if (!fpSetPropertyWritable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE, g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_DESCRIPTION, true)) {
		std::cerr << "Failed to make the description property writable for Device" << std::endl;
		return false;
	}
	// Object Name
	if (!fpSetPropertyWritable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE, g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME, true)) {
		std::cerr << "Failed to make the object name property writable for Device" << std::endl;
		return false;
	}
	// Object Identifier
	if (!fpSetPropertyWritable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE, g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_IDENTIFIER, true)) {
		std::cerr << "Failed to make the object name property writable for Device" << std::endl;
		return false;
	}

	// Make some object creatable (optional)
	if (!fpSetObjectTypeSupported(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, true)) {
		std::cerr << "Failed to make Analog Values as supported object types in Device" << std::endl;
		return false;
	}
	if (!fpSetObjectTypeCreatable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, true)) {
		std::cerr << "Failed to make Analog Values as creatable object types in Device" << std::endl;
		return false;
	}


	// Add Objects
	// ---------------------------------------
	// AnalogInput (AI) 
	std::cout << "Adding AnalogInput. analogInput.instance=[" << g_exampleDatabase.analogInput.instance << "]... ";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_exampleDatabase.analogInput.instance)) {
		std::cerr << "Failed to add AnalogInput" << std::endl;
		return -1;
	}
	// Enable ProprietaryProperty for an object 
	// These properties are not part of the BACnet spec 
	fpSetProprietaryProperty(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_exampleDatabase.analogInput.instance, 512 + 1, false, false, CASBACnetStackExampleConstants::DATA_TYPE_CHARACTER_STRING, false, false, false);
	fpSetProprietaryProperty(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_exampleDatabase.analogInput.instance, 512 + 2, true, false, CASBACnetStackExampleConstants::DATA_TYPE_CHARACTER_STRING, false, false, false);
	fpSetProprietaryProperty(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_exampleDatabase.analogInput.instance, 512 + 3, true, true, CASBACnetStackExampleConstants::DATA_TYPE_CHARACTER_STRING, false, false, false);
	fpSetProprietaryProperty(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_exampleDatabase.analogInput.instance, 512 + 4, false, false, CASBACnetStackExampleConstants::DATA_TYPE_DATETIME, false, false, false);
	fpSetProprietaryProperty(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_exampleDatabase.analogInput.instance, 512 + 5, false, true, CASBACnetStackExampleConstants::DATA_TYPE_REAL, false, false, false);
	fpSetProprietaryProperty(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_exampleDatabase.analogInput.instance, 512 + 6, false, false, CASBACnetStackExampleConstants::DATA_TYPE_REAL, true, false, false);

	// Set the Present value to subscribable 
	fpSetPropertySubscribable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_exampleDatabase.analogInput.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	fpSetPropertyWritable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_exampleDatabase.analogInput.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_COV_INCURMENT, true);

	// Enable the description, and Reliability property 
	fpSetPropertyByObjectTypeEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_DESCRIPTION, true);
	fpSetPropertyByObjectTypeEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_RELIABILITY, true);

	// Enable a specific property to be subscribable for COVProperty 
	fpSetPropertySubscribable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_exampleDatabase.analogInput.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_RELIABILITY, true);
	std::cout << "OK" << std::endl;

	// AnalogOutput (AO) 
	std::cout << "Added AnalogOutput. analogOutput.instance=[" << g_exampleDatabase.analogOutput.instance << "]... ";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT, g_exampleDatabase.analogOutput.instance)) {
		std::cerr << "Failed to add AnalogOutput" << std::endl;
		return -1;
	}
	fpSetPropertyByObjectTypeEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_MIN_PRES_VALUE, true);
	fpSetPropertyByObjectTypeEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_MAX_PRES_VALUE, true);
	std::cout << "OK" << std::endl;

	// AnalogValue (AV) 
	std::cout << "Added AnalogValue. analogValue.instance=[" << g_exampleDatabase.analogValue.instance << "]... ";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, g_exampleDatabase.analogValue.instance)) {
		std::cerr << "Failed to add AnalogValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, g_exampleDatabase.analogValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	fpSetPropertySubscribable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, g_exampleDatabase.analogValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// BinaryInput (BI)
	std::cout << "Adding BinaryInput. binaryInput.instance=[" << g_exampleDatabase.binaryInput.instance << "]... ";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_INPUT, g_exampleDatabase.binaryInput.instance)) {
		std::cerr << "Failed to add BinaryInput" << std::endl;
		return -1;
	}
	std::cout << "OK" << std::endl;

	// BinaryOutput (BO)
	std::cout << "Added BinaryOutput. binaryOutput.instance=[" << g_exampleDatabase.binaryOutput.instance << "]... ";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_OUTPUT, g_exampleDatabase.binaryOutput.instance)) {
		std::cerr << "Failed to add BinaryOutput" << std::endl;
		return -1;
	}
	std::cout << "OK" << std::endl;

	// BinaryValue (BV)
	std::cout << "Added BinaryValue. binaryValue.instance=[" << g_exampleDatabase.binaryValue.instance << "]... ";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_VALUE, g_exampleDatabase.binaryValue.instance)) {
		std::cerr << "Failed to add BinaryValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_VALUE, g_exampleDatabase.binaryValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// MultiStateInput (MSI) 
	std::cout << "Added MultiStateInput. multiStateInput.instance=[" << g_exampleDatabase.multiStateInput.instance << "]... ";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT, g_exampleDatabase.multiStateInput.instance)) {
		std::cerr << "Failed to add MultiStateInput" << std::endl;
		return -1;
	}
	fpSetPropertyByObjectTypeEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_STATE_TEXT, true);
	std::cout << "OK" << std::endl;

	// MultiStateOutput (MSO)
	std::cout << "Added MultiStateOutput. multiStateOutput.instance=[" << g_exampleDatabase.multiStateOutput.instance << "]... ";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT, g_exampleDatabase.multiStateOutput.instance)) {
		std::cerr << "Failed to add MultiStateOutput" << std::endl;
		return -1;
	}
	fpSetPropertyByObjectTypeEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_STATE_TEXT, true);
	std::cout << "OK" << std::endl;

	// MultiStateValue (MSV)
	std::cout << "Added MultiStateValue. multiStateValue.instance=[" << g_exampleDatabase.multiStateValue.instance << "]... ";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE, g_exampleDatabase.multiStateValue.instance)) {
		std::cerr << "Failed to add MultiStateValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE, g_exampleDatabase.multiStateValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	fpSetPropertyByObjectTypeEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_STATE_TEXT, true);
	std::cout << "OK" << std::endl;

	// BitstringValue (BSV)
	std::cout << "Added BitstringValue. bitstringValue.instance=[" << g_exampleDatabase.bitstringValue.instance << "]...";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE, g_exampleDatabase.bitstringValue.instance)) {
		std::cerr << "Failed to add BitstringValue" << std::endl;
		return -1;
	}
	fpSetPropertyEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE, g_exampleDatabase.bitstringValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_BIT_TEXT, true);
	fpSetPropertyWritable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE, g_exampleDatabase.bitstringValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// characterStringValue (CSV)
	std::cout << "Added characterStringValue. characterStringValue.instance=[" << g_exampleDatabase.characterStringValue.instance << "]...";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_CHARACTERSTRING_VALUE, g_exampleDatabase.characterStringValue.instance)) {
		std::cerr << "Failed to add characterStringValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_CHARACTERSTRING_VALUE, g_exampleDatabase.characterStringValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// DateValue (DV)
	std::cout << "Added DateValue. dateValue.instance=[" << g_exampleDatabase.dateValue.instance << "]... ";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_DATE_VALUE, g_exampleDatabase.dateValue.instance)) {
		std::cerr << "Failed to add DateValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_DATE_VALUE, g_exampleDatabase.dateValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// IntegerValue (IV)
	std::cout << "Added IntegerValue. integerValue.instance=[" << g_exampleDatabase.integerValue.instance << "]... ";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_INTEGER_VALUE, g_exampleDatabase.integerValue.instance)) {
		std::cerr << "Failed to add IntegerValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_INTEGER_VALUE, g_exampleDatabase.integerValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// LargeAnalogValue (LAV)
	std::cout << "Added LargeAnalogValue. largeAnalogValue.instance=[" << g_exampleDatabase.largeAnalogValue.instance << "]... ";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_LARGE_ANALOG_VALUE, g_exampleDatabase.largeAnalogValue.instance)) {
		std::cerr << "Failed to add LargeAnalogValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_LARGE_ANALOG_VALUE, g_exampleDatabase.largeAnalogValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// octetStringValue (OSV)
	std::cout << "Added octetStringValue. octetStringValue.instance=[" << g_exampleDatabase.octetStringValue.instance << "]... ";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_OCTETSTRING_VALUE, g_exampleDatabase.octetStringValue.instance)) {
		std::cerr << "Failed to add octetStringValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_OCTETSTRING_VALUE, g_exampleDatabase.octetStringValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// PositiveIntegerValue (PIV)
	std::cout << "Added PositiveIntegerValue. positiveIntegerValue.instance=[" << g_exampleDatabase.positiveIntegerValue.instance << "]... ";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_POSITIVE_INTEGER_VALUE, g_exampleDatabase.positiveIntegerValue.instance)) {
		std::cerr << "Failed to add PositiveIntegerValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_POSITIVE_INTEGER_VALUE, g_exampleDatabase.positiveIntegerValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// TimeValue (TV)
	std::cout << "Added TimeValue. timeValue.instance=[" << g_exampleDatabase.timeValue.instance << "]... ";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_TIME_VALUE, g_exampleDatabase.timeValue.instance)) {
		std::cerr << "Failed to add TimeValue" << std::endl;
		return -1;
	}
	fpSetPropertyWritable(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_TIME_VALUE, g_exampleDatabase.timeValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, true);
	std::cout << "OK" << std::endl;

	// Add Trend Log Object
	std::cout << "Added TrendLog. trendLog.instance=[" << g_exampleDatabase.trendLog.instance << "]... ";
	if (!fpAddTrendLogObject(g_exampleDatabase.device.instance, g_exampleDatabase.trendLog.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_exampleDatabase.analogInput.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, CASBACnetStackExampleConstants::MAX_TREND_LOG_MAX_BUFFER_SIZE, false, 0)) {
		std::cerr << "Failed to add TrendLog" << std::endl;
		return -1;
	}

	// Setup TrendLog Object
	if (!fpSetTrendLogTypeToPolled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_TREND_LOG, g_exampleDatabase.trendLog.instance, true, false, 3000)) {
		std::cerr << "Failed to setup TrendLog to poll every 30 seconds";
		return -1;
	}
	std::cout << "OK" << std::endl;

	// Add Trend Log Multiple Object
	std::cout << "Added TrendLogMultiple. trendLogMultiple.instance=[" << g_exampleDatabase.trendLogMultiple.instance << "]... ";
	if (!fpAddTrendLogMultipleObject(g_exampleDatabase.device.instance, g_exampleDatabase.trendLogMultiple.instance, CASBACnetStackExampleConstants::MAX_TREND_LOG_MAX_BUFFER_SIZE)) {
		std::cerr << "Failed to add TrendLogMultiple" << std::endl;
		return -1;
	}

	// Setup TrendLogMultiple Object
	if (!fpAddLoggedObjectToTrendLogMultiple(g_exampleDatabase.device.instance, g_exampleDatabase.trendLogMultiple.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_exampleDatabase.analogInput.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0, false, 0)) {
		std::cerr << "Failed to add AnalogInput to be logged by TrendLogMultiple" << std::endl;
		return -1;
	}
	if (!fpAddLoggedObjectToTrendLogMultiple(g_exampleDatabase.device.instance, g_exampleDatabase.trendLogMultiple.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_INPUT, g_exampleDatabase.binaryInput.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE, false, 0, false, 0)) {
		std::cerr << "Failed to add BinaryInput to be logged by TrendLogMultiple" << std::endl;
		return -1;
	}
	if (!fpSetTrendLogTypeToPolled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_TREND_LOG_MULTIPLE, g_exampleDatabase.trendLogMultiple.instance, true, false, 3000)) {
		std::cerr << "Failed to setup TrendLogMultiple to poll every 30 seconds";
		return -1;
	}
	std::cout << "OK" << std::endl;

	// Add the Network Port Object
	std::cout << "Added NetworkPort. networkPort.instance=[" << g_exampleDatabase.networkPort.instance << "]... ";
	if (!fpAddNetworkPortObject(g_exampleDatabase.device.instance, g_exampleDatabase.networkPort.instance, CASBACnetStackExampleConstants::NETWORK_TYPE_IPV4, CASBACnetStackExampleConstants::PROTOCOL_LEVEL_BACNET_APPLICATION, CASBACnetStackExampleConstants::NETWORK_PORT_LOWEST_PROTOCOL_LAYER)) {
		std::cerr << "Failed to add NetworkPort" << std::endl;
		return -1;
	}

	uint8_t ipPortConcat[6];
	memcpy(ipPortConcat, g_exampleDatabase.networkPort.IPAddress, 4);
	ipPortConcat[4] = g_exampleDatabase.networkPort.BACnetIPUDPPort / 256;
	ipPortConcat[5] = g_exampleDatabase.networkPort.BACnetIPUDPPort % 256;
	fpAddBDTEntry(ipPortConcat, 6, g_exampleDatabase.networkPort.IPSubnetMask, 4);		// First BDT Entry must be server device
	std::cout << "OK" << std::endl;

	// Add the DateTimeValue Object
	std::cout << "Added DateTimeValue. dateTimeValue.instance=[" << g_exampleDatabase.dateTimeValue.instance << "]... ";
	if (!fpAddObject(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_DATETIME_VALUE, g_exampleDatabase.dateTimeValue.instance)) {
		std::cerr << "Failed to add DateTimeValue" << std::endl;
		return -1;
	}

	std::cout << "OK" << std::endl;

	// Debug. Print the current IP address of this device incase there are muliple network cards on the PC that is using the 
	// Example. This is not required, its just for debug 
	std::cout << "FYI: NetworkPort.IPAddress: " << (int)g_exampleDatabase.networkPort.IPAddress[0] << "." << (int)g_exampleDatabase.networkPort.IPAddress[1] << "." << (int)g_exampleDatabase.networkPort.IPAddress[2] << "." << (int)g_exampleDatabase.networkPort.IPAddress[3] << std::endl;
	std::cout << "FYI: NetworkPort.IPSubnetMask: " << (int)g_exampleDatabase.networkPort.IPSubnetMask[0] << "." << (int)g_exampleDatabase.networkPort.IPSubnetMask[1] << "." << (int)g_exampleDatabase.networkPort.IPSubnetMask[2] << "." << (int)g_exampleDatabase.networkPort.IPSubnetMask[3] << std::endl;
	std::cout << "FYI: NetworkPort.BroadcastIPAddress: " << (int)g_exampleDatabase.networkPort.BroadcastIPAddress[0] << "." << (int)g_exampleDatabase.networkPort.BroadcastIPAddress[1] << "." << (int)g_exampleDatabase.networkPort.BroadcastIPAddress[2] << "." << (int)g_exampleDatabase.networkPort.BroadcastIPAddress[3] << std::endl;
	return true;
}

// Sends an I-Am broadcast.
bool SendIAm(uint8_t* connectionString, uint8_t connectionStringLength) {
	if (connectionStringLength < 6) {
		std::cerr << "Connection String array too small" << std::endl;
		return false;
	}
	
	std::cout << "FYI: Sending I-AM broadcast" << std::endl;
	 //= { 0xC0, 0xA8, 0x01, 0xFF, 0xBA, 0xC0 };
	memcpy(connectionString, g_exampleDatabase.networkPort.BroadcastIPAddress, 4);
	connectionString[4] = g_exampleDatabase.networkPort.BACnetIPUDPPort / 256;
	connectionString[5] = g_exampleDatabase.networkPort.BACnetIPUDPPort % 256;

	if (!fpSendIAm(g_exampleDatabase.device.instance, connectionString, 6, CASBACnetStackExampleConstants::NETWORK_TYPE_IP, true, 65535, NULL, 0)) {
		std::cerr << "Unable to send IAm broadcast" << std::endl;
		return false;
	}
	return true;
}

// Resets the flags and warm starts the device. Sends an I-Am broadcast to notify its existence on the network.
void WarmStart() {
	std::cout << "FYI: Warm Start Initiating..." << std::endl;
	g_warmStart = false;
	g_warmStartTimer = time(0);
	fpReset();
	RegisterCallbacks();
	SetupDevice();
	uint8_t connectionString[6];
	SendIAm(connectionString, 6);
}

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

		if (!g_bbmdEnabled) {
			// BBMD Properties of the Network Port Object, only enable if another BBMD is added to the BDT table
			fpSetPropertyEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT, g_exampleDatabase.networkPort.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_BBMD_ACCEPT_FD_REGISTRATIONS, true);
			fpSetPropertyEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT, g_exampleDatabase.networkPort.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_BBMD_BROADCAST_DISTRIBUTION_TABLE, true);
			fpSetPropertyEnabled(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT, g_exampleDatabase.networkPort.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_BBMD_FOREIGN_DEVICE_TABLE, true);
			fpSetBBMD(g_exampleDatabase.device.instance, g_exampleDatabase.networkPort.instance);

			g_bbmdEnabled = true;
		}

		break;
	}
	case 'i': {
		// Increment the Analog Value
		g_exampleDatabase.analogValue.presentValue += 1.1f;
		std::cout << "Incrementing Analog Value to " << g_exampleDatabase.analogValue.presentValue << std::endl;

		// Notify the stack that this data point was updated so the stack can check for logic
		// that may need to run on the data.  Example: check if COV (change of value) occurred.
		if (fpValueUpdated != NULL) {
			fpValueUpdated(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE, g_exampleDatabase.analogValue.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE);
		}
		break;
	}
	case 'r': {
		// Toggle the Analog Input reliability status
		// no-fault-detected (0), unreliable-other (7)
		if (g_exampleDatabase.analogInput.reliability == 0) {
			g_exampleDatabase.analogInput.reliability = 7; // unreliable-other (7)
		}
		else {
			g_exampleDatabase.analogInput.reliability = 0; //no-fault-detected (0)
		}
		std::cout << "Toggle the Analog Input reliability status to " << g_exampleDatabase.analogInput.reliability << std::endl;

		// Notify the stack that this data point was updated so the stack can check for logic
		// that may need to run on the data. Example: Check if COVProperty (change of value) occurred.
		if (fpValueUpdated != NULL) {
			fpValueUpdated(g_exampleDatabase.device.instance, CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_exampleDatabase.analogInput.instance, CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_RELIABILITY);
		}
		break;
	}
	case 'f': {
		// Send Foreign Device Registration
		
		uint8_t connectionString[6];
		memcpy(connectionString, g_exampleDatabase.networkPort.FdBbmdAddressHostIp, 4);
		connectionString[4] = g_exampleDatabase.networkPort.FdBbmdAddressPort / 256;
		connectionString[5] = g_exampleDatabase.networkPort.FdBbmdAddressPort % 256;
		std::cout << "Sending Register Foreign Device to " << static_cast<uint16_t>(g_exampleDatabase.networkPort.FdBbmdAddressHostIp[0]) << "." <<
			static_cast<uint16_t>(g_exampleDatabase.networkPort.FdBbmdAddressHostIp[1]) << "." << static_cast<uint16_t>(g_exampleDatabase.networkPort.FdBbmdAddressHostIp[2]) << "." <<
			static_cast<uint16_t>(g_exampleDatabase.networkPort.FdBbmdAddressHostIp[3]) << ":" << g_exampleDatabase.networkPort.FdBbmdAddressPort << std::endl;

		if (!fpSendRegisterForeignDevice(g_exampleDatabase.networkPort.FdSubscriptionLifetime, connectionString, 6)) {
			std::cout << "Error - failed to send Register Foreign Device" << std::endl;
		}
		break;
	}
	case 'm': {
		// Send text message
		uint8_t connectionString[6];
		memcpy(connectionString, g_exampleDatabase.networkPort.FdBbmdAddressHostIp, 4);
		connectionString[4] = g_exampleDatabase.networkPort.FdBbmdAddressPort / 256;
		connectionString[5] = g_exampleDatabase.networkPort.FdBbmdAddressPort % 256;

		uint8_t message[24] = "This is a test message.";
		std::cout << "Sending text message to " << static_cast<uint16_t>(g_exampleDatabase.networkPort.FdBbmdAddressHostIp[0]) << "." <<
			static_cast<uint16_t>(g_exampleDatabase.networkPort.FdBbmdAddressHostIp[1]) << "." << static_cast<uint16_t>(g_exampleDatabase.networkPort.FdBbmdAddressHostIp[2]) << "." <<
			static_cast<uint16_t>(g_exampleDatabase.networkPort.FdBbmdAddressHostIp[3]) << ":" << g_exampleDatabase.networkPort.FdBbmdAddressPort << std::endl;

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
		std::cout << "i - (i)ncrement Analog Value: " << g_exampleDatabase.analogValue.instance << " by 1.1" << std::endl;
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
		if (fpDecodeAsJSON((char*)message, bytesRead, jsonRenderBuffer, MAX_RENDER_BUFFER_LENGTH, CASBACnetStackExampleConstants::NETWORK_TYPE_IP) > 0) {
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
			connectionString[0] | ~g_exampleDatabase.networkPort.IPSubnetMask[0],
			connectionString[1] | ~g_exampleDatabase.networkPort.IPSubnetMask[1],
			connectionString[2] | ~g_exampleDatabase.networkPort.IPSubnetMask[2],
			connectionString[3] | ~g_exampleDatabase.networkPort.IPSubnetMask[3]);
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
	if (fpDecodeAsJSON((char*)message, messageLength, jsonRenderBuffer, MAX_RENDER_BUFFER_LENGTH, networkType) > 0) {
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
	return time(0) - g_exampleDatabase.device.currentTimeOffset;
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
	g_exampleDatabase.device.currentTimeOffset = time(0) - mktime(&timeInfo);
	return true;
}

// Callback used by the BACnet Stack to get Bitstring property values from the user
bool CallbackGetPropertyBitString(uint32_t deviceInstance, uint16_t objectType, uint32_t objectInstance, uint32_t propertyIdentifier, bool* value, uint32_t* valueElementCount, uint32_t maxElementCount, bool useArrayIndex, uint32_t propertyArrayIndex)
{
	// Example of Bitstring Value Object Present Value property
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE && objectInstance == g_exampleDatabase.bitstringValue.instance) {
			if (g_exampleDatabase.bitstringValue.presentValue.size() > maxElementCount) {
				return false;
			}
			else {
				uint32_t valueArrayLength = 0;
				for (std::vector<bool>::const_iterator itr = g_exampleDatabase.bitstringValue.presentValue.begin(); itr != g_exampleDatabase.bitstringValue.presentValue.end(); itr++) {
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
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT && objectInstance == g_exampleDatabase.analogOutput.instance) {
				if (propertyArrayIndex <= CASBACnetStackExampleConstants::MAX_BACNET_PRIORITY) {
					*value = g_exampleDatabase.analogOutput.priorityArrayNulls[propertyArrayIndex - 1];
					return true;
				}
				else {
					return false; // property array index out of range
				}
			}
			else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_OUTPUT && objectInstance == g_exampleDatabase.binaryOutput.instance) {
				if (propertyArrayIndex <= CASBACnetStackExampleConstants::MAX_BACNET_PRIORITY) {
					*value = g_exampleDatabase.binaryOutput.priorityArrayNulls[propertyArrayIndex - 1];
					return true;
				}
				else {
					return false; // property array index out of range
				}
			}
			else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT && objectInstance == g_exampleDatabase.multiStateOutput.instance) {
				if (propertyArrayIndex <= CASBACnetStackExampleConstants::MAX_BACNET_PRIORITY) {
					*value = g_exampleDatabase.multiStateOutput.priorityArrayNulls[propertyArrayIndex - 1];
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
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && objectInstance == g_exampleDatabase.device.instance) {
			time_t currentTime = time(0);
			struct tm* timeinfo = localtime(&currentTime);
			*value = (timeinfo->tm_isdst != 0);
			return true;
		}
	}
	// Network Port Object - Changes Pending property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_CHANGES_PENDING) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_exampleDatabase.networkPort.instance) {
			*value = g_exampleDatabase.networkPort.ChangesPending;
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
		if (g_exampleDatabase.device.description.size() <= maxElementCount) {
			memcpy(value, g_exampleDatabase.device.description.c_str(), g_exampleDatabase.device.description.size());
			*valueElementCount = (uint32_t)g_exampleDatabase.device.description.size();
			return true;
		}
		return false;
	}
	// Example of Character String Value Object Present Value property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_CHARACTERSTRING_VALUE && objectInstance == g_exampleDatabase.characterStringValue.instance) {
			if (g_exampleDatabase.characterStringValue.presentValue.size() <= maxElementCount) {
				memcpy(value, g_exampleDatabase.characterStringValue.presentValue.c_str(), g_exampleDatabase.characterStringValue.presentValue.size());
				*valueElementCount = (uint32_t)g_exampleDatabase.characterStringValue.presentValue.size();
				return true;
			}
		}
		return false;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE && propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_BIT_TEXT && objectInstance == g_exampleDatabase.bitstringValue.instance && useArrayIndex) {
		if (propertyArrayIndex <= g_exampleDatabase.bitstringValue.presentValue.size()) {
			memcpy(value, g_exampleDatabase.bitstringValue.bitText[propertyArrayIndex - 1].c_str(), g_exampleDatabase.bitstringValue.bitText[propertyArrayIndex - 1].size());
			*valueElementCount = (uint32_t)g_exampleDatabase.bitstringValue.bitText[propertyArrayIndex - 1].size();
			return true;
		}
		return false;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_DESCRIPTION && objectInstance == g_exampleDatabase.analogInput.instance) {
		if (g_exampleDatabase.analogInput.description.size() <= maxElementCount) {
			memcpy(value, g_exampleDatabase.analogInput.description.c_str(), g_exampleDatabase.analogInput.description.size());
			*valueElementCount = (uint32_t)g_exampleDatabase.analogInput.description.size();
			return true;
		}
		return false;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_APPLICATION_SOFTWARE_VERSION)
	{
		*valueElementCount = snprintf(value, maxElementCount, APPLICATION_VERSION.c_str());
		return true;
	}
	

	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && propertyIdentifier == 512 + 1 && objectInstance == g_exampleDatabase.analogInput.instance)
	{
		*valueElementCount = snprintf( value, maxElementCount, "Example custom property 512 + 1");
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && propertyIdentifier == 512 + 2 && objectInstance == g_exampleDatabase.analogInput.instance)
	{
		*valueElementCount = snprintf(value, maxElementCount, "Example custom property 512 + 2");
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && propertyIdentifier == 512 + 3 && objectInstance == g_exampleDatabase.analogInput.instance)
	{
		*valueElementCount = snprintf(value, maxElementCount, "Example custom property 512 + 3");
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT && propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_STATE_TEXT && objectInstance == g_exampleDatabase.multiStateInput.instance) {
		if (useArrayIndex && propertyArrayIndex > 0 && propertyArrayIndex <= g_exampleDatabase.multiStateInput.stateText.size()) {
			// 0 is number of dates. 
			*valueElementCount = snprintf(value, maxElementCount, g_exampleDatabase.multiStateInput.stateText[propertyArrayIndex - 1].c_str());
			return true;
		}
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT && propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_STATE_TEXT && objectInstance == g_exampleDatabase.multiStateOutput.instance) {
		if (useArrayIndex && propertyArrayIndex > 0 && propertyArrayIndex <= g_exampleDatabase.multiStateOutput.stateText.size()) {
			// 0 is number of dates. 
			*valueElementCount = snprintf(value, maxElementCount, g_exampleDatabase.multiStateOutput.stateText[propertyArrayIndex - 1].c_str());
			return true;
		}
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE && propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_STATE_TEXT && objectInstance == g_exampleDatabase.multiStateValue.instance) {
		if (useArrayIndex && propertyArrayIndex > 0 && propertyArrayIndex <= g_exampleDatabase.multiStateValue.stateText.size()) {
			// 0 is number of dates. 
			*valueElementCount = snprintf(value, maxElementCount, g_exampleDatabase.multiStateValue.stateText[propertyArrayIndex - 1].c_str());
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
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DATE_VALUE && objectInstance == g_exampleDatabase.dateValue.instance) {
			*year = g_exampleDatabase.dateValue.presentValueYear;
			*month = g_exampleDatabase.dateValue.presentValueMonth;
			*day = g_exampleDatabase.dateValue.presentValueDay;
			*weekday = g_exampleDatabase.dateValue.presentValueWeekday;
			return true;
		}
	}
	// Example of getting Device Local Date property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_LOCAL_DATE) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && objectInstance == g_exampleDatabase.device.instance) {
			time_t adjustedTime = time(0) - g_exampleDatabase.device.currentTimeOffset;
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
			*year = g_exampleDatabase.dateTimeValue.presentValueYear;
			*month = g_exampleDatabase.dateTimeValue.presentValueMonth;
			*day = g_exampleDatabase.dateTimeValue.presentValueDay;
			*weekday = g_exampleDatabase.dateTimeValue.presentValueWeekDay;
			return true;
			}
	}
	// Example of getting Analog Input object Date Time Proprietary property
	if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_exampleDatabase.analogInput.instance) {
		if (propertyIdentifier == 512 + 4) {
			*year = g_exampleDatabase.analogInput.proprietaryYear;
			*month = g_exampleDatabase.analogInput.proprietaryMonth;
			*day = g_exampleDatabase.analogInput.proprietaryDay;
			*weekday = g_exampleDatabase.analogInput.proprietaryWeekDay;
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
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_LARGE_ANALOG_VALUE && objectInstance == g_exampleDatabase.largeAnalogValue.instance) {
			*value = g_exampleDatabase.largeAnalogValue.presentValue;
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
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_INPUT && objectInstance == g_exampleDatabase.binaryInput.instance) {
			*value = g_exampleDatabase.binaryInput.presentValue;
			return true;
		}
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_VALUE && objectInstance == g_exampleDatabase.binaryValue.instance) {
			*value = g_exampleDatabase.binaryValue.presentValue;
			return true;
		}
	}
	// Example of Binary Output Priority Array property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRIORITY_ARRAY) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_OUTPUT && objectInstance == g_exampleDatabase.binaryOutput.instance) {
			if (useArrayIndex) {
				if (propertyArrayIndex <= CASBACnetStackExampleConstants::MAX_BACNET_PRIORITY) {
					*value = g_exampleDatabase.binaryOutput.priorityArrayValues[propertyArrayIndex - 1];
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
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_exampleDatabase.analogInput.instance) {
			*value = g_exampleDatabase.analogInput.reliability; 
			return true;
		}
	}
	// Network Port Object - FdBbmdAddress Host Type
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_FD_BBMD_ADDRESS) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_exampleDatabase.networkPort.instance) {
			*value = g_exampleDatabase.networkPort.FdBbmdAddressHostType;
			return true;
		}
	}
	
	// Debug for customer 
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_SYSTEM_STATUS &&
		objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE)
	{
		std::cout << "Debug: Device:System Status" << std::endl;
		*value = g_exampleDatabase.device.systemStatus;
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
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_OCTETSTRING_VALUE && objectInstance == g_exampleDatabase.octetStringValue.instance) {
			if (g_exampleDatabase.octetStringValue.presentValue.size() > maxElementCount) {
				return false;
			}
			else {

				uint32_t valueLength = 0;
				for (std::vector<uint8_t>::const_iterator itr = g_exampleDatabase.octetStringValue.presentValue.begin(); itr != g_exampleDatabase.octetStringValue.presentValue.end(); itr++) {
					*(value + valueLength) = (*itr);
					valueLength++;
				}
				*valueElementCount = valueLength;

				// memcpy(value, g_exampleDatabase.octetStringValuePresentValue, g_exampleDatabase.octetStringValue.presentValue.size());
				// *valueElementCount = g_exampleDatabase.octetStringValue.presentValue.size();
				return true;
			}
		}
	}
	// Example of Network Port Object IP Address property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_IP_ADDRESS) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_exampleDatabase.networkPort.instance) {
			memcpy(value, g_exampleDatabase.networkPort.IPAddress, g_exampleDatabase.networkPort.IPAddressLength);
			*valueElementCount = g_exampleDatabase.networkPort.IPAddressLength;
			return true;
		}
	}
	// Example of Network Port Object IP Default Gateway property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_IP_DEFAULT_GATEWAY) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_exampleDatabase.networkPort.instance) {
			memcpy(value, g_exampleDatabase.networkPort.IPDefaultGateway, g_exampleDatabase.networkPort.IPDefaultGatewayLength);
			*valueElementCount = g_exampleDatabase.networkPort.IPDefaultGatewayLength;
			return true;
		}
	}
	// Example of Network Port Object IP Subnet Mask property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_IP_SUBNET_MASK) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_exampleDatabase.networkPort.instance) {
			memcpy(value, g_exampleDatabase.networkPort.IPSubnetMask, g_exampleDatabase.networkPort.IPSubnetMaskLength);
			*valueElementCount = g_exampleDatabase.networkPort.IPSubnetMaskLength;
			return true;
		}
	}
	// Example of Network Port Object IP DNS Server property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_IP_DNS_SERVER) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_exampleDatabase.networkPort.instance) {
			// The IP DNS Server property is an array of DNS Server addresses
			if (useArrayIndex) {
				if (propertyArrayIndex != 0 && propertyArrayIndex <= g_exampleDatabase.networkPort.IPDNSServers.size()) {
					memcpy(value, g_exampleDatabase.networkPort.IPDNSServers[propertyArrayIndex - 1], g_exampleDatabase.networkPort.IPDNSServerLength);
					*valueElementCount = g_exampleDatabase.networkPort.IPDNSServerLength;
					return true;
				}
			}
		}
	}
	// Network Port Object FdBbmdAddress Host (as IP Address)
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_FD_BBMD_ADDRESS) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_exampleDatabase.networkPort.instance) {
			if (useArrayIndex && propertyArrayIndex == CASBACnetStackExampleConstants::HOST_TYPE_IPADDRESS) {
				memcpy(value, g_exampleDatabase.networkPort.FdBbmdAddressHostIp, 4);
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
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_INTEGER_VALUE && objectInstance == g_exampleDatabase.integerValue.instance) {
			*value = g_exampleDatabase.integerValue.presentValue;
			return true;
		}
	}
	// Example of Device UTC Offset property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_UTC_OFFSET) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && objectInstance == g_exampleDatabase.device.instance) {
			*value = g_exampleDatabase.device.UTCOffset;
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
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_exampleDatabase.analogInput.instance) {
			*value = g_exampleDatabase.analogInput.presentValue;
			return true;
		}
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && objectInstance == g_exampleDatabase.analogValue.instance) {
			*value = g_exampleDatabase.analogValue.presentValue;
			return true;
		}
		// Check if this is for a created analog value
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && g_exampleDatabase.CreatedAnalogValueData.count(objectInstance) > 0) {
			*value = g_exampleDatabase.CreatedAnalogValueData[objectInstance].value;
			return true;
		}
	}
	// Example of Analog Output Priority Array property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRIORITY_ARRAY) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT && objectInstance == g_exampleDatabase.analogOutput.instance) {
			if (useArrayIndex) {
				if (propertyArrayIndex <= CASBACnetStackExampleConstants::MAX_BACNET_PRIORITY) {
					*value = g_exampleDatabase.analogOutput.priorityArrayValues[propertyArrayIndex - 1];
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
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_COV_INCURMENT && objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_exampleDatabase.analogInput.instance) {
		*value = g_exampleDatabase.analogInput.covIncurment;
		return true;
	}
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_MAX_PRES_VALUE && objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && objectInstance == g_exampleDatabase.analogValue.instance) {
		*value = g_exampleDatabase.analogValue.maxPresValue;
		return true;
	}
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_MIN_PRES_VALUE && objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && objectInstance == g_exampleDatabase.analogValue.instance) {
		*value = g_exampleDatabase.analogValue.minPresValue;
		return true;
	}
	else if (propertyIdentifier == 512 + 5 && objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_exampleDatabase.analogInput.instance) {
		*value = g_exampleDatabase.analogInput.proprietaryReal;
		return true;
	}
	// Example of Proprietary Array of Real primitives
	else if (propertyIdentifier == 512 + 6 && objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_exampleDatabase.analogInput.instance && useArrayIndex) {
		*value = g_exampleDatabase.analogInput.proprietaryArrayOfReal[propertyArrayIndex - 1];
		return true;
	}

	return false;
}

// Callback used by the BACnet Stack to get Time property values from the user
bool CallbackGetPropertyTime(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, uint8_t* hour, uint8_t* minute, uint8_t* second, uint8_t* hundrethSeconds, const bool useArrayIndex, const uint32_t propertyArrayIndex)
{
	// Example of getting Time Value Object Present Value property
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_TIME_VALUE && objectInstance == g_exampleDatabase.timeValue.instance) {
			*hour = g_exampleDatabase.timeValue.presentValueHour;
			*minute = g_exampleDatabase.timeValue.presentValueMinute;
			*second = g_exampleDatabase.timeValue.presentValueSecond;
			*hundrethSeconds = g_exampleDatabase.timeValue.presentValueHundrethSecond;
			return true;
		}
	}
	// Example of getting Device Local Time property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_LOCAL_TIME) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && objectInstance == g_exampleDatabase.device.instance) {
			time_t adjustedTime = time(0) - g_exampleDatabase.device.currentTimeOffset;
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
			*hour = g_exampleDatabase.dateTimeValue.presentValueHour;
			*minute = g_exampleDatabase.dateTimeValue.presentValueMinute;
			*second = g_exampleDatabase.dateTimeValue.presentValueSecond;
			*hundrethSeconds = g_exampleDatabase.dateTimeValue.presentValueHundredthSeconds;
			return true;
			}
	}
	// Example of getting Analog Input object DateTime Proprietary property
	if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_exampleDatabase.analogInput.instance) {
		if (propertyIdentifier == 512 + 4) {
			*hour = g_exampleDatabase.analogInput.proprietaryHour;
			*minute = g_exampleDatabase.analogInput.proprietaryMinute;
			*second = g_exampleDatabase.analogInput.proprietarySecond;
			*hundrethSeconds = g_exampleDatabase.analogInput.proprietaryHundredthSeconds;
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
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_POSITIVE_INTEGER_VALUE && objectInstance == g_exampleDatabase.positiveIntegerValue.instance) {
			*value = g_exampleDatabase.positiveIntegerValue.presentValue;
			return true;
		}
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT && objectInstance == g_exampleDatabase.multiStateInput.instance) {
			*value = g_exampleDatabase.multiStateInput.presentValue;
			return true;
		}
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE && objectInstance == g_exampleDatabase.multiStateValue.instance) {
			*value = g_exampleDatabase.multiStateValue.presentValue;
			return true;
		}
	}
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_RELINQUISH_DEFAULT) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT && objectInstance == g_exampleDatabase.multiStateOutput.instance) {
			*value = g_exampleDatabase.multiStateOutput.relinquishDefault;
			return true;
		}
	}


	// Example of Multi-State Output Priority Array property
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRIORITY_ARRAY) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT && objectInstance == g_exampleDatabase.multiStateOutput.instance) {
			if (useArrayIndex) {
				if (propertyArrayIndex <= CASBACnetStackExampleConstants::MAX_BACNET_PRIORITY) {
					*value = g_exampleDatabase.multiStateOutput.priorityArrayValues[propertyArrayIndex - 1];
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
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_exampleDatabase.networkPort.instance) {
			*value = g_exampleDatabase.networkPort.BACnetIPUDPPort;
			return true;
		}
	}
	// Example of Network Port Object IP DNS Server Array Size property
	// Any properties that are an array must have an entry here for the array size.
	// The array size is provided only if the useArrayIndex parameter is set to true and the propertyArrayIndex is zero.
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_IP_DNS_SERVER) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_exampleDatabase.networkPort.instance) {
			if (useArrayIndex && propertyArrayIndex == 0) {
				*value = (uint32_t)g_exampleDatabase.networkPort.IPDNSServers.size();
				return true;
			}
		}
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE && propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_BIT_TEXT) {
		if (objectInstance == g_exampleDatabase.bitstringValue.instance && useArrayIndex && propertyArrayIndex == 0) {
			*value = (uint32_t)g_exampleDatabase.bitstringValue.presentValue.size();
			return true;
		}
	}
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_NUMBER_OF_STATES) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT && objectInstance == g_exampleDatabase.multiStateInput.instance) {
			*value = g_exampleDatabase.multiStateInput.stateText.size();
			return true;
		}
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT && objectInstance == g_exampleDatabase.multiStateOutput.instance) {
			*value = g_exampleDatabase.multiStateOutput.stateText.size();
			return true;
		}
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE && objectInstance == g_exampleDatabase.multiStateValue.instance) {
			*value = g_exampleDatabase.multiStateValue.stateText.size();
			return true;
		}
	}
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_STATE_TEXT && useArrayIndex && propertyArrayIndex == 0) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT && objectInstance == g_exampleDatabase.multiStateInput.instance) {
			*value = g_exampleDatabase.multiStateInput.stateText.size();
			return true;
		}
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT && objectInstance == g_exampleDatabase.multiStateOutput.instance) {
			*value = g_exampleDatabase.multiStateOutput.stateText.size();
			return true;
		}
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE && objectInstance == g_exampleDatabase.multiStateValue.instance) {
			*value = g_exampleDatabase.multiStateValue.stateText.size();
			return true;
		}

	}
	// Network Port Object FdBbmdAddress Port
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_FD_BBMD_ADDRESS) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_exampleDatabase.networkPort.instance) {
			if (useArrayIndex && propertyArrayIndex == CASBACnetStackExampleConstants::FD_BBMD_ADDRESS_PORT) {
				// Check for index 2, which is looking for the fdBbmdAddress port portion
				*value = g_exampleDatabase.networkPort.FdBbmdAddressPort;
				return true;
			}
		}
	}
	// Network Port Object FdSubscriptionLifetime
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_FD_SUBSCRIPTION_LIFETIME) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_exampleDatabase.networkPort.instance) {
			*value = g_exampleDatabase.networkPort.FdSubscriptionLifetime;
			return true;
		}
	}
	// Example of Customer Property that is an array. This returns the size of the array
	else if (propertyIdentifier == 512 + 6) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_exampleDatabase.analogInput.instance) {
			*value = g_exampleDatabase.analogInput.proprietaryArrayOfReal.size();
			return true;
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set Bitstring property values to the user
bool CallbackSetPropertyBitString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool* value, const uint32_t length, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode)
{
	if (deviceInstance == g_exampleDatabase.device.instance) {
		// Example of writing to Bitstring Value Object Present Value property
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE && objectInstance == g_exampleDatabase.bitstringValue.instance) {
				if (length > g_exampleDatabase.bitstringValue.presentValue.size()) {
					*errorCode = CASBACnetStackExampleConstants::ERROR_NO_SPACE_TO_WRITE_PROPERTY;
					return false;
				}
				else {
					g_exampleDatabase.bitstringValue.Resize(length);
					for (uint32_t offset = 0; offset < length; offset++) {
						g_exampleDatabase.bitstringValue.SetPresentValue(offset, *(value + offset));
					}
					return true;
				}
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set Boolean property values to the user
bool CallbackSetPropertyBool(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const bool value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode)
{
	return false;
}

// Callback used by the BACnet Stack to set Charstring property values to the user
bool CallbackSetPropertyCharString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const char* value, const uint32_t length, const uint8_t encodingType, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode)
{
	if (deviceInstance == g_exampleDatabase.device.instance) {
		// Example of setting Charstring Value Object Present Value property
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_CHARACTERSTRING_VALUE && objectInstance == g_exampleDatabase.characterStringValue.instance) {
				g_exampleDatabase.characterStringValue.presentValue = std::string(value, length);
				return true;
			}
			return false;
		}
		// Example of setting description property of the Device
		else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_DESCRIPTION) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && objectInstance == g_exampleDatabase.device.instance) {
				g_exampleDatabase.device.description = std::string(value, length);
				return true;
			}
			return false;
		}
		// Example of setting object name property of the Device
		else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && objectInstance == g_exampleDatabase.device.instance) {
				g_exampleDatabase.device.objectName = std::string(value, length);
				return true;
			}
			return false;
		}
		// Check if trying to set the object name of an analog value that was created.
		// Used in initializing objects
		else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_NAME && objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && g_exampleDatabase.CreatedAnalogValueData.count(objectInstance) > 0) {
			g_exampleDatabase.CreatedAnalogValueData[objectInstance].name = std::string(value, length);
			return true;
		}
	}

	return false;
}

// Callback used by the BACnet Stack to set Date property values to the user
bool CallbackSetPropertyDate(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint8_t year, const uint8_t month, const uint8_t day, const uint8_t weekday, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode)
{
	if (deviceInstance == g_exampleDatabase.device.instance) {
		// Example of setting Date Value Present Value property
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DATE_VALUE && objectInstance == g_exampleDatabase.dateValue.instance) {
				g_exampleDatabase.dateValue.presentValueYear = year;
				g_exampleDatabase.dateValue.presentValueMonth = month;
				g_exampleDatabase.dateValue.presentValueDay = day;
				g_exampleDatabase.dateValue.presentValueWeekday = weekday;
				return true;
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set Double property values to the user
bool CallbackSetPropertyDouble(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const double value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode)
{
	if (deviceInstance == g_exampleDatabase.device.instance) {
		// Example of setting Large Analog Value Object Present Value property
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_LARGE_ANALOG_VALUE && objectInstance == g_exampleDatabase.largeAnalogValue.instance) {
				g_exampleDatabase.largeAnalogValue.presentValue = value;
				return true;
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set Enumerated property values to the user
bool CallbackSetPropertyEnum(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint32_t value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode)
{
	if (deviceInstance == g_exampleDatabase.device.instance) {
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			// Example of setting Binary Value Present Value property
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_VALUE && objectInstance == g_exampleDatabase.binaryValue.instance) {
				g_exampleDatabase.binaryValue.presentValue = value;
				return true;
			}
			// Example of setting Binary Output Present Value / Priority Array property
			else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_OUTPUT && objectInstance == g_exampleDatabase.binaryOutput.instance) {
				g_exampleDatabase.binaryOutput.priorityArrayValues[priority - 1] = value;
				g_exampleDatabase.binaryOutput.priorityArrayNulls[priority - 1] = false;
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
	if (deviceInstance == g_exampleDatabase.device.instance) {
		// Examples of setting Analog, Binary, and Multi-State Outputs Present Value / Priority Array property to Null
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT && objectInstance == g_exampleDatabase.analogOutput.instance) {
				g_exampleDatabase.analogOutput.priorityArrayValues[priority - 1] = 0.0f;
				g_exampleDatabase.analogOutput.priorityArrayNulls[priority - 1] = true;
				return true;
			}
			else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_OUTPUT && objectInstance == g_exampleDatabase.binaryOutput.instance) {
				g_exampleDatabase.binaryOutput.priorityArrayValues[priority - 1] = 0;
				g_exampleDatabase.binaryOutput.priorityArrayNulls[priority - 1] = true;
				return true;
			}
			else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT && objectInstance == g_exampleDatabase.multiStateOutput.instance) {
				g_exampleDatabase.multiStateOutput.priorityArrayValues[priority - 1] = 0;
				g_exampleDatabase.multiStateOutput.priorityArrayNulls[priority - 1] = true;
				return true;
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set OctetString property values to the user
bool CallbackSetPropertyOctetString(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint8_t* value, const uint32_t length, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode)
{
	if (deviceInstance == g_exampleDatabase.device.instance) {
		// Example of setting Octet String Value Object Present Value property
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_OCTETSTRING_VALUE && objectInstance == g_exampleDatabase.octetStringValue.instance) {
				if (length > g_exampleDatabase.octetStringValue.presentValue.size()) {
					*errorCode = CASBACnetStackExampleConstants::ERROR_NO_SPACE_TO_WRITE_PROPERTY;
					return false;
				}
				else {
					g_exampleDatabase.octetStringValue.presentValue.resize(length);
					for (uint32_t offset = 0; offset < length; offset++) {
						g_exampleDatabase.octetStringValue.presentValue[offset] = *(value + offset);
					}
					return true;
				}
			}
		}

		// Example of setting FdBbmdAddress Host IP
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_FD_BBMD_ADDRESS) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_exampleDatabase.networkPort.instance) {
				if (useArrayIndex && propertyArrayIndex == CASBACnetStackExampleConstants::FD_BBMD_ADDRESS_HOST) {
					if (length > 4) {
						*errorCode = CASBACnetStackExampleConstants::ERROR_VALUE_OUT_OF_RANGE;
						return false;
					}
					if (memcmp(g_exampleDatabase.networkPort.FdBbmdAddressHostIp, value, length) == 0) {
						// No change, return true
						return true;
					}
					else {
						// Store new value and set changes pending to true
						memcpy(g_exampleDatabase.networkPort.FdBbmdAddressHostIp, value, length);
						g_exampleDatabase.networkPort.ChangesPending = true;
						return true;
					}
				}
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set Object Identifier property values to the user
bool CallbackSetPropertyObjectIdentifier(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint16_t valueObjectType, const uint32_t valueObjectInstance, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode)
{
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_OBJECT_IDENTIFIER &&
		objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE &&
		objectInstance == g_exampleDatabase.device.instance)
	{

		// This value needs to be saved to the EEprom
		g_exampleDatabase.device.instance = valueObjectInstance;
		std::cout << "Database: " << g_exampleDatabase.device.instance << std::endl;
		return true;
	}
	return false;
}

// Callback used by the BACnet Stack to set Integer property values to the user
bool CallbackSetPropertyInt(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const int32_t value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode)
{
	if (deviceInstance == g_exampleDatabase.device.instance) {
		// Example of setting Integer Value Object Present Value property
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_INTEGER_VALUE && objectInstance == g_exampleDatabase.integerValue.instance) {
				g_exampleDatabase.integerValue.presentValue = value;
				return true;
			}
		}
		// Example of setting Device UTC Offset property
		else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_UTC_OFFSET) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && objectInstance == g_exampleDatabase.device.instance) {
				if (value < -1440 || value > 1440) {
					*errorCode = CASBACnetStackExampleConstants::ERROR_VALUE_OUT_OF_RANGE;
					return false;
				}
				else {
					g_exampleDatabase.device.UTCOffset = value;
					return true;
				}
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set Real property values to the user
bool CallbackSetPropertyReal(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const float value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode)
{
	if (deviceInstance != g_exampleDatabase.device.instance) {
		return false; // Not this device.
	}
	if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
		// Example of setting Analog Value Present Value Property
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && objectInstance == g_exampleDatabase.analogValue.instance) {
			if (value < g_exampleDatabase.analogValue.minPresValue || value > g_exampleDatabase.analogValue.maxPresValue) {
				*errorCode = CASBACnetStackExampleConstants::ERROR_VALUE_OUT_OF_RANGE;
				return false;
			}
			g_exampleDatabase.analogValue.presentValue = value;
			return true;
		}
		// Example of setting Analog Output Present Value / Priority Array property
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT && objectInstance == g_exampleDatabase.analogOutput.instance) {
			g_exampleDatabase.analogOutput.priorityArrayValues[priority - 1] = value;
			g_exampleDatabase.analogOutput.priorityArrayNulls[priority - 1] = false;
			return true;
		}
		// Check if setting present value of a create analog value
		else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && g_exampleDatabase.CreatedAnalogValueData.count(objectInstance) > 0) {
			g_exampleDatabase.CreatedAnalogValueData[objectInstance].value = value;
			return true;
		}
	}
	else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_COV_INCURMENT) {
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_exampleDatabase.analogInput.instance) {
			g_exampleDatabase.analogInput.covIncurment = value;
			return true;
		}
	}
	else if (propertyIdentifier == 512 + 5 && objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_exampleDatabase.analogInput.instance) {
		g_exampleDatabase.analogInput.proprietaryReal = value;
		return true;
	}
	return false;
}

// Callback used by the BACnet Stack to set Time property values to the user
bool CallbackSetPropertyTime(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint8_t hour, const uint8_t minute, const uint8_t second, const uint8_t hundrethSeconds, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode)
{
	if (deviceInstance == g_exampleDatabase.device.instance) {
		// Example of setting Time Value Object Present Value property
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_TIME_VALUE && objectInstance == g_exampleDatabase.timeValue.instance) {
				g_exampleDatabase.timeValue.presentValueHour = hour;
				g_exampleDatabase.timeValue.presentValueMinute = minute;
				g_exampleDatabase.timeValue.presentValueSecond = second;
				g_exampleDatabase.timeValue.presentValueHundrethSecond = hundrethSeconds;
				return true;
			}
		}
	}
	return false;
}

// Callback used by the BACnet Stack to set Date property values to the user
bool CallbackSetPropertyUInt(const uint32_t deviceInstance, const uint16_t objectType, const uint32_t objectInstance, const uint32_t propertyIdentifier, const uint32_t value, const bool useArrayIndex, const uint32_t propertyArrayIndex, const uint8_t priority, uint32_t* errorCode)
{
	if (deviceInstance == g_exampleDatabase.device.instance) {
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_PRESENT_VALUE) {
			// Example of setting Positive Integer Value Object Present Value property
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_POSITIVE_INTEGER_VALUE && objectInstance == g_exampleDatabase.positiveIntegerValue.instance) {
				g_exampleDatabase.positiveIntegerValue.presentValue = value;
				return true;
			}
			// Example of setting Multi-State Value Object Present Value property
			else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE && objectInstance == g_exampleDatabase.multiStateValue.instance) {
				g_exampleDatabase.multiStateValue.presentValue = value;
				return true;
			}
			// Example of setting Multi-State Output Present Value / Priority Array property
			else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT && objectInstance == g_exampleDatabase.multiStateOutput.instance) {
				g_exampleDatabase.multiStateOutput.priorityArrayValues[priority - 1] = value;
				g_exampleDatabase.multiStateOutput.priorityArrayNulls[priority - 1] = false;
				return true;
			}
		}
		// Network Port Object FdBbmdAddress Port
		if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_FD_BBMD_ADDRESS) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_exampleDatabase.networkPort.instance) {
				if (useArrayIndex && propertyArrayIndex == CASBACnetStackExampleConstants::FD_BBMD_ADDRESS_PORT) {
					g_exampleDatabase.networkPort.FdBbmdAddressPort = value;
					g_exampleDatabase.networkPort.ChangesPending = true;
					return true;
				}
			}
		}
		// Network Port Object FdSubscriptionLifetime
		else if (propertyIdentifier == CASBACnetStackExampleConstants::PROPERTY_IDENTIFIER_FD_SUBSCRIPTION_LIFETIME) {
			if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_exampleDatabase.networkPort.instance) {
				g_exampleDatabase.networkPort.FdSubscriptionLifetime = value;
				g_exampleDatabase.networkPort.ChangesPending = true;
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
	if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DEVICE && objectInstance == g_exampleDatabase.device.instance) {
		stringSize = g_exampleDatabase.device.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance <<" ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.device.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t)stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_INPUT && objectInstance == g_exampleDatabase.analogInput.instance) {
		stringSize = g_exampleDatabase.analogInput.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.analogInput.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_OUTPUT && objectInstance == g_exampleDatabase.analogOutput.instance) {
		stringSize = g_exampleDatabase.analogOutput.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.analogOutput.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t)stringSize;
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
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && objectInstance == g_exampleDatabase.analogValue.instance) {
		stringSize = g_exampleDatabase.analogValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.analogValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_INPUT && objectInstance == g_exampleDatabase.binaryInput.instance) {
		stringSize = g_exampleDatabase.binaryInput.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.binaryInput.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_OUTPUT && objectInstance == g_exampleDatabase.binaryOutput.instance) {
		stringSize = g_exampleDatabase.binaryOutput.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.binaryOutput.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BINARY_VALUE && objectInstance == g_exampleDatabase.binaryValue.instance) {
		stringSize = g_exampleDatabase.binaryValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.binaryValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_INPUT && objectInstance == g_exampleDatabase.multiStateInput.instance) {
		stringSize = g_exampleDatabase.multiStateInput.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.multiStateInput.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_OUTPUT && objectInstance == g_exampleDatabase.multiStateOutput.instance) {
		stringSize = g_exampleDatabase.multiStateOutput.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.multiStateOutput.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_MULTI_STATE_VALUE && objectInstance == g_exampleDatabase.multiStateValue.instance) {
		stringSize = g_exampleDatabase.multiStateValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.multiStateValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_TREND_LOG && objectInstance == g_exampleDatabase.trendLog.instance) {
		stringSize = g_exampleDatabase.trendLog.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.trendLog.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_TREND_LOG_MULTIPLE && objectInstance == g_exampleDatabase.trendLogMultiple.instance) {
	stringSize = g_exampleDatabase.trendLogMultiple.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << "]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.trendLogMultiple.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t)stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_BITSTRING_VALUE && objectInstance == g_exampleDatabase.bitstringValue.instance) {
		stringSize = g_exampleDatabase.bitstringValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.bitstringValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_CHARACTERSTRING_VALUE && objectInstance == g_exampleDatabase.characterStringValue.instance) {
		stringSize = g_exampleDatabase.characterStringValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.characterStringValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DATE_VALUE && objectInstance == g_exampleDatabase.dateValue.instance) {
		stringSize = g_exampleDatabase.dateValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance << " ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.dateValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_INTEGER_VALUE && objectInstance == g_exampleDatabase.integerValue.instance) {
		stringSize = g_exampleDatabase.integerValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance <<" ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.integerValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t)stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_LARGE_ANALOG_VALUE && objectInstance == g_exampleDatabase.largeAnalogValue.instance) {
		stringSize = g_exampleDatabase.largeAnalogValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance <<" ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.largeAnalogValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_OCTETSTRING_VALUE && objectInstance == g_exampleDatabase.octetStringValue.instance) {
		stringSize = g_exampleDatabase.octetStringValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance <<" ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.octetStringValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_POSITIVE_INTEGER_VALUE && objectInstance == g_exampleDatabase.positiveIntegerValue.instance) {
		stringSize = g_exampleDatabase.positiveIntegerValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance <<" ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.positiveIntegerValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_TIME_VALUE && objectInstance == g_exampleDatabase.timeValue.instance) {
		stringSize = g_exampleDatabase.timeValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance <<" ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.timeValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_NETWORK_PORT && objectInstance == g_exampleDatabase.networkPort.instance) {
		stringSize = g_exampleDatabase.networkPort.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance <<" ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.networkPort.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_DATETIME_VALUE && objectInstance == g_exampleDatabase.dateTimeValue.instance) {
		stringSize = g_exampleDatabase.dateTimeValue.objectName.size();
		if (stringSize > maxElementCount) {
			std::cerr << "Error - not enough space to store full name of objectType=[" << objectType << "], objectInstance=[" << objectInstance <<" ]" << std::endl;
			return false;
		}
		memcpy(value, g_exampleDatabase.dateTimeValue.objectName.c_str(), stringSize);
		*valueElementCount = (uint32_t) stringSize;
		return true;
	}
	else {
		// Check if the value is an Analog Value and check if it was a created object
		if (objectType == CASBACnetStackExampleConstants::OBJECT_TYPE_ANALOG_VALUE && g_exampleDatabase.CreatedAnalogValueData.count(objectInstance) > 0) {
			std::string objectName = g_exampleDatabase.CreatedAnalogValueData[objectInstance].name;
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
		g_exampleDatabase.CreatedAnalogValueData.insert(std::pair<uint32_t, CreatedAnalogValue>(objectInstance, CreatedAnalogValue()));
		g_exampleDatabase.CreatedAnalogValueData[objectInstance].name = std::string("AnalogValue_") + ChipkinCommon::ChipkinConvert::ToString(objectInstance);
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
		if (g_exampleDatabase.CreatedAnalogValueData.count(objectInstance) > 0) {
			g_exampleDatabase.CreatedAnalogValueData.erase(objectInstance);
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
	//		If password invalid, missing, or incorrect: set errorCode to PasswordInvalid (26)
	// In this example, a password of 12345 is required.
	
	if (password == NULL || passwordLength == 0) {
		*errorCode = CASBACnetStackExampleConstants::ERROR_PASSWORD_FAILURE;
		return false;
	}

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
		g_exampleDatabase.networkPort.ChangesPending = false;
		return true;
	}
	else if (reinitializedState == CASBACnetStackExampleConstants::REINITIALIZED_STATE_WARM_START) {
		// Flag for reboot and handle reboot after stack responds with SimpleAck.
		g_warmStart = true;
		g_exampleDatabase.networkPort.ChangesPending = false;
		g_warmStartTimer = time(0);
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
	//		If password invalid, missing, or incorrect: set errorCode to PasswordInvalid (26)
	// In this example, a password of 12345 is required.
	if (password == NULL || passwordLength == 0) {
		*errorCode = CASBACnetStackExampleConstants::ERROR_PASSWORD_FAILURE;
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

