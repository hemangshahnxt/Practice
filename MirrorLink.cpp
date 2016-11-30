#include "stdafx.h"

// MirrorLink.cpp : implementation file
//

#include "MirrorLink.h"
#include "mirror.h"
#include "globaldatautils.h"
#include "dontshowdlg.h"
#include "practicerc.h"
#include "MirrorSettingsDlg.h"
#include "NxCanfieldLink.h"
#include "PracProps.h"
#include "GlobalDrawingUtils.h"
#include "NxPropManager.h"
#include "MirrorLinkCommonPtsDlg.h"

using namespace ADODB;
using namespace MirrorCross;
using namespace NXDATALISTLib;

extern CNxPropManager g_propManager;
extern const CString &GetNetworkLibraryString();
extern CString GetPassword();
extern long GetConnectionTimeout();
extern long GetCommandTimeout();

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (c.haag 2008-02-06 10:09) - PLID 28622 - Cleaned up these enumerations
enum {
	ecFirstContactDate = 0L,
	ecName,
	ecChartNumber,
	ecInternalMirrorID,
} EPtListColumns;

static void SetStatusBarText(HWND hWnd, LPCSTR lpszText)
{
	::PostMessage(hWnd, SB_SETTEXT, (255|0), (LPARAM)lpszText);
}


///////////////////////////////////////cross reference stuff
namespace MirrorCross
{
	bool bCrossReferencing = FALSE;

	bool IsRunning()
	{
		return bCrossReferencing;
	}

