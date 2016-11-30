#include "stdafx.h"
#include "NxCoCImportInfo.h"
#include "NxCoCUtils.h"
#include "FileUtils.h"
#include "AuditTrail.h"
#include "ShowProgressFeedbackDlg.h"
#include "NxCocWizardMasterDlg.h"

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



///////////////////////
//Helper functionality
///////////////////////

//Defines for reading data from the file.  This just make it a little easier to read the source code.
#define READ_NXCOC_INT(nNumber)			pFile->Read(&nNumber, sizeof(DWORD))
#define READ_NXCOC_ULONGLONG(ullNumber)	pFile->Read(&ullNumber, sizeof(ULONGLONG))
#define READ_NXCOC_STRING(str) \
{ \
	DWORD dwSize; \
	pFile->Read(&dwSize, sizeof(DWORD)); \
	char* szBuffer = str.GetBuffer(dwSize + 1); \
	pFile->Read(szBuffer, dwSize); \
	szBuffer[dwSize] = '\0'; \
	DecryptNxCoCString(szBuffer); \
	str.ReleaseBuffer(); \
}

//Does the actual work of reading the header file, when given an open CFile pointing to a NxCoC data file.  Will return all
//	the header information in the reference parameters.
bool ReadHeaderInfo(CFile *pFile, long& nContentFileVersion, long& nNxCoCVersion, COleDateTime& dtDateWritten)
{
	//First, test that the file starts with the appropriate marker.  Read manually instead of using the
	//	READ_STRING macro, because it's possible a user gave us some other file, and who knows what the
	//	first few bytes of it are.
	{
		//Obtain the size of the string if we have the right file.
		DWORD dwSize;
		pFile->Read(&dwSize, sizeof(DWORD));

		//First, we'll see if the size given matches the length of our expected header text.  If it does not, then we
		//	know the user picked an invalid file.
		const CString strExpectedHeader(NXCOC_CONTENT_FILE_HEADER_TEXT);
		if((int)dwSize != strExpectedHeader.GetLength()) {
			return false;
		}

		//The sizes match, so go ahead and read out the next dwSize bytes as a string, and decrypt it.
		CString strHeader;
		char* szBuffer = strHeader.GetBuffer(dwSize + 1);
		pFile->Read(szBuffer, dwSize);
		//Don't forget to properly terminate our string
		szBuffer[dwSize] = '\0';
		DecryptNxCoCString(szBuffer);
		strHeader.ReleaseBuffer();

		//Ensure that our strings match.  If not, they've opened some other non-content file.
		if(strHeader != strExpectedHeader) {
			return false;
		}
	}

	//
	//See documentation of the file format in NxCoCUtils.h
	//

	//First int is the file version
	READ_NXCOC_INT(nContentFileVersion);

	//Next int is the NxCoC version
	READ_NXCOC_INT(nNxCoCVersion);

	//Lastly is the date the file was exported
	SYSTEMTIME st;
	pFile->Read(&st, sizeof(SYSTEMTIME));
	dtDateWritten = COleDateTime(st);

	//And that's the end of the header
	return true;
}

//Given a file name/path, inserts _New before the extension
CString RenameNew(CString strIn)
{
	long nPos = strIn.ReverseFind('.');
	if(nPos == -1) {
		//No extension, just tack it on the end
		return strIn + "_New";
	}

	CString strRet = strIn.Left(nPos) + "_New" + strIn.Mid(nPos);
	return strRet;
}

//Given a file name/path, inserts _Old before the extension
CString RenameOld(CString strIn)
{
	long nPos = strIn.ReverseFind('.');
	if(nPos == -1) {
		//No extension, just tack it on the end
		return strIn + "_Old";
	}

	CString strRet = strIn.Left(nPos) + "_Old" + strIn.Mid(nPos);
	return strRet;
}

/////////////////////
//Class Functionality
/////////////////////

//DRT 10/28/2008 - PLID 31789 - Created for NxCoC Importer
CNxCoCImportInfo::CNxCoCImportInfo(void)
{
	m_hevDestroying = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_eitImportType = eitUnknown;
}

CNxCoCImportInfo::~CNxCoCImportInfo(void)
{
}

//Sets the member type.  This is REQUIRED for an import to go through.
void CNxCoCImportInfo::SetType(eImportType eit)
{
	m_eitImportType = eit;
}

//Given the path to a NxCoC content file, reads the header data and returns that data to the caller.
bool CNxCoCImportInfo::ReadVersionInfoFromNxCoCContentFile(CString strFile, long& nContentFileVersion, long& nNxCoCVersion, COleDateTime& dtDateWritten)
{
	//File only needed in read-only, and we don't care about anyone else reading it.
	CFile fDat(strFile, CFile::modeRead|CFile::shareDenyNone);

	//Just call our helper function which does all the work.
	bool bRet = ReadHeaderInfo(&fDat, nContentFileVersion, nNxCoCVersion, dtDateWritten);

	fDat.Close();
	return bRet;
}

