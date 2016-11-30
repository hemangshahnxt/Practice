// HistoryTabsDropSource.h: interface for the CHistoryTabsDropSource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HISTORYTABSDROPSOURCE_H__8E18BBC0_C8A1_479A_9C51_5E11D3753008__INCLUDED_)
#define AFX_HISTORYTABSDROPSOURCE_H__8E18BBC0_C8A1_479A_9C51_5E11D3753008__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "historydlg.h"

class CHistoryTabsDropSource : public COleDropSource  
{
public:
	CHistoryTabsDropSource();
	virtual ~CHistoryTabsDropSource();

	virtual BOOL OnBeginDrag(CWnd* pWnd);
	virtual SCODE GiveFeedback(DWORD dwDropEffect);
	virtual SCODE QueryContinueDrag(BOOL bEscapePressed, DWORD dwKeyState);

	BOOL m_bIsPacket;
	CString m_strDragFilename;

	// (a.walling 2008-09-22 15:04) - PLID 31471 - Allow passing in the icon
	HICON m_hIcon;
	
	CHistoryDlg *m_wndParentHistoryDlg;
protected:
	CImageList m_ilDragImage;
};

#endif // !defined(AFX_HISTORYTABSDROPSOURCE_H__8E18BBC0_C8A1_479A_9C51_5E11D3753008__INCLUDED_)
