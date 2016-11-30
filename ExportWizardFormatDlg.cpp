// ExportWizardFormatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "financialrc.h"
#include "ExportWizardFormatDlg.h"
#include "ExportWizardDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CExportWizardFormatDlg property page

IMPLEMENT_DYNCREATE(CExportWizardFormatDlg, CPropertyPage)

CExportWizardFormatDlg::CExportWizardFormatDlg() : CPropertyPage(CExportWizardFormatDlg::IDD)
{
	m_bEditingDatalist = true;
	//{{AFX_DATA_INIT(CExportWizardFormatDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CExportWizardFormatDlg::~CExportWizardFormatDlg()
{
}

void CExportWizardFormatDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportWizardFormatDlg)
	DDX_Control(pDX, IDC_REMOVE_SPECIAL_CHAR, m_btnRemove);
	DDX_Control(pDX, IDC_ADD_SPECIAL_CHAR, m_btnAdd);
	DDX_Control(pDX, IDC_RECORD_SEPARATOR_REPLACE, m_editRecordReplace);
	DDX_Control(pDX, IDC_TEXT_DELIMITER_REPLACE, m_editDelimiterReplace);
	DDX_Control(pDX, IDC_FIELD_SEPARATOR_REPLACE, m_editSeparatorReplace);
	DDX_Control(pDX, IDC_TEXT_DELIMITER, m_editDelimiter);
	DDX_Control(pDX, IDC_SEPARATOR, m_editSeparator);
	DDX_Control(pDX, IDC_OTHER_SEPARATOR_TEXT, m_nxeditOtherSeparatorText);
	DDX_Control(pDX, IDC_SPECIAL_CHARACTERS_DESCRIPTION, m_nxstaticSpecialCharactersDescription);
	DDX_Control(pDX, IDC_EXPORT_OUTPUT_GROUPBOX, m_btnExportOutputGroupbox);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CExportWizardFormatDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CExportWizardFormatDlg)
	ON_BN_CLICKED(IDC_CHARACTER_SEPARATED, OnCharacterSeparated)
	ON_BN_CLICKED(IDC_FIXED_WIDTH, OnFixedWidth)
	ON_BN_CLICKED(IDC_ADD_SPECIAL_CHAR, OnAddSpecialChar)
	ON_BN_CLICKED(IDC_REMOVE_SPECIAL_CHAR, OnRemoveSpecialChar)
	ON_BN_CLICKED(IDC_REPLACE_SEPARATOR, OnReplaceSeparator)
	ON_BN_CLICKED(IDC_REPLACE_DELIMITER, OnReplaceDelimiter)
	ON_BN_CLICKED(IDC_CARRIAGE_RETURN, OnCarriageReturn)
	ON_BN_CLICKED(IDC_LINE_FEED, OnLineFeed)
	ON_BN_CLICKED(IDC_OTHER_SEPARATOR, OnOtherSeparator)
	ON_BN_CLICKED(IDC_PAIR, OnPair)
	ON_BN_CLICKED(IDC_REPLACE_RECORD_SEPARATOR, OnReplaceRecordSeparator)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportWizardFormatDlg message handlers