//Function to load the entire content file.  Given a progress dialog for the template section, which is
//	by far the lengthiest portion.
bool CNxCoCImportInfo::LoadFromNxCoCContentFile(CString strFile, CShowProgressFeedbackDlg *pProgressDlg)
{
	//Only need access to read, and we do not mind sharing.
	CFile fDat(strFile, CFile::modeRead|CFile::shareDenyNone);

	//First off, get the header information.  We'll load their values into the member variables of the class.
	if(!ReadHeaderInfo(&fDat, m_nContentFileVersion, m_nNxCoCVersion, m_dtDateWritten)) {
		return false;
	}

	//Next up, get the packet data.  This will load the m_aryPackets array with all packets in the data file.
	if(!ReadPackets(&fDat, &m_aryPackets)) {
		return false;
	}

	//Lastly, get the template data.  This will load the m_aryTemplates array with all templates in the data file.  It will
	//	also write any actual template files to the temporary path in m_strTempPath where they can be referenced later.
	//Progress bar:  The templates are most of the work, so let them assign their own progress as we go.  The previous
	//	stuff is trivial and we'll just leave it off.
	if(!ReadTemplates(&fDat, &m_aryTemplates, pProgressDlg)) {
		return false;
	}

	//Success!
	return true;
}

//Given a pointer to an open file that has already been read past the header, reads the contents of that file
//	and creates and fills the given array of packets.
bool CNxCoCImportInfo::ReadPackets(CFile *pFile, CArray<CNxCoCPacket, CNxCoCPacket&> *paryPackets)
{
	//
	//	See documentation of the file format in NxCoCUtils.h
	//

	//Next up in the file is a DWORD of the # of packets to read
	DWORD dwPacketCount = 0;
	READ_NXCOC_INT(dwPacketCount);

	//Loop over each packet to read them out
	for(DWORD i = 0; i < dwPacketCount; i++) {
		CNxCoCPacket pkt;
		//First a DWORD, the ID of the packet
		READ_NXCOC_INT(pkt.nID);

		//Next a string of the packet name
		READ_NXCOC_STRING(pkt.strName);

		//Lastly a 0/1 DWORD that tells whether this is procedure related or not
		DWORD dwProcRel = 0;
		READ_NXCOC_INT(dwProcRel);
		if(dwProcRel == 0)
			pkt.bProcRelated = false;
		else
			pkt.bProcRelated = true;

		//add the packet to our array
		paryPackets->Add(pkt);
	}

	//Success!
	return true;
}

