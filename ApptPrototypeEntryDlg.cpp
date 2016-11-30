// ApptPrototypeEntryDlg.cpp : implementation file
// (b.cardillo 2011-02-22 16:25) - PLID 40419 - Admin UI for managing Appointment Prototypes
//

#include "stdafx.h"
#include "Practice.h"
#include "ApptPrototypeEntryDlg.h"
#include "ApptPrototypeUtils.h"
#include "ApptPrototypePropertySetAvailabilityDlg.h"
#include "ApptPrototypePropertySetNamedSetDlg.h"
#include "MultiSelectDlg.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

namespace EPropertySetListColumns
{
	enum _Enum {
		ID,
		Name,
		Duration,
		Available,
		Types,
		PurposeSets,
		ResourceSets,
		Locations,
	};
};

// CApptPrototypeEntryDlg dialog

IMPLEMENT_DYNAMIC(CApptPrototypeEntryDlg, CNxDialog)

CApptPrototypeEntryDlg::CApptPrototypeEntryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CApptPrototypeEntryDlg::IDD, pParent)
{
	m_readyToDoModal = false;
	m_appointmentPrototypeID = -1;
	m_parystrOtherPrototypeNames = NULL;
}

CApptPrototypeEntryDlg::~CApptPrototypeEntryDlg()
{
}

void CApptPrototypeEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADD_BTN, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE_BTN, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CApptPrototypeEntryDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CApptPrototypeEntryDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_ADD_BTN, &CApptPrototypeEntryDlg::OnBnClickedAddBtn)
	ON_BN_CLICKED(IDC_DELETE_BTN, &CApptPrototypeEntryDlg::OnBnClickedDeleteBtn)
END_MESSAGE_MAP()


// CApptPrototypeEntryDlg message handlers

INT_PTR CApptPrototypeEntryDlg::DoModal()
{
	if (m_readyToDoModal) {
		// Ok, we can do modal, go for it but first clear the boolean so that we can't go modal again 
		// without being initialized again
		m_readyToDoModal = false;
		return CNxDialog::DoModal();
	} else {
		ThrowNxException(_T("Dialog cannot open because it has not been given initialization properties."));
	}
}

INT_PTR CApptPrototypeEntryDlg::DoModal(INT appointmentPrototypeID, const CStringArray &arystrOtherPrototypeNames)
{
	m_appointmentPrototypeID = appointmentPrototypeID;
	m_parystrOtherPrototypeNames = &arystrOtherPrototypeNames;
	m_nextNegativeID = -1;
	m_nNextNegativeNamedSetID = -1;
	m_readyToDoModal = true;
	return DoModal();
}

typedef CApptPrototypePropertySetNamedObject CApptPrototypePropertySetAptType;
typedef CApptPrototypePropertySetNamedObject CApptPrototypePropertySetLocation;

typedef CApptPrototypePropertySetNamedSet CApptPrototypePropertySetAptPurposeSet;
typedef CApptPrototypePropertySetNamedSet CApptPrototypePropertySetResourceSet;

// Since aptpurposesets and resourcesets are stored in identical structures, the loading and 
// saving code (among other code) treats them identically.  So we pass this enum around in 
// order to identify which kind of namedset we're dealing with.
namespace ENamedSetOfDetailsType {
	enum _Enum {
		AptPurposeSet,
		ResourceSet,
	};

	CString GetTableNamePrefix(_Enum ensodtSetType)
	{
		switch (ensodtSetType) {
		case ENamedSetOfDetailsType::AptPurposeSet: return _T("AptPurpose");
		case ENamedSetOfDetailsType::ResourceSet: return _T("Resource");
		default: ThrowNxException(_T("Unknown named set type '%li'."), ensodtSetType);
		}
	}
};

// Since apttypes and locations are stored in identical structures, the loading and saving 
// code (among other code) treats them identically.  So we pass this enum around in order to 
// identify which kind of object we're dealing with.
namespace ENamedObjectsType {
	enum _Enum {
		AptType,
		Location,
	};
	
	CString GetTableNamePrefix(_Enum enotType)
	{
		switch (enotType) {
		case ENamedObjectsType::AptType: return _T("AptType");
		case ENamedObjectsType::Location: return _T("Location");
		default: ThrowNxException(_T("Unknown named object type '%li'."), enotType);
		}
	}
};

// Provides a generic way of iterating through complex records.  Meant to be specialized in a 
// derived class.  This isn't unique to appointment prototypes, it would be useful for reading 
// any complex records.  For example, say you had the following records:
//   ID    DetailID  DetailName
//    1           1  "one"
//    1           2  "two"
//    2           3  "three"
// and want to load up two objects (1 and 2) where the first has 2 entries in an array ("one" 
// and "two") and the second has 1 entry in its array ("three").  You could derive a class 
// from this class that overrides Prepare() to read the ID into a member variable, then you'd 
// override ReadSubRecord() to get the DetailID and DetailName out, add them to your active 
// parent object, and movenext.  You'd override IsStillSubrecord() to check if ID has changed 
// and if you wanted to you could even override Finish() to add the parent object into some 
// external map.  This is just a basic example of how this class could be used.
class CRecordsReader
{
public:
	CRecordsReader(_Recordset *lprs)
		: m_prs(lprs)
	{
	}

public:
	// Called once, on the first record.  Allows you to set up your parent-specific members.
	virtual BOOL Prepare()
	{
		return TRUE;
	}

	// Called for each record until IsStillSubrecord() returns false.  NOT called on that 
	// record.  It is this function's responsibility to call MoveNext().  This doesn't mean it 
	// literally has to call it directly, it may call some other class that reads its own 
	// subrecords, in which case it would then be that class's responsibility to movenext as 
	// necessary.  In the end though, when leaving this function, the current record should be 
	// beyond the one it was on when entering this function.
	virtual void ReadSubRecord() = 0;

	// Called after ReadSubRecord() if not eof.  Determines if the new current record is still 
	// under the same parent.  Usually checks some ID against the original ID stored by the 
	// Prepare() function.
	virtual BOOL IsStillSubrecord() = 0;

	// Called after the last subrecord is processed.  Not often overridden, but it's available 
	// if you need to do something with the object you've read after it's finished being read.
	virtual void Finish()
	{
	}

	// This holds the standard reading mechanism.  Usually not overridden, since it's the core 
	// logic of this entire class.
	virtual void Read()
	{
		if (!m_prs->eof) {
			// Define "locals" and read the "parent" info from the first record
			if (Prepare()) {
				// Read each record until we find a record that's not a subrecord of the "parent"
				while (true) {
					// Handle the current subrecord
					ReadSubRecord();

					// Go to the next record, and if that hits the end or moves us off the current "parent", then we're done
					if (m_prs->eof || !IsStillSubrecord()) {
						break;
					}
				}
				Finish();
			} else {
				m_prs->MoveNext();
			}
		}
	}

protected:
	_RecordsetPtr m_prs;
};

// Holds all information about a propertyset (a propertyset is the core storage for appt prototypes; a prototype has any number of propertysets, each of which defines one set of appointment properties)
class CApptPrototypePropertySet
{
public:
	CApptPrototypePropertySet()
	{
		m_nID = -1;
		m_strName.Empty();
		m_nApptPrototypeID = -1;
		m_nAppointmentDuration = 0;
		m_nAvailTimes = EApptPrototypeAllowTimes::None;
		m_nAvailDays = EApptPrototypeAllowDays::None;
		m_aryAptTypes.RemoveAll();
		m_aryAptPurposeSets.RemoveAll();
		m_aryResourceSets.RemoveAll();
		m_aryLocations.RemoveAll();
	}

	CApptPrototypePropertySet(const CApptPrototypePropertySet &src)
	{
		m_nID = src.m_nID;
		m_strName = src.m_strName;
		m_nApptPrototypeID = src.m_nApptPrototypeID;
		m_nAppointmentDuration = src.m_nAppointmentDuration;
		m_nAvailTimes = src.m_nAvailTimes;
		m_nAvailDays = src.m_nAvailDays;
		m_aryAptTypes.Copy(src.m_aryAptTypes);
		m_aryAptPurposeSets.Copy(src.m_aryAptPurposeSets);
		m_aryResourceSets.Copy(src.m_aryResourceSets);
		m_aryLocations.Copy(src.m_aryLocations);
	}

public:
	LONG m_nID;
	CString m_strName;
	LONG m_nApptPrototypeID;
	LONG m_nAppointmentDuration;
	LONG m_nAvailTimes;
	LONG m_nAvailDays;
	CArray<CApptPrototypePropertySetAptType, CApptPrototypePropertySetAptType &> m_aryAptTypes;
	CArray<CApptPrototypePropertySetAptPurposeSet, CApptPrototypePropertySetAptPurposeSet &> m_aryAptPurposeSets;
	CArray<CApptPrototypePropertySetResourceSet, CApptPrototypePropertySetResourceSet &> m_aryResourceSets;
	CArray<CApptPrototypePropertySetLocation, CApptPrototypePropertySetLocation &> m_aryLocations;

public:
	// Access either m_aryAptPurposeSets or m_aryResourceSets depending on ensodtSetType
	CArray<CApptPrototypePropertySetNamedSet, CApptPrototypePropertySetNamedSet &> &GetNamedSets(ENamedSetOfDetailsType::_Enum ensodtSetType)
	{
		switch (ensodtSetType) {
		case ENamedSetOfDetailsType::AptPurposeSet:
			return m_aryAptPurposeSets;
		case ENamedSetOfDetailsType::ResourceSet:
			return m_aryResourceSets;
		default:
			ThrowNxException(_T("Unknown named set type '%li'."), ensodtSetType);
		}
	}

	// Access either m_aryAptTypes or m_aryLocations depending on enotType
	CArray<CApptPrototypePropertySetNamedObject, CApptPrototypePropertySetNamedObject &> &GetNamedObjects(ENamedObjectsType::_Enum enotType)
	{
		switch (enotType) {
		case ENamedObjectsType::AptType:
			return m_aryAptTypes;
		case ENamedObjectsType::Location:
			return m_aryLocations;
		default:
			ThrowNxException(_T("Unknown named object type '%li'."), enotType);
		}
	}

public:
	// Functions for retrieving the contents of this class in user-friendly text
	CString CalcAptTypesText() { return CalcNamedObjectsText(m_aryAptTypes); }
	CString CalcLocationsText() { return CalcNamedObjectsText(m_aryLocations); }
	CString CalcAptPurposeSetsText() { return CalcNamedObjectsText(m_aryAptPurposeSets); }
	CString CalcResourceSetsText() { return CalcNamedObjectsText(m_aryResourceSets); }

	// Returns a user-friendly string describing the current state of the day/time availability for this propertyset
	CString CalcAvailabilityText()
	{
		CString strAvailTimes;
		{
			// Morning
			if (m_nAvailTimes & EApptPrototypeAllowTimes::EarlyMorning) {
				strAvailTimes += ((m_nAvailTimes & EApptPrototypeAllowTimes::LateMorning) ? _T("AM/") : _T("AM1/"));
			} else {
				strAvailTimes += ((m_nAvailTimes & EApptPrototypeAllowTimes::LateMorning) ? _T("AM2/") : _T(""));
			}
			// Afternoon
			if (m_nAvailTimes & EApptPrototypeAllowTimes::EarlyAfternoon) {
				strAvailTimes += ((m_nAvailTimes & EApptPrototypeAllowTimes::LateAfternoon) ? _T("PM/") : _T("PM1/"));
			} else {
				strAvailTimes += ((m_nAvailTimes & EApptPrototypeAllowTimes::LateAfternoon) ? _T("PM2/") : _T(""));
			}
			strAvailTimes.TrimRight('/');
		}
		CString strAvailDays;
		{
			strAvailDays += ((m_nAvailDays & EApptPrototypeAllowDays::Sunday) ? _T("Su/") : _T(""));
			strAvailDays += ((m_nAvailDays & EApptPrototypeAllowDays::Monday) ? _T("M/") : _T(""));
			strAvailDays += ((m_nAvailDays & EApptPrototypeAllowDays::Tuesday) ? _T("Tu/") : _T(""));
			strAvailDays += ((m_nAvailDays & EApptPrototypeAllowDays::Wednesday) ? _T("W/") : _T(""));
			strAvailDays += ((m_nAvailDays & EApptPrototypeAllowDays::Thursday) ? _T("Th/") : _T(""));
			strAvailDays += ((m_nAvailDays & EApptPrototypeAllowDays::Friday) ? _T("F/") : _T(""));
			strAvailDays += ((m_nAvailDays & EApptPrototypeAllowDays::Saturday) ? _T("Sa/") : _T(""));
			strAvailDays.TrimRight('/');
		}
		return strAvailTimes.IsEmpty() ? strAvailDays : (strAvailDays.IsEmpty() ? strAvailTimes : (strAvailDays + _T(" - ") + strAvailTimes));
	}

protected:
	// Helper function joins the names of the specified objects in comma-delimited list in alphabetical 
	// order, returning "{None}" if the list is empty.  Used by many of the Calc*Text() functions.
	template <class TYPE> static CString CalcNamedObjectsText(const CArray<TYPE, TYPE &> &objects)
	{
		int count = objects.GetSize();
		if (count > 0) {
			// Copy the names into a sorted array
			int nExpectedLength;
			CStringSortedArrayNoCase sorted;
			{
				sorted.SetSize(0, count);
				nExpectedLength = 0;
				for (int i=0; i<count; i++) {
					const CString &str = objects.GetAt(i).m_strName;
					nExpectedLength += str.GetLength() + 2;
					sorted.Insert(str);
				}
			}
			// Then generate the return string based on the sorted array
			CString ans;
			ans.Preallocate(nExpectedLength);
			int countAllButLast = sorted.GetSize() - 1;
			for (int i=0; i<countAllButLast; i++) {
				ans += sorted.GetAt(i) + _T(", ");
			}
			ans += sorted.GetAt(countAllButLast);
			return ans;
		} else {
			return _T("{None}");
		}
	}

public:
	// Handy function to read all namedsets for all propertysets from the given recordset.  For 
	// each one it looks the propertyset up in the given map and adds the namedsets to that 
	// propertyset.  It is critical that the recordset be populated exactly right, including the 
	// precise sort order.
	static void ReadNamedSets(_Recordset *lprs, CMap<LONG, LONG, CApptPrototypePropertySet *, CApptPrototypePropertySet *> &map, ENamedSetOfDetailsType::_Enum ensodtSetType)
	{
		// Simple, just read until the end!
		_RecordsetPtr prs = lprs;
		while (!prs->eof) {
			CNamedSetsReader_PropertySet reader(prs, map, ensodtSetType);
			reader.Read();
		}
	}


// Helper classes for reading "named set" (purposeset or resourceset) records from data
private:
	/*
	 - The CNamedSetsReader_PropertySet class reads each propertyset from the recordset and for each one calls the CNamedSetsReader_NamedSet class to read its namedsets.
	 - The CNamedSetsReader_NamedSet class reads each namedset for the current propertyset and for each one calls the CNamedSetsReader_Detail class to read its details.
	 - The CNamedSetsReader_Detail class reads each detail for the current namedset; since that leaves nothing left in the record to read, it's the only class that calls MoveNext().
	*/

