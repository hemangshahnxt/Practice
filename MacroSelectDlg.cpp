// MacroSelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MacroSelectDlg.h"
#include "MacroEditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2011-05-10 10:05) - PLID 41789 - enum for columns
enum SelectMacroColumns {
	smcID = 0,
	smcCategoryID,
	smcCategory,
	smcDescription,
	smcShowOnStatement
};

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CMacroSelectDlg dialog

CMacroSelectDlg::CMacroSelectDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMacroSelectDlg::IDD, pParent)
	, m_bForBillingNotes(false) // (a.walling 2011-05-10 10:05) - PLID 41789 - Whether this is for billing notes or not
{
	//{{AFX_DATA_INIT(CMacroSelectDlg)
		// NOTE: the ClassWizard will add member initialization here
		m_nSelectedID = -1;
	//}}AFX_DATA_INIT
}


void CMacroSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMacroSelectDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMacroSelectDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMacroSelectDlg)
	ON_BN_CLICKED(IDC_EDIT_MACROS, OnEditMacros)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMacroSelectDlg message handlers

void CMacroSelectDlg::OnEditMacros() 
{
	CMacroEditDlg dlg(this);
	dlg.DoModal();
	m_dlMacroList->Requery();
}

BEGIN_EVENTSINK_MAP(CMacroSelectDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMacroSelectDlg)
	ON_EVENT(CMacroSelectDlg, IDC_MACRO_LIST, 3 /* DblClickCell */, OnDblClickCellMacroList, VTS_I4 VTS_I2)
	ON_EVENT(CMacroSelectDlg, IDC_MACRO_LIST, 2 /* SelChanged */, OnSelChangedMacroList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMacroSelectDlg::OnDblClickCellMacroList(long nRowIndex, short nColIndex) 
{
	// return the ID of the macro selected
	m_nSelectedID = m_dlMacroList->GetValue(nRowIndex, 0);
	OnOK();
}

BOOL CMacroSelectDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2008-04-25 12:24) - PLID 29790 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_dlMacroList = BindNxDataListCtrl(this, IDC_MACRO_LIST, GetRemoteData(), false);

		// (a.walling 2011-05-10 10:05) - PLID 41789 - Collapse the show on statement column if not for billing notes
		if (!m_bForBillingNotes) {
			m_dlMacroList->GetColumn(smcShowOnStatement)->StoredWidth = 0;
		}

		m_dlMacroList->Requery();

	}NxCatchAll("Error loading MacroList");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CMacroSelectDlg::DoModal()
{
	return CNxDialog::DoModal();
}

void CMacroSelectDlg::OnOK() 
{
	CNxDialog::OnOK();
}

void CMacroSelectDlg::OnSelChangedMacroList(long nNewSel) 
{
	try{
		if(nNewSel == sriNoRow){
			m_nSelectedID = -1;
		}
		else{
			m_nSelectedID = m_dlMacroList->GetValue(nNewSel, 0);
		}
	}NxCatchAll("Error in CMacroSelectDlg::OnSelChangedMacroList");
	
}
