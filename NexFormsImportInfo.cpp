// NexFormsImportInfo.cpp: implementation of the CNexFormsImportInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <NxPracticeSharedLib/RichEditUtils.h>
#include "NexFormsImportInfo.h"
#include "MsgBox.h"
#include "ShowProgressFeedbackDlg.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"
#include "FileUtils.h"
#include <SharedTrackingUtils.h> // (z.manning 2011-06-24 15:51) - PLID 42916

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/********************************************************************************/
/*																				*/
/* For documentation on the NexForms importer/exporter, see:					*/
/*																				*/
/*		http://192.168.1.2/developers/doku.php?id=nexforms_importer_exporter	*/
/*																				*/
/********************************************************************************/


#define SET_IMPORT_PLAIN(field) \
	if(prsPlainTextFields->eof) { \
		procedure->bImport##field = TRUE; \
	} \
	else if(!procedure->str##field.IsEmpty()) { \
		procedure->bImport##field = AdoFldString(prsPlainTextFields, #field, "").IsEmpty(); \
	}

#define SET_IMPORT_RTF_KNOWN_VALUE(field) \
	if(prsPlainTextFields->eof) { \
		procedure->bImport##field = TRUE; \
	} \
	else if(!procedure->str##field.IsEmpty()) { \
		procedure->bImport##field = IsRichTextEmpty(AdoFldString(prsPlainTextFields, #field, "")); \
	}

