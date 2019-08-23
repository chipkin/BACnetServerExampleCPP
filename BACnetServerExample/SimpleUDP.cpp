/*******************************************************************************
SimpleUDP.cpp
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

#include "SimpleUDP.h"
#include <sstream>

CSimpleUDP::CSimpleUDP() {
	m_connected = false;
	m_port = 0;
	this->m_socket = 0;
}

bool CSimpleUDP::ReConnect() {
	// Check if the resource is still connected and if so, disconnect
	if (this->m_connected) {
		this->Disconnect();
	}

	// Connect using the existing port
	if (this->m_port > 0) {
		return this->Connect(this->m_port);
	}

	return false;
}

void CSimpleUDP::Disconnect() {
	// Check if the resource has already been disconnected
	if (!this->IsConnected()) {
		return;
	}

	#ifdef _MSC_VER
	closesocket(this->m_socket);
	WSACleanup();
	#elif defined (__GNUC__)
	close(this->m_socket);
	#endif
	
	this->m_connected = false;
}

bool CSimpleUDP::Connect(unsigned short port, bool bindport /* = true */, const char * ipAddress /* = NULL */) {

	struct sockaddr_in addr;
	struct timeval tv;
	int ret;

	// Disconnect if already connected
	this->Disconnect();

	// Set the port internally
	this->m_port = port;

	// If Windows, setup Winsock
#ifdef _MSC_VER
	// Declare variables
	//---------------------------------------
	WSADATA  wsaData;

	// Initialize Winsock
	//---------------------------------------
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
		// Error starting winsock
		return false;
	}
#endif

	// Create the UDP socket
	this->m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if ((int)m_socket < 0) {
		this->Disconnect();
		return false;
	}

	// Set socket options
	int bOptVal = 1;
	int bOptLen = sizeof(int);
	// Set Reuse Address
	if (setsockopt(this->m_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&bOptVal, bOptLen) == SOCKET_ERROR) {
		this->Disconnect();
		return false;
	}
	// Set Timeout
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if (setsockopt(this->m_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval)) == SOCKET_ERROR) {
		this->Disconnect();
		return false;
	}
	// Set broadcast
	if (setsockopt(this->m_socket, SOL_SOCKET, SO_BROADCAST, (char*)&bOptVal, bOptLen) == SOCKET_ERROR) {
		Disconnect();
		return false;
	}

	// Zero out the sockaddr_in structure
	memset((char*)&addr, 0, sizeof(addr));

	// Prepare the sockaddr_in structure
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (ipAddress == NULL) {
		// bind to any available
		addr.sin_addr.s_addr = INADDR_ANY;
	}
	else {
		// bind to specific interface
		#ifdef _MSC_VER
		inet_pton(AF_INET, ipAddress, &addr.sin_addr);
		#elif defined (__GNUC__)
		inet_aton(ipAddress, &addr.sin_addr);
		#endif	
	}

	// Bind the port
	if (bindport) {
		ret = bind(this->m_socket, (struct sockaddr*)&addr, sizeof(addr));
		if( ret != 0 ) {
			this->Disconnect();
			return false;
		}
	}

	this->m_connected = true;
	return true;
}


bool CSimpleUDP::SendMessage(const char * ipAddress, unsigned short portnum, unsigned char * buffer, unsigned short bufferLength) {
	struct sockaddr_in toAddr;
	int toAddrLen = sizeof(toAddr);
	int ret;
	
	// Check to see if we have created a connection 
	if (!this->IsConnected()) {
		// Not connected, try to reconnect
		if (!this->ReConnect()) {
			// we can not create a connection 
			return false;
		}
	}

	// Check parameters
	if (ipAddress == NULL) {
		return false;	// No IP Address provided
	}
	if (buffer == NULL || bufferLength == 0) {
		return false;	// Nothing to send
	}
    
	// Setup the toAddr
	toAddr.sin_family       = AF_INET;
	toAddr.sin_port         = htons(portnum);
	#ifdef _MSC_VER
	inet_pton(AF_INET, ipAddress, &toAddr.sin_addr);
	#elif defined (__GNUC__)
	inet_aton(ipAddress, &toAddr.sin_addr);
	#endif

    // Send the message 
	ret = sendto(this->m_socket, (char*)buffer, bufferLength, 0, (struct sockaddr *)&toAddr, toAddrLen);
	if (ret == bufferLength) {
		return true;
	}

	if (ret == SOCKET_ERROR) {
		// Issue with the socket, disconnect
		this->Disconnect();
	}
    return false;
}

