// NexFormsUtils.cpp: implementation of the NexFormsUtils class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <NxPracticeSharedLib/RichEditUtils.h>
#include "NexFormsUtils.h"
#include "PathStringUtils.h"
#include "FileUtils.h"
#include "PhaseTracking.h"
#include "ShowProgressFeedbackDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/********************************************************************************/
/*																				*/
/* For documentation on the NexForms importer/exporter, see:					*/
/*																				*/
/*		http://192.168.1.2/developers/doku.php?id=nexforms_importer_exporter	*/
/*																				*/
/********************************************************************************/


// (f.dinatale 2010-09-02) - PLID 40362 - Replacing the READ_NEXFORMS_STRING with a function call.
// Replaced the macro with this function because dwSize was not initialized and there was no validation
// of what was being read from the file.
void ReadNexFormsString(CString & str, UINT & nBytesRead, CFile *pFile)
{
	DWORD dwSize = 0;
	UINT nRead = 0;
	nRead = pFile->Read(&dwSize, sizeof(DWORD));

	// Check to see if the number of bytes read in is 0.  If it is we know we've finished reading so just return.
	if(nRead != 0){
		// If we read in something that's between 0 and 4 bytes, then we know that something is wrong witht he file.
		if(nRead != sizeof(DWORD)) {
			ThrowNxException("ReadNexFormsString : Failed to read the size of NexForms field " + str); 
		} else {
			// We've successfully figured out how many bytes to read in.
			nBytesRead += nRead;

			// If there aren't any bytes to be read in, just return an empty string.
			if(dwSize == 0) {
				str.Empty();
				return;
			} else {
				// Allocate the buffer to the proper size just like before since we need to.
				char* szBuffer = str.GetBuffer(dwSize + 1);
				nRead = pFile->Read(szBuffer, dwSize);

				// If we read in any other amount of bytes than what we expect, something is wrong.
				if(nRead != dwSize) {
					ThrowNxException("ReadNexFormsString : Failed to read NexForms field " + str); 
				} else {
					// Allocate just like before.
					nBytesRead += nRead;
					szBuffer[dwSize] = '\0';
					DecryptNexFormsString(szBuffer);
					str.ReleaseBuffer();
				}
			}
		}
	}
}


// (z.manning, 08/30/2007) - PLID 18359 - Created a new importer for NexForms.

