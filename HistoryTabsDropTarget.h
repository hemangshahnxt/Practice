// HistoryTabsDropTarget.h: interface for the CHistoryTabsDropTarget class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HISTORYTABSDROPTARGET_H__34BF5C44_BE8C_47CF_A557_F7E4A9C6EC14__INCLUDED_)
#define AFX_HISTORYTABSDROPTARGET_H__34BF5C44_BE8C_47CF_A557_F7E4A9C6EC14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define CF_NXHISTDD 			TEXT("NxHistoryDragDrop")

#define TT_TABS			0
#define TT_DOCUMENTS	1

class CHistoryTabsDropTarget : public COleDropTarget  
{
public:
	CHistoryTabsDropTarget();
	virtual ~CHistoryTabsDropTarget();

	virtual DROPEFFECT OnDragEnter(CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd *pWnd, COleDataObject *pDataObject, DROPEFFECT dropEffect, CPoint point);

	int m_nTargetType;
	int m_nInitialTab;
	// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
	NxTab::_DNxTabPtr m_ptrTabs;

};

#endif // !defined(AFX_HISTORYTABSDROPTARGET_H__34BF5C44_BE8C_47CF_A557_F7E4A9C6EC14__INCLUDED_)
