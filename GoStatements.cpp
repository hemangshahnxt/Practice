// GoStatements.cpp : implementation file
//

#include "stdafx.h"
#include "GlobalDataUtils.h"
#include "GlobalReportUtils.h"
#include "GoStatements.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "StatementSetup.h"
#include "StatementSetup.h"
#include "dontshowdlg.h"

using namespace ADODB;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_RUNPREVIEW   7

// (a.walling 2008-07-07 17:31) - PLID 29900 - GetActivePatientID replaced with m_nPatientID

/////////////////////////////////////////////////////////////////////////////
// CGoStatements dialog


CGoStatements::CGoStatements(CWnd* pParent /*=NULL*/)
	: CNxDialog(CGoStatements::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGoStatements)
		m_nPatientID = -1;
	//}}AFX_DATA_INIT
}


void CGoStatements::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGoStatements)
	DDX_Control(pDX, IDC_BYPATIENT, m_btnByPatient);
	DDX_Control(pDX, IDC_BYLOCATION, m_btnByLocation);
	DDX_Control(pDX, IDC_BYPROVIDER, m_btnByProvider);
	DDX_Control(pDX, IDC_BILL_FILTER_CHECK, m_billCheck);
	DDX_Control(pDX, IDC_TRANSACTION, m_dateCheck);
	DDX_Control(pDX, IDC_LOCFILTERCHK, m_locationCheck);
	DDX_Control(pDX, IDC_DETAILED, m_detailedRadio);
	DDX_Control(pDX, IDC_SUMMARY, m_summaryRadio);
	DDX_Control(pDX, IDC_TO, m_to);
	DDX_Control(pDX, IDC_FROM, m_from);
	DDX_Control(pDX, IDC_FROM_LABEL, m_nxstaticFromLabel);
	DDX_Control(pDX, IDC_TO_LABEL, m_nxstaticToLabel);
	DDX_Control(pDX, IDC_STMTPREVIEW, m_btnStmtPreview);
	DDX_Control(pDX, IDC_RUNCONFIG, m_btnRunConfig);
	DDX_Control(pDX, IDC_CANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGoStatements, CNxDialog)
	//{{AFX_MSG_MAP(CGoStatements)
	ON_BN_CLICKED(IDC_RUNCONFIG, OnRunConfig)
	ON_BN_CLICKED(IDC_STMTPREVIEW, OnClickPreview)
	ON_BN_CLICKED(IDC_DETAILED, OnDetailed)
	ON_BN_CLICKED(IDC_SUMMARY, OnSummary)
	ON_BN_CLICKED(IDC_TRANSACTION, OnTransaction)
	ON_BN_CLICKED(IDC_LOCFILTERCHK, OnLocfilterchk)
	ON_BN_CLICKED(IDC_BILL_FILTER_CHECK, OnBillFilterCheck)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	ON_BN_CLICKED(IDC_BYLOCATION, OnBylocation)
	ON_BN_CLICKED(IDC_BYPATIENT, OnBypatient)
	ON_BN_CLICKED(IDC_BYPROVIDER, OnByprovider)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CGoStatements, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CGoStatements)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CGoStatements, IDC_DATE_FILTER_TYPE_LIST, 1, CGoStatements::SelChangingDateFilterTypeList, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()


