// NewInsuranceReferralDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NewInsuranceReferralDlg.h"
#include "GlobalDrawingUtils.h"
#include "MultiSelectDlg.h"
#include "DateTimeUtils.h"
#include "DiagSearchUtils.h" // (a.levy 2014-02-20 11:15) - PLID - 60768  - loadsearchbox and preferences
#include "PatientsRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CNewInsuranceReferralDlg dialog


CNewInsuranceReferralDlg::CNewInsuranceReferralDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNewInsuranceReferralDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewInsuranceReferralDlg)
		m_InsuredPartyID = -1;
		m_LocationID = -1;
		m_ID = -1;
		m_bIsCPTListHidden = FALSE;
		m_bIsDiagListHidden = FALSE;
	//}}AFX_DATA_INIT

	m_bEditing = false;
	m_bAllowChange = true;

	m_rcMultiDiagLabel.top = m_rcMultiDiagLabel.bottom = m_rcMultiDiagLabel.left = m_rcMultiDiagLabel.right = 0;
	m_rcMultiCPTLabel.top = m_rcMultiCPTLabel.bottom = m_rcMultiCPTLabel.left = m_rcMultiCPTLabel.right = 0;
}


void CNewInsuranceReferralDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewInsuranceReferralDlg)
	DDX_Control(pDX, IDC_REFERRAL_START_DATE, m_dtStartDate);
	DDX_Control(pDX, IDC_REFERRAL_END_DATE, m_dtEndDate);
	DDX_Control(pDX, IDC_AUTH_NUM, m_nxeditAuthNum);
	DDX_Control(pDX, IDC_NUM_VISITS, m_nxeditNumVisits);
	DDX_Control(pDX, IDC_COMMENTS, m_nxeditComments);
	DDX_Control(pDX, IDC_MULTI_CPT_LABEL, m_nxstaticMultiCptLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}



