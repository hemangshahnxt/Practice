#include "stdafx.h"
#include "EMRTableCellCodes.h"
#include "EMRItemEntryDlg.h"
#include <boost/container/flat_set.hpp>

using namespace ADODB;

// (a.walling 2014-04-24 12:00) - VS2013 - no using std in global headers
using namespace std;

// (j.gruber 2013-09-30 10:58) - PLID 58676 - created for

// (a.walling 2014-06-30 10:21) - PLID 62497 - CEMRCode now overloads operator< and operator==


/*void CEMRCodeEditRowPtr::PutCode(CEMRCode code)
{			
	GetInterfacePtr()->PutValue(clcID, code.m_nID);
	GetInterfacePtr()->PutValue(clcVocab, _variant_t(code.m_strVocab));
	GetInterfacePtr()->PutValue(clcCode, _variant_t(code.m_strCode));
	GetInterfacePtr()->PutValue(clcName, _variant_t(code.m_strName));
}

void CEMRCodeEditRowPtr::CopyRowValues(CEMRCodeEditRowPtr pRow)
{
	GetInterfacePtr()->PutValue(clcID, pRow->GetValue(clcID));
	GetInterfacePtr()->PutValue(clcCode, pRow->GetValue(clcCode));
	GetInterfacePtr()->PutValue(clcVocab, pRow->GetValue(clcVocab));
	GetInterfacePtr()->PutValue(clcName, pRow->GetValue(clcName));
}*/

// (a.walling 2014-06-30 10:21) - PLID 62497 - Removed incorrect assignment operator; now uses auto-generated copy and assignment

long CEMRCode::GetID() const
{
	return m_nID;
}

CString CEMRCode::GetVocab() const
{
	return m_strVocab;
}

CString CEMRCode::GetCode() const
{
	return m_strCode;
}


CString CEMRCode::GetName() const
{
	return m_strName;
}



// (a.walling 2014-06-30 10:21) - PLID 62497 - Fixed copy and assignment

CEMRCodeArray::CEMRCodeArray(const CEMRCodeArray& r)
{
	this->Copy(r);
}

CEMRCodeArray& CEMRCodeArray::operator=(const CEMRCodeArray& r)
{	
	RemoveAll();

	this->Copy(r);

	return *this;
}

BOOL CEMRCodeArray::IsDifferent(const CEMRCodeArray *pAryToCompare) const
{
	//first check if the sizes are the same
	if (GetSize() != pAryToCompare->GetSize()) {
		//it changed
		return TRUE;
	}

	// (a.walling 2014-06-30 10:21) - PLID 62497 - Just compare two sorted vectors.
	std::vector<CEMRCode> l(GetData(), GetData() + GetSize());
	std::vector<CEMRCode> r(pAryToCompare->GetData(), pAryToCompare->GetData() + pAryToCompare->GetSize());

	std::sort(begin(l), end(l));
	std::sort(begin(r), end(r));

	if (l == r) {
		return FALSE;
	} else {
		return TRUE;
	}
}

CString CEMRCodeArray::GetCodeString() const
{

	CString strReturn;

	//loop through the array and get the codes
	for(int i=0; i < GetSize(); i++)
	{
		CEMRCode code = GetAt(i);
		strReturn += code.GetCode() + ", ";
	}

	//take off the extra comma
	strReturn = strReturn.Left(strReturn.GetLength() - 2);

	return strReturn;
}

CString CEMRCodeArray::GenerateXML(CString strRoot) const
{
	CString strCodeXml;

	for(int nCodeIndex = 0; nCodeIndex < GetCount(); nCodeIndex++)
	{
		CEMRCode code = GetAt(nCodeIndex);
		strCodeXml += FormatString("<%s CodeID=\"%li\" />\r\n", strRoot, code.GetID());
	}
	
	return strCodeXml;
}

CEMRTableCell::CEMRTableCell(CString strRowID, CString strColID)
{
	m_strRowID = strRowID;
	m_strColID = strColID;

	m_nRowDataID = -1;
	m_nColDataID = -1;
}
CEMRTableCell::~CEMRTableCell()
{
}

