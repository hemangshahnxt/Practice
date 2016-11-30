#include "stdafx.h"
#include "NxStandard.h"
#include "GlobalReportUtils.h"
#include "GlobalUtils.h"
#include "PracProps.h"
#include "Practice.h"
#include "MainFrm.h"
#include "NxReportJob.h"
//#include "ReportDocView.h"
#include "peplus.h"
#include "Reports.h"
#include "ChildFrm.h"
#include "ReportInfo.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "WhereClause.h"
#include "ReportInfo.h"
#include "EStatementPatientSelect.h"	// (j.dinatale 2011-03-21 10:18) - PLID 41444
#include "StatementSql.h"

using namespace ADODB;
using namespace NXDATALISTLib;

#define PAT_COMMA AdoFldLong(flds, "RespID", -1234) == -1234 ? AdoFldString(flds, "PatComma") : AdoFldString(flds, "RespLast", "") + ", " + AdoFldString(flds, "RespFirst", "") + " " + AdoFldString(flds, "RespMiddle", "") + " for " + AdoFldString(flds, "PatComma")

#define PAT_FORWARD AdoFldLong(flds, "RespID", -1234) == -1234 ?  AdoFldString(flds, "PatForward") : AdoFldString(flds, "RespFirst", "") + " " + AdoFldString(flds, "RespMiddle", "") + " " + AdoFldString(flds, "RespLast", "") + " for " + AdoFldString(flds, "PatForward") 

#define GUAR_FORWARD AdoFldLong(flds, "RespID", -1234) == -1234 ? (AdoFldString(flds, "PriGuarForward", "") == "") ? AdoFldString(flds, "SecGuarForward", "")  + " for " : AdoFldString(flds, "PriGuarForward")  + " for "   : AdoFldString(flds, "RespFirst", "") + " " + AdoFldString(flds, "RespMiddle", "") + " " + AdoFldString(flds, "RespLast", "") +    " for "

#define GUAR_COMMA AdoFldLong(flds, "RespID", -1234) == -1234 ? (AdoFldString(flds, "PriGuarComma", "") == "") ? AdoFldString(flds, "SecGuarComma", "")  + " for " : AdoFldString(flds, "PriGuarComma")  + " for " : AdoFldString(flds, "RespLast", "") + ", " + AdoFldString(flds, "RespFirst", "") + " " + AdoFldString(flds, "RespMiddle", "") +    " for "

// (j.gruber 2014-03-05 11:33) - PLID 61192 - not needed anymore
/*#define DIAG_1 strTemp.Find("1") != -1 ? AdoFldString(flds, "DiagCode", "") : "" 
#define DIAG_2 strTemp.Find("2") != -1 ? AdoFldString(flds, "DiagCode2", "") : "" 
#define DIAG_3 strTemp.Find("3") != -1 ? AdoFldString(flds, "DiagCode3", "") : "" 
#define DIAG_4 strTemp.Find("4") != -1 ? AdoFldString(flds, "DiagCode4", "") : "" */

#define CHARGE_INS nType == 10 ? AdoFldCurrency(flds, "Insurance") : cyZero
#define CHARGE_PAT nType == 10 ? AdoFldCurrency(flds, "Total") - AdoFldCurrency(flds, "Insurance") : cyZero
#define PAY_INS (nType == 1 || nType == 2 || nType == 3) && AdoFldCurrency(flds, "Insurance") != cyZero ? AdoFldCurrency(flds, "Insurance") : cyZero
#define PAY_PAT (nType == 1 || nType == 2 || nType == 3) && AdoFldCurrency(flds, "Insurance") == cyZero ? AdoFldCurrency(flds, "Total") : cyZero

#define SUPPRESS_BILL   (nCombineBalancesAfterXDays != 0 && ((dtNow - dtBill) > dtSpanDaysOld)) ? TRUE : FALSE

// (j.dinatale 2011-03-28 15:09) - PLID 43023 - Put opening file in its own function
bool DetermineSavePath(CString &strExportPath)
{
	// (j.armen 2011-10-25 14:05) - PLID 46134 - GetPracPath is prompting the user to save, so it is safe to use the practice path
	// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure. Also, changed to FileUtils::CreatePath() so that 
	// non-existent intermediate paths are created.
	FileUtils::CreatePath(GetEnvironmentDirectory() ^ "EStatements\\");

	CFileDialog SaveAs(FALSE,NULL,"Statements.txt");
	// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
	CString dir = GetEnvironmentDirectory() ^ "EStatements\\";
	SaveAs.m_ofn.lpstrInitialDir = (LPCSTR)dir;
	SaveAs.m_ofn.lpstrFilter = "Text Files\0*.txt\0All Files\0*.*\0\0";
	if (SaveAs.DoModal() == IDCANCEL) {
		//SaveAs.EndDialog(IDCANCEL);
		return false;
	}
	
	strExportPath = SaveAs.GetPathName();
	return true;
}

// (j.dinatale 2011-03-28 15:09) - PLID 43023 - filter generation
bool AppendFilters(CReportInfo *pReport)
{
	if (pReport->bUseFilter) {
		CString strFilter;
		if (GenerateFilter(pReport, &strFilter)) {
			if (! strFilter.IsEmpty()) {
				pReport->strFilterString = "(" + strFilter + ")";
			}
			else {
				if(AfxMessageBox("There are no patients that match this filter, would you like to run the report on all patients?", MB_YESNO) == IDNO)
					return false;
				//well they want to run you anyways, so i suppose we'll let it slide
				pReport->strFilterString = _T("");
			}
		} 
		else {
			return false;
		}
	}
	else {
		//resent thefilter string
		pReport->strFilterString = _T("");
	}

	return true;
}

// (j.dinatale 2011-03-29 09:45) - PLID 43023 - This will set up the parameters to run an individual report and return the recordset
// (c.haag 2016-03-21) - PLID 68251 - Since patient statement queries can have more than just SQL now, use a CComplexReportQuery object
_RecordsetPtr GetIndividualEStatement(CReportInfo *pReport, IN const CComplexReportQuery& patientStatementQuery)
{
	CString strTemp;

	// (j.dinatale 2011-03-21 15:05) - PLID 41444 - move this to be within in the if statement because the else below can have different outcomes
	// (j.jones 2011-04-13 10:02) - PLID 43258 - added BillID to the sort
	strTemp.Format("%s ORDER BY Last ASC, PatientID ASC, BillID ASC, ID Asc, Type DESC", patientStatementQuery.Flatten("Q"));

	CString strDateFrom = FormatDateTimeForSql(pReport->DateFrom, dtoDate);
	CString strDateTo = FormatDateTimeForSql(pReport->DateTo, dtoDate);						

	// (a.walling 2009-08-11 13:25) - PLID 35178 - Use the snapshot connection
	_CommandPtr pCmd = OpenParamQuery(GetRemoteDataSnapshot(), strTemp);

	// (j.gruber 2008-09-24 08:37) -  PLID 31485 - add in the timeout for good measure, 
	//although if an individual statement took > 47 seconds...that'd be one huge statement
	pCmd->CommandTimeout = 600;
	AddParameterLong(pCmd, "PatientID", pReport->nPatient);
	AddParameterString(pCmd, "DateFrom", strDateFrom);								
	AddParameterString(pCmd, "DateTo", strDateTo);								
	AddParameterLong(pCmd, "PatientID", pReport->nPatient);
	AddParameterString(pCmd, "DateFrom", strDateFrom);								
	AddParameterString(pCmd, "DateTo", strDateTo);								
	AddParameterLong(pCmd, "PatientID", pReport->nPatient);
	AddParameterString(pCmd, "DateFrom", strDateFrom);								
	AddParameterString(pCmd, "DateTo", strDateTo);								
	AddParameterLong(pCmd, "PatientID", pReport->nPatient);
	AddParameterString(pCmd, "DateFrom", strDateFrom);								
	AddParameterString(pCmd, "DateTo", strDateTo);
	AddParameterLong(pCmd, "PatientID", pReport->nPatient);
	AddParameterLong(pCmd, "PatientID", pReport->nPatient);
	AddParameterLong(pCmd, "PatientID", pReport->nPatient);
	AddParameterLong(pCmd, "PatientID", pReport->nPatient);
	AddParameterLong(pCmd, "PatientID", pReport->nPatient);
	AddParameterLong(pCmd, "PatientID", pReport->nPatient);
	AddParameterLong(pCmd, "PatientID", pReport->nPatient);
	AddParameterLong(pCmd, "PatientID", pReport->nPatient);
	AddParameterLong(pCmd, "PatientID", pReport->nPatient);
	AddParameterLong(pCmd, "PatientID", pReport->nPatient);
	AddParameterLong(pCmd, "PatientID", pReport->nPatient);
	AddParameterLong(pCmd, "PatientID", pReport->nPatient);

	return CreateRecordset(pCmd);
}

