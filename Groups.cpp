// Groups.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "Groups.h"
#include "FilterEditDlg.h"
#include "PracProps.h"
#include "GlobalUtils.h"
#include "NxException.h"
#include "Filter.h"
#include "AuditTrail.h"
#include "ApptSelector.h"

#include "InternationalUtils.h"
#include "GlobalDrawingUtils.h"

//#include "ListItems.h"
//#include "ListItem.h"

#include "GlobalDataUtils.h"
using namespace ADODB;
using namespace NXDATALISTLib;

#include "LetterWriting.h"
#include "nxmessagedef.h"

//(c.copits 2011-09-13) PLID 43485 - "Go to Patient" right-click option in letter writing
#define	ID_PATIENT_UNSELECTED_GOTO_PATIENT	30333
#define	ID_PATIENT_SELECTED_GOTO_PATIENT	30334
#define ID_APPT_UNSELECTED_GOTO_PATIENT		30335
#define ID_APPT_SELECTED_GOTO_PATIENT		30336

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



enum LetterWritingColumns {

	lwcID = 0,
	lwcUserDefinedID,
	lwcLastName,
	lwcFirstName,
	lwcCompany,
	lwcAddress,
};

// (a.wetta 2007-02-20 17:04) - PLID 24174 - Added the ApptStart column to the appointment datalist
enum LetterWritingApptColumns {

	lwacApptID = 0,
	lwacPersonID,
	lwacUserDefinedID,
	lwacLastName,
	lwacFirstName,
	lwacApptDate,
	lwacApptStart,
	lwacApptType,
	lwacApptPurpose,
	lwacApptResource,
	lwacApptLocation,
	lwacApptStatus,
};

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_APPOINTMENT_SELECTOR_NEW		46000
#define ID_FILTER_EDITOR_NEW			46001
#define ID_APPOINTMENT_SELECTOR_EDIT	46002
#define ID_FILTER_EDITOR_EDIT			46003

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGroups dialog
CGroups::CGroups(CWnd* pParent)
: CNxDialog(CGroups::IDD, pParent), m_groupChecker(NetUtils::Groups), m_filterChecker(NetUtils::FiltersT)
{
	//{{AFX_DATA_INIT(CGroups)
	//}}AFX_DATA_INIT
	m_nCurGroupID = GROUP_ID_NEW;
	m_bGroupModified = false;

	m_nCurFilterID = FILTER_ID_NEW;
	m_strUnsavedFilterString.Empty();

	m_nCurSelectionId = -25;

	m_strGroupList = "-1";

	m_nStyle = -1;

	m_bCurrentFilterGroupNeedsRefresh = false;

	// (j.jones 2016-04-15 09:16) - NX-100214 - no longer need a brush
	//m_brush.CreateSolidBrush(PaletteColor(0xCAE4FF));

	m_bAppointTabLoaded = FALSE;
}

void CGroups::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGroups)
	DDX_Control(pDX, IDC_REMEMBER_COLUMNS, m_btnRememberColumns);
	DDX_Control(pDX, IDC_REMOVE_ALL, m_removeAllBtn);
	DDX_Control(pDX, IDC_REMOVE, m_removeBtn);
	DDX_Control(pDX, IDC_ADD_ALL, m_addAllBtn);
	DDX_Control(pDX, IDC_ADD, m_addBtn);
	DDX_Control(pDX, IDC_LABEL, m_nxstaticLabel);
	DDX_Control(pDX, IDC_AVAILTEXT, m_nxstaticAvailtext);
	DDX_Control(pDX, IDC_LABEL3, m_nxstaticLabel3);
	DDX_Control(pDX, IDC_AVAILCOUNT, m_nxstaticAvailcount);
	DDX_Control(pDX, IDC_SELCOUNT, m_nxstaticSelcount);
	DDX_Control(pDX, IDC_REPOSITION, m_nxstaticReposition);
	DDX_Control(pDX, IDC_NEW_FILTER_BTN, m_btnNewFilter);
	DDX_Control(pDX, IDC_EDIT_FILTERS, m_btnEditFilter);
	DDX_Control(pDX, IDC_SAVE_FILTERS, m_btnSaveFilter);
	DDX_Control(pDX, IDC_DELETEFILTER, m_btnDeleteFilter);
	DDX_Control(pDX, IDC_NEW_GROUP_BTN, m_btnNewGroup);
	DDX_Control(pDX, IDC_SAVEGROUP, m_btnSaveGroup);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGroups, CNxDialog)
	//{{AFX_MSG_MAP(CGroups)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_ADD_ALL, OnAddAll)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_REMOVE_ALL, OnRemoveAll)
	ON_BN_CLICKED(IDC_EDIT_FILTERS, OnEditFilters)
	ON_BN_CLICKED(IDC_SAVEGROUP, OnSaveGroup)
	ON_BN_CLICKED(IDC_DELETE, OnDeleteGroup)
	ON_BN_CLICKED(IDC_DELETEFILTER, OnDeleteFilter)
	ON_BN_CLICKED(IDC_NEW_GROUP_BTN, OnNewGroupBtn)
	ON_BN_CLICKED(IDC_NEW_FILTER_BTN, OnNewFilterBtn)
	ON_BN_CLICKED(IDC_SAVE_FILTERS, OnSaveFiltersBtn)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_REMEMBER_COLUMNS, OnRememberColumns)
	ON_WM_DESTROY()
	ON_MESSAGE(NXM_INVALID_FILTER, OnInvalidFilter)
	ON_COMMAND(ID_APPOINTMENT_SELECTOR_NEW, OnApptSelectorNew)
	ON_COMMAND(ID_FILTER_EDITOR_NEW, OnFilterEditorNew)
	ON_COMMAND(ID_APPOINTMENT_SELECTOR_EDIT, OnApptSelectorEdit)
	ON_COMMAND(ID_FILTER_EDITOR_EDIT, OnFilterEditorEdit)
	//(c.copits 2011-09-13) PLID 43485 - "Go to Patient" right-click option in letter writing
	ON_BN_CLICKED(ID_PATIENT_UNSELECTED_GOTO_PATIENT, OnGoToPatientUnselectedList)
	ON_BN_CLICKED(ID_PATIENT_SELECTED_GOTO_PATIENT, OnGoToPatientSelectedList)
	ON_BN_CLICKED(ID_APPT_UNSELECTED_GOTO_PATIENT, OnGoToApptUnselectedList)
	ON_BN_CLICKED(ID_APPT_SELECTED_GOTO_PATIENT, OnGoToApptSelectedList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CGroups, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CGroups)
	ON_EVENT(CGroups, IDC_SELECTED, 3 /* DblClickCell */, OnDblClickCellSelected, VTS_I4 VTS_I2)
	ON_EVENT(CGroups, IDC_UNSELECTED, 3 /* DblClickCell */, OnDblClickCellUnselected, VTS_I4 VTS_I2)
	ON_EVENT(CGroups, IDC_SELECTED, 18 /* RequeryFinished */, OnRequeryFinishedSelected, VTS_I2)
	ON_EVENT(CGroups, IDC_UNSELECTED, 18 /* RequeryFinished */, OnRequeryFinishedUnselected, VTS_I2)
	ON_EVENT(CGroups, IDC_PERSON_TYPE_COMBO, 16 /* SelChosen */, OnSelChosenPersonTypeCombo, VTS_I4)
	ON_EVENT(CGroups, IDC_SELECTED, 22 /* ColumnSizingFinished */, OnColumnSizingFinishedSelected, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CGroups, IDC_UNSELECTED, 22 /* ColumnSizingFinished */, OnColumnSizingFinishedUnselected, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CGroups, IDC_APPT_SELECTED, 22 /* ColumnSizingFinished */, OnColumnSizingFinishedSelected, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CGroups, IDC_APPT_UNSELECTED, 22 /* ColumnSizingFinished */, OnColumnSizingFinishedUnselected, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CGroups, IDC_FILTER_TYPE_TAB, 1 /* SelectTab */, OnSelectTab, VTS_I2 VTS_I2)
	ON_EVENT(CGroups, IDC_APPT_SELECTED, 3 /* DblClickCell */, OnDblClickCellSelected, VTS_I4 VTS_I2)
	ON_EVENT(CGroups, IDC_APPT_UNSELECTED, 3 /* DblClickCell */, OnDblClickCellUnselected, VTS_I4 VTS_I2)
	ON_EVENT(CGroups, IDC_APPT_SELECTED, 18 /* RequeryFinished */, OnRequeryFinishedApptSelected, VTS_I2)
	ON_EVENT(CGroups, IDC_APPT_UNSELECTED, 18 /* RequeryFinished */, OnRequeryFinishedApptUnselected, VTS_I2)
	ON_EVENT(CGroups, IDC_EXISTING_FILTER_LIST, 16 /* SelChosen */, OnSelChosenExistingFilterList, VTS_DISPATCH)
	ON_EVENT(CGroups, IDC_EXISTING_GROUP_LIST, 16 /* SelChosen */, OnSelChosenExistingGroupList, VTS_DISPATCH)
	ON_EVENT(CGroups, IDC_EXISTING_FILTER_LIST, 1 /* SelChanging */, OnSelChangingExistingFilterList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CGroups, IDC_EXISTING_GROUP_LIST, 1 /* SelChanging */, OnSelChangingExistingGroupList, VTS_DISPATCH VTS_PDISPATCH)
	//(c.copits 2011-09-13) PLID 43485 - "Go to Patient" right-click option in letter writing
	ON_EVENT(CGroups, IDC_UNSELECTED, 6 /* RButtonDown */, OnRButtonDownPatientUnselectedList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CGroups, IDC_SELECTED, 6 /* RButtonDown */, OnRButtonDownPatientSelectedList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CGroups, IDC_APPT_UNSELECTED, 6 /* RButtonDown */, OnRButtonDownApptUnselectedList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CGroups, IDC_APPT_SELECTED, 6 /* RButtonDown */, OnRButtonDownApptSelectedList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGroups message handlers

void CGroups::UpdateToolBars(long nStyle)
{
	CWnd *p = GetParent();
	if (p->GetSafeHwnd()) {
		CWnd *pGrandParent = p->GetParent();
		if (pGrandParent->GetSafeHwnd() && pGrandParent->IsKindOf(RUNTIME_CLASS(CNxView))) {
			CNxView *pModule = (CNxView *)pGrandParent;
			if (nStyle & fbjPatient) {
				pModule->m_bPatientsToolBar = true;
				pModule->m_bContactsToolBar = false;
			} else if (nStyle & (fbjContact|fbjProvider|fbjRefPhys|fbjSupplier|fbjUser|fbjRefSource)) {
				pModule->m_bPatientsToolBar = false;
				pModule->m_bContactsToolBar = true;
			}
		
			pModule->ShowToolBars();
		}
	}
}

void CGroups::OnSelChosenPersonTypeCombo(long nRow) 
{
	// Figure out what kinds of persons we care about
	try {
		//First, if it's -1, we'll just pretend they clicked on the first row.
		if(nRow == -1) {
			m_dlTypeOfPerson->SetSelByColumn(0, (long)fbjPatient);
			nRow = 0;
		}
		//Now, proceed as normal.

		// Prepare the appropriate values for setting the state of the group dialog
		long nBaseJoinFlags = m_dlTypeOfPerson->Value[nRow][0];
		m_nCurFilterID = VarLong(m_pFilterList->CurSel->GetValue(0));
		// Re-Initialize the groupEditor (just so it can reinit the filter)
		if (DoInitDialog(nBaseJoinFlags, m_nCurFilterID)) {
			// And show different toolbars based on what makes most sense
			UpdateToolBars(nBaseJoinFlags);
		}
	} NxCatchAll("CGroups::OnSelChosenPersonTypeCombo");
}

#define INSERT_PERSONTYPE_ROW(rowindex, type, text) { pRow = m_dlTypeOfPerson->Row[-1]; pRow->Value[0] = \
			(long)type; pRow->Value[1] = text; m_dlTypeOfPerson->InsertRow(pRow, rowindex); }

BOOL CGroups::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	//Attach to the NxTab
	CWnd *pWnd = GetDlgItem(IDC_FILTER_TYPE_TAB);
	if (pWnd)
		m_tab = pWnd->GetControlUnknown();
	else m_tab = NULL;

	m_tab->Size = 2;
	m_tab->TabWidth = 2;
	m_tab->PutLabel(0,"Person Based Merging");
	m_tab->PutLabel(1,"Appointment Based Merging");
	m_tab->CurSel = 0;

	//DRT 4/18/2008 - PLID 29711 - Set the text color of the tabs.
	// (j.jones 2016-04-18 17:46) - NX-100214 - the NxTab control now defaults
	// to the value of CNexTechDialog::GetDefaultNxTabTextColor()

	try {
		// Fill the TypeOfPerson datalist
		m_dlTypeOfPerson = BindNxDataListCtrl(IDC_PERSON_TYPE_COMBO, false);
		IRowSettingsPtr pRow;
		INSERT_PERSONTYPE_ROW(0, fbjPatient, "Patients and Prospects");
		INSERT_PERSONTYPE_ROW(1, fbjProvider, "Providers");
		INSERT_PERSONTYPE_ROW(2, fbjRefSource, "Referral Sources");
		INSERT_PERSONTYPE_ROW(3, fbjRefPhys, "Referring Physicians");
		INSERT_PERSONTYPE_ROW(4, fbjInsParty, "Insured Parties");
		INSERT_PERSONTYPE_ROW(5, fbjInsCo, "Insurance Companies");
		INSERT_PERSONTYPE_ROW(6, fbjSupplier, "Suppliers");
		INSERT_PERSONTYPE_ROW(7, fbjUser, "Users");
		INSERT_PERSONTYPE_ROW(8, fbjContact, "Other Contacts");
		m_dlTypeOfPerson->SetSelByColumn(0, (long)fbjPatient);
	} NxCatchAll("CLetterWriting::OnInitDialog TypeOfPerson");

	try {
		// Bind to the datalists
		m_selected = BindNxDataListCtrl(IDC_SELECTED, false);
		m_unselected = BindNxDataListCtrl(IDC_UNSELECTED, false);
		m_apptSelected = BindNxDataListCtrl(IDC_APPT_SELECTED, false);
		m_apptUnselected = BindNxDataListCtrl(IDC_APPT_UNSELECTED, false);

		m_pFilterList = BindNxDataList2Ctrl(IDC_EXISTING_FILTER_LIST, false);
		m_pGroupList = BindNxDataList2Ctrl(IDC_EXISTING_GROUP_LIST, false);

	} NxCatchAll("CGroups::OnInitDialog")

	OnSelectTab(0,-1);

	m_addBtn.AutoSet(NXB_RIGHT);
	m_addAllBtn.AutoSet(NXB_RRIGHT);
	m_removeBtn.AutoSet(NXB_LEFT);
	m_removeAllBtn.AutoSet(NXB_LLEFT);
	// (z.manning, 04/25/2008) - PLID 29795 - Set more button styles
	m_btnNewFilter.AutoSet(NXB_NEW);
	m_btnEditFilter.AutoSet(NXB_MODIFY);
	m_btnSaveFilter.AutoSet(NXB_MODIFY);
	m_btnDeleteFilter.AutoSet(NXB_DELETE);
	m_btnNewGroup.AutoSet(NXB_NEW);
	m_btnSaveGroup.AutoSet(NXB_MODIFY);
	m_btnDelete.AutoSet(NXB_DELETE);

	// We want to do this AFTER other messages have been processed
	PostMessage(NXM_GROUP_RESIZECOLUMNS);

	DoInitDialog(fbjDefault, GetFilterIdLast(), GetGroupIdLast());

	SetTimer(NXT_CHECK_CURRENT_PATIENT, 200, NULL);

	//this should be handled by the OnRememberColumns
	//SetSelectedColumnSizes();
	//SetUnselectedColumnSizes();

	if(GetRemotePropertyInt("RememberLWColumns", 0, 0, GetCurrentUserName(), true) == 1) {
		CheckDlgButton(IDC_REMEMBER_COLUMNS, TRUE);
	}
	else {
		CheckDlgButton(IDC_REMEMBER_COLUMNS, FALSE);
	}

	OnRememberColumns();

	// (z.manning 2013-07-17 11:33) - PLID 57609 - No point in doing this yet as the appt datalist isn't even visible.
	//LoadAppointmentDataList();

	return TRUE;
}

int CGroups::SetControlPositions()
{
	//TES 2/22/2007 - PLID 20642 - Unnecessary now that this is a datalist.
	/*// (a.wetta 2007-01-19 15:20) - PLID 24347 - While resize the filter list combo would flash, so we'll hide it
	// until we're done resizing
	GetDlgItem(IDC_FILTERLIST_COMBO)->ShowWindow(SW_HIDE);*/

	long nAns = CNxDialog::SetControlPositions();

	CRect rc;
	//TES 2/22/2007 - PLID 20642 - Unnecessary now that this is a datalist.
	// Make sure the filter list isn't too tall
	/*GetDlgItem(IDC_EXISTING_FILTER_LIST)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	rc.bottom = rc.top + 250;
	GetDlgItem(IDC_EXISTING_FILTER_LIST)->MoveWindow(rc);*/

	//TES 2/22/2007 - PLID 20642 - Unnecessary now that this is a datalist.
	// Make sure the group list isn't too tall
	/*m_cboGroupList.GetWindowRect(&rc);
	ScreenToClient(&rc);
	rc.bottom = rc.top + 250;
	m_cboGroupList.MoveWindow(rc);*/

	// (a.wetta 2007-01-19 15:21) - PLID 24347 - Make sure that all of the controls are correctly positioned
	RepositionControls();

	//TES 2/22/2007 - PLID 20642 - Unnecessary now that this is a datalist.
	//GetDlgItem(IDC_FILTERLIST_COMBO)->ShowWindow(SW_SHOW);

	return nAns;
}

bool CGroups::DoInitDialog(long nInitAsStyle, long nChangeToFilterId /*= FILTER_ID_UNSPECIFIED*/, long nChangeToGroupId /*= GROUP_ID_UNSPECIFIED*/)
{
	if (nInitAsStyle != m_nStyle) {
		if (nChangeToGroupId != GROUP_ID_UNSPECIFIED) {
			// First check to see if the current group has been modified
			if (IsCurrentGroupModified()) {
				switch (AfxMessageBox("The group for Person Based Merging has been modified.  Would you like to save the changes?", MB_YESNOCANCEL|MB_ICONQUESTION)) {
				case IDYES:
					{
						// Save the changes and then do some manual processing before proceeding
						long nNewGroupId = DoSaveCurrentGroup(m_nCurGroupID);
						if (nNewGroupId > 0) {
							SetGroupIdLast(nNewGroupId);
						}
					}
					break;
				case IDNO:
					// Proceed without saving the changes
					break;
				case IDCANCEL:
				default:
					// Pretend they didn't make a selection change and don't proceed
					return false;
					break;
				}
			}
		}

		// Now proceed with the initialization

		// Handle the new style (patients? contacts? etc)
		m_nStyle = nInitAsStyle;

		bool bRefreshGroup;
		bool bRefreshFilter;
		if (nChangeToGroupId != GROUP_ID_UNSPECIFIED) {
			// Fill the groups combo and select the last selected group (without refreshing the listboxes)
			FillGroupCombo(nChangeToGroupId);
			if (ChangeGroupSelection(m_nCurGroupID, false)) {
				bRefreshGroup = true;
			} else {
				bRefreshGroup = false;
			}
		} else {
			FillGroupCombo(VarLong(m_pGroupList->CurSel->GetValue(eglcID)));
			bRefreshGroup = false;
		}

		if (nChangeToFilterId != FILTER_ID_UNSPECIFIED) {
			// Fill the filters combo and select the last selected filter (without refreshing the listboxes)
			FillFilterCombo(nChangeToFilterId);
			if (ChangeFilterSelection(m_nCurFilterID, false)) {
				bRefreshFilter = true;
			} else {
				bRefreshFilter = false;
			}
		} else {
			FillFilterCombo(VarLong(m_pFilterList->CurSel->GetValue(eflcID)));
			bRefreshFilter = false;
		}

		RefreshCurrentListings(bRefreshGroup, bRefreshFilter);
	}
	return true;
}

