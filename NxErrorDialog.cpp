// NxErrorDialog.cpp: implementation of the CNxErrorDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxErrorDialog.h"
#include "ClipboardUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


enum EGetSystemFontType {
	gsftMenu,
	gsftStatus,
	gsftMessage,
};

BOOL GetSystemFont(OUT CFont &fnt, IN const EGetSystemFontType egsft)
{
	NONCLIENTMETRICS ncm;
	memset(&ncm, 0, sizeof(NONCLIENTMETRICS));
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0)) {
		switch (egsft) {
		case gsftMenu:
			return fnt.CreateFontIndirect(&(ncm.lfMenuFont));
			break;
		case gsftStatus:
			return fnt.CreateFontIndirect(&(ncm.lfStatusFont));
			break;
		case gsftMessage:
		default:
			return fnt.CreateFontIndirect(&(ncm.lfMessageFont));
			break;
		}
	} else {
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNxErrorDialog::CNxErrorDialog()
	:CDialog(IDD_ERROR)
{
	m_pbrush = NULL;

	// Decent defaults, but these will be overridden before the dialog is 
	// visible so it doesn't matter what we set them to here
	l_cnButtonWidth = 120;
	l_cnButtonXBuffer = 20;
	l_cnButtonHeight = 30;
	l_cnButtonYBuffer = 16;
}

CNxErrorDialog::~CNxErrorDialog()
{
	if (m_pbrush)
		delete m_pbrush;
}

BEGIN_MESSAGE_MAP(CNxErrorDialog, CDialog)
	//{{AFX_MSG_MAP(CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_DRAWITEM()
	ON_COMMAND(ID_HELP, OnBtnHelp)
	ON_COMMAND(IDRETRY, OnBtnRetry)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CNxErrorDialog::DoModal(const CString &message/*= ""*/,  
							const CString &title/* = "Error"*/, 
							const ErrorLevel errorLevel/* = USER_ERROR*/,
							OPTIONAL const CString &strBtnText_OK, 
							OPTIONAL const CString &strBtnText_Retry,
							OPTIONAL const CString &strBtnText_Cancel,
							OPTIONAL const CString &strManualLocation,
							OPTIONAL const CString &strManualBookmark)
{
	CString font;
	int fontSize;

	switch (errorLevel)
	{
		case ErrorLevel::WARNING:
			font = "Arial Black";
			fontSize = 120;
			break;			
		case ErrorLevel::ROUTINE_ERROR:
			font = "Times New Roman";
			fontSize = 120;
			break;
		case ErrorLevel::NETWORK_ERROR:
			font = "Times New Roman";
			fontSize = 120;
			break;
		case ErrorLevel::CRITICAL_ERROR:
			font = "Courier";
			fontSize = 200;
			break;
		case ErrorLevel::BUG_ERROR:
		case ErrorLevel::API_ERROR:	// (j.armen 2014-09-09 11:09) - PLID 63594 - API Error's and Warnings can use this font too
		case ErrorLevel::API_WARNING:
		default:
			// (a.walling 2007-07-31 12:18) - We intended to use Lucida Console, but misspelled it as Lucidia.
			// regardless, it looks better with the fallback system font, so we will leave it here for now.
			font = "Lucidia Console";
			fontSize = 110;
			break;
	}


	m_errorLevel = errorLevel;
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&m_font, fontSize, font);
	m_strBtnText_OK = strBtnText_OK;
	m_strBtnText_Retry = strBtnText_Retry;
	m_strBtnText_Cancel = strBtnText_Cancel;
	m_messageText = message;
	m_messageText.TrimRight();
	m_titleText = title;
	m_strManualLocation = strManualLocation;
	m_strManualBookmark = strManualBookmark;
	m_bModal = TRUE;

	return CDialog::DoModal();
}

