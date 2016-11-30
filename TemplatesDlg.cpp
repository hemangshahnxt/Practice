// TemplatesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "TemplatesDlg.h"
#include "TemplateEntryDlg.h"
#include "MainFrm.h"
#include "GlobalUtils.h"
#include "NxSchedulerDlg.h"
#include "NxTabView.h"
#include "NxStandard.h"
#include "ChildFrm.h"
#include "CommonSchedUtils.h"
#include "GlobalDataUtils.h"
#include "NameAndColorEntryDlg.h"
#include "TemplateCollectionEntryDlg.h"

using namespace NXDATALISTLib;
using namespace NexTech_Accessor;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTemplatesDlg dialog


// (z.manning 2014-12-03 14:31) - PLID 64332 - Added param for dialog type
// (z.manning 2014-12-04 09:24) - PLID 64215 - Now takes in the parent template editor dialog
CTemplatesDlg::CTemplatesDlg(ESchedulerTemplateEditorType eType, CWnd* pParent)
	: CNxDialog(CTemplatesDlg::IDD, (CNxView*)pParent)
{
	//{{AFX_DATA_INIT(CTemplatesDlg)
	//}}AFX_DATA_INIT
	m_pdlgScheduler = NULL;
	//TES 6/19/2010 - PLID 39262 - We'll use different permissions depending on whether we're editing Resource Availability templates.
	m_bio = (eType == stetLocation) ? bioResourceAvailTemplating : bioSchedTemplating;
	// (z.manning 2014-12-03 14:32) - PLID 64332 - Set the type
	m_eEditorType = eType;

	switch (eType)
	{
		case stetNormal:
			m_strShowOldTempaltesPropertyName = "ShowOldTemplatesInConfigDlg";
			break;

		case stetLocation:
			m_strShowOldTempaltesPropertyName = "ShowOldResourceAvailTemplatesInConfigDlg";
			break;

		case stetCollection:
			m_strShowOldTempaltesPropertyName = "ShowOldTemplateCollectionsInConfigDlg";
			break;
	}
}


void CTemplatesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTemplatesDlg)
	DDX_Control(pDX, IDC_MOVE_PRIORITY_DOWN_BTN, m_movePriorityDownBtn);
	DDX_Control(pDX, IDC_MOVE_PRIORITY_UP_BTN, m_movePriorityUpBtn);
	DDX_Control(pDX, IDC_ADD_TEMPLATE_BTN, m_btnAdd);
	DDX_Control(pDX, IDC_REMOVE_TEMPLATE_BTN, m_btnRemove);
	DDX_Control(pDX, IDC_EDIT_TEMPLATE_BTN, m_btnEdit);
	DDX_Control(pDX, IDC_CHECK_OLDTEMPLATES, m_btnCheckOldTemplates);
	//}}AFX_DATA_MAP
}

// (z.manning, 07/28/2008) - PLId 30865 - Changed the message map to use CNxDialog instead of
// CDialog, otherwise it won't draw the standard NxDialog white background.
BEGIN_MESSAGE_MAP(CTemplatesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CTemplatesDlg)
	ON_BN_CLICKED(IDC_ADD_TEMPLATE_BTN, OnAddTemplateBtn)
	ON_BN_CLICKED(IDC_REMOVE_TEMPLATE_BTN, OnRemoveTemplateBtn)
	ON_BN_CLICKED(IDC_EDIT_TEMPLATE_BTN, OnEditTemplateBtn)
	ON_BN_CLICKED(IDC_MOVE_PRIORITY_UP_BTN, OnMovePriorityUpBtn)
	ON_BN_CLICKED(IDC_MOVE_PRIORITY_DOWN_BTN, OnMovePriorityDownBtn)
	ON_BN_CLICKED(IDC_CHECK_OLDTEMPLATES, OnCheckOldTemplates)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTemplatesDlg message handlers

