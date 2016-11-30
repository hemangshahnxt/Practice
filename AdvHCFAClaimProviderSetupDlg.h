#if !defined(AFX_ADVHCFACLAIMPROVIDERSETUPDLG_H__09A32192_3005_4E99_91E3_0405F1CD31C7__INCLUDED_)
#define AFX_ADVHCFACLAIMPROVIDERSETUPDLG_H__09A32192_3005_4E99_91E3_0405F1CD31C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvHCFAClaimProviderSetupDlg.h : header file
//

// (j.jones 2008-08-01 09:52) - PLID 30918 - created

/////////////////////////////////////////////////////////////////////////////
// CAdvHCFAClaimProviderSetupDlg dialog

class CAdvHCFAClaimProviderSetupDlg : public CNxDialog
{
// Construction
public:
	CAdvHCFAClaimProviderSetupDlg(CWnd* pParent);   // standard constructor

	OLE_COLOR m_nColor;

	long m_nDefaultHCFAGroupID;	//given to us if we are told to auto-select all companies in a group
	
	long m_nDefaultInsCoID;	//given to us if we are told to auto-select one company

	long m_nDefaultProviderID;	//given to us if we are told to auto-select certain providers
	long m_nDefault2010AA;
	long m_nDefault2310B;
	long m_nDefaultBox33;
	long m_nDefaultBox24J;

// Dialog Data
	//{{AFX_DATA(CAdvHCFAClaimProviderSetupDlg)
	enum { IDD = IDD_ADV_HCFA_CLAIM_PROVIDER_SETUP_DLG };
	CNxStatic	m_nxstaticDefaultProviderLabel;
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnUnselectAll;
	CNxIconButton	m_btnSelectOne;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnApply;
	CNxIconButton	m_btnClose;
	CNxColor	m_bkg;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvHCFAClaimProviderSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_UnselectedList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedList;
	NXDATALIST2Lib::_DNxDataListPtr m_MainProviderCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_2010AACombo;
	NXDATALIST2Lib::_DNxDataListPtr m_2310BCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_Box33Combo;
	NXDATALIST2Lib::_DNxDataListPtr m_Box24JCombo;

	void SetColor(OLE_COLOR nNewColor);

	// Generated message map functions
	//{{AFX_MSG(CAdvHCFAClaimProviderSetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBtnClose();
	afx_msg void OnDblClickCellAdvClaimProvUnselectedList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellAdvClaimProvSelectedList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBtnAdvClaimProvSelectOne();
	afx_msg void OnBtnAdvClaimProvSelectAll();
	afx_msg void OnBtnAdvClaimProvUnselectAll();
	afx_msg void OnBtnAdvClaimProvUnselectOne();
	afx_msg void OnBtnApplyClaimProv();
	afx_msg void OnSelChosenMainProviderCombo(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedMainProviderCombo(short nFlags);
	afx_msg void OnRequeryFinishedAdvClaimProvUnselectedList(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVHCFACLAIMPROVIDERSETUPDLG_H__09A32192_3005_4E99_91E3_0405F1CD31C7__INCLUDED_)
