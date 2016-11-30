#if !defined(AFX_INSURANCEREFERRALSSELECTDLG_H__E41B45C8_AD32_4B2F_87F5_49B6F826B303__INCLUDED_)
#define AFX_INSURANCEREFERRALSSELECTDLG_H__E41B45C8_AD32_4B2F_87F5_49B6F826B303__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InsuranceReferralsSelectDlg.h : header file
//
//

/////////////////////////////////////////////////////////////////////////////
// CInsuranceReferralsSelectDlg dialog

class CInsuranceReferralsSelectDlg : public CNxDialog
{
public:
	enum EDefaultInsuredParty{
		dipNoDefaultGiven = -1
	};

// Construction
public:
	CInsuranceReferralsSelectDlg(CWnd* pParent);   // standard constructor
	CInsuranceReferralsSelectDlg(CWnd* pParent, bool bAppointmentMenu); // (b.spivey, March 22, 2012) - PLID 47435 - overload constructor. 
	CString GetReferralCPTString(long nID);
	CString GetReferralDiagString(long nID);

	NXDATALISTLib::_DNxDataListPtr m_ReferralList;
	NXDATALISTLib::_DNxDataListPtr m_InsCoList;

	long m_InsuredPartyID;
	long m_nPatientID; // (a.walling 2008-07-07 17:34) - PLID 29900 - Store the patient id
	long m_nSelectedInsuranceReferralID;
	bool m_bIsNewBill;
	bool m_bUseDateFilter;
	COleDateTime m_dtSelectedStartDate;
	COleDateTime m_dtSelectedEndDate;
	long m_nSelectedLocationID;

	// (b.spivey, February 20, 2012) - PLID 47435 - If this is an appointment menu, we have specific options for this. 
	bool m_bIsAppointmentMenu; 

	COleDateTime m_dtFilterDate;
	CString m_strSelectedAuthNum;

	void EditInsuranceReferral();
	
// Dialog Data
	//{{AFX_DATA(CInsuranceReferralsSelectDlg)
	enum { IDD = IDD_INSURANCE_REFERRALS_SELECT_DLG };
	CDateTimePicker	m_date;
	CNxIconButton	m_btnUseNewReferral;
	CNxIconButton	m_btnProceedNewReferral;
	CNxIconButton	m_btnProceedSelectedReferral;
	CNxIconButton	m_btnUseSelectedReferral;
	CNxIconButton	m_btnProceedNoReferral;
	CNxIconButton	m_btnCancelBill;
	CNxIconButton	m_btnCancelSelection;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInsuranceReferralsSelectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnCancel();
	virtual void OnOK();

	//}}AFX_VIRTUAL

// Implementation
protected:
	void InitInsCoList(IN const long nSetSelTo);
	void Refresh();
	void ReflectCurrentStateOnBtns();
	bool ValidateFilterDate(IN const long &nCurSel);
	bool ValidateInsuredPartyID(IN const long &nCurSel);
	bool ValidateLocationID(IN const long &nCurSel);
	bool ValidateSelectedReferralAgainstFilters(IN const long &nCurSel);
	CBrush m_brush;
		
	// Generated message map functions
	//{{AFX_MSG(CInsuranceReferralsSelectDlg)
	afx_msg void OnOK_ProceedWithNewReferral();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenInsCoList(long nRow);
	afx_msg void OnRequeryFinishedInsuranceReferralList(short nFlags);
	virtual void OnCancel_CancelBill();
	afx_msg void OnOK_ProceedNoReferral();
	afx_msg void OnOK_ProceedWithSelectedReferral();
	afx_msg void OnSelChangedInsuranceReferralList(long nNewSel);
	afx_msg void OnCancel_CancelSelection();
	afx_msg void OnOK_UseNewReferral();
	afx_msg void OnOK_UseSelectedReferral();
	afx_msg void OnChangeDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTrySetSelFinishedReferralList(long nRowEnum, long nFlags);
	afx_msg void OnDblClickCellInsuranceReferralList(long nRowIndex, short nColIndex);
	afx_msg void OnRButtonDownInsuranceReferralList(long nRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSURANCEREFERRALSSELECTDLG_H__E41B45C8_AD32_4B2F_87F5_49B6F826B303__INCLUDED_)
