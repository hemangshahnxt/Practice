// NxToolbar.cpp : implementation file
//

#include "stdafx.h"
#include "NxToolbar.h"
#include "NxGdiPlusUtils.h"
#include "childfrm.h"
#include "globalutils.h"
#include "WindowUtils.h" // (j.luckoski 2013-05-07) - PLID 52484

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

using namespace Gdiplus;

// (a.walling 2008-04-16 09:41) - PLID 29673 - Helper class to handle our own drawing of the toolbar

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNxToolBar

CNxToolBar::CNxToolBar()
 : m_uxtTheme(*(new UXTheme))
{
	// (a.walling 2012-01-04 17:13) - PLID 47324 - Disable active accessibility for this window due to module state issues (see this item notes for more info)
	m_bEnableActiveAccessibility = false;

	m_bThemeInit = FALSE;
	m_nLButtonDownIndex = -1;

	//m_bEnableActiveAccessibility = false;
}

CNxToolBar::~CNxToolBar()
{	
	m_uxtTheme.CloseThemeData();
	delete &m_uxtTheme;
}

// (a.walling 2008-06-17 16:32) - PLID 30420 - Override WM_ERASEBKGND
// (a.walling 2008-10-02 17:23) - PLID 31575 - Border drawing (NcPaint)
// (a.walling 2008-10-08 16:34) - PLID 31575 - Eat RButton messages
BEGIN_MESSAGE_MAP(CNxToolBar, CToolBar)
	//{{AFX_MSG_MAP(CToolBar)
		ON_NOTIFY_REFLECT_EX(NM_CUSTOMDRAW, OnCustomDraw)
		ON_WM_LBUTTONDOWN()
		ON_WM_LBUTTONUP()
		ON_WM_RBUTTONDOWN()
		ON_WM_RBUTTONUP()
		ON_WM_ERASEBKGND()
		ON_WM_SETCURSOR()
		ON_WM_NCPAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNxToolBar message handlers

const BOOL g_cbUseClassicToolbars = FALSE;

//#define NXTOOLBAR_DEBUG

