#if !defined(AFX_ELIGIBILITYREVIEWDLG_H__4C4CBE9A_EACF_4804_908B_C7E66F6B40DD__INCLUDED_)
#define AFX_ELIGIBILITYREVIEWDLG_H__4C4CBE9A_EACF_4804_908B_C7E66F6B40DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EligibilityReviewDlg.h : header file
//

// (j.jones 2007-06-18 10:09) - PLID 26369 - created the EligibilityReviewDlg

#include "EEligibilityTabDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CEligibilityReviewDlg dialog
	
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker

class CEligibilityReviewDlg : public CNxDialog
{
// Construction
public:
	CEligibilityReviewDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_EligibilityList;

	// (j.jones 2008-05-07 15:40) - PLID 29854 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CEligibilityReviewDlg)
	enum { IDD = IDD_ELIGIBILITY_REVIEW_DLG };
	NxButton	m_btnCreateDate;
	NxButton	m_btnSentDate;
	NxButton	m_btnAllDates;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnRebatch;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEligibilityReviewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void Refilter();

	// (r.goldschmidt 2014-10-10 15:54) - PLID 62644 - Because now modeless, refresh EEligibility Tab only if it is the active sheet
	void RefreshEEligibilityTab();

	// Generated message map functions
	//{{AFX_MSG(CEligibilityReviewDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioEligCreateDate();
	afx_msg void OnRadioEligSentDate();
	afx_msg void OnRadioEligAllDates();
	afx_msg void OnChangeEligFromDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpEligFromDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeEligToDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpEligToDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClickCellEligibilityRequests(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnRButtonDownEligibilityRequests(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBtnRebatchSelectedElig();
	afx_msg void OnRequeryFinishedEligibilityRequests(short nFlags);
	afx_msg void OnReviewSelectedElig();
	// (j.jones 2010-03-26 08:52) - PLID 37712 - added ability to go to patient
	afx_msg void OnGoToPatient();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ELIGIBILITYREVIEWDLG_H__4C4CBE9A_EACF_4804_908B_C7E66F6B40DD__INCLUDED_)
