#ifndef MERGE_ENGINE_H
#define MERGE_ENGINE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "NxPracticeSharedLib/MergeEngineBase.h"

// Other useful declarations /////////////////////////////////////////////////////////
#define acExportDelim				2


////////////////////////////////////////////////////////////////////////////////////////

class CProgressMgr;
class CGenericWordProcessorApp;

typedef CString (CALLBACK *LPEXTRAMERGEFIELDSFUNC)(BOOL bFieldNamesInsteadOfData, const CString &strKeyFieldValue, LPVOID pParam);

////////////////
// The Merge Engine is a class used for easily merging to word in a uniform way
// (c.haag 2012-05-07) - PLID 49951 - CMergeEngine now inherits from CMergeEngineBase in the Practice shared library
class CMergeEngine : public CMergeEngineBase
{
public:
	CMergeEngine();	// Sets all members to appropriate default values
	~CMergeEngine();	// Deletes all temporary queries

private:
	// (c.haag 2012-05-07) - PLID 49951 - Returns the location image path
	CString GetLocationImagePath();
	// (c.haag 2012-05-07) - PLID 49951 - Returns the contact image path
	CString GetContactImagePath();
	// (c.haag 2012-05-07) - PLID 49951 - Returns the current location ID
	int GetCurrentLocationID();
	// (c.haag 2012-05-07) - PLID 49951 - Returns non-zero if we're running from internal
	bool IsNexTechInternal();

private:
	// (c.haag 2012-05-07) - PLID 49951 - Returns a new ID from the mail batch table.
	int GetNewMailBatchID();
	// (c.haag 2012-05-07) - PLID 49951 - Returns true if SSN numbers can be visible
	long GetPatientSSNMask();
	// (c.haag 2012-05-07) - PLID 49951 - Returns true if patient NexWeb security codes can be visible
	bool ShowNexWebSecurityCode();
	// (c.haag 2012-05-07) - PLID 49951 - Returns true if patient billing merge data can be visible
	bool ShowBillingMergeData();
	// (c.haag 2012-05-07) - PLID 49951 - Format procedure rich text fields
	void FormatProcedureRtfFields(CString& strProcInfoQuery);

private:
	// (c.haag 2012-05-12) - PLID 49936 - We require a handler for logging
	int Log(LPCTSTR strFormat, ...);
	// (c.haag 2012-05-12) - PLID 49936 - We require a handler for logging
	int LogIndent(LPCTSTR strFormat, ...);
	// (c.haag 2012-05-12) - PLID 49936 - We require a handler for logging
	int LogUnindent(LPCTSTR strFormat, ...);

	// Interface members
public:
	// Parameters that can be set before the call to MergeToDoc
	CString m_strSavedAs;

	// (z.manning, 03/05/2008) - PLID 29131 - Added a function to load sender data so that we don't have
	// to duplicate the same code 50 times anymore.
	BOOL LoadSenderInfo(BOOL bPromptIfPreferenceSet);

	CDWordArray m_arydwEMRIDs;
	CStringArray m_astrTempFiles;

	// (c.haag 2004-11-11 09:39) - PLID 14074 - MailSent now ties into
	// procedure information objects and EMR groups. These variables
	// dictate what ID's to submit to MailSent in the event we merge
	// from the PIC or an EMR.
	//TES 7/26/2005 - Combined into single ID.
	long m_nPicID;

	long m_nEMNID; // (j.jones 2006-06-23 11:19) - PLID 20341 - now we track EMN ID as well

