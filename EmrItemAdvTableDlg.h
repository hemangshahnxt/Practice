#if !defined(AFX_EmrItemAdvTableDlg_H__6F27AADE_3B2D_468A_9A66_2FD4A222D0EF__INCLUDED_)
#define AFX_EmrItemAdvTableDlg_H__6F27AADE_3B2D_468A_9A66_2FD4A222D0EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrItemAdvTableDlg.h : header file
//

#include "EmrItemAdvDlg.h"
#include "EmrItemAdvTableBase.h"

/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvTableDlg dialog

class CEmrItemAdvTableDlg :
	public CEmrItemAdvDlg,
	public CEmrItemAdvTableBase
{
// Construction
public:
	CEmrItemAdvTableDlg(class CEMNDetail *pDetail);
	virtual ~CEmrItemAdvTableDlg();

public:
	void UpdateLinkedItemList();
	//DRT 7/30/2008 - PLID 30893 - Removed RemoveLinkedItemSelection

	void CalcComboSql();

private:
	// (c.haag 2008-01-14 11:15) - PLID 17936 - This function returns the source
	// string for a dropdown column given a column ID. The string is generated
	// from information stored inside a member map where the key is the column ID.
	// (z.manning 2011-10-11 11:14) - PLID 42061 - Added stamp ID
	CString GetDropdownSource(long nColumnID, const long nStampID);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrItemAdvTableDlg)
	//}}AFX_VIRTUAL

public:
	virtual BOOL RepositionControls(IN OUT CSize &szArea, BOOL bCalcOnly);

	virtual void ReflectCurrentContent();
	virtual void DestroyContent();

	//TES 3/23/2010 - PLID 37757 - This dialog doesn't maintain its own ReadOnly flag, so I changed the function name from
	// SetReadOnly() to ReflectReadOnlyStatus()
	virtual void ReflectReadOnlyStatus(BOOL bReadOnly);

	void ReflectCurrentState();

	// (a.walling 2014-01-30 00:00) - PLID 60546 - Quantize 
	Nx::Quantum::Batch UpdateColumnInfoInData(BOOL bSaveTableColumnWidths, BOOL bIsTemplate = FALSE);

protected:
	CString GenerateNewVarState();

protected:
	// (j.jones 2008-06-05 09:46) - PLID 18529 - TryAdvanceNextDropdownList will potentially send
	// NXM_START_EDITING_EMR_TABLE which will fire this function, which will start editing the
	// row and column in question
	//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
	// (a.walling 2008-10-02 09:16) - PLID 31564 - VS2008 - Message handlers must fit the LRESULT fn(WPARAM, LPARAM) format
	LRESULT OnStartEditingEMRTable(WPARAM wParam, LPARAM lParam);

protected:
	// (c.haag 2008-10-17 11:44) - PLID 31700 - Overload necessary for EmrItemAdvTableBase support
	// (a.walling 2010-06-21 15:07) - PLID 38779 - This is never called with not bCalcOnly
	BOOL RepositionTableControls(IN OUT CSize &szArea);

// Implementation
protected:
	// (c.haag 2011-03-18) - PLID 42891 - Added WindowProc and OnDestroy
	// Generated message map functions
	//{{AFX_MSG(CEmrItemAdvTableDlg)
	afx_msg void OnEditingFinishedTable(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnLButtonUpTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (a.walling 2012-11-06 11:56) - PLID 53609 - Display context menu during datalist ShowContextMenu event
	afx_msg void OnShowContextMenuTable(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, VARIANT_BOOL* pbContinue);
	afx_msg void OnColumnSizingFinishedTable(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	afx_msg void OnEditingFinishingTable(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg LRESULT OnAddNewDropdownColumnSelection(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEditingStartingTable(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnDestroy();
	afx_msg LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	// (a.walling 2012-08-30 07:05) - PLID 51953 - Detect actual VK_TAB keypresses to ignore stuck keys
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EmrItemAdvTableDlg_H__6F27AADE_3B2D_468A_9A66_2FD4A222D0EF__INCLUDED_)