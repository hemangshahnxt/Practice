// (r.gonet 09/21/2011) - PLID 45606 - Added

// LabCustomFieldListItemEditor.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "LabCustomFieldListItemEditor.h"
#include "InternationalUtils.h"
#include <sstream>

// CLabCustomFieldListItemEditor dialog

IMPLEMENT_DYNAMIC(CLabCustomFieldListItemEditor, CNxDialog)

// (r.gonet 09/21/2011) - PLID 45606 - Creates a new editor that can edit a lab custom field list item. We work on a temporary copy of the list item rather that it itself, then copy it back when we save.
CLabCustomFieldListItemEditor::CLabCustomFieldListItemEditor(CLabCustomListFieldItem &lclfiItem, CWnd* pParent/*= NULL*/)
: CNxDialog(CLabCustomFieldListItemEditor::IDD, pParent), m_lclfiItem(lclfiItem), m_lclfiTempItem(lclfiItem)
{
}

// (r.gonet 09/21/2011) - PLID 45606
CLabCustomFieldListItemEditor::~CLabCustomFieldListItemEditor()
{
}

void CLabCustomFieldListItemEditor::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ITEM_HEADER_STATIC, m_nxsHeader);
	DDX_Control(pDX, IDC_LIST_ITEM_DESCRIPTION_STATIC, m_nxsDescription);
	DDX_Control(pDX, IDC_LIST_ITEM_DESCRIPTION_EDIT, m_nxeditDescription);
	DDX_Control(pDX, IDC_LIST_ITEM_VALUE_TYPE_STATIC, m_nxsValueType);
	DDX_Control(pDX, IDC_LIST_ITEM_VALUE_STATIC, m_nxsValue);
	DDX_Control(pDX, IDC_LIST_ITEM_BOOL_VALUE_TRUE, m_radioBoolValueTrue);
	DDX_Control(pDX, IDC_LIST_ITEM_BOOL_VALUE_FALSE, m_radioBoolValueFalse);
	DDX_Control(pDX, IDC_LIST_ITEM_TEXT_VALUE, m_nxeditTextValue);
	DDX_Control(pDX, IDC_LIST_ITEM_INTEGER_VALUE, m_nxeditIntValue);
	DDX_Control(pDX, IDC_LIST_ITEM_FLOAT_VALUE, m_nxeditFloatValue);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_LIST_ITEM_EDITOR_HELP_BTN, m_btnHelp);
}

// (r.gonet 09/21/2011) - PLID 45606
BOOL CLabCustomFieldListItemEditor::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		m_pValueTypeList = BindNxDataList2Ctrl(this, IDC_LIST_ITEM_DATA_TYPE_LIST, GetRemoteData(), false);
		m_nxtimeDateTimeValue = BindNxTimeCtrl(this, IDC_LIST_ITEM_DATETIME_VALUE);
		m_nxtimeDateTimeValue->ReturnType = 2; // we only allow date and time format
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_nxeditDescription.SetLimitText(255); // Maximum before a memo is necessary
		m_nxeditTextValue.SetLimitText(255); // Maximum before a memo is necessary
		m_nxeditIntValue.SetLimitText(11); // Text length of the maxmimum value of a long
		m_nxeditFloatValue.SetLimitText(15); // Text length of the maximum value for a double

		// Load the available types for list items
		LoadValueTypeList();
		// Load the values we have saved from an existing item into the controls
		LoadControlValues();
		// Ensure things are hidden and disabled based on the item's state
		EnsureControls();
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// (r.gonet 09/21/2011) - PLID 45606 - Loads the available value types.
void CLabCustomFieldListItemEditor::LoadValueTypeList()
{
	m_pValueTypeList->Clear();
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pValueTypeList->GetNewRow();

	pRow = m_pValueTypeList->GetNewRow();
	pRow->PutValue(evtlcID, _variant_t((long)lcftBool));
	pRow->PutValue(evtlcName, _variant_t("True/False"));
	m_pValueTypeList->AddRowSorted(pRow, NULL);
	
	pRow = m_pValueTypeList->GetNewRow();
	pRow->PutValue(evtlcID, _variant_t((long)lcftInteger));
	pRow->PutValue(evtlcName, _variant_t("Whole Number"));
	m_pValueTypeList->AddRowSorted(pRow, NULL);
	
	pRow = m_pValueTypeList->GetNewRow();
	pRow->PutValue(evtlcID, _variant_t((long)lcftFloat));
	pRow->PutValue(evtlcName, _variant_t("Real Number"));
	m_pValueTypeList->AddRowSorted(pRow, NULL);
	
	pRow = m_pValueTypeList->GetNewRow();
	pRow->PutValue(evtlcID, _variant_t((long)lcftText));
	pRow->PutValue(evtlcName, _variant_t("Text"));
	m_pValueTypeList->AddRowSorted(pRow, NULL);

	pRow = m_pValueTypeList->GetNewRow();
	pRow->PutValue(evtlcID, _variant_t((long)lcftDateTime));
	pRow->PutValue(evtlcName, _variant_t("Date/Time"));
	m_pValueTypeList->AddRowSorted(pRow, NULL);
}

