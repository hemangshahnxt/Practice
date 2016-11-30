// UpdateFeesByRVUDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UpdateFeesByRVUDlg.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUpdateFeesByRVUDlg dialog

using namespace ADODB;

#define COLUMN_SERVICE_ID	0
#define COLUMN_IS_ANESTH	1
#define COLUMN_IS_FACILITY	2
#define COLUMN_CPT_CODE		3
#define COLUMN_CPT_SUBCODE	4
#define COLUMN_DESCRIPTION	5
#define COLUMN_RVU			6
#define COLUMN_PRICE		7
#define COLUMN_NEW_PRICE	8

CUpdateFeesByRVUDlg::CUpdateFeesByRVUDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUpdateFeesByRVUDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpdateFeesByRVUDlg)
		m_FeeScheduleID = -1;
		m_dblConversionFactor = 0.0;
	//}}AFX_DATA_INIT
}


void CUpdateFeesByRVUDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpdateFeesByRVUDlg)
	DDX_Control(pDX, IDC_BTN_CALCULATE_PRICES, m_btnCalculatePrices);
	DDX_Control(pDX, IDC_BTN_APPLY_NEW_FEES, m_btnApplyNewFees);
	DDX_Control(pDX, IDC_MOVE_ONE_CPT_UP, m_btnMoveOneCPTUp);
	DDX_Control(pDX, IDC_BTN_MOVE_ONE_CPT_DOWN, m_btnMoveOneCPTDown);
	DDX_Control(pDX, IDC_BTN_MOVE_ALL_CPT_UP, m_btnMoveAllCPTUp);
	DDX_Control(pDX, IDC_BTN_MOVE_ALL_CPT_DOWN, m_btnMoveAllCPTDown);
	DDX_Control(pDX, IDC_CONVERSION_FACTOR, m_nxeditConversionFactor);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUpdateFeesByRVUDlg, CNxDialog)
	//{{AFX_MSG_MAP(CUpdateFeesByRVUDlg)
	ON_BN_CLICKED(IDC_BTN_MOVE_ONE_CPT_DOWN, OnBtnMoveOneCptDown)
	ON_BN_CLICKED(IDC_BTN_MOVE_ALL_CPT_DOWN, OnBtnMoveAllCptDown)
	ON_BN_CLICKED(IDC_MOVE_ONE_CPT_UP, OnMoveOneCptUp)
	ON_BN_CLICKED(IDC_BTN_MOVE_ALL_CPT_UP, OnBtnMoveAllCptUp)
	ON_BN_CLICKED(IDC_BTN_CALCULATE_PRICES, OnBtnCalculatePrices)
	ON_BN_CLICKED(IDC_BTN_APPLY_NEW_FEES, OnBtnApplyNewFees)
	ON_EN_CHANGE(IDC_CONVERSION_FACTOR, OnChangeConversionFactor)
	ON_EN_KILLFOCUS(IDC_CONVERSION_FACTOR, OnKillfocusConversionFactor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateFeesByRVUDlg message handlers

BOOL CUpdateFeesByRVUDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnMoveOneCPTDown.SetIcon(IDI_DARROW);
	m_btnMoveOneCPTUp.SetIcon(IDI_UARROW);
	m_btnMoveAllCPTDown.SetIcon(IDI_DDARROW);
	m_btnMoveAllCPTUp.SetIcon(IDI_UUARROW);
	m_btnClose.AutoSet(NXB_CLOSE);
	m_btnApplyNewFees.AutoSet(NXB_MODIFY);
	
	m_UnselectedList = BindNxDataListCtrl(this,IDC_UNSELECTED_CPT_RVU_LIST,GetRemoteData(),false);
	m_SelectedList = BindNxDataListCtrl(this,IDC_SELECTED_CPT_RVU_LIST,GetRemoteData(),false);

	if(m_FeeScheduleID != -1) {
		SetWindowText("Update Fee Schedule Prices By RVU");
		CString str;
		str.Format("CPTCodeT INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID LEFT OUTER JOIN (SELECT MultiFeeItemsT.* FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li) AS MultiFeeItemsT ON CPTCodeT.ID = MultiFeeItemsT.ServiceID",m_FeeScheduleID);
		m_UnselectedList->FromClause = _bstr_t(str);
		m_UnselectedList->GetColumn(COLUMN_PRICE)->FieldName = "MultiFeeItemsT.Price";
	}	

	CString strWhere = "Active = 1 AND RVU > 0 AND UseAnesthesiaBilling = 0 AND UseFacilityBilling = 0 ";
	m_UnselectedList->WhereClause = _bstr_t(strWhere);

	m_UnselectedList->Requery();

	m_UnselectedList->GetColumn(COLUMN_NEW_PRICE)->PutBackColor(RGB(220,220,255));
	m_SelectedList->GetColumn(COLUMN_NEW_PRICE)->PutBackColor(RGB(220,220,255));

	//disable this button by default
	m_btnApplyNewFees.EnableWindow(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUpdateFeesByRVUDlg::OnBtnMoveOneCptDown() 
{
	try {

		if(m_UnselectedList->CurSel == -1) {
			AfxMessageBox("Please select a service code from the unselected list.");
			return;
		}

		m_SelectedList->TakeCurrentRow(m_UnselectedList);
		m_btnApplyNewFees.EnableWindow(FALSE);

	}NxCatchAll("Error moving service code.");
}

void CUpdateFeesByRVUDlg::OnBtnMoveAllCptDown() 
{
	try {

		m_SelectedList->TakeAllRows(m_UnselectedList);
		m_btnApplyNewFees.EnableWindow(FALSE);

	}NxCatchAll("Error moving service codes.");
}

void CUpdateFeesByRVUDlg::OnMoveOneCptUp() 
{
	try {

		if(m_SelectedList->CurSel == -1) {
			AfxMessageBox("Please select a service code from the selected list.");
			return;
		}

		m_UnselectedList->TakeCurrentRow(m_SelectedList);

		if(m_SelectedList->GetRowCount() == 0)
			m_btnApplyNewFees.EnableWindow(FALSE);
		
		ClearUnselectedNewFees();

	}NxCatchAll("Error moving service code.");
}

void CUpdateFeesByRVUDlg::OnBtnMoveAllCptUp() 
{
	try {

		m_UnselectedList->TakeAllRows(m_SelectedList);

		m_btnApplyNewFees.EnableWindow(FALSE);

		ClearUnselectedNewFees();

	}NxCatchAll("Error moving service codes.");
}

BEGIN_EVENTSINK_MAP(CUpdateFeesByRVUDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CUpdateFeesByRVUDlg)
	ON_EVENT(CUpdateFeesByRVUDlg, IDC_UNSELECTED_CPT_RVU_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedCptRvuList, VTS_I4 VTS_I2)
	ON_EVENT(CUpdateFeesByRVUDlg, IDC_SELECTED_CPT_RVU_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedCptRvuList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CUpdateFeesByRVUDlg::OnDblClickCellUnselectedCptRvuList(long nRowIndex, short nColIndex) 
{
	m_UnselectedList->CurSel = nRowIndex;

	if(nRowIndex != -1)
		OnBtnMoveOneCptDown();
}

void CUpdateFeesByRVUDlg::OnDblClickCellSelectedCptRvuList(long nRowIndex, short nColIndex) 
{
	m_SelectedList->CurSel = nRowIndex;

	if(nRowIndex != -1)
		OnMoveOneCptUp();
}

void CUpdateFeesByRVUDlg::ClearUnselectedNewFees()
{
	try {

		_variant_t varNull;
		varNull.vt = VT_NULL;

		for(int i=0;i<m_UnselectedList->GetRowCount();i++) {
			m_UnselectedList->PutValue(i,COLUMN_NEW_PRICE,varNull);
		}

	}NxCatchAll("Error clearing fees.");
}

void CUpdateFeesByRVUDlg::OnBtnCalculatePrices() 
{
	try {

		if(m_SelectedList->GetRowCount() == 0) {
			AfxMessageBox("Please select at least one Service Code.");
			return;
		}

		CString strConversionFactor;
		GetDlgItemText(IDC_CONVERSION_FACTOR, strConversionFactor);
		m_dblConversionFactor = atof((LPCTSTR)strConversionFactor);

		if(m_dblConversionFactor <= 0.0) {
			AfxMessageBox("Please enter a positive Conversion Rate.");
			return;
		}

		BOOL bWarn = FALSE;

		CWaitCursor pWait;

		//first unselect all codes with an RVU of 0.0 or if they are Anesthesia Codes or Facility Fee codes
		for(int i=m_SelectedList->GetRowCount()-1;i>=0;i--) {
			_variant_t varRVU = m_SelectedList->GetValue(i,COLUMN_RVU);
			double dblRVU = VarDouble(varRVU,0.0);

			if(dblRVU == 0.0) {
				//remove from the list
				bWarn = TRUE;
				m_UnselectedList->TakeRow(m_SelectedList->GetRow(i));
			}
			else if(VarBool(m_SelectedList->GetValue(i,COLUMN_IS_ANESTH))) {
				//remove from the list
				bWarn = TRUE;
				m_UnselectedList->TakeRow(m_SelectedList->GetRow(i));
			}
			else if(VarBool(m_SelectedList->GetValue(i,COLUMN_IS_FACILITY))) {
				//remove from the list
				bWarn = TRUE;
				m_UnselectedList->TakeRow(m_SelectedList->GetRow(i));
			}
		}

		//warn if we removed any
		if(bWarn) {
			ClearUnselectedNewFees();
			AfxMessageBox("You selected at least one Service Code that could not be updated due to having\n"
				"no RVU value, was an Anesthesia code, or was a Facility Fee code.\n"
				"All non-updateable Service Codes have been returned to the unselected list.\n"
				"Please press the \"Calculate New Fees\" button again to calculate the prices\n"
				"with the currently selected Service Codes.");
			return;
		}
		
		for(i=0;i<m_SelectedList->GetRowCount();i++) {
			_variant_t varRVU = m_SelectedList->GetValue(i,COLUMN_RVU);
			double dblRVU = VarDouble(varRVU,0.0);
			double dblNewAmount = dblRVU * m_dblConversionFactor;
			CString str;
			str.Format("%f", dblNewAmount);
			COleCurrency cyNewAmount;
			cyNewAmount.ParseCurrency(str);
			RoundCurrency(cyNewAmount);
			m_SelectedList->PutValue(i,COLUMN_NEW_PRICE,_variant_t(cyNewAmount));
		}
	
		//if successful, show enable the "apply" button
		m_btnApplyNewFees.EnableWindow(TRUE);

		AfxMessageBox("Please verify the contents of the \"New Price\" field,\n"
					  "and click \"Apply New Fees\" when you are ready to save the new prices.");

	}NxCatchAll("Error calculating new fees.");
}

void CUpdateFeesByRVUDlg::OnBtnApplyNewFees() 
{
	try {

		if(IDNO == MessageBox("This will update all selected Service Codes to use the \"New Price\".\n\n"
			"Are you sure you wish to do this?","Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		CWaitCursor pWait;

		BOOL bAnesFacSkipped = FALSE;

		for(int i=0;i<m_SelectedList->GetRowCount();i++) {
			_variant_t varNewPrice = m_SelectedList->GetValue(i,COLUMN_NEW_PRICE);
			COleCurrency cyNewPrice;
			if(varNewPrice.vt == VT_CY) {
				cyNewPrice = VarCurrency(varNewPrice);

				BOOL bIndivSkipped = FALSE;

				long ServiceID = VarLong(m_SelectedList->GetValue(i,COLUMN_SERVICE_ID));

				if(m_FeeScheduleID != -1) {
					//insert/update the multifee
					_RecordsetPtr rs = CreateRecordset("SELECT Count(ServiceID) AS ItemCount FROM MultiFeeItemsT WHERE ServiceID = %li AND FeeGroupID = %li", ServiceID, m_FeeScheduleID);
					if(VarLong(rs->Fields->GetItem("ItemCount")->Value, 0) != 0)
						ExecuteSql("UPDATE MultiFeeItemsT SET Price = Convert(money,%s) WHERE ServiceID = %li AND FeeGroupID = %li", FormatCurrencyForSql(cyNewPrice), ServiceID, m_FeeScheduleID);
					else
						ExecuteSql("INSERT INTO MultiFeeItemsT (FeeGroupID, ServiceID, Price) VALUES (%li, %li, Convert(money,%s))", m_FeeScheduleID, ServiceID, FormatCurrencyForSql(cyNewPrice));
					rs->Close();
				}
				else {
					//update the standard fee

					//only do so if the fee is not an anesthesia code, if anesthesia billing is enabled
					if(!VarBool(m_SelectedList->GetValue(i,COLUMN_IS_ANESTH))
						&& !VarBool(m_SelectedList->GetValue(i,COLUMN_IS_FACILITY))) {

						ExecuteSql("UPDATE ServiceT SET Price = Convert(money,%s) WHERE ID = %li",FormatCurrencyForSql(cyNewPrice),ServiceID);
					}
					else {
						bAnesFacSkipped = TRUE;
						bIndivSkipped = TRUE;
					}
				}

				if(!bIndivSkipped)
					m_SelectedList->PutValue(i,COLUMN_PRICE,_variant_t(cyNewPrice));
			}
		}

		m_UnselectedList->TakeAllRows(m_SelectedList);
		ClearUnselectedNewFees();

		m_btnApplyNewFees.EnableWindow(FALSE);

		if(!bAnesFacSkipped)
			AfxMessageBox("All selected Service Codes were updated successfully.");
		else
			AfxMessageBox("Service Codes were updated successfully.\n"
				"Some codes were anesthesia/facility fee codes and were skipped, as you cannot update the standard fee for these codes.");

	}NxCatchAll("Error applying new fees.");
}

void CUpdateFeesByRVUDlg::OnChangeConversionFactor() 
{
	try {

		CString strConversionFactor;
		GetDlgItemText(IDC_CONVERSION_FACTOR, strConversionFactor);
		double dblConversion = atof((LPCTSTR)strConversionFactor);

		if(m_dblConversionFactor != dblConversion)			
			//if they change the conversion factor, disable the "apply" button immediately
			m_btnApplyNewFees.EnableWindow(FALSE);
		else
			m_btnApplyNewFees.EnableWindow(TRUE);

	}NxCatchAll("Error changing conversion factor.");
}

void CUpdateFeesByRVUDlg::OnKillfocusConversionFactor() 
{
	try {

		CString strConversionFactor;
		GetDlgItemText(IDC_CONVERSION_FACTOR, strConversionFactor);
		double dblConversion = atof((LPCTSTR)strConversionFactor);
		CString str;
		str.Format("%f", dblConversion);
		m_dblConversionFactor = dblConversion;
		SetDlgItemText(IDC_CONVERSION_FACTOR, str);

	}NxCatchAll("Error checking conversion factor.");
}
