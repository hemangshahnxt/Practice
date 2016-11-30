// EEligibility.cpp : implementation file
//

#include "stdafx.h"
#include "EEligibility.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "DocumentOpener.h"
#include "EligibilitySetupDlg.h."
#include "TrizettoEligibility.h"
#include "ANSI271Parser.h"
#include "EligibilityRequestDetailDlg.h"
#include "ECPEligibility.h"
#include "GlobalFinancialUtils.h"

// (j.jones 2007-04-26 10:40) - PLID 25867 - created E-Eligibility

#define LAYMANS_DEBUGGING_HEADERS

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define Success 0
#define Error_Missing_Info 1
#define Error_Other 2
// (j.jones 2007-10-05 14:40) - PLID 25867 - added Cancel_Silent
#define Cancel_Silent 3

#define EELIGIBILITY_TIMER_EVENT		22102

#define RETURN_ON_FAIL(function)	{ int nResult = function; if(nResult!=0) return nResult; }

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CEEligibility dialog


CEEligibility::CEEligibility(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEEligibility::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEEligibility)
		m_bCapitalizeEbilling = FALSE;
		m_bStripANSIPunctuation = FALSE;
		m_bUseRealTimeElig = FALSE;	// (j.jones 2010-07-02 09:17) - PLID 39486
		m_ertcClearinghouse = ertcTrizetto;
		m_FormatID = -1;
	//}}AFX_DATA_INIT

	// (c.haag 2010-10-14 11:15) - PLID 40352
	// (j.jones 2012-02-01 11:02) - PLID 40880 - the default is now 5010
	m_avANSIVersion = av5010;

	//TES 7/1/2011 - PLID 40938
	m_bUse1000APER = FALSE;
	m_strISA01Qual = "00";
	m_strISA03Qual = "00";
	m_strSubmitterISA05Qual = "ZZ";
	m_strReceiverISA07Qual = "ZZ";
	m_bEbillingFormatRecordExists = false;
	//(s.dhole 09/11/2012) PLID 52414
	m_bCEligibilityRequestDetailDlg = TRUE;
}


void CEEligibility::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEEligibility)
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CURR_ELIG_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_CURR_ELIG_EVENT, m_nxstaticCurrEligEvent);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEEligibility, CNxDialog)
	//{{AFX_MSG_MAP(CEEligibility)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CEEligibility::DestroyWindow() 
{
	try {
		//clear out the data
		for(int i=m_aryEligibilityInfo.GetSize()-1;i>=0;i--) {
			EligibilityInfo *pInfo = (EligibilityInfo*)m_aryEligibilityInfo.GetAt(i);
			delete pInfo;
			m_aryEligibilityInfo.RemoveAt(i);
		}
	}NxCatchAll("Error closing window.");
	
	return CDialog::DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CEEligibility message handlers

BOOL CEEligibility::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		// (j.jones 2008-05-07 15:32) - PLID 29854 - added nxiconbutton for modernization
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.jones 2010-07-06 10:25) - PLID 39486 - cached preferences
		g_propManager.CachePropertiesInBulk("CEEligibility-Number", propNumber,
			"(Username = '<None>') AND ("
			"Name = 'GEDIEligibilityRealTime_OpenNotepad' OR "
			"Name = 'GEDIEligibilityRealTime_UnbatchOnReceive' OR "
			"Name = 'LastEligBatchID' OR "
			"Name = 'EnvoyProduction' OR "
			"Name = 'EligibilityProduction' OR "
			"Name = 'Clearinghouse_LoginType' OR "			// (j.jones 2010-10-25 17:22) - PLID 40914 - added type of clearinghouse, 0 - Trizetto, 1 - ECP
			"Name = 'RealTimeEligibility_Clearinghouse' OR "
			// (j.jones 2016-05-16 09:58) - NX-100357 - added PhoneFormatString
			"Name = 'PhoneFormatString' "
			")");

		g_propManager.CachePropertiesInBulk("CEEligibility-Text", propText,
			"(Username = '<None>') AND ("
			"Name = 'GlobalClearinghouseLoginName' "
			")");

		g_propManager.CachePropertiesInBulk("CEEligibility-Image", propImage,
			"(Username = '<None>') AND ("
			"Name = 'GlobalClearinghousePassword'"
			")");

		// (j.jones 2010-10-25 17:22) - PLID 40914 - load the clearinghouse type
		m_ertcClearinghouse = (EligibilityRealTime_ClearingHouse)GetRemotePropertyInt("RealTimeEligibility_Clearinghouse", (long)ertcTrizetto, 0, "<None>", true);

		// (c.haag 2010-10-14 11:19) - PLID 40352 - Cache the toggle for ANSI 4010/5010
		//TES 7/1/2011 - PLID 40938 - Cache everything in EbillingFormatsT
		_RecordsetPtr rs = CreateParamRecordset("SELECT * FROM EbillingFormatsT WHERE ID = {INT}",m_FormatID);
		if(!rs->eof) {
			// (j.jones 2012-02-01 11:02) - PLID 40880 - the default is now 5010 (although this field is not nullable)
			m_avANSIVersion = (ANSIVersion)AdoFldLong(rs, "ANSIVersion", (long)av5010);
			m_bUse1000APER = AdoFldBool(rs, "Use1000APER", FALSE);
			m_strFilenameElig = AdoFldString(rs, "FilenameElig", "");
			m_strPER05Qual_1000A = AdoFldString(rs, "PER05Qual_1000A", "");
			m_strPER06ID_1000A = AdoFldString(rs, "PER06ID_1000A", "");
			m_strISA01Qual = AdoFldString(rs, "ISA01Qual", "00");
			m_strISA02 = AdoFldString(rs, "ISA02", "");
			m_strISA03Qual = AdoFldString(rs, "ISA03Qual", "00");
			m_strISA04 = AdoFldString(rs, "ISA04", "");
			m_strSubmitterISA05Qual = AdoFldString(rs, "SubmitterISA05Qual", "ZZ");
			m_strSubmitterISA06ID = AdoFldString(rs, "SubmitterISA06ID", "");
			m_strReceiverISA07Qual = AdoFldString(rs, "ReceiverISA07Qual", "ZZ");
			m_strReceiverISA08ID = AdoFldString(rs, "ReceiverISA08ID", "");
			m_strSubmitterGS02ID = AdoFldString(rs, "SubmitterGS02ID", "");
			m_strReceiverGS03ID = AdoFldString(rs, "ReceiverGS03ID", "");

			//TES 7/1/2011 - PLID 40938 - Remember that a valid record exists
			m_bEbillingFormatRecordExists = true;

		}
		rs->Close();
		
		SetTimer(EELIGIBILITY_TIMER_EVENT, 20, NULL);

	}NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEEligibility::ExportData()
{
	long nBatchID = 0;

	try {
		
		// update LastEligBatchID
		int iLastBatch = GetRemotePropertyInt(_T("LastEligBatchID"),0,0,_T("<None>"));
		if (iLastBatch == 0) {
			SetRemotePropertyInt(_T("LastEligBatchID"),1,0,_T("<None>"));
			nBatchID = 1;
		}
		else { // record does exist
			nBatchID = iLastBatch + 1;
			SetRemotePropertyInt(_T("LastEligBatchID"),nBatchID,0,_T("<None>"));
		}  // end of LastBatchID update

	} NxCatchAllCall("Error In CEEligibility::ExportData - Initialize", { return; });

	try {

		//set up the progress bar
		m_Progress.SetMin(0.0);		// progress bar setup
		m_Progress.SetMax(200.0);	//0 through 100 is for loading, 101 to 200 is for exporting
		m_Progress.SetValue(0.0);
		m_Progress.SetRedraw(TRUE);

		//the progress increment is decided in LoadRequestInfo (based on the count of requests)

		//load all the default information we will need		
		int nError = LoadRequestInfo();
		ResolveErrors(nError);
		if(nError != Success) {
			//ResolveErrors will give an appropriate warning, all we need to do is return
			return;
		}

		m_strBatchNumber.Format("%li",nBatchID);
		while (m_strBatchNumber.GetLength() < 4)
			m_strBatchNumber.Insert(0,'0');

		// (j.jones 2010-07-02 09:18) - PLID 39486 - if we are using SOAP calls,
		// we will not create an export file
		// (j.jones 2010-10-21 12:34) - PLID 40914 - renamed to reflect that this is not Trizetto-only
		if(!m_bUseRealTimeElig) {

			// (j.jones 2008-10-13 14:55) - PLID 31636 - use the configured filename
			CString strUserFile, strExt, strFilter;

			//TES 7/1/2011 - PLID 40938 - We've already loaded this value
			strUserFile = m_strFilenameElig;

			strUserFile.Replace(" ","");
			if(strUserFile.IsEmpty()) {
				strUserFile = "eeligibility.txt";
			}

			//see if they want to include the batch number
			if(strUserFile.Find("%b") != -1) {
				strUserFile.Replace("%b", m_strBatchNumber);
			}
			else if(strUserFile.Find("%") != -1) {
				//get the digits they need
				int pos = strUserFile.Find("%");
				
				CString strToReplace;
				if(strUserFile.GetLength() < pos + 2) {
					//bad formatting, just slap on the full batch number
					strUserFile = strUserFile.Left(pos) + m_strBatchNumber;
				}
				else {

					CString strToReplace = strUserFile.Mid(pos, 3);

					CString strTest = strToReplace;
					strTest.TrimLeft("%");
					strTest.TrimRight("b");
					if(!strTest.IsEmpty() && atoi(strTest) == 0) {
						//bad formatting, just slap on the full batch number
						strUserFile = strUserFile.Left(pos) + m_strBatchNumber;
					}
					else {

						CString strChar = strUserFile.GetAt(pos+1);
						int num = atoi(strChar);
						if(num == 0) {
							num = 2;
						}

						CString strNewNumber;
						if(num <= m_strBatchNumber.GetLength()) {
							strNewNumber = m_strBatchNumber.Right(num);
						}
						else {
							//pad the number with zeroes if needed
							strNewNumber = m_strBatchNumber;
							while(strNewNumber.GetLength() < num) {
								strNewNumber = "0" + strNewNumber;
							}
						}

						strUserFile.Replace(strToReplace, strNewNumber);
					}
				}
			}

			if(strUserFile.Find(".") != -1) {
				strExt = strUserFile.Right(strUserFile.GetLength() - strUserFile.Find(".") - 1);
			}
			else {
				strUserFile += ".txt";
				strExt = "txt";
			}

			//create the ebilling directory - does not give error if directory exists
			// (j.armen 2011-10-25 13:48) - PLID 46134 - Ebilling is located in the practice path
			// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure. Also, changed to FileUtils::CreatePath() so that 
			// non-existent intermediate paths are created.
			FileUtils::CreatePath(GetEnvironmentDirectory() ^ "EBilling\\");

			strFilter.Format("Eligibility Files (*.%s)|*.%s|All Files (*.*)|*.*||", strExt, strExt);
			CFileDialog SaveAs(FALSE,NULL,strUserFile,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,strFilter);
			// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
			CString dir = GetEnvironmentDirectory() ^ "EBilling\\";
			SaveAs.m_ofn.lpstrInitialDir = (LPCSTR)dir;
			if (SaveAs.DoModal() == IDCANCEL) {
				EndDialog(IDCANCEL);
				return;
			}
			m_ExportName = SaveAs.GetPathName();
		}

	} NxCatchAllCall("Error In CEEligibility::ExportData - Load", { return; });

	//begin the export!

	try {
		
		ResolveErrors(ExportEligibility());

	} NxCatchAllCall("Error In CEEligibility::ExportData - Export", { return; });
}

int CEEligibility::LoadRequestInfo()
{
	//this recordset gets all the information we need to start the export. The idea is to get out all the key IDs that
	//will be used in further queries, and hopefully this will reduce the number of joins used in later queries.

	try {

		// (j.jones 2010-07-02 09:31) - PLID 39486 - if given m_aryRequestIDsToExport, only export those
		// request IDs, otherwise export all batched, selected requests
		CString strIDs;
		CSqlFragment sqlWhere("EligibilityRequestsT.Batched = 1 AND EligibilityRequestsT.Selected = 1");
		for(int i=0; i<m_aryRequestIDsToExport.GetSize(); i++) {
			if(!strIDs.IsEmpty()) {
				strIDs += ",";
			}
			strIDs += AsString((long)m_aryRequestIDsToExport.GetAt(i));
		}
		if(!strIDs.IsEmpty()) {
			sqlWhere = CSqlFragment("EligibilityRequestsT.ID IN ({INTSTRING})", strIDs);
		}
		
		// (j.jones 2008-09-09 15:57) - PLID 31138 - supported EligibilityPayerID
		// (j.jones 2009-08-05 10:08) - PLID 34467 - supported payer IDs per location / insurance
		// (j.jones 2012-08-06 14:36) - PLID 51917 - supported the payer ID structure change and the new EligPayerID field, also parameterized
		// (j.jones 2014-03-13 10:32) - PLID 61363 - use ICD-10 if the insurance go live date is <= today, also added diag codes here
		_RecordsetPtr rs = CreateParamRecordset("SELECT "
			"EligibilityRequestsT.ID, "
			"PatientsT.PersonID AS PatientID, "			
			"InsuredPartyT.PersonID AS InsuredPartyID, "
			"InsuranceCoT.PersonID AS InsuranceCoID, "
			"ProvidersT.PersonID AS ProviderID, "
			"EligibilityRequestsT.LocationID, "
			"InsuredPartyT.RelationToPatient, "
			"InsuranceCoT.Name AS InsCoName, "
			"CASE WHEN LocationPayerIDsQ.ID Is Not Null THEN LocationPayerIDsQ.EbillingID ELSE EbillingInsCoIDs.EbillingID END AS PayerID, "
			"InsuranceCoT.InsType, "
			"InsuranceCoT.HCFASetupGroupID, "
			"ProvidersT.TaxonomyCode, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, "
			"PersonProvidersT.Last + ', ' + PersonProvidersT.First + ' ' + PersonProvidersT.Middle AS ProviderName, "
			"LocationsT.Name AS LocationName, "
			"Convert(bit, CASE WHEN InsuranceCoT.ICD10GoLiveDate Is Not Null AND InsuranceCoT.ICD10GoLiveDate <= GetDate() THEN 1 ELSE 0 END) AS UseICD10Codes, "
			"Diag1.CodeNumber AS DiagCode1, Diag1.ICD10 AS DiagCode1_ICD10, "
			"Diag2.CodeNumber AS DiagCode2, Diag2.ICD10 AS DiagCode2_ICD10, "
			"Diag3.CodeNumber AS DiagCode3, Diag3.ICD10 AS DiagCode3_ICD10, "
			"Diag4.CodeNumber AS DiagCode4, Diag4.ICD10 AS DiagCode4_ICD10 "
			"FROM EligibilityRequestsT "
			"LEFT JOIN InsuredPartyT ON EligibilityRequestsT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN InsuranceLocationPayerIDsT ON InsuranceCoT.PersonID = InsuranceLocationPayerIDsT.InsuranceCoID AND EligibilityRequestsT.LocationID = InsuranceLocationPayerIDsT.LocationID "
			"LEFT JOIN EbillingInsCoIDs LocationPayerIDsQ ON InsuranceLocationPayerIDsT.EligibilityPayerID = LocationPayerIDsQ.ID "
			"LEFT JOIN EbillingInsCoIDs ON InsuranceCoT.EligPayerID = EbillingInsCoIDs.ID "
			"LEFT JOIN PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
			"LEFT JOIN ProvidersT ON EligibilityRequestsT.ProviderID = ProvidersT.PersonID "
			"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT JOIN PersonT PersonProvidersT ON ProvidersT.PersonID = PersonProvidersT.ID "
			"LEFT JOIN LocationsT ON EligibilityRequestsT.LocationID = LocationsT.ID "
			"LEFT JOIN DiagCodes Diag1 ON EligibilityRequestsT.Diagnosis1 = Diag1.ID "
			"LEFT JOIN DiagCodes Diag2 ON EligibilityRequestsT.Diagnosis2 = Diag2.ID "
			"LEFT JOIN DiagCodes Diag3 ON EligibilityRequestsT.Diagnosis3 = Diag3.ID "
			"LEFT JOIN DiagCodes Diag4 ON EligibilityRequestsT.Diagnosis4 = Diag4.ID "
			"WHERE {SQL} "
			"ORDER BY InsuredPartyT.InsuranceCoID, EligibilityRequestsT.ProviderID, EligibilityRequestsT.InsuredPartyID, EligibilityRequestsT.ID", sqlWhere);
		
		if(rs->eof) {
			//no requests!
			rs->Close();
			AfxMessageBox("There are no requests to be exported.");
			EndDialog(IDCANCEL);
			// (j.jones 2007-10-05 14:40) - PLID 25867 - changed from returning Success to returning Cancel_Silent
			//return Cancel_Silent so that we don't fire any other warnings after this one
			return Cancel_Silent;
		}

		long nCount = rs->GetRecordCount();

		m_ProgIncrement = (float)100.0 / (float)nCount;

		//store all this information into a struct, to keep less recordsets open and speed up the process.
		//again, this info will make subsequent queries more efficient
		while(!rs->eof) {
			FieldsPtr fields = rs->Fields;
			
			EligibilityInfo *pNew = new EligibilityInfo;

			//load fields into the new EligibilityInfo
			pNew->nID = AdoFldLong(rs, "ID");
			pNew->nPatientID = AdoFldLong(rs, "PatientID");			
			pNew->nInsuranceCoID = AdoFldLong(rs, "InsuranceCoID");
			pNew->nInsuredPartyID = AdoFldLong(rs, "InsuredPartyID");
			pNew->nProviderID = AdoFldLong(rs, "ProviderID");
			pNew->nLocationID = AdoFldLong(rs, "LocationID");
			// (j.jones 2009-01-16 08:59) - PLID 32754 - added strRelationToPatient as a tracked field
			pNew->strRelationToPatient = AdoFldString(rs, "RelationToPatient", "");
			pNew->bPatientIsInsured = (pNew->strRelationToPatient == "Self");
			pNew->strInsuranceCoName = AdoFldString(rs, "InsCoName", "");
			pNew->strPayerID = AdoFldString(rs, "PayerID", "");
			// (j.jones 2008-09-09 11:10) - PLID 18695 - converted NSF Code to InsType
			pNew->eInsType = (InsuranceTypeCode)AdoFldLong(rs, "InsType", (long)itcInvalid);
			pNew->nHCFASetupID = AdoFldLong(rs, "HCFASetupGroupID", -1);
			pNew->strTaxonomyCode = AdoFldString(rs, "TaxonomyCode", "");

			// (j.jones 2010-07-06 13:05) - PLID 39499 - track patient, provider, and location names, only used for reporting errors
			pNew->strPatientName = AdoFldString(rs, "PatientName", "");
			pNew->strProviderName = AdoFldString(rs, "ProviderName", "");
			pNew->strLocationName = AdoFldString(rs, "LocationName", "");

			// (j.jones 2014-03-13 10:27) - PLID 61363 - track the insurance ICD-10 setting
			pNew->bUseICD10Codes = AdoFldBool(rs, "UseICD10Codes") ? true : false;

			// (j.jones 2014-03-13 10:33) - PLID 61363 - load only unique ICD-9 or 10 codes
			// based on the ANSI version and the ICD-10 setting
			{
				std::vector<CString> aryDiagCodes9;
				std::vector<CString> aryDiagCodes10;

				CString strDiagCode = AdoFldString(rs, "DiagCode1", "");
				strDiagCode.TrimLeft(); strDiagCode.TrimRight();
				if(!strDiagCode.IsEmpty()) {
					if(AdoFldBool(rs, "DiagCode1_ICD10", FALSE)) {
						aryDiagCodes10.push_back(strDiagCode);
					}
					else {
						aryDiagCodes9.push_back(strDiagCode);
					}
				}
				strDiagCode = AdoFldString(rs, "DiagCode2", "");
				strDiagCode.TrimLeft(); strDiagCode.TrimRight();
				if(!strDiagCode.IsEmpty()) {
					if(AdoFldBool(rs, "DiagCode2_ICD10", FALSE)) {
						aryDiagCodes10.push_back(strDiagCode);
					}
					else {
						aryDiagCodes9.push_back(strDiagCode);
					}
				}
				strDiagCode = AdoFldString(rs, "DiagCode3", "");
				strDiagCode.TrimLeft(); strDiagCode.TrimRight();
				if(!strDiagCode.IsEmpty()) {
					if(AdoFldBool(rs, "DiagCode3_ICD10", FALSE)) {
						aryDiagCodes10.push_back(strDiagCode);
					}
					else {
						aryDiagCodes9.push_back(strDiagCode);
					}
				}
				strDiagCode = AdoFldString(rs, "DiagCode4", "");
				strDiagCode.TrimLeft(); strDiagCode.TrimRight();
				if(!strDiagCode.IsEmpty()) {
					if(AdoFldBool(rs, "DiagCode4_ICD10", FALSE)) {
						aryDiagCodes10.push_back(strDiagCode);
					}
					else {
						aryDiagCodes9.push_back(strDiagCode);
					}
				}

				//ICD-10 is not supported in 4010, so if they are exporting in 4010,
				//always use only ICD-9 codes
				if(av5010 == m_avANSIVersion && pNew->bUseICD10Codes) {
					//output only ICD-10 codes
					pNew->aryDiagCodes.insert(pNew->aryDiagCodes.end(), aryDiagCodes10.begin(), aryDiagCodes10.end());
				}
				else {
					//output only ICD-9 codes
					pNew->aryDiagCodes.insert(pNew->aryDiagCodes.end(), aryDiagCodes9.begin(), aryDiagCodes9.end());
				}
			}

			m_aryEligibilityInfo.Add(pNew);
			
			rs->MoveNext();

			m_Progress.SetValue(m_Progress.GetValue() + m_ProgIncrement);
		}

		rs->Close();

		m_Progress.SetValue(100.0);

		return Success;

	} NxCatchAll("Error in CEEligibility::LoadRequestInfo");

	return Error_Other;
}

