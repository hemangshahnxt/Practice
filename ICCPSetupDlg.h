#pragma once


// ICCPSetupDlg dialog
// Integrated Credit Card Processing Setup dialog
// (d.lange 2015-07-22 16:35) - PLID 67188 - Initial implementation
#include "PracticeRc.h"

class CICCPSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CICCPSetupDlg)

public:
	CICCPSetupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CICCPSetupDlg();

	// Dialog Data
	enum { IDD = IDD_ICCP_SETUP_DLG };

protected:
	// Controls
	CNxIconButton m_btnClose;
	CNxIconButton m_btnAddMerchant;
	CNxIconButton m_btnRenameMerchant;
	CNxIconButton m_btnDeleteMerchant;
	CNxIconButton m_btnTestEMV; // (c.haag 2015-07-30) - PLID 67191
	CNxIconButton m_btnEnableICCP; // (c.haag 2015-07-30) - PLID 67190
	CNxEdit m_nxeditMerchantID;
	NXDATALIST2Lib::_DNxDataListPtr m_pMerchAccountList;

protected:
	// Variables

	// True if we're currently loading merchant data into the form fields
	BOOL m_bIsLoadingMerchant;
	// The active/inactive checkbox
	NxButton m_checkActive;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog() override;

	// (c.haag 2015-07-30) - PLID 67187 - Adds a merchant to data and returns the merchant object with an ID
	LPUNKNOWN AddMerchant(const CString& strMerchantName);
	// (c.haag 2015-07-30) - PLID 67187 - Changes the merchant in data
	void SaveMerchant(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// Toggles the enabled status of every control that can be used to modify or delete the selected merchant
	void EnableSelectedMerchantControls(BOOL bEnable);
	// Loads the selected mechant to the form
	void LoadSelectedMerchAccount();
	// Moves a window from its present position by delta values
	void PushWindow(CWnd* pWnd, const CPoint& ptDelta); // (c.haag 2015-07-30) - PLID 67187
	// (c.haag 2015-07-30) - PLID 67187 - Hides the Integrated Credit Card Processing window
	void HideEnableICCPWindow();
	// (c.haag 2015-07-30) - PLID 67187
	void SelChangingMerchAccountList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenMerchAccountList(LPDISPATCH lpRow);

	// (z.manning 2015-09-04 08:50) - PLID 67236
	BOOL TestEmv();
	BOOL CheckForDeviceConnection();
	BOOL CheckForDeviceConnection(const CString &strNoDeviceMessageOverride);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBnClickedBtnAddmerchant();
	afx_msg void OnBnClickedClose();
	afx_msg void OnBnClickedBtnRenamemerchant();
	afx_msg void OnBnClickedBtnDeletemerchant();
//	afx_msg void OnEnChangeMerchId();
	afx_msg void OnBnClickedCheckActive();
	afx_msg void OnBnClickedBtnEnableICCP();
	afx_msg void OnBnClickedBtnTestEmv();
	// (z.manning 2015-08-11 12:40) - PLID 67235
	afx_msg void OnBnClickedBtnInstallIngenicoDriver();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnEnKillfocusMerchId();
	// (z.manning 2015-09-30 11:10) - PLID 67255
	afx_msg void OnBnClickedBtnIccpAdvSettings();
};
