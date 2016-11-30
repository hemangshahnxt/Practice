// NxSliderCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "NxSliderCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNxSliderCtrl

CNxSliderCtrl::CNxSliderCtrl()
{
	m_bAutoThumbDrag = TRUE;
}

CNxSliderCtrl::~CNxSliderCtrl()
{
}


BEGIN_MESSAGE_MAP(CNxSliderCtrl, CSliderCtrl)
	//{{AFX_MSG_MAP(CNxSliderCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNxSliderCtrl message handlers

void CNxSliderCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// See if we're in auto-thumb-grab mode
	if (m_bAutoThumbDrag) {
		// Calculate the exact rectangle that constitutes the "slider area", that is the area in 
		// which clicking has an effect.  This is also (conveniently) the exact same area that 
		// all of the slider's position-calculations are based on, so we also base ours on it.
		CRect rcClickable;
		CPoint pt;
		{
			// Get the basic range, we'll adjust this below to EXCLUDE the thumb thickness (half 
			// on either side, because that's where the ticks start and end), and to INCLUDE the 
			// thumb tallness (because that's the area the slider allows the user to click in).
			// Notice I'm avoiding words like "width" and "height" because of the slider gives 
			// us a horizontal rect for the channel rect, even if the slider is oriented 
			// vertically, yet for the thumb it gives us the exact rect (which is different when 
			// the slider is in vertical mode vs. horizontal mode).  So the "thickness" means 
			// the thumb's "width" if the slider is horizontal, and "height" when the slider is 
			// vertical.  The "tallness" is the thumb's "height" when the slider is in horizontal 
			// mode, and it's "width" when in vertical.  If this isn't clear, just look at a 
			// slider in horizontal and vertical modes and you'll get it.

			// Get the rect of just the channel itself, as described above we'll adjust this to 
			// be the precise clickable area.
			CRect rc;
			GetChannelRect(&rc);

			// Get the thickness and tallness properties, as well as the "horizontalized" mouse 
			// click coordinates.  Since we're generating a horizontal clickable area regardless 
			// of whether we're actually a vertical or horizontal slider, we have to adjust our 
			// mouse point coordinates to be horizontally oriented as well.
			long nThumbThickness, nThumbTallnessTop, nThumbTallnessBottom;
			{
				CRect rcThumb;
				GetThumbRect(&rcThumb);
				if (GetStyle() & TBS_VERT) {
					// Vertical, so thickness is the thumb height, and tallness is the thumb width
					nThumbThickness = rcThumb.Height();
					nThumbTallnessTop = rcThumb.left;
					nThumbTallnessBottom = rcThumb.right;
					// Vertical, so x is the literal mouse y, and y is the literal mouse x (thus 
					// swapping them to horizontal coords)
					pt.x = point.y;
					pt.y = point.x;
				} else {
					// Horizontal, so thickness is the thumb width, and tallness is the thumb height
					nThumbThickness = rcThumb.Width();
					nThumbTallnessTop = rcThumb.top;
					nThumbTallnessBottom = rcThumb.bottom;
					// Horizontal, so x is the literal mouse x, and y is the literal mouse y
					pt.x = point.x;
					pt.y = point.y;
				}
			}

			// Finally set our clickable rect based on the thickness and tallness attributes, 
			// relative to the original "channel rect"
			rcClickable.top = nThumbTallnessTop;
			rcClickable.bottom = nThumbTallnessBottom;
			rcClickable.left = rc.left + (nThumbThickness / 2);
			rcClickable.right = rc.right - (nThumbThickness - nThumbThickness / 2);
		}

		// Now make sure the click was in that area (because if it wasn't, the slider itself 
		// would ignore it (except to take focus), and so should we)
		if (rcClickable.PtInRect(pt)) {
			// Get the slider range
			int nMin, nMax;
			GetRange(nMin, nMax);

			// Pre-calculate some important values and place them in double variables so that we 
			// don't lose any precision when we divide
			double dblRangeCount = nMax - nMin;
			double dblRangeWidth = rcClickable.Width();
			double dblTickWidth = dblRangeWidth / dblRangeCount;
			double dblClickOffsetDist = pt.x - rcClickable.left;
			// And finally get the actual tick position this click best corresponds with
			double dblTickOffset = dblClickOffsetDist / dblTickWidth;
			int nTickPos = nMin + (int)(dblTickOffset + 0.5);
			// And set us to that position
			SetPos(nTickPos);

			
			
			// Now our job is finished, we've tried our best to jump the thumb to be UNDER the 
			// mouse click.  BUT what if the ticks are so far apart that the thumb (which HAS 
			// to be ON a tick) doesn't end up being physically under the mouse at all?  So 
			// what we do is before calling the base class we make it THINK the user did click 
			// directly over the thumb.  We do this by setting point.x equal to the position of 
			// the tick that we just placed the thumb on!  But we can't just set the point.x 
			// because of two things.  First, if we're in a vertically oriented slider, the .x 
			// and .y have been reversed, so in that case we have to se the .y instead, which 
			// is easy enough.  But SECOND, setting point or pt has no effect on the slider's 
			// own handler of this WM_LBUTTONDOWN message.  That would just change our own 
			// local variable and the slider can't possibly know what's in our local variables.  
			// We have to change the originating pMsg->lParam.  Within MFC we can do that by 
			// getting the threadstate object because it has the "last sent message" member, 
			// and that's exactly the object that we need to adjust.  So that's what we do.

			// First reset the correct .x and .y of the pt object.  
			if (GetStyle() & TBS_VERT) {
				// Here we do two things, we return pt to its original vertical coords, and at 
				// the same time set the .y to correspond to the new position of the thumb.
				pt.x = point.x;
				pt.y = rcClickable.left + (long)((double)(nTickPos - nMin) * dblTickWidth);
			} else {
				// Here we only do one thing, we set the .x to correspond to the new position 
				// of the thumb.  We don't have to swap coords back, because we didn't swap 
				// them in the first place.  But for consistency we set .y anyway (even though 
				// this is guaranteed not to change the value of pt.y)
				pt.x = rcClickable.left + (long)((double)(nTickPos - nMin) * dblTickWidth);
				pt.y = point.y; // No effect because we didn't swap in the first place
			}

			// Now at this point we know we changed either pt.x or pt.y to correspond to the 
			// position of the new thumb, so we get the thread state and adjust the last sent 
			// message's lParam to refer to the new mouse coordinates, so the slider thinks 
			// that's where the user put the mouse button down.
			_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
			ASSERT(pThreadState);
			if (pThreadState) {
				pThreadState->m_lastSentMsg.lParam = MAKELONG(pt.x, pt.y);
			}
		} else {
			// Here we set the pt back to being identical to point.  This only has an effect 
			// if we're in a vertically-oriented slider, because that's the only time we would 
			// swap the .x and .y coords.  Also, this doesn't really matter anyway because 
			// when we call the base class implementation it doesn't actually use the second 
			// parameter, it pulls it from the last sent message in the thread state, which 
			// we didn't change.  So again, this is unnecessary but it's here for consistency.
			pt = point;
		}

		// Call the base class (this is important, not just for consistency but because the 
		// slider needs to do things, like take focus as well as (most importantly) take 
		// capture if the mouse was clicked inside the thumb).
		CSliderCtrl::OnLButtonDown(nFlags, pt);
	} else {
		// Nope, we're just doing normal thumb drag behavior, so just call the base class.
		CSliderCtrl::OnLButtonDown(nFlags, point);
	}
}

