// ProcedureReplaceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ProcedureReplaceDlg.h"
#include "globalDataUtils.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CProcedureReplaceDlg dialog


CProcedureReplaceDlg::CProcedureReplaceDlg(CWnd* pParent)
	: CNxDialog(CProcedureReplaceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProcedureReplaceDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CProcedureReplaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcedureReplaceDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcedureReplaceDlg, CNxDialog)
	//{{AFX_MSG_MAP(CProcedureReplaceDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CProcedureReplaceDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CProcedureReplaceDlg)
	ON_EVENT(CProcedureReplaceDlg, IDC_FIND_TYPE, 16 /* SelChosen */, OnSelChosenFindType, VTS_I4)
	ON_EVENT(CProcedureReplaceDlg, IDC_REPLACE_TYPE, 16 /* SelChosen */, OnSelChosenReplaceType, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


bool CProcedureReplaceDlg::GetID(long &id, _DNxDataListPtr &list)
{
	long curSel = list->CurSel;
	_variant_t var;

	if (curSel == -1)
		return false;

	var = list->Value[curSel][0];
	if (var.vt != VT_I4)//all combos here have NULL ID's allowed
		return false;

	id = VarLong(var);
	return true;
}

// (j.gruber 2008-06-27 09:34) - PLID 26542 - used for auditing
bool CProcedureReplaceDlg::GetName(CString &strName, _DNxDataListPtr &list)
{
	try {
		long curSel = list->CurSel;
		_variant_t var;

		if (curSel == -1)
			return false;

		var = list->Value[curSel][1];
		if (var.vt != VT_BSTR)//all combos here have NULL ID's allowed
			return false;

		strName = VarString(var);
		return true;
	}NxCatchAll("Error in CProcedureReplaceDlg::GetName");
	return false;
}

void CProcedureReplaceDlg::RequeryFindPurpose()
{
	CString where;
	long	typeID,
			purposeID;
	bool	selected;
	
	if (GetID(typeID, m_findType))
		where.Format("AppointmentsT.AptTypeID = %i", typeID);
	else where = "AppointmentsT.AptTypeID IS NULL";

	m_findPurpose->WhereClause = _bstr_t(where);

	selected = GetID(purposeID, m_findPurpose);

	m_findPurpose->Requery();

	if (selected)
		m_findPurpose->SetSelByColumn(0, purposeID);
	
	if (m_findType->CurSel == -1)
		m_findType->CurSel = 0;
}

void CProcedureReplaceDlg::RequeryReplacePurpose()
{
	CString where;
	long	typeID,
			purposeID;
	bool	selected;
	IRowSettingsPtr pRow;

	selected = GetID(purposeID, m_replacePurpose);

	// (c.haag 2008-12-17 17:42) - PLID 32264 - Filter out inactive procedures
	if (GetID(typeID, m_replaceType))
	{	where.Format("((ProcedureT.ID IS NULL AND AptTypeT.Category = 0) "
			"OR (ProcedureT.ID IS NOT NULL AND ProcedureT.Inactive = 0 AND AptTypeT.Category <> 0))"
			"AND AptPurposeTypeT.AptTypeID = %i", typeID);

		m_replacePurpose->WhereClause = _bstr_t(where);
	}
	else m_replacePurpose->WhereClause = _bstr_t("0 = 1");
	
	m_replacePurpose->Requery();
	pRow = m_replacePurpose->GetRow(-1);
	pRow->Value[1] = _bstr_t("<None>");
	m_replacePurpose->InsertRow(pRow,0);

	if (selected)
		m_replaceType->SetSelByColumn(0, purposeID);
	
	if (m_replaceType->CurSel == -1)
		m_replaceType->CurSel = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CProcedureReplaceDlg message handlers

BOOL CProcedureReplaceDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	IRowSettingsPtr pRow;
	m_findType			= BindNxDataListCtrl(IDC_FIND_TYPE);
	m_findPurpose		= BindNxDataListCtrl(IDC_FIND_PURPOSE, false);

	m_replaceType		= BindNxDataListCtrl(IDC_REPLACE_TYPE);
	m_replacePurpose	= BindNxDataListCtrl(IDC_REPLACE_PURPOSE, false);

	pRow = m_replaceType->GetRow(-1);
	pRow->Value[1] = _bstr_t("<None>");
	m_replaceType->InsertRow(pRow,0);

	RequeryFindPurpose();
	RequeryReplacePurpose();

	m_findType->CurSel = 0;
	m_findPurpose->CurSel = 0;
	m_replaceType->CurSel = 0;
	m_replacePurpose->CurSel = 0;

	return TRUE;
}

void CProcedureReplaceDlg::OnOK() 
{
	CString strFindPurpose, strFindType, strReplacePurpose, strReplaceType, msg;
	long id;

	// (j.gruber 2008-06-27 09:19) - PLID 26542 - audit each appointment
	long nAuditTransactionID = -1;

	try
	{
		// (c.haag 2007-02-23 09:27) - PLID 24897 - This will take a long time (meaning more
		// than a second) on large data sets
		CWaitCursor wc;

		if (GetID(id, m_findType))
			strFindType.Format ("= %i", id);
		else strFindType = "IS NULL";

		if (GetID(id, m_findPurpose))
			strFindPurpose.Format ("= %i", id);
		else strFindPurpose = "IS NULL";

		_RecordsetPtr rs = CreateRecordset("SELECT COUNT (AppointmentsT.ID) AS CID FROM AppointmentsT "
			"LEFT JOIN AppointmentPurposeT ON AppointmentPurposeT.AppointmentID = AppointmentsT.ID "
			"WHERE AptTypeID %s AND AppointmentPurposeT.PurposeID %s", strFindType, strFindPurpose);

		msg.Format("This will change %i Appointments."
			"\nThis cannot be undone.\n"
			"Are You sure?", AdoFldLong(rs, "CID"));

		rs->Close();

		if (IDYES != AfxMessageBox(msg, MB_YESNO))
			return;

		if (GetID(id, m_replaceType))
			strReplaceType.Format ("= %i", id);
		else strReplaceType = "= NULL";

		if (GetID(id, m_replacePurpose))
			strReplacePurpose.Format ("= %i", id);
		else strReplacePurpose = "= NULL";

		// Make a temporary table to store all the appointments with a matching type
		// (c.haag 2007-02-23 09:19) - PLID 24897 - Instead of using temp tables, we
		// now use table variables and batch SQL.
		/*try {
			ExecuteSql("DROP TABLE #AptTypeTemp");
		}
		catch(...)
		{
		}
		ExecuteSql("CREATE TABLE #AptTypeTemp (ID INT NOT NULL)");
		ExecuteSql("INSERT INTO #AptTypeTemp SELECT ID FROM AppointmentsT WHERE AptTypeID %s",
			strFindType);*/

		CString strSqlBatch = BeginSqlBatch();
		AddStatementToSqlBatch(strSqlBatch, "DECLARE @AptTypeTemp TABLE (ID INT NOT NULL) \r\n"
			"INSERT INTO @AptTypeTemp (ID) SELECT ID FROM AppointmentsT WHERE AptTypeID %s",
			strFindType);

		// Replace the types
		if (strFindType != strReplaceType)
		{
			AddStatementToSqlBatch(strSqlBatch, "UPDATE AppointmentsT SET AptTypeID %s WHERE AppointmentsT.ID IN (SELECT AppointmentsT.ID FROM AppointmentsT "
				"LEFT JOIN AppointmentPurposeT ON AppointmentPurposeT.AppointmentID = AppointmentsT.ID "
				"WHERE AptTypeID %s AND AppointmentPurposeT.PurposeID %s)", 
				strReplaceType, strFindType, strFindPurpose);

		

		}
		// Replace the purpii
		if (strFindPurpose != strReplacePurpose)
		{
			// If there is a source purpose, then we need to modify
			// existing AppointmentPurpose records
			if (GetID(id, m_findPurpose))
			{
				if (GetID(id, m_replacePurpose))
				{
					// (a.walling 2008-06-17 11:38) - PLID 30415 - Handle possible duplicates
					AddStatementToSqlBatch(strSqlBatch, "UPDATE AppointmentPurposeT SET PurposeID = %d WHERE PurposeID %s AND AppointmentID IN (SELECT ID FROM @AptTypeTemp)"
						" AND AppointmentID NOT IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID = %d)",
						id, strFindPurpose, id);
					// (a.walling 2008-06-17 12:04) - PLID 30415 - Now for the ones that would have been duplicated, we need to delete
					GetID(id, m_findPurpose);
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM AppointmentPurposeT WHERE AppointmentID IN (SELECT ID FROM @AptTypeTemp)"
						" AND PurposeID = %d", id);
				}
				else
				{
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM AppointmentPurposeT WHERE PurposeID %s AND AppointmentID IN (SELECT ID FROM @AptTypeTemp)",
						strFindPurpose);
				}
			}
			else // If there is no source purpose, we either add new AppointmentPurpose
				// records, or do nothing
			{
				if (GetID(id, m_replacePurpose))
				{
					// (a.walling 2008-06-17 11:38) - PLID 30415 - Handle possible duplicates
					AddStatementToSqlBatch(strSqlBatch, "INSERT INTO AppointmentPurposeT (AppointmentID, PurposeID) SELECT ID, %d FROM AppointmentsT WHERE AppointmentsT.ID IN (SELECT ID FROM @AptTypeTemp)"
						" AND AppointmentsT.ID NOT IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID = %d)",
						id, id);
				}
			}
			
		}

		
		CString strFindTypeName, strReplaceTypeName;
		long nFindTypeID, nReplaceTypeID;
		CString strFindPurposeName, strReplacePurposeName;
		long nFindPurposeID, nReplacePurposeID;
		//need to redo the ID check because if the selection is null, then the above comparisons don't work correctly and looks dumb in auditing
		//types
		if (!GetID(nFindTypeID, m_findType)) {
			nFindTypeID = -1;
		}
		if (!GetID(nReplaceTypeID, m_replaceType)) {
			nReplaceTypeID = -1;
		}
		if (!GetName(strFindTypeName, m_findType)) {
			strFindTypeName = "<None>";
		}
		if (!GetName(strReplaceTypeName, m_replaceType)) {
			strReplaceTypeName = "<None>";
		}
		//purposes
		if (!GetID(nFindPurposeID, m_findPurpose)) {
			nFindPurposeID = -1;			
		}
		if (!GetID(nReplacePurposeID, m_replacePurpose)) {
			nReplacePurposeID = -1;
		}
		if (!GetName(strFindPurposeName, m_findPurpose)) {
			strFindPurposeName = "<None>";
		}
		if (!GetName(strReplacePurposeName, m_replacePurpose)) {
			strReplacePurposeName = "<None>";
		}
			
		if ((nFindTypeID != nReplaceTypeID) || (nFindPurposeID != nReplacePurposeID)) {
			_RecordsetPtr rsAudit = CreateRecordset("SELECT AppointmentsT.ID, PersonT.ID as PatID, First, Middle, Last "
				" FROM AppointmentsT "
				" LEFT JOIN AppointmentPurposeT ON AppointmentPurposeT.AppointmentID = AppointmentsT.ID "
				" LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
				" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE AptTypeID %s AND AppointmentPurposeT.PurposeID %s", strFindType, strFindPurpose);
			FieldsPtr fldsAudit = rsAudit->Fields;
			while (! rsAudit->eof) {
				
				CString strPersonName = AdoFldString(fldsAudit, "Last", "") + ", " + AdoFldString(fldsAudit, "First", "");
				long nRecordID = AdoFldLong(fldsAudit, "ID");

				long nPatID = AdoFldLong(fldsAudit, "PatID");
				
				if (nFindTypeID != nReplaceTypeID) {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(nPatID, strPersonName, nAuditTransactionID, aeiApptType, nRecordID, strFindTypeName, strReplaceTypeName, aepMedium, aetChanged);
				}

				if (nFindPurposeID != nReplacePurposeID) {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(nPatID, strPersonName, nAuditTransactionID, aeiApptPurpose, nRecordID, strFindPurposeName, strReplacePurposeName, aepMedium, aetChanged);
				}

				rsAudit->MoveNext();
			}
		}

		//now for the batch item
		if (nAuditTransactionID != -1) {
			//we know something got audited
			if (nFindTypeID != nReplaceTypeID) {
				//there were some types
				AuditEvent(-1, "", nAuditTransactionID, aeiSchedReplaceUtilTypes, -1, strFindTypeName, strReplaceTypeName, aepMedium, aetChanged);
			}

			if (nFindPurposeID != nReplacePurposeID) {
				//there were some purposes
				AuditEvent(-1, "", nAuditTransactionID, aeiSchedReplaceUtilPurposes, -1, strFindPurposeName, strReplacePurposeName, aepMedium, aetChanged);
			}
		}


		ExecuteSqlBatch(strSqlBatch);
		if (nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

		// (c.haag 2007-02-23 09:20) - PLID 24897 - We no longer use temp tables
		//ExecuteSql("DROP TABLE #AptTypeTemp");
		CDialog::OnOK();
	}NxCatchAllCall("Could not replace appointments", 
		if (nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

void CProcedureReplaceDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CProcedureReplaceDlg::OnSelChosenFindType(long nRow) 
{
	RequeryFindPurpose();
}

void CProcedureReplaceDlg::OnSelChosenReplaceType(long nRow) 
{
	RequeryReplacePurpose();
}
