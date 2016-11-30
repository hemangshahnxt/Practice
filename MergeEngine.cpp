#include "stdafx.h"
#include <NxPracticeSharedLib/RichEditUtils.h>
#include "MergeEngine.h"
#include "LetterWriting.h"
#include "NxStandard.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "PracProps.h"
#include "MsgBox.h"
#include "WhereClause.h"
#include "GlobalDataUtils.h"
#include "DoesExist.h"
#include "FileUtils.h"
#include <CxImage/ximage.h> // (a.walling 2013-05-08 16:15) - PLID 56610 - ximage.h now in CxImage/
#include "SelectSenderDlg.h"

#include "HistoryDlg.h"

#include "InternationalUtils.h"
#include "DateTimeUtils.h"

#include "HistoryUtils.h"

#include "SureScriptsPractice.h"
#include "DontShowDlg.h"

#include "ProgressMgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 13:55) - PLID 28000 - VS2008 - Moved this after all the header inclusions
using namespace ADODB;

CString GetPatientDocumentPath(long nPatientID);

//CString ConvertToHeaderName(const CString &strPrefix, const CString &strHeaderBaseText, OPTIONAL OUT BOOL *pbTruncated = NULL);

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Merge Engine Class
/////////////////////////////////////////////////////////////////////

CMergeEngine::CMergeEngine()
{
	// Set reasonable defaults for parameters
	m_nFlags = BMS_DEFAULT;
	m_strOrderBy = "";
	m_strSender = "";
	m_strSenderFirst = "";
	m_strSenderMiddle = "";
	m_strSenderLast = "";
	m_strSenderEmail = "";
	m_strSenderTitle = "";
	m_strSubjectMatter = ""; 
	// CAH 4/16/01
	m_lstExtraFields.RemoveAll();

	// Clear out implementation variables
	m_aryFieldList.RemoveAll();
	m_strResultsT = _T("");
	m_strGroupListQ = _T("");
	m_strGroupListSql = _T("");

	m_pfnCallbackExtraFields = NULL;
	m_pCallbackParam = NULL;

	m_nMailBatchId = -1;

	m_nCategoryID = -1;

	m_nPicID = -1;
	m_nEMNID = -1;

	m_nMergeFieldCountLimit = -1;
	m_bDroppedFieldsPastLimit = FALSE;

}

CMergeEngine::~CMergeEngine()
{
	// Temp objects (tables and queries) are deleted automatically by m_omTempObjects destructor

	// Delete temporary files
	for (long i=0; i < m_astrTempFiles.GetSize(); i++)
		DeleteFile(m_astrTempFiles[i]);
}

// (c.haag 2012-05-07) - PLID 49951 - Returns the location image path
CString CMergeEngine::GetLocationImagePath()
{
	return ::GetLocationImagePath();
}

// (c.haag 2012-05-07) - PLID 49951 - Returns the contact image path
CString CMergeEngine::GetContactImagePath()
{
	return ::GetContactImagePath();
}

// (c.haag 2012-05-07) - PLID 49951 - Returns the current location ID
int CMergeEngine::GetCurrentLocationID()
{
	return ::GetCurrentLocationID();
}

// (c.haag 2012-05-07) - PLID 49951 - Returns non-zero if we're running from internal
bool CMergeEngine::IsNexTechInternal()
{
	return ::IsNexTechInternal();
}

// (c.haag 2012-05-07) - PLID 49951 - Returns a new ID from the mail batch table.
int CMergeEngine::GetNewMailBatchID()
{
	return NewNumber("MailSent", "MailBatchID");
}

// (c.haag 2012-05-07) - PLID 49951 - Returns the patient SSN visibility mask
long CMergeEngine::GetPatientSSNMask()
{
	BOOL bSSNReadPermission = CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE);
	BOOL bSSNDisableMasking = CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE);
	return ((bSSNReadPermission && bSSNDisableMasking) ? -1 : (bSSNReadPermission && !bSSNDisableMasking) ? 0 : 1);
}

// (c.haag 2012-05-07) - PLID 49951 - Returns true if patient NexWeb security codes can be visible
bool CMergeEngine::ShowNexWebSecurityCode()
{
	return (GetCurrentUserPermissions(bioPatientNexWebLogin) & (sptRead | sptReadWithPass)) ? true : false;
}

//// (c.haag 2012-05-07) - PLID 49951 - Returns true if patient billing merge data can be visible
bool CMergeEngine::ShowBillingMergeData()
{
	return (GetCurrentUserPermissions(bioBillingMerge) & SPT__R_________ANDPASS) ?  true : false;
}

// (c.haag 2012-05-07) - PLID 49951 - Format procedure rich text fields
void CMergeEngine::FormatProcedureRtfFields(CString& strProcInfoQuery)
{
	_RecordsetPtr prs = CreateRecordset("SELECT * FROM %s", strProcInfoQuery);
	long nRecords = prs->GetRecordCount();
	// (S.Dhole 09/17/2012) - PLID 52680  
	long iCurRecord = 0;
	DWORD dwTickCount = GetTickCount();
	while (!prs->eof)
	{
		//m.hancock - 12/1/2005 - PLID 18478 - Some RTF merge fields should retain their formatting.
		//ConvertToRTFField now requires a boolean parameter, bRetainFormatting, to designate RTF data that utilizes
		//its own font attributes.  If the parameter is true, the data will use its formatting when merged to the
		//template rather than font attributes applied to the merge field on the template.  I've updated each of the
		//calls below to pass "true" when ConvertToRTFField is called because these fields are all for NexForms content,
		//which utilize their own font attributes.
		long nProcInfoID = AdoFldLong(prs, "ProcInfoDetailID");
		// (S.Dhole 09/17/2012) - PLID 52680   Passing dwTickCount 
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "CustomSection10", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "CustomSection1", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "CustomSection2", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "CustomSection3", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "CustomSection4", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "CustomSection5", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "CustomSection6", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "CustomSection7", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "CustomSection8", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "CustomSection9", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "MiniDescription", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "Preop", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "TheDayOf", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "PostOp", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "Recovery",nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "ProcDetails", nProcInfoID, true, dwTickCount); // (z.manning, 09/04/2007) - PLID 27286 - Renamed field in data to ProcDetails.
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "Risks", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "Alternatives", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "Complications", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "SpecialDiet", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "Showering", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "Bandages", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "Consent", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "AltConsent", nProcInfoID, true, dwTickCount);
		ConvertToRTFField(prs, strProcInfoQuery, "ProcedureT", "HospitalStay",nProcInfoID, true, dwTickCount);
		prs->MoveNext();
		iCurRecord++;
	}
	prs->Close();
}

// (c.haag 2012-05-12) - PLID 49936 - We require a handler for logging
int CMergeEngine::Log(LPCTSTR strFormat, ...)
{
	// Convert the ... to and argumented string
	CString strLog;
	va_list argList;
	va_start(argList, strFormat);
	strLog.FormatV(strFormat, argList);
	va_end(argList);
	return ::Log("%s", strLog);
}

// (c.haag 2012-05-12) - PLID 49936 - We require a handler for logging
int CMergeEngine::LogIndent(LPCTSTR strFormat, ...)
{
	// Convert the ... to and argumented string
	CString strLog;
	va_list argList;
	va_start(argList, strFormat);
	strLog.FormatV(strFormat, argList);
	va_end(argList);
	return ::LogIndent("%s", strLog);
}

// (c.haag 2012-05-12) - PLID 49936 - We require a handler for logging
int CMergeEngine::LogUnindent(LPCTSTR strFormat, ...)
{
	// Convert the ... to and argumented string
	CString strLog;
	va_list argList;
	va_start(argList, strFormat);
	strLog.FormatV(strFormat, argList);
	va_end(argList);
	return ::LogUnindent("%s", strLog);
}


// #define IsFieldIncluded(fldname)												(HAS_FLAG(m_nFlags, BMS_IGNORE_TEMPLATE_FIELD_LIST) || (m_aryFieldList.Find(fldname) >= 0))


/* Obsoleted by GetAllBalanceInfoSubQ()

CString CMergeEngine::GetPatientPrepaysSubQ()
{
	return "("
		"SELECT " + m_strGroupListQ + ".ID, SUM(LineItemT.Amount) AS SumOfAmount "
		"FROM (((LineItemT INNER JOIN (" + m_strGroupListSql + ") " + m_strGroupListQ + " ON LineItemT.PatientID = " + m_strGroupListQ + ".ID) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN AppliesT AS AppliesT_1 ON PaymentsT.ID = AppliesT_1.SourceID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID "
		"WHERE ((LineItemT.Deleted=0) AND (LineItemT.Type<4) AND (AppliesT_1.SourceID IS NULL) AND (AppliesT.DestID IS NULL) AND (PaymentsT.PrePayment=1)) "
		"GROUP BY " + m_strGroupListQ + ".ID"
		") MergePatientPrepaysSubQ ";
}

CString CMergeEngine::GetPatientPaysSubQ()
{
	return "("
		"SELECT " + m_strGroupListQ + ".ID, SUM(LineItemT.Amount) AS SumOfAmount "
		"FROM (LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) INNER JOIN (" + m_strGroupListSql + ") " + m_strGroupListQ + " ON LineItemT.PatientID = " + m_strGroupListQ + ".ID "
		"WHERE ((LineItemT.Deleted=0) AND ((LineItemT.Type=1) OR (LineItemT.Type=2) OR (LineItemT.Type=3))) "
		"GROUP BY " + m_strGroupListQ + ".ID"
		") MergePatientPaysSubQ ";
}

CString CMergeEngine::GetPatientChargesSubQ()
{
	return "("
		"SELECT LineItemT.PatientID, SUM(LineItemT.Amount*Quantity*TaxRate*(CASE WHEN CPTModifierT.Number IS NOT NULL THEN (Multiplier+100)/100 ELSE 1 END)) AS ChargeAmount "
		"FROM ((LineItemT LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number) RIGHT JOIN (" + m_strGroupListSql + ") " + m_strGroupListQ + " ON LineItemT.PatientID = " + m_strGroupListQ + ".ID "
		"WHERE ((LineItemT.Type=10) AND (LineItemT.Deleted=0)) "
		"GROUP BY LineItemT.PatientID"
		") MergePatientChargesSubQ ";
}

CString CMergeEngine::GetPatientBalanceSubQ()
{
	if (!m_strPatientBalanceSubQ.IsEmpty() ||
		 HAS_FLAG(m_nFlags, BMS_HIDE_ALL_DATA) ||
		 HAS_FLAG(m_nFlags, BMS_IGNORE_TEMPLATE_FIELD_LIST) ||
		 IsFieldIncluded("Person_Balance")) {
		
		// Only make this query once
		if (m_strPatientBalanceSubQ.IsEmpty()) {
			// Now make the query
			m_strPatientBalanceSubQ = "("
				"SELECT " + m_strGroupListQ +  ".ID, (CASE WHEN MergePatientChargesSubQ.ChargeAmount IS NULL THEN 0 ELSE MergePatientChargesSubQ.ChargeAmount END) - (CASE WHEN MergePatientPaysSubQ.SumOfAmount IS NULL THEN 0 ELSE MergePatientPaysSubQ.SumOfAmount END) + (CASE WHEN MergePatientPrepaysSubQ.SumOfAmount IS NULL THEN 0 ELSE MergePatientPrepaysSubQ.SumOfAmount END) AS AccountBal "
				"FROM (((" + m_strGroupListSql + ") " + m_strGroupListQ +  " LEFT JOIN " + GetPatientPaysSubQ() + " ON " + m_strGroupListQ +  ".ID = MergePatientPaysSubQ.ID) LEFT JOIN " + GetPatientChargesSubQ() + " ON " + m_strGroupListQ +  ".ID = MergePatientChargesSubQ.PatientID) LEFT JOIN " + GetPatientPrepaysSubQ() + " ON " + m_strGroupListQ +  ".ID = MergePatientPrepaysSubQ.ID"
				") MergePatientBalanceSubQ ";
		}

		// Return the name of the query
		return m_strPatientBalanceSubQ;
	} else {
		return "";
	}
}
//*/

	// ToDo: when you get a chance, search the practice.mde and the source code for "MergeResPatientExtendedQ".  I think it can be removed
	 

void AfxThrowNxLastError(LPCTSTR strFunction)
{
	CString strError;	
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
	strError.ReleaseBuffer();
	if (strError == "") {
		strError = "<null>";
	}

	AfxThrowNxException("%s: %s", strFunction, strError);
}

// (j.jones 2014-01-27 14:06) - PLID 58400 - this now returns a boolean for success/failure,
// and the created filename is an output parameter
bool CreateTempLocalFile(const CString &strFromPath, OUT CString &strCreatedTempFileName)
{
	// Save it as a temp file
	CString strTempPath = GetNxTempPath();
	
	// Get a temp path for the file
	strCreatedTempFileName = GetTempFileName(GetFileName(strFromPath), 't', strTempPath);

	if (!strCreatedTempFileName.IsEmpty()) {
		// Copy the file to the local temp path
		if (CopyFile(strFromPath, strCreatedTempFileName, TRUE)) {
			// Success
			return true;
		} else {
			// Couldn't copy for some reason, this should only happen when there are permission or other network/harddrive problems
			// (j.jones 2014-01-27 14:00) - PLID 58400 - give a clean message
			long nLastError = GetLastError();
			CString strSysError;
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, nLastError, 0, strSysError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
			strSysError.ReleaseBuffer();
			CString strError;
			strError.Format("Could not copy '%s' to '%s' due to the following error:\r\n\r\nError: %s",
				strFromPath, strCreatedTempFileName, strSysError);
			MessageBox(GetActiveWindow(), strError, "Practice", MB_ICONEXCLAMATION|MB_OK);
			strCreatedTempFileName = "";
			return false;
		}
	} else {
		// The temp directory already must have too many files based on this 
		// filename, so we're not allowed to make more (NxStandard won't let us).
		// (b.cardillo 2005-07-01 14:13) - PLID 16536 - In this case, we throw a 
		// an exception that uses a neen value so it can be safely trapped outside 
		// and handled meaningfully.
		// (j.jones 2014-01-27 14:00) - PLID 58400 - give a clean message
		CString strError;
		strError.Format("The temp path, '%s', already contains too many temp files based on "
			"the given filename, '%s'. No more files based on this name can be "
			"created until the temp path has been cleaned.", 
			strTempPath, GetFileName(strFromPath));
		MessageBox(GetActiveWindow(), strError, "Practice", MB_ICONEXCLAMATION|MB_OK);
		strCreatedTempFileName = "";
		return false;
	}
}

