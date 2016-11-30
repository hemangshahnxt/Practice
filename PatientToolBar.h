#if !defined(AFX_PATIENTTOOLBAR_H__B42ECFF4_AC4E_11D1_B2D6_00001B4B970B__INCLUDED_)
#define AFX_PATIENTTOOLBAR_H__B42ECFF4_AC4E_11D1_B2D6_00001B4B970B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// PatientToolBar.h : header file
//
#define IDC_TEXT1_TOOLBAR	1871871
#define IDC_TEXT2_TOOLBAR	1871872
#define IDC_PATIENTS_SEARCH	1871873
#define IDC_ACTIVE_TOOLBAR	1871874
#define IDC_PROSPECT_SEARCH	1871875
#define IDC_ALL_TOOLBAR		1871876
#define IDC_ALL_SEARCH		1871877
/////////////////////////////////////////////////////////////////////////////
// CPatientToolBar DAO record view

// (a.walling 2013-03-01 15:59) - PLID 55398 - Cache recent rows for person IDs in the patient toolbar to avoid massive memory faults

namespace PatientToolbar
{
struct CachedRow
{
	CachedRow(NXDATALIST2Lib::IRowSettingsPtr pRow = NULL)
		: ticks(::GetTickCount())
		, pRow(pRow)
	{}

	DWORD ticks;
	NXDATALIST2Lib::IRowSettingsPtr pRow;

	friend bool operator<(const CachedRow& l, const CachedRow& r) {
		return l.ticks < r.ticks;
	}
};
typedef std::map<long, CachedRow> RowCache;
}

class CPatientToolBar : public CToolBar
{
public:
	typedef enum
	{
		ptbcPersonID = 0,
		// (a.walling 2010-05-26 16:00) - PLID 38906 - We can save a lot of memory by calculating the 'full name' in the datalist's display column
		//ptbcFullName,
		ptbcCurrentStatus,
		// (j.gruber 2011-07-22 15:45) - PLID 44118
		ptbcForeColor,
		ptbcHardColumnCount,
	} Column; // Hard-coded columns

// Attributes
public:
	CWnd m_wndPatientsCombo;
	//TES 1/6/2010 - PLID 36761 - Let's protect the datalist.
protected:
	// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
	NXDATALIST2Lib::_DNxDataListPtr m_toolBarCombo;	
public:

// Implementation
public:
	long GetLastPatientID();
	//TES 1/5/2010 - PLID 36761 - We're not going to let people just set the patient IDs any more.
protected:
	long m_ActivePatientID;
	long m_LastPatientID;
	long m_TempLastPatientID;

	// (a.walling 2010-05-21 10:12) - PLID 17768 - Drop a breadcrumb (keep track of this patient in the history)
	void DropBreadcrumb(long nPatientID);
public:

	bool SetComboSQL(const CString &strNewSQL, bool bForce = false, bool bManual = false, long id = -25);
	CPatientToolBar();
	~CPatientToolBar();
	long m_PatientStatusFromSchedule;
	bool m_bChangedInScheduler;
	// (a.walling 2007-11-12 10:40) - PLID 28062 - Transparent CStatic and CButton classes for toolbar dialog controls
	// (a.walling 2008-04-21 11:21) - PLID 29642 - Use NxButton/CNxStatic for these now.
	NxButton m_butActive, m_butInactive, m_butAll;
	NxButton m_butPatient, m_butProspect, m_butPatientProspect;
	NxButton m_butFilter;
	//TES 1/6/2010 - PLID 36761 - Let's protect these variables (we ought to protect all of them, but these will do for now).
protected:
	// (a.walling 2008-08-20 15:47) - PLID 29642 - No more blank statics. What a terrible idea in the first place.
	CNxStatic m_text, m_text2, m_text3;
public:
	//TES 1/6/2010 - PLID 36761 - Encapsulates updating the "1234/4321" label with the correct position and count
	void UpdateStatusText();
	//TES 1/6/2010 - PLID 36761 - Copied here, was a static global function in MainFrm.cpp
	void ResetFilter();

	// (a.walling 2010-05-21 10:12) - PLID 17768 - Let the toolbar save the last patient (along with the other patient history info)
	void SaveLastPatient();