// (j.dinatale 2011-03-29 09:46) - PLID 43023 - Run all statements for the given query
// (c.haag 2016-03-21) - PLID 68251 - Since patient statement queries can have more than just SQL now, use a CComplexReportQuery object
_RecordsetPtr GetAllEStatements(IN const CComplexReportQuery& patientStatementQuery)
{
	CString strTemp;
	// (j.jones 2011-04-13 10:02) - PLID 43258 - added BillID to the sort
	// (c.haag 2016-03-21) - PLID 68251 - The CTE needs to go before the SELECT
	strTemp.Format("%s ORDER BY Last ASC, PatientID ASC, BillID ASC, ID Asc, Type DESC", patientStatementQuery.Flatten("Q"));

	// (a.walling 2009-08-11 13:25) - PLID 35178 - Use the snapshot connection
	return CreateRecordsetStd(GetRemoteDataSnapshot(), strTemp);
}

// (j.dinatale 2011-03-30 11:48) - PLID 42982 - This function now needs to show the E-Statements Dialog and thats it, the E-Statements dialog can
//		then handle running the E-Statements export
// (j.dinatale 2011-03-29 09:46) - PLID 43023 - Allow the user to choose what patients they want to run
// (c.haag 2016-03-21) - PLID 68251 - Since patient statement queries can have more than just SQL now, use a CComplexReportQuery object
// (c.haag 2016-05-19 14:15) - PLID-68687 - Deprecated in a larger effort to simply e-statement patient selection
/*bool ShowEStatementPatSel(IN const CComplexReportQuery& patientStatementQuery, OUT ADODB::_RecordsetPtr rs, CReportInfo *pReport, BOOL bSummary)
{
	if (GetMainFrame()->GetSafeHwnd()) {
		// (j.dinatale 2011-03-21 15:06) - PLID 41444 - otherwise, get all the PersonIDs for the patients, and put them in an array.
		//		Pass the array along to the EStatementPatientSelect dialog. If the user cancels, just return false, otherwise, get the
		//		list of selected personIDs and filter our big export query by it.

		// (a.walling 2013-08-09 09:59) - PLID 57948 - Always reload the EStatement patient selection info
		// (c.haag 2016-03-21) - PLID 68251 - The CTE needs to go before the SELECT
		CString strPatQuery;
		strPatQuery.Format("SET NOCOUNT OFF; %s SELECT DISTINCT PatID FROM (%s) Q ORDER BY PatID ASC", patientStatementQuery.m_strCTE, patientStatementQuery.m_strSQL);

		_RecordsetPtr rsTemp = CreateRecordsetStd(GetRemoteDataSnapshot(), strPatQuery);
		FieldsPtr flds = rsTemp->Fields;

		CArray<long, long> aryPatientIDs;

		while (!rsTemp->eof) {
			int nPatientID = AdoFldLong(flds, "PatID");
			aryPatientIDs.Add(nPatientID);
			rsTemp->MoveNext();
		}

		// (j.dinatale 2011-03-29 11:08) - PLID 42982 - Ask the Mainfrm to show our form
		GetMainFrame()->ShowEStatementsPatientSelect(pReport, bSummary, aryPatientIDs);

		return true;
	}

	return false;
}*/

