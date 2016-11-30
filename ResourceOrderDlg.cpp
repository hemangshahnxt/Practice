// ResourceOrderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "ResourceOrderDlg.h"
#include "GlobalSchedUtils.h"
#include "GlobalUtils.h"
#include "PracProps.h"
#include "Administratorrc.h"
#include "MultiSelectDlg.h"
#include "ResEntryDlg.h"
#include "GlobalDataUtils.h"
#include "HL7ParseUtils.h"
#include "ListMergeDlg.h"
using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum {
	rofID = 0,
	rofVisible,
	rofName,
	rofRelevence,
	rofProviderNames,
	rofProviderIDs,
	rofLocationID,
	rofUserID,
	rofInactive,
	rofDefaultInterval,
};

// (c.haag 2010-05-04 10:29) - PLID 37263 - Pop-up commands for copying resources
#define ID_COPY_RES_TO_MULTIPLE 43000
#define ID_COPY_RES_TO_ALL 43001

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CResourceOrderDlg dialog


CResourceOrderDlg::CResourceOrderDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CResourceOrderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResourceOrderDlg)
	//}}AFX_DATA_INIT
	
	m_nLastChosenRow = sriNoRow;
	m_bNeedToSave = FALSE;

	m_nCurResourceViewID = CSchedulerView::srvUnknownResourceView;
	m_nCopyResourcesSrcViewIndex = -1; // (c.haag 2010-05-04 10:29) - PLID 37263

	m_nPendingLocationID = -2;

	m_nCurLocationID = -2;
}

CResourceOrderDlg::~CResourceOrderDlg()
{
	// (z.manning, 02/20/2007) - PLID 24167 - Clean up any memory allocated by the allowed
	// purposes dialog.
	for(int i = 0; i < m_apSavedResPurpTypeCombinations.GetSize(); i++) {
		if(m_apSavedResPurpTypeCombinations.GetAt(i) != NULL) {
			delete m_apSavedResPurpTypeCombinations.GetAt(i);
		}
	}
	m_apSavedResPurpTypeCombinations.RemoveAll();

	for(int j = 0; j < m_apChanged.GetSize(); j++) {
		if(m_apChanged.GetAt(j) != NULL) {
			delete m_apChanged.GetAt(j);
		}
	}
	m_apChanged.RemoveAll();
}


void CResourceOrderDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResourceOrderDlg)
	DDX_Control(pDX, IDC_MOVE_DOWN_BTN, m_downButton);
	DDX_Control(pDX, IDC_MOVE_UP_BTN, m_upButton);
	DDX_Control(pDX, IDC_NEW_RESOURCE_BTN, m_btnAddResource);
	DDX_Control(pDX, IDC_DELETE_RESOURCE_BTN, m_btnDeleteResource);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_RESOURCEAPPTTYPES, m_btnAllowedPurposes);
	DDX_Control(pDX, IDC_BTN_PROVIDERLINKING, m_btnProviderLinking);
	DDX_Control(pDX, IDC_BTN_RESORDER_MERGE_RESOURCES, m_btnMergeResources);
	//}}AFX_DATA_MAP
}

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define IDM_ADD    46101
#define IDM_EDIT 46102
#define IDM_DELETE 46103

BEGIN_MESSAGE_MAP(CResourceOrderDlg, CNxDialog)
	//{{AFX_MSG_MAP(CResourceOrderDlg)
	ON_BN_CLICKED(IDC_MOVE_UP_BTN, OnMoveUpBtn)
	ON_BN_CLICKED(IDC_MOVE_DOWN_BTN, OnMoveDownBtn)
	ON_BN_CLICKED(IDC_NEW_RESOURCE_BTN, OnNewResourceBtn)
	ON_BN_CLICKED(IDC_DELETE_RESOURCE_BTN, OnDeleteResourceBtn)
	ON_BN_CLICKED(IDC_EDITVIEW_BTN, OnEditviewBtn)
	ON_COMMAND(IDM_ADD, OnViewAdd)
	ON_COMMAND(IDM_EDIT, OnViewEdit)
	ON_COMMAND(IDM_DELETE, OnViewDelete)
	ON_BN_CLICKED(IDC_BTN_PROVIDERLINKING, OnBtnProviderLinking)
	ON_BN_CLICKED(IDC_BTN_COPYRESOURCESETTINGS, OnCopyResourceSettings)
	ON_BN_CLICKED(IDC_BTN_RESOURCEAPPTTYPES, OnBtnResourceappttypes)
	ON_WM_DESTROY()
	ON_COMMAND(ID_COPY_RES_TO_MULTIPLE, OnCopyResourceSettingsToMultipleUsers)
	ON_COMMAND(ID_COPY_RES_TO_ALL, OnCopyResourceSettingsToAllUsers)
	ON_BN_CLICKED(IDC_BTN_RESORDER_MERGE_RESOURCES, OnBtnMergeResources)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceOrderDlg message handlers

