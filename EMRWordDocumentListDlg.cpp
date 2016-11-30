// EMRWordDocumentListDlg.cpp : implementation file
//
#pragma TODO("PLID 61808 - rename this dialog and its functions to better match its more vague use now.")

#include "stdafx.h"
#include "EMRWordDocumentListDlg.h"
#include "EmrRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum EMRWordListColumns {
	ewlcID = 0,
	ewlcDate,
	ewlcIcon,
	ewlcDescription,
	ewlcFileName,
};

using namespace NXDATALIST2Lib;

/////////////////////////////////////////////////////////////////////////////
// CEMRWordDocumentListDlg dialog


CEMRWordDocumentListDlg::CEMRWordDocumentListDlg(CWnd* pParent /*=NULL*/, EMRDocumentType edtType /*= edtWord*/)
: CNxDialog(CEMRWordDocumentListDlg::IDD, pParent), m_edtType(edtType)
{
	//{{AFX_DATA_INIT(CEMRWordDocumentListDlg)
		m_nPatientID = -1;
		m_nPICID = -1;
		m_nEMNID = -1;

		//(a.wilson 2014-5-15) PLID 61809
		m_hIconClinicalSummaryMerged = NULL;
		m_hIconClinicalSummaryNexWeb = NULL;
	//}}AFX_DATA_INIT
}


void CEMRWordDocumentListDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRWordDocumentListDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_EMR_DOCUMENT_LABEL, m_nxstaticEmrDocumentLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRWordDocumentListDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRWordDocumentListDlg)
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRWordDocumentListDlg message handlers

BOOL CEMRWordDocumentListDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-28 13:27) - PLID 29806 - NxIconified buttons
		m_btnCancel.AutoSet(NXB_CLOSE);
		
		m_List = BindNxDataList2Ctrl(this, IDC_EMR_WORD_DOCUMENT_LIST, GetRemoteData(), false);

		m_brush.CreateSolidBrush(PaletteColor(0x00CFE3BB));

		//(a.wilson 2014-5-15) PLID 61809 - load icons
		m_hIconClinicalSummaryMerged = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_CLIN_SUM_MERGED), IMAGE_ICON, 16, 16, 0);
		m_hIconClinicalSummaryNexWeb = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_CLIN_SUM_NEXWEB), IMAGE_ICON, 16, 16, 0);

		//(a.wilson 2014-5-15) PLID 61808 - depending on our enum this dialog can either be filled with word or clinical summary documents.
		switch (m_edtType)
		{
			default:
			case edtWord:
			{
				CString strWhere = "";
				SetWindowText("EMR Word Document List");
				if (m_nEMNID != -1) {
					strWhere.Format("Selection = 'BITMAP:MSWORD' AND EMNID = %li", m_nEMNID);
					SetDlgItemText(IDC_EMR_DOCUMENT_LABEL, "The following documents are attached to this EMN:");
				}
				else if (m_nPICID != -1) {
					strWhere.Format("Selection = 'BITMAP:MSWORD' AND PicID = %li", m_nPICID);
					SetDlgItemText(IDC_EMR_DOCUMENT_LABEL, "The following documents are attached to this EMR:");
				}
				else {
					strWhere.Format("Selection = 'BITMAP:MSWORD' AND PersonID = %li", m_nPatientID);
					SetDlgItemText(IDC_EMR_DOCUMENT_LABEL, "The following documents are attached to this Patient:");
				}
				m_List->PutFromClause(FormatBstr(
					"( "
					"SELECT MailSent.MailID, MailSent.Date, MailSent.Selection, MailSent.EMNID, MailSent.PicID, "
					"MailSent.PersonID, MailSent.[PathName], MailSentNotesT.Note, NULL AS Icon "
					"FROM MailSent "
					"LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
					") SubQ"));
				m_List->PutWhereClause(_bstr_t(strWhere));
				break;
			}
			case edtClinicalSummary:
			{
				//(a.wilson 2014-5-15) PLID 61809 - handle icon display in requery
				IColumnSettingsPtr pColumn = m_List->GetColumn(ewlcIcon);
				pColumn->PutStoredWidth(25);
				SetWindowText("EMR Clinical Summary Document List");
				m_List->PutFromClause(FormatBstr(
					"( "
					"SELECT		MailSent.MailID, MailSent.Date, MailSentNotesT.Note, MailSent.[PathName], "
					"CASE WHEN	COALESCE(AccessHistoryQ.AccessType, 0) IN (-1,-2,-3) THEN %li ELSE %li END AS Icon "
					"FROM		MailSent "
					"LEFT JOIN	MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
					"LEFT JOIN	(SELECT MailSentMailID, MAX(AccessType) AS AccessType FROM NexWebCcdaAccessHistoryT GROUP BY MailSentMailID) AccessHistoryQ ON MailSent.MailID = AccessHistoryQ.MailSentMailID "
					"WHERE		MailSent.Selection = 'BITMAP:CCDA' AND MailSent.CCDAtypeField = 2 AND MailSent.EMNID = %li "
					") SubQ ", (long)m_hIconClinicalSummaryNexWeb, (long)m_hIconClinicalSummaryMerged, m_nEMNID));
				m_List->PutWhereClause(_bstr_t(""));
				SetDlgItemText(IDC_EMR_DOCUMENT_LABEL, "The following Clinical Summary Documents are attached to this EMN:");
				break;
			}
		}
		m_List->Requery();
	}
	NxCatchAll("Error in CEMRWordDocumentListDlg::OnInitDialog");
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRWordDocumentListDlg::OnOK() 
{
	//(a.wilson 2014-5-15) PLID 61808 - call a single function.
	try {
		OpenSelectedDocument();

	}NxCatchAll("Error in CEMRWordDocumentListDlg::OnOK.");
}