// (j.dinatale 2011-03-29 09:46) - PLID 43023 - Now that we have the recordset, run through it and get the information
bool ProcessEStatements(_RecordsetPtr rs, CFile &OutFile, CReportInfo *pReport, BOOL bSummary, BOOL bIndiv)
{

	// (r.goldschmidt 2016-01-08 11:08) - PLID 67839 - is the e-statement a by location report
	bool bByLocation = (pReport->nID == 337 || pReport->nID == 338 || pReport->nID == 355 || pReport->nID == 356 || pReport->nID == 435 || pReport->nID == 437);
	bool bCheckedAllRecordsForLocation = false;

	CString strTemp;

	//Header Fields
	CString strDocName, strPracAddress1, strPracAddress2, strPracCity, strPracState, strPracZip, strPracPhone, 
		strPracFax, strPatName, strAddress1, strAddress2, strCity, strState, strZip, strTitle, 
		strRemitLocationName, strRemitAddress1, strRemitAddress2, strRemitCity, strRemitState, strRemitZip, 
		strPriInsCo, strSecInsCo, strNote, strNextAppt;

	COleDateTime dtPrintDate, dtNextApptDate, dtNextApptTime;
	COleCurrency cyPatTotResp, cyTotPatResp;
	BOOL bSuppressStatement = FALSE;
	long nPatientID = -1;
	long nLocationID = -1; // (r.goldschmidt 2016-01-07 15:45) - PLID 67839 - need to track location in case separating by location

	//Detail Fields
	COleDateTime dtDate;
	CString strTransProv, strDescription, strDiagCodes, strStar;
	COleCurrency cyChargeAmt, cyPayAmt;
	long nType;
	COleCurrency cyPatCharge, cyInsCharge, cyPatPay, cyInsPay;

	//Bill Fields
	COleCurrency cyBillBalance(0,0), cyChargeBalance(0,0), cyBillPatResp(0,0), cyChargePatResp(0,0);
	long nChargeID, nBillID;

	//Footer Fields
	COleCurrency cyThirty, cySixty, cyNinety, cyNinetyPlus, cyInsPending, cyAccountBal, cyTotalBalance;
	CString strPatientNote;

	//Output stuff
	CString strOutMain;

	//Group Stuff
	long nPrevPatientID, nPrevBillID, nPrevChargeID, nPrevLocationID; 

	// (r.goldschmidt 2016-01-07 15:45) - PLID 67839 - keep a map of locations and if they've been processed
	CMap<long, long, bool, bool> mapLocationIDs;

	//Misc
	COleCurrency cyZero(0,0);

	// (j.gruber 2010-12-01 10:38) - PLID 41540 - map for excluding patients and we need the PersonID
	CMap<long, long, long, long> mapExcludedPatientIDs;
	long nPersonID = -1; 

	// (j.gruber 2007-01-10 16:44) - PLID 24188 - Supress Certain bills over X days old
	COleCurrency cyBalanceForwardBalance(0,0), cyBalanceForwardPatient(0,0);
	long nDaysToCombine = GetRemotePropertyInt("SttmntDaysOld", 60, 0, "<None>");
	long nCombineBalancesAfterXDays = GetRemotePropertyInt("SttmntCombineBillsAfterXDaysOld", 0, 0, "<None>");
	COleDateTimeSpan dtSpanDaysOld(nDaysToCombine, 0,0,0);
	BOOL bHasBalanceForward = FALSE;
	COleDateTime dtNow, dtBill;
	dtNow = COleDateTime::GetCurrentTime();

	// (j.jones 2011-04-12 10:46) - PLID 43258 - added ability to show all charges
	// on any bills with balances, when on the summary statement
	BOOL bIncludePaidCharges = FALSE;
	if(bSummary) {
		bIncludePaidCharges = (GetRemotePropertyInt("SttmntIncludePaidCharges", 0, 0, "<None>") == 1);
	}

	//set up the parameters
	long nUseGuarantor = GetRemotePropertyInt("SttmntUseGuarantor");

	//Person's name that they can fill in, usually the patient coordinator
	CString strName = GetRemotePropertyMemo("SttmntName");

	//whether to use the Location name or the doctor's name
	long nUseDocName = GetRemotePropertyInt("SttmntUseDocName");

	//whether to use the Location address or the doctor's address
	long nUseDocAddress = GetRemotePropertyInt("SttmntUseDocAddress");

	//Title that gets appended to the person, normally patient coordinator
	CString strCallMe = GetRemotePropertyMemo( "SttmntCallMe");	

	//How to format the patient's name
	long nUseComma = GetRemotePropertyInt("SttmntUseComma");

	//the custom text for the statement, for all patients
	CString strText = GetRemotePropertyMemo("SttmntText",(CString)"-");

	//whether or not to show diagnosis codes
	long nShowDiags = GetRemotePropertyInt("SttmntShowDiag");

	//what age they want to use for the guarantor stuff
	long nAge = GetRemotePropertyInt("SttmntAge", 16);

	//Payment Description
	CString strPayDesc, strAdjDesc, strRefDesc;
	strPayDesc = GetRemotePropertyText("SttmntPaymentDesc", "", 0, "<None>");
	strAdjDesc = GetRemotePropertyText("SttmntAdjustmentDesc", "", 0, "<None>");
	strRefDesc = GetRemotePropertyText("SttmntRefundDesc", "", 0, "<None>");

	FieldsPtr flds;
	flds = rs->Fields;

	//we can now change the timeout back
	//g_ptrRemoteData->CommandTimeout = nOldTimeout;

	if (!rs->eof) {
		rs->MoveFirst();
		nPatientID = AdoFldLong(flds, "PatientID");
		nChargeID = AdoFldLong(flds, "ID");
		// (j.gruber 2010-12-01 14:49) - PLID 41540
		nPersonID = AdoFldLong(flds, "PatID");
		// (j.jones 2011-04-12 16:57) - PLID 43258 - load the bill ID
		nBillID = AdoFldLong(flds, "BillID", -1);
		nPrevPatientID = 0;
		nPrevChargeID = nChargeID;
		nPrevBillID = nBillID;
		// (r.goldschmidt 2016-01-07 15:53) - PLID 67839 - intialize the location settings
		nLocationID = AdoFldLong(flds, "LocID");
		nPrevLocationID = nLocationID;
		mapLocationIDs.SetAt(nLocationID, true);
	}

	//Get the fields
	while (!rs->eof) {

		//this code resets for each patient

		//PatientID
		nPatientID = AdoFldLong(flds, "PatientID");
		nChargeID = AdoFldLong(flds, "ID");
		// (j.gruber 2010-12-01 14:49) - PLID 41540
		nPersonID = AdoFldLong(flds, "PatID");
		// (j.jones 2011-04-12 16:57) - PLID 43258 - load the bill ID
		nBillID = AdoFldLong(flds, "BillID", -1);
		// (r.goldschmidt 2016-01-07 15:53) - PLID 67839 - load location ID
		nLocationID = AdoFldLong(flds, "LocID");

		//PracName
		if (nUseDocName == 0) {

			//use the PracName
			strDocName = AdoFldString(flds, "PracName", "");
		}
		else {
			//use the DocName
			strDocName = AdoFldString(flds, "DocName", "");
		}

		//Address 
		if (nUseDocAddress == 0) {

			//use the Practice's Address
			strPracAddress1 = AdoFldString(flds, "PracAddress1", "");
			strPracAddress2 = AdoFldString(flds, "PracAddress2", "");
			strPracCity = AdoFldString(flds, "PracCity", "");
			strPracState = AdoFldString(flds, "PracState", "");
			strPracZip = AdoFldString(flds, "PracZip", "");
		}
		else {
			//use the Doctor's Address
			strPracAddress1 = AdoFldString(flds, "DocAddress1", "");
			strPracAddress2 = AdoFldString(flds, "DocAddress2", "");
			strPracCity = AdoFldString(flds, "DocCity", "");
			strPracState = AdoFldString(flds, "DocState", "");
			strPracZip = AdoFldString(flds, "DocZip", "");
		}


		//Phone and Fax
		strPracPhone = AdoFldString(flds, "PracPhone", "");
		strPracFax = AdoFldString(flds, "PracFax", "");

		//Patient Name
		if (AdoFldLong(flds, "Age", -1) == -1) {
			if (nUseComma == 0) {
				strPatName = PAT_FORWARD;
				/*if (AdoFldLong(flds, "RespID", -1234) == -1234) {
				strPatName = AdoFldString(flds, "PatForward", "");
				}
				else {
				strPatName = AdoFldString(flds, "RespFirst", "") + " " + AdoFldString(flds, "RespMiddle", "") + " " + AdoFldString(flds, "RespLast", "");
				}*/
			}
			else {
				//PatComma
				strPatName = PAT_COMMA;
				/*if (AdoFldLong(flds, "RespID", -1234) == -1234) {
				strPatName = AdoFldString(flds, "PatComma", "");
				}
				else {
				strPatName = AdoFldString(flds, "RespLast", "") + ", " + AdoFldString(flds, "RespFirst", "") + " " + AdoFldString(flds, "RespMiddle", "");

				}*/
			}
		}
		else if (nUseGuarantor != 0) {
			if (AdoFldLong(flds, "Age", -1) <= nAge) {
				if (nUseComma == 0) {
					CString strTemp = GUAR_FORWARD;
					strTemp.TrimLeft();
					strTemp.TrimRight();
					if (strTemp.CompareNoCase("for") == 0) {
						strPatName = AdoFldString(flds, "PatForward", "");
					}
					else {
						strPatName.Format("%s %s", GUAR_FORWARD, AdoFldString(flds, "PatForward", ""));
					}
				}
				else {
					CString strTemp = GUAR_COMMA;
					strTemp.TrimLeft();
					strTemp.TrimRight();
					if (strTemp.CompareNoCase("for") == 0) {
						strPatName = AdoFldString(flds, "PatComma", "");
					}
					else {
						strPatName.Format("%s %s", GUAR_COMMA, AdoFldString(flds, "PatComma", ""));
					}
				}
			}
			else if (nUseComma == 0) {
				strPatName = PAT_FORWARD;					
			}
			else {
				strPatName = PAT_COMMA;
			}
		}
		else if (nUseComma == 0) {
			strPatName = PAT_FORWARD;
		}
		else {
			strPatName = PAT_COMMA;
		}


		//Patient Address
		if (AdoFldLong(flds, "RespID", -1234) == -1234) {
			strAddress1 = AdoFldString(flds, "Address1", "");
			strAddress2 = AdoFldString(flds, "Address2", "");
			strCity = AdoFldString(flds, "City", "");
			strState= AdoFldString(flds, "State", "");
			strZip= AdoFldString(flds, "Zip", "");
		}
		else {
			strAddress1 = AdoFldString(flds, "RespAdd1", "");
			strAddress2 = AdoFldString(flds, "RespAdd2", "");
			strCity = AdoFldString(flds, "RespCity", "");
			strState= AdoFldString(flds, "RespState", "");
			strZip= AdoFldString(flds, "RespZip", "");

		}

		//Print Date
		dtPrintDate = COleDateTime::GetCurrentTime();

		//Amount to pay from query
		// (j.gruber 2008-09-24 08:37) -  PLID 31485 - take out the bIndiv since it was never used before
		//if (bIndiv) {
		cyPatTotResp = COleCurrency(0,0);
		//}
		//else {
		//	cyPatTotResp = AdoFldCurrency(flds, "PatTotResp", cyZero);
		//}

		//Title
		strTitle = strCallMe +  " - " + strName;

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
				return FALSE;

			}
			else {

				strRemitLocationName = AdoFldString(rsLocation, "Name", "");
				strRemitAddress1 = AdoFldString(rsLocation, "Address1", "");
				strRemitAddress2 = AdoFldString(rsLocation, "Address2", "");
				strRemitAddress2.TrimLeft();
				strRemitAddress2.TrimRight();
				if (strRemitAddress2.IsEmpty()) {
					strRemitAddress2 = "";
				}

				strRemitCity= AdoFldString(rsLocation, "City", "");
				strRemitState= AdoFldString(rsLocation, "State", "");
				strRemitZip= AdoFldString(rsLocation, "Zip", "");
			}


		}
		else {

			if (nUseDocName == 0) {
				strRemitLocationName = AdoFldString(flds, "PracName", "");
			}
			else {
				strRemitLocationName = AdoFldString(flds, "DocName", "");
			}

			if (nUseDocAddress == 0) {

				//use the Practice's Address
				strRemitAddress1 = AdoFldString(flds, "PracAddress1", "");
				strRemitAddress2 = AdoFldString(flds, "PracAddress2", "");
				strRemitCity = AdoFldString(flds, "PracCity", "");
				strRemitState = AdoFldString(flds, "PracState", "");
				strRemitZip = AdoFldString(flds, "PracZip", "");
			}
			else {
				//use the Doctor's Address
				strRemitAddress1 = AdoFldString(flds, "DocAddress1", "");
				strRemitAddress2 = AdoFldString(flds, "DocAddress2", "");
				strRemitCity = AdoFldString(flds, "DocCity", "");
				strRemitState = AdoFldString(flds, "DocState", "");
				strRemitZip = AdoFldString(flds, "DocZip", "");
			}


		}

		//PriInsCo
		strPriInsCo = AdoFldString(flds, "PriInsCo", "");

		//Sec Ins Co
		strSecInsCo = AdoFldString(flds, "SecInsCo", "");


		//Next Appointment
		COleDateTime dtDefault;
		dtDefault.SetDate(1899,1,1);
		dtNextApptDate  = AdoFldDateTime(flds, "AppDate", dtDefault);
		dtNextApptTime = AdoFldDateTime(flds, "StartTime", dtDefault);

		// (j.gruber 2008-09-24 08:37) -  PLID 31485 - moved the supression variable up here to prevent eof/bof errors
		if (bIndiv) {
			bSuppressStatement = FALSE;
		}
		else if (AdoFldBool(flds, "SuppressStatement") == 0) {
			bSuppressStatement = FALSE;
		}
		else {
			bSuppressStatement = TRUE;
		}

		if (dtNextApptDate != dtDefault) {

			strNextAppt.Format("%s at %s", FormatDateTimeForInterface(dtNextApptDate, NULL, dtoDate, TRUE),
				FormatDateTimeForInterface(dtNextApptTime, NULL, dtoTime, TRUE));
		}
		else {
			strNextAppt = "None Scheduled";
		}

		strNote = strText;

		//write it to the file
		// (j.gruber 2008-09-24 08:37) -  PLID 31485 - take out the bIndiv since it was never used before
		//if (bIndiv) {
		strOutMain.Format(" PracName:           %s \r\n"
			" PracAdd1:           %s \r\n"
			" PracAdd2:           %s \r\n"
			" PracCity:           %s \r\n"
			" PracState:          %s \r\n"
			" PracZip:            %s \r\n"
			" PracPhone:          %s \r\n"
			" PracFax:            %s \r\n"
			" PatName:            %s \r\n"
			" PatAdd1:            %s \r\n"
			" PatAdd2:            %s \r\n"
			" PatCityStZip:       %s, %s %s \r\n"
			" PrintDate:          %s \r\n"
			" Title:              %s \r\n"
			" AmtToPay:           <AMOUNT_TO_PAY> \r\n"
			" PatID:              %li \r\n"
			" RemitLocName:       %s \r\n"
			" RemitAdd1:          %s \r\n"
			" RemitAdd2:          %s \r\n"
			" RemitCityStZip:     %s, %s %s \r\n"
			" PrimaryIns:         %s \r\n"
			" SecondaryIns:       %s \r\n"
			" NextAppt:           %s \r\n"
			" PracticeNote:       %s \r\n"
			" <BEGIN TRANSACTIONS> \r\n",
			strDocName, strPracAddress1, strPracAddress2, strPracCity, strPracState, strPracZip,
			strPracPhone, strPracFax, strPatName, strAddress1, strAddress2, strCity, strState, strZip,
			FormatDateTimeForInterface(dtPrintDate, NULL, dtoDate, FALSE), strTitle, 
			nPatientID, strRemitLocationName, strRemitAddress1, strRemitAddress2, strRemitCity, strRemitState, strRemitZip,
			strPriInsCo, strSecInsCo, strNextAppt, strNote);
		/*}
		else { 
		strOut.Format(" PracName:           %s \r\n"
		" PracAdd1:           %s \r\n"
		" PracAdd2:           %s \r\n"
		" PracCity:           %s \r\n"
		" PracState:          %s \r\n"
		" PracZip:            %s \r\n"
		" PracPhone:          %s \r\n"
		" PracFax:            %s \r\n"
		" PatName:            %s \r\n"
		" PatAdd1:            %s \r\n"
		" PatAdd2:            %s \r\n"
		" PatCityStZip:       %s, %s %s \r\n"
		" PrintDate:          %s \r\n"
		" Title:              %s \r\n"
		" AmtToPay:           %s \r\n"
		" PatID:              %li \r\n"
		" RemitLocName:       %s \r\n"
		" RemitAdd1:          %s \r\n"
		" RemitAdd2:          %s \r\n"
		" RemitCityStZip:     %s, %s %s \r\n"
		" PrimaryIns:         %s \r\n"
		" SecondaryIns:       %s \r\n"
		" NextAppt:           %s \r\n"
		" PracticeNote:       %s \r\n"
		" <BEGIN TRANSACTIONS> \r\n",
		strDocName, strPracAddress1, strPracAddress2, strPracCity, strPracState, strPracZip,
		strPracPhone, strPracFax, strPatName, strAddress1, strAddress2, strCity, strState, strZip,
		FormatDateTimeForInterface(dtPrintDate, NULL, dtoDate, FALSE), strTitle, FormatCurrencyForInterface(cyPatTotResp, TRUE, TRUE),
		nPatientID, strRemitLocationName, strRemitAddress1, strRemitAddress2, strRemitCity, strRemitState, strRemitZip,
		strPriInsCo, strSecInsCo, strNextAppt, strNote);
		}*/

		//Format The String
		CString strPatientToOutput = strOutMain;

		//do the footer struff that we can do now
		cyThirty = AdoFldCurrency(flds, "Thirty");
		cySixty = AdoFldCurrency(flds, "Sixty");
		cyNinety = AdoFldCurrency(flds, "Ninety");
		cyNinetyPlus = AdoFldCurrency(flds, "NinetyPlus");
		strPatientNote = AdoFldString(flds, "StatementNote");

		// (j.gruber 2007-01-09 09:41) - PLID 24168 - add last patient payment information 
		_variant_t varLastPayDate = flds->Item["LastPatientPaymentDate"]->Value;
		CString strLastPayDate, strLastPayAmt;
		if (varLastPayDate.vt == VT_DATE) {
			COleDateTime dtLastPay = VarDateTime(varLastPayDate);
			strLastPayDate = FormatDateTimeForInterface(dtLastPay, NULL, dtoDate, TRUE);
		}
		else {
			strLastPayDate = "";
		}
		_variant_t varLastPayAmt = flds->Item["LastPatientPaymentAmount"]->Value;
		if (varLastPayAmt.vt == VT_CY ) {
			COleCurrency cyLastPay = VarCurrency(varLastPayAmt);
			strLastPayAmt = FormatCurrencyForInterface(cyLastPay);
		}
		else {
			strLastPayAmt = "";
		}


		//cyInsPending = AdoFldCurrency(flds, "InsTotResp");
		//cyTotalBalance = cyInsPending + cyPatTotResp;


		nPrevPatientID = nPatientID;
		
		// (r.goldschmidt 2016-01-07 17:04) - PLID 67839 - if we are going by location, then the location needs to match too
		while (nPatientID == nPrevPatientID && (!rs->eof) && (!bByLocation || nLocationID == nPrevLocationID)) {

			//this loop resets for each *actual* BillsT entry,
			//which is not the same as the BEGIN BILL headers

			CString strBillToOutputDetailed = "";
			CString strBillToOutputSummary = "";

			dtBill = AdoFldDateTime(flds, "BillDate");

			cyBillBalance = cyZero;

			// (j.jones 2011-04-12 16:48) - PLID 43258 - need to group each bill together
			nPrevBillID = nBillID;
			// (r.goldschmidt 2016-01-07 17:04) - PLID 67839 - if we are going by location, then the location needs to match too
			while (nBillID == nPrevBillID && (!rs->eof) && (!bByLocation || nLocationID == nPrevLocationID)) {

				//this code resets for each charge, which counts its applies as part of the "charge",
				//ie. one "charge" includes each payment applied to it.
				//We output the BILL header and footer surrounding each "charge" and its applies, but
				//we will do this later in the code.

				/******
				BEGIN BILL / END BILL intentionally wraps around each charge (and its applies) so
				that we can have a balance line per charge, rather than per bill, which matches how
				we display charge balances in the modern 7.0 statements. It does NOT wrap around
				each actual "bill" as we know it in our system.
				******/

				cyChargeBalance = cyZero;
				cyChargePatResp = cyZero;

				CString strChargeToOutput = "";

				//on to the detail
				nPrevChargeID = nChargeID;
				// (r.goldschmidt 2016-01-07 17:04) - PLID 67839 - if we are going by location, then the location needs to match too
				while (nChargeID == nPrevChargeID && (!rs->eof) && (!bByLocation || nLocationID == nPrevLocationID)) {

					//this code resets for each line item associated with a charge,
					//meaning both the charge itself and each of its applied items

					CString strLineItem;

					//charge information
					nType = AdoFldLong(flds, "Type");

					//date
					dtDate = AdoFldDateTime(flds, "TDate");

					//star
					if (nType && AdoFldCurrency(flds, "Insurance") != cyZero) {
						strStar = "*";
					}
					else {
						strStar = "";
					}

					//Provider
					strTransProv = AdoFldString(flds, "TransProv", "");

					// (j.gruber 2010-06-15 10:38) - PLID 39168 - added billing notes to e-statement
					// (j.gruber 2010-08-20 09:46) - PLID 40193 - only use for e-statement if the option is checked.
					CString strBillingNotes;
					if (GetRemotePropertyInt("ShowBillingNotesOnEStatement", 0, 0, "<None>")) {
						strBillingNotes = AdoFldString(flds, "LineItemStatementNote", "");
						if (!strBillingNotes.IsEmpty()) {
							strBillingNotes = " Note: " + strBillingNotes;
						}
					}

					//description
					switch (nType) {
							case 1: 
								strDescription = strPayDesc + " " + AdoFldString(flds, "Description", "") + strBillingNotes;
								break;

							case 2:
								strDescription = strAdjDesc + " " + AdoFldString(flds, "Description", "") + strBillingNotes;
								break;

							case 3:
								strDescription = strRefDesc + " " + AdoFldString(flds, "Description", "") + strBillingNotes;
								break;

							default:
								strDescription = AdoFldString(flds, "Description", "") + strBillingNotes;
								break;
					}

					//Charge
					if (nType == 10) {
						cyChargeAmt = AdoFldCurrency(flds, "Total", cyZero);
					}
					else {
						cyChargeAmt = cyZero;
					}

					//Payment
					if (nType == 1 || nType == 2 || nType == 3) {
						cyPayAmt = AdoFldCurrency(flds, "Total", cyZero);
					}
					else {
						cyPayAmt = cyZero;
					}

					//Diag Codes
					// (j.gruber 2014-03-05 11:34) - PLID 61192 - updated for ICD10, now we are just going to show the string of both codes
					/*strTemp = AdoFldString(flds, "WhichCodes", "");
					if (strTemp.GetLength() > 1 && strTemp.Find(",") == -1) {
						//it is just the code itself 
						strDiagCodes = strTemp;
					}
					else if (strTemp == "2" ) {
						strDiagCodes = AdoFldString(flds, "DiagCode2", "");
					}
					else if (strTemp == "3") {
						strDiagCodes = AdoFldString(flds, "DiagCode3", "");
					}
					else if (strTemp == "4") {
						strDiagCodes = AdoFldString(flds, "DiagCode4", "");
					}
					else {
						strDiagCodes.Format(" %s %s %s %s ", DIAG_1, DIAG_2, DIAG_3, DIAG_4);
					}*/
					strDiagCodes = AdoFldString(flds, "WhichCodesBoth", "");

					strDiagCodes.TrimLeft();
					strDiagCodes.TrimRight();

					// (j.gruber 2014-03-05 11:35) - PLID 61192 - not needed 
					/*if (strDiagCodes.Left(1) == ",") {
						strDiagCodes = strDiagCodes.Right(strDiagCodes.GetLength() - 1);
					}*/

					if (nShowDiags) {
						//Write it to the output file
						strLineItem.Format(" <BEGIN LINEITEM> \r\n"
							" Date:               %s  \r\n"
							" *                   %s  \r\n"
							" Provider:           %s  \r\n"
							" Description:        %s  \r\n"
							" Charges:            %s  \r\n"
							" Credits:            %s  \r\n"
							" Diag.:              %s  \r\n"
							" <END LINEITEM> \r\n",
							FormatDateTimeForInterface(dtDate, NULL, dtoDate, TRUE), 
							strStar, strTransProv, strDescription, 
							FormatCurrencyForInterface(cyChargeAmt, TRUE, TRUE), FormatCurrencyForInterface(cyPayAmt, TRUE, TRUE),
							strDiagCodes);
					}
					else {
						strLineItem.Format(" <BEGIN LINEITEM> \r\n"
							" Date:               %s  \r\n"
							" *                   %s  \r\n"
							" Provider:           %s  \r\n"
							" Description:        %s  \r\n"
							" Charges:            %s  \r\n"
							" Credits:            %s  \r\n"
							" <END LINEITEM> \r\n",
							FormatDateTimeForInterface(dtDate, NULL, dtoDate, TRUE), 
							strStar, strTransProv, strDescription, 
							FormatCurrencyForInterface(cyChargeAmt, TRUE, TRUE), FormatCurrencyForInterface(cyPayAmt, TRUE, TRUE));

					}

					//add the bill information
					cyInsCharge = CHARGE_INS;
					cyPatCharge = CHARGE_PAT;
					cyInsPay = PAY_INS;
					cyPatPay = PAY_PAT;

					// (j.jones 2011-04-12 16:58) - PLID 43258 - add up the balance					
					COleCurrency cyCurBalance = ((cyPatCharge - cyPatPay) + (cyInsCharge - cyInsPay));

					cyBillBalance += cyCurBalance;
					cyChargeBalance += cyCurBalance;
					cyBillPatResp += (cyPatCharge - cyPatPay);
					cyChargePatResp += (cyPatCharge - cyPatPay);

					if (! SUPPRESS_BILL) {
						// (j.jones 2011-04-13 10:28) - PLID 43258 - track this
						// line item for the "charge", when we show or hide a charge
						// via detailed vs. summary, we show/hide all its applies too
						strChargeToOutput += strLineItem;
					}

					rs->MoveNext();

					// (r.goldschmidt 2016-01-07 16:22) - PLID 67839 - Advance through recordset according to by location
					//
					// Assume that the query is already sorted by patient/bill/charge because of this:
					// "SELECT * FROM (%s) Q ORDER BY Last ASC, PatientID ASC, BillID ASC, ID Asc, Type DESC" (ID is essentially LineItemT.ID)
					// [see GetIndividualEStatement(), GetAllEStatements(), etc]
					//
					if (bByLocation) {
						bool bIsLocationAdded;
						long nTempNextLocationID = nLocationID;
						// set temporary next location to next record's location
						if (!rs->eof) {
							nTempNextLocationID = AdoFldLong(flds, "LocID");
						}
						// if the next location is different from the current location
						//  1) add to the map of locations as unprocessed if it isn't there
						//  2) advance to the next record and check locations again
						while ((!rs->eof) && (nTempNextLocationID != nLocationID)) {
							if (!bCheckedAllRecordsForLocation || !mapLocationIDs.Lookup(nTempNextLocationID, bIsLocationAdded)) {
								mapLocationIDs.SetAt(nTempNextLocationID, false);
							}
							rs->MoveNext();
							if (!rs->eof) {
								nTempNextLocationID = AdoFldLong(flds, "LocID");
							}
						}

						// if eof is reached
						if (rs->eof){
							// assume all locations have been added to map
							bCheckedAllRecordsForLocation = true;

							// find out if all the locations in the map have been processed
							bool bAllLocationsProcessed = true;
							POSITION position = mapLocationIDs.GetStartPosition();
							long nHolder;
							while (bAllLocationsProcessed && position != NULL) {
								mapLocationIDs.GetNextAssoc(position, nHolder, bAllLocationsProcessed);
							}

							// if all locations in the map have not been processed
							//  1) move back to start of recordset
							//  2) advance to first record for a location that hasn't been processed yet
							//  3) mark the selected unprocessed location as processed/in process
							if (!bAllLocationsProcessed) {
								nTempNextLocationID = nLocationID;
								rs->MoveFirst();
								if (!rs->eof) {
									nTempNextLocationID = AdoFldLong(flds, "LocID");
								}
								while ((!rs->eof) && mapLocationIDs.Lookup(nTempNextLocationID, bIsLocationAdded) && bIsLocationAdded) {
									rs->MoveNext();
									if (!rs->eof) {
										nTempNextLocationID = AdoFldLong(flds, "LocID");
									}
								}
								mapLocationIDs.SetAt(nTempNextLocationID, true);

							}
						}
					}

					nPrevLocationID = nLocationID;
					if (!rs->eof) {
						nLocationID = AdoFldLong(flds, "locID");
					}

					nPrevChargeID = nChargeID;
					if (! rs->eof) {
						nChargeID = AdoFldLong(flds, "ID");
					}

					nPrevBillID = nBillID;
					if(!rs->eof) {
						nBillID = AdoFldLong(flds, "BillID", -1);
					}

					nPrevPatientID = nPatientID;
					if (!rs->eof) {
						nPatientID = AdoFldLong(flds, "PatientID");
					}
				}
				//done with this charge (and all its applies)

				if(!strChargeToOutput.IsEmpty()) {

					//write the "bill" information (remember this wraps around each "charge" and its applies)

					//output the Bill header, this is actually sent per charge (surrounding its applies)
					CString strBillHeader;
					strBillHeader.Format(" <BEGIN BILL> \r\n");

					CString strBillFooter;
					strBillFooter.Format(" Balance:            %s \r\n"
						" Pt. Resp:           %s \r\n"
						" <END BILL> \r\n",
						FormatCurrencyForInterface(cyChargeBalance, TRUE, TRUE),
						FormatCurrencyForInterface(cyChargePatResp, TRUE, TRUE));

					if (! SUPPRESS_BILL) {
						// (j.jones 2011-04-13 10:31) - PLID 43258 - in this context,
						// detailed vs. summary simply means whether we show $0.00 balance
						// charges, the decision for which to use is made later
						if(cyChargeBalance != cyZero) {
							strBillToOutputSummary += (strBillHeader + strChargeToOutput + strBillFooter);
						}
						strBillToOutputDetailed += (strBillHeader + strChargeToOutput + strBillFooter);
					}
				}
			}
			//done with this bill

			if (SUPPRESS_BILL) {
				cyBalanceForwardBalance += cyBillBalance;
				cyBalanceForwardPatient += cyBillPatResp;
				bHasBalanceForward = TRUE;
			}
			else {

				// (j.jones 2011-04-12 17:34) - PLID 43258 - we now have a string
				// of all charges on this bill, or just charges with a balance,
				// so which should we use?
				if (!bSummary) {
					strPatientToOutput += strBillToOutputDetailed;
				}
				else {
					if(cyBillBalance != cyZero) {
						// (j.jones 2011-04-12 17:34) - PLID 43258 - bIncludePaidCharges
						// tells us if we should include all charges on a bill or not
						if(bIncludePaidCharges) {
							strPatientToOutput += strBillToOutputDetailed;
						}
						else {
							strPatientToOutput += strBillToOutputSummary;
						}
					}
				}
			}
			cyTotPatResp += cyBillPatResp;
			cyTotalBalance += cyBillBalance;
			cyBillBalance = cyZero;
			cyBillPatResp = cyZero;
		}
		//done with this patient

		// (j.gruber 2007-01-10 17:21) - PLID 24188 - print out the balance if we need to
		if (nCombineBalancesAfterXDays != 0) {
			//see if we have anything to output
			CString strOutBill;
			strOutBill.Format(" <BEGIN BILL>\r\n"
				" <BEGIN LINEITEM> \r\n"
				" Date:                 \r\n"
				" *                     \r\n"
				" Provider:             \r\n"
				" Description:        Balance Before %li Days Ago  \r\n"
				" Charges:              \r\n"
				" Credits:              \r\n"
				" <END LINEITEM> \r\n"
				" Balance:            %s \r\n"
				" Pt. Resp:           %s \r\n"
				" <END BILL> \r\n",
				nDaysToCombine, FormatCurrencyForInterface(cyBalanceForwardBalance, TRUE, TRUE),
				FormatCurrencyForInterface(cyBalanceForwardPatient, TRUE, TRUE));
			if (bHasBalanceForward) {
				if (!bSummary) {
					strPatientToOutput += strOutBill;						
				}
				else {

					if (cyBalanceForwardBalance != cyZero || cyBalanceForwardPatient != cyZero) {
						strPatientToOutput += strOutBill;
					}
				}
			}
		}

		//we are out of the patient, so do the footer



		//footer

		cyInsPending = cyTotalBalance - cyTotPatResp;

		// (j.gruber 2007-01-09 09:41) - PLID 24168 - add last patientpayment information if they want it
		if (GetRemotePropertyInt("SttmntShowLastPayInfo", 0, 0, "<None>")) {

			strOutMain.Format(" <END TRANSACTIONS> \r\n"
				" 0-30 Days:          %s  \r\n"
				" 30-60 Days:         %s \r\n"
				" 60-90 Days:         %s  \r\n"
				" 90+ Days:           %s  \r\n"
				" Due From Patient:   %s \r\n"
				" Insurance Pending:  %s \r\n"
				" Account Balance:    %s   \r\n"
				" * indicates the item was billed to your insurance company \r\n"
				" PatStatementNote:   %s \r\n"
				" LastPatPayDate:     %s \r\n"
				" LastPatPayAmount:   %s \r\n"
				" <END STATEMENT> \r\n",
				FormatCurrencyForInterface(cyThirty, TRUE, TRUE),
				FormatCurrencyForInterface(cySixty, TRUE, TRUE),
				FormatCurrencyForInterface(cyNinety, TRUE, TRUE),
				FormatCurrencyForInterface(cyNinetyPlus, TRUE, TRUE),
				FormatCurrencyForInterface(cyTotPatResp, TRUE, TRUE),
				FormatCurrencyForInterface(cyInsPending, TRUE, TRUE),
				FormatCurrencyForInterface(cyTotalBalance, TRUE, TRUE),
				strPatientNote, strLastPayDate, strLastPayAmt);
		}
		else {


			strOutMain.Format(" <END TRANSACTIONS> \r\n"
				" 0-30 Days:          %s  \r\n"
				" 30-60 Days:         %s \r\n"
				" 60-90 Days:         %s  \r\n"
				" 90+ Days:           %s  \r\n"
				" Due From Patient:   %s \r\n"
				" Insurance Pending:  %s \r\n"
				" Account Balance:    %s   \r\n"
				" * indicates the item was billed to your insurance company \r\n"
				" PatStatementNote:   %s \r\n"
				" <END STATEMENT> \r\n",
				FormatCurrencyForInterface(cyThirty, TRUE, TRUE),
				FormatCurrencyForInterface(cySixty, TRUE, TRUE),
				FormatCurrencyForInterface(cyNinety, TRUE, TRUE),
				FormatCurrencyForInterface(cyNinetyPlus, TRUE, TRUE),
				FormatCurrencyForInterface(cyTotPatResp, TRUE, TRUE),
				FormatCurrencyForInterface(cyInsPending, TRUE, TRUE),
				FormatCurrencyForInterface(cyTotalBalance, TRUE, TRUE),
				strPatientNote);
		}

		if (! bSummary) {
			// (j.gruber 2008-09-24 08:37) -  PLID 31485 - take out the bIndiv since it was never used before
			//if (bIndiv) {
			strPatientToOutput.Replace("<AMOUNT_TO_PAY>", FormatCurrencyForInterface(cyTotPatResp, TRUE, TRUE));
			//}				
			strPatientToOutput += strOutMain;
			// (j.gruber 2008-09-24 08:37) -  PLID 31485 -  changed this because it threw errors because the recordset was already eof
			if (! bSuppressStatement) {
				OutFile.Write((LPCTSTR)strPatientToOutput, strPatientToOutput.GetLength());
			}
			else {
				// (j.gruber 2010-12-01 10:24) - PLID 41540 - this patient isn't getting a statement
				//exclude them from write to history
				mapExcludedPatientIDs.SetAt(nPersonID, nPersonID);
			}


		}
		else {
			// (j.gruber 2008-09-24 08:37) -  PLID 31485 - take out the bIndiv because is was never used before
			//if (bIndiv) {
			strPatientToOutput.Replace("<AMOUNT_TO_PAY>", FormatCurrencyForInterface(cyTotPatResp, TRUE, TRUE));
			//}
			// (j.gruber 2010-09-07 13:34) - PLID 38011 - instead of checking total balance, 
			//check both the insurance and patient because if either are not 0, we want to show it
			//just in case the patient or insurance has a balance, but the sum of those balances is 0

			if (cyInsPending != cyZero || cyTotPatResp != cyZero) {
				strPatientToOutput += strOutMain;
				// (j.gruber 2008-09-24 08:37) -  PLID 31485 -  changed this because it threw errors because the recordset was already eof
				if (!bSuppressStatement) {
					OutFile.Write((LPCTSTR)strPatientToOutput, strPatientToOutput.GetLength());
				}
				else {
					// (j.gruber 2010-12-01 10:24) - PLID 41540 - this patient isn't getting a statement
					//exclude them from write to history
					mapExcludedPatientIDs.SetAt(nPersonID, nPersonID);
				}
			}
			else {				
				// (j.gruber 2010-12-01 10:24) - PLID 41540 - this patient isn't getting a statement
				//exclude them from write to history
				mapExcludedPatientIDs.SetAt(nPersonID, nPersonID);
			}
		}


		cyTotPatResp = cyZero;
		cyInsPending = cyZero;
		cyTotalBalance = cyZero;
		strPatientToOutput = "";

		// (j.gruber 2007-01-11 11:15) - PLID 24188 - clear the variables
		cyBalanceForwardBalance = cyZero;
		cyBalanceForwardPatient = cyZero;
		bHasBalanceForward = FALSE;

		nPrevBillID = nBillID;
		nPrevChargeID = nChargeID;
		nPrevPatientID = nPatientID;
		nPrevLocationID = nLocationID; // (r.goldschmidt 2016-01-08 09:41) - PLID 67839
		if (! rs->eof) {
			//rs->MoveNext();	
		}

	}

	//write it to history
	// (j.gruber 2010-12-01 10:27) - PLID 41540 - we need to run an extra recordset here 
	//so that we aren't writing to history
	// (r.gonet 12/18/2012) - PLID 53629 - The statement output types might have changed
	//  while the report was running. Fortunately, we have them saved away from when the
	// report started.
	// (r.goldschmidt 2014-08-05 14:43) - PLID 62717 - category type also
	CReportInfo::WriteToHistory(pReport->nID, 0,0, rs, CReportInfo::NXR_RUN, &mapExcludedPatientIDs, pReport->nStatementType, pReport->nOutputType, pReport->nCategoryType);

	return true;
}

