// VoidSuperbillDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "VoidSuperbillDlg.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////
//Columns
#define COL_ID		0
#define COL_PRINT	1
#define	COL_NAME	2
#define COL_APPT	3
#define COL_VOID	4

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define IDM_VOID	50000
#define IDM_UNVOID	50001

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CVoidSuperbillDlg dialog
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker

CVoidSuperbillDlg::CVoidSuperbillDlg(CWnd* pParent)
	: CNxDialog(CVoidSuperbillDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVoidSuperbillDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CVoidSuperbillDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVoidSuperbillDlg)
	DDX_Control(pDX, IDC_FROM, m_dtpFrom);
	DDX_Control(pDX, IDC_TO, m_dtpTo);
	DDX_Control(pDX, IDC_VOID, m_btnVoid);
	DDX_Control(pDX, IDC_UNVOID, m_btnUnvoid);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_DATE_FILTER, m_btnDateFilter);
	DDX_Control(pDX, IDC_INCLUDE_APPLIED, m_btnIncludeApplied);
	DDX_Control(pDX, IDC_HIDE_VOID, m_btnHideVoid);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CVoidSuperbillDlg, IDC_TO, 2 /* Change */, OnChangeTo, VTS_NONE)
//	ON_EVENT(CVoidSuperbillDlg, IDC_FROM, 2 /* Change */, OnChangeFrom, VTS_NONE)

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CVoidSuperbillDlg, CNxDialog)
	//{{AFX_MSG_MAP(CVoidSuperbillDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_FROM, OnChangeFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TO, OnChangeTo)
	ON_BN_CLICKED(IDC_DATE_FILTER, OnDateFilter)
	ON_BN_CLICKED(IDC_VOID, OnVoid)
	ON_BN_CLICKED(IDC_UNVOID, OnUnvoid)
	ON_BN_CLICKED(IDC_INCLUDE_APPLIED, OnIncludeApplied)
	ON_COMMAND(IDM_VOID, OnMarkVoid)
	ON_COMMAND(IDM_UNVOID, OnMarkUnvoid)
	ON_BN_CLICKED(IDC_HIDE_VOID, OnHideVoid)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVoidSuperbillDlg message handlers

BOOL CVoidSuperbillDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnVoid.AutoSet(NXB_MODIFY);
	m_btnUnvoid.AutoSet(NXB_MODIFY);
	m_btnClose.AutoSet(NXB_CLOSE);

	//set the date filters to today
	m_dtpFrom.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
	m_dtpTo.SetValue(_variant_t(COleDateTime::GetCurrentTime()));

	m_listSuperbills = BindNxDataListCtrl(IDC_LIST_SUPERBILLS, false);

	RequeryWithFilters();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVoidSuperbillDlg::OnOK() 
{
	//we're actually cancelling, don't want Enter key to accidentally close
	OnCancel();
}

void CVoidSuperbillDlg::OnCancel() {

	CDialog::OnCancel();
}

void CVoidSuperbillDlg::OnDateFilter() 
{
	if(IsDlgButtonChecked(IDC_DATE_FILTER)) {
		//enable the dates
		m_dtpTo.EnableWindow(true);
		m_dtpFrom.EnableWindow(true);
	}
	else {
		m_dtpTo.EnableWindow(false);
		m_dtpFrom.EnableWindow(false);
	}

	RequeryWithFilters();
}

void CVoidSuperbillDlg::OnVoid() 
{
	try {
		bool bOneChanged = false;	//make sure we actually changed something

		//loop through all the selected items and mark them void
		LPDISPATCH pDisp = NULL;
		int i = 0;
		long p = m_listSuperbills->GetFirstSelEnum();

		if(!p) {
			//there are no items selected)
			MsgBox("You must select at least 1 item to void first.");
			return;
		}


		//confirm
		if(AfxMessageBox("Are you absolutely sure you wish to mark these Superbills as void?", MB_YESNO) == IDNO)
			return;


		while (p)
		{
			i++;
			m_listSuperbills->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			if(!VarBool(pRow->GetValue(COL_VOID)))
				bOneChanged = true;

			ExecuteSql("UPDATE PrintedSuperbillsT SET Void = 1, VoidDate = GetDate(), VoidUser = '%s' WHERE PrintedSuperbillsT.SavedID = %li", _Q(GetCurrentUserName()), VarLong(pRow->GetValue(COL_ID)));

			//mark the row void
			pRow->PutValue(COL_VOID, bool(true));

			// (m.hancock 2006-05-12 10:56) - PLID 14275 - If we're hiding void superbills,
			// we need to remove this from the list.
			if(IsDlgButtonChecked(IDC_HIDE_VOID)) {
				m_listSuperbills->RemoveRow(pRow->GetIndex());
			}

			pDisp->Release();
		}

		if(!bOneChanged) {
			//we didn't actually update anything
			MsgBox("All selected items are already void.");
		}

	} NxCatchAll("Error Voiding Superbills");

}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CVoidSuperbillDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CVoidSuperbillDlg)
	ON_EVENT(CVoidSuperbillDlg, IDC_LIST_SUPERBILLS, 6 /* RButtonDown */, OnRButtonDownListSuperbills, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CVoidSuperbillDlg::OnChangeTo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	RequeryWithFilters();

	*pResult = 0;
}

