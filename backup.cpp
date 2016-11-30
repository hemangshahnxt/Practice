#include <stdafx.h>
// Make it so backup.h doesn't include netutils.h
#include "nxpackets.h"
#include "backup.h"
#include "RegUtils.h"
#include "client.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "NxBackupUtils.h"
#include "ASDDlg.h"
#include "NxTaskDialog.h"

//using namespace NetUtils;

static _BACKUP_STATS g_bkpStats;
static _BACKUP_DOCTEMPLATE_CONFIG g_DocTmpInfo;
static _BACKUP_DATABKP_CONFIG g_BkpCfg;
static BOOL g_bBackupIgnoringAsyncMessages = FALSE;

static CString g_strLastServerErrorText;
static CString g_strLastWSErrorText;
static CString g_strLastUserErrorText;
static CString g_strLastDocumentsErrorText;
static CString g_strLastTemplatesErrorText;
static CString g_strLastCustomReportsErrorText;
static CString g_strLastImagesErrorText; // (z.manning 2008-09-30 09:37) - PLID 31532

static EBackupUserConfig g_bkpUserCfg;
CStringArray g_astrbkpUsernames;

CString GetLastBackupTime(EBackupType type)
{
	COleDateTime dt;
	CString strOut;
	
	switch (type)
	{
	case eBackupServer: dt = g_bkpStats.dtLastBackupModDate[0]; break;
	case eBackupClient: dt = g_bkpStats.dtLastBackupModDate[1]; break;
	case eBackupUser: dt = g_bkpStats.dtLastBackupModDate[2]; break;
	}

	if (dt.GetStatus() == COleDateTime::valid)
		strOut = FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoDateTime);
	else
		strOut = INDETERMINANT_BACKUP_NAME;

	return strOut;
}

CString GetNextBackupTime()
{
	time_t lTime = time(NULL);
	unsigned long lCurTime;
	struct tm* today;
	COleDateTime dtStartTime = COleDateTime::GetCurrentTime();

	///////////////////////////////////////////////////////
	// Calculate the time
	lTime = time(NULL);
	today = localtime( &lTime );
	lCurTime = (unsigned long)((today->tm_hour * 60) + today->tm_min);

	// Jump forward a day if necessary
	if (lCurTime > g_BkpCfg.lStartTime)
	{
		dtStartTime += COleDateTimeSpan(1,0,0,0);
	}

	dtStartTime -= COleDateTimeSpan(0, dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond());
	dtStartTime += COleDateTimeSpan(0, 0, g_BkpCfg.lStartTime, 0);

//DRT 6/20/03 - international settings
//	return dtStartTime.Format("%x %I:%M %p");
	return FormatDateTimeForInterface(dtStartTime, DTF_STRIP_SECONDS, dtoDateTime);
}

CString GetNextBackupTimeEnd()
{
	time_t lTime = time(NULL);
	unsigned long lCurTime;
	struct tm* today;
	COleDateTime dtStartTime = COleDateTime::GetCurrentTime();

	///////////////////////////////////////////////////////
	// Calculate the time
	lTime = time(NULL);
	today = localtime( &lTime );
	lCurTime = (unsigned long)((today->tm_hour * 60) + today->tm_min);

	// Jump forward a day if necessary
	if (lCurTime > g_BkpCfg.lStartTime)
	{
		dtStartTime += COleDateTimeSpan(1,0,0,0);
	}

	dtStartTime -= COleDateTimeSpan(0, dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond());
	dtStartTime += COleDateTimeSpan(0, 0, g_BkpCfg.lStartTime, 0);

	// Add the backup time span
	dtStartTime += COleDateTimeSpan(0,0,g_BkpCfg.lTimeSpan,0);

//	return dtStartTime.Format("%x %I:%M %p");
	return FormatDateTimeForInterface(dtStartTime, DTF_STRIP_SECONDS, dtoDateTime);
}

CString GetLastBackupErrorText(EBackupType type)
{
	switch (type)
	{
	case eBackupServer: return g_strLastServerErrorText;
	case eBackupClient: return g_strLastWSErrorText;
	case eBackupUser: return g_strLastUserErrorText;
	case eBackupDocuments: return g_strLastDocumentsErrorText;
	case eBackupTemplates: return g_strLastTemplatesErrorText;
	case eBackupCustomReports: return g_strLastCustomReportsErrorText;
	// (z.manning 2008-09-30 09:51) - PLID 31532 - Added images folder backup
	case eBackupImages: return g_strLastImagesErrorText;
	}
	return "";
}

CString GetUserBackupDirectory()
{
	return g_BkpCfg.szUserBackupPath;
}

CString GetClientBackupDirectory()
{
	return g_bkpStats.szLastWSBackupPath;
}

void Backup_OnBackupMessage(BOOL bNexTechUser, _BACKUP_STATS* pBkpMsg)
{
	memcpy(&g_bkpStats, pBkpMsg, sizeof(_BACKUP_STATS));

	//DRT 10/6/2005 - PLID 17827 - We should give this all as 1 message box, not as numerous
	//	different ones.
	CString strMessage = "";

	// Check for free hard drive space
	if (!pBkpMsg->bHasFreeSpace[0])
	{
		CString str;
		str.Format(" - The drive on the server containing your backups is low on hard drive space. Practice recommends at least %d MB be available on that drive. Please make more free space on the server to ensure future backups, and to improve software performance.\r\n",
			pBkpMsg->nRequiredFreeSpace);
		strMessage += str;
	}
	if (!pBkpMsg->bHasFreeSpace[1])
	{
		CString str;
		str.Format(" - The drive that holds your workstation backup path '%s' is low on hard drive space. Practice recommends at least %d MB be available on that drive.\r\n",
			pBkpMsg->szLastWSBackupPath, pBkpMsg->nRequiredFreeSpace);
		strMessage += str;
	}
	if (!pBkpMsg->bHasFreeSpace[2])
	{
		CString str;
		str.Format(" - The drive that holds the user-defined path '%s' is low on hard drive space. Practice recommends at least %d MB be available on that drive.\r\n",
			pBkpMsg->szLastUserBackupPath, pBkpMsg->nRequiredFreeSpace);
		strMessage += str;
	}

	if(!strMessage.IsEmpty())
		MsgBox("The following errors have occurred:\r\n\r\n" + strMessage);

	strMessage.Empty();

	//DRT 10/6/2005 - I'm leaving these as a separate message.  Corruption is something we need to fix, but most likely if one happens, more are going to.

	//
	// (c.haag 2006-04-07 09:55) - PLID 18703 - We will now use a less scary way to inform the client
	//
	// Check for corruption
	CString str;
	switch (pBkpMsg->bkpcorrupt[0]) {
	case eCorrupt:
		str.Format(" - NexTech Practice has detected that one of your server backups may be corrupt. Please contact NexTech Technical Support for assistance.\r\n");
		strMessage += str;
		break;
	case eUnknownCorruption:
		str.Format(" - NexTech Practice could not determine the status of your server backups. Please contact NexTech Technical Support for assistance.\r\n");
		strMessage += str;
		break;
	default:
		break;
	}

	switch (pBkpMsg->bkpcorrupt[1]) {
	case eCorrupt:
		str.Format(" - NexTech Practice has detected that one of your workstation backups may be corrupt. Please contact NexTech Technical Support for assistance.\r\n");
		strMessage += str;
		break;
	case eUnknownCorruption:
		str.Format(" - NexTech Practice could not determine the status of your workstation backups. Please contact NexTech Technical Support for assistance.\r\n");
		strMessage += str;
		break;
	default:
		break;
	}

	switch (pBkpMsg->bkpcorrupt[2]) {
	case eCorrupt:
		str.Format(" - NexTech Practice has detected that one of your user-defined backups may be corrupt. Please contact NexTech Technical Support for assistance.\r\n");
		strMessage += str;
		break;
	case eUnknownCorruption:
		str.Format(" - NexTech Practice could not determine the status of your user-defined backups. Please contact NexTech Technical Support for assistance.\r\n");
		strMessage += str;
		break;
	default:
		break;
	}

	if(!strMessage.IsEmpty())
		MsgBox("The following corruption has occurred:\r\n\r\n" + strMessage);
}

