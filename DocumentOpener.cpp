// DocumentOpener.cpp : implementation file
//
// (c.haag 2015-05-04) - NX-100442 - This class contains implementations for opening various kinds
// of supported documents.
//

#include "stdafx.h"
#include "DocumentOpener.h"
#include "CaseHistoryDlg.h"
#include "NxXMLUtils.h"
#include "GenericXMLBrowserDlg.h"
#include "CCDAViewerDlg.h"
#include "CCDUtils.h"
#include "CCDInterface.h"
#include "GlobalUtils.h"

using namespace ADODB;

// Attempts to open a reproductive form and returns to the caller whether the object was opened and whether 
// any data changed in the process. The caller is responsible for determining that the path points to a valid object.
/* static */ CDocumentOpener::Result CDocumentOpener::OpenReproductiveForm(const CString& strPath)
{
	// (d.thompson 2014-02-20) - PLID 60924 - Removed IVF forms.  Just in case anyone has data that would hit this line, throw an exception.
	AfxThrowNxException("IVF functionality is not available.  Please contact NexTech Technical Support");
	return CDocumentOpener::Result(FALSE, FALSE);
}

// Attempts to open a case history and returns to the caller whether the object was opened and whether 
// any data changed in the process. The caller is responsible for determining that the path points to a valid object.
/* static */ CDocumentOpener::Result CDocumentOpener::OpenCaseHistory(const CString& strPath, long nPersonID, long nMailID, CWnd* pParentWnd, IN OUT BOOL& bCaseHistoryIsOpen)
{
	if (IsSurgeryCenter(true)) 
	{
		//if they are on the case history tab, don't let them leave if they have open case histories
		if (theApp.m_arypCaseHistories.GetSize() > 0) {
			long nSize = theApp.m_arypCaseHistories.GetSize();
			for (long i = 0; i<nSize; i++) {
				CWnd *pWnd = (CWnd *)theApp.m_arypCaseHistories[i];
				if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible()) {
					AfxMessageBox("Please close all open Case Histories before continuing.", MB_OK | MB_ICONSTOP);
					return CDocumentOpener::Result(FALSE, FALSE);
				}
			}
		}

		// Get the case history id
		long nCaseHistoryID = 0;
		{
			_RecordsetPtr prs = CreateRecordset("SELECT InternalRefID FROM MailSent WHERE MailID = %li", nMailID);
			nCaseHistoryID = AdoFldLong(prs, "InternalRefID");
		}

		//make sure it still exists
		if (IsRecordsetEmpty("SELECT ID FROM CaseHistoryT WHERE ID = %li", nCaseHistoryID)) {
			MsgBox(MB_ICONINFORMATION | MB_OK, "This Case History cannot be found - it is possible it has been deleted by another user.\n"
				"Practice will now delete this record.");

			// (j.jones 2008-09-04 17:45) - PLID 30288 - supported MailSentNotesT
			CString strSqlBatch = BeginSqlBatch();
			// (c.haag 2009-10-12 12:19) - PLID 35722 - Because MailSent has so many dependencies, we have a function to build the SQL to delete from it now.
			AddDeleteMailSentQueryToSqlBatch(strSqlBatch, FormatString("%d", nMailID));
			ExecuteSqlBatch(strSqlBatch);

			// (c.haag 2010-06-10 17:19) - PLID 39057 - Send table checkers. Because todo alarms can now link with MailSent records,
			// we have to send those as well.
			// (j.jones 2014-08-04 13:31) - PLID 63159 - this now sends an Ex tablechecker, we know IsPhoto is always false here
			CClient::RefreshMailSentTable(nPersonID, nMailID);
			if (CanCreateTodoForDocument(strPath)) {
				CClient::RefreshTable(NetUtils::TodoList, -1);
			}

			return CDocumentOpener::Result(FALSE, TRUE);
		}

		// Open the case history

		// (j.jones 2007-02-12 13:04) - PLID 24709 - first ensure a case is not already open

		if (!bCaseHistoryIsOpen) 
		{
			bCaseHistoryIsOpen = TRUE;
			CCaseHistoryDlg dlg(pParentWnd);
			int nResult = dlg.OpenExistingCase(nCaseHistoryID);
			// (j.jones 2009-08-07 08:34) - PLID 7397 - the case can now return more than just ok/cancel
			if (nResult != IDCANCEL) {
				//DRT 11/11/2005 - PLID 18322 - We don't need to do this, because the act of changing the 
				//	case history dialog fires a table checker message which requeries the entire list.
				//m_pDocuments->PutValue(nRow, phtdlcNotes, _bstr_t((LPCTSTR)dlg.m_strName));
			}
			bCaseHistoryIsOpen = FALSE;
			return CDocumentOpener::Result(TRUE, FALSE);
		}
		else {
			//a case history is already open, do not open another one
			ASSERT(FALSE);
			return CDocumentOpener::Result(FALSE, FALSE);
		}
	}
	else {
		// They don't have a surgery center license, so they can't do anything with surgery center items in the patient history
		MsgBox(MB_ICONINFORMATION | MB_OK,
			"You may not open this case history because you are not licensed for use of the Surgery Center Components.\n\n"
			"If you would like to purchase a license to use these features or you feel this message is in error, please contact NexTech at 1-888-417-8464");
		return CDocumentOpener::Result(FALSE, FALSE);
	}
}