BOOL CExportWizardFormatDlg::OnSetActive()
{
	//Set our title based on the name of this export
	//

	//Fill in the controls based on the stored data.
	if(((CExportWizardDlg*)GetParent())->m_eotOutputType == eotFixedWidth) {
		CheckRadioButton(IDC_CHARACTER_SEPARATED, IDC_FIXED_WIDTH, IDC_FIXED_WIDTH);
		OnFixedWidth();
	}
	else {
		CheckRadioButton(IDC_CHARACTER_SEPARATED, IDC_FIXED_WIDTH, IDC_CHARACTER_SEPARATED);
		OnCharacterSeparated();
	}

	CheckDlgButton(IDC_INCLUDE_FIELD_NAMES,((CExportWizardDlg*)GetParent())->m_bIncludeFieldNames?BST_CHECKED:BST_UNCHECKED);

	m_editSeparator.SetLimitText(5);
	m_editSeparator.SetWindowText(((CExportWizardDlg*)GetParent())->m_strFieldSeparator);

	m_editSeparatorReplace.SetLimitText(5);
	if(((CExportWizardDlg*)GetParent())->m_strFieldEscape != ((CExportWizardDlg*)GetParent())->m_strFieldSeparator) {
		CheckDlgButton(IDC_REPLACE_SEPARATOR, BST_CHECKED);
		m_editSeparatorReplace.SetWindowText(((CExportWizardDlg*)GetParent())->m_strFieldEscape);
	}
	else {
		CheckDlgButton(IDC_REPLACE_SEPARATOR, BST_UNCHECKED);
	}
	OnReplaceSeparator();

	m_editDelimiter.SetLimitText(5);
	m_editDelimiter.SetWindowText(((CExportWizardDlg*)GetParent())->m_strTextDelimiter);

	m_editDelimiterReplace.SetLimitText(5);
	if(((CExportWizardDlg*)GetParent())->m_strTextEscape != ((CExportWizardDlg*)GetParent())->m_strTextDelimiter) {
		CheckDlgButton(IDC_REPLACE_DELIMITER, BST_CHECKED);
		m_editDelimiterReplace.SetWindowText(((CExportWizardDlg*)GetParent())->m_strTextEscape);
	}
	else {
		CheckDlgButton(IDC_REPLACE_DELIMITER, BST_UNCHECKED);
	}
	OnReplaceDelimiter();


	m_pSpecialChars->Clear();
	for(int i = 0; i < ((CExportWizardDlg*)GetParent())->m_arSpecialChars.GetSize(); i++) {
		IRowSettingsPtr pRow = m_pSpecialChars->GetRow(-1);
		pRow->PutValue(0, _bstr_t(((CExportWizardDlg*)GetParent())->m_arSpecialChars.GetAt(i).strSourceChar));
		pRow->PutValue(1, _bstr_t(((CExportWizardDlg*)GetParent())->m_arSpecialChars.GetAt(i).strReplaceChar));
		m_pSpecialChars->AddRow(pRow);
	}

	OnSelChangedSpecialChars(-1);

	if(((CExportWizardDlg*)GetParent())->m_strRecordSeparator == "\r") {
		CheckRadioButton(IDC_CARRIAGE_RETURN, IDC_OTHER_SEPARATOR, IDC_CARRIAGE_RETURN);
		OnCarriageReturn();
	}
	else if(((CExportWizardDlg*)GetParent())->m_strRecordSeparator == "\n") {
		CheckRadioButton(IDC_CARRIAGE_RETURN, IDC_OTHER_SEPARATOR, IDC_LINE_FEED);
		OnLineFeed();
	}
	else if(((CExportWizardDlg*)GetParent())->m_strRecordSeparator == "\r\n") {
		CheckRadioButton(IDC_CARRIAGE_RETURN, IDC_OTHER_SEPARATOR, IDC_PAIR);
		OnPair();
	}
	else {
		CheckRadioButton(IDC_CARRIAGE_RETURN, IDC_OTHER_SEPARATOR, IDC_OTHER_SEPARATOR);
		OnOtherSeparator();
		SetDlgItemText(IDC_OTHER_SEPARATOR_TEXT, ((CExportWizardDlg*)GetParent())->m_strRecordSeparator);
	}

	m_editRecordReplace.SetLimitText(5);
	if(((CExportWizardDlg*)GetParent())->m_strRecordSeparatorEscape != ((CExportWizardDlg*)GetParent())->m_strRecordSeparator) {
		CheckDlgButton(IDC_REPLACE_RECORD_SEPARATOR, BST_CHECKED);
		m_editRecordReplace.SetWindowText(((CExportWizardDlg*)GetParent())->m_strRecordSeparatorEscape);
	}
	else {
		CheckDlgButton(IDC_REPLACE_RECORD_SEPARATOR, BST_UNCHECKED);
	}
	OnReplaceRecordSeparator();

	//They can always go back or next..
	((CExportWizardDlg*)GetParent())->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);

	((CExportWizardDlg*)GetParent())->SetTitle("Output format for export \"" + ((CExportWizardDlg*)GetParent())->m_strName + "\"");

	// (z.manning 2009-12-10 14:21) - PLID 36519 - Skip this sheet for history exports
	if(((CExportWizardDlg*)GetParent())->m_ertRecordType == ertHistory) {
		return FALSE;
	}

	return CPropertyPage::OnSetActive();
}

