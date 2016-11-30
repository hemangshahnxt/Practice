#if !defined(AFX_APTBOOKALARMDLG_H__8A467505_25AC_43C1_8BF3_3831A383A488__INCLUDED_)
#define AFX_APTBOOKALARMDLG_H__8A467505_25AC_43C1_8BF3_3831A383A488__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AptBookAlarmDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAptBookAlarmDlg dialog

class CAptBookAlarmDlg : public CNxDialog
{
// Construction
public:
	CAptBookAlarmDlg(CWnd* pParent);   // standard constructor

	long m_ID;
	BOOL m_bIsNew;

	void Load();

	BOOL Save();

	NXDATALISTLib::_DNxDataListPtr m_AlarmDetailList;

	void PostEditDetail(long nRow, long nAptTypeID, long nAptPurposeID);

	// (z.manning, 04/30/2008) - PLID 29845 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CAptBookAlarmDlg)
	enum { IDD = IDD_APT_BOOK_ALARM_DLG };
	NxButton	m_checkIncludeNoShows;
	NxButton	m_checkAllowSave;
	NxButton	m_checkSameType;
	NxButton	m_checkSamePurpose;
	NxButton	m_checkSameResource;
	NxButton	m_checkAptAfter;
	NxButton	m_checkAptBefore;
	CNxEdit	m_nxeditEditAlarmDescription;
	CNxStatic	m_nxstaticLabelDaysBefore;
	CNxStatic	m_nxstaticLabelDaysAfter;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnAddAptAlarmDetail;
	CNxIconButton	m_btnEditAptAlarmDetail;
	CNxIconButton	m_btnRemoveAptAlarmDetail;
	NxButton	m_btnWarnOptionsGroupbox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAptBookAlarmDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void DoClickHyperlink(UINT nFlags, CPoint point);

	CRect m_rcDaysBeforeLabel, m_rcDaysAfterLabel;
	CRect m_rcDaysBeforeNumber, m_rcDaysAfterNumber;

	void DrawBeforeLabel(CDC *pdc);
	void DrawAfterLabel(CDC *pdc);

	long m_nDaysBefore;
	long m_nDaysAfter;

	void RefreshButtons();

	// Generated message map functions
	//{{AFX_MSG(CAptBookAlarmDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnCheckAptBefore();
	afx_msg void OnCheckAptAfter();
	afx_msg void OnDestroy();
	virtual void OnOK();
	afx_msg void OnAddAptAlarmDetailBtn();
	afx_msg void OnEditAptAlarmDetailBtn();
	afx_msg void OnRemoveAptAlarmDetailBtn();
	afx_msg void OnDblClickCellAptBookAlarmDetailList(long nRowIndex, short nColIndex);
	virtual void OnCancel();
	afx_msg void OnSelChangedAptBookAlarmDetailList(long nNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APTBOOKALARMDLG_H__8A467505_25AC_43C1_8BF3_3831A383A488__INCLUDED_)
