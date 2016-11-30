#if !defined(AFX_REWARDPOINTSSETUPDLG_H__35FC5A32_564A_40D4_AFC6_571C679E6652__INCLUDED_)
#define AFX_REWARDPOINTSSETUPDLG_H__35FC5A32_564A_40D4_AFC6_571C679E6652__INCLUDED_

// (a.walling 2007-05-17 09:13) - PLID 20838 - Dialog to set up reward points.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RewardPointsSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRewardPointsSetupDlg dialog

enum EActionColumns {
	eacID = 0,
	eacName
};

// SAVED TO DATA - only append
enum ERewardActions {
	eraInvalid = -1,	// Invalid
	eraBill = 0,		// When saving a new bill...
	eraCharge,			// When saving a new charge...
	eraRefPatient,		// When referring a patient...
	eraRefBill			// When a referred patient has a new bill saved...
};

class CRewardPointsSetupDlg : public CNxDialog
{
// Construction
public:
	CRewardPointsSetupDlg(CWnd* pParent);   // standard constructor
	
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

// Dialog Data
	//{{AFX_DATA(CRewardPointsSetupDlg)
	enum { IDD = IDD_REWARD_POINTS_SETUP_DLG };
	NxButton	m_btnAccumNoPoints;
	CNxIconButton	m_btnRedeemRewards;
	CNxIconButton	m_btnSetupPointsDetail;
	CNxIconButton	m_btnResetRewardPoints; // (a.wilson 2013-02-22 11:27) - PLID 50591
	CNxEdit	m_nxeditEditPointsPerBill;
	CNxEdit	m_nxeditEditPointsPerDollar;
	CNxEdit	m_nxeditEditPointsPerPoint;
	CNxEdit	m_nxeditEditDollarsForEvery;
	CNxEdit	m_nxeditEditBillPointsForEvery;
	CNxEdit	m_nxeditEditDollarsAccumulate;
	CNxEdit	m_nxeditEditBillPointsAccumulate;
	CNxStatic	m_nxstaticPointsPerPoint;
	CNxStatic	m_nxstaticRewardPointsTitle;
	CNxStatic	m_nxstaticPointsPerBill;
	CNxStatic	m_nxstaticPointsPerDollar;
	CNxStatic	m_nxstaticDollarsForEvery;
	CNxStatic	m_nxstaticDollarsDollars;
	CNxStatic	m_nxstaticDollarsAccumulate;
	CNxStatic	m_nxstaticDollarsPoints;
	CNxStatic	m_nxstaticBillPointsForEvery;
	CNxStatic	m_nxstaticBillPointsBillPoints;
	CNxStatic	m_nxstaticBillPointsAccumulate;
	CNxStatic	m_nxstaticBillPointsPoints;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRewardPointsSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_dlAction;

	void HandleSelChanged(long nOldSel, long nNewSel);

	BOOL SaveChanges(long nCurSel);
	bool m_bModified;

	void ShowAllControls(BOOL bShow);
	void ShowSetupPointValues(BOOL bShow);
	void ShowPointsPerPoints(BOOL bShow);
	void ShowPointsPerDollar(BOOL bShow);
	void ShowPointsPer(CString strText, BOOL bShow);

	void SaveValue(const CString &strName, long nVal);

	ERewardActions m_nCurAction;

	CBrush m_brush;
	CArray<unsigned long, unsigned long> m_arControlIDs;
	CArray<unsigned long, unsigned long> m_arEditControlIDs;

	void UpdatePointsPer();
	void UpdateDollarsPer();

	void LoadValues(long nAction);
	void LoadValue(const CString &strName, long nID);

	// (a.wetta 2007-05-24 17:01) - PLID 25392 - Keep track of when we're loading.  If this is positive, then we are loading;
	// if it's 0, then we're not.  This number basically indicates how many functions are loading on the dialog.
	int m_nLoading;

	// Generated message map functions
	//{{AFX_MSG(CRewardPointsSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangingRewardActionList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangedRewardActionList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChangeEditDollarsForEvery();
	afx_msg void OnChangeEditDollarsAccumulate();
	afx_msg void OnChangeEditBillPointsForEvery();
	afx_msg void OnChangeEditBillPointsAccumulate();
	afx_msg void OnDestroy();
	afx_msg void OnKillfocusEditBillPointsForEvery();
	afx_msg void OnKillfocusEditBillPointsAccumulate();
	afx_msg void OnKillfocusEditDollarsAccumulate();
	afx_msg void OnKillfocusEditDollarsForEvery();
	afx_msg void OnKillfocusEditPointsPerBill();
	afx_msg void OnCheckAccumulateWhenNoPoints();
	afx_msg void OnChangeEditPointsPerBill();
	afx_msg void OnSetupPointsDetail();
	afx_msg void OnRedeemRewards();
	afx_msg void OnBnClickedResetRewardPointsBtn(); // (a.wilson 2013-02-22 11:28) - PLID 50591
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REWARDPOINTSSETUPDLG_H__35FC5A32_564A_40D4_AFC6_571C679E6652__INCLUDED_)
