// TemplateEntryDlg.cpp : implementation file
//
// (c.haag 2007-02-21 14:53) - PLID 23784 - The three listbox form controls have been replaced with
// NxDataList 2.0's
//

#include "stdafx.h"
#include "Practice.h"
#include "TemplateEntryDlg.h"
#include "TemplateLineItemDlg.h"
#include "GlobalUtils.h"
#include "PracProps.h"
#include "NxStandard.h"
#include "RuleEntryDlg.h"
#include "GlobalDataUtils.h"
#include "TemplateRuleInfo.h"
#include "AuditTrail.h"
#include "DateTimeUtils.h"
#include "Reports.h"
#include "ReportInfo.h"
#include "GlobalReportUtils.h"
#include "InternationalUtils.h"
#include "CommonSchedUtils.h"
#include "TemplateExceptionInfo.h"
#include "SchedulerRc.h"
#include "MsgBox.h"
#include "GlobalSchedUtils.h"
#include "TemplateItemEntryGraphicalDlg.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef enum {
	ecliData = 0,
	ecliDescription,
	ecliOriginalData, // (z.manning 2011-12-07 16:32) - PLID 46906
} ELineItemColumns;

typedef enum {
	ecrData = 0,
	ecrDescription,
	ecrOriginalData, // (z.manning 2011-12-07 16:32) - PLID 46906
} ERuleColumns;

typedef enum {
	eceData = 0,
	eceDescription,
	eceOriginalData, // (z.manning 2011-12-07 16:32) - PLID 46906
} EExceptionColumns;

/////////////////////////////////////////////////////////////////////////////
// CTemplateEntryDlg dialog


// (z.manning 2014-12-11 09:33) - PLID 64230 - Added bUseResourceAvailTemplates
CTemplateEntryDlg::CTemplateEntryDlg(bool bUseResourceAvailTemplates, CWnd* pParent /*=NULL*/)
	: CNxDialog(CTemplateEntryDlg::IDD, pParent)
	, m_dlgLineItem(bUseResourceAvailTemplates ? stetLocation : stetNormal, this)
	, m_dlgException(this)
{
	//{{AFX_DATA_INIT(CTemplateEntryDlg)
	m_strTemplateName = _T("");
	m_bAllowEdit = TRUE;
	m_bIsPreviewing = FALSE;
	//}}AFX_DATA_INIT

	// Initialize the template id to -1 for a new template
	m_nTemplateID = -1;
	
	m_nColor = 255;

	m_bUseResourceAvailTemplates = bUseResourceAvailTemplates;
	m_nPendingLocationID = -1;
}


void CTemplateEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTemplateEntryDlg)
	DDX_Text(pDX, IDC_TEMPLATE_NAME_EDIT, m_strTemplateName);
	DDX_Control(pDX, IDC_COLOR_PICKER_CTRL, m_ctrlColorPicker);
	DDX_Control(pDX, IDC_TEMPLATE_NAME_EDIT, m_nxeditTemplateNameEdit);
	DDX_Control(pDX, IDC_ADD_LINE_ITEM_BTN, m_btnAddLineItem);
	DDX_Control(pDX, IDC_EDIT_LINE_ITEM_BTN, m_btnEditLineItem);
	DDX_Control(pDX, IDC_REMOVE_LINE_ITEM_BTN, m_btnRemoveLineItem);
	DDX_Control(pDX, IDC_ADD_RULE_BTN, m_btnAddRule);
	DDX_Control(pDX, IDC_EDIT_RULE_BTN, m_btnEditRule);
	DDX_Control(pDX, IDC_REMOVE_RULE_BTN, m_btnRemoveRule);
	DDX_Control(pDX, IDC_ADD_EXCEPTION_BTN, m_btnAddException);
	DDX_Control(pDX, IDC_EDIT_EXCEPTION_BTN, m_btnEditException);
	DDX_Control(pDX, IDC_REMOVE_EXCEPTION_BTN, m_btnRemoveException);
	DDX_Control(pDX, IDC_BTN_PREVIEW_REPORT, m_btnPreviewReport);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CHECK_HIDE_OLD_LINEITEMS, m_checkHideOldLineItems);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTemplateEntryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CTemplateEntryDlg)
	ON_BN_CLICKED(IDC_ADD_LINE_ITEM_BTN, OnAddLineItemBtn)
	ON_BN_CLICKED(IDC_EDIT_LINE_ITEM_BTN, OnEditLineItemBtn)
	ON_BN_CLICKED(IDC_REMOVE_LINE_ITEM_BTN, OnRemoveLineItemBtn)
	ON_BN_CLICKED(IDC_CHOOSE_COLOR_BTN, OnChooseColorBtn)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_ADD_RULE_BTN, OnAddRuleBtn)
	ON_BN_CLICKED(IDC_EDIT_RULE_BTN, OnEditRuleBtn)
	ON_BN_CLICKED(IDC_REMOVE_RULE_BTN, OnRemoveRuleBtn)
	ON_BN_CLICKED(IDC_BTN_PREVIEW_REPORT, OnBtnPreviewReport)
	ON_BN_CLICKED(IDC_ADD_EXCEPTION_BTN, OnAddExceptionBtn)
	ON_BN_CLICKED(IDC_EDIT_EXCEPTION_BTN, OnEditExceptionBtn)
	ON_BN_CLICKED(IDC_REMOVE_EXCEPTION_BTN, OnRemoveExceptionBtn)
	ON_BN_CLICKED(IDC_CHECK_HIDE_OLD_LINEITEMS, OnCheckHideOldLineitems)
	ON_BN_CLICKED(IDC_HIDE_COLLECTION_LINE_ITEMS, OnCheckHideAppliedLineItems)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTemplateEntryDlg message handlers

void CTemplateEntryDlg::OnAddLineItemBtn() 
{
	CTemplateLineItemInfo *pLineItem;
	pLineItem = new CTemplateLineItemInfo;
	//TES 6/19/2010 - PLID 39262 - Tell the line item whether or not to use Resource Availability templates
	pLineItem->m_bUseResourceAvailTemplates = m_bUseResourceAvailTemplates;

	if (m_dlgLineItem.ZoomLineItem(pLineItem) == IDOK)
	{ 
		// (z.manning 2011-12-07 17:42) - PLID 46906 - We used to refresh the entire line item list here but that was unnecessary
		// so let's just add the newly created row and be done with it.
		IRowSettingsPtr pNewRow = m_pLineItemList->GetNewRow();
		pNewRow->Value[ecliData] = (long)pLineItem;
		pNewRow->Value[ecliDescription] = _bstr_t(pLineItem->GetApparentDescription(m_bUseResourceAvailTemplates ? stetLocation : stetNormal));
		pNewRow->Value[ecliOriginalData] = (long)NULL;
		pNewRow = m_pLineItemList->AddRowAtEnd(pNewRow, NULL);

		// (c.haag 2006-11-15 12:39) - PLID 23423 - Warn the user if the line item will
		// not be visible because of the time constraints
		if (!ShouldLineItemBeVisible(pLineItem)) {
			MsgBox("The line item was created; but because line items that terminate before today are hidden, you will not be able to see it in the list.");
			pNewRow->PutVisible(VARIANT_FALSE);
		}
		else {
			pNewRow->PutVisible(VARIANT_TRUE);
			m_pLineItemList->EnsureRowInView(pNewRow);
		}
	}
	else {
		delete pLineItem;
	}
	ReflectCurrentStateOnBtns();
}

void CTemplateEntryDlg::OnEditLineItemBtn() 
{
	IRowSettingsPtr pRowSel = m_pLineItemList->CurSel;
	// Make sure something is selected
	if (NULL != pRowSel)
	{
		CTemplateLineItemInfo *pLineItem = (CTemplateLineItemInfo*)(long)pRowSel->Value[ecliData];
		CTemplateLineItemInfo *pOriginalLineItem = (CTemplateLineItemInfo*)VarLong(pRowSel->Value[ecliOriginalData]);
		if(pOriginalLineItem == NULL) {
			// (z.manning 2011-12-08 09:36) - PLID 46906 - Since they are editing this line item, keep track of the
			// original value of the line item so we can later determine which line items actually changed.
			pOriginalLineItem = new CTemplateLineItemInfo(pLineItem);
			pRowSel->Value[ecliOriginalData] = (long)pOriginalLineItem;
		}

		//TES 6/19/2010 - PLID 39262 - Tell the line item whether or not to use resource availability templates
		if (m_dlgLineItem.ZoomLineItem(pLineItem) == IDOK)
		{
			// (c.haag 2006-11-15 11:34) - PLID 23423 - Refresh the line item list
			// (z.manning 2011-12-07 17:35) - PLID 46906 - No need to refresh the whole list, just update this row.
			pRowSel->Value[ecliDescription] = _bstr_t(pLineItem->GetApparentDescription(m_bUseResourceAvailTemplates ? stetLocation : stetNormal));

			// (c.haag 2006-11-15 12:39) - PLID 23423 - Warn the user if the line item will
			// not be visible because of the time constraints
			if (ShouldLineItemBeVisible(pLineItem)) {
				pRowSel->PutVisible(VARIANT_TRUE);
			}
			else {
				MsgBox("The line item was modified; but because line items that terminate before today are hidden, you will not be able to see it in the list.");
				pRowSel->PutVisible(VARIANT_FALSE);
			}
		}
	}

	ReflectCurrentStateOnBtns();
}

