// (b.cardillo 2011-02-22 16:25) - PLID 40419 - Admin UI for managing Appointment Prototypes
#include "stdafx.h"
#include "ApptPrototypeUtils.h"

using namespace NXDATALIST2Lib;

void CopySet(CMap<LONG,LONG,BYTE,BYTE> &dest, const CMap<LONG,LONG,BYTE,BYTE> &src)
{
	dest.RemoveAll();
	for (POSITION pos = src.GetStartPosition(); pos != 0; ) {
		LONG key;
		BYTE value;
		src.GetNextAssoc(pos, key, value);
		dest.SetAt(key, value);
	}
}

// Compares two sets, returning true if they have all the same elements
BOOL IsSameSet(const CMap<LONG,LONG,BYTE,BYTE> &map1, const CMap<LONG,LONG,BYTE,BYTE> &map2)
{
	if (map1.GetSize() == map2.GetSize()) {
		for (POSITION pos = map1.GetStartPosition(); pos != 0; ) {
			LONG key;
			BYTE value;
			map1.GetNextAssoc(pos, key, value);
			if (!map2.PLookup(key)) {
				return FALSE;
			}
		}
		return TRUE;
	} else {
		return FALSE;
	}
}

template <class TYPE, class ARG_TYPE> BOOL FindFirstDuplicateName_Generic(const CArray<TYPE, ARG_TYPE> &ary, OUT CString &strFirstDuplicateName)
{
	for (INT_PTR i=0, nCount=ary.GetSize(); i<nCount; i++) {
		CString str1 = ary.GetAt(i).m_strName;
		str1.Trim();
		for (INT_PTR j=i+1; j<nCount; j++) {
			CString str2 = ary.GetAt(j).m_strName;
			str2.Trim();
			if (str1.CompareNoCase(str2) == 0) {
				// Found a duplicate
				strFirstDuplicateName = str1;
				return TRUE;
			}
		}
	}

	// No duplicates
	return FALSE;
}

BOOL FindFirstDuplicateName(const CArray<CApptPrototypePropertySetNamedObject, CApptPrototypePropertySetNamedObject&> &ary, OUT CString &strFirstDuplicateName)
{
	return FindFirstDuplicateName_Generic(ary, strFirstDuplicateName);
}

BOOL FindFirstDuplicateName(const CArray<CApptPrototypePropertySetNamedSet, CApptPrototypePropertySetNamedSet&> &ary, OUT CString &strFirstDuplicateName)
{
	return FindFirstDuplicateName_Generic(ary, strFirstDuplicateName);
}
