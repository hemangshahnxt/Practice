//AMACode.cpp
//This code is shared with the AMACreateFile project ($/Install/AMACreateFile).
//	Ensure all changes work there as well.

#include "stdafx.h"
#include "AMACode.h"

IMPLEMENT_SERIAL(CAMACode, CObject, AMACODE_FILE_VERSION)

CAMACode::CAMACode()
{

}

CAMACode::CAMACode(CString strCode, CString strDesc, CString strCat)
{
	m_strCode = strCode;
	m_strDesc = strDesc;

	AddCategory(strCat);
}

CAMACode::~CAMACode()
{

}

void CAMACode::Serialize(CArchive &ar)
{
	//call base class first
	CObject::Serialize(ar);

	if(ar.IsStoring()) {
		//when writing, we offset to the bad side
		CString strCode = m_strCode, strDesc = m_strDesc;
		ConvertText(strCode, ToBad);
		ConvertText(strDesc, ToBad);

		ar << strCode << strDesc;

		//for the categories, write out the count of cats, then each category
		int nSize = m_aryCats.GetSize();

		ar << nSize;

		for(int i = 0; i < nSize; i++) {
			CString str = m_aryCats.GetAt(i);
			ConvertText(str, ToBad);
			ar << str;
		}
	}
	else {
		//when reading, we offset to the good side
		CString strCode, strDesc;
		ar >> strCode >> strDesc;
		ConvertText(strCode, ToGood);
		ConvertText(strDesc, ToGood);

		m_strCode = strCode;
		m_strDesc = strDesc;

		//read out the size of the category list
		int nSize;
		ar >> nSize;

		for(int i = 0; i < nSize; i++) {
			CString str;
			ar >> str;
			ConvertText(str, ToGood);
			AddCategory(str);
		}
	}
}

//accessors
CString CAMACode::GetCode()
{
	return m_strCode;
}

CString CAMACode::GetDesc()
{
	return m_strDesc;
}

CString CAMACode::GetCatList()
{
	CString strRet;

	for(int i = 0; i < m_aryCats.GetSize(); i++) {
		strRet += m_aryCats.GetAt(i);
		strRet += ", ";
	}

	strRet.TrimRight(", ");

	return strRet;
}

//looks at all categories this item is in, and returns
//TRUE if we are in the one passed as an argument
BOOL CAMACode::InCategory(const CString strCat)
{
	for(int i = 0; i < m_aryCats.GetSize(); i++) {
		if(m_aryCats.GetAt(i).CompareNoCase(strCat) == 0)
			return TRUE;
	}

	//not found
	return FALSE;
}

//set the data
void CAMACode::AddCategory(const CString strCat)
{
	m_aryCats.Add(strCat);
}