void CTemplateEntryDlg::OnRemoveLineItemBtn() 
{
	// (z.manning 2008-06-12 11:50) - PLID 29273 - Do not allow any template related
	// deleting without this permission.
	//TES 2/20/2015 - PLID 64336 - This was just checking bioSchedTemplating, regardless of m_bUseResourceAvailTemplates
	if (!CheckCurrentUserPermissions(m_bUseResourceAvailTemplates ? bioResourceAvailTemplating : bioSchedTemplating, sptDelete)) {
		return;
	}

	IRowSettingsPtr pRowSel = m_pLineItemList->CurSel;
	// Make sure something is selected
	if (NULL != pRowSel)
	{
		if (AfxMessageBox("Are you sure you want to remove the selected line item?", 
							MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			CTemplateLineItemInfo *pLineItem;
			pLineItem = (CTemplateLineItemInfo *)(long)pRowSel->Value[ecliData];
			CTemplateLineItemInfo *pOriginalLineItem = (CTemplateLineItemInfo*)(long)pRowSel->Value[ecliOriginalData];
			m_pLineItemList->RemoveRow(pRowSel);
			// (z.manning 2011-12-08 10:06) - PLID 46906 - Keep track of any deleted line items
			if(pLineItem->m_nID != -1) {
				m_arynDeletedLineItemIDs.Add(pLineItem->m_nID);
			}
			if (pLineItem) {
				delete pLineItem;
			}

			// (z.manning 2011-12-07 17:15) - PLID 46906 - Also clean up original data
			if(pOriginalLineItem != NULL) {
				delete pOriginalLineItem;
			}
		}
	}
	ReflectCurrentStateOnBtns();
}

BOOL CTemplateEntryDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	// (c.haag 2006-11-02 13:17) - PLID 23329 - There are no longer template-specific date ranges
	//m_to.SetValue(COleVariant(COleDateTime::GetCurrentTime()));
	//m_from.SetValue(COleVariant(COleDateTime::GetCurrentTime()));

	try
	{
		// (z.manning 2014-12-23 10:41) - PLID 64296 - Bulk cache
		g_propManager.CachePropertiesInBulk("CTemplateEntryDlg", propNumber,
			"(Username IN ('<None>', '%s')) AND Name IN ( \r\n"
			"	'HideOldTemplateLineItemsInConfigDlg', \r\n"
			"	'HideOldResourceAvailTemplateLineItemsInConfigDlg', \r\n"
			"	'HideAppliedTemplateLineItemsInConfigDlg' \r\n"
			"	) \r\n",
			_Q(GetCurrentUserName()));

		// (z.manning, 04/29/2008) - PLID 29814 - Set button styles
		m_btnAddLineItem.AutoSet(NXB_NEW);
		m_btnEditLineItem.AutoSet(NXB_MODIFY);
		m_btnRemoveLineItem.AutoSet(NXB_DELETE);
		m_btnAddRule.AutoSet(NXB_NEW);
		m_btnEditRule.AutoSet(NXB_MODIFY);
		m_btnRemoveRule.AutoSet(NXB_DELETE);
		m_btnAddException.AutoSet(NXB_NEW);
		m_btnEditException.AutoSet(NXB_MODIFY);
		m_btnRemoveException.AutoSet(NXB_DELETE);
		m_btnPreviewReport.AutoSet(NXB_PRINT_PREV);
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.jones 2007-04-20 10:06) - PLID 25689 - limit the template name
		((CNxEdit*)GetDlgItem(IDC_TEMPLATE_NAME_EDIT))->SetLimitText(50);

		//Find which resources
		// (c.haag 2006-11-02 13:26) - PLID 23329 - There are no longer template-specific resources.
		// They are now specific to template line items.
		/*CString strFrom;
		m_pResourceList = BindNxDataListCtrl(this, IDC_LIST_RESOURCE, GetRemoteData(), false);
		strFrom.Format("(SELECT ID AS ResourceID,  CONVERT(BIT, CASE WHEN ID IN (SELECT ResourceID FROM TemplateConnectT WHERE TemplateID = %li) THEN 1 ELSE 0 END) AS Selected, Item AS ResourceName FROM ResourceT) subQ", m_nTemplateID);
		m_pResourceList->FromClause = _bstr_t(strFrom);
		m_pResourceList->Requery();
		m_pResourceList->WaitForRequery(dlPatienceLevelWaitIndefinitely);*/

		// (c.haag 2007-02-21 10:42) - PLID 23784 - We now use datalists for all the boxes
		m_pLineItemList = BindNxDataList2Ctrl(this, IDC_LIST_LINE_ITEMS, NULL, false);
		m_pRuleList = BindNxDataList2Ctrl(this, IDC_LIST_RULES, NULL, false);
		m_pExceptionList = BindNxDataList2Ctrl(this, IDC_LIST_EXCEPTIONS, NULL, false);
		//TES 6/19/2010 - PLID 39262 - Added a location combo for Resource Availability templates
		m_pLocationCombo = BindNxDataList2Ctrl(IDC_TEMPLATE_LOCATION);

		//TES 6/19/2010 - PLID 39262 - If we're editing a Resource Availability template, hide the "Rules" section.
		if(m_bUseResourceAvailTemplates) {
			GetDlgItem(IDC_TEMPLATE_RULES_CAPTION)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LIST_RULES)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ADD_RULE_BTN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EDIT_RULE_BTN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_REMOVE_RULE_BTN)->ShowWindow(SW_HIDE);
			//TES 6/19/2010 - PLID 39262 - Also hide the Preview Report button, we don't yet have a report
			// for Resource Availability templates
			GetDlgItem(IDC_BTN_PREVIEW_REPORT)->ShowWindow(SW_HIDE);

			// (j.jones 2010-07-19 12:15) - PLID 39625 - default the color to dark gray
			m_nColor = 8421504;
		}
		else {
			//TES 6/19/2010 - PLID 39262 - Otherwise, hide the Location dropdown
			GetDlgItem(IDC_TEMPLATE_LOCATION_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_TEMPLATE_LOCATION)->ShowWindow(SW_HIDE);

			// (j.jones 2010-07-19 12:15) - PLID 39625 - default the color to red
			m_nColor = 255;
		}

		//TES 6/21/2010 - PLID 39262 - Update the window title to indicate whether these are Resource Availability templates
		if(m_bUseResourceAvailTemplates) {
			//(e.lally 2010-07-15) PLID 39626 - Renamed to Location Template
			SetWindowText("Location Template Entry");
		}

	}NxCatchAll("Error in CTemplateEntryDlg::OnInitDialog() ");

	// (c.haag 2006-11-15 11:17) - PLID 23423 - Update the checkbox that dictates whether we hide old line items
	//TES 6/19/2010 - PLID 39262 - Store the checkboxes separately for Resource Availability templates
	CString strProperty = m_bUseResourceAvailTemplates?"HideOldResourceAvailTemplateLineItemsInConfigDlg":"HideOldTemplateLineItemsInConfigDlg";
	SetDlgItemCheck(IDC_CHECK_HIDE_OLD_LINEITEMS, GetRemotePropertyInt(strProperty, 0, 0, GetCurrentUserName()));

	// (z.manning 2014-12-23 10:46) - PLID 64296
	SetDlgItemCheck(IDC_HIDE_COLLECTION_LINE_ITEMS, (GetRemotePropertyInt("HideAppliedTemplateLineItemsInConfigDlg", 0, 0, GetCurrentUserName()) != 0 ? BST_CHECKED : BST_UNCHECKED));

	// (c.haag 2006-11-02 13:26) - PLID 23329 - There are no longer template-specific resources.
	/*
	m_nResourceRadio = 0; // Default resource radio
	
	if(m_nTemplateID == -1) {
		OnAllResourcesRadio();
	}

	else*/ {
		
		LoadCurrentTemplate(m_nTemplateID);
		// Select the first item
		if (m_pLineItemList->GetRowCount() > 0) {
			m_pLineItemList->CurSel = m_pLineItemList->GetFirstRow();
		}
		//TES 6/19/2010 - PLID 39262 - Resource Availability templates don't have rules.
		if (!m_bUseResourceAvailTemplates && m_pRuleList->GetRowCount() > 0) {
			m_pRuleList->CurSel = m_pRuleList->GetFirstRow();
		}
		if (m_pExceptionList->GetRowCount() > 0) {
			m_pExceptionList->CurSel = m_pExceptionList->GetFirstRow();
		}
	}

	// Enable/disable buttons appropriately
	ReflectCurrentStateOnBtns();

	if(!m_bAllowEdit) {
		GetDlgItem(IDC_TEMPLATE_NAME_EDIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADD_LINE_ITEM_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_LINE_ITEM_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_REMOVE_LINE_ITEM_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHOOSE_COLOR_BTN)->EnableWindow(FALSE);
		// (c.haag 2006-11-02 13:22) - PLID 23329 - There are no longer template-specific resources
		//GetDlgItem(IDC_SELECT_RESOURCES_RADIO)->EnableWindow(FALSE);
		//GetDlgItem(IDC_ALL_RESOURCES_RADIO)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADD_RULE_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_RULE_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_REMOVE_RULE_BTN)->EnableWindow(FALSE);
		//m_pResourceList->Enabled = FALSE;
		// (c.haag 2006-11-02 13:17) - PLID 23329 - There are no longer template-specific date ranges
		//m_from.EnableWindow(FALSE);
		//m_to.EnableWindow(FALSE);
	}
	
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTemplateEntryDlg::OnOK() 
{
	try {

		if(!Save()) {
			return;
		}

		CNxDialog::OnOK();

	} NxCatchAll("CTemplateEntryDlg::OnOK");
}