	long m_nCategoryID; // The category the merged document will have in the History tab.  Value -1 means no 
						// category.  This variable is only used if merge is being written to the history.

public:
	//TES 7/8/2011 - PLID 20536 - Added an optional pdtServiceDate parameter
	void SaveInPatientHistory(const CString &strSavedAs, const CString &strTemplateName, long nMergedPacketID = -1, long nPacketCategoryID = -1, bool bSeperateDocs = false, OPTIONAL COleDateTime *pdtServiceDate = NULL);
	// Interface
	//TES 12/21/2006 - PLID 23957 - Added an optional parameter, strExtraProgress.  If this is passed in, it will be shown above
	// the usual progress text ("Calculating document name", etc.), and is intended to say "Template m of n".  While I was at it,
	// I consolidated the 6 different overloads of this function into a single overload with a bunch of optional parameters, 
	// in order to make it easier to avoid ambiguity.
	/*void MergeToWord(const CString &strTemplateName, const CString &strSqlMergeTo, OPTIONAL const CString &strExtraProgress = "");
	void MergeToWord(const CString &strTemplateName, const CString &strSqlMergeTo, const CString &strPatientIDFieldName, OPTIONAL const CString &strExtraProgress = "");
	void MergeToWord(const CString &strTemplateName, const CString &strSqlMergeTo, long nMergedPacketID, OPTIONAL const CString &strExtraProgress = "");
	void MergeToWord(const CString &strTemplateName, const CString &strSqlMergeTo, const long nMergedPacketID, long nPacketCategoryID, bool bSeperateDocs, OPTIONAL const CString &strExtraProgress = "");
	void MergeToWord(const CString &strTemplateName, const CString &strSqlMergeTo, const CString &strPatientIDFieldName, long nMergedPacketID, OPTIONAL const CString &strExtraProgress = "");*/
	//TES 7/8/2011 - PLID 20536 - Added an optional pdtServiceDate parameter
	// (a.wilson 2013-04-18 13:37) - PLID 56142 - added return value to depict if the merge was stopped.
	/// <summary>
	/// Merges a Word document based on content from a SQL query
	/// </summary>
	/// <param name="strTemplateName">The full path to the Word template</param>
	/// <param name="vecTempFilePathNames">The full paths to all temporary files used in the merge</param>
	/// <param name="strSqlMergeTo">The SQL query to use for getting data</param>
	/// <param name="strPatientIDFieldName">The SQL field name of the patient ID, or an empty string if not applicable</param>
	/// <param name="nMergedPacketID">If merging a packet, this must be the packet ID</param>
	/// <param name="nPacketCategoryID">If merging a packet, this may be the packet category ID, or -1 if none</param>
	/// <param name="bSeparateDocs">True if we should calculate the category ID for each template when merging packets</param>
	/// <param name="strExtraProgress">Extra progress text</param>
	/// <param name="pdtServiceDate">The service date if applicable</param>
	/// <returns>TRUE if the merge succeeded; otherwise FALSE</returns>
	bool MergeToWord(const CString &strTemplateName, IN const std::vector<CString>& vecTempFilePathNames, const CString &strSqlMergeTo, const CString &strPatientIDFieldName = "", long nMergedPacketID = -1, long nPacketCategoryID = -1, bool bSeparateDocs = false, const CString &strExtraProgress = "", OPTIONAL COleDateTime *pdtServiceDate = NULL);

	CString CreateMergeInfo();
	static CString CreateBlankMergeInfo(long nFlags, OPTIONAL LPEXTRAMERGEFIELDSFUNC pfnCallbackExtraFields = NULL, OPTIONAL LPVOID pCallbackParam = NULL);
	const CString &GetResultsQ();
	const CString &GetGroupListQ();
	long GetGroupSingularId();
	// (c.haag 2004-05-20 11:36) - Call this function when you want to
	// merge an image to a document. The return value is the data to
	// be inserted into the document in the merge.
	CString GetBitmapMergeName(HBITMAP hBmp);
	CString GetBitmapMergeName(CString strExistingFilePathName);
	static long GetPacketCategory(const long nPacketID);

	// (j.jones 2008-01-10 14:38) - PLID 18709 - similar to CreateBlankMergeInfo(), this function
	// takes in nFlags with the standard BMS_ settings, and returns the sql that, when executed,
	// will return an empty recordset with a column for each available merge field
	CString CreateBlankMergeInfoRecordsetSql(long nFlags, OPTIONAL LPEXTRAMERGEFIELDSFUNC pfnCallbackExtraFields = NULL, OPTIONAL LPVOID pCallbackParam = NULL);
	// (a.walling 2013-05-08 12:13) - PLID 56601 - CreateBlankMergeInfoFields returns a set, so we don't have to execute the sql just to get the field names.
	std::set<CiString> CreateBlankMergeInfoFields(long nFlags, OPTIONAL LPEXTRAMERGEFIELDSFUNC pfnCallbackExtraFields = NULL, OPTIONAL LPVOID pCallbackParam = NULL);
	// (j.jones 2008-01-10 14:40) - PLID 18709 - similar to MergeToWord(), this will take in an aryFieldList of all the fields
	// we need to load data for, information on how to filter the patients for the results, and return an sql statement that,
	// when executed, will return a recordset with all the merge data we need
	// (j.gruber 2013-01-17 15:02) - PLID 54689 - added ResFilter
	CString GenerateMergeRecordsetSql(CStringSortedArrayNoCase &aryFieldList, const CString &strSqlMergeTo, const CString &strPatientIDFieldName = "", const CString &strResFilter = "");	

protected:
	// Non-shared members
	/* Obsoleted by GetAllBalanceInfoSubQ()
	CString m_strPatientBalanceSubQ;
	*/

protected:
	/// <summary>
	/// Goes through the content of every text file in a collection looking for references to absolute file paths,
	/// copies those absolute files to temp files and updates those references to point to the temp files
	/// </summary>
	/// <param name="vecTextFiles">The collection of text files to iterate through</param>
	/// <returns>The collection of all newly added temp filenames</returns>
	std::vector<CString> RemapAbsoluteFileReferencesToTempFiles(const std::vector<CString>& vecTextFiles);

