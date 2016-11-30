#pragma once

// (j.jones 2008-11-18 11:55) - PLID 28572 - created

// CPasswordConfigDlg dialog

class CPasswordConfigDlg : public CNxDialog
{
public:
	CPasswordConfigDlg(CWnd* pParent);   // standard constructor

	//this will tell the caller if we updated UsersT.PWExpireNextLogin for all users
	BOOL m_bMadeAllUserPasswordsExpire;

// Dialog Data
	enum { IDD = IDD_PASSWORD_CONFIG_DLG };
	CNxEdit		m_editMinimumLength;
	CNxEdit		m_editMinimumLetters;
	CNxEdit		m_editMinimumNumbers;
	CNxEdit		m_editMinimumUpper;
	CNxEdit		m_editMinimumLower;
	CNxStatic	m_nxstaticSamplePassword;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	// (j.jones 2009-05-19 14:32) - PLID 33801 - supported preventing reuse of last X passwords
	NxButton	m_checkPreventReuseOfLastPasswords;
	CNxEdit		m_editPreventLastPasswords;
	// (a.walling 2009-12-16 14:58) - PLID 35784 - Supported a login banner
	CNxEdit		m_editLoginBanner;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	//CalcSamplePassword will display a sample valid password on the screen
	void CalcSamplePassword();

	//CalcAuditDesc will generate an audit description including each non-zero field
	// (j.jones 2009-05-19 16:02) - PLID 33801 - supported preventing reuse of last X passwords
	CString CalcAuditDesc(long nLength, long nLetters, long nNumbers, long nUpper, long nLower,
		BOOL bPreventReuseOfLastPasswords, long nPreventLastPasswords);

	//these variables will help track whether we made changes
	long m_nOrig_MinLength;
	long m_nOrig_MinLetters;
	long m_nOrig_MinNumbers;
	long m_nOrig_MinUpper;
	long m_nOrig_MinLower;

	// (j.jones 2009-05-19 16:02) - PLID 33801 - supported preventing reuse of last X passwords
	BOOL m_bOrig_PreventReuseOfLastPasswords;
	long m_nOrig_PreventLastPasswords;

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnOk();
	afx_msg void OnCancel();
	afx_msg void OnEnKillfocusEditPasswordMinCharacters();
	afx_msg void OnEnKillfocusEditPasswordMinLetters();
	afx_msg void OnEnKillfocusEditPasswordMinNumbers();
	afx_msg void OnEnKillfocusEditPasswordMinUppercase();
	afx_msg void OnEnKillfocusEditPasswordMinLowercase();
	// (j.jones 2009-05-19 14:32) - PLID 33801 - supported preventing reuse of last X passwords
	afx_msg void OnCheckPreventReusePasswords();
};
