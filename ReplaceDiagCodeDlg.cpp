// ReplaceDiagCodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReplaceDiagCodeDlg.h"
#include "DiagSearchUtils.h"
#include "DiagCodeInfo.h"

// (j.jones 2014-12-22 14:06) - PLID 64489 - created

// CReplaceDiagCodeDlg dialog

using namespace NXDATALIST2Lib;

enum DiagCodeListColumns
{
	dclcICD9ID = 0,
	dclcICD10ID,
	dclcICD9Code,
	dclcICD9Desc,
	dclcICD10Code,
	dclcICD10Desc,
};

IMPLEMENT_DYNAMIC(CReplaceDiagCodeDlg, CNxDialog)

CReplaceDiagCodeDlg::CReplaceDiagCodeDlg(CWnd* pParent /*=NULL*/)
: CNxDialog(CReplaceDiagCodeDlg::IDD, pParent)
{
	//default to the bill color
	m_nBkgColor = (long)0xFFC0C0;

	//initialize these
	m_oldDiagCode = make_shared<DiagCodeInfo>();
	m_newDiagCode = make_shared<DiagCodeInfo>();
}

CReplaceDiagCodeDlg::~CReplaceDiagCodeDlg()
{
}

void CReplaceDiagCodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_REPLACE_DIAG_CODE_BKG, m_bkg);
}

BEGIN_MESSAGE_MAP(CReplaceDiagCodeDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOK)
END_MESSAGE_MAP()

// CReplaceDiagCodeDlg message handlers

int CReplaceDiagCodeDlg::DoModal(long nBkgColor, DiagCodeInfoPtr oldDiagCode)
{
	m_nBkgColor = nBkgColor;
	m_oldDiagCode = oldDiagCode;
	
	return CNxDialog::DoModal();
}

BOOL CReplaceDiagCodeDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_bkg.SetColor(m_nBkgColor);

		m_DiagSearch = DiagSearchUtils::BindDiagPreferenceSearchListCtrl(this, IDC_REPLACEMENT_DIAG_SEARCH, GetRemoteData());
		m_OldCodeList = BindNxDataList2Ctrl(IDC_OLD_CODE, false);
		m_NewCodeList = BindNxDataList2Ctrl(IDC_NEW_CODE, false);

		IRowSettingsPtr pRow = m_OldCodeList->GetNewRow();
		pRow->PutValue(dclcICD9ID, m_oldDiagCode->nDiagCode9ID);
		pRow->PutValue(dclcICD10ID, m_oldDiagCode->nDiagCode10ID);
		pRow->PutValue(dclcICD9Code, _bstr_t(m_oldDiagCode->strDiagCode9Code.IsEmpty() ? "<None>" : m_oldDiagCode->strDiagCode9Code));
		pRow->PutValue(dclcICD9Desc, _bstr_t(m_oldDiagCode->strDiagCode9Desc));
		pRow->PutValue(dclcICD10Code, _bstr_t(m_oldDiagCode->strDiagCode10Code.IsEmpty() ? "<None>" : m_oldDiagCode->strDiagCode10Code));
		pRow->PutValue(dclcICD10Desc, _bstr_t(m_oldDiagCode->strDiagCode10Desc));
		m_OldCodeList->AddRowAtEnd(pRow, NULL);
		
		//this intentionally adds a gray color to the old list,
		//while the new list keeps a white color
		m_OldCodeList->PutReadOnly(VARIANT_TRUE);
		
		DiagSearchUtils::SizeDiagnosisListColumnsBySearchPreference(m_OldCodeList, dclcICD9Code, dclcICD10Code,
			65, 65, "<None>", "<None>", dclcICD9Desc, dclcICD10Desc, true, false, false);
		DiagSearchUtils::SizeDiagnosisListColumnsBySearchPreference(m_NewCodeList, dclcICD9Code, dclcICD10Code,
			65, 65, "<None>", "<None>", dclcICD9Desc, dclcICD10Desc, true, false, false);

	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CReplaceDiagCodeDlg, CNxDialog)
	ON_EVENT(CReplaceDiagCodeDlg, IDC_REPLACEMENT_DIAG_SEARCH, 16, CReplaceDiagCodeDlg::OnSelChosenDiagSearch, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CReplaceDiagCodeDlg::OnSelChosenDiagSearch(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}

		CDiagSearchResults results = DiagSearchUtils::ConvertPreferenceSearchResults(lpRow);
		if (results.m_ICD9.m_nDiagCodesID == -1 && results.m_ICD10.m_nDiagCodesID == -1) {
			return;
		}

		//remove any existing value
		m_NewCodeList->Clear();

		IRowSettingsPtr pNewRow = m_NewCodeList->GetNewRow();
		pNewRow->PutValue(dclcICD9ID, results.m_ICD9.m_nDiagCodesID);
		pNewRow->PutValue(dclcICD10ID, results.m_ICD10.m_nDiagCodesID);
		pNewRow->PutValue(dclcICD9Code, _bstr_t(results.m_ICD9.m_strCode.IsEmpty() ? "<None>" : results.m_ICD9.m_strCode));
		pNewRow->PutValue(dclcICD9Desc, _bstr_t(results.m_ICD9.m_strDescription));
		pNewRow->PutValue(dclcICD10Code, _bstr_t(results.m_ICD10.m_strCode.IsEmpty() ? "<None>" : results.m_ICD10.m_strCode));
		pNewRow->PutValue(dclcICD10Desc, _bstr_t(results.m_ICD10.m_strDescription));
		m_NewCodeList->AddRowAtEnd(pNewRow, NULL);

		DiagSearchUtils::SizeDiagnosisListColumnsBySearchPreference(m_NewCodeList, dclcICD9Code, dclcICD10Code,
			65, 65, "<None>", "<None>", dclcICD9Desc, dclcICD10Desc, true, false, false);

	}NxCatchAll(__FUNCTION__);
}

