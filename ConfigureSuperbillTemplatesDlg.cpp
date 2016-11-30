// ConfigureSuperbillTemplatesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigureSuperbillTemplatesDlg.h"
#include "ConfigureSuperbillAddFileDlg.h"
#include "ConfigureSuperbillModifyGroupsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;
using namespace ADODB;

//DRT 6/6/2008 - PLID 30306 - Created.
/////////////////////////////////////////////////////////////////////////////
// CConfigureSuperbillTemplatesDlg dialog

//
//Enums for datalists
enum eTypeColumns {
	etcGroupID = 0,
	etcTypeText,
	etcPathText,
};

enum eResourceColumns {
	ercGroupID = 0,
	ercResourceText,
	ercPathText,
};

enum ePickListColumns {
	eplcID = 0,
	eplcTemplateID,
	eplcFilename,
};

CConfigureSuperbillTemplatesDlg::CConfigureSuperbillTemplatesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigureSuperbillTemplatesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigureSuperbillTemplatesDlg)
	//}}AFX_DATA_INIT
}


void CConfigureSuperbillTemplatesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigureSuperbillTemplatesDlg)
	DDX_Control(pDX, IDC_SUPERBILL_HEADER_TEXT, m_nxstaticHeaderText);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_SUPERBILL_REMOVE_PICKLIST, m_btnRemovePickList);
	DDX_Control(pDX, IDC_SUPERBILL_REMOVE_RESOURCE, m_btnRemoveResource);
	DDX_Control(pDX, IDC_SUPERBILL_REMOVE_TYPE, m_btnRemoveType);
	DDX_Control(pDX, IDC_SUPERBILL_MODIFY_RESOURCE, m_btnModifyResource);
	DDX_Control(pDX, IDC_SUPERBILL_MODIFY_TYPE, m_btnModifyType);
	DDX_Control(pDX, IDC_SUPERBILL_ADD_TYPE, m_btnAddType);
	DDX_Control(pDX, IDC_SUPERBILL_ADD_PICKLIST, m_btnAddPickList);
	DDX_Control(pDX, IDC_SUPERBILL_ADD_RESOURCE, m_btnAddResource);
	DDX_Control(pDX, IDC_SUPERBILL_BROWSE, m_btnBrowse);
	DDX_Control(pDX, IDC_SUPERBILL_GLOBAL, m_btnGlobal);
	DDX_Control(pDX, IDC_SUPERBILL_BY_PICKLIST, m_btnByPickList);
	DDX_Control(pDX, IDC_SUPERBILL_BY_RESOURCE, m_btnByResource);
	DDX_Control(pDX, IDC_SUPERBILL_BY_TYPE, m_btnByType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigureSuperbillTemplatesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConfigureSuperbillTemplatesDlg)
	ON_BN_CLICKED(IDC_SUPERBILL_ADD_TYPE, OnSuperbillAddType)
	ON_BN_CLICKED(IDC_SUPERBILL_ADD_RESOURCE, OnSuperbillAddResource)
	ON_BN_CLICKED(IDC_SUPERBILL_ADD_PICKLIST, OnSuperbillAddPicklist)
	ON_BN_CLICKED(IDC_SUPERBILL_REMOVE_TYPE, OnSuperbillRemoveType)
	ON_BN_CLICKED(IDC_SUPERBILL_REMOVE_RESOURCE, OnSuperbillRemoveResource)
	ON_BN_CLICKED(IDC_SUPERBILL_REMOVE_PICKLIST, OnSuperbillRemovePicklist)
	ON_BN_CLICKED(IDC_SUPERBILL_MODIFY_TYPE, OnSuperbillModifyType)
	ON_BN_CLICKED(IDC_SUPERBILL_MODIFY_RESOURCE, OnSuperbillModifyResource)
	ON_BN_CLICKED(IDC_GLOBAL_TEMPLATE_BROWSE, OnGlobalTemplateBrowse)
	ON_BN_CLICKED(IDC_SUPERBILL_BROWSE, OnSuperbillBrowse)
	ON_BN_CLICKED(IDC_SUPERBILL_GLOBAL, OnSuperbillGlobal)
	ON_BN_CLICKED(IDC_SUPERBILL_BY_TYPE, OnSuperbillByType)
	ON_BN_CLICKED(IDC_SUPERBILL_BY_RESOURCE, OnSuperbillByResource)
	ON_BN_CLICKED(IDC_SUPERBILL_BY_PICKLIST, OnSuperbillByPicklist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigureSuperbillTemplatesDlg message handlers

BOOL CConfigureSuperbillTemplatesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		//Batch ConfigRT number settings
		g_propManager.CachePropertiesInBulk("ConfigureSuperbillTemplates", propNumber, 
			"Username = '<None>' AND ("
			"Name = 'SuperbillConfig_Types' OR "
			"Name = 'SuperbillConfig_Resources' OR "
			"Name = 'SuperbillConfig_PickList' OR "
			"Name = 'SuperbillConfig_UseGlobal') ");

		//Bind controls
		m_pPickList = BindNxDataList2Ctrl(IDC_SUPERBILL_PICK_LIST, true);
		m_pTypeList = BindNxDataList2Ctrl(IDC_SUPERBILL_TYPE_LIST, false);
		m_pResourceList = BindNxDataList2Ctrl(IDC_SUPERBILL_RESOURCE_LIST, false);

		//Configure the button displays
		m_btnRemovePickList.AutoSet(NXB_DELETE);
		m_btnRemoveResource.AutoSet(NXB_DELETE);
		m_btnRemoveType.AutoSet(NXB_DELETE);
		m_btnModifyResource.AutoSet(NXB_MODIFY);
		m_btnModifyType.AutoSet(NXB_MODIFY);
		m_btnAddType.AutoSet(NXB_NEW);
		m_btnAddPickList.AutoSet(NXB_NEW);
		m_btnAddResource.AutoSet(NXB_NEW);
		m_btnClose.AutoSet(NXB_CLOSE);


		//Load the data
		LoadFromData();

		//Ensure things are enabled/disabled appropriately
		EnsureInterface();

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//Loads from data and fills the interface.  The eltLoadInfo parameter is used to determine what types
//	of data need to be loaded.  I have added this because I need to refresh the type and resource data
//	if they change, and there is not a simple way to do it with the info known at the time, so it needs
//	reloaded from data.  This is designed as a sort of "requery" attempt, while maintaining the ability 
//	to use parameter loading queries and keep it all in 1 function.
//eLoadTypes:
//	eltGeneral - All the checkboxes, radio buttons, and the global template.
//	eltTypes - Just the data for the appointment types and their paths.  Basically a requery of the type datalist.
//	eltResources - Just the data for the resources and their paths.  Basically a requery of the resource datalist.
//The lists are not cleared before load, if you want them cleared, you must do it yourself.
void CConfigureSuperbillTemplatesDlg::LoadFromData(eLoadTypes eltLoadInfo /*= eltAll*/)
{
	if(eltLoadInfo & eltGeneral) {
		//First, the easy stuff.  Pull the state of each of the configuration buttons from ConfigRT.  These all
		//	default to off because there is an existing preference that has the name of the default superbill, 
		//	which is the only one that should be turned on by default.
		CheckDlgButton(IDC_SUPERBILL_BY_TYPE, GetRemotePropertyInt("SuperbillConfig_Types", 0, 0, "<None>", true));
		CheckDlgButton(IDC_SUPERBILL_BY_RESOURCE, GetRemotePropertyInt("SuperbillConfig_Resources", 0, 0, "<None>", true));
		CheckDlgButton(IDC_SUPERBILL_BY_PICKLIST, GetRemotePropertyInt("SuperbillConfig_PickList", 0, 0, "<None>", true));
		//Default the use of the global to on
		UINT nGlobal = GetRemotePropertyInt("SuperbillConfig_UseGlobal", 1, 0, "<None>", true);

		CheckDlgButton(IDC_SUPERBILL_GLOBAL, nGlobal);
		CheckDlgButton(IDC_SUPERBILL_BROWSE, !nGlobal);


		//Next, the global template is also in ConfigRT.  This is the old legacy value.
		CString strDefTemplate = GetPropertyText("DefaultSuperbillFilename", "", 0, false);
		SetDlgItemText(IDC_GLOBAL_TEMPLATE_PATH, strDefTemplate);
	}

	//Everything else is stored in data.  We'll load everything in 1 trip to the database.
	//1)  The pick list was loaded with the datalist requery at the beginning.

	CString strLoadSql;
	//2)  Load the Type data.  There are 3 queries here, and we will select them all in order.
	strLoadSql += 
		"SELECT ID FROM SuperbillTemplateTypeGroupsT ORDER BY ID;\r\n"
		"SELECT GroupID, TypeID, AptTypeT.Name "
			"FROM SuperbillTemplateTypeT INNER JOIN AptTypeT ON SuperbillTemplateTypeT.TypeID = AptTypeT.ID ORDER BY GroupID, AptTypeT.Name;\r\n"
		"SELECT GroupID, TemplateID, MergeTemplatesT.Path "
			"FROM SuperbillTemplateTypePathsT INNER JOIN MergeTemplatesT ON SuperbillTemplateTypePathsT.TemplateID = MergeTemplatesT.ID ORDER BY GroupID;\r\n";

	//3)  Get the Resource data.  There are 3 queries here, we'll piece everything together in code
	strLoadSql += 
		"SELECT ID FROM SuperbillTemplateResourceGroupsT ORDER BY ID;\r\n"
		"SELECT GroupID, ResourceID, ResourceT.Item "
			"FROM SuperbillTemplateResourceT INNER JOIN ResourceT ON SuperbillTemplateResourceT.ResourceID = ResourceT.ID ORDER BY GroupID, ResourceT.Item;\r\n"
		"SELECT GroupID, TemplateID, MergeTemplatesT.Path "
			"FROM SuperbillTemplateResourcePathsT INNER JOIN MergeTemplatesT ON SuperbillTemplateResourcePathsT.TemplateID = MergeTemplatesT.ID ORDER BY GroupID;\r\n";


	//During the loading process, we're just going to use the datalist itself to load data into, no intermediate structures.
	//	So to start, disable the redraw on the datalist so it looks cleaner
	m_pTypeList->SetRedraw(VARIANT_FALSE);

	_RecordsetPtr prsLoad = CreateParamRecordset(strLoadSql);

	//First, the GroupIDs.  We'll create a datalist row for each.
	if(eltLoadInfo & eltTypes) {
		while(!prsLoad->eof) {
			long nID = AdoFldLong(prsLoad, "ID");

			IRowSettingsPtr pRow = m_pTypeList->GetNewRow();
			pRow->PutValue(etcGroupID, (long)nID);
			pRow->PutValue(etcTypeText, _bstr_t(""));
			pRow->PutValue(etcPathText, _bstr_t(""));
			m_pTypeList->AddRowSorted(pRow, NULL);

			prsLoad->MoveNext();
		}
	}

	//Second, the Types
	prsLoad = prsLoad->NextRecordset(NULL);
	if(eltLoadInfo & eltTypes) {
		while(!prsLoad->eof) {
			long nGroupID = AdoFldLong(prsLoad, "GroupID");
			CString strName = AdoFldString(prsLoad, "Name");

			//Find the right row
			IRowSettingsPtr pRow = m_pTypeList->FindByColumn(etcGroupID, (long)nGroupID, NULL, VARIANT_FALSE);
			if(pRow == NULL) {
				//This shouldn't be possible with our table constraints -- you've got a type that's not tied to a group.
				AfxThrowNxException("Invalid superbill configuration group %li while loading types.", nGroupID);
				return;
			}

			AddTextToCell(pRow, etcTypeText, strName);

			prsLoad->MoveNext();
		}
	}

	//Third, the Paths
	prsLoad = prsLoad->NextRecordset(NULL);
	if(eltLoadInfo & eltTypes) {
		while(!prsLoad->eof) {
			long nGroupID = AdoFldLong(prsLoad, "GroupID");
			CString strPath = AdoFldString(prsLoad, "Path");

			//Since these are already saved to data, we do not need to ensure their status in the MergeTemplatesT table.
			//We do, however, need to remove the preceeding '\Templates\Forms\' text.
			strPath = strPath.Mid(17);

			//Find the right row
			IRowSettingsPtr pRow = m_pTypeList->FindByColumn(etcGroupID, (long)nGroupID, NULL, VARIANT_FALSE);
			if(pRow == NULL) {
				//This shouldn't be possible with our table constraints -- you've got a path that's not tied to a group.
				AfxThrowNxException("Invalid superbill configuration group %li while loading type paths.", nGroupID);
				return;
			}

			AddTextToCell(pRow, etcPathText, strPath);

			prsLoad->MoveNext();
		}
	}

	//We're done loading the types, let the datalist draw again
	m_pTypeList->SetRedraw(VARIANT_TRUE);

	//Now do the same for resources
	m_pResourceList->SetRedraw(VARIANT_FALSE);

	//Fourth, the GroupIDs.  We'll create a datalist row for each.
	prsLoad = prsLoad->NextRecordset(NULL);
	if(eltLoadInfo & eltResources) {
		while(!prsLoad->eof) {
			long nID = AdoFldLong(prsLoad, "ID");

			IRowSettingsPtr pRow = m_pResourceList->GetNewRow();
			pRow->PutValue(ercGroupID, (long)nID);
			pRow->PutValue(ercResourceText, _bstr_t(""));
			pRow->PutValue(ercPathText, _bstr_t(""));
			m_pResourceList->AddRowSorted(pRow, NULL);

			prsLoad->MoveNext();
		}
	}

	//Fifth, the Resources
	prsLoad = prsLoad->NextRecordset(NULL);
	if(eltLoadInfo & eltResources) {
		while(!prsLoad->eof) {
			long nGroupID = AdoFldLong(prsLoad, "GroupID");
			CString strName = AdoFldString(prsLoad, "Item");

			//Find the right row
			IRowSettingsPtr pRow = m_pResourceList->FindByColumn(ercGroupID, (long)nGroupID, NULL, VARIANT_FALSE);
			if(pRow == NULL) {
				//This shouldn't be possible with our table constraints -- you've got a resource that's not tied to a group.
				AfxThrowNxException("Invalid superbill configuration group %li while loading resources.", nGroupID);
				return;
			}

			AddTextToCell(pRow, ercResourceText, strName);

			prsLoad->MoveNext();
		}
	}

	//Sixth, the Paths
	prsLoad = prsLoad->NextRecordset(NULL);
	if(eltLoadInfo & eltResources) {
		while(!prsLoad->eof) {
			long nGroupID = AdoFldLong(prsLoad, "GroupID");
			CString strPath = AdoFldString(prsLoad, "Path");

			//Since these are already saved to data, we do not need to ensure their status in the MergeTemplatesT table.
			//We do, however, need to remove the preceeding '\Templates\Forms\' text.
			strPath = strPath.Mid(17);

			//Find the right row
			IRowSettingsPtr pRow = m_pResourceList->FindByColumn(ercGroupID, (long)nGroupID, NULL, VARIANT_FALSE);
			if(pRow == NULL) {
				//This shouldn't be possible with our table constraints -- you've got a path that's not tied to a group.
				AfxThrowNxException("Invalid superbill configuration group %li while loading resource paths.", nGroupID);
				return;
			}

			AddTextToCell(pRow, ercPathText, strPath);

			prsLoad->MoveNext();
		}
	}

	m_pResourceList->SetRedraw(VARIANT_TRUE);
}

//Ensures that the interface is appropriately enabled, disabled, hidden, etc, based on current settings.  This
//	function will go through each of the checkboxes and enable/disable the corresponding controls that go with
//	that checkbox.
void CConfigureSuperbillTemplatesDlg::EnsureInterface()
{
	//
	//If the type is on, enable the type related interfaces
	BOOL bEnableType = FALSE;
	if(IsDlgButtonChecked(IDC_SUPERBILL_BY_TYPE)) {
		bEnableType = TRUE;
	}

	EnableDlgItem(IDC_SUPERBILL_ADD_TYPE, bEnableType);
	EnableDlgItem(IDC_SUPERBILL_MODIFY_TYPE, bEnableType);
	EnableDlgItem(IDC_SUPERBILL_REMOVE_TYPE, bEnableType);
	EnableDlgItem(IDC_SUPERBILL_TYPE_LIST, bEnableType);

	//
	//If the resource is on, enable the resource related interfaces
	BOOL bEnableResource = FALSE;
	if(IsDlgButtonChecked(IDC_SUPERBILL_BY_RESOURCE)) {
		bEnableResource = TRUE;
	}

	EnableDlgItem(IDC_SUPERBILL_ADD_RESOURCE, bEnableResource);
	EnableDlgItem(IDC_SUPERBILL_MODIFY_RESOURCE, bEnableResource);
	EnableDlgItem(IDC_SUPERBILL_REMOVE_RESOURCE, bEnableResource);
	EnableDlgItem(IDC_SUPERBILL_RESOURCE_LIST, bEnableResource);

	//
	//If the pick list is on, enable the pick list related interfaces
	BOOL bEnablePickList = FALSE;
	if(IsDlgButtonChecked(IDC_SUPERBILL_BY_PICKLIST)) {
		bEnablePickList = TRUE;
	}

	EnableDlgItem(IDC_SUPERBILL_ADD_PICKLIST, bEnablePickList);
	EnableDlgItem(IDC_SUPERBILL_REMOVE_PICKLIST, bEnablePickList);
	EnableDlgItem(IDC_SUPERBILL_PICK_LIST, bEnablePickList);

	//
	//If the Global is on, enable the global related interfaces
	BOOL bEnableGlobal = FALSE;
	if(IsDlgButtonChecked(IDC_SUPERBILL_GLOBAL)) {
		bEnableGlobal = TRUE;
	}

	EnableDlgItem(IDC_GLOBAL_TEMPLATE_PATH, bEnableGlobal);
	EnableDlgItem(IDC_GLOBAL_TEMPLATE_BROWSE, bEnableGlobal);
}

//////////////////////////////////////////////////////////////////////
//		These functions only cause the interface to enable/disable	//
//////////////////////////////////////////////////////////////////////
void CConfigureSuperbillTemplatesDlg::OnSuperbillBrowse() 
{
	try {
		SetRemotePropertyInt("SuperbillConfig_UseGlobal", 0, 0, "<None>");

		EnsureInterface();
	} NxCatchAll("OnSuperbillBrowse");
}

void CConfigureSuperbillTemplatesDlg::OnSuperbillGlobal() 
{
	try {
		SetRemotePropertyInt("SuperbillConfig_UseGlobal", 1, 0, "<None>");

		EnsureInterface();
	} NxCatchAll("OnSuperbillGlobal");
}

void CConfigureSuperbillTemplatesDlg::OnSuperbillByType() 
{
	try {
		SetRemotePropertyInt("SuperbillConfig_Types", IsDlgButtonChecked(IDC_SUPERBILL_BY_TYPE), 0, "<None>");

		EnsureInterface();
	} NxCatchAll("OnSuperbillByType");
}

void CConfigureSuperbillTemplatesDlg::OnSuperbillByResource() 
{
	try {
		SetRemotePropertyInt("SuperbillConfig_Resources", IsDlgButtonChecked(IDC_SUPERBILL_BY_RESOURCE), 0, "<None>");

		EnsureInterface();
	} NxCatchAll("OnSuperbillByResource");
}

void CConfigureSuperbillTemplatesDlg::OnSuperbillByPicklist() 
{
	try {
		SetRemotePropertyInt("SuperbillConfig_PickList", IsDlgButtonChecked(IDC_SUPERBILL_BY_PICKLIST), 0, "<None>");

		EnsureInterface();
	} NxCatchAll("OnSuperbillByPicklist");
}

//////////////////////////////////////////////////////////////////////
//			General Helper Functions								//
//////////////////////////////////////////////////////////////////////

//Given a path, starting at the end of the shared path, this function will find that record in MergeTemplatesT,
//	or, if it does not exist, will create it, and return the ID to you.  This function will throw an exception
//	if some failure happens in doing that.
//Parameters:
//	strPath - A path to a template, without the shared path.  This should start at \Templates\ in almost all cases.
//Return value:
//	The ID of the corresponding template in MergeTemplatesT
long CConfigureSuperbillTemplatesDlg::GetMergeTemplateID(CString strPath)
{
	//First, see if it already exists.
	_RecordsetPtr prs = CreateParamRecordset("SELECT ID FROM MergeTemplatesT WHERE Path = {STRING}", strPath);
	if(!prs->eof) {
		//Found it!
		return AdoFldLong(prs, "ID");
	}
	prs->Close();

	//Otherwise, the record was not found.  We'll want to insert a new one.
	_RecordsetPtr prsResults = CreateParamRecordset(
		"SET NOCOUNT ON;\r\n"
		"DECLARE @NewID int;\r\n"
		"SET @NewID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM MergeTemplatesT);\r\n"
		"INSERT INTO MergeTemplatesT (ID, Path, DefaultScope) values (@NewID, {STRING}, 0);\r\n"
		"SET NOCOUNT OFF;\r\n"
		"SELECT @NewID AS ID;\r\n", strPath);
	if(!prsResults->eof) {
		return AdoFldLong(prsResults, "ID");
	}

	//Otherwise, some kind of failure.  This should never happen
	AfxThrowNxException("Failed to create new MergeTemplate record for superbill '%s'.", strPath);
	return -1;
}


//Given a row, column, and text, this function will add that text to the existing
//	datalist row, separating it with a ', ' text.  It handles determining when
//	the separator is needed, etc.
//Parameters:
//	pRow - Any NxDataList 2.0 row.
//	nColumn - The column index where the text will be added.  This must be valid, no error checking is done.
//	strTextToAdd - The text that will be appended to the text already in the cell.
void CConfigureSuperbillTemplatesDlg::AddTextToCell(NXDATALIST2Lib::IRowSettingsPtr pRow, short nColumn, CString strTextToAdd)
{
	//Get the current text
	CString strCurrentText = VarString(pRow->GetValue(nColumn), "");

	//Add the separator if necessary
	if(!strCurrentText.IsEmpty()) {
		strCurrentText += ", ";
	}

	//Add the text we were given
	strCurrentText += strTextToAdd;

	//Put it back in the row
	pRow->PutValue(nColumn, _bstr_t(strCurrentText));
}


//////////////////////////////////////////////////////////////////////
//						Pick List Functions							//
//////////////////////////////////////////////////////////////////////
void CConfigureSuperbillTemplatesDlg::OnSuperbillAddPicklist() 
{
	try {
		CConfigureSuperbillAddFileDlg dlg(this);
		if(dlg.DoModal() == IDOK) {
			CString strSuperbill = dlg.m_strResults_SuperbillPath;

			//strSuperbill is now just the path from Forms onward.  Add it to our pick list.

			//First, we are saving our templates in MergeTemplatesT.  This table is a "fill as you go" sort of table, 
			//	so when a template is used somewhere, it should be inserted there.
			//The values in MergeTemplatesT always start at the shared path.
			long nMergeTemplateID = GetMergeTemplateID("\\Templates\\Forms" ^ strSuperbill);

			//And now that we have that value, we need to insert this record into our pick list data
			_RecordsetPtr prsInsert = CreateParamRecordset(
				"SET NOCOUNT ON;\r\n"
				"INSERT INTO SuperbillTemplatePickListT (TemplateID) values ({INT});\r\n"
				"SET NOCOUNT OFF;\r\n"
				"SELECT convert(int, @@identity) AS ID;\r\n", nMergeTemplateID);
			if(prsInsert->eof) {
				//This should never happen, we must get a value back.
				AfxThrowNxException("Failed to insert template '%s' into superbill pick list.", strSuperbill);
			}

			long nPickListID = AdoFldLong(prsInsert, "ID");
			prsInsert->Close();

			//Now create a row in the datalist and insert our values
			IRowSettingsPtr pRowNew = m_pPickList->GetNewRow();
			pRowNew->PutValue(eplcID, (long)nPickListID);
			pRowNew->PutValue(eplcTemplateID, (long)nMergeTemplateID);
			pRowNew->PutValue(eplcFilename, _bstr_t(strSuperbill));
			m_pPickList->AddRowSorted(pRowNew, NULL);
		}

	} NxCatchAll("OnSuperbillAddPicklist");
}

void CConfigureSuperbillTemplatesDlg::OnSuperbillRemovePicklist() 
{
	try {
		//Get the row selected
		IRowSettingsPtr pRowSel = m_pPickList->GetCurSel();
		if(pRowSel == NULL) {
			AfxMessageBox("You must first select a row in the pick list to remove.");
			return;
		}

		//Confirm it
		if(AfxMessageBox("Are you sure you wish to remove this superbill?", MB_YESNO) != IDYES) {
			return;
		}

		//Get the ID of the pick list record.  We will not touch the MergeTemplate table.
		long nID = VarLong(pRowSel->GetValue(eplcID));

		//Remove it from data
		ExecuteParamSql("DELETE FROM SuperbillTemplatePickListT WHERE ID = {INT}", nID);

		//Now remove it from the interface
		m_pPickList->RemoveRow(pRowSel);

	} NxCatchAll("OnSuperbillRemovePicklist");
}

//////////////////////////////////////////////////////////////////////
//						Appt Type Functions							//
//////////////////////////////////////////////////////////////////////
void CConfigureSuperbillTemplatesDlg::OnSuperbillAddType() 
{
	try {
		CConfigureSuperbillModifyGroupsDlg dlg(this);
		dlg.m_csitItemType = csitApptType;
		if(dlg.DoModal() == IDOK) {
			//The user is allowed to select nothing and commit, so check that first
			long nSize = dlg.m_aryItemsAdded.GetSize();
			if(nSize == 0) {
				return;
			}

			//Otherwise, we have a new group, and need to add the values that were just added to it 
			//	in data and put them into the interface.
			CNxParamSqlArray aryParams;
			CString strSqlBatch = BeginSqlBatch();
			//First, create a new group.
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
				"SET NOCOUNT ON;\r\n"
				"DECLARE @NewGroupID int;\r\n"
				"INSERT INTO SuperbillTemplateTypeGroupsT DEFAULT VALUES;\r\n"
				"SET @NewGroupID = convert(int, @@identity);\r\n");

			//Setup for our datalist row.  We'll only be adding one
			CString strAllTypes, strAllTemplates;

			//1)  Setup data for the types
			{
				for(int i = 0; i < nSize; i++) {
					ConfigItem ci = dlg.m_aryItemsAdded.GetAt(i);

					//Generate the SQL text to insert this new record.
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
						"INSERT INTO SuperbillTemplateTypeT (GroupID, TypeID) values (@NewGroupID, {INT});\r\n", ci.nID);

					//Now add to the interface setup
					strAllTypes += ci.strName + ", ";
				}

				//Cleanup the type text for interface, trim the last ', '
				if(!strAllTypes.IsEmpty()) {
					strAllTypes = strAllTypes.Left(strAllTypes.GetLength() - 2);
				}
			}

			//2)  Setup data for the templates
			{
				long nTemplateSize = dlg.m_aryTemplatesAdded.GetSize();
				for(int i = 0; i < nTemplateSize; i++) {
					CString strPath = dlg.m_aryTemplatesAdded.GetAt(i);

					//Make sure it's in MergeTemplatesT
					long nTemplateID = GetMergeTemplateID("\\Templates\\Forms" ^ strPath);

					//Generate the SQL text to insert this new record.
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
						"INSERT INTO SuperbillTemplateTypePathsT (GroupID, TemplateID) values (@NewGroupID, {INT});\r\n", nTemplateID);

					//Now add to the interface setup
					strAllTemplates += strPath + ", ";
				}

				//Cleanup the text for interface, trim the last ', '
				if(!strAllTemplates.IsEmpty()) {
					strAllTemplates = strAllTemplates.Left(strAllTemplates.GetLength() - 2);
				}
			}

			//Finalize stuff
			AddStatementToSqlBatch(strSqlBatch, 
				"SET NOCOUNT OFF;\r\n"
				"SELECT @NewGroupID AS ID;\r\n");

			//DRT Note:  At the time of this writing, ExecuteParamSqlBatch inexplicably throws
			//	away the results of the query, which I need.  So until that gets fixed, I'm
			//	copying it's functionality here.
			// (e.lally 2009-06-21) PLID 34679 - Fixed to use create recordset function.
			_RecordsetPtr prs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, aryParams);
			if(prs->eof) {
				//Should not be possible.
				AfxThrowNxException("The superbill type configuration data was saved, but NexTech was unable to process the return values to display it.  "
					"Please close and re-open this screen.");
			}

			long nGroupID = AdoFldLong(prs, "ID");

			//Now create a datalist record and place it in the interface
			IRowSettingsPtr pRowNew = m_pTypeList->GetNewRow();
			pRowNew->PutValue(etcGroupID, (long)nGroupID);
			pRowNew->PutValue(etcTypeText, _bstr_t(strAllTypes));
			pRowNew->PutValue(etcPathText, _bstr_t(strAllTemplates));
			m_pTypeList->AddRowSorted(pRowNew, NULL);
		}

	} NxCatchAll("OnSuperbillAddType");
}

