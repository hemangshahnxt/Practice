#include "stdafx.h"
#include "EMNDetailStructures.h"
#include "EMN.h"
#include "Mirror.h"
#include "InvVisionWebUtils.h"
#include "WoundCareCalculator.h"
#include "EMRTopic.h"
#include "EMRHotSpot.h"
#include "EMNDetail.h"
#include "NxImageCache.h"
#include "EmrItemAdvImageState.h"	// (j.armen 2014-07-21 16:32) - PLID 62836

// (a.walling 2010-03-08 11:48) - PLID 37640 - Moved a lot of EMN Detail releated classes that are not CEMNDetail to EMNDetailStructures

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace MSINKAUTLib;

// (a.walling 2011-08-11 16:43) - PLID 45021 - TableRow.m_pID is now TableRow.m_ID, which is not allocated on the heap.


// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp


// (a.walling 2012-10-12 15:05) - PLID 53165 - static CString default for Data here lets others reference without allocating memory
const CString ListElement::defaultLongForm = "<Data>";


// (a.walling 2012-12-03 12:50) - PLID 53983 - EmrListState provides a quick way to access elements by ID, as well as get child elements for hide/show
void EmrListState::Reset()
{
	elements.clear();
	children.clear();
	selected.clear();
}

void EmrListState::Load(CEMNDetail* pDetail)
{
	Reset();
	if (!pDetail) return;

	// first let's get our elements into a map
	foreach (ListElement& le, pDetail->GetListElements()) {
		elements.insert(std::make_pair(le.nID, le));
		long nParentLabelID = le.nParentLabelID;
		if (nParentLabelID == -1) continue;
		children.insert(std::make_pair(nParentLabelID, le.nID));
	}
	
	// now remember which parents and children should be visible even if not expanded
	{
		foreach (ListElement& le, pDetail->GetListElements()) {

			if (le.bIsLabel || !le.bIsSelected) continue;

			selected.insert(le.nID);

			// parent labels will be hidden regardless of whether a child is selected
		}
	}
}

TableDropdownItem::TableDropdownItem()
{
	nID = 1;
	bVisible = TRUE;
}

TableDropdownItem::TableDropdownItem(const TableDropdownItem &item)
{
	*this = item;
}

TableDropdownItem::TableDropdownItem(const long nDropdownID, LPCTSTR strDropdownText, const BOOL bDropdownVisible)
{
	nID = nDropdownID;
	strText = strDropdownText;
	bVisible = bDropdownVisible;
}

TableDropdownItem& TableDropdownItem::operator=(const TableDropdownItem &source)
{
	nID = source.nID;
	strText = source.strText;
	bVisible = source.bVisible;

	mapStampsIDs.RemoveAll();
	POSITION pos = source.mapStampsIDs.GetStartPosition();
	while(pos != NULL)
	{
		long nStampID;
		bool b;
		source.mapStampsIDs.GetNextAssoc(pos, nStampID, b);

		mapStampsIDs.SetAt(nStampID, b);
	}

	return *this;
}

// (a.walling 2013-07-24 11:18) - PLID 57698 - const
CString TableDropdownItem::GetComboText() const
{
	CString strFormattedText = strText;
	strFormattedText.Replace("\"", "\"\"");
	CString strComboText = FormatString("%li;\"%s\";%d;", nID, strFormattedText, bVisible ? 1 : 0);
	return strComboText;
}

void TableDropdownItem::AddStamp(const long nStampID)
{
	mapStampsIDs.SetAt(nStampID, true);
}

// (a.walling 2013-07-24 11:18) - PLID 57698 - const
BOOL TableDropdownItem::HasStampID(const long nStampID) const
{
	// (j.jones 2013-07-03 12:16) - PLID 57439 - if the stamp ID is -1,
	// don't bother looking it up
	if(nStampID == -1) {
		return FALSE;
	}

	bool b;
	if(mapStampsIDs.Lookup(nStampID, b)) {
		return TRUE;
	}

	return FALSE;
}

// (a.walling 2013-07-24 11:18) - PLID 57698 - CTableDropdownItemArray::GetComboText can be made even faster 
// by avoiding string allocations and taking advantage of memcpy and strlen intrinsics
CString CTableDropdownItemArray::GetComboText(const long nStampID) const
{	
	BOOL bStampHasFilter = FALSE;
	static const char prefix[] = ";;-1;;"; // use the -1 / extended format so we don't need to escape quotes
	int totalLen = _countof(prefix) + (GetCount() * (11 + 1 + 1 + 1 + 2)) + 1; // size of 32-bit integer as base10 string will not exceed 11 chars (including - sign)
	{
		// first scan to gather alloc size and determine whether we will be using filtered results
		for(int nDropdownIndex = 0; nDropdownIndex < GetCount(); nDropdownIndex++)
		{
			const TableDropdownItem& dropdownItem = GetAt(nDropdownIndex);
			totalLen += dropdownItem.strText.GetLength();
			if(!bStampHasFilter && dropdownItem.HasStampID(nStampID)) {
				// (z.manning 2011-10-17 12:30) - PLID 42061 - Remember that at least one dropdown item
				// in this column has a filter for this stamp.
				bStampHasFilter = TRUE;
			}
		}
	}

	CString strComboText;
	{
		// allocate buffer in one shot
		CString::CStrBuf buf(strComboText, totalLen);

		char* bufBegin = buf;
		char* bufPos = bufBegin;

		memcpy(bufPos, prefix, _countof(prefix) - 1);
		bufPos += _countof(prefix) - 1;

		for(int nDropdownIndex = 0; nDropdownIndex < GetCount(); nDropdownIndex++)
		{
			const TableDropdownItem& dropdownItem = GetAt(nDropdownIndex);

			// (j.jones 2013-07-03 12:13) - PLID 57439 - get the current dropdown text for strAllComboText

			// in the -1 format, fields are separated by \x02 and rows by \x03
			_itoa(dropdownItem.nID, bufPos, 10);
			bufPos += strlen(bufPos);
			*bufPos++ = 0x02;
			memcpy(bufPos, dropdownItem.strText, dropdownItem.strText.GetLength());
			bufPos += dropdownItem.strText.GetLength();
			*bufPos++ = 0x02;

			// if we are filtering, set as not visible if nID > 0 && does not have the stampid in the filter
			if (bStampHasFilter && dropdownItem.nID > 0 && !dropdownItem.HasStampID(nStampID)) {
				*bufPos++ = '0';
			} else {
				*bufPos++ = dropdownItem.bVisible ? '1' : '0';
			}
			*bufPos++ = 0x02;
			*bufPos++ = 0x03;
		}
		*bufPos++ = 0;

		buf.SetLength(bufPos - bufBegin);
	}

	return strComboText;
}


// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp
// (r.gonet 06/20/2012) - PLID 51075 - Initialize the last saved revision to no revision (Null)
// (a.walling 2012-10-31 17:17) - PLID 53552 - LastSavedDetail - LastSavedStamps no longer necessary
EmrDetailImageStamp::EmrDetailImageStamp() :
	nID(-1),
	nLoadedFromID(-1),
	nStampID(-1),
	nOrderIndex(0),
	eRule(esstsrAddNewRow),
	x(0),
	y(0),
	m_nRefCnt(1)
	, bUsedInTableData(FALSE)
	, var3DPosX(g_cvarNull)
	, var3DPosY(g_cvarNull)
	, var3DPosZ(g_cvarNull)
	, varNormalX(g_cvarNull)
	, varNormalY(g_cvarNull)
	, varNormalZ(g_cvarNull)
	, n3DHotSpotID(-1)
{
	pCopiedFromDetailStamp = NULL;
}

EmrDetailImageStamp::EmrDetailImageStamp(EmrDetailImageStamp *pStampToCopy)
{
	// (z.manning 2010-03-11 16:16) - Copy the contents of the stamp but set the ID to -1 and the ref count to 1
	*this = *pStampToCopy;
	nID = -1;
	m_nRefCnt = 1;
	// (z.manning 2013-03-12 17:06) - PLID 55016 - Remember the stamp we were copied from
	SetCopiedFromDetailStamp(pStampToCopy);
}

// (z.manning 2010-02-23 15:15) - PLID 37412 - Constructor to load stamp from fields pointer
EmrDetailImageStamp::EmrDetailImageStamp(ADODB::FieldsPtr pflds) :
	m_nRefCnt(1)
{
	nID = AdoFldLong(pflds, "ID", -1);
	// (j.jones 2010-03-03 12:36) - PLID 37231 - nLoadedFromID is always a copy of nID
	// because we may be loading from a remembered value, and SetNew() will reset our nID
	// to -1, but the table will still need to find this stamp by the original ID
	nLoadedFromID = nID;
	nStampID = AdoFldLong(pflds, "EmrImageStampID", -1);
	nOrderIndex = AdoFldLong(pflds, "OrderIndex", -1);
	eRule = (EMRSmartStampTableSpawnRule)AdoFldByte(pflds, "SmartStampTableSpawnRule", esstsrAddNewRow);
	x = AdoFldLong(pflds, "XPos", -1);
	y = AdoFldLong(pflds, "YPos", -1);
	bUsedInTableData = AdoFldBool(pflds, "UsedInTableData"); // (z.manning 2011-01-27 14:47) - PLID 42335
	// (z.manning 2011-09-08 09:45) - PLID 45335 - 3D stamp fields
	var3DPosX = pflds->GetItem("XPos3D")->GetValue();
	var3DPosY = pflds->GetItem("YPos3D")->GetValue();
	var3DPosZ = pflds->GetItem("ZPos3D")->GetValue();
	varNormalX = pflds->GetItem("XNormal")->GetValue();
	varNormalY = pflds->GetItem("YNormal")->GetValue();
	varNormalZ = pflds->GetItem("ZNormal")->GetValue();
	n3DHotSpotID = AdoFldShort(pflds, "HotSpot3D", -1);
	pCopiedFromDetailStamp = NULL;

	// (j.jones 2013-07-25 16:52) - PLID 57583 - Assert if the positioning data is invalid.
	// Don't throw an exception just yet, because if they are merely loading an EMN, we will
	// let it load with the bad data anyways.
	// Exceptions are thrown if these bad coordinates are saved.
	if(HasInvalidPositioning()) {
		ASSERT(FALSE);
		//confirm this bad data was created prior to the release that
		//disallowed this saving, otherwise find out what created
		//this bad data
	}
}

// (z.manning 2010-02-18 14:38) - PLID 37404 - Added overloaded equality operator
bool EmrDetailImageStamp::operator==(EmrDetailImageStamp& source)
{
	return (
		nID == source.nID &&
		nLoadedFromID == source.nLoadedFromID &&
		nStampID == source.nStampID &&
		nOrderIndex == source.nOrderIndex &&
		eRule == source.eRule &&
		x == source.x &&
		y == source.y &&
		bUsedInTableData == source.bUsedInTableData &&
		var3DPosX == source.var3DPosX &&
		var3DPosY == source.var3DPosY &&
		var3DPosZ == source.var3DPosZ &&
		varNormalX == source.varNormalX &&
		varNormalY == source.varNormalY &&
		varNormalZ == source.varNormalZ &&
		n3DHotSpotID == source.n3DHotSpotID
		);
}

// (a.walling 2012-02-16 13:56) - PLID 48209 - Converted to interlocked / threadsafe reference counting

int EmrDetailImageStamp::AddRef()
{
	return _InterlockedIncrement(&m_nRefCnt);
}

int EmrDetailImageStamp::Release()
{
	// (a.walling 2012-02-16 13:56) - PLID 48209 - This was return m_nRefCnt despite it possibly being deleted if the count drops to zero
	long nRefCnt = _InterlockedDecrement(&m_nRefCnt);
	if (nRefCnt == 0) {
		// (z.manning 2013-03-12 17:05) - PLID 55016 - Clear out the copied from stamp pointer
		SetCopiedFromDetailStamp(NULL);
		delete this;
	}
	return nRefCnt;
}

// (z.manning 2013-03-12 17:04) - PLID 55016
void EmrDetailImageStamp::SetCopiedFromDetailStamp(EmrDetailImageStamp *pCopiedFrom)
{
	if(pCopiedFromDetailStamp != NULL) {
		pCopiedFromDetailStamp->Release();
	}
	pCopiedFromDetailStamp = pCopiedFrom;
	if(pCopiedFromDetailStamp != NULL) {
		pCopiedFromDetailStamp->AddRef();
	}
}

// (z.manning 2013-03-12 17:04) - PLID 55016
EmrDetailImageStamp* EmrDetailImageStamp::GetCopiedFromDetailStamp()
{
	return pCopiedFromDetailStamp;
}

_variant_t EmrDetailImageStamp::GetXVar()
{
	if(x == -1) {
		return g_cvarNull;
	}
	
	return x;
}

_variant_t EmrDetailImageStamp::GetYVar()
{
	if(y == -1) {
		return g_cvarNull;
	}
	
	return y;
}

_variant_t EmrDetailImageStamp::Get3DHotSpotIDVar()
{
	if(n3DHotSpotID == -1) {
		return g_cvarNull;
	}
	
	return n3DHotSpotID;
}

BOOL EmrDetailImageStamp::Is3D()
{
	if(x == -1 && y == -1 && var3DPosX.vt != VT_NULL) {
		return TRUE;
	}
	return FALSE;
}

// (j.jones 2013-07-25 16:54) - PLID 57583 - the positioning is invalid 
// if all positioning data (X/Y or X/Y/Z if 3D) is empty, or if neither
// positioning dataset is complete (X with no Y, X/Y but no Z, etc.)
bool EmrDetailImageStamp::HasInvalidPositioning()
{
	bool bHasValid2DCoordinates = false;
	bool bHasValid3DCoordinates = false;

	//2D coordinates are considered invalid if they are -1.
	//A non -1 negative is allowed if they stamped slightly outside
	//the bounds of the detail, such as with an image stamp.
	if(x != -1 && y != -1) {
		bHasValid2DCoordinates = true;
	}
	//valid 3D coordinates require X, Y, and Z to all be doubles (they can be negative),
	//same for X/Y/Z normals
	if(var3DPosX.vt == VT_R8 && var3DPosY.vt == VT_R8 && var3DPosZ.vt == VT_R8
		&& varNormalX.vt == VT_R8 && varNormalY.vt == VT_R8 && varNormalZ.vt == VT_R8) {
		bHasValid3DCoordinates = true;
	}

	if(!bHasValid2DCoordinates && !bHasValid3DCoordinates) {
		//We have no valid 2D nor 3D coordinates.
		//An exception will later be thrown if saving is attempted on this stamp.
		//Don't assert or throw an exception here, our caller is responsible for that decision.		
		return true;
	}

	//return false if the positioning is indeed valid
	return false;
}

// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
void EmrDetailImageStamp::LogEmrObjectData(int nIndent)
{
	// Log this object
	::LogEmrObjectData(nIndent, nID, this, esotDetailImageStamp, (nID == -1), FALSE, FALSE, "",
		"nStampID = %d  nOrderIndex = %d  eRule = %d  x = %d  y = %d"
		, nStampID
		, nOrderIndex
		, eRule
		, x
		, y
	);
}

// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp
TableColumn::TableColumn() :
	nID(-1),
	nType(-1),
	nSubType(lstDefault),
	bIsGrouped(FALSE),
	// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
	m_nDecimalPlaces(0),
	m_bIsLabel(FALSE),
	m_bHasActiveDropdownElements(TRUE), // (z.manning 2011-03-11) - PLID 42778
	m_bHasDropdownElements(TRUE), // (z.manning 2011-03-11) - PLID 42778
	//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
	m_GlassesOrderDataType(godtInvalid),
	m_nGlassesOrderDataID(-1),
	m_eAutofillType(etatNone), // (z.manning 2011-03-21 11:12) - PLID 30608
	m_nFlags(0), // (z.manning 2011-05-26 14:58) - PLID 43865
	m_bIsCurrentMedOrAllergyUsageCol(FALSE), //(e.lally 2011-12-08) PLID 46471
	m_ewccWoundCareDataType(wcdtNone) // (r.gonet 08/03/2012) - PLID 51948 - Initialize the Wound Care Data Type to the non-special type
{
}

TableColumn::~TableColumn()
{
	ClearStampDefaultDropdownInfo(); // (z.manning 2011-10-12 14:59) - PLID 45728
}

// (z.manning 2008-06-11 16:33) - PLID 30155 - Added assignment operator and copy constructor
TableColumn& TableColumn::operator=(const TableColumn &source)
{
	nID = source.nID;
	strName = source.strName;
	nType = source.nType;
	nSubType = source.nSubType;
	bIsGrouped = source.bIsGrouped;
	// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
	m_strFormula = source.m_strFormula;
	m_bAutoAlphabetizeDropdown = source.m_bAutoAlphabetizeDropdown;	// (c.haag 2010-02-24 15:33) - PLID 21301 - EmrDataT.AutoAlphabetizeDropdown
	m_nDecimalPlaces = source.m_nDecimalPlaces;
	m_strInputMask = source.m_strInputMask; // (z.manning 2009-01-15 15:21) - PLID 32724
	m_bIsLabel = source.m_bIsLabel; // (z.manning 2010-04-13 16:06) - PLID 38175
	m_strLongForm = source.m_strLongForm; // (z.manning 2010-07-29 15:03) - PLID 36150
	// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
	m_etantAutoNumberType = source.m_etantAutoNumberType;
	m_strAutoNumberPrefix = source.m_strAutoNumberPrefix;
	m_bHasDropdownElements = source.m_bHasDropdownElements; // (z.manning 2011-03-11) - PLID 42778
	m_bHasActiveDropdownElements = source.m_bHasActiveDropdownElements; // (z.manning 2011-03-11) - PLID 42778
	//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
	m_GlassesOrderDataType = source.m_GlassesOrderDataType;
	m_nGlassesOrderDataID = source.m_nGlassesOrderDataID;
	m_eAutofillType = source.m_eAutofillType; // (z.manning 2011-03-21 11:12) - PLID 30608
	m_nFlags = source.m_nFlags; // (z.manning 2011-05-26 14:58) - PLID 43865
	m_strDropdownSeparator = source.m_strDropdownSeparator;
	m_strDropdownSeparatorFinal = source.m_strDropdownSeparatorFinal;
	m_strSpawnedItemsSeparator = source.m_strSpawnedItemsSeparator; // (z.manning 2011-11-07 10:48) - PLID 46309

	aryAdditionalEmbeddedComboValues.RemoveAll();
	aryAdditionalEmbeddedComboValues.Append(source.aryAdditionalEmbeddedComboValues);

	m_bIsCurrentMedOrAllergyUsageCol = source.m_bIsCurrentMedOrAllergyUsageCol; //(e.lally 2011-12-08) PLID 46471

	// (r.gonet 08/03/2012) - PLID 51948 - Copy the WoundCareDataType
	m_ewccWoundCareDataType = source.m_ewccWoundCareDataType;

	return *this;
}

TableColumn::TableColumn(const TableColumn &tc)
{
	*this = tc;
}

BOOL TableColumn::IsCalculated()
{
	// (z.manning 2011-05-26 17:53) - PLID 42131 - A cell is not considered caclulated if the formula is only for transforms
	return !m_strFormula.IsEmpty() && ((m_nFlags & edfFormulaForTransform) == 0);
}

// (z.manning 2010-04-19 16:12) - PLID 38228
BOOL TableColumn::IsReadOnly()
{
	// (j.jones 2010-08-11 10:59) - PLID 39496 - the auto-number column is also read-only
	return (
		IsCalculated()
		|| nSubType == lstSmartStampQuantity
		|| nSubType == lstSmartStampAutoNumber
		|| m_bIsLabel);
}

// (z.manning 2008-06-11 16:33) - PLID 30155 - To get the ID for a calculated embedded combo value
// we make the array index negative then subtract 100000.
long TableColumn::GetEmbeddedComboIDFromArrayIndex(const long nArrayIndex)
{
	ASSERT(nArrayIndex >= 0);
	return nArrayIndex * -1 - 100000;
}

long TableColumn::GetArrayIndexFromEmbeddedComboID(const long nEmbeddedComboID)
{
	return (nEmbeddedComboID + 100000) * -1;
}

// (z.manning 2008-06-11 16:34) - PLID 30155 - This will return the embedded combo ID for the specified value
// or 0 if the value is not yet part of the array.
long TableColumn::GetEmbeddedComboIDFor(const CString strValue)
{
	for(int i = 0; i < aryAdditionalEmbeddedComboValues.GetSize(); i++) {
		CString str = aryAdditionalEmbeddedComboValues.GetAt(i).strText;
		if(str == strValue) {
			return GetEmbeddedComboIDFromArrayIndex(i);
		}
	}

	return 0;
}

// (z.manning 2008-06-11 16:35) - PLID 30155 - This function is to add a value to the embedded combo
// for this column.
long TableColumn::AddEmbeddedComboValue(const CString strValue, const BOOL bVisible)
{
	TableDropdownItem value;
	value.nID = GetEmbeddedComboIDFromArrayIndex(aryAdditionalEmbeddedComboValues.GetCount());
	value.strText = strValue;
	value.bVisible = bVisible;
	aryAdditionalEmbeddedComboValues.Add(value);
	return GetEmbeddedComboIDFromArrayIndex(aryAdditionalEmbeddedComboValues.GetSize() - 1);
}

