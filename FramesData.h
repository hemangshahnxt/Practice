#pragma once

#include "InternationalUtils.h"

enum EFramesProductCategoryOption
{
	fpcvNone = -1,
	fpcvCollection = -2,
	fpcvManufacturer = -3,
	fpcvBrand = -4,
	fpcvGroup = -5,
};


// (z.manning 2010-06-22 16:38) - PLID 39306 - FramesDataField struct contains info about a specific field
// within a CFramesData object
struct FramesDataField
{
	_variant_t varValue;
	VARTYPE vtTypeIfNotEmpty;
	CString strDbField;
	int nFileStartPos;
	int nFileLength;
	// (z.manning 2010-06-23 15:04) - PLID 39222 - Added display name and col width
	CString strDisplayName;
	long nColumnWidth;

	FramesDataField(const CString strDatabaseField, int nFileStart, int nLengthInFile, CString strDisName, long nColWidth, VARTYPE vtDefaultType)
	{
		strDbField = strDatabaseField;
		vtTypeIfNotEmpty = vtDefaultType;
		varValue.vt = VT_NULL;
		nFileStartPos = nFileStart;
		nFileLength = nLengthInFile;
		strDisplayName = strDisName;
		nColumnWidth = nColWidth;
	}

	CString GetOutput()
	{
		CString strOutput;
		switch(varValue.vt)
		{
			case VT_NULL:
			case VT_EMPTY:
				break;

			case VT_CY:
				strOutput = FormatCurrencyForInterface(VarCurrency(varValue));
				break;

			case VT_BOOL:
				strOutput = VarBool(varValue) ? "Yes" : "No";
				break;

			default:
				strOutput = AsString(varValue);
				break;
		}
		return strOutput;
	}

	void SetValue(CString strValue)
	{
		CString strOutput;
		switch(vtTypeIfNotEmpty)
		{
			case VT_NULL:
			case VT_EMPTY:
				break;

			case VT_CY:
				varValue.cyVal = ParseCurrencyFromInterface(strValue);
				break;

			case VT_BOOL:
				if (strValue.CompareNoCase("true") == 0) {
					varValue = g_cvarTrue;
				}
				else {
					varValue = g_cvarFalse;
				}
				
			break;

			default:
				varValue = _variant_t(strValue);
			break;
		}		
	}

	CString GetSqlValue(BOOL bForBulkInsert = FALSE)
	{
		CString strValue;
		switch(varValue.vt)
		{
			case VT_BSTR:
				strValue = FormatString("%s", VarString(varValue));
				if(!bForBulkInsert) {
					strValue = "'" + _Q(strValue) + "'";
				}
				break;

			case VT_CY:
				strValue = FormatCurrencyForSql(VarCurrency(varValue));
				break;

			case VT_BOOL:
				strValue = VarBool(varValue) ? "1" : "0";
				break;

			case VT_NULL:
				strValue = bForBulkInsert ? "" : "NULL";
				break;

			default:
				ASSERT(FALSE);
				strValue = bForBulkInsert ? "" : "NULL";
				break;
		}
		return strValue;
	}
};

// (z.manning 2010-06-22 16:39) - PLID 39306 - This class keeps track of indivdual frame records within a set of FramesData
class CFramesData
{
public:
	CFramesData(void);
	~CFramesData(void);

	CArray<FramesDataField*,FramesDataField*> m_aryFields;

	// (z.manning 2010-06-22 16:39) - PLID 39306 - Loads FramesInfo from data based on a product ID
	BOOL LoadFromProductID(const long nProductID);

	// (z.manning 2010-06-22 16:40) - PLID 39306 - Will set the values for the FramesData object based on parsing
	// a line from a SpexUPC file.
	void LoadFromFramesFileLine(const CString strLine);

	// (z.manning 2010-06-23 13:18) - PLID 39311 - Loads values based on a datalist2 row (assumes ALL fields are present).
	void LoadFromDatalistRow(LPDISPATCH lpDatalist2, LPDISPATCH lpRow);

