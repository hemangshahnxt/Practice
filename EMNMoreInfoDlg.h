#if !defined(AFX_EMNMOREINFODLG_H__B40A2A33_A778_4879_8426_26B4CDDB57A8__INCLUDED_)
#define AFX_EMNMOREINFODLG_H__B40A2A33_A778_4879_8426_26B4CDDB57A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMNMoreInfoDlg.h : header file
//

#include "Client.h"
#include "EmrTreeWnd.h"
#include "EmrHelpers.h"
#include <vector>
#include "EmrRc.h"
#include <boost/unordered_set.hpp>

class CEMN;

//TES 3/31/2009 - PLID 33750 - Removed Strength and DosageFormID
enum EMedicationListColumns {
	mclID, 
	mclMedicationID, 
	mclProblemIcon,	// (j.jones 2008-07-21 14:02) - PLID 30792
	mclMedication, 
	mclExplanation, 
	mclRefills, 
	mclPills, 
	mclUnit,
	mclObjectPtr, // (a.walling 2007-09-28 18:10) - PLID 27568 - EMNMedication* pointer
	mclNewCropGUID, // (j.gruber 2009-03-30 11:42) - PLID 33736 - sync new crop information
	mclIsDiscontinued, // (j.gruber 2009-03-30 11:42) - PLID 33736 - sync new crop information
};

// (z.manning, 04/11/2007) - PLID 25569
enum EChartListColumns
{
	chartID = 0,
	chartDescription,
	chartDelimitedCategoryIDs, // (z.manning 2013-06-04 10:43) - PLID 56962
};

// (z.manning, 04/11/2007) - PLID 25569
enum ECategoryListColumns
{
	catID = 0,
	catDescription,
};

/////////////////////////////////////////////////////////////////////////////
// CEMNMoreInfoDlg dialog

class CReconcileMedicationsDlg;

// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
class CEMNMoreInfoDlg : public CNxDialog, public Emr::InterfaceAccessImpl<CEMNMoreInfoDlg>
{
// Construction
public:
		


	CEMNMoreInfoDlg(CWnd* pParent);   // standard constructor
	~CEMNMoreInfoDlg();

	NXDATALISTLib::_DNxDataListPtr m_DiagCodeCombo;
	NXDATALISTLib::_DNxDataListPtr m_MedicationList;
	NXDATALIST2Lib::_DNxDataListPtr m_pProviderCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pLocationCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pStatusCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pProcedureCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pProcedureList;
	NXDATALISTLib::_DNxDataListPtr m_pTemplateCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pSecondaryProvCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pChartCombo; // (z.manning, 04/11/2007) - PLID 25569
	NXDATALIST2Lib::_DNxDataListPtr m_pCategoryCombo; // (z.manning, 04/11/2007) - PLID 25569
	
	// (c.haag 2008-07-07 11:06) - PLID 30607 - Todo alarms for this EMN
	NXDATALIST2Lib::_DNxDataListPtr m_pTodoList;
	// (d.lange 2011-03-22 16:40) - PLID 42136 - Assistant/Technician combo
	NXDATALIST2Lib::_DNxDataListPtr m_pTechnicianCombo;

	long m_nPatientID;
	BOOL m_bIsTemplate;
	
	// (a.walling 2012-07-06 17:37) - PLID 49154 - More info synchronization
	void OnMoreInfoChanged();

	// (a.walling 2007-10-01 09:09) - PLID 27568 - Use a pointer to an EMNMedication
	void AddMedication(EMNMedication* pMed);

	// (a.walling 2007-09-28 17:53) - PLID 27568 - Remove the prescription
	// not just the first row that has the same drug
	void RemoveMedication(EMNMedication* pMedication);

	//TES 12/26/2006 - PLID 23400 - There can be more than one now.
	void SetProviders(const CArray<long,long> &arProviderIDs);
	// (j.gruber 2007-01-08 11:22) - PLID 23399 - Secondary Providers
	void SetSecondaryProviders(const CArray<long,long> &arSecondaryProviderIDs);
	// (d.lange 2011-03-23 10:26) - PLID 42136 - Assistant/Technician
	void SetTechnicians(const CArray<long,long> &arTechnicianIDs);
	
	void SetLocation(long nLocationID);
	// (a.walling 2008-12-30 17:03) - PLID 30252 - If bUpdate, we want to update the dialog and EMN as well
	void SetDate(COleDateTime dt, BOOL bUpdate = FALSE);
	void SetStatus(long nStatus);
	// (z.manning, 04/11/2007) - PLID 25569 - Chart and category
	void SetChart(const long nChartID);
	void SetCategory(const long nCategoryID);
	long GetChartID();
	NXDATALIST2Lib::IRowSettingsPtr GetChartRowByID(const long nChartID);
	boost::unordered_set<long> GetLinkedCategoryIDs(const long nChartID);