CString CGroups::GetStyleName(long nStyle, BOOL bSingular)
{
	switch (nStyle) {
	case fbjPatient:
		if (bSingular) return "Patient";
		else return "Patients";
	case fbjContact:
		if (bSingular) return "Contact";
		else return "Contacts";
	case fbjProvider:
		if (bSingular) return "Provider";
		else return "Providers";
	case fbjRefPhys:
		if (bSingular) return "Referring Physician";
		else return "Referring Physicians";
	case fbjSupplier:
		if (bSingular) return "Supplier";
		else return "Suppliers";
	case fbjInsParty:
		if (bSingular) return "Insured Party";
		else return "Insured Parties";
	case fbjInsCo:
		if (bSingular) return "Insurance Company";
		else return "Insurance Companies";
	case fbjUser:
		if (bSingular) return "User";
		else return "Users";
	case fbjRefSource:
		if (bSingular) return "Referral Source";
		else return "Referral Sources";
	default:
		if (bSingular) return "Item";
		else return "Items";
	}
}

void CGroups::OnAdd()
{
	if (m_bAppointmentMerge)
		OnApptAdd();
	else
		OnPersonAdd();
}

void CGroups::OnAddAll()
{
	if (m_bAppointmentMerge)
		OnApptAddAll();
	else
		OnPersonAddAll();
}

void CGroups::OnRemove()
{
	if (m_bAppointmentMerge)
		OnApptRemove();
	else
		OnPersonRemove();
}

void CGroups::OnRemoveAll()
{
	if (m_bAppointmentMerge)
		OnApptRemoveAll();
	else
		OnPersonRemoveAll();
}

void CGroups::OnPersonAdd() 
{
	long nCount = 0;
	if (CheckCurrentUserPermissions(bioLWGroup, sptWrite)) {
		if (!IsGroupReadOnly(m_nCurGroupID)) {
			CWaitCursor	wait;
			try {
				//*
				long p = m_unselected->GetFirstSelEnum();
				bool bAlreadySetModified = IsCurrentGroupModified();
				while (p) {
					LPDISPATCH lpDisp;
					m_unselected->GetNextSelEnum(&p, &lpDisp);
					if (lpDisp) {
						// CAH 7/18/02: If the row is already in the selected list, don't add it.
						// We define a row is "already in the selected list" if the value in column 0
						// already exists for a row in the selected list.
						IRowSettingsPtr pRow = lpDisp;
						if (-1 == m_selected->FindByColumn(0, pRow->Value[0], 0, FALSE))
						{
							m_selected->AddRow(lpDisp);
							if (!bAlreadySetModified) {
								SetCurrentGroupModified(true);
								bAlreadySetModified = true;
							}
						}
					}
				}
				//*/
				/*
				if (m_selected->TakeCurrentRow(m_unselected)) {
					SetCurrentGroupModified(true);
				}
				//*/
			} NxCatchAll("CGroups::OnAdd");
		} else {
			try {
				long nCurSel = m_unselected->CurSel;
				if (nCurSel != -1) {
					int nResult = MsgBox(MB_YESNO|MB_ICONQUESTION, 
						"\"%s\" is a built-in group and therefore may not be changed.\n\n"
						"Would you like to add the selected %s to a new group?", 
						GetGroupName(m_nCurGroupID), GetStyleName(m_nStyle, TRUE));
					
					if (nResult == IDYES) {
						m_pGroupList->SetSelByColumn(eglcID, (long)GROUP_ID_NEW);

						// Switch what group we're on, but don't change what's in the listing
						m_nCurGroupID = GROUP_ID_NEW;
						SetGroupIdLast(m_nCurGroupID);
						SetCurrentGroupModified(false);

						// And then send the OnAdd message again so that items are added to the new group
						// TODO: Why do we post a message instead of just adding the selected items here?
						PostMessage(WM_COMMAND, MAKELONG(IDC_ADD, BN_CLICKED));
					}
				}
			} NxCatchAll("CGroups::OnAdd 2");
		}
	}
	// Reflect the list counts
	ReflectListCount(LWL_ALL_LISTS);
}

void CGroups::OnPersonAddAll() 
{
	if (CheckCurrentUserPermissions(bioLWGroup, sptWrite)) {
		if (!IsGroupReadOnly(m_nCurGroupID)) {
		
			CWaitCursor	wait;
			try {
				//*
				long p = m_unselected->GetFirstRowEnum();
				bool bAlreadySetModified = IsCurrentGroupModified();
				while (p) {
					LPDISPATCH lpDisp;
					m_unselected->GetNextRowEnum(&p, &lpDisp);
					if (lpDisp) {

						IRowSettingsPtr pRow = lpDisp;
						lpDisp->Release();
						if (-1 == m_selected->FindByColumn(0, pRow->Value[0], 0, FALSE))
						{
							m_selected->InsertRow(lpDisp, m_selected->GetRowCount());
							if (!bAlreadySetModified) {
								SetCurrentGroupModified(true);
								bAlreadySetModified = true;
							}
						}
					}
				}
				m_selected->Sort();
				//*/
				/*
				if (m_selected->TakeAllRows(m_unselected) > 0) {
					// At least one record has been modified
					SetCurrentGroupModified(true);
				}
				//*/
			} NxCatchAll("CGroups::OnAddAll");
		} else {
			// Ask the user what she wants to do with the built-in group
			int nResult = MsgBox(MB_YESNO|MB_ICONQUESTION, 
				"\"%s\" is a built-in group and therefore may not be changed.\n\n"
				"Would you like to create a new group with every %s", 
				GetGroupName(m_nCurGroupID), GetStyleName(m_nStyle, TRUE));
			
			if (nResult == IDYES) {
				m_nCurGroupID = GROUP_ID_NEW;
				m_pGroupList->SetSelByColumn(0, m_nCurGroupID);
				if (ChangeGroupSelection(m_nCurGroupID, false)) {
					// TODO: Why do we post a message instead of just adding the selected items here?
					PostMessage(WM_COMMAND, MAKELONG(IDC_ADD_ALL, BN_CLICKED));
				}
			}
		}
	}
	// Reflect the list counts
	ReflectListCount(LWL_ALL_LISTS);
}

void CGroups::OnPersonRemove() 
{
	if (CheckCurrentUserPermissions(bioLWGroup, sptWrite)) {
		if (!IsGroupReadOnly(m_nCurGroupID)) {
		
			CWaitCursor	wait;
			try {
				// Loop through all selected rows and remove them all
				{
					// First get the indexes of all rows we're going to remove (NOTE: we used to just iterate 
					// through and remove any row that was selected, but since the datalist now changes the 
					// selection to a new row when you remove the current row, that algorithm can't work 
					// anymore.  Now we have to first say, "ok what rows are we going to remove", and THEN 
					// remove exactly those rows)
					CDWordArray arydwRemoveRows;
					// Note, we have to make sure we're looping in ascending INDEX order, that's why we can't 
					// use "GetFirstSelEnum".
					long p = m_selected->GetFirstRowEnum();
					for (long nIndex=0; p; nIndex++) {
						// Get the row dispatch
						LPDISPATCH lpDisp;
						m_selected->GetNextRowEnum(&p, &lpDisp);
						if (lpDisp) {
							// See if the row is highlighted
							IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
							if (pRow->IsHighlighted()) {
								// Add the row's index to the array
								// (b.cardillo 2003-07-11 13:28) This is slightly dangerous, we're assuming 
								// nIndex is truly the row index
								arydwRemoveRows.Add(nIndex);
							}
						}
					}
					
					// See if we're going to remove any rows
					long nCountToRemove = arydwRemoveRows.GetSize();
					if (nCountToRemove > 0) {
						// We are so remove them
						for (long i=0; i<nCountToRemove; i++) {
							// Note we have to subtract i from the index, because i represents how many rows 
							// we've removed since we filled the arydwRemoteRows array; (this is another 
							// reason it's so important to create our array in ascending order)
							m_selected->RemoveRow(arydwRemoveRows.GetAt(i) - i);
						}
						// And mark the group as modified
						SetCurrentGroupModified(true);
					}
				}
				//*/

				// (b.cardillo 2003-07-11 13:15) TODO: We don't use the datalist's "TakeCurrentRow" because we 
				// don't want to add the row to the m_unselected list, we just want to remove it from the 
				// m_selected list. It would be nice if the datalist had a "CopyCurrentRow".
				/*
				if (m_unselected->TakeCurrentRow(m_selected)) {
					SetCurrentGroupModified(true);
				}
				//*/
			} NxCatchAll("CGroups::OnRemove");
		} else {
			// Prompt the user to create a new empty group since the current one is read-only
			int nResult = MsgBox(MB_YESNO|MB_ICONQUESTION, 
				"\"%s\" is a built-in group and therefore may not be changed.\n\n"
				"Would you like to create a new empty group?", GetGroupName(m_nCurGroupID));
			
			// If they want to, go ahead and create it
			if (nResult == IDYES) {
				m_nCurGroupID = GROUP_ID_NEW;
				m_pGroupList->SetSelByColumn(0, m_nCurGroupID);
				if (ChangeGroupSelection(m_nCurGroupID, false)) {
					// TODO: Why do we post a message instead of just adding the selected items here?
					PostMessage(WM_COMMAND, MAKELONG(IDC_REMOVE_ALL, BN_CLICKED));
				}
			}
		}
	}
	// Reflect the list counts
	ReflectListCount(LWL_ALL_LISTS);
}

void CGroups::OnPersonRemoveAll() 
{
	if (CheckCurrentUserPermissions(bioLWGroup, sptWrite)) {
		if (!IsGroupReadOnly(m_nCurGroupID)) {
		
			CWaitCursor	wait;
			try {
				if (m_selected->GetRowCount() > 0) {
					m_selected->Clear();
					SetCurrentGroupModified(true);
				}
				/*
				if (m_unselected->TakeAllRows(m_selected) > 0) {
					// At least one record has been modified
					SetCurrentGroupModified(true);
				}
				//*/
			} NxCatchAll("CGroups::OnRemoveAll");
		} else {
			// Prompt the user to create a new empty group since the current one is read-only
			int nResult = MsgBox(MB_YESNO|MB_ICONQUESTION, 
				"\"%s\" is a built-in group and therefore may not be changed.\n\n"
				"Would you like to create a new empty group?", GetGroupName(m_nCurGroupID));
			
			// If they want to, go ahead and create it
			if (nResult == IDYES) {
				m_nCurGroupID = GROUP_ID_NEW;
				m_pGroupList->SetSelByColumn(0, m_nCurGroupID);
				if (ChangeGroupSelection(GROUP_ID_NEW, false)) {
					// TODO: Why do we post a message instead of just adding the selected items here?
					PostMessage(WM_COMMAND, MAKELONG(IDC_REMOVE_ALL, BN_CLICKED));
				}
			}
		}
	}
	// Reflect the list counts
	ReflectListCount(LWL_ALL_LISTS);
}

void CGroups::OnApptAdd()
{
	try {
		long p = m_apptUnselected->GetFirstSelEnum();
		while (p) {
			LPDISPATCH lpDisp;
			m_apptUnselected->GetNextSelEnum(&p, &lpDisp);
			if (lpDisp) {
				IRowSettingsPtr pRow = lpDisp;
				if (-1 == m_apptSelected->FindByColumn(0, pRow->Value[0], 0, FALSE))
				{
					m_apptSelected->AddRow(lpDisp);
				}
			}
		}

		ReflectListCount(LWL_ALL_LISTS);
	}NxCatchAll("Error in CGroups::OnApptAdd");
}

void CGroups::OnApptAddAll()
{
	try {
		long p = m_apptUnselected->GetFirstRowEnum();
		while (p) {
			LPDISPATCH lpDisp;
			m_apptUnselected->GetNextRowEnum(&p, &lpDisp);
			if (lpDisp) {
				IRowSettingsPtr pRow = lpDisp;
				lpDisp->Release();
				if (-1 == m_apptSelected->FindByColumn(0, pRow->Value[0], 0, FALSE))
				{
					m_apptSelected->AddRow(lpDisp);
				}
			}
		}
		m_apptSelected->Sort();
					
		// Reflect the list counts
		ReflectListCount(LWL_ALL_LISTS);

	}NxCatchAll("Error in CGroups::OnApptAddAll");
}

void CGroups::OnApptRemove() 
{
	try {
		// First get the indexes of all rows we're going to remove (NOTE: we used to just iterate 
		// through and remove any row that was selected, but since the datalist now changes the 
		// selection to a new row when you remove the current row, that algorithm can't work 
		// anymore.  Now we have to first say, "ok what rows are we going to remove", and THEN 
		// remove exactly those rows)
		CDWordArray arydwRemoveRows;
		// Note, we have to make sure we're looping in ascending INDEX order, that's why we can't 
		// use "GetFirstSelEnum".
		long p = m_apptSelected->GetFirstRowEnum();
		for (long nIndex=0; p; nIndex++) {
			// Get the row dispatch
			LPDISPATCH lpDisp;
			m_apptSelected->GetNextRowEnum(&p, &lpDisp);
			if (lpDisp) {
				// See if the row is highlighted
				IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
				if (pRow->IsHighlighted()) {
					// Add the row's index to the array
					// (b.cardillo 2003-07-11 13:28) This is slightly dangerous, we're assuming 
					// nIndex is truly the row index
					arydwRemoveRows.Add(nIndex);
				}
			}
		}
		
		// See if we're going to remove any rows
		long nCountToRemove = arydwRemoveRows.GetSize();
		if (nCountToRemove > 0) {
			// We are so remove them
			for (long i=0; i<nCountToRemove; i++) {
				// Note we have to subtract i from the index, because i represents how many rows 
				// we've removed since we filled the arydwRemoteRows array; (this is another 
				// reason it's so important to create our array in ascending order)
				m_apptSelected->RemoveRow(arydwRemoveRows.GetAt(i) - i);
			}
		}

		// Reflect the list counts
		ReflectListCount(LWL_ALL_LISTS);

	}NxCatchAll("Error in CGroups::OnApptRemove");
}

void CGroups::OnApptRemoveAll()
{
	try {		
		if (m_apptSelected->GetRowCount() > 0) {
			m_apptSelected->Clear();
		}

		// Reflect the list counts
		ReflectListCount(LWL_ALL_LISTS);

	}NxCatchAll("Error in CGroups::OnApptRemoveAll");
}

void CGroups::OnDblClickCellSelected(long nRow, short nCol) 
{
	OnRemove();
}

void CGroups::OnDblClickCellUnselected(long nRow, short nCol) 
{
	OnAdd();
}


void CGroups::OnSaveGroup() 
{
	long nOldGroupId = m_nCurGroupID;
	long nNewGroupId = DoSaveCurrentGroup(nOldGroupId, GROUP_ID_UNSPECIFIED);
	if (nNewGroupId > 0) {
		if (nOldGroupId == nNewGroupId) {
			SetCurrentGroupModified(false);
		} else {
			ChangeGroupSelection(nNewGroupId);
		}
	}
}

void CGroups::OnDeleteGroup() 
{
	if (!CheckCurrentUserPermissions(bioLWGroup, sptWrite))
		return;

	long nGroupId = m_nCurGroupID;
	CString strOld;		//for auditing
	strOld = VarString(m_pGroupList->CurSel->GetValue(eglcName));
	if (!IsGroupReadOnly(nGroupId)) {

		// (c.haag 2003-10-01 15:06) - Check to see which filters use this group.
		CString strFilters = "This group is used in the following filters:\r\n\r\n";
		_RecordsetPtr prsFilters = CreateRecordset("SELECT ID, Name, Filter FROM FiltersT WHERE Filter LIKE '%%\"%d\"%%'", nGroupId);
		BOOL bInFilters = FALSE;
		while (!prsFilters->eof)
		{
			// Get the filter SQL string to filter on
			CString strFilter = AdoFldString(prsFilters, "Filter");
			CString strWhere, strFrom;
			
			// Grab only the WHERE clause from the filter string
			if (CFilter::ConvertFilterStringToClause(AdoFldLong(prsFilters, "ID"), strFilter, fboPerson, &strWhere, &strFrom))
			{
				CString strSearch;
				strSearch.Format("GroupID = %d", nGroupId);
				if (-1 != strWhere.Find(strSearch, 0))
				{
					bInFilters = TRUE;
					strFilters += AdoFldString(prsFilters, "Name", "") + "\r\n";
					break;
				}
			}
			prsFilters->MoveNext();
		}
		prsFilters->Close();
		if (bInFilters)
		{
			strFilters += "\r\nThese filters must be changed before you can delete this group.";
			if (AfxMessageBox(strFilters))
				return;
		}

		if (AfxMessageBox("Are you sure you want to permanently delete this group?", MB_YESNO|MB_ICONQUESTION) == IDYES) {
			if (!DoDeleteGroup(nGroupId)) {
				MsgBox(
					"\"%s\" is a built-in group and therefore may not be deleted.\n\n"
					"Please select another group in order to make changes, or click the \n"
					"\"New\" button if you would like to create a new group.", GetGroupName(nGroupId));
			}

			//auditing
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiGroupDeleted, -1, strOld, "<Deleted>", aepMedium, aetDeleted);
 		}
	} else {
		// Cannot delete built-in groups
		MsgBox(
			"\"%s\" is a built-in group and may not be deleted.", GetGroupName(nGroupId));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//// Created for Letter Writing 5.0

/////////////////////////////////////////////////////////////////////////
//  Groups 5.0
/////////////////////////

long CGroups::GetGroupIdLast()
{
	CString strPropName;
	long nDefault;
	strPropName = "GroupIDPatientLast";
	nDefault = GROUP_ID_CURRENT_PATIENT;
	return GetRemotePropertyInt(strPropName, nDefault, 0, GetCurrentUserName(), false);
}

void CGroups::SetGroupIdLast(long nGroupId)
{
	CString strPropName;
	strPropName = "GroupIDPatientLast";
	SetRemotePropertyInt(strPropName, nGroupId, 0, GetCurrentUserName());
}

long CGroups::GetFilterIdLast(BOOL bAppointmentFilter /*= FALSE*/)
{
	CString strPropName;
	long nDefault;
	if (bAppointmentFilter)
		strPropName = "ApptFilterIDPatientLast";
	else
		strPropName = "FilterIDPatientLast";
	nDefault = FILTER_ID_ALL;
	return GetRemotePropertyInt(strPropName, nDefault, 0, GetCurrentUserName(), false);
}

void CGroups::SetFilterIdLast(long nFilterId, BOOL bAppointmentFilter /*= FALSE*/)
{
	CString strPropName;
	if (bAppointmentFilter)
		strPropName = "ApptFilterIDPatientLast";
	else
		strPropName = "FilterIDPatientLast";
	SetRemotePropertyInt(strPropName, nFilterId, 0, GetCurrentUserName());
}


// Pass the Group ID of the group to be selected after refilling (or -1 to select the "New Group..." group)
UINT CGroups::FillGroupCombo(long nSelectId)
{
	// Prepare variables
	int nIndex = 0;
	try {
		//TES 2/22/2007 - PLID 20642 - First, requery from data.
		m_pGroupList->Requery();
		
		////TES 2/22/2007 - PLID 20642 - Obviously it would be better if this weren't synchronous, but this function has
		// always been synchronous, so that's a project for another time.
		m_pGroupList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);;

		// Add built-in entries
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pGroupList->GetNewRow();
		pRow->PutValue(eglcID, (long)GROUP_ID_CURRENT_PATIENT);
		pRow->PutValue(eglcName, _bstr_t(GetGroupName(GROUP_ID_CURRENT_PATIENT)));
		m_pGroupList->AddRowAtEnd(pRow, NULL);

		pRow = m_pGroupList->GetNewRow();
		pRow->PutValue(eglcID, (long)GROUP_ID_CURRENT_LOOKUP);
		pRow->PutValue(eglcName, _bstr_t(GetGroupName(GROUP_ID_CURRENT_LOOKUP)));
		m_pGroupList->AddRowAtEnd(pRow, NULL);

		pRow = m_pGroupList->GetNewRow();
		pRow->PutValue(eglcID, (long)GROUP_ID_CURRENT_FILTER);
		pRow->PutValue(eglcName, _bstr_t(GetGroupName(GROUP_ID_CURRENT_FILTER)));
		m_pGroupList->AddRowAtEnd(pRow, NULL);

		if(GetCurrentUserPermissions(bioLWGroup) & (SPT___W________ANDPASS)) {
			pRow = m_pGroupList->GetNewRow();
			pRow->PutValue(eglcID, (long)GROUP_ID_NEW);
			pRow->PutValue(eglcName, _bstr_t(GetGroupName(GROUP_ID_NEW)));
			m_pGroupList->AddRowAtEnd(pRow, NULL);
		} 

		m_nCurGroupID = nSelectId;
		// Set the current group to be what was specified above (or default)
		pRow = m_pGroupList->FindByColumn(eglcID, m_nCurGroupID, 0, VARIANT_FALSE);
		if (pRow == NULL) {
			//Try the default (GROUP_ID_NEW)
			pRow = m_pGroupList->FindByColumn(eglcID, (long)GROUP_ID_NEW, 0, VARIANT_FALSE);
			if(pRow == NULL) {
				//GROUP_ID_NEW may not exist if they don't have permissions.  Go to our secondary default.
				pRow = m_pGroupList->FindByColumn(eglcID, (long)GROUP_ID_CURRENT_PATIENT, 0, VARIANT_FALSE);
			}
		}
		//CURRENT_PATIENT, at least should always exist (we just added it).
		ASSERT(pRow != NULL);
		m_pGroupList->CurSel = pRow;
		m_nCurGroupID = VarLong(pRow->GetValue(eglcID));
	} NxCatchAll("CGroups::FillGroupCombo");
	
	return m_pGroupList->GetRowCount();
}

