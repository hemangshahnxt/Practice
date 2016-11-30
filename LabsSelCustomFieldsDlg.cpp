// (r.gonet 09/21/2011) - PLID 45584 - Added

// LabsSelCustomFieldsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "LabsSelCustomFieldsDlg.h"
#include "LabsEditCustomFieldDlg.h"
#include "SingleSelectDlg.h"

// (r.gonet 09/21/2011) - PLID 45584 - Some popup menu item IDs
#define ID_NEW_FIELD 100
#define ID_EXISTING_FIELD 101

// (r.gonet 10/21/2011) - PLID 45584 - Added a delay to refreshing the preview to try to cut down on input lag.
#define REFRESH_TIMER_EVENT		22102

using namespace ADODB;

// CLabsSelCustomFieldsDlg dialog

IMPLEMENT_DYNAMIC(CLabsSelCustomFieldsDlg, CNxDialog)

// (r.gonet 09/21/2011) - PLID 45584 - Creates a field selector dialog that is currently pointing at the template associated with the given lab procedure ID.
CLabsSelCustomFieldsDlg::CLabsSelCustomFieldsDlg(long nLabProcedureID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CLabsSelCustomFieldsDlg::IDD, pParent)
{
	m_nLabProcedureID = nLabProcedureID;	
	m_pTemplate = make_shared<CLabProcCFTemplate>();
	// Attempt to load the associated template, if there is one
	if(m_pTemplate->LoadByLabProcedureID(nLabProcedureID)) {
		// We have a template, create an instance of it for the preview.
		m_pTemplateInstance = new CCFTemplateInstance(m_pTemplate);
	} else {
		// This procedure does not have a template associated with it. Show blanks.
		CLabProcCFTemplatePtr pNullTemplate;
		m_pTemplate = pNullTemplate;
		m_pTemplateInstance = NULL;
	}
	// In any case, the preview needs created. It can handle null instances just fine. We don't want values syncing back, so set the edit mode to none.
	m_pdlgPreview = NULL;
}

// (r.gonet 09/21/2011) - PLID 45584 - We created two things dynamically, so delete them both.
CLabsSelCustomFieldsDlg::~CLabsSelCustomFieldsDlg()
{
	if(m_pTemplateInstance) {
		delete m_pTemplateInstance;
	}
	if(m_pdlgPreview) {
		delete m_pdlgPreview;
	}
}

void CLabsSelCustomFieldsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SEL_LAB_CF_TEMPLATE_LABEL, m_nxsTemplateHeader);
	DDX_Control(pDX, IDC_ADD_CF_TEMPLATE_BTN, m_nxbAddTemplate);
	DDX_Control(pDX, IDC_DELETE_CF_TEMPLATE_BTN, m_nxbDeleteTemplate);
	DDX_Control(pDX, IDC_RENAME_CF_TEMPLATE_BTN, m_nxbRenameTemplate);
	DDX_Control(pDX, IDC_CUSTOM_FIELDS_LABEL, m_nxsFieldsHeader);
	DDX_Control(pDX, IDC_ADD_CUSTOM_FIELD_BTN, m_nxbAdd);
	DDX_Control(pDX, IDC_EDIT_CUSTOM_FIELD_BTN, m_nxbEdit);
	DDX_Control(pDX, IDC_REMOVE_CUSTOM_FIELD_BTN, m_nxbRemove);
	DDX_Control(pDX, IDC_FIELD_UP_BTN, m_nxbUp);
	DDX_Control(pDX, IDC_FIELD_DOWN_BUTTON, m_nxbDown);
	DDX_Control(pDX, IDC_FIELDS_PREVIEW_LABEL, m_nxsPreviewHeader);
	DDX_Control(pDX, IDOK, m_nxbClose);
}


