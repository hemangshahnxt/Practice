//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_USERPROPSDLG_H__BEA1CD27_EA65_4622_9DFC_01405443EDA3__INCLUDED_)
#define AFX_USERPROPSDLG_H__BEA1CD27_EA65_4622_9DFC_01405443EDA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UserPropsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUserPropsDlg dialog

class CUserPropsDlg : public CNxDialog
{
	// (b.savon 2012-02-21 14:56) - PLID 48274 - Respect password rules
private:
	BOOL m_bNewUser;

// Construction
public:
	NXDATALIST2Lib::_DNxDataListPtr m_ProvCombo; // (j.luckoski 2013-03-28 10:26) - PLID 55913
	CUserPropsDlg(CWnd* pParent);   // standard constructor
	CString m_name, m_pass, m_oldName, m_strFullName;
	long m_nOldProviderID, m_nNewProviderID; // (j.luckoski 2013-03-28 11:30) - PLID 55928 - For auditing
	CString m_strOldProvider, m_strNewProvider; // (j.luckoski 2013-03-29 14:01) - PLID 55928 - Auditing
	CDWordArray m_adwImportPermissionsFrom;
	// (j.politis 2015-07-08 14:16) - PLID 64164 - It would be convenient to have a preference that allows users to set newly added contacts to be added to all practice locatoins, rather than just one.
	bool m_bMemory, m_bInactivity, m_bAdministrator, m_bExpires, m_bAllLocations;
	// (j.jones 2008-11-20 08:49) - PLID 23227 - added m_bPasswordExpiresNextLogin
	BOOL m_bPasswordExpiresNextLogin;
	int m_nInactivityMinutes;
	int m_nPwExpireDays;
	long m_nUserID;
	//TES 4/30/2009 - PLID 28573 - Added m_nFailedLogins
	long m_nFailedLogins;
	// (j.gruber 2010-04-14 13:49) - PLID 38186 - whether to show configure button
	BOOL m_bShowConfigureGroups;

	// (b.savon 2012-02-21 14:58) - PLID 48274 - Respect password rules
	void SetNewUser(BOOL bNewUser){ m_bNewUser = bNewUser;	}
	BOOL GetNewUser(){ return m_bNewUser; }

	// (j.jones 2008-11-18 15:18) - PLID 28572 - added m_btnConfigPasswordStrength
	// (j.jones 2008-11-20 08:49) - PLID 23227 - added m_checkPasswordExpiresNextLogin
// Dialog Data
	//{{AFX_DATA(CUserPropsDlg)
	enum { IDD = IDD_USER_CHANGE_PASS_DLG };	
	NxButton	m_checkAdministrator;
	NxButton	m_inactivity;
	CNxEdit	m_userName;
	NxButton	m_memory;
	NxButton m_expires;
	CNxEdit	m_nxeditUserPassBox;
	CNxEdit	m_nxeditUserPassVerBox;
	CNxEdit	m_nxeditEditPwExpires;
	CNxEdit	m_nxeditInactivityBox;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnClearPermissions;
	CNxIconButton	m_btnImportPermsFrom;
	CNxIconButton	m_btnConfigPasswordStrength;
	NxButton	m_checkPasswordExpiresNextLogin;
	CNxStatic	m_nxsUnlockLabel;
	CNxIconButton	m_btnUnlockUser;
	CNxIconButton m_btnConfigureGroups;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUserPropsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.jones 2008-11-18 15:18) - PLID 28572 - added OnBtnConfigurePasswordStrength
	// (j.gruber 2010-04-13 16:30) - PLID 38186 - added configure Groups Button
	// Generated message map functions
	//{{AFX_MSG(CUserPropsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnInactivityChk();
	afx_msg void OnBtnImportPermsFrom();
	afx_msg void OnAdministratorCheck();
	afx_msg void OnExpireCheck();
	afx_msg void OnBtnClearPermissions();
	afx_msg void OnBtnConfigurePasswordStrength();
	afx_msg void OnBnClickedConfigureGroups();
	afx_msg void OnSelChosenProvIDCombo(LPDISPATCH lpRow); // (j.luckoski 2013-03-28 10:52) - PLID 55913
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	
public:
	afx_msg void OnUnlockUser();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USERPROPSDLG_H__BEA1CD27_EA65_4622_9DFC_01405443EDA3__INCLUDED_)
