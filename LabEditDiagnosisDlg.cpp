// LabEditDiagnosisDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DiagSearchUtils.h"
#include "LabEditDiagnosisDlg.h"
#include "LabEditDiagnosisLinkDlg.h"

using namespace NXDATALIST2Lib;

// CLabEditDiagnosisDlg dialog
// (z.manning 2008-10-27 09:36) - PLID 24630 - Created

IMPLEMENT_DYNAMIC(CLabEditDiagnosisDlg, CNxDialog)

CLabEditDiagnosisDlg::CLabEditDiagnosisDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLabEditDiagnosisDlg::IDD, pParent)
{

}

CLabEditDiagnosisDlg::~CLabEditDiagnosisDlg()
{
}

void CLabEditDiagnosisDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_ADD_LAB_DIAG, m_btnAdd);
	DDX_Control(pDX, IDC_EDIT_LAB_DIAG, m_btnEdit);
	DDX_Control(pDX, IDC_DELETE_LAB_DIAG, m_btnDelete);
}


BEGIN_MESSAGE_MAP(CLabEditDiagnosisDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_LAB_DIAG, &CLabEditDiagnosisDlg::OnBnClickedAddLabDiag)
	ON_BN_CLICKED(IDC_EDIT_LAB_DIAG, &CLabEditDiagnosisDlg::OnBnClickedEditLabDiag)
	ON_BN_CLICKED(IDC_DELETE_LAB_DIAG, &CLabEditDiagnosisDlg::OnBnClickedDeleteLabDiag)
	ON_BN_CLICKED(IDC_LINK_DIAGNOSIS, &CLabEditDiagnosisDlg::OnBnClickedLinkDiagnosis)
END_MESSAGE_MAP()


// CLabEditDiagnosisDlg message handlers

BOOL CLabEditDiagnosisDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnEdit.AutoSet(NXB_MODIFY);
		m_btnDelete.AutoSet(NXB_DELETE);

		// (r.gonet 03/03/2014) - PLID 60776 - Replace the old dropdown with 9/10 dual search code box.
		m_pdlDiagCombo = DiagSearchUtils::BindDiagDualSearchListCtrl(this, IDC_SELECT_DIAG_COMBO, GetRemoteData());
		m_pdlLabDiagList = BindNxDataList2Ctrl(IDC_LAB_DIAGNOSIS_LIST, true);

	}NxCatchAll("CLabEditDiagnosisDlg::OnInitDialog");

	return TRUE;
}

void CLabEditDiagnosisDlg::OnBnClickedAddLabDiag()
{
	try
	{
		IRowSettingsPtr pNewRow = m_pdlLabDiagList->GetNewRow();
		pNewRow->PutValue(ldlcID, g_cvarNull);
		pNewRow->PutValue(ldlcDiag, g_cvarNull);
		pNewRow->PutValue(ldlcDescription, "[New lab diagnosis]");
		pNewRow->PutValue(ldlcResultComment, "");
		m_pdlLabDiagList->AddRowAtEnd(pNewRow, NULL);
		// (r.gonet 03/27/2014) - PLID 60776 - Ensure the new row is in view so you can see what you are typing.
		m_pdlLabDiagList->EnsureRowInView(pNewRow);
		m_pdlLabDiagList->StartEditing(pNewRow, ldlcDescription);

	}NxCatchAll("CLabEditDiagnosisDlg::OnBnClickedAddLabDiag");
}

void CLabEditDiagnosisDlg::OnBnClickedEditLabDiag()
{
	try
	{
		IRowSettingsPtr pRow = m_pdlLabDiagList->GetCurSel();
		if(pRow == NULL) {
			MessageBox("Select a row first");
			return;
		}

		m_pdlLabDiagList->StartEditing(pRow, ldlcDescription);

	}NxCatchAll("CLabEditDiagnosisDlg::OnBnClickedEditLabDiag");
}

void CLabEditDiagnosisDlg::OnBnClickedDeleteLabDiag()
{
	try
	{
		IRowSettingsPtr pRow = m_pdlLabDiagList->GetCurSel();
		if(pRow == NULL) {
			MessageBox("Select a row first");
			return;
		}

		if(IDYES != MessageBox("Are you sure you want to delete the selected lab diagnosis?", NULL, MB_YESNO)) {
			return;
		}

		long nID = VarLong(pRow->GetValue(ldlcID));

		// (j.gruber 2016-02-10 10:55) - PLID 68154 - also delete from LabClinicalDiagnosisLinkT
		ExecuteParamSql("DELETE FROM LabClinicalDiagnosisLinkT WHERE LabDiagnosisID = {INT}", nID);

		ExecuteParamSql("DELETE FROM LabDiagnosisT WHERE ID = {INT}", nID);
		m_pdlLabDiagList->RemoveRow(pRow);

	}NxCatchAll("CLabEditDiagnosisDlg::OnBnClickedDeleteLabDiag");
}