BOOL CNxToolBar::OnCustomDraw(NMHDR* pNotifyStruct, LRESULT* result)
{
	try {
		if (g_cbUseClassicToolbars) {
			*result = CDRF_DODEFAULT;

			return FALSE;
		}

		LPNMTBCUSTOMDRAW pDraw = (LPNMTBCUSTOMDRAW)pNotifyStruct;
		BOOL bHandled = FALSE;

		switch(pDraw->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			*result = CDRF_NOTIFYITEMDRAW;
			bHandled = TRUE;
			break;
		case CDDS_ITEMPREPAINT: {

#ifdef NXTOOLBAR_DEBUG
			TRACE("\nRECT rc(%li, %li, %li, %li);	DWORD_PTR dwItemSpec (0x%08x); UINT  uItemState (%lu);	LPARAM lItemlParam (0x%08x);\n",
				pDraw->nmcd.rc.top, pDraw->nmcd.rc.left, pDraw->nmcd.rc.bottom, pDraw->nmcd.rc.right,
				pDraw->nmcd.dwItemSpec, pDraw->nmcd.uItemState, pDraw->nmcd.lItemlParam);
#endif

			Bitmap* pBitmap = NULL;

			// (a.walling 2008-04-23 15:50) - PLID 29768 - Modify rects to have a 0,0 origin for the buffer drawing.
			CRect rcScreen = pDraw->nmcd.rc;
			CRect rcArea(0, 0, rcScreen.Width(), rcScreen.Height());

			CRect rcIcon = rcArea;
			rcIcon.bottom -= g_cnToolbarTextHeight;
			// (a.walling 2008-04-23 15:50) - PLID 29768 - This rect is relative to the client area, not the buffer
			CRect rcScreenIcon = rcScreen;
			rcScreenIcon.bottom -= g_cnToolbarTextHeight;
		
			CRect rcAdjust = rcIcon;
			rcAdjust.DeflateRect(1, 1, 1, 1);

			CView *pView = NULL;
			CNxTabView *pTabView = NULL;
			CChildFrame *pFrame = NULL;
			BOOL bIsActive = FALSE;
	
			// (a.walling 2008-12-30 16:43) - PLID 32591 - Sometimes this can throw an invalid argument exception if
			// the frame is not what we expect. It is ridiculously hard to reproduce this, but it has happened somewhat
			// consistently on a client's machine.
			try {
				pFrame = GetMainFrame()->GetActiveViewFrame();
			
				// (a.walling 2008-10-23 17:39) - PLID 31821 - This can crash; ensure the view is actually an CNxTabView
				if (pFrame) {
					pView = pFrame->GetActiveView();
					// (j.jones 2012-08-08 10:11) - PLID 51063 - added check that the active view is non-null
					if (pView && pView->IsKindOf(RUNTIME_CLASS(CNxTabView))) {
						pTabView = (CNxTabView*) pView;
					}
				}
			} catch (CException* e) {
				// ignore
				e->Delete();
			}

			// which toolbar icon is this?
			CString strLabel; // (a.walling 2008-04-21 13:28) - PLID 29731 - Label text
			WORD nID = 0;
			BOOL bActionButton = FALSE;

			switch(pDraw->nmcd.dwItemSpec) {
				// main toolbar
				case ID_PATIENTS_MODULE:
				{
					if (pFrame && pView && pFrame->IsOfType(PATIENT_MODULE_NAME)) {
						// (z.manning 2015-03-13 14:28) - NX-100432 - Handle the preference to open the dashboard tab instead
						if (pTabView == NULL || pTabView->GetActiveTab() == GetMainFrame()->GetPrimaryEmrTab())
						{
							bIsActive = FALSE;
						}
						else {
							bIsActive = TRUE;
						}
					}

					nID = IDR_PNG_PATIENTS;
					strLabel = "Patients";
					break;
				}
				case ID_SCHEDULER_MODULE: {
					if (pFrame && pView && pFrame->IsOfType(SCHEDULER_MODULE_NAME)) {
						bIsActive = TRUE;
					}
					
					nID = IDR_PNG_SCHED;
					strLabel = "Scheduler";
					break;
				}
				case ID_LETTER_WRITING_MODULE: {
					if (pFrame && pView && pFrame->IsOfType(LETTER_MODULE_NAME)) {
						bIsActive = TRUE;
					}
					
					nID = IDR_PNG_LETTERWRITING;
					strLabel = "Letters";
					break;
				}
				case ID_CONTACTS_MODULE: {
					if (pFrame && pView && pFrame->IsOfType(CONTACT_MODULE_NAME)) {
						bIsActive = TRUE;
					}
					
					nID = IDR_PNG_CONTACTS;
					strLabel = "Contacts";
					break;
				}
				case ID_MARKETING_MODULE: {
					if (pFrame && pView && pFrame->IsOfType(MARKET_MODULE_NAME)) {
						bIsActive = TRUE;
					}
					
					nID = IDR_PNG_MARKETING;
					strLabel = "Marketing";
					break;
				}
				case ID_INVENTORY_MODULE: {
					if (pFrame && pView && pFrame->IsOfType(INVENTORY_MODULE_NAME)) {
						bIsActive = TRUE;
					}
					
					nID = IDR_PNG_INVENTORY;
					strLabel = "Inventory";
					break;
				}
				case ID_FINANCIAL_MODULE: {
					if (pFrame && pView && pFrame->IsOfType(FINANCIAL_MODULE_NAME)) {
						bIsActive = TRUE;
					}
					
					nID = IDR_PNG_FINANCIAL;
					strLabel = "Financial";
					break;
				}
				case ID_SURGERYCENTER_MODULE: {
					if (pFrame && pView && pFrame->IsOfType(SURGERY_CENTER_MODULE_NAME)) {
						bIsActive = TRUE;
					}
					
					nID = IDR_PNG_ASC;
					strLabel = "Surgery";
					break;
				}
				case ID_REPORTS_MODULE: {
					if (pFrame && pView && pFrame->IsOfType(REPORT_MODULE_NAME)) {
						bIsActive = TRUE;
					}
					
					nID = IDR_PNG_REPORTS;
					strLabel = "Reports";
					break;
				}
				case ID_ADMINISTRATOR_MODULE: {
					if (pFrame && pView && pFrame->IsOfType(ADMIN_MODULE_NAME)) {
						bIsActive = TRUE;
					}
					
					nID = IDR_PNG_ADMIN;
					strLabel = "Admin.";
					break;
				}
				// (d.thompson 2009-11-16) - PLID 36134
				case ID_LINKS_MODULE: {
					if (pFrame && pView && pFrame->IsOfType(LINKS_MODULE_NAME)) {
						bIsActive = TRUE;
					}
					
					nID = IDR_PNG_LINKS;
					strLabel = "Links";
					break;
				}

				case ID_NEW_PATIENT_BTN: {
					nID = IDR_PNG_NEWPATIENT;
					strLabel = "New Pat.";
					bActionButton = TRUE;
					break;
				}
				case ID_FIRST_AVAILABLE_APPT: {
					nID = IDR_PNG_FFA;
					strLabel = "First Avail.";
					bActionButton = TRUE;
					break;
				}
				case ID_TB_NEW_EMR: {
					if (pFrame && pView && pFrame->IsOfType(PATIENT_MODULE_NAME))
					{
						// (z.manning 2015-03-13 14:28) - NX-100432 - Handle the preference to open the dashboard tab instead
						int nNexEMRIconPref = GetRemotePropertyInt("NexEMRIconTabToOpen", 1, 0, GetCurrentUserName());
						if (pTabView != NULL && pTabView->GetActiveTab() == GetMainFrame()->GetPrimaryEmrTab()) {
							bIsActive = TRUE;
						}
						else {
							bIsActive = FALSE;
						}
					}

					nID = IDR_PNG_NEXEMR;
					strLabel = "NexEMR";
					bActionButton = TRUE;
					break;
				}
				case ID_CARECREDIT_BTN: {
					nID = IDR_PNG_CARECREDIT;
					strLabel = "CareCredit";
					bActionButton = TRUE;
					break;
				}

				// GUI toolbar
				case IDM_UPDATE_VIEW: {
					nID = IDR_PNG_REFRESH;
					strLabel = "Refresh";
					bActionButton = TRUE;
					break;
				}
				// (z.manning 2009-03-03 16:11) - PLID 33104 - Renamed the help button to ID_MANUAL_TOOLBAR_BTN
				case ID_MANUAL_TOOLBAR_BTN: {
					nID = IDR_PNG_HELP;
					strLabel = "Help";
					bActionButton = TRUE;
					break;
				}
				case ID_FILE_PRINT_PREVIEW: {
					nID = IDR_PNG_PRINTPREVIEW;
					strLabel = "Preview";
					bActionButton = TRUE;
					break;
				}
				case ID_FILE_PRINT: {
					nID = IDR_PNG_PRINT;
					strLabel = "Print";
					bActionButton = TRUE;
					break;
				}
				default: {
					ASSERT(FALSE);
					break;
				}
			};

			if (nID != 0) {
				pBitmap = NxGdi::BitmapFromPNGResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(nID), rcAdjust.Width());
			} else {
				ASSERT(FALSE);
			}

			if (pBitmap) {

				WORD nFlatState = 0;

				if(pDraw->nmcd.uItemState & CDIS_SELECTED) {
					// item is selected, and most likely also hot
					nFlatState = CDIS_SELECTED;
				} else if (pDraw->nmcd.uItemState & CDIS_HOT) {
					nFlatState = CDIS_HOT;
					// item is hot (mouseover)
				} else if ( (pDraw->nmcd.uItemState & CDIS_GRAYED) || (pDraw->nmcd.uItemState & CDIS_DISABLED) ) {
					nFlatState = CDIS_GRAYED;
					// gray
				} else if ( (pDraw->nmcd.uItemState == 0) || (pDraw->nmcd.uItemState == CDIS_DEFAULT) || (pDraw->nmcd.uItemState == CDIS_INDETERMINATE) ) {
					nFlatState = CDIS_DEFAULT;
					// normal (default/indeterminate)
				}

				if	(
						(pDraw->nmcd.uItemState & CDIS_CHECKED) ||
						(pDraw->nmcd.uItemState & CDIS_FOCUS) ||
						(pDraw->nmcd.uItemState & CDIS_MARKED)
					) {
					ASSERT(FALSE); // We should not have these flags; if we do, figure out how to handle them.
				}

				if (!m_bThemeInit) {
					m_uxtTheme.OpenThemeData(GetSafeHwnd(), "Toolbar");
					m_bThemeInit = TRUE;
				}
				
				// (a.walling 2008-04-23 15:48) - PLID 29768 - Prepare an offscreen buffer with no alpha needed
				Bitmap bmpBuffer(rcArea.Width(), rcArea.Height(), PixelFormat32bppRGB);
				Graphics g(&bmpBuffer);
				
				HDC hDC = g.GetHDC();
				// (a.walling 2008-06-17 16:31) - PLID 30420 - Erase the background onto the buffer
				// this supports the inability for remote users to handle alpha blending and keeps
				// all drawing on the offscreen buffer
				{
					// (a.walling 2009-01-14 13:33) - PLID 32734 - Delegate background erasing to the parent
					CWnd* pWndParent = GetParent();
					CPoint pt(rcScreen.left, rcScreen.top);
					CPoint ptNew(pt);

					// We need to offset the origin so the background is erased correctly. This was never an issue
					// with solid color fills, but now the background may be different depending on it's Y value.
					MapWindowPoints(pWndParent, &ptNew, 1);
					int nOffsetY = pt.y - ptNew.y;

					CPoint ptOld;
					::SetBrushOrgEx(hDC, 0, nOffsetY, &ptOld);
					
					::SendMessage(pWndParent->m_hWnd, WM_ERASEBKGND, (WPARAM)hDC, 0);

					::SetBrushOrgEx(hDC, ptOld.x, ptOld.y, NULL);
				}

				// (a.walling 2008-04-23 15:49) - PLID 29768 - Draw the theme background initially directly to the screen
				if (nFlatState == CDIS_SELECTED) {
					// offset the rect and continue
					rcAdjust.OffsetRect(1, 1);
				}

				// (a.walling 2008-06-17 16:32) - PLID 30420 - Draw the focus state onto the offscreen buffer
				if (nFlatState == CDIS_HOT || nFlatState == CDIS_SELECTED) {
					// Draw the hot focus rect
					if (m_uxtTheme.IsOpen()) {
						m_uxtTheme.DrawThemeBackground(hDC, TP_BUTTON, nFlatState == CDIS_SELECTED ? TS_PRESSED : TS_HOT, rcIcon, NULL);
					} else {
						CDC dc;
						dc.Attach(hDC);

						dc.Draw3dRect(rcIcon, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT));

						dc.Detach();
					}
				}
				g.ReleaseHDC(hDC);

				// (a.walling 2009-08-13 09:56) - PLID 35214 - If we are in any indexed color mode, change smoothing
				BOOL bIndexedColor = (GetDeviceCaps(pDraw->nmcd.hdc, BITSPIXEL) <= 8) ? TRUE : FALSE;
				if (bIndexedColor) {
					g.SetSmoothingMode(SmoothingModeNone);
					g.SetInterpolationMode(InterpolationModeNearestNeighbor);
				} else {
					g.SetSmoothingMode(SmoothingModeHighQuality);
					g.SetInterpolationMode(InterpolationModeHighQuality);
				}

				// (a.walling 2008-06-17 16:34) - PLID 30420 - This is now handled above and all drawing goes into an offscreen buffer
				// (a.walling 2008-04-23 15:48) - PLID 29768 - Now, if they are remote, we need to blit the current background into the buffer
				/*if (NxGdi::IsRemote()) {
					HDC hDCBitmap = g.GetHDC();
					HDC hDCScreen = pDraw->nmcd.hdc;
					::BitBlt(hDCBitmap, 0, 0, rcScreen.Width(), rcScreen.Height(), hDCScreen, rcScreen.left, rcScreen.top, SRCCOPY);
					g.ReleaseHDC(hDCBitmap);
				}*/

				switch(nFlatState) {
					case CDIS_SELECTED:
					case CDIS_HOT:
						{
							if (bActionButton && nFlatState == CDIS_SELECTED) {
								// This is an action button, so draw a highlight as it is clicked.
								Rect rect = NxGdi::RectFromCRect(rcArea);							

								Color colorA(0x7F, 0xFF, 0xEF, 0x50);
								Color colorB(0xDF, 0xFF, 0xF0, 0xF0);

								DrawHighlight(g, rect, colorA, colorB);
							} else {
								// draw a special sphere for hot images
								GraphicsPath path;

								Rect rect = NxGdi::RectFromCRect(rcIcon);
								NxGdi::CreateRoundedRectanglePath(path, rect, 20);

								PathGradientBrush brush(&path);
					
								brush.SetWrapMode(WrapModeClamp);

								brush.SetFocusScales(1 - (35.0f / (REAL)rect.Width), 1 - (35.0f / (REAL)rect.Height));

								Color colors[2] = {
									Color(Color::Transparent),
									NxGdi::ColorFromArgb(0xA0, Color::Lavender),
								};

								REAL positions[2] = {
									0.0f,
									1.0f
								};

								brush.SetInterpolationColors(colors, positions, 2);

								g.FillPath(&brush, &path);
							}
							

							g.DrawImage(pBitmap, NxGdi::RectFromCRect(rcAdjust), 0, 0, pBitmap->GetWidth(), pBitmap->GetHeight(), UnitPixel, NULL);
						}
						break;
					case CDIS_DEFAULT: {

						/*
						// desaturate slightly, full saturation on mouseover.
						float fSaturation = 0.75f; // 0 - 1.0

						float fComplement = 1.0f - fSaturation;

						// adjust for luminance of primary colors
						float fComplementR = 0.3086f * fComplement;
						float fComplementG = 0.6094f * fComplement;
						float fComplementB = 0.0820f * fComplement;

						REAL matrix[5][5] = { {fComplementR + fSaturation,  fComplementR,  fComplementR,  0.0f, 0.0f},
							{fComplementG,  fComplementG + fSaturation,  fComplementG,  0.0f, 0.0f},
							{fComplementB,  fComplementB,  fComplementB + fSaturation,  0.0f, 0.0f},
							{0.0f,  0.0f,  0.0f,  1.0f,  0.0f},
							{0.0f,  0.0f,  0.0f,  0.0f,  1.0f}
						};
						
						ColorMatrix cm;
						memcpy(cm.m, matrix, sizeof(REAL)*5*5);

						ImageAttributes ia;
						ia.SetColorMatrix(&cm);
						*/

						// actually don't desaturate

						g.DrawImage(pBitmap, NxGdi::RectFromCRect(rcAdjust), 0, 0, pBitmap->GetWidth(), pBitmap->GetHeight(), UnitPixel, NULL);
						break;
					}
					case CDIS_GRAYED: {
						if (bIsActive) {
							Rect rect = NxGdi::RectFromCRect(rcArea);
							Color colorA(0x7F, 0xFF, 0xEF, 0x50);
							Color colorB(0xDF, 0xFF, 0xFF, 0xFF);
							DrawHighlight(g, rect, colorA, colorB);
						}

						//Gilles Khouzams colour corrected grayscale shear
						/*REAL matrix[5][5] =	{	{0.3f,0.3f,0.3f,0,0},
												{0.59f,0.59f,0.59f,0,0},
												{0.11f,0.11f,0.11f,0,0},
												{0,0,0,1,0},
												{0,0,0,0,1} };*/

						float fContrast = 0.5f;

						ColorMatrix cm = {
							fContrast,0.1f,0.1f,0,0,
							0.1f,fContrast,0.1f,0,0,
							0,0,fContrast,0,0,
							0,0,0,1,0,
							0.001f,0.001f,0.001f,0,1
						};

						ImageAttributes ia;
						ia.SetColorMatrix(&cm);
						
						g.DrawImage(pBitmap, NxGdi::RectFromCRect(rcAdjust), 0, 0, pBitmap->GetWidth(), pBitmap->GetHeight(), UnitPixel, bIsActive ? NULL : &ia);
						break;
					}
					default:
						ASSERT(FALSE);
						break;
				}

				delete pBitmap;
				

				// (a.walling 2008-04-21 13:28) - PLID 29731 - Draw the text after everything else
				{				
					RectF rectLabel((float)rcArea.left,
									(float)rcArea.bottom - g_cnToolbarTextHeight,
									(float)rcArea.Width(),
									(float)g_cnToolbarTextHeight);

					DrawButtonText(g, strLabel, rectLabel, bIsActive, nFlatState, bIndexedColor);
				}

				// (a.walling 2009-01-14 14:08) - PLID 32734 - Use GDI primitives
				
				BOOL bBlitSuccess = FALSE;
				
				HDC hdcBuffer = ::CreateCompatibleDC(pDraw->nmcd.hdc);
				if (hdcBuffer) {
					HBITMAP hbmpBuffer = NULL;
					if (Gdiplus::Ok == bmpBuffer.GetHBITMAP(Color::White, &hbmpBuffer)) {
						HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hdcBuffer, hbmpBuffer);

						// (a.walling 2009-08-12 16:06) - PLID 35136 - Realize the palette
						HPALETTE hPal = (HPALETTE)theApp.m_palette.GetSafeHandle();
						HPALETTE hOld = ::SelectPalette(pDraw->nmcd.hdc, hPal, FALSE);
						::RealizePalette(pDraw->nmcd.hdc);
						bBlitSuccess = ::BitBlt(pDraw->nmcd.hdc, rcScreen.left, rcScreen.top, rcScreen.Width(), rcScreen.Height(), hdcBuffer, 0, 0, SRCCOPY);
						::SelectPalette(pDraw->nmcd.hdc, hOld, FALSE);

						::SelectObject(hdcBuffer, hOldBitmap);
						::DeleteObject(hbmpBuffer);
					}
					::DeleteDC(hdcBuffer);
				}

				if (!bBlitSuccess) {
					// (a.walling 2008-04-23 15:49) - PLID 29768 - Finally, blit the buffer to the screen
					Graphics gScreen(pDraw->nmcd.hdc);
					gScreen.DrawImage(&bmpBuffer, rcScreen.left, rcScreen.top);
				}
				
				// We just want to do the default for now and leave the framework in place for the future.
				*result = CDRF_SKIPDEFAULT;
				bHandled = TRUE;
			} else {
				// We just want to do the default for now and leave the framework in place for the future.
				*result = CDRF_DODEFAULT;
				bHandled = FALSE;
			}


			break;

			// debug routine, draws a string instead of an icon. 
			/*
			Gdiplus::Graphics g(pDraw->nmcd.hdc);

			Gdiplus::SolidBrush b(Gdiplus::Color(255, 128, 128, 255));

			CString str;
			str.Format("%li : %li", pDraw->nmcd.dwItemSpec, pDraw->nmcd.uItemState);

			Gdiplus::Font font(_bstr_t("Times New Roman"), 12);

			Gdiplus::RectF rectf((float)pDraw->nmcd.rc.left, (float)pDraw->nmcd.rc.top, (float)pDraw->nmcd.rc.right - (float)pDraw->nmcd.rc.left, (float)pDraw->nmcd.rc.bottom - (float)pDraw->nmcd.rc.top);

			g.DrawString(_bstr_t(str), -1, &font, rectf, Gdiplus::StringFormat::GenericDefault(), &b);

			*result = CDRF_SKIPDEFAULT;
			bHandled = TRUE;
			*/
		}
		default:
			*result = CDRF_DODEFAULT;
			bHandled = FALSE;
			break;
		}

		return bHandled;
	} NxCatchAllCallIgnore({LogDetail("Error handling toolbar OnCustomDraw");};);

	*result = CDRF_DODEFAULT;
	return FALSE;
}

