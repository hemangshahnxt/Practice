#if !defined(AFX_RULEENTRYDLG_H__5B900FA4_599F_4B83_BAFB_E560F3E8C4DD__INCLUDED_)
#define AFX_RULEENTRYDLG_H__5B900FA4_599F_4B83_BAFB_E560F3E8C4DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RuleEntryDlg.h : header file
//

class CTemplateRuleInfo;

/////////////////////////////////////////////////////////////////////////////
// CRuleEntryDlg dialog

class CRuleEntryDlg : public CNxDialog
{
// Construction
public:
	CRuleEntryDlg(CWnd* pParent);   // standard constructor

	virtual int ZoomRule(CTemplateRuleInfo *pRuleInfo);

	// (z.manning, 04/29/2008) - PLID 29814 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CRuleEntryDlg)
	enum { IDD = IDD_RULE_ENTRY_DLG };
	CNxEdit	m_nxeditDescriptionEdit;
	CNxEdit	m_nxeditWarningEdit;
	CNxStatic	m_nxstaticListofTypes;
	CNxStatic	m_nxstaticListofPurposes;
	CNxStatic	m_nxstaticListofBookings;
	CNxStatic	m_nxstaticListofAnddetails;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	NxButton	m_btnDescGroupbox;
	NxButton	m_btnRuleOptionsGroupbox;
	NxButton	m_btnWarningGroupbox;
	NxButton	m_checkAll;
	NxButton	m_checkType;
	NxButton	m_checkPurpose;
	NxButton	m_checkBooking;
	NxButton	m_checkWarn;
	NxButton	m_checkPrevent;
	NxButton	m_checkOverrideLocationTemplating;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRuleEntryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CRect m_rcTypeList;
	
	CRect m_rcTypeListIs;
	CString m_strTypeListIs;
	
	CRect m_rcTypeListList;
	CDWordArray m_aryTypeListIDs;
	CString m_strTypeListNames;

	void DrawTypeList(CDC *pdc);

protected:
	CRect m_rcPurposeList;
	
	CRect m_rcPurposeListIs;
	CString m_strPurposeListIs;
	
	CRect m_rcPurposeListList;
	CDWordArray m_aryPurposeListIDs;
	CString m_strPurposeListNames;

	void DrawPurposeList(CDC *pdc);

protected:
	CRect m_rcBookingLabel;
	
	CRect m_rcBookingCount;
	long m_nBookingCount;

	void DrawBookingLabel(CDC *pdc);

protected:
	BOOL m_bAndDetails;
	
	CRect m_rcAndDetailsLabel;
	CRect m_rcAllOrAny;

	void DrawAndDetails(CDC *pdc);

	
protected:
	void DoClickHyperlink(UINT nFlags, CPoint point);
	
	CTemplateRuleInfo *m_pRuleInfo;
	
	BOOL ValidateRule();
	void SaveRule();
	void SaveTypeList();
	void SavePurposeList();

	void LoadRule();
	void LoadTypeList();
	void LoadPurposeList();

	// Generated message map functions
	//{{AFX_MSG(CRuleEntryDlg)
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTypeCheck();
	afx_msg void OnPurposeCheck();
	afx_msg void OnWarnCheck();
	virtual void OnOK();
	afx_msg void OnBookingCheck();
	afx_msg void OnAllCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RULEENTRYDLG_H__5B900FA4_599F_4B83_BAFB_E560F3E8C4DD__INCLUDED_)
