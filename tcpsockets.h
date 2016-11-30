//JMM 04/03  This file is used in NxWeb so any changes made to this file have to be reflected in that one also
#ifndef __TCP_SOCKETS_H__
#define __TCP_SOCKETS_H__

#pragma once



#define TCP_MAX_DEFAULT_DATASIZE	4096

#include <winsock2.h>

// Enumeration of what kind of information was
// given from the Windows message
typedef enum {
	EClientConnected,
	EServerAccepted,	// The server accepted you as a client
	EClientDisconnected,
	EServerDisconnected,
	EReceivedData,
	EError,
} ETCPResult;

// One-time initialize
BOOL TCP_Init(UINT uiMessage);
// One-time destruction
void TCP_Destroy();

// Get your own IP address in binary format
in_addr GetHostIP();


// Host a server
DWORD TCP_Host();
// Connect to a server
DWORD TCP_Connect(DWORD dwHostIP);
// Disconnect OR unhost
DWORD TCP_Disconnect();
// Send data to a server or a client
DWORD TCP_Send(DWORD dwIP, void* pData, WORD wSize);
// If you are a server, send data to all clients
DWORD TCP_Broadcast(void* pData, WORD wSize);
// TCP message handler (Does all the work)
//ETCPResult TCP_HandleMessage(WPARAM wParam, LPARAM lParam, char** ppszIPSource, void* pData, WORD* pwSize);
ETCPResult TCP_HandleMessage(WPARAM wParam, LPARAM lParam, DWORD * pIPSource /* out */, void* pData /* in out */, WORD* pwSize /* in out */);

#endif