void ReadNexFormsProcedureFromFile(CFile *pFile, NexFormsProcedure *procedure)
{
	DWORD dwTemp;
	UINT nBytesRead = 0;

	// (z.manning, 06/26/2007) - 4 bytes for procedure ID
	pFile->Read(&dwTemp, sizeof(DWORD));
	// (z.manning, 06/26/2007) - Store the ID as a negative value. This is how we differentiate
	// IDs from the export with already existing IDs.
	procedure->nID = dwTemp * -1;

	// (z.manning, 06/19/2007) - N bytes for the procedure name
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strName, nBytesRead, pFile);

	// (z.manning, 06/19/2007) - 4 bytes for master procedure ID
	pFile->Read(&dwTemp, sizeof(DWORD));
	// (z.manning, 06/26/2007) - Store the ID as a negative value. This is how we differentiate
	// IDs from the export with already existing IDs.
	procedure->nMasterProcedureID = dwTemp * -1;

	// (z.manning, 06/19/2007) - N bytes for the official name
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strOfficialName, nBytesRead, pFile);

	// (z.manning, 06/19/2007) - 4 bytes for recur
	pFile->Read(&dwTemp, sizeof(DWORD));
	procedure->bRecur = dwTemp == 0 ? FALSE : TRUE;

	// (z.manning, 06/19/2007) - 4 bytes for EMR preferred procedure
	pFile->Read(&dwTemp, sizeof(DWORD));
	procedure->bEmrPreferredProcedure = dwTemp == 0 ? FALSE : TRUE;

	// (z.manning, 06/19/2007) - 4 bytes for arrival prep minutes
	pFile->Read(&procedure->nArrivalPrepMinutes, sizeof(DWORD));

	// (z.manning, 06/19/2007) - N bytes for custom 1
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strCustom1, nBytesRead, pFile);

	// (z.manning, 06/19/2007) - N bytes for custom 2
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strCustom2, nBytesRead, pFile);

	// (z.manning, 06/19/2007) - N bytes for custom 3
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strCustom3, nBytesRead, pFile);

	// (z.manning, 06/19/2007) - N bytes for custom 4
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strCustom4, nBytesRead, pFile);

	// (z.manning, 06/19/2007) - N bytes for custom 5
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strCustom5, nBytesRead, pFile);

	// (z.manning, 06/19/2007) - N bytes for custom 6
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strCustom6, nBytesRead, pFile);

	// (z.manning, 06/19/2007) - N bytes for anesthesia
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strAnesthesia, nBytesRead, pFile);

	// (z.manning, 06/19/2007) - N bytes for NexForms custom section 1
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strCustomSection1, nBytesRead, pFile);
	procedure->bNeedReviewCustomSection1 = procedure->strCustomSection1.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for NexForms custom section 2
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strCustomSection2, nBytesRead, pFile);
	procedure->bNeedReviewCustomSection2 = procedure->strCustomSection2.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for NexForms custom section 3
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strCustomSection3, nBytesRead, pFile);
	procedure->bNeedReviewCustomSection3 = procedure->strCustomSection3.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for NexForms custom section 4
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strCustomSection4, nBytesRead, pFile);
	procedure->bNeedReviewCustomSection4 = procedure->strCustomSection4.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for NexForms custom section 5
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strCustomSection5, nBytesRead, pFile);
	procedure->bNeedReviewCustomSection5 = procedure->strCustomSection5.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for NexForms custom section 6
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strCustomSection6, nBytesRead, pFile);
	procedure->bNeedReviewCustomSection6 = procedure->strCustomSection6.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for NexForms custom section 7
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strCustomSection7, nBytesRead, pFile);
	procedure->bNeedReviewCustomSection7 = procedure->strCustomSection7.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for NexForms custom section 8
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strCustomSection8, nBytesRead, pFile);
	procedure->bNeedReviewCustomSection8 = procedure->strCustomSection8.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for NexForms custom section 9
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strCustomSection9, nBytesRead, pFile);
	procedure->bNeedReviewCustomSection9 = procedure->strCustomSection9.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for NexForms custom section 10
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strCustomSection10, nBytesRead, pFile);
	procedure->bNeedReviewCustomSection10 = procedure->strCustomSection10.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for the mini description
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strMiniDescription, nBytesRead, pFile);
	procedure->bNeedReviewMiniDescription = procedure->strMiniDescription.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for pre op info
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strPreOp, nBytesRead, pFile);
	procedure->bNeedReviewPreOp = procedure->strPreOp.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for the day of info
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strTheDayOf, nBytesRead, pFile);
	procedure->bNeedReviewTheDayOf = procedure->strTheDayOf.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for post op info
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strPostOp, nBytesRead, pFile);
	procedure->bNeedReviewPostOp = procedure->strPostOp.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for recovery
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strRecovery, nBytesRead, pFile);
	procedure->bNeedReviewRecovery = procedure->strRecovery.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for procedure details
	// (z.manning, 09/04/2007) - PLID 27286 - Renamed field in data to ProcDetails.
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strProcDetails, nBytesRead, pFile);
	procedure->bNeedReviewProcDetails = procedure->strProcDetails.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for risks
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strRisks, nBytesRead, pFile);
	procedure->bNeedReviewRisks = procedure->strRisks.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for alternatives
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strAlternatives, nBytesRead, pFile);
	procedure->bNeedReviewAlternatives = procedure->strAlternatives.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for complications
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strComplications, nBytesRead, pFile);
	procedure->bNeedReviewComplications = procedure->strComplications.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for special diet info
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strSpecialDiet, nBytesRead, pFile);
	procedure->bNeedReviewSpecialDiet = procedure->strSpecialDiet.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for showering info
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strShowering, nBytesRead, pFile);
	procedure->bNeedReviewShowering = procedure->strShowering.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for bandage info
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strBandages, nBytesRead, pFile);
	procedure->bNeedReviewBandages = procedure->strBandages.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for consent
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strConsent, nBytesRead, pFile);
	procedure->bNeedReviewConsent = procedure->strConsent.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for hospital stay info
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strHospitalStay, nBytesRead, pFile);
	procedure->bNeedReviewHospitalStay = procedure->strHospitalStay.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 06/19/2007) - N bytes for the alternative consent
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(procedure->strAltConsent, nBytesRead, pFile);
	procedure->bNeedReviewAltConsent = procedure->strAltConsent.Find(NEXFORMS_NEEDS_REVIEWED_TEXT) != -1;

	// (z.manning, 07/10/2007) - Procedures may have more than one ladder. First read the 
	// count of ladders, then read that manny ladder IDs.
	long nLadderIDCount;
	pFile->Read(&nLadderIDCount, sizeof(DWORD));
	for(int i = 0; i < nLadderIDCount; i++)
	{
		// (z.manning, 06/26/2007) - Store the ID as a negative value. This is how we differentiate
		// IDs from the export with already existing IDs.
		pFile->Read(&dwTemp, sizeof(DWORD));
		procedure->arynLadderTemplateIDs.Add(dwTemp * -1);
		ASSERT(procedure->nMasterProcedureID == 0);
	}
}

