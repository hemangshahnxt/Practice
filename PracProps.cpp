#include "stdafx.h"
#include "PracProps.h"
#include "NxStandard.h"
#include "GlobalDataUtils.h"
#include "RegUtils.h"
#include "ConnectionDlg.h"
#include "tcpsockets.h"
#include "nxpackets.h"
#include "NxServer.h"
#include "backup.h"
#include "NxBackupUtils.h"
#include "NxPropManager.h"

#include "ShowConnectingFeedbackDlg.h"
#include "MiscSystemUtils.h"

#include "ConnectionHealth.h"
#include <NxSystemUtilitiesLib\NxConnectionUtils.h>

#include <map>
#include <list>

using std::map;
using std::list;

// (a.walling 2011-09-07 18:01) - PLID 45448 - NxAdo unification - Practice changes

using namespace ADODB;
#define SLOW_REMOTE_PROPERTY

#define CHECK_SNAPSHOT_ISOLATION_STATE_DELAY			10 * 1000

//DRT 6/15/2007 - PLID 25531 - Packets are no longer part of NetUtils namespace, removed numerous NetUtils::

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

long GetCommandTimeout();

// (c.haag 2010-02-24 11:59) - PLID 37526 - Protect this connection substring with a critical section
// (a.walling 2010-06-02 07:49) - PLID 31316 - We really only need one critical section for all this, not 4. Just use l_csConnectionProperties
CCriticalSection l_csConnectionProperties; 

// (c.haag 2010-02-24 11:59) - PLID 37526 - We need to protect certain global Get functions with critical sections
// for thread-safe accessibility. This class is instantiated in those functions to accomplish that.
// (a.walling 2010-06-02 07:49) - PLID 31316 - This already exists as a CSingleLock
/*
class CConnectionSubstringSection
{
	CCriticalSection& m_cs;
public:
	CConnectionSubstringSection(CCriticalSection& cs) : m_cs(cs)
	{
		m_cs.Lock();
	}
	~CConnectionSubstringSection()
	{
		m_cs.Unlock();
	}
};
*/

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - lots of stuff moved to NxAdo

const CString &GetNetworkLibraryString();

static CString l_strSqlServerName;
// (a.walling 2010-06-02 07:49) - PLID 31316 - We really only need one critical section for all this, not 4. Just use l_csConnectionProperties
//CCriticalSection l_csSqlServerName; // (c.haag 2010-02-24 11:59) - PLID 37526 - Protect this connection substring with a critical section
const CString &GetSqlServerName()
{
	//BVB
	CSingleLock cs(&l_csConnectionProperties, TRUE);
	CString nl = GetNetworkLibraryString();
	//first, try to get it from the registry
	if (nl.IsEmpty())
	{	l_strSqlServerName = NxRegUtils::ReadString(
			_T(GetRegistryBase() + "SqlServerName"));
	}
	else
	{	l_strSqlServerName = NxRegUtils::ReadString(
			_T(GetRegistryBase() + "SqlServerIP"));
	}
			
	return l_strSqlServerName;
}

// (a.walling 2010-06-02 07:49) - PLID 31316 - Get (and check) the data provider string (eg Provider=SQLOLEDB)
static BOOL l_bDataProviderStringSet = FALSE;
static CString l_strDataProviderString;
const CString &GetDataProviderString()
{	
	CSingleLock cs(&l_csConnectionProperties, TRUE);

	if (!l_bDataProviderStringSet) 
	{
		CString strDataProvider = NxRegUtils::ReadString(
			_T(GetRegistryBase() + "DataProvider"), "SQLOLEDB");

		CString strRegistryLocation;
		strRegistryLocation.Format("HKEY_CLASSES_ROOT\\%s", strDataProvider);
		if (strDataProvider.IsEmpty() || (strDataProvider.CompareNoCase("SQLOLEDB") != 0 && !NxRegUtils::DoesKeyExist(strRegistryLocation)) ) {
			strDataProvider = "SQLOLEDB";
		}

		CString strUpper = strDataProvider;
		strUpper.MakeUpper();

		if (strUpper.Find("SQLNCLI") != -1) {
			// native client; usually SQLNCLI. it's possible we may end up having to use SQLNCLI.1 or something like that,
			// which is why we use a string rather than a DWORD in the registry. Ensure we set the data type compatibility.
			// (a.walling 2012-08-16 09:12) - PLID 52166 - When using SQLNCLI, DataTypeCompatibility should remain set to 80 instead of 90; the latter is undefined.
			// set datatypecompatibility to 80
			l_strDataProviderString.Format("Provider=%s;DataTypeCompatibility=80;", strDataProvider);
		} else {
			// sql OLEDB (don't allow anything other than sqlncli or sqloledb)
			l_strDataProviderString = "Provider=SQLOLEDB;";
		}

		l_bDataProviderStringSet = TRUE;
	}

	return l_strDataProviderString;
}

static BOOL l_bNetworkLibraryStringSet = FALSE;
static CString l_strNetworkLibraryString;
// (a.walling 2010-06-02 07:49) - PLID 31316 - We really only need one critical section for all this, not 4. Just use l_csConnectionProperties
//CCriticalSection l_csNetworkLibraryString; // (c.haag 2010-02-24 11:59) - PLID 37526 - Protect this connection substring with a critical section
const CString &GetNetworkLibraryString()
{
	CSingleLock cs(&l_csConnectionProperties, TRUE);
	if (!l_bNetworkLibraryStringSet) 
	{
		//First, try loading from the registry
		CString str = NxRegUtils::ReadString(
			_T(GetRegistryBase() + "ConnectionType"));

		if (str == "LAN")
		{	l_strNetworkLibraryString = _T("");
			l_bNetworkLibraryStringSet = TRUE;
			return l_strNetworkLibraryString;
		}
		else if (str == "IP")
		{	l_strNetworkLibraryString = _T("Network Library=dbmssocn;");
			l_bNetworkLibraryStringSet = TRUE;
			return l_strNetworkLibraryString;
		}
	}

	return l_strNetworkLibraryString;
}


// (a.walling 2010-06-02 07:49) - PLID 31316 - Standard database connection string
// will use PracData[_subkey] if no override database passed in.
// (a.walling 2010-07-28 13:08) - PLID 39871 - Suffix to be appended to the application name
// (a.walling 2010-07-28 13:06) - PLID 39871 - We now use the application name (and the application name suffix passed in)
// in order to explicitly differentiate connections for connection pooling purposes.
CString GetStandardConnectionString(bool bIncludePassword, const CString& strDatabase, const CString& strApplicationNameSuffix)
{
	//DRT 7/3/02 - This uses the database that is the same as the subkey we're currently using.
	//			If no key is specified, we just good ole PracData
	CString strDB = strDatabase;
	CString strSubkey;
	if (strDB.IsEmpty()) {
		strSubkey = GetSubRegistryKey();
		if (strSubkey.IsEmpty()) {
			strDB = "PracData";
		}
		else {
			strDB.Format("PracData_%s", strSubkey);
		}
	}

	CString strConnectionString;

	//DRT - 9/12/2006 - PLID 22485 - This must be added for Vista security.  If you do not put
	//	this 'Persist Security Info' line in place, then anyone running Vista that calls
	//	GetConnectionString() will not get the password.  And outside apps, like the datalist, 
	//	would be unable to connect to the data in a thread.
	// (a.walling 2010-07-28 13:06) - PLID 39871 - We now use the application name (and the application name suffix passed in)
	// in order to explicitly differentiate connections for connection pooling purposes.
	// (b.savon 2016-05-18 13:19) - NX-100670 - Check the flag to use Windows Auth for SQL connection 
	if (NxConnectionUtils::UseSqlIntegratedSecurity(strSubkey)) {
		strConnectionString.Format(
			"%s"								// provider (and possibly other options)
			"%s"								// network library
			"Data Source=%s;"					// server
			"Initial Catalog=%s;"				// database
			"Persist Security Info=True;"
			//"Language=us_english;"			// (a.walling 2012-02-09 17:13) - PLID 48101 - We use collation of the db, no point setting this here as far as I can tell
			"Application Name=%s%s;"			// application name
			"Integrated Security=SSPI;",						// sa password
			GetDataProviderString(),
			GetNetworkLibraryString(),
			GetSqlServerName(),
			strDB,
			"NexTech Practice", strApplicationNameSuffix		// (a.walling 2010-07-29 11:32) - PLID 39871 - For now I'll just hardcode NexTech Practice
		);
	}
	else {
		strConnectionString.Format(
			"%s"								// provider (and possibly other options)
			"%s"								// network library
			"Data Source=%s;"					// server
			"Initial Catalog=%s;"				// database
			"Persist Security Info=True;"
			//"Language=us_english;"			// (a.walling 2012-02-09 17:13) - PLID 48101 - We use collation of the db, no point setting this here as far as I can tell
			"Application Name=%s%s;"			// application name
			"User ID=sa;"
			"Password=%s",						// sa password
			GetDataProviderString(),
			GetNetworkLibraryString(),
			GetSqlServerName(),
			strDB,
			"NexTech Practice", strApplicationNameSuffix,		// (a.walling 2010-07-29 11:32) - PLID 39871 - For now I'll just hardcode NexTech Practice
			bIncludePassword ? GetPassword() : ""
		);
	}

	return strConnectionString;
}

