#if !defined(AFX_EYEDRAWDLG_H__56430065_F26E_49C2_9308_AD387495C996__INCLUDED_)
#define AFX_EYEDRAWDLG_H__56430065_F26E_49C2_9308_AD387495C996__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EyeDrawDlg.h : header file
//

#include "DrawEvent.h"

/////////////////////////////////////////////////////////////////////////////
// CEyeDrawDlg dialog

class CEyeDrawDlg : public CNxDialog
{
// Construction
public:

	CString m_strFileName;
	void ClearDrawItems();
	int m_iFileSize;				//number of DrawEvents in the file (the m_aryDrawEvents.GetSize())
	bool CanUndo();
	void SwitchBitmaps(int nID);
	bool m_bDrawMode;				//are we drawing?
	COLORREF m_nCurForeColor;		//current color
	CDrawEvent *m_pCurDrawEvent;	//current brush stroke
	CObArray m_aryDrawEvents;		//array of all brush strokes
	int m_iPenSize;					//current pen size

	void Load();
	void Save();

	CDC *m_dc, m_memdc;				//handles to saved displays
	CPen *m_pen,*m_OldPen;			//handles to saved pens

	void StartDraw(UINT nFlags, CPoint point);
	void ContinueDraw(UINT nFlags, CPoint point);
	void EndDraw();

	CPoint m_ptLastPoint;			//the last point we drew on
	CRect m_rcImageRect;			//the image rectangle

	CEyeDrawDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEyeDrawDlg)
	enum { IDD = IDD_EYE_DRAW_DLG };
	CNxStatic	m_Image;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEyeDrawDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEyeDrawDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnColor();
	afx_msg void OnUndo();
	afx_msg void OnSmall();
	afx_msg void OnMedium();
	afx_msg void OnLarge();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EYEDRAWDLG_H__56430065_F26E_49C2_9308_AD387495C996__INCLUDED_)