BOOL CNxErrorDialog::Create(CWnd* pParent,
					const CString &message,
					const CString &title, 
					const ErrorLevel errorLevel,
					OPTIONAL const CString &strBtnText_OK, 
					OPTIONAL const CString &strBtnText_Retry,
					OPTIONAL const CString &strBtnText_Cancel,
					OPTIONAL const CString &strManualLocation,
					OPTIONAL const CString &strManualBookmark)
{
	CString font;
	int fontSize;

	switch (errorLevel)
	{
		case ErrorLevel::WARNING:
			font = "Arial Black";
			fontSize = 120;
			break;
		case ErrorLevel::ROUTINE_ERROR:
			font = "Times New Roman";
			fontSize = 120;
			break;
		case ErrorLevel::NETWORK_ERROR:
			font = "Times New Roman";
			fontSize = 120;
			break;
		case ErrorLevel::CRITICAL_ERROR:
			font = "Courier";
			fontSize = 200;
			break;
		case ErrorLevel::BUG_ERROR:
		case ErrorLevel::API_ERROR: // (j.armen 2014-09-09 11:09) - PLID 63594 - API Errors and Warnings can use this font too
		case ErrorLevel::API_WARNING:
		default:
			// (a.walling 2007-07-31 12:18) - We intended to use Lucida Console, but misspelled it as Lucidia.
			// regardless, it looks better with the fallback system font, so we will leave it here for now.
			font = "Lucidia Console";
			fontSize = 110;
			break;
	}


	m_errorLevel = errorLevel;
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&m_font, fontSize, font);
	m_strBtnText_OK = strBtnText_OK;
	m_strBtnText_Retry = strBtnText_Retry;
	m_strBtnText_Cancel = strBtnText_Cancel;
	m_messageText = message;
	m_messageText.TrimRight();
	m_titleText = title;
	m_strManualLocation = strManualLocation;
	m_strManualBookmark = strManualBookmark;
	m_bModal = FALSE;

	return CDialog::Create(IDD_ERROR, pParent);
}


DWORD CNxErrorDialog::GetColor()
{
	switch (m_errorLevel)
	{
		case ErrorLevel::WARNING:
			return 0xA0FFFF;
		case ErrorLevel::ROUTINE_ERROR:
			return 0x80D080;
		case ErrorLevel::NETWORK_ERROR:
			return 0xFFC0C0;
		case ErrorLevel::BUG_ERROR:
			return 0xC0C0C0;
		case ErrorLevel::CRITICAL_ERROR:
			return 0x00FFFF;
		case ErrorLevel::API_ERROR: // (j.armen 2014-09-09 11:09) - PLID 63594 - API Errors use the beige dialog background color
			return 0xF0F0F0;
		case ErrorLevel::API_WARNING: // (j.armen 2014-09-09 11:09) - PLID 63594 - API Warnings are completly white
			return 0xFFFFFF;
		default:
			return 0x000000;
	}
}