void CReplaceDiagCodeDlg::OnOK()
{
	try {

		//this should never have more than one row
		ASSERT(m_NewCodeList->GetRowCount() <= 1);

		//make sure they selected a code
		IRowSettingsPtr pRow = m_NewCodeList->GetFirstRow();
		if (pRow == NULL) {
			AfxMessageBox("You must search for a diagnosis code to use as a replacement before clicking OK.");
			return;
		}

		long nNewDiag9CodeID = VarLong(pRow->GetValue(dclcICD9ID), -1);
		long nNewDiag10CodeID = VarLong(pRow->GetValue(dclcICD10ID), -1);

		if (nNewDiag9CodeID == -1 && nNewDiag10CodeID == -1) {
			//how did a blank row get added?
			ASSERT(FALSE);
			AfxMessageBox("You must search for a diagnosis code to use as a replacement before clicking OK.");
			return;
		}

		//make sure they selected a different code than what they started with
		if (nNewDiag9CodeID == m_oldDiagCode->nDiagCode9ID && nNewDiag10CodeID == m_oldDiagCode->nDiagCode10ID) {
			AfxMessageBox("You have selected the same diagnosis code. Please select a different code to use as a replacement, or click Cancel.");
			return;
		}

		m_newDiagCode = make_shared<DiagCodeInfo>();
		m_newDiagCode->nDiagCode9ID = nNewDiag9CodeID;
		if (m_newDiagCode->nDiagCode9ID != -1) {
			m_newDiagCode->strDiagCode9Code = VarString(pRow->GetValue(dclcICD9Code), "");
			m_newDiagCode->strDiagCode9Desc = VarString(pRow->GetValue(dclcICD9Desc), "");
		}
		m_newDiagCode->nDiagCode10ID = nNewDiag10CodeID;
		if (m_newDiagCode->nDiagCode10ID != -1) {
			m_newDiagCode->strDiagCode10Code = VarString(pRow->GetValue(dclcICD10Code), "");
			m_newDiagCode->strDiagCode10Desc = VarString(pRow->GetValue(dclcICD10Desc), "");
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}