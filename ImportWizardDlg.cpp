// ImportWizardDlg.cpp: implementation of the ImportWizardDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxStandard.h"
#include "ImportWizardDlg.h"
#include "ImportWizardTypeDlg.h"
#include "ImportWizardFieldsDlg.h"
#include "ImportWizardPreviewDlg.h"
#include "Globalutils.h"
// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace ADODB;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImportWizardDlg::CImportWizardDlg()
{
	//Construct("New Stored Import");
	SetWizardMode();
	
	m_irtRecordType = irtPatients;

	m_strFieldSeparator = ",";

	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	m_bNeedInit = TRUE;
	m_nMinCX = 0;
	m_nMinCY = 0;
}

// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
BOOL CImportWizardDlg::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	
	try
	{
		CRect rc;
		GetClientRect(&rc);
		m_ClientSize = rc.Size();

		// Init m_nMinCX/Y
		m_nMinCX = m_ClientSize.cx;
		m_nMinCY = m_ClientSize.cy;

		//retreiving the right and bottom padding so as to resize pages correctly later
		{
			CRect rc;
			CWnd *pPage = GetActivePage();

			ASSERT(pPage);

			if (pPage){
				pPage->GetWindowRect(&rc);
				ScreenToClient(&rc);
				m_pageRightPadding = m_ClientSize.cx - rc.right;
				m_pageBottomPadding = m_ClientSize.cy - rc.bottom;
			}
		}

		// After this point, the resize code runs.
		m_bNeedInit = FALSE;
	}
	NxCatchAll(__FUNCTION__);

	return bResult;
}

CImportWizardDlg::~CImportWizardDlg()
{
	if(m_pProgressDlg != NULL)
		delete m_pProgressDlg;
}

// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
// https://support.microsoft.com/en-us/kb/325613
// This function must be a STATIC method. 
// Callback to allow you to set the default window styles 
// for the property sheet.
int CALLBACK CImportWizardDlg::XmnPropSheetCallback(HWND hWnd, UINT message, LPARAM lParam)
{
	//It seems as though this doens't really do anything, but it's in the microsoft example
	//so we'll leave it for now
	extern int CALLBACK AfxPropSheetCallback(HWND, UINT message, LPARAM lParam);
	// XMN: Call MFC's callback.
	int nRes = AfxPropSheetCallback(hWnd, message, lParam);

	switch (message)
	{
	case PSCB_PRECREATE:
		// Set your own window styles.
		((LPDLGTEMPLATE)lParam)->style |= (
			WS_CLIPSIBLINGS
			| WS_CLIPCHILDREN
			| WS_MAXIMIZEBOX
			| DS_3DLOOK 
			| DS_SETFONT
			| WS_THICKFRAME 
			| WS_SYSMENU 
			| WS_POPUP 
			| WS_VISIBLE 
			| WS_CAPTION);
		return TRUE;
	}
	return nRes;
}

int CImportWizardDlg::DoModal()
{
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	// Hook into property sheet creation code
	m_psh.dwFlags |= PSH_USECALLBACK;
	m_psh.pfnCallback = XmnPropSheetCallback;

	//(e.lally 2007-06-29) PLID 26508 - Use a progress bar for larger files
	m_pProgressDlg = NULL;

	
	CImportWizardTypeDlg dlgType;
	AddPage(&dlgType);
	CImportWizardFieldsDlg dlgFields;
	AddPage(&dlgFields);
	CImportWizardPreviewDlg dlgPreview;
	AddPage(&dlgPreview);

	return CPropertySheet::DoModal();
}

// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
BEGIN_MESSAGE_MAP(CImportWizardDlg, CPropertySheet)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
void CImportWizardDlg::OnSize(UINT nType, int cx, int cy)
{
	CPropertySheet::OnSize(nType, cx, cy);

	if (m_bNeedInit)
		return;

	SetRedraw(FALSE);
	try{
		//Get the delta in size
		int dx = cx - m_ClientSize.cx;
		int dy = cy - m_ClientSize.cy;

		//Remeber the new size
		m_ClientSize.cx = cx;
		m_ClientSize.cy = cy;

		//Adjust buttons to bottom right
		//Adjust the line to grow and size bottom right
		for (CWnd *pChild = GetWindow(GW_CHILD); pChild != NULL; pChild = pChild->GetWindow(GW_HWNDNEXT))
		{
			LRESULT winType = pChild->SendMessage(WM_GETDLGCODE);
			if (winType & DLGC_BUTTON)
			{
				ChangeDlgItemPos(this, pChild->GetDlgCtrlID(), dx, dy, EChangeDlgItemPosAnchor::RightBottom);
			}
			else if (winType & DLGC_STATIC)
			{
				ChangeDlgItemPos(this, pChild->GetDlgCtrlID(), dx, dy, EChangeDlgItemPosAnchor::RightBottomLeft);
			}
		}

		//Resize the active page
		ResizePage(GetActivePage());

		SetRedraw(TRUE);
		RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASENOW | RDW_UPDATENOW);
	}NxCatchAllSilentCall(SetRedraw(TRUE););
}

// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
void CImportWizardDlg::ResizePage(CPropertyPage *pPage)
{
	if (m_bNeedInit)
		return;

	ASSERT(pPage->GetSafeHwnd());

	//Make sure we have a page to work with
	if (pPage->GetSafeHwnd())
	{
		CRect rc;

		//Get it's size, according the the client area of the propertysheet
		pPage->GetWindowRect(&rc);
		ScreenToClient(&rc);

		//Make adjustments to right and bottom, including the padding we calculated earlier
		rc.right = m_ClientSize.cx - m_pageRightPadding;
		rc.bottom = m_ClientSize.cy - m_pageBottomPadding;

		//Set the new size
		pPage->SetWindowPos(NULL,0, 0, rc.Width(), rc.Height(),
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
	}
}

// (j.politis 2015-04-30 14:00) - PLID 65524 - Allow the import wizard to be resizable
void CImportWizardDlg::OnPageSetActive(CPropertyPage *pPage)
{
	try{
		ResizePage(pPage);
	}NxCatchAll(__FUNCTION__);
}

// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
void CImportWizardDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CPropertySheet::OnGetMinMaxInfo(lpMMI);
	try{
		lpMMI->ptMinTrackSize.x = max(lpMMI->ptMinTrackSize.x, m_nMinCX);
		lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, m_nMinCY);
	}NxCatchAll(__FUNCTION__);
}