BOOL CResourceOrderDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// Abort if the resentry dialog is open
	CSchedulerView* pView = (CSchedulerView*)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
	if (pView)
	{
		if (pView->GetResEntry() &&
			pView->GetResEntry()->GetSafeHwnd() && 
			pView->GetResEntry()->IsWindowVisible())
		{
			MsgBox("You must close the active appointment before configuring the scheduler resources.");
			PostMessage(WM_COMMAND, IDCANCEL);
			return TRUE;
		}
	}
	
	// Set the button icons appropriately
	extern CPracticeApp theApp;

	m_upButton.AutoSet(NXB_UP);
	m_downButton.AutoSet(NXB_DOWN);
	m_btnAddResource.AutoSet(NXB_NEW);
	m_btnDeleteResource.AutoSet(NXB_DELETE);
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	m_btnAllowedPurposes.AutoSet(NXB_MODIFY);
	m_btnProviderLinking.AutoSet(NXB_MODIFY);
	// (j.jones 2012-04-11 11:23) - PLID 44174 - added ability to merge resources
	m_btnMergeResources.AutoSet(NXB_MODIFY);

	// (j.jones 2012-04-11 11:24) - PLID 44174 - disable the merge button if no access exists
	if(!(GetCurrentUserPermissions(bioMergeSchedulerResources) & (sptDynamic0 | sptDynamic0WithPass))) {
		m_btnMergeResources.EnableWindow(FALSE);
	}

	m_aryDeleteViews.RemoveAll();
	m_aryDeleteResources.RemoveAll();
	m_bSaveResourcePurpTypes = FALSE;

	//TES 7/26/2010 - PLID 39445 - Load our Location dropdown.
	m_pDefaultLocation = BindNxDataList2Ctrl(IDC_DEFAULT_VIEW_LOCATION);
	NXDATALIST2Lib::IRowSettingsPtr pLocRow = m_pDefaultLocation->GetNewRow();
	//TES 7/26/2010 - PLID 39445 - Add the {All Locations} option.
	pLocRow->PutValue(0, (long)-1);
	pLocRow->PutValue(1, _bstr_t(SCHEDULER_TEXT_FILTER__ALL_LOCATIONS));
	m_pDefaultLocation->AddRowSorted(pLocRow, NULL);
	//TES 7/26/2010 - PLID 39445 - Also add a <No Default> option
	pLocRow = m_pDefaultLocation->GetNewRow();
	pLocRow->PutValue(0, (long)-2);
	pLocRow->PutValue(1, _bstr_t(" <No Default> "));
	m_pDefaultLocation->AddRowSorted(pLocRow, NULL);


	//DRT 4/16/2004 - Hide the "Scheduling..." button if not NexSpa
	if(!IsSpa(TRUE)) {
		GetDlgItem(IDC_BTN_RESOURCEAPPTTYPES)->ShowWindow(SW_HIDE);
	}
	else {
		GetDlgItem(IDC_BTN_RESOURCEAPPTTYPES)->ShowWindow(SW_SHOW);
	}

	try {
		// Requery the view list
		{
			m_lstViews = BindNxDataListCtrl(this, IDC_VIEW_LIST, GetRemoteData(), true);
			
			IRowSettingsPtr pRow = m_lstViews->Row[-1];
			_variant_t varNull;
			varNull.vt = VT_NULL;
			pRow->Value[0] = varNull; // The one and only row that has a NULL ID is the one that indicates it's THIS USER'S view
			pRow->Value[1] = _bstr_t(" { Standard View }");
			m_lstViews->AddRow(pRow);

			// (b.cardillo 2003-06-24 17:12) - On July 12, 2002 d.thompson Added the m_nLastChosenRow to track the 
			// index of the last intentionally selected entry (or "chosen row") in the list of available views.  He 
			// added it "so if you change the default view then switch to another, it will know what to correctly save"

			// Try to set to the requested view
			if (m_nCurResourceViewID != CSchedulerView::srvUnknownResourceView) {
				// Default it to the row containing the given id
				if (m_nCurResourceViewID == CSchedulerView::srvCurrentUserDefaultView) {
					m_lstViews->SetSelByColumn(0, varNull);
				} else {
					m_lstViews->SetSelByColumn(0, m_nCurResourceViewID);
				}
				// If we couldn't set it to the one we wanted, we'll have to default to the first row
				if (m_lstViews->GetCurSel() == sriNoRow) {
					ASSERT(FALSE);
					m_lstViews->PutCurSel(0);
				}
			} else {
				// Default it to the first row
				ASSERT(FALSE);
				m_lstViews->PutCurSel(0);
			}
			
			// Regardless of what method was used to set it, now we want to get the official "current row" because 
			// that's the one we'll be loading down below
			m_nLastChosenRow = m_lstViews->GetCurSel();
		}

		// Requery the resource list
		{
			m_lstResource = BindNxDataListCtrl(this, IDC_RESOURCE_LIST, GetRemoteData(), false);

			IColumnSettingsPtr pCol = m_lstResource->GetColumn(rofProviderNames);

			//JMJ - removed 11/10/2003
			//the resource/provider linking is ASC-only			
			//if (IsSurgeryCenter()) {			

				pCol->ForeColor = RGB(128,128,128);

				/* Remove this when we support multiple providers per resource */
				_RecordsetPtr prs = CreateRecordset("SELECT PersonID, [Last] + ', ' + [First] + ' ' + [Middle] AS Name FROM ProvidersT INNER JOIN PersonT ON PersonT.ID = ProvidersT.PersonID WHERE PersonT.Archived = 0");
				CString strComboSource = "0;(None)";
				while (!prs->eof)
				{
					CString str, strName;
					//DRT 7/3/03 - This is a ; delimited list, if the provider name has a ;, 
					//		it throws things into chaos!  We'll just display without the semicolon,
					//		there's no valid reason a provider should have one.
					strName = AdoFldString(prs, "Name");
					strName.Remove(';');

					str.Format(";%d;%s", AdoFldLong(prs, "PersonID"),
						strName);
					strComboSource += str;
					prs->MoveNext();
				}
				pCol->ComboSource = (LPCTSTR)strComboSource;

			//JMJ - removed 11/10/2003
			//}
			//else {
			//	pCol->PutColumnStyle(csVisible|csFixedWidth);
			//	pCol->PutStoredWidth(0);
			//	pCol->Editable = FALSE;
			//}

			LoadResourceList();
		}

	} NxCatchAllCall("CResourceOrderDlg::OnInitDialog", {
		OnCancel();
		return FALSE;
	});

	//JMJ - removed 11/10/2003
	//if (!IsSurgeryCenter())
	//{
		//GetDlgItem(IDC_BTN_PROVIDERLINKING)->ShowWindow(SW_HIDE);
	//}
	ResolveButtons(m_lstResource->CurSel);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CResourceOrderDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CResourceOrderDlg)
	ON_EVENT(CResourceOrderDlg, IDC_RESOURCE_LIST, 10 /* EditingFinished */, OnEditingFinishedResourceList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CResourceOrderDlg, IDC_RESOURCE_LIST, 2 /* SelChanged */, OnSelChangedResourceList, VTS_I4)
	ON_EVENT(CResourceOrderDlg, IDC_RESOURCE_LIST, 9 /* EditingFinishing */, OnEditingFinishingResourceList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CResourceOrderDlg, IDC_VIEW_LIST, 16 /* SelChosen */, OnSelChosenViewList, VTS_I4)
	ON_EVENT(CResourceOrderDlg, IDC_RESOURCE_LIST, 8 /* EditingStarting */, OnEditingStartingResourceList, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CResourceOrderDlg, IDC_RESOURCE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedResourceList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CResourceOrderDlg, IDC_DEFAULT_VIEW_LOCATION, 1, CResourceOrderDlg::OnSelChangingDefaultViewLocation, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CResourceOrderDlg, IDC_DEFAULT_VIEW_LOCATION, 16, CResourceOrderDlg::OnSelChosenDefaultViewLocation, VTS_DISPATCH)
	ON_EVENT(CResourceOrderDlg, IDC_DEFAULT_VIEW_LOCATION, 20, CResourceOrderDlg::OnTrySetSelFinishedDefaultViewLocation, VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// Sets the relevences of all the items in the resource list
// Sets the checked items to positive relevences
// Sets the unchecked items to negative relevences
// Returns the MAX relevence
// Throws _com_error
long CResourceOrderDlg::UniquifyRelevences()
{
	//  1. Loop through the datalist backwards, setting the relevence of all checked rows to non-negative numbers starting with zero.
	//  2. Loop through the datalist forwards, setting the relevence of all unchecked rows to negative numbers starting with -1.
	long nNewRelevence;
	long nMaxRelevence;

	// 1. see above
	{
		nNewRelevence = 0;
		long p = m_lstResource->GetLastRowEnum();
		LPDISPATCH lpRow;
		while (p) {
			m_lstResource->GetPrevRowEnum(&p, &lpRow);
			// If the row is checked as being visible set the relevence and increment our relevence number
			IRowSettingsPtr pCurRow(lpRow);
			if (VarBool(pCurRow->Value[rofVisible]) && !VarBool(pCurRow->Value[rofInactive])) {
				pCurRow->Value[rofRelevence] = nNewRelevence;
				nNewRelevence++;
			}
			// Release lpRow so we decrement the reference count
			lpRow->Release();
		}
	}
	
	// Between step one and two, we can get our ultimate max relevence
	// Our nNewRelevence would now be equal to the count of 
	// checked rows, and the max would be one less than that
	nMaxRelevence = nNewRelevence - 1;

	// 2. see above
	{
		nNewRelevence = -1;
		long p = m_lstResource->GetFirstRowEnum();
		LPDISPATCH lpRow;
		while (p) {
			m_lstResource->GetNextRowEnum(&p, &lpRow);
			// If the row is checked as being visible set the relevence and decrement our relevence number
			IRowSettingsPtr pCurRow(lpRow);
			if (!VarBool(pCurRow->Value[rofVisible]) || VarBool(pCurRow->Value[rofInactive])) {
				pCurRow->Value[rofRelevence] = nNewRelevence;
				nNewRelevence--;
			}
			// Release lpRow so we decrement the reference count
			lpRow->Release();
		}
	}

	return nMaxRelevence;
}

void CResourceOrderDlg::OnMoveUpBtn() 
{
	//DRT 4/30/03 - Requires sptWrite permission to move anything in the dialog
	if (!CheckCurrentUserPermissions(bioSchedResourceOrder, sptWrite)) 
		return;

	try {
		long nCurSel = m_lstResource->CurSel;
		if (nCurSel != -1) {
			if (nCurSel > 0) {
				// Swap the relevences
				long nRelTemp = m_lstResource->Value[nCurSel][rofRelevence];
				m_lstResource->Value[nCurSel][rofRelevence] = m_lstResource->Value[nCurSel-1][rofRelevence];
				m_lstResource->Value[nCurSel-1][rofRelevence] = nRelTemp;
				// Remember something's changed so we'll need to save at the end
				m_bNeedToSave = TRUE;
				// Re-apply the sort
				m_lstResource->Sort();
				ResolveButtons(nCurSel-1);
			} else {
				// The first item is selected so do nothing because it can't be moved up
			}
		} else {
			// There is nothing selected
			MsgBox(MB_ICONINFORMATION|MB_OK, "Please select a resource");
		}
	} NxCatchAll("CResourceOrderDlg::OnMoveUpBtn");
}

void CResourceOrderDlg::OnMoveDownBtn() 
{
	//DRT 4/30/03 - Requires sptWrite permission to move anything in the dialog
	if (!CheckCurrentUserPermissions(bioSchedResourceOrder, sptWrite)) 
		return;

	try {
		long nCurSel = m_lstResource->CurSel;
		if (nCurSel != -1) {
			if (nCurSel < (m_lstResource->GetRowCount()-1)) {
				// Swap the relevences
				_variant_t varRelTemp = m_lstResource->Value[nCurSel][rofRelevence];
				m_lstResource->Value[nCurSel][rofRelevence] = m_lstResource->Value[nCurSel+1][rofRelevence];
				m_lstResource->Value[nCurSel+1][rofRelevence] = varRelTemp;
				// Remember something's changed so we'll need to save at the end
				m_bNeedToSave = TRUE;
				// Re-apply the sort
				m_lstResource->Sort();
				ResolveButtons(nCurSel+1);
			} else {
				// The first item is selected so do nothing because it can't be moved up
			}
		} else {
			// There is nothing selected
			MsgBox(MB_ICONINFORMATION|MB_OK, "Please select a resource");
		}
	} NxCatchAll("CResourceOrderDlg::OnMoveUpBtn");
}

void CResourceOrderDlg::LoadResourceList()
{
	CString strFrom, strWhere;
	CString strDeletedResources;
	CWaitCursor wc;

	long nDelResourceCount = m_aryDeleteResources.GetSize();
	for (long i=0; i<nDelResourceCount; i++) {
		CString str;
		str.Format("%d", m_aryDeleteResources[i]);
		strDeletedResources += str;
	}
	if (strDeletedResources.IsEmpty()) strDeletedResources = "-1";
	strDeletedResources.TrimRight(",");

	_variant_t varViewID = m_lstViews->Value[m_lstViews->CurSel][0];
	if (varViewID.vt != VT_I4) {
		// Built-in view
		strFrom.Format("ResourceT INNER JOIN UserResourcesT ON ResourceT.ID = UserResourcesT.ResourceID LEFT JOIN ResourceLocationConnectT ON ResourceLocationConnectT.ResourceID = ResourceT.ID "
			"LEFT JOIN ResourceProviderLinkT ON ResourceProviderLinkT.ResourceID = ResourceT.ID LEFT JOIN PersonT ON PersonT.ID = ResourceProviderLinkT.ProviderID LEFT JOIN ResourceUserLinkT ON ResourceUserLinkT.ResourceID = ResourceT.ID ");
		strWhere.Format("UserResourcesT.UserID = %d AND ResourceT.ID NOT IN (%s)", GetCurrentUserID(), strDeletedResources);
		GetDlgItem(IDC_BTN_COPYRESOURCESETTINGS)->ShowWindow(SW_SHOW);
		//TES 7/26/2010 - PLID 39445 - You can't have a default location for the built-in view.
		GetDlgItem(IDC_DEFAULT_LOCATION_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DEFAULT_VIEW_LOCATION)->ShowWindow(SW_HIDE);
	} else {
		if (VarLong(varViewID) != -1) {
			// Custom view (already saved)
			strFrom.Format("ResourceT INNER JOIN ResourceViewDetailsT ON ResourceT.ID = ResourceViewDetailsT.ResourceID LEFT JOIN ResourceLocationConnectT ON ResourceLocationConnectT.ResourceID = ResourceT.ID "
				"LEFT JOIN ResourceProviderLinkT ON ResourceProviderLinkT.ResourceID = ResourceT.ID LEFT JOIN PersonT ON PersonT.ID = ResourceProviderLinkT.ProviderID LEFT JOIN ResourceUserLinkT ON ResourceUserLinkT.ResourceID = ResourceT.ID ");
			strWhere.Format("ResourceViewDetailsT.ResourceViewID = %li AND ResourceT.ID NOT IN (%s)", VarLong(varViewID), strDeletedResources);
		} else {
			// Custom view (not saved yet)
			strFrom.Format("(SELECT ResourceT.ID, Item, -1 AS Relevence, LocationID, 15 AS DefaultInterval, Inactive, ProviderID FROM ResourceT LEFT JOIN ResourceLocationConnectT ON ResourceLocationConnectT.ResourceID = ResourceT.ID "
				"LEFT JOIN ResourceProviderLinkT ON ResourceProviderLinkT.ResourceID = ResourceT.ID LEFT JOIN PersonT ON PersonT.ID = ResourceProviderLinkT.ProviderID "
				//TES 9/11/2006 - One of the columns looks for ResourceT.ID, so make sure that exists.
				") AS BogusRelevanceT INNER JOIN (SELECT ID FROM ResourceT) AS ResourceT ON BogusRelevanceT.ID = ResourceT.ID LEFT JOIN ResourceUserLinkT ON ResourceUserLinkT.ResourceID = ResourceT.ID ");
		}
		GetDlgItem(IDC_BTN_COPYRESOURCESETTINGS)->ShowWindow(SW_HIDE);
		//TES 7/26/2010 - PLID 39445 - Show the default location combo.
		GetDlgItem(IDC_DEFAULT_LOCATION_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_DEFAULT_VIEW_LOCATION)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_VIEW_LIST)->SetFocus();
	}

	try {

		//TES 7/26/2010 - PLID 39445 - We also need to load the default location for this view.
		if(VarLong(varViewID, -1) != -1) {
			_RecordsetPtr rsLocation = CreateParamRecordset("SELECT LocationID FROM ResourceViewsT "
				"WHERE ID = {INT}", VarLong(varViewID));
			long nLocationID = AdoFldLong(rsLocation, "LocationID", -2);
			long nSel = m_pDefaultLocation->TrySetSelByColumn_Deprecated(0, nLocationID);
			if(nSel == sriNoRow) {
				//maybe it's inactive?
				_RecordsetPtr rsLoc = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", nLocationID);
				if(!rsLoc->eof) {
					m_nPendingLocationID = nLocationID;
					m_pDefaultLocation->PutComboBoxText(_bstr_t(AdoFldString(rsLoc, "Name", "")));
				}
				else 
					m_pDefaultLocation->PutCurSel(NULL);
			}
			else if(nSel == sriNoRowYet_WillFireEvent) {
				m_nPendingLocationID = nLocationID;
			}
		}

		// Requery

		// (c.haag 2006-05-08 11:12) - PLID 20470 - Disable the OK button. We don't want
		// anyone to be able to save changes until the resource list is fully loaded
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		GetDlgItem(IDC_NEW_RESOURCE_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_DELETE_RESOURCE_BTN)->EnableWindow(FALSE);

		m_lstResource->FromClause = _bstr_t(strFrom);
		m_lstResource->WhereClause = _bstr_t(strWhere);
		m_lstResource->Requery();

		//
		// (c.haag 2006-05-08 10:46) - PLID 20470 - All post-requery-completion code has been moved
		// to CResourceOrderDlg::OnRequeryFinishedResourceList
		//
		// Wait for it to finish
		//m_lstResource->WaitForRequery(dlPatienceLevelWaitIndefinitely);

		// Fill in the provider columns
		//
		// (c.haag 2006-05-08 10:46) - PLID 20470 - We now query for this information in the list itself.
		// Even when we support multiple providers per resource, we should still pull the names in the list,
		// even if it means adding a stored procedure
		//

		//for (i=0; i < m_lstResource->GetRowCount(); i++)
		//{			
			/*_RecordsetPtr prs = CreateRecordset("SELECT ProviderID FROM ResourceProviderLinkT INNER JOIN PersonT ON ResourceProviderLinkT.ProviderID = PersonT.ID WHERE ResourceID = %d",
				VarLong(m_lstResource->GetValue(i, rofID)));
			if (!prs->eof) {
				//DRT TODO - I'm not sure if it's possible to put in combo text on the embedded lists, but we should here
				m_lstResource->PutValue(i, rofProviderNames, prs->Fields->Item["ProviderID"]->Value);
			}*/

			/* Uncomment all this out when we support linking
			multiple providers to a resource */
			/*
			// Get a recordset of all the providers for this resource
			_RecordsetPtr prs = CreateRecordset("SELECT ProviderID, [Last] + ', ' + [First] + ' ' + [Middle] AS Name FROM ResourceProviderLinkT INNER JOIN PersonT ON ResourceProviderLinkT.ProviderID = PersonT.ID WHERE ResourceID = %d",
				VarLong(m_lstResource->GetValue(i, rofID)));
			CString strProviderNames;
			CString strProviderIDs, strProviderID;

			// Build our provider names and provider IDs strings
			while (!prs->eof)
			{
				strProviderNames += AdoFldString(prs, "Name") + ", ";
				strProviderID.Format("%d", AdoFldLong(prs, "ProviderID"));
				strProviderIDs += strProviderID + " ";
				prs->MoveNext();
			}

			// Get rid of the extra characters
			if (strProviderNames.GetLength() >= 2)
				strProviderNames = strProviderNames.Left( strProviderNames.GetLength() - 2 );
			if (strProviderIDs.GetLength() >= 1)
				strProviderIDs = strProviderIDs.Left( strProviderIDs.GetLength() - 1 );

			// Put the provider name and provider IDs strings in the list
			m_lstResource->PutValue(i, rofProviderNames, _bstr_t(strProviderNames));
			m_lstResource->PutValue(i, rofProviderIDs, _bstr_t(strProviderIDs));*/
		//}

		m_bNeedToSave = FALSE;
	} NxCatchAll("CResourceOrderDlg::LoadResourceList");
}