// (z.manning, 10/22/2007) - PLID 26846 - Function to compare ladder step orders to use with qsort.
int CompareLadderStepsByOrder(const void *pDataA, const void *pDataB)
{
	NexFormsLadderStep **ppStepA = (NexFormsLadderStep**)pDataA;
	NexFormsLadderStep **ppStepB = (NexFormsLadderStep**)pDataB;

	if(*ppStepA != NULL && *ppStepB != NULL)
	{
		long nOrderA = (*ppStepA)->nStepOrder;
		long nOrderB = (*ppStepB)->nStepOrder;
		
		if(nOrderA < nOrderB) {
			return -1;
		}
		else if(nOrderA == nOrderB) {
			return 0;
		}
		else {
			ASSERT(nOrderA > nOrderB);
			return 1;
		}
	}
	else 
	{
		// (z.manning, 06/25/2007) - Uhh, we shouldn't have a null pointer.
		ASSERT(FALSE);
		return -1;
	}
}

void ReadNexFormsLadderFromFile(CFile *pFile, NexFormsLadder *ladder)
{
	DWORD dwTemp;
	UINT nBytesRead = 0;

	// (z.manning, 06/26/2007) - 4 bytes for Ladder ID
	pFile->Read(&dwTemp, sizeof(DWORD));
	// (z.manning, 06/26/2007) - Store the ID as a negative value. This is how we differentiate
	// IDs from the export with already existing IDs.
	ladder->nID = dwTemp * -1;

	// (z.manning, 06/19/2007) - N bytes for ladder name
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(ladder->strName, nBytesRead, pFile);

	// (z.manning, 06/19/2007) - 4 bytes for auto create
	pFile->Read(&dwTemp, sizeof(DWORD));
	ladder->bAutoCreate = dwTemp == 0 ? FALSE : TRUE;

	// (z.manning, 06/19/2007) - 4 bytes for number of steps
	long nStepCount;
	pFile->Read(&nStepCount, sizeof(DWORD));

	for(int nStepIndex = 0; nStepIndex < nStepCount; nStepIndex++)
	{
		NexFormsLadderStep *step = new NexFormsLadderStep;

		// (z.manning, 06/19/2007) - 4 bytes for order
		pFile->Read(&step->nStepOrder, sizeof(DWORD));

		// (z.manning, 06/19/2007) - N bytes for step name
		// (f.dinatale 2010-09-03) - PLID 40362
		ReadNexFormsString(step->strStepName, nBytesRead, pFile);

		// (z.manning, 06/19/2007) - 4 bytes for action
		pFile->Read(&step->nAction, sizeof(DWORD));

		// (z.manning, 06/19/2007) - N bytes for note
		// (f.dinatale 2010-09-03) - PLID 40362
		ReadNexFormsString(step->strNote, nBytesRead, pFile);

		// (z.manning, 06/19/2007) - 4 bytes activation type
		pFile->Read(&step->nActivateType, sizeof(DWORD));

		// (z.manning, 06/19/2007) - 4 bytes for activation interval
		pFile->Read(&step->nActivateInterval, sizeof(DWORD));

		// (z.manning, 06/19/2007) - N bytes for activation text
		// (f.dinatale 2010-09-03) - PLID 40362
		ReadNexFormsString(step->strActivateStringData, nBytesRead, pFile);

		// (z.manning, 06/19/2007) - 4 bytes for skippable
		pFile->Read(&dwTemp, sizeof(DWORD));
		step->bSkippable = dwTemp == 0 ? FALSE : TRUE;

		// (z.manning, 06/19/2007) - 4 bytes for the number of step criteria
		long nCriteriaCount;
		pFile->Read(&nCriteriaCount, sizeof(DWORD));
		
		for(int nCriteriaIndex = 0; nCriteriaIndex < nCriteriaCount; nCriteriaIndex++)
		{
			long nActionID;
			CString str;
			switch(step->nAction)
			{
				// (z.manning, 07/23/2007) - Packet and template action types just store a string with the
				// name of the packet or template.
				case PhaseTracking::PA_WritePacket:
				case PhaseTracking::PA_WriteTemplate:
					// (f.dinatale 2010-09-03) - PLID 40362
					ReadNexFormsString(str, nBytesRead, pFile);
					step->aryvarActions.Add(_variant_t(str));
					break;

				// (z.manning, 07/23/2007) - We do not yet support action IDs for the create EMR action.
				// (Note: there are other actions types we don't support, but they aren't part of the official
				// content, and if they ever are, the exporter will warn about them.)
				// (j.jones 2010-04-30 10:01) - PLID 38353 - renamed PA_CreateEMR to PA_CreateEMRByCollection (existing behavior),
				// and added PA_CreateEMRByTemplate, which lets you match by template instead of collection
				case PhaseTracking::PA_CreateEMRByCollection:
				case PhaseTracking::PA_CreateEMRByTemplate:
					pFile->Read(&nActionID, sizeof(DWORD));
					break;

				default:
					// (z.manning, 06/19/2007) - 4 bytes for each action ID
					pFile->Read(&nActionID, sizeof(DWORD));
					step->aryvarActions.Add(_variant_t(nActionID));
					break;
			}
		}

		// (z.manning, 07/27/2007) - PLID 26846 - Only add the action if it's something the user is licensed for.
		if(PhaseTracking::IsTrackingActionLicensed(step->nAction)) {
			ladder->arypSteps.Add(step);
		}
	}

	// (z.manning, 07/27/2007) - PLID 26846 - We're done with this ladder, but let's make sure there
	// aren't any gaps in the step order.
	// (z.manning, 10/22/2007) - Make sure the steps are actually in order first!!
	qsort(ladder->arypSteps.GetData(), ladder->arypSteps.GetSize(), sizeof(NexFormsLadderStep*), CompareLadderStepsByOrder);
	for(int nIndex = 0; nIndex < ladder->arypSteps.GetSize(); nIndex++) {
		ladder->arypSteps.GetAt(nIndex)->nStepOrder = nIndex + 1;
	}
}

