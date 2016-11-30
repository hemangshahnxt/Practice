// InactiveModifiersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InactiveModifiersDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


enum EInactiveModiferColumns
{
	imcID = 0,
	imcDescription,
};

/////////////////////////////////////////////////////////////////////////////
// CInactiveModifiersDlg dialog


CInactiveModifiersDlg::CInactiveModifiersDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInactiveModifiersDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInactiveModifiersDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CInactiveModifiersDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInactiveModifiersDlg)
	DDX_Control(pDX, IDC_INACTIVE_TITLE_LABEL, m_nxstaticInactiveTitleLabel);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInactiveModifiersDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInactiveModifiersDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInactiveModifiersDlg message handlers

BOOL CInactiveModifiersDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		// (z.manning, 05/01/2008) - PLID 29860 - Set button styles
		m_btnClose.AutoSet(NXB_CLOSE);
		
		m_pdlList = BindNxDataListCtrl(this, IDC_INACTIVE_MODIFIER_LIST, GetRemoteData(), true);

	}NxCatchAll("CInactiveModifiersDlg::OnInitDialog");
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CInactiveModifiersDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInactiveModifiersDlg)
	ON_EVENT(CInactiveModifiersDlg, IDC_INACTIVE_MODIFIER_LIST, 3 /* DblClickCell */, OnDblClickCellInactiveModifierList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInactiveModifiersDlg::OnDblClickCellInactiveModifierList(long nRowIndex, short nColIndex) 
{
	try 
	{
		if(nRowIndex == -1) {
			return;
		}

		// (z.manning, 05/02/2007) - PLID 16623 - Prompt and then inactivate the currently selected modifier.
		CString strModifierID = VarString(m_pdlList->GetValue(nRowIndex, imcID));
		CString strDescription = VarString(m_pdlList->GetValue(nRowIndex, imcDescription), "");

		CString strMessage = FormatString("You are about to restore the modifier '%s'.\r\n\r\n"
			"This code will become available for use and editing in the Administrator module and in billing.\r\n\r\n"
			"Are you sure you wish to restore this code to active use?", strDescription);

		if(IDYES == MessageBox(strMessage, "Activate Modifer", MB_YESNO|MB_ICONQUESTION)) 
		{
			ExecuteSql("UPDATE CptModifierT SET Active = 1 WHERE Number = '%s'", _Q(strModifierID));

			m_pdlList->RemoveRow(m_pdlList->CurSel);

			CClient::RefreshTable(NetUtils::CPTModifierT);	
		}

	}NxCatchAll("CInactiveModifiersDlg::OnDblClickCellInactiveModifierList");
}
