// AdvRevCodeSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdvRevCodeSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CAdvRevCodeSetupDlg dialog


CAdvRevCodeSetupDlg::CAdvRevCodeSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdvRevCodeSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAdvRevCodeSetupDlg)
		m_bIsInv = FALSE;
	//}}AFX_DATA_INIT
}


void CAdvRevCodeSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvRevCodeSetupDlg)
	DDX_Control(pDX, IDC_RADIO_CPT, m_btnCpt);
	DDX_Control(pDX, IDC_RADIO_INV, m_btnInv);
	DDX_Control(pDX, IDC_UNSELECT_ONE_INS_CO, m_btnUnselectOneInsCo);
	DDX_Control(pDX, IDC_UNSELECT_ALL_INSCOS, m_btnUnselectAllInsCos);
	DDX_Control(pDX, IDC_SELECT_ALL_INSCOS, m_btnSelectAllInsCos);
	DDX_Control(pDX, IDC_SELECT_ONE_INS_CO, m_btnSelectOneInsCo);
	DDX_Control(pDX, IDC_UNSELECT_ALL_SERVICE_ITEMS, m_btnUnselectAllItems);
	DDX_Control(pDX, IDC_UNSELECT_ONE_SERVICE_ITEM, m_btnUnselectOneItem);
	DDX_Control(pDX, IDC_SELECT_ALL_SERVICE_ITEMS, m_btnSelectAllItems);
	DDX_Control(pDX, IDC_SELECT_ONE_SERVICE_ITEM, m_btnSelectOneItem);
	DDX_Control(pDX, IDC_UNSELECTED_ITEMS_LABEL, m_nxstaticUnselectedItemsLabel);
	DDX_Control(pDX, IDC_SELECTED_ITEMS_LABEL, m_nxstaticSelectedItemsLabel);
	DDX_Control(pDX, IDC_APPLY_REV_CODES, m_btnApplyRevCodes);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAdvRevCodeSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAdvRevCodeSetupDlg)
	ON_BN_CLICKED(IDC_SELECT_ONE_SERVICE_ITEM, OnSelectOneServiceItem)
	ON_BN_CLICKED(IDC_SELECT_ALL_SERVICE_ITEMS, OnSelectAllServiceItems)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_SERVICE_ITEM, OnUnselectOneServiceItem)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_SERVICE_ITEMS, OnUnselectAllServiceItems)
	ON_BN_CLICKED(IDC_APPLY_REV_CODES, OnApplyRevCodes)
	ON_BN_CLICKED(IDC_RADIO_CPT, OnRadioCpt)
	ON_BN_CLICKED(IDC_RADIO_INV, OnRadioInv)
	ON_BN_CLICKED(IDC_SELECT_ONE_INS_CO, OnSelectOneInsCo)
	ON_BN_CLICKED(IDC_SELECT_ALL_INSCOS, OnSelectAllInscos)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_INS_CO, OnUnselectOneInsCo)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_INSCOS, OnUnselectAllInscos)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvRevCodeSetupDlg message handlers

