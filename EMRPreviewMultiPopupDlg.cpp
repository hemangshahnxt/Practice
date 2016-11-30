// EMRPreviewMultiPopupDlg.cpp : implementation file
//

// (a.walling 2009-11-30 10:56) - PLID 24194 - This dialog simply provides a list of EMN and related info and allows the user to print multiple EMNs.

#include "stdafx.h"
#include "EmrRc.h"
#include "EMRPreviewMultiPopupDlg.h"
#include "NxCDO.h"
#include <mshtmcid.h>
#include "AuditTrail.h"
// (b.savon 2011-11-10 11:02) - PLID 25782 - Files needed for printing complete medical history
#include "FileUtils.h"
#include "HistoryUtils.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "EMN.h"
#include "EMR.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CEMRPreviewMultiPopupDlg dialog

IMPLEMENT_DYNAMIC(CEMRPreviewMultiPopupDlg, CNxDialog)

CEMRPreviewMultiPopupDlg::CEMRPreviewMultiPopupDlg(long nPatientID, CWnd* pParent) // must use a parent
	: CNxDialog(CEMRPreviewMultiPopupDlg::IDD, pParent)
{
	m_bReady = false;
	m_bPrinting = false;
	m_bPrintPreview = false;
	m_pEMRPreviewCtrlDlg = NULL;

	// (b.savon 2011-11-10 10:11) - PLID 25782 - Initialize
	m_nPatientID = nPatientID;
	m_bPrintTemplateTeardownFired = FALSE;
	m_bPrintEntireHistory = FALSE;
}

// (a.walling 2009-11-23 12:44) - PLID 36404 - Get notified when the printing is complete
BEGIN_MESSAGE_MAP(CEMRPreviewMultiPopupDlg, CNxDialog)
	ON_WM_DESTROY()
	// (a.walling 2010-01-12 08:38) - PLID 36840 - Changed to defined messages with EMRPREVIEW_ rather than a registered message
	ON_MESSAGE(NXM_EMRPREVIEW_DOCUMENT_COMPLETE, CEMRPreviewMultiPopupDlg::OnDocumentComplete)
	ON_MESSAGE(NXM_EMRPREVIEW_PRINT_COMPLETE, CEMRPreviewMultiPopupDlg::OnPrintComplete)
	ON_BN_CLICKED(IDC_BUTTON_PRINT, CEMRPreviewMultiPopupDlg::OnBtnPrint)
	ON_BN_CLICKED(IDC_BUTTON_PREVIEW, CEMRPreviewMultiPopupDlg::OnBtnPrintPreview)
	ON_BN_CLICKED(IDC_CHECK_PRINT_REVERSE, CEMRPreviewMultiPopupDlg::OnCheckPrintReverse)
	ON_BN_CLICKED(IDC_BUTTON_PRINT_ALL, &CEMRPreviewMultiPopupDlg::OnBnClickedButtonPrintAll)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEMRPreviewMultiPopupDlg, CNxDialog)
	ON_EVENT(CEMRPreviewMultiPopupDlg, IDC_EMRPREVIEW_MULTI_DATALIST, 19, CEMRPreviewMultiPopupDlg::OnLeftClickList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

CEMRPreviewMultiPopupDlg::~CEMRPreviewMultiPopupDlg()
{
	POSITION pos = m_listEMNs.GetHeadPosition();

	while (pos) {
		CEMNPointer& emn(m_listEMNs.GetNext(pos));

		if (emn.bDelete && emn.pEMN) {
			delete emn.pEMN;
		}
	}
	m_listEMNs.RemoveAll();
	
	if(m_pEMRPreviewCtrlDlg) {
		ASSERT(FALSE);
		m_pEMRPreviewCtrlDlg->DestroyWindow();
		delete m_pEMRPreviewCtrlDlg;
		m_pEMRPreviewCtrlDlg = NULL;
	}

	CleanupTempFiles();
}

void CEMRPreviewMultiPopupDlg::OnDestroy() 
{
	try {
		if(m_pEMRPreviewCtrlDlg) {
			m_pEMRPreviewCtrlDlg->DestroyWindow();
			delete m_pEMRPreviewCtrlDlg;
			m_pEMRPreviewCtrlDlg = NULL;
		}

		CleanupTempFiles();
	} NxCatchAll("Error in CEMRPreviewMultiPopupDlg::OnDestroy");

	CDialog::OnDestroy();
}

void CEMRPreviewMultiPopupDlg::CleanupTempFiles()
{
	if (!m_strPrintTemplatePath.IsEmpty()) {
		DeleteFileWhenPossible(m_strPrintTemplatePath);
		m_strPrintTemplatePath.Empty();
	}

	for (int i = 0; i < m_arTempFiles.GetSize(); i++) {
		DeleteFileWhenPossible(m_arTempFiles.GetAt(i));
	}

	m_arTempFiles.RemoveAll();
}

void CEMRPreviewMultiPopupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PREVIEW_AREA_MULTI, m_nxstaticPreviewArea);
	DDX_Control(pDX, IDC_CHECK_PRINT_REVERSE, m_checkPrintReverse);
	DDX_Control(pDX, IDC_BUTTON_PREVIEW, m_nxbPrintPreview);
	DDX_Control(pDX, IDC_BUTTON_PRINT, m_nxbPrint);
	DDX_Control(pDX, IDC_BUTTON_PRINT_ALL, m_nxbPrintAll);
	DDX_Control(pDX, IDCANCEL, m_nxbClose);
	DDX_Control(pDX, IDC_EMRPREVIEW_MULTI_NXCOLOR, m_nxcolor);
}


