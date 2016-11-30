// NxRichEditCtrl.cpp: implementation of the CNxRichEditCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <NxPracticeSharedLib/RichEditUtils.h>
#include "NxRichEditCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNxRichEditCtrl::CNxRichEditCtrl()
{

}

CNxRichEditCtrl::~CNxRichEditCtrl()
{

}

BEGIN_MESSAGE_MAP(CNxRichEditCtrl, CRichEditCtrl)
	//{{AFX_MSG_MAP(CNxRichEditCtrl)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CNxRichEditCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar == VK_TAB) {
		//Why do we have to do this ourselves?  I don't know.
		CHARRANGE cr;
		SendMessage(EM_EXGETSEL, 0, (LPARAM)&cr);
        cr.cpMax = cr.cpMin; // caret only
        SendMessage(EM_REPLACESEL, 0, (LPARAM)"\t");
	}
	else {
		CRichEditCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

BOOL CNxRichEditCtrl::PreCreateWindow(CREATESTRUCT &cs)
{
	cs.lpszClass = RICHEDIT_CLASS; //This makes us into a RichEdit 2.0 window.

	return CRichEditCtrl::PreCreateWindow(cs);
}

void CNxRichEditCtrl::SetRichText(const CString &strRichText)
{
	SetRtfText(this, strRichText, SF_RTF);
}

void CNxRichEditCtrl::SetPlainText(const CString &strPlainText)
{
	SetRtfText(this, strPlainText, SF_TEXT);
}

void CNxRichEditCtrl::GetRichText(CString &strRichText)
{
	GetRtfText(this, strRichText, SF_RTF);
}

void CNxRichEditCtrl::GetPlainText(CString &strPlainText)
{
	GetRtfText(this, strPlainText, SF_TEXT);
}