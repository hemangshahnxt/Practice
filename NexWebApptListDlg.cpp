// NexWebApptListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practicerc.h"
#include "NexWebApptListDlg.h"
#include "NexWebImportDlg.h"
#include "NexWebApptInfoDlg.h"
#include "GlobalSchedUtils.h"
#include "CommonSchedUtils.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CNexWebApptListDlg dialog


CNexWebApptListDlg::CNexWebApptListDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexWebApptListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNexWebApptListDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// (j.gruber 2007-02-23 11:21) - PLID 24722 - clean up the memory leaks
CNexWebApptListDlg::~CNexWebApptListDlg() {

	long nCount = m_ApptList.GetSize();
	for (int i = nCount-1; i >= 0; i--) {
		ApptImport *pCurAppt = m_ApptList.GetAt(i);
		// (j.jones 2014-11-26 14:57) - PLID 64169 - clear our insurance placements
		::ClearAppointmentInsuranceMap(pCurAppt->mapInsPlacements);
		delete pCurAppt;
		m_ApptList.RemoveAt(i);
	}
}

void CNexWebApptListDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexWebApptListDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexWebApptListDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNexWebApptListDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CNexWebApptListDlg::SetPersonID(long nPersonID, BOOL bIsNewPatient) {

	m_nPersonID = nPersonID;
	m_bIsNewPatient = bIsNewPatient;

}


/////////////////////////////////////////////////////////////////////////////
// CNexWebApptListDlg message handlers

BOOL CNexWebApptListDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	// (j.jones 2014-11-14 14:32) - PLID 64169 - added AutoFillApptInsurance and AutoFillApptInsurance_DefaultCategory
	g_propManager.CachePropertiesInBulk("CNexWebApptListDlg", propNumber,
		"(Username = '<None>' OR Username = '%s') AND Name IN ( "
		"	'AutoFillApptInsurance', "
		"	'AutoFillApptInsurance_DefaultCategory' "
		")"
		, _Q(GetCurrentUserName()));
	
	m_pApptList = BindNxDataListCtrl(this, IDC_NEXWEB_APPT_LIST, GetRemoteData(), FALSE);

	LoadApptList();	


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CString CNexWebApptListDlg::GenerateResourceString(long nApptID) {

	try {

		//loop through the resource array for this appt and generate the string
		ApptImport *pImport;
		for (int i = 0; i < m_ApptList.GetSize(); i++) {
			ApptImport *pCurAppt = m_ApptList.GetAt(i);
			if (pCurAppt->nID == nApptID) {
				pImport = pCurAppt;
			}
		}
		if (pImport->dwResourceList.GetSize() == 0) {
			return "";
		}

		CString strIDs;
		strIDs = "(";
		for (i = 0; i < pImport->dwResourceList.GetSize(); i++) {
			strIDs += AsString((long)pImport->dwResourceList.GetAt(i)) + ",";
		}

		//take off the last comma
		strIDs = strIDs.Left(strIDs.GetLength() - 1);
		strIDs +=")";

		CString strResources = "";
		_RecordsetPtr rs = CreateParamRecordset("SELECT Item From ResourceT WHERE ID IN ({INTARRAY})", pImport->dwResourceList);
		while (! rs->eof) {
			strResources += AdoFldString(rs, "Item") + ",";
			 
			rs->MoveNext();
		}

		//take off the last comma
		strResources = strResources.Left(strResources.GetLength() - 1);

		//return it
		return strResources;
	}NxCatchAllCall("Error in CNexWebApptListDlg::GenerateResourceString", return "");

}

// (j.gruber 2006-11-13 11:37) - PLID 23154 - save the purpose right away
void CNexWebApptListDlg::GeneratePurposeArray(long nApptID, CDWordArray *pdw, BOOL bIsNewAppt) {

	try {
		_RecordsetPtr rs;
		if (bIsNewAppt) {
			rs = CreateParamRecordset("SELECT Value, Field FROM NexwebTransactionsT WHERE ObjectID = {INT} AND Field IN (3004,3005) GROUP BY Value, Field ORDER BY FIELD ASC", nApptID);

			long nFieldID;
			long nValue;
			while (! rs->eof) {
				nFieldID = AdoFldLong(rs, "Field");
				nValue = atoi(AdoFldString(rs, "Value"));

				switch (nFieldID) {
					case 3004: //add Purpose
						pdw->Add(nValue);
					break;

					case 3005: //remove Purpose
						for (int i = 0; i < pdw->GetSize(); i++) {
							if (((long)pdw->GetAt(i)) == nValue) {
								pdw->RemoveAt(i);
							}
						}
					break;
				}
				rs->MoveNext();
			}
		}
		else {
			rs = CreateParamRecordset("SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = {INT}", nApptID);
			while (! rs->eof) {
				pdw->Add(AdoFldLong(rs, "PurposeID"));

				rs->MoveNext();
			}
		}
		
	}NxCatchAll("Error In CNexWebApptListDlg::GeneratePurposeArray");

}