//TES 6/19/2010 - PLID 39262 - Pass in the Location ID (for Resource Availability templates)
void CTemplateEntryDlg::SaveCurrentTemplate(long nLocationID)
{
	//(e.lally 2008-06-04) PLID 27575 - Put the audit transaction usage inside a try catch so we can roll it back if an exception
	//is thrown before it is committed.
	// (z.manning 2011-12-07 15:12) - PLID 46906 - Changed this to an audit transaction
	CAuditTransaction auditTran;
	{
		CPtrArray aryLineItems;
		CArray<CTemplateRuleInfo*,CTemplateRuleInfo*> aryRules;
		CPtrArray aryExceptions;

		//TES 6/19/2010 - PLID 39262 - Figure out which tables we're saving to.
		CString strTablePrefix = m_bUseResourceAvailTemplates ? "ResourceAvail" : "";

		CString strSaveSql = BeginSqlBatch();

		strSaveSql += "DECLARE @nTemplateRuleID INT \r\n";
		strSaveSql += "DECLARE @nTemplateRuleDetailID INT \r\n";
		strSaveSql += "DECLARE @nSecurityObjectID INT \r\n";
		if(m_nTemplateID == -1) {
			strSaveSql += "DECLARE @nNewPriorityID INT \r\n";
			strSaveSql += "SET @nNewPriorityID = (SELECT Coalesce(Max(Priority),0) + 1 FROM " + strTablePrefix + "TemplateT) \r\n";
		}
		
		// Put all the list box line items into an array
		{
			for(IRowSettingsPtr pLineItemRow = m_pLineItemList->FindAbsoluteFirstRow(VARIANT_FALSE); pLineItemRow != NULL; pLineItemRow = m_pLineItemList->FindAbsoluteNextRow(pLineItemRow, VARIANT_FALSE))
			{
				CTemplateLineItemInfo *pLineItem = (CTemplateLineItemInfo*)VarLong(pLineItemRow->Value[ecliData]);
				CTemplateLineItemInfo *pOriginalLineItem = (CTemplateLineItemInfo*)VarLong(pLineItemRow->Value[ecliOriginalData]);
				// (z.manning 2011-12-08 09:49) - PLID 46906 - We used to save every line item no matter what. I optimized this
				// to now only save new or changed line items.
				if(pLineItem->IsNew() || (pOriginalLineItem != NULL && *pLineItem != *pOriginalLineItem)) {
					aryLineItems.Add(pLineItem);
				}
			}
		}

		{
			// (c.haag 2007-02-21 11:15) - PLID 23784 - We now pull from a datalist 2 control
			//TES 6/19/2010 - PLID 39262 - Resource Availability templates don't have rules.
			if(!m_bUseResourceAvailTemplates)
			{
				IRowSettingsPtr pRow = m_pRuleList->GetFirstRow();
				while (NULL != pRow) {
					CTemplateRuleInfo *pRule = (CTemplateRuleInfo*)VarLong(pRow->Value[ecrData]);
					CTemplateRuleInfo *pOriginalRule = (CTemplateRuleInfo*)VarLong(pRow->Value[ecrOriginalData]);
					// (z.manning 2011-12-09 09:45) - PLID 46906 - Only save new or changed rules
					if(pRule->m_nID == -1 || (pOriginalRule != NULL && *pRule != *pOriginalRule)) {
						aryRules.Add(pRule);
					}
					pRow = pRow->GetNextRow();
				}
			}
		}

		{
			// (c.haag 2007-02-21 11:29) - PLID 23784 - We now pull from a datalist 2 control
			IRowSettingsPtr pRow = m_pExceptionList->GetFirstRow();
			while (NULL != pRow)
			{
				CTemplateExceptionInfo *pException = (CTemplateExceptionInfo*)VarLong(pRow->Value[eceData]);
				CTemplateExceptionInfo *pOriginalException = (CTemplateExceptionInfo*)VarLong(pRow->Value[eceOriginalData]);
				// (z.manning 2011-12-09 09:01) - PLID 46906 - Only save new or changed exceptions
				if(pException->m_nID == -1 || (pOriginalException != NULL && *pException != *pOriginalException)) {
					aryExceptions.Add(pException);
				}
				pRow = pRow->GetNextRow();
			}
		}
		
		// Get the sql that writes to the template record
		// (j.jones 2007-04-20 10:07) - PLID 25689 - do not pass in the member template ID,
		// because we don't want to update it until the execute succeeds
		//TES 6/19/2010 - PLID 39262 - Pass in the Location ID
		long nTemplateID = m_nTemplateID;
		PrepareWriteTemplate(strSaveSql, &auditTran, nTemplateID, m_strTemplateName,
			nLocationID, m_nColor, &aryLineItems, &aryRules, &aryExceptions);
		

		//now save the sql
	#ifdef _DEBUG
		//(e.lally 2008-04-10)- Switched to our CMsgBox dialog
		CMsgBox dlg(this);
		dlg.msg = strSaveSql;
		dlg.DoModal();
	#endif
		{
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(10000);
			ExecuteSqlBatch(strSaveSql);
		}

		// (j.jones 2007-04-20 10:08) - PLID 25689 - now that the batch has executed,
		// update our member template ID
		m_nTemplateID = nTemplateID;

		auditTran.Commit();

		/*
		//now add to the global array
		//TODO: This only adds to the current user's global array. Ponder this.
		char szDynamicNames[64];
		memset(szDynamicNames, 0, 64);
		strcpy(szDynamicNames, "Override save prevention");
		extern CArray<CBuiltInObject*,CBuiltInObject*> gc_aryUserDefinedObjects;
		CBuiltInObject *pObj = new CBuiltInObject((EBuiltInObjectIDs)nSecurityObjectID, nAllPerms, strTemplateRuleName, "Controls access to override this template rule when scheduling appointments", szDynamicNames, bioSchedTemplateRules, nNewRuleID);
		gc_aryUserDefinedObjects.Add(pObj);
		*/

		// (c.haag 2006-05-23 09:13) - PLID 20609 - Pondering is complete. We need to send a table
		// checker and update our global array. We don't know what ID's in SecurityObjectT to update,
		// so lets just do all the scheduler template records
		//TES 6/19/2010 - PLID 39262 - Resource Availability templates don't have rules.
		if(!m_bUseResourceAvailTemplates) {
			CString strWhere;
			strWhere.Format("BuiltInID = %d", bioSchedTemplateRules);
			LoadUserDefinedPermissions(strWhere);
			CClient::RefreshTable(NetUtils::UserDefinedSecurityObject, -1);
		}


		//re-login to update your permissions
		//TODO: There is code elsewhere to handle when another user attempts to access this new permission,
		//however we should consider a way to automatically have them re-log in. This should not happen though until
		//we find a way to auto-update their global array first.
		LogInUser(GetCurrentUserName(), GetCurrentUserPassword(), GetCurrentLocationID());
		GetMainFrame()->UpdateToolBarButtons(TRUE);
	}
}

//TES 6/19/2010 - PLID 39262 - Added a Location ID parameter, for Resource Availability templates
void CTemplateEntryDlg::PrepareWriteTemplate(CString &strSql, CAuditTransaction *pAuditTran, long &nTemplateID, CString strTemplateName,
											 long nLocationID, OLE_COLOR nColor, CPtrArray * pLineItems, CArray<CTemplateRuleInfo*,CTemplateRuleInfo*> *paryRules, CPtrArray* pExceptions)
{
	//TES 6/19/2010 - PLID 39262 - Figure out which tables we're saving to.
	CString strTablePrefix = m_bUseResourceAvailTemplates ? "ResourceAvail" : "";

	int nNewTemplateID = nTemplateID;
	// Create the record if there isn't one or update the existing one
	if (nNewTemplateID <= 0) {
		nNewTemplateID = NewNumber(strTablePrefix+"TemplateT", "ID");
		// (j.jones 2006-09-21 08:43) - PLID 22616 - the Priority is created in the initialization of the save batch
		// (c.haag 2006-11-02 13:24) - PLID 23329 - Date ranges are now handled by template line items
		//TES 6/19/2010 - PLID 39262 - Resource Availability templates have a locationID
		AddStatementToSqlBatch(strSql, "INSERT INTO %sTemplateT (ID, Name, Color, Priority%s) "
			"VALUES (%li, '%s', %li, @nNewPriorityID%s)", strTablePrefix, 
			m_bUseResourceAvailTemplates ? ", LocationID" : "",nNewTemplateID, _Q(strTemplateName),
			nColor, m_bUseResourceAvailTemplates ? ","+AsString(nLocationID) : "");

		//audit the creation
		AuditEvent(-1, strTemplateName, *pAuditTran, m_bUseResourceAvailTemplates ? aeiResourceAvailTemplateCreate : aeiTemplateCreate, nNewTemplateID, "", "Created", aepHigh, aetCreated);
		
	}
	else {
		// (c.haag 2006-11-02 13:24) - PLID 23329 - Date ranges are now handled by template line items
		AddStatementToSqlBatch(strSql, "UPDATE %sTemplateT SET Name = '%s', Color = %li WHERE ID = %li", strTablePrefix, _Q(strTemplateName), nColor, nNewTemplateID);
		//TES 6/19/2010 - PLID 32692 - Resource Availability templates have a location ID
		if(m_bUseResourceAvailTemplates) {
			AddStatementToSqlBatch(strSql, "UPDATE ResourceAvailTemplateT SET LocationID = %li WHERE ID = %li",
				nLocationID, nNewTemplateID);
		}
	}

	// (c.haag 2006-11-13 10:33) - PLID 5336 - Delete exceptions
	// (z.manning 2011-12-08 10:44) - PLID 46906 - Only delete exceptions that were deleted
	if(m_arynDeletedExceptionIDs.GetCount() > 0)
	{
		AddStatementToSqlBatch(strSql, "DELETE FROM %sTemplateExceptionT WHERE ID IN (%s)", strTablePrefix, ArrayAsString(m_arynDeletedExceptionIDs));
	}

	// (c.haag 2006-11-29 10:01) - PLID 23665 - Delete the old line items
	// (z.manning 2011-12-08 10:07) - PLID 46906 - No more, now only delete line items that were actually deleted.
	if(m_arynDeletedLineItemIDs.GetCount() > 0)
	{
		AddStatementToSqlBatch(strSql, "DELETE FROM %sTemplateItemExceptionT WHERE TemplateItemID IN (%s)", strTablePrefix, ArrayAsString(m_arynDeletedLineItemIDs));
		AddStatementToSqlBatch(strSql, "DELETE FROM %sTemplateDetailsT WHERE TemplateItemID IN (%s)", strTablePrefix, ArrayAsString(m_arynDeletedLineItemIDs));
		AddStatementToSqlBatch(strSql, "DELETE FROM %sTemplateItemResourceT WHERE TemplateItemID IN (%s)", strTablePrefix, ArrayAsString(m_arynDeletedLineItemIDs));
		AddStatementToSqlBatch(strSql, "DELETE FROM %sTemplateItemT WHERE ID IN (%s)", strTablePrefix, ArrayAsString(m_arynDeletedLineItemIDs));
	}

	// (z.manning 2011-12-09 10:24) - PLID 46906 - Since rules are more complex, let's go ahead and delete any rules
	// that we're saving (which should only be new or changed rules anyway).
	CArray<long,long> arynRulesIDsToDelete;
	for(int nRuleIndex = 0; nRuleIndex < paryRules->GetCount(); nRuleIndex++) {
		CTemplateRuleInfo *pRule = paryRules->GetAt(nRuleIndex);
		if(pRule->m_nID != -1) {
			arynRulesIDsToDelete.Add(pRule->m_nID);
		}
	}

	arynRulesIDsToDelete.Append(m_arynDeletedRuleIDs);

	//TES 6/19/2010 - PLID 39262 - Resource Availability templates don't have rules.
	// (z.manning 2011-12-09 09:45) - PLID 46906 - We no longer delete every rule for the template we're saving
	if(arynRulesIDsToDelete.GetCount() > 0 && !m_bUseResourceAvailTemplates)
	{
		AddStatementToSqlBatch(strSql, "DELETE FROM TemplateRuleDetailsT WHERE TemplateRuleID IN (%s)", ArrayAsString(arynRulesIDsToDelete));	
		AddStatementToSqlBatch(strSql, "DELETE FROM TemplateRuleT WHERE ID IN (%s)", ArrayAsString(arynRulesIDsToDelete));

		DeleteRulePermissions(&arynRulesIDsToDelete);
	}

	{
		// Add all the new line items
		int nLineItemsSize = pLineItems->GetSize();
		for (int i=0; i<nLineItemsSize; i++) {
			CTemplateLineItemInfo* pLineItem = (CTemplateLineItemInfo*)(pLineItems->GetAt(i));			
			pLineItem->m_nTemplateID = nNewTemplateID;
			pLineItem->AddSaveStringToBatch(strSql);
		}
	}

	{
		//TES 6/19/2010 - PLID 39262 - Resource Availability templates don't have rules.
		if(!m_bUseResourceAvailTemplates) {
			// Add all the new rules
			int nRulesSize = paryRules->GetSize();
			for (int i=0; i<nRulesSize; i++) {
				PrepareAddTemplateRule(strSql, nNewTemplateID, paryRules->GetAt(i));
			}
		}
	}

	{
		// (c.haag 2006-11-13 10:30) - PLID 5993 - Add all the new exceptions
		int nExceptionsSize = pExceptions->GetSize();
		for (int i=0; i < nExceptionsSize; i++) {
			PrepareAddException(strSql, nNewTemplateID, (CTemplateExceptionInfo*)pExceptions->GetAt(i));
		}
	}
	
	//Set the output parameter
	nTemplateID = nNewTemplateID;
}