void CTemplatesDlg::RequeryTemplateList()
{
	// (c.haag 2005-02-08 12:01) - PLID 15217 - Determine which templates to show
	// based on the user's preference.
	CWaitCursor wc;
	UpdateData(TRUE);

	if (m_eEditorType == stetCollection)
	{
		// (z.manning 2014-12-03 15:15) - PLID 64332 - Load the collections
		long nSelectedCollectionID = -1;
		if (m_lstTemplates->GetCurSel() != -1) {
			nSelectedCollectionID = VarLong(m_lstTemplates->GetValue(m_lstTemplates->GetCurSel(), tlcID));
		}
		// (c.haag 2014-12-15) - PLID 64246 - Select a default collection
		else if (m_bstrDefaultCollectionID.length() > 0)
		{
			nSelectedCollectionID = atol(m_bstrDefaultCollectionID);
		}

		m_lstTemplates->Clear();
		m_mapCollections.clear();
		// (c.haag 2014-12-16) - PLID 64243 - Specify whether to include collections with only past applies
		// (c.haag 2015-01-06) - PLID 64520 - We now use SchedulerTemplateCollectionFilter
		BOOL bExcludeCollectionsWithOnlyPastApplies = (GetRemotePropertyInt(m_strShowOldTempaltesPropertyName, 1, 0, GetCurrentUserName()) != 0);
		_SchedulerTemplateCollectionFilterPtr pCollectionFilter(__uuidof(SchedulerTemplateCollectionFilter));
		pCollectionFilter->ExcludeCollectionsWithOnlyPastApplies = bExcludeCollectionsWithOnlyPastApplies ? VARIANT_TRUE : VARIANT_FALSE;
		_SchedulerTemplateCollectionsPtr pCollections = GetAPI()->GetSchedulerTemplateCollections(GetAPISubkey(), GetAPILoginToken(), Nx::SafeArray<IUnknown*>::FromValue(pCollectionFilter));
		Nx::SafeArray<IUnknown *> saCollections(pCollections->Collections);
		for each (_SchedulerTemplateCollectionPtr pCollection in saCollections)
		{
			// (c.haag 2014-12-15) - PLID 64245 - Use AddCollectionToList
			AddCollectionToList(pCollection);
		}

		if (nSelectedCollectionID != -1) {
			m_lstTemplates->SetSelByColumn(tlcID, nSelectedCollectionID);
		}

		// (z.manning 2015-01-13 09:43) - PLID 64215 - Need to refresh the parent view
		if (GetParent() != NULL) {
			GetParent()->PostMessage(NXM_UPDATEVIEW);
		}
	}
	else
	{
		CString strWhere;
		// (z.manning, 11/15/2006) - The checkbox actually says "Hide" which is why we don't filter
		// when the property is zero.
		// (c.haag 2006-11-15 11:09) - Yes; that was brilliant thinking on my part a long time ago
		//TES 6/19/2010 - PLID 5888 - Check the appropriate preference.
		if (GetRemotePropertyInt(m_strShowOldTempaltesPropertyName, 1, 0, GetCurrentUserName()) == 0) {
			strWhere = "";
		}
		else {
			COleDateTime dtToday = COleDateTime::GetCurrentTime();
			dtToday.SetDate(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay());
			// (z.manning, 12/07/2006) - PLID 23801 - Make sure we hanlde the case where EndDate is NULL.
			//TES 6/19/2010 - PLID 39262 - Check the appropriate tables.
			if (m_eEditorType == stetLocation) {
				strWhere.Format("ID IN (SELECT TemplateID FROM ResourceAvailTemplateItemT WHERE EndDate >= '%s' OR EndDate IS NULL)", FormatDateTimeForSql(dtToday));
			}
			else {
				strWhere.Format("ID IN (SELECT TemplateID FROM TemplateItemT WHERE EndDate >= '%s' OR EndDate IS NULL)", FormatDateTimeForSql(dtToday));
			}
		}

		//TES 6/19/2010 - PLID 39262 - Set the list to pull from the correct table.
		CString strFrom = (m_eEditorType == stetLocation) ? "ResourceAvailTemplateT" : "TemplateT";
		m_lstTemplates->FromClause = _bstr_t(strFrom);
		m_lstTemplates->WhereClause = _bstr_t(strWhere);
		m_lstTemplates->Requery();

		// Wait for the data list to finish requerying to get
		// an accurate row count
		m_lstTemplates->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	}

	for (int i = 0; i < m_lstTemplates->GetRowCount(); i++)
	{
		COleVariant var = m_lstTemplates->GetValue(i, tlcColorValue);
		IRowSettingsPtr pRow = m_lstTemplates->GetRow(i);
		pRow->PutCellBackColor(tlcColorDisplay, var.lVal);
		pRow->PutCellBackColorSel(tlcColorDisplay, var.lVal);
	}

	// (c.haag 2015-01-13) - PLID 64243 - Whenever a collection is modified or the "hide" checkbox are checked,
	// we need to make sure the singleday is cleared controls are disabled if the selection got blown away.
	if (m_eEditorType == stetCollection && -1 == m_lstTemplates->CurSel)
	{
		OnSelChangedTemplatesList(-1);
	}
}

void CTemplatesDlg::OnAddTemplateBtn() 
{
	if (!CheckCurrentUserPermissions(m_bio, sptCreate)) {
		return;
	}

	if (m_eEditorType == stetCollection)
	{
		// (z.manning 2014-12-04 16:05) - PLID 64217 - Handle template collections
		CNameAndColorEntryDlg dlgEntry(this);
		dlgEntry.m_strWindowTitle = "New Template Collection";
		dlgEntry.m_nTextLimit = 50;
		dlgEntry.m_strSqlTable = "TemplateCollectionT";
		dlgEntry.m_strSqlColumn = "Name";
		if (dlgEntry.DoModal() == IDOK)
		{
			_SchedulerTemplateCollectionCommitPtr pCommit(__uuidof(SchedulerTemplateCollectionCommit));
			pCommit->Name = _bstr_t(dlgEntry.m_strName);
			pCommit->Color = _bstr_t(ConvertCOLORREFToHexString(dlgEntry.m_nColor));

			_SchedulerTemplateCollectionPtr pNewCollection = GetAPI()->CreateSchedulerTemplateCollection(GetAPISubkey(), GetAPILoginToken(), pCommit);

			// (c.haag 2014-12-15) - PLID 64245 - Use AddCollectionToList
			long nNewRow = AddCollectionToList(pNewCollection);
			m_lstTemplates->PutCurSel(nNewRow);
			m_lstTemplates->Sort();

			EnableControls();
			if (GetParent() != NULL) {
				GetParent()->PostMessage(NXM_UPDATEVIEW);
			}
		}
	}
	else
	{
		// (z.manning 2014-12-11 16:24) - PLID 64230 - Changed the constructor to include the the location template flag
		CTemplateEntryDlg dlgTemplateEntry((m_eEditorType == stetLocation), this);
		//TES 6/19/2010 - PLID 5888 - Pass in m_bUseResourceAvailTemplates
		if (dlgTemplateEntry.AddTemplate() == IDOK) {
			RequeryTemplateList();
			// (z.manning, 02/27/2007) - PLID 23745 - Only refresh the schedule if they saved the template.
			ReflectTemplateChangesOnSchedule();
		}
		EnableControls();

		// (z.manning, 12/20/2006) - PLID 23937 - Also need to close the parent now that this dialog is a sheet.
		if (dlgTemplateEntry.m_bIsPreviewing) {
			if (GetParent()) {
				GetParent()->PostMessage(WM_CLOSE);
			}
			CDialog::OnOK();
		}
	}
}