	UINT CrossRef(LPVOID pParam)
	{
		CROSSREF *p = (CROSSREF *)pParam;

		if (p->nextechList == NULL || p->statusBar == NULL || p->mirrorList == NULL ||
			p->exportList == NULL || p->importList == NULL)
			return -1;

		bCrossReferencing = TRUE;
		SetStatusBarText(p->statusBar, "Cross Referencing...");

		int nNexTechCount = p->nextechList->GetRowCount();
		int nExportCount = p->exportList->GetRowCount();
		int i;

		CString mirrorID;
		variant_t var;
		CWaitCursor wait;
		static CString output = "Ready";

		try
		{
			// Check for yellow rows in the nextech patient list
			for (i = 0; i < nNexTechCount; i++)
			{
				if (p->stopRequest)
				{
					bCrossReferencing = FALSE;
					SetStatusBarText(p->statusBar, "Cross-reference cancelled");
					AfxEndThread(0, 0);
				}
				try
				{
					IRowSettingsPtr pRow = p->nextechList->GetRow(i);
					var = pRow->Value[ecInternalMirrorID];
					if (var.vt == VT_BSTR)
					{	
						long nRow = p->mirrorList->FindByColumn(ecInternalMirrorID, var, 0, 0);
						if (nRow == -1) {
							nRow = p->importList->FindByColumn(ecInternalMirrorID, var, 0, 0);
							if (nRow == -1) {
								pRow->BackColor = 0xFFFF;
							} else {
								pRow->BackColor = MIRROR_COLOR;
								pRow = p->importList->GetRow(nRow);
								pRow->BackColor = NEXTECH_COLOR;
							}
						} else {
							pRow->BackColor = MIRROR_COLOR;
							pRow = p->mirrorList->GetRow(nRow);
							pRow->BackColor = NEXTECH_COLOR;
						}
					}
				}
				catch(...)
				{
					//do nothing for now, we are in a thread!
					output = "Error in Cross-Referencing Row";
				}
			}

			// Check for yellow rows in the export list
			for (i = 0; i < nExportCount; i++)
			{
				if (p->stopRequest)
				{
					bCrossReferencing = FALSE;
					SetStatusBarText(p->statusBar, "Cross-reference cancelled");
					AfxEndThread(0, 0);
				}
				try
				{
					IRowSettingsPtr pRow = p->exportList->GetRow(i);
					var = pRow->Value[ecInternalMirrorID];
					if (var.vt == VT_BSTR)
					{	
						long nRow = p->mirrorList->FindByColumn(ecInternalMirrorID, var, 0, 0);
						if (nRow == -1) {
							nRow = p->importList->FindByColumn(ecInternalMirrorID, var, 0, 0);
							if (nRow == -1) {
								pRow->BackColor = 0xFFFF;
							} else {
								pRow->BackColor = MIRROR_COLOR;
								pRow = p->importList->GetRow(nRow);
								pRow->BackColor = NEXTECH_COLOR;
							}
						} else {
							pRow->BackColor = MIRROR_COLOR;
							pRow = p->mirrorList->GetRow(nRow);
							pRow->BackColor = NEXTECH_COLOR;
						}
					}
				}
				catch(...)
				{
					//do nothing for now, we are in a thread!
					output = "Error in Cross-Referencing Row";
				}
			}
		}
		catch(...)
		{	output = "Error in Cross-Referencing Data";
		}
		SetStatusBarText(p->statusBar, output);
		bCrossReferencing = FALSE;
		return 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMirrorLink dialog

CMirrorLink::CMirrorLink()
	: CNxDialog(CMirrorLink::IDD, NULL),
	m_pathChecker(NetUtils::MirrorDataPath)
{
	//{{AFX_DATA_INIT(CMirrorLink)
	//}}AFX_DATA_INIT
	m_pThread = NULL;
	m_otherUserChanged = false;
	m_exporting = false;
	m_bMirrorFinishedRequerying = false;
	m_nStep = 0;
	m_bCancel61Requery = false;
	m_bForcedRequeryStop = false;
}

CMirrorLink::~CMirrorLink()
{
	KillThread();
}

void CMirrorLink::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMirrorLink)
	DDX_Control(pDX, IDC_BTN_FIXBADLINKS, m_fixbadlinks);
	DDX_Control(pDX, IDC_REINDEX_MIRROR, m_btnReindex);
	DDX_Control(pDX, IDC_REFRESH_MIRROR, m_refreshMirrorButton);
	DDX_Control(pDX, IDC_LINK, m_linkButton);
	DDX_Control(pDX, IDC_UNLINK, m_unlinkButton);
	DDX_Control(pDX, IDC_IMPORT_BTN, m_import);
	DDX_Control(pDX, IDC_EXPORT_BTN, m_export);
	DDX_Control(pDX, IDC_PRAC_REMOVE_ALL, m_pracRemAll);
	DDX_Control(pDX, IDC_PRAC_REMOVE, m_pracRem);
	DDX_Control(pDX, IDC_PRAC_ADD, m_pracAdd);
	DDX_Control(pDX, IDC_MIR_REMOVE_ALL, m_mirRemAll);
	DDX_Control(pDX, IDC_MIR_REMOVE, m_mirRem);
	DDX_Control(pDX, IDC_MIR_ADD, m_mirAdd);
	DDX_Control(pDX, IDC_NEXTECH_COUNT, m_nxstaticNextechCount);
	DDX_Control(pDX, IDC_NEXTECH_COUNT2, m_nxstaticNextechCount2);
	DDX_Control(pDX, IDC_MIRROR_COUNT, m_nxstaticMirrorCount);
	DDX_Control(pDX, IDC_MIRROR_COUNT2, m_nxstaticMirrorCount2);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMirrorLink, CNxDialog)
	//{{AFX_MSG_MAP(CMirrorLink)
	ON_BN_CLICKED(IDC_PRAC_ADD, OnPracAdd)
	ON_BN_CLICKED(IDC_PRAC_REMOVE, OnPracRemove)
	ON_BN_CLICKED(IDC_PRAC_REMOVE_ALL, OnPracRemoveAll)
	ON_BN_CLICKED(IDC_MIR_ADD, OnMirAdd)
	ON_BN_CLICKED(IDC_MIR_REMOVE, OnMirRemove)
	ON_BN_CLICKED(IDC_MIR_REMOVE_ALL, OnMirRemoveAll)
	ON_EN_KILLFOCUS(IDC_IMAGE_OVERRIDE, OnKillfocusOverride)
	ON_BN_CLICKED(IDC_EXPORT_BTN, OnExportBtn)
	ON_BN_CLICKED(IDC_IMPORT_BTN, OnImportBtn)
	ON_BN_CLICKED(IDC_UNLINK, OnUnlink)
	ON_BN_CLICKED(IDC_LINK, OnLink)
	ON_BN_CLICKED(IDC_BTN_LINK_COMMON_PATIENTS, OnBtnLinkCommonPatients)
	ON_BN_CLICKED(IDC_REFRESH_MIRROR, OnRefreshMirror)
	ON_BN_CLICKED(IDC_BTN_HELP, OnHelp)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_REINDEX_MIRROR, OnReindexMirror)
	ON_BN_CLICKED(IDC_BTN_MIRROR_ADVOPT, OnBtnMirrorAdvopt)
	ON_BN_CLICKED(IDC_BTN_FIXBADLINKS, OnBtnFixbadlinks)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_STOP_MIRROR_LOAD, OnBtnStopMirrorLoad)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CMirrorLink, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMirrorLink)
	ON_EVENT(CMirrorLink, IDC_NEXTECH, 3 /* DblClickCell */, OnDblClickCellNextech, VTS_I4 VTS_I2)
	ON_EVENT(CMirrorLink, IDC_EXPORT, 3 /* DblClickCell */, OnDblClickCellExport, VTS_I4 VTS_I2)
	ON_EVENT(CMirrorLink, IDC_IMPORT, 3 /* DblClickCell */, OnDblClickCellImport, VTS_I4 VTS_I2)
	ON_EVENT(CMirrorLink, IDC_MIRROR, 3 /* DblClickCell */, OnDblClickCellMirror, VTS_I4 VTS_I2)
	ON_EVENT(CMirrorLink, IDC_NEXTECH, 18 /* RequeryFinished */, OnRequeryFinishedNextech, VTS_I2)
	ON_EVENT(CMirrorLink, IDC_MIRROR, 18 /* RequeryFinished */, OnRequeryFinishedMirror, VTS_I2)
	ON_EVENT(CMirrorLink, IDC_NEXTECH, 7 /* RButtonUp */, OnRButtonUpNexTech, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

///////////////////////////////////////////////////////////////////////////
//CMirrorLink public functions

void CMirrorLink::OnRButtonUpNexTech(long nRow, short nCol, long x, long y, long nFlags) 
{
	CMenu menPopup;
	menPopup.m_hMenu = CreatePopupMenu();
	IRowSettingsPtr pRow = m_nextechList->GetRow(nRow);
	OLE_COLOR clr = pRow->BackColor;
	CWnd* pWnd = GetDlgItem(IDC_NEXTECH);
	CPoint pt;

	pt.x = x;
	pt.y = y;
	pWnd->ClientToScreen(&pt);

	switch (clr)
	{
	case MIRROR_COLOR:
		menPopup.InsertMenu(0, MF_BYPOSITION, 32700 /* Arbitrary number */, "This patient is properly linked with Canfield Mirror.");
		break;
	case 0x000FFFF:
		//
		// TODO: Make the act of clicking on this pop-up bring up the online help.
		//
		menPopup.InsertMenu(0, MF_BYPOSITION, 32700 /* Arbitrary number */, "This patient is incorrectly linked with Canfield Mirror.");
		break;
	default:
		menPopup.InsertMenu(0, MF_BYPOSITION, 32700 /* Arbitrary number */, "This patient is not linked with Canfield Mirror.");
		break;
	}

	menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
}

void CMirrorLink::DoFakeModal()
{
	m_failedToOpen = false;
	if (!IsWindow(GetSafeHwnd()) && !Create(IDD))
	{	HandleException(NULL, "Could not create Mirror Link window");
		return;
	}

	if (m_failedToOpen)
		return;

	if (!Mirror::HasMirrorLinkLicense()) {
		MsgBox("You do not have a license to access the link to Canfield Mirror for this session of Practice. Please contact your Sales Representative for assistance.");
		return;
	}

	// Force the opening of Canfield Mirror so we know the link is established
	// (c.haag 2006-12-01 10:35) - PLID 23725 - Check if we force Mirror 6.0 integration
	// (c.haag 2009-04-02 11:24) - PLID 33630 - New handling
	if (Mirror::eCSDK_Success == Mirror::InitCanfieldSDK(TRUE)) {
		// Get the version and put it in the title
		CString str;
		long nMajor, nMinor;
		Mirror::Get61Version(nMajor, nMinor);
		str.Format("Integration with Canfield Mirror Link %d.%d", nMajor, nMinor);
		SetWindowText(str);
	} else {
		// If any other error occured, the link will have already been disabled by now.
		// If the client is using Mirror 6.0, we don't support getting the version number
		SetWindowText("Integration with Canfield Mirror");
	}

	/*
	if (Mirror::IsMirrorEnabled() && GetPropertyInt("MirrorAllow61Link", 1))
	{
		switch (CanfieldLink::EnsureLink(GetSubRegistryKey(), GetNetworkLibraryString(), GetSqlServerName(), GetPassword(), GetConnectionTimeout(), GetCommandTimeout(), 0, &g_propManager, NULL, 0))
		{
		case S_OK:
			{
				// Get the version and put it in the title
				CString str;
				long nMajor, nMinor;
				Mirror::Get61Version(nMajor, nMinor);
				str.Format("Integration with Canfield Mirror Link %d.%d", nMajor, nMinor);
				SetWindowText(str);
			}
			break;
		case 0x80000000: // S_FAILED
			// (b.cardillo 2007-04-30 13:59) - PLID 25839 - We need to check for a new status that 
			// the NxCanfieldLink.dll can now set, which is that it timed out waiting for Mirror.  
			// The previous logic was that if it timed out, we should just try again.  But there 
			// were other bugs that caused subsequent attempts to give exceptions, so the timeouts 
			// would show up as weird things (like "Access Denied").  When we fixed those bugs, 
			// Practice started behaving according to the original intended logic, which was BAD 
			// because if Mirror was never going to succeed, Practice would effectively loop 
			// forever just trying over and over.  So now we're correcting that design by letting 
			// Practice know that it failed due to a timeout.  Right now, the correct behavior in 
			// such situation is to disable the link for the remainder of the session.
			if (CanfieldLink::GetLinkStatus() == CanfieldLink::eStatusLinkTimedOut) {
				if (!Mirror::g_bForceDisable) {
					MsgBox(MB_OK|MB_ICONEXCLAMATION, "%s", "The link from NexTech Practice to Canfield Mirror could not be esablished because Canfield Mirror did not respond in a timely manner.\r\n\r\nThe link will not be attempted until the next time Practice is started.");
					Mirror::g_bForceDisable = TRUE;
				}
			}
			break;
		default:
			if (!Mirror::g_bForceDisable)
			{
				MsgBox("The link from NexTech Practice to Mirror will be disabled for the remainder of this program session.");
				Mirror::g_bForceDisable = TRUE;
			}
			break;
		}
	}*/

	//put us in a modal-like state
	GetMainFrame()->EnableWindow(FALSE);
	EnableWindow(TRUE);

	// (c.haag 2004-12-15 13:54) - PLID 14525 - Show the reindexing button
	// if ctrl and shift are held down
	if (GetAsyncKeyState(VK_SHIFT) && GetAsyncKeyState(VK_CONTROL))
	{
		GetDlgItem(IDC_REINDEX_MIRROR)->ShowWindow(SW_SHOW);
	}
	else {
		GetDlgItem(IDC_REINDEX_MIRROR)->ShowWindow(SW_HIDE);
	}


	BOOL bRefreshSucceeded = RefreshLink();

	if (Mirror::IsMirrorEnabled())
	{
		if (m_failedToOpen || !bRefreshSucceeded)//refresh could have failed
		{
			if (Mirror::GetMirrorDataPath() == "")
			{
				if (IDYES == ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(),
					"The Mirror Link has not been properly set.\r\n"
					"Do you want to browse for your mirror data?", 
					"Mirror data could not be opened", MB_YESNO))
				{
					if (Mirror::BrowseMirrorPath())
					{
						if (RefreshLink())
							m_failedToOpen = FALSE;
					}
					else
					{
						Close();
						return;
					}
				}
				else
				{
					Close();
					return;
				}

				if (m_failedToOpen)
				{
					MsgBox("The mirror data could not be accessed. Please click on 'Reload Mirror Database' to get more information and attempt to access the data.");
					GetDlgItem(IDC_EXPORT_BTN)->EnableWindow(FALSE);
					GetDlgItem(IDC_IMPORT_BTN)->EnableWindow(FALSE);
					//GetDlgItem(IDC_REFRESH_MIRROR)->ShowWindow(TRUE);
				}
			}
			else {
				if (!IsJet40())
					MsgBox("Your system does not have the Microsoft Jet 4.0 Database drivers installed, and is therefore unable to access the Mirror data. Please consult your hardware administrator.");
				else
					MsgBox("The mirror data could not be accessed. Please click on 'Reload Mirror Database' to get more information and attempt to access the data.");
				GetDlgItem(IDC_EXPORT_BTN)->EnableWindow(FALSE);
				GetDlgItem(IDC_IMPORT_BTN)->EnableWindow(FALSE);
				//GetDlgItem(IDC_REFRESH_MIRROR)->ShowWindow(TRUE);
			}
		}
		else
		{
			if (Mirror::HasInvalidRECNUMs())
			{
				if (IDYES == MsgBox(MB_YESNO, "Practice has detected that some Mirror records need to be reindexed. The process may take anywhere from one to fifteen minutes depending on how many pictures you have. Would you like to do this now?"))
					OnReindexMirror();
			}
			GetDlgItem(IDC_EXPORT_BTN)->EnableWindow(TRUE);
			GetDlgItem(IDC_IMPORT_BTN)->EnableWindow(TRUE);
		}
	}
	else
	{
		MsgBox("The link to Mirror is disabled on this computer. You may not see any Mirror patient demographics unless the link is enabled.");
		m_statusBar.SetText("The link to Mirror is disabled on this computer.", 255, 0);
	}
	m_exportList->Clear();
	m_importList->Clear();

	ShowWindow(SW_SHOW);//show window last, so they don't see what we are doing
}

void CMirrorLink::SetOtherUserChanged()
{
	if (!m_exporting)
		m_otherUserChanged = true;
}

/////////////////////////////////////////////////////////////////////////////
// CMirrorLink message handlers

