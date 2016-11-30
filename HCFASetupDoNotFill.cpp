// HCFASetupDoNotFill.cpp : implementation file
//

#include "stdafx.h"
#include "HCFASetupInfo.h"
#include "HCFASetupDoNotFill.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int BACKGROUND_COLOR  = GetNxColor(GNC_ADMIN, 0);

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CHCFASetupDoNotFill dialog


CHCFASetupDoNotFill::CHCFASetupDoNotFill(CWnd* pParent /*=NULL*/)
	: CNxDialog(CHCFASetupDoNotFill::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHCFASetupDoNotFill)
	m_nHCFASetupGroupID = -1;
	m_bAnyCompaniesOnNewHCFA = true;
	m_bAnyCompaniesOnOldHCFA = false;
	//}}AFX_DATA_INIT
}


void CHCFASetupDoNotFill::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHCFASetupDoNotFill)
	DDX_Control(pDX, IDC_CHECK_HIDE_BOX17B, m_checkHideBox17b);
	DDX_Control(pDX, IDC_CHECK_EXCLUDE_ALL_BOX29, m_checkExcludeBox29);
	DDX_Control(pDX, IDC_CHECK_24J_NPI, m_checkHideBox24JNPI);
	DDX_Control(pDX, IDC_CHECK_HIDE_BOX_32_NAME_ADD, m_checkHideBox32NameAdd);
	DDX_Control(pDX, IDC_CHECK_HIDE_BOX_32_NPI_ID, m_checkHideBox32NPIID);
	DDX_Control(pDX, IDC_CHECK_24A_TO, m_checkHide24ATo);
	DDX_Control(pDX, IDC_CHECK_HIDE_BOX11C, m_checkHideBox11c);
	DDX_Control(pDX, IDC_CHECK_HIDE_BOX11B, m_checkHideBox11b);
	DDX_Control(pDX, IDC_CHECK_HIDE_BOX11A_GENDER, m_checkHideBox11a_Gender);
	DDX_Control(pDX, IDC_CHECK_HIDE_BOX11A_BIRTHDATE, m_checkHideBox11a_Birthdate);
	DDX_Control(pDX, IDC_CHECK_SHOW_WHICH_CODES_COMMAS, m_checkShowWhichCodesCommas);
	DDX_Control(pDX, IDC_CHECK_SECONDARY_FILL_BOX_11, m_checkSecondaryFillBox11);
	DDX_Control(pDX, IDC_CHECK_SHOW_ANESTHESIA_MINUTES, m_checkShowAnesthMinutesOnly);
	DDX_Control(pDX, IDC_CHECK_PRINT_P_IN_BOX_1A, m_checkPrintPInBox1a);
	DDX_Control(pDX, IDC_CHECK_HIDE_BOX11D_ALWAYS, m_checkHideBox11DAlways);
	DDX_Control(pDX, IDC_CHECK_SHOW_SECONDARY_24K_NUMBER, m_checkShowSecondary24JNumber);
	DDX_Control(pDX, IDC_CHECK_SHOW_SECONDARY_PIN_NUMBER, m_checkShowSecondaryPINNumber);
	DDX_Control(pDX, IDC_CHECK_HIDE_BOX30, m_checkHideBox30);
	DDX_Control(pDX, IDC_CHECK_HIDE_BOX11, m_checkHideBox11);
	DDX_Control(pDX, IDC_CHECK_HIDE_BOX9, m_checkHideBox9);
	DDX_Control(pDX, IDC_CHECK_HIDE_BOX7, m_checkHideBox7);
	DDX_Control(pDX, IDC_CHECK_HIDE_BOX4, m_checkHideBox4);
	DDX_Control(pDX, IDC_CHECK_11D, m_check11D);
	DDX_Control(pDX, IDC_31_CHECK, m_31);
	DDX_Control(pDX, IDC_CHECK_SHOW_RESP_NAME, m_checkShowRespName);
	DDX_Control(pDX, IDC_CHECK_SHOW_ANESTHESIA_TIMES, m_checkShowAnesthTimes);
	DDX_Control(pDX, IDC_CHECK_SHOW_SECONDARY_INS_ADD, m_checkShowSecInsAdd);
	DDX_Control(pDX, IDC_CHECK_SHOW_APPLIES, m_checkShowApplies);
	DDX_Control(pDX, IDC_CHECK_EXCLUDE_ADJUSTMENTS, m_checkExcludeAdjustments);
	DDX_Control(pDX, IDC_CHECK_EXCLUDE_ADJUSTMENTS_FROM_BALANCE, m_checkExcludeAdjustmentsFromBalance);
	DDX_Control(pDX, IDC_LEFT_ALIGN_INS_ADDRESS, m_checkLeftAlignInsAddress);
	DDX_Control(pDX, IDC_CHECK_SHOW_INS_ADDRESS, m_checkShowAddress);
	DDX_Control(pDX, IDC_CHECK_SHOW_DIAG_DESC, m_checkShowDiagDesc);
	DDX_Control(pDX, IDC_CHECK_SHOW_DIAG_DECIMALS, m_checkShowDiagDecimals);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHCFASetupDoNotFill, CNxDialog)
	//{{AFX_MSG_MAP(CHCFASetupDoNotFill)
	ON_BN_CLICKED(IDC_CHECK_SHOW_INS_ADDRESS, OnCheckShowInsAddress)
	ON_BN_CLICKED(IDC_CHECK_SHOW_APPLIES, OnCheckShowApplies)
	ON_BN_CLICKED(IDC_CHECK_SHOW_SECONDARY_INS_ADD, OnCheckShowSecondaryInsAdd)
	ON_BN_CLICKED(IDC_CHECK_EXCLUDE_ADJUSTMENTS, OnCheckExcludeAdjustments)
	ON_BN_CLICKED(IDC_CHECK_HIDE_BOX11D_ALWAYS, OnCheckHideBox11dAlways)
	ON_BN_CLICKED(IDC_CHECK_11D, OnCheck11d)
	ON_BN_CLICKED(IDC_CHECK_SHOW_ANESTHESIA_TIMES, OnCheckShowAnesthesiaTimes)
	ON_BN_CLICKED(IDC_CHECK_HIDE_BOX_32_NAME_ADD, OnCheckHideBox32NameAdd)
	ON_BN_CLICKED(IDC_CHECK_HIDE_BOX_32_NPI_ID, OnCheckHideBox32NpiId)
	ON_BN_CLICKED(IDC_CHECK_EXCLUDE_ALL_BOX29, OnCheckExcludeAllBox29)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHCFASetupDoNotFill message handlers

