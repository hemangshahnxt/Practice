// EmrTemplateManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "EmrTemplateManagerDlg.h"
#include "EmrTemplateEditorDlg.h"
#include "SelectDlg.h"
#include "PicContainerDlg.h"
#include "AuditTrail.h"
#include "EMN.h"
#include "internationalutils.h"
#include "AdministratorRc.h"
#include "EMRCustomPreviewLayoutsMDIFrame.h"

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
//(e.lally 2011-05-04) PLID 43537 - Renamed slightly
#define ID_NEXWEBTEMPLATESETUP_SET_NEXWEB_TEMPLATES 44412
// (c.haag 2012-01-17) - PLID 54593 - Custom preview layouts
#define ID_EDIT_CUSTOM_PREVIEW_LAYOUTS	44413

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum EEmrTemplateManagerTemplateListColumns {
	etmtlcID, 
	etmtlcName,
	etmtlcCollection,
	etmtlcCollectionName,
	etmtlModifiedDate,
	etmtlCreatedDate,
	etmtlcAddOnce,
	etmtlcIsUniversal, //TES 5/28/2008 - PLID 30169 - Added
	etmtlcNexwebVisible, // (j.gruber 2009-10-30 09:29) - PLID 35806 - add nexweb template identifier
	etmtlcRowForeColor, // (b.cardillo 2010-09-23 15:41) - PLID 39568 - Not really related to this pl item per se, but for efficiency I made row foreground color auto-set
};

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEmrTemplateManagerDlg dialog


CEmrTemplateManagerDlg::CEmrTemplateManagerDlg(CWnd* pParent)
	: CNxDialog(CEmrTemplateManagerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEmrTemplateManagerDlg)
		m_pParent = pParent;
		m_nSavedTopRow = -1;
	//}}AFX_DATA_INIT
}


void CEmrTemplateManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmrTemplateManagerDlg)
	DDX_Control(pDX, IDC_NEW_EMR_TEMPLATE, m_btnNewEmrTemplate);
	DDX_Control(pDX, IDC_COPY_EMR_TEMPLATE, m_btnCopyEmrTemplate);
	DDX_Control(pDX, IDC_DELETE_EMR_TEMPLATE, m_btnDeleteEmrTemplate);
	DDX_Control(pDX, IDC_EMR_TEMPLATE_EDIT, m_btnEmrTemplateEdit);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_EMN_TEMPLATES_SELECT_ALL, m_btnSelectAll);
	DDX_Control(pDX, IDC_EMN_TEMPLATES_UNSELECT_ALL, m_btnUnselectAll);
	DDX_Control(pDX, IDC_KEYWORD_FILTER, m_btnKeywordFilter);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmrTemplateManagerDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEmrTemplateManagerDlg)
	ON_BN_CLICKED(IDC_EMR_TEMPLATE_EDIT, OnEmrTemplateEdit)
	ON_BN_CLICKED(IDC_DELETE_EMR_TEMPLATE, OnDeleteEmrTemplate)
	ON_BN_CLICKED(IDC_COPY_EMR_TEMPLATE, OnCopyEmrTemplate)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_NEW_EMR_TEMPLATE, OnNewEmrTemplate)
	ON_BN_CLICKED(IDC_EMN_TEMPLATES_SELECT_ALL, OnEmnTemplatesSelectAll)
	ON_BN_CLICKED(IDC_EMN_TEMPLATES_UNSELECT_ALL, OnEmnTemplatesUnselectAll)
	ON_BN_CLICKED(IDC_KEYWORD_FILTER, OnKeywordFilter)	
	ON_COMMAND(ID_NEXWEBTEMPLATESETUP_SET_NEXWEB_TEMPLATES, OnGotoNexWebListDisplayManager)	
	ON_COMMAND(ID_EDIT_CUSTOM_PREVIEW_LAYOUTS, OnEditCustomPreviewLayouts)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrTemplateManagerDlg message handlers