void CNxToolBar::DrawHighlight(Gdiplus::Graphics& g, const Gdiplus::Rect& rect, const Gdiplus::Color& colorA, const Gdiplus::Color& colorB)
{
	GraphicsPath path;

	NxGdi::CreateRoundedRectanglePath(path, rect, 10);

	PathGradientBrush brush(&path);

	brush.SetWrapMode(WrapModeClamp);

	brush.SetFocusScales(1 - (35.0f / (REAL)rect.Width), 1 - (35.0f / (REAL)rect.Height));

	Color colors[3] = {
		Color(Color::Transparent),
		//Color(0x7F, 0xFF, 0xEF, 0xA0)
		colorA,
		colorB
		//Color(0x7F, 0xFF, 0xEF, 0x50),
		//Color(0xDF, 0xFF, 0xFF, 0xFF)

		//Color(0x7F, 0xFF, 0xEF, 0xA0)
	};

	REAL positions[3] = {
		0.0f,
		0.2f,
		1.0f
	};

	brush.SetInterpolationColors(colors, positions, 3);

	g.FillPath(&brush, &path);
}

// (a.walling 2009-08-13 10:30) - PLID 35214 - Let us know if the screen uses indexed color so we can disable smoothing
void CNxToolBar::DrawButtonText(Gdiplus::Graphics& g, const CString& strLabel, RectF rectLabel, BOOL bIsActive, WORD nFlatState, BOOL bIndexedColor)
{
	// (a.walling 2008-10-02 09:24) - PLID 31567 - Gdiplus namespace must be specified to prevent ambiguities
	Gdiplus::Font* pFontLabel = NULL;

	// (a.walling 2008-04-29 13:07) - PLID 29731 - Arial Narrow is not always available. Therefore,
	// we use a workaround by drawing a different standard font onto a buffer and then interpolating
	// it into our limited rect. This works out pretty well.

	// (a.walling 2009-10-01 16:13) - PLID 35136 - Draw the text using Arial Narrow (not offscreen) if we are limited in colors.

	BOOL bUseOffscreen = TRUE; // always true for now

	REAL fSize = 8.0f;
	FontFamily* pFontFamily = NULL;

	if (!bUseOffscreen || bIndexedColor) {
		pFontFamily = new FontFamily(_bstr_t("Arial Narrow"), NULL);
	}
	if (pFontFamily == NULL || pFontFamily->GetLastStatus() != Ok) {
		if (pFontFamily) {
			delete pFontFamily;
		}

		pFontFamily = new FontFamily(_bstr_t("Verdana"), NULL);
		fSize = 8.0f;

		if (bIndexedColor) {
			fSize = 6.0f;
		}
		bUseOffscreen = TRUE;
		ASSERT(pFontFamily);
		ASSERT(pFontFamily->GetLastStatus() == Ok);
	}

	Color c;
	if (bIsActive) {
		c = Color(0xFF, 0x0F, 0x13, 0x30);
		pFontLabel = new Gdiplus::Font(pFontFamily, fSize, FontStyleBold | FontStyleUnderline);
		rectLabel.Y -= 1;
	} else {
		if (nFlatState == CDIS_GRAYED) {
			c = Color(0xFF, 0x90, 0x90, 0x80);
		} else {
			c = Color(0xFF, 0x14, 0x27, 0x73);
		}
		if (nFlatState == CDIS_HOT || nFlatState == CDIS_SELECTED) {
			pFontLabel = new Gdiplus::Font(pFontFamily, fSize, FontStyleUnderline);
		} else {
			pFontLabel = new Gdiplus::Font(pFontFamily, fSize);
		}
	}

	_ASSERTE(pFontLabel->IsAvailable());

	SolidBrush b(c);

	StringFormat sf(StringFormat::GenericDefault());
	sf.SetAlignment(StringAlignmentCenter);
	sf.SetLineAlignment(StringAlignmentNear);
	//sf.SetFormatFlags(StringFormatFlagsNoWrap);
	sf.SetTrimming(StringTrimmingCharacter);


	/*RectF rectfCalc;
	g.MeasureString(_bstr_t(strLabel), -1, pFontLabel, PointF(rectLabel.X, rectLabel.Y), &sf, &rectfCalc);*/

	if (bUseOffscreen && !bIndexedColor) {
		const long nOffscreenWidth = 66;
		Bitmap bmpLabel(nOffscreenWidth, DWORD(ceil(rectLabel.Height)));
		{
			Graphics gLabel(&bmpLabel);

			// (a.walling 2009-08-13 10:30) - PLID 35214 - If we are in indexed color mode, disable smoothing. This will not look too good,
			// but at least it will be (somewhat) readable, rather than fuzzy garbage due to dithering.
			if (bIndexedColor) {
				gLabel.SetTextRenderingHint(TextRenderingHintSingleBitPerPixelGridFit);
				gLabel.SetSmoothingMode(SmoothingModeNone);
				gLabel.SetInterpolationMode(InterpolationModeNearestNeighbor);
			} else {
				gLabel.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
				gLabel.SetSmoothingMode(SmoothingModeHighQuality);
				gLabel.SetInterpolationMode(InterpolationModeHighQuality);
			}

			RectF rectfOffscreen(0.0f, 0.0f, (REAL)nOffscreenWidth, rectLabel.Height);
			gLabel.DrawString(_bstr_t(strLabel), -1, pFontLabel, rectfOffscreen, &sf, &b);
		}

		// (a.walling 2009-08-13 10:30) - PLID 35214 - If we are in indexed color mode, disable smoothing.
		InterpolationMode previousInterpolationMode = g.GetInterpolationMode();
		if (bIndexedColor) {
			g.SetInterpolationMode(InterpolationModeNearestNeighbor);
		} else {
			g.SetInterpolationMode(InterpolationModeBilinear);
		}
		g.DrawImage(&bmpLabel, rectLabel, 0.0f, 0.0f, (REAL)bmpLabel.GetWidth(), (REAL)bmpLabel.GetHeight(), UnitPixel);
		g.SetInterpolationMode(previousInterpolationMode);
	} else {		
		// (a.walling 2009-08-13 10:30) - PLID 35214 - If we are in indexed color mode, disable smoothing. This will not look too good,
		// but at least it will be (somewhat) readable, rather than fuzzy garbage due to dithering.
		if (bIndexedColor) {
			g.SetTextRenderingHint(TextRenderingHintSingleBitPerPixelGridFit);
		} else {
			g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
		}
		g.DrawString(_bstr_t(strLabel), -1, pFontLabel, rectLabel, &sf, &b);
	}

	delete pFontLabel;
	delete pFontFamily;
}