// (j.jones 2008-01-10 14:38) - PLID 18709 - similar to CreateBlankMergeInfo(), this function
// takes in nFlags with the standard BMS_ settings, and returns the sql that, when executed,
// will return an empty recordset with a column for each available merge field
CString CMergeEngine::CreateBlankMergeInfoRecordsetSql(long nFlags, OPTIONAL LPEXTRAMERGEFIELDSFUNC pfnCallbackExtraFields /*= NULL*/, OPTIONAL LPVOID pCallbackParam /*= NULL*/)
{
	//this replicates the CreateBlankMergeInfo() function, but only returns the results sql, nothing more

	// Create a merge engine object based on the just the fields, and no records
	CMergeEngine mi;
	mi.m_nFlags = nFlags;
	mi.m_pfnCallbackExtraFields = pfnCallbackExtraFields;
	mi.m_pCallbackParam = pCallbackParam;
	mi.m_nMergeFieldCountLimit = -1;
	// Create the temp query
	mi.m_strResultsT = mi.BuildMergeFields("BogusTableName");	
	
	return mi.m_strResultsT;
}

// (a.walling 2013-05-08 12:13) - PLID 56601 - CreateBlankMergeInfoFields returns a set, so we don't have to execute the sql just to get the field names.
std::set<CiString> CMergeEngine::CreateBlankMergeInfoFields(long nFlags, OPTIONAL LPEXTRAMERGEFIELDSFUNC pfnCallbackExtraFields /*= NULL*/, OPTIONAL LPVOID pCallbackParam /*= NULL*/)
{
	//this replicates the CreateBlankMergeInfo() function, but only returns the results sql, nothing more

	// Create a merge engine object based on the just the fields, and no records
	CMergeEngine mi;
	mi.m_nFlags = nFlags;
	mi.m_pfnCallbackExtraFields = pfnCallbackExtraFields;
	mi.m_pCallbackParam = pCallbackParam;
	mi.m_nMergeFieldCountLimit = -1;
	// Create the temp query
	mi.m_strResultsT = mi.BuildMergeFields("BogusTableName");	

	std::set<CiString> fields;
	swap(fields, mi.m_setMergeFields);
	return fields;
}


// (j.jones 2008-01-10 14:40) - PLID 18709 - similar to MergeToWord(), this will take in an aryFieldList of all the fields
// we need to load data for, information on how to filter the patients for the results, and return an sql statement that,
// when executed, will return a recordset with all the merge data we need
// (j.gruber 2013-01-17 14:54) - PLID 54689 - add resFilter
CString CMergeEngine::GenerateMergeRecordsetSql(CStringSortedArrayNoCase &aryFieldList, const CString &strSqlMergeTo, const CString &strPatientIDFieldName /*= ""*/, const CString &strResFilter /*=""*/)
{
	//this replicates portions of the MergeToWord() function, but only returns the results sql, nothing more

	if(aryFieldList.GetSize() == 0) {
		return "";
	}

	// Make sure the given alternate patient id field name is not the same as one of the fields we always use (ID or RowNumber)
	ASSERT(strPatientIDFieldName.CompareNoCase("ID") != 0 && strPatientIDFieldName.CompareNoCase("RowNumber") != 0);
	if ((strPatientIDFieldName.CompareNoCase("ID") == 0 || strPatientIDFieldName.CompareNoCase("RowNumber") == 0)) {
		ThrowNxException("CMergeEngine::MergeToWord: Invalid alternate patient ID field name given: %s", strPatientIDFieldName);
	}
	// Store the value in a member variable for later use
	m_strPatientIDFieldName = strPatientIDFieldName;

	// (j.gruber 2013-01-17 14:55) - PLID 54689 - if the res filter is not blank, set it
	if (!strResFilter.IsEmpty()) {
		m_strResFilter = strResFilter;
	}

	for(int i=0;i<aryFieldList.GetSize();i++) {
		if(m_aryFieldList.Find(aryFieldList.GetAt(i)) < 0) {
			m_aryFieldList.Insert(aryFieldList.GetAt(i));
		}
	}

	// Create a temporary query based on the given SQL and use it as the list to merge to
	CString strWorkingSqlMergeTo = strSqlMergeTo;
	strWorkingSqlMergeTo.TrimLeft(); 
	strWorkingSqlMergeTo.TrimRight();
	if (strWorkingSqlMergeTo.Left(6).CompareNoCase("SELECT") != 0) {
		if (!m_strPatientIDFieldName.IsEmpty()) {
			// Set the string based on a SELECT of the given merge-to string, make sure to also select the patient id field
			m_strGroupListSql.Format("SELECT DISTINCT ID, RowNumber, %s FROM %s", m_strPatientIDFieldName, strWorkingSqlMergeTo);
		} else {
			// Just set the string based on a SELECT of the given merge-to string
			m_strGroupListSql.Format("SELECT DISTINCT ID, RowNumber FROM %s", strWorkingSqlMergeTo);
		}
	} else {
		// If the given string was already a select statement, just use that statement
		m_strGroupListSql = strWorkingSqlMergeTo;
		// TODO: Find a way to ASSERT that the given select statement has an ID and a RowNumber
	}

	m_strGroupListQ = m_omTempObjects.CreateTempQuery(m_strGroupListSql);
	if (!m_strGroupListQ.IsEmpty()) {
		
		// Used to group mail batches together at the data level (this must be set before BuildMergeSql is called)
		m_nMailBatchId = NewNumber("MailSent", "MailBatchID");

		// Build a temp query out of the merge sql
		CString strMergeSql;
		strMergeSql = BuildMergeSql();
		m_strResultsT = strMergeSql;
		// (a.walling 2007-11-07 12:52) - PLID 27998 - VS2008 - DEBUG is an MFC definition, so we will call this DEBUGCODE
		DEBUGCODE(Log("CMergeEngine::MergeToWord Results = %s", m_strResultsT));
		
		// If that was successful, let's merge, baby!
		if (!m_strResultsT.IsEmpty()) {

			return m_strResultsT;
		}
	}

	return "";
}

//TES 7/8/2011 - PLID 20536 - Added an optional pdtServiceDate parameter
// (a.wilson 2013-04-18 13:37) - PLID 56142 - added return value to depict if the merge was stopped (missing prescription fields).
/// <summary>
/// Merges a Word document based on content from a SQL query
/// </summary>
/// <param name="strTemplateName">The full path to the Word template</param>
/// <param name="vecTempFilePathNames">The full paths to all temporary files used in the merge</param>
/// <param name="strSqlMergeTo">The SQL query</param>
/// <param name="strPatientIDFieldName">The SQL field name of the patient ID, or an empty string if not applicable</param>
/// <param name="nMergedPacketID">If merging a packet, this must be the packet ID</param>
/// <param name="nPacketCategoryID">If merging a packet, this may be the packet category ID, or -1 if none</param>
/// <param name="bSeparateDocs">True if we should calculate the category ID for each template when merging packets</param>
/// <param name="strExtraProgress">Extra progress text</param>
/// <param name="pdtServiceDate">The service date if applicable</param>
/// <returns>TRUE if the merge succeeded; otherwise FALSE</returns>
bool CMergeEngine::MergeToWord(IN const CString &strTemplateName, IN const std::vector<CString>& vecTempFilePathNames, IN const CString &strSqlMergeTo, OPTIONAL IN const CString &strPatientIDFieldName /*= ""*/, OPTIONAL IN const long nMergedPacketID /*= -1*/, OPTIONAL IN long nPacketCategoryID /*= -1*/, OPTIONAL IN bool bSeparateDocs /*= false*/, OPTIONAL const CString &strExtraProgress /*= ""*/, OPTIONAL COleDateTime *pdtServiceDate /*= NULL*/)
{
	// (j.jones 2014-01-27 14:42) - PLID 58400 - abort now if the template name does not exist
	if(!FileUtils::DoesFileOrDirExist(strTemplateName)) {
		CString strError;
		strError.Format("The template '%s' could not be found. Please ensure that this template has not been deleted or renamed.\n\n"
			"The merge will be cancelled.", strTemplateName);
		MessageBox(GetActiveWindow(), strError, "Practice", MB_ICONEXCLAMATION|MB_OK);
		return false;
	}

	CProgressMgr pgs("Mail Merge", "Preparing to merge", 0, 100, NULL, strExtraProgress);

	// Make sure the given alternate patient id field name is not the same as one of the fields we always use (ID or RowNumber)
	ASSERT(strPatientIDFieldName.CompareNoCase("ID") != 0 && strPatientIDFieldName.CompareNoCase("RowNumber") != 0);
	if ((strPatientIDFieldName.CompareNoCase("ID") == 0 || strPatientIDFieldName.CompareNoCase("RowNumber") == 0)) {
		ThrowNxException("CMergeEngine::MergeToWord: Invalid alternate patient ID field name given: %s", strPatientIDFieldName);
	}
	// Store the value in a member variable for later use
	m_strPatientIDFieldName = strPatientIDFieldName;

	// Create a temporary query based on the given SQL and use it as the list to merge to
	CString strWorkingSqlMergeTo = strSqlMergeTo;
	strWorkingSqlMergeTo.TrimLeft(); 
	strWorkingSqlMergeTo.TrimRight();
	if (strWorkingSqlMergeTo.Left(6).CompareNoCase("SELECT") != 0) {
		if (!m_strPatientIDFieldName.IsEmpty()) {
			// Set the string based on a SELECT of the given merge-to string, make sure to also select the patient id field
			m_strGroupListSql.Format("SELECT DISTINCT ID, RowNumber, %s FROM %s", m_strPatientIDFieldName, strWorkingSqlMergeTo);
		} else {
			// Just set the string based on a SELECT of the given merge-to string
			m_strGroupListSql.Format("SELECT DISTINCT ID, RowNumber FROM %s", strWorkingSqlMergeTo);
		}
	} else {
		// If the given string was already a select statement, just use that statement
		m_strGroupListSql = strWorkingSqlMergeTo;
		// TODO: Find a way to ASSERT that the given select statement has an ID and a RowNumber
	}

	m_strGroupListQ = m_omTempObjects.CreateTempQuery(m_strGroupListSql);
	if (!m_strGroupListQ.IsEmpty()) {
		
		pgs.SetProgress("Loading template from server", 10);
		
		// We're going to pull the template across the network before we use it, 
		// so that all the crap we do doesn't get bogged by network slowness
		CString strLocalTemplatePath = "";
		if(!CreateTempLocalFile(strTemplateName, strLocalTemplatePath) || strLocalTemplatePath.IsEmpty()) {
			// (j.jones 2014-01-27 14:00) - PLID 58400 - CreateTempLocalFile should have given a message already,
			// so now simply abort the merge process
			return false;
		}

		CString strExtension = FileUtils::GetFileExtension(strTemplateName);
				
		pgs.SetProgress("Scanning template for merge fields", 20);

		// Fill the list of possible fields based on the template
		BOOL bLoadTemplateMergeFieldsResult = TRUE;
		if (!HAS_FLAG(m_nFlags, BMS_IGNORE_TEMPLATE_FIELD_LIST)) {
			bLoadTemplateMergeFieldsResult = GetWPManager()->LoadTemplateMergeFields(strLocalTemplatePath, m_aryFieldList, strExtension);
		}
		else if (SureScripts::IsEnabled() && !m_arydwPrescriptionIDs.IsEmpty()) {
			// (a.walling 2009-07-08 13:55) - PLID 34261 - Warn (don't show me again) if the template does not have 'Prescription_Note_To_Pharmacist'

			bLoadTemplateMergeFieldsResult = GetWPManager()->LoadTemplateMergeFields(strLocalTemplatePath, m_aryFieldList, strExtension);
		}
		// (z.manning 2016-06-02 14:39) - NX-100790 - If this failed, do not continue
		if (!bLoadTemplateMergeFieldsResult) {
			DeleteFileWhenPossible(strLocalTemplatePath);
			return false;
		}

		// (a.wilson 2013-04-08 15:48) - PLID 56142 - check for denied response message id
		if (SureScripts::IsEnabled() && !m_arydwPrescriptionIDs.IsEmpty()) {
			bool bFoundNoteToPharmacist = false, bFoundDeniedResponseMessageID = false, bFoundPharmacyReferenceNumber = false;

			for (int ixMergeField = 0; (ixMergeField < m_aryFieldList.GetSize() && (!bFoundNoteToPharmacist || !bFoundDeniedResponseMessageID || !bFoundPharmacyReferenceNumber)); ixMergeField++) {
				if (m_aryFieldList[ixMergeField].CompareNoCase("Prescription_Note_To_Pharmacist") == 0) {
					bFoundNoteToPharmacist = true;
				} else if (m_aryFieldList[ixMergeField].CompareNoCase("Prescription_Denied_Response_Message_ID") == 0) {
					bFoundDeniedResponseMessageID = true;
				} else if (m_aryFieldList[ixMergeField].CompareNoCase("Prescription_Pharmacy_Reference_Number") == 0) {
					bFoundPharmacyReferenceNumber = true;
				}
			}
			if (HAS_FLAG(m_nFlags, BMS_IGNORE_TEMPLATE_FIELD_LIST)) {
				m_aryFieldList.RemoveAll();
			}
			if (!bFoundNoteToPharmacist) {
				DontShowMeAgain(NULL, "Your prescription template does not include the 'Prescription_Note_To_Pharmacist' merge field. Please include this field in your prescription templates.", "SureScriptsWarnNoteToPharmacistFieldMissing", "Practice");
			}
			// (a.wilson 2013-04-08 16:14) - PLID 56142 - if they have ERx and we could not find atleast one of the merge fields
			// and atleast one of the prescriptions are from a DenyNewRx, then we need to stop them from merging.
			if ((!bFoundDeniedResponseMessageID || !bFoundPharmacyReferenceNumber) 
				&& (g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts)
				&& ReturnsRecordsParam("SELECT PatMeds.ID FROM PatientMedications PatMeds "
				"INNER JOIN DrugList ON PatMeds.MedicationID = DrugList.ID "
				"WHERE PatMeds.DenyNewRxResponseID IS NOT NULL AND PatMeds.ID IN ({INTARRAY}) ", m_arydwPrescriptionIDs))
			{
				MessageBox(NULL, FormatString("The following merge fields were not included on your prescription word template:%s\r\n\r\n"
					"You must include all necessary merge fields on your template when printing a prescription from a deny and rewrite.", 
					FormatString("%s%s", (!bFoundDeniedResponseMessageID ? "\r\n- 'Prescription_Denied_Response_Message_ID'" : ""), 
					(!bFoundPharmacyReferenceNumber ? "\r\n- 'Prescription_Pharmacy_Reference_Number'" : ""))), 
					"Practice", MB_OK | MB_ICONSTOP);
				//ensure the file gets deleted since we do this at the end of this function if it succeeds.
				DeleteFileWhenPossible(strLocalTemplatePath);
				return false;
			}

		}

		pgs.SetProgress("Building database query", 30);

		// Used to group mail batches together at the data level (this must be set before BuildMergeSql is called)
		m_nMailBatchId = NewNumber("MailSent", "MailBatchID");

		// Build a temp query out of the merge sql
		CString strMergeSql;
		strMergeSql = BuildMergeSql();
		m_strResultsT = strMergeSql;
		// (a.walling 2007-11-07 12:52) - PLID 27998 - VS2008 - DEBUG is an MFC definition, so we will call this DEBUGCODE
		DEBUGCODE(Log("CMergeEngine::MergeToWord Results = %s", m_strResultsT));
		
		// If that was successful, let's merge, baby!
		if (!m_strResultsT.IsEmpty()) {


			pgs.SetProgress("Obtaining patient listing", 35);

			// First get the patient's shared document path (get this first so that we guarantee its 
			// existence BEFORE we calculate and save the filename in the patient history table MailSent)
			long nGroupSingularID = GetGroupSingularId();

			// (c.haag 2006-06-20 16:39) - PLID 15556 - Get the first name, last name and user-defined ID
			// here so that we don't have to pull them from data in three completely separate places
			CString strFirst, strLast;
			long nUserDefinedID;
			_RecordsetPtr prs = CreateRecordset(
				"SELECT Last, First, CASE WHEN PatientsT.UserDefinedID IS NULL THEN PersonT.ID ELSE PatientsT.UserDefinedID END AS UserDefinedID FROM PersonT "
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"WHERE PersonT.ID = %li", nGroupSingularID);
			FieldsPtr flds = prs->Fields;
			strLast = AdoFldString(flds, "Last");
			strFirst = AdoFldString(flds, "First");
			nUserDefinedID = AdoFldLong(flds, "UserDefinedID");
			prs->Close();

			// (a.walling 2010-04-28 17:38) - PLID 38410 - Pass in a connection to GetPatientDocumentPath
			CString strDocPath = GetPatientDocumentPath(GetRemoteData(), nGroupSingularID, strFirst, strLast, nUserDefinedID);

			pgs.SetProgress("Calculating document name", 40);

			// Get a good filename to save the document as
			// (a.walling 2010-02-16 10:49) - PLID 37390
			// (z.manning 2016-02-22 14:15) - PLID 68394 - Moved this code to its own function
			// (z.manning 2016-05-16 8:38) - NX-100642 - No longer takes any parameters
			CString strSaveExtension = GetWPManager()->GetFileExtension();

			m_strSavedAs = CalcDocumentSaveName(nGroupSingularID, strFirst, strLast, nUserDefinedID, strSaveExtension);


			pgs.SetProgress("Saving merge in patient history", 45);
			// Store it in the patient history
			//TES 7/8/2011 - PLID 20536 - Pass in whatever service date we were given.
			SaveInPatientHistory(m_strSavedAs, strTemplateName, nMergedPacketID, nPacketCategoryID, bSeparateDocs, pdtServiceDate);

			pgs.SetProgress("Starting merge", 50);

			// Do the merge
			// (z.manning 2016-06-02 15:09) - NX-100790 - Need to check the return value of DoMerge
			if (!DoMerge(strLocalTemplatePath, vecTempFilePathNames, strDocPath ^ m_strSavedAs, &pgs)) {
				DeleteFileWhenPossible(strLocalTemplatePath);
				return false;
			}

			pgs.SetProgress("Merge complete", 95);

		}


		pgs.SetProgress("Deleting temporary files", 98);

		// The merge is done, delete the temporary file
		// (b.cardillo 2005-10-03 17:39) - PLID 17335 - Made it call DeleteFileWhenPossible() 
		// instead of just DeleteFile().  This ensures that even if the file is still in use 
		// by Word when we get to this point (which it is in the case of this pl item) then 
		// the temp file will still eventually be deleted.
		DeleteFileWhenPossible(strLocalTemplatePath);
	}

	pgs.SetProgress("Finished", 100);

	return true;
}

