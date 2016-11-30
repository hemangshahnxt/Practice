#if !defined(AFX_CONFIGUREDEPARTMENTSDLG_H__1BEF6D66_566B_4519_81BC_19426B95F0D6__INCLUDED_)
#define AFX_CONFIGUREDEPARTMENTSDLG_H__1BEF6D66_566B_4519_81BC_19426B95F0D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigureDepartmentsDlg.h : header file
//

#include "AttendanceUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CConfigureDepartmentsDlg dialog
//
// (z.manning, 02/28/2008) - PLID 29152 - Created

class CAttendanceDlg;

class CConfigureDepartmentsDlg : public CNxDialog
{
	friend class CDepartmentsAssignUsersDlg;

// Construction
public:
	CConfigureDepartmentsDlg(AttendanceInfo *pInfo, CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigureDepartmentsDlg)
	enum { IDD = IDD_CONFIGURE_DEPARMENTS };
	CNxIconButton	m_btnAssignUsers;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigureDepartmentsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	AttendanceInfo *m_pAttendanceInfo;

	NXDATALIST2Lib::_DNxDataListPtr m_pdlDepartments;

	void AddDepartment();
	void DeleteDepartment();

	void UpdateManagerDisplay();

	// Generated message map functions
	//{{AFX_MSG(CConfigureDepartmentsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddDepartment();
	afx_msg void OnDeleteDepartment();
	afx_msg void OnEditingFinishingDepartmentList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedDepartmentList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnAssignUsersToDeparments();
	afx_msg void OnLeftClickDepartmentList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedDepartmentList(short nFlags);
	afx_msg void OnRButtonUpDepartmentList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGUREDEPARTMENTSDLG_H__1BEF6D66_566B_4519_81BC_19426B95F0D6__INCLUDED_)
