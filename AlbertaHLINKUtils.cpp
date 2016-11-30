//AlbertaHLINKUtils.cpp
//

#include "stdafx.h"
#include "AlbertaHLINKUtils.h"
#include "SelectAMACodeDlg.h"
#include "ProgressDialog.h"

using namespace ADODB;

// (j.jones 2010-11-03 15:04) - PLID 39620 - created
bool UseAlbertaHLINK()
{
	long nAlbertaHLINK = GetRemotePropertyInt("UseAlbertaHLINK", 0, 0, "<None>", true);
	if(nAlbertaHLINK == 0)
		return false;
	else
		return true;
}

// (j.jones 2014-07-28 13:10) - PLID 62947 - Ensures g_mapAlbertaBillingStatusToID is filled
// with the correct BillStatusT.IDs, will auto-create all required statuses if they do not exist.
BOOL EnsureAlbertaBillingStatuses()
{
	//return true if it exists
	BOOL bStatusExisted = TRUE;

	if (g_mapAlbertaBillingStatusToID.size() == 0) {
		//if the map is not filled, fill it now

		_RecordsetPtr rs = CreateParamRecordset("SELECT ID, Name, Custom FROM BillStatusT "
			"WHERE Name IN ({STRING}, {STRING}, {STRING}, {STRING}, {STRING})",
			ALBERTA_ENTERED_STATUS_NAME, ALBERTA_PENDING_STATUS_NAME, ALBERTA_POSTED_STATUS_NAME,
			ALBERTA_PARTIALLY_PAID_STATUS_NAME, ALBERTA_REJECTED_STATUS_NAME);
		if (!rs->eof) {

			//if we didn't find all the records, don't bother loading any, we'll
			//create what we need and load the rest later
			rs->MoveLast();
			rs->MoveFirst();
			//whatever the last status is, that's what the recordset size should be
			if (rs->GetRecordCount() == (long)eRejected) {
				//we got the expected number of records

				std::vector<int> aryCustomIDs;

				while (!rs->eof) {
					long nID = VarLong(rs->Fields->Item["ID"]->Value);
					CString strName = VarString(rs->Fields->Item["Name"]->Value);
					bool bCustom = VarBool(rs->Fields->Item["Custom"]->Value) ? true : false;
					
					if (bCustom) {
						//It is possible a user may have made a status with this name, and
						//now we're stealing it as a built-in status. If so, we will need
						//to turn off the Custom bit so they can no longer edit it.
						aryCustomIDs.push_back(nID);
					}

					//now fill our map, matching the name to the status enum
					if (strName.CompareNoCase(ALBERTA_ENTERED_STATUS_NAME) == 0) {
						g_mapAlbertaBillingStatusToID[eEntered] = nID;
					}
					else if (strName.CompareNoCase(ALBERTA_PENDING_STATUS_NAME) == 0) {
						g_mapAlbertaBillingStatusToID[ePending] = nID;
					}
					else if (strName.CompareNoCase(ALBERTA_POSTED_STATUS_NAME) == 0) {
						g_mapAlbertaBillingStatusToID[ePosted] = nID;
					}
					else if (strName.CompareNoCase(ALBERTA_PARTIALLY_PAID_STATUS_NAME) == 0) {
						g_mapAlbertaBillingStatusToID[ePartiallyPaid] = nID;
					}
					else if (strName.CompareNoCase(ALBERTA_REJECTED_STATUS_NAME) == 0) {
						g_mapAlbertaBillingStatusToID[eRejected] = nID;
					}
					else {
						//how did we get an unknown name?
						ASSERT(FALSE);
					}

					rs->MoveNext();
				}

				//if any statuses need their custom bit set to zero, do this now
				if (aryCustomIDs.size() > 0) {
					ExecuteParamSql("UPDATE BillStatusT SET Custom = 0 WHERE ID IN ({INTVECTOR})", aryCustomIDs);
					//this is not audited because it is not user-editable

					//send a tablechecker
					CClient::RefreshTable(NetUtils::BillStatusT, -1);
				}
			}
		}
		rs->Close();

		//whatever the last status is, that's what the map size should be
		if (g_mapAlbertaBillingStatusToID.size() != (long)AlbertaBillingStatus::eRejected) {

			//the map is not the right size, which should only happen the first time any
			//user calls this function when they have no Alberta statuses, or if we've
			//added more built-in statuses
			CreateAlbertaBillingStatuses();

			//double-check the size
			if (g_mapAlbertaBillingStatusToID.size() != (long)AlbertaBillingStatus::eRejected) {
				//this should be impossible unless you've added a new enumeration and
				//did not code support for it properly
				ASSERT(FALSE);
				ThrowNxException("CreateAlbertaBillingStatuses failed to create all statuses.");
			}

			bStatusExisted = FALSE;
		}
	}

	//if this failed, we cannot continue		
	if (g_mapAlbertaBillingStatusToID.size() == 0) {
		ThrowNxException("EnsureAlbertaBillingStatuses failed to load all statuses.");
	}

	return bStatusExisted;
}

