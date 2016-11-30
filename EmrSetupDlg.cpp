// EMRSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "EMRSetupDlg.h"
#include "globaldatautils.h"
#include "nxstandard.h"
#include "EMRActionDlg.h"
#include "EMRCategoriesDlg.h"
#include "EMRHeaderDlg.h"
#include "OfficeVisitConfigDlg.h"
#include "globaldrawingutils.h"
#include "SelectImageDlg.h"
#include "mergeengine.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "singleselectdlg.h"
#include "client.h"
#include "NxPackets.h"
#include "EmrCollectionSetupDlg.h"
#include "EmrUtils.h"
#include "MultiSelectDlg.h"
#include "EmrTemplateManagerDlg.h"
#include "EMRSelectProductDlg.h"
#include "EmrItemEntryDlg.h"
#include "GlobalUtils.h"
#include "EMRProblemStatusDlg.h"
#include "EMRItemLinkingDlg.h"
#include "internationalutils.h"
#include "EMRVisitTypesDlg.h"
#include "EMREMChecklistSetupDlg.h"
#include "SignatureDlg.h"
#include "InterventionTemplatesDlg.h"
#include "WellnessDataUtils.h"
#include "EmrProviderConfigDlg.h"
#include "EMNStatusConfigDlg.h"
#include "EmrCodingGroupEditDlg.h"

#define NXM_EXPLICIT_CHECK_RADIO (NXM_BASE + 0x0057)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_DELETE_PROCEDURE		37000
#define ID_DELETE_DIAG			37001
#define ID_DELETE_DATA			37002
#define ID_INACTIVATE_DATA		37003
#define ID_ACTIVATE_DATA		37004

using namespace ADODB;
using namespace NXDATALISTLib;

// Returns the index of the first occurrence of the given value in the given array
// Returns -1 if the value doesn't occur in the array
int FindDWordArrayElement(const CDWordArray &ary, DWORD dwFindValue);
enum ECompareSetResult
{
	csrEqual,		// (eg. A={1,2,3}, B={2,1,3))	The sets are have the same elements
	csrSubset,		// (eg. A={1,2},   B={2,1,3))	All elements in the first set are in the second set, but not all elements in the second set are in the first
	csrSuperset,	// (eg. A={1,2,3}, B={1,3))		All elements in the second set are in the first set, but not all elements in the first set are in the second
	csrIntersect,	// (eg. A={1,2,3}, B={3,4,5))	The two sets share some elements, but each set has at least one element that is not in the other
	csrDisparate,	// (eg. A={1,2,3}, B={4,5,6))	The two sets share no elements
};
ECompareSetResult CompareDWordArray(const CDWordArray &ary1, const CDWordArray &ary2);
CString GenerateXMLFromSemiColonDelimitedIDList(const CString &strSemiColonDelimitedIDList);

// Prompts the user for various criteria and returns a filter sufficient for producing a 
// reasonably small set of EMR Info items to be included in a given mergeinfo for use in 
// editing a Word template.  Returns TRUE for success, FALSE if the user canceled.
BOOL GetEMRFilter(CString &strOutEMRFilter)
{
	//TES 11/1/2004 - Since ProcedureToEMRInfoT and DiagCodeToEMRInfoT are no longer editable, we don't want to use them.
	//Instead, we'll prompt them to select from a list of mints.
	
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(NULL, "EmrTemplateT");
	//m.hancock - 2/24/2006 - PLID 19428 - Set the flag to allow text wrapping
	dlg.m_bWrapText = true;
	// Prompt the user, including only the diag codes that have emr info items associated with them
	if (IDCANCEL == dlg.Open(
		"EmrTemplateT", "EmrTemplateT.Deleted = 0 AND EmrTemplateT.ID IN (SELECT TemplateID FROM EMRTemplateTopicsT)", 
		"EmrTemplateT.ID", "EmrTemplateT.Name", "Please select EMN Templates which will be merged"))
	{
		// User canceled
		return FALSE;
	}
	
	// Return
	CString strMintFilter;
	if (dlg.GetMultiSelectIDString() == "") {
		// The user chose no filter
		strMintFilter = "";
	} else {
		//(a.walling 2007-04-30 15:32) - PLID 25859 - Added delimiter to the GetMultiSelectIDString calls (was crashing otherwise)
		//All items that are a.) in this template, b.) spawned by an item in this template, c.) spawned by a list item
		//of an item in this template.
		//TES 7/12/2007 - PLID 16028 - This filter was only partially updated when the EmrInfoMasterID data structure
		// change was made.  As a result, this query always returned all items.
		//TES 9/12/2007 - PLID 16028 - My 7/12 fix still did not finish updating this query.  It should be fully 
		// up-to-date now.
		strMintFilter.Format("EmrInfoT.ID IN (SELECT ActiveEmrInfoID FROM EmrTemplateDetailsT INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID WHERE TemplateID IN (%s)) "
			"OR EmrInfoT.EmrInfoMasterID IN (SELECT DestID FROM EmrActionsT WHERE DestType = %i AND SourceType = %i AND SourceID IN "
			"(SELECT ActiveEmrInfoID FROM EmrTemplateDetailsT INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID  WHERE TemplateID IN (%s))) "
			"OR EmrInfoT.EmrInfoMasterID IN (SELECT DestID FROM EmrActionsT WHERE DestType = %i AND SourceType = %i AND SourceID IN "
			"(SELECT ID FROM EmrDataT WHERE EmrInfoID IN (SELECT ActiveEmrInfoID FROM EmrTemplateDetailsT INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID  WHERE TemplateID IN (%s))))",
			dlg.GetMultiSelectIDString(","), eaoEmrItem, eaoEmrItem, dlg.GetMultiSelectIDString(","), eaoEmrItem, eaoEmrDataItem, 
			dlg.GetMultiSelectIDString(","));
	}

	if (!strMintFilter.IsEmpty()) {
		strOutEMRFilter.Format("(%s)", strMintFilter);
	} 
	else {
		// 0: They're both empty, leave so we have no emr filter
		strOutEMRFilter.Empty();
	}

	// If we made it here the user has not canceled so return TRUE.
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CEMRSetupDlg dialog

CEMRSetupDlg::CEMRSetupDlg(CWnd* pParent)
	: CNxDialog(CEMRSetupDlg::IDD, pParent),
	m_EMRTemplateChecker(NetUtils::EMRTemplateT),
	m_EMRInfoChecker(NetUtils::EMRInfoT)
{
	//{{AFX_DATA_INIT(CEMRSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bSendTableCheckers = TRUE;
	m_pdlgTemplateManager = NULL;

	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_NexEMR_Module/The_NexEMR_Module.htm";

	// (d.thompson 2009-03-03) - PLID 33103
	m_bFiltering = false;
}


void CEMRSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRSetupDlg)
	DDX_Control(pDX, IDC_CHECK_SHOWINACTIVEITEMS, m_btnShowInactive);
	DDX_Control(pDX, IDC_ADD_INFO_ITEM, m_btnAddEmrItem);
	DDX_Control(pDX, IDC_DELETE_INFO_ITEM, m_btnDeleteEmrItem);
	DDX_Control(pDX, IDC_EDIT_MINT_TEMPLATE, m_btnEditEmnTemplates);
	DDX_Control(pDX, IDC_MANAGE_EMRCOLLECTIONS_BTN, m_btnManageCollections);
	DDX_Control(pDX, IDC_EDIT_EMR_TEMPLATE, m_btnEditWordTemplates);
	DDX_Control(pDX, IDC_EDIT_LINKED_ITEMS, m_btnEditLinkedItems);
	DDX_Control(pDX, IDC_EDIT_EMR_VISIT_TYPES, m_btnEditEmrVisitTypes);
	DDX_Control(pDX, IDC_EDIT_EMR_EM_CHECKLISTS, m_btnEditEmChecklists);
	DDX_Control(pDX, IDC_EMR_STATUS_LISTS, m_btnStatusLists);
	DDX_Control(pDX, IDC_OFFICE_VISIT, m_btnOfficeVisit);
	DDX_Control(pDX, IDC_EDIT_EMR_SIGNATURE, m_btnEditSignature);
	DDX_Control(pDX, IDC_FILTER_EMR_DETAILS, m_btnFilterDetails);
	DDX_Control(pDX, IDC_EDIT_WELLNESS_TEMPLATES, m_btnEditWellnessTemplates);
	DDX_Control(pDX, IDC_BTN_EDIT_IMAGE_STAMPS, m_btnEditImageStamps);
	DDX_Control(pDX, IDC_EMR_PROVIDER_CONFIG, m_btnProviderConfig);
	DDX_Control(pDX, IDC_EMR_CODING_SETUP_BUTTON, m_btnEmrCoding);
	//}}AFX_DATA_MAP	
}


