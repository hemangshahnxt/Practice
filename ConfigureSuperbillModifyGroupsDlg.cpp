// ConfigureSuperbillModifyGroupsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigureSuperbillModifyGroupsDlg.h"
#include "ConfigureSuperbillAddFileDlg.h"
#include "MultiSelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//DRT 6/6/2008 - PLID 30306 - Created.

//Datalist column enumerations
enum eItemColumns {
	eicID = 0,
	eicName,
};

enum eTemplateColumns {
	etcPath = 0,
};

//Compare function for the ConfigItem struct.  We only care about the ID, so just search on that
bool IsConfigItemInArray(ConfigItem ci, CArray<ConfigItem, ConfigItem&> *pary)
{
	long nSize = pary->GetSize();
	for(int i = 0; i < nSize; i++) {
		ConfigItem ciCompare = pary->GetAt(i);
		if(ci.nID == ciCompare.nID) {
			//Matched
			return true;
		}
	}

	//Not found
	return false;
}


/////////////////////////////////////////////////////////////////////////////
// CConfigureSuperbillModifyGroupsDlg dialog

CConfigureSuperbillModifyGroupsDlg::CConfigureSuperbillModifyGroupsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigureSuperbillModifyGroupsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigureSuperbillModifyGroupsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_csitItemType = csitUnknown;
	m_nGroupID = -1;
}


void CConfigureSuperbillModifyGroupsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigureSuperbillModifyGroupsDlg)
	DDX_Control(pDX, IDC_SUPERBILL_GROUPNAME, m_nxstaticItemLabel);
	DDX_Control(pDX, IDC_SUPERBILL_TEMPLATE_LABEL, m_nxstaticTemplateLabel);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_SUPERBILL_GROUP_REMOVE_TEMPLATE, m_btnRemoveTemplate);
	DDX_Control(pDX, IDC_SUPERBILL_GROUP_REMOVE_ITEM, m_btnRemoveItem);
	DDX_Control(pDX, IDC_SUPERBILL_GROUP_ADD_TEMPLATE, m_btnAddTemplate);
	DDX_Control(pDX, IDC_SUPERBILL_GROUP_ADD_ITEM, m_btnAddItem);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigureSuperbillModifyGroupsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConfigureSuperbillModifyGroupsDlg)
	ON_BN_CLICKED(IDC_SUPERBILL_GROUP_ADD_ITEM, OnSuperbillGroupAddItem)
	ON_BN_CLICKED(IDC_SUPERBILL_GROUP_ADD_TEMPLATE, OnSuperbillGroupAddTemplate)
	ON_BN_CLICKED(IDC_SUPERBILL_GROUP_REMOVE_ITEM, OnSuperbillGroupRemoveItem)
	ON_BN_CLICKED(IDC_SUPERBILL_GROUP_REMOVE_TEMPLATE, OnSuperbillGroupRemoveTemplate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigureSuperbillModifyGroupsDlg message handlers

BOOL CConfigureSuperbillModifyGroupsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		//Bind Lists
		m_pItemList = BindNxDataList2Ctrl(IDC_SUPERBILL_GROUP_LIST, false);
		m_pTemplateList = BindNxDataList2Ctrl(IDC_SUPERBILL_GROUP_TEMPLATE_LIST, false);

		//Setup buttons
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnOK.AutoSet(NXB_OK);
		m_btnRemoveTemplate.AutoSet(NXB_DELETE);
		m_btnRemoveItem.AutoSet(NXB_DELETE);
		m_btnAddTemplate.AutoSet(NXB_NEW);
		m_btnAddItem.AutoSet(NXB_NEW);

		//Ensure that the developer has correctly passed in a type
		if(m_csitItemType == csitUnknown) {
			AfxMessageBox("Illegal item type specified for superbill group configuration.");
			CDialog::OnOK();
			return TRUE;
		}

		//Load the "initial" settings into the datalist.
		LoadInitialData();

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigureSuperbillModifyGroupsDlg::LoadInitialData()
{
	//Sync the arrays with the datalists

	//
	//1)	Items.
	{
		//The data doesn't matter what type of dialog this is, it's all made to be generic.
		for(int i = 0; i < m_aryInitialItems.GetSize(); i++) {
			ConfigItem ci = m_aryInitialItems.GetAt(i);

			AddRowToItemList(&ci);
		}
	}

	//
	//2)  Templates
	{
		for(int i = 0; i < m_aryInitialTemplates.GetSize(); i++) {
			AddRowToTemplateList(m_aryInitialTemplates.GetAt(i));
		}
	}

	//
	//3)  Interface.  We need to modify some of the screen text to reference the types.
	{
		CString strName = "Items";
		switch(m_csitItemType) {
		case csitApptType:
			{
				strName = "Types";
			}
			break;
		case csitApptResource:
			{
				strName = "Resources";
			}
			break;
		}

		SetDlgItemText(IDC_SUPERBILL_GROUPNAME, strName);
	}
}

//This function will parse the existing datalist of items against the member array of
//	initial items, filling both m_aryItemsAdded and m_aryItemsRemoved with 
//	their respective items.
void CConfigureSuperbillModifyGroupsDlg::DetermineItemListChanges()
{
	//Iterate through the datalist.  For each row, see if it already existed in our initial array.
	//	If it did not, it's a new template.
	//Also while we're iterating here, copy all the values into a temporary "current" list for faster
	//	execution on the 2nd go round.
	CArray<ConfigItem, ConfigItem&> aryCurrent;

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pItemList->FindAbsoluteFirstRow(VARIANT_TRUE);
	while(pRow != NULL) {
		ConfigItem ci;
		ci.nID = VarLong(pRow->GetValue(eicID));
		ci.strName = VarString(pRow->GetValue(eicName));

		//Add to our current array so we only need iterate the datalist once.
		aryCurrent.Add(ci);

		//See if it exists in our initial array
		if(IsConfigItemInArray(ci, &m_aryInitialItems)) {
			//It does exist, so we know this is not a new template.  Do nothing
		}
		else {
			//It did not exist when we started, so it must be new.  Add it to the 'new' list.
			m_aryItemsAdded.Add(ci);
		}

		//Move to the next datalist record
		pRow = m_pItemList->FindAbsoluteNextRow(pRow, VARIANT_TRUE);
	}

	//
	//At this point we're done with the "added" list, but we need to find the ones removed.  We'll use the "current" array that
	//	we generated in the first iteration to make this a little easier.
	long nInitialSize = m_aryInitialItems.GetSize();
	for(int i = 0; i < nInitialSize; i++) {
		ConfigItem ci = m_aryInitialItems.GetAt(i);

		if(IsConfigItemInArray(ci, &aryCurrent)) {
			//This does exist in the current still, so it was not removed.  Do nothing
		}
		else {
			//It no longer exists, must have been removed.  Add it to the 'removed' list.
			m_aryItemsRemoved.Add(ci);
		}
	}
}

//This function will parse the existing datalist of templates against the member array of
//	initial templates, filling both m_aryTemplatesAdded and m_aryTemplatesRemoved with 
//	their respective templates.
void CConfigureSuperbillModifyGroupsDlg::DetermineTemplateListChanges()
{
	//Iterate through the datalist.  For each row, see if it already existed in our initial array.
	//	If it did not, it's a new template.
	//Also while we're iterating here, copy all the values into a temporary "current" list for faster
	//	execution on the 2nd go round.
	CStringArray aryCurrent;

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTemplateList->FindAbsoluteFirstRow(VARIANT_TRUE);
	while(pRow != NULL) {
		CString strTemplate = VarString(pRow->GetValue(etcPath));

		//Add to our current array so we only need iterate the datalist once.
		aryCurrent.Add(strTemplate);

		//See if it exists in our initial array
		if(IsStringInStringArray(strTemplate, &m_aryInitialTemplates)) {
			//It does exist, so we know this is not a new template.  Do nothing
		}
		else {
			//It did not exist when we started, so it must be new.  Add it to the 'new' list.
			m_aryTemplatesAdded.Add(strTemplate);
		}

		//Move to the next datalist record
		pRow = m_pTemplateList->FindAbsoluteNextRow(pRow, VARIANT_TRUE);
	}

	//
	//At this point we're done with the "added" list, but we need to find the ones removed.  We'll use the "current" array that
	//	we generated in the first iteration to make this a little easier.
	long nInitialSize = m_aryInitialTemplates.GetSize();
	for(int i = 0; i < nInitialSize; i++) {
		CString strTemplate = m_aryInitialTemplates.GetAt(i);

		if(IsStringInStringArray(strTemplate, &aryCurrent)) {
			//This does exist in the current still, so it was not removed.  Do nothing
		}
		else {
			//It no longer exists, must have been removed.  Add it to the 'removed' list.
			m_aryTemplatesRemoved.Add(strTemplate);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//						Interface Button Handlers						//
//////////////////////////////////////////////////////////////////////////
void CConfigureSuperbillModifyGroupsDlg::OnOK() 
{
	try {
		//First, do we need to do any validation?
		//	Ensure that if there's anything in 1 list, there's also something in the other.  You can
		//	commit with nothing in either list, but you can't commit with 1 empty and the other not.
		long nItems = m_pItemList->GetRowCount();
		long nTemplates = m_pTemplateList->GetRowCount();
		if( (nItems != 0 && nTemplates == 0) || (nTemplates != 0 && nItems == 0)) {
			AfxMessageBox("You must select data on both lists to commit your changes.");
			return;
		}

		//We need to figure out what happened during the dialog's execution and fill the 
		//	lists accordingly.
		DetermineItemListChanges();

		DetermineTemplateListChanges();

		//At this point our arrays are set and we are happy to finish.  The user can retrieve their
		//	results from the member arrays.
		CDialog::OnOK();
	} NxCatchAll("Error in OnOK");
}

//Nothing to cleanup, just dismiss.
void CConfigureSuperbillModifyGroupsDlg::OnCancel() 
{
	CDialog::OnCancel();
}

//////////////////////////////////////////////////////////////////////////
//						Functionality for Items							//
//////////////////////////////////////////////////////////////////////////
void CConfigureSuperbillModifyGroupsDlg::OnSuperbillGroupAddItem() 
{
	try {
		CMultiSelectDlg dlg(this, "");

		//Determine the fields to use based on the type of dialog we're in
		CString strFrom;
		CString strWhere;
		CString strID;
		CString strValue;
		CString strDesc;

		//The idea behind the filtering is thus:
		//	Since we're asking them to setup what templates go with which types, we cannot allow duplication, or
		//	there's no way to choose which one is right.  There is no ordering to this stuff.  So this modify dialog
		//	will ONLY show items which don't exist in an existing group.  However, we must include the ones from
		//	the group we are currently modifying, otherwise it would be possible to remove an element and have no way
		//	to get it back unless you saved and re-modified.  There is then additional code below that checks for
		//	duplication specific to this dialog.
		switch(m_csitItemType) {
		case csitApptType:
			strFrom = "AptTypeT";
			strWhere = FormatString("Inactive = 0 AND AptTypeT.ID NOT IN (SELECT TypeID FROM SuperbillTemplateTypeT WHERE GroupID <> %li)", m_nGroupID);
			strID = "ID";
			strValue = "Name";
			strDesc = "Please select Appointment Types";
			break;
		case csitApptResource:
			strFrom = "ResourceT";
			strWhere = FormatString("Inactive = 0 AND ResourceT.ID NOT IN (SELECT ResourceID FROM SuperbillTemplateResourceT WHERE GroupID <> %li)", m_nGroupID);
			strID = "ID";
			strValue = "Item";
			strDesc = "Please select Resources";
			break;
		}

		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		dlg.SetSizingConfigRT(strFrom);

		if(dlg.Open(strFrom, strWhere, strID, strValue, strDesc) == IDOK) {
			//We need both the ID and name from here, so we'll have to parse the datalist
			//	for results.  Keep in mind that this is a datalist1, and the rest of our
			//	dialog uses datalist2's, so we need to be careful to use the right namespace.
			// (j.jones 2009-08-12 09:47) - PLID 35189 - the multi-select list is now a datalist 2
			NXDATALIST2Lib::_DNxDataListPtr pMultiList = dlg.GetDataList2();

			//Iterate through all the rows in the datalist
			CString strSkippedMessage;
			NXDATALIST2Lib::IRowSettingsPtr pRow = pMultiList->GetFirstRow();
			while(pRow) {
				//Get the next row

				// (a.walling 2008-10-02 10:34) - PLID 31567 - Specifying the enumeration's name is unnecessary
				if(VarBool(pRow->GetValue(CMultiSelectDlg::mslcSelected), FALSE)) {
					//This row is selected, we want its secrets
					ConfigItem ci;
					// (a.walling 2008-10-02 10:34) - PLID 31567 - Specifying the enumeration's name is unnecessary
					ci.nID = VarLong(pRow->GetValue(CMultiSelectDlg::mslcID));
					ci.strName = VarString(pRow->GetValue(CMultiSelectDlg::mslcName));

					//We cannot allow duplicates to be added to the list -- there would be no particular order in which
					//	to pick them.  So if the item already exists, just skip adding it and warn them.
					//Remember that the main list we're searching is a datalist 2.
					if(m_pItemList->FindByColumn(eicID, (long)ci.nID, NULL, VARIANT_FALSE) != NULL) {
						strSkippedMessage += FormatString(" - %s\r\n", ci.strName);
					}
					else {
						AddRowToItemList(&ci);
					}
				}

				pRow = pRow->GetNextRow();
			}

			if(!strSkippedMessage.IsEmpty()) {
				AfxMessageBox("The following items already existed and were skipped:\r\n" + strSkippedMessage, MB_OK);
			}
		}

	} NxCatchAll("Error in OnSuperbillGroupAddItem");
}

void CConfigureSuperbillModifyGroupsDlg::OnSuperbillGroupRemoveItem() 
{
	try {
		//Ensure they're on a row
		NXDATALIST2Lib::IRowSettingsPtr pRowSel = m_pItemList->GetCurSel();
		if(pRowSel == NULL) {
			AfxMessageBox("You must select a row from the list to remove.");
			return;
		}

		//Confirm
		if(AfxMessageBox("Are you sure you wish to remove this item from the group?", MB_YESNO) != IDYES) {
			return;
		}

		//Drop it
		m_pItemList->RemoveRow(pRowSel);

	} NxCatchAll("Error in OnSuperbillGroupRemoveItem");
}

//////////////////////////////////////////////////////////////////////////
//						Functionality for Templates						//
//////////////////////////////////////////////////////////////////////////
void CConfigureSuperbillModifyGroupsDlg::OnSuperbillGroupAddTemplate() 
{
	try {
		//Use the same generic "add superbill" dialog that we use everywhere else.
		CConfigureSuperbillAddFileDlg dlg(this);
		if(dlg.DoModal() == IDOK) {
			//We only want to work with the "superbill path", which is the path starting after \Templates\Forms
			CString strPath = dlg.m_strResults_SuperbillPath;

			//Ensure it does not already exist.  We don't want them adding the same template twice, that'd be silly.
			if(m_pTemplateList->FindByColumn(etcPath, _bstr_t(strPath), NULL, VARIANT_FALSE) != NULL) {
				//Already exists
				AfxMessageBox(FormatString("The path '%s' already exists and will not be added a second time.", strPath), MB_OK);
			}
			else {
				AddRowToTemplateList(strPath);
			}
		}

	} NxCatchAll("Error in OnSuperbillGroupAddTemplate");
}

void CConfigureSuperbillModifyGroupsDlg::OnSuperbillGroupRemoveTemplate() 
{
	try {
		//Ensure they're on a row
		NXDATALIST2Lib::IRowSettingsPtr pRowSel = m_pTemplateList->GetCurSel();
		if(pRowSel == NULL) {
			AfxMessageBox("You must select a row from the template list to remove.");
			return;
		}

		//Confirm
		if(AfxMessageBox("Are you sure you wish to remove this template from the group?", MB_YESNO) != IDYES) {
			return;
		}

		//Drop it
		m_pTemplateList->RemoveRow(pRowSel);

	} NxCatchAll("Error in OnSuperbillGroupRemoveTemplate");
}

//////////////////////////////////////////////////////////////////////////
//						Generic Helper Functions						//
//////////////////////////////////////////////////////////////////////////

//Given a path, performs the work required to add that path to the template datalist.
void CConfigureSuperbillModifyGroupsDlg::AddRowToTemplateList(CString strPath)
{
	NXDATALIST2Lib::IRowSettingsPtr pRowNew = m_pTemplateList->GetNewRow();
	pRowNew->PutValue(etcPath, _bstr_t(strPath));
	m_pTemplateList->AddRowSorted(pRowNew, NULL);
}

//Given a pointer to a ConfigItem structure, performs all work required to add that item
//	to the datalist of items.
void CConfigureSuperbillModifyGroupsDlg::AddRowToItemList(ConfigItem *pci)
{
	//Ensure it exists
	if(pci == NULL)
		return;

	NXDATALIST2Lib::IRowSettingsPtr pRowNew = m_pItemList->GetNewRow();
	pRowNew->PutValue(eicID, (long)pci->nID);
	pRowNew->PutValue(eicName, _bstr_t(pci->strName));
	m_pItemList->AddRowSorted(pRowNew, NULL);
}
