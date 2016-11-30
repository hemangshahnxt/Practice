#pragma once
#include "AdministratorRc.h"

// CConfigureNexWebPasswordComplexityRequirementsDlg dialog

class CConfigureNexWebPasswordComplexityRequirementsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CConfigureNexWebPasswordComplexityRequirementsDlg)
private:
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	// (b.savon 2012-08-16 17:27) - PLID 50584
	CNxIconButton m_btnQuestion;

	NxButton m_chkLoginAttempts;
	NxButton m_chkLoginDuration;
	NxButton m_chkAdminReset;

	CNxColor m_nxcBackground;
	CNxColor m_nxcLockoutBackground;
	CNxColor m_nxcSecurityCode;

	CNxEdit m_nxeMinPasswordLength;
	CNxEdit m_nxeMinLetters;
	CNxEdit m_nxeMinNumbers;
	CNxEdit m_nxeMinUppercaseLetters;
	CNxEdit m_nxeMinLowercaseLetters;
	// (b.savon 2012-08-16 17:27) - PLID 50584
	CNxEdit m_nxeLockoutAttempts;
	CNxEdit m_nxeLockoutDuration;
	CNxEdit m_nxeLockoutReset;
	// (b.savon 2012-08-21 09:21) - PLID 50589
	CNxEdit m_nxeSecurityCodeExpire;

	BOOL IsPasswordRequirementSet();
	CString CalcAuditDesc(long nLength, long nLetters, long nNumbers, long nUpper, long nLower);
	// (b.savon 2012-08-16 17:27) - PLID 50584
	CString CalcLockoutAuditDesc(long nAttempts, long nDuration, long nReset);
	// (b.savon 2012-08-21 09:29) - PLID 50589
	CString CalcSecuirtyCodeExpirationAuditDesc(long nSecurityCodeExpiration);

	long m_nMinPasswordLength;
	long m_nMinLetters;
	long m_nMinNumbers;
	long m_nMinUppercaseLetters;
	long m_nMinLowercaseLetters;
	long m_nPasswordRequirementsEnforced;
	// (b.savon 2012-08-16 17:27) - PLID 50584
	long m_nLockoutAttempts;
	long m_nLockoutDuration;
	long m_nLockoutReset;
	// (b.savon 2012-08-21 09:21) - PLID 50589
	long m_nSecurityCodeExpire;

public:
	CConfigureNexWebPasswordComplexityRequirementsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfigureNexWebPasswordComplexityRequirementsDlg();
	
// Dialog Data
	enum { IDD = IDD_NEXWEB_CONFIGURE_PASSWORD_SECURITY_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	BOOL ValidateAndSavePasswordComplexity();
	// (b.savon 2012-08-16 17:28) - PLID 50584
	void SetBoldText(long nControlID);
	BOOL ValidateAndSaveLockoutSettings();
	BOOL ValidateLockoutSettings();
	// (b.savon 2012-08-21 09:26) - PLID 50589
	void ValidateAndSaveSecurityCodeExpiration();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnKillfocusMinLowercaseLetters();
	afx_msg void OnEnKillfocusMinUppercaseLetters();
	afx_msg void OnEnKillfocusMinNumbers();
	afx_msg void OnEnKillfocusMinLetters();
	afx_msg void OnEnKillfocusMinPasswordLength();
	afx_msg void OnBnClickedBtnMoreInfo();
	afx_msg void OnEnKillfocusLockoutAttempts();
	afx_msg void OnEnKillfocusLockoutMinutes();
	afx_msg void OnEnKillfocusLockoutCycles();
	afx_msg void OnEnKillfocusSecurityExpire();
	afx_msg void OnBnClickedEnableAttempts();
	afx_msg void OnBnClickedEnableDuration();
	afx_msg void OnBnClickedEnableAdmin();
};
