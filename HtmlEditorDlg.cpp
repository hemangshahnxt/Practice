// HtmlEditorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "HtmlEditorDlg.h"
#include <NxSystemUtilitiesLib/ResourceUtils.h>
#include <NxSystemUtilitiesLib/NxCompressUtils.h>
#include <boost/assign/list_of.hpp>

// (b.cardillo 2013-03-11 18:00) - PLID 55576 - Dialog for easy WYSIWYG editing of html

// NOTE: Because this is a general use dialog, we don't put an NxColor on the background. It's 
// akin to InputBox(). Someday we may want to consider making both InputBox() and this dialog 
// have optional background colors, but that day is not today.

#define IDC_HTML_EDITOR_CONTROL 1001

// CHtmlEditorDlg dialog

IMPLEMENT_DYNAMIC(CHtmlEditorDlg, CNxDialog)

// (b.cardillo 2013-03-12 14:06) - PLID 55576 supplemental - Since we're sizable, support storing/recalling our size and position
// Simple as possible: just call this static function to edit some html
// Always updates strHtmlInOut with the final html even if the user clicked the Cancel button.
// Returns IDOK or IDCANCEL depending on what the user clicked to dismiss the dialog.
int CHtmlEditorDlg::InputHtmlBox(CWnd* pParent, const CString &strTitle, const CString &strSizingConfigRT, IN OUT CString &strHtmlInOut)
{
	CHtmlEditorDlg dlg(pParent);
	dlg.m_strTitle = strTitle;
	dlg.m_strSizingConfigRT = strSizingConfigRT;
	dlg.m_strHtml = strHtmlInOut;
	int ans = dlg.DoModal();
	strHtmlInOut = dlg.m_strHtml;
	return ans;
}

CHtmlEditorDlg::CHtmlEditorDlg(CWnd* pParent /*=NULL*/)
	: SafeMsgProc<CNxDialog>(CHtmlEditorDlg::IDD, pParent)
{
	// Only this first one really needs to be initialized, as it's checked to know whether the rest have been
	m_nSizePos_OKandCancelDistanceBetween = 0;
	m_nSizePos_OKandCancelTop = 0;
	m_nSizePos_OKandCancelHeight = 0;
	m_nSizePos_OKWidth = 0;
	m_nSizePos_CancelWidth = 0;
	m_nSizePos_WindowMargin = 0;

	// Decent minimum size for the dialog so the user on a typical machine can see the toolbars and the content area
	SetMinSize(495, 221);
}

CHtmlEditorDlg::~CHtmlEditorDlg()
{
}

void CHtmlEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}

BOOL CHtmlEditorDlg::OnInitDialog() 
{
	BOOL ans = __super::OnInitDialog();

	m_btnOK.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	// Reflect the title our caller wants us to have
	SetWindowText(m_strTitle);
	m_pHtmlEditor->SetHtml(m_strHtml);

	// Now that all our controls have been created, we need to do the initial positioning of everything
	SetControlPositions();

	// (b.cardillo 2013-03-12 14:06) - PLID 55576 supplemental - Since we're sizable, support storing/recalling our size and position
	// If the caller has provided a specific sizing recall key, then do the recall, otherwise just default 
	// to standard middle-of-the-screen-default-size
	if (!m_strSizingConfigRT.IsEmpty()) {
		SetRecallSizeAndPosition("CHtmlEditorDlg-" + m_strSizingConfigRT, true);
	}

	// Set initial focus and return false to let MFC know not to change it
	// NOTE: At present there is a bug (plid 55589) in the way we embed the web browser control in a child 
	// window so our call to SetFocus() here doesn't have the intended result of letting the user interact 
	// with the browser immediately (without having to click with the mouse). However, once that bug is 
	// fixed, this should then work correctly.
	GetDlgItem(IDC_HTML_EDITOR_CONTROL)->SetFocus();
	return FALSE;
}

BEGIN_MESSAGE_MAP(CHtmlEditorDlg, CNxDialog)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDOK, &CHtmlEditorDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CHtmlEditorDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CHtmlEditorDlg message handlers

