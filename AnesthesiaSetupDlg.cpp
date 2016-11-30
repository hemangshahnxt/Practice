// AnesthesiaSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GlobalDataUtils.h"
#include "InternationalUtils.h"
#include "AnesthesiaSetupDlg.h"
#include "AnesthesiaInsCoSetupDlg.h"
#include "GlobalDrawingUtils.h"
#include "FacilityFeeNewSchedDlg.h"
#include "SingleSelectDlg.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLUMN_ID		0
#define	COLUMN_HOURS	1
#define	COLUMN_MINUTES	2
#define	COLUMN_FEE		3

#define POS_COLUMN_ID	0
#define POS_COLUMN_NAME	1

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CAnesthesiaSetupDlg dialog


CAnesthesiaSetupDlg::CAnesthesiaSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAnesthesiaSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnesthesiaSetupDlg)
		m_nServiceID = -1; // (j.jones 2007-10-15 10:36) - PLID 27757 - added ServiceID & ServiceCode
		m_strServiceCode = "";
		m_nLocationID = -1;
		m_nAnesthesiaSetupID = -1;
	//}}AFX_DATA_INIT
}


void CAnesthesiaSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnesthesiaSetupDlg)
	DDX_Control(pDX, IDC_BTN_COPY_ANESTH_TO_POS, m_btnCopyToPOS);
	DDX_Control(pDX, IDC_BTN_COPY_ANESTH_TO_CODE, m_btnCopyToCode);
	DDX_Control(pDX, IDC_BTN_DELETE_ANESTH_FEE_OPTION, m_btnDeleteFeeOption);
	DDX_Control(pDX, IDC_BTN_ADD_NEW_ANESTH_FEE_OPTION, m_btnAddFeeOption);
	DDX_Control(pDX, IDC_CHECK_ANESTH_QUOTE_AS_OUTSIDE_FEE, m_checkQuoteAsOutsideFee);
	DDX_Control(pDX, IDC_RADIO_MINUTES, m_radioUseMinutes);
	DDX_Control(pDX, IDC_RADIO_BEGIN_END_TIMES, m_radioUseBeginEndTimes);
	DDX_Control(pDX, IDC_RADIO_USE_LESSER_ANESTH_TIME, m_radioUseLesserTime);
	DDX_Control(pDX, IDC_RADIO_USE_GREATER_ANESTH_TIME, m_radioUseGreaterTime);
	DDX_Control(pDX, IDC_RADIO_ANESTH_FLAT_FEE, m_radioFlatFee);
	DDX_Control(pDX, IDC_RADIO_USE_ANESTH_INCREMENTAL_FEE, m_radioIncrementalFee);
	DDX_Control(pDX, IDC_RADIO_USE_SPECIFIC_ANESTH_FEE_SCHEDULE, m_radioSpecificFeeTable);
	DDX_Control(pDX, IDC_RADIO_USE_SPECIFIC_ANESTH_UNIT_BILLING, m_radioUnitBased);
	DDX_Control(pDX, IDC_EDIT_FLAT_ANESTH_FEE, m_nxeditEditFlatAnesthFee);
	DDX_Control(pDX, IDC_EDIT_ANESTH_INCREMENTAL_BASE_FEE, m_nxeditEditAnesthIncrementalBaseFee);
	DDX_Control(pDX, IDC_EDIT_ANESTH_INCREMENTAL_ADDITIONAL_FEE, m_nxeditEditAnesthIncrementalAdditionalFee);
	DDX_Control(pDX, IDC_EDIT_UNIT_COST, m_nxeditEditUnitCost);
	DDX_Control(pDX, IDC_EDIT_TIME_UNITS, m_nxeditEditTimeUnits);
	DDX_Control(pDX, IDC_ANES_FEE_PAID_TO, m_nxeditAnesFeePaidTo);
	DDX_Control(pDX, IDC_ANESTH_SERVICE_LABEL, m_nxstaticAnesthServiceLabel);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_EDIT_INSURANCE_CO_SETUP, m_btnEditInsuranceCoSetup);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnesthesiaSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAnesthesiaSetupDlg)
	ON_BN_CLICKED(IDC_BTN_EDIT_INSURANCE_CO_SETUP, OnBtnEditInsuranceCoSetup)
	ON_BN_CLICKED(IDC_RADIO_ANESTH_FLAT_FEE, OnRadioFlatFee)
	ON_BN_CLICKED(IDC_RADIO_USE_ANESTH_INCREMENTAL_FEE, OnRadioUseIncrementalFee)
	ON_BN_CLICKED(IDC_RADIO_USE_SPECIFIC_ANESTH_FEE_SCHEDULE, OnRadioUseSpecificFeeSchedule)
	ON_BN_CLICKED(IDC_RADIO_USE_SPECIFIC_ANESTH_UNIT_BILLING, OnRadioUseUnitBased)
	ON_EN_KILLFOCUS(IDC_EDIT_FLAT_ANESTH_FEE, OnKillfocusEditFlatAnesthFee)
	ON_EN_KILLFOCUS(IDC_EDIT_ANESTH_INCREMENTAL_BASE_FEE, OnKillfocusEditIncrementalBaseFee)
	ON_EN_KILLFOCUS(IDC_EDIT_ANESTH_INCREMENTAL_ADDITIONAL_FEE, OnKillfocusEditIncrementalAdditionalFee)
	ON_BN_CLICKED(IDC_BTN_ADD_NEW_ANESTH_FEE_OPTION, OnBtnAddNewFeeOption)
	ON_BN_CLICKED(IDC_BTN_DELETE_ANESTH_FEE_OPTION, OnBtnDeleteFeeOption)
	ON_EN_KILLFOCUS(IDC_EDIT_UNIT_COST, OnKillfocusEditUnitCost)
	ON_EN_KILLFOCUS(IDC_EDIT_TIME_UNITS, OnKillfocusEditTimeUnits)
	ON_BN_CLICKED(IDC_BTN_COPY_ANESTH_TO_CODE, OnBtnCopyAnesthToCode)
	ON_BN_CLICKED(IDC_BTN_COPY_ANESTH_TO_POS, OnBtnCopyAnesthToPos)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnesthesiaSetupDlg message handlers

