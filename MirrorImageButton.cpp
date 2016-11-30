// MirrorImageButton.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "MirrorImageButton.h"
#include "NxMessageDef.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMirrorImageButton

CMirrorImageButton::CMirrorImageButton()
{	
	m_image = NULL;
	m_nError = eNoError;
	m_bAutoDeleteImage = true;
}

CMirrorImageButton::~CMirrorImageButton()
{
	if (m_image && m_bAutoDeleteImage)
		DeleteObject(m_image);
}


BEGIN_MESSAGE_MAP(CMirrorImageButton, CButton)
	//{{AFX_MSG_MAP(CMirrorImageButton)
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMirrorImageButton message handlers

#define HIRES

void CMirrorImageButton::Draw(CDC *pdc)
{
	CRect	imageRect, 
			windowRect,
			srcRect,
			bkgRect;

	BITMAP	tmpBmp;
	CDC		bmpDC;
	CPen	pn(PS_SOLID, 1, RGB(0,0,0));

	GetClientRect(windowRect);

	//TES 2003-12-29: Let's handle any error situations.
	switch(m_nError) {
	case eErrorUnspecified:
	{
		CPen* pOldPen = pdc->SelectObject(&pn);
		//TES 5/14/2008 - PLID 30041 - Don't draw the background ourselves, let NxColor do its gradient.
		//pdc->FillRect(windowRect, &m_brush);
		pdc->MoveTo(windowRect.left, windowRect.top);
		pdc->LineTo(windowRect.right-1, windowRect.top);
		pdc->LineTo(windowRect.right-1, windowRect.bottom-1);
		pdc->LineTo(windowRect.left, windowRect.bottom-1);
		pdc->LineTo(windowRect.left, windowRect.top);
		pdc->SelectObject(pOldPen);

		pdc->SetBkMode(TRANSPARENT);
		pdc->SetTextColor(0);
		windowRect.top = (windowRect.top + windowRect.bottom) / 2 - 22;
		pdc->DrawText("Could not", windowRect, DT_CENTER);
		windowRect.top += 12;
		long lHeight = pdc->DrawText("open image", windowRect, DT_CENTER);
		if(m_source == eImageSrcPractice) {
			windowRect.top += 12;
			pdc->DrawText(m_strPracticeFileName.Find("\\") == -1 ? m_strPracticeFileName : m_strPracticeFileName.Mid(m_strPracticeFileName.ReverseFind('\\')+1), windowRect, DT_CENTER);
		}

		windowRect.top += 22;
		pdc->DrawText("Click here for help", windowRect, DT_CENTER);
		
		return;
		break;
	}
	case eErrorNoPermission:
	{
		CPen* pOldPen = pdc->SelectObject(&pn);
		//TES 5/14/2008 - PLID 30041 - Don't draw the background ourselves, let NxColor do its gradient.
		//pdc->FillRect(windowRect, &m_brush);
		pdc->MoveTo(windowRect.left, windowRect.top);
		pdc->LineTo(windowRect.right-1, windowRect.top);
		pdc->LineTo(windowRect.right-1, windowRect.bottom-1);
		pdc->LineTo(windowRect.left, windowRect.bottom-1);
		pdc->LineTo(windowRect.left, windowRect.top);
		pdc->SelectObject(pOldPen);

		pdc->SetBkMode(TRANSPARENT);
		pdc->SetTextColor(0);
		windowRect.top = (windowRect.top + windowRect.bottom) / 2 - 22;
		pdc->DrawText("You have no", windowRect, DT_CENTER);
		windowRect.top += 12;
		pdc->DrawText("permission to", windowRect, DT_CENTER);
		windowRect.top += 12;
		pdc->DrawText("view this image.", windowRect, DT_CENTER);

		windowRect.top += 22;
		pdc->DrawText("Click here for help", windowRect, DT_CENTER);
		
		return;
		break;
	}
	// (c.haag 2009-04-01 17:24) - PLID 33630 - Text for when the Mirror link is initializing
	case eImageBtnInitializingMirror:
	{
		CPen* pOldPen = pdc->SelectObject(&pn);
		pdc->MoveTo(windowRect.left, windowRect.top);
		pdc->LineTo(windowRect.right-1, windowRect.top);
		pdc->LineTo(windowRect.right-1, windowRect.bottom-1);
		pdc->LineTo(windowRect.left, windowRect.bottom-1);
		pdc->LineTo(windowRect.left, windowRect.top);
		pdc->SelectObject(pOldPen);

		pdc->SetBkMode(TRANSPARENT);
		pdc->SetTextColor(0);
		windowRect.top = (windowRect.top + windowRect.bottom) / 2 - 22;
		pdc->DrawText("Initializing", windowRect, DT_CENTER);
		windowRect.top += 12;
		pdc->DrawText("The Link To", windowRect, DT_CENTER);
		windowRect.top += 12;
		pdc->DrawText("Canfield Mirror.", windowRect, DT_CENTER);

		windowRect.top += 22;
		pdc->DrawText("Please Wait...", windowRect, DT_CENTER);
		
		return;
	}
	default: 
		//No errors, but there may be other invalid states here.
		if (m_image == (HBITMAP)MIRRORIMAGEBUTTON_PENDING)
		{
			CPen* pOldPen = pdc->SelectObject(&pn);
			CRect rcProgress;
			CalcProgressRect(rcProgress);

			// (j.jones 2008-06-17 16:00) - PLID 30419 - added a function to safely ensure the progress control exists
			CheckEnsureProgressControl();

			if (m_progress.IsWindowVisible()) {
				windowRect.bottom = rcProgress.top - 1;
			}
			//TES 5/14/2008 - PLID 30041 - Don't draw the background ourselves, let NxColor do its gradient.
			//pdc->FillRect(windowRect, &m_brush);
			pdc->MoveTo(windowRect.left, windowRect.top);
			pdc->LineTo(windowRect.right-1, windowRect.top);
			pdc->LineTo(windowRect.right-1, windowRect.bottom-1);
			pdc->LineTo(windowRect.left, windowRect.bottom-1);
			pdc->LineTo(windowRect.left, windowRect.top);
			pdc->SelectObject(pOldPen);

			if (m_progress.IsWindowVisible()) {
				windowRect.bottom = rcProgress.bottom;
			}

			pdc->SetBkMode(TRANSPARENT);
			pdc->SetTextColor(0);		
			windowRect.top = (windowRect.top + windowRect.bottom) / 2 - 12;
			pdc->DrawText("Loading image...", windowRect, DT_CENTER);	
			return;
		}
		else if (!m_image)
		{	
			//TES 5/14/2008 - PLID 30041 - Don't draw the background ourselves, let NxColor do its gradient.
			//pdc->FillRect(windowRect, &m_brush);
			return;
		}
		break;
	}

	GetObject(m_image, sizeof(tmpBmp), &tmpBmp);

	int min = windowRect.Height();
	if (min > windowRect.Width())
		min = windowRect.Width();
	if (!min)
		return;

	imageRect = DrawDIBitmapInRect(pdc, windowRect, m_image);

	//left
	bkgRect.top = windowRect.top;
	bkgRect.bottom = windowRect.bottom;
	bkgRect.left = windowRect.left;
	bkgRect.right = imageRect.left;
	//TES 5/14/2008 - PLID 30041 - Don't draw the background ourselves, let NxColor do its gradient.
	//pdc->FillRect(bkgRect, &m_brush);

	//right
	bkgRect.left = imageRect.right;
	bkgRect.right = windowRect.right;
	//TES 5/14/2008 - PLID 30041 - Don't draw the background ourselves, let NxColor do its gradient.
	//pdc->FillRect(bkgRect, &m_brush);

	//top
	bkgRect.bottom = imageRect.top;
	bkgRect.top = windowRect.top;
	bkgRect.left = windowRect.left;
	//TES 5/14/2008 - PLID 30041 - Don't draw the background ourselves, let NxColor do its gradient.
	//pdc->FillRect(bkgRect, &m_brush);

	//bottom
	bkgRect.top = imageRect.bottom;
	bkgRect.bottom = windowRect.bottom;
	//TES 5/14/2008 - PLID 30041 - Don't draw the background ourselves, let NxColor do its gradient.
	//pdc->FillRect(bkgRect, &m_brush);
}

void CMirrorImageButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC *pdc;
	
	pdc = CDC::FromHandle(lpDrawItemStruct->hDC);
	Draw(pdc);
}

#define UNMAKEPOINTS(pts) (*((LPARAM *)&(pts)))
//BVB - my reverse of the standard MAKEPOINTS macro

void CMirrorImageButton::OnRButtonUp(UINT nFlags, CPoint point) 
{
	//pass the right click event to our parent

	CWnd *p = GetParent();
	POINTS pts;
	
	//have to undo a bit of what MFC did to use the API standard
	pts.x = (short)point.x;
	pts.y = (short)point.y;

	if (p && p->GetSafeHwnd())
		::SendMessage(p->GetSafeHwnd(), NXM_RBUTTONCLK, nFlags, UNMAKEPOINTS(pts));
	CButton::OnRButtonUp(nFlags, point);
}

void CMirrorImageButton::CalcProgressRect(CRect& rc)
{
	GetClientRect(rc);
	long lHeight = rc.Height();
	rc.top = rc.bottom - (long)((float)lHeight * 0.05f);
}

void CMirrorImageButton::OnSize(UINT nType, int cx, int cy) 
{
	CButton::OnSize(nType, cx, cy);
	
	CRect rc;
	CalcProgressRect(rc);
	
	// WM_CREATE messages are not passed to this window, so we have to
	// do this the dumb way.
	
	// (j.jones 2008-06-17 16:00) - PLID 30419 - added a function to safely ensure the progress control exists
	CheckEnsureProgressControl();
	
	m_progress.MoveWindow(0, rc.top, rc.Width(), rc.Height());
}

// (j.jones 2008-06-17 16:00) - PLID 30419 - added a function to safely ensure the progress control exists
void CMirrorImageButton::CheckEnsureProgressControl()
{
	//let exceptions be thrown to the caller

	if (!m_progress.m_hWnd) {

		CRect rc;
		CalcProgressRect(rc);

		m_progress.Create(WS_CHILD, rc, this, 0x0000967F);
		m_progress.SetRange(0,100);
	}
}