HRESULT SyncPacketWait(SOCKET s, PacketType pkttype, PacketType pkttypeWaitingFor, _NxHeader* pHdr, WORD wPktSize)
{
	BOOL bSuccess = FALSE;
	int timeout = 2000; // 2 seconds
	int iRcvSize = 0;
	char rbuf[8192];
	_NxHeader* pIncomingHdr = (_NxHeader*)rbuf;

	pHdr->dwKey = 0;
	pHdr->dwVer = g_dwCurrentNxPacketVersion;
	pHdr->type = pkttype;
	pHdr->wSize = wPktSize;

	/////////////////////////////////////////////////////////////////////////
	// Assign an initial 3 second connection and transmission timeout
	if (setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) != NO_ERROR)
	{
		LogDetail("Failed to set transmission timeout");
		return -1;
	}
	if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) != NO_ERROR)
	{
		LogDetail("Failed to set transmission timeout");
		return -1;
	}

	while (!bSuccess)
	{
		long nRetries = 3;
		while (!bSuccess && nRetries--)
		{
			// (c.haag 2004-06-26) - PLID 13198 - Reset our recv buffer and its size
			char* cbCur = rbuf;
			iRcvSize = 0;

			// Send the packet
			if (send(s, (const char*)pHdr, pHdr->wSize, 0) == SOCKET_ERROR)
			{
				LogDetail("Failed to send data to NexTech Network Server");
				return -1;
			}

			// (c.haag 2004-06-25 11:09) - PLID 13198 - Wait for the entire packet.
			// I'm not going to change the underlying error-control schema; I
			// will probably end up integrating this with NxSocketUtils later.
			BOOL bWait = TRUE;
			while (bWait)
			{
				int n = recv(s, cbCur, 8192 - iRcvSize, 0);
				if (n == SOCKET_ERROR) break; // We failed; so quit
				iRcvSize += n;
				cbCur += n;

				// (c.haag 2004-06-25 11:12) - If we haven't received a full
				// header yet, keep waiting
				if (iRcvSize < sizeof(_NxHeader)) continue;
				// (c.haag 2004-06-25 11:14) - If we're getting the wrong packet,
				// that means we failed, so quit. This is not a good way to handle it,
				// but I will make a better way in a future scope.
				else if (pIncomingHdr->type != pkttypeWaitingFor) break;
				// (c.haag 2004-06-25 11:13) - If we haven't received a full
				// packet yet, keep waiting
				else if (iRcvSize < pIncomingHdr->wSize) continue;

				// (c.haag 2004-06-25 11:15) - Everything worked out fine
				memcpy(pHdr, rbuf, iRcvSize);
				bSuccess = TRUE;
				bWait = FALSE;
			}
		}

		// If this is true, we either got the wrong packet, or got no packet. So, based
		// on what our timeout is, determine how to react.
		if (!bSuccess)
		{
			if (timeout == 2000)
			{
				LogDetail("2 second timeout when trying to communicate with server");

				// Set our new timeout to 10 seconds
				timeout = 10000;
				if (setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) != NO_ERROR)
				{
					LogDetail("Failed to set transmission timeout");
					return -1;
				}
				if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) != NO_ERROR)
				{
					LogDetail("Failed to set transmission timeout");
					return -1;
				}
			}
			else
			{
				if (IDNO == MsgBox(MB_YESNO, "There was a timeout while waiting for information from the server. This is likely an indication of slowness in the network. Would you like to try connecting to the database again?"))
				{
					LogDetail("There was a timeout while waiting for information from the server. This is likely an indication of slowness in the network.");
					return -1;
				}
			}
		}
	}
	return 0;
}


#ifdef _DEBUG
#define DEBUG_NEXTECH_USER
#define DEBUG_SKIP_NXSERVER_VERSION_CHECK
//#define DO_NOT_CONNECT_TO_NXSERVER
#endif