//this function is responsible for gathering the EMRDataGroupID or the string that needs to be passed into the save string
CString CEMRTableCell::GetColDataID(CEmrInfoDataElementArray *pDataElementArray)
{
	//we have to search the given array for our columnID
	long nIndex = pDataElementArray->FindDataElement(m_strColID);
	CEmrInfoDataElement *pElement = pDataElementArray->GetAt(nIndex);

	if (pElement) 
	{
		//is this a new row?
		if (pElement->m_nID == -1) {
			//return the arbitrary value
			CString strReturn;						
			strReturn.Format("@nEMRDataGroupBase + %s", pElement->m_strArbitraryGeneratedXmlValue);
			return strReturn;
		}
		else {
			//its a saved value already, so we have the dataID
			CString strReturn;
			strReturn.Format("%li", pElement->m_nDataGroupID);
			return strReturn;
		}
	}
	
	//if we get here, something went wrong, error out
	ThrowNxException("Error in CEMRTableCell::GetColDataID - Could not find data in Column Array; Data = " + m_strColID);
}

//this function is responsible for gathering the EMRDataGroupID or the string that needs to be passed into the save string
CString CEMRTableCell::GetRowDataID(CEmrInfoDataElementArray *pDataElementArray)
{
	//we have to search the given array for our row	
	long nIndex = pDataElementArray->FindDataElement(m_strRowID);
	CEmrInfoDataElement *pElement = pDataElementArray->GetAt(nIndex);

	if (pElement) 
	{
		//is this a new row?
		if (pElement->m_nID == -1) {
			//return the arbitrary value
			CString strReturn;
			strReturn.Format("@nEMRDataGroupBase + %s", pElement->m_strArbitraryGeneratedXmlValue);
			return strReturn;
		}
		else {
			//its a saved value already, so we have the dataID
			CString strReturn;
			strReturn.Format("%li", pElement->m_nDataGroupID);
			return strReturn;
		}
	}
	
	//if we get here, something went wrong, error out
	ThrowNxException("Error in CEMRTableCell::GetRowDataID - Could not find data in Row Array; Data = " + m_strRowID);

}

CString CEMRTableCell::GetText()
{
	//since we are using the row/col names as our IDs, we just need to spit that back out
	return m_strRowID + " / " + m_strColID;

}

void CEMRTableCell::SetDataIDs(long nRowDataID, long nColDataID)
{
	m_nRowDataID = nRowDataID;
	m_nColDataID = nColDataID;
}
long CEMRTableCell::GetRowDataID(){
	return m_nRowDataID;
}

long CEMRTableCell::GetColDataID()
{
	return m_nColDataID;
}



CEMRTableCellCodes::CEMRTableCellCodes()
{

}

CEMRTableCellCodes::~CEMRTableCellCodes()
{
	Clear();
}


CEMRCodeArray * CEMRTableCellCodes::GetCodes(CEMRTableCell cell, BOOL bRemove)
{
	//look up in our map to see if our row/col combination exists yet
	CEMRTableCellCodeIterator it = find(CEMRCellPair(cell.m_strRowID, cell.m_strColID));
	CEMRCodeArray* pAry = NULL;
	
	if (it != end()) 
	{
		//we already have one
		pAry = it->second;

		if (bRemove) {
			m_mapCodes.erase(it);
		}
	}
	else 
	{
		//make a new array
		pAry = new CEMRCodeArray();
	}

	return pAry;
}

void CEMRTableCellCodes::Clear()
{
/*	CEMRTableCellCodeReverseIterator itr;	
	for (itr; itr != m_mapCodes.rend(); itr++)
	{
		CEMRCodeArray *pAry = itr->second;

		m_mapCodes.erase(itr);

		if (pAry->GetCount() > 0) {
			//they aren't pointers, so we can just remove them
			pAry->RemoveAll();
		}
		delete pAry;
		pAry = NULL;
	}*/

	BOOST_FOREACH( CEMRTableCellCodeMap::value_type& p, m_mapCodes)
	{
		delete p.second;
	}
	m_mapCodes.clear();

}

