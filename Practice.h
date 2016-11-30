// Practice.h : main header file for the PRACTICE application
//

// (j.armen 2011-10-26 17:17) - PLID 46139 - Removed Dead Code

#if !defined(AFX_PRACTICE_H__F2B94DAC_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_)
#define AFX_PRACTICE_H__F2B94DAC_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI
#include <NxAdvancedUILib/NxAdvancedUIApp.h>
#include "nxtwainlauncher.h"
#include "nxperform.h"
#include <NxAdvancedUILib/NxMenu.h>

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif


#define SCHEDULER_MODULE_NAME	"Scheduler"
#define PATIENT_MODULE_NAME		"Patients"
#define ADMIN_MODULE_NAME		"Administrator"
//#define BILLING_MODULE_NAME		"NxBill"
#define INVENTORY_MODULE_NAME	"Inventory"
#define FINANCIAL_MODULE_NAME	"Financial"
#define MARKET_MODULE_NAME		"Marketing"
#define REPORT_MODULE_NAME		"Reports"
#define REPORTDOC_MODULE_NAME	"Report Documents"
#define CONTACT_MODULE_NAME		"Contacts"
#define LETTER_MODULE_NAME		"Letter Writing"
#define SURGERY_CENTER_MODULE_NAME "Surgery Center"
// (d.thompson 2009-11-16) - PLID 36134
#define LINKS_MODULE_NAME		"Links"

#define DEFAULT_MODULE_NAME		SCHEDULER_MODULE_NAME
#define CREATE_LICENSE_KEY(strlicense)		::SysAllocString(strlicense)
#define SS_LICENCE_KEY							L"061001000029844750"

// (j.jones 2013-03-29 11:49) - PLID 54281 - Removed global defines for database size limit,
// they are now returned values in GetMaximumDatabaseSize() in GlobalUtils.

#pragma warning(push)
#pragma warning(disable : 4192 4146)
// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "InkObj.tlb" 
#pragma warning(pop)

/////////////////////////////////////////////////////////////////////////////
// CPracticeApp:
// See Practice.cpp for the implementation of this class
//

class CLoginDlg;
class CMainFrame;
class CPracticeInstanceData;
class CNxAPIManager;
class CGenericWordProcessorManager;

// (a.walling 2011-09-16 13:01) - PLID 45531 - Now derives from CWinAppEx
// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI - use CNxAdvancedUIApp
class CPracticeApp : public CNxAdvancedUIApp
{
	friend CMainFrame *GetMainFrame();
	friend CPracticeInstanceData;

public:
	CPracticeApp();
	virtual ~CPracticeApp();

	void WinHelp( DWORD dwData, UINT nCmd = HELP_CONTEXT );
	bool ShowOtherInstance();

	// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI - moved several helpers into CNxAdvancedUIApp
	// (a.walling 2010-07-29 11:37) - PLID 39871 - Wrote this to test, keeping it around for posterity.
	void CacheAllProps();
	// (a.walling 2007-05-04 10:03) - PLID 4850 - Include bInitialLogin var, and a CLoginDlg pointer, for switching users.
	// (z.manning 2015-12-03 10:41) - PLID 67637 - Added password param
	BOOL PostLogin(const CString &strPassword, BOOL bInitialLogin = TRUE, CWnd* pLoginDlg = NULL);//must be seen by the outside
	
	CDocument *OpenFile(LPCTSTR nameFile = NULL);
	CDocTemplate * GetTemplate(LPCTSTR TemplateName = NULL);

	CPalette m_palette;

	// (a.walling 2012-04-25 16:19) - PLID 49987 - This class is defined in the cpp; this can be used to construct and destroy objects in InitInstance and ExitInstance
	// without polluting the header file here with things that are unnecessary
	boost::scoped_ptr<CPracticeInstanceData> m_pPracticeInstanceData;
	
	CLoginDlg *m_dlgLogin;
	
