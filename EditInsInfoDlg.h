#if !defined(AFX_EDITINSINFODLG_H__B3FA20F3_1CAC_11D2_8097_00104B2FE914__INCLUDED_)
#define AFX_EDITINSINFODLG_H__B3FA20F3_1CAC_11D2_8097_00104B2FE914__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// EditInsInfoDlg.h : header file
//

#include <Guard.h>

enum InsuranceTypeCode;

/////////////////////////////////////////////////////////////////////////////
// CEditInsInfoDlg dialog

class CEditInsInfoDlg : public CNxDialog
{
// Construction
public:
	void ChangeTaxType();
	OLE_COLOR m_nColor;
	CString m_strNewDataTmp;
	CString m_strCurDataTmp;
	CBoolGuard m_bSettingBox; // (a.walling 2011-08-29 12:20) - PLID 45232 - Use safe guards
	CEditInsInfoDlg(CWnd* pParent);   // standard constructor
	void WriteField(int nID, CString & data);
	void StoreBox(int nID);
	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	void GetBox(int nID, CString & data);
	void SetColor(OLE_COLOR nNewColor);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	void DisplayWindow(OLE_COLOR BackColor, long nCompanyID = -1, long nContactID = -1);
	long m_nCompanyID, m_nContactID;
	
protected:
	enum EExitingCurInsCoAction
	{
		ecicaClosing,
		ecicaChangingSel,
		ecicaCreatingInsCo,
	};

	bool CheckUnusualInsuranceInfo(IN const EExitingCurInsCoAction eecica);
	bool CheckOnExitingCurrentInsCo(IN const EExitingCurInsCoAction eecica);
	// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
	void ReflectInsListSelection(NXDATALIST2Lib::IRowSettingsPtr pRow);
	
	BOOL m_bAutoSetContact;

	// (j.gruber 2009-10-08 09:56) - PLID 35825 - Override the tab order if they have city lookup	
	BOOL m_bLookupByCity;

