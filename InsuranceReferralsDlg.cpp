// InsuranceReferralsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InsuranceReferralsDlg.h"
#include "NewInsuranceReferralDlg.h"
#include "DateTimeUtils.h"
#include "GlobalDrawingUtils.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLUMN_ID				0
#define COLUMN_INSURED_PARTY_ID	1
#define COLUMN_AUTH_NUM			2
#define COLUMN_START_DATE		3
#define COLUMN_END_DATE			4
#define COLUMN_NUM_VISITS		5
#define COLUMN_NUM_USED			6
#define COLUMN_DIAG_CODE		7
#define COLUMN_CPT_CODE			8
#define COLUMN_LOCATION			9
#define COLUMN_PROVIDER			10
#define COLUMN_COMMENTS			11

#define ID_EDIT_REFERRAL	37893

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CInsuranceReferralsDlg dialog


CInsuranceReferralsDlg::CInsuranceReferralsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInsuranceReferralsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInsuranceReferralsDlg)
		m_InsuredPartyID = -1;
		m_nSelectedInsuranceReferralID = -1;
		m_strSelectedAuthNum = "";
		m_iEntryType = ENTRY_ONLY;
	//}}AFX_DATA_INIT
}


void CInsuranceReferralsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInsuranceReferralsDlg)
	DDX_Control(pDX, IDC_RADIO_SHOW_AVAILABLE_REFERRALS, m_btnShowAvail);
	DDX_Control(pDX, IDC_RADIO_SHOW_UNAVAILABLE_REFERRALS, m_btnShowUnavail);
	DDX_Control(pDX, IDC_RADIO_SHOW_ALL_REFERRALS, m_btnShowAll);
	DDX_Control(pDX, IDC_ADD_NEW_REFERRAL, m_btnAddNewReferral);
	DDX_Control(pDX, IDC_DELETE_REFERRAL, m_btnDeleteReferral);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInsuranceReferralsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInsuranceReferralsDlg)
	ON_BN_CLICKED(IDC_ADD_NEW_REFERRAL, OnAddNewReferral)
	ON_BN_CLICKED(IDC_DELETE_REFERRAL, OnDeleteReferral)
	ON_BN_CLICKED(IDC_RADIO_SHOW_ALL_REFERRALS, OnRadioShowAllReferrals)
	ON_BN_CLICKED(IDC_RADIO_SHOW_AVAILABLE_REFERRALS, OnRadioShowAvailableReferrals)
	ON_BN_CLICKED(IDC_RADIO_SHOW_UNAVAILABLE_REFERRALS, OnRadioShowUnavailableReferrals)
	ON_COMMAND(ID_EDIT_REFERRAL, EditInsuranceReferral)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInsuranceReferralsDlg message handlers

BOOL CInsuranceReferralsDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-30 13:31) - PLID 29847 - NxIconify the buttons
		m_btnAddNewReferral.AutoSet(NXB_NEW);
		m_btnDeleteReferral.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_CLOSE);

		if(m_iEntryType == SELECT_AUTH)
			SetWindowText("Select An Insurance Referral");
		
		m_ReferralList = BindNxDataListCtrl(this,IDC_INSURANCE_REFERRAL_LIST,GetRemoteData(),false);

		((CButton*)GetDlgItem(IDC_RADIO_SHOW_AVAILABLE_REFERRALS))->SetCheck(TRUE);
		OnRadioShowAvailableReferrals();
		RefreshButtons();
	}
	NxCatchAll("Error in CInsuranceReferralsDlg::OnInitDialog");
			
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInsuranceReferralsDlg::OnAddNewReferral() 
{
	CNewInsuranceReferralDlg dlg(this);
	dlg.m_InsuredPartyID = m_InsuredPartyID;

	if(dlg.DoModal() == IDOK) {
		m_ReferralList->Requery();
	}
}

