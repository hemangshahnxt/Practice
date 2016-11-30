// AdvSurgeryItemsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdvSurgeryItemsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAdvSurgeryItemsDlg dialog


CAdvSurgeryItemsDlg::CAdvSurgeryItemsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdvSurgeryItemsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAdvSurgeryItemsDlg)
	//}}AFX_DATA_INIT
}


void CAdvSurgeryItemsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvSurgeryItemsDlg)
	DDX_Control(pDX, IDC_UNSELECT_ONE_SURGERY, m_btnUnselectOneSurgery);
	DDX_Control(pDX, IDC_UNSELECT_ONE_INV, m_btnUnselectOneInv);
	DDX_Control(pDX, IDC_UNSELECT_ALL_SURGERIES, m_btnUnselectAllSurgeries);
	DDX_Control(pDX, IDC_UNSELECT_ALL_INV, m_btnUnselectAllInv);
	DDX_Control(pDX, IDC_SELECT_ONE_SURGERY, m_btnSelectOneSurgery);
	DDX_Control(pDX, IDC_SELECT_ONE_INV, m_btnSelectOneInv);
	DDX_Control(pDX, IDC_SELECT_ALL_SURGERIES, m_btnSelectAllSurgeries);
	DDX_Control(pDX, IDC_SELECT_ALL_INV, m_btnSelectAllInv);
	DDX_Control(pDX, IDC_UNSELECT_ALL_CPT, m_btnUnselectAllCPT);
	DDX_Control(pDX, IDC_UNSELECT_ONE_CPT, m_btnUnselectOneCPT);
	DDX_Control(pDX, IDC_SELECT_ONE_CPT, m_btnSelectOneCPT);
	DDX_Control(pDX, IDC_SELECT_ALL_CPT, m_btnSelectAllCPT);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_APPLY, m_btnApply);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAdvSurgeryItemsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAdvSurgeryItemsDlg)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	ON_BN_CLICKED(IDC_SELECT_ONE_SURGERY, OnSelectOneSurgery)
	ON_BN_CLICKED(IDC_SELECT_ALL_SURGERIES, OnSelectAllSurgeries)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_SURGERY, OnUnselectOneSurgery)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_SURGERIES, OnUnselectAllSurgeries)
	ON_BN_CLICKED(IDC_SELECT_ONE_CPT, OnSelectOneCpt)
	ON_BN_CLICKED(IDC_SELECT_ALL_CPT, OnSelectAllCpt)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_CPT, OnUnselectOneCpt)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_CPT, OnUnselectAllCpt)
	ON_BN_CLICKED(IDC_SELECT_ONE_INV, OnSelectOneInv)
	ON_BN_CLICKED(IDC_SELECT_ALL_INV, OnSelectAllInv)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_INV, OnUnselectOneInv)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_INV, OnUnselectAllInv)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvSurgeryItemsDlg message handlers