// (r.gonet 09/21/2011) - PLID 45606 - Loads control values from an existing list item.
void CLabCustomFieldListItemEditor::LoadControlValues()
{
	// Default type is a text value.
	if(m_lclfiTempItem.GetType() == lcftUndefined) {
		m_pValueTypeList->SetSelByColumn(evtlcID, (long)lcftText);
		m_lclfiTempItem.SetType(lcftText);
	} else {
		m_pValueTypeList->SetSelByColumn(evtlcID, _variant_t((long)m_lclfiTempItem.GetType()));
	}

	// Now called the display type on the dialog
	m_nxeditDescription.SetWindowText(_bstr_t(m_lclfiTempItem.GetDescription()));

	switch(m_lclfiTempItem.GetType())
	{
	case lcftBool:
		{
			// Load a bool value
			if(m_lclfiTempItem.GetValue().vt == VT_BOOL) {
				BOOL bValue = VarBool(m_lclfiTempItem.GetValue());
				m_radioBoolValueTrue.SetCheck(bValue);
				m_radioBoolValueFalse.SetCheck(!bValue);
			} else {
				m_radioBoolValueFalse.SetCheck(TRUE);
			}
		}
		break;
	case lcftText:
		{
			// Load a text value
			CString strValue = m_lclfiTempItem.GetValue().vt == VT_BSTR ? VarString(m_lclfiTempItem.GetValue()) : "";
			m_nxeditTextValue.SetWindowText(_bstr_t(strValue));
		}
		break;
	case lcftInteger:
		{
			// Load a int value
			long nValue = m_lclfiTempItem.GetValue().vt == VT_I4 ? VarLong(m_lclfiTempItem.GetValue()) : 0;
			CString strIntValue = FormatString("%li", nValue);
			m_nxeditIntValue.SetWindowText(_bstr_t(strIntValue));
		}
		break;
	case lcftFloat:
		{
			// Load a float value
			double dValue = m_lclfiTempItem.GetValue().vt == VT_R8 ? VarDouble(m_lclfiTempItem.GetValue()) : 0.0;
			CString strDoubleValue = FormatString("%.6f", dValue);
			m_nxeditFloatValue.SetWindowText(_bstr_t(strDoubleValue));
		}
		break;
	case lcftDateTime:
		{
			// Load a datetime value
			if(m_lclfiTempItem.GetValue().vt == VT_DATE && VarDateTime(m_lclfiTempItem.GetValue()).GetStatus() == COleDateTime::valid) {
				COleDateTime dtValue = VarDateTime(m_lclfiTempItem.GetValue()); 
				m_nxtimeDateTimeValue->SetDateTime(dtValue);
			}
		}
		break;
	default:
		// There are no other types allowed
		break;
	}

	
}

