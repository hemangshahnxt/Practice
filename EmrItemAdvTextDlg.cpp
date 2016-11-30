// EmrItemAdvTextDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EmrItemAdvTextDlg.h"
#include "EMNDetail.h"
#include "LabEntryDlg.h"
#include "EmrTreeWnd.h"
#include "EmrEditorDlg.h"
#include "PicContainerDlg.h"
#include "emrtextmacrodlg.h"
#include "WindowlessUtils.h"
#include "EMRTopic.h"
#include "EMR.h"
#include "EMN.h"

// (a.walling 2011-11-11 11:11) - PLID 46632 - WindowlessUtils - Various functions replaced with windowless-safe versions.

extern CPracticeApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// (c.haag 2010-01-07 13:44) - PLID 36794 - This section reserved for the special edit
// context menu handling

#define MES_UNDO        _T("&Undo")
#define MES_CUT         _T("Cu&t")
#define MES_COPY        _T("&Copy")
#define MES_PASTE       _T("&Paste")
#define MES_DELETE      _T("&Delete")
#define MES_SELECTALL   _T("Select &All")
// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
// this is also an ID, not a message, so get rid of WM_APP
#define ME_SELECTALL    40015
// (c.haag 2010-01-11 12:31) - PLID 31924
#define MES_PASTE_MACRO_TEXT		_T("Paste &Macro Text...")

BEGIN_MESSAGE_MAP(CEmrItemAdvTextEdit, CEdit)
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

void CEmrItemAdvTextEdit::OnContextMenu(CWnd* pWnd, CPoint point)
{
    SetFocus();
    // (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
	CNxMenu menu;
    menu.CreatePopupMenu();
    BOOL bReadOnly = GetStyle() & ES_READONLY;
    DWORD flags = CanUndo() && !bReadOnly ? 0 : MF_GRAYED;
    menu.InsertMenu(0, MF_BYPOSITION | flags, EM_UNDO,
        MES_UNDO);

    menu.InsertMenu(1, MF_BYPOSITION | MF_SEPARATOR);

    DWORD sel = GetSel();
    flags = LOWORD(sel) == HIWORD(sel) ? MF_GRAYED : 0;
    menu.InsertMenu(2, MF_BYPOSITION | flags, WM_COPY,
        MES_COPY);

    flags = (flags == MF_GRAYED || bReadOnly) ? MF_GRAYED : 0;
    menu.InsertMenu(2, MF_BYPOSITION | flags, WM_CUT,
        MES_CUT);
    menu.InsertMenu(4, MF_BYPOSITION | flags, WM_CLEAR,
        MES_DELETE);

    flags = IsClipboardFormatAvailable(CF_TEXT) &&
        !bReadOnly ? 0 : MF_GRAYED;
    menu.InsertMenu(4, MF_BYPOSITION | flags, WM_PASTE,
        MES_PASTE);

    menu.InsertMenu(6, MF_BYPOSITION | MF_SEPARATOR);

    int len = GetWindowTextLength();
    flags = (!len || (LOWORD(sel) == 0 && HIWORD(sel) ==
        len)) ? MF_GRAYED : 0;
    menu.InsertMenu(7, MF_BYPOSITION | flags, ME_SELECTALL,
        MES_SELECTALL);

	// (c.haag 2010-01-11 12:31) - PLID 31924
	flags = (!bReadOnly) ? 0 : MF_GRAYED;
	menu.InsertMenu(8, MF_BYPOSITION | MF_SEPARATOR);
	menu.InsertMenu(9, MF_BYPOSITION | flags, NXM_EMR_ITEM_PASTE_MACRO_TEXT, MES_PASTE_MACRO_TEXT);

	// Make sure we show up if pressed by the Context Menu key
	if (point.x == -1 || point.y == -1)
	{
		GetCursorPos(&point);
	}

    menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON |
        TPM_RIGHTBUTTON, point.x, point.y, this);
}

