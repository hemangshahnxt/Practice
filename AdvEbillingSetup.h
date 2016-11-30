#if !defined(AFX_ADVEBILLINGSETUP_H__C308CAD8_648B_40CF_8207_128D37CD8A7D__INCLUDED_)
#define AFX_ADVEBILLINGSETUP_H__C308CAD8_648B_40CF_8207_128D37CD8A7D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvEbillingSetup.h : header file
//

#include "AdministratorRc.h"

/////////////////////////////////////////////////////////////////////////////
// CAdvEbillingSetup dialog

class CAdvEbillingSetup : public CNxDialog
{
// Construction
public:

	CAdvEbillingSetup(CWnd* pParent);   // standard constructor

	long m_GroupID;
	BOOL m_bIsUB92;

	// (j.jones 2008-04-01 17:39) - PLID 29486 - added m_checkHide2330AREF
	// (z.manning, 04/30/2008) - PLID 29850 - Added NxIconButtons
	// (j.jones 2008-05-12 17:09) - PLID 29986 - removed all NSF controls
	// (j.jones 2008-06-23 09:42) - PLID 30434 - added m_btnEligibilitySetup
	// (j.jones 2008-10-06 10:07) - PLID 31580 - added 2300/2400 NTE radio buttons for Send Correspondence 
	// (j.jones 2008-10-06 13:00) - PLID 31578 - added 2300/2400 NTE radio buttons for HCFA Box 19
	// (j.jones 2008-11-12 12:23) - PLID 32010 - added m_nxstaticCorrespondenceLabel2
	// (j.jones 2008-12-11 14:55) - PLID 32413 - added 2310E controls
// Dialog Data
	//{{AFX_DATA(CAdvEbillingSetup)
	enum { IDD = IDD_ADV_EBILLING_SETUP };
	CNxStatic	m_nxstatic2310ELabel;
	NxButton	m_checkUse2310E;
	CNxEdit		m_nxedit2310EQualifier;
	CNxEdit		m_nxedit2310ENumber;
	NxButton m_radioSendBox19_2400NTE;
	NxButton m_radioSendBox19_2300NTE;
	NxButton m_radioSendCorresp2400NTE;
	NxButton m_radioSendCorresp2300NTE;
	CNxIconButton	m_btnEligibilitySetup;
	NxButton	m_btn2420A;
	NxButton	m_btn2310B;
	NxButton	m_btn2010AB;
	NxButton	m_btn2010AA;
	NxButton	m_checkHide2330AREF;
	NxButton	m_checkUseSecondaryInsCode;
	NxButton	m_radioExtraRefUseEIN;
	NxButton	m_radioExtraRefUseNPI;
	NxButton	m_radioExtraRefUseNone;
	NxButton	m_radioNM109UseNPI;
	NxButton	m_radioNM109UseEIN;
	NxButton	m_radio2000APRVWhenNotGroup;
	NxButton	m_radio2000APRVAlways;
	NxButton	m_radio2000APRVNever;
	NxButton	m_checkUseAnesthMinutes;
	NxButton	m_checkSendBox19;
	NxButton	m_checkSendRefPhyIn2300;
	NxButton	m_checkUse2310B;
	NxButton	m_checkUse2310BPRVSegment;
	CNxEdit	m_nxedit2010AaQualifier;
	CNxEdit	m_nxedit2010AaNumber;
	CNxEdit	m_nxedit2010AbQualifier;
	CNxEdit	m_nxedit2010AbNumber;
	CNxEdit	m_nxedit2310BQualifier;
	CNxEdit	m_nxedit2310BNumber;
	CNxEdit	m_nxedit2420AQualifier;
	CNxEdit	m_nxedit2420ANumber;
	CNxEdit	m_nxeditEditCorrespondenceDate;
	CNxStatic	m_nxstaticFormatLabel;
	CNxStatic	m_nxstaticGroupName;
	CNxStatic	m_nxstatic2010AaText;
	CNxStatic	m_nxstatic2010AbText;
	CNxStatic	m_nxstatic2310BText;
	CNxStatic	m_nxstatic2420AText;
	CNxStatic	m_nxstaticAnsiNm109Label;
	CNxStatic	m_nxstaticAnsiExtraRefLabel;
	CNxStatic	m_nxstaticRadio2000APrvLabel;
	CNxStatic	m_nxstaticCorrespondenceLabel;
	CNxStatic	m_nxstaticCorrespondenceLabel2;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	// (j.jones 2010-04-19 12:13) - PLID 38265 - added ability to send the UB Statement Date as a range
	NxButton	m_checkUBAlwaysSendStatementDateRange;
	// (j.jones 2010-08-30 16:53) - PLID 15025 - added TPL options
	NxButton	m_radioSendTPLIn2330B;
	NxButton	m_radioSendTPLIn2430;
	NxButton	m_radioSendTPLInBoth;
	NxButton	m_radioDoNotSendTPL;
	// (j.jones 2010-11-01 09:26) - PLID 40919 - added ability to edit the 2010AB address
	CNxIconButton	m_btnEdit2010AB_Address;
	// (j.jones 2011-11-16 16:11) - PLID 46489 - added ability to edit the 2010AB address
	CNxIconButton	m_btnEdit2010AA_Address;
	// (j.jones 2012-05-14 10:10) - PLID 50338 - added ability to control 2000B/2320 SBR04
	// (j.jones 2012-10-04 10:07) - PLID 53018 - removed old SBR04 controls, and added
	// controls for 2000B SBR03, 2000B SBR04, 2320 SBR04
	NxButton	m_checkHide2000BSBR03;
	NxButton	m_radioSend2000BSBR04Always;
	NxButton	m_radioSend2000BSBR04WhenGroupBlank;
	NxButton	m_radioSend2000BSBR04Never;
	NxButton	m_radioSend2320SBR04Always;
	NxButton	m_radioSend2320SBR04WhenGroupBlank;
	NxButton	m_radioSend2320SBR04Never;
	// (j.jones 2013-04-11 09:01) - PLID 56166 - added ANSI_HideRefPhyFields
	NxButton	m_checkHideRefPhyFields;
	// (j.jones 2013-06-20 11:39) - PLID 57245 - added OrigRefNo_2300 and OrigRefNo_2330B
	NxButton	m_checkOrigRefNo_2300;
	NxButton	m_checkOrigRefNo_2330B;
	// (j.jones 2013-09-05 10:18) - PLID 58252 - added taxonomy code overrides
	CNxStatic	m_nxstatic2310BTaxonomyLabel;
	CNxStatic	m_nxstatic2420ATaxonomyLabel;
	CNxEdit		m_nxedit2000ATaxonomy;
	CNxEdit		m_nxedit2310BTaxonomy;
	CNxEdit		m_nxedit2420ATaxonomy;
	// (j.jones 2014-01-22 09:49) - PLID 60034 - added ordering provider settings
	CNxStatic	m_nxstaticOrderingProviderLabel;
	NxButton	m_radioOrderingProviderServiceCode;
	NxButton	m_radioOrderingProviderAlways;
	NxButton	m_radioOrderingProviderNever;
	// (j.jones 2014-04-23 13:47) - PLID 61840 - added Hide 2310A settings
	CNxStatic	m_nxstaticHide2310ALabel;
	// (b.spivey January 26, 2015) - PLID 64452 - added SendN3N4PERsegment
	NxButton	m_checkSendN3N4PERSegment; 
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvEbillingSetup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;