void CExportWizardFormatDlg::OnCharacterSeparated() 
{
	m_editSeparator.EnableWindow(TRUE);
	m_editDelimiter.EnableWindow(TRUE);
	GetDlgItem(IDC_REPLACE_SEPARATOR)->EnableWindow(TRUE);
	OnReplaceSeparator();
	GetDlgItem(IDC_REPLACE_DELIMITER)->EnableWindow(TRUE);
	OnReplaceDelimiter();
}

void CExportWizardFormatDlg::OnFixedWidth() 
{
	m_editSeparator.EnableWindow(FALSE);
	m_editSeparatorReplace.EnableWindow(FALSE);
	m_editDelimiter.EnableWindow(FALSE);
	m_editDelimiterReplace.EnableWindow(FALSE);
	GetDlgItem(IDC_REPLACE_SEPARATOR)->EnableWindow(FALSE);
	GetDlgItem(IDC_REPLACE_DELIMITER)->EnableWindow(FALSE);
}

BOOL CExportWizardFormatDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// (j.jones 2008-05-08 09:19) - PLID 29953 - added nxiconbuttons for modernization
	m_btnAdd.AutoSet(NXB_NEW);
	m_btnRemove.AutoSet(NXB_DELETE);
	
	m_pSpecialChars = BindNxDataListCtrl(this, IDC_SPECIAL_CHARS, GetRemoteData(), false);

	//If you have a string > 255 characters in the resources, you get an irritating warning.
	SetDlgItemText(IDC_SPECIAL_CHARACTERS_DESCRIPTION, "Special characters are defined by the program which will parse your output file.  For example, some programs may use the ampersand (&&) character to indicate a special formatting code.  Therefore, if your data had any actual ampersands in it (perhaps in an employer name like \"Smith, Jones && Doe\"), you would need them to be replaced by &&&&, to tell the parser that ampersand is meant to be there.");
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CExportWizardFormatDlg, CPropertyPage)
    //{{AFX_EVENTSINK_MAP(CExportWizardFormatDlg)
	ON_EVENT(CExportWizardFormatDlg, IDC_SPECIAL_CHARS, 2 /* SelChanged */, OnSelChangedSpecialChars, VTS_I4)
	ON_EVENT(CExportWizardFormatDlg, IDC_SPECIAL_CHARS, 9 /* EditingFinishing */, OnEditingFinishingSpecialChars, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CExportWizardFormatDlg, IDC_SPECIAL_CHARS, 10 /* EditingFinished */, OnEditingFinishedSpecialChars, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CExportWizardFormatDlg, IDC_SPECIAL_CHARS, 8 /* EditingStarting */, OnEditingStartingSpecialChars, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CExportWizardFormatDlg::OnSelChangedSpecialChars(long nNewSel) 
{
	if(nNewSel == -1) {
		GetDlgItem(IDC_REMOVE_SPECIAL_CHAR)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_REMOVE_SPECIAL_CHAR)->EnableWindow(TRUE);
	}
}

