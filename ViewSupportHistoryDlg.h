#if !defined(AFX_VIEWSUPPORTHISTORYDLG_H__7538AFAA_63F7_4832_84CA_D2E055A46F4E__INCLUDED_)
#define AFX_VIEWSUPPORTHISTORYDLG_H__7538AFAA_63F7_4832_84CA_D2E055A46F4E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewSupportHistoryDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewSupportHistoryDlg dialog

class CViewSupportHistoryDlg : public CNxDialog
{
// Construction
public:
	CViewSupportHistoryDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CViewSupportHistoryDlg)
	enum { IDD = IDD_VIEW_SUPPORT_HISTORY_DLG };
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewSupportHistoryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pList;
	void LoadHistory();

	// Generated message map functions
	//{{AFX_MSG(CViewSupportHistoryDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	// (j.shoben 2013-06-24 11:05) - PLID 56163 - Checkbox to hide automatic update client download for auditing purposes
	bool m_bHideAutoUpdate;
	afx_msg void OnBnClickedHideautoupdate();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWSUPPORTHISTORYDLG_H__7538AFAA_63F7_4832_84CA_D2E055A46F4E__INCLUDED_)
