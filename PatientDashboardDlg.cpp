// PatientDashboardDlg.cpp : implementation file
//

// (j.gruber 2012-03-20 13:27) - PLID 48702 - created for - basic tab implementation


#include "stdafx.h"
#include "EMRRc.h"
#include "PatientDashboardDlg.h"
#include "DocumentOpener.h"
#include "SelectDlg.h"
#include "EMRSummaryDlg.h"
#include "PatientSummaryDlg.h"
#include "PatientWellnessDlg.h"
#include "EmrTemplateEditorDlg.h"
#include "EmrCollectionSetupDlg.h"
#include "EMRUtils.h"
#include "ConfigureNexEMRGroupsDlg.h"
#include "ConfigurePatDashboardDlg.h"
#include <mshtmcid.h>
#include "HistoryUtils.h"

_COM_SMARTPTR_TYPEDEF(IHTMLInputElement, __uuidof(IHTMLInputElement));


#define IDT_LOAD_HISTORY_IMAGES 12837 

// CPatientDashboardDlg dialog

IMPLEMENT_DYNAMIC(CPatientDashboardDlg, CBrowserEnabledDlg)



#pragma region DispatchMap
BEGIN_DISPATCH_MAP(CPatientDashboardDlg, CCmdTarget)
	//{{AFX_DISPATCH_MAP(CPatientDashboardDlg)
	// (j.gruber 2012-03-20 13:35) - PLID 49046 - create new EMN
	DISP_FUNCTION(CPatientDashboardDlg, "CreateNewEMN", CreateNewEMN, VT_NULL, VTS_I4)
	// (j.gruber 2012-03-20 13:35) - PLID 49047 - open summary
	DISP_FUNCTION(CPatientDashboardDlg, "OpenEmnSummary", OpenEMNSummary, VT_NULL, VTS_I4)
	// (j.gruber 2012-03-20 13:35) - PLID 49053 - open existing EMN
	DISP_FUNCTION(CPatientDashboardDlg, "OpenExistingEMN", OpenExistingEMN, VT_NULL, VTS_I4)
	// (j.gruber 2012-03-20 13:35) - PLID 49049 - Configure EMNs
	DISP_FUNCTION(CPatientDashboardDlg, "ConfigureEMNs", ConfigureEMNs, VT_NULL, VTS_NONE)
	// (j.gruber 2012-03-20 13:35) - PLID 49051 - Configure Dashboard
	DISP_FUNCTION(CPatientDashboardDlg, "ConfigureDashboard", ConfigureDashboard, VT_NULL, VTS_NONE)
	// (j.gruber 2012-03-20 13:35) - PLID 49052 - Open Part 1
	DISP_FUNCTION(CPatientDashboardDlg, "OpenLockManager", OpenLockManager, VT_NULL, VTS_NONE)
	// (j.gruber 2012-03-20 13:38) - PLID 49054 - editing templates
	DISP_FUNCTION(CPatientDashboardDlg, "EditTemplates", EditTemplates, VT_NULL, VTS_NONE)
	// (j.gruber 2012-03-20 13:35) - PLID 49052 - Open Part 1
	DISP_FUNCTION(CPatientDashboardDlg, "OpenProblemList", OpenProblemList, VT_NULL, VTS_NONE)
	// (j.gruber 2012-03-20 13:35) - PLID 49052 - Open Part 1
	DISP_FUNCTION(CPatientDashboardDlg, "OpenEMRSummary", OpenEMRSummary, VT_NULL, VTS_NONE)
	// (j.gruber 2012-03-20 13:35) - PLID 49052 - Open Part 1
	DISP_FUNCTION(CPatientDashboardDlg, "OpenPtSummary", OpenPtSummary, VT_NULL, VTS_NONE)
	// (j.gruber 2012-03-20 13:35) - PLID 49052 - Open Part 1
	DISP_FUNCTION(CPatientDashboardDlg, "OpenPtSeen", OpenPtSeen, VT_NULL, VTS_NONE)
	// (j.gruber 2012-03-20 13:35) - PLID 49053 - Open Part 2
	DISP_FUNCTION(CPatientDashboardDlg, "OpenPtsToBill", OpenPtsToBill, VT_NULL, VTS_NONE)
	// (j.gruber 2012-03-20 13:35) - PLID 49053 - Open Part 2
	DISP_FUNCTION(CPatientDashboardDlg, "OpenAnalysis", OpenAnalysis, VT_NULL, VTS_NONE)
	// (j.gruber 2012-03-20 13:35) - PLID 49053 - Open Part 2
	DISP_FUNCTION(CPatientDashboardDlg, "OpenWellness", OpenWellness, VT_NULL, VTS_NONE)
	// (j.gruber 2012-03-20 13:35) - PLID 49053 - Open Part 2
	DISP_FUNCTION(CPatientDashboardDlg, "OpenLab", OpenLab, VT_NULL, VTS_I4)		
	// (j.gruber 2012-06-26 17:00) - PLID 51214 - open history images
	DISP_FUNCTION(CPatientDashboardDlg, "OpenImage", OpenAttachment, VT_NULL, VTS_I4)
	// (c.haag 2015-04-30) - NX-100444 - Allow attachments to open
	DISP_FUNCTION(CPatientDashboardDlg, "OpenAttachment", OpenAttachment, VT_NULL, VTS_I4)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()
#pragma endregion