BOOL CHCFASetupDoNotFill::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/30/2008) - PLID 29860 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	// (j.jones 2013-08-09 15:31) - PLID 57958 - Track whether any companies in this group
	// are on the new HCFA form, vs. on the old HCFA form.
	ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT Sum(CASE WHEN HCFAUpgradeDate <= GetDate() THEN 1 ELSE 0 END) AS CountCompaniesOnNewHCFA, "
				"Sum(CASE WHEN HCFAUpgradeDate > GetDate() THEN 1 ELSE 0 END) AS CountCompaniesOnOldHCFA "
				"FROM InsuranceCoT "
				"WHERE HCFASetupGroupID = {INT} "
				"GROUP BY HCFASetupGroupID", m_nHCFASetupGroupID);
	if(!rs->eof) {
		m_bAnyCompaniesOnNewHCFA = VarLong(rs->Fields->Item["CountCompaniesOnNewHCFA"]->Value, 0) > 0 ? true : false;
		m_bAnyCompaniesOnOldHCFA = VarLong(rs->Fields->Item["CountCompaniesOnOldHCFA"]->Value, 0) > 0 ? true : false;
	}
	rs->Close();

	IRowSettingsPtr pRow;

	m_Accepted12Combo = BindNxDataListCtrl(this, IDC_ACCEPTED12_COMBO, GetRemoteData(), false);
	m_Accepted13Combo = BindNxDataListCtrl(this, IDC_ACCEPTED13_COMBO, GetRemoteData(), false);	
	// (j.jones 2007-05-11 14:11) - PLID 25932 - reworked Box 32 options
	m_HideBox32NameAddCombo = BindNxDataListCtrl(this, IDC_HIDE_BOX_32_NAME_ADD_COMBO, GetRemoteData(), false);
	m_HideBox32NPIIDCombo = BindNxDataListCtrl(this, IDC_HIDE_BOX_32_NPI_ID_COMBO, GetRemoteData(), false);

	pRow = m_Accepted12Combo->GetRow(-1);
	pRow->PutValue(0,_bstr_t("Never"));
	m_Accepted12Combo->AddRow(pRow);
	pRow = m_Accepted13Combo->GetRow(-1);
	pRow->PutValue(0,_bstr_t("If Accepted"));
	m_Accepted12Combo->AddRow(pRow);
	pRow = m_Accepted12Combo->GetRow(-1);
	pRow->PutValue(0,_bstr_t("If Not Accepted"));
	m_Accepted12Combo->AddRow(pRow);
	pRow = m_Accepted12Combo->GetRow(-1);
	pRow->PutValue(0,_bstr_t("Always"));
	m_Accepted12Combo->AddRow(pRow);

	pRow = m_Accepted13Combo->GetRow(-1);
	pRow->PutValue(0,_bstr_t("Never"));
	m_Accepted13Combo->AddRow(pRow);
	pRow = m_Accepted13Combo->GetRow(-1);
	pRow->PutValue(0,_bstr_t("If Accepted"));
	m_Accepted13Combo->AddRow(pRow);
	pRow = m_Accepted13Combo->GetRow(-1);
	pRow->PutValue(0,_bstr_t("If Not Accepted"));
	m_Accepted13Combo->AddRow(pRow);
	pRow = m_Accepted13Combo->GetRow(-1);
	pRow->PutValue(0,_bstr_t("Always"));
	m_Accepted13Combo->AddRow(pRow);

	// (j.jones 2007-05-11 14:17) - PLID 25932 - reworked Box 32 options
	pRow = m_HideBox32NameAddCombo->GetRow(-1);
	pRow->PutValue(0,(long)1);
	pRow->PutValue(1,_bstr_t("When POS Is 11"));
	m_HideBox32NameAddCombo->AddRow(pRow);
	pRow = m_HideBox32NameAddCombo->GetRow(-1);
	pRow->PutValue(0,(long)2);
	pRow->PutValue(1,_bstr_t("When POS Is Not 11"));
	m_HideBox32NameAddCombo->AddRow(pRow);
	pRow = m_HideBox32NameAddCombo->GetRow(-1);
	pRow->PutValue(0,(long)3);
	pRow->PutValue(1,_bstr_t("Always"));
	m_HideBox32NameAddCombo->AddRow(pRow);

	pRow = m_HideBox32NPIIDCombo->GetRow(-1);
	pRow->PutValue(0,(long)1);
	pRow->PutValue(1,_bstr_t("When POS Is 11"));
	m_HideBox32NPIIDCombo->AddRow(pRow);
	pRow = m_HideBox32NPIIDCombo->GetRow(-1);
	pRow->PutValue(0,(long)2);
	pRow->PutValue(1,_bstr_t("When POS Is Not 11"));
	m_HideBox32NPIIDCombo->AddRow(pRow);
	pRow = m_HideBox32NPIIDCombo->GetRow(-1);
	pRow->PutValue(0,(long)3);
	pRow->PutValue(1,_bstr_t("Always"));
	m_HideBox32NPIIDCombo->AddRow(pRow);

	CHCFASetupInfo HCFAInfo(m_nHCFASetupGroupID);

	//Box11D
	m_check11D.SetCheck(HCFAInfo.Box11D == 1);
	OnCheck11d();
	m_checkHideBox11DAlways.SetCheck(HCFAInfo.Box11D == 2);

	//HideBox4
	m_checkHideBox4.SetCheck(HCFAInfo.HideBox4);

	//HideBox7
	m_checkHideBox7.SetCheck(HCFAInfo.HideBox7);

	//HideBox9
	m_checkHideBox9.SetCheck(HCFAInfo.HideBox9);

	//HideBox11
	m_checkHideBox11.SetCheck(HCFAInfo.HideBox11);

	//HideBox30
	m_checkHideBox30.SetCheck(HCFAInfo.HideBox30);

	// (j.jones 2007-05-11 11:44) - PLID 25932 - revised Box 32 options
	
	//HideBox32NameAdd
	m_checkHideBox32NameAdd.SetCheck(HCFAInfo.HideBox32NameAdd > 0);
	m_HideBox32NameAddCombo->SetSelByColumn(0, HCFAInfo.HideBox32NameAdd);
	OnCheckHideBox32NameAdd();

	//HideBox32NPIID
	m_checkHideBox32NPIID.SetCheck(HCFAInfo.HideBox32NPIID > 0);
	m_HideBox32NPIIDCombo->SetSelByColumn(0, HCFAInfo.HideBox32NPIID);
	OnCheckHideBox32NpiId();

	//Box31Show
	m_31.SetCheck(HCFAInfo.Box31Show);

	//Box12Accepted
	m_Accepted12Combo->PutCurSel(HCFAInfo.Box12Accepted);

	//Box13Accepted
	m_Accepted13Combo->PutCurSel(HCFAInfo.Box13Accepted);

	//Address
	if(HCFAInfo.Address)
		m_checkLeftAlignInsAddress.ShowWindow(SW_SHOW);
	else
		m_checkLeftAlignInsAddress.ShowWindow(SW_HIDE);
	m_checkShowAddress.SetCheck(HCFAInfo.Address);

	//ShowPays
	m_checkShowApplies.SetCheck(HCFAInfo.ShowPays);

	//IgnoreAdjustments
	if(HCFAInfo.ShowPays) {
		m_checkExcludeAdjustments.ShowWindow(SW_SHOW);
		// (j.jones 2008-05-06 10:38) - PLID 29092 - don't show unless the "ShowPays" option is checked
		m_checkExcludeAdjustmentsFromBalance.ShowWindow(SW_SHOW);
		// (j.jones 2008-06-18 09:06) - PLID 30403 - added ability to not fill Box 29
		m_checkExcludeBox29.ShowWindow(SW_SHOW);
	}
	else {
		m_checkExcludeAdjustments.ShowWindow(SW_HIDE);
		// (j.jones 2008-05-06 10:37) - PLID 29092 - also hide the "exclude from balance" option
		m_checkExcludeAdjustmentsFromBalance.ShowWindow(SW_HIDE);
		// (j.jones 2008-06-18 09:06) - PLID 30403 - added ability to not fill Box 29
		m_checkExcludeBox29.ShowWindow(SW_HIDE);
	}

	m_checkExcludeAdjustments.SetCheck(HCFAInfo.IgnoreAdjustments);

	//ExcludeAdjFromBal
	m_checkExcludeAdjustmentsFromBalance.SetCheck(HCFAInfo.ExcludeAdjFromBal);

	// (j.jones 2008-06-18 09:06) - PLID 30403 - added ability to not fill Box 29
	//DoNotFillBox29
	m_checkExcludeBox29.SetCheck(HCFAInfo.DoNotFillBox29);
	
	//m_checkExcludeBox29 and m_checkExcludeAdjustments are mutually exclusive
	if(HCFAInfo.DoNotFillBox29) {
		m_checkExcludeBox29.EnableWindow(TRUE);
		m_checkExcludeAdjustments.EnableWindow(FALSE);
	}
	else if(HCFAInfo.IgnoreAdjustments) {
		m_checkExcludeBox29.EnableWindow(FALSE);
		m_checkExcludeAdjustments.EnableWindow(TRUE);
	}
	else {
		m_checkExcludeBox29.EnableWindow(TRUE);
		m_checkExcludeAdjustments.EnableWindow(TRUE);
	}

	//ShowDiagDecimals
	m_checkShowDiagDecimals.SetCheck(HCFAInfo.ShowDiagDecimals);

	//ShowICD9Desc
	m_checkShowDiagDesc.SetCheck(HCFAInfo.ShowICD9Desc);

	// (j.jones 2013-08-09 15:31) - PLID 57958 - this setting is obsolete in the new HCFA,
	// hide it unless any companies in this group are still using the old one
	if(!m_bAnyCompaniesOnOldHCFA) {
		m_checkShowDiagDesc.ShowWindow(SW_HIDE);
	}

	// (j.jones 2007-03-26 15:50) - PLID 24538 - I removed this ability, but by client demand reinstated it
	//LeftAlignInsAddress
	m_checkLeftAlignInsAddress.SetCheck(HCFAInfo.LeftAlignInsAddress);

	//ShowSecInsAdd
	m_checkShowSecInsAdd.SetCheck(HCFAInfo.ShowSecInsAdd);
	OnCheckShowSecondaryInsAdd();

	//ShowSecPINNumber
	m_checkShowSecondaryPINNumber.SetCheck(HCFAInfo.ShowSecPINNumber);

	//ShowSec24JNumber
	m_checkShowSecondary24JNumber.SetCheck(HCFAInfo.ShowSec24JNumber);

	//ShowAnesthesiaTimes
	m_checkShowAnesthTimes.SetCheck(HCFAInfo.ShowAnesthesiaTimes);
	OnCheckShowAnesthesiaTimes();

	//ShowRespName
	m_checkShowRespName.SetCheck(HCFAInfo.ShowRespName);

	//PrintPInBox1a
	m_checkPrintPInBox1a.SetCheck(HCFAInfo.PrintPinBox1a);

	//ShowAnesthMinutesOnly
	m_checkShowAnesthMinutesOnly.SetCheck(HCFAInfo.ShowAnesthMinutesOnly);

	//SecondaryFillBox11
	m_checkSecondaryFillBox11.SetCheck(HCFAInfo.SecondaryFillBox11);

	// (j.jones 2007-03-28 13:56) - PLID 25392 - added ShowWhichCodesCommas
	m_checkShowWhichCodesCommas.SetCheck(HCFAInfo.ShowWhichCodesCommas);

	// (j.jones 2007-04-09 16:22) - PLID 25537 - added HideBox11a/b/c
	m_checkHideBox11a_Birthdate.SetCheck(HCFAInfo.HideBox11a_Birthdate);
	m_checkHideBox11a_Gender.SetCheck(HCFAInfo.HideBox11a_Gender);
	m_checkHideBox11b.SetCheck(HCFAInfo.HideBox11b);
	m_checkHideBox11c.SetCheck(HCFAInfo.HideBox11c);

	// (j.jones 2007-04-09 17:52) - PLID 25539 - added Hide24ATo
	m_checkHide24ATo.SetCheck(HCFAInfo.Hide24ATo);

	// (j.jones 2007-07-12 14:18) - PLID 26636 - added ability to hide 24J NPI
	m_checkHideBox24JNPI.SetCheck(HCFAInfo.HideBox24JNPI);

	// (j.jones 2009-01-06 08:52) - PLID 32614 - added ability to hide 17b
	m_checkHideBox17b.SetCheck(HCFAInfo.HideBox17b);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHCFASetupDoNotFill::OnOK() 
{
	try {

		long Box11D = m_check11D.GetCheck();
		long Box11DAlways = m_checkHideBox11DAlways.GetCheck();

		long Box11DToSave = 0;
		if(Box11D)
			Box11DToSave = 1;
		if(Box11DAlways)
			Box11DToSave = 2;

		long HideBox4 = m_checkHideBox4.GetCheck();
		long HideBox7 = m_checkHideBox7.GetCheck();
		long HideBox9 = m_checkHideBox9.GetCheck();
		long HideBox11 = m_checkHideBox11.GetCheck();
		long HideBox30 = m_checkHideBox30.GetCheck();

		// (j.jones 2007-05-11 14:20) - PLID 25932 - save the new Box 32 settings
		long HideBox32NameAdd = 0;
		long HideBox32NPIID = 0;

		if(m_checkHideBox32NameAdd.GetCheck()) {

			if(m_HideBox32NameAddCombo->CurSel == -1) {
				AfxMessageBox("You must select an option in the Box 32 Name and Address dropdown.");
				return;
			}

			HideBox32NameAdd = VarLong(m_HideBox32NameAddCombo->GetValue(m_HideBox32NameAddCombo->CurSel, 0), 0);
		}
		if(m_checkHideBox32NPIID.GetCheck()) {

			if(m_HideBox32NPIIDCombo->CurSel == -1) {
				AfxMessageBox("You must select an option in the Box 32 NPI and 32b dropdown.");
				return;
			}

			HideBox32NPIID = VarLong(m_HideBox32NPIIDCombo->GetValue(m_HideBox32NPIIDCombo->CurSel, 0), 0);
		}

		// (j.jones 2010-07-23 10:44) - PLID 39794 - this now defaults to 3
		long Box12Accepted = 3;

		if(m_Accepted12Combo->GetCurSel()==3)
			Box12Accepted = 3;
		else if(m_Accepted12Combo->GetCurSel()==2)
			Box12Accepted = 2;
		else if(m_Accepted12Combo->GetCurSel()==1)
			Box12Accepted = 1;
		else
			Box12Accepted = 0;

		// (j.jones 2010-07-22 11:48) - PLID 39779 - this defaults to 3
		long Box13Accepted = 3;

		if(m_Accepted13Combo->GetCurSel()==3)
			Box13Accepted = 3;
		else if(m_Accepted13Combo->GetCurSel()==2)
			Box13Accepted = 2;
		else if(m_Accepted13Combo->GetCurSel()==1)
			Box13Accepted = 1;
		else
			Box13Accepted = 0;

		long Box31Show = m_31.GetCheck();

		long ShowAddress = m_checkShowAddress.GetCheck();

		long ShowPays = m_checkShowApplies.GetCheck();

		long ShowDiagDecimals = m_checkShowDiagDecimals.GetCheck();

		long ShowICD9Desc = m_checkShowDiagDesc.GetCheck();

		long LeftAlignInsAddress = m_checkLeftAlignInsAddress.GetCheck();

		// (j.jones 2008-06-18 09:05) - PLID 30403 - added DoNotFillBox29
		long DoNotFillBox29 = m_checkExcludeBox29.GetCheck() ? 1 : 0;

		long IgnoreAdjustments = m_checkExcludeAdjustments.GetCheck();

		long ExcludeAdjFromBal = m_checkExcludeAdjustmentsFromBalance.GetCheck();

		long ShowSecInsAdd = m_checkShowSecInsAdd.GetCheck();

		long ShowSecPINNumber = m_checkShowSecondaryPINNumber.GetCheck();

		long ShowSec24JNumber = m_checkShowSecondary24JNumber.GetCheck();

		long ShowAnesthesiaTimes = m_checkShowAnesthTimes.GetCheck();

		long ShowRespName = m_checkShowRespName.GetCheck() ? 1 : 0;

		long PrintPinBox1a = m_checkPrintPInBox1a.GetCheck() ? 1 : 0;

		long ShowAnesthMinutesOnly = m_checkShowAnesthMinutesOnly.GetCheck() ? 1 : 0;

		long SecondaryFillBox11 = m_checkSecondaryFillBox11.GetCheck() ? 1 : 0;

		// (j.jones 2007-03-28 13:56) - PLID 25392 - added ShowWhichCodesCommas
		long ShowWhichCodesCommas = m_checkShowWhichCodesCommas.GetCheck() ? 1 : 0;

		// (j.jones 2007-04-09 16:24) - PLID 25537 - added HideBox11a/b/c
		long HideBox11a_Birthdate = m_checkHideBox11a_Birthdate.GetCheck() ? 1 : 0;
		long HideBox11a_Gender = m_checkHideBox11a_Gender.GetCheck() ? 1 : 0;
		long HideBox11b = m_checkHideBox11b.GetCheck() ? 1 : 0;
		long HideBox11c = m_checkHideBox11c.GetCheck() ? 1 : 0;

		// (j.jones 2007-04-09 17:53) - PLID 25539 - added Hide24ATo
		long Hide24ATo = m_checkHide24ATo.GetCheck() ? 1 : 0;

		// (j.jones 2007-07-12 14:18) - PLID 26636 - added ability to hide 24J NPI
		long HideBox24JNPI = m_checkHideBox24JNPI.GetCheck() ? 1 : 0;

		// (j.jones 2009-01-06 08:52) - PLID 32614 - added ability to hide 17b
		long HideBox17b = m_checkHideBox17b.GetCheck() ? 1 : 0;

		// (j.jones 2006-09-14 10:13) - the Secondary24KNumber field name has not yet changed because we
		// have not yet changed the HCFA Image to support the new HCFA. So that is not a typo.

		// (j.jones 2008-06-18 09:05) - PLID 30403 - added DoNotFillBox29

		ExecuteSql("UPDATE HCFASetupT SET Box11D = %li, HideBox4 = %li, HideBox7 = %li, HideBox9 = %li, HideBox11 = %li, "
			"HideBox32NameAdd = %li, HideBox32NPIID = %li, HideBox30 = %li, "
			"Box12Accepted = %li, Box13Accepted = %li, Box31Show = %li, Address = %li, ShowPays = %li, "
			"ShowDiagDecimals = %li, ShowICD9Desc = %li, LeftAlignInsAddress = %li, IgnoreAdjustments = %li, "
			"ShowSecInsAdd = %li, ShowAnesthesiaTimes = %li, ShowRespName = %li, ShowSecPINNumber = %li, "
			"ShowSec24KNumber = %li, ExcludeAdjFromBal = %li, PrintPinBox1a = %li, ShowAnesthMinutesOnly = %li, "
			"SecondaryFillBox11 = %li, ShowWhichCodesCommas = %li, "
			"HideBox11a_Birthdate = %li, HideBox11a_Gender = %li, HideBox11b = %li, HideBox11c = %li, "
			"Hide24ATo = %li, HideBox24JNPI = %li, DoNotFillBox29 = %li, HideBox17b = %li "
			"WHERE ID = %li", 
			Box11DToSave, HideBox4, HideBox7, HideBox9, HideBox11, HideBox32NameAdd, HideBox32NPIID, HideBox30,
			Box12Accepted, Box13Accepted, Box31Show, ShowAddress, ShowPays, 
			ShowDiagDecimals, ShowICD9Desc, LeftAlignInsAddress, IgnoreAdjustments, ShowSecInsAdd, ShowAnesthesiaTimes, 
			ShowRespName, ShowSecPINNumber, ShowSec24JNumber, ExcludeAdjFromBal, PrintPinBox1a, ShowAnesthMinutesOnly, 
			SecondaryFillBox11, ShowWhichCodesCommas, HideBox11a_Birthdate, HideBox11a_Gender, HideBox11b, HideBox11c, 
			Hide24ATo, HideBox24JNPI, DoNotFillBox29, HideBox17b,
			m_nHCFASetupGroupID);

	}NxCatchAll("Error saving 'Do Not Fill' settings.");
	
	CDialog::OnOK();
}

