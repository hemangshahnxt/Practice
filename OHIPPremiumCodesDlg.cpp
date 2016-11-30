// OHIPPremiumCodesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OHIPPremiumCodesDlg.h"
#include "InternationalUtils.h"

// COHIPPremiumCodesDlg dialog

// (j.jones 2009-03-26 17:15) - PLID 32324 - created

using namespace ADODB;
using namespace NXDATALIST2Lib;

enum CodeComboColumns {

	cccID = 0,
	cccCode,
	cccSubCode,
	cccName,
	tccPremiumCode,
	tccUsePercent,	
	tccCalculateText,
	tccPercent,
	tccPrice,
	tccAddMultiple,
	tccAddMultipleText,
};

COHIPPremiumCodesDlg::COHIPPremiumCodesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(COHIPPremiumCodesDlg::IDD, pParent)
{
	m_nDefaultServiceID = -1;
	m_bUsePercent = TRUE;
	m_bPercentChanged = FALSE;
}

COHIPPremiumCodesDlg::~COHIPPremiumCodesDlg()
{
}

void COHIPPremiumCodesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_CLOSE_PREMIUM_CODES, m_btnClose);
	DDX_Control(pDX, IDC_CHECK_IS_PREMIUM_CODE, m_checkIsPremiumCode);
	DDX_Control(pDX, IDC_RADIO_ADD_PERCENT, m_radioPercent);
	DDX_Control(pDX, IDC_RADIO_ADD_FLAT_FEE, m_radioFlatFee);
	DDX_Control(pDX, IDC_RADIO_ADD_ONCE, m_radioAddOnce);
	DDX_Control(pDX, IDC_RADIO_ADD_MULTIPLE, m_radioAddMultiple);
	DDX_Control(pDX, IDC_EDIT_ADD_PERCENT, m_editPercent);
	DDX_Control(pDX, IDC_EDIT_ADD_AMT, m_editFlatFee);
}


BEGIN_MESSAGE_MAP(COHIPPremiumCodesDlg, CNxDialog)
	ON_BN_CLICKED(IDC_CHECK_IS_PREMIUM_CODE, OnCheckIsPremiumCode)
	ON_BN_CLICKED(IDC_RADIO_ADD_PERCENT, OnRadioAddPercent)
	ON_BN_CLICKED(IDC_RADIO_ADD_FLAT_FEE, OnRadioAddFlatFee)
	ON_BN_CLICKED(IDC_BTN_CLOSE_PREMIUM_CODES, OnBtnClose)
	ON_EN_KILLFOCUS(IDC_EDIT_ADD_PERCENT, OnEnKillfocusEditAddPercent)
	ON_BN_CLICKED(IDC_RADIO_ADD_ONCE, OnRadioAddOnce)
	ON_BN_CLICKED(IDC_RADIO_ADD_MULTIPLE, OnRadioAddMultiple)
	ON_EN_CHANGE(IDC_EDIT_ADD_PERCENT, OnEnChangeEditAddPercent)
END_MESSAGE_MAP()


// COHIPPremiumCodesDlg message handlers

