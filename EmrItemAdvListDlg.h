#if !defined(AFX_EMRITEMADVLISTDLG_H__6F27AADE_3B2D_468A_9A66_2FD4A222D0EF__INCLUDED_)
#define AFX_EMRITEMADVLISTDLG_H__6F27AADE_3B2D_468A_9A66_2FD4A222D0EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrItemAdvListDlg.h : header file
//

#include "EmrItemAdvDlg.h"

#include <set>

//TES 1/29/2008 - PLID 28673 - //TES 1/29/2008 - PLID 28673 - Define IDCs for any controls we create.  Checkboxes 
// will continue to have a range of 1000-1999, the * labels which may be put next to them will be 2000-2999.
#define MIN_CHECKBOX_IDC	1000
#define MAX_CHECKBOX_IDC	1999
#define MIN_LABEL_IDC		2000
#define MAX_LABEL_IDC		2999
#define MIN_DATA_LABEL_IDC	3000
#define MAX_DATA_LABEL_IDC	3999
#define IDC_ADVLIST_MORE	0xD000

//////////////////////////////////////////////////////////////////////////

enum EEmrItemAdvListType
{
	eialtListSingleSelect,
	eialtListMultiSelect,
};

/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvListDlg dialog

class CEmrItemAdvListDlg : public CEmrItemAdvDlg
{
friend class CReqStateChange;

// Construction
public:
	CEmrItemAdvListDlg(class CEMNDetail *pDetail);

	CArray<long, long> m_arySpawnItems;

	// (a.walling 2011-11-11 11:11) - PLID 46619 - use AxControl
	// (a.walling 2011-11-11 11:11) - PLID 46623 - 'More' label for overflowing list items
	// (a.walling 2011-11-11 11:11) - PLID 46633 - Use windowless controls where possible to avoid exhausting the desktop heap with GDI and USER objects
	NxWindowlessLib::NxFreeLabelControl m_overflowButton;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrItemAdvListDlg)
	//}}AFX_VIRTUAL

public:
	virtual BOOL RepositionControls(IN OUT CSize &szArea, BOOL bCalcOnly);

	
	// (a.walling 2011-11-11 11:11) - PLID 46622 - Emr list item positioning improvements
	BOOL RepositionControlsCalcOnly(CDC& dc, IN OUT CSize &szArea, CRect& rLabel);
	BOOL RepositionControlsForDisplay(CDC& dc, IN OUT CSize &szArea, CRect& rLabel);

	virtual void ReflectCurrentContent();
	virtual void DestroyContent();

	//TES 3/23/2010 - PLID 37757 - This dialog doesn't maintain its own ReadOnly flag, so I changed the function name from
	// SetReadOnly() to ReflectReadOnlyStatus()
	virtual void ReflectReadOnlyStatus(BOOL bReadOnly);

	// (a.walling 2012-12-11 09:44) - PLID 53988 - Try to reflect current state, and recreate content if necessary due to state
	void ReflectCurrentState();	
	bool TryReflectCurrentState();

	BOOL DataElementPopUp(CPoint pt);

public:
	EEmrItemAdvListType m_eeialtType;

public:
	COLORREF GetHighlightColor();
	// Pass 0 to clear the hilight color
	void ChangeHilightColor(COLORREF clrNewColor);
	COLORREF m_clrHilightColor;

// Implementation
protected:
	CArray<CWnd*,CWnd*> m_arypControls;

	// (a.walling 2012-12-03 12:50) - PLID 53983 - Keep track of expanded labels
	std::set<long> m_expandedLabelIDs;

protected:
	BOOL m_bRequestingStateChange;

	// (a.walling 2011-11-11 11:11) - PLID 46623 - 'More' label for overflowing list items
	int m_nFirstOverflowControlID;

protected:
	// Generated message map functions
	//{{AFX_MSG(CEmrItemAdvListDlg)
	
	BOOL OnButtonClickedEvent(UINT nID);
	BOOL OnLabelClickedEvent(UINT nID);

	// (a.walling 2011-11-11 11:11) - PLID 46623 - 'More' label for overflowing list items
	void OnMoreButtonClicked();
	void PopupAllElements();

	afx_msg void OnButtonClicked(UINT nID);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRITEMADVLISTDLG_H__6F27AADE_3B2D_468A_9A66_2FD4A222D0EF__INCLUDED_)
