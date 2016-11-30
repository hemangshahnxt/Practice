// ContactsNotes.cpp : implementation file
#include "stdafx.h"
#include "practice.h"
#include "ContactsNotes.h"
#include "MainFrm.h"
#include "ContactView.h"
#include "GlobalDataUtils.h"
#include "NoteCategories.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_ADDNOTE	42305
#define ID_PRINT	42306
#define ID_DELETE	42307

// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CContactsNotes dialog

CContactsNotes::CContactsNotes(CWnd* pParent)
	: CNxDialog(CContactsNotes::IDD, pParent)
{
	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Contacts_Module/print_notes_for_a_single_contact.htm";
	//{{AFX_DATA_INIT(CContactsNotes)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CContactsNotes::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CContactsNotes)
	DDX_Control(pDX, IDC_CONTACTS_NOTES_SHOW_GRID, m_btnShowGridlines);
	DDX_Control(pDX, IDC_EDIT, m_editButton);
	DDX_Control(pDX, IDC_DELETE, m_deleteButton);
	DDX_Control(pDX, IDC_ADD, m_addButton);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CContactsNotes, CNxDialog)
	//{{AFX_MSG_MAP(CContactsNotes)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_BN_CLICKED(IDC_CONTACTS_NOTES_SHOW_GRID, OnContactsNotesShowGrid)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CContactsNotes, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CContactsNotes)
	ON_EVENT(CContactsNotes, IDC_LIST, 6 /* RButtonDown */, OnRButtonDownList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CContactsNotes, IDC_LIST, 9 /* EditingFinishing */, OnEditingFinishingList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CContactsNotes, IDC_LIST, 10 /* EditingFinished */, OnEditingFinishedList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CContactsNotes, IDC_LIST, 17 /* ColumnClicking */, OnColumnClickingList, VTS_I2 VTS_PBOOL)
	ON_EVENT(CContactsNotes, IDC_LIST, 8 /* EditingStarting */, OnEditingStartingList, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CContactsNotes, IDC_LIST, 21 /* EditingStarted */, OnEditingStartedList, VTS_I4 VTS_I2 VTS_I4)
	ON_EVENT(CContactsNotes, IDC_LIST, 2 /* SelChanged */, OnSelChangedList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CContactsNotes message handlers

BOOL CContactsNotes::OnInitDialog() 
{
	m_addButton.AutoSet(NXB_NEW);
	m_editButton.AutoSet(NXB_MODIFY);
	m_deleteButton.AutoSet(NXB_DELETE);

	EnsureRemoteData();
	m_list = BindNxDataListCtrl(IDC_LIST,false);
	CNxDialog::OnInitDialog();
	m_id = GetActiveContactID();
	SecureControls();
	EnableAppropriateButtons();

	if(GetRemotePropertyInt("ContactsNotesShowGrid", 0, 0, GetCurrentUserName(), true)) {
		CheckDlgButton(IDC_CONTACTS_NOTES_SHOW_GRID, BST_CHECKED);
		m_list->GridVisible = true;
	}
	UpdateView();
	return TRUE;
}

void CContactsNotes::RecallDetails()
{
//	UpdateView();
}

void CContactsNotes::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	m_id = GetActiveContactID();
	Requery (m_id);
	EnableAppropriateButtons();
}

void CContactsNotes::Requery(long id)
{
	CString strSQL;

	strSQL.Format ("PersonID = %i", id);
	if (m_list->WhereClause != _bstr_t(strSQL))
	{	m_list->WhereClause = _bstr_t(strSQL);
		m_list->Requery();
	}
	else m_list->Requery();
}

void CContactsNotes::Add() 
{
	try {

		if (!CheckCurrentUserPermissions(bioContactsNotes, sptCreate))
		return;

		CString strNote;
		strNote = CString(GetCurrentUserName()) + " - ";

		// (z.manning, 03/05/2007) - PLID 25065 - Pull the note's time from the server.
		// (a.walling 2010-11-01 11:07) - PLID 40965 - Parameterized
		// (j.armen 2014-01-31 09:31) - PLID 60568 - Idenitate NoteDataT
		_RecordsetPtr rsID = CreateParamRecordset(
			"SET NOCOUNT ON "
			"DECLARE @nNewID int "
			"DECLARE @dtCurrent datetime "
			"SET @dtCurrent = GetDate() "
			"INSERT INTO Notes (PersonID, Date, UserID, Note) VALUES ({INT}, @dtCurrent, {INT}, {STRING})\r\n"
			"SET @nNewID = SCOPE_IDENTITY()\r\n"
			"SET NOCOUNT OFF "
			"SELECT @nNewID, @dtCurrent AS CurrentDate ",
			m_id, GetCurrentUserID(), strNote);

		long ID = VarLong(rsID->Fields->GetItem((long)0)->Value);
		_variant_t vtCurrentDate = rsID->Fields->GetItem("CurrentDate")->Value;

		//This will either return exactly one row, or throw an exception
		IRowSettingsPtr pRow;
		pRow = m_list->GetRow(-1);
		pRow->PutValue(0,(long)ID);
		pRow->PutValue(1,(long)m_id);
		pRow->PutValue(2, vtCurrentDate);
		pRow->PutValue(3, vtCurrentDate);
		pRow->PutValue(5, _variant_t(strNote));
		m_list->InsertRow(pRow, -1);

		m_list->SetSelByColumn(0, VarLong(ID));

		m_list->StartEditing(m_list->CurSel,5);

		EnableAppropriateButtons();
		
	}NxCatchAll("Error in adding new note.");
}