// (j.jones 2014-07-28 13:47) - PLID 62947 - creates Alberta statuses in data, as needed
void CreateAlbertaBillingStatuses()
{
	//This function should only be called once ever, per office, the first time
	//any user calls an Alberta function that needs a status.
	//It may be called again if we add more enumerations, so this code has to support
	//the possibility of some enums existing and some being absent.

	CParamSqlBatch sqlBatch;
	sqlBatch.Add("SET NOCOUNT ON");
	sqlBatch.Add("DECLARE @EnteredID INT, @PendingID INT, @PostedID INT, @PartialPayID INT, @RejectedID INT");
	
	sqlBatch.Add("SELECT @EnteredID = (SELECT ID FROM BillStatusT WHERE Name = {STRING})", ALBERTA_ENTERED_STATUS_NAME);
	sqlBatch.Add("IF (@EnteredID Is Null) \r\n"
		"BEGIN \r\n"
		"	INSERT INTO BillStatusT (Name, Custom) VALUES ({STRING}, 0) \r\n"
		"	SET @EnteredID = SCOPE_IDENTITY() \r\n"
		"END \r\n", ALBERTA_ENTERED_STATUS_NAME);
	
	sqlBatch.Add("SELECT @PendingID = (SELECT ID FROM BillStatusT WHERE Name = {STRING})", ALBERTA_PENDING_STATUS_NAME);
	sqlBatch.Add("IF (@PendingID Is Null) \r\n"
		"BEGIN \r\n"
		"	INSERT INTO BillStatusT (Name, Custom) VALUES ({STRING}, 0) \r\n"
		"	SET @PendingID = SCOPE_IDENTITY() \r\n"
		"END \r\n", ALBERTA_PENDING_STATUS_NAME);

	sqlBatch.Add("SELECT @PostedID = (SELECT ID FROM BillStatusT WHERE Name = {STRING})", ALBERTA_POSTED_STATUS_NAME);
	sqlBatch.Add("IF (@PostedID Is Null) \r\n"
		"BEGIN \r\n"
		"	INSERT INTO BillStatusT (Name, Custom) VALUES ({STRING}, 0) \r\n"
		"	SET @PostedID = SCOPE_IDENTITY() \r\n"
		"END \r\n", ALBERTA_POSTED_STATUS_NAME);
	
	sqlBatch.Add("SELECT @PartialPayID = (SELECT ID FROM BillStatusT WHERE Name = {STRING})", ALBERTA_PARTIALLY_PAID_STATUS_NAME);
	sqlBatch.Add("IF (@PartialPayID Is Null) \r\n"
		"BEGIN \r\n"
		"	INSERT INTO BillStatusT (Name, Custom) VALUES ({STRING}, 0) \r\n"
		"	SET @PartialPayID = SCOPE_IDENTITY() \r\n"
		"END \r\n", ALBERTA_PARTIALLY_PAID_STATUS_NAME);
	
	sqlBatch.Add("SELECT @RejectedID = (SELECT ID FROM BillStatusT WHERE Name = {STRING})", ALBERTA_REJECTED_STATUS_NAME);
	sqlBatch.Add("IF (@RejectedID Is Null) \r\n"
		"BEGIN \r\n"
		"	INSERT INTO BillStatusT (Name, Custom) VALUES ({STRING}, 0) \r\n"
		"	SET @RejectedID = SCOPE_IDENTITY() \r\n"
		"END \r\n", ALBERTA_REJECTED_STATUS_NAME);

	//make sure Custom is set to 0 for any of these statuses
	sqlBatch.Add("UPDATE BillStatusT SET Custom = 0 WHERE ID IN (@EnteredID, @PendingID, @PostedID, @PartialPayID, @RejectedID)");

	sqlBatch.Add("SET NOCOUNT OFF");

	sqlBatch.Add("SELECT @EnteredID AS EnteredID, @PendingID AS PendingID, @PostedID AS PostedID, @PartialPayID AS PartialPayID, @RejectedID AS RejectedID");

	_RecordsetPtr rs = sqlBatch.CreateRecordset(GetRemoteData());
	if (rs->eof) {
		ThrowNxException("CreateAlbertaBillingStatuses failed to create the required statuses.");
	}
	else {
		long nEnteredID = VarLong(rs->Fields->Item["EnteredID"]->Value, -1);
		if (nEnteredID == -1) {
			ThrowNxException("CreateAlbertaBillingStatuses failed to create the 'Entered' status.");
		}

		long nPendingID = VarLong(rs->Fields->Item["PendingID"]->Value);
		if (nPendingID == -1) {
			ThrowNxException("CreateAlbertaBillingStatuses failed to create the 'Pending' status.");
		}

		long nPostedID = VarLong(rs->Fields->Item["PostedID"]->Value);
		if (nPostedID == -1) {
			ThrowNxException("CreateAlbertaBillingStatuses failed to create the 'Posted' status.");
		}

		long nPartialPayID = VarLong(rs->Fields->Item["PartialPayID"]->Value);
		if (nPartialPayID == -1) {
			ThrowNxException("CreateAlbertaBillingStatuses failed to create the 'Partially Paid' status.");
		}

		long nRejectedID = VarLong(rs->Fields->Item["RejectedID"]->Value);
		if (nRejectedID == -1) {
			ThrowNxException("CreateAlbertaBillingStatuses failed to create the 'Rejected' status.");
		}

		//fill our map, we're done
		g_mapAlbertaBillingStatusToID.clear();
		g_mapAlbertaBillingStatusToID[eEntered] = nEnteredID;
		g_mapAlbertaBillingStatusToID[ePending] = nPendingID;
		g_mapAlbertaBillingStatusToID[ePosted] = nPostedID;
		g_mapAlbertaBillingStatusToID[ePartiallyPaid] = nPartialPayID;
		g_mapAlbertaBillingStatusToID[eRejected] = nRejectedID;
	}
	rs->Close();

	//send a tablechecker
	CClient::RefreshTable(NetUtils::BillStatusT, -1);
}