// Attempts to open an eligibility request and returns to the caller whether the object was opened and whether 
// any data changed in the process. The caller is responsible for determining that the path points to a valid object.
/* static */ CDocumentOpener::Result CDocumentOpener::OpenEligibilityRequest(const CString& strPath, long nPersonID, long nMailID)
{
	//we need to know the EligibilityRequestsT.ID

	long nRequestID = -1;
	long nResponseID = -1;
	if (strPath.CompareNoCase(PATHNAME_OBJECT_ELIGIBILITY_REQUEST) == 0) {
		//if this is a request, then the InternalRefID is the ID we want, but validate it inside the recordset call
		_RecordsetPtr prs = CreateRecordset("SELECT ID FROM EligibilityRequestsT WHERE ID IN ("
			"SELECT InternalRefID FROM MailSent WHERE MailID = %li)", nMailID);
		if (!prs->eof) {
			nRequestID = AdoFldLong(prs, "ID");
		}
		prs->Close();
	}
	else if (strPath.CompareNoCase(PATHNAME_OBJECT_ELIGIBILITY_RESPONSE) == 0) {
		//if this is a response, then the InternalRefID is the ID of a response,
		//but we need the request ID
		_RecordsetPtr prs = CreateRecordset("SELECT ID, RequestID FROM EligibilityResponsesT WHERE ID IN ("
			"SELECT InternalRefID FROM MailSent WHERE MailID = %li)", nMailID);
		if (!prs->eof) {
			nRequestID = AdoFldLong(prs, "RequestID");
			nResponseID = AdoFldLong(prs, "ID");
		}
		prs->Close();
	}

	//warn if we couldn't find one
	if (nRequestID == -1) {
		MsgBox(MB_ICONINFORMATION | MB_OK, "The Eligibility record cannot be found - it is possible it has been deleted by another user.\n"
			"Practice will now delete this history record.");

		// (j.jones 2008-09-04 17:45) - PLID 30288 - supported MailSentNotesT
		CString strSqlBatch = BeginSqlBatch();
		// (c.haag 2009-10-12 12:19) - PLID 35722 - Because MailSent has so many dependencies, we have a function to build the SQL to delete from it now.
		AddDeleteMailSentQueryToSqlBatch(strSqlBatch, FormatString("%d", nMailID));
		ExecuteSqlBatch(strSqlBatch);

		// (c.haag 2010-06-10 17:19) - PLID 39057 - Send table checkers. Because todo alarms can now link with MailSent records,
		// we have to send those as well.
		// (j.jones 2014-08-04 13:31) - PLID 63159 - this now sends an Ex tablechecker, we know IsPhoto is always false here
		CClient::RefreshMailSentTable(nPersonID, nMailID);
		if (CanCreateTodoForDocument(strPath)) {
			CClient::RefreshTable(NetUtils::TodoList, -1);
		}

		return CDocumentOpener::Result(FALSE, TRUE);
	}

	//Open the detailed review

	// (j.jones 2007-06-20 10:48) - PLID 26387 - added ability to review individual requests/responses
	// (j.jones 2010-07-07 09:52) - PLID 39534 - this dialog now takes in an array of request IDs,
	// though right now we are only viewing one request
	// (r.goldschmidt 2014-10-10 12:51) - PLID 62644 - convert eligibility request detail dialog to modeless
	std::vector<long> aryRequestIDsUpdated, aryResponseIDsReturned;
	aryRequestIDsUpdated.push_back(nRequestID);
	//if we have a response ID, use it
	if (nResponseID != -1) {
		aryResponseIDsReturned.push_back(nResponseID);
	}
	GetMainFrame()->ShowEligibilityRequestDetailDlg(aryRequestIDsUpdated, aryResponseIDsReturned);

	//that's it, don't need to do anything to the History tab because
	//it reflects information upon send/receive, not the current data

	return CDocumentOpener::Result(TRUE, FALSE);
}

