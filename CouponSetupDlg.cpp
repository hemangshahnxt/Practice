// (j.gruber 2007-04-02 16:57) - // CouponSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorRc.h"
#include "CouponSetupDlg.h"
#include "GlobalUtils.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "Barcode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.




// (j.gruber 2007-04-02 10:21) - PLID 25164 - class created
/////////////////////////////////////////////////////////////////////////////
// CCouponSetupDlg dialog


CCouponSetupDlg::CCouponSetupDlg(CWnd* pParent)
	: CNxDialog(CCouponSetupDlg::IDD, pParent)
{
	m_strDescription = "";
	m_strBarCode = "";
	m_dtStartDate = COleDateTime::GetCurrentTime();
	m_dtEndDate = COleDateTime::GetCurrentTime();
	m_nPercent = 0;
	m_cyDiscount = COleCurrency(0,0);
	//{{AFX_DATA_INIT(CCouponSetupDlg)
	//}}AFX_DATA_INIT
}


void CCouponSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCouponSetupDlg)
	DDX_Control(pDX, IDC_PERCENT_OFF, m_btnPercentOff);
	DDX_Control(pDX, IDC_DOLLAR_DISCOUNT, m_btnDollarDiscount);
	DDX_Control(pDX, IDC_SHOW_EXPIRED_COUPONS, m_btnShowExpired);
	DDX_Control(pDX, IDC_ADD_COUPON, m_btnAddCoupon);
	DDX_Control(pDX, IDC_DELETE_COUPON, m_btnDeleteCoupon);
	DDX_Control(pDX, IDC_COUPON_DESCRIPTION, m_nxeditCouponDescription);
	DDX_Control(pDX, IDC_COUPON_AMOUNT, m_nxeditCouponAmount);
	DDX_Control(pDX, IDC_COUPON_BARCODE, m_nxeditCouponBarcode);
	DDX_Control(pDX, IDC_COUPONS_TITLE, m_nxstaticCouponsTitle);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCouponSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCouponSetupDlg)
	ON_BN_CLICKED(IDC_ADD_COUPON, OnAddCoupon)
	ON_BN_CLICKED(IDC_DELETE_COUPON, OnDeleteCoupon)
	ON_BN_CLICKED(IDC_SHOW_EXPIRED_COUPONS, OnShowExpiredCoupons)
	ON_BN_CLICKED(IDC_PERCENT_OFF, OnPercentOff)
	ON_BN_CLICKED(IDC_DOLLAR_DISCOUNT, OnDollarDiscount)
	ON_WM_SIZE()
	ON_EN_KILLFOCUS(IDC_COUPON_AMOUNT, OnKillfocusCouponAmount)
	ON_EN_KILLFOCUS(IDC_COUPON_BARCODE, OnKillfocusCouponBarcode)
	ON_EN_KILLFOCUS(IDC_COUPON_DESCRIPTION, OnKillfocusCouponDescription)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCouponSetupDlg message handlers

