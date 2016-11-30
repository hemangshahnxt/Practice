#if !defined(AFX_NEWVERSIONDLG_H__2B666E01_31F7_4E43_9974_5FDB9F0D735A__INCLUDED_)
#define AFX_NEWVERSIONDLG_H__2B666E01_31F7_4E43_9974_5FDB9F0D735A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewVersionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewVersionDlg dialog

class CNewVersionDlg : public CNxDialog
{
// Construction
public:
	CNewVersionDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewVersionDlg)
	enum { IDD = IDD_NEW_VERSION_DLG };
	CNxIconButton	m_btnNewVersion;
	CNxIconButton	m_btnDeleteVersion;
	CNxIconButton	m_btnClose;
	CNxEdit	m_nxeditVersionNumber;
	CNxEdit	m_nxeditAmaVersion;
	CNxEdit	m_nxeditMsiVersion;
	CNxEdit	m_nxeditLicenseVersion;
	CNxEdit	m_nxeditScopeVersion;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewVersionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pVersionList;
	NXTIMELib::_DNxTimePtr m_pReleaseDate;

	// Generated message map functions
	//{{AFX_MSG(CNewVersionDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnKillFocusReleaseDate();
	afx_msg void OnKillfocusVersionNumber();
	afx_msg void OnKillfocusAmaVersion();
	afx_msg void OnSelChosenVersionList(long nRow);
	afx_msg void OnSelChangingVersionList(long FAR* nNewSel);
	afx_msg void OnNewVersion();
	afx_msg void OnDeleteVersion();
	afx_msg void OnKillfocusMsiVersion();
	afx_msg void OnKillfocusLicenseVersion();
	afx_msg void OnKillfocusScopeVersion();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWVERSIONDLG_H__2B666E01_31F7_4E43_9974_5FDB9F0D735A__INCLUDED_)