void CTemplateEntryDlg::PrepareAddTemplateRule(CString &strSql, long nTemplateID, CTemplateRuleInfo *pRule)
{
	//TES 6/19/2010 - PLID 39262 - Resource Availability templates don't have rules
	if(m_bUseResourceAvailTemplates) {
		ASSERT(FALSE);
		return;
	}
	// Get an appropriate new ID for the line item
	CString str;
	str.Format("SET @nTemplateRuleID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM TemplateRuleT)");
	AddStatementToSqlBatch(strSql, str);

	// Insert a record in the line item table with the appropriate info
	//TES 8/31/2010 - PLID 39630 - Added OverrideLocationTemplating
	//TES 9/13/2010 - PLID 39630 - Only allow the Override flag to be set if this is an "of any kind" type rule.
	AddStatementToSqlBatch(strSql, "INSERT INTO TemplateRuleT (ID, TemplateID, Description, WarningOnFail, PreventOnFail, AndDetails, AllAppts, OverrideLocationTemplating) "
		"VALUES (@nTemplateRuleID, %li, '%s', %s, %li, %li, %li, %li)",
		nTemplateID, _Q(pRule->m_strDescription), pRule->m_bWarningOnFail ? "'" + _Q(pRule->m_strWarningOnFail) + "'" : "NULL", pRule->m_bPreventOnFail?1:0, pRule->m_bAndDetails?1:0, pRule->m_bAllAppts?1:0, pRule->m_bAllAppts?(pRule->m_bOverrideLocationTemplating?1:0):0);

	if (pRule->m_nTypeListObjectType != 0) {
		long nSize = pRule->m_aryTypeList.GetSize();
		for (long i=0; i<nSize; i++) {
			// Insert each TYPE detail

			CString str;
			str.Format("SET @nTemplateRuleDetailID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM TemplateRuleDetailsT)");
			AddStatementToSqlBatch(strSql, str);

			AddStatementToSqlBatch(strSql, "INSERT INTO TemplateRuleDetailsT (ID, TemplateRuleID, ObjectType, ObjectID) "
				"VALUES (@nTemplateRuleDetailID, @nTemplateRuleID, %li, %li)", 
				pRule->m_nTypeListObjectType, pRule->m_aryTypeList[i]);
		}
	}

	if (pRule->m_nPurposeListObjectType != 0) {
		long nSize = pRule->m_aryPurposeList.GetSize();
		for (long i=0; i<nSize; i++) {
			// Insert each TYPE detail

			CString str;
			str.Format("SET @nTemplateRuleDetailID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM TemplateRuleDetailsT)");
			AddStatementToSqlBatch(strSql, str);

			AddStatementToSqlBatch(strSql, "INSERT INTO TemplateRuleDetailsT (ID, TemplateRuleID, ObjectType, ObjectID) "
				"VALUES (@nTemplateRuleDetailID, @nTemplateRuleID, %li, %li)", 
				pRule->m_nPurposeListObjectType, pRule->m_aryPurposeList[i]);
		}
	}

	if (pRule->m_nBookingCount >= 0) {

		CString str;
		str.Format("SET @nTemplateRuleDetailID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM TemplateRuleDetailsT)");
		AddStatementToSqlBatch(strSql, str);

		AddStatementToSqlBatch(strSql, "INSERT INTO TemplateRuleDetailsT (ID, TemplateRuleID, ObjectType, ObjectID) "
			"VALUES (@nTemplateRuleDetailID, @nTemplateRuleID, 3, %li)", 
			pRule->m_nBookingCount);
	}

	// (c.haag 2003-08-01 12:50) - Remake the permissions
	long nAllPerms = sptDynamic0|sptDynamic0WithPass;
	CString strTemplateRuleName;
	CString strTemplateName;
	_RecordsetPtr prsTemplateName = CreateParamRecordset("SELECT Name FROM TemplateT WHERE ID = {INT}", nTemplateID);
	if (!prsTemplateName->eof)
		strTemplateName = AdoFldString(prsTemplateName, "Name","");
	prsTemplateName->Close();
	strTemplateRuleName = strTemplateName + " - " + pRule->m_strDescription;

	str.Format("SET @nSecurityObjectID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM SecurityObjectT)");
	AddStatementToSqlBatch(strSql, str);

	AddStatementToSqlBatch(strSql, "INSERT INTO SecurityObjectT (ID, BuiltInID, ObjectValue, DisplayName, Description, AvailablePermissions, DynamicName0) "
		"VALUES (@nSecurityObjectID, %li, @nTemplateRuleID, '%s', 'Controls access to override this template rule when scheduling appointments.', %li, 'Override save prevention')",
		bioSchedTemplateRules, _Q(strTemplateRuleName), nAllPerms);
	AddStatementToSqlBatch(strSql, "INSERT INTO PermissionT (UserGroupID, ObjectID, Permissions) SELECT PersonID,@nSecurityObjectID,0 FROM UsersT UNION SELECT PersonID,@nSecurityObjectID,0 FROM UserGroupsT");

	long nSize = pRule->m_adwUsers.GetSize();
	for (long i=0; i < nSize; i++)
	{
		AddStatementToSqlBatch(strSql, "UPDATE PermissionT SET Permissions = %li WHERE UserGroupID = %li AND ObjectID = @nSecurityObjectID",
			pRule->m_adwPerms[i], pRule->m_adwUsers[i]);
	}
}

void CTemplateEntryDlg::PrepareAddException(CString& strSql, long nTemplateID, CTemplateExceptionInfo* pException)
{
	//TES 6/19/2010 - PLID 39262 - Figure out which tables we're saving to.
	CString strTablePrefix = m_bUseResourceAvailTemplates?"ResourceAvail":"";
	//
	// (c.haag 2006-11-13 10:32) - PLID 5993 - Prepare to add the exception to data
	//
	if(pException->m_nID == -1)
	{
		CString str;
		str.Format("INSERT INTO %sTemplateExceptionT (Description, TemplateID, StartDate, EndDate, Flags) "
			"VALUES ('%s', %d, '%s', '%s', %d)",
			strTablePrefix,
			_Q(pException->m_strDescription),
			nTemplateID,
			FormatDateTimeForSql(pException->m_dtStartDate, dtoDate),
			FormatDateTimeForSql(pException->m_dtEndDate, dtoDate),
			pException->m_nFlags);
		AddStatementToSqlBatch(strSql, str);
	}
	else
	{
		// (z.manning 2011-12-09 08:59) - PLID 46906 - Also support saving existing exceptions
		AddStatementToSqlBatch(strSql,
			"UPDATE %sTemplateExceptionT SET \r\n"
			"	Description = '%s' \r\n"
			"	, TemplateID = %li \r\n"
			"	, StartDate = '%s' \r\n"
			"	, EndDate = '%s' \r\n"
			"	, Flags = %li \r\n"
			"WHERE ID = %li "
			, strTablePrefix, _Q(pException->m_strDescription), nTemplateID, FormatDateTimeForSql(pException->m_dtStartDate, dtoDate)
			, FormatDateTimeForSql(pException->m_dtEndDate, dtoDate), pException->m_nFlags
			, pException->m_nID);
	}
}

void CTemplateEntryDlg::LoadCurrentTemplate(long nTemplateID)
{
	// (c.haag 2006-11-02 13:17) - PLID 23329 - There are no longer template-specific date ranges
	//COleDateTime dtStartDate, dtEndDate;
	m_nTemplateID = nTemplateID;

	//TES 6/19/2010 - PLID 39262 - Resource Availability templates have a location ID.
	long nLocationID = -1;

	// Read the data from the database for this template
	ReadTemplate(m_nTemplateID, m_strTemplateName, m_nColor, nLocationID);

	//TES 6/19/2010 - PLID 39262 - If we're on a resource availability template, set the location.
	long nSel = m_pLocationCombo->TrySetSelByColumn_Deprecated(0, nLocationID);
	if(nSel == sriNoRow) {
		//maybe it's inactive?
		_RecordsetPtr rsLoc = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", nLocationID);
		if(!rsLoc->eof) {
			m_nPendingLocationID = nLocationID;
			m_pLocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLoc, "Name", "")));
		}
		else 
			m_pLocationCombo->PutCurSel(NULL);
	}
	else if(nSel == sriNoRowYet_WillFireEvent) {
		m_nPendingLocationID = nLocationID;
	}

	UpdateData(FALSE);
}

// (c.haag 2006-11-02 13:26) - PLID 23329 - There are no longer template-specific resources.
// They are now specific to template line items. TemplateConnectT has been rendered obselete.
/*void CTemplateEntryDlg::ReadTemplateConnect(long nTemplateID, CDWordArray &aryResourceIDs)
{
	try {
		aryResourceIDs.RemoveAll();
		// Open the template connect table
		_RecordsetPtr prs = CreateRecordset("SELECT DISTINCT ResourceID FROM TemplateConnectT WHERE TemplateID = %li", nTemplateID);
		FieldPtr fldID = prs->Fields->Item["ResourceID"];
		// For each appropriate connect line
		while (!prs->eof) {
			// Add the resource id to the array
			aryResourceIDs.Add(AdoFldLong(fldID));
			HR(prs->MoveNext());
		}
	} NxCatchAll("CTemplateEntryDlg::ReadTemplateConnect");
}*/

