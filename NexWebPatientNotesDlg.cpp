// NexWebPatientNotesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "NexWebPatientNotesDlg.h"
#include "InternationalUtils.h"
#include "nexwebNoteChangeDlg.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CNexWebPatientNotesDlg dialog

using namespace ADODB;
CNexWebPatientNotesDlg::CNexWebPatientNotesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexWebPatientNotesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNexWebPatientNotesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_strError = "";
}


void CNexWebPatientNotesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexWebPatientNotesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexWebPatientNotesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNexWebPatientNotesDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexWebPatientNotesDlg message handlers


void CNexWebPatientNotesDlg::SetPersonID(long nPersonID, BOOL bIsNewPatient) {

	m_nPersonID = nPersonID;
	m_bIsNewPatient = bIsNewPatient;

}

BOOL CNexWebPatientNotesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
	m_pNoteList = BindNxDataList2Ctrl(this, IDC_NEXWEB_PAT_NOTES, NULL, FALSE);

	LoadNoteList();


	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNexWebPatientNotesDlg::LoadNoteList() {

	try {
		_RecordsetPtr rs = CreateRecordset("SELECT ObjectID, "
			" (SELECT Top 1 CONVERT(datetime, Value) FROM NexwebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND NTrans2.PersonID = NexWEbTransactionsT.PersonID AND Field = 2043 ORDER BY ID DESC) AS Date, "
			" (SELECT Top 1 Value FROM NexwebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND NTrans2.PersonID = NexWEbTransactionsT.PersonID AND Field = 2044 ORDER BY ID DESC) AS Note "
			" FROM NexWebTransactionsT WHERE Field = 2042 AND PersonID = %li", m_nPersonID);

		const _variant_t varFalse(VARIANT_FALSE, VT_BOOL);
		const _variant_t varTrue(VARIANT_TRUE, VT_BOOL);
		while (! rs->eof) {

			long nNoteID = AdoFldLong(rs, "ObjectID");
			COleDateTime dtDate = AdoFldDateTime(rs, "Date", COleDateTime::GetCurrentTime());
			CString strNote = AdoFldString(rs, "Note", "");


			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pNoteList->GetNewRow();

			pRow->PutValue(0, (long)nNoteID);
			pRow->PutValue(1, varFalse);
			pRow->PutValue(2, (long)1);
			pRow->PutValue(3, _variant_t("Added"));			
			pRow->PutValue(4, _variant_t(dtDate, VT_DATE));
			pRow->PutValue(5, (long)-1);
			pRow->PutValue(6, _variant_t(strNote));
			pRow->PutValue(7, varTrue);

			m_pNoteList->AddRowAtEnd(pRow, NULL);

			rs->MoveNext();
		}
		rs->Close();


		//now do the changed ones
		rs = CreateRecordset(" SELECT ID, "
			" CASE WHEN (SELECT top 1 Convert(datetime, value) FROM NexwebTransactionsT nTrans2 WHERE nTrans2.ObjectID = Notes.ID AND nTrans2.Field = 2043) IS NULL then Notes.Date "
			" ELSE (SELECT top 1 Convert(datetime, value) FROM NexwebTransactionsT nTrans2 WHERE nTrans2.ObjectID = Notes.ID AND nTrans2.Field = 2043 ORDER BY ID DESC) END as Date, "
			" Category,  "
			" CASE WHEN (SELECT top 1  value FROM NexwebTransactionsT nTrans2 WHERE nTrans2.ObjectID = Notes.ID AND nTrans2.Field = 2044) IS NULL then Notes.Note"
			" ELSE (SELECT top 1 value FROM NexwebTransactionsT nTrans2 WHERE nTrans2.ObjectID = Notes.ID AND nTrans2.Field = 2044 ORDER BY ID DESC) END as Note "
			" FROM Notes WHERE ID IN (SELECT ObjectID FROM NexwebTransactionsT WHERE Field IN (2043, 2044))"
			" AND ID NOT IN (SELECT ObjectID FROM NexwebTransactionsT WHERE Field = 2042) AND PersonID = %li", m_nPersonID);

		while (! rs->eof) {

			long nNoteID = AdoFldLong(rs, "ID");
			COleDateTime dtDate = AdoFldDateTime(rs, "Date", COleDateTime::GetCurrentTime());
			CString strNote = AdoFldString(rs, "Note", "");


			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pNoteList->GetNewRow();

			pRow->PutValue(0, (long)nNoteID);
			pRow->PutValue(1, varFalse);
			pRow->PutValue(2, (long)0);
			pRow->PutValue(3, _variant_t("Changed"));
			pRow->PutValue(4, _variant_t(dtDate, VT_DATE));
			pRow->PutValue(5, (long)-1);
			pRow->PutValue(6, _variant_t(strNote));
			pRow->PutValue(7, varFalse);

			m_pNoteList->AddRowAtEnd(pRow, NULL);

			rs->MoveNext();
		}
		rs->Close();

		//now fill in the category listing
		rs = CreateRecordset("SELECT ID, Description FROM NoteCatsF WHERE IsPatientTab = 1");
		CString strDelim;
		strDelim = "-1; <No Category>;";
		while (!rs->eof) {

			strDelim += AsString(AdoFldLong(rs, "ID")) + ";";
			strDelim += AdoFldString(rs, "Description") + ";";

			rs->MoveNext();
		}

		NXDATALIST2Lib::IColumnSettingsPtr pCol;
		pCol = m_pNoteList->GetColumn(5);
		pCol->PutComboSource((LPCTSTR)strDelim);
		

	}NxCatchAll("Error Loading Patient Notes");

}