BOOL CEmrItemAdvTextEdit::OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
    case EM_UNDO:
    case WM_CUT:
    case WM_COPY:
    case WM_CLEAR:
    case WM_PASTE:
		return SendMessage(LOWORD(wParam));
	case NXM_EMR_ITEM_PASTE_MACRO_TEXT: // (c.haag 2010-01-11 12:31) - PLID 31924
        return GetParent()->SendMessage(LOWORD(wParam));
    case ME_SELECTALL:
        return SendMessage (EM_SETSEL, 0, -1);
    default:
        return CEdit::OnCommand(wParam, lParam);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvTextDlg dialog


CEmrItemAdvTextDlg::CEmrItemAdvTextDlg(class CEMNDetail *pDetail)
	: CEmrItemAdvDlg(pDetail)
{
	//{{AFX_DATA_INIT(CEmrItemAdvTextDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CEmrItemAdvTextDlg::~CEmrItemAdvTextDlg()
{
}

// (z.manning, 04/14/2008) - PLID 29632 - Use the defined IDC rather than a hardcoded value
BEGIN_MESSAGE_MAP(CEmrItemAdvTextDlg, CEmrItemAdvDlg)
	//{{AFX_MSG_MAP(CEmrItemAdvTextDlg)
	ON_WM_CREATE()
	ON_EN_CHANGE(EDIT_IDC, OnChangeEdit)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(SPELL_CHECK_IDC, OnSpellCheck)
	ON_BN_CLICKED(LAB_BUTTON_IDC, OnLabButton)
	ON_MESSAGE(NXM_EMR_ITEM_PASTE_MACRO_TEXT, OnPasteMacroText)
	ON_MESSAGE(NXM_LAB_ENTRY_DLG_CLOSED, OnLabEntryDlgClosed)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvTextDlg message handlers

int CEmrItemAdvTextDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	try {
		if (CEmrItemAdvDlg::OnCreate(lpCreateStruct) == -1)
			return -1;

		//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
		DWORD dwDisabled = m_pDetail->GetReadOnly() ? WS_DISABLED : 0;

		// (b.cardillo 2006-11-22 15:06) - PLID 23632 - Create the button that the user will 
		// click in order to spell check the text.  We use an id of 3000 so as to stay out 
		// of the way of the other potential controls on CEmrItemAdvDlg-derived classes.  We 
		// use the system's current title icon font, and we color it blue for consistency 
		// with other buttons on the EMR window.
		// (b.cardillo 2007-01-23 14:47) - PLID 24388 - Got rid of the "Check Spelling" text, 
		// leaving just the icon by itself, so as to save space.
		//TES 1/29/2008 - PLID 28673 - Use a #defined IDC, not a numeric literal.
		// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
		m_btnSpellCheck.Create("", WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_GROUP|BS_PUSHBUTTON|BS_CENTER|BS_OWNERDRAW|dwDisabled, CRect(0,0,0,0), this, SPELL_CHECK_IDC);
		// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
		m_btnSpellCheck.SetFont(CFont::FromHandle(EmrFonts::GetRegularFont()));
		m_btnSpellCheck.SetTextColor(RGB(0,0,255));
		// (b.cardillo 2007-01-23 14:47) - PLID 24388 - Got rid of the "Check Spelling" text, 
		// leaving just the icon by itself, so as to save space.
		m_btnSpellCheck.SetIcon(IDI_SPELLCHECK_ICON);
		m_btnSpellCheck.SetToolTip("Check Spelling");

		if(m_pDetail->IsLabDetail())
		{
			// (z.manning 2008-10-08 12:03) - PLID 31613 - Added a lab button
			// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
			m_btnOpenLab.Create("", WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_GROUP|BS_PUSHBUTTON|BS_CENTER|BS_OWNERDRAW|dwDisabled, CRect(0,0,0,0), this, LAB_BUTTON_IDC);
			// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
			// (z.manning 2012-07-20 15:01) - PLID 51676 - Use a smaller font for this button's text
			m_btnOpenLab.SetFont(CFont::FromHandle(EmrFonts::GetSmallFont()));
			m_btnOpenLab.SetTextColor(RGB(0,0,255));
			m_btnOpenLab.SetWindowText("Open Lab");
		}
		
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

		// (b.cardillo 2006-11-28 15:03) - PLID 23680 - If we're read-only, make the edit box 
		// read-only (instead of disabled).
		//TES 1/29/2008 - PLID 28673 - Use a #defined IDC, not a numeric literal.
		// (z.manning 2008-10-07 16:07) - PLID 31561 - Do not allow the editing of lab details from this dialog.
		//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
		m_edit.CreateEx(
			WS_EX_CLIENTEDGE/*|WS_EX_NOPARENTNOTIFY*/, _T("EDIT"), NULL, 
			WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_TABSTOP|WS_GROUP|ES_LEFT|ES_MULTILINE|ES_WANTRETURN|(m_pDetail->GetReadOnly() || m_pDetail->IsLabDetail() ? ES_READONLY : 0), 
			CRect(0, 0, 300, 300), this, EDIT_IDC);
		// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
		m_edit.SetFont(CFont::FromHandle(EmrFonts::GetRegularFont()));

		return 0;

	} NxCatchAll(__FUNCTION__);

	return -1;
}

