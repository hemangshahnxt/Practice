#if !defined(AFX_PATIENTINTERESTCALCULATIONDLG_H__6085CE77_14D0_477D_B0BF_9B612A470BD4__INCLUDED_)
#define AFX_PATIENTINTERESTCALCULATIONDLG_H__6085CE77_14D0_477D_B0BF_9B612A470BD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PatientInterestCalculationDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPatientInterestCalculationDlg dialog

class CPatientInterestCalculationDlg : public CNxDialog
{
// Construction
public:
	CPatientInterestCalculationDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2009-06-11 16:05) - PLID 34577 - added an IN clause of patient IDs to be
	// displayed on this dialog, rather than showing all patients
	CString m_strPatientIDs;

	// (j.jones 2009-06-11 16:05) - PLID 34577 - added an array of patient IDs to skip
	CArray<long, long> m_aryPatientIDsToSkip;

	// (z.manning, 05/01/2008) - PLID 29864 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CPatientInterestCalculationDlg)
	enum { IDD = IDD_PATIENT_INTEREST_CALCULATION_DLG };
	CNxIconButton	m_btnUnselectAll;
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnSelectOne;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	// (j.jones 2009-11-03 11:06) - PLID 29666 - added LW controls
	NxButton		m_radioAll;
	NxButton		m_radioFilter;
	NxButton		m_radioGroup;
	CNxIconButton	m_btnEditLW;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPatientInterestCalculationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_UnselectedList, m_SelectedList;

	// (j.jones 2009-11-03 11:06) - PLID 29666 - added LW controls
	NXDATALIST2Lib::_DNxDataListPtr m_FilterCombo, m_GroupCombo;

	// Generated message map functions
	//{{AFX_MSG(CPatientInterestCalculationDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBtnSelectOnePatient();
	afx_msg void OnBtnSelectAllPatients();
	afx_msg void OnBtnUnselectOnePatient();
	afx_msg void OnBtnUnselectAllPatients();
	afx_msg void OnDblClickCellUnselectedPatientList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedPatientList(long nRowIndex, short nColIndex);
	virtual void OnCancel();
	// (j.jones 2009-11-03 11:06) - PLID 29666 - added LW controls
	afx_msg void OnBtnEditLw();
	afx_msg void OnRadioLwAll();
	afx_msg void OnRadioLwFilter();
	afx_msg void OnRadioLwGroup();
	afx_msg void OnSelChosenLwFilterCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenLwGroupCombo(LPDISPATCH lpRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATIENTINTERESTCALCULATIONDLG_H__6085CE77_14D0_477D_B0BF_9B612A470BD4__INCLUDED_)