int CEEligibility::ExportEligibility()
{
	//The loops go like this:

	//Header
	//Information Source (Loop 2000A)
		//Information Receiver (Loop 2000B)
			//Subscriber (Loop 2000C)
				//Eligibility or Benefit Inquiry
			//Subscriber (Loop 2000C)
				//Dependent (Loop 2000D)
					//Eligibility or Benefit Inquiry
					//Eligibility or Benefit Inquiry
		//Information Receiver (Loop 2000B)
			//Subscriber (Loop 2000C)
				//Eligibility or Benefit Inquiry
	//Information Source (Loop 2000A)
		//Information Receiver (Loop 2000B)
			//Subscriber (Loop 2000C)
				//Eligibility or Benefit Inquiry
			//Subscriber (Loop 2000C)
				//Dependent (Loop 2000D)
				//Eligibility or Benefit Inquiry
	//Trailer

	//But since there are no records to signify the end of a loop, we just output the
	//source, receiver, subscriber, dependent, or eligibility lines to start a new loop. Easy!

	//For the file as a whole, it is surrounded with an interchange header and a functional group header,
	//and associated trailers.

	try {

		CString str;

		// (j.jones 2010-07-02 09:26) - PLID 39486 - change this label if using SOAP calls
		CString strLabel = "Exporting to Disk";
		if(m_bUseRealTimeElig) {
			// (j.jones 2010-10-21 13:12) - PLID 40914 - supported ECP
			if(m_ertcClearinghouse == ertcECP) {
				strLabel = "Exporting to ECP";
			}
			else {
				strLabel = "Exporting to TriZetto Provider Solutions";
			}
		}
		SetDlgItemText(IDC_CURR_ELIG_EVENT,strLabel);

		// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls
		// (j.jones 2010-10-21 12:34) - PLID 40914 - renamed to reflect that this is not Trizetto-only
		// (j.armen 2011-10-25 13:49) - PLID 46134 - Ebilling is in the practice path
		// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure. Also, changed to FileUtils::CreatePath() so that 
		// non-existent intermediate paths are created.
		if(!m_bUseRealTimeElig) {
			//open the file for writing
			if(!m_OutputFile.Open(m_ExportName,CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
				FileUtils::CreatePath(GetEnvironmentDirectory() ^ "Ebilling\\");
				if(!m_OutputFile.Open(m_ExportName,CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
					AfxMessageBox("The eligibility export file could not be created. Contact Nextech for assistance.");
					return Error_Other;
				}
			};
		}

		//-2, because if they have an invalid ProviderID/InsCoID it would be -1, and we compare those IDs
		//prior to stopping the export for an invalid Provider/InsCoID
		m_PrevInsCo = -2;
		m_PrevProvider = -2;		
		m_PrevInsParty = -2;
		m_PrevPatient = -2;

		//Run all the loops and output all the records.

		// (j.jones 2010-07-02 10:18) - PLID 39486 - if using Trizetto, we restart each message clean,
		// otherwise if writing to a file we write one header right now
		// (j.jones 2010-10-21 12:34) - PLID 40914 - renamed to reflect that this is not Trizetto-only anymore
		if(!m_bUseRealTimeElig) {
			RETURN_ON_FAIL(ANSI_InterchangeHeader());	//Interchange Control Header

			RETURN_ON_FAIL(ANSI_FunctionalGroupHeader());	//Functional Group Header
		}

		//set this segment count to zero AFTER the functional headers are called.
		//We only want to count the segments between the transaction header and trailer.
		m_ANSISegmentCount = 0;
		
		// (j.jones 2010-07-02 10:18) - PLID 39486 - if using Trizetto, we restart each message clean,
		// otherwise if writing to a file we write one header right now
		// (j.jones 2010-10-21 12:34) - PLID 40914 - renamed to reflect that this is not Trizetto-only anymore
		if(!m_bUseRealTimeElig) {
			RETURN_ON_FAIL(ANSI_Header());	//Header
		}

		// (j.jones 2010-07-06 12:56) - PLID 39486 - track requests that failed to send,
		// and those that came back empty/invalid
		// (j.jones 2015-11-12 11:16) - PLID 67578 - track the requests that timed out
		CArray<EligibilityInfo*, EligibilityInfo*> arySOAPFailed, arySOAPEmptyOrInvalid, arySOAPTimeout;

		// (j.jones 2010-11-05 09:30) - PLID 41341 - track the requests that received a response
		CArray<EligibilityInfo*, EligibilityInfo*> arySOAPReceived;
		
		//initialize all counters
		m_ANSIHLCount = 0;
		m_ANSICurrPayerHL = 0;
		m_ANSICurrProviderHL = 0;
		m_ANSICurrSubscriberHL = 0;
		m_ANSICurrPatientHL = 0;

		// (j.jones 2015-11-13 08:36) - PLID 67578 - added a real-time SOAP timeout, in seconds, and a timeout retry count
		long nTimeoutSeconds = 30;			//default to 30 seconds
		long nTimeoutRetryCount = 0;		//do not attempt a retry, we will only attempt once

		//loop through all claims
		for(int i=0;i<m_aryEligibilityInfo.GetSize();i++) {

			// (j.jones 2010-07-02 10:18) - PLID 39486 - if using Trizetto, we restart each message clean
			// (j.jones 2010-10-21 12:34) - PLID 40914 - renamed to reflect that this is not Trizetto-only anymore
			if(m_bUseRealTimeElig) {
				m_strCurrentRequestText = "";
				RETURN_ON_FAIL(ANSI_InterchangeHeader());	//Interchange Control Header
				RETURN_ON_FAIL(ANSI_FunctionalGroupHeader());	//Functional Group Header

				//set this segment count to zero AFTER the functional headers are called.
				//We only want to count the segments between the transaction header and trailer.
				m_ANSISegmentCount = 0;

				RETURN_ON_FAIL(ANSI_Header());	//Header

				//initialize all counters
				m_ANSIHLCount = 0;
				m_ANSICurrPayerHL = 0;
				m_ANSICurrProviderHL = 0;
				m_ANSICurrSubscriberHL = 0;
				m_ANSICurrPatientHL = 0;
			}

			// (j.jones 2010-07-02 09:26) - PLID 39486 - change this label if using SOAP calls
			if(m_bUseRealTimeElig) {
				// (j.jones 2010-10-21 13:12) - PLID 40914 - supported ECP
				if(m_ertcClearinghouse == ertcECP) {
					strLabel.Format("Exporting to ECP - Request %li of %li",i+1,m_aryEligibilityInfo.GetSize());
				}
				else {
					strLabel.Format("Exporting to TriZetto Provider Solutions - Request %li of %li",i+1,m_aryEligibilityInfo.GetSize());
				}
			}
			else {
				strLabel.Format("Exporting to Disk - Request %li of %li",i+1,m_aryEligibilityInfo.GetSize());
			}
			SetDlgItemText(IDC_CURR_ELIG_EVENT,strLabel);
			
			m_pEligibilityInfo = ((EligibilityInfo*)m_aryEligibilityInfo.GetAt(i));

			//load the HCFA info, which we pull a couple settings from
			m_HCFAInfo.LoadData(m_pEligibilityInfo->nHCFASetupID);

			//if the insurance changed, export the insco info again
			// (j.jones 2011-02-04 17:49) - PLID 42342 - always export every time if using real-time
			if(m_PrevInsCo != m_pEligibilityInfo->nInsuranceCoID || m_bUseRealTimeElig) {

				//reset the other counts
				m_PrevProvider = -2;
				m_PrevInsParty = -2;
				m_PrevPatient = -2;
			
				//for each insurance company
				RETURN_ON_FAIL(ANSI_2000A());	//Information Source Level
				RETURN_ON_FAIL(ANSI_2100A());	//Information Source Name
			}

			//if the provider changed, export the provider info again
			// (j.jones 2011-02-04 17:49) - PLID 42342 - always export every time if using real-time
			if(m_PrevProvider != m_pEligibilityInfo->nProviderID || m_bUseRealTimeElig) {

				//reset the other counts
				m_PrevInsParty = -2;
				m_PrevPatient = -2;

				//for each provider
				RETURN_ON_FAIL(ANSI_2000B());	//Information Receiver Level
				RETURN_ON_FAIL(ANSI_2100B());	//Information Receiver Name
			}

			//if the insured party has changed, export the subscriber again
			// (j.jones 2011-02-04 17:49) - PLID 42342 - always export every time if using real-time
			if(m_PrevInsParty != m_pEligibilityInfo->nInsuredPartyID || m_bUseRealTimeElig) {

				//reset the other counts
				m_PrevPatient = -2;

				//for each subscriber
				RETURN_ON_FAIL(ANSI_2000C());	//Subscriber Level
				RETURN_ON_FAIL(ANSI_2100C());	//Subscriber Name
			}

			if(m_pEligibilityInfo->bPatientIsInsured) {

				//for each eligibility request with the same subscriber
				//(only when the subscriber is the patient)
				RETURN_ON_FAIL(ANSI_2110C());	//Subscriber Eligibility Or Benefit Inquiry Information
			}
			else {

				//if the patient has changed, export the patient again
				// (j.jones 2011-02-04 17:49) - PLID 42342 - always export every time if using real-time
				if(m_PrevPatient != m_pEligibilityInfo->nPatientID || m_bUseRealTimeElig) {

					//for each patient (only when not the subscriber)
					RETURN_ON_FAIL(ANSI_2000D());	//Dependent Level
					RETURN_ON_FAIL(ANSI_2100D());	//Dependent Name
				}

				//for each eligibility request with the same patient (only when patient is not the subscriber)
				RETURN_ON_FAIL(ANSI_2110D());	//Dependent Eligibility Or Benefit Inquiry Information
			}

			//this request is done, now do all the calculations needed prior to the next request

			//set the previous provider/ins.party/etc., as this is the end of the request
			m_PrevInsCo = m_pEligibilityInfo->nInsuranceCoID;
			m_PrevProvider = m_pEligibilityInfo->nProviderID;
			m_PrevInsParty = m_pEligibilityInfo->nInsuredPartyID;
			m_PrevPatient = m_pEligibilityInfo->nPatientID;

			// (j.jones 2010-07-02 10:18) - PLID 39486 - if using Trizetto, finish the message, send it, 
			// and clear the current request
			// (j.jones 2010-10-21 13:14) - PLID 40914 - renamed to reflect that this is not Trizetto-only
			if(m_bUseRealTimeElig) {
				
				//finish the message
				RETURN_ON_FAIL(ANSI_Trailer());	//Trailer
				RETURN_ON_FAIL(ANSI_FunctionalGroupTrailer()); //Functional Group Trailer
				RETURN_ON_FAIL(ANSI_InterchangeTrailer()); //Interchange Control Trailer

				// (j.jones 2015-11-13 09:10) - PLID 67578 - add the request text to the eligibility object
				m_pEligibilityInfo->strRealTime270Request = m_strCurrentRequestText;
				m_strCurrentRequestText = "";

				// (j.jones 2015-11-13 08:45) - PLID 67578 - moved the send logic to its own function
				if(!SendRealTimeEligibility(nTimeoutSeconds, nTimeoutRetryCount, arySOAPReceived, arySOAPFailed, arySOAPEmptyOrInvalid, arySOAPTimeout)) {
					//the user should already have been told why this failed
					return Error_Other;
				}
			}

			//JJ - because of fun rounding and division and lovely math all around, it is possible for
			//us to increment too high on the last claim (ex. 9 claims would increase it to 200.0001).
			//If you aren't sending to Envoy, this will be greater than the max and therefore cause
			//an error. This code is a safeguard against going past 200. And since we will always go up to
			//200 even if we are sending claims, we will still adjust the code here regardless.
			if(m_Progress.GetValue()+m_ProgIncrement > 200.0)
				m_Progress.SetValue(200.0);
			else
				m_Progress.SetValue(m_Progress.GetValue() + m_ProgIncrement);

		}	//end looping through requests

		// (j.jones 2015-11-13 09:03) - PLID 67578 - if any requests failed due to timeouts, 
		// offer to try those again
		if (m_bUseRealTimeElig && arySOAPTimeout.GetSize() > 0) {

			bool bRetry = false;
			long nCountRetries = 0;

			// (j.jones 2015-11-13 09:03) - PLID 67578 - if we're sending a batch of requests (count > 1)
			// and any requests failed due to timeouts, always try again one more time without prompting,
			// otherwise ask the user if they wish to try again
			while (arySOAPTimeout.GetSize() > 0 &&
				((nCountRetries == 0 && m_aryEligibilityInfo.GetSize() > 1)
					|| IDYES == MessageBox(GenerateTimeoutWarning(arySOAPTimeout, true), "Practice", MB_ICONQUESTION | MB_YESNO))) {

				nCountRetries++;

				CArray<EligibilityInfo*, EligibilityInfo*> aryRetry;
				aryRetry.Append(arySOAPTimeout);
				arySOAPTimeout.RemoveAll();

				m_Progress.SetMin(0.0);
				m_Progress.SetMax(100.0);
				m_Progress.SetValue(0.0);
				m_ProgIncrement = (float)100.0 / (float)aryRetry.GetSize();

				//For the retry we're still using 30 seconds and 1 attempt

				nTimeoutSeconds = 30;		//30 seconds
				nTimeoutRetryCount = 0;		//no retries

				for (int i = 0; i < aryRetry.GetSize(); i++) {

					m_pEligibilityInfo = ((EligibilityInfo*)aryRetry.GetAt(i));

					strLabel.Format("Waiting on payer response - Request %li of %li", i + 1, aryRetry.GetSize());
					SetDlgItemText(IDC_CURR_ELIG_EVENT, strLabel);

					if (m_Progress.GetValue() + m_ProgIncrement > 100.0)
						m_Progress.SetValue(100.0);
					else
						m_Progress.SetValue(m_Progress.GetValue() + m_ProgIncrement);

					// (j.jones 2015-11-13 08:45) - PLID 67578 - moved the send logic to its own function
					if (!SendRealTimeEligibility(nTimeoutSeconds, nTimeoutRetryCount, arySOAPReceived, arySOAPFailed, arySOAPEmptyOrInvalid, arySOAPTimeout)) {
						//the user should already have been told why this failed
						return Error_Other;
					}
				}
			}
		}
		
		// (j.jones 2010-07-02 10:18) - PLID 39486 - if not sending to Trizetto,
		// add the trailers to the end of the output file
		// (j.jones 2010-10-21 12:34) - PLID 40914 - renamed to reflect that this is not Trizetto-only anymore
		if(!m_bUseRealTimeElig) {
			RETURN_ON_FAIL(ANSI_Trailer());	//Trailer

			RETURN_ON_FAIL(ANSI_FunctionalGroupTrailer()); //Functional Group Trailer

			RETURN_ON_FAIL(ANSI_InterchangeTrailer()); //Interchange Control Trailer
		}

		// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls
		// (j.jones 2010-10-21 12:34) - PLID 40914 - renamed to reflect that this is not Trizetto-only anymore
		if(!m_bUseRealTimeElig) {
			m_OutputFile.Close();
		}

		m_Progress.SetValue(m_Progress.GetMax());

		SetDlgItemText(IDC_CURR_ELIG_EVENT,"Updating Request Histories");
		
		// (j.jones 2007-06-27 16:55) - PLID 26480 - log the export in MailSent
		UpdateEligibilityHistory();
		
		// (j.jones 2010-07-02 09:26) - PLID 39486 - do not give a message if we're using SOAP calls
		if(!m_bUseRealTimeElig) {
			AfxMessageBox("All eligibility requests were successfully exported and are ready to send to your Ebilling Clearinghouse.");
		}
		else {

			// (j.jones 2010-07-06 12:59) - PLID 39499 - warn if any are empty
			if(arySOAPEmptyOrInvalid.GetSize() > 0) {
				CString strNames;
				//warn up to 20 names, one can assume that if 20 failed, they likely all failed
				for(int i=0;i<arySOAPEmptyOrInvalid.GetSize() && i<21;i++) {
					EligibilityInfo *pInfo = ((EligibilityInfo*)arySOAPEmptyOrInvalid.GetAt(i));
					if(pInfo) {
						if(!strNames.IsEmpty()) {
							strNames += "\n";
						}
						if(i == 20) {
							strNames += "<More...>";
						}
						else {
							CString str;
							str.Format("%s (%s)", pInfo->strPatientName, pInfo->strInsuranceCoName);
							strNames += str;
						}
					}
				}

				if(!strNames.IsEmpty()) {
					CString strWarn;
					// (j.jones 2010-10-21 13:12) - PLID 40914 - supported ECP, and tweaked this message
					// to indicate it could mean invalid data was received, not just empty
					strWarn.Format("The eligibility requests for the following patients were submitted, "
						"but no valid 271 responses were received:\n\n"
						"%s\n\n"
						"Please contact NexTech for assistance.",
						strNames);
					AfxMessageBox(strWarn);
				}
			}

			// (j.jones 2015-11-12 11:16) - PLID 67578 - report the requests that timed out			
			if (arySOAPTimeout.GetSize() > 0) {
				AfxMessageBox(GenerateTimeoutWarning(arySOAPTimeout, false));
			}

			// (j.jones 2010-07-02 13:42) - PLID 39499 - if we are using soap calls, import them now
			if(arySOAPReceived.GetSize() > 0) {
				CANSI271Parser parser;
				//this function will also import the results to SQL

				// (j.jones 2010-07-07 10:14) - PLID 39499 - aryRequestIDsUpdated will be filled with the IDs we updated
				std::vector<long> aryRequestIDsUpdated;
				//this will be filled with the responses we just imported
				std::vector<long> aryResponseIDsReturned;
				
				// (r.goldschmidt 2015-11-12 12:55) - PLID 65363 - bNotepadWasOpened will be set when we parse
				bool bNotepadWasOpened;

				//(s.dhole 09/11/2012) PLID 52414
				if(parser.ParseRealTimeResponses(arySOAPReceived, aryRequestIDsUpdated, aryResponseIDsReturned, bNotepadWasOpened) && m_bCEligibilityRequestDetailDlg == TRUE) {
					
					// (j.jones 2010-07-07 10:16) - PLID 39499 - auto-open the result dialog
					// (r.goldschmidt 2014-10-10 12:17) - PLID 62644 - make eligibility request detail dlg modeless
					GetMainFrame()->ShowEligibilityRequestDetailDlg(aryRequestIDsUpdated, aryResponseIDsReturned, true);
				}
			}
			else {
				//we would have already warned if either array has entries
				// (j.jones 2015-11-13 09:35) - PLID 67578 - added a timeout array
				if(arySOAPEmptyOrInvalid.GetSize() == 0 && arySOAPFailed.GetSize() == 0 && arySOAPTimeout.GetSize() == 0) {
					//shouldn't be possible, because how did this happen without a failure?
					// (j.jones 2010-10-21 13:12) - PLID 40914 - supported ECP
					if(m_ertcClearinghouse == ertcECP) {
						AfxMessageBox("All eligibility requests were exported to ECP, but no responses were received. Contact NexTech for assistance.");
					}
					else {
						AfxMessageBox("All eligibility requests were exported to TriZetto Provider Solutions, but no responses were received. Contact NexTech for assistance.");
					}
				}				
			}
		}

		m_Progress.SetValue(m_Progress.GetMax());

		EndDialog(IDOK);

	return Success;

	} NxCatchAll("Error in CEEligibility::ExportEligibility");

	return Error_Other;

}

// (j.jones 2015-11-13 08:45) - PLID 67578 - Sends one request and returns the response.
// Returns false if a warning prevented the send from being attempted.
// Returns true if a send was attempted. It may or may not have succeeded.
bool CEEligibility::SendRealTimeEligibility(long nTimeoutSeconds, long nTimeoutRetryCount,
	OUT CArray<EligibilityInfo*, EligibilityInfo*> &arySOAPReceived,
	OUT CArray<EligibilityInfo*, EligibilityInfo*> &arySOAPFailed,
	OUT CArray<EligibilityInfo*, EligibilityInfo*> &arySOAPEmptyOrInvalid,
	OUT CArray<EligibilityInfo*, EligibilityInfo*> &arySOAPTimeout)
{
	//moved this logic from ExportEligibility

	// (j.jones 2010-07-06 10:39) - PLID 39319 - load the login info
	CString strSiteID = "";
	CString strPassword = "";

	long nLoginType = GetRemotePropertyInt("Clearinghouse_LoginType", 0, (long)m_ertcClearinghouse, "<None>", true);
	//0 - global, 1 - indiv.
	if (nLoginType == 1) {
		//individual

		if (m_pEligibilityInfo->nProviderID != -1 && m_pEligibilityInfo->nLocationID != -1) {
			//load this provider/location's login
			_RecordsetPtr rs = CreateParamRecordset("SELECT SiteID, Password FROM ClearinghouseLoginInfoT "
				"WHERE ClearinghouseType = {CONST_INT}, ProviderID = {INT} AND LocationID = {INT}", 
				(long)m_ertcClearinghouse, m_pEligibilityInfo->nProviderID, m_pEligibilityInfo->nLocationID);
			if (!rs->eof) {
				strSiteID = AdoFldString(rs, "SiteID");
				_variant_t varPassword = rs->Fields->Item["Password"]->Value;
				if (varPassword.vt != VT_EMPTY && varPassword.vt != VT_NULL) {
					strPassword = DecryptStringFromVariant(varPassword);
				}
				else {
					strPassword = "";
				}
			}
			rs->Close();
		}

		if (strSiteID.IsEmpty() || strPassword.IsEmpty()) {
			CString strWarn;
			strWarn.Format("The eligibility export failed because you have not entered in your Site ID & Password in the Real-Time Settings screen "
				"for the provider %s and the location %s.", m_pEligibilityInfo->strProviderName, m_pEligibilityInfo->strLocationName);
			AfxMessageBox(strWarn);
			return false;
		}
	}
	else {
		//global

		//our NexTech test values are V093 for Site ID, and a password of ix98m21h (not sure if that will ever expire)
		strSiteID = GetRemotePropertyText("GlobalClearinghouseLoginName", "", (long)m_ertcClearinghouse, "<None>", true);

		//the password is encrypted in our data
		_variant_t varPassword = GetRemotePropertyImage("GlobalClearinghousePassword", (long)m_ertcClearinghouse, "<None>", true);
		if (varPassword.vt != VT_EMPTY && varPassword.vt != VT_NULL) {
			strPassword = DecryptStringFromVariant(varPassword);
		}
		else {
			strPassword = "";
		}

		if (strSiteID.IsEmpty() || strPassword.IsEmpty()) {
			AfxMessageBox("The eligibility export failed because you have not entered in your Site ID & Password in the Real-Time Settings screen.");
			return false;
		}
	}

	//send this request to the appropriate clearinghouse
	if (m_ertcClearinghouse == ertcECP) {

		CString str271Response;
		ECPEligibility::EECPInquiryReturnValues eirvRetVal = ECPEligibility::PerformEligibilityInquiry(strSiteID, strPassword,
			nTimeoutSeconds, nTimeoutRetryCount,
			m_pEligibilityInfo->strRealTime270Request, str271Response);

		if (eirvRetVal != ECPEligibility::eirvSuccess) {
			//something failed, track and report on it later
			if (eirvRetVal == ECPEligibility::eirvSendFail) {
				arySOAPFailed.Add(m_pEligibilityInfo);
			}
			else if (eirvRetVal == ECPEligibility::eirvResponseEmptyOrInvalid) {
				//could be invalid, not empty, but track all as one list
				arySOAPEmptyOrInvalid.Add(m_pEligibilityInfo);
			}
			// (j.jones 2015-11-12 11:14) - PLID 67578 - returned if we timed out after several attempts
			else if (eirvRetVal == ECPEligibility::eirvTimeoutExceeded) {
				arySOAPTimeout.Add(m_pEligibilityInfo);
			}
			else if (eirvRetVal == ECPEligibility::eirvCatastrophicFail) {
				//should only be sent if we caught and handled an exception,
				//we must abort the process
				return false;
			}
		}
		else {
			//it worked, track our response text
			// (j.jones 2010-11-05 09:28) - PLID 41341 - this is now tracked in the memory object
			m_pEligibilityInfo->strRealTime271Response = str271Response;
			//track this successful response
			arySOAPReceived.Add(m_pEligibilityInfo);
		}
	}
	else {
		//TriZetto

		CString str271Response;

		TrizettoEligibility::ETrizettoInquiryReturnValues tirvRetVal = TrizettoEligibility::PerformEligibilityInquiry(strSiteID, strPassword,
			m_pEligibilityInfo->strPayerID, nTimeoutSeconds, nTimeoutRetryCount,
			m_pEligibilityInfo->strRealTime270Request, str271Response);

		if (tirvRetVal != TrizettoEligibility::tirvSuccess) {
			//something failed, track and report on it later
			if (tirvRetVal == TrizettoEligibility::tirvSendFail) {
				arySOAPFailed.Add(m_pEligibilityInfo);
			}
			else if (tirvRetVal == TrizettoEligibility::tirvResponseEmpty) {
				arySOAPEmptyOrInvalid.Add(m_pEligibilityInfo);
			}
			// (j.jones 2015-11-12 11:14) - PLID 67578 - returned if we timed out after several attempts
			else if (tirvRetVal == TrizettoEligibility::tirvTimeoutExceeded) {
				arySOAPTimeout.Add(m_pEligibilityInfo);
			}
			else if (tirvRetVal == TrizettoEligibility::tirvCatastrophicFail) {
				//should only be sent if we caught and handled an exception, like a SOAP failure,
				//we must abort the process
				return false;
			}
		}
		else {
			//it worked, track our response text
			// (j.jones 2010-11-05 09:28) - PLID 41341 - this is now tracked in the memory object
			m_pEligibilityInfo->strRealTime271Response = str271Response;
			//track this successful response
			arySOAPReceived.Add(m_pEligibilityInfo);
		}
	}

	return true;
}

void CEEligibility::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == EELIGIBILITY_TIMER_EVENT)
	{
		KillTimer(EELIGIBILITY_TIMER_EVENT);

		ExportData();
	}
	
	CDialog::OnTimer(nIDEvent);
}