bool ValidateDeleteResource(long nResourceId)
{
	// Figure out how many appointments would be affected
	long nApptCount = 0;
	long nCanAppNoPat = 0;
	_RecordsetPtr rsCanAppNoPat;
	bool bSingle; // stores whether messagebox sentence is single
	GetApptCountForResource(nResourceId, nApptCount);

	// c.haag 8/26/05
	// PLID 16820 - Don't delete the resource if it's in use by the NexPDA link
	CString strOutlookUsers;
	_RecordsetPtr prsOutlook = CreateRecordset("select last + ', ' + first + ' ' + middle as username from outlookprofilet "
		"left join persont on persont.id = outlookprofilet.userid "
		"where outlookprofilet.userid in (select userid from outlookcategoryt where id in "
		"(select categoryid from outlookaptresourcet where aptresourceid = %d))", nResourceId);
	FieldsPtr fOutlook = prsOutlook->Fields;
	while (!prsOutlook->eof)
	{	strOutlookUsers += AdoFldString(fOutlook, "Username", "") + "\n";
		prsOutlook->MoveNext();
	}
	if (strOutlookUsers.GetLength()) {
		// (z.manning 2009-11-12 15:36) - PLID 31879 - May also be NexSync
		MsgBox(MB_OK|MB_ICONINFORMATION,
			"You may not delete this resource because it is in use by the following NexPDA or NexSync profiles:\n\n" +
			strOutlookUsers);
		return false;
	}

	// m.carlson 4/15/05
	// We need to check out how many cancelled, patientless appointments there are
	// and below, we'll offer to delete them for the user (because they can't do it manually)
	// with a severe warning about the number
	if (nApptCount > 0)
	{
		rsCanAppNoPat = CreateRecordset("SELECT Count(*) AS nCount FROM appointmentresourcet "
			"left join appointmentst on appointmentresourcet.appointmentID = appointmentsT.ID where resourceid = %li "
			"and patientid = -25 and status = 4",nResourceId);

		nCanAppNoPat = AdoFldLong(rsCanAppNoPat,"nCount");
		rsCanAppNoPat->Close();
	}

	// Warn the user (or don't allow the deletion depending on the scenario)
	if (nApptCount > 0 && nApptCount != nCanAppNoPat) {
		// Can't remove resources if they have appointments
		bSingle = (nApptCount == 1);
		MsgBox(MB_OK|MB_ICONINFORMATION, 
			"There %s %li %s scheduled for the selected resource.  Please move or \n"
			"completely delete %s before proceeding.\n\n"
			"Use the 'Daily Schedule' and the 'Appointments Cancelled' reports to view a listing of \n"
			"appointments, filtered by resource.", 
			bSingle ? "is" : "are", nApptCount, bSingle ? "appointment" : "appointments", 
			bSingle ? "this appointment" : "these appointments");
		// Delete is not allowed
		return false;
	} else if (nCanAppNoPat > 0) {
		bSingle = (nCanAppNoPat == 1);
		if (MsgBox(MB_YESNO|MB_ICONQUESTION, 
			 "There %s %li %s associated with this resource, which %s not associated with a patient, and "
			 "which %s been cancelled. %s must be deleted permanently from the database before deleting this resource. "
			 "Because %s cannot be deleted manually, Practice will delete %s for you automatically as part of "
			 "deleting the resource. Are you sure you want to continue deleting the selected resource and its cancelled, "
			 "patientless %s?\n\n"
			 "If you click yes, the resource and its cancelled, patientless appointments will be permanently deleted "
			 "from all views.",bSingle ? "is" : "are",nCanAppNoPat,bSingle ? "appointment" : "appointments",bSingle ? "is" : "are",
			 bSingle ? "has" : "have",bSingle ? "This" : "These",bSingle ? "this" : "these",bSingle ? "it" : "them",bSingle ? "appointment" : "appointments") != IDYES)
			// Delete was canceled by the user
			return false;
	} 
	
	// (d.moore 2007-10-24) - PLID 4013 - We need to check to see if the resource is used in the waiting list.
	_RecordsetPtr rs = CreateRecordset("SELECT TOP 1 ID FROM WaitingListItemResourceT WHERE ResourceID = %li", nResourceId);
	if (!rs->eof) {
		MsgBox(MB_OK|MB_ICONINFORMATION,
			"You may not delete this resource because it is in use on the waiting list.");
		return false;
	}

	//DRT 6/11/2008 - PLID 30306 - If the resource is tied to a default superbill, don't let them delete it.
	rs = CreateParamRecordset("SELECT GroupID FROM SuperbillTemplateResourceT WHERE ResourceID = {INT};", nResourceId);
	if(!rs->eof) {
		MsgBox(MB_OK|MB_ICONINFORMATION, "You may not delete a resource which is tied to a default superbill configuration.");
		return false;
	}

	// (b.cardillo 2011-02-26 11:01) - These should have been closed or just put in their own code blocks, but weren't.
	rs->Close();
	prsOutlook->Close();

	// (b.cardillo 2011-02-26 10:15) - PLID 40419 - Check for appointment prototypes that reference this resource
	{
		// Get the list of prototypes, if any, that depend on this resource
		CString strReferencedPrototypes = GenerateDelimitedListFromRecordsetColumn(CreateParamRecordset(
				_T("SELECT Name FROM ApptPrototypeT WHERE ID IN (\r\n")
				_T(" SELECT PS.ApptPrototypeID \r\n")
				_T(" FROM ApptPrototypePropertySetT PS \r\n")
				_T(" INNER JOIN ApptPrototypePropertySetResourceSetT PSRS ON PS.ID = PSRS.ApptPrototypePropertySetID \r\n")
				_T(" INNER JOIN ApptPrototypePropertySetResourceSetDetailT PSRSD ON PSRS.ID = PSRSD.ApptPrototypePropertySetResourceSetID \r\n")
				_T(" WHERE PSRSD.ResourceID = {INT} \r\n")
				_T(") \r\n")
				_T("ORDER BY Name \r\n")
				, nResourceId)
			, AsVariant("Name"), "", "\r\n");
		if (!strReferencedPrototypes.IsEmpty()) {
			MsgBox(MB_OK|MB_ICONINFORMATION, 
				_T("You may not delete this resource because it is referenced by the following Appointment Prototypes:\r\n\r\n%s")
				, strReferencedPrototypes);
			return false;
		}
	}
		
	if (MsgBox(MB_YESNO|MB_ICONQUESTION, 
		 "Are you sure you want to delete the selected resource?\n\n"
		 "If you click yes, the resource will be permanently deleted from all views.") != IDYES) {
		// Delete was canceled by the user
		return false;
	}
	
	return true;
}