// CEMRPreviewMultiPopupDlg message handlers
BOOL CEMRPreviewMultiPopupDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		m_nxcolor.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		m_nxbPrint.EnableWindow(FALSE);
		m_nxbPrintAll.EnableWindow(FALSE);
		m_nxbPrintPreview.EnableWindow(FALSE);
		m_checkPrintReverse.EnableWindow(FALSE);

		m_nxbPrint.AutoSet(NXB_PRINT);
		m_nxbPrintAll.AutoSet(NXB_PRINT);
		m_nxbPrintPreview.AutoSet(NXB_PRINT_PREV);
		m_nxbClose.AutoSet(NXB_CLOSE);

		// (c.haag 2013-02-28) - PLID 55373 - We can now display this window with just one EMN in it. In those cases,
		// we need to hide the reverse printing because it makes no sense. Its value will not have any effect on actual
		// printing order.
		if (m_listEMNs.GetCount() <= 1) {
			m_checkPrintReverse.ShowWindow(SW_HIDE);
		}
		m_checkPrintReverse.SetCheck(GetRemotePropertyInt("PrintMultiEMNsInReverse", FALSE, 0, GetCurrentUserName(), true) ? BST_CHECKED : BST_UNCHECKED);

		m_pList = BindNxDataList2Ctrl(IDC_EMRPREVIEW_MULTI_DATALIST, false);
		
		if (m_pEMRPreviewCtrlDlg == NULL) {
			CRect rRect;
			// (j.jones 2009-09-22 11:22) - PLID 31620 - use IDC_PREVIEW_AREA,
			// not the full client area
			GetDlgItem(IDC_PREVIEW_AREA_MULTI)->GetWindowRect(rRect);
			ScreenToClient(rRect);

			m_pEMRPreviewCtrlDlg = new CEMRPreviewCtrlDlg;
			m_pEMRPreviewCtrlDlg->SetInteractive(FALSE);
			// (a.walling 2012-11-05 11:58) - PLID 53588 - CEMRPreviewCtrlDlg now expecting a control ID
			m_pEMRPreviewCtrlDlg->Create(rRect, this, -1);
			m_pEMRPreviewCtrlDlg->EnsureCSSFile();
		}

		long nValidInputDateCount = 0;

		// (c.haag 2013-02-28) - PLID 55373 - Did we get the custom preview layouts yet? If not, do it now.
		if (NULL == m_pCustomPreviewLayouts)
		{
			NexTech_Accessor::_EMRCustomPreviewLayoutFilterPtr pFilter(__uuidof(NexTech_Accessor::EMRCustomPreviewLayoutFilter)); 
			NexTech_Accessor::_EMRCustomPreviewLayoutOptionsPtr pOptions(__uuidof(NexTech_Accessor::EMRCustomPreviewLayoutOptions));
			Nx::SafeArray<BSTR> saEmnIDs;
			{
				POSITION pos = m_listEMNs.GetHeadPosition();
				while (pos) 
				{
					CEMNPointer& emn(m_listEMNs.GetNext(pos));
					saEmnIDs.Add(_bstr_t(emn.nID));
				}
			}
			if (saEmnIDs.GetCount() > 0) // Not that I'd ever expect it to be 0...but check anyway
			{
				pFilter->EmnIDs = saEmnIDs;
				pOptions->IncludeData = VARIANT_TRUE;
				//	Create our SAFEARRAY to be passed to the function in the API
				Nx::SafeArray<IUnknown *> saFilters = Nx::SafeArray<IUnknown *>::FromValue(pFilter);
				m_pCustomPreviewLayouts = GetAPI()->GetEMRCustomPreviewLayouts(
					GetAPISubkey(), GetAPILoginToken(), saFilters, pOptions);		
			}
		}

		// (c.haag 2013-02-28) - PLID 55373 - There are three ways we can populate the tree:
		// 1. One emn: When there's only one EMN, we make each EMN/layout combination its own root-level row
		// 2. Multiple emn's, no layouts: When there are multiple EMN's, we make each EMN/layout combination its own root-level row
		// 3. Multiple emn's, multiple layouts: When there are multiple EMN's and there is more than one layout betwixt them all, then
		// we have to do a tree structure.
		Nx::SafeArray<IUnknown *> saryLayouts(m_pCustomPreviewLayouts->Layouts);
		if (m_listEMNs.GetCount() <= 1)
		{
			// 1. One or less EMN's (not that there should ever be 0). All root nodes must be EMN's.
			PopulateTreeWithEMNRootNodes();
		}
		else if (saryLayouts.GetCount() == 0)
		{
			// 2. Multple EMN's with no layouts. All root nodes must be EMN's.
			PopulateTreeWithEMNRootNodes();
		}
		else
		{
			// Multiple EMN's with at least one custom preview layout are always grouped by layout. Here we need a full tree structure.
			PopulateTreeWithLayoutRootNodes();
		}

		NavigateToMessage("Please choose which EMNs to print.");

	} NxCatchAll("Error in CEMRPreviewMultiPopupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (c.haag 2013-02-28) - PLID 55373 - Populate the tree with just EMN's. There will be no child nodes; this is how we used to populate before this PL item.
void CEMRPreviewMultiPopupDlg::PopulateTreeWithEMNRootNodes()
{
	POSITION pos = m_listEMNs.GetHeadPosition();
	int nValidInputDateCount = 0;
	while (pos) 
	{
		CEMNPointer& emn(m_listEMNs.GetNext(pos));
		Nx::SafeArray<IUnknown *> saryLayouts(m_pCustomPreviewLayouts->Layouts);
		// Add the default layout
		NXDATALIST2Lib::IRowSettingsPtr pEMNRow = CreateEMNTreeRow(emn, NULL, nValidInputDateCount);
		// (c.haag 2013-02-28) - PLID 55373 - Add rows sorted by EMN date descending, then EMN description asc, then layout name asc
		m_pList->AddRowSorted(pEMNRow, NULL);
		// Now add all the other layouts
		if (saryLayouts.GetCount() > 0)
		{
			pEMNRow->PutValue(lcLayoutName, " < Default > ");
			foreach(NexTech_Accessor::_EMRCustomPreviewLayoutPtr pLayout, saryLayouts)
			{
				pEMNRow = CreateEMNTreeRow(emn, pLayout, nValidInputDateCount);
				pEMNRow->PutValue(lcLayoutName, pLayout->Name);
				// (c.haag 2013-02-28) - PLID 55373 - Add rows sorted by EMN date descending, then EMN description asc, then layout name asc
				m_pList->AddRowSorted(pEMNRow, NULL);
			}
			// Expand the layout name column
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pList->GetColumn(lclLayoutName);
			pCol->PutStoredWidth(150);
		}
	}

	if (nValidInputDateCount == 0) {
		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pList->GetColumn(lcEMNInputDate);
		if (pCol) {
			pCol->PutStoredWidth(0);
			pCol->PutColumnStyle(NXDATALIST2Lib::csFixedWidth);
		}
	}
}

// (c.haag 2013-02-28) - PLID 55373 - Populate the tree such that the root nodes are layouts and all children are EMN's.
void CEMRPreviewMultiPopupDlg::PopulateTreeWithLayoutRootNodes()
{
	Nx::SafeArray<IUnknown *> saryLayouts(m_pCustomPreviewLayouts->Layouts);
	int nValidInputDateCount = 0;

	// Add the default layout
	PopulateLayoutNode(NULL, nValidInputDateCount);

	// Do for all layouts
	foreach(NexTech_Accessor::_EMRCustomPreviewLayoutPtr pLayout, saryLayouts)
	{
		PopulateLayoutNode(pLayout, nValidInputDateCount);
	}

	if (nValidInputDateCount == 0) {
		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pList->GetColumn(lcEMNInputDate);
		if (pCol) {
			pCol->PutStoredWidth(0);
			pCol->PutColumnStyle(NXDATALIST2Lib::csFixedWidth);
		}
	}

	// Expand the layout name column
	NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pList->GetColumn(lclLayoutName);
	pCol->PutStoredWidth(150);
}

// (c.haag 2013-02-28) - PLID 55373 - Called by PopulateTreeWithLayoutRootNodes for eachlayout
void CEMRPreviewMultiPopupDlg::PopulateLayoutNode(NexTech_Accessor::_EMRCustomPreviewLayoutPtr pLayout, int& nValidInputDateCount)
{
	// Do for all EMN's
	POSITION pos = m_listEMNs.GetHeadPosition();
	while (pos) 
	{
		CEMNPointer& emn(m_listEMNs.GetNext(pos));
		if (NULL == pLayout || emn.nTemplateID == atol(pLayout->EMRTemplateID))
		{
			NXDATALIST2Lib::IRowSettingsPtr pEMNRow = CreateEMNTreeRow(emn, pLayout, nValidInputDateCount);
			pEMNRow->PutValue(lcLayoutName, (NULL == pLayout) ? " < Default > " : pLayout->Name);
			// (c.haag 2013-02-28) - PLID 55373 - Add rows sorted by EMN date descending, then EMN description asc, then layout name asc
			m_pList->AddRowSorted(pEMNRow, NULL);
		}
	}
}

// (c.haag 2013-02-28) - PLID 55373 - Creates an EMN row in the tree
NXDATALIST2Lib::IRowSettingsPtr CEMRPreviewMultiPopupDlg::CreateEMNTreeRow(CEMNPointer& emn, NexTech_Accessor::_EMRCustomPreviewLayoutPtr pLayout, int& nValidInputDateCount)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetNewRow();
	pRow->PutValue(lcEMNPointer, _variant_t((long)&emn));
	// (c.haag 2013-09-03) - PLID 57610 - Checkboxes should default to unchecked
	pRow->PutValue(lcCheck, g_cvarFalse);
	if (NULL != pLayout) {
		pRow->PutValue(lclLayoutID, pLayout->ID);
	}
	pRow->PutValue(lcEMNDate, _variant_t(emn.dtDate, VT_DATE));
	if (emn.dtInputDate.GetStatus() == COleDateTime::valid) {
		pRow->PutValue(lcEMNInputDate, _variant_t(emn.dtInputDate, VT_DATE));
		nValidInputDateCount++;
	} else {
		pRow->PutValue(lcEMNInputDate, g_cvarNull);
	}
	pRow->PutValue(lcEMNTitle, (LPCTSTR)emn.strEMNTitle);
	pRow->PutValue(lcEMRTitle, (LPCTSTR)emn.strEMRTitle);
	return pRow;
}

