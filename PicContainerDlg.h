#if !defined(AFX_PICCONTAINERDLG_H__AFBF66FA_C391_4DA4_98E6_E53683F88452__INCLUDED_)
#define AFX_PICCONTAINERDLG_H__AFBF66FA_C391_4DA4_98E6_E53683F88452__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PicContainerDlg.h : header file
//

#include "EmrFrameWnd.h"
#include "EmrPatientFrameWnd.h"
#include "MergeEngine.h"
#include "HistoryDlg.h"
#include "EmrUtils.h"
#include "PatientLabsDlg.h"
#include "PicViews.h"

/////////////////////////////////////////////////////////////////////////////
// CPicContainerDlg dialog
class CProcInfoCenterDlg;
class CBillingModuleDlg;
class CEmrEditorDlg;
class CEMN;
class CPicDocTemplate;

enum ClosePicReason {
	cprNone = 0,//Simply close without doing anything
	cprScheduler = 1, //Close, then go to the scheduler module.
	cprEmnTemplate = 2, //Close, then switch to the EMR tab, and start editing the template with the id given in LPARAM
};

enum ClosePicAction {
	cpaCreateBill = 0,
	cpaCreateQuote,
	cpaWritePrescriptions,
	cpaDoNothing,
	cpaDoPartialBilling,	// (j.dinatale 2012-01-18 17:22) - PLID 47539 - we can do partial billing now
};

//TES 2/3/2006 - Used when checking for duplicates.
struct DetailInfo
{
	CEMNDetail *pDetail;
	BOOL m_bMarkedForDeletion;
	CString m_strFormatted;
};

class CNxRibbonAutoComboBox;

// (a.walling 2011-10-20 21:22) - PLID 46076 - Facelift - EMR frame window
// (a.walling 2012-02-28 14:53) - PLID 48451 - Now deriving from CEmrPatientFrameWnd
class CPicContainerDlg : public CEmrPatientFrameWnd
{
	DECLARE_DYNCREATE(CPicContainerDlg)
// Construction
public:
	CPicContainerDlg();   // standard constructor
	virtual ~CPicContainerDlg();

	long m_nColor;

	// (j.jones 2012-09-26 14:04) - PLID 52879 - added drug interactions
	// (j.jones 2012-11-30 11:09) - PLID 53194 - added bShowEvenIfUnchanged, which
	// will show the dialog even if its current interactions have not changed since the last popup
	void ViewDrugInteractions(bool bShowEvenWhenEmpty = false, bool bShowEvenIfUnchanged = false);

protected:

	////
	/// UI state

