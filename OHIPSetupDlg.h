#if !defined(AFX_OHIPSETUPDLG_H__F193DCD9_C1B1_4EC6_8ECB_D87790F44153__INCLUDED_)
#define AFX_OHIPSETUPDLG_H__F193DCD9_C1B1_4EC6_8ECB_D87790F44153__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OHIPSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COHIPSetupDlg dialog

class COHIPSetupDlg : public CNxDialog
{
// Construction
public:
	COHIPSetupDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_HealthNumberCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_VersionCodeCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_RegistrationNumberCombo;

	// (j.jones 2010-05-04 11:41) - PLID 32325 - we now cache the Health Number field
	// and tell the caller if it changed
	long m_nHealthNumberCustomField;
	BOOL m_bHealthNumberCustomFieldChanged;

	// (j.jones 2008-05-08 10:17) - PLID 29953 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(COHIPSetupDlg)
	enum { IDD = IDD_OHIP_SETUP_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxEdit	m_nxeditEditMohOfficeCode;
	CNxEdit	m_nxeditEditGroupNumber;
	// (j.jones 2009-06-26 09:13) - PLID 34292 - moved Specialty to Contacts module
	//CNxEdit	m_nxeditEditSpecialty;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COHIPSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COHIPSetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OHIPSETUPDLG_H__F193DCD9_C1B1_4EC6_8ECB_D87790F44153__INCLUDED_)
