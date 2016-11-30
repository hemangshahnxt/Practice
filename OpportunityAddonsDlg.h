#pragma once


// COpportunityAddonsDlg dialog

// (d.lange 2010-07-16 17:20) - PLID 39691 - created

////////////////////////
//Current Pricing 
//		This list should be kept in sync with the options in OpportunityPriceStructureT
enum eCurrentAddOnPricing {
	ecapScheduler = 0,
	ecap1License,
	ecapBilling,
	ecapHCFA,
	ecapEBilling,
	ecapLetters,
	ecapQuotes,
	ecapTracking,
	ecapNexForms,
	ecapInventory,
	ecapSpa,
	ecapASC,
	ecapWorkstation,
	ecapDoctor,
	ecapPDA,
	ecapMirror,
	ecapUnited,
	ecapEMRFirstDoctor,
	ecapEMRAddtlDoctor,
	ecapTraining,
	ecapConversion,
	ecapTravel,
	ecapPkgSched,
	ecapPkgFinancial,
	ecapPkgCosmetic,
	ecapHL7,
	ecapInform,
	ecapQuickbooks,
	ecapERemittance,			
	ecapEEligibility,		
	ecapExtraDB,				
	ecapSpecialResident,		
	ecapSpecialStartup,		
	ecapResidentAddOn,		
	ecapStartupAddOn,		
	ecapSchedulerStandard,	
	ecapLettersStandard,		
	ecapEMRFirstStandard,	
	ecapEMRAddtlStandard,	
	ecapPkgSchedStandard,	
	ecapPkgCosmeticStandard,	
	ecapCandA,				
	ecapPkgNexWeb,			
	ecapNexWebLeads,			
	ecapNexWebPortal,		
	ecapNexWebAddtlDomains,	
	ecapNexPhoto,			
	ecapNexSync,				



	//This must be last
	ecapTotalPrices,
};

enum AvailableAddonsColumns {
	aacAddOnID = 0,
	aacAddOnName,
	aacAddOnPrice,
	aacAddOnType,
};

enum AddOnColumns {
	aocAddOnID = 0,
	aocAddOnName,
	aocQuantity,
	aocPrice,
	aocSubTotal,
	aocComments,
};

struct AddOn {
	long nAddOnID;					//This element references the eCurrentAddOnPricing enum index
	CString strAddOnName;			//This element is the name of the AddOn, which is hardcoded
	CString strProposalTColumn;		//This element is the name of the OpportunityProposalsT column
	COleCurrency cyPrice;			//This element is referenced from OpportunityPriceStructureT
	BOOL bMultiple;					//This element is a flag for AddOns that can have multiple quantities 
};

class COpportunityAddonsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(COpportunityAddonsDlg)

public:
	COpportunityAddonsDlg(CWnd* pParent);   // standard constructor
	virtual ~COpportunityAddonsDlg();

	int OpenNewAddOn(long nOpportunityID, long nPatientID, CString strPatientName, long nTypeID);
	int OpenExistingAddOn(long nOpportunityID, long nPatientID, long nExistingAddOnID, CString strPatientName, long nTypeID);

// Dialog Data
	enum { IDD = IDD_OPPORTUNITY_ADDONS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	NXDATALIST2Lib::_DNxDataListPtr m_dlAvailableAddOns;
	NXDATALIST2Lib::_DNxDataListPtr m_dlSelectedAddOns;

	int OpenAddOn(long nAddOnID, long nOpportunityID, long nPatientID, CString strPatientName, long nTypeID);
	void FillPriceArray();				//Fills the Pricing array 
	void CreateAvailableAddOnsCombo();	//Builds the dropdown, displaying the Addon names and prices
	CString GetLineItemSubtotal(long nQuantity, COleCurrency cyPrice);
	CString GetTableColumnName(long nAddOnEnum);
	void UpdateAllTotals();
	void COpportunityAddonsDlg::MergeFields();
	CArray<AddOn*, AddOn*> GetCurrentSelectedAddOns();
	AddOn* GetAddOnFromColumnName(CString strColName);
	void InsertSelectedAddOn(AddOn* addOn, long nQuantity);
	void UpdateSupportCosts();
	BOOL ApplyChanges();
	BOOL IsValidDiscount();
	COleCurrency CalculateCurrentMonthlySupport();
	COleCurrency GetCurrentSubTotalList();

	CArray<COleCurrency, COleCurrency> m_aryPrices;			//Current Pricing
	CArray<AddOn*, AddOn*> m_aryAddOns;
	long m_nID;
	long m_nOpportunityID;
	long m_nPatientID;
	long m_nTypeID;
	long m_nExistingAddOnID;
	long m_nPriceStructure;
	long m_nMailSentID;
	long m_nCurrentDiscountUserID;
	CString m_strPatientName;
	CString m_strUpdateValues;

	CDateTimePicker	m_pickerAddOnDate;
	CDateTimePicker	m_pickerExpireDate;

	CNxIconButton m_btnCancel;
	CNxIconButton m_btnSaveEdit;
	CNxIconButton m_btnMerge;
	CNxIconButton m_btnDelete;
	CNxIconButton m_btnChangeDiscount;
	NxButton m_chkCosmeticBilling;
	NxButton m_chkHCFA;
	NxButton m_chkEBilling;
	NxButton m_chkLetter;
	NxButton m_chkCA;
	NxButton m_chkQuotes;
	NxButton m_chkNexTrak;
	NxButton m_chkNexForms;
	NxButton m_chkInv;
	NxButton m_chkNexSpa;
	CNxEdit m_nxeditSubtotalDiscount;
	CNxEdit m_nxeditSubtotalSel;
	CNxEdit m_nxeditGrandTotal;
	CNxEdit m_nxeditSupportPercent;
	CNxEdit m_nxeditSupportNum;
	CNxEdit m_nxeditSupportTotal;
	CNxEdit m_nxeditDiscountUsername;
	CNxEdit m_nxeditDiscountPercent;
	CNxEdit m_nxeditDiscountTotal;
	CNxEdit m_nxeditEMRTraining;
	CNxEdit m_nxeditPMTraining;
	CNxEdit m_nxeditTrainingTotal;
	CNxEdit m_nxeditTravelNum;
	CNxEdit m_nxeditTravelTotal;
	CNxLabel m_labelSupportMonthly;
	CNxLabel m_labelTraining;

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void SelChosenAvailableAddonsList(LPDISPATCH lpRow);
	void EditingFinishedSelectedAddonsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnBnClickedBtnDeleteAddon();
	afx_msg LRESULT OnLoadOpportunityAddOn(WPARAM wParam, LPARAM lParam);
	void LeftClickSelectedAddonsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void SelSetSelectedAddonsList(LPDISPATCH lpSel);
	afx_msg void OnBnClickedBtnSaveEdit();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedBtnChangeDiscount();
	afx_msg void OnBnClickedBtnMergeAddon();
	afx_msg void OnEnChangeSupportNumAddon();
};
