#include "PatientDialog.h"
#if !defined(AFX_HISTORYDLG_H__DC332784_2AD0_11D2_B1FA_0000C0832801__INCLUDED_)
#define AFX_HISTORYDLG_H__DC332784_2AD0_11D2_B1FA_0000C0832801__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// (z.manning 2008-07-03 15:15) - PLID 25574 - If you ever add something here other than
// OBJECT: or FORM: something then please handle it in CPatientNexEMRDlg::ReloadEMRList

// HistoryDlg.h : header file
//

#include "PhotoViewerCtrl.h"
#include "HistoryTabsDropTarget.h"
#include "NxWIA.h" // (a.walling 2014-03-12 12:31) - PLID 61334 - reduce stdafx dependencies and #imports
/////////////////////////////////////////////////////////////////////////////
// CHistoryDlg dialog
struct HistoryTabInfo {
	long nCatID;
	CString strDescription;
};

struct HistoryDragDocInfo {
	CArray<long, long> arynMailID;
	CArray<CString, CString> arystrFilename;
	CArray<CString, CString> arystrFileNameOnly;
	CArray<BOOL, BOOL> arybIsPacket;
	// (j.jones 2014-08-04 13:56) - PLID 63159 - added IsPhoto variants
	CArray<_variant_t, _variant_t> aryvIsPhoto;
	CArray<int, int> arynOriginalCatID;
	int nOriginalTabCatID;
};

class CHistoryDlg : public CPatientDialog
{
// Construction
public:
	CHistoryDlg(CWnd* pParent);   // standard constructor
	~CHistoryDlg();
	virtual void SetColor(OLE_COLOR nNewColor);
	void ResetColumnSizes();
	void SetColumnSizes();
	void TWAINAcquireFromDevice(const CString& strDeviceName);
	// (a.walling 2008-10-28 13:05) - PLID 31334 - Launched from WIA auto-start
	void WIAAcquireFromDevice(const CString& strDeviceName);
	CString m_strOriginalColSizes;
	CTableChecker m_NoteChecker, m_PatChecker;

// Dialog Data
	//{{AFX_DATA(CHistoryDlg)
	enum { IDD = IDD_HISTORY_DLG };
	CNxIconButton	m_btnEditCategories;
	CNxIconButton	m_btnDetach;
	CNxIconButton	m_btnNew;
	NxButton	m_checkFilterOnEMR;
	NxButton	m_chkRememberColumns;
	NxButton	m_checkShowUnattached;
	NxButton	m_checkFilterOnCategory;
	NxButton	m_checkPhotosSortDescending;
	CNxColor	m_bkg1;
	// (j.jones 2016-04-15 13:35) - NX-100214 - added a second colored background
	CNxColor	m_bkg2;
	CPhotoViewerCtrl	m_PhotoViewer;
	CNxStatic	m_nxstaticPhotosSortByLabel;
	NxButton	m_btnShowEmns;
	CNxIconButton m_btnAcquire;
	CNxIconButton m_btnRecordAudio;
	// (j.jones 2009-10-26 14:40) - PLID 15385 - supported date range
	NxButton	m_radioAllDates;
	NxButton	m_radioDateRange;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	NxButton m_btnShowGridlines; // (z.manning 2010-01-08 10:00) - PLID 12186
	NxButton m_checkPhotosPrintNotes; //(e.lally 2010-01-12) PLID 25808
	CNxIconButton	m_btnDeviceImport;	// (d.lange 2010-10-26 12:20) - PLID 40030 - Added Device Import launch button
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr	m_pDocuments;
	NXDATALISTLib::_DNxDataListPtr m_pFilter;
	// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
	NxTab::_DNxTabPtr m_tab;

	// (c.haag 2010-07-01 16:06) - PLID 39473 - Extra notes icon
	HICON m_hExtraNotesIcon;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHistoryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	// (j.jones 2014-08-04 14:58) - PLID 32474 - added Ex handling
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	void OpenDocumentByID(long nMailID);
	void OpenPacket(long nMergedPacketID);
	bool m_bInPatientsModule;
	// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
	//bool m_bInPICContainer;
	class CPicContainerDlg* GetPicContainer() const
	{
		return m_pPicContainer;
	}