/*
void Backup_OnBackupMessage(BOOL bNexTechUser, _BACKUP_MESSAGE* pBkpMsg)
{
	unsigned long dwTimeSpan;

	memcpy(&g_bkpStats, pBkpMsg, sizeof(_BACKUP_MESSAGE));

	if (!pBkpMsg->bHasFreeSpace[0])
		MsgBox("The server is low on hard drive space. Please make more free space on the server to ensure future backups and improve software performance.");
	if (!pBkpMsg->bHasFreeSpace[1])
		MsgBox("The computer containing the path %s is low on hard drive space. Please make more free space on that computer to ensure future backups.",
		GetClientBackupDirectory());
	if (!pBkpMsg->bHasFreeSpace[2])
		MsgBox("The computer containing the path %s is low on hard drive space. Please make more free space on that computer to ensure future backups.",
		pBkpMsg->szUserBackupPath);

	// By definition, if the time span is 0, don't start the timer.
	if (pBkpMsg->lTimeSpan && pBkpMsg->lTimeSpan <= 60*24 &&
		pBkpMsg->lStartTime <= 60*24)
	{
		dwTimeSpan = (pBkpMsg->lStartTime - pBkpMsg->lCurTime);

		if (dwTimeSpan <= 1)
		{
			//MsgBox("Your data is now being backed up. Practice will now terminate.");

			// CLOSE PRACTICE //
			//if (!bNexTechUser)
			//{
				PostQuitMessage(0);
				return;
			//}
		}
		else if (dwTimeSpan <= 15)
		{
			MsgBox("Your data will be backed up in %d minute(s). Please finish your work and close Practice.",
				dwTimeSpan);
		}
	}
}
*/
BOOL Backup_OnDBInfoMessage(BOOL bNexTechUser, _BACKUP_STATS* pBkpMsg, _BACKUP_DOCTEMPLATE_CONFIG* pDocTmpCfg, _BACKUP_DATABKP_CONFIG* pBkpCfg)
{
	// (c.haag 2006-10-23 15:46) - PLID 23184 - We now look to the NxServer packets themselves
	// to see if we need to run a backup, and if so, why. Some of the legacy code was decisively
	// copied into this new body of code

	// (a.walling 2010-06-14 07:33) - PLID 38442
	//CString strBkpMsg = "Your last NexTech Practice backup failed. The following errors were reported:\n\n";
	CString strBackupMessage = "Your last NexTech Practice backup failed.\n\n";
	CString strBackupErrors;

	// (c.haag 2006-11-27 16:21) - PLID 23668 - If bNeedsToRunBackup is true as a consequence of a
	// document-related (documents/templates/custom reports) backup, then both must be true:
	// - NxServer thinks a backup needs to be run
	// - The last backup message has a length, but does not have the word "success" or "skipped" in it
	BOOL bNeedsToRunBackup = FALSE;

	// (a.walling 2010-06-04 13:47) - PLID 21048 - We can still ignore the server backup if it is not too old (but we attempt to backup again in the background)
	BOOL bNeedsToRunServerBackup = FALSE;

	// (a.walling 2010-06-04 13:47) - PLID 21048 - We actually need to prevent them from logging in.
	BOOL bForceBackup = FALSE;

	//
	// Copy the packets
	//
	memcpy(&g_bkpStats, pBkpMsg, sizeof(_BACKUP_STATS));
	memcpy(&g_DocTmpInfo, pDocTmpCfg, sizeof(_BACKUP_DOCTEMPLATE_CONFIG));
	memcpy(&g_BkpCfg, pBkpCfg, sizeof(_BACKUP_DATABKP_CONFIG));

	//
	// Build a list of data backup error messages
	//
	// (c.haag 2006-10-25 14:13) - PLID 23184 - A comment about how this works: One of three things
	// is true: A data backup failed and NxServer reported an error, a data backup does
	// not exist, or a data backup is more than 24 hours old. We should not get to the
	// latter statement, but this is the best we can do given the granularity of the messages.
	// If the message comes out awkward, it's NxServer fault.
	//
	// (c.haag 2007-01-18 12:49) - PLID 23184 - NxServer considers a backup to be "old" if it's 18 hours old,
	// not 24 hours old. The whole premise of PLID 23184 was to avoid this problem. Please disregard
	// my comment about a data backup being 24 hours old.
	//
	if (pBkpMsg->bNeedsServerDataBackup) {
		bNeedsToRunBackup = TRUE;
		bNeedsToRunServerBackup = TRUE; // Flag the backup as being required to run

		// Server messages
		if (-1 == g_strLastServerErrorText.Find("success") && -1 == g_strLastServerErrorText.Find("Success") &&
			-1 == g_strLastServerErrorText.Find("skipped") && g_strLastServerErrorText.GetLength() > 0)	{
			strBackupErrors += "Server - " + g_strLastServerErrorText + "\n\n"; 
		} else {
			strBackupErrors += "Server - No existing data backup was detected on your system, or your existing backup is out of date.\n\n";
		}

		// (a.walling 2010-06-04 13:45) - PLID 21048 - Prevent running if the backup is more than 3 weekdays old.
		// (j.camacho 2016-05-16 10:34) - NX-100455 - Changed to 5 weekdays old. 
		COleDateTime dtBackup = pBkpMsg->dtLastBackupModDate[0];
		if (dtBackup.GetStatus() != COleDateTime::valid) {
			// we cannot tell how old the backup is, so we must force it.
			bForceBackup = TRUE;
		} else {
			COleDateTime dtNxServer = pBkpMsg->dtNxServer;
			
			COleDateTime dtNxServerMinimum = dtNxServer;
			{
				// (j.camacho 2016-05-16 10:34) - NX-100455 - changed allowed days to 5
				int nAllowedDays = 5;
				const COleDateTimeSpan dtsOneDay(1, 0, 0, 0);

				while (nAllowedDays > 0) {
					int nDayOfWeek = dtNxServerMinimum.GetDayOfWeek();
					if (nDayOfWeek == 1 || nDayOfWeek == 7) {
						// weekend
					} else {
						nAllowedDays--;
					}

					dtNxServerMinimum -= dtsOneDay;
				}
			
			}
			if (dtBackup <= dtNxServerMinimum) {
				// we cannot tell how old the backup is, so we must force it.
				bForceBackup = TRUE;
			}
		}
	}

	if (pBkpMsg->bNeedsWorkstationDataBackup) {
		bNeedsToRunBackup = TRUE;

		// Workstation messages
		if (-1 == g_strLastWSErrorText.Find("success") && -1 == g_strLastWSErrorText.Find("Success") &&
			-1 == g_strLastWSErrorText.Find("skipped") && g_strLastWSErrorText.GetLength() > 0) {
			strBackupErrors += "Workstation - " + g_strLastWSErrorText + "\n\n";
		} else  {
			strBackupErrors += "Workstation - No existing data backup was detected on your system, or your existing backup is out of date.\n\n";
		}
	}

	if (pBkpMsg->bNeedsUserDefinedDataBackup) {
		bNeedsToRunBackup = TRUE;

		// User-defined backup messages
		if (-1 == g_strLastUserErrorText.Find("success") && -1 == g_strLastUserErrorText.Find("Success") &&
			-1 == g_strLastUserErrorText.Find("skipped") && g_strLastUserErrorText.GetLength() > 0) {
			strBackupErrors += "User-Defined - " + g_strLastUserErrorText + "\n\n";
		} else {
			strBackupErrors += "User-Defined - No existing data backup was detected on your system, or your existing backup is out of date.\n\n";
		}
	}

	//
	// Determine if the documents backup needs to be run, and if so, generate the warning message
	//
	// (c.haag 2006-10-25 12:03) - PLID 23184 - While I can't defend the idea that we have just one
	// document-related error message while we have up to three different backup locations, I can
	// say that this is consistent with the NxBackupUtils backup function. If we have an unusual message
	// here, then it's NxServer's fault. We still need to check if there is a document-related backup present
	// on the server because g_strLast...ErrorText will probably be blank the first time you log into
	// Practice.
	// 
	if (pBkpMsg->bNeedsDocumentsBackup) {
		// (c.haag 2006-11-27 16:25) - PLID 23668 - Maybe the documents "got old" after last night's backup.
		// We can check for this by scanning the backup message for text inferring success or the backup skipping
		if (-1 == g_strLastDocumentsErrorText.Find("success") && -1 == g_strLastDocumentsErrorText.Find("Success") &&
			-1 == g_strLastDocumentsErrorText.Find("skipped") && g_strLastDocumentsErrorText.GetLength() > 0) {
			bNeedsToRunBackup = TRUE;
			COleDateTime dtBackup = pBkpMsg->dtLastDocBkpModDate[0]; // Server's document backup modified date
			if (dtBackup.GetStatus() != COleDateTime::valid) {
				strBackupErrors += "Documents - No existing documents backup was detected on your server.\n\n";
			} else {
				strBackupErrors += "Documents - " + g_strLastDocumentsErrorText + "\n\n";
			}
		}
	}

	//
	// Determine if the templates backup needs to be run, and if so, generate the warning message
	// 
	if (pBkpMsg->bNeedsTemplatesBackup) {
		// (c.haag 2006-11-27 16:25) - PLID 23668 - Maybe the documents "got old" after last night's backup.
		// We can check for this by scanning the backup message for text inferring success or the backup skipping
		if (-1 == g_strLastTemplatesErrorText.Find("success") && -1 == g_strLastTemplatesErrorText.Find("Success") &&
			-1 == g_strLastTemplatesErrorText.Find("skipped") && g_strLastTemplatesErrorText.GetLength() > 0) {
			bNeedsToRunBackup = TRUE;
			COleDateTime dtBackup = pBkpMsg->dtLastTemplateBkpModDate[0]; // Server's templates backup modified date
			if (dtBackup.GetStatus() != COleDateTime::valid) {
				strBackupErrors += "Templates - No existing templates backup was detected on your server.\n\n";
			} else {
				strBackupErrors += "Templates - " + g_strLastTemplatesErrorText + "\n\n";
			}
		}
	}

	//
	// Determine if the custom reports backup needs to be run, and if so, generate the warning message
	// 
	if (pBkpMsg->bNeedsCustomReportsBackup) {
		// (c.haag 2006-11-27 16:25) - PLID 23668 - Maybe the documents "got old" after last night's backup.
		// We can check for this by scanning the backup message for text inferring success or the backup skipping
		if (-1 == g_strLastCustomReportsErrorText.Find("success") && -1 == g_strLastCustomReportsErrorText.Find("Success") &&
			-1 == g_strLastCustomReportsErrorText.Find("skipped") && g_strLastCustomReportsErrorText.GetLength() > 0) {
			bNeedsToRunBackup = TRUE;
			COleDateTime dtBackup = pBkpMsg->dtLastCustomReportBkpModDate[0]; // Server's custom reports backup modified date
			if (dtBackup.GetStatus() != COleDateTime::valid) {
				strBackupErrors += "Custom Reports - No existing custom reports backup was detected on your server.\n\n";
			} else {
				strBackupErrors += "Custom Reports - " + g_strLastCustomReportsErrorText + "\n\n";
			}
		}
	}

	//
	// (z.manning 2008-09-30 09:38) - PLID 31532 - Determine if the images backup needs
	// to be run, and if so, generate the warning message
	// 
	if (pBkpMsg->bNeedsImagesBackup) {
		// (c.haag 2006-11-27 16:25) - PLID 23668 - Maybe the documents "got old" after last night's backup.
		// We can check for this by scanning the backup message for text inferring success or the backup skipping
		if (-1 == g_strLastImagesErrorText.Find("success") && -1 == g_strLastImagesErrorText.Find("Success") &&
			-1 == g_strLastImagesErrorText.Find("skipped") && g_strLastImagesErrorText.GetLength() > 0) {
			bNeedsToRunBackup = TRUE;
			COleDateTime dtBackup = pBkpMsg->dtLastImageBkpModDate[0]; // Server's images backup modified date
			if (dtBackup.GetStatus() != COleDateTime::valid) {
				strBackupErrors += "Images - No existing images backup was detected on your server.\n\n";
			} else {
				strBackupErrors += "Images - " + g_strLastImagesErrorText + "\n\n";
			}
		}
	}
	
	if (!bNeedsToRunBackup) {
		return FALSE;
	}

	//
	// Now prompt the user that the backup needs to be run, and do so if necessary
	//
				
	// We want to do a synchronous backup
	// (a.walling 2010-06-14 08:19) - PLID 38442
	DWORD dwFlags = 0;
	if (pBkpMsg->bNeedsServerDataBackup) dwFlags |= BACKUP_FLAG_SERVER;
	if (pBkpMsg->bNeedsWorkstationDataBackup) dwFlags |= BACKUP_FLAG_WORKSTATION;
	if (pBkpMsg->bNeedsUserDefinedDataBackup) dwFlags |= BACKUP_FLAG_USERDEFINED;
	if (pBkpMsg->bNeedsDocumentsBackup) dwFlags |= BACKUP_FLAG_DOCUMENTS;
	if (pBkpMsg->bNeedsTemplatesBackup) dwFlags |= BACKUP_FLAG_TEMPLATES;
	if (pBkpMsg->bNeedsCustomReportsBackup) dwFlags |= BACKUP_FLAG_CUSTOM_REPORTS;
	if (pBkpMsg->bNeedsImagesBackup) dwFlags |= BACKUP_FLAG_IMAGES;

	strBackupMessage.TrimRight();
	strBackupErrors.TrimRight();

	return Backup_TaskDialog(dwFlags, strBackupMessage, strBackupErrors, bNexTechUser, bForceBackup, bNeedsToRunServerBackup, Backup_CanWindowsUserPerformBackup());

	// (a.walling 2010-06-04 09:34) - Got rid of old commented-out code
}

