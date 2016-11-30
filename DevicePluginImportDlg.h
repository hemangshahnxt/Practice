#pragma once
#include "DevicePluginUtils.h"

// (j.jones 2010-12-29 17:42) - PLID 41796 - PDF Creator Pilot is used to convert PDFs to images
// (a.walling 2010-12-30 15:01) - PLID 41796 - Here's to the pilot that weathered the storm. 
#import "PDFCreatorPilot.tlb"
//(e.lally 2011-05-24) PLID 42819
#import "NexTech.COM.tlb"
#import <bepconv.tlb>

// (r.gonet 06/11/2013) - PLID 56370 - Added some forward declarations to get rid of an include of a header file.
enum EConvertPDFToImage;
enum EDevicePatientMatchRule;

// (b.savon 2014-12-03 10:41) - PLID 64186 - Likewise
enum EPDFToImageConverter;

// (d.lange 2010-05-24 15:34) - PLID 38850 - Created
// CDevicePluginImportDlg dialog

enum DataListColumn {
	dlcChecked = 0,
	dlcPatientPointer,
	dlcPatientID,
	dlcUserDefinedID,
	// (j.jones 2011-03-10 16:08) - PLID 42329 - added file time
	dlcFileTime,
	dlcPatientName,
	dlcType,
	dlcFilePath,
	// (j.jones 2010-10-25 13:35) - PLID 41008 - added a column just to search on
	dlcFilePathSearchable,
	// (j.jones 2010-12-14 16:50) - PLID 41598 - clarified these column names,
	// first one is the internal file description (may be blank), and the second
	// column is what we display in the import screen only
	dlcFileDescriptionToUse,
	// (j.jones 2011-08-25 15:15) - PLID 41683 - renamed to plugin name
	dlcPluginName,
	dlcCategory,
	// (j.jones 2010-11-02 16:06) - PLID 41189 - added DevicePatientMatchRule
	dlcDevicePatientMatchRule,
	// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
	dlcConvertPDFToImage,
	// (r.gonet 06/10/2013) - PLID 56370 - added the Image DPI column
	dlcImageDPI,
	// (d.lange 2011-05-19 17:26) - PLID 43253 - added a column to store the ToDo alarm ID
	dlcToDoAlarmID,
	// (d.lange 2011-05-05 15:18) - PLID 43253 - added Create Todo alarm
	dlcCreateToDoAlarm,
	//(e.lally 2011-04-21) PLID 43372 - added PluginFileName
	dlcDeviceFileName,
	// (b.savon 2014-12-03 13:37) - PLID 64186 - Added converter
	dlcPDFToImageConverter,
};

enum PatientDropdownColumn {
	pdcLast = 0,
	pdcMiddle,
	pdcFirst,
	pdcPatientID,
};

enum ReturnedFileType {
	rftJpeg = 0,
	rftTable,
};

// (j.jones 2011-03-11 12:59) - PLID 42328 - added an enum for sort order,
// these are stored in data (as a preference), so don't change them
enum EDeviceImportSortOrder {
	disoDateAsc = 1,
	discDateDesc = 2,
	discPatientNameAsc = 3,
};

struct ImportFile {
	long nPatientID;
	CString strFilePath;
	long nFileCategory;
	// (j.jones 2010-11-02 10:15) - PLID 41188 - added device name
	CString strDeviceName;
	// (j.jones 2010-12-15 09:21) - PLID 41598 - added file description, could be blank
	CString strFileDescription;
	// (j.jones 2010-12-28 17:29) - PLID 41796 - added ConvertPDFToImage
	EConvertPDFToImage eConvertPDFToImage;
	// (r.gonet 06/10/2013) - PLID 56370 - added Image DPI
	long nImageDPI;
	// (j.jones 2011-03-10 16:08) - PLID 42329 - added file time
	COleDateTime dtFileTime;
	// (d.lange 2011-05-20 11:28) - PLID 43253 - added ToDo Task ID
	long nToDoTaskID;
	// (b.savon 2014-12-03 13:37) - PLID 64186 - Added converter
	EPDFToImageConverter eConverter;
};

