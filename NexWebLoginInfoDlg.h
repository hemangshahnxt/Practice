#if !defined(AFX_NEXWEBLOGININFODLG_H__B316DDCD_0FAD_47A2_AF8E_E29055F50FBC__INCLUDED_)
#define AFX_NEXWEBLOGININFODLG_H__B316DDCD_0FAD_47A2_AF8E_E29055F50FBC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexWebLoginInfoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNexWebLoginInfoDlg dialog

class CNexWebLoginInfoDlg : public CNxDialog
{
// Construction
public:
	CNexWebLoginInfoDlg(long nPersonID, CWnd* pParent);   // standard constructor
	long m_nPersonID;
	CString m_strPersonName;
	CString m_strSecurityCode;
	NXDATALISTLib::_DNxDataListPtr m_pLoginList;

// Dialog Data
	//(e.lally 2009-01-22) PLID 32481 - Added new buttons
	//{{AFX_DATA(CNexWebLoginInfoDlg)
	enum { IDD = IDD_NEXWEB_LOGIN_INFO_DLG };
	CNxIconButton	m_btnAddLogin;
	CNxIconButton	m_btnRemoveLogin;
	CNxIconButton	m_btnChangePassword;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnCreateEmail;
	NxButton	m_checkEnableSecurityCode;
	BOOL	m_bEnableSecurityCode;
	CNxEdit		m_nxeditSecurityCode;
	// (b.savon 2012-08-21 09:16) - PLID 50589
	CNxEdit		m_nxeditSecurityCodeExpiration;
	NxButton	m_radioIncludeSecurityCode;
	NxButton	m_radioIncludeLogin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexWebLoginInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//(e.lally 2009-01-22) PLID 32481
	CNxColor m_nxcTop, m_nxcBottom;

	//(e.lally 2009-01-28) PLID 32814
	void HandleEmailUseRadio();
	void SetUseSecurityCode(BOOL bUseSecurityCode);
	BOOL AddLogin(CString strDefaultUserName = "");
	// (b.savon 2012-08-21 11:21) - PLID 50589
	void SetSecurityCodeLabel(ADODB::_RecordsetPtr rs);

	// Generated message map functions
	//{{AFX_MSG(CNexWebLoginInfoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddLogin();
	afx_msg void OnExitDialog();
	afx_msg void OnRemoveLogin();
	afx_msg void OnEditingFinishedLoginList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingLoginList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnRequeryFinishedLoginList(short nFlags);
	//(e.lally 2011-08-09) PLID 37287 
	afx_msg void OnEditingStartingLoginList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnChangePassword();
	afx_msg void OnCreateNexWebEmail();
	afx_msg void OnCheckUseSecurityCode();
	afx_msg void OnIncludeSecurityCode();
	afx_msg void OnIncludeLogin();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXWEBLOGININFODLG_H__B316DDCD_0FAD_47A2_AF8E_E29055F50FBC__INCLUDED_)
