// (r.gonet 09/21/2011) - PLID 45585 - Added

// LabsEditCustomFieldDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "LabsEditCustomFieldDlg.h"
#include "LabCustomField.h"
#include "LabCustomFieldListItemEditor.h"

using namespace ADODB;

#define ADD_TYPE_ROW(pList, nId, strName) \
	pRow = (pList)->GetNewRow(); \
	pRow->PutValue(lftID, _variant_t((long)nId)); \
	pRow->PutValue(lftName, _variant_t(strName)); \
	(pList)->AddRowSorted(pRow, NULL)

// CLabsEditCustomFieldDlg dialog

IMPLEMENT_DYNAMIC(CLabsEditCustomFieldDlg, CNxDialog)

// (r.gonet 09/21/2011) - PLID 45585 - Creates a new editor dialog with a new field.
CLabsEditCustomFieldDlg::CLabsEditCustomFieldDlg(CWnd* pParent /*=NULL*/)
: CNxDialog(CLabsEditCustomFieldDlg::IDD, pParent), m_cfTemplateInstance(make_shared<CLabProcCFTemplate>()), m_dlgPreview(this, &m_cfTemplateInstance, CLabCustomFieldControlManager::emDefaultValues)
{
	// We are being asked to edit a new field, so create it now.
	m_nFieldID = -1;
}

// (r.gonet 09/21/2011) - PLID 45585 - Creates a new editor dialog with an existing field.
CLabsEditCustomFieldDlg::CLabsEditCustomFieldDlg(long nFieldID, CWnd* pParent /*=NULL*/)
: CNxDialog(CLabsEditCustomFieldDlg::IDD, pParent), m_cfTemplateInstance(make_shared<CLabProcCFTemplate>()), m_dlgPreview(this, &m_cfTemplateInstance, CLabCustomFieldControlManager::emDefaultValues)
{
	m_nFieldID = nFieldID;
}

// (r.gonet 09/21/2011) - PLID 45585 - Empty virtual destructor
CLabsEditCustomFieldDlg::~CLabsEditCustomFieldDlg()
{
}

void CLabsEditCustomFieldDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_COMMON_COLOR, m_nxcCommonProperties);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_NAME_LABEL, m_nxstaticName);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_NAME_EDIT, m_nxeName);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_DISPNAME_LABEL, m_nxstaticDisplayName);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_DISPNAME_EDIT, m_nxeDisplayName);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_DESCRIPTION_LABEL, m_nxstaticDescription);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_DESC_EDIT, m_nxeDescription);

	DDX_Control(pDX, IDC_CUSTOM_FIELD_TYPE_COLOR, m_nxcFieldType);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_TYPE_LABEL, m_nxstaticFieldType);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_DISPTYPE_LABEL, m_nxstaticDisplayType);

	DDX_Control(pDX, IDC_CUSTOM_FIELD_PROPERTIES_COLOR, m_nxcOtherProperties);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_OTHERPROPS_LABEL, m_nxstaticOtherProperties);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_WIDTH_LABEL, m_nxstaticWidth);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_WIDTH_EDIT, m_nxeWidth);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_HEIGHT_LABEL, m_nxstaticHeight);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_HEIGHT_EDIT, m_nxeHeight);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_MAX_CHARS_LABEL, m_nxstaticMaxLength);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_MAX_CHARS_EDIT, m_nxeMaxLength);

	DDX_Control(pDX, IDC_CUSTOM_FIELD_PREVIEW_COLOR, m_nxcFieldPreview);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_PREVIEW_LABEL, m_nxstaticFieldPreview);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_DEFAULT_LABEL, m_nxstaticDefault);

	DDX_Control(pDX, IDC_CUSTOM_FIELD_LISTITEMS_COLOR, m_nxcListItems);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_LISTITEMS_LABEL, m_nxstaticListItems);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_LIST_ADD_BTN, m_btnAddListItem);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_LIST_EDIT_BTN, m_btnEditListItem);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_LIST_DELETE_BTN, m_btnDeleteListItem);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_UP_BTN, m_btnMoveListItemUp);
	DDX_Control(pDX, IDC_CUSTOM_FIELD_DOWN_BTN, m_btnMoveListItemDown);
	DDX_Control(pDX, IDC_LCF_EDIT_AUTO_SIZE_RADIO, m_radioAutoSize);
	DDX_Control(pDX, IDC_LCF_EDIT_FIXED_SIZE_RADIO, m_radioFixedSize);

	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);

	DDX_Control(pDX, IDC_COMMON_FIELD_HELP_BTN, m_btnCommonPropertiesHelp);
	DDX_Control(pDX, IDC_FIELD_TYPE_HELP_BTN, m_btnFieldTypeHelp);
	DDX_Control(pDX, IDC_DISPLAY_TYPE_HELP_BTN, m_btnDisplayTypeHelp);
	DDX_Control(pDX, IDC_OTHER_PROPERTIES_HELP_BTN, m_btnOtherPropertiesHelp);
	DDX_Control(pDX, IDC_FIELD_PREVIEW_HELP_BTN, m_btnFieldPreviewHelp);
	DDX_Control(pDX, IDC_LIST_ITEMS_HELP_BTN, m_btnListItemsHelp);
}


