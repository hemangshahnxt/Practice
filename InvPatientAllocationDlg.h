#if !defined(AFX_INVPATIENTALLOCATIONDLG_H__FD6CD8F4_06B3_4AA9_984B_54C97EC47EA8__INCLUDED_)
#define AFX_INVPATIENTALLOCATIONDLG_H__FD6CD8F4_06B3_4AA9_984B_54C97EC47EA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvPatientAllocationDlg.h : header file
//

// (j.jones 2007-11-05 17:08) - PLID 27987 - created

#include "InvUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CInvPatientAllocationDlg dialog

class CInvPatientAllocationDlg : public CNxDialog
{
// Construction
public:
	CInvPatientAllocationDlg(CWnd* pParent);   // standard constructor
	~CInvPatientAllocationDlg();

	long m_nID;	//the allocation batch ID, defaults to -1, set by calling function when opening an existing batch
	
	//the following "default" fields are potentially given by the caller,
	//and only used on init, when creating new batches	
	long m_nDefaultPatientID;
	long m_nDefaultAppointmentID;
	long m_nDefaultLocationID;
	long m_nDefaultProviderID;	// (j.jones 2008-03-05 11:47) - PLID 29201 - added provider to allocations

	// (j.jones 2008-02-27 16:47) - PLID 29108 - added ability to create from a case history
	long m_nDefaultCaseHistoryID;
	CString m_strDefaultCaseHistoryName;

	// (j.jones 2008-03-20 09:20) - PLID 29311 - added ability to create from an inventory order
	long m_nCreateFromOrderID;

	// (j.gruber 2008-09-02 11:17) - PLID 30814 - added preview button
// Dialog Data
	//{{AFX_DATA(CInvPatientAllocationDlg)
	enum { IDD = IDD_INV_PATIENT_ALLOCATION_DLG };
	CNxIconButton	m_btnPreview;
	CNxIconButton	m_btnCompleteAllocation;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxEdit	m_nxeditEditAllocationNotes;
	CNxStatic	m_nxstaticCaseHistorySelectLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvPatientAllocationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;	//used for label painting

	NXDATALIST2Lib::_DNxDataListPtr m_PatientCombo;		//dropdown list of patients
	NXDATALIST2Lib::_DNxDataListPtr m_LocationCombo;	//dropdown list of managed, active locations
	NXDATALIST2Lib::_DNxDataListPtr m_SelectionCombo;	//dropdown list of products/surgeries/quotes
	NXDATALIST2Lib::_DNxDataListPtr m_ProductCombo;		//dropdown list of products
	NXDATALIST2Lib::_DNxDataListPtr m_SurgeryCombo;		//dropdown list of surgeries
	NXDATALIST2Lib::_DNxDataListPtr m_QuoteCombo;		//dropdown list of quotes
	NXDATALIST2Lib::_DNxDataListPtr m_AppointmentCombo;	//dropdown list of appointments
	NXDATALIST2Lib::_DNxDataListPtr m_AllocationList;	//list of products to be allocated to the patient upon OK
	// (j.jones 2008-02-27 11:14) - PLID 29102 - added the case history link
	NXDATALIST2Lib::_DNxDataListPtr m_CaseHistoryCombo;
	// (j.jones 2008-03-05 11:47) - PLID 29201 - added provider to allocations
	NXDATALIST2Lib::_DNxDataListPtr m_ProviderCombo;

	BOOL m_bSurgeriesRequeried;	//tracks whether we have yet requeried the surgery combo
	BOOL m_bQuotesRequeried;	//tracks whether we have yet requeried the quotes combo

	void ResetQuotesFromClause();	//sets the quote from clause with the current patient ID

	BOOL Save();	// (j.jones 2007-11-27 10:48) - PLID 28196 - added a standalone Save function	

	//this function will take in basic product information and check stock,
	//request product items, etc., adding to the list if successful
	//TES 7/3/2008 - PLID 24726 - Added bSerialNumIsLotNum
	void TryAddProductToList(long nProductID, CString strProductName, double dblDesiredQuantity, BOOL bHasSerialNum, BOOL bHasExpDate, BOOL bSerialNumIsLotNum);

	//this function is called only from TryAddProductToList and
	//will take all the product information we need to add to the list, and do so
	// (c.haag 2008-03-11 12:39) - PLID 29255 - Added a type (Purchased Inv./Consignment) field.
	// I also made the parameters no longer optional. This helps ensure that every call to it doesn't 
	// unintentionally leave anything out.
	//TES 7/3/2008 - PLID 24726 - Added bSerialNumIsLotNum
	//TES 7/18/2008 - PLID 29478 - Added bAddToBeOrdered and bSerialized parameters
	void AddProductToList(long nProductID, CString strProductName, double dblQuantity,
		_variant_t varProductItemID, _variant_t varSerialNumber, _variant_t varExpDate, _variant_t varProductType, BOOL bSerialNumIsLotNum,
		BOOL bAddToBeOrdered, BOOL bSerialized);

	//m_mapExistingQuantities is only filled when editing an existing Allocation record,
	//and tracks the quantities used in the saved Allocation, per ServiceID
	CMap<long,long,double,double> m_mapExistingQuantities;
	
	//CalculateCurrentQuantity will search the current makeup of the Allocation and
	//tell you how many total products are in use, per ServiceID
	double CalculateCurrentQuantity(long nServiceID);

	//GetExistingProductItemWhereClause finds all the ProductItemsT records in the
	//current allocation that match the passed-in ServiceID,
	//and returns a string of "ProductItemsT.ID NOT IN (1, 2, 3....)" etc.
	CString GetExistingProductItemWhereClause(long nServiceID);