int CHtmlEditorDlg::OnCreate(LPCREATESTRUCT lpcs)
{
	try {
		int nResult = __super::OnCreate(lpcs);
		if (nResult == 0) {
			// The base class succeeded, go ahead and create our html editor control

			// Put the editor files we need out on the hd
			CString strEditorHtmlFilePath = GetNxTempPath() ^ _T("NxHtmlEditor") ^ _T("NxHtmlEditor.html");
			if (!FileUtils::DoesFileOrDirExist(strEditorHtmlFilePath)) {
				_variant_t vt = ResourceUtils::LoadResourceDataInfoSafeArrayOfBytes(NULL, "CKEditor.zip", "ZIP");
				FileUtils::CreateFileFromSafeArrayOfBytes(vt, GetNxTempPath() ^ "CKEditor.zip");
				NxCompressUtils::NxUncompressFileToFile(GetNxTempPath() ^ "CKEditor.zip", GetNxTempPath() ^ "NxHtmlEditor");
			}

			// Create the editor control
			m_pHtmlEditor.reset(new CNxHtmlEditor(strEditorHtmlFilePath));
			if (!m_pHtmlEditor->Create(CRect(0, 0, 1, 1), this, IDC_HTML_EDITOR_CONTROL)) {
				ThrowNxException("Failed to create NxHtmlEditor control.");
			}

			// Update the window styles
			if (m_pHtmlEditor->GetSafeHwnd()) {
				m_pHtmlEditor->ModifyStyleEx(WS_EX_NOPARENTNOTIFY, WS_EX_CONTROLPARENT);
			} else {
				ThrowNxException("Created NxHtmlEditor control but could not update styles.");
			}

			// Success
			return 0;
		} else {
			// The base class failed, we need to return whatever it returned
			return nResult;
		}
	} NxCatchAllCall(__FUNCTION__, {
		// Failed
		return -1;
	});
}

// Our own implementation of GetControlPositions() and SetControlPositions() since we don't want 
// the crazy "scale everything" functionality, we just want our embedded html editor control to 
// scale with the dialog.
int CHtmlEditorDlg::GetControlPositions()
{
	__super::GetControlPositions();

	{
		// From the OK button we can get almost everything we need: window margin, ok width, and ok/cancel button top position and height; we also get ok button right position for use in a calculation below
		int nRightOK;
		{
			CWnd *pwndOK = GetDlgItem(IDOK);
			if (IsWindow(pwndOK->GetSafeHwnd())) {
				CRect rc;
				pwndOK->GetWindowRect(&rc);
				ScreenToClient(&rc);
				m_nSizePos_OKandCancelTop = rc.top;
				m_nSizePos_OKandCancelHeight = rc.Height();
				m_nSizePos_OKWidth = rc.Width();
				nRightOK = rc.right;
				CRect rcClient;
				GetClientRect(&rcClient);
				m_nSizePos_WindowMargin = rcClient.bottom - rc.bottom;
			}
		}
		// So for the cancel button since we have the OK button's right position we can calculate the distance between then, and we also need the cancel button's width
		{
			CWnd *pwndCancel = GetDlgItem(IDCANCEL);
			if (IsWindow(pwndCancel->GetSafeHwnd())) {
				CRect rc;
				pwndCancel->GetWindowRect(&rc);
				ScreenToClient(&rc);
				m_nSizePos_CancelWidth = rc.Width();
				m_nSizePos_OKandCancelDistanceBetween = rc.left - nRightOK;
			}
		}
	}

	return 1;
}