// (c.haag 2014-12-15) - PLID 64245 - This should be called any time a collection is added to the list.
// Returns the ordinal of the newly created row.
long CTemplatesDlg::AddCollectionToList(_SchedulerTemplateCollectionPtr pCollection)
{
	// (z.manning 2014-12-04 09:38) - PLID 64215 - Keep track of the collections in a map
	m_mapCollections[AsLong(pCollection->ID)] = pCollection;

	IRowSettingsPtr pNewRow = m_lstTemplates->GetRow(-1);
	pNewRow->PutValue(tlcID, AsLong(pCollection->ID));
	pNewRow->PutValue(tlcColorValue, ConvertHexStringToCOLORREF(pCollection->Color));
	pNewRow->PutCellBackColor(tlcColorDisplay, ConvertHexStringToCOLORREF(pCollection->Color));
	pNewRow->PutCellBackColorSel(tlcColorDisplay, ConvertHexStringToCOLORREF(pCollection->Color));
	pNewRow->PutValue(tlcName, pCollection->Name);
	pNewRow->PutValue(tlcPriority, g_cvarNull);
	return m_lstTemplates->AddRow(pNewRow);
}

// (c.haag 2014-12-15) - PLID 64246 - Set a default collection ID
void CTemplatesDlg::SetDefaultCollectionID(_bstr_t bstrCollectionID)
{
	m_bstrDefaultCollectionID = bstrCollectionID;
}

BOOL CTemplatesDlg::OnInitDialog()
{
	try
	{
		// (a.walling 2008-05-23 12:55) - PLID 30099
		CNxDialog::OnInitDialog();

		//TES 6/19/2010 - PLID 5888 - Check the appropriate preference.
		SetDlgItemCheck(IDC_CHECK_OLDTEMPLATES, GetRemotePropertyInt(m_strShowOldTempaltesPropertyName, 1, 0, GetCurrentUserName()));

		// Connect our variable to the control
		m_lstTemplates = BindNxDataListCtrl(IDC_TEMPLATES_LIST, false);

		// (z.manning 2014-12-03 15:18) - PLID 64332 - Special handling for collection editing
		if (m_eEditorType == stetCollection)
		{
			m_lstTemplates->GetColumn(tlcName)->PutColumnTitle("Collection Name");
			m_lstTemplates->GetColumn(tlcName)->PutSortPriority(0);

			GetDlgItem(IDC_MOVE_PRIORITY_UP_BTN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOVE_PRIORITY_UP_BTN)->EnableWindow(FALSE);
			GetDlgItem(IDC_MOVE_PRIORITY_DOWN_BTN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MOVE_PRIORITY_DOWN_BTN)->EnableWindow(FALSE);

			// (c.haag 2014-12-16) - PLID 64243 - Added bulk caching
			g_propManager.CachePropertiesInBulk("CTemplatesDlg", propNumber,
				"Username IN ('<None>', '%s') AND Name IN ( \r\n"
				"	'ShowOldTemplatesInConfigDlg' \r\n"
				"	,'ShowOldResourceAvailTemplatesInConfigDlg' \r\n"
				"	,'ShowOldTemplateCollectionsInConfigDlg' \r\n"
				"	) \r\n"
				, _Q(GetCurrentUserName()));
		}

		RequeryTemplateList();

		// Set the button icons appropriately
		//	extern CPracticeApp theApp;
		//	((CButton *)GetDlgItem(IDC_MOVE_PRIORITY_UP_BTN))->SetIcon(theApp.LoadIcon(IDI_UP_BTN_ICON));
		//	((CButton *)GetDlgItem(IDC_MOVE_PRIORITY_DOWN_BTN))->SetIcon(theApp.LoadIcon(IDI_DOWN_BTN_ICON));
		m_movePriorityUpBtn.AutoSet(NXB_UP);
		m_movePriorityDownBtn.AutoSet(NXB_DOWN);
		// (z.manning, 04/29/2008) - PLID 29814 - More button styles
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnEdit.AutoSet(NXB_MODIFY);
		m_btnRemove.AutoSet(NXB_DELETE);

		// (c.haag 2014-12-16) - PLID 64243 - Change the "old templates" checkbox name
		if (m_eEditorType == stetCollection)
		{
			m_btnCheckOldTemplates.SetWindowText("Hide collections with only past applies");
		}

		// (z.manning 2009-07-10 17:02) - PLID 22054 - Ensure main frame exists
		if (GetMainFrame() != NULL) {
			// Get a pointer to the active Scheduler sheet if there is an open one
			CChildFrame *frmSched = GetMainFrame()->GetOpenViewFrame(SCHEDULER_MODULE_NAME);
			if (frmSched) {
				CNxTabView *viewSched = (CNxTabView *)frmSched->GetActiveView();
				// (j.jones 2012-08-08 10:11) - PLID 51063 - added check that the active view is non-null
				if (viewSched) {
					m_pdlgScheduler = (CNxSchedulerDlg *)viewSched->GetActiveSheet();
				}
			}
		}

		// Enable/Disable the appropriate buttons
		EnableControls();
		GetControlPositions();
	}
	NxCatchAll("CTemplatesDlg::OnInitDialog");
	return TRUE;
}