BEGIN_MESSAGE_MAP(CLabsSelCustomFieldsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_CUSTOM_FIELD_BTN, &CLabsSelCustomFieldsDlg::OnBnClickedAddCustomFieldBtn)
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_FIELD_BTN, &CLabsSelCustomFieldsDlg::OnBnClickedEditCustomFieldBtn)
	ON_BN_CLICKED(IDC_REMOVE_CUSTOM_FIELD_BTN, &CLabsSelCustomFieldsDlg::OnBnClickedRemoveCustomFieldBtn)
	ON_BN_CLICKED(IDOK, &CLabsSelCustomFieldsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_FIELD_UP_BTN, &CLabsSelCustomFieldsDlg::OnBnClickedFieldUpBtn)
	ON_BN_CLICKED(IDC_FIELD_DOWN_BUTTON, &CLabsSelCustomFieldsDlg::OnBnClickedFieldDownButton)
	ON_BN_CLICKED(IDC_ADD_CF_TEMPLATE_BTN, &CLabsSelCustomFieldsDlg::OnBnClickedAddCfTemplateBtn)
	ON_BN_CLICKED(IDC_DELETE_CF_TEMPLATE_BTN, &CLabsSelCustomFieldsDlg::OnBnClickedDeleteCfTemplateBtn)
	ON_BN_CLICKED(IDC_RENAME_CF_TEMPLATE_BTN, &CLabsSelCustomFieldsDlg::OnBnClickedRenameCfTemplateBtn)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CLabsSelCustomFieldsDlg message handlers

// (r.gonet 09/21/2011) - PLID 45584
BOOL CLabsSelCustomFieldsDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_pTemplateCombo = BindNxDataList2Ctrl(IDC_SEL_LAB_CF_TEMPLATE_COMBO, true);
		m_pTemplateCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

		// Always have the choice of having no template associated with this lab procedure type.
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pTemplateCombo->GetNewRow();
		pNewRow->PutValue(lcftcID, (long)-2);
		pNewRow->PutValue(lcftcName, " <No Custom Fields Template>");
		m_pTemplateCombo->AddRowBefore(pNewRow, m_pTemplateCombo->FindAbsoluteFirstRow(VARIANT_FALSE));
		
		// If we have a template, select it now, otherwise select No Template
		if(m_pTemplate != NULL) {
			m_pTemplateCombo->SetSelByColumn(lcftcID, m_pTemplate->GetID());
		} else {
			m_pTemplateCombo->SetSelByColumn(lcftcID, (long)-2);
		}

		// (r.gonet 09/21/2011) - PLID 45558 - Load the non-deleted fields associated with the selected template.
		m_pFieldsList = BindNxDataList2Ctrl(IDC_CUSTOM_FIELDS_LIST, false);
		m_pFieldsList->FromClause = 
			"LabProcCFTemplateFieldsT "
			"	JOIN "
			"LabCustomFieldsT ON LabProcCFTemplateFieldsT.FieldID = LabCustomFieldsT.ID ";
		m_pFieldsList->WhereClause = _bstr_t(FormatString(
			"LabProcCFTemplateFieldsT.LabProcCFTemplateID IN (SELECT LabProceduresT.LabProcCFTemplateID FROM LabProceduresT WHERE ID = %li) AND LabProcCFTemplateFieldsT.Deleted = 0",
			m_nLabProcedureID));
		m_pFieldsList->Requery();

		m_nxbAddTemplate.AutoSet(NXB_NEW);
		m_nxbDeleteTemplate.AutoSet(NXB_DELETE);
		m_nxbRenameTemplate.AutoSet(NXB_MODIFY);
		m_nxbAdd.AutoSet(NXB_NEW);
		m_nxbEdit.AutoSet(NXB_MODIFY);
		m_nxbRemove.AutoSet(NXB_DELETE);
		m_nxbUp.AutoSet(NXB_UP);
		m_nxbDown.AutoSet(NXB_DOWN);

		// Create the fields preview and move it into position.
		SetRefreshTimer();

		m_nxbClose.AutoSet(NXB_CLOSE);
		EnsureControls();

	} NxCatchAll(__FUNCTION__);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// (r.gonet 10/30/2011) - PLID 45584 - Refreshes the preview window by deleting it and recreating it. I suprisingly found this quicker than just recreating all the
