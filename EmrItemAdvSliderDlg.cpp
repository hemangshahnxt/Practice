// EmrItemAdvSliderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EmrItemAdvSliderDlg.h"
#include "EMNDetail.h"
#include "WindowlessUtils.h"
#include "EMRTopic.h"
#include "EMN.h"

// (a.walling 2011-11-11 11:11) - PLID 46632 - WindowlessUtils - Various functions replaced with windowless-safe versions.

extern CPracticeApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvSliderDlg dialog


CEmrItemAdvSliderDlg::CEmrItemAdvSliderDlg(class CEMNDetail *pDetail)
	: CEmrItemAdvDlg(pDetail)
{
	//{{AFX_DATA_INIT(CEmrItemAdvSliderDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CEmrItemAdvSliderDlg, CEmrItemAdvDlg)
	//{{AFX_MSG_MAP(CEmrItemAdvSliderDlg)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SHOWWINDOW()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvSliderDlg message handlers

BOOL CEmrItemAdvSliderDlg::RepositionControls(IN OUT CSize &szArea, BOOL bCalcOnly)
{
	CEmrItemAdvDlg::RepositionControls(szArea, bCalcOnly);
	// The caller is giving us a full window area so we have to adjust off the 
	// border to get what will be our client area.
	CSize szBorder;
	CSize szMergeBtn;
	CalcWindowBorderSize(this, &szBorder);
	// Adjust off the border
	szArea.cx -= szBorder.cx;
	szArea.cy -= szBorder.cy;
	
	const long cnSliderSize = 80;
	long cnMergeBtnMargin = 5;

	// Make sure the merge status icon button reflects the state of the data,
	// because that will have a direct influence on our size calculations.
	UpdateStatusButtonAppearances();

	CClientDC dc(this);

	long nTopMargin = 3;
	long nIdealWidth = 320;
	if (IsControlValid(&m_wndLabel)) {
		CSize sz(LONG_MAX, LONG_MAX);
		CalcControlIdealDimensions(&dc, &m_wndLabel, sz);
		
		//first see if the label is too wide to be displayed
		if(sz.cx > szArea.cx) {

			//if so, find out how many lines we must create
			int nHeight = 1;
			while (nHeight < 10 && sz.cx / nHeight > szArea.cx) {
				nHeight++;
			}
			
			//now increase the height of the item to match
			if((sz.cx / (nHeight - 1)) > szArea.cx) {
				sz.cx = szArea.cx;
				sz.cy *= nHeight;
			}
		}

		nTopMargin += sz.cy;

		if (nIdealWidth < sz.cx) {
			nIdealWidth = sz.cx;
		}
		if (!bCalcOnly) {
			CRect rPrior, rLabel;
			GetControlChildRect(this, &m_wndLabel, &rPrior);

			m_wndLabel.MoveWindow(6, 2, sz.cx, sz.cy, FALSE);

			GetControlChildRect(this, &m_wndLabel, &rLabel);

			rPrior.InflateRect(2,2,2,2);
			rLabel.InflateRect(2,2,2,2);
			InvalidateRect(rPrior, TRUE);
			InvalidateRect(rLabel, TRUE);
		}
	}
	// (c.haag 2006-06-30 17:00) - PLID 19977 - We now calculate
	// merge button dimensions if either the merge or problem
	// button is visible
	if (IsMergeButtonVisible() || IsProblemButtonVisible()) {
		CSize sz(LONG_MAX, LONG_MAX);
		CalcControlIdealDimensions(&dc, m_pBtnMergeStatus, szMergeBtn);
	}
	else {
		szMergeBtn = CSize(0,0);
		cnMergeBtnMargin = 0;
	}

	long nIdealHeight = 150;



	if (bCalcOnly) {
		if (szArea.cx > nIdealWidth) {
			szArea.cx = nIdealWidth;
		}
		if (szArea.cy > nIdealHeight) {
			szArea.cy = nIdealHeight;
		}
	}

	//Now, are we horizontal or vertical?
	bool bVert = szArea.cy > szArea.cx;
	
	//I'm just guessing at these for now.
	long nMinSliderWidth = bVert ? 20 : 50;
	long nMinSliderHeight = bVert ? 50 : 20;
	long nMinCaptionWidth = 50;
	long nMinCaptionHeight = 20;

	// Make sure we're not smaller than the minimum
	long nMinWidth = nMinSliderWidth + 20 + (bVert?nMinCaptionWidth:0);
	long nMinHeight = nTopMargin + nMinSliderHeight + cnMergeBtnMargin + szMergeBtn.cy + 10 + (bVert?0:nMinCaptionHeight);

	if (szArea.cx < nMinWidth) {
		szArea.cx = nMinWidth;
	}
	if (szArea.cy < nMinHeight) {
		szArea.cy = nMinHeight;
	}

	if (!bCalcOnly && m_Slider.m_hWnd) {
		if(bVert) {
			//Slider: 30 pixels wide, centered in the window, 
			//height = from (top of area + top margin) to (bottom of the area - the merge button margin - the size of the merge button - 10)
			if(!(m_Slider.GetStyle() & TBS_VERT)) {
				m_Slider.ModifyStyle(TBS_HORZ, TBS_VERT,0);
				m_MinCaption.ModifyStyle(SS_LEFT, SS_RIGHT, 0);
				ReflectCurrentState();
			}
			CRect rSlider((szArea.cx/2) - 15, nTopMargin, (szArea.cx/2)+15, szArea.cy - nTopMargin - cnMergeBtnMargin - szMergeBtn.cy - 10);
			m_Slider.MoveWindow(&rSlider);
			CRect rCaption;
			// (a.wetta 2007-02-21 11:16) - PLID 18847 - Determine if the slider value should be displayed on the right or left side of the slider
			// (a.walling 2007-08-15 15:14) - PLID 27083 - Default to left
			if (GetRemotePropertyInt("EMR_AlignSliderValueRight", 0, 0, GetCurrentUserName(), true)) {
				//Caption, 20 pixels high, centered vertically, from 10 pixels right of slider to end of window.
				rCaption.SetRect((szArea.cx/2) + 25, (szArea.cy/2)-10, szArea.cx, (szArea.cy/2)+10);
			}
			else {
				if(m_pBtnMergeStatus && m_pBtnMergeStatus->GetSafeHwnd()) {
					//Caption, 20 pixels high, centered vertically above merge button, from beginning of window to 10 pixels to left of slider.
					rCaption.SetRect(0, ((szArea.cy - cnMergeBtnMargin - szMergeBtn.cy)/2) - 10, (szArea.cx/2) - 25, ((szArea.cy - cnMergeBtnMargin - szMergeBtn.cy)/2) + 10);
				}
				else {
					//Caption, 20 pixels high, centered vertically, from beginning of window to 10 pixels to left of slider.
					rCaption.SetRect(0, (szArea.cy/2)-10, (szArea.cx/2) - 25, (szArea.cy/2)+10);
				}
			}
			m_Caption.MoveWindow(&rCaption);
			m_Caption.RedrawWindow();
			//Minimum label, 20 pixels high, bottom-aligned with slider, from left of screen to 5 pixels right of slider.
			CRect rMinCaption(0, szArea.cy - nTopMargin - cnMergeBtnMargin - szMergeBtn.cy - 15 - 20, (szArea.cx/2)-20,
				szArea.cy - nTopMargin - cnMergeBtnMargin - szMergeBtn.cy - 15);
			m_MinCaption.MoveWindow(&rMinCaption);
			m_MinCaption.RedrawWindow();
			//Maximum label, 20 pixels high, top-aligned with slider, from left of screen to 5 pixels right of slider.
			CRect rMaxCaption(0, nTopMargin, (szArea.cx/2)-20, nTopMargin + 20);
			m_MaxCaption.MoveWindow(&rMaxCaption);
			m_MaxCaption.RedrawWindow();
		}
		else {

			BOOL bAlignSliderBottom = GetRemotePropertyInt("EMR_AlignSliderValueBottom", 0, 0, GetCurrentUserName(), true);

			// (j.jones 2007-09-18 11:08) - PLID 27083 - if aligning the caption above the slider,
			// offset the slider control and its min/max values by 20
			long nYOffset = 0;
			if(!bAlignSliderBottom) {
				nYOffset = 20;
			}
			
			CRect rCaption;
			// (a.wetta 2007-02-21 11:16) - PLID 18847 - Determine if the slider value should be displayed above or below the slider
			// (a.walling 2007-08-15 15:14) - PLID 27083 - Default to top
			if (bAlignSliderBottom) {
				if(m_pBtnMergeStatus && m_pBtnMergeStatus->GetSafeHwnd()) {
					//Caption, 20 pixels high, 20 pixels below slider, from right of merge button to right of screen - 10.
					rCaption.SetRect(cnMergeBtnMargin + szMergeBtn.cx + 10, (szArea.cy/2)+35, szArea.cx - 10, (szArea.cy/2)+55);
				}
				else {
					//Caption, 20 pixels high, 20 pixels below slider, width of window - 10 pixels on each side.
					rCaption.SetRect(10, (szArea.cy/2)+35, szArea.cx-10, (szArea.cy/2)+55);
				}
			}
			else {
				CRect rLabel;
				GetControlWindowRect(this, &m_wndLabel, &rLabel);
				// (j.jones 2007-08-17 09:10) - PLID 18847 - fixed so the width is the same as the caption
				// for the value on the bottom (although that caption is sometimes affected by the merge
				// button and this one is not)
				// (j.jones 2007-09-18 11:03) - PLID 18847 - moved the top-caption down some so as not to interfere
				// so as not to interfere with the item label
				//Caption, 20 pixels high, 20 pixels above slider, width of window - 10 pixels on each side.
				rCaption.SetRect(10, (szArea.cy/2)-55+nYOffset, szArea.cx-10, (szArea.cy/2)-35+nYOffset);
			}
			m_Caption.MoveWindow(&rCaption);
			m_Caption.RedrawWindow();

			//Slider: 30 pixels high, centered vertically, width of window - 30 pixels on each side.
			if(!(m_Slider.GetStyle() & TBS_HORZ)) {
				m_Slider.ModifyStyle(TBS_VERT, TBS_HORZ,0);
				m_MinCaption.ModifyStyle(SS_RIGHT, SS_LEFT, 0);
				ReflectCurrentState();
			}
			// (j.jones 2007-09-18 11:03) - PLID 18847 - if the slider caption is on top, nYOffset will move the slider control down some
			CRect rSlider(10, (szArea.cy/2)-15+nYOffset, szArea.cx-10, (szArea.cy/2)+15+nYOffset);
			m_Slider.MoveWindow(&rSlider);

			//Minimum label,20 pixels high, just above slider, left half of screen (-10 pixel margin).
			CRect rMinCaption(10, (szArea.cy/2)-35+nYOffset, szArea.cx/2, (szArea.cy/2)-15+nYOffset);
			m_MinCaption.MoveWindow(&rMinCaption);
			m_MinCaption.RedrawWindow();
			//Minimum label,20 pixels high, just above slider, right half of screen (-10 pixel margin).
			CRect rMaxCaption(szArea.cx/2, (szArea.cy/2)-35+nYOffset, szArea.cx - 10, (szArea.cy/2)-15+nYOffset);
			m_MaxCaption.MoveWindow(&rMaxCaption);
			m_MaxCaption.RedrawWindow();
		}
	}

	// (c.haag 2006-07-03 09:29) - PLID 19944 - We now use the variable x to calculate
	// the correct position of each iconic button
	int x = 10;
	if (!bCalcOnly && IsMergeButtonVisible()) {
		m_pBtnMergeStatus->MoveWindow(x, szArea.cy - szMergeBtn.cy - 10, szMergeBtn.cx, szMergeBtn.cy);
		m_pBtnMergeStatus->RedrawWindow();
		x += szMergeBtn.cx + 3;
	}
	if (!bCalcOnly && IsProblemButtonVisible()) {
		m_pBtnProblemStatus->MoveWindow(x, szArea.cy - szMergeBtn.cy - 10, szMergeBtn.cx, szMergeBtn.cy);
		m_pBtnProblemStatus->RedrawWindow();
	}

	BOOL bAns;
	if (szArea.cx >= nIdealWidth && szArea.cy >= nIdealHeight) {
		// Our ideal fits within the given area
		bAns = TRUE;
	} else {
		// Our ideal was too big for the given area
		bAns = FALSE;
	}

	// Return the new area, but adjust back on the border size since the caller wants window area
	szArea.cx += szBorder.cx;
	szArea.cy += szBorder.cy;

	return bAns;
}

void CEmrItemAdvSliderDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		CEmrItemAdvDlg::OnShowWindow(bShow, nStatus);
		
		// (a.walling 2012-05-16 15:25) - PLID 50430 - Don't set focus when showing the window!!
		//if (bShow) {
		//	// (z.manning, 04/14/2008) - PLID 29632 - Use the defined IDC rather than a hardcoded value
		//	GetDlgItem(SLIDER_IDC)->SetFocus();
		//}
	} NxCatchAll("Error in OnShowWindow");
}

