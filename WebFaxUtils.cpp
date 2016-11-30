//
//WebFaxUtils.cpp
#include "stdafx.h"
#include "WebFaxUtils.h"
#include "Base64.h"
#include "SOAPUtils.h"
#include <Wininet.h>

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


////////////////////////////////////////////////////////////////
//Implementation of the CMyFaxSend class

//DRT 6/27/2008 - PLID 30524 - Implementation of sending a fax through MyFax service.
//(e.lally 2011-10-31) PLID 41195 - Added PersonID and PicID for the mailSent log entry
bool CMyFaxSend::SendFax(CFaxSettings settings, long nPersonID/* = -1*/, long nPicID/* = -1*/)
{
	CWaitCursor wc;
	//Actually generate the body of the document.  This is formatted according to the specs required by 
	//	the SendSingleFax web service.
	CString strXmlDocument;
	if(!GenerateSendSingleFaxDocument(settings, strXmlDocument)) {
		//Something failed in the XML generation, probably a bad parameter.
		return false;
	}

	//(e.lally 2011-10-31) PLID 41195
	CString strFaxDetails = " To: " + Trim(settings.m_strRecipName + " " + settings.m_strRecipNumber);
	if(!settings.m_strCoverPageSubject.IsEmpty()){
		strFaxDetails += ". Subject: " + Trim(settings.m_strCoverPageSubject);
	}



	//Soap 1.1 Messaging Setup for MyFax, for SendSingleFax functionality.  Documentation at:
	//	https://www.protusfax.com/protus/xmlwebservices/xmlsubmitter/messaging.asmx?op=SendSingleFax
	//
	//I loaded these as ConfigRT entries just in case the addresses change in the future.
	CString strURL = GetRemotePropertyText("MyFaxURL", "https://www.protusfax.com/protus/xmlwebservices/xmlsubmitter/messaging.asmx", 
		0, NULL, true);
	CString strURI = GetRemotePropertyText("MyFaxURI", "https://www.protus.com/WebServices/Messaging/2005/2", 
		0, NULL, true);

	// (j.gruber 2009-03-02 16:27) - PLID 33273 - changed from MSXML to MSXML2
	// (b.cardillo 2014-06-04 22:38) - PLID 61764 - Use the myfax send timeout preference
	// (b.cardillo 2014-06-09 16:54) - PLID 61764 - Give friendly prompt if the problem is a timeout since we have a preference to control that
	MSXML2::IXMLDOMNodePtr pnodeResult;
	DWORD before = GetTickCount();
	try {
		pnodeResult = ForwardFunctionAsSOAP_RAW(strURL, strURI, GetRemotePropertyInt("MyFaxSoapCallTimeoutSeconds", 30) * 1000, "SendSingleFax", 1, "XmlDocument", strXmlDocument);
	} catch (_com_error &e) {
		DWORD after = GetTickCount();
		if (e.Error() == HRESULT_FROM_WIN32(ERROR_INTERNET_TIMEOUT)) {
			AfxMessageBox(_T("The fax server did not respond to the send request within the timeout period. This may be due to a temporary network condition. If this problem persists, ask your office manager to increase the 'MyFax Send Fax Timeout' preference.\r\n\r\nPlease check your MyFax account to see if the fax was transmitted. No record of this transaction was logged in NexTech Practice."));
			return false;
		} else {
			throw;
		}
	}
	if(pnodeResult == NULL) {
		AfxMessageBox("No response was received from the fax server.  Please check your MyFax account to see if the fax was transmitted.\r\n"
			"No record of this transaction was logged in NexTech Practice.");
		return false;
	}
	else {
		//We got a result message back

		//First, see if there was an error
		CString strError = GetOptionalSubNodeText(pnodeResult, "ErrorFlag", "", "");
		//And get the text (same parameter either way)
		CString strRetMsg = GetOptionalSubNodeText(pnodeResult, "ReturnMessage", "", "");
		if(strError.CompareNoCase("false") == 0) {
			//There is no error.  In this case, we need to retrieve the transaction ID and save it to data.
			CString strTransID = GetOptionalSubNodeText(pnodeResult, "TransactionID", "", "");
			if(strTransID == "") {
				//Error!  This should never happen if the fax sent successfully
				AfxMessageBox("A successful response was received from the fax server, but no transaction ID was contained.  Please check "
					"your MyFax account to ensure the document was faxed.\r\n"
					"No record of this transaction was logged in NexTech Practice.\r\n\r\nReturned Message:\r\n" + strRetMsg);
				return false; // failed
			}
			else {
				//Valid trans ID.  Save the whole action to data
				long nTransID = atoi(strTransID);
				//Last error check
				if(nTransID == 0) {
					//0 means atoi() got non-numeric text (or it was 0, which isn't possible in the MyFax spec)
					AfxMessageBox("A successful response was received from the fax server, but an invalid transaction ID was contained.  "
						"Please check your MyFax account to ensure the document was faxed.\r\n"
						"No record of this transaction was logged in NexTech Practice.\r\n\r\nReturned Message:\r\n" + strRetMsg);
					return false; // failed
				}
				else {
					//Finally, a valid response.  Save a record of this transaction so the user can look it up later, and it
					//	gives us a history of faxed documents.
					
					_variant_t varMailID = g_cvarNull;
					if(GetRemotePropertyInt("SaveSentFaxInHistory", 1, 0, NULL, true) != 0){
						if(nPersonID > 0){
							//(e.lally 2011-10-31) PLID 41195 - Create an entry in the mailSent history
							// (d.singleton 2013-11-15 11:23) - PLID 59513 - need to insert the CCDAType when generating a CCDA
							CString strNote = "Fax sent successfully." + strFaxDetails;
							varMailID = (long)CreateNewMailSentEntry(nPersonID, strNote, "", "", GetCurrentUserName(), "", GetCurrentLocationID(),
								g_cdtNull, -1, -1, nPicID, -1, FALSE, -1, "", ctNone);
						}
					}

					//We'll need to insert all the documents in the batch, and the resultant TransactionID can be
					//	used as a sort of "batch id" to let us know these all go together.
					//(e.lally 2011-10-31) PLID 41195 - Reference the mailsent entry, if we made one
					CNxParamSqlArray params;
					CString strSql = BeginSqlBatch();
					for(int i = 0; i < settings.m_aryDocPaths.GetSize(); i++) {
						AddParamStatementToSqlBatch(strSql, params, 
							"INSERT INTO FaxLogT (FaxType, UserID, Path, TransactionID, DateSent, MailID) "
							"values ({INT}, {INT}, {STRING}, {INT}, GetDate(), {VT_I4})", 
							esfsMyFax, GetCurrentUserID(), settings.m_aryDocPaths.GetAt(i), nTransID, varMailID);
					}

					//Insert it
					if(strSql.IsEmpty()) {
						//This should be impossible
						AfxThrowNxException("The fax was sent, but no documents were found.");
						return false;
					}

					// (e.lally 2009-06-21) PLID 34680 - Leave as execute batch
					ExecuteParamSqlBatch(GetRemoteData(), strSql, params);
					AfxMessageBox(strRetMsg);
					return true; // the only full success path
				}
			}
		}
		else {
			//Some kind of error occurred.  We already got the message, and will output it 
			//	below.  No further processing needs done.
			if(GetRemotePropertyInt("SaveSentFaxInHistory", 1, 0, NULL, true) != 0){
				if(nPersonID > 0){
					//(e.lally 2011-10-31) PLID 41195 - Log failures in mailSent also
					// (d.singleton 2013-11-15 11:23) - PLID 59513 - need to insert the CCDAType when generating a CCDA
					CString strNote = "Fax submission failed." + strFaxDetails;
					CreateNewMailSentEntry(nPersonID, strNote, "", "", GetCurrentUserName(), "", GetCurrentLocationID(),
						g_cdtNull, -1, -1, nPicID, -1, FALSE, -1, "", ctNone);
				}
			}

			AfxMessageBox(strRetMsg);
			return false; // failed
		}
	}
}