void CInsuranceReferralsDlg::OnDeleteReferral() 
{

	try {

		long nCurSel = m_ReferralList->CurSel;
		if(nCurSel == -1) {
			MsgBox("Please select a referral to delete.");
			return;
		}

		//1)  If it's in use, don't let them delete
		if(VarLong(m_ReferralList->GetValue(nCurSel, COLUMN_NUM_USED)) > 0) {
			MsgBox("You cannot delete a referral that has been used.");
			return;
		}

		//2)  Confirm
		if(MsgBox(MB_YESNO, "Are you sure you wish to delete this referral?") == IDNO)
			return;

		//3)  Actually delete it
		long nID = VarLong(m_ReferralList->GetValue(nCurSel, COLUMN_ID));
		ExecuteSql("UPDATE BillsT SET InsuranceReferralID = NULL WHERE InsuranceReferralID = %li; "
			"DELETE FROM InsuranceReferralDiagsT WHERE ReferralID = %li; "
			"DELETE FROM InsuranceReferralCPTCodesT WHERE ReferralID = %li; "
			"DELETE FROM InsuranceReferralsT WHERE ID = %li; ", nID, nID, nID, nID);

		m_ReferralList->Requery();

	} NxCatchAll("Error deleting insurance referral.");

}

void CInsuranceReferralsDlg::OnRadioShowAllReferrals() 
{
	CString str;
	str.Format("InsuredPartyID = %li",m_InsuredPartyID);
	m_ReferralList->PutWhereClause(_bstr_t(str));
	m_ReferralList->Requery();
}

void CInsuranceReferralsDlg::OnRadioShowAvailableReferrals() 
{
	CString str;
	CString strDate = FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate);
	//TODO: the date filter needs to be based on the bill date, not necessarily today's date
	str.Format("InsuredPartyID = %li AND NumVisits > (CASE WHEN NumUsed Is NULL THEN 0 ELSE NumUsed END) AND StartDate <= '%s' AND EndDate >= '%s' ",m_InsuredPartyID,strDate,strDate);
	m_ReferralList->PutWhereClause(_bstr_t(str));
	m_ReferralList->Requery();
}

void CInsuranceReferralsDlg::OnRadioShowUnavailableReferrals() 
{
	CString str;
	CString strDate = FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate);
	//TODO: the date filter needs to be based on the bill date, not necessarily today's date
	str.Format("InsuredPartyID = %li AND (NumVisits <= (CASE WHEN NumUsed Is NULL THEN 0 ELSE NumUsed END) OR StartDate > '%s' OR EndDate < '%s')",m_InsuredPartyID,strDate,strDate);
	m_ReferralList->PutWhereClause(_bstr_t(str));
	m_ReferralList->Requery();
}

BEGIN_EVENTSINK_MAP(CInsuranceReferralsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInsuranceReferralsDlg)
	ON_EVENT(CInsuranceReferralsDlg, IDC_INSURANCE_REFERRAL_LIST, 3 /* DblClickCell */, OnDblClickCellInsuranceReferralList, VTS_I4 VTS_I2)
	ON_EVENT(CInsuranceReferralsDlg, IDC_INSURANCE_REFERRAL_LIST, 18 /* RequeryFinished */, OnRequeryFinishedReferralList, VTS_I2)
	ON_EVENT(CInsuranceReferralsDlg, IDC_INSURANCE_REFERRAL_LIST, 6 /* RButtonDown */, OnRButtonDownInsuranceReferralList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CInsuranceReferralsDlg, IDC_INSURANCE_REFERRAL_LIST, 2 /* SelChanged */, OnSelChangedInsuranceReferralList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInsuranceReferralsDlg::EditInsuranceReferral() {

	try {

		long CurSel = m_ReferralList->GetCurSel();
		if(CurSel == -1)
			return;
	
		CNewInsuranceReferralDlg dlg(this);
		dlg.m_InsuredPartyID = m_InsuredPartyID;
		dlg.m_ID = m_ReferralList->GetValue(CurSel,0).lVal;
		dlg.m_bEditing = true;
		if(dlg.DoModal() == IDOK) {
			m_ReferralList->Requery();
		}
		RefreshButtons();
	
	}NxCatchAll("Error editing insurance referral.");
}

void CInsuranceReferralsDlg::OnDblClickCellInsuranceReferralList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1)
		return;

	EditInsuranceReferral();
}

