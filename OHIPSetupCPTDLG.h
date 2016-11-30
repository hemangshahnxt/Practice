#pragma once
#include "AdministratorRc.h"


// COHIPCPTSetupDlg dialog


// (s.tullis 2014-05-22 09:52) - PLID 62120 - created
class COHIPCPTSetupDlg : public CNxDialog
{
	

public:
	COHIPCPTSetupDlg( CWnd* pParent = NULL);   // standard constructor
	
	long m_nServiceID;
	BOOL bCanWrite;
	CNxIconButton m_btnOk;
	NxButton	m_checkAssistingCode;
	CNxIconButton	m_btnOHIPPremiumCodes;
	CNxEdit		m_editAssistingBaseUnits;

	// Dialog Data
	enum { IDD = IDD_BILLTAB_OHIP_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	void SecureControls();
	BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnCloseClick(); 
	afx_msg void OnBtnOhipPremiumCodes();
	afx_msg void OnCheckAssisting();

	
	
	

	//DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedOk();
	
	
};
