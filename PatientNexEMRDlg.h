#if !defined(AFX_PATIENTNEXEMRDLG_H__E0497ED4_045A_414D_95B3_96F391F6432B__INCLUDED_)
#define AFX_PATIENTNEXEMRDLG_H__E0497ED4_045A_414D_95B3_96F391F6432B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PatientNexEMRDlg.h : header file
//

#include "PatientDialog.h"
#include "EMRLockReminderDlg.h"
#include "EMRPreviewPopupDlg.h"
#include "EmrRc.h"

/////////////////////////////////////////////////////////////////////////////
// CPatientNexEMRDlg dialog

struct EMNTabInfo {
	long nID;
	CString strDescription;
	long nColor;
};

// (a.walling 2007-04-23 11:48) - PLID 24535 - Enumerator to differentiate between various IDs that may be in the EMR list
enum EMRObjectType {
	eotInvalid = -1,
	eotEMR = 0,
	eotEMN,
	eotHistory, // (z.manning 2008-07-01 16:19) - PLID 25574
	eotReserved
};

// (a.walling 2007-04-23 11:48) - PLID 24535 - store an ID and the type of object that is referenced by that id
struct EMRListObject {
	EMRObjectType eotType;
	long nID;
	// (j.jones 2007-06-19 10:17) - PLID 26276 - this is now obsolete
	//EMNCompletionStatus ecsCompletionStatus;
};

// (j.jones 2007-06-11 10:33) - PLID 26271 - store an ID of a template and its name
struct EMRTemplateInfo {
	long nID;
	CString strName;
};

// (a.walling 2007-04-23 10:54) - PLID 24535 - Create a CWinThread-derived class to colourize the EMN list when it is finished loading.
class CFinishEMRListThread : public CWinThread
{
public:
	// constructor
	CFinishEMRListThread(HWND hwnd, AFX_THREADPROC pfnThreadProc);
	// destructor
	~CFinishEMRListThread();

	static UINT FinishEMRListThreadFunction(LPVOID p);

	HANDLE m_hStopThreadEvent; // set this event to signal the thread to stop.
	HWND m_hwndNotify; // HWND of the window to send messages to
	CArray<EMRListObject, EMRListObject> m_arListObjects; // list of EMR objects to query upon for word docs and color
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized - use the patient ID
	long m_nPatientID;
};

class CPatientNexEMRDlg : public CPatientDialog
{
// Construction
public:
	CPatientNexEMRDlg(CWnd* pParent);   // standard constructor
	~CPatientNexEMRDlg();

	NXDATALIST2Lib::_DNxDataListPtr m_pNewTree;

	NXDATALIST2Lib::_DNxDataListPtr m_pExistingList;

	virtual void SetColor(OLE_COLOR nNewColor);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	// (a.walling 2010-10-14 17:32) - PLID 40978
	virtual void Refresh();

	// (b.cardillo 2006-11-20 10:37) - PLID 22565 - Need to override the RecallDetails() function 
	// so that we can reset m_nReconstructedDetailsLastNotifiedPatientID when the user changes to 
	// the tab.
	virtual void RecallDetails();
	virtual int SetControlPositions();

	// (a.walling 2009-11-24 14:48) - PLID 36418
	void PrintMultipleEMNs();

	BOOL IsEMNTabView();
	void EnsureTabsPosition();

	// (j.jones 2014-08-12 09:58) - PLID 63189 - added ability for an outside source to
	// refresh the new template list
	void RefreshNewTemplateList();

	// (j.jones 2014-08-21 11:33) - PLID 63189 - added tablechecker object for EMR templates
	CTableChecker m_templateChecker;

	/*bool m_bCancelThread; // (a.walling 2007-02-01 10:37) - PLID 24513 - Josh says I should implement a stop
	// functionality like the MirrorLink.cpp. This is an ugly fix, but we are waiting for next scope when I'll
	// rewrite all this threading correctly to not directly access all these dialog members from the thread.*/
	// (a.walling 2007-04-23 08:54) - PLID 24535 - Now we correctly synchronize everything.

	// (a.walling 2007-04-13 13:11) - PLID 25648 - Called by the preview popup to indicate that it was closed (hidden)
	void SetShowPreview(BOOL bShow);

