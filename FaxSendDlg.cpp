// FaxSendDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "FaxSendDlg.h"
#include "WebFaxUtils.h"

//DRT 6/30/2008 - PLID 30541 - Created.

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFaxSendDlg dialog


CFaxSendDlg::CFaxSendDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFaxSendDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFaxSendDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_strBeginNumber = "";
	m_strBeginName = "";
	//(e.lally 2011-10-31) PLID 41195 - Initialize
	m_nPersonID = -1;
	m_nPicID = -1;
}


void CFaxSendDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFaxSendDlg)
	DDX_Control(pDX, IDC_FAX_USERID, m_editUser);
	DDX_Control(pDX, IDC_FAX_RECIP_NUMBER, m_editRecipNum);
	DDX_Control(pDX, IDC_FAX_RECIP_NAME, m_editRecipName);
	DDX_Control(pDX, IDC_FAX_PASSWORD, m_editPassword);
	DDX_Control(pDX, IDC_FAX_FROM_NAME, m_editFromName);
	DDX_Control(pDX, IDC_FAX_SUBJECT, m_editSubject);
	DDX_Control(pDX, IDC_FAX_DOC_REMOVE, m_btnRemove);
	DDX_Control(pDX, IDC_FAX_DOC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_FAX_DOC_DOWN, m_btnDown);
	DDX_Control(pDX, IDC_FAX_DOC_UP, m_btnUp);
	DDX_Control(pDX, IDC_SEND_FAX, m_btnSendFax);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFaxSendDlg, CNxDialog)
	//{{AFX_MSG_MAP(CFaxSendDlg)
	ON_BN_CLICKED(IDC_SEND_FAX, OnSendFax)
	ON_BN_CLICKED(IDC_FAX_DOC_ADD, OnFaxDocAdd)
	ON_BN_CLICKED(IDC_FAX_DOC_REMOVE, OnFaxDocRemove)
	ON_BN_CLICKED(IDC_FAX_DOC_UP, OnFaxDocUp)
	ON_BN_CLICKED(IDC_FAX_DOC_DOWN, OnFaxDocDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFaxSendDlg message handlers
//(e.lally 2011-10-31) PLID 41195 - Added PersonID and PicID for the mailSent log entry
int CFaxSendDlg::BeginSingleDocFax(CString strNumber, CString strName, CString strDocumentPath, long nPersonID/* = -1*/, long nPicID /*= -1*/)
{
	CStringArray ary;
	ary.Add(strDocumentPath);

	//Call the standard array version
	return BeginFax(strNumber, strName, ary, nPersonID, nPicID);
}

//(e.lally 2011-10-31) PLID 41195 - Added PersonID and PicID for the mailSent log entry
int CFaxSendDlg::BeginFax(CString strNumber, CString strName, CStringArray& aryDocumentPaths, long nPersonID/*=-1*/, long nPicID /*= -1*/)
{
	m_strBeginNumber = strNumber;
	m_strBeginName = strName;

	//(e.lally 2011-10-31) PLID 41195 - Set our member variables
	m_nPersonID = nPersonID;
	m_nPicID = nPicID;

	//copy the documents
	for(int i = 0; i < aryDocumentPaths.GetSize(); i++) {
		m_aryDocumentPaths.Add(aryDocumentPaths.GetAt(i));
	}

	return DoModal();
}

BOOL CFaxSendDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		//cache preferences
		//(e.lally 2011-10-31) PLID 41195
		g_propManager.BulkCache("FaxSend", propbitText | propbitNumber, 
		"Username IN('<None>', '%s') AND Name IN("
		"'MyFaxURL' "
		", 'MyFaxURI' "
		", 'SaveSentFaxInHistory' "
		")", _Q(GetCurrentUserName()) );

		//Bind datalists
		m_pServiceList = BindNxDataList2Ctrl(IDC_FAX_SERVICE_LIST, false);
		m_pResolutionList = BindNxDataList2Ctrl(IDC_FAX_RESOLUTION_LIST, false);
		m_pDocumentList = BindNxDataList2Ctrl(IDC_DOCUMENT_LIST, false);

		//Setup interface buttons
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnRemove.AutoSet(NXB_DELETE);
		m_btnUp.AutoSet(NXB_UP);
		m_btnDown.AutoSet(NXB_DOWN);

		// (z.manning 2009-09-29 13:05) - PLID 32472 - Subject has a max length of 50 characters
		m_editSubject.SetLimitText(50);

		//Default data might be passed in
		SetDlgItemText(IDC_FAX_RECIP_NUMBER, m_strBeginNumber);
		SetDlgItemText(IDC_FAX_RECIP_NAME, m_strBeginName);

		//load datalists
		LoadResolutions(m_pResolutionList);
		LoadDocuments();

		//The list of services will be filtered by the current user ID
		{
			m_pServiceList->WhereClause = _bstr_t(FormatString("UserID = %li", GetCurrentUserID()));
			m_pServiceList->Requery();
			m_pServiceList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		}

		//Auto select the first fax type
		m_pServiceList->PutCurSel(m_pServiceList->FindAbsoluteFirstRow(VARIANT_TRUE));
		LoadDefaultValues();


	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//Loads the default settings from the database for the currently selected fax type into the interface.
void CFaxSendDlg::LoadDefaultValues()
{
	//Get the current row and ensure it's valid
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pServiceList->GetCurSel();
	if(pRow == NULL) {
		return;
	}

	//Get the ID from the current row
	long nID = VarLong(pRow->GetValue(0));

	//Lookup its defaults
	CString strUser, strPass, strFrom, strResolution;

	_RecordsetPtr prs = CreateParamRecordset("SELECT Username, Password, FromName, Resolution FROM FaxConfigT  WHERE ID = {INT}", nID);
	if(!prs->eof) {
		FieldsPtr pFlds = prs->Fields;
		strUser = AdoFldString(pFlds, "Username");
		// (j.jones 2009-04-30 14:19) - PLID 34132 - used AES encryption on the password
		strPass = DecryptStringFromVariant(pFlds->Item["Password"]->Value);
		strFrom = AdoFldString(pFlds, "FromName");
		strResolution = AdoFldString(pFlds, "Resolution");
	}

	//Fill them to the interface
	SetDlgItemText(IDC_FAX_USERID, strUser);
	SetDlgItemText(IDC_FAX_PASSWORD, strPass);
	SetDlgItemText(IDC_FAX_FROM_NAME, strFrom);
	m_pResolutionList->SetSelByColumn(0, _bstr_t(strResolution));
}

void CFaxSendDlg::LoadDocuments()
{
	for(int i = 0; i < m_aryDocumentPaths.GetSize(); i++) {
		AddPathToDocumentList(m_aryDocumentPaths.GetAt(i));
	}
}

void CFaxSendDlg::AddPathToDocumentList(CString strPath)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDocumentList->GetNewRow();
	pRow->PutValue(0, _bstr_t(strPath));
	m_pDocumentList->AddRowAtEnd(pRow, NULL);
}

