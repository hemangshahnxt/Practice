#include "stdafx.h"
#include "Syslog.h"

// (a.walling 2009-12-17 08:32) - PLID 36624 - Support the syslog protocol

SyslogClient::SyslogClient(DWORD dwIPAddress, USHORT nPort)
:	m_socket(INVALID_SOCKET),
	m_dwIPAddress(dwIPAddress),
	m_nPort(nPort)
{
	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_socket == INVALID_SOCKET) {
		ThrowNxException("Failed to create UDP socket: 0x%08x", WSAGetLastError());
	}
}

SyslogClient::~SyslogClient(void)
{
	if (m_socket != INVALID_SOCKET) {
		shutdown(m_socket, SD_SEND);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

/*
Standard facility values:
          Numerical             Facility
             Code

              0             kernel messages
              1             user-level messages
              2             mail system
              3             system daemons
              4             security/authorization messages
              5             messages generated internally by syslogd
              6             line printer subsystem
              7             network news subsystem
              8             UUCP subsystem
              9             clock daemon
             10             security/authorization messages
             11             FTP daemon
             12             NTP subsystem
             13             log audit
             14             log alert
             15             clock daemon (note 2)
             16             local use 0  (local0)
             17             local use 1  (local1)
             18             local use 2  (local2)
             19             local use 3  (local3)
             20             local use 4  (local4)
             21             local use 5  (local5)
             22             local use 6  (local6)
             23             local use 7  (local7)

Standard severity values:
           Numerical         Severity
             Code

              0       Emergency: system is unusable
              1       Alert: action must be taken immediately
              2       Critical: critical conditions
              3       Error: error conditions
              4       Warning: warning conditions
              5       Notice: normal but significant condition
              6       Informational: informational messages
              7       Debug: debug-level messages
*/

void SyslogClient::Send(const char* szMessage, int nFacility /*= 1*/, int nSeverity /*= 6*/, const char* szMSGID /*= "-"*/)
{
	CString strFullMessage;

	// PRI - always in <brackets>
	long nPriority = (nFacility * 8) + nSeverity;
	CString strPRI;
	strPRI.Format("<%li>", nPriority);

	// the format is basically as follows:
	// PRI VERSION TIMESTAMP HOSTNAME APPNAME PROCID MSGID STRUCTURED_DATA MSG
	// any element can be omitted by using the NILVALUE, which is simply a -

	strFullMessage.Format("%s1 %s %s Practice %lu %s - %s", strPRI, GetISO8601Timestamp(), GetHostName(), GetCurrentProcessId(), szMSGID, szMessage);

	TRACE("SYSLOG: %s\n", strFullMessage);

	SendPacket(strFullMessage, strFullMessage.GetLength());
}

CString SyslogClient::GetHostName()
{
	char name[255];
	if (0 == gethostname(name, sizeof(name))) {
		return name;
	} else {
		CString strIP;
		in_addr ipAddress = GetHostIP();
		strIP.Format("%lu.%lu.%lu.%lu",	long(ipAddress.S_un.S_un_b.s_b1), long(ipAddress.S_un.S_un_b.s_b2), long(ipAddress.S_un.S_un_b.s_b3), long(ipAddress.S_un.S_un_b.s_b4));
		return strIP;
	}
}

void SyslogClient::SendPacket(const char* pData, int dataLength)
{
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = m_nPort;
	serverAddr.sin_addr.s_addr = htonl(m_dwIPAddress);

	int ret = sendto(m_socket, pData, dataLength, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

	// (a.walling 2009-12-18 08:37) - PLID 36624 - truncate message if over maximum packet length. Yes, this is specified behavior.
	// There are other transports (TCP, TLS, etc) to workaround this, as well as Reliable-syslog over UDP, but since it is unlikely
	// anyone will ever use this feature anyway, we will just truncate as noted in the UDP transmission spec of BSD-syslog.

	if (ret == SOCKET_ERROR) {
		int lastError = WSAGetLastError();

		if (lastError != WSAEMSGSIZE) {
			ThrowNxException("Failed to send to syslog: 0x%08x", WSAGetLastError());
		} else {
			DWORD dwMaxSize = 0;
			int nBufferSize = sizeof(dwMaxSize);
			ret = getsockopt(m_socket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&dwMaxSize, &nBufferSize);

			if (SOCKET_ERROR != ret) {
				if ((int)dwMaxSize < dataLength) {
					SendPacket(pData, dwMaxSize);
				} else {
					ThrowNxException("Failed to send to syslog; unable to truncate packet");
				}
			} else {
				ThrowNxException("Failed to send to syslog; unable to truncate packet");
			}
		}
	}
}