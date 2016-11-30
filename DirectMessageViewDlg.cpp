// DirectMessageViewDlg.cpp : implementation file
//
// (j.camacho 2013-10-21 18:07) - PLID 59112

#include "stdafx.h"
#include "Practice.h"
#include "DirectMessageViewDlg.h"
#include "NxAPI.h"
#include <iostream>
#include <fstream>
#include "FinancialRC.h"
#include "NxXMLUtils.h"
#include "SingleSelectMultiColumnDlg.h"
#include "NxSystemUtilitiesLib\FileUtils.h"


using namespace NXDATALIST2Lib;


// (b.spivey - November 5, 2013) - PLID 59336 - 
enum DirectMessageAttachmentList {

	dmalFileName = 0,
	dmalView,
	dmalImport,
};

//The list of patients enums
enum DirectMessagePatientImportList {

	dmpilPersonID = 0,
	dmpilUserDefinedID,
	dmpilPatientLast,
	dmpilPatientFirst,
	dmpilPatientMiddle, 
};

IMPLEMENT_DYNAMIC(CDirectMessageViewDlg, CNxDialog)

CDirectMessageViewDlg::CDirectMessageViewDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CDirectMessageViewDlg::IDD, pParent),
	m_tempDirectory(new FileUtils::CAutoRemoveTempDirectory(""))
{
	// (j.camacho 2013-11-07 12:28) - PLID 59303
	//initialize delete flag, default false
	m_bDeleted = false;
	m_bErrorOnLoading = false; // (j.camacho 2013-12-23 12:09) - PLID 59112
	m_bCloseUnread = false; // (d.singleton 2014-05-16 08:57) - PLID 62173 - add option to close message and mark as Unread.  in case they open the wrong message. 
	}

CDirectMessageViewDlg::~CDirectMessageViewDlg()
{
}

void CDirectMessageViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DIRECTMESSAGE_VIEW_FROM,m_nxeditFrom);
	DDX_Control(pDX, IDC_DIRECTMESSAGE_VIEW_TO,m_nxeditTo);
	DDX_Control(pDX, IDC_DIRECTMESSAGE_VIEW_SUBJECT,m_nxeditSubject);
	DDX_Control(pDX, IDC_DIRECTMESSAGE_DELETE, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_DIRECTMESSAGE_UNREAD, m_btnCloseUnread);
}


