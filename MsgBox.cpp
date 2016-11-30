// MsgBox.cpp : implementation file
//

#include "stdafx.h"
#include "MsgBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMsgBox dialog


CMsgBox::CMsgBox(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMsgBox::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMsgBox)
	//}}AFX_DATA_INIT

	m_bAllowCancel = FALSE;
	//Only a few places actually use this in release mode. Default to something not message box for the window title
	#ifdef _DEBUG 
		m_strWindowText = "Message Box";
	#else
		m_strWindowText = "Practice";
	#endif
		// (r.gonet 05/01/2014) - PLID 49432 - Default to word wrapping the text
		m_bWordWrap = true;
		// (r.gonet 05/01/2014) - PLID 49432 - Default to using the default font.
		m_pFont = NULL;
		// (r.gonet 05/16/2014) - PLID 49432 - Need to retain the dynamic allocation to prevent memory leaks.
		m_pDynamicEdit = NULL;
}

CMsgBox::~CMsgBox()
{
	// (r.gonet 05/16/2014) - PLID 49432 - Free resources if we allocated the edit box dynamically.
	if (m_pDynamicEdit != NULL) {
		m_pDynamicEdit->DestroyWindow();
		delete m_pDynamicEdit;
		m_pDynamicEdit = NULL;
	}

}

// (a.walling 2009-04-20 15:06) - PLID 33951 - Changed from CNxEdit back to CEdit, we were not using any of NxEdit's
// functionality, and it was just causing flickering. Also added VSCROLL since we seem to only use this form of
// messagebox for big messages or in debug mode.
void CMsgBox::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMsgBox)
	//DDX_Control(pDX, IDC_MSGBOX, m_nxeditMsgbox);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMsgBox, CNxDialog)
	//{{AFX_MSG_MAP(CMsgBox)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

WNDPROC g_pEditBoxWindowProc = nullptr;
LRESULT CALLBACK EditBoxWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Handle VM_CHAR instead of WM_KEYDOWN to avoid a beep for unhandled WM_CHAR.
	// Apparently holding down Ctrl while pressing an alphabetic character will 
	// generate a keycode equal to the value of the uppercase version of that 
	// character minus 64. So in this case it's 'A'-64 = 1. Going to check CTRL state
	// anyway though because 0x01 should be VM_LBUTTON normally.
	// MSDN: "Some CTRL key combinations are translated into ASCII control characters. 
	// For example, CTRL+A is translated to the ASCII ctrl-A (SOH) character (ASCII value 0x01)."
	if (msg == WM_CHAR && GetKeyState(VK_CONTROL) & 0x8000 && wParam == ('A' - 64))
	{ 
		SendMessage(hwnd, EM_SETSEL, 0, -1); 
		return 1; 
	}
	else
	{
		return CallWindowProc(g_pEditBoxWindowProc, hwnd, msg, wParam, lParam);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMsgBox message handlers

BOOL CMsgBox::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	GetDlgItem(IDCANCEL)->ShowWindow(m_bAllowCancel);
	
	CEdit *pEditBox = static_cast<CEdit *>(GetDlgItem(IDC_MSGBOX));
	if(!m_bWordWrap) {
		// (r.gonet 05/01/2014) - PLID 49432 - If word wrap is off, then we have to
		// recreate the edit box because there is no way to do disable word wrap after
		// the edit box is created.
		DWORD dwStyle = pEditBox->GetStyle();
		CRect rcEditBox;
		pEditBox->GetRect(&rcEditBox);
		pEditBox->ClientToScreen(&rcEditBox);
		this->ScreenToClient(&rcEditBox);
		CFont *pFont = pEditBox->GetFont();
		pEditBox->DestroyWindow();
		pEditBox = new CEdit();
		dwStyle |= ES_AUTOHSCROLL|WS_HSCROLL;
		pEditBox->Create(dwStyle, rcEditBox, this, IDC_MSGBOX);
		pEditBox->SetFont(pFont);
		// (r.gonet 05/16/2014) - PLID 49432 - We now have to clean it up ourselves.
		m_pDynamicEdit = pEditBox;
	}
	if(m_pFont != NULL) {
		// (r.gonet 05/01/2014) - PLID 49432 - Use whatever font the caller told us to use.
		pEditBox->SetFont(m_pFont);
	}

	// Have the edit box handle CTRL+A since that's most often what we want to do.
	if (!(g_pEditBoxWindowProc = (WNDPROC)SetWindowLongPtr(pEditBox->GetSafeHwnd(), GWL_WNDPROC, (LONG)&EditBoxWindowProc)))
	{
		// Failed, but that just means CTRL+A won't be handled.
	}
	else
	{
		// Edit box's WndProc now overridden.
	}
	
	CString strMsgWithStandardizedLineEndings(msg);
	strMsgWithStandardizedLineEndings.Replace("\r\n", "\n");
	strMsgWithStandardizedLineEndings.Replace("\n", "\r\n");
	pEditBox->SetWindowText(strMsgWithStandardizedLineEndings);

	// (e.lally 2009-06-04) Set the window title text
	SetWindowText(m_strWindowText);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CMsgBox::OnCancel()
{
	if(m_bAllowCancel ==FALSE){
		CDialog::OnOK();
		return;
	}
	CDialog::OnCancel();
}

HBRUSH CMsgBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_MSGBOX:
		{
			// (z.manning, 05/16/2008) - PLID 30050 - make borderless edit controls transparent
			pDC->SetBkColor(GetSolidBackgroundColor());
			return m_brBackground;
		}
		break;
	}

	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}