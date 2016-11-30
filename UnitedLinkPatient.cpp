// UnitedLinkPatient.cpp: implementation of the CUnitedLinkPatient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "practice.h"
#include "UnitedLinkPatient.h"
#include "ExportDuplicates.h"
#include "GlobalDataUtils.h"
#include "GlobalUtils.h"
#include "WellnessDataUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnitedLinkPatient::CUnitedLinkPatient(CWnd* pParent, _ConnectionPtr& pConPractice, _ConnectionPtr& pConRemote, CString strRemoteDataPath, CString strRemotePassword)
: CGenericLinkPatient(pParent, pConPractice, pConRemote, strRemoteDataPath, "United", strRemotePassword)
{
	m_strLocalLinkIDField = "UnitedID";
	SetLocalSQL("SELECT TOP 1 First, Middle, Last, SocialSecurity, BirthDate, FirstContactDate, Gender, Address1, Address2, City, State, Zip, HomePhone, Fax, UserDefinedID FROM PersonT INNER JOIN PatientsT on PatientsT.PersonID = PersonT.ID");
	SetRemoteSQL("SELECT TOP 1 First, Middle, Last, SSN, BirthDate, DateOfContact, [M/F], Address1, Address2, City, State, Zip, DayPhone, Fax, uExternalID FROM tblPatient",
		40, 50, 60, 11, 0, 0, 0, 30, 50, 30, 2, 16, 20, 20); // Field sizes
}

CUnitedLinkPatient::~CUnitedLinkPatient()
{

}