// (a.walling 2008-06-17 16:32) - PLID 30420 - Override WM_ERASEBKGND
BOOL CNxToolBar::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

// (a.walling 2008-07-02 13:59) - PLID 29757
void CNxToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	try {
		CToolBarCtrl& tb = GetToolBarCtrl();

		long nIndex = tb.HitTest(&point);

		if (nIndex >= 0) {
			m_nLButtonDownIndex = nIndex;
		} else {
			m_nLButtonDownIndex = -1;
		}
	} NxCatchAllCallIgnore({LogDetail("Error handling toolbar OnLButtonDown");};);

	CToolBar::OnLButtonDown(nFlags, point);
}

// (a.walling 2008-07-02 13:59) - PLID 29757 - Handle mousedown/up messages to catch clicking on disabled buttons
void CNxToolBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	try {
		// (a.walling 2008-08-21 14:06) - PLID 31056 - This bit of a hack is for non-composited, non-themed drawing issues
		// with tooltips. Basically we just find out where the tooltip last was, and invalidate that portion of the window.
		// The toolbar (with CBRS_TOOLTIPS style) will use MFC's internal CToolTipCtrl, which is a per-thread object.

		// (a.walling 2008-10-08 15:09) - PLID 31575 - m_pToolTip (common control tooltip) is now in AFX_MODULE_THREAD_STATE
		AFX_MODULE_THREAD_STATE* pThreadState = AfxGetModuleThreadState();
		CToolTipCtrl* pToolTip = pThreadState->m_pToolTip;
		if (pToolTip->GetSafeHwnd()) { // this is also, implicitly, a check for null.
			CRect rc;
			rc.SetRectEmpty();

			pToolTip->GetWindowRect(rc);
			ScreenToClient(rc);

			CRect rcClient;
			GetClientRect(rcClient);

			CRect rcInvalid;

			if (rcInvalid.IntersectRect(rcClient, rc)) {
				//RedrawWindow(rcInvalid, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ERASENOW | RDW_ALLCHILDREN);
				//just invalidate so we can redraw during the next paint cycle
				InvalidateRect(rcInvalid);
			}
		}

		// (j.luckoski 2013-05-07 10:36) - PLID 52484 - Kill focus if focus is set which saves and then restores focus for flow
		// Don't assert this as if it returns false, we should just continue as we have always done in the past. It wouldn't hurt anything. 
		// But under most circumstances it should work for the purpose of releasing the focus for the sake of saving.
		WindowUtils::PulseFocus();

		CToolBarCtrl& tb = GetToolBarCtrl();

		long nIndex = tb.HitTest(&point);

		if (nIndex >= 0 && nIndex == m_nLButtonDownIndex) {
			TBBUTTON tbButton;
			if (tb.GetButton(nIndex, &tbButton)) {
				// (a.walling 2008-07-02 13:59) - PLID 29757 - Let the mainframe decide what to do
				GetMainFrame()->PostMessage(NXM_TB_BTN_MOUSE_MESSAGE, (WPARAM)tbButton.idCommand, (LPARAM)TRUE);
			}
		}
	} NxCatchAllCallIgnore({LogDetail("Error handling toolbar OnLButtonUp");};);

	m_nLButtonDownIndex = -1;

	CToolBar::OnLButtonUp(nFlags, point);
}