void CResourceOrderDlg::CommitDeletedResources(CString& strSqlBatch)
{
	bool bSkipFlag;
	long nCount = m_aryDeleteResources.GetSize();
	for (long i=0; i<nCount; i++) {
		long nDeletedResourceID = m_aryDeleteResources[i];
		if (nDeletedResourceID > 0) {

			// m.carlson 4/15/05
			// We need to check out how many cancelled, patientless appointments there are
			// and below, we'll then delete them for the user because we already asked them about it
			// in ValidateDeleteResources()
			_RecordsetPtr rsCanAppNoPatList = CreateRecordset("SELECT appointmentID AS ID FROM appointmentresourcet "
				"left join appointmentst on appointmentresourcet.appointmentID = appointmentsT.ID where resourceid = %li "
				"and patientid = -25 and status = 4",nDeletedResourceID);
			// Don't try to MoveFirst if we have no records
			if (!rsCanAppNoPatList->eof)
				rsCanAppNoPatList->MoveFirst();
			
			// This flag is to skip the resource deletion below - it will only be set if appointmentdeletenohistory()
			// returns false. Looking at its current code (and the parameters I sent), it will only return false if
			// it encounters an exception. By skipping the lines below, we'd just be avoiding a second consecutive exception.
			bSkipFlag = false;
			while (!bSkipFlag && !rsCanAppNoPatList->eof)
			{
				if (!AppointmentDeleteNoHistory(AdoFldLong(rsCanAppNoPatList,"ID"),TRUE))
					bSkipFlag = true;

				rsCanAppNoPatList->MoveNext();
			}
			rsCanAppNoPatList->Close();

			if (!bSkipFlag)
			{
				// Actually delete the resource (or try at least)

				// (j.jones 2012-04-11 09:48) - PLID 44174 - added global function for deleting resources
				DeleteSchedulerResource(strSqlBatch, nDeletedResourceID);

				//JMJ - 06/04/2003 - Delete the associated permission
				DeleteUserDefinedPermission(bioSchedIndivResources, nDeletedResourceID);
			}
			else
			{
				_RecordsetPtr rsName = CreateRecordset("SELECT ITEM FROM ResourceT WHERE ID = %li",nDeletedResourceID);
				if (rsName->eof)
					MessageBox("Error deleting resource! Please contact Nextech support.",MB_OK);
				else
				{
					CString cstrResourceName = AdoFldString(rsName,"ITEM");
					rsName->Close();
					MessageBox("Error deleting resource \"" + cstrResourceName + "\"! Please contact Nextech Support.",MB_OK);
				}
			}
		}
	}
	//we don't want to clear out the array until we are sure the deletions have been commited, which is is after
	//the transaction in SaveResourceList
}

void CResourceOrderDlg::CommitDeletedViews(CString& strSqlBatch)
{
	long nCount = m_aryDeleteViews.GetSize();
	for (long i=0; i<nCount; i++) {
		long nDeletedViewID = m_aryDeleteViews[i];
		if (nDeletedViewID > 0) {
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ResourceViewDetailsT WHERE ResourceViewID = %li", nDeletedViewID);
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ResourceViewsT WHERE ID = %li", nDeletedViewID);
		}

		// Make sure noone is using this view anymore
		AddStatementToSqlBatch(strSqlBatch, "UPDATE ConfigRT Set IntParam = (SELECT TOP 1 ID FROM ResourceViewsT) WHERE Name = 'CurResourceView' AND IntParam = %d",
			nDeletedViewID);
	}
	//we don't want to clear out the array until we are sure the deletions have been commited, which is is after
	//the transaction in SaveResourceList
}

// Returns CSchedulerView::srvCurrentUserDefaultView if the view is a built-in view and therefore could not be saved
// Returns >0 if the view was saved.  This number is the view's ID in the database
long CResourceOrderDlg::SaveViewName(long nSaveViewIndex)
{
	// nViewID is the ID of our currently selected view.  CSchedulerView::srvCurrentUserDefaultView indicates it's a user's built-in view, not a custom view
	_variant_t varViewID = m_lstViews->Value[nSaveViewIndex][0];
	_variant_t varViewName = m_lstViews->Value[nSaveViewIndex][1];
	long nViewID = CSchedulerView::srvCurrentUserDefaultView;
	// Create the custom view if necessary
	if (varViewID.vt == VT_I4) {
		// Custom view
		nViewID = VarLong(varViewID);

		//TES 7/26/2010 - PLID 39445 - Save the default location
		NXDATALIST2Lib::IRowSettingsPtr pLocRow = m_pDefaultLocation->CurSel;
		long nLocationID = -3;
		if(pLocRow) {
			nLocationID = VarLong(pLocRow->GetValue(0),-2);
		}
		if(nLocationID == -3) {
			if(m_pDefaultLocation->IsComboBoxTextInUse) {
				//TES 7/26/2010 - PLID 39445 - It's an inactive location, we'll have stored the id in m_nPendingLocationID
				nLocationID = m_nPendingLocationID;
			}
		}
		CString strLocationID = "NULL";
		//TES 7/26/2010 - PLID 39445 - If the default is <No Default>, then leave the filter as whatever's currently selected.
		if(nLocationID != -2) {
			strLocationID = AsString(nLocationID);
		}
		if (nViewID == -1) {
			// New custom view
			nViewID = NewNumber("ResourceViewsT", "ID");
			if (nViewID > 0) {
				// Create the view and add all resources to it with a relevence of -1
				ExecuteSql(
					"INSERT INTO ResourceViewsT (ID, Name, LocationID) "
					"VALUES (%li, '%s', %s)", 
					nViewID, _Q(VarString(varViewName)), strLocationID);
				ExecuteSql(
					"INSERT INTO ResourceViewDetailsT (ResourceViewID, ResourceID, Relevence) "
					"SELECT %li AS ResourceViewID, ID AS ResourceID, -1 AS Relevence FROM ResourceT", 
					nViewID);
			} else {
				AfxThrowNxException("Could not create new resource view");
			}
		} else {
			ExecuteSql(
				"UPDATE ResourceViewsT SET Name = '%s', LocationID = %s "
				"WHERE ID = %li", 
				_Q(VarString(varViewName)), strLocationID, nViewID);
		}
	} else {
		// Built-in view
		nViewID = CSchedulerView::srvCurrentUserDefaultView;
	}
	return nViewID;
}

// (j.jones 2010-04-20 09:14) - PLID 38273 - removed in favor of the standardized batch functions
/*
void CResourceOrderDlg::AddStatementToSqlBatch(CString& strSqlBatch, LPCTSTR strFmt, ...)
{
	CString strSql;

	// Parse string inserting arguments where appropriate
	va_list argList;
	va_start(argList, strFmt);
	strSql.FormatV(strFmt, argList);
	va_end(argList);

	strSqlBatch += strSql + ";";
}
*/

