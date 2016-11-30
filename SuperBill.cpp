#include "stdafx.h"
#include "PracProps.h"
#include "superbill.h"
#include "letterWriting.h"
// (a.walling 2007-11-07 10:29) - PLID 27998 - VS2008 - Need to use standard headers and update code to reference the std:: namespace
#include <fstream>
#include "NxStandard.h"
#include "MergeEngine.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"

using namespace ADODB;

void WriteField (std::ofstream &outfile, const CString &text)
{
	outfile << '\"' << text << '\"';
}

void WriteField (std::ofstream &outfile, COleVariant &var)
{
	CString text;
	try
	{	switch (var.vt)
		{	case	VT_NULL:
				break;
			case	VT_I2:
			case	VT_I4:
			case	VT_UI1:
			case	VT_UI2:
			case	VT_UI4:
			case	VT_I8:
			case	VT_UI8:
			case	VT_INT:
			case	VT_UINT:
			case	VT_I1:
				var.ChangeType(VT_BSTR);
				text = (CString)(char *)var.bstrVal;
				outfile << text;
				break;
			default:
				var.ChangeType(VT_BSTR);
				text = (CString)(char *)var.bstrVal;
				outfile << '\"' << text << '\"';
		}
	}
	catch (CException *e)
	{	e->ReportError();
		e->Delete();
	}
}

void SuperBill(int nPatientId)
{
	//DRT - Some changes made to the way this is generated, here are our cases:
	//		Note that we are not looking at the time at all, so if they have had 
	//		an appt already this morning, we still list it when generating this 
	//		list of id's.
	//		1)  Patient has no appointments at all.  In this case it emulates the old behavior
	//		and just prints out a blank superbill for appt id -1.
	//		2)  Patient has 1 appointment some day in the future (tomorrow, thursday, 3rd friday
	//		of a month that begins with H, etc).  In this case we generate a list of the appt id
	//		and run the merge in an appt based manner.  This is the most common case.
	//		3)  Patient has multiple appts in the future, all on the same day.  In this case we
	//		look ahead, find the date of the next appt, and then do a search for all appts on that
	//		day.  Note that case #2 here is just a subset of case #3, so they will be handled
	//		by the same code.
	//		4)  Patient has multiple appts in the future, some on one day, some on another.  This is
	//		handled by case #3 still, because we look ahead to the next appt and get all on that day.
	//		The ones past that day are not printed out.
	CString strSql;
	bool bApptBased;

	//step 1, get the next appt
	_RecordsetPtr prs = CreateRecordset("SELECT TOP 1 Date FROM AppointmentsT WHERE PatientID = %li AND Date >= convert(nvarchar, GetDate(), 10) AND Status <> 4 ORDER BY Date", nPatientId);
	if(prs->eof) {
		//this patient has no future appts, case #1 from above
		//we're already in a case ready for patient-based merging
		bApptBased = false;
		strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", nPatientId);
	}
	else {
		//step 2, select all appts on that day
		bApptBased = true;
		COleDateTime dtNext;
		dtNext = AdoFldDateTime(prs, "Date");

		strSql.Format("SELECT ID, PatientID FROM AppointmentsT WHERE PatientID = %li AND Date = '%s' AND Status <> 4", nPatientId, dtNext.Format("%m/%d/%Y"));
	}

	//DRT 6/12/2008 - PLID 9679 - For now, the per-patient method will still use the default superbill.
	PrintSuperbill(strSql, bApptBased, "");

}

//DRT 6/12/2008 - PLID 9679 - It is now possible to pass in the path to use for this superbill, in leiu of
//	the default global superbill.
void SuperbillByAppt(long nApptID, CString strOverridePath) 
{
	//just format for this one appointment

	// (a.walling 2010-09-15 09:20) - PLID 7018 - This would filter out canceled ones once we started, so just filter it out here
	CString strSql;
	strSql.Format("SELECT ID, PatientID FROM AppointmentsT WHERE ID = %li AND AppointmentsT.Status <> 4", nApptID);

	PrintSuperbill(strSql, true, strOverridePath);
}

