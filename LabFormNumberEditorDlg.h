// LabFormNumberEditorDlg.h: interface for the CLabFormNumberEditorDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LABFORMNUMBEREDITORDLG_H__9BA372F9_51F6_4870_A62B_139737227363__INCLUDED_)
#define AFX_LABFORMNUMBEREDITORDLG_H__9BA372F9_51F6_4870_A62B_139737227363__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define LAB_FORM_NUMBER_FORMAT_DEFAULT				"%y-%n#4"

// (z.manning, 07/25/2006) - PLID 21576 - Given a format and a length for the increment counter, returns
// a new lab form number.
CString GetNewLabFormNumber(CString strFormat, long nLabProcedureGroupID);
CString GetNewLabFormNumber(CString strFormat, OUT long &nLabFormNumberCounter, long nLabProcedureGroupID);
CString GetLabFormNumberFormat(long nLabProcedureGroupID);

//TES 3/30/2012 - PLID 48205 - Added bNumberIncludesYear; if it is false, then the counter will not be reset even if it's a new year
long GetNextLabFormNumber(long nLabProcedureGroupID, bool bNumberIncludesYear);
// (z.manning, 07/25/2006) - PLID 21576 - Increments the value that is pulled when generating a new lab form number.
void IncrementLabFormNumberCounter(int nIncrementBy, long nLabProcedureGroupID);
void SetLabFormNumberCounterIfLess(long nCount, long nLabProcedureGroupID);

class CLabFormNumberEditorDlg : public CNxDialog  
{
public:
	CLabFormNumberEditorDlg(CDialog* pParent = NULL);   // standard constructor
	// (r.gonet 03/29/2012) - PLID 45856 - Explicit because long can be cast as a pointer.
	explicit CLabFormNumberEditorDlg(long nLabProcedureGroupID, CDialog* pParent = NULL);

	void OnOK();

	void UpdatePreview();

	BOOL m_bIncrementValueChanged; // (z.manning 2010-02-02 09:47) - PLID 34808

// Dialog Data
	//{{AFX_DATA(CLabsSetupDlg)
	enum { IDD = IDD_LAB_FORM_NUMBER_EDITOR };
	// (r.gonet 03/29/2012) - PLID 45856
	NxButton m_checkOverrideDefaultFormat;
	CNxEdit	m_nxeditLabNumberFormat;
	CNxEdit	m_nxeditIncrementalPortionDigits;
	CNxEdit m_nxeditIncrementalPortionValue; // (z.manning 2010-02-01 15:16) - PLID 34808
	CNxStatic	m_nxstaticPreviewFormNumberLabel;
	CNxStatic	m_nxstaticPreviewFormNumber;
	// (r.gonet 03/29/2012) - PLID 45856 - 
	NxButton m_checkUseSeparateCount;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLabsSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int CalculateProjectedFormNumberLength();
	// (r.gonet 03/29/2012) - PLID 45856
	void EnsureControls();
	// (r.gonet 03/29/2012) - PLID 45856
	void InitializeFormNumberEditor();

	int m_nOriginalIncrementValue; // (z.manning 2010-02-02 09:45) - PLID 34808
	int m_nLabProcedureGroupID; // (r.gonet 03/29/2012) - PLID 45856 - Tracks the Lab Procedure Group we are editing the form number format for.
	// (r.gonet 03/29/2012) - PLID 45856 - The original values of the override flags in LabProcedureGroupsT
	BOOL m_bOldOverrideDefaultFormat;
	BOOL m_bOldUseSeparateSequence;

	// Generated message map functions
	//{{AFX_MSG(CLabsSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedLfneOverrideDefaultFormat();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LABFORMNUMBEREDITORDLG_H__9BA372F9_51F6_4870_A62B_139737227363__INCLUDED_)