//TES 6/19/2010 - PLID 39262 - Added a Location ID parameter, for Resource Availability templates
void CTemplateEntryDlg::ReadTemplate(long nTemplateID, CString &strTemplateName, OLE_COLOR &nColor, long &nLocationID)
{
	//TES 6/19/2010 - PLID 39262 - Figure out which tables we're reading from.
	CString strTablePrefix = m_bUseResourceAvailTemplates?"ResourceAvail":"";

	// First read the template
	if (nTemplateID > 0)
	{
		// Open the template with the given ID
		_RecordsetPtr prs = CreateParamRecordset(FormatString(
			"SELECT * FROM %sTemplateT WHERE ID = {INT}", strTablePrefix), nTemplateID);
		if (!prs->eof) {
			// Found so proceed
			FieldsPtr flds = prs->Fields;
			strTemplateName = AdoFldString(flds, "Name");
			//dtStartDate = AdoFldDateTime(flds, "StartDate", dtNull);
			//dtEndDate = AdoFldDateTime(flds, "EndDate", dtNull);
			nColor = AdoFldLong(flds, "Color");
			//TES 6/19/2010 - PLID 39262 - Resource Availability templates also have a location.
			if(m_bUseResourceAvailTemplates) {
				nLocationID = AdoFldLong(flds, "LocationID");
			}

		} else {
			return;
		}
	}

	// (z.manning 2011-12-07 12:18) - PLID 46910 - Need a resource map to load template items
	CMap<long,long,CString,LPCTSTR> mapResources;
	FillResourceMap(&mapResources);

	// Add all the new line items to the array
	try {
		// (c.haag 2006-11-02 14:56) - PLID 23241 - Per PLID 23276, the ParentID has been rendered obselete
		// in favor of a superior data structure.
		// (z.manning 2011-12-07 11:08) - PLID 46910 - Also load template item exceptions in this query
		// (c.haag 2014-12-16) - PLID 64256 - Added CollectionID
		_RecordsetPtr prs = CreateParamRecordset(FormatString(
			"SELECT %sTemplateItemT.*, dbo.Get%sTemplateItemResourceIDString(%sTemplateItemT.ID) AS ResourceIDString \r\n"
			"	, Date AS ExceptionDate \r\n"
			"%s"
			"FROM %sTemplateItemT \r\n"
			"%s"
			"LEFT JOIN %sTemplateItemExceptionT ON %sTemplateItemT.ID = %sTemplateItemExceptionT.TemplateItemID \r\n"
			"WHERE TemplateID = {INT} \r\n"
			"ORDER BY %sTemplateItemT.ID \r\n"
			, strTablePrefix, strTablePrefix, strTablePrefix
			, m_bUseResourceAvailTemplates ? ", NULL AS CollectionID \r\n" : ", CollectionID \r\n"
			, strTablePrefix
			, m_bUseResourceAvailTemplates ? "" : "LEFT JOIN TemplateCollectionApplyT ON TemplateCollectionApplyT.ID = TemplateItemT.TemplateCollectionApplyID \r\n"
			, strTablePrefix, strTablePrefix, strTablePrefix
			, strTablePrefix)
			, m_nTemplateID);
		FieldsPtr flds = prs->Fields;
		m_pLineItemList->SetRedraw(VARIANT_FALSE);
		for (int nTemplateItemIndex = 0; !prs->eof; prs->MoveNext(), nTemplateItemIndex++)
		{
			CTemplateLineItemInfo *pNewInfo = new CTemplateLineItemInfo;
			pNewInfo->m_bUseResourceAvailTemplates = m_bUseResourceAvailTemplates;
			pNewInfo->LoadFromRecordset(prs, TRUE, &mapResources);			

			IRowSettingsPtr pRow = m_pLineItemList->GetNewRow();
			pRow->Value[ecliData] = (long)pNewInfo;
			pRow->Value[ecliDescription] = (LPCTSTR)pNewInfo->GetApparentDescription(m_bUseResourceAvailTemplates ? stetLocation : stetNormal);
			pRow->Value[ecliOriginalData] = (long)NULL;
			pRow = m_pLineItemList->AddRowAtEnd(pRow, NULL);
			if(!ShouldLineItemBeVisible(pNewInfo)) {
				pRow->PutVisible(VARIANT_FALSE);
			}
		}
		m_pLineItemList->SetRedraw(VARIANT_TRUE);
	}
	NxCatchAll("CTemplateEntryDlg::ReadTemplate 1");

	// Add all the rules items to the array
	try
	{
		//TES 6/19/2010 - PLID 39262 - Resource Availability templates don't have rules.
		if(!m_bUseResourceAvailTemplates)
		{
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT * FROM TemplateRuleT WHERE TemplateID = {INT}", m_nTemplateID);
			FieldsPtr flds = prs->Fields;
			while (!prs->eof)
			{
				CTemplateRuleInfo *pRule = NewTemplateRule(flds);
				// (c.haag 2007-02-21 11:18) - PLID 23784 - The list is now a datalist 2 control
				IRowSettingsPtr pRow = m_pRuleList->GetNewRow();
				pRow->Value[ecrData] = (long)pRule;
				pRow->Value[ecrDescription] = (LPCTSTR)pRule->m_strDescription;
				pRow->Value[ecrOriginalData] = (long)NULL;
				m_pRuleList->AddRowAtEnd(pRow, NULL);
				HR(prs->MoveNext());
			}
		}
	} NxCatchAll("CTemplateEntryDlg::ReadTemplate 2");

	// (c.haag 2006-11-13 09:12) - PLID 5993 (Yes, 5993) - Add template exceptions to the array
	try
	{
		_RecordsetPtr prs = CreateParamRecordset(FormatString(
			"SELECT * FROM %sTemplateExceptionT WHERE TemplateID = {INT}", strTablePrefix), m_nTemplateID);
		FieldsPtr flds = prs->Fields;
		while (!prs->eof)
		{
			CTemplateExceptionInfo *pException = NewException(flds);
			// (c.haag 2007-02-21 11:30) - PLID 23784 - The list is now a datalist 2 control
			IRowSettingsPtr pRow = m_pExceptionList->GetNewRow();
			pRow->Value[eceData] = (long)pException;
			pRow->Value[eceDescription] = (LPCTSTR)FormatExceptionText(pException);
			pRow->Value[eceOriginalData] = (long)NULL;
			m_pExceptionList->AddRowAtEnd(pRow, NULL);
			HR(prs->MoveNext());
		}
	} NxCatchAll("CTemplateEntryDlg::ReadTemplate 3");
}

CTemplateRuleInfo *CTemplateEntryDlg::NewTemplateRule(FieldsPtr &pfldsTemplateRuleT)
{
	//TES 6/19/2010 - PLID 39262 - Resource Availability templates don't have rules
	ASSERT(!m_bUseResourceAvailTemplates);

	CTemplateRuleInfo *pNewInfo = new CTemplateRuleInfo;

	pNewInfo->m_nID = AdoFldLong(pfldsTemplateRuleT, "ID");
	pNewInfo->m_strDescription = AdoFldString(pfldsTemplateRuleT, "Description");
	pNewInfo->m_bPreventOnFail = AdoFldBool(pfldsTemplateRuleT, "PreventOnFail");

	_variant_t varWarning = pfldsTemplateRuleT->Item["WarningOnFail"]->Value;
	if (varWarning.vt == VT_NULL) {
		pNewInfo->m_bWarningOnFail = FALSE;
		pNewInfo->m_strWarningOnFail = "";
	} else {
		pNewInfo->m_bWarningOnFail = TRUE;
		pNewInfo->m_strWarningOnFail = VarString(varWarning);
	}

	pNewInfo->m_bAndDetails = AdoFldBool(pfldsTemplateRuleT, "AndDetails");

	pNewInfo->m_bAllAppts = AdoFldBool(pfldsTemplateRuleT, "AllAppts");

	//TES 8/31/2010 - PLID 39630 - Added OverrideLocationTemplating
	pNewInfo->m_bOverrideLocationTemplating = AdoFldBool(pfldsTemplateRuleT, "OverrideLocationTemplating");

	// Fill the details
	pNewInfo->m_nTypeListObjectType = 0;
	pNewInfo->m_nPurposeListObjectType = 0;
	pNewInfo->m_aryTypeList.RemoveAll();
	pNewInfo->m_aryPurposeList.RemoveAll();

	// Loop through the stored details loading them into our arrays
	_RecordsetPtr prsDetails = CreateParamRecordset(
		"SELECT ID, ObjectType, ObjectID "
		"FROM TemplateRuleDetailsT "
		"WHERE TemplateRuleID = {INT}", AdoFldLong(pfldsTemplateRuleT, "ID"));
	FieldsPtr pfldsDetails = prsDetails->Fields;
	while (!prsDetails->eof) {
		switch (AdoFldLong(pfldsDetails, "ObjectType")) {
		case 1:
			pNewInfo->m_nTypeListObjectType = 1;
			pNewInfo->m_aryTypeList.Add(AdoFldLong(pfldsDetails, "ObjectID"));
			break;
		case 101:
			pNewInfo->m_nTypeListObjectType = 101;
			pNewInfo->m_aryTypeList.Add(AdoFldLong(pfldsDetails, "ObjectID"));
			break;
		case 2:
			pNewInfo->m_nPurposeListObjectType = 2;
			pNewInfo->m_aryPurposeList.Add(AdoFldLong(pfldsDetails, "ObjectID"));
			break;
		case 102:
			pNewInfo->m_nPurposeListObjectType = 102;
			pNewInfo->m_aryPurposeList.Add(AdoFldLong(pfldsDetails, "ObjectID"));
			break;
		case 3:
			pNewInfo->m_nBookingCount = AdoFldLong(pfldsDetails, "ObjectID");
			break;
		default:
			// Unexpected detail object type
			AfxThrowNxException("Unknown detail object type given");
			break;
		}
		prsDetails->MoveNext();
	}

	// (c.haag 2003-08-01 12:38) - Loop through permissions loading them into our arrays
	_RecordsetPtr prsPerms = CreateParamRecordset("SELECT UserGroupID, Permissions FROM PermissionT WHERE ObjectID IN (SELECT ID FROM SecurityObjectT WHERE BuiltInID = {INT} AND ObjectValue = {INT})",
		bioSchedTemplateRules, AdoFldLong(pfldsTemplateRuleT, "ID"));
	FieldsPtr pfldsPerms = prsPerms->Fields;
	while (!prsPerms->eof)
	{
		pNewInfo->m_adwUsers.Add(AdoFldLong(pfldsPerms, "UserGroupID"));
		pNewInfo->m_adwPerms.Add(AdoFldLong(pfldsPerms, "Permissions"));
		prsPerms->MoveNext();
	}
	return pNewInfo;
}

CTemplateExceptionInfo* CTemplateEntryDlg::NewException(FieldsPtr &pfldsTemplateExceptionT)
{
	CTemplateExceptionInfo *pNewInfo = new CTemplateExceptionInfo(pfldsTemplateExceptionT);
	return pNewInfo;
}

int CTemplateEntryDlg::AddTemplate()
{
	// (z.manning, 02/27/2007) - PLID 23745 - We used to return m_nTemplateID here, but it was never
	// even getting set when saving a new template, nor is it currently referenced by anywhere that
	// calls this function;
	return DoModal();
}

