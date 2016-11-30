// ChooseDateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "ChooseDateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseDateDlg dialog
//
// (c.haag 2006-11-20 09:21) - PLID 23589 - This dialog was originally
// created to let users choose a template line item exception date
//

CChooseDateDlg::CChooseDateDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CChooseDateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseDateDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChooseDateDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseDateDlg)
	DDX_Control(pDX, IDC_DATE, m_dtpDate);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseDateDlg, CNxDialog)
	//{{AFX_MSG_MAP(CChooseDateDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseDateDlg message handlers

BOOL CChooseDateDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	if (COleDateTime::invalid == m_dtDate.GetStatus()) {
		m_dtpDate.SetValue((COleVariant)COleDateTime::GetCurrentTime());
	} else {
		m_dtpDate.SetValue((COleVariant)m_dtDate);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

COleDateTime CChooseDateDlg::Open(const COleDateTime& dtDefaultDate)
{
	m_dtDate = dtDefaultDate;
	if (IDOK == DoModal()) {
		return m_dtDate;
	} else {
		COleDateTime dtInvalid;
		dtInvalid.SetStatus( COleDateTime::invalid );
		return dtInvalid;
	}
}

void CChooseDateDlg::OnOK() 
{
	// (c.haag 2006-12-27 13:25) - PLID 23589 - Ensure that the date combo does
	// not have focus, or else we may lose our last date change before saving
	GetDlgItem(IDOK)->SetFocus();

	// (c.haag 2006-11-20 09:25) - PLID 23589 - Store the selection in m_dtDate
	m_dtDate = VarDateTime(m_dtpDate.GetValue());

	CDialog::OnOK();
}