void CConfigureSuperbillTemplatesDlg::OnSuperbillRemoveType() 
{
	try {
		//First ensure a row is selected
		IRowSettingsPtr pRow = m_pTypeList->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("You must select a row to remove.");
			return;
		}

		//Confirm
		if(AfxMessageBox("Are you sure you wish to remove this group?", MB_YESNO) != IDYES) {
			return;
		}

		//Do the removal
		long nGroupID = VarLong(pRow->GetValue(etcGroupID));

		CString strSqlBatch = BeginSqlBatch();
		CNxParamSqlArray aryParams;
		//Type tables
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
			"DELETE FROM SuperbillTemplateTypeT WHERE GroupID = {INT};\r\n", nGroupID);
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
			"DELETE FROM SuperbillTemplateTypePathsT WHERE GroupID = {INT};\r\n", nGroupID);
		//Clear the group
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
			"DELETE FROM SuperbillTemplateTypeGroupsT WHERE ID = {INT};\r\n", nGroupID);

		// (e.lally 2009-06-21) PLID 34679 - Leave as execute batch
		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);

		//Remove the datalist from the interface
		m_pTypeList->RemoveRow(pRow);

	} NxCatchAll("OnSuperbillRemoveType");
}

void CConfigureSuperbillTemplatesDlg::OnSuperbillModifyType() 
{
	try {
		//Get the current row
		IRowSettingsPtr pRow = m_pTypeList->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("You must first select a row to modify.");
			return;
		}

		//Now get the info out of the row and pop up the dialog to make changes.
		long nGroupID = VarLong(pRow->GetValue(etcGroupID));

		CConfigureSuperbillModifyGroupsDlg dlg(this);
		//Set the style
		dlg.m_csitItemType = csitApptType;
		dlg.m_nGroupID = nGroupID;

		//Since this is fairly simple data, and rarely changing, it's easier to just query the IDs needed now than to do it
		//	in the load and track them.
		_RecordsetPtr prsModify = CreateParamRecordset(
			"SELECT TypeID, AptTypeT.Name FROM SuperbillTemplateTypeT "
				"INNER JOIN AptTypeT ON SuperbillTemplateTypeT.TypeID = AptTypeT.ID "
				"WHERE SuperbillTemplateTypeT.GroupID = {INT};\r\n"
			"SELECT TemplateID, MergeTemplatesT.Path FROM SuperbillTemplateTypePathsT "
				"INNER JOIN MergeTemplatesT ON SuperbillTemplateTypePathsT.TemplateID = MergeTemplatesT.ID "
				"WHERE SuperbillTemplateTypePathsT.GroupID = {INT};\r\n", nGroupID, nGroupID);

		//First recordset is types
		while(!prsModify->eof) {
			ConfigItem ci;
			ci.nID = AdoFldLong(prsModify, "TypeID");
			ci.strName = AdoFldString(prsModify, "Name");

			dlg.m_aryInitialItems.Add(ci);

			prsModify->MoveNext();
		}

		//Next recordset is paths
		prsModify = prsModify->NextRecordset(NULL);
		while(!prsModify->eof) {
			CString strPath = AdoFldString(prsModify, "Path");

			//Remove the preceeding '\Templates\Forms\'
			strPath = strPath.Mid(17);

			dlg.m_aryInitialTemplates.Add(strPath);

			prsModify->MoveNext();
		}

		//Launch the dialog and let the user make changes
		if(dlg.DoModal()) {
			//User pressed OK, some changes may have been made.  We need to remove anything
			//	that was deleted, and add anything that was new.  For both types & paths
			CString strBatchSql = BeginSqlBatch();
			CNxParamSqlArray aryParams;
			int i = 0;	//loop control

			//1a)  Removals of types
			for(i = 0; i < dlg.m_aryItemsRemoved.GetSize(); i++) {
				ConfigItem ci = dlg.m_aryItemsRemoved.GetAt(i);
				AddParamStatementToSqlBatch(strBatchSql, aryParams, 
					"DELETE FROM SuperbillTemplateTypeT WHERE GroupID = {INT} AND TypeID = {INT};\r\n", nGroupID, ci.nID);
			}

			//1b)  Removals of paths
			for(i = 0; i < dlg.m_aryTemplatesRemoved.GetSize(); i++) {
				//Get the merge template ID
				long nTemplateID = GetMergeTemplateID("\\Templates\\Forms" ^ dlg.m_aryTemplatesRemoved.GetAt(i));

				AddParamStatementToSqlBatch(strBatchSql, aryParams, 
					"DELETE FROM SuperbillTemplateTypePathsT WHERE GroupID = {INT} AND TemplateID = {INT};\r\n", nGroupID, nTemplateID);
			}

			//2a)  Addition of types
			for(i = 0; i < dlg.m_aryItemsAdded.GetSize(); i++) {
				ConfigItem ci = dlg.m_aryItemsAdded.GetAt(i);
				AddParamStatementToSqlBatch(strBatchSql, aryParams, 
					"INSERT INTO SuperbillTemplateTypeT (GroupID, TypeID) values ({INT}, {INT});\r\n", nGroupID, ci.nID);
			}

			//2b)  Addition of paths
			for(i = 0; i < dlg.m_aryTemplatesAdded.GetSize(); i++) {
				//Find the template ID
				long nTemplateID = GetMergeTemplateID("\\Templates\\Forms" ^ dlg.m_aryTemplatesAdded.GetAt(i));

				AddParamStatementToSqlBatch(strBatchSql, aryParams, 
					"INSERT INTO SuperbillTemplateTypePathsT (GroupID, TemplateID) values ({INT}, {INT});\r\n", nGroupID, nTemplateID);
			}

			//Update data
			if(!strBatchSql.IsEmpty()) {
				// (e.lally 2009-06-21) PLID 34679 - Leave as execute batch
				ExecuteParamSqlBatch(GetRemoteData(), strBatchSql, aryParams);

				//Force a reload of the type data, we cannot rely on text matching with comma delimiters
				//	to re-parse and re-create the proper row text, so we'll just have to requery it.
				m_pTypeList->Clear();
				LoadFromData(eltTypes);
			}
		}

	} NxCatchAll("OnSuperbillModifyType");
}

