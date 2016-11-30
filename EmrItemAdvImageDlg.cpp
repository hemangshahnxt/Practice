// EmrItemAdvImageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EmrItemAdvImageDlg.h"
#include "SelectImageDlg.h"
#include "EMR.h"
#include "EMN.h"
#include "EMRTopic.h"
#include "GlobalDrawingUtils.h"
#include "FileUtils.h"
#include "Mirror.h"
#include "EmrTopicWnd.h"
#include "NxInkPictureText.h"
#include "EmrTreeWnd.h"
#include "ImageArray.h"
#include "WindowlessUtils.h"
#include "StampSearchDlg.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include "TopazSigPad.h"
#include "EMNDetail.h"
#include "PatientDocumentStorageWarningDlg.h"
#include "HistoryUtils.h"
#include "NxImageCache.h"
#include "EmrItemAdvImageState.h"	// (j.armen 2014-07-21 16:32) - PLID 62836

extern CPracticeApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2011-11-11 11:11) - PLID 46632 - WindowlessUtils - Various functions replaced with windowless-safe versions.

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace NXINKPICTURELib;

// (c.haag 2007-10-09 13:08) - PLID 27599 - Replaced most calls
// to m_pInkPicture->ReadOnly with calls to UpdateInkPictureReadOnlyState()

/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvImageDlg dialog

// (j.jones 2010-04-05 17:03) - PLID 38056 - set the dimensions for a reasonable maximum
// default width & height of new image items

// (a.walling 2011-05-25 17:57) - PLID 43847 - Modified the limits a bit from (500,500) now that stamps are a big thing
const CSize g_cszMaxDefaultImageSize(1024, 768);


CEmrItemAdvImageDlg::CEmrItemAdvImageDlg(class CEMNDetail *pDetail)
	: CEmrItemAdvDlg(pDetail)
{
	m_szMinSize.cx = m_szMinSize.cy = 0;
	m_bEnableTooltips = false;
	m_pInkPicture = NULL;

	m_bIsEraserState = FALSE;

	// (c.haag 2007-10-09 13:07) - PLID 27599 - We always accept ink
	// input by default
	m_bEnableInkInput = TRUE;

	// (c.haag 2008-06-03 14:15) - PLID 27777 - TRUE if the topic is
	// visible (default value is TRUE)
	m_bIsTopicVisible = TRUE;
}

BEGIN_MESSAGE_MAP(CEmrItemAdvImageDlg, CEmrItemAdvDlg)
	//{{AFX_MSG_MAP(CEmrItemAdvImageDlg)
	ON_WM_CREATE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_GETMINMAXINFO()
	ON_WM_DROPFILES()
	ON_MESSAGE(NXM_EMN_ON_SHOW_TOPIC, OnShowTopic)
	ON_MESSAGE(NXM_EMR_TOGGLE_HOT_SPOT_SELECTION, OnToggleHotSpot)
	ON_WM_MOUSEACTIVATE()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// (z.manning, 04/14/2008) - PLID 29632 - Use the defined IDC rather than a hardcoded value
BEGIN_EVENTSINK_MAP(CEmrItemAdvImageDlg, CEmrItemAdvDlg)
    //{{AFX_EVENTSINK_MAP(CEmrItemAdvImageDlg)
	ON_EVENT(CEmrItemAdvImageDlg, INK_PICTURE_IDC, 2 /* Stroke */, OnStrokeInkPicture, VTS_DISPATCH VTS_DISPATCH VTS_PBOOL)
	ON_EVENT(CEmrItemAdvImageDlg, INK_PICTURE_IDC, 3 /* Browse */, OnBrowseInkPicture, VTS_PBOOL)
	ON_EVENT(CEmrItemAdvImageDlg, INK_PICTURE_IDC, 4 /* FullScreen */, OnFullScreenInkPicture, VTS_NONE)
	ON_EVENT(CEmrItemAdvImageDlg, INK_PICTURE_IDC, 5 /* TextChanged */, OnTextChangedInkPicture, VTS_I4)
	ON_EVENT(CEmrItemAdvImageDlg, INK_PICTURE_IDC, 6 /* CustomStampsChanged */, OnCustomStampsChangedInkPicture, VTS_BSTR)
	ON_EVENT(CEmrItemAdvImageDlg, INK_PICTURE_IDC, 7 /* ClickedHotSpot */, OnClickedHotSpotInkPicture, VTS_I4)
	ON_EVENT(CEmrItemAdvImageDlg, INK_PICTURE_IDC, 8 /* OpenStampSetup */, OnOpenStampSetupInkPicture, VTS_NONE)
	ON_EVENT(CEmrItemAdvImageDlg, INK_PICTURE_IDC, 11 /* OpenStampFilterSetup */, OnOpenStampFilterSetupInkPicture, VTS_NONE)
	ON_EVENT(CEmrItemAdvImageDlg, INK_PICTURE_IDC, 12 /* OpenStampSearch */, OnOpenStampSearch, VTS_NONE)
	ON_EVENT(CEmrItemAdvImageDlg, INK_PICTURE_IDC, 13 /* Topaz Signature */, OnClickedTopazSignature, VTS_NONE)
	// (j.armen 2014-07-22 09:03) - PLID 62836 - Handle PenColorChanged and PenSizeChanged
	ON_EVENT(CEmrItemAdvImageDlg, INK_PICTURE_IDC, 14 /* CurrentPenColorChanged */, OnCurrentPenColorChanged, VTS_NONE)
	ON_EVENT(CEmrItemAdvImageDlg, INK_PICTURE_IDC, 15 /* CurrentPenSizeChanged */, OnCurrentPenSizeChanged, VTS_NONE)
	// (r.gonet 05/31/2011) - PLID 43896 - Put back in when we want to save zoom and pan offsets.
	//ON_EVENT(CEmrItemAdvImageDlg, INK_PICTURE_IDC, 9 /* ZoomLevelChanged */, OnZoomLevelChangedInkPicture, VTS_R8)
	//ON_EVENT(CEmrItemAdvImageDlg, INK_PICTURE_IDC, 10 /* ViewportOffsetsChanged */, OnViewportOffsetsChangedInkPicture, VTS_I4 VTS_I4)
	ON_EVENT(CEmrItemAdvImageDlg, NX3D_IDC, 1 /* HotSpotClicked */, OnClickedHotSpot3DControl, VTS_I2)
	ON_EVENT(CEmrItemAdvImageDlg, NX3D_IDC, 3 /* FullScreen */, OnFullScreen3DControl, VTS_NONE)
	ON_EVENT(CEmrItemAdvImageDlg, NX3D_IDC, 19 /* SmartStampAdd */, OnSmartStampAdd3DControl, VTS_NONE)
	ON_EVENT(CEmrItemAdvImageDlg, NX3D_IDC, 20 /* SmartStampErase */, OnSmartStampErase3DControl, VTS_NONE)
	ON_EVENT(CEmrItemAdvImageDlg, NX3D_IDC, 23 /* OpenStampSetup */, OnOpenStampSetup3DControl, VTS_NONE)
	ON_EVENT(CEmrItemAdvImageDlg, NX3D_IDC, 31 /* OpenStampFilterSetup */, OnOpenStampFilterSetup3DControl, VTS_NONE)
	ON_EVENT(CEmrItemAdvImageDlg, NX3D_IDC, 24 /* PreviewModified */, OnPreviewModified3DControl, VTS_NONE)
	ON_EVENT(CEmrItemAdvImageDlg, NX3D_IDC, 37 /* OpenStampSearch */, OnOpenStampSearch3D, VTS_NONE) // (j.gruber 2012-08-14 12:23) - PLID 52134
	
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvImageDlg message handlers

int CEmrItemAdvImageDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		if (CEmrItemAdvDlg::OnCreate(lpCreateStruct) == -1)
			return -1;

		// (c.haag 2008-06-03 14:18) - PLID 27777 - Track the parent's visibilty
		if (NULL != m_pDetail && NULL != m_pDetail->m_pParentTopic) {
			CEmrTopicWndPtr pWnd = m_pDetail->m_pParentTopic->GetTopicWnd();
			// (a.walling 2012-02-22 14:53) - PLID 48320
			if (pWnd && IsWindow(pWnd->GetSafeHwnd())) {
				m_bIsTopicVisible = (pWnd->IsWindowVisible()) ? TRUE : FALSE;
			}
		}
		
		//TES 5/6/2008 - PLID 27829 - Don't allow dragging onto templates.
		//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
		DragAcceptFiles(!m_pDetail->GetReadOnly() && !m_pDetail->m_bIsTemplateDetail);

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

		// (z.manning 2011-10-25 13:04) - PLID 39401 - Pass in the detail
		CString strStamps = GenerateInkPictureStampInfo(m_pDetail);
		
		if(m_pDetail->Is3DImage())
		{
			// (z.manning 2011-07-21 15:50) - PLID 44649 - Added support for 3D images
			// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
			if(!m_wndInkPic.CreateControl(__uuidof(Nx3DLib::Nx3D), NULL, WS_CHILD|WS_VISIBLE|WS_TABSTOP, CRect(0, 0, 0, 0), this, NX3D_IDC)) {
				ASSERT(FALSE);
				AfxThrowNxException("Failed to create wnd for 3D control");
			}
			if (m_wndInkPic.GetSafeHwnd()) {
				m_wndInkPic.ModifyStyleEx(WS_EX_NOPARENTNOTIFY, 0);
			}
			m_p3DControl = m_wndInkPic.GetControlUnknown();

			m_p3DControl->SetTextDataVersion(CURRENT_TEXT_STRUCTURE_VERSION);

			// (j.armen 2011-09-06 15:44) - PLID 45245 - Implemented BackGround Color
			m_p3DControl->BackColor = GetBackgroundColor();

			// (j.armen 2011-09-15 14:50) - PLID 45424 - Set the background of the opengl area
			m_p3DControl->GLBackColor = (COLORREF)GetRemotePropertyInt("Nx3DBackgroundColor", GetNxColor(GNC_EMR_ITEM_BG, 0), 0, GetCurrentUserName(), true);

			// (j.armen 2011-09-06 17:18) - PLID 45347 - Implemented Full Screen
			m_p3DControl->IsFullScreen = VARIANT_FALSE;

			m_p3DControl->UseCustomStamps = (GetRemotePropertyInt("EMR_Image_Use_Custom_Stamps", 1, 0, GetCurrentUserName(), TRUE) == 1 ? VARIANT_TRUE : VARIANT_FALSE);
			m_p3DControl->CustomStamps = _bstr_t(strStamps); // (z.manning 2011-09-07 16:53) - PLID 44693

			// (j.armen 2011-10-10 12:43) - PLID 45849
			m_p3DControl->UseCustomViews = VARIANT_TRUE;
				
			// (j.armen 2011-10-10 16:56) - PLID 45245
			m_p3DControl->IsItemSetup = VARIANT_FALSE;
		}
		else
		{
			// Create the InkPicture control
			//TES 1/29/2008 - PLID 28673 - Use a #defined IDC, not a numeric literal.
			// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
			m_wndInkPic.CreateControl(__uuidof(NxInkPicture), NULL, WS_CHILD|WS_VISIBLE|WS_TABSTOP, 
				CRect(0, 0, 0, 0), 
				this, INK_PICTURE_IDC);

			if (m_wndInkPic.GetSafeHwnd()) {
				m_wndInkPic.ModifyStyleEx(WS_EX_NOPARENTNOTIFY, 0);
			}

			m_pInkPicture = m_wndInkPic.GetControlUnknown();
			// If we are using a later version of the NxInkPicture control, then we need to force it to not save data
			//  that we can't load with our own statically linked NxUILib's CNxInkPictureText class.
			m_pInkPicture->SetTextDataVersion(CURRENT_TEXT_STRUCTURE_VERSION);
			//DRT 4/29/2008 - PLID 29771 - Set the background color appropriately.
			m_pInkPicture->BackColor = GetBackgroundColor();

			// (j.jones 2010-04-12 09:30) - PLID 16594 - tell the ocx that we are not using a signature date
			m_pInkPicture->PutEnableSignatureDateStamp(VARIANT_FALSE);

			// (c.haag 2014-07-17) - PLID 50572 - Special handling if this is a signature detail
			if (m_pDetail->IsSignatureDetail())
			{
				m_pInkPicture->ViewOnly = VARIANT_TRUE;
			}

			// (a.wetta 2007-04-06 09:36) - PLID 24324 - Load the custom stamps
			// (c.haag 2014-07-17) - PLID 50572 - Special handling if this is a signature detail
			m_pInkPicture->UseCustomStamps = m_pDetail->IsSignatureDetail() ? VARIANT_FALSE : (GetRemotePropertyInt("EMR_Image_Use_Custom_Stamps", 1, 0, GetCurrentUserName(), TRUE) == 1 ? VARIANT_TRUE : VARIANT_FALSE);

			// (j.jones 2010-02-10 14:26) - PLID 37312 - tell the ocx that we control stamp setup
			m_pInkPicture->PutApplicationHandlesStampSetup(VARIANT_TRUE);

			// (j.jones 2010-02-18 10:01) - PLID 37423 - tell the ocx whether it is a SmartStamp image,
			// and if it is, whether the preference is enabled to fire hotspots when stamping on them
			SetIsSmartStampImage(m_pDetail->GetSmartStampTableDetail() ? TRUE : FALSE);

			// (j.jones 2010-04-08 09:35) - PLID 38097 - tell the control whether the preference is enabled to fire hotspots when stamping on them,
			// this used to be SmartStamp only but now it applies to all stamps, so disregard the SmartStamp ConfigRT naming
			m_pInkPicture->PutStampFireHotspotClick(GetRemotePropertyInt("EMR_AddSmartStampOnHotSpot", 0, 0, GetCurrentUserName(), true) == 1 ? VARIANT_TRUE : VARIANT_FALSE);
			
			// (j.jones 2010-02-10 11:11) - PLID 37224 - stamps are now tracked globally, just call the
			// EmrUtils function to generate the stamp info that is readable by the ink picture			
			// (c.haag 2014-07-17) - PLID 50572 - Special handling if this is a signature detail
			m_pInkPicture->CustomStamps = m_pDetail->IsSignatureDetail() ? "" : _bstr_t(strStamps);

			// Assign the default pen color
			// (j.armen 2014-07-23 14:57) - PLID 62837 - If the detail has a default pen color, override the preference
			if (m_pDetail->GetDefaultPenColor())
				m_pInkPicture->DefaultPenColor = *m_pDetail->GetDefaultPenColor();
			else
				m_pInkPicture->DefaultPenColor = GetRemotePropertyInt("DefaultEMRImagePenColor", RGB(255, 0, 0), 0, GetCurrentUserName(), TRUE);

			// (j.armen 2014-07-23 14:57) - PLID 62837
			// Assign the default pen size
			if (m_pDetail->GetDefaultPenSizePercent())
				m_pInkPicture->CurrentPenSizePercent = *m_pDetail->GetDefaultPenSizePercent();

			// (r.gonet 05/06/2011) - PLID 43542 - Set whether text should scale
			// (z.manning 2011-11-21 10:26) - PLID 46558 - This now only checks the preference
			m_pInkPicture->EnableTextScaling = GetRemotePropertyInt("EnableEMRImageTextScaling", 1, 0, "<None>", true) == 1 ? VARIANT_TRUE : VARIANT_FALSE;		

			// (b.spivey, May 02, 2013) - PLID 56542
			m_pInkPicture->SetTopazSigPadConnected(GetMainFrame()->GetIsTopazConnected());

			RefreshHotSpots(m_pInkPicture, m_pDetail);
		}
		
		UpdateInkPictureReadOnlyState();

		return 0;

	} NxCatchAll("Error in OnCreate");

	return -1;
}