//  controls in the preview window for reasons I wasn't able to explain...
void CLabsSelCustomFieldsDlg::RefreshPreview()
{
	try {
		this->SetWindowText("Lab Custom Field Templates (Refreshing, please wait...)");
		if(m_pdlgPreview) {
			m_pdlgPreview->DestroyWindow();
			delete m_pdlgPreview;
			m_pdlgPreview = NULL;
		}
		m_pdlgPreview = new CLabCustomFieldsView(this, m_pTemplateInstance, CLabCustomFieldControlManager::emNone);
		m_pdlgPreview->Create(IDD_LAB_CUSTOM_FIELDS_VIEW, this);	
		CRect rcRect;
		GetClientRect(&rcRect);
		CRect rcPlaceholder;
		GetDlgItem(IDC_SEL_CUSTOM_FIELDS_PREVIEW_PH)->GetWindowRect(&rcPlaceholder);
		ScreenToClient(&rcPlaceholder);
		m_pdlgPreview->MoveWindow(rcPlaceholder);
		m_pdlgPreview->SetWindowPos(GetDlgItem(IDC_SEL_CUSTOM_FIELDS_PREVIEW_PH), 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		m_pdlgPreview->ShowWindow(SW_SHOW);
		m_pdlgPreview->ShowControls(true);
		this->SetWindowText("Lab Custom Field Templates");
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45584 - Adding can add either a new field or an existing one. Do either.
void CLabsSelCustomFieldsDlg::OnBnClickedAddCustomFieldBtn()
{
	try {
		CMenu menu;
		menu.CreatePopupMenu();
		menu.InsertMenu(-1, MF_BYPOSITION, ID_NEW_FIELD, "New Field...");
		menu.InsertMenu(-1, MF_BYPOSITION, ID_EXISTING_FIELD, "Existing Field...");
		
		long nMenuResult;
		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_ADD_CUSTOM_FIELD_BTN);
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			nMenuResult = menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, rc.right, rc.top, this, NULL);
		} else {
			CPoint pt;
			GetCursorPos(&pt);
			nMenuResult = menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
		}
		
		// Handle the user's menu choice
		switch(nMenuResult) {
			case ID_NEW_FIELD:
				{
					// Add a new field. Let the editor hadle that completely
					CLabsEditCustomFieldDlg dlg(this);
					if(dlg.DoModal() == IDOK) {
						// Now get the field that was created
						CLabCustomFieldPtr pField = dlg.GetField();
						ASSERT(pField != NULL);
						// Create a temporary ID for this template field
						CCFTemplateFieldPtr pTemplateField = make_shared<CCFTemplateField>(pField, m_pTemplate);
						m_pTemplate->AddField(pTemplateField);
						m_pTemplateInstance->AddFieldInstance(new CLabCustomFieldInstance(pTemplateField, m_pTemplateInstance));
						// Now add the new field to the fields datalist
						NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldsList->GetNewRow();
						pRow->PutValue(ecflcID, _variant_t(pTemplateField->GetID()));
						pRow->PutValue(ecflcFieldID, _variant_t(pField->GetID()));
						pRow->PutValue(ecflcSortOrder, _variant_t(m_pFieldsList->GetRowCount()));
						pRow->PutValue(ecflcName, _variant_t(pField->GetName()));
						pRow->PutValue(ecflcDisplayName, _variant_t(pField->GetFriendlyName()));
						pRow->PutValue(ecflcRequired, _variant_t(FALSE));
						m_pFieldsList->AddRowAtEnd(pRow, NULL);
						SetRefreshTimer();
						m_pFieldsList->SetSelByColumn(ecflcID, _variant_t(pTemplateField->GetID()));
						EnsureControls();
					}
				}
				break;
			case ID_EXISTING_FIELD:
				{
					// Add an existing field. Let the user select from them.
					CString strQuery = "(SELECT ID, Name FROM LabCustomFieldsT) SubQ";
					CSingleSelectDlg dlg(this);
					if(IDOK == dlg.Open(strQuery, "", "ID", "Name", "Please select an existing lab custom field from the drop down box below.", true)) {
						long nFieldID = dlg.GetSelectedID();
						NXDATALIST2Lib::IRowSettingsPtr pRowIter = m_pFieldsList->GetFirstRow();
						while(pRowIter) {
							long nIterFieldID = VarLong(pRowIter->GetValue(ecflcFieldID));
							if(nFieldID == nIterFieldID) {
								MessageBox("This field already exists on this template. Duplicates aren't allowed.", "Duplicate Field");
								return;
							}
							pRowIter = pRowIter->GetNextRow();
						}
						// Alright, so it is a new field. Load it and add it to the template.
						CLabCustomFieldPtr pField;
						pField = CLabCustomField::Load(nFieldID);
						CCFTemplateFieldPtr pTemplateField = make_shared<CCFTemplateField>(pField, m_pTemplate);
						m_pTemplate->AddField(pTemplateField);
						m_pTemplateInstance->AddFieldInstance(new CLabCustomFieldInstance(pTemplateField, m_pTemplateInstance));
						// Now add the field to the datalist.
						NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldsList->GetNewRow();
						pRow->PutValue(ecflcID, pTemplateField->GetID());
						pRow->PutValue(ecflcFieldID, _variant_t(pField->GetID()));
						pRow->PutValue(ecflcSortOrder, _variant_t(m_pFieldsList->GetRowCount()));
						pRow->PutValue(ecflcName, _variant_t(pField->GetName()));
						pRow->PutValue(ecflcDisplayName, _variant_t(pField->GetFriendlyName()));
						pRow->PutValue(ecflcRequired, _variant_t(FALSE));
						m_pFieldsList->AddRowAtEnd(pRow, NULL);
						SetRefreshTimer();
						m_pFieldsList->SetSelByColumn(ecflcID, _variant_t(pTemplateField->GetID()));
						EnsureControls();
					}
				}
				break;
			default:
				// No choice or not handled
				break;

		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45584 - Edit an existing field
void CLabsSelCustomFieldsDlg::OnBnClickedEditCustomFieldBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldsList->CurSel;
		if(pRow) {
			long nFieldID = VarLong(pRow->GetValue(ecflcFieldID));
			if(nFieldID <= 0) {
				ThrowNxException("CLabsSelCustomFieldsDlg::OnBnClickedEditCustomFieldBtn : Attempted to edit a field with ID < 0.");
			}
			// Open the editor for the selected field.
			CLabsEditCustomFieldDlg dlg(nFieldID, this);
			if(dlg.DoModal() == IDOK) {
				CLabCustomFieldPtr pField = dlg.GetField();
				ASSERT(pField != NULL);
				long nTemplateFieldID = VarLong(pRow->GetValue(ecflcID));
				// Replace what we have in the template with whatever the editor came up with.
				m_pTemplate->ReplaceField(nTemplateFieldID, pField);
				// Clear out the instance value, it is junk anyway
				CLabCustomFieldInstance *pFieldInstance = m_pTemplateInstance->GetFieldInstanceByFieldID(pField->GetID());
				_variant_t varNull = g_cvarNull;
				pFieldInstance->SetValue(varNull);
				// Update the visible datalist rows.
				pRow->PutValue(ecflcName, _variant_t(pField->GetName()));
				pRow->PutValue(ecflcDisplayName, _variant_t(pField->GetFriendlyName()));
				SetRefreshTimer();
			}
		} else {
			MessageBox("Please select a field to edit.");
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45584 - Removes a field from the template
void CLabsSelCustomFieldsDlg::OnBnClickedRemoveCustomFieldBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldsList->CurSel;
		if(pRow) {
			long nTemplateFieldID = VarLong(pRow->GetValue(ecflcID));
			// Remove the selected field from the template
			m_pTemplate->RemoveField(nTemplateFieldID);
			NXDATALIST2Lib::IRowSettingsPtr pSiblingRow = pRow->GetNextRow();
			if(pSiblingRow == NULL) {
				pSiblingRow = pRow->GetPreviousRow();
			}
			// Remove the row associated with the field.
			m_pFieldsList->RemoveRow(pRow);
			SetRefreshTimer();
			EnsureControls();
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45584 - Saves the template and closes the dialog.
void CLabsSelCustomFieldsDlg::OnBnClickedOk()
{
	try {
		// We need to save the custom fields at this point
		if(m_pTemplate != NULL) {
			if(!SaveTemplate()) {
				// The user has cancelled saving
				return;
			}
		}
		// Did the user select a different template to use with this procedure type?
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT LabProcCFTemplateID FROM LabProceduresT WHERE ID = {INT}; ", m_nLabProcedureID);
		long nOldTemplateID = VarLong(prs->Fields->Item["LabProcCFTemplateID"]->Value, -2);
		long nNewTemplateID = -2;
		if(m_pTemplate != NULL) {
			nNewTemplateID = m_pTemplate->GetID();
		}
		if(nOldTemplateID != nNewTemplateID) {	
			
			CLabProcCFTemplate cfTemplate;

			// They selected another template to use. Use that from now on.
			ExecuteParamSql(
				"UPDATE LabProceduresT SET LabProcCFTemplateID = {VT_I4} WHERE ID = {INT}; ",
				m_pTemplate != NULL ? _variant_t(m_pTemplate->GetID()) : g_cvarNull, m_nLabProcedureID);
		}
		CNxDialog::OnOK();
	} NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CLabsSelCustomFieldsDlg, CNxDialog)
	ON_EVENT(CLabsSelCustomFieldsDlg, IDC_CUSTOM_FIELDS_LIST, 10, CLabsSelCustomFieldsDlg::EditingFinishedCustomFieldsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CLabsSelCustomFieldsDlg, IDC_CUSTOM_FIELDS_LIST, 2, CLabsSelCustomFieldsDlg::SelChangedCustomFieldsList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CLabsSelCustomFieldsDlg, IDC_SEL_LAB_CF_TEMPLATE_COMBO, 16, CLabsSelCustomFieldsDlg::SelChosenSelLabCfTemplateCombo, VTS_DISPATCH)
	ON_EVENT(CLabsSelCustomFieldsDlg, IDC_CUSTOM_FIELDS_LIST, 3, CLabsSelCustomFieldsDlg::OnDblClickCellCustomFieldsList, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

// (r.gonet 09/21/2011) - PLID 45584 - The user can edit the required property for a template field in place.
void CLabsSelCustomFieldsDlg::EditingFinishedCustomFieldsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			switch(nCol) {
				case ecflcRequired:
					{
						CCFTemplateFieldPtr pTemplateField = m_pTemplate->GetTemplateField(pRow->GetValue(ecflcID));
						if(!pTemplateField) {
							ThrowNxException("CLabsSelCustomFieldsDlg::EditingFinishedCustomFieldsList : Template out of sync with list. Template field is in list but not in template.");
						}
						pTemplateField->SetRequired(VarBool(varNewValue) != FALSE);
						SetRefreshTimer();
						m_pFieldsList->EnsureRowInView(pRow);
					}
					break;
				default:
					break;
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45584 - Move a template field up one in sort order
void CLabsSelCustomFieldsDlg::OnBnClickedFieldUpBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldsList->CurSel;
		if(pRow) {
			NXDATALIST2Lib::IRowSettingsPtr pPrevRow = pRow->GetPreviousRow();
			if(pPrevRow == NULL){
				//we're already the first row
				return;
			}
			
			m_pFieldsList->RemoveRow(pRow);
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pFieldsList->AddRowBefore(pRow, pPrevRow);
			m_pFieldsList->CurSel = pNewRow;
			// Ensure our selected row is in view.
			m_pFieldsList->EnsureRowInView(pNewRow);

			// Now swap the sort orders within the items
			CCFTemplateFieldPtr pSelectedTemplateField = m_pTemplate->GetTemplateField(pNewRow->GetValue(ecflcID));
			CCFTemplateFieldPtr pPrevTemplateField = m_pTemplate->GetTemplateField(pPrevRow->GetValue(ecflcID));
			long nTempSortOrder = pSelectedTemplateField->GetSortOrder();
			pSelectedTemplateField->SetSortOrder(pPrevTemplateField->GetSortOrder());
			pPrevTemplateField->SetSortOrder(nTempSortOrder);
			// We now set a timer that when it expires will refresh the preview. This is to allow the user to rapidly click this button
			SetRefreshTimer();
			EnsureControls();
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45584 - Move a template field down one in sort order
void CLabsSelCustomFieldsDlg::OnBnClickedFieldDownButton()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldsList->CurSel;
		if(pRow) {
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = pRow->GetNextRow();
			if(pNextRow == NULL){
				//we're already the first row
				return;
			}
			
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pFieldsList->AddRowBefore(pNextRow, pRow);
			m_pFieldsList->CurSel = pRow;
			m_pFieldsList->RemoveRow(pNextRow);
			// Ensure our selected row is in view.
			m_pFieldsList->EnsureRowInView(pRow);


			// Now swap the sort orders within the items
			CCFTemplateFieldPtr pSelectedTemplateField = m_pTemplate->GetTemplateField(pRow->GetValue(ecflcID));
			CCFTemplateFieldPtr pPrevTemplateField = m_pTemplate->GetTemplateField(pNewRow->GetValue(ecflcID));
			long nTempSortOrder = pSelectedTemplateField->GetSortOrder();
			pSelectedTemplateField->SetSortOrder(pPrevTemplateField->GetSortOrder());
			pPrevTemplateField->SetSortOrder(nTempSortOrder);
			// We now set a timer that when it expires will refresh the preview. This is to allow the user to rapidly click this button
			SetRefreshTimer();
			EnsureControls();
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45584 - Enable or disable controls based on current selections.
void CLabsSelCustomFieldsDlg::EnsureControls()
{
	if(m_pTemplate == NULL) {
		m_pFieldsList->Enabled = VARIANT_FALSE;
		m_nxbDeleteTemplate.EnableWindow(FALSE);
		m_nxbRenameTemplate.EnableWindow(FALSE);
		m_nxbAdd.EnableWindow(FALSE);
		m_nxbEdit.EnableWindow(FALSE);
		m_nxbRemove.EnableWindow(FALSE);
		m_nxbUp.EnableWindow(FALSE);
		m_nxbDown.EnableWindow(FALSE);
	} else {
		m_pFieldsList->Enabled = VARIANT_TRUE;
		m_nxbDeleteTemplate.EnableWindow(TRUE);
		m_nxbRenameTemplate.EnableWindow(TRUE);
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldsList->CurSel;
		if(pRow) {
			m_nxbAdd.EnableWindow(TRUE);
			m_nxbEdit.EnableWindow(TRUE);
			m_nxbRemove.EnableWindow(TRUE);
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = pRow->GetNextRow();
			m_nxbDown.EnableWindow(pNextRow != NULL ? TRUE : FALSE);
			NXDATALIST2Lib::IRowSettingsPtr pPrevRow = pRow->GetPreviousRow();
			m_nxbUp.EnableWindow(pPrevRow != NULL ? TRUE : FALSE);
		} else {
			m_nxbAdd.EnableWindow(TRUE);
			m_nxbEdit.EnableWindow(FALSE);
			m_nxbRemove.EnableWindow(FALSE);
			m_nxbDown.EnableWindow(FALSE);
			m_nxbUp.EnableWindow(FALSE);
		}
	}
}

// (r.gonet 09/21/2011) - PLID 45584 - Saves a template. Returns true if successful at saving. False otherwise.
bool CLabsSelCustomFieldsDlg::SaveTemplate()
{
	if(m_pTemplate == NULL) {
		// No template selected? No problem.
		return true;
	}

	// We've been adding new template fields here without consequence but now they need to be saved. We've been using temporary IDs for them so far,
	//  so we need to update our copy of the ID in the datalist rows.
	std::map<long, CCFTemplateFieldPtr> mapNewTemplateFields;
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldsList->GetFirstRow();
	while(pRow) {
		long nTemplateFieldID = pRow->GetValue(ecflcID);
		if(nTemplateFieldID < 0) {
			// This will be finalized in the save, so add it to the list of fields to update our rows with in our list.
			CCFTemplateFieldPtr pTemplateField = m_pTemplate->GetTemplateField(nTemplateFieldID);
			mapNewTemplateFields[nTemplateFieldID] = pTemplateField;
		}
		pRow = pRow->GetNextRow();
	}

	// Save the template
	m_pTemplate->Save();

	// IDs for new template fields may have been finalized by the save.
	std::map<long, CCFTemplateFieldPtr>::iterator iter;
	for(iter = mapNewTemplateFields.begin(); iter != mapNewTemplateFields.end(); iter++) {
		long nOldTemplateFieldID = iter->first;
		CCFTemplateFieldPtr pTemplateField = iter->second;
		if(nOldTemplateFieldID != pTemplateField->GetID()) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldsList->FindByColumn(ecflcID, nOldTemplateFieldID, m_pFieldsList->GetFirstRow(), VARIANT_FALSE);
			if(pRow) {
				// Exchange our old temporary ID with the new finalized ID
				pRow->PutValue(ecflcID, (long)pTemplateField->GetID());
			}
		}
	}

	return true;
}

// (r.gonet 09/21/2011) - PLID 45584 - The current row selection was changed in the fields list, ensure our controls.
void CLabsSelCustomFieldsDlg::SelChangedCustomFieldsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		EnsureControls();

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldsList->CurSel;
		if(pRow) {
			EnsureFieldInView(pRow);
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45584 - The user chose a new template, save the current one we are editing and switch to the next one.
void CLabsSelCustomFieldsDlg::SelChosenSelLabCfTemplateCombo(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if(!pRow) {
			// They haven't selected a template, go to the <No Template> entry.
			pRow = m_pTemplateCombo->SetSelByColumn(lcftcID, _variant_t((long)-2));
			if(!pRow) {
				// How doesn't this exist?
				ThrowNxException("CLabsSelCustomFieldsDlg::SelChosenSelLabCfTemplateCombo : Missing template in template combo box.");
			}
		}
		// What is the new template they chose?
		long nTemplateID = pRow->GetValue(lcftcID);

		// Save our old template
		if(m_pTemplate != NULL) {
			SaveTemplate();
			delete m_pTemplateInstance; // and get rid of its instance
			m_pTemplateInstance = NULL;
		}

		if(nTemplateID > 0) {
			// Load the new template and update the selection query in the datalist to get fields for that now.
			m_pTemplate = make_shared<CLabProcCFTemplate>();
			m_pTemplate->Load(nTemplateID);
			// (r.gonet 09/21/2011) - PLID 45558 - Don't load deleted template fields.
			m_pFieldsList->WhereClause = _bstr_t(FormatString(
				"LabProcCFTemplateFieldsT.LabProcCFTemplateID = %li AND LabProcCFTemplateFieldsT.Deleted = 0",
				m_pTemplate->GetID()));
			m_pTemplateInstance = new CCFTemplateInstance(m_pTemplate);
			m_pFieldsList->Requery();
		} else {
			// They chose no template. So don't load anything.
			CLabProcCFTemplatePtr pNullTemplate;
			m_pTemplate = pNullTemplate;
			m_pFieldsList->Clear();
		}
		// In any case, refresh the fields preview.
		SetRefreshTimer();
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45584 - Add a new template and switch to that.
void CLabsSelCustomFieldsDlg::OnBnClickedAddCfTemplateBtn()
{
	try {
		CString strNewName;
		if (IDOK == (InputBoxLimited(this, "Please enter a name for the new template.", strNewName, "", 255, false, false, NULL))) {
			if(m_pTemplate != NULL) {
				SaveTemplate();
			}
			
			// Check for blanks and duplicates
			if(strNewName.IsEmpty()) {
				MessageBox("All templates must have names.", "Error", MB_ICONERROR);
				return;
			}
			// (r.gonet 12/09/2011) - PLID 45558 - We don't care if deleted templates have the same name.
			if(ReturnsRecordsParam("SELECT Name FROM LabProcCFTemplatesT WHERE Name LIKE {STRING} AND Deleted = 0", strNewName)) {
				MessageBox("A template already exists with that name. All templates must have unique names.", "Error", MB_ICONERROR);
				return;
			}
			
			// Save the old template we were on
			if(m_pTemplate != NULL) {
				delete m_pTemplateInstance;
				m_pTemplateInstance = NULL;
			}

			// Create a new template by the given name
			m_pTemplate = make_shared<CLabProcCFTemplate>();
			m_pTemplate->SetName(strNewName);
			SaveTemplate();
			m_pTemplateInstance = new CCFTemplateInstance(m_pTemplate);

			// Add it to the template combo and select it.
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pTemplateCombo->GetNewRow();
			pNewRow->PutValue(lcftcID, _variant_t(m_pTemplate->GetID()));
			pNewRow->PutValue(lcftcName, _variant_t(strNewName));
			m_pTemplateCombo->AddRowSorted(pNewRow, NULL);
			m_pTemplateCombo->SetSelByColumn(lcftcID, m_pTemplate->GetID());

			// There will initally be no fields.
			m_pFieldsList->Clear();
			SetRefreshTimer();
			EnsureControls();
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45584 - Delete the current template and select No Template
void CLabsSelCustomFieldsDlg::OnBnClickedDeleteCfTemplateBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTemplateCombo->CurSel;
		if(pRow == NULL || VarLong(pRow->GetValue(lcftcID), -1) <= 0) {
			MessageBox("You must select a template to delete.");
			return;
		}
		// Delete the old template
		long nTemplateID = VarLong(pRow->GetValue(lcftcID));
		m_pTemplate->SetModificationStatus(CLabProcCFTemplate::emsDeleted);
		SaveTemplate();
		delete m_pTemplateInstance;
		m_pTemplateInstance = NULL;

		// Set a new one as the default No Template Selection
		CLabProcCFTemplatePtr pNullTemplate;
		m_pTemplate = pNullTemplate;
		m_pFieldsList->Clear();
		m_pTemplateCombo->RemoveRow(pRow);
		m_pTemplateCombo->SetSelByColumn(lcftcID, _variant_t((long)-2));

		SetRefreshTimer();
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45584 - Rename the current template, which is easy.
void CLabsSelCustomFieldsDlg::OnBnClickedRenameCfTemplateBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTemplateCombo->CurSel;
		if(pRow == NULL || VarLong(pRow->GetValue(lcftcID), -1) <= 0) {
			MessageBox("You must select a template to rename.");
			return;
		}

		CString strNewName;
		if (IDOK == (InputBoxLimited(this, "Please enter a new name for the template.", strNewName, "",255,false,false,NULL))) {
			// Check for blanks and duplicates
			if(strNewName.IsEmpty()) {
				MessageBox("All templates must have names.", "Error", MB_ICONERROR);
				return;
			}
			// (r.gonet 12/09/2011) - PLID 45558 - We don't care if deleted templates have the same name.
			if(ReturnsRecordsParam("SELECT Name FROM LabProcCFTemplatesT WHERE Name LIKE {STRING} AND ID <> {INT} AND Deleted = 0 ", strNewName, m_pTemplate->GetID())) {
				MessageBox("A template already exists with that name. All templates must have unique names.", "Error", MB_ICONERROR);
				return;
			}

			m_pTemplate->SetName(strNewName);
			pRow->PutValue(lcftcName, _variant_t(strNewName));
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 10/30/2011) - PLID 45584 - Starts a countdown to the refresh event. If the timer is currently running, it is reset.
//  Use is to give the user some time to rapidly click on Required or Up/Down without the preview refreshing each time.
void CLabsSelCustomFieldsDlg::SetRefreshTimer()
{
	// Reset the timer
	KillTimer(REFRESH_TIMER_EVENT);
	SetTimer(REFRESH_TIMER_EVENT, 1000, NULL);
}

// (r.gonet 10/30/2011) - PLID 45584 - Refreshes the fields preview.
void CLabsSelCustomFieldsDlg::OnTimer(UINT nEventID)
{
	if (nEventID == REFRESH_TIMER_EVENT)
	{
		KillTimer(REFRESH_TIMER_EVENT);
		try {		
			RefreshPreview();
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldsList->CurSel;
			if(pRow && m_pdlgPreview) {
				long nTemplateFieldID = VarLong(pRow->GetValue(ecflcID));
				CCFTemplateFieldPtr pTemplateField = m_pTemplate->GetTemplateField(nTemplateFieldID);
				if(pTemplateField) {
					CLabCustomFieldPtr pField = pTemplateField->GetField();
					m_pdlgPreview->EnsureFieldInView(pField);
				}
			}
		} NxCatchAll(__FUNCTION__);
	}
	
	CNxDialog::OnTimer(nEventID);
}

// Opens the editor on the current row
void CLabsSelCustomFieldsDlg::OnDblClickCellCustomFieldsList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			if(nColIndex == ecflcName || nColIndex == ecflcDisplayName) {
				OnBnClickedEditCustomFieldBtn();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 10/30/2011) - PLID 45555 - Fields in the preview can now scroll into view.
void CLabsSelCustomFieldsDlg::EnsureFieldInView(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	if(pRow && m_pdlgPreview) {
		long nTemplateFieldID = VarLong(pRow->GetValue(ecflcID));
		CCFTemplateFieldPtr pTemplateField = m_pTemplate->GetTemplateField(nTemplateFieldID);
		if(pTemplateField) {
			CLabCustomFieldPtr pField = pTemplateField->GetField();
			m_pdlgPreview->EnsureFieldInView(pField);
		}
	}
}