// (a.walling 2008-07-02 14:27) - PLID 29757 - Use a special cursor for the toolbar buttons
BOOL CNxToolBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	try {
		if (nHitTest == HTCLIENT) {
			CPoint pt;
			GetMessagePos(pt);

			ScreenToClient(&pt);

			CToolBarCtrl& tb = GetToolBarCtrl();

			long nIndex = tb.HitTest(&pt);

			if (nIndex >= 0) {
				TBBUTTON tbButton;
				if (tb.GetButton(nIndex, &tbButton)) {
					if (GetMainFrame()->HandleToolbarButtonMouseMessage(tbButton.idCommand, FALSE)) {
						//d.thompson 10/6/2015 - PLID 67291 - IDC_HELP is already MAKEINTRESOURCE(id), don't call it again.
						SetCursor(::LoadCursor(NULL, IDC_HELP));
						return TRUE;
					}
				}
			}
		}
	} NxCatchAllCallIgnore({LogDetail("Error handling toolbar OnSetCursor");};);

	return CToolBar::OnSetCursor(pWnd, nHitTest, message);
}

// (a.walling 2008-10-02 17:23) - PLID 31575 - Border drawing (NcPaint)
void CNxToolBar::OnNcPaint()
{
	// (a.walling 2012-03-02 10:43) - PLID 48589 - No more toolbar borders
	/*if (g_bClassicToolbarBorders) {
		CToolBar::OnNcPaint();
	}*/
	return;
}