void ReadNexFormsPacketFromFile(CFile *pFile, NexFormsPacket *packet)
{
	UINT nBytesRead = 0;

	// (z.manning, 07/09/2007) - N bytes for packet name
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(packet->strName, nBytesRead, pFile);

	// (z.manning, 07/09/2007) - 4 bytes for component count
	long nComponentCount;
	pFile->Read(&nComponentCount, sizeof(DWORD));

	for(int nIndex = 0; nIndex < nComponentCount; nIndex++)
	{
		NexFormsPacketComponent *component = new NexFormsPacketComponent;

		// (z.manning, 07/09/2007) - 4 bytes for order
		pFile->Read(&component->nComponentOrder, sizeof(DWORD));

		// (z.manning, 07/09/2007) - 4 bytes for scope
		pFile->Read(&component->nScope, sizeof(DWORD));

		// (z.manning, 07/09/2007) - N bytes for path
		// (f.dinatale 2010-09-03) - PLID 40362
		ReadNexFormsString(component->strPath, nBytesRead, pFile);
		component->SetTemplateNameFromPath();

		packet->m_arypComponents.Add(component);
	}
}

UINT ReadNexFormsWordTemplateFromFile(CFile *pFile, NexFormsWordTemplate *word)
{
	UINT nBytesRead = 0;

	// (z.manning, 07/05/2007) - N bytes for the relative path.
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(word->strRelativePath, nBytesRead, pFile);
	// (z.manning, 07/05/2007) - If we didn't read anything, we're already at the end of the file.
	if(nBytesRead < sizeof(DWORD)) {
		return nBytesRead;
	}

	// (z.manning, 07/05/2007) - N bytes for the filename.
	// (f.dinatale 2010-09-03) - PLID 40362
	ReadNexFormsString(word->strFilename, nBytesRead, pFile);

	// (z.manning, 07/05/2007) - N bytes for the actual Word template.
	// (z.manning, 07/05/2007) - We need to create the file now because we can't have Practice
	// using the amount of memory it would take to keep track of all the byte arrays.
	// (a.walling 2010-05-25 12:24) - PLID 38553 - NexTechSDK static libraries
	CString strTempFile = FileUtils::GetTempPath() ^ word->strFilename + ".tmp";
	CFile file;
	if(!file.Open(strTempFile, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone))
	{
		// (z.manning, 07/19/2007) - PLID 26746 - The file failed to open. Maybe they already have
		// a previous temp version with the same filename/path open. Let's try up to 5 times with
		// changing the filename.
		int nCount = 1;
		while(FileUtils::DoesFileOrDirExist(strTempFile) && nCount <= 5)
		{
			if(nCount > 1) {
				strTempFile.Delete(strTempFile.GetLength() - 1);
			}
			strTempFile += AsString((long)nCount++);
		}

		CFileException exception;
		// (z.manning, 07/19/2007) - PLID 26746 - Hopefully by this point we definitely have a unique temp file name.
		// Let's try again to open the file and if we fail this time, just throw an exception.
		if(!file.Open(strTempFile, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone)) {
			CString strError;
			exception.GetErrorMessage(strError.GetBuffer(4096), 4095);
			strError.ReleaseBuffer();
			AfxThrowNxException("%s", strError);
		}
	}
	DWORD dwSize;
	nBytesRead += pFile->Read(&dwSize, sizeof(DWORD));
	BYTE *pbyte = new BYTE[dwSize];
	nBytesRead += pFile->Read(pbyte, dwSize);
	file.Write(pbyte, dwSize);
	file.Close();
	delete [] pbyte;
	word->strTempFileFullPath = strTempFile;

	return nBytesRead;
}