// (d.lange 2011-03-11 09:22) - PLID 42754 - Created this enum for the row status colors
enum ERowStatusColor {
	rscUnselectedPatient = RGB(255,127,127),
	rscMatchedPatient = RGB(255,242,0),
	rscSelectedPatient = RGB(128,255,128),
};

// (d.lange 2011-03-10 15:03) - PLID 42754 - Structs for holding PatientElements*, needed for implementing filters
struct ChildRowRecord {
	
	// (r.gonet 06/11/2013) - PLID 56370 - Moved implementation to the cpp file.
	ChildRowRecord();

	// (r.gonet 06/11/2013) - PLID 56370 - Moved the implementation to the cpp file.
	// (b.savon 2014-12-03 13:37) - PLID 64186 - Added converter
	ChildRowRecord(DevicePluginUtils::RecordElement *reRecordElement, EDevicePatientMatchRule eDevicePatientMatchRule, EConvertPDFToImage eConvertPDFToImage, long nImageDPI, long nParentPointer, EPDFToImageConverter converter);

	BOOL bTableChecked;
	BOOL bFileChecked;
	long nParentPointer;
	long nCategoryID;
	CString strFullFilePath;
	CString strFileDescriptionOverride;
	EDevicePatientMatchRule eDevicePatientMatchRule;
	EConvertPDFToImage eConvertPDFToImage;
	// (r.gonet 06/10/2013) - PLID 56370 - Added Image DPI
	long nImageDPI;
	DevicePluginUtils::RecordElement *reRecordElement;
	// (j.jones 2011-03-10 16:13) - PLID 42329 - added dtFileTime
	COleDateTime dtFileTime;
	// (d.lange 2011-05-20 11:11) - PLID 43253 - Added ToDo Task ID
	long nToDoTaskID;
	// (b.savon 2014-12-03 13:37) - PLID 64186 - Added converter
	EPDFToImageConverter eConverter;
};

struct ParentRowRecord {
	// (r.gonet 06/11/2013) - PLID 56370 - Moved the implementation to the cpp file.
	ParentRowRecord();

	// (r.gonet 06/11/2013) - PLID 56370 - Moved the implementation to the cpp file.
	// (b.savon 2014-12-03 13:37) - PLID 64186 - Added converter
	ParentRowRecord(DevicePluginUtils::PatientElement *pePatientElement, const CString& strPluginName, const CString& strPluginFileName, EDevicePatientMatchRule eDevicePatientMatchRule, EConvertPDFToImage eConvertPDFToImage, long nImageDPI, EPDFToImageConverter eConverter);

	DevicePluginUtils::PatientElement *pePatientElement;
	CString strPluginName;
	CString strPluginFileName; //(e.lally 2011-04-21) PLID 43372
	EDevicePatientMatchRule eDevicePatientMatchRule;
	EConvertPDFToImage eConvertPDFToImage;
	// (r.gonet 06/10/2013) - PLID 56370 - Added Image DPI
	long nImageDPI;
	ERowStatusColor eRowStatusColor;
	BOOL bChecked;
	long nPatientID;
	long nPatientUserDefinedID;
	// (j.jones 2011-03-10 16:13) - PLID 42329 - added dtFileTime
	COleDateTime dtFileTime;
	// (b.savon 2014-12-03 13:37) - PLID 64186 - Added converter
	EPDFToImageConverter eConverter;

	CArray<ChildRowRecord*, ChildRowRecord*> aryChildRowRecords;
};

// (b.savon 2012-03-26 17:18) - PLID 49143 - Minimize Preference
enum EMinimizeUIPreference{
	muipAlways = 0,
	muipEmptyList,
	muipNever,
};

// (b.savon 2012-04-06 12:53) - PLID 41861 - We now need an array of ParentRowRecord pointers
struct ArrayParentRowPointers{
	CArray<long, long> aryParentRecordPointers;
};

// (j.armen 2012-06-06 12:39) - PLID 50830 - Removed GetMinMaxInfo
class CDevicePluginImportDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CDevicePluginImportDlg)