BEGIN_EVENTSINK_MAP(CEMRWordDocumentListDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRWordDocumentListDlg)
	ON_EVENT(CEMRWordDocumentListDlg, IDC_EMR_WORD_DOCUMENT_LIST, 3 /* DblClickCell */, OnDblClickCellEmrWordDocumentList, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEMRWordDocumentListDlg::OnDblClickCellEmrWordDocumentList(LPDISPATCH lpRow, short nColIndex) 
{
	//(a.wilson 2014-5-15) PLID 61808 - call a single function.
	try {
		OpenSelectedDocument();

	}NxCatchAll("Error in CEMRWordDocumentListDlg::OnDblClickCellEmrWordDocumentList");
}

BOOL CEMRWordDocumentListDlg::OpenWordDocument(CString strFileName)
{
	try {

		//we should be only filtered on word documents!
		// (a.walling 2007-07-19 10:18) - PLID 26748 - Support Word 2007 documents
		// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
		if( (strnicmp(strFileName.Right(4), ".doc", 4) != 0) || (strnicmp(strFileName.Right(5), ".docx", 5) != 0) || (strnicmp(strFileName.Right(5), ".docm", 5) != 0)) {
			//it does NOT end in .doc!  Oh dear.
			//for 1 last check, make sure there is NO extension on it.
			if(GetFileExtension(strFileName).IsEmpty()) {
				//there is no extension
				CString strMsg;
				strMsg.Format("Practice has detected your Word document does not end in the standard .doc or .docx/.docm extension.  Would you "
					"like to fix this in your data?");
				if(MsgBox(MB_YESNO, strMsg) == IDYES) {
					//they want to fix it manually.  We know that we have a filename w/o an extension, like
					//"MultiPatDoc00000", and we want to add .doc to that.
					try {
						// (a.walling 2007-07-19 10:50) - PLID 26748 - Need to find out whether we should rename to .doc or .docx
						CString strFullPath, strExtension;
						if (strFileName.Find('\\') == -1) {
							// The "path" doesn't have a backslash, so it's just a filename, which means it should use patient's shared documents path
							strFullPath = GetPatientDocumentPath(m_nPatientID) ^ strFileName;
						}
						CFile fDoc;
						if (fDoc.Open(strFullPath, CFile::modeRead | CFile::shareCompat)) {
							char magic[2];

							DWORD dwRead = fDoc.Read(magic, 2);
							fDoc.Close();
							if (dwRead < 2) {
								ThrowNxException("File %s has an invalid length", strFileName);
							} else {
								if (memcmp(magic, "PK", 2) == 0) {
									// (a.walling 2007-10-12 14:27) - PLID 26342 - There is no easy way to know if this is a docx or docm, so just rename
									// to docx by default.
									strExtension = ".docx";
								} else {
									strExtension = ".doc";
								}
							}
						} else {
							// if we can't open it, not much we can do, so let's just exit since we can't open the document anyway.
							ThrowNxException("Could not access %s: %s", strFileName, FormatLastError());
						}

						// (j.jones 2011-07-22 15:41) - PLID 21784 - get a list of the MailIDs affected (rarely more than one)
						CArray<long, long> aryMailIDs;
						CMap<long, long, long, long> mapMailIDToPatientID;
						// (j.jones 2014-08-04 11:37) - PLID 63141 - this now sends an Ex tablechecker which needs more information
						ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID, MailID FROM MailSent WHERE PathName = {STRING}", strFileName);
						
						while(!rs->eof) {
							long nMailID = VarLong(rs->Fields->Item["MailID"]->Value);
							aryMailIDs.Add(nMailID);
							// (j.jones 2014-08-04 11:49) - PLID 63141 - track the person ID
							mapMailIDToPatientID.SetAt(nMailID, AdoFldLong(rs, "PersonID"));

							rs->MoveNext();
						}
						rs->Close();

						ExecuteParamSql("UPDATE MailSent SET PathName = {STRING} WHERE MailID IN ({INTARRAY})", strFileName + strExtension, aryMailIDs);

						// (j.jones 2011-07-22 15:43) - PLID 21784 - send a tablechecker for each file changed, usually it is only one
						// (j.jones 2014-08-04 11:37) - PLID 63141 - this now sends an Ex tablechecker, we know IsPhoto is always false here
						for (int i = 0; i<aryMailIDs.GetSize(); i++) {
							long nMailID = aryMailIDs.GetAt(i);
							long nPatientID = -1;
							mapMailIDToPatientID.Lookup(nMailID, nPatientID);
							CClient::RefreshMailSentTable(nPatientID, nMailID);
						}

						strFileName += strExtension;	//update it so we can open the file
					} NxCatchAll("Error updating MailSent with .doc[xm] extension.");
				}
				else {
					//they don't want to fix it, oh well, we tried
				}
			}
			else {
				//there is some kind of extension here, so we'll just let this go
			}
		}

		if(!OpenDocument(strFileName, m_nPatientID)) {
			MsgBox(RCS(IDS_NO_FILE_OPEN));
			return FALSE;
		}

		return TRUE;

	}NxCatchAll("Error loading word document.");

	return FALSE;
}

//(a.wilson 2014-5-15) PLID 61808 - handles opening word or clinical summary documents from open button or double clicking.
void CEMRWordDocumentListDlg::OpenSelectedDocument()
{
	IRowSettingsPtr pRow(m_List->GetCurSel());

	if (!pRow) {
		AfxMessageBox("Please select a document from the list.");
		return;
	}

	CString strFileName = VarString(pRow->GetValue(ewlcFileName),"");

	switch (m_edtType)
	{
	default:
	case edtWord:
		if (OpenWordDocument(strFileName)) {
			CDialog::OnOK();
		}
		break;
	case edtClinicalSummary:
		CString strFullPath = (GetPatientDocumentPath(m_nPatientID) ^ strFileName);
		ViewXMLDocument(strFullPath, m_pParentWnd);
		CDialog::OnOK();
		break;
	}
}

//(a.wilson 2014-5-15) PLID 61809 - clean up icons as dialog closes.
void CEMRWordDocumentListDlg::OnDestroy()
{
	try {
		DestroyIcon(m_hIconClinicalSummaryMerged);
		DestroyIcon(m_hIconClinicalSummaryNexWeb);
		m_hIconClinicalSummaryMerged = NULL;
		m_hIconClinicalSummaryNexWeb = NULL;
	} NxCatchAll(__FUNCTION__);

	CNxDialog::OnDestroy();
}