// (z.manning, 08/02/2007) - I tried to come up with a way to set this up in official content, but
// Meikin and I couldn't agree on anything, so, for now at least, we hard code the procedures that
// are derm only (note: it's assumed that plastics will use them all).
BOOL IsDermProcedure(CString strProcedure)
{
	CStringArray arystrDermProcedures;
	arystrDermProcedures.Add("Botox ®");
	arystrDermProcedures.Add("Chemical Peel/Skin Treatments");
	arystrDermProcedures.Add("Laser Hair Removal");
	arystrDermProcedures.Add("Laser Skin Resurfacing");
	arystrDermProcedures.Add("Restylane ®");
	arystrDermProcedures.Add("Scar Revision");
	arystrDermProcedures.Add("Skin Cancer");
	arystrDermProcedures.Add("Skin Graft");
	arystrDermProcedures.Add("Skin Lesion/Tumor");

	for(int i = 0; i < arystrDermProcedures.GetSize(); i++) 
	{
		if(strProcedure.CompareNoCase(arystrDermProcedures.GetAt(i)) == 0) {
			return TRUE;
		}
	}

	return FALSE;
}

// (d.moore 2007-06-29 12:25) - PLID 23863 - Adds in text at the begining of the section for 'Needs Reviewed'.
void InsertNeedsReviewedText(CRichEditCtrl *pRichEditCtrl)
{
	// Insert the text.
	CString strText = CString(NEXFORMS_NEEDS_REVIEWED_TEXT) + "\n";

	// (z.manning, 08/03/2007) - Don't add the text if it's already there.
	FINDTEXTEX ft;
	ft.chrg.cpMin = 0; // Start searching at the begining of the text.
	ft.chrg.cpMax = -1; // Search till the end of the text.
	ft.lpstrText = strText;
	long nStart = pRichEditCtrl->FindText(FR_WHOLEWORD, &ft);
	long nEnd = ft.chrgText.cpMax;
	if(nStart == -1)
	{
		pRichEditCtrl->SetSel(0, 0);
		pRichEditCtrl->ReplaceSel(strText, FALSE);
		// Select all of the new text so that we can set styles on it.
		pRichEditCtrl->SetSel(0, strText.GetLength());

		// Set Bold:
		CHARFORMAT2 cf;
		cf.dwMask = CFM_BOLD|CFM_FACE;
		cf.dwEffects = CFE_BOLD;
		strcpy(cf.szFaceName, "Tahoma");
		pRichEditCtrl->SetSelectionCharFormat(cf);

		// Set Not Underlined:
		cf.dwMask = CFM_UNDERLINE|CFM_UNDERLINETYPE;
		cf.dwEffects = 0;
		cf.bUnderlineType = CFU_UNDERLINENONE;
		pRichEditCtrl->SetSelectionCharFormat(cf);

		pRichEditCtrl->SetSel(0, 0);
	}
}

