// BlockTimeWarningDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorrc.h"
#include "BlockTimeWarningDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBlockTimeWarningDlg dialog


CBlockTimeWarningDlg::CBlockTimeWarningDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CBlockTimeWarningDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBlockTimeWarningDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CBlockTimeWarningDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBlockTimeWarningDlg)
	DDX_Control(pDX, IDC_BLOCK_WARNING, m_nxstaticBlockWarning);
	DDX_Control(pDX, IDOK, m_btnOk);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBlockTimeWarningDlg, CNxDialog)
	//{{AFX_MSG_MAP(CBlockTimeWarningDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBlockTimeWarningDlg message handlers

BOOL CBlockTimeWarningDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	// (z.manning, 04/30/2008) - PLID 29850 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	
	CString strCaption;
	strCaption.Format("By definition, appointments with a category of \"Block Time\" cannot be associated with patients.\n\n"
		"The appointment type \"%s\" has %li appointments assigned to it which have associated patients.  Therefore, this type "
		"cannot be set to a category of \"Block Time.\"", m_strTypeName, m_nAffectedCount);
	SetDlgItemText(IDC_BLOCK_WARNING, strCaption);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
