// PrescriptionDiagnosisCodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "PrescriptionDiagnosisCodeDlg.h"
#include "SelectDlg.h"
#include "DiagSearchUtils.h"
#include "PatientsRc.h" // (r.gonet 03/21/2014) - PLID 61187 - Added the resource header to avoid rebuilds

// (b.savon 2013-09-24 08:05) - PLID 58747 - Move the Diagnosis Codes from the Prescription Edit screen to a separate dialog - Created

// CPrescriptionDiagnosisCodeDlg dialog

IMPLEMENT_DYNAMIC(CPrescriptionDiagnosisCodeDlg, CNxDialog)

enum DiagListColumns {
	dlcID = 0,
	dlcCode = 1,
	dlcDescription = 2,
	dlcSortOrder = 3,
};

CPrescriptionDiagnosisCodeDlg::CPrescriptionDiagnosisCodeDlg(long nPatientMedicationID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CPrescriptionDiagnosisCodeDlg::IDD, pParent)
{
	m_nPatientMedicationID = nPatientMedicationID;
	m_bCommitChanges = FALSE;
	m_strDiagnosisCodeDisplay = "";
}

CPrescriptionDiagnosisCodeDlg::~CPrescriptionDiagnosisCodeDlg()
{
}

void CPrescriptionDiagnosisCodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NXC_PRESCRIPTION_DIAG_CODE, m_nxcBackground);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_MOVE_DIAG_UP, m_btnMoveDiagUp);
	DDX_Control(pDX, IDC_MOVE_DIAG_DOWN, m_btnMoveDiagDown);
}

BOOL CPrescriptionDiagnosisCodeDlg::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog();

		m_nxcBackground.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnMoveDiagUp.AutoSet(NXB_UP);
		m_btnMoveDiagDown.AutoSet(NXB_DOWN);
		// (r.gonet 03/21/2014) - PLID 61187 - Bind the diagnosis dual search
		m_nxdlDiagnosisSearch = DiagSearchUtils::BindDiagDualSearchListCtrl(this, IDC_RX_DIAG_CODE_SEARCH, GetRemoteData());
		m_nxdlDiagnosisCodes = BindNxDataList2Ctrl(IDC_DIAGNOSIS_CODES, false);
		m_nxdlDiagnosisCodes->WhereClause = _bstr_t(FormatString("PatientMedicationID = %li", m_nPatientMedicationID));
		m_nxdlDiagnosisCodes->Requery();

		return TRUE;
	}NxCatchAll(__FUNCTION__);
	return FALSE;
}