#pragma region DispatchMapImplementations
// (j.gruber 2012-03-20 13:39) - PLID 49046
void CPatientDashboardDlg::CreateNewEMN(long nID) {

	try {

		BOOL bEMRLaunched = CreateEMNFromTemplate(nID, this);
		if(!bEMRLaunched) {
			//HidePreviewWindow(); // (a.walling 2007-06-04 15:13) - PLID 25648
			GlobalLaunchPICEditorWithNewEMR(-1, nID);
		}
	}
	NxCatchAll(__FUNCTION__);	

}

// (j.gruber 2012-03-20 13:39) - PLID 49053
void CPatientDashboardDlg::OpenExistingEMN(long nEMNID) {

	try {
		//this will find our PICID for us
		GetMainFrame()->EditEmrRecord(-1, nEMNID);	
	}NxCatchAll(__FUNCTION__);
	
}
// (j.gruber 2012-03-20 13:39) - PLID 49047
void CPatientDashboardDlg::OpenEMNSummary(long nID) {

	try {
		CString strID;
		strID.Format("%li", nID);		
		RefreshPartialDashboard(pdtEMNSummary, strID);
		
	}NxCatchAll(__FUNCTION__);

}

// (j.gruber 2012-03-20 13:39) - PLID 49053
void CPatientDashboardDlg::OpenLab(long nLabID)
{
	try {
		if(!CheckCurrentUserPermissions(bioPatientLabs, sptWrite))
		{
			return;
		}
		GetMainFrame()->OpenLab(NULL, GetActivePatientID(), -1, ltInvalid, nLabID, -1, "", -1, FALSE, FALSE, GetSafeHwnd());

	}NxCatchAll(__FUNCTION__);

}

// (j.gruber 2012-06-26 17:01) - PLID 51214
// (c.haag 2015-04-30) - NX-100444 - Renamed to OpenAttachment
void CPatientDashboardDlg::OpenAttachment(long nMailID)
{
	try {		
		// Check permissions like the history tab does
		if (!CheckCurrentUserPermissions(bioPatientHistory, sptWrite)){
			return;
		}

		// Get fields from MailSent
		ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID, Selection, PathName FROM Mailsent WHERE MailID = {INT}", nMailID);
		if (rs->eof)
		{
			AfxMessageBox("The history attachment could not be found. It may have been deleted by another user.", MB_OK | MB_ICONSTOP);
		}
		else
		{
			long nPersonID = AdoFldLong(rs->Fields, "PersonID");
			CString strSelection = AdoFldString(rs->Fields, "Selection");
			CString strPath = AdoFldString(rs->Fields, "Pathname");
			if (CDocumentOpener::OpenHistoryAttachment(strPath, nPersonID, nMailID, this, m_bCaseHistoryIsOpen, strSelection).bNeedUpdateView)
			{
				UpdateView();
			}
		}

	}NxCatchAll(__FUNCTION__);

}



// (j.gruber 2012-03-20 13:39) - PLID 49049
void CPatientDashboardDlg::ConfigureEMNs()
{
	try {	
		
		if(CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
			CConfigureNexEMRGroupsDlg dlg(this);
			if(dlg.DoModal() == IDOK) {
				RefreshTemplateList();
			}
		}

	}NxCatchAll(__FUNCTION__);

}

// (j.gruber 2012-03-20 13:39) - PLID 49051
void CPatientDashboardDlg::ConfigureDashboard()
{
	try {
		CConfigurePatDashboardDlg dlg;
		if (IDOK == dlg.DoModal() ){

			CRect rcBrowser;
			GetDlgItem(IDC_PAT_DASH_HTML)->GetWindowRect(rcBrowser);
			ScreenToClient(rcBrowser);

			//refresh the whole dashboard
			// (j.gruber 2012-03-07 11:27) - PLID 48690 - ensure the login
			// (z.manning 2012-08-31 10:47) - PLID 52413 - Now uses the main API login
			// (c.haag 2015-03-30) - NX-100445 - GeneratePatientDashboard can be slow so use a wait cursor
			CWaitCursor wc;
			CString strHTML = (LPCTSTR)GetPatientDashboardGenerator()->GeneratePatientDashboard(_bstr_t(GetSubRegistryKey()), GetAPILoginToken(), GetActivePatientUserDefinedID(), rcBrowser.Width(), rcBrowser.Height(), m_nMainLeftWidth, _bstr_t(m_strSplitPos), m_bCloseRight);
			SetBrowserHTML(strHTML);

			SetTimer(IDT_LOAD_HISTORY_IMAGES, 100, NULL);
		}		

	}NxCatchAll(__FUNCTION__);

}

// (j.gruber 2012-03-20 13:39) - PLID 49052
void CPatientDashboardDlg::OpenLockManager()
{
	try
	{
		if(GetMainFrame() != NULL) {
			GetMainFrame()->ShowLockManager();
		}

	}NxCatchAll(__FUNCTION__);

}

// (j.gruber 2012-03-20 13:39) - PLID 49054
void CPatientDashboardDlg::EditTemplates()
{
	try {		
		//if (m_bExpired) return;

		// Pop up a menu to let the user choose which collection to base this emr template on		
		CPoint pt;
		GetCursorPos(&pt);

		long nEMRCollectionID = SelectEMRCollection(pt.x, pt.y, this, FALSE);
		if (nEMRCollectionID != -2) {
			if(CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
				StartEMRTemplateWithCollection(nEMRCollectionID,this);
			}
		}

	} NxCatchAll(__FUNCTION__);

}