	// Internal implementation functions
	// (z.manning 2016-06-02 15:03) - NX-100790 - Now returns a boolean
	/// <summary>
	/// Merges a Word document based on internal object content
	/// </summary>
	/// <param name="strTemplatePathName">The full path to the Word template</param>
	/// <param name="vecTempFilePathNames">The full paths to all temporary files used in the merge</param>
	/// <param name="strSaveFilePathName">The full path to the output file</param>
	/// <param name="ppgs">The progress indicator manager</param>
	/// <returns>TRUE on success; otherwise FALSE</returns>
	BOOL DoMerge(const CString &strTemplatePathName, IN const std::vector<CString>& vecTempFilePathNames, IN const CString &strSaveFilePathName, IN CProgressMgr *ppgs = NULL);
	// (a.walling 2010-02-16 10:49) - PLID 37390 - Requires an extension
	CString CalcDocumentSaveName(long nPersonID, const CString& strExtension);
	// (a.walling 2010-02-16 10:49) - PLID 37390 - Requires an extension
	CString CalcDocumentSaveName(long nPersonID, const CString& strFirst, const CString& strLast, unsigned long nUserDefinedID, const CString& strExtension);
	
	/* Obsoleted by GetAllBalanceInfoSubQ()
	// For merging the "Patient Balance" information when requested
	CString GetPatientBalanceSubQ();
	CString GetPatientPrepaysSubQ();
	CString GetPatientPaysSubQ();
	CString GetPatientChargesSubQ();
	*/

protected:
	//m.hancock - 12/1/2005 - PLID 18478 - Some RTF merge fields should retain their formatting
	//MH - Added bool bRetainFormatting.
	//  If true, the merged data will retain its own font attributes.
	//  If false, the merged data will use the font attributes applied to the merge field on the template.
	// (S.Dhole 09/17/2012) - PLID 52680  Added dwTickCount
	void ConvertToRTFField(IDispatch* rs, CString& strQuery, const CString& strTable, const CString& strField,
		long nProcInfoID, bool bRetainFormatting, DWORD dwTickCount);

	// (c.haag 2016-01-20) - PLID 68172 - Moved ParseNexTech functions into the CWordDocument object
};



// Checks the data to get a unique document name for the given patient
// Throws CException
CString GetPatientDocumentName(long nPatientId, CString strExt = "doc");
CString GetPatientDocumentName(long nPatientId, const CString& strFirst, const CString& strLast, unsigned long nUserDefinedID, CString strExt = "doc");

// The boolean parameter bCancel will be set to true if 
// the user clicked cancel, false if she chose to proceed
// Returns true to attach, false not to attach
bool ShouldAttachBatchToPatientHistory(IN const CStringArray &arystrTemplateFilePathNames, IN const CString &strGroupListT, OUT bool &bCancel);




// Fast version does our own export
// Pass -1 for nFieldCountLimit to have it unlimited, otherwise pass the field count you want to limit it to
void SaveTableToText(const CString &strTableName, const CString &strOutFilePathName, OPTIONAL LPEXTRAMERGEFIELDSFUNC pfnCallbackExtraFields, OPTIONAL LPVOID pCallbackParam, OPTIONAL long nFieldCountLimit, OPTIONAL OUT BOOL *pbDroppedFieldsPastLimit);

class CStoreTableEngine
{
public:
	CStoreTableEngine();

public:
	// Pass -1 for nFieldCountLimit to have it unlimited, otherwise pass the field count you want to limit it to
	void SaveTable(const CString &strTableName, const CString &strOutputFilePathName, OPTIONAL long nFieldCountLimit, OPTIONAL OUT BOOL *pbDroppedFieldsPastLimit);
	// Pass -1 for nFieldCountLimit to have it unlimited, otherwise pass the field count you want to limit it to
	void SaveHeaders(const CString &strTableName, const CString &strOutputFilePathName, OPTIONAL long nFieldCountLimit, OPTIONAL OUT BOOL *pbDroppedFieldsPastLimit);

public:
	LPEXTRAMERGEFIELDSFUNC m_pfnCallbackExtraFields;
	LPVOID m_pCallbackParam;

protected:
	ADODB::_RecordsetPtr m_pRecordset;
	CFile m_fileOut;
	
protected:
	char m_pstrOutText[65536]; // Allow text max length of 65,536
	// (j.jones 2011-10-20 09:42) - PLID 38339 - changed this to a CArray so it would not have a maximum size limit
	CArray<ADODB::FieldPtr, ADODB::FieldPtr> m_aryFieldPtrs;
	int m_nFieldMax;
	long m_nOutLen;
	_variant_t m_varFieldValue;

protected:
	// Pass -1 for nFieldCountLimit to have it unlimited, otherwise pass the field count you want to limit it to
	void WriteFieldNames(OPTIONAL long nFieldCountLimit, OPTIONAL BOOL *pbTruncatedFieldsToMaxCount);
	void WriteValue(const _variant_t *pvar, short nType);
	void WriteFieldValue(int nFieldIndex);
	void WriteRecords(OPTIONAL long nFieldCountLimit);
	void LoadFieldInfo();
};


#endif