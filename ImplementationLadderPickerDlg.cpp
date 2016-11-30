// ImplementationLadderPickerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsrc.h"
#include "ImplementationLadderPickerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImplementationLadderPickerDlg dialog

enum ImplementationLadderColumns {
	ilcID = 0,
	ildName = 1,
};

CImplementationLadderPickerDlg::CImplementationLadderPickerDlg(CWnd* pParent)
	: CNxDialog(CImplementationLadderPickerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImplementationLadderPickerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CImplementationLadderPickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImplementationLadderPickerDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CImplementationLadderPickerDlg, CNxDialog)
	//{{AFX_MSG_MAP(CImplementationLadderPickerDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImplementationLadderPickerDlg message handlers

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CImplementationLadderPickerDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CImplementationLadderPickerDlg)
	ON_EVENT(CImplementationLadderPickerDlg, IDC_IMPLEMENTATION_LADDER_LIST, 1 /* SelChanging */, OnSelChangingImplementationLadderList, VTS_DISPATCH VTS_PDISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CImplementationLadderPickerDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CImplementationLadderPickerDlg::OnOK() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pLadderList->CurSel;

		if (pRow) {
			m_nLadderID = VarLong(pRow->GetValue(ilcID));
		
			CDialog::OnOK();
		}
		else {
			MsgBox("Please choose a ladder or click cancel.");
		}
	}NxCatchAll("Error in CImplementationLadderPickerDlg::OnOK() ");
}



void CImplementationLadderPickerDlg::OnSelChangingImplementationLadderList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("CImplementationLadderPickerDlg::OnSelChangingImplementationLadderList");
	
}

BOOL CImplementationLadderPickerDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog(); // (a.walling 2011-01-14 16:44) - no PLID - Fix bad base class OnInitDialog call

	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_pLadderList = BindNxDataList2Ctrl(IDC_IMPLEMENTATION_LADDER_LIST, TRUE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