// (j.gruber 2012-03-20 13:39) - PLID 49052
void CPatientDashboardDlg::OpenProblemList()
{	
	try {	
		
		CMainFrame *pFrame = GetMainFrame();
		if(pFrame) {
			//the mainframe's ShowEMRProblemList function will handle permission checking
			GetMainFrame()->PostMessage(NXM_OPEN_EMR_PROBLEM_LIST, GetActivePatientID());
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-03-20 13:39) - PLID 49052
void CPatientDashboardDlg::OpenEMRSummary()
{
	try {
		if(!CheckCurrentUserPermissions(bioPatientEMR, sptRead)) {
			return;
		}

		CEMRSummaryDlg dlg(this);
		dlg.m_nPatientID = GetActivePatientID();
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-03-20 13:39) - PLID 49052
void CPatientDashboardDlg::OpenPtSummary()
{
	try {

		CPatientSummaryDlg dlg(this);
		dlg.m_nPatientID = GetActivePatientID();
		dlg.m_strPatientName = GetActivePatientName();
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-03-20 13:39) - PLID 49052
void CPatientDashboardDlg::OpenPtSeen()
{
	try {
		GetMainFrame()->ShowSeenToday();	
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-03-20 13:39) - PLID 49053
void CPatientDashboardDlg::OpenPtsToBill()
{
	try {
		//	if (m_bExpired) return;
		GetMainFrame()->ShowPatientEMNsToBeBilled();
	}NxCatchAll(__FUNCTION__);

}

// (j.gruber 2012-03-20 13:39) - PLID 49053
void CPatientDashboardDlg::OpenAnalysis()
{
	try {

		if(GetMainFrame()) {
			GetMainFrame()->ShowEMRAnalysisDlg();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-03-20 13:39) - PLID 49053
void CPatientDashboardDlg::OpenWellness() 
{
	try {

		CPatientWellnessDlg dlg(GetActivePatientID(), this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}



#pragma endregion

#pragma region StolenFromPatientNexEMRDlg_for_the_most_part
using namespace ADODB;

// (j.gruber 2012-03-20 13:44) - PLID 49054
long CPatientDashboardDlg::SelectEMRCollection(long x, long y, CWnd *pParent, BOOL bIsEMR /*as opposed to an EMR template*/)
{
	CDWordArray arydwIDs;
	
	// Creat	e a popup menu with all the collections as menu items
	_RecordsetPtr prs = CreateRecordset("SELECT ID, Name, MenuOrder FROM EMRCollectionT WHERE Inactive = 0 ORDER BY MenuOrder");
	FieldsPtr pflds = prs->GetFields();
	FieldPtr fldID = pflds->GetItem("ID");
	FieldPtr fldName = pflds->GetItem("Name");
	FieldPtr fldMenuOrder = pflds->GetItem("MenuOrder");
	//(e.lally 2012-03-28) PLID 49275 - Put the list of collections in a submenu if there are a lot of them.
	CMenu mnuCollections;
	mnuCollections.CreatePopupMenu();
	long nLastMenuOrder;
	BOOL bLastMenuOrderSet = FALSE;
	BOOL bGrouping = FALSE;
	while (!prs->eof) {
		// Get the menu order number
		long nMenuOrder = AdoFldLong(fldMenuOrder, -1);
		// If we're now hitting the 2nd item in a group, insert the separator above the first item 
		// in the group; on the other hand, if we WERE grouping and we just hit a new menu order 
		// which would mean we're no longer grouping, then add a separator at the end (before we 
		// add our new menu item below).
		if (bLastMenuOrderSet) {
			if (nMenuOrder == nLastMenuOrder) {
				if (!bGrouping) {
					long nCount = mnuCollections.GetMenuItemCount();
					if (nCount > 1 && !(mnuCollections.GetMenuState(nCount-2, MF_BYPOSITION) & MF_SEPARATOR)) {
						mnuCollections.InsertMenu(nCount-1, MF_SEPARATOR|MF_BYPOSITION);
					}
					bGrouping = TRUE;
				}
			} else if (bGrouping) {
				// Add a separator
				mnuCollections.AppendMenu(MF_SEPARATOR|MF_BYPOSITION);
				bGrouping = FALSE;
			}
		}
		// Add it to the array
		int nIndex = arydwIDs.Add(AdoFldLong(fldID));
		// The command id will be the index plus 1, which leaves 0 as a valid sentinal return 
		// value for TrackPopupMenu() below.
		int nCmdId = nIndex + 1;
		// Add the menu item, passing the command id we just calculated
		mnuCollections.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nCmdId, AsMenuItem(AdoFldString(fldName)));
		// Remember this order number for next time
		nLastMenuOrder = nMenuOrder;
		if (nLastMenuOrder >= 0) {
			bLastMenuOrderSet = TRUE;
		} else {
			ASSERT(bLastMenuOrderSet == FALSE);
		}
		// Move to the next collection name
		prs->MoveNext();
	}

	CMenu mnu;
	bool bUseSubmenu = false;
	//(e.lally 2012-03-28) PLID 49275 - Put the list of collections in a submenu if there are a lot of them.
	if(arydwIDs.GetSize() > 30){
		bUseSubmenu = true;
		mnu.CreatePopupMenu();
		mnu.InsertMenu(0, MF_BYPOSITION|MF_POPUP, (UINT)mnuCollections.m_hMenu, "&Filter On Collection...");
	}
	else {
		mnu.m_hMenu = mnuCollections.GetSafeHmenu(); 
	}


	// Add the built-in menu items
	{
		// If any came before, add a separator
		BOOL bNeedSeparator = FALSE;
		if (arydwIDs.GetSize() > 0) {
			bNeedSeparator = TRUE;
		}
		if (bIsEMR) {
			if(bNeedSeparator) {
				mnu.AppendMenu(MF_ENABLED|MF_SEPARATOR|MF_BYPOSITION);
				bNeedSeparator = FALSE;
			}
			// One guaranteed one so there's always the option of having a collectionless emr
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, -1, "<&Generic EMR>");
		}
		if ((GetCurrentUserPermissions(bioAdminEMR) & SPT__R_________ANDPASS)) {
			if(bNeedSeparator) {
				mnu.AppendMenu(MF_ENABLED|MF_SEPARATOR|MF_BYPOSITION);
			}
			// One that lets the user add collections
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, -2, "&New Collection...");
			// Add one that lets you manage the collection list
			mnu.AppendMenu(MF_ENABLED|MF_SEPARATOR|MF_BYPOSITION);
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, -3, "Manage &Collections...");
		}
		if ((GetCurrentUserPermissions(bioAdminEMR) & SPT__R_________ANDPASS) && !bIsEMR) {
			if(bNeedSeparator) {
				mnu.AppendMenu(MF_ENABLED|MF_SEPARATOR|MF_BYPOSITION);
			}
			// Let the user manage their templates
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, -4, "Manage &Templates...");
		}
	}

	// Pop up the menu to the right of the button
	int nCmdId = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, x, y, pParent, NULL);
	//This will destroy the menu and any submenus
	mnu.DestroyMenu();
	if (nCmdId == 0) {
		// The user canceled the menu
		return -2;
	} else if (nCmdId == -1) {
		// The user chose the built-in "generic emr" collection
		return -1;
	} else if (nCmdId == -2) {
		// The user wants to create a new collection
		if(CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
			long nNewEMRCollectionID = CreateNewEMRCollection();
			if (nNewEMRCollectionID == -1) {
				// The user cancelled
				return -2;
			} else {
				return nNewEMRCollectionID;
			}
		}
		else {
			return -2;
		}
	} else if (nCmdId == -3) {
		// The user wants to manage the whole set of collections rather than select one
		if(CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
			CEmrCollectionSetupDlg dlg(this);
			// (z.manning, 02/20/2007) - If they saved changes, we need to refresh the new list in case
			// they re/inactivated any collections.
			if(dlg.DoModal() == IDOK) {
				RefreshTemplateList();				
			}
		}
		// Since the user didn't want to select one, return as if she canceled the menu
		return -2;
	} else if (nCmdId == -4) {		
		if (m_pdlgTemplateManager) {
			// (c.haag 2006-04-04 11:01) - PLID 19890 - Check permissions
			if(CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
				m_pdlgTemplateManager->DoModal();
			}
		} else {
			ASSERT(FALSE);
		}
		return -2;
	} else {
		// Get the ID out of the array based on the index chosen by the user (remember to 
		// subtract 1 because we have a command id and we want to convert it back to an 
		// index, see above loop).
		return arydwIDs.GetAt(nCmdId - 1);
	}
}
#pragma endregion

// (j.gruber 2012-03-20 13:45) - PLID 48702
#pragma region CPatientDashboardDlg_Implementation
CPatientDashboardDlg::CPatientDashboardDlg(CNxView* pParent /*=NULL*/)
	: CBrowserEnabledDlg(CPatientDashboardDlg::IDD, pParent)		
{
	EnableAutomation();

	g_propManager.BulkCache("PatientDashboard", propbitNumber | propbitText,
		"(Username IN('<None>', '%s') AND Name IN("
		"'PatientDashboardMainLeftWidth', "	// (j.armen 2012-07-03 17:09) - PLID 51206
		"'PatientDashboardCloseRight', "	// (j.armen 2012-07-03 17:09) - PLID 51206
		"'PatientDashboardSplitPos'"		// (j.armen 2012-07-03 17:09) - PLID 51206
		"))", _Q(GetCurrentUserName()));
	
	m_pdlgTemplateManager = NULL;	
	m_bCaseHistoryIsOpen = FALSE; // (c.haag 2015-04-30) - NX-100444

	// (j.gruber 2012-05-16 10:22) - PLID 50401 - set the widths to -1 initially so it'll use the default
	// (j.armen 2012-07-03 17:09) - PLID 51206 - Set the properties based on ConfigRT
	m_nMainLeftWidth = GetRemotePropertyInt("PatientDashboardMainLeftWidth", -1, 0, GetCurrentUserName());
	m_bCloseRight = GetRemotePropertyInt("PatientDashboardCloseRight", FALSE, 0, GetCurrentUserName()) ? TRUE : FALSE;
	m_strSplitPos = GetRemotePropertyText("PatientDashboardSplitPos", "", 0, GetCurrentUserName());

}

// (j.gruber 2012-03-20 13:45) - PLID 48702
CPatientDashboardDlg::~CPatientDashboardDlg()
{
	if (m_pdlgTemplateManager) {
		delete m_pdlgTemplateManager;
		m_pdlgTemplateManager = NULL;
	}
}

// (j.gruber 2012-03-20 13:45) - PLID 48702
void CPatientDashboardDlg::DoDataExchange(CDataExchange* pDX)
{
	CBrowserEnabledDlg::DoDataExchange(pDX);
}

// (j.gruber 2012-03-20 13:45) - PLID 48702
BEGIN_EVENTSINK_MAP(CPatientDashboardDlg, CWnd)
    //{{AFX_EVENTSINK_MAP(CPatientDashboardDlg)
	ON_EVENT(CPatientDashboardDlg, IDC_PAT_DASH_HTML, 250 /* BeforeNavigate2 */, OnBeforeNavigate2Browser, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)	
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

// (j.gruber 2012-03-20 13:45) - PLID 48702
BEGIN_MESSAGE_MAP(CPatientDashboardDlg, CBrowserEnabledDlg)
	// (j.gruber 2012-06-11 16:11) - PLID 50225
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)	
	ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx)
	ON_MESSAGE(NXM_EDIT_EMR_OR_TEMPLATE, OnMsgEditEMRTemplate) // (j.gruber 2012-06-26 10:06) - PLID 49054
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

extern CString g_strLoginToken;

// (j.gruber 2012-03-20 13:45) - PLID 48702
BOOL CPatientDashboardDlg::OnInitDialog() 
{
	try {

		m_pdlgTemplateManager = new CEmrTemplateManagerDlg(this);

		m_pBrowser = GetDlgItem(IDC_PAT_DASH_HTML)->GetControlUnknown();

		m_piClient = new IPatientDashboardBrowserInterface();

		m_piClient->SetBrowser(m_pBrowser);
		m_piClient->SetBrowserDlg(this);


		IUnknown* pUnkBrowser = GetDlgItem(IDC_PAT_DASH_HTML)->GetControlUnknown(); // not reference counted, do not release

		if (pUnkBrowser) {
			IOleObjectPtr pBrowserOleObject = NULL;

			// retrieve our OleObject interface so we can set our custom client site
			pUnkBrowser->QueryInterface(IID_IOleObject, (void**)&pBrowserOleObject);

			if (pBrowserOleObject != NULL)
			{
				IOleClientSite *oldClientSite = NULL;

				if (pBrowserOleObject->GetClientSite(&oldClientSite) == S_OK)
				{
					m_piClient->SetDefaultClientSite(oldClientSite);
					oldClientSite->Release();
				}

				pBrowserOleObject->SetClientSite(m_piClient);
				
			}

			COleVariant varUrl("about:blank");

			if (m_pBrowser) {
				m_pBrowser->Navigate2(varUrl, NULL, NULL, NULL, NULL);
			}
			
			CBrowserEnabledDlg::OnInitDialog();			
		}		
	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CPatientDashboardDlg::OnDestroy() 
{
	try {
		// (j.armen 2012-07-03 17:10) - PLID 51206 - StoreDetails in our member vars, and then set the remote props
		StoreDetails();
		SetRemotePropertyInt("PatientDashboardMainLeftWidth", m_nMainLeftWidth, 0, GetCurrentUserName());
		SetRemotePropertyInt("PatientDashboardCloseRight", m_bCloseRight, 0, GetCurrentUserName());
		SetRemotePropertyText("PatientDashboardSplitPos", m_strSplitPos, 0, GetCurrentUserName());
	} NxCatchAll(__FUNCTION__);

	CBrowserEnabledDlg::OnDestroy();
}

void CPatientDashboardDlg::NavigateToStream(IStreamPtr pStream)
{
	try {
		if (pStream == NULL) {		
			_com_issue_error(DISP_E_BADVARTYPE);
			return;
		}

		m_pPendingStream = pStream;

		LoadPendingStream();
	}NxCatchAll(__FUNCTION__);
}


void CPatientDashboardDlg::RefreshPartialDashboard(PatientDashboardType pdtTypeToRefresh, CString strIDToRefresh)
{
	try {			
		// (j.gruber 2012-06-19 10:17) - PLID 48690 - we need to use the global one
		if (GetPatientDashboardGenerator() != NULL)
		{			
			//declare our return list
			NexTech_COM::IPatientDashboardControlListPtr pUpdateList;
			pUpdateList.CreateInstance("NexTech_COM.PatientDashboardControlList");
			if (pUpdateList) {
				
				// (j.gruber 2012-06-19 10:17) - PLID 48690 - we need to use the global one
				// (z.manning 2012-08-31 11:17) - PLID 52413 - Use the API's login token
				pUpdateList = GetPatientDashboardGenerator()->GeneratePartialDashboard2(_bstr_t(GetSubRegistryKey()), GetAPILoginToken(), GetActivePatientUserDefinedID(), pdtTypeToRefresh, _bstr_t(strIDToRefresh));

				//make sure we have the webpage
				HRESULT hr;			
				IDispatchPtr p;

				hr = m_pBrowser->get_Document(&p);
				if (SUCCEEDED(hr)) {
					//find the element we are replacing
					IHTMLDocument2Ptr pDoc;
					hr = p->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);

					if (SUCCEEDED(hr)) {

						//now we have our list of things we need to refresh, so let's loop through them and refresh
						for (int i=0; i < pUpdateList->GetCount(); i++) {
							CString strID = (LPCTSTR)pUpdateList->GetItemID(i);
							CString strHTML = (LPCTSTR)pUpdateList->GetItemHTMLContents(i);
							CString strDivToScrollID = (LPCTSTR)pUpdateList->GetItemDivToScrollID(i);
							CString strScrollValueID = (LPCTSTR)pUpdateList->GetItemScrollID(i);							
					
							IHTMLElementPtr pElement = GetElementByID(strID);							

							if (pElement) {								
								_bstr_t bstrtNewHTML((LPCTSTR)strHTML);
								hr = pElement->put_outerHTML(bstrtNewHTML);
							}

							// (j.gruber 2012-05-15 12:23) - PLID 50397 - refilter
							// (j.gruber 2012-06-06 13:23) - PLID 49047 - set scroll
							// (j.gruber 2012-07-02 13:19) - PLID 48702 - we can't check the type that we sent in, we only want this to hit if we are currently updating an emn summary box
							if (strID.Find("EMNSummary") != -1) {
								CString strTempID = strID;
								strTempID.Replace("outerDiv", "");
								ExecScript("SetEMRScroll(" + strTempID + "," + strScrollValueID + ")");
								ExecScript("FilterEMRSummary(" + strTempID + ", " + strTempID + "Search" + ",'tdContentSummaryDescription') ");								
							}

							//now let's handle our scroll
							if (strID.Find("EMNSummary") == -1) {
								if (strDivToScrollID != "") {
									SetScrollPosition(strDivToScrollID, strScrollValueID);
								}
							}
						}
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CPatientDashboardDlg::SetScrollPosition(CString strDivToSetScroll, CString strScrollValue) 
{
	try {
		IHTMLElementPtr pElement = GetElementByID(strScrollValue);

		if (pElement) {
			//we need to get the value of the scroll first
			BSTR bstr;
			//CString temp = "value";
			//BSTR att = _bstr_t(temp);
			//_variant_t var;
			//pElement->getAttribute(att, 2, &var);
			IHTMLInputElementPtr(pElement)->get_value(&bstr);
			
			IHTMLElementPtr pDiv = GetElementByID(strDivToSetScroll);
			if (pDiv) {
				IHTMLElement2Ptr pDiv2(pDiv);

				if (pDiv2) {

					//now we have to set the scroll Pos of the div
					CString strValue = bstr;
					long nValue = atoi(strValue);	
					if (strDivToSetScroll.Find("vscroll") == -1) {
						pDiv2->put_scrollLeft(nValue);
					}
					else {
						pDiv2->put_scrollTop(nValue);
					}

				}
			}
		}


	}NxCatchAll(__FUNCTION__);
}

void CPatientDashboardDlg::LoadPendingStream()
{	
	try {
		if (m_pPendingStream) {
			// load the stream into the browser
			IDispatchPtr pDoc;
			HR(m_pBrowser->get_Document(&pDoc));
			if (pDoc) {
				IPersistStreamInitPtr pPersistStreamPtr;

				HR(pDoc->QueryInterface(IID_IPersistStreamInit, (void**)&pPersistStreamPtr));
				if (pPersistStreamPtr) {
					HR(pPersistStreamPtr->InitNew());
					HR(pPersistStreamPtr->Load(m_pPendingStream));

					m_pPendingStream = NULL;
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CPatientDashboardDlg::SetBrowserHTML(CString strHTML)
{
	try {
		IStreamPtr pStrmI;
				 
		HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &pStrmI);
		ASSERT(SUCCEEDED(hr));
		
		 
		ULONG dwWritten;			
		pStrmI->Write(strHTML, strHTML.GetLength(), &dwWritten);

		// move to the beginning
		LARGE_INTEGER nnMove;
		nnMove.QuadPart = 0;
		pStrmI->Seek(nnMove, STREAM_SEEK_SET, NULL);
		
		NavigateToStream(pStrmI);
	}NxCatchAll(__FUNCTION__);
}

void CPatientDashboardDlg::OnTimer(UINT nIDEvent) 
{

	try {
		switch (nIDEvent) {
			case IDT_LOAD_HISTORY_IMAGES:
				KillTimer(IDT_LOAD_HISTORY_IMAGES);
				// (j.gruber 2012-06-19 10:17) - PLID 48690 - we need to use the global one				
				if (GetPatientDashboardGenerator()) {								
					RefreshPartialDashboard(pdtHistoryImages, "");										
				}
			break;
			
		}
		CNxDialog::OnTimer(nIDEvent);

	}NxCatchAll(__FUNCTION__);
	
}

void CPatientDashboardDlg::OnBeforeNavigate2Browser(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel) 
{
	try {
		CString strUrl = VarString(URL, "");

		if (strUrl == "about:blank") {
			//let it through
		}
		else {

			//cancel  everything coming from the preview pane
			if (strUrl.Left(9) == "nexemr://") {
				*Cancel = TRUE;
			}
		}
	}NxCatchAll(__FUNCTION__);

}

void CPatientDashboardDlg::UpdateView(bool bForceRefresh)
{
	try
	{			
		StoreDetails();

		CRect rcBrowser;
		GetDlgItem(IDC_PAT_DASH_HTML)->GetWindowRect(rcBrowser);
		ScreenToClient(rcBrowser);
		
		// (j.gruber 2012-06-19 10:17) - PLID 48690 - we need to use the global one	
		if (GetPatientDashboardGenerator()) {
		
			// (z.manning 2012-08-31 11:18) - PLID 52413 - Use the API's login token
			// (c.haag 2015-03-30) - NX-100445 - GeneratePatientDashboard can be slow so use a wait cursor
			CWaitCursor wc;
			CString strHTML = (LPCTSTR)GetPatientDashboardGenerator()->GeneratePatientDashboard(_bstr_t(GetSubRegistryKey()), GetAPILoginToken(), GetActivePatientUserDefinedID(), rcBrowser.Width(), rcBrowser.Height(), m_nMainLeftWidth, _bstr_t(m_strSplitPos), m_bCloseRight);
			SetBrowserHTML(strHTML);			

			//set the timer to load the history images
			SetTimer(IDT_LOAD_HISTORY_IMAGES, 100, NULL);
		}
	}NxCatchAll(__FUNCTION__);

}

void CPatientDashboardDlg::StoreDetails()
{
	try {
		// (j.gruber 2012-05-16 10:19) - PLID 50401 - we need to get the current width of the left and right
		IHTMLElementPtr pElementLeft = GetElementByID("MainLeft");
		if (pElementLeft) {
			pElementLeft->get_offsetWidth(&m_nMainLeftWidth);
		}

		// (j.gruber 2012-05-16 10:19) - PLID 50401 - we need to get the current width of the left and right
		IHTMLElementPtr pElementSplit = GetElementByID("splitPos");
		if (pElementSplit) {
			BSTR bstr;
			IHTMLInputElementPtr(pElementSplit)->get_value(&bstr);		
			m_strSplitPos = bstr;				
		}	

		// (j.gruber 2012-05-16 10:19) - PLID 50401 - we need to get the current width of the left and right
		IHTMLElementPtr pElementClosed = GetElementByID("isClosed");
		if (pElementClosed) {
			BSTR bstr;
			IHTMLInputElementPtr(pElementClosed)->get_value(&bstr);		
			CString str = bstr;
			if (str == "True") {
				m_bCloseRight = true;
			}
			else {
				m_bCloseRight = false;
			}		
		}
	}NxCatchAll(__FUNCTION__);

}

// (j.gruber 2012-05-25 15:25) - PLID 49049
void CPatientDashboardDlg::RefreshTemplateList() {

	try {
		RefreshPartialDashboard(pdtCreateEMNs, "");		
	}NxCatchAll(__FUNCTION__);
}

BOOL CPatientDashboardDlg::PreTranslateMessage(MSG* pMsg) 
{
	//I looked at a bunch of pretranslatemessage functions and none had error handling, so I left it out of this also.

	switch (pMsg->message) {
	case WM_KEYUP:
		if (SUCCEEDED(IOleInPlaceActiveObjectPtr(m_pBrowser)->TranslateAccelerator(pMsg))) {
			return TRUE;
		}
		break;
	default:
		break;
	}

		
	return CNxDialog::PreTranslateMessage(pMsg);
}

// (j.gruber 2012-05-08 09:55) - PLID 50225 - implement table checkers
LRESULT CPatientDashboardDlg::OnTableChangedEx(WPARAM wParam, LPARAM lParam) {

	try {
		switch(wParam) {

			case NetUtils::AppointmentsT: {
				//use the extra details to update the screen
				CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
				// (j.jones 2014-08-05 10:47) - PLID 63183 - we have the patient ID now
				long nPatientID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiPatientID), -1);
				if (nPatientID == GetActivePatientID()) {
					RefreshPartialDashboard(pdtAppointments, "");
				}
			}
			break;

			case NetUtils::MailSent:
			{
				// (j.jones 2014-08-04 14:49) - PLID 63183 - only refresh if the patient ID is our current patient
				CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
				long nPatientID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::MailSent_DetailIndex::msdiPatientID), -1);

				if (nPatientID == GetActivePatientID()) {
					RefreshPartialDashboard(pdtHistoryImages, "");
					RefreshPartialDashboard(pdtHistoryAttachments, ""); // (c.haag 2015-04-30) - NX-100444
				}
			}
			break;

			case NetUtils::LabsT:
			{
				// (r.gonet 08/25/2014) - PLID 63221 - Is this for our patient? If so, update the labs portion of the dashboard
				CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
				long nPatientID = VarLong(pDetails->GetDetailData((short)TableCheckerDetailIndex::Labs_DetailIndex::PatientID), -1);

				if (nPatientID == GetActivePatientID()) {
					RefreshPartialDashboard(pdtLabs, "");
				}
			}
			break;
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.gruber 2012-05-08 09:55) - PLID 50225 - implement table checkers
LRESULT CPatientDashboardDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		// We now pass table checker
		// messages to the template list
		// (j.jones 2014-08-08 16:59) - PLID 63182 - this function had nothing in it, so I removed its existence
		/*
		if (m_pdlgTemplateManager && IsWindow(m_pdlgTemplateManager->GetSafeHwnd())) {
			m_pdlgTemplateManager->OnTableChanged(wParam, lParam);
		}
		*/

		long nPatientID = GetActivePatientID();

		switch(wParam) {
			case NetUtils::EMRMasterT: {
					try {
						// (j.jones 2014-08-08 11:22) - PLID 63183 - only refresh if the tablechecker is for this patient
						if(lParam == nPatientID) {
							//just refresh our one control
							RefreshPartialDashboard(pdtEMNSummary, "");										
						}
					} NxCatchAll("Error in CPatientDashboardDlg::OnTableChanged:EMRMasterT");
			}
			break;

			case NetUtils::AppointmentsT:
				// (j.jones 2014-08-05 11:04) - PLID 63183 - There should not be a non-EX AppointmentsT
				// tablechecker. If we get one, do nothing.			
			break;

			case NetUtils::CurrentPatientMedsT:
				//is this our patient
				if (lParam == nPatientID) {
					RefreshPartialDashboard(pdtCurrentMeds, "");
				}
			break;

			case NetUtils::EMRProblemsT:
				if (lParam == nPatientID) {
					RefreshPartialDashboard(pdtProblems, "");
				}
			break;

			case NetUtils::EMRTemplateT:
				// (j.jones 2014-08-08 11:23) - PLID 63183 - don't refresh on template changes
				//RefreshPartialDashboard(pdtCreateEMNs, "");
			break;

			case NetUtils::LabsT:
				// (r.gonet 09/02/2014) - PLID 63221 - LabsT is now an Ex table checker
			break;

			case NetUtils::MailSent:
				// (j.jones 2014-08-04 14:49) - PLID 63183 - MailSent should only be sent in EX form,
				// if a regular version is sent we will not respond to it
			break;

			case NetUtils::PatientAllergyT:
				if (lParam == nPatientID) {
					RefreshPartialDashboard(pdtAllergies, "");
				}
			break;				   
		}
		
	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.gruber 2012-06-26 10:06) - PLID 59054
LRESULT CPatientDashboardDlg::OnMsgEditEMRTemplate(WPARAM wParam, LPARAM lParam)
{
	try {		
		
		GetMainFrame()->PostMessage(NXM_EDIT_EMR_OR_TEMPLATE, wParam, lParam);

	} NxCatchAll(__FUNCTION__);

	return 0;
}

#pragma endregion


#pragma region IPatientDashboardBrowserInterface
IPatientDashboardBrowserInterface::IPatientDashboardBrowserInterface()
{

}


HRESULT STDMETHODCALLTYPE IPatientDashboardBrowserInterface::GetExternal(IDispatch **ppDispatch)
{
	try {
		*ppDispatch = m_pBrowserDlg->GetIDispatch(TRUE);
	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.gruber 2012-05-08 09:33) - PLID 50223 - custom context menu
HRESULT IPatientDashboardBrowserInterface::CustomContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
	try {
		//return S_OK; // we will prevent the context menu by default

		IOleWindow	*oleWnd			= NULL;
		HWND		hwnd			= NULL;
		HMENU		hMainMenu		= NULL;
		HMENU		hPopupMenu		= NULL;
		HRESULT		hr				= 0;

		if ((ppt == NULL) || (pcmdtReserved == NULL))
			return S_OK;

		hr = pcmdtReserved->QueryInterface(IID_IOleWindow, (void**)&oleWnd);
		if ( (hr != S_OK) || (oleWnd == NULL))
			return S_OK;

		hr = oleWnd->GetWindow(&hwnd);
		if ( (hr != S_OK) || (hwnd == NULL))
			return S_OK;
		
		CMenu mnu;
		long n = 0;
		mnu.CreatePopupMenu();

		enum EGenericContextMenuItems {
			miPrint = 1638, // just some random number
			miPrintPreview,
			miCopy,
			miPaste,
			miSaveAs,
			miFind,
			miViewSource,
			miViewGeneratedSource,
		};

		//mnu.InsertMenu(n++, MF_BYPOSITION, miPrint, "&Print...");
		//mnu.InsertMenu(n++, MF_BYPOSITION, miPrintPreview, "Print Pre&view...");
		mnu.InsertMenu(n++, MF_BYPOSITION, miFind, "&Find...");
		mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
		mnu.InsertMenu(n++, MF_BYPOSITION, miCopy, "&Copy...");
		mnu.InsertMenu(n++, MF_BYPOSITION, miPaste, "&Paste...");		
		//mnu.InsertMenu(n++, MF_BYPOSITION, miSaveAs, "&Save As...");
		//mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");	
		
	#ifdef _DEBUG
		mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");	
		mnu.InsertMenu(n++, MF_BYPOSITION, miViewSource, "View Source");
		mnu.InsertMenu(n++, MF_BYPOSITION, miViewGeneratedSource, "View Generated Source");
	#endif
		
		long nSelection = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY | TPM_VERPOSANIMATION,
			ppt->x, ppt->y, CWnd::FromHandle(hwnd), NULL);

		if (nSelection > 0) {
			IOleCommandTargetPtr pCmdTarg = m_pBrowserDlg->GetOleCommandTarget();
			switch (nSelection) {
				case miCopy:
							{ 
							// Copy
							pCmdTarg->Exec(&CGID_MSHTML, 
									IDM_COPY, 
									OLECMDEXECOPT_DONTPROMPTUSER, 
									NULL,
									NULL);
							} break;

				case miPaste:
							{ 
							// Copy
							pCmdTarg->Exec(&CGID_MSHTML, 
									IDM_PASTE, 
									OLECMDEXECOPT_DONTPROMPTUSER, 
									NULL,
									NULL);
							} break;
				case miPrint:
					{
						pCmdTarg->Exec(&CGID_MSHTML, 
								IDM_PRINT, 
								OLECMDEXECOPT_PROMPTUSER, 
								NULL, // use default print template
								NULL);
					} break;
				case miPrintPreview:
					{
						pCmdTarg->Exec(&CGID_MSHTML, 
								IDM_PRINTPREVIEW, 
								OLECMDEXECOPT_PROMPTUSER, 
								NULL, // use default print template
								NULL);
					} break;
				case miSaveAs:
					{
						CString strWindowTitle;
						m_pBrowserDlg->GetWindowText(strWindowTitle);
						CString strPath = MakeValidFolderName(strWindowTitle);
						strPath += ".htm";
						_variant_t varPath = (LPCTSTR)strPath;
						pCmdTarg->Exec(&CGID_MSHTML, 
								IDM_SAVEAS, 
								OLECMDEXECOPT_PROMPTUSER, 
								&varPath,
								NULL);
					} break;
				case miFind:
					{
						pCmdTarg->Exec(&CGID_MSHTML, 
								IDM_FIND, 
								OLECMDEXECOPT_DODEFAULT, 
								NULL,
								NULL);
					} break;

				case miViewSource:
					pCmdTarg->Exec(&CGID_MSHTML, 
									IDM_VIEWSOURCE, 
									OLECMDEXECOPT_DONTPROMPTUSER, 
									NULL,
									NULL);
				break;

				case miViewGeneratedSource:
					{
						CString strSource = m_pBrowserDlg->GetCurrentHTML();
						CStdioFile fOut;
						if (fOut.Open(GetPracPath(PracPath::SessionPath) ^ "NxPatDashboard.html", CFile::modeCreate | CFile::modeWrite | CFile::shareCompat)) {
							fOut.WriteString(strSource);

							fOut.Close();

							// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
							int nResult = (int)ShellExecute ((HWND)this, NULL, "notepad.exe", (CString("'") + GetPracPath(PracPath::SessionPath) ^ "NxPatDashboard.html" + CString("'")), NULL, SW_SHOW);
						}
				}

			}
		}

	}NxCatchAll(__FUNCTION__);
	
	return S_OK;

}

#pragma endregion