// (j.gruber 2013-09-23 09:40) - PLID 58676
void CEMRTableCellCodes::Load(ADODB::_RecordsetPtr rs)
{
	//clear our existing map if necessary
	Clear();	

	while (! rs->eof) 
	{
		// (j.gruber 2014-01-20 10:30) - PLID 6095 - the X's and Y's were reversed
		long nRowDataID = AdoFldLong(rs->Fields, "EMRDataGroupID_X");
		long nColDataID = AdoFldLong(rs->Fields, "EMRDataGroupID_Y");

		CString strRowData = AdoFldString(rs->Fields, "X_Data");
		CString strColData = AdoFldString(rs->Fields, "Y_Data");
		
		long nCodeID = AdoFldLong(rs->Fields, "CodeID");
		CString strCode = AdoFldString(rs->Fields, "Code");
		CString strVocab = AdoFldString(rs->Fields, "Vocab");
		CString strName = AdoFldString(rs->Fields, "CodeName");

		CEMRCode code(nCodeID, strVocab, strCode, strName) ;

		

		CEMRTableCell cell(strRowData, strColData);
		cell.SetDataIDs(nRowDataID, nColDataID);

		CEMRCodeArray *pAry = GetCodes(cell, true);		

		if (pAry)
		{
			//add our struct to the array
			pAry->Add(code);

			//add the array back to the map
			insert(cell, pAry);
		}
		rs->MoveNext();
	}

}



CEMRTableCellCodeIterator CEMRTableCellCodes::begin()
{
	return m_mapCodes.begin();
}

CEMRTableCellCodeIterator CEMRTableCellCodes::end()
{
	return m_mapCodes.end();
}

CEMRTableCellCodeIterator CEMRTableCellCodes::find(CEMRCellPair pair)
{
	return m_mapCodes.find(CEMRCellPair(pair));
}

long CEMRTableCellCodes::size()
{
	return m_mapCodes.size();
}

void CEMRTableCellCodes::swap(CEMRTableCellCodes *pSwap)
{
	m_mapCodes.swap(pSwap->m_mapCodes);
}

void CEMRTableCellCodes::insert(CEMRTableCell cell, CEMRCodeArray *pAry)
{

	m_mapCodes.insert(std::make_pair(CEMRCellPair(cell.m_strRowID, cell.m_strColID), pAry));

}

