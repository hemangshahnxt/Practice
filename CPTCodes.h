#pragma once
// CPTCodes.h : header file
//

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_MOD_ADD				33767
#define ID_MOD_DELETE			33768
#define ID_ICD_ADD				33769
#define ID_ICD_DELETE			33770
#define ID_ICD_MARK_INACTIVE	33771
#define ID_ICD_MAKE_DEFAULT		33772
#define ID_ICD_REMOVE_DEFAULT	33773
#define ID_ICD_FILTER			33774
#define ID_ICD_UNFILTER			33775
#define ID_MOD_MARK_INACTIVE	33776
#define ID_ICD_UPDATE_FROM_FDB	33778 //TES 5/14/2013 - PLID 56631
/////////////////////////////////////////////////////////////////////////////
// CCPTCodes dialog

//DRT 4/2/2008 - PLID 29518 - Converted all edit controls to CNxEdit


enum CCDAProcedureType;	// (j.armen 2014-03-05 17:52) - PLID 61207 - Forward Declare

class CCPTCodes : public CNxDialog
{
// Construction
public:

	enum DiagDatalist
	{
		DiagCodes,
		CustomNexGEMs
	};

	BOOL m_bIsSavingCPT;

	CCPTCodes(CWnd* pParent);   // standard constructor
	BOOL	m_IsModal;
	BOOL    bCanWrite;
	virtual void	StoreDetails();
	virtual void Refresh();

// Dialog Data
	enum { IDD = IDD_CPT_CODES };
	CNxEdit	m_editCategory;
	CNxEdit	m_editBilling;
	//CNxEdit	m_editAnesthBaseUnits;
	CNxEdit m_editTOS;
	CNxEdit	m_editBarcode;
	CNxEdit	m_editGlobalPeriod;
	CNxEdit	m_editRVU;

