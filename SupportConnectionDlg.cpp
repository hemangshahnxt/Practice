// SupportConnectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsrc.h"
#include "SupportConnectionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSupportConnectionDlg dialog


CSupportConnectionDlg::CSupportConnectionDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSupportConnectionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSupportConnectionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSupportConnectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSupportConnectionDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PCA, m_nxeditPca);
	DDX_Control(pDX, IDC_PCA_IP, m_nxeditPcaIp);
	DDX_Control(pDX, IDC_LOGIN, m_nxeditLogin);
	DDX_Control(pDX, IDC_PASSWORD, m_nxeditPassword);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSupportConnectionDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSupportConnectionDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSupportConnectionDlg message handlers

void CSupportConnectionDlg::OnOK() 
{
	try {
		CString strPCA, strPCAIP, strLogin, strPassword;
		GetDlgItemText(IDC_PCA, strPCA);
		GetDlgItemText(IDC_PCA_IP, strPCAIP);
		GetDlgItemText(IDC_LOGIN, strLogin);
		GetDlgItemText(IDC_PASSWORD, strPassword);
		ExecuteSql("UPDATE NxClientsT SET PCA = '%s', PCAIP = '%s', Login = '%s', Password = '%s' "
			"WHERE PersonID = %li", _Q(strPCA), _Q(strPCAIP), _Q(strLogin), _Q(strPassword), m_nPatientID);
	}NxCatchAll("Error in CSupportConnectionDlg::OnOK()");
	
	CDialog::OnOK();
}

BOOL CSupportConnectionDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	((CNxColor*)GetDlgItem(IDC_SUPPORT_CONNECTION_COLOR))->SetColor(m_Color);

	try {
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		ADODB::_RecordsetPtr rsPatient = CreateRecordset("SELECT PCA, PCAIP, Login, Password FROM NxClientsT "
			"WHERE PersonID = %li", m_nPatientID);
		if(rsPatient->eof) {
			MsgBox("Invalid Patient ID specified!");
			CDialog::OnOK();
		}
		else {
			SetDlgItemText(IDC_PCA, AdoFldString(rsPatient, "PCA", ""));
			SetDlgItemText(IDC_PCA_IP, AdoFldString(rsPatient, "PCAIP", ""));
			SetDlgItemText(IDC_LOGIN, AdoFldString(rsPatient, "Login", ""));
			SetDlgItemText(IDC_PASSWORD, AdoFldString(rsPatient, "Password", ""));
		}
	}NxCatchAll("Error in CSupportConnectionDlg::OnInitDialog()");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