BEGIN_MESSAGE_MAP(CPrescriptionDiagnosisCodeDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CPrescriptionDiagnosisCodeDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_MOVE_DIAG_UP, &CPrescriptionDiagnosisCodeDlg::OnMoveDiagUp)
	ON_BN_CLICKED(IDC_MOVE_DIAG_DOWN, &CPrescriptionDiagnosisCodeDlg::OnMoveDiagDown)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CPrescriptionDiagnosisCodeDlg, CNxDialog)
	ON_EVENT(CPrescriptionDiagnosisCodeDlg, IDC_DIAGNOSIS_CODES, 2, CPrescriptionDiagnosisCodeDlg::OnSelChangedDiagnosisCodes, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CPrescriptionDiagnosisCodeDlg, IDC_DIAGNOSIS_CODES, 7, CPrescriptionDiagnosisCodeDlg::OnRButtonUpDiagnosisCodes, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPrescriptionDiagnosisCodeDlg, IDC_RX_DIAG_CODE_SEARCH, 16, CPrescriptionDiagnosisCodeDlg::SelChosenRxDiagCodeSearch, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CPrescriptionDiagnosisCodeDlg::EnableDiagButtons()
{
	NXDATALIST2Lib::IRowSettingsPtr pNewSel = m_nxdlDiagnosisCodes->GetCurSel();

	if(pNewSel == NULL) {
		m_btnMoveDiagDown.EnableWindow(FALSE);
		m_btnMoveDiagUp.EnableWindow(FALSE);
		return;
	}
	long nSortOrder = VarLong(pNewSel->GetValue(dlcSortOrder));
	if(nSortOrder < m_nxdlDiagnosisCodes->GetRowCount()-1) {
		m_btnMoveDiagDown.EnableWindow(TRUE);
	}
	else {
		m_btnMoveDiagDown.EnableWindow(FALSE);
	}
	if(nSortOrder > 0) {
		m_btnMoveDiagUp.EnableWindow(TRUE);
	}
	else {
		m_btnMoveDiagUp.EnableWindow(FALSE);
	}
}

// (r.gonet 03/21/2014) - PLID 61187 - Try to add a selected search result diagnosis code to 
void CPrescriptionDiagnosisCodeDlg::AddDiagCode(const CDiagSearchResults::CDiagCode& diagCode)
{
	try {
		if(diagCode.m_nDiagCodesID == -1) {
			// (r.gonet 03/21/2014) - PLID 61187 - This shouldn't be possible. We checked for it in the OnSelChosen handler.
			ThrowNxException("%s : Attempted to add a diagnosis code with ID of -1.");
			return;
		}
		//Don't let them add one that's already in the list.
		if(m_nxdlDiagnosisCodes->GetRowCount()) {
			CString strIDList;
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlDiagnosisCodes->GetFirstRow();
			while(pRow) {
				// (r.gonet 03/21/2014) - PLID 61187 - Compare what the user selected versus what we already have.
				long nExistingDiagCodeID = VarLong(pRow->GetValue(dlcID));
				if(diagCode.m_nDiagCodesID == nExistingDiagCodeID) {
					MessageBox("This code is already selected and cannot be duplicated.", "Duplicate Code", MB_ICONERROR|MB_OK);
					return;
				}
				pRow = pRow->GetNextRow();
			}
		}

		//Add a row for the code they selected.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlDiagnosisCodes->GetNewRow();
		// (r.gonet 03/21/2014) - PLID 61187 - Get the diagnosis properties from the search result.
		pRow->PutValue(dlcID, diagCode.m_nDiagCodesID);
		pRow->PutValue(dlcCode, _variant_t(diagCode.m_strCode));
		pRow->PutValue(dlcDescription, _variant_t(diagCode.m_strDescription));
		pRow->PutValue(dlcSortOrder, m_nxdlDiagnosisCodes->GetRowCount());
		m_nxdlDiagnosisCodes->AddRowSorted(pRow, NULL);
		//Commit changes
		CommitDiagnosisCodeList();

	}NxCatchAll(__FUNCTION__);
}

void CPrescriptionDiagnosisCodeDlg::OnSelChangedDiagnosisCodes(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		EnableDiagButtons();
	}NxCatchAll(__FUNCTION__);
}

void CPrescriptionDiagnosisCodeDlg::CommitDiagnosisCodeList()
{
	CArray<int,int> aryDiagnosisCodes;
	CString strCode;
	CString strDescription;
	CString strDisplay;
	m_strDiagnosisCodeDisplay = "";
	//Fill the CArray with all diagnosis codes
	NXDATALIST2Lib::IRowSettingsPtr pRowDiagList = m_nxdlDiagnosisCodes->GetFirstRow();
	while(pRowDiagList)
	{
		aryDiagnosisCodes.Add(pRowDiagList->GetValue(dlcID));
		strCode = VarString(pRowDiagList->GetValue(dlcCode), "");
		strDescription = VarString(pRowDiagList->GetValue(dlcDescription), "");
		if( strCode.IsEmpty() ){
			strDisplay = strDescription;
		}else{
			if( strDescription.IsEmpty() ){
				strDisplay = strCode;
			}else{
				strDisplay = strCode + " - " + strDescription;
			}
		}
			
		m_strDiagnosisCodeDisplay += strDisplay;
		pRowDiagList = pRowDiagList->GetNextRow();
		if( pRowDiagList != NULL ){
			m_strDiagnosisCodeDisplay += ", ";
		}
	}

	//Convert into a SafeArray and create a API struct for it
	m_saryDiagnosisCodes = Nx::SafeArray<int>::From(aryDiagnosisCodes);
	m_bCommitChanges = TRUE;
}