BOOL CNxSliderCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	//TES 10/1/2008 - PLID 31504 - The new version of the Microsoft Common Controls (version 6) which we now use behaves
	// differently when using the scrollwheel on a slider.  Rather than moving the slider one tick per scroll wheel notch, 
	// it now sometimes moves it more (as many as 5 ticks) if the slider is very compressed.  We (and our clients) like the old
	// way better, so we will just implement it ourselves.
	CWnd *pParent = GetParent();
	if(pParent) {
		//TES 10/1/2008 - PLID 31504 - Offset our position by the number of scroll wheel notches passed to us in zDelta.
		int nPos = GetPos();
		nPos -= zDelta/WHEEL_DELTA;
		SetPos(nPos);

		//TES 10/1/2008 - PLID 31504 - Now, notify our parent that the scroll position has changed.
		if (GetStyle() & TBS_VERT) {
			pParent->PostMessage(WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, nPos), (LPARAM)GetSafeHwnd());
		}
		else {
			pParent->PostMessage(WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, nPos), (LPARAM)GetSafeHwnd());
		}
		return TRUE;
	}
	else {
		//TES 10/1/2008 - PLID 31504 - We don't have a parent!  That should never happen, so let's just revert to the default.
		return CSliderCtrl::OnMouseWheel(nFlags, zDelta, pt);
	}
}
