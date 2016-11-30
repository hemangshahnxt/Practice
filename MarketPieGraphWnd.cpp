// MarketPieGraphWnd.cpp : implementation file
//
// (c.haag 2008-04-17 17:24) - PLID 29704 - Initial implementation for GDI+ drawing
// and non-GDI-specific functionality
//
// (c.haag 2008-04-17 17:26) - PLID 29705 - Initial implementation for standard GDI
// drawing

#include "stdafx.h"
#include "MarketPieGraphWnd.h"
#include "GlobalDrawingUtils.h"
#include <gdiplus.h>
using namespace Gdiplus;

// Converts degrees to radians
#define RAD(n) ((n) * 3.14159f / 180.0f)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarketPieGraphWnd

CMarketPieGraphWnd::CMarketPieGraphWnd()
{
	// (c.haag 2008-04-24 12:01) - PLID 29704 - Populate with default colors.
	// If the chart requires more, we will unfortunately have to cycle through
	// them. The MSChart control also repeats colors if you input enough values.
	// Colors chosen from http://www.colorjack.com/sphere/
	m_aColors.Add(RGB(0xFF,0xAD,0xB0));
	m_aColors.Add(RGB(0xAD,0xB0,0xFF));
	m_aColors.Add(RGB(0xB0,0xFF,0xAD));
	m_aColors.Add(RGB(0xFF,0xD3,0xAD));
	m_aColors.Add(RGB(0xD3,0xAD,0xFF));
	m_aColors.Add(RGB(0xA0,0xFF,0xD3));

	m_aColors.Add(RGB(0x99,0xD6,0xFF));
	m_aColors.Add(RGB(0xFF,0x99,0xD6));
	m_aColors.Add(RGB(0xD6,0xFF,0x99));
	m_aColors.Add(RGB(0x99,0xA3,0xFF));
	m_aColors.Add(RGB(0xFF,0x99,0xA3));
	m_aColors.Add(RGB(0xA3,0xFF,0x99));

	const int nColors = m_aColors.GetSize();
	for (int i=0; i < nColors; i++) {
		const COLORREF clrCur = m_aColors[i];
		COLORREF clr = RGB(
			((int)GetRValue(clrCur) * 3) / 4,
			((int)GetGValue(clrCur) * 3) / 4,
			((int)GetBValue(clrCur) * 3) / 4
		);
		m_aColors.Add(clr);
	}

	m_BitsPerPel = 0;
	m_nLegendVSpace = 30;
}

CMarketPieGraphWnd::~CMarketPieGraphWnd()
{
}

// (c.haag 2008-05-20 10:14) - PLID 29705 - Returns TRUE if we can use enhanced drawing
// with GDI+, or FALSE if not.
BOOL CMarketPieGraphWnd::UseEnhancedDrawing(CDC* pDC)
{
	if (!GetRemotePropertyInt("DrawPieGraphsWithGradients", 1, 0, GetCurrentUserName(), true)) {
		return FALSE;
	}
	else if (GetBitsPerPel() < 16) {
		return FALSE;
	}
	else if (pDC->IsPrinting()) {
		return FALSE;
	}
	else if (NxGdi::IsRemote()) {
		return FALSE;
	}
	return TRUE;
}

unsigned long CMarketPieGraphWnd::GetBitsPerPel()
{
	// (c.haag 2008-04-17 17:42) - PLID 29705 - Returns the number of bits-per-pixel
	// rendered on the current display (taken from the NexPDA link)
	DEVMODE dm;
	if (m_BitsPerPel == 0) {
		// (c.haag 2008-04-17 17:42) - Cache the resolution so we don't
		// enumerate display settings every time we paint
		EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
		m_BitsPerPel = dm.dmBitsPerPel;
	}
	return m_BitsPerPel;
}

void CMarketPieGraphWnd::SetTitleText(const CString& str)
{
	// (c.haag 2008-04-17 17:36) - PLID 29704 - Sets the title text at the top of the window
	m_strTitleText = str;
}

