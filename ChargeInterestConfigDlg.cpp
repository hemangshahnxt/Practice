// ChargeInterestConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ChargeInterestConfigDlg.h"
#include "InternationalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CChargeInterestConfigDlg dialog


CChargeInterestConfigDlg::CChargeInterestConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CChargeInterestConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChargeInterestConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChargeInterestConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChargeInterestConfigDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_RADIO_USE_PERCENTAGE, m_btnUsePercent);
	DDX_Control(pDX, IDC_RADIO_USE_FLAT_FEE, m_btnUseFee);
	DDX_Control(pDX, IDC_RADIO_PER_CHARGE, m_radioFlatFeePerCharge);
	DDX_Control(pDX, IDC_RADIO_PER_PATIENT, m_radioFlatFeePerPatient);
	DDX_Control(pDX, IDC_RADIO_ALL_PATIENT_CHARGES, m_btnAllChgs);
	DDX_Control(pDX, IDC_RADIO_ONLY_PATIENT_CHARGES, m_btnPtChgs);
	DDX_Control(pDX, IDC_CHECK_INCREMENT_FINANCE_CHARGES, m_btnIncFin);
	DDX_Control(pDX, IDC_CHECK_INCLUDE_EXISTING_FINANCE_CHARGES, m_btnIncludeExist);
	DDX_Control(pDX, IDC_RADIO_AUTO_ADD_CHARGES, m_btnAutoAdd);
	DDX_Control(pDX, IDC_RADIO_PROMPT_USER, m_btnPromptUser);
	DDX_Control(pDX, IDC_RADIO_MANUAL_ONLY, m_btnManualOnly);
	DDX_Control(pDX, IDC_CHECK_CHOOSE_PATIENTS, m_btnChoosePts);
	DDX_Control(pDX, IDC_DAYS_UNTIL_OVERDUE, m_nxeditDaysUntilOverdue);
	DDX_Control(pDX, IDC_INTEREST_INTERVAL, m_nxeditInterestInterval);
	DDX_Control(pDX, IDC_INTEREST_PERCENTAGE, m_nxeditInterestPercentage);
	DDX_Control(pDX, IDC_FLAT_FEE_AMOUNT, m_nxeditFlatFeeAmount);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CALCULATE_AS_GROUPBOX, m_btnCalculateAsGroupbox);
	DDX_Control(pDX, IDC_CALCULATE_ON_GROUPBOX, m_btnCalculateOnGroupbox);
	DDX_Control(pDX, IDC_CALCULATE_WHEN_GROUPBOX, m_btnCalculateWhenGroupbox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChargeInterestConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CChargeInterestConfigDlg)
	ON_BN_CLICKED(IDC_RADIO_USE_PERCENTAGE, OnRadioUsePercentage)
	ON_BN_CLICKED(IDC_RADIO_USE_FLAT_FEE, OnRadioUseFlatFee)
	ON_BN_CLICKED(IDC_RADIO_AUTO_ADD_CHARGES, OnRadioAutoAddCharges)
	ON_BN_CLICKED(IDC_RADIO_PROMPT_USER, OnRadioPromptUser)
	ON_BN_CLICKED(IDC_RADIO_MANUAL_ONLY, OnRadioManualOnly)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChargeInterestConfigDlg message handlers