//DRT 6/27/2008 - PLID 30524
//Returns an XML document formatted to the specifications required for the 
//	web service SendSingleFax.  The settings object passed in will be 
//	used to flesh out the XML parameters.
bool CMyFaxSend::GenerateSendSingleFaxDocument(IN CFaxSettings settings, OUT CString &strXmlDocument)
{
	//
	//Validate input data.  These are from the MyFax XML Web Services User Guide.  See d.thompson
	//	who has a copy in his email, it's a PDF document.
	{
		//First, format the fax number.  This needs stripped of all non-numerics.
		{
			CString str = settings.m_strRecipNumber;
			CString strRes;

			for(int i = 0; i < str.GetLength(); i++) {
				// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
				if(isdigit(unsigned char(str.GetAt(i))))
					strRes += str.GetAt(i);
			}

			//Ensure the recipient number has the country code.  MyFax will not prepend a 1 for US dialing
			//	for you.
			if(strRes.GetLength() == 10) {
				//North America number without the leading 1
				strRes = "1" + strRes;
			}

			settings.m_strRecipNumber = strRes;
		}

		//ID 1 to 2 billion
		long nID = atoi(settings.m_strUser);
		if(nID >= 1 && nID <= 2000000000) {
			//We are happy
		}
		else {
			AfxMessageBox("The MyFax User ID value must be between 1 and 2,000,000,000");
			return false;
		}

		//Password 20 chars
		if(settings.m_strPassword.GetLength() > 20) {
			AfxMessageBox("The MyFax Password value must be 20 characters or less.");
			return false;
		}

		//from name 50 chars
		if(settings.m_strFromName.GetLength() > 50) {
			AfxMessageBox("The MyFax From Name value must be 50 characters or less.");
			return false;
		}

		//recipient 7 - 20 chars
		if(settings.m_strRecipNumber.GetLength() > 20 || settings.m_strRecipNumber.GetLength() < 7) {
			AfxMessageBox("The MyFax Recipient Fax Number must be between 7 and 20 characters.");
			return false;
		}

		//recipient name 50 chars
		if(settings.m_strRecipName.GetLength() > 50) {
			AfxMessageBox("The MyFax Recipient Fax Name must be 50 characters or less.");
			return false;
		}
	}
	//end validation
	//

	//
	//The main portion of the XML output is converting the document itself into a sendable bit of text.  Do that here.
	CString strDocumentXML;
	{
		//There can be multiple documents, so run them each through the generation, then compile
		CString strTmp;
		for(int i = 0; i < settings.m_aryDocPaths.GetSize(); i++) {
			if(!GenerateDocumentXML(settings.m_aryDocPaths.GetAt(i), strTmp)) {
				//Something failed generating the document (like the file not being accessible), so quit.
				return false;
			}

			strDocumentXML += strTmp;

			
			// (a.walling 2010-01-29 17:44) - PLID 36790 - Check if the combination of all the documents is too large.
			const DWORD cdwMaximumLength = 10000000 - 10000;
			if (strDocumentXML.GetLength() > cdwMaximumLength) {
				AfxMessageBox("The documents you are attempting to fax are too large; there is a limit of approximately 7.5 megabytes for single faxes. Please try again with a fewer and/or smaller documents.");
				strDocumentXML.Empty();
				return false;
			}
		}
	}

	//DRT 6/26/2008 - If I set this to true, even if the file really is a tiff image, the fax will fail.  So I guess we'll
	//	just always leave it false.
	CString strTiff = "false";

	//
	//Generate the output
	strXmlDocument = 
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\r\n"
	"<single_fax_info xmlns=\"http://www.protus.com\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\r\n"
		"<SchemaVersion>1.2</SchemaVersion>\r\n"
		"<login_key>\r\n" + 
			 GetXMLElementValuePair("user_id", settings.m_strUser) +
			 GetXMLElementValuePair("user_password", settings.m_strPassword) + 
		"</login_key>\r\n"
		"<single_fax_options>\r\n" + 
			GetXMLElementValuePair("from_name", settings.m_strFromName) + 
			GetXMLElementValuePair("tiff_image_flag", strTiff) + 
			GetXMLElementValuePair("resolution", settings.m_strResolution) + 
			"<cover_page>\r\n" + 
			// (z.manning 2009-09-29 13:10) - PLID 32472 - Added cover page subject
				GetXMLElementValuePair("cover_page_subject", settings.m_strCoverPageSubject) + 
			"</cover_page>\r\n"
		"</single_fax_options>\r\n"
		"<fax_recipient>\r\n" + 
			GetXMLElementValuePair("fax_recipient_number", settings.m_strRecipNumber) + 
			GetXMLElementValuePair("fax_recipient_name", settings.m_strRecipName) + 
		"</fax_recipient>\r\n"
		"<document_list>\r\n" + 
			strDocumentXML +
		"</document_list>\r\n"
	"</single_fax_info>";

	return true;
}

