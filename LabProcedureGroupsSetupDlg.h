// (r.gonet 03/29/2012) - PLID 49299 - Created.

#pragma once
#include "AdministratorRc.h"

// CLabProcedureGroupsSetupDlg dialog

class CLabProcedureGroupsSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLabProcedureGroupsSetupDlg)

public:
	CLabProcedureGroupsSetupDlg(CWnd* pParent = NULL);
	virtual ~CLabProcedureGroupsSetupDlg();


	// Dialog Data
	//{{AFX_DATA(CLabProcedureGroupsSetupDlg)
	enum { IDD = IDD_LAB_PROCEDURE_GROUPS_SETUP_DLG };

private:
	// Enums
	enum ELabProcedureGroupCBColumns {
		lpgcID,
		lpgcName,
	};

	enum ELabProcedureGroupUnselectedColumns {
		lpgucID,
		lpgucName,
	};

	enum ELabProcedureGroupSelectedColumns {
		lpgscID,
		lpgscName,
	};

	// Controls
	NXDATALIST2Lib::_DNxDataListPtr m_pGroupsComboBox;
	CNxIconButton m_nxbAddGroup;
	CNxIconButton m_nxbRemoveGroup;
	CNxIconButton m_nxbRenameGroup;
	
	NXDATALIST2Lib::_DNxDataListPtr m_pUnselectedLabProceduresList;
	CNxIconButton m_nxbSelectOne;
	CNxIconButton m_nxbSelectAll;
	CNxIconButton m_nxbUnselectAll;
	CNxIconButton m_nxbUnselectOne;
	NXDATALIST2Lib::_DNxDataListPtr m_pSelectedLabProceduresList;

	CNxIconButton m_nxbFormNumberSetup;

	CNxIconButton m_nxbOK;

	// Data
	// (r.gonet 03/29/2012) - PLID 49299 - Stores the ID of the currently selected Lab Procedure Group.
	//  Is kept up to date by being set whenever a group row is set.
	long m_nCurrentGroupID;

	// Methods
	void ReloadGroupsComboBox();
	void ReloadUnselectedList();
	void ReloadSelectedList();
	void EnsureControls();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// Generated message map functions
	//{{AFX_MSG(CLabProcedureGroupsSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedLpgAddGroupBtn();
	afx_msg void OnBnClickedLpgRemoveGroupBtn();
	afx_msg void OnBnClickedLpgRenameGroupBtn();
	afx_msg void OnBnClickedLpgSelectOneBtn();
	afx_msg void OnBnClickedLpgSelectAllBtn();
	afx_msg void OnBnClickedLpgUnselectAllBtn();
	afx_msg void OnBnClickedLpgUnselectOneBtn();
	afx_msg void OnBnClickedLpgFormNumberSettingsBtn();
	afx_msg void OnBnClickedOk();
	//}}AFX_MSG
	DECLARE_EVENTSINK_MAP()
	void SelChosenLpgGroupsList(LPDISPATCH lpRow);
	void SelChangingLpgGroupsList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangedLpgUnselectedProceduresList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void SelChangedLpgSelectedProceduresList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void CurSelWasSetLpgGroupsList();
};
