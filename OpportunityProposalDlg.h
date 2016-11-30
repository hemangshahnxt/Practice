#if !defined(AFX_OPPORTUNITYPROPOSALDLG_H__D46D8A3B_ED97_4FF9_B421_F3E4B98F0008__INCLUDED_)
#define AFX_OPPORTUNITYPROPOSALDLG_H__D46D8A3B_ED97_4FF9_B421_F3E4B98F0008__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OpportunityProposalDlg.h : header file
//


////////////////////////
//Current Pricing 
//		This list should be kept in sync with the options in OpportunityPriceStructureT
enum eCurrentPrice {
	ecpScheduler = 0,
	ecp1License,
	ecpBilling,
	ecpHCFA,
	ecpEBilling,
	ecpLetters,
	ecpQuotes,
	ecpTracking,
	ecpNexForms,
	ecpInventory,
	ecpSpa,
	ecpASC,
	ecpWorkstation,
	ecpDoctor,
	ecpPDA,
	ecpMirror,
	ecpUnited,
	ecpEMRFirstDoctor,
	ecpEMRAddtlDoctor,
	ecpTraining,
	ecpConversion,
	ecpTravel,
	ecpPkgSched,
	ecpPkgFinancial,
	ecpPkgCosmetic,
	ecpHL7,
	ecpHIE,					// (d.thompson 2013-07-08) - PLID 57334
	ecpInform,
	ecpQuickbooks,
	ecpERemittance,			//DRT 2/11/2008 - PLID 28881
	ecpEEligibility,		//DRT 2/11/2008 - PLID 28881
	ecpExtraDB,				//DRT 2/13/2008 - PLID 28905
	ecpSpecialResident,		//DRT 4/2/2008 - PLID 29493
	ecpSpecialStartup,		//DRT 4/2/2008 - PLID 29493
	ecpResidentAddOn,		//DRT 4/2/2008 - PLID 29493
	ecpStartupAddOn,		//DRT 4/2/2008 - PLID 29493
	ecpSchedulerStandard,	//DRT 9/17/2008 - PLID 31395
	ecpLettersStandard,		//DRT 9/17/2008 - PLID 31395
	ecpEMRFirstStandard,	//DRT 9/17/2008 - PLID 31395
	ecpEMRAddtlStandard,	//DRT 9/17/2008 - PLID 31395
	ecpPkgSchedStandard,	//DRT 9/17/2008 - PLID 31395
	ecpPkgCosmeticStandard,	//DRT 9/17/2008 - PLID 31395
	ecpEMRCosmetic,			// (d.thompson 2013-07-05) - PLID 57333
	ecpCandA,				//DRT 1/30/2009 - PLID 32890
	ecpPkgNexWeb,			// (d.thompson 2009-08-10) - PLID 35152
	ecpNexWebLeads,			// (d.thompson 2009-08-10) - PLID 35152
	ecpNexWebPortal,		// (d.thompson 2009-08-10) - PLID 35152
	ecpNexWebAddtlDomains,	// (d.thompson 2009-08-10) - PLID 35152
	ecpNexPhoto,			// (d.thompson 2009-11-06) - PLID 36123
	ecpNexSync,				// (d.thompson 2009-11-13) - PLID 36124
	ecpEmrConversion,		// (d.lange 2010-09-17 12:28) - PLID 40361
	ecpFinancialConversion,	// (d.lange 2010-09-17 12:28) - PLID 40361
	ecpTravelCanada,		// (d.lange 2010-12-21 14:23) - PLID 41889
	ecpTravelInternational,	// (d.lange 2010-12-21 14:23) - PLID 41889
	ecpTravelNewYorkCity,	// (d.lange 2010-12-21 14:23) - PLID 41889



	//This must be last
	ecpTotalPrices,
};

