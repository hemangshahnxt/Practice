// DiscountsSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorRc.h"
#include "DiscountsSetupDlg.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CDiscountsSetupDlg dialog

// (j.gruber 2007-03-19 14:25) - PLID 25165 - created discounts tab and put category information on it
CDiscountsSetupDlg::CDiscountsSetupDlg(CWnd* pParent)
	: CNxDialog(CDiscountsSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDiscountsSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDiscountsSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDiscountsSetupDlg)
	DDX_Control(pDX, IDC_DISC_CAT_INACTIVE, m_inactivate_disc_cat);
	DDX_Control(pDX, IDC_DELETE_DISC_CAT, m_del_disc_cat);
	DDX_Control(pDX, IDC_ADD_DISC_CAT, m_add_disc_cat);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDiscountsSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CDiscountsSetupDlg)
	ON_BN_CLICKED(IDC_ADD_DISC_CAT, OnAddDiscCat)
	ON_BN_CLICKED(IDC_DELETE_DISC_CAT, OnDeleteDiscCat)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_DISC_CAT_INACTIVE, OnDiscCatInactive)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiscountsSetupDlg message handlers

void CDiscountsSetupDlg::OnAddDiscCat() 
{
	try { 
		//Pop up a box with for a new item
		CString strName;
		const _variant_t varFalse(VARIANT_FALSE, VT_BOOL);
		if (IDOK == InputBoxLimited(this, "Enter a new discount category:", strName, "",255,false,false,NULL)) {

			//make sure that this item doesn't exist already
			if (ReturnsRecords("SELECT ID FROM DiscountCategoriesT WHERE Description like '%s'", _Q(strName))) {

				MsgBox("This discount category already exists.  Please choose a different category name.");
			}
			else {

				strName.TrimLeft();
				strName.TrimRight();
				//make sure they actually entered a value
				if (strName.IsEmpty()) {
					MsgBox("You may not enter blank discount category names");
				}
				else {
				
					//woo hoo we can keep going
					long nID = NewNumber("DiscountCategoriesT", "ID");
					ExecuteSql("INSERT INTO DiscountCategoriesT (ID, Description) "
						" VALUES (%li, '%s')", nID, _Q(strName));

					//add it to the datalist
					NXDATALIST2Lib::IRowSettingsPtr pRow;
					pRow = m_pDiscountCatList->GetNewRow();

					pRow->PutValue(0, nID);
					pRow->PutValue(1, _variant_t(strName));
					pRow->PutValue(2, varFalse);

					m_pDiscountCatList->AddRowAtEnd(pRow, NULL);

					long nAuditID = BeginNewAuditEvent();
					// (j.gruber 2007-04-02 09:30) - PLID 25165 - changed the audit name to not be the current username
					AuditEvent(-1, "", nAuditID, aeiAdminDiscountCategoryAdded, nID, "", strName, 2, aetCreated);
				}
			}
		}
	}NxCatchAll("Error in CDiscountsSetupDlg::OnAddDiscCat() ");


	
}