void CEEligibility::ResolveErrors(int Error)
{
	if(Error==Success) {
		//do nothing
	}
	else if(Error==Error_Missing_Info) {
		EndDialog(ID_CANCEL);
		MessageBox("Errors were found in your eligibility request that will cause a rejection,\n"
			"as a result, the export has been aborted. Please correct the errors before exporting again.","Electronic Eligibility",MB_OK|MB_ICONEXCLAMATION);
	}
	else if(Error==Error_Other) {
		EndDialog(ID_CANCEL);
		MessageBox("An unexpected error has occurred and the export has been aborted. \n"
			"Please contact NexTech for assistance.","Electronic Eligibility",MB_OK|MB_ICONEXCLAMATION);
	}
	else if(Error==Cancel_Silent) {
		//do nothing, it should already be handled appropriately
	}
}

// (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
CString CEEligibility::NormalizeName(CString str)
{
	auto shouldIgnoreChar = [](char c) {
		// all alphanumerics, though i dont know why someone might have a number in their name,
		// turns out that actually does happen -- Jennifer 8. Lee
		if (::isalnum((unsigned char)c)) {
			return false;
		}
		switch (c) {
		case '\'': // apostrophes are fine
		case '-':  // so are hyphens
		case ' ': // spaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaace
			return false;
		}
		return true;
	};

	// quick exit if nothing to escape
	if (0 == std::count_if((const char*)str, (const char*)str + str.GetLength(), shouldIgnoreChar)) {
		return str;
	}

	{
		CString::CStrBuf buf(str);

		char* szBegin = buf;
		char* szEnd = szBegin + str.GetLength();

		// std::remove will move anything that matches shouldIgnoreChar to the end of the string
		// and return a pointer to the 'new' end of the string, which we can use to SetLength.
		const char* szJunk = std::remove_if(szBegin, szEnd, shouldIgnoreChar);

		buf.SetLength(szJunk - szBegin);
	}

	return str;
}

CString CEEligibility::StripNonNum(CString str)
{
	try {
		CString strbuf = "";
		for(int i=0;i<str.GetLength();i++) {
			if(str.GetAt(i)>='0' && str.GetAt(i)<='9')
				strbuf += str.GetAt(i);
		}
		return strbuf;
	} NxCatchAll("Error in StripNonNum: Data = " + str);
	return str;
}

CString CEEligibility::StripPunctuation(CString str, BOOL bReplaceWithSpaces /*= TRUE*/)
{
	try {
		CString strbuf = "";
		for(int i=0;i<str.GetLength();i++) {
			// (j.jones 2007-10-05 14:26) - PLID 25867 - fixed so we won't try to search A to z
			if((str.GetAt(i)>='0' && str.GetAt(i)<='9') || (str.GetAt(i)>='A' && str.GetAt(i)<='Z') || (str.GetAt(i)>='a' && str.GetAt(i)<='z') || (bReplaceWithSpaces && str.GetAt(i)==' '))
				strbuf += str.GetAt(i);
			else {
				if(bReplaceWithSpaces)
					strbuf += " ";
			}
		}
		return strbuf;
	} NxCatchAll("Error in StripPunctuation: Data = " + str);
	return str;
}

CString CEEligibility::StripANSIPunctuation(CString str)
{
	try {

		CString strbuf = "";
		for(int i=0;i<str.GetLength();i++) {
			//if we are taking out all punctuation, then take out all non numeric, non alpha chars
			//but we ALWAYS take out asterisks and tildes. ALWAYS.
			// (j.jones 2007-10-05 14:26) - PLID 25867 - fixed so we won't try to search A to z
			if((m_bStripANSIPunctuation && !((str.GetAt(i)>='0' && str.GetAt(i)<='9') || (str.GetAt(i)>='A' && str.GetAt(i)<='Z') || (str.GetAt(i)>='a' && str.GetAt(i)<='z') || str.GetAt(i)==':' || str.GetAt(i)=='.' || str.GetAt(i)==' '))
				|| str.GetAt(i) == '*' || str.GetAt(i) == '~'
				// (j.jones 2010-10-15 11:30) - PLID 32848 - 5010 differences are that ^ and ` are in the always-remove list
				|| (m_avANSIVersion == av5010 && (str.GetAt(i) == '^' || str.GetAt(i) == '`')))

				strbuf += " ";
			else
				strbuf += str.GetAt(i);
		}
		return strbuf;

	} NxCatchAll("Error in StripANSIPunctuation: Data = " + str);
	return str;
}

CString CEEligibility::ParseANSIField(CString data, long Min, long Max, BOOL bForceFill /* = FALSE */, char Justify /* = 'L'*/, char FillChar /* = ' '*/)
{
	CString strData = "";
	long strLen;

	try {

		strData = data;

		strData = StripANSIPunctuation(strData);

		strData.TrimRight(" ");

		strLen = strData.GetLength();
		if(strLen>Max)
			//if the data is bigger than the field, truncate
			strData = strData.Left(Max);
		else if(strLen<Min && (strData != "" || bForceFill)) {
			//otherwise (and this is more likely), fill with the FillChar
			
			//JJ - for ANSI, only fill if the strData is not deliberately blank

			for(int i=strLen;i<Min;i++) {
				if(Justify=='R') {
					//right justify
					strData = FillChar + strData;
				}
				else {
					//by default, left justify
					strData += FillChar;					
				}
			}
		}

		strData = "*" + strData;

		if(m_bCapitalizeEbilling)
			strData.MakeUpper();

		return strData;

	} NxCatchAll("Error in ParseANSIField: Data = " + data);

	//if it fails, just return a *
	strData = "*";

	return strData;
}

void CEEligibility::EndANSISegment(CString OutputString) {

	OutputString.TrimRight("*");

	//this is so we don't output segments with no data, such as "N3~"
	if(OutputString.Find("*") == -1)
		return;

	OutputString += "~";
	
	// (j.jones 2009-09-23 11:51) - I have no idea why, but if you exported a lot of
	// requests, Visual Studio would delay the tracing until after the export finished,
	// and would take forever to finish, locking up Practice while you waited. I don't
	// really need the trace anymore, so I am removing it.
	//TRACE0(OutputString);

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// instead we track the current request text
	// (j.jones 2010-10-21 12:34) - PLID 40914 - renamed to reflect that this is not Trizetto-only anymore
	if(m_bUseRealTimeElig) {
		m_strCurrentRequestText += OutputString;
	}
	else {
		m_OutputFile.Write(OutputString,OutputString.GetLength());
	}

	//increment the count of segments
	m_ANSISegmentCount++;

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

}

//ANSI X12 4010

//The following functions represent individual 'loops' of a generated ANSI eligibility file.
//Each function will generate its own line and write it to the output file.

int CEEligibility::ANSI_Header() {

	//Header

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\nHEADER\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							HEADER

//36		010		ST		Transaction Set Header					M		1

		OutputString = "ST";

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//ST01	143			Transaction Set Identifier Code			M	ID	3/3
		
		//static "270"
		OutputString += ParseANSIField("270",3,3);

		//ST02	329			Transaction Set Control Number			M	AN	4/9
		
		//m_strBatchNumber
		OutputString += ParseANSIField(m_strBatchNumber,4,9,TRUE,'R','0');

		//ST03					Implementation Convention Reference	M	AN	1/35
		// (c.haag 2010-10-14 11:15) - PLID 40352 - ST03 is for ANSI 5010 only. New Element.
		if (av5010 == m_avANSIVersion) {
			OutputString += ParseANSIField("005010X279A1",1,35);
		}

		EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

//38		020		BHT		Beginning of Hierarchical Transaction	M		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "BHT";

		//BHT01	1005		Hierarchical Structure Code				M	ID	4/4
		
		//static "0022"
		OutputString += ParseANSIField("0022",4,4);

		//BHT02	353			Transaction Set Purpose Code			M	ID	2/2
		
		//13 is a request, 01 is to cancel a request, 36 is "Authority to Deduct (Reply)"
		//until we have a reason to use anything else, use "13"
		OutputString += ParseANSIField("13",2,2);

		//BHT03	127			Reference Identification				O	AN	1/30

		//batch number
		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version. Increase.
		if (av5010 == m_avANSIVersion) {
			OutputString += ParseANSIField(m_strBatchNumber,1,50,TRUE,'R','0');
		} else {
			OutputString += ParseANSIField(m_strBatchNumber,1,30,TRUE,'R','0');
		}

		//BHT04	373			Date									O	DT	8/8

		//current date YYYY/MM/DD
		COleDateTime dt = COleDateTime::GetCurrentTime();
		str = dt.Format("%Y%m%d");
		OutputString += ParseANSIField(str,8,8);
	
		//BHT05	337			Time									O	TM	4/8

		//current time	
		str = dt.Format("%H%M%S");
		OutputString += ParseANSIField(str,4,8);

		//BHT06	640			Transaction Type Code					O	ID	2/2

		//"Certain Medicaid programs support additional functionality for
		//Spend Down or Medical Services Reservation. Use this code when
		//necessary to further specify the type of transaction to a Medicaid
		//program that supports this functionality."

		//RT - Spend Down
		//RU - Medical Services Reservation

		//This seems to be optional, for Medicaid only. Wait until
		//we have a requirement for this before implementing.

		//OutputString += ParseANSIField("",2,2);

		EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in CEEligibility::ANSI_Header");

	return Error_Other;
}