//Generates the XML for the entire <document> tag.  strDocPath needs to be a full path
//	to the file, or local to the current working directory (accessible by a CFile constructor).
//Example:
//			<document document_content_type=\"text/plain\" document_encoding_type=\"none\" document_extension=\"txt\">
//				TEXT OF DOCUMENT HERE
//			</document>
bool CMyFaxSend::GenerateDocumentXML(IN CString strDocPath, OUT CString &strDocumentXML)
{
	//First, open the file given
	//	This will throw an exception if there's a problem with the file, ending the fax attempt.
	CFile fDoc(strDocPath, CFile::modeRead|CFile::shareDenyNone);

	CString strFileName = fDoc.GetFileName();
	//Get the extension
	CString strExt;
	long nLastDot = strFileName.ReverseFind('.');
	if(nLastDot > -1) {
		strExt = strFileName.Mid(nLastDot + 1);
	}
	else {
		//no extension to the file.
		strExt = "";
	}

	//Determine the content type (text/plain, application/pdf, etc)
	CString strContentType = GetWebContentType(strExt);

	//Read the document
	// (a.walling 2008-10-02 10:47) - PLID 31567 - VS2008 - GetLength() can be a 64 bit int
	ULONGLONG ullFileSize = (ULONGLONG)fDoc.GetLength();
	DWORD dwFileSize = (DWORD)ullFileSize;

	if(ullFileSize > LONG_MAX) {
		AfxThrowNxException("GenerateDocumentXML() - Cannot be called for files over 2 GB in size.");
	}

	// (a.walling 2010-01-19 17:45) - PLID 36790 - Check that the encoded output size is not too large. The limit is 10 mb according to MyFax.
	// However this encoded document is wrapped in other XML and the SOAP request itself. Additionally it is not specified whether this is 10
	// megabytes as in 1024 or 10 megabytes as in 1000. So let's be pessimistic and subtract 10,000 for overhead from 10,000,000 bytes.
	const DWORD cdwMaximumLength = 10000000 - 10000;
	DWORD dwEncodedSize = CBase64::GetEncodedSize(dwFileSize);

	if (dwEncodedSize > cdwMaximumLength) {
		// (a.walling 2010-01-19 17:48) - PLID 36790 - Too big. Give a message box and fail.
		// Just use an AfxMessageBox like GenerateSendSingleFaxDocument does
		AfxMessageBox("The document you are attempting to fax is too large; there is a limit of approximately 7.5 megabytes for single faxes. Please try again with a smaller document.");
		return false;
	}

	BYTE *pDoc = new BYTE[dwFileSize];
	fDoc.Read(pDoc, dwFileSize);

	//Encode the document using base64 encoding.  This is the recommended encoding
	//	scheme for MyFax, and much web content.  Documentation is in the CBase64 class.
	CString strEncodedDoc = CBase64::Encode(pDoc, dwFileSize);

	//Finally, format our XML document with the appropriate data, and with the encoded document itself.
	strDocumentXML.Format(
		"<document document_content_type=\"" + strContentType + "\" document_encoding_type=\"base64\" document_extension=\"" + strExt + "\">\r\n"
			"%s\r\n"
		"</document>\r\n", strEncodedDoc);

	//cleanup after ourselves.
	fDoc.Close();
	delete pDoc;

	//everyone is happy
	return true;
}