BOOL CAdvSurgeryItemsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnUnselectOneSurgery.AutoSet(NXB_LEFT);
	m_btnUnselectOneInv.AutoSet(NXB_LEFT);
	m_btnUnselectAllSurgeries.AutoSet(NXB_LLEFT);
	m_btnUnselectAllInv.AutoSet(NXB_LLEFT);
	m_btnSelectOneSurgery.AutoSet(NXB_RIGHT);
	m_btnSelectOneInv.AutoSet(NXB_RIGHT);
	m_btnSelectAllSurgeries.AutoSet(NXB_RRIGHT);
	m_btnSelectAllInv.AutoSet(NXB_RRIGHT);
	m_btnUnselectAllCPT.AutoSet(NXB_LLEFT);
	m_btnUnselectOneCPT.AutoSet(NXB_LEFT);
	m_btnSelectOneCPT.AutoSet(NXB_RIGHT);
	m_btnSelectAllCPT.AutoSet(NXB_RRIGHT);
	m_btnClose.AutoSet(NXB_CLOSE); // (z.manning, 04/30/2008) - PLID 29850
	m_btnApply.AutoSet(NXB_MODIFY);

	m_UnselectedSurgeries = BindNxDataListCtrl(this,IDC_UNSELECTED_SURGERY_LIST,GetRemoteData(), true);
	m_SelectedSurgeries = BindNxDataListCtrl(this,IDC_SELECTED_SURGERY_LIST,GetRemoteData(), false);
	m_UnselectedCPTs = BindNxDataListCtrl(this,IDC_UNSELECTED_CPT_LIST,GetRemoteData(), true);
	m_SelectedCPTs = BindNxDataListCtrl(this,IDC_SELECTED_CPT_LIST,GetRemoteData(), false);
	m_UnselectedInv = BindNxDataListCtrl(this,IDC_UNSELECTED_INV_LIST,GetRemoteData(), true);
	m_SelectedInv = BindNxDataListCtrl(this,IDC_SELECTED_INV_LIST,GetRemoteData(), false);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAdvSurgeryItemsDlg::OnApply() 
{
	try {

		//first make sure at least one CPT or Product is selected
		if(m_SelectedCPTs->GetRowCount() == 0 && m_SelectedInv->GetRowCount() == 0) {
			AfxMessageBox("You must have at least one Service Code or Inventory Item selected.");
			return;
		}

		//now make sure at least one Surgery is selected
		if(m_SelectedSurgeries->GetRowCount() == 0) {
			AfxMessageBox("You must select at least one surgery to add the selected services.");
			return;
		}

		//warn them about what they are doing, give them a chance to cancel
		if(IDNO == MessageBox("You are about to add all of the selected Service Codes and Inventory Items to all of the selected Surgeries.\n\n"
			"Are you sure you wish to do this?","Practice",MB_YESNO|MB_ICONQUESTION)) {
			return;
		}

		CWaitCursor pWait;

		//okay, now we can proceed

		for(int i=0; i<m_SelectedSurgeries->GetRowCount(); i++) {
			//loop through each surgery

			long nSurgeryID = m_SelectedSurgeries->GetValue(i,0).lVal;
			
			//now loop through each CPT code and add it
			for(int a=0;a<m_SelectedCPTs->GetRowCount();a++) {

				long nCPTID = m_SelectedCPTs->GetValue(a,0).lVal;

				int nNewID = NewNumber("SurgeryDetailsT", "ID");

				// (j.jones 2009-08-24 10:32) - PLID 35271 - removed Billable
				ExecuteParamSql("INSERT INTO SurgeryDetailsT (ID, SurgeryID, ServiceID, Amount, Quantity, PayToPractice) "
					"SELECT {INT}, {INT}, ID, Price, 1, -1 FROM ServiceT WHERE ID = {INT};", nNewID,
					 nSurgeryID, nCPTID);			
			}

			//now loop through each Product and add it
			for(int b=0;b<m_SelectedInv->GetRowCount();b++) {

				long nProductID = m_SelectedInv->GetValue(b,0).lVal;

				int nNewID = NewNumber("SurgeryDetailsT", "ID");

				// (j.jones 2008-04-28 09:51) - PLID 21338 - ensured we put the cost in the surgery
				// (j.jones 2009-08-24 10:32) - PLID 35271 - removed Billable and Cost
				ExecuteParamSql("INSERT INTO SurgeryDetailsT (ID, SurgeryID, ServiceID, Amount, Quantity, PayToPractice) "
					"SELECT {INT}, {INT}, ServiceT.ID, Price, 1.0, -1 "
					"FROM ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
					"WHERE ServiceT.ID = {INT}", nNewID, nSurgeryID, nProductID);
			}

		}

		CClient::RefreshTable(NetUtils::SurgeriesT);

		AfxMessageBox("The selected items have been added to each of the selected surgeries.");

		//now that we are done, clear the selected lists
		OnUnselectAllSurgeries();
		OnUnselectAllCpt();
		OnUnselectAllInv();
		
	}NxCatchAll("Error applying selected items to surgeries.");
}

void CAdvSurgeryItemsDlg::OnSelectOneSurgery() 
{
	if(m_UnselectedSurgeries->CurSel == -1)
		return;

	m_SelectedSurgeries->TakeCurrentRow(m_UnselectedSurgeries);
}

void CAdvSurgeryItemsDlg::OnSelectAllSurgeries() 
{
	m_SelectedSurgeries->TakeAllRows(m_UnselectedSurgeries);	
}

