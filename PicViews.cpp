#include "stdafx.h"
#include "PicViews.h"
#include "PicContainerDlg.h"


// (a.walling 2011-12-08 17:27) - PLID 46641 - Create views for Labs, History, and ProcInfoCenter


IMPLEMENT_DYNAMIC(CPicDocTemplate, CMultiDocTemplate)

// (a.walling 2012-11-09 08:45) - PLID 53671 - Views are now created / accessed via the doctemplate

CHistoryView* CPicDocTemplate::GetHistoryView(bool bAutoCreate)
{
	if (!this) {
		return NULL;
	}

	if (CHistoryView* pView = FindView<CHistoryView>()) {
		return pView;
	} else if (!bAutoCreate) {
		return NULL;
	}

	CPicDocument* pDoc = polymorphic_downcast<CPicDocument*>(CreateNewDocument());
	// (a.walling 2012-11-09 08:45) - PLID 53671 - Set a pseudo-path for the document
	pDoc->SetPathNameDirect("History.View.Emr.Nx");

	CFrameWnd* pFrameWnd = CreateNewEmrFrame(pDoc, RUNTIME_CLASS(CHistoryView));

	CHistoryView* pHistoryView = GetHistoryView(false);

	ASSERT(pHistoryView);

	pFrameWnd->SetWindowText("History");

	// (a.walling 2013-05-22 09:19) - PLID 56812 - Do not 'make visible'
	// this is already visible, passing TRUE to InitialUpdateFrame will activate this MDI window, which we don't really want in our case.
	pHistoryView->GetParentFrame()->InitialUpdateFrame(pDoc, FALSE);

	return pHistoryView;
}

CPatientLabsView* CPicDocTemplate::GetPatientLabsView(bool bAutoCreate)
{
	if (!this) {
		return NULL;
	}

	if (CPatientLabsView* pView = FindView<CPatientLabsView>()) {
		return pView;
	} else if (!bAutoCreate) {
		return NULL;
	}

	CPicDocument* pDoc = polymorphic_downcast<CPicDocument*>(CreateNewDocument());
	// (a.walling 2012-11-09 08:45) - PLID 53671 - Set a pseudo-path for the document
	pDoc->SetPathNameDirect("Labs.View.Emr.Nx");

	CFrameWnd* pFrameWnd = CreateNewEmrFrame(pDoc, RUNTIME_CLASS(CPatientLabsView));

	CPatientLabsView* pLabsView = GetPatientLabsView(false);

	ASSERT(pLabsView);

	pFrameWnd->SetWindowText("Labs");

	// (a.walling 2013-05-22 09:19) - PLID 56812 - Do not 'make visible'
	// this is already visible, passing TRUE to InitialUpdateFrame will activate this MDI window, which we don't really want in our case.
	pLabsView->GetParentFrame()->InitialUpdateFrame(pDoc, FALSE);

	return pLabsView;
}

CProcInfoCenterView* CPicDocTemplate::GetProcInfoCenterView(bool bAutoCreate)
{
	if (!this) {
		return NULL;
	}

	if (CProcInfoCenterView* pView = FindView<CProcInfoCenterView>()) {
		return pView;
	} else if (!bAutoCreate) {
		return NULL;
	}

	CPicDocument* pDoc = polymorphic_downcast<CPicDocument*>(CreateNewDocument());
	// (a.walling 2012-11-09 08:45) - PLID 53671 - Set a pseudo-path for the document
	pDoc->SetPathNameDirect("ProcInfoCenter.View.Emr.Nx");

	CFrameWnd* pFrameWnd = CreateNewEmrFrame(pDoc, RUNTIME_CLASS(CProcInfoCenterView));

	CProcInfoCenterView* pPicView = GetProcInfoCenterView(false);

	ASSERT(pPicView);

	pFrameWnd->SetWindowText("PIC / Non-clinical");

	// (a.walling 2013-05-22 09:19) - PLID 56812 - Do not 'make visible'
	// this is already visible, passing TRUE to InitialUpdateFrame will activate this MDI window, which we don't really want in our case.
	pPicView->GetParentFrame()->InitialUpdateFrame(pDoc, FALSE);

	return pPicView;
}


CFrameWnd* CPicDocTemplate::CreateNewEmrFrame(CDocument* pDoc, CRuntimeClass* pViewClass)
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

void CPicDocTemplate::SetDefaultTitle(CDocument* pDocument)
{
	pDocument->SetTitle("");
}

IMPLEMENT_DYNCREATE(CPicDocument, CDocument)

CPicDocTemplate* CPicDocument::GetPicDocTemplate() const
{
	return polymorphic_downcast<CPicDocTemplate*>(GetDocTemplate());
}

IMPLEMENT_DYNCREATE(CHistoryView, CNexTechDialogView)

CWnd* CHistoryView::CreateChildWnd()
{
	m_historyDlg.SetPicContainer(GetPicContainer());
	m_historyDlg.Create(CHistoryDlg::IDD, this);
	//(e.lally 2012-04-11) PLID 49546 - HistoryDlg expects update view to be called right away so the current person id gets set.
	m_historyDlg.UpdateView(true);
	return &m_historyDlg;
}


IMPLEMENT_DYNCREATE(CPatientLabsView, CNexTechDialogView)

CWnd* CPatientLabsView::CreateChildWnd()
{
	m_patientLabsDlg.SetPicContainer(GetPicContainer());
	m_patientLabsDlg.Create(CPatientLabsDlg::IDD, this);
	// (d.thompson 2013-01-30) - PLID 54933 - Patient labs also need to be updated immediately, otherwise no data will load.
	m_patientLabsDlg.UpdateView(true);
	return &m_patientLabsDlg;
}


IMPLEMENT_DYNCREATE(CProcInfoCenterView, CNexTechDialogView)

CWnd* CProcInfoCenterView::CreateChildWnd()
{
	m_procInfoCenterDlg.m_nColor = GetPicContainer()->m_nColor;
	m_procInfoCenterDlg.m_nProcInfoID = GetPicContainer()->GetCurrentProcInfoID();
	m_procInfoCenterDlg.SetPicContainer(GetPicContainer());
	m_procInfoCenterDlg.Create(CProcInfoCenterDlg::IDD, this);
	m_procInfoCenterDlg.Load(PIC_AREA_ALL);
	return &m_procInfoCenterDlg;
}
