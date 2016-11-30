#if !defined(AFX_CASEHISTORYSELECTDETAILDLG_H__DB975E98_BE25_4883_B73D_9E1DE460F934__INCLUDED_)
#define AFX_CASEHISTORYSELECTDETAILDLG_H__DB975E98_BE25_4883_B73D_9E1DE460F934__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CaseHistorySelectDetailDlg.h : header file
//

enum ECaseHistoryDetailItemType;
/*
{
	chsdtProduct = -1,
	chsdtCptCode = -2,
	chsdtPerson = -3,
};
*/

/////////////////////////////////////////////////////////////////////////////
// CCaseHistorySelectDetailDlg dialog

class CCaseHistorySelectDetailDlg : public CNxDialog
{
// Construction
public:
	CCaseHistorySelectDetailDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCaseHistorySelectDetailDlg)
	enum { IDD = IDD_CASE_HISTORY_SELECT_DETAIL_DLG };
	// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
	NxButton	m_btnPreferenceCard;
	NxButton	m_btnPerson;
	NxButton	m_btnCpt;
	NxButton	m_btnProduct;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCaseHistorySelectDetailDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	long m_nID;
	CString m_strName;
	ECaseHistoryDetailItemType m_nType;
	double m_dblQuantity;
	COleCurrency m_cyAmount;
	COleCurrency m_cyCost;
	bool m_bBillable;
	// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
	//bool m_bPayToPractice;
	
	// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
	long m_nPreferenceCardID;

	// (j.jones 2008-05-20 17:52) - PLID 2949 - added field that tracks whether the user
	// selected a serialized product
	BOOL m_bIsSerializedProduct;
	//TES 7/16/2008 - PLID 27983 - Added field that tracks whether the user selected a CPT code that is linked to products.
	BOOL m_bIsLinkedToProducts;

	//TES 7/16/2008 - PLID 27983 - We show different CPT Codes depending on whether we are called from a completed case history.
	bool m_bCaseHistoryIsCompleted;

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_dlProductCombo;
	NXDATALISTLib::_DNxDataListPtr m_dlCptCodeCombo;
	NXDATALISTLib::_DNxDataListPtr m_dlPersonCombo;
	// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
	NXDATALISTLib::_DNxDataListPtr m_dlPreferenceCardCombo;

	// (j.jones 2011-06-15 10:55) - PLID 24507 - added InitializeCPTCombo
	void InitializeCPTCombo();

	// Generated message map functions
	//{{AFX_MSG(CCaseHistorySelectDetailDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCptcodeRadio();
	afx_msg void OnProductRadio();
	virtual void OnOK();
	afx_msg void OnPersonRadio();
	// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
	afx_msg void OnPreferenceCardRadio();
	afx_msg void OnRequeryFinishedPreferenceCardCombo(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2011-06-15 10:44) - PLID 24507 - added OnDestroy
	afx_msg void OnDestroy();
	// (j.jones 2011-06-15 10:47) - PLID 24507 - added OnBarcodeScan
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CASEHISTORYSELECTDETAILDLG_H__DB975E98_BE25_4883_B73D_9E1DE460F934__INCLUDED_)
