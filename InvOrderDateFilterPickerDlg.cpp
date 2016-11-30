// InvOrderDateFilterPickerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "inventoryrc.h"
#include "InvOrderDateFilterPickerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


enum FilterColumnList {
	fclID,
	fclName,
};

/////////////////////////////////////////////////////////////////////////////
// CInvOrderDateFilterPickerDlg dialog
// (j.gruber 2008-06-30 10:01) - PLID 29205 - created for

CInvOrderDateFilterPickerDlg::CInvOrderDateFilterPickerDlg(long nFilterID, CView* pParent /*=NULL*/)
	: CNxDialog(CInvOrderDateFilterPickerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInvOrderDateFilterPickerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nFilterID = nFilterID;
	m_nOrigFilterID = nFilterID;
}


void CInvOrderDateFilterPickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvOrderDateFilterPickerDlg)
	DDX_Control(pDX, IDCANCEL, m_CancelBtn);
	DDX_Control(pDX, IDOK, m_OKBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInvOrderDateFilterPickerDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInvOrderDateFilterPickerDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvOrderDateFilterPickerDlg message handlers

BEGIN_EVENTSINK_MAP(CInvOrderDateFilterPickerDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvOrderDateFilterPickerDlg)
	ON_EVENT(CInvOrderDateFilterPickerDlg, IDC_INV_ORDER_DATE_FILTER_LIST, 16 /* SelChosen */, OnSelChosenInvOrderDateFilterList, VTS_DISPATCH)
	ON_EVENT(CInvOrderDateFilterPickerDlg, IDC_INV_ORDER_DATE_FILTER_LIST, 1 /* SelChanging */, OnSelChangingInvOrderDateFilterList, VTS_DISPATCH VTS_PDISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInvOrderDateFilterPickerDlg::OnOK() 
{
	try {
		//make sure its a valid id
		if ((m_nFilterID != DATE_CREATED) && (m_nFilterID != DATE_RECEIVED)) {
			MsgBox("Please select a date filter from the list or click cancel.");
		}
		else {
		
			CNxDialog::OnOK();
		}
	}NxCatchAll("Error in OnOK");
}

void CInvOrderDateFilterPickerDlg::OnCancel() 
{
	try {
		//set it back to the original
		m_nFilterID = m_nOrigFilterID;
	}NxCatchAll("Error in OnCancel");
	
	CNxDialog::OnCancel();
}

void CInvOrderDateFilterPickerDlg::OnSelChosenInvOrderDateFilterList(LPDISPATCH lpRow) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			
			m_nFilterID = VarLong(pRow->GetValue(fclID));
		}

	}NxCatchAll("Error in CInvOrderDateFilterPickerDlg::OnSelChosenInvOrderDateFilterList");
	
}

void CInvOrderDateFilterPickerDlg::OnSelChangingInvOrderDateFilterList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {

		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

	}NxCatchAll("Error in CInvOrderDateFilterPickerDlg::OnSelChangingInvOrderDateFilterList");
	
}

BOOL CInvOrderDateFilterPickerDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
	
		m_pFilterList = BindNxDataList2Ctrl(IDC_INV_ORDER_DATE_FILTER_LIST, false);

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFilterList->GetNewRow();
		pRow->PutValue(fclID, (long)DATE_CREATED);
		pRow->PutValue(fclName, _variant_t("Created Date"));
		m_pFilterList->AddRowAtEnd(pRow, NULL);

		pRow = m_pFilterList->GetNewRow();
		pRow->PutValue(fclID, (long)DATE_RECEIVED);
		pRow->PutValue(fclName, _variant_t("Received Date"));
		m_pFilterList->AddRowAtEnd(pRow, NULL);

		m_pFilterList->SetSelByColumn(fclID, m_nFilterID);


		m_OKBtn.AutoSet(NXB_OK);
		m_CancelBtn.AutoSet(NXB_CANCEL);
		
	}NxCatchAll("Error in CInvOrderDateFilterPickerDlg::OnInitDialog() ");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CInvOrderDateFilterPickerDlg::SetFilterID(long nFilterID) 
{
	try {

		m_nFilterID = nFilterID;

	}NxCatchAll("Error in SetFilterID");

}


long CInvOrderDateFilterPickerDlg::GetFilterID() 
{
	try {

		return m_nFilterID;

	}NxCatchAll("Error in GetFilterID");

	// (a.walling 2008-10-02 10:34) - PLID 31567 - Needs a return value
	return -1;
}