// (r.gonet 09/21/2011) - PLID 45606 - Updates control enable statuses
void CLabCustomFieldListItemEditor::EnsureControls()
{
	// If this list item is in use somewhere, then it can't be changed.
	if(CLabCustomListFieldItem::IsInUse(m_lclfiItem.GetID())) {
		m_pValueTypeList->Enabled = VARIANT_FALSE;
		m_radioBoolValueTrue.EnableWindow(FALSE);
		m_radioBoolValueFalse.EnableWindow(FALSE);
		m_nxeditTextValue.EnableWindow(FALSE);
		m_nxeditIntValue.EnableWindow(FALSE);
		m_nxeditFloatValue.EnableWindow(FALSE);
		m_nxtimeDateTimeValue->Enabled = VARIANT_FALSE;
		m_nxeditDescription.EnableWindow(FALSE);

		this->SetWindowText("List Item Editor (Item In Use - Read Only)");
	}

	// Make invisible all value type controls
	m_radioBoolValueTrue.ShowWindow(SW_HIDE);
	m_radioBoolValueFalse.ShowWindow(SW_HIDE);
	m_nxeditTextValue.ShowWindow(SW_HIDE);
	m_nxeditIntValue.ShowWindow(SW_HIDE);
	m_nxeditFloatValue.ShowWindow(SW_HIDE);
	CWnd *pDateTimeWnd = this->GetDlgItem(IDC_LIST_ITEM_DATETIME_VALUE);
	pDateTimeWnd->ShowWindow(SW_HIDE);

	// Show the appropriate control for this data type
	switch(m_lclfiTempItem.GetType())
	{
	case lcftBool:
		m_radioBoolValueTrue.ShowWindow(SW_SHOW);
		m_radioBoolValueFalse.ShowWindow(SW_SHOW);
		break;
	case lcftText:
		m_nxeditTextValue.ShowWindow(SW_SHOW);
		break;
	case lcftInteger:
		m_nxeditIntValue.ShowWindow(SW_SHOW);
		break;
	case lcftFloat:
		m_nxeditFloatValue.ShowWindow(SW_SHOW);
		break;
	case lcftDateTime:
		pDateTimeWnd->ShowWindow(SW_SHOW);
		break;
	default:
		// You need to handle this type.
		ASSERT(FALSE);
		break;
	}
}

BEGIN_MESSAGE_MAP(CLabCustomFieldListItemEditor, CNxDialog)
	ON_BN_CLICKED(IDOK, &CLabCustomFieldListItemEditor::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CLabCustomFieldListItemEditor::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_LIST_ITEM_BOOL_VALUE_TRUE, &CLabCustomFieldListItemEditor::OnBnClickedListItemBoolValueTrue)
	ON_BN_CLICKED(IDC_LIST_ITEM_BOOL_VALUE_FALSE, &CLabCustomFieldListItemEditor::OnBnClickedListItemBoolValueFalse)
	ON_EN_KILLFOCUS(IDC_LIST_ITEM_FLOAT_VALUE, &CLabCustomFieldListItemEditor::OnEnKillfocusListItemFloatValue)
	ON_EN_KILLFOCUS(IDC_LIST_ITEM_TEXT_VALUE, &CLabCustomFieldListItemEditor::OnEnKillfocusListItemTextValue)
	ON_EN_KILLFOCUS(IDC_LIST_ITEM_INTEGER_VALUE, &CLabCustomFieldListItemEditor::OnEnKillfocusListItemIntegerValue)
	ON_BN_CLICKED(IDC_LIST_ITEM_EDITOR_HELP_BTN, &CLabCustomFieldListItemEditor::OnBnClickedListItemEditorHelpBtn)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CLabCustomFieldListItemEditor, CNxDialog)
	ON_EVENT(CLabCustomFieldListItemEditor, IDC_LIST_ITEM_DATA_TYPE_LIST, 16, CLabCustomFieldListItemEditor::SelChosenListItemDataTypeList, VTS_DISPATCH)
	ON_EVENT(CLabCustomFieldListItemEditor, IDC_LIST_ITEM_DATETIME_VALUE, 1 /* KillFocus */, CLabCustomFieldListItemEditor::KillFocusListItemDatetimeValue, VTS_NONE)
