#if !defined(AFX_EMRITEMLINKINGDLG_H__968D832E_3FCC_4F19_8003_B89C1CBF3CEF__INCLUDED_)
#define AFX_EMRITEMLINKINGDLG_H__968D832E_3FCC_4F19_8003_B89C1CBF3CEF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrItemLinkingDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEmrItemLinkingDlg dialog

class CEmrItemLinkingDlg : public CNxDialog
{
// Construction
public:
	CEmrItemLinkingDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEmrItemLinkingDlg)
	enum { IDD = IDD_EMR_ITEM_LINKING_DLG };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnNewItemLink;
	CNxIconButton m_btnDeleteItemLink;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrItemLinkingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pList;

	void LoadLinks();

	// Generated message map functions
	//{{AFX_MSG(CEmrItemLinkingDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnNewItemLink();
	afx_msg void OnDeleteItemLink();
	afx_msg void OnDblClickCellListItemLinks(LPDISPATCH lpRow, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRITEMLINKINGDLG_H__968D832E_3FCC_4F19_8003_B89C1CBF3CEF__INCLUDED_)