//void CMergeEngine::SaveInPatientHistory(const CString &strSavedAs, const CString &strTemplateName, long nMergedPacketID /*= -1*/)
//TES 7/8/2011 - PLID 20536 - Added an optional pdtServiceDate parameter
// (j.dinatale 2012-03-02 09:40) - PLID 48165 - Miracles do happen! Cleaned this function up and made it all paramaterized
void CMergeEngine::SaveInPatientHistory(const CString &strSavedAs, const CString &strTemplateName, long nMergedPacketID /*= -1*/, long nPacketCategoryID /*= -1*/, bool bSeperateDocs /*= false*/, OPTIONAL COleDateTime *pdtServiceDate /*= NULL*/)
{
	// If the merge was successful and we merged to patients and we want to save the file in the patients history
	if ((HAS_FLAG(m_nFlags, BMS_PATIENT_BASED) || !m_strPatientIDFieldName.IsEmpty()) && // Is a patient-based merge (because the ID field is assumed to be the patient id) or is any kind of merge where the patient-id-field-name is given
		(HAS_FLAG(m_nFlags, BMS_SAVE_FILE_AND_HISTORY) || HAS_FLAG(m_nFlags, BMS_SAVE_HISTORY_NO_FILE)) && // Is a save-file-and-history or a save-history-no-file; either way, we know we want to save a history
		!m_strResultsT.IsEmpty() // We're merging something
		) {
		// Only proceed if there is a filename to save
		if (strSavedAs.IsEmpty()) {
			// If we've been asked to save to the patient history, we 
			// better have already figured out what file name we're saving
			AfxThrowNxException("The patient history document name was unspecified");
		} else {
			// Get the name of the template on which the new document was based
			CString strTitle = GetFileName(strTemplateName);
			strTitle = strTitle.Left(strTitle.GetLength() - GetFileExtension(strTitle).GetLength() - 1);
			CString strHistoryLink, strType;
			if (HAS_FLAG(m_nFlags, BMS_MERGETO_EMAIL_PLAIN) || HAS_FLAG(m_nFlags, BMS_MERGETO_EMAIL_HTML)) {
				// It's an email, so there is no file to link to
				strHistoryLink = "";
				strType = "";
				strTitle += " (Email)";
			} else if (HAS_FLAG(m_nFlags, BMS_SAVE_HISTORY_NO_FILE)) {
				// No file was saved, so don't link to the file
				strHistoryLink = "";
				strType = "";
			} else {
				// Normal, just link to the doc path
				strHistoryLink = strSavedAs;
				strType = SELECTION_WORDDOC;
			}

			// (a.walling 2010-09-15 09:20) - PLID 7018 - Append (Reprint) if it is a reprint
			if (m_nFlags & BMS_REPRINT) {
				strTitle += " (Reprint)";
			}

			// We don't want the order by because the query won't run and who needs it for these inserts anyway?
			//TES 3/10/2004: Actually, we do want the order by, if we're writing to PrintedSuperbillsT.  But we can't use
			//it in the subquery, so just remember it.
			CString strLocSql(m_strResultsT);
			long nOrderBy, nNext;
			CString strOrderBy;
			FindClause(strLocSql, fcOrderBy, nOrderBy, nNext);
			if (nOrderBy >= 0) {
				strOrderBy = strLocSql.Mid(nOrderBy, nNext-nOrderBy);
				strLocSql.Delete(nOrderBy, nNext-nOrderBy);
			}

			//Find out if this template has been stored.
			//First, we need to take off the shared path, if it's on there, because the record in the table is always relative to the shared path.
			CString strRecordValue = strTemplateName;
			if(strRecordValue.Left(GetSharedPath().GetLength()).CompareNoCase(GetSharedPath()) == 0) {
				strRecordValue = strRecordValue.Mid(GetSharedPath().GetLength());
				//Also, make sure it begins with exactly one backslash.
				if(strRecordValue.GetAt(0) != '\\') strRecordValue = "\\" + strRecordValue;
			}

			//Either way, let's make it all uppercase
			strRecordValue.MakeUpper();

			// Calculate the appropriate field name to use for the queries below
			CString strPatientIDFieldName;
			if (m_strPatientIDFieldName.IsEmpty()) {
				strPatientIDFieldName = "ID";
			} else {
				strPatientIDFieldName = m_strPatientIDFieldName;
			}

			// (j.dinatale 2012-03-01 18:03) - PLID 48165 - fixed this tree to be a little bit more concise, also got rid of the strings for all the NULL stuff
			// (b.cardillo 2004-06-21 10:01) - PLID 12909 - Get the category id into a string, and 
			// "NULL" if no category id was specified.
			long nCategoryID = -1;

			// (a.walling 2006-04-26) PLID 18343 Get packet category
			if (nMergedPacketID != -1) { // is a packet
				if (nPacketCategoryID != -1) { // packet has a default category
					nCategoryID = nPacketCategoryID;
				}
				else { // category has no default set
					if (bSeperateDocs) { // separate documents should have separate categories
						nCategoryID = GetTemplateCategory(GetFileName(strTemplateName));
					}
				}
			}
			else // not a packet, get the category if possible
			{
				if (m_nCategoryID != -1) {
					//TES 8/2/2011 - PLID 44814 - Make sure the user has permission for this category
					if(CheckCurrentUserPermissions(bioHistoryCategories, sptView, TRUE, m_nCategoryID, TRUE)) {
						nCategoryID = m_nCategoryID;
					}
				} else {
					long nCatID = GetTemplateCategory(GetFileName(strTemplateName));
					if (nCatID != -1) {
						//TES 8/2/2011 - PLID 44814 - Make sure the user has permission for this category
						if(CheckCurrentUserPermissions(bioHistoryCategories, sptView, TRUE, nCatID, TRUE)) {
							nCategoryID = nCatID;
						}
					}
				}
			}

			// Run the queries
			//We are tracking this template so...
			//First we need to get the MailID and Patient id out of the query.
			// Insert the records into each patient's history
			//DRT 6/12/03 - This now groups by everything but id (takes the lowest of that), so only 1 record is put in the patients history
			//		tab if they have 2 superbill ids generated. (PLID 8180)
			// (a.walling 2008-01-03 13:18) - PLID 28276 - Pass in the strTemplateID, which can be NULL.
			// (j.jones 2008-09-04 17:03) - PLID 30288 - supported MailSentNotesT
			// (c.haag 2010-01-27 12:10) - PLID 36271 - Use GetDate() for the service date; not COleDateTime::GetCurrentTime
			//TES 7/8/2011 - PLID 20536 - We now calculate the service date up a bove
			// (j.dinatale 2012-03-01 18:06) - PLID 48165 - do you believe in parameterization? cleared out a bunch of code that was making strings for nulls etc.
			//		Also got rid of the weird query logic that was here and cleaned up the sql.
			// (j.dinatale 2012-03-15 11:56) - PLID 48165 - merged the collection of the template ID into the main query
			// (d.singleton 2012-07-31 08:55) - PLID 51835 - switch coalesce and max() so it will return results even in no records exist.
			// (j.armen 2014-01-30 10:38) - PLID 55225 - Idenitate MailSent
			CSqlFragment sqlMailMerge;
			sqlMailMerge.Create(
				"SET NOCOUNT ON; \r\n"
				"DECLARE @nCurrMaxID INT; \r\n"
				"SET @nCurrMaxID = (SELECT COALESCE(MAX(MailID), 0) FROM MailSent WITH(UPDLOCK, HOLDLOCK)); \r\n"
				"DECLARE @nTemplateID INT; \r\n"
				"SET @nTemplateID = (SELECT COALESCE((SELECT TOP 1 ID FROM MergeTemplatesT WHERE Path = {STRING}), -1) AS TemplateID) \r\n"
				" \r\n"
				"DECLARE @MailMergeOffset TABLE \r\n"
				"( \r\n"
				"	Offset INT IDENTITY(1,1), \r\n"
				"	PatientID INT \r\n"
				") \r\n"
				" \r\n"
				"INSERT INTO @MailMergeOffset SELECT DISTINCT {SQL} FROM ({SQL}) MergeQ \r\n"
				" \r\n"
				"SET IDENTITY_INSERT MailSent ON\r\n"
				"INSERT INTO MailSent  \r\n"
				"(MailID, MailBatchID, PersonID, Selection, PathName, Subject, Sender, [Date], Location, TemplateID, MergedPacketID, ServiceDate, CategoryID, PicID, EMNID) \r\n"		
				"	SELECT (@nCurrMaxID + Offset) AS MailID, {INT} AS BatchID, PatientID AS PatientID,  \r\n"
				"	{STRING} AS Selection, {STRING} AS Path, {STRING} AS Subject, {STRING} AS Sender, GETDATE() AS Date, {INT} AS Location, \r\n"
				"	CASE WHEN @nTemplateID = -1 THEN NULL ELSE @nTemplateID END AS TemplateID, {VT_I4} AS MergedPacketID, COALESCE({VT_DATE}, GETDATE()) AS ServiceDate, {VT_I4} AS CategoryID, {VT_I4} AS PicID, {VT_I4} AS EMNID \r\n"
				"	FROM \r\n"
				"	@MailMergeOffset MailMergeOffsetT; \r\n"
				" \r\n"
				"SET IDENTITY_INSERT MailSent OFF\r\n"
				"INSERT INTO MailSentNotesT (MailID, Note) \r\n"
				"	SELECT (@nCurrMaxID + Offset) AS MailID, {STRING} AS Note \r\n"
				"	FROM \r\n"
				"	@MailMergeOffset MailMergeOffsetT; \r\n"
				"	SET NOCOUNT OFF; \r\n"
				" \r\n"
				"SELECT (@nCurrMaxID + Offset) AS NewMailID, PatientID AS {SQL} FROM @MailMergeOffset MailMergeOffsetT; ", 
				strRecordValue, CSqlFragment(strPatientIDFieldName), CSqlFragment(m_strGroupListSql), m_nMailBatchId, strType, strHistoryLink, m_strSubjectMatter, GetCurrentUserName(), 
				GetCurrentLocation(), (nMergedPacketID == -1 ? g_cvarNull : variant_t(nMergedPacketID)), (pdtServiceDate ? _variant_t(*pdtServiceDate, VT_DATE) : g_cvarNull),
				(nCategoryID == -1 ? g_cvarNull : _variant_t(nCategoryID)), (m_nPicID < 0 ? g_cvarNull : m_nPicID), (m_nEMNID < 0 ? g_cvarNull : m_nEMNID), strTitle, CSqlFragment(strPatientIDFieldName)
			);

			// (j.dinatale 2012-03-02 10:29) - PLID 48165 - collect the mailIDs that were actually inserted, instead of our "best" guess
			_RecordsetPtr rsMail = CreateParamRecordset(
				"BEGIN TRAN; \r\n"
				"{SQL} \r\n"
				"COMMIT TRAN;", sqlMailMerge);

			// (j.jones 2014-08-05 08:48) - PLID 63141 - always track the MailIDs/PersonIDs
			// to be used later in tablechecker sends
			CArray<long, long> aryPersonIDs;
			CArray<long, long> aryMailIDs;
			std::set<long> aryUniquePatientIDs;
			while (!rsMail->eof) {
				long nPersonID = AdoFldLong(rsMail, strPatientIDFieldName, -1);
				aryPersonIDs.Add(nPersonID);
				//also track patient IDs in aryUniquePatientIDs
				aryUniquePatientIDs.insert(nPersonID);
				
				aryMailIDs.Add(AdoFldLong(rsMail, "NewMailID"));				
				rsMail->MoveNext();
			}
			rsMail->Close();

			ASSERT(aryPersonIDs.GetSize() == aryMailIDs.GetSize());

			//Now actually track.
			// (a.walling 2010-09-15 09:20) - PLID 7018 - Ignore tracking when reprinting
			if (!(m_nFlags & BMS_REPRINT)) {
				for (int i = 0; i < aryPersonIDs.GetSize(); i++) {
					PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_TemplateSent, aryPersonIDs.GetAt(i), COleDateTime::GetCurrentTime(), aryMailIDs.GetAt(i));
				}
			}

			// (a.walling 2010-09-15 09:20) - PLID 7018 - Ignore PrintedSuperbillsT etc when reprinting
			if ( (m_nFlags & BMS_SUPERBILL) && !(m_nFlags & BMS_REPRINT) ) {
				// This is a superbill so generate the saved super bill record and connect it to this Mail Batch and this patient
				CString str1;
				//TES 3/10/2004: Now, we have to take our ORDER BY clause and translate it from strLocSql scope to MergeQ scope.
				//This will involve two steps.
				//1.) Each of these must be selected in strLocSql so they'll still be valid for MergeQ
				CString strNewLocSql, strNewOrderBy;
				if(strOrderBy == "") {
					strNewLocSql = strLocSql;
					strNewOrderBy = "";
				}
				else {
					//2.) If any of these fields have been aliased, use the alias.  Also, if they are in Table.Field format,
					//    remove the 
					int nChar = 9;//Skip the ORDER BY part.
					strNewOrderBy = "ORDER BY ";
					CString strOrderByFields;
					long nFieldCount = 1;
					while(nChar < strOrderBy.GetLength()) {
						int nNextChar = strOrderBy.Find(",", nChar);
						if(nNextChar == -1) nNextChar = strOrderBy.GetLength();
						CString strField = strOrderBy.Mid(nChar, nNextChar-nChar);
						strOrderByFields += strField;
						// (z.manning 2009-08-03 16:23) - PLID 19645 - If we have a function in the order by
						// clause then we MUST give it an alias within the select statement that will eventually
						// become MergeQ. It doesn't matter what the alias is, so let's just use the name of
						// the function.
						if(strField.Left(4).CompareNoCase("dbo.") == 0 && strField.Find('(') >= 4) {
							// (z.manning 2009-08-05 14:23) - PLID 19645 - Set the field name for the actual
							// order by clause to the alias.
							strField = strField.Mid(4, strField.Find('(') - 4);
							// (z.manning 2009-08-05 14:23) - PLID 19645 - Now add the alias to the part that's
							// going into the select clause.
							strOrderByFields += FormatString(" AS %s", strField);
						}
						strOrderByFields += ", ";
						CString strAlias;
						if(GetAliasByField(strLocSql, strField, strAlias)) strField = strAlias;
						if(strField.Find(".") != -1 && strField.Left(4).CompareNoCase("dbo.") != 0) {
							strField = "MergeQ" + strField.Mid(strField.Find("."));
						}
						strNewOrderBy += strField + ", ";
						nChar = nNextChar+1;
						nFieldCount++;
					}
					strNewOrderBy = strNewOrderBy.Left(strNewOrderBy.GetLength()-2);

					long nFrom = ReverseFindNoCaseNotEmbedded(strLocSql, "FROM ");
					strOrderByFields = strOrderByFields.Left(strOrderByFields.GetLength() - 2);
					strNewLocSql = strLocSql.Left(nFrom) + ", " + strOrderByFields + " " + strLocSql.Mid(nFrom);
				}

				//Now we have to use this new ORDER BY clause in a temporary table, which we can then use in our insert
				//to calculate the ids in the correct order.
				str1.Format("DECLARE @Order table (OrderNum int identity, OutsideID int UNIQUE NOT NULL) "
					"INSERT INTO @Order (OutsideID) SELECT [Record_RowNumber] FROM (%s) MergeQ %s "
					"INSERT INTO PrintedSuperbillsT (ID, MailBatchID, PrintedOn, PatientID, ReservationID, BillID, SavedID) " 
					"  SELECT (SELECT CASE WHEN MAX(ID) IS NOT NULL THEN MAX(ID) ELSE 0 END FROM PrintedSuperbillsT) + OrderedQ.OrderNum, "
					"          %li, GETDATE(), [Person_Internal_ID], CASE WHEN ReservationID IS NOT NULL THEN ReservationID ELSE -1 END, -1, (SELECT CASE WHEN MAX(SavedID) IS NOT NULL THEN MAX(SavedID) ELSE 0 END FROM PrintedSuperbillsT) + OrderedQ.OrderNum "
					"  FROM (%s) MergeQ INNER JOIN @Order OrderedQ ON MergeQ.Record_RowNumber = OutsideID "
					"",
					strNewLocSql, strNewOrderBy, m_nMailBatchId, strNewLocSql);

				// (j.anspach 05-11-2005 14:55 - PLID 16495: Since we've already formatted the string we have to use
				//   ExecuteSqlStd() instead of ExecuteSql() otherwise it will throw errors when attempting to use a string
				//   that contains a %
				ExecuteSqlStd(str1);
			}

			// (j.jones 2014-08-04 13:31) - PLID 63141 - this now sends an Ex tablechecker, we know IsPhoto is always false here
			for (int i = 0; i < aryMailIDs.GetSize(); i++) {
				CClient::RefreshMailSentTable(aryPersonIDs.GetAt(i), aryMailIDs.GetAt(i));
			}

			// (j.jones 2007-09-13 08:58) - PLID 27371 - need to send an EMR tablechecker if attaching to a PIC or an EMN
			if(m_nPicID != -1 || m_nEMNID != -1) {
				// (j.jones 2014-08-05 08:51) - PLID 63141 - don't need to run a query, we already have this data
				for each(long nPatientID in aryUniquePatientIDs) {
					CClient::RefreshTable(NetUtils::EMRMasterT, nPatientID);
				}
			}
		}
	}
}