void CEmrItemAdvSliderDlg::ReflectCurrentState()
{
	CEmrItemAdvDlg::ReflectCurrentState();

	if(m_Slider.GetSafeHwnd()) {
		m_Slider.SetPos(ValueToSliderPos(VarDouble(m_pDetail->GetState(),m_pDetail->GetSliderMin())));
		// (c.haag 2007-05-17 10:20) - PLID 26046 - Use GetStateVarType to get the detail state type
		if(m_pDetail->GetStateVarType() == VT_NULL) {
			m_Caption.SetWindowText("");
		}
		else {
			m_Caption.SetWindowText(AsString(m_pDetail->GetState()));
		}
	}
}

//TES 3/23/2010 - PLID 37757 - This dialog doesn't maintain its own ReadOnly flag, so I changed the function name from
// SetReadOnly() to ReflectReadOnlyStatus()
void CEmrItemAdvSliderDlg::ReflectReadOnlyStatus(BOOL bReadOnly)
{
	//TES 3/15/2010 - PLID 37757 - The ReadOnly status lives in the detail now
	//m_bReadOnly = bReadOnly;
	if(GetSafeHwnd()) {
		m_wndLabel.EnableWindow(!m_pDetail->GetReadOnly());
		m_Slider.EnableWindow(!m_pDetail->GetReadOnly());
		m_Caption.EnableWindow(!m_pDetail->GetReadOnly());
	}

	CEmrItemAdvDlg::ReflectReadOnlyStatus(bReadOnly);
}

void CEmrItemAdvSliderDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		//For some insane reason, it was decided that slider controls would notify their parents through the WM_HSCROLL message.
		//And it passes a pointer to the slider control, but casts it as a CScrollBar!!!!
		ASSERT(pScrollBar->GetSafeHwnd() == m_Slider.GetSafeHwnd());

		// (j.jones 2005-09-16 10:53) - PLID 17534 - the slider sent data to the rest of the EMR and Narrative
		// in a constant stream if you tried to drag the slider, and then Narratives stole focus (MS Rich Edit problem)
		// which in the end prevented any form of sliding at all if the slider was on the Narrative.
		// This new code only updates the rest of EMR when the LButton is released.

		double dValue = SliderPosToValue(m_Slider.GetPos());

		//handle southpaws
		int nKey = GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON;
		if(!(GetAsyncKeyState(nKey) & 0x80000000)) { //see if they released the "left" mouse button
			//if they released, send to the rest of EMR, which will update narratives and such
			m_pDetail->RequestStateChange(dValue);
		}
		else {
			//if they didn't release yet, only update the local caption
			if (m_pToolTipWnd && IsWindow(m_pToolTipWnd->GetSafeHwnd())) m_pToolTipWnd->ShowWindow(SW_HIDE);
			m_Slider.SetPos(ValueToSliderPos(dValue));
			m_Caption.SetWindowText(AsString(dValue));
		}
	} NxCatchAll("Error in OnHScroll");
}

void CEmrItemAdvSliderDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		//For some insane reason, it was decided that slider controls would notify their parents through the WM_VSCROLL message.
		//And it passes a pointer to the slider control, but casts it as a CScrollBar!!!!
		ASSERT(pScrollBar->GetSafeHwnd() == m_Slider.GetSafeHwnd());

		// (j.jones 2005-09-16 10:53) - PLID 17534 - the slider sent data to the rest of the EMR and Narrative
		// in a constant stream if you tried to drag the slider, and then Narratives stole focus (MS Rich Edit problem)
		// which in the end prevented any form of sliding at all if the slider was on the Narrative.
		// This new code only updates the rest of EMR when the LButton is released.

		double dValue = SliderPosToValue(m_Slider.GetPos());

		//handle southpaws
		int nKey = GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON;
		if(!(GetAsyncKeyState(nKey) & 0x80000000)) { //see if they released the "left" mouse button
			//if they released, send to the rest of EMR, which will update narratives and such
			m_pDetail->RequestStateChange(dValue);
		}
		else {
			//if they didn't release yet, only update the local caption
			if (m_pToolTipWnd && IsWindow(m_pToolTipWnd->GetSafeHwnd())) m_pToolTipWnd->ShowWindow(SW_HIDE);
			m_Slider.SetPos(ValueToSliderPos(dValue));
			m_Caption.SetWindowText(AsString(dValue));
		}
	} NxCatchAll("Error in OnVScroll");
}