void CTemplatesDlg::OnRemoveTemplateBtn()
{
	try {

		//TES 2/20/2015 - PLID 64336 - Check m_bio, not bioSchedTemplating
		if (!CheckCurrentUserPermissions(m_bio, sptDelete))
			return;

		// (c.haag 2014-12-12 10:02) - PLID 64244 - Act based on our editor type
		switch (m_eEditorType)
		{
		case stetCollection:
			OnRemoveCollection();
			break;

		default:
			OnRemoveTemplate();
			break;
		}
	}
	NxCatchAll(__FUNCTION__);
	EnableControls();
	ReflectTemplateChangesOnSchedule();
}

// (c.haag 2014-12-12 10:02) - PLID 64244 - Invoked to remove a collection
void CTemplatesDlg::OnRemoveCollection()
{
	long nCurSel = m_lstTemplates->CurSel;
	if (nCurSel != -1) {
		if (IDYES == MessageBox(R"(This will permanently delete this collection and all of its applies. This action is not recoverable!
			
Are you sure you wish to continue?)", "Confirm Delete", MB_YESNO | MB_ICONWARNING))
		{
			GetAPI()->DeleteSchedulerTemplateCollection(GetAPISubkey(), GetAPILoginToken(), _bstr_t(VarLong(m_lstTemplates->Value[nCurSel][0])));
			m_lstTemplates->RemoveRow(nCurSel);
			EnableControls();
		}
	}
}

// (c.haag 2014-12-12 10:02) - PLID 64244 - Invoked to remove a template. Moved from OnRemoveTemplateBtn
void CTemplatesDlg::OnRemoveTemplate()
{
	long nCurSel = m_lstTemplates->CurSel;
	if (nCurSel != -1) {

		// (c.haag 2006-12-11 12:07) - PLID 23808 - Gracefully fail if the template no longer exists
		//TES 6/19/2010 - PLID 5888 - Pull from the correct table.
		if (!ReturnsRecordsParam(FormatString("SELECT TOP 1 ID FROM %sTemplateT WHERE ID = {INT}", (m_eEditorType == stetLocation) ? "ResourceAvail" : ""), VarLong(m_lstTemplates->Value[nCurSel][0]))) {
			MessageBox("This template has been deleted by another user.");
			ReflectTemplateChangesOnSchedule();
			return;
		}

		long nTemplateID = VarLong(m_lstTemplates->Value[nCurSel][0]);
		// (c.haag 2014-01-06) - PLID 64520 - Warn if this template is used in a collection
		//TES 3/3/2015 - PLID 64336 - Don't do this check if this is a location template
		if (m_eEditorType == stetNormal) {			
			_SchedulerTemplateCollectionFilterPtr pCollectionFilter(__uuidof(SchedulerTemplateCollectionFilter));
			pCollectionFilter->TemplateIDs = Nx::SafeArray<BSTR>::FromValue(_bstr_t(nTemplateID));
			_SchedulerTemplateCollectionsPtr pCollections = GetAPI()->GetSchedulerTemplateCollections(GetAPISubkey(), GetAPILoginToken(), Nx::SafeArray<IUnknown*>::FromValue(pCollectionFilter));
			Nx::SafeArray<IUnknown *> saCollections(pCollections->Collections);
			if (saCollections.GetLength() > 0)
			{
				CString strWarning = R"(This scheduler template exists in the following template collections:

	)";
				ULONG count = 0;
				for each (_SchedulerTemplateCollectionPtr pCollection in saCollections)
				{
					strWarning += (LPCTSTR)pCollection->Name;
					strWarning += "\r\n";
					if (++count == 10 && saCollections.GetLength() > count)
					{
						strWarning += "<more>\r\n";
						break;
					}
				}
				strWarning += R"(
	Deleting this template will remove it from any existing collection and/or collection applies. Are you sure you wish to continue?)";

				if (IDNO == MessageBox(strWarning, NULL, MB_ICONQUESTION | MB_YESNO))
				{
					return;
				}
			}
		}

		// (z.manning, 01/23/2007) - PLID 24392 - No longer needlessly create a CTemplateEntryDlg here.
		//TES 6/19/2010 - PLID 5888 - Resource Availability templates don't have rules
		int nResult = MessageBox("Are you sure you want to permanently delete this template" + CString((m_eEditorType == stetLocation) ? "" : " and all its rules") + "?", "Confirm Delete", MB_YESNO | MB_ICONQUESTION);
		if (nResult == IDYES) {
			//TES 6/19/2010 - PLID 5888 - Make sure we remove from the correct list.
			if (m_eEditorType == stetLocation) {
				RemoveResourceAvailTemplate(nTemplateID);
			}
			else {
				RemoveTemplate(nTemplateID);
			}
			//TES 5/10/2006 - No need to requery the whole list here.
			m_lstTemplates->RemoveRow(nCurSel);
		}
	}
}