BOOL COHIPPremiumCodesDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);

		CWaitCursor pWait;

		m_CodeCombo = BindNxDataList2Ctrl(IDC_SERVICE_CODE_COMBO, true);

		IRowSettingsPtr pRow = NULL;
		if(m_nDefaultServiceID != -1) {
			pRow = m_CodeCombo->SetSelByColumn(cccID, m_nDefaultServiceID);
		}
		
		if(pRow == NULL) {
			//get the first row
			m_CodeCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			pRow = m_CodeCombo->GetFirstRow();
			m_CodeCombo->PutCurSel(pRow);
		}

		if(pRow == NULL) {
			AfxMessageBox("You have no active service codes in your system. This feature cannot be used.");
			CNxDialog::OnCancel();
			return TRUE;
		}

		Load();

	}NxCatchAll("Error in COHIPPremiumCodesDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COHIPPremiumCodesDlg::OnBtnClose()
{
	try {

		CNxDialog::OnOK();

	}NxCatchAll("Error in COHIPPremiumCodesDlg::OnBtnClose");
}

void COHIPPremiumCodesDlg::OnCheckIsPremiumCode()
{
	try {

		BOOL bIsPremiumCode = m_checkIsPremiumCode.GetCheck();

		//enable the controls accordingly
		m_radioPercent.EnableWindow(bIsPremiumCode);
		m_radioFlatFee.EnableWindow(bIsPremiumCode);
		m_radioAddOnce.EnableWindow(bIsPremiumCode);
		m_radioAddMultiple.EnableWindow(bIsPremiumCode);
		m_editPercent.EnableWindow(bIsPremiumCode && m_radioPercent.GetCheck());

		//the flat fee edit is always read-only

		Save();

	}NxCatchAll("Error in COHIPPremiumCodesDlg::OnCheckIsPremiumCode");
}

void COHIPPremiumCodesDlg::OnRadioAddPercent()
{
	try {

		//only do something if we changed the value
		if(m_bUsePercent != m_radioPercent.GetCheck()) {

			m_bUsePercent = m_radioPercent.GetCheck();		
		
			//enable the Percent edit
			m_editPercent.EnableWindow(m_radioPercent.GetCheck());

			//the flat fee edit is always read-only
		
			Save();
		}

	}NxCatchAll("Error in COHIPPremiumCodesDlg::OnRadioAddPercent");
}

void COHIPPremiumCodesDlg::OnRadioAddFlatFee()
{
	try {

		OnRadioAddPercent();

	}NxCatchAll("Error in COHIPPremiumCodesDlg::OnRadioAddFlatFee");
}
void COHIPPremiumCodesDlg::OnEnKillfocusEditAddPercent()
{
	try {

		//only save if it changed
		if(m_bPercentChanged) {
			Save();
		}

	}NxCatchAll("Error in COHIPPremiumCodesDlg::OnEnKillfocusEditAddPercent");
}

void COHIPPremiumCodesDlg::OnRadioAddOnce()
{
	try {

		Save();

	}NxCatchAll("Error in COHIPPremiumCodesDlg::OnRadioAddOnce");
}

void COHIPPremiumCodesDlg::OnRadioAddMultiple()
{
	try {

		Save();

	}NxCatchAll("Error in COHIPPremiumCodesDlg::OnRadioAddMultiple");
}

void COHIPPremiumCodesDlg::Load()
{
	try {

		IRowSettingsPtr pRow = m_CodeCombo->GetCurSel();
		if(pRow == NULL) {
			ThrowNxException("No service code is selected!");
		}

		//pull the data from the dropdown
		BOOL bIsPremiumCode = VarBool(pRow->GetValue(tccPremiumCode), FALSE);
		m_bUsePercent = VarBool(pRow->GetValue(tccUsePercent), TRUE);
		COleCurrency cyPrice = VarCurrency(pRow->GetValue(tccPrice), COleCurrency(0,0));
		double dblPercent = VarDouble(pRow->GetValue(tccPercent), 0.0);
		BOOL bAddMultiple = VarBool(pRow->GetValue(tccAddMultiple), FALSE);

		//fill the controls
		m_checkIsPremiumCode.SetCheck(bIsPremiumCode);
		m_radioPercent.SetCheck(m_bUsePercent);
		m_radioFlatFee.SetCheck(!m_bUsePercent);
		m_editPercent.SetWindowText(AsString(dblPercent));
		m_bPercentChanged = FALSE;
		m_editFlatFee.SetWindowText(FormatCurrencyForInterface(cyPrice));
		m_radioAddOnce.SetCheck(!bAddMultiple);
		m_radioAddMultiple.SetCheck(bAddMultiple);

		//enable the controls accordingly
		m_radioPercent.EnableWindow(bIsPremiumCode);
		m_radioFlatFee.EnableWindow(bIsPremiumCode);
		m_radioAddOnce.EnableWindow(bIsPremiumCode);
		m_radioAddMultiple.EnableWindow(bIsPremiumCode);
		m_editPercent.EnableWindow(bIsPremiumCode && m_radioPercent.GetCheck());

	}NxCatchAll("Error in COHIPPremiumCodesDlg::Load");
}

BOOL COHIPPremiumCodesDlg::Save()
{
	try {

		IRowSettingsPtr pRow = m_CodeCombo->GetCurSel();
		if(pRow == NULL) {
			ThrowNxException("No service code is selected! (1)");
		}

		long nServiceID = VarLong(pRow->GetValue(cccID));

		if(nServiceID == -1) {
			ThrowNxException("No service code is selected! (2)");
		}

		BOOL bIsPremiumCode = m_checkIsPremiumCode.GetCheck();
		BOOL bAddMultiple = m_radioAddMultiple.GetCheck();

		double dblPercent = 0.0;
		CString strPercent;
		m_editPercent.GetWindowText(strPercent);
		dblPercent = atof(strPercent);

		//keep it reasonable
		if(dblPercent < 0.0 || dblPercent > 500.0) {
			dblPercent = 0.0;
		}

		//whatever it is we are saving, show it on the screen
		CString strNewPercent = AsString(dblPercent);
		m_editPercent.SetWindowText(strNewPercent);
		m_bPercentChanged = FALSE;

		//this can't be parameterized because parameter statements do not currently support double values
		ExecuteSql("UPDATE CPTCodeT SET OHIPPremiumCode = %li, "
			"OHIPPC_UsePercent = %li, OHIPPC_Percentage = %g, OHIPPC_AddMultiple = %li "
			"WHERE ID = %li",
			bIsPremiumCode ? 1 : 0, m_bUsePercent ? 1 : 0, dblPercent, bAddMultiple ? 1 : 0, nServiceID);

		//now apply this data to our dropdown
		pRow->PutValue(tccPremiumCode, bIsPremiumCode ? g_cvarTrue : g_cvarFalse);
		pRow->PutValue(tccUsePercent, m_bUsePercent ? g_cvarTrue : g_cvarFalse);
		pRow->PutValue(tccCalculateText, m_bUsePercent ? "Percent" : "Standard Fee");
		pRow->PutValue(tccPercent, dblPercent);
		pRow->PutValue(tccAddMultiple, bAddMultiple ? g_cvarTrue : g_cvarFalse);
		pRow->PutValue(tccAddMultipleText, bAddMultiple ?  "Add Per Service" : "Add Once");

		//re-sort incase they are sorting on any of these fields
		m_CodeCombo->Sort();

		//send a tablechecker
		CClient::RefreshTable(NetUtils::CPTCodeT, nServiceID);

		//now warn if the percent isn't what they typed in
		if(strNewPercent != strPercent) {
			AfxMessageBox("The percentage value has been changed from the value that was entered. You may have entered an invalid value.\n"
				"Please confirm that the percentage field is correct.");
		}

	}NxCatchAll("Error in COHIPPremiumCodesDlg::Save");

	return FALSE;
}

BEGIN_EVENTSINK_MAP(COHIPPremiumCodesDlg, CNxDialog)
ON_EVENT(COHIPPremiumCodesDlg, IDC_SERVICE_CODE_COMBO, 1, OnSelChangingServiceCodeCombo, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(COHIPPremiumCodesDlg, IDC_SERVICE_CODE_COMBO, 16, OnSelChosenServiceCodeCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()

void COHIPPremiumCodesDlg::OnSelChangingServiceCodeCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		//require a row to be selected
		if(lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

	}NxCatchAll("Error in COHIPPremiumCodesDlg::OnSelChangingServiceCodeCombo");
}

void COHIPPremiumCodesDlg::OnSelChosenServiceCodeCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//get the first row
			pRow = m_CodeCombo->GetFirstRow();
			m_CodeCombo->PutCurSel(pRow);
		}

		if(pRow == NULL) {
			AfxMessageBox("You have no active service codes in your system. This dialog will now close.");
			CNxDialog::OnCancel();
			return;
		}

		Load();

	}NxCatchAll("Error in COHIPPremiumCodesDlg::OnSelChosenServiceCodeCombo");
}

void COHIPPremiumCodesDlg::OnEnChangeEditAddPercent()
{
	try {

		m_bPercentChanged = TRUE;

	}NxCatchAll("Error in COHIPPremiumCodesDlg::OnEnChangeEditAddPercent");
}
