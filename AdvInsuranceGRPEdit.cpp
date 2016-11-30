// AdvInsuranceGRPEdit.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "pracprops.h"
#include "AdvInsuranceGRPEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CAdvInsuranceGRPEdit dialog


CAdvInsuranceGRPEdit::CAdvInsuranceGRPEdit(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdvInsuranceGRPEdit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAdvInsuranceGRPEdit)
		m_IDType = 1;
	//}}AFX_DATA_INIT
}


void CAdvInsuranceGRPEdit::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvInsuranceGRPEdit)
	DDX_Control(pDX, IDC_RADIO_GRP, m_btnGrp);
	DDX_Control(pDX, IDC_RADIO_BOX24J, m_btn24J);
	DDX_Control(pDX, IDC_RADIO_BOX31, m_btnBox31);
	DDX_Control(pDX, IDC_RADIO_NETWORKID, m_btnNetworkID);
	DDX_Control(pDX, IDC_RADIO_BOX51, m_btnBox51);
	DDX_Control(pDX, IDC_UNSELECT_ONE_PROV, m_btnUnselOneProv);
	DDX_Control(pDX, IDC_UNSELECT_ONE_INSCO, m_btnUnselOneInsco);
	DDX_Control(pDX, IDC_UNSELECT_ALL_PROV, m_btnUnselAllProv);
	DDX_Control(pDX, IDC_UNSELECT_ALL_INSCO, m_btnUnselAllInsco);
	DDX_Control(pDX, IDC_SELECT_ALL_PROV, m_btnSelAllProv);
	DDX_Control(pDX, IDC_SELECT_ALL_INSCO, m_btnSelAllInsco);
	DDX_Control(pDX, IDC_SELECT_ONE_PROV, m_btnSelOneProv);
	DDX_Control(pDX, IDC_SELECT_ONE_INSCO, m_btnSelOneInsco);
	DDX_Control(pDX, IDC_EDIT_BOX24I_QUAL, m_nxeditEditBox24iQual);
	DDX_Control(pDX, IDC_NEW_ID_NUMBER, m_nxeditNewIdNumber);
	DDX_Control(pDX, IDC_BOX24I_QUAL_LABEL, m_nxstaticBox24iQualLabel);
	DDX_Control(pDX, IDC_ID_LABEL, m_nxstaticIdLabel);
	DDX_Control(pDX, IDC_APPLY, m_btnApply);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_INS_PROV_GROUPBOX, m_btnInsProvGroupbox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAdvInsuranceGRPEdit, CNxDialog)
	//{{AFX_MSG_MAP(CAdvInsuranceGRPEdit)
	ON_BN_CLICKED(IDC_RADIO_BOX24J, OnRadioBox24J)
	ON_BN_CLICKED(IDC_RADIO_GRP, OnRadioGrp)
	ON_BN_CLICKED(IDC_SELECT_ONE_INSCO, OnSelectOneInsco)
	ON_BN_CLICKED(IDC_SELECT_ALL_INSCO, OnSelectAllInsco)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_INSCO, OnUnselectOneInsco)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_INSCO, OnUnselectAllInsco)
	ON_BN_CLICKED(IDC_SELECT_ONE_PROV, OnSelectOneProv)
	ON_BN_CLICKED(IDC_SELECT_ALL_PROV, OnSelectAllProv)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_PROV, OnUnselectOneProv)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_PROV, OnUnselectAllProv)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	ON_BN_CLICKED(IDC_RADIO_NETWORKID, OnRadioNetworkid)
	ON_BN_CLICKED(IDC_RADIO_BOX51, OnRadioBox51)
	ON_BN_CLICKED(IDC_RADIO_BOX31, OnRadioBox31)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvInsuranceGRPEdit message handlers

