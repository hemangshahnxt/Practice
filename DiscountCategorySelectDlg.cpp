// DiscountCategorySelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "billingRc.h"
#include "DiscountCategorySelectDlg.h"
#include "CouponSelectDlg.h"
#include "Barcode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDiscountCategorySelectDlg dialog


// (j.gruber 2007-04-04 14:22) - PLID 9796 - added coupons
// (j.gruber 2007-05-14 14:45) - PLID 25173 - adding support for not showing coupons
CDiscountCategorySelectDlg::CDiscountCategorySelectDlg(CWnd* pParent, long nDiscountCatID, CString strCustomDesc, long nCouponID, BOOL bShowCoupons /*= TRUE*/)
	: CNxDialog(CDiscountCategorySelectDlg::IDD, pParent)
{

	m_nDiscountCatID = nDiscountCatID;
	m_strCustomDescription = strCustomDesc;
	// (j.gruber 2007-04-04 14:22) - PLID 9796 - added coupons
	m_nCouponID = nCouponID;
	m_bShowCoupons = bShowCoupons;
	//{{AFX_DATA_INIT(CDiscountCategorySelectDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bScanning = false;
}

CDiscountCategorySelectDlg::~CDiscountCategorySelectDlg()
{
	try {

		// (j.jones 2008-12-30 15:15) - PLID 32584 - unregister from barcode scanning
		if(GetMainFrame()) {
			if(!GetMainFrame()->UnregisterForBarcodeScan(this)) {
				MsgBox("Error unregistering for barcode scans.");
			}
		}

	}NxCatchAll("Error in ~CDiscountCategorySelectDlg");
}


void CDiscountCategorySelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDiscountCategorySelectDlg)
	DDX_Control(pDX, IDC_USE_CUSTOM, m_btnCustom);
	DDX_Control(pDX, IDC_CUSTOM_DISCOUNT_CAT, m_nxeditCustomDiscountCat);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDiscountCategorySelectDlg, CNxDialog)
	//{{AFX_MSG_MAP(CDiscountCategorySelectDlg)
	ON_BN_CLICKED(IDC_USE_CUSTOM, OnUseCustom)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiscountCategorySelectDlg message handlers

BEGIN_EVENTSINK_MAP(CDiscountCategorySelectDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CDiscountCategorySelectDlg)
	ON_EVENT(CDiscountCategorySelectDlg, IDC_DISCOUNT_CATEGORY_LIST, 16 /* SelChosen */, OnSelChosenDiscountCategoryList, VTS_DISPATCH)
	ON_EVENT(CDiscountCategorySelectDlg, IDC_DISCOUNT_CATEGORY_LIST, 1 /* SelChanging */, OnSelChangingDiscountCategoryList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CDiscountCategorySelectDlg, IDC_DISCOUNT_CATEGORY_LIST, 18 /* RequeryFinished */, OnRequeryFinishedDiscountCategoryList, VTS_I2)
	ON_EVENT(CDiscountCategorySelectDlg, IDC_DISCOUNT_CATEGORY_LIST, 20 /* TrySetSelFinished */, OnTrySetSelFinishedDiscountCategoryList, VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CDiscountCategorySelectDlg::OnUseCustom() 
{
	if (IsDlgButtonChecked(IDC_USE_CUSTOM)) {
		//gray out the list
		GetDlgItem(IDC_DISCOUNT_CATEGORY_LIST)->EnableWindow(FALSE);
		GetDlgItem(IDC_CUSTOM_DISCOUNT_CAT)->EnableWindow(TRUE);
	}
	else {
		GetDlgItem(IDC_DISCOUNT_CATEGORY_LIST)->EnableWindow(TRUE);
		GetDlgItem(IDC_CUSTOM_DISCOUNT_CAT)->EnableWindow(FALSE);

		//see if we have a category
		if (m_nDiscountCatID != -3) {
			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pCatList->SetSelByColumn(0, m_nDiscountCatID);
			if (pRow == NULL) {
				m_pCatList->SetSelByColumn(0, (long)-3);
			}
		}
		else {
			m_pCatList->SetSelByColumn(0, (long)-3);
		}
	}	
}

void CDiscountCategorySelectDlg::OnSelChosenDiscountCategoryList(LPDISPATCH lpRow) 
{
	//see if it is a coupon
	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
	if (pRow) {

		long nID = pRow->GetValue(0);

		if (nID == -2) {
			// (j.gruber 2007-04-04 14:22) - PLID 9796 - added coupons
			//its a coupon, do something
			CCouponSelectDlg dlg(this);
			if (dlg.DoModal() == IDOK) {

				//we should be able to get a coupon ID
				m_nCouponID = dlg.m_nCouponID;
				CString strCouponName = dlg.m_strCouponName;

				//add it to the datalist
				pRow->PutValue(1, _variant_t("<Coupon> - " + strCouponName));				
			}
			else {
				if (m_nCouponID == -1) {
					//none selected
					pRow->PutValue(1, _variant_t("<Coupon>"));				
				}				
			}

		}
	}
	
	
}

BOOL CDiscountCategorySelectDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
			
		// (c.haag 2008-05-01 15:49) - PLID 29871 - NxIconified buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pCatList = BindNxDataList2Ctrl(this, IDC_DISCOUNT_CATEGORY_LIST, GetRemoteData(), true);

		// (j.jones 2008-12-30 15:15) - PLID 32584 - this dialog needs to register for barcode scans
		if(GetMainFrame()) {
			if(!GetMainFrame()->RegisterForBarcodeScan(this)) {
				MsgBox("Error registering for barcode scans.  You may not be able to scan in this screen.");
			}
		}
	}
	NxCatchAll("Error in CDiscountCategorySelectDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDiscountCategorySelectDlg::OnSelChangingDiscountCategoryList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	if (*lppNewSel == NULL) {
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	}
	
}