// (z.manning 2008-06-25 14:48) - PLID 30155 - Returns true if the specified embedded combo ID
// is one of the additional values we store here.
BOOL TableColumn::IsAdditionalEmbeddedComboID(IN const long nEmbeddedComboID, OUT CString &strValue)
{
	long nIndex = GetArrayIndexFromEmbeddedComboID(nEmbeddedComboID);
	int nArraySize = aryAdditionalEmbeddedComboValues.GetSize();
	if(nIndex >= 0 && nIndex < nArraySize) {
		strValue = aryAdditionalEmbeddedComboValues.GetAt(nIndex).strText;
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// (z.manning 2011-03-17 15:20) - PLID 42618
BOOL TableColumn::CanHaveDropdownElements()
{
	if(nType == LIST_TYPE_DROPDOWN) {
		return TRUE;
	}
	else if(nType == LIST_TYPE_TEXT)
	{
		// (z.manning 2011-03-17 15:52) - PLID 42618 - Don't allow dropdowns if the text column is read-only.
		if(IsReadOnly()) {
			return FALSE;
		}

		if(!m_strInputMask.IsEmpty()) {
			// (z.manning 2011-03-17 15:35) - PLID 42618 - Dropdowns in text columns are used primarily for auto-complete
			// so if we have an input mask let's not bother with auto-complete at this point.
			return FALSE;
		}

		// (z.manning 2011-04-13 10:42) - PLID 42618 - Don't allow dropdown elements in auto-fill text type columns as 
		// that will make it too easy to add data that can be easily overwritten. It also makes no sense on a practical 
		// level to have have dropdown elements for most common selections on a column that is automatically filled.
		if(m_eAutofillType != etatNone) {
			return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}

// (z.manning 2011-03-17 15:20) - PLID 42618
BOOL TableColumn::ShouldLoadDropdownElementsFromData()
{
	if(nType == LIST_TYPE_DROPDOWN)
	{
		// (z.manning 2011-03-17 15:25) - PLID 42618 - If a dropdown list has any elements at all (including inactive)
		// then we need to load them from data.
		if(m_bHasDropdownElements) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	else if(nType == LIST_TYPE_TEXT)
	{
		// (z.manning 2011-03-17 15:26) - PLID 42618 - Text columns only need to load if they have active dropdown elements.
		if(m_bHasActiveDropdownElements) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	else {
		// (z.manning 2011-03-17 15:27) - PLID 42618 - No other column types support dropdown elements at this time.
		return FALSE;
	}
}

// (z.manning 2011-03-22 09:41) - PLID 30608
BOOL TableColumn::AllowAutofill()
{
	if(IsReadOnly()) {
		return FALSE;
	}

	if(nType != LIST_TYPE_TEXT) {
		return FALSE;
	}

	if(IsSmartStampListSubType(nSubType)) {
		return FALSE;
	}

	// (j.jones 2011-05-03 16:08) - PLID 43527 - disallow in current medication built-in columns
	if(IsCurrentMedicationListSubType(nSubType)) {
		return FALSE;
	}

	if(!m_strInputMask.IsEmpty()) {
		return FALSE;
	}

	return TRUE;
}

// (z.manning 2011-05-27 10:00) - PLID 42131
BOOL TableColumn::HasTransformFormula()
{
	// (z.manning 2011-05-31 15:53) - PLID 42131 - We can't apply transformations to read only columns.
	// Also, don't allow transforming if the column has an input mask or is set to auto-fill as that can cause
	// quirky or invalid behavior (this would be a dumb setup, but there's no need to actually prevent such a setup).
	if(!IsReadOnly() && m_strInputMask.IsEmpty() && m_eAutofillType == etatNone)
	{
		// (z.manning 2011-05-27 09:40) - PLID 42131 - See if this column has a formula and if the formula
		// is meant for transformations.
		if(!m_strFormula.IsEmpty() && (m_nFlags & edfFormulaForTransform) != 0) {
			return TRUE;
		}
	}

	return FALSE;
}

// (z.manning 2011-05-26 17:39) - PLID 42131
BOOL TableColumn::ShouldShowPlusSign()
{
	return (m_nFlags & edfShowPlusSign) != 0;
}

// (z.manning 2011-10-12 15:05) - PLID 45728
void TableColumn::ClearStampDefaultDropdownInfo()
{
	POSITION pos = m_mapStampIDToDefaultTableDropdownIDs.GetStartPosition();
	while(pos != NULL)
	{
		long nStampID;
		CArray<long,long> *parynDropdownIDs;
		m_mapStampIDToDefaultTableDropdownIDs.GetNextAssoc(pos, nStampID, parynDropdownIDs);
		delete parynDropdownIDs;
	}
	m_mapStampIDToDefaultTableDropdownIDs.RemoveAll();
}

// (z.manning 2011-10-12 15:05) - PLID 45728
void TableColumn::AddStampDefaultDropdownID(const long nStampID, const long nDropdownInfoID)
{
	CArray<long,long> *parynDropdownIDs = NULL;
	// (z.manning 2011-10-12 15:06) - PLID 45728 - See if we already have a dropdown ID array for this stamp ID
	if(!m_mapStampIDToDefaultTableDropdownIDs.Lookup(nStampID, parynDropdownIDs)) {
		// (z.manning 2011-10-12 15:06) - PLID 45728 - We don't have an array yet so allocate one and set it in the map.
		parynDropdownIDs = new CArray<long,long>();
		m_mapStampIDToDefaultTableDropdownIDs.SetAt(nStampID, parynDropdownIDs);
	}

	if(!IsIDInArray(nDropdownInfoID, *parynDropdownIDs)) {
		parynDropdownIDs->Add(nDropdownInfoID);
	}
}

// (z.manning 2011-10-12 15:33) - PLID 45728
CArray<long,long>* TableColumn::GetDefaultDropdownIDsForStampID(const long nStampID)
{
	CArray<long,long>* parynDropdownIDs = NULL;
	m_mapStampIDToDefaultTableDropdownIDs.Lookup(nStampID, parynDropdownIDs);
	return parynDropdownIDs;
}


// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp
TableRowID::TableRowID() :
	nDataID(-1),
	nEmrDetailImageStampID(-1),
	pDetailImageStamp(NULL),
	nStampID(-1), //TES 3/17/2010 - PLID 37530
	nStampIndexInDetailByType(-1) //TES 3/17/2010 - PLID 37530
{
}

// (z.manning 2010-03-17 14:58) - For safety, force users to use the 4 long overload
/*TableRowID::TableRowID(const long _nDataID, const long _nEmrDetailImageStampID, const long _nStampIndexInDetailByType) :
	nDataID(_nDataID),
	nEmrDetailImageStampID(_nEmrDetailImageStampID),
	pDetailImageStamp(NULL),
	nStampIndexInDetailByType(_nStampIndexInDetailByType)//TES 3/17/2010 - PLID 37530
{
}*/

TableRowID::TableRowID(const long _nDataID, const long _nEmrDetailImageStampID, const long _nEmrDetailImageStampPointer, const long _nStampID, const long _nStampIndexInDetailByType) :
	nDataID(_nDataID),
	nEmrDetailImageStampID(_nEmrDetailImageStampID),
	pDetailImageStamp((EmrDetailImageStamp*)_nEmrDetailImageStampPointer),
	nStampID(_nStampID), //TES 3/17/2010 - PLID 37530
	nStampIndexInDetailByType(_nStampIndexInDetailByType)//TES 3/17/2010 - PLID 37530
{
	if(pDetailImageStamp != NULL) {
		pDetailImageStamp->AddRef();
		// (c.haag 2012-10-26) - PLID 53440 - If the image stamp is not null, then we mus reset the
		// image stamp ID and stamp ID values since they won't be used.
		nEmrDetailImageStampID = -1;
		nStampID = -1;
	}
}

// (c.haag 2012-10-26) - PLID 53440 - Updated so the ID fields are always -1 since we now pull
// them from the stamp object
TableRowID::TableRowID(EmrDetailImageStamp *p, const long _nStampIndexInDetailByType) :
	nDataID(-1),
	nEmrDetailImageStampID(-1),
	pDetailImageStamp(p),
	nStampID(-1), //TES 3/17/2010 - PLID 37530
	nStampIndexInDetailByType(_nStampIndexInDetailByType)//TES 3/17/2010 - PLID 37530
{
	if(pDetailImageStamp != NULL) {
		pDetailImageStamp->AddRef();
	}
}

TableRowID::~TableRowID()
{
	if(pDetailImageStamp != NULL) {
		pDetailImageStamp->Release();
	}
}

TableRowID& TableRowID::operator=(const TableRowID &source)
{
	nDataID = source.nDataID;
	nEmrDetailImageStampID = source.nEmrDetailImageStampID;
	//TES 3/17/2010 - PLID 37530
	nStampID = source.nStampID;
	nStampIndexInDetailByType = source.nStampIndexInDetailByType;

	// (c.haag 2012-11-13) - PLID 53440 - We should have never been calling
	// any setters here. Add a reference to the source stamp if one exists, and
	// assign it here.
	if(NULL != source.pDetailImageStamp) {
		pDetailImageStamp = source.pDetailImageStamp;
		pDetailImageStamp->AddRef();
	} else {
		pDetailImageStamp = NULL;
	}

	return *this;
}

bool TableRowID::operator==(TableRowID& source)
{
	// (c.haag 2012-10-26) - PLID 53440 - We now use the property function to match stamps
	if(nDataID != -1 && nDataID == source.nDataID) { return true; }
	if(-1 != GetDetailImageStampID() && GetDetailImageStampID() == source.GetDetailImageStampID()) { return true; }
	if(pDetailImageStamp != NULL && pDetailImageStamp == source.pDetailImageStamp) { return true; }
	return false;
}

bool TableRowID::operator!=(TableRowID& source)
{
	return (!(*this == source));
}

// (c.haag 2012-10-26) - PLID 53440 - This will assign the stamp object to this table row
void TableRowID::SetDetailImageStamp(EmrDetailImageStamp *pStamp)
{
	// (c.haag 2012-11-13) - Check for bad data creation
	if (NULL == pStamp && -1 != nEmrDetailImageStampID)
	{
		// If we get here, it means we don't have a stamp but we do have a smart stamp detail ID.
		// This should never happen because it will cause us to lose our smart stamp detail relationship.
		ThrowNxException("SetDetailImageStamp was called with a NULL stamp for a table row linked to a smart stamp! nEmrDetailImageStampID = %d", nEmrDetailImageStampID);
	}

	if(pDetailImageStamp != NULL) {
		pDetailImageStamp->Release();
	}
	pDetailImageStamp = pStamp;
	if(pDetailImageStamp != NULL) {
		pDetailImageStamp->AddRef();
	}
	// (c.haag 2012-10-26) - PLID 53440 - In all cases we must reset the stamp-related IDs.
	// If pStamp is null, obviously they would be -1. If pStamp isn't null, they would still be -1
	// because we must defer to pStamp to get those values.
	nEmrDetailImageStampID = -1;
	nStampID = -1;
}

// (c.haag 2012-10-26) - PLID 53440 - Returns the smart stamp object. This can be null despite GetDetailImageStamp()
// not returning -1 in the event that the stamp object hasn't been created or assigned in memory yet.
EmrDetailImageStamp* TableRowID::GetDetailImageStampObject() const
{
	return pDetailImageStamp;
}

// (c.haag 2012-10-26) - PLID 53440 - This will get the detail image stamp ID
long TableRowID::GetDetailImageStampID() const
{
	return (NULL != pDetailImageStamp) ? pDetailImageStamp->nID : this->nEmrDetailImageStampID;
}

// (c.haag 2012-10-26) - PLID 53440 - This will set the detail image stamp ID. If this table row contains a valid
// EmrDetailImageStamp object, it will be assigned this new value.
void TableRowID::SetDetailImageStampID(long ID)
{
	if (NULL == pDetailImageStamp)
	{
		nEmrDetailImageStampID = ID;
	}
	else
	{
		pDetailImageStamp->nID = ID;
	}
}

// (c.haag 2012-10-26) - PLID 53440 - This will return the image stamp ID (not the image stamp detail ID)
long TableRowID::GetImageStampID() const
{
	return (NULL != pDetailImageStamp) ? pDetailImageStamp->nStampID : this->nStampID;
}

// (c.haag 2012-10-26) - PLID 53440 - This will set the image stamp ID. If this table row contains a valid
// EmrDetailImageStamp object, it will be assigned this new value.
void TableRowID::SetImageStampID(long ID)
{
	if (NULL == pDetailImageStamp)
	{
		nStampID = ID;
	}
	else
	{
		pDetailImageStamp->nStampID = ID;
	}
}

// (c.haag 2012-10-26) - PLID 53440 - This will reset the detail image stamp ID
void TableRowID::SetNew()
{
	// All we really need to do is set the detail image stamp ID to -1
	SetDetailImageStampID(-1);
}

// (c.haag 2012-10-26) - PLID 53440 - This will return true if this table row is associated with an image stamp
// whether by ID or by pointer.
BOOL TableRowID::HasDetailImageStamp() const
{
	return (nEmrDetailImageStampID != -1 || NULL != pDetailImageStamp);
}

// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp
TableRow::TableRow() :
	m_nDecimalPlaces(0),
	nGroupID(-1),
	m_bDeleteOnStateChange(FALSE),
	m_bIsLabel(FALSE),
	//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
	m_GlassesOrderDataType(godtInvalid),
	m_nGlassesOrderDataID(-1),
	m_nFlags(0), // (z.manning 2011-05-26 14:59) - PLID 43865
	m_bIsCurrentMedOrAllergy(FALSE), //(e.lally 2011-12-08) PLID 46471
	m_bIsCurrentMedOrAllergyChecked(FALSE) //(e.lally 2011-12-08) PLID 46471
{
}

// (z.manning 2010-02-24 17:20) - PLID 37532 - New constructor
TableRow::TableRow(const long nDataGroupID, const long nDetailStampID, const long nStampID, const long nStampIDInDetailByType) :
	m_nDecimalPlaces(0),
	nGroupID(nDataGroupID),
	m_bDeleteOnStateChange(FALSE),
	m_bIsLabel(FALSE),
	//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
	m_GlassesOrderDataType(godtInvalid),
	m_nGlassesOrderDataID(-1),
	m_nFlags(0), // (z.manning 2011-05-26 14:59) - PLID 43865
	m_bIsCurrentMedOrAllergy(FALSE), //(e.lally 2011-12-08) PLID 46471
	m_bIsCurrentMedOrAllergyChecked(FALSE) //(e.lally 2011-12-08) PLID 46471
{
	// (c.haag 2012-10-26) - PLID 53440 - Use the setter functions
	m_ID.SetDetailImageStampID(nDetailStampID);
	//TES 3/17/2010 - PLID 37530
	m_ID.SetImageStampID(nStampID);
	m_ID.nStampIndexInDetailByType = nStampIDInDetailByType;
}

// (z.manning 2010-02-19 11:51) - PLID 37427 - Added assignment operator
TableRow& TableRow::operator=(const TableRow &source)
{
	m_ID = source.m_ID; // (a.walling 2011-08-11 16:43) - PLID 45021 - This is not allocated on the heap any longer
	strName = source.strName;
	nGroupID = source.nGroupID;
	// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
	m_strFormula = source.m_strFormula;
	m_nDecimalPlaces = source.m_nDecimalPlaces;
	m_bDeleteOnStateChange = source.m_bDeleteOnStateChange;
	m_bIsLabel = source.m_bIsLabel; // (z.manning 2010-04-13 16:04) - PLID 38175
	//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
	m_GlassesOrderDataType = source.m_GlassesOrderDataType;
	m_nGlassesOrderDataID = source.m_nGlassesOrderDataID;
	m_nFlags = source.m_nFlags; // (z.manning 2011-05-26 14:59) - PLID 43865
	m_bIsCurrentMedOrAllergy = source.m_bIsCurrentMedOrAllergy; //(e.lally 2011-12-08) PLID 46471 - 
	m_bIsCurrentMedOrAllergyChecked = source.m_bIsCurrentMedOrAllergyChecked; //(e.lally 2011-12-08) PLID 46471 - 

	return *this;
}

TableRow::TableRow(const TableRow &trSource)
{
	// (a.walling 2011-08-11 16:43) - PLID 45021 - This is not allocated on the heap any longer
	//m_pID = new TableRowID;
	*this = trSource;
}

BOOL TableRow::IsCalculated()
{
	// (z.manning 2011-05-26 17:53) - PLID 42131 - A cell is not considered caclulated if the formula is only for transforms
	return !m_strFormula.IsEmpty() && ((m_nFlags & edfFormulaForTransform) == 0);
}

// (z.manning 2010-04-19 16:12) - PLID 38228
BOOL TableRow::IsReadOnly()
{
	return (IsCalculated() || m_bIsLabel);
}

// (z.manning 2011-02-23 13:46) - PLID 42549
BOOL TableRow::IsSmartStampRow()
{
	// (c.haag 2012-10-26) - PLID 53440 - Use the new getter function
	if(m_ID.HasDetailImageStamp()) {
		return TRUE;
	}

	return FALSE;
}

// (z.manning 2011-05-26 17:39) - PLID 42131
BOOL TableRow::ShouldShowPlusSign()
{
	return (m_nFlags & edfShowPlusSign) != 0;
}


// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp
TableElement::TableElement() :
	m_pRow(NULL),
	m_pColumn(NULL),
	// (c.haag 2008-01-11 17:26) - PLID 17936 - We now support multiple selections for dropdown values
	//m_nDropdownID(-1),
	m_bChecked(FALSE),
	m_pLinkedDetail(NULL),
	m_bHtmlCached(false),
	m_bFormulaHasUnusedNumbers(FALSE) //TES 10/12/2012 - PLID 39000
{
}

// (c.haag 2008-01-11 17:34) - PLID 17936 - Copy constructor
TableElement::TableElement(const TableElement& tSource)
{
	*this = tSource;
}

void TableElement::LoadValueFromString(const CString &strValue, OPTIONAL CEMN *pContainingEMN) 
{
	switch(m_pColumn->nType) {
	case LIST_TYPE_CHECKBOX:	//checkbox
		m_bChecked = strValue == "1";
		//(e.lally 2011-12-08) PLID 46471 - If this checkbox is on a current medication or allergy and it is the usage column, flag it so the whole row knows it has been checked
		if(m_pRow->m_bIsCurrentMedOrAllergy && m_pColumn->m_bIsCurrentMedOrAllergyUsageCol){
			m_pRow->m_bIsCurrentMedOrAllergyChecked = m_bChecked;
		}
		break;

	case LIST_TYPE_DROPDOWN:	//dropdown
		// (c.haag 2008-01-11 17:12) - PLID 17936 - We now support multiple selections
		//m_nDropdownID = atol(strValue);
		// (z.manning 2008-06-11 16:20) - PLID 30155 - If this element's row is calculated,
		// then we use the calculated value instead.
		if(m_pRow != NULL && m_pRow->IsCalculated()) {
			m_strValue = strValue;
		}
		else {
			ParseCommaDeliminatedText(m_anDropdownIDs, strValue);
		}
		break;
	case LIST_TYPE_LINKED:		//linked
		if(strValue.IsEmpty()) {
			m_pLinkedDetail = NULL;
			m_strLinkedSentenceHtml = "";
			m_strLinkedSentenceNonHtml = "";
		}
		else if(strValue.Find("\r") == -1) {
			//This is stored the old way, meaning it's a name.  Do we have an EMN to search?
			if(pContainingEMN) {
				m_pLinkedDetail = NULL;
				long nCount = pContainingEMN->GetTotalDetailCount();
				for(int i = 0; i < nCount && !m_pLinkedDetail; i++) {
					CEMNDetail *pDetail = pContainingEMN->GetDetail(i);
					if(pDetail->GetLabelText() == strValue) {
						// (c.haag 2007-08-13 17:36) - PLID 27049 - We need the detail pointer
						// itself, so we must use an EMN Assign function
						if (NULL == (m_pLinkedDetail = pContainingEMN->AssignDetail(i))) {
							// (c.haag 2007-08-13 09:43) - PLID 27049 - If we could not find the detail,
							// preserve it in this value, and we will attempt to recover it later when
							// EMR actions are processed.
							m_strMissingLinkedDetailID = strValue;
						} else {
							// (c.haag 2007-03-16 11:48) - PLID 25242 - Pass in the containing EMN
							// to ReloadSentence in case the detail has no parent topic for
							// GetSentence to get the EMN from
							ReloadSentence(pContainingEMN);
						}
					}
				}
				if(m_strMissingLinkedDetailID.IsEmpty() && m_pLinkedDetail == NULL) {
					//Hmm. This detail doesn't exist any more.
					// (c.haag 2008-06-03 14:02) - PLID 27778 - If we get here, it means we're loading
					// a legacy table value corresponding to a detail that was deleted at some point
					// in the past. This won't cause any problems. Other changes related to this item
					// ensure that newly saved data won't have this issue.
					//ASSERT(FALSE);
					m_strLinkedSentenceHtml = m_strLinkedSentenceNonHtml = strValue;
				}
			}
			else {
				//We can't look it up.  Rats.
				m_strLinkedSentenceHtml = m_strLinkedSentenceNonHtml = strValue;
			}
		}
		else {
			//OK, we've got the new way of doing things.  Do we have an EMN to search?
			if(pContainingEMN) {
				//Yup, find the right detail
				// (c.haag 2007-08-13 17:36) - PLID 27049 - Use "Assign..." EMN functions
				// because we intend to get a pointer for permanent storage into m_pLinkedDetail
				CString strID = strValue.Left(strValue.Find("\r"));
				if(strID.GetLength() >= 2) {
					if(strID.Left(2) == "T:") {
						m_pLinkedDetail = pContainingEMN->AssignDetailByTemplateDetailID(atol(strID.Mid(2)));
					}
					else if(strID.Left(2) == "I:") {
						m_pLinkedDetail = pContainingEMN->AssignDetailByID(atol(strID.Mid(2)));
					}
					else if(strID.Left(2) == "P:") {
						m_pLinkedDetail = (CEMNDetail*)atol(strID.Mid(2));
					}
				}
				if(m_pLinkedDetail) {
					// (c.haag 2007-03-16 11:48) - PLID 25242 - Pass in the containing EMN
					// to ReloadSentence in case the detail has no parent topic for
					// GetSentence to get the EMN from
					ReloadSentence(pContainingEMN);
				} else {
					// (c.haag 2007-08-13 09:43) - PLID 27049 - If we could not find the detail,
					// preserve it in this value, and we will attempt to recover it later when
					// EMR actions are processed.
					m_strMissingLinkedDetailID = strValue;
				}
			}
			else {
				//Nope, fortunately we have the cached output.
				CString strSentences = strValue.Mid(strValue.Find("\r")+1);;
				//We delimit with a \r, which is not followed by a \n.
				int nDelimit = strSentences.Find("\r");
				bool bFound = false;
				while(nDelimit != -1 && !bFound) {
					if(strSentences.GetLength() == nDelimit+1
						|| strSentences.GetAt(nDelimit+1) != '\n') {
						bFound = true;
					}
					else {
						nDelimit = strSentences.Find("\r",nDelimit+1);
					}
				}
				if(bFound) {
					m_strLinkedSentenceHtml = strSentences.Left(nDelimit);
					m_strLinkedSentenceNonHtml = strSentences.GetLength() == nDelimit+1?"":strSentences.Mid(nDelimit+1);
				}
				else {
					ASSERT(FALSE);
					//Hmm, invalid value.  We'll just use the whole value.
					m_strLinkedSentenceHtml = m_strLinkedSentenceNonHtml = strSentences;
				}
			}
		}
		break;

	case LIST_TYPE_TEXT:	//text
	default:
		m_strValue = strValue;
		break;
	}
}

CString TableElement::GetValueAsString()
{
	switch(m_pColumn->nType) {
	case LIST_TYPE_CHECKBOX:
		return m_bChecked ? "1" : "0";
		break;

	case LIST_TYPE_DROPDOWN:
		// (c.haag 2008-01-11 17:16) - PLID 17936 - We now support multiple dropdown selections
		//return AsString(m_nDropdownID);
		// (z.manning 2008-06-11 16:20) - PLID 30155 - If this element's row is calculated,
		// then we use the calculated value instead.
		if(m_pRow != NULL && m_pRow->IsCalculated()) {
			return m_strValue;
		}
		return ArrayAsString(m_anDropdownIDs);
		break;

	case LIST_TYPE_LINKED:
		if(m_pLinkedDetail) {
			if((m_pLinkedDetail->m_bIsTemplateDetail && m_pLinkedDetail->m_nEMRTemplateDetailID == -1) ||
				(!m_pLinkedDetail->m_bIsTemplateDetail && m_pLinkedDetail->m_nEMRDetailID == -1)) {
					//Crap, we don't know what the detail is yet.  Just store it the old way, for now.
					return m_pLinkedDetail->GetLabelText();
			}
			else {
				//We're probably about to be saved to data, make sure the output is up to date.
				ReloadSentence();
				if(m_pLinkedDetail->m_bIsTemplateDetail) {
					// (c.haag 2006-04-04 17:22) - PLID 19398 - The addition of the ":" is brought to you by t.schneider
					return "T:" + AsString(m_pLinkedDetail->m_nEMRTemplateDetailID) + "\r" + m_strLinkedSentenceHtml + "\r" + m_strLinkedSentenceNonHtml;
				}
				else {
					// (c.haag 2006-04-04 17:22) - PLID 19398 - The addition of the "I:" is brought to you by t.schneider
					return "I:" + AsString(m_pLinkedDetail->m_nEMRDetailID) + "\r" + m_strLinkedSentenceHtml + "\r" + m_strLinkedSentenceNonHtml;
				}
			}
		}
		else {
			return m_strLinkedSentenceNonHtml;
		}
		break;

	case LIST_TYPE_TEXT:
	default:
		return m_strValue;
	}
}

// (a.walling 2007-10-19 17:55) - PLID 27820 - Flag for preview pane special case
// (a.walling 2011-05-31 11:57) - PLID 42448 - pDetail cannot be NULL now for performance reasons, but may be in the future
CString TableElement::GetValueAsOutput(CEMNDetail* pDetail, bool bHtml, CStringArray &saTempFiles, bool bForPreview /*=FALSE*/, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL */)
{
	if (!m_pColumn) {
		return "";
	}

	switch(m_pColumn->nType) {
	case LIST_TYPE_CHECKBOX:
		// (a.walling 2007-10-19 17:25) - PLID 27820 - Return an HTML checkbox
		// (c.haag 2008-10-23 14:20) - PLID 31760 - Make sure the checkbox is centered within its cell
		if (bHtml && bForPreview) {
			return m_bChecked ? "<center><input type='checkbox' checked='checked' disabled='disabled' value='x'/></center>" : "";
		} else {
			return m_bChecked ? "X" : "";
		}
		break;
	case LIST_TYPE_DROPDOWN:
		{
			// (z.manning 2008-06-11 16:20) - PLID 30155 - If this element's row is calculated,
			// then we use the calculated value instead.
			if(m_pRow != NULL && m_pRow->IsCalculated()) {
				return bHtml ? ConvertToHTMLEmbeddable(m_strValue) : m_strValue;
			}

			// (c.haag 2006-06-21 17:38) - PLID 21165 - Don't query the data if the dropdown id is non-positive
			// because it will always return an empty string
			// (c.haag 2008-01-11 17:17) - PLID 17936 - Format the output as a comma-delimited text list
			//CString strData = (m_nDropdownID > 0) ? AsString(GetTableField("EmrTableDropdownInfoT", "Data", "ID", m_nDropdownID)) : "";

			CString strData;
			if (m_anDropdownIDs.GetSize() > 0) {

				CString strUnsavedValue;
				if(m_anDropdownIDs.GetSize() == 1 && m_pColumn->IsAdditionalEmbeddedComboID(m_anDropdownIDs.GetAt(0), strUnsavedValue)) {
					return bHtml ? ConvertToHTMLEmbeddable(strUnsavedValue) : strUnsavedValue;
				}

				// (c.haag 2008-12-02 17:02) - PLID 32300 - m_anDropdownIDs can have non-positive elements in it. Make a copy of it
				// that only includes positive ID's. This will prevent any unnecessary querying.
				CArray<long,long> anPositiveDropdownIDs;
				for (int n=0; n < m_anDropdownIDs.GetSize(); n++) {
					if (m_anDropdownIDs[n] > 0) {
						anPositiveDropdownIDs.Add( m_anDropdownIDs[n] );
					}
				}

				if (anPositiveDropdownIDs.GetSize() > 0) {
					// (c.haag 2008-01-14 09:46) - We really need to optimize this! I propose we check for
					// the presence of the table datalist, and pull the value from there if it exists. Another
					// option is to cache all the possible values for all dropdown columns for this table. Will
					// reserve that work for another PL item. For now, maintain the status quo of getting one
					// recordset.
					_ConnectionPtr pCon;
					if(lpCon) pCon = lpCon;
					else pCon = GetRemoteData();

					// First build a map that maps dropdown ID's to text output
					// (a.walling 2013-07-02 09:02) - PLID 57407 - CMap's ARG_VALUE should be const CString& instead of LPCTSTR so it can use CString reference counting
					CMap<long, long, CString, const CString&> mapIDToData;
					const int nDropdownIDs = anPositiveDropdownIDs.GetSize();

					// (a.walling 2011-05-31 10:20) - PLID 42448 - This function will cache results until ClearContent is called; 
					// not the best optimization, but considering we don't have a shared object that can handle changes in the 
					// underlying data, this is the most reliable while still providing a significant benefit over the previous
					// approaches.
					// (j.jones 2013-02-20 15:43) - PLID 55217 - we now pass in the column ID
					CEMNDetail::GetDropdownData(anPositiveDropdownIDs, mapIDToData);

					//// (a.walling 2011-02-14 12:30) - PLID 42448 - This must be optimized away; see 42448
					//// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
					//_RecordsetPtr prs = CreateParamRecordset(pCon, "SELECT ID, Data FROM EmrTableDropdownInfoT WHERE ID IN ({INTSTRING})", strDropdownIDs);
					//FieldsPtr f = prs->Fields;
					//while (!prs->eof) {
					//	mapIDToData.SetAt(AsString(AdoFldLong(f, "ID")), AdoFldString(f, "Data"));
					//	prs->MoveNext();
					//}

					// Now go through the dropdown ID's and build a comma-delimited string of text output
					// to represent the multi-selection. This is obviously hardcoded to be ", ". In the
					// future, if there is demand for it, we can use different formatting.
					for (int i=0; i < nDropdownIDs; i++)
					{
						CString strValue;
						const long nID = anPositiveDropdownIDs.GetAt(i);
						if (nID > 0)
						{
							if (mapIDToData.Lookup(nID, strValue)) {
								if (!strData.IsEmpty()) {
									// If there's already an entry in strData, add a comma delimiter to it
									// (z.manning 2011-09-19 14:46) - PLID 41954 - We now have fields for dropdown separators
									if(i == (nDropdownIDs - 1)) {
										strData += m_pColumn->m_strDropdownSeparatorFinal;
									}
									else {
										strData += m_pColumn->m_strDropdownSeparator;
									}
								}
								strData += strValue;
							}
							else {
								// The ID wasn't in the map, meaning it wasn't in data. This should never happen
								ASSERT(FALSE);
								ThrowNxException(FormatString("TableElement::GetValueAsOutput was called for a dropdown column, but an ID was missing from data! "
									"ID = %d", nID));
							}

						}
						else {
							// Ignore this invalid ID. It could be that there's just one element in anPositiveDropdownIDs
							// and it's -1.
						}
					}
				} // if (anPositiveDropdownIDs.GetSize() > 0) {
			}
			return bHtml? ConvertToHTMLEmbeddable(strData) : strData;
		}
		break;
	case LIST_TYPE_LINKED:
		if(bHtml) {
			if(m_bHtmlCached) {
				// (a.walling 2007-10-18 14:09) - PLID 25548 - We are not escaping this, although we should be, just like all the other branches in this code
				// (a.walling 2007-10-19 12:37) - PLID 27820 - Reverting my change here. GetSentence should ultimately be responsible for escaping
				return m_strLinkedSentenceHtml;
			}
			else {
				//This used temp files, so we couldn't generate it until now.
				if(m_pLinkedDetail) {
					// (a.walling 2007-04-25 09:06) - PLID 25454 - bInHtml is true, so GetSentence will not generate a temp file
					// (a.walling 2007-10-18 14:09) - PLID 25548 - We are not escaping this, although we should be.
					// (a.walling 2007-10-19 12:37) - PLID 27820 - Reverting my change here. GetSentence should ultimately be responsible for escaping
					// (c.haag 2008-02-22 10:59) - PLID 29064 - Added connection pointer parameter
					return GetSentence(m_pLinkedDetail, NULL, true, true, saTempFiles, ecfParagraph, NULL, NULL, NULL, lpCon);
				}
				else {
					//We can't output HTML unless we've loaded and linked to our details.
					//Just return the non-html form, and hope for the best.
					return ConvertToHTMLEmbeddable(m_strLinkedSentenceNonHtml);
				}
			}

		}
		else {
			return m_strLinkedSentenceNonHtml;
		}
		break;

	case LIST_TYPE_TEXT:
	default:
		return bHtml?ConvertToHTMLEmbeddable(m_strValue):m_strValue;
	}
}


// (a.walling 2012-10-31 17:17) - PLID 53552 - LastSavedDetail - removed unnecessary string copy from _bstr_t to _variant_t
_variant_t TableElement::GetValueAsVariant()
{
	if (!m_pColumn) {
		return g_cvarNull;
	}

	switch(m_pColumn->nType) {
	case LIST_TYPE_CHECKBOX:
		return m_bChecked ? VARIANT_TRUE:VARIANT_FALSE;
		break;
		
	case LIST_TYPE_DROPDOWN:
		// (c.haag 2008-01-11 17:20) - PLID 17936 - Dropdowns can now have multiple selections. It
		// should be noted that now the caller should expect a comma-delimited string of ID's, not
		// a long integer.
		//return m_nDropdownID;
		//TES 3/27/2008 - PLID 29432 - VS2008 - Need to explicitly return a _variant_t
		// (z.manning 2008-06-11 16:20) - PLID 30155 - If this element's row is calculated,
		// then we use the calculated value instead.
		if(m_pRow != NULL && m_pRow->IsCalculated()) {
			return _variant_t(m_strValue);
		}
		return _variant_t(ArrayAsString(m_anDropdownIDs));
		break;

	case LIST_TYPE_LINKED:
		if(m_pLinkedDetail) {
			return _variant_t(m_pLinkedDetail->GetUniqueIdentifierAgainstEMN());
		}
		else {
			return g_cvarNull;
		}
		break;

	case LIST_TYPE_TEXT:
	default:
		return _variant_t(m_strValue);
		break;
	}
}


// (a.walling 2009-03-09 10:13) - PLID 33409 - Determine if the value is empty (whether it should be saved to data or not)
BOOL TableElement::IsValueEmpty()
{
	switch(m_pColumn->nType) {
	case LIST_TYPE_CHECKBOX:
		return m_bChecked == FALSE;
		break;

	case LIST_TYPE_DROPDOWN:
		// (c.haag 2008-01-11 17:16) - PLID 17936 - We now support multiple dropdown selections
		//return AsString(m_nDropdownID);
		// (z.manning 2008-06-11 16:20) - PLID 30155 - If this element's row is calculated,
		// then we use the calculated value instead.
		if(m_pRow != NULL && m_pRow->IsCalculated()) {
			return m_strValue.IsEmpty();
		} else {
			for (int i = 0; i < m_anDropdownIDs.GetSize(); i++) {
				if (m_anDropdownIDs[i] > 0) {
					return FALSE;
				}
			}

			return TRUE;
		}
		break;

	case LIST_TYPE_LINKED:
		{
			CString strOutput;

			if(m_pLinkedDetail) {
				if((m_pLinkedDetail->m_bIsTemplateDetail && m_pLinkedDetail->m_nEMRTemplateDetailID == -1) ||
					(!m_pLinkedDetail->m_bIsTemplateDetail && m_pLinkedDetail->m_nEMRDetailID == -1)) {
						//Crap, we don't know what the detail is yet.  Just store it the old way, for now.
						strOutput = m_pLinkedDetail->GetLabelText();
				}
				else {
					//We're probably about to be saved to data, make sure the output is up to date.
					ReloadSentence();
					if(m_pLinkedDetail->m_bIsTemplateDetail) {
						// (c.haag 2006-04-04 17:22) - PLID 19398 - The addition of the ":" is brought to you by t.schneider
						strOutput = "T:" + AsString(m_pLinkedDetail->m_nEMRTemplateDetailID) + "\r" + m_strLinkedSentenceHtml + "\r" + m_strLinkedSentenceNonHtml;
					}
					else {
						// (c.haag 2006-04-04 17:22) - PLID 19398 - The addition of the "I:" is brought to you by t.schneider
						strOutput = "I:" + AsString(m_pLinkedDetail->m_nEMRDetailID) + "\r" + m_strLinkedSentenceHtml + "\r" + m_strLinkedSentenceNonHtml;
					}
				}
			}
			else {
				strOutput = m_strLinkedSentenceNonHtml;
			}

			return strOutput.IsEmpty();
		}
		break;

	case LIST_TYPE_TEXT:
	default:
		return m_strValue.IsEmpty();
		break;
	}
}

void TableElement::LoadValueFromVariant(const _variant_t &varValue, CEMN *pContainingEMN)
{
	switch(m_pColumn->nType) {
	case LIST_TYPE_CHECKBOX:
		if(varValue.vt == VT_EMPTY)
			m_bChecked = FALSE;
		else
			m_bChecked = VarBool(varValue);
		break;

	case LIST_TYPE_DROPDOWN:
		if(varValue.vt == VT_EMPTY) {
			// (c.haag 2008-01-11 17:21) - PLID 17936 - Dropdowns may now have multiple selections.
			// To be as consistent with the old form as possible, our array should have only one
			// value in it; and it should be -1
			//m_nDropdownID = -1;
			m_anDropdownIDs.RemoveAll();
			m_anDropdownIDs.Add(-1);
			// (z.manning 2008-06-11 16:20) - PLID 30155 - If this element's row is calculated,
			// then we use the calculated value instead.
			if(m_pRow != NULL && m_pRow->IsCalculated()) {
				m_strValue.Empty();
			}
		} else {
			// (c.haag 2008-01-11 17:22) - PLID 17936 - The input value should be a comma-delimited
			// string of dropdown ID's instead of a long integer.
			//m_nDropdownID = VarLong(varValue, -1);
			// (z.manning 2008-06-11 16:20) - PLID 30155 - If this element's row is calculated,
			// then we use the calculated value instead.
			if(m_pRow != NULL && m_pRow->IsCalculated()) {
				m_strValue = VarString(varValue, "");
			}
			else {
				ParseCommaDeliminatedText(m_anDropdownIDs, VarString(varValue, ""));
			}
		}
		break;
		
	case LIST_TYPE_LINKED:
		if(varValue.vt == VT_BSTR && !VarString(varValue).IsEmpty()) {
			// (c.haag 2007-08-13 17:36) - PLID 27049 - We need the detail pointer
			// itself, so we must use an EMN Assign function
			m_pLinkedDetail = pContainingEMN->AssignDetailByUniqueIdentifier(VarString(varValue));
			ReloadSentence();
		}
		else {
			m_pLinkedDetail = NULL;
			m_strLinkedSentenceHtml = m_strLinkedSentenceNonHtml = "";
		}
		break;

	case LIST_TYPE_TEXT:
	default:
		if(varValue.vt == VT_EMPTY)		
			m_strValue = "";
		else
			m_strValue = VarString(varValue,"");
		break;
	}
}

// (c.haag 2007-03-16 11:48) - PLID 25242 - We now support getting EMN's passed through to GetSentence
void TableElement::ReloadSentence(OPTIONAL CEMN* pContainingEMN /*= NULL */)
{
	CStringArray sa;
	m_strLinkedSentenceNonHtml = GetSentence(m_pLinkedDetail, NULL, false, false, sa, ecfParagraph, pContainingEMN);
	ASSERT(!sa.GetSize());
	m_strLinkedSentenceHtml = GetSentence(m_pLinkedDetail, NULL, true, true, sa, ecfParagraph, pContainingEMN);
	if(sa.GetSize()) {
		//This uses temp files, we can't cache it.
		m_bHtmlCached = false;
		m_strLinkedSentenceHtml = "";
		for(int i = 0; i < sa.GetSize(); i++) DeleteFile(sa[i]);
	}
	else {
		//This didn't use temp files, so it will remain valid.
		m_bHtmlCached = true;
	}
}

// (c.haag 2008-01-11 17:09) - PLID 17936 - Returns TRUE if a dropdown value is selected
BOOL TableElement::IsDropdownValueSelected() const
{
	// (j.jones 2008-06-06 10:32) - PLID 18529 - this code mistakenly assumed that
	// an "unselected" dropdown would have a value of -1, it is in fact 0 if unselected,
	// -1 if there is no data at all

	if (m_anDropdownIDs.GetSize() == 0) {
		return FALSE; // No value is selected if there are no elements in the array
	} else if (m_anDropdownIDs.GetSize() == 1 && m_anDropdownIDs.GetAt(0) <= 0) {
		return FALSE; // No value is selected if the only value in the array is -1 or 0
	} else {
		return TRUE; // In all other cases, there must be something selected
	}
}

void TableElement::operator =(const TableElement &tSource)
{
	// (c.haag 2008-01-11 17:28) - PLID 17936 - Copy operator
	m_pRow = tSource.m_pRow;
	m_pColumn = tSource.m_pColumn;
	m_strValue = tSource.m_strValue;
	m_bChecked = tSource.m_bChecked;
	m_pLinkedDetail = tSource.m_pLinkedDetail;
	m_strLinkedSentenceHtml = tSource.m_strLinkedSentenceHtml;
	m_strLinkedSentenceNonHtml = tSource.m_strLinkedSentenceNonHtml;
	m_strMissingLinkedDetailID = tSource.m_strMissingLinkedDetailID;
	m_bHtmlCached = tSource.m_bHtmlCached;

	const long nIDs = tSource.m_anDropdownIDs.GetSize();
	m_anDropdownIDs.RemoveAll();
	for (long i=0; i < nIDs; i++) {
		m_anDropdownIDs.Add( tSource.m_anDropdownIDs.GetAt(i) );
	}
}

// (c.haag 2008-01-11 17:13) - PLID 17936 - Parses a comma-delimited list of ID's into an array of long integers
void TableElement::ParseCommaDeliminatedText(CArray<long,long> &anIDs, CString str) const
{
	// Clear the array
	anIDs.RemoveAll();

	// Commense parsing
	while (!str.IsEmpty())	{
		long comma = str.Find(',');
		
		// If we have no comma, we must have 1 more value in the string
		if(comma != -1) {
			anIDs.Add(atol(str.Left(comma)));
			str = str.Right(str.GetLength() - str.Left(comma+1).GetLength());
		} else {
			anIDs.Add(atol(str));
			str = "";
		}
	}
}

// (z.manning 2008-06-26 13:48) - PLID 30155 - Returns true if the element is calculated from a formula, false otherwise.
BOOL TableElement::IsCalculated()
{
	if(m_pRow == NULL || m_pColumn == NULL) {
		return FALSE;
	}

	return (m_pRow->IsCalculated() || m_pColumn->IsCalculated());
}

// (z.manning 2010-04-13 16:08) - PLID 38175 - Returns true if the element is in a row or column that's a label
BOOL TableElement::IsLabel()
{
	if(m_pRow == NULL || m_pColumn == NULL) {
		return FALSE;
	}

	return (m_pRow->m_bIsLabel || m_pColumn->m_bIsLabel);
}



// (z.manning 2010-04-19 16:12) - PLID 38228
BOOL TableElement::IsReadOnly()
{
	if(m_pRow == NULL || m_pColumn == NULL) {
		return FALSE;
	}

	return (m_pRow->IsReadOnly() || m_pColumn->IsReadOnly());
}

// (z.manning 2010-12-20 10:22) - PLID 41886 - Moved logic from CEmrItemAdvTableBase::AppendTextToTableCell
// to a TableElement member function.
void TableElement::Clear()
{
	if(m_pColumn == NULL) {
		return;
	}

	if(m_pColumn->nType == LIST_TYPE_CHECKBOX) {
		m_bChecked = FALSE;
	}
	else if(m_pColumn->nType == LIST_TYPE_DROPDOWN) {
		m_anDropdownIDs.RemoveAll();
		m_anDropdownIDs.Add(-1);
	}
	else if(m_pColumn->nType == LIST_TYPE_LINKED) {
		m_pLinkedDetail = NULL;
		m_strLinkedSentenceHtml = m_strLinkedSentenceNonHtml = "";
		m_strValue = "";
	}
	else {
		m_strValue = "";
	}
}

// (z.manning 2010-12-20 10:39) - PLID 41886
BOOL TableElement::IsEmpty()
{
	if(m_pColumn == NULL) {
		return TRUE;
	}

	if(m_pColumn->nType == LIST_TYPE_CHECKBOX) {
		if(!m_bChecked) {
			return TRUE;
		}
	}
	else if(m_pColumn->nType == LIST_TYPE_DROPDOWN) {
		if(m_anDropdownIDs.GetSize() == 0) {
			return TRUE;
		}
		if(m_anDropdownIDs.GetSize() == 1) {
			if(m_anDropdownIDs.GetAt(0) == -1) {
				return TRUE;
			}
		}
	}
	else if(m_pColumn->nType == LIST_TYPE_LINKED) {
		if(m_pLinkedDetail == NULL && m_strLinkedSentenceHtml.IsEmpty() && m_strLinkedSentenceNonHtml.IsEmpty() && m_strValue.IsEmpty()) {
			return TRUE;
		}
	}
	else {
		if(m_strValue.IsEmpty()) {
			return TRUE;
		}
	}

	return FALSE;
}

// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp
CEmrDetailImageStampArray::CEmrDetailImageStampArray() :
	// (z.manning 2010-02-23 11:50) - PLID 37412 - Initial version of the variant data from this array
	// (z.manning 2011-01-27 15:28) - PLID 42335 - Version 2 - Added UsedInTableData
	// (z.manning 2011-09-08 09:54) - PLID 45335 - Version 3 - Added 3D fields
	m_nVersion(3)
{
}

void CEmrDetailImageStampArray::Clear()
{
	for(int nStampIndex = 0; nStampIndex < this->GetSize(); nStampIndex++) {
		this->GetAt(nStampIndex)->Release();
	}
	this->RemoveAll();
}

// (z.manning 2010-02-23 12:14) - PLID 37412 - Need to be able to load the detail stamp array from a variant array.
void CEmrDetailImageStampArray::LoadFromSafeArrayVariant(_variant_t varDetailStampData)
{
	// Make sure the data type is good; we only allow null, empty, or an array variant type.
	if(varDetailStampData.vt == VT_NULL || varDetailStampData.vt == VT_EMPTY) {
		//Load as blank.
		Clear();
		return;
	}
	else if (!(varDetailStampData.vt & VT_ARRAY)) {
		// Invalid type, just load as blank (but assert false first)
		ASSERT(FALSE);
		Clear();
		return;
	}
	Clear();

	// Copy the const parameter to our writable safe array
	COleSafeArray sa;
	{
		_variant_t varSafeArray(varDetailStampData);
		sa.Attach(varSafeArray.Detach());
	}

	// Access the safe array byte data
	{
		BYTE *pSafeArrayData = NULL;
		sa.AccessData((LPVOID*)&pSafeArrayData);
		ASSERT(pSafeArrayData);
		if (pSafeArrayData)
		{
			// Copy from the given memory to our member variables
			{
				BYTE *pCurPos = pSafeArrayData;

				// first 4 bytes: (v1+) current version
				DWORD dwVersion = 0;
				memcpy(&dwVersion, pCurPos, sizeof(DWORD));
				pCurPos += sizeof(DWORD);

				DWORD dwDetailStampCount = 0;
				if(dwVersion >= 1) {
					// next 4 bytes: (v1+) number of image detail stamps
					memcpy(&dwDetailStampCount, pCurPos, sizeof(DWORD));
					pCurPos += sizeof(DWORD);
				}

				//Now, start copying the strings.
				Clear();
				for(DWORD dwDetailStampIndex = 0; dwDetailStampIndex < dwDetailStampCount; dwDetailStampIndex++)
				{
					EmrDetailImageStamp *pDetailStamp = new EmrDetailImageStamp;
					DWORD dw;
					_variant_t var;
					if(dwVersion >= 1) {
						memcpy(&dw, pCurPos, sizeof(DWORD));
						pDetailStamp->nID = dw;
						// (j.jones 2010-03-03 12:36) - PLID 37231 - nLoadedFromID is always a copy of nID
						// because we may be loading from a remembered value, and SetNew() will reset our nID
						// to -1, but the table will still need to find this stamp by the original ID
						pDetailStamp->nLoadedFromID = dw;
						pCurPos += sizeof(DWORD);
					}
					if(dwVersion >= 1) {
						memcpy(&dw, pCurPos, sizeof(DWORD));
						pDetailStamp->nStampID = dw;
						pCurPos += sizeof(DWORD);
					}
					if(dwVersion >= 1) {
						memcpy(&dw, pCurPos, sizeof(DWORD));
						pDetailStamp->nOrderIndex = dw;
						pCurPos += sizeof(DWORD);
					}
					if(dwVersion >= 1) {
						memcpy(&dw, pCurPos, sizeof(DWORD));
						pDetailStamp->eRule = (EMRSmartStampTableSpawnRule)dw;
						pCurPos += sizeof(DWORD);
					}
					if(dwVersion >= 1) {
						memcpy(&dw, pCurPos, sizeof(DWORD));
						pDetailStamp->x = dw;
						pCurPos += sizeof(DWORD);
					}
					if(dwVersion >= 1) {
						memcpy(&dw, pCurPos, sizeof(DWORD));
						pDetailStamp->y = dw;
						pCurPos += sizeof(DWORD);
					}
					// (z.manning 2011-01-27 15:31) - PLID 42335
					if(dwVersion >= 2) {
						memcpy(&dw, pCurPos, sizeof(DWORD));
						pDetailStamp->bUsedInTableData = dw == 0 ? FALSE : TRUE;
						pCurPos += sizeof(DWORD);
					}
					// (z.manning 2011-09-08 09:55) - PLID 45335 - Handle 3D fields
					if(dwVersion >= 3)
					{
						memcpy(&var, pCurPos, sizeof(_variant_t));
						pDetailStamp->var3DPosX = var;
						pCurPos += sizeof(_variant_t);
						memcpy(&var, pCurPos, sizeof(_variant_t));
						pDetailStamp->var3DPosY = var;
						pCurPos += sizeof(_variant_t);
						memcpy(&var, pCurPos, sizeof(_variant_t));
						pDetailStamp->var3DPosZ = var;
						pCurPos += sizeof(_variant_t);

						memcpy(&var, pCurPos, sizeof(_variant_t));
						pDetailStamp->varNormalX = var;
						pCurPos += sizeof(_variant_t);
						memcpy(&var, pCurPos, sizeof(_variant_t));
						pDetailStamp->varNormalY = var;
						pCurPos += sizeof(_variant_t);
						memcpy(&var, pCurPos, sizeof(_variant_t));
						pDetailStamp->varNormalZ = var;
						pCurPos += sizeof(_variant_t);

						memcpy(&pDetailStamp->n3DHotSpotID, pCurPos, sizeof(short));
						pCurPos += sizeof(short);
					}

					// (z.manning 2010-03-01 13:10) - PLID 37412 - This array assumes ownership
					// of the default reference of 1 to this detail stamp pointer.
					this->Add(pDetailStamp);
				}
			}

			// Let go of the data pointer
			sa.UnaccessData();
			pSafeArrayData = NULL;
		}
		else {
			// Failure
			sa.UnaccessData();
			pSafeArrayData = NULL;
			ThrowNxException(_T("CNxInkPictureText::LoadFromVariant(): Could not access data from SafeArray"));
		}
	}
}

_variant_t CEmrDetailImageStampArray::GetAsVariant()
{
	/////////////////////////////////////////////////////////////////////////////////
	// first 4 bytes: (v1+) current version
	// next 4 bytes: (v1+) number of detail image stamps
	// next dynamic len: (v1+) all the detail stamps, structured as follows:
	//		first 4 bytes: (v1+) nID
	//		next 4 bytes: (v1+) nStampID
	//		next 4 bytes: (v1+) nOrderIndex
	//		next 4 bytes: (v1+) eRule
	//		next 4 bytes: (v1+) x position
	//		next 4 bytes: (v1+) y position
	//		next 4 bytes: (v2+) bUsedInTableData
	//		next 16 bytes: (v3+) var3DPosX
	//		next 16 bytes: (v3+) var3DPosY
	//		next 16 bytes: (v3+) var3DPosZ
	//		next 16 bytes: (v3+) varNormalX
	//		next 16 bytes: (v3+) varNormalY
	//		next 16 bytes: (v3+) varNormalZ
	//		next 4 bytes: (v3+) n3DHotSpotID
	/////////////////////////////////////////////////////////////////////////////////

	BYTE *pDetailStampData = NULL;
	try {
		// First just calculate the size our byte array will be
		DWORD dwTotalSize;
		{
			dwTotalSize = 0;
			// first 4 bytes: (v1+) current version
			dwTotalSize += sizeof(DWORD);
			// next 4 bytes: (v1+) number of detail image stamps
			dwTotalSize += sizeof(DWORD);
			for(int nDetailStampIndex = 0; nDetailStampIndex < this->GetSize(); nDetailStampIndex++)
			{
				//first 4 bytes: (v1+) nID
				dwTotalSize += sizeof(DWORD);
				//next 4 bytes: (v1+) nStampID
				dwTotalSize += sizeof(DWORD);
				//next 4 bytes: (v1+) nOrderIndex
				dwTotalSize += sizeof(DWORD);
				//next 4 bytes: (v1+) eRule
				dwTotalSize += sizeof(DWORD);
				//next 4 bytes: (v1+) x position
				dwTotalSize += sizeof(DWORD);
				//next 4 bytes: (v1+) y position
				dwTotalSize += sizeof(DWORD);
				//next 4 bytes: (v2+) used in table data
				dwTotalSize += sizeof(DWORD);
				//next 16 bytes: (v3+) var3DPosX
				dwTotalSize += sizeof(_variant_t);
				//next 16 bytes: (v3+) var3DPosY
				dwTotalSize += sizeof(_variant_t);
				//next 16 bytes: (v3+) var3DPosZ
				dwTotalSize += sizeof(_variant_t);
				//next 16 bytes: (v3+) varNormalX
				dwTotalSize += sizeof(_variant_t);
				//next 16 bytes: (v3+) varNormalY
				dwTotalSize += sizeof(_variant_t);
				//next 16 bytes: (v3+) varNormalZ
				dwTotalSize += sizeof(_variant_t);
				//next 2 bytes: (v3+) n3DHotSpotID
				dwTotalSize += sizeof(short);
			}
		}
		
		// Now create a byte array that large
		COleSafeArray sa;
		sa.CreateOneDim(VT_UI1, dwTotalSize);
		
		// Fill the variant array
		BYTE *pSafeArrayData = NULL;
		sa.AccessData((LPVOID *)&pSafeArrayData);
		ASSERT(pSafeArrayData);
		if (pSafeArrayData)
		{
			// Copy from the given memory to the safearray memory
			BYTE *pCurPos = pSafeArrayData;
			// first 4 bytes: (v1+) current version
			DWORD dwVersion = m_nVersion;
			memcpy(pCurPos, &dwVersion, sizeof(DWORD));
			pCurPos += sizeof(DWORD);
			// next 4 bytes: (v1+) number of detail image stamps
			DWORD dwDetailStampCount = this->GetSize();
			memcpy(pCurPos, &dwDetailStampCount, sizeof(DWORD));
			pCurPos += sizeof(DWORD);
			for(int nDetailStampIndex = 0; nDetailStampIndex < this->GetSize(); nDetailStampIndex++)
			{
				EmrDetailImageStamp *pDetailStamp = this->GetAt(nDetailStampIndex);
				_variant_t var;

				//first 4 bytes: (v1+) nID
				DWORD dw = pDetailStamp->nID;
				memcpy(pCurPos, &dw, sizeof(DWORD));
				pCurPos += sizeof(DWORD);

				//next 4 bytes: (v1+) nStampID
				dw = pDetailStamp->nStampID;
				memcpy(pCurPos, &dw, sizeof(DWORD));
				pCurPos += sizeof(DWORD);

				//next 4 bytes: (v1+) nOrderIndex
				dw = pDetailStamp->nOrderIndex;
				memcpy(pCurPos, &dw, sizeof(DWORD));
				pCurPos += sizeof(DWORD);

				//next 4 bytes: (v1+) eRule
				dw = pDetailStamp->eRule;
				memcpy(pCurPos, &dw, sizeof(DWORD));
				pCurPos += sizeof(DWORD);

				//next 4 bytes: (v1+) x position
				dw = pDetailStamp->x;
				memcpy(pCurPos, &dw, sizeof(DWORD));
				pCurPos += sizeof(DWORD);

				//next 4 bytes: (v1+) y position
				dw = pDetailStamp->y;
				memcpy(pCurPos, &dw, sizeof(DWORD));
				pCurPos += sizeof(DWORD);

				// (z.manning 2011-01-27 15:30) - PLID 42335
				//next 4 bytes: (v2+) UsedInTableData
				dw = pDetailStamp->bUsedInTableData ? 1 : 0;
				memcpy(pCurPos, &dw, sizeof(DWORD));
				pCurPos += sizeof(DWORD);

				// (z.manning 2011-09-08 10:10) - PLID 45335 - Handle the 3D stamp fields
				//next 16 bytes: (v3+) var3DPosX
				var = pDetailStamp->var3DPosX;
				memcpy(pCurPos, &var, sizeof(_variant_t));
				pCurPos += sizeof(_variant_t);

				//next 16 bytes: (v3+) var3DPosY
				var = pDetailStamp->var3DPosY;
				memcpy(pCurPos, &var, sizeof(_variant_t));
				pCurPos += sizeof(_variant_t);

				//next 16 bytes: (v3+) var3DPosZ
				var = pDetailStamp->var3DPosZ;
				memcpy(pCurPos, &var, sizeof(_variant_t));
				pCurPos += sizeof(_variant_t);

				//next 16 bytes: (v3+) varNormalX
				var = pDetailStamp->varNormalX;
				memcpy(pCurPos, &var, sizeof(_variant_t));
				pCurPos += sizeof(_variant_t);

				//next 16 bytes: (v3+) varNormalY
				var = pDetailStamp->varNormalY;
				memcpy(pCurPos, &var, sizeof(_variant_t));
				pCurPos += sizeof(_variant_t);

				//next 16 bytes: (v3+) varNormalZ
				var = pDetailStamp->varNormalZ;
				memcpy(pCurPos, &var, sizeof(_variant_t));
				pCurPos += sizeof(_variant_t);

				//next 2 bytes: (v3+) n3DHotSpotID
				memcpy(pCurPos, &pDetailStamp->n3DHotSpotID, sizeof(short));
				pCurPos += sizeof(short);
			}

			// Let go of the data pointer
			sa.UnaccessData();
			pSafeArrayData = NULL;
			if(pDetailStampData) {
				delete [] pDetailStampData;
			}
			// Get the variant object out of the safe array and return the variant
			// (a.walling 2010-08-10 13:16) - PLID 40060 - Memory leak. Detach returns a VARIANT which is copied into the _variant_t return value. The original
			// VARIANT is leaked. Returning the result of a _variant_t constructor to take advantage of in-place return value optimizations, so no copies occur.
			return _variant_t(sa.Detach(), false);
		} 
		else {
			// Failure, let go of the data pointer and throw an exception
			sa.UnaccessData();
			pSafeArrayData = NULL;
			if (pDetailStampData) {
				delete [] pDetailStampData;
			}
			ThrowNxException(_T("CEmrDetailImageStampArray::GetAsVariant(): Could not access data from SafeArray"));
		}
	}
	catch (...) {
		if (pDetailStampData) {
			delete [] pDetailStampData;
		}
		throw;
	}
}

// (z.manning 2010-02-18 14:08) - PLID 37404 - Function to find detail stamp by ID
EmrDetailImageStamp* CEmrDetailImageStampArray::FindByID(const long nID)
{
	for(int i = 0; i < this->GetSize(); i++) {
		EmrDetailImageStamp *pDetailStamp = this->GetAt(i);
		if(pDetailStamp->nID == nID) {
			return pDetailStamp;
		}
	}
	return NULL;
}

// (z.manning 2010-02-22 12:02) - PLID 37230 - Given a table row, will attempt to find a 
// corresponding detail image stamp based on the properies in the given row's ID.
EmrDetailImageStamp* CEmrDetailImageStampArray::FindByTableRow(const TableRow *ptr)
{
	for(int nDetailStampIndex = 0; nDetailStampIndex < this->GetSize(); nDetailStampIndex++) {
		EmrDetailImageStamp *pDetailStamp = this->GetAt(nDetailStampIndex);
		// (c.haag 2012-10-26) - PLID 53440 - Use the new getter functions
		if(pDetailStamp != NULL && pDetailStamp == ptr->m_ID.GetDetailImageStampObject()) {
			return pDetailStamp;
		}
		if(pDetailStamp->nID != -1 && pDetailStamp->nID == ptr->m_ID.GetDetailImageStampID()) {
			return pDetailStamp;
		}
		// (j.jones 2010-03-03 12:51) - PLID 37231 - also search by nLoadedFromID if nID is -1
		if(pDetailStamp->nID == -1 && pDetailStamp->nLoadedFromID != -1
			&& pDetailStamp->nLoadedFromID == ptr->m_ID.GetDetailImageStampID()) {
			return pDetailStamp;
		}
	}
	return NULL;
}

// (z.manning 2010-02-22 12:01) - PLID 37230 - Returns the max order index of all image detail
// stamps plus one.
long CEmrDetailImageStampArray::GetNextOrderIndex()
{
	long nNextOrderIndex = 1;
	for(int nDetailStampIndex = 0; nDetailStampIndex < this->GetSize(); nDetailStampIndex++) {
		EmrDetailImageStamp *pDetailStamp = this->GetAt(nDetailStampIndex);
		if(pDetailStamp->nOrderIndex >= nNextOrderIndex) {
			nNextOrderIndex = pDetailStamp->nOrderIndex + 1;
		}
	}
	return nNextOrderIndex;
}

// (z.manning 2010-02-22 12:00) - PLID 37230 - Returns true if an instance of an image stamp
// with ID nStampID already exists in the given hotspot.
BOOL CEmrDetailImageStampArray::DoesStampAlreadyExistInHotspot(const long nStampID, CEMRHotSpot *pHotSpot, EmrDetailImageStamp *pDetailStampToIgnore)
{
	for(int nDetailStampIndex = 0; nDetailStampIndex < this->GetSize(); nDetailStampIndex++)
	{
		EmrDetailImageStamp *pDetailStamp = this->GetAt(nDetailStampIndex);
		if(pDetailStamp->nStampID == nStampID && pDetailStamp != pDetailStampToIgnore)
		{
			if(pDetailStamp->Is3D())
			{
				if(pHotSpot->Get3DHotSpotID() == pDetailStamp->n3DHotSpotID) {
					return TRUE;
				}
			}
			else
			{
				if(pHotSpot->PtInSpot(CPoint(pDetailStamp->x,pDetailStamp->y))) {
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

//TES 3/16/2012 - PLID 45127 - Added, pulls the logic out of CEmrDetailImageStampArray::FindByTextString,
// so that other places can compare EmrDetailImageStamps and TextStrings
bool DoStampsMatch(EmrDetailImageStamp *pDetailStamp, const TextString ts) 
{
	// (j.jones 2013-07-25 16:52) - PLID 57583 - fail if our detail is null
	if(pDetailStamp == NULL) {
		ThrowNxException("DoStampsMatch was called with a NULL pDetailStamp.");
	}
	// (j.jones 2013-07-25 16:52) - PLID 57583 - If the position is invalid,
	// assert, but do not fail. We only fail when trying to save invalid
	// positioning to the database.
	if(pDetailStamp->HasInvalidPositioning()) {
		ASSERT(FALSE);
	}
	else if(ts.Is3D()) {
		//If this stamp is a valid 2D stamp, we wouldn't have asserted above.
		//But if the code thinks it should be a 3D stamp, assert now, because
		//this is unexpected data. VarDouble will silently continue and treat
		//these fields as 0.0.
		if(pDetailStamp->var3DPosX.vt != VT_R8 || pDetailStamp->var3DPosX.vt != VT_R8 || pDetailStamp->var3DPosZ.vt != VT_R8) {
			//why is a 2D stamp on a 3D image?
			ASSERT(FALSE);
		}
	}

	if(ts.Is3D())
	{
		// (z.manning 2011-09-09 14:44) - PLID 45335 - Also support 3D images here
		// (j.jones 2013-07-25 16:52) - PLID 57583 - Null 3D positioning is bad data, but we would have
		// already asserted if that was the case, and we will fail if this bad data is attempted to be saved
		// to the database. Permitting this function to continue allows users to add good stamps to images
		// that happen to already have bad data.
		if(LooseCompareDouble(VarDouble(ts.var3DPosX, 0.0), VarDouble(pDetailStamp->var3DPosX, 0.0), 0.00000001) == 0 &&
			LooseCompareDouble(VarDouble(ts.var3DPosY, 0.0), VarDouble(pDetailStamp->var3DPosY, 0.0), 0.00000001) == 0 &&
			LooseCompareDouble(VarDouble(ts.var3DPosZ, 0.0), VarDouble(pDetailStamp->var3DPosZ, 0.0), 0.00000001) == 0)
		{
			return true;
		}
		else {
			return false;
		}
	}
	else {
		if(pDetailStamp->x == ts.x && pDetailStamp->y == ts.y) {
			return true;
		}
		else {
			return false;
		}
	}
}


// (z.manning 2010-02-24 10:42) - PLID 37225 - Given a text string, will attempt to find a 
// corresponding detail image stamp.
EmrDetailImageStamp* CEmrDetailImageStampArray::FindByTextString(const TextString ts)
{
	for(int nDetailStampIndex = 0; nDetailStampIndex < this->GetSize(); nDetailStampIndex++)
	{
		EmrDetailImageStamp *pDetailStamp = this->GetAt(nDetailStampIndex);
		if(pDetailStamp != NULL)
		{
			//TES 3/16/2012 - PLID 45127 - Moved the comparison to a separate function
			if(DoStampsMatch(pDetailStamp, ts)) {
				return pDetailStamp;
			}
		}
	}
	return NULL;
}

// (z.manning 2010-02-24 10:42) - PLID 37225 - Removes the specified detail image stamp
void CEmrDetailImageStampArray::RemoveDetailStamp(EmrDetailImageStamp *pDetailStamp)
{
	for(int nDetailStampIndex = 0; nDetailStampIndex < this->GetSize(); nDetailStampIndex++) {
		EmrDetailImageStamp *pCurrent = this->GetAt(nDetailStampIndex);
		if(pCurrent == pDetailStamp) {
			pDetailStamp->Release();
			this->RemoveAt(nDetailStampIndex);
			break;
		}
	}
}

// (z.manning 2010-02-24 11:54) - PLID 37225 - Returns the first detail with the same stamp ID
EmrDetailImageStamp* CEmrDetailImageStampArray::FindReplacementDetailStamp(EmrDetailImageStamp *pDetailStampToReplace)
{
	for(int nDetailStampIndex = 0; nDetailStampIndex < this->GetSize(); nDetailStampIndex++) {
		EmrDetailImageStamp *pDetailStamp = this->GetAt(nDetailStampIndex);
		// (j.jones 2010-04-22 13:17) - PLID 38069 - do not replace with a stamp with a different rule
		if(pDetailStamp != pDetailStampToReplace
			&& pDetailStamp->nStampID == pDetailStampToReplace->nStampID
			&& pDetailStamp->eRule == pDetailStampToReplace->eRule) {
			return pDetailStamp;
		}
	}
	return NULL;
}

// (z.manning 2011-01-20 12:01) - PLID 42338
BOOL CEmrDetailImageStampArray::Contains(EmrDetailImageStamp *pDetailStamp)
{
	for(int nDetailStampIndex = 0; nDetailStampIndex < this->GetSize(); nDetailStampIndex++) {
		if(pDetailStamp == this->GetAt(nDetailStampIndex)) {
			return TRUE;
		}
	}
	return FALSE;
}

// (z.manning 2011-02-02 18:06) - PLID 42335
long CEmrDetailImageStampArray::GetTotalByGlobalStampID(const long nGlobalStampID)
{
	int nCount = 0;
	for(int nDetailStampIndex = 0; nDetailStampIndex < this->GetSize(); nDetailStampIndex++)
	{
		EmrDetailImageStamp *pDetailImageStamp = GetAt(nDetailStampIndex);
		if(pDetailImageStamp->nStampID == nGlobalStampID) {
			nCount++;
		}
	}
	return nCount;
}

// End class CEmrDetailImageStampArray
/////////////////////////////////////////////////////////////////////////////////////////////////////


// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp
void CTableColumnWidths::CopyFrom(CTableColumnWidths* pFrom) 
{
	for(int i = 0;i < pFrom->m_aryWidths.GetSize(); i++) {
		TableColumnWidth *p = pFrom->m_aryWidths.GetAt(i);
		// (r.gonet 02/14/2013) - PLID 40017 - Added the column width in the popped up table.
		AddWidth(p->strColName, p->nWidth, p->nPopupWidth);
	}
}

// (r.gonet 02/14/2013) - PLID 40017 - Compare two sets of table column widths for deep equality.
//  Returns true if all the column widths are the same between the two sets. false otherwise.
bool CTableColumnWidths::Equals(CTableColumnWidths *pOther) const
{
	if(pOther == NULL) {
		// (r.gonet 02/14/2013) - PLID 40017 - There is nothing to compare against.
		return false;
	}
	if(this == pOther) {
		// (r.gonet 02/14/2013) - PLID 40017 - These are actually the same object. So definitely equal.
		return true;
	}
	if(this->m_aryWidths.GetSize() != pOther->m_aryWidths.GetSize()) {
		// (r.gonet 02/14/2013) - PLID 40017 - They don't have the same number of columns, so certainly not equal.
		return false;
	}

	// (r.gonet 02/14/2013) - PLID 40017 - Compare each column name and width against the other's columns.
	for(int i = 0; i < pOther->m_aryWidths.GetSize(); i++) {
		TableColumnWidth *pWidth1 = this->m_aryWidths.GetAt(i);
		TableColumnWidth *pWidth2 = pOther->m_aryWidths.GetAt(i);
		if(pWidth1->strColName != pWidth2->strColName || pWidth1->nWidth != pWidth2->nWidth || 
			pWidth1->nPopupWidth != pWidth2->nPopupWidth) 
		{
			// (r.gonet 02/14/2013) - PLID 40017 - An early out. Not the same.
			return false;
		}
	}
	// (r.gonet 02/14/2013) - PLID 40017 - They're equal.
	return true;
}

// (c.haag 2008-06-26 11:09) - PLID 27968 - Set the width of the first column
// of a table; it's a non-data-entry column.
void CTableColumnWidths::SetFirstColumnWidth(long nWidth)
{
	if (0 == m_aryWidths.GetSize()) {
		AddWidth("", nWidth);
	} else {
		m_aryWidths[0]->nWidth = nWidth;
	}
}

// (r.gonet 02/14/2013) - PLID 40017 - Sets the first column's width in the popped up table.
void CTableColumnWidths::SetFirstPopupColumnWidth(long nWidth)
{
	if (0 == m_aryWidths.GetSize()) {
		AddWidth("", -1, nWidth);
	} else {
		m_aryWidths[0]->nPopupWidth = nWidth;
	}
}

// (c.haag 2008-06-26 11:38) - PLID 27968 - Returns the width of the first column
// of a table; it's a non-data-entry column.
long CTableColumnWidths::GetFirstColumnWidth() const
{
	if (0 == m_aryWidths.GetSize()) {
		return -1;
	} else {
		return m_aryWidths[0]->nWidth;
	}
}

// (r.gonet 02/14/2013) - PLID 40017 - Returns the first column's width in the popped up table.
// - bDefaultToNonPopupWidth - Returns the regular width in the non popped up version if the popped up column has no width defined.
long CTableColumnWidths::GetFirstPopupColumnWidth(bool bDefaultToNonPopupWidth) const
{
	if (0 == m_aryWidths.GetSize()) {
		return -1;
	} else {
		if(bDefaultToNonPopupWidth && m_aryWidths[0]->nPopupWidth == -1) {
			return m_aryWidths[0]->nWidth;
		} else {
			return m_aryWidths[0]->nPopupWidth;
		}
	}
}

//Change the width of an existing column.  Adds newly if it does not exist
void CTableColumnWidths::SetWidthByName(CString strName, long nWidth) 
{
	TableColumnWidth *p = FindTableColumnByName(strName);
	if(p) {
		p->nWidth = nWidth;
	}
	else {
		AddWidth(strName, nWidth);
	}
}

// (r.gonet 02/14/2013) - PLID 40017 - Sets a column's width in the popped up table.
void CTableColumnWidths::SetPopupWidthByName(CString strName, long nWidth) 
{
	TableColumnWidth *p = FindTableColumnByName(strName);
	if(p) {
		p->nPopupWidth = nWidth;
	}
	else {
		AddWidth(strName, -1, nWidth);
	}
}

//Retrieves the currently stored width of a given column
long CTableColumnWidths::GetWidthByName(CString strName) 
{
	TableColumnWidth *p = FindTableColumnByName(strName);
	if(p)
		return p->nWidth;
	else
		return -1;
}

// (r.gonet 02/14/2013) - PLID 40017 - Returns a column's width in the popped up table.
// - bDefaultToNonPopupWidth - Returns the regular width in the non popped up version if the popped up column has no width defined.
long CTableColumnWidths::GetPopupWidthByName(CString strName, bool bDefaultToNonPopupWidth) 
{
	TableColumnWidth *p = FindTableColumnByName(strName);
	if(p)
	{
		if(bDefaultToNonPopupWidth && p->nPopupWidth == -1) {
			return p->nWidth;
		} else {
			return p->nPopupWidth;
		}
	}
	else
		return -1;
}

//Wipes out everything in the structure (erase the saved widths)
void CTableColumnWidths::ClearAll() 
{
	for(int i = 0; i < m_aryWidths.GetSize(); i++) {
		TableColumnWidth *p = m_aryWidths.GetAt(i);
		delete p;
	}

	m_aryWidths.RemoveAll();
}

//Helper to find a specific, rather than copying this for loop to a bunch
//	of different functions.
// (c.haag 2008-06-26 11:09) - PLID 27968 - Start at column 1 because column
// 0 is the special reserved column that has no name. If we included it, then
// we could have more than one qualifying column.
CTableColumnWidths::TableColumnWidth* CTableColumnWidths::FindTableColumnByName(CString strName) 
{
	for(int i = 1; i < m_aryWidths.GetSize(); i++) {
		TableColumnWidth *p = m_aryWidths.GetAt(i);
		if(p->strColName == strName) {
			return p;
		}
	}

	return NULL;
}

//Add a new width to be tracked
// (r.gonet 02/14/2013) - PLID 40017 - Added the column's width in the popup
void CTableColumnWidths::AddWidth(CString strName, long nWidth, long nPopupWidth/*= -1*/)	
{
	TableColumnWidth *pNew = new TableColumnWidth;
	pNew->strColName = strName;
	pNew->nWidth = nWidth;
	// (r.gonet 02/14/2013) - PLID 40017 - Added
	pNew->nPopupWidth = nPopupWidth;
	m_aryWidths.Add(pNew);
}

// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp
TableSpawnInfo::TableSpawnInfo() :
	nDropdownID(-1),
	nStampID(-1)
{
}

TableSpawnInfo::TableSpawnInfo(const long _nDropdownID, TableRow _tr, const long _nStampID) :
	nDropdownID(_nDropdownID),
	tr(_tr),
	nStampID(_nStampID)
{
}

CEmrItemAdvImageRenderState::CEmrItemAdvImageRenderState()
	: rImageSize(0,0,0,0)
	, eitOrigBackgroundImageType(itUndefined)
	, nEnableTextScaling(1)
{}

// (a.walling 2013-06-06 09:41) - PLID 57069 - Keeps track of all dependencies when generating an image's
// render output. If these items are equivalent, then the output should also be identical. We can use this
// to keep track of the last rendered image state so redundant generation can be avoided
// (z.manning 2015-05-28 14:36) - PLID 66102 - Added max width
CEmrItemAdvImageRenderState::CEmrItemAdvImageRenderState(CTextStringFilterPtr pFilter, const CEmrItemAdvImageState& ais
	, IN CRect rImageSize, IN CString strOrigBackgroundImageFilePath, IN eImageType eitOrigBackgroundImageType
	, bool bEnableEMRImageTextScaling, IN const long nMaxWidth
	)	
	: stateHash(ais.m_hash)
	, rImageSize(rImageSize)
	, eitOrigBackgroundImageType(eitOrigBackgroundImageType)
	, strOrigBackgroundImageFilePath(strOrigBackgroundImageFilePath)
	, nEnableTextScaling(bEnableEMRImageTextScaling ? 1 : 0)
	, nMaxWidth(nMaxWidth)
{
	md5_state_t state;
	md5_init(&state);

	if (pFilter) {
		for (int i = 0, count = pFilter->GetTypeCount(); i < count; ++i) {
			CString str = pFilter->GetType(i);
			md5_append(&state, (BYTE*)(LPCTSTR)str, str.GetLength());
		}
	}

	md5_finish(&state, optionHash.m_digest);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// (z.manning 2011-01-20 09:53) - PLID 42338 - Added a class for an array of EMN details so that I could
// add member functions to it as needed.

void CEMNDetailArray::AddRefAll(LPCTSTR strDescription)
{
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		pDetail->__AddRef(strDescription);
	}
}

void CEMNDetailArray::ReleaseAll(LPCTSTR strDescription)
{
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		pDetail->__Release(strDescription);
	}
}

// (z.manning 2011-01-31 13:27) - PLID 42338
void CEMNDetailArray::GetAllDetailImageStampsInOrder(OUT CEmrDetailImageStampArray *paryDetailImageStamps)
{
	paryDetailImageStamps->RemoveAll();
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		CEmrDetailImageStampArray *paryTemp = pDetail->GetDetailImageStamps();
		for(int nTempStampIndex = 0; nTempStampIndex < paryTemp->GetSize(); nTempStampIndex++)
		{
			BOOL bInserted = FALSE;
			for(int nAllStampIndex = 0; nAllStampIndex < paryDetailImageStamps->GetSize() && !bInserted; nAllStampIndex++)
			{
				if(paryTemp->GetAt(nTempStampIndex)->nOrderIndex < paryDetailImageStamps->GetAt(nAllStampIndex)->nOrderIndex) {
					paryDetailImageStamps->InsertAt(nAllStampIndex, paryTemp->GetAt(nTempStampIndex));
					bInserted = TRUE;
				}
			}
			if(!bInserted) {
				paryDetailImageStamps->Add(paryTemp->GetAt(nTempStampIndex));
			}
		}
	}
}

long CEMNDetailArray::GetStampIndexInDetailByType(EmrDetailImageStamp *pDetailStamp)
{
	CEmrDetailImageStampArray aryAllDetailStamps;
	GetAllDetailImageStampsInOrder(&aryAllDetailStamps);
	long nIndex = 0;
	for(int nDetailStampIndex = 0; nDetailStampIndex < aryAllDetailStamps.GetCount(); nDetailStampIndex++)
	{
		EmrDetailImageStamp *pTempDetailStamp = aryAllDetailStamps.GetAt(nDetailStampIndex);
		if(pDetailStamp->nStampID == pTempDetailStamp->nStampID) {
			nIndex++;
			if(pDetailStamp == pTempDetailStamp) {
				return nIndex;
			}
		}
	}
	//TES 3/17/2010 - PLID 37530 - We should never be passed in a stamp that isn't actually on this detail.
	ASSERT(FALSE);
	return -1;
}

BOOL CEMNDetailArray::Contains(CEMNDetail *pDetailToCheck)
{
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		if(pDetail == pDetailToCheck) {
			return TRUE;
		}
	}
	return FALSE;
}

void CEMNDetailArray::UpdateChildEMRTemplateDetailIDs(const long nID)
{
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		pDetail->m_nChildEMRTemplateDetailID = nID;
	}
}

void CEMNDetailArray::UpdateChildEMRDetailIDs(const long nID)
{
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		pDetail->m_nChildEMRDetailID = nID;
	}
}

BOOL CEMNDetailArray::AreSmartStampsEnabled()
{
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		if(pDetail->m_bSmartStampsEnabled) {
			return TRUE;
		}
	}
	return FALSE;
}

void CEMNDetailArray::SetSmartStampTableDetail(CEMNDetail *pSmartStampTable)
{
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		pDetail->SetSmartStampTableDetail(pSmartStampTable);
	}
}


// (z.manning 2010-03-16 12:29) - PLID 37493 - Returns a hot spot based on a quantity based smart stamp
// (z.manning 2011-01-20 17:32) - PLID 42338 - Moved to CEMNDetailArray and modified accordingly
CEMRHotSpot* CEMNDetailArray::GetHotSpotFromQuantityBasedSmartStamp(const long nStampID)
{
	CEMRHotSpot *pHotSpotToReturn = NULL;
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		for(int nDetailStampIndex = 0; nDetailStampIndex < pDetail->m_arypImageStamps.GetSize(); nDetailStampIndex++)
		{
			EmrDetailImageStamp *pDetailStamp = pDetail->m_arypImageStamps.GetAt(nDetailStampIndex);
			if(pDetailStamp->nStampID == nStampID) {
				// (z.manning 2010-03-16 12:43) - PLID 37493 - This detail stamp matches the stamp ID we're looking for
				// so let's see if it falls within a hotspot.
				CEMRHotSpot *pHotSpot = pDetail->GetHotSpotFromDetailStamp(pDetailStamp);
				if(pHotSpot != NULL) {
					// (z.manning 2010-03-16 12:43) - PLID 37493 - We did find a hot spot
					if(pHotSpotToReturn == NULL) {
						// (z.manning 2010-03-16 12:44) - PLID 37493 - This is the first hot spot we found for
						// the given stamp ID, so let's remember it.
						pHotSpotToReturn = pHotSpot;
					}
					else if(!pHotSpotToReturn->DoAnatomicLocationsMatch(pHotSpot)) {
						// (z.manning 2010-03-16 12:44) - PLID 37493 - We found multiple matching hot spots for
						// this stamp ID that do not have the same anatomic location. We cannot safely return a single
						// hotspot.
						return NULL;
					}
				}
			}
		}
	}
	return pHotSpotToReturn;
}

void CEMNDetailArray::SetVisible(BOOL bVisible, BOOL bRedraw, BOOL bIsInitialLoad)
{
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		if(pDetail->GetVisible() != bVisible) {
			pDetail->SetVisible(bVisible, bRedraw, bIsInitialLoad);
		}
	}
}

CEMNDetail* CEMNDetailArray::GetDetailFromDetailImageStamp(EmrDetailImageStamp *pDetailImageStamp)
{
	if(pDetailImageStamp == NULL) {
		return NULL;
	}

	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		if(pDetail->m_arypImageStamps.Contains(pDetailImageStamp)) {
			return pDetail;
		}
	}
	return NULL;
}

CEMNDetail* CEMNDetailArray::GetDetailFromDetailStampID(const long nDetailStampID)
{
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		EmrDetailImageStamp *pDetailStamp = pDetail->m_arypImageStamps.FindByID(nDetailStampID);
		if(pDetailStamp != NULL) {
			return pDetail;
		}
	}
	return NULL;
}

