// AnesthesiaTimePromptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnesthesiaTimePromptDlg.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAnesthesiaTimePromptDlg dialog

using namespace ADODB;

CAnesthesiaTimePromptDlg::CAnesthesiaTimePromptDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAnesthesiaTimePromptDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnesthesiaTimePromptDlg)
		m_nServiceID = -1;
		m_nMinutes = 0;
		m_eTimePromptType = tptAnesthesia;
		m_strStartTime = "";
		m_strEndTime = "";
	//}}AFX_DATA_INIT
}


void CAnesthesiaTimePromptDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnesthesiaTimePromptDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_START_LABEL, m_nxstaticStartLabel);
	DDX_Control(pDX, IDC_END_LABEL, m_nxstaticEndLabel);
	DDX_Control(pDX, IDC_CODE_LABEL, m_nxstaticCodeLabel);
	DDX_Control(pDX, IDC_TOP_LABEL, m_nxstaticTopLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnesthesiaTimePromptDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAnesthesiaTimePromptDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnesthesiaTimePromptDlg message handlers

BOOL CAnesthesiaTimePromptDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (c.haag 2008-05-01 15:07) - PLID 29871 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_nxtStart = BindNxTimeCtrl(this, IDC_ANESTH_START_TIME);
		m_nxtEnd = BindNxTimeCtrl(this, IDC_ANESTH_END_TIME);

		// (j.jones 2010-11-22 17:21) - PLID 39602 - change the labels based on the enum,
		// they default to anesthesia naming
		if(m_eTimePromptType == tptFacilityFee) {
			//change the labels to reflect the facility fee
			SetWindowText("Enter Facility Fee Times");
			SetDlgItemText(IDC_TOP_LABEL, "Enter the Facility Fee Start and End time for the code:");
			SetDlgItemText(IDC_START_LABEL, "Facility Fee Start Time");
			SetDlgItemText(IDC_END_LABEL, "Facility Fee End Time");
		}
		else if(m_eTimePromptType == tptAssistingCode) {
			//change the labels to reflect the assisting code
			SetWindowText("Enter Assisting Times");
			SetDlgItemText(IDC_TOP_LABEL, "Enter the Assisting Start and End time for the code:");
			SetDlgItemText(IDC_START_LABEL, "Assisting Start Time");
			SetDlgItemText(IDC_END_LABEL, "Assisting End Time");
		}

		_RecordsetPtr rs = CreateParamRecordset("SELECT Code + ' - ' + Name AS Description FROM ServiceT "
			"INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE CPTCodeT.ID = {INT}",m_nServiceID);
		if(!rs->eof) {
			CString strDesc = AdoFldString(rs, "Description","");
			SetDlgItemText(IDC_CODE_LABEL,strDesc);
		}
		rs->Close();

		// (j.jones 2010-01-18 15:16) - PLID 36913 - this screen can now accept default times,
		// so show them if we have them (we could have one and not the other)
		if(m_strStartTime != "") {
			COleDateTime dtStart;
			if(dtStart.ParseDateTime(m_strStartTime)) {
				m_nxtStart->SetDateTime(dtStart);
			}
		}
		if(m_strEndTime != "") {
			COleDateTime dtEnd;
			if(dtEnd.ParseDateTime(m_strEndTime)) {
				m_nxtEnd->SetDateTime(dtEnd);
			}
		}

	}NxCatchAll("Error loading dialog.");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAnesthesiaTimePromptDlg::OnOK() 
{
	try {

		if(m_nxtStart->GetStatus() == 2 || m_nxtEnd->GetStatus() == 2) {
			MessageBox("You have entered an invalid time");
			return;
		}

		COleDateTime dtStart, dtEnd;

		if(m_nxtStart->GetStatus() == 1) {
			dtStart = m_nxtStart->GetDateTime();
		}
		else {
			dtStart.SetStatus(COleDateTime::invalid);
		}

		if(m_nxtEnd->GetStatus() == 1) {
			dtEnd = m_nxtEnd->GetDateTime();
		}
		else {
			dtEnd.SetStatus(COleDateTime::invalid);
		}

		if(dtStart.GetStatus() == COleDateTime::invalid || dtEnd.GetStatus() == COleDateTime::invalid) {
			MessageBox("You have entered an invalid time");
			return;
		}

		_RecordsetPtr rs = CreateRecordset("SELECT "
			"CASE WHEN Convert(datetime,'%s') >= Convert(datetime,'%s') "
			"THEN DATEDIFF(minute,DATEADD(day,-1,'%s'),'%s') "
			"ELSE DATEDIFF(minute,'%s','%s') END AS Minutes",
			FormatDateTimeForSql(dtStart,dtoTime),FormatDateTimeForSql(dtEnd,dtoTime),
			FormatDateTimeForSql(dtStart,dtoTime),FormatDateTimeForSql(dtEnd,dtoTime),
			FormatDateTimeForSql(dtStart,dtoTime),FormatDateTimeForSql(dtEnd,dtoTime));
		m_nMinutes = AdoFldLong(rs, "Minutes",0);

		if(m_nMinutes < 0) {
			MessageBox("You must enter a valid time range (greater than zero minutes).");
			return;
		}

		m_strStartTime = dtStart.Format("%H:%M:%S");
		m_strEndTime = dtEnd.Format("%H:%M:%S");
		
		CDialog::OnOK();

	}NxCatchAll("Error calculating anesthesia minutes.");
}