int CTemplateEntryDlg::EditTemplate(long nTemplateID)
{
	m_nTemplateID = nTemplateID;
	return DoModal();
}

void CTemplateEntryDlg::OnChooseColorBtn() 
{
	while (1) {
		m_ctrlColorPicker.SetColor(m_nColor);
		m_ctrlColorPicker.ShowColor();

		if (m_ctrlColorPicker.GetColor() != 0)
			break;

		MsgBox("Sorry, black is not a valid template color. Please pick another one.");
	}
	m_nColor = m_ctrlColorPicker.GetColor();
	GetDlgItem(IDC_CHOOSE_COLOR_BTN)->RedrawWindow();
}

void CTemplateEntryDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if (nIDCtl == IDC_CHOOSE_COLOR_BTN) {
		CDC *pdc = CDC::FromHandle(lpDrawItemStruct->hDC);
		CRect rc = lpDrawItemStruct->rcItem;
		
		// Draw the border of the button as 3d
		rc.DeflateRect(1, 1, 1, 1);
		pdc->Draw3dRect(&rc, GetSysColor(COLOR_BTNHIGHLIGHT), GetSysColor(COLOR_3DSHADOW));
		
		// Create rectangle to put the color
		rc.DeflateRect(1, 1, 3, 3);
		
		// Draw colored rectangle
		CBrush brshColor(m_nColor);
		CPen penColor(PS_SOLID, 0, m_nColor);
		CBrush * brshTemp = pdc->SelectObject(&brshColor);
		CPen * penTemp = pdc->SelectObject(&penColor);
		pdc->Rectangle(rc);
		pdc->SelectObject(penTemp);
		pdc->SelectObject(brshTemp);
		
		// Draw the border around the rectangle appropriately for whether or not the button is pressed
		if (lpDrawItemStruct->itemState & ODS_SELECTED) {
			rc.InflateRect(-1, -1, 1, 1);
			pdc->Draw3dRect(&rc, GetSysColor(COLOR_3DDKSHADOW), GetSysColor(COLOR_BTNHIGHLIGHT));
		} else {
			rc.InflateRect(0, 0, 1, 1);
			pdc->Draw3dRect(&rc, GetSysColor(COLOR_3DFACE), GetSysColor(COLOR_3DFACE));
		}

		// Set the border around the whole button to black if the button has the focus
		rc = lpDrawItemStruct->rcItem;
		if (lpDrawItemStruct->itemState & ODS_FOCUS) {
			pdc->Draw3dRect(&rc, GetSysColor(COLOR_3DDKSHADOW), GetSysColor(COLOR_3DDKSHADOW));
			rc.DeflateRect(0, 0, 1, 1);
			pdc->Draw3dRect(&rc, GetSysColor(COLOR_3DDKSHADOW), GetSysColor(COLOR_3DDKSHADOW));
		} else {
			pdc->Draw3dRect(&rc, GetSysColor(COLOR_BTNHIGHLIGHT), GetSysColor(COLOR_3DDKSHADOW));
		}
	} else {	
		CNxDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
	}
}

//TES 6/19/2010 - PLID 39262 - Dead code
/*long CTemplateEntryDlg::GetTemplateID(const CString &strTemplateName)
{
	long nAns = -1;
	try {
		_RecordsetPtr prs = CreateRecordset("SELECT ID FROM TemplateT WHERE Name = '%s'", _Q(strTemplateName));
		if (!prs->eof) {
			nAns = AdoFldLong(prs, "ID", -1);
		}
	} NxCatchAll("CTemplateEntryDlg::GetTemplateID");
	
	return nAns;
}*/

CString CTemplateEntryDlg::FormatExceptionText(const CTemplateExceptionInfo* pException)
{
	CString str;
	if (pException->m_dtStartDate == pException->m_dtEndDate) {
		str.Format("On %s: %s", FormatDateTimeForInterface(pException->m_dtStartDate, NULL, dtoDate),
			pException->m_strDescription);
	} else {
		str.Format("From %s through %s: %s", FormatDateTimeForInterface(pException->m_dtStartDate, NULL, dtoDate),
			FormatDateTimeForInterface(pException->m_dtEndDate, NULL, dtoDate), pException->m_strDescription);
	}
	return str;
}

void CTemplateEntryDlg::OnCancel() 
{
	// (z.manning 2011-12-07 17:07) - PLID 46906 - Moved memory clean up to another function
	CleanUp();
	
	CNxDialog::OnCancel();
}

void CTemplateEntryDlg::OnAddRuleBtn() 
{
	//TES 6/19/2010 - PLID 39262 - Resource Availability templates don't have rules.
	if(m_bUseResourceAvailTemplates) {
		ASSERT(FALSE);
		return;
	}

	CTemplateRuleInfo *pRule;
	pRule = new CTemplateRuleInfo;

	CRuleEntryDlg dlg(this);
	if (dlg.ZoomRule(pRule) == IDOK) { 
		// (c.haag 2007-02-21 11:20) - PLID 23784 - The rule list is now a datalist 2.0
		IRowSettingsPtr pRow = m_pRuleList->GetNewRow();
		pRow->Value[ecrData] = (long)pRule;
		pRow->Value[ecrDescription] = (LPCTSTR)pRule->m_strDescription;
		pRow->Value[ecrOriginalData] = (long)NULL;
		m_pRuleList->AddRowAtEnd(pRow, NULL);
	} else {
		delete pRule;
	}
	ReflectCurrentStateOnBtns();
}

void CTemplateEntryDlg::OnEditRuleBtn() 
{
	//TES 6/19/2010 - PLID 39262 - Resource Availability templates don't have rules.
	if(m_bUseResourceAvailTemplates) {
		ASSERT(FALSE);
		return;
	}

	IRowSettingsPtr pRowSel = m_pRuleList->CurSel;
	// Make sure something is selected
	if (NULL != pRowSel)
	{
		CTemplateRuleInfo *pRule = (CTemplateRuleInfo *)(long)pRowSel->Value[ecrData];
		CTemplateRuleInfo *pOriginalRule = (CTemplateRuleInfo*)VarLong(pRowSel->Value[eceOriginalData]);
		if(pOriginalRule == NULL) {
			// (z.manning 2011-12-08 09:36) - PLID 46906 - Since they are editing this rule, keep track of the
			// original value of the rule so we can later determine which actually changed.
			pOriginalRule = new CTemplateRuleInfo;
			*pOriginalRule = *pRule;
			pRowSel->Value[ecrOriginalData] = (long)pOriginalRule;
		}
		CRuleEntryDlg dlg(this);
		if (dlg.ZoomRule(pRule) == IDOK) { 
			// (c.haag 2007-02-21 11:22) - PLID 23784 - The rule list is now a datalist 2.0
			pRowSel->Value[ecrDescription] = (LPCTSTR)pRule->m_strDescription;
		}
	}
	ReflectCurrentStateOnBtns();
}

void CTemplateEntryDlg::OnRemoveRuleBtn() 
{
	//TES 6/19/2010 - PLID 39262 - Resource Availability templates don't have rules.
	if(m_bUseResourceAvailTemplates) {
		ASSERT(FALSE);
		return;
	}

	// (z.manning 2008-06-12 11:50) - PLID 29273 - Do not allow any template related
	// deleting without this permission.
	//TES 2/20/2015 - PLID 64336 - This was just checking bioSchedTemplating, regardless of m_bUseResourceAvailTemplates
	if (!CheckCurrentUserPermissions(m_bUseResourceAvailTemplates ? bioResourceAvailTemplating : bioSchedTemplating, sptDelete)) {
		return;
	}

	IRowSettingsPtr pRowSel = m_pRuleList->CurSel;
	// Make sure something is selected
	if (NULL != pRowSel)
	{
		if (AfxMessageBox("Are you sure you want to remove the selected rule?", 
							MB_YESNO | MB_ICONQUESTION) == IDYES) {
			CTemplateRuleInfo *pRule;
			pRule = (CTemplateRuleInfo *)(long)pRowSel->Value[ecrData];
			CTemplateRuleInfo *pOriginalRule = (CTemplateRuleInfo*)(long)pRowSel->Value[ecrOriginalData];
			// (z.manning 2011-12-09 09:34) - PLID 46906 - Keep track of deleted rules
			if(pRule->m_nID != -1) {
				m_arynDeletedRuleIDs.Add(pRule->m_nID);
			}
			m_pRuleList->RemoveRow(pRowSel);			
			if (pRule) {
				delete pRule;
			}
			// (z.manning 2011-12-07 17:19) - PLID 46906 - Clean up original data
			if(pOriginalRule != NULL) {
				delete pOriginalRule;
			}
		}
	}
	ReflectCurrentStateOnBtns();
}

void CTemplateEntryDlg::ReflectCurrentStateOnBtns()
{
	// (c.haag 2006-11-13 09:56) - PLID 5993 - Enable or disable the
	// edit and delete buttons based on whether or not an item in the
	// list is selected.
	if (NULL == m_pExceptionList->CurSel || !m_bAllowEdit) {
		GetDlgItem(IDC_EDIT_EXCEPTION_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_REMOVE_EXCEPTION_BTN)->EnableWindow(FALSE);
	} else {
		GetDlgItem(IDC_EDIT_EXCEPTION_BTN)->EnableWindow(TRUE);
		GetDlgItem(IDC_REMOVE_EXCEPTION_BTN)->EnableWindow(TRUE);
		//TES 2/23/2007 - PLID 24903 - There were 4 different buttons trying to be the default on this dialog,
		// from now on it will just be IDOK.
		//SetDefID(IDC_EDIT_EXCEPTION_BTN);
	}

	//TES 6/19/2010 - PLID 39262 - Resource Availability templates don't have rules.
	if(!m_bUseResourceAvailTemplates) {
		// Enable or disable the edit and delete buttons based
		// on whether or not an item in the list is selected
		if (NULL == m_pRuleList->CurSel || !m_bAllowEdit) {
			GetDlgItem(IDC_EDIT_RULE_BTN)->EnableWindow(FALSE);
			GetDlgItem(IDC_REMOVE_RULE_BTN)->EnableWindow(FALSE);
		} else {
			GetDlgItem(IDC_EDIT_RULE_BTN)->EnableWindow(TRUE);
			GetDlgItem(IDC_REMOVE_RULE_BTN)->EnableWindow(TRUE);
			//TES 2/23/2007 - PLID 24903 - There were 4 different buttons trying to be the default on this dialog,
			// from now on it will just be IDOK.
			//SetDefID(IDC_EDIT_RULE_BTN);
		}
	}

	// Enable or disable the edit and delete buttons based
	// on whether or not an item in the list is selected
	if (NULL == m_pLineItemList->CurSel || !m_bAllowEdit) {
		GetDlgItem(IDC_EDIT_LINE_ITEM_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_REMOVE_LINE_ITEM_BTN)->EnableWindow(FALSE);
	} else {
		GetDlgItem(IDC_EDIT_LINE_ITEM_BTN)->EnableWindow(TRUE);
		GetDlgItem(IDC_REMOVE_LINE_ITEM_BTN)->EnableWindow(TRUE);
		//TES 2/23/2007 - PLID 24903 - There were 4 different buttons trying to be the default on this dialog,
		// from now on it will just be IDOK.
		//SetDefID(IDC_EDIT_LINE_ITEM_BTN);
	}
}

