#if !defined(AFX_EDITLOGTIME_H__4B490F55_CC2A_4A60_8B1F_A13139ADF334__INCLUDED_)
#define AFX_EDITLOGTIME_H__4B490F55_CC2A_4A60_8B1F_A13139ADF334__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditLogTime.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// EditLogTime dialog

class CEditLogTime : public CNxDialog
{
// Construction
public:
	CEditLogTime(CWnd* pParent);   // standard constructor

	COleDateTime GetStart();
	COleDateTime GetEnd();
	void SetStart(COleDateTime dtStart);
	void SetEnd(COleDateTime dtEnd);

	// (j.gruber 2008-06-25 12:17) - PLID 26136 - added location
	void SetLocationID(long nLocationID);
	long GetLocationID();
	CString GetLocationName();
	

// Dialog Data
	//{{AFX_DATA(EditLogTime)
	enum { IDD = IDD_EDIT_LOG_TIME };
	CDateTimePicker	m_dtpStartDate;
	CDateTimePicker	m_dtpEndDate;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditLogTime)
	public:
	//TES 9/1/2010 - PLID 39233 - Changed to take bIsNew, rather than the window caption (which was just Add or Edit anyway)
	int DoModal(bool bIsNew);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	COleDateTime m_dtStart;
	COleDateTime m_dtEnd;
	NXTIMELib::_DNxTimePtr m_nxtStartTime;
	NXTIMELib::_DNxTimePtr m_nxtEndTime;
	CString m_strTitle;
	// (j.gruber 2008-06-25 12:17) - PLID 26136 - added location
	long m_nLocationID;
	CString m_strLocationName;
	NXDATALIST2Lib::_DNxDataListPtr  m_pLocationList;

	BOOL IsValid();
	void LoadStart(COleDateTime dtStart);
	void LoadEnd(COleDateTime dtEnd);

	//TES 9/8/2009 - PLID 26888 - We need to remember whether we were initially given a valid end time; if so, we need
	// to ensure that it is still valid when they say OK.
	bool m_bInitialEndTimeValid;

	//TES 9/1/2010 - PLID 39233 - We will also use whether it's a new entry or not in determining whether blank end times are allowed.
	bool m_bIsNew;

	// Generated message map functions
	//{{AFX_MSG(CEditLogTime)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedTimeLogLocation(short nFlags);
	afx_msg void OnSelChosenTimeLogLocation(LPDISPATCH lpRow);
	afx_msg void OnSelChangingTimeLogLocation(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITLOGTIME_H__4B490F55_CC2A_4A60_8B1F_A13139ADF334__INCLUDED_)
