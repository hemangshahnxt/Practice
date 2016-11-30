#if !defined(AFX_REFERRALSUBDLG_H__DF9C8E20_E23B_484B_8100_8C2C8E3580AA__INCLUDED_)
#define AFX_REFERRALSUBDLG_H__DF9C8E20_E23B_484B_8100_8C2C8E3580AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ReferralSubDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CReferralSubDlg dialog

class CReferralSubDlg : public CNxDialog
{
// Construction
public:
	CReferralSubDlg(CWnd* pParent);   // standard constructor

	// (a.wilson 2012-06-27 16:25) - PLID 50602 - pass true if permission needs to be ignored for enable drag button
	void Update(bool bIgnoreDragPermission = false);
	void EnableAll(BOOL bEnable);

	// (a.wilson 2012-5-14) PLID 50378 - adding a default param for handling inactives in marketing.
	//retrieving data functionality
	long GetSelectedReferralID(bool bReturnInactive = false);
	CString GetSelectedReferralName(bool bReturnInactive = false);

	// (a.wilson 2012-6-26) PLID 50378 - added inactive check.
	//setting data functionality
	void ExternalAddNew();
	BOOL SelectReferralID(long nReferralID, BOOL bInactive = FALSE);

	// (r.goldschmidt 2014-08-29 12:18) - PLID 31191 - Preference setting can make certain selections illegal. Check if selection is restricted, if so, also get warning message.
	long ReferralRestrictedByPreference(long nReferralID, CString& strWarning);

	void UseBackgroundColor(COLORREF dwColor);
	void HideBackground();

	// (j.gruber 2006-11-14 15:21) - PLID 23535 - functions added to merge duplicates
	BOOL HandleDuplicates(CString strNewName, CString strOldName, long nID);
	BOOL IsDuplicate(const CString &name, long ID);

// Dialog Data
	//{{AFX_DATA(CReferralSubDlg)
	enum { IDD = IDD_REFERRAL_SUBDIALOG };
	// (a.wilson 2012-5-9) PLID 14874 - adding checkbox for showing inactive referral sources.
	NxButton	m_btnEnableDrag, m_btnShowInactive;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReferralSubDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pList;
	CTableChecker m_tcReferralChecker;
	CNxIconButton m_btnAddNewTopLevelReferral; // (c.haag 2009-08-03 11:00) - PLID 23269

	CBrush m_brushBackground;

	void ReloadTree();

	// (z.manning 2010-04-27 09:48) - PLID 38331
	void AddNewReferralSource(BOOL bTopLevel);

	//
	//For dragging
	LPDISPATCH m_lpDraggingRow;
	CArray<NXDATALIST2Lib::IRowSettings*,NXDATALIST2Lib::IRowSettings*> m_aryDragPlaceholders;

	void ClearDragPlaceholders(NXDATALIST2Lib::IRowSettingsPtr pRowToPreserve = NULL);
	BOOL IsValidDrag(NXDATALIST2Lib::IRowSettingsPtr pFromRow, NXDATALIST2Lib::IRowSettingsPtr pDestRow, CString& strReasonForFailure);
	NXDATALIST2Lib::IRowSettingsPtr CReferralSubDlg::InsertPlaceholder(NXDATALIST2Lib::IRowSettingsPtr pParentRow);
	//End dragging
	//

	// (a.walling 2008-04-03 14:22) - PLID 29497 - Added handler for OnEraseBkgnd
	// Generated message map functions
	//{{AFX_MSG(CReferralSubDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnAdd();
	afx_msg void OnAddTopLevel();
	afx_msg void OnRename();
	afx_msg void OnDelete();
	afx_msg void OnUnselect();
	afx_msg void OnRButtonDownTreeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDragBeginTreeList(BOOL FAR* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags);
	afx_msg void OnDragOverCellTreeList(BOOL FAR* pbShowDrop, LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	afx_msg void OnDragEndTreeList(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDblClickCellTreeList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnSelChangedTreeList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	// (a.wilson 2012-5-8) PLID 14874 - new functions to handle inactive status.
	afx_msg void OnInactivate();
	afx_msg void OnReactivate();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (a.wilson 2012-5-23) PLID 50602 - update to use new permissions.
	afx_msg void OnBnClickedInactiveReferrals();
	afx_msg void OnBnClickedEnableDrag();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REFERRALSUBDLG_H__DF9C8E20_E23B_484B_8100_8C2C8E3580AA__INCLUDED_)