	// Reads each propertyset from lprs, calls CNamedSetsReader_NamedSet class for the namedsets within the propertyset
	class CNamedSetsReader_PropertySet : public CRecordsReader
	{
	public:
		CNamedSetsReader_PropertySet(_Recordset *lprs, CMap<LONG, LONG, CApptPrototypePropertySet *, CApptPrototypePropertySet *> &map, ENamedSetOfDetailsType::_Enum ensodtSetType)
			: CRecordsReader(lprs), m_map(map), m_ensodtSetType(ensodtSetType) { }
	protected:
		CMap<LONG, LONG, CApptPrototypePropertySet *, CApptPrototypePropertySet *> &m_map;
		ENamedSetOfDetailsType::_Enum m_ensodtSetType;
	private:
		struct Locals {
			FieldPtr fldPropSetID;
			CApptPrototypePropertySet *pPropSet;
		} locals;
	public:
		virtual BOOL Prepare()
		{
			locals.fldPropSetID = m_prs->GetFields()->GetItem(_T("ApptPrototypePropertySetID"));
			locals.pPropSet = m_map[AdoFldLong(locals.fldPropSetID)];
			return TRUE;
		}
		virtual void ReadSubRecord()
		{
			CNamedSetsReader_NamedSet reader(m_prs, locals.pPropSet, m_ensodtSetType);
			reader.Read();
		}
		virtual BOOL IsStillSubrecord()
		{
			if (AdoFldLong(locals.fldPropSetID) == locals.pPropSet->m_nID) {
				return TRUE;
			} else {
				return FALSE;
			}
		}
	};
	
	// Reads each namedset for the current propertyset, calls CNamedSetsReader_Detail class for the details within the namedset
	class CNamedSetsReader_NamedSet : public CRecordsReader
	{
	public:
		CNamedSetsReader_NamedSet(_Recordset *lprs, CApptPrototypePropertySet *pPropSet, ENamedSetOfDetailsType::_Enum ensodtSetType)
			: CRecordsReader(lprs), m_pPropSet(pPropSet), m_ensodtSetType(ensodtSetType) { }
	protected:
		CApptPrototypePropertySet *m_pPropSet;
		ENamedSetOfDetailsType::_Enum m_ensodtSetType;
	private:
		struct Locals {
			FieldPtr fldPropSetID, fldNamedSetID, fldName;
			LONG nCurrentPropertySetID;
		} locals;
	public:
		virtual BOOL Prepare()
		{
			FieldsPtr flds = m_prs->GetFields();
			locals.fldNamedSetID = flds->GetItem(AsVariant(_T("ApptPrototypePropertySet") + ENamedSetOfDetailsType::GetTableNamePrefix(m_ensodtSetType) + _T("SetID")));
			_variant_t varNamedSetID = locals.fldNamedSetID->GetValue();
			if (varNamedSetID.vt != VT_NULL) {
				locals.fldPropSetID = flds->GetItem(_T("ApptPrototypePropertySetID"));
				locals.nCurrentPropertySetID = AdoFldLong(locals.fldPropSetID);
				locals.fldName = flds->GetItem(_T("Name"));
				return TRUE;
			} else {
				return FALSE;
			}
		}
		virtual void ReadSubRecord()
		{
			CArray<CApptPrototypePropertySetNamedSet, CApptPrototypePropertySetNamedSet &> &sets = m_pPropSet->GetNamedSets(m_ensodtSetType);
			INT_PTR index = sets.Add(CApptPrototypePropertySetNamedSet());
			CApptPrototypePropertySetNamedSet &set = sets.GetAt(index);
			set.m_nID = AdoFldLong(locals.fldNamedSetID);
			set.m_strName = AdoFldString(locals.fldName);
			CNamedSetsReader_Detail reader(m_prs, &set, m_ensodtSetType);
			reader.Read();
		}
		virtual BOOL IsStillSubrecord()
		{
			return (AdoFldLong(locals.fldPropSetID) == locals.nCurrentPropertySetID);
		}
	};

	// Reads each detail for the current namedset, it'll be our job to complete the reading of each record, and therefore we're the only one who calls MoveNext()
	class CNamedSetsReader_Detail : public CRecordsReader
	{
	public:
		CNamedSetsReader_Detail(_Recordset *lprs, CApptPrototypePropertySetNamedSet *pSet, ENamedSetOfDetailsType::_Enum ensodtSetType) 
			: CRecordsReader(lprs), m_pSet(pSet), m_ensodtSetType(ensodtSetType) { }
	protected:
		CApptPrototypePropertySetNamedSet *m_pSet;
		ENamedSetOfDetailsType::_Enum m_ensodtSetType;
	private:
		struct Locals {
			FieldPtr fldDetailID, fldPropSetID, fldNamedSetID;
			LONG nCurrentPropertySetID;
		} locals;
	public:
		virtual BOOL Prepare()
		{
			FieldsPtr flds = m_prs->GetFields();
			locals.fldDetailID = flds->GetItem(AsVariant(ENamedSetOfDetailsType::GetTableNamePrefix(m_ensodtSetType) + _T("ID"))); // Technically we should have a ENamedSetOfDetailsType::GetDetailFieldName(), but it happens to work out that for all current uses the table name prefix works out to be the same as the detail ID field prefix, so we just use it directly.
			_variant_t varDetailID = locals.fldDetailID->GetValue();
			if (varDetailID.vt != VT_NULL) {
				locals.fldPropSetID = flds->GetItem(_T("ApptPrototypePropertySetID"));
				locals.nCurrentPropertySetID = AdoFldLong(locals.fldPropSetID);
				locals.fldNamedSetID = flds->GetItem(AsVariant(_T("ApptPrototypePropertySet") + ENamedSetOfDetailsType::GetTableNamePrefix(m_ensodtSetType) + _T("SetID")));
				return TRUE;
			} else {
				return FALSE;
			}
		}
		virtual void ReadSubRecord()
		{
			m_pSet->m_mapdwDetailIDs.SetAt(AdoFldLong(locals.fldDetailID), 0);
			m_prs->MoveNext();
		}
		virtual BOOL IsStillSubrecord()
		{
			return (AdoFldLong(locals.fldPropSetID) == locals.nCurrentPropertySetID && AdoFldLong(locals.fldNamedSetID) == m_pSet->m_nID);
		}
	};
};

CApptPrototypePropertySetMap::CApptPrototypePropertySetMap()
{
}

CApptPrototypePropertySetMap::~CApptPrototypePropertySetMap()
{
	this->RemoveAll();
}

void CApptPrototypePropertySetMap::RemoveAll()
{
	// Free the memory before emptying
	for (POSITION pos = this->GetStartPosition(); pos != 0; ) {
		LONG nPropSetID;
		CApptPrototypePropertySet *pPropSet;
		this->GetNextAssoc(pos, nPropSetID, pPropSet);
		delete pPropSet;
	}
	CMap<LONG, LONG, class CApptPrototypePropertySet *, class CApptPrototypePropertySet *>::RemoveAll();
}

void CApptPrototypePropertySetMap::Copy(const CApptPrototypePropertySetMap &src)
{
	// Clear us
	this->RemoveAll();
	// Iterate through each entry in the source and add a copy to us at the same key in the map
	for (POSITION pos = src.GetStartPosition(); pos != 0; ) {
		LONG nPropSetID;
		CApptPrototypePropertySet *pPropSet;
		src.GetNextAssoc(pos, nPropSetID, pPropSet);
		this->SetAt(nPropSetID, new CApptPrototypePropertySet(*pPropSet));
	}
}