BOOL CEmrItemAdvImageDlg::RepositionControls(IN OUT CSize &szArea, BOOL bCalcOnly)
{
	try {
		CEmrItemAdvDlg::RepositionControls(szArea, bCalcOnly);

		CClientDC dc(this);

		// (a.walling 2011-05-19 18:07) - PLID 43843 - This is absolute insanity, and eventually
		// I ended up having to rewrite most of it. Oddly enough I predicted I would be doing so exactly
		// one year before this point.

		// Make sure the merge status icon button reflects the state of the data,
		// because that will have a direct influence on our size calculations.
		UpdateStatusButtonAppearances();

		// The caller is giving us a full window area so we have to adjust off the 
		// border to get what will be our client area.
		const long cnMergeBtnMargin = 5; // Amount of space above and below the merge button
		const CPoint ptOffset(6, 2);

		// the border of the window
		CSize szBorder;
		// The label at the top left
		CSize szLabel(LONG_MAX, LONG_MAX);
		// The merge icon button below the picture
		CSize szMergeBtn(LONG_MAX, LONG_MAX);
		// the minimum content size
		CSize szMinimumContent;
		// The size of the content (the ink image control)
		CSize szContent;
		// the available client area (szArea - szBorder)
		CSize szClient;

		// Get the border size
		CalcWindowBorderSize(this, &szBorder);

		// Update the client area minus the border
		szClient.SetSize(szArea.cx - szBorder.cx, szArea.cy - szBorder.cy);

		// Update the client area minus the offset
		szClient.cx -= ptOffset.x;
		szClient.cy -= ptOffset.y;
		
		// (c.haag 2005-01-21 12:06) - PLID 14950 - Make sure the area is never negative
		szClient.SetSize(max(0, szClient.cx), max(0, szClient.cy));

		// get the size of the bottom buttons
		if (IsMergeButtonVisible() || IsProblemButtonVisible()) {
			CalcControlIdealDimensions(&dc, m_pBtnMergeStatus, szMergeBtn);
		} else {
			szMergeBtn = CSize(0,0);
		}

		// now get the minimum possible size
		szMinimumContent.SetSize(0, 0);
		if(m_pInkPicture != NULL) {
			m_pInkPicture->GetMinimumDimensions((long*)&szMinimumContent.cx, (long*)&szMinimumContent.cy);
		}
		else if(m_p3DControl != NULL) {
			m_p3DControl->GetMinimumDimensions(&szMinimumContent.cx, &szMinimumContent.cy);
		}
		
		// now that we have the provisional width so figure out the size of the label
		if (!IsControlValid(&m_wndLabel)) {
			szLabel.SetSize(0, 0);
		} else {
			CalcControlIdealDimensions(&dc, &m_wndLabel, szLabel);

			//check if the label is too wide to be displayed
			if(szLabel.cx > szClient.cx) {
				szLabel.cx = szClient.cx;
				szLabel.cy = 0;
				CalcControlIdealDimensions(&dc, &m_wndLabel, szLabel, TRUE);
			}
		}

		if (bCalcOnly) {
			// bCalcOnly means get the best fit for the item.
			
			// (a.walling 2011-05-25 17:57) - PLID 43847 - Constrain to a reasonable limit
			szContent = g_cszMaxDefaultImageSize;
			if(m_pInkPicture != NULL) {
				m_pInkPicture->GetIdealDimensionsForControl((long*)&szContent.cx, (long*)&szContent.cy);
			}
			else if(m_p3DControl != NULL) {
				m_p3DControl->GetIdealDimensionsForControl(&szContent.cx, &szContent.cy);
			}

			if(m_pDetail->GetItemAdvDlgScalingFactor() != 100)
			{
				// (z.manning 2011-09-23 16:06) - PLID 42648 - we have a scaling factor other than 100% so let's adjust
				// the ideal size accordingly.
				double dNewWidth = (double)szContent.cx * ((double)m_pDetail->GetItemAdvDlgScalingFactor() / 100.0);
				double dNewHeight = (double)szContent.cy * ((double)m_pDetail->GetItemAdvDlgScalingFactor() / 100.0);
				int nNewWidth = (int)dNewWidth;
				int nNewHeight = (int)dNewHeight;
				szContent.SetSize(nNewWidth, nNewHeight);
			}

			CSize szAdjustedContent(max(szContent.cx, szMinimumContent.cx), max(szContent.cy, szMinimumContent.cy));

			if (szAdjustedContent != szContent) {
				szContent = szAdjustedContent;
				if(m_pInkPicture != NULL) {
					m_pInkPicture->GetIdealDimensionsForControl((long*)&szContent.cx, (long*)&szContent.cy);
				}
				else if(m_p3DControl != NULL) {
					m_p3DControl->GetIdealDimensionsForControl(&szContent.cx, &szContent.cy);
				}
			}

			// of course we may need to adjust the label size again
			if (szContent.cx < szLabel.cx && IsControlValid(&m_wndLabel)) {		
				szLabel.cx = szContent.cx;
				szLabel.cy = 0;
				CalcControlIdealDimensions(&dc, &m_wndLabel, szLabel, TRUE);
			}
		} else {
			// Otherwise we simply squeeze into what we are given!

			// figure out our available size
			// the size available within szArea for the content
			szContent = szClient;
			szContent.cy -= szLabel.cy;
			szContent.cy -= cnMergeBtnMargin;
			szContent.cy -= szMergeBtn.cy;
			szContent.cy -= cnMergeBtnMargin;

			szContent.SetSize(max(szContent.cx, szMinimumContent.cx), max(szContent.cy, szMinimumContent.cy));
		}

		// now we have the various sizes we require.
		CPoint currentOffset = ptOffset;
		CRect rcLabel(currentOffset, szLabel);
		currentOffset.y += rcLabel.Height();

		CRect rcContent(currentOffset, szContent);
		currentOffset.y += rcContent.Height();

		currentOffset.y += cnMergeBtnMargin;
		CRect rcMergeButton(currentOffset, szMergeBtn);
		CRect rcProblemButton(currentOffset + CPoint(3, 0), szMergeBtn);		
		// currentOffset.y += cnMergeBtnMargin; // I think this is unnecessary

		CRect rcClient(0, 0, szContent.cx, currentOffset.y);

		// add the borders and padding in to get the final area
		CSize szIdeal(
			rcClient.Width() + szBorder.cx + ptOffset.x,
			rcClient.Height() + szBorder.cy + ptOffset.y + szLabel.cy + szMergeBtn.cy
		);
		
		m_szMinSize.SetSize(
			szMinimumContent.cx  + szBorder.cx + ptOffset.x,
			szMinimumContent.cy + szBorder.cy + ptOffset.y + szLabel.cy + szMergeBtn.cy
		);

		if (!bCalcOnly) {
			bool bMoved = false;
			// (b.cardillo 2006-03-09 17:41) - PLID 19648 - Move the ink picture control, but only 
			// if its position or size is changing.
			CRect rcCur(0,0,0,0);
			m_wndInkPic.GetWindowRect(&rcCur);
			ScreenToClient(&rcCur);
			if (rcCur != rcClient) {
				bMoved = true;
				m_wndInkPic.MoveWindow(rcContent, FALSE);
			}
			// (c.haag 2006-07-03 09:29) - PLID 19944 - We now use the variable x to calculate
			// the correct position of each iconic button
			int x = 2;
			if (IsMergeButtonVisible()) {
				m_pBtnMergeStatus->GetWindowRect(&rcCur);
				ScreenToClient(&rcCur);
				if (rcCur != rcMergeButton) {
					bMoved = true;
					m_pBtnMergeStatus->MoveWindow(rcMergeButton, FALSE);
				}
			}
			// (c.haag 2006-06-30 16:48) - PLID 19944 - Move the problem status button, too
			if (IsProblemButtonVisible()) {				
				m_pBtnProblemStatus->GetWindowRect(&rcCur);
				ScreenToClient(&rcCur);
				if (rcCur != rcProblemButton) {
					bMoved = true;
					m_pBtnProblemStatus->MoveWindow(rcProblemButton, FALSE);
				}
			}

			if (IsControlValid(&m_wndLabel)) {
				GetControlChildRect(this, &m_wndLabel, &rcCur);

				if (rcCur != rcLabel) {
					bMoved = true;
					m_wndLabel.MoveWindow(rcLabel, FALSE);
				}
			}

			if (bMoved) {				
				RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_ERASE);
			}
		
			// (a.walling 2012-06-14 14:36) - PLID 49297 - Don't do this, we don't have tabs in the same sense any longer
			//BringWindowToTop();
		}
		
		// Determine our return value
		BOOL bAns;
		if (szArea.cx >= szIdeal.cx && szArea.cy >= szIdeal.cy) {
			bAns = TRUE;
			szArea.SetSize(min(szArea.cx, szIdeal.cx), min(szArea.cy, szIdeal.cy));
		} else {
			bAns = FALSE;
			szArea = szIdeal;
		}
		
		// (a.walling 2011-05-31 13:32) - PLID 43843
		szArea.SetSize(max(m_szMinSize.cx, szArea.cx), max(m_szMinSize.cy, szArea.cy));

		return bAns;
	}NxCatchAll("Error in CEmrItemAdvImageDlg::RepositionControls");
	return FALSE;
}

void CEmrItemAdvImageDlg::OnStrokeInkPicture(LPDISPATCH Cursor, LPDISPATCH Stroke, BOOL FAR* Cancel) 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		UpdateRememberedPenState();
		
		//Tell our detail.
		m_pDetail->SetInkData(m_pInkPicture->InkData);

		//for auditing, track what this stroke was
		if(!m_bIsEraserState)
			m_pDetail->SetInkAdded();
		else
			m_pDetail->SetInkErased();
	} NxCatchAll("Error in OnStrokeInkPicture");
}

void CEmrItemAdvImageDlg::OnBrowseInkPicture(BOOL FAR* pbProcessed)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		*pbProcessed = TRUE; //We've handled the browsing, so the control doesn't need to.

		// (a.wetta 2007-02-26 17:15) - PLID 23816 - If a new image is selected for the detail, make sure the new image is sized to the detail
		// if the preference is selected.  But if this is a blank built-in image detail, then we always want it to use the images original size
		// (a.walling 2011-05-25 17:57) - PLID 43847 - This is now no longer a property of the detail.
		/*if (m_pDetail->m_nEMRInfoID != EMR_BUILT_IN_INFO__IMAGE || HasValidImage())
			m_pDetail->SetSizeImageToDetail(GetRemotePropertyInt("EMR_SizeImageToDetail", 1, 0, GetCurrentUserName(), true));*/
		
		// (a.walling 2011-05-25 17:57) - PLID 43847 - Instead, manually call the RestoreIdealSize if necessary.
		/*bool bRestoreIdealSize = (
			(m_pDetail->m_nEMRInfoID == EMR_BUILT_IN_INFO__IMAGE) 
			|| 
			!HasValidImage() 
			|| 
			!GetRemotePropertyInt("EMR_SizeImageToDetail", 1, 0, GetCurrentUserName(), true) 
		);*/
		// (a.walling 2011-08-01 17:04) - PLID 44743 - This was just confusing in many situations. To simplify, we either resize, or we don't.
		bool bRestoreIdealSize = !GetRemotePropertyInt("EMR_SizeImageToDetail", 1, 0, GetCurrentUserName(), true);

		if (m_pDetail->RequestContentChange(ITEM_ADD)) {
			// (b.cardillo 2004-05-05 10:19) - We used to incorrectly call OnStrokeInkPicture here as a quick-fix to the 
			// problem of images not saving correctly if they had never been drawn on.  This quick-fix was incorrectly 
			// being called from both the ReflectCurrentState function and the OnBrowseBtn function.
		}

		// (a.walling 2011-05-25 17:57) - PLID 43847 - Instead, manually call the RestoreIdealSize if necessary.
		if (bRestoreIdealSize) {
			RestoreIdealSize(true);
		}

		//m_pDetail->SetSizeImageToDetail(FALSE);
	} NxCatchAll("Error in OnBrowseInkPicture");
}

void CEmrItemAdvImageDlg::OnFullScreenInkPicture()
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		UpdateRememberedPenState();

		m_pDetail->Popup();
	// (j.jones 2014-10-23 14:33) - PLID 62836 - changed to use __FUNCTION__
	} NxCatchAll(__FUNCTION__);
}

// (j.armen 2011-09-06 17:18) - PLID 45347 - Popup the window when event is fired
void CEmrItemAdvImageDlg::OnFullScreen3DControl()
{
	try {
		m_pDetail->Popup();
	// (j.jones 2014-10-23 14:33) - PLID 62836 - changed to use __FUNCTION__, it used to say "Error in OnFullScreenInkPicture"
	} NxCatchAll(__FUNCTION__);
}

void CEmrItemAdvImageDlg::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		CEmrItemAdvDlg::OnWindowPosChanged(lpwndpos);
	} NxCatchAll("Error in OnWindowPosChanged");
}