// (d.moore 2007-06-29 11:40) - PLID 23863 - Strip out text that is normally displayed when a section is marked 'Needs Reviewed'
void RemoveNeedsReviewedText(CRichEditCtrl *pRichEditCtrl)
{
	LPCTSTR strText = NEXFORMS_NEEDS_REVIEWED_TEXT;

	FINDTEXTEX ft;
	ft.chrg.cpMin = 0; // Start searching at the begining of the text.
	ft.chrg.cpMax = -1; // Search till the end of the text.
	ft.lpstrText = (LPSTR) strText;
	long nStart = pRichEditCtrl->FindText(FR_WHOLEWORD, &ft);
	long nEnd = ft.chrgText.cpMax;

	if (nStart != -1) {
		// Remove the label.
		pRichEditCtrl->SetSel(nStart, nEnd);
		pRichEditCtrl->Clear();
		nEnd = nStart;
		
		// Check for any whitespace left over from in front of the label.
		CString strLeadingText;
		if (nStart > 0) {
			pRichEditCtrl->SetSel(0, nStart);
			strLeadingText = pRichEditCtrl->GetSelText();
			strLeadingText.TrimRight();
			if (strLeadingText.GetLength() == 0) {
				nStart = 0;
			}
		}

		// There will usually be a few newline characters trailing after the label. 
		// This is a quick way to get rid of the first few.
		pRichEditCtrl->SetSel(nStart, nEnd + 2);
		strLeadingText = pRichEditCtrl->GetSelText();
		nEnd = nStart + (strLeadingText.GetLength());
		strLeadingText.TrimLeft();
		nEnd -= strLeadingText.GetLength();
		pRichEditCtrl->SetSel(nStart, nEnd);
		pRichEditCtrl->Clear();
		pRichEditCtrl->SetSel(nStart, nStart);
	}
}

BOOL IsSameProcedureName(CString strProcedure1, CString strProcedure2)
{
	// (z.manning, 08/30/2007) - Delete any occurrances of the registered symbol before comparing.
	strProcedure1.Remove('®');
	strProcedure2.Remove('®');

	strProcedure1.TrimLeft();
	strProcedure1.TrimRight();
	strProcedure2.TrimLeft();
	strProcedure2.TrimRight();

	if(strProcedure1.CompareNoCase(strProcedure2) == 0) {
		return TRUE;
	}

	return FALSE;
}

//TES 7/25/2007 - PLID 26685 - This define and function were copied, with some modifications, from 
// CProcedureSectionEditDlg::ValidateAndSave().  This function simply updates the ProcedureT field identified
// by strField and nProcedureID, setting it to the value passed in as strRichText.
#define TEXT_INCREMENT	8000
void UpdateProcedureField(const CString &strField, const CString &strRichText, long nProcedureID)
{
	if(strRichText.GetLength() > TEXT_INCREMENT) {
		CStringArray arTextSections;
		//DRT 10/9/2008 - PLID 31647 - VS2008
		int j = 0;
		for(j = 0; j+TEXT_INCREMENT < strRichText.GetLength(); j += TEXT_INCREMENT) {
			arTextSections.Add(strRichText.Mid(j,TEXT_INCREMENT));
		}
		arTextSections.Add(strRichText.Mid(j));
		ExecuteSql("UPDATE ProcedureT SET %s = '%s' WHERE ID = %li", strField,
			_Q(arTextSections[0]), nProcedureID);
		for(j = 1; j < arTextSections.GetSize(); j++) {
			CString str = arTextSections[j];
			ExecuteSql("DECLARE @ptrval binary(16) "							
				"SELECT @ptrval = TEXTPTR(%s) "
				"FROM ProcedureT "
				"WHERE ProcedureT.ID = %li "
				"UPDATETEXT ProcedureT.%s @ptrval NULL 0 '%s' ",
				strField, nProcedureID, strField, _Q(arTextSections[j]));
		}
	}
	else {
		ExecuteSql("UPDATE ProcedureT SET %s = '%s' WHERE ID = %li", strField, _Q(strRichText), nProcedureID);
	}
}

