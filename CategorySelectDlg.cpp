// CategorySelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CategorySelectDlg.h"
#include "MultiSelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;
using namespace ADODB;

#define MAX_LEVELS 4

enum CategoryTreeColumns
{
	ctcID = 0,
	// (j.jones 2015-03-02 08:52) - PLID 64962 - added Selected checkbox, only shown when "allow multiple" is checked
	ctcSelected,
	ctcName,
	ctcParentID,
};

/////////////////////////////////////////////////////////////////////////////
// CCategorySelectDlg dialog

// (j.jones 2015-02-27 16:22) - PLID 64962 - added bAllowMultiSelect
// (j.jones 2015-03-02 15:36) - PLID 64970 - added strItemType, like "service code" or "inventory item"
CCategorySelectDlg::CCategorySelectDlg(CWnd* pParent, bool bAllowMultiSelect, CString strItemType)
	: CNxDialog(CCategorySelectDlg::IDD, pParent)
{
	m_nInitialDefaultCategoryID = -1;
	m_nSelectedDefaultCategoryID = -1;
	m_bAllowMultiSelect = bAllowMultiSelect;
	m_strItemType = strItemType;

	//ensure this is filled
	if (m_strItemType.IsEmpty()) {
		//make your calling code fill this!
		ASSERT(FALSE);
		m_strItemType = "item";
	}
}


void CCategorySelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCategorySelectDlg)
	DDX_Control(pDX, IDC_CATEGORY_TREE_ALLOW_MULTI_SELECT, m_checkAllowMultiSelect);
	DDX_Control(pDX, IDC_CATEGORY_TREE_ENABLE_DRAG, m_btnEnableDragDrop);
	DDX_Control(pDX, IDC_ADD_NEW, m_btnAddNew);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCategorySelectDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCategorySelectDlg)
	ON_BN_CLICKED(IDC_ADD_NEW, OnAddNew)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CATEGORY_TREE_ALLOW_MULTI_SELECT, OnAllowMultiSelect)
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CCategorySelectDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCategorySelectDlg)
	ON_EVENT(CCategorySelectDlg, IDC_CATEGORIES_TREE, 3 /* DblClickCell */, OnDblClickCellCategoriesTree, VTS_DISPATCH VTS_I2)
	ON_EVENT(CCategorySelectDlg, IDC_CATEGORIES_TREE, 6 /* RButtonDown */, OnRButtonDownCategoriesTree, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCategorySelectDlg, IDC_CATEGORIES_TREE, 12 /* DragBegin */, OnDragBeginCategoriesTree, VTS_PBOOL VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CCategorySelectDlg, IDC_CATEGORIES_TREE, 13 /* DragOverCell */, OnDragOverCellCategoriesTree, VTS_PBOOL VTS_DISPATCH VTS_I2 VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CCategorySelectDlg, IDC_CATEGORIES_TREE, 14 /* DragEnd */, OnDragEndCategoriesTree, VTS_DISPATCH VTS_I2 VTS_DISPATCH VTS_I2 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCategorySelectDlg message handlers

BOOL CCategorySelectDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		// (z.manning, 04/30/2008) - PLID 29850 - Set button styles
		m_btnAddNew.AutoSet(NXB_NEW);
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.jones 2015-02-27 16:32) - PLID 64962 - multi-select is not always allowed,
		// hide the checkbox if not allowed
		m_checkAllowMultiSelect.ShowWindow(m_bAllowMultiSelect ? SW_SHOW : SW_HIDE);

		// (z.manning, 03/07/2007) - PLID 24615 - The category tree now uses a datalist2.
		m_pdlCategoryTree = BindNxDataList2Ctrl(this, IDC_CATEGORIES_TREE, GetRemoteData(), true);

		if(m_dlgName != "") {
			SetWindowText(m_dlgName);
		}

		// (j.jones 2015-03-02 08:43) - PLID 64962 - fix incorrect data
		if (m_aryInitialCategoryIDs.size() == 0 && m_nInitialDefaultCategoryID != -1) {
			//there can't be a default category ID and no categories,
			//find and fix the calling code
			ASSERT(FALSE);
			m_aryInitialCategoryIDs.push_back(m_nInitialDefaultCategoryID);
		}
		else if (m_aryInitialCategoryIDs.size() == 1 && m_nInitialDefaultCategoryID == -1)
		{
			//not necessarily bad code, but if an item has one category,
			//it is the default category
			m_nInitialDefaultCategoryID = m_aryInitialCategoryIDs[0];
		}

		// (j.jones 2015-03-02 08:39) - PLID 64962 - there can now be multiple categories
		// and an optional default, the default is not displayed on this dialog
		if (m_aryInitialCategoryIDs.size() > 1) {
			//check the multi-select checkbox
			m_checkAllowMultiSelect.SetCheck(true);
		}

		//if multi-select is unchecked, hide the Selected column
		if (!m_checkAllowMultiSelect.GetCheck()) {
			m_pdlCategoryTree->GetColumn(ctcSelected)->PutStoredWidth(0);
		}

		for each (long nCategoryID in m_aryInitialCategoryIDs)
		{			
			IRowSettingsPtr pRow = m_pdlCategoryTree->FindByColumn(ctcID, nCategoryID, m_pdlCategoryTree->GetFirstRow(), VARIANT_FALSE);
			if (pRow) {				
				pRow->PutValue(ctcSelected, g_cvarTrue);

				//make sure the parent rows are expanded
				IRowSettingsPtr pParentRow = pRow->GetParentRow();
				while (pParentRow) {
					pParentRow->PutExpanded(VARIANT_TRUE);
					pParentRow = pParentRow->GetParentRow();
				}
			}
			else {
				//why does this row not exist?
				ASSERT(FALSE);
			}
		}

		// (j.jones 2015-03-02 16:12) - PLID 64962 - if there is a default value, select that row
		if (m_nInitialDefaultCategoryID != -1) {
			m_pdlCategoryTree->SetSelByColumn(ctcID, m_nInitialDefaultCategoryID);
		}
		else {
			//otherwise set the selection to be the first checked row, if any
			m_pdlCategoryTree->SetSelByColumn(ctcSelected, g_cvarTrue);
		}

	}NxCatchAll("CCategorySelectDlg::OnInitDialog");

	return TRUE;
}