BOOL CEmrTemplateManagerDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-30 09:16) - PLID 29840 - NxIconify buttons
		m_btnNewEmrTemplate.AutoSet(NXB_NEW);
		m_btnCopyEmrTemplate.AutoSet(NXB_NEW);
		m_btnDeleteEmrTemplate.AutoSet(NXB_DELETE);
		m_btnEmrTemplateEdit.AutoSet(NXB_MODIFY);
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnSelectAll.AutoSet(NXB_MODIFY);
		m_btnUnselectAll.AutoSet(NXB_MODIFY);

		//TES 3/2/2009 - PLID 33102 - Set our filter button's text
		m_btnKeywordFilter.SetWindowText("Filter");
		
		m_pTemplateList = BindNxDataListCtrl(this, IDC_EMR_TEMPLATES, GetRemoteData(), false);

		//TES 3/2/2009 - PLID 33102 - Since we're not currently filtering, use the default WHERE clause.
		m_strDefaultWhereClause = "Deleted = 0 AND (EmrCollectionT.ID Is Null OR EmrCollectionT.Inactive = 0)";
		m_pTemplateList->WhereClause = _bstr_t(m_strDefaultWhereClause);
		m_pTemplateList->Requery();

		GetDlgItem(IDC_DELETE_EMR_TEMPLATE)->EnableWindow(FALSE);
		GetDlgItem(IDC_COPY_EMR_TEMPLATE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EMR_TEMPLATE_EDIT)->EnableWindow(FALSE);
	}
	NxCatchAll("Error in CEmrTemplateManagerDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEmrTemplateManagerDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEmrTemplateManagerDlg)
	ON_EVENT(CEmrTemplateManagerDlg, IDC_EMR_TEMPLATES, 2 /* SelChanged */, OnSelChangedEmrTemplates, VTS_I4)
	ON_EVENT(CEmrTemplateManagerDlg, IDC_EMR_TEMPLATES, 9 /* EditingFinishing */, OnEditingFinishingEmrTemplates, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEmrTemplateManagerDlg, IDC_EMR_TEMPLATES, 10 /* EditingFinished */, OnEditingFinishedEmrTemplates, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEmrTemplateManagerDlg, IDC_EMR_TEMPLATES, 3 /* DblClickCell */, OnDblClickCellEmrTemplates, VTS_I4 VTS_I2)
	ON_EVENT(CEmrTemplateManagerDlg, IDC_EMR_TEMPLATES, 6 /* RButtonDown */, OnRButtonDownEmrTemplates, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrTemplateManagerDlg, IDC_EMR_TEMPLATES, 18 /* RequeryFinished */, OnRequeryFinishedEmrTemplates, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEmrTemplateManagerDlg::OnRButtonDownEmrTemplates(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		GetDlgItem(IDC_EMR_TEMPLATES)->SetFocus();
		m_pTemplateList->PutCurSel(nRow);
		OnSelChangedEmrTemplates(nRow);		
	} NxCatchAll("Error in OnRButtonDownEmrTemplates");
}

void CEmrTemplateManagerDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	try {
		// Is the focus on the template list?
		CWnd *pEmnTemplateWnd = GetDlgItem(IDC_EMR_TEMPLATES);
		if (pWnd->GetSafeHwnd() == pEmnTemplateWnd->GetSafeHwnd()) {
			// Yes, so show the context menu
			CMenu mnu;
			mnu.CreatePopupMenu();
			if (mnu.m_hMenu) {
				// Decide on whether the menu items will be enabled or not
				long nEnabledFlag = (m_pTemplateList->CurSel == sriNoRow) ? MF_DISABLED|MF_GRAYED : MF_ENABLED;
				// Create the menu items
				int nPosition = 0;
				mnu.InsertMenu(nPosition++, MF_BYPOSITION|nEnabledFlag|MF_STRING, IDC_EMR_TEMPLATE_EDIT, "&Edit");
				mnu.InsertMenu(nPosition++, MF_BYPOSITION|nEnabledFlag|MF_STRING, IDC_COPY_EMR_TEMPLATE, "&Create Copy");
				mnu.InsertMenu(nPosition++, MF_BYPOSITION|nEnabledFlag|MF_SEPARATOR);
				mnu.InsertMenu(nPosition++, MF_BYPOSITION|nEnabledFlag|MF_STRING, IDC_DELETE_EMR_TEMPLATE, "&Delete");
				// (j.gruber 2009-10-30 10:52) - PLID 35806 - add the nexweb template setting
				
				if (m_pTemplateList->CurSel != sriNoRow) {
					// (j.gruber 2009-11-17 11:57) - PLID 36106 - check the permissions
					//allow write and writewithpass here, then we'll check the password before we set it
					long nHasPermissions = CheckCurrentUserPermissions(bioNexwebObjects, sptWrite, FALSE, 0, TRUE, TRUE) ? MF_ENABLED: MF_DISABLED|MF_GRAYED ; 
					//(e.lally 2011-05-04) PLID 43537 - Changed to NexWebVisible bool
					BOOL bNexWebVisible = AsBool(m_pTemplateList->GetValue(m_pTemplateList->CurSel, etmtlcNexwebVisible));
					if (!bNexWebVisible) {
						if(g_pLicense->CheckForLicense(CLicense::lcNexWebPortal, CLicense::cflrSilent)) {
							//(e.lally 2011-05-04) PLID 43537 - This template is not visible in NexWeb, but we have the portal license, so we can give the option to go to the setup.
							mnu.InsertMenu(nPosition++, MF_BYPOSITION|nHasPermissions|MF_SEPARATOR);
							mnu.InsertMenu(nPosition++, MF_BYPOSITION|nHasPermissions|MF_STRING, ID_NEXWEBTEMPLATESETUP_SET_NEXWEB_TEMPLATES, "&Set NexWeb Templates");															
						}
					}
					else {
						//(e.lally 2011-05-04) PLID 43537 - This template is visible in NexWeb, always give the option to go to the setup.
						mnu.InsertMenu(nPosition++, MF_BYPOSITION|nHasPermissions|MF_SEPARATOR);
						mnu.InsertMenu(nPosition++, MF_BYPOSITION|nHasPermissions|MF_STRING, ID_NEXWEBTEMPLATESETUP_SET_NEXWEB_TEMPLATES, "&Set NexWeb Templates");						
					}
					// (c.haag 2012-01-17) - PLID 54593 - Access to the custom previews window
					mnu.InsertMenu(nPosition++, MF_BYPOSITION|nEnabledFlag|MF_SEPARATOR);
					mnu.InsertMenu(nPosition++, MF_BYPOSITION|nEnabledFlag|MF_STRING, ID_EDIT_CUSTOM_PREVIEW_LAYOUTS, "Edit Custom Preview &Layouts...");
				}

				// Set the default item
				mnu.SetDefaultItem(IDC_EMR_TEMPLATE_EDIT);

				// Make sure we have an appropriate place to pop up the menu
				if (point.x == -1) {
					CRect rc;
					pWnd->GetWindowRect(&rc);
					GetCursorPos(&point);
					if (!rc.PtInRect(point)) {
						point.x = rc.left+5;
						point.y = rc.top+5;
					}
				}

				// Show the popup
				mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
				mnu.DestroyMenu();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CEmrTemplateManagerDlg::OnSelChangedEmrTemplates(long nNewSel) 
{
	GetDlgItem(IDC_DELETE_EMR_TEMPLATE)->EnableWindow(nNewSel != -1);
	GetDlgItem(IDC_COPY_EMR_TEMPLATE)->EnableWindow(nNewSel != -1);
	GetDlgItem(IDC_EMR_TEMPLATE_EDIT)->EnableWindow(nNewSel != -1);
}

void CEmrTemplateManagerDlg::OnEmrTemplateEdit() 
{
	try {
		if(m_pTemplateList->CurSel != -1) {
			long nEmrTemplateID = VarLong(m_pTemplateList->GetValue(m_pTemplateList->CurSel, etmtlcID));
			CDialog::OnOK();
			m_pParent->PostMessage(NXM_EDIT_EMR_OR_TEMPLATE, nEmrTemplateID);
		}
	} NxCatchAll("Error in OnEmrTemplateEdit");
}

void CEmrTemplateManagerDlg::OnDblClickCellEmrTemplates(long nRowIndex, short nColIndex) 
{
	if (nRowIndex == sriNoRow) {
		return;
	}

	try {
		m_pTemplateList->PutCurSel(nRowIndex);
		OnEmrTemplateEdit();
	} NxCatchAll("CEmrTemplateManagerDlg::OnDblClickCellEmrTemplates");	
}

void CEmrTemplateManagerDlg::OnDeleteEmrTemplate() 
{
	try {
		if(m_pTemplateList->CurSel == -1) {
			return;
		}

		long nEmrTemplateID = VarLong(m_pTemplateList->GetValue(m_pTemplateList->CurSel, etmtlcID));

		// (j.jones 2009-03-27 09:48) - PLID 33703 - warn about EMR Analysis Configurations using this template
		// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
		if(ReturnsRecordsParam("SELECT ID FROM EMRAnalysisConfigT WHERE FilterByTemplateID = {INT}", nEmrTemplateID)) {
			if(IDNO == MessageBox("This template is in use as a filter for at least one EMR Analysis Configuration.\n"
				"If you delete this template, it will be removed from these filters.\n\n"
				"Are you sure you wish to continue?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
		}

		
		// (j.gruber 2012-08-31 13:07) - PLID 52285 - let them know if has an OMR form
		if(ReturnsRecordsParam("SELECT ID FROM OMRFormT WHERE EMRTemplateID = {INT}", nEmrTemplateID)) {
			if(IDNO == MessageBox("This template is associated with at least one OMR form.\n"		
				"Are you sure you wish to continue?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
		}

		if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to permanently delete this EMR template?  This action can not be undone!")) {
			return;
		}

		// (a.walling 2008-06-25 16:17) - PLID 30515 - Need to ensure the template is not being edited
		// (j.jones 2009-03-27 09:52) - PLID 33703 - added ability to clear EMRAnalysisConfigT
		//(e.lally 2011-05-05) PLID 43481 - Set to not visible in the NexWeb display setup too for clarity
		// (j.dinatale 2012-09-10 10:15) - PLID 52551 - delete from our OMR tables
		// (j.armen 2013-05-14 12:04) - PLID 56683 - Moved this into a batch, refactored EMN Template Access
		CParamSqlBatch sql;

		sql.Add("DECLARE @TemplateID INT");
		sql.Add("SELECT @TemplateID = {INT}", nEmrTemplateID);

		sql.Add(
			"IF EXISTS(\r\n"
			"	SELECT 1\r\n"
			"	FROM EMNTemplateAccessT\r\n"
			"	WHERE EmnID = @TemplateID)\r\n"
			"BEGIN\r\n"
			"	RAISERROR('Template cannot be deleted; it is being modified by another user.', 16, 43)\r\n"
			"	ROLLBACK TRAN\r\n"
			"	RETURN\r\n"
			"END");

		sql.Add("UPDATE NexWebDisplayT SET Visible = 0 WHERE EmrTemplateID = @TemplateID");
		sql.Add("UPDATE EMRActionsT SET Deleted = 1\r\n"
				"WHERE DestType IN({CONST_INT}, {CONST_INT}) AND DestID = @TemplateID", eaoMint, eaoMintItems);
		sql.Add("UPDATE EMRAnalysisConfigT SET FilterByTemplateID = NULL WHERE FilterByTemplateID = @TemplateID");
		sql.Add("UPDATE EmrTemplateT SET Deleted = 1 WHERE ID = @TemplateID");
		sql.Add("DELETE OMRFormDetailT FROM OMRFormDetailT\r\n"
				"INNER JOIN OMRFormT ON OMRFormDetailT.OMRFormID = OMRFormT.ID\r\n"
				"WHERE OMRFormT.EMRTemplateID = @TemplateID");
		sql.Add("DELETE FROM OMRFormT WHERE OMRFormT.EMRTemplateID = @TemplateID");

		sql.Execute(GetRemoteData());

		m_pTemplateList->RemoveRow(m_pTemplateList->CurSel);

		//Audit the deletion
		//(e.lally 2006-01-25) PLID 18522 - We need to audit changes to an EMN template
		//get the template name
		CString strTemplateName = "";
		_RecordsetPtr rs = CreateRecordset("SELECT Name FROM EMRTemplateT WHERE ID = %li", nEmrTemplateID);
		if(!rs->eof) {
			strTemplateName = AdoFldString(rs, "Name","");
		}
		rs->Close();
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiEMNTemplateDeleted, nEmrTemplateID, strTemplateName, "<Deleted>", aepHigh, aetDeleted);

		// (j.jones 2014-08-12 10:17) - PLID 63189 - we now call RefreshEMRTemplateTable
		// to send an EMRTemplateT tablechecker, which also tells some of our local
		// lists to refresh
		RefreshEMRTemplateTable();

	}NxCatchAll("Error in CEmrTemplateManagerDlg::OnDeleteEmrTemplate()");
}

CString ValidateTemplateName(const CString &strTemplateName)
{
	CString strAns(strTemplateName);

	const long cnEMRTemplateNameMaxLen = 200;
	
	// Make sure it's below the max length
	if (strAns.GetLength() > cnEMRTemplateNameMaxLen) {
		strAns.Delete(cnEMRTemplateNameMaxLen, strAns.GetLength() - cnEMRTemplateNameMaxLen);
	}

	// Make sure it doesn't already exist
	{
		for (DWORD i=0; ; i++) {
			CString strThisName;
			if (i == 0) {
				strThisName = strAns;
			} else {
				CString strNum;
				strNum.Format("%lu", i);
				long nPrefLen = strAns.GetLength() + strNum.GetLength();
				if (nPrefLen <= cnEMRTemplateNameMaxLen) {
					strThisName = strAns + strNum;
				} else {
					strThisName = strAns.Left(cnEMRTemplateNameMaxLen - strNum.GetLength()) + strNum;
				}
			}
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			if (!ReturnsRecordsParam("SELECT * FROM EMRTemplateT WHERE Name = {STRING} AND Deleted = 0", strThisName)) {
				// Found a unique name, so break out of the loop
				strAns = strThisName;
				break;
			}
		}
	}

	// We now have our valid name
	return strAns;
}

void CEmrTemplateManagerDlg::OnCopyEmrTemplate() 
{
	try {
		if(m_pTemplateList->CurSel == -1) return;
		long nOldTemplateID = VarLong(m_pTemplateList->GetValue(m_pTemplateList->CurSel, etmtlcID));
		long nOldCollectionID = VarLong(m_pTemplateList->GetValue(m_pTemplateList->CurSel, etmtlcCollection));
		bool bAddOnce = VarBool(m_pTemplateList->GetValue(m_pTemplateList->CurSel, etmtlcAddOnce))?true:false;
		//TES 5/28/2008 - PLID 30169 - Added a new IsUniversal column.
		bool bIsUniversal = VarBool(m_pTemplateList->GetValue(m_pTemplateList->CurSel, etmtlcIsUniversal))?true:false;
		CString strExistingTemplateName = VarString(m_pTemplateList->GetValue(m_pTemplateList->GetCurSel(), etmtlcName));

		long nCollectionID;
		CString strCollectionName;
		{
			CSelectDlg dlgCollection(this);
			dlgCollection.m_strCaption = "Please select a collection for the new template:";
			dlgCollection.m_strFromClause = "EmrCollectionT";
			dlgCollection.m_strTitle = "Select EMR Collection";
			dlgCollection.m_strWhereClause = "Inactive = 0";
			//TES 1/4/2010 - PLID 36413 - Support adding a new collection on the fly.
			dlgCollection.m_bAllowAddRecord = TRUE;
			dlgCollection.m_strRecordType = "collection";
			dlgCollection.m_strRecordTable = "EMRCollectionT";
			dlgCollection.m_strRecordField = "Name";
			dlgCollection.m_strParamSqlRecordAdd =
				"SET NOCOUNT ON;"
				"INSERT INTO EMRCollectionT (Name, MenuOrder, Inactive) "
				"SELECT {STRING}, COALESCE(MAX(MenuOrder), 0) + 1, 0 "
				"FROM EMRCollectionT;"
				"SET NOCOUNT OFF;"
				"\r\n"
				"SELECT Convert(int, SCOPE_IDENTITY()) AS NewID";
			dlgCollection.m_nRecordIDColumn = 0;
			dlgCollection.m_nRecordNameColumn = 1;
			DatalistColumn dcID, dcName, dcAddOnce;
			dcID.strField = "ID";
			dcID.strTitle = "ID";
			dcID.nWidth = 0;
			dcID.nStyle = csVisible|csFixedWidth;
			dcID.nSortPriority = -1;
			dcID.bSortAsc = TRUE;
			dlgCollection.m_arColumns.Add(dcID);
			dcName.strField = "Name";
			dcName.strTitle = "Collection";
			dcName.nWidth = -1;
			dcName.nStyle = csVisible|csWidthAuto;
			dcName.nSortPriority = 0;
			dcName.bSortAsc = TRUE;
			dlgCollection.m_arColumns.Add(dcName);
			// (b.eyers 2015-04-20) - PLID 34676 - auto-select the collection from the copied template
			dlgCollection.SetPreSelectedID(0, nOldCollectionID);

			UINT nReturn = dlgCollection.DoModal();
			if (dlgCollection.m_bWereRecordsAdded) {
				//TES 1/4/2010 - PLID 36413 - Make sure the new collections get included in the collection dropdown
				RequeryTemplateList();
			}
			if(IDOK != nReturn) {
				return;
			}

			nCollectionID = VarLong(dlgCollection.m_arSelectedValues[0]);
			strCollectionName = VarString(dlgCollection.m_arSelectedValues[1]);
		}

		if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to make a new '%s' template with the details and layout of the '%s' template?",
			strCollectionName, strExistingTemplateName)) {
			return;
		}
		//OK, we've gotten the user's go-ahead.
		CString strNewTemplateName = ValidateTemplateName("Copy of " + strExistingTemplateName);

		CWaitCursor cuWait;
		//TES 2/16/2006 - Load the template, set its new name, then save it as a new template.
		CEMN emn(NULL);
		SourceActionInfo saiBlank; // (z.manning 2009-03-04 15:39) - PLID 33338
		emn.LoadFromTemplateID(nOldTemplateID, TRUE, saiBlank, TRUE);
		emn.SetCollectionID(nCollectionID);
		emn.SetDescription(strNewTemplateName);
		// (j.jones 2007-01-23 11:00) - PLID 24027 - update the source details such that their pointers are set
		// and also clear their detail IDs
		emn.UpdateSourceDetailPointers(TRUE);
		emn.SetNew();

		// (j.jones 2012-09-28 09:00) - PLID 52820 - SaveEMRObject requires this parameter, but it
		// is not used on templates.
		BOOL bDrugInteractionsChanged = FALSE;

		//OK, now save it.
		// (a.walling 2007-12-26 08:58) - PLID 28443 - We should ensure this worked before adding anything to the list.
		CDWordArray arNewCDSInterventions;
		//TES 10/31/2013 - PLID 59251 - SaveEMRObject has a parameter for interventions now, but templates should never trigger any
		EmrSaveStatus ess = SaveEMRObject(esotEMN, (long)&emn, FALSE, bDrugInteractionsChanged, arNewCDSInterventions);
		ASSERT(arNewCDSInterventions.GetCount() == 0);
		// (a.walling 2008-06-26 16:22) - PLID 30513 - Use SUCEEDED macro to check for success
		if (SUCCEEDED(ess)) {
			long nNewTemplateID = emn.GetID();		

			//All done.  Make sure the new template is on screen, and highlight it.
			{
				IRowSettingsPtr pRow = m_pTemplateList->GetRow(-1);
				pRow->PutValue(etmtlcID,(long)nNewTemplateID);
				pRow->PutValue(etmtlcName, (LPCTSTR)strNewTemplateName);
				pRow->PutValue(etmtlcCollection,(long)nCollectionID);
				pRow->PutValue(etmtlcCollectionName,_bstr_t(strCollectionName));
				pRow->PutValue(etmtlcAddOnce, bAddOnce);
				//TES 5/28/2008 - PLID 30169 - Added a new IsUniversal column.
				pRow->PutValue(etmtlcIsUniversal, bIsUniversal);
				// (a.walling 2010-03-25 08:28) - PLID 37820 - This was not being set, causing the value to be VT_EMPTY, and therefore throwing exceptions when right clicking after copying
				//(e.lally 2011-05-04) PLID 43537 - Changed to NexWebVisible bool
				pRow->PutValue(etmtlcNexwebVisible, VARIANT_FALSE);
				pRow->PutValue(etmtlcRowForeColor, (long)dlColorNotSet); // Included for code completeness; it has no actual effect though.
				m_pTemplateList->AddRow(pRow);
				m_pTemplateList->SetSelByColumn(etmtlcID,(long)nNewTemplateID);
				m_pTemplateList->StartEditing(m_pTemplateList->GetCurSel(), etmtlcName);
			}

			// (j.jones 2014-08-12 10:17) - PLID 63189 - we now call RefreshEMRTemplateTable
			// to send an EMRTemplateT tablechecker, which also tells some of our local
			// lists to refresh
			RefreshEMRTemplateTable();
		}
	}NxCatchAll("Error in CEMRTemplateManagerDlg::OnCopyEmrTemplate()");
}

void CEmrTemplateManagerDlg::OnEditingFinishingEmrTemplates(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		if (COleVariant(varOldValue) == COleVariant(*pvarNewValue)) {
			// (a.walling 2006-09-26 10:50) - PLID 22217 - Nothing changed, don't refresh, don't write to database.
			*pbCommit = FALSE;
			return;
		}
		switch(nCol) {
		case etmtlcCollection://Collection.
			if(*pbCommit) {
				//Double-check with the user.
				if (IDYES != MsgBox(MB_YESNO, 
					"Are you sure you wish to change the collection of this template?\r\n\r\n    NOTE: The collection is usually "
					"considered a to be a fundamental feature of a given template.  If you proceed, you might also want to review "
					"the other properties of the template, such as its name, to make sure the template is still self-consistent.")) {
					*pbCommit = FALSE;
				}
			}
			break;
		case etmtlcName:
			if(*pbCommit) {
				CString strOldName = AsString(varOldValue);
				strOldName.TrimRight();
				strOldName.TrimLeft();
				CString strNewName = AsString(*pvarNewValue);
				strNewName.TrimRight();
				strNewName.TrimLeft();

				long nTemplateID = VarLong(m_pTemplateList->GetValue(nRow, etmtlcID));

				if (strNewName.CompareNoCase(strOldName) != 0) {
					// They're changing the name so we have to see if it conflicts with any other name
					if (strNewName.GetLength() > 200) {
						// It's too long
						MsgBox(MB_OK|MB_ICONEXCLAMATION, "This name is too long.  Please choose a name with fewer than 200 characters for this template.");
						*pbContinue = FALSE;
					} else if (strNewName.GetLength() == 0) {
						// It's blank
						MsgBox(MB_OK|MB_ICONEXCLAMATION, "A template name cannot be blank.");
						*pbContinue = FALSE;						
					}
					else {
						// (j.jones 2009-09-15 08:45) - PLID 30448 - ensure the name doesn't already exist
						_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ID FROM EMRTemplateT "
							"WHERE Name = {STRING} AND Deleted = 0 AND ID <> {INT}", strNewName, nTemplateID);
						if(!rs->eof) {
							// It's a dup, so we can't proceed
							MsgBox(MB_OK|MB_ICONEXCLAMATION, "There is already a template with this name.  Please choose a unique name for this template.");
							*pbContinue = FALSE;
						}
						// Double-check with the user.
						else if (IDYES != MsgBox(MB_YESNO|MB_ICONQUESTION, "Are you sure you wish to change the name of this template?")) {
							*pbContinue = FALSE;
						}
					}
				}
			}
			break;
		}
	} NxCatchAll("Error in CEmrTemplateManagerDlg::OnEditingFinishingEmrTemplates()");
}

void CEmrTemplateManagerDlg::OnEditingFinishedEmrTemplates(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		switch(nCol) {
		case etmtlcCollection://Collection
			if(bCommit) {
				long nCollectionID = VarLong(varNewValue);
				long nTemplateID = VarLong(m_pTemplateList->GetValue(nRow, etmtlcID));
				
				//Update the template
				// (a.walling 2008-06-26 09:18) - PLID 30515 - Ensure this does not affect in-use templates
				long nAffected = 0;
				ExecuteParamSql(GetRemoteData(), &nAffected,
					"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
					"UPDATE EmrTemplateT "
					"SET CollectionID = {INT} "
					"WHERE ID = {INT} AND ID NOT IN (SELECT EmnID FROM EMNTemplateAccessT)", nCollectionID, nTemplateID);

				if (nAffected <= 0) {
					m_pTemplateList->PutValue(nRow, etmtlcCollection, varOldValue);
					MessageBox("Could not update collection; the template is currently being modified by another user.");
				} else {
					// (j.jones 2014-08-12 10:17) - PLID 63189 - we now call RefreshEMRTemplateTable
					// to send an EMRTemplateT tablechecker, which also tells some of our local
					// lists to refresh
					RefreshEMRTemplateTable();
				}
			}
			break;
		case etmtlcName:
			if (bCommit) {
				CString strNewName = AsString(varNewValue);
				strNewName.TrimRight();
				strNewName.TrimLeft();
				// (a.walling 2008-06-26 09:18) - PLID 30515 - Ensure this does not affect in-use templates
				long nAffected = 0;
				ExecuteParamSql(GetRemoteData(), &nAffected,
					"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
					"UPDATE EMRTemplateT SET Name = {STRING}, ModifiedDate = GetDate() WHERE ID = {INT} AND ID NOT IN (SELECT EmnID FROM EMNTemplateAccessT)", strNewName, VarLong(m_pTemplateList->GetValue(nRow, etmtlcID)));
				if (nAffected <= 0) {
					m_pTemplateList->PutValue(nRow, etmtlcName, varOldValue);
					MessageBox("Could not update name; the template is currently being modified by another user.");
				} else {
					// (j.jones 2014-08-12 10:17) - PLID 63189 - we now call RefreshEMRTemplateTable
					// to send an EMRTemplateT tablechecker, which also tells some of our local
					// lists to refresh
					RefreshEMRTemplateTable();

					//DRT 5/23/2006 - PLID 20771 - We need to audit the change of description!
					long nAuditID = BeginNewAuditEvent();
					AuditEvent(-1, "", nAuditID, aeiEMNTemplateDescription, VarLong(m_pTemplateList->GetValue(nRow, etmtlcID)), AsString(varOldValue), strNewName, aepMedium, aetChanged);

					try {
						// (a.walling 2007-03-19 09:22) - PLID 24614 - Update the modified date; we'll use the local time instead of server time
						// which will save us a query.
						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						m_pTemplateList->PutValue(nRow, etmtlModifiedDate, AsVariant(FormatDateTimeForInterface(dtNow, 0, dtoDate)));
					} NxCatchAll("Error updating modified date in template list");
				}
			}
			break;
		case etmtlcAddOnce:
			if(bCommit) {
				// (a.walling 2008-06-26 09:18) - PLID 30515 - Ensure this does not affect in-use templates
				long nAffected = 0;
				ExecuteParamSql(GetRemoteData(), &nAffected, 
					"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
					"UPDATE EmrTemplateT SET AddOnce = {BIT} WHERE ID = {INT} AND ID NOT IN (SELECT EmnID FROM EMNTemplateAccessT)", VarBool(varNewValue), VarLong(m_pTemplateList->GetValue(nRow, etmtlcID)));
				if (nAffected <= 0) {
					m_pTemplateList->PutValue(nRow, etmtlcAddOnce, varOldValue);
					MessageBox("Could not update Add Once status; the template is currently being modified by another user.");
				} else {
					// (j.jones 2014-08-12 10:17) - PLID 63189 - we now call RefreshEMRTemplateTable
					// to send an EMRTemplateT tablechecker, which also tells some of our local
					// lists to refresh
					RefreshEMRTemplateTable();
				}
			}
			break;
		case etmtlcIsUniversal:
			//TES 5/28/2008 - PLID 30169 - Added a new IsUniversal column.
			if(bCommit) {
				// (a.walling 2008-06-26 09:18) - PLID 30515 - Ensure this does not affect in-use templates
				long nAffected = 0;
				ExecuteParamSql(GetRemoteData(), &nAffected, 
					"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
					"UPDATE EmrTemplateT SET IsUniversal = {BIT} WHERE ID = {INT} AND ID NOT IN (SELECT EmnID FROM EMNTemplateAccessT)", VarBool(varNewValue), VarLong(m_pTemplateList->GetValue(nRow, etmtlcID)));
				if (nAffected <= 0) {
					m_pTemplateList->PutValue(nRow, etmtlcIsUniversal, varOldValue);
					MessageBox("Could not update Universal status; the template is currently being modified by another user.");
				} else {
					// (j.jones 2014-08-12 10:17) - PLID 63189 - we now call RefreshEMRTemplateTable
					// to send an EMRTemplateT tablechecker, which also tells some of our local
					// lists to refresh
					RefreshEMRTemplateTable();
				}
			}
			break;
		}

		// (a.walling 2007-03-19 10:11) - PLID 24614 - We may have modified something that affected the sort! Thankfully, Sort() preserves the cursel.
		m_pTemplateList->Sort();
	}NxCatchAll("Error in CEmrTemplateManagerDlg::OnEditingFinishedEmrTemplates()");
}

void CEmrTemplateManagerDlg::OnNewEmrTemplate() 
{
	try {

		//first prompt for a collection
		long nCollectionID;
		CString strCollectionName;
		{
			CSelectDlg dlgCollection(this);
			dlgCollection.m_strCaption = "Please select a collection for the new template:";
			dlgCollection.m_strFromClause = "EmrCollectionT";
			dlgCollection.m_strTitle = "Select EMR Collection";
			dlgCollection.m_strWhereClause = "Inactive = 0";
			DatalistColumn dcID, dcName, dcAddOnce;
			dcID.strField = "ID";
			dcID.strTitle = "ID";
			dcID.nWidth = 0;
			dcID.nStyle = csVisible|csFixedWidth;
			dcID.nSortPriority = -1;
			dcID.bSortAsc = TRUE;
			dlgCollection.m_arColumns.Add(dcID);
			dcName.strField = "Name";
			dcName.strTitle = "Collection";
			dcName.nWidth = -1;
			dcName.nStyle = csVisible|csWidthAuto;
			dcName.nSortPriority = 0;
			dcName.bSortAsc = TRUE;
			dlgCollection.m_arColumns.Add(dcName);
			// (c.haag 2009-08-24 16:16) - PLID 29310 - We now support adding new collections on the spot
			dlgCollection.m_bAllowAddRecord = TRUE;
			dlgCollection.m_strRecordType = "collection";
			dlgCollection.m_strRecordTable = "EMRCollectionT";
			dlgCollection.m_strRecordField = "Name";
			dlgCollection.m_strParamSqlRecordAdd =
				"SET NOCOUNT ON;"
				"INSERT INTO EMRCollectionT (Name, MenuOrder, Inactive) "
				"SELECT {STRING}, COALESCE(MAX(MenuOrder), 0) + 1, 0 "
				"FROM EMRCollectionT;"
				"SET NOCOUNT OFF;"
				"\r\n"
				"SELECT Convert(int, SCOPE_IDENTITY()) AS NewID";
			dlgCollection.m_nRecordIDColumn = 0;
			dlgCollection.m_nRecordNameColumn = 1;
			
			if(IDOK != dlgCollection.DoModal()) {
				if (dlgCollection.m_bWereRecordsAdded) {
					// Make sure the new collections get included in the collection dropdown
					RequeryTemplateList();
				}
				return;
			}

			nCollectionID = VarLong(dlgCollection.m_arSelectedValues[0]);
			strCollectionName = VarString(dlgCollection.m_arSelectedValues[1]);
		}

		// now open the new emr using the above selected collection

		// (a.walling 2012-02-29 06:50) - PLID 48469 - Handle new template editor

		CDialog::OnOK();
		m_pParent->PostMessage(NXM_EDIT_EMR_OR_TEMPLATE, nCollectionID, 2);

	} NxCatchAll("Error creating new template.");
}

void CEmrTemplateManagerDlg::OnEmnTemplatesSelectAll() 
{
	try {
		long p = m_pTemplateList->GetFirstRowEnum();
		LPDISPATCH pDisp = NULL;

		long nSkipped = 0;

		while (p) {
			m_pTemplateList->GetNextRowEnum(&p, &pDisp);
			IRowSettingsPtr pRow(pDisp);

			//If it's already checked, don't need to do anything
			BOOL bChecked = VarBool(pRow->GetValue(etmtlcAddOnce), FALSE);

			if(!bChecked) {
				//check the box
				pRow->PutValue(etmtlcAddOnce, true);
				long nAffected = 0;
				ExecuteParamSql(GetRemoteData(), &nAffected, 
					"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
					"UPDATE EmrTemplateT SET AddOnce = {BIT} WHERE ID = {INT} AND ID NOT IN (SELECT EmnID FROM EMNTemplateAccessT)", TRUE, VarLong(pRow->GetValue(etmtlcID)));

				// (a.walling 2008-06-26 09:15) - PLID 30515 - Could not update, so revert the box
				if (nAffected <= 0) {
					nSkipped++;					
					pRow->PutValue(etmtlcAddOnce, false);
				}
			}

			pDisp->Release();
		}

		// (a.walling 2008-06-26 09:16) - PLID 30515 - Inform the user if any template updates were skipped
		if (nSkipped > 0) {
			MessageBox(FormatString("Could not update %li templates which are currently being modified by another user.", nSkipped));
		}
	} NxCatchAll("Error in EMNTemplateManager:SelectAll");
}

void CEmrTemplateManagerDlg::OnEmnTemplatesUnselectAll() 
{
	try {
		long p = m_pTemplateList->GetFirstRowEnum();
		LPDISPATCH pDisp = NULL;
		
		long nSkipped = 0;

		while (p) {
			m_pTemplateList->GetNextRowEnum(&p, &pDisp);
			IRowSettingsPtr pRow(pDisp);

			//If it's already checked, don't need to do anything
			BOOL bChecked = VarBool(pRow->GetValue(etmtlcAddOnce), FALSE);

			if(bChecked) {
				//check the box
				pRow->PutValue(etmtlcAddOnce, false);
				long nAffected = 0;
				
				ExecuteParamSql(GetRemoteData(), &nAffected, 
					"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
					"UPDATE EmrTemplateT SET AddOnce = {BIT} WHERE ID = {INT} AND ID NOT IN (SELECT EmnID FROM EMNTemplateAccessT)", FALSE, VarLong(pRow->GetValue(etmtlcID)));

				// (a.walling 2008-06-26 09:15) - PLID 30515 - Could not update, so revert the box
				if (nAffected <= 0) {
					nSkipped++;					
					pRow->PutValue(etmtlcAddOnce, true);
				}
			}

			pDisp->Release();
		}

		// (a.walling 2008-06-26 09:16) - PLID 30515 - Inform the user if any template updates were skipped
		if (nSkipped > 0) {
			MessageBox(FormatString("Could not update %li templates which are currently being modified by another user.", nSkipped));
		}
	} NxCatchAll("Error in EMNTemplateManager:SelectAll");
}

// (j.jones 2014-08-08 16:59) - PLID 63182 - this function had nothing in it, so I removed it
//void CEmrTemplateManagerDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
//{
	// (c.haag 2006-03-01 13:32) - 19208 - We should get table checker
	// messages when templates change
	//

	// (a.walling 2007-03-19 09:08) - PLID 24614 - This is a modal dialog with a minimal on-screen time. We don't
	// really need to refresh the whole thing every time we get a table checker. Even if the message includes
	// a single ID, our design guidelines supposedly state that information should not change right in front
	// of the user. The list here can refresh right in the middle of editing names and other info. 

	/*
	if (wParam == NetUtils::EMRTemplateT) {
		if (NULL != m_pTemplateList) {
			if(!m_pTemplateList->IsRequerying())
				RequeryTemplateList();
		}
	}
	*/
//}

void CEmrTemplateManagerDlg::RequeryTemplateList()
{
	m_nSavedTopRow = m_pTemplateList->TopRowIndex;
	m_pTemplateList->Requery();
}

void CEmrTemplateManagerDlg::OnRequeryFinishedEmrTemplates(short nFlags) 
{
	try {
		if(m_nSavedTopRow != -1)
			m_pTemplateList->TopRowIndex = m_nSavedTopRow;
		m_nSavedTopRow = -1;

		// (b.cardillo 2010-09-23 15:41) - PLID 39568 - Not really related to this pl item per se, but for 
		// efficiency I made row foreground color auto-set from resources instead of looping through after 
		// the requery has finished here.
	}NxCatchAll(__FUNCTION__);
}

void CEmrTemplateManagerDlg::OnKeywordFilter()
{
	try {
		//TES 3/2/2009 - PLID 33102 - Check if we're already filtering
		if(CString((LPCTSTR)m_pTemplateList->WhereClause) == m_strDefaultWhereClause) {
			//TES 3/2/2009 - PLID 33102 - Nope, let's filter.
			if (FilterDatalist(m_pTemplateList, etmtlcName, etmtlcID))
			{
				//Set the filter button's text
				m_btnKeywordFilter.SetWindowText("Unfilter");
			}
		}
		else {
			//TES 3/2/2009 - PLID 33102 - We are, so let's un-filter (including resetting the background
			// color wich FilterDatalist will have changed).
			m_pTemplateList->WhereClause = _bstr_t(m_strDefaultWhereClause);
			for (short i=0; i < m_pTemplateList->ColumnCount; i++) {
				m_pTemplateList->GetColumn(i)->PutBackColor(RGB(255,255,255));
			}
			m_pTemplateList->Requery();
			m_btnKeywordFilter.SetWindowText("Filter");
		}
	}NxCatchAll("Error in CEmrTemplateManagerDlg::OnKeywordFilter()");
}


// (j.gruber 2009-10-30 11:08) - PLID 35806 - set/clear nexweb template
//(e.lally 2011-05-04) PLID 43537 - changed direct nexweb template management to a goto action
void CEmrTemplateManagerDlg::OnGotoNexWebListDisplayManager()
{
	try {
		// (j.gruber 2009-11-17 12:01) - PLID 36106 - check the permission and prompt for password if necessary
		if (CheckCurrentUserPermissions(bioNexwebObjects, sptWrite, FALSE, 0, FALSE, FALSE)  ) {

			//do the flipping
			CMainFrame *pMainFrame = GetMainFrame();
			CNxTabView *pView;
			if(pMainFrame == NULL){
				return;
			}

			if(pMainFrame->FlipToModule(ADMIN_MODULE_NAME)) {
				pView = (CNxTabView *)pMainFrame->GetOpenView(ADMIN_MODULE_NAME);
				if (pView != NULL) {
					if(pView->GetActiveTab()==AdminModule::LinksTab){
						pView->UpdateView();
					}
					else {
						pView->SetActiveTab(AdminModule::LinksTab);
					}
				}
			}

			CNxDialog::OnOK();
		}
			
	}NxCatchAll(__FUNCTION__);
}

void CEmrTemplateManagerDlg::OnEditCustomPreviewLayouts()
{
	try {
		// (c.haag 2012-01-17) - PLID 54593 
		if (-1 != m_pTemplateList->CurSel)
		{
			long nEmrTemplateID = VarLong(m_pTemplateList->GetValue(m_pTemplateList->CurSel, etmtlcID));
			CString strEmrTemplateName = VarString(m_pTemplateList->GetValue(m_pTemplateList->CurSel, etmtlcName));
			CEMRCustomPreviewLayoutsMDIFrame::LaunchWithTemplate(nEmrTemplateID, strEmrTemplateName);
			CDialog::OnOK();
		}
	}NxCatchAll(__FUNCTION__);
}