BEGIN_MESSAGE_MAP(CNewInsuranceReferralDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNewInsuranceReferralDlg)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// (a.levy 2014-02-20 10:36) - PLID 60768 - Relocated and Cleaned up EventSinkMap
BEGIN_EVENTSINK_MAP(CNewInsuranceReferralDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNewInsuranceReferralDlg)
	ON_EVENT(CNewInsuranceReferralDlg, IDC_CPTCODE_COMBO, 16 /* SelChosen */, OnSelChosenCptcodeCombo, VTS_I4)
    ON_EVENT(CNewInsuranceReferralDlg, IDC_DEFAULT_DIAG_INSREF_SEARCH_LIST, 16 /*SelInsurance */ , OnSelectInsuranceRefDiagnosisSearch, VTS_DISPATCH)
    ON_EVENT(CNewInsuranceReferralDlg, IDC_INSREF_DIAG_CODELIST,6 ,/*OnRightButtonDown */ OnRightClickDiagCodeList,VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4) // (a.levy 2014-03-05 12:22) - PLID 60868 - Handle Right Click and delete code
	ON_EVENT(CNewInsuranceReferralDlg, IDC_CPTCODE_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedCptcodeCombo, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNewInsuranceReferralDlg message handlers


// (a.levy 2014-02-20 11:15) - PLID 60768 - For result list
enum DiagnosisCodeListColumns {
	dclcDiagID = 0,
    dclcDiagCode,
    dclcDiagCodeDescription

};



BOOL CNewInsuranceReferralDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-30 15:42) - PLID 29847 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_CPTCodeCombo = BindNxDataListCtrl(this,IDC_CPTCODE_COMBO,GetRemoteData(),true);
		m_ProviderCombo = BindNxDataListCtrl(this,IDC_PROVIDER_REF_COMBO,GetRemoteData(),true);
		m_LocationCombo = BindNxDataListCtrl(this,IDC_LOCATION_REF_COMBO,GetRemoteData(),true);
        // (a.levy 2014-02-20 10:36) - PLID - 60768 - Added Search Control for Diag insurance referrals and List of Diagnosis codes
        m_diagInsSearch = DiagSearchUtils::BindDiagDualSearchListCtrl(this, IDC_DEFAULT_DIAG_INSREF_SEARCH_LIST, GetRemoteData());
        m_InsDiagList = BindNxDataList2Ctrl(IDC_INSREF_DIAG_CODELIST,false);
  
   

		IRowSettingsPtr pRow = m_CPTCodeCombo->GetRow(-1);
		pRow = m_CPTCodeCombo->GetRow(-1);
		pRow->PutValue(0,(long)-2);
		pRow->PutValue(1," {Multiple}");
		pRow->PutValue(2," {Multiple Service Codes}");	
		m_CPTCodeCombo->InsertRow(pRow, 0);

		pRow = m_CPTCodeCombo->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1," {None}");
		pRow->PutValue(2," {No Service Code}");
		m_CPTCodeCombo->InsertRow(pRow, 0);

		pRow = m_ProviderCombo->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1," {No Provider}");
		m_ProviderCombo->InsertRow(pRow, 0);

		pRow = m_LocationCombo->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1," {No Location}");
		m_LocationCombo->InsertRow(pRow, 0);


		m_dtStartDate.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
		m_dtEndDate.SetValue(_variant_t(COleDateTime::GetCurrentTime()));

		// Calculate hyperlink rectangles
		{
			CWnd *pWnd;
			pWnd = GetDlgItem(IDC_MULTI_CPT_LABEL);
			if (pWnd->GetSafeHwnd()) {
				// Get the position of the is hotlinks
				pWnd->GetWindowRect(m_rcMultiCPTLabel);
				ScreenToClient(&m_rcMultiCPTLabel);

				// Hide the static text that was there
				pWnd->ShowWindow(SW_HIDE);
			}
		}

		if(m_ID != -1) {
			SetWindowText("Edit Insurance Referral");
			Load();
		}

		if(m_bEditing) {

			try {
				//if we're editing an existing, run a check and see if it's in use already.  If
				//so, warn them they aren't going to be allowed to change anything
				// (j.jones 2011-09-29 15:33) - PLID 44980 - ignore if in use on a voided bill
				// I also changed this to match the query used later in this file that actually
				// reports the count, so that SQL can re-use the same query cache.
				_RecordsetPtr rs = CreateParamRecordset("SELECT Count(BillsT.ID) AS CountOfBills "
					"FROM InsuranceReferralsT "
					"INNER JOIN BillsT ON InsuranceReferralsT.ID = BillsT.InsuranceReferralID "
					"WHERE InsuranceReferralsT.ID = {INT} AND BillsT.Deleted = 0 "
					"AND BillsT.ID NOT IN (SELECT OriginalBillID FROM BillCorrectionsT)", m_ID);
				if(!rs->eof) {
					long nCount = AdoFldLong(rs, "CountOfBills",0);
					if(nCount > 0) {
						//it is indeed in use.  warn them.
						MsgBox("You are editing a referral that is already in use.  You will not be allowed to make any changes\n"
							   "to this referral except for the End Date and Number Of Visits.");
						m_bAllowChange = false;
						EnableControls(false);
					}
				}
			} NxCatchAll("Error in OnInitDialog()");
		}	
	}
	NxCatchAll("Error in CNewInsuranceReferralDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNewInsuranceReferralDlg::Load()
{
	try {

		_RecordsetPtr rs = CreateRecordset("SELECT AuthNum, StartDate, EndDate, NumVisits, LocationID, ProviderID, Comments "
			"FROM InsuranceReferralsT WHERE ID = %li",m_ID);

		if(!rs->eof) {
			
			//authorization number
			CString strAuthNum = AdoFldString(rs, "AuthNum","");
			SetDlgItemText(IDC_AUTH_NUM,strAuthNum);

			//start date
			m_dtStartDate.SetValue(rs->Fields->Item["StartDate"]->Value);

			//end date
			m_dtEndDate.SetValue(rs->Fields->Item["EndDate"]->Value);

			//number of visits
			long nNumVisits = AdoFldLong(rs, "NumVisits");
			SetDlgItemInt(IDC_NUM_VISITS,nNumVisits);

			//location
			long nLocationID = AdoFldLong(rs, "LocationID",-1);
			if(nLocationID != -1) {
				//handle inactive locations
				if(IsRecordsetEmpty("SELECT ID FROM LocationsT WHERE ID = %li AND Active = 1",nLocationID)) {
					AfxMessageBox("The Location previously selected on this referral is currently Inactive.\n"
						"If you save this referral without choosing a new Location, it will save with no Location selected.");
				}
				else {
					m_LocationCombo->SetSelByColumn(0,nLocationID);
				}
			}

			//provider
			if(m_ProviderCombo->SetSelByColumn(0,rs->Fields->Item["ProviderID"]->Value) == -1) {
				//they may have an inactive provider
				_RecordsetPtr rsProv = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = (SELECT ProviderID FROM InsuranceReferralsT WHERE ID = %li)", m_ID);
				if(!rsProv->eof) {
					m_ProviderCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
				}
				else 
					m_ProviderCombo->PutCurSel(-1);
			}


			//comments
			CString strComments = AdoFldString(rs, "Comments","");
			SetDlgItemText(IDC_COMMENTS,strComments);

			//diagnosis codes
			//load the diag string
			m_strDiagIDs.Empty();
			// (a.levy 2014-02-20 10:36) - PLID - 60768 - Load DiagCodes - Cleaned up Legacy Code
            _RecordsetPtr rsDiag = CreateParamRecordset("SELECT ID AS DiagID,CodeNumber, CodeDesc FROM DiagCodes "
                                                        "INNER JOIN "
                                                        "InsuranceReferralDiagsT ON DiagCodes.ID = InsuranceReferralDiagsT.DiagID "
                                                        "WHERE ReferralID = {INT} ", m_ID);
                                                    
			while(!rsDiag->eof) {
				CString strDiagCode, strDiagDescription;
				long nDiagID = AdoFldLong(rsDiag, "DiagID");
                strDiagCode.Format("%s", AdoFldString(rsDiag, "CodeNumber"));
                strDiagDescription.Format("%s", AdoFldString(rsDiag,"CodeDesc"));
                UpdateDiagList(nDiagID,strDiagCode,strDiagDescription);
               
				rsDiag->MoveNext();
			}

			rsDiag->Close();
            m_strDiagIDs.TrimRight(", ");
             

			//CPT Codes
			//load the CPT string
			m_strCPTIDs.Empty();
			_RecordsetPtr rsCPT = CreateRecordset("SELECT ServiceID FROM InsuranceReferralCPTCodesT WHERE ReferralID = %li",m_ID);
			while(!rsCPT->eof) {
				CString str;
				str.Format("%li ",AdoFldLong(rsCPT, "ServiceID"));
				m_strCPTIDs += str;
				rsCPT->MoveNext();
			}
			rsCPT->Close();
			m_strCPTIDs.TrimRight();
			DisplayCPTInfo();
			
		}
		rs->Close();

	}NxCatchAll("Error loading referral.");	
}

void CNewInsuranceReferralDlg::OnOK() 
{
	try {

		CString strAuthNum, strComments, strDiagCode;
		COleDateTime dtStart, dtEnd;
		long nNumVisits;
		CString strProvID = "NULL", strLocID = "NULL";

		GetDlgItemText(IDC_AUTH_NUM,strAuthNum);
		if(strAuthNum.IsEmpty()) {
			MsgBox("You have entered an empty authorization number.  Please enter a valid authorization num before saving.");
			return;
		}
		else{
			m_strAuthNum = strAuthNum;
		}

		dtStart = COleDateTime(m_dtStartDate.GetValue());
		dtEnd = COleDateTime(m_dtEndDate.GetValue());
		m_dtStart = dtStart;
		m_dtEnd = dtEnd;
		nNumVisits = GetDlgItemInt(IDC_NUM_VISITS);
		if(nNumVisits < 1) {
			MsgBox("You have entered an invalid number of visits.  Please enter a number > 0 before saving.");
			return;
		}
		
		// (j.jones 2011-09-29 15:33) - PLID 44980 - ignore if in use on a voided bill
		_RecordsetPtr rs = CreateParamRecordset("SELECT Count(BillsT.ID) AS CountOfBills "
			"FROM InsuranceReferralsT "
			"INNER JOIN BillsT ON InsuranceReferralsT.ID = BillsT.InsuranceReferralID "
			"WHERE InsuranceReferralsT.ID = {INT} AND BillsT.Deleted = 0 "
			"AND BillsT.ID NOT IN (SELECT OriginalBillID FROM BillCorrectionsT)", m_ID);
		if(!rs->eof) {
			long nCount = AdoFldLong(rs, "CountOfBills",0);
			if(nNumVisits < nCount) {
				CString str;
				str.Format("You have entered %li for the number of visits, but this referral has already been used %li times.\n\n"
					"Please change the number of visits to be no less than %li.",nNumVisits,nCount,nCount);
				MsgBox(str);
				return;
			}
		}
		rs->Close();

		COleDateTime dtTemp;
		dtTemp.ParseDateTime("01/01/1753");

		if(dtStart.GetStatus() == COleDateTime::invalid || dtStart < dtTemp) {
			MsgBox("You have entered an out of range start date value.  Please choose a value after 1753.");
			return;
		}

		if(dtEnd.GetStatus() == COleDateTime::invalid || dtEnd < dtTemp) {
			MsgBox("You have entered an out of range end date value.  Please choose a value after 1753.");
			return;
		}
	
		if (dtStart > dtEnd)
		{
			MsgBox("You have entered a start date that occurs after the end date.  Please correct this inconsistency.");
			return;
		}

		long CurSel;

		if(m_strCPTIDs.IsEmpty()) {
			//no items in the multi list, see if 1 is selected
			CurSel = m_CPTCodeCombo->GetCurSel();
			if(CurSel != -1 && m_CPTCodeCombo->GetValue(CurSel,0).lVal != -1)
				m_strCPTIDs.Format("%li", m_CPTCodeCombo->GetValue(CurSel,0).lVal);
		}

		CurSel = m_ProviderCombo->GetCurSel();
		if(CurSel != -1 && m_ProviderCombo->GetValue(CurSel,0).lVal != -1)
			strProvID.Format("%li", m_ProviderCombo->GetValue(CurSel,0).lVal);

		CurSel = m_LocationCombo->GetCurSel();
		if(CurSel != -1) {
			if(m_LocationCombo->GetValue(CurSel,0).lVal == -1) {
				m_LocationID = -1;
			}
			else {
				m_LocationID = m_LocationCombo->GetValue(CurSel,0).lVal;
				strLocID.Format("%li", m_LocationID);
			}
		}

		GetDlgItemText(IDC_COMMENTS,strComments);

		if(m_bAllowChange && m_strCPTIDs.IsEmpty() && m_ID == -1 &&
			IDNO == MessageBox("You have not chosen any Service Codes. Are you sure you wish to save this insurance referral?",
			"Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		//DRT 5/5/03 - Cpts and diags are now saved in their own tables
		//		parse each of the id lists for insertion
		CString strExec = "";

		if(m_ID == -1) {

			//new referral

			BEGIN_TRANS("InsuranceReferrals") {
				long nNewID = NewNumber("InsuranceReferralsT","ID");
				m_ID = nNewID;
				ExecuteSql("INSERT INTO InsuranceReferralsT "
					"(ID, InsuredPartyID, AuthNum, StartDate, EndDate, NumVisits, LocationID, ProviderID, Comments) VALUES "
					"(%li, %li, '%s', Convert(datetime,'%s'), Convert(datetime,'%s'), %li, %s, %s, '%s')",
					nNewID,m_InsuredPartyID,_Q(strAuthNum),FormatDateTimeForSql(dtStart, dtoDate),FormatDateTimeForSql(dtEnd, dtoDate),nNumVisits,strLocID,strProvID,_Q(strComments));

				//////////////////////////////
				// Insert cpt codes
				CString strTemp = m_strCPTIDs + " ";	//add a space for the parsing - it has to have some cpt codes
				CString strInsert;

				long nSpace = strTemp.Find(" ");
				while(nSpace > 0) {
					long nID = atoi(strTemp.Left(nSpace));
					strTemp = strTemp.Right(strTemp.GetLength() - (nSpace + 1));

					strInsert.Format("INSERT INTO InsuranceReferralCPTCodesT values (%li, %li);", nNewID, nID);
					strExec += strInsert + "\n";

					nSpace = strTemp.Find(" ");
				}

				if(!strExec.IsEmpty())
					ExecuteSql("%s", strExec);

				strExec.Empty();
				strTemp.Empty();

				///////////////////////////////
				// Insert diag codes
				strTemp = m_strDiagIDs + " ";	//add a space for parsing
				strTemp.Remove(',');
				strTemp.TrimLeft();	//it does not have to have diag codes

				nSpace = strTemp.Find(" ");
				while(nSpace > 0) {
					CString strID = (strTemp.Left(nSpace));
					strTemp = strTemp.Right(strTemp.GetLength() - (nSpace + 1));

					strInsert.Format("INSERT INTO InsuranceReferralDiagsT values (%li, %s)", nNewID, strID);
					strExec += strInsert;

					nSpace = strTemp.Find(" ");
				}

				if(!strExec.IsEmpty())
					ExecuteSql("%s", strExec);

			} END_TRANS_CATCH_ALL("InsuranceReferrals");
		}
		else {

			//update existing referral

			BEGIN_TRANS("InsuranceReferrals") {
				//format the provider so it doesnt get overwritten if an inactive is there
				CString strProvider;
				if(!m_ProviderCombo->IsComboBoxTextInUse)
					strProvider.Format("ProviderID = %s,", strProvID);

				ExecuteSql("UPDATE InsuranceReferralsT SET AuthNum = '%s', StartDate = Convert(datetime,'%s'), "
					"EndDate = Convert(datetime,'%s'), NumVisits = %li, LocationID = %s, %s Comments = '%s' "
					"WHERE ID = %li",
					_Q(strAuthNum),FormatDateTimeForSql(dtStart,dtoDate),FormatDateTimeForSql(dtEnd, dtoDate),nNumVisits,strLocID,strProvider,_Q(strComments),m_ID);

				//////////////////////////////
				// re-insert cpt codes
				CString strTemp = m_strCPTIDs + " ";	//add a space for the parsing - it has to have some cpt codes
				CString strInsert;

				ExecuteSql("DELETE FROM InsuranceReferralCPTCodesT WHERE ReferralID = %li",m_ID);

				long nSpace = strTemp.Find(" ");
				while(nSpace > 0) {
					long nID = atoi(strTemp.Left(nSpace));
					strTemp = strTemp.Right(strTemp.GetLength() - (nSpace + 1));

					strInsert.Format("INSERT INTO InsuranceReferralCPTCodesT values (%li, %li);", m_ID, nID);
					strExec += strInsert + "\n";

					nSpace = strTemp.Find(" ");
				}

				if(!strExec.IsEmpty())
					ExecuteSql("%s", strExec);

				strExec.Empty();
				strTemp.Empty();

				///////////////////////////////
				// re-insert diag codes
				strTemp = m_strDiagIDs + " ";	//add a space for parsing
				strTemp.Remove(',');
				strTemp.TrimLeft();	//it does not have to have diag codes

				ExecuteSql("DELETE FROM InsuranceReferralDiagsT WHERE ReferralID = %li",m_ID);

				nSpace = strTemp.Find(" ");
				while(nSpace > 0) {
					CString strID = (strTemp.Left(nSpace));
					strTemp = strTemp.Right(strTemp.GetLength() - (nSpace + 1));

					strInsert.Format("INSERT INTO InsuranceReferralDiagsT values (%li, %s)", m_ID, strID);
					strExec += strInsert;

					nSpace = strTemp.Find(" ");
				}

				if(!strExec.IsEmpty())
					ExecuteSql("%s", strExec);

			} END_TRANS_CATCH_ALL("InsuranceReferrals");
		}

	}NxCatchAll("Error saving referral information.");

	CDialog::OnOK();
}

void CNewInsuranceReferralDlg::OnMultiCPT()
{
	if(!m_bAllowChange)
		return;

	try {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "CPTCodeT");

		//see if we have anything already
		CString str = m_strCPTIDs + " ";
		str.TrimLeft();
		long nSpace = str.Find(" ");
		while(nSpace > 0) {
			dlg.PreSelect(atoi(str.Left(nSpace)));
			str = str.Right(str.GetLength() - (nSpace + 1));

			nSpace = str.Find(" ");
		}

		dlg.m_strNameColTitle = "Code";

		int res = dlg.Open("CPTCodeT INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID", "Active = 1", "ServiceT.ID", "CPTCodeT.Code", "Select Service Codes.");

		if(res == IDCANCEL)
			return;

		//save all our id's for later parsing
		m_strCPTIDs = dlg.GetMultiSelectIDString();

		DisplayCPTInfo();

	} NxCatchAll("Error in OnMultiCPT()");
}

void CNewInsuranceReferralDlg::OnSelChosenCptcodeCombo(long nRow) 
{
	try {
		//a single item was chosen.  this is disabled if they've done a multi-select, 
		//so we can safely update our id list
		if(m_CPTCodeCombo->GetCurSel() != -1) {
			//if the ID is -2, it's the "multiple" row
			if(VarLong(m_CPTCodeCombo->GetValue(nRow, 0)) == -2) {
				m_CPTCodeCombo->PutCurSel(-1);
				OnMultiCPT();
			}
			//if the ID is -1, it's the "none selected" row
			else if(VarLong(m_CPTCodeCombo->GetValue(nRow, 0)) == -1) {
				m_strCPTIDs = "";
			}
			else{
				m_strCPTIDs.Format("%li", VarLong(m_CPTCodeCombo->GetValue(nRow, 0)));
			}
		}
		else{
			m_strCPTIDs = "";
		}
	} NxCatchAll("Error in OnSelChosenCptcodeCombo()");
}



CString CNewInsuranceReferralDlg::GetStringOfCPTCodes() {

	CString str = "";

	CString strCPTs = m_strCPTIDs;
	strCPTs.TrimLeft();
	strCPTs.TrimRight();
	strCPTs.Replace(" "," OR ID = ");
	_RecordsetPtr rs = CreateRecordset("SELECT Code FROM CPTCodeT WHERE ID = %s",strCPTs);
	while(!rs->eof) {
		str += AdoFldString(rs, "Code","");
		str += ", ";
		rs->MoveNext();
	}
	rs->Close();

	str.TrimRight(", ");

	return str;
}

// (a.levy 2014-02-20 11:15) - PLID 60768  - Update the List with Values
void CNewInsuranceReferralDlg::UpdateDiagList(long nDiagID,CString strDiagCode, CString strDiagDesc) {


    NXDATALIST2Lib::IRowSettingsPtr pRow = m_InsDiagList->GetNewRow();

    //Add Rows
    pRow->PutValue(dclcDiagID, nDiagID);
    pRow->PutValue(dclcDiagCode,_bstr_t(strDiagCode.Trim()));
    pRow->PutValue(dclcDiagCodeDescription,_bstr_t(strDiagDesc.Trim()));
	m_InsDiagList->AddRowAtEnd(pRow, NULL);

    //Add Code and space for parsing
    m_strDiagIDs += " " + AsString(nDiagID);
    m_strDiagIDs.TrimLeft();

    if(m_InsDiagList->GetRowCount() >= 4) {
      
        m_InsDiagList->SetRedraw(TRUE);
    }
    

}

// (a.levy 2014-02-20 11:15)= - PLID 60768  - Overload - Just General update if record is removed etc.
void CNewInsuranceReferralDlg::UpdateDiagList() {

    m_strDiagIDs.Empty();
    NXDATALIST2Lib::IRowSettingsPtr pRow = m_InsDiagList->GetFirstRow();
    while(pRow) {
        
        m_strDiagIDs += " " + AsString(pRow->GetValue(dclcDiagID));
        m_strDiagIDs.TrimLeft();
        pRow = pRow->GetNextRow();
        

    }



}

void CNewInsuranceReferralDlg::DisplayCPTInfo()
{
	try {
		//if we only have 1 item, select it in the datalist, don't bother setting this all up
		CString strTemp = m_strCPTIDs;
		if(strTemp.Find(" ") == -1) {
			m_bIsCPTListHidden = FALSE;
			((CWnd*)GetDlgItem(IDC_CPTCODE_COMBO))->ShowWindow(SW_SHOW);
			((CWnd*)GetDlgItem(IDC_MULTI_CPT_LABEL))->ShowWindow(SW_HIDE);

			m_CPTCodeCombo->SetSelByColumn(0, long(atoi(strTemp)));
			return;
		}

		m_bIsCPTListHidden = TRUE;

		//populate the readable string
		m_strCPTList = GetStringOfCPTCodes();

		//hide the datalist, the string will paint itself
		((CWnd*)GetDlgItem(IDC_CPTCODE_COMBO))->ShowWindow(SW_HIDE);

		InvalidateRect(m_rcMultiCPTLabel);
	
	}NxCatchAll("Error displaying CPT information.");
}


void CNewInsuranceReferralDlg::DoClickHyperlink(UINT nFlags, CPoint point)
{

	if (m_bIsCPTListHidden) {
		if (m_rcMultiCPTLabel.PtInRect(point)) {
			OnMultiCPT();
		}	
	}	
}

void CNewInsuranceReferralDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	DoClickHyperlink(nFlags, point);
	CDialog::OnLButtonDown(nFlags, point);
}

void CNewInsuranceReferralDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	DoClickHyperlink(nFlags, point);
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CNewInsuranceReferralDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	DrawCPTLabel(&dc);
}