	// (j.jones 2008-10-14 15:37) - PLID 31691 - removed m_btnMoreTemplates
	// (j.jones 2008-10-14 16:59) - PLID 14411 - added m_btnEMRAnalysis
	// (j.gruber 2009-05-26 13:04) - PLID 34348 - added m_btnWellness
	// (z.manning 2009-08-25 09:25) - PLID 31944 - Added lock manager button
	// (d.thompson 2009-11-03) - PLID 36112 - Removed the 'open a...' label
// Dialog Data
	//{{AFX_DATA(CPatientNexEMRDlg)
	enum { IDD = IDD_PATIENT_NEXEMR };
	CNxIconButton	m_btnEMRAnalysis;
	CNxIconButton	m_btnPatSummary;
	CNxIconButton	m_btnShowPreview;
	CNxIconButton	m_btnConfigureColumns;
	CNxIconButton	m_btnConfigureTabs;
	NxButton	m_btnTabViewCheck;
	CNxIconButton	m_btnEMRPatientsSeenToday;
	CNxIconButton	m_btnEMRSummary;
	CNxIconButton	m_btnPatientsToBeBilled;
	CNxIconButton	m_btnEditTemplates;
	CNxIconButton	m_btnConfigureGroups;
	CNxIconButton	m_btnEMRProblemList;
	CNxColor	m_bkgLeft;
	CNxColor	m_bkgTop;
	// (j.jones 2016-04-18 16:48) - NX-100214 - added a third colored background
	CNxColor	m_bkgBottom;
	CNxStatic	m_nxlPatientChart;
	CNxStatic	m_nxstaticEmrButtonHeader;
	NxButton	m_btnShowHistory;
	CNxIconButton m_btnWellness;
	CNxIconButton m_btnLockManager;
	CNxStatic	m_nxlNewMedicalNote;	// (j.armen 2012-04-23 14:59) - PLID 49608 - Added label for New Medical Note
	// (j.armen 2012-04-24 17:31) - PLID 49863 - Dynamic buttons
	CNxIconButton m_btnDynamic1;
	CNxIconButton m_btnDynamic2;
	CNxIconButton m_btnDynamic3;
	CNxIconButton m_btnDynamic4;
	CNxIconButton m_btnDynamic5;
	CNxIconButton m_btnDynamic6;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPatientNexEMRDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	///////////////////////////
	//Existing EMR List related
	///////////////////////////

	//Functions
	void ReloadEMRList();
	//void LoadSingleEMR(long nEMRID, bool bExpanded);
	//void LoadEMNsForEMR(long nPicID, long nEMRID, NXDATALIST2Lib::IRowSettingsPtr pParentRow);

	// (a.walling 2010-10-14 17:29) - PLID 40978
	long m_id;

	// (b.cardillo 2006-11-20 10:37) - PLID 22565 - This member is used so as to avoid prompting 
	// the user multiple times that this patient's EMRs contain reconstructed details.
	long m_nReconstructedDetailsLastNotifiedPatientID;

	// (z.manning, 03/30/2007) - PLID 25427 - Returns the number of rows currently selected in the existing list.
	long GetSelectionCountExistingList();

	// (j.fouts 2012-06-05 12:33) - PLID 49611 - Load Bulk Cache Properties
	void ReloadProperties();

	// (z.manning, 03/30/2007) - PLID 25427 - We now handle selection differently when we're in tab view since
	// it allows multiple selection, so created a general function to handle selection changes.
	void HandleSelectionChange(LPDISPATCH lpNewSel);

	// (z.manning, 03/30/2007) - PLID 25427 - The datalist's CurSel member can't be used when multiple selection is
	// allowed, so use this function to get the existing list's current selection.
	NXDATALIST2Lib::IRowSettingsPtr GetCurSelExistingList();

	// (z.manning, 03/30/2007) - PLID 25427 - Returns the EMN IDs for all currently selected rows.
	void GetAllSelectedEmnIDs(OUT CArray<long> &naryEmnIDs);

