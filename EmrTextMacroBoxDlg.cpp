// EmrTextMacroBoxDlg.cpp : implementation file
//
// (c.haag 2008-06-06 09:25) - PLID 26038 - Initial implementation
//

#include "stdafx.h"
#include "administratorrc.h"
#include "EmrTextMacroBoxDlg.h"
#include "EmrItemAdvTextDlg.h"
#include "SpellExUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CPracticeApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CEmrTextMacroBoxDlg dialog


CEmrTextMacroBoxDlg::CEmrTextMacroBoxDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrTextMacroBoxDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEmrTextMacroBoxDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pButtonFont = NULL;
}

void CEmrTextMacroBoxDlg::RepositionControls()
{
	CRect rcWindow;
	GetClientRect(&rcWindow);
	CSize szArea(rcWindow.Width(),rcWindow.Height());
	CSize szBorder;
	CSize szMergeBtn;
	CalcWindowBorderSize(this, &szBorder);
	// Adjust off the border
	szArea.cx -= szBorder.cx;
	szArea.cy -= szBorder.cy;
	
	const long cnMinEditWidth = 80;
	const long cnMinEditHeight = 25;

	long nTopMargin = 3;
	long nIdealWidth = 320;
	long nLabelRight = 0;

	CClientDC dc(this);

	// Reposition the label
	if (IsWindow(m_wndLabel.GetSafeHwnd())) {
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

		// (b.cardillo 2006-11-22 15:14) - Fill in the nLabelRight variable so we 
		// can use it below to determine the correct placement for the spell-check button.
		nLabelRight = 6 + sz.cx;

		{
			CRect rPrior, rLabel;
			m_wndLabel.GetWindowRect(rPrior);
			ScreenToClient(&rPrior);

			m_wndLabel.MoveWindow(6, 2, sz.cx, sz.cy, FALSE);

			m_wndLabel.GetWindowRect(rLabel);
			ScreenToClient(&rLabel);
			rPrior.InflateRect(2,2,2,2);
			rLabel.InflateRect(2,2,2,2);
			InvalidateRect(rPrior, TRUE);
			InvalidateRect(rLabel, TRUE);
		}		
	}

	// Reposition the spell check button
	CRect rcSpellCheckButton;
	if (IsWindow(m_btnSpellCheck.GetSafeHwnd())) {
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
		m_btnSpellCheck.MoveWindow(rcSpellCheckButton, TRUE);
	}

	// Reposition the edit control
	if (IsWindow(m_edit.GetSafeHwnd())) {
		// Make sure we're not smaller than the minimum
		long nMinWidth = 20 + cnMinEditWidth;
		long nMinHeight = nTopMargin + cnMinEditHeight + 10;
		if (szArea.cx < nMinWidth) {
			szArea.cx = nMinWidth;
		}
		if (szArea.cy < nMinHeight) {
			szArea.cy = nMinHeight;
		}
		m_edit.MoveWindow(10, nTopMargin, szArea.cx - 20, szArea.cy - nTopMargin - 10);
	}
}

void CEmrTextMacroBoxDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmrTextMacroBoxDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmrTextMacroBoxDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEmrTextMacroBoxDlg)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(SPELL_CHECK_IDC, OnSpellCheck)
	ON_EN_CHANGE(EMR_TEXT_MACRO_EDIT_IDC, OnChangeEditTextData)
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrTextMacroBoxDlg message handlers

