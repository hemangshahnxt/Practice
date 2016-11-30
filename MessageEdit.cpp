//MessageEdit.cpp

////////////////////////////////////////
//Tom Schneider
//The implementation of the MessageEdit control, used in MessagerDlg.
#include "stdafx.h"
#include "MessageEdit.h"

// (z.manning, 04/18/2008) - PLID 29675 - Now inherit from CNxEdit
CMessageEdit::CMessageEdit(UINT msg) : CNxEdit() {
	m_msg = msg;
	// (a.walling 2010-08-31 18:58) - PLID 9491 - Don't bother with keeping track of the CTRL button's state
	//m_bCtrlDown = false;
}

CMessageEdit::CMessageEdit() : CNxEdit() {
	m_msg = WM_USER + 1000;
	// (a.walling 2010-08-31 18:58) - PLID 9491 - Don't bother with keeping track of the CTRL button's state
	//m_bCtrlDown = false;
}

BEGIN_MESSAGE_MAP (CMessageEdit, CNxEdit)
    //ON_WM_KEYUP ()
	ON_WM_KEYDOWN ()
	ON_WM_CHAR ()
END_MESSAGE_MAP ()

void CMessageEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// (a.walling 2010-08-31 18:58) - PLID 9491 - Don't bother with keeping track of the CTRL button's state
	//if(nChar == 17) {
	//	m_bCtrlDown = true;
	//}
	//else if(nChar == 13) {//Enter
	//	if(m_bCtrlDown) {
	//		GetOwner()->PostMessage(m_msg);
	//		return;
	//	}
	//}

	// (a.walling 2010-08-31 19:01) - PLID 9491 - Check the state of the CTRL key via GetKeyState
	if(nChar == VK_RETURN) {
		if(GetKeyState(VK_CONTROL) & 0x8000) {
			CWnd* pOwner = GetOwner();
			if(pOwner) {
				pOwner->PostMessage(m_msg);
				return;
			}
		}
	}

	CNxEdit::OnKeyDown(nChar, nRepCnt, nFlags);
} 

// (a.walling 2010-08-31 18:58) - PLID 9491 - Don't bother with keeping track of the CTRL button's state
//void CMessageEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
//{
//	if(nChar == 17) m_bCtrlDown = false;
//	else if(nChar == 13) return;
//	CNxEdit::OnKeyUp(nChar, nRepCnt, nFlags);
//}

void CMessageEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// (a.walling 2010-08-31 18:58) - PLID 9491 - Don't bother with keeping track of the CTRL button's state
	//if(nChar == 13) {//Enter
	//	if(m_bCtrlDown) {
	//		GetOwner()->PostMessage(m_msg);
	//		return;
	//	}
	//}

	// (a.walling 2010-08-31 19:01) - PLID 9491 - Check the state of the CTRL key via GetKeyState
	if(nChar == VK_RETURN) {
		if(GetKeyState(VK_CONTROL) & 0x8000) {
			CWnd* pOwner = GetOwner();
			if(pOwner) {
				pOwner->PostMessage(m_msg);
				return;
			}
		}
	}
	CNxEdit::OnChar(nChar, nRepCnt, nFlags);
}

BOOL CMessageEdit::PreTranslateMessage(MSG* pMsg)
{
	// (a.walling 2010-08-31 18:58) - PLID 9491 - Don't bother with keeping track of the CTRL button's state
	//if(pMsg->message == WM_KEYDOWN) {
	//	if(pMsg->wParam == 13) {
	//		if(m_bCtrlDown) {
	//			CWnd* pOwner = GetOwner();
	//			if(pOwner)
	//				pOwner->PostMessage(m_msg);
	//			return TRUE;
	//		}
	//	}
	//	else if (pMsg->wParam == 17) {
	//		m_bCtrlDown = true;
	//	}
	//}

	// (a.walling 2010-08-31 19:01) - PLID 9491 - Check the state of the CTRL key via GetKeyState
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
		if(GetKeyState(VK_CONTROL) & 0x8000) {
			CWnd* pOwner = GetOwner();
			if(pOwner) {
				pOwner->PostMessage(m_msg);
				return TRUE;
			}
		}
	}
	return CNxEdit::PreTranslateMessage(pMsg);
}