//Given a pointer to an open file that has already been read past the header, reads the contents of that file
///	and creates and fills the given array of templates.  The progress bar will be set to 0 to start, and filled
//	over the size of the array as it is parsed.
bool CNxCoCImportInfo::ReadTemplates(CFile *pFile, CArray<CNxCoCTemplate, CNxCoCTemplate&> *paryTemplates, CShowProgressFeedbackDlg *pProgressDlg)
{
	//As we read the file, we will pull all the content out to a temporary location, and export
	//	the templates there.  The import can then just copy them in when needed.  This helps with
	//	our "duplication" problem, where only the first occurrence of a template will have the file
	//	content, the others just signify to look elsewhere.  We'll put all in a folder named to the 
	//	current second.
	m_strTempPath = FileUtils::GetTempPathWithoutTrailingBackslash() ^ COleDateTime::GetCurrentTime().Format("%Y-%m-%d-%H-%M-%S");
	if(!CreateDirectory(m_strTempPath, NULL)) {
		//Some kind of failure, we cannot continue to export
		AfxMessageBox("Failed to create temp folder '" + m_strTempPath + "'.  Please ensure you have access to the temp directory and try again.");
		return false;
	}

	//Next up in the file is a DWORD of the # of templates to read
	DWORD dwTemplateCount = 0;
	READ_NXCOC_INT(dwTemplateCount);

	//Progress bar:  Begin the progress for templates
	pProgressDlg->SetProgress(0, dwTemplateCount, 0);

	//
	//	See documentation of the file format in NxCoCUtils.h
	//

	//Loop over the number of templates that we should have
	for(DWORD i = 0; i < dwTemplateCount; i++) {
		CNxCoCTemplate tmp;

		//First up is the string path
		READ_NXCOC_STRING(tmp.strRelativePath);

		//Then the packet ID
		READ_NXCOC_INT(tmp.nPacketID);

		//Then the order
		READ_NXCOC_INT(tmp.nComponentOrder);

		//Then the type
		READ_NXCOC_INT(tmp.nType);

		//If the type is 0, this means we also have a file embedded in our content, and need to read that out.  If
		//	the type is 1, this means it's a duplicate template record, and some other element will also have the file already, 
		//	it is NOT packed into the content here.
		if(tmp.nType == 0) {
			//First up is the entire file size, in a 64 bit int.
			ULONGLONG nFileSize = 0;
			READ_NXCOC_ULONGLONG(nFileSize);

			//We now will loop over the file size, reading a chunk at a time
			const long nChunkSize = 4096;	//4k at a time
			ULONGLONG nRemainingToRead = nFileSize;
			//Now create the temp file for writing to disk.  We will use our temp path and the relative path.  Just blindly overwrite
			//	if something already is there.
			CString strNewPath = m_strTempPath ^ tmp.strRelativePath;
			//Ensure that all subfolders exist
			FileUtils::EnsureDirectory( FileUtils::GetFilePath(strNewPath) );
			//Create the temp file for writing
			CFile fTmp(strNewPath, CFile::modeCreate|CFile::modeWrite | CFile::shareCompat);
			while(nRemainingToRead > 0) {
				BYTE buf[nChunkSize];
				UINT nSizeToRead = nChunkSize;
				//If the "remaining to be read" is smaller than our chunk size, just read what's left.  This is
				//	a safe conversion from int64 to int32 because we have already guaranteed that the remaining is
				//	smaller than the chunk size, which is an int32.
				if(nRemainingToRead < (ULONGLONG)nChunkSize)
					nSizeToRead = (DWORD)nRemainingToRead;
				UINT nBytesRead = pFile->Read(buf, nSizeToRead);
				if(nBytesRead == 0) {
					//Failure!  We should not get here if there is anything legit remaining to read, so this must
					//	be some kind of file corruption!
					AfxMessageBox("Failed to read template data for file '" + tmp.strRelativePath + "'.  Please check that your content file is valid and try again.");
					ClearTempPath();
					return false;
				}

				//Decrement by what we read
				nRemainingToRead -= nBytesRead;

				//Output what we just read back out to disk
				fTmp.Write(buf, nBytesRead);
			}
			//Cleanup
			fTmp.Close();
		}
		else {
			//File will be exported elsewhere
		}

		//Add the template to our array
		paryTemplates->Add(tmp);

		//Progress bar:  update the subprogress
		pProgressDlg->SetProgress(0, dwTemplateCount, i);
	}

	//Success!
	return true;
}

