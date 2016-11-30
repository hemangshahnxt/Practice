// ReferralTreeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "ReferralTreeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReferralTreeDlg dialog


CReferralTreeDlg::CReferralTreeDlg(CWnd* pParent)
	: CNxDialog(CReferralTreeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReferralTreeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pReferralSubDlg = NULL;
}


void CReferralTreeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReferralTreeDlg)
	DDX_Control(pDX, IDC_REFERRAL_SPACE, m_btnReferralSpace);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReferralTreeDlg, CNxDialog)
	//{{AFX_MSG_MAP(CReferralTreeDlg)
	ON_BN_CLICKED(IDC_HELP_BTN, OnHelp)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReferralTreeDlg message handlers

void CReferralTreeDlg::OnDestroy() 
{
	CNxDialog::OnDestroy();

	try {
		if(m_pReferralSubDlg) {
			m_pReferralSubDlg->DestroyWindow();
			delete m_pReferralSubDlg;
			m_pReferralSubDlg = NULL;
		}
	} NxCatchAll("Error in OnDestroy()");
}
// (a.wilson 2012-5-9) PLID 14874 - change wording to include inactive selection.
void CReferralTreeDlg::OnOK() 
{
	try {
		if(m_pReferralSubDlg == NULL)
			return;

		long nID = m_pReferralSubDlg->GetSelectedReferralID();
		if(nID <= 0) {
			//no selection
			AfxMessageBox("You must select an active referral source before pressing OK.");
			return;
		}
		// (r.goldschmidt 2014-08-29 12:18) - PLID 31191 - Check if selection is restricted.
		CString strWarning;
		long nPreferenceViolation = m_pReferralSubDlg->ReferralRestrictedByPreference(nID, strWarning);
		if (nPreferenceViolation != 0){
			AfxMessageBox(strWarning);
			return;
		}
		else {
			// nPreferenceViolation == 0, which means there is no violation
		}

		EndDialog(nID);
	} NxCatchAll("Error in OnOK()");
}

BOOL CReferralTreeDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//Load the new referral subdialog and position it
		m_pReferralSubDlg = new CReferralSubDlg(this);
		m_pReferralSubDlg->Create(IDD_REFERRAL_SUBDIALOG, this);

		CRect rc;
		GetDlgItem(IDC_REFERRAL_SPACE)->GetWindowRect(&rc);
		ScreenToClient(&rc);
		m_pReferralSubDlg->MoveWindow(rc);
	} NxCatchAll("Error in OnInitDialog()");

	return TRUE;
}

void CReferralTreeDlg::OnCancel() 
{
	EndDialog(-2);	
}

void CReferralTreeDlg::OnHelp(){

	try {
		//(j.anspach 06-09-2005 10:53 PLID 16662) - Updating the OpenManual call to work with the new help system.
		OpenManual("NexTech_Practice_Manual.chm", "Patient_Information/Referral_Source/add_a_referral_source.htm");
	} NxCatchAll("Error in OnHelp()");
}

LRESULT CReferralTreeDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case NXM_REFERRAL_ONCANCEL:
		//The user pressed escape on the subdialog.  Treat this as cancel
		OnCancel();
		return 0;
		break;
	case NXM_REFERRAL_ONOK:
		//The user pressed enter on the subdialog.  Treat this as OK
		OnOK();
		return 0;
		break;
	case NXM_REFERRAL_DOUBLECLICK:
		//The user double clicked on a referral source in the tree.  Treat this as them clicking it then OK.
		OnOK();
		return 0;
		break;


	}

	return CWnd::WindowProc(message, wParam, lParam);
}
