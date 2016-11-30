// VerifyInstructionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "reports.h"
#include "VerifyInstructionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVerifyInstructionsDlg dialog


CVerifyInstructionsDlg::CVerifyInstructionsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CVerifyInstructionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVerifyInstructionsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CVerifyInstructionsDlg::CVerifyInstructionsDlg(CString strFileName, CWnd* pParent /*=NULL*/)
	: CNxDialog(CVerifyInstructionsDlg::IDD, pParent)
{
	strFileName = m_strTtxFileName;
}



void CVerifyInstructionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVerifyInstructionsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_INSTRUCTIONS, m_nxeditInstructions);
	DDX_Control(pDX, IDC_CLOSE_INSTRUCTIONS, m_btnCloseInstructions);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVerifyInstructionsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CVerifyInstructionsDlg)
	ON_BN_CLICKED(IDC_CLOSE_INSTRUCTIONS, OnCloseInstructions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVerifyInstructionsDlg message handlers

void CVerifyInstructionsDlg::OnCloseInstructions() 
{
	CWnd::DestroyWindow();
	
}

BOOL CVerifyInstructionsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/28/2008) - PLID 29807 - Set button styles
	m_btnCloseInstructions.AutoSet(NXB_CLOSE);
	
	//load the text into the dialog
	CString strInstructions;
	strInstructions.Format(" Please do the following: "
		"\r\n1. Put your mouse over the left side of the report, in the white space underneath \"Unbound Fields\""
		"\r\n2. Right click and choose Database and then SetLocation "
		"\r\n3. Click the \"Set Location\" Button "
		"\r\n4. Click on the + next to Current Connections to expand that item "
		"\r\n5. Expand the item again by clicking the + next to %s "
		"\r\n6. Double click on the %s underneath the %s you just expanded "
		"\r\n7. Click the Done button "
		"\r\n8. Click the Verify button again", m_strTtxFileName, m_strTtxFileName, m_strTtxFileName);

	SetDlgItemText(IDC_INSTRUCTIONS, strInstructions);

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