// A date is valid if in the last 24 hours
BOOL Backup_ValidDate(COleDateTime dtBackup)
{
	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	if (dtBackup.GetStatus() != COleDateTime::valid)
		return FALSE;
	if (dtBackup < dtNow - COleDateTimeSpan(1,0,0,0))
		return FALSE;
	return TRUE;
}

DWORD Backup_DetectNeededBackups()
{
	DWORD dwFlags = 0;

	//DRT 6/15/2007 - PLID 25531 - Removing NetUtils.  The defines here should be pulled
	//	from NxBackupUtils, which have the same number, just a different name.
	if (!Backup_ValidDate(g_bkpStats.dtLastBackupModDate[0]))
		dwFlags |= BACKUP_FLAG_SERVER;
	if (!Backup_ValidDate(g_bkpStats.dtLastBackupModDate[1]))
		dwFlags |= BACKUP_FLAG_WORKSTATION;
	if (strlen(g_BkpCfg.szUserBackupPath))
		if (!Backup_ValidDate(g_bkpStats.dtLastBackupModDate[2]))
			dwFlags |= BACKUP_FLAG_USERDEFINED;

	return dwFlags;
}

SOCKET Backup_SocketConnect()
{
	CString strNxServerIP = NxRegUtils::ReadString(GetRegistryBase() + "NxServerIP");

	SOCKET s;
	SOCKADDR_IN namesock;
	int timeout = 10000; // 10 seconds

	// Create a streaming socket
	if ((s = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET)
	{
		AfxThrowNxException("Could not create TCP/IP socket. Please ensure your network drivers are installed correctly.");
		return INVALID_SOCKET;
	}

	// Assign a connection and transmission timeout of ten seconds for
	// the act of connect()'ing
	if (setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) != NO_ERROR)
	{
		AfxThrowNxException("Could not create TCP/IP socket. Please ensure your network drivers are installed correctly.");
		return INVALID_SOCKET;
	}
	if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) != NO_ERROR)
	{
		AfxThrowNxException("Could not create TCP/IP socket. Please ensure your network drivers are installed correctly.");
		return INVALID_SOCKET;
	}
    namesock.sin_family=PF_INET;
	namesock.sin_port=htons(11983);
	namesock.sin_addr.s_addr=inet_addr(strNxServerIP);
	
	// Connect to NxServer
	if(connect(s,(PSOCKADDR)&namesock,sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		closesocket(s);
		s = INVALID_SOCKET;
		AfxThrowNxException("Could not connect to NexTech Network Service");
		return INVALID_SOCKET;
	}
	// (a.walling 2012-06-13 08:32) - PLID 50956 - Set TCP_NODELAY to disable the Nagle algorithm and eliminate the 200 ms delay
	NxSocketUtils::SetNoDelay(s);
	return s;
}

