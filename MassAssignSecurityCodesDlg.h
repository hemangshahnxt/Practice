#pragma once
#include "PracticeRc.h"
//(s.dhole 12/5/2014 2:28 PM ) - PLID 64337 Added new dialog to show Mass Assign Security Codes from admin ->Activities
// CmassAssignSecurityCodesDlg dialog

class CMassAssignSecurityCodesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CMassAssignSecurityCodesDlg)

public:
	CMassAssignSecurityCodesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMassAssignSecurityCodesDlg();

// Dialog Data
	enum { IDD = IDD_MASS_ASSIGN_SECURITY_CODESDlg };
	CNxIconButton	m_btnYes;
	CNxIconButton	m_btnNo;
	CNxColor m_nxclrBackground;
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL ProcessSecuritycode();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedYes();
	afx_msg void OnBnClickedNo();
};
