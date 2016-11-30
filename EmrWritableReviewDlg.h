#if !defined(AFX_EMRWRITABLEREVIEWDLG_H__81149D51_74C6_414D_945E_A13A55472AA4__INCLUDED_)
#define AFX_EMRWRITABLEREVIEWDLG_H__81149D51_74C6_414D_945E_A13A55472AA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrWritableReviewDlg.h : header file
//

// (a.walling 2008-06-26 13:23) - PLID 30531 - Dialog to review EMNs/Templates that are currently locked for editing

/////////////////////////////////////////////////////////////////////////////
// CEmrWritableReviewDlg dialog

class CEmrWritableReviewDlg : public CNxDialog
{
// Construction
public:
	CEmrWritableReviewDlg(CWnd* pParent);   // standard constructor
	~CEmrWritableReviewDlg();

// Dialog Data
	//{{AFX_DATA(CEmrWritableReviewDlg)
	enum { IDD = IDD_EMR_WRITABLE_REVIEW };
	CNxIconButton	m_nxibRefresh;
	CNxStatic	m_lblInfo;
	CNxIconButton	m_nxibOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrWritableReviewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pList;

	BOOL m_bNeedRefresh;

	// Generated message map functions
	//{{AFX_MSG(CEmrWritableReviewDlg)
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEmrWritableRefreshBtn();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnLeftClickEmrWritableList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBtnClose();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRWRITABLEREVIEWDLG_H__81149D51_74C6_414D_945E_A13A55472AA4__INCLUDED_)
