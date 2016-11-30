// EmrTemplateEditorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsrc.h"
#include "EmrTemplateEditorDlg.h"
#include "EmrTreeWnd.h"
#include "MultiSelectDlg.h"
#include "EmrItemEntryDlg.h"
#include "EmrSetupDlg.h"
#include "NxModalParentDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEmrTemplateEditorDlg dialog

// (a.walling 2012-02-29 06:42) - PLID 46644 - Moved some CEmrTemplateEditorDlg logic to EmrTemplateFrameWnd; removed a lot


CEmrTemplateEditorDlg::CEmrTemplateEditorDlg()
{
}


BEGIN_MESSAGE_MAP(CEmrTemplateEditorDlg, CEmrEditorBase)
	//{{AFX_MSG_MAP(CEmrTemplateEditorDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrTemplateEditorDlg message handlers

void CEmrTemplateEditorDlg::Initialize() 
{

	// (j.jones 2007-07-17 13:44) - PLID 26712 - bulk cache template editor preferences
	g_propManager.CachePropertiesInBulk("EmrTemplateEditorDlg", propNumber,
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'EMRSavePref' OR "
		"Name = 'EmrAutoCollapse' OR "
		"Name = 'EMNPreviewAutoScroll' OR " 
		"Name = 'EMRTreeIconSize' OR "
		"Name = 'LoadGeneral2ICD9ToEMR' OR "
		"Name = 'EMNTemplateActionBold' OR "
		"Name = 'PICSaveDocsInHistory' OR "
		"Name = 'EMR_Image_Use_Custom_Stamps' OR "
		"Name = 'DefaultEMRImagePenColor' OR "
		"Name = 'EMRColorPartial' OR "
		"Name = 'EMRColorFull' OR "
		"Name = 'EMRColorLocked' OR "
		"Name = 'EmnPopupAutoDismiss' OR "
		"Name = 'NxInkPicture_ShrinkToFitArea' OR " // (a.walling 2010-05-19 11:21) - PLID 38778 - Option to shrink to fit the display area
		"Name = 'EMR_AlignSliderValueBottom' OR "
		"Name = 'EMR_AlignSliderValueRight' OR " // (a.walling 2007-08-15 15:56) - PLID 27083 - Added the Right as well.
		"Name = 'EmnTableDropdownAutoAdvance' OR "	// (j.jones 2008-06-05 09:17) - PLID 18529
		"Name = 'EMR_AddSmartStampOnHotSpot' OR "	// (j.jones 2010-02-18 10:10) - PLID 37423
		"Name = 'EMNChargesAllowQtyIncrement' OR " // (j.jones 2010-09-22 08:12) - PLID 24221
		"Name = 'ChargeAllowQtyIncrement' "
		// (j.jones 2011-04-07 09:29) - PLID 43166 - added EMR_SkipLinkingDiagsToNonBillableCPTs, and a couple other missing prefs.
		"OR Name = 'EMR_SkipLinkingDiagsToNonBillableCPTs' "
		"OR Name = 'EMR_AutoLinkSpawnedDiagCodesToCharges' "
		"OR Name = 'EMR_SpawnedDiagCodes_NoLinkPrompt' "
		"OR Name = 'EMR_LinkSpawnedChargesDiagsByTopic' "
		// (r.gonet 05/27/2011) - PLID 43542 - Bulk cache enable text scaling property
		"OR Name = 'EnableEMRImageTextScaling' "
		"OR Name = 'EMRLoadEMNsSortOrder' "	// (j.jones 2012-07-31 17:06) - PLID 51750
		"OR Name = 'EMRDrugInteractionChecks' "		// (j.jones 2012-09-27 09:27) - PLID 52820
		"OR Name = 'EMRRememberPoppedUpTableColumnWidths' " // (r.gonet 02/14/2013) - PLID 40017 -
		"OR Name = 'DisableSmartStampTableDropdownStampFilters' "	// (j.jones 2013-03-07 12:41) - PLID 55511
		// (j.jones 2013-05-08 13:27) - PLID 44634 - added more options for linking diags/charges, also cached two that were previously missed
		"OR Name = 'EMR_SpawnedDiagCodes_AutoLinkPrompt' "
		"OR Name = 'EMR_SpawnedCharges_AutoLinkPrompt' "
		"OR Name = 'EMR_LinkSpawnedChargesDiagsByTopic_Prompt' "
		"OR Name = 'EMR_AutoLinkSpawnedChargesToDiagCodes' "
		"OR Name = 'EMR_SpawnedCharges_NoLinkPrompt' "
		"OR Name = 'RequireCPTCodeEMNLocking' " // (d.singleton 2013-07-24 16:40) - PLID 44840
		"OR Name = 'RequireDiagCodeEMNLocking' " // (d.singleton 2013-07-24 16:40) - PLID 44840
		")",
		_Q(GetCurrentUserName()));

	m_wndEmrTree.SetIsTemplate(TRUE); // (c.haag 2007-08-30 16:14) - PLID 27256 - Make sure the tree knows that this is
									// a template so that it won't create buttons not used in templates
}

