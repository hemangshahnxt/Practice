// InsuranceReferralsSelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InsuranceReferralsSelectDlg.h"
#include "NewInsuranceReferralDlg.h"
#include "BillingModuleDlg.h"
#include "globalutils.h"
#include "DateTimeUtils.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace ADODB;
using namespace NXDATALISTLib;

#define COLUMN_REFLIST_ID				0
#define COLUMN_REFLIST_INSURED_PARTY_ID	1
#define COLUMN_REFLIST_AUTH_NUM			2
#define COLUMN_REFLIST_START_DATE		3
#define COLUMN_REFLIST_END_DATE			4
#define COLUMN_REFLIST_NUM_VISITS		5
#define COLUMN_REFLIST_NUM_USED			6
#define COLUMN_REFLIST_DIAG_CODE		7
#define COLUMN_REFLIST_CPT_CODE			8
#define COLUMN_REFLIST_LOCATION_ID		9
#define COLUMN_REFLIST_LOCATION_NAME	10
#define COLUMN_REFLIST_PROVIDER			11
#define COLUMN_REFLIST_COMMENTS			12

#define COLUMN_INSCOLIST_INSPARTYID		0
#define COLUMN_INSCOLIST_INS_CO_ID		1
#define COLUMN_INSCOLIST_INS_CO_NAME	2
#define COLUMN_INSCOLIST_RESP_TYPE_ID	3
#define COLUMN_INSCOLIST_RESPONSIBILITY	4

#define ID_EDIT_REFERRAL	37892

/////////////////////////////////////////////////////////////////////////////
// CInsuranceReferralsSelectDlg dialog


CInsuranceReferralsSelectDlg::CInsuranceReferralsSelectDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInsuranceReferralsSelectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInsuranceReferralsSelectDlg)
	//}}AFX_DATA_INIT
	m_nPatientID = -1;
	m_bIsAppointmentMenu = false; // (b.spivey, March 22, 2012) - PLID 47435 - For any existing call if the function. 
}

// (b.spivey, March 22, 2012) - PLID 47435 - Overload for the constructor so the existing calls don't break. 
CInsuranceReferralsSelectDlg::CInsuranceReferralsSelectDlg(CWnd* pParent /*=NULL*/, bool bAppointmentMenu = false)
	: CNxDialog(CInsuranceReferralsSelectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInsuranceReferralsSelectDlg)
	//}}AFX_DATA_INIT
	m_nPatientID = -1;
	m_bIsAppointmentMenu = bAppointmentMenu; 
}

void CInsuranceReferralsSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInsuranceReferralsSelectDlg)
	DDX_Control(pDX, IDC_DATE, m_date);
	DDX_Control(pDX, IDC_USE_NEW_REFERRAL, m_btnUseNewReferral);
	DDX_Control(pDX, IDC_PROCEED_NEW_REFERRAL, m_btnProceedNewReferral);
	DDX_Control(pDX, IDC_PROCEED_SELECTED_REFERRAL, m_btnProceedSelectedReferral);
	DDX_Control(pDX, IDC_USE_SELECTED_REFERRAL, m_btnUseSelectedReferral);
	DDX_Control(pDX, IDC_PROCEED_NO_REFERRAL, m_btnProceedNoReferral);
	DDX_Control(pDX, IDC_CANCEL_BILL, m_btnCancelBill);
	DDX_Control(pDX, IDC_CANCEL_SELECTION, m_btnCancelSelection);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CInsuranceReferralsSelectDlg, IDC_DATE, 2 /* Change */, OnChangeDate, VTS_NONE)
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker

BEGIN_MESSAGE_MAP(CInsuranceReferralsSelectDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInsuranceReferralsSelectDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE, OnChangeDate)
	ON_BN_CLICKED(IDC_PROCEED_NEW_REFERRAL, OnOK_ProceedWithNewReferral)
	ON_BN_CLICKED(IDC_PROCEED_NO_REFERRAL, OnOK_ProceedNoReferral)
	ON_BN_CLICKED(IDC_PROCEED_SELECTED_REFERRAL, OnOK_ProceedWithSelectedReferral)
	ON_BN_CLICKED(IDC_CANCEL_SELECTION, OnCancel_CancelSelection)
	ON_BN_CLICKED(IDC_USE_NEW_REFERRAL, OnOK_UseNewReferral)
	ON_BN_CLICKED(IDC_USE_SELECTED_REFERRAL, OnOK_UseSelectedReferral)
	ON_BN_CLICKED(IDC_CANCEL_BILL, OnCancel_CancelBill)
	ON_COMMAND(ID_EDIT_REFERRAL, EditInsuranceReferral)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInsuranceReferralsSelectDlg message handlers


BOOL CInsuranceReferralsSelectDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (b.spivey, March 26, 2012) - PLID 47435 - Cache properties. I doubt we'll ever add more but consistency is important.
		g_propManager.CachePropertiesInBulk("InsuranceReferralsProps", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'WarnOnCloseInsAuthSched'"
			")",
			_Q(GetCurrentUserName()));

		// (c.haag 2008-04-30 13:40) - PLID 29847 - NxIconify the buttons
		m_btnUseNewReferral.AutoSet(NXB_OK);
		m_btnProceedNewReferral.AutoSet(NXB_OK);
		m_btnProceedSelectedReferral.AutoSet(NXB_OK);
		m_btnUseSelectedReferral.AutoSet(NXB_OK);
		m_btnProceedNoReferral.AutoSet(NXB_OK);
		m_btnCancelBill.AutoSet(NXB_CANCEL);
		m_btnCancelSelection.AutoSet(NXB_CANCEL);

		// this is initialize for now, but in the future if we added a checkbox that would allow the user
		// to disable the date filter, then we would just want to read this variable from the dialog
		m_bUseDateFilter = true;
		m_date.SetValue(COleVariant(m_dtFilterDate));
		m_ReferralList = BindNxDataListCtrl(this,IDC_INSURANCE_REFERRAL_LIST,GetRemoteData(),false);
		m_InsCoList = BindNxDataListCtrl(this,IDC_INS_CO_LIST,GetRemoteData(),false);

		m_brush.CreateSolidBrush(PaletteColor(0x0FFB9A8));

		//load the available insurance companies
		InitInsCoList(m_InsuredPartyID);

		//load the referrals and show the appropriate buttons
		Refresh();

		// (b.spivey, March 22, 2012) - PLID 47435 - If this is an appointment menu, change the window text and hide buttons. 
		if(m_bIsAppointmentMenu) { 
			this->SetWindowTextA("Create an Insurance Referral for this patient."); 
		}
	}
	NxCatchAll("Error in CInsuranceReferralsSelectDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CInsuranceReferralsSelectDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInsuranceReferralsSelectDlg)
	ON_EVENT(CInsuranceReferralsSelectDlg, IDC_INS_CO_LIST, 16 /* SelChosen */, OnSelChosenInsCoList, VTS_I4)
	ON_EVENT(CInsuranceReferralsSelectDlg, IDC_INSURANCE_REFERRAL_LIST, 18 /* RequeryFinished */, OnRequeryFinishedInsuranceReferralList, VTS_I2)
	ON_EVENT(CInsuranceReferralsSelectDlg, IDC_INSURANCE_REFERRAL_LIST, 2 /* SelChanged */, OnSelChangedInsuranceReferralList, VTS_I4)
	ON_EVENT(CInsuranceReferralsSelectDlg, IDC_INSURANCE_REFERRAL_LIST, 20 /* TrySetSelFinished */, OnTrySetSelFinishedReferralList, VTS_I4 VTS_I4)
	ON_EVENT(CInsuranceReferralsSelectDlg, IDC_INSURANCE_REFERRAL_LIST, 3 /* DblClickCell */, OnDblClickCellInsuranceReferralList, VTS_I4 VTS_I2)
	ON_EVENT(CInsuranceReferralsSelectDlg, IDC_INSURANCE_REFERRAL_LIST, 6 /* RButtonDown */, OnRButtonDownInsuranceReferralList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInsuranceReferralsSelectDlg::OnOK_ProceedWithNewReferral() 
{
	// MSC - 6/9/03 - as of now,  the implementation is the same as OnOK_UseNewReferral, but conceptually
	// they are different to the user
	CNewInsuranceReferralDlg dlg(this);
	long nCurSel = m_InsCoList->GetCurSel();
	if(nCurSel != sriNoRow){
		long nInsPartyID = VarLong(m_InsCoList->GetValue(nCurSel, COLUMN_INSCOLIST_INSPARTYID));
		dlg.m_InsuredPartyID = nInsPartyID;

		if(dlg.DoModal() == IDOK) {
			//select the referral
			m_InsuredPartyID = dlg.m_InsuredPartyID;
			m_nSelectedInsuranceReferralID = dlg.m_ID;
			m_strSelectedAuthNum = dlg.m_strAuthNum;
			m_dtSelectedStartDate = dlg.m_dtStart;
			m_dtSelectedEndDate = dlg.m_dtEnd;
			m_nSelectedLocationID = dlg.m_LocationID;
			m_bUseDateFilter = false;
			CNxDialog::OnOK();
		}
	}
	else{
		AfxMessageBox("There is no insurance company selected.  Please select an insurance company to add "
			"an insurance referral to");
	}
}

void CInsuranceReferralsSelectDlg::OnOK_UseNewReferral() 
{
	// MSC - 6/9/03 - as of now,  the implementation is the same as OnOK_ProceedWithNewReferral, but conceptually
	// they are different to the user
	CNewInsuranceReferralDlg dlg(this);
	long nCurSel = m_InsCoList->GetCurSel();
	if(nCurSel != sriNoRow){
		long nInsPartyID = VarLong(m_InsCoList->GetValue(nCurSel, COLUMN_INSCOLIST_INSPARTYID));
		dlg.m_InsuredPartyID = nInsPartyID;
	
		if(dlg.DoModal() == IDOK) {
			//select the referral
			m_InsuredPartyID = dlg.m_InsuredPartyID;
			m_nSelectedInsuranceReferralID = dlg.m_ID;
			m_strSelectedAuthNum = dlg.m_strAuthNum;
			m_dtSelectedStartDate = dlg.m_dtStart;
			m_dtSelectedEndDate = dlg.m_dtEnd;
			m_nSelectedLocationID = dlg.m_LocationID;
			m_bUseDateFilter = false;
			CNxDialog::OnOK();
		}
	}
	else{
		AfxMessageBox("There is no insurance company selected.  Please select an insurance company to add "
			"an insurance referral to");
	}
}

void CInsuranceReferralsSelectDlg::OnOK_ProceedWithSelectedReferral() 
{
	// MSC - 6/9/03 - as of now,  the implementation is the same as OnOK_UseSelectedReferral, but conceptually
	// they are different to the user

	try{	
		// (b.spivey, March 26, 2012) - PLID 47435 - If we're an appointment menu we just ignore this code.
		if (m_bIsAppointmentMenu) {
			OnOK_ProceedNoReferral();
			return;
		}

		long nCurSel = m_ReferralList->GetCurSel();
		if(nCurSel == -1){
			return;
		}
		// check to see if the filters are valid for the current selection
		if(ValidateSelectedReferralAgainstFilters(nCurSel)){
			//select the referral ID
			m_InsuredPartyID = VarLong(m_InsCoList->GetValue(m_InsCoList->GetCurSel(), COLUMN_INSCOLIST_INSPARTYID));
			m_nSelectedInsuranceReferralID = VarLong(m_ReferralList->GetValue(nCurSel,COLUMN_REFLIST_ID));
			m_strSelectedAuthNum = VarString(m_ReferralList->GetValue(nCurSel,COLUMN_REFLIST_AUTH_NUM));
			m_dtSelectedStartDate = VarDateTime(m_ReferralList->GetValue(nCurSel,COLUMN_REFLIST_START_DATE));
			m_dtSelectedEndDate = VarDateTime(m_ReferralList->GetValue(nCurSel,COLUMN_REFLIST_END_DATE)); 
			m_nSelectedLocationID = VarLong(m_ReferralList->GetValue(nCurSel,COLUMN_REFLIST_LOCATION_ID),-1);
			COleDateTime dtFilterDate = VarDateTime(m_date.GetValue());
			m_dtFilterDate.SetDate(dtFilterDate.GetYear(), dtFilterDate.GetMonth(), dtFilterDate.GetDay());
			m_bUseDateFilter = true;
			CNxDialog::OnOK();
		}
	}NxCatchAll("Error selecting referral.  No referral is selected");
}

void CInsuranceReferralsSelectDlg::OnOK_UseSelectedReferral() 
{
	// MSC - 6/9/03 - as of now,  the implementation is the same as OnOK_ProceedWithSelectedReferral, but conceptually
	// they are different to the user
	
	try{	
		// (b.spivey, March 26, 2012) - PLID 47435 - If we're an appointment menu we just ignore this code. This appears to be defunct code 
		//		but I cannot be sure as the button still exists in the resource file and I have seen it, better to handle this case 
		//		and not hit it than hit this case and not handle it.
		if (m_bIsAppointmentMenu) {
			OnOK_ProceedNoReferral();
			return;
		}

		long nCurSel = m_ReferralList->GetCurSel();
		if(nCurSel == -1){
			return;
		}
		
		// check to see if the filters are valid for the current selection
		if(ValidateSelectedReferralAgainstFilters(nCurSel)){
			//select the referral ID
			m_InsuredPartyID = m_InsCoList->GetValue(m_InsCoList->GetCurSel(), COLUMN_INSCOLIST_INSPARTYID);
			m_nSelectedInsuranceReferralID = VarLong(m_ReferralList->GetValue(nCurSel,COLUMN_REFLIST_ID));
			m_strSelectedAuthNum = VarString(m_ReferralList->GetValue(nCurSel,COLUMN_REFLIST_AUTH_NUM));
			m_dtSelectedStartDate = VarDateTime(m_ReferralList->GetValue(nCurSel,COLUMN_REFLIST_START_DATE));
			m_dtSelectedEndDate = VarDateTime(m_ReferralList->GetValue(nCurSel,COLUMN_REFLIST_END_DATE));
			m_nSelectedLocationID = VarLong(m_ReferralList->GetValue(nCurSel,COLUMN_REFLIST_LOCATION_ID),-1);
			COleDateTime dtFilterDate = VarDateTime(m_date.GetValue());
			m_dtFilterDate.SetDate(dtFilterDate.GetYear(), dtFilterDate.GetMonth(), dtFilterDate.GetDay());
			m_bUseDateFilter = true;
			CNxDialog::OnOK();
		}
	}NxCatchAll("Error selecting referral.  No referral is selected");
}

void CInsuranceReferralsSelectDlg::OnOK_ProceedNoReferral() 
{
	m_nSelectedInsuranceReferralID = -1;
	m_bUseDateFilter = false;
	CNxDialog::OnOK();
}

void CInsuranceReferralsSelectDlg::OnCancel_CancelBill() 
{
	try {
		// (b.spivey, March 26, 2012) - PLID 47435 - If we have the warning pref and it's an appointment menu, lets warn. 
		if(GetRemotePropertyInt("WarnOnCloseInsAuthSched", 0, 0, "<None>", true) == 1 && m_bIsAppointmentMenu) {
			if(AfxMessageBox("Canceling this will close the appointment. If this is a new appointment it will cancel the "
				"appointment altogether. \r\n\r\nAre you sure you want to close the appointment?", MB_YESNO) == IDNO) {	
					return;
			}
		}
	} NxCatchAll(__FUNCTION__); 

	CNxDialog::OnCancel();
}

void CInsuranceReferralsSelectDlg::OnCancel_CancelSelection() 
{
	CNxDialog::OnCancel();
}

void CInsuranceReferralsSelectDlg::OnSelChosenInsCoList(long nRow) 
{
	//if the user changes ins co's, we need to refresh the list
	Refresh();
}

void CInsuranceReferralsSelectDlg::Refresh()
{
	//this function refreshes the referral list as well as the buttons
	try{
		const long cnNoCurRefSelected = -1;

		long nCurInsID;
		{
			long nCurInsCoSel = m_InsCoList->GetCurSel();
			if(nCurInsCoSel == sriNoRow){
				m_ReferralList->Clear();
				ReflectCurrentStateOnBtns();
				return;
			}
			nCurInsID = VarLong(m_InsCoList->GetValue(nCurInsCoSel, COLUMN_INSCOLIST_INSPARTYID));
		}
		
		long nCurRefID;
		{
			long nCurRefSel = m_ReferralList->GetCurSel();
			if(nCurRefSel != sriNoRow){
				nCurRefID = VarLong(m_ReferralList->GetValue(nCurRefSel, COLUMN_REFLIST_ID));
			}
			else{
				nCurRefID = cnNoCurRefSelected;
			}
		}
		
		//show the list of available referrals - that is any referral who has visits left and is available for this insurance co
		CString str, strDate;
		strDate = FormatDateTimeForSql(VarDateTime(m_date.GetValue()), dtoDate);
		str.Format("InsuredPartyID = %li AND NumVisits > (CASE WHEN NumUsed Is NULL THEN 0 ELSE NumUsed END) AND StartDate <= '%s' AND EndDate >= '%s' ",nCurInsID, strDate, strDate);
		m_ReferralList->PutWhereClause(_bstr_t(str));
		m_ReferralList->Requery();

		if(nCurRefID != cnNoCurRefSelected ){
			m_ReferralList->TrySetSelByColumn(COLUMN_REFLIST_ID,nCurRefID);
		}

		//update the buttons
		ReflectCurrentStateOnBtns();

	}NxCatchAllCall("Error loading Referral List", ReflectCurrentStateOnBtns());

}

void CInsuranceReferralsSelectDlg::OnRequeryFinishedInsuranceReferralList(short nFlags) 
{
	//when requery finishes, we need to update the CPT and Diag fields - they can be multiple items,
	//not just 1 anymore
	try {
		for(int i = 0; i < m_ReferralList->GetRowCount(); i++) {
			CString strCPT = GetReferralCPTString(VarLong(m_ReferralList->GetValue(i, COLUMN_REFLIST_ID)));
			CString strDiag = GetReferralDiagString(VarLong(m_ReferralList->GetValue(i, COLUMN_REFLIST_ID)));
	
			m_ReferralList->PutValue(i, COLUMN_REFLIST_CPT_CODE, _bstr_t(strCPT));
			m_ReferralList->PutValue(i, COLUMN_REFLIST_DIAG_CODE, _bstr_t(strDiag));
		}

		if(m_ReferralList->GetRowCount() == 1) {
			m_ReferralList->CurSel = 0;
			ReflectCurrentStateOnBtns();
		}

	} NxCatchAll("Error in OnRequeryFinishedReferralList()");
	
}

CString CInsuranceReferralsSelectDlg::GetReferralCPTString(long nID)
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

CString CInsuranceReferralsSelectDlg::GetReferralDiagString(long nID)
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

void CInsuranceReferralsSelectDlg::InitInsCoList(IN const long nSetSelTo)
{
	try{
		//show the list of available insured parties
		CString str;
		// (a.walling 2008-07-07 17:34) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
		str.Format("PatientID = %li AND RespTypeID > 0", m_nPatientID);
		m_InsCoList->PutWhereClause(_bstr_t(str));
		m_InsCoList->Requery();
		if(nSetSelTo == dipNoDefaultGiven){
			m_InsCoList->PutCurSel(0);
		}
		else{
			m_InsCoList->SetSelByColumn(COLUMN_INSCOLIST_INSPARTYID, nSetSelTo);
		}
	}NxCatchAll("Error in CInsuranceReferralsSelectDlg::LoadInsCoList");
}

void CInsuranceReferralsSelectDlg::OnSelChangedInsuranceReferralList(long nNewSel) 
{
	// the user changed changed or selected a referral, update the buttons to make sure they are correct
	ReflectCurrentStateOnBtns();
}

void ShowEnableDlgButton(CWnd *pParent, IN const UINT nID, IN const BOOL bShow, IN const BOOL bEnable)
{
	CWnd *pBtn = pParent->GetDlgItem(nID);
	if (pBtn->GetSafeHwnd()) {
		if (bShow) {
			pBtn->EnableWindow(bEnable);
			pBtn->ShowWindow(SW_SHOW);
		} else {
			pBtn->ShowWindow(SW_HIDE);
			pBtn->EnableWindow(FALSE);
		}
	}
}

void CInsuranceReferralsSelectDlg::ReflectCurrentStateOnBtns()
{
	try {
		// Determine whether we're enabling or disabling the IDC_*_SELECTED_REFERRAL
		BOOL bSomethingIsSelected;
		if (m_ReferralList->GetCurSel() != sriNoRow) {
			bSomethingIsSelected = TRUE;
		} else {
			bSomethingIsSelected = FALSE;
		}

		// We have all the info we need, do the enabling/disabling and showing/hiding for all buttons
		{
			// In these three, we're SHOWing if m_bIsNewBill is true, and HIDEing if m_bIsNewBill is false
			ShowEnableDlgButton(this, IDC_PROCEED_NEW_REFERRAL, m_bIsNewBill, TRUE);
			ShowEnableDlgButton(this, IDC_PROCEED_SELECTED_REFERRAL, m_bIsNewBill, bSomethingIsSelected);
			//ShowEnableDlgButton(this, IDC_PROCEED_NO_REFERRAL, m_bIsNewBill, TRUE);
			ShowEnableDlgButton(this, IDC_CANCEL_BILL, m_bIsNewBill, TRUE);

			// In these three, we're SHOWing if m_bIsNewBill is false, and HIDEing if m_bIsNewBill is true
			ShowEnableDlgButton(this, IDC_USE_NEW_REFERRAL, !m_bIsNewBill, TRUE);
			ShowEnableDlgButton(this, IDC_USE_SELECTED_REFERRAL, !m_bIsNewBill, bSomethingIsSelected);
			ShowEnableDlgButton(this, IDC_CANCEL_SELECTION, !m_bIsNewBill, TRUE);
		}

	} NxCatchAllCall("CInsuranceReferralsSelectDlg::ReflectCurrentStateOnBtns", CNxDialog::OnCancel());
}

void CInsuranceReferralsSelectDlg::OnChangeDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// the filter date was changed, refresh the referral list
	Refresh();
	
	*pResult = 0;
}

void CInsuranceReferralsSelectDlg::OnTrySetSelFinishedReferralList(long nRowEnum, long nFlags)
{
	// the referral list is done loading, make sure the buttons are correct
	ReflectCurrentStateOnBtns();
}

bool CInsuranceReferralsSelectDlg::ValidateSelectedReferralAgainstFilters(IN const long &nCurSel)
{
	return (ValidateFilterDate(nCurSel) && ValidateInsuredPartyID(nCurSel) && ValidateLocationID(nCurSel));
}

// this function validates the current filter date against the current selection
bool CInsuranceReferralsSelectDlg::ValidateFilterDate(IN const long &nCurSel){
		
	//the filter date should only be a date
	ASSERT(VarDateTime(m_date.GetValue()).GetHour() == 0 && 
		VarDateTime(m_date.GetValue()).GetMinute() == 0 && 
		VarDateTime(m_date.GetValue()).GetSecond() == 0);
	
	
	// get whatever has focus now so we can set it back
	CWnd *pFocus = GetFocus();
	// get the filterdate before refreshing the date
	COleDateTime dtPreRefreshFilterDate = VarDateTime(m_date.GetValue());
	// setting the focus to the referral list will kill focus on m_date which will force it to refresh it's date if
	// the user has changed it
	GetDlgItem(IDC_INSURANCE_REFERRAL_LIST)->SetFocus();
	//get the filterdate after refreshing the date
	COleDateTime dtPostRefreshFilterDate = VarDateTime(m_date.GetValue());
	// set the focus back to what had it before, we have the values we needed to get
	if(pFocus){
		pFocus->SetFocus();
	}
	// if the dates are the same, then no change has been made, otherwise we don't know which date the user wanted 
	// to use, so force the user to reselect the referral
	if(CompareDates(dtPreRefreshFilterDate, dtPostRefreshFilterDate) == 0){
		COleDateTime dtStartDate = VarDateTime(m_ReferralList->GetValue(nCurSel,COLUMN_REFLIST_START_DATE)); 
		COleDateTime dtEndDate = VarDateTime(m_ReferralList->GetValue(nCurSel,COLUMN_REFLIST_END_DATE));
		COleDateTime dtFilterDate = VarDateTime(m_date.GetValue());
		// make sure the filter date is in the range of the selected referral
		if(CompareDates(dtFilterDate, dtStartDate) >= 0 && CompareDates(dtFilterDate, dtEndDate) <= 0){
			return true;
		}
		else{
			AfxMessageBox("There is a conflict between the filter date and date range of the selected referral.  Please "
				"reselect an insurance referral");
			return false;
		}
	}
	else{
		AfxMessageBox("The filter date has been changed.  Please reselect an insurance referral.");
		return false;
	}
}

bool CInsuranceReferralsSelectDlg::ValidateInsuredPartyID(IN const long &nCurSel)
{
	// this is the insured party associated with the selected insurance company for this patient
	long nInsPartyID = VarLong(m_InsCoList->GetValue(m_InsCoList->GetCurSel(),COLUMN_INSCOLIST_INSPARTYID));
	// this is the selected referral insured party
	long nSelRefInsPartyID= VarLong(m_ReferralList->GetValue(nCurSel, COLUMN_REFLIST_INSURED_PARTY_ID));
	if( nInsPartyID == nSelRefInsPartyID){
		return true;
	}
	else{
		return false;
	}
}

bool CInsuranceReferralsSelectDlg::ValidateLocationID(IN const long &nCurSel)
{
	long nLocationID = VarLong(m_ReferralList->GetValue(nCurSel,COLUMN_REFLIST_LOCATION_ID),-1);
	if(nLocationID != -1 && IsRecordsetEmpty("SELECT ID FROM LocationsT WHERE ID = %li AND Active = 1",nLocationID)){
		//it is not an active location, so warn the user
		if(IDNO == MessageBox("The Location selected on this Referral has since been made Inactive.\n"
			"If you continue, the Location on this Referral will be ignored.\n\n"
			"Are you sure you wish to use this Referral?","Practice",MB_YESNO|MB_ICONEXCLAMATION)) {		
			return false;
		}
	}
	return true;
}

void CInsuranceReferralsSelectDlg::EditInsuranceReferral() {

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
			m_ReferralList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		}

		m_ReferralList->PutCurSel(CurSel);
		ReflectCurrentStateOnBtns();
	
	}NxCatchAll("Error editing insurance referral.");
}