BOOL CAdvRevCodeSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnUnselectOneItem.AutoSet(NXB_LEFT);
	m_btnUnselectAllItems.AutoSet(NXB_LLEFT);
	m_btnSelectOneItem.AutoSet(NXB_RIGHT);
	m_btnSelectAllItems.AutoSet(NXB_RRIGHT);

	m_btnUnselectOneInsCo.AutoSet(NXB_LEFT);
	m_btnUnselectAllInsCos.AutoSet(NXB_LLEFT);
	m_btnSelectOneInsCo.AutoSet(NXB_RIGHT);
	m_btnSelectAllInsCos.AutoSet(NXB_RRIGHT);

	// (z.manning, 04/30/2008) - PLID 29850 - More button styles
	m_btnClose.AutoSet(NXB_CLOSE);
	m_btnApplyRevCodes.AutoSet(NXB_MODIFY);
	
	m_UnselectedServiceList = BindNxDataListCtrl(this, IDC_UNSELECTED_SERVICE_ITEM_LIST, GetRemoteData(), false);
	m_SelectedServiceList = BindNxDataListCtrl(this, IDC_SELECTED_SERVICE_ITEM_LIST, GetRemoteData(), false);
	m_UnselectedInsCoList = BindNxDataListCtrl(this, IDC_UNSELECTED_INSCO_LIST, GetRemoteData(), true);
	m_SelectedInsCoList = BindNxDataListCtrl(this, IDC_SELECTED_INSCO_LIST, GetRemoteData(), false);
	m_RevCodeCombo = BindNxDataListCtrl(this, IDC_REV_CODE_COMBO, GetRemoteData(), true);

	if(m_bIsInv) {
		CheckDlgButton(IDC_RADIO_INV,TRUE);
		OnRadioInv();
	}
	else {
		CheckDlgButton(IDC_RADIO_CPT,TRUE);
		OnRadioCpt();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAdvRevCodeSetupDlg::OnSelectOneServiceItem() 
{
	if(m_UnselectedServiceList->CurSel == -1)
		return;
	
	m_SelectedServiceList->TakeCurrentRow(m_UnselectedServiceList);
}

void CAdvRevCodeSetupDlg::OnSelectAllServiceItems() 
{
	m_SelectedServiceList->TakeAllRows(m_UnselectedServiceList);
}

void CAdvRevCodeSetupDlg::OnUnselectOneServiceItem() 
{
	if(m_SelectedServiceList->CurSel == -1)
		return;

	m_UnselectedServiceList->TakeCurrentRow(m_SelectedServiceList);
}

void CAdvRevCodeSetupDlg::OnUnselectAllServiceItems() 
{
	m_UnselectedServiceList->TakeAllRows(m_SelectedServiceList);	
}

void CAdvRevCodeSetupDlg::OnApplyRevCodes() 
{
	try {

		if(m_RevCodeCombo->CurSel == -1) {
			AfxMessageBox("Please select a revenue code.");
			return;
		}

		long count = m_SelectedServiceList->GetRowCount();

		if(count == 0) {
			AfxMessageBox("Please select at least one service item.");
			return;
		}

		if(m_SelectedInsCoList->GetRowCount() == 0) {

			//no insurance companies

			if(IDNO == MessageBox("You are about to change the revenue code for all the selected service items.\n"
				"Are you sure you wish to do this?","Practice",MB_YESNO|MB_ICONQUESTION)) {
				return;
			}

			CWaitCursor pWait;

			long UB92CategoryID = VarLong(m_RevCodeCombo->GetValue(m_RevCodeCombo->GetCurSel(),0));		

			for(int i=0;i<m_SelectedServiceList->GetRowCount(); i++) {
				long ServiceID = VarLong(m_SelectedServiceList->GetValue(i,0));
				ExecuteSql("UPDATE ServiceT SET UB92Category = %li, RevCodeUse = 1 WHERE ID = %li",UB92CategoryID,ServiceID);
			}

			AfxMessageBox("The selected revenue code has been assigned to all selected service items.");

		}
		else {

			//apply to insurance companies

			if(IDNO == MessageBox("You are about to change the revenue code for all of the selected service item / insurance company pairs.\n"
				"Are you sure you wish to do this?","Practice",MB_YESNO|MB_ICONQUESTION)) {
				return;
			}

			CWaitCursor pWait;

			long UB92CategoryID = VarLong(m_RevCodeCombo->GetValue(m_RevCodeCombo->GetCurSel(),0));		

			for(int i=0;i<m_SelectedServiceList->GetRowCount(); i++) {
				long ServiceID = VarLong(m_SelectedServiceList->GetValue(i,0));

				ExecuteSql("UPDATE ServiceT SET RevCodeUse = 2 WHERE ID = %li",ServiceID);

				for(int j=0;j<m_SelectedInsCoList->GetRowCount(); j++) {
					long InsuranceID = VarLong(m_SelectedInsCoList->GetValue(j,0));					

					CString str;
					str.Format(
						"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
						"UPDATE ServiceRevCodesT SET UB92CategoryID = %li WHERE InsuranceCompanyID = %li AND ServiceID = %li",
						UB92CategoryID, InsuranceID, ServiceID);
					_variant_t var;
					GetRemoteData()->Execute(_bstr_t(str),&var,adCmdText);
					if (var.lVal == 0) {
						str.Format("INSERT INTO ServiceRevCodesT (ServiceID, InsuranceCompanyID, UB92CategoryID) VALUES (%li, %li, %li)",
							ServiceID, InsuranceID, UB92CategoryID);
						ExecuteSql(str);
					}
				}
			}

			AfxMessageBox("The selected revenue code has been assigned to all selected service item / insurance company pairs.");

		}

	}NxCatchAll("Error applying revenue code.");
}

void CAdvRevCodeSetupDlg::OnRadioCpt() 
{
	if(m_SelectedServiceList->GetRowCount() > 0) {
		if(IDNO == MessageBox("Changing to the Service Code lists will unselect the items currently in the selected list. (No data will be changed, however.)\n"
			"Do you wish to continue?","Practice",MB_YESNO|MB_ICONQUESTION)) {
			CheckDlgButton(IDC_RADIO_INV,TRUE);
			return;
		}
		m_SelectedServiceList->Clear();
	}

	SetDlgItemText(IDC_UNSELECTED_ITEMS_LABEL,"Unselected Service Codes");
	SetDlgItemText(IDC_SELECTED_ITEMS_LABEL,"Selected Service Codes");

	m_UnselectedServiceList->GetColumn(1)->StoredWidth = 30;
	m_SelectedServiceList->GetColumn(1)->StoredWidth = 30;
	m_UnselectedServiceList->GetColumn(1)->PutFieldName("Code");
	m_SelectedServiceList->GetColumn(1)->PutFieldName("Code");

	m_UnselectedServiceList->FromClause = _bstr_t("CPTCodeT INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID");
	m_SelectedServiceList->FromClause = _bstr_t("CPTCodeT INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID");

	m_UnselectedServiceList->Requery();
}

void CAdvRevCodeSetupDlg::OnRadioInv() 
{
	if(m_SelectedServiceList->GetRowCount() > 0) {
		if(IDNO == MessageBox("Changing to the Inventory Item lists will unselect the items currently in the selected list. (No data will be changed, however.)\n"
			"Do you wish to continue?","Practice",MB_YESNO|MB_ICONQUESTION)) {
			CheckDlgButton(IDC_RADIO_CPT,TRUE);
			return;
		}
		m_SelectedServiceList->Clear();
	}

	SetDlgItemText(IDC_UNSELECTED_ITEMS_LABEL,"Unselected Inventory Items");
	SetDlgItemText(IDC_SELECTED_ITEMS_LABEL,"Selected Inventory Items");

	m_UnselectedServiceList->GetColumn(1)->StoredWidth = 0;
	m_SelectedServiceList->GetColumn(1)->StoredWidth = 0;
	m_UnselectedServiceList->GetColumn(1)->PutFieldName("Name");
	m_SelectedServiceList->GetColumn(1)->PutFieldName("Name");

	m_UnselectedServiceList->FromClause = _bstr_t("ProductT INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID");
	m_SelectedServiceList->FromClause = _bstr_t("ProductT INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID");

	m_UnselectedServiceList->Requery();
}

BEGIN_EVENTSINK_MAP(CAdvRevCodeSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAdvRevCodeSetupDlg)
	ON_EVENT(CAdvRevCodeSetupDlg, IDC_UNSELECTED_SERVICE_ITEM_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedServiceItemList, VTS_I4 VTS_I2)
	ON_EVENT(CAdvRevCodeSetupDlg, IDC_SELECTED_SERVICE_ITEM_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedServiceItemList, VTS_I4 VTS_I2)
	ON_EVENT(CAdvRevCodeSetupDlg, IDC_UNSELECTED_INSCO_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedInscoList, VTS_I4 VTS_I2)
	ON_EVENT(CAdvRevCodeSetupDlg, IDC_SELECTED_INSCO_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedInscoList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAdvRevCodeSetupDlg::OnDblClickCellUnselectedServiceItemList(long nRowIndex, short nColIndex) 
{
	m_UnselectedServiceList->CurSel = nRowIndex;
	OnSelectOneServiceItem();
}

void CAdvRevCodeSetupDlg::OnDblClickCellSelectedServiceItemList(long nRowIndex, short nColIndex) 
{
	m_SelectedServiceList->CurSel = nRowIndex;
	OnUnselectOneServiceItem();
}

void CAdvRevCodeSetupDlg::OnSelectOneInsCo() 
{
	if(m_UnselectedInsCoList->CurSel == -1)
		return;
	
	m_SelectedInsCoList->TakeCurrentRow(m_UnselectedInsCoList);
}

void CAdvRevCodeSetupDlg::OnSelectAllInscos() 
{
	m_SelectedInsCoList->TakeAllRows(m_UnselectedInsCoList);
}

void CAdvRevCodeSetupDlg::OnUnselectOneInsCo() 
{
	if(m_SelectedInsCoList->CurSel == -1)
		return;
	
	m_UnselectedInsCoList->TakeCurrentRow(m_SelectedInsCoList);
}

void CAdvRevCodeSetupDlg::OnUnselectAllInscos() 
{
	m_UnselectedInsCoList->TakeAllRows(m_SelectedInsCoList);	
}

void CAdvRevCodeSetupDlg::OnDblClickCellUnselectedInscoList(long nRowIndex, short nColIndex) 
{
	m_UnselectedInsCoList->CurSel = nRowIndex;
	OnSelectOneInsCo();
}

void CAdvRevCodeSetupDlg::OnDblClickCellSelectedInscoList(long nRowIndex, short nColIndex) 
{
	m_SelectedInsCoList->CurSel = nRowIndex;
	OnUnselectOneInsCo();
}