CEMNDetail* CEMNDetailArray::GetDetailFromEmrInfoMasterID(const long nEmrInfoMasterID)
{
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		if(pDetail->m_nEMRInfoMasterID == nEmrInfoMasterID) {
			return pDetail;
		}
	}
	return NULL;
}

// (z.manning 2011-01-26 12:12) - PLID 42336
BOOL CEMNDetailArray::AtLeastOneVisibleDetail()
{
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		if(pDetail->m_bVisible) {
			return TRUE;
		}
	}
	return FALSE;
}

// (z.manning 2011-01-27 15:48) - PLID 42335
EmrDetailImageStamp* CEMNDetailArray::FindReplacementDetailStamp(EmrDetailImageStamp *pDetailStampToReplace)
{
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		EmrDetailImageStamp *pDetailStamp = pDetail->GetDetailImageStamps()->FindReplacementDetailStamp(pDetailStampToReplace);
		if(pDetailStamp != NULL) {
			return pDetailStamp;
		}
	}
	return NULL;
}

// (z.manning 2011-01-31 15:25) - PLID 42338
long CEMNDetailArray::GetNextOrderIndex()
{
	long nNextOrderIndex = 1;
	CEmrDetailImageStampArray aryDetailStamps;
	GetAllDetailImageStampsInOrder(&aryDetailStamps);
	if(aryDetailStamps.GetSize() > 0) {
		EmrDetailImageStamp *pLastDetailStamp = aryDetailStamps.GetAt(aryDetailStamps.GetSize() - 1);
		nNextOrderIndex = pLastDetailStamp->nOrderIndex + 1;
	}
	return nNextOrderIndex;
}

