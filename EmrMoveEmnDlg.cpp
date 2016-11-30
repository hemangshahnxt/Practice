// EmrMoveEmnDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EmrMoveEmnDlg.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CEmrMoveEmnDlg dialog

// (j.jones 2009-09-24 15:04) - PLID 31672 - added ability to move between patients,
// as an alternative to moving between EMRs
// (j.jones 2009-09-28 08:39) - PLID 31672 - removed this feature
CEmrMoveEmnDlg::CEmrMoveEmnDlg(long nEmnID, /*BOOL bMoveToPatient,*/ CWnd* pParent)
	: CNxDialog(CEmrMoveEmnDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEmrMoveEmnDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	//m_bMoveToPatient = bMoveToPatient;
	m_nEmnID = nEmnID;
	m_nPatientID = -1;
	m_dtEMNDate.SetStatus(COleDateTime::invalid);
}


void CEmrMoveEmnDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmrMoveEmnDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_MOVE_EMN_INSTRUCTIONS, m_nxstaticMoveEmnInstructions);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmrMoveEmnDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEmrMoveEmnDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEmrMoveEmnDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEmrMoveEmnDlg)
	ON_EVENT(CEmrMoveEmnDlg, IDC_EMR_LIST, 3 /* DblClickCell */, OnDblClickCellEmrList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEmrMoveEmnDlg, IDC_EMR_LIST, 18 /* RequeryFinished */, OnRequeryFinishedEmrList, VTS_I2)
	//ON_EVENT(CEmrMoveEmnDlg, IDC_MOVE_TO_PATIENT_LIST, 3, OnDblClickCellMoveToPatientList, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrMoveEmnDlg message handlers