BOOL CChargeInterestConfigDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/30/2008) - PLID 29850 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	// (j.jones 2009-06-10 11:19) - PLID 33834 - added caching
	g_propManager.CachePropertiesInBulk("CChargeInterestConfigDlg-1", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'FCChargeDaysUntilOverdue' OR "
			"Name = 'FCChargeInterestInterval' OR "
			"Name = 'FCChargeInterestFeeType' OR "
			"Name = 'FCChargesToCalculate' OR "
			"Name = 'FCIncrementExistingFinanceCharges' OR "
			"Name = 'FCCompoundExistingFinanceCharges' OR "
			"Name = 'FCPromptApplyFinanceCharges' OR "
			"Name = 'FCPromptUserID' OR "
			"Name = 'FCChooseInvididualPatients' OR "
			"Name = 'FCChargeInterestFlatFeeApplyType' "
			")",
			_Q(GetCurrentUserName()));

	g_propManager.CachePropertiesInBulk("CChargeInterestConfigDlg-2", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'FCChargeInterestRate' OR "
			"Name = 'FCChargeFlatFee' "
			")",
			_Q(GetCurrentUserName()));
	
	m_UserList = BindNxDataListCtrl(this,IDC_PROMPT_USER_COMBO,GetRemoteData(),true);

	SetDlgItemInt(IDC_DAYS_UNTIL_OVERDUE,GetRemotePropertyInt("FCChargeDaysUntilOverdue",30,0,"<None>",true));
	SetDlgItemInt(IDC_INTEREST_INTERVAL,GetRemotePropertyInt("FCChargeInterestInterval",30,0,"<None>",true));

	long nFeeType = GetRemotePropertyInt("FCChargeInterestFeeType",0,0,"<None>",true);
	if(nFeeType == 0)
		CheckDlgButton(IDC_RADIO_USE_PERCENTAGE,TRUE);
	else
		CheckDlgButton(IDC_RADIO_USE_FLAT_FEE,TRUE);

	OnChangeFeeType();

	// (j.jones 2009-06-10 11:21) - PLID 33834 - added flat fee apply types
	long nFlatFeeApplyType = GetRemotePropertyInt("FCChargeInterestFlatFeeApplyType",0,0,"<None>",true);
	if(nFlatFeeApplyType == 0) {
		CheckDlgButton(IDC_RADIO_PER_CHARGE, TRUE);
	}
	else {
		CheckDlgButton(IDC_RADIO_PER_PATIENT,TRUE);
	}

	CString strInterestRate = GetRemotePropertyText("FCChargeInterestRate","1.0",0,"<None>",true);	
	double dblInterestRate = atof(strInterestRate);
	dblInterestRate = (dblInterestRate - 1) * 100;
	strInterestRate.Format("%0.06f", dblInterestRate);

	//JMJ - having lots of zeros at the end is silly to me so lets trim the zeros
	//strInterestRate.TrimRight("%");
	strInterestRate.TrimRight("0");
	strInterestRate.TrimRight(".");
	//strInterestRate += "%";
	//now the strInterestRate is showing up until the rightmost zero

	SetDlgItemText (IDC_INTEREST_PERCENTAGE, strInterestRate);

	SetDlgItemText(IDC_FLAT_FEE_AMOUNT,GetRemotePropertyText("FCChargeFlatFee","$0.00",0,"<None>",true));

	long nChargeType = GetRemotePropertyInt("FCChargesToCalculate",0,0,"<None>",true);
	if(nChargeType == 0)
		CheckDlgButton(IDC_RADIO_ALL_PATIENT_CHARGES,TRUE);
	else
		CheckDlgButton(IDC_RADIO_ONLY_PATIENT_CHARGES,TRUE);

	CheckDlgButton(IDC_CHECK_INCREMENT_FINANCE_CHARGES,GetRemotePropertyInt("FCIncrementExistingFinanceCharges",0,0,"<None>",true) == 1);
	CheckDlgButton(IDC_CHECK_INCLUDE_EXISTING_FINANCE_CHARGES,GetRemotePropertyInt("FCCompoundExistingFinanceCharges",0,0,"<None>",true) == 1);

	long nPromptType = GetRemotePropertyInt("FCPromptApplyFinanceCharges",2,0,"<None>",true);
	if(nPromptType == 0)
		CheckDlgButton(IDC_RADIO_AUTO_ADD_CHARGES,TRUE);
	else if(nPromptType == 1)
		CheckDlgButton(IDC_RADIO_PROMPT_USER,TRUE);
	else
		CheckDlgButton(IDC_RADIO_MANUAL_ONLY,TRUE);
	
	OnChangePromptType();

	IRowSettingsPtr pRow = m_UserList->GetRow(-1);
	pRow->PutValue(0,(long)-1);
	pRow->PutValue(1,_bstr_t("{Any User}"));
	m_UserList->AddRow(pRow);

	long nUserID = GetRemotePropertyInt("FCPromptUserID",-1,0,"<None>",true);

	m_UserList->SetSelByColumn(0,(long)nUserID);

	CheckDlgButton(IDC_CHECK_CHOOSE_PATIENTS,GetRemotePropertyInt("FCChooseInvididualPatients",0,0,"<None>",true) == 1);
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChargeInterestConfigDlg::OnOK() 
{
	try {

		//get days until overdue
		long nDaysUntilOverdue = GetDlgItemInt(IDC_DAYS_UNTIL_OVERDUE);

		if(nDaysUntilOverdue == 0) {
			AfxMessageBox("The days until the charge is overdue cannot be zero. Please correct this before saving.");
			return;
		}

		//get charge interest interval
		long nChargeInterestInterval = GetDlgItemInt(IDC_INTEREST_INTERVAL);

		if(nDaysUntilOverdue == 0) {
			if(MessageBox("The interest interval is zero days. If you leave it as zero, interest will only be added one time when the bill has become overdue.\n\n"
				"Would you like you continue with this value?","Practice",MB_ICONQUESTION|MB_YESNO) == IDNO) {
				return;
			}
		}

		//get fee type
		long nFeeType = 0;
		if(IsDlgButtonChecked(IDC_RADIO_USE_FLAT_FEE))
			nFeeType = 1;

		// (j.jones 2009-06-10 11:21) - PLID 33834 - added flat fee apply types
		long nFlatFeeApplyType = 0;
		if(IsDlgButtonChecked(IDC_RADIO_PER_PATIENT)) {
			nFlatFeeApplyType = 1;
		}

		//get interest rate
		CString strInterestRate = "0";
		GetDlgItemText (IDC_INTEREST_PERCENTAGE, strInterestRate);
		strInterestRate = strInterestRate.SpanIncluding("1234567890.");
		double dblInterestRate = ((atof(strInterestRate.GetBuffer(strInterestRate.GetLength())) / 100.0) + 1.0);
		strInterestRate.Format("%0.06f", dblInterestRate);

		//get flat fee
		CString strFlatFee = "$0.00";
		GetDlgItemText(IDC_FLAT_FEE_AMOUNT,strFlatFee);
		COleCurrency cyFlatFee;
		if(!cyFlatFee.ParseCurrency(strFlatFee) || cyFlatFee.m_status == COleCurrency::invalid) {
			if(nFeeType == 1) {
				AfxMessageBox("Invalid amount entered for the 'Flat Fee' field. Please correct before saving.");
				return;
			}
			else {
				//if they aren't using this feature, just save $0.00 rather than yell at them
				cyFlatFee = COleCurrency(0,0);
			}
		}
		strFlatFee = FormatCurrencyForInterface(cyFlatFee,TRUE,TRUE);		

		//get charges to be calculated
		long nChargeType = 0;
		if(IsDlgButtonChecked(IDC_RADIO_ONLY_PATIENT_CHARGES))
			nChargeType = 1;

		//get prompt type
		long nPromptType = 0;
		if(IsDlgButtonChecked(IDC_RADIO_PROMPT_USER))
			nPromptType = 1;
		else if(IsDlgButtonChecked(IDC_RADIO_MANUAL_ONLY))
			nPromptType = 2;

		//get user ID
		long nUserID = -1;
		if(m_UserList->CurSel != -1)
			nUserID = m_UserList->GetValue(m_UserList->GetCurSel(),0).lVal;

		//////////////////////////////////////
		//now save all of these settings

		//save days until overdue
		SetRemotePropertyInt("FCChargeDaysUntilOverdue",nDaysUntilOverdue,0,"<None>");

		//save charge interest interval
		SetRemotePropertyInt("FCChargeInterestInterval",nChargeInterestInterval,0,"<None>");

		//save fee type
		SetRemotePropertyInt("FCChargeInterestFeeType",nFeeType,0,"<None>");

		// (j.jones 2009-06-10 11:21) - PLID 33834 - added flat fee apply types
		SetRemotePropertyInt("FCChargeInterestFlatFeeApplyType", nFlatFeeApplyType, 0, "<None>");

		//save interest rate
		SetRemotePropertyText("FCChargeInterestRate",strInterestRate,0,"<None>");

		//save flat fee
		SetRemotePropertyText("FCChargeFlatFee",strFlatFee,0,"<None>");

		//save charges to be calculated
		SetRemotePropertyInt("FCChargesToCalculate",nChargeType,0,"<None>");

		//save finance charge option
		SetRemotePropertyInt("FCIncrementExistingFinanceCharges",IsDlgButtonChecked(IDC_CHECK_INCREMENT_FINANCE_CHARGES) ? 1 : 0,0,"<None>");

		//save compound charge option
		SetRemotePropertyInt("FCCompoundExistingFinanceCharges",IsDlgButtonChecked(IDC_CHECK_INCLUDE_EXISTING_FINANCE_CHARGES) ? 1 : 0,0,"<None>");

		//save prompt type
		SetRemotePropertyInt("FCPromptApplyFinanceCharges",nPromptType,0,"<None>");

		//save user ID
		SetRemotePropertyInt("FCPromptUserID",nUserID,0,"<None>");

		//save invididual patient option
		SetRemotePropertyInt("FCChooseInvididualPatients",IsDlgButtonChecked(IDC_CHECK_CHOOSE_PATIENTS) ? 1 : 0,0,"<None>");
	
		CDialog::OnOK();

	}NxCatchAll("Error saving charge interest configuration.");
}

