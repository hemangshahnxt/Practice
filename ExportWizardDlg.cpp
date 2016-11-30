// ExportWizardDlg.cpp: implementation of the CExportWizardDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ExportWizardDlg.h"
#include "ExportWizardNameDlg.h"
#include "ExportWizardFormatDlg.h"
#include "ExportWizardFieldsDlg.h"
#include "ExportWizardFiltersDlg.h"
#include "ExportWizardTodoDlg.h"
#include "ExportWizardFileDlg.h"
#include "DateTimeUtils.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CExportWizardDlg::CExportWizardDlg()
{
	Construct("New Stored Export");
	SetWizardMode();
	
	m_nExportID = -1;
	m_ertRecordType = ertPatients;

	//Default to .csv
	m_eotOutputType = eotCharacterSeparated;
	m_strFieldSeparator = ",";
	m_strFieldEscape = ",";
	m_strTextDelimiter = "\"";
	m_strTextEscape = "\"";
	m_strRecordSeparator = "\r\n";
	m_strRecordSeparatorEscape = "\r\n";
	m_bIncludeFieldNames = false;

	m_efoFilterOption = efoAllNew;
	m_fdDateFilter = fdFirstContactDate;
	m_dfoFromDateType = dfoToday;
	m_nFromDateOffset = 0;
	m_dtFilterFrom = COleDateTime::GetCurrentTime();
	m_dfoToDateType = dfoToday;
	m_nToDateOffset = 0;
	m_dtFilterTo = COleDateTime::GetCurrentTime();
	m_nLwFilterID = -1;
	m_bManualSort = false;

	m_bCreateTodo = false;
	m_nTodoIntervalAmount = 1;
	m_diTodoIntervalUnit = diDays;
	m_nTodoUser = -1;

	m_bPromptForFile = true;

	m_bAllowOtherTemplates = false;

	m_nFilterFlags = effNone; // (z.manning 2009-12-14 09:27) - PLID 36576
}

CExportWizardDlg::~CExportWizardDlg()
{

}

using namespace ADODB;