void CEmrItemAdvImageDlg::ReflectCurrentState()
{
	try {

		CEmrItemAdvDlg::ReflectCurrentState();

		// Calculate the image source
		CString strFullPath = m_pDetail->GetImagePath(true);

		if(m_pInkPicture)
		{
			try {

				// Update the ink color, size, state
				m_pInkPicture->PutIsEraserState(m_bIsEraserState);

				// Update the ink strokes
				m_pInkPicture->PutInkData(m_pDetail->GetInkData());

				//TES 11/7/2006 - PLID 18159 - Also the text data.
				m_pInkPicture->PutTextData(m_pDetail->GetImageTextData());

				// (r.gonet 02/13/2012) - PLID 37682 - If there is a filter, assign it.
				m_pDetail->ReflectImageTextStringFilter();

			}NxCatchAll("Error in ReflectCurrentState() - Setting Ink");

			for(int nHotSpotIndex = 0; nHotSpotIndex < m_pDetail->GetHotSpotArray()->GetSize(); nHotSpotIndex++)
			{
				CEMRHotSpot *pSpot(m_pDetail->GetHotSpotArray()->GetAt(nHotSpotIndex));
				m_pInkPicture->SetHotSpotState(pSpot->GetID(), pSpot->GetSelected() ? VARIANT_TRUE : VARIANT_FALSE);
			}
			
			//Now, only proceed if the picture has changed.
			if(strFullPath != m_strCurrentImageFile)
			{
				m_strCurrentImageFile = strFullPath;
				
				// Set the image to that path
				if (m_pDetail->GetImageType() == itMirrorImage)
				{
					// (c.haag 2005-03-07 16:44) - Get the Mirror image and assign it to the
					// picture control
					// (a.walling 2011-05-02 15:41) - PLID 43530 - Cache the mirror image as well (loading duplicate full-res images takes time and a lot of memory)
					//g_EmrImageCache.GetCachedImage(CString("mirror://") + strFullPath, m_pCachedImage);
					//
					//HBITMAP hbmp = m_pCachedImage ? m_pCachedImage->GetImage() : NULL;
					//m_pInkPicture->Image = (OLE_HANDLE)hbmp;
					// (a.walling 2013-06-27 13:21) - PLID 57348 - Support NxImageLib::ISourceImage
					NxImageLib::ISourceImagePtr pSource = NxImageLib::Cache::OpenSourceImage(CString("mirror://") + strFullPath);
					if (pSource && SUCCEEDED(pSource->raw_Load())) {
						m_pInkPicture->Source = (IUnknown*)(pSource);
					} else {
						m_pInkPicture->Image = (OLE_HANDLE)NULL;
					}
				}
				// (a.walling 2011-05-02 15:25) - PLID 43530 - We should only do this if we are NOT a mirror image! The 'else' was missing.
				else if (strFullPath.GetLength())// && FileUtils::DoesFileOrDirExist(strFullPath))
				{
					try {						
						// (a.walling 2010-10-25 10:04) - PLID 41043 - Get a cached image reference and use that instead
						/*g_EmrImageCache.GetCachedImage(strFullPath, m_pCachedImage);

						HBITMAP hbmp = m_pCachedImage ? m_pCachedImage->GetImage() : NULL;
						m_pInkPicture->Image = (OLE_HANDLE)hbmp;*/
						// (a.walling 2013-06-27 13:21) - PLID 57348 - Support NxImageLib::ISourceImage						
						NxImageLib::ISourceImagePtr pSource = NxImageLib::Cache::OpenSourceImage(strFullPath);
						if (pSource && SUCCEEDED(pSource->raw_Load())) {
							m_pInkPicture->Source = (IUnknown*)(pSource);
						} else {
							m_pInkPicture->Image = (OLE_HANDLE)NULL;
						}
						m_pInkPicture->PictureFileName = pSource ? _bstr_t(strFullPath) : _bstr_t("");

					}NxCatchAll("Error in ReflectCurrentState() - Setting Image");
				}
				else {
					m_pInkPicture->PictureFileName = _bstr_t("");
					m_pInkPicture->Image = NULL;
				}

				// (b.cardillo 2004-05-05 10:19) - We used to incorrectly call OnStrokeInkPicture here as a quick-fix to the 
				// problem of images not saving correctly if they had never been drawn on.  This quick-fix was incorrectly 
				// being called from both the ReflectCurrentState function and the OnBrowseBtn function.
				
				// And recalc everything
				CRect rcWindow;
				GetWindowRect(&rcWindow);
				RepositionControls(CSize(rcWindow.Width(), rcWindow.Height()), FALSE);
				//m.hancock - 3/2/2006 - PLID 19521 - If we can get the topic's window, then we can resize the
				//image item and maintain proper placement with regards to other details.
				// (j.jones 2006-09-19 15:41) - PLID 22501 - Don't resize if this is the initial load
				if(!m_bIsLoading && m_pDetail->m_pParentTopic->GetTopicWnd()) {
					// (j.jones 2007-07-26 09:31) - PLID 24686 - renamed this function
					// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
					m_pDetail->m_pParentTopic->GetTopicWnd()->RepositionDetailsInTopicByInfoID(m_pDetail->m_nEMRInfoID, FALSE);
				}

				// (r.gonet 05/31/2011) - PLID 43896 - Put back in when we want to save and restore zoom and pan offsets.
				/*if(m_pDetail->m_EMRInfoType == eitImage) {
					// (r.gonet 05/31/2011) - PLID 43896 
					m_pInkPicture->SetLoading(FALSE);
					double dZoomLevel = m_pDetail->GetZoomLevel();
					long nOffsetX = m_pDetail->GetOffsetX();
					long nOffsetY = m_pDetail->GetOffsetY();
					// (r.gonet 05/31/2011) - PLID 43896 - This is the first place where we know it is safe to zoom in on an image. All initial resizing has ceased.
					if(dZoomLevel != -1) {
						m_pInkPicture->SetZoomLevel(dZoomLevel);
						m_pInkPicture->SetViewportOffsets(nOffsetX, nOffsetY);
					} else {
						m_pInkPicture->ZoomToFit(TRUE);
					}
					Invalidate();
				}
				*/
			}
		}
		else if(m_p3DControl != NULL)
		{
			if(strFullPath != m_strCurrentImageFile)
			{
				if(!FileUtils::DoesFileOrDirExist(strFullPath)) {
					strFullPath.Empty();
				}
				// (z.manning 2011-07-22 16:53) - PLID 44649 - Load the image file if it changed
				m_strCurrentImageFile = strFullPath;
				m_p3DControl->PutModelFileName(_bstr_t(strFullPath));
			}
			

			for(int nHotSpotIndex = 0; nHotSpotIndex < m_pDetail->GetHotSpotArray()->GetSize(); nHotSpotIndex++)
			{
				CEMRHotSpot *pSpot(m_pDetail->GetHotSpotArray()->GetAt(nHotSpotIndex));
				m_p3DControl->SetHotSpotState(pSpot->Get3DHotSpotID(), pSpot->GetSelected() ? VARIANT_TRUE : VARIANT_FALSE);
			}

			// (z.manning 2011-09-09 12:25) - PLID 45335 - Update the stamp data
			m_p3DControl->PutTextData(m_pDetail->GetImageTextData());

			// (z.manning 2011-10-05 17:14) - PLID 45842 - Print data
			m_p3DControl->PutViewData(m_pDetail->GetImagePrintData());
		}

	} catch(CNxPersonDocumentException *pException) {
		// (d.thompson 2013-07-01) - PLID 13764 - Specially handle exceptions regarding the person documents
		CPatientDocumentStorageWarningDlg dlg(pException->m_strPath);
		//No longer need the exception, clean it up
		pException->Delete();

		//Inform the user
		dlg.DoModal();

	}NxCatchAll("Error in CEmrItemAdvImageDlg::ReflectCurrentState()");
}

void CEmrItemAdvImageDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// Call the normal implementation to ensure *lpMMI is filled properly
		CEmrItemAdvDlg::OnGetMinMaxInfo(lpMMI);

		// Then reduce the minimum to be no greater than our calculated minimum
		if (m_szMinSize.cx != 0 || m_szMinSize.cy != 0) {
			// Adjust minsize to include the window area, since it's stored in terms of client area.
			CSize szMin;
			{
				CSize szBorders;
				CalcWindowBorderSize(this, &szBorders);
				szMin = m_szMinSize + szBorders;
			}

			// Only change each direction of mintracksize if our value for that direction is larger 
			// than the current value (we don't want this code to reduce the minimum thus overriding 
			// what someone has determined the size must not go below).
			if (lpMMI->ptMinTrackSize.x < szMin.cx) {
				lpMMI->ptMinTrackSize.x = szMin.cx;
			}
			if (lpMMI->ptMinTrackSize.y < szMin.cy) {
				lpMMI->ptMinTrackSize.y = szMin.cy;
			}
		}
	} NxCatchAll("Error in OnGetMinMaxInfo");
}

