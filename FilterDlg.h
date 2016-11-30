#if !defined(AFX_FILTERDLG_H__23BC0ED3_4DE4_11D4_957C_00C04F4C8415__INCLUDED_)
#define AFX_FILTERDLG_H__23BC0ED3_4DE4_11D4_957C_00C04F4C8415__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FilterDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg dialog

#include "Filter.h"

// These must be related in that you must make sure the dialog 
// height fits ITEM_COUNT_PER_PAGE items each of ITEM_SIZE_VERT closely
#define ITEM_SIZE_VERT				21
#define ITEM_COUNT_PER_PAGE		9

// Logically this should always be 5 because it should take 5 clicks 
// on the up or down scroll to move the distance of a single item
#define SCROLL_POS_PER_ITEM		5

// Handy Automated Calculations
#define SCROLL_POS_PER_PAGE		((long)(ITEM_COUNT_PER_PAGE * SCROLL_POS_PER_ITEM))
#define SCROLL_POS_HEIGHT			((long)(ITEM_SIZE_VERT / SCROLL_POS_PER_ITEM))
#define SCROLL_TOP_POS				((long)0)
#define SCROLL_BOTTOM_POS			((long)(m_nItemCount * SCROLL_POS_PER_ITEM - SCROLL_POS_PER_PAGE + 1))


class CFilterDlg : public CDialog, public CFilter
{
	friend class CFilterDetailDlg;
	friend class CFilterEditDlg;

	DECLARE_DYNAMIC(CFilterDlg)

// Construction
public:
	CFilterDlg(long nFilterId, CWnd* pParent, long nFilterBase, BOOL (WINAPI *pfnIsActionSupported)(SupportedActionsEnum, long), BOOL (WINAPI* pfnCommitSubfilterAction)(SupportedActionsEnum, long, long&, CString&, CString&, CWnd*), BOOL (WINAPI* pfnGetNewFilterString)(long, long, CString&, LPCTSTR, CString&));   // standard constructor
	~CFilterDlg();   // standard destructor

// Dialog Data
	//{{AFX_DATA(CFilterDlg)
	enum { IDD = IDD_FILTER_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFilterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

public:
	enum EnumDetailFocusPos {
		dfpFieldType,
		dfpOperator,
		dfpValue
	};

public:
	BOOL SetDetailFocus(long nDetailIndex, EnumDetailFocusPos nFocusPos);

public:
	virtual CFilterDetail *CreateNewDetail();
	virtual void Refresh();
	void FormatFieldNameApparent(CString &strFieldNameApparent);

// Implementation
protected:
	long DoScrollTo(long nNewTopPos);
	long m_nScrolledHeight;

	void UpdatePreviousUseOrAfter(long nPreviousToThisDetailIndex);

	// Generated message map functions
	//{{AFX_MSG(CFilterDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILTERDLG_H__23BC0ED3_4DE4_11D4_957C_00C04F4C8415__INCLUDED_)