// (j.jones 2014-07-28 13:12) - PLID 62947 - given a status enum, will return the proper BillStatusT.ID
// that represents that status
long GetAlbertaBillStatusID(AlbertaBillingStatus eStatus)
{
	//make sure our map is filled
	EnsureAlbertaBillingStatuses();

	long nStatusID = g_mapAlbertaBillingStatusToID[eStatus];
	if (nStatusID <= 0) {
		//either the map is not completely filled, or this really is an invalid status
		ASSERT(FALSE);
		ThrowNxException("GetAlbertaBillStatusID was called with an invalid/unknown status (%li).", (long)eStatus);
	}
	return nStatusID;
}

CString GetAlbertaHLINKHealthTitle()
{
	CString strHealthTitle = "Health Number";
	//if enabled, get the actual name of the custom field
	long nHealthNumberCustomField = GetRemotePropertyInt("Alberta_PatientULICustomField", 1, 0, "<None>", true);
	ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM CustomFieldsT WHERE ID = {INT}", nHealthNumberCustomField);
	if(!rs->eof) {
		CString str = AdoFldString(rs, "Name", "");
		str.TrimLeft();
		str.TrimRight();
		if(!str.IsEmpty()) {
			strHealthTitle = str;
		}
	}
	rs->Close();
	return strHealthTitle;
}