void CMarketPieGraphWnd::Clear()
{
	// (c.haag 2008-04-17 17:36) - PLID 29704 - Clears the pie chart data values
	m_anValues.RemoveAll();
	m_astrLabels.RemoveAll();
}

void CMarketPieGraphWnd::Add(long nValue, const CString& strLabel)
{
	// (c.haag 2008-04-17 17:36) - PLID 29704 - Adds a data value to the pie chart
	m_anValues.Add(nValue);
	m_astrLabels.Add(strLabel);
}


void CMarketPieGraphWnd::DrawTitleText(CDC* pDC, const CRect& rcBounds)
{
	// (c.haag 2008-04-17 17:36) - PLID 29705 - Draws the title text
	CFont fnt;
	CRect rc(rcBounds);
	fnt.CreatePointFont(160, "Arial Bold", pDC);
	CFont* pOldFont = pDC->SelectObject(&fnt);
	pDC->SetTextColor(RGB(16,16,64));
	pDC->DrawText(m_strTitleText, rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pDC->SelectObject(pOldFont);
	fnt.DeleteObject();
}

void CMarketPieGraphWnd::DrawTitleTextEnhanced(CDC* pDC, const CRect& rcBounds)
{
	// (c.haag 2008-04-17 17:36) - PLID 29704 - Draws the title text using GDI+
	Graphics graphics(pDC->m_hDC);
	_bstr_t bstrFamily("Arial Bold");
	_bstr_t bstrText(m_strTitleText);
	// (a.walling 2008-10-02 09:26) - PLID 31567 - Must state Gdiplus namespace
	Gdiplus::Font fnt(bstrFamily, 16);
	RectF rc((float)rcBounds.left, (float)rcBounds.top, (float)rcBounds.Width(), (float)rcBounds.Height());
	SolidBrush br(Color(255, 16, 16, 64));
	StringFormat format;
	format.SetAlignment(StringAlignmentCenter);
	format.SetLineAlignment(StringAlignmentCenter);
	graphics.DrawString(bstrText, m_strTitleText.GetLength(), &fnt, rc, &format, &br);
}

void CMarketPieGraphWnd::DrawPie(CDC* pDC, const CRect& rcBounds)
{
	// (c.haag 2008-04-17 17:37) - PLID 29705 - Draws the pie
	CRect rcPie = rcBounds;
	int nSumValues = 0;
	float fDeg;
	int iValue;

	// Get the sum of the values
	const int nValues = m_anValues.GetSize();
	for (iValue=0; iValue < nValues; iValue++) {
		nSumValues += m_anValues[iValue];
	}

	// Calculate the height of the 3D pie, and reduce the pie's ellipse height
	// by both that and the shadow depth so that it marginally fits in rcBounds
	float hPie = (float)rcPie.Height() / 10.0f; // Height of the 3D pie
	rcPie.bottom -= (int)hPie;

	CPoint ptPieCenter = rcPie.CenterPoint();
	float cx = (float)ptPieCenter.x; // Center of the pie
	float cy = (float)ptPieCenter.y;
	float rx = (float)rcPie.Width()/2; // Radius of the pie
	float ry = (float)rcPie.Height()/2;

	// Now draw the pie
	for (iValue=0, fDeg = 90; iValue < nValues; iValue++) {

		// Get the size of the slice in degrees
		float fSweep = (float)m_anValues[iValue] * 360.0f / (float)nSumValues;

		// Get other variables
		float tFirst = fDeg; // Starting degree
		float tLast = fDeg + fSweep; // Ending degree
		float tInterval = 3.0f; // Number of degrees per slice
		float t; // Loop counter

		// Get the main color
		unsigned char r = GetRValue(m_aColors[iValue % m_aColors.GetSize()]);
		unsigned char g = GetGValue(m_aColors[iValue % m_aColors.GetSize()]);
		unsigned char b = GetBValue(m_aColors[iValue % m_aColors.GetSize()]);
		int h,s,v;
		RGB2HSV(r,g,b,h,s,v);
		// Increase the global saturation for more defined colors
		s = min(100,(s*4)/3);

		// Calculate the bounding box of the pie slice
		CRect rcSliceBounds(ptPieCenter.x,ptPieCenter.y,ptPieCenter.x,ptPieCenter.y);
		for (t = tFirst; t < tLast + tInterval; t += tInterval) {
			float t1 = t;
			float t2 = min(tLast, t + tInterval);
			if (t1 < t2) {
				CPoint ptPie[2] = {
					CPoint((int)(cx + (rx * cosf(RAD(t1)))), (int)(cy - (ry * sinf(RAD(t1))))),
					CPoint((int)(cx + (rx * cosf(RAD(t2)))), (int)(cy - (ry * sinf(RAD(t2))))),
				};
				rcSliceBounds.left = min(rcSliceBounds.left, min(ptPie[0].x, ptPie[1].x));
				rcSliceBounds.top = min(rcSliceBounds.top, min(ptPie[0].y, ptPie[1].y));
				rcSliceBounds.right = max(rcSliceBounds.right, max(ptPie[0].x, ptPie[1].x));
				rcSliceBounds.bottom = max(rcSliceBounds.bottom, max(ptPie[0].y, ptPie[1].y));
			}
		}

		// Build the pie brushes
		unsigned char rPie1, gPie1, bPie1;
		unsigned char rPie2, gPie2, bPie2;
		HSV2RGB(h,s,(v*3)/4, rPie1,gPie1,bPie1);
		HSV2RGB(h,s,v, rPie2,gPie2,bPie2);
		CBrush brPie(RGB(rPie2,gPie2,bPie2));
		CBrush brDepth(RGB(rPie1,gPie1,bPie1));
		CPen penPie(PS_SOLID, 1, RGB(rPie2,gPie2,bPie2));
		CPen penDepth(PS_SOLID, 1, RGB(rPie1,gPie1,bPie1));
		CBrush* pbrOld;
		CPen* ppenOld;

		for (t = tFirst; t < tLast + tInterval; t += tInterval) {
			float t1 = t;
			float t2 = min(tLast, t + tInterval);

			if (t1 < t2) {

				// Draw the pie slice from t1 to t2
				POINT ptPie[3] = {
					{ (int)( cx + (rx * cosf(RAD(t1))) ), (int)( cy - (ry * sinf(RAD(t1))) ) },
					{ (int)( cx + (rx * cosf(RAD(t2))) ), (int)( cy - (ry * sinf(RAD(t2))) ) },
					{ (int)cx, (int)cy },
				};
				pbrOld = pDC->SelectObject(&brPie);
				ppenOld = pDC->SelectObject(&penPie);
				pDC->Polygon(ptPie, 3);

				// Draw a height bar from t1 to t2 with height h. We only need to draw when either
				// t1 or t2 are in the range [180,360)
				if ((t1 < tLast) && ((t1 >= 180.0f && t1 < 360.0f) || (t2 >= 180.0f && t2 < 360.0f))) {
					t1 = max(t1,180.0f);
					t2 = min(t2,360.0f);
					
					POINT ptHeight[4] = {
						{ (int)( cx + (rx * cosf(RAD(t1))) ), (int)( cy - (ry * sinf(RAD(t1))) ) },
						{ (int)( cx + (rx * cosf(RAD(t1))) ), (int)( cy - (ry * sinf(RAD(t1))) + hPie ) },
						
						{ (int)( cx + (rx * cosf(RAD(t2))) ), (int)( cy - (ry * sinf(RAD(t2))) + hPie ) },
						{ (int)( cx + (rx * cosf(RAD(t2))) ), (int)( cy - (ry * sinf(RAD(t2))) ) },
					};
					pDC->SelectObject(&brDepth);
					pDC->SelectObject(&penDepth);
					pDC->Polygon(ptHeight, 4);
				}

				pDC->SelectObject(pbrOld);
				pDC->SelectObject(ppenOld);

			} // if (t1 < t2) {

		} // for (t = tFirst; t < tLast + tInterval; t += tInterval) {

		fDeg += fSweep;

	} // for (iValue=0, fDeg = 90; iValue < nValues; iValue++) {
}

void CMarketPieGraphWnd::DrawPieEnhanced(CDC* pDC, const CRect& rcBounds)
{
	// (c.haag 2008-04-17 17:37) - PLID 27904 - Draws the pie using GDI+
	Graphics graphics(pDC->m_hDC);
	Rect rcPie(rcBounds.left, rcBounds.top, rcBounds.Width(), rcBounds.Height());
	Point ptPieCenter;
	int nSumValues = 0;
	int nShadowDepth = 6;
	int nShadowStepHeight = 2;
	float fDeg;
	int iValue;

	// Get the sum of the values
	const int nValues = m_anValues.GetSize();
	for (iValue=0; iValue < nValues; iValue++) {
		nSumValues += m_anValues[iValue];
	}

	// Calculate the height of the 3D pie, and reduce the pie's ellipse height
	// by both that and the shadow depth so that it marginally fits in rcBounds
	float hPie = (float)rcPie.Height / 10.0f; // Height of the 3D pie
	rcPie.Height -= (int)hPie;
	rcPie.Height -= nShadowDepth * nShadowStepHeight;

	// Calculate the pie center
	ptPieCenter.X = rcPie.X + rcPie.Width/2;
	ptPieCenter.Y = rcPie.Y + rcPie.Height/2;

	// Draw the shadow
	for (iValue=6; iValue >= 0; iValue--) {
		int nIntensity = min(255, 170 + iValue * 12);
		COLORREF clr = RGB(nIntensity, nIntensity, nIntensity);
		CBrush br(clr);
		CPen pen(PS_SOLID, 1, clr);
		CBrush* pOldBrush = (CBrush*)pDC->SelectObject(&br);
		CPen* pOldPen = (CPen*)pDC->SelectObject(&pen);
		CRect rcEllipse(
			rcPie.X - iValue,
			rcPie.Y + (long)hPie + 2 + iValue*2,
			rcPie.X + rcPie.Width + iValue,
			rcPie.Y + rcPie.Height + (long)hPie + 2 + iValue*2
			);
		pDC->Ellipse(rcEllipse);
		pDC->SelectObject(pOldBrush);
		pDC->SelectObject(pOldPen);
	}

	// Now draw the pie
	for (iValue=0, fDeg = 90; iValue < nValues; iValue++) {

		// Get the size of the slice in degrees
		// (c.haag 2008-04-18 14:41) - PLID 29704 - Sweep by 361 degrees instead. This is because
		// if we have a full 360 sweep, there will be an open line that doesn't belong at the top
		// of the chart. If we do 361, that line is covered.
		float fSweep = (float)m_anValues[iValue] * 361.0f / (float)nSumValues;
		if (fSweep > 0) {
			// Get other variables
			float tFirst = fDeg; // Starting degree
			float tLast = fDeg + fSweep; // Ending degree
			float tInterval = 3.0f; // Number of degrees per slice
			float cx = (float)ptPieCenter.X; // Center of the pie
			float cy = (float)ptPieCenter.Y;
			float rx = (float)rcPie.Width/2; // Radius of the pie
			float ry = (float)rcPie.Height/2;
			float t; // Loop counter


			// Get the main color
			unsigned char r = GetRValue(m_aColors[iValue % m_aColors.GetSize()]);
			unsigned char g = GetGValue(m_aColors[iValue % m_aColors.GetSize()]);
			unsigned char b = GetBValue(m_aColors[iValue % m_aColors.GetSize()]);
			int h,s,v;
			RGB2HSV(r,g,b,h,s,v);
			// Increase the global saturation for more defined colors
			s = min(100,(s*4)/3);

			// Calculate the bounding box of the pie slice
			CRect rcSliceBounds(ptPieCenter.X,ptPieCenter.Y,ptPieCenter.X,ptPieCenter.Y);
			for (t = tFirst; t < tLast + tInterval; t += tInterval) {
				float t1 = t;
				float t2 = min(tLast, t + tInterval);
				if (t1 < t2) {
					PointF ptPie[2] = {
						PointF(cx + (rx * cosf(RAD(t1))), cy - (ry * sinf(RAD(t1)))),
						PointF(cx + (rx * cosf(RAD(t2))), cy - (ry * sinf(RAD(t2)))),
					};
					rcSliceBounds.left = min(rcSliceBounds.left, (int)min(ptPie[0].X, ptPie[1].X));
					rcSliceBounds.top = min(rcSliceBounds.top, (int)min(ptPie[0].Y, ptPie[1].Y));
					rcSliceBounds.right = max(rcSliceBounds.right, (int)max(ptPie[0].X, ptPie[1].X));
					rcSliceBounds.bottom = max(rcSliceBounds.bottom, (int)max(ptPie[0].Y, ptPie[1].Y));
				}
			}

			// Build the pie brush
			unsigned char rPie1, gPie1, bPie1;
			unsigned char rPie2, gPie2, bPie2;
			HSV2RGB(h,s,(v*3)/4, rPie1,gPie1,bPie1);
			HSV2RGB(h,s,v, rPie2,gPie2,bPie2);
			LinearGradientBrush brushPie(Rect(rcSliceBounds.left, rcSliceBounds.top, rcSliceBounds.Width(), rcSliceBounds.Height()),
				Color(255, rPie1,gPie1,bPie1),
				Color(255, rPie2,gPie2,bPie2),
				90);

			// Build the pie height brush
			HSV2RGB(h,s,v/2, rPie1,gPie1,bPie1);
			HSV2RGB(h,s,(v*3)/4, rPie2,gPie2,bPie2);
			LinearGradientBrush brushDepth(Rect(rcSliceBounds.left, rcBounds.top, rcSliceBounds.Width(), rcBounds.Height()),
				Color(255, rPie1,gPie1,bPie1),
				Color(255, rPie2,gPie2,bPie2),
				180);

			for (t = tFirst; t < tLast + tInterval; t += tInterval) {
				float t1 = t;
				float t2 = min(tLast, t + tInterval);

				if (t1 < t2) {
					// Draw the pie slice from t1 to t2
					PointF ptPie[3] = {
						PointF(cx + (rx * cosf(RAD(t1))), cy - (ry * sinf(RAD(t1)))),
						PointF(cx + (rx * cosf(RAD(t2))), cy - (ry * sinf(RAD(t2)))),
						PointF(cx, cy)
					};
					graphics.FillPolygon(&brushPie, ptPie, 3);

					// Draw a height bar from t1 to t2 with height h. We only need to draw when either
					// t1 or t2 are in the range [180,361]
					if ((t1 <= tLast) && ((t1 >= 180.0f && t1 <= 361.0f) || (t2 >= 180.0f && t2 <= 361.0f))) {
						t1 = max(t1,180.0f);
						t2 = min(t2,361.0f);
						
						PointF ptHeight[4] = {
							PointF(cx + (rx * cosf(RAD(t1))), cy - (ry * sinf(RAD(t1)))),
							PointF(cx + (rx * cosf(RAD(t1))), cy - (ry * sinf(RAD(t1))) + hPie),
							
							PointF(cx + (rx * cosf(RAD(t2))), cy - (ry * sinf(RAD(t2))) + hPie),
							PointF(cx + (rx * cosf(RAD(t2))), cy - (ry * sinf(RAD(t2))))
						};
						graphics.FillPolygon(&brushDepth, ptHeight, 4);
					}

				} // if (t1 < t2) {

			} // for (t = tFirst; t < tLast + tInterval; t += tInterval) {

			fDeg += fSweep;

		} // if (fSweep > 0) {

	} // for (iValue=0, fDeg = 90; iValue < nValues; iValue++) {
}

void CMarketPieGraphWnd::DrawLegend(CDC* pDC, const CRect& rcBounds, int nFirstValue, int nLastValue)
{
	// (c.haag 2008-04-17 17:37) - PLID 29705 - Draws the legend
	const int nValues = m_anValues.GetSize();
	CSize sColor(16,16);
	int iValue;
	int nHSpaceAfterBox = 8;
	int nLegendVSpace = m_nLegendVSpace;
	CFont fnt;
	fnt.CreatePointFont(80, "Arial", pDC);
	CFont* pOldFont = pDC->SelectObject(&fnt);
	int nOldBkMode = pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(16,16,64));

	nLastValue = min(nLastValue,nValues);

	if (pDC->IsPrinting()) {
		sColor.cx = 160;
		sColor.cy = 160;
		nHSpaceAfterBox *= 10;
		nLegendVSpace *= 10;
	}

	int y = (rcBounds.top + rcBounds.bottom) / 2 - ((nLastValue - nFirstValue) * nLegendVSpace) / 2;

	// Do for all data values
	for (iValue=nFirstValue; iValue < nLastValue; iValue++, y += nLegendVSpace) {
		CRect rcColor(rcBounds.left, y,
			rcBounds.left + sColor.cx, y + sColor.cy);
		CRect rcLabel(rcColor.right + nHSpaceAfterBox, rcColor.top,
			rcBounds.right, rcColor.bottom);

		// Draw the box
		pDC->FillSolidRect(rcColor, m_aColors[iValue % m_aColors.GetSize()]);

		// Draw the label
		pDC->DrawText(m_astrLabels[iValue], rcLabel, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	}

	pDC->SelectObject(pOldFont);
	pDC->SetBkMode(nOldBkMode);
	fnt.DeleteObject();
}

void CMarketPieGraphWnd::DrawLegendEnhanced(CDC* pDC, const CRect& rcBounds, int nFirstValue, int nLastValue)
{
	// (c.haag 2008-04-17 17:37) - PLID 27904 - Draws the legend using GDI+
	Graphics graphics(pDC->m_hDC);
	_bstr_t bstrFamily("Arial Bold");
	// (a.walling 2008-10-02 09:26) - PLID 31567 - Must state Gdiplus namespace
	Gdiplus::Font fnt(bstrFamily, 8);
	const int nValues = m_anValues.GetSize();
	CSize sColor(16,16);
	int iValue;
	int nHSpaceAfterBox = 8;
	int nLegendVSpace = m_nLegendVSpace;
	int y = (rcBounds.top + rcBounds.bottom) / 2 - ((nLastValue - nFirstValue) * nLegendVSpace) / 2;
	StringFormat format;

	nLastValue = min(nLastValue,nValues);

	// Do for all data values
	for (iValue=nFirstValue; iValue < nLastValue; iValue++, y += nLegendVSpace) {
		Rect rcColor(rcBounds.left, y, sColor.cx, sColor.cy);
		RectF rcLabel((float)(rcColor.X + rcColor.Width + nHSpaceAfterBox), (float)rcColor.Y,
			(float)(rcBounds.right - (rcColor.X + rcColor.Width + nHSpaceAfterBox)), (float)sColor.cy);
		SolidBrush brText(Color(255, 16, 16, 64));
		COLORREF clrValue = m_aColors[iValue % m_aColors.GetSize()];
		COLORREF clrDark = RGB(
			(((int)GetRValue(clrValue))*3)/4,
			(((int)GetGValue(clrValue))*3)/4,
			(((int)GetBValue(clrValue))*3)/4
			);

		// Draw the box
		LinearGradientBrush brBox(rcColor,
			Color(255, GetRValue(clrValue),GetGValue(clrValue),GetBValue(clrValue)),
			Color(255, GetRValue(clrDark),GetGValue(clrDark),GetBValue(clrDark)),
			90);
		graphics.FillRectangle(&brBox, rcColor);

		// Draw the label
		_bstr_t bstrText(m_astrLabels[iValue]);
		format.SetLineAlignment(StringAlignmentCenter);
		graphics.DrawString(bstrText, bstrText.length(), &fnt, rcLabel, &format, &brText);
	}
}

BEGIN_MESSAGE_MAP(CMarketPieGraphWnd, CWnd)
	//{{AFX_MSG_MAP(CMarketPieGraphWnd)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMarketPieGraphWnd message handlers

// (c.haag 2008-04-18 12:28) - PLID 29704 - Exposed drawing to outside callers
void CMarketPieGraphWnd::Draw(CDC* pDC, const CRect& rcBounds)
{
	// Enhanced drawing requires 16 bpp or better, and no printing is allowed for enhanced drawing
	const BOOL bDrawEnhanced = UseEnhancedDrawing(pDC);
	int nLegendColumnWidth = 200;
	int nBufferSpaceBelowLabel = 20;
	int nLabelHeight = 60;
	int nLegendVSpace = m_nLegendVSpace;
	int nBufferSpaceAboveBottom = 60;

	if (pDC->IsPrinting()) {
		nLegendColumnWidth = 2000;
		nBufferSpaceBelowLabel = 200;
		nLabelHeight = 600;
		nLegendVSpace *= 10;
		nBufferSpaceAboveBottom *= 10;
	}

	// Calculate our rectangles
	CRect rcLabel(rcBounds.left, rcBounds.top, rcBounds.right, rcBounds.top + nLabelHeight);
	CRect rcLegend(rcBounds.right - nLegendColumnWidth, rcLabel.bottom + nBufferSpaceBelowLabel, rcBounds.right, rcBounds.bottom - nBufferSpaceAboveBottom);

	// The legend rectangle must fit all the labels. If the labels need 200 pixels for example,
	// and the legend is only 150 pixels high...then we need to "increase the width" of the legend
	// so that the values exist in multiple columns.
	const int nRequiredVSpace = m_astrLabels.GetSize() * nLegendVSpace;
	int nLegendColumns = (pDC->IsPrinting()) ? 1 : (1 + (nRequiredVSpace / rcLegend.Height()));
	rcLegend.left -= (nLegendColumns - 1) * nLegendColumnWidth;
	rcLegend.left = max(rcLegend.left, 0);

	// Draw the title text at the top
	if (bDrawEnhanced) {
		DrawTitleTextEnhanced(pDC, rcLabel);
	} else {
		DrawTitleText(pDC, rcLabel);
	}

	// Draw the pie and legend if there are any values and the pie has room
	CRect rcPie(rcBounds.left, rcLegend.top, rcLegend.left - 10, rcLegend.bottom);

	// Shrink the pie area so that there's some breathing room on either side,
	// and crunch it vertically to give it an angled 3D look. Also move it up
	// so that the shadow drawing doesn't overstep rcBounds.
	rcPie.top += rcBounds.Height() / 12;
	rcPie.bottom -= rcBounds.Height() / 12;
	rcPie.left += rcBounds.Width() / 25;
	rcPie.right -= rcBounds.Width() / 25;

	if (m_anValues.GetSize() > 0 && rcPie.Width() > 20 && rcPie.Height() > 0) {
		if (bDrawEnhanced) {
			DrawPieEnhanced(pDC, rcPie);
		} else {
			DrawPie(pDC, rcPie);
		}
	} // if (m_anValues.GetSize() > 0) {

	// Draw the legend if there are any values
	if (m_anValues.GetSize() > 0) {
		int nLabelsPerColumn = m_anValues.GetSize() / nLegendColumns;
		for (int i=0; i < nLegendColumns; i++) {
			CRect rc(
				rcLegend.left + i*nLegendColumnWidth,
				rcLegend.top,
				rcLegend.left + (i+1)*nLegendColumnWidth,
				rcLegend.bottom
				);

			if (bDrawEnhanced) {
				DrawLegendEnhanced(pDC, rc, i*nLabelsPerColumn, (i+1)*nLabelsPerColumn);
			} else {
				DrawLegend(pDC, rc, i*nLabelsPerColumn, (i+1)*nLabelsPerColumn);
			}

		} // for (int i=0; i < nLegendColumns; i++) {

	} // if (m_anValues.GetSize() > 0) {
}

void CMarketPieGraphWnd::OnPaint() 
{
	// (c.haag 2008-04-17 17:39) - PLID 29704 - Window painting function
	CPaintDC dc(this); // device context for painting
	//DRT 10/23/2008 - PLID 31816 - Renamed CMemDC to CNxMemDC
	CNxMemDC memdc(&dc);
	
	CRect rcClient;
	GetClientRect(&rcClient);

	Draw(&memdc, rcClient);

	// Do not call CWnd::OnPaint() for painting messages
}
