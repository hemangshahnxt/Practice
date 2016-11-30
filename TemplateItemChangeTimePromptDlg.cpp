// TemplateItemChangeTimePrompt.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "TemplateItemChangeTimePromptDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTemplateItemChangeTimePromptDlg dialog


CTemplateItemChangeTimePromptDlg::CTemplateItemChangeTimePromptDlg(CTemplateLineItemInfo* pLineItem, ESchedulerTemplateEditorType eType, CWnd* pParent /*=NULL*/)
	: CNxDialog(CTemplateItemChangeTimePromptDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTemplateItemChangeTimePromptDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pLineItem = pLineItem;
	m_strChangeType = "Change";
	m_eButtonStyle = NXB_MODIFY;
	m_eEditorType = eType;
}


void CTemplateItemChangeTimePromptDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTemplateItemChangeTimePromptDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_CHANGE_TIME_PROMPT, m_nxstaticChangeTimePrompt);
	DDX_Control(pDX, IDC_CHANGE_LINE_ITEM, m_btnChangeLineItem);
	DDX_Control(pDX, IDC_CHANGE_ONCE, m_btnChangeOnce);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTemplateItemChangeTimePromptDlg, CNxDialog)
	//{{AFX_MSG_MAP(CTemplateItemChangeTimePromptDlg)
	ON_BN_CLICKED(IDC_CHANGE_LINE_ITEM, OnChangeLineItem)
	ON_BN_CLICKED(IDC_CHANGE_ONCE, OnChangeOnce)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTemplateItemChangeTimePromptDlg message handlers

void CTemplateItemChangeTimePromptDlg::OnChangeLineItem() 
{
	EndDialog(ID_CHANGE_LINE_ITEM);	
}

void CTemplateItemChangeTimePromptDlg::OnChangeOnce() 
{
	EndDialog(ID_CHANGE_ONCE);
}

BOOL CTemplateItemChangeTimePromptDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/29/2008) - PLID 29814 - Set button styles
	m_btnChangeLineItem.AutoSet(m_eButtonStyle);
	m_btnChangeOnce.AutoSet(m_eButtonStyle);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	CString strChangeTypeLowercase = m_strChangeType;
	strChangeTypeLowercase.MakeLower();
	CString strPrompt = FormatString("You have chosen to %s a template item. Do you want to %s "
		"this item just for the currently visible date or change it everywhere it occurs, which is:\r\n\r\n(%s)",
		strChangeTypeLowercase, strChangeTypeLowercase, m_pLineItem->GetApparentDescription(m_eEditorType));
	SetDlgItemText(IDC_CHANGE_TIME_PROMPT, strPrompt);

	SetDlgItemText(IDC_CHANGE_LINE_ITEM, FormatString("%s All",m_strChangeType));
	SetDlgItemText(IDC_CHANGE_ONCE, FormatString("%s Once",m_strChangeType));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTemplateItemChangeTimePromptDlg::SetChangeType(CString strChangeType)
{
	m_strChangeType = strChangeType;
	
	// (z.manning, 04/29/2008) - PLID 29814 - The button style can differ depending on whether we're prompting
	// to change or delete
	if(m_strChangeType.CompareNoCase("delete") == 0) {
		m_eButtonStyle = NXB_DELETE;
	}
	else {
		ASSERT(m_strChangeType.CompareNoCase("change") == 0);
		m_eButtonStyle = NXB_MODIFY;
	}
}