int CEEligibility::ANSI_2000A() {

	//Information Source Level

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\n2000A\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

	try {

		//increment the HL count
		m_ANSIHLCount++;

		//increment the count for the current payer
		m_ANSICurrPayerHL = m_ANSIHLCount;

		CString OutputString,str;
		_variant_t var;

//							INFORMATION SOURCE LEVEL

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP 2000A - INFORMATION SOURCE LEVEL							>1

//41		010		HL		Hierarchical Level						M		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HL";

		//HL01	628			Hierarchical ID Number					M	AN	1/12

		//m_ANSIHLCount

		str.Format("%li",m_ANSIHLCount);
		OutputString += ParseANSIField(str,1,12);

		//HL02 NOT USED
		OutputString += "*";

		//HL03	735			Hierarchical Level Code					M	ID	1/2

		//static "20"
		OutputString += ParseANSIField("20",1,2);

		//HL04	736			Hierarchical Child Code					O	ID	1/1

		//static "1"
		OutputString += ParseANSIField("1",1,1);

		EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in CEEligibility::ANSI_2000A");

	return Error_Other;
}

int CEEligibility::ANSI_2100A() {

	//Information Source Name

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\n2100A\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

//							INFORMATION SOURCE NAME

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP 2100A - INFORMATION SOURCE NAME							1

//44		030		NM1		Individual or Organizational Name		M		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M	ID	2/3

		//2B - Third-Party Administrator
		//36 - Employer
		//GP - Gateway Provider
		//P5 - Plan Sponsor
		//PR - Payer

		//use "PR"
		OutputString += ParseANSIField("PR",2,3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

		//1 - Person
		//2 - Non-Person Entity

		//use 2
		OutputString += ParseANSIField("2",1,1);

		//NM103	1035		Name Last or Organization Name			O	AN	1/35

		//InsuranceCoT.Name
		str = m_pEligibilityInfo->strInsuranceCoName;

		str = StripPunctuation(str);
		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version. Increase Usage Change. Required
		if (av5010 == m_avANSIVersion) {
			OutputString += ParseANSIField(str,1,60);
		} else {
			OutputString += ParseANSIField(str,1,35);
		}

		//NM104	1036		Name First								O	AN	1/25

		//blank if NM102 = 2
		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version. Increase.
		if (av5010 == m_avANSIVersion) {
			OutputString += ParseANSIField("",1,35);
		} else {
			OutputString += ParseANSIField("",1,25);
		}

		//NM105	1037		Name Middle								O	AN	1/25

		//blank if NM102 = 2
		OutputString += ParseANSIField("",1,25);

		//NM106	NOT USED
		OutputString += "*";

		//NM107	1039		Name Suffix								O	AN	1/10

		//blank if NM102 = 2
		OutputString += ParseANSIField("",1,10);

		//NM108	66			Identification Code	Qualifier			X	ID	1/2

		//load the payer ID
		CString strID = m_pEligibilityInfo->strPayerID;

		//static "PI" for Payor Identification
				
		if(strID != "")
			OutputString += ParseANSIField("PI",1,2);
		else
			OutputString += "*";

		//NM109	67			Identification Code						X	AN	2/80

		//InsuranceCoT.EligPayerID

		if(strID != "")		
			OutputString += ParseANSIField(strID,2,80);
		else
			OutputString += "*";

		//NM110 NOT USED
		OutputString += "*";

		//NM111	NOT USED
		OutputString += "*";

		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version.
		//NM112			Name Last or Organization Name			AN	1/60
		//NOT USED
		if (av5010 == m_avANSIVersion) {
			OutputString += "*";
		}

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in CEEligibility::ANSI_2100A");

	return Error_Other;
}

int CEEligibility::ANSI_2000B() {

	//Information Receiver Level

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\n2000B\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

	try {

		//increment the HL count
		m_ANSIHLCount++;

		//increment the count for the current provider
		m_ANSICurrProviderHL = m_ANSIHLCount;

		CString OutputString,str;
		_variant_t var;

//							INFORMATION RECEIVER LEVEL

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP 2000B - INFORMATION RECEIVER LEVEL							>1

//47		010		HL		Hierarchical Level						M		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HL";

		//HL01	628			Hierarchical ID Number					M	AN	1/12

		//m_ANSIHLCount

		str.Format("%li",m_ANSIHLCount);
		OutputString += ParseANSIField(str,1,12);

		//HL02	734			Hierarchical Parent ID Number			O	AN	1/12

		//m_ANSICurrPayerHL
		str.Format("%li",m_ANSICurrPayerHL);
		OutputString += ParseANSIField(str,1,12);

		//HL03	735			Hierarchical Level Code					M	ID	1/2

		//static "21"
		OutputString += ParseANSIField("21",1,2);

		//HL04	736			Hierarchical Child Code					O	ID	1/1

		//static "1"
		OutputString += ParseANSIField("1",1,1);

		EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in CEEligibility::ANSI_2000B");

	return Error_Other;
}

int CEEligibility::ANSI_2100B() {

	//Information Receiver Name

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\n2100B\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		CString strSql;
		
		//determine if the address will be the provider's contact address,
		//or the location's address

		// (j.jones 2009-09-23 11:43) - PLID 35637 - I altered these queries to always return
		// both the location NPI and provider NPI, because the setting that determines which
		// ID to use is independent of the DocAddress setting.

		if(m_HCFAInfo.DocAddress == 1) {
			//use the location address
			strSql.Format("SELECT PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Title, ProvidersT.TaxonomyCode, "
				"LocationsT.Address1, LocationsT.Address2, LocationsT.City, LocationsT.State, LocationsT.Zip, "
				"LocationsT.Phone, LocationsT.Fax, "
				"ProvidersT.NPI AS ProviderNPI, LocationsT.NPI AS LocationNPI "
				"FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
				"CROSS JOIN LocationsT "
				"WHERE ProvidersT.PersonID = %li AND LocationsT.ID = %li",m_pEligibilityInfo->nProviderID, m_pEligibilityInfo->nLocationID);
		}
		else {
			//use the contact address
			strSql.Format("SELECT PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Title, ProvidersT.TaxonomyCode, "
				"PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, "
				"PersonT.WorkPhone AS Phone, PersonT.Fax, "
				"ProvidersT.NPI AS ProviderNPI, LocationsT.NPI AS LocationNPI "
				"FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
				"CROSS JOIN LocationsT "
				"WHERE ProvidersT.PersonID = %li AND LocationsT.ID = %li",m_pEligibilityInfo->nProviderID, m_pEligibilityInfo->nLocationID);
		}

		_RecordsetPtr rs = CreateRecordsetStd(strSql);
		if(rs->eof) {
			//if the recordset is empty, there is no provider. So halt everything!!!
			rs->Close();
			rs = CreateRecordset("SELECT [First] + ' ' + [Last] AS Name FROM PersonT WHERE ID = %li",m_pEligibilityInfo->nPatientID);
			if(!rs->eof){
				//give a HELPFUL error
				str.Format("Could not open provider information for patient '%s'.",AdoFldString(rs, "Name",""));
			}
			else
				//serious problems if you get here
				str = "Could not open provider information.";
			
			rs->Close();
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		// (j.jones 2010-03-01 17:49) - PLID 37585 - added override for 2100B NM109,
		// and moved the loading of all overrides to the top of the function
		BOOL bUse2100B_REF_1 = FALSE;
		CString strANSI_2100B_REF_Qual_1 = "";
		CString strANSI_2100B_REF_Value_1 = "";
		BOOL bUse2100B_REF_2 = FALSE;
		CString strANSI_2100B_REF_Qual_2 = "";
		CString strANSI_2100B_REF_Value_2 = "";
		BOOL bUse2100B_NM109 = FALSE;
		CString strANSI_2100B_NM109_Qual = "";
		CString strANSI_2100B_NM109_Value = "";

		_RecordsetPtr rsOver = CreateParamRecordset("SELECT Use2100B_REF_1, ANSI_2100B_REF_Qual_1, ANSI_2100B_REF_Value_1, "
			"Use2100B_REF_2, ANSI_2100B_REF_Qual_2, ANSI_2100B_REF_Value_2, "
			"Use2100B_NM109, ANSI_2100B_NM109_Qual, ANSI_2100B_NM109_Value "
			"FROM EligibilitySetupT "
			"WHERE HCFASetupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}", m_pEligibilityInfo->nHCFASetupID, m_pEligibilityInfo->nProviderID, m_pEligibilityInfo->nLocationID);

		if(!rsOver->eof) {

			bUse2100B_REF_1 = AdoFldBool(rsOver, "Use2100B_REF_1", FALSE);
			strANSI_2100B_REF_Qual_1 = AdoFldString(rsOver, "ANSI_2100B_REF_Qual_1", "");
			strANSI_2100B_REF_Value_1 = AdoFldString(rsOver, "ANSI_2100B_REF_Value_1", "");
			bUse2100B_REF_2 = AdoFldBool(rsOver, "Use2100B_REF_2", FALSE);
			strANSI_2100B_REF_Qual_2 = AdoFldString(rsOver, "ANSI_2100B_REF_Qual_2", "");
			strANSI_2100B_REF_Value_2 = AdoFldString(rsOver, "ANSI_2100B_REF_Value_2", "");

			// (j.jones 2010-03-01 17:28) - PLID 37585 - added override for 2100B NM109
			bUse2100B_NM109 = AdoFldBool(rsOver, "Use2100B_NM109", FALSE);
			strANSI_2100B_NM109_Qual = AdoFldString(rsOver, "ANSI_2100B_NM109_Qual", "");
			strANSI_2100B_NM109_Value = AdoFldString(rsOver, "ANSI_2100B_NM109_Value", "");
		}
		rsOver->Close();

//							INFORMATION RECEIVER NAME

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP 2100B - INFORMATION RECEIVER NAME							1

//50		030		NM1		Individual or Organizational Name		M		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M	ID	2/3

		//1P - Provider
		//2B - Third-Party Administrator
		//36 - Employer
		//80 - Hospital
		//FA - Facility
		//GP - Gateway Provider
		//P5 - Plan Sponsor
		//PR - Payer

		//use "1P"
		OutputString += ParseANSIField("1P",2,3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

		//1 - Person
		//2 - Non-Person Entity

		//use 1
		OutputString += ParseANSIField("1",1,1);

		//NM103	1035		Name Last or Organization Name			O	AN	1/35

		//PersonT.Last
		str = AdoFldString(rs, "Last","");

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,35);

		//NM104	1036		Name First								O	AN	1/25

		//blank if NM102 = 2

		//PersonT.First
		str = AdoFldString(rs, "First","");

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation		
		OutputString += ParseANSIField(str,1,25);

		//NM105	1037		Name Middle								O	AN	1/25

		//blank if NM102 = 2

		//PersonT.Middle
		str = AdoFldString(rs, "Middle","");

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation		
		OutputString += ParseANSIField(str,1,25);

		//NM106	NOT USED
		OutputString += "*";

		//NM107	1039		Name Suffix								O	AN	1/10

		//blank if NM102 = 2

		//PersonT.Title
		// (j.jones 2007-08-03 12:08) - PLID 26934 - changed from Suffix to Title
		str = AdoFldString(rs, "Title","");

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation		
		OutputString += ParseANSIField(str,1,10);

		//NM108	66			Identification Code	Qualifier			X	ID	1/2

		//load the provider NPI number
		CString strID = "";
		CString strQual = "XX";

		// (j.jones 2009-09-23 11:43) - PLID 35637 - use the location NPI
		// if the HCFA group says to do so
		// (j.jones 2011-06-15 12:51) - PLID 42181 - added EligNPIUsage, so eligibility now
		// has its own unique setting for this
		if(m_HCFAInfo.EligNPIUsage == 1) {
			//location NPI
			strID = AdoFldString(rs, "LocationNPI","");
		}
		else {
			//provider NPI
			strID = AdoFldString(rs, "ProviderNPI","");
		}

		// (j.jones 2010-03-01 17:48) - PLID 37585 - added override for 2100B NM109
		if(bUse2100B_NM109) {
			strANSI_2100B_NM109_Qual.TrimLeft();
			strANSI_2100B_NM109_Qual.TrimRight();
			if(!strANSI_2100B_NM109_Qual.IsEmpty()) {
				strQual = strANSI_2100B_NM109_Qual;
			}
			strANSI_2100B_NM109_Value.TrimLeft();
			strANSI_2100B_NM109_Value.TrimRight();
			if(!strANSI_2100B_NM109_Value.IsEmpty()) {
				strID = strANSI_2100B_NM109_Value;
			}
		}


		//24 - Employer’s Identification Number (Use this code only when the 270/271 transaction sets are used by an employer inquiring about eligibility and benefits of their employees.)
		//34 - Social Security Number (The social security number may not be used for any Federally administered programs such as Medicare.)
		//FI - Federal Taxpayer’s Identification Number
		//PI - Payor Identification (Use this code only when the 270/271 transaction sets are used between two payers.)
		//PP - Pharmacy Processor Number
		//SV - Service Provider Number (Use this code for the identification number assigned by the information source to be used by the information receiver in health care transactions.)
		//XX - Health Care Financing Administration National Provider Identifier (Required value if the National Provider ID is mandated for use. Otherwise, one of the other listed codes may be used.)

		//the official specs say that SV was recommended until NPI is in use,
		//which it is now, in which case we use XX
				
		if(strID != "")
			OutputString += ParseANSIField(strQual,1,2);
		else
			OutputString += "*";

		//NM109	67			Identification Code						X	AN	2/80

		//ProvidersT.NPI

		if(strID != "")		
			OutputString += ParseANSIField(strID,2,80);
		else
			OutputString += "*";

		//NM110 NOT USED
		OutputString += "*";

		//NM111	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//54		040		REF		Information Receiver Additional Information		O	9

		//This is an optional segment.

		// (j.jones 2008-06-23 12:16) - PLID 30434 - this is now configurable in the E-Eligibility Setup

		//send up to two REF segments

		CString strLastQual = "";

		if(bUse2100B_REF_1) {

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "REF";

			//REF01	128			Reference Ident Qual					M	ID	2/3

			CString strQual, strNum;

			//0B - State License Number (The state assigning the license number must be identified in REF03.)
			//1C - Medicare Provider Number (This code is only to be used when the information source is not Medicare. If the information source is Medicare, the Medicare provider number is to be supplied in NM109 using Identification Code Qualifier of 'SV' in NM108.)
			//1D - Medicaid Provider Number (This code is only to be used when the information source is not Medicaid. If the information source is Medicaid, the Medicaid provider number is to be supplied in NM109 using Identification Code Qualifier of 'SV' in NM108.)
			//1J - Facility ID Number
			//4A - Personal Identification Number (PIN)
			//CT - Contract Number (This code is only to be used once the HCFA National Provider Identifier has been mandated for use, and must be sent if required in the contract between the provider identified in Loop 2000B and the Information Source identified in Loop 2000A.)
			//EL - Electronic device pin number
			//EO - Submitter Identification Number
			//HPI - Health Care Financing Administration National Provider Identifier (The Health Care Financing Administration National Provider Identifier may be used in this segment prior to being mandated for use.)		
			//JD - User Identification
			//N5 - Provider Plan Network Identification Number
			//N7 - Facility Network Identification Number
			//Q4 - Prior Identifier Number
			//SY - Social Security Number (The social security number may not be used for any Federally administered programs such as Medicare.)
			//TJ - Federal Taxpayer’s Identification Number

			//Presumably we should use HPI, but Edifecs HIPAADesk testing says
			//HPI should not be used if NM108 is XX. Simply put, don't send
			//the NPI in this segment.
			
			// (j.jones 2008-06-23 12:19) - PLID 30434 - this all comes from the E-Eligibility Setup now
			strQual = strANSI_2100B_REF_Qual_1;
			strNum = strANSI_2100B_REF_Value_1;
			strLastQual = strQual;

			OutputString += ParseANSIField(strQual, 2, 3);

			//REF02	127			Reference Ident							X	AN	1/30
			
			OutputString += ParseANSIField(strNum, 1, 30);

			//REF03 352			Description								X	AN	1/80

			//only used if we need to output a state code, when REF01 is 0B

			OutputString += "*";

			//REF04	NOT USED
			OutputString += "*";

			//if there is no number, don't output
			if(!strQual.IsEmpty() && !strNum.IsEmpty()) {
				EndANSISegment(OutputString);
			}
		}
		
		if(bUse2100B_REF_2) {

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "REF";

			//REF01	128			Reference Ident Qual					M	ID	2/3

			CString strQual, strNum;

			//0B - State License Number (The state assigning the license number must be identified in REF03.)
			//1C - Medicare Provider Number (This code is only to be used when the information source is not Medicare. If the information source is Medicare, the Medicare provider number is to be supplied in NM109 using Identification Code Qualifier of 'SV' in NM108.)
			//1D - Medicaid Provider Number (This code is only to be used when the information source is not Medicaid. If the information source is Medicaid, the Medicaid provider number is to be supplied in NM109 using Identification Code Qualifier of 'SV' in NM108.)
			//1J - Facility ID Number
			//4A - Personal Identification Number (PIN)
			//CT - Contract Number (This code is only to be used once the HCFA National Provider Identifier has been mandated for use, and must be sent if required in the contract between the provider identified in Loop 2000B and the Information Source identified in Loop 2000A.)
			//EL - Electronic device pin number
			//EO - Submitter Identification Number
			//HPI - Health Care Financing Administration National Provider Identifier (The Health Care Financing Administration National Provider Identifier may be used in this segment prior to being mandated for use.)		
			//JD - User Identification
			//N5 - Provider Plan Network Identification Number
			//N7 - Facility Network Identification Number
			//Q4 - Prior Identifier Number
			//SY - Social Security Number (The social security number may not be used for any Federally administered programs such as Medicare.)
			//TJ - Federal Taxpayer’s Identification Number

			//Presumably we should use HPI, but Edifecs HIPAADesk testing says
			//HPI should not be used if NM108 is XX. Simply put, don't send
			//the NPI in this segment.
			
			// (j.jones 2008-06-23 12:19) - PLID 30434 - this all comes from the E-Eligibility Setup now
			strQual = strANSI_2100B_REF_Qual_2;
			strNum = strANSI_2100B_REF_Value_2;

			OutputString += ParseANSIField(strQual, 2, 3);

			//REF02	127			Reference Ident							X	AN	1/30
			
			OutputString += ParseANSIField(strNum, 1, 30);

			//REF03 352			Description								X	AN	1/80

			//only used if we need to output a state code, when REF01 is 0B

			OutputString += "*";

			//REF04	NOT USED
			OutputString += "*";

			//if there is no number, don't output
			if(!strQual.IsEmpty() && !strNum.IsEmpty() && strQual.CompareNoCase(strLastQual) != 0) {
				EndANSISegment(OutputString);
			}
		}		

///////////////////////////////////////////////////////////////////////////////

//57		060		N3		Information Receiver Address			O		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//N301	166			Address Information						M	AN	1/55

		//Address1 - previously we decided whether it should be provider
		//or location, the recordset is pulling the desired value
		str = AdoFldString(rs, "Address1","");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		//N302	166			Address Information						O	AN	1/55

		//Address2 - previously we decided whether it should be provider
		//or location, the recordset is pulling the desired value
		str = AdoFldString(rs, "Address2","");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//58		070		N4		Information Receiver City/State/Zip		O		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O	AN	2/30

		//City - previously we decided whether it should be provider
		//or location, the recordset is pulling the desired value
		str = AdoFldString(rs, "City","");

		OutputString += ParseANSIField(str, 2, 30);

		//N402	156			State or Prov Code						O	ID	2/2

		str = "";

		//State - previously we decided whether it should be provider
		//or location, the recordset is pulling the desired value
		str = AdoFldString(rs, "State","");

		OutputString += ParseANSIField(str, 2, 2);

		//N403	116			Postal Code								O	ID	3/15
		
		//Zip - previously we decided whether it should be provider
		//or location, the recordset is pulling the desired value
		str = AdoFldString(rs, "Zip","");

		str = StripNonNum(str);
		OutputString += ParseANSIField(str, 3, 15);

		//N404	26			Country Code							O	ID	2/3

		//we don't use this
		OutputString += "*";

		//N405	NOT USED		
		OutputString += "*";

		//N406	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//60		080		PER		Information Receiver Contact Information	O	3
		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version. Deleted segment.
		if (av4010 == m_avANSIVersion) {
			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "PER";

			//PER01	366			Contact Function Code					M	ID	2/2

			//static "IC";
			OutputString += ParseANSIField("IC",2,2);

			//PER02	93			Name									O	AN	1/60

			str = "";
			//this should not be needed unless different from the NM1 segment
			
			OutputString += ParseANSIField(str,1,60);

			//PER03	365			Communication Number Qualifier			X	ID	2/2

			//static "TE"

			//WorkPhone or LocationsT.Phone - previously we decided
			//whether it should be provider or location, the recordset
			//is pulling the desired value
			str = AdoFldString(rs, "Phone","");
			
			str = StripNonNum(str);
			if(str != "")
				OutputString += ParseANSIField("TE",2,2);
			else
				OutputString += "*";

			//PER04	364			Communication Number					X	AN	1/80

			if(str != "") {
				OutputString += ParseANSIField(str,1,80);
			} else
				OutputString += "*";

			//PER05	365			Communication Number Qualifier			X	ID	2/2		

			// (j.jones 2007-04-05 10:44) - PLID 25506 - If EbillingFormatsT.Use1000APER is checked,
			// then we want to output PER05Qual_1000A and PER06ID_1000A.
			// Otherwise, we send the fax number.

			//TES 7/1/2011 - PLID 40938 - We've already loaded these values
			CString strPER05Qual = m_strPER05Qual_1000A;
			CString strPER06ID = m_strPER06ID_1000A;
			BOOL bUse1000APER = m_bUse1000APER;

			if(!bUse1000APER) {
				//output the fax number

				//supposedly, you can't have two TE qualifiers, so we can't output Phone2

				//what we output next depends if Fax was filled in
				BOOL bFax = FALSE;
				CString strFax;
				//Fax - previously we decided whether it should be provider or location,
				//the recordset is pulling the desired value
				strFax = AdoFldString(rs, "Fax","");
				strFax = StripNonNum(strFax);
				if(strFax != "")
					bFax = TRUE;

				//if LocationsT.Fax not empty
				//static "FX"

				if(bFax) {
					strPER05Qual = "FX";
					strPER06ID = strFax;
				}
			}

			//do not output anything if the ID is blank
			strPER06ID.TrimLeft();
			strPER06ID.TrimRight();
			if(strPER06ID.IsEmpty())
				strPER05Qual = "";

			OutputString += ParseANSIField(strPER05Qual,2,2);

			//PER06	364			Communication Number					X	AN	1/80
			OutputString += ParseANSIField(strPER06ID,1,80);

			//PER07	365			Communication Number Qualifier			X	ID	2/2

			OutputString += "*";

			//PER08	364			Communication Number					X	AN	1/80

			OutputString += "*";

			//PER09	NOT USED
			OutputString += "*";

			// (j.jones 2008-09-02 13:45) - PLID 31114 - if the segment is still only PER*IC, then
			// don't bother exporting anything
			CString strTest = OutputString;
			strTest.TrimRight("*");
			if(strTest != "PER*IC") {
				EndANSISegment(OutputString);
			}
		}

///////////////////////////////////////////////////////////////////////////////

//64		090		PRV		Information Receiver Provider Information	O	1

		//load ProvidersT.TaxonomyCode
		CString strTaxonomy = AdoFldString(rs, "TaxonomyCode","");

		if(!strTaxonomy.IsEmpty()) {

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "PRV";

		//PRV01	1221		Provider Code							M	ID	1/3

		//AD - Admitting
		//AT - Attending
		//BI - Billing
		//CO - Consulting
		//CV - Covering
		//H - Hospital
		//HH - Home Health Care
		//LA - Laboratory
		//OT - Other Physician
		//P1 - Pharmacist
		//P2 - Pharmacy
		//PC - Primary Care Physician
		//PE - Performing
		//R - Rural Health Clinic
		//RF - Referring
		//SB - Submitting
		//SK - Skilled Nursing Facility
		//SU - Supervising

		//for now, use "BI" unless told otherwise

		OutputString += ParseANSIField("BI", 1, 3);	

		//PRV02	128			Reference Identification Qualifier		M	ID	2/3

		//static value "ZZ"
		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version. Now PXC.
		if (av5010 == m_avANSIVersion) {
			OutputString += ParseANSIField("PXC",2,3);
		} else {
			OutputString += ParseANSIField("ZZ",2,3);
		}

		//PRV03	127			Reference Identification				M	AN	1/30

		//ProvidersT.TaxonomyCode
		//This comes from a published HIPAA list.
		//"2086S0122X" is for Plastic and Reconstructive Surgery
		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version. Increase.
		if (av5010 == m_avANSIVersion) {
			OutputString += ParseANSIField(strTaxonomy,1,50);
		} else {
			OutputString += ParseANSIField(strTaxonomy,1,30);
		}

		//PRV04	NOT USED
		OutputString += "*";

		//PRV05	NOT USED
		OutputString += "*";

		//PRV06	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

		}

///////////////////////////////////////////////////////////////////////////////

	rs->Close();

	return Success;

	} NxCatchAll("Error in CEEligibility::ANSI_2100B");

	return Error_Other;
}

int CEEligibility::ANSI_2000C() {

	//Subscriber Level

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\n2000C\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

	try {

		//increment the HL count
		m_ANSIHLCount++;

		//increment the count for the current subscriber
		m_ANSICurrSubscriberHL = m_ANSIHLCount;

		CString OutputString,str;
		_variant_t var;

//							SUBSCRIBER LEVEL

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP 2000C - SUBSCRIBER LEVEL							>1

//67		010		HL		Hierarchical Level						M		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HL";

		//HL01	628			Hierarchical ID Number					M	AN	1/12

		//m_ANSIHLCount

		str.Format("%li",m_ANSIHLCount);
		OutputString += ParseANSIField(str,1,12);

		//HL02	734			Hierarchical Parent ID Number			O	AN	1/12

		//m_ANSICurrProviderHL
		str.Format("%li",m_ANSICurrProviderHL);
		OutputString += ParseANSIField(str,1,12);

		//HL03	735			Hierarchical Level Code					M	ID	1/2

		//static "22"
		OutputString += ParseANSIField("22",1,2);

		//HL04	736			Hierarchical Child Code					O	ID	1/1

		//0 - no patient level HL loop
		//1 - additional patient level HL loop

		str = "0";
		//bPatientIsInsured will be true if the insured party's
		//relation to patient is "Self"
		if(!m_pEligibilityInfo->bPatientIsInsured)
			str = "1";

		OutputString += ParseANSIField(str,1,1);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//69		020		TRN		Subscriber Trace Number					O		9

		//only send if the subscriber is the patient)
		if(m_pEligibilityInfo->bPatientIsInsured) {

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "TRN";

		//TRN01	481			Trace Type Code							M	ID	1/2

		//static "1"
		OutputString += ParseANSIField("1",1,2);

		//TRN02	127			Reference Identification				M	AN	1/30

		///this is a trace number assigned by us
		//send EligibilityRequestsT.ID
		str.Format("%li", m_pEligibilityInfo->nID);
		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version. Increase.
		if (av5010 == m_avANSIVersion) {
			OutputString += ParseANSIField(str,1,50);
		} else {
			OutputString += ParseANSIField(str,1,30);
		}

		//TRN03	509			Originating Company Identifier			O	AN	10/10

		//send the EIN, preceded by a 1
		
		//LocationsT.EIN
		_RecordsetPtr rsLoc = CreateRecordset("SELECT EIN FROM LocationsT WHERE ID = %li", m_pEligibilityInfo->nLocationID);
		if(!rsLoc->eof) {
			CString strEIN = AdoFldString(rsLoc, "EIN","");
			strEIN = StripNonNum(strEIN);
			str.Format("1%s", strEIN);
		}
		else {
			str = "";
		}
		rsLoc->Close();

		OutputString += ParseANSIField(str,10,10);

		//TRN04	127			Reference Identification				O	AN	1/30

		//we can send another trace number if we want
		//let's send PatientsT.PersonID + InsuredPartyT.PersonID
		str.Format("%li-%li", m_pEligibilityInfo->nPatientID, m_pEligibilityInfo->nInsuredPartyID);
		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version. Increase.
		if (av5010 == m_avANSIVersion) {
			OutputString += ParseANSIField(str,1,50);
		}
		else {
			OutputString += ParseANSIField(str,1,30);
		}

		EndANSISegment(OutputString);

		}

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in CEEligibility::ANSI_2000C");

	return Error_Other;
}

int CEEligibility::ANSI_2100C() {

	//Subscriber Name

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\n2100C\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		// (j.jones 2012-11-12 10:20) - PLID 53693 - added CountryCode
		// (j.jones 2014-03-13 10:42) - PLID 61363 - diag codes are no longer loaded here
		_RecordsetPtr rs = CreateParamRecordset("SELECT PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, "
			"InsuredPartyT.IDForInsurance, InsuredPartyT.PolicyGroupNum, PersonT.SocialSecurity, "
			"PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, "
			"PersonT.BirthDate, PersonT.Gender, InsuredPartyT.AssignDate, InsuredPartyT.ExpireDate, "
			"CountriesT.ISOCode AS CountryCode "
			"FROM InsuredPartyT "
			"INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
			"INNER JOIN EligibilityRequestsT ON EligibilityRequestsT.InsuredPartyID = InsuredPartyT.PersonID "			
			"LEFT JOIN CountriesT ON PersonT.Country = CountriesT.CountryName "
			"WHERE InsuredPartyT.PersonID = {INT}", m_pEligibilityInfo->nInsuredPartyID);
		if(rs->eof) {
			//if the recordset is empty, there is no insured party. So halt everything!!!
			rs->Close();
			rs = CreateRecordset("SELECT [First] + ' ' + [Last] AS Name FROM PersonT WHERE ID = %li",m_pEligibilityInfo->nPatientID);
			if(!rs->eof){
				//give a HELPFUL error
				str.Format("Could not open insurance information for patient '%s'.",AdoFldString(rs, "Name",""));
			}
			else
				//serious problems if you get here
				str = "Could not open insurance information.";
			
			rs->Close();
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

//							SUBSCRIBER NAME

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP 2100C - SUBSCRIBER NAME							1

//71		030		NM1		Individual or Organizational Name		M		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M	ID	2/3

		//static "IL"
		OutputString += ParseANSIField("IL",2,3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

		//1 - Person
		OutputString += ParseANSIField("1",1,1);

		//NM103	1035		Name Last or Organization Name			O	AN	1/35

		//PersonT.Last
		str = AdoFldString(rs, "Last","");

		// (j.jones 2013-07-08 15:26) - PLID 57469 - if the setting to include
		// the Title in the Last name is on, append it to the last name
		CString strTitle = AdoFldString(rs, "Title","");
		if(m_HCFAInfo.EligTitleInLast == 1) {
			str += " ";
			str += strTitle;
			str.TrimRight();
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version. Increase.
		if (av5010 == m_avANSIVersion) {
			OutputString += ParseANSIField(str,1,60);
		} else {
			OutputString += ParseANSIField(str,1,35);
		}

		//NM104	1036		Name First								O	AN	1/25

		//blank if NM102 = 2

		//PersonT.First
		str = AdoFldString(rs, "First","");

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation		
		// (c.haag 2010-10-14 12:19) - PLID 40352 - ANSI 5010 version. Increase.
		if (av5010 == m_avANSIVersion) {
			OutputString += ParseANSIField(str,1,35);
		} else {
			OutputString += ParseANSIField(str,1,25);
		}

		//NM105	1037		Name Middle								O	AN	1/25

		//blank if NM102 = 2

		//PersonT.Middle
		str = AdoFldString(rs, "Middle","");

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation		
		OutputString += ParseANSIField(str,1,25);

		//NM106	NOT USED
		OutputString += "*";

		//NM107	1039		Name Suffix								O	AN	1/10

		//blank if NM102 = 2

		//PersonT.Title
		
		// (j.jones 2013-07-08 15:26) - PLID 57469 - if the setting to include
		// the Title in the Last name is on, send nothing here
		if(m_HCFAInfo.EligTitleInLast == 0) {
			str = strTitle;
		}
		else {
			str = "";
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation		
		OutputString += ParseANSIField(str,1,10);

		//NM108	66			Identification Code	Qualifier			X	ID	1/2

		//load the insured's IDForInsurance
		CString strID = AdoFldString(rs, "IDForInsurance","");

		//MI - Member Identification Number
		//ZZ - Mutually Defined

		//use "MI" with IDForInsurance
				
		if(strID != "")
			OutputString += ParseANSIField("MI",1,2);
		else
			OutputString += "*";

		//NM109	67			Identification Code						X	AN	2/80

		//InsuredPartyT.IDForInsurance

		if(strID != "")		
			OutputString += ParseANSIField(strID,2,80);
		else
			OutputString += "*";

		//NM110 NOT USED
		OutputString += "*";

		//NM111	NOT USED
		OutputString += "*";

		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version.
		//NM112 NOT USED
		if (av5010 == m_avANSIVersion) {
			OutputString += "*";
		}

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//74		040		REF		Subscriber Additional Information		O	9

		//This is an optional segment, and from what I can tell, seemingly unnecessary.
		
		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2008-06-23 12:23) - PLID 30434 - this is now configurable in the E-Eligibility Setup,
		// but part of m_HCFAInfo

		if(m_HCFAInfo.Eligibility_2100C_REF_Option != value2100C_None) {

			OutputString = "REF";

			//REF01	128			Reference Ident Qual					M	ID	2/3

			CString strQual = "";
			strID = "";

			if(m_HCFAInfo.Eligibility_2100C_REF_Option == value2100C_SSN) {
				strQual = "SY";
				//load Insured SSN
				strID = AdoFldString(rs, "SocialSecurity","");
				strID = StripNonNum(strID);
			}
			else if(m_HCFAInfo.Eligibility_2100C_REF_Option == value2100C_PolicyGroupNum) {
				strQual = "IG";
				//load Insured PolicyGroupNum
				strID = AdoFldString(rs, "PolicyGroupNum","");
			}

			strID.TrimLeft();
			strID.TrimRight();

			OutputString += ParseANSIField(strQual, 2, 3);

			//REF02	127			Reference Ident							X	AN	1/30

			//InsuredPartyT.PolicyGroupNum
			// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version. Increase.
			if (av5010 == m_avANSIVersion) {
				OutputString += ParseANSIField(strID, 1, 50);
			} else {
				OutputString += ParseANSIField(strID, 1, 30);
			}

			//REF03 352			Description								X	AN	1/80

			//only used if we need to output a state code, when REF01 is 0B

			OutputString += "*";

			//REF04	NOT USED
			OutputString += "*";

			//if there is no number, don't output
			if(!strID.IsEmpty()) {
				EndANSISegment(OutputString);
			}
		}

///////////////////////////////////////////////////////////////////////////////

//77		060		N3		Subscriber Address						O		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//N301	166			Address Information						M	AN	1/55

		//PersonT.Address1
		str = AdoFldString(rs, "Address1","");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		//N302	166			Address Information						O	AN	1/55

		//PersonT.Address2
		str = AdoFldString(rs, "Address2","");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//78		070		N4		Subscriber City/State/Zip				O		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O	AN	2/30

		//PersonT.City
		str = AdoFldString(rs, "City","");

		OutputString += ParseANSIField(str, 2, 30);

		//N402	156			State or Prov Code						O	ID	2/2

		//PersonT.State
		str = AdoFldString(rs, "State","");

		OutputString += ParseANSIField(str, 2, 2);

		//N403	116			Postal Code								O	ID	3/15
		
		//PersonT.Zip
		str = AdoFldString(rs, "Zip","");

		str = StripNonNum(str);
		OutputString += ParseANSIField(str, 3, 15);

		//N404	26			Country Code							O	ID	2/3

		// (j.jones 2012-11-12 10:07) - PLID 53693 - use the insured party's country code, if not US
		CString strCountryCode = AdoFldString(rs, "CountryCode", "");
		if(strCountryCode.CompareNoCase("US") == 0) {
			//leave blank
			strCountryCode = "";
		}
		else {
			//our ISO codes should already be uppercase, force it just incase
			strCountryCode.MakeUpper();
		}
		OutputString += ParseANSIField(strCountryCode, 2, 3);

		//N405	NOT USED		
		OutputString += "*";

		//N406	NOT USED
		OutputString += "*";

		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 support
		//N407		Country Subdivision Code		S	ID	1/3
		if (av5010 == m_avANSIVersion)
		{			
			// Not supported in Practice
			OutputString += "*";			
		}

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//80		090		PRV		Provider Information					O	1
		// (c.haag 2010-10-18 12:09) - PLID 40352 - For ANSI 5010, do not send this because
		// we already send it in 2100B.
		if (av4010 == m_avANSIVersion) {
			CString strTaxonomy = m_pEligibilityInfo->strTaxonomyCode;

			if(!strTaxonomy.IsEmpty()) {

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "PRV";

			//PRV01	1221		Provider Code							M	ID	1/3

			//AD - Admitting
			//AT - Attending
			//BI - Billing
			//CO - Consulting
			//CV - Covering
			//H - Hospital
			//HH - Home Health Care
			//LA - Laboratory
			//OT - Other Physician
			//P1 - Pharmacist
			//P2 - Pharmacy
			//PC - Primary Care Physician
			//PE - Performing
			//R - Rural Health Clinic
			//RF - Referring
			//SB - Submitting
			//SK - Skilled Nursing Facility
			//SU - Supervising

			//for now, use "BI" unless told otherwise

			OutputString += ParseANSIField("BI", 1, 3);	

			//PRV02	128			Reference Identification Qualifier		M	ID	2/3

			//static value "ZZ"
			// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 support requires PXC
			if (av5010 == m_avANSIVersion) {
				OutputString += ParseANSIField("PXC",2,3);
			}
			else {
				OutputString += ParseANSIField("ZZ",2,3);
			}

			//PRV03	127			Reference Identification				M	AN	1/30

			//ProvidersT.TaxonomyCode
			//This comes from a published HIPAA list.
			//"2086S0122X" is for Plastic and Reconstructive Surgery
			// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 - Length increased
			if (av5010 == m_avANSIVersion) {
				OutputString += ParseANSIField(strTaxonomy,1,50);
			} else {
				OutputString += ParseANSIField(strTaxonomy,1,30);
			}

			//PRV04	NOT USED
			OutputString += "*";

			//PRV05	NOT USED
			OutputString += "*";

			//PRV06	NOT USED
			OutputString += "*";

			EndANSISegment(OutputString);

			}
		}

///////////////////////////////////////////////////////////////////////////////

//83		100		DMG		Subscriber Demographic Information		R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DMG";

		//DMG01	1250		Date Time format Qual					X	ID	2/3

		//static value "D8"

		//PersonT.BirthDate
		var = rs->Fields->Item["BirthDate"]->Value;		//insured party birthdate

		if(var.vt == VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%Y%m%d");
		}
		else
			str = "";

		if(str != "")
			OutputString += ParseANSIField("D8", 2, 3);
		else
			OutputString += ParseANSIField("", 2, 3);

		//DMG02	1251		Date Time Period						X	AN	1/35

		OutputString += ParseANSIField(str,1,35);

		//DMG03	1068		Gender Code								O	ID	1/1

		//Allowed Values
		//	Female		"F"
		//	Male		"M"

		//do not output if not known

		//PersonT.Gender
		var = rs->Fields->Item["Gender"]->Value;		//insured party gender

		long gender = VarByte(var,0);
		
		if(gender == 1)
			str = "M";
		else if (gender == 2)
			str = "F";
		else
			str = "";

		OutputString += ParseANSIField(str, 1, 1);

		//DMG04	NOT USED
		OutputString += "*";

		//DMG05	NOT USED
		OutputString += "*";

		//DMG06	NOT USED
		OutputString += "*";

		//DMG07	NOT USED
		OutputString += "*";

		//DMG08	NOT USED
		OutputString += "*";

		//DMG09	NOT USED
		OutputString += "*";

		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version.		
		if (av5010 == m_avANSIVersion) {
			// DMG10 NOT USED
			OutputString += "*";
			// DMG11 NOT USED
			OutputString += "*";
		}

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//85		110		INS		Subscriber Relationship			O		1

		// (j.jones 2009-01-16 08:43) - PLID 32754 - The specs suggest this is
		// almost never used, but Trizetto requires it. So send it.
		// (c.haag 2010-10-18 12:16) - PLID 40352 - Do not use this starting in 5010
		if (av4010 == m_avANSIVersion) {

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "INS";

			//INS01	1073		Yes/No Condition or Response Code		M	ID	1/1

			//INS01 indicates status of the insured. A “Y” value indicates the insured
			//is a subscriber: an “N” value indicates the insured is a dependent.

			//always "Y"
			OutputString += ParseANSIField("Y",1,1);

			//INS02	1069		Individual Relationship Code			M	ID	2/2
			
			//Code indicating the relationship between two individuals or entities
			//18 - Self

			OutputString += ParseANSIField("18",2,2);

			//INS03 NOT USED
			OutputString += "*";

			//INS04 NOT USED
			OutputString += "*";

			//INS05 NOT USED
			OutputString += "*";

			//INS06 NOT USED
			OutputString += "*";

			//INS07 NOT USED
			OutputString += "*";

			//INS08 NOT USED
			OutputString += "*";

			//INS09 NOT USED
			OutputString += "*";

			//INS10 NOT USED
			OutputString += "*";

			//INS11 NOT USED
			OutputString += "*";

			//INS12 NOT USED
			OutputString += "*";

			//INS13 NOT USED
			OutputString += "*";

			//INS14 NOT USED
			OutputString += "*";

			//INS15 NOT USED
			OutputString += "*";

			//INS16 NOT USED
			OutputString += "*";

			//INS17 1470		Number							O	N0	1/9

			//INS17 is the number assigned to each family member born with the
			//same birth date. This number identifies birth sequence for multiple births allowing
			//proper tracking and response of benefits for each dependent (i.e., twins, triplets,
			//etc.).

			//Use to indicate the birth order in the event of multiple birth’s in
			//association with the birth date supplied in DMG02.

			//we do not currently support this field
			OutputString += "*";		

			//we always send 18 - Self, so we always have data in this segment
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

// (c.haag 2010-10-14 16:05) - PLID 40352 - HI segment for ANSI 5010
//86		1150		HI			Subscriber Health Care Diagnosis Code			S	1
		if (av5010 == m_avANSIVersion) 
		{
			OutputString = "HI";

			BOOL bOutput = TRUE;

			//HI01	C022		Health Care Code Info.					M 1

			//HI01-1	1270	Code List Qualifier Code				M 1	ID	1/3

			//BK - Principal Diagnosis (ICD-9)
			//ABK - Principal Diagnosis (ICD-10)
			
			// (j.jones 2014-03-13 10:33) - PLID 61363 - supported ICD-10			
			CString strQual = "BK";
			if(m_pEligibilityInfo->bUseICD10Codes) {
				strQual = "ABK";
			}

			OutputString += ParseANSIField(strQual,1,3);

			//HI01-2	1271	Industry Code							M 1	AN	1/30

			// (j.jones 2014-03-13 10:49) - PLID 61363 - this now just reads from our array
			str = "";
			if(m_pEligibilityInfo->aryDiagCodes.size() > 0) {
				//get the first diagnosis code
				str = m_pEligibilityInfo->aryDiagCodes[0];
				str.Replace(".","");
				str.TrimRight();
			}

			if(str == "")
				bOutput = FALSE;

			OutputString += ":";
			OutputString += str;

			//HI01-3	NOT USED
			//HI01-4	NOT USED
			//HI01-5	NOT USED
			//HI01-6	NOT USED
			//HI01-7	NOT USED

			//HI02	C022		Health Care Code Info.					O 1

			//with no easy way to backtrack, we won't output this element at all
			//if the diag code is blank

			str = "";
			if(m_pEligibilityInfo->aryDiagCodes.size() > 1) {
				//get the second diagnosis code
				str = m_pEligibilityInfo->aryDiagCodes[1];
				str.Replace(".","");
				str.TrimRight();
			}

			//HI02-1	1270	Code List Qualifier Code				M 1	ID	1/3

			//BF - Diagnosis (ICD-9)
			//ABF - Diagnosis (ICD-10)

			strQual = "BF";
			if(m_pEligibilityInfo->bUseICD10Codes) {
				strQual = "ABF";
			}
			if(str != "")
				OutputString += ParseANSIField(strQual,1,3);

			//HI02-2	1271	Industry Code							M 1	AN	1/30

			//BillsT.DiagCode2

			if(str != "") {
				OutputString += ":";
				OutputString += str;
			}

			//HI02-3	NOT USED
			//HI02-4	NOT USED
			//HI02-5	NOT USED
			//HI02-6	NOT USED
			//HI02-7	NOT USED

			//HI03	C022		Health Care Code Info.					O 1

			//with no easy way to backtrack, we won't output this element at all
			//if the diag code is blank

			str = "";
			if(m_pEligibilityInfo->aryDiagCodes.size() > 2) {
				//get the third diagnosis code
				str = m_pEligibilityInfo->aryDiagCodes[2];
				str.Replace(".","");
				str.TrimRight();
			}

			//HI03-2	1270	Code List Qualifier Code				M 1	ID	1/3

			//BF - Diagnosis (ICD-9)
			//ABF - Diagnosis (ICD-10)
			if(str != "")
				OutputString += ParseANSIField(strQual,1,3);

			//HI03-2	1271	Industry Code							M 1	AN	1/30

			//BillsT.DiagCode3

			if(str != "") {
				OutputString += ":";
				OutputString += str;
			}

			//HI03-3	NOT USED
			//HI03-4	NOT USED
			//HI03-5	NOT USED
			//HI03-6	NOT USED
			//HI03-7	NOT USED

			//HI04	C022		Health Care Code Info.					O 1

			//with no easy way to backtrack, we won't output this element at all
			//if the diag code is blank

			str = "";
			if(m_pEligibilityInfo->aryDiagCodes.size() > 3) {
				//get the fourth diagnosis code
				str = m_pEligibilityInfo->aryDiagCodes[3];
				str.Replace(".","");
				str.TrimRight();
			}

			//HI04-2	1270	Code List Qualifier Code				M 1	ID	1/3

			//BF - Diagnosis (ICD-9)
			//ABF - Diagnosis (ICD-10)
			if(str != "")
				OutputString += ParseANSIField(strQual,1,3);

			//HI04-2	1271	Industry Code							M 1	AN	1/30

			//BillsT.DiagCode4

			if(str != "") {
				OutputString += ":";
				OutputString += str;
			}

			//HI04-3	NOT USED
			//HI04-4	NOT USED
			//HI04-5	NOT USED
			//HI04-6	NOT USED
			//HI04-7	NOT USED

			// We only support four diagnosis codes

			if(bOutput) {
				EndANSISegment(OutputString);
			}

		}

///////////////////////////////////////////////////////////////////////////////

//87		120		DTP		Subscriber Date					O		9

		//Ref.	Data		Name									Attributes
		//Des.	Element
		// (c.haag 2010-10-18 12:09) - PLID 40352 - Do not send as of ANSI 5010.
		// In 4010 it specifically states "absence of this date implies the request is
		// for the date the transaction is processed"
		if (av4010 == m_avANSIVersion) 
		{
			OutputString = "DTP";

			//DTP01	374			Date/Time Qualifier						M	ID	3/3

			//static "307" - Eligibility
			OutputString += ParseANSIField("307",3,3);

			//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

			//static "D8"
			OutputString += ParseANSIField("D8",2,3);

			//DTP03	1251		Date Time Period						M	AN	1/35

			// (j.jones 2010-03-25 16:16) - PLID 37902 - fixed to be today's date, year-first
			CString strToday;
			COleDateTime dtNow = COleDateTime::GetCurrentTime();
			strToday = dtNow.Format("%Y%m%d");

			OutputString += ParseANSIField(strToday,1,35);	
				
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

	rs->Close();

	return Success;

	} NxCatchAll("Error in CEEligibility::ANSI_2100C");

	return Error_Other;
}

int CEEligibility::ANSI_2110C() {

	//Subscriber Eligibility Or Benefit Inquiry Information

	//only to be used if the subscriber is the patient

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\n2110C\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		// (c.haag 2010-10-14 16:53) - PLID 40352 - Added PlaceCodes
		// (j.jones 2013-03-28 15:07) - PLID 52182 - benefit categories are now in EligibilityBenefitCategoriesT
		// (j.jones 2014-03-13 10:42) - PLID 61363 - diag codes are no longer loaded here
		_RecordsetPtr rs = CreateParamRecordset("SELECT EligibilityBenefitCategoriesT.Code AS CategoryCode, CPTCodeT.Code, "
			"Modifier1, Modifier2, Modifier3, Modifier4, "
			"PlaceOfServiceCodesT.PlaceCodes "
			"FROM EligibilityRequestsT "
			"LEFT JOIN EligibilityBenefitCategoriesT ON EligibilityRequestsT.BenefitCategoryID = EligibilityBenefitCategoriesT.ID "
			"LEFT JOIN ServiceT ON EligibilityRequestsT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"LEFT JOIN PlaceOfServiceCodesT ON EligibilityRequestsT.PlaceOfServiceID = PlaceOfServiceCodesT.ID "
			"WHERE EligibilityRequestsT.ID = {INT}", m_pEligibilityInfo->nID);
		if(rs->eof) {
			//if the recordset is empty, there is no eligibility request. So halt everything!!!
			rs->Close();
			rs = CreateRecordset("SELECT [First] + ' ' + [Last] AS Name FROM PersonT WHERE ID = %li",m_pEligibilityInfo->nPatientID);
			if(!rs->eof){
				//give a HELPFUL error
				str.Format("Could not open eligibility information for patient '%s'.",AdoFldString(rs, "Name",""));
			}
			else
				//serious problems if you get here
				str = "Could not open eligibility information.";
			
			rs->Close();
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		//used to determine if we send EQ01 or EQ02
		BOOL bUseSpecificCode = FALSE;

		CString strCategoryCode = AdoFldString(rs, "CategoryCode","");
		CString strCPTCode = AdoFldString(rs, "Code","");

		bUseSpecificCode = !strCPTCode.IsEmpty();

//							SUBSCRIBER ELIGIBILITY OR BENEFIT INQUIRY INFORMATION

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP 2110C - SUBSCRIBER ELIGIBILITY OR BENEFIT INQUIRY INFORMATION		>1

//89		130		EQ		Eligibility or Benefit Inquiry			O		1

		//The specs state that you can send EQ01 or EQ02, but not both.
		//EQ01 is a service type, and 30 is a generic benefit inquiry.
		//EQ02 is a service code. The specs suggest that EQ01 codes other
		//than 30, and any EQ02 code, may be supported at the discretion
		//of the receiver. Translation: some people won't support it, some
		//will, and probably someone will require it. Wonderful.

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "EQ";

		//EQ01		1365	Service Type Code						X	ID	1/2

		str = "";

		//there are over 100 service type codes, refer to the specs for the full list,
		//but 30 is the standard to use (others may not even be supported)
		
		//30 - Health Benefit Plan Coverage (If only a single category of inquiry can be supported, use this code.)

		//if not requesting a specific code, then use 30 in EQ01
		if(!bUseSpecificCode) {
			str = strCategoryCode;
			if(str.IsEmpty())
				str = "30";
		}

		OutputString += ParseANSIField(str,1,2);

		//EQ02		C003	COMPOSITE MEDICAL PROCEDURE	IDENTIFIER	X

		if(bUseSpecificCode) {

		//EQ02 - 1	235		Product/Service ID Qualifier			M	ID	2/2

		//static "HC"
		CString strQual = "HC";

		// (j.jones 2013-01-25 12:25) - PLID 54853 - IsValidServiceCodeQualifier
		// needs updated any time we support exporting a new qualifier type,
		// currently we only support HC. This ensures that we never send other 
		// qualifiers without updating this function.
		if(!IsValidServiceCodeQualifier(strQual)) {
			//You just tried to send a qualifier not supported by IsValidServiceCodeQualifier,
			//Go into that function and add your new qualifier to the supported list.
			ASSERT(FALSE);
			ThrowNxException("Loop 2110C tried to send invalid service qualifier %s.", strQual);			
		}

		OutputString += ParseANSIField(strQual,2,2);

		//EQ02 - 2	234		Product/Service ID						M	AN	1/48

		//we loaded the CPT Code earlier
		str = strCPTCode;

		OutputString += ":";
		OutputString += str;

		//EQ02 - 3	1339	Procedure Modifier						O	AN	2/2

		//load modifier 1
		str = AdoFldString(rs, "Modifier1","");

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//EQ02 - 4	1339	Procedure Modifier						O	AN	2/2

		//load modifier 2
		str = AdoFldString(rs, "Modifier2","");

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//EQ02 - 5	1339	Procedure Modifier						O	AN	2/2

		//load modifier 3
		str = AdoFldString(rs, "Modifier3","");

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//EQ02 - 6	1339	Procedure Modifier						O	AN	2/2

		//load modifier 4
		str = AdoFldString(rs, "Modifier4","");

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//EQ02 - 7	NOT USED

		// (c.haag 2010-10-14 15:41) - PLID 40352 - ANSI 5010 has EQ02 - 8
		//EQ02 - 8  NOT USED

		}
		else {
			//if not using EQ02, don't forget the separator
			OutputString += "*";
		}

		//EQ03		1207	Coverage Level Code						O	ID	3/3

		//"Use EQ03 when an information source supports or may be thought
		//to support the function of identifying benefits by the Benefit
		//Coverage Level Code. Use this code to identify the types and
		//number of entities that the request is to apply to. If not supported,
		//the information source will process without this data element."

		//CHD - Children Only
		//DEP - Dependents Only
		//ECH - Employee and Children
		//EMP - Employee Only
		//ESP - Employee and Spouse
		//FAM - Family
		//IND - Individual
		//SPC - Spouse and Children
		//SPO - Spouse Only

		//ignore for now
		OutputString += ParseANSIField("",3,3);

		//EQ04		1336	Insurance Type Code						O	ID	1/3
		// (c.haag 2010-10-14 11:15) - PLID 40352 - NU for ANSI 5010
		if (av5010 == m_avANSIVersion) 
		{
			OutputString += "*";
		}
		else {
			str = "";

			//"Use this code to identify the specific type of insurance the inquiry
			//applies to if the information source has multiple insurance lines
			//that apply to the person being inquired about. Do not use if the
			//insurance type can be determined either by the person’s identifiers
			//or the information source’s identifiers."

			//not sure if this is needed or not, very few categories exist

			// (j.jones 2008-09-09 11:13) - PLID 18695 - This is not entirely
			// 1-to-1 with the ANSI SBR09 codes, so do a manual conversion here,
			// even when it does happen to be the same.

			str = "";

			switch(m_pEligibilityInfo->eInsType) {
				case(itcCommercial):
					str = "C1";
					break;
				// (j.jones 2009-03-27 17:21) - PLID 33724 - supported supplemental types
				case(itcCommercialSupplemental):
					str = "SP";
					break;
				// (j.jones 2010-10-15 14:41) - PLID 40953 - new for 5010
				case(itcMedicarePartA):
					if (av5010 == m_avANSIVersion) {
						str = "MA";
					}
					else {
						str = "";
					}
					break;
				case(itcMedicarePartB):
					str = "MB";
					break;
				case(itcMedicaid):
					str = "MC";
					break;
				case(itcWorkersComp):
					str = "WC";
					break;
				case(itcHMO):
					str = "HM";
					break;
				case(itcHMO_MedicareRisk):
					str = "HN";
					break;
				case(itcAutomobileMedical):
					str = "AP";
					break;
				case(itcPPO):
					str = "PR";
					break;
				case(itcPOS):
					str = "PS";
					break;
				default:
					str = "";
					break;
			}

			OutputString += ParseANSIField(str,1,3);
		}

		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 support for EQ05
		//EQ05		1328	Diagnosis Code Pointer						O	ID	1/2
		if (av5010 == m_avANSIVersion) {
			// (j.jones 2014-03-13 10:58) - PLID 61363 - just reference our array
			if(m_pEligibilityInfo->aryDiagCodes.size() > 0) {
				OutputString += ParseANSIField("1",1,2);
			}
			if(m_pEligibilityInfo->aryDiagCodes.size() > 1) {
				OutputString += ParseANSIField("2",1,2);
			}
			if(m_pEligibilityInfo->aryDiagCodes.size() > 2) {
				OutputString += ParseANSIField("3",1,2);
			}
			if(m_pEligibilityInfo->aryDiagCodes.size() > 3) {
				OutputString += ParseANSIField("4",1,2);
			}
		}

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//99		135		AMT		Subscriber Spend Down Amount			O	2

		//"Use this segment only if it is necessary to report a Spend Down
		//amount. Under certain Medicaid programs, individuals must indicate
		//the dollar amount that they wish to apply towards their deductible.
		//These programs require individuals to pay a certain amount towards
		//their health care cost before Medicaid coverage starts."

		//currently unused

///////////////////////////////////////////////////////////////////////////////

//101		170		III		Subscriber Eligibility Or Benefit		O	10
//							Additional Inquiry Information

		//used to report diagnosis codes

		//Diagnosis 1

		OutputString = "III";

		//III01		1270	Code List Qualifier Code				X	ID	1/3		ANSI 4010
		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version has a different usage
		if (av5010 == m_avANSIVersion) {
			//static "ZZ" (mutually defined)
			OutputString += ParseANSIField("ZZ",1,3);
		} else {
			//static "BK" (principal diagnosis)
			//this is 4010, which doesn't support ICD-10
			OutputString += ParseANSIField("BK",1,3);
		}

		//III02		1271	Industry Code							X	AN	1/30

		// (c.haag 2010-10-14 16:53) - PLID 40352 - ANSI 5010 version has a different usage
		if (av5010 == m_avANSIVersion)	{
			// Load the place of service code
			str = AdoFldString(rs, "PlaceCodes","");
		} else  {
			//load the diagnosis 1
			// (j.jones 2014-03-13 10:49) - PLID 61363 - this now just reads from our array
			str = "";
			if(m_pEligibilityInfo->aryDiagCodes.size() > 0) {
				//get the first diagnosis code
				str = m_pEligibilityInfo->aryDiagCodes[0];
			}
		}
		str.Replace(".","");
		str.TrimRight();

		OutputString += ParseANSIField(str,1,30);

		//III03 NOT USED
		OutputString += "*";

		//III04 NOT USED
		OutputString += "*";

		//III05 NOT USED
		OutputString += "*";
		
		//III06 NOT USED
		OutputString += "*";

		//III07 NOT USED
		OutputString += "*";

		//III08 NOT USED
		OutputString += "*";

		//III09 NOT USED
		OutputString += "*";

		if(str != "")
			EndANSISegment(OutputString);


		// (c.haag 2010-10-14 16:53) - PLID 40352 - If using ANSI 5010, there's only one code
		// (Place of service designation), so we don't need to do more than that.
		if (av5010 == m_avANSIVersion) 
		{
		}
		else {
			//Diagnosis 2

			OutputString = "III";

			//III01		1270	Code List Qualifier Code				X	ID	1/3

			//static "BF" (diagnosis)
			//this is 4010, which doesn't support ICD-10
			OutputString += ParseANSIField("BF",1,3);

			//III02		1271	Industry Code							X	AN	1/30

			//load the diagnosis 2
			// (j.jones 2014-03-13 10:49) - PLID 61363 - this now just reads from our array
			str = "";
			if(m_pEligibilityInfo->aryDiagCodes.size() > 1) {
				//get the second diagnosis code
				str = m_pEligibilityInfo->aryDiagCodes[1];
				str.Replace(".","");
				str.TrimRight();
			}

			OutputString += ParseANSIField(str,1,30);

			//III03 NOT USED
			OutputString += "*";

			//III04 NOT USED
			OutputString += "*";

			//III05 NOT USED
			OutputString += "*";

			//III06 NOT USED
			OutputString += "*";

			//III07 NOT USED
			OutputString += "*";

			//III08 NOT USED
			OutputString += "*";

			//III09 NOT USED
			OutputString += "*";

			if(str != "")
				EndANSISegment(OutputString);

			//Diagnosis 3

			OutputString = "III";

			//III01		1270	Code List Qualifier Code				X	ID	1/3

			//static "BF" (diagnosis)
			//this is 4010, which doesn't support ICD-10
			OutputString += ParseANSIField("BF",1,3);

			//III02		1271	Industry Code							X	AN	1/30

			//load the diagnosis 3
			// (j.jones 2014-03-13 10:49) - PLID 61363 - this now just reads from our array
			str = "";
			if(m_pEligibilityInfo->aryDiagCodes.size() > 2) {
				//get the third diagnosis code
				str = m_pEligibilityInfo->aryDiagCodes[2];
				str.Replace(".","");
				str.TrimRight();
			}

			OutputString += ParseANSIField(str,1,30);

			//III03 NOT USED
			OutputString += "*";

			//III04 NOT USED
			OutputString += "*";

			//III05 NOT USED
			OutputString += "*";
			
			//III06 NOT USED
			OutputString += "*";

			//III07 NOT USED
			OutputString += "*";

			//III08 NOT USED
			OutputString += "*";

			//III09 NOT USED
			OutputString += "*";

			if(str != "")
				EndANSISegment(OutputString);

			//Diagnosis 4

			OutputString = "III";

			//III01		1270	Code List Qualifier Code				X	ID	1/3

			//static "BF" (diagnosis)
			//this is 4010, which doesn't support ICD-10
			OutputString += ParseANSIField("BF",1,3);

			//III02		1271	Industry Code							X	AN	1/30

			//load the diagnosis 4
			// (j.jones 2014-03-13 10:49) - PLID 61363 - this now just reads from our array
			str = "";
			if(m_pEligibilityInfo->aryDiagCodes.size() > 3) {
				//get the fourth diagnosis code
				str = m_pEligibilityInfo->aryDiagCodes[3];
				str.Replace(".","");
				str.TrimRight();
			}

			OutputString += ParseANSIField(str,1,30);

			//III03 NOT USED
			OutputString += "*";

			//III04 NOT USED
			OutputString += "*";

			//III05 NOT USED
			OutputString += "*";

			//III06 NOT USED
			OutputString += "*";

			//III07 NOT USED
			OutputString += "*";

			//III08 NOT USED
			OutputString += "*";

			//III09 NOT USED
			OutputString += "*";

			if(str != "")
				EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//104		190		REF		Subscriber Additional Information		O	1

		//used to report referral or prior authorization numbers

		OutputString = "REF";

		//REF01		128		Reference Identification Qualifier		M	ID	2/3

		CString strID = "";
		CString strQual = "";

		//TODO: consider loading an insurance referral or prior auth num

		//9F - Referral Number
		//G1 - Prior Authorization Number

		OutputString += ParseANSIField(strQual,2,3);

		//REF02		127		Reference Identification				X	AN	1/30

		OutputString += ParseANSIField(strID,1,30);

		//REF03 NOT USED
		OutputString += "*";

		//REF04 NOT USED
		OutputString += "*";

		if(strID != "")
			EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//106		200		DTP		Subscriber Eligibility/Benefit Date		O		9

		//only used to override dates in 2100C, at the moment unused

///////////////////////////////////////////////////////////////////////////////

	rs->Close();

	return Success;

	} NxCatchAll("Error in CEEligibility::ANSI_2110C");

	return Error_Other;
}

int CEEligibility::ANSI_2000D() {

	//Dependent Level

	//only used if the patient is not the insured

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\n2000D\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

	try {

		//increment the HL count
		m_ANSIHLCount++;

		//increment the count for the current patient
		m_ANSICurrPatientHL = m_ANSIHLCount;

		CString OutputString,str;
		_variant_t var;

//							DEPENDENT LEVEL

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP 2000D - DEPENDENT LEVEL							>1

//108		010		HL		Hierarchical Level						M		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HL";

		//HL01	628			Hierarchical ID Number					M	AN	1/12

		//m_ANSIHLCount

		str.Format("%li",m_ANSIHLCount);
		OutputString += ParseANSIField(str,1,12);

		//HL02	734			Hierarchical Parent ID Number			O	AN	1/12

		//m_ANSICurrSubscriberHL
		str.Format("%li",m_ANSICurrSubscriberHL);
		OutputString += ParseANSIField(str,1,12);

		//HL03	735			Hierarchical Level Code					M	ID	1/2

		//static "23"
		OutputString += ParseANSIField("23",1,2);

		//HL04	736			Hierarchical Child Code					O	ID	1/1

		//static "0"
		OutputString += ParseANSIField("0",1,1);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//112		020		TRN		Dependent Trace Number					O		9

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "TRN";

		//TRN01	481			Trace Type Code							M	ID	1/2

		//static "1"
		OutputString += ParseANSIField("1",1,2);

		//TRN02	127			Reference Identification				M	AN	1/30

		///this is a trace number assigned by us
		//send EligibilityRequestsT.ID
		str.Format("%li", m_pEligibilityInfo->nID);
		OutputString += ParseANSIField(str,1,30);

		//TRN03	509			Originating Company Identifier			O	AN	10/10

		//send the EIN, preceded by a 1
		
		//LocationsT.EIN
		_RecordsetPtr rsLoc = CreateRecordset("SELECT EIN FROM LocationsT WHERE ID = %li", m_pEligibilityInfo->nLocationID);
		if(!rsLoc->eof) {
			CString strEIN = AdoFldString(rsLoc, "EIN","");
			strEIN = StripNonNum(strEIN);
			str.Format("1%s", strEIN);
		}
		else {
			str = "";
		}
		rsLoc->Close();

		OutputString += ParseANSIField(str,10,10);

		//TRN04	127			Reference Identification				O	AN	1/30

		//we can send another trace number if we want
		//let's send PatientsT.PersonID + InsuredPartyT.PersonID
		str.Format("%li-%li", m_pEligibilityInfo->nPatientID, m_pEligibilityInfo->nInsuredPartyID);
		OutputString += ParseANSIField(str,1,30);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in CEEligibility::ANSI_2000D");

	return Error_Other;
}

int CEEligibility::ANSI_2100D() {

	//Dependent Name

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\n2100D\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		// (j.jones 2010-05-25 16:09) - PLID 38868 - added the patient's SocialSecurity to the query
		// (j.jones 2012-11-12 10:20) - PLID 53693 - added CountryCode
		_RecordsetPtr rs = CreateParamRecordset("SELECT PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, "
			"InsuredPartyT.IDForInsurance, InsuredPartyT.PolicyGroupNum, "
			"PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, "
			"PersonT.BirthDate, PersonT.Gender, InsuredPartyT.AssignDate, InsuredPartyT.ExpireDate, PersonT.SocialSecurity, "
			"CountriesT.ISOCode AS CountryCode "
			"FROM InsuredPartyT "
			"INNER JOIN PersonT ON InsuredPartyT.PatientID = PersonT.ID "
			"LEFT JOIN CountriesT ON PersonT.Country = CountriesT.CountryName "
			"WHERE InsuredPartyT.PersonID = {INT}", m_pEligibilityInfo->nInsuredPartyID);
		if(rs->eof) {
			//if the recordset is empty, there is no patient linked to this insured party. So halt everything!!!
			rs->Close();
			rs = CreateRecordset("SELECT [First] + ' ' + [Last] AS Name FROM PersonT WHERE ID = %li",m_pEligibilityInfo->nPatientID);
			if(!rs->eof){
				//give a HELPFUL error
				str.Format("Could not open dependent information for patient '%s'.",AdoFldString(rs, "Name",""));
			}
			else
				//serious problems if you get here
				str = "Could not open dependent information.";
			
			rs->Close();
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

//							DEPENDENT NAME

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP 2100D - DEPENDENT NAME							1

//114		030		NM1		Individual or Organizational Name		M		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M	ID	2/3

		//static "03"
		OutputString += ParseANSIField("03",2,3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

		//1 - Person
		OutputString += ParseANSIField("1",1,1);

		//NM103	1035		Name Last or Organization Name			O	AN	1/35

		//PersonT.Last
		str = AdoFldString(rs, "Last","");

		// (j.jones 2013-07-08 15:26) - PLID 57469 - if the setting to include
		// the Title in the Last name is on, append it to the last name
		CString strTitle = AdoFldString(rs, "Title","");
		if(m_HCFAInfo.EligTitleInLast == 1) {
			str += " ";
			str += strTitle;
			str.TrimRight();
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version. Increase.
		if (av5010 == m_avANSIVersion) {
			OutputString += ParseANSIField(str,1,60);
		} else {
			OutputString += ParseANSIField(str,1,35);
		}

		//NM104	1036		Name First								O	AN	1/25

		//PersonT.First
		str = AdoFldString(rs, "First","");

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation		
		OutputString += ParseANSIField(str,1,25);

		//NM105	1037		Name Middle								O	AN	1/25

		//PersonT.Middle
		str = AdoFldString(rs, "Middle","");

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation	
		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version. Increase.
		if (av5010 == m_avANSIVersion) {
			OutputString += ParseANSIField(str,1,35);
		} else {
			OutputString += ParseANSIField(str,1,25);
		}

		//NM106	NOT USED
		OutputString += "*";

		//NM107	1039		Name Suffix								O	AN	1/10

		//PersonT.Title
		
		// (j.jones 2013-07-08 15:26) - PLID 57469 - if the setting to include
		// the Title in the Last name is on, send nothing here
		if(m_HCFAInfo.EligTitleInLast == 0) {
			str = strTitle;
		}
		else {
			str = "";
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation		
		OutputString += ParseANSIField(str,1,10);

		//NM108	NOT USED
		OutputString += "*";

		//NM109	NOT USED
		OutputString += "*";

		//NM110 NOT USED
		OutputString += "*";

		//NM111	NOT USED
		OutputString += "*";

		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version. Increase.
		if (av5010 == m_avANSIVersion) {
			//NM112 NOT USED
			OutputString += "*";
		}

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//116		040		REF		Dependent Additional Information		O	9

		//This is an optional segment, and from what I can tell, seemingly unnecessary.
		
		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2010-05-25 15:44) - PLID 38868 - this is now configurable in the E-Eligibility Setup,
		// but part of m_HCFAInfo

		if(m_HCFAInfo.Eligibility_2100D_REF != value2100D_None) {

			OutputString = "REF";

			//REF01	128			Reference Ident Qual					M	ID	2/3

			CString strQual = "";
			CString strID = "";

			if(m_HCFAInfo.Eligibility_2100D_REF == value2100D_SSN) {
				strQual = "SY";
				//load patient SSN
				strID = AdoFldString(rs, "SocialSecurity","");
				strID = StripNonNum(strID);
			}
			else if(m_HCFAInfo.Eligibility_2100D_REF == value2100D_PolicyGroupNum) {
				strQual = "IG";
				//load insured PolicyGroupNum
				strID = AdoFldString(rs, "PolicyGroupNum","");
			}

			strID.TrimLeft();
			strID.TrimRight();

			OutputString += ParseANSIField(strQual, 2, 3);

			//REF02	127			Reference Ident							X	AN	1/30

			//InsuredPartyT.PolicyGroupNum

			OutputString += ParseANSIField(strID, 1, 30);

			//REF03 352			Description								X	AN	1/80

			//only used if we need to output a state code, when REF01 is 0B

			OutputString += "*";

			//REF04	NOT USED
			OutputString += "*";

			//if there is no number, don't output
			if(!strID.IsEmpty()) {
				EndANSISegment(OutputString);
			}
		}

///////////////////////////////////////////////////////////////////////////////

//118		060		N3		Dependent Address						O		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//N301	166			Address Information						M	AN	1/55

		//PersonT.Address1
		str = AdoFldString(rs, "Address1","");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		//N302	166			Address Information						O	AN	1/55

		//PersonT.Address2
		str = AdoFldString(rs, "Address2","");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//119		070		N4		Dependent City/State/Zip				O		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O	AN	2/30

		//PersonT.City
		str = AdoFldString(rs, "City","");

		OutputString += ParseANSIField(str, 2, 30);

		//N402	156			State or Prov Code						O	ID	2/2

		//PersonT.State
		str = AdoFldString(rs, "State","");

		OutputString += ParseANSIField(str, 2, 2);

		//N403	116			Postal Code								O	ID	3/15
		
		//PersonT.Zip
		str = AdoFldString(rs, "Zip","");

		str = StripNonNum(str);
		OutputString += ParseANSIField(str, 3, 15);

		//N404	26			Country Code							O	ID	2/3

		// (j.jones 2012-11-12 10:07) - PLID 53693 - use the insured party's country code, if not US
		CString strCountryCode = AdoFldString(rs, "CountryCode", "");
		if(strCountryCode.CompareNoCase("US") == 0) {
			//leave blank
			strCountryCode = "";
		}
		else {
			//our ISO codes should already be uppercase, force it just incase
			strCountryCode.MakeUpper();
		}
		OutputString += ParseANSIField(strCountryCode, 2, 3);

		//N405	NOT USED
		OutputString += "*";

		//N406	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//121		090		PRV		Provider Information					O	1
		// (c.haag 2010-10-18 12:09) - PLID 40352 - For ANSI 5010, do not send this because
		// we already send it in 2100B.
		if (av4010 == m_avANSIVersion) {
			CString strTaxonomy = m_pEligibilityInfo->strTaxonomyCode;

			if(!strTaxonomy.IsEmpty()) {

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "PRV";

			//PRV01	1221		Provider Code							M	ID	1/3

			//AD - Admitting
			//AT - Attending
			//BI - Billing
			//CO - Consulting
			//CV - Covering
			//H - Hospital
			//HH - Home Health Care
			//LA - Laboratory
			//OT - Other Physician
			//P1 - Pharmacist
			//P2 - Pharmacy
			//PC - Primary Care Physician
			//PE - Performing
			//R - Rural Health Clinic
			//RF - Referring
			//SB - Submitting
			//SK - Skilled Nursing Facility
			//SU - Supervising

			//for now, use "BI" unless told otherwise

			OutputString += ParseANSIField("BI", 1, 3);	

			//PRV02	128			Reference Identification Qualifier		M	ID	2/3

			//static value "ZZ"
			OutputString += ParseANSIField("ZZ",2,3);

			//PRV03	127			Reference Identification				M	AN	1/30

			//ProvidersT.TaxonomyCode
			//This comes from a published HIPAA list.
			//"2086S0122X" is for Plastic and Reconstructive Surgery
			OutputString += ParseANSIField(strTaxonomy,1,30);

			//PRV04	NOT USED
			OutputString += "*";

			//PRV05	NOT USED
			OutputString += "*";

			//PRV06	NOT USED
			OutputString += "*";

			EndANSISegment(OutputString);

			}
		}

///////////////////////////////////////////////////////////////////////////////

//124		100		DMG		Dependent Demographic Information		R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DMG";

		//DMG01	1250		Date Time format Qual					X	ID	2/3

		//static value "D8"

		//PersonT.BirthDate
		var = rs->Fields->Item["BirthDate"]->Value;		//patient birthdate

		if(var.vt == VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%Y%m%d");
		}
		else
			str = "";

		if(str != "")
			OutputString += ParseANSIField("D8", 2, 3);
		else
			OutputString += ParseANSIField("", 2, 3);

		//DMG02	1251		Date Time Period						X	AN	1/35

		OutputString += ParseANSIField(str,1,35);

		//DMG03	1068		Gender Code								O	ID	1/1

		//Allowed Values
		//	Female		"F"
		//	Male		"M"

		//do not output if not known

		//PersonT.Gender
		var = rs->Fields->Item["Gender"]->Value;		//patient gender

		long gender = VarByte(var,0);
		
		if(gender == 1)
			str = "M";
		else if (gender == 2)
			str = "F";
		else
			str = "";

		OutputString += ParseANSIField(str, 1, 1);

		//DMG04	NOT USED
		OutputString += "*";

		//DMG05	NOT USED
		OutputString += "*";

		//DMG06	NOT USED
		OutputString += "*";

		//DMG07	NOT USED
		OutputString += "*";

		//DMG08	NOT USED
		OutputString += "*";

		//DMG09	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//126		110		INS		Dependent Relationship			O		1

		// (j.jones 2009-01-16 08:43) - PLID 32754 - The specs suggest this is
		// almost never used, but Trizetto requires it. So send it.

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "INS";

		//INS01	1073		Yes/No Condition or Response Code		M	ID	1/1

		//INS01 indicates status of the insured. A “Y” value indicates the insured
		//is a subscriber: an “N” value indicates the insured is a dependent.

		//always "N"
		OutputString += ParseANSIField("N",1,1);

		//INS02	1069		Individual Relationship Code			M	ID	2/2
		
		//Code indicating the relationship between two individuals or entities
		//01 - Spouse
		//19 - Child
		//34 - Other Adult

		CString strRelationshipCode = "";
		if(m_pEligibilityInfo->strRelationToPatient == "Spouse") {
			strRelationshipCode = "01";
		}
		else if(m_pEligibilityInfo->strRelationToPatient == "Child") {
			strRelationshipCode = "19";
		}
		else if(m_pEligibilityInfo->strRelationToPatient == "Other Adult") {
			strRelationshipCode = "34";
		}
		OutputString += ParseANSIField(strRelationshipCode,2,2);

		//INS03 NOT USED
		OutputString += "*";

		//INS04 NOT USED
		OutputString += "*";

		//INS05 NOT USED
		OutputString += "*";

		//INS06 NOT USED
		OutputString += "*";

		//INS07 NOT USED
		OutputString += "*";

		//INS08 NOT USED
		OutputString += "*";

		//INS09 NOT USED
		OutputString += "*";

		//INS10 NOT USED
		OutputString += "*";

		//INS11 NOT USED
		OutputString += "*";

		//INS12 NOT USED
		OutputString += "*";

		//INS13 NOT USED
		OutputString += "*";

		//INS14 NOT USED
		OutputString += "*";

		//INS15 NOT USED
		OutputString += "*";

		//INS16 NOT USED
		OutputString += "*";

		//INS17 1470		Number							O	N0	1/9

		//INS17 is the number assigned to each family member born with the
		//same birth date. This number identifies birth sequence for multiple births allowing
		//proper tracking and response of benefits for each dependent (i.e., twins, triplets,
		//etc.).

		//Use to indicate the birth order in the event of multiple birth’s in
		//association with the birth date supplied in DMG02.

		//we do not currently support this field
		OutputString += "*";		

		//don't output if we have no data
		if(strRelationshipCode != "") {			
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//129		120		DTP		Dependent Date					O		9

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M	ID	3/3

		//static "307" - Eligibility
		OutputString += ParseANSIField("307",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

		//static "D8"
		OutputString += ParseANSIField("D8",2,3);

		//DTP03	1251		Date Time Period						M	AN	1/35

		// (j.jones 2010-03-25 16:16) - PLID 37902 - fixed to be today's date, year-first
		CString strToday;
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		strToday = dtNow.Format("%Y%m%d");

		OutputString += ParseANSIField(strToday,1,35);	
			
		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in CEEligibility::ANSI_2100D");

	return Error_Other;
}

int CEEligibility::ANSI_2110D() {

	//Dependent Eligibility Or Benefit Inquiry Information

	//only to be used if the subscriber is not the patient

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\n2110D\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		// (c.haag 2010-10-15 9:21) - PLID 40352 - Added PlaceCodes
		// (j.jones 2013-03-28 15:07) - PLID 52182 - benefit categories are now in EligibilityBenefitCategoriesT
		// (j.jones 2014-03-13 10:42) - PLID 61363 - diag codes are no longer loaded here
		_RecordsetPtr rs = CreateParamRecordset("SELECT EligibilityBenefitCategoriesT.Code AS CategoryCode, CPTCodeT.Code, "
			"Modifier1, Modifier2, Modifier3, Modifier4, "
			"PlaceOfServiceCodesT.PlaceCodes "
			"FROM EligibilityRequestsT "
			"LEFT JOIN EligibilityBenefitCategoriesT ON EligibilityRequestsT.BenefitCategoryID = EligibilityBenefitCategoriesT.ID "
			"LEFT JOIN ServiceT ON EligibilityRequestsT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "			
			"LEFT JOIN PlaceOfServiceCodesT ON EligibilityRequestsT.PlaceOfServiceID = PlaceOfServiceCodesT.ID "
			"WHERE EligibilityRequestsT.ID = {INT}", m_pEligibilityInfo->nID);
		if(rs->eof) {
			//if the recordset is empty, there is no eligibility request. So halt everything!!!
			rs->Close();
			rs = CreateRecordset("SELECT [First] + ' ' + [Last] AS Name FROM PersonT WHERE ID = %li",m_pEligibilityInfo->nPatientID);
			if(!rs->eof){
				//give a HELPFUL error
				str.Format("Could not open eligibility information for patient '%s'.",AdoFldString(rs, "Name",""));
			}
			else
				//serious problems if you get here
				str = "Could not open eligibility information.";
			
			rs->Close();
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		//used to determine if we send EQ01 or EQ02
		BOOL bUseSpecificCode = FALSE;

		CString strCategoryCode = AdoFldString(rs, "CategoryCode","");
		CString strCPTCode = AdoFldString(rs, "Code","");

		bUseSpecificCode = !strCPTCode.IsEmpty();

//							DEPENDENT ELIGIBILITY OR BENEFIT INQUIRY INFORMATION

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP 2110D - DEPENDENT ELIGIBILITY OR BENEFIT INQUIRY INFORMATION		>1

//131		130		EQ		Eligibility or Benefit Inquiry			O		1

		//The specs state that you can send EQ01 or EQ02, but not both.
		//EQ01 is a service type, and 30 is a generic benefit inquiry.
		//EQ02 is a service code. The specs suggest that EQ01 codes other
		//than 30, and any EQ02 code, may be supported at the discretion
		//of the receiver. Translation: some people won't support it, some
		//will, and probably someone will require it. Wonderful.

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "EQ";

		//EQ01		1365	Service Type Code						X	ID	1/2

		str = "";

		//there are over 100 service type codes, refer to the specs for the full list,
		//but 30 is the standard to use (others may not even be supported)
		
		//30 - Health Benefit Plan Coverage (If only a single category of inquiry can be supported, use this code.)

		//if not requesting a specific code, then use 30 in EQ01
		if(!bUseSpecificCode) {
			str = strCategoryCode;
			if(str.IsEmpty())
				str = "30";
		}

		OutputString += ParseANSIField(str,1,2);

		//EQ02		C003	COMPOSITE MEDICAL PROCEDURE	IDENTIFIER	X

		if(bUseSpecificCode) {

		//EQ02 - 1	235		Product/Service ID Qualifier			M	ID	2/2

		//static "HC"
		CString strQual = "HC";

		// (j.jones 2013-01-25 12:25) - PLID 54853 - IsValidServiceCodeQualifier
		// needs updated any time we support exporting a new qualifier type,
		// currently we only support HC. This ensures that we never send other 
		// qualifiers without updating this function.
		if(!IsValidServiceCodeQualifier(strQual)) {
			//You just tried to send a qualifier not supported by IsValidServiceCodeQualifier,
			//Go into that function and add your new qualifier to the supported list.
			ASSERT(FALSE);
			ThrowNxException("Loop 2110D tried to send invalid service qualifier %s.", strQual);			
		}

		OutputString += ParseANSIField(strQual,2,2);

		//EQ02 - 2	234		Product/Service ID						M	AN	1/48

		//we loaded the CPT Code earlier
		str = strCPTCode;

		OutputString += ":";
		OutputString += str;

		//EQ02 - 3	1339	Procedure Modifier						O	AN	2/2

		//load modifier 1
		str = AdoFldString(rs, "Modifier1","");

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//EQ02 - 4	1339	Procedure Modifier						O	AN	2/2

		//load modifier 2
		str = AdoFldString(rs, "Modifier2","");

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//EQ02 - 5	1339	Procedure Modifier						O	AN	2/2

		//load modifier 3
		str = AdoFldString(rs, "Modifier3","");

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//EQ02 - 6	1339	Procedure Modifier						O	AN	2/2

		//load modifier 4
		str = AdoFldString(rs, "Modifier4","");

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//EQ02 - 7	NOT USED

		}
		else {
			//if not using EQ02, don't forget the separator
			OutputString += "*";
		}

		//EQ03		1207	Coverage Level Code						O	ID	3/3
		// (b.savon 2012-01-24 16:33) - PLID 47763 - Dont send EQ03 in 5010
		if (av5010 == m_avANSIVersion) 
		{
			OutputString += "*";
		}
		else {
			//"Use EQ03 when an information source supports or may be thought
			//to support the function of identifying benefits by the Benefit
			//Coverage Level Code. Use this code to identify the types and
			//number of entities that the request is to apply to. If not supported,
			//the information source will process without this data element."

			//CHD - Children Only
			//DEP - Dependents Only
			//ECH - Employee and Children
			//EMP - Employee Only
			//ESP - Employee and Spouse
			//FAM - Family
			//IND - Individual
			//SPC - Spouse and Children
			//SPO - Spouse Only

			//ignore for now
			OutputString += ParseANSIField("",3,3);
		}

		//EQ04		1336	Insurance Type Code						O	ID	1/3
		// (b.savon 2012-01-24 16:33) - PLID 47763 - Dont send EQ04 in 5010
		if (av5010 == m_avANSIVersion) 
		{
			OutputString += "*";
		}
		else {

			//"Use this code to identify the specific type of insurance the inquiry
			//applies to if the information source has multiple insurance lines
			//that apply to the person being inquired about. Do not use if the
			//insurance type can be determined either by the person’s identifiers
			//or the information source’s identifiers."

			//not sure if this is needed or not, very few categories exist
			
			// (j.jones 2008-09-09 11:13) - PLID 18695 - This is not entirely
			// 1-to-1 with the ANSI SBR09 codes, so do a manual conversion here,
			// even when it does happen to be the same.

			//the qualifiers for Medicare and Medicaid are not present in 2110D

			str = "";

			switch(m_pEligibilityInfo->eInsType) {
				case(itcCommercial):
					str = "C1";
					break;
				// (j.jones 2009-03-27 17:21) - PLID 33724 - supported supplemental types
				case(itcCommercialSupplemental):
					str = "SP";
					break;
				case(itcWorkersComp):
					str = "WC";
					break;
				case(itcHMO):
					str = "HM";
					break;
				case(itcAutomobileMedical):
					str = "AP";
					break;
				case(itcPPO):
					str = "PR";
					break;
				case(itcPOS):
					str = "PS";
					break;
				default:
					str = "";
					break;
			}

			OutputString += ParseANSIField(str,1,3);
		}

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//140		170		III		Subscriber Eligibility Or Benefit		O	10
//							Additional Inquiry Information

		//used to report diagnosis codes

		//Diagnosis 1

		OutputString = "III";

		//III01		1270	Code List Qualifier Code				X	ID	1/3

		// (c.haag 2010-10-14 11:15) - PLID 40352 - ANSI 5010 version has a different usage
		if (av5010 == m_avANSIVersion) {
			//static "ZZ" (mutually defined)
			OutputString += ParseANSIField("ZZ",1,3);
		} else {
			//static "BK" (principal diagnosis)
			//this is 4010, which doesn't support ICD-10
			OutputString += ParseANSIField("BK",1,3);
		}

		//III02		1271	Industry Code							X	AN	1/30

		//load the diagnosis 1
		// (c.haag 2010-10-14 16:53) - PLID 40352 - ANSI 5010 version has a different usage
		if (av5010 == m_avANSIVersion) {
			// Load the place of service code
			str = AdoFldString(rs, "PlaceCodes","");
		} else {
			//load the diagnosis 1
			// (j.jones 2014-03-13 10:49) - PLID 61363 - this now just reads from our array
			str = "";
			if(m_pEligibilityInfo->aryDiagCodes.size() > 0) {
				//get the first diagnosis code
				str = m_pEligibilityInfo->aryDiagCodes[0];
			}
		}
		
		str.Replace(".","");
		str.TrimRight();

		OutputString += ParseANSIField(str,1,30);

		//III03 NOT USED
		OutputString += "*";

		//III04 NOT USED
		OutputString += "*";

		//III05 NOT USED
		OutputString += "*";
		
		//III06 NOT USED
		OutputString += "*";

		//III07 NOT USED
		OutputString += "*";

		//III08 NOT USED
		OutputString += "*";

		//III09 NOT USED
		OutputString += "*";

		if(str != "")
			EndANSISegment(OutputString);

		// (c.haag 2010-10-14 16:53) - PLID 40352 - If using ANSI 5010, there's only one code
		// (Place of service designation), so we don't need to do more than that.
		if (av5010 == m_avANSIVersion) 
		{
		}
		else {
			//Diagnosis 2

			OutputString = "III";

			//III01		1270	Code List Qualifier Code				X	ID	1/3

			//static "BF" (diagnosis)
			//this is 4010, which doesn't support ICD-10
			OutputString += ParseANSIField("BF",1,3);

			//III02		1271	Industry Code							X	AN	1/30

			//load the diagnosis 2
			// (j.jones 2014-03-13 10:49) - PLID 61363 - this now just reads from our array
			str = "";
			if(m_pEligibilityInfo->aryDiagCodes.size() > 1) {
				//get the second diagnosis code
				str = m_pEligibilityInfo->aryDiagCodes[1];
				str.Replace(".","");
				str.TrimRight();
			}

			OutputString += ParseANSIField(str,1,30);

			//III03 NOT USED
			OutputString += "*";

			//III04 NOT USED
			OutputString += "*";

			//III05 NOT USED
			OutputString += "*";

			//III06 NOT USED
			OutputString += "*";

			//III07 NOT USED
			OutputString += "*";

			//III08 NOT USED
			OutputString += "*";

			//III09 NOT USED
			OutputString += "*";

			if(str != "")
				EndANSISegment(OutputString);

			//Diagnosis 3

			OutputString = "III";

			//III01		1270	Code List Qualifier Code				X	ID	1/3

			//static "BF" (diagnosis)
			//this is 4010, which doesn't support ICD-10
			OutputString += ParseANSIField("BF",1,3);

			//III02		1271	Industry Code							X	AN	1/30

			//load the diagnosis 3
			// (j.jones 2014-03-13 10:49) - PLID 61363 - this now just reads from our array
			str = "";
			if(m_pEligibilityInfo->aryDiagCodes.size() > 2) {
				//get the third diagnosis code
				str = m_pEligibilityInfo->aryDiagCodes[2];
				str.Replace(".","");
				str.TrimRight();
			}

			OutputString += ParseANSIField(str,1,30);

			//III03 NOT USED
			OutputString += "*";

			//III04 NOT USED
			OutputString += "*";

			//III05 NOT USED
			OutputString += "*";
			
			//III06 NOT USED
			OutputString += "*";

			//III07 NOT USED
			OutputString += "*";

			//III08 NOT USED
			OutputString += "*";

			//III09 NOT USED
			OutputString += "*";

			if(str != "")
				EndANSISegment(OutputString);

			//Diagnosis 4

			OutputString = "III";

			//III01		1270	Code List Qualifier Code				X	ID	1/3

			//static "BF" (diagnosis)
			//this is 4010, which doesn't support ICD-10
			OutputString += ParseANSIField("BF",1,3);

			//III02		1271	Industry Code							X	AN	1/30

			//load the diagnosis 4
			// (j.jones 2014-03-13 10:49) - PLID 61363 - this now just reads from our array
			str = "";
			if(m_pEligibilityInfo->aryDiagCodes.size() > 3) {
				//get the fourth diagnosis code
				str = m_pEligibilityInfo->aryDiagCodes[3];
				str.Replace(".","");
				str.TrimRight();
			}

			OutputString += ParseANSIField(str,1,30);

			//III03 NOT USED
			OutputString += "*";

			//III04 NOT USED
			OutputString += "*";

			//III05 NOT USED
			OutputString += "*";

			//III06 NOT USED
			OutputString += "*";

			//III07 NOT USED
			OutputString += "*";

			//III08 NOT USED
			OutputString += "*";

			//III09 NOT USED
			OutputString += "*";

			if(str != "")
				EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//143		190		REF		Dependent Additional Information		O	1

		//used to report referral or prior authorization numbers

		OutputString = "REF";

		//REF01		128		Reference Identification Qualifier		M	ID	2/3

		CString strID = "";
		CString strQual = "";

		//TODO: consider loading an insurance referral or prior auth num

		//9F - Referral Number
		//G1 - Prior Authorization Number

		OutputString += ParseANSIField(strQual,2,3);

		//REF02		127		Reference Identification				X	AN	1/30

		OutputString += ParseANSIField(strID,1,30);

		//REF03 NOT USED
		OutputString += "*";

		//REF04 NOT USED
		OutputString += "*";

		if(strID != "")
			EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//145		200		DTP		Dependent Eligibility/Benefit Date		O		9

		//only used to override dates in 2100D, at the moment unused

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in CEEligibility::ANSI_2110D");

	return Error_Other;
}

int CEEligibility::ANSI_Trailer() {

	//Trailer

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\nTRAILER\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							TRAILER

//147		210		SE		Transaction Set Trailer					R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "SE";		

		//SE01	96			Number Of Included Segments				M	N0	1/10

		//m_ANSISegmentCount
		str.Format("%li",m_ANSISegmentCount + 1);
		OutputString += ParseANSIField(str,1,10);

		//SE02	329			Transaction Set Control Number			M	AN	4/9

		//m_strBatchNumber
		OutputString += ParseANSIField(m_strBatchNumber,4,9,TRUE,'R','0');

		EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in CEEligibility::ANSI_Trailer");

	return Error_Other;
}

int CEEligibility::ANSI_InterchangeHeader() {

	//Interchange Control Header (page B.3)

	//The Interchange Control Header is one of the few records with a required length limit.
	//As a result, each element will be filled to a set size, even if the entire element is blank.
	//This will be the only time we really need to se bForceFill to TRUE in the ParseANSIField() function.

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\nInterchange Control Header\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

	try {

		//TES 7/1/2011 - PLID 40938 - We've already determined whether a record exists.
		if(!m_bEbillingFormatRecordExists) {
			//if the recordset is empty, there is no ebilling formats set up. So halt everything!!!
			AfxMessageBox("Could not open configuration information. Please go into the ANSI Properties and add an entry for your clearinghouse.");
			return Error_Missing_Info;
		}

		CString OutputString,str;
		_variant_t var;

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "ISA";

		//ISA01	I01			Authorization Information Qualifier		M	ID	2/2

		//TES 7/1/2011 - PLID 40938 - We've already loaded this value
		CString strISA01Qual = m_strISA01Qual;

		//EbillingFormatsT.ISA01Qual
		OutputString += ParseANSIField(strISA01Qual,2,2,TRUE);

		//ISA02	I02			Authorization Information				M	AN	10/10
		
		//(I01 and I02 are advised to be 00 and empty for now)
		//TES 7/1/2011 - PLID 40938 - We've already loaded this value
		CString strISA02 = m_strISA02;

		//EbillingFormatsT.ISA02
		OutputString += ParseANSIField(strISA02,10,10,TRUE);

		//ISA03	I03			Security Information Qualifier			M	ID	2/2

		//TES 7/1/2011 - PLID 40938 - We've already loaded this value
		CString strISA03Qual = m_strISA03Qual;

		//EbillingFormatsT.ISA03Qual
		OutputString += ParseANSIField(strISA03Qual,2,2,TRUE);

		//ISA04	I04			Security Information					M	AN	10/10

		//(I03 and I04 are advised to be 00 and empty for now)
		//TES 7/1/2011 - PLID 40938 - We've already loaded this value
		CString strISA04 = m_strISA04;

		//EbillingFormatsT.ISA04
		OutputString += ParseANSIField(strISA04,10,10,TRUE);

		//ISA05	I05			Interchange ID Qualifier				M	ID	2/2

		//default "ZZ" ("ZZ" is "Mutually Defined")
		//TES 7/1/2011 - PLID 40938 - We've already loaded this value
		CString strSubmitterQual = m_strSubmitterISA05Qual;
		OutputString += ParseANSIField(strSubmitterQual,2,2,TRUE);

		//ISA06	I06			Interchange Sender ID					M	AN	15/15

		//TES 7/1/2011 - PLID 40938 - We've already loaded this value
		CString strSubmitterID = m_strSubmitterISA06ID;

		//EbillingFormatsT.SubmitterISA06ID
		OutputString += ParseANSIField(strSubmitterID,15,15,TRUE);

		//ISA07	I05			Interchange ID Qualifier				M	ID	2/2

		//default "ZZ" ("ZZ" is "Mutually Defined")
		//TES 7/1/2011 - PLID 40938 - We've already loaded this value
		CString strReceiverQual = m_strReceiverISA07Qual;
		OutputString += ParseANSIField(strReceiverQual,2,2,TRUE);

		//ISA08	I07			Interchange Receiver ID					M	AN	15/15

		//TES 7/1/2011 - PLID 40938 - We've already loaded this value
		CString strReceiverID = m_strReceiverISA08ID;

		//EbillingFormatsT.ReceiverISA08ID
		str = strReceiverID;
		OutputString += ParseANSIField(str,15,15,TRUE);

		//ISA09	I08			Interchange Date						M	DT	6/6

		//current date YYMMDD
		COleDateTime dt = COleDateTime::GetCurrentTime();
		str = dt.Format("%y%m%d");
		OutputString += ParseANSIField(str,6,6,TRUE);

		//ISA10	I09			Interchange Time						M	TM	4/4

		//current time HHMM
		str = dt.Format("%H%M");
		OutputString += ParseANSIField(str,4,4,TRUE);

		//4010 - ISA11	I10			Interchange Control Standards Ident.	M	ID	1/1
		//5010 - ISA11	I65			Repetition Separator	M	1/1
		//static "U"
		// (c.haag 2010-10-14 11:15) - PLID 40352 - Now a Repetition Separator		
		if (av5010 == m_avANSIVersion) {
			//static "^", do not parse, just add raw
			OutputString += "*^";
		} else {
			OutputString += ParseANSIField("U",1,1,TRUE);
		}

		//ISA12	I11			Interchange Control Version Number		M	ID	5/5		

		//static "00401"
		// (c.haag 2010-10-14 11:15) - PLID 40352 - "00501" for ANSI 5010
		if (av5010 == m_avANSIVersion) {
			OutputString += ParseANSIField("00501",5,5,TRUE);
		}
		else {
			OutputString += ParseANSIField("00401",5,5,TRUE);
		}

		//ISA13	I12			Interchange Control Number				M	N0	9/9

		//m_strBatchNumber
		OutputString += ParseANSIField(m_strBatchNumber,9,9,TRUE,'R','0');

		//ISA14	I13			Acknowledgement Requested				M	ID	1/1

		//static "0" (set this to 1 if we want acknowledgement)
		OutputString += ParseANSIField("0",1,1,TRUE);

		//ISA15	I14			Usage Indicator							M	ID	1/1

		//Prod/Test

		//the eligibility test/production setting defaults to the ebilling setting
		long nEbillingProduction = GetRemotePropertyInt("EnvoyProduction", 0, 0, _T("<None>"), true);
		long nEligibilityProduction = GetRemotePropertyInt("EligibilityProduction", nEbillingProduction, 0, _T("<None>"), true);
		if (nEligibilityProduction == 1)
		{
			str = "P";	//Production
		}
		else {
			str = "T";	//Test
		}
		OutputString += ParseANSIField(str,1,1,TRUE);

		//ISA16	I15			Component Element Separator				M		1/1

		//static ":", do not parse, just add raw
		OutputString += "*:";
		
		EndANSISegment(OutputString);

	return Success;

	} NxCatchAll("Error in EEligibility::ANSI_InterchangeHeader");

	return Error_Other;

}

int CEEligibility::ANSI_InterchangeTrailer() {

	//Interchange Control Trailer (page B.7)

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\nInterchange Control Trailer\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "IEA";

		//IEA01	I16			Number Of Included Functional Groups	M	N0	1/5

		//JJ - I don't foresee us supporting multiple functional groups. I don't
		//quite know why they would be needed. If we do use them, however, we must
		//make this another member counter.

		//static "1"
		OutputString += ParseANSIField("1",1,5,TRUE,'R','0');

		//IEA02	I12			Interchange Control Number				M	N0	9/9
		
		//m_strBatchNumber
		OutputString += ParseANSIField(m_strBatchNumber,9,9,TRUE,'R','0');
		
		EndANSISegment(OutputString);

	return Success;

	} NxCatchAll("Error in EEligibility::ANSI_InterchangeTrailer");

	return Error_Other;

}

int CEEligibility::ANSI_FunctionalGroupHeader() {

	//Functional Group Header (page B.9)

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\nFunctional Group Header\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "GS";

		//GS01	479			Functional Identifier Code				M	ID	2/2

		//static "HS" for eligibility inquiry
		OutputString += ParseANSIField("HS",2,2);

		//GS02	142			Application Sender's Code				M	AN	2/15

		//TES 7/1/2011 - PLID 40938 - We've already loaded this value
		CString strSubmitterID = m_strSubmitterGS02ID;
		
		//EbillingFormatsT.SubmitterGS02ID
		OutputString += ParseANSIField(strSubmitterID,2,15);

		//GS03	124			Application Receiver's Code				M	AN	2/15

		//TES 7/1/2011 - PLID 40938 - We've already loaded this value
		CString strReceiverID = m_strReceiverGS03ID;
		
		//EbillingFormatsT.ReceiverGS03ID
		str = strReceiverID;
		OutputString += ParseANSIField(str,2,15);

		//GS04	373			Date									M	DT	8/8

		//current date CCYYMMDD
		COleDateTime dt = COleDateTime::GetCurrentTime();
		str = dt.Format("%Y%m%d");
		OutputString += ParseANSIField(str,8,8);

		//GS05	337			Time									M	TM	4/8

		//current time HHMM
		str = dt.Format("%H%M");
		OutputString += ParseANSIField(str,4,8);

		//GS06	28			Group Control Number					M	N0	1/9

		//m_strBatchNumber

		//in this instance, trim out leading zeros
		m_strBatchNumber.TrimLeft("0");

		OutputString += ParseANSIField(m_strBatchNumber,1,9);

		//GS07	455			Responsible Agency Code					M	ID	1/2

		//static "X"
		OutputString += ParseANSIField("X",1,2);

		//GS08	480			Version/Release/Industry Ident. Code	M	AN	1/12

		//004010X092A1 - Eligibility
		// (c.haag 2010-10-14 11:15) - PLID 40352 - "005010X279A1" for ANSI 5010
		if (av5010 == m_avANSIVersion) {
			str = "005010X279A1";
		}
		else {
			str = "004010X092A1";
		}
		
		OutputString += ParseANSIField(str,1,12);
		
		EndANSISegment(OutputString);

	return Success;

	} NxCatchAll("Error in EEligibility::ANSI_FunctionalGroupHeader");

	return Error_Other;

}

int CEEligibility::ANSI_FunctionalGroupTrailer() {

	//Functional Group Trailer (page B.10)

#if defined(_DEBUG) && defined (LAYMANS_DEBUGGING_HEADERS)

	// (j.jones 2010-07-02 10:11) - PLID 39486 - there is no file if using SOAP calls,
	// and we don't want to fill debug stuff in a SOAP call either
	if(!m_bUseRealTimeElig) {

		CString str;

		str = "\r\nFunctional Group Trailer\r\n";
		m_OutputFile.Write(str,str.GetLength());
	}
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "GE";

		//GE01	97			Number of Transaction Sets Included		M	N0	1/6

		//JJ - I don't foresee us supporting multiple transaction sets, though it's
		//probably more likely than using multiple functional groups.
		//If we do use them, however, we must make this another member counter.

		//static "1"
		OutputString += ParseANSIField("1",1,6);

		//GE02	28			Group Control Number					M	N0	1/9

		//m_strBatchNumber

		//in this instance, trim out leading zeros
		m_strBatchNumber.TrimLeft("0");

		OutputString += ParseANSIField(m_strBatchNumber,1,9);

		EndANSISegment(OutputString);

	return Success;

	} NxCatchAll("Error in EEligibility::ANSI_FunctionalGroupTrailer");

	return Error_Other;

}

// (j.jones 2007-06-27 16:55) - PLID 26480 - logs the export in MailSent
void CEEligibility::UpdateEligibilityHistory()
{
	try {

		CString strSqlBatch;
		BOOL bDeclarationsMade = FALSE;

		for(int i=0;i<m_aryEligibilityInfo.GetSize();i++) {
			
			m_pEligibilityInfo = ((EligibilityInfo*)m_aryEligibilityInfo.GetAt(i));

			//first update the request to use the current date as the last send date
			AddStatementToSqlBatch(strSqlBatch, "UPDATE EligibilityRequestsT SET LastSentDate = GetDate() WHERE ID = %li", m_pEligibilityInfo->nID);

			CString strNote;
			strNote.Format("Eligibility Request sent to Ebilling Clearinghouse (Insurance Company: %s)", m_pEligibilityInfo->strInsuranceCoName);

			//have we made our SQL declarations yet?
			if(!bDeclarationsMade) {
				AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewMailSentID int");
				AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewMailBatchID int");
				// (j.jones 2014-08-04 13:30) - PLID 63141 - we now track the patient ID too
				AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewMailSentIDsT TABLE (ID int, PatientID int)");

				// (j.dinatale 2012-02-06 10:35) - PLID 39756 - solve deadlocks!
				AddDeclarationToSqlBatch(strSqlBatch, "SELECT @nNewMailBatchID = COALESCE(MAX(MailBatchID), 0) + 1 FROM MailSent WITH (UPDLOCK, HOLDLOCK);");
				bDeclarationsMade = TRUE;
			}
			else {
				//increment the IDs
				AddStatementToSqlBatch(strSqlBatch, "SET @nNewMailBatchID = @nNewMailBatchID + 1");
			}

			// (j.jones 2008-09-04 15:15) - PLID 30288 - supported MailSentNotesT
			// (c.haag 2010-01-27 12:04) - PLID 36271 - Use GetDate() for the service date, not COleDateTime::GetCurrentTime()
			// (j.armen 2014-01-30 10:38) - PLID 55225 - Idenitate MailSent
			AddStatementToSqlBatch(strSqlBatch, 
				"INSERT INTO MailSent (MailBatchID, PersonID, Selection, Subject, PathName, Sender, [Date], Location, ServiceDate, InternalRefID, InternalTblName) "
				"VALUES (@nNewMailBatchID, %li, '%s', '', '%s', '%s', GetDate(), %li, GETDATE(), %li, 'EligibilityRequestsT')",
				m_pEligibilityInfo->nPatientID, _Q(SELECTION_FILE), _Q(PATHNAME_OBJECT_ELIGIBILITY_REQUEST), _Q(GetCurrentUserName()), m_pEligibilityInfo->nLocationID, m_pEligibilityInfo->nID);
			AddStatementToSqlBatch(strSqlBatch, "SET @nNewMailSentID = SCOPE_IDENTITY()");
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO MailSentNotesT (MailID, Note) VALUES (@nNewMailSentID, '%s')", _Q(strNote));
			// (j.jones 2014-08-04 13:30) - PLID 63141 - we now track the patient ID too
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO @NewMailSentIDsT (ID, PatientID) VALUES (@nNewMailSentID, %li)", m_pEligibilityInfo->nPatientID);
		}

		if(!strSqlBatch.IsEmpty()) {

			CString strFinalRecordset;
			strFinalRecordset.Format(
					"SET NOCOUNT ON \r\n"
					"BEGIN TRAN \r\n"
					"%s "
					"COMMIT TRAN \r\n"
					"SET NOCOUNT OFF \r\n"
					"SELECT ID, PatientID FROM @NewMailSentIDsT",
					strSqlBatch);

			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushPerformanceWarningLimit ppw(-1);
			_RecordsetPtr prsResults = CreateRecordsetStd(strFinalRecordset,
				adOpenForwardOnly, adLockReadOnly, adCmdText, adUseClient);
			while(!prsResults->eof) {
				//refresh the mailsent tablechecker
				long nID = AdoFldLong(prsResults, "ID",-1);
				long nPatientID = AdoFldLong(prsResults, "PatientID", -1);
				// (j.jones 2014-08-04 13:31) - PLID 63141 - this now sends an Ex tablechecker, we know IsPhoto is always false here
				CClient::RefreshMailSentTable(nPatientID, nID);
				prsResults->MoveNext();
			}
			prsResults->Close();
		}

	} NxCatchAll("Error updating eligibility histories.");
}


// (j.jones 2015-11-18 12:41) - PLID 67578 - generates a list of timed out patients
CString CEEligibility::GenerateTimeoutWarning(CArray<EligibilityInfo*, EligibilityInfo*> &aryTimeouts, bool bPromptForRetry)
{
	CString strWarn;

	if (aryTimeouts.GetSize() == 0) {
		//this function should not have been called
		ASSERT(FALSE);
		strWarn= "The eligibility requests were submitted, "
			"but the payer failed to respond in a timely fashion.\n\n";
	}
	else {
		CString strNames;
		//warn up to 20 names, one can assume that if 20 failed, they likely all failed
		for (int i = 0; i<aryTimeouts.GetSize() && i<21; i++) {
			EligibilityInfo *pInfo = ((EligibilityInfo*)aryTimeouts.GetAt(i));
			if (pInfo) {
				if (!strNames.IsEmpty()) {
					strNames += "\n";
				}
				if (i == 20) {
					strNames += "<More...>";
				}
				else {
					CString str;
					str.Format("%s (%s)", pInfo->strPatientName, pInfo->strInsuranceCoName);
					strNames += str;
				}
			}
		}

		if (!strNames.IsEmpty()) {
			strWarn.Format("The eligibility requests for the following patients were submitted, "
				"but the payer failed to respond in a timely fashion:\n\n"
				"%s\n\n",
				strNames);
		}
	}

	if (bPromptForRetry) {
		strWarn += "Would you like to retry sending these requests now?";
	}
	else {
		strWarn += "These requests have been batched for export at a later time. You can export these requests again in the E-Eligibility tab of the Financial Module.";
	}

	return strWarn;
}