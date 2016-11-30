#if !defined(AFX_PATIENTGROUPSDLG_H__40C2C17D_67B4_402D_B761_E26F6375F3C9__INCLUDED_)
#define AFX_PATIENTGROUPSDLG_H__40C2C17D_67B4_402D_B761_E26F6375F3C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PatientGroupsDlg.h : header file
//
#include "Client.h"
/////////////////////////////////////////////////////////////////////////////
// CPatientGroupsDlg dialog

class CPatientGroupsDlg : public CNxDialog
{
// Construction
public:
	BOOL m_bAutoWriteToData;
	void UpdateCheckedArray();
	CPatientGroupsDlg(CWnd* pParent);   // standard constructor
	CTableChecker m_groupChecker;
	BOOL StoreData_Flat();
	//This should be given before you run the dialog if you're auto-writing; otherwise, it doesn't need to be set until
	//just before you call StoreData_Flat().
	long m_nPatID;

	virtual void Refresh();

// Dialog Data
	//{{AFX_DATA(CPatientGroupsDlg)
	enum { IDD = IDD_PATIENT_GROUPS_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnConfigGroups;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPatientGroupsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool IsGroupNameValid(CString strGroupNameToValidate, CString strOldGroupName = "");
	NXDATALISTLib::_DNxDataListPtr m_groupList;

	//Strings to hold data until we're ready to write it.
	CArray<int, int> m_arCheckedGroupIDs;
	
	// Generated message map functions
	//{{AFX_MSG(CPatientGroupsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishedGroupList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnConfigGroups();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATIENTGROUPSDLG_H__40C2C17D_67B4_402D_B761_E26F6375F3C9__INCLUDED_)