void CTemplatesDlg::OnEditTemplateBtn()
{
	/* Might do this the right way right now
	// ToDo: remove this and do it the right way //////
	int nPos = GetSelPosition();
	if (nPos != -1) {
		COleVariant var;
		var = m_ctrlTemplatesList.GetRecordItem(nPos, 1);
		if (var.vt != VT_NULL && var.vt != VT_EMPTY) {
			if (strcmp(var.pbVal, STRING_OFFICE_HOURS_TEMPLATE_NAME) == 0) {
				// If the user is trying to edit the office hours, don't let her
				MsgBox("I'm sorry, the 'Office Hours' template is currently managed by the system.\nIf you would like to change you're scheduled hours of operation, please use the Administration Module.  
	////////////////////////////////////////////////
	*/

	CWaitCursor wc;

	if (m_eEditorType == stetCollection)
	{
		// (z.manning 2014-12-10 10:58) - PLID 64228 - Open the collection entry dialog

		_SchedulerTemplateCollectionPtr pCollection = GetSelectedCollection();
		if (pCollection == NULL) {
			return;
		}
		
		//TES 2/20/2015 - PLID 64336 - Check m_bio, not bioSchedTemplating
		if (!CheckCurrentUserPermissions(m_bio, sptWrite)) {
			return;
		}

		CTemplateCollectionEntryDlg dlgCollectionEntry(this);
		if (dlgCollectionEntry.EditCollection(pCollection) == IDOK)
		{
			RequeryTemplateList();
		}
	}
	else
	{
		BOOL bAllowEdit = FALSE;

		//TES 2/20/2015 - PLID 64336 - Check m_bio, not bioSchedTemplating
		if (GetCurrentUserPermissions(m_bio) & sptWrite)
			bAllowEdit = TRUE;
		else if ((GetCurrentUserPermissions(m_bio) & sptWriteWithPass) && CheckCurrentUserPassword())
			bAllowEdit = TRUE;

		if (!bAllowEdit) {
			AfxMessageBox("You do not have permission to edit templates. You will only be able to view the template settings.");
		}

		// Otherwise open the template for editing
		long nTempID = GetSelTemplateID();

		if (nTempID != -1) {

			// (c.haag 2006-12-11 12:07) - PLID 23808 - Gracefully fail if the template no longer exists
			//TES 6/19/2010 - PLID 5888 - Pull from the correct table.
			if (!ReturnsRecordsParam(FormatString("SELECT TOP 1 ID FROM %sTemplateT WHERE ID = {INT}", (m_eEditorType == stetLocation) ? "ResourceAvail" : ""), nTempID)) {
				MsgBox("This template has been deleted by another user.");
				ReflectTemplateChangesOnSchedule();
				return;
			}

			// (z.manning 2014-12-11 16:24) - PLID 64230 - Changed the constructor to include the the location template flag
			CTemplateEntryDlg dlgTemplateEntry((m_eEditorType == stetLocation), this);
			dlgTemplateEntry.m_bAllowEdit = bAllowEdit;
			//TES 6/19/2010 - PLID 5888 - Pass in m_bUseResourceAvailTemplates
			if (dlgTemplateEntry.EditTemplate(nTempID) == IDOK) {
				RequeryTemplateList();
				// (z.manning, 02/27/2007) - PLID 23745 - Only refresh the schedule if they saved the template.
				ReflectTemplateChangesOnSchedule();
			}

			// (z.manning, 12/20/2006) - PLID 23937 - Also need to close the parent now that this dialog is a sheet.
			if (dlgTemplateEntry.m_bIsPreviewing) {
				if (GetParent()) {
					GetParent()->PostMessage(WM_CLOSE);
				}
				CDialog::OnOK();
			}
		}
		EnableControls();
	}
}

// (c.haag 2014-12-15) - PLID 64245 - Called when the user elects to copy the selected collection
void CTemplatesDlg::OnCopyCollectionBtn()
{
	_SchedulerTemplateCollectionPtr pCollection = GetSelectedCollection();
	if (pCollection == NULL) {
		return;
	}

	// Check permissions
	//TES 2/20/2015 - PLID 64336 - Check m_bio, not bioSchedTemplating
	if (!CheckCurrentUserPermissions(m_bio, sptCreate)) {
		return;
	}

	CNameAndColorEntryDlg dlgEntry(this);
	dlgEntry.m_strWindowTitle = "New Template Collection";
	dlgEntry.m_nTextLimit = 50;
	dlgEntry.m_strSqlTable = "TemplateCollectionT";
	dlgEntry.m_strSqlColumn = "Name";

	if (dlgEntry.DoModal() == IDOK)
	{
		_SchedulerTemplateCollectionPtr pNewCollection = GetAPI()->CopySchedulerTemplateCollection(GetAPISubkey(), GetAPILoginToken(),
			pCollection->ID, _bstr_t(dlgEntry.m_strName), _bstr_t(ConvertCOLORREFToHexString(dlgEntry.m_nColor)));

		long nNewRow = AddCollectionToList(pNewCollection);
		m_lstTemplates->PutCurSel(nNewRow);
		m_lstTemplates->Sort();

		if (GetParent() != NULL) {
			GetParent()->PostMessage(NXM_UPDATEVIEW);
		}
	}
}