void CNexWebApptListDlg::GenerateResourceArray(long nApptID, CDWordArray *pdw) {

	try {
		CDWordArray dwResources;
		_RecordsetPtr rs = CreateParamRecordset("SELECT Value, Field FROM NexwebTransactionsT WHERE ObjectID = {INT} AND Field IN (3006,3007) GROUP BY Value, Field ORDER BY FIELD ASC", nApptID);
		if (rs->eof) {
			// (j.gruber 2006-11-13 11:28) -  PLID 23444 they either didn't put in a resource for this appt which is impossible unless they manually edited the xml file, or its an existing appt.
			//let's assume the latter and generate our own list
			rs = CreateParamRecordset("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = {INT}", nApptID);
			if (rs->eof) {
				//well, this seems to be an error condition, an appt with no resources
				ASSERT(FALSE);
			}
			else {
				while (! rs->eof) {
					pdw->Add(AdoFldLong(rs, "ResourceID"));

					rs->MoveNext();
				}
			}
		}
		else {

			long nFieldID;
			long nValue;
			while (! rs->eof) {
				nFieldID = AdoFldLong(rs, "Field");
				nValue = atoi(AdoFldString(rs, "Value"));

				switch (nFieldID) {
					case 3006: //add resource
						pdw->Add(nValue);
					break;

					case 3007: //remove resource
						for (int i = 0; i < pdw->GetSize(); i++) {
							if (((long)pdw->GetAt(i)) == nValue) {
								pdw->RemoveAt(i);
							}
						}
					break;
				}
				rs->MoveNext();
			}
		}
		
	}NxCatchAll("Error In CNexWebApptListDlg::GenerateResourceArray");

}


void CNexWebApptListDlg::GenerateApptList() {

	//loop through the values in the datalist and fill our appointment list
	long p = m_pApptList->GetFirstRowEnum();
	LPDISPATCH lpDisp = NULL;
	while (p) {
		m_pApptList->GetNextRowEnum(&p, &lpDisp);
		IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
		
		ApptImport * pApptImport = new ApptImport;
		pApptImport->nID = VarLong(pRow->GetValue(0));
		pApptImport->bSelected = VarBool(pRow->GetValue(1));
		pApptImport->dtApptDate = VarDateTime(pRow->GetValue(2));
		pApptImport->dtStartTime = VarDateTime(pRow->GetValue(3));
		pApptImport->dtEndTime = VarDateTime(pRow->GetValue(4));
		GenerateResourceArray(VarLong(pRow->GetValue(0)), &pApptImport->dwResourceList);
		// (j.gruber 2007-02-23 09:53) - PLID 24720 - could throw bad variable type errors
		pApptImport->strNotes = VarString(pRow->GetValue(7), "");
		pApptImport->bHasBeenSaved = FALSE;
		pApptImport->bIsNew = VarBool(pRow->GetValue(8));
		pApptImport->nLocationID = VarLong(pRow->GetValue(9));
		// (j.gruber 2006-11-13 10:01) - PLID 23154 - add the rest of the fields to the array so they can save
		pApptImport->nTypeID = VarLong(pRow->GetValue(10), -1);
		GeneratePurposeArray(VarLong(pRow->GetValue(0)), &pApptImport->dwPurposeList, VarBool(pRow->GetValue(8)));
		pApptImport->bMoveUp = VarBool(pRow->GetValue(11), FALSE) == 0 ? FALSE : TRUE;
		pApptImport->nStatus = VarByte(pRow->GetValue(12));

		// (j.jones 2014-11-14 14:31) - PLID 64169 - try to load the patient's insurance information
		if (m_nPersonID != -1 && m_nPersonID != -25) {
			::TryAutoFillAppointmentInsurance(m_nPersonID, pApptImport->mapInsPlacements);
		}
		
		m_ApptList.Add(pApptImport);
	}
}