BOOL CEmrItemAdvTextDlg::RepositionControls(IN OUT CSize &szArea, BOOL bCalcOnly)
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
	
	const long cnMinEditWidth = 80;
	const long cnMinEditHeight = 25;
	long cnMergeBtnMargin = 5;

	// Make sure the merge status icon button reflects the state of the data,
	// because that will have a direct influence on our size calculations.
	UpdateStatusButtonAppearances();

	CClientDC dc(this);

	long nTopMargin = 3;
	long nIdealWidth = 320;
	long nLabelRight = 0;
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

		// (b.cardillo 2006-11-22 15:14) - PLID 23632 - Fill in the nLabelRight variable so we 
		// can use it below to determine the correct placement for the spell-check button.
		nLabelRight = 6 + sz.cx;

		if (!bCalcOnly) {
			CRect rPrior, rLabel;
			GetControlWindowRect(this, &m_wndLabel, &rPrior);

			m_wndLabel.MoveWindow(6, 2, sz.cx, sz.cy, FALSE);

			GetControlWindowRect(this, &m_wndLabel, &rLabel);

			rPrior.InflateRect(2,2,2,2);
			rLabel.InflateRect(2,2,2,2);
			InvalidateRect(rPrior, TRUE);
			InvalidateRect(rLabel, TRUE);
		}
	}

	// (b.cardillo 2006-11-22 15:11) - PLID 23632 - Prepare the position and dimensions of the 
	// spell-check button based on those of the label.  The spell-check button is immediately 
	// to the right of the label, and immediately above the edit box.
	CRect rcSpellCheckButton;
	if (m_btnSpellCheck.m_hWnd) {
		// Get the size the button needs to be 
		CSize sz(LONG_MAX, LONG_MAX);
		CalcControlIdealDimensions(&dc, &m_btnSpellCheck, sz);

		// Now determine the left based on the right of the label, and the top based on the top 
		// of the edit box (i.e. the bottom of the label) minus the height of the button
		// allows (i.e. never < nTopMargin).  If the right side of the button would go off the 
		// right side of szArea, then put the button below the label instead.
		CPoint ptTopLeft;
		if (nLabelRight + 3 + sz.cx <= szArea.cx) {
			ptTopLeft.x = nLabelRight + 3;
			// Make sure there's vertical room for it by adjusting our overall top margin.
			if (nTopMargin < sz.cy + 1) {
				nTopMargin = sz.cy + 1;
			}
			ptTopLeft.y = nTopMargin - sz.cy;
		} else {
			ptTopLeft.x = 6;
			ptTopLeft.y = nTopMargin;
			// Since it's being placed BELOW the label, we have to increase or overall top 
			// margin to fit it.
			nTopMargin += sz.cy + 1;
		}

		// Regardless of what we actually decided our position would be, IDEALLY we would want 
		// to be on the right side of the label, so indicate that in our nIdealWidth check
		if (nIdealWidth < nLabelRight + 3 + sz.cx) {
			nIdealWidth = nLabelRight + 3 + sz.cx;
		}

		// Remember the position and dimentions of the button
		rcSpellCheckButton = CRect(ptTopLeft, sz);
		
		// Make sure our ideal width includes the button
		if (nIdealWidth < rcSpellCheckButton.right) {
			nIdealWidth = rcSpellCheckButton.right;
		}
		
		// And finally move the button to the appropriate spot (if we're not in calc-only mode)
		if (!bCalcOnly) {
			m_btnSpellCheck.MoveWindow(rcSpellCheckButton, TRUE);
		}

		if(IsWindow(m_btnOpenLab.GetSafeHwnd()))
		{
			// (z.manning 2008-10-08 12:09) - PLID 31613 - Move the open lab button if we have one
			CRect rcLabButton(rcSpellCheckButton);
			rcLabButton.left = rcSpellCheckButton.right + 2;
			rcLabButton.right = rcLabButton.left + 60;
			if(!bCalcOnly) {
				m_btnOpenLab.MoveWindow(rcLabButton, TRUE);
			}
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
	
	// Make sure we're not smaller than the minimum
	long nMinWidth = 20 + cnMinEditWidth;
	long nMinHeight = nTopMargin + cnMinEditHeight + cnMergeBtnMargin + szMergeBtn.cy + 10;
	if (szArea.cx < nMinWidth) {
		szArea.cx = nMinWidth;
	}
	if (szArea.cy < nMinHeight) {
		szArea.cy = nMinHeight;
	}

	if (!bCalcOnly && m_edit.m_hWnd) {
		m_edit.MoveWindow(10, nTopMargin, szArea.cx - 20, szArea.cy - nTopMargin - szMergeBtn.cy - cnMergeBtnMargin - 10);
	}

	// (c.haag 2006-07-03 09:29) - PLID 19944 - We now use the variable x to calculate
	// the correct position of each iconic button
	int x = 10;
	if (!bCalcOnly && IsMergeButtonVisible()) {
		m_pBtnMergeStatus->MoveWindow(x, szArea.cy - szMergeBtn.cy - 10, szMergeBtn.cx, szMergeBtn.cy);
		x += szMergeBtn.cx + 3;
	}
	if (!bCalcOnly && IsProblemButtonVisible()) {
		m_pBtnProblemStatus->MoveWindow(x, szArea.cy - szMergeBtn.cy - 10, szMergeBtn.cx, szMergeBtn.cy);
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

void CEmrItemAdvTextDlg::OnChangeEdit() 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		CString str;
		// (z.manning, 04/14/2008) - PLID 29632 - Use the defined IDC rather than a hardcoded value
		GetDlgItemText(EDIT_IDC, str);
		m_pDetail->RequestStateChange((LPCTSTR)str);
		GetDlgItem(EDIT_IDC)->SetFocus();
	} NxCatchAll("Error in OnChangeEdit");
}

void CEmrItemAdvTextDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		CEmrItemAdvDlg::OnShowWindow(bShow, nStatus);
		
		// (a.walling 2012-05-16 15:25) - PLID 50430 - Don't set focus when showing the window!!
		//if (bShow) {
		//	// (z.manning, 04/14/2008) - PLID 29632 - Use the defined IDC rather than a hardcoded value
		//	GetDlgItem(EDIT_IDC)->SetFocus();
		//}
	} NxCatchAll("Error in OnShowWindow");
}

