// EditProductAdjustmentCategoriesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "inventoryrc.h"
#include "EditProductAdjustmentCategoriesDlg.h"
#include "DontShowDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//TES 6/24/2008 - PLID 26142 - Created
/////////////////////////////////////////////////////////////////////////////
// CEditProductAdjustmentCategoriesDlg dialog


CEditProductAdjustmentCategoriesDlg::CEditProductAdjustmentCategoriesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditProductAdjustmentCategoriesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditProductAdjustmentCategoriesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditProductAdjustmentCategoriesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditProductAdjustmentCategoriesDlg)
	DDX_Control(pDX, IDC_EDIT_ADJUSTMENT_CATEGORY, m_nxbEdit);
	DDX_Control(pDX, IDC_DELETE_CATEGORY, m_nxbDelete);
	DDX_Control(pDX, IDC_CLOSE_PRODUCT_ADJUSTMENT_CATEGORIES, m_nxbClose);
	DDX_Control(pDX, IDC_ADD_CATEGORY, m_nxbAdd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditProductAdjustmentCategoriesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditProductAdjustmentCategoriesDlg)
	ON_BN_CLICKED(IDC_ADD_CATEGORY, OnAddCategory)
	ON_BN_CLICKED(IDC_EDIT_ADJUSTMENT_CATEGORY, OnEditCategory)
	ON_BN_CLICKED(IDC_DELETE_CATEGORY, OnDeleteCategory)
	ON_BN_CLICKED(IDC_CLOSE_PRODUCT_ADJUSTMENT_CATEGORIES, OnCloseProductAdjustmentCategories)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditProductAdjustmentCategoriesDlg message handlers

enum CategoryListColumns {
	clcID = 0,
	clcName = 1,
	clcInactive = 2,
};

BEGIN_EVENTSINK_MAP(CEditProductAdjustmentCategoriesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditProductAdjustmentCategoriesDlg)
	ON_EVENT(CEditProductAdjustmentCategoriesDlg, IDC_PRODUCT_ADJUSTMENT_CATEGORIES, 2 /* SelChanged */, OnSelChangedProductAdjustmentCategories, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CEditProductAdjustmentCategoriesDlg, IDC_PRODUCT_ADJUSTMENT_CATEGORIES, 9 /* EditingFinishing */, OnEditingFinishingProductAdjustmentCategories, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEditProductAdjustmentCategoriesDlg, IDC_PRODUCT_ADJUSTMENT_CATEGORIES, 10 /* EditingFinished */, OnEditingFinishedProductAdjustmentCategories, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

using namespace NXDATALIST2Lib;

BOOL CEditProductAdjustmentCategoriesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		m_nxbAdd.AutoSet(NXB_NEW);
		m_nxbEdit.AutoSet(NXB_MODIFY);
		m_nxbDelete.AutoSet(NXB_DELETE);
		m_nxbClose.AutoSet(NXB_CLOSE);

		m_pCategories = BindNxDataList2Ctrl(IDC_PRODUCT_ADJUSTMENT_CATEGORIES);

		EnableButtons();

	}NxCatchAll("Error in CEditProductAdjustmentCategoriesDlg::OnInitDialog()");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditProductAdjustmentCategoriesDlg::OnSelChangedProductAdjustmentCategories(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		//TES 6/24/2008 - PLID 26142 - Enable/disable the appropriate buttons.
		EnableButtons();
	}NxCatchAll("Error in CEditProductAdjustmentCategoriesDlg::OnSelChangedProductAdjustmentCategories()");
}

