#if !defined(AFX_PRACTICEINFO_H__0E582594_C2A1_11D2_A500_00104B2FE914__INCLUDED_)
#define AFX_PRACTICEINFO_H__0E582594_C2A1_11D2_A500_00104B2FE914__INCLUDED_

#include "stdafx.h"
#include "client.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

// PracticeInfo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPracticeInfo dialog

class CPracticeInfo : public CNxDialog
{
// Construction
public:
	CPracticeInfo(CWnd* pParent);   // standard constructor
	
	void FormatDlgItem(UINT nId);
	bool PreKillFocusZipCode();
	// (j.gruber 2009-10-07 17:30) - PLID 35826 - added prekillfocuscity
	bool PreKillFocusCity();
	bool PreKillFocusOfficeHours(UINT nId);
	CString GetCtrlFieldName(UINT nId);
	CString GetCtrlDefaultString(UINT nId);
	UINT GetCtrlPartnerId(UINT nId);
	bool PreKillFocusDlgItem(UINT nId);
	CBrush m_brush;

	CTableChecker m_ProviderChecker, m_PatCoordChecker;


// Dialog Data
	// (a.walling 2009-03-30 09:38) - PLID 33729 - Checkbox to link with pharmacy directory
	//{{AFX_DATA(CPracticeInfo)
	enum { IDD = IDD_PRACTICE };
	CNxIconButton	m_btnEditPlaceOfService;
	CNxIconButton	m_btnLogo;
	CNxIconButton	m_btnImage;
	CNxIconButton	m_btnBiography;
	NxButton	m_checkDefaultForNewPatients;
	NxButton	m_buActive;
	NxButton	m_checkLinkWithDirectory;
	CNxIconButton	m_btnPracticeLeft;
	CNxIconButton	m_btnPracticeRight;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_remAllBtn;
	CNxIconButton	m_remBtn;
	CNxIconButton	m_addAllBtn;
	CNxIconButton	m_addBtn;
	CNxIconButton	m_btnEditPharmacyIDs;
	CNxIconButton	m_btnEditPharmacyStaff;
	NxButton	m_managed;
	//long	m_lID; // this is not used anywhere, removed so noone else gets confused
	CNxEdit	m_nxeditName;
	CNxEdit	m_nxeditPracaddress1;
	CNxEdit	m_nxeditPracaddress2;
	CNxEdit	m_nxeditPraczip;
	CNxEdit	m_nxeditPraccity;
	CNxEdit	m_nxeditPracstate;
	CNxEdit	m_nxeditWebsite;
	CNxEdit	m_nxeditPracein;
	CNxEdit	m_nxeditPracNpi;
	CNxEdit	m_nxeditPractaxrate;
	CNxEdit	m_nxeditPractaxrate2;
	CNxEdit	m_nxeditPracmainnum;
	CNxEdit	m_nxeditPractollfree;
	CNxEdit	m_nxeditPracaltnum;
	CNxEdit	m_nxeditModem;
	CNxEdit	m_nxeditPracfaxnum;
	CNxEdit	m_nxeditPracemail;
	CNxEdit	m_nxeditOpensun;
	CNxEdit	m_nxeditClosesun;
	CNxEdit	m_nxeditOpenmon;
	CNxEdit	m_nxeditClosemon;
	CNxEdit	m_nxeditOpentues;
	CNxEdit	m_nxeditClosetues;
	CNxEdit	m_nxeditOpenwed;
	CNxEdit	m_nxeditClosewed;
	CNxEdit	m_nxeditOpenthur;
	CNxEdit	m_nxeditClosethur;
	CNxEdit	m_nxeditOpenfri;
	CNxEdit	m_nxeditClosefri;
	CNxEdit	m_nxeditOpensat;
	CNxEdit	m_nxeditClosesat;
	CNxEdit	m_nxeditPracnotesBox;
	CNxStatic m_nxstaticLocationNpiLabel;
	// (j.jones 2012-03-21 11:58) - PLID 48155 - added NxStatic for the POS Code label
	CNxStatic m_nxstaticPOSCodeLabel;
	// (j.jones 2012-03-23 14:23) - PLID 42388 - added taxonomy code
	CNxEdit	m_nxeditTaxonomy;
	// (b.spivey, March 27, 2012) - PLID 47521 - added abbreviation
	CNxEdit m_nxeditAbbreviation; 
	CNxStatic m_nxstaticAbbreviationLabel;
	// (j.fouts 2013-06-07 09:19) - PLID 57047 - Removed SPI Root from Locations Tab
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPracticeInfo)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);	
	//}}AFX_VIRTUAL

