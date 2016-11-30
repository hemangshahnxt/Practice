#if !defined(AFX_WAITINGLISTENTRYDLG_H__8CF1DD62_2C3C_462C_81B2_1389EFB5D3EE__INCLUDED_)
#define AFX_WAITINGLISTENTRYDLG_H__8CF1DD62_2C3C_462C_81B2_1389EFB5D3EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WaitingListEntryDlg.h : header file
//

#include "WaitingListUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CWaitingListEntryDlg dialog

// (d.moore 2007-05-23 11:31) - PLID 4013

class CWaitingListEntryDlg : public CNxDialog
{
// Construction
public:
	long m_WaitListID; // Usually set from outside.

	CWaitingListEntryDlg(CWnd* pParent);   // standard constructor
	~CWaitingListEntryDlg();

	// (z.manning, 04/29/2008) - PLID 29814 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CWaitingListEntryDlg)
	enum { IDD = IDD_WAITING_LIST_ENTRY_DLG };
	CNxLabel	m_nxlPurposeLabel;
	CNxEdit	m_nxeditDateEntered;
	CNxEdit	m_nxeditWaitListNotes;
	CNxStatic	m_nxstaticPatient;
	CNxStatic	m_nxstaticApptType;
	CNxStatic	m_nxstaticPurpose;
	CNxStatic	m_nxstaticDateEntered;
	CNxStatic	m_nxstaticWaitListItems;
	CNxStatic	m_nxstaticNotes;
	CNxIconButton	m_btnAddRequestItem;
	CNxIconButton	m_btnEditRequestItem;
	CNxIconButton	m_btnRemoveRequestItem;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaitingListEntryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_nAppointmentID;
	long m_nPatientID;
	//TES 3/26/2008 - PLID 29426 - Renamed this from m_nInnitialApptTypeID
	long m_nInitialApptTypeID;
	
	NXDATALIST2Lib::_DNxDataListPtr m_pPatient;
	NXDATALIST2Lib::_DNxDataListPtr m_pApptType;
	NXDATALIST2Lib::_DNxDataListPtr m_pApptPurpose;
	NXDATALIST2Lib::_DNxDataListPtr m_pRequestItems;

	// This array stores the list of Purpose IDs currently selected.
	CArray<long, long> m_arPurposeIDs;

	// Stores new resource line items that have not yet been saved to the database.
	CArray<WaitListLineItem, WaitListLineItem> m_arLineItems;
	
	// Load purpose values appropriate to the value currently selected in the Type dropdown.
	void LoadPurposeDropdown(long nTypeID);

	// Set all controls from database values.
	void QueryData();

	// Set all controls with basic defaults.
	void SetDefaultValues();

	// Create a new Waiting List item entry in the database.
	bool SaveNewData();

	// Update the database entry for an existing Waiting List item.
	bool UpdateData();

	// Get data from all of the dropdown boxes and other fields.
	//  If data is missing then the user is prompted and NULL is returned.
	bool GetFormData(WaitListEntry &wlData);
	
	// Build a batch SQL query for adding all new line item
	//  entries to the database in the same connection.
	CString BuildLineItemSaveQuerys();

	CString GetDayQueryString(const WaitListLineItem &wlItem);
	CString GetResourceQueryString(const WaitListLineItem &wlItem);

	// Build a batch SQL query for adding all new appointment 
	//  purpose entries to the database in the same connection.
	CString BuildPurposeTableInsertQuery();

	// Fill the rgPurposeIDs array from the database.
	void QueryPurposeData();

	void QueryLineItemData();

	// Adds a new line item to the list but does not actually update the database.
	void AddNewLineItemData(WaitListLineItem &wlItem);

	// Replaces a line item in the list. It does not actually update the database.
	void ReplaceLineItem(const WaitListLineItem &wlItem, 
		NXDATALIST2Lib::IRowSettingsPtr pRow = NULL);

	// Drops an item from the list, but does not remove it from the database.
	void DeleteLineItem();

	void CWaitingListEntryDlg::OpenPurposeMultiList();

	
	// Generated message map functions
	//{{AFX_MSG(CWaitingListEntryDlg)
	afx_msg void OnAddRequestedItem();
	afx_msg void OnRemoveRequestedItem();
	afx_msg void OnEditRequestedItem();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangedRequestList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnSelChosenWlApptType(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedWlApptType(short nFlags);
	afx_msg void OnRequeryFinishedWlPurpose(short nFlags);
	afx_msg void OnSelChosenWlPurpose(LPDISPATCH lpRow);
	afx_msg void OnDblClickCellRequestList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnRequeryFinishedWlPatient(short nFlags);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAITINGLISTENTRYDLG_H__8CF1DD62_2C3C_462C_81B2_1389EFB5D3EE__INCLUDED_)
