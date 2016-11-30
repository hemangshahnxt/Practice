#include "stdafx.h"
#include "FramesData.h"
#include "NxException.h"
#include "FileUtils.h"
#include "NxSystemUtilitiesLib\BulkImportUtils.h"

CFramesData::CFramesData(void)
{
	// (z.manning 2010-06-22 16:50) - PLID 39306 - Add all the fields contained in the FramesData SpexUPC files
	// including their positions in the files in case we need to parse them later.
	// (z.manning 2010-06-23 14:59) - PLID 39222 - The order of these fields are added here is the order in which
	// they show up on the Frames tab.
	AddField("ManufacturerName", 311, 50, "Manufacturer", 150);
	AddField("CollectionName", 411, 50, "Collection", 150);
	AddField("StyleName", 36, 50, "Style", 150);
	AddField("ColorCode", 136, 20, "Clr Code", 55);
	AddField("ColorDescription", 86, 50, "Color", 100);
	AddField("Eye", 226, 3, "", 50);
	AddField("Bridge", 229, 10, "", 50);
	AddField("Temple", 239, 10, "", 50);
	AddField("ProductGroupName", 487, 15, "Group", 100);
	AddField("RimType", 502, 25, "Rim", 100);
	AddField("CompletePrice", 304, 7, "Wholesale", 80, VT_CY);
	AddField("UPC", 14, 14, "", 90);
	AddField("BrandName", 361, 50, "Brand", 100);
	AddField("LensColor", 156, 50, "Lens Color", 90);
	AddField("Material", 527, 50, "", 90);
	AddField("FrameShape", 577, 15, "Frame Shape", 100);
	AddField("GenderType", 461, 10, "Gender", 80);
	AddField("AgeGroup", 471, 15, "Age Group", 80);
	AddField("SKU", 624, 30, "", 80);
	AddField("Country", 594, 30, "", 80);
	AddField("YearIntroduced", 654, 4, "Year", 60);

	AddField("FPC", 0, 14, "", 0);
	AddField("StyleID", 28, 8, "", 0);
	AddField("LensColorCode", 206, 20, "", 0);
	AddField("DBL", 249, 10, "", 0);
	AddField("A", 259, 5, "", 0);
	AddField("B", 264, 5, "", 0);
	AddField("ED", 269, 5, "", 0);
	AddField("Circumference", 274, 6, "", 0);
	AddField("EDAngle", 280, 6, "", 0);
	AddField("FrontPrice", 286, 6, "", 0, VT_CY);
	AddField("HalfTemplesPrice", 292, 6, "", 0, VT_CY);
	AddField("TemplesPrice", 298, 6, "", 0, VT_CY);
	AddField("ActiveStatus", 486, 1, "", 0);
	AddField("StyleNew", 592, 1, "", 0, VT_BOOL);
	AddField("ChangedPrice", 593, 1, "", 0, VT_BOOL);
}

CFramesData::~CFramesData(void)
{
	for(int nFieldIndex = 0; nFieldIndex < m_aryFields.GetSize(); nFieldIndex++) {
		FramesDataField *pField = m_aryFields.GetAt(nFieldIndex);
		delete pField;
	}
	m_aryFields.RemoveAll();
}

// (z.manning 2010-06-22 16:46) - PLID 39306 - Adds a new field to the FramesDataObject
void CFramesData::AddField(const CString strDbField, int nFileStartPos, int nFileLength, CString strDisplayName, long nColWidth, VARTYPE vt /* = VT_BSTR */)
{
	if(strDisplayName.IsEmpty()) {
		strDisplayName = strDbField;
	}

	FramesDataField *pFramesField = new FramesDataField(strDbField, nFileStartPos, nFileLength, strDisplayName, nColWidth, vt);
	int nIndex = m_aryFields.Add(pFramesField);
	m_mapDataFieldToArrayIndex.SetAt(strDbField, nIndex);
}