void CCategorySelectDlg::OnOK() 
{
	try {

		// (j.jones 2015-03-02 09:57) - PLID 64962 - reworked this dialog to support
		// multiple categories, we need to support returning all selected categories
		// and the default category (optional when multiple are used)
				
		if (!m_checkAllowMultiSelect.GetCheck()) {
			//if multiple categories are not enabled, use the selected row	
			IRowSettingsPtr pRow = m_pdlCategoryTree->GetCurSel();
			if (pRow == NULL) {
				MessageBox("Please select a category, or choose cancel if you don't want to select one.", "Practice", MB_ICONEXCLAMATION|MB_OK);
				return;
			}
			else {				
				m_nSelectedDefaultCategoryID = VarLong(pRow->GetValue(ctcID));
				m_arySelectedCategoryIDs.clear();
				m_arySelectedCategoryIDs.push_back(m_nSelectedDefaultCategoryID);
			}
		}
		else {
			//multiple categories are enabled, find all checked rows
			m_nSelectedDefaultCategoryID = -1;
			m_arySelectedCategoryIDs.clear();

			//this will fill m_aryCategoryIDs with the currently checked rows
			GatherSelectedCategories();
			
			if (m_arySelectedCategoryIDs.size() == 0) {
				MessageBox("Please select at least one category, or choose cancel if you don't want to select one.", "Practice", MB_ICONEXCLAMATION | MB_OK);
				return;
			}

			//if there is one category, it is the default
			if (m_arySelectedCategoryIDs.size() == 1) {
				m_nSelectedDefaultCategoryID = m_arySelectedCategoryIDs[0];
			}
			// (j.jones 2015-03-02 11:34) - PLID 64970 - if they selected multiple categories, prompt
			// for an optional default category
			else if (m_arySelectedCategoryIDs.size() > 1) {

				bool bContinue = true;
				while (bContinue) {

					bContinue = false;

					CMultiSelectDlg dlg(this, "DefaultServiceCategories");

					//pre-select the initial category ID, if it still exists in our selected categories
					long nDefaultCategoryID = -1;
					for each (long nCategoryID in m_arySelectedCategoryIDs)
					{
						if (m_nInitialDefaultCategoryID != -1 && m_nInitialDefaultCategoryID == nCategoryID) {
							nDefaultCategoryID = m_nInitialDefaultCategoryID;
						}
					}

					//-1 is allowed here, we will default to the <No Category Selected> row
					dlg.PreSelect(nDefaultCategoryID);

					CSqlFragment sql("(SELECT -1 AS ID, ' <No Default Category>' AS Name UNION SELECT ID, Name FROM CategoriesT WHERE ID IN ({INTVECTOR})) AS Q", m_arySelectedCategoryIDs);
					if (IDOK == dlg.Open(sql.Flatten(), "", "ID", "Name",
						FormatString("You have selected multiple categories for this %s. Please select a default category:", m_strItemType), 1, 1)) {

						CArray<long, long> arySelections;
						dlg.FillArrayWithIDs(arySelections);

						if (arySelections.GetSize() == 1) {
							m_nSelectedDefaultCategoryID = arySelections.GetAt(0);
						}
						else {
							//this should not be possible, we require one selection
							ASSERT(FALSE);
							m_nSelectedDefaultCategoryID = -1;
						}
					}
					else {
						//they cancelled - find out if they really meant to just have no default category
						if (IDNO == MessageBox("Are you sure you wish to save without a default category?", "Practice", MB_ICONQUESTION | MB_YESNO)) {
							//this will reopen the selection dialog
							bContinue = true;
							continue;
						}
						m_nSelectedDefaultCategoryID = -1;
					}
				}
			}
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CCategorySelectDlg::OnCancel() 
{
	try {
		
		// (j.jones 2015-03-02 09:57) - PLID 64962 - this no longer uses EndDialog
		CNxDialog::OnCancel();

	}NxCatchAll(__FUNCTION__);
}

void CCategorySelectDlg::OnAddNew() 
{
	try {

		// (z.manning, 03/07/2007) - PLID 24615 - Have the "Add New" button add a top level category.
		// (They can right click on categories to add children.)
		AddCategory(NULL);

	}NxCatchAll("CCategorySelectDlg::OnAddNew");
}

void CCategorySelectDlg::AddCategory(LPDISPATCH lpParentRow)
{
	// (z.manning, 03/07/2007) - PLID 24615 - Make sure we don't get too deep.
	IRowSettingsPtr pParentRow(lpParentRow);
	if(CountDepthOfRowFromTop(pParentRow) >= MAX_LEVELS) {
		MessageBox(FormatString("Your category tree may only be %li levels deep.", MAX_LEVELS));
		return;
	}

	// (z.manning, 03/07/2007) - PLID 24615 - Prompt for the name of the new category.
	// (z.manning 2011-01-24 15:32) - PLID 42193 - CategoriesT.Name now supports up to 100 characters.
	CString strName;
	while(strName.IsEmpty()) {
		if(InputBoxLimited(this, "Enter the new category name",strName,"",100,false,false,NULL) != IDOK) {
			return;
		}
		strName.TrimRight();
	}

	// (z.manning, 03/07/2007) - PLID 24615 - Check to see if this category already exists.
	if(ReturnsRecords("SELECT Name FROM CategoriesT WHERE NAME = '%s'", _Q(strName))) {
		MessageBox("Duplicate category names are not allowed.");
		return;
	}

	// (z.manning, 03/07/2007) - PLID 24615 - Don't ask me why, but 0 is the default value for
	// CategoriesT.Parent.
	long nParentID = 0;
	if(pParentRow != NULL) {
		nParentID = VarLong(pParentRow->GetValue(ctcID), 0);
	}

	// (z.manning, 03/07/2007) - PLID 24615 - Ok, let's add the category to data.
	CString strInsertCategory = FormatString(
		"SET NOCOUNT ON  \r\n"
		"DECLARE @nNewCategoryID int  \r\n"
		"SET @nNewCategoryID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM CategoriesT)  \r\n"
		"INSERT INTO CategoriesT (ID, Name, Parent)  \r\n"
		"VALUES (@nNewCategoryID, '%s', %li)  \r\n"
		"SET NOCOUNT OFF  \r\n"
		"SELECT @nNewCategoryID AS NewCategoryID "
		, _Q(strName), nParentID);
	ADODB::_RecordsetPtr prsInsert = GetRemoteData()->Execute(_bstr_t(strInsertCategory), NULL, ADODB::adCmdText);

	// (z.manning, 03/07/2007) - PLID 24615 - Add the new row in the tree.
	IRowSettingsPtr pNewRow = m_pdlCategoryTree->GetNewRow();
	pNewRow->PutValue(ctcID, AdoFldLong(prsInsert, "NewCategoryID"));
	// (j.jones 2015-03-02 16:28) - PLID 64962 - select this new category
	pNewRow->PutValue(ctcSelected, g_cvarTrue);
	pNewRow->PutValue(ctcName, _bstr_t(strName));
	m_pdlCategoryTree->AddRowSorted(pNewRow, lpParentRow);
	m_pdlCategoryTree->PutCurSel(pNewRow);

	// (j.jones 2011-06-17 13:29) - PLID 40091 - send a tablechecker
	CClient::RefreshTable(NetUtils::CPTCategories, AdoFldLong(prsInsert, "NewCategoryID"));
}

void CCategorySelectDlg::OnDblClickCellCategoriesTree(LPDISPATCH lpRow, short nColIndex) 
{
	try {

		// (z.manning, 03/07/2007) - PLID 24615 - Make sure we have a selection.
		if(m_pdlCategoryTree->GetCurSel() == NULL) {
			return;
		}

		// (z.manning, 03/07/2007) - PLID 24615 - Double clicking will commit the selected row.
		OnOK();
		
	}NxCatchAll("CCategorySelectDlg::OnDblClickCellCategoriesTree");
}

// (z.manning, 03/07/2007) - PLID 24615 - Pop up a menu with options for the category tree.
void CCategorySelectDlg::OnRButtonDownCategoriesTree(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		long nCategoryID = -1;
		if(pRow != NULL) {
			m_pdlCategoryTree->PutCurSel(pRow);
			nCategoryID = VarLong(pRow->GetValue(ctcID));
		}

		enum CategoryTreeMenuOptions
		{
			ctmoAdd = 1,
			ctmoDelete,
			ctmoRename,
			ctmoUnselect,
		};

		CMenu mnuPopup;
		mnuPopup.CreatePopupMenu();
		UINT nEnable;
		if(pRow == NULL) {
			mnuPopup.AppendMenu(MF_ENABLED, ctmoAdd, "Add");
			nEnable = MF_GRAYED;
		}
		else {
			mnuPopup.AppendMenu(MF_ENABLED, ctmoAdd, "Add child below");
			nEnable = MF_ENABLED;
		}
		mnuPopup.AppendMenu(nEnable, ctmoDelete, "Delete");
		mnuPopup.AppendMenu(nEnable, ctmoRename, "Rename");
		mnuPopup.AppendMenu(nEnable, ctmoUnselect, "Unselect");

		CPoint ptScreen, ptClient;
		GetCursorPos(&ptScreen);
		ptClient = ptScreen;
		ScreenToClient(&ptClient);

		switch(mnuPopup.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_LEFTBUTTON|TPM_RETURNCMD, ptScreen.x, ptScreen.y, this))
		{
			case ctmoAdd:
				AddCategory(pRow);
				break;

			case ctmoDelete:
				DeleteCategory(pRow);
				break;

			case ctmoRename:
				PromptRenameCategory(pRow);
				break;

			case ctmoUnselect:
				m_pdlCategoryTree->PutCurSel(NULL);
				break;
				
			default:
				break;
		}

	}NxCatchAll("CCategorySelectDlg::OnRButtonDownCategoriesTree");
}

// (z.manning, 03/07/2007) - PLID 24615 - Delete the given category so long as it's safe to do so.
void CCategorySelectDlg::DeleteCategory(LPDISPATCH lpRow)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		ASSERT(FALSE);
		return;
	}

	long nID = VarLong(pRow->GetValue(ctcID));

	// (j.jones 2015-03-19 08:47) - PLID 64974 - this now finally checks to see if it's in use on a charge
	if (ReturnsRecordsParam("SELECT ID FROM ChargesT WHERE Category = {INT} OR SubCategory = {INT}", nID, nID)) {
		MessageBox("This category is in use on at least one charge, and cannot be deleted.", "Practice", MB_ICONINFORMATION | MB_OK);
		return;
	}
	// (s.tullis 2015-04-01 16:11) - PLID 64978 - Check Emr Charges
	if (ReturnsRecordsParam("SELECT ID FROM EMRChargesT WHERE Category = {INT} ", nID)) {
		MessageBox("This category is in use on at least one EMR charge, and cannot be deleted.", "Practice", MB_ICONINFORMATION | MB_OK);
		return;
	}
	if (ReturnsRecordsParam("SELECT ID FROM CategoriesT WHERE Parent = {INT}", nID)) {
		MessageBox("This category has children. Please delete or move the children of this category before deleting the parent.", "Practice", MB_ICONINFORMATION | MB_OK);
		return;
	}

	// (j.jones 2015-02-26 10:52) - PLID 65063 - this now also checks ServiceMultiCategoryT
	if (ReturnsRecordsParam("SELECT ID FROM ServiceT WHERE Category = {INT}", nID)
		|| ReturnsRecordsParam("SELECT ServiceID FROM ServiceMultiCategoryT WHERE CategoryID = {INT}", nID)) {
		if(IDYES != MessageBox("There are products or service codes associated with this category, are you SURE you wish to delete it?","Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}
	}

	// (j.gruber 2008-07-21 16:39) - PLID 30695 - check if they exist in report
	if (ReturnsRecordsParam("SELECT ID FROM ReportSegmentDetailsT WHERE SegmentID IN (SELECT ID FROM ReportSegmentsT WHERE TypeID = 1) AND ItemID = {INT}", nID)) {
		if (IDYES != MessageBox("There are report segments associated with this category, are you SURE you wish to delete it?", "Practice", MB_ICONQUESTION | MB_YESNO)) {
			return;
		}		
	}
	
	if (IDYES == MessageBox("Are you sure you want to delete this category?", "Practice", MB_ICONQUESTION | MB_YESNO))
	{
		// (j.jones 2008-03-25 16:16) - PLID 28152 - added tablechecker for products
		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ProductT.ID FROM ProductT "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"WHERE Category = {INT}", nID);
		if(!rs->eof) {
			//if any producs match, send one tablechecker, do not send the ID
			CClient::RefreshTable(NetUtils::Products);
		}
		
		ExecuteParamSql("DELETE FROM ReportSegmentDetailsT WHERE ItemID = {INT} AND SegmentID IN (SELECT ID FROM ReportSegmentsT WHERE TypeID = 1)", nID);
		// (j.jones 2015-02-26 10:52) - PLID 65063 - clear ServiceMultiCategoryT
		ExecuteParamSql("DELETE FROM ServiceMultiCategoryT WHERE CategoryID = {INT}", nID);
		// (j.jones 2015-02-26 10:52) - PLID 65063 - if the ServiceT record now has only one entry in ServiceMultiCategoryT, switch to it
		ExecuteParamSql("UPDATE ServiceT SET ServiceT.Category = ServiceMultiCategoryT.CategoryID "
			"FROM ServiceT "
			"INNER JOIN ServiceMultiCategoryT ON ServiceT.ID = ServiceMultiCategoryT.ServiceID "
			"WHERE ServiceT.Category = {INT} AND ServiceMultiCategoryT.CategoryID <> {INT} "
			"AND (SELECT Count(*) FROM ServiceMultiCategoryT WHERE ServiceID = ServiceT.ID AND CategoryID <> {INT}) = 1",
			nID, nID, nID);
		ExecuteParamSql("UPDATE ServiceT SET Category = NULL WHERE Category = {INT}", nID);
		ExecuteParamSql("DELETE FROM CategoriesT WHERE ID = {INT}", nID);

		m_pdlCategoryTree->RemoveRow(pRow);
		m_pdlCategoryTree->PutCurSel(NULL);

		// (j.jones 2011-06-17 13:29) - PLID 40091 - send a tablechecker
		CClient::RefreshTable(NetUtils::CPTCategories, nID);
	}
}

// (z.manning, 03/07/2007) - PLID 24615 - Prompt for a new category name and then save it.
void CCategorySelectDlg::PromptRenameCategory(LPDISPATCH lpRow)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		ASSERT(FALSE);
		return;
	}

	CString strOldName = VarString(pRow->GetValue(ctcName), "");

	CString strName;
	while(strName.IsEmpty()) {
		// (z.manning 2011-01-24 17:24) - PLID 42193 - Name can now be 100 chars
		if(InputBoxLimited(this, "Enter the new category name",strName,"",100,false,false,NULL) != IDOK) {
			return;
		}
	}

	if(strName == strOldName) {
		// (z.manning, 05/30/2007) - PLID 24615 - Name didn't change, so no need to do anything.
		return;
	}

	// (z.manning, 05/30/2007) - PLID 24615 - Make sure we don't have a duplicate name.
	long nID = VarLong(pRow->GetValue(ctcID));
	if(ReturnsRecords("SELECT Name FROM CategoriesT WHERE Name = '%s' AND ID <> %li", _Q(strName), nID)) {
		MessageBox(FormatString("There is already a category named '%s'. You may not have duplicate category names.",strName));
		return;
	}

	ExecuteSql("UPDATE CategoriesT SET Name = '%s' WHERE ID = %li", _Q(strName), nID);

	pRow->PutValue(ctcName, _bstr_t(strName));

	// (j.jones 2011-06-17 13:29) - PLID 40091 - send a tablechecker
	CClient::RefreshTable(NetUtils::CPTCategories, nID);
}

