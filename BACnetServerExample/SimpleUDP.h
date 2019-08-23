#ifndef _SIMPLEUDP_H_
#define _SIMPLEUDP_H_

/*******************************************************************************
SimpleUDP.h

UDP Resource class to handle UDP communication
********************************************************************************

Copyright and License Notice
Chipkin Automation Systems Inc (CAS)

Changing this file in any way without the written permission of Chipkin Automation
Systems Inc is a violation of your license agreement. This notice is integral to
the product. It may not be removed or changed under any circumstances.

Licensee:
A full and detailed license agreement exists between CAS and the Licensee.
Amongst other things, it prohibits the unauthorized use, copying or
distribution of these files or parts thereof. Such acts can harm CAS and cause
financial loss. You may be responsible for the loss and consequent reparations.
If you have any doubt about your rights or obligations with respect to this
file, its use or how it forms a component of the delivered CAS product then
please consult with CAS or a suitable manager of the company identified as the
Licensee. Separating components of the product is also prohibited. Thus you may
not re-arrange the files or components, rename them, consolidate them, delete
any, or copy/transfer a single component item.

This file was written by Steven Smethurst

Warranty
CAS do not warrant that the Product is free of defects but CAS do undertake to
repair, where CAS determines that it is feasible do so. CAS makes no warranty
about the time frame over which defects will be repaired. This warranty does not
imply anything not stated explicitly. How you use this product, how you use the
data provided by the product and how suitable this product is for any purpose at
all is not asserted by CAS. CAS makes no assertions with respect to merchantability,
fitness for a particular purpose, or noninfringement.


Publisher Information:
This Product is published by CAS A Company registered in the province of
British Columbia, Canada.

Contact Information:
CAS can be contacted by email at support@chipkin.com or in writing at
3381 Cambie St, #211 Vancouver, BC, Canada, V5Z 4R3

Reward:
CAS offers a reward to anyone who helps expose violations of the license
agreement.

Unique Identifiers:
These files have unique identifiers that have been customized for the Licensee
to create a unique and verifiable version.

********************************************************************************/

/*
*  Version,	Date,			User,	Notes
*     0.01,	12 Oct 2016 	ACF		Created version Table
*     0.02,	01 Nov 2016  	ACF		Added error and debug logging
*     0.03,	02 Nov 2016		ACF		Added mem dump debugging
*     0.04  25 Sep 2017     ACF     Made class platform independent
*     0.05  25 Sep 2017     ACF     Added header files needed for linux
*									Cleaned up the code for use with linux
*
*/



#include <string.h>
#include <stdio.h>

#ifdef _MSC_VER
#include <winsock2.h>
#include <ws2tcpip.h>
#include <shlobj.h>
#pragma comment(lib,"Ws2_32.lib")

// IP and MAC Address
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#elif defined(__GNUC__)
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <net/if.h>
#include <netinet/in.h>
#include <unistd.h>
#include <resolv.h>
#include <arpa/inet.h>

#define INT_TO_ADDR(_addr) \
	(_addr & 0xFF), \
	(_addr >> 8 & 0xFF), \
	(_addr >> 16 & 0xFF), \
	(_addr >> 24 & 0xFF)

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif // SOCKET_ERROR

#endif


class CSimpleUDP
{

private:
	unsigned short		m_port;			// Stores the port that the resource is connected to	
	bool				m_connected;	// flag that gets set when the resource is successfully connected
	
#ifdef _MSC_VER
	SOCKET				m_socket;
#elif defined(__GNUC__)
	int m_socket;
#endif

	//Function used to force a reconnect of the resource to the stored port
	bool ReConnect();

public:

	CSimpleUDP();
	~CSimpleUDP() {
		this->Disconnect();
	}

	bool IsConnected() { return m_connected; }
	void Disconnect();

	bool Connect(const unsigned short port, bool bindport = true, const char * ipAddress = NULL);
	bool SendMessage(const char * ipAddress, unsigned short port, unsigned char * buffer, unsigned short bufferLength);
	int GetMessage(unsigned char * buffer, unsigned short maxLength, char * ipAddress, unsigned short * port = NULL);
		 
	int GetBroadcastIPAddress(char * broadcastIPAddress, unsigned short maxLength);
	


};

#endif // _SIMPLEUDP_H_