BOOL CGoStatements::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		COleDateTime dateTmp;

		// (c.haag 2008-05-01 17:25) - PLID 29876 - NxIconify buttons
		m_btnStmtPreview.AutoSet(NXB_PRINT_PREV);
		m_btnRunConfig.AutoSet(NXB_MODIFY);
		m_btnCancel.AutoSet(NXB_CLOSE);

		m_ptrLocations = BindNxDataListCtrl(this, IDC_LOCFILTER, GetRemoteData(), true);
		m_ptrBills = BindNxDataListCtrl(this, IDC_BILLFILTER, GetRemoteData(), FALSE);
		m_pReportList = BindNxDataListCtrl(this, IDC_REPORT_LIST, GetRemoteData(), FALSE);

		// (j.gruber 2009-02-17 14:56) - PLID 22534 - Added date type filter
		m_pDateFilterList = BindNxDataList2Ctrl(this, IDC_DATE_FILTER_TYPE_LIST, GetRemoteData(), FALSE);

		// (j.jones 2011-04-12 11:01) - PLID 31219 - added caching
		g_propManager.CachePropertiesInBulk("CGoStatements-Number", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'SttmntIncludePaidCharges' OR "
			"Name = 'SttmntEnvelope' OR "
			"Name = 'SttmntAge' OR "
			"Name = 'SttmntUseGuarantor' OR "
			"Name = 'SttmntUseDocName' OR "
			"Name = 'SttmntUseDocAddress' OR "
			"Name = 'SttmntHidePrePayments' OR "
			"Name = 'SttmntUseComma' OR "
			"Name = 'SttmntShowDiag' OR "
			"Name = 'SttmntRemitLocation' OR "
			"Name = 'SttmntShowFooter' OR "
			"Name = 'SttmntUseRemit' OR "
			"Name = 'SttmntUse70Version' OR "
			"Name = 'SttmntAcceptVisa' OR "
			"Name = 'SttmntAcceptMstrCard' OR "
			"Name = 'SttmntAcceptDiscover' OR "
			"Name = 'SttmntAcceptAmex' OR "
			"Name = 'SttmntDaysOld' OR "
			"Name = 'SttmntCombineBillsAfterXDaysOld' OR "
			"Name = 'SttmntShowLastPayInfo' OR "
			"Name = 'SttmntHideChargebacks' " //TES 7/17/2014 - PLID 62563
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("CGoStatements-Text", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'SttmntPaymentDesc' OR "
			"Name = 'SttmntAdjustmentDesc' OR "
			"Name = 'SttmntRefundDesc' "
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("CGoStatements-Memo", propMemo,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'SttmntName' OR "
			"Name = 'SttmntCallMe' OR "
			"Name = 'SttmntText' OR "
			"Name = 'Sttmnt30DayNote' OR "
			"Name = 'Sttmnt60DayNote' OR "
			"Name = 'Sttmnt90DayNote' OR "
			"Name = 'Sttmnt90+DayNote' "
			")",
			_Q(GetCurrentUserName()));

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDateFilterList->GetNewRow();
		pRow->PutValue(0, (long)1);
		pRow->PutValue(1, _variant_t("Transaction Date"));
		m_pDateFilterList->AddRowAtEnd(pRow, NULL);
		m_pDateFilterList->CurSel = pRow;

		pRow = m_pDateFilterList->GetNewRow();
		pRow->PutValue(0, (long)2);
		pRow->PutValue(1, _variant_t("Bill Date/Payment Date"));
		m_pDateFilterList->AddRowAtEnd(pRow, NULL);

		GetDlgItem(IDC_DATE_FILTER_TYPE_LIST)->EnableWindow(FALSE);

		
		//set detailed to be checked as the default
		m_detailedRadio.SetCheck(1);
		m_summaryRadio.SetCheck(0);
		//set the grayed out date boxes to today's date
		m_from.SetValue(COleVariant(COleDateTime::GetCurrentTime()));
		m_to.SetValue(COleVariant(COleDateTime::GetCurrentTime()));	
		//grey out the date buttons as default
		m_from.EnableWindow(FALSE);
		m_to.EnableWindow(FALSE);

		//set the bill filter to be 0
		m_billCheck.SetCheck(0);
		GetDlgItem(IDC_BILLFILTER)->EnableWindow(FALSE);
		m_ptrBills->DropDownWidth = 400;

		//set the where clause to be just for this patient
		CString strBillFilter;
		strBillFilter.Format("BillsT.EntryType = 1 AND BillsT.Deleted = 0 AND BillsT.PatientID = %li", m_nPatientID);
		m_ptrBills->WhereClause = (LPCTSTR)strBillFilter;
		m_ptrBills->Requery();

		//set locations filter check box to unchecked as default
		m_locationCheck.SetCheck(0);

		//grey out the locations datalist
		GetDlgItem(IDC_LOCFILTER)->EnableWindow(false);

		m_brush.CreateSolidBrush(PaletteColor(0x00BCB3AD));


		//get out of the data whether they normally use by location or by patient
		long nUseLocation = GetRemotePropertyInt("SttmntUseLocation", 0); 
		long nStmtType;
		if (nUseLocation) {
			//they use the by location one
			CheckDlgButton(IDC_BYLOCATION, true);
			nStmtType = BYLOCATION;
		}
		else {
			
			long nUseProvider = GetRemotePropertyInt("SttmntUseProvider", 0);
			if (nUseProvider) {
				CheckDlgButton(IDC_BYPROVIDER, true);
				nStmtType = BYPROVIDER;
			}
			else {
				CheckDlgButton(IDC_BYPATIENT, true);
				nStmtType = BYPATIENT;
			}
		}

		ResetReportList();
	}
	NxCatchAll("Error in CGoStatements::OnInitDialog");
		
	
	return TRUE;  
}

/////////////////////////////////////////////////////////////////////////////
// CGoStatements message handlers

