// PatientInterestCalculationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientInterestCalculationDlg.h"
#include "Filter.h"
#include "Groups.h"
#include "FilterEditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPatientInterestCalculationDlg dialog

// (j.jones 2009-11-03 12:32) - PLID 29666 - added LW controls
enum FilterComboColumn {

	fccID = 0,
	fccName,
	fccFilter,
};

enum GroupComboColumn {

	gccID = 0,
	gccName,
};

CPatientInterestCalculationDlg::CPatientInterestCalculationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPatientInterestCalculationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPatientInterestCalculationDlg)

	//}}AFX_DATA_INIT
}


void CPatientInterestCalculationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPatientInterestCalculationDlg)
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_PATIENTS, m_btnUnselectAll);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_PATIENT, m_btnUnselectOne);
	DDX_Control(pDX, IDC_BTN_SELECT_ALL_PATIENTS, m_btnSelectAll);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_PATIENT, m_btnSelectOne);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_FC_RADIO_LW_ALL, m_radioAll);
	DDX_Control(pDX, IDC_FC_RADIO_LW_FILTER, m_radioFilter);
	DDX_Control(pDX, IDC_FC_RADIO_LW_GROUP, m_radioGroup);
	DDX_Control(pDX, IDC_FC_BTN_EDIT_LW, m_btnEditLW);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPatientInterestCalculationDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPatientInterestCalculationDlg)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_PATIENT, OnBtnSelectOnePatient)
	ON_BN_CLICKED(IDC_BTN_SELECT_ALL_PATIENTS, OnBtnSelectAllPatients)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_PATIENT, OnBtnUnselectOnePatient)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_PATIENTS, OnBtnUnselectAllPatients)
	ON_BN_CLICKED(IDC_FC_BTN_EDIT_LW, OnBtnEditLw)
	ON_BN_CLICKED(IDC_FC_RADIO_LW_ALL, OnRadioLwAll)
	ON_BN_CLICKED(IDC_FC_RADIO_LW_FILTER, OnRadioLwFilter)
	ON_BN_CLICKED(IDC_FC_RADIO_LW_GROUP, OnRadioLwGroup)
	//}}AFX_MSG_MAP	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPatientInterestCalculationDlg message handlers

BOOL CPatientInterestCalculationDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnSelectOne.AutoSet(NXB_RIGHT);
		m_btnSelectAll.AutoSet(NXB_RRIGHT);
		m_btnUnselectOne.AutoSet(NXB_LEFT);
		m_btnUnselectAll.AutoSet(NXB_LLEFT);
		// (z.manning, 04/30/2008) - PLID 29864 - More button styles
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		
		m_UnselectedList = BindNxDataListCtrl(this, IDC_UNSELECTED_PATIENT_LIST, GetRemoteData(), false);
		m_SelectedList = BindNxDataListCtrl(this, IDC_SELECTED_PATIENT_LIST, GetRemoteData(), false);
		// (j.jones 2009-11-03 11:06) - PLID 29666 - added LW controls
		m_FilterCombo = BindNxDataList2Ctrl(IDC_FC_LW_FILTER_COMBO);
		m_GroupCombo = BindNxDataList2Ctrl(IDC_FC_LW_GROUP_COMBO);

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_FilterCombo->GetNewRow();
		pRow->PutValue(fccID, (long)-1);
		pRow->PutValue(fccName, _bstr_t(" <Add New Filter>"));
		pRow->PutValue(fccFilter, "");		
		m_FilterCombo->AddRowSorted(pRow, NULL);

		//the caller should never call this dialog with an empty patient ID string
		ASSERT(!m_strPatientIDs.IsEmpty());

		// (j.jones 2009-11-03 11:06) - PLID 29666 - the LW controls will requery the list
		m_radioAll.SetCheck(TRUE);
		OnRadioLwAll();

	}NxCatchAll("Error in CPatientInterestCalculationDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPatientInterestCalculationDlg::OnOK() 
{
	try {

		if(m_SelectedList->GetRowCount() == 0) {
			AfxMessageBox("You must have at least one patient selected to continue.");
			return;
		}

		CWaitCursor pWait;

		// (j.jones 2009-06-11 16:07) - PLID 34577 - we now need to build an array of patient IDs to skip,
		// which means we are only looking at the unselected list

		m_aryPatientIDsToSkip.RemoveAll();

		// Loop through the datalist
		LONG nLastIDVal = -1;
		long i = 0;
		long p = m_UnselectedList->GetFirstRowEnum();
		LPDISPATCH lpDisp = NULL;
		while (p) {
			m_UnselectedList->GetNextRowEnum(&p, &lpDisp);
			NXDATALISTLib::IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
			long nPatientID = VarLong(pRow->GetValue(0));

			m_aryPatientIDsToSkip.Add(nPatientID);
		}

		//warn if they are skipping patients
		if(m_aryPatientIDsToSkip.GetSize() > 0) {
			CString strWarn;
			if(m_aryPatientIDsToSkip.GetSize() == 1) {
				strWarn = "You have marked one patient to be skipped. This patient will not have finance charges created.\n\n"
					"Are you sure you wish to skip this patient?";
			}
			else {
				strWarn.Format("You have marked %li patients to be skipped. These patients will not have finance charges created.\n\n"
					"Are you sure you wish to skip these patients?", m_aryPatientIDsToSkip.GetSize());
			}

			if(IDNO == MessageBox(strWarn, "Practice", MB_YESNO|MB_ICONQUESTION)) {
				return;
			}
		}
	
		CDialog::OnOK();

	}NxCatchAll("Error generating patient list.");
}

void CPatientInterestCalculationDlg::OnBtnSelectOnePatient() 
{
	if(m_UnselectedList->CurSel != -1)
		m_SelectedList->TakeCurrentRow(m_UnselectedList);	
}

void CPatientInterestCalculationDlg::OnBtnSelectAllPatients() 
{
	m_SelectedList->TakeAllRows(m_UnselectedList);	
}

void CPatientInterestCalculationDlg::OnBtnUnselectOnePatient() 
{
	if(m_SelectedList->CurSel != -1)
		m_UnselectedList->TakeCurrentRow(m_SelectedList);	
}

void CPatientInterestCalculationDlg::OnBtnUnselectAllPatients() 
{
	m_UnselectedList->TakeAllRows(m_SelectedList);	
}

BEGIN_EVENTSINK_MAP(CPatientInterestCalculationDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPatientInterestCalculationDlg)
	ON_EVENT(CPatientInterestCalculationDlg, IDC_UNSELECTED_PATIENT_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedPatientList, VTS_I4 VTS_I2)
	ON_EVENT(CPatientInterestCalculationDlg, IDC_SELECTED_PATIENT_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedPatientList, VTS_I4 VTS_I2)
	ON_EVENT(CPatientInterestCalculationDlg, IDC_FC_LW_FILTER_COMBO, 16, OnSelChosenLwFilterCombo, VTS_DISPATCH)
	ON_EVENT(CPatientInterestCalculationDlg, IDC_FC_LW_GROUP_COMBO, 16, OnSelChosenLwGroupCombo, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CPatientInterestCalculationDlg::OnDblClickCellUnselectedPatientList(long nRowIndex, short nColIndex) 
{
	OnBtnSelectOnePatient();
}

void CPatientInterestCalculationDlg::OnDblClickCellSelectedPatientList(long nRowIndex, short nColIndex) 
{
	OnBtnUnselectOnePatient();
}

void CPatientInterestCalculationDlg::OnCancel() 
{
	if(IDNO == MessageBox("If you cancel, no finance charges will be created.\n"
		"Are you sure you wish to cancel?","Practice",MB_YESNO|MB_ICONQUESTION)) {
		return;
	}
	
	CDialog::OnCancel();
}

// (j.jones 2009-11-03 11:06) - PLID 29666 - added LW controls
void CPatientInterestCalculationDlg::OnBtnEditLw()
{
	try {

		if(!m_radioFilter.GetCheck()) {
			return;
		}

		long nCurFilterID = -1;
		CString strCurFilter = "";

		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_FilterCombo->GetCurSel();
			if(pRow) {
				nCurFilterID = VarLong(pRow->GetValue(fccID));
				strCurFilter = VarString(pRow->GetValue(fccFilter), "");
			}
		}

		//check their permissions and their license
		if(!g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrUse)) {
			return;
		}

		if(!CheckCurrentUserPermissions(bioLWFilter, sptWrite)) {
			return;
		}

		//open up the filter dialog with patient filters only
		CFilterEditDlg dlg(NULL, 1, CGroups::IsActionSupported, CGroups::CommitSubfilterAction);
		long nResult;
		if(nCurFilterID == -1) {
			nResult = dlg.NewFilter();
		}
		else {
			nResult = dlg.EditFilter(nCurFilterID, strCurFilter);
		}

		if (nResult == 1) {
			//they clicked OK
			long nID = dlg.GetFilterId();
		
			m_FilterCombo->Requery();

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_FilterCombo->GetNewRow();
			pRow->PutValue(fccID, (long)-1);
			pRow->PutValue(fccName, _bstr_t(" <Add New Filter>"));
			pRow->PutValue(fccFilter, "");		
			m_FilterCombo->AddRowSorted(pRow, NULL);

			OnSelChosenLwFilterCombo(m_FilterCombo->SetSelByColumn(fccID, nID));

			//send a tablechecker
			CClient::RefreshTable(NetUtils::FiltersT, nID);
		}

	}NxCatchAll("Error in CPatientInterestCalculationDlg::OnBtnEditLw");
}