void CInsuranceReferralsSelectDlg::OnDblClickCellInsuranceReferralList(long nRowIndex, short nColIndex) 
{
	// (b.spivey, March 26, 2012) - PLID 47435 - Added try catch and changed the behavior here. 
	try {
		if(m_bIsAppointmentMenu) {
			//Proceed like no referral. 
			OnOK_ProceedNoReferral(); 
		}
		else {
			// use the referral the user double clicked on
			OnOK_ProceedWithSelectedReferral();	
		}
	}NxCatchAll(__FUNCTION__)
}

void CInsuranceReferralsSelectDlg::OnCancel()
{
	if(m_bIsNewBill){
		if(GetDlgItem(IDC_CANCEL_BILL)->IsWindowEnabled()){
			OnCancel_CancelBill();
		}
		else
		{
			HandleException(NULL, "CInsuranceReferralsSelectDlg::OnCancel CANCEL_BILL is not enabled", __LINE__, __FILE__);
		}
	}
	else{
		if(GetDlgItem(IDC_CANCEL_SELECTION)->IsWindowEnabled()){
			OnCancel_CancelSelection();
		}
		else{
			HandleException(NULL, "CInsuranceReferralsSelectDlg::OnCancel CANCEL_SELECTION is not enabled", __LINE__, __FILE__);
		}
	}
}

void CInsuranceReferralsSelectDlg::OnOK()
{
	// there is no IDOK and there is always a default button so this should never be called	
	HandleException(NULL, "CInsuranceReferralsSelectDlg::OnOK() is not valid", __LINE__, __FILE__);
}

void CInsuranceReferralsSelectDlg::OnRButtonDownInsuranceReferralList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if(nRow == -1)
		return;

	m_ReferralList->CurSel = nRow;
	ReflectCurrentStateOnBtns();

	CMenu tmpMenu;
	tmpMenu.CreatePopupMenu();
	tmpMenu.InsertMenu(0, MF_BYPOSITION, ID_EDIT_REFERRAL, "Edit Referral");

	CPoint pt;
	GetCursorPos(&pt);
	tmpMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
}