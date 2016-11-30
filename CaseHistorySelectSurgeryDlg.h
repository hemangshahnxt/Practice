#if !defined(AFX_CASEHISTORYSELECTSURGERYDLG_H__DEA72102_353D_472C_ADCA_CC6A7DA96FE7__INCLUDED_)
#define AFX_CASEHISTORYSELECTSURGERYDLG_H__DEA72102_353D_472C_ADCA_CC6A7DA96FE7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CaseHistorySelectSurgeryDlg.h : header file
//

#include "PatientsRc.h"

/////////////////////////////////////////////////////////////////////////////
// CCaseHistorySelectSurgeryDlg dialog

class CCaseHistorySelectSurgeryDlg : public CNxDialog
{
// Construction
public:
	CCaseHistorySelectSurgeryDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCaseHistorySelectSurgeryDlg)
	enum { IDD = IDD_CASE_HISTORY_SELECT_SURGERY_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	// (j.jones 2009-08-31 17:34) - PLID 17734 - added appt. purpose filter
	NxButton	m_checkFilterByApptPurposes;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCaseHistorySelectSurgeryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	long m_nPatientID; // (c.haag 2007-03-09 11:09) - PLID 25138 - We now have a patient ID
	// for calculating the default case history provider
	long m_nProviderID;

	// (j.jones 2009-08-31 17:35) - PLID 17734 - used for filtering by purposes
	long m_nAppointmentID;

	// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
	// (j.jones 2009-08-31 14:56) - PLID 35378 - changed to allow multi-selection
	CArray<long, long> m_arynPreferenceCardIDs;
	CString m_strNewCaseHistoryName;

	// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
	// (j.jones 2009-08-31 14:49) - PLID 35378 - changed to datalist 2s
	NXDATALIST2Lib::_DNxDataListPtr m_pPreferenceCardList, m_pProviderCombo;

// Implementation
public:
	// (c.haag 2007-03-09 11:08) - PLID 25138 - Designate the patient ID for calculating
	// the default case history provider
	void SetPatientID(long nPatientID);

protected:

	// (j.jones 2009-08-31 17:41) - PLID 17734 - added central refiltering function
	void RefilterPreferenceCards();

	// Generated message map functions
	//{{AFX_MSG(CCaseHistorySelectSurgeryDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	void OnSelChosenChssProviderCombo(LPDISPATCH lpRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2009-08-31 17:34) - PLID 17734 - added appt. purpose filter
	afx_msg void OnCheckFilterByApptPurposes();
	void OnRequeryFinishedPreferenceCardMultiselectList(short nFlags);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CASEHISTORYSELECTSURGERYDLG_H__DEA72102_353D_472C_ADCA_CC6A7DA96FE7__INCLUDED_)
