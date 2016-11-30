#if !defined(AFX_ONEDAYDLG_H__8D6FB32B_B8E2_11D1_B2DD_00001B4B970B__INCLUDED_)
#define AFX_ONEDAYDLG_H__8D6FB32B_B8E2_11D1_B2DD_00001B4B970B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CSchedulerView;

// OneDayDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COneDayDlg dialog

class CNxSchedulerDlg;

class COneDayDlg : public CNxSchedulerDlg
{
// Construction
public:
	COneDayDlg(CWnd* pParent);   // standard constructor

	// (a.walling 2015-01-22 16:08) - PLID 64662 - Rescheduling Queue - selecting specific appointments
	void ShowReschedulingQueueEmbedded(bool bShow, long nApptID = -1); // (a.walling 2015-01-12 10:28) - PLID 64558 - Rescheduling Queue - Dock and embed

	virtual int SetControlPositions();
	virtual BOOL OnInitDialog();
	virtual void PrePrint();
	virtual void PostPrint();

	// (j.luckoski 2012-04-26 08:51) - PLID 11597 - Show cancelled appts.
	long m_nDateRange;
	long m_nCancelledAppt;
	long m_nCancelColor;

// Dialog Data
	//{{AFX_DATA(COneDayDlg)
	enum { IDD = IDD_ONE_DAY_DLG };
	CNxStatic	m_nxstaticActiveDateLabel;
	CNxLabel	m_nxlabelActiveDateAptcountLabel; // (c.haag 2009-12-22 13:44) - PLID 28977 - Now a CNxLabel
	//}}AFX_DATA

	// (a.walling 2010-06-23 09:41) - PLID 39263 - Update all columns' available location info
	virtual void UpdateColumnAvailLocationsDisplay();

	// (a.walling 2010-06-23 18:00) - PLID 39263 - Now a virtual function
	virtual void UpdateActiveDateLabel();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COneDayDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
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
public:

	std::unique_ptr<class CReschedulingQueue> m_pReschedulingQ;

protected:
	// Generated message map functions
	//{{AFX_MSG(COneDayDlg)
	afx_msg void OnMove(int x, int y);
	afx_msg void OnMoving(UINT nSide, LPRECT lpRect);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ONEDAYDLG_H__8D6FB32B_B8E2_11D1_B2DD_00001B4B970B__INCLUDED_)