BEGIN_MESSAGE_MAP(CDirectMessageViewDlg, CNxDialog)
	ON_BN_CLICKED(IDCANCEL, &CDirectMessageViewDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_DIRECTMESSAGE_DELETE, &CDirectMessageViewDlg::OnBnClickedDirectmessageDelete)
	ON_BN_CLICKED(IDC_DIRECTMESSAGE_UNREAD, &CDirectMessageViewDlg::OnBnClickedDirectmessageUnread)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CDirectMessageViewDlg, CNxDialog)
	ON_EVENT(CDirectMessageViewDlg, IDC_DIRECT_MESSAGE_ATTACHMENTS_LIST, 19, CDirectMessageViewDlg::LeftClickDirectMessageAttachments, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// CDirectMessageViewDlg message handlers
// (j.camacho 2013-10-22 09:51) - PLID 59112 - Loads up the info from the API about the requested message
BOOL CDirectMessageViewDlg::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog();
		//initialize the datafields
		NexTech_Accessor::_DirectMessageContentsPtr pMessage;
		try
		{
			pMessage = GetAPI()->GetDirectMessageByID(GetAPISubkey(), GetAPILoginToken(), m_lDirectMessageID, m_nDirectMessageSize, _bstr_t(m_strEmailID));
		}
		catch(_com_error e)
		{
			//error caught on load, log and close dialog.
			Log(e.Description());
			m_bErrorOnLoading = true;
			CNxDialog::OnCancel();
			return false;
		}NxCatchAll(__FUNCTION__);
		//SetDlgItemText(IDC_DIRECTMESSAGE_VIEW_BODYTEXT,pMessage->GetBodyText());		//Body message
		// (j.camacho 2013-10-22 12:24) - PLID 59139
		CWnd* pBrowserWnd = GetDlgItem(IDC_DIRECTMESSAGE_EXPLORER);
		//m_pBrowser = GetDlgItem(IDC_DIRECTMESSAGE_EXPLORER)->GetControlUnknown();
		_bstr_t htmlBody;
		if( VarString(pMessage->BodyHTML,"") !="")
		{
			htmlBody = pMessage->BodyHTML;
		}
		else
		{
			htmlBody = pMessage->BodyText;
		}
		//b.spivey (November 19th, 2013) - PLID 59336 - Create a temp directory to hold these files. This prevents collision.
		// (j.camacho 2013-12-13 11:10) - PLID 59139 - use same temp folder as the attachment files will.
		m_strTempDirectory = GetPracPath(PracPath::SessionPath) 
			^ ("DirectMessage_" + NewPlainUUID()); 
		
		FileUtils::CreatePath(m_strTempDirectory);
		m_tempDirectory.reset(new FileUtils::CAutoRemoveTempDirectory(m_strTempDirectory));	
		
		// (j.camacho 2013-12-13 14:08) - PLID 59139 - Save html to browse to it. 
		//There is some possibility about converting this to read html from memory rather than making a new file.
		CString strHTMLPath = m_strTempDirectory;
		strHTMLPath.Replace("\\","/");
		strHTMLPath.Append("/message.html");
		std::ofstream myfile;
		myfile.open (strHTMLPath);
		myfile << htmlBody ;
		myfile.close();
		
		m_pBrowser = pBrowserWnd->GetControlUnknown();
		m_pBrowser->Navigate(_bstr_t(strHTMLPath), NULL, NULL, NULL, NULL);
		
		CString subjectLine = (LPCTSTR) pMessage->GetSecureSubject();
		if(subjectLine != "")
		{
			SetDlgItemText(IDC_DIRECTMESSAGE_VIEW_SUBJECT,pMessage->GetSecureSubject());	//'Sensitive' subject line
		}
		else
		{
			SetDlgItemText(IDC_DIRECTMESSAGE_VIEW_SUBJECT,pMessage->GetPublicSubject()); //Public subject line
		}
		SetDlgItemText(IDC_DIRECTMESSAGE_VIEW_FROM,pMessage->GetDirectFromAddress());	//From address

		//the to addresses are stored in a safe array and must be iterated through
		Nx::SafeArray<BSTR> saToAddress(pMessage->ToAddress);
		CString m_strAddresses;
		for(ULONG i=0; i<saToAddress.GetLength();i++)
		{
			m_strAddresses += saToAddress.GetAt(i);
			m_strAddresses +="; ";
		}
		SetDlgItemText(IDC_DIRECTMESSAGE_VIEW_TO,m_strAddresses); //To addresses


		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnCloseUnread.AutoSet(NXB_MODIFY);

		// (b.spivey - November 5, 2013) - PLID 59336 - bind the list, get the list of attachments, then load it. 
		m_dlAttachments = BindNxDataList2Ctrl(IDC_DIRECT_MESSAGE_ATTACHMENTS_LIST, false); 

		Nx::SafeArray<BSTR> saAttachments(pMessage->DirectAttachments);
		CArray<CString, CString> aryAttachments;
		//for every BSTR in the safe array, add it to the CArray. 
		foreach (BSTR str, saAttachments) {
			aryAttachments.Add(VarString(str)); 
		}

		//Load the final list.
		LoadAttachmentsIntoList(aryAttachments); 
		
	}NxCatchAll(__FUNCTION__);
	return true;
}

