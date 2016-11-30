// DirectMessageSendDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "DirectMessageSendDlg.h"
#include "NxApi.h"
#include "FinancialRC.h"
#include "SingleSelectMultiColumnDlg.h"
#include "NxXMLUtils.h"
#include <iostream>

// (a.walling 2014-04-24 12:00) - VS2013 - no using std in global headers
using namespace std;

// (j.camacho 2013-11-06 12:30) - PLID 59303
enum DirectMessageAttachmentList {

	dmalFileName = 0,
	dmalView,
};

// (j.camacho 2013-11-11 13:27) - PLID 58929
enum DirectMessageFromAddress {
	dmfaAddress =0	
};


// (j.camacho 2013-10-09 17:37) - PLID 58929 - CCDA message dialog 
// CDirectMessageSendDlg dialog

IMPLEMENT_DYNAMIC(CDirectMessageSendDlg, CNxDialog)

CDirectMessageSendDlg::CDirectMessageSendDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CDirectMessageSendDlg::IDD, pParent)
{
	m_lMailID = NULL;
	m_nPatientID = -1;
}

CDirectMessageSendDlg::~CDirectMessageSendDlg()
{
}

void CDirectMessageSendDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_DIRECTMESSAGE_SEND_NXCOLORCTRL1, m_nxcolorBackgroundTop);
	DDX_Control(pDX, IDC_DIRECTMESSAGE_TO_HELP, m_btnQuestion);
	DDX_Control(pDX, IDC_DIRECTMESSAGE_SEND_TO, m_nxeditCCDATo);
	DDX_Control(pDX, IDC_DIRECTMESSAGE_SEND_BODYTEXT, m_nxeditCCDAMessage); 
	DDX_Control(pDX, IDC_DIRECTMESSAGE_ATTACH_FILES, m_btnAttachFiles); 
}

BOOL CDirectMessageSendDlg::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog();

	//set dialog visuals
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnQuestion.AutoSet(NXB_QUESTION);
		m_nxcolorBackgroundTop.SetColor(GetNxColor(GNC_FINANCIAL, 0));
		m_btnAttachFiles.AutoSet(NXB_MODIFY); 

		//initialize From list
		m_dlDirectmessageFrom = BindNxDataList2Ctrl(IDC_DIRECTMESSAGE_SEND_FROM,false);
		m_dlDirectmessageFrom->PutWhereClause(FormatBstr("USERID = %li",GetCurrentUserID()));
		m_dlDirectmessageFrom->Requery();
		m_dlDirectmessageFrom->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_dlDirectmessageFrom->GetFirstRow());
		m_dlDirectmessageFrom->PutCurSel(pRow);

		//initialize referring physician list
		// (j.camacho 2013-11-20 16:19) - PLID 59444
		m_dlRefAddress = BindNxDataList2Ctrl(IDC_DIRECT_MESSAGE_SEND_REFADDRESS,false);
		m_dlRefAddress->Requery();
		m_dlRefAddress->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		

		//initialize attachments list
		m_dlAttachments = BindNxDataList2Ctrl(IDC_DIRECT_MESSAGE_ATTACHMENTS_SEND_LIST,false);
		LoadAttachmentsList();
		// (b.spivey, February 5th, 2014) - PLID 60648 - If we have no thumbprint, then don't let them do anything lest they get erros.
		if (!ReturnsRecords("SELECT * FROM DirectCertificateThumbprintT ")) {
			AfxMessageBox("There is no Direct Certificate Thumbprint in data. Please enter one in the Direct Message tab "
				"of the Links Module.");
			this->CenterWindow(); 
			DisableControls(); 
		}
		return TRUE;

	}NxCatchAll(__FUNCTION__);
	return FALSE;
}

