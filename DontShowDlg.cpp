// DontShowDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "DontShowDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//DRT 6/9/2008 - PLID 30331 - Added cancel option
int DontShowMeAgain(CWnd* pParent, const CString &strText, const CString &strName, const CString &strTitle /*= ""*/, BOOL bIgnoreUser /*= FALSE*/, BOOL bUseYesNo  /*=FALSE*/, BOOL bUseCancelWithYesNo /*= FALSE*/)
{
	CDontShowDlg dlg(pParent);
	if (bIgnoreUser) {
		// (r.gonet 2016-05-19 18:13) - NX-100689 - Get the system name from the property 
		//manager rather than the license object.
		dlg.m_strCurUser = g_propManager.GetSystemName();
	}

	return dlg.DoModal(strText, strName, strTitle, bUseYesNo, bUseCancelWithYesNo);
}

// (a.walling 2007-02-26 15:57) - PLID 24421 - DontShowMeAgains need to support a parent window
//	so they can be used in modeless dialogs.

//TES 8/12/2009 - PLID 35201 - Added an overload that returns a BOOL for whether the "Don't Show Me Again" box was
// checked, rather than a property name.
int DontShowMeAgain(CWnd* pParent, const CString &strText, OUT BOOL &bDontShowChecked, const CString &strTitle /*= ""*/, BOOL bIgnoreUser /*= FALSE*/, BOOL bUseYesNo  /*=FALSE*/)
{
	CDontShowDlg dlg(pParent);
	if (bIgnoreUser) {
		// (r.gonet 2016-05-19 18:13) - NX-100689 - Get the system name from the property 
		//manager rather than the license object.
		dlg.m_strCurUser = g_propManager.GetSystemName();
	}

	return dlg.DoModal(strText, bDontShowChecked, strTitle, bUseYesNo);
}

int DontShowMeAgainByFileDate(CWnd* pParent, const CString &strText, const CString &strName, const CString &strFilePath, const CString &strTitle /*= ""*/, BOOL bIgnoreUser /*= FALSE*/, BOOL bUseYesNo /*= FALSE*/)
{
	// (j.armen 2011-10-24 17:54) - PLID 46132 - GetPracPath appears to need ConfigRT.  It appears that currently this code is never called however
	CDontShowDlg dlg(pParent);
	if (bIgnoreUser) {
		// (r.gonet 2016-05-19 18:13) - NX-100689 - Get the system name from the property 
		//manager rather than the license object.
		dlg.m_strCurUser = g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT);
	}
	dlg.m_strByFileDate = strFilePath;
	return dlg.DoModal(strText, strName, strTitle, bUseYesNo);
}

/////////////////////////////////////////////////////////////////////////////
// CDontShowDlg dialog


CDontShowDlg::CDontShowDlg(CWnd* pParent /*=NULL*/)
	: CSizeableTextDlg(CDontShowDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDontShowDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_strByFileDate.Empty();
	m_bIgnoreDontShow = FALSE;
}

void CDontShowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDontShowDlg)
	DDX_Control(pDX, IDC_DONTSHOW_CHECK, m_btnDontShow);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDYES, m_btnYes);
	DDX_Control(pDX, IDNO, m_btnNo);
	DDX_Control(pDX, IDC_TEXT_EDIT, m_nxstaticTextEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDontShowDlg, CSizeableTextDlg)
	//{{AFX_MSG_MAP(CDontShowDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDontShowDlg message handlers

