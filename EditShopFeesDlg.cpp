// EditShopFeesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EditShopFeesDlg.h"
#include "AuditTrail.h"
#include "InvUtils.h"

// (j.gruber 2012-10-25 15:04) - PLID 53416 - created for

enum LocationListColumns{
	llcID = 0,
	llcName,
	llcShopFee,
};

// CEditShopFeesDlg dialog

IMPLEMENT_DYNAMIC(CEditShopFeesDlg, CNxDialog)

CEditShopFeesDlg::CEditShopFeesDlg(long nServiceID, CString strServiceName, COleCurrency cyStandardFee, COLORREF color, CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditShopFeesDlg::IDD, pParent)
{
	m_nServiceID = nServiceID;
	m_cyStandardFee = cyStandardFee;
	m_strServiceName = strServiceName;
	m_color = color;
}

CEditShopFeesDlg::~CEditShopFeesDlg()
{
}


void CEditShopFeesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ALL_RIGHT, m_btnMoveAllRight);
	DDX_Control(pDX, IDC_ALL_LEFT, m_btnMoveAllLeft);
	DDX_Control(pDX, IDC_ONE_RIGHT, m_btnMoveOneRight);
	DDX_Control(pDX, IDC_ONE_LEFT, m_btnMoveOneLeft);
	DDX_Control(pDX, IDC_APPLY, m_btnApply);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_SHOP_FEE_NXCOLOR, m_bkgColor);
}


BEGIN_MESSAGE_MAP(CEditShopFeesDlg, CNxDialog)
	ON_BN_CLICKED(IDC_APPLY, &CEditShopFeesDlg::OnBnClickedApply)
	ON_BN_CLICKED(IDC_ALL_RIGHT, &CEditShopFeesDlg::OnBnClickedAllRight)
	ON_BN_CLICKED(IDC_ONE_RIGHT, &CEditShopFeesDlg::OnBnClickedOneRight)
	ON_BN_CLICKED(IDC_ONE_LEFT, &CEditShopFeesDlg::OnBnClickedOneLeft)
	ON_BN_CLICKED(IDC_ALL_LEFT, &CEditShopFeesDlg::OnBnClickedAllLeft)
	ON_BN_CLICKED(IDOK, &CEditShopFeesDlg::OnBnClickedOk)
	ON_EN_KILLFOCUS(IDC_SHOP_FEE, &CEditShopFeesDlg::OnEnKillfocusShopFee)
END_MESSAGE_MAP()


// CEditShopFeesDlg message handlers

void CEditShopFeesDlg::FillSelectedLocationIDs(CArray<long, long> *paryLocations)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelectedList->GetFirstRow();
	while (pRow) {
		long nLocationID = VarLong(pRow->GetValue(llcID));
		paryLocations->Add(nLocationID);

		pRow = pRow->GetNextRow();
	}
}
void CEditShopFeesDlg::OnBnClickedApply()
{
	try {
		if (Validate()) {
			CArray<long, long> aryLocations;
			FillSelectedLocationIDs(&aryLocations);
			
			//audit first
			CString strOldValue = InvUtils::GetServiceLocationShopFeeAuditString(m_nServiceID, &aryLocations);

			//now save
			CString strShopFee;
			GetDlgItemText(IDC_SHOP_FEE, strShopFee);
			COleCurrency cyShopFee = ParseCurrencyFromInterface(strShopFee);
			
			ExecuteParamSql("UPDATE ServiceLocationInfoT SET ShopFee = CONVERT(money, {STRING}) WHERE ServiceID = {INT} AND LocationID IN ({INTARRAY})"
				,strShopFee, m_nServiceID, aryLocations);

			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, m_strServiceName, nAuditID, aeiShopFee, m_nServiceID, strOldValue, FormatCurrencyForInterface(cyShopFee), aepHigh, aetChanged);

			Reload();		
		}
	}NxCatchAll(__FUNCTION__);
}