BOOL CNxErrorDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the dialog title
	SetWindowText (m_titleText);

	
	// If the OK and Cancel buttons are both empty, then forcibly make the Cancel button NOT empty
	if (m_strBtnText_OK.IsEmpty() && m_strBtnText_Retry.IsEmpty() && m_strBtnText_Cancel.IsEmpty()) {
		// You should never call this dialog without an OK or a Cancel button, because 
		// they are the only ways for the user to dismiss the window
		ASSERT(FALSE);
		m_strBtnText_Cancel = "Close";
	}

	// Figure out how many buttons there will be
	long nButtonVisibleCount = 0;
	if (!m_strBtnText_OK.IsEmpty()) nButtonVisibleCount++;
	if (!m_strBtnText_Retry.IsEmpty()) nButtonVisibleCount++;
	if (!m_strBtnText_Cancel.IsEmpty()) nButtonVisibleCount++;
	if (!m_strManualLocation.IsEmpty() && !m_strManualBookmark.IsEmpty()) nButtonVisibleCount++;


	// Get the system's official "message box" font if we don't already have it
	GetSystemFont(m_fntMessageBox, gsftMessage);

	// Calculate the button dimensions (including margins i.e. "buffers")
	{
		CDC *pdc = GetDC();
		CFont *pOldFont = pdc->SelectObject(&m_fntMessageBox);
		TEXTMETRIC tm;
		if (pdc->GetTextMetrics(&tm)) {
			// Set the button height to 1.5x the font height plus 10 (2+2 for the top/bottom border 
			// width, 2+2 for the top/bottom distance to the focus rect, 1 for the distance to the "&" 
			// character underline, and 1 for the "&" character underline itself)
			l_cnButtonHeight = tm.tmAscent * 3 / 2 + 10;
			l_cnButtonYBuffer = l_cnButtonHeight / 2;
			// Assume around 20 characters average to do our best to set a good width
			l_cnButtonWidth = tm.tmAveCharWidth * 20 + 10;
			l_cnButtonXBuffer = tm.tmAveCharWidth;
		}
		pdc->SelectObject(pOldFont);
		ReleaseDC(pdc);
	}

	// Get the full desktop work area (that's the screen minus any docked toolbars like the taskbar)
	CRect rcDesktop;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);

	// Assume our window's max size is 3/4 the size of the work area
	CRect rcText;
	{
		long nReduceWidth = (rcDesktop.Width())*3/8; // reduce by 3/8
		long nReduceHeight = (rcDesktop.Height())*3/8; // reduce by 3/8
		rcText.SetRect(0, 0, rcDesktop.Width()-nReduceWidth + GetSystemMetrics(SM_CXBORDER), rcDesktop.Height()-nReduceHeight + GetSystemMetrics(SM_CYBORDER));
	}
	// That's our max window size, now reduce that to get our max area in which the text can be placed
	CRect rcReduceTextArea(l_cnButtonXBuffer*3, l_cnButtonYBuffer, l_cnButtonXBuffer*3, l_cnButtonYBuffer*3 + l_cnButtonHeight + GetSystemMetrics(SM_CYBORDER) * 2 + GetSystemMetrics(SM_CYCAPTION));
	rcText.DeflateRect(rcReduceTextArea);

	// Now we'll reduce that even more by calculating how the text will look
	{
		CDC *pdc = GetDC();
		CFont *pf = pdc->SelectObject(&m_font);
		if (pdc->DrawText(m_messageText, &rcText, DT_TOP|DT_CENTER|DT_VCENTER|DT_WORDBREAK|DT_EXPANDTABS|DT_EXTERNALLEADING|DT_CALCRECT)) {
			// Got the height of that text assuming the width was ok, but there's a special case if it 
			// would be so tall it would go off the screen.  In that case, recalculate the rect allowing 
			// it to take up the whole width of the desktop.
			if (rcText.Height() > rcDesktop.Height()) {
				rcText.right = rcText.left + rcDesktop.Width();
				pdc->DrawText(m_messageText, &rcText, DT_TOP|DT_CENTER|DT_VCENTER|DT_WORDBREAK|DT_EXPANDTABS|DT_EXTERNALLEADING|DT_CALCRECT);
			}
		} else {
			rcText.right = rcText.left;
			rcText.bottom = rcText.top;
		}
		pdc->SelectObject(pf);
		ReleaseDC(pdc);
	}

	// Create a textbox of this size with the text in it
	m_text.Create (m_messageText, WS_CHILD | WS_VISIBLE | WS_GROUP, rcText, this);
	m_text.ModifyStyleEx (0, WS_EX_TRANSPARENT);
	m_text.SetFont(&m_font);

	// Now increase it by the same margins we reduced it by before, to generate final dialog size
	rcText.InflateRect(rcReduceTextArea);
	ASSERT(rcText.left == 0 && rcText.top == 0);  // By inflating by the margin amount we should be back at left and top being 0
	// And make sure it's not too small
	long nMinDlgWidth = (nButtonVisibleCount * (l_cnButtonWidth + l_cnButtonXBuffer) - l_cnButtonXBuffer) + 6 * l_cnButtonXBuffer; // width of all buttons (including buffers between them) PLUS three times the xbuffer on EACH SIDE of the dialog (hence 6*xbuffer)
	if (rcText.Width() < nMinDlgWidth) rcText.InflateRect((nMinDlgWidth-rcText.Width())/2, 0);
	if (rcText.Height() < 100) rcText.InflateRect(0, (100-rcText.Height())/2);

	// Just center the rectangle and make sure it has positive width and height
	rcText.OffsetRect(rcDesktop.left + rcDesktop.Width()/2-rcText.Width()/2-rcText.left, rcDesktop.top + rcDesktop.Height()/2-rcText.Height()/2-rcText.top);
	rcText.NormalizeRect(); ASSERT(rcText.right>rcText.left && rcText.bottom>rcText.top);
	// Make sure the top-left is in the workarea
	{
		if (rcText.left < rcDesktop.left) {
			rcText.OffsetRect(rcDesktop.left - rcText.left, 0);
		}
		if (rcText.top < rcDesktop.top) {
			rcText.OffsetRect(0, rcDesktop.top - rcText.top);
		}
	}

	// Commit the size adjustment while at the same time making the dialog a top-level window
	SetWindowPos(&wndTopMost, rcText.left, rcText.top, rcText.Width(), rcText.Height(), 0);

	// Now base the other position calculations on the new dialog size
	CRect rcClient;
	GetClientRect(&rcClient);

	{
		CRect rcButton;
		
		// All buttons are the same height and vertical position
		rcButton.top = rcClient.bottom - l_cnButtonYBuffer - l_cnButtonHeight;
		rcButton.bottom = rcButton.top + l_cnButtonHeight;
		
		// Initialize the right to be l_cnButtonXBuffer less than the left of the first button so 
		// that the first button to be visible can successfully add l_cnButtonXBuffer in order to 
		// properly calculate its own left
		{
			// The left to be the left of the first button assuming all buttons are to be 
			// centered and that there are nButtonVisibleCount buttons, each with a width of 
			// l_cnButtonWidth with separated by a distance of l_cnButtonXBuffer.
			long nFirstButtonLeft = rcClient.Width() / 2 - (nButtonVisibleCount * (l_cnButtonWidth + l_cnButtonXBuffer) - l_cnButtonXBuffer) / 2;
			// Now knowing the correct anticipated left of the first button, we can just subtract 
			// l_cnButtonXBuffer to get the "init" right.
			rcButton.right = nFirstButtonLeft - l_cnButtonXBuffer;
		}

		// Create the OK button if requested
		if (!m_strBtnText_OK.IsEmpty()) {
			rcButton.left = rcButton.right + l_cnButtonXBuffer;
			rcButton.right = rcButton.left + l_cnButtonWidth;
			m_btnOK.Create(m_strBtnText_OK, WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW, rcButton, this, IDOK);
		}
		// Create the Retry button if requested
		if (!m_strBtnText_Retry.IsEmpty()) {
			rcButton.left = rcButton.right + l_cnButtonXBuffer;
			rcButton.right = rcButton.left + l_cnButtonWidth;
			m_btnRetry.Create(m_strBtnText_Retry, WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW, rcButton, this, IDRETRY);
		}
		// Create the Cancel button if requested
		if (!m_strBtnText_Cancel.IsEmpty()) {
			rcButton.left = rcButton.right + l_cnButtonXBuffer;
			rcButton.right = rcButton.left + l_cnButtonWidth;
			m_btnCancel.Create (m_strBtnText_Cancel, WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW, rcButton, this, IDCANCEL);
		}
		// Create the Help button if requested
		if (!m_strManualLocation.IsEmpty() && !m_strManualBookmark.IsEmpty()) {
			rcButton.left = rcButton.right + l_cnButtonXBuffer;
			rcButton.right = rcButton.left + l_cnButtonWidth;
			m_btnHelp.Create("&Help", WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW, rcButton, this, ID_HELP);
		}
	}


	m_pbrush = new CBrush(GetColor());

	if (!m_strBtnText_OK.IsEmpty()) {
		SetDefID(IDOK);
		m_btnOK.SetFocus();
		m_btnOK.ModifyStyle(0, WS_GROUP);
		return FALSE;
	} else if (!m_strBtnText_Retry.IsEmpty()) {
		SetDefID(IDRETRY);
		m_btnRetry.SetFocus();
		m_btnRetry.ModifyStyle(0, WS_GROUP);
		return FALSE;
	} else if (!m_strBtnText_Cancel.IsEmpty()) {
		SetDefID(IDCANCEL);
		m_btnCancel.SetFocus();
		m_btnCancel.ModifyStyle(0, WS_GROUP);
		return FALSE;
	} else if (!m_strManualLocation.IsEmpty() && !m_strManualBookmark.IsEmpty()) {
		SetDefID(ID_HELP);
		m_btnHelp.SetFocus();
		m_btnHelp.ModifyStyle(0, WS_GROUP);
		return FALSE;
	} else {
		return TRUE;
	}
}