// Returns the Template ID referred to by nRow1
long CTemplatesDlg::SwapTemplates(long nRow1, long nRow2)
{
	if (m_eEditorType == stetCollection) {
		// (z.manning 2014-12-03 14:42) - PLID 64332 - The buttons should not be visible when editing collections
		ASSERT(FALSE);
		return -1;
	}

	long lCurSel = m_lstTemplates->Value[nRow1][tlcID];
	long lSel2 = m_lstTemplates->Value[nRow2][tlcID];
	long lPri1 = m_lstTemplates->Value[nRow1][tlcPriority];
	long lPri2 = m_lstTemplates->Value[nRow2][tlcPriority];

	//TES 6/19/2010 - PLID 5888 - Make sure we're updating the correct tables throughout.
	CString strTablePrefix = (m_eEditorType == stetLocation) ? "ResourceAvail" : "";
	// (j.jones 2006-09-21 09:20) - PLID 22631 - if they are the same priority,
	// (which is bad data) we need to force them to have different ones
	if(lPri1 == lPri2) {
		long nPivotID, nPivotPriority;
		long nFixedID, nFixedPriority;
		if(nRow1 < nRow2) {
			//they want to move nRow1 down
			nPivotID = lSel2;
			nPivotPriority = lPri2;
			nFixedID = lCurSel;
			nFixedPriority = lPri1;
		}
		else { //they want to move nRow1 up
			nPivotID = lCurSel;
			nPivotPriority = lPri1;
			nFixedID = lSel2;
			nFixedPriority = lPri2;
		}

		//remember, the priority for templates sorts by highest number to lowest,
		//ie. 10 comes before 9.

		//the "pivot" variables mean we are going to only alter what ends up to be the
		//first of the two swapped templates, and potentially upping the
		//priority of all previous templates
		//the "fixed" variables mean we are not altering the second of the two swapped
		//templates, but we may potentially up the priority of all previous templates

		nPivotPriority++;
		ExecuteSql("UPDATE %sTemplateT SET Priority = %li WHERE ID = %li",
			strTablePrefix, nPivotPriority, nPivotID);

		//If any template's priority now conflicts with the templates we just swapped,
		//that template and all those that precede it need to have their priority increased by 1.
		//This ensures that the templates they are swapping will still be positioned properly
		//amongst the other templates. At the same time, we can't determine the intended order of
		//any other templates that share priorities, so those templates remain. We are
		//only correcting the templates they are intentionally moving.

		//examples:
		//ex 1: four templates are all priority 1, we swapped the first two,
		//result: the templates have priorities 3, 2, 1, 1.
		//ex 2: your four templates have priorities of 3, 2, 2, 1, and we swapped the middle two,
		//result: the templates have priorities 4, 3, 2, 1.
		//ex 3: your four templates have priorities of 2, 2, 1, 1 , and we swapped the middle two
		//result: the templates still have priorities of 2, 2, 1, 1, but the middle two are the same, this
		//code would have never been fired

		//fix conflicts with our fixed template priority
		if(ReturnsRecordsParam(FormatString("SELECT Priority FROM %sTemplateT WHERE Priority = {INT} AND ID <> {INT}", strTablePrefix), nFixedPriority, nFixedID)) {
			ExecuteParamSql(FormatString("UPDATE %sTemplateT SET Priority = Priority + 1 WHERE Priority > {INT} OR ID = {INT}", strTablePrefix), nFixedPriority, nFixedID);
			//we would have increased our pivot priority by 1
			nPivotPriority++;
		}
		//fix conflicts with our pivot template priority (some of which may have been caused by the previous statement)
		if(ReturnsRecordsParam(FormatString("SELECT Priority FROM %sTemplateT WHERE Priority = {INT} AND ID <> {INT}", strTablePrefix), nPivotPriority, nPivotID)) {
			ExecuteParamSql(FormatString("UPDATE %sTemplateT SET Priority = Priority + 1 WHERE Priority >= {INT} AND ID <> {INT}", strTablePrefix), nPivotPriority, nPivotID);
		}

		// (c.haag 2006-12-12 10:58) - PLID 23808 - Notify other computers about the change.
		//TES 6/21/2010 - PLID 5888 - Send the appropriate tablechecker
		CClient::RefreshTable((m_eEditorType == stetLocation) ? NetUtils::ResourceAvailTemplateT : NetUtils::TemplateT, -1);

		//we likely changed many templates, just requery the whole list and leave
		RequeryTemplateList();
		return lCurSel;
	}

	//if we get here, these templates did not have bad data so proceed normally

	ExecuteSql("UPDATE %sTemplateT SET Priority = %li WHERE ID = %li",
		strTablePrefix, lPri2, lCurSel);

	ExecuteSql("UPDATE %sTemplateT SET Priority = %li WHERE ID = %li",
		strTablePrefix, lPri1, lSel2);
	
	m_lstTemplates->Value[nRow1][4] = lPri2;
	m_lstTemplates->Value[nRow2][4] = lPri1;
	m_lstTemplates->Sort();

	// (c.haag 2006-12-12 10:58) - PLID 23808 - Notify other computers about the change.
	//TES 6/21/2010 - PLID 5888 - Send the appropriate tablechecker
	CClient::RefreshTable((m_eEditorType == stetLocation) ? NetUtils::ResourceAvailTemplateT : NetUtils::TemplateT, -1);

	// Make sure the original one is still selected
	//return nTempId1;
	return lCurSel;
}

void CTemplatesDlg::OnMovePriorityUpBtn() 
{
	try {

		//TES 2/20/2015 - PLID 64336 - Check m_bio, not bioSchedTemplating
		if(!CheckCurrentUserPermissions(m_bio, sptWrite))
			return;

		long nCurSel = m_lstTemplates->CurSel;
		if (nCurSel != -1) {
			long nSwapWithRow = nCurSel-1;
			if (nSwapWithRow >= 0) {
				// Switch the templates in the data and on screen
				long nTemplateId = SwapTemplates(nCurSel, nSwapWithRow);
				// Make sure the original one is still selected
				SetSelTemplateID(nTemplateId);
			}
		}
	} NxCatchAllCall("CTemplatesDlg::OnMovePriorityUpBtn", return);
	EnableControls();
	ReflectTemplateChangesOnSchedule();
}

void CTemplatesDlg::OnMovePriorityDownBtn() 
{
	try {

		//TES 2/20/2015 - PLID 64336 - Check m_bio, not bioSchedTemplating
		if(!CheckCurrentUserPermissions(m_bio, sptWrite))
			return;

		long nCurSel = m_lstTemplates->CurSel;
		if (nCurSel != -1) {
			long nSwapWithRow = nCurSel+1;
			if (nSwapWithRow < m_lstTemplates->GetRowCount()) {
				// Switch the templates in the data and on screen
				long nTemplateId = SwapTemplates(nCurSel, nSwapWithRow);
				// Make sure the original one is still selected
				SetSelTemplateID(nTemplateId);
			}
		}
	} NxCatchAllCall("CTemplatesDlg::OnMovePriorityDownBtn", return);
	EnableControls();
	ReflectTemplateChangesOnSchedule();
}

