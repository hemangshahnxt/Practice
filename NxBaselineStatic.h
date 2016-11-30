// NxBaselineStatic.h: interface for the CNxBaselineStatic class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXBASELINESTATIC_H__995CEBCC_2346_4531_B34C_0DFDF6629B41__INCLUDED_)
#define AFX_NXBASELINESTATIC_H__995CEBCC_2346_4531_B34C_0DFDF6629B41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// (z.manning, 04/22/2008) - PLID 29745 - Now inherit from CNxStatic
class CNxBaselineStatic : public CNxStatic  
{
// Construction
public:
	CNxBaselineStatic();

// Attributes
public:
	void SetPenColor(COLORREF clr);
	void SetPenStyle(long iStyle);
	void SetTextOffset(const CPoint& pt);
	void SetClipBounds(const CRect& rc);
	
public:
	long GetTrueYPos();
	void SetTrueYPos(long nPos);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxBaselineStatic)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNxBaselineStatic();

	// Generated message map functions
protected:
	CPen* m_pPen;
	CFont* m_pFont;
	CPoint m_ptTextOffset;
	COLORREF m_clrPen;
	CRect m_rcClip;
	int m_iPenStyle;

protected:
	//       | 
	// Point |+----                   <-- True Y
	//       |     -
	//       |      ---- Label        <-- Label Y
	//

	// (c.haag 2005-11-25 12:53) - PLID 16976 - We store the
	// true Y position of this baseline in the graph as an integer
	// which describes its position in the graph window.
	long m_nTrueYPos;

protected:
	void UpdatePen();

	//{{AFX_MSG(CNxBaselineStatic)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_NXBASELINESTATIC_H__995CEBCC_2346_4531_B34C_0DFDF6629B41__INCLUDED_)
