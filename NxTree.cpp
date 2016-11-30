// NxTree.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "NxTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNxTree

CNxTree::CNxTree()
{
}

CNxTree::~CNxTree()
{
}

CString CNxTree::GetSelectedItemText()
{
	if (GetSelectedItem())
		return GetItemText(GetSelectedItem());
	else return "";
}

void CNxTree::SetSelectedItemText(CString set)
{
	SetItemText(GetSelectedItem(), set);
}

HTREEITEM CNxTree::GetSelectedItemParent()
{
	return GetParentItem(GetSelectedItem());
}

void CNxTree::SelectFirstVisible()
{
	SelectItem(GetFirstVisibleItem());
}

BEGIN_MESSAGE_MAP(CNxTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CNxTree)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginlabeledit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNxTree message handlers
void CNxTree::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_add = (GetSelectedItemText() == "Add. . .");
	*pResult = 0;
}

BOOL CNxTree::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
	{	CEdit* edit = GetEditControl();
		if (edit)
		{	if (pMsg->wParam == VK_ESCAPE)
				edit->SetWindowText("");
			edit->SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
			return TRUE;
		}
		else if (pMsg->wParam == VK_RETURN)
			EditLabel(GetSelectedItem());
	}
	return CTreeCtrl::PreTranslateMessage(pMsg);
}
