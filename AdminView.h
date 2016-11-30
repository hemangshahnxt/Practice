#if !defined(AFX_ADMINVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_)
#define AFX_ADMINVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// (j.jones 2013-05-07 12:19) - PLID 53969 - removed the .h files for the child tabs
class CPracticeInfo;
class CCPTCodes;
class CRetail;
class CHcfaSetup;
class CUB92Setup;
class CMultiFees;
class CMainSurgeries;
class CSchedulerSetupDlg;
class CProcedureDlg;
class CCustomRecordSetupDlg;
class CEMRSetupDlg;
class CPhaseDlg;
class CZipcodes;
class CAuditing;
class CLabsSetupDlg;
class CLinksSetupDlg;

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and ResolveDefaultTab to the modules code

/////////////////////////////////////////////////////////////////////////////
// CAdminView view

class CAdminView : public CNxTabView
{

public:

protected:
	CAdminView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CAdminView)

// Attributes
public:
	// (j.jones 2013-05-07 12:02) - PLID 53969 - changed the child tabs to be declared by reference,
	// which avoids requiring their .h files in this file
	CPracticeInfo		&m_LocationSheet;
	CCPTCodes			&m_BillingSheet;
	// (j.gruber 2007-03-19 11:19) - PLID 25165 - adding a discounts tab
	// (a.wetta 2007-03-29 10:38) - PLID 25407 - This tab is being turned into the Retail tab and discounts 
	// are on this tab
	// (a.wetta 2007-05-16 08:58) - PLID 25960 - This has been changed to the NexSpa tab
	CRetail			    &m_NexSpaSheet;
	CHcfaSetup			&m_HCFASheet;	
	CUB92Setup			&m_UB92Sheet;
	CMultiFees			&m_MultiFeeSheet;
	CMainSurgeries		&m_SurgerySheet;
	CSchedulerSetupDlg	&m_SchedulerSheet;
	CProcedureDlg		&m_ProcedureSheet;
	CCustomRecordSetupDlg &m_CustomRecordSheet;
	CEMRSetupDlg		&m_EMRSheet;
	CPhaseDlg			&m_PhaseSheet;
	CZipcodes			&m_ZipCodeSheet;	
	CAuditing			&m_AuditingSheet;
	CLabsSetupDlg		&m_LabsSheet;
	//(e.lally 2010-10-25) PLID 40994
	CLinksSetupDlg	&m_LinksSheet;

// Operations
public:
	BOOL CheckPermissions();
	void GoToBilling();
	void ShowTabs();
	int ShowPrefsDlg();
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdminView)
	public:
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnInitialUpdate();
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual			~CAdminView();
	virtual int	Hotkey (int key);
	virtual void OnSelectTab(short newTab, short oldTab);
	BOOL PrintTab(bool bPreview, CPrintInfo *pInfo = 0);

	//Used for initial tab selection.
	bool m_bTriedFirstTab;
	// Generated message map functions
protected:
	//{{AFX_MSG(CAdminView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateFilePrint(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);
	afx_msg void OnPrintPreview();
	afx_msg void OnPrint();
	// (a.walling 2016-02-15 08:13) - PLID 67827 - Combine Locations
	afx_msg void OnLocationCombine();
	afx_msg void OnUpdateLocationCombine(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//global stuff that should be local

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