// Attempts to open an XML document and returns to the caller whether the object was opened and whether 
// any data changed in the process. The caller is responsible for determining that the path points to a valid object.
/* static */ CDocumentOpener::Result CDocumentOpener::OpenXMLDocument(CString strPath, long nMailID, CWnd* pParentWnd)
{
	return OpenXMLDocument(NxXMLUtils::LoadXMLDocument(strPath), nMailID, pParentWnd);
}

// Attempts to open an XML document and returns to the caller whether the object was opened and whether 
// any data changed in the process. The caller is responsible for determining that the document is valid.
/* static */ CDocumentOpener::Result CDocumentOpener::OpenXMLDocument(MSXML2::IXMLDOMDocument2Ptr pDocument, long nMailID, CWnd* pParentWnd)
{
	// sanity check		
	// (a.walling 2010-01-06 12:06) - PLID 36809 - Handle CCR documents
	// (j.jones 2010-06-30 10:57) - PLID 38031 - GetXMLDocumentType is now
	// part of NxXMLUtils, not part of the CCD namespace
	NxXMLUtils::EDocumentType documentType = NxXMLUtils::GetXMLDocumentType(pDocument);
	CGenericXMLBrowserDlg* pBrowser = new CGenericXMLBrowserDlg(pParentWnd);
	CDocumentOpener::Result result = CDocumentOpener::Result(FALSE, FALSE);

	if (pBrowser) 
	{
		if (documentType == NxXMLUtils::Generic_XML_Document) {
			pBrowser->m_bAllowToggle = FALSE;
		}

		pBrowser->Create(IDD_GENERIC_BROWSER_DLG, pParentWnd);

		// (b.spivey September 25, 2013) - PLID 57964 - new document type. 
		//TES 12/4/2013 - PLID 57415 - Treat Cancer Case submissions just as CCDAs, at least for the time being
		//TES 7/10/2015 - PLID 62273 - Actually, it makes more sense, and looks better, to treat them as CCDs
		if (documentType == NxXMLUtils::CCDA_Document /*|| documentType == NxXMLUtils::CancerCase_Document*/) {
			//CCDA document						
			// (j.gruber 2013-11-20 15:47) - PLID 59658 - add a maximize box
			// (d.singleton 2014-05-30 13:40) - PLID 61927 - In the CCDA Viewer, add “Save as PDF” to the right click menu.
			CCCDAViewerDlg *pBrowser2 = new CCCDAViewerDlg(pParentWnd, nMailID);
			pBrowser2->Create(IDD_GENERIC_BROWSER_DLG, pParentWnd);
			pBrowser2->ModifyStyle(0, WS_MAXIMIZEBOX, SWP_FRAMECHANGED);
			pBrowser2->NavigateToXMLDocument(pDocument, CCD::GetCCDAXSLResourcePath());
			pBrowser2->ShowWindow(SW_SHOW);
		}
		else {
			if (documentType == NxXMLUtils::CCD_Document || documentType == NxXMLUtils::CancerCase_Document) {
				// CCD document
				// (j.gruber 2013-11-20 15:47) - PLID 59658 - add a maximize box			
				pBrowser->ModifyStyle(0, WS_MAXIMIZEBOX, SWP_FRAMECHANGED);
				pBrowser->NavigateToXMLDocument(pDocument, CCD::GetCCDXSLResourcePath());
			}
			else if (documentType == NxXMLUtils::CCR_Document) {
				// CCR document
				// (j.gruber 2013-11-20 15:47) - PLID 59658 - add a maximize box
				pBrowser->ModifyStyle(0, WS_MAXIMIZEBOX, SWP_FRAMECHANGED);
				pBrowser->NavigateToXMLDocument(pDocument, CCD::GetCCRXSLResourcePath());
			}
			else if (documentType == NxXMLUtils::PQRS_Document) {
				//PQRS document
				pBrowser->NavigateToXMLDocument(pDocument, GetPQRSXSLResourcePath());
			}
			// (j.jones 2013-10-11 10:04) - PLID 58806 - added QRDA XSL
			else if (documentType == NxXMLUtils::QRDA_Document) {
				pBrowser->NavigateToXMLDocument(pDocument, GetQRDAXSLResourcePath());
			}
			else if (documentType == NxXMLUtils::Generic_XML_Document) {
				// xml only
				// (j.jones 2010-06-30 12:00) - PLID 38031 - GetGenericXMLXSLResourcePath is no longer
				// in the CCD namespace
				pBrowser->NavigateToXMLDocument(pDocument, GetGenericXMLXSLResourcePath());
			}
			
			pBrowser->ShowWindow(SW_SHOW);
		}
	}
	return result;
}