BOOL CMirrorLink::OnInitDialog() 
{
	CRect rcClient;
	CRect rcZero(0,0,0,0);
	GetClientRect(rcClient);
	m_statusBar.Create(WS_VISIBLE|WS_CHILD|CBRS_BOTTOM, rcClient, (this), AFX_IDW_STATUS_BAR);
	m_statusBar.SetSimple();
	m_Progress.Create(WS_CHILD, rcZero, &m_statusBar, 1);

	CNxDialog::OnInitDialog();

	// (c.haag 2009-03-31 13:33) - PLID 33630 - This variable is TRUE if we are using SDK functionality
	const BOOL bUsingSDKFunctionality = Mirror::IsUsingCanfieldSDK();
	if (!bUsingSDKFunctionality)
	{
		if (GetFileAttributes(Mirror::GetMirrorDataPath()) != 0xFFFFFFFF)
		{
			if (GetFileAttributes(Mirror::GetMirrorDataPath()) & FILE_ATTRIBUTE_READONLY)
			{
				if (GetDlgItem(IDC_EXPORT_BTN)->IsWindowEnabled())
					AfxMessageBox("You will be unable to export patients in Mirror because the Mirror database is read-only");	

				GetDlgItem(IDC_EXPORT_BTN)->EnableWindow(FALSE);
			}
			else {
				GetDlgItem(IDC_EXPORT_BTN)->EnableWindow(TRUE);
			}
		}
	}

	m_pracAdd.AutoSet(NXB_RIGHT);
	m_pracRem.AutoSet(NXB_LEFT);
	m_pracRemAll.AutoSet(NXB_LLEFT);
	m_mirAdd.AutoSet(NXB_RIGHT);
	m_mirRem.AutoSet(NXB_LEFT);
	m_mirRemAll.AutoSet(NXB_LLEFT);
	m_import.AutoSet(NXB_UP);
	m_export.AutoSet(NXB_EXPORT);
	m_btnCancel.AutoSet(NXB_CLOSE);

	CWnd *pMirrorList = GetDlgItem(IDC_MIRROR);

	if (pMirrorList)
		m_mirrorList = pMirrorList->GetControlUnknown();
	
	if (m_mirrorList == NULL)
	{	ASSERT(FALSE);
		Close();
		return TRUE;
	}

	if (m_pathChecker.Changed())
		Mirror::ResetMirrorDataPath();

	//LoadMirrorList();

	m_nextechList	= BindNxDataListCtrl(IDC_NEXTECH, false);
	m_importList	= BindNxDataListCtrl(IDC_IMPORT, false);
	m_exportList	= BindNxDataListCtrl(IDC_EXPORT, false);

	m_cross.mirrorList = m_mirrorList;
	m_cross.nextechList = m_nextechList;
	m_cross.importList = m_importList;
	m_cross.exportList = m_exportList;
	m_cross.statusBar = m_statusBar.GetSafeHwnd();

	if (bUsingSDKFunctionality) {
		GetDlgItem(IDC_REINDEX_MIRROR)->ShowWindow(SW_HIDE);
	}

	return TRUE;
}

extern _ConnectionPtr g_pConMirror;

BOOL CMirrorLink::RefreshLink()
{
	try
	{
		m_nextechList->Clear();
		m_importList->Clear();
		m_exportList->Clear();
		m_mirrorList->Clear();
		m_nStep = 0;

		// (c.haag 2008-02-06 10:09) - PLID 28622 - Disable the Link Common Patients button
		GetDlgItem(IDC_BTN_LINK_COMMON_PATIENTS)->EnableWindow(FALSE);

		if (!Mirror::IsMirrorEnabled())
		{
			// If the mirror link is disabled,just abort.
			m_statusBar.SetText("The link to Mirror is disabled on this computer.", 255, 0);
		}
		else
		{
			if (/*m_otherUserChanged || m_pathChecker.Changed()*/ TRUE)
			{	
				// (c.haag 2009-03-31 13:33) - PLID 33630 - This variable is TRUE if we are using SDK functionality
				const BOOL bUsingSDKFunctionality = Mirror::IsUsingCanfieldSDK();
				if (!bUsingSDKFunctionality && (Mirror::GetMirrorData() == NULL))
					return FALSE;

				m_statusBar.SetText("Loading...", 255, 0);
				LoadMirrorList();

				m_nextechList->Requery();
				m_otherUserChanged = false;
			}
		}
	}
	catch (...)
	{
		Close();
		return FALSE;
	}
	return TRUE;
}

void CMirrorLink::OnExportBtn() 
{
	if (!Mirror::IsMirrorEnabled())
	{
		MsgBox("The link to Mirror is disabled on this computer. To enable it, click on the Advanced Options button and uncheck the 'Disable the link on this computer' checkbox.");
		return;
	}
	if (!CheckCurrentUserPermissions(bioMirrorIntegration, SPT_______1___))
		return;

	CWaitCursor wait;
	long result,
		 id,
		 p;
	CString mirrorID;

	LPDISPATCH pDisp = NULL;
	CString strMsg;
	BOOL bAssumeOneMatchingNameLinks = FALSE;

	if (m_exportList->GetRowCount() == 0)
		return;

	strMsg.Format("Are you sure you wish to export the most recent demographics of %d patient(s) from NexTech Practice to Canfield Mirror?", m_exportList->GetRowCount());
	if (IDYES != MessageBox(strMsg, "Patient Export", MB_YESNO))
		return;
	
	if (m_exportList->GetRowCount() >= 10)
	{
		// Ask the user if he is sure he wants to do the export
		if (IDNO == MessageBox("You are exporting ten or more patients. It is STRONGLY recommended that you make a backup of your Canfield Mirror database before doing this. Are you sure wish to continue with the export?", "Canfield Mirror Integration", MB_YESNO))
			return;

		// Ask the user if he wants to be prompted when duplicate names appear in both databases
		// only once. It will update the patient automatically.
		switch (MessageBox("Patients that are not linked yet, but have one matching first and last name patient in Canfield Mirror, will be exported. "
			"Normally, you would be prompted as to whether to export this kind of patient as a new patient, or to export the data into an existing patient in "
			"Canfield Mirror. Because so many patients are being exported, you may assume that every one of these patients in NexTech Practice should be exported "
			"into existing Canfield Mirror patients with the same first and last name.\n\nDo you wish to make this assumption? If you select 'Yes', then you will not be prompted when a patient being exported has one matching name in Canfield Mirror; it will be assumed the two must be linked together.", "Canfield Mirror Integration", MB_YESNOCANCEL))
		{
			case IDYES:
				bAssumeOneMatchingNameLinks = TRUE;
				break;
			case IDCANCEL:
				return;
		}
	}

	m_exporting = true;
	try
	{
		p = m_exportList->GetFirstRowEnum();
		
		while (p)
		{
			MSG msg;
			m_exportList->GetNextRowEnum(&p, &pDisp);
			// (c.haag 2008-02-06 13:47) - PLID 28622 - We now discern Mirror rows from Practice rows. Not
			// doing so makes code maintenance much more difficult.
			IRowSettingsPtr pPracticeRow(pDisp);
			IRowSettingsPtr pMirrorRow;
			pDisp->Release();

			while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
				if (msg.message == WM_QUIT) return;
				else AfxGetApp()->PumpMessage();
			}
			// (c.haag 2004-10-11 14:36) - If the user closed the dialog,
			// stop the export
			if (!IsWindowVisible()) {
				return;
			}

			id = VarLong(pPracticeRow->Value[ecChartNumber]);
			mirrorID = VarString(pPracticeRow->Value[ecInternalMirrorID], "");
			result = Mirror::Export(id, mirrorID, bAssumeOneMatchingNameLinks);

			if (result == Mirror::Addnew)
			{	m_nextechList->TakeRow(pPracticeRow);
				pPracticeRow->Value[ecInternalMirrorID] = _bstr_t(mirrorID);
				pPracticeRow->BackColor = MIRROR_COLOR;
				pMirrorRow = m_mirrorList->GetRow(m_mirrorList->AddRow(pPracticeRow));
				pMirrorRow->BackColor = NEXTECH_COLOR;

				// (c.haag 2008-02-06 10:25) - PLID 28757 - If we link MRN's with Practice
				// user-defined ID's, then make sure the visible MRN is up to date.
				if (Mirror::GetLinkMirrorMRNToUserDefinedID()) {
					pMirrorRow->Value[ecChartNumber] = _bstr_t(AsString(id)); // (c.haag 2008-02-06 10:27) - PLID 28622 - The MRN column in the Mirror list MUST have VT_BSTR entries
				} else {
					// Newly exported records have no chart numbers
					_variant_t vNull;
					vNull.vt = VT_NULL;
					pMirrorRow->Value[ecChartNumber] = vNull;
				}
			}
			else if (result == Mirror::Update)
			{	m_nextechList->TakeRow(pPracticeRow);
				pPracticeRow->BackColor = MIRROR_COLOR;
				pPracticeRow->Value[ecInternalMirrorID] = _variant_t(mirrorID);

				long lRes = m_mirrorList->FindByColumn(ecInternalMirrorID, _variant_t(mirrorID), 0, 0);
				if (lRes != -1)
				{
					pMirrorRow = m_mirrorList->Row[lRes];
					pMirrorRow->BackColor = NEXTECH_COLOR;

					// (c.haag 2008-02-06 10:25) - PLID 28757 - If we link MRN's with Practice
					// user-defined ID's, then make sure the visible MRN is up to date.
					if (Mirror::GetLinkMirrorMRNToUserDefinedID()) {
						pMirrorRow->Value[ecChartNumber] = _bstr_t(AsString(id)); // (c.haag 2008-02-06 10:27) - PLID 28622 - The MRN column in the Mirror list MUST have VT_BSTR entries
					} else {
						// Don't change the existing MRN
					}
				}
			}
			else if (result == Mirror::Stop)
			{	break;
			}

			GetDlgItem(IDC_EXPORT)->UpdateWindow();
		}
	}
	NxCatchAll("Exception while exporting mirror patients");
	m_exporting = false;

	UpdateCount();
}