/*void CGoStatements::AddToReportList(CString strReportName, long nReportID) {

	long nCount = 0;
	long nFormatType = GetRemotePropertyInt("SttmntEnvelope", 1, 0, "<None>");
	IRowSettingsPtr pRow;

	switch (nFormatType) {

		case 0: //avery 1

			//we just need detailed or summary here
			pRow = m_pReportList->GetRow(-1);
			pRow->PutValue(0, (long) 0);
			pRow->PutValue(1, (long) -1);
			pRow->PutValue(2, _variant_t("Detailed"));
			pRow->PutValue(3, _variant_t(strReportName + "dtldavery1"));
			m_pReportList->AddRow(pRow);

			pRow = m_pReportList->GetRow(-1);
			pRow->PutValue(0, (long) 1);
			pRow->PutValue(1, (long) -1);
			pRow->PutValue(2, _variant_t("Summary"));
			pRow->PutValue(3, _variant_t(strReportName + "smryavery1"));
			m_pReportList->AddRow(pRow);

			//set the current selection to be detailed
			m_pReportList->SetSelByColumn(0, (long)0);			

			break;

		case 1: //avery2

			//we just need detailed or summary here
			pRow = m_pReportList->GetRow(-1);
			pRow->PutValue(0, (long) 0);
			pRow->PutValue(1, (long) -1);
			pRow->PutValue(2, _variant_t("Detailed"));
			pRow->PutValue(3, _variant_t(strReportName + "dtldavery2"));
			m_pReportList->AddRow(pRow);

			pRow = m_pReportList->GetRow(-1);
			pRow->PutValue(0, (long) 1);
			pRow->PutValue(1, (long) -1);
			pRow->PutValue(2, _variant_t("Summary"));
			pRow->PutValue(3, _variant_t(strReportName + "smryavery2"));
			m_pReportList->AddRow(pRow);

			//set the current selection to be detailed
			m_pReportList->SetSelByColumn(0, (long)0);			

			break;

		case 2: { //default report 
			_RecordsetPtr rs = CreateRecordset("SELECT ID, Number, Title, FileName FROM CustomReportsT WHERE ID = %li", nReportID);
			while (!rs->eof) {
				pRow = m_pReportList->GetRow(-1);
				pRow->PutValue(0, AdoFldLong(rs, "ID"));
				pRow->PutValue(1, AdoFldLong(rs, "Number"));
				pRow->PutValue(2, _variant_t(AdoFldString(rs, "Title")));
				pRow->PutValue(3, _variant_t(AdoFldString(rs, "FileName")));
				m_pReportList->AddRow(pRow);

				rs->MoveNext();
			}

			//set the selection to be the default report
			rs = CreateRecordset("SELECT CustomReportID FROM DefaultReportsT WHERE ID = %li", nReportID);
			if (! rs->eof) {
				m_pReportList->SetSelByColumn(1, AdoFldLong(rs, "CustomReportID"));
			}
				}
		break;

			

		case 3: //estatement

			//e-statement report
			pRow = m_pReportList->GetRow(-1);
			pRow->PutValue(0, (long) 0);
			pRow->PutValue(1, (long) -1);
			pRow->PutValue(2, _variant_t("E-Statement Summary"));
			pRow->PutValue(3, "");
			m_pReportList->AddRow(pRow);

			pRow = m_pReportList->GetRow(-1);
			pRow->PutValue(0, (long)1);
			pRow->PutValue(1, (long)-1);
			pRow->PutValue(2, _variant_t("E-Statement Detailed"));
			pRow->PutValue(3, "");
			m_pReportList->AddRow(pRow);

			//set the summary to be default
			m_pReportList->SetSelByColumn(0,(long)0);
		break;
	}



}

*/

void CGoStatements::ResetReportList() {

	//first see what is checked
	CString strReportName;
	long nStmtType;
	if (IsDlgButtonChecked(IDC_BYPROVIDER)) {
		nStmtType = BYPROVIDER;
	}
	else if (IsDlgButtonChecked(IDC_BYLOCATION)) {
		nStmtType = BYLOCATION;
	}
	else {
		nStmtType = BYPATIENT;
	}

	long nVersion;
	//now see whether they are using 6.0 or 7.0 version
	if (GetRemotePropertyInt("SttmntUse70Version", 0, 0, "<None>")) {
		strReportName += "70";
		nVersion = 7;
	}
	else {
		nVersion = 6;
	}

	long nReportID;
	switch (nStmtType) {
		case BYPATIENT:
			if (nVersion == 7) {
				nReportID = 353;
			}
			else {
				nReportID = 234;
			}
		break;

		case BYLOCATION:
			if (nVersion == 7) {
				nReportID = 356;
			}
			else {
				nReportID = 338;
			}
		break;

		case BYPROVIDER:
			if (nVersion == 7) {
				nReportID = 486;
			}
			else {
				nReportID = 484;
			}
		break;
	}
	
	// (j.gruber 2010-03-11 12:29) - PLID 29120 - added variable
	SetReportList(m_pReportList, nReportID, NULL,TRUE);




}

void CGoStatements::OnRunConfig() 
{
	//if they click configuration, show the configuration dialog
	CStatementSetup dlgConfig(this);
	dlgConfig.DoModal();
	ResetReportList();

}