BEGIN_EVENTSINK_MAP(CCouponSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCouponSetupDlg)
	ON_EVENT(CCouponSetupDlg, IDC_COUPON_LIST, 16 /* SelChosen */, OnSelChosenCouponList, VTS_DISPATCH)
	ON_EVENT(CCouponSetupDlg, IDC_COUPON_LIST, 1 /* SelChanging */, OnSelChangingCouponList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CCouponSetupDlg, IDC_COUPON_LIST, 18 /* RequeryFinished */, OnRequeryFinishedCouponList, VTS_I2)
	ON_EVENT(CCouponSetupDlg, IDC_COUPON_END_DATE, 1 /* KillFocus */, OnKillFocusCouponEndDate, VTS_NONE)
	ON_EVENT(CCouponSetupDlg, IDC_COUPON_START_DATE, 1 /* KillFocus */, OnKillFocusCouponStartDate, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CCouponSetupDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (z.manning, 04/25/2008) - PLID 29566 - Set button styles
		m_btnAddCoupon.AutoSet(NXB_NEW);
		m_btnDeleteCoupon.AutoSet(NXB_DELETE);

		// (b.eyers 2015-04-10) - PLID 58658 - Only allow 255 characters in the barcode text box
		m_nxeditCouponBarcode.SetLimitText(255);

		CheckDlgButton(IDC_SHOW_EXPIRED_COUPONS, 0);

		m_pCouponList = BindNxDataList2Ctrl(IDC_COUPON_LIST, GetRemoteData(), false);

		m_pCouponList->WhereClause = "EndDate >= CONVERT(datetime, Convert(nvarchar, getDate(), 101))";

		m_pCouponList->Requery();

		m_pStartDate = BindNxTimeCtrl(this, IDC_COUPON_START_DATE);
		m_pEndDate = BindNxTimeCtrl(this, IDC_COUPON_END_DATE);

		NXDATALIST2Lib::IColumnSettingsPtr pCol;
		pCol = m_pCouponList->GetColumn(2);
		//pCol->ColumnStyle = NXDATALIST2Lib::csFixedWidth;
		pCol->StoredWidth = 0;		

		m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_ADMIN, 0)));

		// (a.wetta 2007-05-18 09:45) - PLID 25394 - Check the user's permissions and disable the appropriate buttons
		GetDlgItem(IDC_ADD_COUPON)->EnableWindow(CheckCurrentUserPermissions(bioCoupons, sptCreate, FALSE, 0, TRUE, TRUE));
		GetDlgItem(IDC_DELETE_COUPON)->EnableWindow(CheckCurrentUserPermissions(bioCoupons, sptDelete, FALSE, 0, TRUE, TRUE));	
		
		// (a.wetta 2007-05-21 14:23) - PLID 25960 - Bold the title
		extern CPracticeApp theApp;
		GetDlgItem(IDC_COUPONS_TITLE)->SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));
	}NxCatchAll("Error in CCouponSetupDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCouponSetupDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh 
{

	try {

		// (j.gruber 2007-05-09 17:40) - PLID 25164 - kill the focus on the window so that it saves
		// (a.walling 2010-10-12 17:43) - PLID 40908 - Forces focus lost messages (now part of CNexTechDialog)
		CheckFocus();

		m_pCouponList->Requery();
	}NxCatchAll("Error in CCouponSetupDlg::UpdateView");

}

void CCouponSetupDlg::OnAddCoupon() 
{
	try {
		// (a.wetta 2007-05-18 09:37) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioCoupons, sptCreate))
			return;
	
		CString strName;
		if (IDOK == InputBoxLimited(this, "Enter a new coupon description:", strName, "",255,false,false,NULL)) {

			//make sure that this item doesn't exist already
			if (ReturnsRecords("SELECT ID FROM CouponsT WHERE Description like '%s'", _Q(strName))) {

				MsgBox("This coupon description already exists.  Please choose a different coupon name.");
			}
			else {

				strName.TrimLeft();
				strName.TrimRight();
				//make sure they actually entered a value
				if (strName.IsEmpty()) {
					MsgBox("You may not enter blank a coupon description");
				}
				else {

					const _variant_t varFalse(VARIANT_FALSE, VT_BOOL);
				
					//woo hoo we can keep going
					long nNewCouponID = NewNumber("CouponsT", "ID");
					ExecuteSql("INSERT INTO CouponsT (ID, Description, StartDate, EndDate, DiscountType, PercentOff) VALUES "
						" (%li, '%s', CONVERT(datetime, Convert(nvarchar, getDate(), 101)), CONVERT(datetime, Convert(nvarchar, getDate(), 101)), 1, 0) ",
						nNewCouponID, _Q(strName));

					//add it to the datalist
					NXDATALIST2Lib::IRowSettingsPtr pRow;
					pRow = m_pCouponList->GetNewRow();

					pRow->PutValue(0, nNewCouponID);
					pRow->PutValue(1, _variant_t(strName));
					pRow->PutValue(2, varFalse);

					m_pCouponList->AddRowAtEnd(pRow, NULL);

					//set the selection to this
					m_pCouponList->CurSel = pRow;
					OnSelChosenCouponList(pRow);
					
					long nAuditID = BeginNewAuditEvent();
					AuditEvent(-1, "", nAuditID, aeiAdminCouponAdded, nNewCouponID, "", strName, 2, aetCreated);
				}
			}
		}
	}NxCatchAll("Error in CCouponSetupDlg::OnAddCoupon()");
	
}


void CCouponSetupDlg::OnSelChosenCouponList(LPDISPATCH lpRow) 
{
	try {
		//load the rest of the values

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		//clear out all our details
		m_strDescription = "";
		m_strBarCode = "";
		m_dtStartDate = COleDateTime::GetCurrentTime();
		m_dtEndDate = COleDateTime::GetCurrentTime();
		m_nPercent = 0;
		m_cyDiscount = COleCurrency(0,0);

		if (pRow) {

			long nCouponID = VarLong(pRow->GetValue(0));



			ADODB::_RecordsetPtr rs = CreateRecordset("SELECT Description, StartDate, EndDate, DiscountType, PercentOff, DiscountAmount, BarCode "
				" FROM CouponsT WHERE ID = %li", nCouponID);

			// (a.wetta 2007-05-18 10:27) - PLID 25394 - Only enable the edit boxes if they have permission to write
			BOOL bWritePermission = CheckCurrentUserPermissions(bioCoupons, sptWrite, FALSE, 0, TRUE, TRUE);

			if (!rs->eof) {

				SetDlgItemText(IDC_COUPON_DESCRIPTION, AdoFldString(rs, "Description", ""));
				GetDlgItem(IDC_COUPON_DESCRIPTION)->EnableWindow(bWritePermission);
				GetDlgItemText(IDC_COUPON_DESCRIPTION, m_strDescription);

				//m_pStartDate.SetValue(rs->Fields->Item["StartDate"]->Value);
				m_pStartDate->SetDateTime(AdoFldDateTime(rs, "StartDate"));
				GetDlgItem(IDC_COUPON_START_DATE)->EnableWindow(bWritePermission);
				//m_dtStartDate = COleDateTime(m_pStartDate.GetValue());
				m_dtStartDate = m_pStartDate->GetDateTime();

				//m_pEndDate.SetValue(rs->Fields->Item["EndDate"]->Value);
				m_pEndDate->SetDateTime(AdoFldDateTime(rs, "EndDate"));
				GetDlgItem(IDC_COUPON_END_DATE)->EnableWindow(bWritePermission);
				//m_dtEndDate = COleDateTime(m_pEndDate.GetValue());
				m_dtEndDate = m_pEndDate->GetDateTime();

				long nDiscountType = AdoFldLong(rs, "DiscountType");
				GetDlgItem(IDC_DOLLAR_DISCOUNT)->EnableWindow(bWritePermission);
				GetDlgItem(IDC_PERCENT_OFF)->EnableWindow(bWritePermission);
				GetDlgItem(IDC_COUPON_AMOUNT)->EnableWindow(bWritePermission);
				if (nDiscountType == 1) {
					//Percent Off
					CheckDlgButton(IDC_PERCENT_OFF, 1);
					CheckDlgButton(IDC_DOLLAR_DISCOUNT, 0);
					SetDlgItemInt(IDC_COUPON_AMOUNT, AdoFldLong(rs, "PercentOff", 0));
					m_nPercent = GetDlgItemInt(IDC_COUPON_AMOUNT);
				}
				else {
					SetDlgItemText(IDC_COUPON_AMOUNT, FormatCurrencyForInterface(AdoFldCurrency(rs, "DiscountAmount", COleCurrency(0,0)), TRUE, TRUE));
					CheckDlgButton(IDC_PERCENT_OFF, 0);
					CheckDlgButton(IDC_DOLLAR_DISCOUNT, 1);
					m_cyDiscount = AdoFldCurrency(rs, "DiscountAmount", COleCurrency(0,0));
					
				}

				SetDlgItemText(IDC_COUPON_BARCODE, AdoFldString(rs, "BarCode", ""));
				GetDlgItem(IDC_COUPON_BARCODE)->EnableWindow(bWritePermission);
				GetDlgItemText(IDC_COUPON_BARCODE, m_strBarCode);
			}
		}
	}NxCatchAll("Error in CCouponSetupDlg::OnSelChosenCouponList");
	
}

void CCouponSetupDlg::OnSelChangingCouponList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	if (*lppNewSel == NULL) {
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	}
	
}