void CEmrItemAdvTextDlg::ReflectCurrentState()
{
	CEmrItemAdvDlg::ReflectCurrentState();

	if(m_edit.GetSafeHwnd()) {
		CString strPriorText;
		m_edit.GetWindowText(strPriorText);
		CString strNewText = AsString(m_pDetail->GetState());
		if (strNewText != strPriorText) {
			int nStart, nEnd;
			m_edit.GetSel(nStart, nEnd);
			m_edit.SetWindowText(AsString(m_pDetail->GetState()));
			m_edit.SetSel(nStart, nEnd);
		}
	}
}

//TES 3/23/2010 - PLID 37757 - This dialog doesn't maintain its own ReadOnly flag, so I changed the function name from
// SetReadOnly() to ReflectReadOnlyStatus()
void CEmrItemAdvTextDlg::ReflectReadOnlyStatus(BOOL bReadOnly)
{
	//TES 3/15/2010 - PLID 37757 - The ReadOnly flag lives in the detail now
	//m_bReadOnly = bReadOnly;
	if(GetSafeHwnd()) {
		m_wndLabel.EnableWindow(!m_pDetail->GetReadOnly());
		// (b.cardillo 2006-11-28 15:03) - PLID 23680 - If we're read-only, make the edit box 
		// read-only (instead of disabled).
		if(m_pDetail->IsLabDetail()) {
			// (z.manning 2008-10-07 16:08) - PLID 31561 - Lab details are ALWAYS read only.
			m_edit.SetReadOnly(TRUE);
			// (z.manning 2008-10-08 15:36) - Don't allow spell checking on lab details since that can
			// change the text.
			m_btnSpellCheck.EnableWindow(FALSE);
		}
		else {
			m_edit.SetReadOnly(bReadOnly);
			// (b.cardillo 2006-11-27 16:32) - PLID 23632 - Need to have the spell check button follow the "read-only"ness just like everything else.
			m_btnSpellCheck.EnableWindow(!m_pDetail->GetReadOnly());
		}
		// (z.manning 2008-10-08 12:23) - PLID 31613 - Disable the open lab button
		if(IsWindow(m_btnOpenLab.GetSafeHwnd())) {
			m_btnOpenLab.EnableWindow(!m_pDetail->GetReadOnly());
		}
	}

	CEmrItemAdvDlg::ReflectReadOnlyStatus(bReadOnly);
}