	void SetPicContainer(class CPicContainerDlg* pPicContainer)
	{
		ASSERT(!m_pPicContainer || !pPicContainer);
		m_pPicContainer = pPicContainer;
	}

protected:
	class CPicContainerDlg* m_pPicContainer;

public:

	// (a.walling 2009-05-13 15:42) - PLID 34243 - Allow calling from outside
	void ImportCCDs();

	// (c.haag 2004-11-11 10:36) - PLID 14074 - We need the PIC to maunally update the view
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	// (a.walling 2010-10-14 13:43) - PLID 40978
	virtual void Refresh();

	HistoryDragDocInfo m_DragDocInfo;
	BOOL m_bDragDropNeedToRefresh;

protected:
	long GetPicID();
	long GetActivePersonID();
	// (a.walling 2006-08-14 17:29) - PLID 21970
	CString GetActivePersonName(); // returns the correct name depending on what module we are in.

	// (a.walling 2014-06-09 10:58) - PLID 62355 - Use the appropriate patient ID/name based on whether we are embedded in the PIC or not
	long GetActivePatientID();
	CString GetActivePatientName();

	void EnableAppropriateButtons();

	// (a.wilson 2014-08-08 15:55) - PLID 63246 - function to update note categories.
	//-1 explains that an ID was not passed.
	void EnsureUpdatedCategoryFilter(const long & nID = -1);

	// For detaching files
	void DetachSelectedDocuments(BOOL bDelete);
	// (m.hancock 2006-08-04 16:14) - PLID 21496 - Call this to delete a file and receive any error messages in astrMessages
	// (m.hancock 2006-11-22 09:56) - PLID 21496 - This function will determine if a file can be deleted and return the 
	//  boolean for that value.  If the file cannot be deleted, the errors encountered are returned in astrMessages.
	// (z.manning, 06/01/2007) - PLID 25086 - Renamed the ignore file errors parameter to make it more general.
	BOOL DeleteSingleFile(CString strFullFileName, CString strNote, CStringArray& astrMessages, BOOL bIgnoreFileErrors = FALSE);

	//for loading files that aren't attached
	void LoadUnattachedFiles();
	// (z.manning 2008-11-12 12:06) - PLID 31180 - Added the start tick parameter so we can timeout here
	// in case someone attaches a directory with a massive amount of files (e.g. an entire drive).
	// (j.jones 2014-08-14 17:22) - PLID 63374 - this now loads the patient's file list one time and
	// passes it into recursive calls
	void LoadFilesFromDir(CString strDir, const BOOL bIsSubDirectory, const DWORD dwStartTick, boost::container::flat_set<CiString> &aryPatFiles, bool &bHasLoadedPatFiles);
	// (a.walling 2008-09-15 15:58) - PLID 23904 - Made this more modular by being per-row
	void FillPathAndFile(NXDATALISTLib::IRowSettingsPtr pRow);
	void UpdateListColors();
	CString GetMultipleWhere(CDWordArray& dw);

	// (a.walling 2008-09-15 16:57) - PLID 23904 - Ensure the row has an appropriate icon
	void EnsureRowIcon(NXDATALISTLib::IRowSettingsPtr pRow);

	bool DoesRowMatch(long nMailID, CString strPath, CString strNote, CString strStaff, COleDateTime ServiceDate, long nCatID, CString strFile);

	// (j.gruber 2010-01-22 13:38) - PLID 23693 - added parameter for if its a image
	long CheckAssignCategoryID(BOOL bIsImageFile);

	long m_id;
	CString m_strCurPatientDocumentPath;
	CString GetCurPatientDocumentPath();

	long m_nRowSel;
	long m_nTopRow;	//the top row currently visible
	CDWordArray m_aryFilter;
	long m_nCurrentFilterID;

	CString m_strWhere;
	
	CArray<HistoryTabInfo,HistoryTabInfo&> m_arTabs;
	void RefreshTabs();
	//DRT 6/15/2007 - PLID 25531 - We now use the more descriptive ERefreshTable instead of just "Table"
	void RefreshTable(NetUtils::ERefreshTable table, DWORD id = -1);

