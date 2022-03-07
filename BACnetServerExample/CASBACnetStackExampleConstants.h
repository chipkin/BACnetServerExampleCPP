/*
 * BACnet Server Example C++ 
 * ----------------------------------------------------------------------------
 * CASBACnetStackExampleConstants.h
 * 
 * This is a fully static class that contains all the constants used by the example.
 * Includes Object Types, Property Identifiers, etc.
 * 
 * Created by: Steven Smethurst
*/

#ifndef __CASBACnetStackExampleConstants_h__
#define __CASBACnetStackExampleConstants_h__

#include "datatypes.h"

class CASBACnetStackExampleConstants {

public:

	// CAS BACnet Stack network type
	static const uint8_t NETWORK_TYPE_IP = 0;
	static const uint8_t NETWORK_TYPE_MSTP = 1;


	// General Constants
	static const uint32_t NETWORK_PORT_LOWEST_PROTOCOL_LAYER = 4194303;

	// Object Types
	static const uint16_t OBJECT_TYPE_ANALOG_INPUT = 0;
	static const uint16_t OBJECT_TYPE_ANALOG_OUTPUT = 1;
	static const uint16_t OBJECT_TYPE_ANALOG_VALUE = 2;
	static const uint16_t OBJECT_TYPE_BINARY_INPUT = 3;
	static const uint16_t OBJECT_TYPE_BINARY_OUTPUT = 4;
	static const uint16_t OBJECT_TYPE_BINARY_VALUE = 5;
	//static const uint16_t OBJECT_TYPE_CALENDAR = 6;
	//static const uint16_t OBJECT_TYPE_COMMAND = 7;
	static const uint16_t OBJECT_TYPE_DEVICE = 8;
	//static const uint16_t OBJECT_TYPE_EVENT_ENROLLMENT = 9;
	//static const uint16_t OBJECT_TYPE_FILE = 10;
	//static const uint16_t OBJECT_TYPE_GROUP = 11;
	//static const uint16_t OBJECT_TYPE_LOOP = 12;
	static const uint16_t OBJECT_TYPE_MULTI_STATE_INPUT = 13;
	static const uint16_t OBJECT_TYPE_MULTI_STATE_OUTPUT = 14;
	//static const uint16_t OBJECT_TYPE_NOTIFICATION_CLASS = 15;
	//static const uint16_t OBJECT_TYPE_PROGRAM = 16;
	//static const uint16_t OBJECT_TYPE_SCHEDULE = 17;
	//static const uint16_t OBJECT_TYPE_AVERAGING = 18;
	static const uint16_t OBJECT_TYPE_MULTI_STATE_VALUE = 19;
	static const uint16_t OBJECT_TYPE_TREND_LOG = 20;
	//static const uint16_t OBJECT_TYPE_LIFE_SAFETY_POINT = 21;
	//static const uint16_t OBJECT_TYPE_LIFE_SAFETY_ZONE = 22;
	//static const uint16_t OBJECT_TYPE_ACCUMULATOR = 23;
	//static const uint16_t OBJECT_TYPE_PULSE_CONVERTER = 24;
	//static const uint16_t OBJECT_TYPE_EVENT_LOG = 25;
	//static const uint16_t OBJECT_TYPE_GLOBAL_GROUP = 26;
	static const uint16_t OBJECT_TYPE_TREND_LOG_MULTIPLE = 27;
	//static const uint16_t OBJECT_TYPE_LOAD_CONTROL = 28;
	//static const uint16_t OBJECT_TYPE_STRUCTURED_VIEW = 29;
	//static const uint16_t OBJECT_TYPE_ACCESS_DOOR = 30;
	//static const uint16_t OBJECT_TYPE_TIMER = 31;
	//static const uint16_t OBJECT_TYPE_ACCESS_CREDENTIAL = 32;
	//static const uint16_t OBJECT_TYPE_ACCESS_POINT = 33;
	//static const uint16_t OBJECT_TYPE_ACCESS_RIGHTS = 34;
	//static const uint16_t OBJECT_TYPE_ACCESS_USER = 35;
	//static const uint16_t OBJECT_TYPE_ACCESS_ZONE = 36;
	//static const uint16_t OBJECT_TYPE_CREDENTIAL_DATA_INPUT = 37;
	//static const uint16_t OBJECT_TYPE_NETWORK_SECURITY = 38;
	static const uint16_t OBJECT_TYPE_BITSTRING_VALUE = 39;
	static const uint16_t OBJECT_TYPE_CHARACTERSTRING_VALUE = 40;
	//static const uint16_t OBJECT_TYPE_DATEPATTERN_VALUE = 41;
	static const uint16_t OBJECT_TYPE_DATE_VALUE = 42;
	//static const uint16_t OBJECT_TYPE_DATETIMEPATTERN_VALUE = 43;
	static const uint16_t OBJECT_TYPE_DATETIME_VALUE = 44;
	static const uint16_t OBJECT_TYPE_INTEGER_VALUE = 45;
	static const uint16_t OBJECT_TYPE_LARGE_ANALOG_VALUE = 46;
	static const uint16_t OBJECT_TYPE_OCTETSTRING_VALUE = 47;
	static const uint16_t OBJECT_TYPE_POSITIVE_INTEGER_VALUE = 48;
	//static const uint16_t OBJECT_TYPE_TIMEPATTERN_VALUE = 49;
	static const uint16_t OBJECT_TYPE_TIME_VALUE = 50;
	//static const uint16_t OBJECT_TYPE_NOTIFICATION_FORWARDER = 51;
	//static const uint16_t OBJECT_TYPE_ALERT_ENROLLMENT = 52;
	//static const uint16_t OBJECT_TYPE_CHANNEL = 53;
	//static const uint16_t OBJECT_TYPE_LIGHTING_OUTPUT = 54;
	//static const uint16_t OBJECT_TYPE_BINARY_LIGHTING_OUTPUT = 55;
	static const uint16_t OBJECT_TYPE_NETWORK_PORT = 56;
	//static const uint16_t OBJECT_TYPE_ELEVATOR_GROUP = 57;
	//static const uint16_t OBJECT_TYPE_ESCALATOR = 58;
	//static const uint16_t OBJECT_TYPE_LIFT = 59;