//
// CUnitedLinkPatient::AddToRemoteDatabase
//
// Assumes that the patient name does not exist
//
// Returns the remote table ID, or -1 on failure.
//
// This function, though United specific, has a generic flow of logic. I don't
// think it's impossible to have CGenericLinkPatient functionality here.
//
DWORD CUnitedLinkPatient::AddToRemoteDatabase(DWORD dwPracticeID, _RecordsetPtr &prsPractice, DWORD& dwRemoteID)
{
	// (c.haag 2006-03-09 09:41) - PLID 19635 - We now write to the United database using more direct ADO
	// functions rather than executing a query.

	//CString strSQL;	
	//_RecordsetPtr prs(__uuidof(Recordset));
	//_variant_t vIndex;
	//int nFields;

	try {
		_RecordsetPtr prsUnited(__uuidof(Recordset));
		FieldsPtr fPrac = prsPractice->Fields;
		FieldsPtr fUnited;
		BOOL bValid = TRUE;
		long nFields = fPrac->GetCount();
		CString str;
		CString strFirst = AdoFldString(fPrac, "First", "");
		CString strMiddle = AdoFldString(fPrac, "Middle", "");
		CString strLast = AdoFldString(fPrac, "Last", "");
		CString strSSN = AdoFldString(fPrac, "SocialSecurity", "");
		long i;

		//
		// Make sure first and last names aren't blank
		//
		if (strFirst.IsEmpty() || strLast.IsEmpty())
		{
			MsgBox("You must have a First and Last name to export to United. Cannot export patient: %s %s.",
				strFirst, strLast);
			return 1;
		}

		// (c.haag 2006-03-10 10:10) - PLID 19652 - It seems that, for new patients, a blank strSSN is
		// actually "           ". We need to trim it, otherwise United Imaging will complain about
		// duplicate strSSN's with values of "           "
		//
		strSSN.TrimRight();

		//
		// Make sure there is not a duplicate SSN
		//
		if (strSSN.GetLength() > 0) {
			str.Format("SELECT ID FROM tblPatient WHERE SSN = '%s'", strSSN);
			prsUnited->Open((LPCTSTR)str, _variant_t((IDispatch *)m_pConRemote, true), adOpenStatic, adLockOptimistic, adCmdText);
			if (!prsUnited->eof) {
				MsgBox("Practice was unable to add %s %s because the social security number '%s' already exists in the United database",
					strFirst, strLast, strSSN);
				bValid = FALSE;
			}
			prsUnited->Close();
			if (!bValid) return 1;
		}

		//
		// Validate all of the field lengths. Do this by traversing the fields list
		// in Practice and comparing the size of each field with m_adwRemoteFieldSizes.
		//
		for (i=0; i < nFields && bValid; i++) {
			_variant_t var = fPrac->Item[i]->Value;
			if (VT_BSTR == var.vt) {
				str = VarString(var, "");
				if (m_adwRemoteFieldSizes[i] > 0 && (DWORD)str.GetLength() > m_adwRemoteFieldSizes[i]) {	
					CString strField = (LPCTSTR)fPrac->Item[i]->Name;
					if (strField == "First") strField = "First Name";
					if (strField == "Middle") strField = "Middle Name";
					if (strField == "Last") strField = "Last Name";
					MsgBox("The %s field in Practice for %s %s is too large to export. The patient will not be sent to United.",
						strField, strFirst, strLast);
					bValid = FALSE;
				}			
			}
		}
		if (!bValid) return 1;


		//
		// Now we can write the data
		//
		str.Format("SELECT * from tblPatient WHERE 1=0");
		prsUnited->Open((LPCTSTR)str, _variant_t((IDispatch *)m_pConRemote, true), adOpenDynamic, adLockOptimistic, adCmdText);
		prsUnited->AddNew();
		fUnited = prsUnited->Fields;
		fUnited->Item["First"]->Value = fPrac->Item["First"]->Value;
		fUnited->Item["Middle"]->Value = fPrac->Item["Middle"]->Value;
		fUnited->Item["Last"]->Value = fPrac->Item["Last"]->Value;
		if (!strSSN.IsEmpty()) fUnited->Item["SSN"]->Value = _bstr_t(strSSN);
		fUnited->Item["BirthDate"]->Value = fPrac->Item["BirthDate"]->Value;
		fUnited->Item["DateOfContact"]->Value = fPrac->Item["FirstContactDate"]->Value;
		switch (AdoFldByte(fPrac, "Gender")) {
			case 1: fUnited->Item["M/F"]->Value = "M";
				break;
			case 2: fUnited->Item["M/F"]->Value = "F";
				break;
			default:
				break;
		}
		fUnited->Item["Address1"]->Value = fPrac->Item["Address1"]->Value;
		fUnited->Item["Address2"]->Value = fPrac->Item["Address2"]->Value;
		fUnited->Item["City"]->Value = fPrac->Item["City"]->Value;
		fUnited->Item["State"]->Value = fPrac->Item["State"]->Value;
		fUnited->Item["Zip"]->Value = fPrac->Item["Zip"]->Value;
		fUnited->Item["DayPhone"]->Value = fPrac->Item["HomePhone"]->Value;
		fUnited->Item["Fax"]->Value = fPrac->Item["Fax"]->Value;
		if (GetPropertyInt("UnitedLinkIDToUnitedID", 1)) {
			fUnited->Item["uExternalID"]->Value = fPrac->Item["UserDefinedID"]->Value;
		}
		prsUnited->Update();

		//
		// Pull the United ID and assign it to our corresponding record
		//
		dwRemoteID = AdoFldLong(fUnited, "ID");
		prsUnited->Close();

		// (c.haag 2006-04-06 10:20) - PLID 19635 - This can fail if United was
		// opened before Practice (that's my running theory, anyways). If this
		// happens, we need to pull the new ID in a, I would say, less proper way.
		//
		if (0 == dwRemoteID) {
			_RecordsetPtr prsID(__uuidof(Recordset));
			CString strSql;
			strSql.Format("SELECT Max(ID) AS MaxID FROM tblPatient WHERE First = '%s' AND Middle = '%s' AND Last = '%s' AND SSN %s",
				_Q(strFirst),
				_Q(strMiddle),
				_Q(strLast),
				(strSSN.IsEmpty()) ? "IS NULL" : (CString("= '") + _Q(strSSN) + "'"));
			prsID->Open((LPCTSTR)strSql, _variant_t((IDispatch *)m_pConRemote, true), adOpenDynamic, adLockOptimistic, adCmdText );
			if (prsID->eof || (0 == (dwRemoteID = AdoFldLong(prsID, "MaxID", 0)))) {
				AfxThrowNxException("Could not calculate the ID of the new patient");
			}
		}

		str.Format("UPDATE PatientsT SET UnitedID = %d WHERE PersonID = %d", dwRemoteID, dwPracticeID);
		m_pConPractice->Execute(_bstr_t(str), NULL, adCmdText);


		/*
		if (GetPropertyInt("UnitedLinkIDToUnitedID", 1))
		{
			strSQL = "INSERT INTO tblPatient ([First], Middle, [Last], SSN, BirthDate, DateOfContact, [M/F], Address1, Address2, City, State, Zip, DayPhone, Fax, uExternalID) VALUES (";
			nFields = fields->GetCount() - 1;
		}
		else
		{
			strSQL = "INSERT INTO tblPatient ([First], Middle, [Last], SSN, BirthDate, DateOfContact, [M/F], Address1, Address2, City, State, Zip, DayPhone, Fax) VALUES (";
			nFields = fields->GetCount() - 2;
		}

		///////////////////////////////////////////////////////////
		// Add the patient to the remote database
		
		// Build an INSERT INTO SQL statement. The - 1 is because
		// the last field is the record's remote ID (UnitedID),
		// which is NULL at this point.
		//
		for (int i=0; i < nFields; i++)
		{
			CString strField;
			_variant_t var;

			vIndex = (long)i;

			strField = (LPCTSTR)fields->Item[vIndex]->Name;
			var = fields->Item[vIndex]->Value;
			
			switch (var.vt)
			{
			case VT_BSTR:
				str = var.bstrVal;

				// Check to see if the string length exceeds the remote
				// field size. If so, pop up a message and run away. If
				// an entry in the remote field sizes array is 0, that
				// means don't check the field size.
				if (m_adwRemoteFieldSizes[i] > 0 &&
					(DWORD)str.GetLength() > m_adwRemoteFieldSizes[i])
				{
					CString strFirst, strLast;

					if (strField == "First") strField = "First Name";
					if (strField == "Middle") strField = "Middle Name";
					if (strField == "Last") strField = "Last Name";

					str.Format("SELECT First, Last FROM PersonT WHERE ID = %d", dwPracticeID);
					prs->Open((LPCTSTR)str, _variant_t((IDispatch *)m_pConPractice, true),
						adOpenStatic, adLockOptimistic, adCmdText);

					strFirst = prs->Fields->Item["First"]->Value.bstrVal;
					strLast = prs->Fields->Item["Last"]->Value.bstrVal;

					MsgBox("The %s field in Practice for %s %s is too large to export. The patient will not be sent to United.",
						strField, strFirst, strLast);

					prs->Close();
					return 1;
				}

				if ((strField == "First" || strField == "Last") && str.GetLength() == 0)
				{
					CString strFirst, strLast;

					// you cannot export if the first or last name is blank

					str.Format("SELECT First, Last FROM PersonT WHERE ID = %d", dwPracticeID);
					prs->Open((LPCTSTR)str, _variant_t((IDispatch *)m_pConPractice, true),
						adOpenStatic, adLockOptimistic, adCmdText);

					strFirst = prs->Fields->Item["First"]->Value.bstrVal;
					strLast = prs->Fields->Item["Last"]->Value.bstrVal;

					MsgBox("You must have a First and Last name to export to United. Cannot export patient: %s %s.",strFirst,strLast);

					prs->Close();
					return 1;
				}

				if (strField == "SocialSecurity")
				{
					CString strTmp;

					// Find out if there is an existent SSN in the remote
					// database
					strTmp.Format("SELECT ID FROM tblPatient WHERE SSN = '%s'",
						str);
					prs->Open((LPCTSTR)strTmp, _variant_t((IDispatch *)m_pConRemote, true),
						adOpenStatic, adLockOptimistic, adCmdText);
					if (!prs->eof)
					{
						CString strFirst, strLast;

						// We have a key violation because there is a matching SSN
						// in the remote database
						prs->Close();

						str.Format("SELECT First, Last FROM PersonT WHERE ID = %d", dwPracticeID);
						prs->Open((LPCTSTR)str, _variant_t((IDispatch *)m_pConPractice, true),
							adOpenStatic, adLockOptimistic, adCmdText);

						strFirst = prs->Fields->Item["First"]->Value.bstrVal;
						strLast = prs->Fields->Item["Last"]->Value.bstrVal;

						MsgBox("Practice was unable to add %s %s because the social security number already exists in the remote database",
							strFirst, strLast);

						prs->Close();
						return 1;
					}
					prs->Close();

					strTmp = str;

					strTmp.Remove(' ');
					strTmp.Remove('-');	//DRT 10/16 - Sometimes we could really have an empty SSN, but we still have -'s with the spaces
					if (strTmp.GetLength() == 0)
						str = "NULL";
					else
						str = CString("'") + _Q(str) + "'";
				}
				else {
					str = CString("'") + _Q(str) + "'";
				}
				break;

			case VT_DATE:
				str = CString("#") + CStringOfVariant(var) + "#";
				break;

			case VT_EMPTY:
			case VT_NULL:
				str = "NULL";
				break;

			default:
				if (strField == "Gender")
				{
					switch (var.bVal) // Assumes gender is 'bVal' type
					{
					case 1: str = "'M'"; break;
					case 2: str = "'F'"; break;
					default: str = "NULL"; break;
					}
				}
				else
					str = CStringOfVariant(var);
			}

			if (i < nFields - 1)
				strSQL += str + ", ";
			else
				strSQL += str + ")";
		}

		// Execute the statement
		m_pConRemote->Execute(_bstr_t(strSQL), NULL, adCmdText);

		////////////////////////////////////////////////////////
		// Get the United ID from the new record
		// in the remote database.
		//
		// TODO: Move the code to, and use GetMaxRemoteID. Hmmmmmmm,
		// I wonder how often we will use GetMaxRemoteID? Maybe we don't
		// need it after all?
		//
		prs->Open("SELECT Max(ID) FROM tblPatient", _variant_t((IDispatch *)m_pConRemote, true),
			adOpenStatic, adLockOptimistic, adCmdText);
		vIndex = (long)0;
		dwRemoteID = prs->Fields->Item[vIndex]->Value.lVal;
		prs->Close();

		//////////////////////////////////////////////
		// Update the ID in Practice
		strSQL.Format("UPDATE PatientsT SET UnitedID = %d WHERE PersonID = %d",
			dwRemoteID, dwPracticeID);

		m_pConPractice->Execute(_bstr_t(strSQL), NULL, adCmdText);
		*/

		return 0;
	}
	NxCatchAll("Error in CUnitedLinkPatient::AddToRemoteDatabase");
	return 1;
}