BOOL CAnesthesiaSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {	

		// (z.manning, 04/30/2008) - PLID 29850 - Set button styles
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAddFeeOption.AutoSet(NXB_NEW);
		m_btnDeleteFeeOption.AutoSet(NXB_DELETE);
		m_btnEditInsuranceCoSetup.AutoSet(NXB_MODIFY);

		// (j.jones 2007-10-15 11:41) - PLID 27757 - display the service code in the label
		CString str;
		str.Format("Anesthesia Configuration for Service Code '%s'", m_strServiceCode);
		SetDlgItemText(IDC_ANESTH_SERVICE_LABEL, str);

		m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_ADMIN, 0)));
	
		m_POSCombo = BindNxDataListCtrl(this,IDC_POS_ANESTH_COMBO,GetRemoteData(),true);
		m_IncrementBaseCombo = BindNxDataListCtrl(this,IDC_ANESTH_INCREMENTAL_BASE_TIME_COMBO,GetRemoteData(),false);
		m_IncrementAdditionalCombo = BindNxDataListCtrl(this,IDC_ANESTH_INCREMENTAL_ADDITIONAL_TIME_COMBO,GetRemoteData(),false);
		m_AnesthFeeTable = BindNxDataListCtrl(this,IDC_ANESTH_FEE_TABLE,GetRemoteData(),false);

		IRowSettingsPtr pRow = m_IncrementBaseCombo->GetRow(-1);
		pRow->PutValue(0,(long)1);
		pRow->PutValue(1,"Hour");
		m_IncrementBaseCombo->AddRow(pRow);
		pRow = m_IncrementBaseCombo->GetRow(-1);
		pRow->PutValue(0,(long)2);
		pRow->PutValue(1,"Half Hour");
		m_IncrementBaseCombo->AddRow(pRow);
		pRow = m_IncrementBaseCombo->GetRow(-1);
		pRow->PutValue(0,(long)3);
		pRow->PutValue(1,"Quarter Hour");
		m_IncrementBaseCombo->AddRow(pRow);

		pRow = m_IncrementAdditionalCombo->GetRow(-1);
		pRow->PutValue(0,(long)1);
		pRow->PutValue(1,"Hour");
		m_IncrementAdditionalCombo->AddRow(pRow);
		pRow = m_IncrementAdditionalCombo->GetRow(-1);
		pRow->PutValue(0,(long)2);
		pRow->PutValue(1,"Half Hour");
		m_IncrementAdditionalCombo->AddRow(pRow);
		pRow = m_IncrementAdditionalCombo->GetRow(-1);
		pRow->PutValue(0,(long)3);
		pRow->PutValue(1,"Quarter Hour");
		m_IncrementAdditionalCombo->AddRow(pRow);

		((CNxEdit*)GetDlgItem(IDC_ANES_FEE_PAID_TO))->LimitText(200);

		m_POSCombo->CurSel = 0;
		m_nLocationID = VarLong(m_POSCombo->GetValue(m_POSCombo->GetCurSel(), POS_COLUMN_ID));
		Load();

	}NxCatchAll("Error loading anesthesia setup.");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CAnesthesiaSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAnesthesiaSetupDlg)
	ON_EVENT(CAnesthesiaSetupDlg, IDC_POS_ANESTH_COMBO, 16 /* SelChosen */, OnSelChosenPosCombo, VTS_I4)
	ON_EVENT(CAnesthesiaSetupDlg, IDC_ANESTH_FEE_TABLE, 9 /* EditingFinishing */, OnEditingFinishingAnesthFeeTable, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CAnesthesiaSetupDlg, IDC_ANESTH_FEE_TABLE, 10 /* EditingFinished */, OnEditingFinishedAnesthFeeTable, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAnesthesiaSetupDlg::OnOK() 
{
	if(!Save())
		return;

	CDialog::OnOK();

	// (j.jones 2005-07-05 15:03) - we used to have only one global unit cost for anesthesia codes
	// so we asked to update the code's standard fee and fee in surgeries to match, for cosmetic purposes only
	// however we can now have altogether different fees for each place of service, so this code is obsolete for now
	/*
	try {
		
		//////////////////////////////////////////////////////////////////////////////////////////
		//now that all the setup has been saved, check to see if we need to update standard fees

		//if there aren't any Anesthesia codes with different prices than our Unit Cost, then leave
		if(IsRecordsetEmpty("SELECT ID FROM ServiceT WHERE Anesthesia = 1 AND Price <> Convert(money,'%s')",FormatCurrencyForSql(cyUnitCost))) {
			CDialog::OnOK();
			return;
		}

		CString strPrompt;
		strPrompt.Format("With the advanced Anesthesia Billing enabled, the standard fee for Anesthesia Service Codes is obsolete.\n\n"
			"For display purposes, and for default values in surgeries, would you like to update the current standard fee for each\n"
			"of these codes to use the anesthesia unit cost of %s, as is configured here in the Anesthesia Setup?",FormatCurrencyForInterface(cyUnitCost));

		if(IDNO == MessageBox(strPrompt,"Practice",MB_YESNO|MB_ICONQUESTION)) {
			CDialog::OnOK();
			return;
		}

		//if we get this far, they want to change the price, so do so
		ExecuteSql("UPDATE ServiceT SET Price = Convert(money,'%s') WHERE Anesthesia = 1",FormatCurrencyForSql(cyUnitCost));
			
		//now check and see if these codes exist in any surgeries with a different price than we had just updated
		
		//TODO - Consider only doing this based on the preference for auto-updating surgeries when they normally
		//change the standard fee. For now, always ask, as this is a different situation.
		if(!IsRecordsetEmpty("SELECT ID FROM SurgeryDetailsT WHERE ServiceID IN (SELECT ID FROM ServiceT WHERE Anesthesia = 1) AND Amount <> Convert(money,'%s')",FormatCurrencyForSql(cyUnitCost))) {
			//there are surgeries, to prompt a second time
			strPrompt.Format("There are surgeries currently using the Service Codes you have just updated.\n"
				"Would you like to update these surgeries to reflect this new price as well?");

			if(IDNO == MessageBox(strPrompt,"Practice",MB_YESNO|MB_ICONQUESTION)) {
				CDialog::OnOK();
				return;
			}

			//and if we're still here, then update those surgeries
			ExecuteSql("UPDATE SurgeryDetailsT SET Amount = Convert(money,'%s') WHERE ServiceID IN (SELECT ID FROM ServiceT WHERE Anesthesia = 1)",FormatCurrencyForSql(cyUnitCost));
		}
	
		CDialog::OnOK();

	}NxCatchAll("Error saving anesthesia settings.");
	*/
}

void CAnesthesiaSetupDlg::OnBtnEditInsuranceCoSetup() 
{
	CAnesthesiaInsCoSetupDlg dlg(this);
	dlg.DoModal();
}

void CAnesthesiaSetupDlg::OnSelChosenPosCombo(long nRow) 
{
	try {

		if(nRow == -1) {
			m_POSCombo->CurSel = 0;
		}

		if(IDYES == MessageBox("Any changes made to the previous Place Of Service will be saved.\n"
			"Do you still wish to switch Places Of Service?","Practice",MB_ICONQUESTION|MB_YESNO)) {

			if(!Save()) {
				//don't load and overwrite their changes yet
				m_POSCombo->SetSelByColumn(POS_COLUMN_ID,(long)m_nLocationID);
				return;
			}
		}
		else {
			//don't load and overwrite their changes yet
			m_POSCombo->SetSelByColumn(POS_COLUMN_ID,(long)m_nLocationID);
			return;
		}

		m_nLocationID = VarLong(m_POSCombo->GetValue(m_POSCombo->GetCurSel(), POS_COLUMN_ID));

		Load();

	}NxCatchAll("Error changing Place Of Service.");	
}

void CAnesthesiaSetupDlg::OnRadioFlatFee() 
{
	OnFeeRadioChanged();
}

void CAnesthesiaSetupDlg::OnRadioUseIncrementalFee() 
{
	OnFeeRadioChanged();	
}

void CAnesthesiaSetupDlg::OnRadioUseSpecificFeeSchedule() 
{
	OnFeeRadioChanged();	
}

void CAnesthesiaSetupDlg::OnRadioUseUnitBased() 
{
	OnFeeRadioChanged();	
}

void CAnesthesiaSetupDlg::OnFeeRadioChanged()
{
	GetDlgItem(IDC_EDIT_FLAT_ANESTH_FEE)->EnableWindow(m_radioFlatFee.GetCheck());
	GetDlgItem(IDC_EDIT_ANESTH_INCREMENTAL_BASE_FEE)->EnableWindow(m_radioIncrementalFee.GetCheck());
	m_IncrementBaseCombo->Enabled = m_radioIncrementalFee.GetCheck();
	GetDlgItem(IDC_EDIT_ANESTH_INCREMENTAL_ADDITIONAL_FEE)->EnableWindow(m_radioIncrementalFee.GetCheck());
	m_IncrementAdditionalCombo->Enabled = m_radioIncrementalFee.GetCheck();
	GetDlgItem(IDC_BTN_ADD_NEW_ANESTH_FEE_OPTION)->EnableWindow(m_radioSpecificFeeTable.GetCheck());
	GetDlgItem(IDC_BTN_DELETE_ANESTH_FEE_OPTION)->EnableWindow(m_radioSpecificFeeTable.GetCheck());
	m_AnesthFeeTable->Enabled = m_radioSpecificFeeTable.GetCheck();
	m_radioUseLesserTime.EnableWindow(m_radioSpecificFeeTable.GetCheck());
	m_radioUseGreaterTime.EnableWindow(m_radioSpecificFeeTable.GetCheck());
	GetDlgItem(IDC_EDIT_UNIT_COST)->EnableWindow(m_radioUnitBased.GetCheck());
	GetDlgItem(IDC_EDIT_TIME_UNITS)->EnableWindow(m_radioUnitBased.GetCheck());
	GetDlgItem(IDC_RADIO_BEGIN_END_TIMES)->EnableWindow(m_radioUnitBased.GetCheck());
	GetDlgItem(IDC_RADIO_MINUTES)->EnableWindow(m_radioUnitBased.GetCheck());
	GetDlgItem(IDC_BTN_EDIT_INSURANCE_CO_SETUP)->EnableWindow(m_radioUnitBased.GetCheck());
	m_radioUseBeginEndTimes.EnableWindow(!m_radioFlatFee.GetCheck());
	m_radioUseMinutes.EnableWindow(!m_radioFlatFee.GetCheck());
}

void CAnesthesiaSetupDlg::OnKillfocusEditFlatAnesthFee() 
{
	try {
		CString strFee;
		GetDlgItemText(IDC_EDIT_FLAT_ANESTH_FEE, strFee);

		if (strFee.GetLength() == 0) {
			MsgBox("Please fill in the 'Flat Fee' box.");
			SetDlgItemText(IDC_EDIT_FLAT_ANESTH_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		COleCurrency cy = ParseCurrencyFromInterface(strFee);
		if(cy.GetStatus() == COleCurrency::invalid) {
			MsgBox("Please enter a valid amount in the 'Flat Fee' box.");
			SetDlgItemText(IDC_EDIT_FLAT_ANESTH_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		if (cy < COleCurrency::COleCurrency(0,0))
		{
			MsgBox("Fees cannot be negative. Please enter a positive number.");
			SetDlgItemText(IDC_EDIT_FLAT_ANESTH_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		//see how much the regional settings allows to the right of the decimal
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
		int nDigits = atoi(strICurr);
		CString strDecimal;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
		if(cy.Format().Find(strDecimal) != -1 && cy.Format().Find(strDecimal) + (nDigits+1) < cy.Format().GetLength()) {
			MsgBox("Please fill only %li places to the right of the %s in the 'Flat Fee' box.",nDigits,strDecimal == "," ? "comma" : "decimal");
			SetDlgItemText(IDC_EDIT_FLAT_ANESTH_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		CString str = FormatCurrencyForInterface(cy, TRUE, TRUE);
		SetDlgItemText(IDC_EDIT_FLAT_ANESTH_FEE, str);

	}NxCatchAll("Error validating anesthesia fee.");
}

void CAnesthesiaSetupDlg::OnKillfocusEditIncrementalBaseFee() 
{
	try {
		CString strFee;
		GetDlgItemText(IDC_EDIT_ANESTH_INCREMENTAL_BASE_FEE, strFee);

		if (strFee.GetLength() == 0) {
			MsgBox("Please fill in the 'Incremental Base Fee' box.");
			SetDlgItemText(IDC_EDIT_ANESTH_INCREMENTAL_BASE_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		COleCurrency cy = ParseCurrencyFromInterface(strFee);
		if(cy.GetStatus() == COleCurrency::invalid) {
			MsgBox("Please enter a valid amount in the 'Incremental Base Fee' box.");
			SetDlgItemText(IDC_EDIT_ANESTH_INCREMENTAL_BASE_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		if (cy < COleCurrency::COleCurrency(0,0))
		{
			MsgBox("Fees cannot be negative. Please enter a positive number.");
			SetDlgItemText(IDC_EDIT_ANESTH_INCREMENTAL_BASE_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		//see how much the regional settings allows to the right of the decimal
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
		int nDigits = atoi(strICurr);
		CString strDecimal;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
		if(cy.Format().Find(strDecimal) != -1 && cy.Format().Find(strDecimal) + (nDigits+1) < cy.Format().GetLength()) {
			MsgBox("Please fill only %li places to the right of the %s in the 'Incremental Base Fee' box.",nDigits,strDecimal == "," ? "comma" : "decimal");
			SetDlgItemText(IDC_EDIT_ANESTH_INCREMENTAL_BASE_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		CString str = FormatCurrencyForInterface(cy, TRUE, TRUE);
		SetDlgItemText(IDC_EDIT_ANESTH_INCREMENTAL_BASE_FEE, str);

	}NxCatchAll("Error validating incremental base fee.");
}

void CAnesthesiaSetupDlg::OnKillfocusEditIncrementalAdditionalFee() 
{
	try {
		CString strFee;
		GetDlgItemText(IDC_EDIT_ANESTH_INCREMENTAL_ADDITIONAL_FEE, strFee);

		if (strFee.GetLength() == 0) {
			MsgBox("Please fill in the 'Incremental Additional Fee' box.");
			SetDlgItemText(IDC_EDIT_ANESTH_INCREMENTAL_ADDITIONAL_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		COleCurrency cy = ParseCurrencyFromInterface(strFee);
		if(cy.GetStatus() == COleCurrency::invalid) {
			MsgBox("Please enter a valid amount in the 'Incremental Additional Fee' box.");
			SetDlgItemText(IDC_EDIT_ANESTH_INCREMENTAL_ADDITIONAL_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		if (cy < COleCurrency::COleCurrency(0,0))
		{
			MsgBox("Fees cannot be negative. Please enter a positive number.");
			SetDlgItemText(IDC_EDIT_ANESTH_INCREMENTAL_ADDITIONAL_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		//see how much the regional settings allows to the right of the decimal
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
		int nDigits = atoi(strICurr);
		CString strDecimal;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
		if(cy.Format().Find(strDecimal) != -1 && cy.Format().Find(strDecimal) + (nDigits+1) < cy.Format().GetLength()) {
			MsgBox("Please fill only %li places to the right of the %s in the 'Incremental Additional Fee' box.",nDigits,strDecimal == "," ? "comma" : "decimal");
			SetDlgItemText(IDC_EDIT_ANESTH_INCREMENTAL_ADDITIONAL_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		CString str = FormatCurrencyForInterface(cy, TRUE, TRUE);
		SetDlgItemText(IDC_EDIT_ANESTH_INCREMENTAL_ADDITIONAL_FEE, str);

	}NxCatchAll("Error validating incremental additional fee.");
}

void CAnesthesiaSetupDlg::OnBtnAddNewFeeOption() 
{
	CFacilityFeeNewSchedDlg dlg(this);
	// (e.frazier 2016-05-09 11:11) - PLID-35037 - Make sure the window text specifies this will add a new anesthesia fee, not a facility fee
	dlg.m_bIsFacilityFee = FALSE;
	if(dlg.DoModal() == IDOK) {

		//ensure we're not duplicating an hour/minute combination
		for(int i=0;i<m_AnesthFeeTable->GetRowCount();i++) {
			BOOL bFound = FALSE;
			long nCompareHours = VarLong(m_AnesthFeeTable->GetValue(i,COLUMN_HOURS),0);
			long nCompareMinutes = VarLong(m_AnesthFeeTable->GetValue(i,COLUMN_MINUTES),0);

			if(nCompareHours == dlg.m_nHours && nCompareMinutes == dlg.m_nMinutes) {
				CString str;
				str.Format("A fee for %li hours and %li minutes already exists.", dlg.m_nHours, dlg.m_nMinutes);
				AfxMessageBox(str);
				return;
			}
		}

		IRowSettingsPtr pRow = m_AnesthFeeTable->GetRow(-1);
		pRow->PutValue(COLUMN_ID,(long)-1);
		pRow->PutValue(COLUMN_HOURS,(long)dlg.m_nHours);
		pRow->PutValue(COLUMN_MINUTES,(long)dlg.m_nMinutes);
		pRow->PutValue(COLUMN_FEE,_variant_t(dlg.m_cyFee));
		m_AnesthFeeTable->AddRow(pRow);
	}	
}

void CAnesthesiaSetupDlg::OnBtnDeleteFeeOption() 
{
	try {

		if(m_AnesthFeeTable->CurSel == -1) {
			AfxMessageBox("Please select a fee before deleting.");
			return;
		}

		m_AnesthFeeTable->RemoveRow(m_AnesthFeeTable->CurSel);		

	}NxCatchAll("Error removing fee.");	
}

void CAnesthesiaSetupDlg::Load()
{
	try {

		//a -1 service shouldn't be possible
		if(m_nServiceID == -1) {
			ASSERT(FALSE);
			return;
		}

		// (j.jones 2007-10-15 12:59) - PLID 27757 - converted to use the new structure
		_RecordsetPtr rs = CreateParamRecordset("SELECT AnesthesiaSetupT.ID, AnesthesiaFeeBillType, AnesthFlatFee, "
			"AnesthIncrementalBaseFee, AnesthBaseIncrement, AnesthIncrementalAddlFee, AnesthAddlIncrement, "
			"AnesthFeeTableSearchType, AnesthUnitBaseCost, AnesthMinutesPerUnit, "
			"AnesthTimePromptType, AnesthesiaPayTo, AnesthOutsideFee FROM AnesthesiaSetupT "
			"INNER JOIN LocationsT ON AnesthesiaSetupT.LocationID = LocationsT.ID "
			"WHERE AnesthesiaSetupT.ServiceID = {INT} AND AnesthesiaSetupT.LocationID = {INT}", m_nServiceID, m_nLocationID);

		if(rs->eof) {
			//if no records exist for this CPT/POS, create a new record, and pull the default values
			rs->Close();
			rs = CreateParamRecordset("SET NOCOUNT ON\r\n"
				"INSERT INTO AnesthesiaSetupT (ServiceID, LocationID) VALUES ({INT}, {INT})\r\n"
				"SET NOCOUNT OFF\r\n"
				"SELECT AnesthesiaSetupT.ID, AnesthesiaFeeBillType, AnesthFlatFee, "
				"AnesthIncrementalBaseFee, AnesthBaseIncrement, AnesthIncrementalAddlFee, AnesthAddlIncrement, "
				"AnesthFeeTableSearchType, AnesthUnitBaseCost, AnesthMinutesPerUnit, "
				"AnesthTimePromptType, AnesthesiaPayTo, AnesthOutsideFee FROM AnesthesiaSetupT "
				"INNER JOIN LocationsT ON AnesthesiaSetupT.LocationID = LocationsT.ID "
				"WHERE AnesthesiaSetupT.ID = Convert(int, SCOPE_IDENTITY())",m_nServiceID, m_nLocationID);
		}

		if(!rs->eof) {

			m_nAnesthesiaSetupID = AdoFldLong(rs, "ID");

			//load the outside fee setting
			m_checkQuoteAsOutsideFee.SetCheck(AdoFldBool(rs, "AnesthOutsideFee", FALSE));

			//load the fee usage setting
			long nAnesthFeeBillType = AdoFldLong(rs, "AnesthesiaFeeBillType",1);
			if(nAnesthFeeBillType == 1) {
				m_radioFlatFee.SetCheck(TRUE);
				m_radioIncrementalFee.SetCheck(FALSE);
				m_radioSpecificFeeTable.SetCheck(FALSE);
				m_radioUnitBased.SetCheck(FALSE);
			}
			else if(nAnesthFeeBillType == 2) {
				m_radioFlatFee.SetCheck(FALSE);
				m_radioIncrementalFee.SetCheck(TRUE);
				m_radioSpecificFeeTable.SetCheck(FALSE);
				m_radioUnitBased.SetCheck(FALSE);
			}
			else if(nAnesthFeeBillType == 3) {
				m_radioFlatFee.SetCheck(FALSE);
				m_radioIncrementalFee.SetCheck(FALSE);
				m_radioSpecificFeeTable.SetCheck(TRUE);
				m_radioUnitBased.SetCheck(FALSE);
			}
			else if(nAnesthFeeBillType == 4) {
				m_radioFlatFee.SetCheck(FALSE);
				m_radioIncrementalFee.SetCheck(FALSE);
				m_radioSpecificFeeTable.SetCheck(FALSE);
				m_radioUnitBased.SetCheck(TRUE);
			}

			OnFeeRadioChanged();

			//load the flat fee
			COleCurrency cyAnesthFlatFee = AdoFldCurrency(rs, "AnesthFlatFee",COleCurrency(0,0));
			SetDlgItemText(IDC_EDIT_FLAT_ANESTH_FEE, FormatCurrencyForInterface(cyAnesthFlatFee,TRUE,TRUE));

			//load the incremental base fee
			COleCurrency cyAnesthIncrementalBaseFee = AdoFldCurrency(rs, "AnesthIncrementalBaseFee",COleCurrency(0,0));
			SetDlgItemText(IDC_EDIT_ANESTH_INCREMENTAL_BASE_FEE, FormatCurrencyForInterface(cyAnesthIncrementalBaseFee,TRUE,TRUE));

			//load the incremental base increment
			long nAnesthBaseIncrement = AdoFldLong(rs, "AnesthBaseIncrement",1);
			m_IncrementBaseCombo->SetSelByColumn(0,nAnesthBaseIncrement);

			//load the incremental additional fee
			COleCurrency cyAnesthIncrementalAddlFee = AdoFldCurrency(rs, "AnesthIncrementalAddlFee",COleCurrency(0,0));
			SetDlgItemText(IDC_EDIT_ANESTH_INCREMENTAL_ADDITIONAL_FEE, FormatCurrencyForInterface(cyAnesthIncrementalAddlFee,TRUE,TRUE));

			//load the incremental additional increment
			long nAnesthAddlIncrement = AdoFldLong(rs, "AnesthAddlIncrement",1);
			m_IncrementAdditionalCombo->SetSelByColumn(0,nAnesthAddlIncrement);
			
			CString str;
			// (j.jones 2007-10-15 13:33) - PLID 27757 - converted to filter on the AnesthesiaSetupT.ID
			str.Format("AnesthesiaSetupID = %li", m_nAnesthesiaSetupID);
			m_AnesthFeeTable->PutWhereClause(_bstr_t(str));
			m_AnesthFeeTable->Requery();

			//load the lesser/greater setting
			long nAnesthFeeTableSearchType = AdoFldLong(rs, "AnesthFeeTableSearchType",1);
			if(nAnesthFeeTableSearchType == 1) {
				m_radioUseLesserTime.SetCheck(TRUE);
				m_radioUseGreaterTime.SetCheck(FALSE);
			}
			else if(nAnesthFeeTableSearchType == 2) {
				m_radioUseLesserTime.SetCheck(FALSE);
				m_radioUseGreaterTime.SetCheck(TRUE);
			}

			//load the base unit cost
			COleCurrency cyAnesthUnitBaseCost = AdoFldCurrency(rs, "AnesthUnitBaseCost",COleCurrency(0,0));
			SetDlgItemText(IDC_EDIT_UNIT_COST,FormatCurrencyForInterface(cyAnesthUnitBaseCost,TRUE,TRUE));		

			//load the minutes per unit
			long nAnesthMinutesPerUnit = AdoFldLong(rs, "AnesthMinutesPerUnit",0);
			SetDlgItemInt(IDC_EDIT_TIME_UNITS,nAnesthMinutesPerUnit);

			//load the start/end time or minutes option
			long nAnesthTimePromptType = AdoFldLong(rs, "AnesthTimePromptType",1);
			if(nAnesthTimePromptType == 1) {
				m_radioUseBeginEndTimes.SetCheck(TRUE);
				m_radioUseMinutes.SetCheck(FALSE);
			}
			else if(nAnesthTimePromptType == 2) {
				m_radioUseBeginEndTimes.SetCheck(FALSE);
				m_radioUseMinutes.SetCheck(TRUE);
			}

			//PLID 16591 Load the pay to field
			CString strPayTo = AdoFldString(rs, "AnesthesiaPayTo", "");
			SetDlgItemText(IDC_ANES_FEE_PAID_TO, strPayTo);
		}
		rs->Close();

	}NxCatchAll("Error loading Anesthesia Fee configuration.");
}

BOOL CAnesthesiaSetupDlg::Save()
{
	try {

		//save the outside fee setting
		long nOutsideFee = m_checkQuoteAsOutsideFee.GetCheck() ? 1 : 0;

		//save the fee usage setting
		long nAnesthFeeBillType = 1;
		if(m_radioFlatFee.GetCheck())
			nAnesthFeeBillType = 1;
		else if(m_radioIncrementalFee.GetCheck())
			nAnesthFeeBillType = 2;
		else if(m_radioSpecificFeeTable.GetCheck())
			nAnesthFeeBillType = 3;
		else if(m_radioUnitBased.GetCheck())
			nAnesthFeeBillType = 4;

		if(nAnesthFeeBillType == 3 && m_AnesthFeeTable->GetRowCount() == 0) {
			AfxMessageBox("You selected the 'Fee Table' option, but entered no fee amounts in the table.");
			return FALSE;
		}

		//save the flat fee
		COleCurrency cyAnesthFlatFee = COleCurrency(0,0);
		CString strFee;
		GetDlgItemText(IDC_EDIT_FLAT_ANESTH_FEE, strFee);
		cyAnesthFlatFee = ParseCurrencyFromInterface(strFee);

		//save the incremental base fee
		COleCurrency cyAnesthIncrementalBaseFee = COleCurrency(0,0);
		GetDlgItemText(IDC_EDIT_ANESTH_INCREMENTAL_BASE_FEE, strFee);
		cyAnesthIncrementalBaseFee = ParseCurrencyFromInterface(strFee);

		//save the incremental base increment
		long nAnesthBaseIncrement = 1;
		if(m_IncrementBaseCombo->CurSel != -1)
			nAnesthBaseIncrement = VarLong(m_IncrementBaseCombo->GetValue(m_IncrementBaseCombo->CurSel,0));

		//save the incremental additional fee
		COleCurrency cyAnesthIncrementalAddlFee = COleCurrency(0,0);
		GetDlgItemText(IDC_EDIT_ANESTH_INCREMENTAL_ADDITIONAL_FEE, strFee);
		cyAnesthIncrementalAddlFee = ParseCurrencyFromInterface(strFee);

		//save the incremental additional increment
		long nAnesthAddlIncrement = 1;
		if(m_IncrementAdditionalCombo->CurSel != -1)
			nAnesthAddlIncrement = VarLong(m_IncrementAdditionalCombo->GetValue(m_IncrementAdditionalCombo->CurSel,0));
		
		ExecuteSql("DELETE FROM LocationAnesthesiaFeesT WHERE AnesthesiaSetupID = %li", m_nAnesthesiaSetupID);
		for(int i=0;i<m_AnesthFeeTable->GetRowCount();i++) {
			long nHours = VarLong(m_AnesthFeeTable->GetValue(i,COLUMN_HOURS),0);
			long nMinutes = VarLong(m_AnesthFeeTable->GetValue(i,COLUMN_MINUTES),0);
			COleCurrency cyFee = VarCurrency(m_AnesthFeeTable->GetValue(i,COLUMN_FEE),COleCurrency(0,0));

			ExecuteSql("INSERT INTO LocationAnesthesiaFeesT (ID, AnesthesiaSetupID, Hours, Minutes, Fee) "
				"VALUES (%li, %li, %li, %li, Convert(money,'%s'))",NewNumber("LocationAnesthesiaFeesT","ID"),
				m_nAnesthesiaSetupID, nHours, nMinutes, FormatCurrencyForSql(cyFee));
		}

		//save the lesser/greater setting
		long nAnesthFeeTableSearchType = 2;
		if(m_radioUseLesserTime.GetCheck())
			nAnesthFeeTableSearchType = 1;
		else if(m_radioUseGreaterTime.GetCheck())
			nAnesthFeeTableSearchType = 2;

		//save the base unit cost
		COleCurrency cyAnesthUnitBaseCost = COleCurrency(0,0);
		GetDlgItemText(IDC_EDIT_UNIT_COST, strFee);
		cyAnesthUnitBaseCost = ParseCurrencyFromInterface(strFee);	

		//save the minutes per unit
		long nAnesthMinutesPerUnit = GetDlgItemInt(IDC_EDIT_TIME_UNITS);

		//save the start/end time or minutes option
		long nAnesthTimePromptType = 1;
		if(m_radioUseBeginEndTimes.GetCheck())
			nAnesthTimePromptType = 1;
		else if(m_radioUseMinutes.GetCheck())
			nAnesthTimePromptType = 2;

		//PLID 16591: Anesthesia pay to field
		CString strPayTo;
		GetDlgItemText(IDC_ANES_FEE_PAID_TO, strPayTo);

		// (j.jones 2007-10-15 13:47) - PLID 27757 - now everything but AnesthesiaPayTo is in its own table
		ExecuteSql("UPDATE LocationsT SET AnesthesiaPayTo = '%s' WHERE ID = %li", _Q(strPayTo), m_nLocationID);

		ExecuteSql("UPDATE AnesthesiaSetupT SET AnesthesiaFeeBillType = %li, AnesthFlatFee = Convert(money, '%s'), "
			"AnesthIncrementalBaseFee = Convert(money, '%s'), AnesthBaseIncrement = %li, "
			"AnesthIncrementalAddlFee = Convert(money, '%s'), AnesthAddlIncrement = %li, "
			"AnesthFeeTableSearchType = %li, "
			"AnesthUnitBaseCost = Convert(money, '%s'), AnesthMinutesPerUnit = %li, "
			"AnesthTimePromptType = %li, AnesthOutsideFee = %li "
			"WHERE ID = %li",
			nAnesthFeeBillType, FormatCurrencyForSql(cyAnesthFlatFee),
			FormatCurrencyForSql(cyAnesthIncrementalBaseFee), nAnesthBaseIncrement,
			FormatCurrencyForSql(cyAnesthIncrementalAddlFee), nAnesthAddlIncrement,
			nAnesthFeeTableSearchType, 
			FormatCurrencyForSql(cyAnesthUnitBaseCost), nAnesthMinutesPerUnit,
			nAnesthTimePromptType, nOutsideFee, 
			m_nAnesthesiaSetupID);
				
		return TRUE;

	}NxCatchAll("Error saving anesthesia fee setup.");

	return FALSE;
}

void CAnesthesiaSetupDlg::OnEditingFinishingAnesthFeeTable(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	switch(nCol) {
		case COLUMN_HOURS: {

			long nHours = VarLong(pvarNewValue);
			long nMinutes = VarLong(m_AnesthFeeTable->GetValue(nRow,COLUMN_MINUTES),0);

			if(nHours < 0) {
				*pbCommit = FALSE;
				AfxMessageBox("You cannot enter a negative number.");
				return;
			}

			if(nHours > 24) {
				*pbCommit = FALSE;
				AfxMessageBox("You cannot enter more than 24 hours.");
				return;
			}
			else if(nHours == 24) {				
				if(nMinutes > 0) {
					*pbCommit = FALSE;
					AfxMessageBox("You cannot enter more than 24 hours and 0 minutes.");
					return;
				}
			}

			//now ensure we're not duplicating an hour/minute combination
			for(int i=0;i<m_AnesthFeeTable->GetRowCount();i++) {
				BOOL bFound = FALSE;
				if(nRow != i) {
					long nCompareHours = VarLong(m_AnesthFeeTable->GetValue(i,COLUMN_HOURS),0);
					long nCompareMinutes = VarLong(m_AnesthFeeTable->GetValue(i,COLUMN_MINUTES),0);

					if(nCompareHours == nHours && nCompareMinutes == nMinutes) {
						*pbCommit = FALSE;
						CString str;
						str.Format("A fee for %li hours and %li minutes already exists.", nHours, nMinutes);
						AfxMessageBox(str);
						return;
					}
				}
			}

			break;
		}

		case COLUMN_MINUTES: {

			long nMinutes = VarLong(pvarNewValue);
			long nHours = VarLong(m_AnesthFeeTable->GetValue(nRow,COLUMN_HOURS),0);

			if(nMinutes < 0) {
				*pbCommit = FALSE;
				AfxMessageBox("You cannot enter a negative number.");
				return;
			}

			if(nHours == 24) {
				if(nMinutes > 0) {
					*pbCommit = FALSE;
					AfxMessageBox("You cannot enter more than 24 hours and 0 minutes.");
					return;
				}
			}

			if(nMinutes > 59) {
				*pbCommit = FALSE;
				AfxMessageBox("You cannot enter more than 59 minutes.");
				return;
			}

			//now ensure we're not duplicating an hour/minute combination
			for(int i=0;i<m_AnesthFeeTable->GetRowCount();i++) {
				BOOL bFound = FALSE;
				if(nRow != i) {
					long nCompareHours = VarLong(m_AnesthFeeTable->GetValue(i,COLUMN_HOURS),0);
					long nCompareMinutes = VarLong(m_AnesthFeeTable->GetValue(i,COLUMN_MINUTES),0);

					if(nCompareHours == nHours && nCompareMinutes == nMinutes) {
						*pbCommit = FALSE;
						CString str;
						str.Format("A fee for %li hours and %li minutes already exists.", nHours, nMinutes);
						AfxMessageBox(str);
						return;
					}
				}
			}

			break;
		}

		case COLUMN_FEE:
			if(pvarNewValue->vt == VT_CY && COleCurrency(pvarNewValue->cyVal) < COleCurrency(0,0)) {
				*pbCommit = FALSE;
				AfxMessageBox("You cannot enter a negative fee.");
				return;
			}
		break;
	}
}

void CAnesthesiaSetupDlg::OnEditingFinishedAnesthFeeTable(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	//we actually don't need to do anything here since we don't save until OnOK()	
	m_AnesthFeeTable->Sort();
}

void CAnesthesiaSetupDlg::OnKillfocusEditUnitCost() 
{
	try {
		CString strFee;
		GetDlgItemText(IDC_EDIT_UNIT_COST, strFee);

		if (strFee.GetLength() == 0) {
			MsgBox("Please fill in the 'Unit Cost' box.");
			SetDlgItemText(IDC_EDIT_UNIT_COST, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		COleCurrency cy = ParseCurrencyFromInterface(strFee);
		if(cy.GetStatus() == COleCurrency::invalid) {
			MsgBox("Please enter a valid amount in the 'Unit Cost' box.");
			SetDlgItemText(IDC_EDIT_UNIT_COST, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		if (cy < COleCurrency::COleCurrency(0,0))
		{
			MsgBox("Fees cannot be negative. Please enter a positive number.");
			SetDlgItemText(IDC_EDIT_UNIT_COST, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		//see how much the regional settings allows to the right of the decimal
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
		int nDigits = atoi(strICurr);
		CString strDecimal;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
		if(cy.Format().Find(strDecimal) != -1 && cy.Format().Find(strDecimal) + (nDigits+1) < cy.Format().GetLength()) {
			MsgBox("Please fill only %li places to the right of the %s in the 'Unit Cost' box.",nDigits,strDecimal == "," ? "comma" : "decimal");
			SetDlgItemText(IDC_EDIT_UNIT_COST, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		CString str = FormatCurrencyForInterface(cy, TRUE, TRUE);
		SetDlgItemText(IDC_EDIT_UNIT_COST, str);

	}NxCatchAll("Error validating anesthesia unit cost.");
}

void CAnesthesiaSetupDlg::OnKillfocusEditTimeUnits() 
{
	long nAnesthMinutesPerUnit = GetDlgItemInt(IDC_EDIT_TIME_UNITS);
	if(nAnesthMinutesPerUnit == 0) {
		AfxMessageBox("The 'Minutes Per Unit' must be greater than zero.");
		return;
	}
}

// (j.jones 2007-10-16 08:48) - PLID 27760 - added ability to copy the setup between service codes
void CAnesthesiaSetupDlg::OnBtnCopyAnesthToCode() 
{
	try {

		//a -1 service shouldn't be possible
		if(m_nServiceID == -1) {
			ASSERT(FALSE);
			return;
		}

		if(IDYES == MessageBox("Any changes made to the current Place Of Service will be saved.\n"
			"Do you still wish to copy this service code's setup to another code?","Practice",MB_ICONQUESTION|MB_YESNO)) {

			if(!Save()) {
				return;
			}
		}
		else {
			return;
		}

		//prompt for a destination service code

		long nNewServiceID = -1;
		BOOL bContinue = TRUE;

		//filter only on active codes, and remove the current code from being an option
		CString str;
		str.Format("(SELECT ServiceT.ID, CPTCodeT.Code + ' ' + CPTCodeT.SubCode + ' - ' + ServiceT.Name AS Name "
				"FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE Active = 1 AND ServiceT.ID <> %li) AS ServiceT", m_nServiceID);

		while(bContinue) {
			CSingleSelectDlg dlg(this);
			if(IDOK == dlg.Open(str, "", "ID", "Name", "Select a Service Code to copy this Anesthesia Setup to:")) {

				nNewServiceID = dlg.GetSelectedID();				
			}
			else {
				bContinue = FALSE;
				continue;
			}

			if(nNewServiceID == -1) {
				AfxMessageBox("Please select a Service Code from the list.");
			}
			else {
				//we have a code, so break out of this loop
				bContinue = FALSE;
			}
		}

		if(nNewServiceID == -1) {
			return;
		}

		//ok, if we get here, we have a valid nNewServiceID to copy to

		//check to see if there is an existing anesthesia setup for this code,
		//also grab its anesthesia statii

		CString strNewCode = "";
		BOOL bUseAnesthesiaBilling = FALSE;
		BOOL bHasAnesthesiaSettings = FALSE;		
		
		_RecordsetPtr rs = CreateParamRecordset("SELECT Code, UseAnesthesiaBilling, "
			"Convert(bit, CASE WHEN EXISTS (SELECT ID FROM AnesthesiaSetupT WHERE ServiceID = ServiceT.ID) THEN 1 ELSE 0 END) AS HasAnesthesiaSettings "
			"FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE ServiceT.ID = {INT}", nNewServiceID);

		if(rs->eof) {
			//shouldn't be possible
			ASSERT(FALSE);
			return;
		}
		else {
			strNewCode = AdoFldString(rs, "Code","");
			bUseAnesthesiaBilling = AdoFldBool(rs, "UseAnesthesiaBilling", FALSE);
			bHasAnesthesiaSettings = AdoFldBool(rs, "HasAnesthesiaSettings", FALSE);			
		}
		rs->Close();

		//now warn accordingly
		if(bHasAnesthesiaSettings) {
			str.Format("This action will clear out all existing anesthesia settings for the selected service code '%s',\n"
				"and copy the anesthesia settings for all places of service from code '%s' to code '%s'.\n\n"
				"Are you sure you wish to copy these settings?", strNewCode, m_strServiceCode, strNewCode);
		}
		else {
			str.Format("This action copy the anesthesia settings for all places of service from code '%s' to code '%s'.\n\n"
				"Are you sure you wish to copy these settings?", m_strServiceCode, strNewCode);
		}

		if(IDNO == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		//one more warning - if UseAnesthesiaBilling is not checked, prompt to check it (even if our source code doesn't have it checked)
		BOOL bNeedsAnesthesiaBillingEnabled = FALSE;
		if(!bUseAnesthesiaBilling) {
			str.Format("The selected service code '%s' does not have the 'Use Anesthesia Billing' option checked.\n"
				"The copied anesthesia setup will not be used until this option is checked.\n\n"
				"Do you wish to enable anesthesia billing for this code now?", strNewCode);
			if(IDYES == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
				//if yes, set our boolean, we'll update in data shortly
				bNeedsAnesthesiaBillingEnabled = TRUE;
			}
		}

		//now we are ready to update in data

		CString strSqlBatch = BeginSqlBatch();

		//first clear out all existing data for this service
		//(note: bHasAnesthesiaSettings would reflect whether any data exists to delete, but in
		//the slim chance any was created while the messageboxes were up, we need to make sure
		//everything is deleted, so always delete even if bHasAnesthesiaSettings = FALSE)
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM LocationAnesthesiaFeesT WHERE AnesthesiaSetupID IN (SELECT ID FROM AnesthesiaSetupT WHERE ServiceID = %li)", nNewServiceID);
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM AnesthesiaSetupT WHERE ServiceID = %li", nNewServiceID);

		//now copy all the data from this code to the new code
		
		//bulk copy the AnesthesiaSetupT records
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO AnesthesiaSetupT (ServiceID, LocationID, "
			"AnesthesiaFeeBillType, AnesthFlatFee, AnesthIncrementalBaseFee, AnesthBaseIncrement, AnesthIncrementalAddlFee, "
			"AnesthAddlIncrement, AnesthUnitBaseCost, AnesthMinutesPerUnit, AnesthFeeTableSearchType, AnesthTimePromptType, "
			"AnesthOutsideFee) "
			"SELECT %li, LocationID, "
			"AnesthesiaFeeBillType, AnesthFlatFee, AnesthIncrementalBaseFee, AnesthBaseIncrement, AnesthIncrementalAddlFee, "
			"AnesthAddlIncrement, AnesthUnitBaseCost, AnesthMinutesPerUnit, AnesthFeeTableSearchType, AnesthTimePromptType, "
			"AnesthOutsideFee FROM AnesthesiaSetupT WHERE ServiceID = %li", nNewServiceID, m_nServiceID);

		//updating LocationAnesthesiaFeesT is a little trickier because we don't have an Identity,
		//therefore we can't bulk copy, so query data and pass in IDs to copy from
		_RecordsetPtr rsFees = CreateParamRecordset("SELECT LocationAnesthesiaFeesT.ID, LocationID FROM LocationAnesthesiaFeesT "
			"INNER JOIN AnesthesiaSetupT ON LocationAnesthesiaFeesT.AnesthesiaSetupID = AnesthesiaSetupT.ID "
			"WHERE AnesthesiaSetupID IN (SELECT ID FROM AnesthesiaSetupT WHERE ServiceID = {INT})", m_nServiceID); 

		if(!rsFees->eof) {
			//if we have any records, declare and initialize the new ID variable as the maximum ID in the table,
			//don't increment it yet
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewLocationAnesthesiaFeeID INT");
			AddStatementToSqlBatch(strSqlBatch, "SET @nNewLocationAnesthesiaFeeID = (SELECT Coalesce(Max(ID),0) AS NewID FROM LocationAnesthesiaFeesT)");

			//also declare our destination AnesthesiaSetupT.ID variable
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewAnesthesiaSetupID INT");
		}

		while(!rsFees->eof) {
			long nLocationAnesthesiaFeeID = AdoFldLong(rsFees, "ID");
			long nCurLocationID = AdoFldLong(rsFees, "LocationID");

			//increment the ID
			AddStatementToSqlBatch(strSqlBatch, "SET @nNewLocationAnesthesiaFeeID = @nNewLocationAnesthesiaFeeID + 1");

			//update our @nNewAnesthesiaSetupID variable
			AddStatementToSqlBatch(strSqlBatch, "SET @nNewAnesthesiaSetupID = (SELECT ID FROM AnesthesiaSetupT WHERE ServiceID = %li AND LocationID = %li)", nNewServiceID, nCurLocationID);

			//and now copy the record
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO LocationAnesthesiaFeesT (ID, AnesthesiaSetupID, Hours, Minutes, Fee) "
				"SELECT @nNewLocationAnesthesiaFeeID, @nNewAnesthesiaSetupID, Hours, Minutes, Fee FROM LocationAnesthesiaFeesT WHERE ID = %li", nLocationAnesthesiaFeeID);

			rsFees->MoveNext();
		}
		rsFees->Close();

		//we don't need to copy AnesthesiaPayTo because it's already set in LocationsT

		//and now update the UseAnesthesiaBilling status, if needed
		if(bNeedsAnesthesiaBillingEnabled) {
			//update the Anesthesia status as well as UseAnesthesiaBilling
			//and ensure it is NOT marked as a facility fee
			AddStatementToSqlBatch(strSqlBatch, "UPDATE ServiceT SET Anesthesia = 1, UseAnesthesiaBilling = 1, FacilityFee = 0, UseFacilityBilling = 0 WHERE ID = %li", nNewServiceID);
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

	}NxCatchAll("Error in CAnesthesiaSetupDlg::OnBtnCopyAnesthToCode");
}

// (j.jones 2007-10-16 10:48) - PLID 27761 - added ability to copy the setup between places of service
void CAnesthesiaSetupDlg::OnBtnCopyAnesthToPos() 
{
	try {

		//a -1 service shouldn't be possible
		if(m_nServiceID == -1) {
			ASSERT(FALSE);
			return;
		}

		//we should have a place of service selected
		if(m_nLocationID == -1 || m_POSCombo->CurSel == -1) {
			AfxMessageBox("Please select a Place Of Service before trying to copy the Anesthesia Setup.");
			return;
		}

		//filter only on active locations, and remove the current location from being an option
		CString strWhere;
		strWhere.Format("Active = 1 AND TypeID = 1 AND ID <> %li", m_nLocationID);

		//ensure the are actually available records in the dropdown list before we continue
		if(IsRecordsetEmpty("SELECT ID FROM LocationsT WHERE %s", strWhere)) {
			AfxMessageBox("You only have one active Place Of Service. There are no other valid Places of Service to copy to.");
			return;
		}

		if(IDYES == MessageBox("Any changes made to the current Place Of Service will be saved.\n"
			"Do you still wish to copy this Place Of Service's setup to another Place Of Service?","Practice",MB_ICONQUESTION|MB_YESNO)) {

			if(!Save()) {
				return;
			}
		}
		else {
			return;
		}

		CString strCurPOS = VarString(m_POSCombo->GetValue(m_POSCombo->GetCurSel(), POS_COLUMN_NAME));

		//prompt for a destination place of service

		long nNewPOSID = -1;
		BOOL bContinue = TRUE;

		while(bContinue) {
			CSingleSelectDlg dlg(this);
			//using the same location filter from earlier
			if(IDOK == dlg.Open("LocationsT", strWhere, "ID", "Name", "Select a Place Of Service to copy this Anesthesia Setup to:")) {

				nNewPOSID = dlg.GetSelectedID();				
			}
			else {
				bContinue = FALSE;
				continue;
			}

			if(nNewPOSID == -1) {
				AfxMessageBox("Please select a Place Of Service from the list.");
			}
			else {
				//we have a POS, so break out of this loop
				bContinue = FALSE;
			}
		}

		if(nNewPOSID == -1) {
			return;
		}

		//ok, if we get here, we have a valid nNewPOSID to copy to

		//check to see if there is an existing anesthesia setup for this POS

		CString strNewPOS = "";
		BOOL bHasAnesthesiaSettings = FALSE;		
		
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name, "
			"Convert(bit, CASE WHEN EXISTS (SELECT ID FROM AnesthesiaSetupT WHERE LocationID = LocationsT.ID) THEN 1 ELSE 0 END) AS HasAnesthesiaSettings "
			"FROM LocationsT WHERE ID = {INT}", nNewPOSID);

		if(rs->eof) {
			//shouldn't be possible
			ASSERT(FALSE);
			return;
		}
		else {
			strNewPOS = AdoFldString(rs, "Name","");
			bHasAnesthesiaSettings = AdoFldBool(rs, "HasAnesthesiaSettings", FALSE);			
		}
		rs->Close();

		//now warn accordingly
		CString str;
		if(bHasAnesthesiaSettings) {
			str.Format("This action will clear out the existing anesthesia settings for the selected Place Of Service '%s' and code '%s',\n"
				"and copy the anesthesia settings from the Place Of Service '%s' to '%s' for the code '%s'.\n\n"
				"Are you sure you wish to copy these settings?", strNewPOS, m_strServiceCode, strCurPOS, strNewPOS, m_strServiceCode);
		}
		else {
			str.Format("This action will copy the anesthesia settings from the Place Of Service '%s' to '%s' for the code '%s'.\n\n"
				"Are you sure you wish to copy these settings?", strCurPOS, strNewPOS, m_strServiceCode);
		}

		if(IDNO == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		//if the current location has anything in the AnesthesiaPayTo field,
		//prompt to copy it to the new Place Of Service
		BOOL bNeedsPayToTextCopied = FALSE;
		if(ReturnsRecords("SELECT AnesthesiaPayTo FROM LocationsT WHERE ID = %li AND AnesthesiaPayTo Is Not Null AND AnesthesiaPayTo <> ''", m_nLocationID)) {
			str.Format("Would you like to also copy the 'Check Payable To...' text from '%s' to '%s'?", strCurPOS, strNewPOS);
			if(IDYES == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
				//if yes, set our boolean, we'll update in data shortly
				bNeedsPayToTextCopied = TRUE;
			}
		}

		//now we are ready to update in data

		CString strSqlBatch = BeginSqlBatch();

		//first clear out all existing data for this service/location combination
		//(note: bHasAnesthesiaSettings would reflect whether any data exists to delete, but in
		//the slim chance any was created while the messageboxes were up, we need to make sure
		//everything is deleted, so always delete even if bHasAnesthesiaSettings = FALSE)
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM LocationAnesthesiaFeesT WHERE AnesthesiaSetupID IN (SELECT ID FROM AnesthesiaSetupT WHERE ServiceID = %li AND LocationID = %li)", m_nServiceID, nNewPOSID);
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM AnesthesiaSetupT WHERE ServiceID = %li AND LocationID = %li", m_nServiceID, nNewPOSID);

		//now copy all the data from this POS to the new POS
		
		//copy the AnesthesiaSetupT record
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO AnesthesiaSetupT (ServiceID, LocationID, "
			"AnesthesiaFeeBillType, AnesthFlatFee, AnesthIncrementalBaseFee, AnesthBaseIncrement, AnesthIncrementalAddlFee, "
			"AnesthAddlIncrement, AnesthUnitBaseCost, AnesthMinutesPerUnit, AnesthFeeTableSearchType, AnesthTimePromptType, "
			"AnesthOutsideFee) "
			"SELECT ServiceID, %li, "
			"AnesthesiaFeeBillType, AnesthFlatFee, AnesthIncrementalBaseFee, AnesthBaseIncrement, AnesthIncrementalAddlFee, "
			"AnesthAddlIncrement, AnesthUnitBaseCost, AnesthMinutesPerUnit, AnesthFeeTableSearchType, AnesthTimePromptType, "
			"AnesthOutsideFee FROM AnesthesiaSetupT WHERE ServiceID = %li AND LocationID = %li", nNewPOSID, m_nServiceID, m_nLocationID);

		//updating LocationAnesthesiaFeesT is a little trickier because we don't have an Identity,
		//therefore we can't bulk copy, so query data and pass in IDs to copy from
		_RecordsetPtr rsFees = CreateParamRecordset("SELECT LocationAnesthesiaFeesT.ID, LocationID FROM LocationAnesthesiaFeesT "
			"INNER JOIN AnesthesiaSetupT ON LocationAnesthesiaFeesT.AnesthesiaSetupID = AnesthesiaSetupT.ID "
			"WHERE AnesthesiaSetupID IN (SELECT ID FROM AnesthesiaSetupT WHERE ServiceID = {INT} AND LocationID = {INT})", m_nServiceID, m_nLocationID); 

		if(!rsFees->eof) {
			//if we have any records, declare and initialize the new ID variable as the maximum ID in the table,
			//don't increment it yet
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewLocationAnesthesiaFeeID INT");
			AddStatementToSqlBatch(strSqlBatch, "SET @nNewLocationAnesthesiaFeeID = (SELECT Coalesce(Max(ID),0) AS NewID FROM LocationAnesthesiaFeesT)");

			//also declare our destination AnesthesiaSetupT.ID variable
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewAnesthesiaSetupID INT");
		}

		while(!rsFees->eof) {
			long nLocationAnesthesiaFeeID = AdoFldLong(rsFees, "ID");

			//increment the ID
			AddStatementToSqlBatch(strSqlBatch, "SET @nNewLocationAnesthesiaFeeID = @nNewLocationAnesthesiaFeeID + 1");

			//update our @nNewAnesthesiaSetupID variable
			AddStatementToSqlBatch(strSqlBatch, "SET @nNewAnesthesiaSetupID = (SELECT ID FROM AnesthesiaSetupT WHERE ServiceID = %li AND LocationID = %li)", m_nServiceID, nNewPOSID);

			//and now copy the record
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO LocationAnesthesiaFeesT (ID, AnesthesiaSetupID, Hours, Minutes, Fee) "
				"SELECT @nNewLocationAnesthesiaFeeID, @nNewAnesthesiaSetupID, Hours, Minutes, Fee FROM LocationAnesthesiaFeesT WHERE ID = %li", nLocationAnesthesiaFeeID);

			rsFees->MoveNext();
		}
		rsFees->Close();

		//and now update the AnesthesiaPayTo text, if needed
		if(bNeedsPayToTextCopied) {
			AddStatementToSqlBatch(strSqlBatch, "UPDATE LocationsT SET AnesthesiaPayTo = (SELECT AnesthesiaPayTo FROM LocationsT WHERE ID = %li) WHERE ID = %li", m_nLocationID, nNewPOSID);
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

	}NxCatchAll("Error in CAnesthesiaSetupDlg::OnBtnCopyAnesthToPos");
}
