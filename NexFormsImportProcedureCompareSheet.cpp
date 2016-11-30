// NexFormsImportProcedureCompareSheet.cpp : implementation file
//

#include "stdafx.h"
#include <NxPracticeSharedLib/RichEditUtils.h>
#include "NexFormsImportProcedureCompareSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/********************************************************************************/
/*																				*/
/* For documentation on the NexForms importer/exporter, see:					*/
/*																				*/
/*		http://192.168.1.2/developers/doku.php?id=nexforms_importer_exporter	*/
/*																				*/
/********************************************************************************/

using namespace NXDATALIST2Lib;


enum ProcedureColumns
{
	pcName = 0,
	pcPointer,
};

enum FieldColumns
{
	fcName = 0,
	fcNameDisplay,
	fcNewContentPointer,
	fcOldContent,
	fcOldContentLoaded,
	fcImportPointer,
	fcNeedReviewPointer,
	fcIsRichTextField, // (z.manning, 10/09/2007) - PLID 27706 - Added column for whether or not field is rich text.
};


// (z.manning, 08/30/2007) - PLID 18359 - Created a new importer for NexForms.
/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportProcedureCompareSheet dialog


CNexFormsImportProcedureCompareSheet::CNexFormsImportProcedureCompareSheet(CNexFormsImportWizardMasterDlg* pParent)
	: CNexFormsImportWizardSheet(CNexFormsImportProcedureCompareSheet::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNexFormsImportProcedureCompareSheet)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CNexFormsImportProcedureCompareSheet::DoDataExchange(CDataExchange* pDX)
{
	CNexFormsImportWizardSheet::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexFormsImportProcedureCompareSheet)
	DDX_Control(pDX, IDC_OVERWRITE_FIELD, m_btnOverwrite);
	DDX_Control(pDX, IDC_NEEDS_REVIEW, m_btnNeedReview);
	DDX_Control(pDX, IDC_PREVIOUS_NEXFORMS_FIELD, m_btnPreviousField);
	DDX_Control(pDX, IDC_NEXT_NEXFORMS_FIELD, m_btnNextField);
	DDX_Control(pDX, IDC_RICH_EDIT_CTRL, m_ctrlRichEdit);
	DDX_Control(pDX, IDC_OLD_CONTENT_PLAIN, m_nxeditOldContentPlain);
	DDX_Control(pDX, IDC_NEW_CONTENT_PLAIN, m_nxeditNewContentPlain);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexFormsImportProcedureCompareSheet, CNexFormsImportWizardSheet)
	//{{AFX_MSG_MAP(CNexFormsImportProcedureCompareSheet)
	ON_BN_CLICKED(IDC_PREVIOUS_NEXFORMS_FIELD, OnPreviousNexformsField)
	ON_BN_CLICKED(IDC_NEXT_NEXFORMS_FIELD, OnNextNexformsField)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_OVERWRITE_FIELD, OnOverwriteField)
	ON_BN_CLICKED(IDC_NEEDS_REVIEW, OnNeedsReview)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportProcedureCompareSheet message handlers