// (c.haag 2013-02-28) - PLID 55373 - Creates a layout row in the tree
NXDATALIST2Lib::IRowSettingsPtr CEMRPreviewMultiPopupDlg::CreateLayoutTreeRow(NexTech_Accessor::_EMRCustomPreviewLayoutPtr pLayout)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetNewRow();
	pRow->PutValue(lclLayoutID, (NULL == pLayout) ? g_cvarNull : pLayout->ID);
	pRow->PutValue(lclLayoutName, (NULL == pLayout) ? " < Default > " : pLayout->Name);
	return pRow;
}

// (z.manning 2012-09-11 17:51) - PLID 52543 - Added modified date
// (c.haag 2013-02-28) - PLID 55368 - We also need the EMN template ID
void CEMRPreviewMultiPopupDlg::AddAvailableEMN(CEMN* pEMN, long nID, long nTemplateID, const CString& strEMRTitle, const CString& strEMNTitle, COleDateTime dtDate, COleDateTime dtInputDate, COleDateTime dtModifiedDate)
{
	m_listEMNs.AddTail(CEMNPointer(pEMN, nID, nTemplateID, strEMRTitle, strEMNTitle, dtDate, dtInputDate, dtModifiedDate));
}

// (c.haag 2013-02-28) - PLID 55368 - Assigns the custom preview layout list.
void CEMRPreviewMultiPopupDlg::SetCustomPreviewLayoutList(NexTech_Accessor::_EMRCustomPreviewLayoutsPtr pLayouts)
{
	m_pCustomPreviewLayouts = pLayouts;
}

// (c.haag 2013-02-28) - PLID 55368 - Gets the number of custom preview layouts
int CEMRPreviewMultiPopupDlg::GetCustomPreviewLayoutCount()
{
	if (NULL != m_pCustomPreviewLayouts) 
	{
		Nx::SafeArray<IUnknown *> saryLayouts(m_pCustomPreviewLayouts->Layouts);
		return saryLayouts.GetCount();
	}
	return 0;
}

// (a.walling 2009-11-23 12:46) - PLID 36404 - Don't allow exit until printing complete
void CEMRPreviewMultiPopupDlg::OnCancel()
{
	try {
		// (a.walling 2015-07-09 16:33) - PLID 66504 - Allow users to ignore and continue if the print template teardown did not fire properly
		long nRet = IDTRYAGAIN;
		while (m_bPrinting && nRet == IDTRYAGAIN) {
			nRet = MessageBox("Please wait for the current print job to complete and be sent to the spooler before exiting. This should complete in less than a minute.", nullptr, MB_CANCELTRYCONTINUE | MB_ICONEXCLAMATION);
			if (nRet == IDCANCEL) {
				return;
			}
		}
	} NxCatchAll(__FUNCTION__);

	CNxDialog::OnCancel();
}

void CEMRPreviewMultiPopupDlg::OnBtnPrint()
{
	try {
		m_bPrintPreview = false;

		PrintAll();
	} NxCatchAll("Could not print");
}

void CEMRPreviewMultiPopupDlg::OnBtnPrintPreview()
{
	try {
		m_bPrintPreview = true;

		PrintAll();
	} NxCatchAll("Could not print preview");
}

void CEMRPreviewMultiPopupDlg::OnCheckPrintReverse()
{
	try {
		SetRemotePropertyInt("PrintMultiEMNsInReverse", m_checkPrintReverse.GetCheck() == BST_CHECKED ? TRUE : FALSE, 0, GetCurrentUserName());
	} NxCatchAll(__FUNCTION__);	
}

void CEMRPreviewMultiPopupDlg::OnLeftClickList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if (nCol == lcCheck) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			_variant_t varSelected = pRow->GetValue(lcCheck);
			_variant_t varNewSelected;

			if (varSelected.vt == VT_BOOL && VarBool(varSelected)) {
				varNewSelected = _variant_t(VARIANT_FALSE, VT_BOOL);
			} else {
				varNewSelected = _variant_t(VARIANT_TRUE, VT_BOOL);
			}

			pRow->PutValue(lcCheck, varNewSelected);
		}
	} NxCatchAll(__FUNCTION__);
}

void CEMRPreviewMultiPopupDlg::NavigateToMessage(CString strMessage)
{
	try {
		CString strFull;
		strFull.Format("<body style='background: #FFFBFB; padding: 0px; margin: 0px; overflow: hidden; font-family: Calibri, Arial, Verdana, Times New Roman, Book Antiqua, Palatino, Serif; font-size: 12pt;'>"
			"<table border=0 cellpadding=0 cellspacing=0 width=100%% height=100%%><tr><td style='padding:5px;'>"
			"%s"
			"</td></tr></table>"
			"</body>", strMessage);

		// (a.walling 2007-10-01 11:43) - PLID 25648 - Safely navigate to HTML text rather than rely on temp
		// files. This is much faster too.
		if (m_pEMRPreviewCtrlDlg) {
			m_pEMRPreviewCtrlDlg->SafeNavigateToHTML(strFull); 
		}
	} NxCatchAll("Error in CEMRPreviewMultiPopupDlg::NavigateToMessage");
}


LRESULT CEMRPreviewMultiPopupDlg::OnDocumentComplete(WPARAM wParam, LPARAM lParam)
{
	try {
		if (!m_bReady) {
			m_bReady = true;
			m_nxbPrint.EnableWindow(TRUE);
			m_nxbPrintAll.EnableWindow(TRUE);
			m_nxbPrintPreview.EnableWindow(TRUE);
			m_checkPrintReverse.EnableWindow(TRUE);
		}
	} NxCatchAll(__FUNCTION__);
	return 0;
}

// (a.walling 2009-11-23 12:44) - PLID 36404
LRESULT CEMRPreviewMultiPopupDlg::OnPrintComplete(WPARAM wParam, LPARAM lParam)
{
	try {
		// (b.savon 2011-11-10 16:35) - PLID 25782 - This will fire multiple times when printing the
		// full patient history if they have more than 1 EMN.  Let's make sure the GUI isn't updated
		// until all our docs, labs, and report are finished sending to the printer.
		if( !m_bPrintEntireHistory ){
			m_bPrinting = false;
			m_nxbClose.EnableWindow(TRUE);
			m_nxbPrint.EnableWindow(TRUE);
			m_nxbPrintAll.EnableWindow(TRUE);
			m_nxbPrintPreview.EnableWindow(TRUE);
			m_checkPrintReverse.EnableWindow(TRUE);	
			m_pList->Enabled = VARIANT_TRUE;
		}

		// (b.savon 2011-11-10 16:36) - PLID - Set flag to break message loop
		m_bPrintTemplateTeardownFired = TRUE;

		// (a.walling 2009-12-02 10:10) - PLID 24194 - Force a redraw of the browser
		m_pEMRPreviewCtrlDlg->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ERASENOW | RDW_ALLCHILDREN);
	} NxCatchAll(__FUNCTION__);
	return 0;
}

