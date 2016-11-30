#if !defined(AFX_ELIGIBILITYSETUPDLG_H__5D680F7B_1612_471B_9DC9_502E7F06FEC2__INCLUDED_)
#define AFX_ELIGIBILITYSETUPDLG_H__5D680F7B_1612_471B_9DC9_502E7F06FEC2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EligibilitySetupDlg.h : header file
//

// (j.jones 2008-06-23 09:00) - PLID 30434 - created

#include "AdministratorRc.h"

//do not change these, they are stored in data, and referenced in CEEligibility
enum Combo2100C_Values {

	value2100C_None = 0,
	value2100C_SSN = 1,
	value2100C_PolicyGroupNum = 2,
};

// (j.jones 2010-05-25 15:43) - PLID 38868 - added 2100D values, also stored in data
enum Combo2100D_Values {

	value2100D_None = 0,
	value2100D_SSN = 1,
	value2100D_PolicyGroupNum = 2,
};

/////////////////////////////////////////////////////////////////////////////
// CEligibilitySetupDlg dialog

class CEligibilitySetupDlg : public CNxDialog
{
// Construction
public:
	CEligibilitySetupDlg(CWnd* pParent);   // standard constructor

	long m_nGroupID;
	CString m_strGroupName;

// Dialog Data
	//{{AFX_DATA(CEligibilitySetupDlg)
	enum { IDD = IDD_ELIGIBILITY_SETUP_DLG };
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxEdit	m_nxedit2100B_Value_2;
	CNxEdit	m_nxedit2100B_Value_1;
	CNxEdit	m_nxedit2100B_Qual_2;
	CNxEdit	m_nxedit2100B_Qual_1;
	NxButton	m_checkUse2100B_2;
	NxButton	m_checkUse2100B_1;
	CNxStatic	m_nxstaticGroupLabel;
	// (j.jones 2010-03-01 17:28) - PLID 37585 - added override for 2100B NM109
	CNxEdit	m_nxedit2100B_NM109_Qual;
	CNxEdit	m_nxedit2100B_NM109_Value;
	NxButton	m_checkUse2100B_NM109;
	// (j.jones 2011-06-15 14:33) - PLID 42181 - added NPI toggle
	NxButton	m_radioProvNPI;
	NxButton	m_radioLocNPI;
	// (j.jones 2013-07-08 15:08) - PLID 57469 - added UseTitleInLast
	NxButton	m_checkUseTitleInLast;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEligibilitySetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr	m_ProviderCombo,
								m_LocationCombo,
								m_2100C_Combo;

	// (j.jones 2010-05-25 15:43) - PLID 38868 - added m_2100D_Combo
	NXDATALIST2Lib::_DNxDataListPtr	m_2100D_Combo;

	void Load();
	BOOL Save();

	BOOL m_bIsLoading;
	BOOL m_bHasChanged;

	long m_nProviderID;
	long m_nLocationID;

	// Generated message map functions
	//{{AFX_MSG(CEligibilitySetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnCheckUse2100b1();
	afx_msg void OnCheckUse2100b2();
	afx_msg void OnSelChosenEligProviderCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenEligLocationCombo(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedEligProviderCombo(short nFlags);
	afx_msg void OnRequeryFinishedEligLocationCombo(short nFlags);
	// (j.jones 2010-03-01 17:28) - PLID 37585 - added override for 2100B NM109
	afx_msg void OnCheckUse2100bNm109();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ELIGIBILITYSETUPDLG_H__5D680F7B_1612_471B_9DC9_502E7F06FEC2__INCLUDED_)