// (z.manning 2011-02-02 18:06) - PLID 42335
long CEMNDetailArray::GetTotalByGlobalStampID(const long nGlobalStampID)
{
	long nCount = 0;
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		nCount += pDetail->GetDetailImageStamps()->GetTotalByGlobalStampID(nGlobalStampID);
	}

	return nCount;
}

// (z.manning 2011-02-11 17:23) - PLID 42446
void CEMNDetailArray::SortWithParentDetailsFirst()
{
	// (z.manning 2011-02-11 17:33) - PLID 42446 - Since all we're doing here is moving parent details to the beginning
	// there's no reason to check the first element.
	for(int nDetailIndex = 1; nDetailIndex < GetSize(); nDetailIndex++)
	{
		CEMNDetail *pDetail = GetAt(nDetailIndex);
		if(pDetail->m_nChildEMRDetailID != -1 || pDetail->m_nChildEMRTemplateDetailID != -1 || pDetail->m_nChildEMRInfoMasterID != -1) {
			// (z.manning 2011-02-11 17:38) - PLID 42446 - This is a parent detail so remove it from this position and move
			// it to the beginning of the array. Since we are iterating forward this will have no affect on upcoming iterations.
			RemoveAt(nDetailIndex);
			InsertAt(0, pDetail);
		}
	}
}

