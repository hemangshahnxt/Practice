// EMRTextDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRTextDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEMRTextDlg dialog


CEMRTextDlg::CEMRTextDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRTextDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMRTextDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEMRTextDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRTextDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_EDIT_EMR_TEXT, m_nxeditEditEmrText);
	DDX_Control(pDX, IDC_EMR_TEXT_TITLE, m_nxstaticEmrTextTitle);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRTextDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRTextDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRTextDlg message handlers

BOOL CEMRTextDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-28 12:25) - PLID 29806 - NxIconify the Close button
		m_btnOK.AutoSet(NXB_CLOSE);
		
		//if m_text was prefilled, fill in the dialog
		SetDlgItemText(IDC_EDIT_EMR_TEXT,m_text);
	}
	NxCatchAll("Error in CEMRTextDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRTextDlg::OnOK() 
{
	GetDlgItemText(IDC_EDIT_EMR_TEXT,m_text);

	if(m_text.GetLength() > 3500) {
		AfxMessageBox("The text you entered is too long. Each individual text item currently supports 3,500 characters.\n"
			"Please shorten your notes to match this limit, or create a new text item.");
		return;
	}
	
	CDialog::OnOK();
}
