//MessageEdit.h

///////////////////////////////////////////////
//Tom Schneider
//An edit box which passes a message to its parent window
//whenever Ctrl-Enter is pressed.  Used by MessagerDlg.

#ifndef MESSAGEEDIT_H
#define MESSAGEEDIT_H

#pragma once



// (z.manning, 04/18/2008) - PLID 29675 - This now inherits from CNxEdit
class CMessageEdit : public CNxEdit
{
public:
	CMessageEdit(UINT msg);
	CMessageEdit();

protected:
	// (a.walling 2010-08-31 18:58) - PLID 9491 - Don't bother with keeping track of the CTRL button's state
	//bool m_bCtrlDown;
	UINT m_msg;
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
};


#endif