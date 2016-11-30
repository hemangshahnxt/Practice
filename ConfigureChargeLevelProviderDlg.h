#pragma once
#include "GlobalFinancialUtils.h"
#include <NxUILib/NxStaticIcon.h>

//(r.gonet 05/19/2014) - PLID 61832 - Created.
// CConfigureChargeLevelProvider dialog

///<summary>Allows configuration of charge level claim providers.</summary>
class CConfigureChargeLevelProviderDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CConfigureChargeLevelProviderDlg)

public:
	///<summary>What kind of ServiceT record are we displaying. It is mutual exclusive and should depend on where we call this dialog from.</summary>
	enum class EServiceType {
		CPTCode,
		Product,
	};
private:
	///<summary>These are special rows for the upper criteria combos: Service Code/Products, Provider, Location, and HCFA Group.</summary>
	enum class ECriteriaSpecialIds {
		All = -3,
		Multiple = -2,
		None = -1,
	};

	///<summary>Columns for the Provider combo.</summary>
	enum class EProviderComboColumns {
		ID = 0,
		Name,
	};

	///<summary>Columns for the Location combo.</summary>
	enum class ELocationComboColumns {
		ID = 0,
		Name,
	};

	///<summary>Columns for the HCFA Group combo.</summary>
	enum class EHCFAGroupComboColumns {
		ID = 0,
		Name,
	};

	///<summary>Columns for the Service Code combo.</summary>
	enum class EServiceCodeComboColumns {
		ID = 0,
		Code,
		Description,
		DisplayColumn,
	};

	///<summary>Columns for the Product combo.</summary>
	enum class EProductComboColumns {
		ID = 0,
		Name,
	};

	///<summary>Columns for the Referring Provider, Ordering Provider, and Supervising Provider combos. All have the same columns and that is unlikely to ever change.</summary>
	enum class ECommonProviderComboColumns {
		ID = 0,
		Name,
	};

public:
	CConfigureChargeLevelProviderDlg(CWnd* pParent, long nServiceID, EServiceType eServiceType);   // standard constructor
	virtual ~CConfigureChargeLevelProviderDlg();

// Dialog Data
	enum { IDD = IDD_CONFIGURE_CHARGE_LEVEL_PROVIDERS };

	CNxColor m_nxcolorBackground;

	// (r.gonet 05/19/2014) - PLID 62248 - Upper criteria section
	CNxStaticIcon m_icoToolTipCriteriaSection;

	CNxStatic m_nxstaticProviders;
	CNxLabel m_nxlMultiProviders;
	NXDATALIST2Lib::_DNxDataListPtr m_pProviderCombo;
	
	CNxStatic m_nxstaticLocations;
	CNxLabel m_nxlMultiLocations;
	NXDATALIST2Lib::_DNxDataListPtr m_pLocationCombo;

	CNxStatic m_nxstaticHCFAGroups;
	CNxLabel m_nxlMultiHCFAGroups;
	NXDATALIST2Lib::_DNxDataListPtr m_pHCFAGroupCombo;

	CNxStatic m_nxstaticServiceCodes;
	CNxLabel m_nxlMultiServiceCodes;
	NXDATALIST2Lib::_DNxDataListPtr m_pServiceCodeCombo;

	CNxStatic m_nxstaticProducts;
	CNxLabel m_nxlMultiProducts;
	NXDATALIST2Lib::_DNxDataListPtr m_pProductCombo;

	// (r.gonet 05/19/2014) - PLID 62249 - Lower Settings section
	CNxStaticIcon m_icoToolTipSettingsSection;

	NxButton m_checkReferringProvider;
	NXDATALIST2Lib::_DNxDataListPtr m_pReferringProviderCombo;
	NxButton m_checkOrderingProvider;
	NXDATALIST2Lib::_DNxDataListPtr m_pOrderingProviderCombo;
	NxButton m_checkSupervisingProvider;
	NXDATALIST2Lib::_DNxDataListPtr m_pSupervisingProviderCombo;

	CNxIconButton m_btnApply;
	CNxIconButton m_btnClose;

