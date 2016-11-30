//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_CONTACTSGENERAL_H__CF6A8C94_55AF_11D3_AD64_00104B318376__INCLUDED_)
#define AFX_CONTACTSGENERAL_H__CF6A8C94_55AF_11D3_AD64_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ContactsGeneral.h : header file
//
#include "PracProps.h"
#include "Client.h"
#include "AuditTrail.h"
#include <NxUILib/NxStaticIcon.h>	// (a.wilson 2014-04-24) PLID 61825
#include <Guard.h>

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;
// using namespace NXTIMELib;

/////////////////////////////////////////////////////////////////////////////
// CContactsGeneral dialog

class CContactsGeneral : public CNxDialog
{
// Construction
public:
	NXDATALISTLib::_DNxDataListPtr m_PrefixCombo, m_PalmCombo, m_LocationCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_ClaimProviderCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pAMASpecialtyList;		//DRT 11/20/2008 - PLID 32082
	CContactsGeneral(CWnd* pParent);   // standard constructor
	virtual void RecallDetails();
	virtual void StoreDetails();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	void UpdateToolbar();
	void UpdateEmailButton();
	// (j.jones 2014-08-08 13:34) - PLID 63250 - we no longer have a ConfigRT tablechecker
	CTableChecker m_labelChecker, m_locationsChecker, m_providerChecker;
	void AuditField(AuditEventItems aeiItem, CString strOld, CString strNew, AuditEventPriorities aepPriority = aepMedium, AuditEventTypes aetType = aetChanged);

	void SecureControls();
	// (s.tullis 2015-10-29 17:27) - PLID 67483 - Show/Hide Capitation
	void ShowCapitation(BOOL bEnable = TRUE);