BEGIN_MESSAGE_MAP(CLabsEditCustomFieldDlg, CNxDialog)
	ON_EN_KILLFOCUS(IDC_CUSTOM_FIELD_NAME_EDIT, &CLabsEditCustomFieldDlg::OnEnKillfocusCustomFieldNameEdit)
	ON_EN_KILLFOCUS(IDC_CUSTOM_FIELD_DISPNAME_EDIT, &CLabsEditCustomFieldDlg::OnEnKillfocusCustomFieldDispnameEdit)
	ON_EN_KILLFOCUS(IDC_CUSTOM_FIELD_DESC_EDIT, &CLabsEditCustomFieldDlg::OnEnKillfocusCustomFieldDescEdit)
	ON_EN_KILLFOCUS(IDC_CUSTOM_FIELD_WIDTH_EDIT, &CLabsEditCustomFieldDlg::OnEnKillfocusCustomFieldWidthEdit)
	ON_EN_KILLFOCUS(IDC_CUSTOM_FIELD_HEIGHT_EDIT, &CLabsEditCustomFieldDlg::OnEnKillfocusCustomFieldHeightEdit)
	ON_EN_KILLFOCUS(IDC_CUSTOM_FIELD_MAX_CHARS_EDIT, &CLabsEditCustomFieldDlg::OnEnKillfocusCustomFieldMaxCharsEdit)
	ON_BN_CLICKED(IDC_CUSTOM_FIELD_LIST_ADD_BTN, &CLabsEditCustomFieldDlg::OnBnClickedCustomFieldListAddBtn)
	ON_BN_CLICKED(IDC_CUSTOM_FIELD_LIST_EDIT_BTN, &CLabsEditCustomFieldDlg::OnBnClickedCustomFieldListEditBtn)
	ON_BN_CLICKED(IDC_CUSTOM_FIELD_LIST_DELETE_BTN, &CLabsEditCustomFieldDlg::OnBnClickedCustomFieldListDeleteBtn)
	ON_BN_CLICKED(IDC_CUSTOM_FIELD_UP_BTN, &CLabsEditCustomFieldDlg::OnBnClickedCustomFieldUpBtn)
	ON_BN_CLICKED(IDC_CUSTOM_FIELD_DOWN_BTN, &CLabsEditCustomFieldDlg::OnBnClickedCustomFieldDownBtn)
	ON_BN_CLICKED(IDOK, &CLabsEditCustomFieldDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CLabsEditCustomFieldDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_LCF_EDIT_AUTO_SIZE_RADIO, &CLabsEditCustomFieldDlg::OnBnClickedLcfEditAutoSizeRadio)
	ON_BN_CLICKED(IDC_LCF_EDIT_FIXED_SIZE_RADIO, &CLabsEditCustomFieldDlg::OnBnClickedLcfEditFixedSizeRadio)
	ON_BN_CLICKED(IDC_COMMON_FIELD_HELP_BTN, &CLabsEditCustomFieldDlg::OnBnClickedCommonFieldHelpBtn)
	ON_BN_CLICKED(IDC_FIELD_TYPE_HELP_BTN, &CLabsEditCustomFieldDlg::OnBnClickedFieldTypeHelpBtn)
	ON_BN_CLICKED(IDC_DISPLAY_TYPE_HELP_BTN, &CLabsEditCustomFieldDlg::OnBnClickedDisplayTypeHelpBtn)
	ON_BN_CLICKED(IDC_OTHER_PROPERTIES_HELP_BTN, &CLabsEditCustomFieldDlg::OnBnClickedOtherPropertiesHelpBtn)
	ON_BN_CLICKED(IDC_FIELD_PREVIEW_HELP_BTN, &CLabsEditCustomFieldDlg::OnBnClickedFieldPreviewHelpBtn)
	ON_BN_CLICKED(IDC_LIST_ITEMS_HELP_BTN, &CLabsEditCustomFieldDlg::OnBnClickedListItemsHelpBtn)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CLabsEditCustomFieldDlg, CNxDialog)
	ON_EVENT(CLabsEditCustomFieldDlg, IDC_CUSTOM_FIELD_TYPE_COMBO, 16, CLabsEditCustomFieldDlg::OnSelChosenCustomFieldTypeCombo, VTS_DISPATCH)
	ON_EVENT(CLabsEditCustomFieldDlg, IDC_CUSTOM_FIELD_DISPTYPE_COMBO, 16, CLabsEditCustomFieldDlg::OnSelChosenCustomFieldDisptypeCombo, VTS_DISPATCH)
	ON_EVENT(CLabsEditCustomFieldDlg, IDC_CUSTOM_FIELD_LIST_ITEMS, 2, CLabsEditCustomFieldDlg::SelChangedCustomFieldListItems, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CLabsEditCustomFieldDlg, IDC_CUSTOM_FIELD_DISPTYPE_COMBO, 1, CLabsEditCustomFieldDlg::SelChangingCustomFieldDisptypeCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CLabsEditCustomFieldDlg, IDC_CUSTOM_FIELD_TYPE_COMBO, 1, CLabsEditCustomFieldDlg::SelChangingCustomFieldTypeCombo, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()


// CLabsEditCustomFieldDlg message handlers

// (r.gonet 09/21/2011) - PLID 45585
BOOL CLabsEditCustomFieldDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		// From the field ID given, load a lab custom field or create a new one.
		if(m_nFieldID == -1) {
			m_pField = make_shared<CLabCustomEmptyField>();
			m_pField->SetModificationStatus(CLabCustomField::emsNew);
		} else {
			m_pField = CLabCustomField::Load(m_nFieldID);
		}
		// The preview requires a template and an instance of a template, so create some dummy ones here.
		m_pTemplateField = make_shared<CCFTemplateField>(m_pField, m_cfTemplateInstance.GetTemplate());
		m_cfTemplateInstance.GetTemplate()->AddField(m_pTemplateField);

		m_pFieldTypeCombo = BindNxDataList2Ctrl(this, IDC_CUSTOM_FIELD_TYPE_COMBO, GetRemoteData(), false);
		m_pDisplayTypeCombo = BindNxDataList2Ctrl(this, IDC_CUSTOM_FIELD_DISPTYPE_COMBO, GetRemoteData(), false);
		// (r.gonet 09/21/2011) - PLID 45583
		m_pCustomListItems = BindNxDataList2Ctrl(this, IDC_CUSTOM_FIELD_LIST_ITEMS, GetRemoteData(), false);

		// (r.gonet 09/21/2011) - PLID 45583
		m_btnAddListItem.AutoSet(NXB_NEW);
		m_btnEditListItem.AutoSet(NXB_MODIFY);
		m_btnDeleteListItem.AutoSet(NXB_DELETE);
		m_btnMoveListItemUp.AutoSet(NXB_UP);
		m_btnMoveListItemDown.AutoSet(NXB_DOWN);
		
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// Cap the text length at our database field sizes
		m_nxeName.SetLimitText(255);
		m_nxeDisplayName.SetLimitText(255);
		m_nxeDescription.SetLimitText(512);
		// Don't let them enter in too big of dimensions.
		m_nxeWidth.SetLimitText(4);
		m_nxeHeight.SetLimitText(4);

		// Load the field's values into the editor.
		LoadControlValues();
		// Disable any unapplicable controls.
		EnsureControls();
		
	} NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// (r.gonet 09/21/2011) - PLID 45585 - Load the edited field's values into the editor.
void CLabsEditCustomFieldDlg::LoadControlValues()
{
	// The most common stuff first, name and description
	if(m_pField) {
		m_nxeName.SetWindowText(_bstr_t(m_pField->GetName()));
		m_nxeDisplayName.SetWindowText(_bstr_t(m_pField->GetFriendlyName()));
		m_nxeDescription.SetWindowText(_bstr_t(m_pField->GetDescription()));
	}

	// Load the types into the type selection box
	LoadFieldTypeCombo();
	// Load the display types, which is dependent on thes selection in the field type combo.
	LoadDisplayTypeCombo();

	// Whether the user or Practice determines the control size.
	if(m_pField->GetFieldType() != lcftBool && 
		!(m_pField->GetFieldType() == lcftSingleSelectList && (m_pField->GetDisplayType() == lcdtDefault || m_pField->GetDisplayType() == lcdtRadioButtons)))
	{
		m_radioAutoSize.SetCheck(m_pField->GetSizeMode() == CLabCustomField::cfsmAutoSize);
		m_radioFixedSize.SetCheck(m_pField->GetSizeMode() == CLabCustomField::cfsmFixedSize);
		if(m_pField->GetSizeMode() == CLabCustomField::cfsmFixedSize) {
			m_nxeWidth.SetWindowText(FormatString("%li", m_pField->GetWidth()));
			m_nxeHeight.SetWindowText(FormatString("%li", m_pField->GetHeight()));
		}
	} else {
		m_radioAutoSize.SetCheck(TRUE);
		m_radioFixedSize.SetCheck(FALSE);
		m_nxeWidth.SetWindowText("");
		m_nxeHeight.SetWindowText("");
		m_pField->SetSizeMode(CLabCustomField::cfsmAutoSize);
	}

	// (r.gonet 09/21/2011) - PLID 45582 - If a text field, load the maximum character length.
	if(m_pField->GetFieldType() == lcftText || m_pField->GetFieldType() == lcftMemo) {
		boost::shared_ptr<CLabCustomTextField> pTextField = boost::dynamic_pointer_cast<CLabCustomTextField>(m_pField);
		if(pTextField == NULL) {
			ThrowNxException("CLabsEditCustomFieldDlg::LoadControlValues : Could not cast the edited field as a text field.");
		}
		m_nxeMaxLength.SetWindowText(FormatString("%li", pTextField->GetMaxLength()));
	} else {
		m_nxeMaxLength.SetWindowText("");
	}
	// (r.gonet 09/21/2011) - PLID 45583 - If a list field, load the list items.
	if(m_pField->GetFieldType() == lcftSingleSelectList) {
		LoadListItems();
	} else {
		m_pCustomListItems->Clear();
	}

	// The preview is effectively a control, so load it here.
	CreateFieldPreview();
}