// (d.lange 2010-4-1) - PLID 38016 - each module, etc. has an index in m_aryModules
enum eCurrentModules {
	ecmScheduler = 0,
	ecmStdScheduler,
	ecm1License,
	ecmBilling,
	ecmHCFA,
	ecmEBilling,
	ecmLetterWriting,
	ecmQuotesMarketing,
	ecmNexTrak,
	ecmNexForms,
	ecmInventory,
	ecmCandA,
	ecmNexSpa,
	ecmASC,
	ecmERemittance,
	ecmEEligibility,
	ecmNexPhoto,
	ecmPDA,
	ecmNexSync,
	ecmMirror,
	ecmUnited,
	ecmHL7,
	ecmHIE,						// (d.thompson 2013-07-08) - PLID 57334
	ecmInform,
	ecmQuickBooks,
	ecmEMR,
	ecmStdEMR,
	ecmCosmeticEMR,				// (d.thompson 2013-07-05) - PLID 57333
	ecmWorkstations,
	ecmWorkstationsTS,			// (d.thompson 2012-10-12) - PLID 53155
	ecmEMRWorkstations,
	ecmMultiDoctor,
	ecmLeadGen,
	ecmPatientPortal,
	ecmAddDomains,
	ecmPMTraining,
	ecmEMRTraining,
	ecmSupportPercent,
	ecmSupportMonths,
	ecmConversion,
	ecmTravel,
	ecmExtraDB,
	ecmEmrConversion,			// (d.lange 2010-11-11 16:19) - PLID 40361 - Added EMR Conversion
	ecmFinancialConversion,		// (d.lange 2010-11-11 16:19) - PLID 40361 - Added Financial Conversion
	ecmTotalModules,
};

//DRT 3/31/2008 - PLID 29493 - These correspond with OpportunityTypesT and should never change.  They are written
//	to data.
enum eProposalTypes {
	eptNewSale = 1,
	eptAddOn = 2,
	eptRenewSupport = 3,
	ept3MonthAddOn = 4,
	eptNexRes = 5,
	eptNexStartup = 6,
};

enum eSubTotalType {
	estWithoutPkgPrice = 0,
	estWithPkgPrice,
};

// (d.lange 2010-09-17 12:44) - PLID 40361 - enum for Conversion Types
enum eConversionType {
	ectMultipleConversion = 0,
	ectDataConversion,
	ectEmrConversion,
	ectFinancialConversion,
};

// (d.lange 2010-12-21 14:23) - PLID 41889 - enum for Travel Type
enum eTravelType {
	ettDomestic = 0,
	ettCanada,
	ettInternational,
	ettNewYorkCity,
};
/////////////////////////////////////////////////////////////////////////////
// COpportunityProposalDlg dialog
#include "ProposalPricing.h"

class COpportunityProposalDlg : public CNxDialog
{
// Construction
public:
	COpportunityProposalDlg(CWnd* pParent);   // standard constructor

	// (d.thompson 2009-08-27) - PLID 35365 - Added financing data to all 3, which is saved outside the proposal
	int OpenNewProposal(long nOpportunityID, long nPatientID, CString strPatientName, long nTypeID, bool bIsFinancing);
	int OpenProposal(long nProposalID, long nOpportunityID, long nPatientID, CString strPatientName, long nTypeID, bool bIsFinancing);
	int OpenFromExisting(long nOpportunityID, long nPatientID, long nExistingProposalID, CString strPatientName, long nTypeID, bool bIsFinancing);

	void SetPatientID(long nID)	{	m_nPatientID = nID;	}
	void SetOpportunityID(long nID)	{	m_nOpportunityID = nID;	}
	void SetProposalID(long nID)	{	m_nID = nID;	}
	void SetImmediateLoad(bool bLoad)	{	bRequestImmediateLoad = bLoad;	}
	void SetTypeID(long nID) {		m_nTypeID = nID;	}