// (a.walling) 5/1/06 PLID 18343 get the category ID of the packet
// this is a static function
long CMergeEngine::GetPacketCategory(const long nPacketID)
{
	_RecordsetPtr rsPacketCategory = CreateRecordset("SELECT PacketCategoryID FROM PacketsT WHERE Deleted = 0 AND ID = %li", nPacketID);
	
	if (!rsPacketCategory->eof) {
		return AdoFldLong(rsPacketCategory, "PacketCategoryID", -1);
	}

	return -1;
}

// If the patient set has 1 patient use that patient's ID, if it has 
// multiple patients use the generic ID -25, otherwise return failure
long CMergeEngine::GetGroupSingularId()
{
	// How many records are we talking about?

	_RecordsetPtr prs;
	CString strPatientIDFieldName;
	if (m_strPatientIDFieldName.IsEmpty()) {
		strPatientIDFieldName = "ID";
	} else {
		strPatientIDFieldName = m_strPatientIDFieldName;
	}
	prs = CreateRecordset(
		"SELECT COUNT(*) AS CountOfRecs, Min(%s) AS MinOfID FROM (%s) %s ", 
		strPatientIDFieldName, m_strGroupListSql, m_strGroupListQ);

	// Even if there are no items in the group, this should always return exactly one record
	ASSERT(!prs->eof);

	// Get COUNT(*)
	FieldsPtr flds = prs->Fields;
	long nRowCount = AdoFldLong(flds, "CountOfRecs");

	// Decide what to do based on how many items there were in the group
	if (nRowCount == 1) {
		// Get the id of the one record
		return AdoFldLong(flds, "MinOfID");
	} else if (nRowCount > 1) {
		// More than one record
		return -25;
	} else {
		// We don't have any records so return failure (I'm not sure this is necessary, we might be okay to 
		// just return -25 in this case as well, but the old incorrect and inefficient way this function was 
		// written would have returned 0 here to I'm leaving it in case something relies on that)
		return 0;
	}
}

CString CMergeEngine::CalcDocumentSaveName(long nPersonID, const CString& strExtension)
{
	CString strAns;
	// We only handle patient-based save names right now
	if (HAS_FLAG(m_nFlags, BMS_PATIENT_BASED)) {
		// (a.walling 2010-02-16 10:49) - PLID 37390 - Requires an extension
		return GetPatientDocumentName(nPersonID, strExtension);
	} else {
		return "";
	}
}

CString CMergeEngine::CalcDocumentSaveName(long nPersonID, const CString& strFirst, const CString& strLast, unsigned long nUserDefinedID, const CString& strExtension)
{
	CString strAns;
	// We only handle patient-based save names right now
	if (HAS_FLAG(m_nFlags, BMS_PATIENT_BASED)) {
		// (a.walling 2010-02-16 10:49) - PLID 37390 - Requires an extension
		return GetPatientDocumentName(nPersonID, strFirst, strLast, nUserDefinedID, strExtension);
	} else {
		return "";
	}
}