void CEMRPreviewMultiPopupDlg::PrintAll()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pFirstSelectedRow = m_pList->FindByColumn(lcCheck, g_cvarTrue, NULL, VARIANT_FALSE);
		// (a.walling 2009-12-07 15:57) - PLID 24194 - Beware = when you intend to say ==
		if (NULL == pFirstSelectedRow) {
			MessageBox("No EMNs have been selected to print!", NULL, MB_ICONSTOP);
			return;
		}
		
		m_bPrinting = true;

		m_nxbClose.EnableWindow(FALSE);
		m_nxbPrint.EnableWindow(FALSE);
		m_nxbPrintAll.EnableWindow(FALSE);
		m_nxbPrintPreview.EnableWindow(FALSE);
		m_checkPrintReverse.EnableWindow(FALSE);
		m_pList->Enabled = VARIANT_FALSE;

		PrepareEMNs();
		m_strPrintTemplatePath = GetMultiPrintTemplate();

		// now let's try printing!
		try {
			IOleCommandTargetPtr pCmdTarg = m_pEMRPreviewCtrlDlg->GetOleCommandTarget();

			if (pCmdTarg) {
				_variant_t varTemplate = _bstr_t(m_strPrintTemplatePath);
				pCmdTarg->Exec(&CGID_MSHTML, 
						m_bPrintPreview ? IDM_PRINTPREVIEW : IDM_PRINT, 
						OLECMDEXECOPT_PROMPTUSER, 
						&varTemplate,
						NULL);
			} else {
				ThrowNxException("Could not send print command.");
			}

		} NxCatchAllThrow("Printing failed");

	} NxCatchAllCallThrow("CEMRPreviewMultiPopupDlg could not print all EMNs!", 
		{
			m_bPrinting = false;
			m_nxbClose.EnableWindow(TRUE);
			m_nxbPrint.EnableWindow(TRUE);
			m_nxbPrintPreview.EnableWindow(TRUE);
			m_checkPrintReverse.EnableWindow(TRUE);	
			m_pList->Enabled = VARIANT_TRUE;

			// (a.walling 2009-12-02 10:10) - PLID 24194 - Force a redraw of the browser
			m_pEMRPreviewCtrlDlg->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ERASENOW | RDW_ALLCHILDREN);
		}
	);	
}

void CEMRPreviewMultiPopupDlg::PrepareEMNs()
{
	try {
		CWaitCursor cws;

		CEMRPreviewCtrlDlg::EnsureCSSFile();

		// (c.haag 2013-02-28) - PLID 55373 - In this function we generate all of the default and custom previews for an EMN,
		// and store them in the tree. It would be as simple as going from top to bottom; but it's not that simple because we 
		// have to batch custom preview loads for EMN's.
		POSITION pos = m_listEMNs.GetHeadPosition();

		// Do for all EMNs
		while (pos) 
		{
			CEMNPointer& emn(m_listEMNs.GetNext(pos));
			Nx::SafeArray<BSTR> saCustomPreviewLayoutIDs; // All the custom preview layouts for this EMN that need to be used in preview generation

			// Iterate through the entire tree from top to bottom searching for this EMN in all the nodes, and prepare each one. By the time this loop is finished,
			// we will have the list of all custom preview layouts to use for this EMN's preview; and we will have also generated its default Practice preview as well
			// if necessary.
			for (NXDATALIST2Lib::IRowSettingsPtr pRootRow = m_pList->GetFirstRow(); NULL != pRootRow; pRootRow = pRootRow->GetNextRow())
			{
				PrepareEMN(emn, pRootRow, saCustomPreviewLayoutIDs);
			}

			if (saCustomPreviewLayoutIDs.GetCount() > 0)
			{
				// If we found any layouts, generate the custom previews.
				NexTech_Accessor::_EMRCustomPreviewLayoutFilterPtr pFilter(__uuidof(NexTech_Accessor::EMRCustomPreviewLayoutFilter)); 
				pFilter->IDs = saCustomPreviewLayoutIDs;
				Nx::SafeArray<IUnknown *> saFilters = Nx::SafeArray<IUnknown *>::FromValue(pFilter);
				NexTech_Accessor::_EMRCustomPreviewsPtr pPreviews = GetAPI()->GenerateEMRCustomPreviews(GetAPISubkey(), GetAPILoginToken(), _bstr_t(emn.nID), saFilters);
				Nx::SafeArray<IUnknown *> saryPreviews(pPreviews->Previews);
				// Now assign the previews to the tree rows
				foreach(NexTech_Accessor::_EMRCustomPreviewPtr pPreview, saryPreviews)
				{
					for (NXDATALIST2Lib::IRowSettingsPtr pRootRow = m_pList->GetFirstRow(); NULL != pRootRow; pRootRow = pRootRow->GetNextRow())
					{
						AssignCustomPreview(emn, pRootRow, pPreview);
					}
				}
			}
		}

	} NxCatchAllThrow(__FUNCTION__);
}

// (c.haag 2013-02-28) - PLID 55373 - Prepares a single EMN for printing and generates its default EMN preview if necessary
void CEMRPreviewMultiPopupDlg::PrepareEMN(CEMNPointer& emn, NXDATALIST2Lib::IRowSettingsPtr pRow, Nx::SafeArray<BSTR>& saCustomPreviewLayoutIDs)
{
	CEMNPointer* p = (CEMNPointer*)VarLong(pRow->GetValue(lcEMNPointer), 0);
	if (p->nID == emn.nID)
	{
		_variant_t vLayoutID = pRow->GetValue(lclLayoutID);
		if (VT_EMPTY == vLayoutID.vt || VT_NULL == vLayoutID.vt) {
			// If the layout is NULL, that means we use the default EMN preview
			CString strDefaultPreview = GetEMNDefaultPreview(emn);
			pRow->PutValue(lcDocumentPath, _bstr_t(GetEMNDefaultPreview(emn)));
		} else {
			// If the layout is not NULL, add it to the layout array so that we can generate all the layouts in one trip
			saCustomPreviewLayoutIDs.Add(VarString(vLayoutID));
		}
	}
}

