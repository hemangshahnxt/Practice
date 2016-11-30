#if !defined(AFX_SELECTSTATUSDLG_H__212CEEF9_2A80_4DF9_9F5B_4369C609A7DF__INCLUDED_)
#define AFX_SELECTSTATUSDLG_H__212CEEF9_2A80_4DF9_9F5B_4369C609A7DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectStatusDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectStatusDlg dialog

class CSelectStatusDlg : public CNxDialog
{
// Construction
public:
	CSelectStatusDlg(CWnd* pParent);   // standard constructor

	long m_nStatusID;

// Dialog Data
	//{{AFX_DATA(CSelectStatusDlg)
	enum { IDD = IDD_SELECT_STATUS_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectStatusDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_pStatusList;
	// Generated message map functions
	//{{AFX_MSG(CSelectStatusDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblClickCellSelectStatusList(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTSTATUSDLG_H__212CEEF9_2A80_4DF9_9F5B_4369C609A7DF__INCLUDED_)