BOOL CResourceOrderDlg::SaveResourceList(long nSaveViewIndex)
{
	try {
		// Loop through the datalist forwards, writing the relevence of each resource to the ResourceT table
		CWaitCursor wc;
		long nMaxRelevence = UniquifyRelevences();
		if (nMaxRelevence >= 0) {
			
			long nViewID = CSchedulerView::srvCurrentUserDefaultView;
			CString strSqlBatch;
			BOOL bUpdateSecurityObjectT = FALSE;
			CString strCaseStatements, strLocationCaseStatements, strIDs, strIDsWithLocation;

			// Begin a transaction
			BEGIN_TRANS("CResourceOrderDlg::SaveResourceList") {
				
				// Make sure any views that were deleted actually get removed from the database
				CommitDeletedViews(strSqlBatch);

				// Make sure any resources that were deleted actually get removed from the database
				CommitDeletedResources(strSqlBatch);
				
				// Make sure our custom view (if that's what we're on) exists in the database
				nViewID = SaveViewName(nSaveViewIndex);

				//first loop through the list and build a case statement to update all of the existing resources
				//we need to build case statements because if we try to rename a resource to a name that existed before 
				//but had its name changed, it will think the old name is still in the list and give us a primary key violation
				long p = m_lstResource->GetFirstRowEnum();
				LPDISPATCH lpRow;
				while(p){
					m_lstResource->GetNextRowEnum(&p, &lpRow);
					IRowSettingsPtr pCurRow = lpRow;
					//get the ID of the resource
					long nResourceId = pCurRow->GetValue(rofID);
					long nLocationID = VarLong(pCurRow->GetValue(rofLocationID), 0);
					if(nResourceId > 0)
					{
						// Convert the item to text appropriate for an action query
						CString strItemString = VarString(pCurRow->Value[rofName]);
						strItemString.TrimLeft(); strItemString.TrimRight();
											
						//add the current name and ID to their respective CStrings so that we can update their values after
						//we are finished gathering the existing resources
						CString strCaseStatementToAdd, strIDToAdd;
						strCaseStatementToAdd.Format(" WHEN ID = %li THEN '%s' ", nResourceId, _Q(strItemString));
						strCaseStatements += strCaseStatementToAdd;
						//(e.lally 2010-07-19) PLID 39628 - Use ResourceT.ID now
						strCaseStatementToAdd.Format(" WHEN ResourceT.ID = %d THEN %d ", nResourceId, nLocationID);
						strLocationCaseStatements += strCaseStatementToAdd;
						strIDToAdd.Format("%li, ", nResourceId);
						strIDs += strIDToAdd;
						//(e.lally 2010-07-19) PLID 39628 - Allow resources to have no default location connection
						if(nLocationID != -1){
							strIDsWithLocation += strIDToAdd;
						}
					}
					lpRow->Release();
				}

				//cleanup the strings
				if (!strCaseStatements.IsEmpty() && !strIDs.IsEmpty())
				{
					DeleteEndingString(strIDs, ", ");
					//(e.lally 2010-07-19) PLID 39628 - Trim our list of resource IDs with locations too
					DeleteEndingString(strIDsWithLocation, ", ");
					AddStatementToSqlBatch(strSqlBatch, "UPDATE ResourceT SET Item = CASE %s END WHERE ID IN (%s)", strCaseStatements, strIDs);
					strCaseStatements.Replace("ID","ObjectValue");
					//PLID 20013, this needs to take BuiltInID into account also otherwise its updating names of a lot of things it shouldn't
					AddStatementToSqlBatch(strSqlBatch, "UPDATE SecurityObjectT SET DisplayName = CASE %s END WHERE ObjectValue IN (%s) AND BuiltInID = %li", strCaseStatements, strIDs, bioSchedIndivResources);
					// (c.haag 2003-07-30 15:16) - Save locations. We make sure every resource has one in ResourceLocationConnectT.
					//(e.lally 2010-07-19) PLID 39628 - Allow resources to have no default location connection. Delete all the entries, then add back just the ones with locations selected
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ResourceLocationConnectT");
					//(e.lally 2010-07-19) PLID 39628 - Use the case statement when selecting the location IDs
					//(a.walling 2010-09-20 22:57) PLID 40591 - This will cause an error if no resources have a default location.
					if (!strIDsWithLocation.IsEmpty()) {
						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ResourceLocationConnectT SELECT ID, CASE %s END FROM ResourceT WHERE ID IN(%s) AND ID NOT IN (SELECT ResourceID FROM ResourceLocationConnectT)", strLocationCaseStatements, strIDsWithLocation);
					}
					bUpdateSecurityObjectT = TRUE;
				}

				//Ok now go through and add all of the new Resources, then update the relevences as well as the Provider
				//Linking for each resouce, new or old

				// Loop forwards through the resource datalist
				CString strItemString;
				long r = m_lstResource->GetFirstRowEnum();
				int nNewResourceCount = 0;
				// (v.maida 2016-01-22 11:27) - PLID 68035 - Store a vector of inactivated or deleted resources, for later checking against the default FFA resource preference.
				std::vector<long> vecInactiveOrDeletedResources;
				while (r) {
					m_lstResource->GetNextRowEnum(&r, &lpRow);
					IRowSettingsPtr pCurRow(lpRow);
					// Get the resource ID of the given row
					long nResourceId = VarLong(pCurRow->Value[rofID]);
					// Get the location ID
					long nLocationID = VarLong(pCurRow->GetValue(rofLocationID), 0);
					// Get the default interval.
					CString strInterval;
					if(pCurRow->Value[rofDefaultInterval].vt == VT_I4 && VarLong(pCurRow->Value[rofDefaultInterval]) != -1) strInterval.Format("%li", VarLong(pCurRow->Value[rofDefaultInterval]));
					else strInterval = "NULL";
					BOOL bInactive = VarBool(pCurRow->Value[rofInactive]);
							
					// Convert the item to text appropriate for an action query
					strItemString = VarString(pCurRow->Value[rofName]);
					strItemString.TrimLeft(); strItemString.TrimRight();
					// Run the appropriate action query (-1 means it's new, other means it already exists)
					if (nResourceId == -1) {
						// Create the new resource
						// (z.manning, 07/27/2006) - PLID 21656 - NewNumber doesn't work becaue we generate the same number
						// for all new resources. Normally we'd use a variable in the sql batch, but we need the resource
						// ID in code for permissions, among other things, so let's just keep track manually.
						nResourceId = NewNumber("ResourceT", "ID") + nNewResourceCount++;
						if (nResourceId >= 0) {
							//TODO:  Noone knows what the Priority and ItemType values are ... I set them to the values that the other resources already had
							AddStatementToSqlBatch(strSqlBatch, 
								"INSERT INTO ResourceT (Item, ID, ItemType, Inactive) "
								"VALUES ('%s', %li, '%s', %d)", 
								_Q(strItemString), nResourceId, "Doctor", bInactive);

							// (c.haag 2004-06-10 10:17) PLID 12928 - Don't forget ResourcePurposeTypeT
							//TES 7/15/2004 - PLID 13466 - Also don't forget not to fill in invalid purpose/type combos.
							// (c.haag 2008-12-09 17:27) - PLID 32376 - Filter out inactive procedures
							AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ResourcePurposeTypeT (ResourceID, AptTypeID, AptPurposeID) "
								"SELECT %d, AptTypeID, AptPurposeID FROM AptPurposeTypeT WHERE AptPurposeID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) ",
								nResourceId);

							// Add the resource to all custom views
							AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ResourceViewDetailsT (ResourceViewID, ResourceID, Relevence, DefaultInterval) "
								"SELECT ID AS ResourceViewID, %li AS ResourceID, -1 AS Relevence, %s AS DefaultInterval FROM ResourceViewsT", nResourceId, strInterval);
							// Add the resource to all users' built-in views
							AddStatementToSqlBatch(strSqlBatch, "INSERT INTO UserResourcesT (UserID, ResourceID, Relevence, DefaultInterval) "
								"SELECT PersonID AS UserID, %li AS ResourceID, -1 AS Relevence, %s AS DefaultInterval FROM UsersT", nResourceId, strInterval);
							// Assign the location to the resource
							//(e.lally 2010-07-19) PLID 39628 - Allow resources to have no default location connection
							if(nLocationID != -1){
								AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ResourceLocationConnectT (ResourceID, LocationID) VALUES (%d, %d)", nResourceId,
									nLocationID);
							}

							//JMJ - 6/4/2003 - create the permission info for this resource, default full control to all
							//JMM - 01/23/2006 - I modified the function to take the current permissions, this was previously hard coded to 21, so I added that for this call
							AddUserDefinedPermission(nResourceId,63,strItemString,"Controls access to view or schedule appointments for this resource.", -94, 21);

							// Save the new ID back to the datalist
							pCurRow->Value[rofID] = nResourceId;
						} else {
							AfxThrowNxException("Could not create new resource record for resource %s",  strItemString);
						}
					}

					// Update any provider linking information
					/* Uncomment all this out when we support linking
					multiple providers to a resource */
/*					CString strProviderIDs = VarString(pCurRow->Value[rofProviderIDs], "");
					ExecuteSql("DELETE FROM ResourceProviderLinkT WHERE ResourceID = %d", nResourceId);
					while (strProviderIDs.GetLength())
					{
						CString strProviderID;

						if (-1 != strProviderIDs.Find(' '))
						{
							strProviderID = strProviderIDs.Left(strProviderIDs.Find(' '));
							strProviderIDs = strProviderIDs.Right( strProviderIDs.GetLength() - strProviderIDs.Find(' ') - 1);
						}
						else
						{
							strProviderID = strProviderIDs;
							strProviderIDs.Empty();
						}

						ExecuteSql("INSERT INTO ResourceProviderLinkT (ResourceID, ProviderID) VALUES (%d, %s)",
							nResourceId, strProviderID);
					}*/
					//If any table references the ResourceProviderLinkT in the future, we'll have to update this table in 
					//	either the existing loop because we won't be able to delete from it
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ResourceProviderLinkT WHERE ResourceID = %d", nResourceId);
					if (VarLong(pCurRow->Value[rofProviderNames], -1) > 0)
					{
						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ResourceProviderLinkT (ResourceID, ProviderID) VALUES (%d, %d)",
							nResourceId, VarLong(pCurRow->Value[rofProviderNames]));
					}

					// (a.wetta 2007-01-11 12:55) - PLID 16065 - Update the user assigned to this resource
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ResourceUserLinkT WHERE ResourceID = %li", nResourceId);
					// Only add a new user if there is one selected
					if (_variant_t(pCurRow->Value[rofUserID]).vt != VT_NULL && _variant_t(pCurRow->Value[rofUserID]).vt != VT_EMPTY) {
						if (VarLong(pCurRow->Value[rofUserID], -1) > 0)
						{
							AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ResourceUserLinkT (ResourceID, UserID) VALUES (%li, %li)",
								nResourceId, VarLong(pCurRow->Value[rofUserID]));
						}
					}

					// If any restricstion are ever put on the relevences, then we'll have to update them when renaming the
					// resources
					// Update the relevence of this resource in the appropriate place in the appropriate table
					if (nViewID > 0) {
						// Saving a custom view
						AddStatementToSqlBatch(strSqlBatch, 
							"UPDATE ResourceViewDetailsT SET Relevence = %li, DefaultInterval = %s "
							"WHERE ResourceViewID = %li AND ResourceID = %li",
							VarLong(pCurRow->Value[rofRelevence]), strInterval, nViewID, nResourceId);
					} else {
						// Saving a built-in view
						AddStatementToSqlBatch(strSqlBatch, 
							"UPDATE UserResourcesT SET Relevence = %li, DefaultInterval = %s "
							"WHERE UserID = %li AND ResourceID = %li",
							VarLong(pCurRow->Value[rofRelevence]), strInterval, GetCurrentUserID(), nResourceId);
					}

					// (c.haag 2005-06-29 11:39) - PLID 6400 - Save the resource inactive state
					AddStatementToSqlBatch(strSqlBatch, "UPDATE ResourceT SET Inactive = %d WHERE ID = %d", bInactive, nResourceId);

					lpRow->Release();

					// (v.maida 2016-01-22 11:27) - PLID 68035 - If the current resource has been inactivated, add it to a vector of inactivated resources,
					// for later checking against the default FFA resource preference.
					if (bInactive)
						vecInactiveOrDeletedResources.push_back(nResourceId);
				}

				// (j.jones 2010-04-20 09:16) - PLID 38273 - converted to a traditional ExecuteSqlBatch
				ExecuteSqlBatch(strSqlBatch);

				// (v.maida 2016-01-22 11:27) - PLID 68035 - If there are any resources that were deleted, then they should now be gone,
				// so add the deleted resources from m_aryDeleteResources in with our vector of inactivated resources, so that both deleted and
				// inactivated resources can be checked to see if they relate to the default FFA Resource preference.
				for (int i = 0; i < m_aryDeleteResources.GetSize(); i++)
				{
					vecInactiveOrDeletedResources.push_back(m_aryDeleteResources[i]);
				}

				// (v.maida 2016-01-22 11:27) - PLID 68035 - Revert the Default FFA resource preference for all relevant users, if necessary.
				if ((int)vecInactiveOrDeletedResources.size() > 0 && UsersHaveResourcesAsDefaultFFAResourcePref(vecInactiveOrDeletedResources))
					RevertDefaultFFAResourcePrefForResources(vecInactiveOrDeletedResources);

				// nothing else has caused a rollback of the data so go ahead and clear out the arrays that had the resources
				// and views, we also want to clear these out inside the transaction because if an exception is thrown, we want 
				// to make sure these arrays haven't been cleared
				m_aryDeleteResources.RemoveAll();
				m_aryDeleteViews.RemoveAll();

			} END_TRANS("CResourceOrderDlg::SaveResourceList");

			// (c.haag 2006-05-22 13:50) - PLID 20609 - Update our local cache of SecurityObjectT
			if (bUpdateSecurityObjectT) {
				CString strWhere;
				strWhere.Format("ObjectValue IN (%s) AND BuiltInID = %li", strIDs, bioSchedIndivResources);
				LoadUserDefinedPermissions(strWhere);
				CClient::RefreshTable(NetUtils::UserDefinedSecurityObject, -1);
			}

			// (b.cardillo 2003-06-30 17:21) We used to try to manually update the views here, but that's 
			// incorrect.  What we do now (correctly) is just let the caller if this dialog decide what to 
			// do.  And the only places it's called from currently are the scheduler module itself (which 
			// sets itself to the requested view, in turn causing it to reload all resources) and the 
			// scheduler tab of the administrator module, which detects if the scheduler module is open 
			// and if so asks it to reload the current view.
			
			// Now that the view is saved, make sure the datalist combo has the right ID
			if (nViewID > 0) {
				m_lstViews->Value[nSaveViewIndex][0] = nViewID;
			}

			// Now tell everyone to repaint their scheduler view if
			// possible
			CClient::RefreshTable(NetUtils::Resources);

			m_bNeedToSave = FALSE;

			return TRUE;
		} else {
			// There wasn't anything checked!!!
			MsgBox(MB_ICONINFORMATION|MB_OK, "Please check at least one active resource to be visible");
			return FALSE;
		}
	} NxCatchAll("CResourceOrderDlg::SaveResourceList");
	return FALSE;
}

void CResourceOrderDlg::OnOK() 
{
	try {
		// Save the currently selected view
		long nCurSel = m_lstViews->CurSel;
		
		if(nCurSel == -1) {
			CDialog::OnOK();
			return;
		}

		if (!SaveResourcePurpTypes())
		{
			return;
		}
		if (SaveResourceList(nCurSel)) {
			
			// Remember what was selected and set our outbound member variable so the caller get get it too
			ASSERT(nCurSel == m_nLastChosenRow);
			m_nCurResourceViewID = VarLong(m_lstViews->Value[nCurSel][0], CSchedulerView::srvCurrentUserDefaultView);

			//TES 7/27/2010 - PLID 39445 - Also set the chosen location (so the scheduler can update itself without accessing data).
			m_nCurLocationID = -2;
			if(m_nCurResourceViewID != CSchedulerView::srvCurrentUserDefaultView) {
				long nLocationID = -3;
				NXDATALIST2Lib::IRowSettingsPtr pLocRow = m_pDefaultLocation->CurSel;
				if(pLocRow) {
					nLocationID = VarLong(pLocRow->GetValue(0),-2);
				}
				if(nLocationID == -3) {
					if(m_pDefaultLocation->IsComboBoxTextInUse) {
						//TES 7/26/2010 - PLID 39445 - It's an inactive location, we'll have stored the id in m_nPendingLocationID
						nLocationID = m_nPendingLocationID;
					}
				}
				m_nCurLocationID = nLocationID;
			}

			// We're done
			CDialog::OnOK();
		}
	} NxCatchAll("CResourceOrderDlg::OnOK");
}