void Backup_OnBackupErrorTextMessage(const char* sz, long nTotalLen)
{
	if (!sz) return;
	if (nTotalLen > 1) { g_strLastServerErrorText = sz; nTotalLen -= strlen(sz)+1; sz += strlen(sz) + 1; }
	if (nTotalLen > 1) { g_strLastWSErrorText = sz; nTotalLen -= strlen(sz)+1; sz += strlen(sz) + 1; }
	if (nTotalLen > 1) { g_strLastUserErrorText = sz; nTotalLen -= strlen(sz)+1; sz += strlen(sz) + 1; }
	if (nTotalLen > 1) { g_strLastDocumentsErrorText = sz; nTotalLen -= strlen(sz)+1; sz += strlen(sz) + 1; }
	if (nTotalLen > 1) { g_strLastTemplatesErrorText = sz; nTotalLen -= strlen(sz)+1; sz += strlen(sz) + 1; }
	if (nTotalLen > 1) { g_strLastCustomReportsErrorText = sz; nTotalLen -= strlen(sz)+1; sz += strlen(sz) + 1; }
	// (z.manning 2008-09-30 09:51) - PLID 31532 - Added images folder backup
	if (nTotalLen > 1) { g_strLastImagesErrorText = sz; nTotalLen -= strlen(sz)+1; sz += strlen(sz) + 1; }
}

