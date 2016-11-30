//This file is also used in nxweb, so that file needs to be changed evertime this one is -JMM  04/03
#include "stdafx.h"

#include "tcpsockets.h"
#include <afxtempl.h>
#include <winsock.h>

#include "Client.h"

// (c.haag 2005-10-31 15:15) - PLID 16595 - Added a dependency on NxPropManager
// (c.haag 2006-04-13 15:33) - PLID 20128 - This is now depreciated
//#include "nxpropmanager.h"
//extern CNxPropManager g_propManager;


#define MAXHOSTNAME					128
//#define VER							0x00000001

typedef struct
{
	SOCKADDR_IN namesock;
	SOCKET s;

	char* pcData;
	long nDataSize;
	long nReceivedBytes;
} _SOCKET;

// If you are the server, this is your listen
// socket. If you are the client, this is the
// server's socket you connected to.
_SOCKET g_sServer;

// If you are the server, these are all the
// clients you are connected to.
CArray<_SOCKET*, _SOCKET*> g_sClients;

// Winsock information
WSADATA g_wsad;

// Message that means TCP communication has occured
UINT g_uiTCPMessage;


/*************************************************************
*	Get self IP address in the form of a string
**************************************************************/

DWORD WINAPI GetHostIP_Thread( LPVOID lpParam )
{
    char szLclHost [MAXHOSTNAME];
    LPHOSTENT lpstHostent;
    SOCKADDR_IN stLclAddr;
    SOCKADDR_IN stRmtAddr;
    int nAddrSize = sizeof(SOCKADDR);
    SOCKET hSock;
	in_addr* pIP = (in_addr*)lpParam;
    int nRet;

	DWORD* dwIP = (DWORD*)lpParam;
  
    /* Init local address (to zero) */
    stLclAddr.sin_addr.s_addr = INADDR_ANY;
    
    /* Get the local hostname */
    nRet = gethostname(szLclHost, MAXHOSTNAME); 
    if (nRet != SOCKET_ERROR) {
      /* Resolve hostname for local address */
      lpstHostent = gethostbyname((LPSTR)szLclHost);
      if (lpstHostent)
        stLclAddr.sin_addr.s_addr = *((u_long FAR*) (lpstHostent->h_addr_list[0]));
    } 
    
    /* If still not resolved, then try second strategy */
    if (stLclAddr.sin_addr.s_addr == INADDR_ANY) {
      /* Get a UDP socket */
      hSock = socket(AF_INET, SOCK_DGRAM, 0);
      if (hSock != INVALID_SOCKET)  {
        /* Connect to arbitrary port and address (NOT loopback) */
        stRmtAddr.sin_family = AF_INET;
        stRmtAddr.sin_port   = htons(IPPORT_ECHO);
        stRmtAddr.sin_addr.s_addr = inet_addr("128.127.50.1");
        nRet = connect(hSock,
                       (LPSOCKADDR)&stRmtAddr,
                       sizeof(SOCKADDR));
        if (nRet != SOCKET_ERROR) {
          /* Get local address */
          getsockname(hSock, 
                      (LPSOCKADDR)&stLclAddr, 
                      (int FAR*)&nAddrSize);
        }
        closesocket(hSock);   /* we're done with the socket */
      }
    }

	*pIP = stLclAddr.sin_addr;
	return 0;
}

in_addr g_HostIP = { 0 };

in_addr GetHostIP()
{
	HANDLE hThread;
	DWORD dwThreadID;
	in_addr ipRes;

	if (g_HostIP.S_un.S_addr != 0)
		return g_HostIP;

	hThread = CreateThread(NULL, 0, GetHostIP_Thread, (void*)&ipRes, 0, &dwThreadID);
	if (WaitForSingleObject(hThread, 3000) == WAIT_TIMEOUT)
	{
		ipRes.S_un.S_addr = 0; // return 0.0.0.0 on failure
		TerminateThread(hThread, 0);
		Log("GetHostIP(): Abnormal thread termination");
	}
	else {
		CloseHandle(hThread);
	}

	g_HostIP = ipRes;

	return ipRes;
}