// Returns TRUE on success
BOOL EnsureNxServer(BOOL& bRunTroubleshooter)
{
	try {
		BOOL bNexTechUser = FALSE;
		// (a.walling 2010-06-28 18:00) - PLID 38443 - Differentiate between debug and actual ctrl+shift override
#ifdef _DEBUG
		BOOL bDebugNexTechUser = TRUE;
#endif

		// (c.haag 2006-06-13 08:39) - PLID 20987 - We don't want workstation or
		// user-defined backups to cause a message box to appear in Practice, so we
		// call this function to tell the backup utilities that we only want a message
		// box to appear for server-related backup events.
		NxBackupUtils::SetBackupProgressMsgFlags(BACKUP_FLAG_SERVER);
		
		//Do we have special powers?
		if (GetAsyncKeyState(VK_SHIFT) && GetAsyncKeyState(VK_CONTROL))
		{
			// Back door
			CString strResult;
			while (InputBox(NULL, "Enter the password", strResult, "", true) == IDOK) {
				if (strResult.CompareNoCase("nxpencil") == 0) {
					// Still try to connect to NxServer but if the connection fails, let the user in anyway
					// (a.walling 2010-06-28 18:00) - PLID 38443 - Differentiate between debug and actual ctrl+shift override
					bNexTechUser = TRUE;
#ifdef _DEBUG
					bDebugNexTechUser = FALSE;
#endif
					break;
				} else if (strResult.CompareNoCase("nxcrayon") == 0) {
					// Don't even try to connect to NxServer
					return true;
				} else {
					// Invalid password, keep trying until the user enters a correct password or hits cancel
					MsgBox(MB_ICONEXCLAMATION|MB_OK, 
						"The password you entered was incorrect.  Please enter the correct "
						"password, or click Cancel to proceed without a password.");
				}
			}
		}

		// (a.walling 2010-06-28 14:42) - PLID 38442 - Get rid of this, just use the one from the globalutils header
		//void SendErrorEmail(CWnd *pParent, const CString &strErrorInfo);

	#ifdef DO_NOT_CONNECT_TO_NXSERVER
		return;
	#endif

		CString strNxServerIP = NxRegUtils::ReadString(GetRegistryBase() + "NxServerIP");
		//SOCKET s;
		//SOCKADDR_IN namesock;
		char szBkpInfo[8192], szDocTmpInfo[1024], szBackupCfg[1024];
		void* pResult;
		DWORD nResultSize;
		_NxHeader* pInfoHdr = (_NxHeader*)szBkpInfo;
		_NxHeader* pDocTmpHdr = (_NxHeader*)szDocTmpInfo;
		_NxHeader* pBkpCfgHdr = (_NxHeader*)szBackupCfg;
		_PACKET_SERVER_VERSION pktQuery;
		int timeout = 10000; // 10 seconds
		BOOL bRetry = FALSE;

#ifdef DEBUG_NEXTECH_USER
		bNexTechUser = TRUE;
		bDebugNexTechUser = FALSE;
#endif

		// Initialize the Winsock subsystem
		if (TCP_Init(WM_TCPIP))
		{
			AfxThrowNxException(
				"Could not initialize TCP/IP framework. Please ensure your network "
				"drivers are installed correctly.");
			return FALSE;
		}

		do {
			NxSocketUtils::HCLIENT hNxServer;

			try {
				 hNxServer = NxSocketUtils::Connect(NULL, strNxServerIP, 11983);
				//NxSocketUtils::SetSendTimeout(hNxServer, timeout);
				//NxSocketUtils::SetRecvTimeout(hNxServer, timeout);

			} catch(CNxException *e) {
				//DRT 10/4/2005 - PLID 17786 - NxSocketUtils now throw NxExceptions themselves instead of letting the
				//	exception rise up to us.  If we get an exception while trying to connect, 
				//	look for 10061 - Connection refused.  This is a potentially common error, which means that the
				//	NxServer is inaccessible - It is turned off, firewalled, etc.  We can provide a better error message
				//	here.
				//TODO:  This is shady and I don't really like it.  For some reason calling WSAGetLastError() returns 0 here.  I'm not sure why, 
				//	as the documentation claims it won't reset on a successful function, nor will it reset from someone calling WSAGetLastError().  So
				//	therefore we have to take the error message (which we know from testing does get the error code), and parse out the error code.
				if(e->m_strDescription.Right(6) == " 10061") {
					//Warn the user in a "special" manner.
					CString strMsg;
					strMsg.Format("Your connection to the NexTech Network Service (at %s) has been refused.  This may be because the service has been stopped "
						"on the server, a firewall is blocking your connection attempts, or the server has been turned off.", strNxServerIP);
					if (IDYES == MsgBox(MB_YESNO, strMsg + "\r\n\r\nWould you like to e-mail NexTech for support?"))	{
						SendErrorEmail(GetMainFrame(), strMsg);
					}

					e->Delete();
					return FALSE;
				}
				else {
					//Unknown case, just throw it out to the usual
					throw e;
				}
			}

			// (z.manning, 03/02/2007) - PLID 25160 - Until a change to SyncPacketWait in 8300, the timeout
			// for this overload of SyncPacketWait used to be forever by default whereas it is now 5 secs by
			// default. It is entirely possible that getting backup info can take longer than 5 sec
			// if either the workstation or user-defined paths are invalid UNC paths, for example. So I changed
			// the timeout to a much longer time because
			//		1. That's the way it used to be.
			//		2. It's much better to have a slow login than not being able to at all.
			//		3. If there's a problem with the connection to NxServer, we most likely will never get here.
			//
			const int nNxServerPacketTimeout = 60000;

			// Make sure the versions of Practice and NxServer match
			//
			pktQuery.bQuery = 0;
			// (z.manning, 03/12/2007) - PLID 25160 - Increased the timeout.
			if (NxSocketUtils::SyncPacketWait(hNxServer, PACKET_TYPE_SERVER_VERSION, PACKET_TYPE_SERVER_VERSION, &pktQuery, sizeof(_PACKET_SERVER_VERSION), pResult, nResultSize, nNxServerPacketTimeout)) {
				// (c.haag 2005-10-25 11:15) - PLID 18070 - Fixed bug where we were checking the wrong string
				// for the version.
				
#ifndef DEBUG_SKIP_NXSERVER_VERSION_CHECK
				const char* szRemoteVerString = (const char*)((_PACKET_SERVER_VERSION*)pResult)->szVerString;
				if (( strcmp(szRemoteVerString, NXSERVER_VERSION)) && !bNexTechUser)
				{
					CString str;
					str.Format("Your version of the NexTech Network Service (%s) is out of date with this program (%s). This could mean your workstation has not been properly upgraded with the last version of Practice. Would you like to e-mail NexTech for support?", szRemoteVerString, NXSERVER_VERSION);
					if (IDYES == MsgBox(MB_YESNO, str))	{
						str.Format("Your version of the NexTech Network Service (%s) is out of date with this program (%s). This could mean your workstation has not been properly upgraded with the last version of Practice.", szRemoteVerString, NXSERVER_VERSION);
						SendErrorEmail(GetMainFrame(), str);
					}
					NxSocketUtils::Disconnect(hNxServer);
					return FALSE;
				}
#endif
			} else {
				NxSocketUtils::Disconnect(hNxServer);
				AfxThrowNxException("Could not query version of NexTech Network Service");	
			}

			//
			// (c.haag 2005-09-30 10:49) - PLID 17585 - Make sure NxServer associates our connection
			// with our database so backup queries are directed toward the right database
			//
			CString strDefaultDatabase = (LPCTSTR)GetRemoteData()->GetDefaultDatabase();
			NxSocketUtils::Send(hNxServer, PACKET_TYPE_DATABASE_NAME, (const void*)(LPCTSTR)strDefaultDatabase, strDefaultDatabase.GetLength()+1);

			//
			// See if the backup is running
			//
			// (z.manning, 03/12/2007) - PLID 25160 - Increased the timeout.
			if (!NxSocketUtils::SyncPacketWait(hNxServer, PACKET_TYPE_IS_BACKUP_RUNNING, PACKET_TYPE_IS_BACKUP_RUNNING, NULL, 0, pResult, nResultSize, nNxServerPacketTimeout))
			{
				if (!bNexTechUser) {
					NxSocketUtils::Disconnect(hNxServer);
					AfxThrowNxException("Could not determine if a backup is running from the NexTech Network Server");
					return FALSE;
				} else {
					MsgBox("Practice could not determine if a backup is running, but NexTech power users are allowed to continue with the login.");
				}
			}
			if (((_PACKET_BKP_IS_RUNNING*)pResult)->bIsRunning) {
				// (a.walling 2010-06-04 13:17) - PLID 28444 - We can continue during a backup (unless it is critical)
				// (a.walling 2010-08-23 12:31) - PLID 28444 - Disconnecting will invalidate the temporary memory buffer used by the socket!
				BOOL bIsRunning = ((_PACKET_BKP_IS_RUNNING*)pResult)->bIsRunning;
				NxSocketUtils::Disconnect(hNxServer);
				if (bIsRunning == TRUE) {
					MsgBox(MB_ICONINFORMATION, "A backup is currently in progress. You may continue using the system normally during this process.");
					return TRUE;
				} else {
					MsgBox("A critical backup is currently in progress. Please try running NexTech Practice again in a few minutes.");
					return FALSE;
				}
			}


			//
			// (c.haag 2005-12-08 17:36) - PLID 6908 - Get a status on the integrity of the previous master.
			// database. If there are any problems, we need to report it to the user
			//
			NxSocketUtils::Send(hNxServer, PACKET_TYPE_DATABASE_NAME, (const void*)"master", strlen("master")+1);
			// (z.manning, 03/12/2007) - PLID 25160 - Increased the timeout.
			if (!NxSocketUtils::SyncPacketWait(hNxServer, PACKET_TYPE_DB_INTEGRITY_QUERY, PACKET_TYPE_DB_INTEGRITY_QUERY, NULL, 0, pResult, nResultSize, nNxServerPacketTimeout)) {
				NxSocketUtils::Disconnect(hNxServer);
				AfxThrowNxException("Could not get backup integrity information from the NexTech Network Server");
			} else {
				Backup_OnDBIntegrityStatus((const _DB_INTEGRITY_RESPONSE*)pResult, "master");
			}

			//
			// (c.haag 2005-12-08 17:36) - PLID 6908 - Get a status on the integrity of the current Practice data.
			// If the integrity check failed, or the database integrity has been compromised, we need to report
			// back to the user
			//
			NxSocketUtils::Send(hNxServer, PACKET_TYPE_DATABASE_NAME, (const void*)(LPCTSTR)strDefaultDatabase, strDefaultDatabase.GetLength()+1);
			// (z.manning, 03/12/2007) - PLID 25160 - Increased the timeout.
			if (!NxSocketUtils::SyncPacketWait(hNxServer, PACKET_TYPE_DB_INTEGRITY_QUERY, PACKET_TYPE_DB_INTEGRITY_QUERY, NULL, 0, pResult, nResultSize, nNxServerPacketTimeout)) {
				NxSocketUtils::Disconnect(hNxServer);
				AfxThrowNxException("Could not get backup integrity information from the NexTech Network Server");
			} else {
				Backup_OnDBIntegrityStatus((const _DB_INTEGRITY_RESPONSE*)pResult, strDefaultDatabase);
			}

			// (c.haag 2006-04-18 16:39) - PLID 19460 - Get the backup user list. We may use it later to calculate if
			// we can do a backup
			//
			// (z.manning, 03/12/2007) - PLID 25160 - Increased the timeout.
			if (!NxSocketUtils::SyncPacketWait(hNxServer, PACKET_TYPE_BACKUP_USER_CFG_QUERY, PACKET_TYPE_BACKUP_USER_CFG, NULL, 0, pResult, nResultSize, nNxServerPacketTimeout)) {
				NxSocketUtils::Disconnect(hNxServer);
				AfxThrowNxException("Could not get backup permission information from the NexTech Network Server");
			} else {
				Backup_OnBackupUserCfg((const void*)pResult, nResultSize);
			}

			//
			// Get the error text of the previous backup, and process it first. We may need it when we go in
			// Backup_OnDBInfoMessage(). 
			//
			// (z.manning, 03/12/2007) - PLID 25160 - Increased the timeout.
			if (!NxSocketUtils::SyncPacketWait(hNxServer, PACKET_TYPE_BKP_ERROR_TEXT, PACKET_TYPE_BKP_ERROR_TEXT, NULL, 0, pResult, nResultSize, nNxServerPacketTimeout)) {
				NxSocketUtils::Disconnect(hNxServer);
				AfxThrowNxException("Could not get backup information from the NexTech Network Server (error text)");
			} else {
				Backup_OnBackupErrorTextMessage((const char*)pResult, nResultSize);
			}

			//
			// Get backup and configuration information
			//

			// Current backup status (includes some settings -- this is for backwards compatibility)
			//
			// (z.manning, 03/12/2007) - PLID 24175 - Increased the timeout.
			if (!NxSocketUtils::SyncPacketWait(hNxServer, PACKET_TYPE_BKP_STATUS_REQUEST, PACKET_TYPE_BKP_STATUS, NULL, 0, pResult, nResultSize, nNxServerPacketTimeout)) {
				NxSocketUtils::Disconnect(hNxServer);
				AfxThrowNxException("Could not get backup information from the NexTech Network Server (status request)");
			} else {
				memcpy(szBkpInfo, pResult, nResultSize);
			}

			// Document and template backup configuration
			//
			// (z.manning, 03/12/2007) - PLID 25160 - Increased the timeout.
			if (!NxSocketUtils::SyncPacketWait(hNxServer, PACKET_TYPE_DOCTEMPLATE_CONFIG_REQUEST, PACKET_TYPE_DOCTEMPLATE_CONFIG, NULL, 0, pResult, nResultSize, nNxServerPacketTimeout)) {
				NxSocketUtils::Disconnect(hNxServer);
				AfxThrowNxException("Could not get backup information from the NexTech Network Server (documents/templates config)");
			} else {
				memcpy(szDocTmpInfo, pResult, nResultSize);
			}

			// Data backup configuration
			//
			// (z.manning, 03/12/2007) - PLID 25160 - Increased the timeout.
			if (!NxSocketUtils::SyncPacketWait(hNxServer, PACKET_TYPE_DATABKP_CONFIG_REQUEST, PACKET_TYPE_DATABKP_CONFIG, NULL, 0, pResult, nResultSize, nNxServerPacketTimeout)) {
				NxSocketUtils::Disconnect(hNxServer);
				AfxThrowNxException("Could not get backup information from the NexTech Network Server (data config)");
			} else {
				memcpy(szBackupCfg, pResult, nResultSize);
			}

			//
			// Socket cleanup
			//
			NxSocketUtils::Disconnect(hNxServer);

			//
			// Post-cleanup backup message processing
			//
			Backup_OnBackupMessage(bNexTechUser, (_BACKUP_STATS*)szBkpInfo);

			// (a.walling 2010-06-28 18:00) - PLID 38443 - Differentiate between debug and actual ctrl+shift override
#ifdef _DEBUG
			bRetry = Backup_OnDBInfoMessage((bNexTechUser && bDebugNexTechUser) ? TRUE : FALSE, (_BACKUP_STATS*)szBkpInfo,
				(_BACKUP_DOCTEMPLATE_CONFIG*)szDocTmpInfo, (_BACKUP_DATABKP_CONFIG*)szBackupCfg);
#else
			bRetry = Backup_OnDBInfoMessage(bNexTechUser, (_BACKUP_STATS*)szBkpInfo,
				(_BACKUP_DOCTEMPLATE_CONFIG*)szDocTmpInfo, (_BACKUP_DATABKP_CONFIG*)szBackupCfg);
#endif
		} while (bRetry);

		return TRUE;
	}NxCatchAll("Error in EnsureNxServer()");
	bRunTroubleshooter = TRUE;
	return FALSE;
}