void CEmrItemAdvImageDlg::UpdateInkPictureReadOnlyState()
{
	//
	// (c.haag 2007-10-09 12:59) - PLID 27599 - This function
	// sets the ink control to be read-only based on m_bReadOnly
	// and m_bEnableInkInput
	//
	if (IsWindow(GetSafeHwnd()))
	{
		VARIANT_BOOL vReadOnly;
		if(NULL != m_pInkPicture)
		{
			if (!m_bEnableInkInput) {
				// If ink input is disabled, override whatever m_bReadOnly
				// is and ensure that the control is read-only
				vReadOnly = VARIANT_TRUE;
			}
			else if (!m_bIsTopicVisible) {
				// (c.haag 2008-06-03 14:21) - PLID 27777 - If our parent topic
				// is invisible, then the overlay must be read-only
				vReadOnly = VARIANT_TRUE;
			}
			else {
				// If ink input is enabled, just check m_bReadOnly to
				// determine whether the ink control is read-only
				//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
				vReadOnly = (m_pDetail->GetReadOnly()) ? VARIANT_TRUE : VARIANT_FALSE;
			}
			m_pInkPicture->ReadOnly = vReadOnly;
		}
		// (z.manning 2011-09-14 11:29) - PLID 44649 - Handle the 3D control too
		else if(m_p3DControl != NULL)
		{
			vReadOnly = m_pDetail->GetReadOnly() ? VARIANT_TRUE : VARIANT_FALSE;
			m_p3DControl->SetReadOnly(vReadOnly);
		}
	}
}

//TES 3/23/2010 - PLID 37757 - This dialog doesn't maintain its own ReadOnly flag, so I changed the function name from
// SetReadOnly() to ReflectReadOnlyStatus()
void CEmrItemAdvImageDlg::ReflectReadOnlyStatus(BOOL bReadOnly)
{
	//TES 3/15/2010 - PLID 37757 - The detail now holds the ReadOnly status
	//m_bReadOnly = bReadOnly;
	if(GetSafeHwnd()) {
		m_wndLabel.EnableWindow(!m_pDetail->GetReadOnly());
		UpdateInkPictureReadOnlyState();
		//TES 5/6/2008 - PLID 27829 - Don't allow dragging onto templates.
		DragAcceptFiles(!m_pDetail->GetReadOnly() && !m_pDetail->m_bIsTemplateDetail);
	}

	CEmrItemAdvDlg::ReflectReadOnlyStatus(bReadOnly);
}

void CEmrItemAdvImageDlg::EnableInkInput(BOOL bEnable)
{
	//
	// (c.haag 2007-10-09 13:00) - PLID 27599 - This function
	// toggles the ability for the internal ink control to accept
	// ink strokes. If bEnable is FALSE, then the control will not
	// accept ink strokes even if m_bReadOnly is FALSE.
	//
	m_bEnableInkInput = bEnable;
	UpdateInkPictureReadOnlyState();
}

// (c.haag 2009-02-17 17:54) - PLID 31327 - Toggles for stamping on hotspots
BOOL CEmrItemAdvImageDlg::GetEnableHotSpotClicks()
{
	if (NULL != m_pInkPicture) {
		VARIANT_BOOL v = m_pInkPicture->GetEnableHotSpotClicks();
		if (VARIANT_FALSE == v) {
			return FALSE;
		} else {
			return TRUE;
		}
	}
	else if(m_p3DControl != NULL) {
		return TRUE;
	}
	else {
		ASSERT(FALSE);
		return FALSE;
	}
}

// (c.haag 2009-02-17 17:54) - PLID 31327 - Toggles for stamping on hotspots
void CEmrItemAdvImageDlg::SetEnableHotSpotClicks(BOOL bSet)
{
	if (NULL != m_pInkPicture) {
		m_pInkPicture->PutEnableHotSpotClicks( bSet ? VARIANT_TRUE : VARIANT_FALSE );
	}
	else if(m_p3DControl != NULL) {
	}
	else {
		ASSERT(FALSE);
	}
}

void CEmrItemAdvImageDlg::OnDropFiles(HDROP hDropInfo)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try
	{
		// (z.manning 2011-07-21 16:04) - PLID 44649 - We do not yet support this on 3D images
		if(m_pInkPicture == NULL) {
			return;
		}

		SetForegroundWindow();
		CString strFileName;
		int nFileCount = DragQueryFile(hDropInfo, -1, strFileName.GetBuffer(MAX_PATH),MAX_PATH);
		strFileName.ReleaseBuffer();
		
		if(nFileCount > 1) {
			MsgBox("You may not drag multiple files into an item.");
			DragFinish(hDropInfo);
			return;
		}
		else {
			DragQueryFile(hDropInfo, 0, strFileName.GetBuffer(MAX_PATH),MAX_PATH);
			strFileName.ReleaseBuffer();
			//Is it an image?
			// (a.walling 2007-07-25 13:40) - PLID 17467 - Use shared function
			if(!IsImageFile(strFileName)) {
				MsgBox("The specified file was not one that Practice recognizes as an image.");
				DragFinish(hDropInfo);
				return;
			}
		}

		// (j.jones 2006-07-12 16:36) - PLID 21421 - if there is already ink on the image,
		// warn the user it will be cleared out, then do so if they agree to proceed

		// To detect the ink status, first create an image state object based on the current detail state
		CEmrItemAdvImageState ais;
		ais.CreateFromSafeArrayVariant(m_pDetail->GetState());

		if((ais.m_varInkData.vt != VT_EMPTY && ais.m_varInkData.vt != VT_NULL) ||
			(ais.m_varTextData.vt != VT_EMPTY && ais.m_varTextData.vt != VT_NULL) ) {
			//there is some ink on this image, so warn the user
			if(IDNO == MessageBox("Selecting a new image will erase the ink you have currently drawn.\n"
				"Are you sure you wish to continue?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
		}

		// (a.walling 2012-03-16 11:22) - PLID 48946 - Check if we are already attached
		bool bAlreadyAttached = false;
		long nPatientID = ((CEMR*)m_pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR())->GetPatientID();

		{
			CString strPatDocPath = GetPatientDocumentPath(nPatientID);
			CString strLowerFileName = strFileName;
			strLowerFileName.MakeLower();

			strPatDocPath.TrimRight("\\");

			CString strPatDocPathPart = FileUtils::GetFileName(strPatDocPath);
			strPatDocPathPart.MakeLower();

			if (-1 != strLowerFileName.Find(strPatDocPathPart)) {
				CString strCanonical = strPatDocPath ^ FileUtils::GetFileName(strFileName);
				if (INVALID_FILE_ATTRIBUTES != ::GetFileAttributes(strCanonical)) {
					bAlreadyAttached = true;
				}
			}
		}

		if(bAlreadyAttached || (IDYES == MsgBox(MB_YESNO, "Are you sure you want to import the image %s into this patient's default documents folder, and set it as the image for this item?", strFileName))) {
			CWaitCursor cuWait;


			// (j.jones 2010-12-29 15:41) - PLID 41943 - importing may rename the file if the same filename
			// already exists, and since we may need to send that filename to the EMR, we need to track it
			CString strFinalFileName = "";
			
			// (j.jones 2010-10-29 12:11) - PLID 41187 - filled the optional parameters for posterity
			if(bAlreadyAttached || (ImportAndAttachFileToHistory(strFileName, nPatientID, GetSafeHwnd(), -1, -1, FALSE, "", &strFinalFileName))) {

				// (a.walling 2012-03-16 11:22) - PLID 48946
				if (bAlreadyAttached) {
					strFinalFileName = FileUtils::GetFileName(strFileName);
					strFinalFileName.Insert(0, '\\'); // for some reason
				}

				//clear out the ink (use the image state from above)
				VariantClear(&ais.m_varInkData);
				m_pDetail->SetInkErased();
				//TES 1/27/2007 - PLID 18159 - Also the text data.
				VariantClear(&ais.m_varTextData);
				m_pDetail->SetImageTextRemoved();

				//assign the new state
				m_pDetail->SetInkData(ais.m_varInkData);
				m_pDetail->SetImageTextData(ais.m_varTextData);

				// (a.wetta 2007-02-26 17:15) - PLID 23816 - If a new image is dragged to the detail, make sure the new image is sized to the detail
				// if the preference is selected.  But if this is a blank built-in image detail, then we always want it to use the images original size
				// (a.walling 2011-05-25 17:57) - PLID 43847 - This is now no longer a property of the detail.
				/*if (m_pDetail->m_nEMRInfoID != EMR_BUILT_IN_INFO__IMAGE || HasValidImage())
					m_pDetail->SetSizeImageToDetail(GetRemotePropertyInt("EMR_SizeImageToDetail", 1, 0, GetCurrentUserName(), true));*/
				
				// (a.walling 2011-05-25 17:57) - PLID 43847 - Instead, manually call the RestoreIdealSize if necessary.
				/*bool bRestoreIdealSize = (
					(m_pDetail->m_nEMRInfoID == EMR_BUILT_IN_INFO__IMAGE) 
					|| 
					!HasValidImage() 
					|| 
					!GetRemotePropertyInt("EMR_SizeImageToDetail", 1, 0, GetCurrentUserName(), true) 
				);*/
				// (a.walling 2011-08-01 17:04) - PLID 44743 - This was just confusing in many situations. To simplify, we either resize, or we don't.
				bool bRestoreIdealSize = !GetRemotePropertyInt("EMR_SizeImageToDetail", 1, 0, GetCurrentUserName(), true);

				// (j.jones 2010-12-29 15:49) - PLID 41943 - the file name could have been renamed if a duplicate
				// filename existed, so make sure we use the strFinalFileName variable, which is the name of
				// the file that is now attached to the patient's history
				ASSERT(!strFinalFileName.IsEmpty());

				m_pDetail->SetImage(strFinalFileName, itPatientDocument);
				//m_pDetail->SetSizeImageToDetail(FALSE);

				// (r.gonet 05/27/2011) - PLID 41801 - Zoom to fit the detail 
				m_pInkPicture->ZoomToFit(TRUE);
				
				//should be impossible to be empty, or the import would have failed
				if(!strFinalFileName.IsEmpty())
				{
					m_pInkPicture->PictureFileName = _bstr_t(strFinalFileName);
				}
				else {
					m_pInkPicture->PictureFileName = _bstr_t("");
				}
				
				// (a.walling 2011-05-25 17:57) - PLID 43847 - Instead, manually call the RestoreIdealSize if necessary.
				if (bRestoreIdealSize) {
					RestoreIdealSize(true);
				}

				// (a.wetta 2007-02-26 16:31) - PLID 23816 - There is no need to mark the detail as needing its content
				// reloaded because it is already loaded.  This just causes problems with sizing of the image instead.
				//m_pDetail->SetNeedContentReload();
				//TODO: Find an appropriate way of causing this tab to refresh itself.
				m_pDetail->m_pParentTopic->GetParentEMN()->InvalidateAllDetailMergeButtons();
			}
		}
		DragFinish(hDropInfo);
	} NxCatchAll("Error in OnDropFiles");
}

LRESULT CEmrItemAdvImageDlg::OnShowTopic(WPARAM wParam, LPARAM lParam)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		//
		// (c.haag 2006-07-06 12:00) - PLID 21296 - Show or hide the image control based
		// on the visibility of our parent topic
		// (c.haag 2008-06-03 14:20) - PLID 27777 - Update m_bIsTopicVisible and the
		// ink picture read-only state
		m_bIsTopicVisible = (BOOL)wParam;
		UpdateInkPictureReadOnlyState();
		if (IsWindow(m_wndInkPic.GetSafeHwnd())) {
			if (m_bIsTopicVisible) {
				m_wndInkPic.ShowWindow(SW_SHOW);
			} else {
				m_wndInkPic.ShowWindow(SW_HIDE);
			}
		}

		// (a.walling 2011-11-11 11:11) - PLID 46616 - was not refreshing the problem status!
		return CEmrItemAdvDlg::OnShowTopic(wParam, lParam);
		
	} NxCatchAll("Error in OnShowTopic");
	return 0;
}