// (z.manning 2009-04-09 12:59) - PLID 33934 - Changed the parent to be a CNxCoCWizardMasterDlg
bool CNxCoCImportInfo::DoNxCoCImport(CNxCoCWizardMasterDlg *pwndParent, CShowConnectingFeedbackDlg *pFeedback)
{
	//As we reach this point, we have our m_ImportInfo object, which contains an array of packets and an 
	//	array of templates, and an import option which tells us whether that object should ovewrite, rename, 
	//	or be skipped entirely.  We also have a temp path full of all the documents in the export.


	//Now we have answers to all conflicts, so let's begin an audit transaction
	long nAuditTransactionID = -1;
	try {
		//Begin a transaction		
		// (a.walling 2010-09-08 13:16) - PLID 40377 - Use CSqlTransaction
		CSqlTransaction trans;
		trans.Begin();

		//And one for auditing too
		nAuditTransactionID = BeginAuditTransaction();

		//For auditing
		long nPktOverwrite = 0, nPktRenExt = 0, nPktRenNew = 0, nPktSkip = 0;
		long nTmpOverwrite = 0, nTmpRenExt = 0, nTmpRenNew = 0, nTmpSkip = 0;

		//We are going to insert all our new records above the old, so we'll need a map
		//	to track the Old Packet ID -> New Packet ID.
		//	Old Packet ID:  The ID in the content file.  This was the ID record in the source database, and will be
		//		unique.
		//	New Packet ID:  The new ID that is inserted into the database, or merged into in PacketsT.  This is not the
		//		nNewID tracked through this block of code.
		CMap<long, long, long, long> mapOldPacketIDToNew;


		//
		//	I wrote up a sheet of test cases that help explain the behavior below.  The conditions
		//		for creating records when overwrites and renames happen is fairly complex.
		//
		//	\\yoda\shared\Development\Documentation\PLIDs\31789\
		//


		//Import all the packets into data
		{
			//Not parameterized because there could be a very large number of packets, and I'm not
			//	confident in the speed/ability to take in several hundred parameters to a query.
			//@tmpOverwriteID is used when overwriting to temporarily hold a value for insert
			CString strSqlBatch;
			strSqlBatch.Format(
				"SET NOCOUNT ON;\r\n"
				"DECLARE @tmpOverwriteID int;\r\n"
				"DECLARE @MaxPacketID int;\r\n"
				"SET @MaxPacketID = (SELECT COALESCE(MAX(ID), 0) FROM PacketsT);\r\n"
				"DECLARE @tblPackets TABLE (ContentID int NOT NULL, ID int NOT NULL, Name nvarchar(50), ProcedureRelated bit NOT NULL);\r\n"
				"DECLARE @tblMappedIDs  TABLE (ContentID int NOT NULL, PracticeID int NOT NULL);\r\n"
				);

			//Now loop through all our packets and insert them into our table variable
			long nNewID = 1;
			for(int i = 0; i < m_aryPackets.GetSize(); i++) {
				CNxCoCPacket pkt = m_aryPackets.GetAt(i);

				//handle the method of importing
				if(pkt.eImportOption == ehiooSkip) {
					//Do not import at all.  This covers both new packets that we chose not to import, 
					//	as well as packets that were duplicates of existing, and we chose 'skip'.
					//auditing
					nPktSkip++;
				}
				else {
					bool bSkipPacketCreation = false;
					if(pkt.eImportOption == ehiooImportNew) {
						//We've already detected that this is a new packet, 
						//	it did not exist previously.  We'll let the shared code below
						//	do its thing to generate the new record, nothing else need happen.

						//for auditing purposes, these are the same as overwrite
						nPktOverwrite++;
					}
					else if(pkt.eImportOption == ehiooRenameNew) {
						//Rename our new packet
						//Ensure it still meets our 50 char criteria
						if(pkt.strName.GetLength() + 4 > 50) {
							//just cut off the last 4 characters so we have room
							pkt.strName = pkt.strName.Left(46);
						}
						pkt.strName += "_New";
						//auditing
						nPktRenNew++;
					}
					else if(pkt.eImportOption == ehiooRenameExisting) {
						//Add a query to rename the one already in data, again ensuring we don't break the 50 char cap
						strSqlBatch += FormatString("	UPDATE PacketsT SET Name = LEFT(Name, 46) + '_Old' WHERE Name = '%s'", _Q(pkt.strName));
						//auditing
						nPktRenExt++;
					}
					else if(pkt.eImportOption == ehiooOverwrite) {
						//For overwriting, we are going to just remove all the components from the old packet and put
						//	new ones in place.  It is important that other historical records are kept referenced to
						//	this ID, so we cannot change the ID.  The names are the same, so we don't even need to
						//	do anything about them.
						//Note that overwrite implies "proc rel will not change"
						//	There are no hard database constraints to keep the name unique (though the app does), so we have to
						//	just pick the first one that's not deleted that matches.
						strSqlBatch += FormatString(
							"	SET @tmpOverwriteID = (SELECT TOP 1 ID FROM PacketsT WHERE Deleted = 0 AND Name = '%s');\r\n"
							"	DELETE FROM PacketComponentsT WHERE PacketID = @tmpOverwriteID;\r\n"
							"	INSERT INTO @tblMappedIDs values (%li, @tmpOverwriteID);\r\n", _Q(pkt.strName), pkt.nID);

						//We do not need to create a new packet record here
						bSkipPacketCreation = true;

						//auditing
						nPktOverwrite++;
					}
					else {
						//There should be no way to get an unknown to this point
						AfxThrowNxException("Invalid import option for packet '" + pkt.strName + "'.");
					}

					if(!bSkipPacketCreation) {
						//We fill the procedure related flag based on:
						//	- if they chose to use LW, they all go in LW, no matter what
						//	- if they chose to use the PIC, then we look at the flag that came 
						//		from the source content.
						long nProcRel = 0;
						if(m_eitImportType == eUsePIC) {
							if(pkt.bProcRelated)
								nProcRel = 1;
						}

						//Fill the table variable with the packet information, including a new ID (auto incrementing from 1), the name, and 
						//	it's procedure related status.
						strSqlBatch += FormatString("	INSERT INTO @tblPackets values (%li, %li, '%s', %li);\r\n", pkt.nID, nNewID, _Q(pkt.strName), nProcRel);

						//increment our new ID
						nNewID++;
					}
				}
			}

			//So we've got the max ID currently in the packet table, and a table variable with all the content.  We can
			//	just run an insert statement from there.
			strSqlBatch += FormatString("INSERT INTO PacketsT (ID, Name, ProcedureRelated, PacketCategoryID, Deleted) "
				"SELECT ID + @MaxPacketID, Name, ProcedureRelated, NULL, 0 FROM @tblPackets;\r\n");

			//Fill our map table with the same
			strSqlBatch += FormatString("INSERT INTO @tblMappedIDs SELECT ContentID, ID + @MaxPacketID FROM @tblPackets;\r\n");

			//We will also need to select out all mapped IDs for templates to use
			strSqlBatch += 
				"SET NOCOUNT OFF;\r\n"
				"SELECT ContentID, PracticeID FROM @tblMappedIDs;\r\n";

			//Do the actual insert
			_RecordsetPtr prsResults = CreateRecordsetStd(strSqlBatch);

			//@tblMappedIDs will now have all the maps from content -> practice ID.  Load them up for templates to use.
			while(!prsResults->eof) {
				long nContentID = AdoFldLong(prsResults, "ContentID");
				long nPracticeID = AdoFldLong(prsResults, "PracticeID");

				//map 'em
				mapOldPacketIDToNew.SetAt(nContentID, nPracticeID);
				prsResults->MoveNext();
			}
		}

		//
		//PacketsT is now fully updated, and our map is populated with the ID links.
		//

		//Next up, we need to fill MergeTemplatesT and PacketComponentsT at the same time.  
		//	Same idea, except we do not need to map the IDs.  This section only manipulates
		//	SQL data, it does not touch the actual files yet.
		{
			//@tmpID - This is just a free-use ID for holding temporary values during queries in the insertion.  All uses should
			//	remain contained in a single block of code.  Do not set the ID and leave it for another loop iteration to read.
			//@MaxTemplateID - The maximum template ID in the database before we do anything.  This gives us our starting point for
			//	future additions.
			//@tblMergeTemplates - A table to hold all new records that need created in MergeTemplatesT.
			//		ID - This is the New ID that will be directly inserted into the live table.  Should be
			//			calculated as MaxTemplateID + (incrementing counter)
			//		Path - The relative path of the template.
			//@tblPacketComponents - A table to hold all new records that need created in PacketComponentsT.
			//		PacketID - This is the ID of the Packet this component relates to.  This should be the live
			//			ID in PacketsT after the first section imports, i.e. the 'PracticeID' portion of the map.
			//		MergeTemplateID - The ID put in @tblMergeTemplatesT.ID.  This is the new ID that can be copied
			//			directly to live data.
			//		ComponentOrder - Order of this component of the packet.
			CString strSqlBatch;
			strSqlBatch.Format(
				"DECLARE @tmpID int;\r\n"
				"DECLARE @MaxTemplateID int;\r\n"
				"SET @MaxTemplateID = (SELECT COALESCE(MAX(ID), 0) FROM MergeTemplatesT);\r\n"
				"DECLARE @tblMergeTemplates TABLE (ID int NOT NULL, Path nvarchar(255));\r\n"
				"DECLARE @tblPacketComponents TABLE (PacketID int NOT NULL, MergeTemplateID int NOT NULL, "
				"ComponentOrder int NOT NULL);\r\n");

			//Now loop over all the templates to fill our table variables
			long nNewMergeTemplateID = 1;
			for(int i = 0; i < m_aryTemplates.GetSize(); i++) {
				CNxCoCTemplate tmp = m_aryTemplates.GetAt(i);

				//
				//All operations are separated into 2 groups -- In-Packet (the template is in a packet that we are importing)
				//	and Non-Packet (the template is not part of any packet, it's a freestanding file).
				//
				if(tmp.eImportOption == ehiooSkipTemplateEntirelyNoPacket) {
					//This condition indicates that the packet was skipped entirely, thus we will do nothing about the template.  This
					//	differs from importing a packet, but choosing to skip the template.
					nTmpSkip++;	//Auditing
				}
				else if(tmp.nPacketID == -1) {
					//All non-packet operations.  Since non-packet operations NEVER cause changes to the database, we don't
					//	have any work to do in this section.  I have outlined and documented all the cases so they can be
					//	understood.

					//Consider each type of import
					if(tmp.eImportOption == ehiooSkip) {
						//The easiest element.  It's a standalone file, and we ignored it, so do absolutely nothing.
						nTmpSkip++;	//Auditing
					}
					else {
						if(tmp.eImportOption == ehiooOverwrite) {
							//No work will need done here.  The new file will just take over where the old one
							//	used to be.  All MergeTemplatesT records will point to this new incoming file automatically.
							nTmpOverwrite++;	//auditing
						}
						else if(tmp.eImportOption == ehiooRenameExisting) {
							//No work here either.  The existing file will be renamed, and a new file will take
							//	its place.  If a MergeTemplatesT record happened to exist before, it will now
							//	use the newly imported file, the old one is just left stranded until needed.
							//	No work to be done here.
							nTmpRenExt++;	//auditing
						}
						else if(tmp.eImportOption == ehiooRenameNew) {
							//Another easy element.  We are renaming the new version, and no database records
							//	need created.  This rename will happen in loop 3, no work to do here.
							nTmpRenNew++;	//auditing
						}
						else if(tmp.eImportOption == ehiooImportNew) {
							//Another easy element.  This is a standalone file being imported, and no database
							//	records need created.  We'll let the file copying do its thing, no work here.
							nTmpOverwrite++;	//auditing
						}
						else {
							//should be impossible to be unknown
							AfxThrowNxException("Invalid import option for template '" + tmp.strRelativePath + "'.");
						}
					}
				}
				else {
					//All in-packet operations.  These require changes to the database to support the change in files
					//	depending on our user's choices.

					//This is needed in all cases, so look it up ahead of time
					long nNewPacketID = -1;
					if(!mapOldPacketIDToNew.Lookup(tmp.nPacketID, nNewPacketID)) {
						//catastrophic failure -- we have a template referencing a packet that doesn't exist
						AfxThrowNxException("Failure to import content -- the template '" + tmp.strRelativePath + "' could not be matched to its packet.");
					}

					//Consider each type of import

					//It turns out that most of these are doing the same thing in terms of database changes.  Lookup the MergeTemplatesT
					//	record that corresponds with the template name we are importing.  Insert a new packet component in our new packet.
					//	Link that MergeTemplate to the new packet component.
					bool bDoLinkOldMergeRecord = false;

					if(tmp.eImportOption == ehiooSkip) {
						//We are skipping the import of the file itself.  We still need to join the existing file to
						//	the packet that we imported.  We will not create a new record in MergeTemplatesT, but we
						//	will link that ID to a new PacketComponent record
						bDoLinkOldMergeRecord = true;
						nTmpSkip++;	//auditing
					}
					else {
						if(tmp.eImportOption == ehiooOverwrite) {
							//We are overwriting the existing record.  This will follow the common behavior as defined
							//	in skip.  We want to link the old MergeTemplatesT record to a new PacketComponentsT record.
							bDoLinkOldMergeRecord = true;
							nTmpOverwrite++;	//auditing
						}
						else if(tmp.eImportOption == ehiooRenameExisting) {
							//We are renaming the existing file, and our new import will "take over" the MergeTemplatesT record.
							//	Follow the common behavior as defined in skip.  We want to link the MergeTemplatesT record to the
							//	new component.
							bDoLinkOldMergeRecord = true;
							nTmpRenExt++;	//auditing
						}
						else if(tmp.eImportOption == ehiooRenameNew) {
							//We are renaming the new file, and will leave the old as-is.  However that old MergeTemplatesT record
							//	needs to be linked to the new packet which is being imported, we cannot just leave off the file.
							//Follow the common behavior defined in Skip.
							bDoLinkOldMergeRecord = true;
							nTmpRenNew++;	//auditing
						}
						else if(tmp.eImportOption == ehiooImportNew) {
							//The simplest case.  This is a new template, which is tied to a packet that we
							//	have imported.  We'll want to create new records in both tables.
							strSqlBatch += FormatString(
								"INSERT INTO @tblMergeTemplates values (@MaxTemplateID + %li, '%s');\r\n"
								"INSERT INTO @tblPacketComponents values (%li, @MaxTemplateID + %li, %li);\r\n", 
								nNewMergeTemplateID, _Q(tmp.strRelativePath), nNewPacketID, nNewMergeTemplateID, tmp.nComponentOrder);

							//Increment our new id counter
							nNewMergeTemplateID++;

							nTmpOverwrite++;	//auditing
						}
						else {
							//should be impossible to be unknown
							AfxThrowNxException("Invalid import option for template '" + tmp.strRelativePath + "'.");
						}
					}

					if(bDoLinkOldMergeRecord) {
						//Common behavior from above.
						//Since MergeTemplatesT has no unique constraints, and all paths are the same if they are duplicate, 
						//	we just want the first available.
						//Additional trouble -- there is no guarantee that all existing templates in the forms folder
						//	have a MergeTemplatesT record.  So we need to do an in-place check, and insert one if not.
						strSqlBatch += FormatString(
							"SET @tmpID = (SELECT TOP 1 ID FROM MergeTemplatesT WHERE Path = '%s');\r\n"
							"IF(@tmpID IS NULL) BEGIN \r\n"
							"	SET @tmpID = @MaxTemplateID + %li;\r\n"
							"	INSERT INTO @tblMergeTemplates values (@tmpID, '%s');\r\n"
							"END "
							"INSERT INTO @tblPacketComponents values (%li, @tmpID, %li);\r\n",
							_Q(tmp.strRelativePath),
							nNewMergeTemplateID, _Q(tmp.strRelativePath), 
							nNewPacketID, nNewMergeTemplateID, tmp.nComponentOrder, 
							nNewPacketID, tmp.nComponentOrder);

						//increment our ID in case we used it
						nNewMergeTemplateID++;
					}
				}
			}

			//Copy the records to the live tables
			strSqlBatch += FormatString(
				"INSERT INTO MergeTemplatesT (ID, Path, DefaultScope) (SELECT ID, Path, NULL FROM @tblMergeTemplates);\r\n"
				"INSERT INTO PacketComponentsT (PacketID, MergeTemplateID, ComponentOrder) "
				"(SELECT PacketID, MergeTemplateID, ComponentOrder FROM @tblPacketComponents)");

			//And now we execute the batch
			//	Set maxRecordsAffected to unlimited, this may be a lot of records.
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(-1);
			ExecuteSqlStd(strSqlBatch);
		}

		//The above sections will run all the queries.  We wait to do the template files until last
		//	to ensure that those queries succeed.  We could do this work in the 2nd loop, but then 
		//	it would be difficult/impossible to properly rollback if there is a failure.  Any failures
		//	in this section will have to be dealt with individually.
		{
			//Track all errors that happen.  We cannot "rollback" at any point during the file copy, because
			//	some files may already be in place.  So we will let the user know the details.
			CString strErrorCopies;
			CString strSuccessCopies;

			for(int i = 0; i < m_aryTemplates.GetSize(); i++) {
				CNxCoCTemplate tmp = m_aryTemplates.GetAt(i);

				//The in-packet vs non-packet status is irrelevant here.  The same thing happens either way.

				//If any of our "setup" movements fail, we don't want to try copying the file from the temp
				//	path.
				bool bFailed = false;

				if(tmp.eImportOption == ehiooSkip || tmp.eImportOption == ehiooSkipTemplateEntirelyNoPacket) {
					//Skiparoo, we do not copy the file at all.
				}
				else {
					if(tmp.eImportOption == ehiooRenameNew) {
						//We rename the temp file in the temp directory.  Then we update our template name to 
						//	point to that, and let the copy at the bottom do its magic.
						CString strNewPath = RenameNew(tmp.strRelativePath);
						if(!CopyFile(m_strTempPath ^ tmp.strRelativePath, m_strTempPath ^ strNewPath, FALSE)) {
							strErrorCopies += FormatString(" - (Rename New) Failed to copy file '%s' to '%s'.  GetLastError = %li.\r\n", m_strTempPath ^ tmp.strRelativePath, m_strTempPath ^ strNewPath, GetLastError());
							bFailed = true;
						}

						//And then change our struct
						tmp.strRelativePath = strNewPath;
						m_aryTemplates.SetAt(i, tmp);
					}
					else if(tmp.eImportOption == ehiooRenameExisting) {
						//First, we need to make sure the destination does not exist, possibly from a previous rename.  We'll
						//	just delete in this case.
						//This will rename the existing file in the shared path to _Old.  Then the file copy at the bottom
						//	will move the new version into place.
						if(!DeleteFile(GetSharedPath() ^ RenameOld(tmp.strRelativePath))) {
							//Probably OK, just means that the file likely did not exist.  If it did
							//	and still failed, the move below will give the user a proper warning.
						}
						if(!CopyFile(GetSharedPath() ^ tmp.strRelativePath, GetSharedPath() ^ RenameOld(tmp.strRelativePath), TRUE)) {
							strErrorCopies += FormatString(" - (Rename Existing) Failed to copy file '%s' to '%s'.  GetLastError = %li.\r\n", GetSharedPath() ^ tmp.strRelativePath, GetSharedPath() ^ RenameOld(tmp.strRelativePath), GetLastError());
							bFailed = true;
						}
					}
					else if(tmp.eImportOption == ehiooOverwrite) {
						//Just to be safe on the ovewrite, we'll delete the old file entirely, then the new copy
						//	at the bottom will put the new version in its place.
						if(!DeleteFile(GetSharedPath() ^ tmp.strRelativePath)) {
							//We are not going to flag failure here.  We may still be able to just blanket copy the 
							//	new file over this old one, so let the below section try it.  The error will be logged
							//	there if it still fails.
						}
					}
					else if(tmp.eImportOption = ehiooImportNew) {
						//We don't need to do any work here, the file is being imported newly, and the file copy
						//	at the bottom will do all the work.
					}
					else {
						//should be impossible to be unknown
						AfxThrowNxException("Invalid import option for template '" + tmp.strRelativePath + "'.");
					}

					//And now, copy our temp file to the final location
					if(!bFailed) {
						// (z.manning 2008-12-29 10:21) - PLID 32572 - Make sure the folder we're copying
						// this template to exists.
						const CString strTemplatePath = FileUtils::GetFilePath(GetSharedPath() ^ tmp.strRelativePath);
						FileUtils::EnsureDirectory(strTemplatePath);
						if(!CopyFile(m_strTempPath ^ tmp.strRelativePath, GetSharedPath() ^ tmp.strRelativePath, FALSE)) {
							strErrorCopies += FormatString(" - (File Copy) Failed to copy file '%s' to '%s'.  GetLastError = %li.\r\n", m_strTempPath ^ tmp.strRelativePath, GetSharedPath() ^ tmp.strRelativePath, GetLastError());
							bFailed = true;
						}
						else {
							//We may or may not use this... save a success message.
							strSuccessCopies += FormatString(" - (File Copy) Successfully copied file '%s' to '%s'.", m_strTempPath ^ tmp.strRelativePath, GetSharedPath() ^ tmp.strRelativePath);
						}
					}
				}
			}

			if(!strErrorCopies.IsEmpty()) {
				//Hide the progress
				pFeedback->ShowWindow(SW_HIDE);
				//Some kind of errors, let's warn the user.
				AfxMessageBox("Errors have occurred while copying files during import.  Some files may have been copied successfully, please review the failed cases individually:\r\n"
					+ strErrorCopies);
				//Show progress again
				pFeedback->ShowWindow(SW_SHOW);
			}
		}

		{
			//Lastly, auditing.  This whole batch is only importing packets (do not audit individually)
			//	and word templates (impossible to properly audit any way), so we're just going to throw
			//	in an audit that says "The import was run".
			CString strText = FormatString("New/Overwritten Packets:  %li, Renamed Incoming Packets:  %li, Renamed Existing Packets:  %li, Skipped Packets:  %li.  "
				"New/Ovewrriten Templates:  %li, Renamed Incoming Templates:  %li, Renamed Existing Templates:  %li, "
				"Skipped Templates:  %li", nPktOverwrite, nPktRenNew, nPktRenExt, nPktSkip, nTmpOverwrite, nTmpRenNew, nTmpRenExt, nTmpSkip);

			AuditEventItems aei = aeiNxCoCImport;
			if(pwndParent->m_bEmrStandard) {
				// (z.manning 2009-04-09 14:43) - PLID 33934 - If this is an EMR std import then audit as much.
				aei = aeiEmrStandardImport;
			}
			AuditEvent(-1, "", nAuditTransactionID, aei, -1, "", strText, aepMedium, aetCreated);
		}

		//Commit the data transaction
		// (a.walling 2010-09-08 13:16) - PLID 40377 - Use CSqlTransaction
		trans.Commit();

		//Commit the audit trans
		CommitAuditTransaction(nAuditTransactionID);

		//Success!
		return true;

		//Exceptions during processing will rollback all SQL code, and rollback the audit transaction.  Any file operations
		//	that take place will be left.  This will then re-throw back to the parent caller.
	} NxCatchAllSilentCallThrow(RollbackAuditTransaction(nAuditTransactionID));

	return false;
}