// Attempts to open a collection of Word documents and returns to the caller whether all documents were opened and 
// whether any data changed in the process. The caller is responsible for determining that the path points to a valid object.
/* static */  CDocumentOpener::Result CDocumentOpener::OpenPacket(long nMergedPacketID, long nPersonID)
{
	_RecordsetPtr rsMailSent = CreateRecordset("SELECT MailID, Selection, PathName, PersonID FROM MailSent WHERE MergedPacketID = %li AND PersonID = %li ORDER BY MailID ASC", nMergedPacketID, nPersonID);
	if (rsMailSent->eof) {
		MsgBox("Packet not found.");
		return CDocumentOpener::Result(FALSE, FALSE);
	}
	else
	{
		BOOL bAllAttachmentsOpened = TRUE;
		while (!rsMailSent->eof)
		{
			CString strTemplate = AdoFldString(rsMailSent, "PathName");
			if (!OpenDocument(strTemplate, nPersonID).bDocumentOpened) {
				MsgBox("Could not open template %s", strTemplate);
				bAllAttachmentsOpened = FALSE;
			}
			rsMailSent->MoveNext();
		}
		return CDocumentOpener::Result(bAllAttachmentsOpened, FALSE);
	}
}

// Attempts to open an attachment from a person history and returns to the caller whether all documents were opened and 
// whether any data changed in the process. This code behaves identically to the code in CHistoryDlg, and may raise modal
// dialogs depending on the situation. This does not support Letter Writing packets.
/* static */ CDocumentOpener::Result CDocumentOpener::OpenHistoryAttachment(CString strPath, long nPersonID, long nMailID, CWnd* pParentWnd, IN OUT BOOL& bCaseHistoryIsOpen, const CString& strSelectionName)
{
	if (IsReproductive()) {
		if (strPath.CompareNoCase(PATHNAME_FORM_REPRODUCTIVE) == 0) {
			//this means it is not an attached document, but an internal form
			return OpenReproductiveForm(strPath);
		}
	}

	// See if this entry is a case history object, open it as such
	if (strPath.CompareNoCase(PATHNAME_OBJECT_CASEHISTORY) == 0) {
		// (c.haag 2015-05-04) - NX-100442 - Moved the business logic of opening the object into DocumentOpener
		return OpenCaseHistory(strPath, nPersonID, nMailID, pParentWnd, bCaseHistoryIsOpen);
	}


	// (j.jones 2007-06-27 17:20) - PLID 26480 - supported Eligibility
	// See if this entry is a eligibility object, open it as such
	if (strPath.CompareNoCase(PATHNAME_OBJECT_ELIGIBILITY_REQUEST) == 0 || strPath.CompareNoCase(PATHNAME_OBJECT_ELIGIBILITY_RESPONSE) == 0) 
	{
		// (c.haag 2015-05-04) - NX-100442 - Moved the business logic of opening the object into DocumentOpener
		return OpenEligibilityRequest(strPath, nPersonID, nMailID);
	}

	// (c.haag 2016-05-04 8:04) - NX-100442 - From here on we will treat the object we're opening as a Word document, generic
	// file, folder, audio clip or XML file
	Result openResult(FALSE, FALSE);

	//DRT 6/13/03 - This is a problem that came about due to an issue with Dr. Clark's office (PLID 8183).
	//		What happens, is the document path is generated by the merge engine to pull the previous merge of that document
	//		and increment the number (ex:  MultiPatDoc00000.doc becomes MultiPatDoc00001.doc).  But somehow, they got one in
	//		there without a .doc on it.  So every time they make a new document, it's pulling the last as MultiPatDoc00000
	//		and making the new one MultiPatDoc00001 every time.  Which means they can't open the file, because that's not its 
	//		name.  
	if (strnicmp(strSelectionName, SELECTION_WORDDOC, 13) == 0)
	{
		//DRT 6/13/03 - This is a problem that came about due to an issue with Dr. Clark's office (PLID 8183).
		//		What happens, is the document path is generated by the merge engine to pull the previous merge of that document
		//		and increment the number (ex:  MultiPatDoc00000.doc becomes MultiPatDoc00001.doc).  But somehow, they got one in
		//		there without a .doc on it.  So every time they make a new document, it's pulling the last as MultiPatDoc00000
		//		and making the new one MultiPatDoc00001 every time.  Which means they can't open the file, because that's not its 
		//		name.  

		//looking at a word doc, see if filename ends in .doc
		// (a.walling 2007-07-19 11:18) - PLID 26748 - Check for .docx too
		// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
		// (c.haag 2016-05-03 14:09) - NX-100442 - This code was previously using the wrong logic to detect Word documents. Oh dear.
		if ((strnicmp(strPath.Right(4), ".doc", 4) != 0) && (strnicmp(strPath.Right(5), ".docx", 5) != 0) && (strnicmp(strPath.Right(5), ".docm", 5) != 0)) {
			//it does NOT end in .doc!  Oh dear.
			//for 1 last check, make sure there is NO extension on it.
			if (GetFileExtension(strPath).IsEmpty()) {
				//there is no extension
				CString strMsg;
				strMsg.Format("Practice has detected your Word document does not end in the standard .doc or .docx/.docm extension.  Would you "
					"like to fix this in your data?");
				// (a.walling 2009-02-24 17:24) - PLID 33229 - Fix bad MsgBox calls
				if (IDYES == AfxMessageBox(strMsg, MB_YESNO)) {
					//they want to fix it manually.  We know that we have a filename w/o an extension, like
					//"MultiPatDoc00000", and we want to add .doc to that.
					try {
						// (a.walling 2007-07-19 10:50) - PLID 26748 - Need to find out whether we should rename to .doc or .docx
						CString strFullPath, strExtension;
						if (strPath.Find('\\') == -1) {
							// The "path" doesn't have a backslash, so it's just a filename, which means it should use patient's shared documents path
							// (a.walling 2011-02-11 15:13) - PLID 42441 - Always use GetCurPatientDocumentPath instead of GetPatientDocumentPath
							strFullPath = GetPatientDocumentPath(nPersonID) ^ strPath;
						}
						else {
							//use the path that was provided
							strFullPath = strPath;
						}
						CFile fDoc;
						if (fDoc.Open(strFullPath, CFile::modeRead | CFile::shareCompat)) {
							char magic[2];

							DWORD dwRead = fDoc.Read(magic, 2);
							fDoc.Close();
							if (dwRead < 2) {
								ThrowNxException("File %s has an invalid length", strPath);
							}
							else {
								if (memcmp(magic, "PK", 2) == 0) {
									// (a.walling 2007-10-12 14:29) - PLID 26342 - There is no easy way to tell if this has macros or not
									// so just rename to docx by default
									strExtension = ".docx";
								}
								else {
									strExtension = ".doc";
								}
							}
						}
						else {
							// if we can't open it, not much we can do, so let's just exit since we can't open the document anyway.
							ThrowNxException("Could not access %s: %s", strPath, FormatLastError());
						}

						// (j.jones 2011-07-22 15:41) - PLID 21784 - get a list of the MailIDs affected (rarely more than one)
						CArray<long, long> aryMailIDs;
						CArray<long, long> aryPatientIDs;
						// (j.jones 2014-08-04 17:25) - PLID 63159 - need to load the patient IDs too
						ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT MailID, PersonID FROM MailSent WHERE PathName = {STRING}", strPath);
						while (!rs->eof) {
							aryMailIDs.Add(VarLong(rs->Fields->Item["MailID"]->Value));
							aryPatientIDs.Add(VarLong(rs->Fields->Item["PersonID"]->Value));
							rs->MoveNext();
						}
						rs->Close();

						ExecuteParamSql("UPDATE MailSent SET PathName = {STRING} WHERE MailID IN ({INTARRAY})", strPath + strExtension, aryMailIDs);

						// (j.jones 2011-07-22 15:43) - PLID 21784 - send a tablechecker for each file changed, usually it is only one
						ASSERT(aryPatientIDs.GetSize() == aryMailIDs.GetSize());
						for (int i = 0; i<aryMailIDs.GetSize(); i++) {
							// (j.jones 2014-08-04 16:58) - PLID 63159 - this now sends an Ex tablechecker
							// (c.haag 2015-05-04) - NX-100442 - This is a Word Document row so GetRowPhotoStatus is always false
							CClient::RefreshMailSentTable(aryPatientIDs.GetAt(i), aryMailIDs.GetAt(i), FALSE /*GetRowPhotoStatus(nRow)*/);
						}

						// (c.haag 2015-05-04) - NX-100442 - We changed data so we should refresh
						openResult.bNeedUpdateView = TRUE;

						strPath += strExtension;	//update it so we can open the file
					} NxCatchAll("Error updating MailSent with .doc(x) extension.");
				}
				else {
					//they don't want to fix it, oh well, we tried
				}
			}
			else {
				//there is some kind of extension here, so we'll just let this go
			}
		}
	}

	CString strFullPath;
	if (strPath.Find('\\') == -1) {
		// The "path" doesn't have a backslash, so it's just a filename, which means it should use patient's shared documents path
		// (a.walling 2011-02-11 15:13) - PLID 42441 - Always use GetCurPatientDocumentPath instead of GetPatientDocumentPath
		strFullPath = GetPatientDocumentPath(nPersonID) ^ strPath;
	}
	else {
		//use the path that was provided
		strFullPath = strPath;
	}

	// (a.walling 2009-05-05 17:15) - PLID 34176 - Special case for CCD documents
	// (j.jones 2010-06-30 11:12) - PLID 38031 - changed to also support generic XML
	// (j.gruber 2013-11-08 11:15) - PLID 59375  - not needed here, since included in IsXMLFileExtension
	BOOL bIsXMLFileType = strnicmp(strSelectionName, SELECTION_CCD, 10) == 0 || NxXMLUtils::IsXMLFileExtension(strFullPath);
	if (bIsXMLFileType && NxXMLUtils::IsValidXMLFile(strFullPath))
	{
		// (c.haag 2015-05-04) - NX-100442 - Moved the business logic of opening the object into DocumentOpener 
		openResult = OpenXMLDocument(strFullPath, nMailID, pParentWnd);
	}
	else if (strnicmp(strSelectionName, SELECTION_WORDDOC, 13) == 0 || strnicmp(strSelectionName, SELECTION_FILE, 11) == 0 ||
		strnicmp(strSelectionName, SELECTION_FOLDER, 13) == 0 || strnicmp(strSelectionName, SELECTION_AUDIO, 12) == 0)
	{
		// (c.haag 2015-05-04) - NX-100442 - Moved the business logic of opening the object into DocumentOpener
		// (c.haag 2016-05-03 14:12) - NX-100442 - Prevent opening multi-patient documents based on preference
		if (strPath.Find("MultiPatDoc") == -1 || GetRemotePropertyInt("HistoryBlockMultiPatDocs", 1, 0, "<None>") == 0)
		{
			openResult = OpenDocument(strFullPath, nPersonID);
		}

		if (!openResult.bDocumentOpened)
		{
			MsgBox(RCS(IDS_NO_FILE_OPEN));
		}
	}
	else
	{
		// Unknown document type; both parameters in openResult should remain FALSE
	}

	return openResult;
}