// Returns the Group ID of the saved group or 0 for failure (if nSaveGroupId refers to a built-in group, a new group will be saved)
long CGroups::DoSaveCurrentGroup(long nSaveGroupId, long nSelectDifferentId /* = GROUP_ID_UNSPECIFIED */)
{
	// Check permission
	if (!CheckCurrentUserPermissions(bioLWGroup, sptWrite)) {
		return GROUP_ID_SAVE_CANCELED;
	}
	
	long nAns = GROUP_ID_UNSPECIFIED;

	//if this is not a "New Group..." and is still a system group, prompt if they want to make a new one
	//we don't want this embedded in the next if statement, so the else works properly
	if(!IsUserDefinedGroup(nSaveGroupId) && nSaveGroupId != GROUP_ID_NEW && IDNO == MessageBox("You cannot change a system group. Would you like to make a new group?","Practice",MB_ICONQUESTION|MB_YESNO))
		return GROUP_ID_SAVE_CANCELED;

	if(m_selected->GetRowCount() == 0) {
		if(IDYES == MessageBox("There are no patients in this group. If you save, the group will be deleted.\n"
			"Do you wish to delete this group?\n\n"
			"Click 'Yes' to delete, click 'No' to cancel the save.","Practice",MB_ICONQUESTION|MB_YESNO)) {
			DoDeleteGroup(nSaveGroupId);
			return GROUP_ID_UNSPECIFIED;
		}
		else {
			return GROUP_ID_SAVE_CANCELED;
		}
	}

	if (!IsUserDefinedGroup(nSaveGroupId) || (nSaveGroupId == GROUP_ID_NEW)) {
		// This is a new group
		CString strGroupName = "Untitled Group";
		if (Prompt("Enter the name for this group:", strGroupName, 50) == IDOK) {

			strGroupName.TrimRight();

			if(strGroupName == "") {
				AfxMessageBox("Please enter a non-blank name for this group.");
				return GROUP_ID_SAVE_CANCELED;
			}

			if(strGroupName == "{Current Patient}" || strGroupName == "{Current Filter}" || strGroupName == "New Group...") {
				AfxMessageBox("You cannot make a group with the same name as a system group.\n"
					"Please enter a different name.");
				return GROUP_ID_SAVE_CANCELED;
			}

			try {
				// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
				CSqlTransaction trans("CGroups::DoSaveCurrentGroup");
				trans.Begin();

				// If anything fails, we want to return failure
				nAns = GROUP_ID_UNSPECIFIED;

				_RecordsetPtr rs = CreateRecordset("SELECT TOP 1 ID FROM GroupsT WHERE Name = '%s'",_Q(strGroupName));
				if(!rs->eof) {
					rs->Close();
					// (a.walling 2010-09-08 13:26) - PLID 40377 - Using CSqlTransaction - Rollback is implicit. But we must rollback before messagebox!
					trans.Rollback();
					AfxMessageBox("That group name already exists, please choose a new name.");
					return GROUP_ID_SAVE_CANCELED;
				}
				else rs->Close();

				CWaitCursor wc;

				// Create the new group
				long nNewGroupId = NewNumber("GroupsT", "ID");
				ExecuteSql("INSERT INTO GroupsT ( ID, Name ) VALUES (%li, '%s')", nNewGroupId, _Q(strGroupName));

				// Remember the sort order
				short arySortPriority[4] = {
					m_selected->GetColumn(0)->GetSortPriority(),
					m_selected->GetColumn(1)->GetSortPriority(),
					m_selected->GetColumn(2)->GetSortPriority(),
					m_selected->GetColumn(3)->GetSortPriority()
				};
				// Set the new sort order (so that we can eliminate duplicates)
				m_selected->GetColumn(0)->SortPriority = 3;
				m_selected->GetColumn(1)->SortPriority = 0;
				m_selected->GetColumn(2)->SortPriority = 1;
				m_selected->GetColumn(3)->SortPriority = 2;
				m_selected->Sort();

				// Create a temp table and then select from the temp table into 
				// the group (this is fast in MSDE 8 because we can use XML. With 
				// SQL 7 it's the same speed as the old days)
				CString strTempGroupT = CreateTempIDTable(m_selected, 0, FALSE, TRUE);

				ExecuteSql(
					"INSERT INTO GroupDetailsT (GroupID, PersonID) SELECT %li AS GroupID, ID AS PersonID FROM %s", 
					nNewGroupId, strTempGroupT);

				// Go back to the original sort order
				m_selected->GetColumn(0)->SortPriority = arySortPriority[0];
				m_selected->GetColumn(1)->SortPriority = arySortPriority[1];
				m_selected->GetColumn(2)->SortPriority = arySortPriority[2];
				m_selected->GetColumn(3)->SortPriority = arySortPriority[3];
				m_selected->Sort();

				//auditing
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, "", nAuditID, aeiGroupCreated, -1, "", strGroupName, aepMedium, aetChanged);

				// We will return success
				nAns = nNewGroupId;
				m_groupChecker.Refresh();

				trans.Commit();
			} NxCatchAll("CGroups::DoSaveCurrentGroup");
		} else {
			return GROUP_ID_SAVE_CANCELED;
		}
	} else if (!IsGroupReadOnly(nSaveGroupId)) {
		CString strGroupName, strOld;	//strOld for auditing
		strGroupName = GetGroupName(nSaveGroupId);
		strOld = strGroupName;
		if (Prompt("You may change the name of this group while you save it:", strGroupName, 50) == IDOK) {
			// This is not a new group but the user may want to change its name
			CString strSql;

			strGroupName.TrimRight();

			if(strGroupName == "") {
				AfxMessageBox("Please enter a non-blank name for this group.");
				return GROUP_ID_SAVE_CANCELED;
			}

			if(strGroupName == "{Current Patient}" || strGroupName == "{Current Filter}" || strGroupName == "New Group...") {
				AfxMessageBox("You cannot make a group with the same name as a system group.\n"
					"Please enter a different name.");
				return GROUP_ID_SAVE_CANCELED;
			}

			try {
				// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
				CSqlTransaction trans("CGroups::DoSaveCurrentGroup");
				trans.Begin();

				// If anything fails, we want to return failure
				nAns = GROUP_ID_UNSPECIFIED;

				_RecordsetPtr rs = CreateRecordset("SELECT ID FROM GroupsT WHERE Name = '%s' AND ID <> %li",_Q(strGroupName),nSaveGroupId);
				if(!rs->eof) {
					// (a.walling 2010-09-08 13:26) - PLID 40377 - Using CSqlTransaction - Rollback is implicit. But we must rollback before messagebox!
					trans.Rollback();
					AfxMessageBox("There is another group that already has that name. Please choose a new name.");
					return GROUP_ID_SAVE_CANCELED;
				}
				else rs->Close();

				// Name the group
				ExecuteSql("UPDATE GroupsT SET Name = '%s' WHERE ID = %li", _Q(strGroupName), nSaveGroupId);

				// Delete all the group's current stored elements
				ExecuteSql("DELETE FROM GroupDetailsT WHERE (GroupID = %li)", nSaveGroupId);

				// Remember the sort order
				short arySortPriority[4] = {
					m_selected->GetColumn(0)->GetSortPriority(),
					m_selected->GetColumn(1)->GetSortPriority(),
					m_selected->GetColumn(2)->GetSortPriority(),
					m_selected->GetColumn(3)->GetSortPriority()
				};
				// Set the new sort order (so that we can eliminate duplicates)
				m_selected->GetColumn(0)->SortPriority = 3;
				m_selected->GetColumn(1)->SortPriority = 0;
				m_selected->GetColumn(2)->SortPriority = 1;
				m_selected->GetColumn(3)->SortPriority = 2;
				m_selected->Sort();

				// Create a temp table and then select from the temp table into 
				// the group (this is fast in MSDE 8 because we can use XML. With 
				// SQL 7 it's the same speed as the old days)
				CString strTempGroupT = CreateTempIDTable(m_selected, 0, FALSE, TRUE);

				ExecuteSql(
					"INSERT INTO GroupDetailsT (GroupID, PersonID) SELECT %li AS GroupID, ID AS PersonID FROM %s", 
					nSaveGroupId, strTempGroupT);

				//auditing
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, "", nAuditID, aeiGroup, -1, strOld, strGroupName, aepMedium, aetChanged);
				
				trans.Commit();

				// Go back to the original sort order
				m_selected->GetColumn(0)->SortPriority = arySortPriority[0];
				m_selected->GetColumn(1)->SortPriority = arySortPriority[1];
				m_selected->GetColumn(2)->SortPriority = arySortPriority[2];
				m_selected->GetColumn(3)->SortPriority = arySortPriority[3];
				m_selected->Sort();

				// We will return success
				nAns = nSaveGroupId;
				m_groupChecker.Refresh();
			} NxCatchAll("CGroups::DoSaveCurrentGroup");
		} else {
			return GROUP_ID_SAVE_CANCELED;
		}
	} else {
		ASSERT(FALSE);
		MsgBox("The group is neither a built-in group, nor a user-defined group.");
		// We will return failure
		nAns = GROUP_ID_UNSPECIFIED;
	}

	// Refresh the combo box (even if there were errors because we want to reflect changes made by remote users)
	long nSelectId;
	if (nSelectDifferentId != GROUP_ID_UNSPECIFIED) {
		nSelectId = nSelectDifferentId;
	} else if ((nAns != GROUP_ID_UNSPECIFIED) && (nAns != GROUP_ID_SAVE_CANCELED)) {
		nSelectId = nAns;
	} else {
		nSelectId = m_nCurGroupID;
	}
	
	FillGroupCombo(nSelectId);

	// Return the result
	return nAns;
}

bool CGroups::DoDeleteGroup(long nDeleteGroupId)
{
	try {
		// Delete the group
		BEGIN_TRANS("CGroups::DoDeleteGroup") {
			CWaitCursor wc;

			ExecuteSql("DELETE FROM GroupDetailsT WHERE GroupID = %i", nDeleteGroupId);
			ExecuteSql("DELETE FROM GroupsT WHERE ID = %i", nDeleteGroupId);
		} END_TRANS("CGroups::DoDeleteGroup");

		m_groupChecker.Refresh();

		// If deletion was successful prepare for a new group
		FillGroupCombo(GetGroupIdLast());
		ChangeGroupSelection(m_nCurGroupID);
		return true;
		
	} NxCatchAll("CGroups::DoDeleteGroup");

	// Failure
	return false;
}

// Refreshes the data so as to ensure that it is consistent with the specified group ID (-1 indicates NEW GROUP)
bool CGroups::ChangeGroupSelection(long nGroupId, bool bAutoRefreshListings /*= true*/)
{
	// Open a block here so the waitcursor goes out of scope prior to the call to refreshcurrentlistings
	{
		CWaitCursor wc;

		// Remember this group for next time
		SetGroupIdLast(nGroupId);
		
		// Now make sure the group listing is accurate
		CString strWhere;
		if (nGroupId == GROUP_ID_CURRENT_PATIENT) {
			// Prepare appropriate sql for the current patient or contact
			if (m_nStyle & fbjPatient) {
				strWhere.Format("PersonT.ID = %li", GetActivePatientID());
			} else {
				strWhere.Format("PersonT.ID = %li", GetActiveContactID());
			}
		} else if (nGroupId == GROUP_ID_CURRENT_LOOKUP) {
			// Since the where clause itself depends on what options will be set at the 
			// time of the refresh of the group listing, we need to hold off on creating 
			// the where clause until then
		} else if (nGroupId == GROUP_ID_CURRENT_FILTER) {
			// No sql is associated with this special group so do nothing
		} else {
			// Prepare appropriate sql for listings
			strWhere.Format(
				"PersonT.ID IN (SELECT PersonID AS ID FROM GroupDetailsT "
				"WHERE GroupID = %i)", nGroupId);
		}

		try {
			// Change the filters
			m_selected->WhereClause = _bstr_t(strWhere);

			// Set it as unmodified
			SetCurrentGroupModified(false);
		} NxCatchAllCall("CGroups::ChangeGroupSelection", return false);

		// Enable and disable various buttons, depending on what's available to the user for the given group
		RefreshGroupAccess(nGroupId);
	}
	
	// Reflect new group on screen
	if (bAutoRefreshListings) RefreshCurrentListings(true, false);

	return true;
}

void CGroups::ReflectListCount(UINT nList)
{
	CString strCount;

	if (nList & LWL_FILTER_LIST) {
		// Report the filter result count
		if (m_bAppointmentMerge) 
			strCount.Format("%li", m_apptUnselected->GetRowCount());
		else
			strCount.Format("%li", m_unselected->GetRowCount());
		SetDlgItemText(IDC_AVAILCOUNT, strCount);

		CRect rc;
		GetDlgItem(IDC_AVAILCOUNT)->GetWindowRect(rc);
		ScreenToClient(rc);
		RedrawWindow(rc);
	}

	if (nList & LWL_GROUP_LIST) {
		// Report the selected count and the unselected count
		if (m_bAppointmentMerge)
			strCount.Format("%li", m_apptSelected->GetRowCount());
		else
			strCount.Format("%li", m_selected->GetRowCount());
		SetDlgItemText(IDC_SELCOUNT, strCount);

		CRect rc;
		GetDlgItem(IDC_SELCOUNT)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		if (!m_bAppointmentMerge) {
			long nGroupId = m_nCurGroupID;
			if (nGroupId == GROUP_ID_NEW) {
				if (m_selected->GetRowCount() > 0) {
					GetDlgItem(IDC_NEW_GROUP_BTN)->EnableWindow(TRUE);
					GetDlgItem(IDC_SAVEGROUP)->EnableWindow(TRUE);
				} else {
					GetDlgItem(IDC_NEW_GROUP_BTN)->EnableWindow(FALSE);
					GetDlgItem(IDC_SAVEGROUP)->EnableWindow(FALSE);
				}
			} else {
				GetDlgItem(IDC_NEW_GROUP_BTN)->EnableWindow(GetCurrentUserPermissions(bioLWGroup) & (SPT___W________ANDPASS));
				// (j.gruber 2007-02-26 10:05) - PLID 23826 - enable this
				GetDlgItem(IDC_SAVEGROUP)->EnableWindow(GetCurrentUserPermissions(bioLWGroup) & (SPT___W________ANDPASS));
			}
		} else {
			GetDlgItem(IDC_NEW_GROUP_BTN)->EnableWindow(GetCurrentUserPermissions(bioLWGroup) & (SPT___W________ANDPASS));

		}
	}
}

void CGroups::SetListCountToLoading()
{
	SetDlgItemText(IDC_AVAILCOUNT, "Loading...");
	CRect rc;
	GetDlgItem(IDC_AVAILCOUNT)->GetWindowRect(rc);
	ScreenToClient(rc);
	RedrawWindow(rc);
}


