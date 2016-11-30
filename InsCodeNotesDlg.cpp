// InsCodeNotesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsrc.h"
#include "InsCodeNotesDlg.h"
#include "DiagSearchUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInsCodeNotesDlg dialog

enum CodeColumns {
	ccID = 0,
	ccCode = 1,
	ccDescription = 2,
};

CInsCodeNotesDlg::CInsCodeNotesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInsCodeNotesDlg::IDD, pParent)
{
	m_bShowingServiceCodes = false;
	//{{AFX_DATA_INIT(CInsCodeNotesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CInsCodeNotesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInsCodeNotesDlg)
	DDX_Control(pDX, IDC_SERVICE_CODE_NOTES, m_btnSvcNotes);
	DDX_Control(pDX, IDC_DIAG_CODE_NOTES, m_btnDiagNotes);
	DDX_Control(pDX, IDC_NOTES_REMOVE_ALL_CODE, m_btnRemoveAllCode);
	DDX_Control(pDX, IDC_NOTES_REMOVE_CODE, m_btnRemoveCode);
	DDX_Control(pDX, IDC_NOTES_ADD_ALL_CODE, m_btnAddAllCode);
	DDX_Control(pDX, IDC_NOTES_ADD_CODE, m_btnAddCode);
	DDX_Control(pDX, IDC_NOTES_REMOVE_ALL_INS_CO, m_btnRemoveAllInsCo);
	DDX_Control(pDX, IDC_NOTES_REMOVE_INS_CO, m_btnRemoveInsCo);
	DDX_Control(pDX, IDC_NOTES_ADD_ALL_INS_CO, m_btnAddAllInsCo);
	DDX_Control(pDX, IDC_NOTES_ADD_INS_CO, m_btnAddInsCo);
	DDX_Control(pDX, IDC_INS_CODE_NOTE, m_nxeditInsCodeNote);
	DDX_Control(pDX, IDC_UNSELECTED_CODE_LABEL, m_nxstaticUnselectedCodeLabel);
	DDX_Control(pDX, IDC_SELECTED_CODE_LABEL, m_nxstaticSelectedCodeLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_APPLY_CODE_NOTE, m_btnApplyCodeNote);
	DDX_Control(pDX, IDC_INS_CPT_GROUPBOX, m_btnInsCptGroupbox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInsCodeNotesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInsCodeNotesDlg)
	ON_BN_CLICKED(IDC_APPLY_CODE_NOTE, OnApplyCodeNote)
	ON_BN_CLICKED(IDC_SERVICE_CODE_NOTES, OnServiceCodeNotes)
	ON_BN_CLICKED(IDC_DIAG_CODE_NOTES, OnDiagCodeNotes)
	ON_BN_CLICKED(IDC_NOTES_ADD_INS_CO, OnNotesAddInsCo)
	ON_BN_CLICKED(IDC_NOTES_ADD_ALL_INS_CO, OnNotesAddAllInsCo)
	ON_BN_CLICKED(IDC_NOTES_REMOVE_INS_CO, OnNotesRemoveInsCo)
	ON_BN_CLICKED(IDC_NOTES_REMOVE_ALL_INS_CO, OnNotesRemoveAllInsCo)
	ON_BN_CLICKED(IDC_NOTES_ADD_CODE, OnNotesAddCode)
	ON_BN_CLICKED(IDC_NOTES_ADD_ALL_CODE, OnNotesAddAllCode)
	ON_BN_CLICKED(IDC_NOTES_REMOVE_CODE, OnNotesRemoveCode)
	ON_BN_CLICKED(IDC_NOTES_REMOVE_ALL_CODE, OnNotesRemoveAllCode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInsCodeNotesDlg message handlers

BOOL CInsCodeNotesDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-30 13:23) - PLID 29847 - NxIconify the close button
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnApplyCodeNote.AutoSet(NXB_MODIFY);
		
		//Set up our controls.
		m_pInsCoAvail = BindNxDataListCtrl(this, IDC_CODE_NOTES_UNSELECTED_INS, GetRemoteData(), true);
		m_pInsCoSelect = BindNxDataListCtrl(this, IDC_CODE_NOTES_SELECTED_INS, GetRemoteData(), false);

		m_pCodeAvail = BindNxDataListCtrl(this, IDC_UNSELECTED_CODE_LIST, GetRemoteData(), false);
		m_pCodeSelect = BindNxDataListCtrl(this, IDC_SELECTED_CODE_LIST, GetRemoteData(), false);

		// (a.wilson 02/18/2014) PLID 60770 - new diagnosis search.
		m_pDiagnosisSearch = DiagSearchUtils::BindDiagDualSearchListCtrl(this, IDC_CODE_NOTES_DIAGNOSIS_SEARCH, GetRemoteData());

		m_btnAddInsCo.AutoSet(NXB_RIGHT);
		m_btnAddAllInsCo.AutoSet(NXB_RRIGHT);
		m_btnRemoveInsCo.AutoSet(NXB_LEFT);
		m_btnRemoveAllInsCo.AutoSet(NXB_LLEFT);
		m_btnAddCode.AutoSet(NXB_RIGHT);
		m_btnAddAllCode.AutoSet(NXB_RRIGHT);
		m_btnRemoveCode.AutoSet(NXB_LEFT);
		m_btnRemoveAllCode.AutoSet(NXB_LLEFT);

		((CNxEdit*)GetDlgItem(IDC_INS_CODE_NOTE))->SetLimitText(4000);

		//Default to Service Codes.
		CheckRadioButton(IDC_SERVICE_CODE_NOTES, IDC_DIAG_CODE_NOTES, IDC_SERVICE_CODE_NOTES);
		OnServiceCodeNotes();
	}
	NxCatchAll("Error in CInsCodeNotesDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInsCodeNotesDlg::OnApplyCodeNote() 
{
	if(m_pInsCoSelect->GetRowCount()==0) {
		MsgBox("You must have at least one insurance company selected.");
		return;
	}

	if(m_pCodeSelect->GetRowCount()==0) {
		if(m_bShowingServiceCodes) {
			MsgBox("You must have at least one service code selected.");
		}
		else {
			MsgBox("You must have at least one diagnosis code selected.");
		}
		return;
	}

	try {

		CString strNote, strMessage;
		GetDlgItemText(IDC_INS_CODE_NOTE,strNote);

		if(strNote.IsEmpty()) {
			if(m_bShowingServiceCodes) {
				strMessage = "This action will remove the popup warning for all selected service code/insurance company combinations.\nAre you sure you wish to do this?";
			}
			else {
				strMessage = "This action will remove the popup warning for all selected diagnosis code/insurance company combinations.\nAre you sure you wish to do this?";
			}
		}
		else {
			if(m_bShowingServiceCodes) {
				strMessage.Format("This action will set the popup warning to '%s' for all selected service code/insurance company combinations.\nAre you sure you wish to do this?", strNote);
			}
			else {
				strMessage.Format("This action will set the popup warning to '%s' for all selected diagnosis code/insurance company combinations.\nAre you sure you wish to do this?", strNote);
			}
		}
		if(IDNO == MsgBox(MB_YESNO, "%s", strMessage)) return;

		{
			CWaitCursor cuWait;
			for(int i = 0; i < m_pInsCoSelect->GetRowCount(); i++) {
				for(int j = 0; j < m_pCodeSelect->GetRowCount(); j++) {
					long nCodeID = VarLong(m_pCodeSelect->GetValue(j,0));
					long nInsCoID = VarLong(m_pInsCoSelect->GetValue(i,0));
					if(m_bShowingServiceCodes) {
						if(ReturnsRecords("SELECT Notes FROM CptInsNotesT WHERE CptCodeID = %li AND InsuranceCoID = %li", nCodeID, nInsCoID)) {
							ExecuteSql("UPDATE CptInsNotesT SET Notes = '%s' WHERE CptCodeID = %li AND InsuranceCoID = %li", _Q(strNote), nCodeID, nInsCoID);
						}
						else {
							ExecuteSql("INSERT INTO CptInsNotesT (CptCodeID, InsuranceCoID, Notes) "
								"VALUES (%li, %li, '%s')", nCodeID, nInsCoID, _Q(strNote));
						}
					}
					else {
						if(ReturnsRecords("SELECT Notes FROM DiagInsNotesT WHERE DiagCodeID = %li AND InsuranceCoID = %li", nCodeID, nInsCoID)) {
							ExecuteSql("UPDATE DiagInsNotesT SET Notes = '%s' WHERE DiagCodeID = %li AND InsuranceCoID = %li", _Q(strNote), nCodeID, nInsCoID);
						}
						else {
							ExecuteSql("INSERT INTO DiagInsNotesT (DiagCodeID, InsuranceCoID, Notes) "
								"VALUES (%li, %li, '%s')", nCodeID, nInsCoID, _Q(strNote));
						}
					}
				}
			}
		}
		MsgBox("Warning text successfully applied!");
	}NxCatchAll("Error in CInsCodeNotesDlg::OnApplyCodeNote()");

}

void CInsCodeNotesDlg::OnServiceCodeNotes() 
{
	SetDlgItemText(IDC_SELECTED_CODE_LABEL, "Selected Service Codes");
	SetDlgItemText(IDC_UNSELECTED_CODE_LABEL, "Unselected Service Codes");

	//Don't requery if we're already showing service codes.
	if(!m_bShowingServiceCodes) {
		m_pCodeAvail->FromClause = _bstr_t("(SELECT ServiceT.ID, ServiceT.Name, CptCodeT.Code "
			"FROM ServiceT INNER JOIN CptCodeT ON ServiceT.ID = CptCodeT.ID WHERE ServiceT.Active = 1) SubQ");
		m_pCodeAvail->Requery();
		m_pCodeSelect->Clear();
		m_btnRemoveCode.EnableWindow(FALSE);
		m_btnRemoveAllCode.EnableWindow(FALSE);
		m_btnAddCode.EnableWindow(FALSE);
		// (a.wilson 02/18/2014) - PLID 60770 - add diagnosis search. grow selected list 
		m_pDiagnosisSearch->Enabled = VARIANT_FALSE;
		GetDlgItem(IDC_CODE_NOTES_DIAGNOSIS_SEARCH)->ShowWindow(SW_HIDE);
		CRect rcCodeSelect;
		GetDlgItem(IDC_SELECTED_CODE_LIST)->GetWindowRect(&rcCodeSelect);
		ScreenToClient(&rcCodeSelect);
		GetDlgItem(IDC_SELECTED_CODE_LIST)->MoveWindow(rcCodeSelect.left, 263, rcCodeSelect.Width(), 197);
		m_bShowingServiceCodes = true;
	}
}

void CInsCodeNotesDlg::OnDiagCodeNotes() 
{
	SetDlgItemText(IDC_SELECTED_CODE_LABEL, "Selected Diagnosis Codes");
	SetDlgItemText(IDC_UNSELECTED_CODE_LABEL, "Unselected Diagnosis Codes");

	//Don't requery if we're already showing diagnosis codes.
	if(m_bShowingServiceCodes) {
		m_pCodeAvail->FromClause = _bstr_t("(SELECT DiagCodes.ID AS ID, DiagCodes.CodeDesc AS Name, DiagCodes.CodeNumber AS Code "
			"FROM DiagCodes WHERE DiagCodes.Active = 1) SubQ");
		m_pCodeAvail->Requery();
		m_pCodeSelect->Clear();
		m_btnRemoveCode.EnableWindow(FALSE);
		m_btnRemoveAllCode.EnableWindow(FALSE);
		m_btnAddCode.EnableWindow(FALSE);
		// (a.wilson 02/18/2014) - PLID 60770 - add diagnosis search. shrink selection to fit search.
		m_pDiagnosisSearch->Enabled = VARIANT_TRUE;
		GetDlgItem(IDC_CODE_NOTES_DIAGNOSIS_SEARCH)->ShowWindow(SW_SHOW);
		CRect rcCodeSelect;
		GetDlgItem(IDC_SELECTED_CODE_LIST)->GetWindowRect(&rcCodeSelect);
		ScreenToClient(&rcCodeSelect);
		GetDlgItem(IDC_SELECTED_CODE_LIST)->MoveWindow(rcCodeSelect.left, 293, rcCodeSelect.Width(), 167);
		m_bShowingServiceCodes = false;
	}
}

void CInsCodeNotesDlg::OnNotesAddInsCo() 
{
	if(m_pInsCoAvail->CurSel != -1) {
		m_pInsCoSelect->TakeCurrentRow(m_pInsCoAvail);
		m_btnAddInsCo.EnableWindow(m_pInsCoAvail->CurSel != -1);
		m_btnRemoveInsCo.EnableWindow(m_pInsCoSelect->CurSel != -1);
		m_btnAddAllInsCo.EnableWindow(m_pInsCoAvail->GetRowCount());
		m_btnRemoveAllInsCo.EnableWindow(TRUE);
	}
}

void CInsCodeNotesDlg::OnNotesAddAllInsCo() 
{
	m_pInsCoSelect->TakeAllRows(m_pInsCoAvail);
	m_btnAddInsCo.EnableWindow(FALSE);
	m_btnAddAllInsCo.EnableWindow(FALSE);
	m_btnRemoveInsCo.EnableWindow(m_pInsCoSelect->CurSel != -1);
	m_btnRemoveAllInsCo.EnableWindow(TRUE);
}

void CInsCodeNotesDlg::OnNotesRemoveInsCo() 
{
	if(m_pInsCoSelect->CurSel != -1) {
		m_pInsCoAvail->TakeCurrentRow(m_pInsCoSelect);
		m_btnAddInsCo.EnableWindow(m_pInsCoAvail->CurSel != -1);
		m_btnRemoveInsCo.EnableWindow(m_pInsCoSelect->CurSel != -1);
		m_btnRemoveAllInsCo.EnableWindow(m_pInsCoSelect->GetRowCount());
		m_btnAddAllInsCo.EnableWindow(TRUE);
	}
}

void CInsCodeNotesDlg::OnNotesRemoveAllInsCo() 
{
	m_pInsCoAvail->TakeAllRows(m_pInsCoSelect);
	m_btnRemoveInsCo.EnableWindow(FALSE);
	m_btnRemoveAllInsCo.EnableWindow(FALSE);
	m_btnAddInsCo.EnableWindow(m_pInsCoAvail->CurSel != -1);
	m_btnAddAllInsCo.EnableWindow(TRUE);
}

void CInsCodeNotesDlg::OnNotesAddCode() 
{
	if(m_pCodeAvail->CurSel != -1) {
		m_pCodeSelect->TakeCurrentRow(m_pCodeAvail);
		m_btnAddCode.EnableWindow(m_pCodeAvail->CurSel != -1);
		m_btnRemoveCode.EnableWindow(m_pCodeSelect->CurSel != -1);
		m_btnAddAllCode.EnableWindow(m_pCodeAvail->GetRowCount());
		m_btnRemoveAllCode.EnableWindow(TRUE);
	}
}

void CInsCodeNotesDlg::OnNotesAddAllCode() 
{
	m_pCodeSelect->TakeAllRows(m_pCodeAvail);
	m_btnAddCode.EnableWindow(FALSE);
	m_btnAddAllCode.EnableWindow(FALSE);
	m_btnRemoveCode.EnableWindow(m_pCodeSelect->CurSel != -1);
	m_btnRemoveAllCode.EnableWindow(TRUE);
}

void CInsCodeNotesDlg::OnNotesRemoveCode() 
{
	if(m_pCodeSelect->CurSel != -1) {
		m_pCodeAvail->TakeCurrentRow(m_pCodeSelect);
		m_btnAddCode.EnableWindow(m_pCodeAvail->CurSel != -1);
		m_btnRemoveCode.EnableWindow(m_pCodeSelect->CurSel != -1);
		m_btnRemoveAllCode.EnableWindow(m_pCodeSelect->GetRowCount());
		m_btnAddAllCode.EnableWindow(TRUE);
	}
}

void CInsCodeNotesDlg::OnNotesRemoveAllCode() 
{
	m_pCodeAvail->TakeAllRows(m_pCodeSelect);
	m_btnRemoveCode.EnableWindow(FALSE);
	m_btnRemoveAllCode.EnableWindow(FALSE);
	m_btnAddCode.EnableWindow(m_pCodeAvail->CurSel != -1);
	m_btnAddAllCode.EnableWindow(TRUE);
}

BEGIN_EVENTSINK_MAP(CInsCodeNotesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInsCodeNotesDlg)
	ON_EVENT(CInsCodeNotesDlg, IDC_CODE_NOTES_UNSELECTED_INS, 2 /* SelChanged */, OnSelChangedCodeNotesUnselectedIns, VTS_I4)
	ON_EVENT(CInsCodeNotesDlg, IDC_CODE_NOTES_SELECTED_INS, 2 /* SelChanged */, OnSelChangedCodeNotesSelectedIns, VTS_I4)
	ON_EVENT(CInsCodeNotesDlg, IDC_UNSELECTED_CODE_LIST, 2 /* SelChanged */, OnSelChangedUnselectedCodeList, VTS_I4)
	ON_EVENT(CInsCodeNotesDlg, IDC_SELECTED_CODE_LIST, 2 /* SelChanged */, OnSelChangedSelectedCodeList, VTS_I4)
	ON_EVENT(CInsCodeNotesDlg, IDC_CODE_NOTES_UNSELECTED_INS, 3 /* DblClickCell */, OnDblClickCellCodeNotesUnselectedIns, VTS_I4 VTS_I2)
	ON_EVENT(CInsCodeNotesDlg, IDC_CODE_NOTES_SELECTED_INS, 3 /* DblClickCell */, OnDblClickCellCodeNotesSelectedIns, VTS_I4 VTS_I2)
	ON_EVENT(CInsCodeNotesDlg, IDC_SELECTED_CODE_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedCodeList, VTS_I4 VTS_I2)
	ON_EVENT(CInsCodeNotesDlg, IDC_UNSELECTED_CODE_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedCodeList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CInsCodeNotesDlg, IDC_CODE_NOTES_DIAGNOSIS_SEARCH, 16, CInsCodeNotesDlg::SelChosenCodeNotesDiagnosisSearch, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CInsCodeNotesDlg::OnSelChangedCodeNotesUnselectedIns(long nNewSel) 
{
	m_btnAddInsCo.EnableWindow(nNewSel != -1);
}

void CInsCodeNotesDlg::OnSelChangedCodeNotesSelectedIns(long nNewSel) 
{
	m_btnRemoveInsCo.EnableWindow(nNewSel != -1);
}

void CInsCodeNotesDlg::OnSelChangedUnselectedCodeList(long nNewSel) 
{
	m_btnAddCode.EnableWindow(nNewSel != -1);
}

void CInsCodeNotesDlg::OnSelChangedSelectedCodeList(long nNewSel) 
{
	m_btnRemoveCode.EnableWindow(nNewSel != -1);
}

void CInsCodeNotesDlg::OnDblClickCellCodeNotesUnselectedIns(long nRowIndex, short nColIndex) 
{
	OnNotesAddInsCo();
}

void CInsCodeNotesDlg::OnDblClickCellCodeNotesSelectedIns(long nRowIndex, short nColIndex) 
{
	OnNotesRemoveInsCo();
}

void CInsCodeNotesDlg::OnDblClickCellSelectedCodeList(long nRowIndex, short nColIndex) 
{
	OnNotesRemoveCode();
}

void CInsCodeNotesDlg::OnDblClickCellUnselectedCodeList(long nRowIndex, short nColIndex) 
{
	OnNotesAddCode();
}

// (a.wilson 02/18/2014) PLID 60770 - add diagnosis code from search to selected list.
void CInsCodeNotesDlg::SelChosenCodeNotesDiagnosisSearch(LPDISPATCH lpRow)
{
	try {
		if (lpRow) {
			long nDiagID = -1;
			CString strDiagCode, strDiagDescription;
			CDiagSearchResults results = DiagSearchUtils::ConvertDualSearchResults(lpRow);
			//Get the code that was selected, it could be either a 9 or 10 code not both.
			if (results.m_ICD9.m_nDiagCodesID != -1) {
				nDiagID = results.m_ICD9.m_nDiagCodesID;
				strDiagCode = results.m_ICD9.m_strCode;
				strDiagDescription = results.m_ICD9.m_strDescription;
			} else if (results.m_ICD10.m_nDiagCodesID != -1) {
				nDiagID = results.m_ICD10.m_nDiagCodesID;
				strDiagCode = results.m_ICD10.m_strCode;
				strDiagDescription = results.m_ICD10.m_strDescription;
			} else {
				return; //empty row.
			}
			//ensure code doesn't exist in selected list.
			long p = m_pCodeSelect->GetFirstRowEnum();
			while (p) {
				IDispatchPtr pDisp;
				m_pCodeSelect->GetNextRowEnum(&p, &pDisp);
				NXDATALISTLib::IRowSettingsPtr pRow = pDisp;
				//ensure the row isn't empty.
				if (!pRow) {
					continue;
				}
				// check if its the same diagnosis code.
				if (AsLong(pRow->GetValue(ccID)) == nDiagID) {
					return;  //cancel selection if its already in the list.
				}
			}
			//remove row from unselected list and add to selected list.
			bool bAdded = false;
			p = m_pCodeAvail->SearchByColumn(ccID, _bstr_t(nDiagID), 0, VARIANT_FALSE);
			if (p != -1) {
				NXDATALISTLib::IRowSettingsPtr pRow = m_pCodeAvail->GetRow(p);
				if (pRow) {
					m_pCodeSelect->TakeRow(pRow);
					//if the unselected list is empty now then disable the controls.
					m_btnAddCode.EnableWindow(m_pCodeAvail->CurSel != -1);
					m_btnRemoveCode.EnableWindow(m_pCodeSelect->CurSel != -1);
					m_btnAddAllCode.EnableWindow(m_pCodeAvail->GetRowCount());
					m_btnRemoveAllCode.EnableWindow(TRUE); //enable removal.
					bAdded = true;
				}
			}
			//if we couldn't add it from the unselected list then add it from the search list.
			if (!bAdded) {
				NXDATALISTLib::IRowSettingsPtr pRow = m_pCodeSelect->GetRow(NXDATALISTLib::sriGetNewRow);
				pRow->PutValue(ccID, nDiagID);
				pRow->PutValue(ccCode, _bstr_t(strDiagCode));
				pRow->PutValue(ccDescription, _bstr_t(strDiagDescription));
				m_pCodeSelect->AddRow(pRow);
				m_btnRemoveCode.EnableWindow(m_pCodeSelect->CurSel != -1);
				m_btnRemoveAllCode.EnableWindow(TRUE); //enable removal.
			}
		}
	} NxCatchAll(__FUNCTION__);
}