BOOL AllowSaveSuperbill()
{
	// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Not save
	if (GetRemotePropertyInt("AllowSaveSuperbill", 0, 0, "<None>", true) != 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

CString FindSuperbill(CString strFilePath, CString strFileName) {

	CString strFoundPath = "";

	CFileFind finder;
	if(finder.FindFile(strFilePath ^ "*.*")) {

		while (finder.FindNextFile())
		{
			if(finder.IsDots())
				continue;

			if(finder.IsDirectory()) {
				strFoundPath = FindSuperbill(finder.GetFilePath(), strFileName);
				if(strFoundPath != "")
					return strFoundPath;
			}
			else {

				// (a.walling 2008-04-28 13:16) - PLID 28108 - NTFS filenames are case-insensitive.
				if(strFileName.CompareNoCase(finder.GetFileName()) == 0) {
					strFoundPath = finder.GetFilePath();
					return strFoundPath;
				}
			}
		}
		//do once more
		if(finder.IsDirectory()) {
			if(!finder.IsDots()) {
				strFoundPath = FindSuperbill(finder.GetFilePath(), strFileName);
				if(strFoundPath != "")
					return strFoundPath;
			}
		}
		else {

			// (a.walling 2008-04-28 13:16) - PLID 28108 - NTFS filenames are case-insensitive.
			if(strFileName.CompareNoCase(finder.GetFileName()) == 0) {
				strFoundPath = finder.GetFilePath();
				return strFoundPath;
			}
		}
	}

	return strFoundPath;
}

//DRT 6/12/2008 - PLID 9679 - Added override path
void PrintSuperbill(CString strSql, bool bApptBased, CString strOverridePath)
{
	CWaitCursor cur;

	// Make sure word is installed
	if (!GetWPManager()->CheckWordProcessorInstalled()) {
		return;
	}

	// (j.jones 2011-07-08 15:28) - PLID 13660 - cached preferences
	g_propManager.CachePropertiesInBulk("SuperBill_Number", propNumber,
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'MergeSuperbillsToPrinter' "
		")",
		_Q(GetCurrentUserName()));

	g_propManager.CachePropertiesInBulk("SuperBill_Text", propText,
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'DefaultSuperbillFilename' "
		")",
		_Q(GetCurrentUserName()));

	// Accept all the default parameters provided by the CMergeInfo class
	CMergeEngine mi;

	mi.m_nFlags |= BMS_SUPERBILL;
	if (AllowSaveSuperbill()) {
		mi.m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;
	} else {
		mi.m_nFlags |= BMS_SAVE_HISTORY_NO_FILE;
	}
	if (g_bMergeAllFields) mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;

	// (j.jones 2011-07-08 15:28) - PLID 13660 - added pref. to merge to printer
	if(GetRemotePropertyInt("MergeSuperbillsToPrinter", 0, 0, GetCurrentUserName(), true) == 1) {
		mi.m_nFlags = (mi.m_nFlags | BMS_MERGETO_PRINTER) & ~BMS_MERGETO_SCREEN;
	}

	CString strMergeTFresh, strMergeTReprint;
	if (bApptBased) {

		CString strSqlFresh = strSql + " AND AppointmentsT.ID NOT IN (SELECT ReservationID FROM PrintedSuperBillsT WHERE Void = 0)";
		CString strSqlExisting = strSql + " AND AppointmentsT.ID IN (SELECT ReservationID FROM PrintedSuperBillsT WHERE Void = 0)";

		mi.m_nFlags |= BMS_APPOINTMENT_BASED;
		CStringArray aryFieldNames, aryFieldTypes;
		aryFieldNames.Add("ID"); // Appointment id
		aryFieldTypes.Add("INT");
		aryFieldNames.Add("PatientID"); // Patient id (obviously)
		aryFieldTypes.Add("INT");

		// (a.walling 2010-09-15 09:20) - PLID 7018 - Check if there are existing superbills for this appt
		ExistingSuperBillInfo existingInfo(NULL);
		existingInfo.Load(FormatString("AppointmentsT.ID IN (SELECT ID FROM (%s) SubQ)", strSql));

		existingInfo.Prompt();

		switch (existingInfo.m_Action) {
			case ExistingSuperBillInfo::Cancel:
				return;
				break;
			case ExistingSuperBillInfo::PrintNew:
				strMergeTFresh = CreateTempIDTable(strSqlFresh, aryFieldNames, aryFieldTypes, TRUE, TRUE, NULL);
				strMergeTReprint.Empty();
				break;
			case ExistingSuperBillInfo::PrintNewWithCopies:
				if (existingInfo.m_nFreshSuperBills > 0) {
					strMergeTFresh = CreateTempIDTable(strSqlFresh, aryFieldNames, aryFieldTypes, TRUE, TRUE, NULL);
				}
				strMergeTReprint = CreateTempIDTable(strSqlExisting, aryFieldNames, aryFieldTypes, TRUE, TRUE, NULL);
				break;
			case ExistingSuperBillInfo::PrintAll:
			default:
				strMergeTFresh = CreateTempIDTable(strSql, aryFieldNames, aryFieldTypes, TRUE, TRUE, NULL);
				strMergeTReprint.Empty();
				break;
		}


		//DRT 4/24/03 - By default the m_strResFilter filters out appointments in the past.  We do not want
		//		to do that in appt based cases, because we have already determined which appointments should
		//		be printed, and this would further filter that list remove things we really should print.
		// (a.walling 2010-09-15 09:20) - PLID 7018 - Removed
		mi.m_strResFilter = "";
	} else {
		strMergeTFresh = CreateTempIDTable(strSql, "ID");
	}

	try {
		CString strSuperbillPathToUse;

		if(!strOverridePath.IsEmpty()) {
			//DRT 6/12/2008 - PLID 9679 - Use this override instead of the global default.
			if(DoesExist(strOverridePath)) {
				strSuperbillPathToUse = strOverridePath;
			}
			else {
				//File inaccessible.  Warn them and quit.
				MsgBox(MB_OK|MB_ICONINFORMATION, 
					"The selected superbill template '%s' cannot be found.  Please ensure the file still exists and you have access "
					"to the directory it is in.", strOverridePath);
				return;
			}
		}
		else {
			//This is the old behavior, still in use in certain cases, like the "superbill for scheduled patients" screen, 
			//	etc.
			CString strSuperbill = GetPropertyText("DefaultSuperbillFilename", "SuperBill.dot", 0, false);
			CString strSuperbillPath = FindSuperbill(GetTemplatePath("Forms", ""),strSuperbill);
			if (strSuperbillPath != "" && DoesExist(strSuperbillPath)) {
				//Path exists, we're happy.
				strSuperbillPathToUse  = strSuperbillPath;
			} else {
				MsgBox(MB_OK|MB_ICONINFORMATION, 
					"The default superbill template cannot be found.  To change the \n"
					"default, go to the \"Print Superbills For Scheduled Patients\" dialog, \n"
					"found under the Activities menu, Superbill submenu.  The current default is\n\n%s", strSuperbillPath);
				return;
			}
		}

		// (z.manning, 03/06/2008) - PLID 29131 - Need to load the sender merge fields
		if(!mi.LoadSenderInfo(TRUE)) {
			return;
		}

		// (a.walling 2010-09-15 09:20) - PLID 7018 - Merge fresh superbills
		if (!strMergeTFresh.IsEmpty()) {
			mi.MergeToWord(strSuperbillPathToUse, std::vector<CString>(), strMergeTFresh, (bApptBased ? "PatientID" : ""));
		}

		// (a.walling 2010-09-15 09:20) - PLID 7018 - Now merge any reprints
		if (!strMergeTReprint.IsEmpty()) {
			mi.m_nFlags |= BMS_REPRINT;
			mi.MergeToWord(strSuperbillPathToUse, std::vector<CString>(), strMergeTReprint, (bApptBased ? "PatientID" : ""));
		}

	} NxCatchAll("::SuperBill");
}


ExistingSuperBillInfo::ExistingSuperBillInfo(CWnd* pParent)
	: m_nFreshSuperBills(0)
	, m_nExistingSuperBills(0)
	, m_nExistingSuperBillsWithFinancialInfo(0)
	, m_Action(Cancel)
	, NxTaskDialog(pParent)
{
}

// (a.walling 2010-09-15 09:20) - PLID 7018 - Gather info on existing superbills
void ExistingSuperBillInfo::Load(const CString& strBaseWhere)
{
	try {
		m_nFreshSuperBills = m_nExistingSuperBills = m_nExistingSuperBillsWithFinancialInfo = 0;

		_RecordsetPtr prs = CreateRecordset(
			"SELECT CAST(CASE WHEN BilledSuperBillsQ.SuperbillID IS NULL THEN 0 ELSE 1 END AS BIT) AS HasFinancialInfo, CAST(COALESCE(PrintedSuperBillsT.Void, 1) AS BIT) AS IsFresh, COUNT(DISTINCT AppointmentsT.ID) AS Count "
			"FROM AppointmentsT "
			"LEFT JOIN PrintedSuperBillsT "
				"ON AppointmentsT.ID = PrintedSuperBillsT.ReservationID AND PrintedSuperBillsT.Void = 0 "
			"LEFT JOIN AppointmentResourceT "
				"ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
			"LEFT JOIN ( "
				"SELECT SuperbillID "
				"FROM ChargesT "
				"INNER JOIN LineItemT "
					"ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN BillsT "
					"ON ChargesT.BillID = BillsT.ID "
				"WHERE "
					"SuperbillID IS NOT NULL "
					"AND LineItemT.Deleted = 0 "
					"AND BillsT.Deleted = 0 "
			") BilledSuperBillsQ "
				"ON PrintedSuperBillsT.SavedID = BilledSuperBillsQ.SuperbillID "
			"WHERE %s "
			// (a.walling 2010-09-23 13:36) - PLID 7018 - SQL 2000 fails if a select statement uses CAST or CONVERT on a grouped 
			// expression which is not identically cast or convert in the group by clause
			"GROUP BY CAST(COALESCE(PrintedSuperBillsT.Void, 1) AS BIT), CAST(CASE WHEN BilledSuperBillsQ.SuperbillID IS NULL THEN 0 ELSE 1 END AS BIT)"
			, strBaseWhere
			);

		while (!prs->eof) {
			BOOL bIsFresh = AdoFldBool(prs, "IsFresh");
			BOOL bHasFinancial = AdoFldBool(prs, "HasFinancialInfo");
			int nCount = AdoFldLong(prs, "Count", 0);
			if (bIsFresh) {
				m_nFreshSuperBills += nCount;
			} else {
				m_nExistingSuperBills += nCount;

				if (bHasFinancial) {
					m_nExistingSuperBillsWithFinancialInfo += nCount;
				}
			}

			prs->MoveNext();
		}
	} NxCatchAllThrow(__FUNCTION__);
}

ExistingSuperBillInfo::Action ExistingSuperBillInfo::Prompt()
{
	if (m_nExistingSuperBills <= 0) {
		m_Action = PrintAll;
		return m_Action;
	}
				
	Config()
		.InformationIcon().CancelOnly()
		.MainInstructionText(FormatString("%s superbills have already been printed!", (m_nFreshSuperBills > 0) ? "Some" : "All"))
		.ContentText(							
			FormatString("Printed superbills are on record for %li out of %li appointment%s. "
				"%s"
				, m_nExistingSuperBills, m_nExistingSuperBills + m_nFreshSuperBills, (m_nExistingSuperBills + m_nFreshSuperBills) > 1 ? "s" : "", 
				(
					(m_nExistingSuperBillsWithFinancialInfo <= 0) 
					? CString(" No financial information has been associated with any of these superbills.")
					: FormatString(" Financial information has already been associated with %li of these superbills.", m_nExistingSuperBillsWithFinancialInfo)
				)
			)
		);

	if (m_nFreshSuperBills > 0) {
		Config()
			.AddCommand(PrintNew, 
				"Only print new superbills\nAppointments with existing superbills will be skipped.")
			.AddCommand(PrintNewWithCopies, 
				"Print new superbills and print copies of existing superbills\nAppointments with existing superbills will be merged again but not assigned a new superbill ID.");
	} else {
		Config()
			.AddCommand(PrintNewWithCopies, 
				"Print copies of existing superbills\nAppointments with existing superbills will be merged again but not assigned a new superbill ID.");
	}			

	Config()
		.AddCommand(PrintAll, 
			FormatString("Print all superbills\nA new superbill ID will be assigned for all appointments being merged.%s",
				(m_nExistingSuperBills > 0) ? " This will create duplicate superbill IDs!" : ""));

	m_Action = (Action)DoModal();

	return m_Action;
}

BOOL ExistingSuperBillInfo::OnButtonClicked(int nButton)
{
	if (nButton == PrintAll && m_nExistingSuperBills > 0) {
		if (IDNO == MessageBox(
			"By printing superbills for all appointments, duplicate superbill IDs will be assigned for appointments with existing superbills.\r\n\r\n"
			"Please ensure that superbills are voided appropriately after creating these duplicate superbill IDs.\r\n\r\n"
			"Do you want to continue?", NULL, MB_ICONWARNING|MB_YESNO))
		{
			return TRUE; // prevent close
		}
	} 

	return NxTaskDialog::OnButtonClicked(nButton);
};