void CGroups::RefreshCurrentListings(bool bIncludeGroupListing /*= true*/, bool bForceFilterListing /*= false*/)
{
	CWaitCursor wc;

	long nGroupId = m_nCurGroupID;
	bool bNeedRefreshFilter = bForceFilterListing;
	bool bNeedRefreshGroup = (bIncludeGroupListing || ((bNeedRefreshFilter) && (nGroupId == GROUP_ID_CURRENT_FILTER)));

	// Requery the filter listing
	if (bNeedRefreshFilter) {
		// Requery the filter result listbox
		EnableFilterAccess(FALSE);
		
		// Start requerying the data
		SetListCountToLoading();

		// (a.walling 2007-11-07 12:52) - PLID 27998 - VS2008 - DEBUG is an MFC definition, so we will call this DEBUGCODE
		DEBUGCODE(Log("%s", (LPCTSTR)m_unselected->GetSqlPending()));

		SetColumnWidths();
		
		if (m_bAppointmentMerge) {
			m_apptUnselected->Requery();
		}
		else
			m_unselected->Requery();
	}

	// Requery the group listing
	if (bNeedRefreshGroup) {
		// Show the user that we're requerying
		EnableGroupAccess(FALSE);
		SetDlgItemText(IDC_SELCOUNT, "Loading...");
		CRect rc;
		GetDlgItem(IDC_SELCOUNT)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		// Are we in the special case of the group being "current filter"?
		if (nGroupId == GROUP_ID_CURRENT_FILTER) {
			// Empty the existing group listing contents
			m_selected->Clear();
			// Plan on requerying the group when the filter is done
			m_bCurrentFilterGroupNeedsRefresh = true;
			// If we're not requering the filter just transfer the filter contents to the group
			if (!m_unselected->IsRequerying()) {
				// We're gonna transfer the filter contents right now so we don't need to do it later
				m_bCurrentFilterGroupNeedsRefresh = false;
				// Transfer the filter contents to the group
				SetCurrentGroupModified(false);
				long p = m_unselected->GetFirstRowEnum();
				while (p) {
					LPDISPATCH lpDisp;
					m_unselected->GetNextRowEnum(&p, &lpDisp);
					if (lpDisp) {
						m_selected->InsertRow(lpDisp, m_selected->GetRowCount());
						lpDisp->Release();
					}
				}
				m_selected->Sort();
				// Reflect the fact that we have an official group listing on screen now
				ReflectListCount(LWL_GROUP_LIST);
				EnableGroupAccess(TRUE);
			} else {
				IRowSettingsPtr pRow = m_selected->Row[-1];
				pRow->Value[1] = _bstr_t("Loading...");
				m_selected->AddRow(pRow);
			}
		} else if (nGroupId == GROUP_ID_CURRENT_LOOKUP) {
			// Make a where clause that gives persons in the filter given by the current patient lookup
			CString strWhere;
			{
				CString strLookupWhere = GetMainFrame()->GetPatientFilter();
				if (!strLookupWhere.IsEmpty()) {
					strLookupWhere.Insert(0, " AND (");
					strLookupWhere += ")";
				}
				// (b.cardillo 2007-03-13 14:06) - PLID 25123 - Included the NOLOCK hint on the PatientsT 
				// table, as this is a frequently run query that doesn't need the lock.
				strWhere.Format("PersonT.ID IN "
					"(SELECT DISTINCT PersonT.ID "
					" FROM PersonT "
					" INNER JOIN PatientsT WITH (NOLOCK) ON PersonT.ID = PatientsT.PersonID "
					" WHERE PersonT.ID > 0 AND PatientsT.CurrentStatus <> 4 %s)", strLookupWhere);
			}
			m_selected->PutWhereClause(_bstr_t(strWhere));
			m_selected->Requery();
		} else {
			// Start requerying the data
			// (a.walling 2007-11-07 12:52) - PLID 27998 - VS2008 - DEBUG is an MFC definition, so we will call this DEBUGCODE
			DEBUGCODE(Log("%s", (LPCTSTR)m_selected->GetSqlPending()));
			m_selected->Requery();
		}
	}

	SetUnselectedColumnSizes();
	SetSelectedColumnSizes();
	SetApptUnselectedColumnSizes();
	SetApptSelectedColumnSizes();
}

void CGroups::SetCurrentGroupModified(bool bModified /*= true*/)
{
	// Save the new status
	m_bGroupModified = bModified;

	// Decide what the save button should say
	if (m_bGroupModified) {
		GetDlgItem(IDC_SAVEGROUP)->EnableWindow(TRUE);
		SetDlgItemText(IDC_SAVEGROUP, "Save");
	} else {
		SetDlgItemText(IDC_SAVEGROUP, "Rename");
	}
}

bool CGroups::IsCurrentGroupModified()
{
	if (m_bGroupModified) {
		if ((m_nCurGroupID == GROUP_ID_NEW) && (m_selected->GetRowCount() == 0)) {
			// Special case, it's a "new group" and they're nothing in it so it hasn't really been modified
			return false;
		}

		// Has been modified
		return true;
	} else {

		// Has not been modified
		return false;
	}
}

bool CGroups::IsUserDefinedGroup(long nGroupId)
{
	if (nGroupId > 0) {
		// Greater than zero indicates a user-defined group
		return true;
	} else {
		// Less than or equal to zero indicates a system-defined group
		return false;
	}
}

void CGroups::RefreshGroupAccess(long nGroupId)
{
	// You can never delete system-defined groups and you can always delete user-defined groups
	if (IsUserDefinedGroup(nGroupId)) {
		GetDlgItem(IDC_DELETE)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
	}

	if (nGroupId == GROUP_ID_NEW) {
		if (m_selected->GetRowCount() > 0) {
			GetDlgItem(IDC_NEW_GROUP_BTN)->EnableWindow(TRUE);
		} else {
			GetDlgItem(IDC_NEW_GROUP_BTN)->EnableWindow(FALSE);
		}
	} else {
		GetDlgItem(IDC_NEW_GROUP_BTN)->EnableWindow(GetCurrentUserPermissions(bioLWGroup) & (SPT___W________ANDPASS));
	}

	GetDlgItem(IDC_SAVEGROUP)->EnableWindow(TRUE);
	GetDlgItem(IDC_ADD)->EnableWindow(TRUE);
	GetDlgItem(IDC_ADD_ALL)->EnableWindow(TRUE);
	GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);
	GetDlgItem(IDC_REMOVE_ALL)->EnableWindow(TRUE);
}

bool CGroups::IsGroupReadOnly(long nGroupId)
{
	if (nGroupId <= 0 && nGroupId != GROUP_ID_NEW) {
		// All built-in groups except GROUP_ID_NEW are read-only groups
		return true;
	} else {
		// All user-defined groups (and also GROUP_ID_NEW which is build-in but refers to a NEW group) are modifiable
		return false;
	}
}

void CGroups::OnNewGroupBtn() 
{
	if(GetCurrentUserPermissions(bioLWGroup) & (SPT___W________ANDPASS)) {
		m_pGroupList->SetSelByColumn(eglcID, (long)GROUP_ID_NEW);
		OnSelChosenExistingGroupList(m_pGroupList->CurSel);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//  Filters 5.0
////////////////////////

// Pass the Filter ID of the filter to be selected after refilling (or -1 to select the "All Patients..." filter)
UINT CGroups::FillFilterCombo(long nSelectId, LPCTSTR strUnsavedString /*= NULL*/)
{
	CWaitCursor wc;	

	// Prepare variables
	int nIndex = 0;
	try {
		//TES 2/22/2007 - PLID 20642 - First, requery from data.
		CString strWhere = FormatString("Type = %li", m_bAppointmentMerge ? fboAppointment : fboPerson);
		m_pFilterList->WhereClause = _bstr_t(strWhere);
		m_pFilterList->Requery();

		//TES 2/22/2007 - PLID 20642 - Obviously it would be better if this weren't synchronous, but this function has
		// always been synchronous, so that's a project for another time.
		m_pFilterList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);;
		
		// Add the built-in entries
		if (m_bAppointmentMerge) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFilterList->GetNewRow();
			pRow->PutValue(eflcID, (long)FILTER_ID_NEW);
			pRow->PutValue(eflcName, _bstr_t("{New Filter}"));
			m_pFilterList->AddRowAtEnd(pRow, NULL);
		}
		else {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFilterList->GetNewRow();
			pRow->PutValue(eflcID, (long)FILTER_ID_ALL);
			pRow->PutValue(eflcName, _bstr_t("{All " + GetStyleName(m_nStyle, FALSE) + "}"));
			m_pFilterList->AddRowAtEnd(pRow, NULL);
			
			pRow = m_pFilterList->GetNewRow();
			pRow->PutValue(eflcID, (long)FILTER_ID_NEW);
			pRow->PutValue(eflcName, _bstr_t("{New Filter}"));
			m_pFilterList->AddRowAtEnd(pRow, NULL);
		}
 
		if (strUnsavedString) {
			// We need to remember the filter where clause if it's specified because it's not stored in data anywhere
			m_strUnsavedFilterString = strUnsavedString;
		}

		// Set the current filter to be what was specified above or default
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFilterList->FindByColumn(eflcID, nSelectId, 0, VARIANT_FALSE);
		if(pRow == NULL) {
			//Our selected ID wasn't valid (possibly it just got deleted).  Use the default.
			m_nCurFilterID = m_bAppointmentMerge ? FILTER_ID_NEW : FILTER_ID_ALL;
		}
		else {
			m_nCurFilterID = nSelectId;
		}
		m_pFilterList->SetSelByColumn(eflcID, m_nCurFilterID);
	} NxCatchAll("CGroups::FillFilterCombo");
	
	return m_pFilterList->GetRowCount();
}

// Refreshes the data so as to ensure that it is consistent with the specified filter ID (-1 indicates NEW GROUP)
bool CGroups::ChangeFilterSelection(long nFilterId, bool bAutoRefreshListings /*= true*/)
{
	// Open a block here so the waitcursor goes out of scope prior to the call to refreshcurrentlistings
	{
		CWaitCursor wc;
		BOOL bResult;

		// Remember this filter for next time
		if (nFilterId != FILTER_ID_NEW) {
			SetFilterIdLast(nFilterId, m_bAppointmentMerge);
		}
		
		// Now make sure the filter listing is accurate
		CString strFrom, strWhere;
		CString strAdditionalWhere = GetDefaultWhereClause(VarLong(m_dlTypeOfPerson->GetValue(m_dlTypeOfPerson->CurSel, 0)), fboPerson);
		if (nFilterId == FILTER_ID_ALL) { // No filter (i.e. ALL)
			// That's easy, we want to select all!
			strWhere = "";	
			if (m_bAppointmentMerge)
				strFrom = "";
			else
				strFrom = BuildBaseFromClause(VarLong(m_dlTypeOfPerson->GetValue(m_dlTypeOfPerson->CurSel, 0)));
		} else if (nFilterId == FILTER_ID_NEW) { // A new filter
			if (!m_strUnsavedFilterString.IsEmpty()) {
				try {
					// This only works if there is a temporary filter specified
					if (m_bAppointmentMerge)
						bResult = CFilter::ConvertFilterStringToClause(FILTER_ID_NEW, m_strUnsavedFilterString, fboAppointment, &strWhere, &strFrom);
					else
						bResult = CFilter::ConvertFilterStringToClause(FILTER_ID_NEW, m_strUnsavedFilterString, fboPerson, &strWhere, &strFrom);
					if (bResult) {
						// (a.walling 2007-11-07 12:52) - PLID 27998 - VS2008 - DEBUG is an MFC definition, so we will call this DEBUGCODE
						DEBUGCODE(Log("SELECT ID FROM %s WHERE %s", strFrom, strWhere));
					} else {
						MsgBox("The current filter cannot be parsed.  Please edit the filter to correct the problem.");
						return false;
					}
				} NxCatchAllCall("CGroups::ChangeFilterSelection", return false);
			} else {
				// When there isn't a temporary filter, we default to ALL
				strWhere = "";
				if (m_bAppointmentMerge)
					strFrom = "";
				else
					strFrom = BuildBaseFromClause(VarLong(m_dlTypeOfPerson->GetValue(m_dlTypeOfPerson->CurSel, 0)));

			}
		} else { // A specified filter
			try {
				// We want only a single filter
				_RecordsetPtr prs = CreateRecordset("SELECT Name, Filter FROM FiltersT WHERE ID = %li", nFilterId);
				if (!prs->eof) {
					// Get the filter SQL string to filter on
					CString strFilter = AdoFldString(prs, "Filter");
					
					// Grab only the WHERE clause from the filter string
					if (m_bAppointmentMerge)
						bResult = CFilter::ConvertFilterStringToClause(nFilterId, strFilter, fboAppointment, &strWhere, &strFrom, NULL, NULL, TRUE);
					else
						bResult = CFilter::ConvertFilterStringToClause(nFilterId, strFilter, fboPerson, &strWhere, &strFrom, NULL, NULL, TRUE);
					if (bResult) {
						// (a.walling 2007-11-07 12:52) - PLID 27998 - VS2008 - DEBUG is an MFC definition, so we will call this DEBUGCODE
						DEBUGCODE(Log("SELECT ID FROM %s WHERE %s", strFrom, strWhere));
					} else {
						// (c.haag 2003-10-02 10:14) - We need to tell the user that this filter is
						// invalid. The thing is, we can reach this point at OnInitDialog, which is a place
						// where we don't want the filter to be edited because the view didn't draw yet.
						// So, we will instead post a message.
						PostMessage(NXM_INVALID_FILTER, nFilterId, (LPARAM)AdoFldString(prs, "Name","").AllocSysString());
						return false;
					}
				} else {
					// Failure
					HandleException(NULL, "Filter Error 200: Could not change to the new filter!", __LINE__, __FILE__);
					return false;
				}
			} NxCatchAllCall("CGroups::ChangeFilterSelection", return false);
		}
		
		try {
			// Set the datalist to have the filter criteria
			if (m_bAppointmentMerge) {
				if (strWhere.IsEmpty()) {
					m_apptUnselected->WhereClause = _bstr_t("PersonT.ID < -25 ");
				}
				else {
					m_apptUnselected->WhereClause = _bstr_t("AppointmentsT.ID IN (SELECT DISTINCT AppointmentsT.ID FROM " + strFrom + " WHERE " + strWhere + ") AND PersonT.ID <> -25 AND PatientsT.CurrentStatus <> 4 ");				
				}
			}
			else {
				m_unselected->FromClause = _bstr_t(BuildBaseFromClause(VarLong(m_dlTypeOfPerson->GetValue(m_dlTypeOfPerson->CurSel, 0))));
				if(strWhere.IsEmpty()) {
					m_unselected->WhereClause = _bstr_t("PersonT.ID IN (SELECT DISTINCT PersonT.ID FROM " + strFrom + ") AND " + strAdditionalWhere);
				}
				else {
					m_unselected->WhereClause = _bstr_t("PersonT.ID IN (SELECT DISTINCT PersonT.ID FROM " + strFrom + " WHERE " + strWhere + ") AND " + strAdditionalWhere);
				}
			}
		} NxCatchAllCall("CGroups::ChangeFilterSelection", return false);

		// Enable and disable various buttons, depending on what's available to the user for the given group
		RefreshFilterAccess(nFilterId);
	}
	
	// Reflect new group on screen
	if (bAutoRefreshListings) {
		RefreshCurrentListings(false, true);
	}

	// We're done
	return true;
}

void CGroups::RefreshFilterAccess(long nFilterId)
{
	// You can never delete built-in filters and you can always delete user-defined filters
	// Furthermore, built-in filters are read-only and user-defined filters are not
	if (IsUserDefinedFilter(nFilterId) || (nFilterId == FILTER_ID_NEW)) {
		GetDlgItem(IDC_DELETEFILTER)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_FILTERS)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_DELETEFILTER)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_FILTERS)->EnableWindow(FALSE);
	}
}

bool CGroups::IsUserDefinedFilter(long nFilterId)
{
	if (nFilterId > 0) {
		return true;
	} else {
		return false;
	}
}

bool CGroups::OnSelectedChanged()
{
	if (m_nCurGroupID == GROUP_ID_CURRENT_PATIENT) {
		ChangeGroupSelection(GROUP_ID_CURRENT_PATIENT);
		return true;
	} else {
		return false;
	}
}

LRESULT CGroups::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) {
	case NXM_GROUP_RESIZECOLUMNS:
//		m_unselected.ResizeColumns();
//		m_selected.ResizeColumns();
		// TODO: when the datalist supports resizing columns, do it here
		return 0;
		break;
	case NXM_SELECTED_CHANGED:
		return OnSelectedChanged();
		break;
	default:
		return CNxDialog::WindowProc(message, wParam, lParam);
		break;
	}
}

void CGroups::OnNewFilterBtn() 
{
	if (!UserPermission(EditFilter))
		return;

	if (m_bAppointmentMerge) {
		CMenu mActions;
		long nIndex = 0;
		if(!mActions.GetSafeHmenu()) 
			mActions.m_hMenu = CreatePopupMenu();
		mActions.InsertMenu(nIndex++, MF_BYPOSITION, ID_APPOINTMENT_SELECTOR_NEW, "Create New using &Appointment Selector");
		mActions.InsertMenu(nIndex++, MF_BYPOSITION, ID_FILTER_EDITOR_NEW, "Create New using &Filter Editor");

		if(mActions.GetSafeHmenu()) {
			//OK, we have at least one option, so let's show the menu.
			CRect rBtn;
			GetDlgItem(IDC_NEW_FILTER_BTN)->GetWindowRect(rBtn);
			mActions.TrackPopupMenu(TPM_LEFTALIGN, rBtn.right, rBtn.top, this, NULL);
		}
	}
	else
		FilterEditorDlg(TRUE);
}

void CGroups::OnFilterEditorNew()
{
	FilterEditorDlg(TRUE);
}

void CGroups::OnFilterEditorEdit()
{
	FilterEditorDlg();
}

int CGroups::FilterEditorDlg(BOOL bNewFilter /*= FALSE*/) 
{	
	CFilterEditDlg dlg(NULL, m_bAppointmentMerge ? fboAppointment : fboPerson, IsActionSupported, CommitSubfilterAction, NULL, m_bAppointmentMerge ? "Appointment Filter" : "Patient Filter");
	
	int nResult = 0;
	if (bNewFilter)
		nResult = dlg.NewFilter();
	else 
		nResult = dlg.EditFilter(m_nCurFilterID, m_strUnsavedFilterString);

	if (nResult == IDOK) {
		FillFilterCombo(dlg.GetFilterId(), dlg.m_strFilterString);
		ChangeFilterSelection(m_nCurFilterID);
	}

	return nResult;
}

void CGroups::OnEditFilters() 
{
	if (!UserPermission(EditFilter))
		return;

	if (m_bAppointmentMerge) {
		CMenu mActions;
		long nIndex = 0;
		if(!mActions.GetSafeHmenu()) 
			mActions.m_hMenu = CreatePopupMenu();
		mActions.InsertMenu(nIndex++, MF_BYPOSITION, ID_APPOINTMENT_SELECTOR_EDIT, "Edit using &Appointment Selector");
		mActions.InsertMenu(nIndex++, MF_BYPOSITION, ID_FILTER_EDITOR_EDIT, "Edit using &Filter Editor");

		if(mActions.GetSafeHmenu()) {
			//OK, we have at least one option, so let's show the menu.
			CRect rBtn;
			GetDlgItem(IDC_EDIT_FILTERS)->GetWindowRect(rBtn);
			mActions.TrackPopupMenu(TPM_LEFTALIGN, rBtn.right, rBtn.top, this, NULL);
		}
	}
	else
		FilterEditorDlg();
}

void CGroups::OnSaveFiltersBtn() 
{
	if (!UserPermission(EditFilter))
		return;
	long nOldFilterId = m_nCurFilterID;
	long nNewFilterId = DoSaveCurrentFilter(nOldFilterId, FILTER_ID_UNSPECIFIED);
	if (nNewFilterId > 0) {
		if (nOldFilterId != nNewFilterId) {
			ChangeFilterSelection(nNewFilterId);
		}
	} else if (nNewFilterId == FILTER_ID_SAVE_CANCELED) {
		// Do nothing
	} else {
		MsgBox("The currently selected filter cannot be saved at this time.");
	}
}