	//TODO
	COleCurrency CalculateSilentTotal();

// Dialog Data
	//{{AFX_DATA(COpportunityProposalDlg)
	enum { IDD = IDD_OPPORTUNITY_PROPOSAL_DLG };
	NxButton	m_btnUnited;
	NxButton	m_btnTracking;
	NxButton	m_btnNexPhoto;
	NxButton	m_btnSched;
	NxButton	m_btnQuotes;
	NxButton	m_btnQBooks;
	NxButton	m_btnNexSpa;
	NxButton	m_btnNexForms;
	NxButton	m_btnMirror;
	NxButton	m_btnLetters;
	NxButton	m_btnInv;
	NxButton	m_btnInform;
	NxButton	m_btnHL7;
	NxButton	m_btnHIE;						// (d.thompson 2013-07-08) - PLID 57334
	NxButton	m_btnHCFA;
	NxButton	m_btnERemit;
	NxButton	m_btnEElig;
	NxButton	m_btnEBilling;
	NxButton	m_btnBilling;
	NxButton	m_btnASC;
	NxButton	m_btn1License;
	NxButton	m_btnStdScheduler;
	NxButton	m_btnStdEMR;
	NxButton	m_btnCosmeticEMR;				// (d.thompson 2013-07-05) - PLID 57333
	NxButton	m_btnCandA;
	NxButton	m_btnNexWebLeads;
	NxButton	m_btnNexWebPortal;
	NxButton	m_btnEMRSplit;				// (d.lange 2010-03-30 - PLID 37956 - EMR Split checkbox
	NxButton	m_btnApplyDiscountSplit;		// (d.lange 2010-04-08 - PLID 38096 - Added checkbox for applying discount to both client & EMR merge
	CNxIconButton	m_btnCheckAllCosmetic;
	CNxIconButton	m_btnCheckAllFinancial;
	CNxLabel	m_labelSupportMonthly;
	CNxLabel	m_labelSpecialDiscountAddOn;
	CNxIconButton	m_btnChangeDiscount;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnMerge;
	CNxEdit	m_nxeditQuoteId;
	CNxEdit	m_nxeditCreatedBy;
	CNxEdit	m_nxeditPkgSched;
	CNxEdit	m_nxeditSelSched;
	CNxEdit	m_nxeditPkgFinancial;
	CNxEdit	m_nxeditSelFinancial;
	CNxEdit	m_nxeditPkgCosmetic;
	CNxEdit	m_nxeditSelCosmetic;
	CNxEdit	m_nxeditSelOther;
	CNxEdit	m_nxeditNumWorkstations;
	CNxEdit m_nxeditNumWorkstationsTS;
	CNxEdit m_nxeditNumEMRWorkstations;			// (d.lange 2010-03-30 - PLID 37956 - EMR Workstations Edit Box
	CNxEdit	m_nxeditSelWorkstations;
	CNxEdit	m_nxeditSelWorkstationsTS;
	CNxEdit m_nxeditSelEMRWorkstations;			// (d.lange 2010-03-30 - PLID 37956 - EMR Workstations Edit Box
	CNxEdit	m_nxeditNumDoctors;
	CNxEdit	m_nxeditSelDoctors;
	CNxEdit	m_nxeditNumPda;
	CNxEdit	m_nxeditSelPda;
	CNxEdit m_nxeditNumNexSync;
	CNxEdit	m_nxeditSelNexSync;
	CNxEdit	m_nxeditSelMirror;
	CNxEdit	m_nxeditSelUnited;
	CNxEdit	m_nxeditSelHl7;
	CNxEdit m_nxeditSelHIE;						// (d.thompson 2013-07-08) - PLID 57334
	CNxEdit	m_nxeditSelInform;
	CNxEdit	m_nxeditSelQuickbooks;
	CNxEdit	m_nxeditNumEmr;
	CNxEdit	m_nxeditSelEmr;
	CNxEdit	m_nxeditSubtotalList;
	CNxEdit	m_nxeditSubtotalPkgDiscounts;
	CNxEdit	m_nxeditSubtotalSel;
	CNxEdit	m_nxeditDiscountUsername;
	CNxEdit	m_nxeditDiscountTotalPercent;
	CNxEdit	m_nxeditDiscountTotalDollar;
	CNxEdit	m_nxeditNumTraining;
	CNxEdit	m_nxeditNumEmrTraining;
	CNxEdit	m_nxeditSelTraining;
	CNxEdit	m_nxeditSupportPercent;
	CNxEdit	m_nxeditNumSupport;
	CNxEdit	m_nxeditSelSupport;
	CNxEdit	m_nxeditNumConversion;
	CNxEdit	m_nxeditSelConversion;
	CNxEdit	m_nxeditNumTravel;
	CNxEdit	m_nxeditSelTravel;
	CNxEdit	m_nxeditNumExtradb;
	CNxEdit	m_nxeditSelExtradb;
	CNxEdit	m_nxeditGrandTotal;
	CNxEdit m_editNexWebAddtlDomains;
	CNxStatic	m_nxstaticLabelSched;
	CNxStatic	m_nxstaticLabel1License;
	CNxStatic	m_nxstaticLabelBilling;
	CNxStatic	m_nxstaticLabelHcfa;
	CNxStatic	m_nxstaticLabelEbilling;
	CNxStatic	m_nxstaticLabelLetters;
	CNxStatic	m_nxstaticLabelQuotes;
	CNxStatic	m_nxstaticLabelTracking;
	CNxStatic	m_nxstaticLabelNexforms;
	CNxStatic	m_nxstaticLabelInv;
	CNxStatic	m_nxstaticLabelSpa;
	CNxStatic	m_nxstaticLabelAsc;
	CNxStatic	m_nxstaticLabelEremittance;
	CNxStatic	m_nxstaticLabelEeligibility;
	CNxStatic	m_nxstaticLabelWorkstations;
	CNxStatic	m_nxstaticLabelWorkstationsTS;
	CNxStatic	m_nxstaticLabelEMRWorkstations;			// (d.lange 2010-03-30 - PLID 37956 - EMR Workstations Label
	CNxStatic	m_nxstaticLabelDoctors;
	CNxStatic	m_nxstaticLabelPda;
	CNxStatic	m_nxstaticLabelNexSync;
	CNxStatic	m_nxstaticLabelMirror;
	CNxStatic	m_nxstaticLabelUnited;
	CNxStatic	m_nxstaticLabelHl7;
	CNxStatic	m_nxstaticLabelHIE;						// (d.thompson 2013-07-08) - PLID 57334
	CNxStatic	m_nxstaticLabelInform;
	CNxStatic	m_nxstaticLabelQuickbooks;
	CNxStatic	m_nxstaticLabelEmr;
	CNxStatic	m_nxstaticLabelTraining;
	CNxStatic	m_nxstaticSpecialDiscountPercent;
	CNxStatic	m_nxstaticSpecialDiscountDollar;
	CNxStatic	m_nxstaticCandA;
	CNxStatic	m_nxstaticNexWebLeads;
	CNxStatic	m_nxstaticNexWebPortal;
	CNxStatic	m_nxstaticNexWebAddtlDomains;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpportunityProposalDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
protected:
// Implementation
	CArray<COleCurrency, COleCurrency> m_aryPrices;		//Current pricing structure
	
