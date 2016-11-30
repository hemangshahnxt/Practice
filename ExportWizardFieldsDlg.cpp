// ExportWizardFieldsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "financialrc.h"
#include "ExportWizardFieldsDlg.h"
#include "ExportWizardDlg.h"
#include "ExportUtils.h"
#include "EmrUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CExportWizardFieldsDlg property page

enum FieldListUnselectedColumn {
	flucID = 0,
	flucName = 1,
	flucType = 2,
	flucDynamicID = 3, 
	flucHasAJoinOrFromClause = 4,
};

enum FieldListSelectedColumn {
	flscID = 0,
	flscName = 1,
	flscType = 2,
	flscDynamicID = 3, 
	flscFormat = 4, 
	flscHasAJoinOrFromClause = 5,
};

IMPLEMENT_DYNCREATE(CExportWizardFieldsDlg, CPropertyPage)

CExportWizardFieldsDlg::CExportWizardFieldsDlg() : CPropertyPage(CExportWizardFieldsDlg::IDD)
{
	m_nBasedOn = -1;
	//{{AFX_DATA_INIT(CExportWizardFieldsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CExportWizardFieldsDlg::~CExportWizardFieldsDlg()
{
}

void CExportWizardFieldsDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportWizardFieldsDlg)
	DDX_Control(pDX, IDC_REMOVE_EXPORT_FIELD, m_btnRemove);
	DDX_Control(pDX, IDC_EXPORT_FIELD_UP, m_btnUp);
	DDX_Control(pDX, IDC_EXPORT_FIELD_DOWN, m_btnDown);
	DDX_Control(pDX, IDC_ADD_EXPORT_FIELD, m_btnAdd);
	DDX_Control(pDX, IDC_PLACEHOLDER_TEXT, m_nxeditPlaceholderText);
	DDX_Control(pDX, IDC_FILL_CHAR, m_nxeditFillChar);
	DDX_Control(pDX, IDC_PLACES, m_nxeditPlaces);
	DDX_Control(pDX, IDC_DECIMAL_CHAR, m_nxeditDecimalChar);
	DDX_Control(pDX, IDC_FIXED_WIDTH_PLACES, m_nxeditFixedWidthPlaces);
	DDX_Control(pDX, IDC_CUSTOM_DATE_FORMAT, m_nxeditCustomDateFormat);
	DDX_Control(pDX, IDC_CURRENCY_SYMBOL_TEXT, m_nxeditCurrencySymbolText);
	DDX_Control(pDX, IDC_CUSTOM_PHONE_FORMAT, m_nxeditCustomPhoneFormat);
	DDX_Control(pDX, IDC_TRUE_TEXT, m_nxeditTrueText);
	DDX_Control(pDX, IDC_FALSE_TEXT, m_nxeditFalseText);
	DDX_Control(pDX, IDC_UNKNOWN_TEXT, m_nxeditUnknownText);
	DDX_Control(pDX, IDC_ADVANCED_FIELD, m_nxeditAdvancedField);
	DDX_Control(pDX, IDC_PLACEHOLDER_TEXT_LABEL, m_nxstaticPlaceholderTextLabel);
	DDX_Control(pDX, IDC_FILL_CHAR_LABEL, m_nxstaticFillCharLabel);
	DDX_Control(pDX, IDC_DECIMAL_LABEL, m_nxstaticDecimalLabel);
	DDX_Control(pDX, IDC_PLACES_LABEL, m_nxstaticPlacesLabel);
	DDX_Control(pDX, IDC_DECIMAL_CHAR_LABEL, m_nxstaticDecimalCharLabel);
	DDX_Control(pDX, IDC_SAMPLE_OUTPUT, m_nxstaticSampleOutput);
	DDX_Control(pDX, IDC_FIXED_WIDTH_PLACES_LABEL, m_nxstaticFixedWidthPlacesLabel);
	DDX_Control(pDX, IDC_PLACEHOLDER_LABEL, m_nxstaticPlaceholderLabel);
	DDX_Control(pDX, IDC_CUSTOM_DATE_LABEL, m_nxstaticCustomDateLabel);
	DDX_Control(pDX, IDC_CURRENCY_SYMBOL_LABEL, m_nxstaticCurrencySymbolLabel);
	DDX_Control(pDX, IDC_CURRENCY_PLACEMENT_LABEL, m_nxstaticCurrencyPlacementLabel);
	DDX_Control(pDX, IDC_CUSTOM_PHONE_LABEL, m_nxstaticCustomPhoneLabel);
	DDX_Control(pDX, IDC_TRUE_LABEL, m_nxstaticTrueLabel);
	DDX_Control(pDX, IDC_FALSE_LABEL, m_nxstaticFalseLabel);
	DDX_Control(pDX, IDC_UNKNOWN_LABEL, m_nxstaticUnknownLabel);
	DDX_Control(pDX, IDC_ADVANCED_LABEL, m_nxstaticAdvancedLabel);
	DDX_Control(pDX, IDC_FORMAT_GROUPBOX, m_btnFormatGroupbox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportWizardFieldsDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CExportWizardFieldsDlg)
	ON_BN_CLICKED(IDC_ADD_EXPORT_FIELD, OnAddExportField)
	ON_BN_CLICKED(IDC_REMOVE_EXPORT_FIELD, OnRemoveExportField)
	ON_BN_CLICKED(IDC_EXPORT_FIELD_DOWN, OnExportFieldDown)
	ON_BN_CLICKED(IDC_EXPORT_FIELD_UP, OnExportFieldUp)
	ON_EN_CHANGE(IDC_PLACEHOLDER_TEXT, OnChangePlaceholderText)
	ON_BN_CLICKED(IDC_CAPITALIZE_TEXT, OnCapitalizeText)
	ON_EN_CHANGE(IDC_FILL_CHAR, OnChangeFillChar)
	ON_BN_CLICKED(IDC_RIGHT_JUSTIFY, OnRightJustify)
	ON_EN_SETFOCUS(IDC_FILL_CHAR, OnSetfocusFillChar)
	ON_BN_CLICKED(IDC_DECIMAL_FLOATING, OnDecimalFloating)
	ON_BN_CLICKED(IDC_DECIMAL_FIXED, OnDecimalFixed)
	ON_EN_CHANGE(IDC_DECIMAL_CHAR, OnChangeDecimalChar)
	ON_EN_CHANGE(IDC_PLACES, OnChangePlaces)
	ON_EN_CHANGE(IDC_FIXED_WIDTH_PLACES, OnChangeFixedWidthPlaces)
	ON_BN_CLICKED(IDC_NEGATIVE_PARENS, OnNegativeParens)
	ON_EN_CHANGE(IDC_CUSTOM_DATE_FORMAT, OnChangeCustomDateFormat)
	ON_EN_CHANGE(IDC_CURRENCY_SYMBOL_TEXT, OnChangeCurrencySymbol)
	ON_EN_CHANGE(IDC_CUSTOM_PHONE_FORMAT, OnChangeCustomPhoneFormat)
	ON_BN_CLICKED(IDC_SSN_INCLUDE_HYPHENS, OnSsnIncludeHyphens)
	ON_EN_CHANGE(IDC_UNKNOWN_TEXT, OnChangeUnknownText)
	ON_EN_CHANGE(IDC_TRUE_TEXT, OnChangeTrueText)
	ON_EN_CHANGE(IDC_FALSE_TEXT, OnChangeFalseText)
	ON_EN_CHANGE(IDC_ADVANCED_FIELD, OnChangeAdvancedField)
	ON_BN_CLICKED(IDC_SHOW_ALL_ITEMS, OnShowAllItems)
	ON_BN_CLICKED(IDC_SENTENCE_FORMAT, OnSentenceFormat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportWizardFieldsDlg message handlers

#define ADD_DATE_OPTION(strFmt) {pRow = m_pDateOptions->GetRow(-1); pRow->PutValue(0, (long)0); pRow->PutValue(1, _bstr_t(dtSample.Format(strFmt))); pRow->PutValue(2, _bstr_t(strFmt)); m_pDateOptions->AddRow(pRow);}
#define ADD_PHONE_OPTION(strFmt) {pRow = m_pPhoneOptions->GetRow(-1); pRow->PutValue(0, (long)0); pRow->PutValue(1, _bstr_t(FormatPhone(strSample,strFmt))); pRow->PutValue(2, _bstr_t(strFmt)); m_pPhoneOptions->AddRow(pRow);}

BOOL CExportWizardFieldsDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_btnAdd.AutoSet(NXB_RIGHT);
	m_btnRemove.AutoSet(NXB_LEFT);
	m_btnUp.AutoSet(NXB_UP);
	m_btnDown.AutoSet(NXB_DOWN);

	m_pAvailFields = BindNxDataListCtrl(this, IDC_AVAILABLE_EXPORT_FIELDS, NULL, false);
	m_pSelectFields = BindNxDataListCtrl(this, IDC_SELECTED_EXPORT_FIELDS, NULL, false);
	
	m_pDateOptions = BindNxDataListCtrl(this, IDC_DATE_FORMAT_OPTIONS, NULL, false);
	IRowSettingsPtr pRow;
	COleDateTime dtSample(1979,5,29,17,55,55);
	ADD_DATE_OPTION("%c");
	ADD_DATE_OPTION("%#m/%#d/%Y %#I:%M:%S %p");
	ADD_DATE_OPTION("%m-%d-%Y %H:%M:%S");
	ADD_DATE_OPTION("%Y%m%d%H%M%S");
	ADD_DATE_OPTION("%x");
	ADD_DATE_OPTION("%#m/%#d/%Y");
	ADD_DATE_OPTION("%m-%d-%Y");
	ADD_DATE_OPTION("%Y%m%d");
	ADD_DATE_OPTION("%X");
	ADD_DATE_OPTION("%#I:%M:%S %p");
	ADD_DATE_OPTION("%H%M%S");
	//And the custom row.
	pRow = m_pDateOptions->GetRow(-1); 
	pRow->PutValue(0, (long)1); 
	pRow->PutValue(1, _bstr_t("Custom")); 
	pRow->PutValue(2, _bstr_t("%#m/%#d/%Y %#I:%M:%S %p")); 
	m_pDateOptions->AddRow(pRow);
	
	m_pCurrencyPlacement = BindNxDataListCtrl(this, IDC_CURRENCY_PLACEMENT, NULL, false);
	pRow = m_pCurrencyPlacement->GetRow(-1);
	pRow->PutValue(0, _bstr_t("$(55.56)"));
	m_pCurrencyPlacement->AddRow(pRow);
	pRow = m_pCurrencyPlacement->GetRow(-1);
	pRow->PutValue(0, _bstr_t("($55.56)"));
	m_pCurrencyPlacement->AddRow(pRow);
	pRow = m_pCurrencyPlacement->GetRow(-1);
	pRow->PutValue(0, _bstr_t("(55.56$)"));
	m_pCurrencyPlacement->AddRow(pRow);
	pRow = m_pCurrencyPlacement->GetRow(-1);
	pRow->PutValue(0, _bstr_t("(55.56)$"));
	m_pCurrencyPlacement->AddRow(pRow);
	pRow = m_pCurrencyPlacement->GetRow(-1);

	m_pPhoneOptions = BindNxDataListCtrl(this, IDC_PHONE_FORMAT_OPTIONS, NULL, false);
	CString strSample = "5555550126";
	ADD_PHONE_OPTION("(###) ###-####");
	ADD_PHONE_OPTION("(###)###-####");
	ADD_PHONE_OPTION("###-###-####");
	ADD_PHONE_OPTION("### ### ####");
	ADD_PHONE_OPTION("##########");
	ADD_PHONE_OPTION("###-####");
	ADD_PHONE_OPTION("### ####");
	ADD_PHONE_OPTION("#######");
	ADD_PHONE_OPTION("0## #### ####");
	ADD_PHONE_OPTION("### ### ###");
	//And the custom row.
	pRow = m_pPhoneOptions->GetRow(-1); 
	pRow->PutValue(0, (long)1); 
	pRow->PutValue(1, _bstr_t("Custom")); 
	pRow->PutValue(2, _bstr_t("(###) ###-####")); 
	m_pPhoneOptions->AddRow(pRow);

	SetDlgItemText(IDC_ADVANCED_LABEL, "This is a special field.  Only use this field when you are specifically directed to do so by NexTech Technical Support.");

	((CNxEdit*)GetDlgItem(IDC_FILL_CHAR))->SetLimitText(1);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CExportWizardFieldsDlg::OnAddExportField() 
{
	//Go through all selected fields, copy them over (don't remove them from the list, they can export the same field
	//more than once if they want to).
	long p = m_pAvailFields->GetFirstSelEnum();
	LPDISPATCH pDisp = NULL;
	while (p)
	{	
		m_pAvailFields->GetNextSelEnum(&p, &pDisp);
		IRowSettingsPtr pRow(pDisp);
		
		IRowSettingsPtr pNewRow = m_pSelectFields->GetRow(-1);
		pNewRow->PutValue(flscID, pRow->GetValue(flucID));
		pNewRow->PutValue(flscName, pRow->GetValue(flucName));
		pNewRow->PutValue(flscType, pRow->GetValue(flucType));
		pNewRow->PutValue(flscDynamicID, pRow->GetValue(flucDynamicID));
		pNewRow->PutValue(flscFormat, _bstr_t());
		pNewRow->PutValue(flscHasAJoinOrFromClause, pRow->GetValue(flucHasAJoinOrFromClause));
		pNewRow->ForeColor = pRow->ForeColor; // (c.haag 2007-02-02 10:39) - PLID 24428 - Copy color information
		m_pSelectFields->AddRow(pNewRow);

		pDisp->Release();
	}

	m_pSelectFields->CurSel = m_pSelectFields->GetRowCount()-1;

	OnSelChangedSelectedExportFields(m_pSelectFields->CurSel);

	// (j.jones 2006-03-22 12:06) - PLID 19384 - the * fields are complex and add more JOIN/FROM clauses
	// to the export, of which there is a maximum of 256. So warn if we have more thn 200 such fields.
	long nJoinFieldCount = 0;
	if(m_pSelectFields->GetRowCount() > 200) {
		for(int i=0; i < m_pSelectFields->GetRowCount(); i++)  {
			if(VarString(m_pSelectFields->GetValue(i, flscHasAJoinOrFromClause), "") == "*") {
				nJoinFieldCount++;
			}
		}

		if(nJoinFieldCount > 200) {
			CString str;
			str.Format("There is a limit of 200 export fields that have the * indicator.\n"
				"Please remove at least %li of these * fields before continuing.", nJoinFieldCount - 200);
			AfxMessageBox(str);
		}
	}
}

void RemoveFromArray(CArray<long, long> *pAr, int nRemoveAt)
{
	CArray<long, long> arNew;
	// (a.walling 2007-11-05 15:18) - PLID 27977 - VS2008 - for() loops
	int i = 0;
	for(i = 0; i < pAr->GetSize(); i++) {
		if(i != nRemoveAt) arNew.Add(pAr->GetAt(i));
	}
	pAr->RemoveAll();
	for(i = 0; i < arNew.GetSize(); i++) {
		pAr->Add(arNew.GetAt(i));
	}
}

void ReverseSortArray(CArray<long,long> *pAr)
{
	CArray<long,long> arNew;
	while(pAr->GetSize()) {
		//Remove the highest value from pAr, put it in arNew
		int nHighestIndex = 0;
		int nHighestValue = pAr->GetAt(0);
		for(int i = 1; i < pAr->GetSize(); i++) {
			if(pAr->GetAt(i) > nHighestValue) {
				nHighestIndex = i;
				nHighestValue = pAr->GetAt(i);
			}
		}
		arNew.Add(nHighestValue);
		RemoveFromArray(pAr, nHighestIndex);
	}

	pAr->RemoveAll();
	for(int i = 0; i < arNew.GetSize(); i++) {
		pAr->Add(arNew.GetAt(i));
	}
}

void CExportWizardFieldsDlg::OnRemoveExportField() 
{
	//Just remove them from our select list, they're still in the available list.
	//We'll have to make a list of the selected rows, then go through and remove them from the bottom up, otherwise the 
	//indices will change.
	CArray<long,long> arSelectedIndices;
	long p = m_pSelectFields->GetFirstSelEnum();
	LPDISPATCH pDisp = NULL;
	while (p)
	{	
		m_pSelectFields->GetNextSelEnum(&p, &pDisp);
		IRowSettingsPtr pRow(pDisp);
		arSelectedIndices.Add(pRow->GetIndex());
		pDisp->Release();
	}

	ReverseSortArray(&arSelectedIndices);
	for(int i = 0; i < arSelectedIndices.GetSize(); i++) {
		m_pSelectFields->RemoveRow(arSelectedIndices.GetAt(i));
	}

	OnSelChangedSelectedExportFields(m_pSelectFields->CurSel);

}

void CExportWizardFieldsDlg::OnExportFieldDown() 
{
	long nRow = m_pSelectFields->CurSel;
	if(nRow == -1 || nRow == m_pSelectFields->GetRowCount()-1) {
		return;
	}
	m_pSelectFields->TakeRowInsert(m_pSelectFields->GetRow(nRow), nRow+2);
	m_pSelectFields->CurSel = nRow+1;
	OnSelChangedSelectedExportFields(nRow+1);
}

void CExportWizardFieldsDlg::OnExportFieldUp() 
{
	long nRow = m_pSelectFields->CurSel;
	if(nRow == -1 || nRow == 0) {
		return;
	}
	m_pSelectFields->TakeRowInsert(m_pSelectFields->GetRow(nRow), nRow-1);
	m_pSelectFields->CurSel = nRow-1;
	OnSelChangedSelectedExportFields(nRow-1);
}

BEGIN_EVENTSINK_MAP(CExportWizardFieldsDlg, CPropertyPage)
    //{{AFX_EVENTSINK_MAP(CExportWizardFieldsDlg)
	ON_EVENT(CExportWizardFieldsDlg, IDC_SELECTED_EXPORT_FIELDS, 2 /* SelChanged */, OnSelChangedSelectedExportFields, VTS_I4)
	ON_EVENT(CExportWizardFieldsDlg, IDC_AVAILABLE_EXPORT_FIELDS, 3 /* DblClickCell */, OnDblClickCellAvailableExportFields, VTS_I4 VTS_I2)
	ON_EVENT(CExportWizardFieldsDlg, IDC_SELECTED_EXPORT_FIELDS, 3 /* DblClickCell */, OnDblClickCellSelectedExportFields, VTS_I4 VTS_I2)
	ON_EVENT(CExportWizardFieldsDlg, IDC_AVAILABLE_EXPORT_FIELDS, 2 /* SelChanged */, OnSelChangedAvailableExportFields, VTS_I4)
	ON_EVENT(CExportWizardFieldsDlg, IDC_DATE_FORMAT_OPTIONS, 16 /* SelChosen */, OnSelChosenDateOptions, VTS_I4)
	ON_EVENT(CExportWizardFieldsDlg, IDC_CURRENCY_PLACEMENT, 16 /* SelChosen */, OnSelChosenCurrencyPlacement, VTS_I4)
	ON_EVENT(CExportWizardFieldsDlg, IDC_PHONE_FORMAT_OPTIONS, 16 /* SelChosen */, OnSelChosenPhoneFormatOptions, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CExportWizardFieldsDlg::OnSelChangedSelectedExportFields(long nNewSel) 
{
	long nSelectedType;
	if(nNewSel == -1) {
		GetDlgItem(IDC_REMOVE_EXPORT_FIELD)->EnableWindow(FALSE);
		GetDlgItem(IDC_EXPORT_FIELD_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_EXPORT_FIELD_DOWN)->EnableWindow(FALSE);
		
		//Hide all formatting controls.
		nSelectedType = -1;
	}
	else {
		GetDlgItem(IDC_REMOVE_EXPORT_FIELD)->EnableWindow(TRUE);
		
		//How many rows are selected?
		long nSelCount = 0;
		long p = m_pSelectFields->GetFirstSelEnum();
		LPDISPATCH pDisp = NULL;
		long nSelRow;
		while (p)
		{	nSelCount++;
			m_pSelectFields->GetNextSelEnum(&p, &pDisp);
			IRowSettingsPtr pRow(pDisp);
			nSelRow = pRow->GetIndex();
			pDisp->Release();
		}

		if(nSelCount > 1) {
			GetDlgItem(IDC_EXPORT_FIELD_UP)->EnableWindow(FALSE);
			GetDlgItem(IDC_EXPORT_FIELD_DOWN)->EnableWindow(FALSE);

			//Hide all formatting controls.
			nSelectedType = -1;
		}
		else {
			GetDlgItem(IDC_EXPORT_FIELD_UP)->EnableWindow(nNewSel != 0);
			GetDlgItem(IDC_EXPORT_FIELD_DOWN)->EnableWindow(nNewSel != m_pSelectFields->GetRowCount()-1);

			//Show formatting controls based on current sel.
			nSelectedType = VarLong(m_pSelectFields->GetValue(nNewSel,flscType));
		}
	}

	//Formatting controls.
	//There will be extra formatting controls if we're fixed width.
	bool bFixedWidth = (((CExportWizardDlg*)GetParent())->m_eotOutputType == eotFixedWidth);
	//Placeholder controls.
	GetDlgItem(IDC_PLACEHOLDER_TEXT)->ShowWindow(nSelectedType == eftPlaceholder ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_PLACEHOLDER_TEXT_LABEL)->ShowWindow(nSelectedType == eftPlaceholder ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_PLACEHOLDER_LABEL)->ShowWindow(nSelectedType == eftPlaceholder ? SW_SHOW : SW_HIDE);
	
	//Generic text controls.
	GetDlgItem(IDC_CAPITALIZE_TEXT)->ShowWindow(nSelectedType == eftGenericText || nSelectedType == eftGenericNtext ? SW_SHOW : SW_HIDE);
	
	//Generic number controls
	GetDlgItem(IDC_DECIMAL_LABEL)->ShowWindow(nSelectedType == eftGenericNumber || nSelectedType == eftCurrency || nSelectedType == eftDiag ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_DECIMAL_FLOATING)->ShowWindow(nSelectedType == eftGenericNumber || nSelectedType == eftCurrency || nSelectedType == eftDiag ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_DECIMAL_FIXED)->ShowWindow(nSelectedType == eftGenericNumber || nSelectedType == eftCurrency || nSelectedType == eftDiag ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_PLACES_LABEL)->ShowWindow(nSelectedType == eftGenericNumber || nSelectedType == eftCurrency || nSelectedType == eftDiag ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_PLACES)->ShowWindow(nSelectedType == eftGenericNumber || nSelectedType == eftCurrency || nSelectedType == eftDiag ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_DECIMAL_CHAR_LABEL)->ShowWindow(nSelectedType == eftGenericNumber || nSelectedType == eftCurrency || nSelectedType == eftDiag ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_DECIMAL_CHAR)->ShowWindow(nSelectedType == eftGenericNumber || nSelectedType == eftCurrency || nSelectedType == eftDiag ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_NEGATIVE_PARENS)->ShowWindow(nSelectedType == eftGenericNumber || nSelectedType == eftCurrency ? SW_SHOW : SW_HIDE);

	//Date controls
	GetDlgItem(IDC_DATE_FORMAT_OPTIONS)->ShowWindow(nSelectedType == eftDateTime ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_CUSTOM_DATE_FORMAT)->ShowWindow(nSelectedType == eftDateTime ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_CUSTOM_DATE_LABEL)->ShowWindow(nSelectedType == eftDateTime ? SW_SHOW : SW_HIDE);

	//Currency controls
	GetDlgItem(IDC_CURRENCY_SYMBOL_LABEL)->ShowWindow(nSelectedType == eftCurrency ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_CURRENCY_SYMBOL_TEXT)->ShowWindow(nSelectedType == eftCurrency ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_CURRENCY_PLACEMENT_LABEL)->ShowWindow(nSelectedType == eftCurrency ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_CURRENCY_PLACEMENT)->ShowWindow(nSelectedType == eftCurrency ? SW_SHOW : SW_HIDE);

	//Phone number controls
	GetDlgItem(IDC_PHONE_FORMAT_OPTIONS)->ShowWindow(nSelectedType == eftPhoneNumber ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_CUSTOM_PHONE_FORMAT)->ShowWindow(nSelectedType == eftPhoneNumber ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_CUSTOM_PHONE_LABEL)->ShowWindow(nSelectedType == eftPhoneNumber ? SW_SHOW : SW_HIDE);

	//SSN controls
	GetDlgItem(IDC_SSN_INCLUDE_HYPHENS)->ShowWindow(nSelectedType == eftSSN ? SW_SHOW : SW_HIDE);

	//Bool controls
	GetDlgItem(IDC_TRUE_LABEL)->ShowWindow(nSelectedType == eftBool || nSelectedType == eftGender || nSelectedType == eftMarital ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_TRUE_TEXT)->ShowWindow(nSelectedType == eftBool || nSelectedType == eftGender || nSelectedType == eftMarital ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_FALSE_LABEL)->ShowWindow(nSelectedType == eftBool || nSelectedType == eftGender || nSelectedType == eftMarital ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_FALSE_TEXT)->ShowWindow(nSelectedType == eftBool || nSelectedType == eftGender || nSelectedType == eftMarital ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_UNKNOWN_LABEL)->ShowWindow(nSelectedType == eftBool || nSelectedType == eftGender || nSelectedType == eftMarital ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_UNKNOWN_TEXT)->ShowWindow(nSelectedType == eftBool || nSelectedType == eftGender || nSelectedType == eftMarital ? SW_SHOW : SW_HIDE);
	if(nSelectedType == eftBool) {
		SetDlgItemText(IDC_TRUE_LABEL, "Value when true:");
		SetDlgItemText(IDC_FALSE_LABEL, "false:");
		SetDlgItemText(IDC_UNKNOWN_LABEL, "unknown:");
	}
	else if(nSelectedType == eftGender) {
		//It was easier to just use the same controls, sorry for the implied value judgement.
		SetDlgItemText(IDC_TRUE_LABEL, "Value when male:");
		SetDlgItemText(IDC_FALSE_LABEL, "female:");
		SetDlgItemText(IDC_UNKNOWN_LABEL, "unknown:");
	}
	else if(nSelectedType == eftMarital) {
		SetDlgItemText(IDC_TRUE_LABEL, "Value when single:");
		SetDlgItemText(IDC_FALSE_LABEL, "married:");
		SetDlgItemText(IDC_UNKNOWN_LABEL, "other:");
	}

	//Diag controls.
	//These were all covered under eftGenericNumber.

	//Advanced controls
	GetDlgItem(IDC_ADVANCED_FIELD)->ShowWindow(nSelectedType == eftAdvanced ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_ADVANCED_LABEL)->ShowWindow(nSelectedType == eftAdvanced ? SW_SHOW : SW_HIDE);

	//EMN Item controls
	GetDlgItem(IDC_SENTENCE_FORMAT)->ShowWindow(nSelectedType == eftEmnItem ? SW_SHOW : SW_HIDE);

	//For all fixed width fields.
	GetDlgItem(IDC_FIXED_WIDTH_PLACES_LABEL)->ShowWindow(bFixedWidth ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_FIXED_WIDTH_PLACES)->ShowWindow(bFixedWidth ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_RIGHT_JUSTIFY)->ShowWindow(bFixedWidth ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_FILL_CHAR)->ShowWindow(bFixedWidth ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_FILL_CHAR_LABEL)->ShowWindow(bFixedWidth ? SW_SHOW : SW_HIDE);


	//Now, fill in those controls based on the format.
	if(nNewSel != -1) {
		CString strFormat = VarString(m_pSelectFields->GetValue(nNewSel, flscFormat));

		//All format fields begin with Length|Right-justified|fill character (though the defaults differ).
		CString strPlaces;
		BOOL bRightJustified = FALSE;
		CString strFillCharacter;
		
		switch(nSelectedType) {
		case eftPlaceholder:
			{
				//Length|Right-justified|fill character|Text for the placeholder
				//5     |0              |_             |""
				strPlaces = GetNthField(strFormat, 0, "5");
				bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
				strFillCharacter = GetNthField(strFormat, 2, " ");
				if(strFillCharacter == "") strFillCharacter = " ";

				CString strText = GetNthField(strFormat, 3, "");
				strText.Replace("\r", "\\r");
				strText.Replace("\n", "\\n");
				SetDlgItemText(IDC_PLACEHOLDER_TEXT, strText);
			}
			break;

		case eftGenericText:
		case eftGenericNtext:
			{
				//Length|Right-justified|fill character|Capitalized
				//10    |0              |_             |0
				strPlaces = GetNthField(strFormat, 0, "10");
				bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
				strFillCharacter = GetNthField(strFormat, 2, " ");
				if(strFillCharacter == "") strFillCharacter = " ";
				CheckDlgButton(IDC_CAPITALIZE_TEXT, GetNthField(strFormat, 3, "0") == "1" ? BST_CHECKED : BST_UNCHECKED);
			}
			break;

		case eftGenericNumber:
			{
				//Number:
				//Length|Right-justified|Fill character|Fixed?|# of places|Decimal character|Use ()
				//10    |1              |0             |0     |2          |.                |0
				strPlaces = GetNthField(strFormat, 0, "10");
				bRightJustified = (GetNthField(strFormat, 1, "1") == "1");
				strFillCharacter = GetNthField(strFormat, 2, "0");
				if(strFillCharacter == "") strFillCharacter = "0";

				if(GetNthField(strFormat, 3, "0") == "1") {
					CheckRadioButton(IDC_DECIMAL_FLOATING, IDC_DECIMAL_FIXED, IDC_DECIMAL_FIXED);
					OnDecimalFixed();
				}
				else {
					CheckRadioButton(IDC_DECIMAL_FLOATING, IDC_DECIMAL_FIXED, IDC_DECIMAL_FLOATING);
					OnDecimalFloating();
				}
				CString strPlaces = GetNthField(strFormat, 4, "2");
				if(strPlaces.IsEmpty()) {
					SetDlgItemText(IDC_PLACES, "0");
				}
				else {
					SetDlgItemText(IDC_PLACES, strPlaces);
				}
				CString strDecimalChar = GetNthField(strFormat, 5, ".");
				SetDlgItemText(IDC_DECIMAL_CHAR, strDecimalChar);
				if(GetNthField(strFormat, 6, "0") == "1") {
					CheckDlgButton(IDC_NEGATIVE_PARENS, BST_CHECKED);
				}
				else {
					CheckDlgButton(IDC_NEGATIVE_PARENS, BST_UNCHECKED);
				}
			}
			break;

		case eftDateTime:
			{
				//Length|Right-justified|Fill character|Format
				//10    |0              |_             |%c
				strPlaces = GetNthField(strFormat, 0, "10");
				bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
				strFillCharacter = GetNthField(strFormat, 2, " ");
				if(strFillCharacter == "") strFillCharacter = " ";

				CString strFmt = GetNthField(strFormat, 3, "%c");
				if(-1 == m_pDateOptions->FindByColumn(2, _bstr_t(strFmt), 0, VARIANT_TRUE)) {
					m_pDateOptions->SetSelByColumn(0, (long)1);
					m_pDateOptions->PutValue(m_pDateOptions->CurSel, 2, _bstr_t(strFmt));
				}
				OnSelChosenDateOptions(m_pDateOptions->CurSel);
			}
			break;
		case eftCurrency:
			{
				//Currency:
				//Length|Right-justified|Fill character|Fixed?|# of places|Decimal character|Use ()|Currency Symbol|Symbol Placement
				//10    |1              |_             |1     |2          |.                |1     |$              |1
				strPlaces = GetNthField(strFormat, 0, "10");
				bRightJustified = (GetNthField(strFormat, 1, "1") == "1");
				strFillCharacter = GetNthField(strFormat, 2, " ");
				if(strFillCharacter == "") strFillCharacter = " ";

				if(GetNthField(strFormat, 3, "1") == "1") {
					CheckRadioButton(IDC_DECIMAL_FLOATING, IDC_DECIMAL_FIXED, IDC_DECIMAL_FIXED);
					OnDecimalFixed();
				}
				else {
					CheckRadioButton(IDC_DECIMAL_FLOATING, IDC_DECIMAL_FIXED, IDC_DECIMAL_FLOATING);
					OnDecimalFloating();
				}
				CString strPlaces = GetNthField(strFormat, 4, "2");
				if(strPlaces.IsEmpty()) {
					SetDlgItemText(IDC_PLACES, "0");
				}
				else {
					SetDlgItemText(IDC_PLACES, strPlaces);
				}
				CString strDecimalChar = GetNthField(strFormat, 5, ".");
				SetDlgItemText(IDC_DECIMAL_CHAR, strDecimalChar);
				if(GetNthField(strFormat, 6, "1") == "1") {
					CheckDlgButton(IDC_NEGATIVE_PARENS, BST_CHECKED);
				}
				else {
					CheckDlgButton(IDC_NEGATIVE_PARENS, BST_UNCHECKED);
				}
				CString strCurrency = GetNthField(strFormat, 7, "$");
				SetDlgItemText(IDC_CURRENCY_SYMBOL_TEXT, strCurrency);
				long nPlacement = atol(GetNthField(strFormat, 8, "1"));
				if(nPlacement >= 0 && nPlacement <= 4) m_pCurrencyPlacement->CurSel = nPlacement;
				else m_pCurrencyPlacement->CurSel = 1;
			}
			break;

		case eftPhoneNumber:
			{
				//Length|Right-justified|Fill character|Format
				//14    |0              |_             |(###) ###-####
				strPlaces = GetNthField(strFormat, 0, "14");
				bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
				strFillCharacter = GetNthField(strFormat, 2, " ");
				if(strFillCharacter == "") strFillCharacter = " ";

				CString strFmt = GetNthField(strFormat, 3, "(###) ###-####");
				if(-1 == m_pPhoneOptions->FindByColumn(2, _bstr_t(strFmt), 0, VARIANT_TRUE)) {
					m_pPhoneOptions->SetSelByColumn(0, (long)1);
					m_pPhoneOptions->PutValue(m_pPhoneOptions->CurSel, 2, _bstr_t(strFmt));
				}
				OnSelChosenPhoneFormatOptions(m_pPhoneOptions->CurSel);
			}
			break;

		case eftSSN:
			{
				//Length|Right-justified|Fill character|Include hyphens
				//11    |0              |_             |1
				strPlaces = GetNthField(strFormat, 0, "11");
				bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
				strFillCharacter = GetNthField(strFormat, 2, " ");
				if(strFillCharacter == "") strFillCharacter = " ";

				if(GetNthField(strFormat, 3, "1") == "1") {
					CheckDlgButton(IDC_SSN_INCLUDE_HYPHENS, BST_CHECKED);
				}
				else {
					CheckDlgButton(IDC_SSN_INCLUDE_HYPHENS, BST_UNCHECKED);
				}
			}
			break;
		
		case eftBool:
			//Length|Right-justified|Fill character|True value|False value|Unknown value
			//1     |0              |_             |T         |F          |U
			{
				strPlaces = GetNthField(strFormat, 0, "1");
				bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
				strFillCharacter = GetNthField(strFormat, 2, " ");
				if(strFillCharacter == "") strFillCharacter = " ";

				SetDlgItemText(IDC_TRUE_TEXT, GetNthField(strFormat, 3, "Y"));
				SetDlgItemText(IDC_FALSE_TEXT, GetNthField(strFormat, 4, "N"));
				SetDlgItemText(IDC_UNKNOWN_TEXT, GetNthField(strFormat, 5, "U"));
			}
			break;

		case eftGender:
			//Length|Right-justified|Fill character|Male value|Female value|Unknown value
			//1     |0              |_             |M         |F           |U
			{
				strPlaces = GetNthField(strFormat, 0, "1");
				bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
				strFillCharacter = GetNthField(strFormat, 2, " ");
				if(strFillCharacter == "") strFillCharacter = " ";

				SetDlgItemText(IDC_TRUE_TEXT, GetNthField(strFormat, 3, "M"));
				SetDlgItemText(IDC_FALSE_TEXT, GetNthField(strFormat, 4, "F"));
				SetDlgItemText(IDC_UNKNOWN_TEXT, GetNthField(strFormat, 5, "U"));
			}
			break;

		case eftMarital:
			//Length|Right-justified|Fill character|Single value|Married value|Unknown value
			//1     |0              |_             |S           |M            |U
			{
				strPlaces = GetNthField(strFormat, 0, "1");
				bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
				strFillCharacter = GetNthField(strFormat, 2, " ");
				if(strFillCharacter == "") strFillCharacter = " ";

				SetDlgItemText(IDC_TRUE_TEXT, GetNthField(strFormat, 3, "S"));
				SetDlgItemText(IDC_FALSE_TEXT, GetNthField(strFormat, 4, "M"));
				SetDlgItemText(IDC_UNKNOWN_TEXT, GetNthField(strFormat, 5, "U"));
			}
			break;

		case eftDiag:
			{
				//Number:
				//Length|Right-justified|Fill character|Fixed?|# of places|Decimal character
				//6     |0              |_             |0     |2          |.
				strPlaces = GetNthField(strFormat, 0, "6");
				bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
				strFillCharacter = GetNthField(strFormat, 2, " ");
				if(strFillCharacter == "") strFillCharacter = " ";

				if(GetNthField(strFormat, 3, "0") == "1") {
					CheckRadioButton(IDC_DECIMAL_FLOATING, IDC_DECIMAL_FIXED, IDC_DECIMAL_FIXED);
					OnDecimalFixed();
				}
				else {
					CheckRadioButton(IDC_DECIMAL_FLOATING, IDC_DECIMAL_FIXED, IDC_DECIMAL_FLOATING);
					OnDecimalFloating();
				}
				CString strPlaces = GetNthField(strFormat, 4, "2");
				if(strPlaces.IsEmpty()) {
					SetDlgItemText(IDC_PLACES, "0");
				}
				else {
					SetDlgItemText(IDC_PLACES, strPlaces);
				}
				CString strDecimalChar = GetNthField(strFormat, 5, ".");
				SetDlgItemText(IDC_DECIMAL_CHAR, strDecimalChar);
			}
			break;

		case eftAdvanced:
			{
				//Length|Right-justified|fill character|field
				//5     |0              |_             |""
				strPlaces = GetNthField(strFormat, 0, "5");
				bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
				strFillCharacter = GetNthField(strFormat, 2, " ");
				if(strFillCharacter == "") strFillCharacter = " ";

				SetDlgItemText(IDC_ADVANCED_FIELD, GetNthField(strFormat, 3, ""));
			}
			break;

		case eftEmnItem:
			{
				//Length|Right-justified|fill character|Sentence format
				//10    |0              |_             |0
				strPlaces = GetNthField(strFormat, 0, "5");
				bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
				strFillCharacter = GetNthField(strFormat, 2, " ");
				if(strFillCharacter == "") strFillCharacter = " ";
				CheckDlgButton(IDC_SENTENCE_FORMAT, GetNthField(strFormat, 3, "0") == "1" ? BST_CHECKED : BST_UNCHECKED);
			}
			break;

		default:
			break;

		}

		if(bFixedWidth) {
			//Length
			SetDlgItemText(IDC_FIXED_WIDTH_PLACES, strPlaces);
			//Right-justified?
			CheckDlgButton(IDC_RIGHT_JUSTIFY, bRightJustified ? BST_CHECKED : BST_UNCHECKED);

			//Fill character.
			SetDlgItemText(IDC_FILL_CHAR, strFillCharacter.Left(1));
		}

		UpdateSampleOutput();
	}

	//They can always go back, but they can only go next if they have at least one field selected.
	((CExportWizardDlg*)GetParent())->SetWizardButtons(PSWIZB_BACK|(m_pSelectFields->GetRowCount()?PSWIZB_NEXT:0));
}

void CExportWizardFieldsDlg::OnDblClickCellAvailableExportFields(long nRowIndex, short nColIndex) 
{
	OnAddExportField();
}

void CExportWizardFieldsDlg::OnDblClickCellSelectedExportFields(long nRowIndex, short nColIndex) 
{
	OnRemoveExportField();
}

void CExportWizardFieldsDlg::OnSelChangedAvailableExportFields(long nNewSel) 
{
	GetDlgItem(IDC_ADD_EXPORT_FIELD)->EnableWindow(nNewSel != -1);
}

void CExportWizardFieldsDlg::OnChangePlaceholderText() 
{
	UpdateFormat();
}

void CExportWizardFieldsDlg::OnCapitalizeText() 
{
	UpdateFormat();
}

void CExportWizardFieldsDlg::OnChangeFillChar() 
{
	UpdateFormat();
}

void CExportWizardFieldsDlg::OnRightJustify() 
{
	UpdateFormat();
}

void CExportWizardFieldsDlg::UpdateFormat()
{
	long nRow = m_pSelectFields->CurSel;
	if(nRow == -1) {
		//This function shouldn't have been called!
		ASSERT(FALSE);
		return;
	}

	ExportFieldType eft = (ExportFieldType)VarLong(m_pSelectFields->GetValue(nRow,flscType));
	bool bFixedWidth = (((CExportWizardDlg*)GetParent())->m_eotOutputType == eotFixedWidth);
	
	//The format always starts with Length|Right-justified|Fill character
	CString strFormat;
	if(bFixedWidth) {
		CString strLength;
		strLength.Format("%i", GetDlgItemInt(IDC_FIXED_WIDTH_PLACES));
		strFormat += strLength + "|";
		strFormat += IsDlgButtonChecked(IDC_RIGHT_JUSTIFY) ? "1|" : "0|";
		CString strFillChar;
		GetDlgItemText(IDC_FILL_CHAR, strFillChar);
		if(strFillChar.IsEmpty()) {
			strFormat += " |";
		}
		else {
			//TES 6/4/2007 - PLID 26160 - Handle escaped characters.
			strFormat += FormatForDelimitedField(strFillChar.Left(1), '|', '\\') + "|";
		}
	}
	else {
		strFormat += "10|||";
	}

	switch(eft) {
	case eftPlaceholder:
		{
			//Length|Right-justified|Fill character|Placeholder text
			CString str;
			GetDlgItemText(IDC_PLACEHOLDER_TEXT, str);
			str.Replace("\\r", "\r");
			str.Replace("\\n", "\n");
			//TES 6/4/2007 - PLID 26160 - Handle escaped characters.
			strFormat += FormatForDelimitedField(str, '|', '\\');
		}
		break;
	case eftGenericText:
	case eftGenericNtext:
		{
			//Length|Right-justified|Fill character|Capitalized
			CString str = IsDlgButtonChecked(IDC_CAPITALIZE_TEXT) ? "1" : "0";
			strFormat += str;
		}
		break;

	case eftGenericNumber:
		{
			//Length|Right-justified|Fill character|Fixed?|# of places|Decimal character|Use ()
			CString str = IsDlgButtonChecked(IDC_DECIMAL_FIXED) ? "1|" : "0|";
			CString strPlaces;
			GetDlgItemText(IDC_PLACES, strPlaces);
			if(strPlaces.IsEmpty()) {
				str += "0|";
			}
			else {
				str += strPlaces + "|";
			}
			CString strDecimalChar;
			GetDlgItemText(IDC_DECIMAL_CHAR, strDecimalChar);
			//TES 6/4/2007 - PLID 26160 - Handle escaped characters.
			str += FormatForDelimitedField(strDecimalChar, '|', '\\') + "|";
			str += IsDlgButtonChecked(IDC_NEGATIVE_PARENS) ? "1" : "0";

			strFormat += str;
		}
		break;

	case eftDateTime:
		{
			//Length|Right-justified|Fill character|Format
			CString str;
			GetDlgItemText(IDC_CUSTOM_DATE_FORMAT, str);
			//TES 6/4/2007 - PLID 26160 - Handle escaped characters.
			strFormat += FormatForDelimitedField(str, '|', '\\');
		}
		break;

	case eftCurrency:
		{
			//Length|Right-justified|Fill character|Fixed?|# of places|Decimal character|Use ()|Currency Symbol|Symbol Placement
			CString str = IsDlgButtonChecked(IDC_DECIMAL_FIXED) ? "1|" : "0|";
			CString strPlaces;
			GetDlgItemText(IDC_PLACES, strPlaces);
			if(strPlaces.IsEmpty()) {
				str += "0|";
			}
			else {
				str += strPlaces + "|";
			}
			CString strDecimalChar;
			GetDlgItemText(IDC_DECIMAL_CHAR, strDecimalChar);
			//TES 6/4/2007 - PLID 26160 - Handle escaped characters.
			str += FormatForDelimitedField(strDecimalChar, '|', '\\') + "|";
			str += IsDlgButtonChecked(IDC_NEGATIVE_PARENS) ? "1|" : "0|";
			CString strCurrency;
			GetDlgItemText(IDC_CURRENCY_SYMBOL_TEXT, strCurrency);
			//TES 6/4/2007 - PLID 26160 - Handle escaped characters.
			str += FormatForDelimitedField(strCurrency, '|', '\\') + "|";
			CString strPlacement;
			strPlacement.Format("%li", m_pCurrencyPlacement->CurSel == -1 ? 1 : m_pCurrencyPlacement->CurSel);
			str += strPlacement;

			strFormat += str;
		}
		break;

	case eftPhoneNumber:
		{
			//Length|Right-justified|Fill character|Format
			CString str;
			GetDlgItemText(IDC_CUSTOM_PHONE_FORMAT, str);
			//TES 6/4/2007 - PLID 26160 - Handle escaped characters.
			strFormat += FormatForDelimitedField(str, '|', '\\');
		}
		break;

	case eftSSN:
		{
			//Length|Right-justified|Fill character|Use Hyphens
			strFormat += IsDlgButtonChecked(IDC_SSN_INCLUDE_HYPHENS) ? "1" : "0";
		}
		break;

	case eftBool:
		//Length|Right-justified|Fill character|True value|False value|Unknown value
	case eftGender:
		//Length|Right-justified|Fill character|Male value|Female value|Unknown value
	case eftMarital:
		//Length|Right-justified|Fill character|Single value|Married value|Unknown value
		{
			CString str;
			GetDlgItemText(IDC_TRUE_TEXT, str);
			//TES 6/4/2007 - PLID 26160 - Handle escaped characters.
			strFormat += FormatForDelimitedField(str, '|', '\\') + "|";
			GetDlgItemText(IDC_FALSE_TEXT, str);
			//TES 6/4/2007 - PLID 26160 - Handle escaped characters.
			strFormat += FormatForDelimitedField(str, '|', '\\') + "|";
			GetDlgItemText(IDC_UNKNOWN_TEXT, str);
			//TES 6/4/2007 - PLID 26160 - Handle escaped characters.
			strFormat += FormatForDelimitedField(str, '|', '\\');;
		}
		break;

	case eftDiag:
		{
			//Length|Right-justified|Fill character|Fixed?|# of places|Decimal character
			CString str = IsDlgButtonChecked(IDC_DECIMAL_FIXED) ? "1|" : "0|";
			CString strPlaces;
			GetDlgItemText(IDC_PLACES, strPlaces);
			if(strPlaces.IsEmpty()) {
				str += "0|";
			}
			else {
				str += strPlaces + "|";
			}
			CString strDecimalChar;
			GetDlgItemText(IDC_DECIMAL_CHAR, strDecimalChar);
			//TES 6/4/2007 - PLID 26160 - Handle escaped characters.
			str += FormatForDelimitedField(strDecimalChar, '|', '\\');;

			strFormat += str;
		}
		break;

	case eftAdvanced:
		{
			//Length|Right-justified|Fill character|field
			CString str;
			GetDlgItemText(IDC_ADVANCED_FIELD, str);
			//TES 6/4/2007 - PLID 26160 - Handle escaped characters.
			strFormat += FormatForDelimitedField(str, '|', '\\');;
		}
		break;

	case eftEmnItem:
		{
			//Length|Right-justified|fill character|Sentence format
			strFormat += IsDlgButtonChecked(IDC_SENTENCE_FORMAT) ? "1" : "0";
		}
		break;

	default:
		break;
	}

	m_pSelectFields->PutValue(nRow, flscFormat, _bstr_t(strFormat));
	UpdateSampleOutput();
}

BOOL CExportWizardFieldsDlg::OnSetActive()
{
	// (z.manning 2009-12-10 14:21) - PLID 36519 - Skip this sheet for history exports
	if(((CExportWizardDlg*)GetParent())->m_ertRecordType == ertHistory) {
		return FALSE;
	}

	if(m_nBasedOn == -1) {
		//This is our first time to this tab.  Load both lists.
		LoadActiveFields();

		//Go through our selected list and generate it from our parent.
		long nCurSel = m_pSelectFields->CurSel;
		m_pSelectFields->Clear();
		for(int i = 0; i < ((CExportWizardDlg*)GetParent())->m_arExportFields.GetSize(); i++) {

			ExportFieldType eft = GetFieldByID(((CExportWizardDlg*)GetParent())->m_arExportFields.GetAt(i).nID).eftType;
			CString strName;
			COLORREF clr = 0; // (c.haag 2007-02-02 08:54) - PLID 24428 - Now supports coloring rows
			long nDynamicID = ((CExportWizardDlg*)GetParent())->m_arExportFields.GetAt(i).nDynamicID;
			if(eft == eftEmnItem) {
				ADODB::_RecordsetPtr rs = CreateRecordset("SELECT Name, DataSubType FROM EMRInfoT INNER JOIN EmrInfoMasterT ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID WHERE EmrInfoMasterT.ID = %li", nDynamicID);
				if(!rs->eof) {
					strName = AdoFldString(rs, "Name","");
					BYTE cDataSubType = AdoFldByte(rs, "DataSubType");
					// (c.haag 2007-02-02 08:54) - PLID 24428 - Color the master Current Medications item in blue
					// (c.haag 2007-04-02 12:39) - PLID 25458 - Color the Allergies item in blue
					if (eistCurrentMedicationsTable == cDataSubType ||
						eistAllergiesTable == cDataSubType) {
						clr = RGB(0,0,255);
					}
				}
				else {
					//the item was deleted!
					continue;
				}
				rs->Close();
			}
			else {				
				// (a.walling 2010-10-04 15:18) - PLID 40738 - Blue-ify any unrestricted fields
				long nFieldID = ((CExportWizardDlg*)GetParent())->m_arExportFields.GetAt(i).nID;
				// (a.walling 2010-10-05 15:01) - PLID 40822
				// (j.jones 2010-12-07 11:15) - PLID 41736 - we no longer restrict fields
				/*
				if (((CExportWizardDlg*)GetParent())->m_ertRecordType == ertPatients && GetPatientExportRestrictions().Enabled(false) && !GetPatientExportRestrictions().IsFieldRestricted(nFieldID)) {
					clr = RGB(0x00, 0x00, 0xFF);
				}
				*/
				strName = GetFieldByID(nFieldID).GetDisplayName();
			}

			IRowSettingsPtr pRow = m_pSelectFields->GetRow(-1);			
			pRow->PutValue(flscID, ((CExportWizardDlg*)GetParent())->m_arExportFields.GetAt(i).nID);			
			pRow->PutValue(flscName, _bstr_t(strName));
			pRow->PutValue(flscType, (long)eft);
			pRow->PutValue(flscDynamicID, nDynamicID);
			pRow->PutValue(flscFormat, _bstr_t(((CExportWizardDlg*)GetParent())->m_arExportFields.GetAt(i).strFormat));
			pRow->PutValue(flscHasAJoinOrFromClause, GetFieldByID(((CExportWizardDlg*)GetParent())->m_arExportFields.GetAt(i).nID).bHasAJoinOrFromClause ? "*" : "");
			// (c.haag 2007-02-02 08:54) - PLID 24428 - Now supports coloring rows
			if (0 != clr) pRow->ForeColor = clr;
			
			m_pSelectFields->AddRow(pRow);
		}
		m_pSelectFields->CurSel = nCurSel;
	}
	else {
		//Has what we're based on changed?
		if(m_nBasedOn != ((CExportWizardDlg*)GetParent())->m_ertRecordType) {
			m_pAvailFields->Clear();
			m_pSelectFields->Clear();

			LoadActiveFields();
		}
	}

	m_nBasedOn = ((CExportWizardDlg*)GetParent())->m_ertRecordType;

	GetDlgItem(IDC_SHOW_ALL_ITEMS)->ShowWindow((m_nBasedOn == ertEMNs) ? SW_SHOWNA:SW_HIDE);
	// (a.walling 2010-10-04 15:18) - PLID 40738 - Explain why some fields are blue.
	// (a.walling 2010-10-05 15:01) - PLID 40822
	// (j.jones 2010-12-07 11:03) - PLID 41736 - we no longer restrict specific fields, but
	// if we ever chose to bring that ability back, just remove this SW_HIDE call and comment in
	// the remainder of this code
	GetDlgItem(IDC_EXPORT_RESTRICTED_FIELDS)->ShowWindow(SW_HIDE /*(m_nBasedOn == ertPatients && GetPatientExportRestrictions().Enabled(false)) ? SW_SHOWNA : SW_HIDE*/);

	OnSelChangedAvailableExportFields(m_pAvailFields->CurSel);
	OnSelChangedSelectedExportFields(m_pSelectFields->CurSel);

	//They can always go back, but they can only go next if they have at least one field selected.
	((CExportWizardDlg*)GetParent())->SetWizardButtons(PSWIZB_BACK|(m_pSelectFields->GetRowCount()?PSWIZB_NEXT:0));

	((CExportWizardDlg*)GetParent())->SetTitle("Select fields to include in export \"" + ((CExportWizardDlg*)GetParent())->m_strName + "\"");

	// (j.jones 2006-03-22 12:06) - PLID 19384 - the * fields are complex and add more JOIN/FROM clauses
	// to the export, of which there is a maximum of 256. So warn if we have more thn 200 such fields.
	long nJoinFieldCount = 0;
	if(m_pSelectFields->GetRowCount() > 200) {
		for(int i=0; i < m_pSelectFields->GetRowCount(); i++)  {
			if(VarString(m_pSelectFields->GetValue(i, flscHasAJoinOrFromClause), "") == "*") {
				nJoinFieldCount++;
			}
		}

		if(nJoinFieldCount > 200) {
			CString str;
			str.Format("There is a limit of 200 export fields that have the * indicator.\n"
				"Please remove at least %li of these * fields before continuing.", nJoinFieldCount - 200);
			AfxMessageBox(str);
		}
	}

	return CPropertyPage::OnSetActive();
}

BOOL CExportWizardFieldsDlg::OnKillActive()
{
	((CExportWizardDlg*)GetParent())->m_arExportFields.RemoveAll();

	// (j.jones 2006-03-22 12:06) - PLID 19384 - the * fields are complex and add more JOIN/FROM clauses
	// to the export, of which there is a maximum of 256. So warn if we have more thn 200 such fields.
	long nJoinFieldCount = 0;
	if(m_pSelectFields->GetRowCount() > 200) {
		for(int i=0; i < m_pSelectFields->GetRowCount(); i++)  {
			if(VarString(m_pSelectFields->GetValue(i, flscHasAJoinOrFromClause), "") == "*") {
				nJoinFieldCount++;
			}
		}

		if(nJoinFieldCount > 200) {
			CString str;
			str.Format("There is a limit of 200 export fields that have the * indicator.\n"
				"Please remove at least %li of these * fields before continuing.", nJoinFieldCount - 200);
			AfxMessageBox(str);
			return FALSE;
		}
	}

	for(int i = 0; i < m_pSelectFields->GetRowCount(); i++) {
		SelectedExportField sef;
		sef.nID = VarLong(m_pSelectFields->GetValue(i,flscID));
		sef.nDynamicID = VarLong(m_pSelectFields->GetValue(i,flscDynamicID));
		sef.strFormat = VarString(m_pSelectFields->GetValue(i,flscFormat));
		sef.bHasAJoinOrFromClause = GetFieldByID(sef.nID).bHasAJoinOrFromClause;
		if(!IsValidField(sef)) {
			MsgBox("At least one field has invalid format options selected.  Please review this field before continuing.");
			m_pSelectFields->CurSel = i;
			OnSelChangedSelectedExportFields(i);
			return FALSE;
		}
		((CExportWizardDlg*)GetParent())->m_arExportFields.Add(sef);
	}

	return CPropertyPage::OnKillActive();
}

void CExportWizardFieldsDlg::OnSetfocusFillChar() 
{
	((CNxEdit*)GetDlgItem(IDC_FILL_CHAR))->SetSel(0,-1);
}

void CExportWizardFieldsDlg::OnDecimalFloating() 
{
	GetDlgItem(IDC_PLACES_LABEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_PLACES)->EnableWindow(FALSE);
	UpdateFormat();
}

void CExportWizardFieldsDlg::OnDecimalFixed() 
{
	GetDlgItem(IDC_PLACES_LABEL)->EnableWindow(TRUE);
	GetDlgItem(IDC_PLACES)->EnableWindow(TRUE);
	UpdateFormat();
}

void CExportWizardFieldsDlg::OnChangeDecimalChar() 
{
	UpdateFormat();
}

void CExportWizardFieldsDlg::OnChangePlaces() 
{
	UpdateFormat();	
}

void CExportWizardFieldsDlg::UpdateSampleOutput()
{
	long nRow = m_pSelectFields->CurSel;
	if(nRow == -1) {
		//This function shouldn't have been called!
		ASSERT(FALSE);
		return;
	}

	ExportFieldType eft = (ExportFieldType)VarLong(m_pSelectFields->GetValue(nRow,flscType));
	bool bFixedWidth = (((CExportWizardDlg*)GetParent())->m_eotOutputType == eotFixedWidth);

	CString strSampleOutput;
	switch(eft) {
	case eftPlaceholder:
		{
			GetDlgItemText(IDC_PLACEHOLDER_TEXT, strSampleOutput);
		}
		break;
	case eftGenericText:
	case eftGenericNtext:
		{
			strSampleOutput = "Text";
			if(IsDlgButtonChecked(IDC_CAPITALIZE_TEXT))
				strSampleOutput.MakeUpper();
		}
		break;
	case eftGenericNumber:
		{
			if(IsDlgButtonChecked(IDC_DECIMAL_FLOATING)) {
				if(IsDlgButtonChecked(IDC_NEGATIVE_PARENS)) {
					strSampleOutput = "(55";
					CString strDecimal;
					GetDlgItemText(IDC_DECIMAL_CHAR, strDecimal);
					strSampleOutput += strDecimal;
					strSampleOutput += "555)";
				}
				else {
					strSampleOutput = "-55";
					CString strDecimal;
					GetDlgItemText(IDC_DECIMAL_CHAR, strDecimal);
					strSampleOutput += strDecimal;
					strSampleOutput += "555";
				}
			}
			else {
				int nPlaces = GetDlgItemInt(IDC_PLACES);
				strSampleOutput = IsDlgButtonChecked(IDC_NEGATIVE_PARENS) ? "(55" : "-55";
				if(nPlaces > 0) {
					CString strDecimal;
					GetDlgItemText(IDC_DECIMAL_CHAR, strDecimal);
					strSampleOutput += strDecimal;
					if(nPlaces == 1) strSampleOutput += "6";
					else if(nPlaces == 2) strSampleOutput += "56";
					else strSampleOutput += "555";
					for(int i = 3; i < nPlaces; i++) strSampleOutput += "0";
				}
				if(IsDlgButtonChecked(IDC_NEGATIVE_PARENS)) strSampleOutput += ")";
			}
		}
		break;

	case eftDateTime:
		{
			CString strFmt;
			GetDlgItemText(IDC_CUSTOM_DATE_FORMAT, strFmt);
			COleDateTime dtSample(1979,5,29,17,55,55);
			strSampleOutput = dtSample.Format(strFmt);
		}
		break;

	case eftCurrency:
		{
			CString strSymbol;
			GetDlgItemText(IDC_CURRENCY_SYMBOL_TEXT, strSymbol);
			long nPlacement = m_pCurrencyPlacement->CurSel;
			if(nPlacement == -1) nPlacement = 1;
			if(IsDlgButtonChecked(IDC_DECIMAL_FLOATING)) {
				if(IsDlgButtonChecked(IDC_NEGATIVE_PARENS)) {
					if(nPlacement == 0) {
						strSampleOutput = strSymbol + "(55";
					}
					else if(nPlacement == 1) {
						strSampleOutput = "(" + strSymbol + "55";
					}
					else {
						strSampleOutput = "(55";
					}

					CString strDecimal;
					GetDlgItemText(IDC_DECIMAL_CHAR, strDecimal);
					strSampleOutput += strDecimal;
					
					if(nPlacement == 2) {
						strSampleOutput += "555" + strSymbol + ")";
					}
					else if(nPlacement == 3) {
						strSampleOutput += "555)" + strSymbol;
					}
					else {
						strSampleOutput += "555)";
					}
				}
				else {
					if(nPlacement == 0) {
						strSampleOutput = strSymbol + "-55";
					}
					else if(nPlacement == 1) {
						strSampleOutput = "-" + strSymbol + "55";
					}
					else {
						strSampleOutput = "-55";
					}
					
					CString strDecimal;
					GetDlgItemText(IDC_DECIMAL_CHAR, strDecimal);
					strSampleOutput += strDecimal;
					
					if(nPlacement == 2 || nPlacement == 3) {
						strSampleOutput += "555" + strSymbol;
					}
					else {
						strSampleOutput += "555";
					}
				}
			}
			else {
				int nPlaces = GetDlgItemInt(IDC_PLACES);
				if(IsDlgButtonChecked(IDC_NEGATIVE_PARENS)) {
					if(nPlacement == 0) {
						strSampleOutput = strSymbol + "(55";
					}
					else if(nPlacement == 1) {
						strSampleOutput = "(" + strSymbol + "55";
					}
					else {
						strSampleOutput = "(55";
					}
				}
				else {
					if(nPlacement == 0) {
						strSampleOutput = strSymbol + "-55";
					}
					else if(nPlacement == 1) {
						strSampleOutput = "-" + strSymbol + "55";
					}
					else {
						strSampleOutput = "-55";
					}
				}

				if(nPlaces > 0) {
					CString strDecimal;
					GetDlgItemText(IDC_DECIMAL_CHAR, strDecimal);
					strSampleOutput += strDecimal;
					if(nPlaces == 1) strSampleOutput += "6";
					else if(nPlaces == 2) strSampleOutput += "56";
					else strSampleOutput += "555";
					for(int i = 3; i < nPlaces; i++) strSampleOutput += "0";
				}
				if(IsDlgButtonChecked(IDC_NEGATIVE_PARENS)) {
					if(nPlacement == 2) {
						strSampleOutput += strSymbol + ")";
					}
					else if(nPlacement == 3) {
						strSampleOutput += ")" + strSymbol;
					}
					else {
						strSampleOutput += ")";
					}
				}
				else if(nPlacement == 2 || nPlacement == 3) {
					strSampleOutput += strSymbol;
				}
			}
		}
		break;

	case eftPhoneNumber:
		{
			CString strFmt;
			GetDlgItemText(IDC_CUSTOM_PHONE_FORMAT, strFmt);
			CString strSample = "5555550126";
			strSampleOutput = FormatPhone(strSample, strFmt);
		}
		break;

	case eftSSN:
		if(IsDlgButtonChecked(IDC_SSN_INCLUDE_HYPHENS)) {
			strSampleOutput = "123-45-6789";
		}
		else {
			strSampleOutput = "123456789";
		}
		break;

	case eftBool:
	case eftGender:
	case eftMarital:
		{
			GetDlgItemText(IDC_TRUE_TEXT, strSampleOutput);
		}
		break;

	case eftDiag:
		{
			if(IsDlgButtonChecked(IDC_DECIMAL_FLOATING)) {
				strSampleOutput = "123";
				CString strDecimal;
				GetDlgItemText(IDC_DECIMAL_CHAR, strDecimal);
				strSampleOutput += strDecimal;
				strSampleOutput += "4";
			}
			else {
				int nPlaces = GetDlgItemInt(IDC_PLACES);
				strSampleOutput = "123";
				if(nPlaces > 0) {
					CString strDecimal;
					GetDlgItemText(IDC_DECIMAL_CHAR, strDecimal);
					strSampleOutput += strDecimal;
					strSampleOutput += "4";
					for(int i = 1; i < nPlaces; i++) strSampleOutput += "0";
				}
			}
		}
		break;

	case eftAdvanced:
		{
			strSampleOutput = "Advanced";
		}
		break;

	case eftEmnItem:
		if(IsDlgButtonChecked(IDC_SENTENCE_FORMAT)) strSampleOutput = VarString(m_pSelectFields->GetValue(nRow,flscName)) + ":<Data>";
		else strSampleOutput = "<Data>";
		break;

	default:
		break;
	}

	if(bFixedWidth) {
		int nLength = GetDlgItemInt(IDC_FIXED_WIDTH_PLACES);
		if(strSampleOutput.GetLength() < nLength) {
			CString strFillChar;
			GetDlgItemText(IDC_FILL_CHAR, strFillChar);
			if(strFillChar == "") strFillChar = " ";
			CString strPad;
			for(int i = 0; i < nLength-strSampleOutput.GetLength(); i++) strPad += strFillChar;
			if(IsDlgButtonChecked(IDC_RIGHT_JUSTIFY)) {
				strSampleOutput = strPad + strSampleOutput;
			}
			else {
				strSampleOutput += strPad;
			}
		}
		else {
			if(IsDlgButtonChecked(IDC_RIGHT_JUSTIFY)) {
				strSampleOutput = strSampleOutput.Right(nLength);
			}
			else {
				strSampleOutput = strSampleOutput.Left(nLength);
			}
		}

		if(IsDlgButtonChecked(IDC_RIGHT_JUSTIFY)) {
			((CNxStatic*)GetDlgItem(IDC_SAMPLE_OUTPUT))->ModifyStyle(SS_LEFT, SS_RIGHT);
		}
		else {
			((CNxStatic*)GetDlgItem(IDC_SAMPLE_OUTPUT))->ModifyStyle(SS_RIGHT, SS_LEFT);
		}
	}
	else {
		((CNxStatic*)GetDlgItem(IDC_SAMPLE_OUTPUT))->ModifyStyle(SS_RIGHT, SS_LEFT);
	}

	SetDlgItemText(IDC_SAMPLE_OUTPUT, strSampleOutput);
}

void CExportWizardFieldsDlg::OnChangeFixedWidthPlaces() 
{
	UpdateFormat();	
}

void CExportWizardFieldsDlg::OnNegativeParens() 
{
	//Change the datalist of currency symbol placement options.
	CString strSymbol;
	GetDlgItemText(IDC_CURRENCY_SYMBOL_TEXT, strSymbol);
	m_pCurrencyPlacement->PutValue(0,0, IsDlgButtonChecked(IDC_NEGATIVE_PARENS) ? _bstr_t(strSymbol + "(55.56)") : _bstr_t(strSymbol + "-55.56"));
	m_pCurrencyPlacement->PutValue(1,0, IsDlgButtonChecked(IDC_NEGATIVE_PARENS) ? _bstr_t("(" + strSymbol + "55.56)") : _bstr_t("-" + strSymbol + "55.56"));
	m_pCurrencyPlacement->PutValue(2,0, IsDlgButtonChecked(IDC_NEGATIVE_PARENS) ? _bstr_t("(55.56" + strSymbol + ")") : _bstr_t("-55.56" + strSymbol));
	m_pCurrencyPlacement->PutValue(3,0, IsDlgButtonChecked(IDC_NEGATIVE_PARENS) ? _bstr_t("(55.56)" + strSymbol) : _bstr_t("-55.56" + strSymbol));
	
	UpdateFormat();
}

void CExportWizardFieldsDlg::OnSelChosenDateOptions(long nRow) 
{
	if(nRow == -1) {
		m_pDateOptions->CurSel = 0;
		OnSelChosenDateOptions(0);
	}
	else {
		if(VarLong(m_pDateOptions->GetValue(nRow,0))) {
			GetDlgItem(IDC_CUSTOM_DATE_FORMAT)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_CUSTOM_DATE_FORMAT)->EnableWindow(FALSE);
		}
		SetDlgItemText(IDC_CUSTOM_DATE_FORMAT, VarString(m_pDateOptions->GetValue(nRow,2)));
	}
	UpdateFormat();
}

void CExportWizardFieldsDlg::OnChangeCustomDateFormat() 
{
	if(GetDlgItem(IDC_CUSTOM_DATE_FORMAT)->IsWindowEnabled()) {
		CString strFmt;
		GetDlgItemText(IDC_CUSTOM_DATE_FORMAT, strFmt);
		m_pDateOptions->PutValue(m_pDateOptions->CurSel, 2, _bstr_t(strFmt));
		UpdateFormat();
	}
}

void CExportWizardFieldsDlg::OnSelChosenCurrencyPlacement(long nRow) 
{
	UpdateFormat();
}

void CExportWizardFieldsDlg::OnChangeCurrencySymbol() 
{
	//Change the datalist of currency symbol placement options.
	CString strSymbol;
	GetDlgItemText(IDC_CURRENCY_SYMBOL_TEXT, strSymbol);
	m_pCurrencyPlacement->PutValue(0,0, IsDlgButtonChecked(IDC_NEGATIVE_PARENS) ? _bstr_t(strSymbol + "(55.56)") : _bstr_t(strSymbol + "-55.56"));
	m_pCurrencyPlacement->PutValue(1,0, IsDlgButtonChecked(IDC_NEGATIVE_PARENS) ? _bstr_t("(" + strSymbol + "55.56)") : _bstr_t("-" + strSymbol + "55.56"));
	m_pCurrencyPlacement->PutValue(2,0, IsDlgButtonChecked(IDC_NEGATIVE_PARENS) ? _bstr_t("(55.56" + strSymbol + ")") : _bstr_t("-55.56" + strSymbol));
	m_pCurrencyPlacement->PutValue(3,0, IsDlgButtonChecked(IDC_NEGATIVE_PARENS) ? _bstr_t("(55.56)" + strSymbol) : _bstr_t("-55.56" + strSymbol));

	UpdateFormat();
}

void CExportWizardFieldsDlg::OnChangeCustomPhoneFormat() 
{
	if(GetDlgItem(IDC_CUSTOM_PHONE_FORMAT)->IsWindowEnabled()) {
		CString strFmt;
		GetDlgItemText(IDC_CUSTOM_PHONE_FORMAT, strFmt);
		m_pPhoneOptions->PutValue(m_pPhoneOptions->CurSel, 2, _bstr_t(strFmt));
		UpdateFormat();
	}
}

void CExportWizardFieldsDlg::OnSelChosenPhoneFormatOptions(long nRow) 
{
	if(nRow == -1) {
		m_pPhoneOptions->CurSel = 0;
		OnSelChosenPhoneFormatOptions(0);
	}
	else {
		if(VarLong(m_pPhoneOptions->GetValue(nRow,0))) {
			GetDlgItem(IDC_CUSTOM_PHONE_FORMAT)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_CUSTOM_PHONE_FORMAT)->EnableWindow(FALSE);
		}
		SetDlgItemText(IDC_CUSTOM_PHONE_FORMAT, VarString(m_pPhoneOptions->GetValue(nRow,2)));
	}
	UpdateFormat();
}

