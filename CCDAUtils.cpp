
// (b.savon 2014-02-25 14:28) - PLID 61029 - Created - Restructure the Clinical Summary and Summary of Care code in EMR so that 
// it can be used by multiple areas in Practice.

#include "stdafx.h"
#include "CCDAUtils.h"
#include "NxAPI.h"
#include "NxAPIUtils.h"
#include "CCDInterface.h"
#include "MergeEngine.h"

namespace CCDAUtils
{
	// (j.gruber 2013-11-11 16:52) - PLID 59415 - created for
	// (d.singleton 2013-11-15 11:01) - PLID 59513 - need to insert the CCDAType when generating a CCDA
	// (a.walling 2014-05-13 14:43) - PLID 61788 - Moved from GlobalUtils - now takes in description as well
	// (r.gonet 05/07/2014) - PLID 61805 - Added PICID
	long AttachToHistory(Nx::SafeArray <BYTE> fileBytes, const CString& strDesc, const CString& strExtension, const CString& strExtraDesc, const CString& strSelection, CCDAType ctType, long nPICID, long nEMNID, long nPersonID, bool bGetCCDDescriptionInfo)
	{
		try {
			if (nPersonID == -1) {
				ThrowNxException("Invalid patient");
			}

			CString strFilePath = GetPatientDocumentPath(nPersonID);
			CString strOutput((const char*)fileBytes.begin(), fileBytes.GetLength());
			CString strFileName = GetPatientDocumentName(nPersonID, strExtension);
			if (strFileName.IsEmpty()) {
				ThrowNxException("Could not get filename to save to history");
			}

			CFile OutFile(strFilePath ^ strFileName, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite);
			OutFile.Write(fileBytes.cbegin(), fileBytes.GetLength());
			OutFile.Close();

			CString strTitle, strDisplay;
			CCD::CTimeRange tr;
			CString strDefault = strDesc;
			CString strDescription;
			if (bGetCCDDescriptionInfo) {
				CCD::GetDescriptionInfo(strFilePath, strDefault, strTitle, strDisplay, strDescription, tr);
			}
			else {
				// emulate the side effect of CCD::GetDescriptionInfo failing to load the file
				strDescription = "Clinical Document: ";
				strDescription += strDefault;
			}

			CString strFinalDesc = strDescription + (strExtraDesc.IsEmpty() ? "" : " " + strExtraDesc);

			// (d.singleton 2013-11-15 11:01) - PLID 59513 - need to insert the CCDAType when generating a CCDA
			COleDateTime dtNull;
			dtNull.SetStatus(COleDateTime::null);
			// (r.gonet 05/07/2014) - PLID 61805 - Pass along the PICID.	
			long nID = CreateNewMailSentEntry(nPersonID, strFinalDesc, strSelection, strFileName, GetCurrentUserName(), "", GetCurrentLocationID(), dtNull, -1, -1, nPICID, -1, FALSE, -1, "", ctType);
			if (nID != -1 && nEMNID != -1) {
				ExecuteParamSql("UPDATE MailSent SET EMNID = {INT} WHERE MailID = {INT}", nEMNID, nID);
			}
			return nID;
		} NxCatchAllThrow(__FUNCTION__);
		return -1;
	}

	//	Caller is responsible for catching any exceptions
	void GenerateSummaryOfCare(long nPatientPersonID, long nEMNID /*= -1*/,  HWND hParentWnd /*= NULL*/)
	{
		CWaitCursor cur;

		if(!CheckCurrentUserPermissions(bioPatientHistory, sptWrite)){
			return;
		}

		long nUserDefinedID = GetExistingPatientUserDefinedID(nPatientPersonID);

		//create our filter
		NexTech_Accessor::_CCDAPatientFilterPtr patFilter(__uuidof(NexTech_Accessor::CCDAPatientFilter)); 
		patFilter->patientID = (LPCTSTR)AsString(nUserDefinedID);			
		
		Nx::SafeArray<BSTR> aryEMNIDs;							

		//for now, we are going to use every emn for this person, in the future maybe we'll let them pick
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT EMRMasterT.ID, getDate() as CurrentDate "
			" FROM EMRMasterT "
			" WHERE DELETED = 0 AND PatientID = {INT}", nPatientPersonID);
		while (!prs->eof)
		{
			long nEMNID = AdoFldLong(prs->Fields, "ID");
			aryEMNIDs.Add(_bstr_t(AsString(nEMNID)));

			dtNow = AdoFldDateTime(prs->Fields, "CurrentDate");

			prs->MoveNext();
		}

		patFilter->EmnIDs = aryEMNIDs;

		//get out outputOptions
		NexTech_Accessor::_CCDAOutputOptionsPtr outputOptions(__uuidof(NexTech_Accessor::CCDAOutputOptions));
		outputOptions->IncludeXMLFile = true;
		outputOptions->IncludePDFFile = false;

		NexTech_Accessor::_CCDAGenerationResultPtr ccdaResult = NULL;
		