void CDiscountCategorySelectDlg::OnRequeryFinishedDiscountCategoryList(short nFlags) 
{
	
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCatList->GetNewRow();
	NXDATALIST2Lib::IRowSettingsPtr pFirstRow = m_pCatList->GetFirstRow();


	if (m_bShowCoupons) {
		//add the coupon line
		pRow->PutValue(0, (long)-2);
		pRow->PutValue(1, _variant_t("<Coupon>"));
		m_pCatList->AddRowBefore(pRow, pFirstRow);

		pFirstRow = m_pCatList->GetFirstRow();
	}

	//add a no line
	pRow = m_pCatList->GetNewRow();
	pRow->PutValue(0, (long)-3);
	pRow->PutValue(1, _variant_t("<No Category>"));
	m_pCatList->AddRowBefore(pRow, pFirstRow);


	//now set the default value
	if (m_nDiscountCatID == -1) {

		GetDlgItem(IDC_DISCOUNT_CATEGORY_LIST)->EnableWindow(FALSE);
		GetDlgItem(IDC_CUSTOM_DISCOUNT_CAT)->EnableWindow(TRUE);

		CheckDlgButton(IDC_USE_CUSTOM, 1);		
		
		//they are using a custom value
		SetDlgItemText(IDC_CUSTOM_DISCOUNT_CAT, m_strCustomDescription);
	}
	else if (m_nDiscountCatID == -2) {
		// (j.gruber 2007-05-14 14:45) - PLID 25173 - adding support for not showing coupons
		if (m_bShowCoupons) {
			//its a coupon
			GetDlgItem(IDC_DISCOUNT_CATEGORY_LIST)->EnableWindow(TRUE);
			GetDlgItem(IDC_CUSTOM_DISCOUNT_CAT)->EnableWindow(FALSE);

			CheckDlgButton(IDC_USE_CUSTOM, 0);		

			pRow = m_pCatList->SetSelByColumn(0, (long)-2);
			//we just put it in, so we know its there
			if (pRow) {
				ADODB::_RecordsetPtr rs = CreateRecordset("SELECT Description FROM CouponsT WHERE ID = %li", m_nCouponID);
				if (!rs->eof) {
					pRow->PutValue(1, _variant_t("<Coupon> - " + AdoFldString(rs, "Description", "")));
				}
			}
		}
		else {
			//this really shouldn't happen, but we'll handle it anyway
			//set it to be no
			GetDlgItem(IDC_DISCOUNT_CATEGORY_LIST)->EnableWindow(TRUE);
			GetDlgItem(IDC_CUSTOM_DISCOUNT_CAT)->EnableWindow(FALSE);

			CheckDlgButton(IDC_USE_CUSTOM, 0);	
			
			m_pCatList->SetSelByColumn(0, (long)-3);
		}

	}
	else {

		GetDlgItem(IDC_DISCOUNT_CATEGORY_LIST)->EnableWindow(TRUE);
		GetDlgItem(IDC_CUSTOM_DISCOUNT_CAT)->EnableWindow(FALSE);

		CheckDlgButton(IDC_USE_CUSTOM, 0);		

		//account for inactives
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		long nSel = m_pCatList->TrySetSelByColumn_Deprecated(0, m_nDiscountCatID);
		if (nSel == -1) {
			//we have an inactive
			CString strDiscName = VarString(GetTableField("DiscountCategoriesT", "Description", "ID", m_nDiscountCatID),"");
			if (! strDiscName.IsEmpty()) {
				m_pCatList->PutComboBoxText(_bstr_t(strDiscName));
			}
		}
		else if (nSel == NXDATALIST2Lib::sriNoRowYet_WillFireEvent) {
			//we don't really need to do anything
		}
	}	
}