void CPatientInterestCalculationDlg::OnRadioLwAll()
{
	try {

		if(m_radioAll.GetCheck()) {
			m_FilterCombo->Enabled = FALSE;
			m_GroupCombo->Enabled = FALSE;
			m_btnEditLW.EnableWindow(FALSE);

			//reset the where clause
			CString strWhere;
			strWhere.Format("Archived = 0 AND PersonID IN (%s)", m_strPatientIDs);

			m_SelectedList->PutWhereClause(_bstr_t(strWhere));
			m_SelectedList->Requery();
			m_UnselectedList->Clear();
		}
		else if(m_radioFilter.GetCheck()) {
			GetDlgItem(IDC_FC_LW_FILTER_COMBO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_FC_LW_GROUP_COMBO)->ShowWindow(SW_HIDE);
			m_FilterCombo->Enabled = TRUE;
			m_GroupCombo->Enabled = FALSE;
			m_btnEditLW.EnableWindow(g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrSilent));
			OnSelChosenLwFilterCombo(m_FilterCombo->GetCurSel());
		}
		else if(m_radioGroup.GetCheck()) {
			GetDlgItem(IDC_FC_LW_FILTER_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_FC_LW_GROUP_COMBO)->ShowWindow(SW_SHOW);
			m_FilterCombo->Enabled = FALSE;
			m_GroupCombo->Enabled = TRUE;
			m_btnEditLW.EnableWindow(FALSE);
			OnSelChosenLwGroupCombo(m_GroupCombo->GetCurSel());
		}		

	}NxCatchAll("Error in CPatientInterestCalculationDlg::OnRadioLwAll");
}

void CPatientInterestCalculationDlg::OnRadioLwFilter()
{
	try {

		OnRadioLwAll();

	}NxCatchAll("Error in CPatientInterestCalculationDlg::OnRadioLwFilter");
}

void CPatientInterestCalculationDlg::OnRadioLwGroup()
{
	try {

		OnRadioLwAll();

	}NxCatchAll("Error in CPatientInterestCalculationDlg::OnRadioLwGroup");
}

