#if !defined(AFX_NOTESDLG_H__3AF89093_2B98_11D2_80A7_00104B2FE914__INCLUDED_)
#define AFX_NOTESDLG_H__3AF89093_2B98_11D2_80A7_00104B2FE914__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
#include "PatientDialog.h"

// NotesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNotesDlg dialog

// (j.jones 2011-09-19 14:46) - PLID 42135 - added enum for billing note type
enum BillingNoteType {

	bntNone = -1,	//not a billing note
	bntBill = 0,	//a note for a bill
	bntCharge,		//a note for a charge
	bntPayment,		//a note for a pay/adj/ref
};

// (d.singleton 2012-03-28 11:48) - PLID 49257
struct UnsavedChargeNote 
{
	_variant_t varNoteID;
	_variant_t varPersonID;
	_variant_t varDate;
	_variant_t varUserID;
	_variant_t varCategory;
	_variant_t varNote;
	_variant_t varPriority;
	_variant_t varClaim;
	_variant_t varStatement;
	_variant_t varLineItemID;

	UnsavedChargeNote() {
		varClaim = g_cvarFalse;
		varStatement = g_cvarFalse;
		varPriority = (int)3;
		varCategory = (int)-25;
	}
};

class CNotesDlg : public CPatientDialog
{
// Construction
public:
	BOOL m_bUnsavedCharge;
	bool m_NewNote;
	bool m_IsComboVisible;
	// (b.cardillo 2006-12-13 13:02) - PLID 6808 - Changed the notes list to a dl2 for pixel-wise vertical scrolling.
	NXDATALIST2Lib::_DNxDataListPtr   m_pNxDlNotes;	
	bool m_bWhiteAdd;
	bool m_bAllowUpdate;
	CString m_SQLStr;
	CNotesDlg(CWnd* pParent, BOOL bUnsavedCharge = FALSE);   // standard constructor
	virtual void SetColor(OLE_COLOR nNewColor);
	BOOL OnInitDialog();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	CString GenerateSQL();
	COleVariant varSelItem;
	virtual int Hotkey(int key);

	// (a.walling 2010-09-20 11:41) - PLID 40589 - Set the patient ID
	void SetPersonID(long nID);

	long m_nBillID;
	long m_nLineItemID;
	long m_nMailID; // (c.haag 2010-07-01 13:45) - PLID 39473

	bool m_bSplit; // set to true when item description is split; is set to false in OnEditingFinished

	//If this dialog is called from the Billing tab this will be set to true
	bool m_bIsBillingNote;
	// (j.jones 2011-09-19 14:46) - PLID 42135 - added BillingNoteType, used if m_bIsBillingNote is true
	BillingNoteType m_bntBillingNoteType;

	// (c.haag 2010-07-01 13:45) - PLID 39473 - If this dialog is called from the History tab then this
	// will be set to true
	bool m_bIsHistoryNote;
	bool m_bIsForPatient; // (a.walling 2010-09-09 12:20) - PLID 40267 - Is this a patient? If not, it's a contact.
	// (a.walling 2010-09-20 11:41) - PLID 40589 - Just use the overridden id now
	//long m_nHistoryNotePersonID; // (c.haag 2010-08-26 11:26) - PLID 39473
	DWORD m_clrHistoryNote; // (c.haag 2010-08-26 11:26) - PLID 39473


	// (j.dinatale 2010-12-22) - PLID 41885 - needed to keep track of lab information
	bool m_bIsLabNotes;
	long m_nLabResultID;
	long m_nLabID;
	DWORD m_clrLabNote;

	// (j.gruber 2014-12-12 15:00) - PLID 64391 - Rescheduling Queue - attached appointments to notes
	void SetApptInformation(long nApptID, COleDateTime dtStartTime, CString strApptTypePurpose, CString strApptResources);

	// (j.dinatale 2010-12-22) - PLID 41915 - text to prepend to a note
	CString m_strPrependText;

	// (j.dinatale 2010-12-23) - PLID 41932 - stores the new note category override
	long m_nCategoryIDOverride;

	// (j.dinatale 2010-12-23) - PLID 41930 - flag to determine if there should be a note added and in edit mode one the dialog pops up
	bool m_bAutoAddNoteOnShow;

	// (j.dinatale 2010-12-23) - PLID 41930 - event to handle when the requery finishes
	void OnRequeryFinishedNoteList(short nFlags);

	// (j.armen 2012-03-19 09:01) - PLID 48780 - needed to keep track of recall information
	_variant_t m_vtRecallID;
	OLE_COLOR m_clrRecallNote;

	//(s.dhole 8/29/2014 10:22 AM ) - PLID 63516 set defult value
	long m_nPatientRemindersSentID;
	bool m_bIsPatientRemindersSent;
	CString m_sPatientRemindersPrefix;

	CArray<UnsavedChargeNote, UnsavedChargeNote> m_arUnsavedChargeNotes;

	// Dialog Data
	//{{AFX_DATA(CNotesDlg)
	enum { IDD = IDD_NOTES_DLG };
	NxButton	m_btnShowGridlines;
	NxButton	m_btnFilterCategory;
	CNxIconButton	m_btnSearchNotes;
	CNxIconButton	m_editMacros;
	CNxIconButton	m_AddMacroBtn;
	CNxIconButton	m_editCategories;
	CNxIconButton	m_addButton;
	CNxColor	m_bkg;
	CNxIconButton	m_btnCheckSpelling; // (b.cardillo 2006-12-13 12:40) - PLID 23713 - Spell-checking of all notes.
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNotesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SecureControls();

	long m_id;
	bool m_bIsValidID;  // (a.walling 2010-09-20 11:41) - PLID 40589 - if m_id set manually, GetActivePersonID will always return m_id
	bool m_bRequeryToSetSel;

