//{{AFX_INCLUDES()
#include "progressbar.h"
//}}AFX_INCLUDES
#if !defined(AFX_EBILLINGVALIDATIONDLG_H__C48C63A4_263F_4644_BBBB_6D8982F35A64__INCLUDED_)
#define AFX_EBILLINGVALIDATIONDLG_H__C48C63A4_263F_4644_BBBB_6D8982F35A64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EbillingValidationDlg.h : header file
//

enum ANSIVersion;

/////////////////////////////////////////////////////////////////////////////
// CEbillingValidationDlg dialog

class CEbillingValidationDlg : public CNxDialog
{
// Construction
public:
	CEbillingValidationDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2009-08-14 13:48) - PLID 35235 - added optional provider filter
	int DoModal(long HCFAID, long FormatID, long FormatType, long LocationID, long nProviderID, long ClaimType, BOOL bElectronic);

	long m_HCFAID;
	long m_FormatID;
	long m_FormatStyle;
	long m_ClaimType;
	long m_LocationID;

	// (j.jones 2010-10-19 17:33) - PLID 40936 - added ANSIVersion
	ANSIVersion m_avANSIVersion;

	// (j.jones 2009-08-14 13:46) - PLID 35235 - need to support optionally filtering by provider
	long m_nProviderID;

	BOOL m_bElectronic;

	BOOL ValidateAll(int Type);
	BOOL ValidateOne(long HCFAID);
	// (j.jones 2006-12-11 17:00) - PLID 23811 - added OHIP-specific validation function
	BOOL ValidateOneOHIP(long nClaimID);
	// (j.jones 2010-11-01 17:51) - PLID 39594 - added Alberta-specific validation function
	BOOL ValidateOneAlberta(long nClaimID);

	void ShowErrors();

	// (a.walling 2008-05-20 10:12) - PLID 27812 - Added out parameter for valid NPI
	BOOL CheckBox33Pin(long HCFASetupGroupID, long Box33, long ProviderID, long LocationID, long InsuranceCoID, CString &strIDName, BOOL &bValidNPI);

	// (j.jones 2007-03-20 10:01) - PLID 25273 - removed CheckBox33PinANSI, as it became no different
	// from CheckBox33Pin

	// (a.walling 2008-05-20 10:12) - PLID 27812 - Added out parameter for valid NPI
	BOOL CheckBox82(long UB92SetupGroupID, long nBox82Num, long nBox82Setup, long ProviderID, long LocationID, long InsuranceCoID, CString &strIDName, BOOL &bValidNPI);
	// (j.jones 2007-04-25 09:13) - PLID 25277 - used to only validate valid ANSI IDs that we can auto-generate
	// qualifiers for - for ANSI UB92s and not UB04s
	// (j.jones 2007-05-10 11:05) - PLID 25948 - handle referring physicians
	
	// (a.walling 2008-05-20 10:12) - PLID 27812 - Added out parameter for valid NPI
	BOOL CheckBox82ANSILimited(long UB92SetupGroupID, long nBox82Num, long nBox82Setup, long ProviderID, long LocationID, CString &strIDName, BOOL &bValidNPI);

	
	// (a.walling 2008-05-20 10:12) - PLID 27812 - Added out parameter for valid NPI
	BOOL CheckBox17a(long HCFASetupGroupID, long Box17a, long RefPhyID, CString &strIDName, BOOL &bValidNPI);

	CString m_strInvalidClaims;

	// (j.jones 2007-03-01 09:06) - PLID 25015 - tracked a boolean to determine if any errors/warnings were shown
	BOOL m_bErrorListDisplayed;

	// (j.jones 2014-04-25 11:30) - PLID 61912 - added tablechecker for the charge claim provider setup
	CTableChecker m_ClaimProviderChecker;

// Dialog Data
	//{{AFX_DATA(CEbillingValidationDlg)
	enum { IDD = IDD_EBILLING_VALIDATION_DLG };
	CProgressBar	m_progress;
	CNxStatic	m_nxstaticCurrentClaimLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEbillingValidationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.jones 2008-12-10 11:53) - PLID 32312 - OHIP Health Numbers are 10 digit numbers,
	// where the last digit is a check digit, so let's validate that check digit
	BOOL ValidateOHIPPatientHealthNumber(CString strHealthNumber);

	// (j.jones 2010-04-14 11:39) - PLID 27457 - call one function to validate all ANSI loops
	// for a given provider ID
	// (j.jones 2010-04-16 12:00) - PLID 38225 - added bWarnings and bDisableREFXX
	// (j.jones 2010-11-11 09:43) - PLID 41396 - added a parameter to validate billing and/or rendering provider IDs
	void ValidateAllANSIProviderREFIDs(BOOL bIsUBClaim, BOOL bIsRefPhy,
							long nProviderID, BOOL bValidateBillingProvIDs, BOOL bValidateRenderingProvIDs,
							long InsuranceCoID, long nLocationID, long nInsuredPartyID,
							long HCFASetupGroupID, CString strDefaultBox33GRP, long nBox33Setup,
							long UB92SetupGroupID, CString strUB04Box76Qual, long nBox82Num, long nBox82Setup,
							CString &strErrors, BOOL &bPassed, BOOL &bWarnings, BOOL bDisableREFXX,
							CString strProviderLast, CString strProviderFirst,
							BOOL bSend2010AB, BOOL bSend2420A,
							long nExtraREF_IDType, BOOL bIsGroup, long nHCFABox25,
							CString strLocationEIN, CString strProvEIN, CString strProvSSN,
							BOOL bUse_Addnl_2010AA, CString strAddnl_2010AA_Qual, CString strAddnl_2010AA);

	// (j.jones 2010-04-14 11:23) - PLID 27457 - this function will calculate
	// if a loaded ANSI provider ID is invalid, and if so, warn about it
	// (j.jones 2010-04-16 12:00) - PLID 38225 - added bWarnings and bDisableREFXX
	// (j.jones 2010-10-20 11:13) - PLID 40396 - added bIs2010AA, which permits EI, SY, and G5 as valid qualifiers in 5010,
	// otherwise they are illegal in 5010
	void CheckANSIProviderREFID(CString strQualifier, CString strID, CString strLoadedFrom, CString strLoopID,
								   BOOL bIsUBClaim, CString &strErrors, BOOL &bPassed, BOOL &bWarnings, BOOL bDisableREFXX,
								   CString strProviderLast, CString strProviderFirst, BOOL bIs2010AA);

	// Generated message map functions
	//{{AFX_MSG(CEbillingValidationDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EBILLINGVALIDATIONDLG_H__C48C63A4_263F_4644_BBBB_6D8982F35A64__INCLUDED_)
