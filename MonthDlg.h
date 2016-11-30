//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_MONTHDLG_H__83743083_DC5F_11D1_9F47_0040051BB763__INCLUDED_)
#define AFX_MONTHDLG_H__83743083_DC5F_11D1_9F47_0040051BB763__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MonthDlg.h : header file
//

#include "NxSchedulerDlg.h"

#define BEGIN_SEMAPHORE(a)	{ bool bTemp = a; a = true; try {
#define END_SEMAPHORE(a)	} catch (CException *e) { e->ReportError(); e->Delete(); } a = bTemp;}
#define DECLARE_SEMAPHORE(a) bool a;
#define INIT_SEMAPHORE(a) a = false;
#define CHECK_SAMAPHORE(a) if (a) 

/////////////////////////////////////////////////////////////////////////////
// CMonthDlg dialog

class CMonthDlg : public CNxSchedulerDlg
{
	typedef struct {
		int iFirstDay, iLastDay;
		COLORREF clr;
		int height;	// 0 = bottom of day
		CString strName;
		long nResID; // The reservation ID

		int x,y;	// Position in month dialog
	} _MONTHEVENT;

	// Each day has an array of drag and drop region structures
	// that are used to tell what appointment the user clicked on
	// for a drag and drop operation
	typedef struct {
		int top;
		int bottom;
		int nNameIndex;
	} _DRAGDROPRGN;

// Construction
public:
	void StoreDetails();
	void RecallDetails();

	// (j.luckoski 2012-04-26 08:44) - PLID 11597 - For showing cancelled appts
	long m_nDateRange;
	long m_nCancelledAppt;
	long m_nCancelColor;

	//void FindFirstAvailableAppt();//(s.dhole 6/1/2015 4:18 PM ) - PLID 65645  Remove 
	CMonthDlg(CWnd* pParent);   // standard constructor
	void EnsureButtonLabels();
	virtual int SetControlPositions();
	virtual COleDateTime GetWorkingDate(int nDay = 0);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	long DrawPageHeader(CDC *pDC, LPRECT rcPage, long nSetId);
	long DrawDayHeader(CDC *pDC, long x, long y, long nBoxWidth, long nBoxHeight, LPCTSTR strDayHdr, BOOL bDrawBorder);
	CString GetPrintText(const CString &strApptText);
	virtual void PrePrint();
	virtual void PostPrint();
	void Print(CDC * pDC, CPrintInfo *pInfo);
	virtual void UpdateViewBySingleAppt(long nResID);	

	// (a.walling 2010-06-23 09:41) - PLID 39263 - Update all columns' available location info
	virtual void UpdateColumnAvailLocationsDisplay();

	// (a.walling 2010-06-23 18:00) - PLID 39263 - Now a virtual function
	virtual void UpdateActiveDateLabel();
	
	//TES 12/17/2008 - PLID 32497 - Need to do this slightly differently on this tab, for Scheduler Standard clients.
	virtual void UpdateBlocks(bool bUpdateTemplateBlocks, bool bForceUpdate, bool bLoadAllTemplateInfoAtOnce);
	virtual void UpdateBlocks(bool bUpdateTemplateBlocks, bool bForceUpdate);

	//TES 12/18/2008 - PLID 32497 - Override this function to keep our controls disabled if we're in Scheduler Standard mode.
	virtual void EnableSingleDayControls();

protected:
	// (c.haag 2008-06-18 17:14) - PLID 26135 - Creates text for a template record from
	// PrepareTemplatePrint()
	CString BuildTemplateElement(ADODB::FieldsPtr& f);

public:
	// (c.haag 2008-06-18 16:22) - PLID 26135 - This function is called before a print
	// or print preview where the user wants to print template names in the month view.
	// It will run a query to fill in m_astrVisibleTemplateNames and 
	// m_adwVisibleTemplateColors.
	void PrepareTemplatePrint();

protected:
// Dialog Data
	//{{AFX_DATA(CMonthDlg)
	enum { IDD = IDD_MONTH_DLG };
	CNxIconButton	m_monthBackButton;
	CNxIconButton	m_monthForwardButton;
	CNxIconButton	m_wk5Button;
	CNxIconButton	m_wk4Button;
	CNxIconButton	m_wk3Button;
	CNxIconButton	m_wk2Button;
	CNxIconButton	m_wk1Button;
	CNxIconButton	m_wk0Button;
	CNxStatic	m_nxstaticActiveDateLabel;
	CNxStatic	m_nxstaticMon;
	CNxStatic	m_nxstaticTue;
	CNxStatic	m_nxstaticWed;
	CNxStatic	m_nxstaticThu;
	CNxStatic	m_nxstaticFri;
	CNxStatic	m_nxstaticSat;
	CNxStatic	m_nxstaticEditActiveSingledayDay;
	CNxLabel	m_nxlabelActiveDateAptcountLabel; // (c.haag 2009-12-22 13:44) - PLID 28977 - Now a CNxLabel
	CNxStatic	m_nxstaticSun;
	NxButton	m_btnFirstDayFrame;
	//}}AFX_DATA

	//CButton	* m_pDayInMonthBtn[35];