void CEmrItemAdvSliderDlg::ReflectCurrentContent()
{
	// Clear out anything that's already there
	DestroyContent();
	
	// Create everything the parent wants
	CEmrItemAdvDlg::ReflectCurrentContent();
	
	//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
	DWORD dwDisabled = m_pDetail->GetReadOnly() ? WS_DISABLED : 0;
	
	// Add the label
	{
		// (b.cardillo 2012-05-02 20:28) - PLID 49255 - Use the detail label text with special modifiers for onscreen presentation
		CString strLabel = GetLabelText(TRUE);
		if (!strLabel.IsEmpty()) {
			strLabel.Replace("&", "&&");
			// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
			m_wndLabel.CreateControl(strLabel, WS_VISIBLE | WS_GROUP | dwDisabled, CRect(0, 0, 0, 0), this, 0xffff);
			//m_wndLabel.ModifyStyleEx(0, WS_EX_TRANSPARENT); // (a.walling 2011-05-11 14:55) - PLID 43661 - Labels need WS_EX_TRANSPARENT exstyle
			// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
			m_wndLabel->NativeFont = EmrFonts::GetTitleFont();
		}
	}
	//TES 1/29/2008 - PLID 28673 - Use a #defined IDC, not a numeric literal.
	m_Slider.Create(WS_BORDER|WS_VISIBLE|WS_GROUP|TBS_HORZ|TBS_AUTOTICKS|TBS_BOTTOM|TBS_LEFT|dwDisabled, CRect(0, 0, 300, 300), this, SLIDER_IDC);

	m_Caption.Create("", WS_CHILD|WS_VISIBLE|WS_GROUP|SS_CENTER|dwDisabled, CRect(0,0,300,300), this);
	//m_Caption.ModifyStyleEx(0, WS_EX_TRANSPARENT); // (a.walling 2011-05-11 14:55) - PLID 43661 - Labels need WS_EX_TRANSPARENT exstyle
	// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
	m_Caption.SetFont(CFont::FromHandle(EmrFonts::GetBoldFont()));

	m_MinCaption.Create("", WS_CHILD|WS_VISIBLE|WS_GROUP|SS_RIGHT|dwDisabled, CRect(0,0,300,300), this);
	//m_MinCaption.ModifyStyleEx(0, WS_EX_TRANSPARENT); // (a.walling 2011-05-11 14:55) - PLID 43661 - Labels need WS_EX_TRANSPARENT exstyle
	// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
	m_MinCaption.SetFont(CFont::FromHandle(EmrFonts::GetRegularFont()));
	m_MinCaption.SetWindowText(AsString(m_pDetail->GetSliderMin()));

	m_MaxCaption.Create("", WS_CHILD|WS_VISIBLE|WS_GROUP|SS_RIGHT|dwDisabled, CRect(0,0,300,300), this);
	//m_MaxCaption.ModifyStyleEx(0, WS_EX_TRANSPARENT); // (a.walling 2011-05-11 14:55) - PLID 43661 - Labels need WS_EX_TRANSPARENT exstyle
	// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
	m_MaxCaption.SetFont(CFont::FromHandle(EmrFonts::GetRegularFont()));
	m_MaxCaption.SetWindowText(AsString(m_pDetail->GetSliderMax()));

	//Set the slider's parameters.
	double dScale = 1.0 / m_pDetail->GetSliderInc();
	if(m_Slider.GetSafeHwnd()) {
		/*m_Slider.SetRangeMin((int)(m_pDetail->GetSliderMin()*dScale));
		m_Slider.SetRangeMax((int)(m_pDetail->GetSliderMax()*dScale));
		m_Slider.SetTicFreq(1);*/

		// (a.walling 2007-02-28 12:31) - PLID 24914 - Several issues with slider values. I've simplified the way
		// we store this data and calculate between values and positions. Now the slider control's minimum value is
		// always zero. This makes all these calculations much easier and avoids rounding errors, not to mention
		// solving the problem with odd increments allowing values below the minimum (and then all the incremented
		// values being off).

		// I don't know why or how this could happen, but might as well check for it.
		if (m_pDetail->GetSliderInc() == 0) {
			ThrowNxException("CEmrItemAdvSliderDlg::ReflectCurrentContent(): Slider increment value is zero!");
		}

		m_Slider.SetRangeMin(0);
		m_Slider.SetRangeMax((int)floor((m_pDetail->GetSliderMax() - m_pDetail->GetSliderMin()) / m_pDetail->GetSliderInc()));
		m_Slider.SetTicFreq(1);
	}

	/*m_Slider.SetFocus();*/
}

