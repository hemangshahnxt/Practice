#if !defined(AFX_EMRITEMADVDLG_H__26D6E8E6_2B43_4D01_9B0D_78E9521D7CD8__INCLUDED_)
#define AFX_EMRITEMADVDLG_H__26D6E8E6_2B43_4D01_9B0D_78E9521D7CD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrItemAdvDlg.h : header file
//

#include <afxdialogex.h>

#include "EMRItemEntryDlg.h"
#include "EmrFonts.h"
#include <AxControl.h>
#include <WindowlessUtils.h>

#import <NxWindowless.tlb>
//#import "libid:9511C7D0-C2F0-4BC8-841A-F980A950A831" version("1.0")

namespace NxWindowlessLib
{
	// (a.walling 2011-11-11 11:11) - PLID 46619 - AxControl - typedefs for the windowless button and label controls
	_AXCONTROL_TYPEDEF(NxFreeButton, __uuidof(NxFreeButton), _DNxFreeButton, __uuidof(_DNxFreeButton));
	_AXCONTROL_TYPEDEF(NxFreeLabel, __uuidof(NxFreeLabel), _DNxFreeLabel, __uuidof(_DNxFreeLabel));
}

// (a.walling 2012-04-02 08:29) - PLID 49304 - Removed a lot of dead code regarding moving items

class CEMNDetail;

#define SIZE_EMR_SNAP_TO_GRID_BLOCK		10

/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvDlg dialog

struct MergeOverrideChanged
{
	CString strOldName;
	CString strNewName;
};

//TES 1/29/2008 - PLID 28673 - Do not use numeric literals for your control IDCs any more.  #define them in your .h file,
// this will help future developers avoid conflicts.  Any "global" controls (like the merge status button) will have
// an ID #defined here, with an ID of 3001 or higher.  All IDs below 3001 are available for use by derived classes.
#define MERGE_STATUS_IDC	3001
#define PROBLEM_STATUS_IDC	3002

// (a.walling 2011-11-11 11:11) - PLID 46634 - Now derived from CWnd
// (a.walling 2012-02-27 12:48) - PLID 48415 - *Now* derived from CNxDialogMenuSupport<CWnd> to inject popup menu handling support
class CEmrItemAdvDlg : public CNxDialogMenuSupport<CWnd>
{
// Construction
public:
	CEmrItemAdvDlg(class CEMNDetail *pDetail);
	~CEmrItemAdvDlg();
	
	// (a.walling 2011-11-11 11:11) - PLID 46634 - Create this window with the given client area
	BOOL CreateWithClientArea(DWORD dwExStyleExtra, DWORD dwStyleExtra, const CRect& rect, CWnd* pParentWnd, UINT nID = -1);

	//TES 3/15/2010 - PLID 37757 - I see no reason this should maintain its own ReadOnly flag, it should just be checking the detail.
	//BOOL m_bReadOnly;
	BOOL m_bGhostly;//This flag means that the dlg should be displayed in such a way as to indicate that it's not really there
					//used by the template editor to display spawned items.
	BOOL m_bCanEditContent; //Are we in "Edit Mode" or not?
	BOOL m_bIsTemplate;	//is it in Template mode?
	BOOL m_bIsLoading;

	// (b.cardillo 2006-11-20 10:37) - PLID 22565 - This returns the correct background color to be 
	// used for this item depending on the value of m_nReviewState.  If there is no special color, 
	// then this returns the sentinel (COLORREF)-1.
	COLORREF GetReviewStateColor();
	//DRT 6/4/2008 - PLID 30269
	HBRUSH GetReviewStateBrush();
	
	bool IsHighColor();
	COLORREF GetGhostlyTextColor();
	COLORREF GetSpawnItemTextColor();

