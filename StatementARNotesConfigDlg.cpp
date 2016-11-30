// StatementARNotesConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "billingRC.h"
#include "StatementARNotesConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStatementARNotesConfigDlg dialog


CStatementARNotesConfigDlg::CStatementARNotesConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CStatementARNotesConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStatementARNotesConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CStatementARNotesConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStatementARNotesConfigDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_THIRTY_DAY_NOTE, m_nxeditThirtyDayNote);
	DDX_Control(pDX, IDC_SIXTY_DAY_NOTE, m_nxeditSixtyDayNote);
	DDX_Control(pDX, IDC_NINETY_DAY_NOTE, m_nxeditNinetyDayNote);
	DDX_Control(pDX, IDC_NINETY_PLUS_NOTE, m_nxeditNinetyPlusNote);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStatementARNotesConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CStatementARNotesConfigDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStatementARNotesConfigDlg message handlers

void CStatementARNotesConfigDlg::OnOK() 
{
	CString strThirty, strSixty, strNinety, strNinetyPlus;
	GetDlgItemText(IDC_THIRTY_DAY_NOTE, strThirty);
	GetDlgItemText(IDC_SIXTY_DAY_NOTE, strSixty);
	GetDlgItemText(IDC_NINETY_DAY_NOTE, strNinety);
	GetDlgItemText(IDC_NINETY_PLUS_NOTE, strNinetyPlus);


	SetRemotePropertyMemo("Sttmnt30DayNote", strThirty, 0, "<None>");
	SetRemotePropertyMemo("Sttmnt60DayNote", strSixty, 0, "<None>");
	SetRemotePropertyMemo("Sttmnt90DayNote", strNinety, 0, "<None>");
	SetRemotePropertyMemo("Sttmnt90+DayNote", strNinetyPlus, 0, "<None>");

	CDialog::OnOK();
}

BOOL CStatementARNotesConfigDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-02 09:58) - PLID 29879 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		SetDlgItemText(IDC_THIRTY_DAY_NOTE, GetRemotePropertyMemo ("Sttmnt30DayNote", "", 0, "<None>"));
		SetDlgItemText(IDC_SIXTY_DAY_NOTE, GetRemotePropertyMemo ("Sttmnt60DayNote", "", 0, "<None>"));
		SetDlgItemText(IDC_NINETY_DAY_NOTE, GetRemotePropertyMemo ("Sttmnt90DayNote", "", 0, "<None>"));
		SetDlgItemText(IDC_NINETY_PLUS_NOTE, GetRemotePropertyMemo ("Sttmnt90+DayNote", "", 0, "<None>"));
	}
	NxCatchAll("Error in CStatementARNotesConfigDlg::OnInitDialog");
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