HRESULT SyncPacketWait(SOCKET s, PacketType pkttype, PacketType pkttypeWaitingFor, _NxHeader* pHdr, WORD wPktSize);

BOOL Backup_SynchronousGetStatus()
{
	try {
		//
		// (c.haag 2005-11-30 11:27) - PLID 17585 - We now have to assign the database name
		// to the socket
		//
		CString strNxServerIP = NxRegUtils::ReadString(GetRegistryBase() + "NxServerIP");
		NxSocketUtils::HCLIENT hNxServer = NxSocketUtils::Connect(NULL, strNxServerIP, 11983);
		void* pResult;
		unsigned long nResultSize;
		CString strDefaultDatabase = (LPCTSTR)GetRemoteData()->GetDefaultDatabase();
		NxSocketUtils::Send(hNxServer, PACKET_TYPE_DATABASE_NAME, (const void*)(LPCTSTR)strDefaultDatabase, strDefaultDatabase.GetLength()+1);

		// (z.manning, 03/12/2007) - PLID 25160 - Until a change to SyncPacketWait in 8300, the timeout
		// for this overload of SyncPacketWait used to be forever by default whereas it is now 5 secs by
		// default. It is entirely possible that getting backup info can take longer than 5 sec
		// if either the workstation or user-defined paths are invalid UNC paths, for example. So I changed
		// the timeout to a much longer time because
		//		1. That's the way it used to be.
		//		2. It's much better to have a slow login than not being able to at all.
		//		3. If there's a problem with the connection to NxServer, we most likely will never get here.
		const int nNxServerPacketTimeout = 60000;

		// Current backup status (includes some settings -- this is for backwards compatibility)
		//
		// (z.manning, 03/02/2007) - PLID 24715 - Increased the timeout.
		if (!NxSocketUtils::SyncPacketWait(hNxServer, PACKET_TYPE_BKP_STATUS_REQUEST, PACKET_TYPE_BKP_STATUS, NULL, 0, pResult, nResultSize, nNxServerPacketTimeout)) {
			NxSocketUtils::Disconnect(hNxServer);
			AfxThrowNxException("Could not get backup information from the NexTech Network Server (status request)");
		} else {
			memcpy(&g_bkpStats, pResult, nResultSize);
		}

		// Document and template backup configuration
		//
		// (z.manning, 03/12/2007) - PLID 25160 - Increased the timeout.
		if (!NxSocketUtils::SyncPacketWait(hNxServer, PACKET_TYPE_DOCTEMPLATE_CONFIG_REQUEST, PACKET_TYPE_DOCTEMPLATE_CONFIG, NULL, 0, pResult, nResultSize, nNxServerPacketTimeout)) {
			NxSocketUtils::Disconnect(hNxServer);
			AfxThrowNxException("Could not get backup information from the NexTech Network Server (documents/templates config)");
		} else {
			memcpy(&g_DocTmpInfo, pResult, nResultSize);
		}
		
		// Data backup configuration
		//
		// (z.manning, 03/12/2007) - PLID 25160 - Increased the timeout.
		if (!NxSocketUtils::SyncPacketWait(hNxServer, PACKET_TYPE_DATABKP_CONFIG_REQUEST, PACKET_TYPE_DATABKP_CONFIG, NULL, 0, pResult, nResultSize, nNxServerPacketTimeout)) {
			NxSocketUtils::Disconnect(hNxServer);
			AfxThrowNxException("Could not get backup information from the NexTech Network Server (data config)");
		} else {
			memcpy(&g_BkpCfg, pResult, nResultSize);
		}

		// (c.haag 2006-04-18 16:39) - PLID 19460 - Get the backup user list. We may use it later to calculate if
		// we can do a backup
		//
		// (z.manning, 03/12/2007) - PLID 25160 - Increased the timeout.
		if (!NxSocketUtils::SyncPacketWait(hNxServer, PACKET_TYPE_BACKUP_USER_CFG_QUERY, PACKET_TYPE_BACKUP_USER_CFG, NULL, 0, pResult, nResultSize, nNxServerPacketTimeout)) {
			NxSocketUtils::Disconnect(hNxServer);
			AfxThrowNxException("Could not get backup permission information from the NexTech Network Server (user config)");
		} else {
			Backup_OnBackupUserCfg((const void*)pResult, nResultSize);
		}

		// Get the error text of the previous backup, and process it first. We may need it when we go in
		// Backup_OnDBInfoMessage(). 
		//
		// (z.manning, 03/12/2007) - PLID 25160 - Increased the timeout.
		if (!NxSocketUtils::SyncPacketWait(hNxServer, PACKET_TYPE_BKP_ERROR_TEXT, PACKET_TYPE_BKP_ERROR_TEXT, NULL, 0, pResult, nResultSize, nNxServerPacketTimeout)) {
			NxSocketUtils::Disconnect(hNxServer);
			AfxThrowNxException("Could not get backup information from the NexTech Network Server");
		} else {
			Backup_OnBackupErrorTextMessage((const char*)pResult, nResultSize);
		}

		//
		// Socket cleanup
		//
		NxSocketUtils::Disconnect(hNxServer);
	}
	NxCatchAll("Error retrieving backup information from the NexTech Network Server");
	return TRUE;
}

void Backup_OnProgressPacket(_PACKET_BACKUP_PROGRESS* pStatus)
{
}

void Backup_OnRestorePacket(_PACKET_RESTORE_PROGRESS* pStatus)
{
}

BOOL Backup_IsIgnoringAsyncMessages()
{
	return g_bBackupIgnoringAsyncMessages;
}

void Backup_IgnoreAsyncMessages(BOOL bIgnore)
{
	g_bBackupIgnoringAsyncMessages = bIgnore;
}

static VOID CALLBACK BackupMsgBoxCallback(LPHELPINFO lpHelpInfo)
{
	//(j.anspach 06-09-2005 10:53 PLID 16662) - Updating the OpenManual call to work with the new help system.
	OpenManual("NexTech_Practice_Manual.chm", "System_Setup//Backup_Setup//Backup_Messages_and_Troubleshooting.htm");
}

//int Backup_MsgBox(const char* szMsg, DWORD dwFlags /* = MB_OK */, const char* szBookmark /* = NULL */)
//{
//	MSGBOXPARAMS params =
//	{
//	sizeof(MSGBOXPARAMS),
//	NULL,
//	AfxGetApp()->m_hInstance,
//	szMsg,
//	"NexTech Practice Backup System",
//	dwFlags | MB_SYSTEMMODAL,
//	NULL,
//	(DWORD)szBookmark,
//	BackupMsgBoxCallback,
//	0
//	};
//	return MessageBoxIndirect(&params);
//}