	static COLORREF GetRegularTextColor();

	
	class CEmrFrameWnd* GetEmrFrameWnd() const;

protected:
	//DRT 5/13/2008 - PLID 29771 - These are designed to handle the background coloring needs of this dialog and all
	//	derived elements.  Since there can be a "real" background color, and we also have to worry about the 
	//	review state color, it is safest to do it in 1 function.
	COLORREF GetBackgroundColor();
	HBRUSH GetBackgroundBrush();
	static HBRUSH GetDefaultBackgroundBrush();
	COLORREF GetDefaultParentBackgroundColor();
	static COLORREF GetDefaultBackgroundColor();
	static COLORREF GetBorderColor();

public:
	//TES 3/23/2010 - PLID 37757 - This dialog doesn't maintain its own ReadOnly flag, so I changed the function name from
	// SetReadOnly() to ReflectReadOnlyStatus()
	virtual void ReflectReadOnlyStatus(BOOL bReadOnly);
	virtual void SetGhostly(BOOL bGhostly);
	virtual void SetCanEditContent(BOOL bCanEditContent);
	virtual void SetIsTemplate(BOOL bIsTemplate);

public:
	// (a.walling 2008-01-16 15:35) - PLID 14982 - Moved this to EmrUtils
	//virtual void BuildCurrentTableStateArray(class CEMRTopic* pTopic, long nEMRInfoID, CArray<CEMNDetail*,CEMNDetail*>& apDetails);

public:
	// Pointer to the object that contains all the state data for this item.  The state data is what makes an item a detail.
	class CEMNDetail *m_pDetail;

public:
	virtual void ReflectCurrentState();

public:
	// Adjusts szArea to reflect the best size for this window given the controls it contains.
	// If bCalcOnly is FALSE, this function also arranges and sizes the 
	// controls to fit snuggly in the given area.
	virtual BOOL RepositionControls(IN OUT CSize &szArea, BOOL bCalcOnly);

	virtual void ReflectCurrentContent();
	virtual void DestroyContent();
	virtual void UpdateStatusButtonAppearances();
	// (c.haag 2008-01-22 09:34) - PLID 28686 - We now allow the caller to define special dialog behaviors
	// (a.walling 2008-03-25 08:10) - PLID 28811 - This is now a static function
	static void OpenItemEntryDlg(CEMNDetail* pDetail, BOOL bIsTemplate, EEmrItemEntryDlgBehavior behavior = eEmrItemEntryDlgBehavior_Normal, LPVOID pBehaviorData = NULL);

protected:
	virtual BOOL EnsureMergeStatusButton();
	virtual BOOL EnsureNotMergeStatusButton();

	virtual BOOL EnsureProblemStatusButton();
	virtual BOOL EnsureNotProblemStatusButton();

	//TES 12/14/2006 - PLID 23833 - Sets m_pDetail to use the ActiveEmrInfoID.
	// (a.walling 2008-03-25 08:13) - PLID 28811 - This is now a static function
	static void UpdateItem(CEMNDetail* pDetail);

// Implementation
protected:
	CNxToolTip* m_pToolTipWnd;
	CNxIconButton* m_pBtnMergeStatus;// We show this if we have a potential merge
									// field name conflict, or an overridden merge
									// field name.

	// (c.haag 2006-06-30 15:49) - PLID 19977 - We show this button if this detail is
	// marked as a problem
	CNxIconButton* m_pBtnProblemStatus;

protected:
	// (c.haag 2006-11-14 15:08) - PLID 23158 - This table checker object listens for
	// changes in the EMR problems table
	class CTableChecker* m_pProblemChecker;

protected:
	virtual void UpdateMergeStatusButtonAppearance();

public:
	// (c.haag 2008-07-29 16:03) - PLID 30878 - Optional parameter for repositioning controls. Defaults
	// to FALSE so we don't break legacy code
	virtual void UpdateProblemStatusButtonAppearance(BOOL bRepositionControls = FALSE);

protected:
	BOOL IsMergeButtonVisible() const;
	BOOL IsProblemButtonVisible() const;

protected:
	CString GetToolTipText();
	// (b.cardillo 2012-05-02 20:28) - PLID 49255 - Added parameter
	// Require the caller to indicate if he wants the official label text, or the text with any special 
	// modifiers attached (right now the only modifier is an asterisk if it's a required detail that 
	// hasn't been filled in)
	CString GetLabelText(BOOL bIncludeSpecialText);

public:
	// (a.walling 2011-11-11 11:11) - PLID 46619 - use AxControl
	// (a.walling 2011-11-11 11:11) - PLID 46633 - Use windowless controls where possible to avoid exhausting the desktop heap with GDI and USER objects
	NxWindowlessLib::NxFreeLabelControl m_wndLabel;

	//TES 6/29/2004: For now this is just here to be turned off by image items, but I figure the occasion will arise that
	//callers will want to turn this off, because of a preference or whatnot.
	bool m_bEnableTooltips;