	// (d.lange 2010-04-02) - PLID 37999 - modify array to leave or remove EMR related fields
	CArray<int, int>& SplitEMRForClient(CArray<int, int> &aryModules);
	CArray<int, int>& SplitEMRForThirdParty(CArray<int, int> &aryModules);

	// (z.manning 2016-01-29 11:41) - No longer supported
	//void MergeFields(CProposalPricing &proposal, COleCurrency cySupportTotal, COleCurrency cyPMSupport, COleCurrency cyTotalDiscount, COleCurrency cyPMDiscount);

	//Members
	long m_nResidentAddOnDiscount, m_nStartupAddOnDiscount;		//DRT 4/2/2008 - PLID 29493
	long m_nID;											//ID of the currently open proposal.  -1 if new
	long m_nOpportunityID;								//ID of the parent Opportunity.  This must be set before OnInitDialog is reached
	long m_nMailSentID;									//ID of the merged proposal, -1 if not set.
	long m_nPriceStructureID;							//ID of the pricing entries in OpportunityPriceStructureT
	long m_nPatientID;									//ID of the patient on this proposal
	long m_nTypeID;										//ID of the type of this proposal (OpportunityTypesT)
	long m_nExistingLoadFromID;							//ID of an existing proposal, we will copy the selections from this value.  m_nID should be -1 in this case.
	bool bRequestImmediateLoad;							//Request that the dialog load immediately.  If this is not set, the dialog will post a message to do the load (slower, but looks better)
	long m_nCurrentDiscountUserID;						//UserID of the person that entered the current discount
	CString m_strPatientName;							//For window display only
	bool m_bIsFinancing;								// (d.thompson 2009-08-27) - PLID 35365 - Is financing in use on this opportunity?
	bool m_bSplitMerge;									// (d.lange 2010-04-06) - PLID 37999 - determines when allow to run a second merge
	long m_nMergeCount;									// (d.lange 2010-04-06) - PLID 37999 - used for toggling between standard and EMR Workstations
	long m_nTravelType;									// (d.lange 2010-12-21 14:23) - PLID 41889 - Travel Type

	//Controls
	CDateTimePicker	m_pickerProposalDate;
	CDateTimePicker	m_pickerExpireDate;

	//Functionality
	void FillPriceArray();
	void FillModuleArray();					// (d.lange 2010-4-1) - PLID 38016 - Set the array based on which modules are checked
	void ReflectPriceArray();
	COleCurrency GetCurrentSubTotalList();
	COleCurrency GetCurrentSubTotalSelected();
	void UpdateAllTotals();
	void UpdateSupportCosts();
	COleCurrency CalculateCurrentMonthlySupport();
	//DRT 5/27/2008 - PLID 29493
	COleCurrency CalculateCurrentMonthlySupport_PackageOnly();
	COleCurrency CalculateDiscountFromPercent(double dblPercent);
	void DisableAllInterface();
	// (z.manning, 11/26/2007) - PLID 28159 - Added a separate parameter for EMR training days as it is now
	// tracked separately.
	void CalculateTrainingFromCurrentInterface(long &nDays, long &nEmrDays, long &nTrips);
	void UpdateTraining();