DWORD CUnitedLinkPatient::LinkExistingToRemote(DWORD dwPracticeID, DWORD dwRemoteID)
{
	try {
		CString strSQL;
		strSQL.Format("UPDATE PatientsT SET UnitedID = %d WHERE PersonID = %d",
			dwRemoteID, dwPracticeID);
		m_pConPractice->Execute(_bstr_t(strSQL), NULL, adCmdText);
		return 0;
	}
	NxCatchAll("CUnitedLinkPatient::LinkExistingToRemote");
	return 1;
}

DWORD CUnitedLinkPatient::UpdateRemote(DWORD dwPracticeID, _RecordsetPtr &prsPractice)
{
	FieldsPtr fields = prsPractice->Fields;
	CString strSQL = "UPDATE tblPatient SET ";
	CString str;
	CString strSSN, strFirst, strLast;
	_variant_t vIndex;

	try {

		///////////////////////////////////////////////////////////
		// Update the patient in the remote database
		
		for (int i=0; i < fields->GetCount() - 1; i++)
		{
			CString strField;
			_variant_t var;

			vIndex = (long)i;

			strField = m_astrRemoteFields[i];

			var = fields->Item[vIndex]->Value;
			
			switch (var.vt)
			{
			case VT_BSTR:
				str = var.bstrVal;

				if (strField == "First") strFirst = str;
				else if (strField == "Last") strLast = str;

				// Check to see if the string length exceeds the remote
				// field size. If so, pop up a message and run away. If
				// an entry in the remote field sizes array is 0, that
				// means don't check the field size.
				if (m_adwRemoteFieldSizes[i] > 0 &&
					(DWORD)str.GetLength() > m_adwRemoteFieldSizes[i])
				{
					_RecordsetPtr prs(__uuidof(Recordset));

					if (strField == "First") strField = "First Name";
					if (strField == "Middle") strField = "Middle Name";
					if (strField == "Last") strField = "Last Name";

					str.Format("SELECT First, Last FROM PersonT WHERE ID = %d", dwPracticeID);
					prs->Open((LPCTSTR)str, _variant_t((IDispatch *)m_pConPractice, true),
						adOpenStatic, adLockOptimistic, adCmdText);

					strFirst = prs->Fields->Item["First"]->Value.bstrVal;
					strLast = prs->Fields->Item["Last"]->Value.bstrVal;

					MsgBox("The %s field in Practice for %s %s is too large to export. The patient will not be sent to United.",
						strField, strFirst, strLast);

					prs->Close();
					return 1;
				}

				if (str.IsEmpty() && (strField == "First" || strField == "Last"))
				{
					str = " ";
				}

				if (strField == "SSN")
				{
					CString strTmp = str;
					strTmp.Remove(' ');
					strTmp.Remove('#');
					strTmp.TrimRight();
					if (strTmp.GetLength() == 0)
					{
						str = "NULL";
						strSSN.Empty();
					}
					else
					{
						str = CString("'") + _Q(strTmp) + "'";
						strSSN = strTmp;
					}
				}
				else {
					str = CString("'") + _Q(str) + "'";
				}
				break;

			case VT_DATE:
				str = CString("#") + CStringOfVariant(var) + "#";
				break;

			case VT_EMPTY:
			case VT_NULL:
				str = "NULL";
				break;

			default:
				if (strField == "M/F")
				{
					switch (var.bVal) // Assumes gender is 'bVal' type
					{
					case 1: str = "'M'"; break;
					case 2: str = "'F'"; break;
					default: str = "NULL"; break;
					}
				}
				else
					str = CStringOfVariant(var);
			}

			if (i < fields->GetCount() - 2)
				strSQL += CString("[") + strField + "] = " + str + ", ";
			else
				strSQL += CString("[") + strField + "] = " + str;
		}

		vIndex = (long)i;
		str.Format(" WHERE ID = %d", VarLong(fields->Item[vIndex]->Value));
		strSQL += str;

		// Look for duplicate SSN's
		// (c.haag 2006-03-29 11:01) - PLID 19911 - Don't look if the SSN is blank.
		// By this point in time, we had already removed the #'s and the whitespace.
		if (strSSN.GetLength()) {
			CString strSSNDups;
			strSSNDups.Format("SELECT First, Last FROM tblPatient WHERE SSN = '%s' AND ID <> %d",
				strSSN, VarLong(fields->Item[vIndex]->Value));
			_RecordsetPtr prsSSNDups = m_pConRemote->Execute(_bstr_t(strSSNDups), NULL, adCmdText);
			if (!prsSSNDups->eof)
			{
				MsgBox("The patient '%s, %s' could not be updated because the patient '%s, %s' has a matching social security number",
					strLast, strFirst, AdoFldString(prsSSNDups, "Last", ""),
					AdoFldString(prsSSNDups, "First", ""));
				return 1;
			}
		}

		//
		// Run the update
		//
		m_pConRemote->Execute(_bstr_t(strSQL), NULL, adCmdText);

		return 0;
	}
	NxCatchAll("Error in CUnitedLinkPatient::UpdateRemote");
	return 1;
}

