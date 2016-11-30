// UpdatePastPendingApptsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "UpdatePastPendingApptsDlg.h"
#include "AuditTrail.h"
#include "globaldatautils.h"
#include "PhaseTracking.h"
#include "DateTimeUtils.h"
#include "GlobalSchedUtils.h"
#include "InternationalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CUpdatePastPendingApptsDlg dialog


CUpdatePastPendingApptsDlg::CUpdatePastPendingApptsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUpdatePastPendingApptsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpdatePastPendingApptsDlg)
	m_dtFrom = COleDateTime::GetCurrentTime();
	m_dtTo = COleDateTime::GetCurrentTime();
	//}}AFX_DATA_INIT
}

// (c.haag 2010-01-12 15:44) - PLID 31157 - We now standardize auditing of mass appointment updates
/* static */ void CUpdatePastPendingApptsDlg::AuditMassUpdate(const CString& strFrom, const CString& strTo,
														 const COleDateTime& dtFrom, const COleDateTime& dtTo)
{
	long AuditID = -1;
	AuditID = BeginNewAuditEvent();
	if(AuditID!=-1) {
		CString strOld;
		strOld.Format("%s (%s - %s)", strFrom, FormatDateTimeForInterface(dtFrom,NULL,dtoDate), FormatDateTimeForInterface(dtTo,NULL,dtoDate));
		CString strNew = strTo;
		AuditEvent(-1, "", AuditID,aeiApptUpdate,-1,strOld,strNew,aepHigh);
	}
}


void CUpdatePastPendingApptsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpdatePastPendingApptsDlg)
	DDX_DateTimeCtrl(pDX, IDC_START_UPDATE, m_dtFrom);
	DDX_DateTimeCtrl(pDX, IDC_END_UPDATE, m_dtTo);
	DDX_Control(pDX, IDC_APT_COUNT, m_nxstaticAptCount);
	DDX_Control(pDX, IDC_BTN_PENDING_TO_NOSHOW, m_btnPendingToNoShow);
	DDX_Control(pDX, IDC_BTN_PENDING_TO_OUT, m_btnPendingToOut);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUpdatePastPendingApptsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CUpdatePastPendingApptsDlg)
	ON_BN_CLICKED(IDC_BTN_PENDING_TO_NOSHOW, OnBtnPendingToNoshow)
	ON_BN_CLICKED(IDC_BTN_PENDING_TO_OUT, OnBtnPendingToOut)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_START_UPDATE, OnDatetimechangeFromDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_END_UPDATE, OnDatetimechangeToDate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

using namespace ADODB;
/////////////////////////////////////////////////////////////////////////////
// CUpdatePastPendingApptsDlg message handlers

void CUpdatePastPendingApptsDlg::OnBtnPendingToNoshow() 
{
	//DRT 8/06/02 - !!!???  I put the comment below this one for a reason.  There is NO reason we should ever do something like this without a confirmation box
	//				to make sure they can stop if they accidentally hit it.  If I come back to this code in a year and find it commented out again...
	//DRT 9/17/01 - How was this ever left out!!
	if(MessageBox("Running this option will change the status of all selected appointments.  Are you absolutely sure you wish to do this?", "Practice", MB_YESNO) == IDNO)
		return;
	
	//DRT 9/17/01 - Change to only update today, not forever
	try {
		UpdateData();
		m_dtTo.SetDateTime(m_dtTo.GetYear(), m_dtTo.GetMonth(), m_dtTo.GetDay(), 0,0,0);
		if(m_dtTo > COleDateTime::GetCurrentTime()) {
			int nReturn = MessageBox("Some of these appointments are in the future.  Do you still wish to continue?", NULL, MB_YESNO);
			if(nReturn != IDYES) {
				return;
			}
		}

		// (c.haag 2009-08-06 11:37) - PLID 25943 - Ask the user about what to do with superbills for these appointments
		BOOL bVoidSuperbills = FALSE;
		// (z.manning 2011-04-01 15:24) - PLID 42973 - Converted where clause to a SQL fragment
		switch (NeedsToVoidNoShowSuperbills(CSqlFragment("ShowState = 0 AND AppointmentsT.Date >= {OLEDATETIME} AND AppointmentsT.Date <= {OLEDATETIME}", m_dtFrom, m_dtTo)))
		{
			case IDYES: // Yes, there are superbills to void
				bVoidSuperbills = TRUE;
				break;
			case IDNO: // Do not void superbills
				bVoidSuperbills = FALSE;
				break;
			case IDCANCEL: // User changed their mind
				return;
		}

		if (bVoidSuperbills) {
			ExecuteSql("UPDATE PrintedSuperbillsT SET Void = 1, VoidDate = GetDate(), VoidUser = '%s' "
				"WHERE PrintedSuperbillsT.ReservationID IN (SELECT ID FROM AppointmentsT WHERE ShowState = 0 AND AppointmentsT.Date >= '%s' AND AppointmentsT.Date <= '%s') AND Void = 0", _Q(GetCurrentUserName()), 
				FormatDateTimeForSql(m_dtFrom, dtoDate), FormatDateTimeForSql(m_dtTo, dtoDate)
				);
		}
		ExecuteSql("UPDATE AppointmentsT SET ShowState = 2 WHERE (ShowState = 1 OR ShowState = 4) AND AppointmentsT.Date >= '%s' AND AppointmentsT.Date <= '%s'", FormatDateTimeForSql(m_dtFrom, dtoDate), FormatDateTimeForSql(m_dtTo, dtoDate));
		ExecuteSql("UPDATE AppointmentsT SET ShowState = 3, NoShowDate = GetDate() WHERE ShowState = 0 AND AppointmentsT.Date >= '%s' AND AppointmentsT.Date <= '%s'", FormatDateTimeForSql(m_dtFrom, dtoDate), FormatDateTimeForSql(m_dtTo, dtoDate));

		if (GetMainFrame()->GetActiveView())
			GetMainFrame()->GetActiveView()->UpdateView();

		//DRT 10/22/01 - put in some auditing stuff
		// (c.haag 2010-01-12 15:46) - PLID 31157 - Now standardized
		AuditMassUpdate("Pending","NoShow", m_dtFrom, m_dtTo);
		AuditMassUpdate("In","Out", m_dtFrom, m_dtTo);

		// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
		GetMainFrame()->HandleRecallChanged();

	}NxCatchAll("Error in OnUpdatePastAppts()");
	OnOK();
}