// Returns the Filter ID of the saved group or 0 for failure
long CGroups::DoSaveCurrentFilter(long nSaveFilterId, long nSelectDifferentId /* = FILTER_ID_UNSPECIFIED */)
{
	long nAns = FILTER_ID_UNSPECIFIED;

	if (nSaveFilterId == FILTER_ID_NEW) {
		// This is a new filter
		CString strFilterName = "Untitled Filter";
		if (Prompt("Enter the name for this filter:", strFilterName, 50) == IDOK) {
			try {

				if(strFilterName.GetLength() > 50) {
					AfxMessageBox("The name you entered is greater than the maximum length (50), please shorten it.");
					nAns = FILTER_ID_SAVE_CANCELED;
				}
				else {
					// Get an appropriate new filter
					long nNewFilterId = NewNumber("FiltersT", "ID");

					// Create and execute the append query to add the filter to the table
					ExecuteSql("INSERT INTO FiltersT (ID, Name, Filter, Type) VALUES (%li, '%s', '%s', %li), ", 
						nNewFilterId, _Q(strFilterName), m_strUnsavedFilterString, fboPerson);

					//auditing
					long nAuditID = BeginNewAuditEvent();
					AuditEvent(-1, "", nAuditID, aeiFilterCreated, -1, "", strFilterName, aepMedium, aetCreated);
		
					// We will return success
					nAns = nNewFilterId;
					m_filterChecker.Refresh(nNewFilterId);
				}
			} NxCatchAllCall("CGroups::DoSaveCurrentFilter", nAns = FILTER_ID_UNSPECIFIED);
		} else {
			nAns = FILTER_ID_SAVE_CANCELED;
		}
	} else if (nSaveFilterId > 0) { // User-defined
		CString strFilterName, strOld;
		NXDATALIST2Lib::IRowSettingsPtr pSavedFilterRow = m_pFilterList->FindByColumn(eflcID, nSaveFilterId, NULL, VARIANT_FALSE);
		strFilterName = VarString(pSavedFilterRow->GetValue(eflcName));
		strOld = strFilterName;
		if (Prompt("Enter the name for this filter:", strFilterName, 50) == IDOK) {
			// This is not a new filter but the user may want to change its name
			try {

				//check to make sure that is name isn't already taken
				if (! ReturnsRecords("SELECT ID FROM FiltersT WHERE Name = '%s' AND ID <> %li", _Q(strFilterName), nSaveFilterId)) {

					// Name the filter
					//TES 3/26/2007 - PLID 20528 - Update the ModifiedDate.
					ExecuteSql("UPDATE FiltersT SET Name = '%s', ModifiedDate = getdate() WHERE ID = %li", _Q(strFilterName), nSaveFilterId);

					//auditing
					long nAuditID = BeginNewAuditEvent();
					AuditEvent(-1, "", nAuditID, aeiFilter, -1, strOld, strFilterName, aepMedium, aetChanged);

					// We will return success
					nAns = nSaveFilterId;
					m_filterChecker.Refresh(nSaveFilterId);
				}
				else {
					//give them a message box and tell them that the name already exists
					CString strMsg;
					strMsg.Format("There is already a filter named '%s'.  Please choose a different name for this filter.", _Q(strFilterName));
					MsgBox(strMsg);
					nAns = FILTER_ID_SAVE_CANCELED;
				}
					
			} NxCatchAllCall("CGroups::DoSaveCurrentFilter", nAns = FILTER_ID_UNSPECIFIED);
		} else {
			nAns = FILTER_ID_SAVE_CANCELED;
		}
	} else {
		MsgBox("The filter is neither a new filter, nor a user-defined filter.");
		// We will return failure
		nAns = FILTER_ID_UNSPECIFIED;
	}

	// Refresh the combo box (even if there were errors because we want to reflect changes made by remote users)
	long nSelectId;
	if (nSelectDifferentId != FILTER_ID_UNSPECIFIED) {
		nSelectId = nSelectDifferentId;
	} else if (nAns != FILTER_ID_UNSPECIFIED) {
		nSelectId = nAns;
	} else {
		nSelectId = m_nCurFilterID;
	}
	
	if (nSelectId != FILTER_ID_SAVE_CANCELED) {
		FillFilterCombo(nSelectId);
	}

	// Return the result
	return nAns;
}

void CGroups::OnDeleteFilter() 
{
	if (!UserPermission(EditFilter))
		return;

	CString strOld;
	long nFilterId = m_nCurFilterID;

	if (nFilterId > 0) {
		// (c.haag 2003-10-01 16:07) - See if this filter is in other filters.
		CString strFilter;
		CString strWhere, strDeletingWhere, strFrom;

		// Get the SQL for this filter
		_RecordsetPtr prsFilters = CreateRecordset("SELECT ID, Name, Filter FROM FiltersT WHERE ID = %d", nFilterId);
		if (prsFilters->eof)
		{
			MsgBox("This filter has already been deleted by another user.");
			return;
		}
		strOld = AdoFldString(prsFilters, "Name");
		strFilter = AdoFldString(prsFilters, "Filter");
		CFilter::ConvertFilterStringToClause(nFilterId, strFilter, fboPerson, &strDeletingWhere, &strFrom);
		prsFilters->Close();
		prsFilters.Release();

		// Now compare it to other filters in the data
		CString strFilters = "This filter is used within the following filters:\r\n\r\n";
		prsFilters = CreateRecordset("SELECT ID, Name, Filter FROM FiltersT WHERE Filter LIKE '%%\"%d\"%%' AND ID <> %d", nFilterId, nFilterId);
		BOOL bInFilters = FALSE;
		while (!prsFilters->eof)
		{
			// Get the filter SQL string to filter on
			strFilter = AdoFldString(prsFilters, "Filter");
			
			// Grab only the WHERE clause from the filter string
			if (CFilter::ConvertFilterStringToClause(AdoFldLong(prsFilters, "ID"), strFilter, fboPerson, &strWhere, &strFrom))
			{
				if (-1 != strWhere.Find(strDeletingWhere, 0))
				{
					bInFilters = TRUE;
					strFilters += AdoFldString(prsFilters, "Name", "") + "\r\n";
					break;
				}
			}
			prsFilters->MoveNext();
		}
		prsFilters->Close();
		if (bInFilters)
		{
			strFilters += "\r\nThese filters must be changed before you can delete this group.";
			if (AfxMessageBox(strFilters))
				return;
		}

		// (j.jones 2005-09-15 14:16) - first see if the filter is used in any export
		if(!CheckAllowDeleteFilter(nFilterId)) {
			return;
		}

		if (AfxMessageBox("Are you sure you want to permanently delete this filter?", MB_YESNO|MB_ICONQUESTION) == IDYES) {
			if (!DoDeleteFilter(nFilterId)) {
				MsgBox("The currently selected filter cannot be deleted at this time.");
			}

			//auditing
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiFilterDeleted, -1, strOld, "<Deleted>", aepMedium, aetDeleted);
 		}
	} else {
		// This is an error condition because the save button should be disabled if the filter is not modified or if it's read-only
		ASSERT(FALSE);
		MsgBox("The currently selected filter is read-only and cannot be deleted.");
	}
}

bool CGroups::DoDeleteFilter(long nDeleteFilterId)
{
	try {
		CString strSql;
		
		// (v.maida 2014-12-4 14:54) - PLID 58276 - Determine which ConfigRT filter property we will need to delete.
		CString strPropName = m_bAppointmentMerge ? "ApptFilterIDPatientLast" : "FilterIDPatientLast";

		// Run the DELETE query to delete only the specified filter
		// (c.haag 2010-09-22 16:06) - PLID 40629 - Delete related records too
		// (v.maida 2014-12-4 14:54) - PLID 58276 - Delete the ConfigRT entry containing the filter id, or else the patient filters and appointment filters can be mismatched.
		ExecuteSql(
			"DELETE FROM EducationTemplatesT WHERE FilterID = %i\r\n"
			"DELETE FROM ConfigRT WHERE Name = '%s' AND Number = 0 AND IntParam = %i\r\n"
			"DELETE FROM FiltersT WHERE ID = %i"
			,nDeleteFilterId, strPropName, nDeleteFilterId, nDeleteFilterId);

		// If deletion was successful prepare for a new Filter
		FillFilterCombo(GetFilterIdLast());
		ChangeFilterSelection(m_nCurFilterID);

		//update the filter checker
		CClient::RefreshTable(NetUtils::FiltersT, nDeleteFilterId);

		// Return success
		return true;
	} NxCatchAllCall("CGroups::DoDeleteFilter", return false);
}

BOOL WINAPI CGroups::CheckAllowDeleteFilter(long nDeleteFilterId)
{
	// (j.jones 2005-09-15 14:16) - first see if the filter is used in any export
	BOOL bAllowDelete = TRUE;
	CString strFilterNames;
	_RecordsetPtr rs = CreateRecordset("SELECT Name FROM ExportT WHERE LetterWritingFilterID = %li", nDeleteFilterId);
	while(!rs->eof) {
		bAllowDelete = FALSE;
		strFilterNames += AdoFldString(rs, "Name","");
		strFilterNames += "\n";
		rs->MoveNext();
	}
	rs->Close();

	if(!bAllowDelete) {
		CString strWarn;
		strWarn.Format("The filter cannot be deleted because it is used in the following Exports:\n\n"
			"%s\n"
			"You must remove references to this filter in the Export tab of the Financial module before you can delete it.",strFilterNames);
		AfxMessageBox(strWarn);		
		return false;
	}

	// (j.gruber 2010-02-25 13:01) - PLID 37537 - Clinical Decisions
	bAllowDelete = TRUE;	
	rs = CreateParamRecordset("SELECT Name, Inactive FROM DecisionRulesT  "
		" LEFT JOIN (SELECT RuleID, Value FROM DecisionRulesCriterionT WHERE Type = 4) DecisionRulesCriterionT ON  "
		" DecisionRulesT.ID = DecisionRulesCriterionT.RuleID "
		" WHERE CONVERT(int, DecisionRulesCriterionT.Value) = {INT} ", nDeleteFilterId);
	while(!rs->eof) {
		bAllowDelete = FALSE;
		strFilterNames += AdoFldString(rs, "Name","");
		//TES 11/27/2013 - PLID 59848 - Indicate which ones are inactive, to help the user find the rule if they want to
		if(AdoFldBool(rs, "Inactive", 0)) {
			strFilterNames += " (Inactive)";
		}
		strFilterNames += "\n";
		rs->MoveNext();
	}
	rs->Close();

	if(!bAllowDelete) {
		CString strWarn;
		strWarn.Format("The filter cannot be deleted because it is used in the following Clinical Decision Support Rule(s):\n\n"
			"%s\n"
			"You must remove references to this filter in Activities->EMR->Configure Clinical Decision Support Rules in the Administrator module before you can delete it.",strFilterNames);
		AfxMessageBox(strWarn);		
		return false;
	}
	else
		return true;
}

void CGroups::OnTimer(UINT nIDEvent) 
{
	switch (nIDEvent) {
	case NXT_CHECK_CURRENT_PATIENT:
		{
			long nCurSelId;
			if (m_nStyle & fbjPatient) {
				nCurSelId = GetActivePatientID();
			} else {
				nCurSelId = GetActiveContactID();
			}
			// Change the selection here if it was changed somewhere else
			if (m_nCurSelectionId != nCurSelId) {
				m_nCurSelectionId = nCurSelId;
				PostMessage(NXM_SELECTED_CHANGED);
			}
		}
		break;
	default:
		CNxDialog::OnTimer(nIDEvent);
		break;
	}
}

long CGroups::CheckAllowClose() 
{
	// First check to see if the current group has been modified
	if (IsCurrentGroupModified()) {
		switch (AfxMessageBox("The group for Person Based Merging has been modified.  Would you like to save the changes?", MB_YESNOCANCEL|MB_ICONQUESTION)) {
		case IDYES:
			{	
				// Save the changes and then do some manual processing before proceeding
				long nNewGroupId = DoSaveCurrentGroup(m_nCurGroupID);
				if (nNewGroupId != GROUP_ID_SAVE_CANCELED) {
					if (nNewGroupId > 0) {
						// Remember the "last" group id now that it's saved
						SetGroupIdLast(nNewGroupId);
					}
					return AC_CAN_CLOSE;
				} else {
					// Act as if they clicked cancel on the above question
					return AC_CANNOT_CLOSE;
				}
			}
			break;
		case IDNO:
			// Proceed without saving the changes
			return AC_CAN_CLOSE;
			break;
		case IDCANCEL:
		default:
			// Cancel everything
			return AC_CANNOT_CLOSE;
			break;
		}
	} else {
		return AC_CAN_CLOSE;
	}
}

void CGroups::PreClose()
{
	// First check to see if the current group has been modified
	if (IsCurrentGroupModified()) {
		switch (AfxMessageBox("The group for Person Based Merging has been modified.  Would you like to save the changes?", MB_YESNO|MB_ICONQUESTION)) {
		case IDYES:
			{	
				// Save the changes and then do some manual processing before proceeding
				long nNewGroupId = DoSaveCurrentGroup(m_nCurGroupID);
				if (nNewGroupId != GROUP_ID_SAVE_CANCELED) {
					if (nNewGroupId > 0) {
						// Remember the "last" group id now that it's saved
						SetGroupIdLast(nNewGroupId);
					}
				}
			}
			break;
		case IDNO:
		default:
			// Proceed without saving the changes
			break;
		}
	}
}

CString CGroups::GetGroupName(long nGroupId)
{
	if (nGroupId > 0) {
		CString str;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pGroupList->FindByColumn(eglcID, nGroupId, 0, VARIANT_FALSE);
		if(pRow != NULL) {
			str = VarString(pRow->GetValue(eglcName));
		}
		return str;
	} else {
		switch (nGroupId) {
		case GROUP_ID_CURRENT_PATIENT:
			if (m_nStyle & fbjPatient) {
				return "{Current Patient}";
			} else {
				return "{Current Contact}";
			}
			break;
		case GROUP_ID_NEW:
			return "New Group...";
			break;
		case GROUP_ID_CURRENT_LOOKUP:
			return "{Current Lookup}";
			break;
		case GROUP_ID_CURRENT_FILTER:
			return "{Current Filter}";
			break;
		case GROUP_ID_SAVE_CANCELED:
		case GROUP_ID_UNSPECIFIED:
		default:
			return "{Invalid Group}";
			break;
		}
	}
}

void CGroups::OnRequeryFinishedSelected(short nFlags)
{
	ReflectListCount(LWL_GROUP_LIST);
	EnableGroupAccess(TRUE);
	SetSelectedColumnSizes();
}

void CGroups::OnRequeryFinishedUnselected(short nFlags)
{
	// If we've been asked to refresh, let's do it
	if (m_bCurrentFilterGroupNeedsRefresh) {
		m_bCurrentFilterGroupNeedsRefresh = false;
		if (m_nCurGroupID == GROUP_ID_CURRENT_FILTER) {
			// Give immediate feedback
			CWaitCursor wc;
			// Transfer the filter contents to the group
			m_selected->Clear();
			SetCurrentGroupModified(false);
			long p = m_unselected->GetFirstRowEnum();
			while (p) {
				LPDISPATCH lpDisp;
				m_unselected->GetNextRowEnum(&p, &lpDisp);
				if (lpDisp) {
					m_selected->InsertRow(lpDisp, m_selected->GetRowCount());
					lpDisp->Release();
				}
			}
			m_selected->Sort();
			// Reflect the fact that we have an official group listing on screen now
			ReflectListCount(LWL_GROUP_LIST);
			EnableGroupAccess(TRUE);
		}
	}
	// Reflect the new information
	ReflectListCount(LWL_FILTER_LIST);
	EnableFilterAccess(TRUE);

	SetUnselectedColumnSizes();
}

void CGroups::OnRequeryFinishedApptSelected(short nFlags)
{
	ReflectListCount(LWL_GROUP_LIST);
	SetApptSelectedColumnSizes();
}

void CGroups::OnRequeryFinishedApptUnselected(short nFlags)
{
	ReflectListCount(LWL_FILTER_LIST);
	EnableFilterAccess(TRUE);
	SetApptUnselectedColumnSizes();
}

void CGroups::EnableGroupAccess(BOOL bEnable)
{
	EnableDlgItem(IDC_REMOVE_ALL, bEnable);
	CWnd *pParent = GetParent();
	if (pParent && pParent->IsKindOf(RUNTIME_CLASS(CLetterWriting))) {
		((CLetterWriting *)pParent)->GetDlgItem(IDC_WRITE_PACKET)->EnableWindow(bEnable);
		((CLetterWriting *)pParent)->GetDlgItem(IDC_WRITE_FORM)->EnableWindow(bEnable);
		((CLetterWriting *)pParent)->GetDlgItem(IDC_WRITE_LETTER)->EnableWindow(bEnable);
		((CLetterWriting *)pParent)->GetDlgItem(IDC_WRITE_ENVELOPE)->EnableWindow(bEnable);
		((CLetterWriting *)pParent)->GetDlgItem(IDC_WRITE_LABEL)->EnableWindow(bEnable);
		((CLetterWriting *)pParent)->GetDlgItem(IDC_WRITE_OTHER)->EnableWindow(bEnable);
	}
	
	//TES 3/15/2007 - PLID 24895 - This variable no longer seems to serve any useful purpose.
	//m_bGroupAccess = bEnable;

	if (!bEnable || !IsUserDefinedGroup(VarLong(m_pGroupList->CurSel->GetValue(eglcID)))) {
		EnableDlgItem(IDC_DELETE, FALSE);
		EnableDlgItem(IDC_SAVEGROUP, FALSE);
	} else {
		EnableDlgItem(IDC_DELETE, TRUE);
		EnableDlgItem(IDC_SAVEGROUP, TRUE);
	}
}

void CGroups::EnableFilterAccess(BOOL bEnable)
{
	EnableDlgItem(IDC_ADD_ALL, bEnable);
	//TES 3/15/2007 - PLID 24895 - This variable no longer seems to serve any useful purpose.
	//m_bFilterAccess = bEnable;

	// (z.manning, 02/26/2007) - PLID 24923 - We used to disable these buttons when disabling filter access,
	// but there is absolutely no reason to.
	if (!IsUserDefinedFilter(VarLong(m_pFilterList->CurSel->GetValue(eflcID)))) {
		//TES 2/20/2007 - PLID 24895 - Um, since this is the EnableFilterAccess function, shouldn't we mess with filter
		// buttons, not group buttons?  Yeah, that's what I thought.
		EnableDlgItem(IDC_EDIT_FILTERS, FALSE);
		EnableDlgItem(IDC_SAVE_FILTERS, FALSE);
		EnableDlgItem(IDC_DELETEFILTER, FALSE);
	} else {
		EnableDlgItem(IDC_EDIT_FILTERS, TRUE);
		EnableDlgItem(IDC_SAVE_FILTERS, TRUE);
		EnableDlgItem(IDC_DELETEFILTER, TRUE);
	}
}

// Not using "Refresh" because NxDialog uses it differently
void CGroups::DoRefresh()
{
	/*m.hancock - 5/5/2006 - PLID 20352 - Speed improvement: the merge group datalist 
	in Letter Writing requeries when you change the patient using the patient toolbar.
	It should only need to refresh if you have "Current Patient" selected.  We always 
	want to refresh the filter results, but only refresh the group if 
	"Current Patient" is selected.*/
	long nGroupId = m_nCurGroupID;
	if(nGroupId == GROUP_ID_CURRENT_PATIENT) {
		// Refresh the group and the filter
		RefreshCurrentListings(true, true);
	}
	else {
		// Only refresh the filter
		RefreshCurrentListings(false, true);
	}

	//table checker for groups - this should do filters as well
	if(m_groupChecker.Changed()) {
		FillGroupCombo(nGroupId);
	}

	//FOR PLID 17069 - adding filter checker
	long nFilterID = m_nCurFilterID;
	if (m_filterChecker.Changed()) {
		FillFilterCombo(nFilterID);
	}

	if (m_bAppointmentMerge)
		RepositionControls();
}