void CNewInsuranceReferralDlg::DrawCPTLabel(CDC *pdc)
{
	// Draw the cpt codes
	if(m_bIsCPTListHidden) {
		// (j.jones 2008-05-01 16:17) - PLID 29874 - Set background color to transparent
		DrawTextOnDialog(this, pdc, m_rcMultiCPTLabel, m_strCPTList, m_bIsCPTListHidden?dtsHyperlink:dtsDisabledHyperlink, false, DT_LEFT, true, false, 0);
	}
}

BOOL CNewInsuranceReferralDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	if (m_bIsDiagListHidden) {
		if (m_rcMultiDiagLabel.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}
	if (m_bIsCPTListHidden) {
		if (m_rcMultiCPTLabel.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CNewInsuranceReferralDlg::OnRequeryFinishedCptcodeCombo(short nFlags) 
{
}

void CNewInsuranceReferralDlg::EnableControls(bool bEnable) {

	GetDlgItem(IDC_AUTH_NUM)->EnableWindow(bEnable);
	GetDlgItem(IDC_REFERRAL_START_DATE)->EnableWindow(bEnable);
	//GetDlgItem(IDC_REFERRAL_END_DATE)->EnableWindow(bEnable);
	//GetDlgItem(IDC_NUM_VISITS)->EnableWindow(bEnable);
	GetDlgItem(IDC_CPTCODE_COMBO)->EnableWindow(bEnable);
	GetDlgItem(IDC_PROVIDER_REF_COMBO)->EnableWindow(bEnable);
	GetDlgItem(IDC_LOCATION_REF_COMBO)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMMENTS)->EnableWindow(bEnable);
	GetDlgItem(IDC_MULTI_CPT_LABEL)->EnableWindow(bEnable);
}

 // (a.levy 2014-02-20 10:36) - PLID - 60768 - Get Selected Code convert to string for display
void CNewInsuranceReferralDlg::OnSelectInsuranceRefDiagnosisSearch(LPDISPATCH lpRow)
{
    CString strDiagCode, strDiagDescription;
    NXDATALIST2Lib::IRowSettingsPtr pFoundRow = NULL;
   long nDiagID = -1;
   long nTempDiagID;
   try {
	  

       if(lpRow) {

       
			CDiagSearchResults results = DiagSearchUtils::ConvertDualSearchResults(lpRow);
			//Get the code that was selected, it could be either a 9 or 10 code not both.
			if (results.m_ICD9.m_nDiagCodesID != -1) {
				nDiagID = results.m_ICD9.m_nDiagCodesID;
				strDiagCode = results.m_ICD9.m_strCode;
				strDiagDescription = results.m_ICD9.m_strDescription;
                pFoundRow = m_InsDiagList->FindByColumn(dclcDiagID, results.m_ICD9.m_nDiagCodesID, m_InsDiagList->GetFirstRow(), VARIANT_FALSE);
			} else if (results.m_ICD10.m_nDiagCodesID != -1) {
				nDiagID = results.m_ICD10.m_nDiagCodesID;
				strDiagCode = results.m_ICD10.m_strCode;
				strDiagDescription = results.m_ICD10.m_strDescription;
                pFoundRow = m_InsDiagList->FindByColumn(dclcDiagID, results.m_ICD10.m_nDiagCodesID, m_InsDiagList->GetFirstRow(), VARIANT_FALSE);
			} else {
             
              return;
            }
             nTempDiagID = nDiagID;
            
            // Check make sure DiagCode Does not already exist if so relay message and ignore
		    //do the codes match?
	        if(pFoundRow)
		    {
		        AfxMessageBox("This diagnosis has already been selected.");
		        return;
	        }

            UpdateDiagList(nDiagID,strDiagCode, strDiagDescription);
       }

	   
       
   }NxCatchAll("Error in NewInsuranceReferralDlg::OnSelectInsuranceRefDiagnosisSearch() " + __LINE__);
}

// (a.levy 2014-03-05 12:22) - PLID 60868 - Handle Right Click and delete code
void CNewInsuranceReferralDlg::OnRightClickDiagCodeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
 
    try {
            NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
            if(pRow == NULL) {
                // No need to remove Empty
                return;
            }

            m_InsDiagList->CurSel = pRow;
            CString strCodeToRemove = AsString(pRow->GetValue(dclcDiagCode));
  
            enum {
			    eRemoveDiag = 1,
		    };
            // Create the menu  - copied from General2Dialog - Alert user code they are removing
		    CMenu mnu;
		    mnu.CreatePopupMenu();
		    CString strLabel;
		    strLabel.Format("&Remove Diagnosis Code %s", strCodeToRemove);
		    mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eRemoveDiag, strLabel);

            CPoint pt;
		    GetCursorPos(&pt);

            int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		    if(nRet == eRemoveDiag) {
            
                //Remove code from List
                m_InsDiagList->RemoveRow(pRow);

				//Update from changed list
				UpdateDiagList();
            }           

    }NxCatchAll(__FUNCTION__);
}

  