	CNxEdit	m_editStdFee;
	CNxEdit	m_editShopFee;
	CNxEdit	m_editDescription;
	CNxIconButton	m_btnProcedureDescription;
	CNxIconButton	m_btnServiceModifierLinking;
	//CNxIconButton	m_btnSetupICD9v3;
	CNxIconButton	m_btnDiscountCategories;
	//NxButton	m_checkUseFacilityBilling;//s.tullis
	//NxButton	m_checkUseAnesthesiaBilling;//s.tullis
	//CNxIconButton	m_btnFacilityFeeSetup;//s.tullis
	//NxButton	m_checkFacility;//s.tulli
	CNxIconButton	m_btnConfigFinanceCharges;
	CNxIconButton	m_btnUpdateTOS;
	//CNxIconButton	m_btnAnesthesiaSetup;//s.tullis
	CNxIconButton	m_btnImportAMA;
	CNxIconButton	m_btnLaunchCodeLink;
	CNxIconButton	m_btnRemoveCategory;
	CNxIconButton	m_btnSelectCategory;
	// (r.gonet 02/20/2014) - PLID 60778 - Renamed the member variable to remove the reference to ICD9
	CNxIconButton	m_btnCPTDiagnosisLinking;
	CNxIconButton	m_btnImportCPT;
	//NxButton	m_checkAnesthesia;//s.tullis
	CNxIconButton	m_btnEditMultipleRevCodes;
	NxButton	m_checkMultipleRevCodes;
	NxButton	m_checkSingleRevCode;
	//CNxIconButton	m_btnAdvRevCodeSetup;
	CNxIconButton	m_btnAdditionalServiceCodeSetup;// (s.tullis 2014-05-01 15:29) - PLID 61939 - Add “Additional Service Code Setup” button to the bottom of the CPT Configuration screen.
	// (j.gruber 2010-08-03 10:32) - PLID 39944 - remove promptforcopay
	//NxButton	m_checkPromptForCoPay;
	CNxIconButton	m_btnConfigBillColumns;
	CNxIconButton	m_filtericd9Button;
	CNxIconButton	m_filterCptButton;
	CNxIconButton	m_btnUpdateCats;
	CNxIconButton	m_ReceiptConfig;
	CNxIconButton	m_btnInactiveICD9s;
	CNxIconButton	m_btnMarkICD9Inactive;
	CNxIconButton	m_btnUpdateFees;
	CNxIconButton	m_btnCptRight;
	CNxIconButton	m_btnCptLeft;
	CNxIconButton	m_btnInactiveCPTs;
	CNxIconButton	m_btnPayCats;
	CNxIconButton	m_markInactiveButton;
	CNxIconButton	m_deleteCPTButton;
	CNxIconButton	m_deleteModifierButton;
	CNxIconButton	m_deleteICD9Button;
	CNxIconButton	m_addModifierButton;
	CNxIconButton	m_addicd9Button;
	CNxIconButton	m_addCptButton;
	CNxIconButton m_addCustomMappingButton; // (c.haag 2015-05-14) - PLID 66020
	NxButton	m_Taxable2;
	NxButton	m_Taxable1;
	CNxEdit			m_CPTSubCode;
	CNxEdit			m_CPTCode;
	CNxLabel		m_nxlAMA;
	CNxIconButton	m_btnMarkModifierInactive;
	CNxIconButton	m_btnInactiveCodes;
	//CNxStatic	m_nxstaticUb92CptLabel;//s.tullis
	CNxStatic	m_nxstaticShopFeeLabel;
	//CNxStatic	m_nxstaticIcd9v3Label;//s.tullis
	// (j.jones 2009-03-30 16:56) - PLID 32324 - added m_btnOHIPPremiumCodes
	CNxIconButton	m_btnOHIPPremiumCodes;//s.tullis
	// (j.jones 2010-07-30 10:35) - PLID 39728 - added support for pay groups
	CNxIconButton	m_btnEditPayGroups;
	// (j.jones 2010-08-02 14:32) - PLID 39912 - added ability to edit adv. pay group settings
	CNxIconButton	m_btnAdvPayGroupSetup;
	// (j.jones 2010-09-29 15:08) - PLID 40686 - added ability to batch even with zero
	//NxButton	m_checkBatchWhenZero;//s.tullis
	// (j.jones 2010-11-19 16:25) - PLID 41544 - supported assisting codes
	//NxButton	m_checkAssistingCode;
	//CNxEdit		m_editAssistingBaseUnits;
	// (j.jones 2011-03-28 09:18) - PLID 43012 - added Billable checkbox
	NxButton	m_checkBillable;
	CNxIconButton	m_btnAdvBillableCPTSetup;
	// (j.jones 2011-07-06 15:57) - PLID 44358 - added UB Box 4
	//CNxEdit		m_editUBBox4; //s.tullis comment out later
	// (j.jones 2012-01-03 17:04) - PLID 47300 - added NOC label
	//CNxLabel	m_nxlNOC;
	// (r.gonet 03/26/2012) - PLID 49043 - Added a per CPT code Require Referring Physician checkbox
	//NxButton m_checkRequireRefPhys;
	// (a.wilson 2014-5-5) PLID 61831 - remove the old ServiceT.SendOrderingPhy field code.
	// (r.gonet 03/06/2014) - PLID 61129 - Removed ability to import from FDB and related controls
	NxButton m_checkDiagICD9;	// (j.armen 2014-03-05 17:52) - PLID 61207
	NxButton m_checkDiagICD10;	// (j.armen 2014-03-05 17:52) - PLID 61207
	NxButton m_checkDiagCustomNexGEMs; // (c.haag 2015-05-11) - PLID 66017

	NXDATALISTLib::_DNxDataListPtr	m_pCodeNames;
	NXDATALISTLib::_DNxDataListPtr	m_pModifiers;
	NXDATALISTLib::_DNxDataListPtr	m_pDiagCodes;
	//NXDATALISTLib::_DNxDataListPtr	m_UB92_Category;// s.tullis comment out later
	NXDATALISTLib::_DNxDataListPtr m_DefaultProviderCombo;
	// (d.singleton 2011-09-21 14:37) - PLID 44946
	NXDATALISTLib::_DNxDataListPtr m_pAlbertaModifiers;