// Attempts to open a document with the default Windows-based viewer. The path may be absolute or relative 
// to the patient documents folder and returns to the caller whether the object was opened and whether 
// any data changed in the process. The caller is responsible for determining that the path points to a valid object.
/* static */  CDocumentOpener::Result CDocumentOpener::OpenDocument(CString strPath, long nPersonID)
{
	//this is the conventional way of opening an attached file
	if (strPath.Find('\\') == -1) {
		// The "path" doesn't have a backslash, so it's just a filename, which means it should use patient's shared documents path
		strPath = GetPatientDocumentPath(nPersonID) ^ strPath;
	}

	BOOL bOpenDocumentResult = ::OpenDocument(strPath);
	return CDocumentOpener::Result(bOpenDocumentResult, FALSE);
}

// Determines whether a todo alarm can be created for a document at a specified path. The path may be a special
// built-in sentinel path.
/* static */ BOOL CDocumentOpener::CanCreateTodoForDocument(const CString& strPath)
{
	// Do not let users create tasks for Case Histories or Reproductive forms. Everything else is fair game,
	// including eligibility forms. We may tweak this later on.
	if (!strPath.CompareNoCase(PATHNAME_OBJECT_CASEHISTORY) ||
		!strPath.CompareNoCase(PATHNAME_FORM_REPRODUCTIVE)
		)
	{
		return FALSE;
	}
	return TRUE;
}
