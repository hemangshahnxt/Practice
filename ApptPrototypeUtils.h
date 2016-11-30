#pragma once
// (b.cardillo 2011-02-22 16:25) - PLID 40419 - Admin UI for managing Appointment Prototypes

namespace EApptPrototypeAllowTimes
{
	enum _Enum {
		None = 0x00,
		EarlyMorning = 0x01,
		LateMorning = 0x02,
		EarlyAfternoon = 0x04,
		LateAfternoon = 0x08,
	};
}

namespace EApptPrototypeAllowDays
{
	enum _Enum {
		None = 0x00,
		Sunday = 0x01,
		Monday = 0x02,
		Tuesday = 0x04,
		Wednesday = 0x08,
		Thursday = 0x10,
		Friday = 0x20,
		Saturday = 0x40,
	};
}

void CopySet(CMap<LONG,LONG,BYTE,BYTE> &dest, const CMap<LONG,LONG,BYTE,BYTE> &src);

// Compares two sets, returning true if they have all the same elements
BOOL IsSameSet(const CMap<LONG,LONG,BYTE,BYTE> &map1, const CMap<LONG,LONG,BYTE,BYTE> &map2);

class CApptPrototypePropertySetNamedObject
{
public:
	CApptPrototypePropertySetNamedObject() : m_nID(-1), m_strName(_T("")) { }
	CApptPrototypePropertySetNamedObject(LONG nID, const CString strName) : m_nID(nID), m_strName(strName) { }

public:
	LONG m_nID;
	CString m_strName;
};


class CApptPrototypePropertySetNamedSet : public CApptPrototypePropertySetNamedObject
{
public:
	CApptPrototypePropertySetNamedSet() : CApptPrototypePropertySetNamedObject() { }
	CApptPrototypePropertySetNamedSet(LONG nID, const CString strName) : CApptPrototypePropertySetNamedObject(nID, strName) { }
	CApptPrototypePropertySetNamedSet(const CApptPrototypePropertySetNamedSet &src) 
		: CApptPrototypePropertySetNamedObject(src.m_nID, src.m_strName) 
	{
		CopySet(m_mapdwDetailIDs, src.m_mapdwDetailIDs);
	}
	CApptPrototypePropertySetNamedSet &operator =(const CApptPrototypePropertySetNamedSet &src) {
		this->m_strName = src.m_strName;
		this->m_nID = src.m_nID;
		CopySet(this->m_mapdwDetailIDs, src.m_mapdwDetailIDs);
		return *this;
	}

public:
	CMap<LONG,LONG,BYTE,BYTE> m_mapdwDetailIDs;
};

// Searches an array of named objects for more than one entry with the same name ignoring case
BOOL FindFirstDuplicateName(const CArray<CApptPrototypePropertySetNamedObject, CApptPrototypePropertySetNamedObject&> &ary, OUT CString &strFirstDuplicateName);
BOOL FindFirstDuplicateName(const CArray<CApptPrototypePropertySetNamedSet, CApptPrototypePropertySetNamedSet&> &ary, OUT CString &strFirstDuplicateName);