BOOL EnsureLicenseServer(BOOL& bRunTroubleshooter)
{
	try {
		try {
			if(g_pLicense) {//Possibly an earlier, failed attempt to connect.
				delete g_pLicense;
				g_pLicense = NULL;
			}

			// (a.walling 2011-04-13 09:40) - PLID 43308 - Send the official server name
			// (j.armen 2011-10-24 13:58) - PLID 46139 - GetPracPath should reference the MDE path
			// (z.manning 2015-05-20 10:23) - PLID 65971 - Changed to CPracticeLicense
			// (j.jones 2015-12-16 09:44) - PLID 67723 - removed official server name, now the license server calculates it
			g_pLicense = new CPracticeLicense(CLicense::GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT));
			// (r.gonet 2016-05-19 18:21) - NX-100689 - The property manager now needs to know certain license information.
			g_propManager.SetLicense(g_pLicense);
			//DRT 6/16/2006 - PLID 21038 - Now must call a 1 time initialization of the
			//	license before use.
			if (!g_pLicense->Initialize())
				return FALSE;
			// (a.walling 2007-03-26 14:06) - PLID 24434 - warning the user is handled inside the initialize function.

			if(!g_pLicense->IsValid()) {
				// (z.manning, 01/23/2007) - 24382 - Before we tell them they are on the wrong version
				// of the license server, we should try to update the license to see if that fixes it
				// because if they ever updated the license with mismatched versions of Practice and the
				// license server, the license info will be very messed up until it gets updated when
				// the versions are in sync.
				// (First try a silent update, and then if that fails, bring up the update code dialog.)
				if(!g_pLicense->TryToUpdateLicense()) {
					g_pLicense->PromptForUpdateCode(FALSE);
				}

				if(!g_pLicense->IsValid()) {
					//DRT 1/22/2007 - PLID 24370 - If the license server is not valid, put up a message box informing
					//	the user that we did connect, but it was not the right version.
					AfxMessageBox("NexTech Practice was able to connect to your license server, but found that the License Server was not the correct version.  "
						"Please ensure that your server is properly upgraded.");
					return FALSE;
				}
			}
			if (g_pLicense->CheckForLicense(CLicense::lcPractice, CLicense::cflrUse)) {
				return TRUE;
			} else {
				// (b.cardillo 2005-06-29 15:35) - PLID 16881 - Either the user didn't have a 
				// license at all, or the license was a trial and has now run out (either it 
				// expired or the usage limit has been reached).  If it was a trial and has 
				// run out, then the CheckForLicense function has already given the message, 
				// so we shouldn't give another one.  But if the user has no license for this 
				// at all, then no message has been given so we really should.  But the thing 
				// is, if the user doesn't have a license for Practice itself, there is some-
				// thing seriously wrong.  So our message has to be sort of middle-of-the-
				// road, because if they just received a message indicating they have run out 
				// of uses of their trial, they don't need some crazy error-looking message.
				AfxMessageBox("You are not licensed to run Practice, or your license is not active.  Please contact NexTech for assistance.", MB_ICONHAND|MB_OK);
				return FALSE;
			}	
		} catch(CNxException *e) {
			// (j.jones 2005-10-12 08:41) - PLID 17862 - If we get an exception while trying to connect, 
			//	look for 10061 - Connection refused.  This is a potentially common error, which means that the
			//	service is inaccessible - It is turned off, firewalled, etc.  We can provide a better error message
			//	here.
			// DRT: TODO:  This is shady and I don't really like it.  For some reason calling WSAGetLastError() returns 0 here.  I'm not sure why, 
			//	as the documentation claims it won't reset on a successful function, nor will it reset from someone calling WSAGetLastError().  So
			//	therefore we have to take the error message (which we know from testing does get the error code), and parse out the error code.
			if(e->m_strDescription.Right(6) == " 10061") {
				//Warn the user in a "special" manner.
				CString strMsg;
				strMsg.Format("Your connection to the NexTech License Server (at %s) has been refused.  This may be because the service has been stopped "
					"on the server, a firewall is blocking your connection attempts, or the server has been turned off.", NxRegUtils::ReadString(GetRegistryBase() + "NxServerIP"));
				if (IDYES == MsgBox(MB_YESNO, strMsg + "\r\n\r\nWould you like to e-mail NexTech for support?"))	{
					SendErrorEmail(GetMainFrame(), strMsg);
				}

				e->Delete();
				return FALSE;
			}
			else {
				//Unknown case, just throw it out to the usual
				throw e;
			}
		}
	}NxCatchAll("Error in EnsureLicenseServer()");

	bRunTroubleshooter = TRUE;
	return FALSE;
}

// (a.walling 2012-02-09 17:13) - PLID 48099 - NxAdo overridable user interface
class PracticeNxAdoUserInterface : public NxAdo::DefaultUserInterface
{
public:	
	virtual void OnLogString(LPCTSTR szText)
	{
		NxAdo::DefaultUserInterface::OnLogString(szText);

		Log("%s\r\n", szText);
	}
};

PracticeNxAdoUserInterface practiceNxAdoUserInterface;

//Ensures remote data has been opened once, and that our datasource is correct
//cannot throw anything, as we have to exit if we aren't docked.
bool EnsureSqlServer(BOOL& bRunTroubleshooter)
{
	// From now on, we can't allow running practice 
	// without having access to the sql data
	try 
	{
		// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification
		CString strDB;
		if(GetSubRegistryKey().IsEmpty())
			strDB = "PracData";
		else 
			strDB.Format("PracData_%s", GetSubRegistryKey());

		
		CString strDataProvider = NxRegUtils::ReadString(
			_T(GetRegistryBase() + "DataProvider"), "SQLOLEDB");

		CString strRegistryLocation;
		strRegistryLocation.Format("HKEY_CLASSES_ROOT\\%s", strDataProvider);
		if (strDataProvider.IsEmpty() || (strDataProvider.CompareNoCase("SQLOLEDB") != 0 && !NxRegUtils::DoesKeyExist(strRegistryLocation)) ) {
			strDataProvider = "SQLOLEDB";
		}

				//First, try loading from the registry
		CString strConnectionType = NxRegUtils::ReadString(
			_T(GetRegistryBase() + "ConnectionType"));

		BOOL bIsLan = TRUE;

		if (strConnectionType == "IP")
		{
			bIsLan = FALSE;
		}

		// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification
		// (b.savon 2016-05-18 15:18) - NX-100657 - Check the flag to use Windows Auth for SQL connection
		GetRemoteConnection()
			.Init(GetSqlServerName(), bIsLan, strDB, NxConnectionUtils::UseSqlIntegratedSecurity(GetSubRegistryKey()), GetCommandTimeout(), GetConnectionTimeout(), strDataProvider)
			.EnableCommandCache();

		// (a.walling 2012-02-09 17:13) - PLID 48099 - NxAdo overridable user interface
		NxAdo::UserInterface::SetUserInterface(&practiceNxAdoUserInterface);

		NxAdo::UserInterface::EnableWarnMaxRecordsExceeded();
		NxAdo::UserInterface::EnableWarnPerformance();

		// First, ensure the data
		EnsureRemoteData();

		g_propManager.SetRemoteDataPointer(GetRemoteData());
	} 
	catch (_com_error& e) 
	{
		if (e.Error() == E_FAIL) 
		{
			MsgBox(MB_ICONEXCLAMATION|MB_OK,
				"Your data is not accessible right now.  Please make "
				"sure you are properly attached to the network.");
			bRunTroubleshooter = TRUE;
			return false;
		}
		else HandleException(e, "EnsureSqlServer Error 1", __LINE__, __FILE__);
		bRunTroubleshooter = TRUE;
		return false;
	} 
	catch (CException *e) 
	{
		HandleException(e, "EnsureSqlServer Error 2", __LINE__, __FILE__);
		return false;
	}
	return true;
}

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - lots of stuff moved to NxAdo

