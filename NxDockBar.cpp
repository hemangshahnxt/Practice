// NxDockBar.cpp : implementation file
//

// (a.walling 2008-04-14 14:21) - PLID 29642 - Overridden CDockBar for custom painting

#include "stdafx.h"
#include "NxDockBar.h"
#include "NxGdiPlusUtils.h"

using namespace Gdiplus;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNxDockBar

CNxDockBar::CNxDockBar()
{
	// (a.walling 2009-01-14 12:33) - PLID 32734 - Use GDI primitives rather than a Gdiplus::Bitmap*
}

CNxDockBar::~CNxDockBar()
{
	// (a.walling 2009-01-14 12:33) - PLID 32734 - Use GDI primitives rather than a Gdiplus::Bitmap*
}


BEGIN_MESSAGE_MAP(CNxDockBar, CDockBar)
	//{{AFX_MSG_MAP(CNxDockBar)
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNxDockBar message handlers

//#define NXDOCKBAR_PROFILE

BOOL CNxDockBar::OnEraseBkgnd(CDC* pDC) 
{
	CRect rc;
	GetClientRect(&rc);
	// (a.walling 2012-03-21 09:00) - PLID 48386 - Use CNexTechDialog's default solid background color
	pDC->FillSolidRect(&rc, CNexTechDialog::GetSolidBackgroundRGBColor());

	return TRUE;
}

CSize CNxDockBar::CalcDynamicLayout(int nLength, DWORD nMode)
{
	return CControlBar::CalcDynamicLayout(nLength, nMode);
	/*
    CSize sz=CControlBar::CalcDynamicLayout(nLength,nMode);

    // keep top docking bar from ever closing up--even if there
    // is nothing docked inside
    if (GetStyle()&CBRS_TOP)
    {
        if (sz.cy<32) sz.cy=32;
    }

    return sz;
	*/
}

void CNxDockBar::OnSize(UINT nType, int cx, int cy) 
{
    CDockBar::OnSize(nType, cx, cy);

	// (a.walling 2008-04-14 14:22) - PLID 29642 - This way is extremely inefficient. We just calculate
	// our gradients based on a fixed point so we will always only have to draw the invalid rect
	
    //Invalidate();
}
