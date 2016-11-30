#pragma once

// CERemitZeroPaysDlg dialog

#include "FinancialRc.h"

// (j.jones 2011-03-21 10:44) - PLID 42099 - created

enum ERemitZeroPays_ReturnValues
{
	erzprv_Skip = 0,
	erzprv_Post,
	erzprv_Cancel,
};

class CERemitZeroPaysDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CERemitZeroPaysDlg)

public:
	CERemitZeroPaysDlg(CWnd* pParent);   // standard constructor
	virtual ~CERemitZeroPaysDlg();

	virtual ERemitZeroPays_ReturnValues DoModal(CString strPatientName, CString strServiceCode, COleDateTime dtChargeDate,
		COleCurrency cyChargeAmount, COleCurrency cyAdjustmentTotal, CString strAdjustmentReasons);

// Dialog Data
	enum { IDD = IDD_EREMIT_ZERO_PAYS_DLG };
	CNxIconButton	m_btnSkip;
	CNxIconButton	m_btnPost;
	CNxIconButton	m_btnCancel;
	CNxStatic		m_nxstaticPatientName;
	CNxStatic		m_nxstaticChargeInfo;
	CNxStatic		m_nxstaticAdjustment;
	CNxStatic		m_nxstaticAdjReasons;

protected:

	CString m_strPatientName;
	CString m_strServiceCode;
	COleDateTime m_dtChargeDate;
	COleCurrency m_cyChargeAmount;
	COleCurrency m_cyAdjustmentTotal;
	CString m_strAdjustmentReasons;

	ERemitZeroPays_ReturnValues m_erzprvReturnValue;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBtnSkipCharge();
	afx_msg void OnBtnPostAdjustments();
	afx_msg void OnBtnCancel();
};