	// (z.manning, 06/06/2007) - PLID 23862 - Made all butons on contacts => general tab NxIconButtons.
	// (z.manning, 12/12/2007) - PLID 28216 - Added a button for attendance setup.
// Dialog Data
	//{{AFX_DATA(CContactsGeneral)
	enum { IDD = IDD_CONTACTS_GENERAL };
	//(a.wilson 2014-04-23) PLID 61825 - new control to display icon with tooltip.
	CNxStaticIcon m_icoProviderTypeInfo;
	NxButton	m_btnNurse;
	NxButton	m_btnAnesthesiologist;
	NxButton m_checkInactive;
	NxButton m_male;
	NxButton m_female;
	NxButton m_btnPatCoord;
	// (d.lange 2011-03-22 12:16) - PLID 42943 - Added checkbox for Assistant/Technician
	NxButton m_btnTechnician;
	// (j.dinatale 2012-04-05 15:33) - PLID 49332 - Added Optician checkbox
	NxButton m_btnOptician;
	CString	m_strContactType;
	CNxIconButton m_btnEmail;
	CNxIconButton m_btnEditPrefixes;
	CNxIconButton m_btnProperties;
	CNxIconButton m_btnShowCommision;
	CNxIconButton m_btnShowProcs;
	CNxIconButton m_btnPermissions;
	CNxIconButton m_btnReferredPats;
	CNxIconButton m_btnReferredProspects;
	CNxIconButton m_btnEditSecurityGroups;
	CNxIconButton m_btnEditBiography;
	CNxIconButton m_btnSelectImage;
	CNxIconButton m_btnAttendanceSetup;
	CNxEdit	m_nxeditEmployerBox;
	CNxEdit	m_nxeditAccountBox;
	CNxEdit	m_nxeditTitleBox;
	CNxEdit	m_nxeditFirstNameBox;
	CNxEdit	m_nxeditMiddleNameBox;
	CNxEdit	m_nxeditLastNameBox;
	CNxEdit	m_nxeditAddress1Box;
	CNxEdit	m_nxeditAddress2Box;
	CNxEdit	m_nxeditZipBox;
	CNxEdit	m_nxeditCityBox;
	CNxEdit	m_nxeditStateBox;
	CNxEdit	m_nxeditMarriageOtherBox;
	CNxEdit	m_nxeditSsnBox;
	CNxEdit	m_nxeditHomePhoneBox;
	CNxEdit	m_nxeditWorkPhoneBox;
	CNxEdit	m_nxeditExtPhoneBox;
	CNxEdit	m_nxeditEmailBox;
	CNxEdit	m_nxeditCellPhoneBox;
	CNxEdit	m_nxeditPagerPhoneBox;
	CNxEdit	m_nxeditOtherPhoneBox;
	CNxEdit	m_nxeditFaxBox;
	CNxEdit	m_nxeditEmergencyFirstName;
	CNxEdit	m_nxeditEmergencyLastName;
	CNxEdit	m_nxeditRelationBox;
	CNxEdit	m_nxeditOtherworkBox;
	CNxEdit	m_nxeditOtherhomeBox;
	CNxEdit	m_nxeditNpiBox;
	CNxEdit	m_nxeditAccountName;
	CNxEdit	m_nxeditNationalnumBox;
	CNxEdit	m_nxeditFedidBox;
	CNxEdit	m_nxeditMethodBox;
	CNxEdit	m_nxeditWorkcompBox;
	CNxEdit	m_nxeditMedicaidBox;
	CNxEdit	m_nxeditLicenseBox;
	CNxEdit	m_nxeditBcbsBox;
	CNxEdit	m_nxeditUpinBox;
	CNxEdit	m_nxeditMedicareBox;
	CNxEdit	m_nxeditOtheridBox;
	CNxEdit	m_nxeditDeaBox;
	CNxEdit	m_nxeditRefphysidBox;
	CNxEdit	m_nxeditTaxonomyCode;
	CNxEdit	m_nxeditConCustom1Box;
	CNxEdit	m_nxeditConCustom2Box;
	CNxEdit	m_nxeditConCustom3Box;
	CNxEdit	m_nxeditConCustom4Box;
	CNxEdit	m_nxeditNotes;
	CNxEdit	m_nxeditNumPatientsRefBox;
	CNxEdit	m_nxeditNumProspectsRefBox;
	CNxEdit	m_nxeditDefaultCostEdit;
	CNxEdit m_editSPI;
	// (s.tullis 2015-10-29 17:27) - PLID 67483- Capitation Controls
	CNxEdit m_editCaptitationDistribution;
	CNxStatic	m_nxstaticCapitationDistribution;
	CNxStatic	m_nxstaticCapitationDistributionPercentsign;
	CNxStatic	m_nxstaticContactType;
	CNxStatic	m_nxstaticLabelNpi;
	CNxStatic	m_nxstaticLabel26;
	CNxStatic	m_nxstaticDateOfHireLabel;
	CNxStatic	m_nxstaticDateOfTermLabel;
	CNxStatic	m_nxstaticNumPatientsRefLabel;
	CNxStatic	m_nxstaticNumProspectsRefLabel;
	CNxStatic	m_nxstaticDefaultCostLabel;
	CNxStatic	m_nxstaticCustom1Label;
	CNxStatic	m_nxstaticCustom2Label;
	CNxStatic	m_nxstaticCustom3Label;
	CNxStatic	m_nxstaticCustom4Label;
	CNxStatic	m_nxstaticIdLabel12;
	CNxStatic	m_nxstaticLabel22;
	CNxStatic	m_nxstaticLabel5;
	CNxStatic	m_nxstaticLabel34;
	CNxStatic	m_nxstaticLabel35;
	CNxStatic	m_nxstaticFaxLabel;
	CNxStatic	m_nxstaticLabel6;
	CNxStatic	m_nxstaticLabel10;
	CNxStatic	m_nxstaticLabel8;
	CNxStatic	m_nxstaticLabel9;
	CNxStatic	m_nxstaticLabel23;
	CNxStatic	m_nxstaticClaimProvLabel;
	CNxStatic	m_nxstaticLabel30;
	CNxStatic	m_nxstaticLabel29;
	CNxStatic	m_nxstaticLabel27;
	CNxStatic	m_nxstaticLabel32;
	CNxStatic	m_nxstaticLabel33;
	CNxStatic	m_nxstaticLabel28;
	CNxStatic	m_nxstaticLabelTaxonomyCode;
	CNxStatic	m_nxstaticLabel31;
	CNxStatic	m_nxstaticLabel24;
	CNxStatic	m_nxstaticMethodLabel;
	CNxStatic	m_nxstaticAccountNameLabel;
	CNxStatic	m_nxstaticLabel19;
	CNxStatic	m_nxstaticLabelDeflocation;
	// (j.jones 2009-06-26 09:13) - PLID 34292 - added OHIP Specialty
	CNxEdit		m_nxeditOHIPSpecialty;
	CNxStatic	m_nxstaticLabelOHIPSpecialty;
	// (j.jones 2011-11-07 14:10) - PLID 46299 - added ProvidersT.UseCompanyOnClaims
	NxButton	m_checkSendCompanyOnClaim;
	// (j.jones 2013-06-10 13:26) - PLID 57089 - added Group NPI
	CNxEdit		m_nxeditGroupNPIBox;
	CNxStatic	m_nxstaticLabelGroupNPI;
	// (b.savon 2013-06-03 10:55) - PLID 56867
	CNxIconButton	m_btnRegisterPrescriber;
	// (b.spivey -- October 16th, 2013) - PLID 59022 - Add an edit control for Direct Address.
	CNxEdit		m_nxeditDirectAddress; 
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CContactsGeneral)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXTIMELib::_DNxTimePtr m_nxtBirthDate;	
	COleDateTime m_dtBirthDate;
	CBoolGuard m_bSavingBirthDate; // (a.walling 2011-08-29 12:20) - PLID 45232 - Use safe guards

	NXTIMELib::_DNxTimePtr m_nxtDateOfHire;
	COleDateTime m_dtDateOfHire;
	CBoolGuard m_bSavingDateOfHire; // (a.walling 2011-08-29 12:20) - PLID 45232 - Use safe guards

	// (a.walling 2007-11-21 15:13) - PLID 28157
	NXTIMELib::_DNxTimePtr m_nxtDateOfTerm;
	COleDateTime m_dtDateOfTerm;
	CBoolGuard m_bSavingDateOfTerm; // (a.walling 2011-08-29 12:20) - PLID 45232 - Use safe guards

	void AuditContactChange(long nID, long status, CString strNew, CString strOld);
	int GetFieldID(int nID);
	BOOL ChangeLabel(const int nID);
	void UpdatePalm();