BEGIN_MESSAGE_MAP(CEMRSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRSetupDlg)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ADD_INFO_ITEM, OnAddInfoItem)
	ON_BN_CLICKED(IDC_DELETE_INFO_ITEM, OnDeleteInfoItem)
	ON_BN_CLICKED(IDC_SETUP_HEADER, OnSetupHeader)
	ON_BN_CLICKED(IDC_EDIT_EMR_TEMPLATE, OnEditTemplate)
	ON_BN_CLICKED(IDC_MANAGE_EMRCOLLECTIONS_BTN, OnManageEmrcollections)
	ON_BN_CLICKED(IDC_OFFICE_VISIT, OnOfficeVisit)
	ON_BN_CLICKED(IDC_EMR_STATUS_LISTS, OnEditStatusLists)
	ON_BN_CLICKED(IDC_EDIT_MINT_TEMPLATE, OnEditMintTemplate)
	ON_MESSAGE(NXM_EDIT_EMR_OR_TEMPLATE, OnEditEMRTemplate)
	ON_COMMAND(ID_EMRSETUP_EDIT, OnEMRSetupEdit)
	ON_COMMAND(ID_EMRSETUP_COPY, OnEMRSetupCopy)
	ON_COMMAND(IDC_CHECK_SHOWINACTIVEITEMS, OnShowInactiveItems)
	ON_WM_DESTROY()
	ON_COMMAND(IDC_INACTIVATE_ITEM, OnInactivateItem)
	ON_COMMAND(IDC_ACTIVATE_ITEM, OnActivateItem)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_EDIT_LINKED_ITEMS, OnEditLinkedItems)
	// (j.jones 2007-08-16 08:36) - PLID 27054 - added Visit Types
	ON_BN_CLICKED(IDC_EDIT_EMR_VISIT_TYPES, OnEditEmrVisitTypes)
	// (j.jones 2007-08-16 10:48) - PLID 27055 - added E/M Checklists
	ON_BN_CLICKED(IDC_EDIT_EMR_EM_CHECKLISTS, OnEditEmrEmChecklists)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_EDIT_EMR_SIGNATURE, OnBnClickedEditEmrSignature)
	ON_BN_CLICKED(IDC_FILTER_EMR_DETAILS, OnFilterEMRDetails)
	ON_BN_CLICKED(IDC_EDIT_WELLNESS_TEMPLATES, OnEditWellnessTemplates)
	ON_BN_CLICKED(IDC_BTN_EDIT_IMAGE_STAMPS, OnBtnEditImageStamps)
	ON_BN_CLICKED(IDC_EMR_PROVIDER_CONFIG, &CEMRSetupDlg::OnBnClickedEmrProviderConfig)
	ON_BN_CLICKED(IDC_EMR_CODING_SETUP_BUTTON, &CEMRSetupDlg::OnBnClickedEmrCodingSetupButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRSetupDlg message handlers

HBRUSH CEMRSetupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (nCtlColor == CTLCOLOR_STATIC)
	{
		if (pWnd->GetExStyle() & WS_EX_TRANSPARENT)
		{	pDC->SetBkMode (TRANSPARENT);
			return (HBRUSH)GetStockObject(NULL_BRUSH);
		}

		pDC->SetBkColor(PaletteColor(GetNxColor(GNC_ADMIN, 0)));
		return hbr;
	}

	return hbr;
}

BOOL CEMRSetupDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {

		// (j.jones 2007-08-30 09:28) - PLID 27243 - disable the office visit incrementing
		// setup button if they have L2 EMR
		if(g_pLicense && g_pLicense->HasEMR(CLicense::cflrSilent) == 2) {
			GetDlgItem(IDC_OFFICE_VISIT)->ShowWindow(SW_HIDE);
		}
		else {
			// (z.manning 2008-10-22 15:41) - PLID 21082 - Otherwise we need to hide the edit signature button
			m_btnEditSignature.ShowWindow(SW_HIDE);
		}

		// Bind to the main item list
		m_pdlItemList = BindNxDataListCtrl(IDC_EMR_INFO_LIST);
		// (b.savon 2014-07-08 07:55) - PLID 62766 - Admin > NexEMR: When an EMR item row is selected, hitting the <enter> key should open the item.
		m_pdlItemList->PutAlwaysWantsReturnKey(VARIANT_TRUE);

		// Bind to the template filter
		m_nTemplateFilterID = -1;
		m_bAutoFilter = FALSE;
		m_pTemplateFilter = BindNxDataListCtrl(IDC_EMR_TEMPLATE_SELECT_LIST);

		// (z.manning, 04/16/2008) - PLID 29566 - Set button styles
		m_btnAddEmrItem.AutoSet(NXB_NEW);
		m_btnDeleteEmrItem.AutoSet(NXB_DELETE);
		m_btnEditEmnTemplates.AutoSet(NXB_MODIFY);
		m_btnManageCollections.AutoSet(NXB_MODIFY);
		m_btnEditWordTemplates.AutoSet(NXB_MODIFY);
		m_btnEditLinkedItems.AutoSet(NXB_MODIFY);
		m_btnEditEmrVisitTypes.AutoSet(NXB_MODIFY);
		m_btnEditEmChecklists.AutoSet(NXB_MODIFY);
		m_btnStatusLists.AutoSet(NXB_MODIFY);
		m_btnOfficeVisit.AutoSet(NXB_MODIFY);
		m_btnEditSignature.AutoSet(NXB_MODIFY);
		// (d.thompson 2009-03-03) - PLID 33103
		m_btnFilterDetails.SetIcon(IDI_FILTER);
		m_btnEditWellnessTemplates.AutoSet(NXB_MODIFY);
		// (j.jones 2010-02-10 17:01) - PLID 37224 - added ability to edit image stamps from Admin.
		m_btnEditImageStamps.AutoSet(NXB_MODIFY);
		m_btnProviderConfig.AutoSet(NXB_MODIFY); // (z.manning 2011-01-31 10:04) - PLID 42334
		m_btnEmrCoding.AutoSet(NXB_MODIFY); // (z.manning 2011-07-05 15:23) - PLID 44421

		// Create the template manager dialog
		m_pdlgTemplateManager = new CEmrTemplateManagerDlg(this);
		m_pdlItemList->Requery();

		// Set focus and return
		GetDlgItem(IDC_EMR_INFO_LIST)->SetFocus();
		return FALSE;  // return TRUE unless you set the focus to a control
					  // EXCEPTION: OCX Property Pages should return FALSE

	} NxCatchAllCall("CEMRSetupDlg::OnInitDialog", {
		EndDialog(IDCANCEL);
		return TRUE;  // return TRUE unless you set the focus to a control
					  // EXCEPTION: OCX Property Pages should return FALSE
	});
}