void CDiscountsSetupDlg::OnDeleteDiscCat() 
{
	try {

		//get the row
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pDiscountCatList->GetCurSel();
		

		if (pRow == NULL) {
			MsgBox("Please select a discount category to delete");
		}
		else {

			long nCatID = VarLong(pRow->GetValue(0));
			//check to see if it is on any charges
			// (j.gruber 2009-03-25 16:19) - PLID 33360 - update discount structure
			if (ReturnsRecords("SELECT ID FROM ChargeDiscountsT WHERE DiscountCategoryID = %li", nCatID)) {

				MsgBox("This discount category has been used on charges.  You may not delete a category that has been used.");

				//TODO:: Check for surgeries also

			}
			// (j.gruber 2007-05-14 17:02) - PLID 25997 - make sure they don't exist on surgeries!
			// (j.gruber 2009-03-25 16:19) - PLID 33360 - change surgery discount structure
			else if (ReturnsRecords("SELECT SurgeryDetailID FROM SurgeryDetailDiscountsT WHERE DiscountCategoryID = %li", nCatID)) {
				MsgBox("This discount category has been used on at least one Surgery.  You may not delete a category that has been used.");
			}
			// (a.wetta 2007-05-15 13:17) - PLID 15998 - Check to see if the discount category is used on a sale
			else if (ReturnsRecords("select * from SalesT where DiscountCategoryID = %li", nCatID) &&
				MessageBox("This discount category has been selected on at least one sale.  If you delete this category "
							"it will be removed from any sales it is associated with.  Are you sure you want to do this?",
							NULL, MB_YESNO|MB_ICONQUESTION) != IDYES) {
				// The user has chosen not to continue
			}
			// (a.walling 2007-05-23 11:43) - PLID 26114 - Ensure it is not selected on any gift certificates
			else if (ReturnsRecords("SELECT ServiceID FROM GCTypesT WHERE DiscountCategoryID = %li", nCatID)) {
				MessageBox("This discount category is used when reward points are redeemed for a gift certificate. You may not delete a category that has been used.");
			}
			else {

				CString strName = VarString(pRow->GetValue(1), "");

				//let'em delete it!
				// (a.wetta 2007-05-15 13:19) - PLID 15998 - Remove the category from any sales
				ExecuteSql("update SalesT set DiscountCategoryID = NULL where DiscountCategoryID = %li", nCatID);
				ExecuteSql("DELETE FROM DiscountCategoriesT WHERE ID = %li", nCatID);

				long nAuditID = BeginNewAuditEvent();
				// (j.gruber 2007-04-02 09:30) - PLID 25165 - changed the audit name to not be the current username
				AuditEvent(-1, "", nAuditID, aeiAdminDiscountCategoryDeleted, nCatID, strName, "", 2, aetDeleted);

				//remove the row
				m_pDiscountCatList->RemoveRow(pRow);
			}
		}

	}NxCatchAll("Error in CDiscountsSetupDlg::OnDeleteDiscCat");
	
}


// (j.gruber 2007-04-02 09:40) - PLID 25165 - implement an update view 
void CDiscountsSetupDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{

	try {
	
		//first clear the list
		m_pDiscountCatList->Clear();

		//now reload it
		GetDlgItem(IDC_DISC_CAT_INACTIVE)->EnableWindow(FALSE);


		const _variant_t varTrue(VARIANT_TRUE, VT_BOOL);
		const _variant_t varFalse(VARIANT_FALSE, VT_BOOL);

		//load the datalist
		ADODB::_RecordsetPtr rs = CreateRecordset("SELECT ID, Description, Active FROM DiscountCategoriesT");
		while (! rs->eof) {

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiscountCatList->GetNewRow();

			pRow->PutValue(0, AdoFldLong(rs, "ID"));
			pRow->PutValue(1, _variant_t(AdoFldString(rs, "Description")));

			long nActive = AdoFldBool(rs, "Active");
			if (nActive == 0) {
				pRow->PutValue(2, varTrue);
				pRow->PutForeColor(RGB(128,128,128));
			}
			else {
				pRow->PutValue(2, varFalse);
				pRow->PutForeColor(RGB(0,0,0));
				
				
			}

			m_pDiscountCatList->AddRowAtEnd(pRow, NULL);
			rs->MoveNext();
		}


		// (a.wetta 2007-03-29 13:05) - PLID 25407 - Make the dialog sizeable
		SetControlPositions();
	}NxCatchAll("Error In CDiscountsSetupDlg::UpdateView()");

}