	// Property Identifiers
	static const uint32_t PROPERTY_IDENTIFIER_ALL = 8;
	static const uint32_t PROPERTY_IDENTIFIER_COV_INCURMENT = 22;
	static const uint32_t PROPERTY_IDENTIFIER_DAY_LIGHT_SAVINGS_STATUS = 24;
	static const uint32_t PROPERTY_IDENTIFIER_DESCRIPTION = 28;
	static const uint32_t PROPERTY_IDENTIFIER_LOCAL_DATE = 56;
	static const uint32_t PROPERTY_IDENTIFIER_LOCAL_TIME = 57;
	static const uint32_t PROPERTY_IDENTIFIER_NUMBER_OF_STATES = 74;
	static const uint32_t PROPERTY_IDENTIFIER_OBJECT_NAME = 77;	
	static const uint32_t PROPERTY_IDENTIFIER_PRESENT_VALUE = 85;
	static const uint32_t PROPERTY_IDENTIFIER_PRIORITY_ARRAY = 87;
	static const uint32_t PROPERTY_IDENTIFIER_RELIABILITY = 103;
	static const uint32_t PROPERTY_IDENTIFIER_STATE_TEXT = 110;
	static const uint32_t PROPERTY_IDENTIFIER_STATUS_FLAGS = 111;
	static const uint32_t PROPERTY_IDENTIFIER_SYSTEM_STATUS = 112;
	static const uint32_t PROPERTY_IDENTIFIER_UTC_OFFSET = 119;
	static const uint32_t PROPERTY_IDENTIFIER_BIT_TEXT = 343;
	

	static const uint32_t PROPERTY_IDENTIFIER_MAX_PRES_VALUE = 65;
	static const uint32_t PROPERTY_IDENTIFIER_MIN_PRES_VALUE = 69;

	// Network Port Property Identifiers
	static const uint32_t PROPERTY_IDENTIFIER_IP_ADDRESS = 400;
	static const uint32_t PROPERTY_IDENTIFIER_IP_DEFAULT_GATEWAY = 401;
	static const uint32_t PROPERTY_IDENTIFIER_IP_DNS_SERVER = 406;
	static const uint32_t PROPERTY_IDENTIFIER_IP_SUBNET_MASK = 411;
	static const uint32_t PROPERTY_IDENTIFIER_BACNET_IP_UDP_PORT = 412;
	static const uint32_t PROPERTY_IDENTIFIER_BBMD_ACCEPT_FD_REGISTRATIONS = 413;
	static const uint32_t PROPERTY_IDENTIFIER_BBMD_BROADCAST_DISTRIBUTION_TABLE = 414;
	static const uint32_t PROPERTY_IDENTIFIER_BBMD_FOREIGN_DEVICE_TABLE = 415;
	static const uint32_t PROPERTY_IDENTIFIER_CHANGES_PENDING = 416;
	static const uint32_t PROPERTY_IDENTIFIER_FD_BBMD_ADDRESS = 418;
	static const uint32_t PROPERTY_IDENTIFIER_FD_SUBSCRIPTION_LIFETIME = 419;
	static const uint32_t PROPERTY_IDENTIFIER_LINK_SPEED = 420;
	static const uint32_t PROPERTY_IDENTIFIER_MAC_ADDRESS = 423;
	
