#if !defined(AFX_HISTORYFILTERDLG_H__C9887820_95C2_4477_922F_C69C3992DF82__INCLUDED_)
#define AFX_HISTORYFILTERDLG_H__C9887820_95C2_4477_922F_C69C3992DF82__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HistoryFilterDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHistoryFilterDlg dialog

class CHistoryFilterDlg : public CNxDialog
{
// Construction
public:
	void EnsureFilters();
	CHistoryFilterDlg(CWnd* pParent);   // standard constructor
	COleDateTime GetDateTo() { return m_dtDateTo; }
	COleDateTime GetDateFrom() { return m_dtDateFrom; }
	bool UseDates() { return m_bUseDates; }
	CString GetLocationName() { return m_strLocationName; }

// Dialog Data
	//{{AFX_DATA(CHistoryFilterDlg)
	enum { IDD = IDD_PRINT_HISTORY_FILTER_DLG };
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	NxButton m_checkPrintHeader;
	NxButton m_radioAllDatesFilter;
	NxButton m_radioFilterDates;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHistoryFilterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pLocations;
	COleDateTime m_dtDateFrom, m_dtDateTo;
	bool m_bUseDates;
	CString m_strLocationName;

	// Generated message map functions
	//{{AFX_MSG(CHistoryFilterDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnAllDatesFilter();
	afx_msg void OnFilterDates();
	afx_msg void OnPrintHeader();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HISTORYFILTERDLG_H__C9887820_95C2_4477_922F_C69C3992DF82__INCLUDED_)
