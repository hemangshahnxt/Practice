
#pragma once

// (j.gruber 2013-09-30 10:58) - PLID 58676 - created for

class CEmrInfoDataElementArray;

class CEMRCode;

/*class CEMRCodeEditRowPtr
{	
public:
	CEMRCodeEditRowPtr();
	friend class CEMRCode;
	friend class CEMREditTableCellCodeDlg;
	friend class CEMRCodeEditorDlg;

	
	
	void PutCode(CEMRCode code);
	void CopyRowValues(CEMRCodeEditRowPtr pRow);
	NXDATALIST2Lib::IRowSettingsPtr GetRow();
	NXDATALIST2Lib::IRowSettingsPtr CEMRCodeEditRowPtr::GetNextRow();
private:
	NXDATALIST2Lib::IRowSettingsPtr m_pRow;


};*/

// (a.walling 2014-06-30 10:21) - PLID 62497 - Removed incorrect assignment operator; now uses auto-generated copy and assignment

class CEMRCode
{
	friend class CEMRCodeEditRowPtr;

public:
	CEMRCode()
	{}

	CEMRCode(long nID, CString strVocab, CString strCode, CString strName)
	{
		m_nID = nID;
		m_strCode = strCode;
		m_strVocab = strVocab;
		m_strName = strName;
	}

	long GetID() const;
	CString GetCode() const;
	CString GetVocab() const;
	CString GetName() const;

	// (a.walling 2014-06-30 10:21) - PLID 62497 - CEMRCode now overloads operator< and operator==

	friend bool operator<(const CEMRCode& l, const CEMRCode& r)
	{
		if (l.m_nID < r.m_nID) {
			return true;
		}
		if (l.m_nID > r.m_nID) {
			return false;
		}
		return false;
	}

	friend bool operator==(const CEMRCode& l, const CEMRCode& r)
	{
		return l.m_nID == r.m_nID;
	}

private:
	CString m_strCode;
	long m_nID = -1;
	CString m_strVocab;
	CString m_strName;
	
};

// (a.walling 2014-06-30 10:21) - PLID 62497 - Fixed copy and assignment
class CEMRCodeArray : public CArray<CEMRCode, CEMRCode> 
{
public:
	CEMRCodeArray()
	{}

	CEMRCodeArray(const CEMRCodeArray& r);
	CEMRCodeArray& operator = (const CEMRCodeArray& r);

	BOOL IsDifferent(const CEMRCodeArray *pAryToCompare) const;
	CString GetCodeString() const;
	CString GenerateXML(CString strRoot) const;
};

// (a.walling 2014-04-24 12:00) - VS2013 - no using std in global headers

typedef std::map<std::pair<CString, CString>, CEMRCodeArray*>::iterator CEMRTableCellCodeIterator;
typedef std::map<std::pair<CString, CString>, CEMRCodeArray*>::reverse_iterator CEMRTableCellCodeReverseIterator;
typedef std::map<std::pair<CString, CString>, CEMRCodeArray *> CEMRTableCellCodeMap;
typedef std::pair<CString, CString> CEMRCellPair;

class CEMRTableCell
{
public :
	CEMRTableCell(CString strRowID, CString strColID);	
	~CEMRTableCell();

	CString GetRowDataID(CEmrInfoDataElementArray* pDataElementArray);
	CString GetColDataID(CEmrInfoDataElementArray* pDataElementArray);

	CString m_strRowID;
	CString m_strColID;
	
	void SetDataIDs(long nRowDataID, long nColDataID);
	long GetRowDataID();
	long GetColDataID();

	CString GetText();
private:
	long m_nRowDataID;
	long m_nColDataID;
};



//	typedef std::map<pair<long, long>, CArray<CodeStruct, CodeStruct>*> TableCellCodes;
class CEMRTableCellCodes
{
public:

	CEMRTableCellCodes();
	~CEMRTableCellCodes();

	void Load(ADODB::_RecordsetPtr rs);
	CEMRCodeArray* GetCodes(CEMRTableCell cell, BOOL bRemove);
	long GetCodeCount(CEMRTableCell cell);
	void Clear();
	BOOL SplitIntoChangedLists(CEMRTableCellCodes *Compare, CEMRTableCellCodes *addedMap, CEMRTableCellCodes *changedMap, CEMRTableCellCodes *deleteMap);
	
	CString GetCodeString(CEMRTableCell cell);	

	void AddCode(CEMRTableCell cell, CString strCode);
	void AddCodes(CEMRTableCell cell, CEMRCodeArray* pCodes);
	void RemoveCode(CEMRTableCell cell, CString strCode);
	void RemoveCodes(CEMRTableCell cell);

	//CEMRTableCellCodeMap GetMap();
	CEMRTableCellCodeIterator begin();
	CEMRTableCellCodeIterator end();
	CEMRTableCellCodeIterator find(CEMRCellPair p);
	void insert(CEMRTableCell cell, CEMRCodeArray *pAry);
	long size();
	void swap(CEMRTableCellCodes *pSwap);

	void UpdateColumnName(CString strOldName, CString strNewName);
	void UpdateRowName(CString strOldName, CString strNewName);
	void RemoveColumn(CString strColumnName);
	void RemoveRow(CString strRowName);	

	BOOL DoesColumnHaveCodes(CString strColumnName);
	BOOL DoesRowHaveCodes(CString strRowName);
	BOOL DoesKeyHaveCodes(BOOL bIsRow, CString strName);
	
	
	void operator=(CEMRTableCellCodes& src);
	//void CopyTableCellMap(TableCellCodes *pFrom, TableCellCodes *pTo);

private:
/*	CString strRowID;
	CString strColID;
	CEMRCodeArray *pArray;*/

	CEMRTableCellCodeMap m_mapCodes;
	void UpdateKey(BOOL bUpdate, CString strOldName, CString strNewName);
	void RemoveKey(BOOL bUpdateRow, CString strKeyName);
};