// (c.haag 2013-02-28) - PLID 55373 - Called after custom previews are generated to assign each custom preview to its respective row in the tree
void CEMRPreviewMultiPopupDlg::AssignCustomPreview(CEMNPointer& emn, NXDATALIST2Lib::IRowSettingsPtr pRow, NexTech_Accessor::_EMRCustomPreviewPtr pPreview)
{
	CEMNPointer* p = (CEMNPointer*)VarLong(pRow->GetValue(lcEMNPointer), 0);
	if (p->nID == emn.nID)
	{
		_variant_t vLayoutID = pRow->GetValue(lclLayoutID);
		if (VT_EMPTY != vLayoutID.vt && VT_NULL != vLayoutID.vt && 0 == VarString(vLayoutID).Compare((LPCTSTR)pPreview->CustomPreviewLayoutID)) 
		{
			CString strDocumentPath;
			CString strData = (LPCTSTR)pPreview->data;
			HANDLE hFile = CreateNxTempFile(FormatString("nexemrt_EMNCustomPreview_%d_%s", emn.nID, (LPCTSTR)pPreview->CustomPreviewLayoutID), "html", &strDocumentPath);
			if (hFile && hFile != INVALID_HANDLE_VALUE) {
				m_arTempFiles.Add(strDocumentPath);
			} else {
				ThrowNxException("Failed to create or access temp file for the EMR Preview!");
			}
			DWORD dwBytesWritten;
			WriteFile(hFile, (LPCTSTR)strData, strData.GetLength(), &dwBytesWritten, NULL);
			CloseHandle(hFile);
			pRow->PutValue(lcDocumentPath, (LPCTSTR)strDocumentPath);

			// (c.haag 2013-03-08) - PLID 55522 - Now generate images
			Nx::SafeArray<IUnknown *> saryCustomPreviewRequests(pPreview->ImageRequests);
			if (saryCustomPreviewRequests.GetCount() > 0)
			{
				Nx::SafeArray<IUnknown *> sarySummaryRequests;
				foreach(NexTech_Accessor::_EMNCustomPreviewImageRequestPtr pCustomPreviewRequest, saryCustomPreviewRequests)
				{
					NexTech_Accessor::_EMNSummaryImageRequestPtr pSummaryRequest(__uuidof(NexTech_Accessor::EMNSummaryImageRequest));
					pSummaryRequest->detailID = pCustomPreviewRequest->detailID;
					pSummaryRequest->ImageFilename = pCustomPreviewRequest->ImageFilename;
					sarySummaryRequests.Add(pSummaryRequest);
				}
				NexTech_Accessor::_EMNSummaryImagesPtr pSummaryImages = GetAPI()->GetEMNSummaryImages(GetAPISubkey(), GetAPILoginToken(), sarySummaryRequests);
				Nx::SafeArray<IUnknown *> saryImages = pSummaryImages->Images;
				if (saryImages.GetCount() > 0)
				{
					foreach(NexTech_Accessor::_EMNSummaryImagePtr pImage, saryImages)
					{
						Nx::SafeArray<BYTE> saBits = pImage->Bits;
						CString strImageFilename = FileUtils::GetFilePath(strDocumentPath) ^ (LPCTSTR)pImage->ImageFilename;
						HANDLE hFile = CreateFile(strImageFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
						if (hFile && hFile != INVALID_HANDLE_VALUE) {
							m_arTempFiles.Add(strImageFilename);
						} else {
							ThrowNxException("Failed to create or access temp file for the EMR Preview!");
						}
						DWORD dwBytesWritten;
						WriteFile(hFile, saBits.begin(), saBits.GetSize(), &dwBytesWritten, NULL);
						CloseHandle(hFile);
					}
				}							
			}
		}
	}
}

// (c.haag 2013-02-28) - PLID 55373 - Returns the full path to the generated default preview of an EMN
CString CEMRPreviewMultiPopupDlg::GetEMNDefaultPreview(CEMNPointer& emn)
{
	CString strDocumentPath;
	if (emn.pEMN == NULL) {
		// load it
		if (emn.nID != -1) {
			// first, let's see if we can get the MHT file
			
			CString strTempDecryptedFile;

			bool bExtractedFromMHT = false;
			
			// Print template doesn't work with MHTML files -- dang.
			try {
				// (a.walling 2009-11-23 11:47) - PLID 36396 - Shared method to decrypt a saved EMN Preview and return the path to the decrypted MHT file
				if (CEMRPreviewCtrlDlg::GetMHTFile(emn.nID, emn.dtModifiedDate, strTempDecryptedFile)) {
					m_arTempFiles.Add(strTempDecryptedFile);

					CString strExtractedHTMLFile;
					// (a.walling 2009-11-23 11:55) - PLID 36396 - Extract an MHTML archive into component parts
					NxCDO::ExtractMHTToPath(strTempDecryptedFile, GetNxTempPath(), strExtractedHTMLFile, m_arTempFiles, "nexemrt_");
					strDocumentPath = strExtractedHTMLFile;
					bExtractedFromMHT = true;
				}
			} NxCatchAll("Could not extract saved preview from MHTML archive."); // will fallback to loading the EMN
			
			if (!bExtractedFromMHT) {
				// nope, let's try to load it

				// (a.walling 2009-12-02 10:00) - PLID 24194 - Pass NULL so the EMN loads (and frees) the EMR itself
				// (this was already passing NULL simply by passing a NULL m_pLocalEMR pointer that was never used)
				CEMN* pLocalEMN = new CEMN(NULL);
				pLocalEMN->LoadFromEmnID(emn.nID);
				emn.pEMN = pLocalEMN;
				emn.bDelete = true;

				// (a.walling 2013-09-05 11:24) - PLID 58369 - GenerateHTMLFile returns path to newly-generated html file
				strDocumentPath = emn.pEMN->GenerateHTMLFile(FALSE, TRUE);
			}
		}
	} else {
		// (a.walling 2013-09-05 11:24) - PLID 58369 - GenerateHTMLFile returns path to newly-generated html file
		strDocumentPath = emn.pEMN->GenerateHTMLFile(FALSE, FALSE); // we already have a pointer; don't put a copy in documents though
	}
	return strDocumentPath;
}

CString CEMRPreviewMultiPopupDlg::GetMultiPrintTemplate()
{
	try {
		CList<NxPrintTemplate::DocInfo, NxPrintTemplate::DocInfo&> listEMNPrintInfo;
		bool bPrintReverse = m_checkPrintReverse.GetCheck() == BST_CHECKED;

		// (d.thompson 2013-11-07) - PLID 59351 - Track the EMNs that we generated a print for, we'll audit them at the end
		std::set<CEMN*> setEMNs;

		// (c.haag 2013-02-28) - PLID 55373 - Have the printing order determine how we traverse the list.
		if (!bPrintReverse)
		{
			// Printing normally. Go from top to bottom in the list
			for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetFirstRow(); NULL != pRow; pRow = pRow->GetNextRow())
			{
				// (d.thompson 2013-11-07) - PLID 59351 - Save the EMNs that we setup to print so we can audit them.
				CEMN *pEMN = GetMultiPrintTemplate(pRow, listEMNPrintInfo);
				if(pEMN && setEMNs.find(pEMN) == setEMNs.end()) {
					setEMNs.insert(pEMN);
				}
			}
		}
		else
		{
			// Printing in reverse. We have to go from bottom to top *by EMN*; and go from top to bottom *for each EMN*. Otherwise
			// each individual document gets their page numbers reversed.
			NXDATALIST2Lib::IRowSettingsPtr pLastEMNRow = NULL;
			CEMNPointer* pLastEMNPointer = NULL;

			// Iterate through the rows from bottom-to-top to search for ranges of rows for the same EMN's. Once we've identified a
			// range of rows, we add them top-to-bottom to the multi print template list. This will result in every EMN but the first
			// one being prepared for printing.
			for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetLastRow(); NULL != pRow; pRow = pRow->GetPreviousRow())
			{
				CEMNPointer* pEmnPointer = (CEMNPointer*)VarLong(pRow->GetValue(lcEMNPointer), 0);
				CEMNPointer& emn(*pEmnPointer);

				// Check whether this is the first iteration. If it is, assign the last-row-anchor
				if (NULL == pLastEMNRow) {
					pLastEMNRow = pRow;
					pLastEMNPointer = (CEMNPointer*)VarLong(pRow->GetValue(lcEMNPointer), 0); 
				}
				else
				{
					// Ok, so this is not the first iteration. See if we've moved to another EMN.
					if (pEmnPointer != pLastEMNPointer)
					{
						// Yes, we moved. Lets prepare all the documents for pLastEMNPointer, and then establish
						// this new EMN we've encountered at pRow as our new last-row-anchor.
						for (NXDATALIST2Lib::IRowSettingsPtr pDocRow = pRow->GetNextRow(); pDocRow != NULL; pDocRow = pDocRow->GetNextRow())
						{
							// (d.thompson 2013-11-07) - PLID 59351 - Save the EMNs that we setup to print so we can audit them.
							CEMN *pEMN = GetMultiPrintTemplate(pDocRow, listEMNPrintInfo);
							if(pEMN && setEMNs.find(pEMN) == setEMNs.end()) {
								setEMNs.insert(pEMN);
							}
							if (pDocRow == pLastEMNRow) {
								break;
							}
						}

						// Now establish this row as the last row of our new EMN.
						pLastEMNRow = pRow;
						pLastEMNPointer = (CEMNPointer*)VarLong(pRow->GetValue(lcEMNPointer), 0); 
					}
				}
			}

			// Now finish up by preparing the first EMN in the list for printing.
			for (NXDATALIST2Lib::IRowSettingsPtr pDocRow = m_pList->GetFirstRow(); pDocRow != NULL; pDocRow = pDocRow->GetNextRow())
			{
				// (d.thompson 2013-11-07) - PLID 59351 - Save the EMNs that we setup to print so we can audit them.
				CEMN *pEMN = GetMultiPrintTemplate(pDocRow, listEMNPrintInfo);
				if(pEMN && setEMNs.find(pEMN) == setEMNs.end()) {
					setEMNs.insert(pEMN);
				}
				if (pDocRow == pLastEMNRow) {
					break;
				}
			}

		}

		// (d.thompson 2013-11-07) - PLID 59351 - We now have a set of unique EMNs that we generated print info for, so go ahead
		//	and audit them.  NOTE:  It is intentionally that always audit on the printing 
		//	"attempt".  If they cancel the print dialog, the printer fails, isn't connected, whatever, we'll still audit that
		//	a print was attempted.
		std::set<CEMN*>::iterator it;
		for(it = setEMNs.begin(); it != setEMNs.end(); ++it)
		{
			CEMN *pEMN = (CEMN*)*it;
			AuditPrinting(pEMN);
		}

		// (a.walling 2009-11-23 12:00) - PLID 36395 - This will prepare and return a print template which will point to our multiple documents
		return m_pEMRPreviewCtrlDlg->PreparePrintTemplate(listEMNPrintInfo);
	} NxCatchAllThrow(__FUNCTION__);
}