void CGoStatements::OnClickPreview() 
{
	CWaitCursor pWait;
	CPtrArray paramList;
	CRParameterInfo *tmpPInfo;
	long PatientID;
	long nDateFilter = -1;
	if(m_nPatientID == -1)
		PatientID = GetActivePatientID();
	else
		PatientID = m_nPatientID;
	COleDateTime  dateto, datefrom, dateTmp;
	CString strtmp;
	long nStyle = GetRemotePropertyInt("SttmntEnvelope", 1, 0, "<None>");

	//check to make sure there is something in the drop down
	if (m_pReportList->GetRowCount() == 0) {
		//they don't have anything selected, so make them pick something
		//the only reason that should happen is because of the default, so check that first
		if (nStyle == 2) {
			MessageBox("There are no custom reports for this type of statement, please create a custom report and run this statement again or run a different statement.");
			return;
		}
	}

	if (m_pReportList->CurSel == -1) {
		MessageBox("Please select a report to run from the drop down");
		return;
	}


	try{
		//check to see the data is there
		EnsureRemoteData();
		
		//check to see if they want to filter by date
		if (m_dateCheck.GetCheck()) {
			// (a.walling 2008-05-13 15:28) - PLID 27591 - .date no longer necessary
			datefrom = m_from.GetValue();
			dateto = m_to.GetValue();

			// (j.gruber 2009-02-17 15:04) - PLID 32534 - added date filter list
			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pDateFilterList->CurSel;
			if (pRow) {
				nDateFilter = VarLong(pRow->GetValue(0));
			}
		}
		else {
			datefrom.ParseDateTime("01/01/1800");
			dateto.ParseDateTime("12/31/4000");
		}

		if (m_billCheck.GetCheck()) {
			long nCurSel = m_ptrBills->CurSel;
			if (nCurSel != -1) {
				m_nBillID = VarLong(m_ptrBills->GetValue(nCurSel, 0));
			}
			else { 
				MsgBox("Please select a valid bill to filter on or uncheck the Filter by Bill box");
				return;
			}
		}
		else {
			m_nBillID = -1;
		}


		 //check to see if they want to filter by location
		 if (m_locationCheck.GetCheck()) {

			 //get the id of the location to filter by
			 m_nLocID = VarLong(m_ptrLocations->GetValue(m_ptrLocations->GetCurSel(), 0));
				
		 }
		 else {
			 m_nLocID = -1;
		 }		 
		


			
			//Set the parameters for the statement
			//Use the guarantor info if the patient is under a specified age
			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%d", GetRemotePropertyInt("SttmntUseGuarantor"));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"UseGuar";
			paramList.Add((void *)tmpPInfo);

			//Person's name that they can fill in, usually the patient coordinator
			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%s", GetRemotePropertyMemo("SttmntName"));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"ContactPerson";
			paramList.Add((void *)tmpPInfo);

			//whether to use the Location name or the doctor's name
			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%d", GetRemotePropertyInt("SttmntUseDocName"));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"DocName";
			paramList.Add((void *)tmpPInfo);

			//whether to use the Location address or the doctor's address
			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%d", GetRemotePropertyInt("SttmntUseDocAddress"));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"DocAddress";
			paramList.Add((void *)tmpPInfo);

			//whether to hide prepayments or not
			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%d", GetRemotePropertyInt("SttmntHidePrePayments"));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"HidePrePays";
			paramList.Add((void *)tmpPInfo);			
			
			
			//I have no idea what this is, isn't that sad
			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%s", GetRemotePropertyMemo( "SttmntCallMe"));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"StmmtTitle";
			paramList.Add((void *)tmpPInfo); 

			//How to format the patient's name
			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%d", GetRemotePropertyInt("SttmntUseComma"));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"UseComma";
			paramList.Add((void *)tmpPInfo);

			//the custom text for the statement, for all patients
			tmpPInfo = new CRParameterInfo;
			tmpPInfo->m_Data = GetRemotePropertyMemo("SttmntText",(CString)"-");
			tmpPInfo->m_Name = (CString)"CustomText";
			paramList.Add((void *)tmpPInfo);

			//whether or not to show diagnosis codes
			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%d", GetRemotePropertyInt("SttmntShowDiag"));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"ShowDiag";
			paramList.Add((void *)tmpPInfo);

			//what age they want to use for the guarantor stuff
			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%d", GetRemotePropertyInt("SttmntAge", 16));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"SttmntAge";
			paramList.Add((void *)tmpPInfo);
			
			//whether or not to show the statement footer
			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%d", GetRemotePropertyInt("SttmntShowFooter", -1));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"SttmntFooter";
			paramList.Add((void *)tmpPInfo);
			
			// (j.gruber 2007-01-05 10:37) - PLID 24036 - add combine bill balance info
			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%d", GetRemotePropertyInt("SttmntCombineBillsAfterXDaysOld", 0, 0, "<None>"));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"SttmntCombineBillBalance";
			paramList.Add((void *)tmpPInfo);

			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%d", GetRemotePropertyInt("SttmntDaysOld", 60, 0, "<None>"));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"SttmntDaysOld";
			paramList.Add((void *)tmpPInfo);

			// (j.gruber 2007-01-09 10:18) - PLID 24168  - add last pay show info
			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%d", GetRemotePropertyInt("SttmntShowLastPayInfo", 0, 0, "<None>"));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"SttmntShowLastPayInfo";
			paramList.Add((void *)tmpPInfo);

			// (j.jones 2011-04-12 10:46) - PLID 31219 - added ability to show all charges
			// on any bills with balances, when on the summary statement
			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%li", GetRemotePropertyInt("SttmntIncludePaidCharges", 0, 0, "<None>"));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"SttmntIncludePaidCharges";
			paramList.Add((void *)tmpPInfo);

			//check to see whether we remit to a certain address
			long nRemit = GetRemotePropertyInt("SttmntUseRemit", 0, 0, "<None>");

			if (nRemit) {

				//get the location id
				long nLocationID = GetRemotePropertyInt("SttmntRemitLocation", -1, 0, "<None>");

				//get the name and address info for the location
				_RecordsetPtr rsLocation = CreateRecordset("SELECT Name, Address1, Address2, City, State, Zip FROM LocationsT WHERE ID = %li", nLocationID);
				CString strName, strAddress1, strAddress2, strCity, strState, strZip;

				if (rsLocation->eof) {

					//output an error message because they said to use remit but have an invalid location
					MsgBox("The location selected to remit to is invalid.  \nPlease check that this is a current location in the statement configuration and then run the statement again.");
					for (long i=0; i < paramList.GetSize(); i++)
						if (paramList[i]) delete ((CRParameterInfo*)paramList[i]);
					paramList.RemoveAll();
					return;

				}
				else {

					strName = AdoFldString(rsLocation, "Name");
					strAddress1 = AdoFldString(rsLocation, "Address1");
					strAddress2 = AdoFldString(rsLocation, "Address2");
					strAddress2.TrimLeft();
					strAddress2.TrimRight();
					if (strAddress2.IsEmpty()) {
						strAddress2 = "Empty";
					}

					strCity= AdoFldString(rsLocation, "City");
					strState= AdoFldString(rsLocation, "State");
					strZip= AdoFldString(rsLocation, "Zip");
				}

				//remit name
				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = strName;
				tmpPInfo->m_Name = (CString)"SttmntRemitLocationName";
				paramList.Add((void *)tmpPInfo);

				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = strAddress1;
				tmpPInfo->m_Name = (CString)"SttmntRemitLocationAddress1";
				paramList.Add((void *)tmpPInfo);

				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = strAddress2;
				tmpPInfo->m_Name = (CString)"SttmntRemitLocationAddress2";
				paramList.Add((void *)tmpPInfo);
			
				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = strCity;
				tmpPInfo->m_Name = (CString)"SttmntRemitLocationCity";
				paramList.Add((void *)tmpPInfo);

				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = strState;
				tmpPInfo->m_Name = (CString)"SttmntRemitLocationState1";
				paramList.Add((void *)tmpPInfo);
			
				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = strZip;
				tmpPInfo->m_Name = (CString)"SttmntRemitLocationZip";
				paramList.Add((void *)tmpPInfo);

			}
			else {

				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = "None";
				tmpPInfo->m_Name = (CString)"SttmntRemitLocationName";
				paramList.Add((void *)tmpPInfo);

				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = "None";
				tmpPInfo->m_Name = (CString)"SttmntRemitLocationAddress1";
				paramList.Add((void *)tmpPInfo);

				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = "None";
				tmpPInfo->m_Name = (CString)"SttmntRemitLocationAddress2";
				paramList.Add((void *)tmpPInfo);
			
				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = "None";
				tmpPInfo->m_Name = (CString)"SttmntRemitLocationCity";
				paramList.Add((void *)tmpPInfo);

				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = "None";
				tmpPInfo->m_Name = (CString)"SttmntRemitLocationState1";
				paramList.Add((void *)tmpPInfo);
			
				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = "None";
				tmpPInfo->m_Name = (CString)"SttmntRemitLocationZip";
				paramList.Add((void *)tmpPInfo);
			}


			//Credit Descriptions
			CString strPaymentDesc, strAdjustmentDesc, strRefundDesc;
			strPaymentDesc = GetRemotePropertyText("SttmntPaymentDesc", "", 0, "<None>");
			strAdjustmentDesc = GetRemotePropertyText("SttmntAdjustmentDesc", "", 0, "<None>");
			strRefundDesc = GetRemotePropertyText("SttmntRefundDesc", "", 0, "<None>");


			tmpPInfo = new CRParameterInfo;
			tmpPInfo->m_Data = strPaymentDesc;
			tmpPInfo->m_Name = (CString)"SttmntPaymentDesc";
			paramList.Add((void *)tmpPInfo);


			tmpPInfo = new CRParameterInfo;
			tmpPInfo->m_Data = strAdjustmentDesc;
			tmpPInfo->m_Name = (CString)"SttmntAdjustmentDesc";
			paramList.Add((void *)tmpPInfo);

			
			tmpPInfo = new CRParameterInfo;
			tmpPInfo->m_Data = strRefundDesc;
			tmpPInfo->m_Name = (CString)"SttmntRefundDesc";
			paramList.Add((void *)tmpPInfo);


			//Credit Card Acceptance
			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%d", GetRemotePropertyInt("SttmntAcceptVisa", 1, 0, "<None>"));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"AcceptVisa";
			paramList.Add((void *)tmpPInfo);

			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%d", GetRemotePropertyInt("SttmntAcceptMstrCard", 1, 0, "<None>"));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"AcceptMstrCard";
			paramList.Add((void *)tmpPInfo);
			
			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%d", GetRemotePropertyInt("SttmntAcceptDiscover", 1, 0, "<None>"));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"AcceptDiscover";
			paramList.Add((void *)tmpPInfo);

			tmpPInfo = new CRParameterInfo;
			strtmp.Format("%d", GetRemotePropertyInt("SttmntAcceptAmex", 1, 0, "<None>"));
			tmpPInfo->m_Data = strtmp;
			tmpPInfo->m_Name = (CString)"AcceptAmex";
			paramList.Add((void *)tmpPInfo);


			//AR Notes
			CString strThirty, strSixty, strNinety, strNinetyPlus;
			strThirty = GetRemotePropertyMemo ("Sttmnt30DayNote", "", 0, "<None>");
			strSixty = GetRemotePropertyMemo ("Sttmnt60DayNote", "", 0, "<None>");
			strNinety = GetRemotePropertyMemo ("Sttmnt90DayNote", "", 0, "<None>");
			strNinetyPlus = GetRemotePropertyMemo ("Sttmnt90+DayNote", "", 0, "<None>");

			tmpPInfo = new CRParameterInfo;
			tmpPInfo->m_Data = strThirty;
			tmpPInfo->m_Name = (CString)"ThirtyDayNote";
			paramList.Add((void *)tmpPInfo);

			tmpPInfo = new CRParameterInfo;
			tmpPInfo->m_Data = strSixty;
			tmpPInfo->m_Name = (CString)"SixtyDayNote";
			paramList.Add((void *)tmpPInfo);

			tmpPInfo = new CRParameterInfo;
			tmpPInfo->m_Data = strNinety;
			tmpPInfo->m_Name = (CString)"NinetyDayNote";
			paramList.Add((void *)tmpPInfo);

			tmpPInfo = new CRParameterInfo;
			tmpPInfo->m_Data = strNinetyPlus;
			tmpPInfo->m_Name = (CString)"NinetyPlusNote";
			paramList.Add((void *)tmpPInfo);



			

			//set which report to run

			//check to see what version of the reports we are using
			long nUse70Version = GetRemotePropertyInt("SttmntUse70Version", 0, 0, "<None>");
			CString strRep;
			long nReportInfoID;
			if (IsDlgButtonChecked(IDC_BYLOCATION)) {
				
				//get what report they are trying to run
				
				if (nUse70Version) {
					strRep = "StatementLoc70";
					nReportInfoID = 356;
				}
				else {
					if (DontShowMeAgain(this, "There is a new, simpler layout of the statement available. \nWould you like to set Version 7.0 as your default?", "PatientStatements", "Statements", true, TRUE) == IDYES) {
						//set the configRt
						SetRemotePropertyInt("SttmntUse70Version", 1);
						//tell them how to change it back
						MsgBox("To set the statement back to the 6.0 version, go into Statement Configuration and uncheck the \"Use Version 7.0\" check box");
						strRep = "StatementLoc70";
						nReportInfoID = 356;
					}
					else {
						strRep = "StatementLoc";
						nReportInfoID = 338;
					}
				}
				SetRemotePropertyInt("SttmntUseLocation", 1);
				SetRemotePropertyInt("SttmntUseProvider", 0);
			}
			else if (IsDlgButtonChecked(IDC_BYPROVIDER)) {

				//check to make sure they aren't using the ebilling setup 
				if (nStyle == 3) {
					// (r.goldschmidt 2016-01-08 11:03) - PLID 67839 - by location e-statements are now allowed
					MsgBox("E-Statements are not available by provider. Please either choose a format other than E-Statement or run the E-Statement by patient or by location." );
					for (long i=0; i < paramList.GetSize(); i++)
						if (paramList[i]) delete ((CRParameterInfo*)paramList[i]);
					paramList.RemoveAll();
					return;
				}
				//get what report they are trying to run
				
				if (nUse70Version) {
					strRep = "StatementProv70";
					nReportInfoID = 486;
				}
				else {
					if (DontShowMeAgain(this, "There is a new, simpler layout of the statement available. \nWould you like to set Version 7.0 as your default?", "PatientStatements", "Statements", true, TRUE) == IDYES) {
						//set the configRt
						SetRemotePropertyInt("SttmntUse70Version", 1);
						//tell them how to change it back
						MsgBox("To set the statement back to the 6.0 version, go into Statement Configuration and uncheck the \"Use Version 7.0\" check box");
						strRep = "StatementProv70";
						nReportInfoID = 486;
					}
					else {
						strRep = "StatementProv";
						nReportInfoID = 484;
					}
				}
				SetRemotePropertyInt("SttmntUseLocation", 0);
				SetRemotePropertyInt("SttmntUseProvider", 1);

			}
			else {
				if (nUse70Version) {
					strRep = "Statement70";
					nReportInfoID = 354;
				}
				else {
					if (DontShowMeAgain(this, "There is a new, simpler layout of the statement available. \nWould you like to set Version 7.0 as your default?", "PatientStatements", "Statements", true, TRUE) == IDYES) {
						//set the configRt
						SetRemotePropertyInt("SttmntUse70Version", 1);

						//tell them how to change it back
						MsgBox("To set the statement back to the 6.0 version, go into Statement Configuration and uncheck the \"Use Version 7.0\" check box");
						strRep = "Statement70";
						nReportInfoID = 354;
					}
					else {
						strRep = "Statement";
						nReportInfoID = 234;
					}
					
				}
				SetRemotePropertyInt("SttmntUseLocation", 0);
				SetRemotePropertyInt("SttmntUseProvider", 0);
			}

			//create and run the report
			CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(nReportInfoID)]);
			infReport.nPatient = PatientID;
			infReport.DateTo = dateto;
			infReport.DateFrom = datefrom;			
			infReport.nDateFilter = (short)nDateFilter;

			infReport.nLocation = m_nLocID;
			//the extra ID here is going to be the Bill ID
			infReport.nExtraID = m_nBillID;

			// (r.gonet 12/18/2012) - PLID 53629 - Save the statement output types so if they change
			//  while the report is running, then we will have a cache of their values at the time of
			//  the report being run. 
			// (r.goldschmidt 2014-08-05 14:43) - PLID 62717 - category type also
			infReport.nStatementType = nStyle;
			infReport.nOutputType = GetRemotePropertyInt("SttmntSendToHistory", 0, 0, "<None>", true);
			infReport.nCategoryType = GetRemotePropertyInt("SttmntSendToHistoryCategory", 0, 0, "<None>", true);

					
			if (nStyle == 2) {
				//they have chosen the default report, so get what that is
				//PLID 15293 - we don't want to look this up anymore because we are going on what they have selected
				/*_RecordsetPtr rs = CreateRecordset("SELECT ID, CustomReportID FROM DefaultReportsT WHERE ID = %li", nReportInfoID);
				long nReportID;
				if (nReportInfoID == 234) {
					nReportID = 169;
				}
				else if (nReportInfoID == 338) {
					nReportID = 337;
				}
				else if (nReportInfoID == 356) {
					nReportID = 355;
				}
				else if (nReportInfoID == 354) {
					nReportID = 353;
				}
				else if (nReportInfoID == 484) {
					nReportID = 483;
				}
				else if (nReportInfoID == 486) {
					nReportID = 485;
				}
								
				if (rs->eof) {

					MsgBox("You have chosen to run a default report, but you have no default report chosen, please select a default report and run this statement again");
				}
				else {

					//It is my understanding that this cannot possibly be -1, because we do not
					//allow the user to select a default report for the statement that is not a
					//custom report.  But I could be wrong.
					long nDefaultCustomReportID = AdoFldLong(rs, "CustomReportID");
					
					//get the filename of the selected report
					_RecordsetPtr rsFileName = CreateRecordset("SELECT Filename FROM CustomReportsT WHERE ID = %li AND Number = %li", nReportID, nDefaultCustomReportID);
					
					if (! rsFileName->eof) { 
						CString strFileName = AdoFldString(rsFileName, "FileName" );*/
						infReport.nDefaultCustomReport = VarLong(m_pReportList->GetValue(m_pReportList->CurSel, 1));

						EndDialog(ID_RUNPREVIEW);
						//PLID 15293 - get the report name from the list instead of anything else
						CString strFileName = VarString(m_pReportList->GetValue(m_pReportList->CurSel, 3));
						infReport.ViewReport ("Patient Statement Custom Report", strFileName, &paramList, true, this);

					/*}
					else {
						MsgBox("Practice cannot find the Custom report.  Please call NexTech for support.");
					}
				}*/
			}
			else {

				CString strType, strFormat, strTitle;
				long nType;
				nType = VarLong(m_pReportList->GetValue(m_pReportList->CurSel, 1), 0);

				//check to see if they want detailed or summary
				if (nType == -1) {
					strType = "DTLD";
					strTitle = "Patient Statement Detailed";
				}
				else if (nType == -2) {
					strType = "SMRY";
					strTitle = "Patient Statement Summary";
				}
				else {
					MessageBox("Please select either summary or detailed");
					for (long i=0; i < paramList.GetSize(); i++)
						if (paramList[i]) delete ((CRParameterInfo*)paramList[i]);
					paramList.RemoveAll();
					return;
				}

				//now check to see what format they want
				if (nStyle == 0) {

					strFormat = "Avery1";
				}
				else if (nStyle == 1) {
					
					strFormat = "Avery2";
				}
				else if (nStyle == 3) {

					strFormat = "EBill";
					strTitle += " Ebilling Format";

					if (strType == "DTLD") {
						EndDialog(ID_RUNPREVIEW);
						RunEStatements(&infReport, FALSE, TRUE);
						for (long i=0; i < paramList.GetSize(); i++)
							if (paramList[i]) delete ((CRParameterInfo*)paramList[i]);
						paramList.RemoveAll();
						return;
					}
					else {
						EndDialog(ID_RUNPREVIEW);
						RunEStatements(&infReport, TRUE, TRUE);
						for (long i=0; i < paramList.GetSize(); i++)
							if (paramList[i]) delete ((CRParameterInfo*)paramList[i]);
						paramList.RemoveAll();
						return;
					}


				
				}

				strRep += strType + strFormat;

				//close the dialog
				CGoStatements::EndDialog(ID_RUNPREVIEW);

				//PLID 15293 - get the report name from the list instead of anything else
				strRep = VarString(m_pReportList->GetValue(m_pReportList->CurSel, 3));

				infReport.ViewReport (strTitle, strRep, &paramList, true, this);
				
			}
			
		} NxCatchAll("Error in GoStatements::OnClickPreview()");
	
		for (long i=0; i < paramList.GetSize(); i++)
			if (paramList[i]) delete ((CRParameterInfo*)paramList[i]);
		paramList.RemoveAll();
}