	// (z.manning, 05/22/2007) - PLID 25569 - The functions help with the loading and handling when charts/categories
	// change because only certain (user-specified) combinates are allowed.
	// (z.manning 2013-06-04 12:37) - PLID 56962 - Renamed this as it no longer does a full requery
	void RefilterCategoryList();
	void TrySetCategorySelection(long nCategoryID, BOOL bSilent);

	BOOL AddProcedure(long nProcedureID, CString strName);
	BOOL LookupProcedureNameFromCombo(IN long nProcedureID, OUT CString& strProcName);
	void RemoveProcedure(long nProcedureID);

	void Initialize(CEMN *pEMN);
	
	void RefreshPrescriptionList();
	// (a.walling 2007-08-06 12:07) - PLID 23714 - Refresh the procedure list
	void RefreshProcedureList();
	
	// (a.walling 2007-08-06 13:20) - PLID 23714 - Refresh patient demographics
	void RefreshPatientDemographics();
	// (c.haag 2008-07-07 12:59) - PLID 30607 - Whenever we query for EMR todo's, we use the SQL returned here
	CString GetTodoSelectSql();
	// (c.haag 2008-07-08 17:42) - PLID 30607 - Returns additional filters for any visible todo list query
	CString GetTodoFilterSql();
	// (c.haag 2008-07-07 13:14) - PLID 30607 - Returns the detail name corresponding to an EMN detail todo
	CString GetEMNTodoDetailName(long nTaskID, long nRegardingID);
	// (c.haag 2008-07-07 11:10) - PLID 30607 - Do a full refresh of the EMN todo list
	void RefreshTodoList();
	// (c.haag 2008-07-07 12:55) - PLID 30607 - Refreshes a single row in the todo list
	void RefreshTodoListItem(long nTaskID);
	// (c.haag 2008-07-07 13:53) - PLID 30607 - Removes an EMN todo list item
	void RemoveTodoListItem(long nTaskID);

	CEMN *m_pEMN;

	CEMN* GetEMN()
	{
		return m_pEMN;
	}

	void SetReadOnly(BOOL bReadOnly);

	void SetSaveDocsInHistory(BOOL bSave);

	// (a.walling 2007-07-03 13:12) - PLID 23714 - Updates any info on the moreinfo dialog due to table checkers
	void UpdateChangedInfo();

	
	//TES 10/30/2008 - PLID 31269 - Call after a problem gets changed to refresh all the problem icons.
	void HandleProblemChange(CEmrProblem* pChangedProblem);

	// (a.walling 2008-04-03 13:16) - PLID 29497 - Added NxButtons
	// (j.gruber 2009-05-11 18:00) - PLID 33688 - added other prov button
	// (c.haag 2009-08-10 11:02) - PLID 29160 - Added Merge to printer
// Dialog Data
	//{{AFX_DATA(CEMNMoreInfoDlg)
	enum { IDD = IDD_EMN_MORE_INFO_DLG };
	NxButton	m_nxbPrintDirectlyToPrinter;
	NxButton	m_btnSaveInHistory;
	NxButton	m_btnMergeToPrinter;
	CNxIconButton	m_btnUseEMChecklist;
	CNxIconButton	m_btnUpdatePatInfo;
	CNxIconButton	m_btnConfigPreview;
	CNxIconButton	m_btnShowHistory;
	CNxIconButton	m_btnEditTemplates;
	CNxIconButton	m_btnMakeDefault;
	CNxIconButton	m_btnMergeToOther;
	CNxIconButton	m_btnMergeTo;
	CNxIconButton	m_btnSavePrintPrescriptions;
	CNxIconButton	m_btnAddMedication;
	CNxColor	m_bkg3;
	CNxColor	m_bkg4;
	CNxColor	m_bkg5;
	CDateTimePicker	m_dtDate;
	CNxColor	m_bkg8;
	CNxColor	m_bkg9;
	CNxColor	m_bkg10;
	CNxLabel	m_nxlProviderLabel;
	CNxLabel	m_nxlSecondaryProvLabel;
	// (d.lange 2011-03-22 17:46) - PLID 42136 - Hyperlink for multiple Assistant/Technician selected
	CNxLabel	m_nxlTechnicianLabel;
	CNxEdit	m_nxeditEmnDescription;
	CNxEdit	m_nxeditEmnPatientName;
	CNxEdit	m_nxeditEmnAge;
	CNxEdit	m_nxeditEmnGender;
	CNxEdit	m_nxeditEditEmrNotes;
	CNxStatic	m_nxstaticNotesLabel;
	CNxStatic	m_nxstaticDiagcodesLabel;
	CNxStatic	m_nxstaticEmrDefaultFor;
	NxButton	m_radioTodoIncomplete;
	NxButton	m_radioTodoCompleted;
	NxButton	m_radioTodoAll;
	CNxIconButton m_btnOtherProviders;
	CNxIconButton	m_btnSpellCheckNotes;
	CNxIconButton	m_btnEditServices;
	CNxIconButton m_btnEPrescribing; // (b.savon 2013-01-24 10:38) - PLID 54817
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMNMoreInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	// (s.dhole 2014-02-28 14:31) - PLID 61007 Removed vertula function BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (z.manning 2013-06-05 10:00) - PLID 56962
	BOOL m_bInitialized;

