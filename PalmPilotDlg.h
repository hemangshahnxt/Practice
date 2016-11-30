//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_PALMPILOTDLG_H__81391D94_1279_11D2_808D_00104B2FE914__INCLUDED_)
#define AFX_PALMPILOTDLG_H__81391D94_1279_11D2_808D_00104B2FE914__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PalmPilotDlg.h : header file
//

#include "PalmDialogDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CPalmPilotDlg dialog

class CPalmPilotDlg : public CNxDialog
{
// Construction
public:
	void PopulateSelected();
	void PopulateAvail();
	void UpdateModifiedAppts();
	void UpdateModifiedContacts();
	int DoModal();
	CPalmPilotDlg(CWnd* pParent);   // standard constructor
	CString m_TextParam;

	CPalmDialogDlg palm_dlg;
	CMapStringToString m_mapFields;

	// (z.manning, 04/30/2008) - PLID 29845 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CPalmPilotDlg)
	enum { IDD = IDD_PALM_PREFERENCES };
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnRemoveAll;
	CNxIconButton	m_btnRemoveOne;
	CNxIconButton	m_btnSelectOne;
	CListCtrl	m_SelectedList;
	CListCtrl	m_AvailList;
	CTabCtrl	m_Tab;
	CNxStatic	m_nxstaticAvailableFields;
	CNxStatic	m_nxstaticDisplayedFields;
	CNxStatic	m_nxstaticResources;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnRename;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPalmPilotDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_dlPalmUsers;
	NXDATALISTLib::_DNxDataListPtr m_dlResources;

	// Current palm user ID
	DWORD m_dwPalmUserID;
	BOOL m_bModified;

	// Loading
	void FillResourceCheckboxes();
	void FillScript();

	// Saving
	void SavePalmScript();
	void SaveResourceCheckboxes();

	// Generated message map functions
	//{{AFX_MSG(CPalmPilotDlg)
	afx_msg void OnAllToAvail();
	afx_msg void OnAllToSelected();
	afx_msg void OnToAvail();
	afx_msg void OnToSelected();
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnClickAdvPalm();
	afx_msg void OnClickBtnAdd();
	afx_msg void OnClickBtnRemove();
	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPalmUserChanged(long nNewSel);
	afx_msg void OnPalmprefRename();
	afx_msg void OnEditingFinishedPalmResourceList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnDblclkAvailableList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkSelectedList(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PALMPILOTDLG_H__81391D94_1279_11D2_808D_00104B2FE914__INCLUDED_)
