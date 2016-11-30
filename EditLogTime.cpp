// EditLogTime.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "EditLogTime.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXTIMELib;

enum LocationColumns {
	lcID = 0,
	lcName = 1,
};

/////////////////////////////////////////////////////////////////////////////
// EditLogTime dialog


CEditLogTime::CEditLogTime(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditLogTime::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditLogTime)
		//Default dates to current date/time
		m_dtStart = COleDateTime::GetCurrentTime();
		m_dtEnd = COleDateTime::GetCurrentTime();
		//set to -1 as default
		m_nLocationID = -1;
		m_strLocationName = "";
		m_bInitialEndTimeValid = false;
	//}}AFX_DATA_INIT
}


void CEditLogTime::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditLogTime)
	DDX_Control(pDX, IDC_LOG_STARTDATE, m_dtpStartDate);
	DDX_Control(pDX, IDC_LOG_ENDDATE, m_dtpEndDate);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditLogTime, CNxDialog)
	//{{AFX_MSG_MAP(CEditLogTime)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EditLogTime message handlers


BOOL CEditLogTime::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try{
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		SetWindowText(m_strTitle);
		m_nxtStartTime = BindNxTimeCtrl(this, IDC_LOG_STARTTIME);
		m_nxtEndTime = BindNxTimeCtrl(this, IDC_LOG_ENDTIME);

		//(e.lally 2006-11-07) PLID 14772 - Add new log entries manually
		//our date member variables should be set by this point (or are accepting the defaults)
		//load the values into the DPT/NxTime controls
		LoadStart(m_dtStart);
		LoadEnd(m_dtEnd);

		// (j.gruber 2008-06-25 10:07) - PLID 26136 - added location
		m_pLocationList = BindNxDataList2Ctrl(IDC_TIME_LOG_LOCATION);

		return TRUE;

	}NxCatchAll("Error Initializing the Edit Log screen");
	
	
	return FALSE;
	             
}

int CEditLogTime::DoModal(bool bIsNew) 
{
	//TES 9/1/2010 - PLID 39233 - This now takes bIsNew rather than strTitle
	m_bIsNew = bIsNew;
	m_strTitle = bIsNew?"Add Log Entry":"Edit Log Entry";
	return CDialog::DoModal();
}

void CEditLogTime::OnOK() 
{
	try{
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//There's not much need to update our member variables until the user is ready to save.
		//We will do it here.
		COleDateTime dtDate, dtTime;
		dtDate = VarDateTime(m_dtpStartDate.GetValue());
		dtTime = m_nxtStartTime->GetDateTime();
		m_dtStart.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), 
			dtTime.GetHour(), dtTime.GetMinute(), 0);

		dtDate = VarDateTime(m_dtpEndDate.GetValue());
		//TES 9/8/2009 - PLID 26888 - The end time may be blank
		if(m_nxtEndTime->GetStatus() == 1) {
			dtTime = m_nxtEndTime->GetDateTime();
			m_dtEnd.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), 
				dtTime.GetHour(), dtTime.GetMinute(), 0);
		}
		else {
			m_dtEnd.SetStatus(COleDateTime::invalid);
		}

		if(!IsValid()){
			return;
		}
	}NxCatchAll("Error saving Log Time");
	CDialog::OnOK();
}


BOOL CEditLogTime::IsValid()
{
	if(m_dtStart.GetStatus() == COleDateTime::invalid){
		MessageBox("Please enter a valid Clock In time.");
		return FALSE;
	}
	//TES 9/8/2009 - PLID 26888 - It's ok for the end time to be invalid, but only if it started out that way.
	//TES 9/1/2010 - PLID 39233 - We'll also allow invalid end times on new entries.
	else if(!m_bIsNew && m_bInitialEndTimeValid && m_dtEnd.GetStatus() == COleDateTime::invalid){
		MessageBox("Please enter a valid Clock Out time.");
		return FALSE;
	}
	else if(m_dtEnd.GetStatus() == COleDateTime::valid && m_dtStart > m_dtEnd){
		MessageBox("You may not have a Clock In time after the Clock Out time.");
		return FALSE;
	}

	//At the very least we will want to warn the user that they are about to enter
	//date(s) that are in the future, we can prevent it later if need be.
	_RecordsetPtr rs = CreateRecordset("SELECT GetDate() AS CurDateTime");
	COleDateTime dtCurDateTime = AdoFldDateTime(rs, "CurDateTime");
	if(m_dtStart > dtCurDateTime || (m_dtEnd.GetStatus() == COleDateTime::valid && m_dtEnd > dtCurDateTime)){
		if(IDNO == MessageBox("At least one time is in the future. Are you sure you want to continue saving?", NULL, MB_YESNO))
			return FALSE;
	}


	return TRUE;
}

