// ConnectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConnectionDlg.h"
#include "RegUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NxRegUtils;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CConnectionDlg dialog


CConnectionDlg::CConnectionDlg(CWnd* pParent)
	: CNxDialog(CConnectionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConnectionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CConnectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConnectionDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_SQLSERVERNAME, m_nxeditSqlservername);
	DDX_Control(pDX, IDC_SQLSERVERIP, m_nxeditSqlserverip);
	DDX_Control(pDX, IDC_NXSERVERIP, m_nxeditNxserverip);
	DDX_Control(pDX, IDC_SHAREDPATH, m_nxeditSharedpath);
	DDX_Control(pDX, IDC_MESSAGE, m_nxstaticMessage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConnectionDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConnectionDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnectionDlg message handlers

BOOL CConnectionDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	SetDlgItemText(IDC_MESSAGE, m_message);

	//do not use Bind function - we may not have a connection, and we don't need one
	CWnd *pWnd = GetDlgItem(IDC_CONNECTIONTYPE_LIST);
	if (!pWnd)
	{	CDialog::OnCancel();
		return TRUE;
	}
	m_conTypeList = pWnd->GetControlUnknown();
	IRowSettingsPtr pRow = m_conTypeList->GetRow(-1);
	pRow->Value[0] = "LAN";
	m_conTypeList->AddRow(pRow);
	pRow = m_conTypeList->GetRow(-1);
	pRow->Value[0] = "IP";
	m_conTypeList->AddRow(pRow);
	
	CString str = LoadNxReg("ConnectionType");
	m_conTypeList->SetSelByColumn(0, _bstr_t(str));

	SetDlgItemText(IDC_SQLSERVERNAME,	LoadNxReg("SqlServerName"));
	SetDlgItemText(IDC_SQLSERVERIP,		LoadNxReg("SqlServerIP"));
	SetDlgItemText(IDC_NXSERVERIP,		LoadNxReg("NxServerIP"));
	SetDlgItemText(IDC_SHAREDPATH,		LoadNxReg("SharedPath"));

	return TRUE;
}

void CConnectionDlg::OnOK() 
{
	long error = 0;
	error += SaveNxReg(IDC_SQLSERVERNAME,	"SqlServerName",	m_sqlServerName);
	error += SaveNxReg(IDC_SQLSERVERIP,		"SqlServerIP",		m_sqlServerIP);
	error += SaveNxReg(IDC_NXSERVERIP,		"NxServerIP",		m_nxServerIP);
	error += SaveNxReg(IDC_SHAREDPATH,		"SharedPath",		m_sharedPath);

	long curSel = m_conTypeList->CurSel;

	if (curSel != -1)
	{	m_connectionType = (LPCSTR)bstr_t(m_conTypeList->Value[curSel][0]);
		if (!WriteString(GetRegistryBase() + "ConnectionType", m_connectionType))
			error++;
	}
	else error++;

	//not an exception, they may simply have no permission
	if (error)
	{
		AfxMessageBox("Could not save settings, but Practice will continue loading.\n\n"
			"To correct this problem, try logging in to Windows as an Administrator.\n"
			"Then log in to Practice.");
	}

	CDialog::OnOK();
}
/////////////////////////////////////////////////////////////////////////////
//protected members
int CConnectionDlg::SaveNxReg(long nID, CString subkey, CString &value)
{
	GetDlgItemText(nID, value);
	if (WriteString(GetRegistryBase() + subkey, value))
		return 0;
	else return 1;
}

CString CConnectionDlg::LoadNxReg(CString subkey)
{
	return ReadString(GetRegistryBase() + subkey);
}

void CConnectionDlg::OnCancel() 
{
	CDialog::OnCancel();
}