	// (z.manning, 04/06/2007) - PLID 25489 - Returns true if the given row is selected, false otherwise (including if the row is null).
	BOOL IsRowSelected(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (z.manning, 04/04/2007) - PLID 25489 - Updates the size of the chart tab control.
	// (a.wetta 2007-06-28 12:57) - PLID 26094 - No longer used
	//void UpdateChartTabsSize();

	// (z.manning, 04/04/2007) - PLID 25489 - Returns the chart ID of the chart for the currently selected tab,
	// or -1 if there is no selection or it does not apply.
	long GetCurSelChartID();
	// (z.manning, 04/05/2007) - PLID 25508 - Returns the category ID of the chart for the currently selected tab
	// or -1 if there's no selection.
	long GetCurSelCategoryID();

	// (z.manning, 05/22/2007) - PLID 25489 - FALSE if there are multiple selections in the existing list and
	// they do not all have the same chart ID, TRUE otherwise.
	BOOL DoAllSelectedRowsHaveSameChartID();

	// (j.gruber 2013-11-11 15:59) - PLID 59415 - generate clincial summary
	// (r.gonet 04/22/2014) - PLID 61805 - Added the PICID because Clinical Summaries now get associated with the PIC.
	void GenerateAndAttachClinicalSummary(long nPICID, long nEMNID, bool bGenerateXML, bool bGeneratePDF);	
	
	///////////////////////////////
	//End Existing EMR List related
	///////////////////////////////

	//////////////////////
	//New EMR List related
	//////////////////////

	//Members

	class CEmrTemplateManagerDlg* m_pdlgTemplateManager;

	CFinishEMRListThread *m_pFinishEMRListThread;

	long PopulateEMRListObjects(CArray<EMRListObject, EMRListObject> &arListObjects);

	void KillEMRListThread(long nTimeMSToWait = 0);

	//Functions

	//(e.lally 2011-10-11) PLID 44728 - Gets the SQL segment for the active nextrack procedures
	// (j.armen 2012-02-14 11:20) - PLID 48131 - Returns CSqlFragment
	CSqlFragment GetNewTreeActiveNexTrackSql(); 
	void RefreshNewList();
	long m_nRefreshNewListCount; // (a.walling 2010-08-31 12:34) - PLID 29140 - Prevent refreshing the new list more often than necessary
	void RefreshNewTreeActiveNexTrackProcedures(); //(e.lally 2011-10-11) PLID 44728 - Refreshes just the patient's active nextrack procedures portion.

	BOOL PromptUserForTemplateFromProcedureID(IN long nProcedureID, OUT long &nTemplateID);
	BOOL PromptUserForTemplateFromPic(IN long nPicID, OUT long &nTemplateID);

	void SecureControls();

	// (a.walling 2006-09-19 11:44) - PLID 18745 - Check for unlocked EMNs over a certain age
	void CheckUnlockedAgedEMNs();
	bool m_bRemindToLock; // only remind once per session
	CEMRLockReminderDlg m_pEMRLockReminderDlg;

	//TES 12/26/2006 - PLID 23403 - Similarly, check whether they need to set up their provider licensing.
	void CheckEmrProviderLicensing();
	bool m_bNeedProviderLicenseCheck; //only remind once per session

	// (z.manning 2014-02-04 15:19) - PLID 55001 - Also do a license check for Nuance dictation users
	void CheckDictationLicensing();
	bool m_bNeedDictationLicenseCheck;


	// (a.walling 2007-11-28 10:00) - PLID 28044
	bool m_bExpired;

	void RefreshEMNTabs();

	long m_nGroupIDToAutoExpand; // (z.manning 2009-08-11 16:59) - PLID 32989
	
	//////////////////////////
	//End New EMR List related
	//////////////////////////


	////////////////////////
	//Dynamic Button Related
	////////////////////////

	//Members
	// (j.armen 2012-04-24 17:35) - PLID 49863 - Replaced ButtonManager w/ a finite set of buttons
	CArray<long> m_aryDynaButtonIDs;
	void ClearDynaButtons();
	void AddDynaButton(const long& nID, CString strName);
	void EmrDynaBnClicked(const long& nID);
	// (j.jones 2007-06-11 10:32) - PLID 26271 - store the list of relevant templates
	CArray<EMRTemplateInfo,EMRTemplateInfo> m_arCurEMRTemplateInfo;
	long m_nCurEMRSelected;
	BOOL m_bCurTemplateInfoIsGeneric;	//used to save an extra call that would regenerate the same information

	//Functions
	// (j.jones 2007-06-11 10:49) - PLID 26271 - removed the nEMRID parameter in lieu
	// of the pre-loaded array
	void ReFilterButtons();
	// (j.jones 2007-06-11 10:32) - PLID 26271 - store the list of relevant templates
	void BuildCurEMRTemplateInfo(long nEMRID, BOOL bForceClear);
	void ClearCurEMRTemplateInfo();

	////////////////////////////
	//END Dynamic Button Related
	////////////////////////////

	void SetNewListStylesRecurse(NXDATALIST2Lib::IRowSettingsPtr);

	/////////
	//General
	/////////

	//Members
	CFont* m_pFont;
	HICON m_hIconHasProblemFlag;
	HICON m_hIconHadProblemFlag;
	HICON m_hIconPreview; // (z.manning 2008-07-02 14:34) - PLID 30596
	HICON m_hIconClinicalSummaryMerged; // (r.gonet 04/23/2014) - PLID 61807
	HICON m_hIconClinicalSummaryNexWeb; // (r.gonet 04/22/2014) - PLID 61807
	// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
	NxTab::_DNxTabPtr m_pEMNTabs;
	// (a.wetta 2007-06-28 11:11) - PLID 26094 - Make the chart tabs be NxTabs
	NxTab::_DNxTabPtr m_pChartTabs;
	BOOL m_bThemeOpen;	//are we in xp theme mode?
	CArray<EMNTabInfo,EMNTabInfo&> m_aryEmnCategoryTabs;
	BOOL m_bEMNTabView;
	// (a.wetta 2007-06-28 12:31) - PLID 26094 - Keep track of the chart tabs
	CArray<EMNTabInfo,EMNTabInfo&> m_aryChartTabs;

	// (a.walling 2007-04-13 09:48) - PLID 25648 - EMR Preview popup dialog
	class CEMRPreviewPopupDlg* m_pEMRPreviewPopupDlg;
	BOOL m_bShowPreview;

	// (a.walling 2007-04-13 12:59) - PLID 25648 - Preview the selected EMN in the popup
	void PreviewSelectedEMN();
	void EnsureShowPreviewButton();
	void CreatePreviewWindow();
	void HidePreviewWindow();

	//Functions
	void LaunchPICEditor(long nPicID);
	void LaunchPICEditorWithEMN(long nPicID, long nEMNID);
	void LaunchPICEditorWithNewEMN(long nPicID, long nTemplateID);
	void LaunchPICEditorWithNewEMR(long nPicID, long nTemplateID);

	long SelectEMRCollection(long x, long y, CWnd *pParent, BOOL bIsEMR);	
	void ConfigureEMNTabs();

	// (z.manning 2008-07-03 11:12) - PLID 25574 0 Opens a history document for the given row
	void OpenHistoryDocument(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (j.gruber 2009-06-04 12:37) - PLID 34484 - refresh our wellness alerts and button
	void RefreshWellnessAlerts();

	// (j.jones 2012-07-20 08:44) - PLID 41791 - This will apply our fixed sort order,
	// which will only apply to Tab View upon creation, and Tree View at all times.
	void InitializeExistingListSortOrder();

	//TES 5/1/2014 - PLID 61916 - Moved CreateNewCancerDocument() to EmrUtils

	/////////////
	//End General
	/////////////

	// (j.jones 2008-07-29 11:19) - PLID 30873 - added OnBtnPtSummary
	// (j.jones 2008-10-14 16:59) - PLID 14411 - added OnBtnEMRAnalysis
	// (j.gruber 2009-05-26 13:01) - PLID 34348 - added OnBtnWellness
	// Generated message map functions
	//{{AFX_MSG(CPatientNexEMRDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditEMR();
	afx_msg void OnDeleteEMR();
	afx_msg void OnViewHistory();
	afx_msg void OnCreateEMR();
	afx_msg void OnColumnClickingNewList(short nCol, BOOL FAR* bAllowSort);
	afx_msg void OnRowExpandedNewList(LPDISPATCH lpRow);
	afx_msg void OnNexemrConfigureGroups();
	afx_msg void OnLeftClickNewList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditEmnTemplate();
	afx_msg LRESULT OnMsgEditEMRTemplate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	// (j.jones 2014-08-04 14:55) - PLID 63182 - added Ex handling
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEMRListFinished(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEMRListWordIcon(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEMRListProblemIcon(WPARAM wParam, LPARAM lParam); // (z.manning 2010-07-28 14:48) - PLID 39874
	afx_msg LRESULT OnEMRListColor(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEMRListCompletionStatus(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLeftClickExistingList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnColumnClickingExistingList(short nCol, BOOL FAR* bAllowSort);
	afx_msg void OnRButtonDownExistingList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelChangedExistingList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnBtnPatientsToBeBilled();
	afx_msg void OnDestroy();
	afx_msg void OnMoreTemplates();
	afx_msg void OnLockEMN();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRequeryFinishedExistingList(short nFlags);
	afx_msg void OnBtnEmrSummary();
	afx_msg void OnMoveEMNToEMR();
	afx_msg void OnBtnEmrProblemList();
	afx_msg void OnRequeryFinishedNewList(short nFlags);
	afx_msg void OnBtnEmrSeenToday();
	afx_msg void OnTabViewCheck();
	afx_msg void OnBtnConfigureTabs();
	afx_msg void OnSelectTabEMNTabs(short newTab, short oldTab);
	afx_msg void OnConfigureCategories();
	afx_msg void OnEditingFinishedExistingList(LPDISPATCH lpRow, short nCol, const _variant_t &varOldValue, const _variant_t &varNewValue, VARIANT_BOOL bCommit);
	afx_msg void OnConfigureNexemrColumns();
	afx_msg void OnCurSelWasSetExistingList();
	afx_msg void OnShowPreview();
	afx_msg void OnMenuShowPreview();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnEditingFinishingExistingList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnRButtonUpNewList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelectTabChartTabs(short newTab, short oldTab);
	afx_msg void OnShowHistory();
	afx_msg void OnEditingStartingExistingList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnBtnPtSummary();
	afx_msg void OnBtnEMRAnalysis();
	afx_msg void OnBtnWellness();
	afx_msg void OnNexemrLockManager();
	// (j.jones 2010-09-30 14:11) - PLID 40730 - added ability to rename an EMR/EMN
	afx_msg void OnRenameEMR();
	//(e.lally 2011-12-13) PLID 46968 - Makes the existing patient EMN available for a patient to fill out online via NexWeb
	afx_msg void OnPublishEMNToNexWeb();
	// (j.gruber 2013-11-11 15:43) - PLID 59415 - clinical summary generation options
	afx_msg void OnGenerateClinicalSummaryXMLOnly();
	// (j.gruber 2014-05-06 14:06) - PLID 61923 - taken out
	//afx_msg void OnGenerateClinicalSummaryPDFOnly();
	//afx_msg void OnGenerateClinicalSummaryXMLAndPDF();
	afx_msg void OnGenerateClinicalSummaryCustomized(); // (j.gruber 2013-11-13 15:40) - PLID 59420
	afx_msg void OnGenerateClinicalSummaryCancerCaseSubmission(); //TES 11/21/2013 - PLID 57415
	// (j.armen 2012-04-24 17:36) - PLID 49863 - button handlers for dynamic buttons
	afx_msg void OnBnClickedEmrDyna1();
	afx_msg void OnBnClickedEmrDyna2();
	afx_msg void OnBnClickedEmrDyna3();
	afx_msg void OnBnClickedEmrDyna4();
	afx_msg void OnBnClickedEmrDyna5();
	afx_msg void OnBnClickedEmrDyna6();
	// (j.fouts 2012-06-06 11:05) - PLID 49611 - Handle the changing of preferences
	afx_msg LRESULT OnPreferenceUpdated(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATIENTNEXEMRDLG_H__E0497ED4_045A_414D_95B3_96F391F6432B__INCLUDED_)