	// (j.jones 2010-10-19 10:02) - PLID 40931 - cache the value for 5010
	BOOL m_bIs5010Enabled;

	long m_ProviderID;
	long m_LocationID;

	CString m_strGroupName;

	NXDATALISTLib::_DNxDataListPtr m_ProvList;
	NXDATALISTLib::_DNxDataListPtr m_LocList;
	// (j.jones 2008-02-06 16:16) - PLID 28843 - added secondary ins. code setting
	NXDATALIST2Lib::_DNxDataListPtr m_SecondaryCodeList;
	// (j.jones 2009-11-24 15:49) - PLID 36411 - added Prior Auth. Qualifier list
	NXDATALIST2Lib::_DNxDataListPtr m_PriorAuthQualifierCombo;
	// (j.jones 2012-03-23 15:41) - PLID 49176 - added 2000A taxonomy dropdown
	NXDATALIST2Lib::_DNxDataListPtr m_2000ATaxonomyCombo;
	// (j.jones 2014-04-23 13:47) - PLID 61840 - added Hide 2310A settings
	NXDATALIST2Lib::_DNxDataListPtr m_Hide2310ACombo;

	void Load();

	// (j.jones 2007-02-20 10:42) - PLID 23953 - converted the Save function to return
	// a boolean to determine success or failure
	BOOL Save();
	