// (a.walling 2011-05-25 09:38) - PLID 43843 - Get the displayed image size from the control itself
CRect CEmrItemAdvImageDlg::GetDisplayedImageSize()
{
	try {
		CSize szDisplayedImageSize(0, 0);
		if(m_pInkPicture != NULL) {
			m_pInkPicture->GetDisplayedImageSize((long*)&szDisplayedImageSize.cx, (long*)&szDisplayedImageSize.cy);
		}
		else if(m_p3DControl != NULL) {
			m_p3DControl->GetDisplayedImageSize(&szDisplayedImageSize.cx, &szDisplayedImageSize.cy);
		}

		return CRect(CPoint(0, 0), szDisplayedImageSize);

	}NxCatchAll("Error in CEmrItemAdvImageDlg::GetDrawingPropertiesAreaDimensions");

	return CRect(0,0,0,0);
}

// (a.walling 2011-05-25 17:57) - PLID 43847 - Allow limiting the max size
void CEmrItemAdvImageDlg::GetIdealDimensions(OUT long &nWidth, OUT long &nHeight, bool bUseMaxSize)
{
	// (a.walling 2011-05-19 18:07) - PLID 43843 - This is absolute insanity, and eventually
	// I ended up having to rewrite most of it. Oddly enough I predicted I would be doing so exactly
	// one year before this point.

	// Make sure the merge status icon button reflects the state of the data,
	// because that will have a direct influence on our size calculations.
	UpdateStatusButtonAppearances();

	// The caller is giving us a full window area so we have to adjust off the 
	// border to get what will be our client area.
	const long cnMergeBtnMargin = 5; // Amount of space above and below the merge button
	const CPoint ptOffset(6, 2);

	// the border of the window
	CSize szBorder;
	// The label at the top left
	CSize szLabel(LONG_MAX, LONG_MAX);
	// The merge icon button below the picture
	CSize szMergeBtn(LONG_MAX, LONG_MAX);
	// the minimum content size
	CSize szMinimumContent;
	// The size of the content (the ink image control)
	CSize szContent;

	// Get the border size
	CalcWindowBorderSize(this, &szBorder);

	CClientDC dc(this);

	// get the size of the bottom buttons
	if (IsMergeButtonVisible() || IsProblemButtonVisible()) {
		CalcControlIdealDimensions(&dc, m_pBtnMergeStatus, szMergeBtn);
	} else {
		szMergeBtn = CSize(0,0);
	}

	// now get the minimum possible size
	szMinimumContent.SetSize(0, 0);
	if(m_pInkPicture != NULL) {
		m_pInkPicture->GetMinimumDimensions((long*)&szMinimumContent.cx, (long*)&szMinimumContent.cy);
	}
	else if(m_p3DControl != NULL) {
		m_p3DControl->GetMinimumDimensions(&szMinimumContent.cx, &szMinimumContent.cy);
	}
	
	// (a.walling 2011-05-25 17:57) - PLID 43847 - Allow limiting the max size
	if (bUseMaxSize) {
		szContent = g_cszMaxDefaultImageSize;
		if(m_pInkPicture != NULL) {
			m_pInkPicture->GetIdealDimensionsForControl((long*)&szContent.cx, (long*)&szContent.cy);
		}
		else if(m_p3DControl != NULL) {
			m_p3DControl->GetIdealDimensionsForControl(&szContent.cx, &szContent.cy);
		}
	} else {
		szContent.SetSize(0, 0);
		if(m_pInkPicture != NULL) {
			m_pInkPicture->GetIdealDimensions((long*)&szContent.cx, (long*)&szContent.cy, VARIANT_TRUE);
		}
		else if(m_p3DControl != NULL) {
			m_p3DControl->GetIdealDimensions(&szContent.cx, &szContent.cy, VARIANT_TRUE);
		}
	}	

	szContent.SetSize(max(szContent.cx, szMinimumContent.cx), max(szContent.cy, szMinimumContent.cy));

	// of course we may need to adjust the label size again
	if (szContent.cx < szLabel.cx && IsControlValid(&m_wndLabel)) {		
		szLabel.cx = szContent.cx;
		szLabel.cy = 0;
		CalcControlIdealDimensions(&dc, &m_wndLabel, szLabel, TRUE);
	}

	// now we have the various sizes we require.
	CPoint currentOffset = ptOffset;
	CRect rcLabel(currentOffset, szLabel);
	currentOffset.y += rcLabel.Height();

	CRect rcContent(currentOffset, szContent);
	currentOffset.y += rcContent.Height();

	currentOffset.y += cnMergeBtnMargin;
	CRect rcMergeButton(currentOffset, szMergeBtn);
	CRect rcProblemButton(currentOffset + CPoint(3, 0), szMergeBtn);		
	// currentOffset.y += cnMergeBtnMargin; // I think this is unnecessary

	CRect rcClient(0, 0, szContent.cx, currentOffset.y);

	// add the borders and padding in to get the final area
	CSize szIdeal(
		rcClient.Width() + (szBorder.cx * 2) + ptOffset.x,
		rcClient.Height() + (szBorder.cy * 2) + ptOffset.y + szLabel.cy + szMergeBtn.cy
	);

	//Return the ideal size
	nWidth = szIdeal.cx;
	nHeight = szIdeal.cy;
}

BOOL CEmrItemAdvImageDlg::HasValidImage()
{
	if(m_pInkPicture != NULL)
	{
		if(m_pInkPicture->Image != NULL) return TRUE;
		else {
			CString strFileName = (LPCTSTR)m_pInkPicture->PictureFileName;
			if(!strFileName.IsEmpty() && DoesExist(strFileName)) {
				return TRUE;
			}
			else {
				return FALSE;
			}
		}
	}
	else if(m_p3DControl != NULL)
	{
		// (z.manning 2011-09-14 12:54) - PLID 44649 - Handle 3D control too
		CString strFileName = (LPCTSTR)m_p3DControl->ModelFileName;
		if(!strFileName.IsEmpty() && DoesExist(strFileName)) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}

	return FALSE;
}