#define SET_IMPORT_RTF(field) \
	if(!procedure->str##field.IsEmpty()) { \
		ADODB::_RecordsetPtr prs = CreateParamRecordset(pConn, FormatString( \
			"SELECT [%s] FROM ProcedureT WHERE ProcedureT.Name = {STRING} ", #field), procedure->strName); \
		if(prs->eof) { \
			procedure->bImport##field = TRUE; \
		} \
		else { \
			procedure->bImport##field = IsRichTextEmpty(AdoFldString(prs, #field, "")); \
		} \
		prs->Close(); \
	}

UINT NexFormsProcedurePostImport_Thread(LPVOID pParam)
{
	try
	{
		//Threads need to have their own connection, let's create one, based on the global connection.
		// (a.walling 2010-07-23 17:11) - PLID 39835 - Use GetThreadRemoteData to get a new connection using default values within a thread
		ADODB::_ConnectionPtr pConn = GetThreadRemoteData();

		NexFormsImportInfo *info = (NexFormsImportInfo*)pParam;

		for(int i = 0; i < info->m_arypProcedures.GetSize(); i++)
		{
			// (z.manning, 07/16/2007) - Make sure we aren't being destoryed, and if we are, then just quit.
			if(WaitForSingleObject(info->m_hevDestroying, 0) != WAIT_TIMEOUT) {
				return S_FALSE;
			}

			NexFormsProcedure *procedure = info->m_arypProcedures.GetAt(i);

			if(info->m_eImportType == nfitNew || info->m_eImportType == nfitNewNexFormsExistingTracking)
			{
				// (z.manning, 10/10/2007) - PLID 27717 - This is a new NexForms user so select to import
				// all fields that have content.

				procedure->bImportOfficialName = !procedure->strOfficialName.IsEmpty();
				procedure->bImportArrivalPrepMinutes = procedure->nArrivalPrepMinutes != 0;
				procedure->bImportCustom1 = !procedure->strCustom1.IsEmpty();
				procedure->bImportCustom2 = !procedure->strCustom2.IsEmpty();
				procedure->bImportCustom3 = !procedure->strCustom3.IsEmpty();
				procedure->bImportCustom4 = !procedure->strCustom4.IsEmpty();
				procedure->bImportCustom5 = !procedure->strCustom5.IsEmpty();
				procedure->bImportCustom6 = !procedure->strCustom6.IsEmpty();
				procedure->bImportAnesthesia = !procedure->strAnesthesia.IsEmpty();

				// (z.manning, 07/17/2007) - To accurately determine if a rich text field is empty, we would need
				// to convert it to plain text because there could very well be rtf formatting stuff but no actual
				// text. However, this was really slow when run for every field of every procedure, so I instead
				// have the NexFormsExporter check this and it will empty a field if there is no plain text. Thus,
				// we can assume that anything that isn't empty has text and avoid the speed hit here.
				procedure->bImportCustomSection1 = !procedure->strCustomSection1.IsEmpty();
				procedure->bImportCustomSection2 = !procedure->strCustomSection2.IsEmpty();
				procedure->bImportCustomSection3 = !procedure->strCustomSection3.IsEmpty();
				procedure->bImportCustomSection4 = !procedure->strCustomSection4.IsEmpty();
				procedure->bImportCustomSection5 = !procedure->strCustomSection5.IsEmpty();
				procedure->bImportCustomSection6 = !procedure->strCustomSection6.IsEmpty();
				procedure->bImportCustomSection7 = !procedure->strCustomSection7.IsEmpty();
				procedure->bImportCustomSection8 = !procedure->strCustomSection8.IsEmpty();
				procedure->bImportCustomSection9 = !procedure->strCustomSection9.IsEmpty();
				procedure->bImportCustomSection10 = !procedure->strCustomSection10.IsEmpty();
				procedure->bImportMiniDescription = !procedure->strMiniDescription.IsEmpty();
				procedure->bImportPreOp = !procedure->strPreOp.IsEmpty();
				procedure->bImportTheDayOf = !procedure->strTheDayOf.IsEmpty();
				procedure->bImportPostOp = !procedure->strPostOp.IsEmpty();
				procedure->bImportRecovery = !procedure->strRecovery.IsEmpty();
				// (z.manning, 09/04/2007) - PLID 27286 - Renamed field in data to ProcDetails.
				procedure->bImportProcDetails = !procedure->strProcDetails.IsEmpty();
				procedure->bImportRisks = !procedure->strRisks.IsEmpty();
				procedure->bImportAlternatives = !procedure->strAlternatives.IsEmpty();
				procedure->bImportComplications = !procedure->strComplications.IsEmpty();
				procedure->bImportSpecialDiet = !procedure->strSpecialDiet.IsEmpty();
				procedure->bImportShowering = !procedure->strShowering.IsEmpty();
				procedure->bImportBandages = !procedure->strBandages.IsEmpty();
				procedure->bImportConsent = !procedure->strConsent.IsEmpty();
				procedure->bImportHospitalStay = !procedure->strHospitalStay.IsEmpty();
				procedure->bImportAltConsent = !procedure->strAltConsent.IsEmpty();
			}
			else
			{
				// (z.manning, 10/10/2007) - PLID 27717 - This is an existing NexForms user. We only want to update
				// fields by default if it does not already have existing content.

				// (z.manning, 10/10/2007) - PLID 27717 - Load all of the commonly used fields from data to help
				// determine the fields we are importing by default. Note: We only load the ones that are used
				// frequently in official content. However, even if we don't load a field now and end up needing
				// it, we will load it on demand later.
				ADODB::_RecordsetPtr prsPlainTextFields = CreateParamRecordset(pConn,
					"SELECT OfficialName, ArrivalPrepMinutes, Custom1, Custom2, Custom3, Custom4, "
					"	Custom5, Custom6, Anesthesia, Alternatives, Consent, AltConsent, Recovery, PostOp "
					"FROM ProcedureT "
					"WHERE ProcedureT.Name = {STRING} "
					, procedure->strName);

				// (z.manning, 10/10/2007) - PLID 27717 - We don't use a macro for ArrivalPrepMinutes because it's the only non-string.
				if(procedure->nArrivalPrepMinutes != 0) {
					if(info->m_eImportType == nfitNew || info->m_eImportType == nfitNewNexFormsExistingTracking || prsPlainTextFields->eof) {
						procedure->bImportArrivalPrepMinutes = TRUE;
					}
					else {
						procedure->bImportArrivalPrepMinutes = AdoFldLong(prsPlainTextFields, "ArrivalPrepMinutes", 0) != 0;
					}
				}

				SET_IMPORT_PLAIN(OfficialName);
				SET_IMPORT_PLAIN(Custom1);
				SET_IMPORT_PLAIN(Custom2);
				SET_IMPORT_PLAIN(Custom3);
				SET_IMPORT_PLAIN(Custom4);
				SET_IMPORT_PLAIN(Custom5);
				SET_IMPORT_PLAIN(Custom6);
				SET_IMPORT_PLAIN(Anesthesia);
				SET_IMPORT_RTF_KNOWN_VALUE(Consent);
				SET_IMPORT_RTF_KNOWN_VALUE(AltConsent);
				SET_IMPORT_RTF_KNOWN_VALUE(Alternatives);
				SET_IMPORT_RTF_KNOWN_VALUE(Recovery);
				SET_IMPORT_RTF_KNOWN_VALUE(PostOp);
				SET_IMPORT_RTF(CustomSection1);
				SET_IMPORT_RTF(CustomSection2);
				SET_IMPORT_RTF(CustomSection3);
				SET_IMPORT_RTF(CustomSection4);
				SET_IMPORT_RTF(CustomSection5);
				SET_IMPORT_RTF(CustomSection6);
				SET_IMPORT_RTF(CustomSection7);
				SET_IMPORT_RTF(CustomSection8);
				SET_IMPORT_RTF(CustomSection9);
				SET_IMPORT_RTF(CustomSection10);
				SET_IMPORT_RTF(MiniDescription);
				SET_IMPORT_RTF(PreOp);
				SET_IMPORT_RTF(TheDayOf);
				SET_IMPORT_RTF(ProcDetails);
				SET_IMPORT_RTF(Risks);
				SET_IMPORT_RTF(Complications);
				SET_IMPORT_RTF(SpecialDiet);
				SET_IMPORT_RTF(Showering);
				SET_IMPORT_RTF(Bandages);
				SET_IMPORT_RTF(HospitalStay);

				prsPlainTextFields->Close();
			}
		}

		if(pConn != NULL) {
			pConn->Close();
		}

	}NxCatchAllThread("NexFormsProcedurePostImport_Thread");
	return S_OK;
}


BOOL ReadVersionInfoFromNexFormsContentFile(IN CFile *pFile, IN OUT NexFormsImportInfo &info)
{
	UINT nBytesRead = 0;

	// (z.manning, 10/17/2007) - The NexForms content file now has a string as the first part of the file
	// and if it's not present, we assume this is not a valid NexForms content data file.
	// (z.manning, 10/18/2007) - We don't use the READ_NEXFORMS_STRING because we need to check and make
	// sure the string size we we isn't too big.
	{
		DWORD dwSize;
		nBytesRead += pFile->Read(&dwSize, sizeof(DWORD));

		const CString strExpectedHeader(NEXFORMS_CONTENT_FILE_HEADER);
		if((int)dwSize != strExpectedHeader.GetLength()) {
			// (z.manning, 10/18/2007) - We didn't even get the correct string size, so no way this is a valid 
			// NexForms content file.
			return FALSE;
		}

		CString strHeader;
		// (a.walling 2008-10-24 11:46) - PLID 31823 - properly append a null terminator to the string to prevent heap corruption / buffer overflows
		char* szBuffer = strHeader.GetBuffer(dwSize + 1);
		nBytesRead += pFile->Read(szBuffer, dwSize);
		szBuffer[dwSize] = '\0';
		DecryptNexFormsString(szBuffer);
		strHeader.ReleaseBuffer();

		if(strHeader != strExpectedHeader) {
			// (z.manning, 10/18/2007) - The strings don't match so we assume this is not a valid NexForms content file.
			return FALSE;
		}
	}

	// (z.manning, 07/12/2007) - Read the versions.
	// (z.manning, 07/12/2007) - 4 bytes for code version
	nBytesRead += pFile->Read(&info.m_nCodeVersion, sizeof(DWORD));

	// (z.manning, 07/12/2007) - 4 bytes for content file version
	nBytesRead += pFile->Read(&info.m_nContentFileVersion, sizeof(DWORD));

	// (z.manning, 07/12/2007) - N bytes for ASPS version
	// (z.manning, 10/15/2007) - PLID 27751 - We now have a macro to read strings from the NexForms data file.
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(info.m_strAspsVersion, nBytesRead, pFile);

	// (z.manning, 07/12/2007) - 4 bytes for NexForms version
	nBytesRead += pFile->Read(&info.m_nNexFormsVersion, sizeof(DWORD));

	// (z.manning, 10/17/2007) - We already verified the header, so if we got this far, we presume a valid
	// NexForms content file and thus all the versions are valid as well.
	return TRUE;
}

BOOL ReadVersionInfoFromNexFormsContentFile(IN CString strFile, OUT long &nCodeVersion, OUT long &nContentFileVersion, OUT CString &strAspsVersion, OUT long &nNexFormsVersion)
{
	CFile fDat(strFile, CFile::modeRead|CFile::shareDenyNone);

	NexFormsImportInfo info;
	BOOL bReturn = ReadVersionInfoFromNexFormsContentFile(&fDat, info);

	nCodeVersion = info.m_nCodeVersion;
	nContentFileVersion = info.m_nContentFileVersion;
	strAspsVersion = info.m_strAspsVersion;
	nNexFormsVersion = info.m_nNexFormsVersion;
	
	fDat.Close();

	return bReturn;
}

void LoadFromNexFormsContentFile(IN CString strFile, OUT NexFormsImportInfo &info)
{
	CWaitCursor wc;
	CFile fDat(strFile, CFile::modeRead|CFile::shareDenyNone);

	//TES 11/7/2007 - PLID 27979 - VS2008 - CFiles now report their lengths as ULONGLONGs, rather than DWORDs.
	// Let's check right now that the file is small enough to be expressed as DWORDs, then we can safely
	// cast any length/position variables as DWORDs for the rest of this function; this file shouldn't be over
	// 4 GB anyway.
	ULONGLONG ullLength = fDat.GetLength();
	if(ullLength > ULONG_MAX) {
		AfxThrowNxException("LoadFromNexFormsContentFile() - Found file larger than maximum supported size (4 GB).");
	}

	info.CleanUpNexFormsContent();

	// (z.manning, 07/18/2007) - PLID 26729 - Added a progress bar when loading from the file.
	CShowProgressFeedbackDlg dlgProgress(0, FALSE);
	dlgProgress.ShowProgress(SW_SHOW);
	dlgProgress.SetCaption("Loading NexForms Content");
	CProgressParameter pp;
	pp.Init(&dlgProgress, 0, 100);

	// (z.manning, 07/12/2007) - Read the version stuff
	if(!ReadVersionInfoFromNexFormsContentFile(&fDat, info)) {
		// (z.manning, 10/17/2007) - If we have an invalid Nexforms file it should have been detected
		// before we ever tried to load from it. So if we fail here, let's go ahead and throw an exception.
		ASSERT(FALSE);
		AfxThrowNxException("::LoadFromNexFormsContentFile - Invalid NexForms content file detected.");
	}

	// (z.manning, 07/09/2007) - Procedures.
	long nProcedureCount;
	fDat.Read(&nProcedureCount, sizeof(DWORD));
	pp.SetSubRange(0, 40);
	for(int nProcedureIndex = 0; nProcedureIndex < nProcedureCount; nProcedureIndex++)
	{
		NexFormsProcedure *procedure = new NexFormsProcedure;
		ReadNexFormsProcedureFromFile(&fDat, procedure);
		info.m_arypProcedures.Add(procedure);
		pp.SetProgress(0, nProcedureCount, nProcedureIndex + 1);
	}
	
	// (z.manning, 07/16/2007) - Ok, all procedure info has been run. Now, in another thread, let's load
	// the default status for what fields we should and should not import for each procedure.
	info.m_pPostProcedureThread = AfxBeginThread(NexFormsProcedurePostImport_Thread, &info, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
	info.m_pPostProcedureThread->m_bAutoDelete = FALSE;
	info.m_pPostProcedureThread->ResumeThread();

	// (z.manning, 07/09/2007) - Tracking Ladders.
	long nLadderCount;
	fDat.Read(&nLadderCount, sizeof(DWORD));
	pp.SetSubRange(40, 43);
	for(int nLadderIndex = 0; nLadderIndex < nLadderCount; nLadderIndex++)
	{
		NexFormsLadder *ladder = new NexFormsLadder;
		ReadNexFormsLadderFromFile(&fDat, ladder);
		info.m_arypLadders.Add(ladder);
		pp.SetProgress(0, nLadderCount, nLadderIndex + 1);
	}

	// (z.manning, 07/09/2007) - Packets.
	long nPacketCount;
	fDat.Read(&nPacketCount, sizeof(DWORD));
	pp.SetSubRange(43, 45);
	for(int nPacketIndex = 0; nPacketIndex < nPacketCount; nPacketIndex++)
	{
		NexFormsPacket *packet = new NexFormsPacket;
		ReadNexFormsPacketFromFile(&fDat, packet);
		info.m_arypPackets.Add(packet);
		pp.SetProgress(0, nPacketCount, nPacketIndex + 1);
	}

	// (z.manning, 07/09/2007) - Word templates.
	BOOL bEof = FALSE;
	DWORD dwStartPosition = (DWORD)fDat.GetPosition();
	pp.SetSubRange(45, 100);
	while(!bEof)
	{
		NexFormsWordTemplate *word = new NexFormsWordTemplate;
		if(ReadNexFormsWordTemplateFromFile(&fDat, word) > 0)
		{
			word->ParseRelativePath();
			info.m_arypWordTemplates.Add(word);
		}
		else
		{
			delete word;
			bEof = TRUE;
		}
		pp.SetProgress(dwStartPosition, (DWORD)fDat.GetLength(), (DWORD)fDat.GetPosition());
	}

	fDat.Close();
}

// (z.manning, 08/30/2007) - PLID 18359 - Created a new importer for NexForms.

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

NexFormsImportInfo::NexFormsImportInfo()
{
	m_nCodeVersion = 0;
	m_nContentFileVersion = 0;
	m_nNexFormsVersion = 0;
	m_pPostProcedureThread = NULL;
	m_hevDestroying = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_nAuditTransactionID = -1;
	m_eImportType = nfitExisting;
	m_bPacketsAndTemplatesHaveBeenSuccessfullyImported = FALSE;
	m_bUpdateExistingProceduresFont = FALSE; // (z.manning, 10/11/2007) - PLID 27719
}

NexFormsImportInfo::~NexFormsImportInfo()
{	
	CleanUpExistingLadders();
	CleanUpNexFormsContent();
}

void NexFormsImportInfo::CleanUpExistingLadders()
{
	for(int i = 0; i < m_aryExistingLadders.GetSize(); i++)
	{
		if(m_aryExistingLadders.GetAt(i) != NULL) {
			delete m_aryExistingLadders.GetAt(i);
		}
	}
	m_aryExistingLadders.RemoveAll();
}

#define ADD_STRING_FIELD_TO_UPDATE(field) \
	if(procedure->bImport##field || procedure->nExistingProcedureID == 0) { \
		strUpdateProcedureSql += FormatString("%s = '%s', \r\n", #field, _Q(procedure->str##field)); \
	} \

#define ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(field) ADD_NEXFORMS_STRING_FIELD_TO_UPDATE_DIFFERENT_REVIEW(field, field)

#define ADD_NEXFORMS_STRING_FIELD_TO_UPDATE_DIFFERENT_REVIEW(field, review_suffix) \
	ADD_STRING_FIELD_TO_UPDATE(field); \
	if(procedure->bImport##field && !procedure->str##field.IsEmpty()) { \
		strUpdateProcedureSql += FormatString("Review%s = %li, \r\n", #review_suffix, procedure->bNeedReview##field ? 1 : 0); \
	} \

void NexFormsImportInfo::ImportProceduresAndLadders(CShowConnectingFeedbackDlg *pFeedback)
{
	// (z.manning, 07/31/2007) - PLID 26884 - These booleans will tell us whether or not these things
	// changed at all to determine if we need to use a table checker later.
	BOOL bProceduresChanged = FALSE, bLaddersChanged = FALSE;

	// (z.manning, 06/28/2007) - Create table variables to map "export" IDs to the new real IDs we're about
	// to create for new procedures and tracking ladders.
	CString strSaveSql = BeginSqlBatch();
	AddDeclarationToSqlBatch(strSaveSql, "DECLARE @LadderIDMapT TABLE (ExportID INT NOT NULL, DataID INT NOT NULL)");
	AddDeclarationToSqlBatch(strSaveSql, "DECLARE @ProcedureIDMapT TABLE (ExportID INT NOT NULL, DataID INT NOT NULL)");
	AddDeclarationToSqlBatch(strSaveSql, "DECLARE @nLadderID INT");
	AddDeclarationToSqlBatch(strSaveSql, "DECLARE @nStepID INT");
	AddDeclarationToSqlBatch(strSaveSql, "DECLARE @nProcedureID INT");
	AddDeclarationToSqlBatch(strSaveSql, "DECLARE @nMasterProcedureID INT");
	AddDeclarationToSqlBatch(strSaveSql, "DECLARE @nActionID INT");
	AddDeclarationToSqlBatch(strSaveSql, "DECLARE @nExistingLadderID INT");
	AddDeclarationToSqlBatch(strSaveSql, "DECLARE @nCounter INT");
	// (z.manning, 07/30/2007) - PLID 26869 - Since we do everything in one giant transaction, we don't know
	// the record IDs for auditing in the middle of generating the sql (at least not without running a query
	// every time). So, let's just make a table variable to store all the stuff we need to audit and then handle
	// it at the end.
	AddDeclarationToSqlBatch(strSaveSql, 
		"DECLARE @AuditInfoT TABLE \r\n"
		"( \r\n"
		"	Item INT, \r\n"
		"	RecordID INT, \r\n"
		"	OldValue nvarchar(255), \r\n"
		"	NewValue nvarchar(255) \r\n"
		") "
		);

	////////////////////////////// BEGIN LADDERS //////////////////////////////////////
	// We can only add new ladders, no way to change existing ladders.
	for(int nLadderIndex = 0; nLadderIndex < m_arypLadders.GetSize(); nLadderIndex++)
	{
		NexFormsLadder *ladder = m_arypLadders.GetAt(nLadderIndex);
		// (z.manning, 06/28/2007) - Make sure we want to import it.
		if(ladder->bImport)
		{
			// (z.manning, 07/31/2007) - PLID 26884 - We're importing at least one ladder, so make sure
			// we note that so we can send a table checker later.
			bLaddersChanged = TRUE;

			if(ladder->bDeleteExistingLadder)
			{
				AddStatementToSqlBatch(strSaveSql, "SET @nExistingLadderID = (SELECT TOP 1 ID FROM LadderTemplatesT WHERE Name = '%s')", _Q(ladder->strName));
				// (j.jones 2008-11-26 13:44) - PLID 30830 - supported multi-users
				AddStatementToSqlBatch(strSaveSql,
					"IF @nExistingLadderID IS NOT NULL BEGIN \r\n"
					"	DELETE FROM TrackingConversionT WHERE LadderTemplateID = @nExistingLadderID; \r\n"
					"	DELETE FROM StepTemplatesAssignToT WHERE StepTemplateID IN (SELECT ID FROM StepTemplatesT WHERE LadderTemplateID = @nExistingLadderID); \r\n"
					"	DELETE FROM StepCriteriaT WHERE StepTemplateID IN (SELECT ID FROM StepTemplatesT WHERE LadderTemplateID = @nExistingLadderID); \r\n"
					"	DELETE FROM StepTemplatesT WHERE LadderTemplateID = @nExistingLadderID; \r\n"
					"	DELETE FROM LadderTemplatesT WHERE ID = @nExistingLadderID; \r\n"
					"END ");
			}

			// (z.manning, 06/28/2007) - Create the ladder record.
			AddStatementToSqlBatch(strSaveSql, "SET @nLadderID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM LadderTemplatesT)");
			AddStatementToSqlBatch(strSaveSql, "INSERT INTO LadderTemplatesT (ID, Name, AutoCreate) VALUES (@nLadderID, '%s', %i)", _Q(ladder->strName), ladder->bAutoCreate ? 1 : 0);
			AddStatementToSqlBatch(strSaveSql, "INSERT INTO @LadderIDMapT (ExportID, DataID) VALUES (%li, @nLadderID)", ladder->nID);

			// (z.manning, 07/30/2007) - PLID 26869 - Store audit info the the newly created ladder
			AddStatementToSqlBatch(strSaveSql,
				"INSERT INTO @AuditInfoT (Item, RecordID, OldValue, NewValue) VALUES (%li, @nLadderID, '', '%s') "
				,aeiLadderCreated, _Q(ladder->strName));

			// (z.manning, 06/28/2007) - All steps for this ladder.
			for(int nStepIndex = 0; nStepIndex < ladder->arypSteps.GetSize(); nStepIndex++)
			{
				NexFormsLadderStep *step = ladder->arypSteps.GetAt(nStepIndex);

				CString strActivateInterval = step->nActivateInterval == -1 ? "NULL" : AsString(step->nActivateInterval);
				
				AddStatementToSqlBatch(strSaveSql, "SET @nStepID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM StepTemplatesT)");
				// (j.jones 2008-11-17 17:27) - PLID 30926 - added OpenPIC, which defaults to 0
				AddStatementToSqlBatch(strSaveSql, 
					"INSERT INTO StepTemplatesT (ID, LadderTemplateID, StepOrder, StepName, Action, "
					"	Note, ActivateType, ActivateInterval, ActivateStringData, Skippable, OpenPIC) "
					"VALUES (@nStepID, @nLadderID, %li, '%s', %li, '%s', %li, %s, '%s', %li, 0) \r\n"
					, step->nStepOrder, _Q(step->strStepName), step->nAction, _Q(step->strNote)
					, step->nActivateType, strActivateInterval, _Q(step->strActivateStringData)
					, step->bSkippable ? 1 : 0);

				// (z.manning, 06/28/2007) - Now all actions for this step.
				for(int nActionIndex = 0; nActionIndex < step->aryvarActions.GetSize(); nActionIndex++)
				{
					switch(step->nAction)
					{
						case PhaseTracking::PA_WritePacket:
							AddStatementToSqlBatch(strSaveSql,
								"SET @nActionID = (SELECT TOP 1 ID FROM PacketsT WHERE Name = '%s')",
								_Q(VarString(step->aryvarActions.GetAt(nActionIndex))));
							break;

						case PhaseTracking::PA_WriteTemplate:
							AddStatementToSqlBatch(strSaveSql,
								"SET @nActionID = (SELECT TOP 1 ID FROM MergeTemplatesT WHERE Path = '%s') \r\n"
								"IF @nActionID IS NULL BEGIN \r\n"
								"	SET @nActionID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM MergeTemplatesT) \r\n"
								"	INSERT INTO MergeTemplatesT (ID, Path) VALUES (@nActionID, '%s') \r\n"
								"END ", _Q(VarString(step->aryvarActions.GetAt(nActionIndex)))
								, _Q(VarString(step->aryvarActions.GetAt(nActionIndex))) );
							break;
				
						default:
							AddStatementToSqlBatch(strSaveSql,
								"SET @nActionID = %li", VarLong(step->aryvarActions.GetAt(nActionIndex)));
							break;
					}

					AddStatementToSqlBatch(strSaveSql,
						"INSERT INTO StepCriteriaT (StepTemplateID, ActionID) VALUES (@nStepID, @nActionID)");
				}
			}
		}
	}
	/////////////////////////////////// END LADDERS ////////////////////////////////////////////


	//////////////////////////////// BEGIN PROCEDURES /////////////////////////////////////////
	NexFormsProcedure *procedureDetailPivot = NULL;
	BOOL bOnlyDetailProceduresRemain = FALSE;
	// (c.haag 2009-01-07 15:28) - PLID 32647 - If this is TRUE, we need to get a list of inactive
	// procedures before we run the big query. Then after the big query has run, we can see which
	// ones were since activated, and report them to the user.
	BOOL bCheckExistingInactiveProcedures = FALSE;

	for(int nProcedureIndex = 0; nProcedureIndex < m_arypProcedures.GetSize(); nProcedureIndex++)
	{
		NexFormsProcedure *procedure = m_arypProcedures.GetAt(nProcedureIndex);

		// (z.manning, 07/02/2007) - If this is a detail procedure, then move it to the end in case
		// we've not yet inserted the master procedure it's references. Unless, of course, we only
		// have master procedures left.
		if(procedure->nMasterProcedureID != 0 && !bOnlyDetailProceduresRemain)
		{
			if(procedureDetailPivot != NULL && procedureDetailPivot == procedure)
			{
				// (z.manning, 07/02/2007) - Ok, we should only have detail procedures left, so we can
				// stop moving them to the end of the array.
				bOnlyDetailProceduresRemain = TRUE;
			}
			else
			{
				// (z.manning, 07/02/2007) - Let's keep track of the first detail procedure that we move to
				// the end of the array because, we'll know that once we've hit this, there are only detail
				// procedures remaining.
				if(procedureDetailPivot == NULL) {
					procedureDetailPivot = m_arypProcedures.GetAt(nProcedureIndex);
				}
				// (z.manning, 07/02/2007) - Found a master procedure, let's move the current
				// detail procedure to the end of the array.
				m_arypProcedures.InsertAt(m_arypProcedures.GetSize(), m_arypProcedures.GetAt(nProcedureIndex));
				m_arypProcedures.RemoveAt(nProcedureIndex);
				nProcedureIndex--;
				// (z.manning, 07/02/2007) - Now just move onto the next procedure.
				continue;
			}
		}

		// (z.manning, 06/28/2007) - Make sure we want to import it.
		if(procedure->bImport)
		{
			// (z.manning, 07/31/2007) - PLID 26884 - We're importing a procedure so make sure we note
			// that so we can send a table checker later.
			bProceduresChanged = TRUE;

			// (z.manning, 06/28/2007) - Are we inserting a new procedure?
			if(procedure->nExistingProcedureID == 0)
			{
				// (z.manning, 06/28/2007) - Inserting new. WE DO NOT handle MasterProcedureID yet because
				// the master procedure may be one we haven't imported yet.
				// (z.manning, 09/21/2007) - Make sure we get the new ID from AptPurposeT and not ProcedureT
				// because we could get PK errors since AptPurposeT uses the same ID but can have records
				// that aren't in ProcedureT.
				AddStatementToSqlBatch(strSaveSql, "SET @nProcedureID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM AptPurposeT)");

				// (z.manning, 09/21/2007) - We need to handle the case when there's a non-procedural appt purpose
				// with the same name as the procedure we're importing or else we can get unique name contrainst
				// violations. This should be very unlikely, and I don't want to prompt the user about this, so let's
				// just silently rename the non-procedural appt purpose.
				AddStatementToSqlBatch(strSaveSql,
					"IF EXISTS (SELECT Name FROM AptPurposeT WHERE Name = '%s') BEGIN \r\n"
					"   SET @nCounter = 2 \r\n"
					"   WHILE EXISTS (SELECT Name FROM AptPurposeT WHERE Name = '%s' + convert(nvarchar(5), @nCounter)) BEGIN \r\n"
					"	  SET @nCounter = @nCounter + 1 \r\n"
					"   END \r\n"
					"   UPDATE AptPurposeT SET Name = '%s' + convert(nvarchar(5), @nCounter) WHERE Name = '%s' \r\n"
					"END "
					, _Q(procedure->strName), _Q(procedure->strName), _Q(procedure->strName), _Q(procedure->strName));

				AddStatementToSqlBatch(strSaveSql, "INSERT INTO AptPurposeT (ID, Name) VALUES (@nProcedureID, '%s')", _Q(procedure->strName));
				AddStatementToSqlBatch(strSaveSql,
					"INSERT INTO ProcedureT (ID, Name, OfficialName, Recur, EMRPreferredProcedure) \r\n"
					"VALUES (@nProcedureID, '%s', '%s', %li, %li) \r\n"
					, _Q(procedure->strName), _Q(procedure->strOfficialName)
					, procedure->bRecur ? 1 : 0, procedure->bEmrPreferredProcedure ? 1 : 0
					);
				AddStatementToSqlBatch(strSaveSql,
					"INSERT INTO @ProcedureIDMapT (ExportID, DataID) VALUES (%li, @nProcedureID) \r\n"
					, procedure->nID);
			}
			else
			{
				// (z.manning, 06/28/2007) - Ok, we're updating an existing procedure.
				ASSERT(procedure->nExistingProcedureID > 0);

				AddStatementToSqlBatch(strSaveSql, "SET @nProcedureID = %li", procedure->nExistingProcedureID);

				// (z.manning, 07/03/2007) - We'll be assigning ladders in a bit, but clear out any old ones now.
				AddStatementToSqlBatch(strSaveSql, "DELETE FROM ProcedureLadderTemplateT WHERE ProcedureID = @nProcedureID");

				// (c.haag 2009-01-07 15:22) - PLID 32647 - Make sure this existing procedure is activated
				AddStatementToSqlBatch(strSaveSql, "UPDATE ProcedureT SET Inactive = 0, InactivatedDate = NULL, InactivatedBy = NULL WHERE ID = @nProcedureID");
				bCheckExistingInactiveProcedures = TRUE;
			}

			// (z.manning, 07/02/2007) - Ok, we have a procedure in data. Now update all the nexforms fields.
			// (z.manning, 07/23/2007) - PLID 26774 - Update the NexFormsVersion field
			CString strUpdateProcedureSql = FormatString("UPDATE ProcedureT SET NexFormsVersion = %li,\r\n", m_nNexFormsVersion);

			if(procedure->nMasterProcedureID != 0)
			{
				// (z.manning, 07/02/2007) - First, handle master procedure ID. Including finding the mapped ID
				// if it's a newly inserted procedure.
				AddStatementToSqlBatch(strSaveSql,
					"SET @nMasterProcedureID = %li \r\n"
					"IF @nMasterProcedureID < 0 BEGIN \r\n"
					"	SET @nMasterProcedureID = (SELECT TOP 1 DataID FROM @ProcedureIDMapT WHERE ExportID = @nMasterProcedureID) \r\n"
					"END \r\n"
					, procedure->nMasterProcedureID);

				strUpdateProcedureSql += " MasterProcedureID = @nMasterProcedureID,";
			}

			// (z.manning, 07/25/2007) - Are we updating the procedure name?
			if(procedure->bUpdateName) {
				strUpdateProcedureSql += FormatString("Name = '%s', \r\n", _Q(procedure->strName));

				// (z.manning, 09/21/2007) - We need to handle the case when there's a non-procedural appt purpose
				// with the same name as the procedure we're importing or else we can get unique name contrainst
				// violations. This should be very unlikely, and I don't want to prompt the user about this, so let's
				// just silently rename the non-procedural appt purpose.
				AddStatementToSqlBatch(strSaveSql,
					"IF EXISTS (SELECT Name FROM AptPurposeT WHERE Name = '%s') BEGIN \r\n"
					"   SET @nCounter = 2 \r\n"
					"   WHILE EXISTS (SELECT Name FROM AptPurposeT WHERE Name = '%s' + convert(nvarchar(5), @nCounter)) BEGIN \r\n"
					"	  SET @nCounter = @nCounter + 1 \r\n"
					"   END \r\n"
					"   UPDATE AptPurposeT SET Name = '%s' + convert(nvarchar(5), @nCounter) WHERE Name = '%s' \r\n"
					"END "
					, _Q(procedure->strName), _Q(procedure->strName), _Q(procedure->strName), _Q(procedure->strName));

				AddStatementToSqlBatch(strSaveSql, "UPDATE AptPurposeT SET Name = '%s' WHERE ID = @nProcedureID", _Q(procedure->strName));
			}

			if(procedure->bImportArrivalPrepMinutes) {
				strUpdateProcedureSql += FormatString("ArrivalPrepMinutes = %li, \r\n", procedure->nArrivalPrepMinutes);
			}
			ADD_STRING_FIELD_TO_UPDATE(Anesthesia);
			ADD_STRING_FIELD_TO_UPDATE(OfficialName);
			ADD_STRING_FIELD_TO_UPDATE(Custom1);
			ADD_STRING_FIELD_TO_UPDATE(Custom2);
			ADD_STRING_FIELD_TO_UPDATE(Custom3);
			ADD_STRING_FIELD_TO_UPDATE(Custom4);
			ADD_STRING_FIELD_TO_UPDATE(Custom5);
			ADD_STRING_FIELD_TO_UPDATE(Custom6);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(CustomSection1);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(CustomSection2);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(CustomSection3);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(CustomSection4);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(CustomSection5);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(CustomSection6);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(CustomSection7);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(CustomSection8);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(CustomSection9);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(CustomSection10);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE_DIFFERENT_REVIEW(MiniDescription, Mini);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(PreOp);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE_DIFFERENT_REVIEW(TheDayOf, DayOf);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(PostOp);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(Recovery);
			// (z.manning, 09/04/2007) - PLID 27286 - Renamed field in data to ProcDetails.
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(ProcDetails);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(Risks);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(Alternatives);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(Complications);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE_DIFFERENT_REVIEW(SpecialDiet, Diet);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(Showering);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(Bandages);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(Consent);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(HospitalStay);
			ADD_NEXFORMS_STRING_FIELD_TO_UPDATE(AltConsent);
			
			if(strUpdateProcedureSql.Find('=') == -1)
			{
				// (z.manning, 06/29/2007) - If we don't find an equals sign, we're not updating anything
				// (z.manning, 07/30/2007) - This should not be possible as we update the NexForms version
				// no matter what if nothing else.
				ASSERT(FALSE);
			}
			else
			{
				// (z.manning, 07/02/2007) - Need to get rid of the extra comma in the update statement.
				strUpdateProcedureSql.TrimRight();
				strUpdateProcedureSql.TrimRight(',');
				strUpdateProcedureSql += "\r\nWHERE ProcedureT.ID = @nProcedureID ";
				AddStatementToSqlBatch(strSaveSql, "%s", strUpdateProcedureSql);
				
				// (z.manning, 07/30/2007) - PLID 26869 - Store audit info the the newly created procedure
				if(procedure->nExistingProcedureID > 0)
				{
					AddStatementToSqlBatch(strSaveSql,
						"INSERT INTO @AuditInfoT (Item, RecordID, OldValue, NewValue) "
						"SELECT %li, @nProcedureID, COALESCE(ProcedureT.Name, ''), CASE WHEN %li = 1 THEN '%s' ELSE COALESCE(ProcedureT.Name, '') END "
						"FROM ProcedureT WHERE ProcedureT.ID = %li "
						, aeiImportNexFormsProcedure, procedure->bUpdateName ? 1 : 0
						, _Q(procedure->strName), procedure->nExistingProcedureID);
				}
				else
				{
					AddStatementToSqlBatch(strSaveSql,
						"INSERT INTO @AuditInfoT (Item, RecordID, OldValue, NewValue) "
						"VALUES (%li, @nProcedureID, '', '%s') "
						, aeiImportNexFormsProcedure, _Q(procedure->strName));
				}
			}
			
			// (z.manning, 06/28/2007) - Can have multiple ladders assigned to a procedure.
			for(int nProcedureLadderIndex = 0; nProcedureLadderIndex < procedure->arynLadderTemplateIDs.GetSize(); nProcedureLadderIndex++)
			{
				// (z.manning, 06/28/2007) - We may want to assign a new ladder, so we need to use
				// our mapping if so.
				long nLadderID = procedure->arynLadderTemplateIDs.GetAt(nProcedureLadderIndex);
				// (z.manning, 06/29/2007) - An ID of 0 means no ladder.
				// (z.manning, 08/01/2007) - Only master procedures should have tracking ladders.
				if(nLadderID != 0 && procedure->nMasterProcedureID == 0)
				{
					AddStatementToSqlBatch(strSaveSql,
						"SET @nLadderID = %li \r\n"
						"IF @nLadderID < 0 BEGIN \r\n"
						"	SET @nLadderID = (SELECT TOP 1 DataID FROM @LadderIDMapT WHERE ExportID = @nLadderID) \r\n"
						"END \r\n"
						"IF @nLadderID IS NOT NULL \r\n"
						"	INSERT INTO ProcedureLadderTemplateT (ProcedureID, LadderTemplateID) VALUES (@nProcedureID, @nLadderID) \r\n"
						, nLadderID);
				}
				else {
					ASSERT(FALSE);
				}
			}
		}
	}
	//////////////////////////////////END PROCEDURES //////////////////////////////////////////

	// (z.manning, 07/13/2007) - Update the ASPS version.
	AddStatementToSqlBatch(strSaveSql,
		"IF EXISTS (SELECT Name FROM ConfigRT WHERE Name = 'ASPSConsentVersion') \r\n"
		"	UPDATE ConfigRT SET TextParam = '%s' WHERE Name = 'ASPSConsentVersion' \r\n"
		"ELSE \r\n"
		"	INSERT INTO ConfigRT (UserName, Name, Number, TextParam) VALUES ('<None>', 'ASPSConsentVersion', 0, '%s') "
		, _Q(m_strAspsVersion), _Q(m_strAspsVersion) );

#ifdef _DEBUG
		CMsgBox dlg(NULL);
		dlg.msg = "BEGIN TRAN \r\n" + strSaveSql + "COMMIT TRAN \r\n";
		// (z.manning, 10/17/2007) - I commented out this line because it causes weird focus issues
		// with the please wait screen, however, I left this code here in case someone ever wants
		// to temporarily uncomment this to easily be able to view the SQL statement.
		//dlg.DoModal();
#endif //_DEBUG


	// (c.haag 2009-01-07 15:27) - PLID 32647 - Just before we run the giant query, get a list of all the inactive
	// procedures. Ones that apply to the import will later be activated, and we want to report them to the user later
	CArray<long,long> anInactiveProcedures;
	if (bCheckExistingInactiveProcedures) {
		ADODB::_RecordsetPtr prs = CreateRecordset("SELECT ID FROM ProcedureT WHERE Inactive = 1");
		ADODB::FieldsPtr f = prs->Fields;
		while (!prs->eof) {
			anInactiveProcedures.Add(AdoFldLong(f, "ID"));
			prs->MoveNext();
		}
	}

	// (z.manning, 07/02/2007) - This could potentially take quite a while, so let's temporarily up the 
	// timeout to 10 minutes.
	long nRealCommandTimeout = GetRemoteData()->GetCommandTimeout();
	GetRemoteData()->PutCommandTimeout(600);

	// (z.manning, 07/30/2007) - PLID 26869 - In addition to executing the sql batch, also select all 
	// the audit info.
	ADODB::_RecordsetPtr prsSave = CreateRecordset(
		"BEGIN TRAN \r\n"
		"SET NOCOUNT ON \r\n"
		"%s \r\n"
		"COMMIT TRAN \r\n"
		"SET NOCOUNT OFF \r\n"
		"SELECT * FROM @AuditInfoT "
		, strSaveSql);

	GetRemoteData()->PutCommandTimeout(nRealCommandTimeout);

	// (z.manning, 07/30/2007) - PLID 26869 - Go through all the stuff we supposed to audit and add it to
	// the class' audit trasaction.
	for(; !prsSave->eof; prsSave->MoveNext()) {
		ADODB::FieldsPtr flds = prsSave->Fields;
		NexFormsImportAuditEvent(AdoFldLong(flds, "Item"), AdoFldLong(flds, "RecordID"), AdoFldString(flds, "OldValue", ""), AdoFldString(flds, "NewValue"));
	}

	// (c.haag 2009-01-07 15:31) - PLID 32647 - Now that all the work has been done, pull up a list of
	// inactive procedures that became active, and report them to the user
	if (anInactiveProcedures.GetSize() > 0)
	{
		ADODB::_RecordsetPtr prs = CreateRecordset("SELECT ID, Name FROM ProcedureT WHERE Inactive = 0 AND ID IN (%s) ORDER BY Name", ArrayAsString(anInactiveProcedures));
		if (!prs->eof) {
			CString strMsg = "The following procedures were reactivated during the import:\r\n\r\n";
			ADODB::FieldsPtr f = prs->Fields;
			long nAuditTransactionID = -1;
			int nLines = 0;

			try {
				while (!prs->eof) {
					long nID = AdoFldLong(f, "ID");
					CString strProc = AdoFldString(f, "Name");

					// Update the message
					nLines++;
					if (nLines < 20) {
						strMsg += strProc + "\r\n";
					} else if (nLines == 20) {
						strMsg += "<more names were omitted>\r\n";
						break;
					} else {
						// Too many names
					}

					// Audit prep
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(-1, "", nAuditTransactionID, aeiSchedProcInactivated, nID, "<Inactivated>", strProc, aepHigh, aetChanged);

					// Go to the next record
					prs->MoveNext();
				}
				if(nAuditTransactionID != -1) {
					CommitAuditTransaction(nAuditTransactionID);
				}
			}
			catch (...) {
				if (nAuditTransactionID != -1) {
					RollbackAuditTransaction(nAuditTransactionID);
				}
				throw;
			}
			// (c.haag 2009-02-03 15:05) - PLID 32647 - Hide the progress feedback dialog before popping up the message
			pFeedback->ShowWindow(SW_HIDE);
			MsgBox(MB_ICONWARNING, strMsg);
			pFeedback->ShowWindow(SW_RESTORE);
		} // if (!prs->eof) {
	} // if (anInactiveProcedures.GetSize() > 0)

	// (z.manning, 07/31/2007) - PLID 26884 - Send table checkers if needed.
	if(bProceduresChanged) {
		CClient::RefreshTable(NetUtils::AptPurposeT);
	}
	if(bLaddersChanged) {
		CClient::RefreshTable(NetUtils::Ladders);
	}
}

BOOL NexFormsImportInfo::ImportPacketsAndTemplates(CWnd *pwndParent, CShowConnectingFeedbackDlg *pFeedback)
{
	// (z.manning, 07/09/2007) - Let's do word templates first.
	for(int nTemplateIndex = 0; nTemplateIndex < m_arypWordTemplates.GetSize(); nTemplateIndex++)
	{
		NexFormsWordTemplate *word = m_arypWordTemplates.GetAt(nTemplateIndex);

		if(word->ShouldImport())
		{
			//TES 9/18/2008 - PLID 31413 - EnsureDirectory() moved to FileUtils
			if(!FileUtils::EnsureDirectory(GetTemplatePath(word->strType))) {
				AfxThrowNxException("NexFormsImportInfo::ImportPacketsAndTemplates\r\nFailed to create path:\r\n%s", GetTemplatePath(word->strType));
			}
			// (z.manning, 07/24/2007) - Used to do move file here, but I'd rather just do copy to ensure the temp
			// files stick around until we choose to delete them (which currently happens in the import wizard
			// master dialog's destructor).
			if(!CopyFile(word->strTempFileFullPath, GetTemplatePath(word->strType, word->strFilename), FALSE)) 
			{
				// (z.manning, 07/19/2007) - PLID 26746 - Failed to move the file, let's open it so we
				// can get more details about what's wrong.
				CFile file;
				CFileException exception;
				if(!file.Open(word->strTempFileFullPath, CFile::modeRead | CFile::shareCompat, &exception)) 
				{
					switch(exception.m_lOsError)
					{
						// (z.manning, 07/19/2007) - PLID 26746 - If it's a sharing violation, it probabaly means
						// the have the file open. Let's give 'em a chance to close it.
						case ERROR_SHARING_VIOLATION:
							pFeedback->ShowWindow(SW_HIDE);
							if( IDRETRY == pwndParent->MessageBox(FormatString(
								"There was a sharing violation when trying to import the '%s' Word template.\r\n\r\n"
								"If you have a copy of this template open, close it and click retry. Otherwise, "
								"choose cancel to stop the import.", word->strFilename), NULL, MB_RETRYCANCEL) )
							{
								pFeedback->ShowWindow(SW_RESTORE);
								// (z.manning, 07/19/2007) - PLID 26746 - We're trying again. Decrement the template array index
								// and then run the next (which will be the same) iteration of the template loop.
								nTemplateIndex--;
								continue;
							}
							else {
								return FALSE;
							}
							break;

						default:
							// (z.manning, 07/19/2007) - PLID 26746 - All other errors-- just throw an exception.
							CString strError;
							exception.GetErrorMessage(strError.GetBuffer(4096), 4095);
							strError.ReleaseBuffer();
							AfxThrowNxException("NexFormsImportInfo::ImportPacketsAndTemplates\r\n%s", strError);
							return FALSE;
							break;
					}
				}
				else
				{
					// (z.manning, 07/19/2007) - PLID 26746 - So we were able to open the file even though
					// we failed when moving it. Odd, but let's run with it.
					CFile fileNewTemplate;
					CFileException exception;
					if(!fileNewTemplate.Open(GetTemplatePath(word->strType, word->strFilename), CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone, &exception)) {
						CString strError;
						exception.GetErrorMessage(strError.GetBuffer(4096), 4095);
						strError.ReleaseBuffer();
						AfxThrowNxException("NexFormsImportInfo::ImportPacketsAndTemplates\r\nFailed to create %s:\r\n%s", GetTemplatePath(word->strType, word->strFilename), strError);
						return FALSE;
					}
					// (z.manning, 07/19/2007) - PLID 26746 - Ok, we have a valid file, let's write to it.
					//TES 11/7/2007 - PLID 27979 - VS2008 - CFiles now report their lengths as ULONGLONGs, but even
					// in VS 6.0 they were DWORDs which is longer than a UINT (which is what it was getting cast
					// as in these functions).  So, if it's longer than UINT_MAX (which is 4 GB, so that's unlikely),
					// let's throw an exception, because this code will not be working properly anyway.
					if(file.GetLength() > UINT_MAX) {
						AfxThrowNxException("NexFormsImportInfo::ImportPacketsAndTemplates() - Found file larger than maximum supported size (4 GB).");
					}
					BYTE *pBuffer = new BYTE[(UINT)file.GetLength()];
					file.Read(pBuffer, (UINT)file.GetLength());
					fileNewTemplate.Write(pBuffer, (UINT)file.GetLength());
					fileNewTemplate.Close();
					file.Close();
					delete [] pBuffer;
				}
			}
		}
	}

	// (z.manning, 08/01/2007) - If we've already imported the packets and templates and since the
	// import wizard dialog is still open, we must have had an exception later on. So go ahead
	// with the import but we can just skip the packets. Yes, it's possible they
	// may have changed the settings in between the failed import and this one, but we simply
	// can't safely import this for fear of exceptions or bad data.
	if(!m_bPacketsAndTemplatesHaveBeenSuccessfullyImported)
	{
		// (z.manning, 07/09/2007) - Now packets.
		CString strSql = BeginSqlBatch();
		AddDeclarationToSqlBatch(strSql, "DECLARE @nPacketID INT");
		AddDeclarationToSqlBatch(strSql, "DECLARE @nMergeTemplateID INT");
		for(int nPacketIndex = 0; nPacketIndex < m_arypPackets.GetSize(); nPacketIndex++)
		{
			NexFormsPacket *packet = m_arypPackets.GetAt(nPacketIndex);

			if(packet->bImport)
			{
				AddStatementToSqlBatch(strSql, "SET @nPacketID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM PacketsT)");
				AddStatementToSqlBatch(strSql,
					"INSERT INTO PacketsT (ID, Name, ProcedureRelated) "
					"VALUES (@nPacketID, '%s', 1) ", _Q(packet->strName));

				// (z.manning, 07/09/2007) - Now add all the packet components.
				for(int nComponentIndex = 0; nComponentIndex < packet->m_arypComponents.GetSize(); nComponentIndex++)
				{
					NexFormsPacketComponent *component = packet->m_arypComponents.GetAt(nComponentIndex);

					// (z.manning, 07/11/2007) - Ensure we have a valid record for this template in MergeTemplatesT.
					AddStatementToSqlBatch(strSql,
						"IF EXISTS (SELECT ID FROM MergeTemplatesT WHERE Path = '%s') BEGIN "
						"	SET @nMergeTemplateID = (SELECT ID FROM MergeTemplatesT WHERE Path = '%s') "
						"END ELSE BEGIN "
						"	SET @nMergeTemplateID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM MergeTemplatesT) "
						"	INSERT INTO MergeTemplatesT (ID, Path) VALUES (@nMergeTemplateID, '%s') "
						"END "
						, _Q(component->strPath), _Q(component->strPath), _Q(component->strPath));

					AddStatementToSqlBatch(strSql,
						"INSERT INTO PacketComponentsT (PacketID, MergeTemplateID, ComponentOrder, Scope) "
						"VALUES (@nPacketID, @nMergeTemplateID, %li, %li) "
						, component->nComponentOrder, component->nScope);
				}
			}
		}

		#ifdef _DEBUG
			CMsgBox dlg(NULL);
			dlg.msg = "BEGIN TRAN \r\n" + strSql + "COMMIT TRAN \r\n";
			// (z.manning, 10/17/2007) - I commented out this line because it causes weird focus issues
			// with the please wait screen, however, I left this code here in case someone ever wants
			// to temporarily uncomment this to easily be able to view the SQL statement.
			//dlg.DoModal();
		#endif //_DEBUG

		ExecuteSqlBatch(strSql);
	}

	m_bPacketsAndTemplatesHaveBeenSuccessfullyImported = TRUE;
	return TRUE;
}

void NexFormsImportInfo::CleanUpNexFormsContent()
{	
	for(int i = 0; i < m_arypProcedures.GetSize(); i++)
	{
		if(m_arypProcedures.GetAt(i) != NULL) {
			delete m_arypProcedures.GetAt(i);
		}
	}
	m_arypProcedures.RemoveAll();

	for(i = 0; i < m_arypLadders.GetSize(); i++)
	{
		NexFormsLadder *ladder = m_arypLadders.GetAt(i);
		for(int j = 0; j < ladder->arypSteps.GetSize(); j++)
		{
			NexFormsLadderStep *step = ladder->arypSteps.GetAt(j);
			if(step != NULL) {
				delete step;
			}
		}
		ladder->arypSteps.RemoveAll();
		if(ladder != NULL) {
			delete ladder;
		}
	}
	m_arypLadders.RemoveAll();

	for(i = 0; i < m_arypWordTemplates.GetSize(); i++)
	{
		if(m_arypWordTemplates.GetAt(i) != NULL) {
			delete m_arypWordTemplates.GetAt(i);
		}
	}
	m_arypWordTemplates.RemoveAll();

	for(i = 0; i < m_arypPackets.GetSize(); i++)
	{
		for(int j = 0; j < m_arypPackets.GetAt(i)->m_arypComponents.GetSize(); j++)
		{
			if(m_arypPackets.GetAt(i)->m_arypComponents.GetAt(j) != 0) {
				delete m_arypPackets.GetAt(i)->m_arypComponents.GetAt(j);
			}
		}
		if(m_arypPackets.GetAt(i) != NULL) {
			delete m_arypPackets.GetAt(i);
		}
	}
	m_arypPackets.RemoveAll();
}

// (z.manning, 07/30/2007) - PLID 26869 - Audit the specified event.
void NexFormsImportInfo::NexFormsImportAuditEvent(long nItem, long nRecordID, CString strOldValue, CString strNewValue)
{
	if(m_nAuditTransactionID == -1) {
		m_nAuditTransactionID = BeginAuditTransaction();
	}

	AuditEvent(-1, "", m_nAuditTransactionID, nItem, nRecordID, strOldValue, strNewValue, aepMedium);
}