void UpdateFontForExistingProcedures(CArray<long,long> &arynProcedureIDs)
{
	if(arynProcedureIDs.GetSize() == 0) {
		return;
	}

	//TES 7/25/2007 - PLID 26685 - Our job now is to go through all the fields that are affected by
	// the NexForms importer, and make sure that anywhere they previously used either Arial or Times
	// New Roman, they now use Tahoma.  We don't replace all typefaces, because in a few cases 
	// (bullet points being the main one), they need to use a special font, such as Symbol, for an
	// isolated character or two.

	//Set up our array of fonts to be replaced.
	CStringArray sa;
	sa.Add("Arial");
	sa.Add("Times New Roman");

	//Pull all NexForms content fields, for any procedures that haven't been imported from NexForms
	// Version 2+ data.
	// (z.manning, 09/04/2007) - PLID 27286 - Renamed field in data to ProcDetails.
	// (z.manning, 10/11/2007) - PLID 27719 - Only load specified procedure IDs and don't bother loading
	// any procedures that have already had this done. Note: This can't be parameterized (not that it does
	// any good anyway) because there may be too many IDs to pass in as a parameter.
	ADODB::_RecordsetPtr rsContent = CreateRecordset(
		"SELECT ID, Name, MiniDescription, PreOp, TheDayOf, PostOp, Recovery, "
		"	ProcDetails, Risks, Alternatives, Complications, SpecialDiet, Showering, Bandages, Consent, "
		"	AltConsent, HospitalStay "
		"FROM ProcedureT "
		"WHERE ProcedureT.ID IN (%s) AND COALESCE(NexFormsFontVersion, 0) < %i"
		, ArrayAsString(arynProcedureIDs,false), NEXFORMS_FONT_VERSION);
	
	//For our progress bar, find out how many procedures we're updating.
	rsContent->MoveLast();
	rsContent->MoveFirst();
	long nProcedureCount = rsContent->GetRecordCount();

	long nProcedure = 0;
	CShowProgressFeedbackDlg dlg(0);
	while(!rsContent->eof) {
		long nID = AdoFldLong(rsContent, "ID");
		CString strName = AdoFldString(rsContent, "Name");
		//Now go through all the fields in this record except the first two (ID and Name), and
		// replace their typefaces.
		for(long i = 2; i < rsContent->Fields->GetCount(); i++) {
			//Pull information from this field.
			ADODB::FieldPtr f = rsContent->Fields->GetItem(i);
			CString strText = AdoFldString(f);
			CString strFieldName = CString((LPCTSTR)f->Name);
			
			//Update our progress bar.
			CString strProgress;
			strProgress.Format("Updating field %li of %li (%s.%s)", 
				nProcedure*15+(i-2), nProcedureCount*15, strName, strFieldName);
			dlg.SetCaption(strProgress);
			dlg.SetProgress(0, nProcedureCount*15, nProcedure*15+(i-2));
			
			//Calculate the next text.
			CString strNewText;
			ReplaceTypefaces(strText, "Tahoma", sa, strNewText);
			if(strText != strNewText) {
				//The text changed, update the database.
				UpdateProcedureField(strFieldName, strNewText, nID);
			}
		}
		nProcedure++;
		rsContent->MoveNext();
	}

	// (z.manning, 10/11/2007) - PLID 27719 - Make sure we update the font version field for all procedures we just updated.
	ExecuteSql("UPDATE ProcedureT SET NexFormsFontVersion = %i WHERE ProcedureT.ID IN (%s)"
		, NEXFORMS_FONT_VERSION, ArrayAsString(arynProcedureIDs,false));
}

// (z.manning 2009-10-22 11:57) - PLID 36033 - Function to determine if a Word template MUST
// be imported.
BOOL MustImportWordTemplate(const CString &strTemplateName)
{
	// (z.manning 2009-10-22 14:05) - PLID 36033 - According to m.clark, ALWAYS import the 
	// medicare waiver template.
	if(strTemplateName.Left(15).CompareNoCase("Medicare Waiver") == 0) {
		return TRUE;
	}

	return FALSE;
}