void CResourceOrderDlg::OnEditingFinishedResourceList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	ASSERT(nRow != -1);
	if (nRow != -1) {
		ASSERT(nCol == rofVisible || nCol == rofName || nCol == rofProviderNames || nCol == rofInactive || nCol == rofLocationID || nCol == rofDefaultInterval || nCol == rofUserID);
		switch (nCol) {
		case rofVisible:
			if (bCommit) {
				ResolveButtons(nRow);
			}
			break;
		case rofName:
			// This is taken care of when saving (if the user clicks ok)
			break;
		case rofProviderNames:
			// This is taken care of when saving (if the user clicks ok)
			break;
		case rofLocationID:
			// This is taken care of when saving (if the user clicks ok)
			break;
		case rofInactive:
			if (bCommit && VarBool(varNewValue)) {
				m_lstResource->Value[nRow][rofVisible] = _variant_t(VARIANT_FALSE, VT_BOOL);
				ResolveButtons(nRow);
			}
			break;
		case rofUserID:
			// This is taken care of when saving (if the user clicks ok)
			break;
		default:
			// Do nothing because this case is asserted above
			break;
		}
	}
}

void CResourceOrderDlg::OnSelChangedResourceList(long nNewSel) 
{
	// Flag the fact we need to save our changes
	ResolveButtons(nNewSel);
}

void CResourceOrderDlg::ResolveButtons(long nIndex)
{
	if (nIndex == -1) {
		// The index is invalid so disable both
		GetDlgItem(IDC_MOVE_UP_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_MOVE_DOWN_BTN)->EnableWindow(FALSE);
	} else {
		// Enable/Disable buttons according to what's possible based on the index
		if (nIndex <= 0) {
			GetDlgItem(IDC_MOVE_UP_BTN)->EnableWindow(FALSE);
		} else {
			GetDlgItem(IDC_MOVE_UP_BTN)->EnableWindow(TRUE);
		}

		if (nIndex >= (m_lstResource->GetRowCount()-1)) {
			GetDlgItem(IDC_MOVE_DOWN_BTN)->EnableWindow(FALSE);
		} else {
			GetDlgItem(IDC_MOVE_DOWN_BTN)->EnableWindow(TRUE);
		}
	}
}

void CResourceOrderDlg::OnNewResourceBtn() 
{
	if(!CheckCurrentUserPermissions(bioAdminScheduler,sptWrite))
		return;

	try {
		CString strItem;
		if (InputBoxLimited(this, "Enter a new resource", strItem, "",50,false,false,NULL) == IDOK) {
			strItem.TrimLeft(); strItem.TrimRight();
			//TS 12-21-2001: Check the datalist, not the data, this is all volatile.
			//check if this String already exists as a resource
			/*_RecordsetPtr rs;
			CString str;
			str.Format("SELECT Item FROM ResourceT WHERE Item = '%s'", _Q(strItem));
			rs = CreateRecordset(str);			
			if(rs->GetRecordCount() > 0)*/
			if(ValidateResourceName(strItem)){
				long nMaxRelevence;
				if (m_lstResource->GetRowCount() == 0) {
					nMaxRelevence = -1;
				} else {
					nMaxRelevence = m_lstResource->Value[0][rofRelevence];
				}

				// Build the new row
				IRowSettingsPtr pNewRow = m_lstResource->Row[-1];
				_variant_t vInactive;
				vInactive.vt = VT_BOOL;
				vInactive.boolVal = FALSE;
				pNewRow->Value[rofID] = (long)-1; // New row indicator
				pNewRow->Value[rofVisible] = _variant_t(VARIANT_TRUE, VT_BOOL); // Default to visible resource
				pNewRow->Value[rofName] = _bstr_t(strItem); // Resource name
				pNewRow->Value[rofRelevence] = (long)(nMaxRelevence+1); // Default to highest relevence
				pNewRow->Value[rofProviderNames] = long(0);
				pNewRow->Value[rofProviderIDs] = "";
				pNewRow->Value[rofLocationID] = GetCurrentLocationID();
				pNewRow->Value[rofUserID] = (long)-1; // No user
				pNewRow->Value[rofInactive] = vInactive;

				// Add the new row (this should put it in sorted order, 
				// and therefore at the top of the list because we 
				// deliberately gave it the highest relevence)
				m_lstResource->AddRow(pNewRow);

				// Remember something's changed so we'll need to save at the end
				m_bNeedToSave = TRUE;
			} 
		}
	} NxCatchAll("CResourceOrderDlg::OnNewResourceBtn");
}

void CResourceOrderDlg::OnDeleteResourceBtn() 
{
	try {
		long nDeleteSel = m_lstResource->CurSel;
		if (nDeleteSel != -1) {

			long nResourceID = VarLong(m_lstResource->GetValue(nDeleteSel,0));

			//check permissions to delete
			if(nResourceID != -1 && !HasWritePermissionForResource(nResourceID))
				return;
			// (s.tullis 2014-12-12 08:41) - PLID 64440 
			if (ReturnsRecordsParam("Select ID FROM ScheduleMixRuleDetailsT WHERE ResourceID = { INT } ", nResourceID))
			{
				MsgBox( "You may not delete this resource because it is associated with a Scheduling Mix Rule template. ");
				return;
			}

			long nDelResourceID = VarLong(m_lstResource->Value[nDeleteSel][rofID]); // -1 means it was added but not saved to data yet so we don't need to (and can't) validate the ID
			if (/*nDelResourceID == -1 ||*/ ValidateDeleteResource(nDelResourceID)) { //TS: We need to warn them about deleting it even if it hasn't been saved yet, for consistency.  We certainly can validate the ID, it'll be 0, but that's fine.
				// Add the resource ID to the array of resources that should be deleted
				m_aryDeleteResources.Add(nDelResourceID);
				
				// And remove it from the listbox (in anticipation of it being deleted later)
				m_lstResource->RemoveRow(nDeleteSel);

				// Remember something's changed so we'll need to save at the end
				m_bNeedToSave = TRUE;
			}
		} else {
			MsgBox(MB_ICONINFORMATION|MB_OK, "Please select a resource before trying to delete");
		}
	} NxCatchAll("CResourceOrderDlg::OnDeleteResourceBtn");
}


void CResourceOrderDlg::OnEditingFinishingResourceList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		if (*pbCommit == FALSE) {
			return;
		}

		if(nRow == -1) {
			return;
		}

		switch(nCol) {
		case rofName:
			{
				CString strNewValue = VarString(pvarNewValue);
				strNewValue.TrimLeft(); strNewValue.TrimRight();
				
				CString strOldValue = VarString(varOldValue);
				strOldValue.TrimLeft(); strOldValue.TrimRight();

				//DRT 8/7/2007 - Because of 26961 below, we don't want to be warning people (or running queries) if nothing changed.  I can't find any
				//	good reason that we flag ourselves "changed" if old and new are the same.  So from here out, we won't. (this is a case sensitive operation)
				if(strNewValue == strOldValue) {
					//We want to continue (this allows them to press "enter" to save their editing, but we don't want anything to commit.  In reality this
					//	doesn't do much right now, because EditingFinished doesn't look at it for this column.
					*pbContinue = TRUE;
					*pbCommit = FALSE;
					return;
				}

				if(!ValidateResourceName(strNewValue, strOldValue)){
					*pbContinue = FALSE;
					*pbCommit = FALSE;
				}
				else{
					//DRT 8/7/2007 - PLID 26961 - If the resource is in use on any appointments, warn the user first.
					long nResourceID = VarLong(m_lstResource->GetValue(nRow, rofID), -1);
					//-1 is the indicator for "new"... if it's new, it clearly can't already exist.
					if(nResourceID > 0) {
						_RecordsetPtr prsCount = CreateParamRecordset("SELECT COUNT(*) AS Cnt FROM AppointmentResourceT WHERE ResourceID = {INT}", nResourceID);
						if(!prsCount->eof) {
							long nCnt = AdoFldLong(prsCount, "Cnt", 0);
							if(nCnt > 0) {
								CString strMsg;
								strMsg.Format("This resource is in use by %li appointments.  Are you sure you wish to continue renaming this resource?", nCnt);
								if(AfxMessageBox(strMsg, MB_YESNO) != IDYES) {
									//Cancel the rename, but set "continue" to true so that the field is reverted to its old state
									*pbContinue = TRUE;
									*pbCommit = FALSE;
									return;
								}
								//Otherwise, let the rename continue
							}
						}
					}


					// set pvarNewVale to the value we validated - strNewValue, the only difference between them 
					// is if pvarNewValue was trimmed
					_variant_t varNewValue;
					varNewValue = _bstr_t((LPCTSTR)strNewValue);
					(*pvarNewValue) = varNewValue.Detach();
					
					// Remember something's changed so we'll need to save at the end
					m_bNeedToSave = TRUE;
				}
			}

			break;

		case rofVisible:
			if (VarBool(*pvarNewValue) && VarBool(m_lstResource->Value[nRow][rofInactive])) {
				// PLID 6400 - If an item is inactive, we can't mark it as visible
				AfxMessageBox("You cannot mark an inactive resource as visible.");
				*pbCommit = FALSE;
			} else {
				// Remember something's changed so we'll need to save at the end
				m_bNeedToSave = TRUE;
			}
			break;			

		case rofProviderNames:
			// Remember something's changed so we'll need to save at the end
			m_bNeedToSave = TRUE;
			break;

		case rofInactive:
			// Remember something's changed so we'll need to save at the end
			m_bNeedToSave = TRUE;
			break;

		case rofDefaultInterval:
			// Remember something's changed so we'll need to save at the end.
			m_bNeedToSave = TRUE;
			break;

		case rofUserID:
			// Remember something's changed so we'll need to save at the end
			m_bNeedToSave = TRUE;
			break;

		case rofLocationID:
			// (z.manning 2009-09-10 14:27) - PLID 28659 - We need to mark as unsaved when changing
			// the location!
			m_bNeedToSave = TRUE;
			break;

		default:
			break;
		}
	} NxCatchAll("Error in OnEditingFinishingResourceList");
}

void CResourceOrderDlg::OnSelChosenViewList(long nRow) 
{
	if (m_nLastChosenRow == nRow)
		return;

	// Save if something's changed
	if (m_bNeedToSave) {
		long nMaxRelevence = UniquifyRelevences();
		if (nMaxRelevence < 0)
		{
			m_lstViews->CurSel = m_nLastChosenRow;
			MsgBox(MB_ICONINFORMATION|MB_OK, "Please check at least one active resource to be visible before selecting another view.");			
			return;
		}

		if (IDYES == MsgBox(MB_YESNO, "You have made changes to this resource view. Do you wish to save your changes? If you select 'No', they will be discarded."))
		{
			// Save
			if (!SaveResourceList(m_nLastChosenRow)) {
				m_lstViews->CurSel = m_nLastChosenRow;
				return;
			}
		}
	}

	if(nRow == -1) {
		//can't set to no row, that would be foolish!
		if(m_lstViews->GetRowCount() > 0) {
			//set the selection to the first row
			m_lstViews->PutCurSel(0);
			nRow = 0;
		}
	}
	// Load the new list, remembering that now this new list is our last view chosen
	LoadResourceList();
	m_nLastChosenRow = nRow;
}

