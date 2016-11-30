#include "stdafx.h"
#include "EmrEditorBase.h"
#include "EMN.h"

// (a.walling 2012-02-28 08:27) - PLID 48429 - EmrEditorBase abstraction for EmrEditor and EmrTemplateEditor

BEGIN_MESSAGE_MAP(CEmrEditorBase, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx)
END_MESSAGE_MAP()

#pragma TODO("Unrequest/RequestTableCheckerMessages?")

CEmrEditorBase::CEmrEditorBase()
{
}

int CEmrEditorBase::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
		
	CRect rTree;
	GetClientRect(&rTree);

	// (b.cardillo 2006-02-21 13:52) - PLID 19385 - Made this clip children instead of being 
	// transparent, because we don't want transparency, we just want it not to draw on top 
	// of its children.  But dropping the transparency exposed another problem with this 
	// window: that it doesn't do any drawing itself to begin with, because we gave the 
	// default mfc window class, which doesn't do background erasing.  So I fixed this by 
	// instead setting it to the Win32 window class "STATIC" so that it would do regular 
	// dialog-colored background erasing.
	// (z.manning, 05/15/2008) - PLID 30050 - I removed the afore mentioned "STATIC" because
	// we now manually erase the tree wnd's background.
	// (j.jones 2007-04-04 08:54) - PLID 25464 - SS_NOTIFY is needed so the treewnd will
	// get windows messages such as WM_MOUSEMOVE
	// (z.manning, 05/15/2008) - PLID 30050 - Set the background color of the tree wnd
	m_wndEmrTree.SetBackgroundColor(CNxDialog::GetSolidBackgroundColor());
	m_wndEmrTree.Create(NULL, "", WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|SS_NOTIFY, rTree, this, IDC_EMR_TREE);
	m_wndEmrTree.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
	m_wndEmrTree.ShowWindow(SW_SHOW);

	Initialize();

	// (j.jones 2006-02-02 15:38) - PLID 19123 - post a message to load the EMR,
	// which helps facilitate a more pleasant appearance while we wait

	PostMessage(NXM_LOAD_EMR);

	return 0;
}

BOOL CEmrEditorBase::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CEmrEditorBase::OnSize(UINT nType, int cx, int cy)
{
	try {
		CWnd::OnSize(nType, cx, cy);
			
		if (m_wndEmrTree.GetSafeHwnd()) {
			CRect rcClient;
			GetClientRect(&rcClient);
			m_wndEmrTree.MoveWindow(rcClient);
		}
	} NxCatchAll("Error in OnSize");
}

LRESULT CEmrEditorBase::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
		case NXM_LOAD_EMR:
			// (j.jones 2006-02-02 15:39) - PLID 19123 - load the object now, via posted message,
			// to facilitate a cleaner appearance while we wait
			LoadEMRObject();
			return 0;
			break;
	}

	return __super::WindowProc(message, wParam, lParam);
}


/////

// (j.jones 2012-10-11 17:58) - PLID 52820 - added bIsClosing, TRUE if the user picked Save & Close
EmrSaveStatus CEmrEditorBase::Save(BOOL bShowProgressBar /*= TRUE*/, BOOL bIsClosing /*= FALSE*/)
{
	//save it all!

	//the save transaction, auditing transaction, and rollbacks, are all handled in SaveEMR
	return m_wndEmrTree.SaveEMR(esotEMR, -1, bShowProgressBar, bIsClosing);
}

BOOL CEmrEditorBase::IsEmpty()
{
	return m_wndEmrTree.IsEmpty();
}

BOOL CEmrEditorBase::IsEMRUnsaved()
{
	return m_wndEmrTree.IsEMRUnsaved();
}

// (a.walling 2009-11-23 12:32) - PLID 36404 - Are we printing?
bool CEmrEditorBase::IsPrinting()
{
	if (::IsWindow(m_wndEmrTree.GetSafeHwnd())) {
		return m_wndEmrTree.IsPrinting();
	} else {
		return false;
	}
}

// (j.jones 2011-07-15 13:45) - PLID 42111 - takes in an image file name (could be a path),
// and returns TRUE if any Image detail on this EMR references it
BOOL CEmrEditorBase::IsImageFileInUseOnEMR(const CString strFileName)
{
	return m_wndEmrTree.IsImageFileInUseOnEMR(strFileName);
}


int CEmrEditorBase::GetEMNCount()
{
	return m_wndEmrTree.GetEMNCount();
}

CEMN* CEmrEditorBase::GetEMN(int nIndex)
{
	return m_wndEmrTree.GetEMN(nIndex);
}

LRESULT CEmrEditorBase::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		if(m_wndEmrTree) {
			m_wndEmrTree.OnTableChanged(wParam, lParam);
		}
	} NxCatchAll("Error in CEmrEditorBase::OnTableChanged"); 
	return 0;
}

//TES 8/13/2014 - PLID 63519 - Added support for EX tablecheckers
LRESULT CEmrEditorBase::OnTableChangedEx(WPARAM wParam, LPARAM lParam)
{
	try {
		//TES 8/13/2014 - PLID 63519 - Pass on to the EmrTreeWnd
		if (m_wndEmrTree) {
			m_wndEmrTree.OnTableChangedEx(wParam, lParam);
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (m.hancock 2006-11-28 11:10) - PLID 22302 - Returns a pointer containing m_pCurEMN.
CEMN* CEmrEditorBase::GetCurrentEMN()
{
	return GetActiveEMN();
}

// (a.walling 2008-07-07 11:32) - PLID 30496 - Get the current EMN from the treewnd
CEMN* CEmrEditorBase::GetCurrentEMNFromTreeWnd()
{
	return m_wndEmrTree.GetCurrentEMN();
}

// (a.walling 2008-06-27 15:49) - PLID 30482 - Get the EMR
CEMR* CEmrEditorBase::GetEMR()
{
	return m_wndEmrTree.GetEMR();
}


// (j.jones 2010-03-31 17:33) - PLID 37980 - returns TRUE if this EMR has an EMN opened to a topic that is writeable
BOOL CEmrEditorBase::HasWriteableEMRTopicOpen()
{
	//CanBeEdited() checks locked status, write token, and permissions
	if(IsWindow(m_wndEmrTree.GetSafeHwnd())
		&& GetActiveEMN() != NULL && GetActiveEMN()->CanBeEdited()
		&& m_wndEmrTree.HasWriteableEMRTopicOpen()) {
		//we have a writeable EMN selected with a selected topic,
		//that's all that is asked for here
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// (j.jones 2010-04-01 11:58) - PLID 37980 - gets the pointer to the active EMN, if there is one
CEMN* CEmrEditorBase::GetActiveEMN()
{
	return m_wndEmrTree.GetActiveEMN();
}

// (c.haag 2010-07-29 09:10) - PLID 39886 - Ensure the medication / allergy viewer is destroyed
void CEmrEditorBase::OnDestroy() 
{
	CWnd::OnDestroy();
}