// Our own implementation of GetControlPositions() and SetControlPositions() since we don't want 
// the crazy "scale everything" functionality, we just want our embedded html editor control to 
// scale with the dialog.
int CHtmlEditorDlg::SetControlPositions()
{
	// Base class does nothing with our controls because we return FALSE from IncludeChildPosition()
	__super::SetControlPositions();

	// If we've been initilized, then set our control positions
	if (m_hWnd && m_nSizePos_OKandCancelDistanceBetween != 0) {
		// Get all three handles so we can make sure they're all valid before we do anything
		HWND hwndOK = GetDlgItem(IDOK)->GetSafeHwnd();
		HWND hwndCancel = GetDlgItem(IDCANCEL)->GetSafeHwnd();
		HWND hwndHtmlEditor = GetDlgItem(IDC_HTML_EDITOR_CONTROL)->GetSafeHwnd();
		if (IsWindow(hwndOK) && IsWindow(hwndCancel) && IsWindow(hwndHtmlEditor)) {
			CRect rcClient;
			GetClientRect(&rcClient);

			// Turn off redraw if we're visible
			bool bNeedResetRedraw = false;
			if (IsWindowVisible()) {
				bNeedResetRedraw = true;
				SetRedraw(FALSE);
			}
			
			// Do the moves all at once without a redraw
			{
				// Begin
				HDWP hdwp = BeginDeferWindowPos(3);

				// Do it
				{
					// OK button
					hdwp = ::DeferWindowPos(hdwp, hwndOK, NULL
						, (rcClient.right + rcClient.left) / 2 - (m_nSizePos_OKandCancelDistanceBetween / 2) - m_nSizePos_OKWidth
						, rcClient.bottom - m_nSizePos_WindowMargin - m_nSizePos_OKandCancelHeight
						, m_nSizePos_OKWidth
						, m_nSizePos_OKandCancelHeight
						, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW|SWP_NOOWNERZORDER);
					// Cancel button
					hdwp = ::DeferWindowPos(hdwp, hwndCancel, NULL
						, (rcClient.right + rcClient.left) / 2 + (m_nSizePos_OKandCancelDistanceBetween - m_nSizePos_OKandCancelDistanceBetween / 2) 
						, rcClient.bottom - m_nSizePos_WindowMargin - m_nSizePos_OKandCancelHeight
						, m_nSizePos_CancelWidth
						, m_nSizePos_OKandCancelHeight
						, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW|SWP_NOOWNERZORDER);
					// Html editor control
					hdwp = ::DeferWindowPos(hdwp, hwndHtmlEditor, NULL
						, rcClient.left + m_nSizePos_WindowMargin
						, rcClient.top + m_nSizePos_WindowMargin
						, rcClient.Width() - (2 * m_nSizePos_WindowMargin)
						, rcClient.bottom - m_nSizePos_WindowMargin - m_nSizePos_OKandCancelHeight - (2 * m_nSizePos_WindowMargin)
						, SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW|SWP_NOOWNERZORDER);
				}

				// Commit
				::EndDeferWindowPos(hdwp);
			}

			// Set redraw back on if we turned it off
			if (bNeedResetRedraw) {
				SetRedraw(TRUE);
			}

			// Ensure a repaint of the client area
			RedrawWindow(rcClient, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
		}
		return 3;
	} else {
		return 0;
	}
}

// Since we're implementing GetControlPositions() and SetControlPositions() ourselves, we don't want 
// the base class to include any of our children in its functionality.
BOOL CHtmlEditorDlg::IncludeChildPosition(IN HWND hwnd)
{
	// We're handling the sizing and positioning of our controls, so tell CNexTechDialog not to handle any
	return FALSE;
}

void CHtmlEditorDlg::OnBnClickedOk()
{
	// Retrieve the edited html before proceeding
	if (m_pHtmlEditor != NULL) {
		m_strHtml = m_pHtmlEditor->GetHtml();
	}

	// And finish with the IDOK response
	__super::OnOK();
}

void CHtmlEditorDlg::OnBnClickedCancel()
{
	// Retrieve the edited html before proceeding, even though we're canceling the caller might still 
	// want it. Also if the user made changes, warn they'll be lost.
	if (m_pHtmlEditor != NULL) {
		CString strHtml = m_pHtmlEditor->GetHtml();
		if (strHtml != m_strHtml) {
			if (MessageBox(_T("If you proceed, the changes you have made will be lost. Are you sure you want to continue and lose the changes?"), m_strTitle, MB_OKCANCEL|MB_ICONEXCLAMATION) != IDOK) {
				// Not canceling after all
				return;
			}
		}
		m_strHtml = strHtml;
	}

	// And finish with the ICANCEL response
	__super::OnCancel();
}

BOOL CHtmlEditorDlg::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message) {
	case WM_KEYUP:
	case WM_KEYDOWN:
		switch (pMsg->wParam) {
		case VK_ESCAPE:
		case VK_RETURN:
			// The RETURN and ESCAPE keys would dismiss our dialog, but the html editor can pop up its own 
			// internal "dialogs" and such so the user could hit one of these keys thinking they are 
			// dismissing those screens. So that that expectation is not fatally disappointed, we'll just 
			// ignore those keys when the focus is in the editor.
			if (IsOrHasAncestor(GetFocus()->GetSafeHwnd(), m_pHtmlEditor->GetSafeHwnd())) {
				// Don't translate this message for our own purposes, let the control handle it
				return TRUE;
			}
		}
		break;
	default:
		break;
	}
	
	return __super::PreTranslateMessage(pMsg);
}