	// (j.jones 2008-07-10 16:26) - PLID 28756 - added payment description/category controls
	// (j.jones 2008-07-11 09:31) - PLID 30679 - added m_btnAdvInsDescSetup
	// (j.jones 2008-08-01 10:35) - PLID 30917 - added m_btnClaimProviderSetup
// Dialog Data
	//{{AFX_DATA(CEditInsInfoDlg)
	enum { IDD = IDD_EDIT_INSURANCE_DLG };
	CNxIconButton	m_btnClaimProviderSetup;
	CNxIconButton	m_btnAdvInsDescSetup;
	CNxIconButton   m_btnDefaultClaimFormSetup;
	CNxEdit	m_nxeditDefAdjDescription;
	CNxEdit	m_nxeditDefPayDescription;
	CNxIconButton	m_btnEditAdjCats;
	CNxIconButton	m_btnEditPayCats;	
	CNxIconButton	m_btnBox31;
	CNxIconButton	m_btnEditCPTNotes;
	NxButton	m_btnInactive;
	NxButton	m_checkWorkersComp;
	NxButton	m_ReferralCheck;
	NxButton	m_radioTaxPatient;
	NxButton	m_radioTaxNone;
	NxButton	m_radioTaxIns;
	CNxIconButton	m_btnAcceptAssignment;
	CNxIconButton	m_btnManageContacts;
	NxButton	m_checkMakeDefaultContact;
	CNxIconButton	m_btnDeleteContact;
	CNxIconButton	m_btnAddContact;
	CNxIconButton	m_btnFacilityID;
	CNxIconButton	m_btnBox51;
	NxButton	m_HMOCheck;
	CNxIconButton	m_btnEditNetworkID;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDelPlan;
	CNxIconButton	m_btnGroups;
	CNxIconButton	m_btnBox24J;
	CNxIconButton	m_btnAddPlan;
	CNxIconButton   m_btnEditDefaultDeductible; //(8/17/2012) r.wilson - PLID 50636
	CComboBox	m_ctrlTypeCombo;
	CNxColor	m_bkg;
	CNxColor	m_bkg2;
	CNxColor	m_bkg3;
	CNxColor	m_bkg4;
	CNxColor	m_bkg5;
	CNxColor	m_bkg6;
	CNxColor	m_bkg7;
	CNxColor	m_bkg8; // (c.haag 2009-03-16 10:28) - PLID 9148
	CNxEdit	m_nxeditInsurancePlanBox;
	CNxEdit	m_nxeditContactAddressBox;
	CNxEdit	m_nxeditContactAddress2Box;
	CNxEdit	m_nxeditContactZipBox;
	CNxEdit	m_nxeditContactCityBox;
	CNxEdit	m_nxeditContactStateBox;
	CNxEdit	m_nxeditRvuBox;
	CNxEdit	m_nxeditCoMemo;
	CNxEdit	m_nxeditEditCrossoverCode;
	CNxEdit	m_nxeditContactFirstBox;
	CNxEdit	m_nxeditContactLastBox;
	CNxEdit	m_nxeditContactTitleBox;
	CNxEdit	m_nxeditContactPhoneBox;
	CNxEdit	m_nxeditContactExtBox;
	CNxEdit	m_nxeditContactFaxBox;
	// (j.jones 2009-08-04 12:56) - PLID 34467 - added ability to configure payer IDs per location
	CNxIconButton m_btnConfigLocationPayerIDs;
	// (j.jones 2010-08-30 16:01) - PLID 40302 - added TPL Code
	CNxEdit m_nxeditTPLCode;
	// (j.jones 2011-04-05 14:50) - PLID 42372 - added the CLIA Setup
	CNxIconButton	m_btnCLIASetup;
	//(e.lally 2011-07-01) PLID 41207 - Added prev/next buttons for the insurance company list
	CNxIconButton	m_btnInsCoPrev;
	CNxIconButton	m_btnInsCoNext;
	// (j.jones 2013-08-05 14:32) - PLID 57805 - added link to the HCFA upgrade date setup
	CNxLabel	m_lblHCFAUpgradeDate;
	CNxLabel	m_lblICD10GoLiveDate; 	// (b.spivey - March 6th, 2014) - PLID 61196 
	//}}AFX_DATA

// Overrides	
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditInsInfoDlg)
	protected:	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pInsList; // (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
	NXDATALISTLib::_DNxDataListPtr	m_pEnvoyList;
	// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
	//NXDATALISTLib::_DNxDataListPtr	m_pTHINList;
	// (j.jones 2009-12-16 15:00) - PLID 36237 - added UB payer ID
	NXDATALISTLib::_DNxDataListPtr	m_pUBPayerList;
	// (j.jones 2008-09-09 13:50) - PLID 31138 - changed the Regence list to the Eligibility list
	NXDATALISTLib::_DNxDataListPtr	m_pEligibilityPayerIDList;
	NXDATALISTLib::_DNxDataListPtr	m_pHCFAGroups;
	NXDATALISTLib::_DNxDataListPtr	m_pUB92Groups;
	NXDATALISTLib::_DNxDataListPtr	m_pInsPlans;
	NXDATALISTLib::_DNxDataListPtr	m_pContactList;
	// (j.jones 2008-09-08 17:53) - PLID 18695 - renamed the NSF list to InsuranceTypeList
	NXDATALISTLib::_DNxDataListPtr	m_pInsuranceTypeList;
	// (j.jones 2008-07-10 16:38) - PLID 28756 - added pay/adj category datalists
	NXDATALISTLib::_DNxDataListPtr	m_pDefPayCategoryCombo;
	NXDATALISTLib::_DNxDataListPtr	m_pDefAdjCategoryCombo;
	// (c.haag 2009-03-16 10:03) - PLID 9148 - Added Financial Class dropdown
	NXDATALISTLib::_DNxDataListPtr	m_pFinancialClassCombo;

	// (j.jones 2008-09-09 08:47) - PLID 18695 - this function will add a new row to m_pInsuranceTypeList
	void AddNewRowToInsuranceTypeList(InsuranceTypeCode eCode, BOOL bColorize = FALSE);

	BOOL m_bFormatPhoneNums;
	CString m_strPhoneFormat;

	CString m_strAreaCode;
	void FillAreaCode(long nID);
	bool SaveAreaCode(long nID);
	void RefreshPlanButtons();
	
	bool m_bIsNewCo;
	// (c.haag 2010-05-06 12:29) - PLID 37525 - We don't need m_nCurInsCoID. If anything, a disparity
	// between that value and the list's current selection can spell trouble.
	//long m_nCurInsCoID;

	// If a message box appears by virtue of CheckOnExitingCurrentInsCo being called during an insurance
	// dropdown selection change, then it's possible for the selection change event to get fired a second
	// time. Use this member variable to check for that condition.
	BOOL m_bSelChangeInProgress;

	// (j.jones 2008-08-01 10:33) - PLID 30917 - colorize the Claim Provider Setup button when in use
	void UpdateClaimProviderBtnAppearance();

	// (j.jones 2009-08-04 17:33) - PLID 34467 - colorize the payer ID by location button when in use
	void UpdateConfigLocationPayerIDBtnAppearance();

	// (j.jones 2013-08-05 11:46) - PLID 57805 - loads the proper label text for m_lblHCFAUpgradeDate
	void LoadHCFAUpgradeDateLabel();

	// (b.spivey - March 6th, 2014) - PLID 61196 - Update the hyperlink with a new date or no date. 
	void LoadICD10GoLiveDateLabel();

	// (j.jones 2008-07-10 16:26) - PLID 28756 - added payment description/category functions
	// (j.jones 2008-07-11 09:31) - PLID 30679 - added OnBtnAdvInsDescSetup
	// (j.jones 2008-08-01 10:35) - PLID 30917 - added OnBtnClaimProviderSetup
	// Generated message map functions
	//{{AFX_MSG(CEditInsInfoDlg)
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectionChangeEnvoyCombo(long iNewRow);
	afx_msg void OnSelectionChangeThinCombo(long iNewRow);
	afx_msg void OnSelChosenHcfaSetup(long nNewSel);
	afx_msg void OnChangeContactFaxBox();
	afx_msg void OnChangeContactPhoneBox();
	afx_msg void OnEditingFinishingInsPlans(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedInsPlans(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	// (j.jones 2009-12-16 15:00) - PLID 36237 - renamed to OnEditPayerIDs
	afx_msg void OnEditPayerIDs();
	// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
	//afx_msg void OnEditThinList();
	afx_msg void OnAddCompany();
	afx_msg void OnDeleteCompany();
	afx_msg void OnBtnOk();
	afx_msg void OnAddPlan();
	afx_msg void OnDeletePlan();
	afx_msg void OnBtnGroups();
	afx_msg void OnBtnBox24J();
	afx_msg void OnEditNetworkid();
	afx_msg void OnHmoCheck();
	afx_msg void OnSelChosenEligibility(long nRow);
	afx_msg void OnSelChosenEnvoy(long nRow);
	// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
	//afx_msg void OnSelChosenThin(long nRow);
	afx_msg void OnSelChosenUb92Setup(long nRow);
	afx_msg void OnBtnBox51();
	afx_msg void OnBtnFacilityId();
	afx_msg void OnSelChosenInsContactsList(long nRow);
	afx_msg void OnAddContact();
	afx_msg void OnDeleteContact();
	afx_msg void OnCheckMakeDefaultContact();
	afx_msg void OnManageContacts();
	afx_msg void OnRequeryFinishedInsContactsList(short nFlags);
	afx_msg void OnBtnAccepted();
	afx_msg void OnRadioTaxIns();
	afx_msg void OnRadioTaxPatient();
	afx_msg void OnRadioTaxNone();
	afx_msg void OnCheckUseReferrals();
	afx_msg void OnSelChosenInsTypeCombo(long nRow);
	afx_msg void OnWorkersCompCheck();
	afx_msg void OnInactiveCheck();
	afx_msg void OnBtnEditCptNotes();
	afx_msg void OnBtnBox31();
	afx_msg void OnLeftClickInsPlans(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelChangedInsPlans(long nNewSel);	
	afx_msg void OnBtnEditPayCats();
	afx_msg void OnBtnEditAdjCats();
	afx_msg void OnBtnAdvInsDescSetup();
	afx_msg void OnSelChosenComboDefPayCategory(long nRow);
	afx_msg void OnSelChosenComboDefAdjCategory(long nRow);
	afx_msg void OnBtnClaimProviderSetup();
	void SelChosenComboFinancialClass(long nRow);
	afx_msg void OnBnClickedBtnEditFinancialClass();
	// (j.jones 2009-08-04 12:56) - PLID 34467 - added ability to configure payer IDs per location
	afx_msg void OnBtnConfigureLocationPayerIds();
	// (j.jones 2009-12-16 15:00) - PLID 36237 - added UB payer ID
	afx_msg void OnSelChosenUbPayerIds(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	
	void SelChangingInsList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangedInsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	// (j.jones 2011-04-05 14:50) - PLID 42372 - added the CLIA Setup
	afx_msg void OnCliaSetup();	
	//(e.lally 2011-07-01) PLID 41207 - Added prev/next buttons for the insurance company list
	afx_msg void OnBtnPrevInsuranceCo();
	afx_msg void OnBtnNextInsuranceCo();
	//(e.lally 2011-07-01) PLID 41207
	void HandleSelChangedInsList(NXDATALIST2Lib::IRowSettingsPtr pOldSel, NXDATALIST2Lib::IRowSettingsPtr pNewSel);
	afx_msg void OnBnClickedBtnEditDefaultDeductible();
	// (j.jones 2013-08-05 14:33) - PLID 57805 - added link to the HCFA upgrade date setup
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedBtnClaimFormSetup();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITINSINFODLG_H__B3FA20F3_1CAC_11D2_8097_00104B2FE914__INCLUDED_)