void CMirrorLink::OnImportBtn()
{
	BOOL bAssumeOneMatchingNameLinks = FALSE;
	if (!Mirror::IsMirrorEnabled())
	{
		MsgBox("The link to Mirror is disabled on this computer. To enable it, click on the Advanced Options button and uncheck the 'Disable the link on this computer' checkbox.");
		return;
	}
	if (!CheckCurrentUserPermissions(bioMirrorIntegration, SPT______0____))
		return;

	if (GetRemotePropertyInt("MirrorImportOverwrite", 0))
	{
		if (IDNO == MsgBox(MB_YESNO, "You have chosen to import patients from Canfield Mirror. In the case where a patient already exists in both NexTech Practice and Canfield Mirror, that patient's demographics will be overwritten with the corresponding demographics in Mirror. This means that any information missing from that patient in Mirror MAY BE UNRECOVERABLY LOST from NexTech Practice! You may change this behavior from the Advanced Options window.\n\nAre you sure you wish to continue?"))
			return;
	}
	else
	{
		if (IDNO == MsgBox(MB_YESNO, "You have chosen to import patients from Canfield Mirror. In the case where a patient already exists in both NexTech Practice and Canfield Mirror, the patient will be linked, but the patient demographics (Address, SSN, etc.) will not be imported from Mirror. You may change this behavior from the Advanced Options window.\n\nDo you still wish to continue?"))
			return;
	}

	// (c.haag 2009-03-31 13:33) - PLID 33630 - This variable is TRUE if we are using SDK functionality
	const BOOL bUsingSDKFunctionality = Mirror::IsUsingCanfieldSDK();

	// (c.haag 2008-02-06 10:25) - PLID 28757 - If MRN or SSN linking is enabled, warn
	// the user that the number will be overwritten with the patient chart number
	if (Mirror::GetLinkMirrorSSNToUserDefinedID()) {
		if (bUsingSDKFunctionality) {
			if (IDNO == MsgBox(MB_YESNO | MB_ICONWARNING, "The Mirror integration is configured to have a patient's Mirror SSN linked with the patient's chart number in Practice. "
				"Performing this import will overwrite any existing SSN information for Mirror patients being imported.\n\n"
				"Are you SURE you wish to continue?"))
			{
				return;
			}
		} else {
			if (IDNO == MsgBox(MB_YESNO | MB_ICONWARNING, "The Mirror integration is configured to have a patient's Mirror SSN linked with the patient's chart number in Practice. "
				"Any selected Mirror patients which do not already exist in NexTech Practice will have their SSN updated to the chart number of their newly created corresponding patient in Practice.\n\n"
				"Are you SURE you wish to continue?"))
			{
				return;
			}
		}
	}
	if (Mirror::GetLinkMirrorMRNToUserDefinedID()) {
		if (bUsingSDKFunctionality) {
			if (IDNO == MsgBox(MB_YESNO | MB_ICONWARNING, "The Mirror integration is configured to have a patient's Mirror MRN linked with the patient's chart number in Practice. "
				"Performing this import will overwrite any existing MRN information for Mirror patients being imported.\n\n"
				"Are you SURE you wish to continue?"))
			{
				return;
			}
		} else {
			if (IDNO == MsgBox(MB_YESNO | MB_ICONWARNING, "The Mirror integration is configured to have a patient's Mirror MRN linked with the patient's chart number in Practice. "
				"Any selected Mirror patients which do not already exist in NexTech Practice will have their MRN updated to the chart number of their newly created corresponding patient in Practice.\n\n"
				"Are you SURE you wish to continue?"))
			{
				return;
			}
		}
	}

	CWaitCursor wait;
	long result,
		 p;
	CString recnum;
	LPDISPATCH pDisp = NULL;
	long userID;

	if (m_importList->GetRowCount() >= 10)
	{
		// Ask the user if he is sure he wants to do the export
		if (IDNO == MessageBox("You are importing ten or more patients. It is STRONGLY recommended that you make a backup of your NexTech Practice database before doing this. Are you sure wish to continue with the import?", "Canfield Mirror Integration", MB_YESNO))
			return;

		// Ask the user if he wants to be prompted when duplicate names appear in both databases
		// only once. It will update the patient automatically.
		switch (MessageBox("Patients that are not linked yet, but have one matching first and last name patient in NexTech, will be imported. "
			"Normally, you would be prompted as to whether to import this kind of patient as a new patient, or to import the data into an existing patient in Practice. "
			"Because so many patients are being imported, you may assume that every one of these patients in Canfield Mirror should be imported into existing NexTech Practice "
			"patients with the same first and last name.\n\nDo you wish to make this assumption? If you select 'Yes', then you will not be prompted when a patient being imported has one matching name in NexTech Practice; it will be assumed the two must be linked together.", "Canfield Mirror Integration", MB_YESNOCANCEL))
		{
			case IDYES:
				bAssumeOneMatchingNameLinks = TRUE;
				break;
			case IDCANCEL:
				return;
		}	
	}

	m_exporting = true;//well, we aren't exporting, but we need to do this here too
	try
	{
		p = m_importList->GetFirstRowEnum();
		
		while (p)
		{
			MSG msg;
			long nPersonID;
			m_importList->GetNextRowEnum(&p, &pDisp);
			// (c.haag 2008-02-06 13:43) - PLID 28622 - We now discern Mirror rows from Practice rows. Not
			// doing so makes code maintenance much more difficult.
			IRowSettingsPtr pMirrorRow(pDisp);
			IRowSettingsPtr pPracticeRow;
			pDisp->Release();

			while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
				if (msg.message == WM_QUIT) return;
				else AfxGetApp()->PumpMessage();
			}
			// (c.haag 2004-10-11 14:36) - If the user closed the dialog,
			// stop the import
			if (!IsWindowVisible()) {
				return;
			}

			recnum = VarString(pMirrorRow->Value[ecInternalMirrorID]);

			// (c.haag 2008-03-17 17:51) - PLID 28757 - It's possible that this update will change the patient's
			// chart number if we're using pre-6.1 logic and existing demographics are overwritten. In that case,
			// we need to fetch the old ID so that we can update it with the new ID in the visible list.
			long nOldUserDefinedID = -1;
			if (Mirror::GetLinkMirrorMRNToUserDefinedID() && !bUsingSDKFunctionality && GetRemotePropertyInt("MirrorImportOverwrite", 0))
			{
				_RecordsetPtr prsOld = CreateRecordset("SELECT UserDefinedID FROM PatientsT WHERE MirrorID = '%s'",
					_Q(recnum));
				if (!prsOld->eof) {
					nOldUserDefinedID = AdoFldLong(prsOld, "UserDefinedID");
				}
			}

			if (Mirror::Stop == (result = Mirror::Import(recnum, nPersonID, bAssumeOneMatchingNameLinks)))
				break;
			else if (Mirror::Skip == result)
			{				
				continue;
			}

			_RecordsetPtr prs = CreateRecordset("SELECT UserDefinedID FROM PatientsT WHERE PersonID = %d",
				nPersonID);
			userID = AdoFldLong(prs, "UserDefinedID");

			// (c.haag 2008-02-06 10:25) - PLID 28757 - If we link MRN's with Practice
			// user-defined ID's, then make sure the visible Mirror MRN is up to date. This only applies
			// to versions 6.1 or better of Mirror. Pre-6.1 versions actually overwrite Practice
			// chart numbers if the setting is turned on.
			if (Mirror::GetLinkMirrorMRNToUserDefinedID()) {
				if (result == Mirror::Addnew || result == Mirror::Update) {
					// Update the MRN of the Mirror row
					pMirrorRow->Value[ecChartNumber] = _bstr_t(AsString(userID)); // (c.haag 2008-02-06 10:27) - PLID 28622 - The MRN column in the Mirror list MUST have VT_BSTR entries
				} else {
					// Don't change the existing MRN
				}
			} else {
				// Don't change the existing MRN
			}

			if (result == Mirror::Addnew)
			{	m_mirrorList->TakeRow(pMirrorRow);				
				pMirrorRow->BackColor = NEXTECH_COLOR;
				pPracticeRow = m_nextechList->GetRow(m_nextechList->AddRow(pMirrorRow));

				// (c.haag 2008-02-06 10:25) - PLID 28757 - pPracticeRow now points to the Practice row. No
				// matter what the GetLinkMirrorMRNToUserDefinedID() setting is, we know userID is
				// indeed the practice chart number (refer to the query above).
				pPracticeRow->Value[ecChartNumber] = userID;

				pPracticeRow->BackColor = MIRROR_COLOR;
			}
			else if (result == Mirror::Update)
			{	m_mirrorList->TakeRow(pMirrorRow);
				pMirrorRow->BackColor = NEXTECH_COLOR;
				long nSearchID = (nOldUserDefinedID > -1) ? nOldUserDefinedID : userID;
				long nRow;
				nRow = m_nextechList->FindByColumn(ecChartNumber, nSearchID, 0, 0);
				if (nRow > -1)
				{
					pPracticeRow = m_nextechList->GetRow(nRow);
					pPracticeRow->BackColor = MIRROR_COLOR;
					pPracticeRow->Value[ecInternalMirrorID] = _bstr_t(recnum);
					// (c.haag 2008-02-06 10:25) - PLID 28757 - pPracticeRow now points to the Practice row. No
					// matter what the GetLinkMirrorMRNToUserDefinedID() setting is, we know userID is
					// indeed the practice chart number (refer to the query above).
					pPracticeRow->Value[ecChartNumber] = userID;
				}
				nRow = m_exportList->FindByColumn(ecChartNumber, nSearchID, 0, 0);
				if (nRow > -1)
				{
					// (a.walling 2008-03-05 16:08) - PLID 29215 - We used to say m_nextechList->GetRow(nRow); we
					// should be using the exportList since that is the index we have.
					pPracticeRow = m_exportList->GetRow(nRow);
					pPracticeRow->BackColor = MIRROR_COLOR;
					pPracticeRow->Value[ecInternalMirrorID] = _bstr_t(recnum);
					// (c.haag 2008-02-06 10:25) - PLID 28757 - pPracticeRow now points to the Practice row. No
					// matter what the GetLinkMirrorMRNToUserDefinedID() setting is, we know userID is
					// indeed the practice chart number (refer to the query above).
					pPracticeRow->Value[ecChartNumber] = userID;
				}
			}
		}
	}
	NxCatchAll("exception while importing mirror patients");
	m_exporting = false;

	UpdateCount();
}

