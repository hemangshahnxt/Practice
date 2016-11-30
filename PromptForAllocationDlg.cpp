// PromptForAllocationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "PromptForAllocationDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//TES 6/13/2008 - PLID 28078 - Created
/////////////////////////////////////////////////////////////////////////////
// CPromptForAllocationDlg dialog


CPromptForAllocationDlg::CPromptForAllocationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPromptForAllocationDlg::IDD, pParent)
{
	m_bCanCreateAllocation = FALSE;
	m_bCanCreateOrder = FALSE;
	//{{AFX_DATA_INIT(CPromptForAllocationDlg)
	//}}AFX_DATA_INIT
}


void CPromptForAllocationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPromptForAllocationDlg)
	DDX_Control(pDX, IDC_DISMISS, m_nxbDismiss);
	DDX_Control(pDX, IDC_CREATE_ORDER, m_nxbCreateOrder);
	DDX_Control(pDX, IDC_CREATE_ALLOCATION, m_nxbCreateAllocation);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPromptForAllocationDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPromptForAllocationDlg)
	ON_BN_CLICKED(IDC_CREATE_ALLOCATION, OnCreateAllocation)
	ON_BN_CLICKED(IDC_CREATE_ORDER, OnCreateOrder)
	ON_BN_CLICKED(IDC_DISMISS, OnDismiss)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPromptForAllocationDlg message handlers

BOOL CPromptForAllocationDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog(); // (a.walling 2011-01-14 16:44) - no PLID - Fix bad base class OnInitDialog call
	
	try {
		m_nxbCreateAllocation.AutoSet(NXB_NEW);
		m_nxbCreateOrder.AutoSet(NXB_NEW);
		m_nxbDismiss.AutoSet(NXB_CLOSE);

		m_nxbCreateAllocation.EnableWindow(m_bCanCreateAllocation);
		m_nxbCreateOrder.EnableWindow(m_bCanCreateOrder);
	}NxCatchAll("Error in CPromptForAllocationDlg::OnInitDialog()");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPromptForAllocationDlg::OnCreateAllocation() 
{
	try {
		EndDialog(CREATE_ALLOCATION);
	}NxCatchAll("Error in CPromptForAllocationDlg::OnCreateAllocation()");
}

void CPromptForAllocationDlg::OnCreateOrder() 
{
	try {
		EndDialog(CREATE_ORDER);
	}NxCatchAll("Error in CPromptForAllocationDlg::OnCreateOrder()");
}

void CPromptForAllocationDlg::OnDismiss() 
{
	try {
		EndDialog(IDCANCEL);
	}NxCatchAll("Error in CPromptForAllocationDlg::OnDismiss()");
}