BOOL CNexWebPatientNotesDlg::ValidateData() {

	//make sure the dates are valid the notes aren't too long
	BOOL bValid = TRUE;
	try {
		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pNoteList->GetFirstRow();
		while (pRow) {
	
			COleDateTime dt = VarDateTime(pRow->GetValue(4));
			COleDateTime dtMin;
			dtMin.SetDate(1800, 1, 1);

			if (dt < dtMin || dt.GetStatus() != 0) {
				CString strError;
				strError.Format("Invalid Date: %s\n", FormatDateTimeForInterface(dt));
				m_strError += strError;
				bValid = FALSE;
			}

			CString strNote = VarString(pRow->GetValue(6));
			if (strNote.GetLength() >  4000) {
				CString strError;
				strError.Format("Note Length exceeds 4000 characters for note: %s\n", strNote);
				m_strError += strError;
				bValid = FALSE;

			}

			pRow = pRow->GetNextRow();
		}
	}NxCatchAll("Error Validating Data");

	return bValid;
}


BOOL CNexWebPatientNotesDlg::SaveInfo(long nPersonID /*= -1*/) {

	try {
		long nPatID;
		if (nPersonID == -1) {
			nPatID = m_nPersonID;
		}
		else {
			nPatID = nPersonID;
		}

		// (c.haag 2007-03-06 15:10) - PLID 25073 - Get the person name for auditing. The other queries here
		// anticipate nPatID being a valid person ID, so throw an exception if none can be found
		_RecordsetPtr prsName = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS FullName FROM PersonT WHERE ID = %d", nPatID);
		CString strPersonName;
		if (prsName->eof) { // This should never happen
			ThrowNxException("Could not find patient to assign notes to!");
		}
		strPersonName = AdoFldString(prsName, "FullName"); // FullName can't be null, so don't assign a default
		prsName->Close();

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pNoteList->GetFirstRow();
		while (pRow) {

			//see if they want to import it
			BOOL bImport = VarBool(pRow->GetValue(1));
			if (bImport) {

				BOOL bIsNew = VarBool(pRow->GetValue(7));
				long nNoteID = VarLong(pRow->GetValue(0));
				CString strNote = VarString(pRow->GetValue(6), "");
				COleDateTime dt = VarDateTime(pRow->GetValue(4));
				long nCatID = VarLong(pRow->GetValue(5), -1);
				//(e.lally 2008-02-28) PLID 29143 - Make 'No category' IDs null instead of -1
				CString strCatID;
				if(nCatID == -1){
					strCatID = "NULL";
				}
				else{
					strCatID.Format("%li", nCatID);
				}

				if (bIsNew) {

					//Add a new note
					//(e.lally 2008-02-28) PLID 29143 - Make 'No category' IDs null instead of -1
					// (j.armen 2014-01-31 09:31) - PLID 60568 - Idenitate NoteDataT
					_RecordsetPtr prs = CreateRecordset(
						"SET NOCOUNT ON\r\n"
						"INSERT INTO Notes (PersonID, Note, Date, Category)\r\n"
						"	VALUES (%li, '%s', '%s', %s)\r\n"
						"SET NOCOUNT OFF\r\n"
						"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS NoteID",
						nPatID, _Q(strNote), FormatDateTimeForSql(dt), strCatID);

					long nNoteID = AdoFldLong(prs, "NoteID");

					// (c.haag 2007-03-06 14:32) - PLID 25073 - Audit the addition of the note in a manner consistent
					// with patients adding notes from the Notes tab. At the time of this writing, we do not include
					// the category since we currently don't audit note category changes.
					long nAuditID = BeginNewAuditEvent();
					AuditEvent(nPatID, strPersonName, nAuditID, aeiPatientNote, nNoteID, "", strNote, aepMedium, aetChanged);
				}
				else {
					_RecordsetPtr prsOldNote = CreateRecordset("SELECT [Note], [Date] FROM Notes WHERE ID = %d", nNoteID);
					if (prsOldNote->eof) { // This should never happen
						ThrowNxException("Could not find note for updating!");
					}
					CString strOldNote = AdoFldString(prsOldNote, "Note");
					CString strOldDate = FormatDateTimeForInterface(AdoFldDateTime(prsOldNote, "Date"), NULL, dtoNaturalDatetime, false);
					CString strNewDate = FormatDateTimeForInterface(dt, NULL, dtoNaturalDatetime, false);
					//(e.lally 2008-02-28) PLID 29143 - Make 'No category' IDs null instead of -1
					ExecuteSql("UPDATE Notes SET Date = '%s', Note = '%s', Category = %s WHERE ID = %li", 
						FormatDateTimeForSql(dt), _Q(strNote), strCatID, nNoteID);

					// (c.haag 2007-03-06 15:13) - PLID 25073 - Audit changing the date and note in a manner consistent
					// with patients changing notes from the Notes tab. At the time of this writing, we do not include
					// the category since we currently don't audit note category changes.
					if (strOldNote != strNote) {
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(nPatID, strPersonName, nAuditID, aeiPatientNote, nNoteID, strOldNote, strNote, aepMedium, aetChanged);
					}
					if (strOldDate != strNewDate) {
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(nPatID, strPersonName, nAuditID, aeiPatientNoteDate, nNoteID, strOldDate, strNewDate, aepMedium, aetChanged);
					}
				}
			}

			pRow = pRow->GetNextRow();
		}
	}NxCatchAllCall("Error Saving Notes", return FALSE;);

	return TRUE;
}