void CExportWizardFormatDlg::OnEditingFinishingSpecialChars(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		if(nCol == 0 && *pbCommit) {
			//It's not allowed to be empty
			if(strUserEntered == "") {
				AfxMessageBox("Please enter a valid special character");
				*pbContinue = FALSE;
			}

			//There can't be duplicate source characters.
			CString strSeparator;
			GetDlgItemText(IDC_SEPARATOR, strSeparator);
			if(strSeparator == strUserEntered) {
				AfxMessageBox("The character you are entering is already being handled as the field separator.");
				*pbContinue = FALSE;
			}
			CString strDelimiter;
			GetDlgItemText(IDC_TEXT_DELIMITER, strDelimiter);
			if(strDelimiter == strUserEntered) {
				AfxMessageBox("The character you are entering is already being handled as the text delimiter.");
				*pbContinue = FALSE;
			}
			for(int i = 0; i < m_pSpecialChars->GetRowCount(); i++) {
				if(i != nRow) {
					if(VarString(m_pSpecialChars->GetValue(i,0)) == strUserEntered) {
						AfxMessageBox("The character you are entering is already being handled as a special character.");
						*pbContinue = FALSE;
					}
				}
			}
		}				
	}NxCatchAll("Error in CExportWizardFormatDlg::OnEditingFinishingSpecialChars()");
}

void CExportWizardFormatDlg::OnEditingFinishedSpecialChars(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if(nCol == 0 && !bCommit && VarString(varNewValue).IsEmpty()) {
		//They made a new row, then hit escape.  Remove this row.
		m_pSpecialChars->RemoveRow(nRow);
	}

	if(bCommit) {
		//Otherwise, update the data in our parent.
		((CExportWizardDlg*)GetParent())->m_arSpecialChars.RemoveAll();
		for(int i = 0; i < m_pSpecialChars->GetRowCount(); i++) {
			SpecialChar sc;
			sc.strSourceChar = VarString(m_pSpecialChars->GetValue(i,0));
			sc.strReplaceChar = VarString(m_pSpecialChars->GetValue(i,1),"");
			((CExportWizardDlg*)GetParent())->m_arSpecialChars.Add(sc);
		}
	}
	
	m_bEditingDatalist = false;
}

void CExportWizardFormatDlg::OnAddSpecialChar() 
{
	IRowSettingsPtr pRow = m_pSpecialChars->GetRow(-1);
	pRow->PutValue(0, _bstr_t(""));
	pRow->PutValue(1, _bstr_t(""));
	m_pSpecialChars->AddRow(pRow);
	m_pSpecialChars->StartEditing(m_pSpecialChars->GetRowCount()-1, 0);
}

void CExportWizardFormatDlg::OnRemoveSpecialChar() 
{
	if(m_pSpecialChars->CurSel != -1) {
		m_pSpecialChars->RemoveRow(m_pSpecialChars->CurSel);
	}
	OnSelChangedSpecialChars(m_pSpecialChars->CurSel);

	//Update the data in our parent.
	((CExportWizardDlg*)GetParent())->m_arSpecialChars.RemoveAll();
	for(int i = 0; i < m_pSpecialChars->GetRowCount(); i++) {
		SpecialChar sc;
		sc.strSourceChar = VarString(m_pSpecialChars->GetValue(i,0));
		sc.strReplaceChar = VarString(m_pSpecialChars->GetValue(i,1),"");
		((CExportWizardDlg*)GetParent())->m_arSpecialChars.Add(sc);
	}
}

void CExportWizardFormatDlg::OnReplaceSeparator() 
{
	GetDlgItem(IDC_FIELD_SEPARATOR_REPLACE)->EnableWindow(IsDlgButtonChecked(IDC_REPLACE_SEPARATOR));
}

void CExportWizardFormatDlg::OnReplaceDelimiter() 
{
	GetDlgItem(IDC_TEXT_DELIMITER_REPLACE)->EnableWindow(IsDlgButtonChecked(IDC_REPLACE_DELIMITER));
}