void CGoStatements::OnCancel() 
{
	CGoStatements::EndDialog(0);
		
}

void CGoStatements::OnDetailed() 
{
	// TODO: Add your control notification handler code here
	
}

void CGoStatements::OnSummary() 
{
	// TODO: Add your control notification handler code here
	
}

void CGoStatements::OnTransaction() 
{
	if (m_dateCheck.GetCheck())
	{
		//enable the date boxes
		m_from.EnableWindow(TRUE);
		m_to.EnableWindow(TRUE);
		// (j.gruber 2009-02-17 15:03) - PLID 32534 - added date filter type
		GetDlgItem(IDC_DATE_FILTER_TYPE_LIST)->EnableWindow(true);
		// Fill the To Date box with the latest transaction date for that patient
		COleDateTime dateTmp;
		_RecordsetPtr rs = CreateRecordset("SELECT Max(Date) as ToDate FROM LineItemT WHERE PatientID = %li AND Deleted = 0 AND Type <> 11", m_nPatientID);
		if (rs->eof) {
			m_to.SetValue((COleVariant)dateTmp.GetCurrentTime());
		}
		else m_to.SetValue((COleVariant)AdoFldDateTime(rs, "ToDate",dateTmp.GetCurrentTime()));
		rs->Close();

		// Fill the FromDate box with the earliest transaction date for that patient
		rs = CreateRecordset("SELECT Min(Date) as FromDate FROM LineItemT WHERE PatientID = %li AND Deleted = 0 AND Type <> 11", m_nPatientID);
		if (rs->eof) {
			m_from.SetValue((COleVariant)dateTmp.GetCurrentTime());
		}
		else m_from.SetValue((COleVariant)AdoFldDateTime(rs, "FromDate",dateTmp.GetCurrentTime()));
		rs->Close();
	}
	else
	{
		//when they click all dates, diable the date boxes
		m_from.EnableWindow(FALSE);
		m_to.EnableWindow(FALSE);
		GetDlgItem(IDC_DATE_FILTER_TYPE_LIST)->EnableWindow(FALSE);
	}
	
}