	// (a.walling 2007-06-20 12:01) - PLID 26413
	//NXDATALIST2Lib::_DNxDataListPtr m_ICD9V3List;// s.tullis comment this out later
	// (j.jones 2010-07-30 10:35) - PLID 39728 - added support for pay groups
	NXDATALIST2Lib::_DNxDataListPtr m_PayGroupCombo;
	// (j.jones 2012-01-03 15:08) - PLID 47300 - added NOC combo
	//NXDATALIST2Lib::_DNxDataListPtr m_NOCCombo;
	// (c.haag 2015-05-11) - PLID 66017 - User-created custom crosswalks
	NXDATALIST2Lib::_DNxDataListPtr	m_pDiagCodeCustomNexGEMs;
	// (c.haag 2015-05-14) - PLID 66019 - Diagnosis searches for custom crosswalks
	NXDATALIST2Lib::_DNxDataListPtr m_pDiag9Search;
	NXDATALIST2Lib::_DNxDataListPtr m_pDiag10Search;
	class CDiagSearchConfig* m_pICD9WithPCSSearchConfig;
	class CDiagSearchConfig* m_pICD10WithPCSSearchConfig;

	// (a.wilson 2014-01-13 16:58) - PLID 59956 - new member variables for additional info dialog.
	CString m_strTOS;
	CCDAProcedureType m_eCCDAType;

	BOOL m_bEditChanged;

