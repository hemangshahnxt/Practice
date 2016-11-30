// MacroEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MacroEditDlg.h"
#include "globalutils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;

#define MACRO_DL_COLUMN_ID 0
#define MACRO_DL_COLUMN_DESCRIPTION 1

/////////////////////////////////////////////////////////////////////////////
// CMacroEditDlg dialog


CMacroEditDlg::CMacroEditDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMacroEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMacroEditDlg)
		// NOTE: the ClassWizard will add member initialization here
		m_ID = -1;
	//}}AFX_DATA_INIT
}


void CMacroEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMacroEditDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_MACRO_DESC, m_nxeditMacroDesc);
	DDX_Control(pDX, IDC_MACRO_NOTES, m_nxeditMacroNotes);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ADD_MACRO, m_btnAddMacro);
	DDX_Control(pDX, IDC_DELETE_MACRO, m_btnDeleteMacro);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMacroEditDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMacroEditDlg)
	ON_BN_CLICKED(IDC_ADD_MACRO, OnAddMacro)
	ON_BN_CLICKED(IDC_DELETE_MACRO, OnDeleteMacro)
	ON_EN_KILLFOCUS(IDC_MACRO_DESC, OnKillfocusMacroDesc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMacroEditDlg message handlers

BOOL CMacroEditDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	// Bind to the datalist
	try {
		// (c.haag 2008-04-25 12:23) - PLID 29790 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAddMacro.AutoSet(NXB_NEW);
		m_btnDeleteMacro.AutoSet(NXB_DELETE);

		m_dlMacroCombo = BindNxDataListCtrl(this, IDC_MACRO_LIST_COMBO, GetRemoteData(), true);
		m_dlCategoryCombo = BindNxDataListCtrl(this, IDC_CATEGORY_COMBOLIST, GetRemoteData(), true);
			
	
		// make sure both datalists are loaded, then select the first macro
		m_dlMacroCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		m_dlCategoryCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		if(m_dlMacroCombo->GetRowCount() > 0){
			m_dlMacroCombo->PutCurSel(0);
			LoadCurrentlySelectedMacro();
		}
	
		EnableAppropriateFields();

	}NxCatchAll("Error loading Macro list");


		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CMacroEditDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMacroEditDlg)
	ON_EVENT(CMacroEditDlg, IDC_MACRO_LIST_COMBO, 16 /* SelChosen */, OnSelChosenMacroListCombo, VTS_I4)
	ON_EVENT(CMacroEditDlg, IDC_CATEGORY_COMBOLIST, 18 /* RequeryFinished */, OnRequeryFinishedCategoryCombolist, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMacroEditDlg::OnAddMacro() 
{
	if(m_dlMacroCombo->GetCurSel() != sriNoRow && (SomethingHasChanged() || m_ID == -1)){
		// the current one is not new and something has changed or else it's a new one so
		if(IDYES == MessageBox("Do you wish to save your current macro?\r\nClicking no will cancel your changes.", "", MB_YESNO)){
			if(!CheckFields()){
				return;
			}
			SaveCurrentMacro();
		}
		else{
			m_dlMacroCombo->Requery();
		}
	}

	CString strNewDesc;
	bool bSuccess = false;
	while(!bSuccess && IDOK == InputBoxLimited(this, "Enter a short description for this macro:", strNewDesc, "",50,false,false,NULL)){
		strNewDesc.TrimLeft(); strNewDesc.TrimRight();
		bSuccess = IsMacroDescValid(strNewDesc);

		if(bSuccess) {
			SetDlgItemText(IDC_MACRO_DESC, strNewDesc);
			SetDlgItemText(IDC_MACRO_NOTES, "");
			m_dlCategoryCombo->PutCurSel(0);
			// enter the item temporarily in the data list
			IRowSettingsPtr pRow = m_dlMacroCombo->GetRow(-1);
			pRow->Value[0] = long(-1);
			pRow->Value[1] = _bstr_t(strNewDesc);
			m_dlMacroCombo->AddRow(pRow);
			m_dlMacroCombo->SetSelByColumn(0, (long)-1);
			// (a.walling 2011-05-10 10:24) - PLID 41789 - New fields for suppress username and show on statement
			CheckDlgButton(IDC_CHECK_SUPPRESS_USERNAME, FALSE);
			CheckDlgButton(IDC_CHECK_SHOW_ON_STATEMENT, FALSE);
		
			m_ID = -1;
		}
	}
	
	EnableAppropriateFields();
}

void CMacroEditDlg::SaveCurrentMacro()
{
	long nCurCategory = m_dlCategoryCombo->GetCurSel();
	long nCurSel = m_dlMacroCombo->GetCurSel();
	
	// make sure something has changed before saving and we have something selected
	if((!SomethingHasChanged() && m_ID != -1) || nCurSel == sriNoRow) {
		return;
	}
	
	try{
		CString strDesc;
		CString strMacroNotes;
		
		GetDlgItemText(IDC_MACRO_DESC, strDesc);
		GetDlgItemText(IDC_MACRO_NOTES, strMacroNotes);
		
		long nCatID;
		
		_variant_t tmpVar = m_dlCategoryCombo->GetValue(nCurCategory,0);
		if(tmpVar.vt == VT_I4 && VarLong(tmpVar) > 0){
			nCatID = VarLong(tmpVar);
		}
		else{
			nCatID = NULL;
		}
		
		if(m_ID == -1){
			// it's a new macro, so we need to find a new ID and insert into the table
			m_ID = NewNumber("NoteMacroT", "ID");
			m_dlMacroCombo->PutValue(nCurSel, 0, m_ID);
			
			// (a.walling 2011-05-10 10:24) - PLID 41789 - New fields for suppress username and show on statement
			// (j.jones 2010-04-20 08:55) - PLID 38273 - corrected potential formatting issues by simply parameterizing
			ExecuteParamSql("INSERT INTO NoteMacroT (ID, Description, MacroNotes, CategoryID, SuppressUserName, ShowOnStatement) "
				"VALUES ({INT}, {STRING}, {STRING}, CASE WHEN {INT} <= 0 THEN NULL ELSE {INT} END, {BIT}, {BIT})"
				, m_ID, strDesc, strMacroNotes, nCatID, nCatID
				, IsDlgButtonChecked(IDC_CHECK_SUPPRESS_USERNAME), IsDlgButtonChecked(IDC_CHECK_SHOW_ON_STATEMENT));
		}
		else{
			// the macro already exists so just update the existing record
			// (a.walling 2011-05-10 10:24) - PLID 41789 - New fields for suppress username and show on statement
			// (j.jones 2010-04-20 08:55) - PLID 38273 - corrected potential formatting issues by simply parameterizing
			ExecuteParamSql("UPDATE NoteMacroT SET Description = {STRING}, MacroNotes = {STRING}, CategoryID = CASE WHEN {INT} <= 0 THEN NULL ELSE {INT} END, "
				"SuppressUserName = {BIT}, ShowOnStatement = {BIT} "
				"WHERE ID = {INT}"
				, strDesc, strMacroNotes, nCatID, nCatID
				, IsDlgButtonChecked(IDC_CHECK_SUPPRESS_USERNAME), IsDlgButtonChecked(IDC_CHECK_SHOW_ON_STATEMENT)
				, m_ID);
		}

	}NxCatchAll("Error SavingCurrentMacro");

	m_dlMacroCombo->Requery();
	m_dlMacroCombo->SetSelByColumn(0, m_ID);
}

bool CMacroEditDlg::CheckFields()
{
	CString strDesc;
	CString strMacroNotes;

	//DRT 12/2/2003 - PLID 10294 - If there is nothing selected, then the fields are A-OK!
	//	This can happen if you add a new item, don't put in any info, and immediately delete it.
	//	You'll then be on a disabled item, and this function will fail.
	if(m_dlMacroCombo->GetCurSel() == -1)
		return true;

	GetDlgItemText(IDC_MACRO_DESC, strDesc);
	GetDlgItemText(IDC_MACRO_NOTES, strMacroNotes);

	if(strDesc == ""){
		AfxMessageBox("Please enter a description.");
		return false;
	}
	if(strDesc.GetLength() > 50){
		AfxMessageBox("The description is too long.  Please enter a description less then 50 characters.");
		return false;
	}
	
	if(strMacroNotes == ""){
		AfxMessageBox("Please enter note for this macro.");
		return false;
	}
	if(strMacroNotes.GetLength() > 4000){
		AfxMessageBox("The notes are too long.  Please enter notes less then 4000 characters.");
		return false;
	}

	return true;
}

void CMacroEditDlg::LoadCurrentlySelectedMacro()
{
	try{
		// get the macro ID of the one selected
		//DRT 3/3/2004 - PLID 11251 - We cannot just blindly grab the current selection, what if there is no current selection!
		if(m_dlMacroCombo->GetCurSel() != sriNoRow)
			m_ID = m_dlMacroCombo->GetValue(m_dlMacroCombo->GetCurSel(), MACRO_DL_COLUMN_ID);

		// (a.walling 2011-05-10 10:24) - PLID 41789 - Parameterized
		_RecordsetPtr rsMacro = CreateParamRecordset("SELECT * FROM NoteMacroT WHERE ID = {INT}", m_ID);
		if(!rsMacro->eof){
			// the record was there so load it in the data
			SetDlgItemText(IDC_MACRO_DESC, AdoFldString(rsMacro, "Description", ""));
			SetDlgItemText(IDC_MACRO_NOTES, AdoFldString(rsMacro, "MacroNotes", ""));
			// (a.walling 2011-05-10 10:24) - PLID 41789 - New fields for suppress username and show on statement
			CheckDlgButton(IDC_CHECK_SUPPRESS_USERNAME, AdoFldBool(rsMacro, "SuppressUsername", FALSE));
			CheckDlgButton(IDC_CHECK_SHOW_ON_STATEMENT, AdoFldBool(rsMacro, "ShowOnStatement", FALSE));
			
			long nCatID = VarLong(AdoFldLong(rsMacro->GetFields(), "CategoryID", NULL));
			if(nCatID != NULL){
				m_dlCategoryCombo->SetSelByColumn(0, nCatID);
			}
			else{
				// there's no category for this macro
				if(m_dlCategoryCombo->FindByColumn(0, long(-1), 0, FALSE) < 0){
					// insert a row that allows them to select "No category" for a macro
					IRowSettingsPtr pRow;
					pRow = m_dlCategoryCombo->GetRow(-1);
					pRow->PutValue(0,long(-1));
					pRow->PutValue(1,"<No Category>");
					m_dlCategoryCombo->InsertRow(pRow,0);
				}
				m_dlCategoryCombo->TrySetSelByColumn(0, long(-1));
			}
		}
	}NxCatchAll("Error in CMacroEditDlg::LoadCurrentlySelectedMacro");
}

void CMacroEditDlg::OnOK()
{
	// make sure the fields we have are ok, then save if they are
	if(!CheckFields()){
		return;
	}
	
	SaveCurrentMacro();
		
	CNxDialog::OnOK();
}

void CMacroEditDlg::EnableAppropriateFields()
{
	if(m_dlMacroCombo->GetCurSel() < 0)
	{
		// nothing's selected so don't let them edit anything
		GetDlgItem(IDC_MACRO_DESC)->EnableWindow(FALSE);
		GetDlgItem(IDC_MACRO_NOTES)->EnableWindow(FALSE);
		GetDlgItem(IDC_DELETE_MACRO)->EnableWindow(FALSE);
		GetDlgItem(IDC_CATEGORY_COMBOLIST)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_SUPPRESS_USERNAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_SHOW_ON_STATEMENT)->EnableWindow(FALSE);
	}
	else{
		GetDlgItem(IDC_MACRO_DESC)->EnableWindow(TRUE);
		GetDlgItem(IDC_MACRO_NOTES)->EnableWindow(TRUE);
		GetDlgItem(IDC_DELETE_MACRO)->EnableWindow(TRUE);
		GetDlgItem(IDC_CATEGORY_COMBOLIST)->EnableWindow(TRUE);
		// (a.walling 2011-05-10 10:24) - PLID 41789 - New fields for suppress username and show on statement
		GetDlgItem(IDC_CHECK_SUPPRESS_USERNAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_SHOW_ON_STATEMENT)->EnableWindow(TRUE);
	}
}
bool CMacroEditDlg::SomethingHasChanged()
{
	CString strDesc;
	CString strMacroNotes;

	GetDlgItemText(IDC_MACRO_DESC, strDesc);
	GetDlgItemText(IDC_MACRO_NOTES, strMacroNotes);

	long nCatID;
	long nCurSel = m_dlCategoryCombo->GetCurSel();
	if(nCurSel == sriNoRow){
		return false;
	}
	_variant_t tmpVar = m_dlCategoryCombo->GetValue(nCurSel,0);
	if(tmpVar.vt == VT_I4 && VarLong(tmpVar) > 0){
		nCatID = VarLong(tmpVar);
	}
	else{
		nCatID = -1;
	}
	
	// (a.walling 2011-05-10 10:24) - PLID 41789 - Parameterized
	// get the fields out of the data and compare it to whats on the screen
	_RecordsetPtr rsMacro = CreateParamRecordset("SELECT * FROM NoteMacroT WHERE ID = {INT}", m_ID);
	if(!rsMacro->eof){
		if(strDesc != AdoFldString(rsMacro, "Description", "")){
			// something has changed
			return true;
		}
		if(strMacroNotes != AdoFldString(rsMacro, "MacroNotes", "")){
			// something has changed
			return true;
		}
		if(nCatID != AdoFldLong(rsMacro, "CategoryID", -1)){
			// something has changed
			return true;
		}
		// (a.walling 2011-05-10 10:24) - PLID 41789 - New fields for suppress username and show on statement
		// I use !! since a BOOL may also be anything that is not zero.
		if (!!IsDlgButtonChecked(IDC_CHECK_SUPPRESS_USERNAME) ^ !!AdoFldBool(rsMacro, "SuppressUserName", FALSE)) {
			return true;
		}
		if (!!IsDlgButtonChecked(IDC_CHECK_SHOW_ON_STATEMENT) ^ !!AdoFldBool(rsMacro, "ShowOnStatement", FALSE)) {
			return true;
		}
	}
	// passed everything without finding any differences
	return false;
}