// (r.gonet 09/21/2011) - PLID 45585 - Loads the field type combo box, whose values control the the basic data type of the field.
void CLabsEditCustomFieldDlg::LoadFieldTypeCombo()
{
	m_pFieldTypeCombo->Clear();
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldTypeCombo->GetNewRow();
	pRow->PutValue(lftID, _variant_t((long)lcftUndefined));
	pRow->PutValue(lftName, _variant_t("<Select a Type>"));
	m_pFieldTypeCombo->AddRowSorted(pRow, NULL);
	
	// (r.gonet 09/21/2011) - PLID 45581 - Added support for the boolean field type
	pRow = m_pFieldTypeCombo->GetNewRow();
	pRow->PutValue(lftID, _variant_t((long)lcftBool));
	pRow->PutValue(lftName, _variant_t("Checkbox"));
	m_pFieldTypeCombo->AddRowSorted(pRow, NULL);
	
	// (r.gonet 09/21/2011) - PLID 45581 - Added support for the integer field type
	pRow = m_pFieldTypeCombo->GetNewRow();
	pRow->PutValue(lftID, _variant_t((long)lcftInteger));
	pRow->PutValue(lftName, _variant_t("Whole Number"));
	m_pFieldTypeCombo->AddRowSorted(pRow, NULL);
	
	// (r.gonet 09/21/2011) - PLID 45581 - Added support for the float field type
	pRow = m_pFieldTypeCombo->GetNewRow();
	pRow->PutValue(lftID, _variant_t((long)lcftFloat));
	pRow->PutValue(lftName, _variant_t("Real Number"));
	m_pFieldTypeCombo->AddRowSorted(pRow, NULL);
	
	// (r.gonet 09/21/2011) - PLID 45582 - Added support for the text field type
	pRow = m_pFieldTypeCombo->GetNewRow();
	pRow->PutValue(lftID, _variant_t((long)lcftText));
	pRow->PutValue(lftName, _variant_t("Text"));
	m_pFieldTypeCombo->AddRowSorted(pRow, NULL);

	// (r.gonet 09/21/2011) - PLID 45581 - Added support for the datetime field type.
	pRow = m_pFieldTypeCombo->GetNewRow();
	pRow->PutValue(lftID, _variant_t((long)lcftDateTime));
	pRow->PutValue(lftName, _variant_t("Date/Time"));
	m_pFieldTypeCombo->AddRowSorted(pRow, NULL);

	// (r.gonet 09/21/2011) - PLID 45583 - Added support for the list field type.
	pRow = m_pFieldTypeCombo->GetNewRow();
	pRow->PutValue(lftID, _variant_t((long)lcftSingleSelectList));
	pRow->PutValue(lftName, _variant_t("List"));
	m_pFieldTypeCombo->AddRowSorted(pRow, NULL);

	// Set the current selection based on the field's type.
	ELabCustomFieldType lcftCurrentType = m_pField->GetFieldType();
	if(lcftCurrentType == lcftMemo) {
		lcftCurrentType = lcftText;
	}
	m_pFieldTypeCombo->SetSelByColumn(lftID, _variant_t((long)lcftCurrentType));
}

// (r.gonet 09/21/2011) - PLID 45585 - Loads the display type combo box, whose values control the look and feel of the field.
void CLabsEditCustomFieldDlg::LoadDisplayTypeCombo()
{
	m_pDisplayTypeCombo->Clear();
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	ADD_TYPE_ROW(m_pDisplayTypeCombo, (long)lcdtDefault, "< Default >");

	switch(m_pField->GetFieldType()) {
		// (r.gonet 09/21/2011) - PLID 45581 - Booleans have just checkboxes
		case lcftBool:
			break;
		// (r.gonet 09/21/2011) - PLID 45581 - Integers just have text boxes
		case lcftInteger:
			break;
		// (r.gonet 09/21/2011) - PLID 45581 - Floats just have text boxes
		case lcftFloat:
			break;
		// (r.gonet 09/21/2011) - PLID 45582 - Text and memo fields get two options. We can display in a single entry box or a multi line box that accepts returns. Both scroll.
		case lcftText:
		case lcftMemo:
			ADD_TYPE_ROW(m_pDisplayTypeCombo, (long)lcdtSingleLineEdit, "Single Line Textbox");
			ADD_TYPE_ROW(m_pDisplayTypeCombo, (long)lcdtMultiLineEdit, "Multi-Line Textbox");
			break;
		case lcftDateTime:
			// (r.gonet 09/21/2011) - PLID 45581 - Datetimes have a couple different possibilities. Default is date and time, but can also display only date or only time.
			ADD_TYPE_ROW(m_pDisplayTypeCombo, (long)lcdtDateAndTime, "Date and Time");
			ADD_TYPE_ROW(m_pDisplayTypeCombo, (long)lcdtDate, "Date");
			ADD_TYPE_ROW(m_pDisplayTypeCombo, (long)lcdtTime, "Time");
			break;
		case lcftSingleSelectList:
			// (r.gonet 10/30/2011) - PLID 45583 - Radio buttons are now possible
			ADD_TYPE_ROW(m_pDisplayTypeCombo, (long)lcdtRadioButtons, "Radio Buttons");
			ADD_TYPE_ROW(m_pDisplayTypeCombo, (long)lcdtDropDown, "Drop Down List");
			break;
		case lcftUndefined:
		default:
			break;
	}
	
	// Set the default selection based on the field's current display type.
	ELabCustomFieldDisplayType lcdtCurrentDisplayType = m_pField->GetDisplayType();
	m_pDisplayTypeCombo->SetSelByColumn(ldtID, _variant_t((long)lcdtCurrentDisplayType));
}

// (r.gonet 09/21/2011) - PLID 45583 - Loads a list field's custom items. It is an error if this is called on a field that is not a list field.
void CLabsEditCustomFieldDlg::LoadListItems()
{
	// Downcast to a list field. We should be guaranteed to get a list field if the caller figured this correctly.
	CLabCustomListFieldPtr pListField = boost::dynamic_pointer_cast<CLabCustomListField>(m_pField);
	if(pListField == NULL) {
		ThrowNxException("CLabsEditCustomFieldDlg::LoadListItems : Attempted to load list items for a non-list field.");
	}

	// (r.gonet 09/21/2011) - PLID 45583 - Load all the list items into the list items datalist.
	m_pCustomListItems->Clear();
	std::map<long, CLabCustomListFieldItem *>::iterator itemIter;
	for(itemIter = pListField->GetItemsBegin(); itemIter != pListField->GetItemsEnd(); itemIter++) {
		CLabCustomListFieldItem *pItem = itemIter->second;
		// (r.gonet 09/21/2011) - PLID 45558 - Load only non-deleted items, since we are non-historical
		if(!pItem->GetDeleted()) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCustomListItems->GetNewRow();
			pRow->PutValue(liID, _variant_t(pItem->GetID()));
			pRow->PutValue(liSortOrder, _variant_t(pItem->GetSortOrder()));
			pRow->PutValue(liDescription, _variant_t(pItem->GetDescription()));
			m_pCustomListItems->AddRowSorted(pRow, NULL);	
		}
	}
}

