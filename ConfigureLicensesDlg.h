#if !defined(AFX_CONFIGURELICENSESDLG_H__D8652B58_E47E_4046_8D14_482E68C9F3B6__INCLUDED_)
#define AFX_CONFIGURELICENSESDLG_H__D8652B58_E47E_4046_8D14_482E68C9F3B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigureLicensesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigureLicensesDlg dialog

class CConfigureLicensesDlg : public CNxDialog
{
// Construction
public:
	CConfigureLicensesDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_PersonCombo, 
					m_LicenseList,
					m_RespUserCombo;

	long m_nPersonID;

	void EnableControls();

	// (z.manning, 04/30/2008) - PLID 29852 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CConfigureLicensesDlg)
	enum { IDD = IDD_CONFIGURE_LICENSES_DLG };
	CNxEdit	m_nxeditExpireWarnDayEdit;
	CNxIconButton	m_btnAddLicense;
	CNxIconButton	m_btnDeleteLicense;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigureLicensesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigureLicensesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenProviderLicenseCombo(long nRow);
	afx_msg void OnEditingFinishingLicenseList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedLicenseList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnBtnAddLicense();
	afx_msg void OnBtnDeleteLicense();
	afx_msg void OnKillfocusExpireWarnDayEdit();
	afx_msg void OnSelChosenUserRespList(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGURELICENSESDLG_H__D8652B58_E47E_4046_8D14_482E68C9F3B6__INCLUDED_)