//////////////////////////////////////////////////////////////////////
//						Appt Resource Functions						//
//////////////////////////////////////////////////////////////////////
void CConfigureSuperbillTemplatesDlg::OnSuperbillAddResource() 
{
	try {
		CConfigureSuperbillModifyGroupsDlg dlg(this);
		dlg.m_csitItemType = csitApptResource;
		if(dlg.DoModal() == IDOK) {
			//The user is allowed to select nothing and commit, so check that first
			long nSize = dlg.m_aryItemsAdded.GetSize();
			if(nSize == 0) {
				return;
			}

			//Otherwise, we have a new group, and need to add the values that were just added to it 
			//	in data and put them into the interface.
			CNxParamSqlArray aryParams;
			CString strSqlBatch = BeginSqlBatch();
			//First, create a new group.
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
				"SET NOCOUNT ON;\r\n"
				"DECLARE @NewGroupID int;\r\n"
				"INSERT INTO SuperbillTemplateResourceGroupsT DEFAULT VALUES;\r\n"
				"SET @NewGroupID = convert(int, @@identity);\r\n");

			//Setup for our datalist row.  We'll only be adding one
			CString strAllResources, strAllTemplates;

			//1)  Setup data for the resources
			{
				for(int i = 0; i < nSize; i++) {
					ConfigItem ci = dlg.m_aryItemsAdded.GetAt(i);

					//Generate the SQL text to insert this new record.
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
						"INSERT INTO SuperbillTemplateResourceT (GroupID, ResourceID) values (@NewGroupID, {INT});\r\n", ci.nID);

					//Now add to the interface setup.
					strAllResources += ci.strName + ", ";
				}

				//Cleanup the resource text for interface, trim the last ', '
				if(!strAllResources.IsEmpty()) {
					strAllResources = strAllResources.Left(strAllResources.GetLength() - 2);
				}
			}

			//2)  Setup data for the templates
			{
				long nTemplateSize = dlg.m_aryTemplatesAdded.GetSize();
				for(int i = 0; i < nTemplateSize; i++) {
					CString strPath = dlg.m_aryTemplatesAdded.GetAt(i);

					//Make sure it's in MergeTemplatesT
					long nTemplateID = GetMergeTemplateID("\\Templates\\Forms" ^ strPath);

					//Generate the SQL text to insert this new record.
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
						"INSERT INTO SuperbillTemplateResourcePathsT (GroupID, TemplateID) values (@NewGroupID, {INT});\r\n", nTemplateID);

					//Now add to the interface setup
					strAllTemplates += strPath + ", ";
				}

				//Cleanup the text for interface, trim the last ', '
				if(!strAllTemplates.IsEmpty()) {
					strAllTemplates = strAllTemplates.Left(strAllTemplates.GetLength() - 2);
				}
			}

			//Finalize stuff
			AddStatementToSqlBatch(strSqlBatch, 
				"SET NOCOUNT OFF;\r\n"
				"SELECT @NewGroupID AS ID;\r\n");

			//DRT Note:  At the time of this writing, ExecuteParamSqlBatch inexplicably throws
			//	away the results of the query, which I need.  So until that gets fixed, I'm
			//	copying it's functionality here.
			// (e.lally 2009-06-21) PLID 34679 - Fixed to use create recordset function.
			_RecordsetPtr prs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, aryParams);
			if(prs->eof) {
				//Should not be possible.
				AfxThrowNxException("The superbill resource configuration data was saved, but NexTech was unable to process the return values to display it.  "
					"Please close and re-open this screen.");
			}

			long nGroupID = AdoFldLong(prs, "ID");

			//Now create a datalist record and place it in the interface
			IRowSettingsPtr pRowNew = m_pResourceList->GetNewRow();
			pRowNew->PutValue(ercGroupID, (long)nGroupID);
			pRowNew->PutValue(ercResourceText, _bstr_t(strAllResources));
			pRowNew->PutValue(ercPathText, _bstr_t(strAllTemplates));
			m_pResourceList->AddRowSorted(pRowNew, NULL);
		}

	} NxCatchAll("OnSuperbillAddResource");
}

