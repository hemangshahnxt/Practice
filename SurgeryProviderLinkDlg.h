#if !defined(AFX_SURGERYPROVIDERLINKDLG_H__D764F06C_3B5B_49DB_8F6F_A9FB5F1451CA__INCLUDED_)
#define AFX_SURGERYPROVIDERLINKDLG_H__D764F06C_3B5B_49DB_8F6F_A9FB5F1451CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SurgeryProviderLinkDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSurgeryProviderLinkDlg dialog

class CSurgeryProviderLinkDlg : public CNxDialog
{
// Construction
public:
	CSurgeryProviderLinkDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_ProviderList;

	// (j.jones 2009-08-24 11:41) - PLID 35124 - changed to preference cards
	long m_nPreferenceCardID;

// Dialog Data
	//{{AFX_DATA(CSurgeryProviderLinkDlg)
	enum { IDD = IDD_SURGERY_PROVIDER_LINK_DLG };
	NxButton	m_radioSelectedProviders;
	NxButton	m_radioAllProviders;
	CNxStatic	m_nxstaticPreferenceCardNameLabel;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSurgeryProviderLinkDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSurgeryProviderLinkDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRadioAllSrgyProviders();
	afx_msg void OnRadioSelectedSrgyProviders();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SURGERYPROVIDERLINKDLG_H__D764F06C_3B5B_49DB_8F6F_A9FB5F1451CA__INCLUDED_)