//	void EnsureRecordset(bool requery = false);
	void Load();
	void Save(int nID);
//	void SetDlgItemVar(int nID, const COleVariant &var);
	void GetDlgItemVar(int nID, COleVariant &var, unsigned short vt);
	long m_id;		//used in place of GetActiveContactID()
	void SendContactsTablecheckerMsg(); 
	void FillAreaCode(long nID);
	bool SaveAreaCode(long nID);
	CString m_strAreaCode;
	BOOL m_bFormatPhoneNums;
	CString m_strPhoneFormat;
	CBrush m_brush;
	long m_PendingClaimProvID;

	// (b.spivey -- October 24th, 2013) - PLID 59022 - Add a function for the direct address button. 
	void UpdateDirectAddressButton();

	//DRT 8/6/2007 - For PLID 26970
	CBoolGuard m_bIsSaving; // (a.walling 2011-08-29 12:20) - PLID 45232 - Use safe guards

	// (j.gruber 2009-10-08 09:56) - PLID 35825 - Override the tab order if they have city lookup
	BOOL m_bLookupByCity;

	// (b.savon 2013-04-23 13:56) - PLID 56409
	BOOL m_bIsConfiguredNexERxPrescriber;
	BOOL IsProviderConfiguredForNexERx();

	// (a.walling 2007-11-21 15:14) - PLID 28157 - Added OnKillFocusDateOfTerm
	// Generated message map functions
	//{{AFX_MSG(CContactsGeneral)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangePrefix(long iNewRow);
	afx_msg void OnChangePalm(long iNewRow);
	afx_msg void OnClickPatcoordCheck();
	afx_msg void OnClickTechnicianCheck();
	afx_msg void OnPermissionsBtn();
	afx_msg void OnPropertiesBtn();
	afx_msg void OnClickEmail();	
	afx_msg void OnFemale();
	afx_msg void OnMale();
	afx_msg void OnEditsecuritygroupsBtn();
	afx_msg void OnNurseCheck();
	afx_msg void OnAnesthesiologist();
	afx_msg void OnKillFocusBdateBox();
	afx_msg void OnEditPrefixes();
	afx_msg void OnSelChosenPrefixList(long nRow);
	afx_msg void OnSelChosenContactLocation(long nRow);
	afx_msg void OnCheckContactInactive();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnShowCommission();
	afx_msg void OnShowProcs();
	afx_msg void OnKillFocusDateOfHire();
	afx_msg void OnKillFocusDateOfTerm();
	afx_msg void OnReferredPats();
	afx_msg void OnReferredProspects();
	afx_msg void OnSelChosenClaimProvider(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedClaimProvider(short nFlags);
	afx_msg void OnTrySetSelFinishedClaimProvider(long nRowEnum, long nFlags);
	afx_msg void OnEditBiography();
	afx_msg void OnSelectImage();
	afx_msg void OnAttendanceSetup();
	afx_msg void OnTrySetSelFinishedProviderlocationCombo(long nRowEnum, long nFlags);
	afx_msg void OnBnClickedAffiliatePhys(); // (j.gruber 2011-09-22 10:54) - PLID 45354
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void OnSelChosenAMASpecialtyList(LPDISPATCH lpRow);
	// (j.jones 2011-11-07 14:23) - PLID 46299 - added ProvidersT.UseCompanyOnClaims
	afx_msg void OnSendCompanyOnClaim();
	void SelChangingClaimProvider(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnBnClickedOpticianCheck();
	afx_msg void OnBnClickedBtnRegisterPrescriber();
	afx_msg void OnBnClickedDirectAddressUsersButton();
	afx_msg void OnBnClickedContactReferringProviderCheck();
	afx_msg void OnBnClickedContactOrderingProviderCheck();
	afx_msg void OnBnClickedContactSupervisingProviderCheck();
	void UpdateProviderType(const CString& strField, const long& nNewValue);
public:
	afx_msg void OnBnClickedContactConfigureProviderTypesButton();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTACTSGENERAL_H__CF6A8C94_55AF_11D3_AD64_00104B318376__INCLUDED_)