/// <summary>
/// Goes through the content of every text file in a collection looking for references to absolute file paths,
/// copies those absolute files to temp files and updates those references to point to the temp files
/// </summary>
/// <param name="vecTextFiles">The collection of text files to iterate through</param>
/// <returns>The collection of all newly added temp filenames</returns>
std::vector<CString> CMergeEngine::RemapAbsoluteFileReferencesToTempFiles(const std::vector<CString>& vecTextFiles)
{
	// This is necessary for virtual channel merges since the remote machine does not have its own
	// copy of the shared images folder

	// Declare the collection of paths we're looking for
	std::vector<CString> vecAbsolutePathsToFind { GetLocationImagePath(), GetContactImagePath() };

	// Declare the map of absolute path files we've remapped to temp files.
	// Key is absolute path, value is temp file
	std::map<CString, CString> mapRemappedFiles;

	// The collection of created temp file names
	std::vector<CString> vecCreatedTempFileNames;

	// Iterate through the file list
	for (auto strTextFile : vecTextFiles)
	{
		// Read in the content
		CString strContent = FileUtils::ReadAllText(strTextFile);
		// True if we changed the content and need to save the file
		bool bNeedsSaving = false;

		// Search for the absolute paths in content and handle as we find them
		for (auto strPathToFind : vecAbsolutePathsToFind)
		{
			for (int nIndex = 0; (nIndex = strContent.Find(strPathToFind, nIndex)) > -1;)
			{
				// The index should point to a place in a tag that looks like
				// {NXIMAGE \\server\pracstation\images\location\logo.jpg}
				// Find the end of the tag and get the full file name
				int nEndOfTagIndex = strContent.Find('}', nIndex);
				if (nEndOfTagIndex < nIndex)
				{
					// Unexpected data; there's no closing } after the filename. Don't change anything					// There is no closing } so we can't do anything with this.
					nIndex++;
				}
				else
				{
					CString strAbsoluteFileName = strContent.Mid(nIndex, nEndOfTagIndex - nIndex);
					CString strTempFileName;

					// See if the absolute file path even exists
					if (!FileUtils::DoesFileOrDirExist(strAbsoluteFileName))
					{
						// No it does not. Just skip it
						strTempFileName = "";
						nIndex = nEndOfTagIndex;
					}
					else
					{
						// Ensure there's a temp file for the absolute file
						auto it = mapRemappedFiles.find(strAbsoluteFileName);
						if (mapRemappedFiles.end() == mapRemappedFiles.find(strAbsoluteFileName))
						{
							// Create the temp file. We must preserve the file extension so we'll
							// have to do our own testing for collisions with existing filenames
							int nCollisions = 0;
							const int nMaxCollisions = 1000;
							for (nCollisions = 0; nCollisions < nMaxCollisions;)
							{
								// Generate the filename
								UUID newId;
								UuidCreate(&newId);
								strTempFileName = GetNxTempPath() ^ FormatString("%s.%s"
									, NewUUID(true)
									, FileUtils::GetFileExtension(strAbsoluteFileName));

								// If it already exists, increment our collision list. Otherwise we're done
								if (FileUtils::DoesFileOrDirExist(strTempFileName))
								{
									nCollisions++;
								}
								else
								{
									break;
								}
							}

							// Throw an exception if we maxed out on name collisions
							if (nCollisions == nMaxCollisions)
							{
								ThrowNxException("Could not get unique temp filename for %s", strAbsoluteFileName);
							}

							// Copy the file at the absolute path to the temp path
							if (!CopyFile(strAbsoluteFileName, strTempFileName, FALSE))
							{
								CString strLastErr = FormatLastError();
								ThrowNxException(R"(Error copying %s to %s:

	%s)",
									strAbsoluteFileName,
									strTempFileName,
									strLastErr);
							}
							mapRemappedFiles.insert(std::map<CString, CString>::value_type(strAbsoluteFileName, strTempFileName));

							// Add it to the merge engine temp file list
							m_astrTempFiles.Add(strTempFileName);
							vecCreatedTempFileNames.insert(vecCreatedTempFileNames.end(), strTempFileName);
						}
						else
						{
							// It's already in the map
							strTempFileName = it->second;
						}

						// Update all references in the content
						strContent.Replace(strAbsoluteFileName, strTempFileName);

						// We need to save strTextFile when done
						bNeedsSaving = true;

						// The index is no longer reliable since the content changed. Reset it
						nIndex = 0;
					}
				}

			} // for (int nIndex = 0; (nIndex = strContent.Find(strPathToFind, nIndex)) > -1;)
		} // for (auto strPathToFind : vecAbsolutePathsToFind)

		// Save changes
		if (bNeedsSaving)
		{
			FileUtils::WriteAllText(strTextFile, strContent);
		}

	} // for (auto strTempPathName : vecTextFilesUnmodified)

	// Return the names of the added text files
	return vecCreatedTempFileNames;
}

// Throws CException, CNxException
// (z.manning 2016-06-02 15:03) - NX-100790 - Now returns a boolean
/// <summary>
/// Merges a Word document based on internal object content
/// </summary>
/// <param name="strTemplatePathName">The full path to the Word template</param>
/// <param name="vecTempFilePathNames">The full paths to all temporary files used in the merge</param>
/// <param name="strSaveFilePathName">The full path to the output file</param>
/// <param name="ppgs">The progress indicator manager</param>
/// <returns>TRUE on success; otherwise FALSE</returns>
BOOL CMergeEngine::DoMerge(const CString &strTemplatePathName, IN const std::vector<CString>& vecTempFilePathNames, IN const CString &strSaveFilePathName, IN CProgressMgr *ppgs /*= NULL*/)
{
	/* Todo: Implement the ability to selectively print/preview
	bool b = (m_nFlags & BMS_PREVIEW) ? false : true;
	*/


	if (ppgs) ppgs->SetProgress("Creating data output - please wait", 51);


	CWaitCursor wc;
	CString strMergeInfoFilePathName;

	BOOL bResult = FALSE;
	try {
		// Export to text
		strMergeInfoFilePathName = CreateMergeInfo();


		if (ppgs) ppgs->SetProgress("Checking merge style", 70);

		// If we're just exporting, then open notepad, otherwise do the full merge with word
		if (HAS_FLAG(m_nFlags, BMS_EXPORT_ONLY)) {

			if (ppgs) ppgs->SetProgress("Opening notepad to view export text", 90);

			// Don't merge, just show the exported merge info
			int nResult = GetMainFrame()->ShellExecuteModal("notepad.exe", "\"" + strMergeInfoFilePathName + "\"");
			// Report any error opening notepad
			if (nResult < 32) {
				CString str;
				str.Format("Error %li while trying to load '%s' in notepad.", nResult, strMergeInfoFilePathName);
				HandleException(NULL, str);
			}
		} else {
			// Show the Word window so that if there are problems the user can deal with them
			// (z.manning 2016-02-12 13:57) - PLID 68230 - Use the base word processor app class
			std::shared_ptr<CGenericWordProcessorApp> pApp = GetWPManager()->GetAppInstance();
			if (nullptr == pApp) {
				// (c.haag 2016-06-01 11:48) - NX-100320 - If it's null then it's not supported and an exception was not thrown
				return FALSE;
			}

			if (ppgs) ppgs->SetProgress("Performing Word merge", 80);

			//DRT 6/15/2004 - PLID 12899 - Read the notes in the PL item, it's quite extensive the research
			//	and ponderings of this problem.  Here's a (somewhat) brief summary:
			//Previously, we just called "Merge" with a flag to print while merging.  Once that finished, 
			//	it tossed the resultant file and quit the merge process.  This caused several problems:
			//	1)  Merge to Printer files are not saved in the history.  2)  Documents with RTF / Images
			//	in the merge are not actually fixed up with the special ParseNextech() function.
			//	To alleviate this, I changed the merge to merge to the screen like normal, then once it finishes,
			//	the nextech parse and the saving happen, THEN we tell MS Word to print the file out.

			// Merge
			// (z.manning 2016-02-18 09:18) - PLID 68366 - This now returns a word processor document object
			// (c.haag 2016-04-22 12:28) - NX-100276 - We no longer get a word processor document object from the merge, and
			// any work we needed to do on it is now done by Merge.

			// (a.walling 2007-12-05 16:06) - PLID 28289 - Preference to parse the headers and footers.
			// Global preference now
			BOOL bMergeFootersPreference = GetRemotePropertyInt("Merge_ParseHeaders", 0, 0, "<None>");
			// I'm also putting in special cases for per-template overrides, in case this comes in handy in the future.
			// activate by saving a copy of the template as "Template.NxFlagH.dot[x]".
			BOOL bDoMergeFootersOverride = strTemplatePathName.Find(".NxFlagH.") != -1;
			BOOL bDoNotMergeFootersOverride = strTemplatePathName.Find(".NxFlagN.") != -1;

			// (c.haag 2016-04-22 12:28) - NX-100276 - Determine the state of merging footers
			BOOL bMergeFooters = FALSE;
			// Yes, I could have combined this into a single line boolean statement, but this is much easier to understand
			// and debug.
			if (bDoMergeFootersOverride || bDoNotMergeFootersOverride) {
				// if either override exists, they take precedence over the preference, of course
				// so if both flags are specified (for some reason) then DoNot will take precedence
				bMergeFooters = bDoMergeFootersOverride && !bDoNotMergeFootersOverride;
			}
			else {
				// otherwise just use the preference
				bMergeFooters = bMergeFootersPreference;
			}

			// (c.haag 2016-04-22 12:28) - NX-100276 - Determine the state of saving the output document
			BOOL bSaveDocument = FALSE;
			if (HAS_FLAG(m_nFlags, BMS_SAVE_FILE_AND_HISTORY) || HAS_FLAG(m_nFlags, BMS_SAVE_FILE_NO_HISTORY))
			{
				ASSERT(!HAS_FLAG(m_nFlags, BMS_SAVE_HISTORY_NO_FILE));
				bSaveDocument = TRUE;
			}

			// (c.haag 2016-04-22 12:28) - NX-100276 - Determine our merge flags
			int mergeFlags = 0;
			if (bMergeFooters) mergeFlags |= CGenericWordProcessorApp::MERGEFLAG_MERGEFOOTERS;
			if (HAS_FLAG(m_nFlags, BMS_CONTINUOUS_PAGE_NUMBERING)) mergeFlags |= CGenericWordProcessorApp::MERGEFLAG_CONTINUOUS_PAGE_NUMBERING;
			if (bSaveDocument) mergeFlags |= CGenericWordProcessorApp::MERGEFLAG_SAVE_DOCUMENT;
			if (HAS_FLAG(m_nFlags, BMS_MERGETO_PRINTER)) mergeFlags |= CGenericWordProcessorApp::MERGEFLAG_MERGE_TO_PRINTER;
			if (HAS_FLAG(m_nFlags, BMS_HIDE_WORDUI)) mergeFlags |= CGenericWordProcessorApp::MERGEFLAG_HIDE_UI;

			// As this object itself maintains a collection of temp files, we must append ours to the remotely managed collection
			std::vector<CString> vecAllTempFilePathNames;
			vecAllTempFilePathNames.insert(vecAllTempFilePathNames.begin(), vecTempFilePathNames.begin(), vecTempFilePathNames.end());
			vecAllTempFilePathNames.insert(vecAllTempFilePathNames.end(), &m_astrTempFiles[0], &m_astrTempFiles[m_astrTempFiles.GetSize() - 1] + 1);

			// If we're using virtual channels then we must convert all non-temp file references into temp file references.
			// This must be done here because it is Practice specific and cannot be entirely done from a library. This will
			// update m_astrTempFiles
			if (WordProcessorType::VTSCMSWord == GetWordProcessorType())
			{
				// Gather the list of text and hypertext files
				std::vector<CString> vecTextFiles;
				vecTextFiles.insert(vecTextFiles.end(), strMergeInfoFilePathName);
				for (auto strTempPathName : vecAllTempFilePathNames)
				{
					// Determine if the file is hypertext and add it to vecTextFiles if it is
					const CiString strRemoteFileExtension = FileUtils::GetFileExtension(strTempPathName);
					if (strRemoteFileExtension == "htm" || strRemoteFileExtension == "html")
					{
						vecTextFiles.insert(vecTextFiles.end(), strTempPathName);
					}
				}
					
				// Create a temp copy of all absolutely pathed files and add the list of those temp file names to the full collection.
				// By this point they've already been added to m_astrTempFiles
				std::vector<CString> vecCreatedTempFileNames = RemapAbsoluteFileReferencesToTempFiles(vecTextFiles);
				vecAllTempFilePathNames.insert(vecAllTempFilePathNames.end(), vecCreatedTempFileNames.begin(), vecCreatedTempFileNames.end());
			}

			if ((HAS_FLAG(m_nFlags, BMS_MERGETO_SCREEN))) {
				bResult = pApp->Merge(strTemplatePathName, strMergeInfoFilePathName, vecAllTempFilePathNames, strSaveFilePathName, CGenericWordProcessorApp::mtNewDocument, mergeFlags, "",
					[&ppgs](LPCTSTR szText, long nProgress) {
					if (ppgs) ppgs->SetProgress(szText, nProgress);
				});
			} else if (HAS_FLAG(m_nFlags, BMS_MERGETO_PRINTER)) {
				//try {
					//DRT 6/15/2004 - PLID 12899 - We now merge to the screen, parse our images / rich text, 
					//	save the document, THEN print it out.  Previously we were not saving or correctly
					//	handling the special merge features.
				bResult = pApp->Merge(strTemplatePathName, strMergeInfoFilePathName, vecAllTempFilePathNames, strSaveFilePathName, CGenericWordProcessorApp::mtNewDocument, mergeFlags, "",
						[&ppgs](LPCTSTR szText, long nProgress) {
						if (ppgs) ppgs->SetProgress(szText, nProgress);
					});
				//}
				//PLID 12899 - Since we aren't printing anymore, I think this extra try/catch can go, but it
				//	can't hurt, so I'm leaving it here.
				// (c.haag 2016-02-25) - PLID 68416 - No, it should definitely go since we're not printing
				//catch (COleDispatchException *e) {
					// This happens if the user cancelled the print job
				//	if (e->m_scError == 0x800a1066) {
				//		e->Delete();
				//	}
				//	else throw e;
				//}
			} else if (HAS_FLAG(m_nFlags, BMS_MERGETO_EMAIL_PLAIN)) {
				bResult = pApp->Merge(strTemplatePathName, strMergeInfoFilePathName, vecAllTempFilePathNames, strSaveFilePathName, CGenericWordProcessorApp::mtEMail, mergeFlags | CGenericWordProcessorApp::MERGEFLAG_EMAIL_PLAIN_TEXT, m_strSubjectMatter,
					[&ppgs](LPCTSTR szText, long nProgress) {
					if (ppgs) ppgs->SetProgress(szText, nProgress);
				});
			} else if (HAS_FLAG(m_nFlags, BMS_MERGETO_EMAIL_HTML)) {
				bResult = pApp->Merge(strTemplatePathName, strMergeInfoFilePathName, vecAllTempFilePathNames, strSaveFilePathName, CGenericWordProcessorApp::mtEMail, mergeFlags, m_strSubjectMatter,
					[&ppgs](LPCTSTR szText, long nProgress) {
					if (ppgs) ppgs->SetProgress(szText, nProgress);
				});
			} else {
				bResult = FALSE;
				AfxThrowNxException("Could not determine output format (i.e. screen, printer, email, or fax).  Flags = %li", m_nFlags);
			}
		}
		
		// Whether the merge was successful or not, we're done with the mergeinfo text file so delete it
		DeleteFileWhenPossible(strMergeInfoFilePathName);

		// (c.haag 2016-05-31 15:49) - NX-100310 - Made this into a silent CallThrow. There's no reason to show an ugly exception box twice
	} NxCatchAllSilentCallThrow(DeleteFile(strMergeInfoFilePathName));

	// (z.manning 2016-06-02 15:09) - NX-100790 - Return the boolean result
	return bResult;
}	

