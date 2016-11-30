#pragma once

// COHIPValidationConfigDlg dialog

// (j.jones 2008-12-03 11:15) - PLID 32258 - created

#include "FinancialRc.h"

class COHIPValidationConfigDlg : public CNxDialog
{

public:
	COHIPValidationConfigDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2008-12-10 11:22) - PLID 32312 - added m_checkPatHealthNumVerify
// Dialog Data
	enum { IDD = IDD_OHIP_VALIDATION_CONFIG_DLG };
	NxButton	m_checkPatHealthNumVerify;
	NxButton	m_checkMOHOfficeCode;
	NxButton	m_checkGroupNumber;
	NxButton	m_checkProvSpecialty;
	NxButton	m_checkProvNPI;
	NxButton	m_checkLocNPI;
	NxButton	m_checkPOSNPI;
	NxButton	m_checkRefPhySelected;
	NxButton	m_checkRefPhyNPI;
	NxButton	m_checkServiceCode;
	NxButton	m_checkPatHealthNum;
	NxButton	m_checkPatVersionCode;
	NxButton	m_checkRegistrationNum;
	NxButton	m_checkPatName;
	NxButton	m_checkPatBirthdate;
	NxButton	m_checkPatGender;
	NxButton	m_checkPatProvince;
	NxButton	m_checkDiagCode;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
	NxButton	m_btnExportedTwiceInSameDay;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COHIPValidationConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.jones 2008-12-10 11:22) - PLID 32312 - added OnCheckPatientHealthNumber
	// Generated message map functions
	//{{AFX_MSG(COHIPValidationConfigDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCheckPatientHealthNumber();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

