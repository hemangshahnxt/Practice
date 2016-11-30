// SaveReportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "reports.h"
#include "SaveReportDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSaveReportDlg dialog

using namespace ADODB;
CSaveReportDlg::CSaveReportDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSaveReportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSaveReportDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CSaveReportDlg::CSaveReportDlg(CWnd* pParent, CString strTitle, long nID, bool bIsCustom, bool bIsNew)
	: CNxDialog(CSaveReportDlg::IDD, pParent)
{
	m_strTitle = strTitle;
	m_bIsNew = bIsNew;
	m_nID = nID;
	m_bIsCustom = bIsCustom;
	m_bUpdateTitle = false;
}


void CSaveReportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaveReportDlg)
	DDX_Control(pDX, IDC_REPORTTITLE, m_nxeditReporttitle);
	DDX_Control(pDX, IDC_SAVEREPORT, m_btnSaveReport);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_NOSAVEREPORT, m_btnNoSaveReport);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSaveReportDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSaveReportDlg)
	ON_BN_CLICKED(IDC_SAVEREPORT, OnSavereport)
	ON_BN_CLICKED(IDC_NOSAVEREPORT, OnNosavereport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaveReportDlg message handlers

void CSaveReportDlg::OnSavereport() 
{

	CString strNewTitle;
	GetDlgItemText(IDC_REPORTTITLE, strNewTitle);

	//check to see that this is the only title of this name for this report unless they are just editing, then they can save
	if (m_bIsNew) {
		_RecordsetPtr rs = CreateRecordset("SELECT Title FROM CustomReportsT WHERE ID = %li AND Title = '%s'",  m_nID, _Q(strNewTitle));
		m_strTitle = strNewTitle;
	
		if (! rs->eof) {
			CString str;
			str.Format("This report already has a title of %s, please enter a new title", _Q(m_strTitle));
			MessageBox(str);
			return;
		}

	}
	else {

		if (m_strTitle.CompareNoCase(strNewTitle) != 0) {

			if (MsgBox(MB_YESNO, "Would you like to change the title of this report?", "NexTech") == IDYES) {

				m_strTitle = strNewTitle;
				m_bUpdateTitle = true;
			}
		}
	}
		
	
	CNxDialog::OnOK();

	
}

void CSaveReportDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	CNxDialog::OnCancel();
	

}

BOOL CSaveReportDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	SetDlgItemText(IDC_REPORTTITLE, m_strTitle);

	// (z.manning, 04/28/2008) - PLID 29807 - Set button styles
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSaveReportDlg::OnNosavereport() 
{
	CNxDialog::EndDialog(0);

}