void CContactsNotes::Delete() 
{
	try {
		long nCurSel = m_list->GetCurSel();
		if (nCurSel == -1) {
			MsgBox("Please Select a note");
			return;
		}

		if (!CheckCurrentUserPermissions(bioContactsNotes, sptDelete))
			return;

		//for audit
		CString strOld;
		strOld = CString(m_list->GetValue(nCurSel, 5).bstrVal);

		if(MessageBox("Are you sure you wish to delete this note?", "Delete?", MB_YESNO)==IDNO)
			return;

		// (a.walling 2010-08-02 12:11) - PLID 39867 - Moved Notes metadata to NoteInfoT
		ExecuteSql("DELETE FROM NoteDataT WHERE ID = %li",VarLong(m_list->GetValue(nCurSel,0)));

		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, GetActiveContactName(), nAuditID, aeiContactNoteDelete, GetActiveContactID(), strOld, "<Deleted>", aepMedium, aetDeleted);
		m_list->RemoveRow(nCurSel);

		EnableAppropriateButtons();
		
	}NxCatchAll("Error in CContactsNotes::Delete()");
	
}

BOOL CContactsNotes::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (wParam) {
	case ID_DELETE:
		if (m_iRow!=-1) {
			//!!
			Delete();
		}
			break;
		case ID_ADDNOTE:
			Add();
			break;
		case ID_PRINT:
			CContactView *tmpView = (CContactView*)(GetMainFrame()->GetOpenView("Contacts"));
			tmpView->OnPrint();
			break;
	}
	return CNxDialog::OnCommand(wParam, lParam);
}

void CContactsNotes::OnRButtonDownList(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_list->PutCurSel(nRow);
	CMenu pMenu;
	m_iRow = nRow;

	bool bAllowAddNote = true;
	if(!(GetCurrentUserPermissions(bioContactsNotes) & (sptCreate|sptCreateWithPass)))
	{
		bAllowAddNote = false;
	}
	else
	{
		bAllowAddNote = true;
	}

	bool bAllowDeleteNote = true;
	if(!(GetCurrentUserPermissions(bioContactsNotes) & (sptDelete|sptDeleteWithPass)))
	{
		bAllowDeleteNote = false;
	}
	else
	{
		bAllowDeleteNote = true;
	}
	
	if (nRow == -1)
	{	pMenu.CreatePopupMenu();
		pMenu.InsertMenu(1, MF_BYPOSITION|(bAllowAddNote ? 0:MF_GRAYED), ID_ADDNOTE, "Add Note");
		pMenu.InsertMenu(2, MF_BYPOSITION, ID_PRINT, "Print Notes");
	}
	else
	{	
		// Build a menu popup with the ability to delete the current row
		pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION|(bAllowDeleteNote ? 0:MF_GRAYED), ID_DELETE, "Delete Note");
		pMenu.InsertMenu(1, MF_BYPOSITION|(bAllowAddNote ? 0:MF_GRAYED), ID_ADDNOTE, "Add Note");
		pMenu.InsertMenu(2, MF_BYPOSITION, ID_PRINT, "Print Notes");
	}
	CPoint pt;
	pt.x = x;
	pt.y = y;
	CWnd* dlNotes = GetDlgItem(IDC_LIST);
	if (dlNotes != NULL) {
		dlNotes->ClientToScreen(&pt);
		pMenu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
	}
	else {
		HandleException(NULL, "An error ocurred while creating menu");
	}

	EnableAppropriateButtons();
}

