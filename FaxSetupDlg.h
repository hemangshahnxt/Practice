#if !defined(AFX_FAXSETUPDLG_H__5B5E9ECB_7D62_4B4E_B273_C22C781F5537__INCLUDED_)
#define AFX_FAXSETUPDLG_H__5B5E9ECB_7D62_4B4E_B273_C22C781F5537__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FaxSetupDlg.h : header file
//

//DRT 6/26/2008 - PLID 30524 - Created.

#include "WebFaxUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CFaxSetupDlg dialog

class CFaxSetupDlg : public CNxDialog
{
// Construction
public:
	CFaxSetupDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFaxSetupDlg)
	enum { IDD = IDD_FAX_SETUP_DLG };
	CNxEdit	m_editUser;
	CNxEdit	m_editPassword;
	CNxEdit	m_editFromName;
	CNxEdit	m_editName;
	NxButton m_checkERX;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnRemove;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFaxSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//Datalist interface members
	NXDATALIST2Lib::_DNxDataListPtr m_pServiceList;
	NXDATALIST2Lib::_DNxDataListPtr m_pResolutionList;
	NXDATALIST2Lib::_DNxDataListPtr m_pConfigList;

	//This is the currently selected ID.  Required to maintain because OnSelChosen doesn't give us
	//	the previous selection.
	long m_nCurrentLoadedID;

	//Helper functions
	void EnsureControls();
	void SyncInterfaceToDatalist();
	void SyncDatalistToInterface();
	bool SaveData();
	long GetNewID();

	//(r.farnworth 2013-04-23) PLID 54667 - Added to remember which row we have checked
	NXDATALIST2Lib::IRowSettingsPtr checkRow;

	// Generated message map functions
	//{{AFX_MSG(CFaxSetupDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangingFaxServiceList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenFaxServiceList(LPDISPATCH lpRow);
	afx_msg void OnFaxConfigAdd();
	afx_msg void OnFaxConfigRemove();
	afx_msg void OnSelChosenFaxConfigList(LPDISPATCH lpRow);
	afx_msg void OnChangeFaxConfigName();
	afx_msg void OnSelChangingFaxConfigList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	long RememberFaxConfig();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedErxSelect();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FAXSETUPDLG_H__5B5E9ECB_7D62_4B4E_B273_C22C781F5537__INCLUDED_)