BOOL CDiscountsSetupDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		m_add_disc_cat.AutoSet(NXB_NEW);
		m_del_disc_cat.AutoSet(NXB_DELETE);
		// (z.manning, 04/30/2008) - PLID 29852 - Set more button styles
		m_inactivate_disc_cat.AutoSet(NXB_MODIFY);
		m_btnClose.AutoSet(NXB_CLOSE);

		m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_ADMIN, 0)));

		//initialize the datalists
		m_pDiscountCatList = BindNxDataList2Ctrl(IDC_DISCOUNT_CATEGORIES, false);

		GetDlgItem(IDC_DISC_CAT_INACTIVE)->EnableWindow(FALSE);


		const _variant_t varTrue(VARIANT_TRUE, VT_BOOL);
		const _variant_t varFalse(VARIANT_FALSE, VT_BOOL);

		//load the datalist
		ADODB::_RecordsetPtr rs = CreateRecordset("SELECT ID, Description, Active FROM DiscountCategoriesT");
		while (! rs->eof) {

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiscountCatList->GetNewRow();

			pRow->PutValue(0, AdoFldLong(rs, "ID"));
			pRow->PutValue(1, _variant_t(AdoFldString(rs, "Description")));

			long nActive = AdoFldBool(rs, "Active");
			if (nActive == 0) {
				pRow->PutValue(2, varTrue);
				pRow->PutForeColor(RGB(128,128,128));
			}
			else {
				pRow->PutValue(2, varFalse);
				pRow->PutForeColor(RGB(0,0,0));
				
				
			}

			m_pDiscountCatList->AddRowAtEnd(pRow, NULL);
			rs->MoveNext();
		}


		// (a.wetta 2007-03-29 13:05) - PLID 25407 - Make the dialog sizeable
		SetControlPositions();
		
	}NxCatchAll("Error In CDiscountsSetupDlg::OnInitDialog() ");
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CDiscountsSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CDiscountsSetupDlg)
	ON_EVENT(CDiscountsSetupDlg, IDC_DISCOUNT_CATEGORIES, 10 /* EditingFinished */, OnEditingFinishedDiscountCategories, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CDiscountsSetupDlg, IDC_DISCOUNT_CATEGORIES, 9 /* EditingFinishing */, OnEditingFinishingDiscountCategories, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CDiscountsSetupDlg, IDC_DISCOUNT_CATEGORIES, 16 /* SelChosen */, OnSelChosenDiscountCategories, VTS_DISPATCH)
	ON_EVENT(CDiscountsSetupDlg, IDC_DISCOUNT_CATEGORIES, 2 /* SelChanged */, OnSelChangedDiscountCategories, VTS_DISPATCH VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CDiscountsSetupDlg::OnEditingFinishedDiscountCategories(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{

	try {

		// (j.gruber 2007-04-02 09:32) - PLID 25378 - add active/inactive statuses
		switch (nCol) {
			case 1: //description
				if (bCommit) {

					NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
					long nID = VarLong(pRow->GetValue(0));


					//get the old value
					CString strOld;
					ADODB::_RecordsetPtr rs = CreateRecordset("SELECT Description FROM DiscountCategoriesT WHERE ID = %li", nID);
					if (!rs->eof) {
						strOld = AdoFldString(rs, "Description");
					}
					
					//woo hoo we can keep going
					CString strNewValue = VarString(varNewValue);
					ExecuteSql("UPDATE DiscountCategoriesT SET Description = '%s' WHERE ID = %li", _Q(strNewValue), nID);
					
					
					long nAuditID = BeginNewAuditEvent();
					// (j.gruber 2007-04-02 09:30) - PLID 25165 - changed the audit name to not be the current username
					AuditEvent(-1, strOld, nAuditID, aeiAdminDiscountCategoryRenamed, nID, strOld, strNewValue, 2, aetChanged);
				}
			break;

			// (j.gruber 2007-04-02 09:32) - PLID 25378 - add active/inactive statuses
			case 2: //active status

				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				
				if (pRow) {
					
					BOOL bInactive = VarBool(varOldValue);

					ChangeActiveStatus (pRow, bInactive);
				}
			break;
		}

	}NxCatchAll("Error in CDiscountsSetupDlg::OnEditingFinishedDiscountCategories");
	
	
}

void CDiscountsSetupDlg::OnEditingFinishingDiscountCategories(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		switch (nCol) {

			case 1:

				CString strNewValue, strOldValue;

				strNewValue = strUserEntered;
				strOldValue = VarString(varOldValue);

				strNewValue.TrimLeft();
				strNewValue.TrimRight();

				strOldValue.TrimRight();
				strOldValue.TrimLeft();

				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				long nID = VarLong(pRow->GetValue(0));
				
				//check if they are cancelling or not
				if ((*pbCommit)) {

					//make sure that this item doesn't exist already
					if (ReturnsRecords("SELECT ID FROM DiscountCategoriesT WHERE Description like '%s'", _Q(strUserEntered))) {

						//check to see if its the exact same because maybe they are just changing capitalization or the like
						if (strNewValue.CompareNoCase(strOldValue) != 0) {

							MsgBox("This discount category already exists.  Please choose a different category name.");
							*pbContinue = FALSE;
						}
					}

					//make sure its not over 255
					if (strNewValue.GetLength() > 255) {
						MsgBox("Please enter a description less then 255 characters in length.");
						*pbContinue = FALSE;
					}
				
						
					//make sure they actually entered a value
					if (strNewValue.IsEmpty()) {
						MsgBox("You may not enter blank discount category names");
						*pbContinue = FALSE;
					}

					// (z.manning, 08/06/2007) - PLID 26963 - Warn about every that this category is used.
					// (j.gruber 2009-03-25 16:21) - PLID 33360 - change discount structures
					ADODB::_RecordsetPtr prsCounts = CreateRecordset(
						"SELECT \r\n"
						"	(SELECT COUNT(*) FROM ChargeDiscountsT WHERE DiscountCategoryID = %li) AS ChargeCount, \r\n"
						"	(SELECT COUNT(*) FROM SurgeryDetailDiscountsT WHERE DiscountCategoryID = %li) AS SurgeryCount, \r\n" 
						"	(SELECT COUNT(*) FROM SalesT WHERE DiscountCategoryID = %li) AS SaleCount, \r\n"
						"	(SELECT COUNT(*) FROM GCTypesT WHERE DiscountCategoryID = %li) AS GCCount \r\n"
						, nID, nID, nID, nID);
					
					long nChargeCount = AdoFldLong(prsCounts, "ChargeCount", 0);
					long nSurgeryCount = AdoFldLong(prsCounts, "SurgeryCount", 0);
					long nSaleCount = AdoFldLong(prsCounts, "SaleCount", 0);
					long nGCCount = AdoFldLong(prsCounts, "GCCount", 0);

					// (z.manning, 08/06/2007) - PLID 26963 - If any of the counts are non zero, then
					// let's warn the user.
					if(_variant_t(varOldValue) != *pvarNewValue)
					{
						if(nChargeCount != 0 || nSurgeryCount != 0 || nSaleCount != 0 || nGCCount != 0)
						{
							CString strWarning = "Changing the name of this discount category will change it in all of the following places:\r\n\r\n";
							if(nChargeCount > 0) {
								strWarning += FormatString(" -- %li charge(s) \r\n", nChargeCount);
							}
							if(nSurgeryCount > 0) {
								strWarning += FormatString(" -- %li surgery charge(s) \r\n", nSurgeryCount);
							}
							if(nSaleCount > 0) {
								strWarning += FormatString(" -- %li sale(s) \r\n", nSaleCount);
							}
							if(nGCCount > 0) {
								strWarning += FormatString(" -- %li gift certificate type(s) \r\n", nGCCount);
							}
							strWarning += "\r\nAre you sure you want to rename this category?";
							
							if(MessageBox(strWarning, NULL, MB_YESNO) != IDYES) {
								*pbCommit = FALSE;
							}
						}
					}
				}
			break;
		}

	}NxCatchAll("Error in CDiscountsSetupDlg::OnEditingFinishingDiscountCategories");
	
}

void CDiscountsSetupDlg::OnSize(UINT nType, int cx, int cy) 
{
	CNxDialog::OnSize(nType, cx, cy);
	
	// (a.wetta 2007-03-29 13:05) - PLID 25407 - Make the dialog sizeable
	//since the dialog isnis a popup, we don't need this anymore
	//SetControlPositions();
	//Invalidate();
		
}

// (j.gruber 2007-04-02 09:32) - PLID 25378 - add active/inactive statuses
void CDiscountsSetupDlg::ChangeActiveStatus(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bInActive) {

	try {
	

		const _variant_t varTrue(VARIANT_TRUE, VT_BOOL);
		const _variant_t varFalse(VARIANT_FALSE, VT_BOOL);

		long nDiscountID = VarLong(pRow->GetValue(0), -1);
		
		if (bInActive) {

			//inactive, change it to be active

			ExecuteSql("UPDATE DiscountCategoriesT SET Active = 1 WHERE ID = %li", nDiscountID);
			pRow->PutValue(2, varFalse);

			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, VarString(pRow->GetValue(1), ""), nAuditID, aeiAdminDiscountCategoryActiveStatusChanged, nDiscountID, "Inactive", "Active", 2, aetChanged);

			//set the forground to be normal
			pRow->PutForeColor(RGB(0,0,0));
			SetDlgItemText(IDC_DISC_CAT_INACTIVE, "Inactivate");
		}
		else {

			//active, change it to be inactive

			ExecuteSql("UPDATE DiscountCategoriesT SET Active = 0 WHERE ID = %li", nDiscountID);
			pRow->PutValue(2, varTrue);

			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, VarString(pRow->GetValue(1), ""), nAuditID, aeiAdminDiscountCategoryActiveStatusChanged, nDiscountID, "Active", "Inactive", 2, aetChanged);
			
			//set the forground to be gray
			pRow->PutForeColor(RGB(128,128,128));
			SetDlgItemText(IDC_DISC_CAT_INACTIVE, "Activate");
		}
		

	}NxCatchAll("Error in CDiscountsSetupDlg::ChangeActiveStatus ");


}

