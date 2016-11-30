#include "stdafx.h"
#include "EmrDocTemplate.h"

IMPLEMENT_DYNAMIC(CEmrDocTemplate, CMultiDocTemplate)

CFrameWnd* CEmrDocTemplate::CreateNewEmrFrame(CDocument* pDoc, CRuntimeClass* pViewClass)
{
	if (pDoc != NULL)
		ASSERT_VALID(pDoc);
	// create a frame wired to the specified document

	ASSERT(m_nIDResource != 0); // must have a resource ID to load from
	CCreateContext context;
	context.m_pCurrentFrame = NULL;
	context.m_pCurrentDoc = pDoc;
	context.m_pNewViewClass = pViewClass;
	context.m_pNewDocTemplate = this;

	if (m_pFrameClass == NULL)
	{
		TRACE(traceAppMsg, 0, "Error: you must override CDocTemplate::CreateNewFrame.\n");
		ASSERT(FALSE);
		return NULL;
	}
	CFrameWnd* pFrame = (CFrameWnd*)m_pFrameClass->CreateObject();
	if (pFrame == NULL)
	{
		TRACE(traceAppMsg, 0, "Warning: Dynamic create of frame %hs failed.\n",
			m_pFrameClass->m_lpszClassName);
		return NULL;
	}
	ASSERT_KINDOF(CFrameWnd, pFrame);

	if (context.m_pNewViewClass == NULL)
		TRACE(traceAppMsg, 0, "Warning: creating frame with no default view.\n");

	CWnd* pParentWnd = NULL;
	if (m_pOverrideFrameParent) {
		pParentWnd = m_pOverrideFrameParent;
	}

	// create new from resource
	// (a.walling 2013-05-02 15:45) - PLID 52629 - Ensure this is always created visible
	if (!pFrame->LoadFrame(m_nIDResource,
			WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE | WS_VISIBLE,   // default frame styles + visible
			pParentWnd, &context))
	{
		TRACE(traceAppMsg, 0, "Warning: CDocTemplate couldn't create a frame.\n");
		// frame will be deleted in PostNcDestroy cleanup
		return NULL;
	}

	// it worked !
	return pFrame;
}

void CEmrDocTemplate::SetDefaultTitle(CDocument* pDocument)
{
	pDocument->SetTitle("");
}

// (a.walling 2013-02-13 09:30) - PLID 56125 - Topic and more info view creation handling now within the doctemplate

CEmrTopicView* CEmrDocTemplate::GetEmrTopicView(CEMN* pEMN, bool bAutoCreate)
{
	{
		POSITION pos = GetFirstDocPosition();
		while (pos) {
			if (CEmrDocument* pDoc = polymorphic_downcast<CEmrDocument*>(GetNextDoc(pos))) {
				if (pDoc->GetAttachedEMN() == pEMN) {
					if (CEmrTopicView* pView = pDoc->FindView<CEmrTopicView>()) {
						return pView;
					}
				}
			}
		}
	}

	if (!bAutoCreate) {
		return NULL;
	}

	// (a.walling 2013-04-17 17:25) - PLID 52629 - not found, take over unattached if possible?
	CEmrTopicView* pView = NULL;
	{
		POSITION pos = GetFirstDocPosition();
		while (pos && !pView) {
			if (CEmrDocument* pDoc = polymorphic_downcast<CEmrDocument*>(GetNextDoc(pos))) {
				if (!pDoc->GetAttachedEMN()) {
					pView = pDoc->FindView<CEmrTopicView>();
				}
			}
		}
	}

	if (!pView) {
		pView = CreateEmrTopicView(pEMN);
	}

	// (a.walling 2012-11-09 17:25) - PLID 53689 - Attach emn first, and update the title immediately
	// this gets rid of the intermediate 'Topics' title
	pView->SetAttachedEMN(pEMN);
	polymorphic_downcast<CEmrDocument*>(pView->GetDocument())->SetAttachedEMN(pEMN);

	pView->UpdateTitle(pView);
	
	// (a.walling 2013-05-22 09:19) - PLID 56812 - Do not 'make visible'
	// this is already visible, passing TRUE to InitialUpdateFrame will activate this MDI window, which we don't really want in our case.
	pView->GetParentFrame()->InitialUpdateFrame(pView->GetDocument(), FALSE);

	return pView;
}

CEmrTopicView* CEmrDocTemplate::CreateEmrTopicView(CEMN* pEMN)
{
	CEmrDocument* pDoc = polymorphic_downcast<CEmrDocument*>(CreateNewDocument());
	// (a.walling 2013-04-17 17:25) - PLID 52629 - Set the internal path name for the topic view
	pDoc->SetPathNameDirect("Topics.View.Emr.Nx");
	pDoc->SetAttachedEMN(pEMN);

	CFrameWnd* pFrameWnd = CreateNewEmrFrame(pDoc, RUNTIME_CLASS(CEmrTopicView));
	
	CEmrTopicView* pEmrTopicView = pDoc->FindView<CEmrTopicView>();

	ASSERT(pEmrTopicView);

	//pEmrTopicView->RefreshTopicWindowPositions();

	return pEmrTopicView;
}