void CEMRSetupDlg::OnDestroy() 
{
	if (m_pdlgTemplateManager) {
		delete m_pdlgTemplateManager;
		m_pdlgTemplateManager = NULL;
	}
	CNxDialog::OnDestroy();
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CEMRSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRSetupDlg)
	ON_EVENT(CEMRSetupDlg, IDC_EMR_TEMPLATE_SELECT_LIST, 16 /* SelChosen */, OnSelChosenEmrTemplateSelectList, VTS_I4)
	ON_EVENT(CEMRSetupDlg, IDC_EMR_TEMPLATE_SELECT_LIST, 18 /* RequeryFinished */, OnRequeryFinishedEmrTemplateSelectList, VTS_I2)
	ON_EVENT(CEMRSetupDlg, IDC_EMR_INFO_LIST, 2 /* SelChanged */, OnSelChangedEmrInfoList, VTS_I4)
	ON_EVENT(CEMRSetupDlg, IDC_EMR_INFO_LIST, 3 /* DblClickCell */, OnDblClickCellEmrInfoList, VTS_I4 VTS_I2)
	ON_EVENT(CEMRSetupDlg, IDC_EMR_INFO_LIST, 18 /* RequeryFinished */, OnRequeryFinishedEmrInfoList, VTS_I2)
	ON_EVENT(CEMRSetupDlg, IDC_EMR_INFO_LIST, 6 /* RButtonDown */, OnRButtonDownEmrInfoList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMRSetupDlg, IDC_EMR_INFO_LIST, 16 /* SelChosen */, OnSelChosenEMRInfoList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

// (b.savon 2014-07-08 07:55) - PLID 62766 - Admin > NexEMR: When an EMR item row is selected, hitting the <enter> key should open the item.
void CEMRSetupDlg::OnSelChosenEMRInfoList(long nRow)
{
	if (nRow != sriNoRow) {
		//Queue the editor
		OnDblClickCellEmrInfoList(nRow, ilcName);
	}
}

void CEMRSetupDlg::OnAddInfoItem()
{
	try {
		// Give the user the "new item" dialog
		CWaitCursor wc;
		CEmrItemEntryDlg dlg(this);
		if (dlg.OpenWithMasterID(-1) == IDOK) {

			CString strType = "";
			_RecordsetPtr rs = CreateRecordset("SELECT "
				"CASE WHEN DataType = 1 THEN 'Text' WHEN DataType = 2 THEN 'Single-Select List' "
				"WHEN DataType = 3 THEN 'Multi-Select List' WHEN DataType = 4 THEN 'Image' "
				"WHEN DataType = 5 THEN 'Slider' WHEN DataType = 6 THEN 'Narrative' "
				"WHEN DataType = 7 THEN 'Table' END AS Type FROM EMRInfoT WHERE ID = %li", dlg.GetID());
			if(!rs->eof) {
				strType = AdoFldString(rs, "Type","");
			}
			rs->Close();

			// If the user saved the new item, then we simply add it to our list
			IRowSettingsPtr pRow = m_pdlItemList->GetRow(sriGetNewRow);
			pRow->PutValue(ilcID, dlg.GetID());
			pRow->PutValue(ilcName, (LPCTSTR)dlg.GetName());
			pRow->PutValue(ilcDataType, (LPCTSTR)strType);
			pRow->PutValue(ilcInactive, VARIANT_FALSE);
			pRow->PutValue(ilcEmrInfoMasterID, dlg.GetInfoMasterID());

			long nRow = m_pdlItemList->AddRow(pRow);
			// And select it
			ASSERT(nRow != sriNoRow);
			if (nRow != sriNoRow) {
				m_pdlItemList->PutCurSel(nRow);
			}
		}

		UpdateView();

		// And make sure the buttons are enabled appropriately
		// (j.jones 2006-10-17 09:38) - PLID 19895 - this is called inside UpdateView
		//EnableAppropriateFields();
	} NxCatchAll("CEMRSetupDlg::OnAddInfoItem");
}

void CEMRSetupDlg::OnDeleteInfoItem()
{
	try {
		if(m_pdlItemList->CurSel == -1) {
			return;
		}

		long nID = VarLong(m_pdlItemList->GetValue(m_pdlItemList->CurSel, ilcID));
		long nMasterID = VarLong(m_pdlItemList->GetValue(m_pdlItemList->CurSel, ilcEmrInfoMasterID));
		// (j.jones 2010-07-07 17:25) - PLID 39555 - changed to a byte for accuracy
		BYTE iDataSubType = VarByte(m_pdlItemList->GetValue(m_pdlItemList->CurSel, ilcDataSubType));

		if(nID == EMR_BUILT_IN_INFO__IMAGE) {
			MsgBox("The 'Image' detail is a built in property that cannot be altered or deleted.");
			return;
		}
		// (c.haag 2008-06-16 11:14) - PLID 30319 - Don't allow deleting of built-in macros
		else if (nID == EMR_BUILT_IN_INFO__TEXT_MACRO) {
			MsgBox("This item is a built-in item that cannot be altered or deleted.");
			return;
		}
		// (j.jones 2010-06-04 17:02) - PLID 39029 - same for generic tables
		else if (iDataSubType == eistGenericTable) {
			MsgBox("This item is a built-in item that cannot be altered or deleted.");
			return;
		}

		// (c.haag 2007-02-01 13:21) - PLID 24423 - Do not allow the user to delete
		// the active Current Medications info item
		if (GetActiveCurrentMedicationsInfoID() == nID) {
			MsgBox("You may not delete the system Current Medications item.");
			return;	
		}
		// (c.haag 2007-04-03 09:01) - PLID 25468 - Do not allow the user to delete
		// the active Allergies info item
		if (GetActiveAllergiesInfoID() == nID) {
			MsgBox("You may not delete the system Allergies item.");
			return;
		}

		if(ReturnsRecords("SELECT ID FROM EMRDetailsT WHERE EMRInfoID IN (SELECT ID FROM EmrInfoT WHERE EmrInfoMasterID = %li)", nMasterID)) {
			MsgBox("This item is already in use by patients.  It cannot be deleted.");
			return;
		}

		// (z.manning 2009-03-17 12:54) - PLID 33242 - Make sure this info item doesn't have any references
		// in any of the SourceDataGroupID fields.
		if(IsDataIDUsedBySpawnedObject(FormatString("SELECT ID FROM EMRDataT WHERE EMRInfoID IN (SELECT ID FROM EmrInfoT WHERE EmrInfoMasterID = %li)",nMasterID))) {
			MessageBox("This item is referenced by spawned objects on patient EMNs.  It cannot be deleted.");
			return;
		}

		// (z.manning 2010-03-17 17:35) - PLID 37228 - Make sure we can't delete child smart stamp tables
		if(ReturnsRecords("SELECT ID FROM EmrInfoT WHERE ChildEmrInfoMasterID = %li", nMasterID)) {
			MessageBox("This item is linked to another item (possibly a Smart Stamp image).  It cannot be deleted.");
			return;
		}

		// (j.gruber 2012-06-11 13:27) - PLID 49700 - check the dashboard
		if (ReturnsRecordsParam("SELECT ID FROM PatientDashboardControlsT WHERE EMRInfoMasterID = {INT}", nMasterID)) {
			MessageBox("This item is in use by the Patient Dashboard.  Please remove the Patient Dashboard Control before deleting this item.");
			return;
		}

		// (a.wilson 2012-07-16 18:10) - PLID 51306 - check emr graphing.
		if (ReturnsRecordsParam("SELECT ID FROM EMRGraphLineDetailsT WHERE EMRInfoMasterID = {INT}", nMasterID)) {
			MsgBox("The following item is assigned in graphing.\r\n"
				"Please remove the item from graphing before deleting.");
			return;
		}

		// (j.gruber 2012-08-31 14:09) - PLID 52285 - OMR Forms
		if (ReturnsRecordsParam("SELECT OMRFormID FROM OMRFormDetailT WHERE EMRDataGroupID IN (SELECT EMRDataGroupID FROM EMRDataT WHERE EMRInfoID IN (SELECT ID FROM EMRInfoT WHERE EMRInfoMasterID = {INT}))", nMasterID)) {
			if (IDNO == MsgBox(MB_YESNO, "This item is associated with at least one OMR Form.\r\n"
				"Are you sure you want to delete it?")) {
				return;
			}
		}

		//TES 12/5/2006 - PLID 23724 - Copies no longer depend on each other in the new structure.
		/*if(ReturnsRecords("SELECT ID FROM EMRInfoT WHERE CopiedFromInfoID = %li", nID)) {
			MsgBox("This item is referenced by a newer version of this item.  It cannot be deleted.");
			return;
		}*/

		// (b.cardillo 2009-07-22 12:07) - PLID 34844 - Protect emr master info items that are referenced 
		// by wellness template criteria from being deleted
		{
			_RecordsetPtr prs = GetWellnessTemplatesThatReferenceEMRInfoMasterItem(GetRemoteData(), nMasterID, FALSE);
			CString strWellTemplateList = GenerateDelimitedListFromRecordsetColumn(prs, "Name", " - ", "\r\n");
			if (!strWellTemplateList.IsEmpty()) {
				// There are some, so warn the user and fail
				CString strMsg;
				strMsg.Format(
					"This item is referenced by the criteria on the following Wellness Templates: "
					"\r\n\r\n%s\r\n\r\n"
					"Please remove this item from all Wellness Criteria before attempting to change its type."
					,
					strWellTemplateList);
				MessageBox(strMsg, NULL, MB_ICONEXCLAMATION|MB_OK);
				return;
			}
		}

		if(ReturnsRecords("SELECT ID FROM EMRTemplateDetailsT WHERE EMRInfoMasterID = %li", nMasterID)) {
			if(IDYES != MsgBox(MB_YESNO, "This item is in use on one or more templates.  If you delete it, it will be removed from those templates.\n"
				"Are you sure you wish to permanently delete this item?")) {
				return;
			}
		}
		else if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to permanently delete this item?")) {
			return;
		}

		CWaitCursor pWait;

		// Get the templates of the item we are deleting
		CArray<long,long> aryTemplateIDs;
		{
			_RecordsetPtr prs = CreateRecordset("SELECT ID FROM EMRTemplateT WHERE "
				"Deleted = 0 AND ID IN (SELECT TemplateID FROM EmrTemplateDetailsT WHERE EmrInfoMasterID = %li) "
				"OR ID IN (SELECT TemplateID FROM EmrTemplateDetailsT WHERE EmrInfoMasterID IN ("
				"SELECT EmrInfoT.EmrInfoMasterID FROM EmrActionsT INNER JOIN EmrInfoT ON EmrActionsT.SourceID = EmrInfoT.ID WHERE SourceType = %i AND DestType = %i AND DestID = %li)) "
				"OR ID IN (SELECT TemplateID FROM EmrTemplateDetailsT WHERE EmrInfoMasterID IN ("
				"SELECT EmrInfoMasterID FROM EmrInfoT WHERE ID IN (SELECT EmrInfoID FROM EmrDataT WHERE EmrDataT.ID IN ("
				"SELECT SourceID FROM EmrActionsT WHERE SourceType = %i AND DestType = %i AND DestID = %li))))",
				nMasterID, eaoEmrItem, eaoEmrItem, nMasterID, eaoEmrDataItem, eaoEmrItem, nMasterID);
			FieldPtr fldTemplateID = prs->GetFields()->GetItem("ID");
			while (!prs->eof) {
				aryTemplateIDs.Add(AdoFldLong(fldTemplateID));
				prs->MoveNext();
			}
		}

		//Are we deleting multiple copies at once?
		_RecordsetPtr rsInfos = CreateRecordset("SELECT ID FROM EmrInfoT WHERE EmrInfoMasterID = %li", nMasterID);
		CArray<long,long> arInfoIDs;
		while(!rsInfos->eof) {
			arInfoIDs.Add(AdoFldLong(rsInfos, "ID"));
			rsInfos->MoveNext();
		}
		
		if(arInfoIDs.GetSize() > 1) {
			if(IDYES != MsgBox(MB_YESNO, "Deleting this item will delete ALL copies of this item.  Are you sure you wish to continue?")) {
				return;
			}
		}

		CWaitCursor pWait2;

		if (FALSE == DeleteEmrInfoMasterItem(nMasterID))
			return;

		for(int i = 0; i < arInfoIDs.GetSize(); i++) {
			RefreshTable(NetUtils::EMRDataT_By_InfoID, arInfoIDs[i]);
			// (j.jones 2006-10-17 09:38) - PLID 19895 - use the tablechecker for the EMRInfo message
			m_EMRInfoChecker.Refresh(arInfoIDs[i]);
		}
		// (j.jones 2014-08-26 09:14) - PLID 63223 - include the master ID
		CClient::RefreshTable(NetUtils::EmrInfoMasterT, nMasterID);
		
		long nCurSel = m_pdlItemList->CurSel;
		// (j.jones 2007-12-11 10:47) - PLID 28321 - safely reselects
		// a nearby row, instead of always selecting the first row
		RemoveItemRow(nCurSel);

		if(m_pdlItemList->GetRowCount() <= 0) {

			// m.carlson 6/8/2004 - PL 12472
			// Instead of blindly requerying, let's intelligently see whether the filter lists need modified
			// or not, Reset one (or both only if necessary), and inform the user of what happened

			long nTemplateID = VarLong(m_pTemplateFilter->GetValue(m_pTemplateFilter->GetCurSel(),0),-1);
			
			// Since we might be resetting filters, let's not give them a message for each filter, but just
			// give one message at the end saying some filters have been reset.
			bool bFlagCategory = false;

			// If the template filter alone would cause the list to be empty
			//TES 1/12/2006 - PLID 1/12/2007 - PLID 23724 - Updated this from clause to be the same as the one in the
				// resources for m_pdlItemList, since that's what CalcInfoFilteredWhereClause() is designed to work with.
			if ((m_pTemplateFilter->GetCurSel() != 0) && IsRecordsetEmpty("SELECT * FROM (SELECT EmrInfoT.ID, Name, DataType, "
				"EmrInfoMasterT.Inactive, InputDate, EmrInfoMasterID FROM EMRInfoT INNER JOIN EmrInfoMasterT ON EmrInfoT.ID "
				"= EmrInfoMasterT.ActiveEmrInfoID) SubQ WHERE %s", CalcInfoFilteredWhereClause(nTemplateID, "ID")))
			{
				// remove it from the list and select <All Categories>
				m_pTemplateFilter->RemoveRow(m_pTemplateFilter->GetCurSel());
				m_pTemplateFilter->CurSel = 0;
				OnSelChosenEmrTemplateSelectList(0);
				MsgBox("Deleting this item has left the filtered item list empty - the template filter has been reset to make items visible again.");
			}

			if(m_pdlItemList->GetRowCount() <= 0) {
				// there are no items left, disable the delete button
				GetDlgItem(IDC_DELETE_INFO_ITEM)->EnableWindow(FALSE);
			}
		}
		else{
			// Since we just deleted an info item that may have been the last one that referenced the 
			// templates it referenced, we need to loop through those templates, and for each one if 
			// no more info items reference it then we need to remove it from the tmeplate filter list.
			for (long i=0; i<aryTemplateIDs.GetSize(); i++) {
				long nTemplateID = aryTemplateIDs.GetAt(i);

				// If there are no more items with the same template as the one we're deleting
				// (this will only be reached if <All Categories> is selected)
				//TES 1/12/2006 - PLID 1/12/2007 - PLID 23724 - Updated this from clause to be the same as the one in the
				// resources for m_pdlItemList, since that's what CalcInfoFilteredWhereClause() is designed to work with.
				/*
				if (!ReturnsRecords("SELECT * FROM (SELECT EmrInfoT.ID, Name, DataType, EmrInfoMasterT.Inactive, InputDate, "
					"EmrInfoMasterID FROM EMRInfoT INNER JOIN EmrInfoMasterT ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID) "
					"SubQ WHERE %s", CalcInfoFilteredWhereClause(nTemplateID, "ID")))
				*/

				// (j.jones 2007-05-21 13:04) - PLID 26061 - The above query was incredibly slow, it's SQL execution
				// did not account for short circuit logic, and despite each step being quick on its own, the combined
				// query took far too long. The below procedure accomplishes the same goal in a much faster manner.
				
				CString strSql;
				strSql.Format("DECLARE @nFound INT "
					"DECLARE @nTemplateID INT "
					"SET @nFound = 0 "
					"SET @nTemplateID = %li "
					""
					//first check to see if a regular detail exists on this template
					" SET @nFound = (SELECT CASE WHEN EXISTS (SELECT ActiveEmrInfoID FROM EmrTemplateDetailsT "
					"	INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID "
					"	WHERE TemplateID = @nTemplateID AND EmrInfoMasterT.Inactive = 0) THEN 1 ELSE 0 END) "
					""
					//next, if none were found, see if a spawned-by-detail-item detail exists on this template
					"IF @nFound <> 1 "
					"BEGIN "
					"SET @nFound = (SELECT CASE WHEN EXISTS (SELECT DestID FROM EmrActionsT WHERE DestType = 3 AND SourceType = 4 "
					"   AND DestID IN (SELECT ID FROM EmrInfoMasterT WHERE Inactive = 0) "
					"	AND SourceID IN "
					"	   (SELECT ID FROM EmrDataT WHERE EmrInfoID IN "
					"		   (SELECT ActiveEmrInfoID FROM EmrTemplateDetailsT "
					"			INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID "
					"		   WHERE TemplateID = @nTemplateID) "
					"	   )) THEN 1 ELSE 0 END) "
					"END "
					""
					//and last, if still nothing found, see if a spawned-by-info-item detail exists on this template
					"IF @nFound <> 1 "
					"BEGIN "
					"SET @nFound = (SELECT CASE WHEN EXISTS (SELECT DestID FROM EmrActionsT WHERE DestType = 3 AND SourceType = 3 "
					"   AND DestID IN (SELECT ID FROM EmrInfoMasterT WHERE Inactive = 0) "
					"	AND SourceID IN "
					"	(SELECT ActiveEmrInfoID FROM EmrTemplateDetailsT "
					"		INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID "
					"		WHERE TemplateID = @nTemplateID AND EmrInfoMasterT.Inactive = 0) "
					"	) THEN 1 ELSE 0 END) "
					"END "
					""
					"SELECT @nFound AS Found", nTemplateID);

				BOOL bFound = FALSE;

				CWaitCursor pWait3;

				_RecordsetPtr rs = CreateRecordsetStd(strSql);
				if(!rs->eof) {
					long nFound = AdoFldLong(rs, "Found",0);
					if(nFound == 1)
						bFound = TRUE;
				}

				//now if no items were found on the template, remove it
				if(!bFound) {

					long nRes = m_pTemplateFilter->FindByColumn(0,nTemplateID,0,VARIANT_FALSE);

					if (nRes != -1) // if it can find the row, remove it
						m_pTemplateFilter->RemoveRow(nRes);
				}
			}
		}
		EnableAppropriateFields();
		
		// (z.manning 2008-06-13 09:32) - PLID 30384 - Update the column widths.
		UpdateColumnWidths();

	}NxCatchAll("Error in CEMRSetupDlg::OnDeleteInfoItem()");
}

void CEMRSetupDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {

		BOOL bItemRequery = FALSE;

		if(m_EMRTemplateChecker.Changed()) {
			m_nTemplateFilterID = -1;
			if(m_pTemplateFilter->CurSel != -1)
				m_nTemplateFilterID = VarLong(m_pTemplateFilter->GetValue(m_pTemplateFilter->CurSel,0),-1);

			m_bAutoFilter = TRUE;
			m_pTemplateFilter->Requery();
		}
		
		if(m_EMRInfoChecker.Changed()) {
			long nInfoID = -1;
			if(m_pdlItemList->CurSel != -1)
				nInfoID = VarLong(m_pdlItemList->GetValue(m_pdlItemList->CurSel,ilcID),-1);

			// (d.thompson 2009-03-03) - PLID 33103 - Remove any filters when requerying
			UnfilterView();

			bItemRequery = TRUE;

			if(nInfoID != -1) {
				m_pdlItemList->SetSelByColumn(ilcID,nInfoID);
			}
		}

		if(!bItemRequery)
			EnableAppropriateFields();
	} NxCatchAll("Error in UpdateView()");
}

void CEMRSetupDlg::OnSetupHeader()
{
	try {
		CEmrHeaderDlg dlg(this);
		dlg.DoModal();
	} NxCatchAll("Error in OnSetupHeader()");
}

void CEMRSetupDlg::OnOfficeVisit()
{
	try {
		COfficeVisitConfigDlg dlg(this);
		dlg.DoModal();
	} NxCatchAll("Error in OnOfficeVisit()");
}

class CEMRSetupDlg_ExtraMergeFields
{
public:
	CString m_strEMRItemFilter;
};

static CString g_strEmrCategoryMergeFieldList;
static CString g_strEmrCategoryEmptyMergeDataList;

static CString g_strEmrItemMergeFieldList;
static CString g_strEmrItemEmptyMergeDataList;

static void Populate_CEMRSetupDlg__ExtraMergeFields_Variables(CEMRSetupDlg_ExtraMergeFields& emf)
{
	GetEmrCategoryMergeFieldList(g_strEmrCategoryMergeFieldList, g_strEmrCategoryEmptyMergeDataList);
	GetEmrItemMergeFieldList(emf.m_strEMRItemFilter, g_strEmrItemMergeFieldList, g_strEmrItemEmptyMergeDataList);
}

CString CALLBACK CEMRSetupDlg__ExtraMergeFields(BOOL bFieldNamesInsteadOfData, const CString &strKeyFieldValue, LPVOID pParam)
{
	try {
		CEMRSetupDlg_ExtraMergeFields *pemf = (CEMRSetupDlg_ExtraMergeFields *)pParam;
		CString strEMRCategoryList, strEMRItemList;
		if (bFieldNamesInsteadOfData) {
			strEMRCategoryList = g_strEmrCategoryMergeFieldList;
			strEMRItemList = g_strEmrItemMergeFieldList;
		} else {
			strEMRCategoryList = g_strEmrCategoryEmptyMergeDataList;
			strEMRItemList = g_strEmrItemEmptyMergeDataList;
		}
		if (strEMRCategoryList.GetLength() && strEMRItemList.GetLength())
			return strEMRCategoryList + "," + strEMRItemList;
		else
			return strEMRCategoryList + strEMRItemList;

	} NxCatchAllCallIgnore({
		return "";
	});
}

