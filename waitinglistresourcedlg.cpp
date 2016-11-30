// WaitingListResourceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "schedulerRc.h"
#include "WaitingListResourceDlg.h"
#include "GlobalUtils.h"
#include "GlobalDataUtils.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWaitingListResourceDlg dialog

// (d.moore 2007-05-23 11:31) - PLID 4013

enum eWaitListResourceCombo {
	ewlrcID, 
	ewlrcInUse, 
	ewlrcItem
};

CWaitingListResourceDlg::CWaitingListResourceDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CWaitingListResourceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWaitingListResourceDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	
	m_bDataChanged = false;
}

CWaitingListResourceDlg::~CWaitingListResourceDlg()
{
	
}

void CWaitingListResourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWaitingListResourceDlg)
	DDX_Control(pDX, IDC_RADIO_SELECT_RESOURCES, m_btnSelectResources);
	DDX_Control(pDX, IDC_RADIO_ALL_RESOURCES, m_btnAllResources);
	DDX_Control(pDX, IDC_WEDNESDAY_CHECK, m_btnWednesday);
	DDX_Control(pDX, IDC_TUESDAY_CHECK, m_btnTuesday);
	DDX_Control(pDX, IDC_THURSDAY_CHECK, m_btnThursday);
	DDX_Control(pDX, IDC_SUNDAY_CHECK, m_btnSunday);
	DDX_Control(pDX, IDC_SATURDAY_CHECK, m_btnSaturday);
	DDX_Control(pDX, IDC_FRIDAY_CHECK, m_btnFriday);
	DDX_Control(pDX, IDC_MONDAY_CHECK, m_btnMonday);
	DDX_Control(pDX, IDC_WL_DATE_START, m_dtcStartDate);
	DDX_Control(pDX, IDC_WL_DATE_END, m_dtcEndDate);
	DDX_Control(pDX, IDC_RESULTS_LABEL, m_nxstaticResultsLabel);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_DATES_GROUPBOX, m_btnDatesGroupbox);
	DDX_Control(pDX, IDC_TIME_GROUPBOX, m_btnTimeGroupbox);
	DDX_Control(pDX, IDC_INCLUDE_FRAME, m_btnIncludeFrame);
	DDX_Control(pDX, IDC_RESOURCE_GROUPBOX, m_btnResourceGroupbox);
	DDX_Control(pDX, IDC_STATIC_RESULTS, m_btnResults);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CWaitingListResourceDlg, IDC_WL_DATE_START, 2 /* Change */, OnChangeDateStart, VTS_NONE)
//	ON_EVENT(CWaitingListResourceDlg, IDC_WL_DATE_END, 2 /* Change */, OnChangeDateEnd, VTS_NONE)


BEGIN_MESSAGE_MAP(CWaitingListResourceDlg, CNxDialog)
	//{{AFX_MSG_MAP(CWaitingListResourceDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_WL_DATE_START, OnChangeDateStart)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_WL_DATE_END, OnChangeDateEnd)
	ON_BN_CLICKED(IDC_MONDAY_CHECK, OnMondayCheck)
	ON_BN_CLICKED(IDC_TUESDAY_CHECK, OnTuesdayCheck)
	ON_BN_CLICKED(IDC_WEDNESDAY_CHECK, OnWednesdayCheck)
	ON_BN_CLICKED(IDC_THURSDAY_CHECK, OnThursdayCheck)
	ON_BN_CLICKED(IDC_FRIDAY_CHECK, OnFridayCheck)
	ON_BN_CLICKED(IDC_SATURDAY_CHECK, OnSaturdayCheck)
	ON_BN_CLICKED(IDC_SUNDAY_CHECK, OnSundayCheck)
	ON_BN_CLICKED(IDC_RADIO_ALL_RESOURCES, OnRadioAllResources)
	ON_BN_CLICKED(IDC_RADIO_SELECT_RESOURCES, OnRadioSelectResources)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaitingListResourceDlg message handlers


BEGIN_EVENTSINK_MAP(CWaitingListResourceDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CWaitingListResourceDlg)
	ON_EVENT(CWaitingListResourceDlg, IDC_WL_RESOURCES, 10 /* EditingFinished */, OnEditingFinishedWlResources, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CWaitingListResourceDlg, IDC_WL_END_TIME, 2 /* Changed */, OnChangedWlEndTime, VTS_NONE)
	ON_EVENT(CWaitingListResourceDlg, IDC_WL_START_TIME, 2 /* Changed */, OnChangedWlStartTime, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CWaitingListResourceDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		// (z.manning, 04/29/2008) - PLID 29814 - Set button styles
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_nxtStartTime = BindNxTimeCtrl(this, IDC_WL_START_TIME);
		m_nxtEndTime = BindNxTimeCtrl(this, IDC_WL_END_TIME);

		m_pResources = BindNxDataList2Ctrl(this, IDC_WL_RESOURCES, GetRemoteData(), false);
		m_pResources->Requery();
		
		LoadFormValues();
		
		return TRUE;  // return TRUE unless you set the focus to a control
					  // EXCEPTION: OCX Property Pages should return FALSE
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnInitDialog");

	return FALSE;
}