		// (j.jones 2014-01-10 16:20) - PLID 60274 - GenerateSummaryofCare can return an API soap fault
		// when a message box (such as when the CCDA is not set up) is desired instead of an exception.
		// We need to look for the special error, and handle it. If it is not handled, then throw it
		// as a normal exception.
		try {
			ccdaResult = GetAPI()->GenerateSummaryofCare(GetAPISubkey(), GetAPILoginToken(), patFilter, outputOptions);
		}catch (_com_error &e) {
			//this function will turn expected soap exceptions into messageboxes,
			//return TRUE if it did, FALSE if the error still needs handled
			if(!ProcessAPIComError_AsMessageBox(hParentWnd, "Practice", e)) {
				throw e;
			}

			return;
		}

		// (r.gonet 06/16/2014) - PLID 61805 - The summary of care should not be attached to a PIC. Pass -1 for it.
		long nMailID = AttachToHistory(ccdaResult->XMLFile, "Generated Summary of Care", "xml", "", SELECTION_CCDA, ctSummaryOfCare, -1, nEMNID, nPatientPersonID, true);

		// (a.walling 2015-10-28 13:16) - PLID 67424 - Attempt to copy the CCDA to export paths if necessary
		GetAPI()->CopyToCCDAExportPaths(GetAPISubkey(), GetAPILoginToken(), (const char*)FormatString("%li", nMailID));

		MessageBox(hParentWnd, "Summary of Care successfully exported to patient history.", "NexTech Practice", MB_OK | MB_ICONINFORMATION);
	}

	//	Caller is responsible for catching any exceptions
	// (b.savon 2014-04-30 09:31) - PLID 61791 - User Story 10 - Auxiliary CCDA Items - Requirement 100150 - Added xml and pdf params
	// (r.gonet 05/07/2014) - PLID 61805 - Added parameter nPICID, which causes the clinical summary to attach to history
	// with an associated PIC
	void GenerateClinicalSummary(long nPatientPersonID, long nPICID, long nEMNID, HWND hParentWnd /*= NULL*/, bool bGenerateXML /* = true */, bool bGeneratePDF /* = false */)
	{
		CWaitCursor cur;

		if(!CheckCurrentUserPermissions(bioPatientHistory, sptWrite)){
			return;
		}

		//get out outputOptions
		// (b.savon 2014-04-30 09:31) - PLID 61791 - User Story 10 - Auxiliary CCDA Items - Requirement 100150 - Added xml and pdf params
		NexTech_Accessor::_CCDAOutputOptionsPtr outputOptions(__uuidof(NexTech_Accessor::CCDAOutputOptions));
		outputOptions->IncludeXMLFile = bGenerateXML;
		outputOptions->IncludePDFFile = bGeneratePDF;

		CString strUserDefinedID = AsString(GetExistingPatientUserDefinedID(nPatientPersonID));	

		NexTech_Accessor::_CCDAGenerationResultPtr ccdaResult = NULL;

		// (j.jones 2014-01-10 16:20) - PLID 60274 - GenerateClinicalSummary can return an API soap fault
		// when a message box (such as when the CCDA is not set up) is desired instead of an exception.
		// We need to look for the special error, and handle it. If it is not handled, then throw it
		// as a normal exception.
		try {
			ccdaResult = GetAPI()->GenerateClinicalSummary(GetAPISubkey(), GetAPILoginToken(), _bstr_t(AsString(nEMNID)), _bstr_t(strUserDefinedID), outputOptions);
		}catch (_com_error &e) {
			//this function will turn expected soap exceptions into messageboxes,
			//return TRUE if it did, FALSE if the error still needs handled
			if(!ProcessAPIComError_AsMessageBox(hParentWnd, "Practice", e)) {
				throw e;
			}

			return;
		}

		// to be consistent with older behavior, we do not try to get more information from the clinical summary output for the description.
		if (bGenerateXML) {
			// (d.singleton 2013-11-15 11:03) - PLID 59513 - need to insert the CCDAType when generating a CCDA
			// (r.gonet 05/07/2014) - PLID 61805 - Pass along the PICID so it gets put into MailSent.
			long nMailID = AttachToHistory(ccdaResult->XMLFile, "Generated Clinical Summary", "xml", "", SELECTION_CCDA, ctClinicalSummary, nPICID, nEMNID, nPatientPersonID, false);

			// (a.walling 2015-10-28 13:16) - PLID 67424 - Attempt to copy the CCDA to export paths if necessary
			GetAPI()->CopyToCCDAExportPaths(GetAPISubkey(), GetAPILoginToken(), (const char*)FormatString("%li", nMailID));
		}

		if (bGeneratePDF){
			// (r.gonet 05/07/2014) - PLID 61805 - Pass along the PICID so it gets put into MailSent.
			AttachToHistory(ccdaResult->PDFFile, "Generated Clinical Summary", "pdf", "- Human Readable", SELECTION_FILE, ctNone, nPICID, nEMNID, nPatientPersonID, false);
		}

		// (r.gonet 04/28/2014) - PLID 61807 - Get those EMR tabs refreshed so that the clinical summary icon shows up.
		CClient::RefreshTable(NetUtils::EMRMasterT, nPatientPersonID);
		//success!
		MessageBox(hParentWnd, "Clinical Summary successfully exported to patient history.", "NexTech Practice", MB_OK|MB_ICONINFORMATION);
	}
}