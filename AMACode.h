//AMACode.h
//This code is shared with the AMACreateFile project ($/Install/AMACreateFile).
//	Ensure all changes work there as well.
#ifndef AMACODE_H
#define AMACODE_H

#pragma once

#define AMACODE_FILE_VERSION 1		//Version used in the serialized object

#define ToGood 0x123
#define ToBad 0x456

//modified from license code to work with CStrings
static void ConvertText(CString &str, DWORD direction = ToGood)
{
	ASSERT(direction == ToGood || direction == ToBad);

	int len = str.GetLength(),
		offset;

	if (direction == ToGood) 
		offset = -100;
	else offset = 100;

	for (int i = 0; i < len; i++)
		str.SetAt(i, str.GetAt(i) + offset);
}

class CAMACode : public CObject {
public:
	DECLARE_SERIAL(CAMACode);

	CAMACode();
	CAMACode(CString strCode, CString strDesc, CString strCat);
	~CAMACode();

	void Serialize(CArchive &ar);

	//accessors
	CString GetCode();
	CString GetDesc();
	CString GetCatList();
	BOOL InCategory(const CString strCat);

	//set the data
	void AddCategory(const CString strCat);

protected:
	CString m_strCode;
	CString m_strDesc;

	CArray<CString, CString> m_aryCats;
};

#endif	//AMACODE_H