// (z.manning 2010-06-22 16:39) - PLID 39309 - Loads FramesInfo from data based on a product ID
BOOL CFramesData::LoadFromProductID(const long nProductID)
{
	ADODB::_RecordsetPtr prs = CreateParamRecordset(FormatString(
		"SELECT %s \r\n"
		"FROM FramesDataT \r\n"
		"INNER JOIN ProductT ON FramesDataT.ID = ProductT.FramesDataID \r\n"
		"WHERE ProductT.ID = {INT} \r\n"
		, GetFramesDataFieldsList())
		, nProductID);
	if(prs->eof) {
		return FALSE;
	}
	else
	{
		const long nFieldCount = prs->GetFields()->GetCount();
		for(long nFieldIndex = 0; nFieldIndex < nFieldCount; nFieldIndex++) {
			ADODB::FieldPtr pfld = prs->GetFields()->GetItem(nFieldIndex);
			FramesDataField *pFramesField = GetFieldByDataField((LPCTSTR)pfld->GetName());
			pFramesField->varValue = pfld->GetValue();
		}

		return TRUE;
	}
}

// (z.manning 2010-06-22 16:40) - PLID 39306 - Will set the values for the FramesData object based on parsing
// a line from a SpexUPC file.
void CFramesData::LoadFromFramesFileLine(const CString strLine)
{
	for(int nFieldIndex = 0; nFieldIndex < m_aryFields.GetSize(); nFieldIndex++)
	{
		FramesDataField *pFramesField = m_aryFields.GetAt(nFieldIndex);
		if(strLine.GetLength() >= (pFramesField->nFileStartPos + pFramesField->nFileLength)) {
			CString strValue = strLine.Mid(pFramesField->nFileStartPos, pFramesField->nFileLength);
			strValue.Trim();
			if(strValue.IsEmpty()) {
				pFramesField->varValue.vt = VT_NULL;
			}
			else {
				pFramesField->varValue.SetString(strValue);
				if(pFramesField->vtTypeIfNotEmpty != VT_BSTR) {
					pFramesField->varValue.ChangeType(pFramesField->vtTypeIfNotEmpty);
				}
			}
		}
	}
}

// (z.manning 2010-06-23 13:18) - PLID 39311 - Loads values based on a datalist2 row (assumes ALL fields are present).
void CFramesData::LoadFromDatalistRow(LPDISPATCH lpDatalist2, LPDISPATCH lpRow)
{
	NXDATALIST2Lib::_DNxDataListPtr pdl(lpDatalist2);
	if(pdl == NULL) {
		AfxThrowNxException("CFramesData::LoadFromDatalistRow - datalist is null");
	}
	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		AfxThrowNxException("CFramesData::LoadFromDatalistRow - row is null");
	}

	short nColCount = pdl->GetColumnCount();
	int nValueSetCount = 0;
	for(short nCol = 0; nCol < nColCount; nCol++)
	{
		NXDATALIST2Lib::IColumnSettingsPtr pCol = pdl->GetColumn(nCol);
		CString strColField = VarString(pCol->GetFieldName(), "");
		int nDot = strColField.ReverseFind('.');
		if(nDot != -1) {
			strColField = strColField.Mid(nDot + 1);
		}

		FramesDataField *pFramesField = GetFieldByDataField(strColField, TRUE);
		if(pFramesField != NULL) {
			pFramesField->varValue = pRow->GetValue(nCol);
			nValueSetCount++;
		}
	}

	// (z.manning 2010-06-23 13:48) - PLID 39311 - Made sure we set the value of every field
	if(nValueSetCount != m_aryFields.GetSize()) {
		AfxThrowNxException("CFramesData::LoadFromDatalistRow - Set %li fields, expected %li", nValueSetCount, m_aryFields.GetSize());
	}
}
// (b.spivey, September 15, 2011) - PLID 45265 - Get FramesData from the database by filtering on FramesID. 
BOOL CFramesData::LoadFromFrameID(const int nFrameID)
{
	// Because there is no product created for the item yet and the UPC is not always unique, we need this function 
	//	 in case the user decides to select their own set of frames instead of using the ones that were scanned in. 
	ADODB::_RecordsetPtr prs = CreateParamRecordset(FormatString(
		"SELECT %s "
		"FROM FramesDataT "
		"WHERE ID = {INT} ", GetFramesDataFieldsList()), nFrameID);

	if(prs->eof) {
		return FALSE;
	}
	else
	{
		//Proceed like LoadFromProductID()
		const long nFieldCount = prs->GetFields()->GetCount();
		for(long nFieldIndex = 0; nFieldIndex < nFieldCount; nFieldIndex++) {
			ADODB::FieldPtr pfld = prs->GetFields()->GetItem(nFieldIndex);
			FramesDataField *pFramesField = GetFieldByDataField((LPCTSTR)pfld->GetName());
			pFramesField->varValue = pfld->GetValue();
		}

		return TRUE;
	}
}

