#pragma once
#include "AdministratorRc.h"

// CDiagQuickListDlg dialog
// (c.haag 2014-02-21) - PLID 60931 - Initial implementation

class CDiagQuickListDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CDiagQuickListDlg)

public:
	// (c.haag 2014-03-05) - PLID 60930 - We need to know if this is an EMR template so
	// we can adjust the Visit button label
	BOOL m_bIsEMRTemplate;

protected:
	CNxIconButton m_btnClose;
	CNxIconButton m_btnAddCodesToVisit;
	CNxIconButton m_btnClearMyQuickList;

protected:
	NXDATALIST2Lib::_DNxDataListPtr	m_UserList;
	NXDATALIST2Lib::_DNxDataListPtr	m_QuickList;
	NXDATALIST2Lib::_DNxDataListPtr	m_diagSearch;

protected:
	// Requeries the QuickList, and optionally, the shared user list
	void Requery(BOOL bRequerySharedUserList);
	// (c.haag 2014-02-24) - PLID 60940 - Adds multiple QuickList items to the list
	void AddQuickListItems(IDispatchPtr pDisp, BOOL bFromSearch);
	// (c.haag 2014-02-24) - PLID 60940 - Adds a single Quick List item to the list
	void AddQuickListItem(IDispatchPtr pDisp, BOOL bFromSearch);
	// (c.haag 2014-02-24) - PLID 60940 - Returns TRUE if the specified QuickList row belongs
	// to another Practice user
	BOOL IsSharedQuickListRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (c.haag 2014-02-25) - PLID 60947 - Returns TRUE if at least one user is selected in the
	// shared users list
	BOOL HasSharedLists();
	// Returns TRUE if we're showing ICD-9s
	BOOL IsShowingICD9s();
	// Returns TRUE if we're showing ICD-10s
	BOOL IsShowingICD10s();
	// Determines if the current row can have it's 9 code changed
	bool CanChange9Code(NXDATALIST2Lib::IRowSettingsPtr pRow);	// (j.armen 2014-03-20 09:47) - PLID 60943
	// Allows user to select a new 9 code when using crosswalk
	void Change9Code(NXDATALIST2Lib::IRowSettingsPtr pRow);		// (j.armen 2014-03-20 09:47) - PLID 60943
	// (r.farnworth 2014-07-17 09:20) - PLID 62826 - Check if we change ICD-10 codes
	bool CanChange10Code(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (r.farnworth 2014-07-17 10:04) - PLID 62826 - Find the appropriate match type for the corresponding ICD-9 code and allow the user to choose a matching 10
	void Change10Code(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (r.farnworth 2014-07-17 15:52) - PLID 62826 - Moved into a function to be used elsewhere
	BOOL CheckForDuplicates(_variant_t vNewICD9ID, _variant_t vNewICD10ID);
	// (r.farnworth 2014-07-18 11:56) - PLID 62967 - Handle Hyperlinking
	void SetHyperLinkFormat(NXDATALIST2Lib::IRowSettingsPtr pRow, enum QuickListColumns linkField, bool bRemove);

public:
	// (c.haag 2014-03-03) - PLID 60928 - The list of selected QuickList items. This is populated
	// when the user clicks on the Add Codes to Visit button.
	// (j.armen 2014-08-11 11:56) - PLID 63238 - MFCArray
	MFCArray<IDispatchPtr> m_aSelectedQuickListItems;

public:
	CDiagQuickListDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDiagQuickListDlg();

// Dialog Data
	enum { IDD = IDD_DIAG_QUICKLIST };

protected:
	virtual BOOL OnInitDialog();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// (r.farnworth 2014-07-18 12:40) - PLID 62967
	struct DiagnosisCodeCommit;
	scoped_ptr<DiagnosisCodeCommit> m_pDiagCodeCommitMultiMatch;
protected:
	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBtnCopyFromUser();
	afx_msg void OnBnClickedBtnClearMyQuicklist();
	afx_msg void OnBnClickedBtnAddCodesToVisit();
	void OnSelChosenDiagSearchList(LPDISPATCH lpRow);
	afx_msg void OnEditingFinishedUserList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRButtonDownQuickList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void LeftClickQuickList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnDestroy();
	// (r.farnworth 2014-07-18 12:40) - PLID 62967
	void EditingFinishedDiags(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	
};