BOOL CApptPrototypeEntryDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		// Set button styles
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// Limit the length allowed in the textboxes
		SendDlgItemMessage(IDC_NAME_EDIT, EM_SETLIMITTEXT, 200);
		SendDlgItemMessage(IDC_DESCRIPTION_EDIT, EM_SETLIMITTEXT, 2000);

		_RecordsetPtr prs = CreateParamRecordset(GetRemoteData(), 
			_T("DECLARE @prototypeID INT SET @prototypeID = {INT} \r\n")
			// 01: Get the prototype info
			_T("SELECT Name, Description FROM ApptPrototypeT WHERE ID = @prototypeID \r\n")
			// 02: Get the prototype's propertsets' info
			_T("SELECT ID, Name, ApptPrototypeID, AppointmentDuration, (0 \r\n")
			_T(" + CASE WHEN AllowTimeEarlyMorning = 1 THEN ") + AsString(EApptPrototypeAllowTimes::EarlyMorning) + _T(" ELSE 0 END \r\n")
			_T(" + CASE WHEN AllowTimeLateMorning = 1 THEN ") + AsString(EApptPrototypeAllowTimes::LateMorning) + _T(" ELSE 0 END \r\n")
			_T(" + CASE WHEN AllowTimeEarlyAfternoon = 1 THEN ") + AsString(EApptPrototypeAllowTimes::EarlyAfternoon) + _T(" ELSE 0 END \r\n")
			_T(" + CASE WHEN AllowTimeLateAfternoon = 1 THEN ") + AsString(EApptPrototypeAllowTimes::LateAfternoon) + _T(" ELSE 0 END \r\n")
			_T(" ) AS AvailTimes, (0 \r\n")
			_T(" + CASE WHEN AllowDaySunday = 1 THEN ") + AsString(EApptPrototypeAllowDays::Sunday) + _T(" ELSE 0 END \r\n")
			_T(" + CASE WHEN AllowDayMonday = 1 THEN ") + AsString(EApptPrototypeAllowDays::Monday) + _T(" ELSE 0 END \r\n")
			_T(" + CASE WHEN AllowDayTuesday = 1 THEN ") + AsString(EApptPrototypeAllowDays::Tuesday) + _T(" ELSE 0 END \r\n")
			_T(" + CASE WHEN AllowDayWednesday = 1 THEN ") + AsString(EApptPrototypeAllowDays::Wednesday) + _T(" ELSE 0 END \r\n")
			_T(" + CASE WHEN AllowDayThursday = 1 THEN ") + AsString(EApptPrototypeAllowDays::Thursday) + _T(" ELSE 0 END \r\n")
			_T(" + CASE WHEN AllowDayFriday = 1 THEN ") + AsString(EApptPrototypeAllowDays::Friday) + _T(" ELSE 0 END \r\n")
			_T(" + CASE WHEN AllowDaySaturday = 1 THEN ") + AsString(EApptPrototypeAllowDays::Saturday) + _T(" ELSE 0 END \r\n")
			_T(" ) AS AvailDays \r\n")
			_T("FROM ApptPrototypePropertySetT \r\n")
			_T("WHERE ApptPrototypeID = @prototypeID \r\n")
			_T("ORDER BY Name \r\n")
			// 03: Get the appointment types for each propertyset
			_T("SELECT PS.ID AS ApptPrototypePropertySetID, PSAT.AptTypeID, (SELECT A.Name FROM AptTypeT A WHERE A.ID = PSAT.AptTypeID) AS Name \r\n")
			_T("FROM ApptPrototypePropertySetAptTypeT PSAT RIGHT JOIN ApptPrototypePropertySetT PS ON PSAT.ApptPrototypePropertySetID = PS.ID \r\n")
			_T("WHERE PS.ApptPrototypeID = @prototypeID \r\n")
			_T("ORDER BY PSAT.ApptPrototypePropertySetID \r\n")
			// 04: Get the appointment purposesets for each propertyset
			_T("SELECT PS.ID AS ApptPrototypePropertySetID, PSAPS.ID AS ApptPrototypePropertySetAptPurposeSetID, PSAPS.Name, PSAPSD.AptPurposeID \r\n")
			_T("FROM ApptPrototypePropertySetT PS \r\n")
			_T("LEFT JOIN ApptPrototypePropertySetAptPurposeSetT PSAPS ON PS.ID = PSAPS.ApptPrototypePropertySetID \r\n")
			_T("LEFT JOIN ApptPrototypePropertySetAptPurposeSetDetailT PSAPSD ON PSAPS.ID = PSAPSD.ApptPrototypePropertySetAptPurposeSetID \r\n")
			_T("WHERE PS.ApptPrototypeID = @prototypeID \r\n")
			_T("ORDER BY PS.ID, PSAPS.ID \r\n")
			// 05: Get the resourceets for each propertyset
			_T("SELECT PS.ID AS ApptPrototypePropertySetID, PSRS.ID AS ApptPrototypePropertySetResourceSetID, PSRS.Name, PSRSD.ResourceID \r\n")
			_T("FROM ApptPrototypePropertySetT PS \r\n")
			_T("LEFT JOIN ApptPrototypePropertySetResourceSetT PSRS ON PS.ID = PSRS.ApptPrototypePropertySetID \r\n")
			_T("LEFT JOIN ApptPrototypePropertySetResourceSetDetailT PSRSD ON PSRS.ID = PSRSD.ApptPrototypePropertySetResourceSetID \r\n")
			_T("WHERE PS.ApptPrototypeID = @prototypeID \r\n")
			_T("ORDER BY PS.ID, PSRS.ID \r\n")
			// 06: Get the locations for each propertyset
			_T("SELECT PS.ID AS ApptPrototypePropertySetID, PSL.LocationID, (SELECT L.Name FROM LocationsT L WHERE L.ID = PSL.LocationID) AS Name \r\n")
			_T("FROM ApptPrototypePropertySetLocationT PSL RIGHT JOIN ApptPrototypePropertySetT PS ON PSL.ApptPrototypePropertySetID = PS.ID \r\n")
			_T("WHERE PS.ApptPrototypeID = @prototypeID \r\n")
			_T("ORDER BY PSL.ApptPrototypePropertySetID \r\n")
			, m_appointmentPrototypeID);

		// 01: Get the prototype info
		if (!prs->eof) {
			FieldsPtr flds = prs->GetFields();
			m_strOriginalName = AdoFldString(flds, _T("Name"));
			SetDlgItemText(IDC_NAME_EDIT, m_strOriginalName);
			m_strOriginalDescription = AdoFldString(flds, _T("Description"));
			SetDlgItemText(IDC_DESCRIPTION_EDIT, m_strOriginalDescription);
			prs->MoveNext();
			if (!prs->eof) {
				ThrowNxException(_T("Encountered more than one prototype record for prototype ID '%li'."), m_appointmentPrototypeID);
			}
		}
		prs = prs->NextRecordset(NULL);

		// 02: Get the prototype's propertsets' info
		m_mapOrigPropSets.RemoveAll();
		if (!prs->eof) {
			FieldsPtr flds = prs->GetFields();
			FieldPtr fldID = flds->GetItem(_T("ID"));
			FieldPtr fldName = flds->GetItem(_T("Name"));
			FieldPtr fldDuration = flds->GetItem(_T("AppointmentDuration"));
			FieldPtr fldAvailDays = flds->GetItem(_T("AvailDays"));
			FieldPtr fldAvailTimes = flds->GetItem(_T("AvailTimes"));
			while (!prs->eof) {
				CApptPrototypePropertySet *pPropSet = new CApptPrototypePropertySet();
				pPropSet->m_nID = AdoFldLong(fldID);
				pPropSet->m_strName = AdoFldString(fldName);
				pPropSet->m_nAppointmentDuration = AdoFldLong(fldDuration);
				pPropSet->m_nAvailTimes = AdoFldLong(fldAvailTimes);
				pPropSet->m_nAvailDays = AdoFldLong(fldAvailDays);
				m_mapOrigPropSets.SetAt(pPropSet->m_nID, pPropSet);
				prs->MoveNext();
			}
		}
		prs = prs->NextRecordset(NULL);

		// 03: Get the appointment types for each propertyset
		if (!prs->eof) {
			FieldsPtr flds = prs->GetFields();
			FieldPtr fldPropSetID = flds->GetItem(_T("ApptPrototypePropertySetID"));
			FieldPtr fldAptTypeID = flds->GetItem(_T("AptTypeID"));
			FieldPtr fldName = flds->GetItem(_T("Name"));
			LONG nLastApptPrototypePropertySetID = -1;
			CApptPrototypePropertySet *pCurPropSet = NULL;
			while (!prs->eof) {
				LONG nCurApptPrototypePropertySetID = AdoFldLong(fldPropSetID);
				if (nCurApptPrototypePropertySetID != nLastApptPrototypePropertySetID) {
					pCurPropSet = m_mapOrigPropSets[nCurApptPrototypePropertySetID];
				}
				_variant_t varAptTypeID = fldAptTypeID->GetValue();
				if (varAptTypeID.vt != VT_NULL) {
					pCurPropSet->m_aryAptTypes.Add(CApptPrototypePropertySetAptType(VarLong(varAptTypeID), AdoFldString(fldName)));
				}
				nLastApptPrototypePropertySetID = nCurApptPrototypePropertySetID;
				prs->MoveNext();
			}
		}
		prs = prs->NextRecordset(NULL);

		// 04: Get the appointment purposesets for each propertyset
		CApptPrototypePropertySet::ReadNamedSets(prs, m_mapOrigPropSets, ENamedSetOfDetailsType::AptPurposeSet);
		prs = prs->NextRecordset(NULL);

		// 05: Get the resourcesets for each propertyset
		CApptPrototypePropertySet::ReadNamedSets(prs, m_mapOrigPropSets, ENamedSetOfDetailsType::ResourceSet);
		prs = prs->NextRecordset(NULL);

		// 06: Get the locations for each propertyset
		if (!prs->eof) {
			FieldsPtr flds = prs->GetFields();
			FieldPtr fldPropSetID = flds->GetItem(_T("ApptPrototypePropertySetID"));
			FieldPtr fldLocationID = flds->GetItem(_T("LocationID"));
			FieldPtr fldName = flds->GetItem(_T("Name"));
			LONG nLastApptPrototypePropertySetID = -1;
			CApptPrototypePropertySet *pCurPropSet = NULL;
			while (!prs->eof) {
				LONG nCurApptPrototypePropertySetID = AdoFldLong(fldPropSetID);
				if (nCurApptPrototypePropertySetID != nLastApptPrototypePropertySetID) {
					pCurPropSet = m_mapOrigPropSets[nCurApptPrototypePropertySetID];
				}
				_variant_t varLocationID = fldLocationID->GetValue();
				if (varLocationID.vt != VT_NULL) {
					pCurPropSet->m_aryLocations.Add(CApptPrototypePropertySetLocation(VarLong(varLocationID), AdoFldString(fldName)));
				}
				nLastApptPrototypePropertySetID = nCurApptPrototypePropertySetID;
				prs->MoveNext();
			}
		}
		prs = prs->NextRecordset(NULL);



		// Finished loading from data, reflect it on screen and copy into our current map
		{
			// And load onto the dialog
			{
				_DNxDataListPtr pdl = GetDlgItem(IDC_APPTPROTOTYPE_PROPERTYSETS_LIST)->GetControlUnknown();
				pdl->Clear();
				for (POSITION pos = m_mapOrigPropSets.GetStartPosition(); pos != 0; ) {
					LONG nPropSetID;
					CApptPrototypePropertySet *pPropSet;
					m_mapOrigPropSets.GetNextAssoc(pos, nPropSetID, pPropSet);

					NXDATALIST2Lib::IRowSettingsPtr pRow = pdl->GetNewRow();
					pRow->PutValue(EPropertySetListColumns::ID, pPropSet->m_nID);
					pRow->PutValue(EPropertySetListColumns::Name, AsVariant(pPropSet->m_strName));
					pRow->PutValue(EPropertySetListColumns::Duration, pPropSet->m_nAppointmentDuration);
					pRow->PutValue(EPropertySetListColumns::Available, AsVariant(pPropSet->CalcAvailabilityText()));
					pRow->PutValue(EPropertySetListColumns::Types, AsBstr(pPropSet->CalcAptTypesText()));
					pRow->PutValue(EPropertySetListColumns::PurposeSets, AsBstr(pPropSet->CalcAptPurposeSetsText()));
					pRow->PutValue(EPropertySetListColumns::ResourceSets, AsBstr(pPropSet->CalcResourceSetsText()));
					pRow->PutValue(EPropertySetListColumns::Locations, AsBstr(pPropSet->CalcLocationsText()));
					pdl->AddRowSorted(pRow, NULL);
				}
			}

			// Fill our 'current' map as a copy of the original (since by definition on load original=current)
			m_mapCurrentPropSets.Copy(m_mapOrigPropSets);
		}

		ReflectEnabledControls();

	} NxCatchAllCall(__FUNCTION__, { EndDialog(IDCANCEL); return FALSE; });

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// Basic validation.
BOOL CApptPrototypeEntryDlg::Validate()
{
	// TODO: We can make this more advanced (like giving you all warnings in a single 
	// messagebox, or just following each prompt with a pop-up dialog to fix it as we 
	// validate) later.

	// Validate name
	{
		CString strNewName;
		GetDlgItemText(IDC_NAME_EDIT, strNewName);
		strNewName.Trim();

		// fail if empty
		if (strNewName.IsEmpty()) {
			MessageBox(_T("Please enter a name for this appointment prototype."), NULL, MB_OK|MB_ICONEXCLAMATION);
			return FALSE;
		}

		// fail if too long
		if (strNewName.GetLength() > 200) {
			MessageBox(_T("The specified prototype name is too long.  Please enter a name of less than 200 characters."), NULL, MB_OK|MB_ICONEXCLAMATION);
			return FALSE;
		}

		// fail if non-unique
		for (INT_PTR i=0, nCount=m_parystrOtherPrototypeNames->GetSize(); i<nCount; i++) {
			CString strOtherName = m_parystrOtherPrototypeNames->GetAt(i);
			strOtherName.Trim();
			if (strNewName.CompareNoCase(strOtherName) == 0) {
				MessageBox(_T("The specified prototype name is already in use by another prototype.  Please enter a unique name."), NULL, MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
		}
	}
	
	// Validate description
	{
		CString strNewDescription;
		GetDlgItemText(IDC_DESCRIPTION_EDIT, strNewDescription);
		strNewDescription.Trim();
		
		// fail if too long
		if (strNewDescription.GetLength() > 2000) {
			MessageBox(_T("The specified prototype description is too long.  Please enter a description of less than 2000 characters."), NULL, MB_OK|MB_ICONEXCLAMATION);
			return FALSE;
		}
	}

	// Validate each propertyset
	for (CApptPrototypePropertySetMap::CPair *p = m_mapCurrentPropSets.PGetFirstAssoc(); p != NULL; p = m_mapCurrentPropSets.PGetNextAssoc(p)) {
		CString strName = p->value->m_strName;
		strName.Trim();

		// Validate name
		{
			// Must be non-empty
			if (strName.IsEmpty()) {
				MessageBox(_T("Please enter a name for each property set."), NULL, MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}

			// Must be no more than 200 characters
			if (strName.GetLength() > 200) {
				MessageBox(_T("Property set names may not be more than 200 characters.  Please enter a shorter name for property set '") + strName + _T("'."), NULL, MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}

			// Disallow duplicate propset names
			// (We could probably get away with reusing 'p' to go to only evaluate the "later" items in the 
			// map, but because the map object is designed to be opaque we can't rely on its internal state 
			// remaining stable through unrelated calls to PGetNextAssoc(), so we use our own totally 
			// separate enumerator.)
			for (CApptPrototypePropertySetMap::CPair *pOther = m_mapCurrentPropSets.PGetFirstAssoc(); pOther != NULL; pOther = m_mapCurrentPropSets.PGetNextAssoc(pOther)) {
				if (pOther->key != p->key) {
					CString strNameOther = pOther->value->m_strName;
					strNameOther.Trim();
					if (strName.CompareNoCase(strNameOther) == 0) {
						 // Another one had the same name as ours!
						MessageBox(_T("Each property set must have a different name.  Please change the name of at least one of the '") + strName + _T("' property sets to ensure uniqueness."), NULL, MB_OK|MB_ICONEXCLAMATION);
						return FALSE;
					}
				}
			}
		}

		// Validate availability
		{
			// At least one available day
			if (p->value->m_nAvailDays == 0) {
				MessageBox(_T("Every property set must allow at least one day of the week.  Please select available days for property set '") + strName + _T("'."), NULL, MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
			// At least one available time
			if (p->value->m_nAvailTimes == 0) {
				MessageBox(_T("Every property set must allow at least one time of the day.  Please select available times for property set '") + strName + _T("'."), NULL, MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
		}

		// Validate duration
		{
			// Duration must be in range
			if (p->value->m_nAppointmentDuration < 1 || p->value->m_nAppointmentDuration > 1439) {
				MessageBox(_T("Please enter a duration in minutes of at least 1 minute and less than 24 hours for property set '") + strName + _T("'."), NULL, MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
		}

		// Validate purposes
		{
			// Unique set names
			{
				CString strFirstDuplicateName;
				if (FindFirstDuplicateName(p->value->m_aryAptPurposeSets, strFirstDuplicateName)) {
					MessageBox(_T("Each purpose set in a property set must have a different name.  Please change the name of at least one of the '") + strFirstDuplicateName + _T("' purpose sets in property set '") + strName + _T("' to ensure uniqueness."), NULL, MB_OK|MB_ICONEXCLAMATION);
					return FALSE;
				}
			}

			// Non-empty set names
			for (INT_PTR i=0, nCount=p->value->m_aryAptPurposeSets.GetSize(); i<nCount; i++) {
				CString strSetName = p->value->m_aryAptPurposeSets.GetAt(i).m_strName;
				strSetName.Trim();
				if (strSetName.IsEmpty()) {
					MessageBox(_T("Each purpose set must have a name.  Please enter a name for all purpose sets in property set '") + strName + _T("'."), NULL, MB_OK|MB_ICONEXCLAMATION);
					return FALSE;
				}
			}
		}

		// Validate resources
		{
			// Unique set names
			{
				CString strFirstDuplicateName;
				if (FindFirstDuplicateName(p->value->m_aryResourceSets, strFirstDuplicateName)) {
					MessageBox(_T("Each resource set in a property set must have a different name.  Please change the name of at least one of the '") + strFirstDuplicateName + _T("' resource sets in property set '") + strName + _T("' to ensure uniqueness."), NULL, MB_OK|MB_ICONEXCLAMATION);
					return FALSE;
				}
			}

			// Non-empty set names
			for (INT_PTR i=0, nCount=p->value->m_aryResourceSets.GetSize(); i<nCount; i++) {
				CString strSetName = p->value->m_aryResourceSets.GetAt(i).m_strName;
				strSetName.Trim();
				if (strSetName.IsEmpty()) {
					MessageBox(_T("Each resource set must have a name.  Please enter a name for all resource sets in property set '") + strName + _T("'."), NULL, MB_OK|MB_ICONEXCLAMATION);
					return FALSE;
				}
			}

			// At least one resource set, and all resource sets must have at least one resource
			if (p->value->m_aryResourceSets.IsEmpty()) {
				MessageBox(_T("Every property set must allow at least one resource.  Please select available resources for property set '") + strName + _T("'."), NULL, MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			} else {
				for (INT_PTR i=0, nCount=p->value->m_aryResourceSets.GetSize(); i<nCount; i++) {
					CApptPrototypePropertySetResourceSet &set = p->value->m_aryResourceSets.GetAt(i);
					if (set.m_mapdwDetailIDs.IsEmpty()) {
						CString strSetName = set.m_strName;
						strSetName.Trim();
						MessageBox(_T("Every resource set must include at least one resource.  Please select resources for resource set '") + strSetName + _T("' in property set '") + strName + _T("'."), NULL, MB_OK|MB_ICONEXCLAMATION);
						return FALSE;
					}
				}
			}
		}
		
		// Validate locations
		{
			// At least one location
			if (p->value->m_aryLocations.IsEmpty()) {
				MessageBox(_T("Every property set must allow at least one location.  Please select available locations for property set '") + strName + _T("'."), NULL, MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
		}
		
		// TODO: We could do some more helpful validation, e.g. making sure the user doesn't create 
		// multiple propertysets that support the same combination of options (i.e. a given 
		// 'apttype+aptpurpose+resource+location' combination should always positively identify a 
		// single propertyset)
		
		// TODO: Another more helpful validation idea: make sure different propertysets don't have 
		// any of the same purposeset names, or the same resourceset names.  The data structure 
		// allows this, but it would confuse things later to have propertyset1 with purposeset 
		// 'bleph' AND propertyset2 with purposeset 'bleph'.
	}

	return TRUE;
}

// Returns expression1 if the two expressions are NOT EQUAL, otherwise returns variant null.
_variant_t NullIf(const CString &expression1, const CString &expression2)
{
	if (expression1 == expression2) {
		return g_cvarNull;
	} else {
		return AsVariant(expression1);
	}
}

void GrowFastConcat(CString &str, LPCTSTR fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);
	GrowFastConcatV(str, fmt, argList);
	va_end(argList);
}

#define CalcPropertySetDiffs_ChangedRootProperties_HANDLE_AVAIL_X(x, attribute_name, enum_name) \
	if ((pPropSetCur->m_nAvail##x & EApptPrototypeAllow##x::enum_name) != (pPropSetOrig->m_nAvail##x & EApptPrototypeAllow##x::enum_name)) { \
		GrowFastConcat(ans, attribute_name _T("=\"%li\" "), (pPropSetCur->m_nAvail##x & EApptPrototypeAllow##x::enum_name) ? 1 : 0); }

#define CPSD_CRP_HANDLE_AVAIL_TIME(attribute_name, enum_name) CalcPropertySetDiffs_ChangedRootProperties_HANDLE_AVAIL_X(Times, attribute_name, enum_name)
#define CPSD_CRP_HANDLE_AVAIL_DAY(attribute_name, enum_name) CalcPropertySetDiffs_ChangedRootProperties_HANDLE_AVAIL_X(Days, attribute_name, enum_name)

CString CalcPropertySetDiffs_ChangedRootProperties(const CApptPrototypePropertySetMap &mapCurrentPropSets, const CApptPrototypePropertySetMap &mapOrigPropSets, OUT OPTIONAL CString *pstrOpenXmlSpec)
{
	// If the caller wants our spec, give it here at the beginning because the caller expects it even if we fail
	if (pstrOpenXmlSpec) {
		*pstrOpenXmlSpec = _T("ID INT, Name NVARCHAR(200) '@N', AppointmentDuration INT '@AD', ")
			_T("AllowTimeEarlyMorning BIT '@TEM', AllowTimeLateMorning BIT '@TLM', AllowTimeEarlyAfternoon BIT '@TEA', AllowTimeLateAfternoon BIT '@TLA', ")
			_T("AllowDaySunday BIT '@DSu', AllowDayMonday BIT '@DMo', AllowDayTuesday BIT '@DTu', AllowDayWednesday BIT '@DWe', AllowDayThursday BIT '@DTh', AllowDayFriday BIT '@DFr', AllowDaySaturday BIT '@DSa'")
			;
	}

	BOOL bSomethingAdded = FALSE;
	CString ans = _T("<root>");
	for (POSITION pos = mapCurrentPropSets.GetStartPosition(); pos != 0; ) {
		LONG nPropSetID;
		CApptPrototypePropertySet *pPropSetCur;
		mapCurrentPropSets.GetNextAssoc(pos, nPropSetID, pPropSetCur);
		// Find this one in the original propertyset map
		CApptPrototypePropertySet *pPropSetOrig;
		if (mapOrigPropSets.Lookup(nPropSetID, pPropSetOrig)) {
			if (pPropSetCur->m_strName != pPropSetOrig->m_strName ||
				pPropSetCur->m_nAppointmentDuration != pPropSetOrig->m_nAppointmentDuration ||
				pPropSetCur->m_nAvailDays != pPropSetOrig->m_nAvailDays || 
				pPropSetCur->m_nAvailTimes != pPropSetOrig->m_nAvailTimes
				) {
				// Something changed, so add an entry for it
				bSomethingAdded = TRUE;
				GrowFastConcat(ans, _T("<r ID=\"%li\" "), nPropSetID);
				// Add name if it changed
				if (pPropSetCur->m_strName != pPropSetOrig->m_strName) {
					GrowFastConcat(ans, _T("N=\"%s\" "), ConvertToQuotableXMLString(pPropSetCur->m_strName));
				}
				// Add duration if it changed
				if (pPropSetCur->m_nAppointmentDuration != pPropSetOrig->m_nAppointmentDuration) {
					GrowFastConcat(ans, _T("AD=\"%li\" "), pPropSetCur->m_nAppointmentDuration);
				}
				// Add the changed times if any changed
				if (pPropSetCur->m_nAvailTimes != pPropSetOrig->m_nAvailTimes) {
					CPSD_CRP_HANDLE_AVAIL_TIME(_T("TEM"), EarlyMorning);
					CPSD_CRP_HANDLE_AVAIL_TIME(_T("TLM"), LateMorning);
					CPSD_CRP_HANDLE_AVAIL_TIME(_T("TEA"), EarlyAfternoon);
					CPSD_CRP_HANDLE_AVAIL_TIME(_T("TLA"), LateAfternoon);
				}
				// Add the changed days if any changed
				if (pPropSetCur->m_nAvailDays != pPropSetOrig->m_nAvailDays) {
					CPSD_CRP_HANDLE_AVAIL_DAY(_T("DSu"), Sunday);
					CPSD_CRP_HANDLE_AVAIL_DAY(_T("DMo"), Monday);
					CPSD_CRP_HANDLE_AVAIL_DAY(_T("DTu"), Tuesday);
					CPSD_CRP_HANDLE_AVAIL_DAY(_T("DWe"), Wednesday);
					CPSD_CRP_HANDLE_AVAIL_DAY(_T("DTh"), Thursday);
					CPSD_CRP_HANDLE_AVAIL_DAY(_T("DFr"), Friday);
					CPSD_CRP_HANDLE_AVAIL_DAY(_T("DSa"), Saturday);
				}
				// Cap the element
				GrowFastConcat(ans, 2, _T("/>"));
			}
		}
	}

	// Return the result, or empty string if there were no changed properties
	if (bSomethingAdded) {
		GrowFastConcat(ans, 7, _T("</root>"));
		return ans;
	} else {
		return _T("");
	}
}

void CalcPropertySetDiffs_DeletedPropertySets(const CApptPrototypePropertySetMap &mapCurrentPropSets, const CApptPrototypePropertySetMap &mapOrigPropSets, OUT CArray<long,long> *parynIDsToDelete)
{
	CArray<long,long> &arynIDsToDelete = *parynIDsToDelete;
	// Initialize to an array of sixteen entries (we'll expand if necessary, and if not then we'll set any 
	// extras to -1).  This is so our query text will rarely change (only when the count is greater than 16).
	arynIDsToDelete.SetSize(16);
	INT_PTR i = 0;
	for (POSITION pos = mapOrigPropSets.GetStartPosition(); pos != 0; ) {
		LONG nPropSetID;
		CApptPrototypePropertySet *pPropSet;
		mapOrigPropSets.GetNextAssoc(pos, nPropSetID, pPropSet);
		if (mapCurrentPropSets.PLookup(nPropSetID) == NULL) {
			arynIDsToDelete.SetAt(i, nPropSetID);
			i++;
		}
	}
	for (INT_PTR nCount = arynIDsToDelete.GetSize(); i < nCount; i++) {
		arynIDsToDelete.SetAt(i, -1);
	}
}

#define CalcPropertySetDiffs_NewPropertySets_ADD_AVAIL_X(x, attribute_name, enum_name) \
	GrowFastConcat(ans, attribute_name _T("=\"%li\" "), (pPropSet->m_nAvail##x & EApptPrototypeAllow##x::enum_name) ? 1 : 0);

#define CPSD_NPS_ADD_AVAIL_TIME(attribute_name, enum_name) CalcPropertySetDiffs_NewPropertySets_ADD_AVAIL_X(Times, attribute_name, enum_name)
#define CPSD_NPS_ADD_AVAIL_DAY(attribute_name, enum_name) CalcPropertySetDiffs_NewPropertySets_ADD_AVAIL_X(Days, attribute_name, enum_name)

CString CalcPropertySetDiffs_NewPropertySets(const CApptPrototypePropertySetMap &mapCurrentPropSets, OUT OPTIONAL CString *pstrOpenXmlSpec, OUT OPTIONAL CString *pstrTableSpec, OUT OPTIONAL CString *pstrFieldList)
{
	// If the caller wants our spec, give it here at the beginning because the caller expects it even if we fail
	if (pstrOpenXmlSpec) {
		*pstrOpenXmlSpec = _T("PseudoID INT '@ID', Name NVARCHAR(200) '@N', AppointmentDuration INT '@AD', ")
			_T("AllowTimeEarlyMorning BIT '@TEM', AllowTimeLateMorning BIT '@TLM', AllowTimeEarlyAfternoon BIT '@TEA', AllowTimeLateAfternoon BIT '@TLA', ")
			_T("AllowDaySunday BIT '@DSu', AllowDayMonday BIT '@DMo', AllowDayTuesday BIT '@DTu', AllowDayWednesday BIT '@DWe', AllowDayThursday BIT '@DTh', AllowDayFriday BIT '@DFr', AllowDaySaturday BIT '@DSa'")
			;
	}
	if (pstrTableSpec) {
		*pstrTableSpec = _T("PseudoID INT NOT NULL, Name NVARCHAR(200) NOT NULL, AppointmentDuration INT NOT NULL, ")
			_T("AllowTimeEarlyMorning BIT NOT NULL, AllowTimeLateMorning BIT NOT NULL, AllowTimeEarlyAfternoon BIT NOT NULL, AllowTimeLateAfternoon BIT NOT NULL, ")
			_T("AllowDaySunday BIT NOT NULL, AllowDayMonday BIT NOT NULL, AllowDayTuesday BIT NOT NULL, AllowDayWednesday BIT NOT NULL, AllowDayThursday BIT NOT NULL, AllowDayFriday BIT NOT NULL, AllowDaySaturday BIT NOT NULL")
			;
	}
	if (pstrFieldList) {
		*pstrFieldList = _T("PseudoID, Name, AppointmentDuration, ")
			_T("AllowTimeEarlyMorning, AllowTimeLateMorning, AllowTimeEarlyAfternoon, AllowTimeLateAfternoon, ")
			_T("AllowDaySunday, AllowDayMonday, AllowDayTuesday, AllowDayWednesday, AllowDayThursday, AllowDayFriday, AllowDaySaturday")
			;
	}

	BOOL bSomethingAdded = FALSE;
	CString ans = _T("<root>");
	for (POSITION pos = mapCurrentPropSets.GetStartPosition(); pos != 0; ) {
		LONG nPropSetID;
		CApptPrototypePropertySet *pPropSet;
		mapCurrentPropSets.GetNextAssoc(pos, nPropSetID, pPropSet);
		if (nPropSetID < 0) {
			// Only new ones have negative IDs
			bSomethingAdded = TRUE;
			GrowFastConcat(ans, _T("<s ID=\"%li\" "), nPropSetID);
			// Add name
			GrowFastConcat(ans, _T("N=\"%s\" "), ConvertToQuotableXMLString(pPropSet->m_strName));
			// Add duration
			GrowFastConcat(ans, _T("AD=\"%li\" "), pPropSet->m_nAppointmentDuration);
			// Add the times
			CPSD_NPS_ADD_AVAIL_TIME(_T("TEM"), EarlyMorning);
			CPSD_NPS_ADD_AVAIL_TIME(_T("TLM"), LateMorning);
			CPSD_NPS_ADD_AVAIL_TIME(_T("TEA"), EarlyAfternoon);
			CPSD_NPS_ADD_AVAIL_TIME(_T("TLA"), LateAfternoon);
			// Add the days
			CPSD_NPS_ADD_AVAIL_DAY(_T("DSu"), Sunday);
			CPSD_NPS_ADD_AVAIL_DAY(_T("DMo"), Monday);
			CPSD_NPS_ADD_AVAIL_DAY(_T("DTu"), Tuesday);
			CPSD_NPS_ADD_AVAIL_DAY(_T("DWe"), Wednesday);
			CPSD_NPS_ADD_AVAIL_DAY(_T("DTh"), Thursday);
			CPSD_NPS_ADD_AVAIL_DAY(_T("DFr"), Friday);
			CPSD_NPS_ADD_AVAIL_DAY(_T("DSa"), Saturday);
			// Cap the node
			GrowFastConcat(ans, 1, _T(">"));
			// Add subelements
			{
				// Add types
				{
					for (int i=0, nCount = pPropSet->m_aryAptTypes.GetSize(); i<nCount; i++) {
						CApptPrototypePropertySetAptType &type = pPropSet->m_aryAptTypes.GetAt(i);
						GrowFastConcat(ans, _T("<st ID=\"%li\" />"), type.m_nID);
					}
				}
				// Add purposesets
				{
					for (int i=0, nCount = pPropSet->m_aryAptPurposeSets.GetSize(); i<nCount; i++) {
						CApptPrototypePropertySetAptPurposeSet &purposeSet = pPropSet->m_aryAptPurposeSets.GetAt(i);
						GrowFastConcat(ans, _T("<sps ID=\"%li\" N=\"%s\">"), purposeSet.m_nID, ConvertToQuotableXMLString(purposeSet.m_strName));
						for (POSITION det = purposeSet.m_mapdwDetailIDs.GetStartPosition(); det != 0; ) {
							LONG detID;
							BYTE dontcare;
							purposeSet.m_mapdwDetailIDs.GetNextAssoc(det, detID, dontcare);
							GrowFastConcat(ans, _T("<spsd PID=\"%li\" />"), detID);
						}
						GrowFastConcat(ans, 6, _T("</sps>"));
					}
				}
				// Add resourcesets
				{
					for (int i=0, nCount = pPropSet->m_aryResourceSets.GetSize(); i<nCount; i++) {
						CApptPrototypePropertySetResourceSet &resourceSet = pPropSet->m_aryResourceSets.GetAt(i);
						GrowFastConcat(ans, _T("<srs ID=\"%li\" N=\"%s\">"), resourceSet.m_nID, ConvertToQuotableXMLString(resourceSet.m_strName));
						for (POSITION det = resourceSet.m_mapdwDetailIDs.GetStartPosition(); det != 0; ) {
							LONG detID;
							BYTE dontcare;
							resourceSet.m_mapdwDetailIDs.GetNextAssoc(det, detID, dontcare);
							GrowFastConcat(ans, _T("<srsd RID=\"%li\" />"), detID);
						}
						GrowFastConcat(ans, 6, _T("</srs>"));
					}
				}
				// Add locations
				{
					for (int i=0, nCount = pPropSet->m_aryLocations.GetSize(); i<nCount; i++) {
						CApptPrototypePropertySetLocation &location = pPropSet->m_aryLocations.GetAt(i);
						GrowFastConcat(ans, _T("<sl ID=\"%li\" />"), location.m_nID);
					}
				}
			}
			// Close the propertyset element
			GrowFastConcat(ans, 4, _T("</s>"));
		}
	}
	if (bSomethingAdded) {
		GrowFastConcat(ans, 7, _T("</root>"));
		return ans;
	} else {
		return _T("");
	}
}

BOOL SaveApptPrototype_AddToBatch_InsertAllNewPropertySets(CWnd *pMsgParent, IN OUT CParamSqlBatch &sql, const CApptPrototypePropertySetMap &mapCurrentPropSets)
{
	// Calculate the xml and on the sql side enter the if block if the xml is non-empty
	CString strOpenXmlSpec, strTableSpec, strFieldList;
	CString xml = CalcPropertySetDiffs_NewPropertySets(mapCurrentPropSets, &strOpenXmlSpec, &strTableSpec, &strFieldList);
	sql.Add(_T("IF ({BIT} = 1) BEGIN \r\n"), !xml.IsEmpty());

	// Fill the staging local variable tables
	{
		sql.Declare(_T("DECLARE @tNewPropertySets TABLE (RealID INT, ") + strTableSpec + _T(") \r\n"));
		sql.Declare(_T("DECLARE @tNewPropertySetAptTypes TABLE (PropertySetPseudoID INT NOT NULL, PropertySetRealID INT, AptTypeID INT NOT NULL) \r\n"));
		sql.Declare(_T("DECLARE @tNewPropertySetAptPurposeSets TABLE (RealID INT, PseudoID INT NOT NULL, Name NVARCHAR(200) NOT NULL, PropertySetPseudoID INT NOT NULL, PropertySetRealID INT) \r\n"));
		sql.Declare(_T("DECLARE @tNewPropertySetAptPurposeSetDetails TABLE (PurposeSetPseudoID INT NOT NULL, PurposeSetRealID INT, AptPurposeID INT NOT NULL) \r\n"));
		sql.Declare(_T("DECLARE @tNewPropertySetResourceSets TABLE (RealID INT, PseudoID INT NOT NULL, Name NVARCHAR(200) NOT NULL, PropertySetPseudoID INT NOT NULL, PropertySetRealID INT) \r\n"));
		sql.Declare(_T("DECLARE @tNewPropertySetResourceSetDetails TABLE (ResourceSetPseudoID INT NOT NULL, ResourceSetRealID INT, ResourceID INT NOT NULL) \r\n"));
		sql.Declare(_T("DECLARE @tNewPropertySetLocations TABLE (PropertySetPseudoID INT NOT NULL, PropertySetRealID INT, LocationID INT NOT NULL) \r\n"));
		// Open the xml and fill the local variable tables from it
		sql.Add(_T("EXEC sp_xml_preparedocument @hDoc OUTPUT, {VT_BSTR} \r\n"), NullIf(xml, _T("")));
		// Propertysets staging
		sql.Add(
			_T("INSERT INTO @tNewPropertySets (") + strFieldList + _T(") \r\n")
			_T("SELECT ") + strFieldList + _T(" FROM OPENXML(@hDoc, '/root/s') WITH (") + strOpenXmlSpec + _T(") T \r\n")
			);
		// Propertyset apttypes staging
		sql.Add(
			_T("INSERT INTO @tNewPropertySetAptTypes \r\n")
			_T("SELECT PropertySetPseudoID, NULL, AptTypeID \r\n")
			_T("FROM OPENXML(@hDoc, '/root/s/st') WITH (PropertySetPseudoID INT '../@ID', AptTypeID INT '@ID') T \r\n")
			);
		// Propertyset aptpurposesets staging
		sql.Add(
			_T("INSERT INTO @tNewPropertySetAptPurposeSets \r\n")
			_T("SELECT NULL, PseudoID, Name, PropertySetPseudoID, NULL \r\n")
			_T("FROM OPENXML(@hDoc, '/root/s/sps') WITH (PseudoID INT '@ID', Name NVARCHAR(200) '@N', PropertySetPseudoID INT '../@ID') T \r\n")
			);
		sql.Add(
			_T("INSERT INTO @tNewPropertySetAptPurposeSetDetails \r\n")
			_T("SELECT PurposeSetPseudoID, NULL, AptPurposeID \r\n")
			_T("FROM OPENXML(@hDoc, '/root/s/sps/spsd') WITH (PurposeSetPseudoID INT '../@ID', AptPurposeID INT '@PID') T \r\n")
			);
		// Propertyset resourcesets staging
		sql.Add(
			_T("INSERT INTO @tNewPropertySetResourceSets \r\n")
			_T("SELECT NULL, PseudoID, Name, PropertySetPseudoID, NULL \r\n")
			_T("FROM OPENXML(@hDoc, '/root/s/srs') WITH (PseudoID INT '@ID', Name NVARCHAR(200) '@N', PropertySetPseudoID INT '../@ID') T \r\n")
			);
		sql.Add(
			_T("INSERT INTO @tNewPropertySetResourceSetDetails \r\n")
			_T("SELECT ResourceSetPseudoID, NULL, ResourceID \r\n")
			_T("FROM OPENXML(@hDoc, '/root/s/srs/srsd') WITH (ResourceSetPseudoID INT '../@ID', ResourceID INT '@RID') T \r\n")
			);
		// Propertyset locations staging
		sql.Add(
			_T("INSERT INTO @tNewPropertySetLocations \r\n")
			_T("SELECT PropertySetPseudoID, NULL, LocationID \r\n")
			_T("FROM OPENXML(@hDoc, '/root/s/sl') WITH (PropertySetPseudoID INT '../@ID', LocationID INT '@ID') T \r\n")
			);
		sql.Add(_T("EXEC sp_xml_removedocument @hDoc \r\n"));
	}

	// Fill the real tables from the staging data, populating the "real" ids in the staging tables as we go
	{
		// Fill the propertyset table
		{
			// Add real records
			sql.Add(
				_T("INSERT INTO ApptPrototypePropertySetT (Name, ApptPrototypeID, AppointmentDuration, AllowTimeEarlyMorning, AllowTimeLateMorning, AllowTimeEarlyAfternoon, AllowTimeLateAfternoon, AllowDaySunday, AllowDayMonday, AllowDayTuesday, AllowDayWednesday, AllowDayThursday, AllowDayFriday, AllowDaySaturday) \r\n")
				_T("SELECT Name, @prototypeID, AppointmentDuration, AllowTimeEarlyMorning, AllowTimeLateMorning, AllowTimeEarlyAfternoon, AllowTimeLateAfternoon, AllowDaySunday, AllowDayMonday, AllowDayTuesday, AllowDayWednesday, AllowDayThursday, AllowDayFriday, AllowDaySaturday \r\n")
				_T("FROM @tNewPropertySets")
				);
			// Reflect real IDs in staging tables
			{
				// Base staging table
				sql.Add(
					_T("UPDATE @tNewPropertySets SET RealID = PS.ID \r\n")
					_T("FROM @tNewPropertySets T INNER JOIN ApptPrototypePropertySetT PS ON T.Name = PS.Name AND PS.ApptPrototypeID = @prototypeID \r\n") // reliable thanks to the uniqueness constraint on ApptPrototypePropertySetT (ApptPrototypeID+Name)
					);
				// Any other staging tables that reference it
				sql.Add(
					_T("UPDATE @tNewPropertySetAptPurposeSets SET PropertySetRealID = S.RealID \r\n")
					_T("FROM @tNewPropertySetAptPurposeSets T INNER JOIN @tNewPropertySets S ON T.PropertySetPseudoID = S.PseudoID \r\n")
					);
				sql.Add(
					_T("UPDATE @tNewPropertySetResourceSets SET PropertySetRealID = S.RealID \r\n")
					_T("FROM @tNewPropertySetResourceSets T INNER JOIN @tNewPropertySets S ON T.PropertySetPseudoID = S.PseudoID \r\n")
					);
				sql.Add(
					_T("UPDATE @tNewPropertySetAptTypes SET PropertySetRealID = S.RealID \r\n")
					_T("FROM @tNewPropertySetAptTypes T INNER JOIN @tNewPropertySets S ON T.PropertySetPseudoID = S.PseudoID \r\n")
					);
				sql.Add(
					_T("UPDATE @tNewPropertySetLocations SET PropertySetRealID = S.RealID \r\n")
					_T("FROM @tNewPropertySetLocations T INNER JOIN @tNewPropertySets S ON T.PropertySetPseudoID = S.PseudoID \r\n")
					);
			}
		}

		// Fill the types table
		{
			// Add real records
			sql.Add(
				_T("INSERT INTO ApptPrototypePropertySetAptTypeT (ApptPrototypePropertySetID, AptTypeID) \r\n")
				_T("SELECT PropertySetRealID, AptTypeID \r\n")
				_T("FROM @tNewPropertySetAptTypes \r\n"));
		}

		// Fill the purposesets and purposesetdetails tables
		{
			// Add real base records
			sql.Add(
				_T("INSERT INTO ApptPrototypePropertySetAptPurposeSetT (Name, ApptPrototypePropertySetID) \r\n")
				_T("SELECT Name, PropertySetRealID \r\n")
				_T("FROM @tNewPropertySetAptPurposeSets \r\n")
				);
			// Reflect real IDs in staging tables
			{
				// Base staging table
				sql.Add(
					_T("UPDATE @tNewPropertySetAptPurposeSets SET RealID = PSAPS.ID \r\n")
					_T("FROM @tNewPropertySetAptPurposeSets T \r\n")
					_T("INNER JOIN ApptPrototypePropertySetAptPurposeSetT PSAPS ON T.Name = PSAPS.Name AND T.PropertySetRealID = PSAPS.ApptPrototypePropertySetID \r\n") // reliable thanks to the uniqueness constraint on ApptPrototypePropertySetAptPurposeSetT (ApptPrototypePropertySetID+Name)
					);
				// Details staging table
				sql.Add(
					_T("UPDATE @tNewPropertySetAptPurposeSetDetails SET PurposeSetRealID = SPS.RealID \r\n")
					_T("FROM @tNewPropertySetAptPurposeSetDetails T INNER JOIN @tNewPropertySetAptPurposeSets SPS ON T.PurposeSetPseudoID = SPS.PseudoID \r\n"));
			}
			// Add real detail records
			sql.Add(
				_T("INSERT INTO ApptPrototypePropertySetAptPurposeSetDetailT (ApptPrototypePropertySetAptPurposeSetID, AptPurposeID) \r\n")
				_T("SELECT T.PurposeSetRealID, T.AptPurposeID \r\n")
				_T("FROM @tNewPropertySetAptPurposeSetDetails T \r\n")
				);
		}

		// Fill the resourcesets and resourcesetdetails tables
		{
			// Add real base records
			sql.Add(
				_T("INSERT INTO ApptPrototypePropertySetResourceSetT (Name, ApptPrototypePropertySetID) \r\n")
				_T("SELECT T.Name, T.PropertySetRealID \r\n")
				_T("FROM @tNewPropertySetResourceSets T \r\n")
				);
			// Reflect real IDs in staging tables
			{
				// Base staging table
				sql.Add(
					_T("UPDATE @tNewPropertySetResourceSets SET RealID = PSRS.ID \r\n")
					_T("FROM @tNewPropertySetResourceSets T \r\n")
					_T("INNER JOIN ApptPrototypePropertySetResourceSetT PSRS ON T.Name = PSRS.Name AND T.PropertySetRealID = PSRS.ApptPrototypePropertySetID \r\n") // reliable thanks to the uniqueness constraint on ApptPrototypePropertySetResourceSetT (ApptPrototypePropertySetID+Name)
					);
				// Details staging table
				sql.Add(
					_T("UPDATE @tNewPropertySetResourceSetDetails SET ResourceSetRealID = SRS.RealID \r\n")
					_T("FROM @tNewPropertySetResourceSetDetails T INNER JOIN @tNewPropertySetResourceSets SRS ON T.ResourceSetPseudoID = SRS.PseudoID \r\n"));
			}
			// Add real detail records
			sql.Add(
				_T("INSERT INTO ApptPrototypePropertySetResourceSetDetailT (ApptPrototypePropertySetResourceSetID, ResourceID) \r\n")
				_T("SELECT T.ResourceSetRealID, T.ResourceID \r\n")
				_T("FROM @tNewPropertySetResourceSetDetails T \r\n")
				);
		}

		// Fill the locations table
		{
			// Add real records
			sql.Add(
				_T("INSERT INTO ApptPrototypePropertySetLocationT (ApptPrototypePropertySetID, LocationID) \r\n")
				_T("SELECT PropertySetRealID, LocationID \r\n")
				_T("FROM @tNewPropertySetLocations \r\n"));
		}
	}

	// Okay we're done, now just "assert" that all the pseudo IDs in the staging tables now correspond to real IDs (which we've just created)
	sql.Add(
		_T("IF (EXISTS (SELECT * FROM @tNewPropertySets WHERE RealID IS NULL) \r\n")
		_T(" OR EXISTS (SELECT * FROM @tNewPropertySetAptTypes WHERE PropertySetRealID IS NULL) \r\n")
		_T(" OR EXISTS (SELECT * FROM @tNewPropertySetAptPurposeSets WHERE RealID IS NULL OR PropertySetRealID IS NULL) \r\n")
		_T(" OR EXISTS (SELECT * FROM @tNewPropertySetAptPurposeSetDetails WHERE PurposeSetRealID IS NULL) \r\n")
		_T(" OR EXISTS (SELECT * FROM @tNewPropertySetResourceSets WHERE RealID IS NULL OR PropertySetRealID IS NULL) \r\n")
		_T(" OR EXISTS (SELECT * FROM @tNewPropertySetResourceSetDetails WHERE ResourceSetRealID IS NULL) \r\n")
		_T(" OR EXISTS (SELECT * FROM @tNewPropertySetLocations WHERE PropertySetRealID IS NULL) \r\n")
		_T(" ) BEGIN \r\n")
		_T("  RAISERROR('Records were not created or could not be positively matched between staging data and real data.', 16, 1) \r\n")
		_T("  ROLLBACK TRAN \r\n")
		_T("  RETURN \r\n")
		_T("END \r\n")
		);
	
	// Leave the if block
	sql.Add(_T("END \r\n"));

	return TRUE;
}

// Creates xml of the form: <root><todelete><d/><d/>..</todelete><tochange><c/><c/>..</tochange><toadd><a/><a/>..</toadd></root>
// Entries are only from property sets that already exist in data, not new property sets.
CString CalcPropertySetDiffs_ChangedNamedObjects(const CApptPrototypePropertySetMap &mapCurrentPropSets, const CApptPrototypePropertySetMap &mapOrigPropSets, ENamedObjectsType::_Enum enotType)
{
	BOOL bAppendedXml = FALSE;
	CString ans = _T("<root>");
	// Append any namedobjects that need to be deleted
	{
		GrowFastConcat(ans, 10, _T("<todelete>"));
		for (POSITION pos = mapCurrentPropSets.GetStartPosition(); pos != 0; ) {
			LONG nPropSetID;
			CApptPrototypePropertySet *pPropSetCur;
			mapCurrentPropSets.GetNextAssoc(pos, nPropSetID, pPropSetCur);
			// Find this one in the original propertyset map
			CApptPrototypePropertySet *pPropSetOrig;
			if (mapOrigPropSets.Lookup(nPropSetID, pPropSetOrig)) {
				CArray<CApptPrototypePropertySetNamedObject, CApptPrototypePropertySetNamedObject &> &origObjects = pPropSetOrig->GetNamedObjects(enotType);
				CArray<CApptPrototypePropertySetNamedObject, CApptPrototypePropertySetNamedObject &> &curObjects = pPropSetCur->GetNamedObjects(enotType);
				// Found the matching one in the original data, so see if it has any namedobjects that our current one doesn't have
				for (INT_PTR iOrig=0, countOrig = origObjects.GetSize(); iOrig<countOrig; iOrig++) {
					int nOrigObjectID = origObjects.GetAt(iOrig).m_nID;
					bool found = false;
					for (INT_PTR iCur=0, countCur = curObjects.GetSize(); iCur<countCur; iCur++) {
						if (nOrigObjectID == curObjects.GetAt(iCur).m_nID) {
							found = true;
							break;
						}
					}
					if (!found) {
						// Ok, it was in the orig but not in our cur, so it needs to be deleted from data
						bAppendedXml = true;
						GrowFastConcat(ans, _T("<d SID=\"%li\" ID=\"%li\" />"), pPropSetCur->m_nID, nOrigObjectID);
					}
				}
			}
		}
		GrowFastConcat(ans, 11, _T("</todelete>"));
	}
	
	// Append any namedobjects that need to be added
	{
		// Since we don't yet store "friendly names" for apttypes or locations in data, we don't 
		// present a UI to let the user change them.  So don't bother comparing them in memory, 
		// they'll never be different.
	}

	// Append any namedobjects that need to be added
	{
		GrowFastConcat(ans, 7, _T("<toadd>"));
		
		for (POSITION pos = mapCurrentPropSets.GetStartPosition(); pos != 0; ) {
			LONG nPropSetID;
			CApptPrototypePropertySet *pPropSetCur;
			mapCurrentPropSets.GetNextAssoc(pos, nPropSetID, pPropSetCur);
			// Find this one in the original propertyset map
			CApptPrototypePropertySet *pPropSetOrig;
			if (mapOrigPropSets.Lookup(nPropSetID, pPropSetOrig)) {
				CArray<CApptPrototypePropertySetNamedObject, CApptPrototypePropertySetNamedObject &> &origObjects = pPropSetOrig->GetNamedObjects(enotType);
				CArray<CApptPrototypePropertySetNamedObject, CApptPrototypePropertySetNamedObject &> &curObjects = pPropSetCur->GetNamedObjects(enotType);
				// Found the matching one in the original data, so see if our current one has any namedobjects that it doesn't have
				for (INT_PTR iCur=0, countCur = curObjects.GetSize(); iCur<countCur; iCur++) {
					CApptPrototypePropertySetNamedObject &objectCur = curObjects.GetAt(iCur);
					bool found = false;
					for (INT_PTR iOrig=0, countOrig = origObjects.GetSize(); iOrig<countOrig; iOrig++) {
						CApptPrototypePropertySetNamedObject &objectOrig = origObjects.GetAt(iOrig);
						if (objectCur.m_nID == objectOrig.m_nID) {
							// Found it, no need to keep looking
							found = true;
							break;
						}
					}
					if (!found) {
						// Ok, it was in our cur but not in the orig, so it needs to be added to data
						bAppendedXml = true;
						GrowFastConcat(ans, _T("<a SID=\"%li\" ID=\"%li\" N=\"%s\" />"), pPropSetCur->m_nID, objectCur.m_nID, ConvertToQuotableXMLString(objectCur.m_strName));
					}
				}
			}
		}
		GrowFastConcat(ans, 8, _T("</toadd>"));
	}

	// If anything was appended, then close the root element and return it, otherwise return empty string 
	if (bAppendedXml) {
		GrowFastConcat(ans, 7, _T("</root>"));
		return ans;
	} else {
		return _T("");
	}
}

// Creates xml of the form: <root><todelete><d/><d/>..</todelete><tochange><c/><c/>..</tochange><toadd><a/><a/>..</toadd></root>
// Entries are only from property sets that already exist in data, not new property sets.
CString CalcPropertySetDiffs_ChangedNamedSets(const CApptPrototypePropertySetMap &mapCurrentPropSets, const CApptPrototypePropertySetMap &mapOrigPropSets, ENamedSetOfDetailsType::_Enum ensodtSetType)
{
	BOOL bAppendedXml = FALSE;
	CString ans = _T("<root>");
	// Append any namedsets that need to be deleted
	{
		GrowFastConcat(ans, 10, _T("<todelete>"));
		for (POSITION pos = mapCurrentPropSets.GetStartPosition(); pos != 0; ) {
			LONG nPropSetID;
			CApptPrototypePropertySet *pPropSetCur;
			mapCurrentPropSets.GetNextAssoc(pos, nPropSetID, pPropSetCur);
			// Find this one in the original propertyset map
			CApptPrototypePropertySet *pPropSetOrig;
			if (mapOrigPropSets.Lookup(nPropSetID, pPropSetOrig)) {
				CArray<CApptPrototypePropertySetNamedSet, CApptPrototypePropertySetNamedSet &> &origPropSetNamedSets = pPropSetOrig->GetNamedSets(ensodtSetType);
				CArray<CApptPrototypePropertySetNamedSet, CApptPrototypePropertySetNamedSet &> &curPropSetNamedSets = pPropSetCur->GetNamedSets(ensodtSetType);
				// Found the matching one in the original data, so see if it has any namedsets that our current one doesn't have
				for (INT_PTR iOrig=0, countOrig = origPropSetNamedSets.GetSize(); iOrig<countOrig; iOrig++) {
					int nOrigNamedSetID = origPropSetNamedSets.GetAt(iOrig).m_nID;
					bool found = false;
					for (INT_PTR iCur=0, countCur = curPropSetNamedSets.GetSize(); iCur<countCur; iCur++) {
						if (nOrigNamedSetID == curPropSetNamedSets.GetAt(iCur).m_nID) {
							found = true;
							break;
						}
					}
					if (!found) {
						// Ok, it was in the orig but not in our cur, so it needs to be deleted from data
						bAppendedXml = true;
						GrowFastConcat(ans, _T("<d ID=\"%li\" />"), nOrigNamedSetID);
					}
				}
			}
		}
		GrowFastConcat(ans, 11, _T("</todelete>"));
	}
	
	// Append any namedsets that need to be added
	{
		GrowFastConcat(ans, 10, _T("<tochange>"));
		for (POSITION pos = mapCurrentPropSets.GetStartPosition(); pos != 0; ) {
			LONG nPropSetID;
			CApptPrototypePropertySet *pPropSetCur;
			mapCurrentPropSets.GetNextAssoc(pos, nPropSetID, pPropSetCur);
			// Find this one in the original propertyset map
			CApptPrototypePropertySet *pPropSetOrig;
			if (mapOrigPropSets.Lookup(nPropSetID, pPropSetOrig)) {
				CArray<CApptPrototypePropertySetNamedSet, CApptPrototypePropertySetNamedSet &> &origPropSetNamedSets = pPropSetOrig->GetNamedSets(ensodtSetType);
				CArray<CApptPrototypePropertySetNamedSet, CApptPrototypePropertySetNamedSet &> &curPropSetNamedSets = pPropSetCur->GetNamedSets(ensodtSetType);
				// Found the matching one in the original data, so see if our current one has any new namedsets
				for (INT_PTR iCur=0, countCur = curPropSetNamedSets.GetSize(); iCur<countCur; iCur++) {
					CApptPrototypePropertySetNamedSet &namedSetCur = curPropSetNamedSets.GetAt(iCur);
					for (INT_PTR iOrig=0, countOrig = origPropSetNamedSets.GetSize(); iOrig<countOrig; iOrig++) {
						CApptPrototypePropertySetNamedSet &namedSetOrig = origPropSetNamedSets.GetAt(iOrig);
						if (namedSetCur.m_nID == namedSetOrig.m_nID) {
							// Found the orig one, see if our name or details have changed and if so append the change
							BOOL changedName = (namedSetCur.m_strName != namedSetOrig.m_strName);
							BOOL changedDetails = !IsSameSet(namedSetCur.m_mapdwDetailIDs, namedSetOrig.m_mapdwDetailIDs);
							if (changedName || changedDetails) {
								bAppendedXml = true;
								GrowFastConcat(ans, _T("<c ID=\"%li\" cd=\"%li\" "), namedSetCur.m_nID, changedDetails?1:0);
								if (changedName) {
									GrowFastConcat(ans, _T("N=\"%s\" "), ConvertToQuotableXMLString(namedSetCur.m_strName));
								}
								if (changedDetails && !namedSetCur.m_mapdwDetailIDs.IsEmpty()) {
									GrowFastConcat(ans, 1, _T(">"));
									for (POSITION det = namedSetCur.m_mapdwDetailIDs.GetStartPosition(); det != 0; ) {
										LONG detID;
										BYTE dontcare;
										namedSetCur.m_mapdwDetailIDs.GetNextAssoc(det, detID, dontcare);
										GrowFastConcat(ans, _T("<det ID=\"%li\" />"), detID);
									}
									GrowFastConcat(ans, 4, _T("</c>"));
								} else {
									GrowFastConcat(ans, 2, _T("/>"));
								}
							}
							// Either way, there's no point in continuing our inner loop (we already found this match) so break out
							break;
						}
					}
				}
			}
		}
		GrowFastConcat(ans, 11, _T("</tochange>"));
	}

	// Append any namedsets that need to be added
	{
		GrowFastConcat(ans, 7, _T("<toadd>"));
		for (POSITION pos = mapCurrentPropSets.GetStartPosition(); pos != 0; ) {
			LONG nPropSetID;
			CApptPrototypePropertySet *pPropSetCur;
			mapCurrentPropSets.GetNextAssoc(pos, nPropSetID, pPropSetCur);
			// Find this one in the original propertyset map
			CApptPrototypePropertySet *pPropSetOrig;
			if (mapOrigPropSets.Lookup(nPropSetID, pPropSetOrig)) {
				CArray<CApptPrototypePropertySetNamedSet, CApptPrototypePropertySetNamedSet &> &origPropSetNamedSets = pPropSetOrig->GetNamedSets(ensodtSetType);
				CArray<CApptPrototypePropertySetNamedSet, CApptPrototypePropertySetNamedSet &> &curPropSetNamedSets = pPropSetCur->GetNamedSets(ensodtSetType);
				// Found the matching one in the original data, so see if our current one has any namedsets that it doesn't have
				for (INT_PTR iCur=0, countCur = curPropSetNamedSets.GetSize(); iCur<countCur; iCur++) {
					CApptPrototypePropertySetNamedSet &namedSetCur = curPropSetNamedSets.GetAt(iCur);
					if (namedSetCur.m_nID < 0) {
						// New, so it needs to be added to data
						bAppendedXml = true;
						GrowFastConcat(ans, _T("<a SID=\"%li\" ID=\"%li\" N=\"%s\">"), pPropSetCur->m_nID, namedSetCur.m_nID, ConvertToQuotableXMLString(namedSetCur.m_strName));
						for (POSITION det = namedSetCur.m_mapdwDetailIDs.GetStartPosition(); det != 0; ) {
							LONG detID;
							BYTE dontcare;
							namedSetCur.m_mapdwDetailIDs.GetNextAssoc(det, detID, dontcare);
							GrowFastConcat(ans, _T("<det ID=\"%li\" />"), detID);
						}
						GrowFastConcat(ans, 4, _T("</a>"));
					}
				}
			}
		}
		GrowFastConcat(ans, 8, _T("</toadd>"));
	}

	// If anything was appended, then close the root element and return it, otherwise return empty string 
	if (bAppendedXml) {
		GrowFastConcat(ans, 7, _T("</root>"));
		return ans;
	} else {
		return _T("");
	}
}

BOOL SaveApptPrototype_AddToBatch_UpdateAllChangedPropertySetNamedSets(CWnd *pMsgParent, IN OUT CParamSqlBatch &sql, const CApptPrototypePropertySetMap &mapCurrentPropSets, const CApptPrototypePropertySetMap &mapOrigPropSets, ENamedSetOfDetailsType::_Enum ensodtSetType)
{
	CString strNamedSetActualName = ENamedSetOfDetailsType::GetTableNamePrefix(ensodtSetType);

	// Prepare the xml
	CString xml = CalcPropertySetDiffs_ChangedNamedSets(mapCurrentPropSets, mapOrigPropSets, ensodtSetType);
	sql.Add(_T("IF ({BIT} = 1) BEGIN \r\n"), !xml.IsEmpty());
	// Open the xml and use it to update the data
	sql.Add(_T("EXEC sp_xml_preparedocument @hDoc OUTPUT, {VT_BSTR} \r\n"), NullIf(xml, _T("")));
	// Delete any that need to be deleted
	{
		sql.Declare(_T("DECLARE @tDelete") + strNamedSetActualName + _T("Sets TABLE (SetID INT NOT NULL) \r\n"));
		sql.Add(
			_T("INSERT INTO @tDelete") + strNamedSetActualName + _T("Sets (SetID) \r\n")
			_T("SELECT SetID \r\n")
			_T("FROM OPENXML(@hDoc, '/root/todelete/d') WITH (SetID INT '@ID') \r\n")
			);
		sql.Add(
			_T("DELETE FROM ApptPrototypePropertySet") + strNamedSetActualName + _T("SetDetailT \r\n")
			_T("WHERE ApptPrototypePropertySet") + strNamedSetActualName + _T("SetID IN (SELECT D.SetID FROM @tDelete") + strNamedSetActualName + _T("Sets D)\r\n")
			);
		sql.Add(
			_T("DELETE FROM ApptPrototypePropertySet") + strNamedSetActualName + _T("SetT \r\n")
			_T("WHERE ID IN (SELECT D.SetID FROM @tDelete") + strNamedSetActualName + _T("Sets D)\r\n")
			);
	}
	// Update the names and details of any that need to be changed
	{
		// Update the names of any changed namedsets
		sql.Declare(_T("DECLARE @tChange") + strNamedSetActualName + _T("Sets TABLE (SetID INT NOT NULL, NewName NVARCHAR(200), ChangedDetails BIT NOT NULL) \r\n"));
		sql.Add(
			_T("INSERT INTO @tChange") + strNamedSetActualName + _T("Sets (SetID, NewName, ChangedDetails) \r\n")
			_T("SELECT C.SetID, C.Name, ChangedDetails \r\n")
			_T("FROM OPENXML(@hDoc, '/root/tochange/c') WITH (SetID INT '@ID', Name NVARCHAR(200) '@N', ChangedDetails BIT '@cd') C \r\n")
			);
		sql.Add(
			_T("UPDATE ApptPrototypePropertySet") + strNamedSetActualName + _T("SetT SET Name = C.NewName \r\n")
			_T("FROM ApptPrototypePropertySet") + strNamedSetActualName + _T("SetT T \r\n")
			_T("INNER JOIN @tChange") + strNamedSetActualName + _T("Sets C ON T.ID = C.SetID \r\n")
			_T("WHERE C.NewName IS NOT NULL \r\n")
			);
		// Remove and re-add the details of any changed namedsets
		{
			// Remove
			sql.Add(
				_T("DELETE ApptPrototypePropertySet") + strNamedSetActualName + _T("SetDetailT \r\n")
				_T("FROM ApptPrototypePropertySet") + strNamedSetActualName + _T("SetDetailT T \r\n")
				_T("INNER JOIN @tChange") + strNamedSetActualName + _T("Sets C ON T.ApptPrototypePropertySet") + strNamedSetActualName + _T("SetID = C.SetID \r\n")
				_T("WHERE C.ChangedDetails = 1 \r\n")
				);
			// Re-add
			sql.Add(
				_T("INSERT INTO ApptPrototypePropertySet") + strNamedSetActualName + _T("SetDetailT (ApptPrototypePropertySet") + strNamedSetActualName + _T("SetID, ") + strNamedSetActualName + _T("ID) \r\n")
				_T("SELECT SetID, DetailID \r\n")
				_T("FROM OPENXML(@hDoc, '/root/tochange/c[@cd=1]/det') WITH (SetID INT '../@ID', DetailID INT '@ID') A \r\n")
				);
		}
	}
	// Insert any new ones
	{
		// Load the staging records
		sql.Declare(_T("DECLARE @tAddPropertySet") + strNamedSetActualName + _T("Sets TABLE (RealID INT, PseudoID INT NOT NULL, Name NVARCHAR(200) NOT NULL, PropertySetID INT NOT NULL) \r\n"));
		sql.Add(
			_T("INSERT INTO @tAddPropertySet") + strNamedSetActualName + _T("Sets \r\n")
			_T("SELECT NULL, PseudoID, Name, PropertySetID \r\n")
			_T("FROM OPENXML(@hDoc, '/root/toadd/a') WITH (PseudoID INT '@ID', Name NVARCHAR(200) '@N', PropertySetID INT '@SID') T \r\n")
			);
		sql.Declare(_T("DECLARE @tAddPropertySet") + strNamedSetActualName + _T("SetDetails TABLE (SetPseudoID INT NOT NULL, SetRealID INT, DetailID INT NOT NULL) \r\n"));
		sql.Add(
			_T("INSERT INTO @tAddPropertySet") + strNamedSetActualName + _T("SetDetails \r\n")
			_T("SELECT SetPseudoID, NULL, DetailID \r\n")
			_T("FROM OPENXML(@hDoc, '/root/toadd/a/det') WITH (SetPseudoID INT '../@ID', DetailID INT '@ID') T \r\n")
			);
		// Add real base records
		sql.Add(
			_T("INSERT INTO ApptPrototypePropertySet") + strNamedSetActualName + _T("SetT (Name, ApptPrototypePropertySetID) \r\n")
			_T("SELECT T.Name, T.PropertySetID \r\n")
			_T("FROM @tAddPropertySet") + strNamedSetActualName + _T("Sets T \r\n")
			);
		// Reflect real IDs in staging tables
		{
			// Base staging table
			sql.Add(
				_T("UPDATE @tAddPropertySet") + strNamedSetActualName + _T("Sets SET RealID = PSAPS.ID \r\n")
				_T("FROM @tAddPropertySet") + strNamedSetActualName + _T("Sets T \r\n")
				_T("INNER JOIN ApptPrototypePropertySet") + strNamedSetActualName + _T("SetT PSAPS ON T.Name = PSAPS.Name AND T.PropertySetID = PSAPS.ApptPrototypePropertySetID \r\n") // reliable thanks to the uniqueness constraint on ApptPrototypePropertySetAptPurposeSetT (ApptPrototypePropertySetID+Name) and similar on the resource-related table
				);
			// Details staging table
			sql.Add(
				_T("UPDATE @tAddPropertySet") + strNamedSetActualName + _T("SetDetails SET SetRealID = SPS.RealID \r\n")
				_T("FROM @tAddPropertySet") + strNamedSetActualName + _T("SetDetails T INNER JOIN @tAddPropertySet") + strNamedSetActualName + _T("Sets SPS ON T.SetPseudoID = SPS.PseudoID \r\n"));
		}
		// Add real detail records
		sql.Add(
			_T("INSERT INTO ApptPrototypePropertySet") + strNamedSetActualName + _T("SetDetailT (ApptPrototypePropertySet") + strNamedSetActualName + _T("SetID, ") + strNamedSetActualName + _T("ID) \r\n")
			_T("SELECT T.SetRealID, T.DetailID \r\n")
			_T("FROM @tAddPropertySet") + strNamedSetActualName + _T("SetDetails T \r\n")
			);
	}
	sql.Add(_T("EXEC sp_xml_removedocument @hDoc \r\n"));
	sql.Add(_T("END \r\n"));
	return TRUE;
}

BOOL SaveApptPrototype_AddToBatch_UpdateAllChangedPropertySetNamedObjects(CWnd *pMsgParent, IN OUT CParamSqlBatch &sql, const CApptPrototypePropertySetMap &mapCurrentPropSets, const CApptPrototypePropertySetMap &mapOrigPropSets, ENamedObjectsType::_Enum enotType)
{
	CString strNamedObjectActualName = ENamedObjectsType::GetTableNamePrefix(enotType);

	// Prepare the xml
	CString xml = CalcPropertySetDiffs_ChangedNamedObjects(mapCurrentPropSets, mapOrigPropSets, enotType);
	sql.Add(_T("IF ({BIT} = 1) BEGIN \r\n"), !xml.IsEmpty());
	// Open the xml and use it to update the data
	sql.Add(_T("EXEC sp_xml_preparedocument @hDoc OUTPUT, {VT_BSTR} \r\n"), NullIf(xml, _T("")));
	// Delete any that need to be deleted
	sql.Add(
		_T("DELETE ApptPrototypePropertySet") + strNamedObjectActualName + _T("T \r\n")
		_T("FROM ApptPrototypePropertySet") + strNamedObjectActualName + _T("T T \r\n")
		_T("INNER JOIN OPENXML(@hDoc, '/root/todelete/d') WITH (PropertySetID INT '@SID', ID INT '@ID') D ON T.ApptPrototypePropertySetID = D.PropertySetID AND T.") + strNamedObjectActualName + _T("ID = D.ID \r\n")
		);
	// Insert any new ones
	sql.Add(
		_T("INSERT INTO ApptPrototypePropertySet") + strNamedObjectActualName + _T("T (ApptPrototypePropertySetID, ") + strNamedObjectActualName + _T("ID) \r\n")
		_T("SELECT PropertySetID, ID \r\n")
		_T("FROM OPENXML(@hDoc, '/root/toadd/a') WITH (PropertySetID INT '@SID', ID INT '@ID') A \r\n")
		);
	sql.Add(_T("EXEC sp_xml_removedocument @hDoc \r\n"));
	sql.Add(_T("END \r\n"));

	return TRUE;
}

// Fills the output sql object with a static query, and adds all the dynamic parameter values to it
// NOTE: For efficiency the sql statement resulting from this function is ALWAYS THE SAME regardless of the state of the parameters.  
BOOL UpdateExistingApptPrototype(IN OUT CParamSqlBatch &sql, CWnd *pMsgParent, INT appointmentPrototypeID, const CString &strNewName, const CString &strOriginalName, const CString &strNewDescription, const CString &strOriginalDescription, const CApptPrototypePropertySetMap &mapCurrentPropSets, const CApptPrototypePropertySetMap &mapOrigPropSets)
{
	sql.Add(_T("SET NOCOUNT ON \r\n"));
	sql.Declare(_T("DECLARE @prototypeID INT \r\n"));
	sql.Add(_T("SET @prototypeID = {INT} \r\n"), appointmentPrototypeID);
	sql.Declare(_T("DECLARE @hDoc AS INT \r\n"));

	// Update the prototype name and description
	{
		// Declare and fill the parameters that we need more than once so that we only have to pass them in once
		{
			sql.Declare(_T("DECLARE @newName NVARCHAR(200), @newDescription NVARCHAR(2000) \r\n"));
			sql.Add(_T("SET @newName = {VT_BSTR} \r\n"), NullIf(strNewName, strOriginalName));
			sql.Add(_T("SET @newDescription = {VT_BSTR} \r\n"), NullIf(strNewDescription, strOriginalDescription));
		}

		// Update the name and description
		sql.Add(
			_T("IF (@newName IS NOT NULL OR @newDescription IS NOT NULL) BEGIN \r\n")
			_T("  UPDATE ApptPrototypeT SET \r\n")
			_T("   Name = COALESCE(@newName, Name), \r\n")
			_T("   Description = COALESCE(@newDescription, Description) \r\n")
			_T("  WHERE ID = @prototypeID \r\n")
			_T("END \r\n")
			);
	}

	// Delete any propertysets being deleted
	{
		CArray<long,long> arynIDsToDelete;
		CalcPropertySetDiffs_DeletedPropertySets(mapCurrentPropSets, mapOrigPropSets, &arynIDsToDelete);
		sql.Declare(_T("DECLARE @tPropertySetsToDelete TABLE (ID INT NOT NULL) \r\n"));
		sql.Add(_T("INSERT INTO @tPropertySetsToDelete (ID) SELECT A.ID FROM ApptPrototypePropertySetT A WHERE A.ID IN ({INTARRAY}) \r\n"), arynIDsToDelete);
		sql.Add(
			_T("IF (EXISTS (SELECT * FROM @tPropertySetsToDelete)) BEGIN \r\n")
			_T("  DELETE FROM ApptPrototypePropertySetAptPurposeSetDetailT WHERE ApptPrototypePropertySetAptPurposeSetID IN (SELECT B.ID FROM ApptPrototypePropertySetAptPurposeSetT B WHERE B.ApptPrototypePropertySetID IN (SELECT A.ID FROM @tPropertySetsToDelete A)) \r\n"));
		sql.Add(
			_T("  DELETE FROM ApptPrototypePropertySetAptPurposeSetT WHERE ApptPrototypePropertySetID IN (SELECT A.ID FROM @tPropertySetsToDelete A) \r\n"));
		sql.Add(
			_T("  DELETE FROM ApptPrototypePropertySetResourceSetDetailT WHERE ApptPrototypePropertySetResourceSetID IN (SELECT B.ID FROM ApptPrototypePropertySetResourceSetT B WHERE B.ApptPrototypePropertySetID IN (SELECT A.ID FROM @tPropertySetsToDelete A)) \r\n"));
		sql.Add(
			_T("  DELETE FROM ApptPrototypePropertySetResourceSetT WHERE ApptPrototypePropertySetID IN (SELECT A.ID FROM @tPropertySetsToDelete A) \r\n"));
		sql.Add(
			_T("  DELETE FROM ApptPrototypePropertySetAptTypeT WHERE ApptPrototypePropertySetID IN (SELECT A.ID FROM @tPropertySetsToDelete A) \r\n"));
		sql.Add(
			_T("  DELETE FROM ApptPrototypePropertySetLocationT WHERE ApptPrototypePropertySetID IN (SELECT A.ID FROM @tPropertySetsToDelete A) \r\n"));
		sql.Add(
			_T("  DELETE FROM ApptPrototypePropertySetT WHERE ID IN (SELECT A.ID FROM @tPropertySetsToDelete A) \r\n")
			_T("END \r\n")
			);
	}

	// Update any propertysets that have changed
	{
		// Update the name and other root properties in any propertysets that changed
		{
			// Prepare the xml
			CString strOpenXmlSpec;
			CString xml = CalcPropertySetDiffs_ChangedRootProperties(mapCurrentPropSets, mapOrigPropSets, &strOpenXmlSpec);
			// Open the xml and use it to update the data
			sql.Add(_T("EXEC sp_xml_preparedocument @hDoc OUTPUT, {VT_BSTR} \r\n"), NullIf(xml, _T("")));
			sql.Add(
				_T("UPDATE ApptPrototypePropertySetT SET \r\n")
				_T(" Name = COALESCE(T.Name, APPS.Name), \r\n")
				_T(" AppointmentDuration = COALESCE(T.AppointmentDuration, APPS.AppointmentDuration), \r\n")
				_T(" AllowTimeEarlyMorning = COALESCE(T.AllowTimeEarlyMorning, APPS.AllowTimeEarlyMorning), \r\n")
				_T(" AllowTimeLateMorning = COALESCE(T.AllowTimeLateMorning, APPS.AllowTimeLateMorning), \r\n")
				_T(" AllowTimeEarlyAfternoon = COALESCE(T.AllowTimeEarlyAfternoon, APPS.AllowTimeEarlyAfternoon), \r\n")
				_T(" AllowTimeLateAfternoon = COALESCE(T.AllowTimeLateAfternoon, APPS.AllowTimeLateAfternoon), \r\n")
				_T(" AllowDaySunday = COALESCE(T.AllowDaySunday, APPS.AllowDaySunday), \r\n")
				_T(" AllowDayMonday = COALESCE(T.AllowDayMonday, APPS.AllowDayMonday), \r\n")
				_T(" AllowDayTuesday = COALESCE(T.AllowDayTuesday, APPS.AllowDayTuesday), \r\n")
				_T(" AllowDayWednesday = COALESCE(T.AllowDayWednesday, APPS.AllowDayWednesday), \r\n")
				_T(" AllowDayThursday = COALESCE(T.AllowDayThursday, APPS.AllowDayThursday), \r\n")
				_T(" AllowDayFriday = COALESCE(T.AllowDayFriday, APPS.AllowDayFriday), \r\n")
				_T(" AllowDaySaturday = COALESCE(T.AllowDaySaturday, APPS.AllowDaySaturday) \r\n")
				_T("FROM ApptPrototypePropertySetT APPS \r\n")
				_T("INNER JOIN OPENXML(@hDoc, '/root/r') WITH (") + strOpenXmlSpec + _T(") T ON APPS.ID = T.ID \r\n")
				);
			sql.Add(_T("EXEC sp_xml_removedocument @hDoc \r\n"));
		}

		// Update types
		if (!SaveApptPrototype_AddToBatch_UpdateAllChangedPropertySetNamedObjects(pMsgParent, sql, mapCurrentPropSets, mapOrigPropSets, ENamedObjectsType::AptType)) {
			return FALSE;
		}

		// Update purposesets
		if (!SaveApptPrototype_AddToBatch_UpdateAllChangedPropertySetNamedSets(pMsgParent, sql, mapCurrentPropSets, mapOrigPropSets, ENamedSetOfDetailsType::AptPurposeSet)) {
			return FALSE;
		}

		// Update resourcesets
		if (!SaveApptPrototype_AddToBatch_UpdateAllChangedPropertySetNamedSets(pMsgParent, sql, mapCurrentPropSets, mapOrigPropSets, ENamedSetOfDetailsType::ResourceSet)) {
			return FALSE;
		}

		// Update locations
		if (!SaveApptPrototype_AddToBatch_UpdateAllChangedPropertySetNamedObjects(pMsgParent, sql, mapCurrentPropSets, mapOrigPropSets, ENamedObjectsType::Location)) {
			return FALSE;
		}
	}

	// Insert any propertysets being inserted
	if (!SaveApptPrototype_AddToBatch_InsertAllNewPropertySets(pMsgParent, sql, mapCurrentPropSets)) {
		return FALSE;
	}

	sql.Add(_T("SET NOCOUNT OFF \r\n"));

	// Finally simply select the prototype ID which the caller needs
	sql.Add(_T("SELECT @prototypeID \r\n"));
	
	return TRUE;
}

BOOL CreateNewApptPrototype(IN OUT CParamSqlBatch &sql, CWnd *pMsgParent, const CString &strNewName, const CString &strNewDescription, const CApptPrototypePropertySetMap &mapCurrentPropSets)
{
	sql.Add(_T("SET NOCOUNT ON \r\n"));
	sql.Declare(_T("DECLARE @prototypeID INT \r\n"));
	sql.Declare(_T("DECLARE @hDoc AS INT \r\n"));

	// Create the prototype record
	{
		sql.Add(_T("INSERT INTO ApptPrototypeT (Name, Description) VALUES ({STRING}, {STRING}) \r\n"), strNewName, strNewDescription);
		sql.Add(_T("SET @prototypeID = SCOPE_IDENTITY() \r\n"));
	}

	// Insert all the prototype propertysets
	if (!SaveApptPrototype_AddToBatch_InsertAllNewPropertySets(pMsgParent, sql, mapCurrentPropSets)) {
		return FALSE;
	}

	sql.Add(_T("SET NOCOUNT OFF \r\n"));

	// Finally simply select the prototype ID which the caller needs
	sql.Add(_T("SELECT @prototypeID \r\n"));
	
	return TRUE;
}

BOOL CApptPrototypeEntryDlg::Save()
{
	CString strNewName, strNewDescription;
	GetDlgItemText(IDC_NAME_EDIT, strNewName);
	strNewName.Trim();
	GetDlgItemText(IDC_DESCRIPTION_EDIT, strNewDescription);
	strNewDescription.Trim();

	// Pick the appropriate query and add all the correct parameter values
	CParamSqlBatch sql;
	if (m_appointmentPrototypeID != -1) {
		// Use the "update existing" query
		if (!UpdateExistingApptPrototype(sql, this, m_appointmentPrototypeID, strNewName, m_strOriginalName, strNewDescription, m_strOriginalDescription, m_mapCurrentPropSets, m_mapOrigPropSets)) {
			return FALSE;
		}
	} else {
		// Use the "create new" query
		if (!CreateNewApptPrototype(sql, this, strNewName, strNewDescription, m_mapCurrentPropSets)) {
			return FALSE;
		}
	}

	// We've got the correct query and parameters, run it
	_RecordsetPtr prs = sql.CreateRecordset(GetRemoteData());

	// In theory now is the time to take the changes we've just committed and reflect them in our 
	// member variables.  But for now since we don't have an "Apply" concept there's really no need 
	// for that so we're not going to bother parsing everything back out of the returned recordsets 
	// etc.  BUT, our caller does happen to need to know the new name, description and ID after we 
	// save, so for convenience we're going to reflect those.
	{
		m_appointmentPrototypeID = AdoFldLong(prs->GetFields()->GetItem((long)0));
		m_strOriginalName = strNewName;
		m_strOriginalDescription = strNewDescription;
	}

	return TRUE;
}

void CApptPrototypeEntryDlg::OnBnClickedOk()
{
	try {
		// Basic validation
		if (Validate()) {
			// Save
			if (Save()) {
				CNxDialog::OnOK();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

CApptPrototypePropertySet *CApptPrototypeEntryDlg::GetPropertySetFromRow(LPDISPATCH lpRow)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
	CApptPrototypePropertySet *pPropSet = m_mapCurrentPropSets[VarLong(pRow->GetValue(EPropertySetListColumns::ID))];
	if (pPropSet != NULL) {
		return pPropSet;
	} else {
		ThrowNxException(_T("Could not find property set associated with the selected row!"));
	}
}

void CApptPrototypeEntryDlg::OnChangePropertySetColumn_Available(LPDISPATCH lpRow)
{
	try {
		CApptPrototypePropertySet *pPropSet = GetPropertySetFromRow(lpRow);
		CApptPrototypePropertySetAvailabilityDlg dlg(this);
		dlg.m_nAvailDays = pPropSet->m_nAvailDays;
		dlg.m_nAvailTimes = pPropSet->m_nAvailTimes;
		if (dlg.DoModal() == IDOK) {
			pPropSet->m_nAvailDays = dlg.m_nAvailDays;
			pPropSet->m_nAvailTimes = dlg.m_nAvailTimes;
			NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
			pRow->PutValue(EPropertySetListColumns::Available, AsVariant(pPropSet->CalcAvailabilityText()));
			_DNxDataListPtr pdl = GetDlgItem(IDC_APPTPROTOTYPE_PROPERTYSETS_LIST)->GetControlUnknown();
			pdl->CalcColumnWidthFromData(EPropertySetListColumns::Available, VARIANT_FALSE, VARIANT_TRUE);
		}
	} NxCatchAll(__FUNCTION__);
}

void CApptPrototypeEntryDlg::OnChangePropertySetColumn_Types(LPDISPATCH lpRow)
{
	try {
		CApptPrototypePropertySet *pPropSet = GetPropertySetFromRow(lpRow);

		// Figure out which types are to be selected
		CVariantArray selected;
		{
			for (INT_PTR i=0, nCount=pPropSet->m_aryAptTypes.GetSize(); i<nCount; i++) {
				CApptPrototypePropertySetAptType &type = pPropSet->m_aryAptTypes.GetAt(i);
				selected.Add(_variant_t(type.m_nID));
			}
		}

		// Now prompt the user to update the selections
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "AptTypeT");
		dlg.PreSelect(selected);
		if (dlg.Open(_T("AptTypeT"), _T("Inactive = 0 OR ID IN (") + ArrayAsString(selected) + _T(",-1)"), _T("ID"), _T("Name"), _T("Please select the allowed Appointment Types")) == IDOK) {
			// Store the user's new selection(s) back to our set
			dlg.FillArrayWithIDs(selected);
			CVariantArray names;
			dlg.FillArrayWithNames(names);
			pPropSet->m_aryAptTypes.RemoveAll();
			for (INT_PTR i=0, nCount=selected.GetSize(); i<nCount; i++) {
				pPropSet->m_aryAptTypes.Add(CApptPrototypePropertySetAptType(VarLong(selected.GetAt(i)), VarString(names.GetAt(i))));
			}
			// And reflect the new selection(s) on screen
			IRowSettingsPtr pRow = lpRow;
			pRow->PutValue(EPropertySetListColumns::Types, AsVariant(pPropSet->CalcAptTypesText()));
		}

	} NxCatchAll(__FUNCTION__);
}

void CApptPrototypeEntryDlg::OnChangePropertySetColumn_Locations(LPDISPATCH lpRow)
{
	try {
		CApptPrototypePropertySet *pPropSet = GetPropertySetFromRow(lpRow);

		// Figure out which locations are to be selected
		CVariantArray selected;
		{
			for (INT_PTR i=0, nCount=pPropSet->m_aryLocations.GetSize(); i<nCount; i++) {
				CApptPrototypePropertySetLocation &location = pPropSet->m_aryLocations.GetAt(i);
				selected.Add(_variant_t(location.m_nID));
			}
		}

		// Now prompt the user to update the selections
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "LocationsT");
		dlg.PreSelect(selected);
		// (b.cardillo 2011-03-02 09:28) - Still for PLID 40419 I've changed this to allow any general location, not just managed.
		if (dlg.Open(_T("LocationsT"), _T("TypeID = 1 AND Active = 1 OR ID IN (") + ArrayAsString(selected) + _T(",-1)"), _T("ID"), _T("Name"), _T("Please select the allowed Locations")) == IDOK) {
			// Store the user's new selection(s) back to our set
			dlg.FillArrayWithIDs(selected);
			CVariantArray names;
			dlg.FillArrayWithNames(names);
			pPropSet->m_aryLocations.RemoveAll();
			for (INT_PTR i=0, nCount=selected.GetSize(); i<nCount; i++) {
				pPropSet->m_aryLocations.Add(CApptPrototypePropertySetLocation(VarLong(selected.GetAt(i)), VarString(names.GetAt(i))));
			}
			// And reflect the new selection(s) on screen
			IRowSettingsPtr pRow = lpRow;
			pRow->PutValue(EPropertySetListColumns::Locations, AsVariant(pPropSet->CalcLocationsText()));
		}

	} NxCatchAll(__FUNCTION__);
}

void CApptPrototypeEntryDlg::OnChangePropertySetColumn_PurposeSets(LPDISPATCH lpRow)
{
	try {
		CApptPrototypePropertySet *pPropSet = GetPropertySetFromRow(lpRow);
		CApptPrototypePropertySetNamedSetDlg dlg(this);
		// (b.cardillo 2011-02-26 12:46) - PLID 40419 - Respect the procedure's inactive status for procedural purposes
		if (dlg.DoModal("Appointment Purpose", pPropSet->m_aryAptPurposeSets, m_nNextNegativeNamedSetID, _T("SELECT AP.ID, AP.Name, CONVERT(BIT, COALESCE(Pr.Inactive, 0)) AS Inactive FROM AptPurposeT AP LEFT JOIN ProcedureT Pr ON AP.ID = Pr.ID"))) {
			m_nNextNegativeNamedSetID = dlg.GetNextNegativeID();
			NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
			pRow->PutValue(EPropertySetListColumns::PurposeSets, AsBstr(pPropSet->CalcAptPurposeSetsText()));
		}
	} NxCatchAll(__FUNCTION__);
}

void CApptPrototypeEntryDlg::OnChangePropertySetColumn_ResourceSets(LPDISPATCH lpRow)
{
	try {
		CApptPrototypePropertySet *pPropSet = GetPropertySetFromRow(lpRow);
		CApptPrototypePropertySetNamedSetDlg dlg(this);
		if (dlg.DoModal("Resource", pPropSet->m_aryResourceSets, m_nNextNegativeNamedSetID, _T("SELECT ID, Item AS Name, Inactive FROM ResourceT"))) {
			m_nNextNegativeNamedSetID = dlg.GetNextNegativeID();
			NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
			pRow->PutValue(EPropertySetListColumns::ResourceSets, AsBstr(pPropSet->CalcResourceSetsText()));
		}
	} NxCatchAll(__FUNCTION__);
}


BEGIN_EVENTSINK_MAP(CApptPrototypeEntryDlg, CNxDialog)
	ON_EVENT(CApptPrototypeEntryDlg, IDC_APPTPROTOTYPE_PROPERTYSETS_LIST, 19, CApptPrototypeEntryDlg::LeftClickApptprototypePropertysetsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CApptPrototypeEntryDlg, IDC_APPTPROTOTYPE_PROPERTYSETS_LIST, 9, CApptPrototypeEntryDlg::EditingFinishingApptprototypePropertysetsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CApptPrototypeEntryDlg, IDC_APPTPROTOTYPE_PROPERTYSETS_LIST, 10, CApptPrototypeEntryDlg::EditingFinishedApptprototypePropertysetsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CApptPrototypeEntryDlg, IDC_APPTPROTOTYPE_PROPERTYSETS_LIST, 2, CApptPrototypeEntryDlg::SelChangedApptprototypePropertysetsList, VTS_DISPATCH VTS_DISPATCH)
END_EVENTSINK_MAP()

void CApptPrototypeEntryDlg::LeftClickApptprototypePropertysetsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if (lpRow != NULL) {
			switch (nCol) {
			case EPropertySetListColumns::Available:
				OnChangePropertySetColumn_Available(lpRow);
				break;
			case EPropertySetListColumns::Types:
				OnChangePropertySetColumn_Types(lpRow);
				break;
			case EPropertySetListColumns::PurposeSets:
				OnChangePropertySetColumn_PurposeSets(lpRow);
				break;
			case EPropertySetListColumns::ResourceSets:
				OnChangePropertySetColumn_ResourceSets(lpRow);
				break;
			case EPropertySetListColumns::Locations:
				OnChangePropertySetColumn_Locations(lpRow);
				break;
			default:
				break;
			}
		}
	} NxCatchAll(__FUNCTION__);// Kind of nonsense to have this here since each of the above sub-handlers has its own try/catch, but just to be safe...
}

LONG CApptPrototypeEntryDlg::GetCommittedPrototypeID()
{
	return m_appointmentPrototypeID;
}

CString CApptPrototypeEntryDlg::GetCommittedPrototypeName()
{
	return m_strOriginalName;
}

CString CApptPrototypeEntryDlg::GetCommittedPrototypeDescription()
{
	return m_strOriginalDescription;
}

void CApptPrototypeEntryDlg::OnBnClickedAddBtn()
{
	try {
		_DNxDataListPtr pdl = GetDlgItem(IDC_APPTPROTOTYPE_PROPERTYSETS_LIST)->GetControlUnknown();
		if (m_mapCurrentPropSets.PLookup(m_nextNegativeID) == NULL) {
			CApptPrototypePropertySet *pNewPropertySet = new CApptPrototypePropertySet();
			try {
				// Set the new id
				pNewPropertySet->m_nID = m_nextNegativeID;

				// Try to find a unique new name
				_variant_t varNewName;
				CString strNewName;
				{
					for (LONG n=1; n<10000; n++) {
						strNewName.Format(_T("New %li"), n);
						varNewName = AsVariant(strNewName);
						if (pdl->FindByColumn(EPropertySetListColumns::Name, varNewName, NULL, VARIANT_FALSE) == NULL) {
							break;
						}
					}
				}
				pNewPropertySet->m_strName = strNewName;

				// Add this entry to the list on screen
				IRowSettingsPtr pRow = pdl->GetNewRow();
				pRow->PutValue(EPropertySetListColumns::ID, pNewPropertySet->m_nID);
				pRow->PutValue(EPropertySetListColumns::Name, varNewName);
				pRow->PutValue(EPropertySetListColumns::Duration, pNewPropertySet->m_nAppointmentDuration);
				pRow->PutValue(EPropertySetListColumns::Available, AsVariant(pNewPropertySet->CalcAvailabilityText()));
				pRow->PutValue(EPropertySetListColumns::Types, AsVariant(pNewPropertySet->CalcAptTypesText()));
				pRow->PutValue(EPropertySetListColumns::PurposeSets, AsVariant(pNewPropertySet->CalcAptPurposeSetsText()));
				pRow->PutValue(EPropertySetListColumns::ResourceSets, AsVariant(pNewPropertySet->CalcResourceSetsText()));
				pRow->PutValue(EPropertySetListColumns::Locations, AsVariant(pNewPropertySet->CalcLocationsText()));
				pRow = pdl->AddRowSorted(pRow, NULL);

				// We've added it on screen so store it in our local memory so we know to save it to data when the user hits ok
				m_mapCurrentPropSets.SetAt(m_nextNegativeID, pNewPropertySet);
				m_nextNegativeID--;

				// Adjust to fit
				pdl->CalcColumnWidthFromData(EPropertySetListColumns::Name, VARIANT_FALSE, VARIANT_TRUE);
				pdl->CalcColumnWidthFromData(EPropertySetListColumns::Duration, VARIANT_FALSE, VARIANT_TRUE);
				pdl->CalcColumnWidthFromData(EPropertySetListColumns::Available, VARIANT_FALSE, VARIANT_TRUE);

				// Make sure it's selected, and start editing it since the user is obviously going to want to rename it
				pdl->PutCurSel(pRow);
				GetDlgItem(IDC_DELETE_BTN)->EnableWindow(TRUE);
				pdl->StartEditing(pRow, EPropertySetListColumns::Name);
			} catch (...) {
				delete pNewPropertySet;
				pNewPropertySet = NULL;
				throw;
			}
		} else {
			ThrowNxException(_T("Cannot add new entry with id '%li' because one already exists."), m_nextNegativeID);
		}
	} NxCatchAll(__FUNCTION__);
}

void CApptPrototypeEntryDlg::OnBnClickedDeleteBtn()
{
	try {
		_DNxDataListPtr pdl = GetDlgItem(IDC_APPTPROTOTYPE_PROPERTYSETS_LIST)->GetControlUnknown();
		pdl->StopEditing(VARIANT_FALSE);
		IRowSettingsPtr row = pdl->GetCurSel();
		if (row != NULL) {
			// Get the ID
			LONG nPropertySetID = VarLong(row->GetValue(EPropertySetListColumns::ID));
			
			// (b.cardillo 2011-02-28 16:19) - Fixed memory leak in PLID 40419
			// Remove the key and free the memory 
			{
				// Find it in the map
				CApptPrototypePropertySet *pDyingPropertySet;
				if (m_mapCurrentPropSets.Lookup(nPropertySetID, pDyingPropertySet)) {
					if (pDyingPropertySet != NULL) {
						// Remove the key
						if (m_mapCurrentPropSets.RemoveKey(nPropertySetID)) {
							// Free the memory
							delete pDyingPropertySet;
						} else {
							ThrowNxException(_T("The selected property set with ID %li could not be removed from memory."), nPropertySetID);
						}
					} else {
						ThrowNxException(_T("The selected property set with ID %li could not be freed."), nPropertySetID);
					}
				} else {
					ThrowNxException(_T("The selected property set with ID %li could not be found in memory."), nPropertySetID);
				}
			}

			// Remove the row from on screen
			pdl->RemoveRow(row);

			// Reflect enabled state
			ReflectEnabledControls();

		} else {
			MessageBox(_T("Please select the property set you want to delete."), NULL, MB_OK|MB_ICONEXCLAMATION);
		}
	} NxCatchAll(__FUNCTION__);
}

void CApptPrototypeEntryDlg::EditingFinishingApptprototypePropertysetsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		if ((pbCommit != NULL) && (*pbCommit) && (pbContinue != NULL) && (*pbContinue)) {
			if (lpRow != NULL) {
				if (nCol == EPropertySetListColumns::Name) {
					// Validate the new name
					CString strNewValue = strUserEntered;
					strNewValue.Trim();
					{
						// Make sure the name is not empty
						if (strNewValue.IsEmpty()) {
							MessageBox(_T("Please enter a name for this property set."), _T("Invalid name"), MB_OK|MB_ICONEXCLAMATION);
							*pbContinue = FALSE;
							return;
						}

						// Make sure the name is not too long
						if (strNewValue.GetLength() > 200) {
							MessageBox(_T("The name you entered is too long.  Please enter a name 200 characters or shorter."), _T("Invalid name"), MB_OK|MB_ICONEXCLAMATION);
							*pbContinue = FALSE;
							return;
						}

						// Make sure the name isn't already in use
						NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
						LONG nCurID = VarLong(pRow->GetValue(EPropertySetListColumns::ID));
						for (CApptPrototypePropertySetMap::CPair *p = m_mapCurrentPropSets.PGetFirstAssoc(); p != NULL; p = m_mapCurrentPropSets.PGetNextAssoc(p)) {
							if (p->key != nCurID) {
								if (p->value->m_strName.CompareNoCase(strNewValue) == 0) {
									MessageBox(_T("The name you entered is already in use by another property set.  Please enter a unique name."), _T("Invalid name"), MB_OK|MB_ICONEXCLAMATION);
									*pbContinue = FALSE;
									return;
								}
							}
						}

						// Reflect the new value we interpret it to be
						VariantClear(pvarNewValue);
						(*pvarNewValue) = AsVariant(strNewValue).Detach();
					}
				} else if (nCol == EPropertySetListColumns::Duration) {
					// Validate the new duration
					LONG nNewvalue = VarLong(*pvarNewValue);
					{
						// Make sure the duration is in range
						if (nNewvalue < 1 || nNewvalue > 1439) {
							MessageBox(_T("Please enter a duration in minutes for this property set of at least 1 minute and less than 24 hours."), _T("Invalid duration"), MB_OK|MB_ICONEXCLAMATION);
							*pbContinue = FALSE;
							return;
						}
					}
				} else {
					ThrowNxException(_T("Cannot edit a cell outside the Name and Duration columns."));
				}
			} else {
				ThrowNxException(_T("Cannot edit non-row."));
			}
		}
	} NxCatchAllCall(__FUNCTION__, { try { *pbContinue = FALSE; } NxCatchAllIgnore() } );
}

void CApptPrototypeEntryDlg::EditingFinishedApptprototypePropertysetsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		if (bCommit) {
			if (lpRow != NULL) {
				if (nCol == EPropertySetListColumns::Name || nCol == EPropertySetListColumns::Duration) {
					// Save the change in our class-local memory
					CApptPrototypePropertySet *pPropSet = GetPropertySetFromRow(lpRow);
					if (pPropSet != NULL) {
						switch (nCol) {
						case EPropertySetListColumns::Name:
							pPropSet->m_strName = VarString(varNewValue);
							break;
						case EPropertySetListColumns::Duration:
							pPropSet->m_nAppointmentDuration = VarLong(varNewValue);
							break;
						default:
							ThrowNxException(_T("Invalid column %hi."), nCol);
						}
					} else {
						ThrowNxException(_T("Cannot find property set associated with edited row."));
					}
				} else {
					ThrowNxException(_T("Cannot finish editing a cell outside the Name column."));
				}
			} else {
				ThrowNxException(_T("Cannot finish editing non-row."));
			}
		} else {
			// User canceled, do nothing
		}
	} NxCatchAll(__FUNCTION__);
}

void CApptPrototypeEntryDlg::ReflectEnabledControls(BOOL bIsRowSelected)
{
	GetDlgItem(IDC_DELETE_BTN)->EnableWindow(bIsRowSelected);
}

void CApptPrototypeEntryDlg::ReflectEnabledControls()
{
	BOOL bIsRowSelected;
	{
		_DNxDataListPtr pdl = GetDlgItem(IDC_APPTPROTOTYPE_PROPERTYSETS_LIST)->GetControlUnknown();
		if (pdl->GetCurSel() != NULL) {
			bIsRowSelected = TRUE;
		} else {
			bIsRowSelected = FALSE;
		}
	}

	ReflectEnabledControls(bIsRowSelected);
}


void CApptPrototypeEntryDlg::SelChangedApptprototypePropertysetsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		ReflectEnabledControls(lpNewSel != NULL);
	} NxCatchAll(__FUNCTION__);
}
