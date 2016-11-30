// ChooseTwoQBAcctsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ChooseTwoQBAcctsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CChooseTwoQBAcctsDlg dialog


CChooseTwoQBAcctsDlg::CChooseTwoQBAcctsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CChooseTwoQBAcctsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseTwoQBAcctsDlg)
		m_strFromAccountLabel = "";
		m_strToAccountLabel = "";
		m_bSettingDefaults = FALSE;
	//}}AFX_DATA_INIT
}


void CChooseTwoQBAcctsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseTwoQBAcctsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_FROM_ACCOUNT_LABEL, m_nxstaticFromAccountLabel);
	DDX_Control(pDX, IDC_TO_ACCOUNT_LABEL, m_nxstaticToAccountLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseTwoQBAcctsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CChooseTwoQBAcctsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseTwoQBAcctsDlg message handlers

BOOL CChooseTwoQBAcctsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2008-05-01 15:41) - PLID 29871 - NxIconified buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		if(m_strFromAccountLabel != "")
			SetDlgItemText(IDC_FROM_ACCOUNT_LABEL,m_strFromAccountLabel);

		if(m_strToAccountLabel != "")
			SetDlgItemText(IDC_TO_ACCOUNT_LABEL,m_strToAccountLabel);
	
		m_dlFromCombo = GetDlgItem(IDC_FROM_ACCOUNT_COMBO)->GetControlUnknown();
		m_dlFromCombo->Clear();
		
		ASSERT(m_strInFromIDs.GetSize() == m_strInFromNames.GetSize());
		if (m_strInFromIDs.GetSize() > 0) {
			for (long i=0; i<m_strInFromIDs.GetSize(); i++) {
				IRowSettingsPtr pRow = m_dlFromCombo->GetRow(-1);
				pRow->PutValue(0, _bstr_t(m_strInFromIDs.GetAt(i)));
				pRow->PutValue(1, _bstr_t(m_strInFromNames.GetAt(i)));
				m_dlFromCombo->AddRow(pRow);
			}
		}

		m_dlToCombo = GetDlgItem(IDC_TO_ACCOUNT_COMBO)->GetControlUnknown();
		m_dlToCombo->Clear();
		
		ASSERT(m_strInToIDs.GetSize() == m_strInToNames.GetSize());
		if (m_strInToIDs.GetSize() > 0) {
			for (long i=0; i<m_strInToIDs.GetSize(); i++) {
				IRowSettingsPtr pRow = m_dlToCombo->GetRow(-1);
				pRow->PutValue(0, _bstr_t(m_strInToIDs.GetAt(i)));
				pRow->PutValue(1, _bstr_t(m_strInToNames.GetAt(i)));
				m_dlToCombo->AddRow(pRow);
			}
		}

		if(m_bSettingDefaults) {
			
			//Add the "<No Default>" rows
			
			IRowSettingsPtr pRow = m_dlFromCombo->GetRow(-1);
			pRow->PutValue(0, _bstr_t("-1"));
			pRow->PutValue(1, _bstr_t("<No Default>"));
			m_dlFromCombo->InsertRow(pRow,0);
			pRow = m_dlToCombo->GetRow(-1);
			pRow->PutValue(0, _bstr_t("-1"));
			pRow->PutValue(1, _bstr_t("<No Default>"));
			m_dlToCombo->InsertRow(pRow,0);
		}

		CString strDefFromAcct, strDefToAcct;
		strDefFromAcct = GetRemotePropertyText("QBDefaultDepositSourceAcct","-1",0,"<None>",TRUE);
		strDefToAcct = GetRemotePropertyText("QBDefaultDepositDestAcct","-1",0,"<None>",TRUE);

		if(strDefFromAcct != "-1" || m_bSettingDefaults)
			m_dlFromCombo->SetSelByColumn(0,_bstr_t(strDefFromAcct));
		if(strDefToAcct != "-1" || m_bSettingDefaults)
			m_dlToCombo->SetSelByColumn(0,_bstr_t(strDefToAcct));

	} NxCatchAll("CChooseTwoQBAcctsDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChooseTwoQBAcctsDlg::OnOK() 
{
	try {
		long nCurSel1 = m_dlFromCombo->GetCurSel();
		if (nCurSel1 != -1) {
			m_strFromOutID = (LPCTSTR)_bstr_t(m_dlFromCombo->GetValue(nCurSel1, 0));
		} else {
			MessageBox("Please make a selection for the source account before you proceed.", NULL, MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		long nCurSel2 = m_dlToCombo->GetCurSel();
		if (nCurSel2 != -1) {
			m_strToOutID = (LPCTSTR)_bstr_t(m_dlToCombo->GetValue(nCurSel2, 0));
		} else {
			MessageBox("Please make a selection for the deposit account before you proceed.", NULL, MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		if(nCurSel1 != -1 && nCurSel2 != -1) {

			//make sure they are not the same accounts
			if(m_dlFromCombo->GetValue(nCurSel1,0) == m_dlToCombo->GetValue(nCurSel2,0)
				&& CString(m_dlFromCombo->GetValue(nCurSel1,0).bstrVal) != "-1") {
				MessageBox("You cannot send funds from and to the same account. Please choose different accounts.", NULL, MB_OK|MB_ICONEXCLAMATION);
				return;
			}

			CDialog::OnOK();
		}

	} NxCatchAll("CChooseTwoQBAcctsDlg::OnOK");
}
