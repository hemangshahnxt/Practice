#if !defined(AFX_SEARCHNOTESDLG_H__A8B762F2_F84F_4072_8084_6FA2E139E2D7__INCLUDED_)
#define AFX_SEARCHNOTESDLG_H__A8B762F2_F84F_4072_8084_6FA2E139E2D7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SearchNotesDlg.h : header file
//

#include <afxdtctl.h>

/////////////////////////////////////////////////////////////////////////////
// CSearchNotesDlg dialog

class CSearchNotesDlg : public CNxDialog
{
// Construction
public:
	void ClearSearchArray();
	void BuildSearchArray(CString strSearchString);
	CString ParseCriteriaToSql(CString strSearchString);

	void OnChangeRadioDate();

	CStringArray m_arySearchParts;

	long nPersonID;

	void GoToPatient(long PatientID);

	NXDATALISTLib::_DNxDataListPtr m_ResultsList;

	CSearchNotesDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSearchNotesDlg)
	enum { IDD = IDD_SEARCH_NOTES_DLG };
	CDateTimePicker	m_dateFrom;
	CDateTimePicker	m_dateTo;
	CNxEdit	m_nxeditSearchCriteria;
	CNxIconButton m_btnOK;
	NxButton	m_radioSearchAllDates;
	NxButton	m_radioSearchDateRange;
	CNxIconButton m_btnFind;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSearchNotesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSearchNotesDlg)
	afx_msg void OnBtnFind();
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDownResultsList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownResultsList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRadioAllDates();
	afx_msg void OnRadioDateRange();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEARCHNOTESDLG_H__A8B762F2_F84F_4072_8084_6FA2E139E2D7__INCLUDED_)