// (a.walling 2010-06-02 07:49) - PLID 31316 - We really only need one critical section for all this, not 4. Just use l_csConnectionProperties
//CCriticalSection l_csGetCommandTimeout; // (c.haag 2010-02-24 11:59) - PLID 37526 - Protect this connection substring with a critical section
long GetCommandTimeout()
{
	CSingleLock cs(&l_csConnectionProperties, TRUE);
	// (a.walling 2010-02-09 09:49) - PLID 37277 - Use a default param. Also modified this to take advantage of static variables.
	// Is 47 seconds really the default time to wait?
	static long l_nCommandTimeout = NxRegUtils::ReadLong(_T(GetRegistryBase() + "CommandTimeout"), 47);
	return l_nCommandTimeout;
}

// (a.walling 2010-06-02 07:49) - PLID 31316 - We really only need one critical section for all this, not 4. Just use l_csConnectionProperties
//CCriticalSection l_csGetConnectionTimeout; // (c.haag 2010-02-24 11:59) - PLID 37526 - Protect this connection substring with a critical section
long GetConnectionTimeout()
{
	CSingleLock cs(&l_csConnectionProperties, TRUE);
	// (a.walling 2010-02-09 09:49) - PLID 37277 - Use a default param. Also modified this to take advantage of static variables.
	static long l_nConnectionTimeout = NxRegUtils::ReadLong(_T(GetRegistryBase() + "ConnectionTimeout"), 60); // default
	return l_nConnectionTimeout;
}

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - lots of stuff moved to NxAdo

#define NX_DEBUG_BREAK() try { AfxDebugBreak(); } catch(...) { };

const long cnCommonError___No_More_Results = 265929;

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - lots of stuff moved to NxAdo

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ConfigRT/ConfigT stuff

_RecordsetPtr g_rsConfigRT;
CNxPropManager g_propManager;

void EnsureConfigRT()
{
	g_propManager.EnsureConfigRT();
	if (g_rsConfigRT == NULL)
	{
		g_rsConfigRT = CreateRecordset (adOpenStatic, 
										adLockReadOnly, 
			"SELECT UserName, Name, Number, TextParam, MemoParam, IntParam, FloatParam, DateTimeParam, ImageParam "
			"FROM ConfigRT");
		g_rsConfigRT->PutRefActiveConnection(NULL);//disconnect
	}
}


// Returns the property value in the form of a variant
COleVariant GetProperty(LPCTSTR strPropName, PropertyType nPropType, COleVariant *varDefault /* = 0 */, int nPropNum /* = 0 */, bool bAutoCreate /*= true*/)
{
	return g_propManager.GetProperty(strPropName, nPropType, varDefault, nPropNum, bAutoCreate);
/*	// Since we used to pull this from the ConfigT on the local machine, 
	// we now pull it from the ConfigRT giving our machine name as the user
	// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
	// than the license object.
	CString strUserParam = g_propManager.GetSystemName() + '.' + GetPracPath();
	return GetRemoteProperty(strPropName, nPropType, varDefault, nPropNum, strUserParam, bAutoCreate);*/
}

// Sets the property value in the form of a variant
// A return value of zero indicates success, otherwise an error code
long SetProperty(LPCTSTR strPropName, PropertyType nPropType, COleVariant varNewValue, int nPropNum /* = 0 */)
{
	return g_propManager.SetProperty(strPropName, nPropType, varNewValue, nPropNum);
/*	// Since we used to pull this from the ConfigT on the local machine, 
	// we now pull it from the ConfigRT giving our machine name as the user
	// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
	// than the license object.
	CString strUserParam = g_propManager.GetSystemName() + '.' + GetPracPath();
	return SetRemoteProperty(strPropName, nPropType, varNewValue, nPropNum, strUserParam);*/
}

CString GetPropertyText(LPCTSTR strPropName, LPCTSTR strDefault /* = 0 */, int nPropNum /* = 0 */, bool bAutoCreate /*= true*/)
{
	return g_propManager.GetPropertyText(strPropName, strDefault, nPropNum, bAutoCreate);
	/*
	COleVariant tmpVar;
	
	// Pass pointer to default variant only if a default value was passed
	if (strDefault) {
		COleVariant varDefault(strDefault);
		tmpVar = GetProperty(strPropName, propText, &varDefault, nPropNum, bAutoCreate);
	} else {
		tmpVar = GetProperty(strPropName, propText, 0, nPropNum, bAutoCreate);
	}
	
	// If null was returned, return a null-like value, otherwise, return the value
	if (tmpVar.vt == VT_NULL) {
		return CString("");
	} else {
		return (LPCTSTR)_bstr_t(tmpVar);
	}*/
}

CString GetPropertyMemo(LPCTSTR strPropName, LPCTSTR strDefault /* = 0 */, int nPropNum /* = 0 */, bool bAutoCreate /*= true*/)
{
	return g_propManager.GetPropertyMemo(strPropName, strDefault, nPropNum, bAutoCreate);
	/*
	COleVariant tmpVar;
	
	// Pass pointer to default variant only if a default value was passed
	if (strDefault) {
		COleVariant varDefault(strDefault);
		tmpVar = GetProperty(strPropName, propMemo, &varDefault, nPropNum, bAutoCreate);
	} else {
		tmpVar = GetProperty(strPropName, propMemo, 0, nPropNum, bAutoCreate);
	}
	
	// If null was returned, return a null-like value, otherwise, return the value
	if (tmpVar.vt == VT_NULL) {
		return CString("");
	}
	else {
		return (LPCTSTR)_bstr_t(tmpVar);
	}*/
}

long GetPropertyInt(LPCTSTR strPropName, long nDefault /* = LONG_MIN + 1 */, int nPropNum /* = 0 */, bool bAutoCreate /*= true*/)
{
	return g_propManager.GetPropertyInt(strPropName, nDefault, nPropNum, bAutoCreate);
	/*
	COleVariant tmpVar;
	
	// Pass pointer to default variant only if a default value was passed
	if (nDefault != LONG_MIN + 1) {
		COleVariant varDefault(nDefault, VT_I4);
		tmpVar = GetProperty(strPropName, propNumber, &varDefault, nPropNum, bAutoCreate);
	} else {
		tmpVar = GetProperty(strPropName, propNumber, 0, nPropNum, bAutoCreate);
	}
	
	// If null was returned, return a null-like value, otherwise, return the value
	if (tmpVar.vt == VT_NULL) {
		return 0L;
	}
	else {
		return tmpVar.lVal;
	}*/
}

float GetPropertyFloat(LPCTSTR strPropName, float *pfltDefault /* = 0 */, int nPropNum /* = 0 */, bool bAutoCreate /*= true*/)
{
	return g_propManager.GetPropertyFloat(strPropName, pfltDefault, nPropNum, bAutoCreate);
/*	COleVariant tmpVar;
	
	// Pass pointer to default variant only if a default value was passed
	if (pfltDefault) {
		COleVariant varDefault(*pfltDefault);
		tmpVar = GetProperty(strPropName, propFloat, &varDefault, nPropNum, bAutoCreate);
	} else {
		tmpVar = GetProperty(strPropName, propFloat, 0, nPropNum, bAutoCreate);
	}
	
	// If null was returned, return a null-like value, otherwise, return the value
	if (tmpVar.vt == VT_NULL) {
		return 0.0;
	}
	else {
		return tmpVar.fltVal;
	}*/
}

