#pragma once

//TES 1/5/2010 - PLID 35774 - Created, inspired by CPatientGroupsConfigDlg.

//TES 1/5/2010 - PLID 35774 - Store information pending saving when the dialog is closed.
struct SecurityGroupInfo
{
	long nID;
	CString strName;
};

class CSecurityGroupsConfigDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSecurityGroupsConfigDlg)

public:
	CSecurityGroupsConfigDlg(CWnd* pParent);   // standard constructor
	virtual ~CSecurityGroupsConfigDlg();

// Dialog Data
	enum { IDD = IDD_SECURITY_GROUPS_CONFIG_DLG };

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pGroups;
	CNxIconButton m_nxbOK, m_nxbCancel, m_nxbAdd, m_nxbRename, m_nxbDelete;

	//TES 1/5/2010 - PLID 35774 - Groups that have been added since we opened the dialog (unique, negative IDs)
	CArray<SecurityGroupInfo,SecurityGroupInfo&> m_arAddedGroups;
	//TES 1/5/2010 - PLID 35774 - Groups that have been renamed since we opened the dialog.
	CArray<SecurityGroupInfo,SecurityGroupInfo&> m_arRenamedGroups;
	//TES 1/5/2010 - PLID 35774 - IDs of groups that have been deleted since we opened the dialog.
	CArray<long,long> m_arDeletedGroupIDs;

	//TES 1/5/2010 - PLID 35774 - Reflect whether a group is selected.
	void UpdateButtons();

	//TES 1/5/2010 - PLID 35774 - Determines whether the new name is valid, and not a duplicate.
	BOOL IsGroupNameValid(const CString &strNewName, OPTIONAL const CString &strOldName = "");
	//TES 1/5/2010 - PLID 35774 - Renames an element in the array, based on its ID
	BOOL RenameElements(CArray<SecurityGroupInfo, SecurityGroupInfo&> &ary, IN SecurityGroupInfo &sgiNewName);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnAddGroup();
	afx_msg void OnDeleteGroup();
	afx_msg void OnBnClickedRenameGroup();
	afx_msg void OnOK();
	DECLARE_EVENTSINK_MAP()
	void OnSelChangedSecurityGroupsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
};