// (z.manning, 03/07/2007) - PLID 24615 - This code is based off of (that is to say almost an exact copy of) the
// code from the referral source dialog.
//////////////////////////////////////////////////////////
//														//
//		Code for drag/drop moving of categories			//
//														//
//////////////////////////////////////////////////////////
#define IDT_DRAG_HOVER	1000
#define HOVER_DELAY_MS	1000		//1 second of hover = create placeholder
#define CATEGORY_PLACEHOLDER_ID	-2	//ID of the placeholder rows.  Not a valid category ID.

void CCategorySelectDlg::OnDragBeginCategoriesTree(BOOL FAR* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags) 
{
	try {

		//If the user has not enabled drag/drop functionality, then cancel it out
		if(!IsDlgButtonChecked(IDC_CATEGORY_TREE_ENABLE_DRAG)) {
			*pbShowDrag = FALSE;
		}

		//Save this row for later use
		m_lpDraggingRow = lpRow;
		
	}NxCatchAll("CCategorySelectDlg::OnDragBeginCategoriesTree");
}

void CCategorySelectDlg::OnDragOverCellCategoriesTree(BOOL FAR* pbShowDrop, LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags) 
{
	try {

		//Clear the hover timer
		KillTimer(IDT_DRAG_HOVER);

		if(!IsDlgButtonChecked(IDC_CATEGORY_TREE_ENABLE_DRAG)) {
			*pbShowDrop = FALSE;
			return;
		}

		IRowSettingsPtr pDestRow(lpRow);
		IRowSettingsPtr pFromRow(lpFromRow);
		ClearDragPlaceholders(pDestRow);
		CString strReasonFailed;
		*pbShowDrop = IsValidDrag(pFromRow, pDestRow, strReasonFailed);

		if(*pbShowDrop) {
			//If the row we are dragging to has children, make sure it is expanded
			if(pDestRow != NULL) {
				if(pDestRow->GetFirstChildRow() != NULL) {
					pDestRow->Expanded = VARIANT_TRUE;
				}
				else {
					//This is a leaf node, perhaps they want to make it a parent node, so we'll start the timer for a placeholder.
					SetTimer(IDT_DRAG_HOVER, HOVER_DELAY_MS, NULL);
				}
			}
		}

	}NxCatchAll("CCategorySelectDlg::OnDragOverCellCategoriesTree");	
}

