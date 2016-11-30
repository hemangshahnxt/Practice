// SupportThirdPartyInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsrc.h"
#include "SupportThirdPartyInfoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSupportThirdPartyInfoDlg dialog


CSupportThirdPartyInfoDlg::CSupportThirdPartyInfoDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSupportThirdPartyInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSupportThirdPartyInfoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSupportThirdPartyInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSupportThirdPartyInfoDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_MIRROR_PATH, m_nxeditMirrorPath);
	DDX_Control(pDX, IDC_INFORM_PATH, m_nxeditInformPath);
	DDX_Control(pDX, IDC_UNITED_PATH, m_nxeditUnitedPath);
	DDX_Control(pDX, IDC_PALM_PATH, m_nxeditPalmPath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSupportThirdPartyInfoDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSupportThirdPartyInfoDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSupportThirdPartyInfoDlg message handlers
using namespace ADODB;
BOOL CSupportThirdPartyInfoDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	((CNxColor*)GetDlgItem(IDC_SUPPORT_LINK_COLOR))->SetColor(m_Color);

	try {
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		_RecordsetPtr rsPatient = CreateRecordset("SELECT MirrorPath, InformPath, UnitedPath, PalmPath FROM NxClientsT "
			"WHERE PersonID = %li", m_nPatientID);
		if(rsPatient->eof) {
			MsgBox("Invalid Patient ID specified!");
			CDialog::OnOK();
		}
		else {
			SetDlgItemText(IDC_MIRROR_PATH, AdoFldString(rsPatient, "MirrorPath", ""));
			SetDlgItemText(IDC_INFORM_PATH, AdoFldString(rsPatient, "InformPath", ""));
			SetDlgItemText(IDC_UNITED_PATH, AdoFldString(rsPatient, "UnitedPath", ""));
			SetDlgItemText(IDC_PALM_PATH, AdoFldString(rsPatient, "PalmPath", ""));
		}
	}NxCatchAll("Error in CSupportThirdPartyInfoDlg::OnInitDialog()");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSupportThirdPartyInfoDlg::OnOK() 
{
	try {
		CString strMirror, strInform, strUnited, strPalm;
		GetDlgItemText(IDC_MIRROR_PATH, strMirror);
		GetDlgItemText(IDC_INFORM_PATH, strInform);
		GetDlgItemText(IDC_UNITED_PATH, strUnited);
		GetDlgItemText(IDC_PALM_PATH, strPalm);
		ExecuteSql("UPDATE NxClientsT SET MirrorPath = '%s', InformPath = '%s', UnitedPath = '%s', PalmPath = '%s' "
			"WHERE PersonID = %li", _Q(strMirror), _Q(strInform), _Q(strUnited), _Q(strPalm), m_nPatientID);
	}NxCatchAll("Error in CSupportThirdPartyInfoDlg::OnOK()");
	
	CDialog::OnOK();
}