BOOL CNexFormsImportProcedureCompareSheet::OnInitDialog() 
{
	try
	{
		CNexFormsImportWizardSheet::OnInitDialog();

		m_pdlProcedures = BindNxDataList2Ctrl(this, IDC_PROCEDURE_SELECT, GetRemoteData(), false);
		m_pdlFields = BindNxDataList2Ctrl(this, IDC_NEXFORMS_FIELD_SELECT, GetRemoteData(), false);

		m_pOldRichText = GetDlgItem(IDC_OLD_CONTENT)->GetControlUnknown();
		// (z.manning, 07/11/2007) - Don't allow users to modify the existing content.
		m_pOldRichText->PutReadOnly(VARIANT_TRUE);
		m_pNewRichText = GetDlgItem(IDC_NEW_CONTENT)->GetControlUnknown();

		// (z.manning, 08/03/2007) - Initialize our invisible rich edit control.
		// (See OnNeedsReview for more details as to why we have an invisible rich edit control.)
		m_ctrlRichEdit.SetEventMask(m_ctrlRichEdit.GetEventMask() | ENM_CHANGE | ENM_SELCHANGE);
		m_ctrlRichEdit.SetOptions(ECOOP_OR, ECO_SAVESEL);
		m_ctrlRichEdit.LimitText(0xFFFFFFF);
	
	}NxCatchAll("CNexFormsImportProcedureCompareSheet::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNexFormsImportProcedureCompareSheet::Load()
{	
	m_bSkipSheet = FALSE;
	// (z.manning, 07/31/2007) - If they're a new NexForms user, this sheet is pointless for them.
	if(m_pdlgMaster->m_ImportInfo.m_eImportType == nfitNew || m_pdlgMaster->m_ImportInfo.m_eImportType == nfitNewNexFormsExistingTracking) {
		m_bSkipSheet = TRUE;
	}
	else
	{
		// (z.manning, 10/10/2007) - PLID 27717 The procedure post update thread determines which fields we want to import
		// for each procedure by default, so wait for it to finish. It may take a while to finish, so if the 
		// user went through the previous portion of the wizard real quick, we'll likely have to wait here.
		if(m_pdlgMaster->m_ImportInfo.m_pPostProcedureThread != NULL) {
			if(WaitForSingleObject(m_pdlgMaster->m_ImportInfo.m_pPostProcedureThread->m_hThread, 120000) == WAIT_TIMEOUT) {
				// (z.manning, 10/10/2007) - It didn't finish, for some odd reason, but that's not the end of the
				// world. All that will happen is that fields that my not have any data will be checked by default.
				ASSERT(FALSE);
				// (z.manning, 10/25/2007) - To avoid potentially odd things happening, we need to end the thread.
				SetEvent(m_pdlgMaster->m_ImportInfo.m_hevDestroying);
				if(WaitForSingleObject(m_pdlgMaster->m_ImportInfo.m_pPostProcedureThread->m_hThread, 5000) == WAIT_TIMEOUT) {
					// (z.manning, 10/25/2007) - This should not ever happen, but just in case, let's kill the thread.
					ASSERT(FALSE);
					TerminateThread(m_pdlgMaster->m_ImportInfo.m_pPostProcedureThread->m_hThread, 0);
				}
			}
		}

		m_pdlProcedures->Clear();
		m_pdlFields->Clear();
		for(int i = 0; i < m_pdlgMaster->m_ImportInfo.m_arypProcedures.GetSize(); i++)
		{
			NexFormsProcedure *procedure = m_pdlgMaster->m_ImportInfo.m_arypProcedures.GetAt(i);

			// (z.manning, 06/28/2007) - Only load procedures that we're updating existing ones.
			if(procedure->bImport && procedure->nExistingProcedureID != 0)
			{
				IRowSettingsPtr pRow = m_pdlProcedures->GetNewRow();
				pRow->PutValue(pcName, _bstr_t(procedure->strName));
				pRow->PutValue(pcPointer, (long)procedure);
				m_pdlProcedures->AddRowSorted(pRow, NULL);
			}
		}

		if(!m_strSelecedProcedure.IsEmpty()) {
			m_pdlProcedures->SetSelByColumn(pcName, _bstr_t(m_strSelecedProcedure));
		}
		if(m_pdlProcedures->GetCurSel() == NULL) {
			m_pdlProcedures->PutCurSel(m_pdlProcedures->GetFirstRow());
		}
		OnSelChangedProcedureSelect(NULL, m_pdlProcedures->GetCurSel());

		// (z.manning, 07/11/2007) - If we don't have any procedures we're updating, we can skip this sheet.
		if(m_pdlProcedures->GetRowCount() == 0) {
			m_bSkipSheet = TRUE;
		}
	}
}

BEGIN_EVENTSINK_MAP(CNexFormsImportProcedureCompareSheet, CNexFormsImportWizardSheet)
    //{{AFX_EVENTSINK_MAP(CNexFormsImportProcedureCompareSheet)
	ON_EVENT(CNexFormsImportProcedureCompareSheet, IDC_PROCEDURE_SELECT, 2 /* SelChanged */, OnSelChangedProcedureSelect, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CNexFormsImportProcedureCompareSheet, IDC_NEXFORMS_FIELD_SELECT, 2 /* SelChanged */, OnSelChangedNexformsFieldSelect, VTS_DISPATCH VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

#define ADD_PLAIN_TEXT_FIELD_IF_NOT_EMPTY(field) \
	if(!procedureNew->str##field.IsEmpty()) { \
		pRowToAdd = m_pdlFields->GetNewRow(); \
		pRowToAdd->PutValue(fcName, #field); \
		CString strFieldName; \
		if(m_pdlgMaster->m_mapCustomFieldNames.Lookup(#field, strFieldName)) { \
			pRowToAdd->PutValue(fcNameDisplay, _bstr_t(strFieldName)); \
		} \
		else { \
			pRowToAdd->PutValue(fcNameDisplay, #field); \
		} \
		pRowToAdd->PutValue(fcNewContentPointer, (long)&procedureNew->str##field); \
		pRowToAdd->PutValue(fcOldContentLoaded, _variant_t(false)); \
		pRowToAdd->PutValue(fcImportPointer, (long)&procedureNew->bImport##field); \
		pRowToAdd->PutValue(fcNeedReviewPointer, (long)NULL); \
		pRowToAdd->PutValue(fcIsRichTextField, _variant_t(false)); \
		m_pdlFields->AddRowAtEnd(pRowToAdd, NULL); \
	}

#define ADD_FIELD_IF_NOT_EMPTY(field) \
	if(!procedureNew->str##field.IsEmpty()) { \
		ADD_PLAIN_TEXT_FIELD_IF_NOT_EMPTY(field) \
		pRowToAdd->PutValue(fcNeedReviewPointer, (long)&procedureNew->bNeedReview##field); \
		pRowToAdd->PutValue(fcIsRichTextField, _variant_t(true)); \
	}

void CNexFormsImportProcedureCompareSheet::OnSelChangedProcedureSelect(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try
	{
		SaveNewContent(m_pdlFields->GetCurSel());
		IRowSettingsPtr pNewRow(lpNewSel);
		m_pdlFields->Clear();
		if(pNewRow == NULL)
		{
			// (z.manning, 07/11/2007) - No selection, clear the field list and disable the rich text control.
			m_pNewRichText->PutReadOnly(VARIANT_TRUE);
			m_strSelecedProcedure.Empty();
			CheckDlgButton(IDC_OVERWRITE_FIELD, BST_UNCHECKED);
			GetDlgItem(IDC_OVERWRITE_FIELD)->EnableWindow(FALSE);
			OnSelChangedNexformsFieldSelect(NULL, m_pdlFields->GetCurSel());
		}
		else
		{
			m_pNewRichText->PutReadOnly(VARIANT_FALSE);
			GetDlgItem(IDC_OVERWRITE_FIELD)->EnableWindow(TRUE);

			// (z.manning, 07/11/2007) - Go through all NexForms fields for this procedure and add all the ones
			// that are set to be imported to the fields list.
			NexFormsProcedure *procedureNew = (NexFormsProcedure*)VarLong(pNewRow->GetValue(pcPointer));
			m_strSelecedProcedure = VarString(pNewRow->GetValue(pcName), "");

			IRowSettingsPtr pRowToAdd;
			// (z.manning, 10/10/2007) - PLID 27706 - Also show the cheat sheet fields (and official name) in this dialog.
			ADD_PLAIN_TEXT_FIELD_IF_NOT_EMPTY(OfficialName);
			ADD_PLAIN_TEXT_FIELD_IF_NOT_EMPTY(Custom1);
			ADD_PLAIN_TEXT_FIELD_IF_NOT_EMPTY(Custom2);
			ADD_PLAIN_TEXT_FIELD_IF_NOT_EMPTY(Custom3);
			ADD_PLAIN_TEXT_FIELD_IF_NOT_EMPTY(Custom4);
			ADD_PLAIN_TEXT_FIELD_IF_NOT_EMPTY(Custom5);
			ADD_PLAIN_TEXT_FIELD_IF_NOT_EMPTY(Custom6);
			ADD_FIELD_IF_NOT_EMPTY(Consent);
			ADD_FIELD_IF_NOT_EMPTY(AltConsent);
			ADD_FIELD_IF_NOT_EMPTY(Alternatives);
			ADD_FIELD_IF_NOT_EMPTY(Bandages);
			ADD_FIELD_IF_NOT_EMPTY(Complications);
			ADD_FIELD_IF_NOT_EMPTY(HospitalStay);
			ADD_FIELD_IF_NOT_EMPTY(MiniDescription);
			ADD_FIELD_IF_NOT_EMPTY(PreOp);
			ADD_FIELD_IF_NOT_EMPTY(ProcDetails); // (z.manning, 09/04/2007) - PLID 27286 - Renamed field in data to ProcDetails.
			ADD_FIELD_IF_NOT_EMPTY(PostOp);
			ADD_FIELD_IF_NOT_EMPTY(Recovery);
			ADD_FIELD_IF_NOT_EMPTY(Risks);
			ADD_FIELD_IF_NOT_EMPTY(Showering);
			ADD_FIELD_IF_NOT_EMPTY(SpecialDiet);
			ADD_FIELD_IF_NOT_EMPTY(TheDayOf);
			ADD_FIELD_IF_NOT_EMPTY(CustomSection1);
			ADD_FIELD_IF_NOT_EMPTY(CustomSection2);
			ADD_FIELD_IF_NOT_EMPTY(CustomSection3);
			ADD_FIELD_IF_NOT_EMPTY(CustomSection4);
			ADD_FIELD_IF_NOT_EMPTY(CustomSection5);
			ADD_FIELD_IF_NOT_EMPTY(CustomSection6);
			ADD_FIELD_IF_NOT_EMPTY(CustomSection7);
			ADD_FIELD_IF_NOT_EMPTY(CustomSection8);
			ADD_FIELD_IF_NOT_EMPTY(CustomSection9);
			ADD_FIELD_IF_NOT_EMPTY(CustomSection10);

			if(m_strSelectedField.IsEmpty() || m_pdlFields->SetSelByColumn(fcName, _bstr_t(m_strSelectedField)) == NULL) {
				m_pdlFields->PutCurSel(m_pdlFields->GetFirstRow());
			}
			OnSelChangedNexformsFieldSelect(NULL, m_pdlFields->GetCurSel());
			if(m_pdlFields->GetCurSel() == NULL) {
				m_pNewRichText->PutReadOnly(VARIANT_TRUE);
			}
		}

		RefreshFieldMovementButtons();

	}NxCatchAll("CNexFormsImportProcedureCompareSheet::OnSelChangedProcedureSelect");
}

void CNexFormsImportProcedureCompareSheet::OnSelChangedNexformsFieldSelect(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try
	{
		// (z.manning, 07/11/2007) - First save the previously selected field.
		if(lpOldSel != NULL) {
			SaveNewContent(lpOldSel);
		}

		IRowSettingsPtr pNewRow(lpNewSel);
		if(pNewRow == NULL)
		{
			GetDlgItem(IDC_OLD_CONTENT)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_NEW_CONTENT)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_OLD_CONTENT_PLAIN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_NEW_CONTENT_PLAIN)->ShowWindow(SW_HIDE);

			m_pNewRichText->PutReadOnly(VARIANT_TRUE);
			m_pNewRichText->PutRichText("");
			m_pOldRichText->PutRichText("");
			m_strSelectedField.Empty();
			CheckDlgButton(IDC_OVERWRITE_FIELD, BST_UNCHECKED);
			GetDlgItem(IDC_OVERWRITE_FIELD)->EnableWindow(FALSE);
		}
		else
		{
			m_pNewRichText->PutReadOnly(VARIANT_FALSE);
			m_strSelectedField = VarString(pNewRow->GetValue(pcName), "");
			GetDlgItem(IDC_OVERWRITE_FIELD)->EnableWindow(TRUE);

			// (z.manning, 07/11/2007) - We store a pointer to the string where this field's content is.
			CString *pstrNewContent = (CString*)VarLong(pNewRow->GetValue(fcNewContentPointer));
			if(VarBool(pNewRow->GetValue(fcIsRichTextField)))
			{
				// (z.manning, 10/10/2007) - PLID 27706 - Show the rich text controls and hide the edit boxes.
				GetDlgItem(IDC_OLD_CONTENT)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_NEW_CONTENT)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_OLD_CONTENT_PLAIN)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_NEW_CONTENT_PLAIN)->ShowWindow(SW_HIDE);

				m_pNewRichText->PutRichText(_bstr_t(*pstrNewContent));
				BOOL *bNeedReview = (BOOL*)VarLong(pNewRow->GetValue(fcNeedReviewPointer));
				CheckDlgButton(IDC_NEEDS_REVIEW, *bNeedReview ? BST_CHECKED : BST_UNCHECKED);
			}
			else
			{
				// (z.manning, 10/10/2007) - PLID 27706 - Hide the rich text controls and show the edit boxes.
				GetDlgItem(IDC_OLD_CONTENT)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_NEW_CONTENT)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_OLD_CONTENT_PLAIN)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_NEW_CONTENT_PLAIN)->ShowWindow(SW_SHOW);

				SetDlgItemText(IDC_NEW_CONTENT_PLAIN, *pstrNewContent);
				CheckDlgButton(IDC_NEEDS_REVIEW, BST_UNCHECKED);
			}

			BOOL *bImport = (BOOL*)VarLong(pNewRow->GetValue(fcImportPointer));
			CheckDlgButton(IDC_OVERWRITE_FIELD, *bImport ? BST_CHECKED : BST_UNCHECKED);

			// (z.manning, 07/11/2007) - If we haven't yet, we need to load the content for this procedure.
			if( !VarBool(pNewRow->GetValue(fcOldContentLoaded)) )
			{
				NexFormsProcedure *procedure = (NexFormsProcedure*)VarLong(m_pdlProcedures->GetCurSel()->GetValue(pcPointer));
				long nProcedureID = procedure->nExistingProcedureID;
				if(nProcedureID > 0) {
					ADODB::_RecordsetPtr prs = CreateParamRecordset(FormatString(
						"SELECT [%s] AS Field FROM ProcedureT WHERE ID = {INT}"
						, _Q(VarString(pNewRow->GetValue(fcName)))), nProcedureID);
					pNewRow->PutValue(fcOldContent, prs->Fields->GetItem("Field")->Value);
					pNewRow->PutValue(fcOldContentLoaded, _variant_t(VARIANT_TRUE, VT_BOOL));
				}
				else {
					ASSERT(FALSE);
				}
			}
			if(VarBool(pNewRow->GetValue(fcIsRichTextField))) {
				// (z.manning, 10/10/2007) - PLID 27706 - It's a rich text field so put the text in that control.
				m_pOldRichText->PutRichText(_bstr_t(pNewRow->GetValue(fcOldContent)));
			}
			else {
				// (z.manning, 10/10/2007) - PLID 27706 - Plain text-- just put it in the multi-line edit box.
				SetDlgItemText(IDC_OLD_CONTENT_PLAIN, VarString(pNewRow->GetValue(fcOldContent)));
			}
		}

		RefreshFieldMovementButtons();
		OnOverwriteField(); // (z.manning, 08/03/2007) - Refreshes the checkboxes.

	}NxCatchAll("CNexFormsImportProcedureCompareSheet::OnSelChangedNexformsFieldSelect");
}

void CNexFormsImportProcedureCompareSheet::OnPreviousNexformsField() 
{
	try
	{
		// (z.manning, 10/31/2007) - Make sure to save the previously selected field.
		SaveNewContent(m_pdlFields->GetCurSel());

		// (z.manning, 07/11/2007) - If there's a previous field in the list, select it.
		IRowSettingsPtr pFieldRow = m_pdlFields->GetCurSel(), pNewFieldRow;
		if(pFieldRow != NULL && pFieldRow->GetPreviousRow() != NULL)
		{
			pNewFieldRow = pFieldRow->GetPreviousRow();
		}
		else
		{
			// (z.manning, 07/11/2007) - Ok, we're on the first field. Instead, let's go to 
			// the previous procedure.
			IRowSettingsPtr pProcedureRow = m_pdlProcedures->GetCurSel();
			if(pProcedureRow != NULL && pProcedureRow->GetPreviousRow() != NULL)
			{
				m_pdlProcedures->PutCurSel(pProcedureRow->GetPreviousRow());
				OnSelChangedProcedureSelect(pProcedureRow, m_pdlProcedures->GetCurSel());
				// (z.manning, 07/11/2007) - We want the last field row.
				pNewFieldRow = m_pdlFields->GetLastRow();
			}
		}

		m_pdlFields->PutCurSel(pNewFieldRow);
		OnSelChangedNexformsFieldSelect(NULL, m_pdlFields->GetCurSel());

	}NxCatchAll("CNexFormsImportProcedureCompareSheet::OnPreviousNexformsField");
}

void CNexFormsImportProcedureCompareSheet::OnNextNexformsField() 
{
	try
	{
		// (z.manning, 10/31/2007) - Make sure to save the previously selected field.
		SaveNewContent(m_pdlFields->GetCurSel());

		IRowSettingsPtr pFieldRow = m_pdlFields->GetCurSel(), pNewFieldRow;
		if(pFieldRow != NULL && pFieldRow->GetNextRow() != NULL)
		{
			// (z.manning, 07/11/2007) - We have a next field row, so select that.
			pNewFieldRow = pFieldRow->GetNextRow();
		}
		else
		{
			// (z.manning, 07/11/2007) - We're on the last field row, go to the next procedure.
			IRowSettingsPtr pProcedureRow = m_pdlProcedures->GetCurSel();
			if(pProcedureRow != NULL && pProcedureRow->GetNextRow() != NULL)
			{
				m_pdlProcedures->PutCurSel(pProcedureRow->GetNextRow());
				OnSelChangedProcedureSelect(pProcedureRow, m_pdlProcedures->GetCurSel());
				pNewFieldRow = m_pdlFields->GetFirstRow();
			}
		}

		m_pdlFields->PutCurSel(pNewFieldRow);
		OnSelChangedNexformsFieldSelect(NULL, m_pdlFields->GetCurSel());

	}NxCatchAll("CNexFormsImportProcedureCompareSheet::OnNextNexformsField");
}

void CNexFormsImportProcedureCompareSheet::RefreshFieldMovementButtons()
{
	// (z.manning, 07/11/2007) - If we're at the first procedure row and first field
	// row, we need to disable the previous button.
	if( m_pdlProcedures->GetCurSel() == m_pdlProcedures->GetFirstRow()
		&& m_pdlFields->GetCurSel() == m_pdlFields->GetFirstRow() )
	{
		GetDlgItem(IDC_PREVIOUS_NEXFORMS_FIELD)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_PREVIOUS_NEXFORMS_FIELD)->EnableWindow(TRUE);
	}

	// (z.manning, 07/11/2007) - If we're at the last procedure row and last field
	// row, we need to disable the next button.
	if( m_pdlProcedures->GetCurSel() == m_pdlProcedures->GetLastRow()
		&& m_pdlFields->GetCurSel() == m_pdlFields->GetLastRow() )
	{
		GetDlgItem(IDC_NEXT_NEXFORMS_FIELD)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_NEXT_NEXFORMS_FIELD)->EnableWindow(TRUE);
	}
}