void CCategorySelectDlg::OnDragEndCategoriesTree(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags) 
{
	try {

		long nID = -1;

		//end the hover timer, if it exists
		KillTimer(IDT_DRAG_HOVER);

		//require the enable to be checked
		if(IsDlgButtonChecked(IDC_CATEGORY_TREE_ENABLE_DRAG)) {

			IRowSettingsPtr pDestRow(lpRow);
			IRowSettingsPtr pFromRow(lpFromRow);

			//Do not allow a drag onto no row (NULL)
			if(lpRow != NULL) {
				//Ensure this is valid to drag
				CString strReasonFailed;
				if(IsValidDrag(pFromRow, pDestRow, strReasonFailed)) {
					//See what our new parent will be
					IRowSettingsPtr pDestParent = pDestRow->GetParentRow();
                    
                    

					nID = VarLong(pFromRow->GetValue(ctcID), -1);

					// (a.wilson 2014-07-11 09:56) - PLID 59982 - changed default value to 0 from -1 since its not a valid value for this.
					long nParentID = 0;
					if(pDestParent)
						nParentID = VarLong(pDestParent->GetValue(ctcID));

					//DRT - NOTE:  We must ADD first, then DELETE second.  If you delete first, you will remove all the children from this
					//	row, and adding will then just bring over the root row.

					//Add the new row
					IRowSettingsPtr pNewRow = m_pdlCategoryTree->AddRowSorted(pFromRow, pDestParent);
                   
					//Remove the old row
					m_pdlCategoryTree->RemoveRow(pFromRow);

					// (a.wilson 2014-07-11 09:57) - PLID 59982 - this fix broke dragging primary and making them sub categories.
					// instead he should have changed the default value of nParentID from -1 to 0 since -1 is consider a valid parent in the report.
                    // (a.levy 2013-06-24 12:01) - PLID - 57286
                     //if(pNewRow->GetLastChildRow() == NULL) {
                     //    nParentID = 0;
                     //    
                     //}

					//Update the parent
					pNewRow->PutValue(ctcParentID, (long)nParentID);

					//Now update the database
					ExecuteSql("UPDATE CategoriesT SET Parent = %li WHERE ID = %li", nParentID, VarLong(pNewRow->GetValue(ctcID)));

					// (j.jones 2011-06-17 13:29) - PLID 40091 - send a tablechecker (for all categories)
					CClient::RefreshTable(NetUtils::CPTCategories, -1);
				}

				else {
					//Notify the user why it failed
					if(!strReasonFailed.IsEmpty()) {
						MessageBox(strReasonFailed);
					}
				}

				//Dragging is over, get rid of our placeholders
				ClearDragPlaceholders();
			}
		}


		//We need to fire the selection changing.  In most cases the selection will have remained
		//	the same (if you are moving a row down in hierarchy.  However, if you are moving it to 
		//	be a sibling, you will "drop" on the sibling, leaving the selection on said sibling.
		//We will only do this if the cursel is different than the dragging row
		IRowSettingsPtr pCurSel = m_pdlCategoryTree->CurSel;

		//clear the saved row for dragging
		m_lpDraggingRow = NULL;
		
	}NxCatchAll("CCategorySelectDlg::OnDragEndCategoriesTree");
}

