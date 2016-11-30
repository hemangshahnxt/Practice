#if !defined(AFX_NXSLIDERCTRL_H__E40A2A79_284E_4BED_B936_AC7277078F7A__INCLUDED_)
#define AFX_NXSLIDERCTRL_H__E40A2A79_284E_4BED_B936_AC7277078F7A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NxSliderCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNxSliderCtrl window

class CNxSliderCtrl : public CSliderCtrl
{
// Construction
public:
	CNxSliderCtrl();

// Attributes
public:
	BOOL m_bAutoThumbDrag;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxSliderCtrl)
	public:
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNxSliderCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CNxSliderCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NXSLIDERCTRL_H__E40A2A79_284E_4BED_B936_AC7277078F7A__INCLUDED_)
