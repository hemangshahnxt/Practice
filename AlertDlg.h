#if !defined(AFX_ALERTDLG_H__41C6D9A0_7F46_49B9_B646_DB2F7E08321B__INCLUDED_)
#define AFX_ALERTDLG_H__41C6D9A0_7F46_49B9_B646_DB2F7E08321B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AlertDlg.h : header file
//

#include <vector>

/////////////////////////////////////////////////////////////////////////////
// CAlertDlg dialog

// (a.walling 2010-10-11 17:13) - PLID 40731 - Use a structure to keep track of alerts
struct AlertItem
{
	AlertItem()
		: m_nAssociatedID(-1)
		, m_alertType(GenericAlert)
	{};

	AlertItem(const CString& strMessage, COleDateTime dt, long nAssociatedID = -1, EAlert alertType = GenericAlert)
		: m_strMessage(strMessage)
		, m_dt(dt)
		, m_nAssociatedID(nAssociatedID)
		, m_alertType(alertType)
	{};

	CString m_strMessage;
	COleDateTime m_dt;

	long m_nAssociatedID;
	EAlert m_alertType;
};

class CAlertDlg : public CNxDialog
{
// Construction
public:
	CAlertDlg(CWnd* pParent);   // standard constructor
	// (a.walling 2010-10-11 17:11) - PLID 40731 - Pass in the associated ID and alert type
	void SetAlertMessage(const char* szMsg, long nAssociatedID = -1, EAlert alertType = GenericAlert);
	long GetAlertCount();
	void Clear(); // (a.walling 2007-05-04 09:52) - PLID 4850 - Clear all alerts (called when switching users)

// Dialog Data
	//{{AFX_DATA(CAlertDlg)
	enum { IDD = IDD_ALERT_DLG };
	CNxIconButton	m_btnPrev;
	CNxIconButton	m_btnNext;
	CNxIconButton	m_btnLast;
	CNxIconButton	m_btnFirst;
	CNxIconButton	m_btnOK;
	CNxEdit	m_nxeditStaticAlert;
	NxButton	m_btnGroupbox;
	CNxLabel	m_nxlActionLabel; // (a.walling 2010-10-11 17:27) - PLID 40731
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlertDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (a.walling 2010-10-11 17:13) - PLID 40731 - Use a structure to keep track of alerts
	std::vector<AlertItem> m_arAlerts;
	DWORD m_dwVisibleAlert;

	void UpdateVisibleAlerts();

	// (z.manning, 05/16/2008) - PLID 30050 - Added OnCtlColor
	// Generated message map functions
	//{{AFX_MSG(CAlertDlg)
	afx_msg void OnBtnFirst();
	afx_msg void OnBtnPrev();
	afx_msg void OnBtnNext();
	afx_msg void OnBtnLast();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message); // (a.walling 2010-10-11 17:27) - PLID 40731
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam); // (a.walling 2010-09-13 14:35) - PLID 40505 - Support dosage units for vaccinations
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALERTDLG_H__41C6D9A0_7F46_49B9_B646_DB2F7E08321B__INCLUDED_)
