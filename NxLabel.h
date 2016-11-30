#if !defined(AFX_NXLABEL_H)
#define AFX_NXLABEL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NxLabel.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CNxLabel window

// (z.manning, 04/22/2008) - PLID 29745 - Now inherit from CNxStatic
class CNxLabel : public CNxStatic
{
// Construction
public:
	CNxLabel();

// Attributes
public:
	void SetText(const CString &strText);
	CString GetText();
	void SetColor(COLORREF color);
	COLORREF GetColor();
	void SetType(EDrawTextOnDialogStyle dtsType);
	EDrawTextOnDialogStyle GetType();
	// (b.cardillo 2016-06-07 02:45) - NX-100775 - This item needed a way to vertically center even word-wrapped text
	// bForceVertCenter defaults to false for backward compatibility
	void SetSingleLine(bool bSingle = true, bool bForceVertCenter = false);
	void SetHzAlign(DWORD dwType);
	// (c.haag 2014-12-26) - PLID 64255 - Sets the text color override
	void SetTextColorOverride(COLORREF color);
	// (b.cardillo 2016-06-07 02:45) - NX-100775 - We've always needed a way to revert the override
	void ClearTextColorOverride();
	//(e.lally 2009-12-03) PLID 36001
	void AskParentToRedrawWindow();
	
// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxLabel)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNxLabel();
	
protected:
	COLORREF m_color;
	COLORREF m_textColorOverride;
	bool m_bUseTextColorOverride;
	CString m_strText;
	EDrawTextOnDialogStyle m_dtsType;
	bool m_bSingleLine;
	// (b.cardillo 2016-06-07 02:45) - NX-100775 - This item needed a way to vertically center even word-wrapped text
	bool m_bForceVCenter;
	DWORD m_dwHzAlign;		//DRT 4/2/2008 - PLID 29526

	// (a.walling 2009-04-06 17:00) - PLID 33870 - Handle doubleclicks, otherwise this would eat the message. 
	//{{AFX_MSG(CNxLabel)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NXLABEL_H)