BOOL CEmrMoveEmnDlg::OnInitDialog()
{	
	try {

		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-28 12:12) - PLID 29806 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_pdlEmrList = BindNxDataList2Ctrl(IDC_EMR_LIST, false);
		// (j.jones 2009-09-24 15:10) - PLID 31672 - added a patient list
		// (j.jones 2009-09-28 08:39) - PLID 31672 - removed this feature
		//m_pdlPatientList = BindNxDataList2Ctrl(IDC_MOVE_TO_PATIENT_LIST, false);

		// (j.jones 2009-09-24 15:00) - PLID 31672 - added date, patient name and ID
		ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT EMRMasterT.Date, "
			"EmrMasterT.Description AS EmnDescription, "
			"EmrGroupsT.Description AS EmrDescription, "
			"Last + ', ' + First + ' ' + Middle AS PatientName, "
			"EmrMasterT.PatientID "
			"FROM EmrMasterT "
			"LEFT JOIN EmrGroupsT ON EmrMasterT.EmrGroupID = EmrGroupsT.ID "
			"INNER JOIN PersonT ON EmrMasterT.PatientID = PersonT.ID "
			"WHERE EmrMasterT.ID = {INT} ", m_nEmnID);
		if(prs->eof) {
			ThrowNxException("CEmrMoveEmnDlg called with an invalid EMN ID! (%li)", m_nEmnID);
		}

		m_strEmnDescription = AdoFldString(prs, "EmnDescription", "");
		m_strOldEmrDescription = AdoFldString(prs, "EmrDescription", "");
		m_strPatientName = AdoFldString(prs, "PatientName", "");
		m_strPatientName.TrimRight();
		m_nPatientID = AdoFldLong(prs, "PatientID");
		m_dtEMNDate = AdoFldDateTime(prs, "Date");

		// (j.jones 2009-09-24 15:07) - PLID 31672 - this dialog can now be used to move
		// between different patients, or between EMRs for the same patient
		// (j.jones 2009-09-28 08:39) - PLID 31672 - removed this feature
		/*
		if(m_bMoveToPatient) {

			GetDlgItem(IDC_EMR_LIST)->ShowWindow(SW_HIDE);

			RequeryPatientList();

			SetWindowText("Select a Patient");
			SetDlgItemText(IDC_MOVE_EMN_INSTRUCTIONS, FormatString("Please select the patient that you would like "
				"to move the \"%s\" EMN to:", m_strEmnDescription));
		}
		else */{

			GetDlgItem(IDC_MOVE_TO_PATIENT_LIST)->ShowWindow(SW_HIDE);

			RequeryEmrList();

			SetWindowText("Select an EMR");
			SetDlgItemText(IDC_MOVE_EMN_INSTRUCTIONS, FormatString("Please select the EMR that you would like "
				"to move the \"%s\" EMN to:", m_strEmnDescription));
		}

	} NxCatchAll("Error in CEmrMoveEmnDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEmrMoveEmnDlg::RequeryEmrList()
{
	CString strWhere = FormatString("PatientID = %li "
		"AND EmrGroupsT.ID <> (SELECT EmrGroupID FROM EmrMasterT WHERE EmrMasterT.ID = %li) "
		"AND Deleted = 0 AND (PicT.IsCommitted IS NULL OR PicT.IsCommitted = 1) ", m_nPatientID, m_nEmnID);

	m_pdlEmrList->WhereClause = _bstr_t(strWhere);
	m_pdlEmrList->Requery();
}

// (j.jones 2009-09-24 15:08) - PLID 31672 - displays all active patients but this one
// (j.jones 2009-09-28 08:39) - PLID 31672 - removed this feature
/*
void CEmrMoveEmnDlg::RequeryPatientList()
{
	CString strWhere = FormatString("PersonT.ID <> %li "
		"AND PersonT.ID > 0 AND PatientsT.CurrentStatus <> 4 AND PersonT.Archived = 0", m_nPatientID);

	m_pdlPatientList->WhereClause = _bstr_t(strWhere);
	m_pdlPatientList->Requery();
}
*/

void CEmrMoveEmnDlg::OnCancel()
{
	CNxDialog::OnCancel();
}

void CEmrMoveEmnDlg::OnOK()
{
	try {
		//moving between EMRs

		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pdlEmrList->GetCurSel();
		if(pCurRow == NULL) {
			MessageBox("You must select a row before continuing.");
			return;
		}

		long nNewEmrID = VarLong(pCurRow->GetValue(elcID), -1);
		CString strNewEmrDescription = VarString(pCurRow->GetValue(elcDescription),"");
		if(nNewEmrID == -1) {
			//Don't know why this would happen, but if it does, we've got problems.
			AfxThrowNxException("CEmrMoveEmnDlg::OnOK - Selected EMR (%s) has an invalid ID",
				strNewEmrDescription);
		}

		CString strConfirmation = FormatString("Are you sure you want to move the \"%s\" EMN to the \"%s\" EMR?",
			m_strEmnDescription, strNewEmrDescription);
		if(IDYES == MessageBox(strConfirmation,NULL,MB_YESNO)) {
			// (a.walling 2008-06-25 16:25) - PLID 30515 - Ensure no one is editing this EMN
			// (a.walling 2008-07-28 16:14) - PLID 30515 - Use UPDLOCK, HOLDLOCK
			// (c.haag 2009-12-22 9:33) - PLID 35660 - Update merged documents linked with EMN's
			// (j.armen 2013-05-14 11:54) - PLID 56680 - Refactored EMN Access, moved into a batch
			CParamSqlBatch sql;

			sql.Add("DECLARE @emnID INT");
			sql.Add("DECLARE @groupID INT");

			sql.Add("SELECT @emnID = {INT}, @groupID = {INT}", m_nEmnID, nNewEmrID);

			sql.Add("IF EXISTS(SELECT 1 FROM EMNAccessT WITH(UPDLOCK, HOLDLOCK) WHERE EmnID = @emnID)");
			sql.Add("BEGIN");
			sql.Add("	RAISERROR('EMN cannot be moved; it is being modified by another user.', 16, 43)");
			sql.Add("	ROLLBACK TRAN");
			sql.Add("	RETURN");
			sql.Add("END");

			sql.Add("UPDATE EMRMasterT SET EMRGroupID = @groupID, ModifiedDate = GETDATE() WHERE EMRMasterT.ID = @emnID");
			
			sql.Add(
				"UPDATE MailSent SET PicID =\r\n"
				"	CASE\r\n"
				"		WHEN (SELECT COUNT(ID) FROM PicT WHERE EMRGroupID = @groupID) = 1 THEN (SELECT ID FROM PicT WHERE EMRGroupID = @groupID)\r\n"
				"		ELSE NULL\r\n"
				"	END\r\n"
				"WHERE EmnID = @emnID");

			sql.Execute(GetRemoteData());

			// Audit the move
			CString strAuditOldValue = FormatString("Old EMR: %s (EMN: %s)", m_strOldEmrDescription, m_strEmnDescription);
			CString strAuditNewValue = FormatString("New EMR: %s", strNewEmrDescription);
			long nAuditID = BeginNewAuditEvent();
			// (j.jones 2009-09-24 14:38) - PLID 31672 - I renamed this audit object
			AuditEvent(m_nPatientID, m_strPatientName, nAuditID, aeiEMNMovedToAnotherEMR, m_nEmnID, strAuditOldValue, strAuditNewValue, aepHigh);
		}
		else {
			OnCancel();
		}

		CNxDialog::OnOK();

	}NxCatchAll("CEmrMoveEmnDlg::OnOK");
}

void CEmrMoveEmnDlg::OnDblClickCellEmrList(LPDISPATCH lpRow, short nColIndex)
{
	if(lpRow == NULL) {
		return;
	}

	// If we have a valid row, let's commit the dialog with this selection
	OnOK();
}

void CEmrMoveEmnDlg::OnRequeryFinishedEmrList(short nFlags)
{
	// If there aren't any options to move to, let's just dismiss the dialog.
	if(m_pdlEmrList->GetRowCount() <= 0) {
		MessageBox("This patient does not have any other EMRs, so this EMN can't be moved.");
		OnCancel();
	}
}

// (j.jones 2009-09-24 14:57) - PLID 31672 - added ability to move EMNs between patients
// (j.jones 2009-09-28 08:39) - PLID 31672 - removed this feature
/*
void CEmrMoveEmnDlg::OnDblClickCellMoveToPatientList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		if(lpRow == NULL) {
			return;
		}

		// If we have a valid row, let's commit the dialog with this selection
		OnOK();

	}NxCatchAll("Error in CEmrMoveEmnDlg::OnDblClickCellMoveToPatientList");
}
*/