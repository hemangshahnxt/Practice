#if !defined(AFX_PATIENTGROUPSCONFIGDLG_H__135B33EB_62CB_43B9_AC3B_4263977D2957__INCLUDED_)
#define AFX_PATIENTGROUPSCONFIGDLG_H__135B33EB_62CB_43B9_AC3B_4263977D2957__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PatientGroupsConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPatientGroupsConfigDlg dialog
//TES 1/11/2010 - PLID 36761 - Moved the IdName struct to GlobalUtils.h

class CPatientGroupsConfigDlg : public CNxDialog
{
// Construction
public:
	CPatientGroupsConfigDlg(CWnd* pParent);   // standard constructor
	CTableChecker m_groupChecker;
	BOOL StoreData_Flat();

// Dialog Data
	//{{AFX_DATA(CPatientGroupsDlg)
	enum { IDD = IDD_PATIENT_GROUPS_CONFIG_DLG };
	CNxIconButton	m_btnNew;
	CNxIconButton	m_btnRenameGroup;
	CNxIconButton	m_btnDeleteGroup;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPatientGroupsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool RenameElements(CArray<IdName, IdName> &ary, IN const IdName &NewName);
	bool IsGroupNameValid(CString strGroupNameToValidate, CString strOldGroupName = "");
	NXDATALISTLib::_DNxDataListPtr m_groupList;
	void RefreshButtons();

	//Strings to hold data until we're ready to write it.
	CArray<int, int> m_arCheckedGroupIDs;
	//(a.wilson 2011-9-22) PLID 28266 - changed from array of ints to an array of IdName structs.
	CArray<IdName, IdName> m_arDeletedGroups;
	//m_arAddedGroups and m_arRenamedGroups must always have unique nIDs
	CArray<IdName, IdName> m_arAddedGroups;
	CArray<IdName, IdName> m_arRenamedGroups;

	// Generated message map functions
	//{{AFX_MSG(CPatientGroupsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishedGroupList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnNew();
	virtual void OnCancel();
	afx_msg void OnDeleteGroup();
	afx_msg void OnRenameGroup();
	virtual void OnOK();
	afx_msg void OnEditingFinishingGroupList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnLeftClickGroupList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelChangedGroupList(long nNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATIENTGROUPSCONFIGDLG_H__135B33EB_62CB_43B9_AC3B_4263977D2957__INCLUDED_)