DWORD CUnitedLinkPatient::Import(DWORD dwRemoteID, DWORD& dwPracticeID, DWORD& dwUserDefinedID, BOOL& bStop, EGenericLinkPersonAction& action)
{
	_RecordsetPtr prs(__uuidof(Recordset));
	CString strSQL = "SELECT ", str;
	CString strFirst, strMiddle, strLast;
	FieldsPtr fields;
	DWORD dwRes = 0;

	//////////////////////////////////////////////////////////////
	// Build the local SQL string based on the field array
	if (m_astrRemoteFields.GetSize() == 0)
	{
		// TODO: throw a huge huge error!!
		return 1;
	}
	if (m_pConRemote == NULL)
	{
		AfxMessageBox("The remote database could not be opened. The import cannot continue.");
		return 1;
	}
	if (m_pConPractice == NULL)
	{
		AfxMessageBox("The local database could not be opened. The import cannot continue.");
		return 1;
	}

	try {
		// See if we already have a remote ID for this record.
		str.Format("SELECT PersonID, UserDefinedID FROM PatientsT WHERE UnitedID = %d",
			dwRemoteID);

		prs->Open((LPCTSTR)str, _variant_t((IDispatch *)m_pConPractice, true),
			adOpenStatic, adLockOptimistic, adCmdText);
		if (!prs->eof)
		{
			dwPracticeID = prs->Fields->Item["PersonID"]->Value.lVal;
			dwUserDefinedID = prs->Fields->Item["UserDefinedID"]->Value.lVal;
			action = EUpdatedRecord;
		}
		else
			action = EAddedRecord;

		prs->Close();

		// Build our input SQL
		for (int i=0; i < m_astrLocalFields.GetSize(); i++)
		{
			strSQL += CString("[") + m_astrRemoteFields[i] + "], ";
		}
		str.Format("%sID FROM tblPatient WHERE ID = %d",
			strSQL, dwRemoteID);
		strSQL = str;

		//////////////////////////////////////////////////////////////
		// Open the remote recordset
		prs->Open((LPCTSTR)str, _variant_t((IDispatch *)m_pConRemote, true),
			adOpenStatic, adLockOptimistic, adCmdText);
		if(prs->eof) {
			//DRT 3/29/2006 - PLID 19909 - The person no longer exists in United!  In this case, we just have
			//	to give up immediately.
			MsgBox("The patient you are trying to import (United ID %li) has been deleted from the United database.  Please refresh the link and try again.", dwRemoteID);
			bStop = TRUE;
			action = ENone;
			return 1;
		}

		fields = prs->Fields;

		if (action == EUpdatedRecord)
		{
			// Hey! The record already exists! Lets do a HotSync!
			// TODO: Message box saying we're updating this patient
			// in the remote database
			dwRes = UpdateLocal(dwPracticeID, prs);
		}
		else
		{
			CExportDuplicates dlg(m_pParent);
			strFirst = AdoFldString(fields, "First");
			strMiddle = "";//AdoFldString(fields, "Middle");
			strLast = AdoFldString(fields, "Last");

			if(dlg.FindDuplicates(strFirst,strLast,strMiddle, "United", "Practice", m_pConRemote, m_strRemoteDataPath))
			{
				switch (dlg.DoModal())
				{
				case 1: // Add new
					dwRes = AddToLocalDatabase(dwRemoteID, prs, dwPracticeID, dwUserDefinedID);
					action = EAddedRecord;
					break;
				case 2: // Update the practice database to link this record.						
					// (c.haag 2011-04-21) - PLID 43351 - Assign dwPracticeID
					dwPracticeID = dlg.m_varIDToUpdate.lVal;
					if (dwRes = LinkExistingToRemote(dwPracticeID, dwRemoteID)) break;					
					dwRes = UpdateLocal(dwPracticeID, prs);
					action = EUpdatedRecord;
					break;
				case 3: // Skip
					action = ENone;
					break;
				case 4: // Stop
					bStop = TRUE;
					break;
				}
			}
			else {
				dwRes = AddToLocalDatabase(dwRemoteID, prs, dwPracticeID, dwUserDefinedID);
				action = EAddedRecord;
			}
		}
		prs->Close();
		prs.Detach();
		return dwRes;
	}
	NxCatchAll("Error in CUnitedLinkPatient::Import");
	return 1;
}