void CEmrTemplateEditorDlg::LoadEMRObject()
{
	// (j.jones 2007-10-01 09:25) - PLID 27562 - added try/catch
	try {

		CEmrTemplateFrameWnd* pFrameWnd = polymorphic_downcast<CEmrTemplateFrameWnd*>(GetTopLevelFrame());

		if (!pFrameWnd) {
			return;
		}

		CWaitCursor pWait;

		//the SetTemplate function will load the template ID as an EMN
		m_wndEmrTree.SetTemplate(pFrameWnd->m_nEmrTemplateID);
		if(pFrameWnd->m_nEmrTemplateID == -1) {
			//new template, so add a new blank EMN
			m_wndEmrTree.ShowEMN(m_wndEmrTree.AddNewTemplateEMN(pFrameWnd->m_nNewTemplateCollectionID));
		}
		else 
			//DRT 2/9/2006 - PLID 19178 - We need to set the topic in the editor, not rely
			//	on the tree wnd to do it.
			m_wndEmrTree.ShowEMN(pFrameWnd->m_nEmrTemplateID);

	}NxCatchAll("Error in CEmrTemplateEditorDlg::LoadEMRObject()");
}
//
//void CEmrTemplateEditorDlg::OnCloseTemplate() 
//{
//	try {
//		// (c.haag 2007-08-30 16:57) - PLID 27058 - Force the user to close the checklist setup dialog before closing the template
//		if (m_wndEmrTree.IsEMREMChecklistDlgVisible()) {
//			MessageBox("Please close the E/M Checklist Setup dialog before dismissing this template.", "Practice", MB_OK | MB_ICONSTOP);
//			m_wndEmrTree.BringEMREMChecklistDlgToTop();
//			return;
//		}
//
//		if(m_wndEmrTree.IsEMRUnsaved()) {
//			int nResult = MessageBox("You have made changes to the template, do you wish to save these changes?\n\n"
//				"'Yes' will save the changes and close the template.\n"
//				"'No' will discard the changes and close the template.\n"
//				"'Cancel' will cancel closing the template.", "Practice", MB_ICONEXCLAMATION|MB_YESNOCANCEL);
//
//			if(nResult == IDCANCEL)
//				return;
//			else if(nResult == IDNO) {
//				if(IDYES == MessageBox("All of your template changes will be unrecoverable! Are you sure you wish to close without saving?",
//					"Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
//
//					//they really want to close without saving
//
//					CDialog::OnOK();
//				}
//				
//				return;
//			}
//
//			//otherwise, keep going, save and close
//		}
//
//		if(Save() != essSuccess)
//			return;
//		else
//			CDialog::OnOK();
//
//	}NxCatchAll("Error saving EMR.");
//}
//
//void CEmrTemplateEditorDlg::OnBtnOkTemplate() 
//{
//	try {
//		// (c.haag 2007-08-30 16:57) - PLID 27058 - Force the user to close the checklist setup dialog before closing the template
//		if (m_wndEmrTree.IsEMREMChecklistDlgVisible()) {
//			MessageBox("Please close the E/M Checklist Setup dialog before dismissing this template.", "Practice", MB_OK | MB_ICONSTOP);
//			m_wndEmrTree.BringEMREMChecklistDlgToTop();
//			return;
//		}
//
//		if(Save() != essSuccess)
//			return;
//		else
//			CDialog::OnOK();
//
//	}NxCatchAll("Error saving EMR.");
//}
//
//void CEmrTemplateEditorDlg::OnBtnCancelTemplate() 
//{
//	try {
//		// (c.haag 2007-08-30 16:57) - PLID 27058 - Force the user to close the checklist setup dialog before closing the template
//		if (m_wndEmrTree.IsEMREMChecklistDlgVisible()) {
//			MessageBox("Please close the E/M Checklist Setup dialog before dismissing this template.", "Practice", MB_OK | MB_ICONSTOP);
//			m_wndEmrTree.BringEMREMChecklistDlgToTop();
//			return;
//		}
//
//		if(m_wndEmrTree.IsEMRUnsaved()) {
//			if(IDNO == MessageBox("You have made changes to the template, do you wish to cancel without saving these changes?",
//				"Practice", MB_ICONEXCLAMATION|MB_YESNO)) { 
//
//				return;
//			}
//		}
//
//		CDialog::OnOK();
//
//	}NxCatchAll("Error cancelling EMR.");
//}
//
