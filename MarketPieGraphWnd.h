#if !defined(AFX_MARKETPIEGRAPHWND_H__525BDF23_90F0_4F12_8469_02411AFC3464__INCLUDED_)
#define AFX_MARKETPIEGRAPHWND_H__525BDF23_90F0_4F12_8469_02411AFC3464__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketPieGraphWnd.h : header file
//
// (c.haag 2008-04-17 17:24) - PLID 29704 - Initial implementation for GDI+ drawing
// and non-GDI-specific functionality
//
// (c.haag 2008-04-17 17:26) - PLID 29705 - Initial implementation for standard GDI
// drawing

/////////////////////////////////////////////////////////////////////////////
// CMarketPieGraphWnd window

class CMarketPieGraphWnd : public CWnd
{
// Construction
public:
	CMarketPieGraphWnd();

// Attributes
private:
	// The number of bits per pixel when the screen resolution was
	// last checked
	unsigned long m_BitsPerPel;

private:
	// Title text
	CString m_strTitleText;

private:
	// The number of vertical pixels a legend label requires room for
	int m_nLegendVSpace;

private:
	// Data values and colors
	CArray<long,long> m_anValues;
	CStringArray m_astrLabels;
	CArray<COLORREF,COLORREF> m_aColors;

// Operations
private:
	// (c.haag 2008-04-17 17:42) - PLID 29705 - Returns the number of bits-per-pixel
	// rendered on the current display (taken from the NexPDA link)
	unsigned long GetBitsPerPel();

private:
	// (c.haag 2008-05-20 10:14) - PLID 29705 - Returns TRUE if we can use enhanced drawing
	// with GDI+, or FALSE if not.
	BOOL UseEnhancedDrawing(CDC* pDC);

public:
	// (c.haag 2008-04-17 17:36) - PLID 29704 - Sets the title text at the top of the window
	void SetTitleText(const CString& str);

public:
	// (c.haag 2008-04-17 17:36) - PLID 29704 - Clears the pie chart data values
	void Clear();
	// (c.haag 2008-04-17 17:36) - PLID 29704 - Adds a data value to the pie chart
	void Add(long nValue, const CString& strLabel);

public:
	// (c.haag 2008-04-18 12:28) - PLID 29704 - Exposed drawing to outside callers
	void Draw(CDC* pDC, const CRect& rcBounds);

protected:
	// (c.haag 2008-04-17 17:36) - PLID 29705 - Draws the title text
	void DrawTitleText(CDC* pDC, const CRect& rcBounds);
	// (c.haag 2008-04-17 17:36) - PLID 29704 - Draws the title text using GDI+
	void DrawTitleTextEnhanced(CDC* pDC, const CRect& rcBounds);

protected:
	// (c.haag 2008-04-17 17:37) - PLID 29705 - Draws the pie
	void DrawPie(CDC* pDC, const CRect& rcBounds);
	// (c.haag 2008-04-17 17:37) - PLID 27904 - Draws the pie using GDI+
	void DrawPieEnhanced(CDC* pDC, const CRect& rcBounds);

protected:
	// (c.haag 2008-04-17 17:37) - PLID 29705 - Draws the legend
	void DrawLegend(CDC* pDC, const CRect& rcBounds, int nFirstValue, int nLastValue);
	// (c.haag 2008-04-17 17:37) - PLID 27904 - Draws the legend using GDI+
	void DrawLegendEnhanced(CDC* pDC, const CRect& rcBounds, int nFirstValue, int nLastValue);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketPieGraphWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMarketPieGraphWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMarketPieGraphWnd)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MARKETPIEGRAPHWND_H__525BDF23_90F0_4F12_8469_02411AFC3464__INCLUDED_)