void CResourceOrderDlg::OnEditviewBtn() 
{
	BOOL bEditable = FALSE;

	long nCurSel = m_lstViews->CurSel;
	if (m_lstViews->Value[nCurSel][0].vt == VT_I4) {
		bEditable = TRUE;
	} else {
		bEditable = FALSE;
	}

	CMenu mnu;
	mnu.m_hMenu = CreatePopupMenu();
	mnu.InsertMenu(0, MF_BYPOSITION, IDM_ADD, "&Add");
	mnu.InsertMenu(1, MF_BYPOSITION|(bEditable?MF_ENABLED:MF_DISABLED|MF_GRAYED), IDM_EDIT, "&Rename");
	mnu.InsertMenu(2, MF_BYPOSITION|(bEditable?MF_ENABLED:MF_DISABLED|MF_GRAYED), IDM_DELETE, "&Delete");
	
	CRect rc;
	CWnd *pWnd = GetDlgItem(IDC_EDITVIEW_BTN);
	if (pWnd) {
		pWnd->GetWindowRect(&rc);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
	} else {
		CPoint pt;
		GetCursorPos(&pt);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
	}
}

void CResourceOrderDlg::OnViewAdd()
{
	//DRT 4/30/03 - Requires sptCreate permission to add views
	if (!CheckCurrentUserPermissions(bioSchedResourceOrder, sptCreate)) 
		return;

	try {
		CString strViewName;
		int nResult = IDCANCEL;
		BOOL bKeepAsking = TRUE;
		while (bKeepAsking) {
			bKeepAsking = FALSE;
			nResult = InputBoxLimited(this, "Enter a name for the new view", strViewName, "",255,false,false,NULL);
			if (nResult == IDOK) {
				strViewName.TrimLeft(); strViewName.TrimRight();
				// Make sure it's not a duplicate
				if (m_lstViews->FindByColumn(1, _bstr_t(strViewName), 0, FALSE) != -1) {
					MsgBox("The name you have selected is already in use by an existing view.  Please enter a different name.");
					bKeepAsking = TRUE;
				}
			}
		}
		//DRT - if you make a change to a view then add a new one w/o switching elsewhere first
		//we need to save the current m_nLastChosenRow before adding the new stuff
		// Save if something's changed
		if (m_bNeedToSave) {
			// Save
			if (!SaveResourceList(m_nLastChosenRow))
				return;
		}

		if (nResult == IDOK) {
			IRowSettingsPtr pRow = m_lstViews->Row[-1];
			pRow->Value[0] = (long)-1;
			pRow->Value[1] = _bstr_t(strViewName);
			long nIndex = m_lstViews->AddRow(pRow);
			if (nIndex >= 0) {
				
				//DRT 7/12/02 - what this was doing, was setting the last chosen row to the current selection, but since we've not selected the new row yet
				//it was setting the last chosen row to the chosen-2-times ago row, and if you switched to another resource view before hitting close with
				//this new one selected, it will be lost (because it saved the wrong one)
					//m_nLastChosenRow = m_lstViews->CurSel;
				m_lstViews->CurSel = nIndex;
				// (b.cardillo 2003-06-24 17:17) - Moved this to after the PutCurSel, and changed it from nIndex to 
				// GetCurSel() just in case for some reason the row index is no longer valid.
				m_nLastChosenRow = m_lstViews->GetCurSel();
				LoadResourceList();
			}

			//TES 7/26/2010 - PLID 39445 - Set the default to <No Default>
			m_pDefaultLocation->SetSelByColumn(0, (long)-2);

			// Remember something's changed so we'll need to save at the end
			m_bNeedToSave = TRUE;
		}
	} NxCatchAll("CResourceOrderDlg::OnViewAdd");
}

void CResourceOrderDlg::OnViewEdit()
{
	//DRT 4/30/03 - Requires sptCreate permission to rename views
	if (!CheckCurrentUserPermissions(bioSchedResourceOrder, sptCreate)) 
		return;

	try {
		long nCurSel = m_lstViews->CurSel;
		if (nCurSel != -1) {
			_variant_t varID = m_lstViews->Value[nCurSel][0];
			if (varID.vt == VT_I4) {
				CString strViewName = VarString(m_lstViews->Value[nCurSel][1]);
				
				// Try to change the name
				int nResult = IDCANCEL;
				BOOL bKeepAsking = TRUE;
				while (bKeepAsking) {
					bKeepAsking = FALSE;
					nResult = InputBoxLimited(this, "Enter a new name for the view", strViewName, "",255,false,false,NULL);
					if (nResult == IDOK) {
						strViewName.TrimLeft(); strViewName.TrimRight();
						// Make sure it's not a duplicate
						// Because we start at nCurSel+1, if we find it at nCurSel, then nCurSel must be the only one, meaning that they didn't change the name.
						long nRow = m_lstViews->FindByColumn(1, _bstr_t(strViewName), nCurSel+1, FALSE);
						if (nRow != -1) {
							if (nRow != nCurSel) {
								MsgBox("The name you have selected is already in use by another view.  Please enter a different name.");
								bKeepAsking = TRUE;
							}
						}
					}
				}

				if (nResult == IDOK) {
					// Write back to the datalist
					m_lstViews->Value[nCurSel][1] = _bstr_t(strViewName);
					// Remember something's changed so we'll need to save at the end
					m_bNeedToSave = TRUE;
				}
			} else {
				MsgBox("You may not modify built-in views.  Please choose a custom view to make changes.");
			}
		}
	} NxCatchAll("CResourceOrderDlg::OnViewEdit");
}

void CResourceOrderDlg::OnViewDelete()
{
	//DRT 4/30/03 - Requires sptDelete permission to remove views
	if (!CheckCurrentUserPermissions(bioSchedResourceOrder, sptDelete)) 
		return;

	try {
		long nCurSel = m_lstViews->CurSel;
		if (nCurSel != -1) {
			_variant_t varID = m_lstViews->Value[nCurSel][0];
			if (varID.vt == VT_I4) {
				// Confirm
				int nResult = MsgBox(MB_YESNO|MB_ICONQUESTION, 
					"Are you sure you want to permanently delete the '%s' custom view?",
					VarString(m_lstViews->Value[nCurSel][1]));

				if (nResult == IDYES) {
					// Store the ID of the row to delete
					long nID = VarLong(varID);
					if (nID != -1) {
						m_aryDeleteViews.Add(nID);
					}
					// Delete the row from the datalist
					m_lstViews->RemoveRow(nCurSel);

					// Refresh the list
					m_bNeedToSave = FALSE;
					m_nLastChosenRow = -1;
					OnSelChosenViewList(m_lstViews->CurSel);

					// Remember something's changed so we'll need to save at the end
					m_bNeedToSave = TRUE;
				}
			} else {
				MsgBox("You may not delete built-in views.  Please choose a custom view to make changes.");
			}
		}
	} NxCatchAll("CResourceOrderDlg::OnViewDelete");
}

void CResourceOrderDlg::OnBtnProviderLinking() 
{
	if (m_lstResource->CurSel == -1)
	{
		MsgBox("Please select a resource to link providers with first.");
		return;
	}

	try {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "ProvidersT");

		// Pre-select the already-existing providers
		CString strIDs = VarString(m_lstResource->GetValue(m_lstResource->CurSel, rofProviderIDs), "");
		while (strIDs.GetLength())
		{
			CString strID;

			if (-1 != strIDs.Find(' '))
			{
				strID = strIDs.Left(strIDs.Find(' '));
				strIDs = strIDs.Right( strIDs.GetLength() - strIDs.Find(' ') - 1);
			}
			else
			{
				strID = strIDs;
				strIDs.Empty();
			}
			dlg.PreSelect(atoi(strID));
		}


		// Let the user choose which providers to link
		if (IDCANCEL == dlg.Open("ProvidersT INNER JOIN PersonT ON PersonT.ID = ProvidersT.PersonID", "", "PersonID", "[Last] + ', ' + [First] + ' ' + [Middle] ", "Please select the providers to associate with the resource."))
			return;

		// Assign the providers to the resource in the list
		m_lstResource->PutValue(m_lstResource->CurSel, rofProviderNames, _bstr_t(dlg.GetMultiSelectString()));
		m_lstResource->PutValue(m_lstResource->CurSel, rofProviderIDs, _bstr_t(dlg.GetMultiSelectIDString()));
		
	}
	NxCatchAll("Error in resource-provider linking");
}

void CResourceOrderDlg::OnEditingStartingResourceList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	if(nRow == -1) {
		return;
	}

	long nResourceID = VarLong(m_lstResource->GetValue(nRow,0));	

	switch(nCol) {

	case rofName:		
		//if they can't write to any, or to this, OR they don't enter the right password, then they can't edit
		if(nResourceID != -1 && !HasWritePermissionForResource(nResourceID)) {		
			*pbContinue = false;	//don't commit these changes
		}
		break;

	case rofVisible:
		//They must have permission to bioSchedIndivResources - sptRead ON THIS RESOURCE
		if(nResourceID != -1 && !CheckCurrentUserPermissions(bioSchedIndivResources,sptRead,TRUE,nResourceID)) {
			*pbContinue = false;	//don't commit these changes
		}
		break;

	case rofProviderNames:
		//if they can't write to any, or to this, OR they don't enter the right password, then they can't edit
		if(nResourceID != -1 && !HasWritePermissionForResource(nResourceID)) {
			*pbContinue = false;	//don't commit these changes
		}
		break;

	case rofInactive:
		if (nResourceID != -1 && !(GetCurrentUserPermissions(bioSchedResourceOrder) & sptWrite)) {
			*pbContinue = false;	//don't commit these changes
		}
		break;

	default:
		break;
	}
}

BOOL CResourceOrderDlg::HasWritePermissionForResource(long nResourceID) {

	//They must have permission to bioSchedResourceOrder - sptWrite, 
	//and they must also have permission to bioSchedIndivResources - sptWrite ON THIS RESOURCE,
	//if we want to return TRUE

	BOOL bWriteAll = (GetCurrentUserPermissions(bioSchedResourceOrder) & sptWrite);
	BOOL bWriteAllWithPass = (GetCurrentUserPermissions(bioSchedResourceOrder) & sptWriteWithPass);
	BOOL bWriteThis = (GetCurrentUserPermissions(bioSchedIndivResources, TRUE, nResourceID) & sptWrite);
	BOOL bWriteThisWithPass = (GetCurrentUserPermissions(bioSchedIndivResources, TRUE, nResourceID) & sptWriteWithPass);

	if((!bWriteAll || !bWriteThis) && !((bWriteThisWithPass || bWriteAllWithPass) && CheckCurrentUserPassword())) {
		// (a.walling 2010-08-02 11:01) - PLID 39182 - Consolidating all these copies of "You do not have permission to access this function"
		// messageboxes with PermissionsFailedMessageBox
		PermissionsFailedMessageBox();
		return FALSE;
	}

	return TRUE;
}