void CAdvSurgeryItemsDlg::OnUnselectOneSurgery() 
{
	if(m_SelectedSurgeries->CurSel == -1)
		return;

	m_UnselectedSurgeries->TakeCurrentRow(m_SelectedSurgeries);	
}

void CAdvSurgeryItemsDlg::OnUnselectAllSurgeries() 
{
	m_UnselectedSurgeries->TakeAllRows(m_SelectedSurgeries);
}

void CAdvSurgeryItemsDlg::OnSelectOneCpt() 
{
	if(m_UnselectedCPTs->CurSel == -1)
		return;
	
	m_SelectedCPTs->TakeCurrentRow(m_UnselectedCPTs);
}

void CAdvSurgeryItemsDlg::OnSelectAllCpt() 
{
	m_SelectedCPTs->TakeAllRows(m_UnselectedCPTs);	
}

void CAdvSurgeryItemsDlg::OnUnselectOneCpt() 
{
	if(m_SelectedCPTs->CurSel == -1)
		return;

	m_UnselectedCPTs->TakeCurrentRow(m_SelectedCPTs);
}

void CAdvSurgeryItemsDlg::OnUnselectAllCpt() 
{
	m_UnselectedCPTs->TakeAllRows(m_SelectedCPTs);	
}

void CAdvSurgeryItemsDlg::OnSelectOneInv() 
{
	if(m_UnselectedInv->CurSel == -1)
		return;

	m_SelectedInv->TakeCurrentRow(m_UnselectedInv);
}

void CAdvSurgeryItemsDlg::OnSelectAllInv() 
{
	m_SelectedInv->TakeAllRows(m_UnselectedInv);
}

void CAdvSurgeryItemsDlg::OnUnselectOneInv() 
{
	if(m_SelectedInv->CurSel == -1)
		return;

	m_UnselectedInv->TakeCurrentRow(m_SelectedInv);
}

void CAdvSurgeryItemsDlg::OnUnselectAllInv() 
{
	m_UnselectedInv->TakeAllRows(m_SelectedInv);	
}

BEGIN_EVENTSINK_MAP(CAdvSurgeryItemsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAdvSurgeryItemsDlg)
	ON_EVENT(CAdvSurgeryItemsDlg, IDC_UNSELECTED_SURGERY_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedSurgeryList, VTS_I4 VTS_I2)
	ON_EVENT(CAdvSurgeryItemsDlg, IDC_SELECTED_SURGERY_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedSurgeryList, VTS_I4 VTS_I2)
	ON_EVENT(CAdvSurgeryItemsDlg, IDC_UNSELECTED_CPT_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedCptList, VTS_I4 VTS_I2)
	ON_EVENT(CAdvSurgeryItemsDlg, IDC_SELECTED_CPT_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedCptList, VTS_I4 VTS_I2)
	ON_EVENT(CAdvSurgeryItemsDlg, IDC_UNSELECTED_INV_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedInvList, VTS_I4 VTS_I2)
	ON_EVENT(CAdvSurgeryItemsDlg, IDC_SELECTED_INV_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedInvList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAdvSurgeryItemsDlg::OnDblClickCellUnselectedSurgeryList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1)
		return;

	m_UnselectedSurgeries->CurSel = nRowIndex;

	OnSelectOneSurgery();	
}

void CAdvSurgeryItemsDlg::OnDblClickCellSelectedSurgeryList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1)
		return;

	m_SelectedSurgeries->CurSel = nRowIndex;

	OnUnselectOneSurgery();
}

void CAdvSurgeryItemsDlg::OnDblClickCellUnselectedCptList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1)
		return;

	m_UnselectedCPTs->CurSel = nRowIndex;

	OnSelectOneCpt();
}

void CAdvSurgeryItemsDlg::OnDblClickCellSelectedCptList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1)
		return;

	m_SelectedCPTs->CurSel = nRowIndex;

	OnUnselectOneCpt();
}

void CAdvSurgeryItemsDlg::OnDblClickCellUnselectedInvList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1)
		return;

	m_UnselectedInv->CurSel = nRowIndex;

	OnSelectOneInv();
}

void CAdvSurgeryItemsDlg::OnDblClickCellSelectedInvList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1)
		return;

	m_SelectedInv->CurSel = nRowIndex;

	OnUnselectOneInv();
}