void CEMRSetupDlg::OnEditTemplate()
{
	CWaitCursor wc;

	//DRT 3/17/03 - Check for permission first!
	if (!CheckCurrentUserPermissions(bioLWEditTemplate, sptView))
		return;

	if (!GetWPManager()->CheckWordProcessorInstalled()) {
		return;
	}

	CString strEMRFilter;
	if (!GetEMRFilter(strEMRFilter)) {
		return;
	}

	long nMergeFlags = BMS_HIDE_ALL_DATA | BMS_DEFAULT |
		(GetRemotePropertyInt("MergeShowPractice",1,0,"<None>",FALSE) ? 0 : BMS_HIDE_PRACTICE_INFO) |
		(GetRemotePropertyInt("MergeShowPerson",1,0,"<None>",FALSE) ? 0 : BMS_HIDE_PERSON_INFO) |
		(GetRemotePropertyInt("MergeShowDate",1,0,"<None>",FALSE) ? 0 : BMS_HIDE_DATE_INFO) |
		(GetRemotePropertyInt("MergeShowPrescription",0,0,"<None>",FALSE) ? 0 : BMS_HIDE_PRESCRIPTION_INFO) |
		(GetRemotePropertyInt("MergeShowCustom",0,0,"<None>",FALSE) ? 0 : BMS_HIDE_CUSTOM_INFO) |
		(GetRemotePropertyInt("MergeShowInsurance",0,0,"<None>",FALSE) ? 0 : BMS_HIDE_INSURANCE_INFO) |
		(GetRemotePropertyInt("MergeShowBillInfo",0,0,"<None>",FALSE) ? 0 : BMS_HIDE_BILL_INFO) |
		(GetRemotePropertyInt("MergeShowProcedureInfo",0,0,"<None>",FALSE) ? 0 : BMS_HIDE_PROCEDURE_INFO) |
		(GetRemotePropertyInt("MergeShowDoctorInfo",0,0,"<None>",FALSE) ? 0 : BMS_HIDE_DOCTOR_INFO) |
		(GetRemotePropertyInt("MergeShowRespPartyInfo",0,0,"<None>",FALSE) ? 0 : BMS_HIDE_RESP_PARTY_INFO);
		// Notice EMR is absent from here because we always want EMR fields.

	char path[MAX_PATH];
	path[0] = 0;
	CString strInitDir = GetTemplatePath();
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetSafeHwnd();

	// (a.walling 2007-06-14 13:22) - PLID 26342 - Should we support word 2007?
	// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
	static char Filter2007[] = "Microsoft Word Templates (*.dot, *.dotx, *.dotm)\0*.DOT;*.DOTX;*.DOTM\0";
	// Always support Word 2007 templates
	ofn.lpstrFilter = Filter2007;
	
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = path;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrInitialDir = strInitDir.GetBuffer(MAX_PATH);
	strInitDir.ReleaseBuffer();
	ofn.lpstrTitle = "Select a template to edit";
	ofn.Flags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = "dot";

	if (!(::GetOpenFileName(&ofn))) {	
		DWORD dwErr = CommDlgExtendedError();
		return;
	}

	CString strMergeInfoFilePath;

	try {
		// (z.manning 2016-02-12 13:57) - PLID 68230 - Use the base word processor app class
		std::shared_ptr<CGenericWordProcessorApp> pApp = GetWPManager()->GetAppInstance();
		if (nullptr == pApp) return; // (c.haag 2016-06-01 11:48) - NX-100320 - If it's null then it's not supported and an exception was not thrown
		pApp->EnsureValid();

		// Create an empty MergeInfo.nxt
		CEMRSetupDlg_ExtraMergeFields emf;
		long nFlags = nMergeFlags;
		emf.m_strEMRItemFilter = strEMRFilter;
		Populate_CEMRSetupDlg__ExtraMergeFields_Variables(emf);
		strMergeInfoFilePath = CMergeEngine::CreateBlankMergeInfo(nFlags, CEMRSetupDlg__ExtraMergeFields, &emf);
		if (!strMergeInfoFilePath.IsEmpty()) {

			// Open the template
			// (z.manning 2016-02-18 09:18) - PLID 68366 - This now returns a word processor document object
			// (c.haag 2016-04-22 10:40) - NX-100275 - OpenTemplate no longer returns a document. We never did anything with it except throw an exception if it were null
			// anyway, and now OpenTemplate does that for us
			pApp->OpenTemplate(path, strMergeInfoFilePath);

			// We can't delete the merge info text file right now because it is in use, but 
			// it's a temp file so mark it to be deleted after the next reboot
			DeleteFileWhenPossible(strMergeInfoFilePath);
			strMergeInfoFilePath.Empty();
		} else {
			AfxThrowNxException("Could not create blank merge info");
		}
		// (c.haag 2016-02-23) - PLID 68416 - We no longer catch Word-specific exceptions here. Those are now managed deep within the WordProcessor application object
	}NxCatchAll("CLetterWriting::OnEditTemplate");

	if (!strMergeInfoFilePath.IsEmpty()) {
		// This means the file wasn't used and/or it wasn't 
		// marked for deletion at startup, so delete it now
		DeleteFile(strMergeInfoFilePath);
	}	
}

void CEMRSetupDlg::OnManageEmrcollections()
{
	try {
		CEmrCollectionSetupDlg dlg(this);
		dlg.DoModal();

		UpdateView();

	} NxCatchAll("CEMRSetupDlg::OnManageEmrcollections");
}

//DRT 6/15/2007 - PLID 25531 - We now use the more descriptive ERefreshTable instead of just "Table"
void CEMRSetupDlg::RefreshTable(NetUtils::ERefreshTable table, DWORD id)
{
	if (m_bSendTableCheckers)
		CClient::RefreshTable(table, id);
}

void CEMRSetupDlg::EnableAppropriateFields()
{
	if (m_pdlItemList->GetCurSel() == sriNoRow) {
		GetDlgItem(IDC_DELETE_INFO_ITEM)->EnableWindow(FALSE);
	} else {
		GetDlgItem(IDC_DELETE_INFO_ITEM)->EnableWindow(TRUE);
	}

	if (m_pdlItemList->GetRowCount() > 0){
		m_pdlItemList->PutEnabled(TRUE);
		m_pTemplateFilter->PutEnabled(TRUE);
	} else{
		m_pdlItemList->PutEnabled(FALSE);
		m_pTemplateFilter->PutEnabled(FALSE);
	}		
}

void CEMRSetupDlg::OnSelChangedEmrInfoList(long nNewSel) 
{
	try {
		EnableAppropriateFields();
	} NxCatchAll("CEMRSetupDlg::OnSelChangedEmrInfoList");
}

void CEMRSetupDlg::OnDblClickCellEmrInfoList(long nRowIndex, short nColIndex)
{
	try {
		if(nRowIndex != sriNoRow) {
			OpenEmrItemEntryDlg(nRowIndex);
		}
	}NxCatchAll("CEMRSetupDlg::OnDblClickCellEmrInfoList");
}

void CEMRSetupDlg::OnSelChosenEmrTemplateSelectList(long nRow) 
{
	try {
		CWaitCursor cuWait;
		if(nRow == -1)
			m_pTemplateFilter->SetSelByColumn(0,(long)-1);

		long nTemplate = VarLong(m_pTemplateFilter->GetValue(m_pTemplateFilter->GetCurSel(),0),-1);
		
		// m.carlson 6/8/2004 PL 12472
		// If they chose something other than "<All Templates>"
		if (nTemplate != -1)
		{
			// m.carlson 6/8/2004 PL 12472
			// Check if the procedure filter alone would result in no items shown - if so, reset it
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			if (!ReturnsRecordsParam("SELECT ID FROM EMRInfoT WHERE ID IN (SELECT ActiveEMRInfoID AS ID FROM EmrTemplateDetailsT INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID WHERE TemplateID = {INT})",nTemplate))
			{
				MsgBox("No EMR Items meet the current template filter criteria.  The template filter will be reset.");

				nTemplate = -1; m_pTemplateFilter->PutCurSel(0);
			}
		}

		// (d.thompson 2009-03-03) - PLID 33103 - Modified this to not requery ourself, do it all in the Unfilter function.
		UnfilterView();

	}NxCatchAll("Error changing procedure filter.");
}

void CEMRSetupDlg::OnRequeryFinishedEmrTemplateSelectList(short nFlags) 
{
	try {
		IRowSettingsPtr pRow = m_pTemplateFilter->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,_bstr_t(" <All Templates>"));
		m_pTemplateFilter->InsertRow(pRow,0);
		m_pTemplateFilter->SetSelByColumn(0,m_nTemplateFilterID);

		if(m_pTemplateFilter->CurSel == -1)
			m_pTemplateFilter->SetSelByColumn(0,(long)-1);

		if(m_bAutoFilter)
			OnSelChosenEmrTemplateSelectList(m_pTemplateFilter->GetCurSel());
	} NxCatchAll("Error in OnRequeryFinishedEmrTemplateSelectList");
}

CString CEMRSetupDlg::CalcInfoFilteredWhereClause(long nTemplateID, CString strIDName)
{
	CString strWhere = "";

	if(nTemplateID != -1) {
		//All items that are a.) in this template, b.) spawned by an item in this template, c.) spawned by a list item
		//of an item in this template.
		strWhere.Format("%s IN (SELECT ActiveEmrInfoID FROM EmrTemplateDetailsT INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID WHERE TemplateID = %li) "
			"OR %s IN (SELECT DestID FROM EmrActionsT WHERE DestType = %i AND SourceType = %i AND SourceID IN "
			"(SELECT ActiveEmrInfoID FROM EmrTemplateDetailsT INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID  WHERE TemplateID = %li)) "
			"OR %s IN (SELECT DestID FROM EmrActionsT WHERE DestType = %i AND SourceType = %i AND SourceID IN "
			"(SELECT ID FROM EmrDataT WHERE EmrInfoID IN (SELECT ActiveEmrInfoID FROM EmrTemplateDetailsT INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID  WHERE TemplateID = %li)))",
			strIDName, nTemplateID, strIDName, eaoEmrItem, eaoEmrItem, nTemplateID, strIDName, eaoEmrItem, eaoEmrDataItem, nTemplateID);
	}
	// (c.haag 2006-02-27 10:17) - PLID 12763 - Include the inactive checkbox
	//
	if (!IsDlgButtonChecked(IDC_CHECK_SHOWINACTIVEITEMS)) {
		CString strNew;
		if (strWhere.IsEmpty()) {
			strWhere = "1=1";
		}
		strNew.Format("Inactive = 0 AND (%s)", strWhere);
		strWhere = strNew;
	}

	// (j.jones 2010-06-07 10:03) - PLID 39029 - exclude generic tables
	if (strWhere.IsEmpty()) {
		strWhere = "1=1";
	}
	CString strFullWhereClause;
	strFullWhereClause.Format("SubQ.ID <> %li AND SubQ.DataSubType <> %li AND (%s)", EMR_BUILT_IN_INFO__TEXT_MACRO, eistGenericTable, strWhere);

	return strFullWhereClause;
}