// (z.manning 2011-09-12 11:48) - PLID 45335
void CEmrItemAdvImageDlg::OnSmartStampAdd3DControl()
{
	try
	{
		Handle3DStampChange();
		m_pDetail->SetImageTextAdded();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-09-12 11:48) - PLID 45335
void CEmrItemAdvImageDlg::OnSmartStampErase3DControl()
{
	try
	{
		Handle3DStampChange();
		m_pDetail->SetImageTextRemoved();
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrItemAdvImageDlg::Handle3DStampChange()
{
	if(m_p3DControl == NULL) {
		return;
	}

	m_pDetail->SetImageTextData(m_p3DControl->TextData, true);
}

void CEmrItemAdvImageDlg::OnTextChangedInkPicture(ETextChangeType TextChangeType)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		if(!m_pInkPicture) {
			// (r.gonet 05/06/2011) - PLID 43542 - The ink picture is not fully created. Ignore it until it is.
			return;
		}

		// (r.gonet 06/10/2011) - PLID 30359 - Don't attempt to save text scaling when we are changing state,
		//  otherwise we will end up wiping out the state due to bad feedback.
		if(TextChangeType == tctScalingChanged && m_pDetail->IsStateChanging()) {
			return;
		}

		//TES 1/19/2007 - PLID 18159 - Just like OnStrokeInkPicture(), we need to remember the current settings.
		UpdateRememberedPenState();

		//Tell our detail.
		// (r.gonet 05/27/2011) - PLID 30359 - Don't request a full state change if we are just updating the text scaling factors.
		m_pDetail->SetImageTextData(m_pInkPicture->TextData, TextChangeType != tctScalingChanged);
		
		//for auditing, track what sort of change happened.
		switch(TextChangeType) {
		case tctStringAdded:
			m_pDetail->SetImageTextAdded();
			break;
		case tctStringRemoved:
			m_pDetail->SetImageTextRemoved();
			break;
		case tctScalingChanged:
			// (r.gonet 05/06/2011) - PLID 43542 - We don't want anything else done in this case but the saving.
			break;
		case tctStampModified:
			// (r.gonet 05/02/2012) - PLID 49946 - Currently this is only the position, size, and angle of an image stamp changing.
			m_pDetail->SetImageStampModified();
			break;
		}
	} NxCatchAll("Error in OnTextChangedInkPicture");
}

void CEmrItemAdvImageDlg::OnCustomStampsChangedInkPicture(LPCTSTR strNewCustomStamps)
{
	try {

		// (j.jones 2010-02-10 14:59) - PLID 37224 - we manage stamp changes in this version of Practice,
		// not the NxInkPicture, so this event should never be received

		/*
		// (a.wetta 2007-04-06 10:02) - PLID 24324 - Save the current custom stamp configuration
		SetPropertyMemo("EMR_Image_Custom_Stamps", strNewCustomStamps, 0);

		// (a.wetta 2007-04-11 10:55) - PLID 25532 - Let the other images know that the stamps have changed
		// (a.walling 2008-12-19 09:29) - PLID 29800 - Refresh custom stamps only
		(m_pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->GetInterface())->RefreshContentByType(eitImage, FALSE, TRUE, TRUE);
		*/

	}NxCatchAll("Error in CEmrItemAdvImageDlg::OnCustomStampsChangedInkPicture");
}

void CEmrItemAdvImageDlg::ReflectCurrentContent()
{
	try {
		// Create everything the parent wants
		CEmrItemAdvDlg::ReflectCurrentContent();

		// (a.wetta 2007-04-09 13:20) - PLID 25532 - Be sure to update the custom stamps on the image
		if(GetRemotePropertyInt("EMR_Image_Use_Custom_Stamps", 1, 0, GetCurrentUserName(), TRUE))
		{	
			// (z.manning 2011-10-25 13:04) - PLID 39401 - Pass in the detail
			CString strStamps = GenerateInkPictureStampInfo(m_pDetail);
			if (m_pInkPicture != NULL) {
				// (j.jones 2010-02-10 11:11) - PLID 37224 - stamps are now tracked globally, just call the
				// EmrUtils function to generate the stamp info that is readable by the ink picture		
				m_pInkPicture->CustomStamps = _bstr_t(strStamps);
			}
			else if (m_p3DControl != NULL) {
				// (z.manning 2011-09-07 16:52) - PLID 44693 - Custom stamps on the 3D control
				m_p3DControl->CustomStamps = _bstr_t(strStamps);
			}
		}

		if(m_pInkPicture != NULL) {
			RefreshHotSpots(m_pInkPicture, m_pDetail);
		}

	}NxCatchAll("Error in CEmrItemAdvListDlg::ReflectCurrentContent");
}

// (a.wetta 2007-04-10 10:15) - PLID 25532 - Added functions to access the custom stamps for the image
void CEmrItemAdvImageDlg::SetCustomStamps(CString strCustomStamps)
{
	try
	{
		if(m_pInkPicture != NULL)
		{
			//TES 5/15/2008 - PLID 27776 - Check whether the stamps have actually changed.
			if(strCustomStamps != (LPCTSTR)m_pInkPicture->CustomStamps) {
				m_pInkPicture->CustomStamps = _bstr_t(strCustomStamps);
				OnCustomStampsChangedInkPicture(strCustomStamps);
			}
		}
		else if(m_p3DControl != NULL)
		{
			// (z.manning 2011-09-07 16:53) - PLID 44693 - Custom stamps on the 3D control
			if(strCustomStamps != (LPCTSTR)m_p3DControl->CustomStamps) {
				m_p3DControl->CustomStamps = _bstr_t(strCustomStamps);
				OnCustomStampsChangedInkPicture(strCustomStamps);
			}
		}
	}NxCatchAll("Error in CEmrItemAdvListDlg::SetCustomStamps");
}

// (r.gonet 02/13/2012) - PLID 37682 - Sets the current filter on the image stamps. NULL will function like ResetImageTextStringFilter()
void CEmrItemAdvImageDlg::SetImageTextStringFilter(CTextStringFilterPtr pTextStringFilter)
{
	_variant_t varData;
	if(pTextStringFilter == NULL) {
		// (r.gonet 02/14/2012) - PLID 37682 - There is no filter, pass an empty to clear it on the image
		varData = GetVariantEmpty();
	} else {
		// (r.gonet 02/14/2012) - PLID 37682 - Get the filter as a variant
		varData = pTextStringFilter->GetAsVariant();
	}
	if(m_pInkPicture != NULL) {
		m_pInkPicture->PutTextStringFilterData(varData);
	}
	else if(m_p3DControl != NULL) {
		m_p3DControl->PutTextStringFilterData(varData);
	}
}

// (j.jones 2010-02-24 15:27) - PLID 37312 - obsolete, we would never need
// to get the stamps from an ink control since they are configured in Practice now
/*
CString CEmrItemAdvImageDlg::GetCustomStamps()
{
	try {
		return (LPCSTR)m_pInkPicture->CustomStamps; 
	}NxCatchAll("Error in CEmrItemAdvListDlg::SetCustomStamps");
	return "";
}
*/

int CEmrItemAdvImageDlg::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// (z.manning, 08/20/2007) - PLID 23867 - There was a problem that when clicking on an image control
		// when the previous focus was in some sort of field where you could type would cause a problem where
		// keyboard focus would get stuck in the previous text area (because this image wnd and control don't
		// have a reason to have keyboard focus) but you'd be unable to type in that text area even after having
		// clicked back into it. So, since the image control can't have keyboard focus, let's still make sure
		// we take it away from whatever may have it.
		// (a.walling 2012-05-16 15:25) - PLID 50430 - Don't set focus here; DefWindowProc will do that automatically for the appropriate child window.
		//::SetFocus(NULL);
		return CEmrItemAdvDlg::OnMouseActivate(pDesktopWnd, nHitTest, message);
	} NxCatchAll("Error in OnMouseActivate");

	return 0;
}

// (z.manning, 01/22/2008) - PLID 28690 - Added message handler for clicking on hot spots.
void CEmrItemAdvImageDlg::OnClickedHotSpotInkPicture(long nHotSpotID)
{
	try
	{
		// (c.haag 2008-01-24 10:50) - PLID 28690 - Don't handle immediately; or else
		// Practice becomes unstable after a while. The message queue has content which
		// is, for some reason, sensitive to what's done here and now.
		PostMessage(NXM_EMR_TOGGLE_HOT_SPOT_SELECTION, nHotSpotID);
		//m_pDetail->ToggleImageHotSpotSelection(nHotSpotID);

	}NxCatchAll("CEmrItemAdvImageDlg::OnClickedHotSpotInkPicture");
}

// (z.manning 2011-07-25 12:29) - PLID 44649
void CEmrItemAdvImageDlg::OnClickedHotSpot3DControl(short n3DHotSpotID)
{
	try
	{
		CEMRHotSpot *pHotSpot = m_pDetail->GetHotSpotArray()->FindBy3DHotSpotID(n3DHotSpotID);
		if(pHotSpot != NULL) {
			PostMessage(NXM_EMR_TOGGLE_HOT_SPOT_SELECTION, pHotSpot->GetID());
		}

	}NxCatchAll(__FUNCTION__);
}

LRESULT CEmrItemAdvImageDlg::OnToggleHotSpot(WPARAM wParam, LPARAM lParam)
{
	try
	{
		// (z.manning 2010-05-05 16:38) - PLID 38503 - We need to remember these before we change the state
		UpdateRememberedPenState();

		// (c.haag 2008-01-24 10:50) - PLID 28690 - Handle image hot spot toggles
		const long nHotSpotID = (const long)wParam;
		m_pDetail->ToggleImageHotSpotSelection(nHotSpotID);

	}NxCatchAll("CEmrItemAdvImageDlg::OnToggleHotSpot");
	return 0;
}