int CMirrorLink::DoModal() 
{
	//do not call DoModal on this dialog
	ASSERT(FALSE);
	return IDC_CANCEL;
}

void CMirrorLink::OnCancel() 
{
	Close();
}

void CMirrorLink::OnPracAdd() 
{
	if (MirrorCross::IsRunning())
		KillThread();
	int curSel = m_nextechList->CurSel;

	if (curSel != -1)	//we need to cross ref, in case we haven't yet
	{
		m_exportList->TakeCurrentRow(m_nextechList);
/*
		var = pRow->Value[MirrorID];
		if (var.vt == VT_BSTR)
		{	int nMirror = m_mirrorList->FindByColumn(MirrorID, var, 0, 0);
			if (nMirror == -1)//row not found in mirror
				pRow->BackColor = 0xFFFF;
			else
			{	pRow->BackColor = MIRROR_COLOR;
				pRow = m_mirrorList->GetRow(nMirror);
//				pRow->BackColor = NEXTECH_COLOR;
			}
		}
		*/
	}
	UpdateCount();
}

void CMirrorLink::OnPracRemove() 
{
	if (MirrorCross::IsRunning())
		KillThread();
	int curSel = m_exportList->CurSel;

	if (curSel != -1)
		m_nextechList->TakeCurrentRow(m_exportList);

	UpdateCount();
}

void CMirrorLink::OnPracRemoveAll() 
{
	if (MirrorCross::IsRunning())
		KillThread();
	try
	{
		long p = m_exportList->GetFirstRowEnum();
		LPDISPATCH pRow = NULL;
		
		while (p)
		{
			m_exportList->GetNextRowEnum(&p, &pRow);
			m_nextechList->TakeRow(pRow);
			pRow->Release();
		}
	}
	NxCatchAll("Could not remove all rows from export list");

	UpdateCount();
}

void CMirrorLink::OnMirAdd() 
{
	if (MirrorCross::IsRunning())
		KillThread();
	int curSel = m_mirrorList->CurSel;

	if (curSel != -1)
		m_importList->TakeCurrentRow(m_mirrorList);

	UpdateCount();
}

void CMirrorLink::OnMirRemove() 
{
	if (MirrorCross::IsRunning())
		KillThread();
	int curSel = m_importList->CurSel;

	if (curSel != -1)
		m_mirrorList->TakeCurrentRow(m_importList);

	UpdateCount();
}

void CMirrorLink::OnMirRemoveAll() 
{
	if (MirrorCross::IsRunning())
		KillThread();
	try
	{
		long p = m_importList->GetFirstRowEnum();
		LPDISPATCH pRow = NULL;
		
		while (p)
		{
			m_importList->GetNextRowEnum(&p, &pRow);
			m_mirrorList->TakeRow(pRow);
			pRow->Release();
		}
	}
	NxCatchAll("Could not remove all rows from import list");

	UpdateCount();
}

void CMirrorLink::OnDblClickCellNextech(long nRowIndex, short nColIndex) 
{
	m_nextechList->CurSel = nRowIndex;
	OnPracAdd();
}

void CMirrorLink::OnDblClickCellExport(long nRowIndex, short nColIndex) 
{
	m_exportList->CurSel = nRowIndex;
	OnPracRemove();
}

void CMirrorLink::OnDblClickCellMirror(long nRowIndex, short nColIndex) 
{
	m_mirrorList->CurSel = nRowIndex;
	OnMirAdd();
}

void CMirrorLink::OnDblClickCellImport(long nRowIndex, short nColIndex) 
{
	m_importList->CurSel = nRowIndex;
	OnMirRemove();	
}

void CMirrorLink::OnKillfocusOverride() 
{
	CString str;
	
	GetDlgItemText(IDC_IMAGE_OVERRIDE, str);
	Mirror::SetMirrorImagePath(str);
}

void CMirrorLink::OnSize(UINT nType, int cx, int cy) 
{
	if (m_Progress.GetSafeHwnd() && m_Progress.IsWindowVisible()) {
		RepositionProgressBar();
	}

	CNxDialog::OnSize(nType, cx, cy);
}

void CMirrorLink::RepositionProgressBar()
{
	CRect rcStatusBar;
	m_statusBar.GetWindowRect(&rcStatusBar);
	m_Progress.SetWindowPos(&wndTop, 110,6,200,rcStatusBar.Height() - 10, 0);
}

void CMirrorLink::OnRequeryFinishedNextech(short nFlags) 
{
	UpdateCount();

	// (c.haag 2009-03-31 13:33) - PLID 33630 - This variable is TRUE if we are using SDK functionality
	const BOOL bUsingSDKFunctionality = Mirror::IsUsingCanfieldSDK();
	if (bUsingSDKFunctionality)
	{
		return;
	}

	if (!m_mirrorList->IsRequerying()) {
		// (c.haag 2008-02-06 10:09) - PLID 28622 - Enable the Link Common Patients button
		// (c.haag 2008-03-05 10:45) - PLID 28622 - But only if the user didn't cancel the requery
		if (!m_bForcedRequeryStop) {
			GetDlgItem(IDC_BTN_LINK_COMMON_PATIENTS)->EnableWindow(TRUE);
		}
		CrossRef();
	}
}

// Not called if Mirror 6.1 is installed
void CMirrorLink::OnRequeryFinishedMirror(short nFlags) 
{
	UpdateCount();
	if (!m_nextechList->IsRequerying()) {
		// (c.haag 2008-02-06 10:09) - PLID 28622 - Enable the Link Common Patients button
		// (c.haag 2008-03-05 10:45) - PLID 28622 - But only if the user didn't cancel the requery
		if (!m_bForcedRequeryStop) {
			GetDlgItem(IDC_BTN_LINK_COMMON_PATIENTS)->EnableWindow(TRUE);
		}
		CrossRef();	
	}
}

void CMirrorLink::OnRefreshMirror() 
{
	if (!Mirror::IsMirrorEnabled())
	{
		MsgBox("The link to Mirror is disabled on this computer. To enable it, click on the Advanced Options button and make sure the 'Disable the link on this computer' checkbox is unchecked, and that you are connected to the correct Mirror database.");
		return;
	}

	if (Mirror::EnsureMirrorData())
	{
		if (GetFileAttributes(Mirror::GetMirrorDataPath()) == -1)
		{
			switch (GetLastError())
			{
			case ERROR_PATH_NOT_FOUND:
			case ERROR_FILE_NOT_FOUND:
				MsgBox("The mirror data cannot be found. Please ensure the Mirror server is turned on.\n\nIf the Mirror server is on, go to the 'Advanced Options' menu and look at the box near the bottom where it says \"Location of the Mirror Database\" and make sure the file path name in the box is correct. You can click on the box to browse to change the file path.");
				break;
			case ERROR_ACCESS_DENIED:
				MsgBox("You do not have the necessary permissions in Microsoft Windows to access the Mirror data.\n\nPlease contact your system administrator for support.");
				break;
			default:
				CString strError;	
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
				strError.ReleaseBuffer();
				MsgBox(CString("Error loading Mirror data: ") + strError);
				break;
			}
		}
		return;
	}

	//m_mirrorList->Requery();
	LoadMirrorList();
}