// (j.dinatale 2011-03-29 09:46) - PLID 43023 - Refactored this function to be cleaner, it was about 1000 lines and it was getting quite hard to understand
BOOL RunEStatements(CReportInfo *pReport, BOOL bSummary, BOOL bIndiv) {

	try {
		CWaitCursor cwait;

		// (j.dinatale 2011-03-24 11:37) - PLID 41444 - Cache 'StatementsShowEStatementsPatientSelect'
		// (j.gruber 2010-08-20 09:51) - PLID 40193 - cache the new property
		g_propManager.CachePropertiesInBulk("StatementSetup", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name =  'ShowBillingNotesOnEStatement' OR "	
			"Name = 'StatementsShowEStatementsPatientSelect' OR "
			// (j.jones 2011-04-12 10:46) - PLID 43258 - added ability to show all charges
			// on any bills with balances, when on the summary statement
			"Name = 'SttmntIncludePaidCharges' "
			")",
			_Q(GetCurrentUserName()));

		// (j.dinatale 2011-03-29 09:46) - PLID 43023 - append the filters to the report using the new utility
		//check for the filter
		if(!AppendFilters(pReport)){
			return false;
		}

		CStatementSqlBuilder statementSqlBuilder(GetRemoteDataSnapshot(), pReport, 0, 0, TRUE);
		// (c.haag 2016-03-21) - PLID 68251 - We now use a patient statement query object
		CComplexReportQuery ptStmtQuery = statementSqlBuilder.GetStatementSql();

		//DRT 10/6/2004 - PLID 14325 - We need to up the timeout here, like we do when we run any report.

		// (a.walling 2009-08-11 13:25) - PLID 35178 - Use the snapshot connection
		CIncreaseCommandTimeout cict(GetRemoteDataSnapshot(), 600);
		//long nOldTimeout = g_ptrRemoteData->CommandTimeout;
		//g_ptrRemoteData->CommandTimeout = 600;

		// (j.dinatale 2011-03-29 09:46) - PLID 43023 - tweaked the if else tree to use the utility functions instead
		// (j.gruber 2008-06-03 11:20) - PLID 29408 - fix the individual e-statement, it wasn't changed when we parameterized the query
		_RecordsetPtr rs;		
		if (bIndiv) {
			// (c.haag 2016-03-21) - PLID 68251 - Use CComplexReportQuery
			rs = GetIndividualEStatement(pReport, ptStmtQuery);
		}
		else {
			// (j.dinatale 2011-03-21 15:05) - PLID 41444 - if this is not an individual report, we want to see if they want to select which patients get
			//		exported
			long nShowPatSel = GetRemotePropertyInt("StatementsShowEStatementsPatientSelect", 0, 0, "<None>");

			if(nShowPatSel == 0){
				// (c.haag 2016-03-21) - PLID 68251 - Use CComplexReportQuery
				rs = GetAllEStatements(ptStmtQuery);
			}else{
				// (j.dinatale 2011-03-30 11:50) - PLID 42982 - at this point, we should just try and show the dialog
				//		and return true, since the dialog is assumed to have been shown.
				// (c.haag 2016-05-19 14:15) - PLID-68687 - Just call the MainFrm version and include the generated SQL
				GetMainFrame()->ShowEStatementsPatientSelect(pReport, bSummary, ptStmtQuery);
				return true;
			}			
		}

		// (j.dinatale 2011-03-29 16:14) - PLID 42982 - Need to do saving after we determine what we are doing in this function
		// (j.dinatale 2011-03-29 09:46) - PLID 43023 - Determine the save path with the new utility
		//first, figure out where they want to save it
		CString strExportPath;
		if(!DetermineSavePath(strExportPath)){
			return false;
		}

		CFile OutFile(strExportPath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite);

		// (j.dinatale 2011-03-29 09:46) - PLID 43023 - Process our EStatements
		return ProcessEStatements(rs, OutFile, pReport, bSummary, bIndiv);

	}NxCatchAll("Error Exporting Statement");
	return TRUE;
}