void CEmrItemAdvSliderDlg::DestroyContent()
{
	CEmrItemAdvDlg::DestroyContent();

	if (IsControlValid(&m_wndLabel)) {
		m_wndLabel.DestroyWindow();
	}

	if(m_Slider.m_hWnd) {
		m_Slider.DestroyWindow();
	}

	if(m_Caption.m_hWnd) {
		m_Caption.DestroyWindow();
	}

	if(m_MinCaption.m_hWnd) {
		m_MinCaption.DestroyWindow();
	}

	if(m_MaxCaption.m_hWnd) {
		m_MaxCaption.DestroyWindow();
	}
}

int CEmrItemAdvSliderDlg::ValueToSliderPos(double dValue)
{
	//First, scale by our increment.
	// (b.cardillo 2006-09-18 13:55) - PLID 22215 - Need to round here, not truncate.  For 
	// example, if dValue is 4.2999999998, and the increment is 0.1, then we would get 42 
	// for nPos by truncating, instead of 43 which is obviously what we want.
	/*int nPos = RoundAsLong(dValue / m_pDetail->GetSliderInc());

	//Now, if we're vertical, we need to flip it.
	if(m_Slider.GetStyle() & TBS_VERT) {
		int nDistanceFromEdge = (int)(m_pDetail->GetSliderMax()/m_pDetail->GetSliderInc()) - nPos;
		nPos = (int)(m_pDetail->GetSliderMin()/m_pDetail->GetSliderInc()) + nDistanceFromEdge;
	}

	return nPos;*/

	// (a.walling 2007-02-28 12:35) - PLID 24914 - Several issues with slider values. I've simplified the way
	// we store this data and calculate between values and positions. Now the slider control's minimum value is
	// always zero. This makes all these calculations much easier and avoids rounding errors, not to mention
	// solving the problem with odd increments allowing values below the minimum (and then all the incremented
	// values being off).

	// I don't know why or how this could happen, but might as well check for it.
	if (m_pDetail->GetSliderInc() == 0) {
		ThrowNxException("CEmrItemAdvSliderDlg::ValueToSliderPos(%g): Slider increment value is zero!", dValue);
	}

	dValue -= m_pDetail->GetSliderMin();
	dValue /= m_pDetail->GetSliderInc();
	long nSliderPos = RoundAsLong(dValue);

	// flip the slider position if vertical. It is easier to flip the position after being calculated
	// rather than introduce more possible rounding errors. See SliderPosToValue for more info.
	if (m_Slider.GetStyle() & TBS_VERT) {
		nSliderPos = m_Slider.GetRangeMax() - nSliderPos;
	}

	// If you reach these asserts, it most likely means that the detail's state is inconsistent with the
	// actual min and max and inc values for the slider. The only way I can see this happening is either
	// corrupted data or a slider which has just been modified on the fly. A vertical slider will have an
	// invalid negative pos, and a horizontal slider will have an invalid pos > max. This situation arose
	// previously, as well, but was silent since there were no asserts. Regardless, setting the slider's
	// pos to these incorrect values as before will effectively set thumb to the max or min position, and
	// the incorrect slider value will stay until the slider is moved. This behaviour will be investigated
	// in PLID 25002	
	// (a.walling 2010-04-26 10:23) - PLID 25002 - This is how it will remain, like everywhere else in the
	// EMN. The out of range value will continue to be saved until the slider is modified.
	/*
	ASSERT(nSliderPos >= 0);
	ASSERT(nSliderPos <= m_Slider.GetRangeMax());
	*/

	return nSliderPos;
}

