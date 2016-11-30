#pragma once

// (j.jones 2010-11-03 09:56) - PLID 41288 - created

#include "FinancialRc.h"

// CAlbertaClaimValidationConfigDlg dialog

class CAlbertaClaimValidationConfigDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAlbertaClaimValidationConfigDlg)

public:
	CAlbertaClaimValidationConfigDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	enum { IDD = IDD_ALBERTA_VALIDATION_CONFIG_DLG };	
	NxButton	m_checkProviderBAID;
	NxButton	m_checkSubmitterPrefix;
	NxButton	m_checkProvNPI;
	NxButton	m_checkPOSNPI;
	NxButton	m_checkRefPhySelected;
	NxButton	m_checkRefPhyNPI;
	NxButton	m_checkProvTaxonomy;
	NxButton	m_checkPatHealthNum;
	NxButton	m_checkRegistrationNum;
	NxButton	m_checkPatName;
	NxButton	m_checkPatBirthdate;
	NxButton	m_checkPatGender;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
	NxButton	m_btnExportedTwiceInSameDay;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlbertaClaimValidationConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAlbertaClaimValidationConfigDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};