void CConfigureSuperbillTemplatesDlg::OnSuperbillRemoveResource() 
{
	try {
		//First ensure a row is selected
		IRowSettingsPtr pRow = m_pResourceList->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("You must select a row to remove.");
			return;
		}

		//Confirm
		if(AfxMessageBox("Are you sure you wish to remove this group?", MB_YESNO) != IDYES) {
			return;
		}

		//Do the removal
		long nGroupID = VarLong(pRow->GetValue(ercGroupID));

		CString strSqlBatch = BeginSqlBatch();
		CNxParamSqlArray aryParams;
		//Resource tables
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
			"DELETE FROM SuperbillTemplateResourceT WHERE GroupID = {INT};\r\n", nGroupID);
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
			"DELETE FROM SuperbillTemplateResourcePathsT WHERE GroupID = {INT};\r\n", nGroupID);
		//Clear the group
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
			"DELETE FROM SuperbillTemplateResourceGroupsT WHERE ID = {INT};\r\n", nGroupID);

		// (e.lally 2009-06-21) PLID 34679 - Leave as execute batch
		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);

		//Remove the datalist from the interface
		m_pResourceList->RemoveRow(pRow);

	} NxCatchAll("OnSuperbillRemoveResource");
}

void CConfigureSuperbillTemplatesDlg::OnSuperbillModifyResource() 
{
	try {
		//Get the current row
		IRowSettingsPtr pRow = m_pResourceList->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("You must first select a row to modify.");
			return;
		}

		//Now get the info out of the row and pop up the dialog to make changes.
		long nGroupID = VarLong(pRow->GetValue(ercGroupID));

		CConfigureSuperbillModifyGroupsDlg dlg(this);
		//Set the style
		dlg.m_csitItemType = csitApptResource;
		dlg.m_nGroupID = nGroupID;

		//Since this is fairly simple data, and rarely changing, it's easier to just query the IDs needed now than to do it
		//	in the load and track them.
		_RecordsetPtr prsModify = CreateParamRecordset(
			"SELECT ResourceID, ResourceT.Item FROM SuperbillTemplateResourceT "
				"INNER JOIN ResourceT ON SuperbillTemplateResourceT.ResourceID = ResourceT.ID "
				"WHERE SuperbillTemplateResourceT.GroupID = {INT};\r\n"
			"SELECT TemplateID, MergeTemplatesT.Path FROM SuperbillTemplateResourcePathsT "
				"INNER JOIN MergeTemplatesT ON SuperbillTemplateResourcePathsT.TemplateID = MergeTemplatesT.ID "
				"WHERE SuperbillTemplateResourcePathsT.GroupID = {INT};\r\n", nGroupID, nGroupID);

		//First recordset is resources
		while(!prsModify->eof) {
			ConfigItem ci;
			ci.nID = AdoFldLong(prsModify, "ResourceID");
			ci.strName = AdoFldString(prsModify, "Item");

			dlg.m_aryInitialItems.Add(ci);

			prsModify->MoveNext();
		}

		//Next recordset is paths
		prsModify = prsModify->NextRecordset(NULL);
		while(!prsModify->eof) {
			CString strPath = AdoFldString(prsModify, "Path");

			//Remove the preceeding '\Templates\Forms\'
			strPath = strPath.Mid(17);

			dlg.m_aryInitialTemplates.Add(strPath);

			prsModify->MoveNext();
		}

		//Launch the dialog and let the user make changes
		if(dlg.DoModal()) {
			//User pressed OK, some changes may have been made.  We need to remove anything
			//	that was deleted, and add anything that was new.  For both resources & paths
			CString strBatchSql = BeginSqlBatch();
			CNxParamSqlArray aryParams;
			int i = 0;	//loop control

			//1a)  Removals of resources
			for(i = 0; i < dlg.m_aryItemsRemoved.GetSize(); i++) {
				ConfigItem ci = dlg.m_aryItemsRemoved.GetAt(i);
				AddParamStatementToSqlBatch(strBatchSql, aryParams, 
					"DELETE FROM SuperbillTemplateResourceT WHERE GroupID = {INT} AND ResourceID = {INT};\r\n", nGroupID, ci.nID);
			}

			//1b)  Removals of paths
			for(i = 0; i < dlg.m_aryTemplatesRemoved.GetSize(); i++) {
				//Get the merge template ID
				long nTemplateID = GetMergeTemplateID("\\Templates\\Forms" ^ dlg.m_aryTemplatesRemoved.GetAt(i));

				AddParamStatementToSqlBatch(strBatchSql, aryParams, 
					"DELETE FROM SuperbillTemplateResourcePathsT WHERE GroupID = {INT} AND TemplateID = {INT};\r\n", nGroupID, nTemplateID);
			}

			//2a)  Addition of resources
			for(i = 0; i < dlg.m_aryItemsAdded.GetSize(); i++) {
				ConfigItem ci = dlg.m_aryItemsAdded.GetAt(i);
				AddParamStatementToSqlBatch(strBatchSql, aryParams, 
					"INSERT INTO SuperbillTemplateResourceT (GroupID, ResourceID) values ({INT}, {INT});\r\n", nGroupID, ci.nID);
			}

			//2b)  Addition of paths
			for(i = 0; i < dlg.m_aryTemplatesAdded.GetSize(); i++) {
				//Find the template ID
				long nTemplateID = GetMergeTemplateID("\\Templates\\Forms" ^ dlg.m_aryTemplatesAdded.GetAt(i));

				AddParamStatementToSqlBatch(strBatchSql, aryParams, 
					"INSERT INTO SuperbillTemplateResourcePathsT (GroupID, TemplateID) values ({INT}, {INT});\r\n", nGroupID, nTemplateID);
			}

			//Update data
			if(!strBatchSql.IsEmpty()) {
				// (e.lally 2009-06-21) PLID 34679 - Leave as execute batch
				ExecuteParamSqlBatch(GetRemoteData(), strBatchSql, aryParams);

				//Force a reload of the resource data, we cannot rely on text matching with comma delimiters
				//	to re-parse and re-create the proper row text, so we'll just have to requery it.
				m_pResourceList->Clear();
				LoadFromData(eltResources);
			}
		}

	} NxCatchAll("OnSuperbillModifyResource");
}

