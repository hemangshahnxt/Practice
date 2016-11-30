// NxModalParentDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "NxModalParentDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNxModalParentDlg dialog


// (j.fouts 2013-01-21 09:29) - PLID 54703 - Added an overload to take in the size of the window
CNxModalParentDlg::CNxModalParentDlg(CWnd* pParent, CNxDialog *child, CString &str, const CRect& rectSize)
	: m_pChild(child), m_title(str), m_rectSize(rectSize), m_bUseDefaultSize(false), 
	CNxDialog(CNxModalParentDlg::IDD, pParent)
{
	ASSERT(child);
}

// (j.fouts 2013-01-21 09:29) - PLID 54703 - Use the default size
CNxModalParentDlg::CNxModalParentDlg(CWnd* pParent, CNxDialog *child, CString &str)
	: m_bUseDefaultSize(true),
	CNxDialog(CNxModalParentDlg::IDD, pParent)
{
	ASSERT(child);
	m_pChild = child;
	m_title = str;
	//{{AFX_DATA_INIT(CNxModalParentDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNxModalParentDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNxModalParentDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNxModalParentDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNxModalParentDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNxModalParentDlg message handlers

// (j.fouts 2013-01-21 09:29) - PLID 54703 - Added a try/ctach, and resize the window if we are not using the defaults
BOOL CNxModalParentDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

		//Resize if we are not using defualt
		if(!m_bUseDefaultSize)
		{
			MoveWindow(m_rectSize);
		}
		
		RECT rect;
		GetClientRect(&rect);
		m_pChild->Create(m_pChild->IDD, this);
		// (z.manning 2010-07-06 17:48) - PLID 36511 - Flag this dialog as being modal
		m_pChild->m_bIsModal = TRUE;
		m_pChild->MoveWindow(&rect, FALSE);
		//DRT 5/1/2008 - PLID 29865 - We must force the dialog to resize its controls to fit in the window.  This always
		//	worked previously because we just happened to keep the resources the same size in the few places this is
		//	used.
		m_pChild->SetControlPositions();
		SetWindowText(m_title);
		m_pChild->ShowWindow(SW_SHOW);

		//Center the window
		CenterWindow();
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

//user the 'x'
void CNxModalParentDlg::OnCancel() 
{
	m_pChild->StoreDetails();
	CDialog::OnCancel();
}

//doesn't happen, do nothing if it did
void CNxModalParentDlg::OnOK() 
{
//	CDialog::OnOK();
}
