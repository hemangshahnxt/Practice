#if !defined(AFX_CONTACTBAR_H__D1C25B83_015A_11D3_9447_00C04F4C8415__INCLUDED_)
#define AFX_CONTACTBAR_H__D1C25B83_015A_11D3_9447_00C04F4C8415__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif
// ContactBar.h : header file
//

#define IDC_MAIN_SEARCH		187
#define IDC_REFERING_SEARCH 188
#define IDC_EMPLOYEE_SEARCH 189
#define IDC_OTHER_SEARCH	190
#define IDC_CONTACTS_SEARCH 191
#define IDC_SUPPLIER_SEARCH 192
#define IDC_ACTIVE_CONTACTS	193
#define IDC_ALL_CONTACTS	194

/////////////////////////////////////////////////////////////////////////////
// CContactBar window

class CContactBar : public CToolBar
{
// Construction
public:
	CContactBar();
	CWnd m_wndContactsCombo;
	NXDATALISTLib::_DNxDataListPtr m_toolBarCombo;	
	long GetActiveContact();
	CString GetActiveContactName();
	CString GetExistingContactName(long PersonID);
	// (c.haag 2009-05-07 15:58) - PLID 28561 - This function returns a username given a valid UserID
	CString GetExistingUserName(long PersonID);
	long GetActiveContactStatus();
	long GetExistingContactStatus(long PersonID);
	void SetExistingContactStatus(long nPersonID, long nStatusID);
	long m_ActiveContact;

	void SetActiveContactID(long nNewContactID);
	void ChangeContact(long nID);
	void Requery();

	// (a.walling 2007-05-04 09:53) - PLID 4850 - Called when the user has been changed, refresh any user-specific settings
	void OnUserChanged();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CContactBar)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL CreateComboBox();
	virtual ~CContactBar();
	
	bool SetComboWhereClause(const CString &strNewSQL, bool bForce = false );
	CString GetComboWhereClause();
	// (a.walling 2007-11-12 10:40) - PLID 28062 - Transparent CStatic and CButton classes for toolbar dialog controls
	// Generated message map functions
	// (a.walling 2008-04-22 12:53) - PLID 29642 - Use NxButton/CNxStatic
	NxButton m_main, 
			m_referring, 
			m_employee, 
			m_other, 
			m_supplier, 
			m_all,
			m_hideInactiveContacts,
			m_allContacts;
	CNxStatic m_lblBlank;

protected:
	// (a.walling 2007-11-12 10:40) - PLID 28062 - Transparent CStatic and CButton classes for toolbar dialog controls
	// (a.walling 2008-04-22 12:53) - PLID 29642 - Use NxButton/CNxStatic
	CNxStatic m_searchText;
	CFont m_searchButtonFont;
	
	// (a.walling 2008-08-20 15:39) - PLID 29642 - Handle themes
	UXTheme &m_uxtTheme;
	BOOL m_bThemeInit;

	// (a.walling 2008-08-20 15:39) - PLID 29642 - Last mouse over button index for hottracking
	long m_nLastMouseOverIx;
	BOOL m_bTrackingMouse;

	// (a.walling 2007-11-12 10:40) - PLID 28062 - Added CtlColor handler
	// (a.walling 2008-08-20 15:40) - PLID 29642 - Added CustomDraw and EraseBkgnd handlers,
	// and also OnMouseMove and OnMouseLeave for custom hottracking
	// (a.walling 2008-08-21 14:09) - PLID 31056 - Added OnLButtonUp handler to fix drawing issue with tooltips
	// (a.walling 2008-10-02 17:23) - PLID 31575 - Border drawing (NcPaint)
	//{{AFX_MSG(CContactBar)
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnSelChosenCombo(long nRow);
	afx_msg void OnRequeryFinishedCombo(short nFlags);
	afx_msg void OnPullUpCombo();
	afx_msg void OnTrySetSelFinishedContactCombo(long nRowEnum, long nFlags);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnCustomDraw(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnNcPaint();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTACTBAR_H__D1C25B83_015A_11D3_9447_00C04F4C8415__INCLUDED_)
