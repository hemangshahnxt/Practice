#pragma once

// CDeleteEMNConfirmDlg dialog

// (j.jones 2009-10-01 11:22) - PLID 30479 - created

#include "EmrRc.h"

#define DELETE_EMN_RETURN_KEEP		10
#define DELETE_EMN_RETURN_DELETE	11

enum EMNConfirmDeletionType {

	ecdtEMN = 0,
	ecdtEMR,
	ecdtCustomRecord,
};

class CDeleteEMNConfirmDlg : public CNxDialog
{

public:
	CDeleteEMNConfirmDlg(EMNConfirmDeletionType eType, CString strDescription, CWnd* pParent);   // standard constructor

	EMNConfirmDeletionType m_eType;
	CString m_strDescription;

// Dialog Data
	enum { IDD = IDD_DELETE_EMN_CONFIRM_DLG };
	CNxIconButton m_btnKeepEMN;
	CNxIconButton m_btnDeleteEMN;
	CNxStatic	m_nxstaticTopText;
	CNxStatic	m_nxstaticDescription;
	CNxStatic	m_nxstaticBottomText;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnBtnKeepEmn();
	afx_msg void OnBtnDeleteEmn();
};