//End CMyFaxSend class
////////////////////////////////////////////////////////////////

//DRT 6/26/2008 - PLID 30524
//Loads a hardcoded list of available services into a datalist2.  The datalist must have 2 columns, an ID for the type, 
//	and a string for the text.
void LoadServiceList(NXDATALIST2Lib::_DNxDataListPtr pList)
{
	//Add each supported service here

	//1)  MyFax.  Added 6/26/2008.
	NXDATALIST2Lib::IRowSettingsPtr pRow = pList->GetNewRow();
	pRow->PutValue(0, (long)esfsMyFax);
	pRow->PutValue(1, _bstr_t("MyFax"));
	pList->AddRowSorted(pRow, NULL);
}

//DRT 6/26/2008 - PLID 30524
//Loads the available document resolutions.  As of 2008-06-26, we only have myFax, and honestly
//	I don't see why you'd ever send it as a low resolution.  But high & low are there, so we 
//	support the options.
//The datalist must have 1 column, a string for the text.
void LoadResolutions(NXDATALIST2Lib::_DNxDataListPtr pList)
{
	//High res
	NXDATALIST2Lib::IRowSettingsPtr pRow = pList->GetNewRow();
	pRow->PutValue(0, _bstr_t("High"));
	pList->AddRowAtEnd(pRow, NULL);

	//Low res
	pRow = pList->GetNewRow();
	pRow->PutValue(0, _bstr_t("Low"));
	pList->AddRowAtEnd(pRow, NULL);
}

//Gets the content type, given an extension.  There really should be a way of automating this and
//	getting the "standard" descriptions, but I cannot find it.
//TES 3/4/2009 - PLID 32078 - Moved here from SOAPUtils
CString GetWebContentType(CString strExt)
{
	//TODO:  myFax supports 178 formats, so finish going through these at some point.  Full list here:
	//	http://www.myfax.com/collaterals/UserGuide_Supported_File_Formats.pdf
	//The initial set of these are created and filled in WebFaxSetup.mod

	//DRT 6/27/2008 - PLID 30524 - I put all these in data, so we can easily make changes on the fly if needed.
	ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT Type FROM WebContentTypesT WHERE Extension = {STRING}", strExt);
	if(!prs->eof) {
		return AdoFldString(prs, "Type");
	}

	//Not found, we're going to default it to plain text.
	return "text/plain";
}