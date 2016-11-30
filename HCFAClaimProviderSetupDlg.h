#if !defined(AFX_HCFACLAIMPROVIDERSETUPDLG_H__2C27FB3E_30FB_4017_A774_E2585C452D41__INCLUDED_)
#define AFX_HCFACLAIMPROVIDERSETUPDLG_H__2C27FB3E_30FB_4017_A774_E2585C452D41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HCFAClaimProviderSetupDlg.h : header file
//

// (j.jones 2008-04-03 11:37) - PLID 28995 - created

/////////////////////////////////////////////////////////////////////////////
// CHCFAClaimProviderSetupDlg dialog

class CHCFAClaimProviderSetupDlg : public CNxDialog
{
// Construction
public:
	CHCFAClaimProviderSetupDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2008-08-01 09:45) - PLID 30917 - added m_nInsCoID and m_nColor
	long m_nInsCoID;
	OLE_COLOR m_nColor;

	// (j.jones 2008-08-01 12:55) - PLID 30918 - added m_btnAdvancedSetup
// Dialog Data
	//{{AFX_DATA(CHCFAClaimProviderSetupDlg)
	enum { IDD = IDD_HCFA_CLAIM_PROVIDER_SETUP_DLG };
	CNxIconButton	m_btnAdvancedSetup;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxStatic		m_DefaultInfoLabel;
	CNxColor	m_bkg;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHCFAClaimProviderSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_MainProviderCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_2010AACombo;
	NXDATALIST2Lib::_DNxDataListPtr m_2310BCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_Box33Combo;
	NXDATALIST2Lib::_DNxDataListPtr m_Box24JCombo;

	BOOL m_bChanged;
	long m_nProviderID;

	void Load();
	BOOL Save();

	// (j.jones 2008-08-01 09:45) - PLID 30917 - added SetColor
	void SetColor(OLE_COLOR nNewColor);

	// (j.jones 2008-08-01 12:55) - PLID 30918 - added OnBtnAdvancedClaimProvSetup
	// Generated message map functions
	//{{AFX_MSG(CHCFAClaimProviderSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenMainProviderCombo(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedMainProviderCombo(short nFlags);
	virtual void OnOK();
	afx_msg void OnSelChosen2010aaCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenBox33Combo(LPDISPATCH lpRow);
	afx_msg void OnSelChosen2310bCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenBox24jCombo(LPDISPATCH lpRow);
	afx_msg void OnBtnAdvancedClaimProvSetup();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HCFACLAIMPROVIDERSETUPDLG_H__2C27FB3E_30FB_4017_A774_E2585C452D41__INCLUDED_)