// (z.manning 2010-06-22 16:46) - PLID 39306 - This function will use the map to look up a specific
// field based on the field name.
FramesDataField* CFramesData::GetFieldByDataField(const CString strDbField, BOOL bIgnoreNotFound /* = FALSE */)
{
	int nFramesFieldIndex = -1;
	if(!m_mapDataFieldToArrayIndex.Lookup(strDbField, nFramesFieldIndex)) {
		if(!bIgnoreNotFound) {
			AfxThrowNxException("Could not find field pointer for database field %s", strDbField);
		}
	}

	FramesDataField *pFramesField = NULL;
	if(nFramesFieldIndex >= 0) {
		pFramesField = m_aryFields.GetAt(nFramesFieldIndex);
	}
	return pFramesField;
}

// (j.gruber 2010-06-23 16:25) - PLID 39323
void CFramesData::SetFieldByDataField(const CString strDbField, CString strValue)
{
	int nFramesFieldIndex = -1;
	if(!m_mapDataFieldToArrayIndex.Lookup(strDbField, nFramesFieldIndex)) {
		AfxThrowNxException("Could not find field pointer for database field %s", strDbField);		
	}

	FramesDataField *pFramesField = NULL;
	if(nFramesFieldIndex >= 0 && nFramesFieldIndex < m_aryFields.GetSize()) {
		pFramesField = m_aryFields.GetAt(nFramesFieldIndex);
	}
	if (pFramesField) {
		pFramesField->SetValue(strValue);
	}
}
void CFramesData::SetFieldByvarValue(const CString strDbField, variant_t  varValue)
{
	int nFramesFieldIndex = -1;
	if(!m_mapDataFieldToArrayIndex.Lookup(strDbField, nFramesFieldIndex)) {
		AfxThrowNxException("Could not find field pointer for database field %s", strDbField);		
	}

	FramesDataField *pFramesField = NULL;
	if(nFramesFieldIndex >= 0 && nFramesFieldIndex < m_aryFields.GetSize()) {
		pFramesField = m_aryFields.GetAt(nFramesFieldIndex);
	}
	if (pFramesField) {
		pFramesField->varValue =varValue ;
	}
}


// (j.gruber 2010-06-23 16:25) - PLID 39323
void CFramesData::ApplyFieldsToData(long nProductID)
{
	CString strSqlBatch;
	CString strTemp;
	
	strSqlBatch = "UPDATE FramesDataT SET "; 
	for (int i = 0; i < m_aryFields.GetSize(); i++) {
		FramesDataField *pField;
		pField = m_aryFields.GetAt(i);

		if (pField) {

			strTemp  = pField->strDbField + " = ";

			switch (pField->vtTypeIfNotEmpty) {

				case VT_NULL:
				case VT_EMPTY:
					strTemp += "''";
				break;

				case VT_CY:					
					strTemp += "convert(money, '" + _Q(pField->GetSqlValue()) + "')";
				break;

				case VT_BOOL:
					if (VarBool(pField->varValue)) {
						strTemp += "1";
					}
					else {
						strTemp += "0";
					}
				break;

				default:
					strTemp += "'" + _Q(pField->GetOutput()) + "'";
				break;
			}

			strSqlBatch += strTemp + ", ";
		}
	}

	if (strSqlBatch != "UPDATE FramesDataT SET ") {

		//take off the last comma
		strSqlBatch.TrimRight(", ");

		strSqlBatch += " FROM FramesDataT INNER JOIN ProductT ON FramesDataT.ID = ProductT.FramesDataID \r\n"
			"WHERE ProductT.ID = " + AsString(nProductID);

		ExecuteSqlStd(strSqlBatch);
	}

}


CString CFramesData::GetOutputByDataField(const CString strDbField)
{
	FramesDataField *pFramesField = GetFieldByDataField(strDbField);
	return pFramesField->GetOutput();
}