// (c.haag 2013-02-28) - PLID 55373 - Overload for building listEMNPrintInfo for one row
// (d.thompson 2013-11-07) - PLID 59351 - Return the EMN that we got the multi print template for.
// (r.farnworth 2015-07-28 09:55) - PLID 64618 - Added bOverrideCheck for when printing entire history
CEMN* CEMRPreviewMultiPopupDlg::GetMultiPrintTemplate(NXDATALIST2Lib::IRowSettingsPtr pRow,
													 CList<NxPrintTemplate::DocInfo, NxPrintTemplate::DocInfo&>& listEMNPrintInfo, bool bOverrideCheck /*= false*/)
{
	// (r.farnworth 2015-07-28 09:55) - PLID 64618 - When we are printing the entire patient history, we don't care if the boxes are checked.
	BOOL bSelected = VarBool(pRow->GetValue(lcCheck)) || bOverrideCheck;
	if (bSelected)
	{
		// (c.haag 2013-02-28) - PLID 55373 - Use the utility function
		NxPrintTemplate::DocInfo printInfo = GetRowDocInfo(pRow);
		listEMNPrintInfo.AddTail(printInfo);

		// (d.thompson 2013-11-07) - PLID 59351 - We need to know what EMNs (not templates) were printed for auditing purposes.  Let's
		//	pull that back to the caller.
		CEMNPointer* pEmnPointer = (CEMNPointer*)VarLong(pRow->GetValue(lcEMNPointer), 0);
		return pEmnPointer->pEMN;
	}

	return NULL;
}


// (b.savon 2011-11-07 09:51) - PLID 25782 - Mechanism for printing a patient's complete medical history
//											(ALL EMN PRINT PREVIEWS) all at once.
void CEMRPreviewMultiPopupDlg::OnBnClickedButtonPrintAll()
{
	try{

		PrintEntireHistory();

	}NxCatchAll("Could not print");
}

// (b.savon 2011-11-10 10:21) - PLID 25782 - Get and Prepare the print template for an EMN at the specified row
CString CEMRPreviewMultiPopupDlg::GetEMNFile( NXDATALIST2Lib::IRowSettingsPtr pRow )
{
	try
	{
		CList<NxPrintTemplate::DocInfo, NxPrintTemplate::DocInfo&> listEMNPrintInfo;
		// (c.haag 2013-02-28) - PLID 55373 - Use the utility function
		NxPrintTemplate::DocInfo printInfo = GetRowDocInfo(pRow);
		listEMNPrintInfo.AddHead(printInfo);
		return m_pEMRPreviewCtrlDlg->PreparePrintTemplate(listEMNPrintInfo);
	}
	NxCatchAllThrow("CEMRPreviewMultiPopupDlg::GetEMNFile - Error getting EMN File.");
}

// (c.haag 2013-02-28) - PLID 55373 - Gets the print info for a given row. This must be called after PrepareEMNs() is called
NxPrintTemplate::DocInfo CEMRPreviewMultiPopupDlg::GetRowDocInfo( NXDATALIST2Lib::IRowSettingsPtr pRow )
{
	CEMNPointer* pEmnPointer = (CEMNPointer*)VarLong(pRow->GetValue(lcEMNPointer), 0);
	CEMNPointer& emn(*pEmnPointer);
	// (c.haag 2013-02-28) - PLID 55373 - We now get the document path from the tree
	CString strDocumentPath = VarString(pRow->GetValue(lcDocumentPath));
	if (!emn.bHeaderFooterValid) {
		// Always pass in the EMN pointer (which may be NULL) and the ID. 
		// We should let the function itself handle using the pEMN or the nID.
		CEMN::GeneratePrintHeaderFooterHTML(emn.pEMN, emn.nID, emn.strHeaderHTML, emn.strFooterHTML);
		emn.bHeaderFooterValid = true;
	}

	// Populate our object for the multi-document print template
	NxPrintTemplate::DocInfo printInfo;
	printInfo.strDocumentPath = strDocumentPath;
	printInfo.strHeaderHTML = emn.strHeaderHTML;
	printInfo.strFooterHTML = emn.strFooterHTML;
	return printInfo;
}

// (b.savon 2011-11-10 10:21) - PLID 25782 - Populate list with all patient's documents
void CEMRPreviewMultiPopupDlg::PopulateAttachedHistoryDocuments( CList<CString, CString&> &lDocuments )
{
	try{
		ADODB::_RecordsetPtr prs = CreateParamRecordset(
		"SELECT	CASE  \r\n"
		"			WHEN PathName LIKE '%MultiPatDoc%'  \r\n"
		"				THEN ''  \r\n"
		"			ELSE Selection  \r\n"
		"	   END AS Selection, \r\n" 
		"	   PathName, \r\n" 
		"	   DATE,  \r\n"
		"	   ServiceDate \r\n"
		"FROM   MailSent \r\n" 
		"	   LEFT JOIN (MergedPacketsT INNER JOIN PacketsT ON MergedPacketsT.PacketID = PacketsT.ID)  \r\n"
		"	   ON MailSent.MergedPacketID = MergedPacketsT.ID  \r\n"
		"	   LEFT JOIN MailSentNotesT  \r\n"
		"	   ON MailSent.MailID = MailSentNotesT.MailID  \r\n"
		"WHERE  MailSent.Personid = {INT} \r\n"
		"	   AND Selection <> '' \r\n" 
		"	   AND PathName <> '' \r\n"
		, m_nPatientID);

		if (prs->eof) {
			return;
		}

		CString strFile;
		while (!prs->eof) {
			strFile = GetPatientDocumentPath(m_nPatientID) ^ AdoFldString(prs, "PathName", "");

			//	Add in order from most recent -> least recent
			if( FileUtils::DoesFileOrDirExist(strFile) ){
				lDocuments.AddHead(strFile);
			}

			prs->MoveNext();
		}
	}NxCatchAllThrow("CEMRPreviewMultiPopupDlg::PopulateAttachedHistoryDocuments - Error populating documents.");
}

// (b.savon 2011-11-10 10:21) - PLID 25782 - Populate list with all patient's labs
void CEMRPreviewMultiPopupDlg::PopulateAttachedLabs( CList<CString, CString&> &lLabs, CList<CString, CString&> &lForms )
{
	try{
		ADODB::_RecordsetPtr prs = CreateParamRecordset(
		"SELECT	LabsT.FormNumberTextID AS Form, \r\n"
		"		LabResultsT.MailID, \r\n"
		"		LabsT.PicID, \r\n"
		"		LabsT.PatientID, \r\n" 
		"		MailSent.Selection, \r\n"
		"		MailSent.PathName \r\n"
		"FROM LabsT \r\n"
		"INNER JOIN LabResultsT ON LabsT.ID = LabResultsT.LabID \r\n"
		"INNER JOIN MailSent ON LabResultsT.MailID = MailSent.MailID \r\n"
		"WHERE	LabsT.PatientID = {INT} \r\n"
		"	AND Selection <> '' \r\n"
		"	AND Pathname <> '' \r\n"
		"	AND LabResultsT.Deleted = 0 \r\n"
		"	AND LabsT.Deleted = 0 \r\n"
		, m_nPatientID);

		if (prs->eof) {
			return;
		}

		CString strFile;
		while (!prs->eof) {
			strFile = GetPatientDocumentPath(m_nPatientID) ^ AdoFldString(prs, "PathName", "");

			//	Grab the form number to be used when printing the lab request report.
			lForms.AddHead(AdoFldString(prs, "Form", ""));

			if( FileUtils::DoesFileOrDirExist(strFile) ){
				//	Add in order from most recent -> least recent
				lLabs.AddHead(strFile);
			}

			prs->MoveNext();
		}
	}NxCatchAllThrow("CEMRPreviewMultiPopupDlg::PopulateAttachedHistoryDocuments - Error populating lab documents.");
}