	CHistoryTabsDropTarget m_DropTargetTabs;
	// (a.walling 2013-09-03 16:47) - PLID 58039 - Removed the Documents/placeholder drop targets

	bool m_bPhotoViewerDetachable;
	BOOL m_bThemeOpen;	//are we in xp theme mode?

	//If it returns FALSE, the user has asked to stop the import.
	BOOL ImportAndAttachFolder(FileFolder &Folder);

	// (a.walling 2009-05-05 17:04) - PLID 34176 - Generic import and attach function
	// (a.walling 2009-05-07 10:31) - PLID 34179 - Return files, mailIDs
	// (a.walling 2009-05-13 16:41) - PLID 34243 - support switching from other patients
	// (j.jones 2014-08-04 13:33) - PLID 63157 - this now needs patient IDs
	void ImportAndAttach(const CString& strSelectionOverride, const char* szExtensionFilter, CStringArray* psaFiles = NULL, CArray<long, long>* parMailIDs = NULL, CArray<long, long>* parPatientIDs = NULL, long* pnSwitchToPatientID = NULL);

	long AttachFileToHistory(const CString &path, long nPatientID, HWND hwndMessageParent, long nCategory = -1, LPCTSTR strSelection = SELECTION_FILE, LPCTSTR strPDAFilename  = NULL);

	
	NXDATALISTLib::_DNxDataListPtr m_dlPhotosSortCriterion;

	void RememberColumns();

	BOOL m_bDragging;

	// (j.jones 2014-08-04 13:56) - PLID 63159 - added IsPhoto variant
	BOOL ChangeDocumentCategory(long nMailID, CString strFilename, CString strFileNameOnly, long nNewCatID, BOOL bIsPacket, _variant_t varIsPhoto);

	void BeginDocumentsDrag();

	// (j.jones 2007-02-12 13:03) - PLID 24709 - prevents opening a case history twice
	BOOL m_bCaseHistoryIsOpen;

	
	// (a.walling 2010-01-28 14:31) - PLID 28806 - Now passes a connection pointer
	static void WINAPI CALLBACK OnNxTwainPreCompress(const LPBITMAPINFO pDib, BOOL &bAttach, BOOL &bAttachChecksum, long &nChecksum, ADODB::_Connection* lpCon);

	// (a.wetta 2007-07-09 12:47) - PLID 17467 - Updates the photo status of a document
	BOOL SetPhotoStatus(NXDATALISTLib::IRowSettingsPtr pRow, BOOL bIsPhoto);

	// (z.manning 2008-07-01 12:48) - PLID 25574 - We can now preview EMNs from the history tab
	class CEMRPreviewPopupDlg* m_pdlgEmrPreview;
	void EnsureEmrPreviewDialog();
	void OpenEmrPreviewDialog(long nEmnID);

	// (a.walling 2009-05-07 10:34) - PLID 34179 - Update descriptions from CCD
	// (j.jones 2014-08-04 13:33) - PLID 63157 - this now needs patient IDs
	void UpdateCCDInformation(CStringArray& saFiles, CArray<long, long>& arMailIDs, CArray<long, long>& arPatientIDs);

	HICON m_hiconEmn;
	static HICON m_hiconPacket; // (a.walling 2008-09-15 17:29) - PLID 23904

	CString m_strFromClause;

	//DRT 7/2/2008 - PLID 30601 - Split out common code.
	void GetSelectedFiles(CStringArray& aryFileNames, CStringArray& aryFilePaths);
	// (c.haag 2010-10-21 12:36) - PLID 27633 - Returns TRUE if any files IN astrFilePaths could not be found
	BOOL AreAnyFilesMissing(const CStringArray& astrFilePaths);

	void AcquireFromWIA(WIA::IDevicePtr pDevice);
	void AcquireItemFromWIA(CStringArray& saTempFiles, WIA::IItemPtr pItem, WIA::ICommonDialogPtr pCommonDialog);

	// (a.walling 2008-09-09 16:07) - PLID 30389 - For WIA support
	void AttachTempFilesToMailSent(CStringArray& saTempFiles);

	// (a.walling 2008-09-11 16:13) - PLID 31334 - Are we in a modal loop?
	BOOL IsInModalLoop();

