#if !defined(AFX_HCFASETUPDONOTFILL_H__E1284572_23F0_4007_850D_200075BC8A70__INCLUDED_)
#define AFX_HCFASETUPDONOTFILL_H__E1284572_23F0_4007_850D_200075BC8A70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HCFASetupDoNotFill.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHCFASetupDoNotFill dialog

class CHCFASetupDoNotFill : public CNxDialog
{
// Construction
public:
	CHCFASetupDoNotFill(CWnd* pParent);   // standard constructor

	long m_nHCFASetupGroupID;

	// (z.manning, 05/01/2008) - PLID 29860 - Added NxIconButtons
	// (j.jones 2008-06-18 09:05) - PLID 30403 - added m_checkExcludeBox29
	// (j.jones 2009-01-06 08:49) - PLID 32614 - added m_checkHideBox17b
// Dialog Data
	//{{AFX_DATA(CHCFASetupDoNotFill)
	enum { IDD = IDD_HCFA_SETUP_DO_NOT_FILL };
	NxButton	m_checkHideBox17b;
	NxButton	m_checkExcludeBox29;
	NxButton	m_checkHideBox24JNPI;
	NxButton m_checkHideBox32NameAdd;
	NxButton m_checkHideBox32NPIID;
	NxButton	m_checkHide24ATo;
	NxButton	m_checkHideBox11c;
	NxButton	m_checkHideBox11b;
	NxButton	m_checkHideBox11a_Gender;
	NxButton	m_checkHideBox11a_Birthdate;
	NxButton	m_checkShowWhichCodesCommas;
	NxButton	m_checkSecondaryFillBox11;
	NxButton	m_checkShowAnesthMinutesOnly;
	NxButton	m_checkPrintPInBox1a;
	NxButton	m_checkHideBox11DAlways;
	NxButton	m_checkShowSecondary24JNumber;
	NxButton	m_checkShowSecondaryPINNumber;
	NxButton	m_checkHideBox30;
	NxButton	m_checkHideBox11;
	NxButton	m_checkHideBox9;
	NxButton	m_checkHideBox7;
	NxButton	m_checkHideBox4;
	NxButton	m_check11D;
	NxButton	m_31;
	NxButton	m_checkShowRespName;
	NxButton	m_checkShowAnesthTimes;
	NxButton	m_checkShowSecInsAdd;
	NxButton	m_checkShowApplies;
	NxButton	m_checkExcludeAdjustments;
	NxButton	m_checkExcludeAdjustmentsFromBalance;
	NxButton	m_checkLeftAlignInsAddress;
	NxButton	m_checkShowAddress;
	NxButton	m_checkShowDiagDesc;
	NxButton	m_checkShowDiagDecimals;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr m_Accepted12Combo;
	NXDATALISTLib::_DNxDataListPtr m_Accepted13Combo;
	// (j.jones 2007-05-11 14:11) - PLID 25932 - reworked Box 32 options
	NXDATALISTLib::_DNxDataListPtr m_HideBox32NameAddCombo;
	NXDATALISTLib::_DNxDataListPtr m_HideBox32NPIIDCombo;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHCFASetupDoNotFill)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.jones 2013-08-09 15:31) - PLID 57958 - Track whether any companies in this group
	// are on the new HCFA form, vs. on the old HCFA form. This setting should not be referenced
	// in claim generation, only in the setup.
	bool m_bAnyCompaniesOnNewHCFA;
	bool m_bAnyCompaniesOnOldHCFA;

	// (j.jones 2008-06-18 09:05) - PLID 30403 - added OnCheckExcludeAllBox29
	// Generated message map functions
	//{{AFX_MSG(CHCFASetupDoNotFill)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCheckShowInsAddress();
	afx_msg void OnCheckShowApplies();
	afx_msg void OnCheckShowSecondaryInsAdd();
	afx_msg void OnCheckExcludeAdjustments();
	afx_msg void OnCheckHideBox11dAlways();
	afx_msg void OnCheck11d();
	afx_msg void OnCheckShowAnesthesiaTimes();
	afx_msg void OnCheckHideBox32NameAdd();
	afx_msg void OnCheckHideBox32NpiId();
	afx_msg void OnCheckExcludeAllBox29();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HCFASETUPDONOTFILL_H__E1284572_23F0_4007_850D_200075BC8A70__INCLUDED_)