// (j.jones 2016-04-15 09:16) - NX-100214 - no longer need OnCtlColor
//HBRUSH CGroups::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)

void CGroups::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	//JMJ 12/3/2003 - it should always refresh
	DoRefresh();

	/*
	if(m_groupChecker.Changed())
	{
		long nGroupId = m_cboGroupList.GetItemData(m_nCurGroupIndex);
		FillGroupCombo(nGroupId);
		RefreshCurrentListings(true, true);

		//do the filters as well
		FillFilterCombo(nGroupId);
	}
	*/
}

CString CGroups::GetDefaultWhereClause(long nBaseJoinFlags, long fboFilterBase)
{
	switch (fboFilterBase) {
	case fboEMN:
	case fboEMR:
		return "";
		break;

	case fboPerson:
		{
			//DRT - 12/18/01 - if we want to put the Patients ID in the patient sections, we need to have something here in the others
			CString strWhere = "";
			
			// Create the joins and the corresponding where clause
			if (nBaseJoinFlags & fbjPatient) {
				if (!strWhere.IsEmpty()) strWhere += " OR";
				//DRT 4/15/2004 - Hide inquiries
				// (b.cardillo 2007-03-13 14:06) - PLID 25123 - Included the NOLOCK hint on the PatientsT 
				// table, as this is a frequently run query that doesn't need the lock.
				strWhere += " PersonT.ID IN (SELECT PersonID FROM PatientsT WITH (NOLOCK) WHERE CurrentStatus <> 4)";
			}
			if (nBaseJoinFlags & fbjContact) {
				if (!strWhere.IsEmpty()) strWhere += " OR";
				strWhere += " PersonT.ID IN (SELECT PersonID FROM ContactsT)";
			}
			if (nBaseJoinFlags & fbjProvider) {
				if (!strWhere.IsEmpty()) strWhere += " OR";
				strWhere += " PersonT.ID IN (SELECT PersonID FROM ProvidersT)";
			}
			if (nBaseJoinFlags & fbjRefPhys) {
				if (!strWhere.IsEmpty()) strWhere += " OR";
				strWhere += " PersonT.ID IN (SELECT PersonID FROM ReferringPhysT)";
			}
			if (nBaseJoinFlags & fbjSupplier) {
				if (!strWhere.IsEmpty()) strWhere += " OR";
				strWhere += " PersonT.ID IN (SELECT PersonID FROM SupplierT)";
			}
			if (nBaseJoinFlags & fbjInsParty) {
				if (!strWhere.IsEmpty()) strWhere += " OR";
				strWhere += " PersonT.ID IN (SELECT PersonID FROM InsuredPartyT)";
			}
			if (nBaseJoinFlags & fbjInsCo) {
				if (!strWhere.IsEmpty()) strWhere += " OR";
				strWhere += " PersonT.ID IN (SELECT PersonID FROM InsuranceCoT)";
			}
			if (nBaseJoinFlags & fbjUser) {
				if (!strWhere.IsEmpty()) strWhere += " OR";
				strWhere += " PersonT.ID IN (SELECT PersonID FROM UsersT WHERE UsersT.PersonID > 0)";
			}
			if (nBaseJoinFlags & fbjRefSource) {
				if (!strWhere.IsEmpty()) strWhere += " OR";
				strWhere += " PersonT.ID IN (SELECT PersonID FROM ReferralSourceT)";
			}

			// Add the where clause to the joins
			if (!strWhere.IsEmpty()) {
				strWhere = " PersonT.ID > 0 AND (" + strWhere + ")";
			}
			else {
				strWhere = " PersonT.ID > 0 ";
			}
			return strWhere;
		}
		break;
	default:
		//Invalid fboBasedOn type.
		ASSERT(FALSE);
		return "";
		break;
	}
}

CString CGroups::BuildBaseFromClause(long nBaseJoinFlags)
{
	//DRT - 12/18/01 - if we want to put the Patients ID in the patient sections, we need to have something here in the others
	CString strBaseJoin = "(SELECT PersonT.*, 0 AS UserDefinedID FROM PersonT WITH (NOLOCK)";
	
	// Create the joins and the corresponding where clause
	if (nBaseJoinFlags & fbjPatient) {
		// (b.cardillo 2007-03-13 14:06) - PLID 25123 - Included the NOLOCK hint on the PatientsT 
		// table, as this is a frequently run query that doesn't need the lock.
		strBaseJoin = "(SELECT PersonT.*, PatientsT.UserDefinedID FROM PersonT WITH (NOLOCK)";
		strBaseJoin += " LEFT JOIN PatientsT WITH (NOLOCK) ON PersonT.ID = PatientsT.PersonID";
	}
	if (nBaseJoinFlags & fbjContact) {
		strBaseJoin += " LEFT JOIN ContactsT WITH (NOLOCK) ON PersonT.ID = ContactsT.PersonID";
	}
	if (nBaseJoinFlags & fbjProvider) {
		strBaseJoin += " LEFT JOIN ProvidersT WITH (NOLOCK) ON PersonT.ID = ProvidersT.PersonID";
	}
	if (nBaseJoinFlags & fbjRefPhys) {
		strBaseJoin += " LEFT JOIN ReferringPhysT WITH (NOLOCK) ON PersonT.ID = ReferringPhysT.PersonID";
	}
	if (nBaseJoinFlags & fbjSupplier) {
		strBaseJoin += " LEFT JOIN SupplierT WITH (NOLOCK) ON PersonT.ID = SupplierT.PersonID";
	}
	if (nBaseJoinFlags & fbjInsParty) {
		strBaseJoin += " LEFT JOIN InsuredPartyT WITH (NOLOCK) ON PersonT.ID = InsuredPartyT.PersonID";
	}
	if (nBaseJoinFlags & fbjInsCo) {
		strBaseJoin += " LEFT JOIN InsuranceCoT WITH (NOLOCK) ON PersonT.ID = InsuranceCoT.PersonID";
	}
	if (nBaseJoinFlags & fbjUser) {
		strBaseJoin += " LEFT JOIN (SELECT * FROM UsersT WITH (NOLOCK) WHERE UsersT.PersonID > 0) UsersT ON PersonT.ID = UsersT.PersonID";
	}
	if (nBaseJoinFlags & fbjRefSource) {
		strBaseJoin += " LEFT JOIN ReferralSourceT WITH (NOLOCK) ON PersonT.ID = ReferralSourceT.PersonID";
	}

	// Finalize the FROM string and name the subquery PersonT 
	// so that everything that refers to it actually works
	strBaseJoin += ") PersonT";
	return strBaseJoin;
}

BOOL WINAPI CGroups::IsActionSupported(SupportedActionsEnum saAction, long nFilterType)
{
	switch(nFilterType) {
	case fboEMN:
	case fboEMR:
	case fboPerson:
	case fboAppointment:
	case fboTodo:
	case fboPayment:
	case fboMedication: //(r.wilson 10/21/2013) PLID 59121 - PatientLists
	case fboAllergy: //(r.wilson 10/21/2013) PLID 59121 - PatientLists
	case fboLab: //(r.wilson 10/21/2013) PLID 59121 - PatientLists
	case fboLabResult: //TES 9/9/2010 - PLID 40457
	case fboImmunization: //TES 9/9/2010 - PLID 40470
	case fboEmrProblem: //TES 9/9/2010 - PLID 40471
		switch(saAction) {
		case saAdd:
		case saEdit:
		case saDelete:
			return TRUE;
			break;
		default:
			ASSERT(FALSE);
			return FALSE;
		}
		break;
	default:
		ASSERT(FALSE);
		return FALSE;
	}
}

BOOL WINAPI CGroups::CommitSubfilterAction(SupportedActionsEnum saAction, long nFilterType, long &nID, CString &strName, CString &strFilter, CWnd *pParentWnd)
{
	switch(nFilterType) {
	case fboEMN:
	case fboEMR:
	case fboPerson:
	case fboAppointment:
	case fboTodo:
	case fboPayment:
	case fboMedication: //(r.wilson 10/21/2013) PLID 59121 - PatientLists
	case fboAllergy: //(r.wilson 10/21/2013) PLID 59121 - PatientLists
	case fboLab: //(r.wilson 10/21/2013) PLID 59121 - PatientLists
	case fboLabResult: //TES 9/9/2010 - PLID 40457
	case fboImmunization: //TES 9/9/2010 - PLID 40470
	case fboEmrProblem: //TES 9/9/2010 - PLID 40471
		switch(saAction) {
		case saAdd:
			//OK, let's save this filter.
			try {
				nID = NewNumber("FiltersT", "ID");
				int nReturn = IDOK;
				while(nReturn != IDCANCEL && (strName.GetLength() > 50 || ReturnsRecords("SELECT ID FROM FiltersT WHERE Name = '%s'", _Q(strName))) ){
					CString strMessage;
					if(strName.GetLength() > 50) {
						strMessage = "You cannot have a filter name longer than 50 characters.";
					}
					else {
						strMessage.Format("There is already a filter named '%s'.  Please choose a different name for this filter.", strName);
					}
					MsgBox(strMessage);
					nReturn = InputBoxLimited(NULL, "Enter new name", strName, "",50,false,false,NULL);
				}
				if(nReturn == IDCANCEL) return FALSE;

				ExecuteSql("INSERT INTO FiltersT (ID, Name, Filter, Type) VALUES (%li, '%s', '%s', %li)", nID, _Q(strName), _Q(strFilter), nFilterType);
				return TRUE;
			}NxCatchAllCall_NoParent("Error 250 in CommitSubfilterAction", return FALSE;); // (a.walling 2014-05-05 13:32) - PLID 61945
			break;

		case saEdit:
			try {
				int nReturn = IDOK;
				while(nReturn != IDCANCEL && (strName.GetLength() > 50 || ReturnsRecords("SELECT ID FROM FiltersT WHERE Name = '%s' AND ID <> %li", _Q(strName), nID)) ){
					CString strMessage;
					if(strName.GetLength() > 50) {
						strMessage = "You cannot have a filter name longer than 50 characters.";
					}
					else {
						strMessage.Format("There is already a filter named '%s'.  Please choose a different name for this filter.", strName);
					}
					MsgBox(strMessage);
					nReturn = InputBoxLimited(NULL, "Enter new name", strName, "",50,false,false,NULL);
				}
				if(nReturn == IDCANCEL) return FALSE;

				//TES 3/26/2007 - PLID 20528 - Update the ModifiedDate.
				ExecuteSql("UPDATE FiltersT SET Name = '%s', Filter = '%s', Type = %li, ModifiedDate = getdate() WHERE ID = %li", _Q(strName), _Q(strFilter), nFilterType, nID);
				return TRUE;
			}NxCatchAllCall_NoParent("Error 350 in CommitSubfilterAction", return FALSE;);
			break;

		case saDelete:
			try {
				if(IDYES == MsgBox(MB_YESNO, "Are you sure you want to delete the filter '%s'?", strName)) {
					if(CheckAllowDeleteFilter(nID)) {
						// (v.maida 2014-12-4 14:54) - PLID 58276 - Determine which ConfigRT filter property we will need to delete. We only care about patient/appointment types getting mismatched.
						CString strPropName = "-99NotARealFilterName-99"; // default to an invalid property name, so that, if we're not deleting a patient or appt filter, the ConfigRT delete query will not delete anything.
						if (nFilterType == 1) // patient subfilter being deleted, so we want to delete patient ConfigRT filter entries
						{
							strPropName = "FilterIDPatientLast";
						}
						else if (nFilterType == 3) // appointment subfilter being deleted, so we want to delete appointment ConfigRT filter entries
						{
							strPropName = "ApptFilterIDPatientLast";
						}
						// (c.haag 2010-09-22 16:06) - PLID 40629 - Delete related records too
						ExecuteSql(
							"DELETE FROM EducationTemplatesT WHERE FilterID = %li\r\n"
							"DELETE FROM ConfigRT WHERE Name = '%s' AND Number = 0 AND IntParam = %li\r\n" // (v.maida 2014-12-4 14:54) - PLID 58276 - Delete the appropriate ConfigRT filter entries to prevent filter mismatching.
							"DELETE FROM FiltersT WHERE ID = %li"
							,nID, strPropName, nID, nID);
						return TRUE;
					}
					else {
						return FALSE;
					}
				}
				else {
					return FALSE;
				}
			}NxCatchAllCall_NoParent("Error 450 in CommitSubfilterAction", return FALSE;);
			break;

		default:
			ASSERT(FALSE);
			return FALSE;
		}
		break;
	default:
		ASSERT(FALSE);
		return FALSE;
	}
}

void CGroups::OnColumnSizingFinishedSelected(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth) 
{
	//uncommitted
	if(!bCommitted)
		return;

	if (m_bAppointmentMerge)
		SaveApptSelectedColumnSizes();
	else
		SaveSelectedColumnSizes();

}

void CGroups::OnColumnSizingFinishedUnselected(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth) 
{
	//uncommitted
	if(!bCommitted)
		return;

	if (m_bAppointmentMerge)
		SaveApptUnselectedColumnSizes();
	else
		SaveUnselectedColumnSizes();
}

void CGroups::SetUnselectedColumnSizes()
{
	//don't want to remember
	if(!IsDlgButtonChecked(IDC_REMEMBER_COLUMNS)) {
		return;
	}

	long nPersonType = VarLong(m_dlTypeOfPerson->GetValue(m_dlTypeOfPerson->CurSel,0),fbjPatient);

	SetColumnSizes(GetRemotePropertyText("DefaultFilterColumnSizes", "", nPersonType, GetCurrentUserName(), false), m_unselected);
}

void CGroups::SetApptUnselectedColumnSizes()
{
	//don't want to remember
	if(!IsDlgButtonChecked(IDC_REMEMBER_COLUMNS)) {
		return;
	}

	long nPersonType = VarLong(m_dlTypeOfPerson->GetValue(m_dlTypeOfPerson->CurSel,0),fbjPatient);

	SetColumnSizes(GetRemotePropertyText("DefaultApptFilterColumnSizes", "", nPersonType, GetCurrentUserName(), false), m_apptUnselected);
}

void CGroups::SetSelectedColumnSizes()
{
	//don't want to remember
	if(!IsDlgButtonChecked(IDC_REMEMBER_COLUMNS))
		return;

	long nPersonType = VarLong(m_dlTypeOfPerson->GetValue(m_dlTypeOfPerson->CurSel,0),fbjPatient);

	SetColumnSizes(GetRemotePropertyText("DefaultGroupColumnSizes", "", nPersonType, GetCurrentUserName(), false), m_selected);
	
}

void CGroups::SetApptSelectedColumnSizes()
{
	//don't want to remember
	if(!IsDlgButtonChecked(IDC_REMEMBER_COLUMNS))
		return;

	long nPersonType = VarLong(m_dlTypeOfPerson->GetValue(m_dlTypeOfPerson->CurSel,0),fbjPatient);

	SetColumnSizes(GetRemotePropertyText("DefaultApptGroupColumnSizes", "", nPersonType, GetCurrentUserName(), false), m_apptSelected);
	
}


void CGroups::SetColumnSizes(CString &strSizesList, _DNxDataListPtr pDataList)
{
	if(!strSizesList.IsEmpty()) {
		IColumnSettingsPtr pCol;
		int nWidth = 0, i = 0;

		//parse the columns out and set them
		//PersonID, UserDefinedID, Last, First, Company
		int nComma = strSizesList.Find(",");
		while(nComma > 0) {
			nWidth = atoi(strSizesList.Left(nComma));
			strSizesList = strSizesList.Right(strSizesList.GetLength() - (nComma+1));

			if(i == 1 && (pDataList == m_selected || pDataList == m_unselected)) {
				//id column - if we're not looking at patients, hide this always
				if(m_dlTypeOfPerson->CurSel != 0) {
					nWidth = 0;
				}
			}

			pCol = pDataList->GetColumn(i);
			pCol->PutStoredWidth(nWidth);

			i++;
			nComma = strSizesList.Find(",");
		}
	}

}

void CGroups::OnRememberColumns() 
{
	//save the setting
	long nRemember = 0;	//default off
	if(IsDlgButtonChecked(IDC_REMEMBER_COLUMNS))
		nRemember = 1;
	else
		nRemember = 0;
	SetRemotePropertyInt("RememberLWColumns", nRemember, 0, GetCurrentUserName());

	//size the datalist appropriately
	if(!IsDlgButtonChecked(IDC_REMEMBER_COLUMNS)) {
		//they don't want to remember, so set to the normal column widths
		
		SetColumnWidths();
	}
	else {
		//they do want to remember!

		for(int i = 2; i <= 5; i++) {
			ChangeColumnStyle(m_unselected->GetColumn(i), csVisible);
		}

		for(i = 2; i <= 5; i++) {
			ChangeColumnStyle(m_selected->GetColumn(i), csVisible);
		}

		SetUnselectedColumnSizes();
		SetSelectedColumnSizes();
		SetApptUnselectedColumnSizes();
		SetApptSelectedColumnSizes();
	}
}

void CGroups::ChangeColumnStyle(IColumnSettingsPtr pCol, long csStyle)
{
	//only need to do this if it differs
	if(pCol->GetColumnStyle() != csStyle) {
		//save current size
		long nSize = pCol->GetStoredWidth();

		//change status
		pCol->PutColumnStyle(csStyle);

		//re-set size
		pCol->PutStoredWidth(nSize);
	}
}

void CGroups::SaveUnselectedColumnSizes()
{
	long nPersonType = VarLong(m_dlTypeOfPerson->GetValue(m_dlTypeOfPerson->CurSel,0),fbjPatient);

	//write it to ConfigRT
	SetRemotePropertyText("DefaultFilterColumnSizes", CreateColumnSizeList(m_unselected), nPersonType, GetCurrentUserName());
}

void CGroups::SaveApptUnselectedColumnSizes()
{
	long nPersonType = VarLong(m_dlTypeOfPerson->GetValue(m_dlTypeOfPerson->CurSel,0),fbjPatient);

	//write it to ConfigRT
	SetRemotePropertyText("DefaultApptFilterColumnSizes", CreateColumnSizeList(m_apptUnselected), nPersonType, GetCurrentUserName());
}

void CGroups::SaveSelectedColumnSizes()
{
	long nPersonType = VarLong(m_dlTypeOfPerson->GetValue(m_dlTypeOfPerson->CurSel,0),fbjPatient);

	//write it to ConfigRT
	SetRemotePropertyText("DefaultGroupColumnSizes", CreateColumnSizeList(m_selected), nPersonType, GetCurrentUserName());
}

void CGroups::SaveApptSelectedColumnSizes()
{
	long nPersonType = VarLong(m_dlTypeOfPerson->GetValue(m_dlTypeOfPerson->CurSel,0),fbjPatient);

	//write it to ConfigRT
	SetRemotePropertyText("DefaultApptGroupColumnSizes", CreateColumnSizeList(m_apptSelected), nPersonType, GetCurrentUserName());
}

