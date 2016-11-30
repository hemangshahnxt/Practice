// (r.gonet 09/21/2011) - PLID 45585 - Added

#pragma once

#include "AdministratorRc.h"
#include "LabCustomField.h"
#include "LabCustomFieldTemplate.h"
#include "LabCustomFieldsView.h"

// CLabsEditCustomFieldDlg dialog

// (r.gonet 09/21/2011) - PLID 45585 - CLabsEditCustomFieldDlg allows the customizing of new or existing lab custom fields. 
class CLabsEditCustomFieldDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLabsEditCustomFieldDlg)
private:
	// Enums
	enum ELabCustomFieldTypeColumn
	{
		lftID = 0,
		lftName,
	};

	enum ELabCustomFieldDisplayTypeColumn
	{
		ldtID = 0,
		ldtName,
	};

	enum ELabCustomListFieldItemColumn
	{
		liID = 0,
		liSortOrder,
		liDescription,
	};

	// Controls
	CNxColor m_nxcCommonProperties;
	CNxStatic m_nxstaticCommonProperties;
	CNxStatic m_nxstaticName;
	CNxEdit m_nxeName;
	CNxStatic m_nxstaticDisplayName;
	CNxEdit m_nxeDisplayName;
	CNxStatic m_nxstaticDescription;
	CNxEdit m_nxeDescription;
	
	CNxColor m_nxcFieldType;
	CNxStatic m_nxstaticFieldType;
	NXDATALIST2Lib::_DNxDataListPtr m_pFieldTypeCombo;
	CNxStatic m_nxstaticDisplayType;
	NXDATALIST2Lib::_DNxDataListPtr m_pDisplayTypeCombo;

	CNxColor m_nxcOtherProperties;
	CNxStatic m_nxstaticOtherProperties;
	NxButton m_radioAutoSize;
	NxButton m_radioFixedSize;
	CNxStatic m_nxstaticWidth;
	CNxEdit m_nxeWidth;
	CNxStatic m_nxstaticHeight;
	CNxEdit m_nxeHeight;
	CNxStatic m_nxstaticMaxLength;
	CNxEdit m_nxeMaxLength;

	CNxColor m_nxcFieldPreview;
	CNxStatic m_nxstaticFieldPreview;
	CNxStatic m_nxstaticDefault;

	CNxColor m_nxcListItems;
	CNxStatic m_nxstaticListItems;
	NXDATALIST2Lib::_DNxDataListPtr m_pCustomListItems;
	CNxIconButton m_btnAddListItem;
	CNxIconButton m_btnEditListItem;
	CNxIconButton m_btnDeleteListItem;
	CNxIconButton m_btnMoveListItemUp;
	CNxIconButton m_btnMoveListItemDown;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

	// (r.gonet 12/08/2011) - PLID 45585
	CNxIconButton m_btnCommonPropertiesHelp;
	CNxIconButton m_btnFieldTypeHelp;
	CNxIconButton m_btnDisplayTypeHelp;
	CNxIconButton m_btnOtherPropertiesHelp;
	CNxIconButton m_btnFieldPreviewHelp;
	CNxIconButton m_btnListItemsHelp;

	// The preview of the lab custom field, dynamically updated each time one of its properties is changed.
	CLabCustomFieldsView m_dlgPreview;

	// Data
	// (r.gonet 09/21/2011) - PLID 45585 - Field database ID of the field we are editing. -1 if it is a new field.
	long m_nFieldID;
	// (r.gonet 09/21/2011) - PLID 45585 - The field being edited
	CLabCustomFieldPtr m_pField;
	// (r.gonet 09/21/2011) - PLID 45585 - The field as displayed on the temporary template
	CCFTemplateFieldPtr m_pTemplateField;
	// (r.gonet 09/21/2011) - PLID 45585 - The template instance for the preview. It holds the edited field.
	CCFTemplateInstance m_cfTemplateInstance;
public:
	CLabsEditCustomFieldDlg(CWnd* pParent);   // standard constructor
	CLabsEditCustomFieldDlg(long nFieldID, CWnd* pParent);
	virtual ~CLabsEditCustomFieldDlg();

// Dialog Data
	enum { IDD = IDD_LABS_EDIT_CUSTOM_FIELD_DLG };

private:
	// Internal Functions
	void LoadControlValues();
	// Loads the field type combo box, whose values control the the basic data type of the field.
	void LoadFieldTypeCombo();
	// Loads the display type combo box, whose values control the look and feel of the field.
	void LoadDisplayTypeCombo();
	// Loads the built in lists combo box with the names of predefined lists like Service Codes, Diagnosis Codes, etc.
	void LoadBuiltInListsCombo();
	// Loads a list field's custom items. Has no effect if the field is not a list field.
	void LoadListItems();
	// Creates the field preview box.
	void CreateFieldPreview();
	// Updates the field preview to reflect the current state of the field.
	void RefreshFieldPreview();
	// Updates states of controls to reflect internal state of the dialog and field.
	void EnsureControls();

public:
	// Public Interface Functions
	CLabCustomFieldPtr GetField();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnEnKillfocusCustomFieldNameEdit();
	afx_msg void OnEnKillfocusCustomFieldDispnameEdit();
	afx_msg void OnEnKillfocusCustomFieldDescEdit();
	DECLARE_EVENTSINK_MAP()
	void OnSelChosenCustomFieldTypeCombo(LPDISPATCH lpRow);
	void OnSelChosenCustomFieldDisptypeCombo(LPDISPATCH lpRow);
	afx_msg void OnEnKillfocusCustomFieldWidthEdit();
	afx_msg void OnEnKillfocusCustomFieldHeightEdit();
	afx_msg void OnEnKillfocusCustomFieldMaxCharsEdit();
	afx_msg void OnBnClickedCustomFieldCustomListRadio();
	afx_msg void OnBnClickedCustomFieldListAddBtn();
	afx_msg void OnBnClickedCustomFieldListEditBtn();
	afx_msg void OnBnClickedCustomFieldListDeleteBtn();
	afx_msg void OnBnClickedCustomFieldUpBtn();
	afx_msg void OnBnClickedCustomFieldDownBtn();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedLcfEditAutoSizeRadio();
	afx_msg void OnBnClickedLcfEditFixedSizeRadio();
	void SelChangedCustomFieldListItems(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void SelChangingCustomFieldDisptypeCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingCustomFieldTypeCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SyncFromEditControls();
	afx_msg void OnStnClickedCommonFieldHelpLink();
	afx_msg void OnStnClickedFieldTypeHelpLink();
	afx_msg void OnStnClickedDisplayTypeHelpLink();
	afx_msg void OnStnClickedOtherFieldPropertiesHelpLink();
	afx_msg void OnStnClickedFieldPreviewHelpLink();
	afx_msg void OnStnClickedListItemsHelpLink();
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedCommonFieldHelpBtn();
	afx_msg void OnBnClickedFieldTypeHelpBtn();
	afx_msg void OnBnClickedDisplayTypeHelpBtn();
	afx_msg void OnBnClickedOtherPropertiesHelpBtn();
	afx_msg void OnBnClickedFieldPreviewHelpBtn();
	afx_msg void OnBnClickedListItemsHelpBtn();
};