	//old emr only
	// (c.haag 2004-07-01 12:01) - This tracks the visibility of our merge status
	// button.
	bool m_bShowMergeStatusButton;

	// This is used to prevent constant reloading to prevent flickering when moving details.
	BOOL m_bPreventContentReload;

	void SetMergeNameConflict(BOOL bConflicts);

	// (c.haag 2009-09-10 17:31) - PLID 35077 - Made public
public:
	// (c.haag 2006-06-29 15:10) - PLID 19977 - We can now associate a detail with a problem
	// (j.jones 2008-07-17 15:17) - PLID 30729 - now a detail can have multiple problems,
	// either linked to the detail or a list item, 
	void EditProblem(EMRProblemRegardingTypes eprtType, long nEMRDataID = -1);
	// (j.jones 2008-07-21 08:36) - PLID 30779 - added NewProblem because we can now have multiple
	// problems per detail
	void NewProblem(EMRProblemRegardingTypes eprtType, long nEMRDataID = -1);
	// (c.haag 2009-05-27 14:41) - PLID 34249 - This function is called when the user wants to link this EMR object
	// with one or more existing problems
	void LinkProblems(EMRProblemRegardingTypes eprtType, long nEMRDataID = -1);

protected:
	// (c.haag 2006-03-22 09:22) - PLID 19786 - True if the controls need to be
	// repositioned. At the time of this comment, only CEmrTopicWnd::RepositionDetailsInTopic
	// calls GetNeedToRepositionControls. In future releases, we may want to use this in
	// other places.
	//
	// Also, m_bNeedToRepositionControls is not used to determine whether controls need
	// to be repositioned in :RepositionControls(). If we ever decide to use it, please
	// make an annotation after this comment, and VERY CAREFULLY implement it.
	//
	BOOL m_bNeedToRepositionControls;

public:
	// (j.jones 2008-08-15 14:43) - PLID 30779 - pass in a value determining if this is called during a save
	BOOL CheckForProblemDataChanges(BOOL bIsSaving = FALSE);

	//TES 10/30/2008 - PLID 31269 - Call after a problem gets changed to refresh all the problem icons.
	virtual void HandleProblemChange(CEmrProblem* pChangedProblem);

public:
	inline BOOL GetNeedToRepositionControls() const { return m_bNeedToRepositionControls; }
	inline void SetNeedToRepositionControls(BOOL bNeedToReposition) { m_bNeedToRepositionControls = bNeedToReposition; }

	
	// (a.walling 2012-04-03 15:20) - PLID 49377 - Only one instance will move at a time
	static CEmrItemAdvDlg* g_pMovingDlg;
	static CEmrItemAdvDlg* g_pSizingDlg;

public:

protected:
	CRect m_rcLastLButtonDownWindowAdj;

public:
	// (a.walling 2012-03-05 15:56) - PLID 46075 - Auto deletes when destroyed
	void SetAutoDelete(bool bAutoDelete)
	{
		m_bAutoDelete = bAutoDelete;
	}

protected:
	bool m_bAutoDelete;

	// (a.walling 2012-03-05 15:56) - PLID 46075 - Auto deletes when destroyed
	virtual void PostNcDestroy();

protected:
	BOOL m_bMergeNameConflicts;

	//DRT 4/24/2008 - PLID 29771 - Brush for the background
	//CBrush m_brBackground;

	// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - Needed LRESULT return for OnNcHitTest
	// Generated message map functions
	//{{AFX_MSG(CEmrItemAdvDlg)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT nSide, LPRECT lpRect);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMoving(UINT nSide, LPRECT lpRect);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	// (a.walling 2012-03-29 08:04) - PLID 49297 - Custom handling of the SC_SIZE syscommand
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point);
	// (b.savon 2012-06-01 11:15) - PLID 49351 - Handle Right Click properly in Edit Mode
	afx_msg void OnNcRButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnButtonClickedMergeConflict();
	afx_msg void OnButtonClickedProblem();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvents);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
	afx_msg LRESULT OnShowTopic(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// (a.walling 2011-11-11 11:11) - PLID 46632 - WindowlessUtils - forward extra messages to windowless controls
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRITEMADVDLG_H__26D6E8E6_2B43_4D01_9B0D_78E9521D7CD8__INCLUDED_)