bool CResourceOrderDlg::ValidateResourceName(const IN CString &strNewName, const IN CString &strOldName /* = ""*/)
{
	// We can't accept an empty string
	if (strNewName.IsEmpty()) {
		AfxMessageBox("You cannot save an empty string.  Please enter a name for this resource.");
		return false;
	}
	
	// if the new name and the old name are the same, then the name is valid
	if(strNewName.CompareNoCase(strOldName) == 0){
		return true;
	}

	try{
		//check to make sure the group name isn't duplicated
		long pCurRowEnum = m_lstResource->GetFirstRowEnum();
		// loop through the resources
		while(pCurRowEnum != 0){
			IRowSettingsPtr pRow;
			{
				IDispatch *lpDisp;
				m_lstResource->GetNextRowEnum(&pCurRowEnum, &lpDisp);
				pRow = lpDisp;
				lpDisp->Release();
				lpDisp = NULL;
			}

			ASSERT(pRow != NULL);
			_variant_t var = pRow->GetValue(2);
			
			CString strResource;
			strResource = VarString(var);
					
			// if the name was found, tell the user why the name is invalid
			if(strNewName.CompareNoCase(strResource) == 0){
				AfxMessageBox("This resource already exists, please enter a name that is unique.");
				return false;
			}
		}
	}NxCatchAll("CResourceOrderDlg::ValidateNewResourceName     Error validating new resource name");

	return true;
}

void CResourceOrderDlg::OnCopyResourceSettings()
{
	// (c.haag 2010-05-04 10:29) - PLID 37263 - Remember the current list selection for when we
	// process the pop-up handler. Though it's unlikely the list selection will change, we must keep
	// the behavior consistent to how it was before the introduction of the pop-up handler.
	m_nCopyResourcesSrcViewIndex = m_lstViews->CurSel;
	if(m_nCopyResourcesSrcViewIndex == -1)
		return;
	if (!CheckCurrentUserPermissions(bioSchedCopyResOrder, sptDynamic0)) 
		return;

	// (c.haag 2010-05-04 10:29) - PLID 37263 - Raise a pop-up menu that allows users to
	// copy the settings to all users.
	CMenu pMenu;
	pMenu.CreatePopupMenu();
	pMenu.InsertMenu(0, MF_BYPOSITION, ID_COPY_RES_TO_MULTIPLE, "Copy to &Specific Users...");
	pMenu.InsertMenu(1, MF_BYPOSITION, ID_COPY_RES_TO_ALL, "Copy to &All Users");

	CPoint pt;
	GetCursorPos(&pt);
	pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);	
}

void CResourceOrderDlg::OnBtnResourceappttypes() 
{
	//DRT 4/16/2004 - Check license first
	if(!IsSpa(TRUE)) {
		MsgBox("You must purchase the NexSpa License to use this feature.");
		return;
	}


	if (CheckCurrentUserPermissions(bioAdminScheduler, sptView|sptRead|sptWrite))
	{
		CResourcePurpTypeDlg dlg(this);
		dlg.SetResults(m_apSavedResPurpTypeCombinations, m_apChanged);
		if (IDOK == dlg.DoModal())
		{
			dlg.GetResults(m_apSavedResPurpTypeCombinations, m_apChanged);
			m_bSaveResourcePurpTypes = TRUE;
		}
	}
}

BOOL CResourceOrderDlg::SaveResourcePurpTypes()
{
	try {
		CString strSQL;

		// If this is true, it means the user never clicked on OK at
		// any point in time, so there is nothing to be saved.
		if (!m_apSavedResPurpTypeCombinations.GetSize())
			return TRUE;

		for (long i=0; i < m_apChanged.GetSize(); i++)
		{
			CResourcePurpTypeDlg::CCombination* p = m_apChanged.GetAt(i);
			CString str;
			if (p->GetSelected())
			{
				str.Format("DELETE FROM ResourcePurposeTypeT WHERE ResourceID = %d AND AptTypeID = %d AND AptPurposeID = %d;INSERT INTO ResourcePurposeTypeT (ResourceID, AptTypeID, AptPurposeID) VALUES (%d, %d, %d);",
					p->GetResourceID(), p->GetAptTypeID(), p->GetAptPurposeID(),
					p->GetResourceID(), p->GetAptTypeID(), p->GetAptPurposeID());
			}
			else
			{
				str.Format("DELETE FROM ResourcePurposeTypeT WHERE ResourceID = %d AND AptTypeID = %d AND AptPurposeID = %d;",
					p->GetResourceID(), p->GetAptTypeID(), p->GetAptPurposeID());
			}
			strSQL += str;
		}
		if (!strSQL.IsEmpty()) {
			// (j.jones 2010-04-20 09:16) - PLID 38273 - converted to ExecuteSqlStd
			ExecuteSqlStd(strSQL);
		}
		// (c.haag 2004-04-02 09:10) - Send table checkers to make sure
		// the resentry dialog requeries itself in case types or purposes
		// are filtered out.
		CClient::RefreshTable(NetUtils::AptTypeT);
		CClient::RefreshTable(NetUtils::AptPurposeT);
		return TRUE;
	}
	NxCatchAll("Error in CResourcePurpTypeDlg::Save()"); 
	return FALSE;
}

void CResourceOrderDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	for (long i=0; i < m_apSavedResPurpTypeCombinations.GetSize(); i++)
	{
		delete m_apSavedResPurpTypeCombinations.GetAt(i);
	}
	m_apSavedResPurpTypeCombinations.RemoveAll();	
}

void CResourceOrderDlg::OnRequeryFinishedResourceList(short nFlags) 
{
	// Remove any deleted resources
	//
	// (c.haag 2006-05-08 11:16) - PLID 20470 - These are now filtered out by the where clause
	// of the list query
	/*
	long nDelResourceCount = m_aryDeleteResources.GetSize();
	for (long i=0; i<nDelResourceCount; i++) {
		long nDelResourceID = m_aryDeleteResources[i];
		if (nDelResourceID > 0) {
			long nDelResourceIndex = m_lstResource->FindByColumn(rofID, nDelResourceID, 0, FALSE);
			if (nDelResourceIndex != -1) {
				m_lstResource->RemoveRow(nDelResourceIndex);
			}
		}
	}*/

	// Make sure the resources have unique relevences to start
	UniquifyRelevences();

	// Default to the first item
	m_lstResource->CurSel = 0;

	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDC_NEW_RESOURCE_BTN)->EnableWindow(TRUE);
	GetDlgItem(IDC_DELETE_RESOURCE_BTN)->EnableWindow(TRUE);
	GetDlgItem(IDC_MOVE_UP_BTN)->EnableWindow(FALSE);
	GetDlgItem(IDC_MOVE_DOWN_BTN)->EnableWindow(TRUE);
}

// (c.haag 2010-05-04 10:29) - PLID 37263 - Called to copy resource settings to multiple users
// (moved from OnCopyResourceSettings)
void CResourceOrderDlg::OnCopyResourceSettingsToMultipleUsers()
{
	try {
		if (IDNO == MsgBox(MB_YESNO, "The current standard view settings will be saved first. Would you like to continue?"))
			return;

		CWaitCursor wc;
		if (!SaveResourceList(m_nCopyResourcesSrcViewIndex))
			return;
		
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "UsersT");
		CString strWhere, str;
		// (c.haag 2010-05-04 11:09) - PLID 38486 - Filter out inactive users
		strWhere.Format("PersonT.ID <> %d AND PersonT.Archived = 0", GetCurrentUserID());
		if (IDOK == dlg.Open("UsersT INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID", strWhere, "PersonT.ID", "Last + ', ' + First + ' ' + Middle", "Select Users to Copy Standard View Settings for", 1))
		{
			str = dlg.GetMultiSelectIDString();
			str.Replace(" ", ", ");
			if (!str.IsEmpty())
			{
				ExecuteSql("DELETE FROM UserResourcesT WHERE UserID IN (%s);"
					"INSERT INTO UserResourcesT (UserID, ResourceID, Relevence, DefaultInterval) "
					"SELECT personid, resourceid, relevence, defaultinterval from userresourcest, userst where userresourcest.userid = %d and userst.personid in (%s)", str, GetCurrentUserID(), str);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-05-04 10:29) - PLID 37263 - Called to copy resource settings to all users
void CResourceOrderDlg::OnCopyResourceSettingsToAllUsers()
{
	try {
		if (IDNO == MsgBox(MB_YESNO, "The current standard view settings will be saved first. Would you like to continue?"))
			return;

		CWaitCursor wc;
		if (!SaveResourceList(m_nCopyResourcesSrcViewIndex))
			return;

		CString strSqlBatch = BeginSqlBatch();
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM UserResourcesT WHERE UserID IN (SELECT ID FROM UsersT INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID WHERE Archived= 0 AND PersonT.ID <> %d);", GetCurrentUserID());
		AddStatementToSqlBatch(strSqlBatch,	"INSERT INTO UserResourcesT (UserID, ResourceID, Relevence, DefaultInterval) "
					"SELECT personid, resourceid, relevence, defaultinterval from userresourcest, userst where userresourcest.userid = %d and userst.personid in (SELECT ID FROM UsersT INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID WHERE Archived= 0 AND PersonT.ID <> %d)", GetCurrentUserID(), GetCurrentUserID());
		ExecuteSqlBatch(strSqlBatch);
	}
	NxCatchAll(__FUNCTION__);
}

void CResourceOrderDlg::OnSelChangingDefaultViewLocation(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			//TES 7/26/2010 - PLID 39445 - Don't let them select nothing, change it back to the old row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

void CResourceOrderDlg::OnSelChosenDefaultViewLocation(LPDISPATCH lpRow)
{
	try {
		//TES 7/26/2010 - PLID 39445 - Remember that we need to save the new selection
		m_bNeedToSave = TRUE;
	}NxCatchAll(__FUNCTION__);
}

void CResourceOrderDlg::OnTrySetSelFinishedDefaultViewLocation(long nRowEnum, long nFlags)
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure && m_nPendingLocationID > 0) {
			//TES 7/26/2010 - PLID 39445 - maybe it's inactive?
			_RecordsetPtr rsLoc = CreateParamRecordset("SELECT Name FROM LocationsT "
				"WHERE ID = {INT}", m_nPendingLocationID);
			if(!rsLoc->eof) {
				m_pDefaultLocation->PutComboBoxText(_bstr_t(AdoFldString(rsLoc, "Name", "")));
			}
			else 
				m_pDefaultLocation->PutCurSel(NULL);
		}
		else {
			m_nPendingLocationID = -2;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-04-11 11:22) - PLID 44174 - added ability to merge resources
void CResourceOrderDlg::OnBtnMergeResources()
{
	try {

		if(!CheckCurrentUserPermissions(bioMergeSchedulerResources, sptDynamic0)) {
			return;
		}

		if (m_bNeedToSave) {
			if (IDNO == MsgBox(MB_YESNO, "You have made changes to this resource view. These changes must be saved before merging resources.\n"
				"Do you wish to save your changes?")) {
				return;
			}

			long nCurSel = m_lstViews->CurSel;
			if(nCurSel != -1) {
				if(!SaveResourceList(m_nLastChosenRow)) {
					return;
				}
			}
		}

		CListMergeDlg dlg(this);
		dlg.m_eListType = mltSchedulerResources;
		dlg.DoModal();

		//if something changed, reload to reflect changes
		if(dlg.m_bCombined) {
			LoadResourceList();
		}

	}NxCatchAll(__FUNCTION__);
}