void CCategorySelectDlg::ClearDragPlaceholders(NXDATALIST2Lib::IRowSettingsPtr pRowToPreserve /*= NULL*/)
{
	for(int i = m_aryDragPlaceholders.GetSize() - 1; i >= 0; i--) {
		IRowSettingsPtr pRow = m_aryDragPlaceholders.GetAt(i);
		if(pRow != pRowToPreserve) {
			m_pdlCategoryTree->RemoveRow(pRow);
			m_aryDragPlaceholders.RemoveAt(i);
		}
	}
}

BOOL CCategorySelectDlg::IsValidDrag(NXDATALIST2Lib::IRowSettingsPtr pFromRow, NXDATALIST2Lib::IRowSettingsPtr pDestRow, CString &strReasonForFailure)
{
	if(pFromRow == NULL || pDestRow == NULL) {
		strReasonForFailure = "Invalid parameters.";
		return FALSE;
	}

	//Must be enabled
	if(!IsDlgButtonChecked(IDC_CATEGORY_TREE_ENABLE_DRAG)) {
		strReasonForFailure = "You must enable drag/drop functionality by checking the box at the bottom of the category list.";
		return FALSE;
	}

	//Currently, the only invalid conditions are:
	//	a)  Dragging a parent to its child
	{
		//pTopmostFromRow will the highest parent of the "from"
		IRowSettingsPtr pTopmostFromRow = pFromRow;
		while(pTopmostFromRow->GetParentRow())
			pTopmostFromRow = pTopmostFromRow->GetParentRow();

		//pTopmostToRow will be the highest parent of the "to"
		IRowSettingsPtr pTopmostDestRow = pDestRow;
		while(pTopmostDestRow->GetParentRow()) {
			//If the row in the destination ever matches the row we're copying from, then we are trying to
			//	drag down in the tree, which is not allowed
			if(pTopmostDestRow == pFromRow) {
				//strReasonForFailure = "You may not move a category in such a way that it would become a child of itself.";
				return FALSE;
			}
			pTopmostDestRow = pTopmostDestRow->GetParentRow();
		}

		//We finally need to check at the very highest level, because the top-level rows have no parents (but not if both are NULL)
		if(pTopmostDestRow == pFromRow) {
			//strReasonForFailure = "You may not move a category into a child of itself.";
			return FALSE;
		}
	}


	//	b)  Dragging a row such that the addition of the row + its children will exceed MAX_LEVELS
	{
		//This is the depth BELOW, not including our current row.
		long nCurrentDepth = CountDepthBelowRow(pFromRow);

		//If we are dropping onto a placeholder, that placeholder will be counted in the below function.
		//now for the destination, we need to now how deep it is, in reference to the top of the tree.
		long nDestDepth = CountDepthOfRowFromTop(pDestRow);

		//Add 'em up.  If we go over the max, then this isn't allowed.
		long nFinalDepth = nCurrentDepth + nDestDepth;
		if(nFinalDepth > MAX_LEVELS) {
			strReasonForFailure.Format("Your category tree may only be %li levels deep.", MAX_LEVELS);
			return FALSE;
		}
	}

	return TRUE;
}