void CWaitingListResourceDlg::OnOK() 
{
	try {
		RefreshLineItemData(m_LineItem);
		if (ValidateData(m_LineItem)) {
			CDialog::OnOK();
		}
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnOK");
}

void CWaitingListResourceDlg::OnCancel() 
{
	try {
		CDialog::OnCancel();
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnCancel");
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CWaitingListResourceDlg::OnChangeDateStart(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		m_bDataChanged = true;
		RefreshLineItemData(m_LineItem);
		RefreshLineItemText(m_LineItem);
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnChangeDateStart");

	*pResult = 0;
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CWaitingListResourceDlg::OnChangeDateEnd(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		m_bDataChanged = true;
		RefreshLineItemData(m_LineItem);
		RefreshLineItemText(m_LineItem);
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnChangeDateEnd");

	*pResult = 0;
}

void CWaitingListResourceDlg::OnChangedWlStartTime() 
{
	try {
		m_bDataChanged = true;
		RefreshLineItemData(m_LineItem);
		RefreshLineItemText(m_LineItem);
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnChangedWlStartTime");
}

void CWaitingListResourceDlg::OnChangedWlEndTime() 
{
	try {
		m_bDataChanged = true;
		RefreshLineItemData(m_LineItem);
		RefreshLineItemText(m_LineItem);
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnChangedWlEndTime");
}

void CWaitingListResourceDlg::OnMondayCheck() 
{
	try {
		m_bDataChanged = true;
		RefreshLineItemData(m_LineItem);
		RefreshLineItemText(m_LineItem);
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnMondayCheck");
}

void CWaitingListResourceDlg::OnTuesdayCheck() 
{
	try {
		m_bDataChanged = true;
		RefreshLineItemData(m_LineItem);
		RefreshLineItemText(m_LineItem);
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnTuesdayCheck");
}

void CWaitingListResourceDlg::OnWednesdayCheck() 
{
	try {
		m_bDataChanged = true;
		RefreshLineItemData(m_LineItem);
		RefreshLineItemText(m_LineItem);
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnWednesdayCheck");
}

void CWaitingListResourceDlg::OnThursdayCheck() 
{
	try {
		m_bDataChanged = true;
		RefreshLineItemData(m_LineItem);
		RefreshLineItemText(m_LineItem);
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnThursdayCheck");
}

void CWaitingListResourceDlg::OnFridayCheck() 
{
	try {
		m_bDataChanged = true;
		RefreshLineItemData(m_LineItem);
		RefreshLineItemText(m_LineItem);
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnFridayCheck");
}

void CWaitingListResourceDlg::OnSaturdayCheck() 
{
	try {
		m_bDataChanged = true;
		RefreshLineItemData(m_LineItem);
		RefreshLineItemText(m_LineItem);
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnSaturdayCheck");
}

void CWaitingListResourceDlg::OnSundayCheck() 
{
	try {
		m_bDataChanged = true;
		RefreshLineItemData(m_LineItem);
		RefreshLineItemText(m_LineItem);
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnSundayCheck");
}

void CWaitingListResourceDlg::OnRadioAllResources() 
{
	try {
		if (IsDlgButtonChecked(IDC_RADIO_ALL_RESOURCES)) {
			GetDlgItem(IDC_WL_RESOURCES)->EnableWindow(false);
		} else {
			GetDlgItem(IDC_WL_RESOURCES)->EnableWindow(true);
		}

		m_bDataChanged = true;
		RefreshLineItemData(m_LineItem);
		RefreshLineItemText(m_LineItem);
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnRadioAllResources");
}

void CWaitingListResourceDlg::OnRadioSelectResources() 
{
	try {
		if (IsDlgButtonChecked(IDC_RADIO_SELECT_RESOURCES)) {
			GetDlgItem(IDC_WL_RESOURCES)->EnableWindow(true);
		} else {
			GetDlgItem(IDC_WL_RESOURCES)->EnableWindow(false);
		}

		m_bDataChanged = true;
		RefreshLineItemData(m_LineItem);
		RefreshLineItemText(m_LineItem);
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnRadioSelectResources");
}

void CWaitingListResourceDlg::OnEditingFinishedWlResources(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		m_bDataChanged = true;
		RefreshLineItemData(m_LineItem);
		RefreshLineItemText(m_LineItem);
	} NxCatchAll("Error In: CWaitingListResourceDlg::OnEditingFinishedWlResources");
}

void CWaitingListResourceDlg::LoadFormValues()
{
	// Sets control values using m_pLineItem.
	
	// Dates
	m_dtcStartDate.SetValue(COleVariant(m_LineItem.dtStartDate));
	m_dtcEndDate.SetValue(COleVariant(m_LineItem.dtEndDate));
	
	// Times
	m_nxtStartTime->SetDateTime(m_LineItem.dtStartTime);
	m_nxtEndTime->SetDateTime(m_LineItem.dtEndTime);
	
	SetDayCheckboxVals(m_LineItem);
	
	// Resource selection list.
	if (m_LineItem.bAllResources) {
		CheckDlgButton(IDC_RADIO_ALL_RESOURCES, TRUE);
		CheckDlgButton(IDC_RADIO_SELECT_RESOURCES, FALSE);
		GetDlgItem(IDC_WL_RESOURCES)->EnableWindow(FALSE);
	} else {
		CheckDlgButton(IDC_RADIO_ALL_RESOURCES, FALSE);
		CheckDlgButton(IDC_RADIO_SELECT_RESOURCES, TRUE);
		GetDlgItem(IDC_WL_RESOURCES)->EnableWindow(TRUE);
	}
	
	// Check the selected boxes in the list.
	SetResourceListVals(m_LineItem);

	// Set the text for the results output box at the bottom of the dialog.
	SetDlgItemText(IDC_RESULTS_LABEL, CWaitingListUtils::FormatLineItem(m_LineItem));
}

void CWaitingListResourceDlg::SetDayCheckboxVals(const WaitListLineItem &wlItem)
{
	// Sets the day checkboxes to match pItem->arDayIDs.
	
	// Initially set all of the checkboxes to off.
	CheckDlgButton(IDC_SUNDAY_CHECK, false);
	CheckDlgButton(IDC_MONDAY_CHECK, false);
	CheckDlgButton(IDC_TUESDAY_CHECK, false);
	CheckDlgButton(IDC_WEDNESDAY_CHECK, false);
	CheckDlgButton(IDC_THURSDAY_CHECK, false);
	CheckDlgButton(IDC_FRIDAY_CHECK, false);
	CheckDlgButton(IDC_SATURDAY_CHECK, false);
	
	// Check each box that matches an entry in the Day ID array.
	long nNumIDs = wlItem.arDayIDs.GetSize();
	for (long i = 0; i < nNumIDs; i++) {
		switch (wlItem.arDayIDs[i])
		{
			case 1:
				CheckDlgButton(IDC_SUNDAY_CHECK, true);
				break;
			case 2:
				CheckDlgButton(IDC_MONDAY_CHECK, true);
				break;
			case 3:
				CheckDlgButton(IDC_TUESDAY_CHECK, true);
				break;
			case 4:
				CheckDlgButton(IDC_WEDNESDAY_CHECK, true);
				break;
			case 5:
				CheckDlgButton(IDC_THURSDAY_CHECK, true);
				break;
			case 6:
				CheckDlgButton(IDC_FRIDAY_CHECK, true);
				break;
			case 7:
				CheckDlgButton(IDC_SATURDAY_CHECK, true);
				break;
			default:
				CheckDlgButton(IDC_SUNDAY_CHECK, true);
				break;
		}
	}
}

void CWaitingListResourceDlg::GetDayCheckboxVals(WaitListLineItem &wlItem)
{
	// Updates pItem.arDayIDs to match the day checkboxes.
	
	// Remove any existing day values.
	wlItem.arDayIDs.RemoveAll();
	wlItem.arDayNames.RemoveAll();

	if (IsDlgButtonChecked(IDC_SUNDAY_CHECK)) {
		wlItem.arDayIDs.Add(1);
	}
	
	if (IsDlgButtonChecked(IDC_MONDAY_CHECK)) {
		wlItem.arDayIDs.Add(2);
	}

	if (IsDlgButtonChecked(IDC_TUESDAY_CHECK)) {
		wlItem.arDayIDs.Add(3);
	}

	if (IsDlgButtonChecked(IDC_WEDNESDAY_CHECK)) {
		wlItem.arDayIDs.Add(4);
	}

	if (IsDlgButtonChecked(IDC_THURSDAY_CHECK)) {
		wlItem.arDayIDs.Add(5);
	}

	if (IsDlgButtonChecked(IDC_FRIDAY_CHECK)) {
		wlItem.arDayIDs.Add(6);
	}

	if (IsDlgButtonChecked(IDC_SATURDAY_CHECK)) {
		wlItem.arDayIDs.Add(7);
	}
	
}

void CWaitingListResourceDlg::SetResourceListVals(const WaitListLineItem &wlItem)
{
	// Sets the resource checkboxes to match pItem.arResourceIDs.
	
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	long nNumIDs = wlItem.arResourceIDs.GetSize();
	for (long i = 0; i < nNumIDs; i++) {
		pRow = m_pResources->FindByColumn(ewlrcID, (long)wlItem.arResourceIDs[i], NULL, FALSE);
		if (pRow != NULL) {
			pRow->PutValue(ewlrcInUse, COleVariant((short)TRUE, VT_BOOL));
		}
	}
}

void CWaitingListResourceDlg::GetResourceListVals(WaitListLineItem &wlItem)
{
	// Updates pItem.arResourceIDs to match the resource checkboxes.
	
	// First remove any existing data.
	wlItem.arResourceIDs.RemoveAll();
	wlItem.arResourceNames.RemoveAll();

	// Check to see if the 'All Resources' radio button is checked.
	if (IsDlgButtonChecked(IDC_RADIO_ALL_RESOURCES)) {
		wlItem.bAllResources = true;
	} else {
		wlItem.bAllResources = false;
	}

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResources->GetFirstRow();
	while (pRow != NULL) {
		if (VarBool(pRow->GetValue(ewlrcInUse), FALSE)) {
			wlItem.arResourceIDs.Add(VarLong(pRow->GetValue(ewlrcID), 0));
			wlItem.arResourceNames.Add(VarString(pRow->GetValue(ewlrcItem), ""));
		}
		pRow = pRow->GetNextRow();
	}
}

void CWaitingListResourceDlg::SetLineItemData(const WaitListLineItem &wlItem)
{
	m_LineItem = wlItem;
}

void CWaitingListResourceDlg::GetLineItemData(WaitListLineItem &wlItem)
{
	// Returns a struct representing the current state of the controls.
	wlItem = m_LineItem;
	wlItem.bModified = m_bDataChanged;
}

void CWaitingListResourceDlg::RefreshLineItemData(WaitListLineItem &wlItem)
{
	// Updates m_pLineItem to match the current values for the controls.
	
	wlItem.dtStartDate = m_dtcStartDate.GetValue();
	wlItem.dtEndDate = m_dtcEndDate.GetValue();
	
	COleDateTime dt;
	dt = m_nxtStartTime->GetDateTime();
	wlItem.dtStartTime.SetTime(dt.GetHour(), dt.GetMinute(), 0);
	dt = m_nxtEndTime->GetDateTime();
	wlItem.dtEndTime.SetTime(dt.GetHour(), dt.GetMinute(), 0);
	
	GetDayCheckboxVals(wlItem);
	GetResourceListVals(wlItem);
}

void CWaitingListResourceDlg::RefreshLineItemText(const WaitListLineItem &wlItem)
{
	// Updates the result area to match the state of the controls.
	SetDlgItemText(IDC_RESULTS_LABEL, CWaitingListUtils::FormatLineItem(wlItem));
}

bool CWaitingListResourceDlg::ValidateData(const WaitListLineItem &wlItem)
{
	// Returns false if dates or times are improperly entered.
	if (wlItem.dtStartDate > wlItem.dtEndDate) {
		MessageBox("The start date is set to a value later than the ending date.");
		return false;
	}
	if (wlItem.dtStartTime > wlItem.dtEndTime) {
		MessageBox("The start time is set to a value later than the ending time.");
		return false;
	}

	// Check to make sure that either 'All Resources' or an item from the resource
	//  list is selected.
	if (!IsDlgButtonChecked(IDC_RADIO_ALL_RESOURCES)) {
		bool bResourceChecked = false;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResources->GetFirstRow();
		while (pRow != NULL) {
			if (VarBool(pRow->GetValue(ewlrcInUse), FALSE)) {
				bResourceChecked = true;
				break;
			}
			pRow = pRow->GetNextRow();
		}
		if (!bResourceChecked) {
			MessageBox("You must select 'All Resources' or check at least one item in the resource list.");
			return false;
		}
	}

	return true;
}