	COleCurrency m_cyShopFee;// (j.gruber 2012-10-19 13:15) - PLID 53240 - shop fee

private:
	// (c.haag 2015-05-14) - PLID 66019 - The internal ID of the ICD-9 code that appears in the
	// read-only box next to the ICD-9 search control under "Configure NexGEMs"
	long m_nICD9IDForCustomNexGEM;
	// (c.haag 2015-05-14) - PLID 66019 - The internal ID of the ICD-10 code that appears in the
	// read-only box next to the ICD-9 search control under "Configure NexGEMs"
	long m_nICD10IDForCustomNexGEM;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCPTCodes)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_CurServiceID;
	void Load();
	bool Save ();
	bool m_bRequeryFinished;
	BOOL IsDefaultICDSelected();
	void SetAsDefault();
	void RemoveDefault();
	void UnfilterIcd9();
	void UnfilterCPT();
	void OnUpdatePricesByPercentage();
	void OnUpdatePricesByRVU();
	void OnUpdatePricesFromFile();
	void PopupAMACopyright();
	void ImportCodes(int nType);

	void CheckEnableStandardFee();

	// (j.jones 2007-10-15 10:41) - PLID 27757 - unified this code into one function
	// (j.jones 2010-11-19 16:38) - PLID 41544 - renamed to include assisting codes
	void CheckEnableAnesthesiaFacilityAssistingControls();

	// (j.jones 2010-11-19 17:02) - PLID 41544 - hides assisting code info. if not OHIP
	// or if Anesthesia is checked
	void CheckDisplayAssistingCodeControls();

	void SecureControls();

	// (j.jones 2012-07-24 09:00) - PLID 51651 - Added a preference to only track global periods for
	// surgical codes only, this will control whether the edit box is enabled for all codes or not.
	// Passes in IsSurgicalCode (VT_BOOL) if we already know that information.
	void ReflectGlobalPeriodReadOnly(_variant_t varIsSurgicalCode = g_cvarNull);

	// (c.haag 2015-05-11) - PLID 66017 - Shows or hides the diagnosis list or diagnosis custom crosswalks
	void ShowDiagDatalist(DiagDatalist listToShow);

	// (c.haag 2015-05-11) - PLID 66017 - Refreshes the custom NexGEMs list if it's visible
	void RefreshCustomNexGEMsList();

	// (c.haag 2015-05-14) - PLID 66020 - Adds a custom NexGEM to the list
	void AddCustomNexGEMToList(LPDISPATCH lpDiagResult);

	// (c.haag 2015-05-14) - PLID 66020 - Updates the enabled state of the add custom mapping button
	void UpdateAddCustomMappingButton();

	// Generated message map functions
	//{{AFX_MSG(CCPTCodes)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnKillfocusCPTCode();
	afx_msg void OnKillfocusCPTSubcode();
	afx_msg void OnKillfocusStdFee();
	virtual void OnOK();
	afx_msg void OnPaymentCategory();
	virtual void OnCancel();
	afx_msg void OnAdditionalServiceCodeSetup();// (s.tullis 2014-05-01 15:47) - PLID 61939 - Add “Additional Service Code Setup” button to the bottom of the CPT Configuration screen.
	afx_msg void OnRButtonDownCptModifiers(long nRow, long nCol, long x, long y, long nFlags);
	afx_msg void OnMenuAddModifier();
	afx_msg void AddModifier();
	afx_msg void OnMenuDeleteModifier();
	afx_msg void DeleteModifier();
	afx_msg void OnRButtonDownDiagCodes(long nRow, long nCol, long x, long y, long nFlags);
	afx_msg void OnMenuAddICD9();
	afx_msg void AddICD9();
	afx_msg void OnMenuDeleteICD9();
	afx_msg void DeleteICD9();
	afx_msg void OnKillfocusDescription();
	afx_msg void OnKillfocusRvu();
	afx_msg void OnKillfocusTOS();
	afx_msg void OnEditingFinishedCptModifiers(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingDiagCodes(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedDiagCodes(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingCptModifiers(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);	
	afx_msg void OnRequeryFinishedCptCodes(short nFlags);
	afx_msg void OnKillfocusGlobalPeriod();
	afx_msg void OnLinkcodes();
	afx_msg void OnTaxable1();
	afx_msg void OnTaxable2();
	afx_msg void OnAddCpt();
	afx_msg void OnAddModifier();
	afx_msg void OnDeleteModifier();
	afx_msg void OnAddIcd9();
	afx_msg void OnDeleteIcd9();
	afx_msg void OnDeleteCpt();
	afx_msg void OnMarkInactive();
	afx_msg void OnSelChosenCptCodes(long nRow);
	//afx_msg void OnSelChosenUb92CptCategories(long nRow);// (s.tullis 2014-05-22 09:13) - PLID 61829 - moved
	afx_msg void OnLeftCpt();
	afx_msg void OnRightCpt();
	afx_msg void OnColumnClickingCptCodes(short nCol, BOOL FAR* bAllowSort);
	afx_msg void OnUpdatePrices();
	afx_msg void OnMarkIcd9Inactive();
	afx_msg void OnReceiptConfig();
	afx_msg void OnSelChosenDefaultCPTProvider(long nRow);
	afx_msg void OnRequeryFinishedDiagCodes(short nFlags);
	afx_msg void OnUpdateCategories();
	afx_msg void OnFilterICD9();
	afx_msg void OnFilterCPT();
	afx_msg void OnKillfocusCptBarcode();
	afx_msg void OnConfigBillColumns();
	// (j.gruber 2010-08-03 10:32) - PLID 39944 - remove promptforcopay
	//afx_msg void OnCheckPromptForCopay();
	//afx_msg void OnAdvRevcodeSetup();
	//afx_msg void OnCheckSingleRevCode();
	//afx_msg void OnCheckMultipleRevCodes();
	//afx_msg void OnBtnEditMultipleRevCodes();// (s.tullis 2014-05-22 09:13) - PLID 61829 - moved
	afx_msg void OnCheckAnesthesia();
	afx_msg void OnBtnImportCpt();
	// (r.gonet 02/20/2014) - PLID 60778 - Renamed the function to remove the reference to ICD9
	afx_msg void OnBtnCptDiagnosisLinking();
	afx_msg void OnBtnSelectCategory();
	afx_msg void OnBtnRemoveCategory();
	afx_msg void OnKillfocusShopFee();
	afx_msg void OnBtnLaunchCodelink();
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnBtnImportAma();
	afx_msg void OnEditingStartingCptModifiers(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingStartingDiagCodes(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnBtnAnesthesiaSetup();
	afx_msg void OnKillfocusAnesthBaseUnits();
	afx_msg void OnUpdateTos();
	afx_msg void OnConfigureFinanceChargeSettings();
	afx_msg void OnCheckFacility();
	afx_msg void OnBtnFacilityFeeSetup();
	afx_msg void OnProcedureDescription();
	afx_msg void OnCheckUseAnesthesiaBilling();
	afx_msg void OnCheckUseFacilityBilling();
	afx_msg void OnTrySetSelFinishedCptCodes(long nRowEnum, long nFlags);
	afx_msg void OnMarkModifierInactive(); // (z.manning, 05/01/2007) - PLID 16623
	afx_msg void OnInactiveCodes();	// (z.manning, 05/02/2007) - PLID 16623
	afx_msg void OnBtnDiscountCategories();
	//afx_msg void OnSelChangingListIcd9v3(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);//s.tullis	
	//afx_msg void OnSelChangedListIcd9v3(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);//s.tullis
	//afx_msg void OnBtnSetupIcd9v3();//s.tullis
	// (j.jones 2007-07-03 15:05) - PLID 26098 - added option to link service codes and modifiers
	afx_msg void OnBtnServiceModifierLinking();
	// (j.jones 2009-03-30 16:56) - PLID 32324 - added OnBtnOhipPremiumCodes
	afx_msg void OnBtnOhipPremiumCodes();//s.tullis
	// (s.dhole 2012-04-10 09:41) - PLID 47399
	afx_msg void OnKillfocusDefaultCost();
	// (c.haag 2015-07-08) - PLID 66019
	afx_msg void OnDestroy();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	CPtrArray		TreeList;
	bool			m_bIsAdding;

	CString StripNonNum(CString inStr);

	// (j.jones 2012-01-03 17:04) - PLID 47300 - added NOC label
	//void PopupNOCDescription();

	// (j.jones 2012-04-11 17:11) - PLID 47313 - added ServiceT.ClaimNote
	void EnterClaimNote();

	CTableChecker m_CPTChecker, m_ModChecker, m_ICD9Checker, m_CPTCategoryChecker, m_UB92CatChecker,m_providerChecker,
		m_PayGroupChecker, m_CustomNexGEMsChecker; // (j.jones 2010-07-30 16:15) - PLID 39728
	// (c.haag 2015-05-11) - PLID 66017 - Added check for NexGEMs

	// (j.jones 2010-07-08 14:01) - PLID 39566 - cache the UseOHIP preference
	BOOL m_bUseOHIP;
	// (j.jones 2010-07-30 10:16) - PLID 39728 - added support for pay groups
	afx_msg void OnBtnEditPayGroups();
	void OnSelChosenPayGroupsCombo(LPDISPATCH lpRow);
	// (j.jones 2010-08-02 14:32) - PLID 39912 - added ability to edit adv. pay group settings
	afx_msg void OnBtnAdvPayGroupSetup();
	// (j.jones 2010-09-29 15:08) - PLID 40686 - added ability to batch even with zero
	afx_msg void OnCheckBatchWhenZero();
	// (j.jones 2010-11-19 16:34) - PLID 41544 - supported assisting codes
	afx_msg void OnCheckAssistingCode();
	afx_msg void OnKillfocusAssistingBaseUnits();
	// (j.jones 2011-03-28 09:18) - PLID 43012 - added Billable checkbox
	afx_msg void OnCPTBillable();
	afx_msg void OnBtnEditBillableCodes();
	// (j.jones 2011-07-06 15:57) - PLID 44358 - added UB Box 4
	afx_msg void OnEnKillfocusUbBox4();
	void EditingFinishedAlbertaModifiers(long nRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	// (j.jones 2012-01-03 15:36) - PLID 47300 - added NOC Combo
	void OnSelChosenNocCombo(LPDISPATCH lpRow);
	// (j.jones 2012-01-03 15:36) - PLID 47300 - added NOC Combo
	void OnSelChangingNocCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	// (r.gonet 03/26/2012) - PLID 49043 - Save and audit Require Ref Phys
	afx_msg void OnBnClickedCptRequireRefPhys();
	afx_msg void OnBnClickedCptAdditionalInfoBtn();
	afx_msg void OnBnClickedBtnEditBillableShopFees(); // (j.gruber 2012-10-25 15:51) - PLID 53240
	// (j.jones 2013-06-03 14:23) - PLID 54091 - added setting to send the claim provider
	// as ordering physician when this service is billed
	afx_msg void OnBnClickedCptCodesDiagIcd();	// (j.armen 2014-03-05 17:52) - PLID 61207
	void OnSelChosenICD9DiagSearchList(LPDISPATCH lpRow); // (c.haag 2015-05-14) - PLID 66019
	void OnSelChosenICD10DiagSearchList(LPDISPATCH lpRow); // (c.haag 2015-05-14) - PLID 66019
	afx_msg void OnBnClickedAddCustomMapping(); // (c.haag 2015-05-14) - PLID 66020
	void OnRButtonDownCustomNexGEMsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags); // (c.haag 2015-05-15) - PLID 66021
public:
	afx_msg void OnStnClickedBillingBar();
	afx_msg void OnStnClickedBillingsetup();
};