// (z.manning 2011-03-01 15:59) - PLID 42335
BOOL CEMNDetailArray::IsGlobalStampIDEverUsedInData(const long nStampID)
{
	CEmrDetailImageStampArray aryAllDetailStamps;
	GetAllDetailImageStampsInOrder(&aryAllDetailStamps);
	for(int nDetailStampIndex = 0; nDetailStampIndex < aryAllDetailStamps.GetCount(); nDetailStampIndex++)
	{
		EmrDetailImageStamp *pDetailStamp = aryAllDetailStamps.GetAt(nDetailStampIndex);
		if(pDetailStamp->nStampID == nStampID && pDetailStamp->bUsedInTableData) {
			return TRUE;
		}
	}

	return FALSE;
}

// End class CEMNDetailArray
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin class CEmrItemStampExclusions

// (z.manning 2011-10-25 14:55) - PLID 39401
void CEmrItemStampExclusions::LoadFromData(const long nEmrInfoMasterID)
{
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT StampID \r\n"
		"FROM EmrInfoStampExclusionsT \r\n"
		"WHERE EmrInfoMasterID = {INT} \r\n"
		, nEmrInfoMasterID);
	LoadFromRecordset(prs);
}

// (z.manning 2011-10-24 11:46) - PLID 46082
void CEmrItemStampExclusions::LoadFromRecordset(ADODB::_RecordsetPtr prs)
{
	RemoveAll();
	for(; !prs->eof; prs->MoveNext())
	{
		const long nStampID = AdoFldLong(prs, "StampID");
		AddExclusion(nStampID);
	}
}