void CGoStatements::OnLocfilterchk() 
{
	if (m_locationCheck.GetCheck()) {
		//enable the location filer
		GetDlgItem(IDC_LOCFILTER)->EnableWindow(TRUE);

		//set it to default to the currect location
		m_ptrLocations->SetSelByColumn(0, GetCurrentLocationID());
	}
	else {
		GetDlgItem(IDC_LOCFILTER)->EnableWindow(FALSE);
	}

}


void CGoStatements::OnBillFilterCheck() 
{
	if (m_billCheck.GetCheck()) {
		//enable the location filer
		GetDlgItem(IDC_BILLFILTER)->EnableWindow(TRUE);

		//set it to -1
		m_ptrBills->CurSel = -1;

	}
	else {
		GetDlgItem(IDC_BILLFILTER)->EnableWindow(FALSE);
	}
	
}

void CGoStatements::OnBylocation() 
{
	ResetReportList();
	
}

void CGoStatements::OnBypatient() 
{
	ResetReportList();
	
}

void CGoStatements::OnByprovider() 
{
	ResetReportList();
	
}

void CGoStatements::SelChangingDateFilterTypeList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{	
	try {
		// (j.gruber 2009-02-17 16:58) - PLID 32534 - added the date type list
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("Error in CGoStatements::SelChangingDateFilterTypeList");
}