void CInsuranceReferralsDlg::OnRequeryFinishedReferralList(short nFlags) 
{
	//when requery finishes, we need to update the CPT and Diag fields - they can be multiple items,
	//not just 1 anymore
	try {
		for(int i = 0; i < m_ReferralList->GetRowCount(); i++) {
			CString strCPT = GetReferralCPTString(VarLong(m_ReferralList->GetValue(i, COLUMN_ID)));
			CString strDiag = GetReferralDiagString(VarLong(m_ReferralList->GetValue(i, COLUMN_ID)));

			m_ReferralList->PutValue(i, COLUMN_CPT_CODE, _bstr_t(strCPT));
			m_ReferralList->PutValue(i, COLUMN_DIAG_CODE, _bstr_t(strDiag));
		}
		RefreshButtons();
	} NxCatchAll("Error in OnRequeryFinishedReferralList()");
}

CString CInsuranceReferralsDlg::GetReferralCPTString(long nID)
{
	try {
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT Code from InsuranceReferralCPTCodesT INNER JOIN CPTCodeT ON ServiceID = CPTCodeT.ID WHERE ReferralID = {INT}", nID);
		CString strList = "";
		while(!rs->eof) {
			strList += AdoFldString(rs, "Code", "");
			strList += ", ";
			rs->MoveNext();
		}

		strList.TrimRight(", ");
		return strList;
	} NxCatchAll("Error in GetReferralCPTString()");

	return "";
}

CString CInsuranceReferralsDlg::GetReferralDiagString(long nID)
{
	try {
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT CodeNumber AS DiagCode from InsuranceReferralDiagsT INNER JOIN DiagCodes ON DiagID = DiagCodes.ID "
			"WHERE ReferralID = {INT}", nID);
		CString strList = "";
		while(!rs->eof) {
			strList += AdoFldString(rs, "DiagCode", "");
			strList += ", ";
			rs->MoveNext();
		}

		strList.TrimRight(", ");
		return strList;
	} NxCatchAll("Error in GetReferralDiagString()");

	return "";
}

void CInsuranceReferralsDlg::OnOK() 
{
	long nCurSel = m_ReferralList->GetCurSel();

	if(m_iEntryType != SELECT_AUTH) {
		//they are in entry-only mode.  It doesn't matter what they have selected, we're not
		//looking at it.
		CDialog::OnOK();
		return;
	}

	if(nCurSel == -1) {
		MsgBox("Please select a referral");
		return;
	}

	//TODO: the date filter needs to be based on the bill date, not necessarily today's date

	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	dtNow.SetDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 0, 0, 0);

	//the referral is only valid if there are visits remaining and we are currently in the given date range
	if(m_ReferralList->GetValue(nCurSel,COLUMN_NUM_VISITS).lVal > m_ReferralList->GetValue(nCurSel,COLUMN_NUM_USED).lVal
			&& m_ReferralList->GetValue(nCurSel,COLUMN_START_DATE).date <= dtNow
			&& m_ReferralList->GetValue(nCurSel,COLUMN_END_DATE).date >= dtNow) {

		//select the referral ID and close
		m_nSelectedInsuranceReferralID = m_ReferralList->GetValue(nCurSel,COLUMN_ID).lVal;
		m_strSelectedAuthNum = CString(m_ReferralList->GetValue(nCurSel,COLUMN_AUTH_NUM).bstrVal);
	}
	else {
		MsgBox("You must choose an available referral to continue.");
		return;
	}

	CDialog::OnOK();
}

void CInsuranceReferralsDlg::OnRButtonDownInsuranceReferralList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if(nRow == -1)
		return;

	m_ReferralList->CurSel = nRow;
	OnSelChangedInsuranceReferralList(nRow);

	CMenu tmpMenu;
	tmpMenu.CreatePopupMenu();
	tmpMenu.InsertMenu(0, MF_BYPOSITION, ID_EDIT_REFERRAL, "Edit Referral");
	tmpMenu.InsertMenu(1, MF_BYPOSITION, IDC_DELETE_REFERRAL, "Delete Referral");

	CPoint pt;
	GetCursorPos(&pt);
	tmpMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
}

void CInsuranceReferralsDlg::OnSelChangedInsuranceReferralList(long nNewSel) 
{
	RefreshButtons();	
}

void CInsuranceReferralsDlg::RefreshButtons()
{
	if(m_ReferralList->GetCurSel() == sriNoRow){
		GetDlgItem(IDC_DELETE_REFERRAL)->EnableWindow(FALSE);
	}
	else{
		GetDlgItem(IDC_DELETE_REFERRAL)->EnableWindow(TRUE);
	}
}