	// (a.walling 2008-09-11 10:54) - PLID 31334 - Allow automatic acquisition without prompts
	BOOL m_bWIAAutoAcquire;

	// (c.haag 2008-09-12 17:04) - PLID 31369 - Populates a menu with image acquision opptions. I moved Adam's code here.
	void PopulateAcquireMenu(CMenu& mnu);

	// (a.walling 2013-10-02 10:02) - PLID 58847 - map of icons no longer necessary, FileUtils maintains a cache by extension

	// (z.manning 2009-01-29 11:47) - PLID 32885 - Added a utility function for merging a new document.
	// strTemplatePath is the path that the file browser will default to
	void MergeNewDocument(CString strTemplatePath);

	// (c.haag 2010-06-08 10:12) - PLID 38731 - This function is called either when the user left-clicks 
	// on the document list, or when another event wants to simulate a left-click.
	void HandleDocumentAction(long nRow, short nCol);

	// (c.haag 2010-05-21 13:39) - PLID 38731 - This function returns TRUE if the row can be associated with a
	// todo alarm. (Todo Regarding ID = Document ID)
	BOOL CanCreateTodoForDocument(long nRow);

	// (j.jones 2009-10-30 10:56) - PLID 15384 - used for date filters
	BOOL m_bDateFromDown, m_bDateToDown;

	// (d.lange 2010-10-26 16:54) - PLID 41088 - Receiving function for message NXM_DEVICE_IMPORT_STATUS, update Device Import button
	LRESULT OnUpdateDeviceImportButton(WPARAM wParam, LPARAM lParam);

	// (j.jones 2014-08-04 17:15) - PLID 63159 - helper functions to decide if a file is a photo
	// based on the IsPhoto status and the path name
	bool IsRowAPhoto(long nRow);
	TableCheckerDetailIndex::MailSent_PhotoStatus GetRowPhotoStatus(long nRow);

	// (e.frazier 2016-05-18 16:23) - PLID-34501 - Return the permission object corresponding to the module we are in (Patients or Contacts)
	EBuiltInObjectIDs GetHistoryPermissionObject();