double CEmrItemAdvSliderDlg::SliderPosToValue(int nSliderPos)
{
	/*
	//First, if we're vertical, flip it.
	if(m_Slider.GetStyle() & TBS_VERT) {
		int nDistanceFromEdge = (int)(m_pDetail->GetSliderMax()/m_pDetail->GetSliderInc()) - nSliderPos;
		nSliderPos = (int)(m_pDetail->GetSliderMin()/m_pDetail->GetSliderInc()) + nDistanceFromEdge;
	}

	//Now, factor in our increment.
	return (double)nSliderPos * m_pDetail->GetSliderInc();*/

	// (a.walling 2007-02-28 12:34) - PLID 24914 - Several issues with slider values. I've simplified the way
	// we store this data and calculate between values and positions. Now the slider control's minimum value is
	// always zero. This makes all these calculations much easier and avoids rounding errors, not to mention
	// solving the problem with odd increments allowing values below the minimum (and then all the incremented
	// values being off).

	// If we are vertical, flip the position. Since the min will internally be zero, it's easy enough
	// to calculate that the opposite is the max minus the current. ie, 0 -> max, 1 -> max - 1, etc.
	if(m_Slider.GetStyle() & TBS_VERT) {
		nSliderPos = m_Slider.GetRangeMax() - nSliderPos;
		ASSERT(nSliderPos >= 0); // see ValueToSliderPos for discussion on related asserts.
	}

	double dblVal = nSliderPos * m_pDetail->GetSliderInc();
	dblVal += m_pDetail->GetSliderMin();

	return dblVal;
}

void CEmrItemAdvSliderDlg::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		CRect rSlider;
		m_Slider.GetWindowRect(&rSlider);
		// (j.jones 2006-08-15 12:37) - PLID 21985 - unselecting is not allowed in locked EMNs
		// (c.haag 2007-05-17 10:20) - PLID 26046 - Use GetStateVarType to get the detail state type
		// (a.walling 2008-08-22 09:16) - PLID 23138 - Don't allow this if we are readonly
		//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
		if(!m_pDetail->GetReadOnly() && rSlider.PtInRect(pos) && m_pDetail->GetStateVarType() != VT_NULL && m_pDetail->m_pParentTopic->GetParentEMN()->GetStatus() != 2) {
			// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
			CNxMenu mnu;
			mnu.CreatePopupMenu();
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, 1, "Unselect");
			int nReturn = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pos.x, pos.y, this);
			if(nReturn == 1) {
				_variant_t varNull;
				varNull.vt = VT_NULL;
				m_pDetail->SetState(varNull);
				m_pDetail->RequestStateChange(varNull);
			}
			mnu.DestroyMenu();
		}
		else {
			CEmrItemAdvDlg::OnContextMenu(pWnd, pos);
		}
	} NxCatchAll("Error in OnContextMenu");
}
