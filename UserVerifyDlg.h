#pragma once

// CUserVerifyDlg dialog

// (j.jones 2013-08-08 09:23) - PLID 57915 - created

#include "PracticeRc.h"

class CUserVerifyDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CUserVerifyDlg)

public:
	CUserVerifyDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUserVerifyDlg();

	int DoModal(CString strDialogTitle = "Please Select A User", CString strLabelText = "Select your user name and enter a password:",
		long nDefaultUserID = -1, CString strUserWhereClause = "PersonT.Archived = 0");

	long m_nSelectedUserID;
	CString m_strSelectedUserName;

// Dialog Data
	enum { IDD = IDD_USER_VERIFY_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxStatic		m_staticUserVerifyLabel;
	CNxEdit			m_editPasswordBox;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_UserCombo;

	CString m_strDialogTitle;
	CString m_strLabelText;
	CString m_strUserWhereClause;
	long m_nDefaultUserID;

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnOk();
	DECLARE_EVENTSINK_MAP()
	void OnSelChangingUserVerifyCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel); 
	// (z.manning 2016-04-19 9:13) - NX-100244 - Created
	void RequeryFinishedUserVerifyCombo(short nFlags);
	void SelChosenUserVerifyCombo(LPDISPATCH lpRow);
};