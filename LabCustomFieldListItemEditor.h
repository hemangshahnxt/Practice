// (r.gonet 09/21/2011) - PLID 45606 - Added

#pragma once

#include "LabCustomField.h"
#include "AdministratorRc.h"

using namespace NXTIMELib;
using namespace NXDATALIST2Lib;

// CLabCustomFieldListItemEditor dialog

class CLabCustomFieldListItemEditor : public CNxDialog
{
	DECLARE_DYNAMIC(CLabCustomFieldListItemEditor)

private:
	// Enums
	enum EValueTypeListColumn
	{
		evtlcID = 0,
		evtlcName,
	};

	// Controls
	CNxStatic m_nxsHeader;
	CNxStatic m_nxsValueType;
	NXDATALIST2Lib::_DNxDataListPtr m_pValueTypeList;
	CNxStatic m_nxsDescription;
	CNxEdit m_nxeditDescription;
	CNxStatic m_nxsValue;
	NxButton m_radioBoolValueTrue;
	NxButton m_radioBoolValueFalse;
	CNxEdit m_nxeditTextValue;
	CNxEdit m_nxeditIntValue;
	CNxEdit m_nxeditFloatValue;
	NXTIMELib::_DNxTimePtr m_nxtimeDateTimeValue;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnHelp;

	// Data
	// (r.gonet 09/21/2011) - PLID 45606 - The item to edit.
	CLabCustomListFieldItem &m_lclfiItem;
	// (r.gonet 09/21/2011) - PLID 45606 - Copy of the item that will hold all intermediate changes.
	CLabCustomListFieldItem m_lclfiTempItem;

public:
	CLabCustomFieldListItemEditor(CLabCustomListFieldItem &lclfiItem, CWnd* pParent);
	virtual ~CLabCustomFieldListItemEditor();

// Dialog Data
	enum { IDD = IDD_LAB_CUSTOM_FIELD_LIST_ITEM_EDITOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void LoadValueTypeList();
	void LoadControlValues();
	void EnsureControls();
public:
	virtual BOOL OnInitDialog();

	DECLARE_EVENTSINK_MAP()

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedListItemBoolValueTrue();
	afx_msg void OnBnClickedListItemBoolValueFalse();
	void SelChosenListItemDataTypeList(LPDISPATCH lpRow);
	afx_msg void OnEnChangeListItemDescriptionEdit();
	afx_msg void OnEnSetfocusListItemDescriptionEdit();
	afx_msg void OnEnKillfocusListItemDescriptionEdit();
	afx_msg void OnEnKillfocusListItemFloatValue();
	afx_msg void OnEnKillfocusListItemTextValue();
	afx_msg void OnEnKillfocusListItemIntegerValue();
	void KillFocusListItemDatetimeValue();
	afx_msg void OnBnClickedListItemEditorHelpBtn();
	
	void SaveValue();
};