void CCouponSetupDlg::OnDeleteCoupon() 
{
	try {
		// (a.wetta 2007-05-18 09:37) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioCoupons, sptDelete))
			return;

		NXDATALIST2Lib::IRowSettingsPtr pRow;

		pRow = m_pCouponList->CurSel;

		if (pRow == NULL) {
			MsgBox("Please select a coupon to delete");
		}
		else {

			long nCouponID = VarLong(pRow->GetValue(0));
			//check to see if it is on any charges
			// (s.dhole 2012-04-16 09:33) - PLID 43785 Check if optical shop is using this discount
			if (ReturnsRecords("SELECT ID FROM GlassesOrderServiceDiscountsT WHERE CouponID = %li", nCouponID)) {
				MsgBox("This coupon has been used on optical(Contact/Glasses) order.  You may not delete a coupon that has been used.");
			}
				// (j.gruber 2009-03-25 16:24) - PLID 33360 - change discount structure
			else if (ReturnsRecords("SELECT ID FROM ChargeDiscountsT WHERE CouponID = %li", nCouponID)) {

				MsgBox("This coupon has been used on charges.  You may not delete a coupon that has been used.");

				//TODO:: Check for surgeries also

			}
			else {

				CString strName = VarString(pRow->GetValue(1), "");

				//let'em delete it!
				ExecuteSql("DELETE FROM CouponsT WHERE ID = %li", nCouponID);

				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, "", nAuditID, aeiAdminCouponDeleted, nCouponID, strName, "", 2, aetDeleted);

				//remove the row
				m_pCouponList->RemoveRow(pRow);

				//see what we can set it to
				if (m_pCouponList->GetRowCount() > 0) {
					pRow = m_pCouponList->GetFirstRow();
					m_pCouponList->CurSel = pRow;
					OnSelChosenCouponList(pRow);
				}
				else {
					//there aren't any rows, let's gray out everything
					SetDlgItemText(IDC_COUPON_DESCRIPTION, "");
					GetDlgItem(IDC_COUPON_DESCRIPTION)->EnableWindow(FALSE);
					GetDlgItem(IDC_COUPON_START_DATE)->EnableWindow(FALSE);
					GetDlgItem(IDC_COUPON_END_DATE)->EnableWindow(FALSE);
					CheckDlgButton(IDC_DOLLAR_DISCOUNT, 0);
					GetDlgItem(IDC_DOLLAR_DISCOUNT)->EnableWindow(FALSE);
					CheckDlgButton(IDC_PERCENT_OFF, 0);
					GetDlgItem(IDC_PERCENT_OFF)->EnableWindow(FALSE);
					SetDlgItemText(IDC_COUPON_AMOUNT, "");
					GetDlgItem(IDC_COUPON_AMOUNT)->EnableWindow(FALSE);
					SetDlgItemText(IDC_COUPON_BARCODE, "");
					GetDlgItem(IDC_COUPON_BARCODE)->EnableWindow(FALSE);
				}
			}
		}

	}NxCatchAll("Error in CCouponSetupDlg::OnDeleteCoupon");
	
}