	// (j.jones 2008-01-07 14:41) - PLID 28479 - we no longer require the product to be billable
	// to be used in an allocation - which means we don't need this function
	//ReflectProductsByLocationID will requery the product list for those billable by a certain location
	//void ReflectProductsByLocationID();

	long m_nCurPatientID; //tracks the currently selected patient
	long m_nCurLocationID; //tracks the currently selected location

	long m_nPendingPatientID;	//used to handle inactive patients
	long m_nPendingLocationID;	//used to handle inactive locations
	long m_nPendingAppointmentID; //used simply to track the trysetsel for appts.
	long m_nPendingProviderID;	// (j.jones 2008-03-05 12:08) - PLID 29201 - used to handle inactive providers

	COleDateTime m_dtInputDate;

	CArray<long, long> m_aryDeletedDetails;	//tracks when we delete a previously saved allocation detail
	// (j.jones 2007-11-16 11:09) - PLID 28043 - track audit descriptions of deleted details
	CMap<long, long, CString, CString> m_mapDeletedDetailDesc;

	BOOL m_bDisableBarcode; //used as a mutex of sorts such that we won't accept barcodes when the product items dialog is visible

	// (j.jones 2007-11-12 15:49) - PLID 28074 - used to make the allocation read-only, if necessary
	void SecureControls();
	BOOL m_bReadOnly;

	// (j.jones 2007-11-16 11:05) - PLID 28043 - added for streamlined auditing
	CString GenerateAuditDescFromAllocationRow(NXDATALIST2Lib::IRowSettingsPtr pAllocationRow);

	// (j.jones 2007-11-16 11:32) - PLID 28043 - cached original values when editing existing allocations,
	// used for auditing purposes
	long m_nOrigPatientID;
	long m_nOrigLocationID;
	CString m_strOrigLocationName;
	long m_nOrigAppointmentID;
	CString m_strOrigApptDate;
	CString m_strOrigNotes;
	InvUtils::InventoryAllocationStatus m_iasOrigStatus;

	// (j.jones 2008-02-27 11:56) - PLID 29104 - store case history information for auditing purposes
	long m_nOrigCaseHistoryID;
	CString m_strOrigCaseHistoryName;

	// (j.jones 2008-03-05 11:47) - PLID 29201 - added provider to allocations
	long m_nCurProviderID;
	long m_nOrigProviderID;
	CString m_strOrigProviderName;

	// (j.jones 2007-11-28 16:39) - PLID 28196 - enables/disables the complete allocation button
	void CheckEnableCompleteAllocationBtn();

	// (j.jones 2008-02-27 16:51) - PLID 29108 - try to add products from a given case history
	// (j.jones 2008-03-07 12:24) - PLID 29108 - we removed this ability, though perhaps it may
	// return in the future
	//void TryAddProductsFromCaseHistory(long nCaseHistoryID, CString strCaseName);

	// (j.jones 2008-03-20 09:20) - PLID 29311 - added ability to create from an inventory order
	void TryAddProductsFromCompletedOrder(long nOrderID);

	// (j.jones 2008-03-20 10:22) - PLID 29311 - put the out-of-stock check in its own function
	//TES 7/18/2008 - PLID 29478 - Added an optional output parameter, pbAddOnOrder.  If this is a valid pointer, then the user
	// will be given the option to add the item with a "To Be Ordered" status, and that parameter will be filled with whether
	// or not they chose to do so.
	void CheckWarnLowStock(long nProductID, CString strProductName, double dblQuantityUsed, OUT BOOL *pbAddOnOrder = NULL);

	// (j.jones 2008-02-29 08:47) - PLID 29102 - lets us know if we're allowed to unlink the case history
	BOOL CanUnlinkCaseHistory();

	//(c.copits 2010-09-14) PLID 40317 - Allow duplicate UPC codes for FramesData certification
	NXDATALIST2Lib::IRowSettingsPtr GetBestUPCProduct(_variant_t varBarcode);

	// (j.jones 2008-02-28 17:41) - PLID 29102 - added OnSelChangingLocationAllocCombo
	// Generated message map functions
	//{{AFX_MSG(CInvPatientAllocationDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChosenProductAllocCombo(LPDISPATCH lpRow);
	afx_msg void OnRButtonDownAllocationList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelChosenLocationAllocCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenPatientCombo(LPDISPATCH lpRow);
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTrySetSelFinishedPatientCombo(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedLocationAllocCombo(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedAppointmentCombo(long nRowEnum, long nFlags);
	afx_msg void OnEditingFinishingAllocationList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingStartingAllocationList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedAllocationList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRequeryFinishedAppointmentCombo(short nFlags);
	afx_msg void OnSelChosenSurgeryAllocCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenQuoteAllocCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenAllocAddCombo(LPDISPATCH lpRow);
	afx_msg void OnBtnCompleteAllocation();
	afx_msg void OnRequeryFinishedAllocationList(short nFlags);
	afx_msg void OnRequeryFinishedCaseHistoryCombo(short nFlags);
	afx_msg void OnSelChangingLocationAllocCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingCaseHistoryCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenProviderAllocCombo(LPDISPATCH lpRow);
	afx_msg void OnTrySetSelFinishedProviderAllocCombo(long nRowEnum, long nFlags);
	afx_msg void OnBtnPreviewAllocations();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVPATIENTALLOCATIONDLG_H__FD6CD8F4_06B3_4AA9_984B_54C97EC47EA8__INCLUDED_)