void CTemplateEntryDlg::OnBtnPreviewReport()
{
	//TES 6/19/2010 - PLID 39262 - We don't show this button for Resource Availability templtes
	if(m_bUseResourceAvailTemplates) {
		ASSERT(FALSE);
		return;
	}

	// (z.manning, 12/20/2006) - PLID 23248 - Added try/catch to this function.
	try {

		if(IDNO == MessageBox("This will save and close this template (and close the template editor screen). Are you sure you wish to do this?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		if(!Save()) {
			return;
		}

		CWaitCursor pWait;

		CReportInfo infReport = CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(429)];
		infReport.nExtraID = m_nTemplateID;
		infReport.nDateRange = -1; //All dates
		infReport.nDateFilter = 1;

		CRParameterInfo *paramInfo;
		CPtrArray paParams;

		paramInfo = new CRParameterInfo;
		COleDateTime dt;
		dt.ParseDateTime("01/01/1000");
		paramInfo->m_Data = FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS);
		paramInfo->m_Name = "DateFrom";
		paParams.Add((void *)paramInfo);

		paramInfo = new CRParameterInfo;
		dt.ParseDateTime("12/31/5000");
		paramInfo->m_Data = FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS);
		paramInfo->m_Name = "DateTo";
		paParams.Add((void *)paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = "StartDate";
		paramInfo->m_Name = "DateFilter";
		paParams.Add((void *)paramInfo);

		RunReport(&infReport, &paParams, TRUE, (CWnd *)this, "Payment Dialog");
		ClearRPIParameterList(&paParams);	//DRT - PLID 18085 - Cleanup after ourselves

		m_bIsPreviewing = TRUE;

		CNxDialog::OnOK();

	}NxCatchAll("CTemplateEntryDlg::OnBtnPreviewReport");
}

BOOL CTemplateEntryDlg::Save()
{
	try
	{
		if (UpdateData(TRUE))
		{
			// Make sure the template name is valid
			m_strTemplateName.TrimLeft();
			m_strTemplateName.TrimRight();
			if (m_strTemplateName.IsEmpty()) {
				MsgBox("In order to keep logical track of your templates, it is necessary\n"
					"to assign a name to each one.  Please enter an appropriate name\n"
					"for this template in the field labeled 'Template Name'.");
				return FALSE;
			}

			//TES 6/19/2010 - PLID 32692 - If this is a resource availability template, make sure we have a location selected.
			long nLocationID = -1;
			if(m_bUseResourceAvailTemplates) {
				IRowSettingsPtr pLocRow = m_pLocationCombo->CurSel;
				if(pLocRow) {
					nLocationID = VarLong(pLocRow->GetValue(0),-1);
				}
				if(nLocationID) {
					if(m_pLocationCombo->IsComboBoxTextInUse) {
						//TES 6/21/2010 - PLID 32692 - It's an inactive location, we'll have stored the id in m_nPendingLocationID
						nLocationID = m_nPendingLocationID;
					}
				}
				if(nLocationID == -1) {
					MsgBox("Please select a location to associate with the specified resources and times for this template.");
					GetDlgItem(IDC_TEMPLATE_LOCATION)->SetFocus();
					m_pLocationCombo->DropDownState = g_cvarTrue;
					return FALSE;
				}
			}

			// (c.haag 2006-11-15 11:40) - PLID 23423 - Check the line item array, not the list, for the count
			// (z.manning, 11/28/2006) - There's is nothing wrong with saving a template without any line items,
			// so let's warn them instead of demanding that they enter a line item.
			if (m_pLineItemList->FindAbsoluteFirstRow(VARIANT_FALSE) == NULL) {
				if(IDNO == MessageBox("There are currently no line items entered for this template.  \r\n\r\n"
					"Are you sure you want to save this empty template?", NULL, MB_YESNO))
				{
					return FALSE;
				}
			}

			// Save the template into the tables
			SaveCurrentTemplate(nLocationID);

			// (c.haag 2006-12-11 15:55) - PLID 23808 - Post the change to other instances of Practice
			//TES 6/21/2010 - PLID 5888 - Send the appropriate tablechecker
			CClient::RefreshTable(m_bUseResourceAvailTemplates ? NetUtils::ResourceAvailTemplateT : NetUtils::TemplateT, m_nTemplateID);

			// (z.manning 2011-12-07 17:10) - PLID 46906 - Moved memory clean up to another function
			CleanUp();
		}
		return TRUE;

	}NxCatchAll("Error saving template.");

	return FALSE;
}

void CTemplateEntryDlg::OnAddExceptionBtn() 
{
	//
	// (c.haag 2006-11-13 10:18) - PLID 5993 - We can now add exceptions. When we do,
	// we have to pass an array of all the existing exceptions first because the dialog
	// needs it for validation (checking for duplicate names and date ranges)
	//
	CTemplateExceptionInfo *pException = new CTemplateExceptionInfo;
	CArray<CTemplateExceptionInfo*, CTemplateExceptionInfo*> aExceptions;

	// Populate the list of existing exceptions
	// (c.haag 2007-02-21 11:36) - PLID 23784 - We now use a datalist 2
	IRowSettingsPtr pRow = m_pExceptionList->GetFirstRow();
	while (pRow) {
		aExceptions.Add( (CTemplateExceptionInfo*)(long)pRow->Value[eceData] );
		pRow = pRow->GetNextRow();
	}

	// Have the user create the exception
	pException->m_nTemplateID = m_nTemplateID;
	//TES 6/19/2010 - PLID 39262 - Pass in whether we're using Resource Availability templates
	if (m_dlgException.ZoomException(m_bUseResourceAvailTemplates, pException, aExceptions) == IDOK) { 
		// (c.haag 2007-02-21 11:37) - PLID 23784 - We now use a datalist 2
		pRow = m_pExceptionList->GetNewRow();
		pRow->Value[eceData] = (long)pException;
		pRow->Value[eceDescription] = (LPCTSTR)FormatExceptionText(pException);
		pRow->Value[eceOriginalData] = (long)NULL;
		m_pExceptionList->AddRowAtEnd(pRow, NULL);
	}
	else {
		delete pException;
	}
	ReflectCurrentStateOnBtns();	
}

void CTemplateEntryDlg::OnEditExceptionBtn() 
{
	//
	// (c.haag 2006-11-13 10:18) - PLID 5993 - We can now edit exceptions. When we do,
	// we have to pass an array of all the existing exceptions first because the dialog
	// needs it for validation (checking for duplicate names and date ranges)
	//
	IRowSettingsPtr pRowSel = m_pExceptionList->CurSel;
	// Make sure something is selected
	if (NULL != pRowSel)
	{
		CArray<CTemplateExceptionInfo*, CTemplateExceptionInfo*> aExceptions;

		// Populate the list of existing exceptions
		// (c.haag 2007-02-21 11:36) - PLID 23784 - We now use a datalist 2
		IRowSettingsPtr pRow = m_pExceptionList->GetFirstRow();
		while (pRow) {
			aExceptions.Add( (CTemplateExceptionInfo*)(long)pRow->Value[eceData] );
			pRow = pRow->GetNextRow();
		}

		// Have the user edit the exception
		CTemplateExceptionInfo* pException = (CTemplateExceptionInfo*)(long)pRowSel->Value[eceData];
		CTemplateExceptionInfo *pOriginalException = (CTemplateExceptionInfo*)VarLong(pRowSel->Value[eceOriginalData]);
		if(pOriginalException == NULL) {
			// (z.manning 2011-12-08 09:36) - PLID 46906 - Since they are editing this exception, keep track of the
			// original value of the excption so we can later determine which actually changed.
			pOriginalException = new CTemplateExceptionInfo;
			*pOriginalException = *pException;
			pRowSel->Value[eceOriginalData] = (long)pOriginalException;
		}
		//TES 6/19/2010 - PLID 39262 - Pass in whether we're using Resource Availability templates
		if (m_dlgException.ZoomException(m_bUseResourceAvailTemplates, pException, aExceptions) == IDOK) { 
			// (c.haag 2007-02-21 11:39) - PLID 23784 - We now use a datalist 2
			pRowSel->Value[eceDescription] = (LPCTSTR)FormatExceptionText(pException);
		}
	}
	ReflectCurrentStateOnBtns();
}

void CTemplateEntryDlg::OnRemoveExceptionBtn() 
{
	// (z.manning 2008-06-12 11:50) - PLID 29273 - Do not allow any template related
	// deleting without this permission.
	//TES 2/20/2015 - PLID 64336 - This was just checking bioSchedTemplating, regardless of m_bUseResourceAvailTemplates
	if (!CheckCurrentUserPermissions(m_bUseResourceAvailTemplates ? bioResourceAvailTemplating : bioSchedTemplating, sptDelete)) {
		return;
	}

	// (c.haag 2006-11-13 10:25) - PLID 5993 - We can now remove exceptions
	IRowSettingsPtr pRowSel = m_pExceptionList->CurSel;
	// Make sure something is selected
	if (NULL != pRowSel) {
		if (AfxMessageBox("Are you sure you want to remove the selected exception?", 
							MB_YESNO | MB_ICONQUESTION) == IDYES) {
			CTemplateExceptionInfo *pException = (CTemplateExceptionInfo*)(long)pRowSel->Value[eceData];
			CTemplateExceptionInfo *pOriginalException = (CTemplateExceptionInfo*)(long)pRowSel->Value[eceOriginalData];
			m_pExceptionList->RemoveRow(pRowSel);
			if(pException->m_nID != -1) {
				// (z.manning 2011-12-08 10:21) - PLID 46906 - Keep track of deleted exceptions
				m_arynDeletedExceptionIDs.Add(pException->m_nID);
			}
			if (pException) {
				delete pException;
			}
			// (z.manning 2011-12-07 17:18) - PLID 46906 - Clean up original data
			if(pOriginalException != NULL) {
				delete pOriginalException;
			}
		}
	}
	ReflectCurrentStateOnBtns();
}

