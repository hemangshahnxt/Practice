//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_WEEKDAYDLG_H__1D80CAD4_C7D0_11D1_804A_00104B2FE914__INCLUDED_)
#define AFX_WEEKDAYDLG_H__1D80CAD4_C7D0_11D1_804A_00104B2FE914__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// WeekDayDlg.h : header file
//

#define MAX_COLUMNS		21


#define MAKE_TWO_DIGIT_NUMBER_DECIMAL(nn)	((1##nn) - 100)

// (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
#define CASE_CWeekDayDlg__OnDayLabel(n_day_index)	\
	case IDC_DAY_LABEL_##n_day_index: \
		{ OnDayLabel(MAKE_TWO_DIGIT_NUMBER_DECIMAL(n_day_index)); } \
		break;

// (a.walling 2010-06-23 09:41) - PLID 39263 - CNxIconButtons now
// (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
#define DECLARE_CWeekDayDlg__DayLabels(n_day_index) \
	CNxLabel m_DayLabelBtn##n_day_index;

#define DDX_CWeekDayDlg__DayLabels(n_day_index) \
	DDX_Control(pDX, IDC_DAY_LABEL_##n_day_index, m_DayLabelBtn##n_day_index);

// (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
#define INIT_CWeekDayDlg__DayLabels(n_day_index) \
	m_DayLabelBtn##n_day_index.SetType(dtsClickableText); \
	m_DayLabelBtn##n_day_index.SetHzAlign(DT_CENTER); \
	m_DayLabelBtn##n_day_index.SetSingleLine(true);

// (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
#define CASE_CWeekDayDlg__OnSetCursor(n_day_index) \
	case IDC_DAY_LABEL_##n_day_index: \
		SetCursor(GetLinkCursor()); \
		return TRUE;

#define FILL_ARRAY_CWeekDayDlg__DayLabels(n_day_index) \
	m_pDayLabelBtn[MAKE_TWO_DIGIT_NUMBER_DECIMAL(n_day_index)] = &m_DayLabelBtn##n_day_index;


#define TENS(mac, n)		mac(n##0) mac(n##1) mac(n##2) mac(n##3) mac(n##4) mac(n##5) mac(n##6) mac(n##7) mac(n##8) mac(n##9)
#define DAY_LABEL_ARRAY(mac)		TENS(mac, 0) TENS(mac, 1) mac(20)

class CNxSchedulerDlg;

/////////////////////////////////////////////////////////////////////////////
// CWeekDayDlg dialog

class CWeekDayDlg : public CNxSchedulerDlg
{
// Construction
public:
	CWeekDayDlg(CWnd* pParent);   // standard constructor
	void EnsureButtonLabels();
	virtual int SetControlPositions();
	virtual bool ReFilter(CReservationReadSet &rsReadSet, bool bForceOpen = false);
	virtual COleDateTime GetWorkingDate(int nDay = 0);
	virtual BOOL OnInitDialog();
	virtual void PrePrint();
	virtual void PostPrint();
	virtual ADODB::_RecordsetPtr GetSingleApptQuery(long nResID);
	virtual BOOL SingleApptFitsSlot(long iDay, ADODB::_RecordsetPtr& prsAppt);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	virtual void EnsureCountLabelText();

	// (j.luckoski 2012-04-26 08:45) - PLID 11597 - Show cancelled appts
	long m_nDateRange;
	long m_nCancelledAppt;
	long m_nCancelColor;

	// (a.walling 2010-06-23 09:41) - PLID 39263 - Update all columns' available location info
	virtual void UpdateColumnAvailLocationsDisplay();
	
	// (z.manning, 02/13/2008) - PLID 28909 - Function to return the open recordset to load attendance appointments.
	virtual ADODB::_RecordsetPtr GetAttendanceResRecordsetFromBaseQuery(CString strBaseQuery);

// Dialog Data
	//{{AFX_DATA(CWeekDayDlg)
	enum { IDD = IDD_WEEK_DAY_DLG };
	CNxStatic	m_nxstaticActiveDateLabel;
	CNxStatic	m_nxstaticActiveDateAptcountLabel;
	CNxLabel	m_nxlabelActiveWeekAptcountLabel; // (c.haag 2009-12-22 13:44) - PLID 28977 - Now a CNxLabel
	//}}AFX_DATA

	// (a.walling 2010-06-23 09:41) - PLID 39263 - CNxIconButtons now
	// (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
	CNxLabel	*m_pDayLabelBtn[MAX_COLUMNS];
	// (a.walling 2008-09-30 17:56) - PLID 31445 - We now use actual static labels for the counts
	CArray <CNxStatic*, CNxStatic*> m_arDayCountLabels;
	// (a.walling 2008-10-01 11:02) - PLID 31445 - Destroy all the static labels
	void InvalidateDayCountLabels();

	CNxIconButton	m_btnWeekBack;
	CNxIconButton	m_btnWeekForward;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWeekDayDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

protected:
	virtual void Internal_GetWorkingResourceAndDateFromGUI(IN const long nColumnIndex, OUT long &nWorkingResourceID, OUT COleDateTime &dtWorkingDate) const;
	virtual BOOL Internal_IsMultiResourceSheet() const;
	virtual BOOL Internal_IsMultiDateSheet() const;

private:
	// (c.haag 2009-12-22 14:02) - PLID 28977 - The user can now click on the count label to
	// configure how counts are calculated
	void DoClickHyperlink(UINT nFlags, CPoint point);

private:
	void Print(CDC *pDC, CPrintInfo *pInfo);
protected:

	long m_nColumnCount;

	void ResizeLabels();

	void OnDayLabel(long nDayLabelIndex);

	DAY_LABEL_ARRAY(DECLARE_CWeekDayDlg__DayLabels)

	// Generated message map functions
	// (a.walling 2008-10-01 11:01) - PLID 31445 - Added WM_DESTROY handler
	//{{AFX_MSG(CWeekDayDlg)
	afx_msg void OnMoveDayBack();
	afx_msg void OnMoveDayForward();
	afx_msg void OnMoveWeekBack();
	afx_msg void OnMoveWeekForward();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	// (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
	afx_msg virtual LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam) override;

	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WEEKDAYDLG_H__1D80CAD4_C7D0_11D1_804A_00104B2FE914__INCLUDED_)
