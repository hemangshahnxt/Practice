#if !defined(AFX_HCFASETUP_H__06898FB6_6F5E_11D3_AD6B_00104B318376__INCLUDED_)
#define AFX_HCFASETUP_H__06898FB6_6F5E_11D3_AD6B_00104B318376__INCLUDED_

#include "Client.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HcfaSetup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHcfaSetup dialog

class CHcfaSetup : public CNxDialog
{
// Construction
public:	
	// (j.jones 2009-03-02 15:46) - PLID 31913 - changed this function to return how many records were affected
	long UpdateTable(CString BoxName, long data);
	CHcfaSetup(CWnd* pParent);   // standard constructor

	// (j.jones 2007-02-21 15:28) - PLID 24776 - supported option to put a space between qualifiers and IDs
	// (j.jones 2008-04-03 12:06) - PLID 28995 - added m_btnConfigClaimProviders
	// (j.jones 2008-06-10 14:47) - PLID 25834 - added m_checkValidateRefPhy
	// (j.jones 2009-03-06 11:21) - PLID 33354 - added m_radioAccUseBill and m_radioAccUseInsuredParty
// Dialog Data
	//{{AFX_DATA(CHcfaSetup)
	enum { IDD = IDD_HCFA_SETUP };
	NxButton	m_checkValidateRefPhy;
	CNxIconButton	m_btnConfigClaimProviders;
	NxButton	m_checkPutSpaceAfterQualifiers;
	NxButton	m_radioUseLocNPI;
	NxButton	m_radioUseProvNPI;
	NxButton	m_checkUse1aIn9aAlways;
	NxButton	m_checkHidePhoneNum;
	NxButton	m_checkUse1aIn9a;
	NxButton	m_checkUseOverrideInBox31;
	NxButton	m_checkUse23In17aAndNotIn23;
	NxButton	m_checkUse23In17a;
	CNxIconButton	m_btnDoNotFillSetup;
	NxButton	m_radioRefFML;
	NxButton	m_radioRefLFM;	
	CNxIconButton	m_btnAdvPINSetup;
	CNxIconButton	m_btnAdvPOSCodeSetup;
	CNxIconButton	m_btnAdvEbillingSetup;
	CNxIconButton	m_btnHCFADates;
	// (j.jones 2011-04-05 14:50) - PLID 42372 - removed the CLIA setup
	NxButton	m_checkOverrideBox19;	
	NxButton	m_checkBox19RefPhyID;	
	NxButton	m_radioUseLocationAddr;
	NxButton	m_radioUseContactAddr;
	NxButton	m_checkUseEmployer;	
	NxButton	m_radioFML;
	NxButton	m_radioLFM;	
	CNxIconButton	m_btnAlignForm;
	CNxIconButton	m_btnEditInsList;
	NxButton	m_radioOverride;
	NxButton	m_radioUseIfBlank;	
	CNxIconButton	m_deleteGroup;
	CNxIconButton	m_newGroup;
	NxButton	m_25ein;
	NxButton	m_25ssn;
	CNxIconButton	m_remBtn;
	CNxIconButton	m_remAllBtn;
	CNxIconButton	m_addBtn;
	CNxIconButton	m_addAllBtn;
	NxButton	m_useBillProvider;
	NxButton	m_useMainProvider;
	NxButton	m_overideProvider;
	NxButton	m_useBillLocation;
	CNxEdit		m_33GRP;
	CNxEdit	m_nxeditBox11;
	CNxEdit	m_nxeditBox19;
	CNxEdit	m_nxeditEditPriorAuth;
	CNxEdit	m_nxedit17AQual;
	CNxEdit	m_nxedit33BQual;
	CNxEdit	m_nxeditName;
	CNxEdit	m_nxeditAddress;
	CNxEdit	m_nxeditZip;
	CNxEdit	m_nxeditCity;
	CNxEdit	m_nxeditState;
	CNxEdit	m_nxeditPhone;
	CNxStatic	m_nxstaticDefBatchLabel;
	CNxStatic	m_nxstaticDefBatchNotPrimaryLabel;
	CNxStatic	m_nxstaticBox33NameLabel;
	CNxStatic	m_nxstaticBox33AddressLabel;
	CNxStatic	m_nxstaticO1static;
	CNxStatic	m_nxstaticO2static;
	CNxStatic	m_nxstaticO6static;
	CNxStatic	m_nxstaticO3static;
	CNxStatic	m_nxstaticO5static;
	CNxStatic	m_nxstaticO4static;
	CNxStatic	m_nxstaticBox33LocNpiLabel;
	NxButton	m_radioAccUseBill;
	NxButton	m_radioAccUseInsuredParty;
	// (j.jones 2010-08-31 16:33) - PLID 40303 - supported TPLIn9a
	NxButton	m_checkUseTPLIn9a;
	// (j.jones 2013-06-10 16:26) - PLID 56255 - added UseRefPhyGroupNPI
	NxButton	m_checkUseRefPhyGroupNPI;
	// (j.jones 2013-07-17 15:49) - PLID 41823 - added DontBatchSecondary
	NxButton	m_checkDontBatchSecondary;
	CNxIconButton	m_btnSecondaryExclusions;
	// (j.jones 2013-08-05 11:46) - PLID 57805 - added link to the HCFA upgrade date setup
	CNxLabel	m_lblUpgradeDate;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr	m_pGroups;
	NXDATALISTLib::_DNxDataListPtr	m_pUnselected;
	NXDATALISTLib::_DNxDataListPtr	m_pSelected;
	NXDATALISTLib::_DNxDataListPtr	m_17a;
	NXDATALISTLib::_DNxDataListPtr m_33;
	NXDATALISTLib::_DNxDataListPtr m_BatchCombo;
	NXDATALISTLib::_DNxDataListPtr m_SecondaryBatchCombo;
	NXDATALISTLib::_DNxDataListPtr m_Box24CCombo;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHcfaSetup)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void AddCustomInfo();
	void Build17aCombo();
	void Build33Combo();	
	void AddItemTo17aCombo(long id, CString item);
	void AddItemTo33Combo(long id, CString item);
	void SetRadio (CButton &radioTrue, CButton &radioFalse, const bool isTrue);
	void Load();
	void ShowBox33Controls(long nBox33Type);
	void Save (int nID);
	bool m_loading;
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	// (j.jones 2007-02-21 15:05) - PLID 24580 - supported colorizing the Advanced PIN Setup button when in use
	void UpdateAdvancedPINSetupAppearance();
	// (j.jones 2008-04-03 12:08) - PLID 28995 - colorize the Claim Provider Setup button when in use
	// (j.jones 2008-08-01 10:17) - PLID 30917 - not anymore, we don't change the color of this button now

	CTableChecker m_companyChecker, m_groupsChecker, m_labelChecker;

	// (j.jones 2010-04-16 09:01) - PLID 38149 - track the old 33b qualifier
	CString m_strOldBox33bQualifier;

	// (j.jones 2013-08-05 11:46) - PLID 57805 - loads the proper label text for m_lblUpgradeDate
	void LoadHCFAUpgradeDateLabel();

	// (j.jones 2008-04-03 12:06) - PLID 28995 - added OnBtnConfigClaimProviders
	// (j.jones 2008-06-10 14:47) - PLID 25834 - added OnCheckValidateRefPhy
	// (j.jones 2009-03-06 11:39) - PLID 33354 - added controls to update PullCurrentAccInfo
	// Generated message map functions
	//{{AFX_MSG(CHcfaSetup)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeGroup(long iNewRow);
	afx_msg void OnDblClickInscoIn();
	afx_msg void OnDblClickInscoOut();
	afx_msg void OnAddAll();
	afx_msg void OnAddCompany();
	afx_msg void OnRemoveCompany();
	afx_msg void OnRemoveAllCompanies();
	afx_msg void On33BillProvider();
	afx_msg void On33MainProvider();
	afx_msg void On33BillLocation();
	afx_msg void On33Override();
	afx_msg void OnDblClickCellHcfaSelected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellHcfaUnselected(long nRowIndex, short nColIndex);
	afx_msg void OnChangePhone();
	afx_msg void OnEditInsInfo();
	afx_msg void OnAlignForm();
	afx_msg void OnNewGroup();
	afx_msg void OnDeleteGroup();
	afx_msg void OnSelChosen17a(long nRow);
	afx_msg void OnSelChosen33(long nRow);	
	afx_msg void OnAdvEbillingSetup();
	afx_msg void OnSelChosenBatchCombo(long nRow);
	afx_msg void OnSelChosenSecondaryBatchCombo(long nRow);
	afx_msg void OnCheckHideInsInfo();	
	afx_msg void OnBtnHcfaDates();
	afx_msg void OnSelChosenHcfaGroups(long nRow);
	afx_msg void OnBtnAdvPinSetup();
	afx_msg void OnCheckUseEmployer();
	afx_msg void OnCheckHidePhoneBox33();
	afx_msg void OnAdvPosCodeSetup();
	afx_msg void OnSelChosenPhoneBox33(long nRow);
	afx_msg void OnSelChosenBox24CCombo(long nRow);	
	afx_msg void OnCheckRefphyidInBox19();	
	afx_msg void OnCheckDefaultInBox19();	
	afx_msg void OnBtnDoNotFill();	
	afx_msg void OnCheckShowRespName();
	afx_msg void OnCheckUse23In17a();
	afx_msg void OnCheckUse23In17aAndNot23();
	afx_msg void OnCheckUseOverrideNameInBox31();
	afx_msg void OnCheckUse1aIn9a();
	afx_msg void OnCheckHidePhoneNum();
	afx_msg void OnCheckUse1aIn9aAlways();
	afx_msg void OnCheckPutSpaceAfterQualifiers();
	afx_msg void OnBtnConfigClaimProviders();
	afx_msg void OnCheckValidateRefPhy();
	afx_msg void OnRadioAccUseBill();
	afx_msg void OnRadioAccUseInsuredParty();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2010-08-31 16:33) - PLID 40303 - supported TPLIn9a
	afx_msg void OnCheckSendTPLIn9a();
	// (j.jones 2013-06-10 16:26) - PLID 56255 - added UseRefPhyGroupNPI
	afx_msg void OnCheckUseRefPhyGroupNpi();
	// (j.jones 2013-07-17 15:49) - PLID 41823 - added DontBatchSecondary
	afx_msg void OnCheckDontBatchSecondary();
	// (j.jones 2013-07-17 15:58) - PLID 41823 - added 'DontBatchSecondary' exclusions
	afx_msg void OnBtnEditSecondaryExclusions();
	// (j.jones 2013-08-05 11:46) - PLID 57805 - added link to the HCFA upgrade date setup
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HCFASETUP_H__06898FB6_6F5E_11D3_AD6B_00104B318376__INCLUDED_)