// Implementation
protected:
	void EnsureButtons();
	NXDATALISTLib::_DNxDataListPtr m_placeOfService;
	NXDATALISTLib::_DNxDataListPtr m_list;
	NXDATALISTLib::_DNxDataListPtr m_provider;
	NXDATALISTLib::_DNxDataListPtr m_coordinator;
	NXDATALISTLib::_DNxDataListPtr m_allowed;
	NXDATALISTLib::_DNxDataListPtr m_disallowed;
	NXDATALIST2Lib::_DNxDataListPtr m_pTypeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlDefaultLab; // (z.manning 2010-01-11 10:43) - PLID 24044
	// (s.tullis 2016-03-07 11:56) - PLID 68444 
	NXDATALIST2Lib::_DNxDataListPtr m_pdlDefaultClaimForm;
	enum EDefaultLabComboColumns {
		dlccID = 0,
		dlccLabName,
	};
	
	long m_CurSelect;
	void SetDlgItemTime(int nID, const COleVariant &var);
	void Save(int nID);
	void Load();
	void LoadUsers();
	void FillAreaCode(long nID);
	bool SaveAreaCode(long nID);
	CString m_strAreaCode;
	BOOL m_bFormatPhoneNums;
	CString m_strPhoneFormat;

	// (j.gruber 2009-10-08 09:56) - PLID 35826 - Override the tab order if they have city lookup	
	BOOL m_bLookupByCity;

	// (a.walling 2008-05-19 17:32) - PLID 27810 - We don't warn unless the ID has been changed.
	CString m_strOriginalNPI;
	
	// (j.jones 2012-03-23 14:29) - PLID 42388 - track the old taxonomy code
	CString m_strPrevTaxonomyCode;

	// (b.spivey, March 28, 2012) - PLID 47521 - track previous abbreviation for auditing. 
	CString m_strPrevAbbrev;

	// (j.jones 2008-10-08 17:26) - PLID 31596 - removes favorite pharmacies, and puts the results in a batch statement
	void RemoveFavoritePharmacy(CString &strSqlBatch, long nPharmacyID);

	//DRT 11/19/2008 - PLID 32081
	void EnsureTypeSpecificElements(long nCurrentTypeID);

	// Generated message map functions
	//{{AFX_MSG(CPracticeInfo)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddlocation();
	afx_msg void OnDeletelocation();
	afx_msg void OnManaged();
	afx_msg void OnDblClickCellAllowed(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellDisallowed(long nRowIndex, short nColIndex);
	afx_msg void OnAdd();
	afx_msg void OnAddAll();
	afx_msg void OnRemove();
	afx_msg void OnRemoveAll();
	afx_msg void OnEditPlaceOfService();
	afx_msg void OnSelChosenLocation(long nRow);
	afx_msg void OnSelChosenProvider(long nRow);
	afx_msg void OnSelChosenPlaceOfService(long nRow);
	afx_msg void OnSelChosenCoordinator(long nRow);
	afx_msg void OnPracticeRight();
	afx_msg void OnPracticeLeft();
	afx_msg void OnLocationActive();
	afx_msg void OnCheckDefaultForNewPatients();
	afx_msg void OnSelChangedLocationType(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel); // (b.cardillo 2006-11-28 16:58) - PLID 23682 - Made this SelChanged instead of SelChosen; see .cpp comments on this function for more info.
	afx_msg void OnSelChangingLocationType(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnButtonLocationBiography();
	afx_msg void OnButtonLocationLogo();
	afx_msg void OnButtonLocationImage();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedEditPharmacyId();
	afx_msg void OnBnClickedEditPharmacyStaff();
	afx_msg void OnBnClickedLinkWithDirectory();
	afx_msg void OnSelChosenDefaultLab(LPDISPATCH lpRow); // (z.manning 2010-01-11 10:59) - PLID 24044
public:
	// (s.tullis 2016-03-07 11:56) - PLID 68444 
	void SelChosenLocationDefaultclaimform(LPDISPATCH lpRow);
	void InitClaimFormList();
	void ShowHideDefaultClaimFormControls(bool bShow = true);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRACTICEINFO_H__0E582594_C2A1_11D2_A500_00104B2FE914__INCLUDED_)
