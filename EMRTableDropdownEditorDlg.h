#if !defined(AFX_EMRTABLEDROPDOWNEDITORDLG_H__03F1E7B7_CFDC_44A4_9A1F_BC7B1C9C0914__INCLUDED_)
#define AFX_EMRTABLEDROPDOWNEDITORDLG_H__03F1E7B7_CFDC_44A4_9A1F_BC7B1C9C0914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRTableDropdownEditorDlg.h : header file
//

#include "EmrUtils.h"
#include "EmrItemEntryDlg.h"

// (z.manning 2009-02-10 15:10) - PLID 33026 - Moved the CEmrTableDropDownItem and 
// CEmrTableDropDownItemArray classes to EmrUtils.h


/////////////////////////////////////////////////////////////////////////////
// CEMRTableDropdownEditorDlg dialog

class CEMNDetail;
class CEmrItemEntryDlg;
// (c.haag 2008-02-20 15:26) - PLID 28686 - Added OnTimer for handling timer events
// that are set when m_bAutoAddNew is TRUE.
enum GlassesOrderDataType;
class CEMRTableDropdownEditorDlg : public CNxDialog
{
// Construction
public:
	CEMRTableDropdownEditorDlg(CEmrItemEntryDlg* pParent);   // standard constructor
	~CEMRTableDropdownEditorDlg();

	NXDATALISTLib::_DNxDataListPtr m_List;

	long m_nDataID;

	//TES 3/15/2011 - PLID 42757 - We can be given a type of Glasses Order field to link individual dropdown entries to.
	GlassesOrderDataType m_GlassesOrderDataType;

	CString m_strColumnName;

	BOOL m_bAllowEdit;

	CEmrTableDropDownItemArray m_arypEMRDropDownList;
	CEmrTableDropDownItemArray m_arypEMRDropDownDeleted;

	BOOL m_bAddingMultipleDataElements;

	CArray<CEMNDetail*,CEMNDetail*> m_apCurrentTableDetails; // If this is a table item, and we are editing this item from an EMR, this
															// array contains pointers to all of the details that have the same EmrInfoID
									// right-clicked on a detail and selected "Edit..." If there are 10 details, then there
									// will be 10 elements in the array.
	BOOL m_bMaintainCurrentTable; // TRUE if we must maintain the current table so that we can stop the user from changing
									// a column type if data exists for the current detail.

	BOOL m_bIsCurrentDetailTemplate; // TRUE if the detail we are maintaining is on a template

	BOOL m_bAutoAddNew; // (c.haag 2008-01-22 10:04) - PLID 28686 - If this is TRUE, the dialog will automatically
						// "press the new button for the user"

	BOOL m_bAutoAlphabetizeDropDown;// (a.vengrofski 2010-02-22 16:44) - PLID <37524> - This will be the local copy

	long m_nColumnType; // (z.manning 2011-03-17 09:18) - PLID 42722

	// (z.manning 2011-09-19 14:01) - PLID 41954 - Dropdown separators
	CString m_strDropdownSeparator;
	CString m_strDropdownSeparatorFinal;

	// (j.gruber 2014-08-05 11:03) - PLID 63164 - When you have the keyword checkbox checked for a drop down, default all new drop down items in that list to have a checked keyword.   Same for unchecked.
	BOOL m_bDefaultKeyword;

public:
	void SwapDataElements(long nDatalistIndex1, long nDatalistIndex2);