	// (a.walling 2008-07-17 15:12) - PLID 30751 - Added OnParseTranscriptions
	// (a.walling 2008-09-05 12:57) - PLID 31282 - Support PDF scanning
	// (a.walling 2008-09-08 09:41) - PLID 31282 - Also support invoking the scan multi doc dialog
	// (a.walling 2008-09-10 17:36) - PLID 31334 - WIA Event support
	// (a.walling 2008-09-09 16:07) - PLID 30389 - WIA support
	// (a.walling 2008-09-11 10:55) - PLID 31334 - Allow automatic acquisition without prompts
	// (r.galicki 2008-09-26 15:18) - PLID 31407 - OnSelectSource - Select TWAIN source menu function
	// (a.walling 2009-05-05 16:57) - PLID 34176 - Import CCD option
	// (j.gruber 2010-01-05 12:39) - PLID 22958 - change patient
	// Generated message map functions
	//{{AFX_MSG(CHistoryDlg)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnClickNew();
	afx_msg void OnClickMerge();
	virtual BOOL OnInitDialog();
	afx_msg void OnClickAttach();
	afx_msg void OnClickAttachFolder();
	afx_msg void OnClickMergePacket();
	afx_msg void OnClickIUI();
	afx_msg void OnClickMedEval();
	afx_msg void OnNewCaseHistory();
	afx_msg void OnAcquire();
	afx_msg void OnAcquirePDF();
	afx_msg void OnAcquireMultiPDF();
	afx_msg void OnAcquireMultiDoc();
	afx_msg void OnAcquireWIA();
	afx_msg void OnWIAOptionsAutoAcquire();
	afx_msg void OnSelectSource();
	afx_msg void OnEditingFinishedDocuments(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingDocuments(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnNew();
	afx_msg void OnDetach();
	afx_msg void OnDetachDocument();
	afx_msg void OnDetachAndDelete();
	afx_msg void OnCurrentFolder();
	afx_msg void OnEnableUnattached();
	afx_msg void OnRequeryFinishedDocuments(short nFlags);
	afx_msg void OnRButtonDownDocuments(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSetPrimaryPicture();
	afx_msg void OnResetPrimaryPicture();
	afx_msg void OnAttachFile();
	afx_msg void OnEmailFile();
	afx_msg void OnSendFax();
	afx_msg void OnClickImportFromPDA();
	afx_msg void OnDeviceImport();			// (d.lange 2010-06-21 16:52) - PLID 39202 - Added to launch the device importer
	afx_msg void OnLeftClickDocuments(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnClickImportAndAttach();
	afx_msg void OnPrintFile();
	afx_msg void OnDestroy();
	afx_msg void OnSelChangedDocuments(long nNewSel);
	afx_msg void OnTrySetSelFinishedDocuments(long nRowEnum, long nFlags);
	afx_msg void OnSendToServer();
	afx_msg void OnEditHistoryCats();
	afx_msg void OnUseHistoryFilter();
	afx_msg void OnSelChosenHistoryFilterList(long nRow);
	afx_msg void OnSelChangingHistoryFilterList(long FAR* nNewSel);
	afx_msg void OnSelectTabDocTabs(short newTab, short oldTab);
	afx_msg void OnRememberColumns();
	afx_msg void OnColumnSizingFinishedDocuments(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	afx_msg void OnParseTranscriptions();
	afx_msg void OnDirectMessage();// (j.camacho 2013-11-04 13:54) - PLID 59303
	afx_msg void OnImportCCD();
	afx_msg void OnCreateCCD();
	afx_msg void OnCreateCCDA(); // (j.gruber 2013-11-08 08:41) - PLID 59375
	afx_msg void OnCreateCCDACustomized(); // (b.savon 2014-05-01 16:55) - PLID 61909
	// (j.jones 2010-06-30 11:09) - PLID 38031 - renamed to view any generic XML document
	afx_msg void OnViewXML();
	afx_msg void OnValidateCCD();
	afx_msg LRESULT OnHistoryTabChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDropFiles(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPhotoViewerDetachable(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDblClickCellDocuments(long nRowIndex, short nColIndex);
	afx_msg void OnSelChosenPhotosSortCriterionCombo(long nRow);
	afx_msg void OnPhotosSortDescendingCheck();
	afx_msg void OnCheckFilterOnEmr();
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	afx_msg void OnDragBeginDocuments(BOOL FAR* pbShowDrag, long nRow, short nCol, long nFlags);
	afx_msg void OnTogglePhotoStatus();
	afx_msg void OnSize(UINT nType, int cx, int cy); // (z.manning, 09/18/2007) - PLID 27425
	afx_msg void OnShowEmns();
	afx_msg void OnEditingStartingDocuments(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	// (a.walling 2008-09-10 13:31) - PLID 31334
	afx_msg LRESULT OnWIAEvent(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBtnAcquire();
	afx_msg void OnRecordAudio();
	afx_msg void OnMergeEmrStdDocument();
	afx_msg void OnMergeEmrStdPacket();
	// (j.jones 2009-10-26 17:24) - PLID 15385 - added date filter
	afx_msg void OnBnClickedRadioHistoryAllDates();
	afx_msg void OnBnClickedRadioHistoryDateRange();
	afx_msg void OnChangeHistoryFromDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnChangeHistoryToDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDropdownHistoryFromDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDropdownHistoryToDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnCloseupHistoryFromDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnCloseupHistoryToDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnChangePatient();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedHistoryShowGridlines();
	afx_msg void OnBnClickedPrintPhotoNotes();
	afx_msg void OnAttachToCurrentPic(); // (z.manning 2010-02-02 15:42) - PLID 34287
	afx_msg void OnNewTodo();
	afx_msg LRESULT OnOpenDocument(WPARAM wParam, LPARAM lParam);
public:
// (s.dhole 2013-11-01 12:15) - PLID 59278
	afx_msg void OnReconcileMedication();
	afx_msg void OnReconcileAllergy();
	afx_msg void OnReconcileProblem();
	BOOL IsValidCCDADocumentImport(CString strFileFullPath);
	CString  GetFileFullPath();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HISTORYDLG_H__DC332784_2AD0_11D2_B1FA_0000C0832801__INCLUDED_)
