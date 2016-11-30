#pragma once

//TES 9/15/2011 - PLID 45523 - Created
// CHL7GeneralSettingsDlg dialog

class CHL7GeneralSettingsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CHL7GeneralSettingsDlg)

	enum ELogLevelColumns
	{
		ellcID = 0,
		ellcLevel,
	};

	// (r.gonet 10/31/2011) - PLID 45367
	// (r.gonet 01/15/2013) - PLID 54287 - Added a location column.
	enum EOverrideFacilityCodeColumns
	{
		eofccID = 0,
		eofccLocationID,
		eofccProviderID,
		eofccSendingFacilityCode,
		eofccReceivingFacilityCode,
		eofccModified,
	};

public:
	CHL7GeneralSettingsDlg(CWnd* pParent);   // standard constructor
	virtual ~CHL7GeneralSettingsDlg();

	//TES 9/15/2011 - PLID 45523 - Set by our owner
	long m_nHL7GroupID;
	// (r.gonet 10/31/2011) - PLID 45367 
	CArray<long, long> m_aryDeletedOverrideFacilityCodes;

// Dialog Data
	enum { IDD = IDD_HL7_GENERAL_SETTINGS_DLG };

protected:
	
	CNxIconButton m_nxbOK, m_nxbCancel;
	CNxEdit m_nxeSendingFacility, m_nxeReceivingApplication, m_nxeReceivingFacility, m_nxeSendingApplication;
	NxButton m_nxbForceFacilityIDMatch, m_nxbTrimSeconds, m_nxbPrependTimestamp;
	NXDATALIST2Lib::_DNxDataListPtr m_pLogLevelCombo;
	// (r.gonet 10/31/2011) - PLID 45367 
	CNxStatic m_nxsOverrideFacilityCodeLabel;
	// (r.gonet 10/31/2011) - PLID 45367 - Allow the user to set a provider specific facility code
	NXDATALIST2Lib::_DNxDataListPtr m_pOverrideFacilityCodeList;
	CNxIconButton m_nxbAddOverride;
	CNxIconButton m_nxbRemoveOverride;
	// (b.savon 2014-12-09 14:36) - PLID 64318 - Add a new text box to the HL7 configuration that will be used to override the version of HL7 messages.
	CNxEdit m_nxeVersionOverride;

	//TES 9/15/2011 - PLID 45523 - Disable the ForceFacilityIDMatch checkbox if the FacilityID is empty
	void ReflectSendingFacility();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeSendingFacility();
	// (r.gonet 10/31/2011) - PLID 45367
	afx_msg void OnBnClickedAddOverrideFacilityCodeBtn();
	// (r.gonet 10/31/2011) - PLID 45367
	afx_msg void OnBnClickedRemoveOverrideFacilityCodeBtn();
private:
	DECLARE_EVENTSINK_MAP()
	// (r.gonet 10/31/2011) - PLID 45367
	void SelChangedFacilityCodeOverrideList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	// (r.gonet 10/31/2011) - PLID 45367
	void EnsureControls();
public:
	// (r.gonet 01/15/2013) - PLID 54287
	void EditingFinishingFacilityCodeOverrideList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	// (r.gonet 10/31/2011) - PLID 45367
	void EditingFinishedFacilityCodeOverrideList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	// (r.gonet 10/31/2011) - PLID 45367
	afx_msg void OnBnClickedForceFacilityIdMatch();
	// (r.gonet 05/01/2014) - PLID 61842 - Don't let the user select nothing
	void SelChangingLogLevelCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
};
