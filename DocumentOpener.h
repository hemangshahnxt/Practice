#pragma once

#define PATHNAME_OBJECT_CASEHISTORY		"OBJECT:CASEHISTORY"
#define PATHNAME_FORM_REPRODUCTIVE		"FORM:REPRODUCTIVE"
// (j.jones 2007-06-27 17:16) - PLID 26480 - added for Eligibility to interact with MailSent
#define PATHNAME_OBJECT_ELIGIBILITY_REQUEST	"OBJECT:ELIGIBILITYREQUEST"
#define PATHNAME_OBJECT_ELIGIBILITY_RESPONSE	"OBJECT:ELIGIBILITYRESPONSE"

// (c.haag 2015-05-04) - NX-100442 - This class contains implementations for opening various kinds
// of supported documents.
class CDocumentOpener
{
public:
	struct Result
	{
		// True if the document was opened
		BOOL bDocumentOpened;
		// True if data changed and the active view needs to be refreshed
		BOOL bNeedUpdateView;

		Result(BOOL bDocumentOpened, BOOL bNeedUpdateView)
		{
			this->bDocumentOpened = bDocumentOpened;
			this->bNeedUpdateView = bNeedUpdateView;
		}
	};

public:
	// Attempts to open a reproductive form and returns to the caller whether the object was opened and whether 
	// any data changed in the process. The caller is responsible for determining that the path points to a valid object.
	static Result OpenReproductiveForm(const CString& strPath);

	// Attempts to open a case history and returns to the caller whether the object was opened and whether 
	// any data changed in the process. The caller is responsible for determining that the path points to a valid object.
	static Result OpenCaseHistory(const CString& strPath, long nPersonID, long nMailID, CWnd* pParentWnd, IN OUT BOOL& bCaseHistoryIsOpen);
	
	// Attempts to open an eligibility request and returns to the caller whether the object was opened and whether 
	// any data changed in the process. The caller is responsible for determining that the path points to a valid object.
	static Result OpenEligibilityRequest(const CString& strPath, long nPersonID, long nMailID);

	// Attempts to open an XML document and returns to the caller whether the object was opened and whether 
	// any data changed in the process. The caller is responsible for determining that the path points to a valid object.
	static Result OpenXMLDocument(CString strPath, long nMailID, CWnd* pParentWnd);

	// Attempts to open an XML document and returns to the caller whether the object was opened and whether 
	// any data changed in the process. The caller is responsible for determining that the document is valid.
	static Result OpenXMLDocument(MSXML2::IXMLDOMDocument2Ptr pDocument, long nMailID, CWnd* pParentWnd);

	// Attempts to open a collection of Word documents and returns to the caller whether all documents were opened and 
	// whether any data changed in the process. The caller is responsible for determining that the path points to a valid object.
	static Result OpenPacket(long nMergedPacketID, long nPersonID);

	// Attempts to open an attachment from a person history and returns to the caller whether all documents were opened and 
	// whether any data changed in the process. This code behaves identically to the code in CHistoryDlg, and may raise modal
	// dialogs depending on the situation. This does not support Letter Writing packets.
	static Result OpenHistoryAttachment(CString strPath, long nPersonID, long nMailID, CWnd* pParentWnd, IN OUT BOOL& bCaseHistoryIsOpen, const CString& strName);

	// Attempts to open a document with the default Windows-based viewer. The path may be absolute or relative 
	// to the patient documents folder and returns to the caller whether the object was opened and whether 
	// any data changed in the process. The caller is responsible for determining that the path points to a valid object.
	static Result OpenDocument(CString strPath, long nPersonID);

private:
	// Determines whether a todo alarm can be created for a document at a specified path. The path may be a special
	// built-in sentinel path.
	static BOOL CanCreateTodoForDocument(const CString& strPath);
	
};