	void AddNewNote(long nMacroID = -1, long nThreadID = -1 );
	
	// (a.walling 2011-05-10 09:16) - PLID 41789 - Gather more macro info
	void GetMacro(const IN long nMacroID, CString &strMacro, long& nCatID, bool& bMacroSuppressUserName, bool& bMacroShowOnStatement);
	// (j.armen 2011-07-21 17:03) - PLID 13283 - Added multiselect for filter
	BOOL FilterNotesByCat(long nCatID, BOOL bShowMultiSelect);
	void FilterNotesByRow(long nRow, BOOL bShowMultiSelect = FALSE);
	CArray<long, long> m_arynCatSelection;
	CNxLabel m_nxlMultipleCatSelection;
	long m_nLastNoteFilterRow;

	// (j.gruber 2014-12-12 15:00) - PLID 64391 - Rescheduling Queue - attached appointments to notes
	long m_nApptID;
	COleDateTime m_dtApptStartTime;
	CString m_strApptTypePurpose;
	CString m_strApptResources;
	
	// (a.walling 2011-05-10 09:16) - PLID 41789 - Use just one function and recordset to get the macro info
	//long GetMacroCategoryID(const IN long nMacroID = -1);
	void AttemptPromptForReferral();

	// (c.haag 2010-08-26 11:26) - PLID 39473 - Returns the correct "active" person ID.
	long GetActivePersonID();
	// (c.haag 2010-08-26 11:26) - PLID 39473 - Returns the correct "active" person name.
	CString GetActivePersonName();

	NXDATALISTLib::_DNxDataListPtr m_pCategories;

	BOOL m_bIsCreatingNewNote;
	// (a.walling 2010-07-13 18:11) - PLID 39182 - Check permissions and etc to see if we can edit this note
	bool CanEditNote(NXDATALIST2Lib::IRowSettingsPtr pCheckRow, bool& bRequireSameDayCheck, bool bSilent, short nCol);

	// (r.galicki 2008-06-27 12:54) - PLID 18881
	void RefreshColors();

	CTableChecker m_tcNoteCategories;

	// (a.wilson 2014-08-08 15:55) - PLID 63244 - function to update note categories.
	//-1 explains that an ID was not passed.
	void EnsureUpdatedCategoryFilter(const long & nID = -1);

	// (j.gruber 2009-11-02 11:21) - PLID 35815 - added subtab for patient messages
	NxTab::_DNxTabPtr m_SubTab;
	BOOL m_bThemeOpen;
	void InitializeTabs();
	void ResetControls();
	void LoadList();
	NXDATALIST2Lib::_DNxDataListPtr m_pMessagesList;



	// (b.spivey, October 02, 2012) - PLID 30398 - Checkbox and label controls. 
	NxButton	m_checkHideBills;
	NxButton	m_checkHideClaims;
	NxButton	m_checkHideStatements;


	enum noteActionType {
		natAddNote = 0,
		natAddMacro,
	};

	void PopupMenu(noteActionType naType);
	void AddMacro(long nThreadID = -1);

	//(c.copits 2011-07-05) PLID 22709 - Remember column settings
	void SetColumnSizes();
	void SetColumnSorts();
	void SaveColumnSettings();
	void ColumnSizingFinishedNotes(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	void ColumnSizingFinishedNotesMessageThreads(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	void OnBnClickedRememberColSettings();

	// Generated message map functions
	//{{AFX_MSG(CNotesDlg)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	// (b.cardillo 2006-12-13 13:02) - PLID 6808 - Changed the notes list to a dl2 for pixel-wise vertical scrolling.
	afx_msg void OnRButtonUpNxdlnotes(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishedNxdlnotes(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnColumnClickingNxdlnotes(short nCol, BOOL FAR* bAllowSort);
	afx_msg void OnEditingStartedNxdlnotes(LPDISPATCH lpRow, short nCol, long nEditType);
	afx_msg void OnEditingFinishingNxdlnotes(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnSelChangedNxdlnotes(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnFocusLostNxdlnotes();
	afx_msg void OnAddButtonClicked();
	afx_msg void OnDeleteNote(long nThreadID = -1);
	afx_msg void OnEditCategory();
	afx_msg void OnAddMacroButtonClicked();
	afx_msg void OnEditMacros();
	afx_msg void OnSearchNotes();
	afx_msg void OnSelChangingNoteCategoryList(long FAR* nNewSel);
	afx_msg void OnRequeryFinishedNoteCategoryList(short nFlags);
	afx_msg void OnSelChosenNoteCategoryList(long nRow);
	afx_msg void OnCategoryFilter();
	afx_msg void OnNotesShowGrid();
	afx_msg void OnCheckSpellingBtn(); // (b.cardillo 2006-12-13 12:40) - PLID 23713 - Spell-checking of all notes.
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam); // (j.armen 2011-08-04 16:19) - PLID 13283
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message); // (j.armen 2011-08-04 16:19) - PLID 13283
	afx_msg void OnHideBillNotes(); // (b.spivey, October 05, 2012) - PLID 30398 - Event handlers for hiding notes.
	afx_msg void OnHideClaimNotes();
	afx_msg void OnHideStatementNotes();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_IsEditing;
protected:
	void SelectTabTabPatientNotes(short newTab, short oldTab);
	void EditingStartingPatientMessagingThreadList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void EditingFinishedPatientMessagingThreadList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void RButtonUpPatientMessagingThreadList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void EditingFinishingPatientMessagingThreadList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NOTESDLG_H__3AF89093_2B98_11D2_80A7_00104B2FE914__INCLUDED_)
