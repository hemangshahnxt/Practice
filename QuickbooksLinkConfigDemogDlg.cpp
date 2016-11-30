// QuickbooksLinkConfigDemogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "QuickbooksLinkConfigDemogDlg.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQuickbooksLinkConfigDemogDlg dialog


CQuickbooksLinkConfigDemogDlg::CQuickbooksLinkConfigDemogDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CQuickbooksLinkConfigDemogDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQuickbooksLinkConfigDemogDlg)
	//}}AFX_DATA_INIT
}


void CQuickbooksLinkConfigDemogDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQuickbooksLinkConfigDemogDlg)
	DDX_Control(pDX, IDC_CHECK_CHARGE, m_checkCharge);
	DDX_Control(pDX, IDC_CHECK_CHECK, m_checkCheck);
	DDX_Control(pDX, IDC_CHECK_CASH, m_checkCash);
	DDX_Control(pDX, IDC_CHECK_PAT_ID, m_checkPatID);
	DDX_Control(pDX, IDC_CHECK_PAT_NAME, m_checkPatient);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQuickbooksLinkConfigDemogDlg, CNxDialog)
	//{{AFX_MSG_MAP(CQuickbooksLinkConfigDemogDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQuickbooksLinkConfigDemogDlg message handlers

BOOL CQuickbooksLinkConfigDemogDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_brush.CreateSolidBrush(PaletteColor(0x009ED6BA));

	m_checkPatient.SetCheck(GetRemotePropertyInt("QuickBooksDescAppendPatientName",0,0,"<None>",TRUE) == 1);
	m_checkPatID.SetCheck(GetRemotePropertyInt("QuickBooksDescAppendPatientID",0,0,"<None>",TRUE) == 1);
	
	m_checkCash.SetCheck(GetRemotePropertyInt("QuickBooksDescAppendOnCash",0,0,"<None>",TRUE) == 1);
	m_checkCheck.SetCheck(GetRemotePropertyInt("QuickBooksDescAppendOnCheck",0,0,"<None>",TRUE) == 1);
	m_checkCharge.SetCheck(GetRemotePropertyInt("QuickBooksDescAppendOnCredit",0,0,"<None>",TRUE) == 1);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CQuickbooksLinkConfigDemogDlg::OnOK() 
{
	if((m_checkPatient.GetCheck() || m_checkPatID.GetCheck()) &&
		!m_checkCash.GetCheck() && !m_checkCheck.GetCheck() && !m_checkCharge.GetCheck()) {
		if(IDNO == MessageBox("You have selected patient information to be sent, but no payment types to send it on.\n"
			"If you continue, no patient information will be sent when you export to QuickBooks.\n\n"
			"Are you sure you wish to continue?","Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}
	}

	if(!m_checkPatient.GetCheck() && !m_checkPatID.GetCheck() &&
		(m_checkCash.GetCheck() || m_checkCheck.GetCheck() || m_checkCharge.GetCheck())) {
		if(IDNO == MessageBox("You have not selected any patient information to be sent, but you have selected a payment type to send it on.\n"
			"If you continue, no patient information will be sent when you export to QuickBooks.\n\n"
			"Are you sure you wish to continue?","Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}
	}

	SetRemotePropertyInt("QuickBooksDescAppendPatientName",m_checkPatient.GetCheck() ? 1 : 0,0,"<None>");
	SetRemotePropertyInt("QuickBooksDescAppendPatientID",m_checkPatID.GetCheck() ? 1 : 0,0,"<None>");
	
	SetRemotePropertyInt("QuickBooksDescAppendOnCash",m_checkCash.GetCheck() ? 1 : 0,0,"<None>");
	SetRemotePropertyInt("QuickBooksDescAppendOnCheck",m_checkCheck.GetCheck() ? 1 : 0,0,"<None>");
	SetRemotePropertyInt("QuickBooksDescAppendOnCredit",m_checkCharge.GetCheck() ? 1 : 0,0,"<None>");
	
	CDialog::OnOK();
}