// (r.gonet 09/21/2011) - PLID 45585 - Creates the field preview box.
void CLabsEditCustomFieldDlg::CreateFieldPreview()
{
	if(m_dlgPreview.m_hWnd != NULL) {
		return;
	}
	m_dlgPreview.Create(IDD_LAB_CUSTOM_FIELDS_VIEW, this);	

	// Sets the location of the preview to be below the preview label.
	CRect rcRect;
	GetClientRect(&rcRect);

	CRect rcColor;
	m_nxcFieldPreview.GetWindowRect(&rcColor);
	ScreenToClient(&rcColor);
	rcRect.left = rcColor.left;
	rcRect.right = rcColor.right;

	CRect rcFieldPreviewLabel;
	m_nxstaticFieldPreview.GetWindowRect(&rcFieldPreviewLabel);
	ScreenToClient(&rcFieldPreviewLabel);
	rcRect.top = rcFieldPreviewLabel.bottom;

	CRect rcDefaultLabel;
	m_nxstaticDefault.GetWindowRect(&rcDefaultLabel);
	ScreenToClient(&rcDefaultLabel);
	rcRect.bottom = rcDefaultLabel.top;
	rcRect.DeflateRect(10, 10);
	m_dlgPreview.MoveWindow(rcRect);
	// (r.gonet 12/09/2011) - PLID 45555 - This is very strange but we need to have this window up at the top in order to be able to pass scroll messages to it.
	m_dlgPreview.SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	m_dlgPreview.ShowWindow(SW_SHOW);
	m_dlgPreview.ShowControls(true);
}

// (r.gonet 09/21/2011) - PLID 45585 - Updates the field preview to reflect the current state of the field.
void CLabsEditCustomFieldDlg::RefreshFieldPreview()
{
	m_dlgPreview.RefreshFields();
}

// (r.gonet 09/21/2011) - PLID 45585 - Updates states of controls to reflect internal state of the dialog and field.
void CLabsEditCustomFieldDlg::EnsureControls()
{
	if(m_pField == NULL) {
		return;
	}

	if(m_pDisplayTypeCombo->GetRowCount() == 1) {
		// The display type only has the <Default> display type, so disable the drop down.
		m_pDisplayTypeCombo->Enabled = VARIANT_FALSE;
	} else {
		// There are more options to choose from than just <Default>, so enable the drop down.
		m_pDisplayTypeCombo->Enabled = VARIANT_TRUE;
	}

	// (r.gonet 09/21/2011) - PLID 45583 - Disable controls conditionally depending on the field's type
	BOOL bIsList = (m_pField->GetFieldType() == lcftSingleSelectList ? TRUE : FALSE);
	m_pCustomListItems->Enabled = bIsList ? VARIANT_TRUE : VARIANT_FALSE;
	m_btnAddListItem.EnableWindow(bIsList);
	m_btnEditListItem.EnableWindow(bIsList);
	m_btnDeleteListItem.EnableWindow(bIsList);
	m_btnMoveListItemUp.EnableWindow(bIsList);
	m_btnMoveListItemDown.EnableWindow(bIsList);
	
	// (r.gonet 11/23/2011) - PLID 45585 - Some fields types and display types can't take sizes.
	BOOL bSizingAllowed = TRUE;
	if(m_pField->GetFieldType() == lcftBool ||
		(m_pField->GetFieldType() == lcftSingleSelectList && (m_pField->GetDisplayType() == lcdtDefault || m_pField->GetDisplayType() == lcdtRadioButtons)))
	{
		bSizingAllowed = FALSE;
	}
	m_radioAutoSize.EnableWindow(bSizingAllowed);
	m_radioFixedSize.EnableWindow(bSizingAllowed);

	// (r.gonet 11/23/2011) - PLID 45585 - Only if we allow sizing and then are set to fixed size do we even let the user change the width and height.
	m_nxeWidth.EnableWindow(bSizingAllowed && m_radioFixedSize.GetCheck() ? TRUE : FALSE);
	m_nxeHeight.EnableWindow(bSizingAllowed && m_radioFixedSize.GetCheck() ? TRUE : FALSE);

	// Text fields are allowed to set a maximum character length. Nothing else makes sense to have this.
	if(m_pField->GetFieldType() == lcftText || m_pField->GetFieldType() == lcftMemo) {
		m_nxeMaxLength.EnableWindow(TRUE);
	} else {
		m_nxeMaxLength.EnableWindow(FALSE);
	}

	// (r.gonet 09/21/2011) - PLID 45583 - Now, if we have a list, then enable the list controls
	if(bIsList) {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCustomListItems->CurSel;
		if(!pRow) {
			m_btnMoveListItemUp.EnableWindow(FALSE);
			m_btnMoveListItemDown.EnableWindow(FALSE);	
			m_btnEditListItem.EnableWindow(FALSE);
			m_btnDeleteListItem.EnableWindow(FALSE);
		} else {
			if(pRow->GetNextRow() == NULL) {
				m_btnMoveListItemDown.EnableWindow(FALSE);
			}
			if(pRow->GetPreviousRow() == NULL) {
				m_btnMoveListItemUp.EnableWindow(FALSE);
			}
		}
	}
}

// (r.gonet 09/21/2011) - PLID 45585 - Public method to get the edited field.
CLabCustomFieldPtr CLabsEditCustomFieldDlg::GetField()
{
	// This function should not be called before the field is created.
	//  It should only be called once the editor has been closed.
	ASSERT(m_pField != NULL && m_pField->GetFieldType() != lcftUndefined);

	return m_pField;
}