void CCouponSetupDlg::OnShowExpiredCoupons() 
{
	try {

		//check to see if it is already checked
		if (IsDlgButtonChecked(IDC_SHOW_EXPIRED_COUPONS)) {

			//they want to show expired items
			m_pCouponList->WhereClause = "";

			NXDATALIST2Lib::IColumnSettingsPtr pCol;
			pCol = m_pCouponList->GetColumn(2);

			//pCol->ColumnStyle = NXDATALIST2Lib::csWidthPercent;
			pCol->StoredWidth = 10;
			
		}
		else {

			//they are already showing expired, so we have to unshow them
			m_pCouponList->WhereClause = "EndDate >= CONVERT(datetime, Convert(nvarchar, getDate(), 101))";

			NXDATALIST2Lib::IColumnSettingsPtr pCol;
			pCol = m_pCouponList->GetColumn(2);

			//pCol->ColumnStyle = NXDATALIST2Lib::csFixedWidth;
			pCol->StoredWidth = 0;			
		}

		m_pCouponList->Requery();

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pCouponList->GetFirstRow();

		OnSelChosenCouponList(pRow);


	}NxCatchAll("Error in CCouponSetupDlg::OnShowExpiredCoupons");
	
}

void CCouponSetupDlg::OnPercentOff() 
{

	SetDlgItemInt(IDC_COUPON_AMOUNT, m_nPercent);
	Save(IDC_PERCENT_OFF);
	
	
}

void CCouponSetupDlg::OnDollarDiscount() 
{
	SetDlgItemText(IDC_COUPON_AMOUNT, FormatCurrencyForInterface(m_cyDiscount));
	Save(IDC_DOLLAR_DISCOUNT);
	
}



/*BOOL CCouponSetupDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh 
{

	return TRUE;

}*/