// Throws CException, CNxException
CString CMergeEngine::CreateMergeInfo()
{
	if (!m_strResultsT.IsEmpty()) {
		// If the path doesn't exist, try to create it
		CString strPath = GetNxTempPath();
		if (DoesExist(strPath)) {
			CString strPathName = GetTempFileName("MergeInfo.nxt", 'm', strPath);
			if (strPathName != "") {
				try {

					// (j.jones 2013-01-25 09:22) - PLID 54841 - increase the timeout to 3 minutes
					CIncreaseCommandTimeout cict(180);

					// Then transfer the results to the text file
					SaveTableToText(m_strResultsT, strPathName, m_pfnCallbackExtraFields, m_pCallbackParam, m_nMergeFieldCountLimit, &m_bDroppedFieldsPastLimit);
					// Remember the name so that we can delete the info file in the destructor
					m_astrTempFiles.Add(strPathName);
					// Return success
					return strPathName;
				} catch (CException *e) {
					// Delete the file because we don't want it there after failure
					DeleteFile(strPathName);
					// Throw the exception so it can be caught by the caller
					throw e;
				} catch (_com_error e) {
					DeleteFile(strPathName);
					throw e;
				}
			} else {
				AfxThrowNxException("Could not create temp file name for %s", strPath ^ "MergeInfo.nxt");
			}
		} else {
			AfxThrowNxException("Could not create temp path %s.  GetLastError returned 2.%lu", strPath, GetLastError());
		}
	} else {
		AfxThrowNxException("Cannot create MergeInfo without a query.", __LINE__, __FILE__);
	}

	return _T("");
}

long CalcMergeFieldCountLimit()
{
	// (z.manning 2016-02-22 11:25) - PLID 68394 - We no longer support a version of Word old enough
	// where this matters. So just return -1 in all cases.
	return -1;
}

CString CMergeEngine::CreateBlankMergeInfo(long nFlags, OPTIONAL LPEXTRAMERGEFIELDSFUNC pfnCallbackExtraFields, OPTIONAL LPVOID pCallbackParam)
{
	CString strAns;

	// Create a merge engine object based on the just the fields, and no records
	CMergeEngine mi;
	mi.m_nFlags = nFlags;
	mi.m_pfnCallbackExtraFields = pfnCallbackExtraFields;
	mi.m_pCallbackParam = pCallbackParam;
	mi.m_nMergeFieldCountLimit = CalcMergeFieldCountLimit();
	// Create the temp query
	mi.m_strResultsT = mi.BuildMergeFields("BugusTableName");	
	// Create the merge info based on the one-record temp query
	strAns = mi.CreateMergeInfo();

	//TES 3/2/2004: Don't let mi's destructor delete our temp file!
	for(int i = 0; i < mi.m_astrTempFiles.GetSize(); i++) {
		if(mi.m_astrTempFiles.GetAt(i) == strAns) mi.m_astrTempFiles.RemoveAt(i);
	}

	// See if we've had to limit the fields
	if (mi.m_nMergeFieldCountLimit != -1 && mi.m_bDroppedFieldsPastLimit) {
		// We did drop some fields, so we have to tell the user
		CString strMsg;
		strMsg.Format(
			"Versions of Microsoft Word prior to Office XP cannot handle more than %li merge fields "
			"when editing templates.  The set of merge fields currently being sent to Word was larger "
			"than this limit, so it has been truncated to %li fields.", 
			mi.m_nMergeFieldCountLimit, mi.m_nMergeFieldCountLimit);
		AfxMessageBox(strMsg, MB_OK|MB_ICONEXCLAMATION);
	}

	return strAns;
}

const CString &CMergeEngine::GetResultsQ()
{
	return m_strResultsT;
}

const CString &CMergeEngine::GetGroupListQ()
{
	return m_strGroupListQ;
}

/* // Todo: When we enable the fields that rely on billing, this 
	// query might be a good place to start.  It looks at payments.
SELECT LineItemT.PatientID, LineItemT.Date, Sum(LineItemT.Amount) AS SumOfAmount
FROM LineItemT
WHERE (((LineItemT.Deleted) Is Null Or (LineItemT.Deleted)<>True) AND ((LineItemT.Type)=1))
GROUP BY LineItemT.PatientID, LineItemT.Date
ORDER BY LineItemT.Date DESC;
*/

/* We discovered a MUCH better way of getting the Reservation ID of the 
// NEXT appointment for a given patient.  It's super fast and it's very 
// consistent.  One of these days we need to implement it, if only maybe 
// in Practice 2001 because it works already in Practice 2000
SELECT First(Patients.ID) AS PatientID, CLng(Right(Min(Format(CDate([Reservations].[Date] & ' ' & [ReservationDetailsT].[StartTime]),"yyyy-mm-dd hh:nn:ss") & ' ' & Format([Reservations].[ID],"000000000000")),12)) AS ResID
FROM Patients INNER JOIN (Reservations LEFT JOIN ReservationDetailsT ON Reservations.ID = ReservationDetailsT.ReservationID) ON Patients.ID = Reservations.PatientID
WHERE (((Reservations.Date)>=Now()) AND ((ReservationDetailsT.Status)<>4))
GROUP BY Reservations.PatientID;
*/

// Checks the data to get a unique document name for the given patient
// Throws CException
// (c.haag 2006-06-20 17:27) - PLID 15556 - This is one of two versions. The second
// version takes in a patient name and user-defined ID, and does not perform database
// accesses. If you change this function, you must change the other as well.
CString GetPatientDocumentName(long nPatientId, CString strExt /* = "doc" */)
{
	//TES 9/18/2008 - PLID 31413 - Call the HistoryUtils version (that's shared with NxServer).
	return GetPatientDocumentName(GetRemoteData(), nPatientId, strExt);
}

// (c.haag 2006-06-20 17:27) - PLID 15556 - This is second of two versions. The first
// version only takes in a patient id, and will access the database. If you change this
// function, make sure you change the other as well.
CString GetPatientDocumentName(long nPatientId, const CString& strFirst, const CString& strLast, unsigned long nUserDefinedID, CString strExt /*= "doc" */)
{
	//TES 9/18/2008 - PLID 31413 - Call the HistoryUtils version (that's shared with NxServer).
	return GetPatientDocumentName(GetRemoteData(), nPatientId, strFirst, strLast, nUserDefinedID, strExt);
}

// Returns the sum of the sizes of all files in the array that could be found on the hd
// If no files in the array, or if none could be found, then returns -1
long GetFileSizeSum(const CStringArray &arystrFilePaths)
{
	// Calculate the sum of the sizes of all files, and the count of files we could find
	long nCountFound = 0;
	long nSizeSum = 0;
	long nCount = arystrFilePaths.GetSize();
	for (long i=0; i<nCount; i++) {
		CFileFind finder;
		if (finder.FindFile(arystrFilePaths.GetAt(i))) {
			// We have to call find next, so get the information about the current file
			finder.FindNextFile();
			
			// Add this file's size to the sum
			//TES 11/7/2007 - PLID 27979 - VS2008 - This now returns a ULONGLONG; casting it as a long can't hurt
			// because our caller already knows, from the return value, that this function can only handle sizes
			// up to LONG_MAX.
			nSizeSum += (long)finder.GetLength();

			// Increment the count of found files
			nCountFound++;
		}
	}

	// Return the total size, or -1 if no files were found
	if (nCountFound >= 1) {
		return nSizeSum;
	} else {
		return -1;
	}
}

// The boolean parameter bCancel will be set to true if 
// the user clicked cancel, false if she chose to proceed
// Returns true to attach, false not to attach
bool ShouldAttachBatchToPatientHistory(IN const CStringArray &arystrTemplateFilePathNames, IN const CString &strGroupListT, OUT bool &bCancel)
{
	static const int cnSizeBig = 1;

	// Default to not attaching
	bool bAns = false;

	// Find file size
	long nFileSize = GetFileSizeSum(arystrTemplateFilePathNames);
	if (nFileSize != -1) {
		// Calculate the total anticipated size (file size sum times number of elements being merged)
		long nTotalSize = (nFileSize * GetRecordCount("SELECT * FROM " + strGroupListT)) >> 20;
		
		// Decide whether to save based on file size
		if (nTotalSize > cnSizeBig) {
			int nResult = MsgBox(MB_ICONQUESTION|MB_YESNOCANCEL|MB_DEFBUTTON2, 
				"This mail merge is estimated to use %i MB.  Because of its large "
				"anticipated size, it is recommended that you do not attach it to "
				"patient histories.\n\nDo you wish to attach it anyway?", nTotalSize);

			switch (nResult) {
			case IDYES:
				// The user wants to continue and attach
				bCancel = false;
				bAns = true;
				break;
			case IDNO:
				// The user wants to continue but not attach
				bCancel = false;
				bAns = false;
				break;
			case IDCANCEL:
			default:
				// The user doesn't want to continue
				bCancel = true;
				bAns = false;
				break;
			}
		} else {
			bCancel = false;
			bAns = true;
		}
	}

	// Return true to attach, false to not
	return bAns;
}

// Fast version calls our own export routines
void SaveTableToText(const CString &strTableName, const CString &strOutFilePathName, OPTIONAL LPEXTRAMERGEFIELDSFUNC pfnCallbackExtraFields, OPTIONAL LPVOID pCallbackParam, OPTIONAL long nFieldCountLimit, OPTIONAL OUT BOOL *pbDroppedFieldsPastLimit) 
{
	CWaitCursor wc;

	// Export the table
	CStoreTableEngine ste;
	ste.m_pfnCallbackExtraFields = pfnCallbackExtraFields;
	ste.m_pCallbackParam = pCallbackParam;
	ste.SaveTable(strTableName, strOutFilePathName, nFieldCountLimit, pbDroppedFieldsPastLimit);
}

// Fast version calls our own export routines
void SaveHeadersToText(const CString &strTableName, const CString &strOutFilePathName, OPTIONAL LPEXTRAMERGEFIELDSFUNC pfnCallbackExtraFields, OPTIONAL LPVOID pCallbackParam, OPTIONAL long nFieldCountLimit, OPTIONAL OUT BOOL *pbDroppedFieldsPastLimit) 
{
	CWaitCursor wc;

	// Export the table
	CStoreTableEngine ste;
	ste.m_pfnCallbackExtraFields = pfnCallbackExtraFields;
	ste.m_pCallbackParam = pCallbackParam;
	ste.SaveHeaders(strTableName, strOutFilePathName, nFieldCountLimit, pbDroppedFieldsPastLimit);
}

/////////////////////////////////////////////////////
///// CStoreTableEngine

CStoreTableEngine::CStoreTableEngine()
{ 
	m_nFieldMax = 0; 
	m_nOutLen = 0; 
	m_pfnCallbackExtraFields = NULL;
	m_pCallbackParam = NULL;
}

long FindNthUnquotedComma(LPCTSTR strCommaSeparatedRecord, long nNthCommaToFind)
{
	long nCommasFound = 0;
	
	BOOL bInQuotes = false;
	for (long i=0; ; i++) {
		switch (strCommaSeparatedRecord[i]) {
		case _T('\0'):
			// End of string and we haven't found enough commas
			return -1;
			break;
		case _T('\"'):
			// A quotation mark, change the "in quotes" flag
			bInQuotes = !bInQuotes;
			break;
		case _T(','):
			// Yay, a comma.  If we're not in quotes, increment our count and see if we've hit the requested count
			if (!bInQuotes) {
				nCommasFound++;
				if (nCommasFound == nNthCommaToFind) {
					// Sweet, we found the "nth" unquoted comma so return this position
					return i;
				}
			}
			break;
		default:
			// Ignore all other characters, just keep looping ahead
			break;
		}
	}
}