CString CGroups::CreateColumnSizeList(_DNxDataListPtr pDataList)
{
	IColumnSettingsPtr pCol;
	CString str, strList;

	//save width of each column
	for(int i = 0; i < pDataList->GetColumnCount(); i++) {
		pCol = pDataList->GetColumn(i);
		long nWidth = pCol->GetStoredWidth();
		if(i == 1) {
			//first column, there are special cases
			if(m_dlTypeOfPerson->CurSel != 0) {
				//we don't want to save a size of 0, so save the default size of 50 for the id field
				nWidth = 50;
			}
			else {
				nWidth = pCol->GetStoredWidth();
			}
		}

		str.Format("%li,", nWidth);
		strList += str;
	}

	return strList;
}

void CGroups::OnDestroy() 
{
	CNxDialog::OnDestroy();

	if(!IsDlgButtonChecked(IDC_REMEMBER_COLUMNS)) {

		long nPersonType = VarLong(m_dlTypeOfPerson->GetValue(m_dlTypeOfPerson->CurSel,0),fbjPatient);

		//if they don't want to remember, and we're quitting, wipe out whatever we've saved
		SetRemotePropertyText("DefaultGroupColumnSizes", "", nPersonType, GetCurrentUserName());
		SetRemotePropertyText("DefaultFilterColumnSizes", "", nPersonType, GetCurrentUserName());
		SetRemotePropertyText("DefaultApptGroupColumnSizes", "", nPersonType, GetCurrentUserName());
		SetRemotePropertyText("DefaultApptFilterColumnSizes", "", nPersonType, GetCurrentUserName());
	}
}

//TES 6/20/03: I leave this code in here as an example of what NOT to use this callback for.  Only implement
//and use this callback if you wish to silently and in-place fix an obsolete field, but for some reason 
//the default functionality isn't enough for you.
/*BOOL WINAPI CGroups::GetNewFilterString(long nObsoleteInfoId, long nOperator, CString &strFieldName, LPCTSTR strDefaultString, CString &strNewString)
{
	switch(nObsoleteInfoId) {
		//APPOINTMENT: fields
	case 240:
	case 241:
	case 242:
	case 243:
	case 244:
	case 245:
	case 246:
	case 247:
	case 248:
	case 249:
	case 261:
	case 266:
		if(m_bDontWarnObsolete) {
		} else {
			MsgBox(MB_ICONEXCLAMATION, "WARNING: The obsolete filter field \"%s\" could not be loaded.\n"
				"Please modify this filter to use the \"Has Appointment\" filter field, with appropriate subfilters.\n"
				"You will not be able to save this filter until the obsolete field has been removed.", strFieldName);
		}
		strNewString = "";
		return TRUE;
	}
	return FALSE;
}
*/

BOOL CGroups::PreTranslateMessage(MSG* pMsg) 
{

	switch (pMsg->message) {
	case WM_KEYDOWN:
		switch (pMsg->wParam) {
		case VK_RETURN:
			{
				//if the focus is on either datalist, hitting enter moves the highlighted item
				if(GetFocus() == GetDlgItem(IDC_UNSELECTED)) {
					OnAdd();
					return TRUE;
				}
				else if(GetFocus() == GetDlgItem(IDC_SELECTED)) {
					OnRemove();
					return TRUE;
				}
			}
			break;
		default:
		break;
		}
	default:
		break;
	}

	return CNxDialog::PreTranslateMessage(pMsg);
}

// A class that allows us to allocate some number of bytes exactly once, and then automatically deallocates them on destruction
class CSafeBytes
{
public:
	CSafeBytes() { m_pData = NULL; };
	~CSafeBytes() { FreeBuffer(); };

public:
	BYTE *AllocBuffer(DWORD dwByteCountToAllocate) { ASSERT(m_pData == NULL); FreeBuffer(); m_pData = new BYTE[dwByteCountToAllocate]; return m_pData; };
	void FreeBuffer() { if (m_pData) { delete []m_pData; m_pData = NULL; } };

public:
	BOOL IsAllocated() { if (m_pData) { return TRUE; } else { return FALSE; } };
	BYTE *GetBuffer() { return m_pData; };

protected:
	BYTE *m_pData;
};

CString GetMonthName(long nMonthNumber)
{
	return FormatDateTimeForInterface(COleDateTime(2003, nMonthNumber, 1, 0, 0, 0), _T("%B"));
}

const TCHAR *CGroups::CalcInternationalMonthList()
{
	// Only load it if we haven't loaded it already
	// We use a static variable to ensure that it's been constructed by the time we need 
	// to use it.  If it's global, then this function (CalcInternationalMonthList) may 
	// have been called PRIOR to the global variable's consturctor being called, which at 
	// best results in a memory leak, at worst results in a pointer to no mans land.
	static CSafeBytes l_ssbInternationalMonthList;
	if (!l_ssbInternationalMonthList.IsAllocated()) {

		// Get the list of months
		CStringArray arystrMonthNames, arystrMonthNumbers;
		arystrMonthNames.Add(GetMonthName(1)); arystrMonthNumbers.Add("1");
		arystrMonthNames.Add(GetMonthName(2)); arystrMonthNumbers.Add("2");
		arystrMonthNames.Add(GetMonthName(3)); arystrMonthNumbers.Add("3");
		arystrMonthNames.Add(GetMonthName(4)); arystrMonthNumbers.Add("4");
		arystrMonthNames.Add(GetMonthName(5)); arystrMonthNumbers.Add("5");
		arystrMonthNames.Add(GetMonthName(6)); arystrMonthNumbers.Add("6");
		arystrMonthNames.Add(GetMonthName(7)); arystrMonthNumbers.Add("7");
		arystrMonthNames.Add(GetMonthName(8)); arystrMonthNumbers.Add("8");
		arystrMonthNames.Add(GetMonthName(9)); arystrMonthNumbers.Add("9");
		arystrMonthNames.Add(GetMonthName(10)); arystrMonthNumbers.Add("10");
		arystrMonthNames.Add(GetMonthName(11)); arystrMonthNumbers.Add("11");
		arystrMonthNames.Add(GetMonthName(12)); arystrMonthNumbers.Add("12");

		// (b.cardillo 2003-07-22 17:27) - According to Microsoft it's possible to get the name of the 
		// 13th month, but apparently our InternationalUtils doesn't handle a COleDateTime with a month 
		// of 13 and I don't think it's something we need to consider.
		// arystrMonthNames.Add(GetMonthName(13)); arystrMonthNumbers.Add("13");

		// Get the number of entries in the array (which right now is always 12, which makes sense of course)
		long nArraySize = arystrMonthNames.GetSize();

		// Sum the size of each record in the array
		DWORD dwByteCountRequired = 0;
		{
			for (long i=0; i<nArraySize; i++) {
				// Add the size of the month name plus the null delimiter
				dwByteCountRequired += sizeof(TCHAR) * (arystrMonthNames.GetAt(i).GetLength() + 1);
				// Add the size of the month number plus the null delimiter
				dwByteCountRequired += sizeof(TCHAR) * (arystrMonthNumbers.GetAt(i).GetLength() + 1);
			}
			// Add one for the null termination (after the last null delimiter, which means the string ends with two nulls)
			dwByteCountRequired++;
		}

		// Allocate space for that much text
		BYTE *pData = l_ssbInternationalMonthList.AllocBuffer(dwByteCountRequired);

		// Fill that buffer that's now allocated with the text
		if (pData) {
			// Loop through each month
			LPTSTR pCurSpot = (LPTSTR)pData;
			for (long i=0; i<nArraySize; i++) {
				// Copy the month name into the current spot in the buffer
				pCurSpot += sprintf(pCurSpot, "%s", arystrMonthNames.GetAt(i));
				// Add the delimiter
				*pCurSpot = _T('\0');
				pCurSpot++; // Adding 1 to an LPTSTR is like adding sizeof(TCHAR) to a (BYTE *)
				
				// Copy the month number into the current spot in the buffer
				pCurSpot += sprintf(pCurSpot, "%s", arystrMonthNumbers.GetAt(i));
				// Add the delimiter
				*pCurSpot = _T('\0');
				pCurSpot++; // Adding 1 to an LPTSTR is like adding sizeof(TCHAR) to a (BYTE *)
				
			}
			// Add the null termination
			*pCurSpot = _T('\0');
			pCurSpot++; // Adding 1 to an LPTSTR is like adding sizeof(TCHAR) to a (BYTE *)
			
			// The byte we're at now had better be dwByteCountRequired bytes greater than the byte we started at
			ASSERT((DWORD)(((BYTE *)pCurSpot) - pData) == dwByteCountRequired);
		}
	}

	// Return the official buffer
	return (LPCTSTR)l_ssbInternationalMonthList.GetBuffer();
}

void CGroups::SetColumnWidths() {

	long nPersonType = VarLong(m_dlTypeOfPerson->GetValue(m_dlTypeOfPerson->CurSel,0),fbjPatient);

	//first, hide the ID column for everyone but patients
	if(nPersonType != fbjPatient) {
		m_unselected->GetColumn(lwcUserDefinedID)->PutStoredWidth(0);
		m_selected->GetColumn(lwcUserDefinedID)->PutStoredWidth(0);
	} else {
		m_unselected->GetColumn(lwcUserDefinedID)->PutStoredWidth(50);
		m_selected->GetColumn(lwcUserDefinedID)->PutStoredWidth(50);
	}

	//DRT 1/9/2004 - PLID 5157 - This PL item inserted the below if/else clause, which caused the remember
	//	checkbox to no longer functions, because the column styles must be manual sizing, not auto sizing.
	//	So if the "remember" checkbox is set, only the address column code is going to be set.

	//next, if an insurance company, hide first and last names
	//but if not, hide the address
	if(nPersonType != fbjInsCo) {
		//not insurance
		if(!IsDlgButtonChecked(IDC_REMEMBER_COLUMNS)) {
			m_unselected->GetColumn(lwcLastName)->PutColumnStyle(csVisible|csWidthAuto);
			m_selected->GetColumn(lwcLastName)->PutColumnStyle(csVisible|csWidthAuto);
			m_unselected->GetColumn(lwcFirstName)->PutColumnStyle(csVisible|csWidthAuto);
			m_selected->GetColumn(lwcFirstName)->PutColumnStyle(csVisible|csWidthAuto);
			m_unselected->GetColumn(lwcCompany)->PutColumnStyle(csVisible|csWidthAuto);
			m_selected->GetColumn(lwcCompany)->PutColumnStyle(csVisible|csWidthAuto);
		}
		m_unselected->GetColumn(lwcAddress)->PutColumnStyle(csVisible|csFixedWidth);
		m_unselected->GetColumn(lwcAddress)->PutStoredWidth(0);
		m_selected->GetColumn(lwcAddress)->PutColumnStyle(csVisible|csFixedWidth);
		m_selected->GetColumn(lwcAddress)->PutStoredWidth(0);
		
	} else {
		//is insurance
		if(!IsDlgButtonChecked(IDC_REMEMBER_COLUMNS)) {
			m_unselected->GetColumn(lwcLastName)->PutColumnStyle(csVisible|csFixedWidth);
			m_unselected->GetColumn(lwcLastName)->PutStoredWidth(0);
			m_selected->GetColumn(lwcLastName)->PutColumnStyle(csVisible|csFixedWidth);
			m_selected->GetColumn(lwcLastName)->PutStoredWidth(0);
			m_unselected->GetColumn(lwcFirstName)->PutColumnStyle(csVisible|csFixedWidth);
			m_unselected->GetColumn(lwcFirstName)->PutStoredWidth(0);
			m_selected->GetColumn(lwcFirstName)->PutColumnStyle(csVisible|csFixedWidth);
			m_selected->GetColumn(lwcFirstName)->PutStoredWidth(0);
			m_unselected->GetColumn(lwcCompany)->PutColumnStyle(csVisible|csWidthAuto);
			m_selected->GetColumn(lwcCompany)->PutColumnStyle(csVisible|csWidthAuto);
		}
		m_unselected->GetColumn(lwcAddress)->PutColumnStyle(csVisible|csWidthAuto);
		m_selected->GetColumn(lwcAddress)->PutColumnStyle(csVisible|csWidthAuto);
	}

	//set appointment datalist column widths
	if(!IsDlgButtonChecked(IDC_REMEMBER_COLUMNS)) {
		m_apptSelected->GetColumn(lwacUserDefinedID)->PutStoredWidth(40);
		m_apptSelected->GetColumn(lwacLastName)->PutStoredWidth(55);
		m_apptSelected->GetColumn(lwacFirstName)->PutStoredWidth(55);
		m_apptSelected->GetColumn(lwacApptDate)->PutStoredWidth(70);
		m_apptSelected->GetColumn(lwacApptStart)->PutStoredWidth(60);
		m_apptSelected->GetColumn(lwacApptType)->PutStoredWidth(60);
		m_apptSelected->GetColumn(lwacApptPurpose)->PutStoredWidth(60);
		m_apptSelected->GetColumn(lwacApptResource)->PutStoredWidth(60);
		m_apptSelected->GetColumn(lwacApptLocation)->PutStoredWidth(60);
		m_apptSelected->GetColumn(lwacApptStatus)->PutStoredWidth(60);

		m_apptUnselected->GetColumn(lwacUserDefinedID)->PutStoredWidth(40);
		m_apptUnselected->GetColumn(lwacLastName)->PutStoredWidth(55);
		m_apptUnselected->GetColumn(lwacFirstName)->PutStoredWidth(55);
		m_apptUnselected->GetColumn(lwacApptDate)->PutStoredWidth(70);
		m_apptUnselected->GetColumn(lwacApptStart)->PutStoredWidth(60);
		m_apptUnselected->GetColumn(lwacApptType)->PutStoredWidth(60);
		m_apptUnselected->GetColumn(lwacApptPurpose)->PutStoredWidth(60);
		m_apptUnselected->GetColumn(lwacApptResource)->PutStoredWidth(60);
		m_apptUnselected->GetColumn(lwacApptLocation)->PutStoredWidth(60);
		m_apptUnselected->GetColumn(lwacApptStatus)->PutStoredWidth(60);

	}
}

LRESULT CGroups::OnInvalidFilter(WPARAM wParam, LPARAM lParam)
{
	try {
		long nGroupID = (long)wParam;
		BSTR bstrName = (BSTR)lParam;
		CString strFilterName = bstrName;
		SysFreeString(bstrName);
		if (IDYES == MsgBox(MB_YESNO, "The filter '%s' cannot be parsed. This may happen for one of the following reasons:\r\n\r\nThe filter contains a subfilter that does not exist.\r\nThe filter contains a group that does not exist.\r\nThe filter has obselete fields.\r\n\r\nWould you like to edit the filter now?", strFilterName))
		{
			// (z.manning 2009-02-17 15:29) - PLID 32974 - Call FilterEditorDlg directly instead of OnEditFilters
			FilterEditorDlg(FALSE);
		}		
	}
	NxCatchAll("Error handling invalid filter");
	return 0;
}