void CPrescriptionDiagnosisCodeDlg::OnMoveDiagUp()
{
	try {
		//We just need to switch the two rows
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlDiagnosisCodes->CurSel;

		if (pRow) {
			if (pRow != m_nxdlDiagnosisCodes->GetFirstRow()) {
				long nOrigOrderID = VarLong(pRow->GetValue(dlcSortOrder));
				long nNewOrderID = nOrigOrderID - 1;

				pRow->PutValue(dlcSortOrder, nNewOrderID);

				pRow = pRow->GetPreviousRow();

				if (pRow) {
					pRow->PutValue(dlcSortOrder, nOrigOrderID);
				}
				m_nxdlDiagnosisCodes->Sort();

				EnableDiagButtons();
				//Commit changes
				CommitDiagnosisCodeList();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CPrescriptionDiagnosisCodeDlg::OnMoveDiagDown()
{
	try {
		//We just need to switch the two rows
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlDiagnosisCodes->GetCurSel();

		if (pRow) {
			if (pRow != m_nxdlDiagnosisCodes->GetLastRow()) {
				long nOrigOrderID = VarLong(pRow->GetValue(dlcSortOrder));
				long nNewOrderID = nOrigOrderID + 1;

				pRow->PutValue(dlcSortOrder, nNewOrderID);

				pRow = pRow->GetNextRow();

				if (pRow) {
					pRow->PutValue(dlcSortOrder, nOrigOrderID);
				}
				m_nxdlDiagnosisCodes->Sort();

				EnableDiagButtons();
				//Commit changes
				CommitDiagnosisCodeList();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CPrescriptionDiagnosisCodeDlg::OnRButtonUpDiagnosisCodes(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if(pRow != NULL) {
			if(!pRow->IsHighlighted()) {
				m_nxdlDiagnosisCodes->PutCurSel(pRow);
			}
		} else {
			// Don't show the context menu on no row.
			return;
		}

		// (r.gonet 03/21/2014) - PLID 61187 - Removed ability to Add since we now have a search box rather than a Single Select popup
		enum EMenuOptions
		{
			moDelete = 1,
		};

		CMenu mnu;
		mnu.CreatePopupMenu();

		UINT nRowSelectedFlag = pRow == NULL ? MF_GRAYED : MF_ENABLED;
		mnu.AppendMenu(nRowSelectedFlag, moDelete, "Delete");

		CPoint ptClicked(x, y);
		GetDlgItem(IDC_DIAGNOSIS_CODES)->ClientToScreen(&ptClicked);
		int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptClicked.x, ptClicked.y, this);

		switch(nResult)
		{
			case moDelete:
				{
					m_nxdlDiagnosisCodes->RemoveRow(pRow);

					int nSortOrder = 0;
					NXDATALIST2Lib::IRowSettingsPtr pSortRow = m_nxdlDiagnosisCodes->GetFirstRow();
					while(pSortRow) {
						pSortRow->PutValue(dlcSortOrder, (long)nSortOrder);
						nSortOrder++;
						pSortRow = pSortRow->GetNextRow();
					}

					CommitDiagnosisCodeList();
				}
				break;

			default:
				ASSERT(nResult == 0);
				break;
		}

	}NxCatchAll(__FUNCTION__);
}

// CPrescriptionDiagnosisCodeDlg message handlers

void CPrescriptionDiagnosisCodeDlg::OnBnClickedOk()
{
	try{
		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 03/21/2014) - PLID 61187 - Handle the case where the user chooses a search result from the diagnosis dual search
void CPrescriptionDiagnosisCodeDlg::SelChosenRxDiagCodeSearch(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(!pRow) {
			// (r.gonet 03/21/2014) - PLID 61187 -No row selected
			return;
		}

		CDiagSearchResults diagSearchResults = DiagSearchUtils::ConvertDualSearchResults(pRow);
		if(diagSearchResults.m_ICD9.m_nDiagCodesID == -1 && diagSearchResults.m_ICD10.m_nDiagCodesID == -1) {
			// (r.gonet 03/21/2014) - PLID 61187 -No search result selected
			return;
		}

		AddDiagCode(diagSearchResults.m_ICD10.m_nDiagCodesID != -1 ? diagSearchResults.m_ICD10 : diagSearchResults.m_ICD9);
	} NxCatchAll(__FUNCTION__);
}