void CHCFASetupDoNotFill::OnCheckShowApplies() 
{
	long ShowPays = m_checkShowApplies.GetCheck();

	if(ShowPays) {
		m_checkExcludeAdjustments.ShowWindow(SW_SHOW);
		// (j.jones 2008-05-06 10:38) - PLID 29092 - don't show unless the "ShowPays" option is checked
		m_checkExcludeAdjustmentsFromBalance.ShowWindow(SW_SHOW);
		// (j.jones 2008-06-18 09:06) - PLID 30403 - added ability to not fill Box 29
		m_checkExcludeBox29.ShowWindow(SW_SHOW);
	}
	else {
		m_checkExcludeAdjustments.ShowWindow(SW_HIDE);
		// (j.jones 2008-05-06 10:37) - PLID 29092 - also hide the "exclude from balance" option
		m_checkExcludeAdjustmentsFromBalance.ShowWindow(SW_HIDE);
		// (j.jones 2008-06-18 09:06) - PLID 30403 - added ability to not fill Box 29
		m_checkExcludeBox29.ShowWindow(SW_HIDE);
	}
}

void CHCFASetupDoNotFill::OnCheckShowInsAddress()
{
	long ShowAddress = m_checkShowAddress.GetCheck();

	if(ShowAddress)
		m_checkLeftAlignInsAddress.ShowWindow(SW_SHOW);
	else
		m_checkLeftAlignInsAddress.ShowWindow(SW_HIDE);
}

