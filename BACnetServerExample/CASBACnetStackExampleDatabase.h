/*
 * BACnet Server Example C++
 * ----------------------------------------------------------------------------
 * CASBACnetStackExampleDatabase.h
 * 
 * The CASBACnetStackExampleDatabase is a data store that contains 
 * some example data used in the BACnetStackDLLExample.
 * This data is represented by BACnet objects for this server example.
 * There will be one object of each type currently supported by the CASBACnetStack.
 * 
 * The database will include the following:
 *	- present value
 *	- name
 *	- for outputs priority array (bool and value)
 * 
 * Created by: Steven Smethurst
*/

#ifndef __CASBACnetStackExampleDatabase_h__
#define __CASBACnetStackExampleDatabase_h__

#include <string>
#include <vector>
#include <stdint.h>
#include <string.h>
#include <map>

// Base class for all object types. 
class ExampleDatabaseBaseObject
{
	public:
		// Const
		static const uint8_t PRIORITY_ARRAY_LENGTH = 16 ; 

		// All objects will have the following properties 
		std::string objectName ; 
		uint32_t instance ; 
};

class ExampleDatabaseAnalogInput : public ExampleDatabaseBaseObject 
{
	public:
		float presentValue ;
		float covIncurment; 
		uint32_t reliability;
		std::string description; // This is an optional property that has been enabled.  

		// DateTime Proprietary Value
		uint8_t proprietaryYear;
		uint8_t proprietaryMonth;
		uint8_t proprietaryDay;
		uint8_t proprietaryWeekDay;
		uint8_t proprietaryHour;
		uint8_t proprietaryMinute;
		uint8_t proprietarySecond;
		uint8_t proprietaryHundredthSeconds;

		// Proprietary Real
		float proprietaryReal;

		// Proprietary Array of Reals
		std::vector<float> proprietaryArrayOfReal;
};

class ExampleDatabaseAnalogOutput : public ExampleDatabaseBaseObject 
{
	public:
		bool priorityArrayNulls[ExampleDatabaseBaseObject::PRIORITY_ARRAY_LENGTH] ;
		float priorityArrayValues[ExampleDatabaseBaseObject::PRIORITY_ARRAY_LENGTH] ;

		ExampleDatabaseAnalogOutput() {
			memset(this->priorityArrayValues, 0, sizeof(float) * ExampleDatabaseBaseObject::PRIORITY_ARRAY_LENGTH);
			memset(this->priorityArrayNulls, true, sizeof(bool) * ExampleDatabaseBaseObject::PRIORITY_ARRAY_LENGTH);
		}
};

class ExampleDatabaseAnalogValue : public ExampleDatabaseBaseObject 
{
	public:
		float presentValue;		
		float maxPresValue; // This is an optional property 
		float minPresValue; // This is an optional property 
};

class ExampleDatabaseBinaryInput : public ExampleDatabaseBaseObject 
{
	public:
		bool presentValue ;
		std::string description ; // This is an optional property that has been enabled.   
};

class ExampleDatabaseBinaryOutput : public ExampleDatabaseBaseObject 
{
	public:
		bool priorityArrayNulls[ExampleDatabaseBaseObject::PRIORITY_ARRAY_LENGTH] ;
		bool priorityArrayValues[ExampleDatabaseBaseObject::PRIORITY_ARRAY_LENGTH] ;

		ExampleDatabaseBinaryOutput() {
			memset(this->priorityArrayValues, 0, sizeof(bool) * ExampleDatabaseBaseObject::PRIORITY_ARRAY_LENGTH);
			memset(this->priorityArrayNulls, true, sizeof(bool) * ExampleDatabaseBaseObject::PRIORITY_ARRAY_LENGTH);
		}
};

class ExampleDatabaseBinaryValue : public ExampleDatabaseBaseObject 
{
	public:
		bool presentValue ;
};

class ExampleDatabaseDevice : public ExampleDatabaseBaseObject 
{
	public:
		int UTCOffset;
		int64_t currentTimeOffset;
		std::string description;
		uint32_t systemStatus;
};

class ExampleDatabaseMultiStateInput : public ExampleDatabaseBaseObject 
{
	public:
		uint32_t presentValue ;
		uint32_t numberOfStates;
		std::vector<std::string> stateText; 

		ExampleDatabaseMultiStateInput() {
			this->presentValue = 1 ; // A value of zero is invalid.
			this->numberOfStates = 3;

			this->stateText.push_back("One");
			this->stateText.push_back("Two");
			this->stateText.push_back("Three");
		}
};

class ExampleDatabaseMultiStateOutput : public ExampleDatabaseBaseObject 
{
	public:
		bool priorityArrayNulls[ExampleDatabaseBaseObject::PRIORITY_ARRAY_LENGTH] ;
		uint32_t priorityArrayValues[ExampleDatabaseBaseObject::PRIORITY_ARRAY_LENGTH] ;
		uint32_t numberOfStates;

		ExampleDatabaseMultiStateOutput() {
			memset(this->priorityArrayValues, 1, sizeof(uint32_t) * ExampleDatabaseBaseObject::PRIORITY_ARRAY_LENGTH);
			memset(this->priorityArrayNulls, true, sizeof(bool) * ExampleDatabaseBaseObject::PRIORITY_ARRAY_LENGTH);
			this->numberOfStates = 5;
		}
};

class ExampleDatabaseMultiStateValue : public ExampleDatabaseBaseObject 
{
	public:
		uint32_t presentValue ;
		uint32_t numberOfStates;

