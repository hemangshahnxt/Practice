#include "stdafx.h"
#include "EmrChildWnd.h"
#include <NxUILib/WindowUtils.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace {
		
	template<typename T>
	class AccessProtectedType : public T
	{
	public:
		friend class CEmrChildWnd;
	};

	template<typename T>
	inline
	AccessProtectedType<T>* AccessProtected(T* pObj)
	{
		return reinterpret_cast<AccessProtectedType<T>*>(pObj);
	}

	template<typename T>
	inline
	AccessProtectedType<T>& AccessProtected(T& pObj)
	{
		return reinterpret_cast<AccessProtectedType<T>&>(pObj);
	}
}

// CEmrChildWnd

IMPLEMENT_DYNCREATE(CEmrChildWnd, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CEmrChildWnd, CMDIChildWndEx)
END_MESSAGE_MAP()

BOOL CEmrChildWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~FWS_ADDTOTITLE;
	// (a.walling 2012-03-29 16:37) - PLID 46076 - Ensure we have set this window to clip children
	cs.style |= WS_CLIPCHILDREN;

	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

// (a.walling 2012-07-05 08:40) - PLID 51335 - Override to use NxSetWindowText
void CEmrChildWnd::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	BOOL bRedraw = AccessProtected(m_Impl).IsOwnerDrawCaption() && IsWindowVisible() &&(GetStyle() & WS_MAXIMIZE) == 0;

	CString strTitle1;

	if (bRedraw)
	{
		GetWindowText(strTitle1);
	}

	CMDIChildWnd_OnUpdateFrameTitle(bAddToTitle);

	if (bRedraw)
	{
		CString strTitle2;
		GetWindowText(strTitle2);

		if (strTitle1 != strTitle2)
		{
			SendMessage(WM_NCPAINT, 0, 0);
		}
	}

	if (m_pMDIFrame != NULL)
	{
		ASSERT_VALID(m_pMDIFrame);
		AccessProtected(m_pMDIFrame)->m_wndClientArea.UpdateTabs();
	}
}

// (a.walling 2012-07-05 08:40) - PLID 51335 - Override to use NxSetWindowText
void CEmrChildWnd::CMDIChildWnd_OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// update our parent window first
	GetMDIFrame()->OnUpdateFrameTitle(bAddToTitle);

	if ((GetStyle() & FWS_ADDTOTITLE) == 0)
		return;     // leave child window alone!

	CDocument* pDocument = GetActiveDocument();
	if (bAddToTitle)
	{
		TCHAR szText[256+_MAX_PATH];
		if (pDocument == NULL)
			Checked::tcsncpy_s(szText, _countof(szText), m_strTitle, _TRUNCATE);
		else
			Checked::tcsncpy_s(szText, _countof(szText), pDocument->GetTitle(), _TRUNCATE);
		if (m_nWindow > 0)
		{
			TCHAR szWinNumber[16+1];
			_stprintf_s(szWinNumber, _countof(szWinNumber), _T(":%d"), m_nWindow);
			
			if( lstrlen(szText) + lstrlen(szWinNumber) < _countof(szText) )
			{
				Checked::tcscat_s( szText, _countof(szText), szWinNumber ); 
			}
		}

		// set title if changed, but don't remove completely
		// (a.walling 2012-07-05 08:40) - PLID 51335 - Use NxSetWindowText
		WindowUtils::NxSetWindowText(this, szText);
	}
}

// CEmrChildWnd message handlers