BOOL CEmrTextMacroBoxDlg::OnInitDialog() 
{
	// (c.haag 2008-06-06 09:40) - Almost all the code here is copied straight
	// from CEmrItemAdvTextDlg
	try {
		CNxDialog::OnInitDialog();

		m_brMacroBackground.CreateSolidBrush(GetBackgroundColor());

		// Create the spell check button
		LOGFONT lf;
		if(SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0)) {
			m_pButtonFont = new CFont;
			if (!m_pButtonFont->CreateFontIndirect(&lf)) {
				delete m_pButtonFont;
				m_pButtonFont = NULL;
			}
		}
		// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
		m_btnSpellCheck.Create("", WS_CHILD|WS_VISIBLE|WS_GROUP|BS_PUSHBUTTON|BS_CENTER|BS_OWNERDRAW, CRect(0,0,0,0), this, SPELL_CHECK_IDC);
		m_btnSpellCheck.SetFont(m_pButtonFont);
		m_btnSpellCheck.SetTextColor(RGB(0,0,255));
		m_btnSpellCheck.SetIcon(IDI_SPELLCHECK_ICON);
		m_btnSpellCheck.SetToolTip("Check Spelling");

		// Create the label
		// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
		m_wndLabel.Create("", WS_CHILD|WS_VISIBLE|WS_GROUP, CRect(0,0,0,0), this);
		//m_wndLabel.ModifyStyleEx(0, WS_EX_TRANSPARENT); // (a.walling 2011-05-11 14:55) - PLID 43661 - Labels need WS_EX_TRANSPARENT exstyle
		m_wndLabel.SetFont(&theApp.m_boldFont);

		// Create the edit box
		m_edit.CreateEx(
			WS_EX_CLIENTEDGE|WS_EX_NOPARENTNOTIFY, _T("EDIT"), NULL, 
			WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_TABSTOP|WS_GROUP|ES_LEFT|ES_MULTILINE|ES_WANTRETURN, 
			CRect(0, 0, 300, 300), this, EMR_TEXT_MACRO_EDIT_IDC);
		// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
		m_edit.SetFont(CFont::FromHandle(EmrFonts::GetRegularFont()));
		m_edit.SetLimitText(2000);
		m_edit.SetFocus();
	
	}
	NxCatchAll("Error in CEmrTextMacroBoxDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEmrTextMacroBoxDlg::OnSize(UINT nType, int cx, int cy) 
{
	// (c.haag 2008-06-06 09:43) - Almost all copied from CEmrItemAdvTextDlg::RepositionControls
	try {
		CNxDialog::OnSize(nType, cx, cy);
		RepositionControls();
	}
	NxCatchAll("Error in CEmrTextMacroBoxDlg::OnSize");
}

HBRUSH CEmrTextMacroBoxDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	// (c.haag 2008-06-06 09:57) - Not adding try-catches; otherwise user may get hung
	// up in an endless loop of dialogs since this is drawing-related code. I also copied
	// this code from EmrItemAdvDlg.cpp
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if(nCtlColor == CTLCOLOR_DLG && pWnd->GetSafeHwnd() == m_hWnd) {
		pDC->SetBkColor(GetBackgroundColor());
		return GetBackgroundBrush();
	}
	else if(nCtlColor == CTLCOLOR_STATIC && pWnd->GetSafeHwnd() == m_wndLabel.GetSafeHwnd()) {
		pDC->SetBkColor(GetBackgroundColor());
		return GetBackgroundBrush();
	}

	return hbr;
}

COLORREF CEmrTextMacroBoxDlg::GetBackgroundColor()
{
	return GetNxColor(GNC_EMR_ITEM_BG, 0);
}

HBRUSH CEmrTextMacroBoxDlg::GetBackgroundBrush()
{
	return (HBRUSH)m_brMacroBackground;
}

void CEmrTextMacroBoxDlg::SetLabelText(const CString& strLabel)
{
	if (IsWindow(m_wndLabel.GetSafeHwnd())) {
		m_wndLabel.SetWindowText(strLabel);
		RepositionControls();
		// (c.haag 2008-06-17 09:48) - PLID 26038 - We have to manually redraw the window
		// because, at least on my computer, the text is sometimes shown fragmented. For 
		// example, if I change the label from "ewewrereewrewr" to "This is a test macro",
		// then the result looks like "Th  s  test macro" on my screen. Forcing the label 
		// to redraw seems to work.
		m_wndLabel.RedrawWindow();
	} else {
		ASSERT(FALSE);
	}
}

CString CEmrTextMacroBoxDlg::GetTextData()
{
	if (IsWindow(m_edit.GetSafeHwnd())) {
		CString strResult;
		m_edit.GetWindowText(strResult);
		return strResult;
	} else {
		ASSERT(FALSE);
		return "";
	}
}

