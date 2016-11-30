#if !defined(AFX_INSURANCEREFERRALSDLG_H__60599B11_7F7E_4C2D_A9E4_28383575629E__INCLUDED_)
#define AFX_INSURANCEREFERRALSDLG_H__60599B11_7F7E_4C2D_A9E4_28383575629E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InsuranceReferralsDlg.h : header file
//

#define ENTRY_ONLY	1
#define SELECT_AUTH 2

/////////////////////////////////////////////////////////////////////////////
// CInsuranceReferralsDlg dialog

class CInsuranceReferralsDlg : public CNxDialog
{
// Construction
public:
	CInsuranceReferralsDlg(CWnd* pParent);   // standard constructor
	CString GetReferralCPTString(long nID);
	CString GetReferralDiagString(long nID);

	NXDATALISTLib::_DNxDataListPtr m_ReferralList;

	long m_InsuredPartyID;
	long m_nSelectedInsuranceReferralID;
	CString m_strSelectedAuthNum;

	int m_iEntryType;

	void EditInsuranceReferral();

// Dialog Data
	//{{AFX_DATA(CInsuranceReferralsDlg)
	enum { IDD = IDD_INSURANCE_REFERRALS_DLG };
	NxButton	m_btnShowAvail;
	NxButton	m_btnShowUnavail;
	NxButton	m_btnShowAll;
	CNxIconButton	m_btnAddNewReferral;
	CNxIconButton	m_btnDeleteReferral;
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInsuranceReferralsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void RefreshButtons();

	// Generated message map functions
	//{{AFX_MSG(CInsuranceReferralsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddNewReferral();
	afx_msg void OnDeleteReferral();
	afx_msg void OnRadioShowAllReferrals();
	afx_msg void OnRadioShowAvailableReferrals();
	afx_msg void OnRadioShowUnavailableReferrals();
	afx_msg void OnDblClickCellInsuranceReferralList(long nRowIndex, short nColIndex);
	afx_msg void OnRequeryFinishedReferralList(short nFlags);
	virtual void OnOK();
	afx_msg void OnRButtonDownInsuranceReferralList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelChangedInsuranceReferralList(long nNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSURANCEREFERRALSDLG_H__60599B11_7F7E_4C2D_A9E4_28383575629E__INCLUDED_)