// (z.manning 2010-09-22 10:53) - PLID 40619
_variant_t CFramesData::GetValueByDataField(const CString strDbField)
{
	FramesDataField *pFramesField = GetFieldByDataField(strDbField);
	return pFramesField->varValue;
}

// (z.manning 2010-06-22 16:44) - PLID 39306 - Will contsruct a comma delimited list of the database field names
// and values that can then be used in a sql statement.
void CFramesData::GetFieldsAndValuesList(IN BOOL bIsCatalog, OUT CString &strFields, OUT CString &strValues)
{
	strFields.Empty();
	strValues.Empty();

	// (z.manning 2010-06-22 16:55) - PLID 39306 - IsCatalog is not part of the FramesData stuff but is just
	// a flag I added to differentiate between catalog data and data that is tied to inventory items.
	strFields += "IsCatalog, ";
	strValues += FormatString("%d, ", bIsCatalog ? 1 : 0);

	for(int nFieldIndex = 0; nFieldIndex < m_aryFields.GetSize(); nFieldIndex++)
	{
		FramesDataField *pFramesField = m_aryFields.GetAt(nFieldIndex);
		strFields += pFramesField->strDbField + ", ";
		strValues += pFramesField->GetSqlValue() + ", ";
	}

	strFields.TrimRight(", ");
	strValues.TrimRight(", ");
}

// (s.dhole 2012-03-23 14:20) - PLID 46662  Allow FPC value to be custom value
void CFramesData::GetFieldsAndValuesList(IN BOOL bIsCatalog,IN CString strFPCValue, OUT CString &strFields, OUT CString &strValues)
{
	strFields.Empty();
	strValues.Empty();

	// (z.manning 2010-06-22 16:55) - PLID 39306 - IsCatalog is not part of the FramesData stuff but is just
	// a flag I added to differentiate between catalog data and data that is tied to inventory items.
	strFields += "IsCatalog, ";
	strValues += FormatString("%d, ", bIsCatalog ? 1 : 0);

	for(int nFieldIndex = 0; nFieldIndex < m_aryFields.GetSize(); nFieldIndex++)
	{
		FramesDataField *pFramesField = m_aryFields.GetAt(nFieldIndex);
		strFields += pFramesField->strDbField + ", ";
		if (!pFramesField->strDbField.IsEmpty() && pFramesField->strDbField.CompareNoCase("FPC")  ){
			strValues += pFramesField->GetSqlValue() + ", ";
		}
		else
		{
			strValues += strFPCValue + ", ";
		}
	}
	strFields.TrimRight(", ");
	strValues.TrimRight(", ");
}



// (z.manning 2010-06-23 14:40) - PLID 39306 - Gets only the FramesDataT fields
CString CFramesData::GetFramesDataFieldsList()
{
	CString strFields;
	for(int nFieldIndex = 0; nFieldIndex < m_aryFields.GetSize(); nFieldIndex++)
	{
		FramesDataField *pFramesField = m_aryFields.GetAt(nFieldIndex);
		strFields += pFramesField->strDbField + ", ";
	}
	strFields.TrimRight(", ");
	return strFields;
}

// (z.manning 2010-06-23 18:48) - PLID 39311 - Gets the FramesData fields for an update statement
CString CFramesData::GetFramesDateSameTableUpdateFieldsList(CString strDestTableAlias, CString strSourceTableAlias)
{
	CString strUpdateList;
	for(int nFieldIndex = 0; nFieldIndex < m_aryFields.GetSize(); nFieldIndex++)
	{
		FramesDataField *pFramesField = m_aryFields.GetAt(nFieldIndex);
		strUpdateList += FormatString("%s.%s = %s.%s, ", strDestTableAlias, pFramesField->strDbField, strSourceTableAlias, pFramesField->strDbField);
	}
	strUpdateList.TrimRight(", ");
	return strUpdateList;
}

// (z.manning 2010-06-22 16:43) - PLID 39306 - Returns an insert sql statement for the current FramesData object
// to the given sql batch.
CString CFramesData::GetInsertSql(IN BOOL bIsCatalog)
{
	CString strFieldSql, strValueSql;
	GetFieldsAndValuesList(bIsCatalog, strFieldSql, strValueSql);
	return FormatString("INSERT INTO FramesDataT (%s) VALUES (%s);", strFieldSql, strValueSql);
}