// (d.singleton 2011-09-15 11:03) - PLID 44946 Create a import feature that can take in a file of service codes & modifiers from Alberta and create/update this data in Practice.
void ImportAlbertaCodes()
{
	try {

		//good bit of code will be copied from the ImportCPTCodes() function in globalutils.cpp
		AfxMessageBox("In the following window, please select your code file to import.");
		CFileDialog BrowseFiles(TRUE,NULL,NULL,OFN_FILEMUSTEXIST);
		if (BrowseFiles.DoModal() == IDCANCEL) return;
		CString strFileName = BrowseFiles.GetPathName();

		CFile InputFile, OutputFile;

		//open the file for reading
		if(!InputFile.Open(strFileName,CFile::modeRead | CFile::shareCompat)) {
			AfxMessageBox("The input file could not be found or opened. Please double-check your path and filename.");
			return;
		}

		CWaitCursor pWait;

		//quickly run through and try to identify that this is a valid file
		CArchive arTest(&InputFile, CArchive::load);
		CString strTest;
		arTest.ReadString(strTest);
		BOOL bFileHasData = FALSE;
		BOOL bValid = TRUE;

		if(ParseField(strTest,36,34) != "ALBERTA HEALTH CARE INSURANCE PLAN")
			bValid = FALSE;

		if(!bValid) {
			AfxMessageBox("This does not appear to be a valid code file. Please double-check the file you selected.");
			return;
		}

		//reopen the file for reading again
		arTest.Close();
		InputFile.Close();
		InputFile.Open(strFileName,CFile::modeRead | CFile::shareCompat);

		BOOL bOutputFileUsed = FALSE;
		BOOL bAdditions = FALSE;

		CString strIn;	//input string
		CArchive arIn(&InputFile, CArchive::load);

		CString OutputString;

		//create variables for all the different data elements in the file
		CString strCPTCode;
		CString strLastCPTCode;
		CString strDescription;
		CString strBaseAmount;
		CString strMod;
		CString strType;
		CString strAction;
		CString strModAmount;
		long nModPerc = -1;
		CString strLastOutput;
		long nLastCPTID;
//		long nModID;
		long nCallUpper = -1;
		long nCallLower = -1;
		AlbertaHLINKModifierActionType eActionType = amatUnknown;
		long nProgress = 0;
		long nTotalRecords = 40000;
		BOOL bIsSkll = FALSE;
		BOOL bAlwaysChoose = FALSE;
		BOOL bIsCall = FALSE;
		BOOL bIsSurt = FALSE;//TES 7/12/2013 - PLID 57501
		BOOL bSavedModifier = FALSE; //(b.eyers 2016-01-19) - PLID 67853
		//create map to hold the already existing mods so we know if we need to update vs create
		CMap<CString, LPCTSTR, CString, LPCTSTR> mapAlbertaMods;

		//fill the map
		ADODB::_RecordsetPtr prsExistingMods = CreateRecordset("SELECT * FROM CptModifierT");
		while(!prsExistingMods->eof)
		{
			CString strMapMod = AdoFldString(prsExistingMods, "Number");

			mapAlbertaMods.SetAt(strMapMod, strMapMod);
			prsExistingMods->MoveNext();
		}

		//clear out the AlbertaCptModLinkT records so the import does not create dublicates
		ExecuteSql("DELETE FROM AlbertaCptModLinkT");

		//create a progress dialog so the user doesnt think it froze on them
		CProgressDialog progressDialog;
		progressDialog.Start(NULL, CProgressDialog::NoCancel | CProgressDialog::NoMinimize | CProgressDialog::NoTime);
		progressDialog.SetLine(1, "Please wait while data imports...");
		progressDialog.SetLine(2, "This could take a few minutes");

		//start parsin
		while(arIn.ReadString(strIn) && strIn.Find("TABLE OF CONTENTS") == -1)
		{
			if(strIn.Mid(3, 6).Find(".") != -1)
			{
				strDescription = ParseField(strIn, 9, 107);
			}
			if(strIn.Left(3) == "   " && strIn.Mid(3, 6).Find(".") == -1 && strIn.Mid(3, 6) != "      " && strIn.Find("NOTE:") == -1)
			{
				strDescription = ParseField(strIn, 3, 107);
			}

			//for debuggin
			//ASSERT(!(strDescription.Find("NOTE:") != -1));
			// (j.jones 2012-03-06 09:51) - PLID 48642 - I don't know why the Mid(16, 4) check exists, but by itself it would skip
			// all (cont'd) lines. I added an OR to include these lines without sacrificing whatever it is that the (16, 4) check is trying to avoid.
			if((strIn.Mid(9, 6).Find(".") != -1 || strIn.Mid(9, 1) == "E" || strIn.Mid(9, 1) == "X" || strIn.Mid(9, 1) == "Y" || strIn.Mid(9, 1) == "F")
				&& strIn.Left(9).Trim() == "" && strIn.Mid(9, 3).Trim() != ""
				&& (strIn.Mid(16, 4).Trim() == "" || strIn.Mid(14, 15).Find("(cont'd)") != -1))
			{
				strCPTCode = ParseField(strIn, 9, 7);
				//ASSERT(!strCPTCode.CompareNoCase("98.03E") == 0);
				if(strIn.Mid(14, 15).Find("(cont'd)") == -1)
				{
					strBaseAmount = ParseField(strIn, 19, 8);
				}
				//check to see if its the beginning of a call code
				if(strIn.Mid(29, 10).Trim() != "" && (strIn.Right(38).Trim() == "" || strIn.Right(38).Trim().GetLength() == 1 || strIn.Mid(29, 4).Trim() == "CALL"))
				{
					bIsCall = TRUE;
				}
				else
				{
					bIsCall = FALSE;
				}

				if(strIn.Mid( 29, 5) != "     ")
				{
					strType = ParseField(strIn, 29, 5);

					//TES 7/12/2013 - PLID 57501 - Check whether this is a SURT code
					if(strType == "SURT") {
						bIsSurt = TRUE;
					}
				}
				strAction = ParseField(strIn, 48, 27);				
				//check to see if its a Call code
				if(strIn.Mid(34, 1).FindOneOf("1234567890") != -1)
				{
					CString strCallTemp = ParseField(strIn, 34, 9);
					strCallTemp.Trim();
					nCallLower = AsLong(_bstr_t(ParseField(strCallTemp, 0,1)));
					if(strCallTemp.Find("-") != -1)
					{				
						nCallUpper = atoi(strCallTemp.Right(strCallTemp.GetLength() - (strCallTemp.Find("-") + 1)));
					}
				}
				//check if its the max amount of a call code
				else if(strIn.Mid(34, 9).Trim().CompareNoCase("MAX") == 0)
				{
					strAction = "MAX";						
				}
				else
				{
					// (b.eyers 2016-01-19) - PLID 67853 
					// if we made it into this else and bSavedModifier is still false, the previous modifier was not saved (most likely was NBTR that got marked as a call but really isn't)
					if (!bSavedModifier && strMod != "") {
						ExecuteParamSql("INSERT INTO AlbertaCptModLinkT (Mod, ServiceID, Action, AmountPec, Amount, CallLowerLimit, CallUpperLimit) VALUES({STRING}, {INT}, {INT}, 0, CONVERT(MONEY,0), NULL, NULL);", strMod, nLastCPTID, (long)amatUnknown);
						bSavedModifier = TRUE;
					}

					strMod = ParseField(strIn, 34, 9);
					nCallLower = -1;
					nCallUpper = -1;
					// (b.eyers 2016-01-19) - PLID 67853
					bSavedModifier = FALSE;
				}
				strModAmount = ParseField(strIn , 76, 7);
				if(strModAmount.Find("%") != -1)
				{
					strModAmount.Replace("%", "");
					nModPerc = AsLong(_bstr_t(strModAmount));
				}
				else
				{
					nModPerc = -1;
				}
			}

			//need to check to see if the line is a chapter title e.g. 'II.  OPERATIONS ON THE NERVOUS SYSTEM '
			CString strTrimedString = strIn.Left(strIn.GetLength()).Trim();
		
			if(strIn.Left(28).Trim() == "" && strIn.Mid(29, 10).Trim() != "" && strIn.Mid(29, 10).Trim() != "----------" && !IsChapterTitle(strTrimedString) && (strIn.FindOneOf("1234567890") != -1 || strIn.GetLength() <= 47))
			{
				//check to see if its the beginning of a call code
				if(strIn.Mid(29, 10).Trim() != "" && (strIn.GetLength() <= 47 || strIn.Right(38).Trim().GetLength() <= 2 || strIn.Mid(29, 4).Trim() == "CALL"))
				{
					bIsCall = TRUE;
				}
				else
				{
					bIsCall = FALSE;
				}

				strType = ParseField(strIn, 29, 5);
				strAction = ParseField(strIn, 48, 27);
				//check to see if its a Call code ( just a number range, no alpha characters )
				if(strIn.Mid(34, 4).FindOneOf("ABCDEFGHIJKLMNOPQRSTUVWXYZ") == -1 && strIn.Mid(34, 1).FindOneOf("1234567890") != -1)
				{
					CString strCallTemp = ParseField(strIn, 34, 9);
					strCallTemp.Trim();
					nCallLower = AsLong(_bstr_t(ParseField(strCallTemp, 0,1)));
					if(strCallTemp.Find("-") != -1)
					{						
						nCallUpper = atoi(strCallTemp.Right(strCallTemp.GetLength() - (strCallTemp.Find("-") + 1)));
					}
				}
				//check if its the max amount of a call code
				else if(strIn.Mid(34, 9).Trim().CompareNoCase("MAX") == 0)
				{
					strAction = "MAX";						
				}				
				else
				{
					// (b.eyers 2016-01-19) - PLID 67853 
					// if we made it into this else and bSavedModifier is still false, the previous modifier was not saved (most likely was NBTR that got marked as a call but really isn't)
					if (!bSavedModifier && strMod != "") {
						ExecuteParamSql("INSERT INTO AlbertaCptModLinkT (Mod, ServiceID, Action, AmountPec, Amount, CallLowerLimit, CallUpperLimit) VALUES({STRING}, {INT}, {INT}, 0, CONVERT(MONEY,0), NULL, NULL);", strMod, nLastCPTID, (long)amatUnknown);
						bSavedModifier = TRUE;
					}

					strMod = ParseField(strIn, 34, 9);
					nCallLower = -1;
					nCallUpper = -1;
					// (b.eyers 2016-01-19) - PLID 67853
					bSavedModifier = FALSE; 
				}
				strModAmount = ParseField(strIn , 75, 7);
				if(strModAmount.Find("%") != -1)
				{
					strModAmount.Replace("%", "");
					nModPerc = AsLong(_bstr_t(strModAmount));
				}
				else
				{
					nModPerc = -1;
				}
			}

			//this is here so we can make sure we are not importing duplicates, and also just so i can see what exactly is exporting in case it blows up
			// (j.jones 2012-03-06 12:30) - PLID 48654 - do not compare by description, because that changes before the rest of the data changes
			CString strTempOutput = strCPTCode + ", " + strBaseAmount + ", " + strMod + ", " + strType + ", " + strAction + ", " + strModAmount + "\r\n";

			if(!strTempOutput.CompareNoCase(", , , , , \r\n") == 0 && strCPTCode != "" && strTempOutput.CompareNoCase(strLastOutput) != 0)
			{
				// (d.singleton 2011-10-19 12:06) - PLID 46032 - Split from 44946,  when parsing alberta mods , if there are any SKLL type mods parse those to a seperate csv file that will be imported as a fee schedule by the office
				if(strType.Trim().CompareNoCase("SKLL") == 0)
				{
					//get rid of invalid file character
					strMod.Replace("/", "_");
					CString strOutputFile = GetNxTempPath() ^ strMod + ".csv";
					
					CString strOutputLine = strCPTCode + ", " + strModAmount + ", " + strModAmount + "\r\n";
					OutputFile.Open(strOutputFile, CFile::modeNoTruncate|CFile::modeCreate|CFile::modeWrite | CFile::shareCompat);
					OutputFile.SeekToEnd();
					OutputFile.Write(strOutputLine, strOutputLine.GetLength());
					
					OutputFile.Close();
					

					bIsSkll = TRUE;
					bSavedModifier = TRUE; // (b.eyers 2016-01-19) - PLID 67853
				}
				else
				{
					bIsSkll = FALSE;
				}

				//reset strLastOutput value
				strLastOutput = strTempOutput;

				//now check if we need to update a code, or add new
				if(strLastCPTCode.CompareNoCase(strCPTCode) != 0)
				{
					strLastCPTCode = strCPTCode;

					// respect the checkbox to "Always choose the first code", by sorting in a
					// predictable manner: AMA codes first, then order by subcode second

					ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT ServiceT.ID, ServiceT.Name, "
						"(SELECT COUNT(*) AS Cnt FROM CPTCodeT WHERE Code = {STRING}) AS CodeCnt "
						"FROM CPTCodeT "
						"INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
						"WHERE Code = {STRING} "
						"ORDER BY IsAMA DESC, SubCode ASC",
						strCPTCode, strCPTCode);

					if(!rs->eof) {
						//update
						nLastCPTID = AdoFldLong(rs, "ID",-1);
						long nCodeCount = AdoFldLong(rs, "CodeCnt", -1);
						
						// do not show the dialog if "Always choose the first code" was checked
						if (nCodeCount > 1 && !bAlwaysChoose)
						{
							progressDialog.Stop();
							//more than one code so prompt them to choose which one they want to update
							CSelectAMACodeDlg dlg(NULL, TRUE);
							dlg.m_strCode = strCPTCode;
							dlg.m_strDesc = strDescription;
							dlg.m_bAlwaysChoose = bAlwaysChoose;
							if(dlg.DoModal() == IDOK) 
							{
								//get the selected ID that was chosen
								nLastCPTID = dlg.m_nChosen;
								bAlwaysChoose = dlg.m_bAlwaysChoose;

								//all we do is change the description, really
								ExecuteParamSql("UPDATE ServiceT SET Name = {STRING}, Price = CONVERT(Money, {STRING}) WHERE ID = {INT}", strDescription, strBaseAmount, nLastCPTID);
							} 
							progressDialog.Start(NULL, CProgressDialog::NoCancel | CProgressDialog::NoMinimize | CProgressDialog::NoTime);
							progressDialog.SetProgress(nProgress, nTotalRecords);
							progressDialog.SetLine(1, "Please wait while data imports...");
							progressDialog.SetLine(2, "This could take a few minutes");
						}
						else {
							//all we do is change the description, really
							ExecuteParamSql("UPDATE ServiceT SET Name = {STRING}, Price = CONVERT(Money, {STRING}) WHERE ID = {INT}", strDescription, strBaseAmount, nLastCPTID);
						}

						pWait.Restore();
					}
					else {
						//add new
						nLastCPTID = NewNumber("ServiceT","ID");
						ExecuteParamSql("INSERT INTO ServiceT (ID, Name, Price, Taxable1, Taxable2) VALUES ({INT}, {STRING}, Convert(money, {STRING}), 0, 0);", nLastCPTID, strDescription, strBaseAmount);
						ExecuteParamSql("INSERT INTO CPTCodeT (ID, Code, SubCode, TypeOfService) VALUES ({INT}, {STRING}, '', '');", nLastCPTID, strCPTCode);
						// (j.gruber 2012-12-04 11:32) - PLID 48566 - ServiceLocationInfoT
						ExecuteParamSql("INSERT INTO ServiceLocationInfoT (ServiceID, LocationID) \r\n"
							"SELECT {INT}, ID FROM LocationsT WHERE Managed = 1 "
						, nLastCPTID);	

						rs->Close();
						bAdditions = TRUE;
					}
				}

				if(bIsSurt) {
					//TES 7/12/2013 - PLID 57501 - The SURT codes are all together, and aren't broken up in quite the same way as the other codes.
					// I spent a few hours trying to fit it into the normal flow, but in the end I just had to parse all the SURT codes
					// in their own section here.  We'll loop through all of them here.
					CString strSurtLine;
					bool bEndOfSurtSection = false;
					//TES 7/12/2013 - PLID 57501 - We've already read the first line of the first code, so the next one we read will be the second line
					bool bOnFirstLine = false; 
					while(!bEndOfSurtSection) {
						arIn.ReadString(strSurtLine);
						if(bOnFirstLine) {
							//TES 7/12/2013 - PLID 57501 - Are we at the end of the SURTs?
							if(strSurtLine.Mid(29, 10).Trim() == "----------" ) {
								bEndOfSurtSection = true;
							}
							else {
								//TES 7/12/2013 - PLID 57501 - It's a new code, read the mod field
								strMod = ParseField(strSurtLine, 34, 9);
								bOnFirstLine = false;
								bSavedModifier = FALSE; // (b.eyers 2016-01-19) - PLID 67853
							}

						}
						else {
							//TES 7/12/2013 - PLID 57501 - We need to pull the "call range" and the price
							CString strCallTemp = ParseField(strSurtLine, 34, 9);
							strCallTemp.Trim();
							nCallLower = AsLong(_bstr_t(ParseField(strCallTemp, 0,1)));
							if(strCallTemp.Find("-") != -1)
							{				
								nCallUpper = atoi(strCallTemp.Right(strCallTemp.GetLength() - (strCallTemp.Find("-") + 1)));
							}
							strModAmount = ParseField(strSurtLine , 76, 7);

							//TES 7/18/2013 - PLID 57501 - We also need to parse the action
							strAction = ParseField(strSurtLine, 48, 27);
							eActionType = EnumerateActionType(strAction);								
							
							//TES 7/12/2013 - PLID 57501 - The special thing about SURT codes is that they are actually a range of codes,
							// named CODEnn, with the range of nn specified by the call limits. 
							if(nCallLower == -1 || nCallUpper == -1) {
								ASSERT(FALSE);
								nCallLower = nCallUpper = 1;
							}

							//TES 7/12/2013 - PLID 57501 - Loop through the range of calls, and insert each modifier with its suffix
							for(int nSuffix = nCallLower; nSuffix <= nCallUpper; nSuffix++) {
								CString strSurtMod = FormatString("%s%02i", strMod, nSuffix);
								if(mapAlbertaMods.Lookup(strSurtMod, strSurtMod) == 0) {
									ExecuteParamSql("INSERT INTO CptModifierT (Number, Note, Multiplier) VALUES ({STRING}, '', 1)", strSurtMod);
									mapAlbertaMods.SetAt(strSurtMod, strSurtMod);
								}

								//TES 7/18/2013 - PLID 57501 - The price is the listed amount multiplied by the suffix.  So TDES04 should cost twice as 
								// much as TDES01
								ExecuteParamSql("INSERT INTO AlbertaCptModLinkT (Mod, ServiceID, Action, AmountPec, Amount, CallLowerLimit, CallUpperLimit) "
									"VALUES({STRING}, {INT}, {INT}, 0, CONVERT(MONEY,{STRING}) * {INT}, NULL, NULL);" , 
									strSurtMod, nLastCPTID, (long)eActionType, strModAmount, nSuffix);
								bSavedModifier = TRUE; // (b.eyers 2016-01-19) - PLID 67853
							}
							bOnFirstLine = true;
						}
					}
					//TES 7/12/2013 - PLID 57501 - We're done with the SURT codes now
					bIsSurt = false;
				}
				else {
					BOOL bMatch = FALSE;				
					//long nTemp;
					if (mapAlbertaMods.Lookup(strMod, strMod) != 0)
					{
						bMatch = TRUE;
					}
					else
						bMatch = FALSE;
					
					if(!bMatch && strMod.Trim().CompareNoCase("") != 0 && !bIsSkll)
					{
						ADODB::_RecordsetPtr prs = CreateParamRecordset(
							"INSERT INTO CptModifierT (Number, Note, Multiplier) VALUES ({STRING}, '', 1) ", strMod);

						mapAlbertaMods.SetAt(strMod, strMod);
					}

					//get enumeration value from action type string
					// (j.jones 2011-10-10 17:58) - PLID 44941 - changed the return value
					eActionType = EnumerateActionType(strAction);

					//ASSERT(!strCPTCode.Trim().CompareNoCase("Y  3") == 0);
					//do not insert if its a Skll code type or the beginning of a call code or if there is no mod!
					if(!bIsSkll && !bIsCall && strMod.Trim().CompareNoCase("") != 0 && eActionType != -1)
					{
						//check to see if its a call code be checking upper call limit value
						if(nCallLower != -1)
						{
							//check to see if there is a upper limit
							if(nCallUpper != -1)
							{
								//if this was a percent mod update correct field
								if(nModPerc != -1)
								{
									ExecuteParamSql("INSERT INTO AlbertaCptModLinkT (Mod, ServiceID, Action, AmountPec, Amount, CallLowerLimit, CallUpperLimit) VALUES({STRING}, {INT}, {INT}, {INT}, CONVERT(MONEY,0), {INT}, {INT});" , strMod, nLastCPTID, (long)eActionType, nModPerc, nCallLower, nCallUpper);
									bSavedModifier = TRUE; // (b.eyers 2016-01-19) - PLID 67853
								}
								else
								{
									ExecuteParamSql("INSERT INTO AlbertaCptModLinkT (Mod, ServiceID, Action, AmountPec, Amount, CallLowerLimit, CallUpperLimit) VALUES({STRING}, {INT}, {INT}, 0, CONVERT(MONEY,{STRING}), {INT}, {INT});" , strMod, nLastCPTID, (long)eActionType, strModAmount, nCallLower, nCallUpper);
									bSavedModifier = TRUE; // (b.eyers 2016-01-19) - PLID 67853
								}
							}
							//no upper limit so insert null for that value
							else
							{
								//if this was a percent mod update correct field
								if(nModPerc != -1)
								{
									ExecuteParamSql("INSERT INTO AlbertaCptModLinkT (Mod, ServiceID, Action, AmountPec, Amount, CallLowerLimit, CallUpperLimit) VALUES({STRING}, {INT}, {INT}, {INT}, CONVERT(MONEY,0), {INT}, NULL);" , strMod, nLastCPTID, (long)eActionType, nModPerc, nCallLower);
									bSavedModifier = TRUE; // (b.eyers 2016-01-19) - PLID 67853
								}
								else
								{
									ExecuteParamSql("INSERT INTO AlbertaCptModLinkT (Mod, ServiceID, Action, AmountPec, Amount, CallLowerLimit, CallUpperLimit) VALUES({STRING}, {INT}, {INT}, 0, CONVERT(MONEY,{STRING}), {INT}, NULL);" , strMod, nLastCPTID, (long)eActionType, strModAmount, nCallLower);
									bSavedModifier = TRUE; // (b.eyers 2016-01-19) - PLID 67853
								}
							}
						}
						//not a call code, update with nulls for upper and lower call limits
						else
						{
							//if this was a percent mod update correct field
							if(nModPerc != -1)
							{
								ExecuteParamSql("INSERT INTO AlbertaCptModLinkT (Mod, ServiceID, Action, AmountPec, Amount, CallLowerLimit, CallUpperLimit) VALUES({STRING}, {INT}, {INT}, {INT}, CONVERT(MONEY,0), NULL, NULL);" , strMod, nLastCPTID, (long)eActionType, nModPerc);
								bSavedModifier = TRUE; // (b.eyers 2016-01-19) - PLID 67853
							}
							else
							{
								ExecuteParamSql("INSERT INTO AlbertaCptModLinkT (Mod, ServiceID, Action, AmountPec, Amount, CallLowerLimit, CallUpperLimit) VALUES({STRING}, {INT}, {INT}, 0, CONVERT(MONEY,{STRING}), NULL, NULL);" , strMod, nLastCPTID, (long)eActionType, strModAmount);
								bSavedModifier = TRUE; // (b.eyers 2016-01-19) - PLID 67853
							}
						}
					}
					progressDialog.SetProgress(nProgress++, nTotalRecords);
				}
			}
		}

		progressDialog.Stop();

		//send table checkers
		CClient::RefreshTable(NetUtils::CPTModifierT, -1);
		CClient::RefreshTable(NetUtils::CPTCodeT, -1);

		AfxMessageBox("The file has finished importing");
	
	//if there is an error make sure you kill the progress dialog
	}NxCatchAll(__FUNCTION__);

}