// (j.camacho 2013-10-23 17:29) - PLID 58929 - Send messages via API
void CDirectMessageSendDlg::OnOK()
{
	try{
		CString strBodyText,strToAddress,strSubjectLine,strFromAddress;
		GetDlgItemTextA(IDC_DIRECTMESSAGE_SEND_BODYTEXT,strBodyText);
		GetDlgItemTextA(IDC_DIRECTMESSAGE_SEND_TO,strToAddress);
		GetDlgItemTextA(IDC_DIRECT_MESSAGE_SEND_SUBJECT,strSubjectLine);
		if(strSubjectLine == "")
		{
			AfxMessageBox("Subject is blank. Please enter a subject and try again.");
			return;
		}
		if(strToAddress == "")
		{
			AfxMessageBox("There are no To addresses. Please enter an address and try again.");
			return;
		}
		if(strBodyText == "" && m_vAttachments.size()<=0)
		{
			AfxMessageBox("Body Text is blank. Please enter a message and try again.");
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlDirectmessageFrom->GetCurSel();
		if(pRow){
			strFromAddress = VarString(pRow->GetValue(dmfaAddress));
		}
		else
		{
			//no row is selected
		}
		//To address
		CStringArray straryToAddress; 
		ParseDelimitedStringToStringArray(strToAddress, ";", straryToAddress);
		Nx::SafeArray<BSTR> saToAddress; 
		for ( int i = 0; i < straryToAddress.GetCount(); i++) {
			CString strToAddress = straryToAddress.ElementAt(i);
			strToAddress = strToAddress.Trim(" ");
			if(strToAddress.GetLength() > 0) {
				saToAddress.Add(strToAddress);
			}
		}

		//Attach Files To Message
		Nx::SafeArray<IUnknown *> saAttachments;
		//go through all files in attachmentslist
		for (unsigned int i = 0; i < m_vAttachments.size(); i++) {
			
			// (b.spivey, May 13th, 2014) - PLID 61804 - One logic loop for attachments. 
			NexTech_Accessor::_DirectMessageAttachmentPtr pAttachment(__uuidof(NexTech_Accessor::DirectMessageAttachment));
			//add actually attaching the file
			pAttachment->AttachmentFileName = _bstr_t(m_vAttachments[i].docname);
			if(!m_vAttachments[i].bHasFileBytes)
			{
				Nx::SafeArray<BYTE> fileData(FileUtils::LoadFileIntoSafeArrayOfBytes(m_vAttachments[i].pathname, FileUtils::pfiNone).parray);
				pAttachment->AttachmentFileData = fileData;
			}
			else if (m_vAttachments[i].bHasFileBytes) {
				pAttachment->AttachmentFileData = m_vAttachments[i].fileBytes; 
			}

			if (m_vAttachments[i].nMailID > 0) {
				 // (j.camacho 2013-11-15 17:09) - PLID 59514
				NexTech_Accessor::_NullableLongPtr pMailID(__uuidof(NexTech_Accessor::NullableLong));
				// (d.singleton 2014-04-23 17:31) - PLID 61806 mailID per attachment now
				pMailID->SetLong(m_vAttachments[i].nMailID); 
				pAttachment->AttachmentMailSentID = pMailID;
			}
			else {
				NexTech_Accessor::_NullableLongPtr pMailID(__uuidof(NexTech_Accessor::NullableLong));
				pMailID->SetNull(); 
				pAttachment->AttachmentMailSentID = pMailID;
			}
			
			saAttachments.Add(pAttachment); 		

		}
		try{
			HRESULT sendResult = GetAPI()->SendDirectMessage(GetAPISubkey(),GetAPILoginToken(),_bstr_t(strBodyText), saToAddress,_bstr_t(strFromAddress), _bstr_t(strSubjectLine), saAttachments); //change email
					//Validate before Successful OnOK
			if(sendResult==S_OK){
				//Successful send, close window
				CDialog::OnOK();
			}
			else
			{
				AfxMessageBox("Message could not be sent. Please try again later.");
			}
		}catch(_com_error e)
		{	
			// (j.camacho 2013-11-20 12:31) - PLID 59650
			CString errorMessage = (LPCTSTR)e.Description();
			//pretty error for bad 'To addresses'
			if(errorMessage.Find("To or Cc")>=0)
			{
				Log(e.Description());
				AfxMessageBox("Invalid direct address. Please try again with a valid direct address.");
			}
			else if(errorMessage.Find("not a valid email address")>=0)
			{
				Log(e.Description());
				AfxMessageBox("One of your addresses was invalid. Please try again with valid direct addresses.");
			}
			else if(errorMessage.Find("MailID")>=0)
			{
				Log(e.Description());
				AfxMessageBox("Please make sure this file is attached to your patient history.");
			}
			// (b.spivey, May 19th, 2014) PLID 62069 - catch configuration errors and tell the user what's going on. 
			else if (errorMessage.Find("An unsecured or incorrectly secured fault was received from the other party.") >= 0
				&& errorMessage.Find("An error occurred when verifying security for the message.") >= 0) {
					AfxMessageBox("An error was discovered verifying the security of the request. Typically this means that the server time is skewed more "
						"than five minutes from the current time. Please check your server time and try again."); 
			} 
			else if (errorMessage.Find("and allowed clock skew is ''00:05:00''") >= 0 && errorMessage.Find("timestamp is invalid because its creation time") >= 0) {
				AfxMessageBox("Your server time is skewed more than five minutes from the current time. Please update your service time and try again."); 
			}
			else
			{				
				//catch general errors just in case
				Log(e.Description());
				AfxMessageBox("An error has occured. Please try again in a few minutes.");
			}
		}
	
	}NxCatchAll(__FUNCTION__);
}

// (j.camacho 2013-11-04 17:31) - PLID 59303
//unused funtion
/*void CDirectMessageSendDlg::AttachFilesToMessage()
{
	try
	{
		Nx::SafeArray<IUnknown *> saAttachments;
		//go through all files in attachmentslist
		for (unsigned int i = 0; i < m_vAttachments.size()+1; i++) {
			NexTech_Accessor::_DirectMessageAttachmentPtr pAttachment(__uuidof(NexTech_Accessor::DirectMessageAttachment));
			//add actually attaching the file
			pAttachment->AttachmentFileName = _bstr_t(m_vAttachments[i].docname);
			Nx::SafeArray<BYTE> fileData(FileUtils::LoadFileIntoSafeArrayOfBytes(m_vAttachments[i].pathname, FileUtils::pfiNone).parray);
			pAttachment->AttachmentFileData = fileData;
			saAttachments.Add(pAttachment); 
		}
	}NxCatchAll(__FUNCTION__);
	
}*/

// (j.camacho 2013-11-05 09:05) - PLID 59303 - Accessor to member attachment lists.

void CDirectMessageSendDlg::AddToAttachments(CString attachmentPath, CString fileName)
{
	try
	{
		attachmentFile Attachment;
		Attachment.docname = fileName;
		Attachment.pathname = attachmentPath^fileName;
		// (j.gruber 2013-11-11 09:11) - PLID 59403 - include our extra fields
		Attachment.bHasFileBytes = false;
		//we have to create this with a length of 0
		//Attachment.fileBytes = NULL;
		Attachment.nMailID = NULL;
		m_vAttachments.push_back(Attachment);

		
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2013-11-11 09:12) - PLID 59403 - overloaded function that takes the actual array of bytes
void CDirectMessageSendDlg::AddToAttachments(Nx::SafeArray<BYTE> fileBytes, CString fileName)
{
	try
	{
		attachmentFile Attachment;
		Attachment.docname = fileName;
		Attachment.pathname = "";		
		Attachment.bHasFileBytes = true;
		Attachment.fileBytes = fileBytes;
		Attachment.nMailID = NULL;
		m_vAttachments.push_back(Attachment);
		
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-04-23 17:23) - PLID 61806 - another overload that takes the mailid so we can add it per attachment,  not once for the dlg
void CDirectMessageSendDlg::AddToAttachments(CString attachmentPath, CString fileName, long nMailID)
{
	try
	{
		attachmentFile Attachment;
		Attachment.docname = fileName;
		Attachment.pathname = attachmentPath^fileName;
		// (j.gruber 2013-11-11 09:11) - PLID 59403 - include our extra fields
		Attachment.bHasFileBytes = false;
		//we have to create this with a length of 0
		//Attachment.fileBytes = NULL;
		// (d.singleton 2014-04-23 17:23) - PLID 61806 need to include the mailID per attachment
		Attachment.nMailID = nMailID;
		m_vAttachments.push_back(Attachment);

		
	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, May 14th, 2014) PLID 62163 - Added a remove function to remove attachments from the list. 
void CDirectMessageSendDlg::RemoveFromAttachments(CString strFileName) 
{
	vector<attachmentFile>::iterator it = m_vAttachments.begin();
	while (it != m_vAttachments.end()) {
		if (it->docname.CompareNoCase(strFileName) == 0) {
			it = m_vAttachments.erase(it); 
			break;
		}
		else {
			it++;
		}
	}
}

void CDirectMessageSendDlg::LoadAttachmentsList()
{
	if(m_dlAttachments->GetRowCount())
	{
		m_dlAttachments->Clear();
	}
	for (unsigned int i = 0; i < m_vAttachments.size(); i++)
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlAttachments->GetNewRow();
		pRow->PutValue(dmalFileName, _bstr_t(m_vAttachments[i].docname));
		pRow->PutValue(dmalView, "");
		m_dlAttachments->AddRowAtEnd(pRow, NULL); 
	}
}

// (b.spivey, May 13th, 2014) - PLID 61804 - Mutator for patient ID. 
void CDirectMessageSendDlg::SetPatientID(long nPatientID) 
{
	m_nPatientID = nPatientID;
}

BEGIN_MESSAGE_MAP(CDirectMessageSendDlg, CNxDialog)
	
	ON_BN_CLICKED(IDC_DIRECTMESSAGE_TO_HELP, &CDirectMessageSendDlg::OnBnClickedDirectmessageToHelp)
	ON_BN_CLICKED(IDC_DIRECTMESSAGE_ATTACH_FILES, &CDirectMessageSendDlg::OnBnClickedAttachFiles)
END_MESSAGE_MAP()


// CDirectMessageSendDlg message handlers

BEGIN_EVENTSINK_MAP(CDirectMessageSendDlg, CNxDialog)
	ON_EVENT(CDirectMessageSendDlg, IDC_DIRECTMESSAGE_SEND_FROM, 1, CDirectMessageSendDlg::SelChangingDirectmessageSendFrom, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CDirectMessageSendDlg, IDC_DIRECT_MESSAGE_SEND_REFADDRESS, 1, CDirectMessageSendDlg::SelChangingDirectMessageSendRefaddress, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CDirectMessageSendDlg, IDC_DIRECT_MESSAGE_SEND_REFADDRESS, 16, CDirectMessageSendDlg::SelChosenDirectMessageSendRefaddress, VTS_DISPATCH)
	ON_EVENT(CDirectMessageSendDlg, IDC_DIRECT_MESSAGE_ATTACHMENTS_SEND_LIST, 6, CDirectMessageSendDlg::RButtonDownDirectMessageAttachments, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CDirectMessageSendDlg::SelChangingDirectmessageSendFrom(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//Don't let them select nothing
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	} NxCatchAll(__FUNCTION__);

}

void CDirectMessageSendDlg::SelChangingDirectMessageSendRefaddress(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//Don't let them select nothing
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.camacho 2013-11-13 15:50) - PLID 59444 - Allow users to set email addresses from their store of referring physicians with addresses available.
void CDirectMessageSendDlg::SelChosenDirectMessageSendRefaddress(LPDISPATCH lpRow)
{
	try{
		if(lpRow){
			//add email to To box
			CString strEmail;
			GetDlgItemTextA(IDC_DIRECTMESSAGE_SEND_TO, strEmail);
		
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			if(pRow){
				if(strEmail == "")
				{
					strEmail = VarString(pRow->GetValue(2));
				}
				else
				{
					if(strEmail.Right(1)==';')
					{
						strEmail += " "+VarString(pRow->GetValue(2))+";";
					}
					else
					{
						strEmail += "; "+VarString(pRow->GetValue(2))+";";
					}
				}
				SetDlgItemTextA(IDC_DIRECTMESSAGE_SEND_TO,strEmail);
				//m_dlRefAddress->RemoveRow(pRow);
				m_dlRefAddress->CurSel = NULL; 
			}
		}
		else{ 
		//no row was chosen
		}
	}NxCatchAll(__FUNCTION__);
	

}

void CDirectMessageSendDlg::OnBnClickedDirectmessageToHelp()
{
	// TODO: Add your control notification handler code here
	AfxMessageBox("To send a message to multiple direct addresses, remember to separate each address with a semi-colon.");
}

// (b.spivey, May 13th, 2014) - PLID 61804 - button menu for attaching files. 
void CDirectMessageSendDlg::OnBnClickedAttachFiles()
{
	try {
		CMenu mnu;
		mnu.CreatePopupMenu();

		if (m_nPatientID > 0) {
			mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, 1, "&Existing File From History...");
		}
		mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, 2, "&New File...");

		CRect rButton;
		GetDlgItem(IDC_DIRECTMESSAGE_ATTACH_FILES)->GetWindowRect(rButton);
		int nSelection = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, rButton.right, rButton.top, this);

		switch(nSelection){
			case 1:		// attach existing
				AttachExistingFile();
				break;
			case 2:		// attach new
				AttachNewFile();
				break;
			case 0:		// nothing selected
				break;
			default:
				ThrowNxException("CDirectMessageSendDlg::OnBnClickedAttachFiles() - Invalid Menu Option!");
		}

	} NxCatchAll(__FUNCTION__); 
}