COleDateTime GetPropertyDateTime(LPCTSTR strPropName, COleDateTime *pdtDefault /* = 0 */, int nPropNum /* = 0 */, bool bAutoCreate /*= true*/)
{
	return g_propManager.GetPropertyDateTime(strPropName, pdtDefault, nPropNum, bAutoCreate);
/*	COleVariant tmpVar;
	
	// Pass pointer to default variant only if a default value was passed
	if (pdtDefault) {
		COleVariant varDefault(*pdtDefault);
		tmpVar = GetProperty(strPropName, propDateTime, &varDefault, nPropNum, bAutoCreate);
	} else {
		tmpVar = GetProperty(strPropName, propDateTime, 0, nPropNum, bAutoCreate);
	}
	
	// If null was returned, return a null-like value, otherwise, return the value
	COleDateTime Ans;
	ASSERT(tmpVar.vt == VT_NULL || tmpVar.vt == VT_EMPTY || tmpVar.vt == VT_DATE);
	switch (tmpVar.vt) {
	case VT_NULL:
	case VT_EMPTY:
		// Nothing to return
		Ans.SetStatus(COleDateTime::invalid);
		break;
	case VT_DATE:
		// Got the date, just return it
		Ans = tmpVar.date;
		break;
	default:
		// Unexpected type
		ThrowNxException("GetPropertyDateTime: Unexpected return value type of %li!", tmpVar.vt);
		break;
	}
	return Ans;*/
}

long SetPropertyText(LPCTSTR strPropName, LPCTSTR strNewValue, int nPropNum /* = 0 */)
{
	return g_propManager.SetPropertyText(strPropName, strNewValue, nPropNum);
	//return SetProperty(strPropName, propText, COleVariant(strNewValue, VT_BSTR), nPropNum);
}

long SetPropertyMemo(LPCTSTR strPropName, LPCTSTR strNewValue, int nPropNum /* = 0 */)
{
	return g_propManager.SetPropertyMemo(strPropName, strNewValue, nPropNum);
	//return SetProperty(strPropName, propMemo, COleVariant(strNewValue, VT_BSTR), nPropNum);
}

long SetPropertyInt(LPCTSTR strPropName, long nNewValue, int nPropNum /* = 0 */)
{
	return g_propManager.SetPropertyInt(strPropName, nNewValue, nPropNum);
	//return SetProperty(strPropName, propNumber, COleVariant(nNewValue, VT_I4), nPropNum);
}

long SetPropertyFloat(LPCTSTR strPropName, float fltNewValue, int nPropNum /* = 0 */)
{
	return g_propManager.SetPropertyFloat(strPropName, fltNewValue, nPropNum);
	//return SetProperty(strPropName, propFloat, COleVariant(fltNewValue), nPropNum);
}

long SetPropertyDateTime(LPCTSTR strPropName, COleDateTime dtNewValue, int nPropNum /* = 0 */)
{
	return g_propManager.SetPropertyDateTime(strPropName, dtNewValue, nPropNum);
	//return SetProperty(strPropName, propDateTime, COleVariant(dtNewValue), nPropNum);
}

// Remote properties
// Returns the property value in the form of a variant
// Done the old way, opening a recordset each time
//COleVariant GetRemotePropertySlow(LPCTSTR strPropName, PropertyType nPropType, COleVariant *varDefault /* = 0 */, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */, bool bAutoCreate /* = true */)
/*{
	try {
		// Make sure we can get to the data
		EnsureRemoteData();

		// Decide which field we care about
		CString strFldName;		
		switch (nPropType) {
			case propText:			strFldName = _T("TextParam"); break;
			case propMemo:			strFldName = _T("MemoParam"); break;
			case propNumber:		strFldName = _T("IntParam"); break;
			case propFloat:		strFldName = _T("FloatParam"); break;
			case propDateTime:	strFldName = _T("DateTimeParam"); break;
			case propImage:		strFldName = _T("ImageParam"); break;
			default:
				// Give an error and return failure
				HandleException(NULL, "Unexpected property type given in SetRemoteProperty", __LINE__, __FILE__);
				COleVariant Ans;
				Ans.Clear();
				return Ans;
				break;
		}
		
		// Generate the username that will be used for the queries
		//DRT 9/18/03 - Removed _Q() from around strUserName.  You should not use _Q() unless you are accessing the data
		//	where you are using it.  In this case, it is using it here, then using it again in the select, which doubles the
		//	quotations and screws up the data retrieval
		CString strUser = strUsername ? strUsername : _T("<None>");

		// Open a recordset requesting that field
		_RecordsetPtr prs;
		prs = CreateRecordset(
			"SELECT %s FROM ConfigRT WHERE Username = '%s' AND Name = '%s' AND Number = %d", 
			_Q(strFldName), _Q(strUser), _Q(strPropName), nPropNum);
		
		// Make sure we're on a record
		if ((prs != NULL) && (!prs->eof)) {
			// We're on a record so return its value
			return prs->Fields->Item[(long)0]->Value;
		} else if (varDefault) {
			// There wasn't a record so create one if we want to
			if (bAutoCreate) {
				SetRemoteProperty(strPropName, nPropType, *varDefault, nPropNum, strUsername);
			}
			return *varDefault;
		} else {
			// Return empty variant
			COleVariant varAns;
			varAns.Clear();
			return varAns;
		}
	} NxCatchAllCall("GetRemoteProperty Error 500", {
		// After reporting the exception, return an empty variant (ideally we would want to 
		// return an invalid variant, but I don't want to do this because what if someone 
		// is relying an en empty variant, which is what this function has always returned 
		// on exception)
		COleVariant varAns;
		varAns.Clear();
		return varAns;
	});
}*/

// Remote properties
// Returns the property value in the form of a variant
//without opening a recorset each time
//COleVariant GetRemotePropertyFast(LPCTSTR strPropName, PropertyType nPropType, COleVariant *varDefault /* = 0 */, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */, bool bAutoCreate /* = true */)
/*{
	CString strFldName;		
	FieldsPtr	fields;
	FieldPtr	fUser, 
				fName,
				fNumber;

	try 
	{
		// Make sure we can get to the data
		EnsureRemoteData();
		EnsureConfigRT();

		// Decide which field we care about
		switch (nPropType) 
		{
			case propText:			
				strFldName = _T("TextParam"); 
				break;
			case propMemo:			
				strFldName = _T("MemoParam"); 
				break;
			case propNumber:		
				strFldName = _T("IntParam"); 
				break;
			case propFloat:		
				strFldName = _T("FloatParam"); 
				break;
			case propDateTime:	
				strFldName = _T("DateTimeParam"); 
				break;
			case propImage:
				strFldName = _T("ImageParam");
				break;
			default:
				// Give an error and return failure
				HandleException(NULL, "Unexpected property type given in SetRemoteProperty", __LINE__, __FILE__);
				COleVariant Ans;
				Ans.Clear();
				return Ans;
				break;
		}

		//loop through all records
		g_rsConfigRT->MoveFirst();

		fields = g_rsConfigRT->Fields;

		fUser	= fields->Item["Username"];
		fName	= fields->Item["Name"];
		fNumber	= fields->Item["Number"];

		if (strUsername) 
		{
			while (!g_rsConfigRT->eof)
			{	if (VarString(fUser->Value) == strUsername 
					&& VarString(fName->Value) == strPropName
					&& VarShort(fNumber->Value) == nPropNum) 
				{
					return fields->Item[_bstr_t(strFldName)]->Value;
				}
				
				g_rsConfigRT->MoveNext();
			}
		}
		else
		{
			while (!g_rsConfigRT->eof)
			{	if (VarString(fName->Value) == strPropName && VarShort(fNumber->Value) == nPropNum)
					return fields->Item[_bstr_t(strFldName)]->Value;
				g_rsConfigRT->MoveNext();
			}
		}

		//item not found
		if (varDefault)
		{	if (bAutoCreate)
				SetRemoteProperty(strPropName, nPropType, *varDefault, nPropNum, strUsername);
			return *varDefault;
		}
	} NxCatchAll("Could not Get Remote Property");

	// Return failure
	COleVariant Ans;
	Ans.Clear();
	return Ans;
}*/

COleVariant GetRemoteProperty(LPCTSTR strPropName, PropertyType nPropType, COleVariant *varDefault /* = 0 */, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */, bool bAutoCreate /* = true */)
{
	return g_propManager.GetRemoteProperty(strPropName, nPropType, varDefault, nPropNum, strUsername, bAutoCreate);
/*#ifdef SLOW_REMOTE_PROPERTY
	return GetRemotePropertySlow(strPropName, nPropType, varDefault, nPropNum, strUsername, bAutoCreate);
#else
	return GetRemotePropertyFast(strPropName, nPropType, varDefault, nPropNum, strUsername, bAutoCreate);
#endif*/
}

void ParseCommaDeliminatedText(CArray<int,int> &ary, const char *p)
//BVB - I take no responsibility for this bad design, RAC told me explicitly to do it this way
//DRT - 10/5/01 - Rewrote this function to read the stuff correctly, previously we read 1 char out of the string at a time ... 
//ignoring the fact that many id's are 2 digits, or possibly 3 ... among other problems it had
{
	long size = strlen(p), comma;
	CString temp;
	temp = p;

	while(temp != "")
	{
		comma = temp.Find(',');
		
		//if we have no comma, we must have 1 more value in the string
		if(comma != -1)
		{
			ary.Add(atoi(temp.Left(comma)));
			temp = temp.Right(temp.GetLength() - temp.Left(comma+1).GetLength());
		}
		else
		{
			ary.Add(atoi(temp));
			temp = "";
		}
		
		
	}
}

