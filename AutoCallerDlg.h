#if !defined(AFX_AUTOCALLERDLG_H__907A4B94_7F08_4AEF_AB3A_A730430B1214__INCLUDED_)
#define AFX_AUTOCALLERDLG_H__907A4B94_7F08_4AEF_AB3A_A730430B1214__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AutoCallerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAutoCallerDlg dialog

class CAutoCallerDlg : public CNxDialog
{
// Construction
public:
	CAutoCallerDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAutoCallerDlg)
	enum { IDD = IDD_AUTOCALLER_DLG };
	CNxIconButton	m_btnRemAll;
	CNxIconButton	m_btnSelAll;
	CNxIconButton	m_btnRemOne;
	CNxIconButton	m_btnSelOne;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr m_listUnsel;
	NXDATALISTLib::_DNxDataListPtr m_listSel;
	NXDATALISTLib::_DNxDataListPtr m_listGroup;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAutoCallerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAutoCallerDlg)
	afx_msg void OnUnselectAll();
	afx_msg void OnSelectAll();
	afx_msg void OnUnselect();
	afx_msg void OnSelect();
	afx_msg void OnStart();
	afx_msg void OnSetup();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnDblClickCellSelected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellUnselected(long nRowIndex, short nColIndex);
	afx_msg void OnTimer(UINT nIDEvent);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUTOCALLERDLG_H__907A4B94_7F08_4AEF_AB3A_A730430B1214__INCLUDED_)
