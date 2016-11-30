#if !defined(AFX_CASEHISTORIESDLG_H__22986F8F_CF83_4911_833E_B405785E6EA4__INCLUDED_)
#define AFX_CASEHISTORIESDLG_H__22986F8F_CF83_4911_833E_B405785E6EA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CaseHistoriesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCaseHistoriesDlg dialog

class CCaseHistoriesDlg : public CNxDialog
{
// Construction
public:
	CCaseHistoriesDlg(CWnd* pParent);   // standard constructor

	void ChangeFilterType();
	void OnClosedCaseHistory(long nCaseHistoryID);

	void OnGoToPatient();
	void OnGoToAppointment();

// Dialog Data	
	// (a.walling 2008-05-28 11:28) - PLID 27591 - Use CDateTimePicker
	//{{AFX_DATA(CCaseHistoriesDlg)
	enum { IDD = IDD_CASE_HISTORIES_DLG };
	NxButton	m_radioAllDates;
	NxButton	m_radioDateRange;
	NxButton	m_radioCompleted;
	NxButton	m_radioIncomplete;
	NxButton	m_radioAllCompleted;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	// (j.jones 2009-08-07 09:55) - PLID 35140 - added billed filters
	NxButton	m_radioUnbilled;
	NxButton	m_radioBilled;
	NxButton	m_radioAllBilled;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCaseHistoriesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_dlCaseHistoryList;

protected:
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	// (j.jones 2009-08-07 11:26) - PLID 24077 - added ability to link to an existing bill
	void OnLinkWithExistingBill();

	// (a.walling 2008-05-14 12:37) - PLID 27591 - Use Notify handlers for DateTimePicker
	// Generated message map functions
	//{{AFX_MSG(CCaseHistoriesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblClickCellCaseHistoryList(long nRowIndex, short nColIndex);
	afx_msg void OnRadioCompleted();
	afx_msg void OnRadioIncomplete();
	afx_msg void OnRadioAllCompleted();
	afx_msg void OnRequeryFinishedCaseHistoryList(short nFlags);
	afx_msg void OnRadioCaseHistoryAllDates();
	afx_msg void OnRadioCaseHistoryDateRange();
	afx_msg void OnChangeDtCaseHistoryFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpDtCaseHistoryFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeDtCaseHistoryTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpDtCaseHistoryTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPrintOne();
	afx_msg void OnRButtonDownCaseHistoryList(long nRow, short nCol, long x, long y, long nFlags);
	// (j.jones 2009-08-07 10:18) - PLID 35140 - added billed filters
	afx_msg void OnRadioUnbilled();
	afx_msg void OnRadioBilled();
	afx_msg void OnRadioAllBilled();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CASEHISTORIESDLG_H__22986F8F_CF83_4911_833E_B405785E6EA4__INCLUDED_)
