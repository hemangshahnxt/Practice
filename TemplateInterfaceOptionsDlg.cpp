// TemplateInterfaceOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "TemplateInterfaceOptionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTemplateInterfaceOptionsDlg dialog


CTemplateInterfaceOptionsDlg::CTemplateInterfaceOptionsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CTemplateInterfaceOptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTemplateInterfaceOptionsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bUseResourceAvailTemplates = false;
}


void CTemplateInterfaceOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTemplateInterfaceOptionsDlg)
	DDX_Control(pDX, IDC_DEFAULT_LINE_ITEMS_ALL_RESOURCES, m_btnDefAllResources);
	DDX_Control(pDX, IDC_DEFAULT_LINE_ITEMS_AS_BLOCKS, m_btnDefAsBlock);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTemplateInterfaceOptionsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CTemplateInterfaceOptionsDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_DEFAULT_LINE_ITEMS_AS_BLOCKS, &CTemplateInterfaceOptionsDlg::OnDefaultLineItemsAsBlocks)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTemplateInterfaceOptionsDlg message handlers

BOOL CTemplateInterfaceOptionsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/29/2008) - PLID 29814 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	//TES 6/19/2010 - PLID 5888 - Resource Availability templates can't be Precision templates, so if that's what we're editing, disable
	// that option.
	if(m_bUseResourceAvailTemplates) {
		GetDlgItem(IDC_DEFAULT_LINE_ITEMS_AS_BLOCKS)->EnableWindow(FALSE);
		//TES 6/21/2010 - PLID 5888 - Update the window title to indicate whether these are Resource Availability templates
		//(e.lally 2010-07-15) PLID 39626 - Renamed to Location Templates
		SetWindowText("Location Template Options");
	}
	else {
		//TES 12/19/2008 - PLID 32537 - Scheduler Standard users aren't allowed to create precision templates.
		if(g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
			if(GetRemotePropertyInt("DefaultTemplateLineItemsAsblocks", 0, 0, GetCurrentUserName(), true) == 1) {
				CheckDlgButton(IDC_DEFAULT_LINE_ITEMS_AS_BLOCKS, BST_CHECKED);
			}
		}
	}
	
	//TES 6/21/2010 - PLID 5888 - Split into separate properties.
	CString strProperty = m_bUseResourceAvailTemplates?"DefaultResourceAvailTemplateLineItemsAllResources":"DefaultTemplateLineItemsAllResources";
	if(GetRemotePropertyInt(strProperty, 0, 0, GetCurrentUserName(), true) == 1) {
		CheckDlgButton(IDC_DEFAULT_LINE_ITEMS_ALL_RESOURCES, BST_CHECKED);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTemplateInterfaceOptionsDlg::OnOK() 
{
	try {
		
		Save();

		CDialog::OnOK();

	}NxCatchAll("CTemplateInterfaceOptionsDlg::OnOK");
}

void CTemplateInterfaceOptionsDlg::Save()
{
	int nValue;

	if(IsDlgButtonChecked(IDC_DEFAULT_LINE_ITEMS_AS_BLOCKS)) {
		nValue = 1;
	}
	else {
		nValue = 0;
	}
	//TES 7/2/2010 - PLID 5888 - Don't set this property if the box is disabled.
	if(!m_bUseResourceAvailTemplates) {
		SetRemotePropertyInt("DefaultTemplateLineItemsAsblocks", nValue, 0, GetCurrentUserName());
	}

	if(IsDlgButtonChecked(IDC_DEFAULT_LINE_ITEMS_ALL_RESOURCES)) {
		nValue = 1;
	}
	else {
		nValue = 0;
	}
	//TES 6/21/2010 - PLID 5888 - Split into separate properties.
	CString strProperty = m_bUseResourceAvailTemplates?"DefaultResourceAvailTemplateLineItemsAllResources":"DefaultTemplateLineItemsAllResources";
	SetRemotePropertyInt(strProperty, nValue, 0, GetCurrentUserName());
}

void CTemplateInterfaceOptionsDlg::OnDefaultLineItemsAsBlocks()
{
	try {
		if(IsDlgButtonChecked(IDC_DEFAULT_LINE_ITEMS_AS_BLOCKS)) {
			//TES 12/19/2008 - PLID 32537 - Scheduler Standard users aren't allowed to create precision templates.
			if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Precision Scheduler templating", "System_Setup/Scheduler_Setup/Precision_Templating.htm")) {
				CheckDlgButton(IDC_DEFAULT_LINE_ITEMS_AS_BLOCKS, BST_UNCHECKED);
			}
		}
	}NxCatchAll("Error in CTemplateInterfaceOptionsDlg::OnDefaultLineItemsAsBlocks()");

}
