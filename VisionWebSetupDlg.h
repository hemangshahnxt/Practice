#pragma once

// CVisionWebSetupDlg dialog
// (a.vengrofski 2010-09-20 11:44) - PLID <40541> - Created.

class CVisionWebSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CVisionWebSetupDlg)

public:
	CVisionWebSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CVisionWebSetupDlg();

// Dialog Data

	enum { IDD = IDD_INV_VISIONWEB_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	void OnCancel();
	BOOL bIsProduction;
	
	NXDATALIST2Lib::_DNxDataListPtr m_VisionWebSupplierList;
	NXDATALIST2Lib::_DNxDataListPtr m_VisionWebSupplierCombo;
	
	CNxIconButton	m_CloseBtn;
	CNxIconButton	m_AddSupplierBtn;
	CNxIconButton	m_SetupVisionWebURLBtn;
	CNxIconButton	m_SupplierCatalogBtn;
	
	CNxEdit  m_nxeUserID;
	CNxEdit  m_nxePassword;
	CNxEdit  m_nxeSupplierCode;
	BOOL bUsernameChanged,bPasswordChanged;
	// (s.dhole 2011-02-15 10:23) - PLID 40535 added OnRButtonDownVisionwebSupplierList to call popup menu on right click event of datalist
	afx_msg void OnBtnClose();
	afx_msg void OnCatalogSetup();
	afx_msg void OnRemoveSupplier();
	afx_msg void OnEnKillfocusVisionwebUsername();
	afx_msg void OnEnKillfocusVisionwebPassword();
	afx_msg void OnEnKillfocusSupplierCode();
	afx_msg void OnRequeryFinishedSupplierCombo(short nFlags);
	afx_msg void OnSelChosenSupplierCombo(LPDISPATCH lpRow);
	afx_msg void OnBtnClickedAddSupplier();
	afx_msg void OnBtnClickedVisionWebUrlSetup();
	void EditingFinishingVisionwebSupplierList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	afx_msg void OnRButtonDownVisionwebSupplierList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
private:
	
	void Save();
	void SaveSupplier(long nSupplierID, LPCTSTR VisionWebCode);
	
	CString m_strVisionWebUserID, m_strVisionWebPassword;

public:

	
	
	void SelChangingVisionwebSupplierList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnBnClickedBtnSupplierCatalog();
	void SelChangedVisionwebSupplierList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
};