static char* IPFromSocket(_SOCKET* s)
{	
	return inet_ntoa(s->namesock.sin_addr);
}

BOOL TCP_Init(UINT uiMessage)
{
	g_sServer.s = INVALID_SOCKET;
	g_sServer.pcData = new char[1];
	g_sServer.nDataSize = 1;
	g_sServer.nReceivedBytes = 0;

	////////////////////////////////
	// Initialize Windows sockets
    //if(WSAStartup(0x0101,&g_wsad))
    //{
    //   return TRUE;
    //}

	//if ( LOBYTE( g_wsad.wVersion ) != 2 ||
    //    HIBYTE( g_wsad.wVersion ) != 2 ) {
	//	/* Tell the user that we could not find a usable */
	//   /* WinSock DLL.                                  */
	//	WSACleanup( );
	//	return TRUE;
	//}
 	g_uiTCPMessage = uiMessage;

	return FALSE;
}

void TCP_Destroy()
{
	TCP_Disconnect();
	if (g_sServer.s != INVALID_SOCKET) {
		closesocket(g_sServer.s);
		g_sServer.s = INVALID_SOCKET;
	}
	delete g_sServer.pcData;
    WSACleanup();
}

DWORD TCP_Host()
{
	SOCKADDR_IN namesock;

	if (g_sServer.s != INVALID_SOCKET)
		return TRUE;

	// Open the server socket
	if ((g_sServer.s = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET) {
		return TRUE;
	}
	// Select a windows message for socket events
	if(WSAAsyncSelect(g_sServer.s, GetMainFrame()->GetSafeHwnd(), g_uiTCPMessage, FD_ACCEPT|FD_CLOSE) == SOCKET_ERROR) {
		closesocket(g_sServer.s);
		g_sServer.s = INVALID_SOCKET;
		return TRUE;
	}
	// Bind the socket to a port
    namesock.sin_family=PF_INET;
	namesock.sin_port=htons(11983);
    namesock.sin_addr.s_addr=INADDR_ANY;
    if(bind(g_sServer.s,(LPSOCKADDR)&namesock,sizeof(SOCKADDR))==SOCKET_ERROR)
    {
		closesocket(g_sServer.s);
		g_sServer.s = INVALID_SOCKET;
        return TRUE;
    }
	// Listen for connections
	// (a.walling 2012-06-25 15:23) - PLID 51191 - Use SOMAXCONN as the backlog value rather than 5; not sure why 5 was chosen in the first place: SOMAXCONN == 'a reasonable number'
    if(listen(g_sServer.s,SOMAXCONN)==SOCKET_ERROR)
    {
		closesocket(g_sServer.s);
		g_sServer.s = INVALID_SOCKET;
        return(TRUE);
    }

	return FALSE;
}

DWORD TCP_Connect(DWORD dwHostIP)
{
	DWORD dwNonBlocking = 1;

	if (g_sServer.s != INVALID_SOCKET)
		return TRUE;

//	if (dwHostIP == GetHostIP().S_un.S_addr)
//	{
//		return TCP_Host();
//	}

	// Open the server socket
	if ((g_sServer.s = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET) {
		return TRUE;
	}
    g_sServer.namesock.sin_family=PF_INET;
	g_sServer.namesock.sin_port=htons(11983);
	g_sServer.namesock.sin_addr.s_addr=dwHostIP;

	// Select a windows message for socket events
    if(WSAAsyncSelect(g_sServer.s, GetMainFrame()->GetSafeHwnd(), g_uiTCPMessage, FD_CONNECT|FD_READ|FD_CLOSE)==SOCKET_ERROR)
    {
		closesocket(g_sServer.s);
		g_sServer.s = INVALID_SOCKET;
        return(TRUE);
	}

	// Connect to the server
	if(connect(g_sServer.s,(PSOCKADDR)&g_sServer.namesock,sizeof(SOCKADDR))==SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			closesocket(g_sServer.s);
			g_sServer.s = INVALID_SOCKET;
			return(TRUE);
		}
	} else {
		// (a.walling 2012-06-13 08:32) - PLID 50956 - Set TCP_NODELAY to disable the Nagle algorithm and eliminate the 200 ms delay
		NxSocketUtils::SetNoDelay(g_sServer.s);
	}

	return FALSE;
}

DWORD TCP_Disconnect()
{
	int i;

	if (g_sServer.s == INVALID_SOCKET) return TRUE;

	// Close server socket
	closesocket(g_sServer.s);
	g_sServer.s = INVALID_SOCKET;

	// Close client sockets
	for (i=0; i < g_sClients.GetSize(); i++) {
		if (g_sClients.GetAt(i)->s != INVALID_SOCKET) closesocket(g_sClients.GetAt(i)->s);
		delete g_sClients.GetAt(i)->pcData;
		delete g_sClients.GetAt(i);
	}
	g_sClients.RemoveAll();

	return FALSE;
}

static int SendPacket(SOCKET s, void* pData, WORD wSize, DWORD dwHeaderFlags)
{
/*	DWORD dwVer = VER;
	char bOut[4096+8];

	memcpy(bOut, &dwVer, 4);
	memcpy(bOut+4, &dwHeaderFlags, 4);
	memcpy(bOut+8, pData, wSize);

	return send(s, bOut, wSize+8, 0);*/

	return send(s, (const char*)pData, wSize, dwHeaderFlags);
}

DWORD TCP_Send(DWORD dwIP, void* pData, WORD wSize)
{
	//DWORD dwVer = VER;
	DWORD dwHeaderFlags = 0;

	if (g_sServer.s == INVALID_SOCKET) return TRUE;
	if (wSize >= 4096) return TRUE;

	// This will be true if you are sending data to
	// the server
//	if (!strcmp(IPFromSocket(&g_sServer), szIP)) {
	if (g_sServer.namesock.sin_addr.S_un.S_addr == dwIP)
	{
		/*TS:  With this code in, if you're on the same computer as the server, the server never gets your packet.
		if (GetHostIP().S_un.S_addr == dwIP)
			return TRUE;*/
		
		SendPacket(g_sServer.s, pData, wSize, 0);
		return 0;
	}
	else {
		for (int i=0; i < g_sClients.GetSize(); i++)
		{
			_SOCKET* skt = g_sClients.GetAt(i);
//			if (!strcmp(IPFromSocket(&skt), szIP)) {
			if (skt->namesock.sin_addr.S_un.S_addr == dwIP) {

				SendPacket(skt->s, pData, wSize, 0);
				return 0;
			}
		}
	}
	return 1;
}

DWORD TCP_Broadcast(void* pData, WORD wSize)
{
	//DWORD dwVer = VER;
	DWORD dwHeaderFlags = 0;

	if (!g_sClients.GetSize()) return 1;

	for (int i=0; i < g_sClients.GetSize(); i++)
	{
		_SOCKET* skt = g_sClients.GetAt(i);
		SendPacket(skt->s, pData, wSize, 0);
	}
	return 0;
}

void TCP_ScanInputData(_SOCKET* pskt, unsigned long ip)
{	
	char* c = pskt->pcData;
	while (c < pskt->pcData + pskt->nReceivedBytes)
	{
		//DRT 6/15/2007 - PLID 25531 - Packets are no longer part of NetUtils namespace
		_NxHeader* pHdr = (_NxHeader*)c;

		// Our first test is to see if c points to a Practice packet.
		if (pHdr->dwKey == 0 && (pHdr->dwVer >= 0 && pHdr->dwVer <= 2) && pHdr->wSize > 0)
		{
			// Ok, it probably does. Lets see if we can handle it right now.
			if (pHdr->wSize > pskt->nReceivedBytes)
			{
				// No, we have more data coming. Lets quit now.
				return;
			}
			WORD wSize = pHdr->wSize; // Preserve this value because it will be destroyed in memmove.

			// Ok, this is something we can handle. Send it to the application.
			CClient::OnReceived(ip, pHdr, wSize);

			// Now that the application has dealt with the packet, flush the data
			// in our input buffer
			memmove(c, c + pHdr->wSize, pskt->nReceivedBytes - pHdr->wSize);
			pskt->nReceivedBytes -= wSize;
		}
		else
		{
			// It's not a NexTech packet, or it's part of an invalid packet. Just
			// move on.
			memmove(c, c + 1, pskt->nReceivedBytes - 1);
			pskt->nReceivedBytes--;
		}
	}
}

// When one of the nominated network events occurs on the specified socket s,
// the application's window hWnd receives message wMsg. The wParam parameter
// identifies the socket on which a network event has occurred. The low
// word of lParam specifies the network event that has occurred. The high
// word of lParam contains any error code. The error code be any error as
// defined in Winsock2.h.

ETCPResult TCP_HandleMessage(WPARAM wParam, LPARAM lParam, DWORD * pIPSource /* out */, void* pData /* in out */, WORD* pwSize /* in out */)
{
	SOCKADDR_IN namesock;
	//WORD wMaxSize = min(4096, *pwSize);
	//int nRecvBytes;
	//char bIn[4096+8], *pbIn = bIn;
	const long nMaxSize = 1024;
	char szData[nMaxSize];
	long nBytesRead = 0;
	_SOCKET* pskt;
	//wMaxSize += 8;	// Header info

	if (wParam == g_sServer.s) {
		WORD nNewBytes = 0;

		switch (LOWORD(lParam)) {
		case FD_ACCEPT: // A client has joined you
			{
				int i = sizeof(SOCKADDR);
				SOCKET sNew = accept(g_sServer.s, (LPSOCKADDR)&namesock, &i);
				if (sNew != INVALID_SOCKET) {
				    if(WSAAsyncSelect(sNew, GetMainFrame()->GetSafeHwnd(), g_uiTCPMessage, FD_READ|FD_CLOSE)==SOCKET_ERROR)
					{
						closesocket(sNew);
					}
					else {
						// (a.walling 2012-06-13 08:32) - PLID 50956 - Set TCP_NODELAY to disable the Nagle algorithm and eliminate the 200 ms delay
						NxSocketUtils::SetNoDelay(sNew);
						_SOCKET* p = new _SOCKET;
						p->namesock = namesock;
						p->s = sNew;
						p->pcData = new char[1];
						p->nDataSize = 1;
						p->nReceivedBytes = 0;

						g_sClients.Add(p);

						//if (pIPSource) {
						//	*pIPSource = skt.namesock.sin_addr.S_un.S_addr;
						//}
					}
				}
			}
			return EClientConnected;

		case FD_READ: // You are a client and the server sent you data
			// Read up to nMaxSize bytes at a time, and be sure to
			// treat it like a stream
			pskt = &g_sServer;
			while ((nBytesRead = recv(g_sServer.s, szData, nMaxSize, 0)) > 0)
			{
				// Scale our input buffer to make sure it can fit
				// what it has now plus this additional data
				while (pskt->nDataSize < pskt->nReceivedBytes + nBytesRead)
				{
					char* p = new char[pskt->nDataSize << 1];
					if (!p)	ThrowNxException("Input data overflow");
					memcpy(p, pskt->pcData, pskt->nDataSize);
					delete pskt->pcData;
					pskt->pcData = p;
					pskt->nDataSize <<= 1;
				}

				// Now append the data to the input buffer
				memcpy(pskt->pcData + pskt->nReceivedBytes, szData, nBytesRead);
				pskt->nReceivedBytes += nBytesRead;
				TCP_ScanInputData(pskt, g_sServer.namesock.sin_addr.S_un.S_addr);
			}

			/*while (wMaxSize && (nRecvBytes = recv(g_sServer.s, pbIn, wMaxSize, 0)) > 0)
			{
				pbIn += nRecvBytes;
				nNewBytes += nRecvBytes;
				wMaxSize -= nRecvBytes;
			}
			// Make sure we are using the same version
			//if (*((DWORD*)bIn) != VER)
			//	return EError;
			
			if (pIPSource) {
				*pIPSource = g_sServer.namesock.sin_addr.S_un.S_addr;
				*pIPSource = g_sServer.namesock.sin_addr.S_un.S_addr;
			}
			*pwSize = nNewBytes;
			memcpy(pData, bIn, nNewBytes);*/

			return EReceivedData;

		case FD_CONNECT:
			{
				WORD errcode = HIWORD(lParam);

				if (!errcode)
				{
					//g_propManager.SetNxServerSocket(g_sServer.s);
					CClient::isConnected = true;
					Log("Connected to network service");
					// (a.walling 2012-06-13 08:32) - PLID 50956 - Set TCP_NODELAY to disable the Nagle algorithm and eliminate the 200 ms delay
					NxSocketUtils::SetNoDelay(g_sServer.s);
				}
				else {
					//g_propManager.SetNxServerSocket(INVALID_SOCKET);
					CClient::isConnected = false; // this should be false anyway
					Log("Failed to connect to network service");
				}
				return EServerAccepted;
			}

		case FD_CLOSE: // You are a client and the server dropped you
			closesocket(g_sServer.s);
			g_sServer.s = INVALID_SOCKET;
			return EServerDisconnected;
		}
	}
	else {
		for (int i=0; i < g_sClients.GetSize(); i++) {
			if (wParam == g_sClients.GetAt(i)->s) {
				WORD nNewBytes = 0;
				pskt = g_sClients.GetAt(i);

				// Give the caller the IP address of the client
				//if (pIPSource) {
					//_SOCKET skt = g_sClients.GetAt(i);
					//*pIPSource = skt.namesock.sin_addr.S_un.S_addr;
				//}

				switch (LOWORD(lParam)) {
				case FD_READ:	// You are a server and the client sent you data

					// Read up to nMaxSize bytes at a time, and be sure to
					// treat it like a stream
					while ((nBytesRead = recv(wParam, szData, nMaxSize, 0)) > 0)
					{
						// Scale our input buffer to make sure it can fit
						// what it has now plus this additional data
						while (pskt->nDataSize < pskt->nReceivedBytes + nBytesRead)
						{
							char* p = new char[pskt->nDataSize << 1];
							if (!p)	ThrowNxException("Input data overflow");
							memcpy(p, pskt->pcData, pskt->nDataSize);
							delete pskt->pcData;
							pskt->pcData = p;
							pskt->nDataSize <<= 1;
						}

						// Now append the data to the input buffer
						memcpy(pskt->pcData + pskt->nReceivedBytes, szData, nBytesRead);
						pskt->nReceivedBytes += nBytesRead;
						TCP_ScanInputData(pskt, g_sServer.namesock.sin_addr.S_un.S_addr);
					}


					/*while (wMaxSize && (nRecvBytes = recv(wParam, pbIn, wMaxSize, 0)) > 0)
					{
						pbIn += nRecvBytes;
						nNewBytes += nRecvBytes;
						wMaxSize -= nRecvBytes;
					}
					// Make sure we are using the same version
					//if (*((DWORD*)bIn) != VER)
					//	return EError;

					*pwSize = nNewBytes;
					memcpy(pData, bIn, nNewBytes);*/

					return EReceivedData;

				case FD_CLOSE: // You are a server and a client dropped you
					{
						_SOCKET* skt = g_sClients.GetAt(i);
						closesocket(wParam);
						skt->s = INVALID_SOCKET;
					}
					return EClientDisconnected;
				}
			}
		}
	}
	return EError;
}