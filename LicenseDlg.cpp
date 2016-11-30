// License.cpp : implementation file
//

#include "stdafx.h"
#include <NxPracticeSharedLib/RichEditUtils.h>
#include "LicenseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (b.cardillo 2004-01-15 13:24) - I don't know why someone chose to define this using OCTAL!
#define ID_LICENSE 0013

/////////////////////////////////////////////////////////////////////////////
// CLicenseDlg dialog

CLicenseDlg::CLicenseDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLicenseDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLicenseDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bAllowAgreeButton = FALSE;
	m_bRequireAgreement = TRUE;
	//DRT 5/16/2008 - PLID 30089 - Default to the license agreement, since this dialog used to only support it.
	m_nLicenseResourceToUse = IDR_LICENSEAGREEMENT_RTF;
}

void CLicenseDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLicenseDlg)
	DDX_Control(pDX, IDC_PRINT_BTN, m_btnPrint);
	DDX_Control(pDX, IDIGNORE, m_btnIgnore);
	DDX_Control(pDX, IDC_AGREEMENT_STATIC, m_nxstaticAgreementStatic);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLicenseDlg, CNxDialog)
	//{{AFX_MSG_MAP(CLicenseDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_VSCROLL(ID_LICENSE, OnLicenseScroll)
	ON_NOTIFY(EN_MSGFILTER, ID_LICENSE, OnLicenseMsgFilter)
	ON_COMMAND(IDIGNORE, OnIgnoreBtn)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLicenseDlg message handlers

BOOL CLicenseDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	GetDlgItem(IDC_CHECK)->ShowWindow(SW_HIDE);	//BVB - sorry to put this here, 
												//wanted to share this resource with CChangesDlg
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


	// Create and fill the rich text edit box
	try {
		
		m_btnPrint.AutoSet(NXB_PRINT);
		m_btnIgnore.AutoSet(NXB_CLOSE);

		if (AfxInitRichEdit()) {

			// Decide where to put it
			CRect rect;
			GetDlgItem(IDC_AGREEMENT_STATIC)->GetWindowRect(&rect);
			ScreenToClient(&rect);

			//TES 11/7/2007 - PLID 27981 - VS2008 - VS 2008 has different parameters for CRichEdit::CreateEx().
#if _MSC_VER > 1300
			//.NET visual studio
			// Create the control with a slightly sunken border (static edge, not standard client edge)
			// (a.walling 2008-10-13 11:25) - PLID 28111 - The newer rich edit control will not wrap anything if WS_HSCROLL
			if (m_RichEdit.CreateEx(WS_EX_STATICEDGE,
				WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL | WS_TABSTOP | WS_CHILD ,
				rect, this, ID_LICENSE)) 
#else
			//VC6
			// Create the control with a slightly sunken border (static edge, not standard client edge)
			if (m_RichEdit.CreateEx(WS_EX_STATICEDGE, _T("RICHEDIT"), NULL,
				WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL | WS_HSCROLL | WS_TABSTOP | WS_CHILD ,
				rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
				GetSafeHwnd(), (HMENU)ID_LICENSE, (LPVOID)NULL)) 
#endif //_MSC_VER > 1300
			{
				// Make sure we're going to get scrolling notifications
				m_RichEdit.SetEventMask(m_RichEdit.GetEventMask() | ENM_SCROLL | ENM_SCROLLEVENTS);

				//DRT 11/28/2007 - PLID 28225 - No limit on the text.  Our newest license agreement didn't fit into
				//	whatever the default is.
				m_RichEdit.LimitText(0);

				// Set the text from our resource
				//DRT 5/16/2008 - PLID 30089 - We now also have an Allergan beta NDA license agreement that works
				//	100% identically.  I just added a flag here.
				SetRtfTextFromResource(&m_RichEdit, NULL, m_nLicenseResourceToUse, "RTF", SF_RTF);
				
				// Zoom out a little bit because the word document generally has the text larger since it's going to 
				// print out on paper.  This way we just have to take the live LicenseAgreement template and save it 
				// as an .rtf file in our resources (after stripping out all template fields and headers/footers and 
				// adding any addendums (like the NexForms addendum) to the document).
				m_RichEdit.SendMessage(EM_SETZOOM, 5, 7);
			} else {
				ThrowNxException("CLicenseDlg::OnInitDialog: Could not create rich edit!");
			}
		} else { 
			ThrowNxException("CLicenseDlg::OnInitDialog: Could not initialize rich edit!");
		}
	} NxCatchAllCall("CLicenseDlg::OnInitDialog", {
		CDialog::OnCancel();
		return FALSE;
	});

	// If we're not requiring agreement, don't show the "agree/disagree" buttons, just show the "dismiss" button
	GetDlgItem(IDOK)->ShowWindow(m_bRequireAgreement ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDCANCEL)->ShowWindow(m_bRequireAgreement ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDIGNORE)->ShowWindow(m_bRequireAgreement ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDOK)->EnableWindow(m_bRequireAgreement);
	GetDlgItem(IDIGNORE)->EnableWindow(!m_bRequireAgreement);
	SetDefID(m_bRequireAgreement ? IDOK : IDIGNORE);

	m_RichEdit.SetFocus();

	// (z.manning 2008-08-28 15:06) - PLID 31199 - This used to be a second, but I changed it to 10 ms
	// in case there's no scroll bar so that it doesn't take long to determine if they're allowed to accept.
	SetTimer(1, 10, NULL);

	return FALSE;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLicenseDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLicenseDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CLicenseDlg::IsLicenseScrolledToEnd()
{
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	// (z.manning 2008-08-28 15:35) - PLID 31199 - Note: When I initially implemented this, GetScrollInfo
	// was returning true even if there was no vertical scrollbar. However, this was not the case when
	// c.haag tested this (I haven't provent it yet, but I'm sure he set me up somehow), so regardless
	// of the return value of GetScrollInfo, we handle the case where there's no scrollbar.
	if (m_RichEdit.GetScrollInfo(SB_VERT, &si)) {
		int nMaxScrollPos = si.nMax - (int)si.nPage;
		// (z.manning 2008-08-28 14:48) - PLID 31199 - Make sure we return success if we don't have 
		// a scroll bar.
		if ( si.nPos >= nMaxScrollPos || si.nTrackPos >= nMaxScrollPos || si.nPage == 0 || si.nMax < (int)si.nPage) {
			// Yes, it's scrolled to the bottom
			return TRUE;
		} 
		else {
			// Not scrolled to the bottom
			return FALSE;
		}
	} 
	else {
		// Couldn't get scroll info
		// (z.manning 2008-08-28 15:30) - PLID 31199 - They can't scroll if there's no scrollbar
		DWORD dwError = GetLastError();
		if(dwError == ERROR_NO_SCROLLBARS) {
			return TRUE;
		}
		else {
			// (z.manning 2008-08-28 15:34) - PLID 31199 - If GetScrollInfo fails for any other reason
			// we should investigate and handle it.
			ASSERT(FALSE);
			return FALSE;
		}
	}
}