COleDateTime CEditLogTime::GetStart()
{
	
	return m_dtStart;

}

COleDateTime CEditLogTime::GetEnd()
{
	
	return m_dtEnd;
}

// (j.gruber 2008-06-25 12:08) - PLID 26136 - added location
void CEditLogTime::SetLocationID(long nLocationID)
{
	m_nLocationID = nLocationID;
}

// (j.gruber 2008-06-25 12:08) - PLID 26136 - added location
long CEditLogTime::GetLocationID()
{
	return m_nLocationID;
}

// (j.gruber 2008-06-25 12:08) - PLID 26136 - added location
CString CEditLogTime::GetLocationName()
{
	return m_strLocationName;
}


void CEditLogTime::SetStart(COleDateTime dtStart)
{
	m_dtStart = dtStart;

}

void CEditLogTime::SetEnd(COleDateTime dtEnd)
{
	m_dtEnd = dtEnd;
}

void CEditLogTime::LoadStart(COleDateTime dtStart)
{
	_variant_t varStart = _variant_t(dtStart);
	m_dtpStartDate.SetValue(varStart);
	m_nxtStartTime->SetDateTime(dtStart);

}

void CEditLogTime::LoadEnd(COleDateTime dtEnd)
{
	if(dtEnd.GetStatus() == COleDateTime::valid) {
		m_dtpEndDate.SetValue(_variant_t(dtEnd));
		m_nxtEndTime->SetDateTime(dtEnd);
		//TES 9/8/2009 - PLID 26888 - Remember whether we were initially given a valid end time
		m_bInitialEndTimeValid = true;
	}
	else {
		//TES 9/8/2009 - PLID 26888 - Remember whether we were initially given a valid end time
		m_bInitialEndTimeValid = false;
	}
}

BEGIN_EVENTSINK_MAP(CEditLogTime, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditLogTime)
	ON_EVENT(CEditLogTime, IDC_TIME_LOG_LOCATION, 18 /* RequeryFinished */, OnRequeryFinishedTimeLogLocation, VTS_I2)
	ON_EVENT(CEditLogTime, IDC_TIME_LOG_LOCATION, 16 /* SelChosen */, OnSelChosenTimeLogLocation, VTS_DISPATCH)
	ON_EVENT(CEditLogTime, IDC_TIME_LOG_LOCATION, 1 /* SelChanging */, OnSelChangingTimeLogLocation, VTS_DISPATCH VTS_PDISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditLogTime::OnRequeryFinishedTimeLogLocation(short nFlags) 
{
	try {

		//add the "none"
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pLocationList->GetNewRow();
		if (pRow) {
			pRow->PutValue(lcID, (long)-1);
			pRow->PutValue(lcName, _variant_t("<None>"));

			m_pLocationList->AddRowSorted(pRow, NULL);
		}

		
		pRow = m_pLocationList->SetSelByColumn(lcID, m_nLocationID);
		
		if (pRow == NULL) {
			//it must be inactive or unmanaged
			_RecordsetPtr rsName = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT} AND (Active = 0 or Managed = 0)", m_nLocationID);
			if (!rsName->eof) {
				CString strName = AdoFldString(rsName, "Name", "");
				m_pLocationList->PutComboBoxText(_bstr_t(strName));
				m_nLocationID = -2;
				m_strLocationName = strName;
			}
		}
		else {
			pRow = m_pLocationList->CurSel;
			if (pRow) {
				//this should never not be a string
				m_strLocationName = VarString(pRow->GetValue(lcName));				
			}
		}
		

	}NxCatchAll("Error in CEditLogTime::OnRequeryFinishedTimeLogLocation");
	
}

void CEditLogTime::OnSelChosenTimeLogLocation(LPDISPATCH lpRow) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		
		if (pRow) {
			//these should always return values
			m_nLocationID = VarLong(pRow->GetValue(lcID));
			m_strLocationName = VarString(pRow->GetValue(lcName));
		}

	}NxCatchAll("Error in CEditLogTime::OnSelChosenTimeLogLocation");
	
}


void CEditLogTime::OnSelChangingTimeLogLocation(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {

		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

	}NxCatchAll("Error in CEditLogTime::OnSelChangingTimeLogLocation");
	
}