void CMacroEditDlg::OnSelChosenMacroListCombo(long nRow) 
{
	try{
		if((m_dlMacroCombo->GetCurSel() != sriNoRow && SomethingHasChanged()) || m_ID == -1 && (m_dlMacroCombo->GetRowCount() > 0)){
			// need to save, make sure they want to
			if(IDYES == MessageBox("Do you wish to save your current macro?\r\nClicking no will cancel your changes.", "", MB_YESNO)){
				if(!CheckFields()){
					// there was a problem with a field, return the selection to what it was before
					m_dlMacroCombo->SetSelByColumn(0, m_ID);
					return;
				}
				SaveCurrentMacro();
			}
			else{
				if(m_ID == -1){
					// its a new row and the user doesn't want to save, delete the row from the data list
					m_dlMacroCombo->RemoveRow(m_dlMacroCombo->FindByColumn(0, m_ID, 0, FALSE));
				}
				else{
					// its an existing macro, but they don't want to change the name, so we need to reset the datalist
					m_dlMacroCombo->Requery();
				}
			}
		}
		m_dlMacroCombo->PutCurSel(nRow);
		LoadCurrentlySelectedMacro();
		EnableAppropriateFields();
	}NxCatchAll("Error changing macro list");
}

void CMacroEditDlg::OnDeleteMacro() 
{
	if(IDNO == MessageBox("Are you sure you wish to delete this macro?", NULL, MB_YESNO)){
		return;
	}
	else{
		try{
			CString strDeleteSql;
			strDeleteSql.Format("DELETE FROM NoteMacroT WHERE ID = %li", m_ID);
			ExecuteSql(strDeleteSql);
			m_dlMacroCombo->Requery();
			// load the first row if there are any rows
			if(m_dlMacroCombo->GetRowCount() > 0){
				m_dlMacroCombo->PutCurSel(0);
			}
			LoadCurrentlySelectedMacro();
			EnableAppropriateFields();
		}NxCatchAll("Error Deleting Macro");
	}
}

