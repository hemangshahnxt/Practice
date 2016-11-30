// FacilityFeeConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FacilityFeeConfigDlg.h"
#include "InternationalUtils.h"
#include "GlobalDrawingUtils.h"
#include "FacilityFeeNewSchedDlg.h"
#include "SingleSelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

#define COLUMN_ID		0
#define	COLUMN_HOURS	1
#define	COLUMN_MINUTES	2
#define	COLUMN_FEE		3

#define POS_COLUMN_ID	0
#define POS_COLUMN_NAME	1

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CFacilityFeeConfigDlg dialog


CFacilityFeeConfigDlg::CFacilityFeeConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFacilityFeeConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFacilityFeeConfigDlg)		
		m_nServiceID = -1; // (j.jones 2007-10-15 10:36) - PLID 27757 - added ServiceID & ServiceCode
		m_strServiceCode = "";
		m_nLocationID = -1;
		m_nFacilityFeeSetupID = -1;
	//}}AFX_DATA_INIT
}


void CFacilityFeeConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFacilityFeeConfigDlg)
	DDX_Control(pDX, IDC_BTN_COPY_FACFEE_TO_POS, m_btnCopyToPOS);
	DDX_Control(pDX, IDC_BTN_COPY_FACFEE_TO_CODE, m_btnCopyToCode);
	DDX_Control(pDX, IDC_BTN_DELETE_FEE_OPTION, m_btnDeleteFeeOption);
	DDX_Control(pDX, IDC_BTN_ADD_NEW_FEE_OPTION, m_btnAddFeeOption);
	DDX_Control(pDX, IDC_CHECK_QUOTE_AS_OUTSIDE_FEE, m_checkQuoteAsOutsideFee);
	DDX_Control(pDX, IDC_RADIO_MINUTES, m_radioUseMinutes);
	DDX_Control(pDX, IDC_RADIO_BEGIN_END_TIMES, m_radioUseBeginEndTimes);
	DDX_Control(pDX, IDC_RADIO_USE_LESSER_TIME, m_radioUseLesserTime);
	DDX_Control(pDX, IDC_RADIO_USE_GREATER_TIME, m_radioUseGreaterTime);
	DDX_Control(pDX, IDC_RADIO_FLAT_FEE, m_radioFlatFee);
	DDX_Control(pDX, IDC_RADIO_USE_INCREMENTAL_FEE, m_radioIncrementalFee);
	DDX_Control(pDX, IDC_RADIO_USE_SPECIFIC_FEE_SCHEDULE, m_radioSpecificFeeTable);
	DDX_Control(pDX, IDC_EDIT_FLAT_FACILITY_FEE, m_nxeditEditFlatFacilityFee);
	DDX_Control(pDX, IDC_EDIT_INCREMENTAL_BASE_FEE, m_nxeditEditIncrementalBaseFee);
	DDX_Control(pDX, IDC_EDIT_INCREMENTAL_ADDITIONAL_FEE, m_nxeditEditIncrementalAdditionalFee);
	DDX_Control(pDX, IDC_FACILITY_FEE_PAID_TO, m_nxeditFacilityFeePaidTo);
	DDX_Control(pDX, IDC_FACILITY_SERVICE_LABEL, m_nxstaticFacilityServiceLabel);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFacilityFeeConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CFacilityFeeConfigDlg)
	ON_BN_CLICKED(IDC_RADIO_FLAT_FEE, OnRadioFlatFee)
	ON_BN_CLICKED(IDC_RADIO_USE_INCREMENTAL_FEE, OnRadioUseIncrementalFee)
	ON_BN_CLICKED(IDC_RADIO_USE_SPECIFIC_FEE_SCHEDULE, OnRadioUseSpecificFeeSchedule)
	ON_EN_KILLFOCUS(IDC_EDIT_FLAT_FACILITY_FEE, OnKillfocusEditFlatFacilityFee)
	ON_EN_KILLFOCUS(IDC_EDIT_INCREMENTAL_BASE_FEE, OnKillfocusEditIncrementalBaseFee)
	ON_EN_KILLFOCUS(IDC_EDIT_INCREMENTAL_ADDITIONAL_FEE, OnKillfocusEditIncrementalAdditionalFee)
	ON_BN_CLICKED(IDC_BTN_ADD_NEW_FEE_OPTION, OnBtnAddNewFeeOption)
	ON_BN_CLICKED(IDC_BTN_DELETE_FEE_OPTION, OnBtnDeleteFeeOption)
	ON_BN_CLICKED(IDC_BTN_COPY_FACFEE_TO_CODE, OnBtnCopyFacfeeToCode)
	ON_BN_CLICKED(IDC_BTN_COPY_FACFEE_TO_POS, OnBtnCopyFacfeeToPos)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFacilityFeeConfigDlg message handlers

BOOL CFacilityFeeConfigDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		// (j.jones 2007-10-15 11:41) - PLID 27757 - display the service code in the label
		CString str;
		str.Format("Facility Fee Configuration for Service Code '%s'", m_strServiceCode);
		SetDlgItemText(IDC_FACILITY_SERVICE_LABEL, str);

		// (z.manning, 04/30/2008) - PLID 29860 - Set button styles
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAddFeeOption.AutoSet(NXB_NEW);
		m_btnDeleteFeeOption.AutoSet(NXB_DELETE);
		
		m_POSCombo = BindNxDataListCtrl(this,IDC_POS_COMBO,GetRemoteData(),true);
		m_IncrementBaseCombo = BindNxDataListCtrl(this,IDC_INCREMENTAL_BASE_TIME_COMBO,GetRemoteData(),false);
		m_IncrementAdditionalCombo = BindNxDataListCtrl(this,IDC_INCREMENTAL_ADDITIONAL_TIME_COMBO,GetRemoteData(),false);
		m_FacilityFeeTable = BindNxDataListCtrl(this,IDC_FACILITY_FEE_TABLE,GetRemoteData(),false);

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

		((CNxEdit*)GetDlgItem(IDC_FACILITY_FEE_PAID_TO))->LimitText(200);

		m_POSCombo->CurSel = 0;
		m_nLocationID = VarLong(m_POSCombo->GetValue(m_POSCombo->GetCurSel(), POS_COLUMN_ID));
		Load();

	}NxCatchAll("Error loading facility fee setup.");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFacilityFeeConfigDlg::OnOK() 
{
	if(!Save())
		return;

	CDialog::OnOK();
}

