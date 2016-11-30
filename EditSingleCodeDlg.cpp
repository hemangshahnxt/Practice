// EditSingleCodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EditSingleCodeDlg.h"
#include "AdministratorRc.h"

// (a.walling 2013-10-21 10:19) - PLID 59113 - Dialog to manually add / edit a SNOMED / UTS code.

// CEditSingleCodeDlg dialog

IMPLEMENT_DYNAMIC(CEditSingleCodeDlg, CNxDialog)

CEditSingleCodeDlg::CEditSingleCodeDlg(CWnd* pParent)
	: CNxDialog(CEditSingleCodeDlg::IDD, pParent)
	, m_nCodeID(-1)
{

}

CEditSingleCodeDlg::~CEditSingleCodeDlg()
{
}

void CEditSingleCodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CEditSingleCodeDlg, CNxDialog)
END_MESSAGE_MAP()


// CEditSingleCodeDlg message handlers

namespace {
	void UpdateEditField(CWnd* pDlg, UINT nID, ADODB::_RecordsetPtr prs, const char* szField)
	{
		CEdit* pWnd = (CEdit*)pDlg->GetDlgItem(nID);
		if (!pWnd) {
			ASSERT(FALSE);
			return;
		}
		ADODB::FieldPtr pField = prs->Fields->Item[szField];
		if (!pField) {
			ASSERT(FALSE);
			return;
		}

		_ASSERTE((long)pWnd->GetLimitText() == pField->DefinedSize);

		pWnd->SetWindowText(AsString(pField->Value));
	}
}


BOOL CEditSingleCodeDlg::OnInitDialog()
{
	__super::OnInitDialog();

	try {	
		AutoSet(IDOK, NXB_OK);
		AutoSet(IDCANCEL, NXB_CANCEL);

		((CEdit*)GetDlgItem(IDC_EDIT_CODE))->SetLimitText(50);
		((CEdit*)GetDlgItem(IDC_EDIT_NAME))->SetLimitText(255);
		((CEdit*)GetDlgItem(IDC_NOTE))->SetLimitText(1024);
		((CEdit*)GetDlgItem(IDC_EDIT_VOCAB))->SetLimitText(24);

		if (-1 == m_nCodeID) {
			// (a.walling 2014-10-16 16:49) - PLID 62911 - SNOMEDCT + SCTUSX == SNOMEDCT_US; use the new name
			SetDlgItemText(IDC_EDIT_VOCAB, "SNOMEDCT_US");
		} else {
			GetDlgItem(IDC_EDIT_CODE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_VOCAB)->EnableWindow(FALSE);

			ADODB::_RecordsetPtr prs = CreateParamRecordset(
				"SELECT Vocab, Code, Name, Description FROM CodesT WHERE ID = {INT}"
				, m_nCodeID
			);
			// we want to throw if prs is eof
			
			UpdateEditField(this, IDC_EDIT_CODE, prs, "Code");
			UpdateEditField(this, IDC_EDIT_NAME, prs, "Name");
			UpdateEditField(this, IDC_NOTE, prs, "Description");
			UpdateEditField(this, IDC_EDIT_VOCAB, prs, "Vocab");
		}

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CEditSingleCodeDlg::OnOK()
{
	try {
		CString code;
		CString name;
		CString description;
		CString vocab;

		GetDlgItemText(IDC_EDIT_CODE, code);
		GetDlgItemText(IDC_EDIT_NAME, name);
		GetDlgItemText(IDC_NOTE, description);
		GetDlgItemText(IDC_EDIT_VOCAB, vocab);

		code.Trim();
		name.Trim();
		description.Trim();
		vocab.Trim();

		if (code.IsEmpty() || name.IsEmpty()) {
			MessageBox("The code and name must not be blank.");
			return;
		}

		// (a.walling 2013-10-18 10:51) - PLID 59096 - Codes are only unique within their vocabulary
		if(ReturnsRecordsParam("SELECT * FROM CodesT where Code = {STRING} AND Vocab = {STRING} AND ID <> {INT}", code, vocab, m_nCodeID)) {					
			MessageBox("This code already exists and cannot be duplicated.");
			return;
		}

		if (-1 == m_nCodeID) {
			ADODB::_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON;\r\n"
				"INSERT INTO CodesT(Code, Name, Description, Vocab) "
					"VALUES({STR}, {STR}, {STR}, {STR});\r\n"
				"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS ID;\r\n"
				"SET NOCOUNT OFF;\r\n"
				, code
				, name
				, description
				, vocab
			);

			m_nCodeID = AdoFldLong(prs, "ID");
		} else {
			ExecuteParamSql(				
				"UPDATE CodesT SET Name = {STR}, Description = {STR} "
				"WHERE ID = {INT} "
				, name
				, description
				, m_nCodeID
			);
		}
		
		__super::OnOK();
	} NxCatchAll(__FUNCTION__);
}
