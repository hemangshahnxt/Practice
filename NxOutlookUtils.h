#ifndef __NXOUTLOOKUTILS_H__
#define __NXOUTLOOKUTILS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning( push )
#pragma warning( disable : 4146 )
//TES 11/8/2007 - PLID 27979 - VS2008 - RGB is already a macro, and nothing uses the imported RGB anyway (it's
// a property of ColorFormat), so rename it.
// (a.walling 2014-05-30 16:50) - PLID 61989 - Never consult registry for #import for consistency
// (a.walling 2014-04-30 15:04) - PLID 61989 - Always using MSO9.tlb now
#import "MSO9.tlb" rename("RGB", "Office_RGB") no_registry
#import "MSOUTL9.OLB" no_registry
#pragma warning( pop )

#include "NxFolderField.h"

//
// This is a utility file used to provide an interface with the
// NxOutlookAddin. Any code specified here must be used strictly
// OUTSIDE the NxOutlookAddin, but this header file will need to
// be used by the add-in.
//

// Definitions
#define NXOLA_BUFFER_SIZE		512
#define ALL_OUTLOOK_USERS		NULL
#define ALL_OUTLOOK_FOLDERS		NULL
#define INVALID_ID				-1

#define OUTLOOK_MONITOR_EXE_NAME "NexPDALinkMonitor.exe"

// (c.haag 2005-07-26 16:49) - Synchronization types. At this time,
// these only apply to a Practice Outlook Profile's configuration
// settings. We may want to abandon our idea of sending the folder
// names through the pipe, and just send flags instead. The reason
// we did it the former way was that using a name instead of bitflags
// made it easier to scale...but practically, I doubt we'll ever scale
// past thirty-two different kinds of folders.
//
#define SYNC_FLAG_CALENDAR		0x00000001
#define SYNC_FLAG_CONTACTS		0x00000002
#define SYNC_FLAG_PATIENTS		0x00000004
#define SYNC_FLAG_NOTES			0x00000008
#define SYNC_FLAG_TODOS			0x00000010


// Pipe messages
#define NXOLPM_BASE							0x0010

#define NXOLPM_FINISHED						NXOLPM_BASE + 0x0001 //Sent when the client disconnects the pipe
#define NXOLPM_SHOW_INTERFACE				NXOLPM_BASE + 0x0002 //Sent to show the interface
#define NXOLPM_SHOW_AIRLOCK					NXOLPM_BASE + 0x0003 //Sent to show the airlock interface
#define NXOLPM_SYNCHRONIZE					NXOLPM_BASE + 0x0004 //Sent to synchronize specified folders
#define NXOLPM_SYNC_BEGIN					NXOLPM_BASE + 0x0005 //Sent from addin when writing to Outlook
#define NXOLPM_SYNC_END						NXOLPM_BASE + 0x0007 //Sent from the addin when done synchronizing
#define NXOLPM_SET_FOREGROUND				NXOLPM_BASE + 0x0008 //Tells the addin to send the window title to the monitor
#define NXOLPM_AIRLOCK_SET_FOREGROUND		NXOLPM_BASE + 0x0009 //Tells the addin to send the airlock title to the monitor
#define NXOLPM_RESTORE_MONITOR_DLG			NXOLPM_BASE + 0x000A //Shows the NexPDA monitor's dlg window

#define NXOLPM_ADD_MANAGER_REF				NXOLPM_BASE + 0x000B //Adds a "reference" to the NexPDA link so that it is
																 //aware that a third party is explicitly trying to use
																 //the manager. If the manager is dormant, it will initialize
																 //itself.

#define NXOLPM_RELEASE_MANAGER_REF			NXOLPM_BASE + 0x000C //Deletes the "reference". If the reference count is 0, and
																 //no explorer windows are open, then the NexPDA manager will shut down

#define NXOLPM_FORCE_SHUTDOWN				NXOLPM_BASE + 0x000D //Forces the "reference" count to 0, deletes all explorer
																 //and inspector windows and effectively terminates Outlook.

#define NXOLPM_NEXPDA_LINK_INIT_COMPLETE	NXOLPM_BASE + 0x000E // Called when the NexPDA link has initialized

#define NXOLPM_NEXPDA_LINK_TERM_COMPLETE	NXOLPM_BASE + 0x000F // Called when the NexPDA link has shut down