int CExportWizardDlg::DoModal()
{
	CString strCategories;
	if(m_nExportID != -1) {
		try {
			//Load everything from data.
			_RecordsetPtr rs = CreateRecordset("SELECT * FROM ExportT WHERE ID = %li", m_nExportID);
			FieldsPtr Fields = rs->Fields;
			m_strName = AdoFldString(Fields, "Name");
			m_ertRecordType = (ExportRecordType)AdoFldLong(Fields, "BasedOn");
			m_eotOutputType = (ExportOutputType)AdoFldLong(Fields, "OutputType");
			m_bIncludeFieldNames = AdoFldBool(Fields, "IncludeFieldNames")?true:false;
			m_strFieldSeparator = AdoFldString(Fields, "FieldSeparator");
			m_strFieldEscape = AdoFldString(Fields, "FieldEscape");
			m_strTextDelimiter = AdoFldString(Fields, "TextDelimiter");
			m_strTextEscape = AdoFldString(Fields, "TextEscape");
			m_strRecordSeparator = AdoFldString(Fields, "RecordSeparator");
			m_strRecordSeparatorEscape = AdoFldString(Fields, "RecordEscape");
			m_efoFilterOption = (ExportFilterOption)AdoFldLong(Fields, "FilterType");
			m_nFilterFlags = AdoFldLong(Fields, "FilterFlags");
			if(m_efoFilterOption == efoDateOrLwFilter) {
				if(m_nFilterFlags & effDate) {
					m_fdDateFilter = (FilterableDate)AdoFldLong(Fields, "FilterDateType");
					m_dfoFromDateType = (DateFilterOption)AdoFldLong(Fields, "FromDateType");
					if(m_dfoFromDateType == dfoCustom) {
						m_dtFilterFrom = AdoFldDateTime(Fields, "FromDate");
					}
					else {
						m_nFromDateOffset = AdoFldLong(Fields, "FromDateOffset");
					}
					m_dfoToDateType = (DateFilterOption)AdoFldLong(Fields, "ToDateType");
					if(m_dfoToDateType == dfoCustom) {
						m_dtFilterTo = AdoFldDateTime(Fields, "ToDate");
					}
					else {
						m_nToDateOffset = AdoFldLong(Fields, "ToDateOffset");
					}
				}

				if(m_nFilterFlags & effLetterWriting) {
					m_nLwFilterID = AdoFldLong(Fields, "LetterWritingFilterID");
				}
			}
			m_bCreateTodo = AdoFldBool(Fields, "CreateTodo")?true:false;
			if(m_bCreateTodo) {
				m_nTodoIntervalAmount = AdoFldLong(Fields, "TodoIntervalAmount");
				m_diTodoIntervalUnit = (DateInterval)AdoFldLong(Fields, "TodoIntervalUnit");
				m_nTodoUser = AdoFldLong(Fields, "TodoUser");
			}
			m_bManualSort = AdoFldBool(Fields, "ManualSort")?true:false;
			m_bPromptForFile = AdoFldBool(Fields, "PromptForFile")?true:false;
			if(!m_bPromptForFile) {
				m_strFileName = AdoFldString(Fields, "Filename");
			}

			m_bAllowOtherTemplates = AdoFldBool(Fields, "AllowOtherTemplates")?true:false;
			rs->Close();

			rs = CreateRecordset("SELECT Replace, ReplaceWith FROM ExportSpecialCharsT WHERE ExportID = %li", m_nExportID);
			while(!rs->eof) {
				SpecialChar sc;
				sc.strSourceChar = AdoFldString(rs, "Replace");
				sc.strReplaceChar = AdoFldString(rs, "ReplaceWith");
				m_arSpecialChars.Add(sc);
				rs->MoveNext();
			}
			rs->Close();

			rs = CreateRecordset("SELECT FieldID, DynamicID, Format FROM ExportFieldsT WHERE ExportID = %li ORDER BY ExportOrder ASC", m_nExportID);
			while(!rs->eof) {
				SelectedExportField sef;
				sef.nID = AdoFldLong(rs, "FieldID");
				sef.nDynamicID = AdoFldLong(rs, "DynamicID", -1);
				sef.strFormat = AdoFldString(rs, "Format");
				sef.bHasAJoinOrFromClause = GetFieldByID(sef.nID).bHasAJoinOrFromClause;
				m_arExportFields.Add(sef);
				rs->MoveNext();
			}
			rs->Close();

			rs = CreateRecordset("SELECT FieldID, SortDescending, ExtraInfo FROM ExportSortFieldsT WHERE ExportID = %li ORDER BY SortOrder ASC", m_nExportID);
			while(!rs->eof) {
				SortField sf;
				sf.nID = AdoFldLong(rs, "FieldID");
				sf.bSortDescending = AdoFldBool(rs, "SortDescending")?true:false;
				sf.strExtraInfo = AdoFldString(rs, "ExtraInfo","");
				m_arSortFields.Add(sf);
				rs->MoveNext();
			}
			rs->Close();
			
			rs = CreateRecordset("SELECT EmnTemplateID FROM ExportEmnTemplatesT WHERE ExportID = %li", m_nExportID);
			while(!rs->eof) {
				m_arEmnTemplateIDs.Add(AdoFldLong(rs, "EmnTemplateID"));
				rs->MoveNext();
			}
			rs->Close();

			// (z.manning 2009-12-11 09:21) - PLID 36519 - Load history category info
			rs = CreateParamRecordset(
				"SELECT CategoryID, NoteCatsF.Description \r\n"
				"FROM ExportHistoryCategoriesT \r\n"
				"LEFT JOIN NoteCatsF ON ExportHistoryCategoriesT.CategoryID = NoteCatsF.ID \r\n"
				"WHERE ExportID = {INT} \r\n"
				, m_nExportID);
			m_arynCategoryIDs.RemoveAll();
			for(; !rs->eof; rs->MoveNext()) {
				m_arynCategoryIDs.Add(AdoFldLong(rs->GetFields(), "CategoryID"));
				strCategories += AdoFldString(rs->GetFields(), "Description", "") + ", ";
			}
			strCategories.TrimRight(", ");

		}NxCatchAll("Error loading stored Export");
	}

	CExportWizardNameDlg dlgName;
	dlgName.m_strInitialCategoryText = strCategories;
	AddPage(&dlgName);
	CExportWizardFormatDlg dlgFormat;
	AddPage(&dlgFormat);
	CExportWizardFieldsDlg dlgFields;
	AddPage(&dlgFields);
	CExportWizardFiltersDlg dlgFilters;
	AddPage(&dlgFilters);
	CExportWizardTodoDlg dlgTodo;
	AddPage(&dlgTodo);
	CExportWizardFileDlg dlgFile;
	AddPage(&dlgFile);

	int nReturn = CPropertySheet::DoModal();
	if(nReturn == IDOK || nReturn == ID_WIZFINISH) {
		Save();
	}
	return nReturn;
}