// (j.jones 2011-07-05 12:06) - PLID 43603 - renamed to just 'status lists'
void CEMRSetupDlg::OnEditStatusLists()
{
	try {

		// (j.jones 2011-07-05 12:08) - PLID 43603 - now we have a menu of options
		enum {
			miEMNStatus = -10,
			miProblemStatus = -11,		
		};

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, miEMNStatus, "Edit &EMN Status List...");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, miProblemStatus, "Edit &Problem Status List...");

		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_EMR_STATUS_LISTS);
		int nResult = 0;
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, rc.right, rc.top, this, NULL);
		} else {
			CPoint pt;
			GetCursorPos(&pt);
			nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
		}

		switch(nResult) {

			case miEMNStatus: {

				CEMNStatusConfigDlg dlg(this);
				dlg.DoModal();
				break;
			}

			case miProblemStatus: {

				CEMRProblemStatusDlg dlg(this);
				dlg.DoModal();
				break;
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CEMRSetupDlg::OnEditMintTemplate() 
{
	try {
		//CEmrTemplateManagerDlg dlg(this);
		//dlg.DoModal();

		if (m_pdlgTemplateManager) {
			m_pdlgTemplateManager->DoModal();
		} else {
			ASSERT(FALSE);
		}
		UpdateView();
	} NxCatchAll("Error in OnEditMintTemplate");
}

void CEMRSetupDlg::OnRequeryFinishedEmrInfoList(short nFlags) 
{
	if (dlRequeryFinishedCompleted == nFlags) {

		// (z.manning 2008-06-13 09:32) - PLID 30384 - Update the column widths.
		UpdateColumnWidths();

		//
		// (c.haag 2007-01-31 10:23) - PLID 24428 - Color the Current Medications
		// info item blue
		//
		_variant_t value = (short)eistCurrentMedicationsTable;
		long nRow = m_pdlItemList->FindByColumn(ilcDataSubType, value, 0, false);
		if (nRow != -1) {
			IRowSettingsPtr pRow = m_pdlItemList->GetRow(nRow);
			if (NULL != pRow) {
				pRow->ForeColor = RGB(0,0,255);
			}			
		}
		// (c.haag 2007-04-02 12:29) - PLID 25458 - Do the same for Allergies
		value = (short)eistAllergiesTable;
		nRow = m_pdlItemList->FindByColumn(ilcDataSubType, value, 0, false);
		if (nRow != -1) {
			IRowSettingsPtr pRow = m_pdlItemList->GetRow(nRow);
			if (NULL != pRow) {
				pRow->ForeColor = RGB(0,0,255);
			}			
		}

		EnableAppropriateFields();
	}
}

LRESULT CEMRSetupDlg::OnEditEMRTemplate(WPARAM wParam, LPARAM lParam)
{
	try {		
		// (a.walling 2012-03-13 14:13) - PLID 48469 - Pass to mainframe
		GetMainFrame()->SendMessage(NXM_EDIT_EMR_OR_TEMPLATE, wParam, lParam);

		// (a.walling 2012-03-13 14:13) - PLID 48469 - This is modeless above, so no point doing any of this updating immediately now.

		//UpdateView();
		//
		////The items in this template may have changed.
		////DRT 1/3/2005 - PLID 15170 - Fixed a crash that was happening under very rare circumstances.
		////	
		//long nCurSel = m_pTemplateFilter->CurSel;
		//if(nCurSel != sriNoRow) {
		//	if(VarLong(m_pTemplateFilter->GetValue(nCurSel, 0)) == (long)wParam) {
		//		// (d.thompson 2009-03-03) - PLID 33103 - Remove any filters when requerying
		//		UnfilterView();
		//	}
		//}
		//else {
		//	//No selection?  this shouldn't happen, we'll just force a requery anyways.
		//	// (d.thompson 2009-03-03) - PLID 33103 - Remove any filters when requerying
		//	UnfilterView();
		//}

	} NxCatchAll("Error in CEMRSetupDlg::OnEditEMRTemplate");

	return 0;
}

void CEMRSetupDlg::OnRButtonDownEmrInfoList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		m_pdlItemList->PutCurSel(nRow);
		EnableAppropriateFields();

		CMenu mnu;
		mnu.LoadMenu(IDR_NEXEMR_SETUP_MENU);
		CMenu *pmnuSub = mnu.GetSubMenu(0);	//contains our setup popup
		if (pmnuSub) {
			//Hide certain items if we're not on a row
			if(nRow == -1) {
				//If no row, right now we don't have anything to do
				return;
			}
			else {
				//all necessary items are enabled by default

				long nOldInfoID = VarLong(m_pdlItemList->GetValue(nRow, ilcID));
				// (j.jones 2010-07-07 17:25) - PLID 39555 - changed to a byte for accuracy
				BYTE iDataSubType = VarByte(m_pdlItemList->GetValue(m_pdlItemList->CurSel, ilcDataSubType));

				//Watch for special image type, we can't copy this  (the copy box should not pop up)
				// (c.haag 2008-06-16 11:15) - PLID 30319 - Also EMR text macros
				// (j.jones 2010-06-04 17:02) - PLID 39029 - same for generic tables
				if(nOldInfoID == EMR_BUILT_IN_INFO__IMAGE || nOldInfoID == EMR_BUILT_IN_INFO__TEXT_MACRO
					 || iDataSubType == eistGenericTable)
					return;
			}

			// (z.manning, 10/16/2006) - PLID 22222 - Give the option to inactivate the item if it's active
			// and vice versa.
			_variant_t vtInactive = m_pdlItemList->GetValue(nRow,ilcInactive);
			if(vtInactive.vt == VT_BOOL) {
				if(VarBool(vtInactive)) {
					pmnuSub->RemoveMenu(IDC_INACTIVATE_ITEM, MF_BYCOMMAND);
				}
				else {
					pmnuSub->RemoveMenu(IDC_ACTIVATE_ITEM, MF_BYCOMMAND);
				}
			}

			CPoint point;
			GetCursorPos(&point);

			// Show the popup
			pmnuSub->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
		}

	} NxCatchAll("Error in OnRButtonDownEmrInfoList");
}

void CEMRSetupDlg::OnEMRSetupEdit()
{
	try {
		long nCurSel = m_pdlItemList->GetCurSel();
		if(nCurSel != sriNoRow) {
			OpenEmrItemEntryDlg(nCurSel);
		}
	}NxCatchAll("CEMRSetupDlg::OnEMRSetupEdit");
}

//DRT 8/2/2005 - PLID 15796 - Ability to fully copy any EMR detail into a new item.
void CEMRSetupDlg::OnEMRSetupCopy()
{
	try {

		//Get the current ID
		long nCurSel = m_pdlItemList->GetCurSel();
		if(nCurSel == sriNoRow)
			return;

		long nOldInfoID = VarLong(m_pdlItemList->GetValue(nCurSel, ilcID));
		long nOldMasterID = VarLong(m_pdlItemList->GetValue(nCurSel, ilcEmrInfoMasterID));
		// (j.jones 2010-07-07 17:25) - PLID 39555 - changed to a byte for accuracy
		BYTE iDataSubType = VarByte(m_pdlItemList->GetValue(m_pdlItemList->CurSel, ilcDataSubType));

		//Watch for special image type, we can't copy this  (the copy box should not pop up)
		// (c.haag 2008-06-16 11:15) - PLID 30319 - Also EMR text macros
		// (j.jones 2010-06-04 17:02) - PLID 39029 - same for generic tables
		if(nOldInfoID == EMR_BUILT_IN_INFO__IMAGE || nOldInfoID == EMR_BUILT_IN_INFO__TEXT_MACRO
			|| iDataSubType == eistGenericTable)
			return;

		//Copy the item.
		CString strNewName;
		long nNewMasterID, nNewInfoID;
		if(CopyEmrInfoItem(nOldMasterID, VarString(m_pdlItemList->GetValue(nCurSel, ilcName), ""), FALSE, &nNewMasterID, &nNewInfoID, &strNewName)) {

			//successfully copied

			CString strType = "";
			_RecordsetPtr rs = CreateRecordset("SELECT "
				"CASE WHEN DataType = 1 THEN 'Text' WHEN DataType = 2 THEN 'Single-Select List' "
				"WHEN DataType = 3 THEN 'Multi-Select List' WHEN DataType = 4 THEN 'Image' "
				"WHEN DataType = 5 THEN 'Slider' WHEN DataType = 6 THEN 'Narrative' "
				"WHEN DataType = 7 THEN 'Table' END AS Type "
				"FROM EMRInfoT  "
				"WHERE ID = %li", nNewInfoID);
			if(!rs->eof) {
				strType = AdoFldString(rs, "Type","");
			}
			rs->Close();
			
			//And finally, add a row to our list of details - ONLY if we successfully saved
			//	the transaction
			IRowSettingsPtr pRow = m_pdlItemList->GetRow(sriGetNewRow);
			pRow->PutValue(ilcID, nNewInfoID);
			pRow->PutValue(ilcName, (LPCTSTR)strNewName);
			pRow->PutValue(ilcDataType, (LPCTSTR)strType);
			pRow->PutValue(ilcInactive, VARIANT_FALSE);
			// (r.goldschmidt 2014-05-19 17:43) - PLID 56233 - fill in ilcInputDate column
			pRow->PutValue(ilcInputDate, variant_t(COleDateTime::GetCurrentTime(), VT_DATE));
			pRow->PutValue(ilcEmrInfoMasterID, nNewMasterID);
			// (j.jones 2010-07-07 17:27) - PLID 39555 - fill the ilcDataSubType column!
			pRow->PutValue(ilcDataSubType, (BYTE)iDataSubType);
			long nRow = m_pdlItemList->AddRow(pRow);
			// And select it
			m_pdlItemList->PutCurSel(nRow);

			UpdateView();
		}

	} NxCatchAll("Error in OnEMRSetupCopy");
}

