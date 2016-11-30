#pragma once

#include "PatientsRc.h"


// CCodeCorrectSetupDlg dialog
// (d.singleton 2012-04-23 11:08) - PLID 49336 added new dialog


class CCodeCorrectSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CCodeCorrectSetupDlg)

public:
	CCodeCorrectSetupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCodeCorrectSetupDlg();

// Dialog Data
	enum { IDD = IDD_CODE_CORRECT_SETUP_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnSetPassword;

	CEdit m_eUsername;

	CString m_strPassword;
	void UpdateButtonAppearance();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedPasswordButton();
};
