// EditTextDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "EditTextDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditTextDlg dialog
// (a.walling 2008-07-18 12:32) - PLID 30720 - Reusable dialog to edit text


CEditTextDlg::CEditTextDlg(CWnd* pParent, const CString& strText, const CString& strTitle, COLORREF color)
	: CNxDialog(CEditTextDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditTextDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_strText = strText;
	m_strTitle = strTitle;
	m_color = color;
}


void CEditTextDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditTextDlg)
	DDX_Control(pDX, IDOK, m_nxibOK);
	DDX_Control(pDX, IDCANCEL, m_nxibCancel);
	DDX_Control(pDX, IDC_EDIT_TEXT, m_text);
	DDX_Control(pDX, IDC_NXCOLORCTRL1, m_nxcolor);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditTextDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditTextDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditTextDlg message handlers

BOOL CEditTextDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		CenterWindow(GetParent());

		m_text.SetLimitText(0);
		
		if (m_color != 0) {
			m_nxcolor.SetColor(m_color);
		} else {
			// default to patient color
			m_nxcolor.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		}

		// (a.walling 2008-08-14 10:59) - PLID 30720 - Did not set styles for OK/Cancel!!
		m_nxibOK.AutoSet(NXB_OK);
		m_nxibCancel.AutoSet(NXB_CANCEL);

		SetWindowText(m_strTitle);
		m_text.SetWindowText(m_strText);

		m_strText.Empty();
	} NxCatchAll("CEditTextDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditTextDlg::OnOK() 
{
	try {
		m_text.GetWindowText(m_strText);
	} NxCatchAll("CEditTextDlg::OnOK");
	
	CNxDialog::OnOK();
}

void CEditTextDlg::OnCancel() 
{	
	CNxDialog::OnCancel();
}

CString CEditTextDlg::GetText()
{
	return m_strText;
}