#pragma once

#include "AdministratorRc.h"

// (k.messina 2010-07-16 10:00) - PLID 39685 - created

/////////////////////////////////////////////////////////////////////////////
// CCNYMedicaidSetupDlg dialog

class CNYMedicaidSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNYMedicaidSetupDlg)

public:
	CNYMedicaidSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CNYMedicaidSetupDlg();

    // Dialog Data
	//{{AFX_DATA(CCNYMedicaidSetupDlg)
	enum { IDD = IDD_NYMEDICAID_SETUP_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	// (j.jones 2013-04-04 17:25) - PLID 56038 - added ability to disable filling Box 25
	NxButton	m_checkBox25;
	//}}AFX_DATA

// Dialog Data

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CCNYMedicaidSetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
