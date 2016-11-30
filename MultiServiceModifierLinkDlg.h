#if !defined(AFX_MULTISERVICEMODIFIERLINKDLG_H__0E44C9C5_FEE1_4212_8563_E63CCC1DD41E__INCLUDED_)
#define AFX_MULTISERVICEMODIFIERLINKDLG_H__0E44C9C5_FEE1_4212_8563_E63CCC1DD41E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultiServiceModifierLinkDlg.h : header file
//

// (j.jones 2007-07-03 12:54) - PLID 26098 - created

/////////////////////////////////////////////////////////////////////////////
// CMultiServiceModifierLinkDlg dialog

class CMultiServiceModifierLinkDlg : public CNxDialog
{
// Construction
public:
	CMultiServiceModifierLinkDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_ModifierCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_ServiceCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_LinkedList;	//memories of Fr. Schoen!

	// (z.manning, 05/01/2008) - PLID 29864 - Added NxIconButton
// Dialog Data
	//{{AFX_DATA(CMultiServiceModifierLinkDlg)
	enum { IDD = IDD_MULTI_SERVICE_MODIFIER_LINK_DLG };
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiServiceModifierLinkDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMultiServiceModifierLinkDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenModifierCombo(LPDISPATCH lpRow);
	afx_msg void OnRButtonDownServiceList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelChosenServiceCombo(LPDISPATCH lpRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTISERVICEMODIFIERLINKDLG_H__0E44C9C5_FEE1_4212_8563_E63CCC1DD41E__INCLUDED_)