BOOL CEMRTableCellCodes::SplitIntoChangedLists(CEMRTableCellCodes *pCompare, CEMRTableCellCodes *paddedMap, CEMRTableCellCodes *pchangedMap, CEMRTableCellCodes *pdeleteMap)
{
	//local possibly changed list
	CEMRTableCellCodes possiblyChanged;

	//first loop through our saved map and see if any our missing from our existing map
	{
		CEMRTableCellCodeIterator itSaved;
		for (itSaved = pCompare->begin(); itSaved != pCompare->end(); itSaved++)
		{
			CEMRCellPair p = itSaved->first;		
			CEMRTableCellCodeIterator itExistsCurrent = find(p);
			CEMRTableCell cell(p.first, p.second);

			//does it exist in our current list
			if (itExistsCurrent != end()) 
			{
				//its still there, so its not deleted, add it to our possibly changed list
				
				possiblyChanged.AddCodes(cell, itSaved->second);
			}
			else {
				//it got deleted, add it to our deleted list				
				pdeleteMap->AddCodes(cell, itSaved->second);
			}

		}
	}

	
	//now loop through the current list and then add to our added list
	{
		CEMRTableCellCodeIterator itCurrent;
		for (itCurrent = begin(); itCurrent != end(); itCurrent++)
		{
			pair<CString, CString> p = itCurrent->first;		
			CEMRTableCellCodeIterator itExistsSaved = pCompare->find(p);
			CEMRTableCell cell(p.first, p.second);

			//does it exist in our saved list
			if (itExistsSaved != pCompare->end()) 
			{
				//its there, so its not added, add it to our possibly changed list
				//only add if its not already there
				CEMRCellPair pChanged = itCurrent->first;		
				CEMRTableCellCodeIterator itExistsChanged = possiblyChanged.find(CEMRCellPair(pChanged));
				if (itExistsChanged != possiblyChanged.end()) {
					//its in our possibly changed already, so do nothing
				}
				else {
					//its not in our possibly changed alrady
					possiblyChanged.AddCodes(cell, itCurrent->second);
				}
			}
			else {
				//it got added, add it to our added list
				paddedMap->AddCodes(cell, itCurrent->second);
			}

		}
	}

	//now loop through our possibly changed and see if they did change
	{
		CEMRTableCellCodeIterator itPossSaved;
		for (itPossSaved = possiblyChanged.begin(); itPossSaved != possiblyChanged.end(); itPossSaved++)
		{
			BOOL bChanged = FALSE;

			//we know it exists in both saved and current lists, otherwise it wouldn't be here
			CEMRCellPair p = itPossSaved->first;		
			CEMRTableCellCodeIterator itCurrent = m_mapCodes.find(CEMRCellPair(p));
			

			CEMRTableCellCodeIterator itSaved = pCompare->find(p);			

			//now we need to compare the two arrays to see if they changed
			CEMRCodeArray *pAryCurrent = itCurrent->second;
			CEMRCodeArray *pArySaved = itSaved->second;

			//first check the lengths
			if (pAryCurrent->GetCount() != pArySaved->GetCount()) 
			{
				//the counts are different, so it definately changed
				bChanged = TRUE;
			}
			else {

				if (pAryCurrent->IsDifferent(pArySaved)) {
					bChanged = TRUE;
				}				
			}

			if (bChanged)
			{
				CEMRTableCell cell(itPossSaved->first.first, itPossSaved->first.second);
				//add the changed map codes to the list, since they've changed
				pchangedMap->AddCodes(cell, pAryCurrent);
			}
						
		}
	}

	if (paddedMap->size() > 0 || 
		pdeleteMap->size() > 0 || 
		pchangedMap->size() > 0)
	{
		return true;
	}
	else {
		return false;
	}

}

void CEMRTableCellCodes::operator=(CEMRTableCellCodes& src)
{
	Clear();

	CEMRTableCellCodeIterator it;	
	for (it = src.begin(); it!= src.end(); it++)
	{
		CEMRCellPair pToAdd = it->first;
		
		CEMRCodeArray *pAryFrom = it->second;
		CEMRCodeArray *pAryTo = new CEMRCodeArray();

		for(int i=0; i < pAryFrom->GetSize(); i++) 
		{
			CEMRCode codeFrom = pAryFrom->GetAt(i);
			CEMRCode codeTo = codeFrom;
			

			pAryTo->Add(codeTo);
		}

		m_mapCodes.insert(std::make_pair(CEMRCellPair(pToAdd.first, pToAdd.second), pAryTo));
	}

}

long CEMRTableCellCodes::GetCodeCount(CEMRTableCell cell)
{
	CEMRCodeArray *pAry = GetCodes(cell, FALSE);
	if (pAry)
	{
		return pAry->GetCount();
	}
	
	return 0;
}




CString CEMRTableCellCodes::GetCodeString(CEMRTableCell cell)
{
	//first, find the value in the map
	CEMRTableCellCodeIterator it = find(CEMRCellPair(cell.m_strRowID, cell.m_strColID));
	CString strReturn;

	if (it != end()) 
	{
		//it was found
		CEMRCodeArray *pAry = it->second;		
		strReturn = pAry->GetCodeString();
	}

	ASSERT(!strReturn.IsEmpty());
	return strReturn;

}

void CEMRTableCellCodes::AddCodes(CEMRTableCell cell, CEMRCodeArray* pCodes)
{

	CEMRTableCellCodeIterator it = find(CEMRCellPair(cell.m_strRowID, cell.m_strColID));
	if (it != end())
	{

		//we aren't expecting to find anything in this list
		ASSERT(FALSE);
		ThrowNxException("Error in CEMRTableCellCodes::AddCodes - attempted to add codes when they already existed");
	}

	CEMRCodeArray *pAry = new CEMRCodeArray();

	for (int i=0; i < pCodes->GetCount(); i++) {
		CEMRCode oldCode = pCodes->GetAt(i);
		CEMRCode newCode = oldCode;

		pAry->Add(newCode);
	}

	//now add the arry to the map
	insert(cell, pAry);
}