	NxButton m_butAllSearchCheck;
	NxButton m_butPatientProspectSearchCheck;
	NxButton m_butProspectSearchCheck;
	NxButton m_butPatientSearchCheck;
	void Requery();
	CString GetActivePatientName();
	// (j.jones 2008-10-31 13:20) - PLID 31891 - supported an optional passed-in connection
	// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
	CString GetExistingPatientName(long PatientID);
	// (b.savon 2011-11-16 15:28) - PLID 45433 - Thread safe, Does Patient Exist?  For Device Import
	// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
	BOOL DoesPatientExistByUserDefinedID(const long nUserDefinedID);
	// (j.jones 2007-07-17 09:21) - PLID 26702 - added birthdate functions
	COleDateTime GetActivePatientBirthDate();
	COleDateTime GetExistingPatientBirthDate(long PatientID);
	// (z.manning 2009-05-05 10:44) - PLID 28529 - Added user defined ID functions
	long GetActivePatientUserDefinedID();
	// (b.savon 2011-09-23 11:48) - PLID 44954 - Added pCon so that this method can be thread safe while processing plugin files
	// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
	long GetExistingPatientUserDefinedID(const long nPatientID);
	//TES 1/6/2010 - PLID 36761 - This now may fail, if the current user doesn't have permission to view this patient.  In that
	// case, this function will give an explanatory message and return FALSE.  Otherwise, it will return TRUE.
	// (a.wilson 2013-01-10 10:26) - PLID 54515 - created to get patient internal id based on patient userdefined id.
	// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
	long GetExistingPatientIDByUserDefinedID(long nPatientUserDefinedID);
	// (a.walling 2010-05-21 10:12) - PLID 17768 - Helper functions
	// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
	long GetUserDefinedIDByRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	COleDateTime GetPatientBirthDateByRow(NXDATALIST2Lib::IRowSettingsPtr pRow);

	BOOL TrySetActivePatientID(long newPatientID);
	long GetActivePatientID();
	BOOL DoesPatientExistInList(long PatID);
	BOOL CreateComboBox();
	BOOL CreateActiveButton();
	long IsActiveChecked();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	BOOL CreateSearchButtons();
	BOOL CreateRightOfComboButtons();
	long m_PatientIDFromScheduler;
	CFont m_searchButtonFont;
	bool m_bRequeried;
	CFont *m_pFont;
	short GetActivePatientStatus();
	short GetExistingPatientStatus(long nPatientID);
	CString GetFirstName();
	CString GetMiddleName(); // (z.manning 2008-11-17 10:41) - PLID 31129
	CString GetLastName();
	CString GetFullName();
	// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
	CString GetFullNameByRow(NXDATALIST2Lib::IRowSettingsPtr pRow); // (z.manning 2008-11-17 10:48) - PLID 31129
	void PopulateColumns();
	short GetFirstNameColumn();
	short GetMiddleNameColumn();
	short GetLastNameColumn();
	short GetPatientIDColumn();
	short GetBirthDateColumn();
	short GetSSNColumn();
	short GetCompanyColumn();
	short GetOHIPHealthCardColumn();	// (j.jones 2010-05-04 10:55) - PLID 32325
	// (j.gruber 2010-10-04 14:25) - PLID 40415  - added security group column
	short GetSecurityGroupColumn();

	// (a.walling 2007-05-04 09:53) - PLID 4850 - Called when the user has been changed, refresh any user-specific settings
	void OnUserChanged();

	// (a.walling 2010-08-19 08:17) - PLID 17768 - Called when preferences have been changed
	void OnPreferencesChanged();

	//TES 1/5/2010 - PLID 36761 - Some new functions, now that the toolbar and patient IDs are protected

	//TES 1/5/2010 - PLID 36761 - Returns the variant representing the value in the indicated column, for the currently selected row.
	_variant_t GetCurrentlySelectedValue(short nCol);

	//TES 1/5/2010 - PLID 36761 - Sets the indicated column to have the given value, for the currently selected row.
	void SetCurrentlySelectedValue(short nCol, const _variant_t &var);

	//TES 1/5/2010 - PLID 36761 - Sets the indicated column to have the given value, for the row represented by the given patient ID.
	void SetValueByPersonID(long nPersonID, short nCol, const _variant_t &var);

	//TES 1/5/2010 - PLID 36761 - Row position accessors
	// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
	NXDATALIST2Lib::IRowSettingsPtr GetCurrentlySelectedRow();
	NXDATALIST2Lib::IRowSettingsPtr GetFirstRow();
	NXDATALIST2Lib::IRowSettingsPtr GetLastRow();
	long GetRowCount();

	//TES 1/5/2010 - PLID 36761 - Dropdown state accessors.
	BOOL IsDroppedDown();
	void SetDroppedDown(BOOL bDroppedDown);

	//TES 1/5/2010 - PLID 36761 - Removes the currently selected row from the list, including setting the new selection.
	void RemoveCurrentRow();
	void RemoveRowByPersonID(long nPersonID);

	//TES 1/5/2010 - PLID 36761 - Ways to change the currently selected patient.
	//TES 1/11/2010 - PLID 36761 - Note that these can return false, if the user doesn't have access to the specified patient.
	// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
	bool SelectByRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	bool SelectFirstRow();
	bool SelectLastRow();

	bool SelectLastPatient();