LRESULT CEMRSetupDlg::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
		// (c.haag 2006-03-01 13:31) -  PLID 19208 - We now pass table checker
		// messages to the template list
		// (j.jones 2014-08-08 16:59) - PLID 63182 - this function had nothing in it, so I removed its existence
		/*
		if (m_pdlgTemplateManager && IsWindow(m_pdlgTemplateManager->GetSafeHwnd())) {
			m_pdlgTemplateManager->OnTableChanged(wParam, lParam);
		}
		*/

		switch(wParam) {

		case NetUtils::EMRTemplateT:
			// (j.jones 2014-08-12 10:06) - PLID 63189 - we no longer immediately refresh on EMRTemplateT
			// tablecheckers, instead we only refresh immediately if our session made the change, which
			// will have already called UpdateView()
			break;
		
		case NetUtils::EMRInfoT:
			if (lParam == -1) {
				UpdateView();
			}
			else {
				// (a.walling 2007-03-12 15:24) - PLID 19884 - Only update one item when necessary
				// we are passed an ID that may not be in the list if it had to be copied when edited.
				// well, a recordset for one item still has less cost than loading everything.
				_RecordsetPtr prs = CreateParamRecordset(
					"SELECT ActiveEMRInfoT.ID, ActiveEMRInfoT.Name, ActiveEMRInfoT.DataType, ActiveEMRInfoMasterT.Inactive, ActiveEMRInfoT.InputDate, ActiveEMRInfoMasterT.ID AS InfoMasterID "
					"FROM EMRInfoT "
					"LEFT JOIN EMRInfoMasterT ON EMRInfoT.EMRInfoMasterID = EMRInfoMasterT.ID "
					"LEFT JOIN EMRInfoT ActiveEMRInfoT ON EMRInfoMasterT.ActiveEMRInfoID = ActiveEMRInfoT.ID "
					"LEFT JOIN EMRInfoMasterT ActiveEMRInfoMasterT ON ActiveEMRInfoT.EMRInfoMasterID = ActiveEMRInfoMasterT.ID "
					"WHERE EMRInfoT.ID = {INT}", lParam);

				if (prs->eof) {
					UpdateView();
				} else {
					long nInfoMasterID = AdoFldLong(prs, "InfoMasterID");
					long nRow = m_pdlItemList->FindByColumn(ilcEmrInfoMasterID, nInfoMasterID, 0, FALSE);

					if (nRow != sriNoRow) {
						BOOL bInactive = AdoFldBool(prs, "Inactive");

						if (bInactive && !IsDlgButtonChecked(IDC_CHECK_SHOWINACTIVEITEMS)) {
							// the item is now inactive, but they do not want to see inactive items.
							// so remove it from here

							// (j.jones 2007-12-11 10:47) - PLID 28321 - safely reselects
							// a nearby row, instead of always selecting no row
							RemoveItemRow(nRow);
							return 0;
						}

						m_pdlItemList->PutValue(nRow, ilcName, prs->Fields->Item["Name"]->Value);
						m_pdlItemList->PutValue(nRow, ilcDataType, (_bstr_t)GetDataTypeName((EmrInfoType)AdoFldByte(prs, "DataType")));
						m_pdlItemList->PutValue(nRow, ilcInactive, (bInactive ? VARIANT_TRUE : VARIANT_FALSE));
						m_pdlItemList->PutValue(nRow, ilcID, prs->Fields->Item["ID"]->Value);
						if (prs->Fields->Item["InputDate"]->Value.vt == VT_DATE)
							m_pdlItemList->PutValue(nRow, ilcInputDate, (_bstr_t)FormatDateTimeForInterface(prs->Fields->Item["InputDate"]->Value, DTF_STRIP_SECONDS));
						else
							m_pdlItemList->PutValue(nRow, ilcInputDate, (_bstr_t)"");

						m_pdlItemList->PutCurSel(nRow);
					} else {
						UpdateView();
					}
				}
			}
			break;
		}
	} NxCatchAll(__FUNCTION__);

	return 0;
}

void CEMRSetupDlg::OnShowInactiveItems()
{
	try {
		long nTemplateID = VarLong(m_pTemplateFilter->GetValue(m_pTemplateFilter->GetCurSel(),0),-1);
		CString strWhere = CalcInfoFilteredWhereClause(nTemplateID, "SubQ.ID");
		if(m_pdlItemList->WhereClause != _bstr_t(strWhere)) {
			m_pdlItemList->WhereClause = _bstr_t(strWhere);
			// (d.thompson 2009-03-03) - PLID 33103 - Remove any filters when requerying
			UnfilterView();
		}
	} NxCatchAll("Error in OnShowInactiveItems()");
}

void CEMRSetupDlg::OpenEmrItemEntryDlg(long nRowIndex)
{
	// Give the user the "item entry" dialog
	CWaitCursor wc;			
	long nMasterID = VarLong(m_pdlItemList->GetValue(nRowIndex, ilcEmrInfoMasterID));
	CEmrItemEntryDlg dlg(this);
	if (dlg.OpenWithMasterID(nMasterID) == IDOK) {
		// We may have to update the name of the item in our list
		long nRow = m_pdlItemList->FindByColumn(ilcEmrInfoMasterID, dlg.GetInfoMasterID(), nRowIndex, VARIANT_FALSE);
		//TES 9/27/2006 - PLID 22742 - Also check for sriNoRowYet_WillFireEvent; that means the list is requerying, so we 
		// don't need to set the value, since the requery will do that for us.
		if (nRow != sriNoRow && nRow != sriNoRowYet_WillFireEvent) {
			m_pdlItemList->PutValue(nRow, ilcName, (LPCTSTR)dlg.GetName());
		}
	}

	// (a.walling 2007-03-08 14:59) - PLID 19884 - Remove this unnecessary UpdateView().
	//UpdateView();

	// And make sure the buttons are enabled appropriately
	// (j.jones 2006-10-17 09:38) - PLID 19895 - this is called inside UpdateView
	// (a.walling 2007-03-12 09:40) - PLID 19884 - Since we do not call UpdateView we'll call this manually now.
	EnableAppropriateFields();
}

void CEMRSetupDlg::OnInactivateItem()
{
	// (z.manning, 10/16/2006) - PLID 22222 - Inactivate the selected item.
	try {

		long nCurSel = m_pdlItemList->CurSel;
		if(nCurSel == sriNoRow) {
			return;
		}

		// (c.haag 2007-02-01 13:21) - PLID 24423 - Do not allow the user to inactivate
		// the active Current Medications info item
		if (GetActiveCurrentMedicationsInfoID() == VarLong(m_pdlItemList->GetValue(nCurSel, ilcID))) {
			MsgBox("You may not inactivate the system Current Medications item.");
			return;	
		}
		// (c.haag 2007-04-03 09:01) - PLID 25468 - Do not allow the user to inactivate
		// the active Allergies item
		if (GetActiveAllergiesInfoID() == VarLong(m_pdlItemList->GetValue(nCurSel, ilcID))) {
			MsgBox("You may not inactivate the system Allergies item.");
			return;
		}

		long nInfoMasterID = VarLong(m_pdlItemList->GetValue(nCurSel, ilcEmrInfoMasterID), -1);
		if(nInfoMasterID != -1) {
			if(InactivateEmrInfoMasterItem(nInfoMasterID)) {
				m_pdlItemList->PutValue(nCurSel, ilcInactive, COleVariant(VARIANT_TRUE,VT_BOOL));
				if(!IsDlgButtonChecked(IDC_CHECK_SHOWINACTIVEITEMS)) {
					// (j.jones 2007-12-11 10:47) - PLID 28321 - safely reselects
					// a nearby row, instead of always selecting no row
					RemoveItemRow(nCurSel);
				}
			}
		}
	
	}NxCatchAll("CEMRSetupDlg::OnInactivateItem");
}

void CEMRSetupDlg::OnActivateItem()
{
	// (z.manning, 10/16/2006) - PLID 22222 - Activate the selected item.
	try {

		long nCurSel = m_pdlItemList->CurSel;
		if(nCurSel == sriNoRow) {
			return;
		}

		long nInfoMasterID = VarLong(m_pdlItemList->GetValue(nCurSel, ilcEmrInfoMasterID), -1);
		if(nInfoMasterID != -1) {
			if(ActivateEmrInfoMasterItem(nInfoMasterID)) {
				m_pdlItemList->PutValue(nCurSel, ilcInactive, COleVariant(VARIANT_FALSE,VT_BOOL));
			}
		}
	
	}NxCatchAll("CEMRSetupDlg::OnActivateItem");
}