BEGIN_EVENTSINK_MAP(CAdvInsuranceGRPEdit, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAdvInsuranceGRPEdit)
	ON_EVENT(CAdvInsuranceGRPEdit, IDC_UNSELECTED_INS_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedInsList, VTS_I4 VTS_I2)
	ON_EVENT(CAdvInsuranceGRPEdit, IDC_SELECTED_INS_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedInsList, VTS_I4 VTS_I2)
	ON_EVENT(CAdvInsuranceGRPEdit, IDC_UNSELECTED_PROVIDERS_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedProvidersList, VTS_I4 VTS_I2)
	ON_EVENT(CAdvInsuranceGRPEdit, IDC_SELECTED_PROVIDERS_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedProvidersList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CAdvInsuranceGRPEdit::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnUnselOneProv.AutoSet(NXB_LEFT);
	m_btnUnselOneInsco.AutoSet(NXB_LEFT);
	m_btnUnselAllProv.AutoSet(NXB_LLEFT);
	m_btnUnselAllInsco.AutoSet(NXB_LLEFT);
	m_btnSelAllProv.AutoSet(NXB_RRIGHT);
	m_btnSelAllInsco.AutoSet(NXB_RRIGHT);
	m_btnSelOneProv.AutoSet(NXB_RIGHT);
	m_btnSelOneInsco.AutoSet(NXB_RIGHT);
	// (c.haag 2008-04-30 13:14) - PLID 29847 - NxIconify additional buttons
	m_btnApply.AutoSet(NXB_MODIFY);
	m_btnOK.AutoSet(NXB_CLOSE);

	//TES 3/12/2007 - PLID 25295 - On the UB04, we'll call this the Other Prv ID, because it can be in multiple boxes.
	if(GetUBFormType() == eUB04) {//UB04
		SetDlgItemText(IDC_RADIO_BOX51, "UB Other Prv ID");
	}
	else {//UB92
		SetDlgItemText(IDC_RADIO_BOX51, "UB92 Box 51");
	}

	switch(m_IDType) {
	case 5:
		((CButton*)GetDlgItem(IDC_RADIO_BOX31))->SetCheck(TRUE);
		OnRadioBox31();
		break;
	case 4:
		((CButton*)GetDlgItem(IDC_RADIO_BOX51))->SetCheck(TRUE);
		OnRadioBox51();
		break;
	case 3:
		((CButton*)GetDlgItem(IDC_RADIO_NETWORKID))->SetCheck(TRUE);
		OnRadioNetworkid();
		break;
	case 2:
		((CButton*)GetDlgItem(IDC_RADIO_BOX24J))->SetCheck(TRUE);
		OnRadioBox24J();
		break;
	case 1:
	default:
		((CButton*)GetDlgItem(IDC_RADIO_GRP))->SetCheck(TRUE);
		OnRadioGrp();
		break;
		
	}

	m_SelectedInsCoList = BindNxDataListCtrl(this,IDC_SELECTED_INS_LIST,GetRemoteData(),false);
	m_UnselectedInsCoList = BindNxDataListCtrl(this,IDC_UNSELECTED_INS_LIST,GetRemoteData(),true);
	m_SelectedProviderList = BindNxDataListCtrl(this,IDC_SELECTED_PROVIDERS_LIST,GetRemoteData(),false);
	m_UnselectedProviderList = BindNxDataListCtrl(this,IDC_UNSELECTED_PROVIDERS_LIST,GetRemoteData(),true);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAdvInsuranceGRPEdit::OnRadioBox51() 
{
	if(((CButton*)GetDlgItem(IDC_RADIO_BOX51))->GetCheck()) {

		ShowBox24IInfo(FALSE);

		//TES 3/12/2007 - PLID 25295 - On the UB04, we'll call this the Other Prv ID, because it can be in multiple boxes.
		if(GetUBFormType() == eUB04) {
			SetDlgItemText(IDC_ID_LABEL,"New UB Other Prv ID");
		}
		else {
			SetDlgItemText(IDC_ID_LABEL,"New Box51 Number");
		}
		m_IDType = 4;
	}
}

void CAdvInsuranceGRPEdit::OnRadioBox24J() 
{
	if(((CButton*)GetDlgItem(IDC_RADIO_BOX24J))->GetCheck()) {

		ShowBox24IInfo(TRUE);

		SetDlgItemText(IDC_ID_LABEL,"New Box24J Number");
		m_IDType = 2;
	}
}

void CAdvInsuranceGRPEdit::OnRadioGrp() 
{
	if(((CButton*)GetDlgItem(IDC_RADIO_GRP))->GetCheck()) {

		ShowBox24IInfo(FALSE);

		SetDlgItemText(IDC_ID_LABEL,"New GRP Number");
		m_IDType = 1;
	}
}

void CAdvInsuranceGRPEdit::OnRadioNetworkid() 
{
	if(((CButton*)GetDlgItem(IDC_RADIO_NETWORKID))->GetCheck()) {

		ShowBox24IInfo(FALSE);

		SetDlgItemText(IDC_ID_LABEL,"New Network ID");
		m_IDType = 3;
	}
}

void CAdvInsuranceGRPEdit::OnDblClickCellUnselectedInsList(long nRowIndex, short nColIndex) 
{
	OnSelectOneInsco();
}

void CAdvInsuranceGRPEdit::OnDblClickCellSelectedInsList(long nRowIndex, short nColIndex) 
{
	OnUnselectOneInsco();	
}

void CAdvInsuranceGRPEdit::OnDblClickCellUnselectedProvidersList(long nRowIndex, short nColIndex) 
{
	OnSelectOneProv();
}

void CAdvInsuranceGRPEdit::OnDblClickCellSelectedProvidersList(long nRowIndex, short nColIndex) 
{
	OnUnselectOneProv();	
}

void CAdvInsuranceGRPEdit::OnSelectOneInsco() 
{
	m_SelectedInsCoList->TakeCurrentRow(m_UnselectedInsCoList);	
}

void CAdvInsuranceGRPEdit::OnSelectAllInsco() 
{
	m_SelectedInsCoList->TakeAllRows(m_UnselectedInsCoList);
}

void CAdvInsuranceGRPEdit::OnUnselectOneInsco() 
{
	m_UnselectedInsCoList->TakeCurrentRow(m_SelectedInsCoList);	
}

void CAdvInsuranceGRPEdit::OnUnselectAllInsco() 
{
	m_UnselectedInsCoList->TakeAllRows(m_SelectedInsCoList);	
}

void CAdvInsuranceGRPEdit::OnSelectOneProv() 
{
	m_SelectedProviderList->TakeCurrentRow(m_UnselectedProviderList);	
}

void CAdvInsuranceGRPEdit::OnSelectAllProv() 
{
	m_SelectedProviderList->TakeAllRows(m_UnselectedProviderList);
}

void CAdvInsuranceGRPEdit::OnUnselectOneProv() 
{
	m_UnselectedProviderList->TakeCurrentRow(m_SelectedProviderList);	
}

void CAdvInsuranceGRPEdit::OnUnselectAllProv() 
{
	m_UnselectedProviderList->TakeAllRows(m_SelectedProviderList);
}

void CAdvInsuranceGRPEdit::OnApply() 
{
	if(m_SelectedProviderList->GetRowCount()==0) {
		AfxMessageBox("You must have at least one provider selected.");
		return;
	}

	if(m_SelectedInsCoList->GetRowCount()==0) {
		AfxMessageBox("You must have at least one insurance company selected.");
		return;
	}

	CWaitCursor pWait;

	try {

		CString strNewID, strNewQual, strIDType,str;

		switch(m_IDType) {
		case 5:
			strIDType = "Box31";
			break;
		case 4:
			strIDType = "Box51";
			break;
		case 3:
			strIDType = "Network ID";
			break;
		case 2:
			strIDType = "Box24J";
			break;
		case 1:
		default:
			strIDType = "GRP";
			break;			
		}
		
		GetDlgItemText(IDC_NEW_ID_NUMBER,strNewID);
		GetDlgItemText(IDC_EDIT_BOX24I_QUAL,strNewQual);

		if(m_IDType != 2) {
			str.Format("This action will update the %s number to be '%s' for all selected provider and insurance company pairs.\nAre you sure you wish to do this?",strIDType,strNewID);
		}
		else {
			//Box 24J has a qualifier
			str.Format("This action will update the Box24J number to be '%s' and the Box24I qualifier to be '%s' for all selected provider and insurance company pairs.\nAre you sure you wish to do this?",strNewID,strNewQual);
		}

		if(IDNO==MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		for(int i=0;i<m_SelectedProviderList->GetRowCount();i++) {
			for(int j=0;j<m_SelectedInsCoList->GetRowCount();j++) {
				
				long ProvID, InsID;
				ProvID = m_SelectedProviderList->GetValue(i,0);
				InsID = m_SelectedInsCoList->GetValue(j,0);
				
				switch(m_IDType) {
				case 5:
					UpdateBox31(ProvID,InsID,strNewID);
					break;
				case 4:
					UpdateBox51(ProvID,InsID,strNewID);
					break;
				case 3:
					UpdateNetworkID(ProvID,InsID,strNewID);
					break;
				case 2:
					UpdateBox24J(ProvID,InsID,strNewID,strNewQual);
					break;
				case 1:
				default:
					UpdateGRP(ProvID,InsID,strNewID);
					break;			
				}
			}
		}

		AfxMessageBox("Update complete.");

	}NxCatchAll("Error applying new ID number.");
}

void CAdvInsuranceGRPEdit::UpdateBox31(long ProvID,long InsID,CString strNewID) {
	
	_RecordsetPtr rs;

	try {

		strNewID.TrimLeft();
		strNewID.TrimRight();

		if(strNewID != "") {

			rs = CreateRecordset("SELECT Count(InsCoID) AS Box31Count FROM InsuranceBox31 WHERE InsCoID = %li AND ProviderID = %li", InsID, ProvID);

			if (rs->Fields->GetItem("Box31Count")->Value.lVal > 0)
				ExecuteSql("UPDATE InsuranceBox31 SET Box31Info = '%s' WHERE InsCoID = %d AND ProviderID = %d",_Q(strNewID), InsID, ProvID);
			else
				ExecuteSql("INSERT INTO InsuranceBox31 (InsCoID, ProviderID, Box31Info) VALUES ( %d, %d, '%s' )",InsID, ProvID, _Q(strNewID));
		}
		else {
			//if empty, remove the record
			ExecuteSql("DELETE FROM InsuranceBox31 WHERE InsCoID = %li AND ProviderID = %li",InsID,ProvID);
		}

	} NxCatchAll("Error in UpdateBox31");
}

void CAdvInsuranceGRPEdit::UpdateBox51(long ProvID,long InsID,CString strNewID) {
	
	_RecordsetPtr rs;

	try {

		strNewID.TrimLeft();
		strNewID.TrimRight();

		if(strNewID != "") {

			rs = CreateRecordset("SELECT Count(InsCoID) AS Box51Count FROM InsuranceBox51 WHERE InsCoID = %li AND ProviderID = %li", InsID, ProvID);

			if (rs->Fields->GetItem("Box51Count")->Value.lVal > 0)
				ExecuteSql("UPDATE InsuranceBox51 SET Box51Info = '%s' WHERE InsCoID = %d AND ProviderID = %d",_Q(strNewID), InsID, ProvID);
			else
				ExecuteSql("INSERT INTO InsuranceBox51 (InsCoID, ProviderID, Box51Info) VALUES ( %d, %d, '%s' )",InsID, ProvID, _Q(strNewID));
		}
		else {
			//if empty, remove the record
			ExecuteSql("DELETE FROM InsuranceBox51 WHERE InsCoID = %li AND ProviderID = %li",InsID,ProvID);
		}

	} NxCatchAll("Error in UpdateBox51");
}

void CAdvInsuranceGRPEdit::UpdateBox24J(long ProvID,long InsID,CString strNewID,CString strNewQual) {
	
	_RecordsetPtr rs;

	try {

		strNewID.TrimLeft();
		strNewID.TrimRight();
		strNewQual.TrimLeft();
		strNewQual.TrimRight();

		if(strNewID != "" || strNewQual != "") {

			rs = CreateRecordset("SELECT Count(InsCoID) AS Box24JCount FROM InsuranceBox24J WHERE InsCoID = %li AND ProviderID = %li", InsID, ProvID);

			if (rs->Fields->GetItem("Box24JCount")->Value.lVal > 0)
				ExecuteSql("UPDATE InsuranceBox24J SET Box24IQualifier = '%s', Box24JNumber = '%s' WHERE InsCoID = %d AND ProviderID = %d", _Q(strNewQual), _Q(strNewID), InsID, ProvID);
			else
				ExecuteSql("INSERT INTO InsuranceBox24J (InsCoID, ProviderID, Box24IQualifier, Box24JNumber) VALUES ( %d, %d, '%s', '%s' )",InsID, ProvID, _Q(strNewQual), _Q(strNewID));
		}
		else {
			//if empty, remove the record
			ExecuteSql("DELETE FROM InsuranceBox24J WHERE InsCoID = %li AND ProviderID = %li",InsID,ProvID);
		}

	} NxCatchAll("Error in UpdateBox24J");
}

void CAdvInsuranceGRPEdit::UpdateGRP(long ProvID,long InsID,CString strNewID) {
	
	_RecordsetPtr rs;

	try {

		strNewID.TrimLeft();
		strNewID.TrimRight();

		if(strNewID != "") {

			rs = CreateRecordset("SELECT Count(InsCoID) AS GRPCount FROM InsuranceGroups WHERE InsCoID = %li AND ProviderID = %li", InsID, ProvID);

			if (rs->Fields->GetItem("GRPCount")->Value.lVal > 0)
				ExecuteSql("UPDATE InsuranceGroups SET GRP = '%s' WHERE InsCoID = %d AND ProviderID = %d",_Q(strNewID), InsID, ProvID);
			else
				ExecuteSql("INSERT INTO InsuranceGroups (InsCoID, ProviderID, GRP) VALUES ( %d, %d, '%s' )",InsID, ProvID, _Q(strNewID));
		}
		else {
			//if empty, remove the record
			ExecuteSql("DELETE FROM InsuranceGroups WHERE InsCoID = %li AND ProviderID = %li",InsID,ProvID);
		}

	} NxCatchAll("Error in UpdateGRP");
}

void CAdvInsuranceGRPEdit::UpdateNetworkID(long ProvID,long InsID,CString strNewID) {

		
	_RecordsetPtr rs;

	try {

		strNewID.TrimLeft();
		strNewID.TrimRight();

		if(strNewID != "") {

			rs = CreateRecordset("SELECT Count(InsCoID) AS NetworkIDCount FROM InsuranceNetworkID WHERE InsCoID = %li AND ProviderID = %li", InsID, ProvID);

			if (rs->Fields->GetItem("NetworkIDCount")->Value.lVal > 0)
				ExecuteSql("UPDATE InsuranceNetworkID SET NetworkID = '%s' WHERE InsCoID = %d AND ProviderID = %d",_Q(strNewID), InsID, ProvID);
			else
				ExecuteSql("INSERT INTO InsuranceNetworkID (InsCoID, ProviderID, NetworkID) VALUES ( %d, %d, '%s' )",InsID, ProvID, _Q(strNewID));
		}
		else {
			//if empty, remove the record
			ExecuteSql("DELETE FROM InsuranceNetworkID WHERE InsCoID = %li AND ProviderID = %li",InsID,ProvID);
		}

	} NxCatchAll("Error in UpdateNetworkID");
}

void CAdvInsuranceGRPEdit::OnRadioBox31() 
{
	if(((CButton*)GetDlgItem(IDC_RADIO_BOX31))->GetCheck()) {

		ShowBox24IInfo(FALSE);

		SetDlgItemText(IDC_ID_LABEL,"New Box31 Number");
		m_IDType = 5;
	}
}

void CAdvInsuranceGRPEdit::ShowBox24IInfo(BOOL bShow)
{
	GetDlgItem(IDC_BOX24I_QUAL_LABEL)->ShowWindow(bShow);
	GetDlgItem(IDC_EDIT_BOX24I_QUAL)->ShowWindow(bShow);	
}
