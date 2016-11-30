// NxLabel.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "NxLabel.h"
#include "NxMessageDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (z.manning, 04/22/2008) - PLID 29745 - Now inherit from CNxStatic

BEGIN_MESSAGE_MAP(CNxLabel, CNxStatic)
	//{{AFX_MSG_MAP(CNxLabel)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNxIconButton

CNxLabel::CNxLabel()
{
	m_color = 0;
	m_strText = "";
	m_dtsType = dtsText;
	m_bSingleLine = false;
	// (b.cardillo 2016-06-07 02:45) - NX-100775 - This item needed a way to vertically center even word-wrapped text
	m_bForceVCenter = false;
	m_dwHzAlign = DT_LEFT;
	m_bUseTextColorOverride = false;
	m_textColorOverride = 0;
}

CNxLabel::~CNxLabel()
{
}


/////////////////////////////////////////////////////////////////////////////
// CNxLabel message handlers

void CNxLabel::SetText(const CString &strText)
{
	if (m_strText != strText) {
		m_strText = strText;
		//(e.lally 2009-12-03) PLID 36001 - Moved the drawing code into its own function for outside use.
		AskParentToRedrawWindow();
	}
}

CString CNxLabel::GetText()
{
	return m_strText;
}

void CNxLabel::SetColor(COLORREF color)
{
	m_color = color;
}

COLORREF CNxLabel::GetColor()
{
	return m_color;
}

void CNxLabel::SetType(EDrawTextOnDialogStyle dtsType)
{
	m_dtsType = dtsType;
}

EDrawTextOnDialogStyle CNxLabel::GetType()
{
	return m_dtsType;
}

//DRT 4/2/2008 - PLID 29526 - Allow the user to specify the horizontal alignment
void CNxLabel::SetHzAlign(DWORD dwType)
{
	m_dwHzAlign = dwType;
}

// (c.haag 2014-12-26) - PLID 64255 - Sets the text color override
void CNxLabel::SetTextColorOverride(COLORREF color)
{
	m_textColorOverride = color;
	m_bUseTextColorOverride = true;
}

// (b.cardillo 2016-06-07 02:45) - NX-100775 - We've always needed a way to revert the override
void CNxLabel::ClearTextColorOverride()
{
	m_textColorOverride = 0;
	m_bUseTextColorOverride = false;
}

void CNxLabel::OnPaint()
{
	CPaintDC dc(this);

	//First, fill in the background.
	CRect rThis;
	GetClientRect(rThis);
	// (a.walling 2008-04-02 16:44) - PLID 29497 - Handle non-solid backgrounds, let the parent do it for us.
	//DRT 5/14/2008 - PLID 29771 - We still need an option for solid backgrounds, not everything is gradient happy and able
	//	to draw no background.
	if(!(GetExStyle() & WS_EX_TRANSPARENT)) {
		dc.FillSolidRect(rThis, m_color);
	}

	//Now, draw the text.
	//DRT 4/2/2008 - PLID 29526 - Use the horizontal alignment instead of hardcoding to DT_LEFT
	// (c.haag 2014-12-26) - PLID 64255 - Use the text color override
	// (b.cardillo 2016-06-07 02:45) - NX-100775 - This item needed a way to vertically center even word-wrapped text
	if (!m_bForceVCenter) {
		// Standard draw
		DrawTextOnDialog(this, &dc, rThis, m_strText, m_dtsType, false, m_dwHzAlign, m_bSingleLine, false, 0, 0, m_bUseTextColorOverride, m_textColorOverride);
	} else {
		// Force vertical center by first calculating the necessary size
		CRect rHeight(0, 0, rThis.Width(), rThis.Height());
		DrawTextOnDialog(this, &dc, rHeight, m_strText, m_dtsType, true, m_dwHzAlign, m_bSingleLine, true, 0, 0, m_bUseTextColorOverride, m_textColorOverride);
		// then creating a new rect tighly wrapping the text but centered within the original rect
		//CRect rFinal(0, (rThis.Height() / 2) - (rHeight.Height() / 2), rThis.Width(), (rThis.Height() / 2) + (rHeight.Height() / 2));
		CRect rFinal(0, (rThis.Height() - rHeight.Height()) / 2, rThis.Width(), (rThis.Height() + rHeight.Height()) / 2);
		// then drawing in that new rect
		DrawTextOnDialog(this, &dc, rFinal, m_strText, m_dtsType, false, m_dwHzAlign, m_bSingleLine, false, 0, 0, m_bUseTextColorOverride, m_textColorOverride);
	}
}

void CNxLabel::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// (b.cardillo 2016-06-06 16:20) - NX-100773 - New clickable text style for this
	if (m_dtsType == dtsHyperlink || m_dtsType == dtsHyperlinkBold || m_dtsType == dtsClickableText) {
		GetParent()->PostMessage(NXM_NXLABEL_LBUTTONDOWN, (WPARAM)GetDlgCtrlID(), (LPARAM)nFlags);
	}
}

// (a.walling 2009-04-06 17:00) - PLID 33870 - Otherwise this would eat the message. Decided not to disable the CS_DBLCLKS style in case we
// want to handle double clicks differently in the future.
void CNxLabel::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// (b.cardillo 2016-06-06 16:20) - NX-100773 - New clickable text style for this
	if (m_dtsType == dtsHyperlink || m_dtsType == dtsHyperlinkBold || m_dtsType == dtsClickableText) {
		GetParent()->PostMessage(NXM_NXLABEL_LBUTTONDOWN, (WPARAM)GetDlgCtrlID(), (LPARAM)nFlags);
	}
}

// (b.cardillo 2016-06-07 02:45) - NX-100775 - This item needed a way to vertically center even word-wrapped text
/// <summary>
/// Lets callers set text drawing to single-line mode or multi-line mode. Windows text drawing doesn't 
/// respect vertical centering in multi-line mode, so we traditionally draw single-line text vertically 
/// centered and multi-line text top aligned. The bForceVCenter option allows you to go against the 
/// default Windows behavior and force multi-line text to be vertically centered. It defaults to false 
/// for backgward compatibility. This flag technically affects single-line text too, but since single-
/// line text is already vertically centered by default, there is no practical difference.
/// 
/// NOTE: After calling this function (outside of initialization) be sure to invalidate or otherwise 
/// compel a re-paint of the affected area of the screen
/// </summary>
/// <param name="bSingle">If true, only line allowed (carriage returns ignored and words not wrapped); if false, multi-line with word-wrapping and newlines</param>
/// <param name="bForceVCenter">If true, the text size will be pre-calculated (whether single-line or multi-line) and always drawn vertically centered. If false or not specified, single-line text will be vertically centered and multi-line text will be top aligned</param>
void CNxLabel::SetSingleLine(bool bSingle /*= true*/, bool bForceVCenter /*= false*/)
{
	m_bSingleLine = bSingle;
	m_bForceVCenter = bForceVCenter;
}

void CNxLabel::AskParentToRedrawWindow()
{
	//(e.lally 2009-12-03) PLID 36001 - Moved this section of code from the SetText function

	// (a.walling 2008-04-02 16:44) - PLID 29497 - Handle non-solid backgrounds
	if (IsWindowVisible() && GetParent()) {
		CRect Rect;
		GetWindowRect(&Rect);
		GetParent()->ScreenToClient(&Rect);
		// (a.walling 2008-05-23 13:13) - PLID 30099
#ifndef NXDIALOG_NOCLIPCHILDEN
		GetParent()->RedrawWindow(Rect, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASENOW);
#else
		GetParent()->InvalidateRect(&Rect);
		GetParent()->UpdateWindow();
#endif
	}
}