void CEMRSetupDlg::OnEditLinkedItems() 
{
	try {
		CEmrItemLinkingDlg dlg(this);
		dlg.DoModal();
	} NxCatchAll("Error in OnEditLinkedItems");
}

// (j.jones 2007-08-16 08:36) - PLID 27054 - added Visit Types
void CEMRSetupDlg::OnEditEmrVisitTypes()
{
	try {

		CEMRVisitTypesDlg dlg(this);
		dlg.DoModal();

	} NxCatchAll("Error in CEMRSetupDlg::OnEditEmrVisitTypes");
}

// (j.jones 2007-08-16 10:48) - PLID 27055 - added E/M Checklists
void CEMRSetupDlg::OnEditEmrEmChecklists()
{
	try {

		// (j.jones 2007-08-29 10:43) - PLID 27135 - added permissions

		//check their read permission and check their write permission, but only
		//prompt for the password once

		BOOL bPasswordKnown = FALSE;

		if(!CheckCurrentUserPermissions(bioAdminEMChecklist, sptRead, FALSE, 0, TRUE, TRUE)) {
			//no read permissions
			MsgBox(MB_ICONEXCLAMATION|MB_OK, "You do not have permission to view the E/M Checklists. Please contact your office manager for assistance.");
			return;
		}
		else if(!CheckCurrentUserPermissions(bioAdminEMChecklist, sptRead, FALSE, 0, TRUE, FALSE)) {
			//if the first statement is true, and this is false, then the user must enter their password
			if (!bPasswordKnown && !CheckCurrentUserPassword("E/M Checklist Setup")) {
				return;
			} else {
				bPasswordKnown = TRUE;
			}
		}

		CEMREMChecklistSetupDlg dlg(this);

		//if we get here, they have read permissions, but do they have write permissions?

		if(!CheckCurrentUserPermissions(bioAdminEMChecklist, sptWrite, FALSE, 0, TRUE, TRUE)) {
			//no write permissions
			MsgBox(MB_ICONEXCLAMATION|MB_OK, "You do not have permission to edit the E/M Checklists. The checklists will be read only.");
			
			dlg.m_bReadOnly = TRUE;
		}
		else if(!CheckCurrentUserPermissions(bioAdminEMChecklist, sptWrite, FALSE, 0, TRUE, FALSE)) {
			//if the first statement is true, and this is false, then the user must enter their password
			if (!bPasswordKnown && !CheckCurrentUserPassword("E/M Checklist Setup")) {
				
				dlg.m_bReadOnly = TRUE;

			} else {
				bPasswordKnown = TRUE;
			}
		}

		//now open the dialog		
		dlg.DoModal();

	} NxCatchAll("Error in CEMRSetupDlg::OnEditEmrEmChecklists");
}

// (j.jones 2007-12-11 10:44) - PLID 28321 - removes the given
// selection, and tries to reselect a nearby row
void CEMRSetupDlg::RemoveItemRow(long nCurSel)
{
	try {

		//this function assumes nCurSel is within the bounds
		//of the datalist - it is the caller's responsibility
		//that this be the case
		
		//remove the row
		m_pdlItemList->RemoveRow(nCurSel);

		//don't select the first row, instead see if the
		//next row is valid, otherwise the previous row
		if(m_pdlItemList->GetRowCount() > 0) {
			if(m_pdlItemList->GetRowCount() <= nCurSel) {
				//our selection is outside the upper bounds, select the last row
				nCurSel = m_pdlItemList->GetRowCount() - 1;				
			}			
			if(nCurSel < 0) {
				nCurSel = 0;
			}

			//now re-select the row
			m_pdlItemList->CurSel = nCurSel;
		}
		
		// (z.manning 2008-06-13 09:32) - PLID 30384 - Update the column widths.
		UpdateColumnWidths();

	}NxCatchAll("Error in CEMRSetupDlg::RemoveItemRow");
}

void CEMRSetupDlg::UpdateColumnWidths()
{
	// (z.manning 2008-06-13 09:16) - PLID 30384 - If the item name column is wider than it needs to
	// be then let's make it smaller so that the other columns are closer making it easier to read.
	IColumnSettingsPtr pItemNameCol = m_pdlItemList->GetColumn(ilcName);
	long nNameColWidht = pItemNameCol->GetStoredWidth();
	long nIdealItemNameColWidth = m_pdlItemList->CalcColumnWidthFromData(ilcName, VARIANT_TRUE, VARIANT_FALSE);
	if(nIdealItemNameColWidth < nNameColWidht) {
		// (z.manning 2008-06-13 09:54) - PLID 30384 - The ideal column width is less than the current column
		// with, so go ahead and resize it.
		pItemNameCol->PutStoredWidth(nIdealItemNameColWidth);
	}
	else {
		int nOtherColumnsTotalWidth = 0;
		for(short nCol = 0; nCol < m_pdlItemList->GetColumnCount(); nCol++) {
			if(nCol != ilcName) {
				nOtherColumnsTotalWidth += m_pdlItemList->GetColumn(nCol)->GetStoredWidth();
			}
		}

		CWnd *pwndItemList = GetDlgItem(IDC_EMR_INFO_LIST);
		CRect rcItemList;
		pwndItemList->GetClientRect(rcItemList);
		int nMaxColWidth = rcItemList.Width() - nOtherColumnsTotalWidth - 30;
		if(nIdealItemNameColWidth < nMaxColWidth) {
			// (z.manning 2008-06-13 09:55) - PLID 30384 - The ideal column with is less than the max
			// width before we'd need a horizontal scrollbar, so use that.
			pItemNameCol->PutStoredWidth(nIdealItemNameColWidth);
		}
		else {
			// (z.manning 2008-06-13 09:55) - Otherwise, just use the max width and cut off the item
			// name rather than having a scrollbar.
			pItemNameCol->PutStoredWidth(nMaxColWidth);
		}
	}
}

// (z.manning 2008-10-22 15:40) - PLID 21082 - Added ability to edit signatures from EMR tab
void CEMRSetupDlg::OnBnClickedEditEmrSignature()
{
	try
	{
		CSignatureDlg dlgSignature(this);
		dlgSignature.m_bSetupOnly = TRUE;
		// (z.manning 2008-12-09 09:07) - PLID 32260 - Added a preference for checking for password when loading signature.
		dlgSignature.m_bCheckPasswordOnLoad = (GetRemotePropertyInt("SignatureCheckPasswordEMR", 1, 0, GetCurrentUserName()) == 1);
		dlgSignature.DoModal();

	}NxCatchAll("CEMRSetupDlg::OnBnClickedEditEmrSignature");
}

// (d.thompson 2009-03-03) - PLID 33103
void CEMRSetupDlg::OnFilterEMRDetails()
{
	try {

		if(m_bFiltering) {
			//If we are already filtering, then we want to turn it off.
			UnfilterView();
		}
		else {
			//Otherwise, start filtering
			if (FilterDatalist(m_pdlItemList, 1, 0)) {
				//Set the filter button's icon
				m_btnFilterDetails.SetIcon(IDI_FILTERDN);
				m_btnFilterDetails.RedrawWindow();
				m_bFiltering = true;
			}
		}

	} NxCatchAll("OnFilterEMRDetails");

}

void CEMRSetupDlg::UnfilterView()
{
	if(m_bFiltering) {
		for (short i=0; i < m_pdlItemList->ColumnCount; i++) {
			m_pdlItemList->GetColumn(i)->PutBackColor(RGB(255,255,255));
		}
		m_btnFilterDetails.SetIcon(IDI_FILTER);
		m_btnFilterDetails.RedrawWindow();

		m_bFiltering = false;
	}

	//Now requery the list with the proper filters

	//We need the current filtered template ID
	long nTemplateID = -1;
	long nRow = m_pTemplateFilter->GetCurSel();
	if(nRow != -1) {
		nTemplateID = VarLong(m_pTemplateFilter->GetValue(nRow, 0), -1);
	}

	//Refresh the screen to whatever is supposed to be there
	CString strWhere = CalcInfoFilteredWhereClause(nTemplateID, "SubQ.ID");
	m_pdlItemList->WhereClause = _bstr_t(strWhere);
	m_pdlItemList->Requery();
}

void CEMRSetupDlg::OnEditWellnessTemplates()
{
	try {
		//TES 5/19/2009 - PLID 34302 - Just pop up the dialog
		// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
		CInterventionTemplatesDlg dlg(this);
		dlg.OpenWellnessConfiguration();
	}NxCatchAll("Error in CEMRSetupDlg::OnEditWellnessTemplates()");
}

// (j.jones 2010-02-10 17:01) - PLID 37224 - added ability to edit image stamps from Admin.
void CEMRSetupDlg::OnBtnEditImageStamps()
{
	try {

		// (a.walling 2012-03-12 10:06) - PLID 46648 - Dialogs must set a parent!
		GetMainFrame()->EditEMRImageStamps(this);
		//we don't care what the return value is
		
	}NxCatchAll(__FUNCTION__);
}

void CEMRSetupDlg::OnBnClickedEmrProviderConfig()
{
	try
	{
		// (z.manning 2011-01-31 10:01) - PLID 42334 - Added EMR provider configuration dialog
		CEmrProviderConfigDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-07-05 15:11) - PLID 44421
void CEMRSetupDlg::OnBnClickedEmrCodingSetupButton()
{
	try
	{
		CEmrCodingGroupEditDlg dlg(this);
		dlg.DoModal();
	}
	NxCatchAll(__FUNCTION__);
}