BEGIN_EVENTSINK_MAP(CLabEditDiagnosisDlg, CNxDialog)
	ON_EVENT(CLabEditDiagnosisDlg, IDC_SELECT_DIAG_COMBO, 16, CLabEditDiagnosisDlg::SelChosenSelectDiagCombo, VTS_DISPATCH)
	ON_EVENT(CLabEditDiagnosisDlg, IDC_LAB_DIAGNOSIS_LIST, 10, CLabEditDiagnosisDlg::EditingFinishedLabDiagnosisList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CLabEditDiagnosisDlg, IDC_LAB_DIAGNOSIS_LIST, 2, CLabEditDiagnosisDlg::SelChangedLabDiagnosisList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CLabEditDiagnosisDlg, IDC_LAB_DIAGNOSIS_LIST, 18, CLabEditDiagnosisDlg::RequeryFinishedLabDiagnosisList, VTS_I2)
END_EVENTSINK_MAP()

void CLabEditDiagnosisDlg::SelChosenSelectDiagCombo(LPDISPATCH lpRow)
{
	try
	{
		IRowSettingsPtr pBaseRow(lpRow);
		if(pBaseRow == NULL) {
			return;
		}

		// (r.gonet 03/03/2014) - PLID 60776 - A dual search diagnosis code datalist has its own column definitions,
		// so we must let the controller give extract the data from the row's cells.
		CDiagSearchResults results = DiagSearchUtils::ConvertDualSearchResults(pBaseRow);

		// (r.gonet 03/03/2014) - PLID 60776 - Since this is a dual search, we have either an ICD-9 xor an ICD-10
		long nDiagID;
		CString strDescription;
		if(results.m_ICD9.m_nDiagCodesID != -1) {
			nDiagID = results.m_ICD9.m_nDiagCodesID;
			strDescription = results.m_ICD9.m_strDescription;
		} else if(results.m_ICD10.m_nDiagCodesID != -1) {
			nDiagID = results.m_ICD10.m_nDiagCodesID;
			strDescription = results.m_ICD10.m_strDescription;
		} else {
			// (r.gonet 03/20/2014) - PLID 60776 - This can occur if they select the no results found row or just press enter at the search prompt.
			return;
		}

		ADODB::_RecordsetPtr prsInsert = CreateParamRecordset(
			"SET NOCOUNT ON \r\n"
			"INSERT INTO LabDiagnosisT (Description, DiagID) VALUES ({STRING}, {INT}) \r\n"
			"SET NOCOUNT OFF \r\n"
			"SELECT convert(int, SCOPE_IDENTITY()) AS NewID \r\n"
			, strDescription, nDiagID);

		NXDATALIST2Lib::IColumnSettingsPtr pDiagColumn = m_pdlLabDiagList->GetColumn(ldlcDiag);
		// (r.gonet 03/08/2014) - PLID 60776 - Force a refresh since an ICD code may just have been imported.
		pDiagColumn->ComboSource = pDiagColumn->ComboSource;
		
		IRowSettingsPtr pDestRow = m_pdlLabDiagList->GetNewRow();
		pDestRow->PutValue(ldlcID, AdoFldLong(prsInsert->GetFields(), "NewID"));
		pDestRow->PutValue(ldlcDiag, nDiagID);
		pDestRow->PutValue(ldlcDescription, _bstr_t(strDescription));
		pDestRow->PutValue(ldlcResultComment, ""); 
		
		m_pdlLabDiagList->AddRowSorted(pDestRow, NULL);
		m_pdlLabDiagList->EnsureRowInView(pDestRow);

		m_pdlDiagCombo->PutCurSel(NULL);
		m_pdlLabDiagList->PutCurSel(pDestRow);
		SelChangedLabDiagnosisList(NULL, pDestRow);
		GetDlgItem(IDC_LAB_DIAGNOSIS_LIST)->SetFocus();

	}NxCatchAll("CLabEditDiagnosisDlg::SelChosenSelectDiagCombo");
}

