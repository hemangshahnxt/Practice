#pragma once
#include "NxChangePasswordDlg.h"

// (j.gruber 2009-01-06 17:48) - PLID 32480 - created for
// CNexwebLoginGeneratePasswordDlg dialog

class CNexwebLoginGeneratePasswordDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNexwebLoginGeneratePasswordDlg)

public:
	CNexwebLoginGeneratePasswordDlg(CWnd* pParent, boost::function<struct NexWebPasswordComplexity*()> fnComplexity);   // standard constructor
	virtual ~CNexwebLoginGeneratePasswordDlg();

	CNxIconButton	m_btnSelectPassword;
	CNxIconButton	m_btnNewPassword;
	CNxIconButton	m_btnCancel;

	CNxStatic	m_nxstaticGeneratedPassword;

	CString m_strPassword;
	CString GeneratePasswordText();

	// Dialog Data
	enum { IDD = IDD_NEXWEB_LOGIN_GENERATE_PASSWORD_DLG };

protected:
	void GeneratePassword();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSelectPassword();
	afx_msg void OnBnClickedGenerateNewPassword();
	afx_msg void OnBnClickedCancel();

private:
	// (j.armen 2013-10-03 07:08) - PLID 57914 - Password complexity function
	boost::function<struct NexWebPasswordComplexity*()> m_fnComplexity;
};