void CChargeInterestConfigDlg::OnRadioUsePercentage() 
{
	OnChangeFeeType();
}

void CChargeInterestConfigDlg::OnRadioUseFlatFee() 
{
	OnChangeFeeType();
}

void CChargeInterestConfigDlg::OnChangeFeeType()
{
	if(IsDlgButtonChecked(IDC_RADIO_USE_PERCENTAGE)) {
		GetDlgItem(IDC_INTEREST_PERCENTAGE)->EnableWindow(TRUE);
		GetDlgItem(IDC_FLAT_FEE_AMOUNT)->EnableWindow(FALSE);

		// (j.jones 2009-06-10 11:21) - PLID 33834 - added flat fee apply types
		GetDlgItem(IDC_RADIO_PER_CHARGE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_PER_PATIENT)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_INTEREST_PERCENTAGE)->EnableWindow(FALSE);
		GetDlgItem(IDC_FLAT_FEE_AMOUNT)->EnableWindow(TRUE);

		// (j.jones 2009-06-10 11:21) - PLID 33834 - added flat fee apply types
		GetDlgItem(IDC_RADIO_PER_CHARGE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_PER_PATIENT)->EnableWindow(TRUE);
	}
}

void CChargeInterestConfigDlg::OnRadioAutoAddCharges() 
{
	OnChangePromptType();
}

void CChargeInterestConfigDlg::OnRadioPromptUser() 
{
	OnChangePromptType();
}

void CChargeInterestConfigDlg::OnRadioManualOnly() 
{
	OnChangePromptType();
}

void CChargeInterestConfigDlg::OnChangePromptType()
{
	if(IsDlgButtonChecked(IDC_RADIO_PROMPT_USER)) {
		GetDlgItem(IDC_PROMPT_USER_COMBO)->EnableWindow(TRUE);
	}
	else {
		GetDlgItem(IDC_PROMPT_USER_COMBO)->EnableWindow(FALSE);
	}
}