protected:
	///<summary>If we are displaying the all row for the Providers combo.</summary>
	///<remarks>It may seem redundant but there is no other way to get the previous selection in SelChosen other than storing it.</remarks>
	bool m_bAllProvidersSelected = true;
	///<summary>The IDs of the currently selected Providers.</summary>
	CArray<long, long> m_arySelectedProviders;
	///<summary>If we are displaying the all row for the Locations combo.</summary>
	bool m_bAllLocationsSelected = true;
	///<summary>The IDs of the currently selected Locations.</summary>
	CArray<long, long> m_arySelectedLocations;
	///<summary>If we are displaying the all row for the HCFA Groups combo.</summary>
	bool m_bAllHCFAGroupsSelected = true;
	///<summary>The IDs of the currently selected HCFA Groups.</summary>
	CArray<long, long> m_arySelectedHCFAGroups;
	///<summary>What kind of ServiceT records are we displaying.</summary>
	EServiceType m_eServiceType;
	///<summary>The IDs of the currently selected ServiceT records. May be CPTCodeT records or ProductT records.</summary>
	CArray<long, long> m_arySelectedServiceCodes;
	///<summary>Causes an exception in OnSetCursor to only show once. Otherwise we would be destroyed with all the exceptions following the first.</summary>
	bool m_bNotifyOnce = true;	

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	
	void SelectMultiProviders();
	void SelectMultiLocations();
	void SelectMultiHCFAGroups();
	void SelectMultiServiceCodes();
	void SelectMultiProducts();
	
	CString GetLowerProviderComboFromSql();
	CString GetReferringProviderComboWhereSql();
	CString GetOrderingProviderComboWhereSql();
	CString GetSupervisingProviderComboWhereSql();

	void HandleMultipleSelection(
		CNxLabel &nxstaticMultiLabel, NXDATALIST2Lib::_DNxDataListPtr pCombo, UINT nComboControlID, CArray<long, long> &aryCurrentSelections,
		bool *pbAllSelected, CString strConfigRTName, CString strDescription,
		short nIDColumnIndex = 0, short nDescriptionColumnIndex = 1, CArray<short, short> *paryExtraColumnIndices = NULL);	
	void EnsureControls();
	// (r.gonet 05/19/2014) - PLID 61833 
	void InitializeToolTipText();
	void ReloadSettings();
	ChargeClaimProviderSettings GetSettingsFromData();
	void Save();
	ChargeClaimProviderSettings GetSettingsFromUI();
	void GetCriteriaSelections(long &nProviderID, long &nLocationID, long &nHCFAGroupID, long &nServiceCodeID);
	void GetSettingSelections(long &nReferringProviderID, long &nOrderingProviderID, long &nSupervisingProviderID);

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void SelChosenCclpProviderCombo(LPDISPATCH lpRow);
	void SelChosenCclpLocationCombo(LPDISPATCH lpRow);
	void SelChosenCclpHcfaGroupCombo(LPDISPATCH lpRow);
	void SelChosenCclpServiceCodeCombo(LPDISPATCH lpRow);
	void SelChosenCclpProductCombo(LPDISPATCH lpRow);
	void RequeryFinishedCclpProviderCombo(short nFlags);
	void RequeryFinishedCclpLocationCombo(short nFlags);
	void RequeryFinishedCclpSupervisingProviderCombo(short nFlags);
	void RequeryFinishedCclpHcfaGroupCombo(short nFlags);
	void RequeryFinishedCclpServiceCodeCombo(short nFlags);
	void RequeryFinishedCclpProductCombo(short nFlags);
	void AddSpecialColummnsToProviderCombos(NXDATALIST2Lib::_DNxDataListPtr pCombo);
	void RequeryFinishedCclpReferringProviderCombo(short nFlags);
	void RequeryFinishedCclpOrderingProviderCombo(short nFlags);	
	afx_msg void OnBnClickedCclpReferringProviderCheck();
	afx_msg void OnBnClickedCclpOrderingProviderCheck();
	afx_msg void OnBnClickedCclpSupervisingProviderCheck();
	void SelChosenCclpReferringProviderCombo(LPDISPATCH lpRow);
	void SelChosenCclpOrderingProviderCombo(LPDISPATCH lpRow);
	void SelChosenCclpSupervisingProviderCombo(LPDISPATCH lpRow);
	afx_msg void OnBnClickedCclpApplyBtn();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEnsureControls(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};