	//TES 1/6/2010 - PLID 36761 - Is the datalist currently loading?
	bool IsLoading();

	//TES 8/14/2014 - PLID 63520 - Note that DoesPatientExistInList() checks the data, to see if the patient SHOULD be in the list. 
	// This function only checks the datalist itself.
	BOOL IsPersonCurrentlyInList(long nPersonID);

// Used for remembering our filter information
public:
	long SafeGetPatientID(long PatientID);
	CString m_strFilterFrom;
	CString m_strFilterWhere;
	CString m_strFilterString;
	// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
	void UpdatePatient(long nID);
	// (a.walling 2010-05-21 10:12) - PLID 17768 - Popup the patient breadcrumbs menu
	void PopupBreadcrumbMenu();

protected:

	// (a.walling 2010-05-21 10:12) - PLID 17768 - Class to keep track of last selected patients and handle adding, pruning, serializing, deserializing
	class CBreadcrumbTrail
	{
	public:
		CBreadcrumbTrail();
		~CBreadcrumbTrail();

		void Clear();

		void Add(long nPatientID, long nUserDefinedID, const CString& strDescription);
		int GetLength();
		long GetHeadPatientID();

		// (a.walling 2010-05-21 10:12) - PLID 17768 - Create the popup menu with all entries
		// (a.walling 2010-06-29 08:11) - PLID 17768 - For safety, pass in the active patient ID
		void CreatePopupMenu(CMenu& menu, CMap<long, long, long, long>& mapIDs, long nActivePatientID);

		// (a.walling 2010-05-21 10:12) - PLID 17768 - Serialize the data to preferences
		void Save();
		// (a.walling 2010-05-21 10:12) - PLID 17768 - Load from preferences
		void Load();

		// (a.walling 2010-08-19 08:44) - PLID 17768 - This is safe to call externally
		void Prune();

		struct CBreadcrumb
		{
			CBreadcrumb() {
				::ZeroMemory(&m_ftDate, sizeof(m_ftDate));
			};

			long m_nPatientID;
			long m_nUserDefinedID;
			FILETIME m_ftDate;
			CString m_strDescription;
		};

	protected:
		CList<CBreadcrumb*, CBreadcrumb*> m_listBreadcrumbs;

		void LoadFromVariant(_variant_t& varData);
		void AddFromData(long nPatientID, long nUserDefinedID, const FILETIME& ftDate, const CString& strDescription);
	};
	CBreadcrumbTrail m_BreadcrumbTrail;

	// (a.walling 2013-03-01 15:59) - PLID 55398 - Check cache, then scan
	// (a.walling 2015-02-19 08:56) - PLID 64653 - Do not FindByColumn if requerying if bWaitForRequery is false
	NXDATALIST2Lib::IRowSettingsPtr FindRowByPersonID(long nID, bool bWaitForRequery = true);

	PatientToolbar::RowCache m_rowCache;

	CString GetDefaultFilter();

	//TES 1/18/2010 - PLID 36895 - Calculates the FROM clause (which needs to take blocked patients into account)
	CString GetFromClause();

	// (a.walling 2008-08-20 15:39) - PLID 29642 - Handle themes
	UXTheme &m_uxtTheme;
	BOOL m_bThemeInit;

	// (a.walling 2008-08-20 15:39) - PLID 29642 - Last mouse over button index for hottracking
	long m_nLastMouseOverIx;
	BOOL m_bTrackingMouse;

	// (j.jones 2013-02-15 10:41) - PLID 40804 - moved the last/first sorting to its own function
	// Takes in the nCol you're sorting on, returns TRUE if the function did sort, which means
	// the caller should not continue with its own sort.
	BOOL ApplySortLastFirstAtOnce(short nCol);

	// (a.walling 2008-08-20 15:40) - PLID 29642 - Added CustomDraw and EraseBkgnd handlers,
	// and also OnMouseMove and OnMouseLeave for custom hottracking
	// (a.walling 2008-08-21 14:09) - PLID 31056 - Added OnLButtonUp handler to fix drawing issue with tooltips
	// Generated message map functions
	// (a.walling 2008-10-02 17:23) - PLID 31575 - Border drawing (NcPaint)
	//{{AFX_MSG(CPatientToolBar)
	// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
	afx_msg void OnSelChosenPatientCombo(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedPatientCombo(short nFlags);
	afx_msg void OnTrySetSelFinishedPatientCombo(long nRowEnum, long nFlags);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnCustomDraw(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnColumnClickingPatientCombo(short nCol, BOOL FAR* bAllowSort);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint pt); // (a.walling 2010-05-21 10:12) - PLID 17768 - Handle right mouse click on the toolbar buttons as well
	afx_msg void OnDestroy();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATIENTTOOLBAR_H__B42ECFF4_AC4E_11D1_B2D6_00001B4B970B__INCLUDED_)