// (z.manning 2010-06-22 16:41) - PLID 39306 - Outputs the current FramesData objec to the given file for use with
// bulk importing.
void CFramesData::WriteToBulkInsertFile(CStdioFile *pfileBulk, const CString strFieldTerminator, const CString strRowTerminator)
{
	CString strLine = "1" + strFieldTerminator;
	for(int nFieldIndex = 0; nFieldIndex < m_aryFields.GetSize(); nFieldIndex++) {
		FramesDataField *pFramesField = m_aryFields.GetAt(nFieldIndex);
		strLine += pFramesField->GetSqlValue(TRUE) + strFieldTerminator;
	}
	strLine.Delete(strLine.GetLength() - strFieldTerminator.GetLength(), strFieldTerminator.GetLength());
	strLine += strRowTerminator;
	pfileBulk->Write(strLine.GetBuffer(strLine.GetLength()), strLine.GetLength());
	strLine.ReleaseBuffer();
}

// (z.manning 2010-07-16 10:39) - PLID 39458
// KEEP IN SYNC WITH CFramesData::GetProductName
CString CFramesData::GetProductNameSql()
{
	return
		"	CASE WHEN COALESCE(CollectionName,'') = '' THEN '' ELSE CollectionName + ', ' END + \r\n"
		"	CASE WHEN COALESCE(StyleName,'') = '' THEN '' ELSE StyleName + ', ' END + \r\n"
		"	CASE WHEN COALESCE(ColorDescription,'') = '' THEN '' ELSE ColorDescription + ', ' END + \r\n"
		"	CASE WHEN COALESCE(ColorCode,'') = '' THEN '' ELSE ColorCode + ', ' END + \r\n"
		"	CASE WHEN COALESCE(Eye,'') = '' AND COALESCE(Bridge,'') = '' AND COALESCE(Temple,'') = '' THEN '' ELSE Eye + '-' + Bridge + '-' + Temple END";
}

// (z.manning 2010-06-23 13:53) - PLID 39311
// KEEP IN SYNC WITH CFramesData::GetProductNameSql
CString CFramesData::GetProductName()
{
	CString strProductName, str;
	FramesDataField *pFramesField = NULL;

	// (z.manning 2010-07-16 10:31) - PLID 39458 - We now use collection and style for the name instead of manufacturer
	pFramesField = GetFieldByDataField("CollectionName");
	str = pFramesField->GetOutput();
	if(!str.IsEmpty()) {
		strProductName += str + ", ";
	}
	
	// (z.manning 2010-07-16 10:31) - PLID 39458 - We now use collection and style for the name instead of manufacturer
	pFramesField = GetFieldByDataField("StyleName");
	str = pFramesField->GetOutput();
	if(!str.IsEmpty()) {
		strProductName += str + ", ";
	}

	pFramesField = GetFieldByDataField("ColorDescription");
	str = pFramesField->GetOutput();
	if(!str.IsEmpty()) {
		strProductName += str + ", ";
	}

	pFramesField = GetFieldByDataField("ColorCode");
	str = pFramesField->GetOutput();
	if(!str.IsEmpty()) {
		strProductName += str + ", ";
	}

	str = GetSize();
	if(!str.IsEmpty()) {
		strProductName += str + ", ";
	}

	strProductName.TrimRight(", ");
	return strProductName;
}

// (z.manning 2010-06-23 14:13) - PLID 39311
CString CFramesData::GetSize()
{
	CString strSize = FormatString("%s-%s-%s", GetFieldByDataField("Eye")->GetOutput(), GetFieldByDataField("Bridge")->GetOutput(), GetFieldByDataField("Temple")->GetOutput());
	if(strSize.SpanIncluding("-") == strSize) {
		return "";
	}
	
	return strSize;
}