// (j.jones 2011-10-10 17:58) - PLID 44941 - changed the return value
AlbertaHLINKModifierActionType EnumerateActionType(CString strActionType)
{
	if(strActionType.CompareNoCase("Replace Base") == 0)
	{
		return amatReplaceBase;
	}
	else if(strActionType.CompareNoCase("Increase By") == 0)
	{
		return amatIncreaseBy;
	}
	else if(strActionType.CompareNoCase("For Each Call Pay Base At") == 0)
	{
		return amatForEachCallPayBaseAt;
	}
	else if(strActionType.CompareNoCase("For Each Call Increase By") == 0)
	{
		return amatForEachCallIncreaseBy;
	}
	else if(strActionType.CompareNoCase("Reduce Base To") == 0)
	{
		return amatReduceBaseTo;
	}
	else if(strActionType.CompareNoCase("Increase Base By") == 0)
	{
		return amatIncreaseBaseBy;
	}
	else if(strActionType.CompareNoCase("Increase Base To") == 0)
	{
		return amatIncreaseBaseTo;
	}
	else if(strActionType.CompareNoCase("MAX") == 0)
	{
		return amatMAX;
	}
	else if(strActionType.CompareNoCase("Reduce Base By") == 0)
	{
		return amatReduceBaseBy;
	}
	else if(strActionType.CompareNoCase("Replace") == 0)
	{
		return amatReplace;
	}
	else
	{
		//unknown action type
		//ASSERT(FALSE);
		return amatUnknown;
	}
}	