	// Services Supported
	static const uint8_t SERVICE_SUBSCRIBE_COV = 5;
	static const uint8_t SERVICE_CREATE_OBJECT = 10;
	static const uint8_t SERVICE_DELETE_OBJECT = 11;
	static const uint8_t SERVICE_READ_PROPERTY_MULTIPLE = 14;
	static const uint8_t SERVICE_WRITE_PROPERTY = 15;
	static const uint8_t SERVICE_WRITE_PROPERTY_MULTIPLE = 16;
	static const uint8_t SERVICE_DEVICE_COMMUNICATION_CONTROL = 17;
	static const uint8_t SERVICE_CONFIRMED_TEXT_MESSAGE = 19;
	static const uint8_t SERVICE_REINITIALIZE_DEVICE = 20;
	static const uint8_t SERVICE_I_AM = 26;
	static const uint8_t SERVICE_I_HAVE = 27;
	static const uint8_t SERVICE_UNCONFIRMED_TEXT_MESSAGE = 31;
	static const uint8_t SERVICE_TIME_SYNCHRONIZATION = 32;
	static const uint8_t SERVICE_READ_RANGE = 35;
	static const uint8_t SERVICE_UTC_TIME_SYNCHRONIZATION = 36;
	static const uint8_t SERVICE_SUBSCRIBE_COV_PROPERTY = 38;
	

	// Error Codes
	static const uint8_t ERROR_MISSING_REQUIRED_PARAMETER = 16;
	static const uint8_t ERROR_NO_SPACE_TO_WRITE_PROPERTY = 20;
	static const uint8_t ERROR_PASSWORD_FAILURE = 26;
	static const uint8_t ERROR_VALUE_OUT_OF_RANGE = 37;
	static const uint8_t ERROR_OPTIONAL_FUNCTIONALITY_NOT_SUPPORTED = 45;
	static const uint8_t ERROR_INVALID_CONFIGURATION_DATA = 46;

	// Network Port FdBBmdAddressHostType
	static const uint8_t HOST_TYPE_NONE = 0;
	static const uint8_t HOST_TYPE_IPADDRESS = 1;
	static const uint8_t HOST_TYPE_NAME = 2;

	// Network Port FdBbmdAddressOffset
	static const uint8_t FD_BBMD_ADDRESS_HOST = 1;
	static const uint8_t FD_BBMD_ADDRESS_PORT = 2;

	// Network Type
	static const uint8_t NETWORK_TYPE_BACNET_IP = 0;
	static const uint8_t NETWORK_TYPE_IPV4 = 5;

	// Protocol Level
	static const uint8_t PROTOCOL_LEVEL_BACNET_APPLICATION = 2;

	// Priority Array
	static const uint8_t MAX_BACNET_PRIORITY = 16;

	// Trend Log Buffer Size
	static const uint32_t MAX_TREND_LOG_MAX_BUFFER_SIZE = 100;

	// Data types 
	static const uint32_t DATA_TYPE_NULL = 0;
	static const uint32_t DATA_TYPE_BOOLEAN = 1;
	static const uint32_t DATA_TYPE_UNSIGNED_INTEGER = 2;
	static const uint32_t DATA_TYPE_SIGNED_INTEGER = 3;
	static const uint32_t DATA_TYPE_REAL = 4;
	static const uint32_t DATA_TYPE_DOUBLE = 5;
	static const uint32_t DATA_TYPE_OCTET_STRING = 6;
	static const uint32_t DATA_TYPE_CHARACTER_STRING = 7;
	static const uint32_t DATA_TYPE_BIT_STRING = 8;
	static const uint32_t DATA_TYPE_ENUMERATED = 9;
	static const uint32_t DATA_TYPE_DATE = 10;
	static const uint32_t DATA_TYPE_TIME = 11;
	static const uint32_t DATA_TYPE_BACNET_OBJECT_IDENTIFIER = 12;
	static const uint32_t DATA_TYPE_DATETIME = 27;

	// Reinitialized State
	static const uint8_t REINITIALIZED_STATE_WARM_START = 1;
	static const uint8_t REINITIALIZED_STATE_ACTIVATE_CHANGES = 7;

	// Debug Message Type
	static const uint8_t BACNET_DEBUG_LOG_TYPE_ERROR = 0;
	static const uint8_t BACNET_DEBUG_LOG_TYPE_INFO = 1;
};

#endif // __CASBACnetStackExampleConstants_h__