		ExampleDatabaseMultiStateValue() {
			this->presentValue = 1 ;  // A value of zero is invalid.
			this->numberOfStates = 4;
		}
};

class ExampleDatabaseTrendLog : public ExampleDatabaseBaseObject 
{
	public:
};

class ExampleDatabaseTrendLogMultiple : public ExampleDatabaseBaseObject
{
public:
};

class ExampleDatabaseBitstringValue : public ExampleDatabaseBaseObject 
{
	public:
		std::vector<bool> presentValue;
		std::vector<std::string> bitText;

		bool Resize( size_t count) ; 
		bool SetPresentValue(size_t offset, bool value);
		bool SetBitText(size_t offset, std::string bitText);
};

class ExampleDatabaseCharacterStringValue : public ExampleDatabaseBaseObject 
{
	public:
		std::string presentValue;		
};

class ExampleDatabaseDateValue : public ExampleDatabaseBaseObject 
{
	public:
		uint8_t presentValueYear;
		uint8_t presentValueMonth;
		uint8_t presentValueDay;
		uint8_t presentValueWeekday;

		void Set(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday ); 
};

class ExampleDatabaseIntegerValue : public ExampleDatabaseBaseObject 
{
	public:
		int32_t presentValue;
};

class ExampleDatabaseLargeAnalogValue : public ExampleDatabaseBaseObject 
{
	public:
		double presentValue;
};

class ExampleDatabaseOctetStringValue : public ExampleDatabaseBaseObject 
{
	public:
		std::vector<uint8_t> presentValue;
};

class ExampleDatabasePositiveIntegerValue : public ExampleDatabaseBaseObject 
{
	public:
		uint32_t presentValue;
};

class ExampleDatabaseTimeValue : public ExampleDatabaseBaseObject 
{
	public:
		uint8_t presentValueHour;
		uint8_t presentValueMinute;
		uint8_t presentValueSecond;
		uint8_t presentValueHundrethSecond;

		void Set( uint8_t hour, uint8_t minute, uint8_t second, uint8_t hundrethSecond);
};

class ExampleDatabaseNetworkPort : public ExampleDatabaseBaseObject 
{
	public:
		// Network Port Properties
		uint16_t BACnetIPUDPPort;
		uint8_t IPAddress[4];
		uint8_t IPAddressLength;
		uint8_t IPDefaultGateway[4];
		uint8_t IPDefaultGatewayLength;
		uint8_t IPSubnetMask[4];
		uint8_t IPSubnetMaskLength;
		std::vector<uint8_t*> IPDNSServers;
		uint8_t IPDNSServerLength;

		uint8_t BroadcastIPAddress[4];

		bool ChangesPending;
		uint8_t FdBbmdAddressHostType;	// 0 = None, 1 = IpAddress, 2 = Name
		uint8_t FdBbmdAddressHostIp[4];
		uint16_t FdBbmdAddressPort;
		uint16_t FdSubscriptionLifetime;
};

struct CreatedAnalogValue {
	std::string name;
	float value;

	CreatedAnalogValue() {
		this->name = "";
		this->value = 0.0f;
	}
};

class ExampleDatabaseDateTimeValue : public ExampleDatabaseBaseObject
{
	public:
		uint8_t presentValueYear;
		uint8_t presentValueMonth;
		uint8_t presentValueDay;
		uint8_t presentValueWeekDay;
		uint8_t presentValueHour;
		uint8_t presentValueMinute;
		uint8_t presentValueSecond;
		uint8_t presentValueHundredthSeconds;
};

class ExampleDatabase {

	public:
		ExampleDatabaseAnalogInput analogInput;
		ExampleDatabaseAnalogOutput analogOutput;
		ExampleDatabaseAnalogValue analogValue;
		ExampleDatabaseBinaryInput binaryInput;
		ExampleDatabaseBinaryOutput binaryOutput;
		ExampleDatabaseBinaryValue binaryValue;
		ExampleDatabaseDevice device;
		ExampleDatabaseMultiStateInput multiStateInput;
		ExampleDatabaseMultiStateOutput multiStateOutput;
		ExampleDatabaseMultiStateValue multiStateValue;
		ExampleDatabaseTrendLog trendLog;
		ExampleDatabaseTrendLogMultiple trendLogMultiple;
		ExampleDatabaseBitstringValue bitstringValue;
		ExampleDatabaseCharacterStringValue characterStringValue;
		ExampleDatabaseDateValue dateValue;
		ExampleDatabaseIntegerValue integerValue;
		ExampleDatabaseLargeAnalogValue largeAnalogValue;
		ExampleDatabaseOctetStringValue octetStringValue;
		ExampleDatabasePositiveIntegerValue positiveIntegerValue;
		ExampleDatabaseTimeValue timeValue;
		ExampleDatabaseNetworkPort networkPort;
		ExampleDatabaseDateTimeValue dateTimeValue;

	// Storage for create objects
	std::map<uint32_t, CreatedAnalogValue> CreatedAnalogValueData;

	// Constructor / Deconstructor
	ExampleDatabase();
	~ExampleDatabase();

	// Set all the objects to have a default value. 
	void Setup();

	// Update the values as needed 
	void Loop(); 

	// Helper Functions	
	void LoadNetworkPortProperties();

	private:
		const std::string GetColorName();

};


#endif // __CASBACnetStackExampleDatabase_h__