void CNexWebPatientNotesDlg::OnOK()
{
	//Do nothing, we're just a tab in a larger dialog.
}

void CNexWebPatientNotesDlg::OnCancel()
{
	//Do nothing, we're just a tab in a larger dialog.
}


BEGIN_EVENTSINK_MAP(CNexWebPatientNotesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNexWebPatientNotesDlg)
	ON_EVENT(CNexWebPatientNotesDlg, IDC_NEXWEB_PAT_NOTES, 3 /* DblClickCell */, OnDblClickCellNexwebPatNotes, VTS_DISPATCH VTS_I2)
	ON_EVENT(CNexWebPatientNotesDlg, IDC_NEXWEB_PAT_NOTES, 9 /* EditingFinishing */, OnEditingFinishingNexwebPatNotes, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CNexWebPatientNotesDlg, IDC_NEXWEB_PAT_NOTES, 10 /* EditingFinished */, OnEditingFinishedNexwebPatNotes, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CNexWebPatientNotesDlg::OnDblClickCellNexwebPatNotes(LPDISPATCH lpRow, short nColIndex) 
{
	//check to see what the statusID is
	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
	long nStatusID = VarLong(pRow->GetValue(2));


	if (nStatusID == 0) {

		long nNoteID = VarLong(pRow->GetValue(0));
		COleDateTime dt = VarDateTime(pRow->GetValue(4));
		CString strNote = VarString(pRow->GetValue(6));


		//its a changed record, so let's open up the dialog
		CNexWebNoteChangeDlg dlg(nNoteID, this);
		long nResult = dlg.DoModal();

		if (nResult == IDOK) {
			pRow->PutValue(4, _variant_t(dlg.m_dtDataDate, VT_DATE));
			pRow->PutValue(6, _variant_t(dlg.m_strDataNote));
		}
	}
	else {
		MsgBox("This note was added by NexWeb, not changed.");
	}

	
}

void CNexWebPatientNotesDlg::OnEditingFinishingNexwebPatNotes(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	
	
}

void CNexWebPatientNotesDlg::OnEditingFinishedNexwebPatNotes(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	// TODO: Add your control notification handler code here
	
}