// (b.savon 2011-11-10 10:21) - PLID 25782 - Print single file with c# printer object
void CEMRPreviewMultiPopupDlg::PrintSingleFile(NexTech_COM::IPrintPtr &pPrinter, CString strFile)
{
	try{
		//	Images, PDFs, and Documents are all printed in a different manner
		//	(not to mention HTML and reports are printed different as well)

		//	Theoretically, pPrinter->SendDocumentToPrinter should be able to handle
		//	any file that has an extension associated with a program.  It creates a process
		//	that opens the file and then prints the document through the programs interface
		//	For now, let's restrict it to Word documents.
		if( IsImageFile(strFile) ){
			pPrinter->SendImageToPrinter(_bstr_t(strFile));
		} else if( strFile.Right(3).CompareNoCase(_T("pdf")) == 0 ){
			pPrinter->SendPDFToPrinter(_bstr_t(strFile));
		} else if( strFile.Right(3).CompareNoCase(_T("doc")) == 0 ||
				   strFile.Right(4).CompareNoCase(_T("docx")) == 0	){
			pPrinter->SendDocumentToPrinter(_bstr_t(strFile));
		}
	}NxCatchAllThrow("CEMRPreviewMultiPopupDlg::PrintSingleFile - Error printing file " + strFile);
}

// (b.savon 2011-11-10 16:27) - PLID 25782 - Needed to update the GUI while printing
void CEMRPreviewMultiPopupDlg::UpdateGUIForPrinting()
{
		m_bPrinting = true;

		m_nxbClose.EnableWindow(FALSE);
		m_nxbPrint.EnableWindow(FALSE);
		m_nxbPrintAll.EnableWindow(FALSE);
		m_nxbPrintPreview.EnableWindow(FALSE);
		m_checkPrintReverse.EnableWindow(FALSE);
		m_pList->Enabled = VARIANT_FALSE;
}

// (b.savon 2011-11-10 10:21) - PLID 25782 - This is the main method to print the history
void CEMRPreviewMultiPopupDlg::PrintEntireHistory()
{
	try {

		m_bPrintEntireHistory = TRUE;

		//	We're Printing, make necessary GUI changes and set flag
		UpdateGUIForPrinting();

		//	Prepare
		PrepareEMNs();

		//	Create a c# Printer instance
		NexTech_COM::IPrintPtr pPrinter;
		pPrinter.CreateInstance("NexTech_COM.Print");
		if( pPrinter ){

			//	Handles resetting the default printer safely when this object
			//	goes out of scope
			CSelectComPrinter cpPrinter = CSelectComPrinter(pPrinter);

			//	Call the custom print dialog so that the user can select a printer.
			if( cpPrinter.SelectPrinter() ){

				//	If they cancelled out of the dialog, don't print anything.
				if( pPrinter->GetCanceled() == VARIANT_FALSE ){

					// (r.gonet 06/13/2013) - PLID 56850 - Print the EMNs combined into a single PDF.
					//  We used to print the EMNs separately.
					PrintHistoryCombined();

					//	Setup lists for the documents, labs, and form#'s
					//	The Documents will check the labs list to avoid double printing
					//	The form #'s will be used to print lab reports for the patient.
					//	Let's also grab the patientID so we can apply filters.
					CList<CString, CString&> lDocuments;
					CList<CString, CString&> lLabs; 
					CList<CString, CString&> lForms;
					
					//	Prepare the report print settings
					CPrintInfo prInfo;
					prInfo.m_bPreview = false;
					prInfo.m_bDirect = true;
					prInfo.m_bDocObject = false;
					extern CPracticeApp theApp;
					prInfo.m_pPD->m_pd.hDevMode = (HGLOBAL)pPrinter->GetDefaultPrinterDevMode();
					prInfo.m_pPD->m_pd.hDevNames = (HGLOBAL)pPrinter->GetDefaultPrinterDevName();

					//	now let's assemble the history documents
					PopulateAttachedHistoryDocuments(lDocuments);
					//	now let's assemble the lab attachments
					PopulateAttachedLabs(lLabs, lForms);

					//	print the history
					for( POSITION pFile = lDocuments.GetHeadPosition(); pFile != NULL; ){
						CString strFile = lDocuments.GetNext(pFile);
						//	Check the labs before printing the document
						if( !lLabs.Find(strFile) ){
							PrintSingleFile(pPrinter, strFile);
						}
					}

					//	print the labs
					for( POSITION pFile = lLabs.GetHeadPosition(); pFile != NULL; ){
						CString strFile = lLabs.GetNext(pFile);
						PrintSingleFile(pPrinter, strFile);
					}

					if( prInfo.m_pPD->m_pd.hDevMode && prInfo.m_pPD->m_pd.hDevNames ){
						//	Print the Lab Results Report for every Form we found for the patient.
						for( POSITION pFile = lForms.GetHeadPosition(); pFile != NULL; ){
							CString strForm = lForms.GetNext(pFile);

							CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(567)]);
							infReport.nPatient = m_nPatientID;	//If we ever want to allow 1 form for multiple patients, just remove this.
							infReport.AddExtraValue(strForm);	//We're hijacking the "extra values" parameter, which is the "use extended" list.			
							CPtrArray aryParams;
							RunReport(&infReport, &aryParams, FALSE, (CWnd*)this, "Lab Results Form", &prInfo);
							ClearRPIParameterList(&aryParams);		//clear our parameters now that the report is done
						}
					}else {
						MessageBox(_T("Unable to print Lab result Reports."),
								   _T("Print Error"),
								   MB_ICONSTOP
								   );

					}//Is Default DEV

				}//END If Not Cancelled
			}//END Is there a Default Printer

			//	Toggle GUI to let the user know were done sending files to the printer.
			m_bPrintEntireHistory = FALSE;
			OnPrintComplete(WPARAM(0), LPARAM(0));
		} else{
			MessageBox( _T("Please register NexTech.COM.dll"), 
						_T("Register DLL"), 
						MB_ICONSTOP
						);
			//	Toggle GUI to let the user know were done sending files to the printer.
			m_bPrintEntireHistory = FALSE;
			OnPrintComplete(WPARAM(0), LPARAM(0));
		}//END Is DLL valid
	} NxCatchAllCallThrow("CEMRPreviewMultiPopupDlg could not print all patient's history!", 
		{
			m_bPrinting = false;
			m_bPrintTemplateTeardownFired = FALSE;
			m_bPrintEntireHistory = FALSE;
			m_nxbClose.EnableWindow(TRUE);
			m_nxbPrint.EnableWindow(TRUE);
			m_nxbPrintPreview.EnableWindow(TRUE);
			m_checkPrintReverse.EnableWindow(TRUE);	
			m_pList->Enabled = VARIANT_TRUE;

			// (a.walling 2009-12-02 10:10) - PLID 24194 - Force a redraw of the browser
			m_pEMRPreviewCtrlDlg->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ERASENOW | RDW_ALLCHILDREN);
		}
	);		
}
// (r.gonet 06/13/2013) - PLID 56850 - Returns the template for the whole history including all EMNs (excluding custom layouts) in the selection list.
CString CEMRPreviewMultiPopupDlg::GetEntireHistoryTemplate()
{
	CList<NxPrintTemplate::DocInfo, NxPrintTemplate::DocInfo&> listEMNPrintInfo;
	// (d.thompson 2013-11-07) - PLID 59351 - Track the EMNs that we generated a print for, we'll audit them at the end
	std::set<CEMN*> setEMNs;

	NXDATALIST2Lib::IRowSettingsPtr pLastEMNRow = NULL;
	CEMNPointer* pLastEMNPointer = NULL;
	// Iterate through the rows from bottom-to-top to search for EMN's with default layouts to print. Remember
	// that an EMN can have multiple layouts below it; so we have to always go to the first row if we find several.
	for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetLastRow(); NULL != pRow; pRow = pRow->GetPreviousRow())
	{
		CEMNPointer* pEmnPointer = (CEMNPointer*)VarLong(pRow->GetValue(lcEMNPointer), 0);
		CEMNPointer& emn(*pEmnPointer);

		// Check whether this is the first iteration. If it is, assign the last-row-anchor
		if (NULL == pLastEMNRow) {
			pLastEMNRow = pRow;
			pLastEMNPointer = (CEMNPointer*)VarLong(pRow->GetValue(lcEMNPointer), 0); 
		}
		else
		{
			// Ok, so this is not the first iteration. See if we've moved to another EMN.
			if (pEmnPointer != pLastEMNPointer)
			{
				// (r.gonet 06/13/2013) - PLID 56850 - Yes, we moved. Lets print the history for last EMN, and then establish
				// this new EMN we've encountered at pRow as our new last-row-anchor.
				// (d.thompson 2013-11-07) - PLID 59351 - Save the EMNs that we setup to print so we can audit them.
				// (r.farnworth 2015-07-28 09:55) - PLID 64618 - When we are printing the entire patient history, we don't care if the boxes are checked, so pass true
				CEMN *pEMN = GetMultiPrintTemplate(pRow->GetNextRow(), listEMNPrintInfo, true);
				if(pEMN && setEMNs.find(pEMN) == setEMNs.end()) {
					setEMNs.insert(pEMN);
				}
				// Now establish this row as the last row of our new EMN.
				pLastEMNRow = pRow;
				pLastEMNPointer = (CEMNPointer*)VarLong(pRow->GetValue(lcEMNPointer), 0); 
			}
		}
	}

	// (r.gonet 06/13/2013) - PLID 56850 - Now finish up by printing the history of the first EMN in the list for printing.
	// (d.thompson 2013-11-07) - PLID 59351 - Save the EMNs that we setup to print so we can audit them.
	// (r.farnworth 2015-07-28 09:55) - PLID 64618 - When we are printing the entire patient history, we don't care if the boxes are checked, so pass true
	CEMN *pEMN = GetMultiPrintTemplate(m_pList->GetFirstRow(), listEMNPrintInfo, true);
	if(pEMN && setEMNs.find(pEMN) == setEMNs.end()) {
		setEMNs.insert(pEMN);
	}

	// (d.thompson 2013-11-07) - PLID 59351 - We now have a set of unique EMNs that we generated print info for, so go ahead
	//	and audit them.  NOTE:  It is intentionally that always audit on the printing 
	//	"attempt".  If they cancel the print dialog, the printer fails, isn't connected, whatever, we'll still audit that
	//	a print was attempted.
	std::set<CEMN*>::iterator it;
	for(it = setEMNs.begin(); it != setEMNs.end(); ++it)
	{
		CEMN *pEMN = (CEMN*)*it;
		AuditPrinting(pEMN);
	}

	// (r.gonet 06/13/2013) - PLID 56850 - This will prepare and return a print template which will point to our multiple documents
	// Since we are printing the Entire History, which is for one party, we want the page count to be continuous instead of resetting every document.
	CString strTemplate =  m_pEMRPreviewCtrlDlg->PreparePrintTemplate(listEMNPrintInfo, false);

	return strTemplate;
}

