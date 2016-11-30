// MDIClientWnd.cpp : implementation file
//

#include "stdafx.h"
#include "MDIClientWnd.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2008-04-04 09:37) - PLID 29544 - Helper class for subclassing MDIClientWnd of MainFrame; fills with a subtle light gradient.

/////////////////////////////////////////////////////////////////////////////
// CMDIClientWnd

CMDIClientWnd::CMDIClientWnd()
{
	m_nMins = -1;
}

CMDIClientWnd::~CMDIClientWnd()
{
}


BEGIN_MESSAGE_MAP(CMDIClientWnd, CWnd)
	//{{AFX_MSG_MAP(CMDIClientWnd)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

using namespace Gdiplus;

//#define DYNAMIC_MDICLIENT_BACKGROUND

/////////////////////////////////////////////////////////////////////////////
// CMDIClientWnd message handlers

void CMDIClientWnd::OnSize(UINT nType, int cx, int cy) 
{
	Invalidate(FALSE);
	CWnd::OnSize(nType, cx, cy);
}

//#define MDICLIENTWND_PROFILE

void CMDIClientWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rc, rcClip;
	GetClientRect(rc);
	int result = dc.GetClipBox(rcClip);

	if (result != NULLREGION && result != ERROR) {	
		// (a.walling 2008-05-08 13:45) - PLID 29963 - Don't use gradients if preference is set.
		if ((GetMainFrame()->GetDisplayGradients() == FALSE) || NxGdi::IsRemote() ) {
			// (a.walling 2008-04-07 10:01) - PLID 29544 - they are TS session, so don't do any gradients, just fill with a pleasing soft color.
			CBrush brush;
			// (a.walling 2009-08-12 16:12) - PLID 35136 - Move MDI client background color to global GetNxColor
			brush.CreateSolidBrush(GetNxColor(GNC_MDICLIENT, 0));
			dc.FillRect(rc, &brush);
		} else {			

#ifdef MDICLIENTWND_PROFILE
			LARGE_INTEGER nFreq, nPre, nPost;
			QueryPerformanceFrequency(&nFreq);
			QueryPerformanceCounter(&nPre);
#endif

			// (a.walling 2009-01-14 14:07) - PLID 32734 - Use the built-in FillGradient function, which is faster and does not suffer from
			// the GDI+ clipping bugs on stupid video cards

			// Create an array of TRIVERTEX structures that describe
			// positional and color values for each vertex. For a rectangle,
			// only two vertices need to be defined: upper-left and lower-right.
			TRIVERTEX vertex[2] ;
			vertex[0].x     = 0;
			vertex[0].y     = 0;
			vertex[0].Red   = 0xD000;
			vertex[0].Green = 0xDF00;
			vertex[0].Blue  = 0xF200;
			vertex[0].Alpha = 0x0000;

			vertex[1].x     = rc.right;
			vertex[1].y     = rc.bottom; 
			vertex[1].Red   = 0xDB00;
			vertex[1].Green = 0xDA00;
			vertex[1].Blue  = 0xD400;
			vertex[1].Alpha = 0x0000;

			// Create a GRADIENT_RECT structure that
			// references the TRIVERTEX vertices.
			GRADIENT_RECT gRect;
			gRect.UpperLeft  = 0;
			gRect.LowerRight = 1;

			// Draw a gradient rectangle.
			dc.GradientFill(vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_V);

#ifdef MDICLIENTWND_PROFILE
			QueryPerformanceCounter(&nPost);
			TRACE("* MDIClientWnd Paint (%li, %li, %li, %li) GradientFille: %I64d us (%I64d ticks @ %I64d /second)-- complete!\n", rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), (nPost.QuadPart - nPre.QuadPart) / (nFreq.QuadPart / 1000), (nPost.QuadPart - nPre.QuadPart), nFreq.QuadPart);
#endif

		}
		/*
		{
#ifdef MDICLIENTWND_PROFILE
			TRACE("Painting MDIClient - (%li, %li, %li, %li)\n", rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height());
			LARGE_INTEGER nFreq, nPre, nPost;

			QueryPerformanceFrequency(&nFreq);
#endif

			
			{
#ifdef MDICLIENTWND_PROFILE
				LARGE_INTEGER nPre2, nPost2;

				QueryPerformanceCounter(&nPre);
#endif

				const long nCacheWidth = 1;

				Bitmap bCache(nCacheWidth, rcClip.Height());
				{
#ifdef MDICLIENTWND_PROFILE					
					QueryPerformanceCounter(&nPre2);
#endif
					Graphics gBuffer(&bCache);

					// (a.walling 2008-04-14 15:10) - PLID 29544 - Looks odd with the gradient toolbar to not have a top-down gradient
					Point Dest(rc.left + rc.Width() / 2, rc.top);
					Point InvDest(rc.left + rc.Width() / 2, rc.top + rc.Height());

					Dest.Y -= (rcClip.top - rc.top);
					InvDest.Y -= (rcClip.top - rc.top);

					// use split-complementary color of toolbar's target at 60 degrees for bottom
					Color cTop(0xFF, 0xD0, 0xDF, 0xF2);
					Color cBottom(255, 219, 218, 212);

					// light subtle bluish-cyan
					//Color cTop(255, 227, 242, 241);
					//Color cBottom(255, 241, 249, 230);

					// Create a nice, light-colored gradient.
					LinearGradientBrush linGrBrush(Dest, InvDest, cTop, cBottom);

					// Performance concerns? Fear not. Less than 5ms.
					gBuffer.FillRectangle(&linGrBrush, Rect(0, 0, nCacheWidth, rcClip.Height()));
						
					
#ifdef MDICLIENTWND_PROFILE
					QueryPerformanceCounter(&nPost2);
					TRACE("* MDIClientWnd Paint Generate Bitmap: %I64d us (%I64d ticks @ %I64d /second)-- complete!\n", (nPost2.QuadPart - nPre2.QuadPart) / (nFreq.QuadPart / 1000), (nPost2.QuadPart - nPre2.QuadPart), nFreq.QuadPart);
#endif
				}
				{

#ifdef MDICLIENTWND_PROFILE
					QueryPerformanceCounter(&nPre2);
#endif
					Graphics graphics(dc);

					Rect rClip = NxGdi::RectFromCRect(rcClip);

					ImageAttributes iaTile;
					iaTile.SetWrapMode(WrapModeTile);

					graphics.SetSmoothingMode(SmoothingModeNone);
					graphics.SetInterpolationMode(InterpolationModeNearestNeighbor);

					graphics.DrawImage(&bCache, NxGdi::RectFromCRect(rcClip), 0, 0, nCacheWidth, rcClip.Height(), UnitPixel, &iaTile);

					graphics.SetSmoothingMode(SmoothingModeDefault);
					graphics.SetInterpolationMode(InterpolationModeDefault);

#ifdef MDICLIENTWND_PROFILE
					QueryPerformanceCounter(&nPost2);
					TRACE("* MDIClientWnd Paint blit: %I64d us (%I64d ticks @ %I64d /second)-- complete!\n", (nPost2.QuadPart - nPre2.QuadPart) / (nFreq.QuadPart / 1000), (nPost2.QuadPart - nPre2.QuadPart), nFreq.QuadPart);
#endif
				}

				
#ifdef MDICLIENTWND_PROFILE
				QueryPerformanceCounter(&nPost);
				TRACE("* MDIClientWnd Paint Generate Bitmap and blit: %I64d us (%I64d ticks @ %I64d /second)-- complete!\n\n", (nPost.QuadPart - nPre.QuadPart) / (nFreq.QuadPart / 1000), (nPost.QuadPart - nPre.QuadPart), nFreq.QuadPart);
#endif
			}
		}
		*/
	}
}

BOOL CMDIClientWnd::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}