	// (a.walling 2007-07-03 11:46) - PLID 23714 - Use table checkers
	// (a.walling 2007-10-04 10:17) - PLID 23714 - Added CPTCode checker
	// (c.haag 2008-07-07 12:50) - PLID 30607 - Todo alarms
	// (d.lange 2011-03-24 11:52) - PLID 42136 - Added checker for Assistant/Tech
	CTableChecker  m_checkProcedures, m_checkProviders, m_checkLocations,
		m_checkCategories, m_checkCharts, m_checkCategoryChartLink,  m_checkTechnicians;

	CBrush m_brush;

	COLORREF m_bkgColor;

	// (j.jones 2013-10-17 12:05) - PLID 58981 - added infobutton icon
	HICON m_hInfoButtonIcon;

	BOOL m_bReadOnly;

	//Calculates the patient's age based on the EMN date
	void CalculateAge();

	//Enables/disables controls, based on whether we're a template, and whether we're read-only.
	void EnableControls();

	void EnsureDefaultTemplate();

	// (a.walling 2012-10-01 08:56) - PLID 52931 - GetDefaultTemplateWhereClause now in CEMN
	//TES 10/26/2007 - PLID 26831 AddFolderToList() returns this, we need to delete it before destroying the window.
	CAddFolderToListThread *m_pAddFolderThread;

	//Need to clean this up after asynchronously preparing the bill list's embedded dropdowns.
	CWinThread *m_pPrepareBillListThread;
	BOOL m_bPreparingBillList;

	//Needed when setting the selection during a requery.
	long m_nPendingLocationID;
	long m_nPendingProviderID;
	long m_nPendingSecondaryProviderID;	
	long m_nPendingTechnicianID;

	//TES 12/26/2006 - PLID 23400 - Called when they select {Multiple Providers} or click on the hyperlink, allows them to 
	// select which provider to assign to this EMN.  Updates the screen and m_pEMN.
	void SelectProviders();

	void SelectSecondaryProviders();