// (a.walling 2008-10-08 16:31) - PLID 31575 - Absorb any right mouse clicks
void CNxToolBar::OnRButtonDown(UINT nFlags, CPoint point)
{

	// (a.walling 2010-11-26 13:08) - PLID 40444 - Create a popup menu for the module, if available
	try {
		CToolBarCtrl& tb = GetToolBarCtrl();

		long nIndex = tb.HitTest(&point);

		if (nIndex >= 0) {
			TBBUTTON tbButton;
			if (tb.GetButton(nIndex, &tbButton)) {

				if (!(tbButton.fsState & TBSTATE_ENABLED)) {
					return;
				}

				Modules::Type moduleType = Modules::Invalid;

				switch (tbButton.idCommand) {
					case ID_PATIENTS_MODULE:
						moduleType = Modules::Patients;
						break;
					case ID_SCHEDULER_MODULE:
						moduleType = Modules::Scheduler;
						break;
					case ID_LETTER_WRITING_MODULE:
						moduleType = Modules::LetterWriting;
						break;
					case ID_CONTACTS_MODULE:
						moduleType = Modules::Contacts;
						break;
					case ID_MARKETING_MODULE:
						moduleType = Modules::Marketing;
						break;
					case ID_INVENTORY_MODULE:
						moduleType = Modules::Inventory;
						break;
					case ID_FINANCIAL_MODULE:
						moduleType = Modules::Financial;
						break;
					case ID_SURGERYCENTER_MODULE:
						moduleType = Modules::SurgeryCenter;
						break;
					case ID_REPORTS_MODULE:
						moduleType = Modules::Reports;
						break;
					case ID_ADMINISTRATOR_MODULE:
						moduleType = Modules::Admin;
						break;
					case ID_LINKS_MODULE:
						moduleType = Modules::Links;
						break;
					case ID_NEW_PATIENT_BTN:
					case ID_FIRST_AVAILABLE_APPT:
					case ID_CARECREDIT_BTN:
					default:
						// Nothing special here.
						return;
						break;
				}

				PracticeModulePtr module(g_Modules[moduleType]);
				
				CMenu menu;
				long nMenuID = 0x80;
				long nCmdID = 0x100;

				Modules::PopupMenuHelperPtr pModulePopup = Modules::PopupMenuHelper::Create(menu, nMenuID, nCmdID);
				pModulePopup->CreateModulePopupMenu(menu, module);

				if (!menu.GetSafeHmenu()) {
					return;
				}
				
				CPoint mouse;
				GetCursorPos(&mouse);   
				int nCommand = menu.TrackPopupMenu(TPM_RIGHTBUTTON|TPM_RETURNCMD|TPM_NONOTIFY, mouse.x, mouse.y, this, NULL);
			
				// (a.walling 2010-11-26 13:08) - PLID 40444 - Handle the module's menu items
				pModulePopup->HandleCommand(nCommand);
			}
		}
	} NxCatchAllCallIgnore({LogDetail("Error handling toolbar OnRButtonUp");};);

	//CToolBar::OnRButtonDown(nFlags, point);
	return;
}

// (a.walling 2008-10-08 16:31) - PLID 31575 - Absorb any right mouse clicks
void CNxToolBar::OnRButtonUp(UINT nFlags, CPoint point)
{
	//CToolBar::OnRButtonUp(nFlags, point);
	return;
}
