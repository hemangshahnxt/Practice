#if !defined(AFX_GENERAL2DLG_H__18527915_CE66_11D1_804C_00104B2FE914__INCLUDED_)
#define AFX_GENERAL2DLG_H__18527915_CE66_11D1_804C_00104B2FE914__INCLUDED_

#include <afxtempl.h>

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// General2Dlg.h : header file

#include "PatientDialog.h"
#include "Client.h"	// Network optimization code
/////////////////////////////////////////////////////////////////////////////
// CGeneral2Dlg dialog
				  
class CGeneral2Dlg : public CPatientDialog
{
// Construction
public:
	CGeneral2Dlg(CWnd* pParent);   // standard constructor

	void UpdatePalm(BOOL bUpdateAppointments);
//	void AddReferralNode(const CString &strNode, HTREEITEM hParent = TVI_ROOT);
	// (a.walling 2010-10-13 10:44) - PLID 40908 - Dead code
	//CString m_EndText;
//	bool m_TreeAddition;
	// (a.walling 2010-10-13 10:44) - PLID 40908 - Dead code
	//CString m_BeginText;
	virtual void SetColor(OLE_COLOR nNewColor);
	virtual void StoreDetails();
	virtual void RecallDetails();
	virtual void Refresh();
	// (a.walling 2010-10-12 17:40) - PLID 40908
	virtual void UpdateView(bool bForceRefresh = true);
	virtual int Hotkey(int key);
	void SecureControls();
	BOOL IsEditBox(CWnd* pWnd);
	BOOL OnInitDialog(); 
	void SetWarningBox();
	void SetRewardsWarningBox(); // (j.luckoski 2013-03-04 11:49) - PLID 33548
	void SetExpireBox();
	bool m_bShowWarningMessage;

	//variables for the company field to have the auto-search feature
	int m_nLastEditLength;
	bool m_bSettingCompany;

// Dialog Data
	//{{AFX_DATA(CGeneral2Dlg)
	enum { IDD = IDD_GENERAL_2_DLG_32 };
	CNxIconButton	m_btnAddAllocation;
	NxButton	m_btnExpiresOn;
	CNxIconButton	m_btnMakePrimary;
	CNxIconButton	m_btnRemoveReferral;
	CNxIconButton	m_btnAddReferral;
	NxButton	m_WarningCheck;
	CNxColor	m_empBkg;
	CNxColor	m_emp3Bkg;
	CNxColor	m_referralBkg;
	CNxColor	m_lastupdatedBkg;
	CNxColor	m_raceBkg;
	NxButton	m_fulltime;
	NxButton	m_fulltimeStudent;
	NxButton	m_parttimeStudent;
	NxButton	m_parttime;
	NxButton	m_retired;
	NxButton	m_other;
	CNxColor	m_typeBkg;
	CDateTimePicker	m_expireDate;
	CNxColor	m_NexwebLoginInfoBkg;
	CNxEdit	m_nxeditOccupation;
	CNxEdit	m_nxeditCompany;
	CNxEdit	m_nxeditManagerFname;
	CNxEdit	m_nxeditManagerMname;
	CNxEdit	m_nxeditManagerLname;
	CNxEdit	m_nxeditEmpAddress1;
	CNxEdit	m_nxeditEmpAddress2;
	CNxEdit	m_nxeditEmpZip;
	CNxEdit	m_nxeditEmpCity;
	CNxEdit	m_nxeditEmpState;
	CNxEdit	m_nxeditWarning;
	CNxEdit	m_nxeditEditWarningUser;
	CNxEdit	m_nxeditNumPatientsRef;
	CNxEdit	m_nxeditNumProspectsRef;
	CNxEdit	m_nxeditLastModified;
	CNxEdit m_nxeditAccumRewardPts;
	CNxStatic	m_nxstaticG2LocationLabel;
	CNxStatic	m_nxstaticDonorLabel;
	CNxStatic	m_nxstaticNumPatientsReferredLabel;
	CNxStatic	m_nxstaticNumProspectsReferredLabel;
	CNxStatic	m_nxstaticSerializedProductsLabel;
	CNxIconButton m_btnEditRewardPoints; // (z.manning 2010-07-20 12:37) - PLID 30127
	CNxColor	m_WarrantyBkg; //(c.copits 2010-10-28) PLID 38598 - Warranty tracking system
	NxButton m_RewardsWarningCheck; // (j.luckoski 2013-03-04 11:50) - PLID 33548
	// (b.spivey, May 17, 2013) - PLID 56872 - Label for multiple patients. 
	CNxLabel	m_nxstaticMultiRaceLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides	
	//{{AFX_VIRTUAL(CGeneral2Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);	
	
