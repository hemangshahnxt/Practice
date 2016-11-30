#pragma once
#include "AdministratorRc.h"
#include "NxAPI.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>

// CNexERxRegistrationDlg dialog
// (b.savon 2012-12-03 15:51) - PLID 53559 - Created.

class CNexERxRegistrationDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNexERxRegistrationDlg)

private:
	CNxColor m_nxcBackground;
	CNxIconButton m_btnSync;
	CNxIconButton m_btnClose;

	IWebBrowser2Ptr m_pBrowser;

	LPCTSTR GetSyncResultMessage(NexTech_Accessor::SyncClientSettingsAndPrescriberStatus syncStatus);

	// (b.savon 2013-05-21 09:05) - PLID 56678
	void CacheProperties();
	CString GetRegistrationWebServiceURL();
	CString GetProductionURL();
	CString GetPreProductionURL();
	CString GetPrescriberRegistrationURL(CString strWebServiceURL);

public:
	CNexERxRegistrationDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNexERxRegistrationDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_NEXERX_REGISTRATION_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBtnSyncNexerxPrescribers();
};