// (j.gruber 2007-04-02 09:32) - PLID 25378 - add active/inactive statuses
void CDiscountsSetupDlg::OnDiscCatInactive() 
{
	try {
		//we aren't going to warn them or anything
		//check to make sure they selected something
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pDiscountCatList->CurSel;

		if (pRow) {

			BOOL bInactive = VarBool(pRow->GetValue(2));

			ChangeActiveStatus(pRow, bInactive);

		}
		else {
			MsgBox("Select a row to inactivate");
		}
	}NxCatchAll("Error In CDiscountsSetupDlg::OnDiscCatInactive()");
	
}

void CDiscountsSetupDlg::OnSelChosenDiscountCategories(LPDISPATCH lpRow) 
{

}

// (j.gruber 2007-04-02 09:32) - PLID 25378 - add active/inactive statuses
void CDiscountsSetupDlg::OnSelChangedDiscountCategories(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);
		if (pRow) {

			BOOL bInActive = VarBool(pRow->GetValue(2));
			GetDlgItem(IDC_DISC_CAT_INACTIVE)->EnableWindow(TRUE);

			if (bInActive) {
				SetDlgItemText(IDC_DISC_CAT_INACTIVE, "Activate");
			}
			else {
				SetDlgItemText(IDC_DISC_CAT_INACTIVE, "Inactivate");
			}
		}
		else {
			GetDlgItem(IDC_DISC_CAT_INACTIVE)->EnableWindow(FALSE);
		}
	}NxCatchAll("Error in CDiscountsSetupDlg::OnSelChangedDiscountCategories");
}

// (a.wetta 2007-05-16 09:59) - PLID 25163 - Now that this is a popup dialog, it needs to be able to be closed
void CDiscountsSetupDlg::OnClose() 
{
	
	CNxDialog::OnCancel();
}

// (j.gruber 2007-05-21 12:49) - PLID 26021 - added since we are a pop up dialog now
void CDiscountsSetupDlg::OnOK() 
{

	//we already saved everything, so we are good
	
	CNxDialog::OnOK();
}

// (j.gruber 2007-05-21 12:49) - PLID 26021 - added since we are a pop up dialog now
void CDiscountsSetupDlg::OnCancel() 
{

	CNxDialog::OnCancel();
}