void CDiscountCategorySelectDlg::OnOK() 
{
	//(e.lally 2009-08-18) PLID 28825 - Added preference for requiring a discount category
	BOOL bRequireDiscCategory = GetRemotePropertyInt("RequireDiscountCategory", 0, 0, "<None>", true) == 0 ? FALSE: TRUE;

	//first see if we are using a custom
	if (IsDlgButtonChecked(IDC_USE_CUSTOM)) {
		m_nDiscountCatID = -1;
		GetDlgItemText(IDC_CUSTOM_DISCOUNT_CAT, m_strCustomDescription);

		//(e.lally 2009-08-18) PLID 28825 - When requiring a discount category, only allow a custom description if it is not blank
		CString strDescriptionTest = m_strCustomDescription;
		strDescriptionTest.TrimRight();
		if(bRequireDiscCategory && strDescriptionTest.IsEmpty()){
			MsgBox("A Discount Category or Discount Description is required before saving.");
			return;
		}

		// (j.gruber 2007-05-23 14:53) - PLID 24870 - make sure the description is not too long
		if (m_strCustomDescription.GetLength() > 255) {
			MsgBox("The length of the custom discount category exceeds 255 characters and will be truncated and then you can edit it.");
			m_strCustomDescription = m_strCustomDescription.Left(255);
			SetDlgItemText(IDC_CUSTOM_DISCOUNT_CAT, m_strCustomDescription);
			return;
		}

	}
	else {

		//see what the current selection is
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCatList->CurSel;
		if (pRow) {

			long nID = pRow->GetValue(0);

			if (nID == -2) {

				//todo - do something about coupons
				//we already have the couponId, so we should be good
				m_nDiscountCatID = -2;
				ASSERT(m_nCouponID != -1);
			}
			else {
				//(e.lally 2009-08-18) PLID 28825 - When requiring a discount category, check that one is selected
				if(bRequireDiscCategory && nID == -3){
					MsgBox("A Discount Category must be selected before saving.");
					return;
				}

				m_nDiscountCatID = nID;
			}
		}
		else {
			//it could be an inactive item
			if (m_pCatList->IsComboBoxTextInUse) {
				//it's inactive, so we aren't going to change anything
			}
			else {
				//(e.lally 2009-08-18) PLID 28825 - When requiring a discount category, check that one is selected
				if(bRequireDiscCategory){
					MsgBox("A Discount Category must be selected before saving.");
					return;
				}
				ASSERT(FALSE);
			}
		}
	}
	
		
		
	
	CDialog::OnOK();
}

void CDiscountCategorySelectDlg::OnTrySetSelFinishedDiscountCategoryList(long nRowEnum, long nFlags) 
{
	if(nFlags == NXDATALIST2Lib::dlTrySetSelFinishedFailure) {
		//It must be inactive.
		CString strDiscName = VarString(GetTableField("DiscountCategoriesT", "Description", "ID", m_nDiscountCatID),"");
		if (! strDiscName.IsEmpty()) {
			m_pCatList->PutComboBoxText(_bstr_t(strDiscName));
		}
	}
	
}

LRESULT CDiscountCategorySelectDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	if (m_bScanning) return 0;

	// (j.gruber 2007-05-14 14:45) - PLID 25173 - adding support for not showing coupons
	if (m_bShowCoupons) {

		m_bScanning = true;
		try {
			// (a.walling 2007-05-09 12:59) - PLID 25171 - We have a barcode; select the coupon if we can
			// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
			_bstr_t bstr = (BSTR)lParam; // We can typecast this to an ANSI string
			CString str = (LPCTSTR)bstr;


			CString strCouponName;

			ADODB::_RecordsetPtr prs = CreateRecordset("SELECT ID, Description, CASE WHEN CONVERT(datetime, Convert(nvarchar, StartDate, 1)) <= CONVERT(datetime, Convert(nvarchar, getDate(), 1)) THEN 1 ELSE 0 END AS Started, CASE WHEN EndDate >= CONVERT(datetime, Convert(nvarchar, getDate(), 1)) THEN 0 ELSE 1 END AS Expired FROM CouponsT WHERE BarCode = '%s'", _Q(str));
			if (!prs->eof) {
				strCouponName = AdoFldString(prs, "Description", "");

				BOOL bStarted = AdoFldLong(prs, "Started");
				BOOL bExpired = AdoFldLong(prs, "Expired");

				if (strCouponName.IsEmpty())
					strCouponName = "<No Coupon Description>";

				if (bExpired) {
					// since the coupon select allows this, we can continue with a warning.
					MessageBox(FormatString("(%s) - You have scanned an expired coupon!", strCouponName), NULL, MB_OK | MB_ICONEXCLAMATION);
					//return 0;
				} else if (!bStarted) {
					MessageBox(FormatString("(%s) - You have scanned a coupon that has not yet become active! Coupon will not be used.", strCouponName), NULL, MB_OK | MB_ICONHAND);
					return 0;
				}

				m_nCouponID = AdoFldLong(prs, "ID");

				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCatList->SetSelByColumn(0, _variant_t((long)-2));

				//add it to the datalist
				if (pRow)
					pRow->PutValue(1, _variant_t("<Coupon> - " + strCouponName));
			} else {
				// ignore
			}
		} NxCatchAll("Error in CDiscountCategorySelectDlg::OnBarcodeScan");

		m_bScanning = false;
	}

	return 0;
}