// (z.manning 2011-10-24 11:46) - PLID 46082
CEmrItemStampExclusions& CEmrItemStampExclusions::operator=(const CEmrItemStampExclusions &source)
{
	RemoveAll();
	POSITION pos = source.m_mapStampIDs.GetStartPosition();
	while(pos != NULL)
	{
		long nStampID;
		bool b;
		source.m_mapStampIDs.GetNextAssoc(pos, nStampID, b);

		m_mapStampIDs.SetAt(nStampID, b);
	}

	return *this;
}

// (z.manning 2011-10-24 11:46) - PLID 46082
void CEmrItemStampExclusions::AddExclusion(const long nStampID)
{
	m_mapStampIDs.SetAt(nStampID, true);
}

// (z.manning 2011-10-24 11:46) - PLID 46082
void CEmrItemStampExclusions::RemoveAll()
{
	m_mapStampIDs.RemoveAll();
}

// (z.manning 2011-10-24 11:46) - PLID 46082
BOOL CEmrItemStampExclusions::IsExcluded(const long nStampID)
{
	bool b;
	return m_mapStampIDs.Lookup(nStampID, b);
}

// (z.manning 2011-10-24 11:46) - PLID 46082
void CEmrItemStampExclusions::FindDifferences(CEmrItemStampExclusions *pCompare, OUT CArray<long,long> *parynNewStampIDs, OUT CArray<long,long> *parynRemovedStampIDs)
{
	POSITION pos = m_mapStampIDs.GetStartPosition();
	while(pos != NULL)
	{
		long nStampID;
		bool b;
		m_mapStampIDs.GetNextAssoc(pos, nStampID, b);

		if(!pCompare->IsExcluded(nStampID)) {
			parynNewStampIDs->Add(nStampID);
		}
	}

	pos = pCompare->m_mapStampIDs.GetStartPosition();
	while(pos != NULL)
	{
		long nStampID;
		bool b;
		pCompare->m_mapStampIDs.GetNextAssoc(pos, nStampID, b);

		if(!this->IsExcluded(nStampID)) {
			parynRemovedStampIDs->Add(nStampID);
		}
	}
}

// (z.manning 2011-10-24 11:46) - PLID 46082
CString CEmrItemStampExclusions::GetStampText()
{
	if(m_mapStampIDs.GetCount() == 0) {
		return "";
	}

	CString strStampText;
	POSITION pos = m_mapStampIDs.GetStartPosition();
	while(pos != NULL)
	{
		long nStampID;
		bool b;
		m_mapStampIDs.GetNextAssoc(pos, nStampID, b);

		EMRImageStamp *pStamp = GetMainFrame()->GetEMRImageStampByID(nStampID);
		if(pStamp != NULL) {
			strStampText += pStamp->strStampText + ", ";
		}
	}

	strStampText.Delete(strStampText.GetLength() - 2, 2);

	return strStampText;
}

// End class CEmrItemStampExclusions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////