BOOL CExportWizardFormatDlg::OnKillActive()
{
	//Pass everything to our parent.
	if(IsDlgButtonChecked(IDC_CHARACTER_SEPARATED)) {
		((CExportWizardDlg*)GetParent())->m_eotOutputType = eotCharacterSeparated;
	}
	else if(IsDlgButtonChecked(IDC_FIXED_WIDTH)) {
		((CExportWizardDlg*)GetParent())->m_eotOutputType = eotFixedWidth;
	}
	else {
		MsgBox("You must select a format for your output.");
		return FALSE;
	}

	((CExportWizardDlg*)GetParent())->m_bIncludeFieldNames = IsDlgButtonChecked(IDC_INCLUDE_FIELD_NAMES)?true:false;

	CString strField;
	GetDlgItemText(IDC_SEPARATOR, strField);
	((CExportWizardDlg*)GetParent())->m_strFieldSeparator = strField;
	if(IsDlgButtonChecked(IDC_REPLACE_SEPARATOR)) {
		GetDlgItemText(IDC_FIELD_SEPARATOR_REPLACE, strField);
	}
	((CExportWizardDlg*)GetParent())->m_strFieldEscape = strField;

	GetDlgItemText(IDC_TEXT_DELIMITER, strField);
	((CExportWizardDlg*)GetParent())->m_strTextDelimiter = strField;
	if(IsDlgButtonChecked(IDC_REPLACE_DELIMITER)) {
		GetDlgItemText(IDC_TEXT_DELIMITER_REPLACE, strField);
	}
	((CExportWizardDlg*)GetParent())->m_strTextEscape = strField;

	//The data in the datalist has been kept up to date all along.
	
	CString strSeparator;
	if(IsDlgButtonChecked(IDC_CARRIAGE_RETURN)) {
		strSeparator = "\r";
	}
	else if(IsDlgButtonChecked(IDC_LINE_FEED)) {
		strSeparator = "\n";
	}
	else if(IsDlgButtonChecked(IDC_PAIR)) {
		strSeparator = "\r\n";
	}
	else {
		GetDlgItemText(IDC_OTHER_SEPARATOR_TEXT, strSeparator);
	}
	((CExportWizardDlg*)GetParent())->m_strRecordSeparator = strSeparator;
	
	CString strSeparatorEscape = strSeparator;
	if(IsDlgButtonChecked(IDC_REPLACE_RECORD_SEPARATOR)) {
		GetDlgItemText(IDC_RECORD_SEPARATOR_REPLACE, strSeparatorEscape);
	}
	((CExportWizardDlg*)GetParent())->m_strRecordSeparatorEscape = strSeparatorEscape;

	return CPropertyPage::OnKillActive();
}

void CExportWizardFormatDlg::OnEditingStartingSpecialChars(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	m_bEditingDatalist = true;
}

int CExportWizardFormatDlg::PreTranslateMessage(MSG *pMsg)
{
	switch(pMsg->message) {
	case WM_KEYDOWN:
		if(pMsg->wParam == VK_RETURN && m_bEditingDatalist) {
			//If we're not careful, the property sheet will steal this and try to take us to the next tab.
			m_pSpecialChars->StopEditing(TRUE);
			return TRUE;
		}
		break;
	}

	return CPropertyPage::PreTranslateMessage(pMsg);
}

void CExportWizardFormatDlg::OnCarriageReturn() 
{
	GetDlgItem(IDC_OTHER_SEPARATOR_TEXT)->EnableWindow(FALSE);
}

void CExportWizardFormatDlg::OnLineFeed() 
{
	GetDlgItem(IDC_OTHER_SEPARATOR_TEXT)->EnableWindow(FALSE);
}

void CExportWizardFormatDlg::OnOtherSeparator() 
{
	GetDlgItem(IDC_OTHER_SEPARATOR_TEXT)->EnableWindow(TRUE);
}

void CExportWizardFormatDlg::OnPair() 
{
	GetDlgItem(IDC_OTHER_SEPARATOR_TEXT)->EnableWindow(FALSE);
}

void CExportWizardFormatDlg::OnReplaceRecordSeparator() 
{
	GetDlgItem(IDC_RECORD_SEPARATOR_REPLACE)->EnableWindow(IsDlgButtonChecked(IDC_REPLACE_RECORD_SEPARATOR));
}