void CCategorySelectDlg::OnTimer(UINT nIDEvent)
{
	try {

		switch(nIDEvent) 
		{
			case IDT_DRAG_HOVER:
				KillTimer(IDT_DRAG_HOVER);
				ASSERT(m_lpDraggingRow);
				if(m_lpDraggingRow) {
					//Only insert a placeholder if this row has no children.  If it has children, the
					//	user should drag it as a sibling.  Also ensure that we aren't hovering on a placeholder
					IRowSettingsPtr pRow = m_pdlCategoryTree->GetCurSel();
					if(pRow->GetFirstChildRow() == NULL && VarLong(pRow->GetValue(ctcID)) != CATEGORY_PLACEHOLDER_ID) {
						IRowSettingsPtr pPH = InsertPlaceholder(pRow);
						m_aryDragPlaceholders.Add(pPH);
						pRow->Expanded = VARIANT_TRUE;
					}
				}
				break;
		}

	}NxCatchAll("CCategorySelectDlg::OnTimer");

	CDialog::OnTimer(nIDEvent);
}

NXDATALIST2Lib::IRowSettingsPtr CCategorySelectDlg::InsertPlaceholder(NXDATALIST2Lib::IRowSettingsPtr pParentRow)
{
	IRowSettingsPtr pRow = m_pdlCategoryTree->GetNewRow();
	pRow->PutValue(ctcID, (long)CATEGORY_PLACEHOLDER_ID);
	// (j.jones 2015-03-02 16:28) - PLID 64962 - fill the selected column with null
	pRow->PutValue(ctcSelected, g_cvarNull);
	pRow->PutValue(ctcName, _bstr_t(""));
	pRow->PutValue(ctcParentID, (long)-1);
	m_pdlCategoryTree->AddRowSorted(pRow, pParentRow);

	return pRow;
}
//////////////////////////////////////////////////////////
//														//
//		End code for drag/drop moving of categories		//
//														//
//////////////////////////////////////////////////////////

