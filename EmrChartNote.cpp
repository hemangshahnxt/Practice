//EmrChartNote.cpp

#include "stdafx.h"
#include "EmrChartNote.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "EmrCategoriesDlg.h"
#include "msword8.h"
#include "globaldrawingutils.h"

#include "EmrItemAdvImageDlg.h"
#include "IconUtils.h"
#include "fileutils.h"
using namespace ADODB;

CEmrChartNote::CEmrChartNote()
{
	m_nEmrID = -1;
}

CEmrChartNote::CEmrChartNote(long nEmrID)
{
	m_nEmrID = nEmrID;
}

CEmrChartNote::~CEmrChartNote()
{
	RemoveTempMergeFiles();
}

// (a.walling 2008-09-18 17:26) - PLID 28040 - This is in fileutils
/*
void GetFileModifiedTime(CFile &f, OUT CTime &tmModified)
{
	FILETIME lastWriteTime;
	if (GetFileTime(reinterpret_cast<HANDLE>(f.m_hFile), NULL, NULL, &lastWriteTime)) {
		// Success
		tmModified = lastWriteTime;
	} else {
		// Failure
		CFileException::ThrowOsError((LONG)::GetLastError());
	}
}

CTime GetFileModifiedTime(LPCTSTR strFullPath)
{
	CTime tmModified;
	CFile fIn(strFullPath, CFile::modeRead|CFile::shareDenyNone);
	GetFileModifiedTime(fIn, tmModified);
	return tmModified;
}
*/

void CleanTempHtmFiles(const CString &strPath)
{
	CTime tm;
	CTime tmMin = CTime::GetCurrentTime() - CTimeSpan(1,0,0,0);

	for (long i=1; i<=99; i++) {
		CString strAns;
		strAns.Format("emr%i.htm", i);
		if (DoesExist(strPath ^ strAns)) {
			// (c.haag 2003-07-16 12:41) - The temp file exists. See if it was modified more
			// than 24 hours ago. If it was, then try to delete it.
			// (a.walling 2008-09-18 17:28) - PLID 28040 - Use fileutils
			tm = FileUtils::GetFileModifiedTime(strPath ^ strAns); 
			if (tm < tmMin)
				DeleteFile(strPath ^ strAns);
		}
	}
}
CString GetTempHtmFileName(const CString &strPath)
{
	CString strFileName;		

	// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops (this may be dead code)
	int i = 1;

	for(i = 1; i <= 99; i++) {
		strFileName.Format("emr%i.htm", i);
		if(!DoesExist(strPath ^ strFileName)) {
			return strPath ^ strFileName;
		}
	}

	//Clean up the temp files, and try again.
	CleanTempHtmFiles(strPath);

	for(i=1; i<=99; i++) {
		strFileName.Format("emr%i.htm", i);
		if(!DoesExist(strPath ^ strFileName)) {
			return strPath ^ strFileName;
		}
	}
	//Give up.
	return "";
}

void CEmrChartNote::GenerateTempMergeFile(long nCategoryID, EmrCategoryFormat fmt, const CString& strCatName)
{
	CString strFilename;
	CString strHTML;
	/*
	CString strNewName = strCatName;

	// (c.haag 2004-05-27 08:56) PLID 12614 - Treat field names consistently
	// with how we do it in EMR (See ConvertToHeaderName() in EMRDlg.cpp)
	// First replace every non-alphanumeric character with an underscore
	for (long i=0; i<strNewName.GetLength(); i++) {
		if (!isalnum(strNewName.GetAt(i))) {
			strNewName.SetAt(i, '_');
		}
	}
	// Then make every sequence of more than one underscore into a single underscore
	while (strNewName.Replace("__", "_"));
	*/

	// (j.jones 2007-01-02 10:53) - PLID 24051 - corrected truncation problems by determing exactly
	// what the merge field was truncated to (if at all)
	CString strNewName = ConvertToHeaderName("EMR_Category", strCatName);
	//remove the EMR_Category prefix
	strNewName = strNewName.Right(strNewName.GetLength() - 13);

	// Generate our temporary file
	strFilename.Format("MergeHTML_Cat_EMR_%s_%d.htm", strNewName, m_nEmrID);
	CString strFullPath = GetNxTempPath() ^ strFilename;
	CStdioFile f(strFullPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareCompat);

	// Write the content to the file
	strHTML = "<html><head></head><body>" + GetParagraph(nCategoryID, fmt) + "</body></html>";
	f.WriteString(strHTML);
	f.Close();

	// Add to our temp file list
	m_astrTempFiles.Add(strFullPath);
}