	BOOL m_bHasChanged;
	BOOL m_bIsLoading;

	// (j.jones 2008-04-29 15:18) - PLID 29619 - validates an individual ANSI ID and qualifier,
	// if both are blank then the strLabel is added to strBlankOverrideNames,
	// if only one or the other is blank then the strLabel is added to strInvalidOverrideNames
	// (j.jones 2010-04-15 17:28) - PLID 38149 - we now warn when the qualifier is XX, using strXXOverrideNames
	void ValidateOneANSIOverride(const CString strLabel, BOOL bFieldChanged, const CString strQual, const CString strID, CString &strInvalidOverrideNames, CString &strBlankOverrideNames, CString &strXXOverrideNames);

	// (j.jones 2008-04-29 16:23) - PLID 29619 - track whether any individual override was modified
	BOOL m_b2010AAChanged;
	BOOL m_b2010ABChanged;
	BOOL m_b2310BChanged;
	BOOL m_b2420AChanged;

	// (j.jones 2009-08-03 14:36) - PLID 33827 - track the old Export2310BRecord value
	BOOL m_bOldExport2310BRecord;

	// (j.jones 2008-05-22 14:46) - PLID 29886 - added stored variables for use when auditing
	BOOL m_bOldUse2010AA;
	CString m_strOld2010AAQual;
	CString m_strOld2010AA;
	BOOL m_bOldUse2010AB;
	CString m_strOld2010ABQual;
	CString m_strOld2010AB;
	BOOL m_bOldUse2310B;
	CString m_strOld2310BQual;
	CString m_strOld2310B;
	BOOL m_bOldUse2420A;
	CString m_strOld2420AQual;
	CString m_strOld2420A;
	long m_nOldNM109;
	long m_nOldExtraREF;

	// (j.jones 2008-12-11 14:55) - PLID 32413 - added 2310E fields
	BOOL m_b2310EChanged;
	BOOL m_bOldUse2310E;
	CString m_strOld2310EQual;
	CString m_strOld2310E;

	// (j.jones 2013-09-05 10:18) - PLID 58252 - added taxonomy code overrides
	CString m_strOld2000ATaxonomyCode;
	CString m_strOld2310BTaxonomyCode;
	CString m_strOld2420ATaxonomyCode;

	// (j.jones 2008-05-22 14:55) - PLID 29886 - returns provider and location names for consistent auditing
	CString GetFullAuditInformation();

	// (j.jones 2010-11-01 09:47) - PLID 40919 - updates the 2010AB button to reflect if it is in use
	void Update2010ABInUseLabel();
	// (j.jones 2011-11-16 16:11) - PLID 46489 - updates the 2010AA button to reflect if it is in use
	void Update2010AAInUseLabel();

	// (j.jones 2008-06-23 09:44) - PLID 30434 - added OnBtnEligibilitySetup
	// Generated message map functions
	// (j.jones 2008-10-06 13:19) - PLID 31578 - added OnBnClickedCheckSendBox19
	//{{AFX_MSG(CAdvEbillingSetup)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenAdvEbillProviders(long nRow);
	// (j.jones 2009-08-03 14:03) - PLID 33827 - removed OnCheckUse2310B() because
	// we no longer enable/disable anything through it
	afx_msg void OnBtnSecondaryClaimSetup();
	afx_msg void OnSelChosenAdvEbillLocations(long nRow);
	afx_msg void OnCheckSecondaryInsTypeCode();
	afx_msg void OnBtnEligibilitySetup();
	afx_msg void OnBnClickedCheckSendBox19();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	
	// (j.jones 2009-12-10 17:36) - PLID 36411 - we will flag that these fields changed
	void OnSelChangingPriorAuthQualCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChangingSecondaryInsCodeDropdown(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	// (j.jones 2010-11-01 09:28) - PLID 40919 - added ability to override the 2010AB address
	afx_msg void OnBtnEdit2010ABAddress();
	// (j.jones 2011-11-16 16:11) - PLID 46489 - added ability to override the 2010AB address
	afx_msg void OnBtnEdit2010AAAddress();
	void OnSelChanging2000aTaxonomyCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	// (j.jones 2014-04-23 13:47) - PLID 61840 - added Hide 2310A settings
	void OnSelChangingHide2310aCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVEBILLINGSETUP_H__C308CAD8_648B_40CF_8207_128D37CD8A7D__INCLUDED_)