DWORD CUnitedLinkPatient::UpdateLocal(DWORD dwPracticeID, _RecordsetPtr &prsRemote)
{
	FieldsPtr fields = prsRemote->Fields;
	CString strSQL = "UPDATE PersonT SET ";
	CString str;
	_variant_t vIndex;
	CString strUserDefinedID;
	CString strFirst, strLast;	// Used for a user warning if necessary

	try {

		///////////////////////////////////////////////////////////
		// Update the patient in the local database
		
		for (int i=0; i < fields->GetCount() - 1; i++)
		{
			CString strField;
			_variant_t var;

			vIndex = (long)i;

			strField = m_astrLocalFields[i];
			var = fields->Item[vIndex]->Value;

			// This is a very unpleasant exception because UserDefinedID does not
			// exist in PersonT. 
			if (strField == "UserDefinedID")
			{
				strUserDefinedID = CString(var.bstrVal);
				continue;
			}
			else if (strField == "First")
				strFirst = var.bstrVal;
			else if (strField == "Last")
				strLast = var.bstrVal;

			if (strField == "FirstContactDate" && (var.vt == VT_NULL ||
				var.vt == VT_EMPTY))
			{
				var = COleDateTime::GetCurrentTime();
			}

			switch (var.vt)
			{
			case VT_BSTR:
				str = var.bstrVal;

				if (strField == "Gender")
				{
					if (str == "M") str = "1";
					else if (str == "F") str = "2";
					else str = "''";
				}
				else {
					str = CString("'") + _Q(str) + "'";
				}
				break;

			case VT_DATE:
				str = CString("'") + CStringOfVariant(var) + "'";
				break;

			case VT_EMPTY:
			case VT_NULL:
				if (strField == "SocialSecurity" || strField == "Address2" || strField == "Fax" ||
					strField == "Middle" || strField == "Address1" || strField == "City" ||
					strField == "State" || strField == "HomePhone")
				{
					str = "''";
				}
				else if (strField == "Gender")
				{
					str = "0";
				}
				else
					str = "NULL";
				break;

			default:
				str = CStringOfVariant(var);
			}

			if (i < fields->GetCount() - 2)
				strSQL += CString("[") + strField + "] = " + str + ", ";
			else
				strSQL += CString("[") + strField + "] = " + str;
		}

		///////////////////////////////////////////////////////////////
		// We need to take out the trailing comma because we skipped
		// the user-defined ID field
		strSQL = strSQL.Left(strSQL.GetLength() - 2);

		// Ok wrap the SQL statement up
		vIndex = (long)i;
		str.Format(" WHERE PersonT.ID = %d", dwPracticeID);
		strSQL += str;


		///////////////////////////////////////////////////////////////
		// Prevent user defined ID duplication
		if (GetPropertyInt("UnitedLinkIDToUnitedID", 1))
		{
			if (!strUserDefinedID.IsEmpty())
			{
				_RecordsetPtr rsDup = CreateRecordset("SELECT TOP 1 UserDefinedID FROM PatientsT WHERE UserDefinedID = %s AND PersonID <> %d", strUserDefinedID,
					dwPracticeID);
				if (!rsDup->eof)
				{
					MsgBox("There is another patient in NexTech Practice whose ID matches the United Imaging patient's new ID of (%s). The import for this patient cannot continue.", 
						strUserDefinedID, strFirst, strLast);
					return 1;
				}
			}
		}

		///////////////////////////////////////////////////////////////
		// Execute the statements
		_RecordsetPtr prsOldInfo = CreateRecordset("SELECT UserDefinedID, [First], [Last] FROM PatientsT INNER JOIN PersonT ON PersonT.ID = PatientsT.PersonID WHERE PersonT.ID = %d",
			dwPracticeID);
		if(prsOldInfo->eof) {
			//DRT 3/29/2006 - PLID 19909
			//We may have had an occurrence of this being EOF, meaning that someone deleted the patient while it was being imported.
			//	Seems unlikely, but definitely possible.  That has to happen sometime after the user clicks "Import Patient",
			//	if you do it beforehand, it will just add new.
			//If this does happen, we'll just pop up a message saying that the patient was deleted while the import took place, 
			//	and to refresh your link.
			MsgBox("The patient (%s %s) you were attempting to update has been deleted (possibly by another user).  The import did not take place.  Please "
				"refresh the United Link and try to import the patient again.", strFirst, strLast);
			return 1;
		}

		CString strOldUserDefinedID;
		CString strOldLast = AdoFldString(prsOldInfo, "Last");
		CString strOldFirst = AdoFldString(prsOldInfo, "First");
		strOldUserDefinedID.Format("%d", AdoFldLong(prsOldInfo, "UserDefinedID"));
		prsOldInfo->Close();

		// PersonT update
		m_pConPractice->Execute(_bstr_t(strSQL), NULL, adCmdText);

		// Now we have to update PatientsT
		if (GetPropertyInt("UnitedLinkIDToUnitedID", 1))
		{
			if (!strUserDefinedID.IsEmpty())
			{

				strSQL.Format("UPDATE PatientsT SET UserDefinedID = %s WHERE PersonID = %d", strUserDefinedID, dwPracticeID);
				m_pConPractice->Execute(_bstr_t(strSQL), NULL, adCmdText);
			}
			else
			{
				MsgBox("The ID for patient %s %s is empty in United; Practice will not import that specific field.",
					strFirst, strLast);
				strUserDefinedID = strOldUserDefinedID;
			}
		}	
		// Now we update the patient's history folder
		EnsureCorrectHistoryFolder(strOldUserDefinedID, strOldFirst, strOldLast, dwPracticeID);
		return 0;
	}
	NxCatchAll("Error in CUnitedLinkPatient::UpdateLocal");
	return 1;
}