// (b.cardillo 2006-11-22 15:15) - PLID 23632 - Handler for the spell-check button.
void CEmrItemAdvTextDlg::OnSpellCheck()
{
	try {

		// (z.manning 2008-12-16 13:29) - PLID 27682 - Moved this code to a global utility function.
		SpellCheckEditControl(this, &m_edit);
		
	} NxCatchAll("CEmrItemAdvTextDlg::OnSpellCheck");
}

// (z.manning 2008-10-08 12:33) - PLID 31613 - Open the lab entry dialog for the lab associated with this detail
void CEmrItemAdvTextDlg::OnLabButton()
{
	try
	{
		OpenLabEntryDialog();

	}NxCatchAll("CEmrItemAdvTextDlg::OnLabButton");
}

// (z.manning 2009-09-23 09:58) - PLID 33612 - Moved this logic to its own function
void CEmrItemAdvTextDlg::OpenLabEntryDialog()
{
	// (z.manning 2009-10-19 10:05) - PLID 33612 - I had been setting 'this' as the parent to
	// the lab entry dialog but doing so resulted in the lab entry not being modal when opening
	// from a popped up lab detail.
	// (c.haag 2010-07-16 9:51) - PLID 34338 - New way of opening labs. Legacy code commented out.
	long nPicID = -1;
	CEMN *pEmn = m_pDetail->m_pParentTopic->GetParentEMN();
	CEmrTreeWnd *pTree = NULL;
	CEMR *pEmr = NULL;
	if(pEmn != NULL) {
		pEmr = pEmn->GetParentEMR();
		pTree = pEmn->GetInterface();
		// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
		if(pTree && pTree->GetPicContainer()) {
			nPicID = pTree->GetPicContainer()->GetCurrentPicID();
		}
	}

	if(pTree != NULL && pEmr != NULL) {
		// (z.manning 2009-06-30 12:49) - PLID 34340 - In order to avoid potential weird issues
		// when viewing problems from the lab dialog, let's force them to save the entire EMR if
		// they have any unsaved problems or problem links.
		if(pEmr->DoesEmrOrChildrenHaveUnsavedProblems() || pEmr->DoesEmrOrChildrenHaveUnsavedProblemLinks()) {
			int nMsgResult = MessageBox("You must save this entire EMR before opening this lab.\r\n\r\n"
				"Would you like to save the EMR now?", "Save EMR", MB_YESNO|MB_ICONQUESTION);
			if(nMsgResult != IDYES) {
				return;
			}
			if(FAILED(pTree->SaveEMR(esotEMR, -1, TRUE))) {
				MessageBox("The EMR failed to save.");
				return;
			}
		}
	}

	long nLabID = VarLong(m_pDetail->GetLabID(), -1);
	if(nLabID == -1) {
		// (z.manning 2008-10-08 12:34) - PLID 31613 - This button should not exist unless we have a valid
		// lab ID
		ThrowNxException("CEmrItemAdvTextDlg::OnLabButton - Tried to open lab dialog for non-lab detail (ID = %li).", m_pDetail->GetID());
	}

	//TES 8/10/2011 - PLID 44901 - They may not have permission to view this lab, based on its location
		CArray<long,long> arAllowedLocationIDs;
		if(!PollLabLocationPermissions(arAllowedLocationIDs)) {
			//TES 8/10/2011 - PLID 44901 - If that function had returned TRUE, we'd know they had permission to all locations.  Since they
			// don't, we need to look up this lab's location.
			ADODB::_RecordsetPtr rsLocationID = CreateParamRecordset("SELECT LocationID FROM LabsT WHERE ID = {INT}", nLabID);
			if(!rsLocationID->eof) {
				long nLocationID = AdoFldLong(rsLocationID, "LocationID");
				bool bAllowed = false;
				for(int i = 0; i < arAllowedLocationIDs.GetSize() && !bAllowed; i++) {
					if(arAllowedLocationIDs[i] == nLocationID) bAllowed = true;
				}
				if(!bAllowed) {
					MsgBox("You do not have permission to view this lab, due to the location to which it is assigned.\r\n"
						"Please see your office manager for assistance.");
					return;
				}
			}
		}

	// (j.jones 2010-09-01 09:43) - PLID 40094 - give the EMN location as the default location for new labs
	// (a.walling 2012-07-10 14:16) - PLID 46648 - Dialogs must use a parent
	GetMainFrame()->OpenLab(this, m_pDetail->GetPatientID(), -1, ltInvalid, nLabID, -1, "", nPicID, FALSE, TRUE, GetSafeHwnd(), pEmn->GetLocationID());
}

