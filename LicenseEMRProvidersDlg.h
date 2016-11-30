#if !defined(AFX_LICENSEEMRPROVIDERSDLG_H__6823D3B6_F5DB_4358_9402_E25D2363101A__INCLUDED_)
#define AFX_LICENSEEMRPROVIDERSDLG_H__6823D3B6_F5DB_4358_9402_E25D2363101A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LicenseEMRProvidersDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLicenseEMRProvidersDlg dialog

// (z.manning 2013-02-04 09:16) - PLID 55001 - This dialog now supports Nuance dictation user setup as well.
enum ELicenseManageDialogType
{
	lmdtEmrProviders = 0,
	lmdtNuanceUsers,
	lmdtPortalProviders, // (z.manning 2015-06-18 14:45) - PLID 66280
};


class CLicenseEMRProvidersDlg : public CNxDialog
{
// Construction
public:
	// (z.manning 2013-02-04 09:18) - PLID 55001 - Added required dialog type param
	CLicenseEMRProvidersDlg(ELicenseManageDialogType eType, CWnd* pParent);   // standard constructor

// Dialog Data
	// (a.walling 2008-04-03 14:09) - PLID 29497 - Added a CNxStatic member
	//{{AFX_DATA(CLicenseEMRProvidersDlg)
	enum { IDD = IDD_LICENSE_MANAGE_EMRPROVIDERS };
	CNxStatic	m_labelLicensingInfo;
	CNxIconButton	m_btnRequest;
	CNxIconButton	m_btnDeactivate;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLicenseEMRProvidersDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_dlAllProvs;
	NXDATALIST2Lib::_DNxDataListPtr m_dlUsedProvs;
	NXDATALIST2Lib::_DNxDataListPtr m_dlInactiveProvs;

	bool m_bAllReady;
	bool m_bUsedReady;
	bool m_bInactiveReady;

	long m_nUsed;
	long m_nAllowed;

	ELicenseManageDialogType m_eType;

	void EnableButtons(); // enables the buttons if all the lists are done requerying.
	void UpdateInfoLabel(); // updates the info label: "Using 1 of 4 EMR Licenses (3 remaining)"

	CBrush m_brush;

	// Generated message map functions
	//{{AFX_MSG(CLicenseEMRProvidersDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEmrProvDeactivate();
	afx_msg void OnEmrProvRequest();
	afx_msg void OnRequeryFinishedListAllProvs(short nFlags);
	afx_msg void OnRequeryFinishedListUsedProvs(short nFlags);
	afx_msg void OnRequeryFinishedListInactiveProvs(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void DblClickCellListAllprovs(LPDISPATCH lpRow, short nColIndex);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LICENSEEMRPROVIDERSDLG_H__6823D3B6_F5DB_4358_9402_E25D2363101A__INCLUDED_)
