#if !defined(FAXSERVICESETUPDLG_H)
#define FAXSERVICESETUPDLG_H

#pragma once
#include "practiceRc.h"
#include "WebFaxUtils.h"
// CFaxServiceSetupDlg dialog

//(e.lally 2011-03-18) PLID 42767 - Created

class CFaxServiceSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CFaxServiceSetupDlg)

public:
	CFaxServiceSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CFaxServiceSetupDlg();

// Dialog Data
	enum { IDD = IDD_FAX_SERVICE_SETUP_DLG };
protected:
	CNxEdit	m_editName;
	CNxEdit	m_editUser;
	CNxEdit	m_editPassword;
	CNxEdit	m_editTargetDirectory;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnRemove;
	NxButton		m_btnEnableChk;
	CNxIconButton	m_btnBrowse;


	//Datalist interface members
	NXDATALIST2Lib::_DNxDataListPtr m_pServiceList;
	NXDATALIST2Lib::_DNxDataListPtr m_pResolutionList;
	NXDATALIST2Lib::_DNxDataListPtr m_pConfigList;

	//This is the currently selected ID.  Required to maintain because OnSelChosen doesn't give us
	//	the previous selection.
	long m_nCurrentLoadedID;

	//Helper functions
	void EnsureControls();
	void SyncInterfaceToDatalist();
	void SyncDatalistToInterface();
	bool SaveData();
	long GetNewID();
	bool ValidateCurrentConfig();
	bool HasCurrentConfigChanged();
	void HandleSelChosenFaxServiceConfigList();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangingFaxServiceServiceList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenFaxServiceServiceList(LPDISPATCH lpRow);
	afx_msg void OnFaxServiceConfigAdd();
	afx_msg void OnFaxServiceConfigRemove();
	afx_msg void OnSelChosenFaxServiceConfigList(LPDISPATCH lpRow);
	afx_msg void OnChangeFaxServiceConfigName();
	afx_msg void OnSelChangingFaxServiceConfigList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnBnClickedFaxServiceBrowseBtn();
	afx_msg void OnBnClickedFaxServiceEnableCheck();
	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()

};

#endif