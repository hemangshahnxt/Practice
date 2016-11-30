#pragma once

// CNewCropSetupDlg dialog

// (j.jones 2009-02-27 12:03) - PLID 33265 - created

class CNewCropSetupDlg : public CNxDialog
{
public:
	CNewCropSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CNewCropSetupDlg();

// Dialog Data
	enum { IDD = IDD_NEWCROP_SETUP_DLG };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	NxButton	m_radioPreProduction;
	NxButton	m_radioProduction;
	// (j.jones 2011-03-07 11:50) - PLID 42313 - added an option to not send patient gender
	NxButton	m_checkSuppressPatGender;
	// (j.jones 2011-06-17 08:35) - PLID 44157 - moved the name overrides to a new dialog
	CNxIconButton m_btnSetupNameOverrides;

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_LocationCombo;

	// (j.gruber 2009-03-31 09:27) - PLID 33328 - added user type datalist
	NXDATALIST2Lib::_DNxDataListPtr m_pUserTypeList;

	// (j.jones 2011-06-17 09:49) - PLID 41709 - added provider role datalist
	NXDATALIST2Lib::_DNxDataListPtr m_pProviderRoleList;

	// (j.jones 2011-06-17 11:29) - PLID 41709 - added a function to re-generate
	// the embedded combo boxes for each provider dropdown column, while
	// maintaining their existing selections
	void RequeryProviderComboColumns();

	//tracks what the production status was when we opened the screen
	// (j.jones 2010-06-09 11:04) - PLID 39013 - production status is read-only now
	//BOOL m_bWasProduction;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnOk();
	DECLARE_EVENTSINK_MAP()
	// (j.jones 2009-06-08 09:39) - PLID 34514 - added OnEditingFinishedNewcropSetupUserList
	void OnEditingFinishedNewcropSetupUserList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	// (j.gruber 2009-06-10 11:56) - PLID 34515 - don't let them edit the provider column if its not licensed prescriber
	void EditingStartingNewcropSetupUserList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	// (j.jones 2011-06-17 08:35) - PLID 44157 - moved the name overrides to a new dialog
	afx_msg void OnBtnNewcropOverrideNames();
	// (j.jones 2011-06-17 10:05) - PLID 41709 - added provider role list
	afx_msg void OnEditingStartingProviderRoleList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	afx_msg void OnEditingFinishedProviderRoleList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
};