CString ExtractFirstNFields(IN const CString &strCommaSeparatedRecord, long nMaxFieldCount, OUT BOOL &bMoreFieldsAvailPastMax)
{
	if (nMaxFieldCount != -1) {
		// Need to limit the return string to only contain the first nMaxFieldCount fields
		if (nMaxFieldCount == 0) {
			// The caller asked for NO fields to be returned.
			if (!strCommaSeparatedRecord.IsEmpty()) {
				// There were some fields, and we've been asked to send none, so that means there were 
				// some past what we were asked for.
				bMoreFieldsAvailPastMax = TRUE;
			} else {
				// There were no fields in the first place
				bMoreFieldsAvailPastMax = FALSE;
			}
			return "";
		} else {
			// Allow some fields, but limit to nMaxFieldCount
			long nNthCommaPos = FindNthUnquotedComma(strCommaSeparatedRecord, nMaxFieldCount);
			if (nNthCommaPos != -1) {
				// Found the Nth comma, which by CSV definition is the first character following the 
				// Nth field and that there were more fields after it.
				bMoreFieldsAvailPastMax = TRUE;
				return strCommaSeparatedRecord.Left(nNthCommaPos); // drop the comma and everything after it
			} else {
				// There aren't enough commas to reach our limit, which means we have <= nMaxFieldCount 
				// fields so we're all set.
				bMoreFieldsAvailPastMax = FALSE;
				return strCommaSeparatedRecord;
			}
		}
	} else {
		// -1 is the sentinel value indicating unlimited fields, so just return the string as is
		bMoreFieldsAvailPastMax = FALSE;
		return strCommaSeparatedRecord;
	}
}

void CStoreTableEngine::WriteFieldNames(OPTIONAL long nFieldCountLimit, OPTIONAL BOOL *pbDroppedFieldsPastLimit)
{
	BOOL bMoreFieldsAvailPastMax = FALSE;

	// Loop through each field
	if (m_nFieldMax > 0) {
		long nFieldsSentSoFar = 0;
		for (int i=0; i<=m_nFieldMax; i++) {
			// Output the field name in quotes
			// (j.jones 2011-10-20 09:56) - PLID 38339 - this is now a CArray of ADODB::FieldPtr
			m_nOutLen = sprintf(m_pstrOutText, "\"%s\"", (LPCTSTR)(m_aryFieldPtrs.GetAt(i)->Name));
			m_fileOut.Write(m_pstrOutText, m_nOutLen);
			nFieldsSentSoFar++;

			// Output a comma if there are more fields
			if (i != m_nFieldMax) {
				// More fields to process
				
				// Make sure we're not at the limit
				if (nFieldCountLimit != -1 && nFieldsSentSoFar >= nFieldCountLimit) {
					// We hit our limit, so stop even though we know there are more fields to process
					bMoreFieldsAvailPastMax = TRUE;
					break;
				}

				// More fields to process, so add the comma
				m_fileOut.Write(",", 1);
			}
		}
		// Insert any extra field names here
		if (m_pfnCallbackExtraFields) {
			// Get the comma-delimited string from the callback function
			CString strExtraFieldNames = m_pfnCallbackExtraFields(TRUE, "", m_pCallbackParam);
			// Now try to cut them off at our max field count limit
			{
				BOOL bDroppedAnyExtraFields = FALSE;
				long nMaxExtraFieldCount = (nFieldCountLimit != -1) ? 
					((nFieldsSentSoFar < nFieldCountLimit) ? (nFieldCountLimit - nFieldsSentSoFar) : 0) : -1;
				// Extract the first nMaxExtraFieldCount fields
				strExtraFieldNames = ExtractFirstNFields(strExtraFieldNames, nMaxExtraFieldCount, bDroppedAnyExtraFields);
				// If we dropped any, we have to set bMoreFieldsAvailPastMax to TRUE too, but if we didn't, 
				// we need to leave bMoreFieldsAvailPastMax as is (it may be TRUE or FALSE).
				if (bDroppedAnyExtraFields) {
					bMoreFieldsAvailPastMax = TRUE;
				}
			}
			// Now if there are any extra fields left to add, add them
			if (!strExtraFieldNames.IsEmpty()) {
				m_fileOut.Write(",", 1);
				m_fileOut.Write(strExtraFieldNames, strExtraFieldNames.GetLength());
			}
		}
		// Terminate with a carriage return
		m_fileOut.Write("\r\n", 2);
	}

	// If the caller wants it, return the boolean of whether fields were dropped
	if (pbDroppedFieldsPastLimit) {
		*pbDroppedFieldsPastLimit = bMoreFieldsAvailPastMax;
	}
}

#ifdef _DEBUG
static const struct { char strDataTypeEnumName[19]; short dte; } lc_aryDataTypeEnumNames[40] = {
	{ "adEmpty", 0 }, 
	{ "adTinyInt", 16 }, 
	{ "adSmallInt", 2 }, 
	{ "adInteger", 3 }, 
	{ "adBigInt", 20 }, 
	{ "adUnsignedTinyInt", 17 }, 
	{ "adUnsignedSmallInt", 18 }, 
	{ "adUnsignedInt", 19 }, 
	{ "adUnsignedBigInt", 21 }, 
	{ "adSingle", 4 }, 
	{ "adDouble", 5 }, 
	{ "adCurrency", 6 }, 
	{ "adDecimal", 14 }, 
	{ "adNumeric", 131 }, 
	{ "adBoolean", 11 }, 
	{ "adError", 10 }, 
	{ "adUserDefined", 132 }, 
	{ "adVariant", 12 }, 
	{ "adIDispatch", 9 }, 
	{ "adIUnknown", 13 }, 
	{ "adGUID", 72 }, 
	{ "adDate", 7 }, 
	{ "adDBDate", 133 }, 
	{ "adDBTime", 134 }, 
	{ "adDBTimeStamp", 135 }, 
	{ "adBSTR", 8 }, 
	{ "adChar", 129 }, 
	{ "adVarChar", 200 }, 
	{ "adLongVarChar", 201 }, 
	{ "adWChar", 130 }, 
	{ "adVarWChar", 202 }, 
	{ "adLongVarWChar", 203 }, 
	{ "adBinary", 128 }, 
	{ "adVarBinary", 204 }, 
	{ "adLongVarBinary", 205 }, 
	{ "adChapter", 136 }, 
	{ "adFileTime", 64 }, 
	{ "adDBFileTime", 137 }, 
	{ "adPropVariant", 138 }, 
	{ "adVarNumeric", 139 }
};
CString DebugGetDataTypeEnumName(short dte) {
	for (long i=0; i<40; i++) {
		if (lc_aryDataTypeEnumNames[i].dte == dte) {
			return lc_aryDataTypeEnumNames[i].strDataTypeEnumName;
		}
	}
	return "Undefined Type";
}

static const struct { char strVariantName[20]; unsigned short vt; } lc_aryVariantTypeNames[51] = {
	{ "VT_NULL", 0 },
	{ "VT_EMPTY", 0 }, 
	{ "VT_NULL", 1 }, 
	{ "VT_I2", 2 }, 
	{ "VT_I4", 3 }, 
	{ "VT_R4", 4 }, 
	{ "VT_R8", 5 }, 
	{ "VT_CY", 6 }, 
	{ "VT_DATE", 7 }, 
	{ "VT_BSTR", 8 }, 
	{ "VT_DISPATCH", 9 }, 
	{ "VT_ERROR", 10 }, 
	{ "VT_BOOL", 11 }, 
	{ "VT_VARIANT", 12 }, 
	{ "VT_UNKNOWN", 13 }, 
	{ "VT_DECIMAL", 14 }, 
	{ "VT_I1", 16 }, 
	{ "VT_UI1", 17 }, 
	{ "VT_UI2", 18 }, 
	{ "VT_UI4", 19 }, 
	{ "VT_I8", 20 }, 
	{ "VT_UI8", 21 }, 
	{ "VT_INT", 22 }, 
	{ "VT_UINT", 23 }, 
	{ "VT_VOID", 24 }, 
	{ "VT_HRESULT", 25 }, 
	{ "VT_PTR", 26 }, 
	{ "VT_SAFEARRAY", 27 }, 
	{ "VT_CARRAY", 28 }, 
	{ "VT_USERDEFINED", 29 }, 
	{ "VT_LPSTR", 30 }, 
	{ "VT_LPWSTR", 31 }, 
	{ "VT_RECORD", 36 }, 
	{ "VT_FILETIME", 64 }, 
	{ "VT_BLOB", 65 }, 
	{ "VT_STREAM", 66 }, 
	{ "VT_STORAGE", 67 }, 
	{ "VT_STREAMED_OBJECT", 68 }, 
	{ "VT_STORED_OBJECT", 69 }, 
	{ "VT_BLOB_OBJECT", 70 }, 
	{ "VT_CF", 71 }, 
	{ "VT_CLSID", 72 }, 
	{ "VT_VERSIONED_STREAM", 73 }, 
	{ "VT_BSTR_BLOB", 0xfff }, 
	{ "VT_VECTOR", 0x1000 }, 
	{ "VT_ARRAY", 0x2000 }, 
	{ "VT_BYREF", 0x4000 }, 
	{ "VT_RESERVED", 0x8000 }, 
	{ "VT_ILLEGAL", 0xffff }, 
	{ "VT_ILLEGALMASKED", 0xfff }, 
	{ "VT_TYPEMASK", 0xfff }
};
CString DebugGetVariantTypeName(VARTYPE vt) {
	for (long i=0; i<40; i++) {
		if (lc_aryVariantTypeNames[i].vt == vt) {
			return lc_aryVariantTypeNames[i].strVariantName;
		}
	}
	return "Undefined Type";
}
#endif

void CStoreTableEngine::WriteValue(const _variant_t *pvar, short nType)
{
	// Empty the string
	m_pstrOutText[0] = '\0';
	m_nOutLen = 0;

	if (pvar->vt != VT_NULL) {
		// Fill it with appropriate output text
		switch (nType) {
		case adBoolean:
			m_nOutLen = sprintf(m_pstrOutText, "%s", VarBool(*pvar) ? "True" : "False");
			break;
		case adTinyInt:
			m_nOutLen = sprintf(m_pstrOutText, "%i", VarByte(*pvar));
			break;
		case adSmallInt:
			m_nOutLen = sprintf(m_pstrOutText, "%i", VarShort(*pvar));
			break;
		case adInteger:
			m_nOutLen = sprintf(m_pstrOutText, "%li", VarLong(*pvar));
			break;
		case adCurrency:
			{
				CString strTemp(FormatCurrencyForInterface(VarCurrency(*pvar), TRUE, TRUE));
				strTemp.Replace("\"", "\"\"");
				m_nOutLen = sprintf(m_pstrOutText, "\"%s\"", strTemp);
			}
			break;
		case adSingle:
			m_nOutLen = sprintf(m_pstrOutText, "%f", VarFloat(*pvar));
			break;
		case adDouble:
			m_nOutLen = sprintf(m_pstrOutText, "%lf", VarDouble(*pvar));
			break;
		case adDate:
		case adDBTimeStamp:
			{
				CString strTemp(FormatDateTimeForInterface(VarDateTime(*pvar), DTF_STRIP_SECONDS, dtoNaturalDatetime, true));
				strTemp.Replace("\"", "\"\"");
				m_nOutLen = sprintf(m_pstrOutText, "\"%s\"", strTemp);
			}
			break;
		case adBSTR:
		case adVarWChar:
		case adVarChar:
		case adWChar:
		case adLongVarWChar:
			{
				
				CString strTemp(VarString(*pvar));
                // (j.kuziel 2011-10-24 16:03) - PLID 45613 - Escape merge info file delimeter characters.
				strTemp = ConvertToQuotableWordCSVString(strTemp);

				m_nOutLen = sprintf(m_pstrOutText, "\"%s\"", strTemp);
			}
			break;
		default:
#ifdef _DEBUG
			AfxThrowNxException(
				"An unknown data type was encountered while exporting records.\n\n"
				"DataType = %s (%li)\n"
				"Value = %s (%s)", 
				DebugGetDataTypeEnumName(nType), nType,
				AsString(*pvar), DebugGetVariantTypeName(pvar->vt));
#else
			AfxThrowNxException(
				"An unknown data type was encountered while exporting records.\n\n"
				"DataType = %li\n"
				"Value = %s", 
				nType, AsString(*pvar));
#endif
			break;
		}
	}

	m_fileOut.Write(m_pstrOutText, m_nOutLen);
}

void CStoreTableEngine::WriteFieldValue(int nFieldIndex)
{
	// (j.jones 2011-10-20 09:56) - PLID 38339 - this is now a CArray of ADODB::FieldPtr
	m_varFieldValue = m_aryFieldPtrs.GetAt(nFieldIndex)->Value;
	WriteValue(&m_varFieldValue, m_aryFieldPtrs.GetAt(nFieldIndex)->Type);
}