	enum EPracticeFontType {
		pftGeneralBold,
		pftGeneralNotBold,
		pftSubtitle,
		pftTitle,
		pftFixed,
		pftMultiRes,
		pftSmall,
		// (j.jones 2016-05-12 14:39) - NX-100625 - added a non-bold header font
		pftHeader,
	};

	CFont m_boldFont;
	CFont m_notboldFont;
	CFont m_subtitleFont;
	CFont m_titleFont;
	// (j.jones 2016-05-12 14:39) - NX-100625 - added a non-bold header font
	CFont m_headerFont;
	CFont m_fixedFont;
	CFont m_multiresFont;
	CFont m_smallFont;
	
	CFont *GetPracticeFont(EPracticeFontType epftFontType);

	CNxTWAIN m_Twain;

	HANDLE GetDevModeHandle();
	void SetDevModeHandle(HANDLE hNewValue);
	HANDLE GetDevNamesHandle();
	void SetDevNamesHandle(HANDLE hNewValue);

	CPtrArray m_arypCaseHistories;
	void DeleteAllCaseHistories();

	// (a.walling 2012-06-04 16:30) - PLID 49829 - Load the current user's data from the db
	void LoadCurrentUserSettingsData();

	// (a.walling 2012-06-04 16:42) - PLID 50763 - Compress or uncompress the serialized settings data
	NxTreeSettingsDataPtr PrepareSettingsData(VARIANT& varData);
	_variant_t PrepareSettingsData(NxTreeSettingsDataPtr pData);

	virtual NxTreeSettingsDataPtr GetSettingsData(BOOL bAdmin, LPCTSTR szPath) override;
	// (a.walling 2012-06-04 16:30) - PLID 49829 - Save the current data to the db
	virtual void SaveSettingsData() override;

	//TES 3/2/2015 - PLID 64734 - Is the DebugMode registry setting currently turned on?
	BOOL GetDebugModeEnabled();
	//TES 3/2/2015 - PLID 64736 - Is a debugger currently attached to this process?
	BOOL GetDebugModeActive();
	//TES 3/2/2015 - PLID 64736 - Set the DebugMode registry setting, and attach/detach NxDebug accordingly
	void SetDebugMode(BOOL bDebug);
	//TES 3/2/2015 - PLID 64736 - Attach NxDebug to the current process
	void AttachNxDebug();
	//TES 3/2/2015 - PLID 64736 - Has DebugMode been turned on or off during this Practice session?
	BOOL GetDebugModeChangedThisSession() { return m_bDebugChangedThisSession; }

	// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI - moved several helpers into CNxAdvancedUIApp
protected:
	//TES 3/2/2015 - PLID 64736 - We need to track whether or not the user turned the DebugMode setting on or off during this Practice session
	BOOL m_bDebugChangedThisSession;
	
	// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI - Called when schema version saved is out of date
	virtual void OnSchemaOutOfDate(int nCurrentSchemaVersion, int nSavedSchemaVersion);
	
	// Use GetPracticeFont() to retrieve the font you want.  Eventually all the fonts will be 
	// moved here to be declared as protected, but for now just the ones that are user-
	// controlled (by preferences) are here because we don't want them to be accessed directly 
	// (since their loading can't be hard-coded).

protected:
	BOOL PreSplash();
	BOOL PreLogin();

	BOOL CreateFonts();

	BOOL EnsureWorkingPathAvail(BOOL bDisplayErrors);

	// (c.haag 2013-01-15) - PLID 59257 - This function is called to warn users if another
	// subkey has the same shared path we have.
	void WarnOtherSubkeysWithSameSharedPath();
	// (c.haag 2013-01-15) - PLID 59257 - This function determines whether this workstation
	// has at least one other subkey with the same shared path we have.
	BOOL OtherSubkeysHaveSameSharedPath();

	// (a.wilson 2014-09-30 12:46) - PLID 63567 - warn users if they currently have unresolved medicaton conflicts.
	void CheckFDBMedsHistory();

	void DefaultPrintSetup();
	