void CPatientInterestCalculationDlg::OnSelChosenLwFilterCombo(LPDISPATCH lpRow)
{
	try {

		CString strSelectedWhere;
		CString strUnselectedWhere;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//reset the where clause			
			strSelectedWhere.Format("Archived = 0 AND PersonID IN (%s)", m_strPatientIDs);
			m_SelectedList->PutWhereClause(_bstr_t(strSelectedWhere));
			m_SelectedList->Requery();
			m_UnselectedList->Clear();
		}
		else {
			//use this filter

			long nFilterID = VarLong(pRow->GetValue(fccID));
			if(nFilterID != -1) {
				CString strFilter = VarString(pRow->GetValue(fccFilter), "");
				CString strFilterFrom = "", strFilterWhere = "";

				if(!CFilter::ConvertFilterStringToClause(nFilterID, strFilter, 1, &strFilterWhere, &strFilterFrom)) {
					AfxMessageBox("The selected filter is invalid, and will not be applied to the patient list.");
					//reset the where clause			
					strSelectedWhere.Format("Archived = 0 AND PersonID IN (%s)", m_strPatientIDs);
					m_SelectedList->PutWhereClause(_bstr_t(strSelectedWhere));
					m_SelectedList->Requery();
					m_UnselectedList->Clear();
				}
				else {
					strSelectedWhere.Format("Archived = 0 "
						"AND PersonID IN (%s) "
						"AND PersonID IN (SELECT PersonT.ID FROM %s WHERE %s)", m_strPatientIDs, strFilterFrom, strFilterWhere);
					strUnselectedWhere.Format("Archived = 0 "
						"AND PersonID IN (%s) "
						"AND PersonID NOT IN (SELECT PersonT.ID FROM %s WHERE %s)", m_strPatientIDs, strFilterFrom, strFilterWhere);
					m_SelectedList->PutWhereClause(_bstr_t(strSelectedWhere));
					m_SelectedList->Requery();
					m_UnselectedList->PutWhereClause(_bstr_t(strUnselectedWhere));
					m_UnselectedList->Requery();
				}
			}
			else {
				//edit the filter, which will requery when it's finished
				OnBtnEditLw();
				return;
			}
		}

	}NxCatchAll("Error in CPatientInterestCalculationDlg::OnSelChosenLwFilterCombo");
}

void CPatientInterestCalculationDlg::OnSelChosenLwGroupCombo(LPDISPATCH lpRow)
{
	try {

		CString strSelectedWhere;
		CString strUnselectedWhere;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//reset the where clause		
			strSelectedWhere.Format("Archived = 0 AND PersonID IN (%s)", m_strPatientIDs);
			m_SelectedList->PutWhereClause(_bstr_t(strSelectedWhere));
			m_SelectedList->Requery();
			m_UnselectedList->Clear();
		}
		else {
			//use this group
			strSelectedWhere.Format("Archived = 0 "
				"AND PersonID IN (%s) "
				"AND PersonID IN (SELECT PersonID FROM GroupDetailsT WHERE GroupID = %li)",
				m_strPatientIDs, VarLong(pRow->GetValue(gccID)));
			strUnselectedWhere.Format("Archived = 0 "
				"AND PersonID IN (%s) "
				"AND PersonID NOT IN (SELECT PersonID FROM GroupDetailsT WHERE GroupID = %li)",
				m_strPatientIDs, VarLong(pRow->GetValue(gccID)));
			m_SelectedList->PutWhereClause(_bstr_t(strSelectedWhere));
			m_SelectedList->Requery();
			m_UnselectedList->PutWhereClause(_bstr_t(strUnselectedWhere));
			m_UnselectedList->Requery();
		}

	}NxCatchAll("Error in CPatientInterestCalculationDlg::OnSelChosenLwGroupCombo");
}
