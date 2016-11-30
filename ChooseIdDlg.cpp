// ChooseIdDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "ChooseIdDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CChooseIdDlg dialog


CChooseIdDlg::CChooseIdDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CChooseIdDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseIdDlg)
		m_strDefaultLabel = "";
	//}}AFX_DATA_INIT
}


void CChooseIdDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseIdDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_ACCOUNT_LABEL, m_nxstaticAccountLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseIdDlg, CNxDialog)
	//{{AFX_MSG_MAP(CChooseIdDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseIdDlg message handlers

void CChooseIdDlg::OnOK() 
{
	try {
		long nCurSel = m_dlCombo->GetCurSel();
		if (nCurSel != -1) {
			m_strOutID = (LPCTSTR)_bstr_t(m_dlCombo->GetValue(nCurSel, 0));
			CDialog::OnOK();
		} else {
			MessageBox("Please make a selection before you proceed.", NULL, MB_OK|MB_ICONEXCLAMATION);
		}

	} NxCatchAll("CChooseIdDlg::OnOK");
}

BOOL CChooseIdDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2008-05-01 15:38) - PLID 29871 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		if(m_strDefaultLabel != "")
			SetDlgItemText(IDC_ACCOUNT_LABEL,m_strDefaultLabel);

		m_dlCombo = GetDlgItem(IDC_COMBO)->GetControlUnknown();
		m_dlCombo->Clear();
		
		ASSERT(m_strInIDs.GetSize() == m_strInNames.GetSize());
		if (m_strInIDs.GetSize() > 0) {
			for (long i=0; i<m_strInIDs.GetSize(); i++) {
				IRowSettingsPtr pRow = m_dlCombo->GetRow(-1);
				pRow->PutValue(0, _bstr_t(m_strInIDs.GetAt(i)));
				pRow->PutValue(1, _bstr_t(m_strInNames.GetAt(i)));
				m_dlCombo->AddRow(pRow);
			}
		}

	} NxCatchAll("CChooseIdDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