#define NXOLPM_NAMED_PIPES_READY			NXOLPM_BASE + 0x0010 // Sent when the addin's named pipe thread is ready and listening

#define NXOLPM_QUERY_SYNC_STATUS			NXOLPM_BASE + 0x0011 // Sent when the monitor wants to know if the addin is currently syncing

#define NXOLPM_NEXPDA_TT_AIRLOCK_TEXT		NXOLPM_BASE + 0x0012 // Called when the monitor is to display the pop-up tooltip balloon for the airlock


// Sent when an error has occured. Contains an eight-byte payload where the first
// four bytes are a long integer of the command requested, and the last four bytes
// is an error code.
#define NXOLPM_ERROR						NXOLPM_BASE + 0x0011



// Error messages
#define NXOLPEM_BASE						0x1000

#define NXOLPEM_MANAGER_SHUTTING_DOWN		NXOLPEM_BASE + 0x0001


// (z.manning 2009-10-19 13:22) - PLID 35997 - Definied this in a file that's shared with Practice
// (z.manning 2009-11-04 15:43) - PLID 35997 - Changed this as indicated by m.clark
// (b.savon 2012-05-07 16:12) - PLID 37288 - Handle the editable notes section
//TES 10/11/2013 - PLID 58771 - Updated the default subject line to not include phone numbers or full names.
//#define DEFAULT_SYNC_SUBJECT_LINE	"[Last][[, ]][First]    [Type] [[(]][Purposes][[)]] [MoveUp][Confirmed] [HomePhone]"
//#define DEFAULT_SYNC_SUBJECT_LINE	"[Last][[, ]][First]   [Type] [[(]][Purposes][[)]]  [[Home: ]][HomePhone]  [[Cell: ]][CellPhone]"
#define DEFAULT_SYNC_SUBJECT_LINE "[LastInitial][[, ]][FirstInitial]   [Type] [[(]][Purposes][[)]]"
#define DEFAULT_SYNC_NOTES_SECTION "[Notes]"


// Namespace
namespace NxOutlookUtils
{
	// (c.haag 2007-01-10 11:25) - PLID 24196 - We now use settings from
	// an ini file to handle NexPDA-specific profiles. The older functions
	// are kept for legacy support
	
	// Returns the path to the ini file. This is always the Practice install
	// path followed by "nexpda.ini"
	CString GetNexPDAProfilePath();

	// Reads a profile string
	CString ReadNexPDAProfileString(const CString& strKey, const CString& strDefault = "", const CString& strSection = "NxOutlookAddin");

	// Writes a profile string
	void WriteNexPDAProfileString(const CString& strKey, const CString& strValue, const CString& strSection = "NxOutlookAddin");

	// Reads a profile integer
	int ReadNexPDAProfileInt(const CString& strKey, int nDefault = 0, const CString& strSection = "NxOutlookAddin");

	// Writes a profile integer
	void WriteNexPDAProfileInt(const CString& strKey, int nValue, const CString& strSection = "NxOutlookAddin");

	/////////////////////////////////////////////////////////////////////////

	// Returns a string of the subkey that the NexPDA link is docked to.
	CString GetRegSubkey();

	// Returns the base path for the current subkey's registry settings.
	CString GetRegBase();

	// Returns the install path for the subkey currently being used by the NexPDA link.
	CString GetPracInstallPath();

	// Returns the path to where the value of whether or not the link should be enabled upon opening the NexPDA Monitor.
	CString GetLinkEnabledRegistryPath();

	// Returns the folder's name from its type value
	CString GetFolderNameFromType(Outlook::OlDefaultFolders eType);

	// Returns the name of the NexPDA addin's named pipe
	CString GetAddinPipeName();

	// Returns the name of the NexPDA monitor's named pipe
	CString GetMonitorPipeName();

	// Returns the session ID of the current terminal services session
	long GetTSSessionID();

	// Connect to the NexTech Outlook Add-In using a named pipe. This should
	// be called in the constructor.
	HANDLE Connect();

	// Disconnect from the Outlook Add-In. This should be called in the destructor.
	void Disconnect(HANDLE hAddIn);

