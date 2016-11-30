#if !defined(AFX_USERGROUPSECURITYDLG_H__5DB3C4BA_AA54_408D_B9DA_8944DD455130__INCLUDED_)
#define AFX_USERGROUPSECURITYDLG_H__5DB3C4BA_AA54_408D_B9DA_8944DD455130__INCLUDED_

#include "contactsrc.h"
#include "nxtree.h"
#include "nxsecurity.h"
#include <afxtempl.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UserGroupSecurityDlg.h : header file
//

#define COL_PERMNAME		0
#define COL_USERSPECIFIC	1
#define COL_PERM			2
#define COL_USERSPECIFIC_WITHPASS	3
#define COL_PERM_WITHPASS			4

/////////////////////////////////////////////////////////////////////////////
// CUserGroupSecurityDlg dialog

class CUserGroupSecurityDlg : public CNxDialog
{
// Construction
public:
	int Open(long nPersonID, BOOL bIsGroup);

	CUserGroupSecurityDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUserGroupSecurityDlg)
	enum { IDD = IDD_USERGROUP_SECURITY };
	NxButton	m_btnPermCoupling;
	CNxTree	m_treeSecurity;
	CNxEdit	m_nxeditEditSecurityobjectDescription;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnImportFrom;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUserGroupSecurityDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_dlPerms;

	// The person or group we are looking at permissions for
	long m_lPersonID;

	// True if this is a group
	BOOL m_bIsGroup;

	// m_lCurPerms is used to compare with when the user changed which security
	// object to see permissions for.
	DWORD m_dwCurPerms;

	// The map that retains all of our permission changes
	CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> m_mapPermChanges;

	// The current security object we are looking at
	EBuiltInObjectIDs m_eCurSecurityObject;

	// Utility function convenient for adding an available permssion row to the list
	BOOL AddPermissionRowToList(IN LPCTSTR strPermissionName, IN const ESecurityPermissionType esptPermType, IN const ESecurityPermissionType esptPermTypeWithPass, IN const CBuiltInObject *pbioSecurityObject);

	void FillSecurityTree();
	void FillPermissionList();
	void SetSecurityObjectDescription();
	HTREEITEM FindSecurityObject(HTREEITEM hParent, EBuiltInObjectIDs obj);
	void ParsePermissionsToList(unsigned long dwPerms);

	BOOL LoadPermissions();
	BOOL SavePermissions(BOOL bWriteToData, BOOL bPrompt = TRUE);

	BOOL m_bSetAdministrator;

	// (j.gruber 2010-03-29 16:04) - PLID 37946
	BOOL LoadSummaryTemplates();
	BOOL LoadSummaryUsers();
	void LoadSummaryList();
	// (j.gruber 2010-03-30 10:51) - PLID 37947
	CString GetPermissionString(const CBuiltInObject *pObj, long nPermissions);
	BOOL CheckPermissionSummary();
	// (j.gruber 2010-03-31 13:27) - PLID 23982	
	CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> m_mapTemplatePermissions;	
	// (j.gruber 2010-04-01 10:05) - PLID 38015
	void SetRowEnabledStatus(NXDATALISTLib::IRowSettingsPtr pRow, IN const ESecurityPermissionType esptPermType, unsigned long dwPerms);

	// (j.gruber 2010-04-14 12:09) - PLID 37947
	void RunSummaryReport();

	// (z.manning 2010-05-12 09:45) - PLID 37400 - Function to determine if certain permissions should be
	// excluded from having the w/ pass option
	BOOL ExcludeWithPassPermission(IN const EBuiltInObjectIDs eObjectID, IN const ESecurityPermissionType ePermType);

	NXDATALIST2Lib::_DNxDataListPtr m_pSummaryList;

	CNxIconButton m_btnClearPerms;
	CNxIconButton m_btnConfigureGroups;

	unsigned long GetPermissionsFromList();

	// (z.manning, 05/16/2008) - PLID 30050 - Added OnCtlColor
	// Generated message map functions
	//{{AFX_MSG(CUserGroupSecurityDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangedTreeSecurity(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditingFinishingListPermissions(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedListPermissions(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	virtual void OnOK();
	afx_msg void OnBtnHelp();
	afx_msg void OnBtnCopyFrom();
	afx_msg void OnPermCouplingCheck();
	afx_msg void OnRButtonDownListPermissions(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedUserSecConfigureGroups();	
	void LeftClickSummaryList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags); // (j.gruber 2010-04-14 12:10) - PLID 37947
	void EditingStartingListPermissions(long nRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);	// (j.gruber 2010-04-14 12:10) - PLID 38015
	afx_msg void OnBnClickedPermsClear(); // (j.gruber 2010-04-14 12:10) - PLID 23982
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USERGROUPSECURITYDLG_H__5DB3C4BA_AA54_408D_B9DA_8944DD455130__INCLUDED_)