	afx_msg void OnUpdateViewPic(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewHistory(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewLabs(CCmdUI* pCmdUI);

	// (a.walling 2011-12-09 17:16) - PLID 46643 - Tool buttons and UI components (time, etc)
	// (a.wilson 2012-4-30) PLID 49479 - removing the graph button because it is no longer necessary.
	//afx_msg void OnUpdateViewGraphs(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewEPrescribing(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewMedAllergy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewBarcode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewEyemaginations(CCmdUI* pCmdUI);
	// (b.savon 2012-02-22 10:39) - PLID 48307 - Recall tool button
	afx_msg void OnUpdateViewRecall(CCmdUI* pCmdUI);
	// (a.walling 2012-04-02 16:51) - PLID 49360 - Device Import handling
	afx_msg void OnUpdateViewDeviceImport(CCmdUI* pCmdUI);
	// (j.jones 2012-09-26 14:04) - PLID 52879 - added drug interactions
	afx_msg void OnUpdateViewDrugInteractions(CCmdUI* pCmdUI);

	// (a.walling 2012-02-28 08:27) - PLID 48429 - EmrEditor is for patients
	afx_msg void OnUpdateStatusTotalTimeOpen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusCurrentTimeOpen(CCmdUI* pCmdUI);

	/// Merging
	// (a.walling 2012-04-30 12:47) - PLID 49832 - Merging UI

	afx_msg void OnUpdateMergeFromPacket(CCmdUI* pCmdUI);

	afx_msg void OnUpdateMergeFromTemplate(CCmdUI* pCmdUI);

	afx_msg void OnUpdateEditPacket(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditTemplate(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNewTemplate(CCmdUI* pCmdUI);

	afx_msg void OnUpdateMergePacketCombo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMergeTemplateCombo(CCmdUI* pCmdUI);

	afx_msg void OnUpdateMergeAdvancedSetup(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMergeRefresh(CCmdUI* pCmdUI);

	afx_msg void OnUpdateMergeOptions(CCmdUI* pCmdUI);

	/// Single EMN Merge

	// (a.walling 2012-10-01 09:15) - PLID 52119 - Handle merging from single EMN, setting default templates

	afx_msg void OnUpdateEmnMergeFromTemplate(CCmdUI* pCmdUI);

	afx_msg void OnUpdateEmnMergeFromOther(CCmdUI* pCmdUI);

	afx_msg void OnUpdateEmnEditTemplate(CCmdUI* pCmdUI);

	afx_msg void OnUpdateEmnMergeTemplateCombo(CCmdUI* pCmdUI);

	afx_msg void OnUpdateEmnMergeTemplateMakeDefault(CCmdUI* pCmdUI);

	CNxRibbonAutoComboBox* GetRibbonPacketCombo();
	CNxRibbonAutoComboBox* GetRibbonTemplateCombo();
	CNxRibbonAutoComboBox* GetRibbonEmnTemplateCombo(); // (a.walling 2012-10-01 09:15) - PLID 52119 - combo for single EMN merging

	struct MergeOptions
	{
		MergeOptions()
			: saveToHistory(true)
			, reverseOrder(false)
			, separateHistory(false)
			, toPrinter(false)
			, allEMNs(true)
		{}

		void Load();
		void Save();

		bool saveToHistory;
		bool reverseOrder;
		bool separateHistory;
		bool toPrinter;
		bool allEMNs;
	};

	bool m_bLoadedMergePacketCombo;
	bool m_bLoadedMergeTemplateCombo;

	MergeOptions m_mergeOptions;

	////
	/// UI commands

	afx_msg void OnViewPic();
	afx_msg void OnViewHistory();
	afx_msg void OnViewLabs();

	// (a.walling 2011-12-09 17:16) - PLID 46643 - Tool buttons and UI components (time, etc)
	// (a.wilson 2012-4-30) PLID 49479 - removing the graph button because it is no longer necessary.
	//afx_msg void OnViewGraphs();
	afx_msg void OnViewEPrescribing();
	afx_msg void OnViewMedAllergy();
	afx_msg void OnViewBarcode();
	afx_msg void OnViewEyemaginations();
	// (b.savon 2012-02-22 10:39) - PLID 48307 - Recall tool button
	afx_msg void OnViewRecall();
	// (a.walling 2012-04-02 16:51) - PLID 49360 - Device Import handling
	afx_msg void OnViewDeviceImport();
	// (j.jones 2012-09-26 14:04) - PLID 52879 - added drug interactions
	afx_msg void OnViewDrugInteractions();

	
	////
	/// Merging
	// (a.walling 2012-04-30 13:38) - PLID 50072 - Merging implementation

	afx_msg void OnMergeFromPacket();
	afx_msg void OnMergeFromPacketToWord();
	afx_msg void OnMergeFromPacketToPrinter();

	void DoMergeFromPacket();

	afx_msg void OnMergeFromTemplate();
	afx_msg void OnMergeFromTemplateToWord();
	afx_msg void OnMergeFromTemplateToPrinter();

	void DoMergeFromTemplate();

	afx_msg void OnEditPacket();
	afx_msg void OnEditTemplate();
	afx_msg void OnNewTemplate();
	afx_msg void OnCopyTemplate();
	
	void DoCreateNewTemplate(const CString& strBaseTemplate);

	afx_msg void OnMergePacketCombo();
	afx_msg void OnMergeTemplateCombo();

	afx_msg void OnMergeAdvancedSetup();
	afx_msg void OnMergeRefresh();

	afx_msg void OnMergeOptions(UINT nID);

	/// Single EMN Merge

	// (a.walling 2012-10-01 09:15) - PLID 52119 - Handle merging from single EMN, setting default templates

	afx_msg void OnEmnMergeFromTemplate();
	afx_msg void OnEmnMergeFromTemplateToWord();
	afx_msg void OnEmnMergeFromTemplateToPrinter();

	afx_msg void OnEmnMergeFromOther();
	afx_msg void OnEmnMergeFromOtherToWord();
	afx_msg void OnEmnMergeFromOtherToPrinter();

	void DoEmnMergeFromTemplate(CString strTemplateFileName, bool bSaveInHistory, bool bDirectToPrinter);

	afx_msg void OnEmnEditTemplate();

	afx_msg void OnEmnMergeTemplateCombo();

	afx_msg void OnEmnMergeTemplateMakeDefault();

public:
	//When opening a PicContainerDlg, you must give it either an EmrGroupID or a ProcInfoID.
	//You may also optionally give it an EMN ID to load initially, or an EMN Template ID to create a new EMN for, 
	//in which case it will launch an EMN with the given criteria.
	//TES 7/26/2005 - Now there is just one function, that takes a PicID, and a tab (can be 0, 1, or 2).
	// (b.cardillo 2006-06-13 18:29) - You may also specify the HWND of your window if you want 
	// to be notified when the user finishes closing the PIC container created by this function.
	// (a.walling 2012-05-01 15:31) - PLID 50117 - Tab numbers don't make sense anymore, now it is bShowPIC
	// (a.walling 2013-01-17 12:08) - PLID 54666 - Removed unused params OPTIONAL HWND hwndReportClosureToWindow = NULL, long nEMNCategoryID = -1
	int OpenPic(long nPicID, bool bShowPIC, long nEmnID = -1, long nEmnTemplateID = -1);

	// (j.dinatale 2012-07-12 14:19) - PLID 51481 - need to keep track of how we opened the pic container
	bool OpenedForPic();

	long GetCurrentPicID();
	long GetCurrentProcInfoID();
	long GetCurrentEMRGroupID();

	// (j.dinatale 2012-06-29 12:49) - PLID 51282 - need to refresh certain panes when a file is attached to history
	void RefreshPhotoPane();

	// (a.walling 2007-08-27 09:17) - PLID 26195 - Return whether the EMR is unsaved or not.
	BOOL IsEMRUnsaved();

	// (j.jones 2011-07-15 13:45) - PLID 42111 - takes in an image file name (could be a path),
	// and returns TRUE if any Image detail on this EMR references it
	BOOL IsImageFileInUseOnEMR(const CString strFileName);

	CBillingModuleDlg *m_BillingDlg;

	// (c.haag 2006-04-12 09:26) - PLID 20040 - We now cache the IsCommitted
	// status of a PIC
	inline BOOL GetIsCommitted() { return m_bIsCommitted; }
	void SetIsCommitted(BOOL bCommitted) { m_bIsCommitted = bCommitted; }
	//TES 2006-08-08 - PLID 21667 - Actually commits the PIC, in data.
	void Commit();

	void HandleNewProcInfoProcedures(CArray<long, long> &arProcIDs);
	void HandleDeletingProcInfoProcedures(CArray<long, long> &arProcIDs);
	void HandleNewEmrProcedures(CArray<long, long> &arProcIDs);
	//TES 5/20/2008 - PLID 27905 - Pass in the EMN that they're being deleted from, so we can take that into account when
	// re-generating our list of IDs.
	void HandleDeletingEmrProcedures(CArray<long, long> &arProcIDs, CEMN *pEMN);
	BOOL Save();

	CString GetDelimitedProcedureList();

	//TES 6/17/2008 - PLID 30414 - Print out prescriptions for the given EMN's medications.  If you already know the template
	// name, pass it in, otherwise the function will calculate it.
	void PrintPrescriptions(CEMN *pEMN, BOOL bDirectlyToPrinter, const CString &strTemplateName = "");

	// (z.manning, 04/25/2008) - PLID 29795 - Added NxIconButtons for merge buttons
// Dialog Data
	//{{AFX_DATA(CPicContainerDlg)
	enum { IDD = IDD_PIC_CONTAINER_DLG };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPicContainerDlg)
public:

protected:

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation

public:
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	// (j.jones 2014-08-04 16:11) - PLID 63181 - added Ex handling
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);

	// (j.jones 2007-02-06 15:22) - PLID 24493 - track when the user changes the workstation time
	void FireTimeChanged();

public:
	// (c.haag 2007-03-07 15:58) - PLID 25110 - Patient ID and name functions
	void SetPatientID(long nPatID);
	long GetPatientID() const;
	CString GetPatientName() const;

	// (a.walling 2008-05-05 11:11) - PLID 29894 - Function sets and returns the new surgery appt id for this PIC
	long UpdateSurgeryAppt();

	// (j.jones 2010-03-31 17:33) - PLID 37980 - returns TRUE if m_pEmrEditorDlg is non-null,
	// and has an EMN opened to a topic that is writeable
	BOOL HasWriteableEMRTopicOpen();

	// (j.jones 2010-04-01 11:58) - PLID 37980 - gets the pointer to the active EMN, if there is one
	CEMN* GetActiveEMN();

	CEMR* GetEmr(); // (z.manning 2011-10-28 17:48) - PLID 44594

	// (j.jones 2013-03-01 09:41) - PLID 52818 - cache the e-prescribing type
	CLicense::EPrescribingType m_eRxType;

	// (a.walling 2012-02-28 08:27) - PLID 48429 - EmrEditor is for patients
	CEmrEditorDlg* GetEmrEditor()
	{
		return m_emrTreePane.GetEmrEditor();
	}

protected:
	virtual void InitializeRibbonTabButtons();

	////
	/// Tools

	scoped_ptr<CEmrMedAllergyViewerDlg> m_pMedAllergyViewerDlg;
	// (c.haag 2010-08-03 17:14) - PLID 38928 - Display the medication / allergy viewer
	void ShowMedAllergyViewer(BOOL bShowExplanation);

	scoped_ptr<CEMRBarcodeDlg> m_pBarcodeDlg;	// (j.dinatale 2011-07-26 17:41) - PLID 44702 - barcode dialog to display this emr's barcode

	// (j.jones 2012-09-26 13:58) - PLID 52879 - added a drug interactions dialog that uses the EMR as its parent
	scoped_ptr<CDrugInteractionDlg> m_pDrugInteractionDlg;

	//(a.wilson 2011-10-17) PLID 45971
	CString m_strEyePath;
	BOOL GenerateLumaXml();

	////
	/// Messages

	////

	BOOL m_bInitialized;

	long m_nPicID;
	bool m_bIsNew;
	bool m_bLoadedForPic;	// (j.dinatale 2012-07-12 14:19) - PLID 51481 - need to keep track of how we opened the pic container

	long m_nInitialTemplateID;
	long m_nInitialEmnID;

	long m_nEmrGroupID;
	long m_nProcInfoID; //These are just here so we don't have to keep pulling them from data.
	BOOL m_bIsCommitted;

	long m_lNewEmnCategoryID;

	//Makes sure that m_nEmrGroupID and m_nProcInfoID are valid.
	void EnsureIDs();

	// (j.jones 2010-04-01 15:50) - PLID 34915 - added patient gender, age, and primary insurance company
	BYTE m_cbPatientGender;
	CString m_strPatientAge;
	CString m_strPrimaryInsCo;

	//TES 6/17/2008 - PLID 30411 - Changed long nBillInsuredPartyID to an LPARAM, which will be interpreted differently 
	// based on cpaAction.
	BOOL SaveAndClose(ClosePicAction cpaAction = cpaDoNothing, CEMN *pEMN = NULL, LPARAM paramActionInfo = NULL);
	void CloseCleanup();

	// (z.manning 2010-01-13 17:52) - PLID 36864 - Removed unused strMergeTemplateName parameter
	BOOL GenerateMergeData(CMergeEngine &mi, OUT CString &strHeaders, OUT CString &strData, BOOL bEditingTemplate);
	BOOL GenerateCommonMergeData(const CString& strEMNIDFilter, CList<MergeField,MergeField&> &listMergeFields);
	BOOL GenerateCategoryMergeData(CList<MergeField,MergeField&> &listMergeFields, BOOL bEditingTemplate);
	//TES 3/21/2006 - Add an output array of the CEMNs loaded by this function, so GenerateRelatedMergeData won't have to reload them.
	// (z.manning 2010-01-13 17:52) - PLID 36864 - Removed unused strMergeTemplateName parameter
	BOOL GenerateDetailMergeData(const CString& strEMNIDFilter, CMergeEngine& mi, CArray<DetailInfo, DetailInfo>& aDetails, POSITION& pLastNonAlphabetized, CList<MergeField,MergeField&> &listMergeFields, BOOL bEditingTemplate, OUT CArray<CEMN*,CEMN*> &arEmns);
	BOOL GenerateRelatedMergeData(const CArray<CEMN*,CEMN*>& arEMNs, CList<MergeField,MergeField&> &listMergeFields, POSITION posLastNonAlphabetized);
	CString GetParagraph(CArray<CEMNDetail*, CEMNDetail*>& aDetails, long nCategoryID, EmrCategoryFormat Format, CMergeEngine &mi, CMap<long,long, CArray<long,long>*, CArray<long,long>*>& mapEmrInfoCat);
	void GenerateTempMergeFiles(CArray<CEMNDetail*, CEMNDetail*>& aDetails, CMergeEngine &mi);
	void GenerateTempMergeFile(CArray<CEMNDetail*, CEMNDetail*>& aDetails, long nCategoryID, EmrCategoryFormat fmt, const CString& strCatName, CMergeEngine &mi, CMap<long,long, CArray<long,long>*, CArray<long,long>*>& mapEmrInfoCat);
	void RemoveTempMergeFiles();
	// (z.manning 2010-01-13 17:52) - PLID 36864 - Removed unused strMergeTemplateName parameter
	BOOL CheckAndWarnOfDuplicates(CList<MergeField,MergeField&> &listMergeFields, CArray<DetailInfo, DetailInfo>& aDetails);

	// (j.jones 2012-07-24 12:49) - PLID 44349 - added filter for only billable codes
	CString GetServiceCodesOutput(CString strEMNIDFilter, BOOL bOnlyIncludeBillableCodes);
	CString GetDiagCodesOutput(CString strEMNIDFilter);
	CString GetMedicationsOutput(CString strEMNIDFilter);

	// (c.haag 2007-03-07 14:51) - PLID 21207 - Returns TRUE if an EMN does not have
	// a procedure that exists in aProcIDs
	BOOL AnyProcedureNotInEMN(CArray<int,int>& aProcIDs, CEMN* pEMN);
	BOOL AnyProcedureNotInEMN(CArray<int,int>& aProcIDs, long nEMNID);

	// (a.walling 2007-10-16 17:26) - PLID 27786 - Throws an exception if the EmrGroupID of this PicT record is not null,
	// not a deleted group, and is not the desired group
	static BOOL VerifyEmrGroupID(long nPicID, long nDesiredGroupID);

	// (a.walling 2011-12-08 17:27) - PLID 46641 - Create views for Labs, History, and ProcInfoCenter
	scoped_ptr<CPicDocTemplate> m_pPicDocTemplate;
	
	// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
	CProcInfoCenterDlg* GetPicDlg(bool bAutoCreate = false);

	CHistoryDlg* GetHistoryDlg(bool bAutoCreate = false);

	CPatientLabsDlg* GetLabsDlg(bool bAutoCreate = false);

	//Stores the temp .htm files so they can be deleted.
	CStringArray m_saTempFiles;

	CArray<long,long> m_arPICProcedureIDs;

	void GenerateProcedureList();
	virtual CString GenerateTitleBarText();

	BOOL m_bPicDlgNeedsToLoad;

	//TES 7/13/2009 - PLID 25154 - This function goes through any EMNs on the dialog, and checks to make sure that, if
	// they have the "Send to HL7" box checked, they get sent to HL7.  This function should be called just before
	// closing the PIC, but after any saving has taken place.
	void SendEmnBillsToHL7();	

	// (a.walling 2012-02-28 08:27) - PLID 48429 - EmrEditor is for patients
	// (a.walling 2011-12-09 17:16) - PLID 46643 - Time tracking
	int m_nLastTotalSecondsOpen;
	int m_nLastCurrentSecondsOpen;

	BOOL BuildMergeEngineObjects(class CEMR_ExtraMergeFields& emf, class CMergeEngine& mi, BOOL bPacket);
	bool PickEMNMergeList(IN OUT CDWordArray& adwEMNIDs);

protected:
	// (c.haag 2007-03-07 15:56) - PLID 25110 - The PIC must now keep track of its
	// patient ID
	long m_nPatientID;

protected:
	// (a.walling 2012-04-30 14:33) - PLID 50076 - Packet and template combo
	virtual void SynchronizeDelayedRibbonSubitems(CMFCRibbonBaseElement* pElement);

	// Generated message map functions
	//{{AFX_MSG(CPicContainerDlg)
	BOOL Initialize();
	// (a.walling 2012-02-28 08:27) - PLID 48429 - EmrEditor is for patients
	void InitializeEmrEditor(long nEmrGroupID, long nPicID, long nInitialTemplateID, long nInitialEmnID);
	void InitializePic();
	void InitializeHistory();
	void InitializeLabs();

	// (a.walling 2012-11-09 08:45) - PLID 53670 - Create a view from path name
	virtual CMDIChildWndEx* CreateDocumentWindow(LPCTSTR lpcszDocName, CObject* pObj) override;

	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnSaveAndClose();	// (j.dinatale 2012-07-13 14:03) - PLID 51481
	afx_msg LRESULT OnClosePic(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnSaveCreateEMNBill(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSaveCreateEMNQuote(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSaveWritePrescriptions(WPARAM wParam, LPARAM lParam);
	//afx_msg void OnNewTemplate();
	afx_msg LRESULT OnNonClinicalProceduresAdded(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPrintPrescriptions(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmrMinimizePic(WPARAM wParam, LPARAM lParam);
	// (a.walling 2009-06-22 10:45) - PLID 34635 - This is useless code which may end up causing problems.
	//afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	// (a.walling 2008-09-10 13:31) - PLID 31334
	afx_msg LRESULT OnWIAEvent(WPARAM wParam, LPARAM lParam);
	// (j.jones 2009-08-25 11:37) - PLID 31459 - supported allowing an external refresh of the Proc Info payments
	afx_msg LRESULT OnReloadProcInfoPays(WPARAM wParam, LPARAM lParam);
	// (j.jones 2010-03-31 17:01) - PLID 37980 - added ability to tell the EMR to add a given image
	afx_msg LRESULT OnAddImageToEMR(WPARAM wParam, LPARAM lParam);
	// (j.jones 2010-06-21 10:22) - PLID 39010 - added ability to add a generic table to the EMR
	afx_msg LRESULT OnAddGenericTableToEMR(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSaveCreatePartialEMNBills(WPARAM wParam, LPARAM lParam);	// (j.dinatale 2012-01-18 17:32) - PLID 47539
	// (j.jones 2014-08-04 10:12) - PLID 63144 - added OnShowWindow
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PICCONTAINERDLG_H__AFBF66FA_C391_4DA4_98E6_E53683F88452__INCLUDED_)