//Used in the below function to store the results of a recordset.
struct PendingAppointment {
	long nApptID;
	long nPatientID;
	COleDateTime dtDate;
};
void CUpdatePastPendingApptsDlg::OnBtnPendingToOut() 
{
	/* 0 = Pending 1 = In 2 = Out 3 = No Show 4 = Received*/
	try {
		//DRT 8/6/02 - read the dire warnings in the function above
		if(MessageBox("Running this option will change the status of all selected appointments.  Are you absolutely sure you wish to do this?", "Practice", MB_YESNO) == IDNO)
			return;

		UpdateData();
		m_dtTo.SetDateTime(m_dtTo.GetYear(), m_dtTo.GetMonth(), m_dtTo.GetDay(), 0,0,0);
		if(m_dtTo > COleDateTime::GetCurrentTime()) {
			int nReturn = MessageBox("Some of these appointments are in the future.  Do you still wish to continue?", NULL, MB_YESNO);
			if(nReturn != IDYES) {
				return;
			}
		}
		CWaitCursor cuWait;
		ExecuteSql("UPDATE AppointmentsT SET ShowState = 2 WHERE (ShowState = 1 OR ShowState = 4) AND AppointmentsT.Date >= '%s' AND AppointmentsT.Date <= '%s'", FormatDateTimeForSql(m_dtFrom, dtoDate), FormatDateTimeForSql(m_dtTo, dtoDate));
		//We have to loop through the ones going from pending to out, and trigger tracking on them.
		_RecordsetPtr rsPending = CreateRecordset("SELECT ID, PatientID, Date FROM AppointmentsT WHERE ShowState = 0 AND AppointmentsT.Date >= '%s' AND AppointmentsT.Date <= '%s'", FormatDateTimeForSql(m_dtFrom, dtoDate), FormatDateTimeForSql(m_dtTo, dtoDate));
		CArray<PendingAppointment,PendingAppointment> arPending;
		while(!rsPending->eof) {
			PendingAppointment pa;
			pa.nApptID = AdoFldLong(rsPending, "ID");
			pa.nPatientID = AdoFldLong(rsPending, "PatientID");
			pa.dtDate = AdoFldDateTime(rsPending, "Date");
			arPending.Add(pa);
			rsPending->MoveNext();
		}
		rsPending->Close();
		for(int i = 0; i < arPending.GetSize(); i++) {
			PendingAppointment pa = arPending.GetAt(i);
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_ActualAppointment, pa.nPatientID, pa.dtDate, pa.nApptID, false);
			ExecuteSql("UPDATE AppointmentsT SET ShowState = 2 WHERE ID = %li", pa.nApptID);
		}


		if (GetMainFrame()->GetActiveView())
			GetMainFrame()->GetActiveView()->UpdateView();

		//DRT 10/22/01 - put in some auditing stuff
		// (c.haag 2010-01-12 15:46) - PLID 31157 - Now standardized
		AuditMassUpdate("Pending","Out", m_dtFrom, m_dtTo);
		AuditMassUpdate("In","Out", m_dtFrom, m_dtTo);

	}NxCatchAll("Error in OnUpdatePastAppts()");
	OnOK();
}

void CUpdatePastPendingApptsDlg::OnDatetimechangeFromDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateCaption();

	*pResult = 0;
}

void CUpdatePastPendingApptsDlg::OnDatetimechangeToDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateCaption();

	*pResult = 0;
}

void CUpdatePastPendingApptsDlg::UpdateCaption()
{
	try {
		UpdateData();
		_RecordsetPtr rsCountPending = CreateRecordset("SELECT Count(ID) AS PendingCount FROM AppointmentsT WHERE Date >= '%s' AND Date <= '%s' AND ShowState = 0", FormatDateTimeForSql(m_dtFrom, dtoDate), FormatDateTimeForSql(m_dtTo, dtoDate));
		long nPendingCount = AdoFldLong(rsCountPending, "PendingCount");
		_RecordsetPtr rsCountIn = CreateRecordset("SELECT Count(ID) AS InCount FROM AppointmentsT WHERE Date >= '%s' AND Date <= '%s' AND ShowState = 1", FormatDateTimeForSql(m_dtFrom, dtoDate), FormatDateTimeForSql(m_dtTo, dtoDate));
		long nInCount = AdoFldLong(rsCountIn, "InCount");
		CString strCaption;
		strCaption.Format("%li Pending, %li In", nPendingCount, nInCount);
		SetDlgItemText(IDC_APT_COUNT, strCaption);
	}NxCatchAll("Error in UpdateCaption");
}

BOOL CUpdatePastPendingApptsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnPendingToNoShow.AutoSet(NXB_MODIFY);
	m_btnPendingToOut.AutoSet(NXB_MODIFY);
	// (z.manning 2008-08-13 17:48) - PLID 29946 - This should be close
	m_btnCancel.AutoSet(NXB_CLOSE);
	m_btnCancel.SetWindowText("Close");
	
	UpdateCaption();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
