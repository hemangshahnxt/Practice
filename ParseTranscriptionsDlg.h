#if !defined(AFX_PARSETRANSCRIPTIONSDLG_H__0EF209CD_28D2_464B_A25E_74C247FB817C__INCLUDED_)
#define AFX_PARSETRANSCRIPTIONSDLG_H__0EF209CD_28D2_464B_A25E_74C247FB817C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ParseTranscriptionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CParseTranscriptionsDlg dialog

#include "GlobalStringUtils.h"
#include "ProgressDialog.h"

class CGenericWordProcessorApp;

class CParseTranscriptionsDlg : public CNxDialog
{
// Construction
public:

	enum EParsePatientListColumns {
		ecPersonID = 0,
		// ecFullName, // (a.walling 2010-06-01 08:31) - PLID 38917 - No more FullName column
		ecLast,
		ecFirst,
		ecMiddle,
		ecUserDefinedID,
		ecColor
	};

	// (j.jones 2013-02-27 16:35) - PLID 55119 - added error columns, and error icon
	enum EParseListColumns {
		ecID = 0,
		ecParentID,
		ecIcon,
		ecObject,
		ecCommit,
		ecFilePath,		// (j.jones 2013-03-04 10:06) - PLID 55120 - we now track the file path
		ecName,
		ecNameError,
		ecCategory,
		ecCategoryError,
		ecProvider,		// (a.wilson 2012-08-01 14:19) - PLID 51901 - add provider column.
		ecProviderError,
		ecLabel,
		ecErrorIcon,
	};
	// (a.wilson 2012-08-01 17:01) - PLID 51901 - enumeration for the provider combo.
	enum EParseProviderListColumns {
		epID = 0,
		epName = 1,
		epFirst = 2,
		epLast = 3,
		epMiddle = 4,
	};
	// (a.wilson 2012-08-01 14:01) - PLID 51901 - add provider for emr imports.
	struct CTxSeg {
		CTxSeg() {
			nUserDefinedID = LONG_MIN;
			nBeginCharIndex = -1;
			nEndCharIndex = -1;
			nCategoryID = -3;
			nPatientID = -1;
			nProviderID = -1;

			nLineCount = 0;

			bReviewed = FALSE;

			dtDate.SetStatus(COleDateTime::null);

			m_pstrOverrideText = NULL;
		};

		~CTxSeg() {
			if (m_pstrOverrideText) {
				delete m_pstrOverrideText;
			}
			m_pstrOverrideText = NULL;
		};

		long nUserDefinedID;
		long nBeginCharIndex;
		long nEndCharIndex;
		long nLineCount;
		//CStringArray saText;

		COleDateTime dtDate;

		CString strTitle;
		CString strCat;
		CString* m_pstrOverrideText;
		long nCategoryID; // -3 = Unknown, -2 = Default, -1 = None
		long nPatientID;
		long nProviderID;
		CString strProvider;

		BOOL bReviewed;
	};

	struct CTxDoc {
		CTxDoc() {
			psaText = NULL;
			nID = 0;
		};

		~CTxDoc() {
			Clear();
		};

		CString GetText() {
			ASSERT(psaText);

			if (!strCachedText.IsEmpty())
				return strCachedText;

			if(psaText) {
				for (int i = 0; i < psaText->GetSize(); i++) {
					CString strLine = psaText->GetAt(i);
					strLine += "\r\n";
					GrowFastConcat(strCachedText, strLine.GetLength(), strLine);
				}
			}

			return strCachedText;
		};

		void Clear() {
			delete psaText;
			psaText = NULL;
			strCachedText.Empty();

			ClearSegments();
		};

		void ClearSegments() {
			for (int i = 0; i < arSegs.GetSize(); i++) {
				delete arSegs[i];
			}
			arSegs.RemoveAll();
		};

		CString strCachedText;
		CString strPath;
		CStringArray* psaText;
		CArray<CTxSeg*, CTxSeg*> arSegs;

		long nID;
	};