BOOL CDontShowDlg::OnInitDialog()
{
	// First (and very importantly) call the base class
	BOOL bAns = CSizeableTextDlg::OnInitDialog();

	m_btnOk.AutoSet(NXB_OK);
	//DRT 6/9/2008 - PLID 30331 - Added cancel button
	m_btnCancel.AutoSet(NXB_CANCEL);

	// (b.cardillo 2005-03-07 18:09) - PLID 15477 - Size ourselves to fit the text as snugly as possible
	{
		// Get our four margins for the text area (current distance from the 
		// four edges of the text to the corresponding four edges of the window)
		CRect rcMargins;
		{
			CRect rcWindow;
			GetWindowRect(&rcWindow);
			ScreenToClient(&rcWindow);
			CRect rcTextEdit;
			GetDlgItem(IDC_TEXT_EDIT)->GetWindowRect(rcTextEdit);
			ScreenToClient(rcTextEdit);
			rcMargins.left = rcTextEdit.left - rcWindow.left;
			rcMargins.top = rcTextEdit.top - rcWindow.top;
			rcMargins.right = rcWindow.right - rcTextEdit.right;
			rcMargins.bottom = rcWindow.bottom - rcTextEdit.bottom;
		}
		
		// Assume our window's max size is 3/4 the size of the work area
		CRect rcMaxWindow;
		CRect rcDesktop;
		{
			// Get the full desktop work area (that's the screen minus any docked toolbars like the taskbar)
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);
			// Reduce it by 3/8
			long nReduceWidth = (rcDesktop.Width())*3/8; // reduce by 3/8
			long nReduceHeight = (rcDesktop.Height())*3/8; // reduce by 3/8
			rcMaxWindow.SetRect(0, 0, rcDesktop.Width()-nReduceWidth + GetSystemMetrics(SM_CXBORDER), rcDesktop.Height()-nReduceHeight + GetSystemMetrics(SM_CYBORDER));
		}

		// That's our max window size, now reduce that to get our max area in which the text can be placed
		CRect rcMaxTextArea(rcMaxWindow);
		rcMaxTextArea.DeflateRect(rcMargins);

		// Now we'll reduce that even more by calculating how the text will look
		{
			CDC *pdc = GetDC();
			CFont *pDlgFont = GetFont();
			CFont *pf = pdc->SelectObject(pDlgFont);
			// For the CDontShowDlg we know we're hard coded to use tsFromString, not 
			// tsFromFile, so we can just pull the string directly from m_strTextOrPath.
			ASSERT((m_nStyle & tsFromFile) == 0);
			CString strText = m_strTextOrPath;
			// Trim it, because like the regular MessageBox, if there are trailing carriage 
			// returns we want to ignore them in our measurements.
			strText.TrimRight();
			if (pdc->DrawText(strText, &rcMaxTextArea, DT_TOP|DT_CENTER|DT_VCENTER|DT_WORDBREAK|DT_EXPANDTABS|DT_EXTERNALLEADING|DT_CALCRECT)) {
				// Got the height of that text assuming the width was ok, but there's a special case if it 
				// would be so tall it would go off the screen.  In that case, recalculate the rect allowing 
				// it to take up the whole width of the desktop.
				if (rcMaxTextArea.Height() > rcDesktop.Height()) {
					rcMaxTextArea.right = rcMaxTextArea.left + rcDesktop.Width();
					pdc->DrawText(strText, &rcMaxTextArea, DT_TOP|DT_CENTER|DT_VCENTER|DT_WORDBREAK|DT_EXPANDTABS|DT_EXTERNALLEADING|DT_CALCRECT);
				}
			} else {
				rcMaxTextArea.right = rcMaxTextArea.left;
				rcMaxTextArea.bottom = rcMaxTextArea.top;
			}
			pdc->SelectObject(pf);
			ReleaseDC(pdc);
		}

		// Now expand that back out to get the full window size, with margins
		CRect rcActualWindow(rcMaxTextArea);
		rcActualWindow.InflateRect(rcMargins);

		// (z.manning 2008-12-01 11:42) - PLID 32182 - Make sure the dialog is wide enough such that
		// the button(s) don't get cut off.
		int nMinWidth;
		if(m_bUseYesNo) {
			CRect rcYes, rcNo;
			m_btnYes.GetWindowRect(rcYes);
			m_btnNo.GetWindowRect(rcNo);
			nMinWidth = rcNo.right - rcYes.left + 20;
		}
		else {
			CRect rcOk;
			m_btnOk.GetWindowRect(rcOk);
			nMinWidth = rcOk.Width() + 10;
		}

		// Now we know the ideal width and height of the dialog, so set it (we rely on 
		// CSizeableTextDlg to realign everything inside the dialog, even the edit box 
		// itself, based on this new dialog size).  Notice, MFC will still center us 
		// later on when we become visible, and it will base it on this new sizing.
		SetWindowPos(NULL, 0, 0, rcActualWindow.Width() >= nMinWidth ? rcActualWindow.Width() : nMinWidth, rcActualWindow.Height(), SWP_NOMOVE|SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
	}

	return bAns;
}