// (c.haag 2010-01-11 12:35) - PLID 31924 - This function will have the user choose a macro, and then
// insert the text of that macro where the cursor is.
LRESULT CEmrItemAdvTextDlg::OnPasteMacroText(WPARAM wParam, LPARAM lParam)
{
	try {
		CEmrTextMacroDlg dlg(this);
		if (IDOK == dlg.DoModal()) {
			// User chose macro text. Replace the current selection with the macro text.
			m_edit.ReplaceSel(dlg.m_strResultTextData);
		} else {
			// User changed their mind
		}
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

// (c.haag 2010-07-15 17:23) - PLID 34338 - This message is received when a lab entry dialog is closed
// and the lab was opened from this specific dialog.
LRESULT CEmrItemAdvTextDlg::OnLabEntryDlgClosed(WPARAM wParam, LPARAM lParam)
{
	try {
		CLabEntryDlg *pDlg = (CLabEntryDlg *)lParam;
		if (pDlg) {
			CLabEntryDlg& dlg = *pDlg;
			CEMN *pEmn = m_pDetail->m_pParentTopic->GetParentEMN();
			CEmrTreeWnd *pTree = NULL;
			CEMR *pEmr = NULL;
			if(pEmn != NULL) {
				pEmr = pEmn->GetParentEMR();
				pTree = pEmn->GetInterface();
			}
			long nLabID = VarLong(m_pDetail->GetLabID(), -1);

			ASSERT_VALID((CObject*)pDlg);

			//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
			if(dlg.HasDataChanged() && !m_pDetail->GetReadOnly())
			{
				//TES 12/17/2009 - PLID 36622 - The dialog now has an array of saved labs, so we need to go through each of them.
				// (c.haag 2010-07-01 11:52) - PLID 36061 - And create details for new labs
				int i=0;
				if (NULL != pEmn) {
					CEMRTopic *pTopic = m_pDetail->m_pParentTopic;
					for(i = 0; i < dlg.m_aryNewLabs.GetSize(); i++) {
						EMNLab lab = dlg.m_aryNewLabs.GetAt(i);
						EMNLab *pNewLab = new EMNLab;
						*pNewLab = lab;

						// (j.jones 2010-09-01 11:54) - PLID 40344 - Formerly we checked to see
						// if the source detail was NULL, and didn't create a lab detail if it was,
						// because we assumed that was impossible to have a lab that wasn't spawned.
						// That is true, but it is however quite possible to simply delete the spawning
						// detail, and keep the lab detail. The lab detail still has a source action info.
						// pointing to the deleted detail, it's pointer is just NULL. So continue to
						// add the new labe, and go ahead and copy that source action info. information,
						// even though the source action is effectively meaningless now.

						// (c.haag 2010-07-01 11:52) - PLID 36061 - Get the source action information from this lab detail.
						pNewLab->sai = m_pDetail->GetSourceActionInfo();

						if(pTopic != NULL) {
							pEmn->CreateNewLabDetail(pTopic, pNewLab, FALSE);
						}
						pEmn->AddLab(pNewLab);
					}
				}
				else {
					// If we get here, there's no EMN to add new labs to.
				}

				// (j.jones 2010-08-30 09:03) - PLID 40095 - check saved labs after checking for new labs,
				// since it is possible a new lab may be in both
				for(i = 0; i < dlg.m_arSavedExistingLabs.GetSize(); i++) {
					EMNLab lab = dlg.m_arSavedExistingLabs[i];
					CEMN *pEMN = m_pDetail->m_pParentTopic->GetParentEMN();
					//TES 12/17/2009 - PLID 36622 - Update the EMNLab object.
					EMNLab *pLab = pEMN->GetLabByID(nLabID);
					if(pLab != NULL) {
						pLab->CopyLabDetailsOnly(lab);
					}
					//TES 12/17/2009 - PLID 36622 - Now, update the details, if the text changed.
					CString strNewText = lab.GetText();
					CArray<CEMNDetail*,CEMNDetail*> arDetails;
					pEMN->GetDetailsByLabID(lab.nID, arDetails);
					for(int i = 0; i < arDetails.GetSize(); i++) {
						CString strPreviousText = VarString(arDetails[i]->GetState(),"");
						if(strPreviousText != strNewText) {
							arDetails[i]->RequestStateChange((LPCTSTR)strNewText);
						}
					}
				}
			}
			
			// (z.manning 2008-11-04 09:56) - PLID 31904 - If we opened a report we need to minimize the PIC
			if(dlg.HasOpenedReport()) {
				if(pTree != NULL) {
					pTree->SendMessage(NXM_EMR_MINIMIZE_PIC);
				}
			}


		}
	}
	NxCatchAll(__FUNCTION__)
	return 0;
}

//(s.dhole 12/19/2014 1:45 PM ) - PLID 63571 handle escape key
BOOL CEmrItemAdvTextDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_ESCAPE) {
		return TRUE;
	}
	return PreTranslateInput(pMsg);;
}