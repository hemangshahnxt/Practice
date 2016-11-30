#if !defined(AFX_FINANCIALAPPLY_H__CC27B2C1_3DBE_11D2_AB75_00A0246CDDA1__INCLUDED_)
#define AFX_FINANCIALAPPLY_H__CC27B2C1_3DBE_11D2_AB75_00A0246CDDA1__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// FinancialApply.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CFinancialApply dialog

class CFinancialApply : public CNxDialog
{
	void OnOK();
// Construction
public:

	CFont *m_pFont;
	COleCurrency m_cyNetPayment;
	COleCurrency m_cyNetCharges;
	COleCurrency m_cyApplyAmount;
	long m_PatientID;
	long m_nResponsibility; // 0 = patient, -1 = inactive, otherwise the RespTypeID
	long m_nShiftToResp; // 0 = patient, otherwise the RespTypeID
	BOOL m_boShiftBalance;
	BOOL m_boAdjustBalance;
	BOOL m_boIncreaseInsBalance;
	BOOL m_boShowIncreaseCheck;
	BOOL m_boShowAdjCheck;
	BOOL m_boIsAdjustment;
	BOOL m_boApplyToPay;
	BOOL m_boZeroAmountAllowed;

	void GetNonNegativeAmountExtent(int &nStart, int &nFinish);

	NXDATALISTLib::_DNxDataListPtr m_RespCombo;

	CFinancialApply(CWnd* pParent);   // standard constructor
	~CFinancialApply();	 // standard destructor

// Dialog Data
	// (a.walling 2008-04-02 16:22) - PLID 29497 - Use NxStatic
	//{{AFX_DATA(CFinancialApply)
	enum { IDD = IDD_FINANCIAL_APPLY_DLG };
	CNxStatic	m_labelAvailApply;
	CNxStatic	m_labelNetCharges;
	NxButton	m_shiftCheck;
	NxButton	m_increaseCheck;
	NxButton	m_adjustCheck;
	CNxEdit	m_nxeditEditPayment;
	CNxStatic	m_nxstaticLabelResponsibility;
	CNxStatic	m_nxstaticLabel4;
	CNxStatic	m_nxstaticLabel3;
	CNxStatic	m_nxstaticCurrencySymbolApply;
	CNxIconButton	m_btnApply;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnCalcPercent;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFinancialApply)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CBrush m_brush;
	// Generated message map functions
	//{{AFX_MSG(CFinancialApply)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBtnApply();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckAdjust();
	afx_msg void OnCheckIncrease();
	afx_msg void OnCheckShift();
	afx_msg void OnCalcPercent();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINANCIALAPPLY_H__CC27B2C1_3DBE_11D2_AB75_00A0246CDDA1__INCLUDED_)
