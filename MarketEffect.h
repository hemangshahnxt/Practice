#include "marketutils.h"
#include "MarketingDlg.h"
#include "ReferralSubDlg.h"
#if !defined(AFX_EFFECTIVENESS_H__E1B0A29E_9126_11D2_8E52_00AA0064A698__INCLUDED_)
#define AFX_EFFECTIVENESS_H__E1B0A29E_9126_11D2_8E52_00AA0064A698__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketEffect.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEffectiveness dialog

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
class CEffectiveness : public CMarketingDlg
{
// Construction
public:
	CEffectiveness (CWnd* pParent);   // standard constructor
	~CEffectiveness();
	virtual void Refresh();

	NxButton	m_btnAllSources;
	NxButton	m_btnSelSources;

	void OnGoToPatient();
	bool IsRequerying(); // (a.walling 2006-08-08 17:39) - PLID 3897 - Is the tab finished loading?
	// (j.jones 2010-08-23 12:47) - PLID 39053 - require a connection pointer
	CString GetReportSQL(ADODB::_ConnectionPtr pCon); // generate the report sql
	CString GetGroupBy();	// this will be bill or patient
	CString GetReferralSource();	// this returns the referral source selected, or "All" if none
	void GetResults(CString &strExpense, CString &strReferrals, CString &strRefCost, CString &strPatients, CString &strPatCost, CString &strFeesBilled, CString &strFeesBilledPercent, CString &strFeesCollected, CString &strFeesCollectedPercent);
		// fills the params with appropriate values

	// (j.jones 2010-07-19 15:27) - PLID 39053 - require a connection pointer
	void GetParameters(CString &from, CString &to, CString &prov, CString &loc, CString &strPatCoord, CString &strDateField, CString &strLocationField, long &nCategory, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable);


	// (a.walling 2008-04-01 17:28) - PLID 29497 - Use CNxEdits to paint correctly
	// (a.walling 2008-04-10 13:01) - PLID 29497 - Changed to be CNxStatic
// Dialog Data
	//{{AFX_DATA(CEffectiveness)
	enum { IDD = IDD_MARKET_EFFECT };
	NxButton	m_btnGroupByBill;
	NxButton	m_btnGroupByPatient;
	CNxStatic	m_lblFeesCollected2;
	CNxStatic	m_lblFeesCollected;
	CNxStatic	m_lblFeesBilled2;
	CNxStatic	m_lblFeesBilled;
	CNxStatic	m_lblPatientCount;
	CNxStatic	m_lblPatientCost;
	CNxStatic	m_lblReferralCost;
	CNxStatic	m_lblReferralCount;
	CNxStatic	m_lblExpense;
	CNxColor	m_color1;
	CNxColor	m_color3;
	CNxStatic	m_nxstaticEffLabel9;
	NxButton	m_btnMktEffectReferralArea;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEffectiveness)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (j.jones 2010-08-19 15:25) - PLID 39493 - pass a connection in to all the Refresh sub-functions
	void RefreshCosts		(ADODB::_ConnectionPtr pCon, const CString &source, const CString &doc, const CString &loc, const CString &patCoord, const CString &from, const CString &to, const long &nCategory);
	void RefreshReferrals	(ADODB::_ConnectionPtr pCon, const CString &source, const CString &doc, const CString &loc, const CString &patCoord, const CString &from, const CString &to, const long &nCategory);
	void RefreshPatients	(); // (a.walling 2006-06-06 16:32) - PLID 20931 Does not require any recordsets or SQL, just m_cost (refreshcosts) and m_patient_count (datalist onrequery callback)
	void RefreshListBox		(ADODB::_ConnectionPtr pCon, const CString &source, const CString &doc, const CString &loc, const CString &patCoord, const CString &from, const CString &to, const long &nCategory);
	void RefreshCharges		(ADODB::_ConnectionPtr pCon, const CString &source, const CString &doc, const CString &loc, const CString &patCoord, const CString &from, const CString &to, const long &nCategory);
	void RefreshPayments	(ADODB::_ConnectionPtr pCon, const CString &source, const CString &doc, const CString &loc, const CString &patCoord, const CString &from, const CString &to, const long &nCategory);
	
	CString GetListBoxSql	(const CString &source, const CString &doc, const CString &loc, const CString &patCoord, const CString &from, const CString &to, const long &nCategory, bool bForceByPatient = false);
	void CEffectiveness::EnsureReferrals(BOOL bRefresh = TRUE);

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true); // a.walling 5/22/06 PLID 20695 Prevent NxDialog from refreshing all the time
	void UpdateFilters();				//<- Updates the allowed filters in the docbar
	void UpdateControls(IN bool bEnable);  //<- Enables or disables the dialog's controls
	
	// a.walling PLID 20695 5/25/06
	BOOL m_bActive;						//<- true if this is the active tab and not just switching from another tab
	bool m_bNeedsRefresh;			// true if we need to refresh everything
	bool m_bDataReady;				// true if data is ready to be read from the dialog

	// (a.walling 2006-08-08 18:02) - PLID 3897 - we will be grabbing the sql from the list itself.
	// (j.jones 2010-08-23 12:45) - PLID 39053 - this is no longer reliable
	//CString m_strSql;					// the datalist's sql query!

	NXDATALISTLib::_DNxDataListPtr   m_list;
	NXDATALISTLib::_DNxDataListPtr   m_BillList;

	CReferralSubDlg* m_pReferralSubDlg;

	// Generated message map functions
	//{{AFX_MSG(CEffectiveness)
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnAllReferralsCheck();
	afx_msg void OnSelReferralsCheck();
	afx_msg void OnRButtonDownNxdatalistctrl(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownListByBill(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnGroupByBill();
	afx_msg void OnGroupByPatient();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg LRESULT OnMarketReady(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRequeryFinishedPatient(short nFlags);
	afx_msg void OnRequeryFinishedBill(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	int				m_referral_count;
	int				m_patient_count;
	COleCurrency	m_cost;
	COleCurrency	m_total_billed;
	COleCurrency	m_total_collected;
};

///{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EFFECTIVENESS_H__E1B0A29E_9126_11D2_8E52_00AA0064A698__INCLUDED_)
