#if !defined(AFX_INVBATCHCOMPLETEALLOCATIONSDLG_H__A5C7F845_699D_4F16_97FB_E07FEEDF1A4C__INCLUDED_)
#define AFX_INVBATCHCOMPLETEALLOCATIONSDLG_H__A5C7F845_699D_4F16_97FB_E07FEEDF1A4C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvBatchCompleteAllocationsDlg.h : header file
//

// (j.jones 2007-12-04 10:05) - PLID 28269 - created

/////////////////////////////////////////////////////////////////////////////
// CInvBatchCompleteAllocationsDlg dialog

class CInvBatchCompleteAllocationsDlg : public CNxDialog
{
// Construction
public:
	CInvBatchCompleteAllocationsDlg(CWnd* pParent);   // standard constructor
	~CInvBatchCompleteAllocationsDlg();

// Dialog Data
	//{{AFX_DATA(CInvBatchCompleteAllocationsDlg)
	enum { IDD = IDD_INV_BATCH_COMPLETE_ALLOCATIONS_DLG };
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	NxButton	m_radioBarcodeUsed;
	NxButton	m_radioBarcodeReleased;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvBatchCompleteAllocationsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;

	NXDATALIST2Lib::_DNxDataListPtr m_AllocationsList;

	void OnRadioBarcodeToggle();

	BOOL Save();

	//Runs through all allocations in the list, if any have been modified,
	//ensure they are completed - ignore any that aren't modified.
	//Return FALSE if some are modified and uncompleted, and the user
	//declined to auto-update the remaining items. Will warn if none are
	//modified.
	BOOL VerifyAllocationsResolved();

	//takes in the datalist pointer to the parent allocation,
	//then fills nCountDetails with the total number of child details
	//and fills nCountCompletedDetails with the number of child details
	//that have Used or Released checked off
	// (j.jones 2008-03-12 12:07) - PLID 29102 - need to know if the allocation is fully released
	void CheckAllocationCompletion(NXDATALIST2Lib::IRowSettingsPtr pParentRow, long &nCountDetails, long &nCountCompletedDetails, BOOL &bIsFullyReleased);

	//called after a "used" or "released" status is changed, these will
	//update the completion status on the parent row, and color that
	//cell appropriately
	void UpdateAllocationCompletionStatus_ByDetailRow(NXDATALIST2Lib::IRowSettingsPtr pDetailRow);
	void UpdateAllocationCompletionStatus_ByParentRow(NXDATALIST2Lib::IRowSettingsPtr pParentRow);

	/* not currently used
	//Run through every allocation that is partially complete, and update
	//incompleted details such that if the parameter is TRUE, mark them used,
	//otherwise mark them released
	void AutoCompletePartialAllocations(BOOL bMarkAsUsed);
	*/

	//PromptForQtyChange will take in a boolean for whether we're using or releasing a given product,
	//takes in the product name and current quantity, and then passes back the quantity they are using,
	//which must be greater than zero and not greater than the current quantity.
	//Return FALSE if the user cancelled, return TRUE if we are returning a valid new quantity.
	BOOL PromptForQtyChange(BOOL bUsed, CString strProductName, double dblCurQuantity, double &dblNewQuantity);

	long m_nNextNewDetailID;	//tracks negative numbers such that all our new details have unique IDs

	//TES 7/18/2008 - PLID 29478 - We may pop up the ProductItemsDlg, in which case we'll want to not handle barcode scans,
	// and pass in a list of ProductItemIDs that are in use on the dialog.
	BOOL m_bDisableBarcode;
	CString GetExistingProductItemWhereClause(long nServiceID);

	
	// Generated message map functions
	//{{AFX_MSG(CInvBatchCompleteAllocationsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRadioBatchBarcodeUsed();
	afx_msg void OnRadioBatchBarcodeReleased();
	afx_msg void OnEditingFinishedMultiAllocationList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingStartingMultiAllocationList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishingMultiAllocationList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVBATCHCOMPLETEALLOCATIONSDLG_H__A5C7F845_699D_4F16_97FB_E07FEEDF1A4C__INCLUDED_)