void CHCFASetupDoNotFill::OnCheckShowSecondaryInsAdd() 
{
	m_checkShowSecondaryPINNumber.EnableWindow(m_checkShowSecInsAdd.GetCheck());
	m_checkShowSecondary24JNumber.EnableWindow(m_checkShowSecInsAdd.GetCheck());	
}

void CHCFASetupDoNotFill::OnCheckExcludeAdjustments() 
{
	// (j.jones 2008-06-18 09:12) - PLID 30403 - now the exclude from balance
	// is always shown, but m_checkExcludeBox29 and m_checkExcludeAdjustments
	// are mutually exclusive
	if(m_checkExcludeBox29.GetCheck()) {
		m_checkExcludeBox29.EnableWindow(TRUE);
		m_checkExcludeAdjustments.EnableWindow(FALSE);
	}
	else if(m_checkExcludeAdjustments.GetCheck()) {
		m_checkExcludeBox29.EnableWindow(FALSE);
		m_checkExcludeAdjustments.EnableWindow(TRUE);
	}
	else {
		m_checkExcludeBox29.EnableWindow(TRUE);
		m_checkExcludeAdjustments.EnableWindow(TRUE);
	}
}

void CHCFASetupDoNotFill::OnCheckHideBox11dAlways() 
{
	if(m_checkHideBox11DAlways.GetCheck())
		m_check11D.SetCheck(FALSE);
}

