#ifndef __BACKUP_H__
#define __BACKUP_H__


#pragma once


#include "NxPackets.h"

#define INDETERMINANT_BACKUP_NAME	"There is no record of a recent successful backup."

typedef enum
{
	eBackupGeneral = 0,
	eBackupServer = 1,
	eBackupClient = 2,
	eBackupUser = 3,
	eBackupDocuments = 4,
	eBackupTemplates = 5,
	eBackupCustomReports = 6,
	eBackupImages = 7, // (z.manning 2008-09-30 09:36) - PLID 31532
} EBackupType;

CString GetLastBackupTime(EBackupType type);
CString GetNextBackupTime();
CString GetNextBackupTimeEnd();
CString GetUserBackupDirectory();
CString GetClientBackupDirectory();
CString GetLastBackupErrorText(EBackupType type);
BOOL Backup_IsIgnoringAsyncMessages();
void Backup_IgnoreAsyncMessages(BOOL bIgnore);
BOOL Backup_SynchronousGetStatus();
DWORD Backup_DetectNeededBackups();

//int Backup_MsgBox(const char* szMsg, DWORD dwFlags = MB_OK, const char* szBookmark = NULL);
// (a.walling 2010-06-14 07:37) - PLID 38442 - Use a task dialog
BOOL Backup_TaskDialog(DWORD dwFlags, const CString& strBackupMessage, const CString& strBackupDetails, BOOL bNexTechUser, BOOL bForceBackup, BOOL bNeedsToRunServerBackup, BOOL bCanWindowsUserPerformBackup);
int Backup_InvokeNxServerConfig(DWORD& dwExitCode);

void Backup_OnBackupMessage(BOOL bNexTechUser, _BACKUP_STATS* pBkpMsg);
BOOL Backup_OnDBInfoMessage(BOOL bNexTechUser, _BACKUP_STATS* pBkpMsg, _BACKUP_DOCTEMPLATE_CONFIG* pDocTmpCfg, _BACKUP_DATABKP_CONFIG* pBkpCfg);
void Backup_OnBackupErrorTextMessage(const char* sz, long nTotalLen);
void Backup_OnProgressPacket(_PACKET_BACKUP_PROGRESS* pStatus);
void Backup_OnRestorePacket(_PACKET_RESTORE_PROGRESS* pStatus);

void Backup_OnDBIntegrityStatus(const _DB_INTEGRITY_RESPONSE* pInfo, LPCTSTR szDatabase);
void Backup_OnBackupUserCfg(const void* pData, unsigned long nDataSize);
BOOL Backup_CanWindowsUserPerformBackup();

#endif