void CGroups::OnSelectTab(short newTab, short oldTab)
{
	bool bIsRequerying = false;

	if (newTab == 0)
	{
		// Person Merge
		m_bAppointmentMerge = FALSE;

		// Hide or Show appropriate controls
		GetDlgItem(IDC_APPT_SELECTED)->ShowWindow(FALSE);
		GetDlgItem(IDC_PERSON_TYPE_COMBO)->ShowWindow(TRUE);
		GetDlgItem(IDC_LABEL)->ShowWindow(TRUE);
		GetDlgItem(IDC_APPT_UNSELECTED)->ShowWindow(FALSE);
		GetDlgItem(IDC_UNSELECTED)->ShowWindow(TRUE);
		GetDlgItem(IDC_SELECTED)->ShowWindow(TRUE);
		GetDlgItem(IDC_NEW_GROUP_BTN)->ShowWindow(TRUE);
		GetDlgItem(IDC_SAVEGROUP)->ShowWindow(TRUE);
		GetDlgItem(IDC_DELETE)->ShowWindow(TRUE);
		GetDlgItem(IDC_EXISTING_GROUP_LIST)->ShowWindow(TRUE);

		// Reposition controls
		RepositionControls();		

		// Refresh lists
		FillFilterCombo(GetFilterIdLast(FALSE));
		ChangeFilterSelection(m_nCurFilterID, false);

		// Make sure the current selection for this tab is selected
		m_pFilterList->SetSelByColumn(eflcID, m_nCurFilterID);

		if (m_unselected->IsRequerying())
			bIsRequerying = true;

		// (m.hancock 2006-12-04 11:04) - PLID 21965 - Display the Person based merge sort options.
		CWnd *pParent = GetParent();
		if (pParent && pParent->IsKindOf(RUNTIME_CLASS(CLetterWriting))) {
			((CLetterWriting *)pParent)->DisplayDefaultSortOptions();
		}
	}
	else if (newTab == 1)
	{
		// Appointment Merge
		m_bAppointmentMerge = TRUE;

		// (z.manning 2013-07-17 11:35) - PLID 57609 - Load the appt datalist if this is the first time selecting the appt tab.
		if(!m_bAppointTabLoaded) {
			m_bAppointTabLoaded = TRUE;
			LoadAppointmentDataList();
		}

		// Hide or Show appropriate controls
		GetDlgItem(IDC_PERSON_TYPE_COMBO)->ShowWindow(FALSE);
		GetDlgItem(IDC_LABEL)->ShowWindow(FALSE);
		GetDlgItem(IDC_APPT_UNSELECTED)->ShowWindow(TRUE);
		GetDlgItem(IDC_UNSELECTED)->ShowWindow(FALSE);
		GetDlgItem(IDC_SELECTED)->ShowWindow(FALSE);
		GetDlgItem(IDC_NEW_GROUP_BTN)->ShowWindow(FALSE);
		GetDlgItem(IDC_SAVEGROUP)->ShowWindow(FALSE);
		GetDlgItem(IDC_DELETE)->ShowWindow(FALSE);
		GetDlgItem(IDC_EXISTING_GROUP_LIST)->ShowWindow(FALSE);
		GetDlgItem(IDC_APPT_SELECTED)->ShowWindow(TRUE);

		// Reposition controls
		RepositionControls();		

		// Refresh lists
		FillFilterCombo(GetFilterIdLast(TRUE));
		ChangeFilterSelection(m_nCurFilterID, false);

		// Make sure the current selection for this tab is selected
		m_pFilterList->SetSelByColumn(0, m_nCurFilterID);

		if (m_apptUnselected->IsRequerying())
			bIsRequerying = true;

		// (m.hancock 2006-12-04 11:04) - PLID 21965 - Display the Appointment based merge sort options.
		CWnd *pParent = GetParent();
		if (pParent && pParent->IsKindOf(RUNTIME_CLASS(CLetterWriting))) {
			((CLetterWriting *)pParent)->DisplayAppointmentBasedSortOptions();
		}
	}

	if (bIsRequerying)
		SetListCountToLoading();
	else
		ReflectListCount(LWL_ALL_LISTS);

	// (a.walling 2008-08-07 16:10) - PLID 30099 - Redraw all child windows
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void CGroups::RepositionControls()
{
	if (m_bAppointmentMerge) {
		CRect rcOrginal, rcNew;
		GetDlgItem(IDC_PERSON_TYPE_COMBO)->GetWindowRect(&rcNew);
		ScreenToClient(&rcNew);
		GetDlgItem(IDC_NEW_FILTER_BTN)->GetWindowRect(&rcOrginal);
		ScreenToClient(&rcOrginal);
		GetDlgItem(IDC_NEW_FILTER_BTN)->MoveWindow(rcOrginal.left, rcNew.top, rcOrginal.Width(), rcOrginal.Height(), TRUE);
		GetDlgItem(IDC_EDIT_FILTERS)->GetWindowRect(&rcOrginal);
		ScreenToClient(&rcOrginal);
		GetDlgItem(IDC_EDIT_FILTERS)->MoveWindow(rcOrginal.left, rcNew.top, rcOrginal.Width(), rcOrginal.Height(), TRUE);
		GetDlgItem(IDC_SAVE_FILTERS)->GetWindowRect(&rcOrginal);
		ScreenToClient(&rcOrginal);
		GetDlgItem(IDC_SAVE_FILTERS)->MoveWindow(rcOrginal.left, rcNew.top, rcOrginal.Width(), rcOrginal.Height(), TRUE);
		GetDlgItem(IDC_DELETEFILTER)->GetWindowRect(&rcOrginal);
		ScreenToClient(&rcOrginal);
		GetDlgItem(IDC_DELETEFILTER)->MoveWindow(rcOrginal.left, rcNew.top, rcOrginal.Width(), rcOrginal.Height(), TRUE);
		// (z.manning, 05/13/2008) - PLID 29795 - Position the filter list below the buttons
		GetDlgItem(IDC_NEW_FILTER_BTN)->GetWindowRect(&rcNew);
		ScreenToClient(&rcNew);
		GetDlgItem(IDC_EXISTING_FILTER_LIST)->GetWindowRect(&rcOrginal);
		ScreenToClient(&rcOrginal);
		GetDlgItem(IDC_EXISTING_FILTER_LIST)->MoveWindow(rcOrginal.left, rcNew.bottom + 4, rcOrginal.Width(), rcOrginal.Height(), TRUE);
	}
	else {
		CRect rcOrginal, rcNew;
		GetDlgItem(IDC_REPOSITION)->GetWindowRect(&rcNew);
		ScreenToClient(&rcNew);
		GetDlgItem(IDC_NEW_FILTER_BTN)->GetWindowRect(&rcOrginal);
		ScreenToClient(&rcOrginal);
		GetDlgItem(IDC_NEW_FILTER_BTN)->MoveWindow(rcOrginal.left, rcNew.top, rcOrginal.Width(), rcOrginal.Height(), TRUE);
		GetDlgItem(IDC_EDIT_FILTERS)->GetWindowRect(&rcOrginal);
		ScreenToClient(&rcOrginal);
		GetDlgItem(IDC_EDIT_FILTERS)->MoveWindow(rcOrginal.left, rcNew.top, rcOrginal.Width(), rcOrginal.Height(), TRUE);
		GetDlgItem(IDC_SAVE_FILTERS)->GetWindowRect(&rcOrginal);
		ScreenToClient(&rcOrginal);
		GetDlgItem(IDC_SAVE_FILTERS)->MoveWindow(rcOrginal.left, rcNew.top, rcOrginal.Width(), rcOrginal.Height(), TRUE);
		GetDlgItem(IDC_DELETEFILTER)->GetWindowRect(&rcOrginal);
		ScreenToClient(&rcOrginal);
		GetDlgItem(IDC_DELETEFILTER)->MoveWindow(rcOrginal.left, rcNew.top, rcOrginal.Width(), rcOrginal.Height(), TRUE);
		GetDlgItem(IDC_EXISTING_FILTER_LIST)->GetWindowRect(&rcOrginal);
		ScreenToClient(&rcOrginal);
		GetDlgItem(IDC_EXISTING_FILTER_LIST)->MoveWindow(rcOrginal.left, rcNew.bottom, rcOrginal.Width(), rcOrginal.Height(), TRUE);	
	}
}

void CGroups::OnApptSelectorNew() 
{
	CApptSelector dlg(TRUE, -1, IsActionSupported, CommitSubfilterAction);

	if (dlg.DoModal() == IDOK) {
		FillFilterCombo(dlg.m_nFilterID);
		ChangeFilterSelection(m_nCurFilterID, true);
	}

}

void CGroups::OnApptSelectorEdit() 
{
	CApptSelector dlg(FALSE, GetFilterIdLast(TRUE), IsActionSupported, CommitSubfilterAction);

	if (dlg.DoModal() == IDOK) {
		FillFilterCombo(dlg.m_nFilterID);
		ChangeFilterSelection(m_nCurFilterID, true);
	}

}

void CGroups::LoadAppointmentDataList() 
{
	try {
		long nFilterID = GetFilterIdLast(TRUE);
		CString strWhere = "", strFrom = "";

		if (nFilterID != FILTER_ID_NEW && nFilterID != FILTER_ID_ALL) {
			_RecordsetPtr prs = CreateRecordset("SELECT Name, Filter FROM FiltersT WHERE ID = %li", nFilterID);
			if (!prs->eof) {
				// Get the filter SQL string to filter on
				CString strFilter = AdoFldString(prs, "Filter");
				
				if (!(CFilter::ConvertFilterStringToClause(nFilterID, strFilter, fboAppointment, &strWhere, &strFrom, NULL, NULL, TRUE))) {
					PostMessage(NXM_INVALID_FILTER, nFilterID, (LPARAM)AdoFldString(prs, "Name","").AllocSysString());
					return;
				}
			} else {
				// Failure
				//(j.anspach 05-31-2006 16:35 PLID 20871) - I commented out this Exception because its not
				//  really an error if we get here.  We can get here 1 of 2 ways I've found ... either the
				//  appointments based merging has no filters there, or the last filter you used has been
				//  deleted.  Before, when either was the case you got this ugly error.
				//HandleException(NULL, "Filter Error 200: Could not load the appointment based filter!", __LINE__, __FILE__);
				return;
			}

			if (strWhere.IsEmpty()) {
				m_apptUnselected->WhereClause = _bstr_t("PersonT.ID < -25 ");
			}
			else {
				m_apptUnselected->WhereClause = _bstr_t("AppointmentsT.ID IN (SELECT DISTINCT AppointmentsT.ID FROM " + strFrom + " WHERE " + strWhere + ") AND PersonT.ID <> -25 AND PatientsT.CurrentStatus <> 4 ");
			}

			m_apptUnselected->Requery();
		}

	}NxCatchAll("Error in CGroups::LoadAppointmentDataList");
}

/*CString CGroups::BuildBaseAppointmentWhereClause() 
{
	CString	strWhereClause = "", strTemp;

	try {
		// If at least some criteria aren't chosen, then don't show any patients in the data list
		if (m_strApptTypeIDs == "" && m_strApptLocIDs == "" && m_strApptStatusIDs == "" && m_strApptResIDs == "" && m_strApptPurposeIDs == "" && (m_strApptFromDate == "" || m_strApptToDate == ""))
			// This will effectively show no one
			strWhereClause = "PersonT.ID < -25 ";
		else {
			// Build WHERE clause
			strWhereClause = "PersonT.ID <> -25 AND PatientsT.CurrentStatus <> 4 ";

			// Cancelled Appt
			if (!m_bApptIncludeCancelled) {
				strTemp.Format(" AND AppointmentsT.Status <> 4 ");
				strWhereClause += strTemp;
			}

			// Appt Type
			if (m_strApptTypeIDs != "") {
				strTemp.Format(" AND AppointmentsT.AptTypeID IN (%s) ", m_strApptTypeIDs);
				strWhereClause += strTemp;
			}

			// Appt Location
			if (m_strApptLocIDs != "") {
				strTemp.Format(" AND AppointmentsT.LocationID IN (%s) ", m_strApptLocIDs);
				strWhereClause += strTemp;
			}

			// Appt Status
			if (m_strApptStatusIDs != "") {
				strTemp.Format(" AND AppointmentsT.Status IN (%s) ", m_strApptStatusIDs);
				strWhereClause += strTemp;
			}

			// Appt Resource
			if (m_strApptResIDs != "") {
				strTemp.Format(" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN (%s)) ", m_strApptResIDs);
				strWhereClause += strTemp;
			}

			// Appt Purpose
			if (m_strApptPurposeIDs != "") {
				strTemp.Format(" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (%s)) ", m_strApptPurposeIDs);
				strWhereClause += strTemp;
			}

			// Appt Date
			if (m_strApptFromDate != "" && m_strApptToDate != "") {
				strTemp.Format(" AND AppointmentsT.Date >= '%s' AND AppointmentsT.Date <= '%s' ", m_strApptFromDate, m_strApptToDate);
				strWhereClause += strTemp;
			}
		}	

	}NxCatchAll("Error in CGroups::BuildAppointmentDataListWhereClause");

	return strWhereClause;
	
}*/

// (m.hancock 2006-12-04 11:32) - PLID 21965 - Returns the index of the selected tab.
long CGroups::GetSelectedTab()
{
	return m_tab->CurSel;
}


void CGroups::OnSelChosenExistingFilterList(LPDISPATCH lpRow) 
{
	//TES 2/22/2007 - PLID 20642 - Copied out of the previous OnSelEndOkCombo() function, adjusted to reflect that this
	// is an NxDataList now, not an MS Combo.
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		long nNewID = VarLong(pRow->GetValue(eflcID));
		if (nNewID == FILTER_ID_NEW) {
			if (FilterEditorDlg(TRUE) != IDOK) {
				m_pFilterList->SetSelByColumn(eflcID, m_nCurFilterID);
			}
		} else if (nNewID != m_nCurFilterID) {
			if(!ChangeFilterSelection(nNewID)) {
				if (m_bAppointmentMerge) 
					m_apptUnselected->Clear();
				else 
					m_unselected->Clear();

				// (z.manning 2009-02-17 15:24) - PLID 23974 - Even the filter selection change wasn't
				// valid, the actual value in the filter combo did change so we need to update m_nCurFilterID
				// to reflect that. If filter selection did change it's likely due to an invalid filter
				// and if we don't update m_nCurFilterID, it will be very difficult for them to edit it.
				m_nCurFilterID = nNewID;

				/*m_nCurFilterIndex = nNewIndex;
				m_cboFilterList.PostMessage(CB_SETCURSEL, m_nCurFilterIndex, 0);*/
				//invalid filter
				//TES 6/20/03: I'm taking this code out because it's far too easy to accidentally delete a 
				//filter that, with just a little fixing, could be perfectly valid.
				/*if(IDYES==MessageBox("The selected filter is invalid. Would you like to delete it?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {				
					OnDeleteFilter();
				}
				// (c.haag 2003-10-02 10:28) - We already handle this in OnInvalidFilter.
				else {*/
	/*				AfxMessageBox("You must edit this filter in order to continue. Invalid filters are not allowed.");
					//Now, we just warned them about any obsolete fields, they know they're wrong, so let's
					//not bother them again, just while we load it.
					OnEditFilters();*/
				//}
			}
			else {
				m_nCurFilterID = nNewID;
				m_pFilterList->SetSelByColumn(eflcID, m_nCurFilterID);
			}
		}
	}NxCatchAll("Error in CGroups::OnSelChosenExistingFilterList()");
}

void CGroups::OnSelChosenExistingGroupList(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		long nNewID = VarLong(pRow->GetValue(eglcID));
		
		if ((nNewID != m_nCurGroupID) || (nNewID == GROUP_ID_NEW)) {
			if (IsCurrentGroupModified()) {
				switch (AfxMessageBox("The group for Person Based Merging has been modified.  Would you like to save the changes?", MB_YESNOCANCEL|MB_ICONQUESTION)) {
				case IDYES:
					{
						// Save the changes and then do some manual processing before proceeding
						long nSavedGroupId = DoSaveCurrentGroup(m_nCurGroupID, nNewID);
						if (nSavedGroupId != GROUP_ID_SAVE_CANCELED) {
							m_nCurGroupID = nNewID;
							// Saved, so change the selection to the group that we're switching to (not the group we just saved)						
							ChangeGroupSelection(nNewID);
							m_pGroupList->SetSelByColumn(eglcID, nNewID);
						}
						return;
					}
					break;
				case IDNO:
					// Proceed without saving the changes
					break;
				case IDCANCEL:
				default:
					// Pretend they didn't make a selection change and don't proceed
					m_pGroupList->SetSelByColumn(eglcID, m_nCurGroupID);
					return;
					break;
				}
			}
			if (ChangeGroupSelection(nNewID, false)) {
				m_nCurGroupID = nNewID;
				RefreshCurrentListings(true, false);
			}
		}
	}NxCatchAll("Error in CGroups::OnSelChosenExistingGroupList()");
}

void CGroups::OnSelChangingExistingFilterList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	//TES 2/23/2007 - PLID 20642 - Don't let them not select anything.
	if(*lppNewSel == NULL) {
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	}
}

void CGroups::OnSelChangingExistingGroupList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	//TES 2/23/2007 - PLID 20642 - Don't let them not select anything.
	if(*lppNewSel == NULL) {
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	}
}

//(c.copits 2011-09-13) PLID 43485 - "Go to Patient" right-click option in letter writing
void CGroups::OnRButtonDownPatientUnselectedList(long nRow, short nCol, long x, long y, long nFlags)
{
	try {

		// This menu should only appear when "Based on" is set to Patients and Prospects. 
		long nPersonType = VarLong(m_dlTypeOfPerson->GetValue(m_dlTypeOfPerson->CurSel,0),-1);
		if (nPersonType != fbjPatient) {
			return;
		}

		// Generate a menu to allow the user to go to patient

		CMenu pMenu;

		m_unselected->CurSel = nRow;

		if(nRow == -1) {
			return;
		}

		pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION, ID_PATIENT_UNSELECTED_GOTO_PATIENT, "&Go To Patient");

		CPoint pt;
		GetCursorPos(&pt);
		pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-09-13) PLID 43485 - "Go to Patient" right-click option in letter writing
void CGroups::OnGoToPatientUnselectedList()
{
	try {

		long nCurSel = m_unselected->CurSel;
		long nPatientID = VarLong(m_unselected->GetValue(nCurSel, 0));

		if (nPatientID != -1) {
			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {

				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
					if(IDNO == MessageBox("This patient is not in the current lookup. \n"
						"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
				
				if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

					//close this dialog
					CNxDialog::OnOK();

					//Now just flip to the patient's module and set the active Patient
					pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
					CNxTabView *pView = pMainFrame->GetActiveView();
					if(pView) {
						pView->UpdateView();
					}
				}
			}
			else {
				MsgBox(MB_ICONSTOP|MB_OK, "ERROR - CGroups::OnGoToPatientUnselectedList(): Cannot Open Mainframe");
			}
		}

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-09-13) PLID 43485 - "Go to Patient" right-click option in letter writing
void CGroups::OnRButtonDownPatientSelectedList(long nRow, short nCol, long x, long y, long nFlags)
{
	try {

		m_selected->CurSel = nRow;

		if(nRow == -1) {
			return;
		}

		// This menu should only appear for patients and prospoects.
		long nPatientID = VarLong(m_selected->GetValue(nRow, 0));
		if (nPatientID != -1) {
			_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 * FROM PatientsT WHERE PersonID = {INT} ", nPatientID);
			if (!prs->eof) {
				// Generate a menu to allow the user to go to patient

				CMenu pMenu;

				pMenu.CreatePopupMenu();
				pMenu.InsertMenu(0, MF_BYPOSITION, ID_PATIENT_SELECTED_GOTO_PATIENT, "&Go To Patient");

				CPoint pt;
				GetCursorPos(&pt);
				pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
			}
			prs->Close();
		}

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-09-13) PLID 43485 - "Go to Patient" right-click option in letter writing
void CGroups::OnGoToPatientSelectedList()
{
	try {

		long nCurSel = m_selected->CurSel;
		long nPatientID = VarLong(m_selected->GetValue(nCurSel, 0));

		if (nPatientID != -1) {
			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {

				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
					if(IDNO == MessageBox("This patient is not in the current lookup. \n"
						"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
				
				if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

					//close this dialog
					CNxDialog::OnOK();

					//Now just flip to the patient's module and set the active Patient
					pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
					CNxTabView *pView = pMainFrame->GetActiveView();
					if(pView) {
						pView->UpdateView();
					}
				}
			}
			else {
				MsgBox(MB_ICONSTOP|MB_OK, "ERROR - CGroups::OnGoToPatientSelectedList(): Cannot Open Mainframe");
			}
		}

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-09-20) PLID 43485 - "Go to Patient" right-click option in letter writing
void CGroups::OnRButtonDownApptUnselectedList(long nRow, short nCol, long x, long y, long nFlags)
{
	try {

		// Generate a menu to allow the user to go to patient

		CMenu pMenu;

		m_apptUnselected->CurSel = nRow;

		if(nRow == -1) {
			return;
		}

		pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION, ID_APPT_UNSELECTED_GOTO_PATIENT, "&Go To Patient");

		CPoint pt;
		GetCursorPos(&pt);
		pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-09-20) PLID 43485 - "Go to Patient" right-click option in letter writing
void CGroups::OnGoToApptUnselectedList()
{
	try {

		long nCurSel = m_apptUnselected->CurSel;
		long nPatientID = VarLong(m_apptUnselected->GetValue(nCurSel, 1));

		if (nPatientID != -1) {
			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {

				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
					if(IDNO == MessageBox("This patient is not in the current lookup. \n"
						"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
				
				if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

					//close this dialog
					CNxDialog::OnOK();

					//Now just flip to the patient's module and set the active Patient
					pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
					CNxTabView *pView = pMainFrame->GetActiveView();
					if(pView) {
						pView->UpdateView();
					}
				}
			}
			else {
				MsgBox(MB_ICONSTOP|MB_OK, "ERROR - CGroups::OnGoToApptUnselectedList(): Cannot Open Mainframe");
			}
		}

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-09-20) PLID 43485 - "Go to Patient" right-click option in letter writing
void CGroups::OnRButtonDownApptSelectedList(long nRow, short nCol, long x, long y, long nFlags)
{
	try {

		// Generate a menu to allow the user to go to patient

		CMenu pMenu;

		m_apptSelected->CurSel = nRow;

		if(nRow == -1) {
			return;
		}

		pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION, ID_APPT_SELECTED_GOTO_PATIENT, "&Go To Patient");

		CPoint pt;
		GetCursorPos(&pt);
		pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-09-20) PLID 43485 - "Go to Patient" right-click option in letter writing
void CGroups::OnGoToApptSelectedList()
{
	try {

		long nCurSel = m_apptSelected->CurSel;
		long nPatientID = VarLong(m_apptSelected->GetValue(nCurSel, 1));

		if (nPatientID != -1) {
			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {

				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
					if(IDNO == MessageBox("This patient is not in the current lookup. \n"
						"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
				
				if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

					//close this dialog
					CNxDialog::OnOK();

					//Now just flip to the patient's module and set the active Patient
					pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
					CNxTabView *pView = pMainFrame->GetActiveView();
					if(pView) {
						pView->UpdateView();
					}
				}
			}
			else {
				MsgBox(MB_ICONSTOP|MB_OK, "ERROR - CGroups::OnGoToApptSelectedList(): Cannot Open Mainframe");
			}
		}

	} NxCatchAll(__FUNCTION__);
}