void CVoidSuperbillDlg::OnChangeFrom(NMHDR* pNMHDR, LRESULT* pResult) 
{
	RequeryWithFilters();

	*pResult = 0;
}

void CVoidSuperbillDlg::RequeryWithFilters() {

	//figure out the where clause, put it in, and call a requery
	CString strFrom, strTo, sql, str;

	if(IsDlgButtonChecked(IDC_DATE_FILTER)) {
		COleDateTime dtTo;
		COleDateTimeSpan dtSpan(1, 0, 0, 0);

		strFrom = FormatDateTimeForSql((COleDateTime)m_dtpFrom.GetValue(), dtoDate);
		dtTo = ((COleDateTime)m_dtpTo.GetValue()) + dtSpan;

		strTo =	FormatDateTimeForSql(dtTo, dtoDate);

		sql.Format("PrintedOn >= '%s' AND PrintedOn <= '%s'", strFrom, strTo);
	}

	//if they want applied, we dont need to filter anything, otherwise:
	if(!IsDlgButtonChecked(IDC_INCLUDE_APPLIED)) {
		//exclude applied
		str.Format("SavedID NOT IN (SELECT SuperbillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID WHERE SuperbillID IS NOT NULL AND LineItemT.Deleted = 0 AND BillsT.Deleted = 0)");
	}

	//now combine our 2 filters
	if(sql.IsEmpty())
		sql = str;
	else {
		if(!str.IsEmpty())
			sql += " AND " + str;
	}

	// (m.hancock 2006-05-12 09:38) - PLID 14275 - Allow users to hide 
	// void superbills on the "Void Superbills" dialog.
	if(IsDlgButtonChecked(IDC_HIDE_VOID)) {
		if(sql.IsEmpty())
			sql = "Void = 0";
		else
			sql += " AND Void = 0";
	}

	m_listSuperbills->PutWhereClause(_bstr_t(sql));

	m_listSuperbills->Requery();

}

void CVoidSuperbillDlg::OnUnvoid() 
{
	try {
		bool bOneChanged = false;

		//loop through all the selected items and mark them void
		LPDISPATCH pDisp = NULL;
		int i = 0;
		long p = m_listSuperbills->GetFirstSelEnum();

		if(!p) {
			MsgBox("You must select at least 1 item to mark as no longer void.");
			return;
		}

		//confirm
		if(AfxMessageBox("Are you absolutely sure you wish to mark these Superbills as no longer void?", MB_YESNO) == IDNO)
			return;

		while (p)
		{
			i++;
			m_listSuperbills->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			if(VarBool(pRow->GetValue(COL_VOID)))
				bOneChanged = true;

			ExecuteSql("UPDATE PrintedSuperbillsT SET Void = 0, VoidDate = NULL, VoidUser = '' WHERE PrintedSuperbillsT.SavedID = %li", VarLong(pRow->GetValue(COL_ID)));

			//mark the row not void
			pRow->PutValue(COL_VOID, bool(false));

			pDisp->Release();
		}

		if(!bOneChanged) {
			//no items have changed
			MsgBox("All selected items are already not marked as void.");
		}

		//remove the selection
//		RequeryWithFilters();

	} NxCatchAll("Error Voiding Superbills");

}

void CVoidSuperbillDlg::OnIncludeApplied() 
{
	RequeryWithFilters();
}

void CVoidSuperbillDlg::OnRButtonDownListSuperbills(long nRow, short nCol, long x, long y, long nFlags) 
{

	//set the selection to the rclicked row
	m_listSuperbills->PutCurSel(nRow);

	if(nRow == -1)
		return;


	CMenu mnu;
	mnu.m_hMenu = CreatePopupMenu();

	_variant_t var = m_listSuperbills->GetValue(nRow, COL_VOID);

	if(!VarBool(var))
		mnu.InsertMenu(0, MF_BYPOSITION, IDM_VOID, "Mark as &Void");
	else
		mnu.InsertMenu(0, MF_BYPOSITION, IDM_UNVOID, "Mark as &Unvoid");

	CPoint pt;
	GetCursorPos(&pt);
	mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);

}

void CVoidSuperbillDlg::OnMarkVoid()
{
	//marking the current item void from the right click menu
	OnVoid();
}

void CVoidSuperbillDlg::OnMarkUnvoid()
{
	//marking the current item unvoid from the right click menu
	OnUnvoid();
}

void CVoidSuperbillDlg::OnHideVoid() 
{
	// (m.hancock 2006-05-12 09:28) - PLID 14275 - Allow users to hide 
	// void superbills on the "Void Superbills" dialog.
	RequeryWithFilters();
}