DWORD CUnitedLinkPatient::AddToLocalDatabase(DWORD dwRemoteID, _RecordsetPtr &prsRemote, DWORD& dwPracticeID, DWORD& dwUserDefinedID)
{
	FieldsPtr fields = prsRemote->Fields;
	// (j.jones 2010-01-15 09:01) - PLID 31927 - supported defaulting the text message privacy field
	CString strSQL = "INSERT INTO PersonT ([First], Middle, [Last], SocialSecurity, BirthDate, FirstContactDate, Gender, Address1, Address2, City, State, Zip, HomePhone, Fax, [ID], Location, TextMessage) VALUES (";
	CString str;
	_RecordsetPtr prs(__uuidof(Recordset));
	_variant_t vIndex;
	CString strFirst, strLast;
	CString strRemoteUserDefinedID;

	try {

		///////////////////////////////////////////////////////////
		// Add the remote patient to the local database
		
		// Build an INSERT INTO SQL statement. The - 1 is because
		// the last field is the record ID in the remote database.
		//
		for (int i=0; i < fields->GetCount() - 1; i++)
		{
			CString strField;
			_variant_t var;

			vIndex = (long)i;

			strField = (LPCTSTR)fields->Item[vIndex]->Name;
			var = fields->Item[vIndex]->Value;

			// This is a very unpleasant exception because UserDefinedID does not
			// exist in PersonT. 
			if (strField == "uExternalID")
			{
				strRemoteUserDefinedID = CString(var.bstrVal);
				continue;
			}
			else if (strField == "First")
				strFirst = var.bstrVal;
			else if (strField == "Last")
				strLast = var.bstrVal;

			if (strField == "DateOfContact" && (var.vt == VT_NULL ||
				var.vt == VT_EMPTY))
			{
				var = COleDateTime::GetCurrentTime();
			}

			switch (var.vt)
			{
			case VT_BSTR:
				str = var.bstrVal;

				if (strField == "M/F")
				{
					if (str == "M") str = "1";
					else if (str == "F") str = "2";
					else str = "0";
				}
				else {
					str = CString("'") + _Q(str) + "'";
				}
				break;

			case VT_DATE:
				str = CString("'") + CStringOfVariant(var) + "'";
				break;

			case VT_EMPTY:
			case VT_NULL:
				if (strField == "SSN" || strField == "Address1" || strField == "Address2" || 
					strField == "City" || strField == "State" || strField == "Fax" ||
					strField == "DayPhone" || strField == "M/F" || strField == "Middle")
					str = "''";
				else
					str = "NULL";
				break;

			default:
				str = CStringOfVariant(var);
			}

			strSQL += str + ", ";
		}

		/////////////////////////////////////////////////////////////////////////
		// We have our SQL statement. Now lets figure out our remote ID
		if (!strRemoteUserDefinedID.IsEmpty())
			dwUserDefinedID = atoi(strRemoteUserDefinedID);
		else
		{
			MsgBox("The ID for patient %s %s is empty in United; Practice will not import that specific field.",
				strFirst, strLast);
			dwUserDefinedID = NewNumber("PatientsT", "UserDefinedID");

			// Now we have to update the external ID in United.
			str.Format("UPDATE tblPatient SET uExternalID = '%d' WHERE ID = %d",
				dwUserDefinedID, dwRemoteID);
			
			// Execute the statement
			m_pConRemote->Execute(_bstr_t(str), NULL, adCmdText);
		}

		// Make sure we have no duplicates
		_RecordsetPtr rsDup = CreateRecordset("SELECT TOP 1 UserDefinedID FROM PatientsT WHERE UserDefinedID = %d", 
			dwUserDefinedID);
		if (!rsDup->eof)
		{
			MsgBox("There is another patient in NexTech Practice whose ID matches the United patient ID (%d). The import for this patient cannot continue.",
				dwUserDefinedID, strFirst, strLast);
			return 1;
		}

		// (j.jones 2010-01-15 09:02) - PLID 31927 - check the default text message privacy field
		long nTextMessagePrivacy = GetRemotePropertyInt("NewPatientsDefaultTextMessagePrivacy", 0, 0, "<None>", true);

		/////////////////////////////////////////////////////////////////////////
		// Now we actually do the export
		// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
		CSqlTransaction trans("United patient import");
		trans.Begin();

		// Add the record in PersonT
		dwPracticeID = NewNumber("PersonT", "ID");
		// (j.jones 2010-01-15 09:01) - PLID 31927 - supported defaulting the text message privacy field
		str.Format("%d, %d, %li)", dwPracticeID, GetCurrentLocationID(), nTextMessagePrivacy);
		strSQL += str;
		m_pConPractice->Execute(_bstr_t(strSQL), NULL, adCmdText);

		// Add the record in PatientsT
		strSQL.Format("INSERT INTO PatientsT (PersonID, UserDefinedID, CurrentStatus, UnitedID) VALUES (%d, %d, 1, %d)",
			dwPracticeID, dwUserDefinedID, dwRemoteID);
		m_pConPractice->Execute(_bstr_t(strSQL), NULL, adCmdText);

		// (j.armen 2014-01-28 16:47) - PLID 60146 - Set Default Security Group
		ExecuteParamSql(m_pConPractice, 
				"DECLARE @SecurityGroupID INT\r\n"
				"SET @SecurityGroupID = (SELECT IntParam FROM ConfigRT WHERE Username = '<None>' AND Name = 'DefaultSecurityGroup')\r\n"
				"IF (ISNULL(@SecurityGroupID, -1) <> -1) AND EXISTS(SELECT ID FROM SecurityGroupsT WHERE ID = @SecurityGroupID)\r\n"
				"BEGIN\r\n"
				"	INSERT INTO SecurityGroupDetailsT (SecurityGroupID, PatientID) VALUES (@SecurityGroupID, {INT})\r\n"
				"END\r\n", dwPracticeID);

		// (b.cardillo 2009-05-28 16:46) - PLIDs 34368 and 34369 - We just created a patient, so all wellness qualification needs to be updated
		UpdatePatientWellnessQualification_NewPatient(GetRemoteData(), dwPracticeID);

		trans.Commit();
		
		// Declare Miller Time
		// Miller Time access denied
		return 0;
	}
	NxCatchAll("Error in CUnitedLinkPatient::AddToLocalDatabase");
	return 1;
}