void CHCFASetupDoNotFill::OnCheck11d() 
{
	if(m_check11D.GetCheck())
		m_checkHideBox11DAlways.SetCheck(FALSE);
}

void CHCFASetupDoNotFill::OnCheckShowAnesthesiaTimes() 
{
	long nShowTimes = m_checkShowAnesthTimes.GetCheck();

	if(nShowTimes)
		m_checkShowAnesthMinutesOnly.ShowWindow(SW_SHOW);
	else
		m_checkShowAnesthMinutesOnly.ShowWindow(SW_HIDE);
}

// (j.jones 2007-05-11 14:14) - PLID 25932 - reworked Box 32 options

void CHCFASetupDoNotFill::OnCheckHideBox32NameAdd() 
{
	m_HideBox32NameAddCombo->Enabled = m_checkHideBox32NameAdd.GetCheck();
	//auto-select "always"
	if(m_HideBox32NameAddCombo->CurSel == -1)
		m_HideBox32NameAddCombo->SetSelByColumn(0, (long)3);
}

void CHCFASetupDoNotFill::OnCheckHideBox32NpiId() 
{
	m_HideBox32NPIIDCombo->Enabled = m_checkHideBox32NPIID.GetCheck();	
	//auto-select "always"
	if(m_HideBox32NPIIDCombo->CurSel == -1)
		m_HideBox32NPIIDCombo->SetSelByColumn(0, (long)3);
}

// (j.jones 2008-06-18 09:06) - PLID 30403 - added ability to not fill Box 29
void CHCFASetupDoNotFill::OnCheckExcludeAllBox29() 
{
	//m_checkExcludeBox29 and m_checkExcludeAdjustments are mutually exclusive
	if(m_checkExcludeBox29.GetCheck()) {
		m_checkExcludeBox29.EnableWindow(TRUE);
		m_checkExcludeAdjustments.EnableWindow(FALSE);
	}
	else if(m_checkExcludeAdjustments.GetCheck()) {
		m_checkExcludeBox29.EnableWindow(FALSE);
		m_checkExcludeAdjustments.EnableWindow(TRUE);
	}
	else {
		m_checkExcludeBox29.EnableWindow(TRUE);
		m_checkExcludeAdjustments.EnableWindow(TRUE);
	}
}