CString GetCommaDeliminatedText(CArray<int,int> &ary)
{//DRT 10/5/01 - Fixed this so that it no longer adds a comma at the end of every text array written to the data!
	CString str, tmp;

	if(ary.GetSize())
		str.Format("%i", ary[0]);

	for (int i = 1; i < ary.GetSize(); i++)
	{	
		tmp.Format(",%i", ary[i]);
		str += tmp;
	}
	return str;
}

// (b.spivey, January 7th, 2015) PLID 64397 - 
void ParseCommaDeliminatedText(CArray<long, long> &ary, const char *p)
{
	long size = strlen(p), comma;
	CString temp;
	temp = p;

	while (temp != "")
	{
		comma = temp.Find(',');

		//if we have no comma, we must have 1 more value in the string
		if (comma != -1)
		{
			ary.Add(strtol(temp.Left(comma), NULL, 10));
			temp = temp.Right(temp.GetLength() - temp.Left(comma + 1).GetLength());
		}
		else
		{
			ary.Add(strtol(temp, NULL, 10));
			temp = "";
		}


	}
}

// (b.spivey, January 7th, 2015) PLID 64397 - 
CString GetCommaDeliminatedText(CArray<long, long> &ary)
{
	CString str, tmp;

	if (ary.GetSize())
		str.Format("%i", ary[0]);

	for (int i = 1; i < ary.GetSize(); i++)
	{
		tmp.Format(",%i", ary[i]);
		str += tmp;
	}
	return str;
}

// (j.jones 2011-02-11 12:29) - PLID 35180 - I added CDWord array versions as well
void ParseCommaDeliminatedText(CDWordArray &ary, const char *p)
{
	long size = strlen(p), comma;
	CString temp;
	temp = p;

	while(temp != "")
	{
		comma = temp.Find(',');
		
		//if we have no comma, we must have 1 more value in the string
		if(comma != -1)
		{
			ary.Add((DWORD)atoi(temp.Left(comma)));
			temp = temp.Right(temp.GetLength() - temp.Left(comma+1).GetLength());
		}
		else
		{
			ary.Add((DWORD)atoi(temp));
			temp = "";
		}
		
		
	}
}

CString GetCommaDeliminatedText(CDWordArray &ary)
{
	CString str, tmp;

	if(ary.GetSize())
		str.Format("%i", (long)ary[0]);

	for (int i = 1; i < ary.GetSize(); i++)
	{	
		tmp.Format(",%i", (long)ary[i]);
		str += tmp;
	}
	return str;
}

void ParseCommaDeliminatedText(CStringArray &ary, const CString &strText)
{
	CString strCurrentString;
	bool bInQuotes = false;
	bool bLastWasQuote = false;
	for(int i = 0; i < strText.GetLength(); i++) {
		if(strText.GetAt(i) == '\'') {
			bInQuotes = !bInQuotes;
			if(bLastWasQuote) //This is the second quote in a row, include it in our string.
				strCurrentString += strText.GetAt(i);
			bLastWasQuote = true;
		}
		else if(strText.GetAt(i) == ',' && !bInQuotes) {
			bLastWasQuote = false;
			//We're done with this string!
			ary.Add(strCurrentString);
			strCurrentString = "";
		}
		else {
			bLastWasQuote = false;
			//Regular character, add it.
			strCurrentString += strText.GetAt(i);
		}
	}
}

CString GetCommaDeliminatedText(CStringArray &ary)
{
	CString strText;
	for(int i = 0; i < ary.GetSize(); i++) {
		strText += "'" + _Q(ary.GetAt(i)) + "',";
	}
	return strText;
}

void GetRemotePropertyArray(LPCSTR strPropName, CArray<int,int> &ary, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */)
{
	g_propManager.GetRemotePropertyArray(strPropName, ary, nPropNum, strUsername);
	/*CString str = GetRemotePropertyText(strPropName, "", nPropNum, strUsername);
	ParseCommaDeliminatedText(ary, str);*/
}

void SetRemotePropertyArray(LPCSTR strPropName, CArray<int,int> &ary, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */)
{
	g_propManager.SetRemotePropertyArray(strPropName, ary, nPropNum, strUsername);
	/*CString str = GetCommaDeliminatedText(ary);
	SetRemotePropertyText(strPropName, str, nPropNum, strUsername);*/
}

void GetRemotePropertyCStringArrayMemo(LPCSTR strPropName, CStringArray &ary, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */)
{
	g_propManager.GetRemotePropertyCStringArrayMemo(strPropName, ary, nPropNum, strUsername);
	/*CString str = GetRemotePropertyMemo(strPropName, "", nPropNum, strUsername);
	ParseCommaDeliminatedText(ary, str);*/
}

void SetRemotePropertyCStringArrayMemo(LPCSTR strPropName, CStringArray &ary, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */)
{
	g_propManager.SetRemotePropertyCStringArrayMemo(strPropName, ary, nPropNum, strUsername);
	/*CString str = GetCommaDeliminatedText(ary);
	SetRemotePropertyMemo(strPropName, str, nPropNum, strUsername);*/
}

// Sets the property value in the form of a variant
// A return value of zero indicates success, otherwise an error code
long SetRemoteProperty(LPCTSTR strPropName, PropertyType nPropType, COleVariant varNewValue, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */)
{
	return g_propManager.SetRemoteProperty(strPropName, nPropType, varNewValue, nPropNum, strUsername);
/*	try {
		// Make sure we have access to the data
		EnsureRemoteData();

		// Get the variant as a string
		CString strValue = FormatAsString(varNewValue);

		// Generate the username that will be used for the queries
		CString strUser = strUsername ? _Q(strUsername) : _T("<None>");
		
		// Determine the name of the field to which we are writing and the markers for that field
		CString strFieldName, strMarker;
		switch (nPropType) {
		case propText:			strFieldName = _T("TextParam"); strValue = _Q(strValue); strMarker = "'"; break;
		case propMemo:			strFieldName = _T("MemoParam"); strValue = _Q(strValue); strMarker = "'"; break;
		case propNumber:		strFieldName = _T("IntParam"); strMarker = ""; break;
		case propFloat:		strFieldName = _T("FloatParam"); strMarker = ""; break;
		case propDateTime:	strFieldName = _T("DateTimeParam"); strMarker = "'"; break;
		case propImage:		
		{
			_RecordsetPtr rsImage = CreateRecordset(adOpenDynamic, adLockOptimistic, "SELECT ImageParam FROM ConfigRT "
				"WHERE Username = '%s' AND Name = '%s' AND Number = %d", strUser, _Q(strPropName), nPropNum);
			if(rsImage->eof) {
				//Add it.
				rsImage->Close();
				ExecuteSql("INSERT INTO ConfigRT (Username, Name, Number) VALUES ('%s', '%s', %d)", strUser, _Q(strPropName), nPropNum);
				rsImage = CreateRecordset(adOpenDynamic, adLockOptimistic, "SELECT ImageParam FROM ConfigRT "
				"WHERE Username = '%s' AND Name = '%s' AND Number = %d", strUser, _Q(strPropName), nPropNum);
			}
			rsImage->Fields->GetItem("ImageParam")->Value = varNewValue;
			rsImage->Update();
			return 0;
		}
		break;
		default:
			// Give an error and return failure
			{
				CString strErr;
				strErr.Format(
					"SetRemoteProperty Error 125: Unexpected property "
					"type (%li) given in SetRemoteProperty", nPropType);
				HandleException(NULL, strErr, __LINE__, __FILE__);
				return -2;
			}
			break;
		}

		// Execute a query assuming the record is already there
		long nRecCount = 0;
		ExecuteSql(&nRecCount, adCmdText, 
			"UPDATE ConfigRT SET [%s] = %s%s%s WHERE Username = '%s' AND Name = '%s' AND Number = %d", 
			strFieldName, strMarker, strValue, strMarker, strUser, _Q(strPropName), nPropNum);
		
		if (nRecCount != 1) {
			try {
				// The record wasn't there so create it
				ExecuteSql(&nRecCount, adCmdText, 
					"INSERT INTO ConfigRT (Username, Name, Number, [%s]) VALUES('%s', '%s', %d, %s%s%s)", 
					strFieldName, strUser, _Q(strPropName), nPropNum, strMarker, strValue, strMarker);
				if (nRecCount != 1) {
					// What else can we do, this is an error
					HandleException(NULL, "SetRemoteProperty Error 750.  Could not create new remote property.", __LINE__, __FILE__);
					return -4;
				}
			} catch (_com_error e) {
				// Give the error
				HandleException(e, "SetRemoteProperty Error 250", __LINE__, __FILE__);
				// Return failure
				return -3;
			}
		}

		// Return success
		return 0;

	} NxCatchAll("SetRemoteProperty Error 500");
	
	// Return general failure
	return -1;*/
}

