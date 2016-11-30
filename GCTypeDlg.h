#if !defined(AFX_GCTYPEDLG_H__43BA203F_B779_48F5_867A_1ADAA7BD12BB__INCLUDED_)
#define AFX_GCTYPEDLG_H__43BA203F_B779_48F5_867A_1ADAA7BD12BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GCTypeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGCTypeDlg dialog

class CGCTypeDlg : public CNxDialog
{
// Construction
public:
	CGCTypeDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGCTypeDlg)
	enum { IDD = IDD_GC_TYPE_DLG };
	NxButton	m_btnRewardPoints;
	NxButton	m_btnRechargeable;
	NxButton	m_btnDefaultDays;
	CNxEdit	m_nxeditGcTypeName;
	// (r.gonet 2015-03-25 10:13) - PLID 65276 - Added an NxEdit control for the Value field.
	CNxEdit	m_nxeditGcTypeValue;
	// (r.gonet 2015-03-25 10:13) - PLID 65276 - Renamed Amount to Price.
	CNxEdit	m_nxeditGcTypePrice;
	CNxEdit	m_nxeditGcTypeDays;
	CNxEdit	m_nxeditGcTypeRewardPointsEdit;
	CNxIconButton	m_btnNewGCType;
	CNxIconButton	m_btnDeleteGCType;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	// (j.jones 2009-06-22 10:58) - PLID 34226 - added support for inactivating gift certificates
	NxButton	m_checkInactive;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGCTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool m_bChanged;
	BOOL m_bNewRowDeleted; // (r.galicki 2008-08-05 17:15) - PLID 30945 - Added flag for when a new row is deleted rather than saved.
	long m_nCurrentSel;
	// (r.gonet 2015-03-25 09:38) - PLID 65276 - Control flag to tell the dialog when to push the changes from the Value field
	// to the Price field. Needed because programmatic changes to the Value field call the edit box's Changed event handler as well.
	// Only turns on when focus is given to the Value field. Turns off when the Value field loses focus.
	bool m_bSyncPriceWithValue;

	NXDATALISTLib::_DNxDataListPtr m_pTypes;
	NXDATALIST2Lib::_DNxDataListPtr m_pDiscountCategories;

	bool SaveIfNecessary(bool bSilent = true);
	void LoadCurrentSel();
	void EnsureDays();

	// (a.walling 2007-05-23 10:56) - PLID 26114
	void EnableRewardItems();
	long m_nRedeemCategoryID;

	// Generated message map functions
	//{{AFX_MSG(CGCTypeDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChangingGcTypeList(long FAR* nNewSel);
	afx_msg void OnSelChosenGcTypeList(long nRow);
	afx_msg void OnGcTypeExp();
	afx_msg void OnDeleteGcType();
	afx_msg void OnNewGcType();
	// (r.gonet 2015-03-25 09:38) - PLID 65276 - Added a handler for when the Value field loses focus.
	afx_msg void OnKillfocusGcTypeValue();
	// (r.gonet 2015-03-25 09:38) - PLID 65276 - Renamed Amount to Price.
	afx_msg void OnKillfocusGcTypePrice();
	afx_msg void OnKillfocusGcTypeDays();
	afx_msg void OnKillfocusGcTypeName();
	afx_msg void OnChangeGcTypeDays();
	afx_msg void OnChangeGcTypeName();
	// (r.gonet 2015-03-25 09:38) - PLID 65276 - Added a handler for when the Value field is changed.
	afx_msg void OnChangeGcTypeValue();
	// (r.gonet 2015-03-25 09:38) - PLID 65276 - Renamed Amount to Price.
	afx_msg void OnChangeGcTypePrice();
	afx_msg void OnGcTypeRecharge();
	afx_msg void OnGcTypeEditDiscountCategories();
	afx_msg void OnTrySetSelFinishedGcTypeRewardsCategoryList(long nRowEnum, long nFlags);
	afx_msg void OnSelChangingGcTypeRewardsCategoryList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangedGcTypeRewardsCategoryList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnChangeGcTypeRewardPointsEdit();
	afx_msg void OnGcTypeRewardsCheck();
	afx_msg void OnKillfocusGcTypeRewardPointsEdit();
	// (j.jones 2009-06-22 10:58) - PLID 34226 - added support for inactivating gift certificates
	afx_msg void OnCheckGcTypeInactive();
	// (j.gruber 2010-10-22 13:37) - PLID 30961
	void ChangeColumnSortFinishedGcTypeList(short nOldSortCol, BOOL bOldSortAscending, short nNewSortCol, BOOL bNewSortAscending);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
public:
	// (r.gonet 2015-03-25 10:13) - PLID 65276 - Added a handler for when the Value field changes.
	afx_msg void OnEnSetfocusGcTypeValueEdit();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GCTYPEDLG_H__43BA203F_B779_48F5_867A_1ADAA7BD12BB__INCLUDED_)