void CMirrorLink::OnLink() 
{
	if (!Mirror::IsMirrorEnabled())
	{
		MsgBox("The link to Mirror is disabled on this computer. To enable it, click on the Advanced Options button and uncheck the 'Disable the link on this computer' checkbox.");
		return;
	}

	try
	{
		int nSel;
		long p;
		LPDISPATCH pRow;

		// (c.haag 2009-03-31 13:33) - PLID 33630 - This variable is TRUE if we are using SDK functionality
		const BOOL bUsingSDKFunctionality = Mirror::IsUsingCanfieldSDK();

		////////////////////////////////////////////////////////////////
		// Make sure only one item is selected in the mirror list
		p = m_mirrorList->GetFirstSelEnum();
		nSel = 0;
		while (p)
		{
			m_mirrorList->GetNextSelEnum(&p, &pRow);
			nSel++;
		}
		if (nSel != 1)
		{
			MsgBox("Please select one patient from the Mirror patient list to link");
			return;
		}

		////////////////////////////////////////////////////////////////
		// Make sure only one item is selected in the nextech list
		p = m_nextechList->GetFirstSelEnum();
		nSel = 0;
		while (p)
		{
			m_nextechList->GetNextSelEnum(&p, &pRow);
			nSel++;
		}
		if (nSel != 1)
		{
			MsgBox("Please select one patient from the NexTech patient list to link");
			return;
		}

		if (IDNO == MsgBox(MB_YESNO, "Are you sure you wish to link the selected patient in the NexTech list with the selected patient in the Mirror list?"))
			return;

		// (c.haag 2008-02-06 10:25) - PLID 28757 - If MRN or SSN linking is enabled,
		// warn the user that the number will be overwritten with the patient chart number
		if (Mirror::GetLinkMirrorSSNToUserDefinedID()) {
			if (IDNO == MsgBox(MB_YESNO | MB_ICONWARNING, "The Mirror integration is configured to have a patient's Mirror SSN linked with the patient's chart number in Practice. "
				"Performing this link will overwrite any existing SSN information for the patient in Mirror.\n\n"
				"Are you SURE you wish to continue?"))
			{
				return;
			}
		}
		if (Mirror::GetLinkMirrorMRNToUserDefinedID()) {
			if (IDNO == MsgBox(MB_YESNO | MB_ICONWARNING, "The Mirror integration is configured to have a patient's Mirror MRN linked with the patient's chart number in Practice. "
				"Performing this link will overwrite any existing MRN information for the patient in Mirror.\n\n"
				"Are you SURE you wish to continue?"))
			{
				return;
			}
		}

		int nMirror = m_mirrorList->CurSel,
			nNextech = m_nextechList->CurSel;

		variant_t	varMirror, 
					varNextech;

		varMirror = m_mirrorList->Value[nMirror][ecInternalMirrorID];
		varNextech = m_nextechList->Value[nNextech][ecChartNumber];

		if (varMirror.vt == VT_BSTR && varNextech.vt == VT_I4)
		{	IRowSettingsPtr pRow;
			variant_t varOldMirror;

			//create the link
			Mirror::Link(VarLong(varNextech), VarString(varMirror));

			if (!bUsingSDKFunctionality) {
				// (c.haag 2008-02-06 10:16) - PLID 28757 - Update the mirror MRN/SSN if necessary
				// (c.haag 2008-03-17 17:12) - We can't do this from Mirror::Link because other functions
				// in the Mirror namespace call it during imports and exports (which I may add is a terrible
				// idea in an ages-old implementation)
				BOOL bLinkMirrorSSNToUserDefinedID = Mirror::GetLinkMirrorSSNToUserDefinedID();
				BOOL bLinkMirrorMRNToUserDefinedID = Mirror::GetLinkMirrorMRNToUserDefinedID();

				if (bLinkMirrorSSNToUserDefinedID || bLinkMirrorMRNToUserDefinedID) {

					CString str;

					if (S_OK == Mirror::EnsureMirrorData()) {
						if (bLinkMirrorSSNToUserDefinedID)
						{
							str.Format("UPDATE M2000 SET PIN = '%d' WHERE RECNUM = '%s'", VarLong(varNextech), _Q(VarString(varMirror)));
							g_pConMirror->Execute(_bstr_t(str), NULL, adCmdText);
						}
						// To be consistent with legacy logic, we have an else condition here
						else if (bLinkMirrorMRNToUserDefinedID)
						{
							// Update the mirror database too
							str.Format("UPDATE M2000 SET REFNO = '%d' WHERE RECNUM = '%s'", VarLong(varNextech), _Q(VarString(varMirror)));
							g_pConMirror->Execute(_bstr_t(str), NULL, adCmdText);
						}
					}
				}
			}

			//remove old color
			varOldMirror = m_nextechList->Value[nNextech][ecInternalMirrorID];
			if (varOldMirror.vt == VT_BSTR)
			{	long nOldMirror = m_mirrorList->FindByColumn(ecInternalMirrorID, varOldMirror, 0, FALSE);
				if (nOldMirror != -1)
				{	pRow = m_mirrorList->GetRow(nOldMirror);
					pRow->BackColor = dlColorNotSet;
				}
			}

			//set new color in nextech list
			pRow = m_nextechList->GetRow(nNextech);
			pRow->BackColor = MIRROR_COLOR;//NEXTECH_COLOR;

			// Set Mirror ID in NexTech list
			pRow->Value[ecInternalMirrorID] = varMirror;

			//set new color in mirror list
			pRow = m_mirrorList->GetRow(nMirror);
			pRow->BackColor = NEXTECH_COLOR;//MIRROR_COLOR;			

			// (c.haag 2008-02-06 10:25) - PLID 28757 - If we link MRN's with Practice user-defined ID's,
			// then make sure the visible MRN for the Mirror row is up to date.
			if (Mirror::GetLinkMirrorMRNToUserDefinedID()) {
				pRow->Value[ecChartNumber] = _bstr_t(AsString(VarLong(varNextech))); // (c.haag 2008-02-06 10:27) - PLID 28622 - The MRN column in the Mirror list MUST have VT_BSTR entries
			} else {
				// Don't change the existing MRN
			}
		}

		MsgBox("The link was successfully established");
	} NxCatchAll("Could not link patient");
}

void CMirrorLink::OnUnlink() 
{
	if (!Mirror::IsMirrorEnabled())
	{
		MsgBox("The link to Mirror is disabled on this computer. To enable it, click on the Advanced Options button and uncheck the 'Disable the link on this computer' checkbox.");
		return;
	}

/*	int nNextech = m_nextechList->CurSel;

	if (nNextech != -1)
	{	variant_t	varNextech;

		varNextech = m_nextechList->Value[nNextech][ID];

		if (varNextech.vt == VT_I4)
		{	IRowSettingsPtr pRow;
			variant_t varOldMirror;

			//destroy the link
			Mirror::Unlink(VarLong(varNextech));

			//remove old color
			varOldMirror = m_nextechList->Value[nNextech][MirrorID];
			if (varOldMirror.vt == VT_BSTR)
			{	long nOldMirror = m_mirrorList->FindByColumn(MirrorID, varOldMirror, 0, FALSE);
				if (nOldMirror != -1)
				{	pRow = m_mirrorList->GetRow(nOldMirror);
					pRow->BackColor = dlColorNotSet;
				}
			}

			//set new color
			pRow = m_nextechList->GetRow(nNextech);
			pRow->BackColor = dlColorNotSet;
		}
	}*/

	try
	{
		long p = m_nextechList->GetFirstSelEnum();
		IRowSettingsPtr pRow = NULL;
		if (!p)
			return;

		if (IDNO == MessageBox("Are you absolutely sure you wish to unlink all the patients selected in the NexTech Practice database list from their respective entries in Canfield Mirror?", "Canfield Mirror Integration", MB_YESNO))
		{
			return;
		}
	
		// Do for all highlighted items in the NexTech list
		while (p)
		{
			variant_t varNextech, varOldMirror;

			m_nextechList->GetNextSelEnum(&p, (IDispatch**)&pRow);
			varNextech = pRow->Value[ecChartNumber];
			
			// Unlink the item
			Mirror::Unlink(VarLong(varNextech));

			//remove old color
			varOldMirror = pRow->Value[ecInternalMirrorID];
			if (varOldMirror.vt == VT_BSTR)
			{					
				long nOldMirror = m_mirrorList->FindByColumn(ecInternalMirrorID, varOldMirror, 0, FALSE);
				if (nOldMirror != -1)
				{	IRowSettingsPtr pMirrorRow;
					pMirrorRow = m_mirrorList->GetRow(nOldMirror);
					pMirrorRow->BackColor = dlColorNotSet;
				}
			}

			// remove mirror id from practice record
			_variant_t vNull;
			vNull.vt = VT_NULL;
			pRow->Value[ecInternalMirrorID] = vNull;

			//set new color
			pRow->BackColor = dlColorNotSet;
		}
	}
	NxCatchAll("Could not unlink patient");

}

// (c.haag 2008-02-06 10:09) - PLID 28622 - This has been depreciated
/*
void CMirrorLink::OnGenerateCommonList()
{
	if (!Mirror::IsMirrorEnabled())
	{
		MsgBox("The link to Mirror is disabled on this computer. To enable it, click on the Advanced Options button and uncheck the 'Disable the link on this computer' checkbox.");
		return;
	}

	long p = m_mirrorList->GetFirstRowEnum();
	LPDISPATCH pDisp = NULL;

	if (!m_mirrorList->GetRowCount()) return;

	AfxMessageBox("This function will take all of the names that exist in both the NexTech Practice and Canfield Mirror databases and add them to the 'Patients to Export to Mirror' list. This is useful when you are linking patients between NexTech Practice and Canfield Mirror for the first time. No data on either database will be modified.");
		
	while (p)
	{	
		COleVariant varName;
		long lRow;

		m_mirrorList->GetNextRowEnum(&p, &pDisp);
		IRowSettingsPtr pRow(pDisp);
		varName = pRow->Value[1];

		lRow = m_nextechList->FindByColumn(1 varName, 0, FALSE);
		IRowSettingsPtr pPracticeRow(m_nextechList->GetRow(lRow));

		m_exportList->TakeRow(pPracticeRow);

		pDisp->Release();
	}
	UpdateCount();
}*/