CString GetRemotePropertyText(LPCTSTR strPropName, LPCTSTR strDefault /* = 0 */, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */, bool bAutoCreate /* = true */)
{
	return g_propManager.GetRemotePropertyText(strPropName, strDefault, nPropNum, strUsername, bAutoCreate);
/*	COleVariant tmpVar;
	
	// Pass pointer to default variant only if a default value was passed
	if (strDefault) {
		COleVariant varDefault(strDefault);
		tmpVar = GetRemoteProperty(strPropName, propText, &varDefault, nPropNum, strUsername, bAutoCreate);
	} else {
		tmpVar = GetRemoteProperty(strPropName, propText, 0, nPropNum, strUsername, bAutoCreate);
	}
	
	// If null was returned, return a null-like value, otherwise, return the value
	if (tmpVar.vt == VT_NULL) {
		return CString("");
	} else {
		return (LPCTSTR)_bstr_t(tmpVar);
	}*/
}

//TES 10/15/2008 - PLID 31646 - Need an overload that takes a connection pointer.
CString GetRemotePropertyText(ADODB::_Connection* lpCon, LPCTSTR strPropName, LPCTSTR strDefault /* = 0 */, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */, bool bAutoCreate /* = true */)
{
	return g_propManager.GetRemotePropertyText(strPropName, strDefault, nPropNum, strUsername, bAutoCreate);
}

CString GetRemotePropertyMemo(LPCTSTR strPropName, LPCTSTR strDefault /* = 0 */, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */, bool bAutoCreate /* = true */)
{
	return g_propManager.GetRemotePropertyMemo(strPropName, strDefault, nPropNum, strUsername, bAutoCreate);
/*	COleVariant tmpVar;
	
	// Pass pointer to default variant only if a default value was passed
	if (strDefault) {
		COleVariant varDefault(strDefault);
		tmpVar = GetRemoteProperty(strPropName, propMemo, &varDefault, nPropNum, strUsername, bAutoCreate);
	} else {
		tmpVar = GetRemoteProperty(strPropName, propMemo, 0, nPropNum, strUsername, bAutoCreate);
	}
	
	// If null was returned, return a null-like value, otherwise, return the value
	if (tmpVar.vt == VT_NULL) {
		return CString("");
	}
	else {
		return (LPCTSTR)_bstr_t(tmpVar);
	}*/
}

long GetRemotePropertyInt(LPCTSTR strPropName, long nDefault /* = LONG_MIN + 1 */, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */, bool bAutoCreate /* = true */)
{
	return g_propManager.GetRemotePropertyInt(strPropName, nDefault, nPropNum, strUsername, bAutoCreate);
/*	COleVariant tmpVar;
	
	// Pass pointer to default variant only if a default value was passed
	if (nDefault != LONG_MIN + 1) {
		COleVariant varDefault(nDefault, VT_I4);
		tmpVar = GetRemoteProperty(strPropName, propNumber, &varDefault, nPropNum, strUsername, bAutoCreate);
	} else {
		tmpVar = GetRemoteProperty(strPropName, propNumber, 0, nPropNum, strUsername, bAutoCreate);
	}
	
	// If null was returned, return a null-like value, otherwise, return the value
	if (tmpVar.vt == VT_NULL) {
		return 0L;
	}
	else {
		return tmpVar.lVal;
	}*/
}

//TES 10/15/2008 - PLID 31646 - Need an overload that takes a connection pointer.
long GetRemotePropertyInt(ADODB::_Connection *lpCon, LPCTSTR strPropName, long nDefault /* = LONG_MIN + 1 */, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */, bool bAutoCreate /* = true */)
{
	return g_propManager.GetRemotePropertyInt(strPropName, nDefault, nPropNum, strUsername, bAutoCreate);
}
float GetRemotePropertyFloat(LPCTSTR strPropName, float *pfltDefault /* = 0 */, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */, bool bAutoCreate /* = true */)
{
	return g_propManager.GetRemotePropertyFloat(strPropName, pfltDefault, nPropNum, strUsername, bAutoCreate);
/*	COleVariant tmpVar;
	
	// Pass pointer to default variant only if a default value was passed
	if (pfltDefault) {
		COleVariant varDefault(*pfltDefault);
		tmpVar = GetRemoteProperty(strPropName, propFloat, &varDefault, nPropNum, strUsername, bAutoCreate);
	} else {
		tmpVar = GetRemoteProperty(strPropName, propFloat, 0, nPropNum, strUsername, bAutoCreate);
	}
	
	// If null was returned, return a null-like value, otherwise, return the value
	if (tmpVar.vt == VT_NULL) {
		return 0.0;
	}
	else {
		return tmpVar.fltVal;
	}*/
}

COleDateTime GetRemotePropertyDateTime(LPCTSTR strPropName, const COleDateTime *pdtDefault /* = 0 */, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */, bool bAutoCreate /* = true */)
{
	return g_propManager.GetRemotePropertyDateTime(strPropName, pdtDefault, nPropNum, strUsername, bAutoCreate);
/*	COleVariant tmpVar;
	
	// Pass pointer to default variant only if a default value was passed
	if (pdtDefault) {
		COleVariant varDefault(*pdtDefault);
		tmpVar = GetRemoteProperty(strPropName, propDateTime, &varDefault, nPropNum, strUsername, bAutoCreate);
	} else {
		tmpVar = GetRemoteProperty(strPropName, propDateTime, 0, nPropNum, strUsername, bAutoCreate);
}

	// If null was returned, return a null-like value, otherwise, return the value
	COleDateTime Ans;
	ASSERT(tmpVar.vt == VT_NULL || tmpVar.vt == VT_EMPTY || tmpVar.vt == VT_DATE);
	switch (tmpVar.vt) {
	case VT_NULL:
	case VT_EMPTY:
		// Nothing to return
		Ans.SetStatus(COleDateTime::invalid);
		break;
	case VT_DATE:
		// Got the date, just return it
		Ans = tmpVar.date;
		break;
	default:
		// Unexpected type
		ThrowNxException("GetRemotePropertyDateTime: Unexpected return value type of %li!", tmpVar.vt);
		break;
	}
	return Ans;*/
}

_variant_t GetRemotePropertyImage(LPCTSTR strPropName, int nPropNum /* = 0*/, LPCTSTR strUsername /* = NULL */, bool bAutoCreate /* = true*/)
{
	return g_propManager.GetRemotePropertyImage(strPropName, nPropNum, strUsername, bAutoCreate);
	//return GetRemoteProperty(strPropName, propImage, 0, nPropNum, strUsername, bAutoCreate);
}

long SetRemotePropertyText(LPCTSTR strPropName, LPCTSTR strNewValue, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */)
{
	return g_propManager.SetRemotePropertyText(strPropName, strNewValue, nPropNum, strUsername);
	//return SetRemoteProperty(strPropName, propText, COleVariant(strNewValue, VT_BSTR), nPropNum, strUsername);
}

long SetRemotePropertyMemo(LPCTSTR strPropName, LPCTSTR strNewValue, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */)
{
	return g_propManager.SetRemotePropertyMemo(strPropName, strNewValue, nPropNum, strUsername);
	//return SetRemoteProperty(strPropName, propMemo, COleVariant(strNewValue, VT_BSTR), nPropNum, strUsername);
}

long SetRemotePropertyInt(LPCTSTR strPropName, long nNewValue, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */)
{
	return g_propManager.SetRemotePropertyInt(strPropName, nNewValue, nPropNum, strUsername);
	//return SetRemoteProperty(strPropName, propNumber, COleVariant(nNewValue, VT_I4), nPropNum, strUsername);
}

long SetRemotePropertyFloat(LPCTSTR strPropName, float fltNewValue, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */)
{
	return g_propManager.SetRemotePropertyFloat(strPropName, fltNewValue, nPropNum, strUsername);
	//return SetRemoteProperty(strPropName, propFloat, COleVariant(fltNewValue), nPropNum, strUsername);
}

long SetRemotePropertyDateTime(LPCTSTR strPropName, COleDateTime dtNewValue, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */)
{
	return g_propManager.SetRemotePropertyDateTime(strPropName, dtNewValue, nPropNum, strUsername);
	//return SetRemoteProperty(strPropName, propDateTime, COleVariant(dtNewValue), nPropNum, strUsername);
}

long SetRemotePropertyImage(LPCTSTR strPropName, _variant_t varNewValue, int nPropNum /* = 0 */, LPCTSTR strUsername /* = NULL */)
{
	return g_propManager.SetRemotePropertyImage(strPropName, varNewValue, nPropNum, strUsername);
	//return SetRemoteProperty(strPropName, propImage, varNewValue, nPropNum, strUsername);
}

void FlushRemotePropertyCache()
{
	g_propManager.FlushRemotePropertyCache();
}

void InitializeRemotePropertyCache(CWnd* pWndTimer)
{
	g_propManager.Initialize(pWndTimer);
}

void DestroyRemotePropertyCache()
{
	g_propManager.Destroy();
}