// (r.gonet 03/27/2014) - PLID 60776 - Restored function from an errant delete.
void CLabEditDiagnosisDlg::EditingFinishedLabDiagnosisList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		long nID = VarLong(pRow->GetValue(ldlcID), -1);
		long nDiagID = VarLong(pRow->GetValue(ldlcDiag), -1);
		CString strDescription = VarString(pRow->GetValue(ldlcDescription), "");
		CString strResultComment = VarString(pRow->GetValue(ldlcResultComment), "");

		if(bCommit)
		{
			switch(nCol)
			{
				// (r.gonet 03/27/2014) - PLID 60776 - We no longer have the ability to edit the code column for free text codes.

				case ldlcDescription:
					if(nID == -1) {
						// (z.manning 2008-10-27 11:43) - New lab diagnosis
						ADODB::_RecordsetPtr prsInsert = CreateParamRecordset(
							"SET NOCOUNT ON \r\n"
							"INSERT INTO LabDiagnosisT (Description) VALUES ({STRING}) \r\n"
							"SET NOCOUNT OFF \r\n"
							"SELECT convert(int, SCOPE_IDENTITY()) AS NewID \r\n"
							, strDescription);
						pRow->PutValue(ldlcID, AdoFldLong(prsInsert->GetFields(), "NewID"));
					}
					else {
						// (z.manning 2008-10-27 11:46) - Existing lab description
						ExecuteParamSql("UPDATE LabDiagnosisT SET Description = {STRING} WHERE ID = {INT}"
							, strDescription, nID);
					}
					break;
				// (b.spivey, February 22, 2016) - PLID 68241 - added result comments. 
				case ldlcResultComment:
					ExecuteParamSql("UPDATE LabDiagnosisT SET AutoFillComment = {STRING} WHERE ID = {INT}"
						, strResultComment, nID);
					break; 
			}
		}
		else
		{
			if(nID == -1) {
				// (z.manning 2008-10-27 11:47) - They didn't commit changes and this is a new row, so
				// get rid of it.
				m_pdlLabDiagList->RemoveRow(pRow);
			}
		}

	}NxCatchAll("CLabEditDiagnosisDlg::EditingFinishedLabDiagnosisList");
}

void CLabEditDiagnosisDlg::SelChangedLabDiagnosisList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try
	{
		BOOL bEnable;
		if(lpNewSel == NULL) {
			bEnable = FALSE;
		}
		else {
			bEnable = TRUE;
		}

		// (z.manning 2008-10-27 11:37) - Only enable edit or delete buttons if we have a selection
		m_btnEdit.EnableWindow(bEnable);
		m_btnDelete.EnableWindow(bEnable);

	}NxCatchAll("CLabEditDiagnosisDlg::SelChangedLabDiagnosisList");
}

void CLabEditDiagnosisDlg::RequeryFinishedLabDiagnosisList(short nFlags)
{
	try
	{
		// (r.gonet 03/03/2014) - PLID 60776 - Remove the embedded dropdown of diagnosis codes for the free text options in data list.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlLabDiagList->GetFirstRow();
		while(pRow != NULL) {
			if(pRow->GetValue(ldlcDiag).vt == VT_NULL) {
				NXDATALIST2Lib::IFormatSettingsPtr pFormatOverride(__uuidof(NXDATALIST2Lib::FormatSettings));
				pFormatOverride->DataType = VT_I4;
				pFormatOverride->Editable = VARIANT_FALSE;
				pFormatOverride->FieldType = cftTextSingleLine;
				pFormatOverride->ComboSource = _bstr_t("");
				pRow->CellFormatOverride[ldlcDiag] = pFormatOverride;
			}
			
			pRow = pRow->GetNextRow();
		}

		// (z.manning 2008-10-27 15:03) - We have to manually sort because the field we want to
		// sort on is ntext in data.
		m_pdlLabDiagList->GetColumn(ldlcDescription)->PutSortPriority(0);
		m_pdlLabDiagList->Sort();

	}NxCatchAll("CLabEditDiagnosisDlg::RequeryFinishedLabDiagnosisList");
}

// (j.gruber 2016-02-08 14:43) - PLID 68155 - Create a setup dialog to link Diagnosis Codes with Microscopic descriptions
void CLabEditDiagnosisDlg::OnBnClickedLinkDiagnosis()
{
	try
	{
		CLabEditDiagnosisLinkDlg dlg;
		dlg.DoModal();
		
	}NxCatchAll(__FUNCTION__)
}