	// (b.spivey, September 19, 2011) - PLID 45265 - Loads FramesInfo from the database based on the ID. 
	BOOL LoadFromFrameID(const int nFrameID);

	CString GetOutputByDataField(const CString strDbField);
	// (z.manning 2010-09-22 10:52) - PLID 40619
	_variant_t GetValueByDataField(const CString strDbField);

	// (z.manning 2010-06-22 16:41) - PLID 39306 - Outputs the current FramesData object to the given file for use with
	// bulk importing.
	void WriteToBulkInsertFile(CStdioFile *pfileBulk, const CString strFieldTerminator, const CString strRowTerminator);

	// (z.manning 2010-06-22 16:43) - PLID 39306 - Returns an insert sql statement for the current FramesData object
	// to the given sql batch.
	CString GetInsertSql(IN BOOL bIsCatalog);

	// (z.manning 2010-06-22 16:44) - PLID 39306 - Will contsruct a comma delimited list of the database field names
	// and values that can then be used in a sql statement.
	void GetFieldsAndValuesList(IN BOOL bIsCatalog, OUT CString &strFields, OUT CString &strValues);
	// (s.dhole 2012-03-23 14:20) - PLID 46662  Allow FPC value to be custom value
	void GetFieldsAndValuesList(IN BOOL bIsCatalog,IN CString strFPCValue, OUT CString &strFields, OUT CString &strValues);
	// (z.manning 2010-06-23 14:40) - PLID 39306 - Gets only the FramesData fields
	CString GetFramesDataFieldsList();
	// (z.manning 2010-06-23 18:48) - PLID 39311 - Gets the FramesData fields for an update statement
	CString GetFramesDateSameTableUpdateFieldsList(CString strDestTableAlias, CString strSourceTableAlias);

	// (z.manning 2010-06-23 13:53) - PLID 39311
	CString GetProductName();
	CString GetProductNameSql(); // (z.manning 2010-07-16 10:38) - PLID 39458
	CString GetSize();

	// (j.gruber 2010-06-23 16:25) - PLID 39323
	void SetFieldByDataField(const CString strDbField, CString strValue);
	
	void ApplyFieldsToData(long nProductID);

	// (z.manning 2010-06-23 19:27) - PLID 39311
	CString GetCategoryText(EFramesProductCategoryOption eCatOption);
	// (s.dhole 2012-03-23 14:20) - PLID 46662  set field value
	void SetFieldByvarValue(const CString strDbField, variant_t  varValue);

protected:
	// (z.manning 2010-06-22 16:45) - PLID 39306 - This map keeps track of where each field is in the array of fields.
	// This is to make it more efficient to find a field based on its field name.
	CMap<CString,LPCTSTR,int,int> m_mapDataFieldToArrayIndex;

	// (z.manning 2010-06-22 16:46) - PLID 39306 - Adds a new field to the FramesDataObject
	void AddField(const CString strDbField, int nFileStartPos, int nFileLength, CString strDisplayName, long nColWidth, VARTYPE vt = VT_BSTR);

	// (z.manning 2010-06-22 16:46) - PLID 39306 - This function will use the map to look up a specific
	// field based on the field name.
	FramesDataField* GetFieldByDataField(const CString strDbField, BOOL bIgnoreNotFound = FALSE);

	
};


// (z.manning 2010-06-22 16:47) - PLID 39306 - Class to store an array of FramesData objects
class CFramesDataArray : public CArray<CFramesData*,CFramesData*>
{
public:
	CFramesDataArray();
	~CFramesDataArray();

	// (z.manning 2010-06-22 16:48) - PLID 39306 - This function will go through all FramesData object in
	// the array and create a temp file and then bulk import all the FramesData to the database as catalog entries.
	// (j.jones 2015-12-14 10:56) - PLID 67713 - now this takes in a file import folder to bulk import from
	void BulkImportCatalog(const CString &strBulkImportFilePath);
};