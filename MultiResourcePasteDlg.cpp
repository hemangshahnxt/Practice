// MultiResourcePasteDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MultiResourcePasteDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CMultiResourcePasteDlg dialog


CMultiResourcePasteDlg::CMultiResourcePasteDlg(CWnd* pParent, CString overrideTitle, CString overrideText)
	: CNxDialog(CMultiResourcePasteDlg::IDD, pParent)
	, m_overrideTitle(overrideTitle)
	, m_overrideText(overrideText)
{
	//{{AFX_DATA_INIT(CMultiResourcePasteDlg)
	//}}AFX_DATA_INIT
}


void CMultiResourcePasteDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiResourcePasteDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiResourcePasteDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMultiResourcePasteDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiResourcePasteDlg message handlers

BOOL CMultiResourcePasteDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (a.walling 2015-01-29 09:05) - PLID 64413 - Override title and text

	if (!m_overrideTitle.IsEmpty()) {
		SetWindowText(m_overrideTitle);
	}

	if (!m_overrideText.IsEmpty()) {
		if (auto pText = GetDlgItem(IDC_STATIC_DESCRIPTION)) {
			pText->SetWindowText(m_overrideText);
		}
	}

	COleVariant varChecked;

	varChecked.vt = VT_BOOL;
	varChecked.lVal = TRUE;
	
	m_dlList = BindNxDataListCtrl(this, IDC_MULTISELECT_LIST, GetRemoteData(), true);
	m_dlList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	
	// (z.manning, 04/30/2008) - PLID 29845 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	for (long i=0; i < m_adwSelected.GetSize(); i++)
	{
		long lRow = m_dlList->FindByColumn(0, _variant_t((long)m_adwSelected[i]), 0, 0);
		if (lRow > -1)
			m_dlList->PutValue(lRow, 1, varChecked);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMultiResourcePasteDlg::OnOK() 
{
	m_adwSelected.RemoveAll();
	for (long i=0; i < m_dlList->GetRowCount(); i++)
	{
		if (m_dlList->GetValue(i, 1).boolVal)
			m_adwSelected.Add(VarLong(m_dlList->GetValue(i,0)));
	}
	
	CNxDialog::OnOK();
}