long GetStatementType(long nReportID) {

	switch (nReportID) {

		case 169:
		case 234:
		case 353:
		case 354:
		case 434:
		case 436:
			return BYPATIENT;
		break;
		case 337:
		case 338:
		case 355:
		case 356:
		case 435:
		case 437:
			return BYLOCATION;
		break;
		case 483:
		case 484:
		case 485:
		case 486:
			return BYPROVIDER;
		break;

		default:
			return -1;
		break;
	}
}

// (j.gruber 2010-03-11 12:39) - PLID 29120 - added parameter
void SetReportList(_DNxDataListPtr pDataList, long nReportID, CReportInfo * pReport, BOOL bResetSelection) {

	try {

		//first, get the report name that is already in the list
		CString strSelectedFileName;
		if (pReport) {
			//get the file name
			//check whether to reset
			if (!bResetSelection) {
				strSelectedFileName = pReport->strReportFile;
			}
		}

		//now, clear out the list
		pDataList->Clear();

		long nStmtType = GetStatementType(nReportID);

		//first see what is checked
		CString strReportName;
		if (nStmtType == BYPROVIDER) {
			strReportName = "StatementProv";
		}
		else if (nStmtType == BYLOCATION) {
			nStmtType = BYLOCATION;
			strReportName = "StatementLoc";
		}
		else {
			strReportName = "Statement";
		}

		BOOL bUse70 = GetRemotePropertyInt("SttmntUse70Version");

		//change the report ID is we need to
		if (nReportID == 169 || nReportID == 337 || nReportID == 434 || nReportID == 435 || nReportID == 483 || nReportID == 484) {
			//check to see if they are using the 7.0 reports
			if (bUse70) {
				switch(nReportID) {
					case 169:
						nReportID = 353;
					break;
					case 337:
						nReportID = 355;
					break;
					case 434:
						nReportID = 436;
						break;
					case 435:
						nReportID = 437;
						break;
					case 483:
						nReportID = 485;
						break;
					case 484:
						nReportID = 486;
						break;
					default:
						ASSERT(FALSE);
					break;
				
				}
				
			}
		}

		if (bUse70) {
			//add the 70 to the name
			strReportName += "70";
		}
			
		AddToReportList(strReportName, nReportID, pDataList, strSelectedFileName);

		//change the report name to be the currently selected item
		if (pDataList->CurSel != -1) {
			if (pReport) {
				pReport->strReportFile = VarString(pDataList->GetValue(pDataList->CurSel, 3));
			}
		}
		else {
			if (pReport) {
				pReport->strReportFile = "";
			}
		}

		if (pDataList->CurSel != -1) {
			if (VarLong(pDataList->GetValue(pDataList->CurSel, 1)) > 0) {
				if (pReport) {
					//its a custom report
					pReport->nDefaultCustomReport = VarLong(pDataList->GetValue(pDataList->CurSel, 1));
				}
			}
			else {
				if (pReport) {
					//(e.lally 2006-11-14) PLID 23549 - For the e-statements, it is possible for this
					//to get reset to zero, but then not set again. From what I can tell, none of the statements
					//actually depend on the zero value so we can just set it to the value of the current selection.
					pReport->nDefaultCustomReport = VarLong(pDataList->GetValue(pDataList->CurSel, 1));
				}
			}
		}
	}NxCatchAll("Error In SetReportList");

}

