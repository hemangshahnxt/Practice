#if !defined(AFX_SuggestedSalesDlg_H__6EE00835_3B7E_41ED_B9A2_66E345367CE3__INCLUDED_)
#define AFX_SuggestedSalesDlg_H__6EE00835_3B7E_41ED_B9A2_66E345367CE3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SuggestedSalesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSuggestedSalesDlg dialog

#include "AdministratorRc.h"

enum ESuggestedSalesColumns {
	essSuggestionID,
	essServiceID,
	essOrderIndex,
	essName,
	essType,
	essReason,
	essColor,
	essForeColor
};

enum ESuggestedSalesServiceDropdownColumns {
	esssdcServiceID,
	esssdcName,
	esssdcCode,
	esssdcPrice,
	esssdcCategoryName
};

enum EAddSuggestionType {
	eastProduct,
	eastCode
};


// (a.walling 2007-03-28 13:18) - PLID 25356 - Dialog to setup suggested sales
class CSuggestedSalesDlg : public CNxDialog
{
// Construction
public:
	CSuggestedSalesDlg(CWnd* pParent);   // standard constructor

	// (a.wetta 2007-05-16 08:50) - PLID 25960 - Added UpdateView function for NexSpa tab
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

// Dialog Data
	//{{AFX_DATA(CSuggestedSalesDlg)
	enum { IDD = IDD_SUGGESTED_SALES_DLG };
	NxButton	m_btnEnableDragDropCopying;
	NxButton	m_btnEnableDragDropOrdering;
	CNxIconButton	m_nxibOrderBy;
	CNxIconButton	m_nxibCopyTo;
	CNxIconButton	m_nxibCopyFrom;
	CNxIconButton	m_nxibRemoveProduct;
	CNxIconButton	m_nxibAddProduct;
	CNxIconButton	m_nxibMoveUp;
	CNxIconButton	m_nxibMoveDown;
	CNxColor	m_ncColor;
	CNxStatic	m_nxstaticSuggestedSalesCaption;
	CNxStatic	m_nxstaticSuggestedSalesTip;
	CNxStatic	m_nxstaticSuggestedSalesTitle;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSuggestedSalesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SaveIndexes(); // recalculates indexes and saves all items in the list.
	void RecalcIndexes(); // recalculates indexes for all items in the list, does not write today. for future use.
	void ReloadAll(); // reloads everything. uses m_bReady.
	void RemoveSelectedProduct(); // removes cursel from list and data
	void EnableDialogItems(BOOL bOverride = FALSE, BOOL bShow = FALSE); // enabled dialog items depending on cursel,
		// or using bShow if bOverride is true (only in initdialog currently).
	void EnableDialogAll(BOOL bEnable);

	void OrderByName(); // order all items in the list by name, then saves.
	void OrderByNameAndType(); // order all items in the list by type and name, then saves.

	void AddSuggestion(EAddSuggestionType eastType); // creates a multiselect box to choose which items to include
		// in the list, using eastType (product, service code, etc)

	BOOL m_bDragCopyEnabled;
	BOOL m_bDragOrderEnabled;

	BOOL m_bOrderModified;
	BOOL m_bReady;
	BOOL m_bBadDrag;

	CTableChecker m_cptcodeChecker, m_productChecker;

	
	long m_nColor;
	long m_nServiceID;


	NXDATALIST2Lib::_DNxDataListPtr m_dlList;
	NXDATALIST2Lib::_DNxDataListPtr m_dlServiceList;
	CBrush m_brush;
	


	// Generated message map functions
	//{{AFX_MSG(CSuggestedSalesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRButtonUpSuggestedSales(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishingSuggestedSales(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedSuggestedSales(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnAddSuggestedProduct();
	afx_msg void OnDragBeginSuggestedSales(BOOL FAR* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags);
	afx_msg void OnDragEndSuggestedSales(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	afx_msg void OnEnableDragDropReasons();
	afx_msg void OnRemoveSuggestedProduct();
	afx_msg void OnEnableDragDropOrdering();
	afx_msg void OnMoveDown();
	afx_msg void OnMoveUp();
	afx_msg void OnCurSelWasSetSuggestedSales();
	afx_msg void OnOrderBy();
	afx_msg void OnRequeryFinishedSuggestedSales(short nFlags);
	afx_msg void OnCopySuggestedFrom();
	afx_msg void OnCopySuggestedTo();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelChangingSuggestedSalesServiceDropdown(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangedSuggestedSalesServiceDropdown(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnEditingStartingSuggestedSales(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnRequeryFinishedSuggestedSalesServiceDropdown(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SuggestedSalesDlg_H__6EE00835_3B7E_41ED_B9A2_66E345367CE3__INCLUDED_)
