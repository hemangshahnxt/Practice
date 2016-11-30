// EMRAuditHistoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRAuditHistoryDlg.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

#define COLUMN_AUDIT_ID			0
#define COLUMN_AUDIT_DETAIL_ID	1
#define COLUMN_CHANGED_DATE		2
#define COLUMN_LOCATION_NAME	3
#define COLUMN_CHANGED_BY_NAME	4
#define COLUMN_PERSON_NAME		5
#define COLUMN_ITEM				6
#define COLUMN_RECORD_ID		7
#define COLUMN_OLD_VALUE		8
#define COLUMN_NEW_VALUE		9
#define COLUMN_PRIORITY			10
#define COLUMN_TYPE				11

using namespace NXDATALISTLib;
////////////////////////////////////////////////////////////////////////////
// CEMRAuditHistoryDlg dialog


CEMRAuditHistoryDlg::CEMRAuditHistoryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRAuditHistoryDlg::IDD, pParent, "CEMRAuditHistoryDlg") // (a.walling 2013-08-19 10:33) - PLID 56327 - Remember size and position
{
	//{{AFX_DATA_INIT(CEMRAuditHistoryDlg)
		m_nEMRID = -1;
		m_nEMNID = -1;
	//}}AFX_DATA_INIT
}


void CEMRAuditHistoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRAuditHistoryDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_COPY_OUTPUT_BTN, m_btnCopyOutput); // (b.cardillo 2010-01-07 13:26) - PLID 35780
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRAuditHistoryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRAuditHistoryDlg)
	ON_BN_CLICKED(IDC_COPY_OUTPUT_BTN, OnCopyOutputBtn) // (b.cardillo 2010-01-07 13:26) - PLID 35780
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRAuditHistoryDlg message handlers

