#if !defined(AFX_ADDPATDASHCONTROLDLG_H__703541C2_6AE7_4AA9_899C_99B4AE4972B4__INCLUDED_)
#define AFX_ADDPATDASHCONTROLDLG_H__703541C2_6AE7_4AA9_899C_99B4AE4972B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CAddPatDashboardControlDlg.h : header file
//
// (j.gruber 2012-04-13 15:46) - PLID 49700 - created for

#include "PatientsRc.h"
#include "PatientDashboardDlg.h"
#include "ConfigurePatDashboardDlg.h"


/////////////////////////////////////////////////////////////////////////////
// CAddPatDashboardControlDlg dialog
class CAddPatDashboardControlDlg : public CNxDialog
{
	
// Construction
public:
	CAddPatDashboardControlDlg(PDControl *pControl, BOOL bWriteable, CWnd* pParent = NULL);   // standard constructor

	void Load();
	void Save();
	BOOL Validate();

	PDControl *m_pControl;
	void LoadFilters();

	NXDATALIST2Lib::_DNxDataListPtr m_pTypeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pEMNItemList;
	NXDATALIST2Lib::_DNxDataListPtr m_pApptTypeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pTimeIntervalList;
	NXDATALIST2Lib::_DNxDataListPtr m_pHistoryCategoryList; // (c.haag 2015-04-29) - NX-100441

	BOOL m_bWriteable;

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

	CRect m_rctWindow;
	CRect m_rctBG;
	CRect m_rctOK;
	CRect m_rctCancel;
	CRect m_rctInclude;
	CRect m_rctChkTime;
	CRect m_rctTimeIncrement;
	CRect m_rctTimeInterval;
	CRect m_rctTimeStatic;
	CRect m_rctInfoStatic;
	CRect m_rctItem;
	CRect m_rctApptDesc;
	CRect m_rctApptBox;
	CRect m_rctDoNotShowOnCCDA;// (s.tullis 2015-02-25 09:53) - PLID 64740 
	// (r.gonet 2015-03-17 10:23) - PLID 65020 - Rectangle for the "Exclude problems flagged to 'Do not show on problem prompt'" checkbox.
	CRect m_rctExcludeDoNotShowOnProblemPrompt;
	// (c.haag 2015-04-29) - NX-100441 - History categories
	CRect m_rctHistCatDesc;
	CRect m_rctHistCatBox;

	CNxColor m_bkg;
	
	
// Dialog Data
	//{{AFX_DATA(CAddPatDashboardControlDlg)
	enum { IDD = IDD_ADD_PAT_DASH_CONTROL_DLG };		
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddPatDashboardControlDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support	
	//}}AFX_VIRTUAL

// Implementation
protected:

	void MoveAndShowWindow(int IDToMove, int IDToMoveBelow, CString strCaption = "");
	// (z.manning 2015-05-07 11:18) - NX-100439 - Removed name param as it is not needed
	void AddTypeRow(PatientDashboardType type);
	void AddTimeIntervalRow(LastTimeFormat ltf, CString strName);
	void ShowControls(PatientDashboardType type);
	void ShowTimeInterval(UINT nIdcToShowBelow);
	void ShowApptType(int idcToShowBelow, long nPxBelow);
	// (c.haag 2015-04-29) - NX-100441 - Show the History Categories list
	void ShowHistoryCategories(int idcToShowBelow, long nPxBelow);
	void ShowInclude(int idcToShowBelow, long nPxBelow, CString strCaption);
	void ShowIncludeOnly(CString strCaption);
	// (z.manning 2015-03-26 13:36) - NX-100399 - Function to show the EMN item list
	void ShowEmnItemList(UINT nIdcToShowBelow);
	void SetFilters();
	void MoveBottom(int idcMoveBelow);
	int GetSpacer();
	void HideControls();
	void SetRects();
	void ResetPositions();
	// (z.manning 2015-04-15 10:31) - NX-100388
	void UpdateVisibleRows(PatientDashboardType type);
	// (z.manning 2015-04-15 10:48) - NX-100388
	void UpdateTimeIntervalLabel();

	// Generated message map functions
	//{{AFX_MSG(CAddPatDashboardControlDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();	
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnRequeryFinishedApptTypeList(short nFlags);
	afx_msg void OnRequeryFinishedHistoryCatList(short nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void RequeryFinishedApdEmnItemList(short nFlags);
	void SelChosenApdTypeList(LPDISPATCH lpRow);
	void SelChosenApdTimeInterval(LPDISPATCH lpRow);
	void SelChosenApdEmnItemList(LPDISPATCH lpRow); // (z.manning 2015-05-04 17:46) - NX-100447
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDPATDASHCONTROLDLG_H__703541C2_6AE7_4AA9_899C_99B4AE4972B4__INCLUDED_)