void CMacroEditDlg::OnRequeryFinishedCategoryCombolist(short nFlags) 
{
	// the "No Category" option is already there, then don't add it again
	if(m_dlCategoryCombo->FindByColumn(0, long(-1), 0, FALSE) < 0){
		IRowSettingsPtr pRow;
		pRow = m_dlCategoryCombo->GetRow(-1);
		pRow->PutValue(0,long(-1));
		pRow->PutValue(1,"<No Category>");
		m_dlCategoryCombo->InsertRow(pRow,0);
	}
}

void CMacroEditDlg::OnKillfocusMacroDesc() 
{
	try{
		// get the new and old descriptions
		CString strNewDesc, strOldDesc;
		GetDlgItemText(IDC_MACRO_DESC, strNewDesc);
		strNewDesc.TrimLeft();strNewDesc.TrimRight();
		// set the test to the trimmed text since that's what we'll be using
		SetDlgItemText(IDC_MACRO_DESC, strNewDesc);
		strOldDesc = VarString(m_dlMacroCombo->GetValue(m_dlMacroCombo->GetCurSel(), 1));

		if(!IsMacroDescValid(strNewDesc, strOldDesc)) {
			// if there is something wrong with the new desc, then set the text back to what it was before
			SetDlgItemText(IDC_MACRO_DESC, strOldDesc);
			// put the focus back in the edit box
			GetDlgItem(IDC_MACRO_DESC)->SetFocus();
			return;
		}
		
		// lets update the datalist to the new description
		IRowSettingsPtr pRow = m_dlMacroCombo->GetRow(m_dlMacroCombo->GetCurSel());
		pRow->PutValue(1 /* this is the column that displays the description*/ , _bstr_t(strNewDesc));
	}NxCatchAll("Error in CMacroEditDlg::OnKillfocusMacroDesc() ");
}

bool CMacroEditDlg::IsMacroDescValid(CString strDescToValidate, CString strOldDesc/* ="" */)
{
	//check to make sure the name isn't blank
	if(strDescToValidate == "") {
			MessageBox("You cannot enter a blank description.");
			return false;
	}

	// if the new name and the old name are the same, then the name is valid
	if(strDescToValidate.CompareNoCase(strOldDesc) == 0){
		return true;
	}
	
	//check to make sure the group name isn't duplicated
	long pCurRowEnum = m_dlMacroCombo->GetFirstRowEnum();
	while(pCurRowEnum != 0){
		IRowSettingsPtr pRow;
		{
			IDispatch *lpDisp;
			m_dlMacroCombo->GetNextRowEnum(&pCurRowEnum, &lpDisp);
			pRow = lpDisp;
			lpDisp->Release();
			lpDisp = NULL;
		}

		ASSERT(pRow != NULL);
		_variant_t var = pRow->GetValue(1);
		
		CString strDesc;
		strDesc = VarString(var);
		
		if(strDescToValidate.CompareNoCase(strDesc) == 0)
		{
			AfxMessageBox("Descriptions must be unique.");
			return false;
		}
	}
	return true;
}