// (r.gonet 09/21/2011) - PLID 45585 - Syncs the name
void CLabsEditCustomFieldDlg::OnEnKillfocusCustomFieldNameEdit()
{
	try {
		CString str;
		m_nxeName.GetWindowText(str);
		if(str != m_pField->GetName()) {
			m_pField->SetName(str);
			m_dlgPreview.SyncControlsToFields();
			RefreshFieldPreview();
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45585 - Syncs the friendly name
void CLabsEditCustomFieldDlg::OnEnKillfocusCustomFieldDispnameEdit()
{
	try {
		CString str;
		m_nxeDisplayName.GetWindowText(str);
		if(str != m_pField->GetFriendlyName()) {
			m_pField->SetFriendlyName(str);
			m_dlgPreview.SyncControlsToFields();
			RefreshFieldPreview();
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45585 - Syncs the description
void CLabsEditCustomFieldDlg::OnEnKillfocusCustomFieldDescEdit()
{
	try {
		CString str;
		m_nxeDescription.GetWindowText(str);
		if(str != m_pField->GetDescription()) {
			m_pField->SetDescription(str);
			m_dlgPreview.SyncControlsToFields();
			RefreshFieldPreview();
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45585 - Syncs the field type, which is tricky.
void CLabsEditCustomFieldDlg::OnSelChosenCustomFieldTypeCombo(LPDISPATCH lpRow)
{
	try {
		ELabCustomFieldType lcftOldFieldType, lcftNewFieldType;
		
		// Get the field type we are converting from
		lcftOldFieldType = m_pField->GetFieldType();

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(!pRow) {
			// Set the selection back to our old field type
			m_pFieldTypeCombo->SetSelByColumn(lftID, _variant_t((long)lcftOldFieldType));
			return;
		}

		// Get the field type we are converting to
		lcftNewFieldType = (ELabCustomFieldType)VarLong(pRow->GetValue(lftID), -1);
		
		if(lcftOldFieldType == lcftNewFieldType) {
			// We would be switching the type to same thing, so don't do anything.
			return;
		}

		// Now make a new field to replace the old one with, but copy over all of the values from the old field.
		CLabCustomFieldPtr pNewField;
		switch(lcftNewFieldType) {
			// (r.gonet 09/21/2011) - PLID 45581 - Added support for changing to boolean
			case lcftBool:
				pNewField = make_shared<CLabCustomBooleanField>(*m_pField);
				break;
			// (r.gonet 09/21/2011) - PLID 45581 - Added support for changing to integer
			case lcftInteger:
				pNewField = make_shared<CLabCustomIntegerField>(*m_pField);
				break;
			// (r.gonet 09/21/2011) - PLID 45581 - Added support for changing to float
			case lcftFloat:
				pNewField = make_shared<CLabCustomFloatField>(*m_pField);
				break;
			case lcftText:
				pNewField = make_shared<CLabCustomTextField>(*m_pField, lcftText);
				break;
			case lcftMemo:
				pNewField = make_shared<CLabCustomTextField>(*m_pField, lcftMemo);
				break;
			// (r.gonet 09/21/2011) - PLID 45581 - Added support for changing to datetime
			case lcftDateTime:
				pNewField = make_shared<CLabCustomDateTimeField>(*m_pField);
				break;
			// (r.gonet 09/21/2011) - PLID 45583 - Added support for list fields.
			case lcftSingleSelectList:
				pNewField = make_shared<CLabCustomListField>(*m_pField);
				break;
			case lcftUndefined:
			default:
				// In lieu of null, we use an empty field.
				pNewField = make_shared<CLabCustomEmptyField>(*m_pField);
				break;
		}
		// Make sure we are using a type that is known to us.
		ASSERT(pNewField->GetFieldType() != lcftUndefined || lcftNewFieldType == lcftUndefined);

		// Switch out the old field for the new one
		m_pField = pNewField;
		m_pField->SetModificationStatus(CLabCustomField::emsModified);
		// Swap out the old field for the new field
		m_cfTemplateInstance.GetTemplate()->ReplaceField(m_pTemplateField->GetID(), m_pField);

		// Reload everything
		LoadControlValues();
		EnsureControls();
		RefreshFieldPreview();

	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45585 - Sync back the display type, which may alter the preview a lot.
void CLabsEditCustomFieldDlg::OnSelChosenCustomFieldDisptypeCombo(LPDISPATCH lpRow)
{
	try {
		ELabCustomFieldDisplayType lcdtOldDisplayType, lcdtNewDisplayType;
		
		// Get the field type we are converting from
		lcdtOldDisplayType = m_pField->GetDisplayType();

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(!pRow) {
			// Set the selection back to our old field type
			m_pFieldTypeCombo->SetSelByColumn(0, _variant_t((long)lcdtOldDisplayType));
			return;
		}

		// Get the field type we are converting to
		lcdtNewDisplayType = (ELabCustomFieldDisplayType)VarLong(pRow->GetValue(0), -1);
		
		if(lcdtOldDisplayType == lcdtNewDisplayType) {
			// We would be switching the type to same thing, so don't do anything.
			return;
		}

		m_dlgPreview.SyncControlsToFields();
		m_pField->SetDisplayType(lcdtNewDisplayType);

		// Checkbox and radio lists aren't affected by sizing, so remove any values from the display.
		if(m_pField->GetFieldType() == lcftBool || 
			(m_pField->GetFieldType() == lcftSingleSelectList && (m_pField->GetDisplayType() == lcdtDefault || m_pField->GetDisplayType() == lcdtRadioButtons)))
		{
			m_radioAutoSize.SetCheck(TRUE);
			m_radioFixedSize.SetCheck(FALSE);
			m_nxeWidth.SetWindowText("");
			m_nxeHeight.SetWindowText("");
			m_pField->SetSizeMode(CLabCustomField::cfsmAutoSize);
		}

		EnsureControls();
		RefreshFieldPreview();

	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45585 - Sync back the fixed width
void CLabsEditCustomFieldDlg::OnEnKillfocusCustomFieldWidthEdit()
{
	try {
		CString strWidth;
		m_nxeWidth.GetWindowText(strWidth);
		if(strWidth.SpanIncluding("0123456789").GetLength() == 0) {
			long nOldWidth = m_pField->GetWidth();
			strWidth = FormatString("%li", nOldWidth);
			m_nxeWidth.SetWindowText(strWidth);
		} else {
			m_pField->SetWidth(atol(strWidth));
		}
		m_dlgPreview.SyncControlsToFields();
		RefreshFieldPreview();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45585 - Sync back the fixed height
void CLabsEditCustomFieldDlg::OnEnKillfocusCustomFieldHeightEdit()
{
	try {
		CString strHeight;
		m_nxeHeight.GetWindowText(strHeight);
		if(strHeight.SpanIncluding("0123456789").GetLength() == 0) {
			long nOldHeight = m_pField->GetHeight();
			strHeight = FormatString("%li", nOldHeight);
			m_nxeHeight.SetWindowText(strHeight);
		} else {
			m_pField->SetHeight(atol(strHeight));
		}
		m_pField->SetHeight(atol(strHeight));
		m_dlgPreview.SyncControlsToFields();
		RefreshFieldPreview();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45582 - Sync back the maximum character length. Due to the way text is saved in the database, if this field is in use, then we can't edit this.
void CLabsEditCustomFieldDlg::OnEnKillfocusCustomFieldMaxCharsEdit()
{
	try {
		shared_ptr<CLabCustomTextField> pTextField = boost::dynamic_pointer_cast<CLabCustomTextField>(m_pField);
		if(!pTextField) {
			ThrowNxException("CLabsEditCustomFieldDlg::OnEnKillfocusCustomFieldMaxCharsEdit : It shouldn't be possible to set max chars for non-text fields.");
		}

		CString str;
		m_nxeMaxLength.GetWindowText(str);
		long nMaxChars = atol(str);

		if(nMaxChars != pTextField->GetMaxLength()) {
			if(CLabCustomField::IsInUse(m_pField->GetID())) {
				MessageBox("This field is in use on existing labs. In order to prevent data loss, the maximum length cannot be changed.", "Field is in use", MB_ICONERROR);
				CString strMaxLength = FormatString("%li", pTextField->GetMaxLength());
				m_nxeMaxLength.SetWindowText(strMaxLength);
				return;
			}

			// (r.gonet 12/05/2011) - PLID 45967 - Test whether this field is in use on a report. If so, we can't change it otherwise the TTX will be messed up.
			if(m_pField->GetModificationStatus() != CLabCustomField::emsNew) {
				long nCustomReportCount = 0;
				_RecordsetPtr prs = CreateParamRecordset(
					"SELECT CustomReportNumber "
					"FROM LabReqCustomFieldsT "
					"WHERE LabCustomFieldID = {INT} "
					"GROUP BY CustomReportNumber; ",
					m_pField->GetID());
				if(!prs->eof) {
					nCustomReportCount = prs->GetRecordCount();
				}
				prs->Close();
				if(nCustomReportCount > 0) {
					MessageBox(FormatString("The maximum length cannot be changed because the field is in use on %li custom Lab Request report(s).", nCustomReportCount), "Field is in use", MB_ICONERROR);
					CString strMaxLength = FormatString("%li", pTextField->GetMaxLength());
					m_nxeMaxLength.SetWindowText(strMaxLength);
					return;
				}
			}
		}

		// (r.gonet 11/23/2011) - PLID 45585 - I'm disallowing 0 to be entered here because NxEdit doesn't seem to let you limit length to 0 and because I think it is useless.
		if(nMaxChars <= 0) {
			MessageBox("Maximum length must be positive. Setting maximum length to 1.", "Error", MB_OK|MB_ICONERROR);
			m_nxeMaxLength.SetWindowText("1");
			nMaxChars = 1;
		} else if(nMaxChars > 20000) { // Entirely arbitrary since we have ntext
			MessageBox("Maximum length must be less than or equal to 20000. Setting maximum length to 20000.", "Error", MB_OK|MB_ICONERROR);
			m_nxeMaxLength.SetWindowText("20000");
			nMaxChars = 20000;
		}
		
		// We need the default value right away to evaluate if it will be truncated.
		m_dlgPreview.SyncControlsToFields();
		// We may need to truncate the character length of the default value
		CString strValue = VarString(pTextField->GetDefaultValue(), "");
		if(strValue.GetLength() > nMaxChars) {
			if(IDYES == MessageBox(
				FormatString("The new maximum length is smaller than the number of characters being used "
				"in the default value. Only the first %li characters of the default value will be saved. "
				"Do you want to continue?", nMaxChars), "Truncate Default Value?", MB_YESNO|MB_ICONQUESTION)) 
			{
				strValue = strValue.Left(nMaxChars);
				pTextField->SetDefaultValue(_variant_t(strValue));
			} else {
				CString strMaxLength = FormatString("%li", pTextField->GetMaxLength());
				m_nxeMaxLength.SetWindowText(strMaxLength);
				return;
			}
		}
		pTextField->SetMaxLength(nMaxChars);
		RefreshFieldPreview();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45583 - Add a new list item to this list field. Opens the list item editor.
void CLabsEditCustomFieldDlg::OnBnClickedCustomFieldListAddBtn()
{
	try {
		if(m_pField->GetFieldType() != lcftSingleSelectList) {
			return;
		}
		CLabCustomListFieldPtr pListField = boost::dynamic_pointer_cast<CLabCustomListField>(m_pField);
		if(pListField == NULL) {
			ThrowNxException("CLabsEditCustomFieldDlg::OnBnClickedCustomFieldListAddBtn : Attempted to add a new item to a non-list field. Field has a type of a list, but is in memory as a non-list.");
		}

		// (r.gonet 09/21/2011) - PLID 45606 - Open the list item editor on a new list item.
		CLabCustomListFieldItem *pItem = new CLabCustomListFieldItem();
		CLabCustomFieldListItemEditor dlg(*pItem, this);
		if(dlg.DoModal() == IDOK) {
			// User succesfully created a list item, so add it to the template and the datalist.
			CLabCustomListFieldItem *pNewItem = pListField->AddItem(pItem->GetValue(), pItem->GetType(), pItem->GetDescription());
			delete pItem;
			pItem = pNewItem;
			
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCustomListItems->GetNewRow();
			pRow->PutValue(liID, _variant_t(pItem->GetID()));
			pRow->PutValue(liSortOrder, _variant_t(pItem->GetSortOrder()));
			pRow->PutValue(liDescription, _variant_t(pItem->GetDescription()));
			m_pCustomListItems->AddRowSorted(pRow, NULL);
			
			EnsureControls();
			m_dlgPreview.SyncControlsToFields();
			RefreshFieldPreview();
		} else {
			if(pItem) {
				delete pItem;
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45583 - Opens the list item editor on the selected list item.
void CLabsEditCustomFieldDlg::OnBnClickedCustomFieldListEditBtn()
{
	try {
		if(m_pField->GetFieldType() != lcftSingleSelectList) {
			return;
		}
		CLabCustomListFieldPtr pListField = boost::dynamic_pointer_cast<CLabCustomListField>(m_pField);
		if(pListField == NULL) {
			ThrowNxException("CLabsEditCustomFieldDlg::OnBnClickedCustomFieldListEditBtn : Attempted to edit an item in a non-list field. Field has a type of a list, but is in memory as a non-list.");
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCustomListItems->CurSel;
		if(pRow) {
			CLabCustomListFieldItem *pItem = pListField->GetItem(pRow->GetValue(liID));
			if(pItem == NULL) {
				ThrowNxException("CLabsEditCustomFieldDlg::OnBnClickedCustomFieldListEditBtn : List item not found with ID = %li.", pRow->GetValue(0)); 
			}
			// (r.gonet 09/21/2011) - PLID 45606 - Open the list item editor on this list item.
			CLabCustomFieldListItemEditor dlg(*pItem, this);
			if(dlg.DoModal() == IDOK) {				
				pRow->PutValue(liDescription, _variant_t(pItem->GetDescription()));

				EnsureControls();
				m_dlgPreview.SyncControlsToFields();
				RefreshFieldPreview();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45583 - Removes an item from the list.
void CLabsEditCustomFieldDlg::OnBnClickedCustomFieldListDeleteBtn()
{
	try {
		if(m_pField->GetFieldType() != lcftSingleSelectList) {
			return;
		}
		CLabCustomListFieldPtr pListField = boost::dynamic_pointer_cast<CLabCustomListField>(m_pField);
		if(pListField == NULL) {
			ThrowNxException("CLabsEditCustomFieldDlg::OnBnClickedCustomFieldListDeleteBtn : Attempted to remove an item from a non-list field. Field has a type of a list, but is in memory as a non-list.");
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCustomListItems->CurSel;
		if(pRow) {
			long nFieldItemID = pRow->GetValue(liID);
			pListField->RemoveItem(nFieldItemID);
			m_pCustomListItems->RemoveRow(pRow);
		}

		EnsureControls();
		m_dlgPreview.SyncControlsToFields();
		RefreshFieldPreview();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45583 - Moves a list item up in the world.
void CLabsEditCustomFieldDlg::OnBnClickedCustomFieldUpBtn()
{
	try {
		if(m_pField->GetFieldType() != lcftSingleSelectList) {
			return;
		}
		CLabCustomListFieldPtr pListField = boost::dynamic_pointer_cast<CLabCustomListField>(m_pField);
		if(pListField == NULL) {
			ThrowNxException("CLabsEditCustomFieldDlg::OnBnClickedCustomFieldUpBtn : Attempted to move an item up in a non-list field. Field has a type of a list, but is in memory as a non-list.");
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCustomListItems->CurSel;
		if(pRow) {
			NXDATALIST2Lib::IRowSettingsPtr pPrevRow = pRow->GetPreviousRow();
			if(pPrevRow == NULL){
				//we're already the first row
				return;
			}
			
			IRowSettingsPtr pNewRow = m_pCustomListItems->AddRowBefore(pRow, pPrevRow);
			m_pCustomListItems->CurSel = pNewRow;
			m_pCustomListItems->RemoveRow(pRow);

			// Now swap the sort orders within the items
			CLabCustomListFieldItem *pSelectedItem = pListField->GetItem(pNewRow->GetValue(0));
			CLabCustomListFieldItem *pPrevItem = pListField->GetItem(pPrevRow->GetValue(0));
			long nTempSortOrder = pSelectedItem->GetSortOrder();
			pSelectedItem->SetSortOrder(pPrevItem->GetSortOrder());
			pPrevItem->SetSortOrder(nTempSortOrder);
		}

		EnsureControls();
		m_dlgPreview.SyncControlsToFields();
		RefreshFieldPreview();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45583 - Moves a list item down in the world.
void CLabsEditCustomFieldDlg::OnBnClickedCustomFieldDownBtn()
{
	try {
		if(m_pField->GetFieldType() != lcftSingleSelectList) {
			return;
		}
		CLabCustomListFieldPtr pListField = boost::dynamic_pointer_cast<CLabCustomListField>(m_pField);
		if(pListField == NULL) {
			ThrowNxException("CLabsEditCustomFieldDlg::OnBnClickedCustomFieldDownBtn : Attempted to move an item down in a non-list field. Field has a type of a list, but is in memory as a non-list.");
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCustomListItems->CurSel;
		if(pRow) {
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = pRow->GetNextRow();
			if(pNextRow == NULL){
				//we're already the last row
				return;
			}
			
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pCustomListItems->AddRowBefore(pNextRow, pRow);
			m_pCustomListItems->CurSel = pRow;
			m_pCustomListItems->RemoveRow(pNextRow);

			// Now swap the sort orders within the items
			CLabCustomListFieldItem *pSelectedItem = pListField->GetItem(pRow->GetValue(0));
			CLabCustomListFieldItem *pNextItem = pListField->GetItem(pNewRow->GetValue(0));
			long nTempSortOrder = pSelectedItem->GetSortOrder();
			pSelectedItem->SetSortOrder(pNextItem->GetSortOrder());
			pNextItem->SetSortOrder(nTempSortOrder);
		}

		EnsureControls();
		m_dlgPreview.SyncControlsToFields();
		RefreshFieldPreview();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45585 - Save the field and close the editor.
void CLabsEditCustomFieldDlg::OnBnClickedOk()
{
	try {
		CParamSqlBatch sqlBatch;
		if(!m_dlgPreview.SyncControlsToFields()) {
			return;
		}
		// (r.gonet 09/21/2011) - PLID 45585 - Kill focus is not called before OK is called, so we need to manually do that to ensure the edit controls are synced back to 
		//  the field. This is a bit of kludge but I think any alternative would be worse.
		SyncFromEditControls();

		// Disallow saving without a defined type of field
		if(m_pField->GetFieldType() == lcftUndefined) {
			MessageBox("You must choose a field type before saving.", "Error", MB_OK|MB_ICONERROR);
			return;
		}

		// We'll also disallow them from saving a field without a name or display name. I can't think of a good reason for allowing blank names.
		if(m_pField->GetName().IsEmpty() || m_pField->GetFriendlyName().IsEmpty()) {
			MessageBox("You must enter both a name and display name for the field before saving.", "Error", MB_OK|MB_ICONERROR);
			return;
		}

		// Check for duplicates
		if(ReturnsRecordsParam("SELECT Name FROM LabCustomFieldsT WHERE Name LIKE {STRING} AND ID <> {INT} ", m_pField->GetName(), m_pField->GetID())) {
			MessageBox("A custom field already exists with that name. All custom fields must have unique names.", "Error", MB_ICONERROR);
			return;
		}

		m_pField->Save(sqlBatch);
		sqlBatch.Execute(GetRemoteData());

		OnOK();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45585 - Ok, so don't save but still close the editor.
void CLabsEditCustomFieldDlg::OnBnClickedCancel()
{
	try {
		OnCancel();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45585 - Sync back the field's size mode to be autosized
void CLabsEditCustomFieldDlg::OnBnClickedLcfEditAutoSizeRadio()
{
	try {
		m_pField->SetSizeMode(CLabCustomField::cfsmAutoSize);
		EnsureControls();

		m_dlgPreview.SyncControlsToFields();
		RefreshFieldPreview();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45585 - Sync back the field's size mode to be fixed size
void CLabsEditCustomFieldDlg::OnBnClickedLcfEditFixedSizeRadio()
{
	try {
		m_pField->SetSizeMode(CLabCustomField::cfsmFixedSize);
		// It wasn't that simple. Set the field values for width and height.
		CString strWidth;
		m_nxeWidth.GetWindowText(strWidth);
		long nWidth = atol(strWidth);
		if(nWidth <= 0) {
			nWidth = m_pField->GetWidth();
			// a Width of -1 indicates that the field doesn't have the width initialized.
			if(nWidth == -1) {
				nWidth = 100;
				m_pField->SetWidth(nWidth);
			}
		} else {
			m_pField->SetWidth(nWidth);
		}
		m_nxeWidth.SetWindowText(FormatString("%li", nWidth));
		CString strHeight;
		m_nxeHeight.GetWindowText(strHeight);
		long nHeight = atol(strHeight);
		if(nHeight <= 0) {
			nHeight = m_pField->GetHeight();
			// a Height of -1 indicates that the field doesn't have the height initialized.
			if(nHeight == -1) {
				nHeight = 20;
				m_pField->SetHeight(nHeight);
			}
		} else {
			m_pField->SetHeight(nHeight);
		}
		m_nxeHeight.SetWindowText(FormatString("%li", nHeight));
		EnsureControls();

		m_dlgPreview.SyncControlsToFields();
		RefreshFieldPreview();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45585 - We have selected a different list item. Make sure all of our arrows and such are updated.
void CLabsEditCustomFieldDlg::SelChangedCustomFieldListItems(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45585 - Don't allow changing the display type to null. 
void CLabsEditCustomFieldDlg::SelChangingCustomFieldDisptypeCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		// Changing the display type is non-destructive. So allow it.
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}	
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45585 - Don't allow changing the field type to anything if we are in use. Also disallow null selections.
void CLabsEditCustomFieldDlg::SelChangingCustomFieldTypeCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
		// Ensure this field is not in use somewhere. Changing the field type is descructive. 
		else if(lpOldSel != *lppNewSel) {
			if(CLabCustomField::IsInUse(m_pField->GetID())) {
				MessageBox("This field is in use on existing labs. In order to prevent data loss, the field type cannot be changed.", "Field is in use", MB_ICONERROR);
				SafeSetCOMPointer(lppNewSel, lpOldSel);
				return;
			}

			// (r.gonet 12/05/2011) - PLID 45967 - Test whether this field is in use on a report. If so, we can't change it otherwise the TTX will be messed up.
			if(m_pField->GetModificationStatus() != CLabCustomField::emsNew) {
				long nCustomReportCount = 0;
				_RecordsetPtr prs = CreateParamRecordset(
					"SELECT CustomReportNumber "
					"FROM LabReqCustomFieldsT "
					"WHERE LabCustomFieldID = {INT} "
					"GROUP BY CustomReportNumber; ",
					m_pField->GetID());
				if(!prs->eof) {
					nCustomReportCount = prs->GetRecordCount();
				}
				prs->Close();
				if(nCustomReportCount > 0) {
					MessageBox(FormatString("The field type cannot be changed because the field is in use on %li custom Lab Request report(s).", nCustomReportCount), "Field is in use", MB_ICONERROR);
					SafeSetCOMPointer(lppNewSel, lpOldSel);
					return;
				}
			}

		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 11/23/2011) - PLID 45585 - OnKillFocus isn't called before OnOK, so that means that OnOK 
//  will need to save the edit fields manually. I decided to do this by directly calling the message handlers
//  in order to avoid having the alternative of having a single save function 
void CLabsEditCustomFieldDlg::SyncFromEditControls()
{
	OnEnKillfocusCustomFieldNameEdit();
	OnEnKillfocusCustomFieldDispnameEdit();
	OnEnKillfocusCustomFieldDescEdit();
	if(m_nxeWidth.IsWindowEnabled() && m_nxeHeight.IsWindowEnabled()) {
		OnEnKillfocusCustomFieldWidthEdit();
		OnEnKillfocusCustomFieldHeightEdit();
	}
	if(m_nxeMaxLength.IsWindowEnabled()) {
		OnEnKillfocusCustomFieldMaxCharsEdit();
	}
}

// (r.gonet 12/08/2011) - PLID 45585 - Display some help for common properties
void CLabsEditCustomFieldDlg::OnBnClickedCommonFieldHelpBtn()
{
	try {
		MessageBox("Common Field Properties:\r\n"
			" * Name - The name of the field, which identifies it.\r\n"
			" * Display Name - What the field will be called when it is available in Additional Fields in a lab. Appears above the field. In most cases, this should be the same as the name.\r\n"
			" * Description - This will be displayed below the Display Name of the field and should explain how this field should be filled or used.\r\n", "Help");
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 12/08/2011) - PLID 45585 - Display some help for field type
void CLabsEditCustomFieldDlg::OnBnClickedFieldTypeHelpBtn()
{
	try {
		MessageBox("The Field Type lets you control what type of data can be entered into a field. The available types are:\r\n"
			" * Checkbox - Chiefly for Yes/No or True/False questions. Since it can only be checked or unchecked, it will always be filled with an answer.\r\n"
			"\r\n"
			" * Whole Number - Only accepts whole numbers as input. Some examples of allowed data are 1, -5, 0, and 1234 .\r\n"
			"\r\n"
			" * Real Number - Allows all numbers, including those with decimal points. Some examples of allowed data are 1, -5.2, 0, and 1234.122 .\r\n"
			"\r\n"
			" * Text - Allows all characters as data. You can set a maximum number of characters the user can enter as well.\r\n"
			"\r\n"
			" * Date/Time - Allows dates and/or times to be entered. Press the down arrow key when your mouse cursor is in this field to open a calendar.\r\n"
			"\r\n"
			" * List - Allows the user to select a single value from a list of possible values. You can set up what the list contains in the List Items section.\r\n");
			
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 12/08/2011) - PLID 45585 - Display some help for display type
void CLabsEditCustomFieldDlg::OnBnClickedDisplayTypeHelpBtn()
{
	try {
		switch(m_pField->GetFieldType()) {
		case lcftBool:
			MessageBox("Checkbox fields are only displayed as checkboxes.", "Display Type Help");
			break;
		case lcftInteger:
			MessageBox("Whole Number fields are only displayed with text entry boxes.", "Display Type Help");
			break;
		case lcftFloat:
			MessageBox("Real Number fields are only displayed with text entry boxes.", "Display Type Help");
			break;
		case lcftText:
		case lcftMemo:
			MessageBox("Text fields can be displayed in different ways. By default, Text fields are displayed with single line text entry boxes.\r\n"
				" * Single Line Textbox - Only a single line of text can be entered. You cannot press enter to cause the line to break. This is useful for short text fields, such as a Name.\r\n"
				"\r\n"
				" * Multi-Line Textbox - Multiple lines of text are allowed. You can press enter to cause the line to break. This is useful for long textual comments.\r\n", "Display Type Help");
			break;
		case lcftDateTime:
			MessageBox("Date/Time fields can be displayed in different ways. By default, Date/Time fields show both the date and the time.\r\n"
				" * Date and Time - Both a date and a time are shown together. Both can be entered in the field.\r\n"
				"\r\n"
				" * Date - Only a date is shown and only a date can be entered ino the field.\r\n"
				"\r\n"
				" * Time - Only a time is shown and only a time can be entered into the field.", 
				"Display Type Help");
			break;
		case lcftSingleSelectList:
			MessageBox("List fields can be displayed in different ways. By default, List fields are shown as a list of Radio Buttons.\r\n"
				" * Radio Buttons - Items in the list are shown as radio buttons where when the buttons next to one item is pressed, the other items' buttons are unselected.\r\n"
				"\r\n"
				" * Drop Down List - Items in the list are shown in a drop down menu from which a selection can be made.\r\n",
				"Display Type Help");
			break;
		case lcftUndefined:
			MessageBox("A field type is required before setting a display type.", "Display Type Help");
			break;
		default:
			break;
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 12/08/2011) - PLID 45585 - Display some help for other properties
void CLabsEditCustomFieldDlg::OnBnClickedOtherPropertiesHelpBtn()
{
	try {
		MessageBox("Other Field Properties include:\r\n"
			" * Sizing - For certain field types, you can select a mode that will determine how large the field will show to the user.\r\n"
			"   - Automatic Size - The field's width and height are automatically adjusted so that they stretch from the left side of the screen to the right side.\r\n"
			"   - Fixed Size - You determine the field's width and height here.\r\n"
			"      * Width (px) - The field's width in pixels.\r\n"
			"      * Height (px) - The field's height in pixels.\r\n"
			"\r\n"
			" * Maximum Character Length - For Text type fields, you can enter the maximum number of characters that can be typed into the field.",
			"Other Field Properties Help");
	} NxCatchAll(__FUNCTION__);
}

void CLabsEditCustomFieldDlg::OnBnClickedFieldPreviewHelpBtn()
{
	try {
		MessageBox("The field preview lets you view how the field will look on a lab.\r\n"
			"You can also setup a default value for the field here by entering it into the previewed field.\r\n"
			"The default value will be automatically selected/entered when you create a new lab.",
			"Field Preview Help");
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 12/08/2011) - PLID 45585 - Display some help for list items.
void CLabsEditCustomFieldDlg::OnBnClickedListItemsHelpBtn()
{
	try {
		MessageBox("List Items Features:\r\n"
			" * List Items - Shows what items are possible selections for this list field.\r\n"
			"\r\n"
			" * Add - Lets you add a new list item to the possible selections for this field.\r\n"
			" * Edit - Lets you edit an existing list item and change its value. If this item is selected on existing labs, you will be prevented from changing the value here.\r\n"
			" * Delete - Lets you remove an existing list item from the possible selections for this field. If this item is selected on existing labs, it will stay selected there.\r\n"
			"\r\n"
			" * Move Up - Lets you change the order in which the item is displayed in the list by moving it up one.\r\n" 
			" * Move Up - Lets you change the order in which the item is displayed in the list by moving it down one.",
			"List Item Features Help");
	} NxCatchAll(__FUNCTION__);
}