BOOL CCouponSetupDlg::Save(long nWindowID) {

	try {
		// (a.wetta 2007-05-18 09:37) - PLID 25394 - This is a last minute check of permissions, it should never get this far if they don't
		// have permission though
		if (!CheckCurrentUserPermissions(bioCoupons, sptWrite))
			return FALSE;

		CString strOldValue, strNewValueForAudit, strField, strDescription, strNewValueForQuery;
		long nAuditID;
		ADODB::_RecordsetPtr rs;

		//get the ID from the dialog
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pCouponList->CurSel;

		if (pRow) {

			long nCouponID = VarLong(pRow->GetValue(0));


			switch (nWindowID) {

				case IDC_COUPON_DESCRIPTION:
		
					rs = CreateRecordset("SELECT Description FROM CouponsT WHERE ID = %li", nCouponID);
					if (! rs->eof) {
						//put in the default, just in case
						strOldValue = AdoFldString(rs, "Description", "");
						GetDlgItemText(IDC_COUPON_DESCRIPTION, strNewValueForAudit);
						nAuditID = aeiAdminCouponDescription;
						strField = "Description";
						strDescription = strOldValue;
						strNewValueForQuery = "'" +  _Q(strNewValueForAudit) + "'";
					}
					rs->Close();
				break;
	
				case IDC_COUPON_START_DATE:
					rs = CreateRecordset("SELECT Description, StartDate FROM CouponsT WHERE ID = %li", nCouponID);
					if (!rs->eof) {
						strOldValue = AsString(rs->Fields->Item["StartDate"]->Value);
						//strNewValueForAudit = FormatDateTimeForInterface(COleDateTime(m_pStartDate.GetValue()), NULL, dtoDate, FALSE);
						strNewValueForAudit = FormatDateTimeForInterface(m_pStartDate->GetDateTime(), NULL, dtoDate, FALSE);
						nAuditID = aeiAdminCouponStartDate;
						strField = "StartDate";
						strDescription = AdoFldString(rs, "Description", "");
						//strNewValueForQuery = "'" + FormatDateTimeForSql(COleDateTime(m_pStartDate.GetValue())) + "'";
						strNewValueForQuery = "'" + FormatDateTimeForSql(COleDateTime(m_pStartDate->GetDateTime())) + "'";
					}
					rs->Close();
				break;

				case IDC_COUPON_END_DATE:
					rs = CreateRecordset("SELECT Description, EndDate FROM CouponsT WHERE ID = %li", nCouponID);
					if (!rs->eof) {
						strOldValue = AsString(rs->Fields->Item["EndDate"]->Value);
						//strNewValueForAudit = FormatDateTimeForInterface(COleDateTime(m_pEndDate.GetValue()), NULL, dtoDate, FALSE);
						strNewValueForAudit = FormatDateTimeForInterface(m_pEndDate->GetDateTime(), NULL, dtoDate, FALSE);
						nAuditID = aeiAdminCouponEndDate;
						strField = "EndDate";
						strDescription = AdoFldString(rs, "Description", "");
						strNewValueForQuery = "'" + FormatDateTimeForSql(m_pEndDate->GetDateTime()) + "'" ;
					}
					rs->Close();
				break;

				case IDC_PERCENT_OFF:
					rs = CreateRecordset("SELECT Description, DiscountType FROM CouponsT WHERE ID = %li", nCouponID);
					if (!rs->eof) {
						strOldValue = "Dollar Discount";
						strNewValueForAudit = "Percent Discount";
						nAuditID = aeiAdminCouponDiscountType;
						strField = "DiscountType";
						strDescription = AdoFldString(rs, "Description", "");
						strNewValueForQuery = "1";
					}
					rs->Close();
				break;

				case IDC_DOLLAR_DISCOUNT:
			
					rs = CreateRecordset("SELECT Description, DiscountType FROM CouponsT WHERE ID = %li", nCouponID);
					if (!rs->eof) {
						strOldValue = "Percent Discount";
						strNewValueForAudit = "Dollar Discount";
						nAuditID = aeiAdminCouponDiscountType;
						strField = "DiscountType";
						strDescription = AdoFldString(rs, "Description", "");
						strNewValueForQuery = "2";
					}
					rs->Close();
				break;

				case IDC_COUPON_AMOUNT:
			
					rs = CreateRecordset("SELECT Description, DiscountAmount, PercentOff FROM CouponsT WHERE ID = %li", nCouponID);
					if (!rs->eof) {
						if (IsDlgButtonChecked(IDC_PERCENT_OFF)) {
							strOldValue = AsString(rs->Fields->Item["PercentOff"]->Value);
							GetDlgItemText(IDC_COUPON_AMOUNT, strNewValueForAudit);
							nAuditID = aeiAdminCouponPercentOff;
							strField = "PercentOff";
							strDescription = AdoFldString(rs, "Description", "");
							//in case they put a % sign, take it out
							strNewValueForQuery = strNewValueForAudit.SpanIncluding("1234567890");
						}
						else {
							strOldValue = AsString(rs->Fields->Item["DiscountAmount"]->Value);
							GetDlgItemText(IDC_COUPON_AMOUNT, strNewValueForAudit);
							COleCurrency cyAmt;
							//we already did this, but we need to format it for SQL, so let's do it again
							cyAmt = ParseCurrencyFromInterface(strNewValueForAudit);
							nAuditID = aeiAdminCouponDiscountAmt;
							strField = "DiscountAmount";
							strDescription = AdoFldString(rs, "Description", "");
							strNewValueForQuery.Format("Convert(money, %s)", FormatCurrencyForSql(cyAmt));
						}
					}
					rs->Close();
				break;
				
				case IDC_COUPON_BARCODE:

					rs = CreateRecordset("SELECT Description, BarCode FROM CouponsT WHERE ID = %li", nCouponID);
					if (!rs->eof) {
						strOldValue = AdoFldString(rs, "BarCode", "");
						GetDlgItemText(IDC_COUPON_BARCODE, strNewValueForAudit);
						nAuditID = aeiAdminCouponBarCode;
						strField = "BarCode";
						strDescription = AdoFldString(rs, "Description", "");
						
						// (b.eyers 2015-04-10) - PLID 58658 - Only allow 255 characters in the barcode text box, this should never hit
						if (strNewValueForAudit.GetLength() > 255) {
							AfxMessageBox("Coupon Barcode exceeds text limit. Please enter in less then 255 characters.", MB_OK);
							return FALSE;
						}

						strNewValueForQuery = "'" + _Q(strNewValueForAudit) + "'" ;
					}
					rs->Close();
				break;

				default:
					ASSERT(FALSE);
					return FALSE;
				break;
			}

			//now update the data and audit
			ExecuteSql("UPDATE CouponsT SET %s = %s WHERE ID = %li", strField, strNewValueForQuery, nCouponID);

			long nAudit = BeginNewAuditEvent();
			AuditEvent(-1, strDescription, nAudit, nAuditID, nCouponID, strOldValue, strNewValueForAudit, 2);

			return TRUE;

		}
	}NxCatchAllCall("Error in CCouponSetupDlg::Save(long nWindowID) ", return FALSE);

	//if we are here, we failed
	return FALSE;
}


void CCouponSetupDlg::OnSize(UINT nType, int cx, int cy) 
{
	CNxDialog::OnSize(nType, cx, cy);

	SetControlPositions();
	Invalidate();
	

	
}