	CPoint m_ptMousePos;
	CPoint m_ptPriorMousePos;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMonthDlg)
	protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation

protected:
	virtual void Internal_GetWorkingResourceAndDateFromGUI(IN const long nColumnIndex, OUT long &nWorkingResourceID, OUT COleDateTime &dtWorkingDate) const;
	virtual BOOL Internal_IsMultiResourceSheet() const;
	virtual BOOL Internal_IsMultiDateSheet() const;

protected:
	DECLARE_SEMAPHORE(m_bStoring);
	int m_startx ;
	int m_starty ;
	int m_daywidth ;
	int m_dayheight ;
	int m_nVisibleNamesPerDay ;
	int m_nVisibleNamesOffset ;
	CString m_strDragInfo;
	COleDateTime m_dtSingleDayDate;


	int m_nWeekCnt ;
	COleDateTime m_dtFirstDayOfMonth;
	COleDateTime m_dtLastUpdate;

	// The font that will be used for each appointment in the little day boxes in the month drawing
	CFont m_fntAppointment;
	// The height of the text in that font
	long m_nHeightAppointmentFont;

	// Generated message map functions
	//{{AFX_MSG(CMonthDlg)
	afx_msg void OnMoveMonthBack();
	afx_msg void OnMoveMonthForward();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSpinDay(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSpinAllDays(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnWeekLabel0();
	afx_msg void OnWeekLabel1();
	afx_msg void OnWeekLabel2();
	afx_msg void OnWeekLabel3();
	afx_msg void OnWeekLabel4();
	afx_msg void OnWeekLabel5();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL DoesDayPrintOutputFit(CDC *pDC, CStringArray *pastrNames, long nBoxWidth, long nBoxHeight);
	BOOL m_bAllowScrollDown;

	//BOOL m_IsShowingFirstAvail;//(s.dhole 6/1/2015 4:18 PM ) - PLID 65645  Remove 
	int m_iDragMonth;
	COLORREF m_clrDragColor;
	CString m_strDragName;
	COleDateTime GetCalendarDateUnderMouse(int &sx, int &sy);
	void UpdateSingleDayCtrl();
	CString BuildCalendarElement(ADODB::FieldPtr& fldResID,
								ADODB::FieldPtr& fldStartTime, ADODB::FieldPtr& fldEndTime,
								ADODB::FieldPtr& fldPatientID, ADODB::FieldPtr& fldFirstName,
								ADODB::FieldPtr& fldLastName, ADODB::FieldPtr& fldAptTypeID,
								ADODB::FieldPtr& fldAptTypeName, ADODB::FieldPtr& fldAptTypeColor,
								ADODB::FieldPtr& fldAptPurposeName);
	void BuildCalendarEvent(_MONTHEVENT& event, ADODB::FieldPtr& fldResID,
										ADODB::FieldPtr& fldStartTime, ADODB::FieldPtr& fldEndTime,
										ADODB::FieldPtr& fldEventName, ADODB::FieldPtr& fldAptTypeColor);

	CStringArray m_astrVisibleNames[31];
	CDWordArray m_adwVisibleColors[31];

	// (c.haag 2008-06-18 16:23) - PLID 26135 - Visible names for template printing
	CStringArray m_astrVisibleTemplateNames[31];
	CDWordArray m_adwVisibleTemplateColors[31];

	//TES 1/5/2012 - PLID 47310 - m_ResourceAvailLocations only stores information for the active day.  This stores information for every
	// day in the month, in order to highlight days on the calendar when the selected resource is at the filtered location.
	CResourceAvailLocations m_ResourceAvailLocations_Month;

	CArray<_MONTHEVENT, _MONTHEVENT> m_aEvents;
	CArray<_DRAGDROPRGN, _DRAGDROPRGN> m_aDragDropRgn[31];
	CString GetWhereClause();
	void UpdateCalendarNames();
	void UpdatePurposes();
	int GetEventCountInDay(int nDay);
	void GetEventsInDay(int nDay, CArray<_MONTHEVENT, _MONTHEVENT>& aEvents);
	COleDateTime m_dtPriorDate;

private:
	// (c.haag 2009-12-22 14:02) - PLID 28977 - The user can now click on the count label to
	// configure how counts are calculated
	void DoClickHyperlink(UINT nFlags, CPoint point);

protected:
	void PrintDay(COleDateTime date, CRect rDayRect, CDC *pDC, bool bBlackAndWhite, bool bPrintMonthName, bool bIsActiveDay);
	// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - strAppt should be const
	long PrintAppt(CDC *pDC, const CString &strAppt, CRect rAppt, BOOL bCalcRect = FALSE);

protected:
	void PaintDay(CDC& dc, const CRect& rcBounds, const COleDateTime& dt);
	void PaintDayBackground(CDC& dc, const CRect& rcBounds, const COleDateTime& dt);
	void PaintDayHeader(CDC& dc, const CRect& rcBounds, const COleDateTime& dt);
	void PaintDayAppointments(CDC& dc, const CRect& rcBounds, const COleDateTime& dt);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MONTHDLG_H__83743083_DC5F_11D1_9F47_0040051BB763__INCLUDED_)
