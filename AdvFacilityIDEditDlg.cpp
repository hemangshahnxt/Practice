// AdvFacilityIDEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdvFacilityIDEditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CAdvFacilityIDEditDlg dialog


CAdvFacilityIDEditDlg::CAdvFacilityIDEditDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdvFacilityIDEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAdvFacilityIDEditDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAdvFacilityIDEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvFacilityIDEditDlg)
	DDX_Control(pDX, IDC_UNSELECT_ONE_INSCO, m_btnUnselectOneInsCo);
	DDX_Control(pDX, IDC_UNSELECT_ONE_LOC, m_btnUnselectOneLoc);
	DDX_Control(pDX, IDC_UNSELECT_ALL_LOC, m_btnUnselectAllLoc);
	DDX_Control(pDX, IDC_UNSELECT_ALL_INSCO, m_btnUnselectAllInsCo);
	DDX_Control(pDX, IDC_SELECT_ONE_LOC, m_btnSelectOneLoc);
	DDX_Control(pDX, IDC_SELECT_ONE_INSCO, m_btnSelectOneInsCo);
	DDX_Control(pDX, IDC_SELECT_ALL_LOC, m_btnSelectAllLoc);
	DDX_Control(pDX, IDC_SELECT_ALL_INSCO, m_btnSelectAllInsCo);
	DDX_Control(pDX, IDC_NEW_FACILITY_ID_QUAL, m_nxeditNewFacilityIdQual);
	DDX_Control(pDX, IDC_NEW_FACILITY_ID, m_nxeditNewFacilityId);
	DDX_Control(pDX, IDC_APPLY, m_btnApply);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_INS_LOC_GROUPBOX, m_btnInsLocGroupbox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAdvFacilityIDEditDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAdvFacilityIDEditDlg)
	ON_BN_CLICKED(IDC_SELECT_ONE_INSCO, OnSelectOneInsco)
	ON_BN_CLICKED(IDC_SELECT_ALL_INSCO, OnSelectAllInsco)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_INSCO, OnUnselectOneInsco)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_INSCO, OnUnselectAllInsco)
	ON_BN_CLICKED(IDC_SELECT_ONE_LOC, OnSelectOneLoc)
	ON_BN_CLICKED(IDC_SELECT_ALL_LOC, OnSelectAllLoc)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_LOC, OnUnselectOneLoc)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_LOC, OnUnselectAllLoc)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvFacilityIDEditDlg message handlers

BOOL CAdvFacilityIDEditDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		m_btnUnselectOneLoc.AutoSet(NXB_LEFT);
		m_btnUnselectOneInsCo.AutoSet(NXB_LEFT);
		m_btnUnselectAllLoc.AutoSet(NXB_LLEFT);
		m_btnUnselectAllInsCo.AutoSet(NXB_LLEFT);
		m_btnSelectAllLoc.AutoSet(NXB_RRIGHT);
		m_btnSelectAllInsCo.AutoSet(NXB_RRIGHT);
		m_btnSelectOneLoc.AutoSet(NXB_RIGHT);
		m_btnSelectOneInsCo.AutoSet(NXB_RIGHT);
		// (c.haag 2008-04-30 13:14) - PLID 29847 - NxIconify additional buttons
		m_btnApply.AutoSet(NXB_MODIFY);
		m_btnOK.AutoSet(NXB_CLOSE);

		m_SelectedInsCoList = BindNxDataListCtrl(this,IDC_SELECTED_INS_LIST,GetRemoteData(),false);
		m_UnselectedInsCoList = BindNxDataListCtrl(this,IDC_UNSELECTED_INS_LIST,GetRemoteData(),true);
		m_SelectedLocList = BindNxDataListCtrl(this,IDC_SELECTED_LOCATIONS_LIST,GetRemoteData(),false);
		m_UnselectedLocList = BindNxDataListCtrl(this,IDC_UNSELECTED_LOCATIONS_LIST,GetRemoteData(),true);
	}
	NxCatchAll("Error in CAdvFacilityIDEditDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CAdvFacilityIDEditDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAdvFacilityIDEditDlg)
	ON_EVENT(CAdvFacilityIDEditDlg, IDC_UNSELECTED_INS_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedInsList, VTS_I4 VTS_I2)
	ON_EVENT(CAdvFacilityIDEditDlg, IDC_SELECTED_INS_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedInsList, VTS_I4 VTS_I2)
	ON_EVENT(CAdvFacilityIDEditDlg, IDC_UNSELECTED_LOCATIONS_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedLocationsList, VTS_I4 VTS_I2)
	ON_EVENT(CAdvFacilityIDEditDlg, IDC_SELECTED_LOCATIONS_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedLocationsList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAdvFacilityIDEditDlg::OnDblClickCellUnselectedInsList(long nRowIndex, short nColIndex) 
{
	m_UnselectedInsCoList->CurSel = nRowIndex;
	OnSelectOneInsco();
}

void CAdvFacilityIDEditDlg::OnDblClickCellSelectedInsList(long nRowIndex, short nColIndex) 
{
	m_SelectedInsCoList->CurSel = nRowIndex;
	OnUnselectOneInsco();	
}

void CAdvFacilityIDEditDlg::OnDblClickCellUnselectedLocationsList(long nRowIndex, short nColIndex) 
{
	m_UnselectedLocList->CurSel = nRowIndex;
	OnSelectOneLoc();	
}

void CAdvFacilityIDEditDlg::OnDblClickCellSelectedLocationsList(long nRowIndex, short nColIndex) 
{
	m_SelectedLocList->CurSel = nRowIndex;
	OnUnselectOneLoc();	
}

void CAdvFacilityIDEditDlg::OnSelectOneInsco() 
{
	m_SelectedInsCoList->TakeCurrentRow(m_UnselectedInsCoList);
}

void CAdvFacilityIDEditDlg::OnSelectAllInsco() 
{
	m_SelectedInsCoList->TakeAllRows(m_UnselectedInsCoList);	
}

void CAdvFacilityIDEditDlg::OnUnselectOneInsco() 
{
	m_UnselectedInsCoList->TakeCurrentRow(m_SelectedInsCoList);	
}

void CAdvFacilityIDEditDlg::OnUnselectAllInsco() 
{
	m_UnselectedInsCoList->TakeAllRows(m_SelectedInsCoList);
}

void CAdvFacilityIDEditDlg::OnSelectOneLoc() 
{
	m_SelectedLocList->TakeCurrentRow(m_UnselectedLocList);
}

void CAdvFacilityIDEditDlg::OnSelectAllLoc() 
{
	m_SelectedLocList->TakeAllRows(m_UnselectedLocList);
}

void CAdvFacilityIDEditDlg::OnUnselectOneLoc() 
{
	m_UnselectedLocList->TakeCurrentRow(m_SelectedLocList);
}

void CAdvFacilityIDEditDlg::OnUnselectAllLoc() 
{
	m_UnselectedLocList->TakeAllRows(m_SelectedLocList);
}

void CAdvFacilityIDEditDlg::OnApply() 
{
	if(m_SelectedLocList->GetRowCount()==0) {
		AfxMessageBox("You must have at least one location selected.");
		return;
	}

	if(m_SelectedInsCoList->GetRowCount()==0) {
		AfxMessageBox("You must have at least one insurance company selected.");
		return;
	}

	CWaitCursor pWait;

	try {

		CString strNewID, strNewQual, str;

		GetDlgItemText(IDC_NEW_FACILITY_ID,strNewID);
		GetDlgItemText(IDC_NEW_FACILITY_ID_QUAL,strNewQual);

		str.Format("This action will update the Facility ID number to be '%s' and the qualifier to be '%s' for all selected location and insurance company pairs.\nAre you sure you wish to do this?", strNewID, strNewQual);

		if(IDNO==MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		for(int i=0;i<m_SelectedLocList->GetRowCount();i++) {
			for(int j=0;j<m_SelectedInsCoList->GetRowCount();j++) {
				
				long LocID, InsID;
				LocID = m_SelectedLocList->GetValue(i,0);
				InsID = m_SelectedInsCoList->GetValue(j,0);
				
				_RecordsetPtr rs;
				
				strNewID.TrimLeft();
				strNewID.TrimRight();

				if(strNewID != "" || strNewQual != "") {

					rs = CreateRecordset("SELECT Count(InsCoID) AS FacIDCount FROM InsuranceFacilityID WHERE InsCoID = %li AND LocationID = %li", InsID, LocID);

					if (rs->Fields->GetItem("FacIDCount")->Value.lVal > 0)
						ExecuteSql("UPDATE InsuranceFacilityID SET FacilityID = '%s', Qualifier = '%s' WHERE InsCoID = %d AND LocationID = %d", _Q(strNewID), _Q(strNewQual), InsID, LocID);
					else
						ExecuteSql("INSERT INTO InsuranceFacilityID (InsCoID, LocationID, FacilityID, Qualifier) VALUES ( %d, %d, '%s', '%s')", InsID, LocID, _Q(strNewID), _Q(strNewQual));
				}
				else {
					//if empty, remove the record
					ExecuteSql("DELETE FROM InsuranceFacilityID WHERE InsCoID = %li AND LocationID = %li",InsID,LocID);
				}
			}
		}

		AfxMessageBox("Update complete.");

	}NxCatchAll("Error applying new facility ID number.");
}