BEGIN_EVENTSINK_MAP(CTemplatesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CTemplatesDlg)
	ON_EVENT(CTemplatesDlg, IDC_TEMPLATES_LIST, 2 /* SelChanged */, OnSelChangedTemplatesList, VTS_I4)
	ON_EVENT(CTemplatesDlg, IDC_TEMPLATES_LIST, 4 /* LButtonDown */, OnLButtonDownTemplatesList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CTemplatesDlg, IDC_TEMPLATES_LIST, 3 /* DblClickCell */, OnDblClickCellTemplatesList, VTS_I4 VTS_I2)
	ON_EVENT(CTemplatesDlg, IDC_TEMPLATES_LIST, 7 /* RButtonUp */, OnRButtonUpTemplatesList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CTemplatesDlg::OnSelChangedTemplatesList(long nRow) 
{
	EnableControls();

	if (m_eEditorType == stetCollection)
	{
		// (z.manning 2014-12-04 10:08) - PLID 64215 - Need to load the collection when in collection mode
		// so let our parent know to refresh.
		if (GetParent()) {
			GetParent()->PostMessage(NXM_UPDATEVIEW);
		}
	}
}

void CTemplatesDlg::OnLButtonDownTemplatesList(long nRow, short nCol, long x, long y, long nFlags) 
{
/*	if (nCol == 2 && nRow < m_lstTemplates->GetRowCount())
	{
		COleVariant var = m_lstTemplates->GetValue(nRow, 1);
		COLORREF clr = var.lVal;

//		m_ctrlColorPicker.SetColor(clr);
//		m_ctrlColorPicker.ShowColor();
//		clr = m_ctrlColorPicker.GetColor();
	}

	ApplyState(nRow);*/
}

void CTemplatesDlg::EnableControls()
{
	long nRow = m_lstTemplates->CurSel;
	//TES 2/20/2015 - PLID 64336 - Check m_bio, not bioSchedTemplating
	if(!(GetCurrentUserPermissions(m_bio) & SPT____C_______ANDPASS))
		GetDlgItem(IDC_ADD_TEMPLATE_BTN)->EnableWindow(FALSE);

	//TES 2/20/2015 - PLID 64336 - Check m_bio, not bioSchedTemplating
	if (!(GetCurrentUserPermissions(m_bio) & SPT_____D______ANDPASS))
		GetDlgItem(IDC_REMOVE_TEMPLATE_BTN)->EnableWindow(FALSE);

	//TES 2/20/2015 - PLID 64336 - Check m_bio, not bioSchedTemplating
	if (!(GetCurrentUserPermissions(m_bio) & SPT___W________ANDPASS)) {
		GetDlgItem(IDC_EDIT_TEMPLATE_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_MOVE_PRIORITY_UP_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_MOVE_PRIORITY_DOWN_BTN)->EnableWindow(FALSE);
	}

	// First enable and disable buttons on this form appropriately
	if (nRow != -1) {
		long nTempID = m_lstTemplates->Value[nRow][0];
		GetDlgItem(IDC_EDIT_TEMPLATE_BTN)->EnableWindow(TRUE);

		//TES 2/20/2015 - PLID 64336 - Check m_bio, not bioSchedTemplating
		if (GetCurrentUserPermissions(m_bio) & SPT_____D______ANDPASS)
			GetDlgItem(IDC_REMOVE_TEMPLATE_BTN)->EnableWindow(TRUE);

		if (nRow >= 1) {
			//TES 2/20/2015 - PLID 64336 - Check m_bio, not bioSchedTemplating
			if (GetCurrentUserPermissions(m_bio) & SPT___W________ANDPASS)
				GetDlgItem(IDC_MOVE_PRIORITY_UP_BTN)->EnableWindow(TRUE);
		} else {
			GetDlgItem(IDC_MOVE_PRIORITY_UP_BTN)->EnableWindow(FALSE);
		}
		if (nRow <= (m_lstTemplates->GetRowCount()-1-1)) {
			//TES 2/20/2015 - PLID 64336 - Check m_bio, not bioSchedTemplating
			if (GetCurrentUserPermissions(m_bio) & SPT___W________ANDPASS)
				GetDlgItem(IDC_MOVE_PRIORITY_DOWN_BTN)->EnableWindow(TRUE);
		} else {
			GetDlgItem(IDC_MOVE_PRIORITY_DOWN_BTN)->EnableWindow(FALSE);
		}
	} else {
		GetDlgItem(IDC_EDIT_TEMPLATE_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_REMOVE_TEMPLATE_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_MOVE_PRIORITY_UP_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_MOVE_PRIORITY_DOWN_BTN)->EnableWindow(FALSE);
	}
}

long CTemplatesDlg::GetSelTemplateID()
{
	long nCurSel = m_lstTemplates->CurSel;
	if (nCurSel != -1) {
		return VarLong(m_lstTemplates->Value[nCurSel][0]);
	} else {
		return -1;
	}
}

long CTemplatesDlg::SetSelTemplateID(long nTemplateID)
{
	_variant_t varTemplateID(nTemplateID, VT_I4);
	if (m_lstTemplates->SetSelByColumn(0, varTemplateID) >= 0) {
		return nTemplateID;
	} else {
		return -1;
	}
}

void CTemplatesDlg::OnDblClickCellTemplatesList(long nRowIndex, short nColIndex) 
{
	if (nRowIndex != -1 && nRowIndex == m_lstTemplates->CurSel) {
		OnEditTemplateBtn();
	}
}

void CTemplatesDlg::OnCheckOldTemplates() 
{
	int nChecked = ((CButton*)GetDlgItem(IDC_CHECK_OLDTEMPLATES))->GetCheck();
	//TES 6/19/2010 - PLID 5888 - Check the appropriate preference.
	SetRemotePropertyInt(m_strShowOldTempaltesPropertyName, nChecked ? 1 : 0, 0, GetCurrentUserName());
	RequeryTemplateList();
}

void CTemplatesDlg::ReflectTemplateChangesOnSchedule()
{
	//Apply changes to the visible singleday control if possible
	if (m_pdlgScheduler) {
		// (z.manning, 12/06/2006) - PLID 23138 - Just update the entire view.
		m_pdlgScheduler->m_bNeedUpdate = true;
		m_pdlgScheduler->PostMessage(NXM_UPDATEVIEW);
	}
	
	// (z.manning, 11/14/2006) - PLID 23443 - Also need to refresh the parent if we have one.
	if(GetParent()) {
		GetParent()->PostMessage(NXM_UPDATEVIEW);
	}
}

void CTemplatesDlg::OnCancel() 
{
	// (z.manning, 11/15/2006) - PLID 7555 - Since the dialog is now primarily used as a sheet on other
	// dialogs, don't allow them to cancel it (such as when they hit the Esc key).
}

void CTemplatesDlg::OnSize(UINT nType, int cx, int cy) 
{
	CNxDialog::OnSize(nType, cx, cy);
	
	SetControlPositions();	
}

int CTemplatesDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{	
	try {
		
		// (a.walling 2008-05-23 12:55) - PLID 30099
		if (CNxDialog::OnCreate(lpCreateStruct) == -1) {
			return -1;
		}
	
		GetControlPositions();

	}NxCatchAll("CTemplatesDlg::OnCreate");
	
	return 0;
}

// (z.manning 2014-12-04 11:19) - PLID 64215
_SchedulerTemplateCollectionPtr CTemplatesDlg::GetSelectedCollection()
{
	_SchedulerTemplateCollectionPtr pCollection = NULL;
	long nCurrentRow = m_lstTemplates->GetCurSel();
	if (nCurrentRow != -1)
	{
		long nCollectionID = VarLong(m_lstTemplates->GetValue(nCurrentRow, tlcID));
		pCollection = m_mapCollections[nCollectionID];
	}

	return pCollection;
}

// (z.manning 2014-12-17 14:01) - PLID 64427
NexTech_Accessor::_SchedulerTemplateCollectionTemplatePtr CTemplatesDlg::GetCollectionTemplateByID(const long nCollectionTemplateID)
{
	_SchedulerTemplateCollectionPtr pCollection = GetSelectedCollection();
	if (pCollection == NULL) {
		return NULL;
	}

	Nx::SafeArray<IUnknown*> saCollectionTemplates(pCollection->Templates);
	for each (_SchedulerTemplateCollectionTemplatePtr pCollectionTemplate in saCollectionTemplates)
	{
		if (AsLong(pCollectionTemplate->ID) == nCollectionTemplateID) {
			return pCollectionTemplate;
		}
	}

	return NULL;
}

// (z.manning 2014-12-10 16:43) - PLID 64228
void CTemplatesDlg::OnRButtonUpTemplatesList(long nRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		if (nRow == -1) {
			return;
		}

		m_lstTemplates->PutCurSel(nRow);
		EnableControls();

		enum EMenuOptions {
			miEdit = 1,
			miCopyCollection, // (c.haag 2014-12-15) - PLID 64245
			miDelete,
		};

		CMenu mnuPopup;
		mnuPopup.CreatePopupMenu();
		mnuPopup.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, miEdit, "Edit...");
		// (c.haag 2014-12-15) - PLID 64245 - Copying template collections
		if (stetCollection == m_eEditorType)
		{
			mnuPopup.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, miCopyCollection, "Copy Collection...");
		}
		mnuPopup.AppendMenu(MF_SEPARATOR | MF_BYPOSITION);
		mnuPopup.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, miDelete, "Delete...");
		mnuPopup.SetDefaultItem(miEdit);

		CPoint pt;
		GetCursorPos(&pt);

		int nResult = mnuPopup.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, this);
		switch (nResult)
		{
			case miEdit:
				OnEditTemplateBtn();
				break;

			case miCopyCollection:
				OnCopyCollectionBtn();
				break;

			case miDelete:
				OnRemoveTemplateBtn();
				break;
		}
	}
	NxCatchAll(__FUNCTION__);
}

BOOL CTemplatesDlg::OnEraseBkgnd(CDC* pDC)
{
	if (g_brTemplateCollectionBackground.m_hObject != NULL)
	{
		// (z.manning 2015-01-15 09:42) - PLID 64210 - We use a custom background color here
		// (Note: no exception handling here because we don't want to hinder drawing performance)
		CRect rcClient;
		GetClientRect(rcClient);
		pDC->FillRect(rcClient, &g_brTemplateCollectionBackground);

		//TRUE = No further erasing is required
		return TRUE;
	}
	else {
		return CNxDialog::OnEraseBkgnd(pDC);
	}
}

HBRUSH CTemplatesDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (g_brTemplateCollectionBackground.m_hObject != NULL) {
		// (z.manning 2015-01-15 11:22) - PLID 64210 - Return our custom brush if we have one
		return g_brTemplateCollectionBackground;
	}
	else {
		return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
}
