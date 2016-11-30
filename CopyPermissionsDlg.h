#if !defined(AFX_COPYPERMISSIONSDLG_H__9581BD4F_5FE6_4649_9894_47E0B7CA11C8__INCLUDED_)
#define AFX_COPYPERMISSIONSDLG_H__9581BD4F_5FE6_4649_9894_47E0B7CA11C8__INCLUDED_

#include <afxtempl.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CopyPermissionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCopyPermissionsDlg dialog

class CCopyPermissionsDlg : public CNxDialog
{
// Construction
public:
	long m_nUserGroup;

	//used externally to detect if we copied the Administrative status
	BOOL m_bAdministratorSet;

	/* Set this value to non-null if you want the individual permissions to
	be saved to this map rather than the data */
	CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD>* m_pmapPermChanges;

	/* Set this value to non-null if you want the users and template ID's
	to be saved instead of writing to the data */
	CDWordArray* m_padwUserGroups;

	CCopyPermissionsDlg(CWnd* pParent);   // standard constructor


	/* Import permissions from one user/template to another user/template through
	data writes. */
	static HRESULT CopyPermissions(long lSrcUserGroup, long lDstUserGroup, CString &strSql);

	/* Import permissions from one user/template to another user/template WITHOUT DATA
	WRITES; but rather, putting the results in a map */
	static HRESULT CopyPermissions(long lSrcUserGroup, long lDstUserGroup, CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD>* mapPermChanges);

	/* Import permissions from several user/teampltes to another user/template */
	static HRESULT CopyPermissions(CDWordArray* padwSrcUserGroup, long lDstUserGroup);


// Dialog Data
	//{{AFX_DATA(CCopyPermissionsDlg)
	enum { IDD = IDD_COPY_PERMISSIONS };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCopyPermissionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_dlItems;

	// Generated message map functions
	//{{AFX_MSG(CCopyPermissionsDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COPYPERMISSIONSDLG_H__9581BD4F_5FE6_4649_9894_47E0B7CA11C8__INCLUDED_)
