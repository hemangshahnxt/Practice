// BackupProgessDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BackupProgessDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (z.manning, 05/21/2008) - PLID 30050 - Created
/////////////////////////////////////////////////////////////////////////////
// CBackupProgessDlg dialog


CBackupProgessDlg::CBackupProgessDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBackupProgessDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBackupProgessDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CBackupProgessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBackupProgessDlg)
	DDX_Control(pDX, IDC_STATIC_LAST_MSG, m_nxstaticLastMsg);
	DDX_Control(pDX, IDC_STATIC_LAST_MSG2, m_nxstaticLastMsg2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBackupProgessDlg, CDialog)
	//{{AFX_MSG_MAP(CBackupProgessDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBackupProgessDlg message handlers