void CNexFormsImportProcedureCompareSheet::SaveNewContent(LPDISPATCH lpFieldRow)
{
	IRowSettingsPtr pRow(lpFieldRow);
	if(pRow == NULL) {
		return;
	}

	CString *pstrContent = (CString*)VarLong(pRow->GetValue(fcNewContentPointer));
	if(VarBool(pRow->GetValue(fcIsRichTextField))) {
		// (z.manning, 07/11/2007) - This dialog uses NxRichText controls which use NxRichText.
		// However, for NexForms fields we want to store as regular rich text, so we need to convert.
		*pstrContent = ConvertTextFormat(VarString(m_pNewRichText->GetRichText()), tfNxRichText, tfRichText);
	}
	else {
		// (z.manning, 10/10/2007) - PLID 27706 - This is a plain text field, so no need to do any sort of converting.
		GetDlgItemText(IDC_NEW_CONTENT_PLAIN, *pstrContent);
	}
}

void CNexFormsImportProcedureCompareSheet::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	try
	{
		CDialog::OnShowWindow(bShow, nStatus);
	
		// (z.manning, 07/11/2007) - Make sure to save the current content field if this sheet
		// is being hidden.
		if(!bShow) {
			SaveNewContent(m_pdlFields->GetCurSel());
		}

	}NxCatchAll("CNexFormsImportProcedureCompareSheet::OnShowWindow");
}