	// Sends data to the Add-In. The format of the data streamed to the Add-In should
	// be:
	//
	// Size of payload (4 bytes)
	// Payload (n bytes)
	//
	void Send(HANDLE hAddIn, int nBytes, void* pPayload);

	// Determines whether or not the Outlook addin is currently running
	BOOL IsOutlookLinkActive();

	// Determines whether or not the Outlook link is connected. The difference between this and IsOutlookLinkActive
	// is that this function will return true even if the link is not completelely initialized.
	BOOL IsOutlookLinkConnected();

	// This will cause the Add-In to perform a synchronization with
	// Outlook. The possible permutations are:
	//
	// ALL_OUTLOOK_USERS,ALL_OUTLOOK_FOLDERS: All data will be synchronized
	// for all users.
	//
	// ALL_OUTLOOK_USERS,<foldername>: Synchronize a specific Outlook folder
	// for all users. This may be either "Calendar", "Contacts", "Todos"
	// or "Notes" (without the quotes).
	//
	// <UserID>,ALL_OUTLOOK_FOLDERS: All data will be synchronized for one user
	//
	// <UserID>,<foldername>: Synchronize a specific Outlook folder for a specific user
	//
	//
	// The data payload consists of the following format:
	//
	// UserID (4 bytes)
	// Outlook folder (0 bytes if szFolderName is ALL_OUTLOOK_FOLDERS; otherwise, n bytes)
	//
	void Synchronize(long nUserID, LPCTSTR szFolderName);

	// This will show the Add-In interface. The data payload consists of the following
	// format:
	//
	// Show (4 bytes) A value of zero means we need to hide the interface, and any
	// non-zero value means to show it.
	//
	void ShowAddIn(BOOL bShow = TRUE);

	// This will show the Airlock interface. The data payload consists of the following
	// format:
	//
	// Show (4 bytes) A value of zero means we need to hide the interface, and any
	// non-zero value means to show it.
	//
	void ShowAirlock(BOOL bShow = TRUE);

	// Sends a message to add 1 to the reference count of the manager for the NexPDA link
	void AddManagerReference();

	// Sends a message to release a reference to the manager for the NexPDA link
	void ReleaseManagerReference();

	// Sends a message to completely shut down the NexPDA link
	void ShutdownNexPDALink();

	// Asks the addin if it is currently performing a sync.
	void CheckForSynchronization();

	// This will show the NexPDA Link Monitor's dialog and bring it to the foreground
	void ShowMonitor();

	// Scans through all currently running processes to see is the Outlook monitor is running.
	BOOL IsOutlookMonitorRunning();
	BOOL IsOutlookMonitorRunningGlobal(); // This one doesn't check if it's running in current session (shouldn't be used for TS)

	// Executes the Outlook monitor if it's not already running.
	void EnsureOutlookMonitorIsRunning();

	// Named pipe functions to interact with the NexPDA Monitor
	BOOL SendToNexPDAMonitor(int nMsg);
	BOOL SendDataToNexPDAMonitor(int nMsg, void* pDataPayload, int nPayloadBytes);
	BOOL SendErrorToNexPDAMonitor(int nOriginalMsg, int nErrorCode);

	BOOL SendToNexPDAMonitor(void* pData, int nBytes = sizeof(int));

	// (z.manning 2009-10-19 15:21) - PLID 35997 - All this logic was stripped out of
	// CWizardCalendarSubjectDlg and moved here (and made to work globally) so that
	// Practice could also use this code for NexSync settings.
	static enum {
		flcCheckbox = 0,
		flcField = 1,
		flcAlias = 2
	} ESubjectLineFieldListColumns;
	void FillSubjectLineFieldList(LPDISPATCH lpFieldsList);
	void GetSupportedSubjectLineFields(CNxFolderFieldArray* paFields);
	void GenerateSubjectLinePreview(CEdit *peditSubject, CEdit *peditSubjectPreview);
	void HandleSubjectLineEditChange(CEdit *peditSubject, CEdit *peditSubjectPreview, LPDISPATCH lpFieldsList);
	void HandleSubjectLineFieldsListEditingFinished(CDialog *pdlg, CEdit *peditSubject, CEdit *peditSubjectPreview, LPDISPATCH lpFieldsList, long nRow, _variant_t varNewValue, IN OUT CString &strSubject);
};

#endif