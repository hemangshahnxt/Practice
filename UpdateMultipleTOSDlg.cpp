// UpdateMultipleTOSDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UpdateMultipleTOSDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUpdateMultipleTOSDlg dialog


CUpdateMultipleTOSDlg::CUpdateMultipleTOSDlg(CWnd* pParent)
	: CNxDialog(CUpdateMultipleTOSDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpdateMultipleTOSDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CUpdateMultipleTOSDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpdateMultipleTOSDlg)
	DDX_Control(pDX, IDC_UNSEL_ONE, m_btnUnselOne);
	DDX_Control(pDX, IDC_UNSEL_ALL, m_btnUnselAll);
	DDX_Control(pDX, IDC_SEL_ALL, m_btnSelAll);
	DDX_Control(pDX, IDC_SEL_ONE, m_btnSelOne);
	DDX_Control(pDX, IDC_TOS_CODE, m_nxeditTosCode);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_UPDATE_TOS, m_btnUpdate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUpdateMultipleTOSDlg, CNxDialog)
	//{{AFX_MSG_MAP(CUpdateMultipleTOSDlg)
	ON_BN_CLICKED(IDC_SEL_ONE, OnSelOne)
	ON_BN_CLICKED(IDC_SEL_ALL, OnSelAll)
	ON_BN_CLICKED(IDC_UNSEL_ONE, OnUnselOne)
	ON_BN_CLICKED(IDC_UNSEL_ALL, OnUnselAll)
	ON_BN_CLICKED(IDC_UPDATE_TOS, OnUpdateTos)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateMultipleTOSDlg message handlers

void CUpdateMultipleTOSDlg::OnOK() 
{
	OnCancel();
}

//Close button
void CUpdateMultipleTOSDlg::OnCancel() 
{
	CDialog::OnCancel();
}

BOOL CUpdateMultipleTOSDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_pAvail = BindNxDataListCtrl(IDC_AVAILABLE_TOS_LIST);
		m_pSelected = BindNxDataListCtrl(IDC_SELECTED_TOS_LIST, false);

		m_btnSelOne.AutoSet(NXB_RIGHT);
		m_btnSelAll.AutoSet(NXB_RRIGHT);
		m_btnUnselOne.AutoSet(NXB_LEFT);
		m_btnUnselAll.AutoSet(NXB_LLEFT);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnUpdate.AutoSet(NXB_MODIFY);

		((CNxEdit*)GetDlgItem(IDC_TOS_CODE))->LimitText(2);

	} NxCatchAll("Error in OnInitDialog()");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUpdateMultipleTOSDlg::OnSelOne() 
{
	try {
		m_pSelected->TakeCurrentRow(m_pAvail);
	} NxCatchAll("Error selecting service code.");
}

void CUpdateMultipleTOSDlg::OnSelAll() 
{
	try {
		m_pSelected->TakeAllRows(m_pAvail);
	} NxCatchAll("Error selecting all service codes.");
}

void CUpdateMultipleTOSDlg::OnUnselOne() 
{
	try {
		m_pAvail->TakeCurrentRow(m_pSelected);
	} NxCatchAll("Error unselecting service codes.");
}

void CUpdateMultipleTOSDlg::OnUnselAll() 
{
	try {
		m_pAvail->TakeAllRows(m_pSelected);
	} NxCatchAll("Error unselecting all service codes.");
}

void CUpdateMultipleTOSDlg::OnUpdateTos() 
{
	try {
		//make sure something is selected
		if(m_pSelected->GetRowCount() == 0) {
			MsgBox("You must select at least 1 code.");
			return;
		}

		//Allow them to not enter a code (it will blank everything out), but give
		//	an extra warning.
		CString strNewTOS;
		GetDlgItemText(IDC_TOS_CODE, strNewTOS);
		if(strNewTOS.IsEmpty()) {
			if(AfxMessageBox("You have chosen to clear out the type of service field (no TOS code entered below).  Are you sure you wish to do this?", MB_YESNO) != IDYES)
				return;
		}

		//The TOS code is limited to only 2 characters!  We have to stop them if they entered more
		if(strNewTOS.GetLength() > 2) {
			MsgBox("Type of Service codes are limited to 2 characters.  You must trim the TOS entered to be no more than 2 characters to proceed.");
			return;
		}

		//confirm
		if(AfxMessageBox("Are you absolutely SURE you wish to update these selected codes?  This cannot be undone.", MB_YESNO) != IDYES)
			return;

		CWaitCursor pWait;

		//generate a query with all these ids
		CString strIDList = "", str = "";
		for(int i = 0; i < m_pSelected->GetRowCount(); i++) {
			str.Format("%li, ", VarLong(m_pSelected->GetValue(i, 0)));
			strIDList += str;	//add the ID number to our comma separated list
		}

		strIDList.TrimRight(", ");	//remove any trailing commas/spaces

		//shouldn't be possible, but the query will fail if true
		if(strIDList.IsEmpty()) {
			MsgBox("Empty list, failed to update.");
			return;
		}

		//Change the timeout so we don't get an error.
		// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Use CIncreaseCommandTimeout instead of accessing g_ptrRemoteData
		CIncreaseCommandTimeout ict(600);

		//now update our cptcode table with all changes
		ExecuteSql("UPDATE CPTCodeT SET TypeOfService = '%s' WHERE ID IN (%s)", _Q(strNewTOS), strIDList);

		MsgBox("Codes successfully updated!");
		OnCancel();
	} NxCatchAll("Error updating TOS codes");
}

BEGIN_EVENTSINK_MAP(CUpdateMultipleTOSDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CUpdateMultipleTOSDlg)
	ON_EVENT(CUpdateMultipleTOSDlg, IDC_AVAILABLE_TOS_LIST, 3 /* DblClickCell */, OnDblClickCellAvailableTosList, VTS_I4 VTS_I2)
	ON_EVENT(CUpdateMultipleTOSDlg, IDC_SELECTED_TOS_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedTosList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CUpdateMultipleTOSDlg::OnDblClickCellAvailableTosList(long nRowIndex, short nColIndex) 
{
	OnSelOne();
}

void CUpdateMultipleTOSDlg::OnDblClickCellSelectedTosList(long nRowIndex, short nColIndex) 
{
	OnUnselOne();
}