void CFaxSendDlg::OnOK() 
{
	OnSendFax();
}

void CFaxSendDlg::OnCancel() 
{
	try {
		CDialog::OnCancel();

	} NxCatchAll("Error in OnCancel");	
}

void CFaxSendDlg::OnSendFax()
{
	try {
		//
		//Get the service we are using to send the fax
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pServiceList->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("Please choose a service to fax with.");
			return;
		}

		//ID of the service
		eSupportedFaxServices esfsService = (eSupportedFaxServices)VarLong(pRow->GetValue(2));

		//Get all of our user-filled settings.  Format them into a settings struct that can be used by the
		//	faxing functionality.
		CFaxSettings settings;
		GetDlgItemText(IDC_FAX_USERID, settings.m_strUser);
		GetDlgItemText(IDC_FAX_PASSWORD, settings.m_strPassword);
		GetDlgItemText(IDC_FAX_FROM_NAME, settings.m_strFromName);
		GetDlgItemText(IDC_FAX_RESOLUTION_LIST, settings.m_strResolution);
		GetDlgItemText(IDC_FAX_RECIP_NUMBER, settings.m_strRecipNumber);
		GetDlgItemText(IDC_FAX_RECIP_NAME, settings.m_strRecipName);
		// (z.manning 2009-09-29 13:04) - PLID 32472 - Added subject
		GetDlgItemText(IDC_FAX_SUBJECT, settings.m_strCoverPageSubject);

		//Copy the documents
		NXDATALIST2Lib::IRowSettingsPtr pDocRow = m_pDocumentList->FindAbsoluteFirstRow(VARIANT_TRUE);
		while(pDocRow != NULL) {
			CString strPath = VarString(pDocRow->GetValue(0));

			//Ensure this path exists
			CFile f;
			if(!f.Open(strPath, CFile::modeRead|CFile::shareDenyNone)) {
				//Failed to load this file
				if(AfxMessageBox("The file '" + strPath + "' cannot be accessed.  Would you like to abort the fax?\r\n"
					"If you choose no, this file will be skipped.", MB_YESNO) != IDYES)
				{
					//skip the file, but continue
				}
				else {
					//Abort entirely
					return;
				}
			}
			else {
				f.Close();

				//Perfectly valid file, add it to our list
				settings.m_aryDocPaths.Add(strPath);
			}

			pDocRow = m_pDocumentList->FindAbsoluteNextRow(pDocRow, VARIANT_TRUE);
		}

		//Since we could have every file fail, make sure there's something valid 
		if(settings.m_aryDocPaths.GetSize() == 0) {
			//Just silently quit
			return;
		}

		//
		//DRT 6/27/2008 - PLID 30524
		//We now need to call the appropriate method.  Each fax service may have different requirements toward 
		//	sending the fax.  I've tried to make these setups as generic as possible so it's easy to add another, 
		//	but it's impossible to guess who or what might be required in the future.
		switch(esfsService) {
		case esfsMyFax:
			{
				CMyFaxSend send;
				//(e.lally 2011-10-31) PLID 41195 - Added PersonID and PicID for the mailSent log entry
				if(!send.SendFax(settings, m_nPersonID, m_nPicID)) {
					//The fax failed to send.  We should quit here, do not close the dialog.  They may have
					//	an invalid setting (like username or password), or have lost connection.
					return;
				}
			}
			break;

		default:
			AfxMessageBox("Invalid Fax Service Selected.");
			return;
		}

		//Dismiss the dialog once the fax has gone out
		CDialog::OnOK();

	} NxCatchAll("Error in OnSendFax");
}

