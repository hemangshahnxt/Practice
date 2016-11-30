#if !defined(AFX_CREDENTIALSDLG_H__9FA58DAE_1879_4603_BF38_8BCB86CFD0AF__INCLUDED_)
#define AFX_CREDENTIALSDLG_H__9FA58DAE_1879_4603_BF38_8BCB86CFD0AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CredentialsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCredentialsDlg dialog

class CCredentialsDlg : public CNxDialog
{
// Construction
public:
	CCredentialsDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_ProviderCombo, 
					m_UnselectedCPTList, m_SelectedCPTList,
					m_UnselectedProcedureList, m_SelectedProcedureList;

	CTableChecker m_providerChecker, m_cptChecker;

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	void Load();

	void EnableControls();

// Dialog Data
	//{{AFX_DATA(CCredentialsDlg)
	enum { IDD = IDD_CREDENTIALS_DLG };
	CNxIconButton	m_btnConfigureLicenses;
	CNxIconButton	m_btnUnselectAllCPT;
	CNxIconButton	m_btnUnselectOneCPT;
	CNxIconButton	m_btnSelectAllCPT;
	CNxIconButton	m_btnSelectOneCPT;
	CNxIconButton	m_btnUnselectAllProcedures;
	CNxIconButton	m_btnUnselectOneProcedure;
	CNxIconButton	m_btnSelectAllProcedures;
	CNxIconButton	m_btnSelectOneProcedure;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCredentialsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;

	// Generated message map functions
	//{{AFX_MSG(CCredentialsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnSelectOneCpt();
	afx_msg void OnBtnSelectAllCpt();
	afx_msg void OnBtnUnselectOneCpt();
	afx_msg void OnBtnUnselectAllCpt();
	afx_msg void OnSelChosenProviderCredentialCombo(long nRow);
	afx_msg void OnDblClickCellUnselectedCptList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedCptList(long nRowIndex, short nColIndex);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDblClickCellUnselectedProcedureList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedProcedureList(long nRowIndex, short nColIndex);
	afx_msg void OnBtnSelectOneProcedure();
	afx_msg void OnBtnSelectAllProcedures();
	afx_msg void OnBtnUnselectOneProcedure();
	afx_msg void OnBtnUnselectAllProcedures();
	afx_msg void OnBtnConfigureLicenses();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREDENTIALSDLG_H__9FA58DAE_1879_4603_BF38_8BCB86CFD0AF__INCLUDED_)