// (b.spivey, May 13th, 2014) - PLID 61804 - attach a new file 
void CDirectMessageSendDlg::AttachNewFile()
{

	CString strInitPath;
	
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT , szCommonlyAttached, this);

	
	if (m_nPatientID > 0) {
		strInitPath = GetPatientDocumentPath(m_nPatientID);
	}
	else {
		strInitPath = GetSharedPath();
	}

	if(!DoesExist(strInitPath)) {
		// This shouldn't be possible because either GetPatientDocumentPath should return a 
		// valid path to a folder that exists, or it should throw an exception
		ASSERT(FALSE);
		AfxThrowNxException("Person document folder '%s' could not be found", strInitPath);
		return;
	}

	dlg.m_ofn.lpstrInitialDir = strInitPath;
	// (b.spivey, June 11th, 2014) - PLID 61804 - When I jacked this code from someone else, I copied the badness. This opened up the possibility for a memory leak. 
	//	 Fixed that by not making it a pointer. 
	char strFile[5000];
	strFile[0] = 0;
	dlg.m_ofn.nMaxFile = 5000;
	dlg.m_ofn.lpstrFile = strFile;

	if (dlg.DoModal() == IDOK) {
		POSITION p = dlg.GetStartPosition();
		if (p) {	

			CString strPath = dlg.GetNextPathName(p);

			if(m_dlAttachments->FindByColumn(dmalFileName, _variant_t(GetFileName(strPath)), 0, FALSE)) {
				AfxMessageBox("A file with that name is already attached. Change the file name and try again.", MB_ICONWARNING|MB_OK);
				return; 
			}

			AddToAttachments(GetFilePath(strPath), GetFileName(strPath)); 

			LoadAttachmentsList();
			
		}
	}
}