void CEMRTableCellCodes::UpdateKey(BOOL bUpdateRow, CString strOldName, CString strNewName)
{
	//we have to loop through our map, and fill a new map with all the existing information, except the new column name

	CEMRTableCellCodes tmp;

	CEMRTableCellCodeIterator it;
	for (it = begin(); it != end(); it++)
	{

		CString strExtRowName;
		CString strExtColName;
		CString strExtCompareName;
		
		strExtRowName = it->first.first;
		strExtColName = it->first.second;
		

		if (bUpdateRow) 
		{
			strExtCompareName = strExtRowName;
		}
		else {
			strExtCompareName = strExtColName;
		}

		if (strExtCompareName == strOldName)
		{
			//insert with the new name
			if (bUpdateRow) {
				CEMRTableCell cell(strNewName, strExtColName);
				tmp.AddCodes(cell, it->second);
			}
			else {
				//column
				CEMRTableCell cell(strExtRowName, strNewName);
				tmp.AddCodes(cell, it->second);
			}
		}
		else {
			//add the old values back in
			CEMRTableCell cell(strExtRowName, strExtColName);
			tmp.AddCodes(cell, it->second);
		}
	}

	//now swap the lists
	swap(&tmp);
	//we are done with tmp, so clear it
	tmp.Clear();

}

void CEMRTableCellCodes::UpdateColumnName(CString strOldName, CString strNewName)
{
	UpdateKey(false, strOldName, strNewName);
}
void CEMRTableCellCodes::UpdateRowName(CString strOldName, CString strNewName)
{
	UpdateKey(true, strOldName, strNewName);
}

void CEMRTableCellCodes::RemoveKey(BOOL bUpdateRow, CString strKeyName)
{
	//we have to loop through our map, and fill a new map with all the existing information, except the new column name

	CEMRTableCellCodes tmp;

	CEMRTableCellCodeIterator it;
	for (it = begin(); it != end(); it++)
	{

		CString strExtRowName;
		CString strExtColName;
		CString strExtCompareName;
		
		strExtRowName = it->first.first;
		strExtColName = it->first.second;
		

		if (bUpdateRow) 
		{
			strExtCompareName = strExtRowName;
		}
		else {
			strExtCompareName = strExtColName;
		}

		if (strExtCompareName == strKeyName)
		{
			//no nothing since we are removing it
		}
		else {
			//add the old values back in
			CEMRTableCell cell(strExtRowName, strExtColName);
			tmp.AddCodes(cell, it->second);
		}
	}

	//now swap the lists
	swap(&tmp);
	//we are done with tmp, so clear it
	tmp.Clear();

}

void CEMRTableCellCodes::RemoveColumn(CString strColumnName)
{
	RemoveKey(false, strColumnName);
}
void CEMRTableCellCodes::RemoveRow(CString strRowName)
{
	RemoveKey(true, strRowName);
}

BOOL CEMRTableCellCodes::DoesKeyHaveCodes(BOOL bIsRow, CString strName)
{
	CEMRTableCellCodeIterator it;
	for (it = begin(); it != end(); it++)
	{
		CString strCompareName;
		CEMRCellPair pair = it->first;
		if (bIsRow) 
		{
			strCompareName = pair.first;
		}
		else {
			strCompareName = pair.second;
		}

		if (strName == strCompareName)
		{
			//this column exists in the map, do we have any codes for it?
			CEMRCodeArray *pAry = it->second;
			if (pAry->GetCount() > 0) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CEMRTableCellCodes::DoesColumnHaveCodes(CString strColName)
{
	return DoesKeyHaveCodes(false, strColName);
}

BOOL CEMRTableCellCodes::DoesRowHaveCodes(CString strRowName)
{
	return DoesKeyHaveCodes(true, strRowName);
}