BOOL CEditShopFeesDlg::Validate()
{
	CString strShopFee;
	GetDlgItemText(IDC_SHOP_FEE, strShopFee);
	COleCurrency cyTmp = ParseCurrencyFromInterface(strShopFee);

	BOOL bSuccess = TRUE;
	if(cyTmp.GetStatus() == COleCurrency::invalid) {
		AfxMessageBox("An invalid currency was entered as the shop fee.\n"
			"Please correct this.");
		bSuccess = FALSE;
	}

	//DRT 10/15/03 - must check for the previous condition to fail or we cannot examine cyTmp
	if(bSuccess && cyTmp < COleCurrency(0,0)) {
		AfxMessageBox("Practice does not allow a negative amount for a shop fee.\n"
			"Please correct this.");
		bSuccess = FALSE;
	}

	if(bSuccess && cyTmp > COleCurrency(100000000,0)) {
		CString str;
		str.Format("Practice does not allow a shop fee greater than %s.",FormatCurrencyForInterface(COleCurrency(100000000,0),TRUE,TRUE));
		AfxMessageBox(str);
		bSuccess = FALSE;
	}

	//DRT 4/23/2004 - The shop fee can't be larger than the cost of the item.
	COleCurrency cyItem;
	CString strTmp;	
	if(bSuccess && (cyTmp > m_cyStandardFee)) {
		MsgBox("You cannot enter a shop fee greater than the item's standard price.");
		bSuccess = FALSE;
	}

	if(bSuccess && !IsValidCurrencyText(strShopFee)) {
		MsgBox("Please enter a valid currency amount for the shop fee field.");
		bSuccess = FALSE;
	}

	//make sure that they selected a location
	if (bSuccess && m_pSelectedList->GetFirstRow() == NULL) {
		MsgBox("Please select a location to change the shop fee at.");
		bSuccess = FALSE;
	}

	return bSuccess;
}

void CEditShopFeesDlg::Reload()
{
	//clear the selected list
	m_pSelectedList->Clear();

	//set the avail list's where clause
	CString strWhere;
	strWhere.Format("(ServiceLocationInfoT.ServiceID = %li) AND LocationsT.Managed = 1 AND LocationsT.Active = 1", m_nServiceID);
	m_pAvailList->WhereClause = _bstr_t(strWhere);

	//requery the avail list
	m_pAvailList->Requery();

}

BOOL CEditShopFeesDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		m_btnMoveAllLeft.AutoSet(NXB_LLEFT);
		m_btnMoveAllRight.AutoSet(NXB_RRIGHT);
		m_btnMoveOneLeft.AutoSet(NXB_LEFT);
		m_btnMoveOneRight.AutoSet(NXB_RIGHT);

		m_btnApply.AutoSet(NXB_MODIFY);
		m_btnClose.AutoSet(NXB_CLOSE);	

		m_bkgColor.SetColor(m_color);

		m_pAvailList = BindNxDataList2Ctrl(IDC_AVAIL_LOCATION_LIST, false);		
		m_pSelectedList = BindNxDataList2Ctrl(IDC_SELECTED_LOCATION_LIST, false);	

		Reload();

		SetDlgItemText(IDC_STATIC_DESC, "Shop Fee(s) for " + m_strServiceName);

		
	}NxCatchAll(__FUNCTION__);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CEditShopFeesDlg::MoveOneLeft(NXDATALIST2Lib::IRowSettingsPtr pRowToMove) 
{
	m_pAvailList->TakeRowAddSorted(pRowToMove);
}

void CEditShopFeesDlg::MoveOneRight(NXDATALIST2Lib::IRowSettingsPtr pRowToMove) 
{
	m_pSelectedList->TakeRowAddSorted(pRowToMove);
}
void CEditShopFeesDlg::OnBnClickedAllRight()
{
	try {
		m_pSelectedList->TakeAllRows(m_pAvailList);
	}NxCatchAll(__FUNCTION__);
}

void CEditShopFeesDlg::OnBnClickedOneRight()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAvailList->CurSel;
		if (pRow) {
			MoveOneRight(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

void CEditShopFeesDlg::OnBnClickedOneLeft()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelectedList->CurSel;
		if (pRow) {
			MoveOneLeft(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

void CEditShopFeesDlg::OnBnClickedAllLeft()
{
	try {
		m_pAvailList->TakeAllRows(m_pSelectedList);
	}NxCatchAll(__FUNCTION__);
}

void CEditShopFeesDlg::OnBnClickedOk()
{
	try {
		OnOK();
	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CEditShopFeesDlg, CNxDialog)
	ON_EVENT(CEditShopFeesDlg, IDC_AVAIL_LOCATION_LIST, 3, CEditShopFeesDlg::DblClickCellAvailLocationList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEditShopFeesDlg, IDC_SELECTED_LOCATION_LIST, 3, CEditShopFeesDlg::DblClickCellSelectedLocationList, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CEditShopFeesDlg::DblClickCellAvailLocationList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			MoveOneRight(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

void CEditShopFeesDlg::DblClickCellSelectedLocationList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			MoveOneLeft(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

void CEditShopFeesDlg::OnEnKillfocusShopFee()
{
	try {
		CString str;
		COleCurrency cyTmp;
		GetDlgItemText(IDC_SHOP_FEE, str);
		cyTmp = ParseCurrencyFromInterface(str);

		str = FormatCurrencyForInterface(cyTmp);
		SetDlgItemText(IDC_SHOP_FEE, str);
	}NxCatchAll(__FUNCTION__);
}