void CLicenseDlg::OnLicenseScroll()
{
	if (IsLicenseScrolledToEnd()) {
		m_bAllowAgreeButton = TRUE;
	}
}

void CLicenseDlg::OnOK()
{
	if (m_bAllowAgreeButton) {
		CDialog::OnOK();
	} else {
		MessageBox(
			"Please read the entire text before you accept the license agreement.  You "
			"may need to scroll down to be able to see the whole agreement.", NULL, 
			MB_ICONEXCLAMATION|MB_OK);
		if (IsKeyDown(VK_SHIFT)) {
			// If the user is holding down shift when dismissing this message, we silently accept it as an 
			// override (this is a failsafe in case the rich edit scrolling messages don't work properly).
			CDialog::OnOK();
			return;
		}
		m_RichEdit.SetFocus();
	}
}

void CLicenseDlg::OnCancel()
{
	CDialog::OnCancel();

}

void CLicenseDlg::OnIgnoreBtn()
{
	EndDialog(IDIGNORE);
}

//TES 11/7/2007 - PLID 28024 - VS2008 - Our first parameter must be a NMHDR*.
void CLicenseDlg::OnLicenseMsgFilter(NMHDR *pNMHDR, LRESULT *pResult)
{
	//TES 11/7/2007 - PLID 28024 - VS2008 - Because this is handling the EN_MSGFILTER notification, pNMHDR will
	// be pointing to a MSGFILTER object.
	MSGFILTER *pMsgFilter = (MSGFILTER*)pNMHDR;
	// Make sure this is a scroll message
	if (pMsgFilter->msg == WM_VSCROLL) {
		// Now treat it as such
		OnLicenseScroll();
	}
}

void CLicenseDlg::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == 1) {
		// Since NT doesn't seem to be giving us vscroll messages for the rich text, we have this timer 
		// backup mechanism that ensures that if we're scrolled to the bottom for more than 1 second, we 
		// are then allowed to click OK.
		OnLicenseScroll();

		// (z.manning 2008-08-28 15:06) - PLID 31199 - The timer is initially set at 10 ms now for the case
		// when there's no scroll bar so that the first check happens quickly. Let's set it at 1 sec from now
		// on to preserve pre-existing functionality.
		KillTimer(1);
		SetTimer(1, 1000, NULL);
	}
	
	CDialog::OnTimer(nIDEvent);
}