void CCouponSetupDlg::OnRequeryFinishedCouponList(short nFlags) 
{
	try {
		//check to see how many rows we have
		if (m_pCouponList->GetRowCount() > 0) {
			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pCouponList->GetFirstRow();
			m_pCouponList->CurSel = pRow;
			OnSelChosenCouponList(pRow);
		}
		else {
			//there aren't any rows, let's gray out everything
			SetDlgItemText(IDC_COUPON_DESCRIPTION, "");
			GetDlgItem(IDC_COUPON_DESCRIPTION)->EnableWindow(FALSE);
			GetDlgItem(IDC_COUPON_START_DATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_COUPON_END_DATE)->EnableWindow(FALSE);
			CheckDlgButton(IDC_DOLLAR_DISCOUNT, 0);
			GetDlgItem(IDC_DOLLAR_DISCOUNT)->EnableWindow(FALSE);
			CheckDlgButton(IDC_PERCENT_OFF, 0);
			GetDlgItem(IDC_PERCENT_OFF)->EnableWindow(FALSE);
			SetDlgItemText(IDC_COUPON_AMOUNT, "");
			GetDlgItem(IDC_COUPON_AMOUNT)->EnableWindow(FALSE);
			SetDlgItemText(IDC_COUPON_BARCODE, "");
			GetDlgItem(IDC_COUPON_BARCODE)->EnableWindow(FALSE);
		}
	}NxCatchAll("Error in CCouponSetupDlg::OnRequeryFinishedCouponList");
}

void CCouponSetupDlg::OnKillfocusCouponAmount() 
{
	try {
		//first see whether we are using percent or discount
		CString strValue;
		GetDlgItemText(IDC_COUPON_AMOUNT, strValue);
		if (IsDlgButtonChecked(IDC_PERCENT_OFF)) {

			strValue.TrimLeft();
			strValue.TrimRight();

			if (strValue.IsEmpty()) {
				strValue = "0";	
			}
			//check to see if there is a decimal and take off the rest of it
			long nResult = strValue.Find(".");
			if (nResult != -1) {
				//take off everything from the decimal on
				strValue = strValue.Left(nResult);

				strValue.TrimRight();
				strValue.TrimLeft();

				//if this results in a blank because the decimal is the at the 0 position, then make it a zero
				if (strValue.IsEmpty()) {
					strValue = "0";
				}
			}

			if (atoi(strValue) != m_nPercent) {
				//make sure it is a valid number
				if(strValue.SpanIncluding("1234567890%").GetLength() == strValue.GetLength()) {

					//make sure it isn't greater then 100
					if (atoi(strValue) > 100 || atoi(strValue) < 0) {
						MsgBox("You may not have a positive percentage amount greater than 100.");
						SetDlgItemInt(IDC_COUPON_AMOUNT, m_nPercent);
					}
					else {

						//we can save it
						//set this just in case we changed it along the way
						SetDlgItemText(IDC_COUPON_AMOUNT, strValue);
						if (Save(IDC_COUPON_AMOUNT)) {
							m_nPercent = atoi(strValue.SpanIncluding("1234567890"));
						}
						else {
							SetDlgItemInt(IDC_COUPON_AMOUNT, m_nPercent);
						}
					}
				}
				else {
					//its not a number
					MsgBox("Please enter a valid percentage");
					SetDlgItemInt(IDC_COUPON_AMOUNT, m_nPercent);
				}
			}
			else {
				//they might have changed from one crap value to another, let's set the box just in case
				SetDlgItemInt(IDC_COUPON_AMOUNT, m_nPercent);
			}
		
		}
		else {

			//make sure that it is a valid currency
			strValue.TrimLeft();
			strValue.TrimRight();
			if (!strValue.IsEmpty() ) {
				COleCurrency cyDiscount = ParseCurrencyFromInterface(strValue);
				if (cyDiscount != m_cyDiscount) {
					if (cyDiscount.GetStatus() == COleCurrency::invalid) {
						MsgBox("Please enter a valid amount.");
						//not sure why we are making this 0, it can just be the last value
						SetDlgItemText(IDC_COUPON_AMOUNT, FormatCurrencyForInterface(m_cyDiscount, TRUE, TRUE));
					}
					else if (cyDiscount < COleCurrency(0,0)) {
						MsgBox("Please enter a positive value.");
						SetDlgItemText(IDC_COUPON_AMOUNT, FormatCurrencyForInterface(m_cyDiscount, TRUE, TRUE));
					}
					else {
						SetDlgItemText(IDC_COUPON_AMOUNT, FormatCurrencyForInterface(cyDiscount));
						if (Save(IDC_COUPON_AMOUNT) ) {
							m_cyDiscount = cyDiscount;
						}
						else {
							SetDlgItemText(IDC_COUPON_AMOUNT, FormatCurrencyForInterface(m_cyDiscount, TRUE, TRUE));
						}
					}
				}
				else {
					//they could have set the box from crap to crap, reset the box to what it is
					SetDlgItemText(IDC_COUPON_AMOUNT, FormatCurrencyForInterface(m_cyDiscount, TRUE, TRUE));
				}
			}
			else {
				if (m_cyDiscount != COleCurrency(0,0)) {
					SetDlgItemText(IDC_COUPON_AMOUNT, FormatCurrencyForInterface(COleCurrency(0,0)));
					if (Save(IDC_COUPON_AMOUNT)) {
						m_cyDiscount = COleCurrency(0,0);
						SetDlgItemText(IDC_COUPON_AMOUNT, FormatCurrencyForInterface(m_cyDiscount, TRUE, TRUE));
					}
					else {
						SetDlgItemText(IDC_COUPON_AMOUNT, FormatCurrencyForInterface(m_cyDiscount, TRUE, TRUE));
					}
				}
			}

		}
	}NxCatchAll("Error in CCouponSetupDlg::OnKillfocusCouponAmount() ");
}