class NxBackupTaskDialog : public NxTaskDialog
{
public:
	CString m_strInformation;
	CString m_strInformationDetails;

#ifdef _DEBUG
	NxBackupTaskDialog() : m_nIgnoreBackupMessages(0) {};
	long m_nIgnoreBackupMessages;
#endif
	
	enum BackupCommands {
		eClosePractice = 0x100,
		eHelp,
		eBackup,
		eBypass,
		eNexTechBypass,
		eContact,
		eContactNotify,
		eContactEmail,
		eContactWebsite,
	};

	// (a.walling 2010-06-28 18:04) - PLID 38442 - Some more advanced handling for sending an error report
	virtual BOOL OnButtonClicked(int nButton)
	{
		if (nButton == NxBackupTaskDialog::eHelp) {
			BackupMsgBoxCallback(NULL);
			return TRUE; // prevent close
		} else if (nButton == NxBackupTaskDialog::eContact) {
			ConfigPushNew()
				.ErrorIcon().CloseOnly()
				.MainInstructionText("Contact NexTech technical support")
				.ContentText("East Coast and Midwest states, call (866) 654-4396\nWest Coast and Central states, call (888) 417-8464")
				.AddCommand(eContactNotify, "Send error report\nRelevant error information will be automatically included and submitted with your notification")
				// (a.walling 2011-08-23 14:15) - PLID 44647 - exception/error emails to allsupport@nextech.com
				.AddCommand(eContactEmail, "Email technical support\nOpens your default email application to send an email to allsupport@nextech.com")
				.AddCommand(eContactWebsite, "More contact options\nOpens the NexTech website for further contact options and information");

#if _DEBUG
			Config()
				.VerificationText("Stop bugging me; I'm a developer!")
				.FooterIcon(TD_SHIELD_ICONT);
			if (m_nIgnoreBackupMessages) {
				Config()
					.VerificationFlagChecked()					
					.FooterText("Backup messages are disabled. Clear the Debug_IgnoreBackupMessages value in HKLM\\Software\\NexTech or hold shift when starting to re-enable.");
			} else {
				Config()
					.FooterText("Backup messages are enabled.");
			}
#endif

			NavigatePage();

			return TRUE;
		} else if (nButton == NxBackupTaskDialog::eContactNotify) {
			// (a.walling 2010-06-28 18:04) - PLID 38442 - SendErrorToNexTech
			SendErrorToNexTech(NULL, "NexTech Backup", m_strInformation, m_strInformationDetails);
			return TRUE;
		} else if (nButton == eContactEmail) {
			CString str;
			str.Format("%s\r\n\r\n%s", m_strInformation, m_strInformationDetails);
			SendErrorEmail(m_pTaskDialogParent, str, "Backup Warning", false);
			return TRUE;
		} else if (nButton == eContactWebsite) {
			ShellExecute(NULL, NULL, "http://www.nextech.com/Contact/Support.aspx", NULL, NULL, SW_SHOW);
			return TRUE;
		} else if (nButton == IDCLOSE) {			
			ConfigPop();
			NavigatePage();
			return TRUE;
		}

		return NxTaskDialog::OnButtonClicked(nButton);
	};

#ifdef _DEBUG
	virtual void OnVerificationClicked(bool bChecked)
	{
		if (bChecked) {
			NxRegUtils::WriteLong(CString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\") + CString("Debug_IgnoreBackupMessages"), 1);
			SetElementText(TDE_FOOTER, "Backup messages are disabled. Clear the Debug_IgnoreBackupMessages value in HKLM\\Software\\NexTech or hold shift when starting to re-enable.");
		} else {
			NxRegUtils::WriteLong(CString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\") + CString("Debug_IgnoreBackupMessages"), 0);
			SetElementText(TDE_FOOTER, "Backup messages are enabled.");
		}
	};
#endif

	virtual void OnHelp()
	{
		BackupMsgBoxCallback(NULL);
	};

	virtual void OnCreated()
	{
		SetButtonElevationRequiredState(NxBackupTaskDialog::eNexTechBypass, TRUE);
	};
};

// (a.walling 2010-06-14 07:37) - PLID 38442 - Use a task dialog
BOOL Backup_TaskDialog(DWORD dwFlags, const CString& strBackupMessage, const CString& strBackupDetails, BOOL bNexTechUser, BOOL bForceBackup, BOOL bNeedsToRunServerBackup, BOOL bCanWindowsUserPerformBackup)
{
	if (IsNexTechInternal()) {
		// (c.haag 2004-05-11 09:04) - Handling if you're a NexTech user
		AfxMessageBox(FormatString("%s\r\n%s", strBackupMessage, strBackupDetails), MB_ICONINFORMATION);
		return FALSE;
	}

#ifdef _DEBUG
	long nIgnoreBackupMessages = 1;//NxRegUtils::ReadLong(CString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\") + CString("Debug_IgnoreBackupMessages"), 1);
	if (!bNexTechUser && nIgnoreBackupMessages) {
		return FALSE;
	}
	bNexTechUser = TRUE;
#endif

	int nResult = 0;

	{
		NxBackupTaskDialog dlg;
		dlg.m_strInformation = strBackupMessage;
		dlg.m_strInformationDetails = strBackupDetails;

#ifdef _DEBUG
		dlg.m_nIgnoreBackupMessages = nIgnoreBackupMessages;
#endif

		//
		// If the data backup on the server is more than a day old, we force a backup
		//
		// (a.walling 2010-06-04 13:45) - PLID 21048 - Critical -- must run a backup and prevent login
		if (bForceBackup && bNeedsToRunServerBackup) {

			// (j.armen 2014-01-24 10:15) - PLID 59681 - Support has situations where they want to allow an office into NexTech
			//	while they are investigating backup issues.  To make sure this is not an indefinite timeframe, we are using a DateTimeParam
			//	Once the date passes, the critical backup warning is enforced.
			COleDateTime dtBypassUntil = GetRemotePropertyDateTime("NexTechBypassCriticalBackup", &g_cdtSqlMin, 0, "<None>", true);
			if(dtBypassUntil.GetStatus() == COleDateTime::invalid || dtBypassUntil < COleDateTime::GetCurrentTime()) {
				dlg.Config()
					.ErrorIcon().ZeroButtons()
					.MainInstructionText(strBackupMessage).ExpandedInformationText(strBackupDetails)
					.ContentText("Practice cannot continue unless the critical server backup has completed successfully.")
					.AddCommand(NxBackupTaskDialog::eBackup, "Backup now\nThe backup process should only take a few minutes")
					.AddCommand(NxBackupTaskDialog::eContact, "Contact NexTech technical support\nWe can answer your questions and help resolve your backup issues")
					.AddCommand(NxBackupTaskDialog::eHelp, "Open help manual")
					.AddCommand(NxBackupTaskDialog::eClosePractice, "Close Practice");
			}
			else
			{
				dlg.Config()
					.ErrorIcon().ZeroButtons()
					.MainInstructionText(strBackupMessage).ExpandedInformationText(strBackupDetails)
					.ContentText("A critical backup has not been performed.\nYou are able to log in while NexTech Technical Support is investigating.")
					.AddCommand(NxBackupTaskDialog::eBypass, "Continue to log in")
					.AddCommand(NxBackupTaskDialog::eClosePractice, "Close Practice");
			}
		} else if (bNeedsToRunServerBackup) {
			dlg.Config()
				.WarningIcon().ZeroButtons()
				.MainInstructionText(strBackupMessage).ExpandedInformationText(strBackupDetails)
				.ContentText("The critical server backup did not complete successfully.")
				.AddCommand(NxBackupTaskDialog::eBackup, "Backup now and continue\nA new server backup will run in the background while you continue to use the software.")
				.AddCommand(NxBackupTaskDialog::eContact, "Contact NexTech technical support\nWe can answer your questions and help resolve your backup issues.")
				.AddCommand(NxBackupTaskDialog::eHelp, "Open help manual")
				.AddCommand(NxBackupTaskDialog::eClosePractice, "Close Practice");

			//strBkpMsg += "\nYour server backup was unsuccessful. A new server backup will run in the background while you continue to use the software.";
			//Backup_MsgBox(strBkpMsg, MB_OK | MB_HELP | MB_ICONINFORMATION, "Backup_Messages_and_Troubleshooting.htm");
		} else {
			//
			// If a less critical backup failed, give the user the option to back up
			//
			dlg.Config()
				.InformationIcon().ZeroButtons()
				.MainInstructionText(strBackupMessage).ExpandedInformationText(strBackupDetails)
				.ContentText("The backup did not complete successfully.");

			if (bCanWindowsUserPerformBackup) {
				dlg.Config().AddCommand(NxBackupTaskDialog::eBackup, "Backup now and continue\nA new server backup will run in the background while you continue to use the software.");
			} else {
				dlg.Config().AddCommand(NxBackupTaskDialog::eBypass, "Continue\nYour account is not authorized to start a backup. Please consult your office manager or network administator to resolve this issue.");
			}
			dlg.Config()
				.AddCommand(NxBackupTaskDialog::eContact, "Contact NexTech technical support\nWe can answer your questions and help resolve your backup issues.")
				.AddCommand(NxBackupTaskDialog::eHelp, "Open help manual")
				.AddCommand(NxBackupTaskDialog::eClosePractice, "Close Practice");

			/*
			if (bCanWindowsUserPerformBackup) {
				//strBkpMsg += "\nPlease consult your office manager or network administator to resolve this issue.";
				strContent = "Please consult your office manager or network administator to resolve this issue.";

				return FALSE;
			} else {
				// (a.walling 2010-06-04 13:45) - PLID 21048
				//strBkpMsg += "\nA new backup will run the background while you continue to use the software.";
			}
			*/
			//Backup_MsgBox(strBkpMsg, MB_OK | MB_HELP | MB_ICONINFORMATION, "Backup_Messages_and_Troubleshooting.htm");
		}

		if (bNexTechUser) {
			dlg.Config().AddCommand(NxBackupTaskDialog::eNexTechBypass, "Bypass backup\nLimited to authorized NexTech techical support staff.");
		}

		nResult = dlg.DoModal();
	}

	switch (nResult) {
		case NxBackupTaskDialog::eHelp:
			BackupMsgBoxCallback(NULL);
			return FALSE;
			break;
		case NxBackupTaskDialog::eBackup:
			{
				// (c.haag 2006-06-19 15:17) - PLID 21068 - Make sure our database is in the database list
				CString strDatabase;
				if(GetSubRegistryKey().IsEmpty()) {
					strDatabase = "pracdata";
				} else  {
					strDatabase.Format("pracdata_%s", GetSubRegistryKey());
				}
				NxBackupUtils::EnsureDatabaseInBackupList(NxRegUtils::ReadString(GetRegistryBase() + "NxServerIP"), strDatabase);

				// (a.walling 2010-06-04 13:45) - PLID 21048 - If this is not a 'forced' backup, then we don't need to be synchronous.
				if (bForceBackup) {
					dwFlags |= BACKUP_FLAG_CRITICAL;
					NxBackupUtils::Backup(GetMainFrame()->GetSafeHwnd(), dwFlags, NxRegUtils::ReadString(GetRegistryBase() + "NxServerIP"));
					return TRUE;
				} else {
					NxBackupUtils::NotifyBackup(dwFlags, NxRegUtils::ReadString(GetRegistryBase() + "NxServerIP"));
					return FALSE;
				}
			}
			break;
		case NxBackupTaskDialog::eNexTechBypass:
		case NxBackupTaskDialog::eBypass:
			return FALSE;
			break;
		default:
			if ( (nResult == NxBackupTaskDialog::eClosePractice) || (bForceBackup && bNeedsToRunServerBackup && !bNexTechUser) ) {
				PostQuitMessage(0);
				return FALSE;
			}
			return FALSE;
	};
}

// Exit codes from NxServerConfig button press:
// 0 - OK
// -1 - Cancel
int Backup_InvokeNxServerConfig(DWORD& dwExitCode)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	// Run the troubleshooter
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);		
	if (CreateProcess("nxserverconfig.exe", "-frompractice", NULL, NULL, false, 0, NULL, GetSharedPath(), &si, &pi))
	{
		CWaitCursor csr;
		WaitForSingleObject(pi.hProcess, INFINITE);

		// Get the exit code and see if the problem was fixed.
		// A return code of zero means it was fixed.
		GetExitCodeProcess(pi.hProcess, &dwExitCode);
	}
	else
		return -1;

