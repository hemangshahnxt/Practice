// EmrDocument.cpp : implementation file
//

#include "stdafx.h"
#include "EmrDocument.h"
#include "EmrTopicView.h"
#include "EmrDocTemplate.h"

// (a.walling 2011-10-20 21:22) - PLID 46078 - Facelift - EMR Document / View / Command routing etc
// (a.walling 2013-02-13 09:30) - PLID 56125 - Topic and more info view creation handling now within the doctemplate

IMPLEMENT_DYNCREATE(CEmrDocument, CDocument)


BOOL CEmrDocument::IsModified()
{
	return CDocument::IsModified();
}

BOOL CEmrDocument::CanCloseFrame(CFrameWnd* pFrame)
{
	return CDocument::CanCloseFrame(pFrame);
}

BOOL CEmrDocument::SaveModified() // return TRUE if ok to continue
{
	return CDocument::SaveModified();
}

void CEmrDocument::PreCloseFrame(CFrameWnd* pFrame)
{
	CDocument::PreCloseFrame(pFrame);
}

CEmrDocTemplate* CEmrDocument::GetEmrDocTemplate() const
{
	return polymorphic_downcast<CEmrDocTemplate*>(GetDocTemplate());
}

void CEmrDocument::OnIdle()
{
	try {
		POSITION pos = GetFirstViewPosition();
		while (pos) {
			CEmrTopicView* pView = dynamic_cast<CEmrTopicView*>(GetNextView(pos));

			if (pView) {
				// (a.walling 2012-07-03 10:56) - PLID 51284 - Update the title (now requires pointer passed into it for AttachedEMNImpl)
				pView->UpdateTitle(pView);
				pView->RefreshTopicWindowPositions();
			}
		}
	} NxCatchAllIgnore();
}