void CCouponSetupDlg::OnKillfocusCouponBarcode() 
{
	try  {
		//make sure another Coupon doesn't have this barcode, because that would be confusing
		CString strValue;
		GetDlgItemText(IDC_COUPON_BARCODE, strValue);
		strValue.TrimLeft();
		strValue.TrimRight();
		if (strValue.Compare(m_strBarCode) != 0) {
			//make sure that this item doesn't exist already
			// (a.walling 2007-05-10 09:56) - PLID 25171 - Warn for collisions, but we'll allow it for now.
			if (Barcode_CheckCollisions(this, strValue)) {
				if (Save(IDC_COUPON_BARCODE)) {
					m_strBarCode = strValue;
				}
				else {
					SetDlgItemText(IDC_COUPON_BARCODE, m_strBarCode);
				}
			} else {
				SetDlgItemText(IDC_COUPON_BARCODE, m_strBarCode);
			}
		}
	}NxCatchAll("CCouponSetupDlg::OnKillfocusCouponBarcode() ");
}


void CCouponSetupDlg::OnKillfocusCouponDescription() 
{
	try {
		//make sure another Coupon doesn't have this description
		CString strValue;
		GetDlgItemText(IDC_COUPON_DESCRIPTION, strValue);
		strValue.TrimLeft();
		strValue.TrimRight();
		//make sure its not blank
		if (strValue.IsEmpty()) {
			MsgBox("You may not enter a blank coupon name");
			SetDlgItemText(IDC_COUPON_DESCRIPTION, m_strDescription);
		}
		else {
			if (strValue.Compare(m_strDescription) != 0) {
				//make sure that this item doesn't exist already
				if (ReturnsRecords("SELECT ID FROM CouponsT WHERE Description like '%s'", _Q(strValue))) {

					MsgBox("This coupon description already exists.  Please choose a different coupon name.");
				}
				else if (strValue.GetLength() > 255) {
					MsgBox("Please enter a description less then 255 characters.");
				}
				else {
					if (Save(IDC_COUPON_DESCRIPTION) ) {
						m_strDescription = strValue;
						NXDATALIST2Lib::IRowSettingsPtr pRow;
						pRow = m_pCouponList->CurSel;
						if (pRow) {
							pRow->PutValue(1, _variant_t(m_strDescription));
						}
					}
					else {
						SetDlgItemText(IDC_COUPON_DESCRIPTION, m_strDescription);
					}
				}	
			}
		}
	}NxCatchAll("Error in CCouponSetupDlg::OnKillfocusCouponDescription() ");
}