void CStoreTableEngine::WriteRecords(OPTIONAL long nFieldCountLimit)
{
	BOOL bMoreFieldsAvailPastMax = FALSE;

	// Loop through each record
	short i;
	while (!m_pRecordset->eof) {
		// Loop through each field
		if (m_nFieldMax > 0) {
			long nFieldsSentSoFar = 0;
			for (i=0; i<=m_nFieldMax; i++) {
				// Output the field value
				WriteFieldValue(i);
				nFieldsSentSoFar++;

				// Output a comma if there are more fields
				if (i != m_nFieldMax) {
					// More fields to process
					
					// Make sure we're not at the limit
					if (nFieldCountLimit != -1 && nFieldsSentSoFar >= nFieldCountLimit) {
						// We hit our limit, so stop even though we know there are more fields to process
						bMoreFieldsAvailPastMax = TRUE;
						break;
					}
	
					// More fields to process, so add the comma
					m_fileOut.Write(",", 1);
				}
			}
			// Insert any extra field values here
			if (m_pfnCallbackExtraFields) {
				// Get the comma-delimited string from the callback function
				CString strExtraFieldValues = m_pfnCallbackExtraFields(FALSE, "", m_pCallbackParam);
				// Now try to cut them off at our max field count limit
				{
					BOOL bDroppedAnyExtraFields = FALSE;
					long nMaxExtraFieldCount = (nFieldCountLimit != -1) ? 
						((nFieldsSentSoFar < nFieldCountLimit) ? (nFieldCountLimit - nFieldsSentSoFar) : 0) : -1;
					// Extract the first nMaxExtraFieldCount fields
					strExtraFieldValues = ExtractFirstNFields(strExtraFieldValues, nMaxExtraFieldCount, bDroppedAnyExtraFields);
					// If we dropped any, we have to set bMoreFieldsAvailPastMax to TRUE too, but if we didn't, 
					// we need to leave bMoreFieldsAvailPastMax as is (it may be TRUE or FALSE).
					if (bDroppedAnyExtraFields) {
						bMoreFieldsAvailPastMax = TRUE;
					}
				}
				// Now if there are any extra fields left to add, add them
				if (!strExtraFieldValues.IsEmpty()) {
					m_fileOut.Write(",", 1);
					m_fileOut.Write(strExtraFieldValues, strExtraFieldValues.GetLength());
				}
			}
			// Write the carriage return that terminates this record
			m_fileOut.Write("\r\n", 2);
		}

		// Move to the next record
		m_pRecordset->MoveNext();
	}
}

void CStoreTableEngine::LoadFieldInfo()
{
	// Get field maximum index
	FieldsPtr flds = m_pRecordset->Fields;
	m_nFieldMax = flds->GetCount()-1;

	m_aryFieldPtrs.RemoveAll();

	// Loop through all the fields to put all the info into the array
	for (long i=0; i<=m_nFieldMax; i++) {
		// Get the field info and put it into the array
		// (j.jones 2011-10-20 09:56) - PLID 38339 - this is now a CArray of ADODB::FieldPtr
		m_aryFieldPtrs.Add(flds->Item[i]);
	}
}

// Throws CException
void CStoreTableEngine::SaveTable(const CString &strTableName, const CString &strOutputFilePathName, OPTIONAL long nFieldCountLimit, OPTIONAL OUT BOOL *pbDroppedFieldsPastLimit)
{
	CException *eThrowMe = NULL;
	HRESULT hrComThrowMe = NULL;
	IErrorInfo *peiComThrowMe = NULL;

	try {
		// Try to open the output file
		CFileException *e = new CFileException;
		if (!m_fileOut.Open(strOutputFilePathName, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite, e)) {
			throw e;
		} else {
			// We're done with our little exception
			e->Delete();
			e = NULL;
		}

		// Open the recordset
		m_pRecordset = CreateRecordsetStd(strTableName);

		// Write all the field headers
		LoadFieldInfo();
		WriteFieldNames(nFieldCountLimit, pbDroppedFieldsPastLimit);

		// Write all records
		WriteRecords(nFieldCountLimit);
	} catch (CException *e) {
		// Catch all exceptions so we are sure to thrown them after everything gets closed
		eThrowMe = e;
	} catch (_com_error e) {
		hrComThrowMe = e.Error();
		peiComThrowMe = e.ErrorInfo();
	}

	// TODO: Close EVERYTHING that we opened
	if (m_fileOut.m_hFile != CFile::hFileNull) m_fileOut.Close();

	// Now throw the exception if there was one
	if (eThrowMe) {
		throw eThrowMe;
	}
	if (hrComThrowMe) {
		_com_raise_error(hrComThrowMe, peiComThrowMe);
	}
}

// Throws CException
void CStoreTableEngine::SaveHeaders(const CString &strTableName, const CString &strOutputFilePathName, OPTIONAL long nFieldCountLimit, OPTIONAL OUT BOOL *pbDroppedFieldsPastLimit)
{
	CException *eThrowMe = NULL;
	HRESULT hrComThrowMe = NULL;
	IErrorInfo *peiComThrowMe = NULL;

	try {
		// Try to open the output file
		CFileException *e = new CFileException;
		if (!m_fileOut.Open(strOutputFilePathName, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite, e)) {
			throw e;
		} else {
			// We're done with our little exception
			e->Delete();
			e = NULL;
		}

		// Write all the field headers
		m_pRecordset = CreateRecordsetStd(strTableName);
		LoadFieldInfo();
		WriteFieldNames(nFieldCountLimit, pbDroppedFieldsPastLimit);
	} catch (CException *e) {
		// Catch all exceptions so we are sure to thrown them after everything gets closed
		eThrowMe = e;
	} catch (_com_error e) {
		hrComThrowMe = e.Error();
		peiComThrowMe = e.ErrorInfo();
	}

	// TODO: Close EVERYTHING that we opened
	if (m_fileOut.m_hFile != CFile::hFileNull) m_fileOut.Close();

	// Now throw the exception if there was one
	if (eThrowMe) {
		throw eThrowMe;
	}
	if (hrComThrowMe) {
		_com_raise_error(hrComThrowMe, peiComThrowMe);
	}
}

// (r.gonet 05/15/2012) - PLID 52680 will pass this dwTickCount value
void CMergeEngine::ConvertToRTFField(IDispatch* rs, CString& strQuery, const CString& strTable, const CString& strField,
									 long nProcInfoID, bool bRetainFormatting,DWORD dwTickCount)
{
	_RecordsetPtr prs(rs);
	CString strFilename;
	//long n = strQuery.Find(strTable + "." + strField);
	if (0 == m_arydwProcInfoIDs.GetSize()) return;

	// Generate our temporary file
	// (r.gonet 05/15/2012) - PLID 47153 - Add a tick count to the end of the filename to prevent sharing violations with Word if this
	// same filename is reused later.
	// (S.Dhole 09/17/2012) - PLID 52680  
	//DWORD dwTickCount = GetTickCount();
	strFilename.Format("MergeRTF_Temp_%s_%s_%d_%lu.rtf", strTable, strField, nProcInfoID, dwTickCount);
	CString strFullPath = GetNxTempPath() ^ strFilename;
	CStdioFile f(strFullPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareCompat);

	// Write the rich text to the file
	CString strRTF = VarString(prs->Fields->Item[_bstr_t(strField)]->Value, "");
	f.WriteString(strRTF);
	f.Close();
	
	// Add to our temp file list
	m_astrTempFiles.Add(strFullPath);

	// Change the table to access the file
	CString strReplace;
	//m.hancock - 12/1/2005 - PLID 18478 - Some RTF merge fields should retain their formatting.
	//If bRetainFormatting is true, we need to set a different RTF tag designating that we should
	//not use the merge field's font attributes from the template.
	CString strTag = "NXRTF";
	if(bRetainFormatting)
		strTag = "NXRTFRETAINFORMATTING";
	//strReplace.Format("'{NXRTF %sMergeRTF_Temp_%s_%s_' + convert(nvarchar, ProcInfoDetailsT.ID) + '.rtf}' AS %s", GetNxTempPath() ^ "", strTable, strField, strField);
	// (r.gonet 05/15/2012) - PLID 47153 - Add a tick count to the end of the filename to prevent sharing violations with Word if this
	strReplace.Format("'{%s %sMergeRTF_Temp_%s_%s_' + convert(nvarchar, ProcInfoDetailsT.ID) + '_%lu.rtf}' AS %s", strTag, GetNxTempPath() ^ "", strTable, strField, dwTickCount, strField);
	if (-1 == strQuery.Find(strReplace))
		strQuery.Replace(strTable + "." + strField, strReplace);
}


class CLateBoundFind
{
public:
	void operator =(IDispatchPtr pSource) {
		m_pObject = pSource;
		m_pDriver = new COleDispatchDriver(pSource);
	}

	void ClearFormatting()
	{
		DISPID dispid;
		LPOLESTR strName = OLESTR("ClearFormatting");;
		m_pObject->GetIDsOfNames(IID_NULL, &strName, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
		m_pDriver->InvokeHelper(dispid, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	BOOL Execute(VARIANT* FindText, VARIANT* MatchCase, VARIANT* MatchWholeWord, VARIANT* MatchWildcards, VARIANT* MatchSoundsLike, VARIANT* MatchAllWordForms, VARIANT* Forward)
	{
		DISPID dispid;
		LPOLESTR strName = OLESTR("Execute");
		m_pObject->GetIDsOfNames(IID_NULL, &strName, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
		static BYTE parms[] =
			VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT;
		BOOL bResult;
		m_pDriver->InvokeHelper(dispid, DISPATCH_METHOD, VT_BOOL, (void*)&bResult, parms, 
			FindText, MatchCase, MatchWholeWord, MatchWildcards, MatchSoundsLike, MatchAllWordForms, Forward);
		return bResult;
	}
	CLateBoundFind() {
		m_pDriver = NULL;
	}
	~CLateBoundFind() {
		if(m_pDriver) {
			m_pDriver->DetachDispatch();
			delete m_pDriver;
		}
	}
private:
	IDispatchPtr m_pObject;
	COleDispatchDriver *m_pDriver;

};

CString CMergeEngine::GetBitmapMergeName(HBITMAP hBmp)
{
	// Load the image into memory
	CxImage image;
	image.CreateFromHBITMAP(hBmp);
	if (!image.IsValid()) {
		// Couldn't load the image for some reason, maybe bad hBmp?
		ASSERT(FALSE);
		return _T("");
	}

	// Create a temp file to store the image in
	CString strFilename;
	HANDLE hTempFile;
	{
		// Starting with the temp path and a base filename
		CString strBase;
		strBase.Format("%s\\TempImage%d", GetNxTempPath(), time(NULL));
		// Loop until we find a filename that doesn't exist
		for (long i=0; i<1000; i++) {
			// Try to create the file
			strFilename.Format("%s_%03li.bmp", strBase, i);
			hTempFile = CreateFile(strFilename, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			// If the file already exists, keep looping; otherwise we either created it or we failed, in either case we stop the loop
			if (hTempFile == INVALID_HANDLE_VALUE && GetLastError() == ERROR_FILE_EXISTS) {
				// Already exists, keep looping
			} else {
				// Either complete success, or complete failure! Either way, we're done looping so break out.
				break;
			}
		}
		// If we weren't successful (whether because there was some error, or because all files 
		// from 000 to 999 already existed), just fail out of the function right now.
		if (hTempFile == INVALID_HANDLE_VALUE) {
			// Failed
			ASSERT(FALSE);
			return _T("");
		}
	}

	// Save the image in the temp file
	if (!image.Save(strFilename, CXIMAGE_FORMAT_BMP)) {
		// Couldn't save for some reason, maybe out of hd space or something crazy like that.
		ASSERT(FALSE);
		// Destroy the temp file (only after closing our file handle)
		CloseHandle(hTempFile);
		hTempFile = NULL;
		DeleteFile(strFilename);
		// And fail
		return _T("");
	}

	// We're done with the our file handle
	CloseHandle(hTempFile);
	hTempFile = NULL;

	// Return success
	return GetBitmapMergeName(strFilename);
}

CString CMergeEngine::GetBitmapMergeName(CString strExistingFilePathName)
{
	CString strMergeName = "{NXIMAGE " + strExistingFilePathName + "}";

	// Add to our temp file list -- this is automatically deleted in
	// the destructor.
	m_astrTempFiles.Add(strExistingFilePathName);

	return strMergeName;
}

// (z.manning, 03/05/2008) - PLID 29131 - Added a function to load sender data so that we don't have
// to duplicate the same code 50 times anymore.
BOOL CMergeEngine::LoadSenderInfo(BOOL bPromptIfPreferenceSet)
{
	if(bPromptIfPreferenceSet && GetRemotePropertyInt("PromptHistoryMerge", 0, 0, GetCurrentUserName(), true))
	{
		//DRT 7/31/03 - A nice dialog that lets you choose what you want for the sender, 
		//the dialog that has a list of users (auto-select the current one), and the same 
		//		available list from LW for the subject (or the ability to type one in).  Make sure 
		//		it has all the fields from above
		CSelectSenderDlg dlg(NULL);
		if(dlg.DoModal() != IDOK) {
			return FALSE;
		}

		//Fill in the merge info with the sender data
		m_strSubjectMatter = dlg.m_strSubjectMatter;
		m_strSender = dlg.m_strFirst + (dlg.m_strMiddle.IsEmpty() ? "" : (" "+ dlg.m_strMiddle)) + " " + dlg.m_strLast;
		m_strSenderFirst = dlg.m_strFirst;
		m_strSenderMiddle = dlg.m_strMiddle;
		m_strSenderLast = dlg.m_strLast;
		m_strSenderEmail = dlg.m_strEmail;
		m_strSenderTitle = dlg.m_strTitle;
	}
	else
	{
		CString strName;

		//User does not have the preference to prompt for sender info, use the currently logged in user
		_RecordsetPtr rs = CreateParamRecordset("SELECT First, Middle, Last, Title, Email FROM PersonT WHERE ID = {INT}", GetCurrentUserID());
		if(!rs->eof) {
			m_strSenderFirst = AdoFldString(rs, "First","");
			m_strSenderMiddle = AdoFldString(rs, "Middle","");
			m_strSenderLast = AdoFldString(rs, "Last","");
			strName = m_strSenderFirst + (m_strSenderMiddle.IsEmpty() ? "" : (" "+ m_strSenderMiddle)) + " " + m_strSenderLast;
			strName.TrimRight();
			m_strSender = strName;
			m_strSenderTitle = AdoFldString(rs, "Title","");
			m_strSenderEmail = AdoFldString(rs, "Email","");
		}
		rs->Close();
	}

	return TRUE;
}