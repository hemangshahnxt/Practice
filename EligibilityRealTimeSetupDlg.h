#pragma once

// CEligibilityRealTimeSetupDlg dialog

// (j.jones 2010-07-02 14:22) - PLID 39506 - created

#include "FinancialRc.h"
#include "NxAPI.h"

class CEligibilityRealTimeSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEligibilityRealTimeSetupDlg)

public:
	CEligibilityRealTimeSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CEligibilityRealTimeSetupDlg();

// Dialog Data
	enum { IDD = IDD_ELIGIBILITY_REALTIME_SETUP_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	NxButton	m_checkEnableRealTime;
	NxButton	m_checkOpenNotepad;
	NxButton	m_radioUnbatch;
	NxButton	m_radioUnselect;
	CNxEdit		m_nxeditSiteID;
	CNxEdit		m_nxeditPassword;
	NxButton	m_radioGlobalLogin;
	NxButton	m_radioIndivLogin;
	// (j.jones 2010-10-21 13:08) - PLID 40914 - added clearinghouse selection
	NxButton	m_radioTrizetto;
	NxButton	m_radioECP;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	NXDATALIST2Lib::_DNxDataListPtr m_ProvCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_LocCombo;

	NexTech_Accessor::_ClearinghouseLoginPtr m_pCurrentLogin;
	//long m_nCurProviderID;
	//long m_nCurLocationID;
	BOOL m_bLoginInfoChanged;

	bool ValidateBeforeSaving();
	BOOL Save();
	void ValidateAfterSaving();
	void SaveLoginType();
	BOOL SaveLogin();
	void Load();

	bool UseGlobalLogin();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnOk();
	afx_msg void OnCheckEnableRealtimeSends();
	NexTech_Accessor::_ClearinghouseLoginFilterPtr CreateLoginFilterFromControls();
	afx_msg void OnBnClickedRadioTrizettoLogin();
	void LoadLogin();
	DECLARE_EVENTSINK_MAP()
	void OnSelChangingEligRealtimeProvCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChangingEligRealtimeLocCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChosenEligRealtimeProvCombo(LPDISPATCH lpRow);
	void OnSelChosenEligRealtimeLocCombo(LPDISPATCH lpRow);
	afx_msg void OnEnChangeEditTrizettoSiteId();
	afx_msg void OnEnChangeEditTrizettoPassword();
public:
	afx_msg void OnBnClickedRadioTrizetto();
	afx_msg void OnBnClickedRadioEcp();
};