// (j.jones 2015-03-02 10:38) - PLID 64962 - added multi-select abilities
void CCategorySelectDlg::OnAllowMultiSelect()
{
	try {

		long nSelectedColWidth = 20;
		if (!m_checkAllowMultiSelect.GetCheck()) {

			//warn if multiple categories are selected
			GatherSelectedCategories();
			if (m_arySelectedCategoryIDs.size() > 0) {
				if (IDNO == MessageBox("You currently have multiple categories selected. "
					"Do you wish to clear your selections and allow only one category to be chosen?", "Practice", MB_ICONQUESTION | MB_YESNO)) {
					m_checkAllowMultiSelect.SetCheck(TRUE);
					return;
				}
				m_nSelectedDefaultCategoryID = -1;
				m_arySelectedCategoryIDs.clear();

				//clear the checks
				IRowSettingsPtr pRow = m_pdlCategoryTree->FindAbsoluteFirstRow(VARIANT_TRUE);
				while (pRow) {
					pRow->PutValue(ctcSelected, g_cvarFalse);
					pRow = m_pdlCategoryTree->FindAbsoluteNextRow(pRow, VARIANT_TRUE);
				}
			}
			else if (m_arySelectedCategoryIDs.size() == 1) {
				//select the single category row
				m_pdlCategoryTree->SetSelByColumn(ctcID, m_arySelectedCategoryIDs[0]);
			}

			nSelectedColWidth = 0;
		}
		m_pdlCategoryTree->GetColumn(ctcSelected)->PutStoredWidth(nSelectedColWidth);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-02 10:40) - PLID 64962 - re-fills m_aryCategoryIDs with
// the currently selected categories, if multiple are selected
void CCategorySelectDlg::GatherSelectedCategories()
{
	m_arySelectedCategoryIDs.clear();

	IRowSettingsPtr pRow = m_pdlCategoryTree->FindAbsoluteFirstRow(VARIANT_TRUE);
	while (pRow) {

		bool bChecked = VarBool(pRow->GetValue(ctcSelected), FALSE) ? true : false;
		if (bChecked) {
			m_arySelectedCategoryIDs.push_back(VarLong(pRow->GetValue(ctcID)));
		}

		pRow = m_pdlCategoryTree->FindAbsoluteNextRow(pRow, VARIANT_TRUE);
	}
}