	struct CParseOptions {
		CParseOptions() {
			bBreakOnPatID = TRUE;
			bCreateWordDocs = FALSE;
			bImportIntoEMR = FALSE;
			nTemplateID = -1;
			bIgnoreSegmentReview = FALSE;
			// (j.jones 2013-02-28 13:05) - PLID 55123 - added ability to permit inactive providers
			bIncludeInactiveProviders = FALSE;
			// (j.jones 2013-02-28 15:53) - PLID 55121 - added ability to force errors to be fixed before they can be imported
			bRequireErrorsFixed = FALSE;
			// (j.jones 2013-03-01 10:55) - PLID 55122 - added a default EMN status
			nEMNStatusID = 0; //defaults to Open
			// (j.jones 2013-03-01 12:56) - PLID 55120 - added ability to move imported files to a folder
			bMoveImportedFiles = FALSE;
			strPostImportFolder = "";
		}

		void Load();
		void Save();
		void ResetToDefaults();

		CString strPatID;
		CString strDate;
		CString strBreak;
		CString strTitle;
		CString strCat;
		CString strProvider; // (a.wilson 2012-08-01 10:17) - PLID 51901 - add for parsing provider for EMR.

		// (j.jones 2013-02-28 13:05) - PLID 55123 - added ability to permit inactive providers
		BOOL bIncludeInactiveProviders;
		BOOL bBreakOnPatID;
		BOOL bCreateWordDocs; // (a.walling 2008-07-17 16:31) - PLID 30774 - Option to create word documents
		BOOL bImportIntoEMR;  // (a.wilson 2012-07-24 12:09) - PLID 51753 - Option to import into EMR
		BOOL bIgnoreSegmentReview; // (a.wilson 2012-10-12 16:57) - PLID 53126 - option to ignore segment review.
		// (j.jones 2013-02-28 15:53) - PLID 55121 - added ability to force errors to be fixed before they can be imported
		BOOL bRequireErrorsFixed;

		// (j.jones 2013-03-01 12:56) - PLID 55120 - added ability to move imported files to a folder
		BOOL bMoveImportedFiles;
		CString strPostImportFolder;

		long nTemplateID;     // (a.wilson 2012-07-24 12:09) - PLID 51753 - variable to store the template id.
		// (j.jones 2013-03-01 10:55) - PLID 55122 - added a default EMN status
		long nEMNStatusID;
	};

	CParseTranscriptionsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CParseTranscriptionsDlg)
	enum { IDD = IDD_PARSE_TRANSCRIPTIONS_DLG };
	CNxStatic	m_lblDefCat;
	CNxStatic	m_lblTitle;
	CNxStatic	m_lblPatient;
	CNxStatic	m_lblDate;
	CNxStatic	m_lblCat;
	CNxStatic	m_lblDrop;
	CNxEdit	m_editTitle;
	CNxIconButton	m_nxibClose;
	CNxIconButton	m_nxibSetToSelection;
	CNxIconButton	m_nxibEditText;
	CNxIconButton	m_nxibParseAll;
	CNxIconButton	m_nxibAddFile;
	CNxIconButton	m_nxibImport;
	CNxIconButton	m_nxibConfigure;
	NxButton	m_nxbReviewed;
	NxButton	m_nxbViewWholeDoc;
	CNxColor	m_nxcolor;
	CEdit	m_text; // purposefully NOT an NxEdit
	CNxColor	m_nxcolorSegment;
	// (j.jones 2013-02-27 16:18) - PLID 55117 - added an error count label
	CNxStatic	m_labelErrorCount;
	// (j.jones 2013-02-28 12:19) - PLID 55118 - added button to go to next error
	CNxIconButton	m_btnGoToNextError;
	// (j.jones 2013-03-01 09:38) - PLID 55122 - added EMN statuses
	CNxStatic	m_nxstaticEMNStatusLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParseTranscriptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	static void CleanOrdinalsFromLongDateString(CString& str);

	void DisplaySegment(CTxSeg* pSeg, CTxDoc* pDoc, long nScrollToLine = -1);

	// (j.jones 2013-04-08 16:32) - PLID 56149 - changed the progress to CProgressDialog
	BOOL SaveSegmentToHistory(CTxDoc* pDoc, CTxSeg* pSeg, CProgressDialog &progressDialog, std::shared_ptr<CGenericWordProcessorApp> pApp);
	long m_nDefaultCategoryID;

	BOOL m_bCheckedForWord;
	BOOL m_bSettingEditText;

	BOOL m_bViewWholeDocument;

	BOOL m_bEditSelectionMode;

	NXDATALIST2Lib::_DNxDataListPtr m_dl;
	NXDATALIST2Lib::_DNxDataListPtr m_dlPat;
	NXDATALIST2Lib::_DNxDataListPtr m_dlCat;
	NXDATALIST2Lib::_DNxDataListPtr m_dlDefCat;
	NXDATALIST2Lib::_DNxDataListPtr m_dlProvider;	// (a.wilson 2012-08-01 14:23) - PLID 51901
	// (j.jones 2013-03-01 09:38) - PLID 55122 - added EMN statuses
	NXDATALIST2Lib::_DNxDataListPtr m_EMNStatusCombo;
	NXTIMELib::_DNxTimePtr m_nxTime;

	long m_nDocumentID;
	void AddDocumentRow(CString strFilePath);
	long RefreshSegments();
	
	CParseOptions m_opt;

	// (j.jones 2013-04-08 16:32) - PLID 56149 - changed the progress to CProgressDialog
	// (z.manning 2016-06-02 16:10) - NX-100790 - Added output param indicating if the action was canceled
	BOOL GetDocumentText(const CString& strPath, OUT CStringArray& saText, IN OUT CString& strErrors, CProgressDialog &progressDialog
		, long nCurrent, long nTotal, std::shared_ptr<CGenericWordProcessorApp> pApp, OUT BOOL &bWasCanceled);

	// (a.walling 2008-07-14 15:41) - PLID 30720 - Configurable parsing for transcriptions
	// (j.jones 2013-04-08 16:32) - PLID 56149 - changed the progress to CProgressDialog
	BOOL ParseText(CTxDoc& doc, CParseOptions& opt, CProgressDialog &progressDialog, long nCurrent, long nTotal, std::shared_ptr<CGenericWordProcessorApp> pApp);
	static long m_cnProgressResolution;

	// (a.walling 2008-07-17 16:40) - PLID 30774
	BOOL EnsureWordApp(const CString &strAfter);

	// (c.haag 2016-05-10 13:27) - NX-100318 - We no longer maintain a persistent Word application in this object
	CArray<HICON, HICON> m_arIcons;
	HICON m_hSubIcon;

	// (j.jones 2013-02-27 16:35) - PLID 55119 - added error icon
	HICON m_hIconRedX;
	
	// (a.wilson 2012-07-25 17:15) - PLID 33429 - function to import parsed info into a new emn and insert into textbox detail.
	// (j.jones 2013-03-01 11:15) - PLID 55122 - added default EMN status
	// (j.jones 2013-04-08 16:32) - PLID 56149 - changed the progress to CProgressDialog
	BOOL SaveToPatientEMN(long nEMNStatus, CTxDoc* pDoc, CTxSeg* pSeg, CProgressDialog &progressDialog);
	// (a.wilson 2012-08-01 16:29) - PLID 51901 - find best matching provider.
	long FindBestMatchingProviderID(const CString & strProvider);
	// (a.wilson 2012-08-02 15:02) - PLID 51932 - function to update category datalists with necessary categories
	void EnsureCategories();
	void ParseAll(bool bSilent = false);
	// (a.wilson 2012-09-10 09:55) - PLID 51901 - check whether provider needs to be hidden
	void EnsureProviderEnabled();
	// (j.jones 2013-03-01 10:30) - PLID 55122 - check whether EMN status needs to be disabled & hidden
	void EnsureEMNStatus();

	// (j.jones 2013-02-27 16:18) - PLID 55117 - added an error count label
	void UpdateErrorCountLabel();

	// (j.jones 2013-02-28 11:15) - PLID 55119 - Returns true if the row has at least one unresolved error.
	bool RowHasError(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (j.jones 2013-02-27 16:35) - PLID 55119 - This function will add or remove an error color to a given column,
	// and show the red X icon if the row has errors when we're done.
	// Takes in a row, a column to track or clear an error for, and a boolean to determine if we're adding an error or removing it.
	void UpdateRowErrorStatus(NXDATALIST2Lib::IRowSettingsPtr pRow, EParseListColumns eColumn, bool bHasError);

	// (j.jones 2013-02-28 10:26) - PLID 55119 - updates a given column with colors
	void UpdateCellColor(NXDATALIST2Lib::IRowSettingsPtr pRow, EParseListColumns eColumn, OLE_COLOR clrText, OLE_COLOR clrHighlightedText); 

	// (j.jones 2013-02-28 15:53) - PLID 55121 - returns true if Commit is true and if the row is permitted to be imported,
	// it might not be if it still has errors
	BOOL NeedCommitRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (j.jones 2013-02-28 15:53) - PLID 55121 - returns true if Commit is checkable (it might not actually be checked)
	// and if the row is permitted to be imported, it might not be if it still has errors
	BOOL CanCommitRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (j.jones 2013-03-05 12:37) - PLID 55121 - takes in a parent-level row representing a file,
	// returns true if RowHasError returns true on at least one child record
	bool FileHasError(NXDATALIST2Lib::IRowSettingsPtr pParentRow);
	// (j.jones 2013-03-05 12:37) - PLID 55121 - Takes in a parent-level row representing a file,
	// returns false if m_opt.bRequireErrorsFixed is true and FileHasError returns true.
	// Always returns true if m_opt.bRequireErrorsFixed is false.
	bool CanImportFile(NXDATALIST2Lib::IRowSettingsPtr pParentRow);

	// (j.jones 2013-03-04 09:46) - PLID 55120 - Checks the setting to move the transcription file
	// to a folder. Returns true if succeeded, in which case the caller needs to remove the parent row.
	bool TryMoveImportedTranscriptionFile(CString strFilePath, bool bPrompt = false);

	// Generated message map functions
	//{{AFX_MSG(CParseTranscriptionsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnRButtonDownTree(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonUpTree(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBtnAddFile();
	afx_msg void OnBtnParseAll();
	afx_msg void OnBtnSetToSelection();
	afx_msg void OnCurSelWasSetTree();
	afx_msg void OnCheckReviewed();
	afx_msg void OnSelSetPatlist(LPDISPATCH lpSel);
	afx_msg void OnBtnImport();
	afx_msg void OnBtnConfigure();
	afx_msg void OnChangedParsetxDate();
	afx_msg void OnSelSetCategoryList(LPDISPATCH lpSel);
	afx_msg void OnChangeEditTitle();
	afx_msg void OnSelSetDefaultCategoryList(LPDISPATCH lpSel);
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnEditingFinishingTree(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnCheckViewWholeDocument();
	afx_msg void OnBtnEditText();
	afx_msg void OnSelChangingPatlist(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingDefaultCategoryList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingCategoryList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnKillFocusDate();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (a.wilson 2012-08-02 13:56) - PLID 51901 - event handlers for provider dropdown.
	void SelChangingProlist(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelSetProlist(LPDISPATCH lpSel);
	// (j.jones 2013-02-28 12:19) - PLID 55118 - added button to go to next error
	afx_msg void OnBtnGoToNextTranscriptionError();
	// (j.jones 2013-02-28 16:30) - PLID 55121 - added so we can prevent committing records with errors
	afx_msg void OnEditingStartingTree(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARSETRANSCRIPTIONSDLG_H__0EF209CD_28D2_464B_A25E_74C247FB817C__INCLUDED_)
