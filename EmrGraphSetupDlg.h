#pragma once
//(a.wilson 2012-4-5) PLID 49479 - implementation of emr graph setup dialog.

// CEmrGraphSetupDlg dialog
#include "EmrRc.h"

class CEmrGraphSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrGraphSetupDlg)

public:
	CEmrGraphSetupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEmrGraphSetupDlg();

// Dialog Data
	enum { IDD = IDD_EMR_GRAPH_SETUP_DLG };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CNxIconButton m_btnClose, m_btnNewLine, m_btnNewGraph, m_btnRemoveGraph, m_btnRemoveLine, m_btnRemoveItem;
	NXDATALIST2Lib::_DNxDataListPtr m_dlpGraphList, m_dlpLineList, m_dlpAddItemList, m_dlpItemList;
	bool m_bGraphChanged, m_bLineChanged;
	

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	void SelSetEmrgsGraphList(LPDISPATCH lpSel);
	void SelSetEmrgsLineList(LPDISPATCH lpSel);
	void RequeryFinishedEmrgsLineList(short nFlags);
	void LeftClickEmrgsLineList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void SelChosenEmrgsAddItemList(LPDISPATCH lpRow);
	void RightClickEmrgsGraphList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RightClickEmrgsLineList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RightClickEmrgsItemList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RequeryFinishedEmrgsGraphList(short nFlags);
	void RequeryFinishedEmrgsItemList(short nFlags);
	void LoadDateValues(LPDISPATCH lpRow);
	void LoadCellValues(LPDISPATCH lpRow, long nType, long nItemListColumn, bool bAddAll);
	void EditingFinishedEmrgsItemList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingStartingEmrgsItemList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void EditingFinishedEmrgsLineList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingFinishedEmrgsGraphList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void OnClose();
	void UpdateActiveControls();
	void SelSetEmrgsItemList(LPDISPATCH lpSel);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRemoveItem();
	afx_msg void OnRemoveLine();
	afx_msg void OnChangeGraphActivity();
	afx_msg void OnNewGraph();
	afx_msg void OnNewLine();
	afx_msg void OnRemoveGraph();
public:
	void SelChangingEmrgsGraphList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingEmrgsLineList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void EditingStartingEmrgsGraphList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
};