void CEmrTextMacroBoxDlg::SetTextData(const CString& strTextData)
{
	if (IsWindow(m_edit.GetSafeHwnd())) {
		m_edit.SetWindowText(strTextData);
	} else {
		ASSERT(FALSE);
	}
}

void CEmrTextMacroBoxDlg::OnSpellCheck()
{
	// (c.haag 2008-06-06 12:43) - Copied from CEmrItemAdvTextDlg::OnSpellCheck
	try {
		CWaitCursor wc;

		// Ask that owner for its spell checking mechanism
		WSPELLLib::_DWSpellPtr pwsSpellChecker = CreateSpellExObject(this);
		if (pwsSpellChecker) {
			// Tell the spell checker to look at our text box
			pwsSpellChecker->PutTextControlHWnd((long)m_edit.m_hWnd);
			pwsSpellChecker->ClearTempDictionary();
			// Do the spell check and respond to the result
			short nAns = pwsSpellChecker->Start();
			if (nAns == 0) {
				// Success
				MessageBox("Spell check completed successfully.", NULL, MB_OK|MB_ICONINFORMATION);
			} else if (nAns == -17) {
				// User canceled
			} else {
				// An error
				CString strErr;
				strErr.Format("ERROR %hi was reported while trying to check spelling!", nAns);
				MessageBox(strErr, NULL, MB_OK|MB_ICONEXCLAMATION);
			}
			// Put focus back inside the text box where it belongs
			m_edit.SetFocus();
		} else {
			MessageBox("The spell check could not begin because the necessary components were not loaded.  Please verify that Practice is installed correctly and try again.", NULL, MB_OK|MB_ICONEXCLAMATION);
		}
	} NxCatchAll("CEmrTextMacroBoxDlg::OnSpellCheck");
}

void CEmrTextMacroBoxDlg::OnChangeEditTextData()
{
	try {
		CString str;
		m_edit.GetWindowText(str);
		GetParent()->SendMessage(NXM_EMR_TEXT_MACRO_BOX_EDIT_CHANGE, (WPARAM)str.AllocSysString(), (LPARAM)str.GetLength());
	}
	NxCatchAll("Error in CEmrTextMacroBoxDlg::OnChangeEditTextData");
}

BOOL CEmrTextMacroBoxDlg::OnEraseBkgnd(CDC* pDC) 
{
	CRect rcClient;
	GetClientRect(rcClient);
	pDC->FillRect(rcClient, const_cast<CBrush*>(&m_brMacroBackground));
	return TRUE;
}
BOOL CEmrTextMacroBoxDlg::PreTranslateMessage(MSG* pMsg) 
{
	// (c.haag 2008-06-16 09:36) - Override tab ordering to span the parent dialog
	const BOOL bIsShiftKeyDown = (GetAsyncKeyState(VK_SHIFT) & 0x80000000);
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB) {
		if (!bIsShiftKeyDown && EMR_TEXT_MACRO_EDIT_IDC == GetFocus()->GetDlgCtrlID()) {
			CWnd* pWnd = GetParent()->GetDlgItem(IDC_EMR_TEXT_MACRO_LIST);
			if (NULL != pWnd) {
				pWnd->SetFocus();
				return TRUE;
			}
		}
		else if (bIsShiftKeyDown && EMR_TEXT_MACRO_EDIT_IDC == GetFocus()->GetDlgCtrlID()) {
			CWnd* pWnd = GetParent()->GetDlgItem(IDC_EDIT_DETAIL_NAME);
			if (NULL != pWnd) {
				pWnd->SetFocus();
				return TRUE;
			}
		}
	}
	return CNxDialog::PreTranslateMessage(pMsg);
}

void CEmrTextMacroBoxDlg::OnDestroy() 
{
	try {
		CNxDialog::OnDestroy();
		
		if (NULL != m_pButtonFont) {
			delete m_pButtonFont;
		}
	}
	NxCatchAll("Error in CEmrTextMacroBoxDlg::OnDestroy");
}
