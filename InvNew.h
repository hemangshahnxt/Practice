#if !defined(AFX_INVNEW_H__E1417650_2A56_11D3_A1C7_00104BD3573F__INCLUDED_)
#define AFX_INVNEW_H__E1417650_2A56_11D3_A1C7_00104BD3573F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvNew.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInvNew dialog
enum TrackStatus {
	tsNotTrackable = 0,
	tsTrackOrders,
	tsTrackQuantity,
};
struct LocationRecord{
	double dblOnHandQty;
	//(e.lally 2007-02-26)PLID 23258- We were originally going to allow these selections to be
		//configured per location on the entry screen, but then
		//decided against it. I left the struct in for easier additions
		//in the future.
	//BOOL bBillable;
	//TrackStatus tsTrackStatus;
};

class CInvNew : public CNxDialog
{
// Construction
public:
	int GetIntTrackableStatus();
	CInvNew();   // standard constructor
	void OnChangeTrackable();

	//This is the final name that is saved to the database
	CString m_strFinalName;
	BOOL m_bFinalBillable;
	NXDATALIST2Lib::_DNxDataListPtr m_pLocationCombo;

// Dialog Data
	//{{AFX_DATA(CInvNew)
	enum { IDD = IDD_INVNEW };
	CNxIconButton	m_prevLocationBtn;
	CNxIconButton	m_nextLocationBtn;
	CNxIconButton	m_btnPickCategory;
	CNxIconButton	m_btnRemoveCategory;
	NxButton	m_radioNotTrackable;
	NxButton	m_radioTrackOrders;
	NxButton	m_radioTrackQuantity;
	NxButton	m_taxable2;
	CNxIconButton	m_okBtn;
	CNxIconButton	m_cancelBtn;
	NxButton	m_taxable;
	NxButton	m_billable;
	CNxStatic	m_location;
	CNxEdit	m_nxeditName;
	CNxEdit	m_nxeditActual;
	CNxEdit	m_nxeditCategory;
	CNxStatic	m_nxstaticActualText;
	//}}AFX_DATA

	// If the user tries to save the item without checking the "Billable" checkbox, 
	// they will receive this warning and have the opportunity to go back and check 
	// it, or just proceed without it checked.  If this string is left blank, the 
	// user will not be prompted regardless of whether the box is checked or not.
	CString m_strWarnIfNotBillable;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvNew)
	public:
	virtual int DoModal(long category);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	// (j.jones 2015-03-03 13:35) - PLID 64965 - products can now have multiple categories
	std::vector<long> m_aryCategoryIDs;
	long m_nDefaultCategoryID;

	BOOL m_bVisitedMultiple;
	CMap <long, long, LocationRecord, LocationRecord> m_locationMap;

	virtual void OnOK();
	virtual void OnCancel();
	void UpdateArrows();
	void HandleSelChangedLocation(long nOldSelLocationID);
	void SetTrackableStatus(int nTrackStatus);
	TrackStatus GetEnumTrackableStatus();

	// Generated message map functions
	//{{AFX_MSG(CInvNew)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeBillable();
	afx_msg void OnRadioNotTrackableItem();
	afx_msg void OnRadioTrackOrdersItem();
	afx_msg void OnRadioTrackQuantityItem();
	afx_msg void OnPreviousLocation();
	afx_msg void OnNextLocation();
	afx_msg void OnTrySetSelFinishedLocation(long nRowEnum, long nFlags);
	afx_msg void OnSelChangedLocation(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnRequeryFinishedLocation(short nFlags);
	afx_msg void OnCategoryPicker();
	afx_msg void OnCategoryRemove();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVNEW_H__E1417650_2A56_11D3_A1C7_00104BD3573F__INCLUDED_)