	// (d.lange 2011-03-22 17:26) - PLID 42136 - handle multiple selection for Assistant/Technician
	void SelectAssistantTechnician();




	
	// (a.walling 2007-07-03 13:08) - PLID 23714
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);

	// (c.haag 2008-07-08 17:34) - PLID 30607 - Colorizes a todo item
	void ColorizeEMNTodoListItem(NXDATALIST2Lib::IRowSettingsPtr& pRow);

	
	// (j.jones 2008-07-22 09:05) - PLID 30792 - this function will show or hide
	// the problem icon column in the medication list, based on whether any meds have the icon
	void ShowHideProblemIconColumn_MedicationList();



	// (j.jones 2008-07-25 09:00) - PLID 30792 - placed problem editing into their own functions
	void EditMedicationProblems();

	// (c.haag 2009-05-28 15:47) - PLID 34312 - Placed problem linking inside their own functions
	void LinkMedicationProblems();


	// (c.haag 2009-05-28 15:22) - PLID 34312 - This function is called when the user wants to link something
	// in the more info section with one or more existing problems. Returns TRUE if at least one problem was
	// added to the output array
	BOOL LinkProblems(CArray<CEmrProblemLink*,CEmrProblemLink*>& aryEMRObjectProblemLinks,
					  EMRProblemRegardingTypes eprtType, long nEMRRegardingID,
					  OUT CArray<CEmrProblemLink*,CEmrProblemLink*>& aNewProblemLinks);
	// (j.jones 2013-01-09 11:55) - PLID 54530 - moved medication reconciliation functions to EmrTreeWnd

	
	// (j.jones 2008-05-20 13:14) - PLID 30079 - added OnLButtonDownMedications
	// (j.jones 2008-07-22 12:11) - PLID 30792 - added OnLeftClickDiags
	// (j.gruber 2009-05-11 18:00) - PLID 33688 - added other prov button handler
	// Generated message map functions
	//{{AFX_MSG(CEMNMoreInfoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnAddMedication();
	afx_msg void OnBtnWritePrescriptions();
	afx_msg void OnRButtonDownMedications(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishingMedications(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedMedications(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);	
	afx_msg void OnEditingStartingMedications(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnEmnMedicationAdded(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmrProcedureAdded(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSelChosenEmnProvider(LPDISPATCH lpRow);
	afx_msg void OnSelChosenEmnLocation(LPDISPATCH lpRow);
	afx_msg void OnSelChosenEmnChart(LPDISPATCH lpRow);
	afx_msg void OnSelChosenEmnCategory(LPDISPATCH lpRow);
	afx_msg void OnChangeEmnDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChosenEmnStatus(LPDISPATCH lpRow);
	afx_msg void OnChangeEmnDescription();
	afx_msg void OnChangeEmnNotes();
	afx_msg void OnSelChosenEmnProcedureCombo(LPDISPATCH lpRow);
	afx_msg void OnRButtonDownEmnProcedureList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRemoveProcedure();
	afx_msg void OnMergeTo();
	afx_msg void OnEmrMakeDefault();
	afx_msg void OnSelChosenTemplateList(long nRow);
	afx_msg void OnPatemrEditTemplates();
	afx_msg void OnBtnMergeToOther();
	afx_msg void OnEmrSaveDocsInHistoryCheck();
	afx_msg void OnEmrMergeToPrinterCheck();	afx_msg LRESULT OnAddFolderNext(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAddFolderCompleted(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPrepareBillListCompleted(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTrySetSelFinishedEmnLocation(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedEmnProvider(long nRowEnum, long nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRequeryFinishedEmnProvider(short nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRequeryFinishedEmnSecondary(short nFlags);
	afx_msg void OnRequeryFinishedEmnTechnician(short nFlags);
	afx_msg void OnTrySetSelFinishedEmnSecondary(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedEmnTechnician(long nRowEnum, long nFlags);
	afx_msg void OnSelChosenEmnSecondary(LPDISPATCH lpRow);
	afx_msg void OnSelChosenEmnTechnician(LPDISPATCH lpRow);
	afx_msg void OnLeftClickBill(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedEmnChart(short nFlags);
	afx_msg void OnRequeryFinishedEmnCategory(short nFlags);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDestroy();
	afx_msg void OnConfigPreview();
	afx_msg void OnBtnUpdatePatInfo();
	afx_msg void OnLButtonDownMedications(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnPrintPrescriptionsToPrinter();
	afx_msg void OnRequeryFinishedEmnTodoList(short nFlags);
	afx_msg void OnDblClickCellEmnTodoList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnRButtonDownEmnTodoList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditEMNTodo();
	afx_msg void OnDeleteEMNTodo();
	afx_msg void OnBnClickedEmnOtherProvs();
	// (a.walling 2012-03-23 16:04) - PLID 49190 - Confidential info now in top level frame
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	afx_msg void OnSpellCheckNotes();
	
	// (a.walling 2012-03-23 15:27) - PLID 49154 - Routes commands up to the top-level frame
	afx_msg BOOL RouteCommandToParent(UINT nID);

	// (a.walling 2012-03-23 18:01) - PLID 50638 - Notes, Description
	afx_msg void OnUpdateNotes(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDescription(CCmdUI* pCmdUI);

	// (a.walling 2012-03-23 09:13) - PLID 49154 - Update the interface while idle
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);

	// (a.walling 2012-07-06 17:37) - PLID 49154 - More info synchronization - keep track of last set providers and etc
	std::vector<long> m_lastProviders;
	std::vector<long> m_lastSecondaryProviders;
	std::vector<long> m_lastTechnicians;

	afx_msg void OnBnClickedBtnRxQueueMoreInfo();

	// (b.savon 2014-02-25 15:53) - PLID 60913 - UPDATE - Add two buttons in More Info topic for "Merge Summary of Care" and "Merge Clinical Summary"
	afx_msg void OnBnClickedMergeSummaryCareBtn();
	afx_msg void OnBnClickedMergeClinicalSummaryBtn();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMNMOREINFODLG_H__B40A2A33_A778_4879_8426_26B4CDDB57A8__INCLUDED_)

