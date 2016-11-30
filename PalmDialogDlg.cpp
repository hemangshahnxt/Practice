// PalmDialogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PalmDialogDlg.h"
#include "globalutils.h"
#include "globaldatautils.h"
#include "practicerc.h"
#include "RegUtils.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About


/////////////////////////////////////////////////////////////////////////////
// CPalmDialogDlg dialog

CPalmDialogDlg::CPalmDialogDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPalmDialogDlg::IDD, pParent)
	, m_AddressDlg(pParent)
	, m_PurposesDlg(pParent)
{
	//{{AFX_DATA_INIT(CPalmDialogDlg)
	m_rs = NULL;
	m_nPUserID = 0;
	m_bAuditSyncs = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
}

void CPalmDialogDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPalmDialogDlg)
	DDX_Control(pDX, IDC_PALM_TAB, m_Tab);
	DDX_Check(pDX, IDC_CHECK_AUDITSYNCHRONIZATIONS, m_bAuditSyncs);
	DDX_Control(pDX, IDC_EDIT_DATABASENAME, m_nxeditEditDatabasename);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CHECK_AUDITSYNCHRONIZATIONS, m_btnAuditSync);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPalmDialogDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPalmDialogDlg)
	ON_WM_SYSCOMMAND()
	ON_NOTIFY(TCN_SELCHANGE, IDC_PALM_TAB, OnSelchangeTab)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPalmDialogDlg message handlers

IMPLEMENT_DYNAMIC(CPalmDialogDlg, CNxDialog)

BOOL CPalmDialogDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog

	// init the tabs
	m_Tab.InsertItem(0, "Appointment Types");
	m_Tab.InsertItem(1, "Address Book");
	
	// init the dialog boxes
	m_PurposesDlg.SetUserID(GetUserID());
	m_AddressDlg.SetUserID(GetUserID());

	m_AddressDlg.Create(IDD_ADDRESS, &m_Tab);
	m_PurposesDlg.Create(IDD_PURPOSESET, &m_Tab);
	
	// functions
	m_PurposesDlg.PUpdate();
	m_AddressDlg.ItemChange();

	m_PurposesDlg.SetWindowPos(&wndBottom, 8, 22, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	m_AddressDlg.SetWindowPos(&wndTop, 8, 22, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE);

	m_bAuditSyncs = GetRemotePropertyInt("PalmAudit", TRUE);

	try {
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		CString strDatabase = NxRegUtils::ReadString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\PalmDatabase");
		if (strDatabase.IsEmpty()) strDatabase = "pracdata";
		SetDlgItemText(IDC_EDIT_DATABASENAME, strDatabase);

		m_dlUsers = BindNxDataListCtrl(this, IDC_USERNAME_COMBO, GetRemoteData(),TRUE);
		_RecordsetPtr prs = CreateRecordset("SELECT UserID, DatabaseName FROM PalmSettingsT WHERE ID = %d", m_nPUserID);
		if (!prs->eof)
		{
			m_dlUsers->TrySetSelByColumn(0, prs->Fields->Item["UserID"]->Value);			
		}
	}
	NxCatchAll("Error opening advanced palm configuration");

	UpdateData(FALSE);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPalmDialogDlg::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	long nSel = m_Tab.GetCurSel();

	switch (nSel) {
	case 0:
		m_AddressDlg.SetWindowPos(&wndTop, 8, 22, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE);
		m_PurposesDlg.SetWindowPos(&wndBottom, 8, 22, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		break;
	case 1:	
		m_AddressDlg.SetWindowPos(&wndTop, 8, 22, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
		m_PurposesDlg.SetWindowPos(&wndBottom, 8, 22, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
		break;
	default:
		break;
	}

	*pResult = 0;
}

void CPalmDialogDlg::OnOK() 
{
	try {
		CString strSQL;
		CString strDatabaseName;

		// Prevent the save if no user is selected
		if (m_dlUsers->CurSel < 0)
		{
			MsgBox("Please select a user before saving the configuration.");
			return;
		}
		GetDlgItemText(IDC_EDIT_DATABASENAME, strDatabaseName);
		if (strDatabaseName.IsEmpty())
		{
			MsgBox("Please enter a database name before saving the configuration.");
			return;
		}

		m_PurposesDlg.OK();
		m_AddressDlg.OK();

		UpdateData(TRUE);
		SetRemotePropertyInt("PalmAudit", m_bAuditSyncs);		
		ExecuteSql("UPDATE PalmSettingsT SET UserID = %d WHERE ID = %d",
			VarLong(m_dlUsers->GetValue(m_dlUsers->CurSel, 0)), GetUserID());
		
		NxRegUtils::WriteString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\PalmDatabase", strDatabaseName);

		CDialog::OnOK();
	}
	NxCatchAll("Error saving configuration");
}

long CPalmDialogDlg::GetUserID()
{
	return m_nPUserID;
}

void CPalmDialogDlg::SetUserID(long ID)
{
	m_nPUserID = ID;
}

void CPalmDialogDlg::OK()
{
	OnOK();
}

void CPalmDialogDlg::Cancel()
{
	OnCancel();
}