//Cleanup, if the temp path has been set, attempts to delete everything in the folder, then remove the folder itself.
void CNxCoCImportInfo::ClearTempPath()
{
	if(m_strTempPath.IsEmpty()) {
		//Already done
		return;
	}

	//We have to clear the files ourself
	SHFILEOPSTRUCT fos;
	ZeroMemory(&fos, sizeof(fos));  
	fos.hwnd = NULL;
	fos.pTo = NULL;
	fos.wFunc = FO_DELETE;
	//pFrom must be double NULL-terminated, so we'll manually copy the string
	long nSize = m_strTempPath.GetLength();
	char* sz = new char[nSize + 2];
	for(int i = 0; i < nSize; i++) {
		sz[i] = m_strTempPath[i];
	}
	sz[nSize] = '\0';
	sz[nSize+1] = '\0';
	fos.pFrom = sz;

	//Do not give errors to the user, do not give a UI to the user.
	fos.fFlags |= FOF_NOERRORUI | FOF_SILENT | FOF_NOCONFIRMATION; 

	int nResult = SHFileOperation(&fos);
	if(nResult != 0) {
		//Failure.   We may have to just leave the files.  If debugging, inform the user.
#ifdef _DEBUG
		AfxMessageBox( FormatString("Error in SHFileOperation:  %li.", nResult) );
#endif
	}

	//Cleanup our path
	delete [] sz;

	//Then remove the directory we created
	RemoveDirectory(m_strTempPath);
	m_strTempPath = "";
}