void CMirrorLink::OnBtnLinkCommonPatients()
{
	// (c.haag 2008-02-06 10:09) - PLID 28622 - This function will invoke the
	// dialog that lets the user link patients that exist in both Practice and
	// Mirror
	try {
		CMirrorLinkCommonPtsDlg dlg(this);
		// Open the dialog
		dlg.Open(m_nextechList, m_mirrorList);
		// (c.haag 2009-03-31 13:33) - PLID 33630 - This variable is TRUE if we are using SDK functionality
		const BOOL bUsingSDKFunctionality = Mirror::IsUsingCanfieldSDK();

		// Now requery the interface since patients may have been linked. This
		// is consistent with how legacy code does it...but only do so if any
		// data was actually changed from the dialog
		if (dlg.GetChangedData()) {
			m_bMirrorFinishedRequerying = false;
			BOOL bRefreshSucceeded = RefreshLink();
			if (!Mirror::IsMirrorEnabled() || (!bUsingSDKFunctionality && !bRefreshSucceeded))
			{
				GetDlgItem(IDC_EXPORT_BTN)->EnableWindow(FALSE);
				GetDlgItem(IDC_IMPORT_BTN)->EnableWindow(FALSE);
			}
			else
			{
				GetDlgItem(IDC_EXPORT_BTN)->EnableWindow(TRUE);
				GetDlgItem(IDC_IMPORT_BTN)->EnableWindow(TRUE);
			}
		}
	}
	NxCatchAll("Error in CMirrorLink::OnBtnLinkCommonPatients");
}

HBRUSH CMirrorLink::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	switch (pWnd->GetDlgCtrlID())
	{
		case IDC_MIRROR_COUNT:
		case IDC_MIRROR_COUNT2:
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x009C9CDA));
			return (HBRUSH)GetStockObject(NULL_BRUSH);
		case IDC_NEXTECH_COUNT:
		case IDC_NEXTECH_COUNT2:
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x00D79D8C));
			return (HBRUSH)GetStockObject(NULL_BRUSH);
		case IDC_HIRES:
//NxDialog cannot tell if a window is shown, or hidden
//	so we need to make all hidden controls WS_EX_TRANSPARENT 
//	so the background always draws under them.
//We also use WS_EX_TRANSPARENT on static text labels, so the NxColor shows through.
//OnCtlColor uses both a CTLSTATIC for static text, radio and check boxes
//	NxDialog returns a NULL_BRUSH to draw the background 
//if the window is WS_EX_TRANSPARENT and the message is a CTLSTATIC.
//	This works on all platforms except WindowsXP
//where using a NULL_BRUSH to draw the background of a check box or radio button
//draws black.
//The only solution is to overide NxDialog for the specific radio or check button
			return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}*/

	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}


///////////////////////////////////////////////////////////////////////////
//CMirrorLink internal functions

void CMirrorLink::UpdateCount()
{
	// CAH 10/25: I had to use this quick hack because of SetDlgItemInit's
	// failure to properly refresh the text box

	CRect rect;
	CString str;

	if (!Mirror::IsMirrorEnabled())
	{
		GetDlgItem(IDC_MIRROR_COUNT)->SetWindowText("                   0");
	}

	if (!m_nextechList->IsRequerying())
	{
		str.Format("         %d", m_nextechList->GetRowCount());
		GetDlgItem(IDC_NEXTECH_COUNT)->SetWindowText(str);
	}

	if (Mirror::IsMirrorEnabled() && !m_mirrorList->IsRequerying() && m_bMirrorFinishedRequerying)
	{
		str.Format("                        %d", m_mirrorList->GetRowCount());
		GetDlgItem(IDC_MIRROR_COUNT)->SetWindowText(str);
	}

	str.Format("         %d", m_exportList->GetRowCount());
	GetDlgItem(IDC_NEXTECH_COUNT2)->SetWindowText(str);

	if (Mirror::IsMirrorEnabled())
	{
		str.Format("         %d", m_importList->GetRowCount());
		GetDlgItem(IDC_MIRROR_COUNT2)->SetWindowText(str);
	}	
}

void CMirrorLink::CrossRef()
{
	KillThread();
	m_cross.stopRequest = false;
	m_pThread = AfxBeginThread(MirrorCross::CrossRef, (void *)&m_cross, THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED);
	m_pThread->m_bAutoDelete = FALSE;
	m_pThread->ResumeThread();
}

HRESULT CMirrorLink::RequeryMirrorList61()
{
	m_nextechList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	m_bMirrorFinishedRequerying = false;
	m_mirrorList->Clear();
	SetStatusBarText(m_statusBar, "Loading Mirror list...");

	// (c.haag 2009-03-31 13:33) - PLID 33630 - This variable is TRUE if we are using SDK functionality
	const BOOL bUsingSDKFunctionality = Mirror::IsUsingCanfieldSDK();
	if (bUsingSDKFunctionality) {
		// (c.haag 2004-10-11 11:06) - We only support progress bars
		// and patient counting with 6.1+
		m_nMirrorPatients = Mirror::GetPatientCount();
		if (m_nMirrorPatients > 0) {
			m_Progress.ShowWindow(SW_SHOW);
			m_Progress.SetRange(0, 10000);
			m_Progress.SetPos(0);
			RepositionProgressBar();
		}
	}
	GetDlgItem(IDC_MIRROR_COUNT)->SetWindowText("         <loading...>");
	m_bCancel61Requery = false;
	m_bForcedRequeryStop = false;
	SetTimer(100, 20, NULL);
	//return Mirror::FillList61(m_mirrorList);
	return 0;
}

UINT RequeryMirrorList61(LPVOID pParam)
{
	CMirrorLink* pLink = (CMirrorLink*)pParam;
	HRESULT hRes = pLink->RequeryMirrorList61();
	if (!hRes)
	{
		pLink->UpdateCount();
		pLink->m_bMirrorFinishedRequerying = true;
		if (!pLink->m_nextechList->IsRequerying())
		{
			pLink->m_cross.stopRequest = false;
			MirrorCross::CrossRef((void*)&pLink->m_cross);	
		}
	}
	return hRes;
}

void CMirrorLink::KillThread()
{
	if (m_pThread)
	{	m_pThread->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
		m_cross.stopRequest = true;
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);
		delete m_pThread;
		m_pThread = NULL;
	}
}

void CMirrorLink::LoadMirrorList()
{
	CString con;
	CString path;
	LPDISPATCH pCon;

	try
	{
		// (c.haag 2009-03-31 13:33) - PLID 33630 - This variable is TRUE if we are using SDK functionality
		const BOOL bUsingSDKFunctionality = Mirror::IsUsingCanfieldSDK();
		if (bUsingSDKFunctionality)
		{
			CWaitCursor w;
			SetStatusBarText(m_statusBar, "Loading Practice list...");
			RequeryMirrorList61();
		}
		else
		{
			pCon = Mirror::GetMirrorData();
			if (pCon)
			{	m_mirrorList->AdoConnection = pCon;
				m_mirrorList->Requery();
			}
		}
	}
	catch (...)
	{
		Close();
	}
}

void CMirrorLink::Close()
{
//in case we are still open
	m_failedToOpen = true;

//stop refreshing the patient list
	Mirror::CancelFillList61();

//pretend to close the window
	GetMainFrame()->EnableWindow(TRUE);
	ShowWindow(SW_HIDE);

// Update all the views to reflect any changes we've made
	GetMainFrame()->UpdateAllViews();
}

void CMirrorLink::OnHelp()
{
	//(j.anspach 06-09-2005 10:53 PLID 16662) - Updating the OpenManual call to work with the new help system.
	OpenManual("NexTech_Practice_Manual.chm", "Links/Mirror_Link/link_not_established.htm");
}
void CMirrorLink::OnReindexMirror() 
{
	if (!Mirror::IsMirrorEnabled())
	{
		MsgBox("The link to Mirror is disabled on this computer. To enable it, click on the Advanced Options button and uncheck the 'Disable the link on this computer' checkbox.");
		return;
	}
	if (Mirror::GetMirrorData() == NULL)
	{
		MsgBox("The mirror data could not be accessed. Please click on 'Reload Mirror Database' to get more information and attempt to access the data.");
		return;
	}
	if (IDNO == MsgBox(MB_YESNO, "Are you absolutely sure you wish to reindex the Mirror database?"))
		return;
	if (!Mirror::HasInvalidRECNUMs())
	{
		if (IDNO == MsgBox(MB_YESNO, "Practice has not detected any incorrectly indexed records. Are you still sure you wish to perform the reindexing?"))
			return;
	}

	int nRepairedRecords = 0;
	MsgBox("Please have anyone using the Canfield Mirror program close it, and then click on the OK button");
	BeginWaitCursor();

	// Make a backup if it doesn't already exist
	CString strDataPath = Mirror::GetMirrorDataPath();
	CopyFile(strDataPath, strDataPath + ".pre-reindexing", TRUE);

	if (!Mirror::RepairImageTable(nRepairedRecords))
	{
		if (!Mirror::RepairM2000Table(nRepairedRecords))
			MsgBox("The operation has completed successfully. %d records were reindexed.",
				nRepairedRecords);
	}
	EndWaitCursor();
}