void CExportWizardDlg::Save()
{
	try {

		// (j.jones 2006-03-22 12:06) - PLID 19384 - the * fields are complex and add more JOIN/FROM clauses
		// to the export, of which there is a maximum of 256. So warn if we have more thn 200 such fields.
		long nJoinFieldCount = 0;
		if(m_arExportFields.GetSize() > 200) {
			for(int i=0; i < m_arExportFields.GetSize(); i++)  {
				if(m_arExportFields.GetAt(i).bHasAJoinOrFromClause) {
					nJoinFieldCount++;
				}
			}

			if(nJoinFieldCount > 200) {
				CString str;
				str.Format("There is a limit of 200 export fields that have the * indicator.\n"
					"Please remove at least %li of these * fields before continuing.", nJoinFieldCount - 200);
				AfxMessageBox(str);
				return;
			}
		}

		//Save our changes.
		CString strLwFilterID;
		CString strFilterDate, strFromDateType, strToDateType, strFromDateOffset, strToDateOffset, strFromDate, strToDate;
		if(m_efoFilterOption == efoDateOrLwFilter) {
			if(m_nFilterFlags & effDate) {
				strFilterDate.Format("%i", m_fdDateFilter);
				strFromDateType.Format("%i", m_dfoFromDateType);
				strToDateType.Format("%i", m_dfoToDateType);
				if(m_dfoFromDateType == dfoCustom) {
					strFromDate = "'" + FormatDateTimeForSql(m_dtFilterFrom, dtoDate) + "'";
					strFromDateOffset = "NULL";
				}
				else {
					strFromDate = "NULL";
					strFromDateOffset.Format("%i", m_nFromDateOffset);
				}
				if(m_dfoToDateType == dfoCustom) {
					strToDate = "'" + FormatDateTimeForSql(m_dtFilterTo, dtoDate) + "'";
					strToDateOffset = "NULL";
				}
				else {
					strToDate = "NULL";
					strToDateOffset.Format("%i", m_nToDateOffset);
				}
			}
			else {
				strFilterDate = "NULL";
				strFromDateType = "NULL";
				strToDateType = "NULL";
				strFromDateOffset = "NULL";
				strToDateOffset = "NULL";
				strFromDate = "NULL";
				strToDate = "NULL";
			}

			if(m_nFilterFlags & effLetterWriting) {
				strLwFilterID.Format("%li", m_nLwFilterID);
			}
			else {
				strLwFilterID = "NULL";
			}
		}
		else {
			strFilterDate = "NULL";
			strFromDateType = "NULL";
			strToDateType = "NULL";
			strFromDateOffset = "NULL";
			strToDateOffset = "NULL";
			strFromDate = "NULL";
			strToDate = "NULL";

			strLwFilterID = "NULL";
		}
		
		CString strTodoAmount, strTodoUnit, strTodoUser;
		if(m_bCreateTodo) {
			strTodoAmount.Format("%i", m_nTodoIntervalAmount);
			strTodoUnit.Format("%i", m_diTodoIntervalUnit);
			strTodoUser.Format("%li", m_nTodoUser);
		}
		else {
			strTodoAmount = "NULL";
			strTodoUnit = "NULL";
			strTodoUser = "NULL";
		}

		CString strFilename;
		if(m_bPromptForFile) {
			strFilename = "NULL";
		}
		else {
			strFilename = "'" + _Q(m_strFileName) + "'";
		}

		// (a.walling 2010-10-05 10:09) - PLID 40815 - After losing a complicated export after an exception, 
		// I decided to batch all this stuff. I would use a parameterized batch, but this can end up exceeding
		// the limit of parameters.

		CSqlBatch batch;

		if(m_nExportID == -1) {
			m_nExportID = NewNumber("ExportT", "ID");

			// (z.manning 2009-12-14 10:25) - PLID 36576 - Added filter flags
			batch.Add("INSERT INTO ExportT (ID, Name, BasedOn, OutputType, IncludeFieldNames, FieldSeparator, FieldEscape, "
				"TextDelimiter, TextEscape, RecordSeparator, FilterType, FilterDateType, FromDateType, FromDateOffset, FromDate, "
				"ToDateType, ToDateOffset, ToDate, LetterWritingFilterID, CreateTodo, TodoIntervalAmount, TodoIntervalUnit, "
				"TodoUser, PromptForFile, FileName, ManualSort, AllowOtherTemplates, RecordEscape, FilterFlags) "
				"VALUES (%li, '%s', %i, %i, %i, '%s', '%s', '%s', '%s', '%s', %i, %s, %s, %s, %s, %s, %s, %s, %s, %i, %s, %s, %s, "
				"%i, %s, %i, %i, '%s', %i)",
				m_nExportID, _Q(m_strName), m_ertRecordType, m_eotOutputType, m_bIncludeFieldNames?1:0, _Q(m_strFieldSeparator), 
				_Q(m_strFieldEscape), _Q(m_strTextDelimiter), _Q(m_strTextEscape), _Q(m_strRecordSeparator), m_efoFilterOption, 
				strFilterDate, strFromDateType, strFromDateOffset, strFromDate, strToDateType, strToDateOffset, strToDate, 
				strLwFilterID, m_bCreateTodo ? 1 : 0, strTodoAmount, strTodoUnit, strTodoUser, m_bPromptForFile ? 1 : 0, 
				strFilename, m_bManualSort?1:0, m_bAllowOtherTemplates?1:0, _Q(m_strRecordSeparatorEscape), m_nFilterFlags);

		}
		else {
			batch.Add("DELETE FROM ExportSpecialCharsT WHERE ExportID = %li", m_nExportID);
			batch.Add("DELETE FROM ExportFieldsT WHERE ExportID = %li", m_nExportID);
			batch.Add("DELETE FROM ExportEmnTemplatesT WHERE ExportID = %li", m_nExportID);
			batch.Add("DELETE FROM ExportSortFieldsT WHERE ExportID = %li", m_nExportID);
			batch.Add("DELETE FROM ExportHistoryCategoriesT WHERE ExportID = %li", m_nExportID);

			// (z.manning 2009-12-14 10:26) - PLID 36576 - Added filter flags
			batch.Add("UPDATE ExportT SET Name = '%s', BasedOn = %i, OutputType = %i, IncludeFieldNames = %i, "
				"FieldSeparator = '%s', FieldEscape = '%s', TextDelimiter = '%s', TextEscape = '%s', RecordSeparator = '%s', "
				"FilterType = %i, FilterDateType = %s, FromDateType = %s, FromDateOffset = %s, FromDate = %s, ToDateType = %s, "
				"ToDateOffset = %s, ToDate = %s, LetterWritingFilterID = %s, CreateTodo = %i, TodoIntervalAmount = %s, "
				"TodoIntervalUnit = %s, TodoUser = %s, PromptForFile = %i, Filename = %s, ManualSort = %i, "
				"AllowOtherTemplates = %i, RecordEscape = '%s', FilterFlags = %i WHERE ID = %li", _Q(m_strName), m_ertRecordType, m_eotOutputType, 
				m_bIncludeFieldNames?1:0, _Q(m_strFieldSeparator), _Q(m_strFieldEscape), _Q(m_strTextDelimiter), 
				_Q(m_strTextEscape), _Q(m_strRecordSeparator), m_efoFilterOption, strFilterDate, strFromDateType, 
				strFromDateOffset, strFromDate, strToDateType, strToDateOffset, strToDate, strLwFilterID, 
				m_bCreateTodo ? 1 : 0, strTodoAmount, strTodoUnit, strTodoUser, m_bPromptForFile ? 1 : 0, strFilename, 
				m_bManualSort?1:0, m_bAllowOtherTemplates?1:0, _Q(m_strRecordSeparatorEscape), m_nFilterFlags, m_nExportID);
		}

		// (a.walling 2007-11-05 15:18) - PLID 27977 - VS2008 - for() loops
		int i = 0;
		for(i = 0; i < m_arSpecialChars.GetSize(); i++) {
			batch.Add("INSERT INTO ExportSpecialCharsT (ExportID, Replace, ReplaceWith) "
				"VALUES (%li, '%s', '%s')", 
				m_nExportID, _Q(m_arSpecialChars.GetAt(i).strSourceChar), _Q(m_arSpecialChars.GetAt(i).strReplaceChar));
		}
		for(i = 0; i < m_arExportFields.GetSize(); i++) {
			batch.Add("INSERT INTO ExportFieldsT (ExportID, FieldID, DynamicID, ExportOrder, Format) "
				"VALUES (%li, %li, %li, %i, '%s')", 
				m_nExportID, m_arExportFields.GetAt(i).nID, m_arExportFields.GetAt(i).nDynamicID, i, _Q(m_arExportFields.GetAt(i).strFormat));
		}
		for(i = 0; i < m_arSortFields.GetSize(); i++) {
			batch.Add("INSERT INTO ExportSortFieldsT (ExportID, FieldID, SortDescending, SortOrder, ExtraInfo) "
				"VALUES (%li, %li, %i, %i, '%s')",
				m_nExportID, m_arSortFields.GetAt(i).nID, m_arSortFields.GetAt(i).bSortDescending, i, _Q(m_arSortFields.GetAt(i).strExtraInfo));
		}
		for(i = 0; i < m_arEmnTemplateIDs.GetSize(); i++) {
			batch.Add("INSERT INTO ExportEmnTemplatesT (ExportID, EmnTemplateID) "
				"VALUES (%li, %li)", m_nExportID, m_arEmnTemplateIDs.GetAt(i));
		}
		// (z.manning 2009-12-11 09:14) - PLID 36519 - History categories
		for(i = 0; i < m_arynCategoryIDs.GetSize(); i++) {
			// (j.jones 2011-03-29 16:34) - PLID 43038 - make sure we don't save if
			// the ID is -1 or -2
			long nCategoryID = m_arynCategoryIDs.GetAt(i);
			if(nCategoryID != -1 && nCategoryID != -2) {
				batch.Add(
					"INSERT INTO ExportHistoryCategoriesT (ExportID, CategoryID) VALUES (%li, %li)"
					, m_nExportID, nCategoryID);
			}
		}

		batch.Execute();

	}NxCatchAll("Error in CExportWizardDlg::Save()");
}