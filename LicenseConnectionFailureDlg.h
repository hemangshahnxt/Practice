#pragma once

// (j.jones 2016-03-03 09:01) - PLID 68479 - created

// CLicenseConnectionFailureDlg dialog

#include "SizeableTextDlg.h"

class CLicenseConnectionFailureDlg : public CNxDialog
{

public:
	//this dialog requires a subkey and a license object
	CLicenseConnectionFailureDlg(CWnd* pParent, CString strSubkey, CPracticeLicense* pLicense);   // standard constructor
	virtual ~CLicenseConnectionFailureDlg();

	//checks to see if we have connectivity, shows the dialog if we do not
	void CheckShowDialog();
	
	// (j.jones 2016-03-03 13:31) - PLID 68478 - added to tell our caller that
	// the user needs to be locked out from Practice
	bool m_bPreventLogin;

// Dialog Data
	enum { IDD = IDD_LICENSE_CONNECTION_FAILURE_DLG };
	CNxStatic	m_nxstaticLicenseWarningLabel1;
	CNxStatic	m_nxstaticLicenseWarningLabel2;
	CNxStatic	m_nxstaticPhoneLabel;
	CNxLabel	m_nxlstaticWebSite;
	CNxLabel	m_nxlstaticEmail;
	CNxStatic	m_nxstaticRetryLabel;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnRetry;

protected:

	//we need the current subkey
	CString m_strSubkey;

	//This dialog needs access to a license, and g_pLicense may not be set yet.
	CPracticeLicense* m_pLicense;

	//tracks how many days before a lockout will occur, can be negative
	long m_nDaysRemaining;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBtnClose();
	afx_msg void OnBtnRetry();
};