//DRT 6/9/2008 - PLID 30331 - Added cancel option with YesNo
int CDontShowDlg::DoModal(const CString &strText, const CString &strName, const CString &strTitle /*= ""*/, const BOOL bUseYesNo /*=FALSE*/, BOOL bUseCancelWithYesNo /*= FALSE*/)
{
	// TODO: I don't like this because it deviates from the way CSizeableTextDlg-derived classes are supposed to work.  Ideally we wouldn't have our own DoModal().

	// For this class we do our Init here
	m_strTextOrPath = strText;
	m_strName = strName;
	m_strTitle = strTitle;
	m_bUseYesNo = bUseYesNo;
	m_bUseCancelWithYesNo = bUseCancelWithYesNo;

	return CSizeableTextDlg::DoModal();
}

//TES 8/12/2009 - PLID 35201 - Added an overload that returns a BOOL for whether the "Don't Show Me Again" box was
// checked, rather than a property name.
int CDontShowDlg::DoModal(const CString &strText, OUT BOOL &bDontShowChecked, const CString &strTitle /*= ""*/, const BOOL bUseYesNo /*=FALSE*/, BOOL bUseCancelWithYesNo /*= FALSE*/)
{
	// TODO: I don't like this because it deviates from the way CSizeableTextDlg-derived classes are supposed to work.  Ideally we wouldn't have our own DoModal().

	// For this class we do our Init here
	m_strTextOrPath = strText;
	//TES 8/12/2009 - PLID 35201 - With this overload, the caller is responsible for handling the "Don't Show Me Again" box.
	m_bIgnoreDontShow = TRUE;
	m_strTitle = strTitle;
	m_bUseYesNo = bUseYesNo;
	m_bUseCancelWithYesNo = bUseCancelWithYesNo;

	int nReturn = CSizeableTextDlg::DoModal();
	//TES 8/12/2009 - PLID 35201 - Return the status of the "Don't Show Me Again" checkbox.
	bDontShowChecked = m_bDontShowChecked;
	return nReturn;
}

BOOL CDontShowDlg::Init()
{
	if (CSizeableTextDlg::Init()) {
		m_nStyle = CSizeableTextDlg::msMsgBoxText|CSizeableTextDlg::tsFromString;
		//TES 8/12/2009 - PLID 35201 - Check whether the caller is handling the "Don't Show Me Again" functionality.
		if(m_bIgnoreDontShow) {
			m_nStyle |= CSizeableTextDlg::drsIgnoreDontRemind;
		}
		else {
			if (m_strByFileDate.IsEmpty()) {
				// Just normal boolean
				m_nStyle |= CSizeableTextDlg::drsDontRemindBool;
				
			} else {
				// By filedate means we want to do this in drsDontRemindUntil mode
				m_nStyle |= CSizeableTextDlg::drsDontRemindUntil;
				
				// Default to NOW in case the GetFileSystemTime fails (if it fails, then both the times will be NOW and the dialog will be displayed)
				COleDateTime dtFileTime = COleDateTime::GetCurrentTime();
				// Get the file modified time
				GetFileSystemTime(m_strByFileDate, dtFileTime);
				// Set both the NextTime and the CurrentTime equal to the file modified time so that 
				// if they choose not to see it again, they won't until the file date is changed
				m_dtNextTime = dtFileTime;
				m_dtOverrideCurrentTime = dtFileTime;
			}
		}

		//see whether they want the yes no or just the Ok
		if (m_bUseYesNo) {
			m_nStyle |= CSizeableTextDlg::bsYesNoOnly;
		}
		else {
			m_nStyle |= CSizeableTextDlg::bsOKOnly;
		}

		//DRT 6/9/2008 - PLID 30331 - Added Cancel option
		if(m_bUseCancelWithYesNo) {
			m_nStyle |= CSizeableTextDlg::bsYesNoCancel;
		}
		else {
		}

		return TRUE;
	} else {
		return FALSE;
	}
}