void CNexWebApptListDlg::LoadApptList() {


	try {
		//First get all the appts that they added for this patient
		// (j.gruber 2006-11-13 14:39) - PLID 23154 - added some fields to the query
		_RecordsetPtr rs = CreateParamRecordset("SELECT ObjectID, (SELECT Top 1 CONVERT(datetime, Value) FROM NexwebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND NTrans2.PersonID = NexWEbTransactionsT.PersonID AND Field = 3012 ORDER BY ID DESC) AS ApptDate, "
			"  (SELECT Top 1 CONVERT(datetime, Value) From NexWebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND NTrans2.PersonID = NexWEbTransactionsT.PersonID AND Field = 3009 ORDER BY ID DESC) AS StartTime, "
			" (SELECT Top 1 CONVERT(datetime, Value) From NexWebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND NTrans2.PersonID = NexWEbTransactionsT.PersonID AND Field = 3010 ORDER BY ID DESC) AS EndTime,  "
			" (SELECT Name FROM AptTypeT WHERE ID = (SELECT Top 1 Value From NexWebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND NTrans2.PersonID = NexWEbTransactionsT.PersonID AND Field = 3003 ORDER BY ID DESC)) AS ApptType, "
			" (SELECT Item FROM ResourceT WHERE ID = (SELECT Top 1 Value From NexWebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND NTrans2.PersonID = NexWEbTransactionsT.PersonID AND Field = 3006 ORDER BY ID DESC)) AS Resource, "
			" (SELECT Top 1 Value From NexWebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND NTrans2.PersonID = NexWEbTransactionsT.PersonID AND Field = 3008 ORDER BY ID DESC) AS Note, "
			" (SELECT Top 1 Convert(int, Value) From NexWebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND NTrans2.PersonID = NexWEbTransactionsT.PersonID AND Field =  3016 ORDER BY ID DESC) AS LocationID,  "
			" (SELECT top 1 Convert(int, Value) FROM NexWebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND nTrans2.PersonID = NexwebTransactionsT.PersonID AND Field = 3003 ORDER BY ID DESC) AS TypeID, "
			" (SELECT top 1 Convert(bit, Value) FROM NexWebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND nTrans2.PersonID = NexwebTransactionsT.PersonID AND Field = 3013 ORDER BY ID DESC) AS MoveUp, "
			" (SELECT top 1 Convert(tinyint, Value) FROM NexWebTransactionsT NTrans2 WHERE NTrans2.ObjectID = NexwebTransactionsT.ObjectID AND nTrans2.PersonID = NexwebTransactionsT.PersonID AND Field = 3014 ORDER BY ID DESC) AS Cancelled, "
			" 1 AS IsNew FROM NexWebTransactionsT  "
			" WHERE Field = 3001 AND PersonID = {INT} ", m_nPersonID);
		CString strApptDate, strStartTime, strEndTime, strType, strResource, strNote;
		long nApptID;
		COleDateTime dtDate, dtStartTime, dtEndTime;
		const _variant_t varFalse(VARIANT_FALSE, VT_BOOL);
		const _variant_t varTrue(VARIANT_TRUE, VT_BOOL);
		//loop through the records and get the other values out that are used in the datalist
		while (!rs->eof) {

			nApptID = AdoFldLong(rs, "ObjectID");
				
			strType = AdoFldString(rs, "ApptType", "");
			strResource = "";
			strNote = AdoFldString(rs, "Note", "");

			//add the record to the datalist
			IRowSettingsPtr pRow = m_pApptList->GetRow(-1);
			pRow->PutValue(0, nApptID);
			pRow->PutValue(1, varFalse);
			pRow->PutValue(2, rs->Fields->Item["ApptDate"]->Value);
			pRow->PutValue(3, rs->Fields->Item["StartTime"]->Value);
			pRow->PutValue(4, rs->Fields->Item["EndTime"]->Value);
			pRow->PutValue(5, _variant_t(strType));
			pRow->PutValue(6, _variant_t(strResource));
			pRow->PutValue(7, _variant_t(strNote));
			pRow->PutValue(8, varTrue);
			pRow->PutValue(9, AdoFldLong(rs, "LocationID", -1));
			// (j.gruber 2006-11-13 14:39) - PLID 23154 - save these in the datalist
			pRow->PutValue(10, AdoFldLong(rs, "TypeID", -1));
			_variant_t varMoveUp = rs->Fields->Item["MoveUp"]->Value;
			if (varMoveUp.vt == VT_BOOL ) {
				if (varMoveUp.boolVal) {
					pRow->PutValue(11, varTrue);
				}
				else {
					pRow->PutValue(11, varFalse);
				}
			}
			else {
				pRow->PutValue(11, varFalse);
			}
			pRow->PutValue(12, AdoFldByte(rs, "Cancelled", 0));
			

			m_pApptList->AddRow(pRow);

			rs->MoveNext();
		}
		
		//now do the existing items
		//we have to do it separately because you can't have an order by in a subquery with a union
		// (j.gruber 2006-11-13 14:39) - PLID 23154 - added some fields to the query
		// (j.gruber 2006-11-13 14:39) - PLID 23143 - fixed the where clause
		// (d.moore 2007-05-22 16:11) - PLID 4013 - Joined on the WaitingList table to replace the original MoveUp functionality.
		rs = CreateParamRecordset(
			"SELECT AppointmentsT.ID, AppointmentsT.Date, AppointmentsT.StartTime, AppointmentsT.EndTime, "
				"AptTypeT.Name, dbo.GetResourceString(AppointmentsT.ID) AS Resource, "
				"AppointmentsT.Notes, 0 AS IsNew, AppointmentsT.LocationID, "
				"CONVERT(bit, CASE WHEN ISNULL(WaitingListT.ID, 0) > 0 THEN 1 ELSE 0 END) AS MoveUp, "
				"AppointmentsT.Status, AppointmentsT.AptTypeID "
			"FROM "
			"AppointmentsT LEFT JOIN AptTypeT ON  AppointmentsT.AptTypeID = AptTypeT.ID "
			"LEFT JOIN WaitingListT ON AppointmentsT.ID = WaitingListT.AppointmentID "
			"WHERE AppointmentsT.ID IN "
				"(SELECT ObjectID FROM NexWebTransactionsT WHERE Field > 3000 AND Field < 4000 "
				"AND PersonID = {INT} AND ObjectID NOT IN "
					"(SELECT ObjectID FROM NexWebTransactionsT WHERE Field = 3001)) "
			"GROUP BY AppointmentsT.ID, AppointmentsT.Date, AppointmentsT.StartTime, AppointmentsT.EndTime, AptTypeT.Name, "
				"AppointmentsT.Notes, AppointmentsT.LocationID, WaitingListT.ID, AppointmentsT.Status, AppointmentsT.AptTypeID ",
			m_nPersonID);

		while (! rs->eof) {

			nApptID = AdoFldLong(rs, "ID");
			// (j.gruber 2007-02-23 11:11) - PLID 24720 - fix bad varialbe type error
			strType = AdoFldString(rs, "Name", "");
			strResource = AdoFldString(rs, "Resource");
			// (j.gruber 2007-02-23 09:52) - PLID 24720 - could produce bad varialbe type errror
			strNote = AdoFldString(rs, "Notes", "");

			//add the record to the datalist
			IRowSettingsPtr pRow = m_pApptList->GetRow(-1);
			pRow->PutValue(0, nApptID);
			pRow->PutValue(1, varFalse);
			pRow->PutValue(2, rs->Fields->Item["Date"]->Value);
			pRow->PutValue(3, rs->Fields->Item["StartTime"]->Value);
			pRow->PutValue(4, rs->Fields->Item["EndTime"]->Value);
			pRow->PutValue(5, _variant_t(strType));
			pRow->PutValue(6, _variant_t(strResource));
			pRow->PutValue(7, _variant_t(strNote));
			pRow->PutValue(8, varFalse);
			pRow->PutValue(9, AdoFldLong(rs, "LocationID"));
			// (j.gruber 2006-11-13 14:39) - PLID 23154 - added some fields to the query
			pRow->PutValue(10, AdoFldLong(rs, "AptTypeID", -1));
			_variant_t varMoveUp = rs->Fields->Item["MoveUp"]->Value;
			if (varMoveUp.vt == VT_BOOL ) {
				if (varMoveUp.boolVal) {
					pRow->PutValue(11, varTrue);
				}
				else {
					pRow->PutValue(11, varFalse);
				}
			}
			else {
				pRow->PutValue(11, varFalse);
			}
			pRow->PutValue(12, rs->Fields->Item["Status"]->Value);
			
			m_pApptList->AddRow(pRow);

			rs->MoveNext();
		}

		//generate the appointment list based on the datalist
		GenerateApptList();

		//now go through and put in the resource string for all of the rows
		long p = m_pApptList->GetFirstRowEnum();
		LPDISPATCH lpDisp = NULL;
		while (p) {
			m_pApptList->GetNextRowEnum(&p, &lpDisp);
			IRowSettingsPtr pRow(lpDisp); lpDisp->Release();

			CString strResource = GenerateResourceString(VarLong(pRow->GetValue(0)));

			pRow->PutValue(6, _variant_t(strResource));
		}


	}NxCatchAll("Error Loading Appointment List");

}
BEGIN_EVENTSINK_MAP(CNexWebApptListDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNexWebApptListDlg)
	ON_EVENT(CNexWebApptListDlg, IDC_NEXWEB_APPT_LIST, 3 /* DblClickCell */, OnDblClickCellNexwebApptList, VTS_I4 VTS_I2)
	ON_EVENT(CNexWebApptListDlg, IDC_NEXWEB_APPT_LIST, 4 /* LButtonDown */, OnLButtonDownNexwebApptList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CNexWebApptListDlg, IDC_NEXWEB_APPT_LIST, 10 /* EditingFinished */, OnEditingFinishedNexwebApptList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CNexWebApptListDlg::OnDblClickCellNexwebApptList(long nRowIndex, short nColIndex) 
{
	//open up the editing dialog

	//first get the ID for this appt and see if it is new or not
	long nApptID = VarLong(m_pApptList->GetValue(nRowIndex, 0));
	BOOL bIsNew = VarBool(m_pApptList->GetValue(nRowIndex, 8));
	ApptImport * pAppt;
	for (int i = 0; i < m_ApptList.GetSize(); i++) {
		ApptImport *pCurAppt = m_ApptList.GetAt(i);
		if (pCurAppt->nID == nApptID) {
			pAppt = pCurAppt;
		}
	}

	CNexWebApptInfoDlg dlg(nApptID, bIsNew, m_nPersonID, pAppt, this);
	dlg.DoModal();
	
	//refresh the list for this appt
	RefreshList(pAppt);

	
}

void CNexWebApptListDlg::RefreshList(ApptImport *pAppt) {

	try {

		long nRow = m_pApptList->FindByColumn(0, pAppt->nID, 0, FALSE);
		IRowSettingsPtr pRow = m_pApptList->GetRow(nRow);

		if (pRow) {

			pRow->PutValue(2, _variant_t(FormatDateTimeForInterface(pAppt->dtApptDate, NULL, dtoDate, FALSE)));
			pRow->PutValue(3, _variant_t(FormatDateTimeForInterface(pAppt->dtStartTime, NULL, dtoTime, FALSE)));
			pRow->PutValue(4, _variant_t(FormatDateTimeForInterface(pAppt->dtEndTime, NULL, dtoTime, FALSE)));
			CString strType;
			_RecordsetPtr rsType = CreateParamRecordset("SELECT Name FROM AptTypeT WHERE ID = {INT}", pAppt->nTypeID);
			if (!rsType->eof) {
				pRow->PutValue(5, _variant_t(AdoFldString(rsType, "Name")));
			}
			CString strResource = GenerateResourceString(pAppt->nID);
			pRow->PutValue(6, _variant_t(strResource));
			pRow->PutValue(7, _variant_t(pAppt->strNotes));
			
		}
	}NxCatchAll("Error In CNexWebApptListDlg::RefreshList()");

}


BOOL CNexWebApptListDlg::CheckExistingApptData() {

	try {
		//first see if this patient has anything to save/change in their appts
		_RecordsetPtr rsAppts = CreateParamRecordset("SELECT * FROM NexWebTransactionsT WHERE PersonID = {INT} "
			" AND Field > 3000 AND Field < 4000", m_nPersonID);
		if (rsAppts->eof) {
			//hooray!! There is nothing to do!! //let them continue saving the rest of their stuff
			return TRUE;
		}

		long nCount = 0;
		for (int i=0; i < m_ApptList.GetSize(); i++) {
			ApptImport *pCurAppt = m_ApptList.GetAt(i);
			if (pCurAppt->bSelected) {
				nCount++;
			}
		}
		
		//ok, so they have appt infomation, lets look at our list of ones they want to save and 
		//make sure that they even know they are here
		if (nCount == 0) {
			//give them a message box that either no appt information is going to be imported
			// or they need to check some boxes
			if (IDYES == MsgBox(MB_YESNO, "There is appointment information in NexWeb for this patient, but no appointment information "
				"has been selected to be imported.  Are you sure you wish to import no appointment information for this patient?")) {
				//glorious day!! we have nothing to do!!
				//let them keep saving
				return TRUE;
			}
			else {
				//tell them how to get to the information
				MsgBox("Please click on the Appointments Tab to review the information added through NexWeb for this patient.  "
					"\nTo select information to be imported, check the box in the left hand column of the list");
				//stop them from saving so they can fix this
				return FALSE;
			}
		}

		//they filled out their stuff, let them keep saving
		return TRUE;
	}NxCatchAll("Error In NexWebApptListDlg::CheckExistingApptData()");
	
	return FALSE;
}

BOOL CNexWebApptListDlg::ValidateData() {

	//loop through the list of appt information and validate each one
	BOOL bValidate = TRUE;
	APPT_INFO pAppt;
	pAppt.pmapInsInfo = NULL;

	for (int i = 0; i < m_ApptList.GetSize(); i++) {

		ApptImport *pCurAppt = m_ApptList.GetAt(i);
		
		if (pCurAppt->bSelected) {

			// (j.gruber 2006-11-13 10:26) - PLID 23154 - don't let them add it if it is a changed appt and they haven't checked it
			if (pCurAppt->bIsNew == 0) {

				//check if they have opened the editor
				if (pCurAppt->bHasBeenSaved == 0) {
					CString strError;
					strError.Format("Appointment On %s At %s is an existing appointment that was changed through NexWeb, please double click on it and save the changes before importing it.", FormatDateTimeForInterface(pCurAppt->dtApptDate, NULL, dtoDate, FALSE),
					FormatDateTimeForInterface(pCurAppt->dtStartTime, NULL, dtoTime, FALSE));
					m_strError += strError;
					bValidate = FALSE;
					return bValidate;
				}
			}

			pAppt.dtApptDate = pCurAppt->dtApptDate;
			pAppt.dtStartTime = pCurAppt->dtStartTime;
			pAppt.dtEndTime = pCurAppt->dtApptDate;
			pAppt.dtArrivalTime = pCurAppt->dtStartTime;
			pAppt.dwPurposeList = &pCurAppt->dwPurposeList;
			pAppt.dwResourceList = &pCurAppt->dwResourceList;
			pAppt.nApptID = -1;
			//pending
			pAppt.nAptInitStatus = 1;
			pAppt.nAptTypeID = pCurAppt->nTypeID;
			pAppt.nLocationID = pCurAppt->nLocationID;
			//we don't have a patientID yet...unless its an existing patient
			pAppt.nPatientID = m_nPersonID;
			pAppt.nShowStateID = -1;

			// (j.jones 2014-11-14 14:31) - PLID 64169 - get the patient's insurance information			
			pAppt.pmapInsInfo = &pCurAppt->mapInsPlacements;

			// (c.haag 2010-03-08 16:36) - PLID 37326 - Updated to use new parameters
			//TES 2/25/2011 - PLID 42523 - For nLineItemOverrideID, pass in -2 (unknown) rather than -1 (definitely not on a precision template).
			// (j.jones 2014-11-26 10:43) - PLID 64179 - added pParentWnd
			// (j.jones 2014-11-26 16:42) - PLID 64178 - Added an optional overriddenMixRules returned,
			// this is only filled if the appt. exceeded a scheduler mix rule and a user overrode it. The caller needs to save this info.
			std::vector<SchedulerMixRule> overriddenMixRules;
			SelectedFFASlotPtr pSelectedFFASlot;
			pSelectedFFASlot.reset();
			if (!ValidateAppt(this, &pAppt, NULL, TRUE, TRUE, FALSE, FALSE, FALSE, -2, TRUE, TRUE, overriddenMixRules, pSelectedFFASlot)) {
				CString strError;
				strError.Format("Appointment On %s At %s", FormatDateTimeForInterface(pCurAppt->dtApptDate, NULL, dtoDate, FALSE),
					FormatDateTimeForInterface(pCurAppt->dtStartTime, NULL, dtoTime, FALSE));
				m_strError += strError;
				bValidate = FALSE;
			}
			else {
				// (j.jones 2014-12-16 10:37) - PLID 64312 - if they overrode the appointment, we need to save
				// this information for when the appointment is saved
				pCurAppt->overriddenMixRules.clear();
				pCurAppt->overriddenMixRules.insert(pCurAppt->overriddenMixRules.end(), overriddenMixRules.begin(), overriddenMixRules.end());

				// (j.jones 2014-12-19 14:28) - PLID 64312 - if they picked a new slot, update the appointment info
				if (pSelectedFFASlot != NULL && pSelectedFFASlot->IsValid()) {
					pCurAppt->dtApptDate = AsDateNoTime(pSelectedFFASlot->dtStart);
					pCurAppt->dtStartTime = pSelectedFFASlot->dtStart;
					pCurAppt->dtEndTime = pSelectedFFASlot->dtEnd;
					pCurAppt->dwResourceList.RemoveAll();
					pCurAppt->dwResourceList.Append(pSelectedFFASlot->dwaResourceIDs);
					pCurAppt->nLocationID = pSelectedFFASlot->nLocationID; // (r.farnworth 2016-02-02 12:08) - PLID 68116 - FFA results transmit location to new appointment.

					long nRow = m_pApptList->FindByColumn(0, pCurAppt->nID, 0, FALSE);
					IRowSettingsPtr pRow = m_pApptList->GetRow(nRow);
					if (pRow) {
						pRow->PutValue(2, _variant_t(FormatDateTimeForInterface(pCurAppt->dtApptDate, NULL, dtoDate, FALSE)));
						pRow->PutValue(3, _variant_t(FormatDateTimeForInterface(pCurAppt->dtStartTime, NULL, dtoTime, FALSE)));
						pRow->PutValue(4, _variant_t(FormatDateTimeForInterface(pCurAppt->dtEndTime, NULL, dtoTime, FALSE)));

						CString strResource = GenerateResourceString(pCurAppt->nID);
						pRow->PutValue(6, _variant_t(strResource));

						// (r.farnworth 2016-02-10 09:12) - PLID 68116 - FFA results transmit location to new appointment.
						pRow->PutValue(9, _variant_t(pCurAppt->nLocationID));
					}
				}
			}
		}

	}

	return bValidate;
}

void CNexWebApptListDlg::GenerateAuditItem(SchedAuditItems &item, ApptImport *pAppt) {
	
	try {

		//first the old
		_RecordsetPtr rs = CreateParamRecordset("SELECT Item, ResourceID FROM AppointmentResourceT LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID WHERE AppointmentID = {INT}", pAppt->nID);
		while (!rs->eof)
		{
			item.aryOldResource.Add(AdoFldString(rs, "Item"));
			rs->MoveNext();
		}
		rs->Close();
		rs.Detach();

		rs = CreateParamRecordset("SELECT Name, PurposeID FROM AppointmentPurposeT LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID WHERE AppointmentID = {INT}", pAppt->nID);
		while (!rs->eof)
		{
			item.aryOldPurpose.Add(AdoFldString(rs, "Name"));
			rs->MoveNext();
		}
		rs->Close();
		rs.Detach();

		//now the new resources
		rs = CreateParamRecordset("SELECT Item From ResourceT WHERE ID IN ({INTARRAY})", pAppt->dwResourceList);
		while (! rs->eof) {
			item.aryNewResource.Add(AdoFldString(rs, "Item"));			 
			rs->MoveNext();
		}
		rs->Close();
		rs.Detach();

		//Purposes
		// (j.gruber 2006-11-13 11:50) - PLID 23444 - fixed if there are no purposes, don't run the recordset
		long nAptPurposeSize = pAppt->dwPurposeList.GetSize();
		if (nAptPurposeSize > 0) {
			rs = CreateParamRecordset("SELECT Name From AptPurposeT WHERE ID IN ({INTARRAY})", pAppt->dwPurposeList);
			while (! rs->eof) {
				item.aryNewPurpose.Add(AdoFldString(rs, "Name"));
			 
				rs->MoveNext();
			}
			rs->Close();
			rs.Detach();
		}		

	}NxCatchAll("Error in CNexWebApptListDlg::GenerateAuditItem");


}

BOOL CNexWebApptListDlg::SaveInfo(long nPersonID /*= -1*/) {

	BOOL bSuccess = TRUE;
	try {
		
		if (nPersonID == -1) {
			//that means we are using our personID
			nPersonID = m_nPersonID;
		}

		//loop through the appointments in the list and save the selected ones
		for (int i = 0; i < m_ApptList.GetSize(); i++) {
			ApptImport *pAppt = m_ApptList.GetAt(i);

			if (pAppt->bSelected) {

				long nApptID = pAppt->nID;

				if (pAppt->bIsNew) {

					// (j.jones 2014-11-26 14:59) - PLID 64169 - passed in the insurance placements
					nApptID = AppointmentCreate(nPersonID, pAppt->dwResourceList, pAppt->nLocationID, pAppt->dtApptDate,
						pAppt->dtStartTime, pAppt->dtEndTime, pAppt->dtStartTime, 0, pAppt->bMoveUp,
						0, pAppt->strNotes, pAppt->nTypeID, pAppt->dwPurposeList, FALSE, -1, FALSE, true, &pAppt->mapInsPlacements);
					
					if (nApptID == -1) {
						bSuccess = FALSE;
					}
					else {
						//cancel the appt if they did
						if (pAppt->nStatus == 4) {
							long nAuditID;
							CString strSql;
							bSuccess = AppointmentCancel(nApptID, FALSE, FALSE, FALSE, {}, FALSE, &strSql, &nAuditID, TRUE);
						}
						//they are only allowed to cancel appts from nexweb not estore new appts, I'm not sure if that is even possible
					}
				}
				else {
					//generate the old and new purpose and resource lists
					SchedAuditItems items;
					GenerateAuditItem(items, pAppt);
					bSuccess = AppointmentModify(nApptID, nPersonID, pAppt->dwResourceList, pAppt->nLocationID, pAppt->dtApptDate,
						pAppt->dtStartTime, pAppt->dtEndTime, pAppt->dtStartTime, 0, pAppt->bMoveUp,
						0, pAppt->strNotes, pAppt->nTypeID, pAppt->dwPurposeList, &items, FALSE, -1);

					if (pAppt->nStatus == 4) {
						long nAuditID;
						CString strSql;
						bSuccess = AppointmentCancel(nApptID, FALSE, FALSE, FALSE, {}, FALSE, &strSql, &nAuditID, TRUE);
					}
					else {

						// (j.gruber 2006-11-13 12:50) - PLID 23154
						//restore the appt if necessary
						//the only way this is possibile is if it was cancelled from the website and they restored it within the dialog
	
						//check to see if the appt is cancelled now
						_RecordsetPtr rs = CreateParamRecordset("SELECT Status FROM AppointmentsT WHERE ID = {INT}", nApptID);
						if (! rs->eof) {
							long nStatus = AdoFldByte(rs, "Status");
							if (nStatus == 4) {
								// (j.jones 2014-12-02 11:36) - PLID 64183 - added pParentWnd
								bSuccess = AppointmentUncancel(this, nApptID);
							}
						}
					}
				}

				// (j.jones 2014-12-16 10:37) - PLID 64312 - if they overrode the appointment, we need to save
				// this information now
				if (nApptID != -1 && pAppt->overriddenMixRules.size() > 0) {
					TrackAppointmentMixRuleOverride(nApptID, pAppt->overriddenMixRules);
				}
			}
		}

	}NxCatchAllCall("Error in CNexWebApptListDlg::SaveInfo", bSuccess = FALSE;);
	return bSuccess;


}

void CNexWebApptListDlg::OnLButtonDownNexwebApptList(long nRow, short nCol, long x, long y, long nFlags) 
{
	
}

void CNexWebApptListDlg::OnEditingFinishedNexwebApptList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	//we only care about the 1st column
	if (nCol != 1) {
		return;
	}

	//they checked the box!
	//get the ID from the first column
	long nID = VarLong(m_pApptList->GetValue(nRow, 0));

	//now set the value in the list
	for (int i = 0; i < m_ApptList.GetSize(); i++) {
		ApptImport *pCurAppt = m_ApptList.GetAt(i);
		if (pCurAppt->nID == nID) {
			pCurAppt->bSelected = VarBool(varNewValue);
		}
	}
	
}

void CNexWebApptListDlg::OnOK()
{
	//Do nothing, we're just a tab in a larger dialog.
}

void CNexWebApptListDlg::OnCancel()
{
	//Do nothing, we're just a tab in a larger dialog.
}