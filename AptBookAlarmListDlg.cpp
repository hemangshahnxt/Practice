// AptBookAlarmListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AptBookAlarmListDlg.h"
#include "AptBookAlarmDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CAptBookAlarmListDlg dialog


CAptBookAlarmListDlg::CAptBookAlarmListDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAptBookAlarmListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAptBookAlarmListDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAptBookAlarmListDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAptBookAlarmListDlg)
	DDX_Control(pDX, IDC_ADD_ALARM_BTN, m_btnAddAlarm);
	DDX_Control(pDX, IDC_EDIT_ALARM_BTN, m_btnEditAlarm);
	DDX_Control(pDX, IDC_REMOVE_ALARM_BTN, m_btnRemoveAlarm);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAptBookAlarmListDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAptBookAlarmListDlg)
	ON_BN_CLICKED(IDC_ADD_ALARM_BTN, OnAddAlarmBtn)
	ON_BN_CLICKED(IDC_EDIT_ALARM_BTN, OnEditAlarmBtn)
	ON_BN_CLICKED(IDC_REMOVE_ALARM_BTN, OnRemoveAlarmBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAptBookAlarmListDlg message handlers

BOOL CAptBookAlarmListDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_AlarmList = BindNxDataListCtrl(this,IDC_APT_BOOKING_ALARM_LIST,GetRemoteData(),true);

	// (z.manning, 04/30/2008) - PLID 29845 - Set button styles
	m_btnAddAlarm.AutoSet(NXB_NEW);
	m_btnEditAlarm.AutoSet(NXB_MODIFY);
	m_btnRemoveAlarm.AutoSet(NXB_DELETE);
	m_btnClose.AutoSet(NXB_CLOSE);

	RefreshButtons();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAptBookAlarmListDlg::OnAddAlarmBtn() 
{
	try {

		CAptBookAlarmDlg dlg(this);
		dlg.m_bIsNew = TRUE;
		dlg.m_ID = -1;
		if(IDOK == dlg.DoModal()) {
			m_AlarmList->Requery();
		}
		RefreshButtons();

	}NxCatchAll("Error adding new appointment booking alarm.");
}

void CAptBookAlarmListDlg::OnEditAlarmBtn() 
{
	try {

		if(m_AlarmList->GetCurSel() == -1) {
			AfxMessageBox("Please select an alarm to edit.");
			return;
		}

		CAptBookAlarmDlg dlg(this);
		dlg.m_bIsNew = FALSE;
		dlg.m_ID = m_AlarmList->GetValue(m_AlarmList->GetCurSel(),0).lVal;
		if(IDOK == dlg.DoModal()) {
			m_AlarmList->Requery();
		}
		RefreshButtons();

	}NxCatchAll("Error editing appointment booking alarm.");	
}

void CAptBookAlarmListDlg::OnRemoveAlarmBtn() 
{
	try {

		if(m_AlarmList->GetCurSel() == -1) {
			AfxMessageBox("Please select an alarm to delete.");
			return;
		}

		if(IDYES == MessageBox("Are you sure you wish to delete this alarm?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
			long nAlarmID = m_AlarmList->GetValue(m_AlarmList->GetCurSel(),0).lVal;
			ExecuteSql("DELETE FROM AptBookAlarmDetailsT WHERE AptBookAlarmID = %li",nAlarmID);
			ExecuteSql("DELETE FROM AptBookAlarmT WHERE ID = %li",nAlarmID);
			m_AlarmList->RemoveRow(m_AlarmList->GetCurSel());
			RefreshButtons();
		}

	}NxCatchAll("Error deleting appointment booking alarm.");	
}

BEGIN_EVENTSINK_MAP(CAptBookAlarmListDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAptBookAlarmListDlg)
	ON_EVENT(CAptBookAlarmListDlg, IDC_APT_BOOKING_ALARM_LIST, 3 /* DblClickCell */, OnDblClickCellAptBookingAlarmList, VTS_I4 VTS_I2)
	ON_EVENT(CAptBookAlarmListDlg, IDC_APT_BOOKING_ALARM_LIST, 2 /* SelChanged */, OnSelChangedAptBookingAlarmList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAptBookAlarmListDlg::OnDblClickCellAptBookingAlarmList(long nRowIndex, short nColIndex) 
{
	OnEditAlarmBtn();
}

void CAptBookAlarmListDlg::RefreshButtons()
{
	if(m_AlarmList->GetCurSel() == sriNoRow){
		GetDlgItem(IDC_EDIT_ALARM_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_REMOVE_ALARM_BTN)->EnableWindow(FALSE);
	}
	else{
		GetDlgItem(IDC_EDIT_ALARM_BTN)->EnableWindow(TRUE);
		GetDlgItem(IDC_REMOVE_ALARM_BTN)->EnableWindow(TRUE);
	}
}

void CAptBookAlarmListDlg::OnSelChangedAptBookingAlarmList(long nNewSel) 
{
	RefreshButtons();
}