void CEditProductAdjustmentCategoriesDlg::OnEditingFinishingProductAdjustmentCategories(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		if(*pbCommit) {
			IRowSettingsPtr pEditedRow(lpRow);
			if(pEditedRow == NULL) return;

			if(nCol == clcName) {
				//TES 6/24/2008 - PLID 26142 - Ensure that this is a valid name.
				CString strNewVal = strUserEntered;
				if(strNewVal.CompareNoCase("<None>") == 0) {
					MsgBox("'<None>' is a reserved system name, and cannot be assigned to a category.");
					*pbContinue = FALSE;
					return;
				}
				else if(strNewVal.IsEmpty()) {
					MsgBox("You cannot enter an empty category name.");
					*pbContinue = FALSE;
					return;
				}

				IRowSettingsPtr pRow = m_pCategories->GetFirstRow();
				while(pRow) {
					if(pRow != pEditedRow && VarString(pRow->GetValue(clcName)).CompareNoCase(strNewVal) == 0) {
						MsgBox("The name you have entered is already in use by another category.  Please enter a unique name.");
						*pbContinue = FALSE;
						return;
					}
					pRow = pRow->GetNextRow();
				}
			}
			else if(nCol == clcInactive) {
				//TES 6/24/2008 - PLID 26142 - Confirm that they really want to do this.
				if(VarBool(*pvarNewValue)) {
					if(IDYES != MsgBox(MB_YESNO, "Inactive categories will no longer be available to assign to inventory adjustments.  Are you sure you wish to mark this category Inactive?")) {
						*pbContinue = FALSE;
						return;
					}
				}
				else {
					if(IDYES != MsgBox(MB_YESNO, "Activating this category will make it available for future inventory adjustments.  Are you sure you wish to re-activate this category?")) {
						*pbContinue = FALSE;
						return;
					}
				}
			}
		}
	}NxCatchAll("Error in CEditProductAdjustmentCategoriesDlg::OnEditingFinishingProductAdjustmentCategories()");

}

void CEditProductAdjustmentCategoriesDlg::OnEditingFinishedProductAdjustmentCategories(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		if(!bCommit) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//TES 6/24/2008 - PLID 26142 - We've already validated what they're doing, so go ahead and commit it to data.
		switch(nCol) {
		case clcName:
			{
				long nID = VarLong(pRow->GetValue(clcID));
				CString strNewName = VarString(varNewValue);
				ExecuteSql("UPDATE ProductAdjustmentCategoriesT SET Name = '%s' WHERE ID = %li", _Q(strNewName), nID);
				//TES 6/27/2008 - PLID 30523 - Update the permission for this category to display the new name.
				UpdateUserDefinedPermissionName(bioIndivAdjustmentCategories, nID, strNewName);
			}
			break;
		case clcInactive:
			ExecuteSql("UPDATE ProductAdjustmentCategoriesT SET Inactive = %i WHERE ID = %li", VarBool(varNewValue) ? 1 : 0, VarLong(pRow->GetValue(clcID)));
			break;
		}

	}NxCatchAll("Error in CEditProductAdjustmentCategoriesDlg::OnEditingFinishedProductAdjustmentCategories()");
}