void CMirrorLink::OnBtnMirrorAdvopt() 
{
	CMirrorSettingsDlg dlg(this);
	if (IDOK == dlg.DoModal())
	{
		// Refresh the cache because the dialog invokes certain NxCanfieldLink.dll
		// functions that write to ConfigRT
		// (c.haag 2005-11-07 16:27) - PLID 16595 - This is now obselete; but we still flush in regular intervals
		//FlushRemotePropertyCache();

		// Turn off the forced disabling so we can try to connect again
		Mirror::g_bForceDisable = FALSE;
		m_bMirrorFinishedRequerying = false;
		BOOL bRefreshSucceeded = RefreshLink();
		// (c.haag 2009-03-31 13:33) - PLID 33630 - This variable is TRUE if we are using SDK functionality
		const BOOL bUsingSDKFunctionality = Mirror::IsUsingCanfieldSDK();
		if (!Mirror::IsMirrorEnabled() || (!bUsingSDKFunctionality && !bRefreshSucceeded))
		{
			GetDlgItem(IDC_EXPORT_BTN)->EnableWindow(FALSE);
			GetDlgItem(IDC_IMPORT_BTN)->EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_EXPORT_BTN)->EnableWindow(TRUE);
			GetDlgItem(IDC_IMPORT_BTN)->EnableWindow(TRUE);
		}
	}
}

int CMirrorLink::GetImproperlyLinkedCount()
{
	int nCount = 0;
	for (int i=0; i < m_nextechList->GetRowCount(); i++)
	{
		IRowSettingsPtr pRow = m_nextechList->GetRow(i);
		if (pRow->BackColor == 0xFFFF)
			nCount++;
	}
	return nCount;
}

void CMirrorLink::OnBtnFixbadlinks() 
{
	if (!Mirror::IsMirrorEnabled())
	{
		MsgBox("The link to Mirror is disabled on this computer. To enable it, click on the Advanced Options button and uncheck the 'Disable the link on this computer' checkbox.");
		return;
	}

	int nImproperlyLinkedPts = GetImproperlyLinkedCount();

	if (!nImproperlyLinkedPts)
	{
		MsgBox("There are no improperly linked patients between NexTech Practice and Canfield Mirror.");
		return;
	}

	if (IDNO == MsgBox(MB_YESNO, "There are %d patients who are improperly linked between NexTech Practice and Mirror. These patients are designated by yellow-colored highlights in the Practice patient list They will be marked as unlinked, and then automatically linked to the patient with a matching name in the Mirror database if there is one. Are you sure you would like to do this now?", nImproperlyLinkedPts))
		return;

	// (c.haag 2008-02-06 10:26) - PLID 28757 - If MRN or SSN linking is enabled,
	// warn the user that the number will be overwritten with the patient chart number
	if (Mirror::GetLinkMirrorSSNToUserDefinedID()) {
		if (IDNO == MsgBox(MB_YESNO | MB_ICONWARNING, "The Mirror integration is configured to have a patient's Mirror SSN linked with the patient's chart number in Practice. "
			"Performing this repair will overwrite any existing SSN information for improperly linked patients in Mirror.\n\n"
			"Are you SURE you wish to continue?"))
		{
			return;
		}
	}
	if (Mirror::GetLinkMirrorMRNToUserDefinedID()) {
		if (IDNO == MsgBox(MB_YESNO | MB_ICONWARNING, "The Mirror integration is configured to have a patient's Mirror MRN linked with the patient's chart number in Practice. "
			"Performing this repair will overwrite any existing MRN information for improperly linked patients in Mirror.\n\n"
			"Are you SURE you wish to continue?"))
		{
			return;
		}
	}

	for (int i=0; i < m_nextechList->GetRowCount(); i++)
	{
		IRowSettingsPtr pRow = m_nextechList->GetRow(i);
		if (pRow->BackColor == 0xFFFF)
		{
			variant_t varOldMirror;

			////////////////////////////////////////////////////
			// Unlink the patient
			Mirror::Unlink(VarLong(pRow->Value[ecChartNumber]));

			//remove old color
			varOldMirror = pRow->Value[ecInternalMirrorID];
			if (varOldMirror.vt == VT_BSTR)
			{					
				long nOldMirror = m_mirrorList->FindByColumn(ecInternalMirrorID, varOldMirror, 0, FALSE);
				if (nOldMirror != -1)
				{	IRowSettingsPtr pMirrorRow;
					pMirrorRow = m_mirrorList->GetRow(nOldMirror);
					pMirrorRow->BackColor = dlColorNotSet;
				}
			}

			// remove mirror id from practice record
			_variant_t vNull;
			vNull.vt = VT_NULL;
			pRow->Value[ecInternalMirrorID] = vNull;

			//set new color
			pRow->BackColor = dlColorNotSet;


			////////////////////////////////////////////////////
			// Link the patient
			
			// Search for a matching patient
			long nMirrorRow = m_mirrorList->FindByColumn(ecName,
				pRow->Value[ecName], 0, 0);
			if (nMirrorRow != -1)
			{
				Mirror::Link(VarLong(pRow->Value[ecChartNumber]), VarString(m_mirrorList->GetValue(nMirrorRow, ecInternalMirrorID)));

				//set new color in nextech list
				pRow->BackColor = MIRROR_COLOR;//NEXTECH_COLOR;

				// Set Mirror ID in NexTech list
				pRow->Value[ecInternalMirrorID] = m_mirrorList->GetValue(nMirrorRow, ecInternalMirrorID);

				//set new color in mirror list
				pRow = m_mirrorList->GetRow(nMirrorRow);
				pRow->BackColor = NEXTECH_COLOR;//MIRROR_COLOR;			
			}
		}
	}
}

void CMirrorLink::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == 100)
	{		
		KillTimer(nIDEvent);

		if (Mirror::IsMirrorEnabled())
		{
			CString strRecnum;
			if (m_bCancel61Requery || S_OK != Mirror::FillList61Incremental(m_mirrorList, m_nStep, strRecnum))
			{
				m_nStep = 0;
				m_bMirrorFinishedRequerying = true;
				m_Progress.ShowWindow(SW_HIDE);
				SetStatusBarText(m_statusBar, "Ready");
				GetDlgItem(IDC_BTN_STOP_MIRROR_LOAD)->EnableWindow(FALSE);
				// (c.haag 2008-02-06 10:09) - PLID 28622 - Enable the Link Common Patients button
				// (c.haag 2008-03-05 10:45) - PLID 28622 - But only if the user didn't cancel the requery
				if (!m_bForcedRequeryStop) {
					GetDlgItem(IDC_BTN_LINK_COMMON_PATIENTS)->EnableWindow(TRUE);
				}
				UpdateCount();

				if (!m_bCancel61Requery)
					CrossRef();
			}
			else
			{
				// (c.haag 2004-02-24 13:05) - Cross reference the new name right off the bat
				COleVariant v = (LPCTSTR)strRecnum;
				long nMirrorRow = m_mirrorList->FindByColumn(ecInternalMirrorID, v, 0, 0);
				if (nMirrorRow != -1)
				{
					// Get the mirror row of this patient
					IRowSettingsPtr pRowMirror = m_mirrorList->GetRow(nMirrorRow);

					// Color the NexTech list
					long nPracticeRow = m_nextechList->FindByColumn(ecInternalMirrorID, v, 0, 0);
					long nExportRow = m_exportList->FindByColumn(ecInternalMirrorID, v, 0, 0);
					if (nPracticeRow != -1)
					{
						IRowSettingsPtr pRowNexTech = m_nextechList->GetRow(nPracticeRow);
						if (pRowNexTech != NULL) {
							pRowMirror->BackColor = NEXTECH_COLOR;
							pRowNexTech->BackColor = MIRROR_COLOR;
						}
					}
					// Now color the export list					
					if (nExportRow != -1)
					{
						IRowSettingsPtr pRowNexTech = m_exportList->GetRow(nExportRow);
						if (pRowNexTech != NULL) {
							pRowMirror->BackColor = NEXTECH_COLOR;
							pRowNexTech->BackColor = MIRROR_COLOR;
						}
					}
				}

				if (!GetDlgItem(IDC_BTN_STOP_MIRROR_LOAD)->IsWindowEnabled())
				{
					GetDlgItem(IDC_BTN_STOP_MIRROR_LOAD)->EnableWindow(TRUE);
				}

				m_nStep++;
				m_Progress.SetPos((m_nStep * 10000) / m_nMirrorPatients);
				SetTimer(100, 20, NULL);
			}
		}
		else
		{
			m_bMirrorFinishedRequerying = true;
			m_Progress.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_STOP_MIRROR_LOAD)->EnableWindow(FALSE);
			// (c.haag 2008-02-06 10:09) - PLID 28622 - Make sure the Link Common Patients button is
			// disabled. I am somewhat certain this is dead code, but that's contingent on changes we make
			// to code blocks which involve setting the timer. Probably best just to note that and leave
			// the change in place.
			GetDlgItem(IDC_BTN_LINK_COMMON_PATIENTS)->EnableWindow(FALSE);
			m_mirrorList->Clear();
			UpdateCount();
		}
	}
	CNxDialog::OnTimer(nIDEvent);
}

void CMirrorLink::OnBtnStopMirrorLoad() 
{
	m_bForcedRequeryStop = true;
	m_bCancel61Requery = true;
}

//DRT 6/2/2008 - PLID 30230 - Added OnOK handler to keep behavior the same as pre-NxDialog changes
void CMirrorLink::OnOK()
{
	//Eat the message
}