void CTemplateEntryDlg::OnCheckHideOldLineitems() 
{
	//
	// (c.haag 2006-11-15 11:15) - PLID 23423 - Update the preference for hiding old line items
	//
	int nChecked = ((CButton*)GetDlgItem(IDC_CHECK_HIDE_OLD_LINEITEMS))->GetCheck();
	//TES 6/19/2010 - PLID 39262 - Store the checkboxes separately for Resource Availability templates
	CString strProperty = m_bUseResourceAvailTemplates?"HideOldResourceAvailTemplateLineItemsInConfigDlg":"HideOldTemplateLineItemsInConfigDlg";
	SetRemotePropertyInt(strProperty, nChecked ? 1 : 0, 0, GetCurrentUserName());

	// (c.haag 2006-11-15 12:32) - Now update the visible list
	RequeryLineItemList();
}

// (z.manning 2014-12-23 10:47) - PLID 64296
void CTemplateEntryDlg::OnCheckHideAppliedLineItems() 
{
	int nChecked = ((CButton*)GetDlgItem(IDC_HIDE_COLLECTION_LINE_ITEMS))->GetCheck();
	SetRemotePropertyInt("HideAppliedTemplateLineItemsInConfigDlg", nChecked == BST_CHECKED ? 1 : 0, 0, GetCurrentUserName());

	RequeryLineItemList();
}

BOOL CTemplateEntryDlg::ShouldLineItemBeVisible(CTemplateLineItemInfo* pLineItem)
{
	//
	// (c.haag 2006-11-15 12:41) - PLID 23423 - Returns true if a line item should be on the screen,
	// or false if not. If the end date is a valid date before today, and the user elected to hide 
	// those items, we return false
	//
	//TES 6/19/2010 - PLID 39262 - Store the checkboxes separately for Resource Availability templates
	CString strProperty = m_bUseResourceAvailTemplates?"HideOldResourceAvailTemplateLineItemsInConfigDlg":"HideOldTemplateLineItemsInConfigDlg";
	BOOL bHideOldLineItems = (GetRemotePropertyInt(strProperty, 0, 0, GetCurrentUserName())) ? TRUE : FALSE;
	BOOL bHideAppliedLineItems = (GetRemotePropertyInt("HideAppliedTemplateLineItemsInConfigDlg", 0, 0, GetCurrentUserName()) != 0 ? TRUE : FALSE);
	if (bHideOldLineItems) {
		COleDateTime dtToday = COleDateTime::GetCurrentTime();
		dtToday.SetDateTime(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay(), 0, 0, 0);
		if (COleDateTime::valid == pLineItem->m_dtEndDate.GetStatus() && pLineItem->m_dtEndDate < dtToday) {
			return FALSE;
		}
	}

	// (z.manning 2014-12-23 10:51) - PLID 64296 - Also handle the option to hide applied items
	if (bHideAppliedLineItems && pLineItem->m_nCollectionID != -1) {
		return FALSE;
	}

	return TRUE;
}

void CTemplateEntryDlg::RequeryLineItemList()
{
	m_pLineItemList->SetRedraw(VARIANT_FALSE);

	// Fill the list
	for(IRowSettingsPtr pLineItemRow = m_pLineItemList->FindAbsoluteFirstRow(VARIANT_FALSE); pLineItemRow != NULL; pLineItemRow = m_pLineItemList->FindAbsoluteNextRow(pLineItemRow, VARIANT_FALSE))
	{
		CTemplateLineItemInfo *pLineItem = (CTemplateLineItemInfo*)VarLong(pLineItemRow->Value[ecliData]);

		// If we are not including old line items, check that the date range of
		// the current item spans the present or future
		if (ShouldLineItemBeVisible(pLineItem)) {
			pLineItemRow->PutVisible(VARIANT_TRUE);
		}
		else {
			pLineItemRow->PutVisible(VARIANT_FALSE);
		}
	}

	m_pLineItemList->SetRedraw(VARIANT_TRUE);

	// (z.manning 2015-01-13 16:54) - PLID 64296 - Update the button states in case our selection changed
	ReflectCurrentStateOnBtns();
}

BEGIN_EVENTSINK_MAP(CTemplateEntryDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CTemplateEntryDlg)
	ON_EVENT(CTemplateEntryDlg, IDC_LIST_LINE_ITEMS, 2 /* SelChanged */, OnSelChangedListLineItems, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CTemplateEntryDlg, IDC_LIST_LINE_ITEMS, 3 /* DblClickCell */, OnDblClickCellListLineItems, VTS_DISPATCH VTS_I2)
	ON_EVENT(CTemplateEntryDlg, IDC_LIST_RULES, 2 /* SelChanged */, OnSelChangedListRules, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CTemplateEntryDlg, IDC_LIST_RULES, 3 /* DblClickCell */, OnDblClickCellListRules, VTS_DISPATCH VTS_I2)
	ON_EVENT(CTemplateEntryDlg, IDC_LIST_EXCEPTIONS, 2 /* SelChanged */, OnSelChangedListExceptions, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CTemplateEntryDlg, IDC_LIST_EXCEPTIONS, 3 /* DblClickCell */, OnDblClickCellListExceptions, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CTemplateEntryDlg, IDC_TEMPLATE_LOCATION, 20, CTemplateEntryDlg::OnTrySetSelFinishedTemplateLocation, VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CTemplateEntryDlg::OnSelChangedListLineItems(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	// (c.haag 2007-02-21 11:08) - PLID 23784 - We now capture events from the new datalist 2
	// list rather than the old listbox form control
	// the user selected a line item, make sure buttons are shown correctly
	ReflectCurrentStateOnBtns();
}

void CTemplateEntryDlg::OnDblClickCellListLineItems(LPDISPATCH lpRow, short nColIndex) 
{
	// (c.haag 2007-02-21 11:08) - PLID 23784 - We now capture events from the new datalist 2
	// list rather than the old listbox form control
	if(!m_bAllowEdit)
		return;

	OnEditLineItemBtn();
}

void CTemplateEntryDlg::OnSelChangedListRules(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	// (c.haag 2007-02-21 11:13) - PLID 23784 - We now capture events from the new datalist 2
	// list rather than the old listbox form control
	// the user selected a line item, make sure buttons are shown correctly
	ReflectCurrentStateOnBtns();
}

void CTemplateEntryDlg::OnDblClickCellListRules(LPDISPATCH lpRow, short nColIndex) 
{
	// (c.haag 2007-02-21 11:13) - PLID 23784 - We now capture events from the new datalist 2
	// list rather than the old listbox form control
	// the user selected a line item, make sure buttons are shown correctly
	if(!m_bAllowEdit)
		return;

	OnEditRuleBtn();	
}

void CTemplateEntryDlg::OnSelChangedListExceptions(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	// (c.haag 2006-11-13 10:23) - PLID 5993 - The user selected an exception,
	// so make sure buttons are shown correctly
	// (c.haag 2007-02-21 11:28) - PLID 23784 - We now capture events from the new datalist 2
	ReflectCurrentStateOnBtns();	
}

void CTemplateEntryDlg::OnDblClickCellListExceptions(LPDISPATCH lpRow, short nColIndex) 
{
	// (c.haag 2006-11-13 10:23) - PLID 5993 - The user double-clicked on an exception to edit it
	// (c.haag 2007-02-21 11:28) - PLID 23784 - We now capture events from the new datalist 2
	if(!m_bAllowEdit)
		return;

	OnEditExceptionBtn();	
}

void CTemplateEntryDlg::OnTrySetSelFinishedTemplateLocation(long nRowEnum, long nFlags)
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure && m_nPendingLocationID != -1) {
			//TES 6/21/2010 - PLID 39262 - maybe it's inactive?
			_RecordsetPtr rsLoc = CreateParamRecordset("SELECT Name FROM LocationsT "
				"WHERE ID = {INT}", m_nPendingLocationID);
			if(!rsLoc->eof) {
				m_pLocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLoc, "Name", "")));
			}
			else 
				m_pLocationCombo->PutCurSel(NULL);
		}
		else {
			m_nPendingLocationID = -1;
		}

	}NxCatchAll("Error in CTemplateEntryDlg::OnTrySetSelFinishedTemplateLocation");
}

// (z.manning 2011-12-07 17:10) - PLID 46906 - Moved some duplicated clean up logic to its own function
void CTemplateEntryDlg::CleanUp()
{
	{
		// Delete all the temporary line item dynamic variables
		// (c.haag 2006-11-15 11:40) - PLID 23423 - We must delete the entries in our member array
		CTemplateLineItemInfo *pLineItem;
		IRowSettingsPtr pRow = m_pLineItemList->FindAbsoluteFirstRow(VARIANT_FALSE);
		while (NULL != pRow) {
			if (NULL != (pLineItem = (CTemplateLineItemInfo*)(long)pRow->Value[ecliData])) {
				delete pLineItem;
			}
			// (z.manning 2011-12-07 16:46) - PLID 46906 - Also clean up any original data we may have
			if (NULL != (pLineItem = (CTemplateLineItemInfo*)(long)pRow->Value[ecliOriginalData])) {
				delete pLineItem;
			}
			pRow = m_pLineItemList->FindAbsoluteNextRow(pRow, VARIANT_FALSE);
		}
	}

	{
		// Delete all the temporary rule dynamic variables
		// (c.haag 2007-02-21 11:19) - PLID 23784 - The rule list is now a datalist 2.0 control
		CTemplateRuleInfo *pRule;
		IRowSettingsPtr pRow = m_pRuleList->GetFirstRow();
		while (NULL != pRow) {
			if (NULL != (pRule = (CTemplateRuleInfo *)(long)pRow->Value[ecrData])) {
				delete pRule;
			}
			// (z.manning 2011-12-07 16:46) - PLID 46906 - Also clean up any original data we may have
			if (NULL != (pRule = (CTemplateRuleInfo *)(long)pRow->Value[ecrOriginalData])) {
				delete pRule;
			}
			pRow = pRow->GetNextRow();
		}
	}

	{
		// (c.haag 2006-11-13 09:24) - PLID 5993 - Delete all the temporary exception info objects
		// (c.haag 2007-02-21 11:31) - PLID 23784 - The exception list is now a datalist 2.0 control
		CTemplateExceptionInfo* pException;
		IRowSettingsPtr pRow = m_pExceptionList->GetFirstRow();
		while (NULL != pRow) {
			if (NULL != (pException = (CTemplateExceptionInfo *)(long)pRow->Value[eceData])) {
				delete pException;
			}
			// (z.manning 2011-12-07 16:46) - PLID 46906 - Also clean up any original data we may have
			if (NULL != (pException = (CTemplateExceptionInfo *)(long)pRow->Value[eceOriginalData])) {
				delete pException;
			}
			pRow = pRow->GetNextRow();
		}
	}
}