// (c.haag 2004-04-30 13:14) - Generate a single HTML file for every
// individual EMR category so they can be merged into a Word template.
void CEmrChartNote::GenerateTempMergeFiles()
{
	try {
		_RecordsetPtr rsCategories = CreateRecordset("SELECT EmrCategoriesT.ID, EmrCategoriesT.Name AS CatName, EmrCategoriesT.Format "
			"FROM EmrCategoriesT WHERE ID IN (SELECT EmrCategoryID FROM EmrInfoCategoryT WHERE EmrInfoID IN (SELECT EmrInfoID FROM EmrDetailsT WHERE Deleted = 0 AND EMRID = %li)) "
			"ORDER BY EmrCategoriesT.Priority ASC", m_nEmrID);

		while (!rsCategories->eof) {
			GenerateTempMergeFile(AdoFldLong(rsCategories, "ID"), (EmrCategoryFormat)AdoFldLong(rsCategories, "Format"),
				AdoFldString(rsCategories, "CatName"));
			rsCategories->MoveNext();
		}
	}
	NxCatchAll("Error generating temporary merge chart notes");
}

void CEmrChartNote::RemoveTempMergeFiles()
{
	for (long i=0; i < m_astrTempFiles.GetSize(); i++)
	{
		DeleteFileWhenPossible(m_astrTempFiles[i]);
	}
	m_astrTempFiles.RemoveAll();
}

