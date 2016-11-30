#if !defined(AFX_DUPLICATE_H__AC01EB53_D658_11D2_9340_00104B318376__INCLUDED_)
#define AFX_DUPLICATE_H__AC01EB53_D658_11D2_9340_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Duplicate.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CDuplicate dialog

//(e.lally 2008-02-27) PLID 27379 - defines for more intuitive return IDs
#define ID_CHANGE_NEW_PATIENT_NAME		4
#define ID_CREATE_NEW_PATIENT			5
#define ID_CANCEL_AND_GOTO_SELECTED		6
#define ID_CANCEL_NEW_PATIENT			7
#define ID_MERGE_WITH_SELECTED			8


enum EStatusFilterTypes {
	esfNone = 0, 
	esfPatient = 1, 
	esfProspect = 2, 
	esfPatientProspect = 4,
	esfInquiry = 8
};

//(e.lally 2008-02-27) PLID 27379 - Changed to a CNxDialog
class CDuplicate : public CNxDialog
{
// Construction
public:
	CString m_name;
	long m_nSelPatientId;

	CDuplicate(CWnd* pParent, bool bForceSelectionOnIgnore = false);
	bool FindDuplicates(CString first, CString last, CString middle);
	// (z.manning 2009-08-24 12:47) - PLID 31135 - Added an overload that takes birth date
	bool FindDuplicates(CString first, CString last, CString middle, COleDateTime dtBirthDate);
	
	// (d.moore 2007-08-16) - PLID 25455 - I modified the dialog to allow filtering the
	//  search and contents by the PatientsT.CurrentStatus field.
	void SetStatusFilter(EStatusFilterTypes nFilterType);
	void SetUseMergeButton(BOOL bUseMergeBtn);
	void EnableGoToPatientBtn(BOOL bEnable);

	// (r.goldschmidt 2014-07-17 18:45) - PLID 62774 - In New Patient Dialog, check against possible duplicates once first name, last name, and date of birth are entered.
	void EnableSaveBtn(BOOL bEnable);

// Dialog Data
	//{{AFX_DATA(CDuplicate)
	enum { IDD = IDD_DUPLICATE_PATIENTS };
	CNxIconButton	m_btnGoToSelected;
	CNxIconButton	m_btnMergeWithSelected;
	CNxIconButton	m_btnChangeName;
	CNxIconButton	m_btnAddNewPatient;
	CNxIconButton	m_btnCancelNewPatient;
	CNxStatic	m_nxstaticNameLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDuplicate)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	long GetSelPatientId();
	void EnsureControls();

	bool m_bForceSelectionOnIgnore;
	NXDATALISTLib::_DNxDataListPtr	m_pDupList;
	
	// (d.moore 2007-08-16) - PLID 25455 - I modified the dialog to allow filtering the
	//  search and contents by the PatientsT.CurrentStatus field.
	long m_nStatusFilter;
	DWORD m_backgroundColor;
	CBrush m_brush;
	CString sql;
	BOOL m_bUseMergeBtn;
	BOOL m_bEnableGoToPatientBtn;
	BOOL m_bEnableSaveBtn; // (r.goldschmidt 2014-07-17 18:45) - PLID 62774
	
	//(e.lally 2008-02-27) PLID 27379 - Changed message map functions to more descriptive names

	// Generated message map functions
	//{{AFX_MSG(CDuplicate)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnSaveAsNewPatient();
	afx_msg void OnChangeNewPatientName();
	afx_msg void OnCancelNewPatientEntry();
	afx_msg void OnGoto();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnMergeWithSelectedPatient();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DUPLICATE_H__AC01EB53_D658_11D2_9340_00104B318376__INCLUDED_)
