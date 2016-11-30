#if !defined(AFX_DEPARTMENTSASSIGNUSERSDLG_H__C540DD90_C856_4132_8ADF_7F36588DE6EC__INCLUDED_)
#define AFX_DEPARTMENTSASSIGNUSERSDLG_H__C540DD90_C856_4132_8ADF_7F36588DE6EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DepartmentsAssignUsersDlg.h : header file
//


#include "ConfigureDepartmentsDlg.h"


/////////////////////////////////////////////////////////////////////////////
// CDepartmentsAssignUsersDlg dialog
//
// (z.manning, 02/28/2008) - PLID 29152 - Created

class CAttendanceDlg;

class CDepartmentsAssignUsersDlg : public CNxDialog
{
// Construction
public:
	CDepartmentsAssignUsersDlg(CArray<AttendanceDepartment,AttendanceDepartment&> &aryDepartments, AttendanceInfo *pInfo, CConfigureDepartmentsDlg* pdlgDepartment);   // standard constructor

	void SetDefaultDepartmentID(long nDepartmentID);

// Dialog Data
	//{{AFX_DATA(CDepartmentsAssignUsersDlg)
	enum { IDD = IDD_ASSIGN_USER_DEPARTMENTS };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnSelectOne;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnUnselectAll;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDepartmentsAssignUsersDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//CConfigureDepartmentsDlg *m_pdlgDepartment;
	AttendanceInfo *m_pAttendanceInfo;

	CArray<AttendanceDepartment,AttendanceDepartment&> m_aryDepartments;

	NXDATALIST2Lib::_DNxDataListPtr m_pdlDepartments;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlSelectedUsers;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlUnselectedUsers;

	long m_nDefaultDepartmentID;

	long m_nLastSelectedDepartmentID;

	BOOL m_bChanged;

	void Load();
	void Save(long nDepartmentID);

	void MoveRowToSelectedList(LPDISPATCH lpRow);
	void MoveRowToUnselectedList(LPDISPATCH lpRow);

	// Generated message map functions
	//{{AFX_MSG(CDepartmentsAssignUsersDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectAllUsers();
	afx_msg void OnSelectOneUser();
	afx_msg void OnUnselectAllUsers();
	afx_msg void OnUnselectOneUser();
	afx_msg void OnDblClickCellUsersInDepartment(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellUsersNotInDepartment(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnSelChosenDepartmentFilter(LPDISPATCH lpRow);
	virtual void OnOK();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEPARTMENTSASSIGNUSERSDLG_H__C540DD90_C856_4132_8ADF_7F36588DE6EC__INCLUDED_)
