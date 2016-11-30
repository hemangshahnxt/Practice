// OverrideUserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "OverrideUserDlg.h"
#include "globalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COverrideUserDlg dialog


//DRT 6/7/2007 - PLID 25892 - This dialog is designed to be a generic "user override" dialog.  It is originally
//	designed for the sales proposal interface, allowing another user (typically a manager) to quickly login
//	"on the spot" and allow a certain action to happen.

COverrideUserDlg::COverrideUserDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(COverrideUserDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COverrideUserDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_strOverrideWhereClause = "";
	m_nApprovedUserID = -1;
}


void COverrideUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COverrideUserDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_OVERRIDE_PASSWORD, m_nxeditOverridePassword);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COverrideUserDlg, CNxDialog)
	//{{AFX_MSG_MAP(COverrideUserDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COverrideUserDlg message handlers

BOOL COverrideUserDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_pUserList = BindNxDataList2Ctrl(this, IDC_OVERRIDE_USER_LIST, GetRemoteData(), false);
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//Allow the user to specify an overriden where clause of their own.
		if(!m_strOverrideWhereClause.IsEmpty()) {
			m_pUserList->WhereClause = _bstr_t(m_strOverrideWhereClause);
		}

		m_pUserList->Requery();

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COverrideUserDlg::OnOK() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pUserList->CurSel;
		if(pRow == NULL) {
			MessageBox("You must select a user before continuing.");
			return;
		}

		long nUserID = VarLong(pRow->GetValue(0));
		CString strUserName = VarString(pRow->GetValue(1), "");
		CString strEnteredPassword;
		GetDlgItemText(IDC_OVERRIDE_PASSWORD, strEnteredPassword);

		//Verify the password.  I made a new function in our global utils to get this.  LoadUserInfo was the closest
		//	thing that we have already, but I do not want this to be tied to a location (a manager may work out of one
		//	location and be perfectly able to approve a discount in another location).
		if(!CompareSpecificUserPassword(nUserID, strEnteredPassword)) {
			// (j.jones 2008-11-19 15:12) - PLID 28578 - passwords are now case-sensitive so we must warn accordingly
			MessageBox("Invalid password entered. Please try again.\n\n"
					"Be sure to use the correct uppercase and lowercase characters in the password.", "Invalid Password", MB_OK|MB_ICONERROR);
			return;
		}

		//We win!
		m_strApprovedUserName = strUserName;
		m_nApprovedUserID = nUserID;
		CDialog::OnOK();

	} NxCatchAll("Error in OnOK");
}

void COverrideUserDlg::OnCancel() 
{
	try {

	} NxCatchAll("Error in OnCancel");

	CDialog::OnCancel();
}