BEGIN_EVENTSINK_MAP(CFaxSendDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CFaxSendDlg)
	ON_EVENT(CFaxSendDlg, IDC_FAX_SERVICE_LIST, 16 /* SelChosen */, OnSelChosenFaxServiceList, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CFaxSendDlg::OnSelChosenFaxServiceList(LPDISPATCH lpRow) 
{
	try {
		//The selection has changed, set the default settings values
		LoadDefaultValues();

	} NxCatchAll("Error in OnSelChosenFaxServiceList");
}

///////////////////////////////
//	Functions for modifying the document list
///////////////////////////////
void CFaxSendDlg::OnFaxDocAdd() 
{
	try {
		CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, "All Files (*.*)|*.*||", this);
		if(dlg.DoModal() == IDOK) {
			AddPathToDocumentList(dlg.GetPathName());
		}

	} NxCatchAll("Error in OnFaxDocAdd");
}

void CFaxSendDlg::OnFaxDocRemove() 
{
	try {
		//ensure we have a row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDocumentList->GetCurSel();
		if(pRow == NULL)
			return;

		//warn the user
		if(AfxMessageBox("Are you sure you wish to remove this document from the fax?", MB_YESNO) != IDYES) {
			return;
		}

		//Otherwise, remove the row from the list
		m_pDocumentList->RemoveRow(pRow);

	} NxCatchAll("Error in OnFaxDocRemove");
}

void CFaxSendDlg::OnFaxDocUp() 
{
	try {
		//Ensure there's a current row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDocumentList->GetCurSel();
		if(pRow == NULL) {
			return;
		}

		//Ensure we aren't the top row
		NXDATALIST2Lib::IRowSettingsPtr pRowPrev = pRow->GetPreviousRow();
		if(pRowPrev == NULL) {
			return;
		}

		//Ready to move.  Just re-insert
		NXDATALIST2Lib::IRowSettingsPtr pRowNew = NULL;
		pRowNew = m_pDocumentList->AddRowBefore(pRow, pRowPrev);

		//Remove the original
		m_pDocumentList->RemoveRow(pRow);

		//Set the new row as cursel
		m_pDocumentList->PutCurSel(pRowNew);

	} NxCatchAll("Error in OnFaxDocUp");
}

void CFaxSendDlg::OnFaxDocDown() 
{
	try {
		//Ensure there's a current row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDocumentList->GetCurSel();
		if(pRow == NULL) {
			return;
		}

		//Ensure we aren't the bottom row
		NXDATALIST2Lib::IRowSettingsPtr pRowNext = pRow->GetNextRow();
		if(pRowNext == NULL) {
			return;
		}

		//Ready to move.  There is no AddRowAfter, so we need to get the next row's next row.
		NXDATALIST2Lib::IRowSettingsPtr pRowNew = NULL;
		NXDATALIST2Lib::IRowSettingsPtr pRowTarget = pRowNext->GetNextRow();
		if(pRowTarget == NULL) {
			//That row is at the bottom, so just insert us AtEnd
			pRowNew = m_pDocumentList->AddRowAtEnd(pRow, NULL);
		}
		else {
			//Insert before the target
			pRowNew = m_pDocumentList->AddRowBefore(pRow, pRowTarget);
		}

		//Remove the original
		m_pDocumentList->RemoveRow(pRow);

		//Set the new row as cursel
		m_pDocumentList->PutCurSel(pRowNew);

	} NxCatchAll("Error in OnFaxDocDown");
}