	return 0;
}

class CNexTechErrorInfo;

class CASDEmailDlg : public CASDDlg
{
public:
	CASDEmailDlg(CWnd* pParent) : CASDDlg(pParent) {};

protected:
	// (a.walling 2010-01-13 14:47) - PLID 31253 - Update the other button text / style
	virtual void UpdateOtherButton()
	{
		m_nxibOther.AutoSet(NXB_MODIFY);
		m_nxibOther.SetWindowText("S&end to NexTech");

		CRect rcOther;
		m_nxibOther.GetWindowRect(rcOther);

		ScreenToClient(rcOther);
		long nWidthAdjust = rcOther.Width() / 2;
		rcOther.left -= nWidthAdjust;
		rcOther.right += nWidthAdjust;

		m_nxibOther.MoveWindow(rcOther);
	};

	virtual void OnOtherButtonClicked()
	{
		// (a.walling 2010-01-13 15:37) - PLID 31253 - Email the error to nextech
		SendErrorEmail(AfxGetMainWnd(), m_strErrorText);
	};

public:
	CString m_strErrorText;
};

void Backup_OnDBIntegrityStatus(const _DB_INTEGRITY_RESPONSE* pInfo, LPCTSTR szDatabase)
{
	// (c.haag 2005-12-08 17:39) - PLID 6908 - Only process this packet if
	// the integrity checks are active
	//
	if (pInfo->bDBIntegrityActive) {

		// If the integrity check failed, and we do not know the status
		// of the data integrity, warn the user
		//
		if (!pInfo->bDBIntegrityCheckSuccess) {
			// (a.walling 2011-11-29 16:49) - PLID 45060 - This needs the red box and a more strongly worded and informative message
			CASDEmailDlg dlg(NULL);

			dlg.m_strErrorText = FormatString("The integrity of the %s database could not be determined.", szDatabase);

			dlg.SetParams(
			FormatString(
				"The integrity of the %s database could not be determined.\r\n\r\n"
				"NexTech periodically runs diagnostics to ensure the validity of your data. Detecting data corruption early is critical to minimize any data loss and avoid any interruption with your business.\r\n\r\n"
				"Attempts to validate the data have been unsuccessful. An expert from our Technical Support staff will be happy to help ensure the diagnostics run properly. Continuing to use the system regardless of these warnings may jeopardize your data and should be limited to critical patient data.\r\n\r\n"
				"\r\n"
				"Please contact NexTech Technical Support as soon as possible to resolve this issue!\r\n\r\n"
				"\tEmail: support@nextech.com\r\n\r\n"
				"\tEast Coast, Midwest States, and Canada:\r\n"
				"\t\t(866) 654 4396\r\n\r\n"
				"\tWest Coast and Central States:\r\n"
				"\t\t(888) 417 8464\r\n\r\n"
				"\tOutside North America:\r\n"
				"\t\t+1 (937) 438 1095 or +1 (813) 425 9270\r\n"
				"",	szDatabase),
			"Database Integrity Warning",
			"DATABASE INTEGRITY WARNING",
			"I have read the warning above and fully understand the situation.", false);

			dlg.DoModal();

			LoadErrorInfoStrings();
			Log("DATABASE INTEGRITY WARNING -- dismissed by windows user '%s' on machine '%s' at '%s'", GetErrorInfoString(GEI_WINDOWS_USERNAME), GetErrorInfoString(GEI_COMPUTER_NAME), GetErrorInfoString(GEI_LOCAL_PATH));
		}
		// If the integrity check succeeded but we know the data is
		// corrupt, warn the user
		//
		else if (!pInfo->bDBIntegrityGood) {
			// (a.walling 2010-01-13 14:08) - PLID 31253
			CASDEmailDlg dlg(NULL);

			dlg.m_strErrorText = FormatString("Data corruption detected in %s database", szDatabase);

			dlg.SetParams(
			FormatString(
				"Corruption has been detected in the %s database during routine integrity checks.\r\n\r\n"
				"Attempts to repair the damage without data loss have failed. The extent of the damage cannot be automatically determined.\r\n\r\n"
				"To prevent further corruption and data loss, it is strongly recommended that no further activity occurs in the database until an expert from our Technical Support staff has analyzed your system. Continuing to use the system regardless of these warnings may jeopardize your data and should be absolutely limited to emergency access to critical patient data.\r\n\r\n"
				"\r\n"
				"Immediately contact NexTech Technical Support to resolve this issue!\r\n\r\n" // removed 'resolve and repair'
				"\tEmail: support@nextech.com\r\n\r\n"
				"\tEast Coast, Midwest States, and Canada:\r\n"
				"\t\t(866) 654 4396\r\n\r\n"
				"\tWest Coast and Central States:\r\n"
				"\t\t(888) 417 8464\r\n\r\n"
				"\tOutside North America:\r\n"
				"\t\t+1 (937) 438 1095 or +1 (813) 425 9270\r\n"
				"",	szDatabase),
			"Critical Data Corruption Warning",
			"CRITICAL DATA CORRUPTION WARNING", // one of few places where allcaps is probably justified. PAY ATTENTION!
			"I have read the warning above and fully understand the situation.", false);

			dlg.DoModal();

			LoadErrorInfoStrings();
			Log("DATA CORRUPTION WARNING -- dismissed by windows user '%s' on machine '%s' at '%s'", GetErrorInfoString(GEI_WINDOWS_USERNAME), GetErrorInfoString(GEI_COMPUTER_NAME), GetErrorInfoString(GEI_LOCAL_PATH));
		}
	}
}