void CEmrChartNote::OutputToWord()
{
	try {
		_RecordsetPtr rsCategories = CreateRecordset("SELECT EmrCategoriesT.ID, EmrCategoriesT.Name AS CatName, EmrCategoriesT.Format "
			"FROM EmrCategoriesT WHERE ID IN (SELECT EmrCategoryID FROM EmrInfoCategoryT WHERE EmrInfoID IN (SELECT EmrInfoID FROM EmrDetailsT WHERE Deleted = 0 AND EMRID = %li)) "
			"ORDER BY EmrCategoriesT.Priority ASC", m_nEmrID);
		if(rsCategories->eof) {
			return;
		}
		_RecordsetPtr rsEmr = CreateRecordset("SELECT ProcedureT.Name, AdditionalNotes FROM EmrMasterT LEFT JOIN (SELECT Min(ProcedureID) AS ProcedureID, EmrID FROM (SELECT * FROM EmrProcedureT WHERE Deleted = 0) AS EmrProcedureT GROUP BY EmrID) EmrProcedureQ ON EmrMasterT.ID = EmrProcedureQ.EmrID LEFT JOIN ProcedureT ON EmrProcedureQ.ProcedureID = ProcedureT.Id WHERE EmrMasterT.ID = %li", m_nEmrID);
		
		//Construct our html string.
		CString strHtml = "<html>";

		//Now, add the header.
		CString strTitle = GetRemotePropertyText("EmrHeaderTitle", "<Procedure> Report", 0, "<None>", true);
		strTitle.Replace("<Procedure>", AdoFldString(rsEmr, "Name", ""));
		if(strTitle != "") {
			strHtml += "<title>" + strTitle + "</title>";
			strHtml += "<h3 align=center>" + strTitle + "</h3>";
		}
		int nField1 = GetRemotePropertyInt("EmrHeaderField1", -1, 0, "<None>", true);
		int nField2 = GetRemotePropertyInt("EmrHeaderField2", -1, 0, "<None>", true);
		int nField3 = GetRemotePropertyInt("EmrHeaderField3", -1, 0, "<None>", true);
		int nField4 = GetRemotePropertyInt("EmrHeaderField4", -1, 0, "<None>", true);
		if(nField1 != -1 || nField2 != -1 || nField3 != -1 || nField4 != -1) {
			//Let's pull these fields from data.
			_RecordsetPtr rsHeaderFields = CreateRecordset("%s", GetHeaderSql(m_nEmrID));
			strHtml += "<table width=\"100%\" border=\"0\" cellspacing=\"0\" frame=\"VOID\" RULES=\"NONE\">";
			if(nField1 != -1 || nField2 != -1) {
				//Put in the first row.
				strHtml += "<tr>";
				if(nField1 == -1) {
					strHtml += "<td align=left></td>";
				}
				else {
					strHtml += "<td align=left>" + GetHeaderFieldDisplayName((EmrHeaderField)nField1) + ": " + 
						AsString(rsHeaderFields->Fields->GetItem(_bstr_t(GetHeaderFieldDataField((EmrHeaderField)nField1)))->Value) + "</td>";
				}
				if(nField2 == -1) {
					strHtml += "<td align=right></td>";
				}
				else {
					strHtml += "<td align=right>" + GetHeaderFieldDisplayName((EmrHeaderField)nField2) + ": " + 
						AsString(rsHeaderFields->Fields->GetItem(_bstr_t(GetHeaderFieldDataField((EmrHeaderField)nField2)))->Value) + "</td>";
				}
				//Done with this row.
				strHtml += "</tr>";
			}
			if(nField3 != -1 || nField4 != -1) {
				//Put in the second row.
				strHtml += "<tr>";
				if(nField3 == -1) {
					strHtml += "<td align=left></td>";
				}
				else {
					strHtml += "<td align=left>" + GetHeaderFieldDisplayName((EmrHeaderField)nField3) + ": " + 
						AsString(rsHeaderFields->Fields->GetItem(_bstr_t(GetHeaderFieldDataField((EmrHeaderField)nField3)))->Value) + "</td>";
				}
				if(nField4 == -1) {
					strHtml += "<td align=right></td>";
				}
				else {
					strHtml += "<td align=right>" + GetHeaderFieldDisplayName((EmrHeaderField)nField4) + ": " + 
						AsString(rsHeaderFields->Fields->GetItem(_bstr_t(GetHeaderFieldDataField((EmrHeaderField)nField4)))->Value) + "</td>";
				}
				//Done with this row.
				strHtml += "</tr>";
			}
			//Done with the table
			strHtml += "</table><p>";
		}


		while(!rsCategories->eof) {
			//Add on the name, bold and underlined
			strHtml += "<b><u>" + AdoFldString(rsCategories, "CatName") + "</u>:</b>  ";

			//Add the paragraph
			strHtml += GetParagraph(AdoFldLong(rsCategories, "ID"), (EmrCategoryFormat)AdoFldLong(rsCategories, "Format"));
			strHtml += "<p>";

			rsCategories->MoveNext();
		}

		//Finally, add the last "category," the Additional Notes.
		CString strNotes = AdoFldString(rsEmr, "AdditionalNotes");
		strNotes.TrimLeft();
		strNotes.TrimRight();
		if(!strNotes.IsEmpty()) {
			strNotes.Replace("\r\n", "<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
			strHtml += "<b><u>Additional Notes</u>:</b> " + strNotes;
		}
		
		//Finish the string.
		strHtml += "</html>";

		//Write this string to a temp file.
		// Save it as a temp file
		CString strTempPath = GetNxTempPath();
		CString strFileName = GetTempHtmFileName(strTempPath);
		
		{
			CFile fiHtm(strFileName, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite);
				
			CArchive arHtm(&fiHtm, CArchive::store);
			try {
				arHtm.WriteString(strHtml);
			
				arHtm.Close();

			} catch (...) {
				arHtm.Abort();
				throw;
			}
			fiHtm.Close();
		}
		
		// (c.haag 2016-01-20) - PLID 68173 - Opens a file in Microsoft Word
		// (z.manning 2016-02-11 09:39) - PLID 68223 - No need to open this in Word
		OpenDocument(strFileName);

		//Delete our file.
		// (z.manning 2016-06-13 12:04) - NX-100836 - We used to open this file in Word so it was okay to
		// try and delete it right away because you can't delete an open Word file. Now that we just open
		// this in the default editor, we may be deleting the file before it even had a chance to open.
		// Instead, let's just flag the file for deletion at next reboot to be safe.
		//DeleteFileWhenPossible(strFileName);
		MoveFileAtStartup(strFileName, nullptr);

		//Done!
	}NxCatchAll("Error in CEmrChartNote::OutputToWord()");

}

CString CEmrChartNote::GetParagraph(long nCategoryID, EmrCategoryFormat Format)
{
	//Get all the items in this category.
	//TES 12/28/2004 - Treat everything that's not single-select lists as text.
	_RecordsetPtr rsItems = CreateRecordset("SELECT CASE WHEN EmrInfoT.DataType = 2 THEN EmrDataT.Data ELSE EmrDetailsT.Text "
		"END AS Data, EmrInfoT.LongForm, PatientID "
		"FROM EmrInfoT INNER JOIN (EmrDetailsT INNER JOIN EmrMasterT ON EmrDetailsT.EmrID = EmrMasterT.ID) ON EmrInfoT.ID = EmrDetailsT.EmrInfoID "
		"LEFT JOIN EMRSelectT ON EMRDetailsT.ID = EMRSelectT.EMRDetailID "
		"LEFT JOIN EmrDataT ON EMRSelectT.EMRDataID = EmrDataT.ID "
		"WHERE EmrDetailsT.EMRID = %li AND EMRDetailsT.Deleted = 0 AND EmrInfoT.ID IN (SELECT EmrInfoID FROM EmrInfoCategoryT WHERE EmrCategoryID = %li) "
		"ORDER BY EmrDetailsT.ID ", 
		m_nEmrID, nCategoryID);

	CString strOutput;
	//First, begin the list based on its format.
	switch(Format) {
	case ecfParagraph:
		//Do nothing.
		break;
	case ecfList:
		strOutput += "<br>";
		break;
	case ecfBulletedList:
		strOutput += "<UL>";
		break;
	case ecfNumberedList:
		strOutput += "<OL>";
		break;
	}
	FieldsPtr fItems = rsItems->Fields;

	while(!rsItems->eof) {
		//Put the pre-sentence tag, based on the format.
		switch(Format) {
		case ecfParagraph:
		case ecfList:
			//Nothing
			break;
		case ecfBulletedList:
		case ecfNumberedList:
			strOutput += "<LI>";
			break;
		}

		//Get the actual sentence.
		CString strSentence = AdoFldString(fItems, "LongForm");
		
		//Get the actual data.
		CString strData = AdoFldString(fItems, "Data");
		
		//Replace all "merge fields" with their actual values.
		strSentence.Replace("<Data>", strData);
		
		//Include endlines.
		strSentence.Replace("\r\n", "<br>");

		//Output the sentence.
		strOutput += strSentence;

		//Put the post-sentence info, based on the format.
		switch(Format) {
		case ecfParagraph:
			strOutput += " ";
			break;
		case ecfList:
		case ecfNumberedList:
		case ecfBulletedList:
			strOutput += "<br>";
			break;
		}

		//Done with this item.
		rsItems->MoveNext();
	}

	// (z.manning 2008-12-10 12:26) - PLID 32392 - For paragraph formatted categories we put a space in between
	// each detail's text. That's fine, but make sure we trim the final space so we're not adding a spece where
	// it doesn't make sense e.g. when a period immediately follows the category merge field.
	if(strOutput.GetLength() > 0) {
		int nLastCharPos = strOutput.GetLength() - 1;
		if(Format == ecfParagraph && strOutput.GetAt(nLastCharPos) == ' ') {
			strOutput.Delete(nLastCharPos);
		}
	}

	//Now the post-paragraph info, based on the format.

	// (j.jones 2005-02-11 14:41) - PLID 15483 - bulleted/numbered lists require a <br>
	// after them to be more compliant with Word. Read this PL item to understand why.
	switch(Format) {
	case ecfParagraph:
	case ecfList:
		//Nothing.
		break;
	case ecfBulletedList:
		strOutput += "</UL>";
		if(strOutput != "<UL></UL>")
			strOutput += "<br>";
		break;
	case ecfNumberedList:
		strOutput += "</OL>";
		if(strOutput != "<OL></OL>")
			strOutput += "<br>";
		break;
	}

	//Done, baby!
	return strOutput;
}