#pragma once

// CMarkAllInsAcceptedDlg dialog

// (j.jones 2010-07-30 14:09) - PLID 39917 - created

#include "PatientsRc.h"

class CMarkAllInsAcceptedDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CMarkAllInsAcceptedDlg)

public:
	CMarkAllInsAcceptedDlg(CWnd* pParent);   // standard constructor
	virtual ~CMarkAllInsAcceptedDlg();

// Dialog Data
	enum { IDD = IDD_MARK_ALL_INS_ACCEPTED_DLG };
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnAcceptAll;
	CNxIconButton	m_btnAcceptNone;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnAcceptAll();
	afx_msg void OnBtnAcceptNone();
};
