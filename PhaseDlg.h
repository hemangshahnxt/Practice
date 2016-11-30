#if !defined(AFX_PHASEDLG_H__19A9FA82_DC24_4A6B_AC40_521CB639632E__INCLUDED_)
#define AFX_PHASEDLG_H__19A9FA82_DC24_4A6B_AC40_521CB639632E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PhaseDlg.h : header file
//
#include "client.h"

/////////////////////////////////////////////////////////////////////////////
// CPhaseDlg dialog

class CPhaseDlg : public CNxDialog
{
	friend class CAdminView; // (a.walling 2006-11-21 16:03) - PLID 3897 - So the view can access GetLadder()
// Construction
public:
	CPhaseDlg(CWnd* pParent);

	virtual void Refresh();

	// (z.manning, 04/16/2008) - PLID 29566 - Set button styles
// Dialog Data
	//{{AFX_DATA(CPhaseDlg)
	enum { IDD = IDD_PHASE_DLG };
	NxButton	m_btnAutoCreate;
	CNxIconButton	m_btnEditStep;
	CNxIconButton	m_editStatusBtn;
	CNxIconButton	m_newLadderBtn;
	CNxIconButton	m_deleteLadderBtn;
	CNxIconButton	m_deleteBtn;
	CNxIconButton	m_newBtn;
	CNxIconButton	m_downBtn;
	CNxIconButton	m_upBtn;
	CNxIconButton	m_newCopyBtn;
	CNxIconButton	m_btnRename;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhaseDlg)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_ladder, 
					m_list;
	CTableChecker	m_ladderChecker, m_eventsChecker;

	// (z.manning, 9/2/2008) - PLID 26886 - Variable to remember selected ladder ID
	long m_nLadderID;

	//Sets the text and enabled state of the Auto Create Ladder checkbox.
	void EnsureCheckBox();

	bool GetStepOrder(long &nStepOrder);
	long GetMaxStep();
	bool GetLadder(long &nLadderID);
	//Verifies that the step orders are a standard index sort of setup.
	void CleanStepOrder(long nLadderID);
	// Generated message map functions
	//{{AFX_MSG(CPhaseDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnNewStep();
	afx_msg void OnDeleteStep();
	afx_msg void OnUp();
	afx_msg void OnDown();
	afx_msg void OnDblClickCellList(long nRowIndex, short nColIndex);
	afx_msg void OnNewLadder();
	afx_msg void OnDeleteLadder();
	afx_msg void OnEditStatus();
	afx_msg void OnRename();
	afx_msg void OnEditStep();
	afx_msg void OnNewCopy();
	afx_msg void OnDragEndList(long nRow, short nCol, long nFromRow, short nFromCol, long nFlags);
	afx_msg void OnRequeryFinishedList(short nFlags);
	afx_msg void OnLeftClickList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnAutoCreateLadder();
	afx_msg void OnSelChosenLadder(long nRow);
	afx_msg void OnRequeryFinishedLadder(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHASEDLG_H__19A9FA82_DC24_4A6B_AC40_521CB639632E__INCLUDED_)