CEmrMoreInfoView* CEmrDocTemplate::GetEmrMoreInfoView(CEMN* pEMN, bool bAutoCreate)
{
	{
		POSITION pos = GetFirstDocPosition();
		while (pos) {
			if (CEmrDocument* pDoc = polymorphic_downcast<CEmrDocument*>(GetNextDoc(pos))) {
				if (pDoc->GetAttachedEMN() == pEMN) {
					if (CEmrMoreInfoView* pView = pDoc->FindView<CEmrMoreInfoView>()) {
						return pView;
					}
				}
			}
		}
	}

	if (!bAutoCreate) {
		return NULL;
	}

	// (a.walling 2013-04-17 17:25) - PLID 52629 - not found, take over unattached if possible?
	CEmrMoreInfoView* pView = NULL;
	{
		POSITION pos = GetFirstDocPosition();
		while (pos && !pView) {
			if (CEmrDocument* pDoc = polymorphic_downcast<CEmrDocument*>(GetNextDoc(pos))) {
				if (!pDoc->GetAttachedEMN()) {
					pView = pDoc->FindView<CEmrMoreInfoView>();
				}
			}
		}
	}

	if (!pView) {
		pView = CreateEmrMoreInfoView();
	}

	pView->SetAttachedEMN(pEMN);
	polymorphic_downcast<CEmrDocument*>(pView->GetDocument())->SetAttachedEMN(pEMN);
	pView->GetMoreInfoDlg()->Initialize(pEMN);
	
	pView->GetParentFrame()->InitialUpdateFrame(pView->GetDocument(), TRUE);

	return pView;
}

CEmrMoreInfoView* CEmrDocTemplate::CreateEmrMoreInfoView(CEMN* pEMN)
{
	CEmrDocument* pDoc = polymorphic_downcast<CEmrDocument*>(CreateNewDocument());
	pDoc->SetAttachedEMN(pEMN);

	CFrameWnd* pFrameWnd = CreateNewEmrFrame(pDoc, RUNTIME_CLASS(CEmrMoreInfoView));
	pFrameWnd->SetWindowText("More Info");
	
	CEmrMoreInfoView* pEmrMoreInfoView = pDoc->FindView<CEmrMoreInfoView>();

	ASSERT(pEmrMoreInfoView);

	return pEmrMoreInfoView;
}

//TES 2/12/2014 - PLID 60740 - Added for the new <Codes> topic
CEmrCodesView* CEmrDocTemplate::GetEmrCodesView(CEMN* pEMN, bool bAutoCreate)
{
	{
		POSITION pos = GetFirstDocPosition();
		while (pos) {
			if (CEmrDocument* pDoc = polymorphic_downcast<CEmrDocument*>(GetNextDoc(pos))) {
				if (pDoc->GetAttachedEMN() == pEMN) {
					if (CEmrCodesView* pView = pDoc->FindView<CEmrCodesView>()) {
						return pView;
					}
				}
			}
		}
	}

	if (!bAutoCreate) {
		return NULL;
	}

	// (a.walling 2013-04-17 17:25) - PLID 52629 - not found, take over unattached if possible?
	CEmrCodesView* pView = NULL;
	{
		POSITION pos = GetFirstDocPosition();
		while (pos && !pView) {
			if (CEmrDocument* pDoc = polymorphic_downcast<CEmrDocument*>(GetNextDoc(pos))) {
				if (!pDoc->GetAttachedEMN()) {
					pView = pDoc->FindView<CEmrCodesView>();
				}
			}
		}
	}

	if (!pView) {
		pView = CreateEmrCodesView();
	}

	pView->SetAttachedEMN(pEMN);
	polymorphic_downcast<CEmrDocument*>(pView->GetDocument())->SetAttachedEMN(pEMN);
	pView->GetEmrCodesDlg()->Initialize(pEMN);
	
	pView->GetParentFrame()->InitialUpdateFrame(pView->GetDocument(), TRUE);

	return pView;
}

//TES 2/12/2014 - PLID 60740 - Added for the new <Codes> topic
CEmrCodesView* CEmrDocTemplate::CreateEmrCodesView(CEMN* pEMN)
{
	CEmrDocument* pDoc = polymorphic_downcast<CEmrDocument*>(CreateNewDocument());
	pDoc->SetAttachedEMN(pEMN);

	CFrameWnd* pFrameWnd = CreateNewEmrFrame(pDoc, RUNTIME_CLASS(CEmrCodesView));
	pFrameWnd->SetWindowText("Codes");
	
	CEmrCodesView* pEmrCodesView = pDoc->FindView<CEmrCodesView>();

	ASSERT(pEmrCodesView);

	return pEmrCodesView;
}