	void LoadPrinterSettingsFromRegistry();
	BOOL SavePrinterSettingsToRegistry();

	// (b.cardillo 2009-03-26 16:03) - PLID 14887 - We now create the file using win32 so we 
	// store a handle instead of using CFile
	// The handle that holds our NxLogin.dat file
	HANDLE m_hFileFolderAvail;

	// (b.cardillo 2011-01-06 20:34) - PLID 33748 - We now hold a global shared mutex for our working path too.
	HANDLE m_hNxLoginDatMutex;

	HANDLE m_hmutex;
	/* TODO: Commenting this out as part of a fix of the problem where non-administrator users were getting an error about registry permissions.
	COleTemplateServer m_server;// Server object for document creation
	//*/

	// (b.cardillo 2006-10-24 11:09) - PLID 19344 - Unused ink collector instantiated on start-
	// up and released on termination of the app.  DO NOT USE this variable.  It's only here to 
	// work around a Microsoft bug in the tablet pc ink controls.
	// TODO: Remove this, once Microsoft resolves our MSDN incident case #srx061019604066.
	MSINKAUTLib::IInkCollectorPtr m_pAlwaysOpenCollector;

	// (a.walling 2008-04-01 12:18) - PLID 29497 - GDI plus initialization
	// Now handled via helper class that prevents multiple per-process initialization. See NxGdiUtils.
	
protected:
	// (z.manning 2012-08-02 10:28) - PLID 51763 - The API manager object
	// (a.walling 2012-08-03 14:31) - PLID 51956 - Using scoped_ptr
	scoped_ptr<CNxAPIManager> m_pAPIManager;
public:
	CNxAPIManager* GetAPIObject();
	BOOL InitializeAPI(const CString &strPassword, CWnd* pParentDlg = NULL); // (b.savon 2015-12-16 09:29) - PLID 67718

	// (z.manning 2016-02-10 16:30) - PLID 68223 - Added members for handling word processing actions
protected:
	scoped_ptr<CGenericWordProcessorManager> m_pWordProcessorManager;
public:
	CGenericWordProcessorManager* GetWPManager();

// Overrides
	// (a.walling 2009-06-08 10:57) - PLID 34512 - Extend the inactivity timer if necessary
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPracticeApp)
	public:
	virtual BOOL InitInstance();
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPracticeApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileNew();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// (a.walling 2009-06-05 16:56) - PLID 34512 - Is this an input message?
bool IsInputMsg(const UINT message);

// (a.walling 2014-06-09 15:15) - PLID 53127 - Practice.h is already globally included
extern CPracticeApp theApp;

inline CPracticeApp* AfxGetPracticeApp()
{
	return &theApp;
}

// (a.walling 2014-06-09 15:15) - PLID 53127 - (CPracticeApp*)AfxGetApp() casts now use theApp

// (z.manning 2012-08-03 10:36) - PLID 51763 - This is simply a forwarding header for the API's method smart pointer
// so that we do not have to include the header in here.
namespace NexTech_Accessor { 
	struct _PracticeMethods;
};

// (a.walling 2012-08-03 14:31) - PLID 51956 - Compiler limits exceeded with imported NxAccessor typelib - moved to Practice unit

// (z.manning 2012-08-02 10:29) - PLID 51763 - Global function to access the API
NexTech_Accessor::_PracticeMethods* GetAPI();
_bstr_t GetAPILoginToken();
// (j.jones 2013-04-17 10:47) - PLID 56303 - added GetLoginTokenID, returns UserLoginTokensT.ID, could be null if using an outdated API
// (j.armen 2013-05-15 09:47) - PLID 56680 - No longer returns null 
long GetAPIUserLoginTokenID();
_bstr_t GetAPISubkey();

// (z.manning 2016-02-10 16:36) - PLID 68223 - Global function to load the word processor manager
CGenericWordProcessorManager* GetWPManager();

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRACTICE_H__F2B94DAC_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_)