void CEditProductAdjustmentCategoriesDlg::OnAddCategory() 
{
	try {
		//TES 6/24/2008 - PLID 26142 - Get a new name to add.
		CString strCatName;
		int nResult = InputBoxLimitedWithParent(this, "Please enter a name for the new category", strCatName, "", 255, false, false, NULL);
		if(nResult != IDOK) {
			return;
		}

		//TES 6/24/2008 - PLID 26142 - Validate it.
		if(strCatName.CompareNoCase("<None>") == 0) {
			MsgBox("'<None>' is a reserved system name, and cannot be assigned to a category.");
			return;
		}
		else if(strCatName.IsEmpty()) {
			MsgBox("You cannot enter an empty category name.");
			return;
		}

		IRowSettingsPtr pRow = m_pCategories->GetFirstRow();
		while(pRow) {
			if(VarString(pRow->GetValue(clcName)).CompareNoCase(strCatName) == 0) {
				MsgBox("The name you have entered is already in use by another category.  Please enter a unique name.");
				return;
			}
			pRow = pRow->GetNextRow();
		}

		//TES 6/24/2008 - PLID 26142 - OK, we've got a valid name, add it to data.
		long nID = NewNumber("ProductAdjustmentCategoriesT", "ID");
		//TES 7/9/2008 - PLID 30522 - If the only entries in the table are the system records, then NewNumber will
		// return a negative number, so make sure we only add positive IDs to the data.
		if(nID < 0) nID = 1;
		ExecuteSql("INSERT INTO ProductAdjustmentCategoriesT (ID, Name) VALUES (%li, '%s')", nID, _Q(strCatName));

		//TES 6/27/2008 - PLID 30523 - Now give everybody sptCreate permissions for this category.  We do this because if they 
		// don't have permission, and if they don't but are later given it, we don't want them to have to then go through
		// and be given permission for each category as well.
		AddUserDefinedPermission(nID, sptCreate|sptCreateWithPass, strCatName, "Controls ability to create Inventory Adjustments with this category.", 
			bioIndivAdjustmentCategories, sptCreate);

		//TES 6/24/2008 - PLID 26142 - And add it to the on-screen list.
		IRowSettingsPtr pNewRow = m_pCategories->GetNewRow();
		pNewRow->PutValue(clcID, nID);
		pNewRow->PutValue(clcName, _bstr_t(strCatName));
		pNewRow->PutValue(clcInactive, g_cvarFalse);
		m_pCategories->AddRowSorted(pNewRow, NULL);

		//TES 6/27/2008 - PLID 30523 - Now make sure the user knows that this was defaulted to give permission (this has the
		// added bonus of ensuring that our users are aware this feature exists).
		DontShowMeAgain(this, "By default, all users with permissions to create Inventory Adjustments have been granted permission "
			"to create adjustments with this category.  If you wish to restrict users from creating adjustments with this category, "
			"edit their Inventory Module->Inventory Item->Inventory Adjustment Categories permissions appropriately.", "InvPostAddAdjCat", "");

	}NxCatchAll("Error in CEditProductAdjustmentCategoriesDlg::OnAddCategory()");
}

void CEditProductAdjustmentCategoriesDlg::OnEditCategory() 
{
	try {
		//TES 6/24/2008 - PLID 26142 - If they have a row highlighted, let them edit the name.
		if(m_pCategories->CurSel != NULL) {
			m_pCategories->StartEditing(m_pCategories->CurSel, clcName);
		}
	}NxCatchAll("Error in CEditProductAdjustmentCategoriesDlg::OnEditCategory()");
}

void CEditProductAdjustmentCategoriesDlg::OnDeleteCategory() 
{
	try {
		if(m_pCategories->CurSel == NULL) {
			return;
		}

		//TES 6/24/2008 - PLID 26142 - Can they delete this?
		long nID = VarLong(m_pCategories->CurSel->GetValue(clcID));
		if(ReturnsRecords("SELECT ID FROM ProductAdjustmentsT WHERE ProductAdjustmentCategoryID = %li", nID)) {
			MsgBox("This category has already been assigned to at least one inventory adjustment.  It cannot be deleted.");
			return;
		}
		else {
			//TES 6/24/2008 - PLID 26142 - Do they really want to?
			if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to delete this category?  This action is unrecoverable!")) {
				return;
			}
			else {
				//TES 6/24/2008 - PLID 26142 - OK, let's delete it.
				ExecuteSql("DELETE FROM ProductAdjustmentCategoriesT WHERE ID = %li", nID);
				m_pCategories->RemoveRow(m_pCategories->CurSel);

				//TES 6/27/2008 - PLID 30523 - Also delete the associated permissions.
				DeleteUserDefinedPermission(bioIndivAdjustmentCategories, nID, TRUE);
			}
		}
	}NxCatchAll("Error in CEditProductAdjustmentCategoriesDlg::OnDeleteCategory()");
}

void CEditProductAdjustmentCategoriesDlg::OnCloseProductAdjustmentCategories() 
{
	CNxDialog::OnOK();
}

void CEditProductAdjustmentCategoriesDlg::EnableButtons()
{
	//TES 6/24/2008 - PLID 26142 - If there's anything highlighted, they can edit or delete it (they can always add).
	if(m_pCategories->CurSel != NULL) {
		m_nxbEdit.EnableWindow(TRUE);
		m_nxbDelete.EnableWindow(TRUE);
	}
	else {
		m_nxbEdit.EnableWindow(FALSE);
		m_nxbDelete.EnableWindow(FALSE);
	}
}