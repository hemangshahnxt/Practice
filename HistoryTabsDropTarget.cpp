// HistoryTabsDropTarget.cpp: implementation of the CHistoryTabsDropTarget class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HistoryTabsDropTarget.h"
#include "historydlg.h"

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace NxTab;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHistoryTabsDropTarget::CHistoryTabsDropTarget()
{
	m_nInitialTab = -1;
	m_ptrTabs = NULL;
}

CHistoryTabsDropTarget::~CHistoryTabsDropTarget()
{
	//Revoke();
}

DROPEFFECT CHistoryTabsDropTarget::OnDragEnter(CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point)
{
	if(pDataObject->IsDataAvailable(CF_HDROP)) {
		return DROPEFFECT_COPY;
	}
	else if (pDataObject->IsDataAvailable(RegisterClipboardFormat(CF_NXHISTDD))) {
		return DROPEFFECT_MOVE;
	}
	else {
		return DROPEFFECT_NONE;
	}
}

DROPEFFECT CHistoryTabsDropTarget::OnDragOver(CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point)
{
	if(pDataObject->IsDataAvailable(CF_HDROP) || 
		pDataObject->IsDataAvailable(RegisterClipboardFormat(CF_NXHISTDD)))
	{
		if (m_nTargetType == TT_TABS) {
			//Switch to the tab we're over.
			_DNxTabPtr pTab = pWnd->GetControlUnknown();
			ClientToScreen(pWnd->GetSafeHwnd(), &point);
			short nTab = pTab->GetTabFromPoint(point.x, point.y);
			if(nTab != -1 && pTab->CurSel != nTab) {
				pWnd->GetParent()->PostMessage(NXM_HISTORY_TAB_CHANGED, nTab, pTab->CurSel);
				pTab->CurSel = nTab;
			}
			if (nTab == 0) {
				// This is the photos tab, you can't drop here
				return DROPEFFECT_NONE;
			}
			else {
				return DROPEFFECT_MOVE;
			}
		}
		else if (m_nTargetType == TT_DOCUMENTS) {
			short nTab;
			if (m_ptrTabs)
				nTab = m_ptrTabs->CurSel;
			else
				nTab = -1;
			if (nTab != m_nInitialTab)
				return DROPEFFECT_MOVE;
			else
				return DROPEFFECT_NONE;
		}
		else {
			return DROPEFFECT_COPY;
		}		
	}
	else {
		return DROPEFFECT_NONE;
	}
}

BOOL CHistoryTabsDropTarget::OnDrop(CWnd *pWnd, COleDataObject *pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	if(pDataObject->IsDataAvailable(CF_HDROP)|| 
		pDataObject->IsDataAvailable(RegisterClipboardFormat(CF_NXHISTDD)))
	{
		if (m_nTargetType == TT_TABS) {
			//Switch to the tab we're over.
			_DNxTabPtr pTab = pWnd->GetControlUnknown();
			ClientToScreen(pWnd->GetSafeHwnd(), &point);
			short nTab = pTab->GetTabFromPoint(point.x, point.y);
			if(nTab != -1 && pTab->CurSel != nTab) {
				//Use SendMessage, because we need to use SendMessage below.
				pWnd->GetParent()->SendMessage(NXM_HISTORY_TAB_CHANGED, nTab, pTab->CurSel);
				pTab->CurSel = nTab;
			}
		}
		//Now, tell history that we're dropping (use SendMessage, otherwise the data won't be valid any more).
		//Make sure to send the right type of data
		if (pDataObject->IsDataAvailable(CF_HDROP))
			pWnd->GetParent()->SendMessage(NXM_DROP_FILES, (WPARAM)pDataObject->GetGlobalData(CF_HDROP));
		else if (pDataObject->IsDataAvailable(RegisterClipboardFormat(CF_NXHISTDD)))
			pWnd->GetParent()->SendMessage(NXM_DROP_FILES, (WPARAM)pDataObject->GetGlobalData(RegisterClipboardFormat(CF_NXHISTDD)));
		
		m_ptrTabs = NULL;

		return TRUE;
	}
	return FALSE;
}
