// FloatingImageWnd.cpp: implementation of the CFloatingImageWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FloatingImageWnd.h"
#include "GlobalDrawingUtils.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFloatingImageWnd::CFloatingImageWnd()
{

}

CFloatingImageWnd::~CFloatingImageWnd()
{

}

BEGIN_MESSAGE_MAP(CFloatingImageWnd, CWnd)
	//{{AFX_MSG_MAP(CFloatingImageWnd)
	ON_WM_PAINT()
	ON_WM_MOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFloatingImageWnd message handlers
void CFloatingImageWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if(m_hImage) {
		CRect rClient;
		GetClientRect(&rClient);
		DrawBitmapInRect(&dc, rClient, m_hImage);
	}
	// Do not call CWnd::OnPaint() for painting messages
}

void CFloatingImageWnd::OnMove(int x, int y)
{
	CPaintDC dc(this);
	
	if(m_hImage) {
		CRect rClient;
		GetClientRect(&rClient);
		DrawBitmapInRect(&dc, rClient, m_hImage);
	}
}