BEGIN_EVENTSINK_MAP(CFacilityFeeConfigDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CFacilityFeeConfigDlg)
	ON_EVENT(CFacilityFeeConfigDlg, IDC_POS_COMBO, 16 /* SelChosen */, OnSelChosenPosCombo, VTS_I4)
	ON_EVENT(CFacilityFeeConfigDlg, IDC_FACILITY_FEE_TABLE, 9 /* EditingFinishing */, OnEditingFinishingFacilityFeeTable, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CFacilityFeeConfigDlg, IDC_FACILITY_FEE_TABLE, 10 /* EditingFinished */, OnEditingFinishedFacilityFeeTable, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CFacilityFeeConfigDlg::OnSelChosenPosCombo(long nRow) 
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

void CFacilityFeeConfigDlg::OnRadioFlatFee() 
{
	OnFeeRadioChanged();
}

void CFacilityFeeConfigDlg::OnRadioUseIncrementalFee() 
{
	OnFeeRadioChanged();	
}

void CFacilityFeeConfigDlg::OnRadioUseSpecificFeeSchedule() 
{
	OnFeeRadioChanged();	
}

void CFacilityFeeConfigDlg::OnFeeRadioChanged()
{
	GetDlgItem(IDC_EDIT_FLAT_FACILITY_FEE)->EnableWindow(m_radioFlatFee.GetCheck());
	GetDlgItem(IDC_EDIT_INCREMENTAL_BASE_FEE)->EnableWindow(m_radioIncrementalFee.GetCheck());
	m_IncrementBaseCombo->Enabled = m_radioIncrementalFee.GetCheck();
	GetDlgItem(IDC_EDIT_INCREMENTAL_ADDITIONAL_FEE)->EnableWindow(m_radioIncrementalFee.GetCheck());
	m_IncrementAdditionalCombo->Enabled = m_radioIncrementalFee.GetCheck();
	GetDlgItem(IDC_BTN_ADD_NEW_FEE_OPTION)->EnableWindow(m_radioSpecificFeeTable.GetCheck());
	GetDlgItem(IDC_BTN_DELETE_FEE_OPTION)->EnableWindow(m_radioSpecificFeeTable.GetCheck());
	m_FacilityFeeTable->Enabled = m_radioSpecificFeeTable.GetCheck();
	m_radioUseLesserTime.EnableWindow(m_radioSpecificFeeTable.GetCheck());
	m_radioUseGreaterTime.EnableWindow(m_radioSpecificFeeTable.GetCheck());
	m_radioUseBeginEndTimes.EnableWindow(!m_radioFlatFee.GetCheck());
	m_radioUseMinutes.EnableWindow(!m_radioFlatFee.GetCheck());
}

void CFacilityFeeConfigDlg::OnKillfocusEditFlatFacilityFee() 
{
	try {
		CString strFee;
		GetDlgItemText(IDC_EDIT_FLAT_FACILITY_FEE, strFee);

		if (strFee.GetLength() == 0) {
			MsgBox("Please fill in the 'Flat Fee' box.");
			SetDlgItemText(IDC_EDIT_FLAT_FACILITY_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		COleCurrency cy = ParseCurrencyFromInterface(strFee);
		if(cy.GetStatus() == COleCurrency::invalid) {
			MsgBox("Please enter a valid amount in the 'Flat Fee' box.");
			SetDlgItemText(IDC_EDIT_FLAT_FACILITY_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		if (cy < COleCurrency::COleCurrency(0,0))
		{
			MsgBox("Fees cannot be negative. Please enter a positive number.");
			SetDlgItemText(IDC_EDIT_FLAT_FACILITY_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
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
			SetDlgItemText(IDC_EDIT_FLAT_FACILITY_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		CString str = FormatCurrencyForInterface(cy, TRUE, TRUE);
		SetDlgItemText(IDC_EDIT_FLAT_FACILITY_FEE, str);

	}NxCatchAll("Error validating facility fee.");
}

void CFacilityFeeConfigDlg::OnKillfocusEditIncrementalBaseFee() 
{
	try {
		CString strFee;
		GetDlgItemText(IDC_EDIT_INCREMENTAL_BASE_FEE, strFee);

		if (strFee.GetLength() == 0) {
			MsgBox("Please fill in the 'Incremental Base Fee' box.");
			SetDlgItemText(IDC_EDIT_INCREMENTAL_BASE_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		COleCurrency cy = ParseCurrencyFromInterface(strFee);
		if(cy.GetStatus() == COleCurrency::invalid) {
			MsgBox("Please enter a valid amount in the 'Incremental Base Fee' box.");
			SetDlgItemText(IDC_EDIT_INCREMENTAL_BASE_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		if (cy < COleCurrency::COleCurrency(0,0))
		{
			MsgBox("Fees cannot be negative. Please enter a positive number.");
			SetDlgItemText(IDC_EDIT_INCREMENTAL_BASE_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
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
			SetDlgItemText(IDC_EDIT_INCREMENTAL_BASE_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		CString str = FormatCurrencyForInterface(cy, TRUE, TRUE);
		SetDlgItemText(IDC_EDIT_INCREMENTAL_BASE_FEE, str);

	}NxCatchAll("Error validating incremental base fee.");
}

void CFacilityFeeConfigDlg::OnKillfocusEditIncrementalAdditionalFee() 
{
	try {
		CString strFee;
		GetDlgItemText(IDC_EDIT_INCREMENTAL_ADDITIONAL_FEE, strFee);

		if (strFee.GetLength() == 0) {
			MsgBox("Please fill in the 'Incremental Additional Fee' box.");
			SetDlgItemText(IDC_EDIT_INCREMENTAL_ADDITIONAL_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		COleCurrency cy = ParseCurrencyFromInterface(strFee);
		if(cy.GetStatus() == COleCurrency::invalid) {
			MsgBox("Please enter a valid amount in the 'Incremental Additional Fee' box.");
			SetDlgItemText(IDC_EDIT_INCREMENTAL_ADDITIONAL_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		if (cy < COleCurrency::COleCurrency(0,0))
		{
			MsgBox("Fees cannot be negative. Please enter a positive number.");
			SetDlgItemText(IDC_EDIT_INCREMENTAL_ADDITIONAL_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
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
			SetDlgItemText(IDC_EDIT_INCREMENTAL_ADDITIONAL_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		CString str = FormatCurrencyForInterface(cy, TRUE, TRUE);
		SetDlgItemText(IDC_EDIT_INCREMENTAL_ADDITIONAL_FEE, str);

	}NxCatchAll("Error validating incremental additional fee.");
}

void CFacilityFeeConfigDlg::OnBtnAddNewFeeOption() 
{
	CFacilityFeeNewSchedDlg dlg(this);
	if(dlg.DoModal() == IDOK) {

		//ensure we're not duplicating an hour/minute combination
		for(int i=0;i<m_FacilityFeeTable->GetRowCount();i++) {
			BOOL bFound = FALSE;
			long nCompareHours = VarLong(m_FacilityFeeTable->GetValue(i,COLUMN_HOURS),0);
			long nCompareMinutes = VarLong(m_FacilityFeeTable->GetValue(i,COLUMN_MINUTES),0);

			if(nCompareHours == dlg.m_nHours && nCompareMinutes == dlg.m_nMinutes) {
				CString str;
				str.Format("A fee for %li hours and %li minutes already exists.", dlg.m_nHours, dlg.m_nMinutes);
				AfxMessageBox(str);
				return;
			}
		}

		IRowSettingsPtr pRow = m_FacilityFeeTable->GetRow(-1);
		pRow->PutValue(COLUMN_ID,(long)-1);
		pRow->PutValue(COLUMN_HOURS,(long)dlg.m_nHours);
		pRow->PutValue(COLUMN_MINUTES,(long)dlg.m_nMinutes);
		pRow->PutValue(COLUMN_FEE,_variant_t(dlg.m_cyFee));
		m_FacilityFeeTable->AddRow(pRow);
	}	
}

void CFacilityFeeConfigDlg::OnBtnDeleteFeeOption() 
{
	try {

		if(m_FacilityFeeTable->CurSel == -1) {
			AfxMessageBox("Please select a fee before deleting.");
			return;
		}

		m_FacilityFeeTable->RemoveRow(m_FacilityFeeTable->CurSel);		

	}NxCatchAll("Error removing fee.");	
}

void CFacilityFeeConfigDlg::Load()
{
	try {

		// (j.jones 2007-10-15 12:59) - PLID 27757 - converted to use the new structure
		_RecordsetPtr rs = CreateParamRecordset("SELECT FacilityFeeSetupT.ID, FacilityFeeBillType, FacilityFlatFee, "
			"FacilityIncrementalBaseFee, FacilityBaseIncrement, FacilityIncrementalAddlFee, FacilityAddlIncrement, "
			"FacilityFeeTableSearchType, FacilityTimePromptType, "
			"FacilityPayTo, FacilityOutsideFee FROM FacilityFeeSetupT "
			"INNER JOIN LocationsT ON FacilityFeeSetupT.LocationID = LocationsT.ID "
			"WHERE FacilityFeeSetupT.ServiceID = {INT} AND FacilityFeeSetupT.LocationID = {INT}", m_nServiceID, m_nLocationID);

		if(rs->eof) {
			//if no records exist for this CPT/POS, create a new record, and pull the default values
			rs->Close();
			rs = CreateParamRecordset("SET NOCOUNT ON\r\n"
				"INSERT INTO FacilityFeeSetupT (ServiceID, LocationID) VALUES ({INT}, {INT})\r\n"
				"SET NOCOUNT OFF\r\n"
				"SELECT FacilityFeeSetupT.ID, FacilityFeeBillType, FacilityFlatFee, "
				"FacilityIncrementalBaseFee, FacilityBaseIncrement, FacilityIncrementalAddlFee, FacilityAddlIncrement, "
				"FacilityFeeTableSearchType, FacilityTimePromptType, "
				"FacilityPayTo, FacilityOutsideFee FROM FacilityFeeSetupT "
				"INNER JOIN LocationsT ON FacilityFeeSetupT.LocationID = LocationsT.ID "
				"WHERE FacilityFeeSetupT.ID = Convert(int, SCOPE_IDENTITY())",m_nServiceID, m_nLocationID);
		}

		if(!rs->eof) {

			m_nFacilityFeeSetupID = AdoFldLong(rs, "ID");

			//load the outside fee setting
			m_checkQuoteAsOutsideFee.SetCheck(AdoFldBool(rs, "FacilityOutsideFee", FALSE));

			//load the fee usage setting
			long nFacilityFeeBillType = AdoFldLong(rs, "FacilityFeeBillType",1);
			if(nFacilityFeeBillType == 1) {
				m_radioFlatFee.SetCheck(TRUE);
				m_radioIncrementalFee.SetCheck(FALSE);
				m_radioSpecificFeeTable.SetCheck(FALSE);
			}
			else if(nFacilityFeeBillType == 2) {
				m_radioFlatFee.SetCheck(FALSE);
				m_radioIncrementalFee.SetCheck(TRUE);
				m_radioSpecificFeeTable.SetCheck(FALSE);
			}
			else if(nFacilityFeeBillType == 3) {
				m_radioFlatFee.SetCheck(FALSE);
				m_radioIncrementalFee.SetCheck(FALSE);
				m_radioSpecificFeeTable.SetCheck(TRUE);
			}

			OnFeeRadioChanged();

			//load the flat fee
			COleCurrency cyFacilityFlatFee = AdoFldCurrency(rs, "FacilityFlatFee",COleCurrency(0,0));
			SetDlgItemText(IDC_EDIT_FLAT_FACILITY_FEE, FormatCurrencyForInterface(cyFacilityFlatFee,TRUE,TRUE));

			//load the incremental base fee
			COleCurrency cyFacilityIncrementalBaseFee = AdoFldCurrency(rs, "FacilityIncrementalBaseFee",COleCurrency(0,0));
			SetDlgItemText(IDC_EDIT_INCREMENTAL_BASE_FEE, FormatCurrencyForInterface(cyFacilityIncrementalBaseFee,TRUE,TRUE));

			//load the incremental base increment
			long nFacilityBaseIncrement = AdoFldLong(rs, "FacilityBaseIncrement",1);
			m_IncrementBaseCombo->SetSelByColumn(0,nFacilityBaseIncrement);

			//load the incremental additional fee
			COleCurrency cyFacilityIncrementalAddlFee = AdoFldCurrency(rs, "FacilityIncrementalAddlFee",COleCurrency(0,0));
			SetDlgItemText(IDC_EDIT_INCREMENTAL_ADDITIONAL_FEE, FormatCurrencyForInterface(cyFacilityIncrementalAddlFee,TRUE,TRUE));

			//load the incremental additional increment
			long nFacilityAddlIncrement = AdoFldLong(rs, "FacilityAddlIncrement",1);
			m_IncrementAdditionalCombo->SetSelByColumn(0,nFacilityAddlIncrement);
			
			CString str;
			// (j.jones 2007-10-15 13:33) - PLID 27757 - converted to filter on the FacilityFeeSetupT.ID
			str.Format("FacilityFeeSetupID = %li", m_nFacilityFeeSetupID);
			m_FacilityFeeTable->PutWhereClause(_bstr_t(str));
			m_FacilityFeeTable->Requery();

			//load the lesser/greater setting
			long nFacilityFeeTableSearchType = AdoFldLong(rs, "FacilityFeeTableSearchType",1);
			if(nFacilityFeeTableSearchType == 1) {
				m_radioUseLesserTime.SetCheck(TRUE);
				m_radioUseGreaterTime.SetCheck(FALSE);
			}
			else if(nFacilityFeeTableSearchType == 2) {
				m_radioUseLesserTime.SetCheck(FALSE);
				m_radioUseGreaterTime.SetCheck(TRUE);
			}

			//load the start/end time or minutes option
			long nFacilityTimePromptType = AdoFldLong(rs, "FacilityTimePromptType",1);
			if(nFacilityTimePromptType == 1) {
				m_radioUseBeginEndTimes.SetCheck(TRUE);
				m_radioUseMinutes.SetCheck(FALSE);
			}
			else if(nFacilityTimePromptType == 2) {
				m_radioUseBeginEndTimes.SetCheck(FALSE);
				m_radioUseMinutes.SetCheck(TRUE);
			}

			//PLID 16591: load the paid to field
			CString strPaidTo = AdoFldString(rs, "FacilityPayTo", "");
			SetDlgItemText(IDC_FACILITY_FEE_PAID_TO, strPaidTo);

		}
		rs->Close();

	}NxCatchAll("Error loading Facility Fee configuration.");
}

BOOL CFacilityFeeConfigDlg::Save()
{
	try {

		//save the outside fee setting
		long nOutsideFee = m_checkQuoteAsOutsideFee.GetCheck() ? 1 : 0;
		
		//save the fee usage setting
		long nFacilityFeeBillType = 1;
		if(m_radioFlatFee.GetCheck())
			nFacilityFeeBillType = 1;
		else if(m_radioIncrementalFee.GetCheck())
			nFacilityFeeBillType = 2;
		else if(m_radioSpecificFeeTable.GetCheck())
			nFacilityFeeBillType = 3;
		
		if(nFacilityFeeBillType == 3 && m_FacilityFeeTable->GetRowCount() == 0) {
			AfxMessageBox("You selected the 'Fee Table' option, but entered no fee amounts in the table.");
			return FALSE;
		}

		//save the flat fee
		COleCurrency cyFacilityFlatFee = COleCurrency(0,0);
		CString strFee;
		GetDlgItemText(IDC_EDIT_FLAT_FACILITY_FEE, strFee);
		cyFacilityFlatFee = ParseCurrencyFromInterface(strFee);

		//save the incremental base fee
		COleCurrency cyFacilityIncrementalBaseFee = COleCurrency(0,0);
		GetDlgItemText(IDC_EDIT_INCREMENTAL_BASE_FEE, strFee);
		cyFacilityIncrementalBaseFee = ParseCurrencyFromInterface(strFee);

		//save the incremental base increment
		long nFacilityBaseIncrement = 1;
		if(m_IncrementBaseCombo->CurSel != -1)
			nFacilityBaseIncrement = VarLong(m_IncrementBaseCombo->GetValue(m_IncrementBaseCombo->CurSel,0));

		//save the incremental additional fee
		COleCurrency cyFacilityIncrementalAddlFee = COleCurrency(0,0);
		GetDlgItemText(IDC_EDIT_INCREMENTAL_ADDITIONAL_FEE, strFee);
		cyFacilityIncrementalAddlFee = ParseCurrencyFromInterface(strFee);

		//save the incremental additional increment
		long nFacilityAddlIncrement = 1;
		if(m_IncrementAdditionalCombo->CurSel != -1)
			nFacilityAddlIncrement = VarLong(m_IncrementAdditionalCombo->GetValue(m_IncrementAdditionalCombo->CurSel,0));
		
		ExecuteSql("DELETE FROM LocationFacilityFeesT WHERE FacilityFeeSetupID = %li", m_nFacilityFeeSetupID);
		for(int i=0;i<m_FacilityFeeTable->GetRowCount();i++) {
			long nHours = VarLong(m_FacilityFeeTable->GetValue(i,COLUMN_HOURS),0);
			long nMinutes = VarLong(m_FacilityFeeTable->GetValue(i,COLUMN_MINUTES),0);
			COleCurrency cyFee = VarCurrency(m_FacilityFeeTable->GetValue(i,COLUMN_FEE),COleCurrency(0,0));

			ExecuteSql("INSERT INTO LocationFacilityFeesT (ID, FacilityFeeSetupID, Hours, Minutes, Fee) "
				"VALUES (%li, %li, %li, %li, Convert(money,'%s'))",NewNumber("LocationFacilityFeesT","ID"),
				m_nFacilityFeeSetupID, nHours, nMinutes, FormatCurrencyForSql(cyFee));
		}

		//save the lesser/greater setting
		long nFacilityFeeTableSearchType = 2;
		if(m_radioUseLesserTime.GetCheck())
			nFacilityFeeTableSearchType = 1;
		else if(m_radioUseGreaterTime.GetCheck())
			nFacilityFeeTableSearchType = 2;

		//save the start/end time or minutes option
		long nFacilityTimePromptType = 1;
		if(m_radioUseBeginEndTimes.GetCheck())
			nFacilityTimePromptType = 1;
		else if(m_radioUseMinutes.GetCheck())
			nFacilityTimePromptType = 2;

		//PLID 16591: update the facility fee paid to field
		CString strPaidTo;
		GetDlgItemText(IDC_FACILITY_FEE_PAID_TO, strPaidTo);

		// (j.jones 2007-10-15 13:47) - PLID 27757 - now everything but FacilityPayTo is in its own table
		ExecuteSql("UPDATE LocationsT SET FacilityPayTo = '%s' WHERE ID = %li", _Q(strPaidTo), m_nLocationID);

		ExecuteSql("UPDATE FacilityFeeSetupT SET FacilityFeeBillType = %li, FacilityFlatFee = Convert(money, '%s'), "
			"FacilityIncrementalBaseFee = Convert(money, '%s'), FacilityBaseIncrement = %li, "
			"FacilityIncrementalAddlFee = Convert(money, '%s'), FacilityAddlIncrement = %li, "
			"FacilityFeeTableSearchType = %li, FacilityTimePromptType = %li, "
			"FacilityOutsideFee = %li WHERE ID = %li",
			nFacilityFeeBillType, FormatCurrencyForSql(cyFacilityFlatFee),
			FormatCurrencyForSql(cyFacilityIncrementalBaseFee), nFacilityBaseIncrement,
			FormatCurrencyForSql(cyFacilityIncrementalAddlFee), nFacilityAddlIncrement,
			nFacilityFeeTableSearchType, nFacilityTimePromptType,
			nOutsideFee, m_nFacilityFeeSetupID);
				
		return TRUE;

	}NxCatchAll("Error saving facility fee setup.");

	return FALSE;
}

void CFacilityFeeConfigDlg::OnEditingFinishingFacilityFeeTable(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	switch(nCol) {
		case COLUMN_HOURS: {

			long nHours = VarLong(pvarNewValue);
			long nMinutes = VarLong(m_FacilityFeeTable->GetValue(nRow,COLUMN_MINUTES),0);

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
			for(int i=0;i<m_FacilityFeeTable->GetRowCount();i++) {
				BOOL bFound = FALSE;
				if(nRow != i) {
					long nCompareHours = VarLong(m_FacilityFeeTable->GetValue(i,COLUMN_HOURS),0);
					long nCompareMinutes = VarLong(m_FacilityFeeTable->GetValue(i,COLUMN_MINUTES),0);

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
			long nHours = VarLong(m_FacilityFeeTable->GetValue(nRow,COLUMN_HOURS),0);

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
			for(int i=0;i<m_FacilityFeeTable->GetRowCount();i++) {
				BOOL bFound = FALSE;
				if(nRow != i) {
					long nCompareHours = VarLong(m_FacilityFeeTable->GetValue(i,COLUMN_HOURS),0);
					long nCompareMinutes = VarLong(m_FacilityFeeTable->GetValue(i,COLUMN_MINUTES),0);

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

void CFacilityFeeConfigDlg::OnEditingFinishedFacilityFeeTable(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	//we actually don't need to do anything here since we don't save until OnOK()	
	m_FacilityFeeTable->Sort();
}

// (j.jones 2007-10-16 08:48) - PLID 27760 - added ability to copy the setup between service codes
void CFacilityFeeConfigDlg::OnBtnCopyFacfeeToCode() 
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
			if(IDOK == dlg.Open(str, "", "ID", "Name", "Select a Service Code to copy this Facility Fee Setup to:")) {

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

		//check to see if there is an existing facility setup for this code,
		//also grab its facility fee statii

		CString strNewCode = "";
		BOOL bUseFacilityFeeBilling = FALSE;
		BOOL bHasFacilityFeeSettings = FALSE;		
		
		_RecordsetPtr rs = CreateParamRecordset("SELECT Code, UseFacilityBilling, "
			"Convert(bit, CASE WHEN EXISTS (SELECT ID FROM FacilityFeeSetupT WHERE ServiceID = ServiceT.ID) THEN 1 ELSE 0 END) AS HasFacilityFeeSettings "
			"FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE ServiceT.ID = {INT}", nNewServiceID);

		if(rs->eof) {
			//shouldn't be possible
			ASSERT(FALSE);
			return;
		}
		else {
			strNewCode = AdoFldString(rs, "Code","");
			bUseFacilityFeeBilling = AdoFldBool(rs, "UseFacilityBilling", FALSE);
			bHasFacilityFeeSettings = AdoFldBool(rs, "HasFacilityFeeSettings", FALSE);			
		}
		rs->Close();

		//now warn accordingly
		if(bHasFacilityFeeSettings) {
			str.Format("This action will clear out all existing facility fee settings for the selected service code '%s',\n"
				"and copy the facility fee settings for all places of service from code '%s' to code '%s'.\n\n"
				"Are you sure you wish to copy these settings?", strNewCode, m_strServiceCode, strNewCode);
		}
		else {
			str.Format("This action copy the facility fee settings for all places of service from code '%s' to code '%s'.\n\n"
				"Are you sure you wish to copy these settings?", m_strServiceCode, strNewCode);
		}

		if(IDNO == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		//one more warning - if UseFacilityBilling is not checked, prompt to check it (even if our source code doesn't have it checked)
		BOOL bNeedsFacilityFeeBillingEnabled = FALSE;
		if(!bUseFacilityFeeBilling) {
			str.Format("The selected service code '%s' does not have the 'Use Facility Billing' option checked.\n"
				"The copied facility fee setup will not be used until this option is checked.\n\n"
				"Do you wish to enable facility billing for this code now?", strNewCode);
			if(IDYES == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
				//if yes, set our boolean, we'll update in data shortly
				bNeedsFacilityFeeBillingEnabled = TRUE;
			}
		}

		//now we are ready to update in data

		CString strSqlBatch = BeginSqlBatch();

		//first clear out all existing data for this service
		//(note: bHasFacilityFeeSettings would reflect whether any data exists to delete, but in
		//the slim chance any was created while the messageboxes were up, we need to make sure
		//everything is deleted, so always delete even if bHasFacilityFeeSettings = FALSE)
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM LocationFacilityFeesT WHERE FacilityFeeSetupID IN (SELECT ID FROM FacilityFeeSetupT WHERE ServiceID = %li)", nNewServiceID);
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM FacilityFeeSetupT WHERE ServiceID = %li", nNewServiceID);

		//now copy all the data from this code to the new code
		
		//bulk copy the FacilityFeeSetupT records
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO FacilityFeeSetupT (ServiceID, LocationID, "
			"FacilityFeeBillType, FacilityFlatFee, FacilityIncrementalBaseFee, FacilityBaseIncrement, FacilityIncrementalAddlFee, "
			"FacilityAddlIncrement, FacilityFeeTableSearchType, FacilityTimePromptType, FacilityOutsideFee) "
			"SELECT %li, LocationID, "
			"FacilityFeeBillType, FacilityFlatFee, FacilityIncrementalBaseFee, FacilityBaseIncrement, FacilityIncrementalAddlFee, "
			"FacilityAddlIncrement, FacilityFeeTableSearchType, FacilityTimePromptType, FacilityOutsideFee "
			"FROM FacilityFeeSetupT WHERE ServiceID = %li", nNewServiceID, m_nServiceID);

		//updating LocationFacilityFeesT is a little trickier because we don't have an Identity,
		//therefore we can't bulk copy, so query data and pass in IDs to copy from
		_RecordsetPtr rsFees = CreateParamRecordset("SELECT LocationFacilityFeesT.ID, LocationID FROM LocationFacilityFeesT "
			"INNER JOIN FacilityFeeSetupT ON LocationFacilityFeesT.FacilityFeeSetupID = FacilityFeeSetupT.ID "
			"WHERE FacilityFeeSetupID IN (SELECT ID FROM FacilityFeeSetupT WHERE ServiceID = {INT})", m_nServiceID); 

		if(!rsFees->eof) {
			//if we have any records, declare and initialize the new ID variable as the maximum ID in the table,
			//don't increment it yet
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewLocationFacilityFeeID INT");
			AddStatementToSqlBatch(strSqlBatch, "SET @nNewLocationFacilityFeeID = (SELECT Coalesce(Max(ID),0) AS NewID FROM LocationFacilityFeesT)");

			//also declare our destination FacilityFeeSetupT.ID variable
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewFacilityFeeSetupID INT");
		}

		while(!rsFees->eof) {
			long nLocationFacilityFeeID = AdoFldLong(rsFees, "ID");
			long nCurLocationID = AdoFldLong(rsFees, "LocationID");

			//increment the ID
			AddStatementToSqlBatch(strSqlBatch, "SET @nNewLocationFacilityFeeID = @nNewLocationFacilityFeeID + 1");

			//update our @nNewFacilityFeeSetupID variable
			AddStatementToSqlBatch(strSqlBatch, "SET @nNewFacilityFeeSetupID = (SELECT ID FROM FacilityFeeSetupT WHERE ServiceID = %li AND LocationID = %li)", nNewServiceID, nCurLocationID);

			//and now copy the record
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO LocationFacilityFeesT (ID, FacilityFeeSetupID, Hours, Minutes, Fee) "
				"SELECT @nNewLocationFacilityFeeID, @nNewFacilityFeeSetupID, Hours, Minutes, Fee FROM LocationFacilityFeesT WHERE ID = %li", nLocationFacilityFeeID);

			rsFees->MoveNext();
		}
		rsFees->Close();

		//we don't need to copy FacilityPayTo because it's already set in LocationsT

		//and now update the UseFacilityBilling status, if needed
		if(bNeedsFacilityFeeBillingEnabled) {
			//update the Facility Fee status as well as UseFacilityBilling
			//and ensure it is NOT marked as an anesthesia fee
			AddStatementToSqlBatch(strSqlBatch, "UPDATE ServiceT SET FacilityFee = 1, UseFacilityBilling = 1, Anesthesia = 0, UseAnesthesiaBilling = 0 WHERE ID = %li", nNewServiceID);
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

	}NxCatchAll("Error in CFacilityFeeConfigDlg::OnBtnCopyFacfeeToCode");
}

// (j.jones 2007-10-16 10:48) - PLID 27761 - added ability to copy the setup between places of service
void CFacilityFeeConfigDlg::OnBtnCopyFacfeeToPos() 
{
	try {
		
		//a -1 service shouldn't be possible
		if(m_nServiceID == -1) {
			ASSERT(FALSE);
			return;
		}

		//we should have a place of service selected
		if(m_nLocationID == -1 || m_POSCombo->CurSel == -1) {
			AfxMessageBox("Please select a Place Of Service before trying to copy the Facility Fee Setup.");
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
			if(IDOK == dlg.Open("LocationsT", strWhere, "ID", "Name", "Select a Place Of Service to copy this Facility Fee Setup to:")) {

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

		//check to see if there is an existing facility fee setup for this POS

		CString strNewPOS = "";
		BOOL bHasFacilityFeeSettings = FALSE;		
		
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name, "
			"Convert(bit, CASE WHEN EXISTS (SELECT ID FROM FacilityFeeSetupT WHERE LocationID = LocationsT.ID) THEN 1 ELSE 0 END) AS HasFacilityFeeSettings "
			"FROM LocationsT WHERE ID = {INT}", nNewPOSID);

		if(rs->eof) {
			//shouldn't be possible
			ASSERT(FALSE);
			return;
		}
		else {
			strNewPOS = AdoFldString(rs, "Name","");
			bHasFacilityFeeSettings = AdoFldBool(rs, "HasFacilityFeeSettings", FALSE);			
		}
		rs->Close();

		//now warn accordingly
		CString str;
		if(bHasFacilityFeeSettings) {
			str.Format("This action will clear out the existing facility fee settings for the selected Place Of Service '%s' and code '%s',\n"
				"and copy the facility fee settings from the Place Of Service '%s' to '%s' for the code '%s'.\n\n"
				"Are you sure you wish to copy these settings?", strNewPOS, m_strServiceCode, strCurPOS, strNewPOS, m_strServiceCode);
		}
		else {
			str.Format("This action will copy the facility fee settings from the Place Of Service '%s' to '%s' for the code '%s'.\n\n"
				"Are you sure you wish to copy these settings?", strCurPOS, strNewPOS, m_strServiceCode);
		}

		if(IDNO == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		//if the current location has anything in the FacilityPayTo field,
		//prompt to copy it to the new Place Of Service
		BOOL bNeedsPayToTextCopied = FALSE;
		if(ReturnsRecords("SELECT FacilityPayTo FROM LocationsT WHERE ID = %li AND FacilityPayTo Is Not Null AND FacilityPayTo <> ''", m_nLocationID)) {
			str.Format("Would you like to also copy the 'Check Payable To...' text from '%s' to '%s'?", strCurPOS, strNewPOS);
			if(IDYES == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
				//if yes, set our boolean, we'll update in data shortly
				bNeedsPayToTextCopied = TRUE;
			}
		}

		//now we are ready to update in data

		CString strSqlBatch = BeginSqlBatch();

		//first clear out all existing data for this service/location combination
		//(note: bHasFacilityFeeSettings would reflect whether any data exists to delete, but in
		//the slim chance any was created while the messageboxes were up, we need to make sure
		//everything is deleted, so always delete even if bHasFacilityFeeSettings = FALSE)
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM LocationFacilityFeesT WHERE FacilityFeeSetupID IN (SELECT ID FROM FacilityFeeSetupT WHERE ServiceID = %li AND LocationID = %li)", m_nServiceID, nNewPOSID);
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM FacilityFeeSetupT WHERE ServiceID = %li AND LocationID = %li", m_nServiceID, nNewPOSID);

		//now copy all the data from this POS to the new POS

		//copy the FacilityFeeSetupT record
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO FacilityFeeSetupT (ServiceID, LocationID, "
			"FacilityFeeBillType, FacilityFlatFee, FacilityIncrementalBaseFee, FacilityBaseIncrement, FacilityIncrementalAddlFee, "
			"FacilityAddlIncrement, FacilityFeeTableSearchType, FacilityTimePromptType, FacilityOutsideFee) "
			"SELECT ServiceID, %li, "
			"FacilityFeeBillType, FacilityFlatFee, FacilityIncrementalBaseFee, FacilityBaseIncrement, FacilityIncrementalAddlFee, "
			"FacilityAddlIncrement, FacilityFeeTableSearchType, FacilityTimePromptType, FacilityOutsideFee "
			"FROM FacilityFeeSetupT WHERE ServiceID = %li AND LocationID = %li", nNewPOSID, m_nServiceID, m_nLocationID);

		//updating LocationFacilityFeesT is a little trickier because we don't have an Identity,
		//therefore we can't bulk copy, so query data and pass in IDs to copy from
		_RecordsetPtr rsFees = CreateParamRecordset("SELECT LocationFacilityFeesT.ID, LocationID FROM LocationFacilityFeesT "
			"INNER JOIN FacilityFeeSetupT ON LocationFacilityFeesT.FacilityFeeSetupID = FacilityFeeSetupT.ID "
			"WHERE FacilityFeeSetupID IN (SELECT ID FROM FacilityFeeSetupT WHERE ServiceID = {INT} AND LocationID = {INT})", m_nServiceID, m_nLocationID); 

		if(!rsFees->eof) {
			//if we have any records, declare and initialize the new ID variable as the maximum ID in the table,
			//don't increment it yet
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewLocationFacilityFeeID INT");
			AddStatementToSqlBatch(strSqlBatch, "SET @nNewLocationFacilityFeeID = (SELECT Coalesce(Max(ID),0) AS NewID FROM LocationFacilityFeesT)");

			//also declare our destination FacilityFeeSetupT.ID variable
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewFacilityFeeSetupID INT");
		}

		while(!rsFees->eof) {
			long nLocationFacilityFeeID = AdoFldLong(rsFees, "ID");

			//increment the ID
			AddStatementToSqlBatch(strSqlBatch, "SET @nNewLocationFacilityFeeID = @nNewLocationFacilityFeeID + 1");

			//update our @nNewFacilityFeeSetupID variable
			AddStatementToSqlBatch(strSqlBatch, "SET @nNewFacilityFeeSetupID = (SELECT ID FROM FacilityFeeSetupT WHERE ServiceID = %li AND LocationID = %li)", m_nServiceID, nNewPOSID);

			//and now copy the record
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO LocationFacilityFeesT (ID, FacilityFeeSetupID, Hours, Minutes, Fee) "
				"SELECT @nNewLocationFacilityFeeID, @nNewFacilityFeeSetupID, Hours, Minutes, Fee FROM LocationFacilityFeesT WHERE ID = %li", nLocationFacilityFeeID);

			rsFees->MoveNext();
		}
		rsFees->Close();

		//and now update the FacilityPayTo text, if needed
		if(bNeedsPayToTextCopied) {
			AddStatementToSqlBatch(strSqlBatch, "UPDATE LocationsT SET FacilityPayTo = (SELECT FacilityPayTo FROM LocationsT WHERE ID = %li) WHERE ID = %li", m_nLocationID, nNewPOSID);
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

	}NxCatchAll("Error in CFacilityFeeConfigDlg::OnBtnCopyFacfeeToPos");
}
