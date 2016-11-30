#if !defined(AFX_COPYEMPLOYERDLG_H__7661DD6B_1BB4_4C1E_A1F1_D53E30FA6D89__INCLUDED_)
#define AFX_COPYEMPLOYERDLG_H__7661DD6B_1BB4_4C1E_A1F1_D53E30FA6D89__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CopyEmployerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCopyEmployerDlg dialog

class CCopyEmployerDlg : public CNxDialog
{
// Construction
public:
	CCopyEmployerDlg(CWnd* pParent);   // standard constructor
	void FilterOnCompany(CString strCompany);

	//strings to send back for insertion
	CString m_company, m_addr1, m_addr2, m_city, m_state, m_zip;

// Dialog Data
	//{{AFX_DATA(CCopyEmployerDlg)
	enum { IDD = IDD_COPY_EMPLOYER_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCopyEmployerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_listCompany;
	CString m_strFilterOn;

	// Generated message map functions
	//{{AFX_MSG(CCopyEmployerDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblClickCellEmployerList(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COPYEMPLOYERDLG_H__7661DD6B_1BB4_4C1E_A1F1_D53E30FA6D89__INCLUDED_)