int CSimpleUDP::GetMessage(unsigned char * buffer, unsigned short maxLength, char * ipAddress, unsigned short * port /* = NULL */) {
	// Check to see if we have created a connection 
	if (!this->IsConnected()) {
		// Not connected, try to reconnect
		if (!this->ReConnect()) {
			// we can not create a connection 
			return 0;
		}
	}

	// Check parameters
	if (buffer == NULL || maxLength == 0) {
		return 0;
	}

	int ret;

	// If windows, do some other checks
#ifdef _MSC_VER
// Set up a time out 
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 500;

	// Set up the search socket 
	fd_set   readflds;
	readflds.fd_count = 1;
	readflds.fd_array[0] = this->m_socket;

	// Check to see if there is any data on the socket 
	ret = select(0, &readflds, NULL, NULL, &timeout);
	if (ret == SOCKET_ERROR) {
		ret = WSAGetLastError();
		return -1;
	}
	if (ret == 0) {
		// Nothing to read yet
		return 0;
	}
#endif

	// Get the data 
	struct sockaddr_in fromAddr;
	socklen_t fromAddrLength = sizeof(fromAddr);
	ret = recvfrom(this->m_socket, (char*)buffer, maxLength, 0, (sockaddr *)&fromAddr, &fromAddrLength);
	if (ret > 0) {
		if (ipAddress != NULL) {
			char * temp = inet_ntoa(fromAddr.sin_addr);
			sprintf(ipAddress, "%s", temp);
			/*
			sprintf(ipAddress, "%d.%d.%d.%d", fromAddr.sin_addr.S_un.S_un_b.s_b1,
				fromAddr.sin_addr.S_un.S_un_b.s_b2,
				fromAddr.sin_addr.S_un.S_un_b.s_b3,
				fromAddr.sin_addr.S_un.S_un_b.s_b4);
			*/
		}
		if (port != NULL) {
			*port = fromAddr.sin_port;
		}
	}

	return ret;
}


int CSimpleUDP::GetBroadcastIPAddress(char * broadcastIPAddress, unsigned short maxLength) {
#ifdef _MSC_VER
	unsigned long ulSize = 0;
	GetAdaptersInfo(NULL, &ulSize);
	LPBYTE pbBuffer = new BYTE[ulSize];
	GetAdaptersInfo((PIP_ADAPTER_INFO)pbBuffer, &ulSize);
	PIP_ADAPTER_INFO pInfo = (PIP_ADAPTER_INFO)pbBuffer;

	while (pInfo != NULL) {
		IP_ADDR_STRING * pIPAddr;
		std::string ipAddress = pInfo->IpAddressList.IpAddress.String;
		std::string netmask = pInfo->IpAddressList.IpMask.String;
		if (ipAddress.empty()) {
			pIPAddr = pInfo->IpAddressList.Next;
			while (pIPAddr) {

				ipAddress = pIPAddr->IpAddress.String;
				netmask = pIPAddr->IpMask.String;
				if (!ipAddress.empty()) {
					break;
				}
				pIPAddr = pIPAddr->Next;
			}
		}

		IP_ADDR_STRING * pGatewayAddr;
		std::string ipGateway = pInfo->GatewayList.IpAddress.String;
		if (ipAddress.empty()) {
			pGatewayAddr = pInfo->GatewayList.Next;
			while (pGatewayAddr) {
				ipGateway = pGatewayAddr->IpAddress.String;
				if (!ipGateway.empty()) {
					break;
				}
				pGatewayAddr = pGatewayAddr->Next;
			}
		}

		u_long host_addr = inet_addr(ipAddress.c_str());   // local IP addr
		u_long net_mask = inet_addr(netmask.c_str());		 // LAN netmask
		u_long net_addr = host_addr & net_mask;				 // 172.16.64.0
		u_long dir_bcast_addr = net_addr | (~net_mask);		 // 172.16.95.255

		unsigned char c[4];
		memcpy(c, &dir_bcast_addr, 4);
		delete(pbBuffer);
		return sprintf(broadcastIPAddress, "%d.%d.%d.%d", c[0], c[1], c[2], c[3]);

		// ToDo: why is this here, this needs to be refactored. 
		pInfo = pInfo->Next;
	}

	delete(pbBuffer);
#elif defined (__GNUC__)
	struct ifconf ifc;
	struct ifreq ifr[10];
	int ifc_num, i;
	char * temp;

	if (this->m_socket > 0) {
		ifc.ifc_len = sizeof(ifr);
		ifc.ifc_ifcu.ifcu_buf = (caddr_t)ifr;

		if (ioctl(this->m_socket, SIOCGIFCONF, &ifc) == 0) {
			ifc_num = ifc.ifc_len / sizeof(struct ifreq);
			for (i = 0; i < ifc_num; ++i) {
				if (ifr[i].ifr_addr.sa_family != AF_INET) {
					continue;
				}

				// Retrieve the IP Address
				if (ioctl(this->m_socket, SIOCGIFADDR, &ifr[i]) == 0) {
					temp = inet_ntoa(((struct sockaddr_in *)(&ifr[i].ifr_addr))->sin_addr);
					if (strcmp(temp, "127.0.0.1") == 0 || // local host
						strcmp(temp, "0.0.0.0") == 0) {  // invalid / unconnected resource
						continue;
					}
				}
				else {
					continue;
				}

				if (ioctl(this->m_socket, SIOCGIFBRDADDR, &ifr[i]) == 0) {
					temp = inet_ntoa(((struct sockaddr_in *)(&ifr[i].ifr_netmask))->sin_addr);
					return snprintf(broadcastIPAddress, maxLength, "%s", temp);
				}
			}
		}
	}
#endif

	return 0;
}