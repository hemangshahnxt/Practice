#if !defined(AFX_CONFIGURENEXEMRGROUPSDLG_H__0D85FE62_EA0B_43D4_B96F_3DBAD5BC00EA__INCLUDED_)
#define AFX_CONFIGURENEXEMRGROUPSDLG_H__0D85FE62_EA0B_43D4_B96F_3DBAD5BC00EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigureNexEMRGroupsDlg.h : header file
//

enum EMNGroupFields {
	egfID = 0,
	egfName,
	egfSortOrder,
	egfExpand, // (z.manning 2009-08-11 16:18) - PLID 32989
};

enum EMNGroupTemplateFields {
	egtfID = 0,
	egtfName,
	egtfCollection,
	egtfSortOrder,
};

enum PreferredProcFields {
	eppfID = 0,
	eppfCheckbox,
	eppfName,
};

enum TypeOfEMNGroupChange {
	tegcCreate = 0,
	tegcModify,
	tegcDelete,
};

struct EMNGroupChangeInfo {
	long nGroupID;
	CString strGroupName;
	CString strOriginalName;
	long nSortOrder;
	long nOriginalSortOrder;
	TypeOfEMNGroupChange tegcTypeOfChange;
};

enum TypeOfEMNGroupLinkChange {
	teglcCreateLink = 0,
	teglcModify,
	teglcDeleteLink,
};

struct EMNGroupTemplateChangeInfo {
	long nGroupID;
	long nTemplateID;
	long nSortOrder;
	long nOriginalSortOrder;
	TypeOfEMNGroupLinkChange teglcTypeOfChange;
};

struct PreferredProcChangeInfo {
	long nProcedureID;
	BOOL bChecked;
};

/////////////////////////////////////////////////////////////////////////////
// CConfigureNexEMRGroupsDlg dialog

class CConfigureNexEMRGroupsDlg : public CNxDialog
{
// Construction
public:
	CConfigureNexEMRGroupsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigureNexEMRGroupsDlg)
	enum { IDD = IDD_CONFIGURE_NEXEMR_GROUPS_DLG };
	NxButton	m_checkShowPreferredProc;
	CNxIconButton	m_btnGroupTemplatePriorityUp;
	CNxIconButton	m_btnGroupTemplatePriorityDown;
	CNxIconButton	m_btnMovePriorityUp;
	CNxIconButton	m_btnMovePriorityDown;
	CNxIconButton	m_btnDeleteEMNGroup;
	CNxIconButton	m_btnAddEMNGroup;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigureNexEMRGroupsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pPreferredList;
	// (a.wetta 2007-06-06 10:56) - PLID 26234 - Changed datalist to allow customization of groups and their templates
	NXDATALIST2Lib::_DNxDataListPtr m_pEMNGroupList;
	NXDATALIST2Lib::_DNxDataListPtr m_pEMNGroupTemplateCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pEMNGroupTemplateList;

	void SelChangedEmnGroupsList(NXDATALIST2Lib::IRowSettingsPtr pOldSel, NXDATALIST2Lib::IRowSettingsPtr pNewSel);
	void Save();
	void SwapEMNGroupSortOrders(NXDATALIST2Lib::IRowSettingsPtr pRow1, NXDATALIST2Lib::IRowSettingsPtr pRow2);
	void SwapEMNGroupTemplateSortOrders(NXDATALIST2Lib::IRowSettingsPtr pRow1, NXDATALIST2Lib::IRowSettingsPtr pRow2);
	void UpdateSortOrderButtons();
	void UpdateTemplateSortOrderButtons();
	void RecordEMNGroupChange(EMNGroupFields egfChangeField, NXDATALIST2Lib::IRowSettingsPtr pChangeRow, _variant_t varOldValue, _variant_t varNewValue);
	void RecordEMNGroupTemplateChange(EMNGroupTemplateFields egtfChangeField, NXDATALIST2Lib::IRowSettingsPtr pChangeRow, _variant_t varOldValue, _variant_t varNewValue);
	void VerifyTemplateSortOrders();
	void VerifyGroupSortOrders();

	long m_nNewID;

	// (a.wetta 2007-06-06 13:45) - PLID 26234 - Keep track of the changes
	CMap<long, long, EMNGroupChangeInfo, EMNGroupChangeInfo> m_mapGroupChanges;
	CMap<CString, LPCTSTR, EMNGroupTemplateChangeInfo, EMNGroupTemplateChangeInfo> m_mapGroupTemplateChanges;
	CMap<long, long, PreferredProcChangeInfo, PreferredProcChangeInfo> m_mapPreferredProcChanges;

	//Functions to setup saving
	//CString GeneratePreferredSaveQuery();
	//CString GenerateSymptomSaveQuery();
	//CString GenerateSubjectiveSaveQuery();

	// Generated message map functions
	//{{AFX_MSG(CConfigureNexEMRGroupsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChangedEmnGroupsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnAddEmnGroup();
	afx_msg void OnDeleteEmnGroup();
	afx_msg void OnEditingFinishingEmnGroupsList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedEmnGroupsList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnMoveEmnGroupPriorityDownBtn();
	afx_msg void OnMoveEmnGroupPriorityUpBtn();
	afx_msg void OnSelChosenEmnGroupTemplatesCombo(LPDISPATCH lpRow);
	afx_msg void OnRButtonUpEmnGroupTemplateList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRemoveGroupTemplate();
	afx_msg void OnRequeryFinishedEmnGroupTempltCombo(short nFlags);
	afx_msg void OnMoveGroupTemplatePriorityDownBtn();
	afx_msg void OnMoveGroupTemplatePriorityUpBtn();
	afx_msg void OnSelChangedEmnGroupTempltList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnEditingFinishedConfigurePreferredProcList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnShowPreferredProc();
	afx_msg void OnRequeryFinishedEmnGroupList(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGURENEXEMRGROUPSDLG_H__0D85FE62_EA0B_43D4_B96F_3DBAD5BC00EA__INCLUDED_)