//////////////////////////////////////////////////////////////////////
//			Remaining Interface Functions							//
//////////////////////////////////////////////////////////////////////

//The user is browsing for a new global template.  If they pick a new one, update
//	the text list and save.  Do nothing on cancel.
void CConfigureSuperbillTemplatesDlg::OnGlobalTemplateBrowse() 
{
	try {
		CConfigureSuperbillAddFileDlg dlg(this);
		if(dlg.DoModal() == IDOK) {
			CString strPath = dlg.m_strResults_SuperbillPath;

			//Just overwrite what is saved in the global setting
			SetPropertyText("DefaultSuperbillFilename", strPath, 0);

			//And show it in the interface
			SetDlgItemText(IDC_GLOBAL_TEMPLATE_PATH, strPath);
		}

	} NxCatchAll("OnGlobalTemplateBrowse");
}

//All saving is done realtime, as changes are made, so these functions need
//	do nothing except dismiss the dialog.
void CConfigureSuperbillTemplatesDlg::OnOK() 
{
	try {
		CDialog::OnOK();
	} NxCatchAll("Error in OnOK");
}

//All saving is done realtime, as changes are made, so these functions need
//	do nothing except dismiss the dialog.
void CConfigureSuperbillTemplatesDlg::OnCancel() 
{
	try {
		OnOK();
	} NxCatchAll("Error in OnCancel");
}

BEGIN_EVENTSINK_MAP(CConfigureSuperbillTemplatesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CConfigureSuperbillTemplatesDlg)
	ON_EVENT(CConfigureSuperbillTemplatesDlg, IDC_SUPERBILL_TYPE_LIST, 3 /* DblClickCell */, OnDblClickCellSuperbillTypeList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CConfigureSuperbillTemplatesDlg, IDC_SUPERBILL_RESOURCE_LIST, 3 /* DblClickCell */, OnDblClickCellSuperbillResourceList, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CConfigureSuperbillTemplatesDlg::OnDblClickCellSuperbillTypeList(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		if(lpRow == NULL) {
			return;
		}

		//Just edit the existing row.
		OnSuperbillModifyType();

	} NxCatchAll("Error in OnDblClickCellSuperbillTypeList");
}

void CConfigureSuperbillTemplatesDlg::OnDblClickCellSuperbillResourceList(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		//ensure a row is selected
		if(lpRow == NULL) {
			return;
		}

		//Just edit the existing row.
		OnSuperbillModifyResource();

	} NxCatchAll("Error in OnDblClickCellSuperbillResourceList");
}
