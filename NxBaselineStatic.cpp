// NxBaselineStatic.cpp: implementation of the CNxBaselineStatic class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxBaselineStatic.h"

// (z.manning, 04/22/2008) - PLID 29745 - Now inherit from CNxStatic

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNxBaselineStatic::CNxBaselineStatic()
{
	m_pFont = new CFont;
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	if (CreateCompatiblePointFont(m_pFont, 90, "Arial"))
	{
		LOGFONT logfont;
		m_pFont->GetLogFont( &logfont );
		delete m_pFont;
		logfont.lfWeight = FW_BOLD;
		m_pFont = new CFont;
		m_pFont->CreateFontIndirect( &logfont );
	}
	m_pPen = NULL;
	m_clrPen = RGB(0,0,0);
	m_iPenStyle = PS_SOLID;
	m_ptTextOffset = CPoint(22,0);
	UpdatePen();
}

CNxBaselineStatic::~CNxBaselineStatic()
{
	if (m_pPen) delete m_pPen;
	if (m_pFont) delete m_pFont;
}

BEGIN_MESSAGE_MAP(CNxBaselineStatic, CNxStatic)
	//{{AFX_MSG_MAP(CNxBaselineStatic)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNxBaselineStatic message handlers

void CNxBaselineStatic::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CPen* pOldPen = dc.SelectObject(m_pPen);
	CFont* pOldFont = dc.SelectObject(m_pFont);
	CPoint ptLine(0, m_nTrueYPos);
	CRect rc, rcClip = m_rcClip;
	CString str;

	GetWindowText(str);
	GetClientRect(&rc);
	ScreenToClient(&rcClip);
	rcClip.right += 1000;
	dc.IntersectClipRect(&rcClip);
	//ScreenToClient(&ptLine);
	dc.MoveTo(0, ptLine.y);
	dc.LineTo(6, ptLine.y);
	dc.LineTo(12, (rc.top + rc.bottom) / 2);
	dc.LineTo(18, (rc.top + rc.bottom) / 2);
	
	rc += m_ptTextOffset;
	dc.SetBkMode(OPAQUE);
	dc.DrawText(str, &rc, DT_END_ELLIPSIS | DT_SINGLELINE | DT_TOP);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldPen);
}

void CNxBaselineStatic::SetPenColor(COLORREF clr)
{
	m_clrPen = clr;
	UpdatePen();
}

void CNxBaselineStatic::SetPenStyle(long iStyle)
{
	m_iPenStyle = iStyle;
	UpdatePen();
}

long CNxBaselineStatic::GetTrueYPos()
{
	return m_nTrueYPos;
}

void CNxBaselineStatic::SetTrueYPos(long nPos)
{
	m_nTrueYPos = nPos;
}

void CNxBaselineStatic::UpdatePen()
{
	if (m_pPen) delete m_pPen;
	m_pPen = new CPen(m_iPenStyle, m_iPenStyle == PS_DASH ? 1 : 2, m_clrPen);
}

void CNxBaselineStatic::SetTextOffset(const CPoint& pt)
{
	m_ptTextOffset = pt;
	m_ptTextOffset.x += 22;
}

void CNxBaselineStatic::SetClipBounds(const CRect& rc)
{
	m_rcClip = rc;
}