void CContactsNotes::OnEditingFinishingList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	if(*pbCommit == FALSE) {
		return;
	}

	if (!CheckCurrentUserPermissions(bioContactsNotes, sptWrite))
	{
		*pbCommit = FALSE;
		return;
	}

	if(nCol == 5) {
/*		JJ 10/18/02 - Removed restriction on blank note, it is valid.  See PL #5954
		if(CString(pvarNewValue->bstrVal) == "") {
			MsgBox("You cannot enter an empty note.");
			*pbCommit = FALSE;
			*pbContinue = FALSE;
		}
		else
*/
		// (a.walling 2012-05-17 17:07) - PLID 50481 - Fix BSTR leaks
		CString strNewValue = VarString(pvarNewValue, "");
		if(strNewValue.GetLength() > 4000) {
			MsgBox("The text you entered is longer than the maximum amount (4000) and has been shortened.\n"
				"Please double-check your note and make changes as needed.");
			::VariantClear(pvarNewValue);
			*pvarNewValue = _variant_t(strNewValue.Left(4000)).Detach();
		}
	}
	
}

void CContactsNotes::OnEditingFinishedList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try{

		switch(nCol)
		{
			case 4:	//category
				if(bCommit)
				{
					long id = m_list->GetValue(nRow, 0).lVal;
					// (a.walling 2010-11-01 11:07) - PLID 40965 - Parameterized
					ExecuteParamSql("UPDATE Notes SET Category = {INT} WHERE ID = {INT}", VarLong(varNewValue.lVal), m_list->GetValue(nRow, 0).lVal);
				}
				break;
			case 5:	//note
				CString str;
				if(bCommit) {
					if(varNewValue.vt == VT_BSTR) {
						str = CString(varNewValue.bstrVal);
						// (a.walling 2010-11-01 11:07) - PLID 40965 - Parameterized
						ExecuteParamSql("UPDATE Notes SET Note = {STRING} WHERE ID = {INT}", str, m_list->GetValue(nRow,0).lVal);
					}
				}
				break;
		}
	}NxCatchAll("Error in changing item.");
}

void CContactsNotes::OnColumnClickingList(short nCol, BOOL FAR* bAllowSort) 
{
	switch(nCol){
	case 3:
		*bAllowSort = FALSE;
		break;
	}
}

void CContactsNotes::EditCategories()
{

	CNoteCategories	dlg(this);
	dlg.DoModal();
	
	m_list->Requery();
}

void CContactsNotes::OnAdd() 
{
	Add();
}

void CContactsNotes::OnDelete() 
{
	Delete();	
}

void CContactsNotes::OnEdit() 
{
	EditCategories();	
}

void CContactsNotes::OnEditingStartingList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	// TODO: Add your control notification handler code here
	
}

void CContactsNotes::OnEditingStartedList(long nRow, short nCol, long nEditType) 
{
	long nBegin, nEnd;
	m_list->GetEditingHighlight(&nBegin, &nEnd);
	m_list->SetEditingHighlight(nEnd, nEnd, TRUE);
}

void CContactsNotes::SecureControls() {

	if (!(GetCurrentUserPermissions(bioContactsNotes) & (SPT___W________ANDPASS)))
	{
		// (d.thompson 2009-10-08) - PLID 35888 - Set to read only, don't disable.
		m_list->ReadOnly = VARIANT_TRUE;
	}

	if (!(GetCurrentUserPermissions(bioContactsNotes) & (SPT____C_______ANDPASS)))
	{
		GetDlgItem(IDC_ADD)->EnableWindow(FALSE);
	}

	if (!(GetCurrentUserPermissions(bioContactsNotes) & (SPT_____D______ANDPASS)))
	{
		GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
	}
}

void CContactsNotes::OnSelChangedList(long nNewSel) 
{
	EnableAppropriateButtons();
	
}

void CContactsNotes::EnableAppropriateButtons()
{
	if(m_list->GetCurSel() == -1) {
		GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_DELETE)->EnableWindow(TRUE);
	}
}



void CContactsNotes::OnContactsNotesShowGrid() 
{
	try {
		if(IsDlgButtonChecked(IDC_CONTACTS_NOTES_SHOW_GRID)) {
			SetRemotePropertyInt("ContactsNotesShowGrid", 1, 0, GetCurrentUserName());
			m_list->GridVisible = true;
		}
		else {
			SetRemotePropertyInt("ContactsNotesShowGrid", 0, 0, GetCurrentUserName());
			m_list->GridVisible = false;
		}
	}NxCatchAll("Error in CContactsNotes::OnContactsNotesShowGrid()");
}


LRESULT CContactsNotes::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
	
		if (wParam == NetUtils::NoteCatsF) {
			try {
				//requery the combo source
				IColumnSettingsPtr pCol = m_list->GetColumn(4);
				pCol->PutComboSource(_bstr_t("SELECT ID, Description FROM NoteCatsF UNION SELECT -1, '{No Category}' ORDER BY Description"));
			} NxCatchAll("Error in CContactsNotes::OnTableChanged:NoteCatsF");
			
		}

	}NxCatchAll("Error in CContactsNotes::OnTableChanged");

	return 0;
}