// (j.jones 2010-02-10 14:44) - PLID 37312 - handle when the image tries to open the stamp setup
void CEmrItemAdvImageDlg::OnOpenStampSetupInkPicture()
{
	try
	{
		HandleStampSetup();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-09-14 10:18) - PLID 44693
void CEmrItemAdvImageDlg::OnOpenStampSetup3DControl()
{
	try
	{
		HandleStampSetup();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-10-25 14:48) - PLID 39401 - Moved common logic to this function
void CEmrItemAdvImageDlg::HandleStampSetup()
{
	// (a.walling 2012-03-12 10:06) - PLID 46648 - Dialogs must set a parent!
	if(GetMainFrame()->EditEMRImageStamps(this))
	{
		(m_pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->GetInterface())->RefreshContentByType(eitImage, FALSE, TRUE, TRUE);
	}
}

// (r.gonet 02/14/2012) - PLID 37682 - User has pressed the stamp filter setup button, open the editor
void CEmrItemAdvImageDlg::OnOpenStampFilterSetupInkPicture()
{
	try
	{
		HandleStampFilterSetup();
	}
	NxCatchAll(__FUNCTION__);
}

// (r.gonet 02/14/2012) - PLID 37682 - User has pressed the setup button, open the editor
void CEmrItemAdvImageDlg::OnOpenStampFilterSetup3DControl()
{
	try
	{
		HandleStampFilterSetup();
	}
	NxCatchAll(__FUNCTION__);
}

// (r.gonet 02/14/2012) - PLID 37682 - Open the stamp filter editor or turn off filtering, toggling between the two.
void CEmrItemAdvImageDlg::HandleStampFilterSetup()
{
	if(m_pDetail->GetImageTextStringFilter() != NULL) {
		// Turn the filter off
		m_pDetail->SetImageTextStringFilter(CTextStringFilterPtr());
		m_pDetail->ReflectImageTextStringFilter();
	} else {
		m_pDetail->EditImageStampFilter(this);
	}
}

/* (r.gonet 05/31/2011) - PLID 43896 - Put back in when we want to save and restore the zoom and pan offsets
void CEmrItemAdvImageDlg::OnZoomLevelChangedInkPicture(double dZoomLevel)
{
	try {
		m_pDetail->SetZoomLevel(dZoomLevel);
	}NxCatchAll(__FUNCTION__);
}

void CEmrItemAdvImageDlg::OnViewportOffsetsChangedInkPicture(long nOffsetX, long nOffsetY)
{
	try {
		m_pDetail->SetOffsetX(nOffsetX);
		m_pDetail->SetOffsetY(nOffsetY);
	}NxCatchAll(__FUNCTION__);
}
*/

// (j.jones 2010-02-18 10:14) - PLID 37423 - this function will tell the NxInkPicture
// whether the image is or is not a SmartStampImage
void CEmrItemAdvImageDlg::SetIsSmartStampImage(BOOL bIsSmartStampImage)
{
	try {

		if(m_pInkPicture) {
			//tell the ocx whether it is a SmartStamp image
			if(bIsSmartStampImage) {
				m_pInkPicture->PutIsSmartStampImage(VARIANT_TRUE);
			}
			else {
				m_pInkPicture->PutIsSmartStampImage(VARIANT_FALSE);
			}
		}
		else if(m_p3DControl != NULL) {
		}
		else {
			//this should not be called if the InkPicture doesn't exist
			ASSERT(FALSE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-05-05 16:45) - PLID 38503 - Moved some repeated code to its own function
void CEmrItemAdvImageDlg::UpdateRememberedPenState()
{
	if(m_pInkPicture != NULL) {
		m_bIsEraserState = m_pInkPicture->GetIsEraserState();
	}
}

// (a.walling 2010-10-25 10:04) - PLID 41043 - Handle OnShowWindow to clear out the cached image when no longer necessary to eat up memory
void CEmrItemAdvImageDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{	
	// (a.walling 2011-01-26 16:08) - PLID 41043 - Actually, templates hide/show items, so this was causing trouble.
	// A proper solution is still a significant development investment away, but this will still continue preventing
	// the same image from being loaded into memory twice.

	//try {
	//	if(!bShow) {
	//		// (a.walling 2010-10-25 10:04) - PLID 41043 - Release our cached image reference (no trouble even if it is NULL)
	//		m_pCachedImage.reset();
	//	}
	//} NxCatchAll(__FUNCTION__);
	CEmrItemAdvDlg::OnShowWindow(bShow, nStatus);
}

// (a.walling 2011-05-25 17:57) - PLID 43847 - Restore the ideal size of the image
void CEmrItemAdvImageDlg::RestoreIdealSize(bool bUseMaxSize)
{	
	long nWidth = 0, nHeight = 0;
	GetIdealDimensions(nWidth, nHeight, bUseMaxSize);
	SetWindowPos(NULL, 0, 0, nWidth, nHeight, SWP_NOMOVE|SWP_NOZORDER);
	
	//TES 10/25/2006 - We need to make sure that all other code related to resizing (e.g., adjusting scrollbar) 
	// is called.
	CRect rcNewClient;
	GetClientRect(&rcNewClient);
	//Needs to be in parent's coordinates.
	CWnd *pParent = GetParent();
	if(pParent) {
		ClientToScreen(&rcNewClient);
		pParent->ScreenToClient(&rcNewClient);
	}

	// (a.walling 2012-04-02 08:29) - PLID 49304 - Removed a lot of dead code regarding moving items

	CalcWindowRect(&rcNewClient);
	MapWindowPoints(GetParent(), &rcNewClient);
	SetWindowPos(NULL, 0, 0, rcNewClient.Width(), rcNewClient.Height(), SWP_NOMOVE);
}

// (z.manning 2011-09-26 09:27) - PLID 45664
void CEmrItemAdvImageDlg::Get3DModelOutputData(OUT CImageArray *paryImages)
{
	if(m_p3DControl != NULL)
	{
		_variant_t varImages = m_p3DControl->GetPreviewData();
		paryImages->LoadFromVariant(varImages);
	}
}

// (z.manning 2011-10-05 13:34) - PLID 45842
void CEmrItemAdvImageDlg::OnPreviewModified3DControl()
{
	try
	{
		if(m_p3DControl != NULL)
		{
			_variant_t varPrintData = m_p3DControl->GetViewData();
			m_pDetail->SetImagePrintData(varPrintData);
		}
	}
	NxCatchAll(__FUNCTION__);
}

//TES 2/24/2012 - PLID 45127 - Returns the index of the stamp at the given point (in screen coordinates).  
// If there is no stamp there, returns -1.
long CEmrItemAdvImageDlg::GetStampIndexFromPoint(long x, long y)
{
	long nIndex = -1;
	CPoint ptInk(x,y);
	m_wndInkPic.ScreenToClient(&ptInk);
	//TES 3/9/2012 - PLID 45127 - Added the same option for the 3D version
	if(m_pDetail->Is3DImage()) {
		if(m_p3DControl != NULL) {
			m_p3DControl->GetStampFromPoint(ptInk.x, ptInk.y, &nIndex);
		}
	}
	else {
		//TES 2/24/2012 - PLID 45127 - The ink picture control's version of this expects client coordinates relative to the control.
		if(m_pInkPicture != NULL) {
			m_pInkPicture->GetStampFromPoint(ptInk.x, ptInk.y, &nIndex);
		}
	}
	return nIndex;
}

void CEmrItemAdvImageDlg::OnOpenStampSearch()
{
	try
	{
		// (a.walling 2012-08-28 08:37) - PLID 52321 - Launch the stamp dialog and process the clicked stamp
		CStampSearchDlg dlg(this);
		if (IDOK == dlg.DoModal() && -1 != dlg.GetClickedStampID()) {				
			long nStampID = dlg.GetClickedStampID();

			// (a.walling 2012-08-28 10:23) - PLID 51742 - Get stamp from the cache
			EMRImageStamp* pStamp = GetMainFrame()->GetEMRImageStampByID(nStampID);
			ENSURE(pStamp);

			// (a.walling 2012-09-04 12:07) - PLID 52331 - Pass image info if available
			_variant_t var;
			if (pStamp->m_ImageInfo.arImageBytes && pStamp->m_ImageInfo.nNumImageBytes) {
				var.vt = VT_UI1 | VT_ARRAY;
				var.parray = Nx::SafeArray<BYTE>(
					pStamp->m_ImageInfo.nNumImageBytes
					, pStamp->m_ImageInfo.arImageBytes
				).Detach();
			}
			m_pInkPicture->SetCurrentStampInfoWithImage((LPCTSTR)pStamp->strStampText, pStamp->nTextColor, pStamp->nID, (LPCTSTR)pStamp->strTypeName, pStamp->bShowDot, var);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-08-14 12:23) - PLID 52134
void CEmrItemAdvImageDlg::OnOpenStampSearch3D()
{
	try
	{
		if (m_p3DControl != NULL) {
			// (a.walling 2012-08-28 08:37) - PLID 52321 - Launch the stamp dialog and process the clicked stamp
			CStampSearchDlg dlg(this);
			if (IDOK == dlg.DoModal() && -1 != dlg.GetClickedStampID()) {				
				long nStampID = dlg.GetClickedStampID();

				// (a.walling 2012-08-28 10:23) - PLID 51742 - Get stamp from the cache
				EMRImageStamp* pStamp = GetMainFrame()->GetEMRImageStampByID(nStampID);
				ENSURE(pStamp);
				m_p3DControl->SetCurrentStampInfo((LPCTSTR)pStamp->strStampText, pStamp->nTextColor, pStamp->nID, (LPCTSTR)pStamp->strTypeName);				
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (d.singleton 2013-4-22 14:32) - PLID 56421 get the signature form the topaz sig pad and display on ink control
void CEmrItemAdvImageDlg::OnClickedTopazSignature()
{
	try {
		// (a.walling 2014-07-28 10:36) - PLID 62825 - Use TopazSigPad::GetSignatureInk
		MSINKAUTLib::IInkDispPtr pInk = TopazSigPad::GetSignatureInk(this);
		if(!pInk) {
			//no ink or failure,  return
			return;
		}
		// (b.spivey, March 27, 2013) - PLID 30035 - Finally, we have created an ink object to add to the control. 
		m_pInkPicture->PutInkData(g_cvarEmpty); 
		m_pInkPicture->AddStrokesFromInkWithOffset(_variant_t((LPDISPATCH)pInk), (float)(-0.20), (float)(0.0));
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-22 09:03) - PLID 62836 - Handle PenColorChanged
void CEmrItemAdvImageDlg::OnCurrentPenColorChanged()
{
	try
	{
		m_pDetail->OnCurrentPenColorChanged(m_pInkPicture->CurrentPenColor);
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-07-22 09:03) - PLID 62836 - Handle PenSizeChanged
void CEmrItemAdvImageDlg::OnCurrentPenSizeChanged()
{
	try
	{
		m_pDetail->OnCurrentPenSizeChanged(m_pInkPicture->CurrentPenSizePercent);
	}NxCatchAll(__FUNCTION__);
}
