// EMRCustomPreviewLayoutView.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "AdministratorRc.h"
#include "EMRCustomPreviewLayoutsMDIFrame.h"
#include "EMRCustomPreviewLayoutView.h"
#include <NxAdvancedUILib/NxHtmlMDIDoc.h>
using namespace boost;
#include <NxAdvancedUILib/NxHtmlEditor.h>
#include <NxSystemUtilitiesLib/ResourceUtils.h>
#include <NxSystemUtilitiesLib/NxCompressUtils.h>

// CEMRCustomPreviewLayoutView

IMPLEMENT_DYNCREATE(CEMRCustomPreviewLayoutView, CNxHtmlMDIView)

CEMRCustomPreviewLayoutView::CEMRCustomPreviewLayoutView()
{

}

CEMRCustomPreviewLayoutView::~CEMRCustomPreviewLayoutView()
{
}

// (j.armen 2013-01-21 11:31) - PLID 54711 - Extract the editor from our exe and decompress it into a temp folder.
//	We only do this once per practice session.  This allows us to upgrade the Practice.exe with a new version of CKEditor
//	Without the worries of a user attempting to use an older version of our editor.
CString CEMRCustomPreviewLayoutView::GetEditorUrl()
{
	if(!FileUtils::DoesFileOrDirExist(GetNxTempPath() ^ "NxHtmlEditor" ^ "NxHtmlEditor.html"))
	{
		_variant_t vt = ResourceUtils::LoadResourceDataInfoSafeArrayOfBytes(NULL, "CKEditor.zip", "ZIP");
		FileUtils::CreateFileFromSafeArrayOfBytes(vt, GetNxTempPath() ^ "CKEditor.zip");
		NxCompressUtils::NxUncompressFileToFile(GetNxTempPath() ^ "CKEditor.zip", GetNxTempPath() ^ "NxHtmlEditor");
	}

	return GetNxTempPath() ^ "NxHtmlEditor" ^ "NxHtmlEditor.html";
}


BEGIN_MESSAGE_MAP(CEMRCustomPreviewLayoutView, CNxHtmlMDIView)
	ON_COMMAND(ID_FILE_RENAMELAYOUT, OnRenameLayout)
	ON_COMMAND(ID_FILE_DELETELAYOUT, OnDeleteLayout)
	ON_COMMAND(ID_FILE_SAVECURRENTLAYOUT, OnFileSaveCurrentLayout)
	ON_COMMAND(ID_VIEW_SHOWSOURCE, OnShowSource)

	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWSOURCE, OnUpdateShowSource)
END_MESSAGE_MAP()

// CEMRCustomPreviewLayoutView message handlers

void CEMRCustomPreviewLayoutView::OnRenameLayout()
{
	// Renames the current layout
	try {
		CNxHtmlMDIDoc* pDoc = dynamic_cast<CNxHtmlMDIDoc*>(GetDocument());
		CString strLayoutName = pDoc->GetDocName();
		CString strOriginalLayoutName = strLayoutName;
		int nResult = CEMRCustomPreviewLayoutsMDIFrame::PromptForLayoutName(this, strLayoutName);
		if(nResult == IDOK && strOriginalLayoutName != strLayoutName)
		{
			// Before we update the view, we have to be sure the document has the latest HTML 
			// because the view will just read it right back in during the update.
			pDoc->SetHtml(GetHtml());
			// Now update the name and flag the document as modiifed
			pDoc->SetDocName(strLayoutName);
			pDoc->SetModifiedFlag();
			// Update the views
			pDoc->UpdateAllViews(NULL);
		}
	} NxCatchAll(__FUNCTION__);
}

// Menu command initiated by the user to delete this layout
void CEMRCustomPreviewLayoutView::OnDeleteLayout()
{
	// (c.haag 2013-01-23) - PLID 54739 - Initial implementation
	try {
		// Confirm the user wants to do this
		CNxHtmlMDIDoc* pDoc = dynamic_cast<CNxHtmlMDIDoc*>(GetDocument());
		if (IDYES == AfxMessageBox(FormatString("This is an unrecoverable operation!\r\n\r\nAre you SURE you wish to delete the '%s' layout?", pDoc->GetDocName() ), MB_YESNO))
		{
			if (!pDoc->IsNew())
			{
				// Get a pointer to the MDI frame because that's the official object for accessing the API
				CMDIChildWndEx *pFrame = dynamic_cast<CMDIChildWndEx*>(GetParentFrame());
				CEMRCustomPreviewLayoutsMDIFrame *pParentFrame = dynamic_cast<CEMRCustomPreviewLayoutsMDIFrame*>(pFrame->GetParentFrame());
				// Delete the data
				pParentFrame->DeleteFromDatabase(this);
			}
			else
			{
				// New documents aren't in data
			}
			// Now close the view
			pDoc->SetModifiedFlag(FALSE);
			PostMessage(WM_COMMAND, ID_FILE_CLOSE);
		}		
	} NxCatchAll(__FUNCTION__);
}

// Menu command initiated by the user to save this layout
void CEMRCustomPreviewLayoutView::OnFileSaveCurrentLayout()
{
	// (c.haag 2013-01-23) - PLID 54739 - Initial implementation
	try {
		// Get a pointer to the MDI frame because that's the official object for accessing the API
		CMDIChildWndEx *pFrame = dynamic_cast<CMDIChildWndEx*>(GetParentFrame());
		CEMRCustomPreviewLayoutsMDIFrame *pParentFrame = dynamic_cast<CEMRCustomPreviewLayoutsMDIFrame*>(pFrame->GetParentFrame());
		// Save the data
		pParentFrame->SaveToDatabase(this);
	} NxCatchAll(__FUNCTION__);
}

// (j.armen 2013-02-01 13:32) - PLID 54981 - Trigger showing source
void CEMRCustomPreviewLayoutView::OnShowSource()
{
	try {
		bool bShowingSource = m_pHtmlEditor->IsShowingSource();

		if(!bShowingSource)
		{
			if(IDNO == DontShowMeAgain(this, "Editing the source HTML for the preview allows you greater customization.  "
				"Use this feature with extreme caution.  Are you sure you wish to continue?",
				"CustomPreviewLayoutSourceModeWarning", "Custom Preview Layout - Source Mode Warning", FALSE, TRUE))
			{
				return;
			}
		}

		m_pHtmlEditor->ShowSource(!bShowingSource);
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2013-02-01 13:32) - PLID 54981 - Update the check if we are showing source
void CEMRCustomPreviewLayoutView::OnUpdateShowSource(CCmdUI* pCmdUI)
{
	try {
		pCmdUI->SetCheck(m_pHtmlEditor->IsShowingSource() ? BST_CHECKED : BST_UNCHECKED);
	} NxCatchAll(__FUNCTION__);
}