public:
	CDevicePluginImportDlg(CWnd* pParent);   // standard constructor
	virtual ~CDevicePluginImportDlg();
	// (d.lange 2011-06-01 17:50) - PLID 43253 - Listen for a todo table checker and update the datalist if the todo task was removed
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);

	// (s.tullis 2014-09-15 14:16) - PLID 63225 - DevicePluginImportDlg needs revisited to see if the Todo response can be removed. It shouldn't need to respond immediately to these.
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);

	void ReflectChangedTodo(CTableCheckerDetails* pDetails);

	// (j.jones 2010-06-17 13:55) - PLID 37976 - Called by MainFrame to add new PatientElements
	// returned by the plugins into this importer screen. Return TRUE if we took ownership of
	// the PatientElement (and plan to delete it), return FALSE if it is a duplicate and MainFrame
	// will then be responsible for deleting.
	// (j.jones 2010-11-02 16:02) - PLID 41189 - added DevicePatientMatchRule
	// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
	//(e.lally 2011-04-21) PLID 43372 added strPluginFileName
	// (b.savon 2011-09-23 11:48) - PLID 44954 - Added pCon so that this method can be thread safe while processing plugin files
	// (r.gonet 06/11/2013) - PLID 56370 - Added Image DPI
	// (b.savon 2014-12-03 13:37) - PLID 64186 - Added converter
	BOOL AddNewPatientElement(DevicePluginUtils::PatientElement *pNewPatientElement, CString strPluginName, CString strPluginFileName, EDevicePatientMatchRule eDevicePatientMatchRule, EConvertPDFToImage eConvertPDFToImage, long nImageDPI, EPDFToImageConverter eConverter, ADODB::_ConnectionPtr pCon = NULL);

	// (j.jones 2010-10-25 13:28) - PLID 41008 - called by MainFrame to remove rows for files
	// we have been told were deleted (returns TRUE if something was removed from the screen)
	BOOL HandleDeletedFile(CString strDeletedFilePath);

	// (b.savon 2011-9-2) - PLID 45326 - Handle support for deleting folders for recursive plugins
	BOOL HandleDeletedFolder(CString strDeletedFolderPath);
	BOOL RemoveRowFromDlg(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (b.savon 2011-9-8) - PLID 45314 - Provide an ability to filter the device integration list per-device plugin setup.
	void BuildDevicePluginsCombo();

	// (b.savon 2012-02-13 11:52) - PLID 46456 - Handle network disconnect
	BOOL IsFileListUpToDate();
	void RescanDevicePluginFolders();
	void Cleanup();

	// (b.savon 2012-02-20 15:07) - PLID 46455 - Window resize / DL resize
	void UpdateFileListWidths();

	// (d.lange 2011-03-10 15:04) - PLID 41010 - When the current patient filter is enabled, check m_aryParentRowRecord for any records for any currently
	// opened EMNs or the active patient
	// (b.savon 2011-09-23 11:48) - PLID 44954 - Added pCon so that this method can be thread safe while processing plugin files
	BOOL IsCurrentPatient(long nPatientUserDefinedID, CArray<long,long> &aryOpenEMRPatientIDs, ADODB::_ConnectionPtr pCon = NULL);

	// (d.lange 2010-10-26 16:28) - PLID 41088 - Returns the status of the datalist (Empty or filled)
	// (d.lange 2011-03-11 09:58) - PLID 42754 - Moved this to be a public function
	// (j.jones 2011-03-15 13:54) - PLID 42738 - this now returns a count of records
	long GetDeviceImportRecordCount();

	// (j.jones 2011-03-11 14:41) - PLID 42328 - called when preferences have changed
	void OnPreferencesChanged();

	// (j.jones 2011-08-25 14:59) - PLID 41683 - clear all records for a given plugin
	void RemoveRecordsForPlugin(CString strPluginName);

// Dialog Data
	enum { IDD = IDD_DEVICES_IMPORT_DLG };

	CNxIconButton m_btnImportHistory;
	CNxIconButton m_btnImportEMR;
	CNxIconButton m_btnClose;
	CNxEdit m_nxeditFirst;
	CNxEdit m_nxeditMiddle;
	CNxEdit m_nxeditLast;
	CNxEdit m_nxeditBirthDate;
	CNxEdit m_nxeditGender;
	CNxEdit m_nxeditSocial;
	// (d.lange 2010-10-20 15:37) - PLID 41006 - Always delete non-imported files checkbox
	NxButton m_checkAlwaysDelete;
	// (d.lange 2011-03-09 15:27) - PLID 41010 - Checkbox to filter files on the current patient
	NxButton m_chkCurPatientFilter;
	CNxIconButton m_btnCombinePdfFiles;
	NxButton m_btnTogglePreview;

	// (b.savon 2012-03-12 14:42) - PLID 48816 - Manual Refresh button
	CNxIconButton m_btnRefresh;

	HICON m_hIconPreview;

protected:
	// (d.lange 2010-10-25 12:05) - PLID 41088 - The datalist needs to be public in order to check the status (empty/filled)
	// (d.lange 2011-03-11 10:01) - PLID 42754 - This should not be available to anything outside this class
	NXDATALIST2Lib::_DNxDataListPtr m_pList;

	NXDATALIST2Lib::_DNxDataListPtr m_pPatientCombo;

	// (b.savon 2011-9-8) - PLID 45314 - Provide an ability to filter the device integration list per-device plugin setup.
	NXDATALIST2Lib::_DNxDataListPtr m_pDetectedDevicePluginCombo;
	CString m_strSelectedPlugin;

	// (b.savon 2012-02-01 12:56) - PLID 47651 - Save current row selection
	CString m_strSelectedFilePath;

	// (b.savon 2012-02-20 14:58) - PLID 46455 - Used for window / DL resize
	long m_nOriginalListWidth;
	long m_nOriginalColumnWidth;

	// (b.savon 2012-04-06 12:54) - PLID 41861 - Patient mapped to parent rows
	// (z.manning 2016-04-06 09:38) - NX-100116 - Got rid of this map as storing pointers to rows that can
	// be added/removed frequently was causing problems.
	//CMap<long, long, NXDATALIST2Lib::IRowSettingsPtr, NXDATALIST2Lib::IRowSettingsPtr> m_mapPatientMatched;
	// Method to parse | delimited parent file names.
	int ParseParentRowFilePath(CString strParentRowFilePath, CArray<CString, CString>& aryParentRowFilePaths);
	// Array to hold all of our arrays of pointers.
	CArray<ArrayParentRowPointers*, ArrayParentRowPointers*> m_aryParentRowPointers;

	// (b.savon 2012-07-02 14:24) - PLID 41861 - Get the first preview-able file from the pipe delimited file name
	CString GetFirstValidPreviewFile(CString strFileName);

	// (b.savon 2012-04-06 17:06) - PLID 49506 - Find and Fill utility
	void FindAndFillPatient(ParentRowRecord* prParentRecord,  ADODB::_ConnectionPtr pCon = NULL);

	//TES 4/21/2011 - PLID 43252 - Preview attached files
	IWebBrowser2Ptr m_pBrowser;	
	//TES 4/21/2011 - PLID 43252 - Reflect the currently selected file in the preview window
	void UpdatePreview();
	//TES 4/21/2011 - PLID 43252 - Track which file we're previewing, no need to update the screen (which causes some
	// flickering) if we're already previewing the correct file
	CString m_strCurrentPreviewFile;

	//TES 4/22/2011 - PLID 43252 - The browser can lock files, so call this function before doing anything that might remove files
	// from the import folder.
	void ClearPreview();

	// (j.jones 2010-12-30 09:18) - PLID 41796 - tracks the loaded PDF converter dll
	PDFCreatorPilotLib::IPDFDocument4Ptr m_pPDF;
	EasyPDFConverter::IPDFConverterPtr m_pBCLPDF;
	// (j.jones 2010-12-30 09:18) - PLID 41796 - tracks whether we tried to load the PDF dll
	BOOL m_bHasLoadedPDF;	
	// (j.jones 2010-12-30 09:18) - PLID 41796 - loads the PDF dll if not already loaded,
	// warns one time only if it failed, tells future callers if it is loaded
	BOOL IsPDFConverterLoaded();

	// (d.lange 2011-03-09 09:36) - PLID 42754 - stores all patient objects that are waiting to be imported or removed
	CArray<ParentRowRecord*, ParentRowRecord*> m_aryParentRowRecord;

	// (j.jones 2010-06-22 09:00) - PLID 38903 - supported previewing a table
	void PreviewTable(DevicePluginUtils::TableContent *pTableContent);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	// (j.jones 2010-11-02 11:06) - PLID 41189 - Renamed to FindPatient, which may
	// end up matching by demographics instead of ID, based on eDevicePatientMatchRule.
	// nOverrideUserDefinedID is used if the row has already been linked to a patient.
	// (b.savon 2011-09-23 11:48) - PLID 44954 - Added pCon so that this method can be thread safe while processing plugin files
	long FindPatient(DevicePluginUtils::PatientElement *pePatientResult, EDevicePatientMatchRule eDevicePatientMatchRule, long nOverrideUserDefinedID, ADODB::_ConnectionPtr pCon = NULL);
	// (j.jones 2010-11-02 16:02) - PLID 41189 - added DevicePatientMatchRule
	// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
	// (d.lange 2011-03-09 16:16) - PLID 42754 - All parameters are now contained in the ParentRowRecord struct
	// (b.savon 2011-09-23 11:48) - PLID 44954 - Added pCon so that this method can be thread safe while processing plugin files
	void AddParentRow(ParentRowRecord *prPatientRecord, ADODB::_ConnectionPtr pCon = NULL);
	// (j.jones 2010-11-02 16:06) - PLID 41189 - added DevicePatientMatchRule
	// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
	// (d.lange 2011-03-09 16:16) - PLID 42754 - All RecordElements are now contained in an array of ChildRowRecords
	void AddChildFile(NXDATALIST2Lib::IRowSettingsPtr pParentRow, CArray<ChildRowRecord*, ChildRowRecord*> &aryChildRowRecords);
	// (d.lange 2011-05-25 12:51) - PLID 43253 - Added a boolean for identifying whether its a manual deletion or the files have been imported
	void RemoveChildRowFiles(NXDATALIST2Lib::IRowSettingsPtr pParentRow, BOOL bManualRemove = FALSE);
	BOOL ComparePatientElement(DevicePluginUtils::PatientElement* peParent);
	// (d.lange 2010-11-19 16:22) - 
	BOOL CheckChildRowsSelected(NXDATALIST2Lib::IRowSettingsPtr pParentRow = NULL);
	BOOL ValidateCategories(CArray<ImportFile*, ImportFile*> &aryFilesToImport);
	// (d.lange 2010-10-20 15:37) - PLID 41006 - This function will only remove the given row and associated file
	void RemoveRowFile(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bLastChildRow);
	// (d.lange 2010-10-26 16:36) - PLID 41088 - Sends the Device Import status to open PICs and History tab to update the Import button
	void SendMessageToUpdateButtons();

	// (j.jones 2010-12-30 12:06) - PLID 41796 - imports the given file to history and optionally EMR
	// (j.jones 2011-03-10 16:26) - PLID 42329 - added dtServiceDate
	// (d.lange 2011-05-23 14:26) - PLID 43253 - added todo task ID
	// (b.spivey, October 30, 2015) PLID 67423 
	BOOL ImportSingleFile(CString strFilePath, long nPatientID, long nCategoryID, CString strFileDescriptionForHistory,
							BOOL bImportToEMR, CString strDeviceName, CPicContainerDlg *pPicContainer, long nPicID,
							COleDateTime dtServiceDate, long nToDoTaskID, OUT long* pnMailSentID = NULL);
	// (j.jones 2010-12-30 12:06) - PLID 41796 - converts the PDF file to images, importing those images into history and optionally EMR
	// (j.jones 2011-03-10 16:26) - PLID 42329 - added dtServiceDate
	// (d.lange 2011-05-23 14:29) - PLID 43253 - added todo task ID
	// (r.gonet 06/11/2013) - PLID 56370 - Added Image DPI
	// (b.savon 2014-12-03 10:43) - PLID 64186 - Added converter
	BOOL ConvertAndImportPDF(CString strPDFFilePath, long nImageDPI, long nPatientID, long nCategoryID, CString strFileDescriptionForHistory,
							BOOL bImportToEMR, CString strDeviceName, CPicContainerDlg *pPicContainer, long nPicID,
							COleDateTime dtServiceDate, long nToDoTaskID, EPDFToImageConverter converter);

	// (d.lange 2011-03-09 19:44) - PLID 41010 - Loads the objects from m_aryParentRowRecord based on the current patient filter
	// (b.savon 2011-9-8) - PLID 45314 - Provide an ability to filter the device integration list per-device plugin setup.
	// (b.savon 2011-9-26) - PLID 42330 - added Hardload all so that when we modify a plugin we can load/remove necessary entries	
	void LoadData( CString strCurrentPluginFilter, BOOL bHardLoadAll = FALSE );
	// (d.lange 2011-03-10 15:10) - PLID 42754 - Destroys a single ParentRowRecord*
	void DeleteParentRowRecord(ParentRowRecord *prParentRowRecord);
	void DeleteChildRowRecord(ChildRowRecord *crChildRowRecord);

	// (j.jones 2011-03-11 12:59) - PLID 42328 - cache the preference for sort order
	EDeviceImportSortOrder m_eSortOrder;

	// (j.jones 2011-03-11 14:48) - PLID 42328 - will apply m_eSortOrder to the datalist,
	// maintaining current rows if necessary
	void ApplySortOrder();

	// (j.jones 2011-03-11 15:21) - PLID 42328 - if we need to force a datalist sort,
	// this function will do so while trying to maintain the current row
	// (b.savon 2012-04-09 15:59) - PLID 41861 - Respect the new DL structure and attempt to maintain current row with pointer to array
	void SortList(/*OPTIONAL IN ParentRowRecord *prRecordToReselect = NULL*/ OPTIONAL IN ArrayParentRowPointers* aryPaRow = NULL);

	// (j.jones 2011-03-11 14:46) - PLID 42328 - bulk cache preferences in one function,
	// as it can be called multiple times
	void CachePreferences();

	//TES 4/28/2011 - PLID 43252 - A local implementation, that accounts for the fact that the preview window sometimes holds a lock
	// on the file it was previewing for a few seconds after being cleared.
	void DeleteFile(const CString &strFile);

	// (d.lange 2011-05-20 11:38) - PLID 43253 - Once a file is imported and there was a Todo created, this function will attach the imported 
	// document to the Todo
	void UpdateTodoRegardingIDs(long nTaskID, long nRegardingID, long nRegardingType, long nPersonID);

	// (a.walling 2013-10-02 10:02) - PLID 58847 - map of icons no longer necessary, FileUtils maintains a cache by extension

	// (b.savon 2012-03-24 12:55) - PLID 49143 - Handle the Minimize preference
	void HandleWindowAfterImport();
	EMinimizeUIPreference m_muipMinimizePreference;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBtnImportToHistory();
	afx_msg void OnBnClickedBtnImportToEmr();
	afx_msg void LeftClickImportList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedDeviceClose();
	afx_msg void EditingFinishedImportList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnGoToPatient();
	afx_msg void OnRemove();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	DECLARE_EVENTSINK_MAP()
	void RButtonUpImportList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedChkRemovefiles();
	afx_msg void OnBnClickedChkCurrentPatientFilter();	
	afx_msg void OnBnClickedDeviceCombineFilesBtn();
	//(e.lally 2011-04-06) PLID 42734
	afx_msg void OnUnselectPatient();
	afx_msg void OnSelChangedImportList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnToggleFilePreview();
	void OnFileDownloadFilePreview(BOOL ActiveDocument, BOOL* Cancel);
	void OnTimer(UINT_PTR nIDEvent);
	//(e.lally 2011-05-24) PLID 42819
	afx_msg void OnSplitPdf();
public:
	//(e.lally 2011-04-21) PLID 43372
	void ShowCombinePdfsButton();
	void Refresh();
	// (b.savon 2011-9-8) - PLID 45314 - Provide an ability to filter the device integration list per-device plugin setup.
	void SelChosenDevicePluginSelect(LPDISPATCH lpRow);
	// (b.savon 2011-09-26 14:39) - PLID 42330 - Apply the Hide EMR Table filter.
	afx_msg void OnBnClickedChkHideEmrTables();
	afx_msg void OnBnClickedBtnRefreshDi();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedRadioAlways();
	afx_msg void OnBnClickedRadioNever();
	afx_msg void OnBnClickedRadioEmptyList();
	afx_msg void OnDestroy();
};
