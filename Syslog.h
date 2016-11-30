#pragma once
#include "WinSock2.h"
#include "Ws2tcpip.h"

// (a.walling 2009-12-17 08:32) - PLID 36624 - Support the syslog protocol

class SyslogClient
{
public:
	SyslogClient(DWORD dwIPAddress, USHORT nPort);
	~SyslogClient(void);

	void Send(const char* szMessage, int nFacility = 1, int nSeverity = 6, const char* szMSGID = "-");

	static CString GetHostName();

protected:
	SOCKET m_socket;

	USHORT m_nPort;
	const DWORD m_dwIPAddress;

	void SendPacket(const char* pData, int dataLength);
};
