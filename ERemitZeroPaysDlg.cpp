// ERemitZeroPaysDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ERemitZeroPaysDlg.h"
#include "InternationalUtils.h"

// CERemitZeroPaysDlg dialog

// (j.jones 2011-03-21 10:44) - PLID 42099 - created

IMPLEMENT_DYNAMIC(CERemitZeroPaysDlg, CNxDialog)

CERemitZeroPaysDlg::CERemitZeroPaysDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CERemitZeroPaysDlg::IDD, pParent)
{
	m_erzprvReturnValue = erzprv_Cancel;
}

CERemitZeroPaysDlg::~CERemitZeroPaysDlg()
{
}

void CERemitZeroPaysDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_SKIP_CHARGE, m_btnSkip);
	DDX_Control(pDX, IDC_POST_ADJUSTMENTS, m_btnPost);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EOB_PATIENT_NAME_LABEL, m_nxstaticPatientName);
	DDX_Control(pDX, IDC_CHARGE_INFO_LABEL, m_nxstaticChargeInfo);
	DDX_Control(pDX, IDC_ZERO_PAY_ADJ_LABEL, m_nxstaticAdjustment);
	DDX_Control(pDX, IDC_ADJ_REASON_LABEL, m_nxstaticAdjReasons);
}

ERemitZeroPays_ReturnValues CERemitZeroPaysDlg::DoModal(CString strPatientName, CString strServiceCode, COleDateTime dtChargeDate,
														COleCurrency cyChargeAmount, COleCurrency cyAdjustmentTotal,
														CString strAdjustmentReasons)
{
	try {
	
		m_strPatientName = strPatientName;
		m_strServiceCode = strServiceCode;
		m_dtChargeDate = dtChargeDate;
		m_cyChargeAmount = cyChargeAmount;
		m_cyAdjustmentTotal = cyAdjustmentTotal;
		m_strAdjustmentReasons = strAdjustmentReasons;

		CNxDialog::DoModal();

		return m_erzprvReturnValue;

	}NxCatchAll("Error in CBillingExtraChargeInfoDlg::DoModal");

	return erzprv_Cancel;
}

BEGIN_MESSAGE_MAP(CERemitZeroPaysDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_SKIP_CHARGE, OnBtnSkipCharge)
	ON_BN_CLICKED(IDC_POST_ADJUSTMENTS, OnBtnPostAdjustments)
	ON_BN_CLICKED(IDCANCEL, OnBtnCancel)
END_MESSAGE_MAP()

// CERemitZeroPaysDlg message handlers

BOOL CERemitZeroPaysDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnSkip.AutoSet(NXB_OK);
		m_btnPost.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_nxstaticPatientName.SetWindowText(m_strPatientName);

		CString strChargeInfo;
		strChargeInfo.Format("Code: %s, Date Of Service: %s, Charge Amount: %s", m_strServiceCode,
			FormatDateTimeForInterface(m_dtChargeDate, NULL, dtoDate), FormatCurrencyForInterface(m_cyChargeAmount));
		m_nxstaticChargeInfo.SetWindowText(strChargeInfo);

		CString strAdjInfo;
		strAdjInfo.Format("The EOB states that a %s adjustment should be applied with the reason:",
			FormatCurrencyForInterface(m_cyAdjustmentTotal));
		m_nxstaticAdjustment.SetWindowText(strAdjInfo);

		m_nxstaticAdjReasons.SetWindowText(m_strAdjustmentReasons);

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CERemitZeroPaysDlg::OnBtnSkipCharge()
{
	try {

		m_erzprvReturnValue = erzprv_Skip;

		CNxDialog::OnOK();

	} NxCatchAll(__FUNCTION__);
}

void CERemitZeroPaysDlg::OnBtnPostAdjustments()
{
	try {

		m_erzprvReturnValue = erzprv_Post;

		CNxDialog::OnOK();

	} NxCatchAll(__FUNCTION__);
}

void CERemitZeroPaysDlg::OnBtnCancel()
{
	try {

		m_erzprvReturnValue = erzprv_Cancel;

		CNxDialog::OnCancel();

	} NxCatchAll(__FUNCTION__);
}