void CExportWizardFieldsDlg::OnSsnIncludeHyphens() 
{
	UpdateFormat();	
}

void CExportWizardFieldsDlg::OnChangeUnknownText() 
{
	UpdateFormat();	
}

void CExportWizardFieldsDlg::OnChangeTrueText() 
{
	UpdateFormat();
}

void CExportWizardFieldsDlg::OnChangeFalseText() 
{
	UpdateFormat();
}

void CExportWizardFieldsDlg::OnChangeAdvancedField() 
{
	UpdateFormat();
}

bool CExportWizardFieldsDlg::IsValidField(SelectedExportField sef)
{
	CExportField ef = GetFieldByID(sef.nID);
	switch(ef.eftType) {
	case eftAdvanced:
		try {
			//Make sure they've entered valid sql.
			CreateRecordset("SELECT TOP 1 %s %s", GetNthField(sef.strFormat, 3, ""), GetExportFromClause(((CExportWizardDlg*)GetParent())->m_ertRecordType));
		}catch(_com_error e) {
			return false;
		}
		return true;
		break;
	default:
		//All other types, we don't know a way they can be invalid.
		return true;
	}
}

void CExportWizardFieldsDlg::LoadActiveFields()
{
	try {
		m_pAvailFields->Clear();
		//Go through all our fields and add them to the available list.
		DWORD dwRecordType = 0;
		switch(((CExportWizardDlg*)GetParent())->m_ertRecordType) {
		case ertPatients:
			dwRecordType = EFS_PATIENTS;
			break;
		case ertAppointments:
			dwRecordType = EFS_APPOINTMENTS;
			break;
		case ertCharges:
			dwRecordType = EFS_CHARGES;
			break;
		case ertPayments:
			dwRecordType = EFS_PAYMENTS;
			break;
		case ertEMNs:
			dwRecordType = EFS_EMNS;
			break;
		}
		for(int i = 0; i < g_nExportFieldCount; i++) {
			if(g_arExportFields[i].dwSupportedRecordTypes & dwRecordType) {
				CExportField ef = g_arExportFields[i];

				if(ef.eftType == eftEmnItem) {
					//Special case!  Add all items which are included on any of the templates they've specified, unless they've
					//told us to include all items.
					CString strTemplateFilter;
					if(IsDlgButtonChecked(IDC_SHOW_ALL_ITEMS)) {
						strTemplateFilter = "1=1";
					}
					else {
						CString strTemplateIDs;
						for(int j = 0; j < ((CExportWizardDlg*)GetParent())->m_arEmnTemplateIDs.GetSize(); j++) {
							CString strID;
							strID.Format("%li,", ((CExportWizardDlg*)GetParent())->m_arEmnTemplateIDs.GetAt(j));
							strTemplateIDs += strID;
						}
						strTemplateIDs.TrimRight(",");
						if(strTemplateIDs.IsEmpty()) strTemplateIDs = "-1";
						strTemplateFilter = "EmrInfoMasterT.ID IN (SELECT EmrInfoMasterID FROM EmrTemplateDetailsT WHERE EmrTemplateDetailsT.TemplateID IN (" + strTemplateIDs + "))";					
					}
					// (c.haag 2008-06-17 18:30) - PLID 30319 - Suppress the built-in EMR text macro item
					// (j.jones 2010-06-04 16:55) - PLID 39029 - do not show the generic table (DataSubType = 3)
					ADODB::_RecordsetPtr rsItems = CreateRecordset("SELECT EmrInfoMasterT.ID, EmrInfoT.Name, EmrInfoT.DataSubType FROM EmrInfoT INNER JOIN EmrInfoMasterT ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID "
						"WHERE DataType <> 4 AND EmrInfoT.ID <> %li AND EMRInfoT.DataSubType <> %li AND %s ORDER BY Name", EMR_BUILT_IN_INFO__TEXT_MACRO, eistGenericTable, strTemplateFilter);
					while(!rsItems->eof) {
						IRowSettingsPtr pRow = m_pAvailFields->GetRow(-1);
						BYTE cDataSubType = AdoFldByte(rsItems, "DataSubType");
						pRow->PutValue(flucID, (long)ef.nID);
						pRow->PutValue(flucName, rsItems->Fields->GetItem("Name")->Value);
						pRow->PutValue(flucType, (long)ef.eftType);
						pRow->PutValue(flucDynamicID, rsItems->Fields->GetItem("ID")->Value);
						pRow->PutValue(flucHasAJoinOrFromClause, ef.bHasAJoinOrFromClause ? "*" : "");
						// (c.haag 2007-02-02 08:53) - PLID 24428 - Color the master Current Medications
						// item in blue
						// (c.haag 2007-04-02 12:40) - PLID 25458 - Color the Allergies item in blue
						if (eistCurrentMedicationsTable == cDataSubType ||
							eistAllergiesTable == cDataSubType) {
							pRow->ForeColor = RGB(0,0,255);
						}
						m_pAvailFields->AddRow(pRow);
						rsItems->MoveNext();
					}
				}
				else {					
					IRowSettingsPtr pRow = m_pAvailFields->GetRow(-1);			
					pRow->PutValue(flucID, (long)ef.nID);
					pRow->PutValue(flucName, _bstr_t(ef.GetDisplayName()));
					pRow->PutValue(flucType, (long)ef.eftType);
					pRow->PutValue(flucDynamicID, (long)-1);
					pRow->PutValue(flucHasAJoinOrFromClause, ef.bHasAJoinOrFromClause ? "*" : "");
					// (a.walling 2010-10-04 15:18) - PLID 40738 - Blue-ify any unrestricted fields
					// (a.walling 2010-10-05 15:01) - PLID 40822
					// (j.jones 2010-12-07 11:14) - PLID 41736 - we no longer restrict fields
					/*
					if (dwRecordType == EFS_PATIENTS && GetPatientExportRestrictions().Enabled(false) && !GetPatientExportRestrictions().IsFieldRestricted(ef.nID)) {
						pRow->ForeColor = RGB(0x00, 0x00, 0xFF);
					}
					*/
					m_pAvailFields->AddRow(pRow);
				}
			}
		}
	}NxCatchAll("Error in CExportWizardFieldsDlg::LoadActiveFields()");
}

void CExportWizardFieldsDlg::OnShowAllItems() 
{
	LoadActiveFields();
}

void CExportWizardFieldsDlg::OnSentenceFormat() 
{
	UpdateFormat();
}
