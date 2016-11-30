// GCTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GCTypeDlg.h"
#include "InternationalUtils.h"
#include "GlobalFinancialUtils.h"
#include "DiscountsSetupDlg.h"
#include "AuditTrail.h"
#include "BillingRc.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CGCTypeDlg dialog

enum TypeColumns {
	tcID = 0,
	tcName,
	tcValue, // (r.gonet 2015-03-25 10:13) - PLID 65276 - Added a column to store the Value of the GC Type.
	tcPrice, // (r.gonet 2015-03-25 10:13) - PLID 65276 - Added a column to store the Price of the GC Type. Often the same as the Value.
	tcExpires,
	tcDays,
	tcRecharge,
	tcRedeemable,		// Redeemable for rewards?
	tcPoints,			// Point cost
	tcCategoryID,		// Discount category (can be null)
	tcActive,			// (j.jones 2009-06-22 11:00) - PLID 34226 - added active
};

enum DiscountCategoryTypeColumns {
	dctcID,
	dctcName
};

CGCTypeDlg::CGCTypeDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CGCTypeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGCTypeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bChanged = false;
	m_bNewRowDeleted = FALSE;
	m_nCurrentSel = -1;
	m_nRedeemCategoryID = -1;
	// (r.gonet 2015-03-25 10:13) - PLID 65276 - Initialize the sync control flag to false to prevent programmatic changes to
	// the Value field from being pushed to the Price field.
	m_bSyncPriceWithValue = false;
}


void CGCTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGCTypeDlg)
	DDX_Control(pDX, IDC_GC_TYPE_REWARDS_CHECK, m_btnRewardPoints);
	DDX_Control(pDX, IDC_GC_TYPE_RECHARGE, m_btnRechargeable);
	DDX_Control(pDX, IDC_GC_TYPE_EXP, m_btnDefaultDays);
	DDX_Control(pDX, IDC_GC_TYPE_NAME, m_nxeditGcTypeName);
	DDX_Control(pDX, IDC_GC_TYPE_VALUE_EDIT, m_nxeditGcTypeValue);
	DDX_Control(pDX, IDC_GC_TYPE_PRICE_EDIT, m_nxeditGcTypePrice);
	DDX_Control(pDX, IDC_GC_TYPE_DAYS, m_nxeditGcTypeDays);
	DDX_Control(pDX, IDC_GC_TYPE_REWARD_POINTS_EDIT, m_nxeditGcTypeRewardPointsEdit);
	DDX_Control(pDX, IDC_NEW_GC_TYPE, m_btnNewGCType);
	DDX_Control(pDX, IDC_DELETE_GC_TYPE, m_btnDeleteGCType);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CHECK_GC_TYPE_INACTIVE, m_checkInactive);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGCTypeDlg, CNxDialog)
	//{{AFX_MSG_MAP(CGCTypeDlg)
	ON_BN_CLICKED(IDC_GC_TYPE_EXP, OnGcTypeExp)
	ON_BN_CLICKED(IDC_DELETE_GC_TYPE, OnDeleteGcType)
	ON_BN_CLICKED(IDC_NEW_GC_TYPE, OnNewGcType)
	ON_EN_KILLFOCUS(IDC_GC_TYPE_VALUE_EDIT, OnKillfocusGcTypeValue)
	ON_EN_KILLFOCUS(IDC_GC_TYPE_PRICE_EDIT, OnKillfocusGcTypePrice)
	ON_EN_KILLFOCUS(IDC_GC_TYPE_DAYS, OnKillfocusGcTypeDays)
	ON_EN_KILLFOCUS(IDC_GC_TYPE_NAME, OnKillfocusGcTypeName)
	ON_EN_CHANGE(IDC_GC_TYPE_DAYS, OnChangeGcTypeDays)
	ON_EN_CHANGE(IDC_GC_TYPE_NAME, OnChangeGcTypeName)
	ON_EN_CHANGE(IDC_GC_TYPE_VALUE_EDIT, OnChangeGcTypeValue)
	ON_EN_CHANGE(IDC_GC_TYPE_PRICE_EDIT, OnChangeGcTypePrice)
	ON_BN_CLICKED(IDC_GC_TYPE_RECHARGE, OnGcTypeRecharge)
	ON_BN_CLICKED(IDC_GC_TYPE_EDIT_DISCOUNT_CATEGORIES, OnGcTypeEditDiscountCategories)
	ON_EN_CHANGE(IDC_GC_TYPE_REWARD_POINTS_EDIT, OnChangeGcTypeRewardPointsEdit)
	ON_BN_CLICKED(IDC_GC_TYPE_REWARDS_CHECK, OnGcTypeRewardsCheck)
	ON_EN_KILLFOCUS(IDC_GC_TYPE_REWARD_POINTS_EDIT, OnKillfocusGcTypeRewardPointsEdit)
	ON_BN_CLICKED(IDC_CHECK_GC_TYPE_INACTIVE, OnCheckGcTypeInactive)
	ON_EN_SETFOCUS(IDC_GC_TYPE_VALUE_EDIT, &CGCTypeDlg::OnEnSetfocusGcTypeValueEdit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGCTypeDlg message handlers

BOOL CGCTypeDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (c.haag 2008-05-01 17:19) - PLID 29876 - NxIconify buttons
		m_btnNewGCType.AutoSet(NXB_NEW);
		m_btnDeleteGCType.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pTypes = BindNxDataListCtrl(this, IDC_GC_TYPE_LIST, GetRemoteData(), true);
		m_pTypes->WaitForRequery(dlPatienceLevelWaitIndefinitely);

		// (a.walling 2007-05-23 10:39) - PLID 26114
		m_pDiscountCategories = BindNxDataList2Ctrl(this, IDC_GC_TYPE_REWARDS_CATEGORY_LIST, GetRemoteData(), true);

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiscountCategories->GetNewRow();
		pRow->PutValue(dctcID, _variant_t(long(-1), VT_I4));
		pRow->PutValue(dctcName, _variant_t(" <No Category>"));
		m_pDiscountCategories->AddRowSorted(pRow, NULL);

		//select the first item if there is one
		if(m_pTypes->GetRowCount()) {
			m_pTypes->PutCurSel(0);
			m_nCurrentSel = 0;
		}

		LoadCurrentSel();

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGCTypeDlg::OnOK() 
{
	try {
		if(!SaveIfNecessary())
			return;

		CNxDialog::OnOK();
	} NxCatchAll(__FUNCTION__);
}

void CGCTypeDlg::OnCancel()
{
	try {
		CNxDialog::OnCancel();
	} NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CGCTypeDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CGCTypeDlg)
	ON_EVENT(CGCTypeDlg, IDC_GC_TYPE_LIST, 1 /* SelChanging */, OnSelChangingGcTypeList, VTS_PI4)
	ON_EVENT(CGCTypeDlg, IDC_GC_TYPE_LIST, 16 /* SelChosen */, OnSelChosenGcTypeList, VTS_I4)
	ON_EVENT(CGCTypeDlg, IDC_GC_TYPE_REWARDS_CATEGORY_LIST, 20 /* TrySetSelFinished */, OnTrySetSelFinishedGcTypeRewardsCategoryList, VTS_I4 VTS_I4)
	ON_EVENT(CGCTypeDlg, IDC_GC_TYPE_REWARDS_CATEGORY_LIST, 1 /* SelChanging */, OnSelChangingGcTypeRewardsCategoryList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CGCTypeDlg, IDC_GC_TYPE_REWARDS_CATEGORY_LIST, 2 /* SelChanged */, OnSelChangedGcTypeRewardsCategoryList, VTS_DISPATCH VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CGCTypeDlg, IDC_GC_TYPE_LIST, 23, CGCTypeDlg::ChangeColumnSortFinishedGcTypeList, VTS_I2 VTS_BOOL VTS_I2 VTS_BOOL)
END_EVENTSINK_MAP()

void CGCTypeDlg::OnSelChangingGcTypeList(long FAR* nNewSel) 
{
	try {
	} NxCatchAll(__FUNCTION__);
}

void CGCTypeDlg::OnSelChosenGcTypeList(long nRow) 
{
	try {
		if(!SaveIfNecessary(false)) {
			//if the save failed, reset the selection
			m_pTypes->PutCurSel(m_nCurrentSel);
		}
		else {
			//save passed, setup the correct row as current

			// (r.galicki 2008-08-04 16:05) - PLID 30945
			//	In the event of a new type not being saved, the nRow choice may not be a valid choice anymore.
			//	Since nRow is obtained before the decision to save, the value may have to adjust.
			if(m_bNewRowDeleted) {
				//	If a new row was not saved, the indicies following the deleted new row are off by 1.
				if(nRow > m_nCurrentSel) {
					nRow--;
				}
				//reset flag (only need one adjustment).
				m_bNewRowDeleted = FALSE;
			}
			m_nCurrentSel = nRow;
			LoadCurrentSel();
		}
	} NxCatchAll(__FUNCTION__);
}

void CGCTypeDlg::OnGcTypeExp() 
{
	try {
		EnsureDays();

		m_bChanged = true;
	} NxCatchAll(__FUNCTION__);
}

void CGCTypeDlg::OnDeleteGcType() 
{
	try {
		long nSel = m_nCurrentSel;
		if(nSel == -1)
			return;

		if(MsgBox(MB_YESNO, "This will immediately delete the selected type.  Are you sure you wish to continue?") != IDYES)
			return;

		//get the ID number
		long nID = VarLong(m_pTypes->GetValue(nSel, tcID));
		CString strName = VarString(m_pTypes->GetValue(nSel, tcName));

		if(nID == -1) {
			//just do the normal stuff (below the else)
		}
		else {
			//this has been saved previously.  First check to see if any GC's rely on it
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Parameterized
			if(ReturnsRecordsParam("SELECT ID FROM GiftCertificatesT WHERE DefaultTypeID = {INT}", nID) || ReturnsRecordsParam("SELECT ID FROM ChargesT WHERE ServiceID = {INT}", nID)) {
				MsgBox("You have gift certificates which are linked to this type.  It cannot be deleted if gift certificates have been billed.");
				return;
			}

			// (a.walling 2007-07-27 09:00) - PLID 15998 - Check to see if it has a sale applied
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Parameterized.
			_RecordsetPtr rs = CreateParamRecordset("SELECT COUNT(*) AS SaleCount FROM SaleItemsT WHERE ServiceID = {INT} GROUP BY ServiceID", nID);
			//TES 5/9/2008 - PLID 29996 - Check for eof!
			if(!rs->eof) {
				long nSaleCount = AdoFldLong(rs, "SaleCount", 0);
				if(nSaleCount > 0) {
					CString strMessage;
					strMessage.Format("This gift certificate exists in %li sale%s. Deleting will remove its discount information from all sales. Would you like to continue deleting this code?", nSaleCount, nSaleCount > 0 ? "s" : "");
					if (IDNO == MessageBox(strMessage,"Practice",MB_YESNO|MB_ICONINFORMATION))
						return;
				}
			}

			// (c.haag 2009-10-12 13:01) - PLID 35722 - Don't delete if linked with MailSent items
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Parameterized.
			if(ReturnsRecordsParam("SELECT ID FROM MailSentServiceCptT WHERE ServiceID = {INT}", nID)) {
				MsgBox("You have patient photos which are linked to this type. It therefore cannot be deleted.");
				return;
			}

			//wipe out the record
			// (a.wetta 2007-03-30 10:22) - PLID 24872 - Also delete any rule links associated with the service code
			// (a.walling 2007-07-27 09:38) - PLID 15998 - Remove any sale info, also batch all this stuff
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Parameterized.
			CParamSqlBatch sqlBatch;
			sqlBatch.Add("DELETE FROM SaleItemsT WHERE ServiceID = {INT}", nID);
			sqlBatch.Add("DELETE FROM CommissionRulesLinkT WHERE ServiceID = {INT}", nID);
			sqlBatch.Add("DELETE FROM CommissionT WHERE ServiceID = {INT}", nID);	//if it's associated with a commission, just delete the commission

			// (j.jones 2007-10-18 08:30) - PLID 27757 - handle deleting anesth/facility setups, even though
			// they are never linked to GCs, but they are linked to ServiceT, so do it for posterity
			sqlBatch.Add("DELETE FROM LocationFacilityFeesT WHERE FacilityFeeSetupID IN (SELECT ID FROM FacilityFeeSetupT WHERE ServiceID = {INT})", nID);
			sqlBatch.Add("DELETE FROM FacilityFeeSetupT WHERE ServiceID = {INT}", nID);
			sqlBatch.Add("DELETE FROM LocationAnesthesiaFeesT WHERE AnesthesiaSetupID IN (SELECT ID FROM AnesthesiaSetupT WHERE ServiceID = {INT})", nID);
			sqlBatch.Add("DELETE FROM AnesthesiaSetupT WHERE ServiceID = {INT}", nID);

			// (j.jones 2010-08-03 08:58) - PLID 39912 - clear out InsCoServicePayGroupLinkT
			sqlBatch.Add("DELETE FROM InsCoServicePayGroupLinkT WHERE ServiceID = {INT}", nID);

			// (j.jones 2008-05-02 16:38) - PLID 29519 - delete reward discounts (shouldn't be possible, but do it anyways)
			sqlBatch.Add("DELETE FROM RewardDiscountsT WHERE ServiceID = {INT}", nID);

			sqlBatch.Add("DELETE FROM GCTypesT WHERE ServiceID = {INT}", nID);

			// (j.jones 2015-02-26 10:52) - PLID 65063 - clear ServiceMultiCategoryT
			sqlBatch.Add("DELETE FROM ServiceMultiCategoryT WHERE ServiceID = {INT}", nID);

			// (j.gruber 2012-12-04 08:48) - PLID 48566 - make sure we delete from ServiceInfoLocationT
			sqlBatch.Add("DELETE FROM ServiceLocationInfoT WHERE ServiceID = {INT}", nID);
			sqlBatch.Add("DELETE FROM ServiceT WHERE ID = {INT}", nID);

			long nAuditID = BeginNewAuditEvent();
			try {
				sqlBatch.Execute(GetRemoteData());

				AuditEvent(-1, "", nAuditID, aeiGCTypeDeleted, nID, strName, "<Deleted>", aepMedium, aetDeleted);
				CommitAuditTransaction(nAuditID);
			} NxCatchAllCall("Error deleting the gift certificate type.", 
			{
				if (nAuditID != -1) {
					RollbackAuditTransaction(nAuditID);
				}
				return;
			});

		}

		//now remove the row
		m_pTypes->RemoveRow(nSel);

		//set selection to first row & load again
		m_pTypes->PutCurSel(0);
		m_nCurrentSel = 0;
		LoadCurrentSel();

		m_bChanged = false;
	} NxCatchAll(__FUNCTION__);
}

void CGCTypeDlg::OnNewGcType() 
{
	try {
		//save the current info if we need to
		if(!SaveIfNecessary(false))
			return;

		//prompt them for a name
		CString strResult;

		int nResult = InputBoxLimited(this, "Enter a name for this type:", strResult, "",255,false,false,NULL);
		strResult.TrimRight();

		if(nResult == IDCANCEL)
			return;

		//make sure it's not empty and not too long
		while (strResult == "" || strResult.GetLength() > 255) {
			if(strResult == "") 
				MessageBox("You cannot add a type with a blank name.");
			else 
				MessageBox("Type names are limited to 255 characters.  Please shorten the name.");

			nResult = InputBoxLimited(this, "Enter a name for this type:", strResult, "",255,false,false,NULL);
			if (nResult == IDCANCEL) {
				return;
			}
			strResult.TrimRight();
		}

		//now we've got an acceptable value, put it in the list
		IRowSettingsPtr pRow = m_pTypes->GetRow(-1);


		// (r.gonet 2015-03-25 10:43) - PLID 65276 - Use the global g_cvarNull rather than constructing our own.		
		pRow->PutValue(tcID, (long)-1);
		pRow->PutValue(tcName, _bstr_t(strResult));
		pRow->PutValue(tcValue, g_cvarNull);
		pRow->PutValue(tcPrice, g_cvarNull);
		pRow->PutValue(tcExpires, (short)0);
		pRow->PutValue(tcDays, g_cvarNull);
		pRow->PutValue(tcRecharge, (short)0);
		pRow->PutValue(tcRedeemable, _variant_t(false));
		pRow->PutValue(tcPoints, _variant_t(COleCurrency(0, 0)));
		pRow->PutValue(tcCategoryID, (long)-1);
		pRow->PutValue(tcActive, _variant_t(true));
		m_nRedeemCategoryID = -1;

		long nSel = m_pTypes->AddRow(pRow);

		//now select the row we just added
		m_pTypes->PutCurSel(nSel);
		m_nCurrentSel = nSel;	//update the current selection

		//set the focus to the amount control and highlight it
		// (r.gonet 2015-03-25 10:43) - PLID 65276 - Set the focus to the Value field, rather than the Price field, since it is now on top.
		CWnd* pWnd = GetDlgItem(IDC_GC_TYPE_VALUE_EDIT);
		if(pWnd) {
			pWnd->SetFocus();
			//TODO:  Highlight the text in here
		}

		LoadCurrentSel();

		//something has changed.  We have to set this after the load, because the load
		//resets the changed variable to false.
		m_bChanged = true;

	} NxCatchAll(__FUNCTION__);
}

//checks to see if we need to save.  If so, prompts the user (if not in silent
//	mode) and saves.  Returns true if save succeeded or if no save was needed.
//	Returns false if the save failed or the user cancelled.
bool CGCTypeDlg::SaveIfNecessary(bool bSilent /*= true*/)
{
	try {
		//if nothing has changed, we've got nothing to do
		if(!m_bChanged)
			return true;

		//if nothing is selected in the list, we cannot save
		long nCurSel = m_nCurrentSel;
		if(nCurSel == -1) {
			MsgBox("You must have a currently selected type to save.");
			return false;
		}

		bool bSave = true;	//do we want to save?  If false, we will continue as normal, just nothing will be saved.  True will be returned.

		if(!bSilent) {
			//if we're not silent, we're probably changing something (like the current
			//	selection).  In that case, we need to prompt them to make sure.
			int nRes = MsgBox(MB_YESNOCANCEL, "Do you wish to save your changes?\r\n"
					" - Choosing YES will save the modifications you have made to this type.\r\n"
					" - Choosing NO will cancel your changes and continue.\r\n"
					" - Choosing CANCEL will not change anything.");

			// (r.galicki 2008-08-04 16:05) - PLID 30945 - Reset m_bNewRowDeleted flag each time a save check is made
			m_bNewRowDeleted = FALSE;

			if(nRes == IDYES) {
			}
			else if(nRes == IDNO) {
				//they are aborting their changes, so we need to not save, but keep going
				bSave = false;

				//If we added a new item (it is not yet saved), then they said "NO", then we have to get rid of the whole entry.
				long nID = VarLong(m_pTypes->GetValue(nCurSel, tcID));
				if(nID == -1) {
					m_bNewRowDeleted = m_pTypes->RemoveRow(nCurSel);
				}
			}
			else if(nRes == IDCANCEL) {
				//save aborted, discontinue all activity
				return false;
			}
		}

		if(bSave) {
			//we need to save this data to the current type
			long nID = VarLong(m_pTypes->GetValue(nCurSel, tcID));
			CString strName, strValue, strPrice, strDays;

			GetDlgItemText(IDC_GC_TYPE_NAME, strName);
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Get the Value from the UI.
			GetDlgItemText(IDC_GC_TYPE_VALUE_EDIT, strValue);
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Renamed Amount to Price.
			GetDlgItemText(IDC_GC_TYPE_PRICE_EDIT, strPrice);
			GetDlgItemText(IDC_GC_TYPE_DAYS, strDays);
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Changed the ints into bools since that's what they effectively are.
			bool bExpires = false;
			bool bRecharge = false;
			if(IsDlgButtonChecked(IDC_GC_TYPE_RECHARGE))
				bRecharge = true;
			else {
				//can't expire if it's a recharge
				if(IsDlgButtonChecked(IDC_GC_TYPE_EXP))
					bExpires = true;
			}

			//validation
			strName.TrimRight();
			if(strName.IsEmpty()) {
				//can't have an empty name
				MsgBox("You cannot have an empty name.");
				return false;
			}

			if(strName.GetLength() > 255) {
				MsgBox("The name cannot be longer than 255 characters.");
				return false;
			}

			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Ensure the Value is valid.
			//value
			COleCurrency cyValue;
			cyValue = ParseCurrencyFromInterface(strValue);
			if (cyValue.GetStatus() != COleCurrency::valid) {
				MsgBox("You have entered an invalid value.");
				return false;
			}

			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Renamed Amount to Price.
			//price
			COleCurrency cyPrice;
			cyPrice = ParseCurrencyFromInterface(strPrice);
			if (cyPrice.GetStatus() != COleCurrency::valid) {
				MsgBox("You have entered an invalid price.");
				return false;
			}

			//days
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Changed nExpires into bExpires
			if(strDays.IsEmpty() && bExpires == true) {
				MsgBox("You must enter a number of days if the expiration box is checked.");
				return false;
			}

			// (a.walling 2007-05-23 10:58) - PLID 26114 - Get the points and other values
			// points
			COleCurrency cyPoints;
			CString strPoints;
			{
				GetDlgItemText(IDC_GC_TYPE_REWARD_POINTS_EDIT, strPoints);
				// (j.gruber 2009-07-10 16:05) - PLID 34175 - fix so it'll save more then 3 digits
				COleCurrency cy = ParseCurrencyFromInterface(strPoints);
				strPoints = FormatCurrencyForSql(cy);
				double dPoints = atof(strPoints);

				long nUnits = (long)floor(dPoints);
				long nFractional = (long)((dPoints - floor(dPoints)) * 100);
				cyPoints.SetCurrency(nUnits, nFractional);

				SetDlgItemText(IDC_GC_TYPE_REWARD_POINTS_EDIT, FormatCurrencyForInterface(cyPoints, FALSE));

				if (IsDlgButtonChecked(IDC_GC_TYPE_REWARDS_CHECK) && dPoints == 0) {
					MsgBox("You are attempting to set the reward points redeemed to acquire this gift certificate to zero! If you do not want to be able to redeem points for this gift certificate, uncheck the 'Redeemable for reward points' checkbox.");
					return false;
				}
			}

			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Changed strCategoryID into varCategoryID since it is nullable.
			_variant_t varCategoryID = m_nRedeemCategoryID == -1 ? g_cvarNull : _variant_t(m_nRedeemCategoryID, VT_I4);

			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Changed strDays into varDays since it is nullable.
			_variant_t varDays = g_cvarNull;
			if(bExpires == false) {
				//we don't care what's in the days field, it's becoming null
				varDays = g_cvarNull;
			} else {
				varDays = _variant_t(atol(strDays), VT_I4);
			}

			if(nID == -1) {
				//new type
				long nNewID = -1;
				long nAuditID = BeginNewAuditEvent();
				try {
					// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
					CSqlTransaction trans("NewGCType");
					trans.Begin();
					nNewID = NewNumber("ServiceT", "ID");
					// (j.jones 2009-06-22 11:01) - PLID 34226 - supported inactive, silly though that may be here
					// (r.gonet 2015-03-25 10:43) - PLID 65276 - Parameterized. Renamed Amount to Price.
					ExecuteParamSql("INSERT INTO ServiceT (ID, Name, Category, Price, Active) values "
						"({INT}, {STRING}, {VT_I4}, {OLECURRENCY}, {BIT})",
						nNewID, strName, g_cvarNull/*TODO*/, cyPrice, m_checkInactive.GetCheck() ? false : true);

					// (j.jones 2009-06-22 11:01) - PLID 34226 - 
					// (r.gonet 2015-03-25 10:43) - PLID 65276 - Parameterized. Added Value.
					ExecuteParamSql("INSERT INTO GCTypesT (ServiceID, DefaultExpires, DefaultDays, IsRecharge, Redeemable, Points, DiscountCategoryID, Value) values "
						"({INT}, {BIT}, {VT_I4}, {BIT}, {BIT}, {OLECURRENCY}, {VT_I4}, {OLECURRENCY})",
						nNewID, bExpires, varDays, bRecharge, IsDlgButtonChecked(IDC_GC_TYPE_REWARDS_CHECK) ? true : false, cyPoints, varCategoryID, cyValue);

					// (j.gruber 2012-12-05 11:43) - PLID 48566 - servicelocationinfoT
					ExecuteParamSql("INSERT INTO ServiceLocationInfoT (ServiceID, LocationID) \r\n"
						"SELECT {INT}, ID FROM LocationsT WHERE Managed = 1 "
						, nNewID);

					trans.Commit();

					AuditEvent(-1, "", nAuditID, aeiGCTypeCreated, nNewID, "", strName, aepMedium, aetCreated);
					{
						CString strOld = FormatString("%s: <None>", strName);
						CString strNew = FormatCurrencyForInterface(cyValue);
						AuditEvent(-1, "", nAuditID, aeiGCTypeValueChanged, nID, strOld, strNew, aepMedium, aetChanged);
					}
					
					{
						CString strOld = FormatString("%s: <None>", strName);
						CString strNew = FormatCurrencyForInterface(cyPrice);
						AuditEvent(-1, "", nAuditID, aeiGCTypePriceChanged, nID, strOld, strNew, aepMedium, aetChanged);
					}

					CommitAuditTransaction(nAuditID);
				} NxCatchAllCall("Error creating a new Gift certificate type.", 
				{ 
					if (nAuditID != -1) {
						RollbackAuditTransaction(nAuditID);
					}
					return false; 
				});

				//update our ID
				nID = nNewID;
			}
			else {
				//existing type
				long nAuditID = BeginNewAuditEvent();
				try {
					// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
					CSqlTransaction trans("EditGCType");
					trans.Begin();
					// (j.jones 2009-06-22 11:01) - PLID 34226 - supported inactive
					// (r.gonet 2015-03-25 10:43) - PLID 65276 - Parameterized. Renamed Amount to Price.
					ExecuteParamSql("UPDATE ServiceT SET Name = {STRING}, Category = {VT_I4}, Price = {OLECURRENCY}, Active = {BIT} WHERE ID = {INT}", 
						strName, g_cvarNull/*TODO*/, cyPrice, m_checkInactive.GetCheck() ? false : true, nID);
					// (r.gonet 2015-03-25 10:43) - PLID 65276 - Parameterized. Added Value.
					ExecuteParamSql(
						"UPDATE GCTypesT "
						"SET DefaultExpires = CONVERT(TINYINT, {INT}), "
							"DefaultDays = {VT_I4}, "
							"IsRecharge = CONVERT(TINYINT, {INT}), "
							"Redeemable = {BIT}, "
							"Points = {OLECURRENCY}, "
							"DiscountCategoryID = {VT_I4}, "
							"Value = {OLECURRENCY} "
						"WHERE ServiceID = {INT}", 
						bExpires ? 1 : 0, 
						varDays, 
						bRecharge ? 1 : 0, 
						IsDlgButtonChecked(IDC_GC_TYPE_REWARDS_CHECK) ? true : false, 
						cyPoints, 
						varCategoryID, 
						cyValue,
						nID);
					trans.Commit();

					COleCurrency cyOldValue = VarCurrency(m_pTypes->GetValue(nCurSel, tcValue));
					if (cyOldValue != cyValue) {
						CString strOld = FormatString("%s: %s", strName, FormatCurrencyForInterface(cyOldValue));
						CString strNew = FormatCurrencyForInterface(cyValue);
						AuditEvent(-1, "", nAuditID, aeiGCTypeValueChanged, nID, strOld, strNew, aepMedium, aetChanged);
					}

					COleCurrency cyOldPrice = VarCurrency(m_pTypes->GetValue(nCurSel, tcPrice));
					if (cyOldPrice != cyPrice) {
						CString strOld = FormatString("%s: %s", strName, FormatCurrencyForInterface(cyOldPrice));
						CString strNew = FormatCurrencyForInterface(cyPrice);
						AuditEvent(-1, "", nAuditID, aeiGCTypePriceChanged, nID, strOld, strNew, aepMedium, aetChanged);
					}
					
					CommitAuditTransaction(nAuditID);
				} NxCatchAllCall("Error updating gift certificate type.",
				{
					if (nAuditID != -1) {
						RollbackAuditTransaction(nAuditID);
					}
					return false;
				});
			}

			//now we need to update the datalist to the new values
			m_pTypes->PutValue(nCurSel, tcID, (long)nID);
			m_pTypes->PutValue(nCurSel, tcName, _bstr_t(strName));
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Set the Value column from the edit box.
			m_pTypes->PutValue(nCurSel, tcValue, _variant_t(cyValue));
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Renamed Amount to Price.
			m_pTypes->PutValue(nCurSel, tcPrice, _variant_t(cyPrice));
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Changed nExpires to bExpires, to reflect that it is effectively a bool.
			m_pTypes->PutValue(nCurSel, tcExpires, (short)(bExpires ? 1 : 0));
			m_pTypes->PutValue(nCurSel, tcDays, (long)atoi(strDays));
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Changed nRecharge to bRecharge, to reflect that it is effectively a bool.
			m_pTypes->PutValue(nCurSel, tcRecharge, (short)(bRecharge ? 1 : 0));
			// (a.walling 2007-05-23 11:56) - PLID 26114
			m_pTypes->PutValue(nCurSel, tcRedeemable, _variant_t(bool(IsDlgButtonChecked(IDC_GC_TYPE_REWARDS_CHECK) == 1)));
			m_pTypes->PutValue(nCurSel, tcPoints, _variant_t(cyPoints));
			m_pTypes->PutValue(nCurSel, tcCategoryID, (long)m_nRedeemCategoryID);
			// (j.jones 2009-06-22 11:01) - PLID 34226 - supported inactive
			m_pTypes->PutValue(nCurSel, tcActive, _variant_t(bool(m_checkInactive.GetCheck() == 0)));

			//reset our changed flag
			m_bChanged = false;
		}

		//and return true, everything went as planned
		return true;

	} NxCatchAll(__FUNCTION__);

	//some kind of failure, report failure
	return false;
}

#define ENABLE_WND(idc, bEnable)	((CWnd*)GetDlgItem(idc))->EnableWindow(bEnable);

void CGCTypeDlg::LoadCurrentSel()
{
	try {
		//pull up the info for the given selection
		long nCurSel = m_nCurrentSel;

		if(nCurSel == -1) {
			//disable all controls so they can't type stuff
			ENABLE_WND(IDC_GC_TYPE_NAME, FALSE);
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Disable the Value edit box.
			ENABLE_WND(IDC_GC_TYPE_VALUE_EDIT, FALSE);
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Renamed Amount to Price.
			ENABLE_WND(IDC_GC_TYPE_PRICE_EDIT, FALSE);
			ENABLE_WND(IDC_GC_TYPE_DAYS, FALSE);
			ENABLE_WND(IDC_GC_TYPE_EXP, FALSE);
			ENABLE_WND(IDC_GC_TYPE_RECHARGE, FALSE);
			ENABLE_WND(IDC_GC_TYPE_REWARDS_CHECK, FALSE);
			ENABLE_WND(IDC_CHECK_GC_TYPE_INACTIVE, FALSE);
			return;
		}
		else {
			//show the controls
			ENABLE_WND(IDC_GC_TYPE_NAME, TRUE);
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Enable the Value edit.
			ENABLE_WND(IDC_GC_TYPE_VALUE_EDIT, TRUE);
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Renamed Amount to Price.
			ENABLE_WND(IDC_GC_TYPE_PRICE_EDIT, TRUE);
			ENABLE_WND(IDC_GC_TYPE_DAYS, TRUE);
			ENABLE_WND(IDC_GC_TYPE_EXP, TRUE);
			ENABLE_WND(IDC_GC_TYPE_RECHARGE, TRUE);
			ENABLE_WND(IDC_GC_TYPE_REWARDS_CHECK, TRUE);
			ENABLE_WND(IDC_CHECK_GC_TYPE_INACTIVE, TRUE);
		}

		// (a.walling 2007-05-23 10:39) - PLID 26114
		CheckDlgButton(IDC_GC_TYPE_REWARDS_CHECK, VarBool(m_pTypes->GetValue(nCurSel, tcRedeemable), FALSE));

		EnableRewardItems();

		SetDlgItemText(IDC_GC_TYPE_REWARD_POINTS_EDIT, FormatCurrencyForInterface(VarCurrency(m_pTypes->GetValue(nCurSel, tcPoints), COleCurrency(0, 0)), FALSE));
		m_nRedeemCategoryID = VarLong(m_pTypes->GetValue(nCurSel, tcCategoryID), -1);
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		m_pDiscountCategories->TrySetSelByColumn_Deprecated(tcID, _variant_t(m_nRedeemCategoryID));

		SetDlgItemText(IDC_GC_TYPE_NAME, VarString(m_pTypes->GetValue(nCurSel, tcName), ""));
		// (r.gonet 2015-03-25 10:43) - PLID 65276 - Set the Value field from the combo.
		SetDlgItemText(IDC_GC_TYPE_VALUE_EDIT, FormatCurrencyForInterface(VarCurrency(m_pTypes->GetValue(nCurSel, tcValue), COleCurrency(0, 0))));
		// (r.gonet 2015-03-25 10:43) - PLID 65276 - Renamed Amount to Price.
		SetDlgItemText(IDC_GC_TYPE_PRICE_EDIT, FormatCurrencyForInterface(VarCurrency(m_pTypes->GetValue(nCurSel, tcPrice), COleCurrency(0, 0))));
		SetDlgItemInt(IDC_GC_TYPE_DAYS, VarLong(m_pTypes->GetValue(nCurSel, tcDays), 0));
		CheckDlgButton(IDC_GC_TYPE_EXP, VarShort(m_pTypes->GetValue(nCurSel, tcExpires), 0) == 1 ? TRUE : FALSE);
		CheckDlgButton(IDC_GC_TYPE_RECHARGE, VarShort(m_pTypes->GetValue(nCurSel, tcRecharge), 0) == 1 ? TRUE : FALSE);
		OnGcTypeRecharge();

		// (j.jones 2009-06-22 11:01) - PLID 34226 - supported inactive
		m_checkInactive.SetCheck(VarBool(m_pTypes->GetValue(nCurSel, tcActive), FALSE) ? FALSE : TRUE);

		//setting the values above can set our flag, but since we've just loaded, they clearly haven't changed anything
		m_bChanged = false;

		EnsureDays();
	} NxCatchAll(__FUNCTION__);
}

void CGCTypeDlg::EnsureDays()
{
	if(IsDlgButtonChecked(IDC_GC_TYPE_EXP)) {
		((CWnd*)GetDlgItem(IDC_GC_TYPE_DAYS))->EnableWindow(TRUE);
	}
	else {
		((CWnd*)GetDlgItem(IDC_GC_TYPE_DAYS))->EnableWindow(FALSE);
	}
}

// (r.gonet 2015-03-25 10:43) - PLID 65276 - Added a handler for when the Value field obtains focus.
void CGCTypeDlg::OnEnSetfocusGcTypeValueEdit()
{
	try {
		// (r.gonet 2015-03-25 10:43) - PLID 65276 - If the Value and Price field are the same,
		// then set a flag to push any updates from the Value field to the Price field.
		CString strPrice, strValue;
		GetDlgItemText(IDC_GC_TYPE_VALUE_EDIT, strValue);
		GetDlgItemText(IDC_GC_TYPE_PRICE_EDIT, strPrice);
		if (strValue == strPrice) {
			m_bSyncPriceWithValue = true;
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2015-03-25 10:43) - PLID 65276 - Added a handler for when the Value field loses focus.
void CGCTypeDlg::OnKillfocusGcTypeValue()
{
	try {
		//format this appropriately
		COleCurrency cy(0, 0);
		CString str;
		GetDlgItemText(IDC_GC_TYPE_VALUE_EDIT, str);

		//parse the amount into a colecurrency
		cy = ParseCurrencyFromInterface(str);
		RoundCurrency(cy);	//round it to 2 digits

		if (cy.GetStatus() == COleCurrency::valid) {
			str = FormatCurrencyForInterface(cy);
		}

		//put the formatted (or $0 if invalid) amount back in
		SetDlgItemText(IDC_GC_TYPE_VALUE_EDIT, str);
		// (r.gonet 2015-03-25 10:43) - PLID 65276 - Stop pushing updates from the Value field to the Price field.
		m_bSyncPriceWithValue = false;
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2015-03-25 10:43) - PLID 65276 - Renamed Amount to Price
void CGCTypeDlg::OnKillfocusGcTypePrice() 
{
	try {
		//format this appropriately
		COleCurrency cy(0, 0);
		CString str;
		GetDlgItemText(IDC_GC_TYPE_PRICE_EDIT, str);

		//parse the amount into a colecurrency
		cy = ParseCurrencyFromInterface(str);
		RoundCurrency(cy);	//round it to 2 digits

		if(cy.GetStatus() == COleCurrency::valid) {
			str = FormatCurrencyForInterface(cy);
		}

		//put the formatted (or $0 if invalid) amount back in
		SetDlgItemText(IDC_GC_TYPE_PRICE_EDIT, str);

	} NxCatchAll(__FUNCTION__);
}


void CGCTypeDlg::OnKillfocusGcTypeDays() 
{
	try {
	} NxCatchAll(__FUNCTION__);
}

void CGCTypeDlg::OnKillfocusGcTypeName() 
{
	try {
	} NxCatchAll(__FUNCTION__);
}

void CGCTypeDlg::OnChangeGcTypeDays() 
{
	try {
		// TODO: If this is a RICHEDIT control, the control will not
		// send this notification unless you override the CDialog::OnInitDialog()
		// function and call CRichEditCtrl().SetEventMask()
		// with the ENM_CHANGE flag ORed into the mask.
	
		//only mark it changed if it actually changed
		static CString strOldDays;
		CString str;
		GetDlgItemText(IDC_GC_TYPE_DAYS, str);

		if(strOldDays != str)
			m_bChanged = true;

		strOldDays = str;
	} NxCatchAll(__FUNCTION__);
}

void CGCTypeDlg::OnChangeGcTypeName()
{
	try {
		// TODO: If this is a RICHEDIT control, the control will not
		// send this notification unless you override the CDialog::OnInitDialog()
		// function and call CRichEditCtrl().SetEventMask()
		// with the ENM_CHANGE flag ORed into the mask.

		//only mark it changed if it actually changed
		static CString strOldName;
		CString str;
		GetDlgItemText(IDC_GC_TYPE_NAME, str);

		if(strOldName != str)
			m_bChanged = true;

		strOldName = str;
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2015-03-25 10:43) - PLID 65276 - Added a handler to track when the Value field changes.
void CGCTypeDlg::OnChangeGcTypeValue()
{
	try {
		// (r.gonet 2015-03-25 10:43) - PLID 65276 - Track the previous value of the Value field
		// so we can mark the GC Type changed only if something changed.
		static CString strOldValue;

		CString str;
		GetDlgItemText(IDC_GC_TYPE_VALUE_EDIT, str);
		
		// (r.gonet 2015-03-25 10:43) - PLID 65276 - Push the Value field's value to the Price field
		// if the flag to sync the value is turned on.
		if (m_bSyncPriceWithValue) {
			COleCurrency cyNewValue = ParseCurrencyFromInterface(str);
			RoundCurrency(cyNewValue);	//round it to 2 digits

			CString strNewPrice = FormatCurrencyForInterface(cyNewValue);
			SetDlgItemText(IDC_GC_TYPE_PRICE_EDIT, strNewPrice);
		}

		if (strOldValue != str) {
			m_bChanged = true;
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2015-03-25 10:43) - PLID 65276 - Renamed Amount to Price.
void CGCTypeDlg::OnChangeGcTypePrice() 
{
	try {
		// TODO: If this is a RICHEDIT control, the control will not
		// send this notification unless you override the CDialog::OnInitDialog()
		// function and call CRichEditCtrl().SetEventMask()
		// with the ENM_CHANGE flag ORed into the mask.

		//only mark it changed if it actually changed
		static CString strOldPrice;
		CString str;
		GetDlgItemText(IDC_GC_TYPE_PRICE_EDIT, str);

		if (strOldPrice != str) {
			m_bChanged = true;
		}

		strOldPrice = str;
	} NxCatchAll(__FUNCTION__);
}


void CGCTypeDlg::OnGcTypeRecharge() 
{
	try {
		BOOL bEnable;

		if (IsDlgButtonChecked(IDC_GC_TYPE_RECHARGE))
			//Hide the expiration stuff, it doesn't apply
			bEnable = FALSE;
		else
			bEnable = TRUE;

		GetDlgItem(IDC_GC_TYPE_EXP)->EnableWindow(bEnable);
		EnsureDays();

		m_bChanged = true;
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2007-05-23 10:22) - PLID 26114
void CGCTypeDlg::OnGcTypeEditDiscountCategories() 
{
	try {
		CDiscountsSetupDlg dlg(this);

		dlg.DoModal();

		// refresh the list
		m_pDiscountCategories->Requery();

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiscountCategories->GetNewRow();
		pRow->PutValue(dctcID, _variant_t(long(-1), VT_I4));
		pRow->PutValue(dctcName, _variant_t(" <No Category>"));
		m_pDiscountCategories->AddRowSorted(pRow, NULL);

		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		m_pDiscountCategories->TrySetSelByColumn_Deprecated(dctcID, _variant_t(m_nRedeemCategoryID));

	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2007-05-23 10:48) - PLID 26114
void CGCTypeDlg::OnTrySetSelFinishedGcTypeRewardsCategoryList(long nRowEnum, long nFlags)
{
	try {
		if (nFlags == dlTrySetSelFinishedFailure) {
			// inactive discount category
			// (r.gonet 2015-03-25 10:43) - PLID 65276 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset("SELECT Description FROM DiscountCategoriesT WHERE ID = {INT}", m_nRedeemCategoryID);
			if (!prs->eof) {
				m_pDiscountCategories->PutComboBoxText(_bstr_t(prs->Fields->Item["Description"]->Value));
			} else {
				// (j.gruber 2011-06-17 14:10) - PLID 30098 - maybe they deleted it
				m_pDiscountCategories->CurSel = NULL;
				m_nRedeemCategoryID = -1;
			}

		}
	} NxCatchAll(__FUNCTION__);
}

void CGCTypeDlg::OnSelChangingGcTypeRewardsCategoryList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__);
}

void CGCTypeDlg::OnSelChangedGcTypeRewardsCategoryList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		m_bChanged = true;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);

		m_nRedeemCategoryID = VarLong(pRow->GetValue(dctcID), -1);

	} NxCatchAll(__FUNCTION__);
}

void CGCTypeDlg::OnChangeGcTypeRewardPointsEdit() 
{
	try {
		m_bChanged = true;
	} NxCatchAll(__FUNCTION__);
}

void CGCTypeDlg::OnGcTypeRewardsCheck() 
{
	try {
		m_bChanged = true;
		EnableRewardItems();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2007-05-23 10:56) - PLID 26114
void CGCTypeDlg::EnableRewardItems()
{
	try {
		if (IsDlgButtonChecked(IDC_GC_TYPE_REWARDS_CHECK)) {
			ENABLE_WND(IDC_GC_TYPE_REWARDS_CATEGORY_LIST, TRUE);
			ENABLE_WND(IDC_GC_TYPE_EDIT_DISCOUNT_CATEGORIES, TRUE);
			ENABLE_WND(IDC_GC_TYPE_REWARD_POINTS_EDIT, TRUE);
		} else {
			ENABLE_WND(IDC_GC_TYPE_REWARDS_CATEGORY_LIST, FALSE);
			ENABLE_WND(IDC_GC_TYPE_EDIT_DISCOUNT_CATEGORIES, FALSE);
			ENABLE_WND(IDC_GC_TYPE_REWARD_POINTS_EDIT, FALSE);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2007-05-23 11:38) - PLID 26114
void CGCTypeDlg::OnKillfocusGcTypeRewardPointsEdit() 
{
	try {
		COleCurrency cyPoints;
		CString strPoints;

		GetDlgItemText(IDC_GC_TYPE_REWARD_POINTS_EDIT, strPoints);
		// (j.gruber 2009-07-10 16:24) - PLID 34175 - fixed it here too in case they typed in a comma, like 2,500
		COleCurrency cy = ParseCurrencyFromInterface(strPoints);
		strPoints = FormatCurrencyForSql(cy);
		double dPoints = atof(strPoints);		

		long nUnits = (long)floor(dPoints);
		long nFractional = (long)((dPoints - floor(dPoints)) * 100);
		cyPoints.SetCurrency(nUnits, nFractional);

		SetDlgItemText(IDC_GC_TYPE_REWARD_POINTS_EDIT, FormatCurrencyForInterface(cyPoints, FALSE));
	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2009-06-22 10:58) - PLID 34226 - added support for inactivating gift certificates
void CGCTypeDlg::OnCheckGcTypeInactive()
{
	try {

		if(m_checkInactive.GetCheck()) {
			if(IDNO == MessageBox("Marking a gift certificate type inactive will hide it as an option for new gift certificates.\n\n"
				"Are you sure you wish to mark this type inactive?", "Practice", MB_YESNO|MB_ICONQUESTION)) {
				m_checkInactive.SetCheck(FALSE);
				return;
			}
		}

		m_bChanged = true;

	} NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-10-22 13:37) - PLID 30961
void CGCTypeDlg::ChangeColumnSortFinishedGcTypeList(short nOldSortCol, BOOL bOldSortAscending, short nNewSortCol, BOOL bNewSortAscending)
{
	try {

		//the sel might've changed, so reset it
		m_nCurrentSel = m_pTypes->GetCurSel();

	}NxCatchAll(__FUNCTION__);
}