	//Saving
	bool IsValidProposal();
	bool ApplyChanges();

	//Package functions
	void ReflectSchedulerPackage();
	void ReflectFinancialPackage();
	void ReflectCosmeticPackage();
	void ReflectOtherModules();
	// (d.thompson 2009-08-10) - PLID 35152 - Added NexWeb
	void ReflectNexWebPackage();
	// (d.lange 2010-12-21 16:43) - PLID 41889 - Determines the travel type based on city or country 
	void SetTravelType(); 

	// Generated message map functions
	//{{AFX_MSG(COpportunityProposalDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnChk1license();
	afx_msg void OnChkAsc();
	afx_msg void OnChkBilling();
	afx_msg void OnChkEbilling();
	afx_msg void OnChkHcfa();
	afx_msg void OnChkInv();
	afx_msg void OnChkEremittance();
	afx_msg void OnChkEeligibility();
	afx_msg void OnChkLetters();
	afx_msg void OnChkMirror();
	afx_msg void OnChkNexforms();
	afx_msg void OnChkNexspa();
	afx_msg void OnChkQuotes();
	afx_msg void OnChkSched();
	afx_msg void OnChkTracking();
	afx_msg void OnChkUnited();
	afx_msg void OnChangeNumConversion();
	afx_msg void OnChangeNumEmrConversion();		// (d.lange 2010-11-11 16:22) - PLID 40361 - Added EMR Conversion
	afx_msg void OnChangeNumFinancialConversion();		// (d.lange 2010-11-11 16:22) - PLID 40361 - Added Financial Conversion
	afx_msg void OnChangeNumDoctors();
	afx_msg void OnChangeNumEmr();
	afx_msg void OnChangeNumPda();
	afx_msg void OnChangeNumSupport();
	afx_msg void OnChangeNumTraining();
	afx_msg void OnChangeNumEmrTraining();
	afx_msg void OnChangeNumTravel();
	afx_msg void OnChangeExtraDB();
	afx_msg void OnChangeNumWorkstations();
	afx_msg void OnChangeNumWorkstationsTS();
	afx_msg void OnChangeNumEmrWorkstations();			// (d.lange 2010-03-30 - PLID 37956 - added for EMR Workstations edit box
	afx_msg void OnMergeProposal();
	afx_msg LRESULT OnLoadOpportunityProposal(WPARAM wParam, LPARAM lParam);
	afx_msg void OnChangeDiscount();
	afx_msg void OnCheckAllFinancial();
	afx_msg void OnCheckAllCosmetic();
	afx_msg void OnChkHl7();
	afx_msg void OnChkHIE();							// (d.thompson 2013-07-08) - PLID 57334
	afx_msg void OnChkInform();
	afx_msg void OnChkQuickbooks();
	afx_msg void OnChkSchedStandard();
	afx_msg void OnChkEmrStandard();
	afx_msg void OnChkEmrCosmetic();					// (d.thompson 2013-07-05) - PLID 57333
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CArray<int, int> m_aryModules;						// (d.lange 2010-4-2) - PLID 38016 - array of module statuses
	CString RemoveDollarSign(CString strDollarValue);	// (d.lange 2010-04-19 11:41) - PLID 38029 - Format string without dollar sign
	afx_msg void OnBnClickedChkCAndA();
	afx_msg void OnBnClickedChkNexwebLeads();
	afx_msg void OnBnClickedChkNexwebPortal();
	afx_msg void OnBnClickedChkEMRSplit();		// (d.lange 2010-03-30 - PLID 37956 - EMR Split checkbox
	afx_msg void OnBnClickedChkDiscountEMR();	// (d.lange 2010-04-08 - PLID 38096 - Added apply discount to emr merge
	afx_msg void OnEnChangeNumNexwebAddtlDomains();
	afx_msg void OnBnClickedChkNexphoto();
	afx_msg void OnEnChangeNumNexsync();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPPORTUNITYPROPOSALDLG_H__D46D8A3B_ED97_4FF9_B421_F3E4B98F0008__INCLUDED_)
