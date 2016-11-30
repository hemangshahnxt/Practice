//NxPreviewView.cpp

#include "stdafx.h"
#include "nxpreviewView.h"
/////////////////////////////////////////////////////////////////////////////
// CNxPreviewView

IMPLEMENT_DYNCREATE(CNxPreviewView, CPreviewView)

CNxPreviewView::CNxPreviewView()
{
}

CNxPreviewView::~CNxPreviewView()
{
}


BEGIN_MESSAGE_MAP(CNxPreviewView, CPreviewView)
	//{{AFX_MSG_MAP(CNxPreviewView)
	ON_COMMAND(AFX_ID_PREVIEW_CLOSE, OnPreviewClose)
	ON_COMMAND(AFX_ID_PREVIEW_PRINT, OnPreviewPrint)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
void CNxPreviewView::OnPreviewClose() {

	GetMainFrame()->PostMessage(NXM_PREVIEW_CLOSED);
	
	CPreviewView::OnPreviewClose();
}

void CNxPreviewView::OnPreviewPrint() {

	GetMainFrame()->PostMessage(NXM_PREVIEW_PRINTED);

	CPreviewView::OnPreviewPrint();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////