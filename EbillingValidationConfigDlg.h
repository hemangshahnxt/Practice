#if !defined(AFX_EBILLINGVALIDATIONCONFIGDLG_H__3BB0EE12_074A_47A7_A9E8_E35D9E2DCDFC__INCLUDED_)
#define AFX_EBILLINGVALIDATIONCONFIGDLG_H__3BB0EE12_074A_47A7_A9E8_E35D9E2DCDFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EbillingValidationConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEbillingValidationConfigDlg dialog
#include "FinancialRc.h"

enum ANSIVersion;

class CEbillingValidationConfigDlg : public CNxDialog
{
// Construction
public:
	CEbillingValidationConfigDlg(CWnd* pParent);   // standard constructor
	BOOL m_bIsPaper;
	BOOL m_bIsANSI;
	// (j.jones 2012-10-22 13:36) - PLID 53297 - added ANSI Version
	ANSIVersion m_avANSIVersion;

	// (j.jones 2008-05-07 15:09) - PLID 29854 - added nxiconbuttons for modernization
	// (a.walling 2008-05-20 09:35) - PLID 27812 - Added NPI checksum button
	// (j.jones 2008-05-21 13:46) - PLID PLID 29280 - added provider and ref. phy. NPI options
// Dialog Data
	//{{AFX_DATA(CEbillingValidationConfigDlg)
	enum { IDD = IDD_EBILLING_VALIDATION_CONFIG_DLG };
	// (j.jones 2011-03-15 10:52) - PLID 42788 - added validation for Box 24J
	NxButton	m_checkProvider24J;
	NxButton	m_checkRefPhyNPI;
	NxButton	m_checkProviderNPI;
	// (j.jones 2009-08-04 12:34) - PLID 14573 - removed THIN ID
	//NxButton	m_btnSecTHIN;
	NxButton	m_btnSecPolicy;
	NxButton	m_btnSecPhone;
	NxButton	m_btnSecPayer;
	NxButton	m_btnSecNSF;
	NxButton	m_btnSecName;
	NxButton	m_btnSecInsID;
	NxButton	m_btnSecContactName;
	NxButton	m_btnSecDOB;
	NxButton	m_btnSecAddr;
	NxButton	m_btnRefPhy;
	NxButton	m_btnProvTaxonomy;
	// (j.jones 2011-03-15 10:52) - PLID 42788 - renamed this checkbox
	NxButton	m_btnProvANSIREFIDs;
	NxButton	m_btnProv;
	// (j.jones 2009-08-04 12:34) - PLID 14573 - removed THIN ID
	//NxButton	m_btnPriTHIN;
	NxButton	m_btnPriPolicy;
	NxButton	m_btnPriPayerID;
	NxButton	m_btnPriNSF;
	NxButton	m_btnPriName;
	NxButton	m_btnPriInsID;
	NxButton	m_btnPriDOB;
	NxButton	m_btnPriAddr;
	NxButton	m_btnPOSNPI;
	NxButton	m_btnPOSCode;
	NxButton	m_btnPtDOB;
	NxButton	m_btnPtAddress;
	NxButton	m_btnInvalidModifiers;
	NxButton	m_btnHospTo;
	NxButton	m_btnHospFrom;
	NxButton	m_btnDiagPtrs;
	NxButton	m_btnDiag1;
	NxButton	m_btnNPIChecksum;
	// (j.jones 2009-08-10 10:55) - PLID 32886 - added gender validations
	NxButton	m_btnPriGender;
	NxButton	m_btnSecGender;
	NxButton	m_btnPatGender;
	// (j.jones 2010-01-07 09:25) - PLID 36786 - added ability to disable the Self name checking
	NxButton	m_btnPatInsNamesMatchWhenSelf;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	// (j.jones 2010-07-23 11:44) - PLID 39797 - added assignment of benefits validation
	NxButton	m_btnAssignmentOfBenefits;
	// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
	NxButton	m_btnExportedTwiceInSameDay;
	// (j.jones 2012-05-01 09:59) - PLID 48530 - added prior auth. number validation
	NxButton	m_btnPriorAuthNum;
	// (j.jones 2012-07-24 17:54) - PLID 51764 - added office visit global period validation
	NxButton	m_btnOfficeVisitGlobalPeriods;
	// (a.wilson 2013-03-26 16:29) - PLID 51773 - added accident type for current accident date validation.
	NxButton	m_btnAccTypeForAccDate;
	// (d.singleton 2014-03-06 15:20) - PLID 61235 - if ins company requires icd9 codes and none are present on bill need to have validation error in ebilling
	NxButton	m_btnRequiresICD9;
	// (d.singleton 2014-03-06 15:21) - PLID 61236 - if ins company requires icd10 codes and none are present on bill need to have validation error in ebilling
	NxButton	m_btnRequiresICD10;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEbillingValidationConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEbillingValidationConfigDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EBILLINGVALIDATIONCONFIGDLG_H__3BB0EE12_074A_47A7_A9E8_E35D9E2DCDFC__INCLUDED_)
