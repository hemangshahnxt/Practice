#pragma once


// CEMNConfidentialInfoDlg dialog
// (d.thompson 2009-05-18) - PLID 29909 - Created
#include "PatientsRc.h"

class CEMNConfidentialInfoDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEMNConfidentialInfoDlg)

public:
	CEMNConfidentialInfoDlg(CWnd* pParent);   // standard constructor
	virtual ~CEMNConfidentialInfoDlg();

	//This is both input (load the dialog) and output (retrieve from the dialog)
	CString m_strConfidentialInfo;

	//Input:  If set, the entire dialog is unchangeable.
	bool m_bReadOnly;

// Dialog Data
	enum { IDD = IDD_EMN_CONFIDENTIAL_INFO_DLG };

protected:
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxEdit m_nxeditInfo;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
};