void Backup_OnBackupUserCfg(const void* pData, unsigned long nDataSize)
{
	// (c.haag 2006-04-18 16:50) - PLID 19460 - Reads in the backup user configuration
	//
	const char* pcData = (const char*)pData;
	const _NXSERVER_BACKUP_USER_CFG* pCfg = (const _NXSERVER_BACKUP_USER_CFG*)pData;
	const char* szUserNames = pcData + sizeof(_NXSERVER_BACKUP_USER_CFG);
	g_bkpUserCfg = pCfg->m_cfg;
	g_astrbkpUsernames.RemoveAll();
	while (szUserNames && *szUserNames) {
		g_astrbkpUsernames.Add(szUserNames);
		szUserNames += strlen(szUserNames) + 1;
	}
}

BOOL Backup_CanWindowsUserPerformBackup()
{
	char szWindowsUsername[256];
	unsigned long nNameSize = 256;
	BOOL bNameInList = FALSE;
	long i;
	GetUserName(szWindowsUsername, &nNameSize);
	for (i=0; i < g_astrbkpUsernames.GetSize() && !bNameInList; i++) {
		if (0 == g_astrbkpUsernames[i].CompareNoCase(szWindowsUsername)) {
			bNameInList = TRUE;
		}
	}

	if ((eAllowAllExceptListed == g_bkpUserCfg && bNameInList) ||
		(eDenyAllExceptListed == g_bkpUserCfg && !bNameInList)) {
		return FALSE;				
	} else {
		return TRUE;
	}
}