	// (a.walling 2011-08-04 14:36) - PLID 44788 - Delay freeing the datalist when hiding / deactivating
	virtual void OnParentViewActivate(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

protected:
	long CGeneral2Dlg::AddNewRefPhys();

	BOOL m_bLookupByCity;

// Implementation
protected:
	void RefreshSlowStuff();
	void RefreshReferrals();

	CTableChecker	m_diagChecker, 
					m_refphyChecker, 
					m_patienttypeChecker, 
					m_defaultlocChecker,
					m_refPatChecker;

	// (j.jones 2014-02-10 15:25) - PLID 60719 - added the diagnosis search control
	// and list of diagnosis codes
	NXDATALIST2Lib::_DNxDataListPtr m_diagSearch;
	NXDATALIST2Lib::_DNxDataListPtr m_defaultDiagList;

	NXDATALISTLib::_DNxDataListPtr m_PatientTypeCombo,
					m_ReferringPhyCombo,
					m_PCPCombo,
					m_DefaultLocationCombo,
					m_referringPatientCombo,
					m_donorCombo, 
					m_referralSourceList,
					m_raceCombo,
					m_ethnicityCombo;	// (j.jones 2009-10-19 10:00) - PLID 34327 - added ethnicity combo

	NXDATALIST2Lib::_DNxDataListPtr m_pAffiliateList, // (j.gruber 2011-09-27 13:12) - PLID 45357
		m_pOccupationCodeList, //TES 11/13/2013 - PLID 59483
		m_pIndustryCodeList; //TES 11/13/2013 - PLID 59483

	// (a.walling 2009-05-25 10:31) - PLID 27859 - Populate the language dropdown
	void PopulateLanguageCombo();

	// (z.manning 2008-12-08 11:58) - PLID 32320 - Added an enum for the ref phys datalist's columns
	// (j.jones 2011-12-01 13:25) - PLID 46771 - enums were not added for the address columns, so I added them
	enum EReferringPhysicianColumns {
		rpcID = 0,
		rpcName,
		rpcNpi,
		rpcRefPhysID,
		rpcUpin,
		rpcShowProcWarning,
		rpcProcWarning,
		rpcAddress1,
		rpcAddress2,
		rpcCity,
		rpcState,
		rpcZip,
	};

	// (j.jones 2011-12-01 13:58) - PLID 46771 - added enums for PCP
	enum EPrimaryCarePhysicianColumns {
		pcpcID = 0,
		pcpcName,
		pcpcNpi,
		pcpcRefPhysID,
		pcpcUpin,
		pcpcAddress1,
		pcpcAddress2,
		pcpcCity,
		pcpcState,
		pcpcZip,
	};

	NXDATALIST2Lib::_DNxDataListPtr m_pRelatedPatientList,
					m_SerializedProductList,	// (j.jones 2008-03-20 16:05) - PLID 29334
					m_languageCombo, // (a.walling 2009-07-02 10:00) - PLID 27859
					m_pWarningCategoryList; // (a.walling 2010-06-30 17:43) - PLID 18081 - Warning categories

	long m_id;
	CString m_strPatientName;
	
	// (a.walling 2010-10-12 14:54) - PLID 40908 - Don't load the id until UpdateView is called
	//long m_lastID;
	long m_nRelatedPatient; // (a.walling 2006-11-15 10:31) - PLID 22715
	void TrySetRelatedPatient(); // tries to select the related patient in the dropdown
	void TrySetReferringPatient(); // (a.walling 2011-08-01 17:34) - PLID 44788

	void Save(int nID);
	bool m_loading;
	// (a.walling 2010-10-13 10:44) - PLID 40908 - Dead code
	//COleDateTime m_dtDateTmp;
	//CString m_strNewDataTmp;
	//bool m_bAutoRefreshing;	
	//bool m_bSettingBox;	
	//CString m_strCurDataTmp;
	long m_nReferralID;
	// (a.walling 2010-10-13 10:44) - PLID 40908 - Dead code
	//CMap<CString, LPCTSTR, BOOL, BOOL> m_mapExpandedNodes;
	void AddReferral(long nReferralID = -1);

	// (r.goldschmidt 2014-09-19 12:28) - PLID 31191 - Check for possible conflict with parent referral preference, AllowParentReferral
	bool IsReferralSelectionAllowed(long nReferralID, CString& strWarning);

	// (a.walling 2007-05-21 11:10) - PLID 26079 - Store the original referring patient ID
	long m_nReferringPatientID;

	NXTIMELib::_DNxTimePtr m_nxtAccidentDate;
	COleDateTime m_dtAccident;
	bool m_bSavingAccidentDate;
//	CReferralSourceSet m_rsReferralTree;

	void UpdateChangedDate(long nAuditID);

	// (j.jones 2008-03-20 16:06) - PLID 29334 - will hide the serialized product list
	// and move other controls to take up its space
	void HideSerializedProductList();
	// (j.jones 2008-03-21 08:56) - PLID 29334 - the clause is rather large, so it's
	// going into its own function for readability
	void SetSerializedProductListFromClause();

	// (j.jones 2008-03-21 10:21) - PLID 29334 - cached the adv. inv. license
	BOOL m_bHasAdvInventoryLicense;

	//(c.copits 2010-10-29) PLID PLID 38598 - Warranty tracking system
	CString MakeWarrantyDlgProductListFromClause();

	// (j.gruber 2011-09-27 15:02) - PLID 45357 - AffiliatePhysician
	long m_nAffiliatePhysID;
	CString m_strAffiliateName;

	// (j.gruber 2012-10-16 17:57) - PLID 47289 - affiliate phys type	
	NXDATALIST2Lib::_DNxDataListPtr m_pAffiliateTypeList;
	AffiliatePhysType m_nAffiliatePhysTypeID;

	// (j.jones 2014-02-18 17:00) - PLID 60719 - the diagnosis list
	// should ideally be fixed to match the search style, which is
	// controlled by a preference, but if it is only showing ICD-9
	// or ICD-10, it needs to dynamically change if codes exist
	// for the hidden columns
	void UpdateDiagnosisListColumnSizes();

	// (z.manning, 05/16/2008) - PLID 30050 - Added OnCtlColor
	// (a.walling 2009-05-25 10:27) - PLID 27859 - Language combo
	// (c.haag 2010-02-01 13:28) - PLID 34727 - Added OnGotoRefPat
	// Generated message map functions
	//{{AFX_MSG(CGeneral2Dlg)
	afx_msg void OnWarningCheck();
	afx_msg void OnReturnReferringPhysicianCombo();
	afx_msg void OnReturnPatientTypeCombo();
	afx_msg void OnSelectReferral(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditReferralTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFulltimeRadio();
	afx_msg void OnFulltimeStudentRadio();
	afx_msg void OnRetiredRadio();
	afx_msg void OnParttimeRadio();
	afx_msg void OnParttimeStudentRadio();
	afx_msg void OnOtherRadio();
	afx_msg void OnEditPatientTypeCombo();
	afx_msg void OnEditRaceCombo();
	afx_msg void OnSelChosenReferringPatientCombo(long nRow);
	afx_msg void OnKillfocusReferralTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChosenRaceCombo(long nRow);
	afx_msg void OnSelChosenDefaultLocationCombo(long nRow);
	afx_msg void OnSelChosenReferringPhysicianCombo(long nRow);
	afx_msg void OnSelChosenPatientTypeCombo(long nRow);
	afx_msg void OnSelChosenPcpCombo(long nRow);
	afx_msg void OnSelChosenDonorPatientCombo(long nRow);
	afx_msg void OnAddReferral();
	afx_msg void OnRemoveReferral();
	afx_msg void OnMakePrimaryReferral();
	afx_msg void OnEditingFinishingReferralList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedReferralList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnReferredPats();
	afx_msg void OnReferredProspects();
	afx_msg void OnSelChangedReferralList(long nNewSel);
	afx_msg void OnKillfocusWarning();
	afx_msg void OnCopyEmployerBtn();
	afx_msg void OnTrySetSelFinishedDefaultLocationCombo(long nRowEnum, long nFlags);
	afx_msg void OnKillFocusAccidentDate();
	afx_msg void OnRButtonUpReferralList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnNewRefPhys();
	afx_msg void OnNewPcp();
	afx_msg void OnExpireDate();
	afx_msg void OnChangeExpireDatePicker(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeCompany();
	afx_msg void OnRequeryFinishedReferralList(short nFlags);
	afx_msg void OnRequeryFinishedPatientTypeCombo(short nFlags);
	afx_msg void OnEditNexwebInfo();
	afx_msg void OnSelChangingRelation(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenRelation(LPDISPATCH lpRow);
	// (a.walling 2011-08-01 17:34) - PLID 44788
	//afx_msg void OnTrySetSelFinishedRelation(long nRowEnum, long nFlags); 
	afx_msg void OnDroppingDownReferringPatientCombo();
	afx_msg void OnDroppingDownRelation();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBtnEditFamilies();
	afx_msg void OnGotoRefPhys();
	afx_msg void OnGotoRefPat();
	afx_msg void OnGotoPcp();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnRButtonUpSerializedProductList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnAddAllocation();
	afx_msg void OnSelChosenLanguageCombo(LPDISPATCH lpRow);
	// (j.jones 2009-10-19 10:34) - PLID 34327 - added ethnicity option
	afx_msg void OnSelChosenEthnicityCombo(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	// (a.walling 2010-10-13 10:44) - PLID 40908 - Dead code
	//int m_iComboRowSelection;
	// (a.walling 2010-06-30 17:43) - PLID 18081 - Warning categories
	void SelChangingListWarningCategories(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenListWarningCategories(LPDISPATCH lpRow);
	void RequeryListWarningCategories();
	void RequeryFinishedListWarningCategories(short nFlags);
	void CurSelWasSetListWarningCategories();
	afx_msg void OnBnClickedAdjustRewardPoints(); // (z.manning 2010-07-20 12:17) - PLID 30127
	// (b.spivey, May 14, 2013) - PLID 56872 - functions to toggle the multiple patient race select/display. 
	void OnRequeryFinishedRaceList(short nFlags); 
	CArray<long, long> m_naryRaceIDs; 
	void ToggleLabels();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	bool MultiSelectRaceDlg();
public:
	//(c.copits 2010-10-28) PLID 38598 - Warranty tracking system
	afx_msg void OnBnClickedWarranty();
	// (j.gruber 2011-09-22 16:24) - PLID 45357
	void SelChosenAffiliatePhysList(LPDISPATCH lpRow);
	void RequeryFinishedAffiliatePhysList(short nFlags);
	afx_msg void OnBnClickedGotoAffiliate();
	void SelChangingAffiliatePhysList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenAffPhysType(LPDISPATCH lpRow); 	// (j.gruber 2012-10-16 17:57) - PLID 47289
	void SelChangingAffPhysType(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel); 	// (j.gruber 2012-10-16 17:57) - PLID 47289	
	afx_msg void OnBnClickedWarningCheck2(); // (j.luckoski 2013-03-04 11:50) - PLID 33548
	void SelChosenOccupationCensusCode(LPDISPATCH lpRow);
	void SelChosenIndustryCensusCode(LPDISPATCH lpRow);
	// (j.jones 2014-02-18 16:54) - PLID 60719 - added the diagnosis search control
	void OnSelChosenDefaultDiagSearchList(LPDISPATCH lpRow);
	// (j.jones 2014-02-19 11:23) - PLID 60864 - added right click ability to remove default codes
	void OnRButtonDownDefaultDiagCodeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (j.jones 2014-03-06 11:35) - PLID 60719 - resize the columns to reflect the new data
	void OnRequeryFinishedDefaultDiagCodeList(short nFlags);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENERAL2DLG_H__18527915_CE66_11D1_804C_00104B2FE914__INCLUDED_)