void CDirectMessageViewDlg::OnOK()
{
	try{
		// (b.spivey - December 6, 2013) - PLID 59336 - Clean up files. 
		CleanUpTempFiles(); 
		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
	
}

// (j.camacho 2013-10-21 18:07) - PLID 59112 - Sets message information for pulling in data from the API
void CDirectMessageViewDlg::SetMessageID(__int64 id,CString email)
{
	try{
		m_lDirectMessageID = id;
		m_strEmailID = email;
	}NxCatchAll(__FUNCTION__);
}

void CDirectMessageViewDlg::OnBnClickedCancel()
{
	try
	{
		// (b.spivey - December 6, 2013) - PLID 59336 - Clean up files. 
		CleanUpTempFiles(); 
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__)
}

void CDirectMessageViewDlg::OnBnClickedDirectmessageDelete()
{
	try
	{
		__int64 messageid = m_lDirectMessageID;
		CString emailid = m_strEmailID; //change email
		NexTech_Accessor::_DirectMessageDeleteResultPtr msgDel;
		msgDel = GetAPI()->DeleteDirectMessageByID(GetAPISubkey(), GetAPILoginToken(), messageid,_bstr_t(emailid));
		//mark that the itme was deleted
		m_bDeleted = true;
		OnOK();//Successful close of dialog
	}NxCatchAll(__FUNCTION__);
}

void CDirectMessageViewDlg::SetMessageSize(__int64 nMessageSize) 
{
	m_nDirectMessageSize = nMessageSize; 
}

// (b.spivey - November 5, 2013) - PLID 59336 - Load list with attachments
void CDirectMessageViewDlg::LoadAttachmentsIntoList(const CArray<CString, CString>& aryAttachments) 
{
	foreach(CString str, aryAttachments) {
		IRowSettingsPtr pRow = m_dlAttachments->GetNewRow();
		pRow->PutValue(dmalFileName, _bstr_t(str));
		pRow->PutValue(dmalView, "View");
		pRow->PutValue(dmalImport, "Import"); 
		m_dlAttachments->AddRowAtEnd(pRow, NULL); 
	}
}

// (b.spivey - December 6, 2013) - PLID 59336 - Clean up files. 
void CDirectMessageViewDlg::CleanUpTempFiles() 
{
	IRowSettingsPtr pRow = m_dlAttachments->GetFirstRow(); 

	//if null, then this won't get hit. 
	while (pRow) {
		CString strFileName = VarString(pRow->GetValue(dmalFileName), "");
		strFileName = m_strTempDirectory ^ strFileName;
		if(!FileUtils::DoesFileOrDirExist(strFileName)) {
			pRow = pRow->GetNextRow(); 
			continue; 
		}
		FileUtils::CAutoRemoveTempFile tempFile(strFileName);
		pRow = pRow->GetNextRow(); 
	}

}

// (b.spivey - November 5, 2013) - PLID 59336 - When you click the datalist, you should be able to view the file. 
void CDirectMessageViewDlg::LeftClickDirectMessageAttachments(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		//if null, return early.
		if(!pRow) {
			return;
		}

		//If they clicked the view column. 
		if(pRow && nCol == dmalView) {
			//Use the file name to get the attachment pointer.
			CString strFileName = VarString(pRow->GetValue(dmalFileName), "");
			NexTech_Accessor::_DirectMessageAttachmentPtr dmap = GetAPI()->GetDirectMessageAttachmentByIDName(GetAPISubkey(), GetAPILoginToken(), m_lDirectMessageID, m_nDirectMessageSize, _bstr_t(m_strEmailID), _bstr_t(strFileName)); 
			//Set this from the value in the API. Just a good idea.
			// (e.frazier 2016-04-26 14:37) - PLID 63575 - Replace invalid filename characters before loading the attachment data
			strFileName = FileUtils::MakeValidFileName(VarString(dmap->AttachmentFileName, ""));
			//safe array of btes.
			Nx::SafeArray<BYTE> saAttachment = dmap->AttachmentFileData;

			//If we have a file, lets proceed to create it and try to open it.
			if (saAttachment.GetLength() > 0) {
				
				strFileName = m_strTempDirectory ^ strFileName;
				if(!FileUtils::DoesFileOrDirExist(strFileName)) {
					_variant_t vt;
					vt = saAttachment.AsVariant(); 
					FileUtils::CreateFileFromSafeArrayOfBytes(vt, strFileName); 
				}

				// (b.spivey - November 5, 2013) - PLID 59336 - we have an xml viewer, and we need this for CCDAs anyways.
				if(FileUtils::GetFileExtension(strFileName).CompareNoCase("xml") == 0) {
					ViewXMLDocument(strFileName, this); 
					return; 
				}

				//Open this file because we're viewing. 
				if ((int)ShellExecute(GetSafeHwnd(), NULL, strFileName, NULL, GetFilePath(strFileName), SW_NORMAL) < 32)
				{
					MsgBox("Could not open attachment.");
					return;
				}
			}
			// (b.spivey, January 6, 2014) - PLID 59182 - Cleaned these up because of changes to the API. 
			else {
				MsgBox(MB_ICONWARNING|MB_OK, "Cannot retrieve data for this attachment."); 
				return; 
			}
		}
		//b.spivey (November 19th, 2013) PLID 59336 - All of this handles importing on a per-file basis.
		else if(pRow && nCol == dmalImport) {

			//Make sure we have attachments a patient selected.
			// (e.frazier 2016-04-26 14:37) - PLID 63575 - Replace invalid filename characters before loading the attachment data
			CString strFileName = VarString(pRow->GetValue(dmalFileName), "");
			CString strFilePath = m_strTempDirectory ^ FileUtils::MakeValidFileName(strFileName);

			//Make sure it's not there to begin with.
			if(!FileUtils::DoesFileOrDirExist(strFilePath)) {
				
				NexTech_Accessor::_DirectMessageAttachmentPtr dmap = GetAPI()->GetDirectMessageAttachmentByIDName(GetAPISubkey(), GetAPILoginToken(), m_lDirectMessageID, m_nDirectMessageSize, _bstr_t(m_strEmailID), _bstr_t(strFileName));

				//safe array of btes.
				Nx::SafeArray<BYTE> saAttachment = dmap->AttachmentFileData;

				//If we have a file, lets proceed to create it and try to open it.
				if (saAttachment.GetLength() > 0) {
					_variant_t vt;
					vt = saAttachment.AsVariant(); 
					FileUtils::CreateFileFromSafeArrayOfBytes(vt, strFilePath); 
				}
				// (b.spivey, January 6, 2014) - PLID 59182 - Cleaned these up because of changes to the API. 
				else {
					MsgBox(MB_ICONWARNING|MB_OK, "Cannot retrieve data for this attachment."); 
					return; 
				}
			} 
		
			//If this is a CCDA, then we use the normal import logic to make sure all flags are set correctly.
			if(!!NxXMLUtils::IsCCDAFile(strFilePath)) {
				//Check if we failed. 
				if(!!ImportAndAttachFileToHistory(strFilePath, -25, GetSafeHwnd()) == false) {
					CString strWarning;
					strWarning.Format("Unable to attach file to history: \r\n\r\n%s", strFilePath);
					MessageBox(strWarning, "Unable to attach file to history", MB_ICONEXCLAMATION);
				}
				//Success!
				else {
					MessageBox("File successfully imported!", "Import successful", MB_OK);
				}
			}
			//Anything else, select the patient from a list. 
			else {
				//1. set up the fields we are interested in having
				CStringArray aryColumns;
				aryColumns.Add("PersonT.ID");
				aryColumns.Add("PersonT.Last");
				aryColumns.Add("PersonT.First");
				aryColumns.Add("PersonT.Middle");
				aryColumns.Add("PatientsT.UserDefinedID");

				//2. Column Headers (order matters)
				CStringArray aryColumnHeaders;
				aryColumnHeaders.Add("PersonID");
				aryColumnHeaders.Add("Last Name");
				aryColumnHeaders.Add("First Name");
				aryColumnHeaders.Add("Middle Name");
				aryColumnHeaders.Add("Patient ID");

				//3. Sort order for columns (order matters)
				CSimpleArray<short> arySortOrder;
				arySortOrder.Add(-1); 
				arySortOrder.Add(0);
				arySortOrder.Add(1);
				arySortOrder.Add(2);
				arySortOrder.Add(3);


				//Open the dialog and let the user pick a patient. 
				CSingleSelectMultiColumnDlg dlg(this); 
				//(b.spivey, December 13th, 2013) - PLID 59336 - Exclude inquiries. 
				HRESULT hr = dlg.Open("PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID ", 
					"PersonT.Archived = 0 AND PersonT.ID > 0 AND CurrentStatus <> 4 ", 
					aryColumns,
					aryColumnHeaders,
					arySortOrder,
					"[1], [2] [3] ([4])", 
					"Please select a patient to attach this document to.",
					"Select a Patient");

				//They clicked OK, lets try to do some stuff. 
				if(hr == IDOK) {
					CVariantArray varySelectedValues;
					dlg.GetSelectedValues(varySelectedValues);

					if(!varySelectedValues.GetSize()){
						MessageBox("You did not select a valid patient. Please try again. ", "No Patient Selected", MB_OK | MB_ICONEXCLAMATION);
						return;
					}

					long nPatientID = VarLong(varySelectedValues.GetAt(0), -1);

					//I can only assume if it's 0 or less it's a failure, so lets go with that mentality. 
					if(!!ImportAndAttachFileToHistory(strFilePath, nPatientID, GetSafeHwnd()) == false) {
						CString strWarning;
						strWarning.Format("Unable to attach file to history: \r\n\r\n%s", strFilePath);
						MessageBox(strWarning, "Unable to attach file to history", MB_ICONEXCLAMATION);
					}
					//Let the user know they successfully attached a file. 
					else {
						MessageBox("File successfully imported!", "Import successful", MB_OK);
					}
				}
			}						
		}
	}NxCatchAll(__FUNCTION__);
}


bool CDirectMessageViewDlg::CheckDelete()
{
	return m_bDeleted;
}

// (d.singleton 2014-05-16 08:36) - PLID 62173 - add option to close message and mark as Unread.  in case they open the wrong message. 
void CDirectMessageViewDlg::OnBnClickedDirectmessageUnread()
{
	try {
		m_bCloseUnread = true;
		OnOK();
	}NxCatchAll(__FUNCTION__);
}

bool CDirectMessageViewDlg::CheckUnread()
{
	return m_bCloseUnread;
}