BOOL CEMRAuditHistoryDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_List = BindNxDataListCtrl(this,IDC_EMR_AUDIT_LIST,GetRemoteData(),false);

	// (z.manning 2009-05-22 16:37) - PLID 34330 - We now have a separate permission for the
	// old/new value columns.
	if(!CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic0, FALSE, FALSE, TRUE)) {
		IColumnSettingsPtr pCol;
		pCol = m_List->GetColumn(COLUMN_OLD_VALUE);
		pCol->PutColumnStyle(pCol->GetColumnStyle() & (~csVisible));
		pCol = m_List->GetColumn(COLUMN_NEW_VALUE);
		pCol->PutColumnStyle(pCol->GetColumnStyle() & (~csVisible));
	}

	// (a.walling 2013-08-19 10:33) - PLID 56327 - EMR Audit History dialog does not change its window title to reflect whether it is showing history for a single EMN or an entire EMR (or a template)
	if (m_nEMNID != -1) {
		SetWindowText("Individual EMN Chart History");
	} else if (m_nEMRID != -1) {
		SetWindowText("EMR Chart History");
	} else if (m_nEMRTemplateID != -1) {
		SetWindowText("EMR Template History");
	}

	try {		
		// (a.walling 2010-06-08 10:02) - PLID 38558 - Audit flags
		EnsureAuditFlags();

		// (c.haag 2008-08-29 10:13) - PLID 22401 - We also need to show audit information for EMR problems.
		// This is the base query that by itself returns a list of audit fields for all problems in the entire database.		
		// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
		// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
		// (c.haag 2009-05-12 12:24) - PLID 34234 - EMR problem link table
		// (j.jones 2009-05-27 12:42) - PLID 34324 - supported problem link auditing.
		// (c.haag 2009-05-30 14:08) - PLID 34234 - There is no longer a single base query;
		// all the queries are now separate.
		// (b.spivey, October 22, 2013) - PLID 58677 - snomed auditing

		// (c.haag 2008-04-28 11:47) - PLID 29806 - NxIconize the close button
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnCopyOutput.AutoSet(NXB_EXPORT); // (b.cardillo 2010-01-07 13:26) - PLID 35780

		BOOL bHasL1EMR = g_pLicense->HasEMR(CLicense::cflrSilent) == 1;

		CSqlFragment sql("");

		// (c.haag 2008-08-29 11:18) - PLID 22401 - I'm also changing the logic to not check the Deleted flag of
		// EmrMasterT at all. If we did check the flag, then we omit the entire audit history of deleted EMN's and
		// when they were deleted. Even if m_nEMNID is not -1 and it's deleted or not in data...we still need to 
		// show everything that we can (Though I don't think it's possible through the program).
		if(m_nEMNID != -1) {
			// (c.haag 2008-08-29 10:31) - PLID 22401 - Updated to include EMR problems
			// (j.jones 2009-05-27 12:42) - PLID 34324 - supported problem link auditing
			// (d.thompson 2010-03-16) - PLID 37721 - Add a recordset before the audit details with the name info
			// (d.thompson 2010-03-17) - PLID 37721 - Lastly, add that particular event to show in the chart history!  ItemID 9330 (won't see current view)
			// (a.walling 2010-06-24 16:59) - PLID 39359 - Exclude aeiEMROpened 7092 (audited with patient ID) and only show aeiViewAuditNexEMR 9330 if it is an EMN-level record
			// (r.gonet 04/22/2013) - PLID 54308 - Replaced multiple OR conditions with UNIONs to reduce the amount of time the query takes to run on large databases (previously timing out).
			//   Sorry, I had to explicitly state the column selection list because TEXT columns cannot be UNIONed (though they can be UNION ALLed).
			// (b.spivey December 10th, 2013) PLID 58677 - explicitly filter out our new audits... plus some old ones.
			// (j.jones 2016-05-05 12:32) - NX-100497 - parameterized and fixed EMRProblems IN clause
			sql = CSqlFragment(
				"SELECT EMRMasterT.Date, EMRMasterT.Description AS AuditText, "
				"PersonT.ID AS PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName "
				"FROM EMRMasterT INNER JOIN PersonT ON EMRMasterT.PatientID = PersonT.ID "
				"WHERE EMRMasterT.ID = {INT};\r\n\r\n"

				"DECLARE @nEMNID INT \r\n "
				"SET @nEMNID = {INT} \r\n "
				"SELECT AuditT.ChangedByUserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate, "
 					"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, CONVERT(VARCHAR(MAX), AuditDetailsT.OldValue) AS OldValue, CONVERT(VARCHAR(MAX), AuditDetailsT.NewValue) AS NewValue, "
 					"AuditDetailsT.Priority, AuditDetailsT.Type, AuditDetailsT.PersonName, AuditDetailsT.ItemID, AuditDetailsT.InternalPatientID, "
 					"AuditDetailsT.ID AS AuditDetailID "
				"FROM AuditT "
				"INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "  
				"WHERE AuditDetailsT.ItemID >= 7001 AND AuditDetailsT.ItemID <= 8000 AND AuditDetailsT.ItemID NOT IN (7032,7033,7034,7035,7048,7049,7053,7078,7092,7093,7094, 7087, 7088, 7122) "
				"AND RecordID = @nEMNID "
				"UNION "
				"SELECT AuditT.ChangedByUserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate, "
 					"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, CONVERT(VARCHAR(MAX), AuditDetailsT.OldValue) AS OldValue, CONVERT(VARCHAR(MAX), AuditDetailsT.NewValue) AS NewValue, "
 					"AuditDetailsT.Priority, AuditDetailsT.Type, AuditDetailsT.PersonName, AuditDetailsT.ItemID, AuditDetailsT.InternalPatientID, "
 					"AuditDetailsT.ID AS AuditDetailID "
				"FROM AuditT "
				"INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"WHERE AuditDetailsT.ItemID = 9330 AND CONVERT(NVARCHAR(21), AuditDetailsT.NewValue) = 'Chart history for EMN' "
				"AND RecordID = @nEMNID "
				"UNION ALL "
				"SELECT AuditT.ChangedByUserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate, "
 					"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, CONVERT(VARCHAR(MAX), AuditDetailsT.OldValue) AS OldValue, CONVERT(VARCHAR(MAX), AuditDetailsT.NewValue) AS NewValue, "
 					"AuditDetailsT.Priority, AuditDetailsT.Type, AuditDetailsT.PersonName, AuditDetailsT.ItemID, AuditDetailsT.InternalPatientID, "
 					"AuditDetailsT.ID AS AuditDetailID "
				"FROM "
				"EMRProblemsT "
				"INNER JOIN (SELECT * FROM AuditDetailsT WHERE ItemID IN (7048, 7049, 7053, 7078, 7087, 7088, 7093, 7094, 7122)) AS AuditDetailsT ON AuditDetailsT.RecordID = EmrProblemsT.ID "
				"INNER JOIN AuditT ON AuditDetailsT.AuditID = AuditT.ID "
				"INNER JOIN ("
				"	SELECT EMRProblemID FROM EMRProblemLinkT WHERE "
				"	CASE EMRProblemLinkT.EmrRegardingType "
				"		WHEN {CONST_INT} THEN (SELECT EMRDetailsT.EmrID FROM EMRDetailsT WHERE EMRDetailsT.ID = EMRProblemLinkT.EmrRegardingID) "
				"		WHEN {CONST_INT} THEN (SELECT EMRDetailsT.EmrID FROM EMRDetailsT WHERE EMRDetailsT.ID = EMRProblemLinkT.EmrRegardingID) "
				"		WHEN {CONST_INT} THEN (SELECT EMRTopicsT.EmrID FROM EMRTopicsT WHERE EMRTopicsT.ID = EMRProblemLinkT.EmrRegardingID) "
				"		WHEN {CONST_INT} THEN EMRProblemLinkT.EmrRegardingID "
				"		WHEN {CONST_INT} THEN (SELECT EMRDiagCodesT.EmrID FROM EMRDiagCodesT WHERE EMRDiagCodesT.ID = EMRProblemLinkT.EmrRegardingID) "
				"		WHEN {CONST_INT} THEN (SELECT EMRChargesT.EmrID FROM EMRChargesT WHERE EMRChargesT.ID = EMRProblemLinkT.EmrRegardingID) "
				"		WHEN {CONST_INT} THEN (SELECT EMRMedicationsT.EmrID FROM EMRMedicationsT WHERE EMRMedicationsT.MedicationID = EMRProblemLinkT.EmrRegardingID) "
				"		ELSE NULL "
				"		END = @nEMNID "	
				"	) EMRProblemsSubQ "
				"ON EMRProblemsT.ID = EMRProblemsSubQ.EMRProblemID "
				,m_nEMNID
				,m_nEMNID
				,eprtEmrDataItem
				,eprtEmrItem
				,eprtEmrTopic
				,eprtEmrEMN
				,eprtEmrDiag
				,eprtEmrCharge
				,eprtEmrMedication
				);
		}
		else if(m_nEMRID != -1) {
			// (c.haag 2008-08-29 10:31) - PLID 22401 - Updated to include EMR problems
			// (c.haag 2009-05-12 12:25) - PLID 34234 - Added problem linking table
			// (j.jones 2009-05-27 12:42) - PLID 34324 - supported problem link auditing
			// (d.thompson 2010-03-16) - PLID 37721 - Add a recordset before the audit details with the name info
			// (d.thompson 2010-03-17) - PLID 37721 - Lastly, add that particular event to show in the chart history!  ItemID 9330 (won't see current view)
			// (a.walling 2010-06-24 16:59) - PLID 39359 - Exclude aeiEMROpened 7092 (audited with patient ID) and only show aeiViewAuditNexEMR 9330 if it is an EMR-level or EMN-level (for this EMR) record
			// (r.gonet 04/22/2013) - PLID 54308 - Replaced multiple OR conditions with UNIONs to reduce the amount of time the query takes to run on large databases (previously timing out).
			// (b.spivey December 10th, 2013) PLID 58677 - explicitly filter out our new audits... plus some old ones.
			// (j.jones 2016-05-05 12:32) - NX-100497 - parameterized and fixed EMRProblems IN clause
			sql = CSqlFragment(
				"SELECT NULL AS Date, EMRGroupsT.Description AS AuditText, "
				"PersonT.ID AS PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName "
				"FROM EMRGroupsT INNER JOIN PersonT ON EMRGroupsT.PatientID = PersonT.ID "
				"WHERE EMRGroupsT.ID = {INT};\r\n\r\n"

				"DECLARE @nEMRID INT \r\n "
				"SET @nEMRID = {INT} \r\n "
				"SELECT AuditT.ChangedByUserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate, "
 					"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, CONVERT(VARCHAR(MAX), AuditDetailsT.OldValue) AS OldValue, CONVERT(VARCHAR(MAX), AuditDetailsT.NewValue) AS NewValue, "
 					"AuditDetailsT.Priority, AuditDetailsT.Type, AuditDetailsT.PersonName, AuditDetailsT.ItemID, AuditDetailsT.InternalPatientID, "
 					"AuditDetailsT.ID AS AuditDetailID "
				"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"INNER JOIN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = @nEMRID) AS EMRMasterQ ON AuditDetailsT.RecordID = EMRMasterQ.ID "
				"WHERE (AuditDetailsT.ItemID >= 7001 AND AuditDetailsT.ItemID <= 8000 AND AuditDetailsT.ItemID NOT IN (7032,7033,7034,7035,7048,7049,7053,7078,7092,7093,7094, 7087, 7088, 7122)) "
				"UNION "
				"SELECT AuditT.ChangedByUserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate, "
 					"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, CONVERT(VARCHAR(MAX), AuditDetailsT.OldValue) AS OldValue, CONVERT(VARCHAR(MAX), AuditDetailsT.NewValue) AS NewValue, "
 					"AuditDetailsT.Priority, AuditDetailsT.Type, AuditDetailsT.PersonName, AuditDetailsT.ItemID, AuditDetailsT.InternalPatientID, "
 					"AuditDetailsT.ID AS AuditDetailID "
				"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"WHERE (AuditDetailsT.ItemID IN (7032,7033,7034,7035) AND RecordID = @nEMRID) "
				"UNION "
				"SELECT AuditT.ChangedByUserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate, "
 					"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, CONVERT(VARCHAR(MAX), AuditDetailsT.OldValue) AS OldValue, CONVERT(VARCHAR(MAX), AuditDetailsT.NewValue) AS NewValue, "
 					"AuditDetailsT.Priority, AuditDetailsT.Type, AuditDetailsT.PersonName, AuditDetailsT.ItemID, AuditDetailsT.InternalPatientID, "
 					"AuditDetailsT.ID AS AuditDetailID "
				"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"WHERE (AuditDetailsT.ItemID = 9330 AND RecordID = @nEMRID AND CONVERT(NVARCHAR(21), AuditDetailsT.NewValue) = 'Chart history for EMR') "
				"UNION "
				"SELECT AuditT.ChangedByUserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate, "
 					"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, CONVERT(VARCHAR(MAX), AuditDetailsT.OldValue) AS OldValue, CONVERT(VARCHAR(MAX), AuditDetailsT.NewValue) AS NewValue, "
 					"AuditDetailsT.Priority, AuditDetailsT.Type, AuditDetailsT.PersonName, AuditDetailsT.ItemID, AuditDetailsT.InternalPatientID, "
 					"AuditDetailsT.ID AS AuditDetailID "
				"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"INNER JOIN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = @nEMRID) AS EMRMasterQ ON AuditDetailsT.RecordID = EMRMasterQ.ID "
				"WHERE (AuditDetailsT.ItemID = 9330 AND CONVERT(NVARCHAR(21), AuditDetailsT.NewValue) = 'Chart history for EMN') "
				" "
				"UNION ALL "
				" "
				"SELECT AuditT.ChangedByUserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate, "
 					"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, CONVERT(VARCHAR(MAX), AuditDetailsT.OldValue) AS OldValue, CONVERT(VARCHAR(MAX), AuditDetailsT.NewValue) AS NewValue, "
 					"AuditDetailsT.Priority, AuditDetailsT.Type, AuditDetailsT.PersonName, AuditDetailsT.ItemID, AuditDetailsT.InternalPatientID, "
 					"AuditDetailsT.ID AS AuditDetailID "
				"FROM "
				"EMRProblemsT "
				"INNER JOIN (SELECT * FROM AuditDetailsT WHERE ItemID IN (7048, 7049, 7053, 7078, 7087, 7088, 7093, 7094, 7122)) AS AuditDetailsT ON AuditDetailsT.RecordID = EmrProblemsT.ID "
				"INNER JOIN AuditT ON AuditDetailsT.AuditID = AuditT.ID "
				"INNER JOIN ("
				"	SELECT EMRProblemID FROM EMRProblemLinkT WHERE "
				"	CASE EMRProblemLinkT.EmrRegardingType "
				"		WHEN {CONST_INT} THEN (SELECT EMRDetailsT.EmrID FROM EMRDetailsT WHERE EMRDetailsT.ID = EMRProblemLinkT.EmrRegardingID) "
				"		WHEN {CONST_INT} THEN (SELECT EMRDetailsT.EmrID FROM EMRDetailsT WHERE EMRDetailsT.ID = EMRProblemLinkT.EmrRegardingID) "
				"		WHEN {CONST_INT} THEN (SELECT EMRTopicsT.EmrID FROM EMRTopicsT WHERE EMRTopicsT.ID = EMRProblemLinkT.EmrRegardingID) "
				"		WHEN {CONST_INT} THEN EMRProblemLinkT.EmrRegardingID "
				"		WHEN {CONST_INT} THEN (SELECT EMRDiagCodesT.EmrID FROM EMRDiagCodesT WHERE EMRDiagCodesT.ID = EMRProblemLinkT.EmrRegardingID) "
				"		WHEN {CONST_INT} THEN (SELECT EMRChargesT.EmrID FROM EMRChargesT WHERE EMRChargesT.ID = EMRProblemLinkT.EmrRegardingID) "
				"		WHEN {CONST_INT} THEN (SELECT EMRMedicationsT.EmrID FROM EMRMedicationsT WHERE EMRMedicationsT.MedicationID = EMRProblemLinkT.EmrRegardingID) "
				"		ELSE NULL "
				"		END IN (SELECT ID FROM EmrMasterT WHERE EMRGroupID = @nEMRID) "	
				"	) EMRProblemsSubQ "
				"ON EMRProblemsT.ID = EMRProblemsSubQ.EMRProblemID "
				"UNION ALL "
				"SELECT AuditT.ChangedByUserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate, "
 					"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, CONVERT(VARCHAR(MAX), AuditDetailsT.OldValue) AS OldValue, CONVERT(VARCHAR(MAX), AuditDetailsT.NewValue) AS NewValue, "
 					"AuditDetailsT.Priority, AuditDetailsT.Type, AuditDetailsT.PersonName, AuditDetailsT.ItemID, AuditDetailsT.InternalPatientID, "
 					"AuditDetailsT.ID AS AuditDetailID "
				"FROM AuditT "
				"INNER JOIN (SELECT * FROM AuditDetailsT WHERE ItemID IN (7048, 7049, 7053, 7078, 7093, 7094, 7087, 7088, 7122)) AS AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"INNER JOIN ("
				"	SELECT EmrProblemsT.ID FROM EmrProblemsT INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID WHERE EmrRegardingType = {CONST_INT} AND EmrRegardingID = @nEMRID "
				") AS EMRProblemsSubQ ON AuditDetailsT.RecordID = EMRProblemsSubQ.ID"
				,m_nEMRID
				,m_nEMRID
				,m_nEMRID
				,m_nEMRID
				,eprtEmrDataItem
				,eprtEmrItem
				,eprtEmrTopic
				,eprtEmrEMN
				,eprtEmrDiag
				,eprtEmrCharge
				,eprtEmrMedication				
				,eprtEmrEMR);
		}
		else if(m_nEMRTemplateID != -1) {
			// (d.thompson 2010-03-16) - PLID 37721 - Add a recordset before the audit details with the name info
			// (d.thompson 2010-03-17) - PLID 37721 - We are not adding the "view audit" to this view.  It really doesn't 
			//	matter who views a template.
			// (r.gonet 04/22/2013) - PLID 54308 - Made the select column list explicit in order to be consistent with the changes to the View EMN History and View EMR History branches.
			// (j.jones 2016-05-05 12:32) - NX-100497 - parameterized
			sql = CSqlFragment(
				"SELECT NULL AS Date, EMRTemplateT.Name AS AuditText, -1 AS PersonID, '' AS PatName "
				"FROM EMRTemplateT WHERE ID = {INT};\r\n\r\n"

				"SELECT AuditT.ChangedByUserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate, "
 					"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, CONVERT(VARCHAR(MAX), AuditDetailsT.OldValue) AS OldValue, CONVERT(VARCHAR(MAX), AuditDetailsT.NewValue) AS NewValue, "
 					"AuditDetailsT.Priority, AuditDetailsT.Type, AuditDetailsT.PersonName, AuditDetailsT.ItemID, AuditDetailsT.InternalPatientID, "
 					"AuditDetailsT.ID AS AuditDetailID "
				"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"WHERE (AuditDetailsT.ItemID >= 9801 AND AuditDetailsT.ItemID <= 10000) "
				"AND RecordID = {INT}",m_nEMRTemplateID, m_nEMRTemplateID);
		}

		IRowSettingsPtr pRow;
		_variant_t var;

		// (j.jones 2016-05-05 12:28) - NX-100497 - increase the timeout
		CIncreaseCommandTimeout cict(GetRemoteDataSnapshot(), 600);

		// (a.walling 2013-08-09 10:37) - PLID 57952 - EMR Audit History should load using snapshot isolation
		// (j.jones 2016-05-05 12:32) - NX-100497 - parameterized
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "{SQL}", sql);

		// (d.thompson 2010-03-16) - PLID 37721 - First recordset is now audit description
		CString strAuditDesc;
		long nAuditPersonID = -1;
		long nAuditRecordID = -1;
		CString strAuditPersonName;
		if(!rs->eof) {
			strAuditDesc = AdoFldString(rs, "AuditText", "");
			_variant_t varDate = rs->Fields->Item["Date"]->Value;
			if(varDate.vt == VT_DATE) {
				strAuditDesc = FormatString("(%s) %s", FormatDateTimeForInterface(VarDateTime(varDate), NULL, dtoDate), strAuditDesc);
			}

			//patient info, if non-template
			nAuditPersonID = AdoFldLong(rs, "PersonID", -1);
			strAuditPersonName = AdoFldString(rs, "PatName", "");
		}
		else {
			//If the EMN / EMR / Template doesn't exist (not yet saved), you can reach here.  Just leave the text blank, we
			//	almost certainly don't have anything in our display data either, and thus won't audit at all.
		}

		//Now move on to the main query
		rs = rs->NextRecordset(NULL);

		if(rs->eof) {
			// (d.thompson 2009-05-29) - PLID 34407 - Renamed "Audit History" to "Chart History"
			CString str = "No chart history was found for this EMN. This means that no changes to the data have been made to the EMN since it was created.";
			if(m_nEMRID != -1)
				str.Replace("EMN","EMR");
			else if(m_nEMRTemplateID != -1)
				str.Replace("EMN","template");
			AfxMessageBox(str);
		}
		else {
			// (d.thompson 2010-03-16) - PLID 37721 - Audit when viewing auditing.
			CString strAuditText = "Chart history for ";
			if(m_nEMNID != -1) {
				strAuditText += "EMN ";
				nAuditRecordID = m_nEMNID;
			}
			else if(m_nEMRID != -1) {
				strAuditText += "EMR ";
				nAuditRecordID = m_nEMRID;
			}
			else if(m_nEMRTemplateID != -1) {
				strAuditText += "Template ";
				nAuditRecordID = m_nEMRTemplateID;
			}
			//Get the description from the query
			strAuditText += strAuditDesc;

			AuditEvent(nAuditPersonID, strAuditPersonName, BeginNewAuditEvent(), aeiViewAuditNexEMR, nAuditRecordID, "", strAuditText, aepMedium, aetOpened);
		}

		while(!rs->eof) {			

			pRow = m_List->GetRow(-1);			

			var = rs->Fields->Item["AuditID"]->Value;
			pRow->PutValue(COLUMN_AUDIT_ID,var);

			var = rs->Fields->Item["AuditDetailID"]->Value;
			pRow->PutValue(COLUMN_AUDIT_DETAIL_ID,var);

			var = rs->Fields->Item["ChangedDate"]->Value;
			pRow->PutValue(COLUMN_CHANGED_DATE,var);

			var = rs->Fields->Item["ChangedAtLocationName"]->Value;
			pRow->PutValue(COLUMN_LOCATION_NAME,var);

			var = rs->Fields->Item["ChangedByUserName"]->Value;
			pRow->PutValue(COLUMN_CHANGED_BY_NAME,var);

			var = rs->Fields->Item["PersonName"]->Value;
			pRow->PutValue(COLUMN_PERSON_NAME,var);

			var = rs->Fields->Item["ItemID"]->Value;
			pRow->PutValue(COLUMN_ITEM,_bstr_t(GetAuditItemDescription(var.lVal)));

			var = rs->Fields->Item["RecordID"]->Value;
			pRow->PutValue(COLUMN_RECORD_ID,var);

			var = rs->Fields->Item["OldValue"]->Value;
			pRow->PutValue(COLUMN_OLD_VALUE,var);

			var = rs->Fields->Item["NewValue"]->Value;
			pRow->PutValue(COLUMN_NEW_VALUE,var);

			var = rs->Fields->Item["Priority"]->Value;
			pRow->PutValue(COLUMN_PRIORITY,var);

			var = rs->Fields->Item["Type"]->Value;
			pRow->PutValue(COLUMN_TYPE,var);

			rs->MoveNext();

			m_List->AddRow(pRow);
		}
		rs->Close();

	}NxCatchAll("Error in OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (b.cardillo 2010-01-07 13:26) - PLID 35780 - Added ability to save the current audit results to .csv
void CEMRAuditHistoryDlg::OnCopyOutputBtn()
{
	try {
		PromptSaveFile_CsvFromDLor2(m_List, this, "AuditExport", TRUE);
	} NxCatchAll(__FUNCTION__);
}
