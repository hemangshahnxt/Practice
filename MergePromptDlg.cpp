// MergePromptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsrc.h"
#include "MergePromptDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMergePromptDlg dialog


CMergePromptDlg::CMergePromptDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMergePromptDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMergePromptDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMergePromptDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMergePromptDlg)
	DDX_Control(pDX, IDC_TO_PRINTER, m_btnMergeDirect);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_MERGE_PROMPT, m_nxstaticMergePrompt);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMergePromptDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMergePromptDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMergePromptDlg message handlers

void CMergePromptDlg::OnOK() 
{
	m_bDirectToPrinter = IsDlgButtonChecked(IDC_TO_PRINTER) ? true : false;

	CDialog::OnOK();
}

BOOL CMergePromptDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		// (c.haag 2008-05-01 12:18) - PLID 29866 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		CString strCaption;
		strCaption.Format("Would you like to merge this %s to Word?", m_strDocumentType);
		SetDlgItemText(IDC_MERGE_PROMPT, strCaption);
	}
	NxCatchAll("Error in CMergePromptDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