BOOL IsChapterTitle(CString strTrimedString)
{
	if(strTrimedString.Find("I. ") != -1 ||
		strTrimedString.Find("II. ") != -1 ||
		strTrimedString.Find("III. ") != -1 ||
		strTrimedString.Find("IV. ") != -1 ||
		strTrimedString.Find("V. ") != -1 ||
		strTrimedString.Find("VI. ") != -1 ||
		strTrimedString.Find("VII. ") != -1 ||
		strTrimedString.Find("VIII. ") != -1 ||
		strTrimedString.Find("IX. ") != -1 ||
		strTrimedString.Find("X. ") != -1 ||
		strTrimedString.Find("XI. ") != -1 ||
		strTrimedString.Find("XIII ") != -1 ||
		strTrimedString.Find("XIV ") != -1 ||
		strTrimedString.Find("XV. ") != -1 ||
		strTrimedString.Find("XVI. ") != -1 ||
		strTrimedString.Find("XVII. ") != -1 || 
		strTrimedString.Find("XVIII. ") != -1 )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//TES 9/24/2014 - PLID 62782 - Added
CString GetAlbertaBillStatusName(AlbertaBillingStatus eStatus)
{
	switch (eStatus) {
	case eEntered:
		return ALBERTA_ENTERED_STATUS_NAME;
		break;
	case ePending:
		return ALBERTA_PENDING_STATUS_NAME;
		break;
	case ePosted:
		return ALBERTA_POSTED_STATUS_NAME;
		break;
	case ePartiallyPaid:
		return ALBERTA_PARTIALLY_PAID_STATUS_NAME;
		break;
	case eRejected:
		return ALBERTA_REJECTED_STATUS_NAME;
		break;
	default:
		ASSERT(FALSE);
		return "";
	}
}