	// (a.walling 2014-06-30 10:21) - PLID 62497 - now returns an iterator rather than an index, which will let us know in debug mode if it has been invalidated.
	std::vector<std::unique_ptr<CEmrTableDropDownItem>>::iterator GetCurDataElementArrayIndex(long nSortOrderNumber);

// Dialog Data
	//{{AFX_DATA(CEMRTableDropdownEditorDlg)
	enum { IDD = IDD_EMR_TABLE_DROPDOWN_EDITOR_DLG };
	CNxIconButton	m_btnDataDown;
	CNxIconButton	m_btnDataUp;
	CNxIconButton	m_btnAddDataItem;
	CNxIconButton	m_btnAddMultipleDataItem;
	// (j.jones 2013-01-28 17:11) - PLID 54899 - renamed Add Product to Add Other
	CNxIconButton	m_btnAddOtherDataItem;
	CNxIconButton	m_btnDeleteDataItem;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnPasteData;
	// (b.spivey, June 25, 2013) - PLID 37100 - New spellcheck button. 
	CNxIconButton	m_btnSpellCheck; 
	CNxStatic		m_nxstaticDropdownListLabel;
	NxButton		m_btnAutoAlphabetizeDropDown;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRTableDropdownEditorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (a.walling 2012-08-02 16:14) - PLID 43277 - Always add new items with the next greatest sort order; upon hitting OK, items are re-numbered as appropriate starting at 1 with no gaps
	long m_nNextSortOrder;
	void UpdateSortOrders();
	// (d.thompson 2012-10-16) - PLID 53217
	void PasteSetupData();
	// (d.thompson 2012-10-17) - PLID 53217
	CEmrTableDropDownItem* GenerateNewDDI(CString strDataName);
	// (d.thompson 2012-10-17) - PLID 53217
	long CreateDatalistRowFromDDI(const CEmrTableDropDownItem *pDDI, long nOverrideGivenID = 0);
	// (d.thompson 2012-10-17) - PLID 53217
	bool CheckStringForDuplicates(CString strToFind, long nSortOrderToIgnore = -1);
	// (d.thompson 2012-10-17) - PLID 53217
	// (j.jones 2013-01-29 10:57) - PLID 54899 - added ability to add an action
	bool TryAddNewElementByNameWithDuplicateChecks(CString strNewName, OUT CString &strFailureReason,
		BOOL bAddAction = FALSE, EmrActionObject eaoDestType = (EmrActionObject)eaoCpt, long nDestID = -1);

	CEmrItemEntryDlg *m_pdlgEmrItemEntry; // (z.manning 2009-02-10 15:12) - PLID 33026

	// (j.jones 2013-01-29 10:50) - PLID 54899 - moved product adding to this function
	void AddProductToList();
	// (j.jones 2013-01-29 10:50) - PLID 54899 - added ability to add medications
	void AddMedicationsToList();

	// (b.spivey, June 27, 2013) - PLID 37100 - Validate functions that can be used in more than one place!
	bool ValidateListDataElement(long nRowIndex, CString strUserEnteredData, CString strOldValue); 
	bool ValidateListDataInUse(long nRowIndex, CString strUserEnteredData, CString strOldValue); 

	// Generated message map functions
	//{{AFX_MSG(CEMRTableDropdownEditorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishedEmrColumnList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnAddDataItem();
	afx_msg void OnAddMultipleDataItem();
	// (j.jones 2013-01-28 17:11) - PLID 54899 - renamed Add Product to Add Other
	afx_msg void OnAddOtherDataItem();
	afx_msg void OnDeleteDataItem();
	afx_msg void OnDataUp();
	afx_msg void OnDataDown();
	virtual void OnOK();
	afx_msg void OnSelChangedEmrColumnList(long nNewSel);
	afx_msg void OnEditingFinishingEmrColumnList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRButtonDownEmrColumnList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnTimer(UINT nIDEvent);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void LeftClickEmrColumnList(long nRow, short nCol, long x, long y, long nFlags); // (z.manning 2009-02-10 15:12) - PLID 33026
	afx_msg void OnBnClickedPasteSetupData();
	afx_msg void OnBnClickedAutoAlphabetizeDropdownData();
	afx_msg void OnBnClickedSpellCheck();
	// (j.gruber 2014-07-25 13:40) - PLID 62628 - KeywordOverride Handler
	// (j.gruber 2014-07-30 10:18) - PLID 62630 - changed to row
	void OnKeywordOverride(NXDATALISTLib::IRowSettingsPtr pRow);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRTABLEDROPDOWNEDITORDLG_H__03F1E7B7_CFDC_44A4_9A1F_BC7B1C9C0914__INCLUDED_)