void CNexFormsImportProcedureCompareSheet::OnOverwriteField()
{
	try
	{
		IRowSettingsPtr pRow = m_pdlFields->GetCurSel();
		if(pRow != NULL) {
			BOOL *bImport = (BOOL*)VarLong(pRow->GetValue(fcImportPointer));
			*bImport = IsDlgButtonChecked(IDC_OVERWRITE_FIELD);

			// (z.manning, 08/03/2007) - There's no point to marking the new content reviewed if they're not
			// going to be importing it, so enable that control based on the overwrite checkbox's value.
			// (z.manning, 10/10/2007) - PLID 27706 - The need review check does not apply to to plain text fields
			// so never enable it for plain text fields.
			if(*bImport && VarBool(pRow->GetValue(fcIsRichTextField))) {
				GetDlgItem(IDC_NEEDS_REVIEW)->EnableWindow(TRUE);
			}
			else {
				GetDlgItem(IDC_NEEDS_REVIEW)->EnableWindow(FALSE);
			}
		}
		else {
			GetDlgItem(IDC_NEEDS_REVIEW)->EnableWindow(FALSE);
		}

	}NxCatchAll("CNexFormsImportProcedureCompareSheet::OnOverwriteField");
}

void CNexFormsImportProcedureCompareSheet::OnNeedsReview()
{
	try
	{
		IRowSettingsPtr pRow = m_pdlFields->GetCurSel();
		if(pRow != NULL) 
		{
			// (z.manning, 10/09/2007) - PLID 27706 - We should not be able to check this button on non-rich text fields.
			ASSERT(VarBool(pRow->GetValue(fcIsRichTextField)));
			BOOL *bNeedReview = (BOOL*)VarLong(pRow->GetValue(fcNeedReviewPointer));
			*bNeedReview = IsDlgButtonChecked(IDC_NEEDS_REVIEW);

			// (z.manning, 08/03/2007) - We're going to remove or add the need review text in this field.
			// There was already code to do this for the NexForms editor, however, it uses a rich edit
			// control instead of an NxRichTextCtrl. So, since there wasn't a good, clean way to do this
			// with NxRichTextCtrl's, I added an invisible rich edit control that I just copy the rich
			// text to then call the code to add or remove the need to review text.
			SaveNewContent(pRow);
			CString strRichText = *((CString*)VarLong(pRow->GetValue(fcNewContentPointer)));
			SetRtfText(&m_ctrlRichEdit, (BYTE*)strRichText.GetBuffer(strRichText.GetLength()), strRichText.GetLength(), SF_RTF);

			// (z.manning, 08/02/2007) - Insert or remove the need review text based on the new value
			// of this checkbox.
			if(*bNeedReview) {
				InsertNeedsReviewedText(&m_ctrlRichEdit);
			}
			else {
				RemoveNeedsReviewedText(&m_ctrlRichEdit);
			}
			
			// (z.manning, 08/03/2007) - Now copy the updated rich text from our invisible rich edit control
			// back to the NxRichText box.
			CString strNewRichText;
			GetRtfText(&m_ctrlRichEdit, strNewRichText, SF_RTF);
			m_pNewRichText->PutRichText(_bstr_t(strNewRichText));
		}

	}NxCatchAll("CNexFormsImportProcedureCompareSheet::OnNeedsReview");
}