// (r.gonet 06/13/2013) - PLID 56850 - Sends the entire history to the selected printer.
void CEMRPreviewMultiPopupDlg::PrintHistoryCombined()
{
	CString strTemplate = GetEntireHistoryTemplate();
	m_bPrintTemplateTeardownFired = FALSE;

	// now let's try printing!
	try {
		IOleCommandTargetPtr pCmdTarg = m_pEMRPreviewCtrlDlg->GetOleCommandTarget();

		if (pCmdTarg) {
			_variant_t varTemplate = _bstr_t(strTemplate);
			pCmdTarg->Exec(&CGID_MSHTML, 
					IDM_PRINT, 
					OLECMDEXECOPT_DONTPROMPTUSER, 
					&varTemplate,
					NULL);
		} else {
			ThrowNxException("Could not send print command.");
		}
	}NxCatchAll(__FUNCTION__);

	//	Wait until the combined history prints.
	//	m_bPrintTemplateTeardownFired is set in OnPrintComplete
	MSG msg;
	while( !m_bPrintTemplateTeardownFired && GetMessage(&msg, NULL, 0, 0) ){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

// (c.haag 2013-03-11) - PLID 55373 - Called by PrintEntireHistory to print the history for a single EMN
// (d.thompson 2013-11-07) - No PLID - While implementing Print Auditing, I noticed this function is not actually called anywhere, 
//	so I commented it out.
/*
void CEMRPreviewMultiPopupDlg::PrintHistory(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	//	Get the EMN Pointer from our current row and Get the template path for that EMN
	CEMNPointer* pEmnPointer = (CEMNPointer*)VarLong(pRow->GetValue(lcEMNPointer), 0);
	m_strPrintTemplatePath = GetEMNFile(pRow);
	m_bPrintTemplateTeardownFired = FALSE;

	// now let's try printing!
	try {
		IOleCommandTargetPtr pCmdTarg = m_pEMRPreviewCtrlDlg->GetOleCommandTarget();

		if (pCmdTarg) {
			_variant_t varTemplate = _bstr_t(m_strPrintTemplatePath);
			//	Print, with default printer, and custom print template
			pCmdTarg->Exec(&CGID_MSHTML, 
							IDM_PRINT, 
							OLECMDEXECOPT_DONTPROMPTUSER, 
							&varTemplate,
							NULL);
		} else {
			ThrowNxException("Could not send print command for an EMN.");
		}

	} NxCatchAllThrow("Printing failed");

	//	Wait until this EMN Prints.
	//	m_bPrintTemplateTeardownFired is set in OnPrintComplete
	//	We need to loop here until the PrintTemplateTeardown event handler is fired
	//	so that we don't print documents out of order.
	MSG msg;
	while( !m_bPrintTemplateTeardownFired && GetMessage(&msg, NULL, 0, 0) ){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}*/

void CEMRPreviewMultiPopupDlg::AuditPrinting(CEMN *pEMNToPrint)
{
	// (d.thompson 2013-11-07) - PLID 59351 - Audit when printing.  It is intentionally that always audit on the printing 
	//	"attempt".  If they cancel the print dialog, the printer fails, isn't connected, whatever, we'll still audit that
	//	a print was attempted.
	if(pEMNToPrint) {
		AuditEventItems aei;
		CString strNewText;
		if(!m_bPrintPreview) {
			aei = aeiEMNPrinted;
			strNewText = "EMN " + pEMNToPrint->GetDescription() + " Printed";
		}
		else {//m_bPrintPreview
			aei = aeiEMNPrintPreviewed;
			strNewText = "EMN " + pEMNToPrint->GetDescription() + " Print Previewed";
		}

		AuditEvent(pEMNToPrint->GetParentEMR()->GetPatientID(), pEMNToPrint->GetPatientName(), BeginNewAuditEvent(), aei, pEMNToPrint->GetID(), 
			"", strNewText, aepMedium, aetOpened);
	}
}