// (b.spivey, May 13th, 2014) - PLID 61804 - attach a file from the patient's record. 
void CDirectMessageSendDlg::AttachExistingFile()
{
	//1. set up the fields we are interested in having
	CStringArray aryColumns;
	aryColumns.Add("MailSent.MailID");
	aryColumns.Add("MailSent.ServiceDate");
	aryColumns.Add("MailSentNotesT.Note");
	aryColumns.Add("CatsSubQ.Description");
	aryColumns.Add("MailSent.PathName");

	//2. Column Headers (order matters)
	CStringArray aryColumnHeaders;
	aryColumnHeaders.Add("Mail ID");
	aryColumnHeaders.Add("Service Date");
	aryColumnHeaders.Add("Description");
	aryColumnHeaders.Add("Category");
	aryColumnHeaders.Add("File Name");

	//3. Sort order for columns (order matters)
	CSimpleArray<short> arySortOrder;
	arySortOrder.Add(-1);
	arySortOrder.Add(0);
	arySortOrder.Add(1);
	arySortOrder.Add(2);
	arySortOrder.Add(3);

	CString strWhere;
	strWhere.Format(" MailSent.PersonID = %li AND InternalRefID IS NULL AND MailSent.Selection <> '' AND MailSent.Selection <> 'BITMAP:FOLDER' ", m_nPatientID);

	// Open the single select dialog
	CSingleSelectMultiColumnDlg dlg(this);
	HRESULT hr = dlg.Open(
		" MailSent LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
		" LEFT JOIN ( "
		"	SELECT ID, Description "
		"	FROM NoteCatsF "
		"	WHERE IsPatientTab = 1 "
		" ) CatsSubQ ON MailSent.CategoryID = CatsSubQ.ID ",								/*From*/
		strWhere,																			/*Where*/
		aryColumns,																			/*Select*/
		aryColumnHeaders,																	/*Column Names*/
		arySortOrder,																		/*Sort Order*/
		"[1] - [2]",																		/*Display Columns*/
		"Please select an existing file to attach to this direct message.",					/*Description*/
		"Select an Existing File"															/*Title Bar Header*/
		);

	// If they clicked OK, be sure to check if they made a valid selection
	if(hr == IDOK){
		CVariantArray varySelectedValues;
		dlg.GetSelectedValues(varySelectedValues);

		if(!varySelectedValues.GetSize()){
			this->MessageBox("You did not select an existing file to attach. Please try again.", "No File Selected", MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		long nNewMailID = VarLong(varySelectedValues.GetAt(0), -1);
		CString strFileName = VarString(varySelectedValues.GetAt(4), "");

		CString strPatDocPath = GetPatientDocumentPath(m_nPatientID);
		if(!FileUtils::EnsureDirectory(strPatDocPath)) {
			// This shouldn't be possible because either GetPatientDocumentPath should return a 
			// valid path to a folder that exists, or it should throw an exception
			ASSERT(FALSE);
			AfxThrowNxException("Person document folder '%s' could not be found", strPatDocPath);
			return;
		}

		if(!FileUtils::DoesFileOrDirExist(strPatDocPath ^ strFileName)) {
			CString strWarning = "";
			strWarning.Format("The following file was not found. Make sure it exists and try again. \r\n\r\n%s", (strPatDocPath ^ strFileName)); 
			AfxMessageBox(strWarning, MB_ICONWARNING|MB_OK);
			return; 
		}

		if(m_dlAttachments->FindByColumn(dmalFileName, _variant_t(strFileName), 0, FALSE)) {
			AfxMessageBox("A file with that name is already attached. Change the file name and try again.", MB_ICONWARNING|MB_OK);
			return; 
		}

		// (b.spivey, June 11th, 2014) - PLID 61804 - If you add a CCDA we need to add a human readable version.
		if (NxXMLUtils::IsCCDAFile(strPatDocPath^strFileName))
		{
			CString strMailSentID = AsString(nNewMailID);

			//we need to add both the xml and a pdf, so get the PDF file now
			NexTech_Accessor::_HistoryEntryPDFResultPtr pResult = GetAPI()->GetHistoryEntryPDF(GetAPISubkey(), GetAPILoginToken(), _bstr_t(strMailSentID));
			CString strExt = FileUtils::GetFileExtension(strFileName);
			CString strPDFFileName = strFileName;
			strPDFFileName.Replace(strExt, "pdf");
			Nx::SafeArray<BYTE> fileBytes = pResult->PDFFile;
			AddToAttachments(fileBytes, strPDFFileName);
		}

		AddToAttachments(strPatDocPath, strFileName, nNewMailID); 

		LoadAttachmentsList();
		
	}
}

// (b.spivey, May 14th, 2014) PLID 62163 - Event handler for right click menu. 
void CDirectMessageSendDlg::RButtonDownDirectMessageAttachments(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
	if(pRow) {
		//make this row the cur sel
		m_dlAttachments->PutCurSel(pRow);
		//now make pop up
		
		CMenu pMenu;
		pMenu.CreatePopupMenu();
		pMenu.AppendMenu(MF_ENABLED|MF_BYPOSITION, 1, "Remove Attachment");
		
		CPoint pt;
		GetCursorPos(&pt);
		int nSelection = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
		if (nSelection == 1) {
			RemoveFromAttachments(VarString(pRow->GetValue(dmalFileName), "")); 
			LoadAttachmentsList();
		}
	}

}

// (b.spivey, February 5th, 2014) - PLID 60648 - Disable all the controls except Cancel.
void CDirectMessageSendDlg::DisableControls() 
{
	m_btnOK.EnableWindow(FALSE);
	m_btnQuestion.EnableWindow(FALSE);
	m_nxeditCCDATo.EnableWindow(FALSE);
	m_nxeditCCDAMessage.EnableWindow(FALSE);
	GetDlgItem(IDC_DIRECT_MESSAGE_SEND_SUBJECT)->EnableWindow(FALSE); 

	GetDlgItem(IDC_DIRECTMESSAGE_SEND_FROM)->EnableWindow(FALSE); 
	GetDlgItem(IDC_DIRECT_MESSAGE_SEND_REFADDRESS)->EnableWindow(FALSE); 
	GetDlgItem(IDC_DIRECT_MESSAGE_ATTACHMENTS_SEND_LIST)->EnableWindow(FALSE); 	
	GetDlgItem(IDC_DIRECTMESSAGE_ATTACH_FILES)->EnableWindow(FALSE); 
}