void AddToReportList(CString strReportName, long nReportID, _DNxDataListPtr pDataList, CString strSelectedFileName) {

	try {
		long nCount = 0;
		long nFormatType = GetRemotePropertyInt("SttmntEnvelope", 1, 0, "<None>");
		IRowSettingsPtr pRow;

		switch (nFormatType) {

			case 0: //avery 1

				//we just need detailed or summary here
				pRow = pDataList->GetRow(-1);
				pRow->PutValue(0, (long) 0);
				pRow->PutValue(1, (long) -1);
				pRow->PutValue(2, _variant_t("Detailed"));
				pRow->PutValue(3, _variant_t(strReportName + "dtldavery1"));
				pDataList->AddRow(pRow);

				pRow = pDataList->GetRow(-1);
				pRow->PutValue(0, (long) 1);
				pRow->PutValue(1, (long) -2);
				pRow->PutValue(2, _variant_t("Summary"));
				pRow->PutValue(3, _variant_t(strReportName + "smryavery1"));
				pDataList->AddRow(pRow);

				//set the current selection to be detailed if they don't have anything already
				if (pDataList->SetSelByColumn(3, _variant_t(strSelectedFileName)) == -1) {
					
					// (j.gruber 2010-02-04 13:03) - PLID 29120 - check whether it should be detailed or summary
					if (GetRemotePropertyInt("SttmntDefaultDetailed", 1, 0, "<None>") == 1) { 
						pDataList->SetSelByColumn(0, (long)0);			
					}
					else {
						pDataList->SetSelByColumn(0, (long)1);			
					}

				}

				break;

			case 1: //avery2

				//we just need detailed or summary here
				pRow = pDataList->GetRow(-1);
				pRow->PutValue(0, (long) 0);
				pRow->PutValue(1, (long) -1);
				pRow->PutValue(2, _variant_t("Detailed"));
				pRow->PutValue(3, _variant_t(strReportName + "dtldavery2"));
				pDataList->AddRow(pRow);

				pRow = pDataList->GetRow(-1);
				pRow->PutValue(0, (long) 1);
				pRow->PutValue(1, (long) -2);
				pRow->PutValue(2, _variant_t("Summary"));
				pRow->PutValue(3, _variant_t(strReportName + "smryavery2"));
				pDataList->AddRow(pRow);

				//set the current selection to be detailed if nothing seleted already
				if (pDataList->SetSelByColumn(3, _variant_t(strSelectedFileName)) == -1) {
					// (j.gruber 2010-02-04 13:03) - PLID 29120 - check whether it should be detailed or summary
					if (GetRemotePropertyInt("SttmntDefaultDetailed", 1, 0, "<None>") == 1) { 
						pDataList->SetSelByColumn(0, (long)0);			
					}
					else {
						pDataList->SetSelByColumn(0, (long)1);			
					}					
				}

				break;

			case 2: { //default report 
				_RecordsetPtr rs = CreateRecordset("SELECT ID, Number, Title, FileName FROM CustomReportsT WHERE ID = %li", nReportID);
				while (!rs->eof) {
					pRow = pDataList->GetRow(-1);
					pRow->PutValue(0, AdoFldLong(rs, "ID"));
					pRow->PutValue(1, AdoFldLong(rs, "Number"));
					pRow->PutValue(2, _variant_t(AdoFldString(rs, "Title")));
					pRow->PutValue(3, _variant_t(AdoFldString(rs, "FileName")));
					pDataList->AddRow(pRow);

					rs->MoveNext();
				}

				//set the selection to be the default report if nothing already selected
				if (pDataList->SetSelByColumn(3, _variant_t(strSelectedFileName)) == -1) {
					rs = CreateRecordset("SELECT CustomReportID FROM DefaultReportsT WHERE ID = %li", nReportID);
					if (! rs->eof) {
						pDataList->SetSelByColumn(1, AdoFldLong(rs, "CustomReportID", -1));
					}
					else {
						if (pDataList->GetRowCount() > 0) {
							pDataList->CurSel = 0;
						}
					}
				}
				}
			break;

				

			case 3: //estatement

				//e-statement report
				pRow = pDataList->GetRow(-1);
				pRow->PutValue(0, (long) 0);
				pRow->PutValue(1, (long) -2);
				pRow->PutValue(2, _variant_t("E-Statement Summary"));
				pRow->PutValue(3, "EbillStatement");
				pDataList->AddRow(pRow);

				pRow = pDataList->GetRow(-1);
				pRow->PutValue(0, (long)1);
				pRow->PutValue(1, (long)-1);
				pRow->PutValue(2, _variant_t("E-Statement Detailed"));
				pRow->PutValue(3, "EBillStatement");
				pDataList->AddRow(pRow);

				//set the summary to be default if nothing selected already
				//if (pDataList->SetSelByColumn(3, _variant_t(strSelectedFileName)) == -1) {
					// (j.gruber 2010-02-04 13:03) - PLID 29120 - check whether it should be detailed or summary
					if (GetRemotePropertyInt("SttmntDefaultDetailed", 1, 0, "<None>") == 1) { 
						pDataList->SetSelByColumn(0, (long)1);			
					}
					else {
						pDataList->SetSelByColumn(0, (long)0);			
					}					
				//}
					
			break;
		}

	}NxCatchAll("Error In AddToReportList");


}

CString StmtConvertTo70Name(CString str60Name) {

	CString strTemp;

	strTemp = str60Name.Left(str60Name.GetLength() - 10) + "70" + str60Name.Right(10);

	return strTemp;
	

}

	

CString GetStatementFileName(long nReportID) {
	
	CString strFile; 
	switch (nReportID) {

		case 169:
		case 234:
		case 353:
		case 354:
		case 434:
		case 436:
			strFile = "Statement";
		break;

		case 337:
		case 338:
		case 355:
		case 356:
		case 435:
		case 437:
			strFile = "StatementLoc";
		break;

		case 483:
		case 484:
		case 485:
		case 486:
			strFile = "StatementProv";
		break;

	}

	//now check whether it is 7.0 or not
	if (GetRemotePropertyInt("SttmntUse70Version")) {
		strFile += "70";
	}

	return strFile;
}