void CCouponSetupDlg::OnKillFocusCouponEndDate() 
{
	try  {
		//make sure its a valid date (even though it really already should be)
		//COleDateTime dtEnd = COleDateTime(m_pEndDate.GetValue());
		COleDateTime dtEnd = m_pEndDate->GetDateTime();
		COleDateTime dtMin;
		dtMin.SetDate(1899,12,31);
		BOOL bExpired = FALSE;

		if (dtEnd.GetYear() != m_dtEndDate.GetYear() || dtEnd.GetMonth() != m_dtEndDate.GetMonth() || 
			dtEnd.GetDay()  != m_dtEndDate.GetDay()) {

			if (dtEnd >= dtMin && dtEnd.GetStatus() == COleDateTime::valid) {

				//check to see if it is before the start date
				COleDateTime dtStart = m_pStartDate->GetDateTime();
				if (dtStart > dtEnd) {
					MsgBox("The end date cannot be before the start date");
					m_pEndDate->SetDateTime(m_dtEndDate);
					return;
				}


				//check to see if they've made it expired now
				COleDateTime dtToday, dtNow = COleDateTime::GetCurrentTime();			
				dtToday.SetDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 0,0,0);
				
				if (dtEnd < dtToday) {
					
					if (MsgBox(MB_YESNO, "You are making this coupon be expired, are you sure you wish to do this?") == IDYES) {
						bExpired = TRUE;
					}
					else {
						m_pEndDate->SetDateTime(m_dtEndDate);
						return;
					}

				}
				
				if (Save(IDC_COUPON_END_DATE) ) {

					m_dtEndDate = dtEnd;			
				
				
					if (bExpired) {
						//they set the enddate to be before today, so its already expired
						if (IsDlgButtonChecked(IDC_SHOW_EXPIRED_COUPONS)) {
							//they are showing expired, so we just need to change the column
							NXDATALIST2Lib::IRowSettingsPtr pRow;
							pRow = m_pCouponList->CurSel;
							if (pRow) {
								pRow->PutValue(2, (long) 1);
							}
						}
						else {
							//hmm, here's a conundrum because they are changing the date on the current selection
							//let's warn them and remove it from the list
							MsgBox("This coupon has been marked as expired, but you aren't showing expired items in your list.  This item will be removed, please click on Show Expired to edit it further.");
							NXDATALIST2Lib::IRowSettingsPtr pRow;
							pRow = m_pCouponList->CurSel;
							if (pRow) {
								m_pCouponList->RemoveRow(pRow);
								
								//make sure there are other items in the list
								if (m_pCouponList->GetRowCount() > 0) {
									pRow = m_pCouponList->GetFirstRow();
									OnSelChosenCouponList(pRow);
								}
								else {
									//there aren't any rows, let's gray out everything
									SetDlgItemText(IDC_COUPON_DESCRIPTION, "");
									GetDlgItem(IDC_COUPON_DESCRIPTION)->EnableWindow(FALSE);
									GetDlgItem(IDC_COUPON_START_DATE)->EnableWindow(FALSE);
									GetDlgItem(IDC_COUPON_END_DATE)->EnableWindow(FALSE);
									CheckDlgButton(IDC_DOLLAR_DISCOUNT, 0);
									GetDlgItem(IDC_DOLLAR_DISCOUNT)->EnableWindow(FALSE);
									CheckDlgButton(IDC_PERCENT_OFF, 0);
									GetDlgItem(IDC_PERCENT_OFF)->EnableWindow(FALSE);
									SetDlgItemText(IDC_COUPON_AMOUNT, "");
									GetDlgItem(IDC_COUPON_AMOUNT)->EnableWindow(FALSE);
									SetDlgItemText(IDC_COUPON_BARCODE, "");
									GetDlgItem(IDC_COUPON_BARCODE)->EnableWindow(FALSE);
								}							
							}
						}
					}
					else {
						//reflect the change in the datalist
						//they set the enddate to be before today, so its already expired
						if (IsDlgButtonChecked(IDC_SHOW_EXPIRED_COUPONS)) {
							//they are showing expired, so we just need to change the column
							NXDATALIST2Lib::IRowSettingsPtr pRow;
							pRow = m_pCouponList->CurSel;
							if (pRow) {
								pRow->PutValue(2, (long) 0);
							}
						}
					}
				}
				else {
					//m_pEndDate.SetValue(_variant_t(m_dtEndDate));
					m_pEndDate->SetDateTime(m_dtEndDate);
				}
			
			}
			else {
				m_pEndDate->SetDateTime(m_dtEndDate);
			}

		}
		
	}NxCatchAll("Error in CCouponSetupDlg::OnKillFocusCouponEndDate() ");
}

void CCouponSetupDlg::OnKillFocusCouponStartDate() 
{
	try {
		//make sure its a valid date (even though it really already should be)
		//COleDateTime dtStart = COleDateTime(m_pStartDate.GetValue());
		COleDateTime dtStart = m_pStartDate->GetDateTime();
		COleDateTime dtMin;
		dtMin.SetDate(1899,12,31);

		if (dtStart.GetYear() != m_dtStartDate.GetYear() || dtStart.GetMonth() != m_dtStartDate.GetMonth() || 
			dtStart.GetDay()  != m_dtStartDate.GetDay()) {

			if (dtStart >= dtMin && dtStart.GetStatus() == COleDateTime::valid) {

				//make sure it is after the start time
				//COleDateTime dtEnd = COleDateTime(m_pEndDate.GetValue());
				COleDateTime dtEnd = m_pEndDate->GetDateTime();
				if (dtEnd < dtStart) {
					MsgBox("The end date cannot be before the start date");
					m_pStartDate->SetDateTime(m_dtStartDate);
					return;
				}
				else {
					if (Save(IDC_COUPON_START_DATE)) {
						m_dtStartDate = dtStart;
					}
					else {
						//m_pStartDate.SetValue(_variant_t(m_dtStartDate));
						m_pStartDate->SetDateTime(m_dtStartDate);
					}
				}
			}
			else {
				m_pStartDate->SetDateTime(m_dtStartDate);
			}		
		}
	}NxCatchAll("Error in CCouponSetupDlg::OnKillFocusCouponStartDate() ");
}


LRESULT CCouponSetupDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	// (a.walling 2007-05-09 12:59) - PLID 25171 - Update the barcode for the coupon
	// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
	_bstr_t bstr = (BSTR)lParam; // We can typecast this to an ANSI string

	GetDlgItem(IDC_COUPON_BARCODE)->SetFocus();
	SetDlgItemText(IDC_COUPON_BARCODE, (LPCTSTR)bstr);
	((CNxEdit*)GetDlgItem(IDC_COUPON_BARCODE))->SetSel(0, -1);

	return 0;
}