END_EVENTSINK_MAP()


// CLabCustomFieldListItemEditor message handlers

// (r.gonet 09/21/2011) - PLID 45606 - Syncs the control values to the item and closes the item editor
void CLabCustomFieldListItemEditor::OnBnClickedOk()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pValueTypeList->CurSel;
		if(pRow) {
			long nValueType = VarLong(pRow->GetValue(evtlcID));

			// Don't let the user leave the value blank. Check if they did.
			bool bValueIsBlank = false;
			CString strValue;
			switch((ELabCustomFieldType)nValueType) {
				case lcftBool:
					bValueIsBlank = !m_radioBoolValueTrue.GetCheck() && !m_radioBoolValueFalse.GetCheck();
					break;
				case lcftInteger:
					m_nxeditIntValue.GetWindowText(strValue);
					bValueIsBlank = strValue.Trim().GetLength() == 0;
					break;
				case lcftFloat:
					m_nxeditFloatValue.GetWindowText(strValue);
					bValueIsBlank = strValue.Trim().GetLength() == 0;
					break;
				case lcftText:
					// All values are acceptable
					m_nxeditTextValue.GetWindowText(strValue);
					if(strValue.IsEmpty()) {
						// I don't know if this has been saved yet, but we don't want a null value.
						m_lclfiTempItem.SetValue(_variant_t(""));
					}
					break;
				case lcftDateTime:
					// Blank for datetime is if they left the datetime in an invalid state
					bValueIsBlank = m_nxtimeDateTimeValue->GetStatus() != 1;
					break;
				default:
					ASSERT(FALSE);
			}
			if(bValueIsBlank) {
				MessageBox("The item needs to have a value before you can save it.", "Validation Error", MB_OK|MB_ICONERROR);
				return;
			}

			// We also need a display name of the item for how it will appear in a list.
			CString strDescription;
			m_nxeditDescription.GetWindowText(strDescription);
			if(strDescription.IsEmpty()) {
				MessageBox("The item needs to have a display name before you can save it.", "Validation Error", MB_OK|MB_ICONERROR);
				return;
			}
			m_lclfiTempItem.SetDescription(strDescription);
			m_lclfiItem.CloneFrom(m_lclfiTempItem);
			OnOK();
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45606
void CLabCustomFieldListItemEditor::OnBnClickedCancel()
{
	try {
		OnCancel();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45606 - User set a value for a boolean value. Update the description.
void CLabCustomFieldListItemEditor::OnBnClickedListItemBoolValueTrue()
{
	try {
		m_lclfiTempItem.SetValue(g_cvarTrue);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45606 - User set a value for a boolean value. Update the description.
void CLabCustomFieldListItemEditor::OnBnClickedListItemBoolValueFalse()
{
	try {
		m_lclfiTempItem.SetValue(g_cvarFalse);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45606 - User set a value type. Update which field is shown.
void CLabCustomFieldListItemEditor::SelChosenListItemDataTypeList(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			// Save the type to the temp item.
			m_lclfiTempItem.SetType((ELabCustomFieldType)VarLong(pRow->GetValue(evtlcID), m_lclfiTempItem.GetType()));
		} else {
			// No problem with NULL, it won't do anything.
		}
		m_pValueTypeList->SetSelByColumn(evtlcID, _variant_t((long)m_lclfiTempItem.GetType()));
		// We have just switched to another value but there was no kill focus, so we need to get that value now and save it to the temp item.
		SaveValue();
		// Hide old controls and show the new ones that this type depends on
		EnsureControls();

	} NxCatchAll(__FUNCTION__);
}

// User set a description. Turn off auto description generation.
void CLabCustomFieldListItemEditor::OnEnChangeListItemDescriptionEdit()
{
	try {
		// Save away the description in the temp item
		CString strDescription;
		m_nxeditDescription.GetWindowText(strDescription);
		m_lclfiTempItem.SetDescription(strDescription);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45606 - User set a text value. Update the description.
void CLabCustomFieldListItemEditor::OnEnKillfocusListItemTextValue()
{
	try {
		// Save away the text value in the temp item.
		CString strTemp;
		m_nxeditTextValue.GetWindowText(strTemp);
		m_lclfiTempItem.SetValue(_variant_t(strTemp));
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45606 - User set an integer value. Update the description.
void CLabCustomFieldListItemEditor::OnEnKillfocusListItemIntegerValue()
{
	try {
		CString strTemp;
		m_nxeditIntValue.GetWindowText(strTemp);

		// What if they entered in bad characters though?
		char *szBadString;
		ASSERT(strlen(strTemp) < 4096);
		strtol(strTemp, &szBadString, 10);
		if(strTemp.GetLength() > 0 && strlen(szBadString) > 0) {
			MessageBox("Please enter a whole number.", "Error", MB_ICONERROR);
			m_nxeditIntValue.SetWindowText((LPCTSTR)FormatString("%li", atol(strTemp)));
			return;
		}

		// To see if the user entered a value that exceeded our ability to store it, we must go a step beyond to long long.
		if(strTemp.GetLength() > 0) {
			std::stringstream sstr((LPCTSTR)strTemp);
			__int64 nnTest;
			sstr >> nnTest;
			if(nnTest > LONG_MAX) {
				MessageBox(FormatString("You've entered too big of a number. The largest number allowed is %li.", LONG_MAX), "Error", MB_ICONERROR);
				m_nxeditIntValue.SetWindowText((LPCTSTR)FormatString("%li", atol(strTemp)));
				return;
			} else if(nnTest < LONG_MIN) {
				MessageBox(FormatString("You've entered too small of a number. The smallest number allowed is %li.", LONG_MIN), "Error", MB_ICONERROR);
				m_nxeditIntValue.SetWindowText((LPCTSTR)FormatString("%li", atol(strTemp)));
				return;
			}
		}

		long nNumber = atol(strTemp);
		CString strNumber = FormatString("%li", nNumber);
		// Save away the value in the temp item
		m_lclfiTempItem.SetValue(_variant_t(nNumber));
		if(strNumber != strTemp) {
			// The conversion from string to long left this number a bit different, make sure the user knows.
			m_nxeditIntValue.SetWindowText(_bstr_t(strNumber));
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45606 - User set a float value. Update the description.
void CLabCustomFieldListItemEditor::OnEnKillfocusListItemFloatValue()
{
	try {
		CString strValue;
		m_nxeditFloatValue.GetWindowText(strValue);
		
		// Prevent empty values or non-double values.
		if(strValue.GetLength() > 0 && !IsValidDouble(strValue)) {
			MessageBox("Please enter a real number.", "Error", MB_ICONERROR);
			m_nxeditFloatValue.SetWindowText((LPCTSTR)FormatString("%.6f", atof(strValue)));
			return;
		}

		// We need to pick some precision for this value, 6 is arbitrary.
		double dNumber = atof(strValue);
		CString strNumber = FormatString("%.6f", dNumber);
		// Save away the value in the temp item
		m_lclfiTempItem.SetValue(_variant_t(dNumber));
		if(strNumber != strValue) {
			// Display the value with the 6 place precision in order to not give the user any illusions on the precision.
			m_nxeditFloatValue.SetWindowText(strNumber);
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45606 - User set a datetime value. Update the description.
void CLabCustomFieldListItemEditor::KillFocusListItemDatetimeValue()
{
	try {
		if(m_nxtimeDateTimeValue->GetStatus() == 1) {
			COleDateTime dtValue = m_nxtimeDateTimeValue->GetDateTime();

			// (r.gonet 12/07/2011) - PLID 45581 - Yes, somebody actually put in something back in 1300, 
			//  so now we check the range of dates that can be saved to SQL.
			if(dtValue.GetYear() < 1753) {
				MessageBox("You have entered a date that is too far in the past. The oldest possible date that can be used is 01-01-1753.", "Date out of Range", MB_ICONERROR);
				m_nxtimeDateTimeValue->Clear();
				m_lclfiTempItem.SetValue(g_cvarNull);
			} else if(dtValue.GetYear() > 9999) {
				// Is this even possible?
				MessageBox("You have entered a date that is too far in the future. The maximum possible date that can be used is 12-31-9999.", "Date out of Range", MB_ICONERROR);
				m_nxtimeDateTimeValue->Clear();
				m_lclfiTempItem.SetValue(g_cvarNull);
			} else {
				m_lclfiTempItem.SetValue(_variant_t(dtValue, VT_DATE));
			}
		} else {
			m_nxtimeDateTimeValue->Clear();
			m_lclfiTempItem.SetValue(g_cvarNull);
		}
		
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45606 - Gives some helpful instructions.
void CLabCustomFieldListItemEditor::OnBnClickedListItemEditorHelpBtn()
{
	try {
		MessageBox(
			"The Field Type controls the type of the value for this item. Available types are:\r\n"
			" * True/False - Chiefly used for Yes/No or True/False questions.\r\n"
			"\r\n"
			" * Whole Number - Only allows a whole number as a value. Some examples of allowed values are 1, -5, 0, and 1234 .\r\n"
			"\r\n"
			" * Real Number - Allows all numbers, including those with decimal points, as values. Some examples of allowed values are 1, -5.2, 0, and 1234.122 .\r\n"
			"\r\n"
			" * Text - Allows all characters to be entered for a value.\r\n"
			"\r\n"
			" * Date/Time - Allows dates and times to be entered as a value.\r\n"
			"\r\n"
			"\r\n"
			"The Value of the list item is what is exported from the program, for example on reports, in HL7, and in barcodes.\r\n"
			"\r\n"
			"The Display Name is what is shown in the list field for this item. It should be more user friendly, if possible, compared to the Value. It could be a short description of the item, an unabbreviated form of the Value, or even the value itself.",
			"List Item Editor Help");
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45606 - We just need to save the value back to the item. We already know that the value is valid because of our KillFocus buttons 
void CLabCustomFieldListItemEditor::SaveValue()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pValueTypeList->CurSel;
		if(pRow) {
			long nValueType = VarLong(pRow->GetValue(evtlcID));

			CString strValue;
			switch((ELabCustomFieldType)nValueType) {
				case lcftBool:
					// Save the bool value the user has entered to the temp item
					m_lclfiTempItem.SetValue(m_radioBoolValueTrue.GetCheck() ? g_cvarTrue : g_cvarFalse);
					break;
				case lcftInteger:
					// Save the int value the user has entered to the temp item
					m_nxeditIntValue.GetWindowText(strValue);
					m_lclfiTempItem.SetValue(_variant_t((long)atol(strValue)));
					break;
				case lcftFloat:
					// Save the float value the user has entered to the temp item
					m_nxeditFloatValue.GetWindowText(strValue);
					m_lclfiTempItem.SetValue(_variant_t((double)atof(strValue), VT_R8));
					break;
				case lcftText:
					// Save the text value the user has entered to the temp item
					m_nxeditTextValue.GetWindowText(strValue);
					m_lclfiTempItem.SetValue(_variant_t(strValue));
					break;
				case lcftDateTime:
					{
						// Save the datetime value the user has entered to the temp item
						COleDateTime dtValue = m_nxtimeDateTimeValue->GetDateTime();
						m_lclfiTempItem.SetValue(_variant_t(dtValue, VT_DATE));
					}
					break;
				default:
					break;
			}
		}
	} NxCatchAll(__FUNCTION__);
}