// (z.manning 2010-06-23 19:27) - PLID 39311
CString CFramesData::GetCategoryText(EFramesProductCategoryOption eCatOption)
{
	CString strCategory;
	switch(eCatOption)
	{
		case fpcvCollection:
			strCategory = GetFieldByDataField("CollectionName")->GetOutput();
			break;

		case fpcvManufacturer:
			strCategory = GetFieldByDataField("ManufacturerName")->GetOutput();
			break;

		case fpcvBrand:
			strCategory = GetFieldByDataField("BrandName")->GetOutput();
			break;

		case fpcvGroup:
			strCategory = GetFieldByDataField("ProductGroupName")->GetOutput();
			break;
	}

	return strCategory;
}


// End class CFramesData
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin class CFramesDataArray

CFramesDataArray::CFramesDataArray()
{
}

CFramesDataArray::~CFramesDataArray()
{
	for(int nFramesIndex = 0; nFramesIndex < this->GetSize(); nFramesIndex++) {
		CFramesData *pFramesData = this->GetAt(nFramesIndex);
		delete pFramesData;
	}
	this->RemoveAll();
}

// (z.manning 2010-06-22 16:48) - PLID 39306 - This function will go through all FramesData object in
// the array and create a temp file and then bulk import all the FramesData to the database as catalog entries.
// (j.jones 2015-12-14 10:56) - PLID 67713 - now this takes in a file import folder to bulk import from
void CFramesDataArray::BulkImportCatalog(const CString &strBulkImportFilePath)
{
	if(GetSize() == 0) {
		return;
	}

	// (z.manning 2010-06-24 10:00) - PLID 39306 - The path must be on the server.
	if(!FileUtils::EnsureDirectory(strBulkImportFilePath)) {
		AfxThrowNxException("Failed to ensure path '%s' exists for frames bulk importing (last error = %u)", strBulkImportFilePath, GetLastError());
	}

	// (z.manning 2010-06-22 16:55) - PLID 39306 - Create a file to use for the bulk import.
	CString strBulkFile = FormatString("%s_%u.txt", strBulkImportFilePath ^ "FramesDataImport", GetTickCount());
	CStdioFile fileBulk;
	if(!fileBulk.Open(strBulkFile, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
		AfxThrowNxException("Failed to create file '%s' for frames bulk importing (last error = %u)", strBulkFile, GetLastError());
	}

	// (z.manning 2010-06-22 16:56) - PLID 39306 - Set the field and row terminators for the bulk import file
	// (and then set escaped versions to pass in the bulk insert query later).
	const CString strFieldTerminator = "|\t|";
	const CString strFieldTerminatorForSql = "|\\t|";
	const CString strRowTerminator = "\r\n";
	const CString strRowTerminatorForSql = "\\r\\n";

	// (z.manning 2010-06-22 16:56) - PLID 39306 - Prepare the bulk import file
	for(int nFramesIndex = 0; nFramesIndex < this->GetSize(); nFramesIndex++) {
		CFramesData *pFramesData = GetAt(nFramesIndex);
		pFramesData->WriteToBulkInsertFile(&fileBulk, strFieldTerminator, strRowTerminator);
	}
	fileBulk.Close();

	CString strSelectFields, strTemp;
	GetAt(0)->GetFieldsAndValuesList(TRUE, strSelectFields, strTemp);

	try
	{
		// (z.manning 2010-06-22 16:57) - PLID 39306 - Instead of bulk inserting directly into the table, create a view
		// to bulk insert into so that we are 100% positive of the order of the columns.
		// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
		NxAdo::PushMaxRecordsWarningLimit pmr(1000000);
		ExecuteSql("IF OBJECT_ID('FramesDataQ', 'V') IS NOT NULL DROP VIEW FramesDataQ \r\n");
		ExecuteSql("CREATE VIEW FramesDataQ AS SELECT %s FROM FramesDataT \r\n", strSelectFields);
		ExecuteSql("BULK INSERT FramesDataQ FROM '%s' WITH (KEEPIDENTITY, KEEPNULLS, CODEPAGE = 'ACP', FIELDTERMINATOR = '%s', ROWTERMINATOR = '%s') \r\n"
			, _Q(strBulkFile), strFieldTerminatorForSql, strRowTerminatorForSql);
		ExecuteSql("DROP VIEW FramesDataQ \r\n");
	}
	NxCatchAllSilentCallThrow(DeleteFile(strBulkFile));

	// (z.manning 2010-06-22 16:58) - PLID 39306 - Delete the temp file as we are now done with it
	DeleteFile(strBulkFile);
}