void CNxErrorDialog::OnOK()
{
	if (!m_bModal) PostMessage(NXM_NXERRDLG_IDOK);
	else CDialog::OnOK();
}

void CNxErrorDialog::OnCancel()
{
	if (!m_bModal) PostMessage(NXM_NXERRDLG_IDCANCEL);
	else CDialog::OnCancel();
}

HBRUSH CNxErrorDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch (nCtlColor)
	{	case CTLCOLOR_STATIC:
			pDC->SetBkMode(TRANSPARENT);
		case CTLCOLOR_DLG:
		case CTLCOLOR_BTN:
			return (HBRUSH)m_pbrush->m_hObject;
	}
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CNxErrorDialog::OnEraseBkgnd(CDC* pDC) 
{
	// Need to fill the background with the right color
	CRect rc;
	GetClientRect(&rc);
	pDC->FillSolidRect(rc, GetColor());
	
	// Return non-zero because we've done our job of erasing the background
	return TRUE;
}

void CNxErrorDialog::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect rect = &(lpDrawItemStruct->rcItem);
	CDC *pDC	= CDC::FromHandle(lpDrawItemStruct->hDC);
	CWnd *pWndItem = CWnd::FromHandle(lpDrawItemStruct->hwndItem);
	
	// If we have a font to use for the buttons, use it
	CFont *pOld = NULL;
	if (m_fntMessageBox.m_hObject) {
		pOld = (CFont *)pDC->SelectObject(&m_fntMessageBox);
	}


	if (lpDrawItemStruct->itemState & ODS_SELECTED) {
		// The button is being pressed
		pDC->DrawEdge(rect, EDGE_SUNKEN, BF_RECT);
		rect.DeflateRect(2,2,2,2);
		pDC->FillRect(rect, m_pbrush);
		rect.DeflateRect(1,1,-1,-1);
	} else {
		// The button is not being pressed

		// First see if it is the default button or if it has focus
		if ((lpDrawItemStruct->itemState & ODS_DEFAULT) != 0 || (lpDrawItemStruct->itemState & ODS_FOCUS) != 0) {
			// It is the default btn or it has focus so draw the black border around it
			CPen pn(PS_SOLID, 1, RGB(0,0,0));
			CPen *pOldPen = pDC->SelectObject(&pn);
			pDC->Rectangle(rect);
			pDC->SelectObject(pOldPen);
			rect.DeflateRect(1,1,1,1);
		}

		// Draw the edge to make it look 3d (but not quite as deeply 3d as normal windows buttons)
		pDC->DrawEdge(rect, BDR_RAISEDINNER, BF_RECT);
		rect.DeflateRect(1,1,1,1);
		pDC->FillRect(rect, m_pbrush);
	}
	
	// Draw the text
	{
		// Get the text to draw
		CString strTextToDraw;
		if (pWndItem) {
			pWndItem->GetWindowText(strTextToDraw);
		}
		// Now draw the text
		pDC->SetBkColor(GetColor());
		pDC->DrawText(strTextToDraw, rect, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
	}

	// Draw the focus rect if the button has focus
	if ((lpDrawItemStruct->itemState & ODS_FOCUS) != 0) {
		rect.DeflateRect(2, 2);
		pDC->DrawFocusRect(rect);
	}
	
	// If we set the font above, return it to what it was before
	if (m_fntMessageBox.m_hObject) {
		pDC->SelectObject(pOld);
	}
}


void CNxErrorDialog::OnBtnHelp()
{
	//Let's make ourselves not so topmost
	SetWindowPos(&wndNoTopMost,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
	OpenManual(m_strManualLocation, m_strManualBookmark);
}

void CNxErrorDialog::OnBtnRetry()
{
	if (!m_bModal) PostMessage(NXM_NXERRDLG_IDRETRY);
	else EndDialog(IDRETRY);
}

BOOL CNxErrorDialog::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == 'C') {			
		if (GetKeyState(VK_CONTROL) & 0x80) {
			// ctrl+c!
			try {
				// (a.walling 2014-04-29 16:42) - PLID 61964 - Copy message text to clipboard upon ctrl+c
				CString text = m_messageText;
				Clipboard clip;
				if (clip.Open(GetSafeHwnd()) && clip.Empty()) {
					clip.AddText(text);
				}
			} NxCatchAllIgnore();
		}
	}
	return __super::PreTranslateMessage(pMsg);
}