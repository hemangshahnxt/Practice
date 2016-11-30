#pragma once

#include "InvVisionWebUtils.h"

// (a.walling 2010-03-08 11:48) - PLID 37640 - Moved a lot of EMN Detail releated classes that are not CEMNDetail to EMNDetailStructures

class CEMN;
struct DetailPopup;
class CEMNDetail;
class CStamp;
class CEmrItemAdvImageState;	// (j.armen 2014-07-21 16:32) - PLID 62836

enum GlassesOrderDataType;

// (r.gonet 08/03/2012) - PLID 51948 - Defines the special EMRDataT column types used in CWoundCareCalculator
enum EWoundCareDataType;

struct ListElement {
	long nID; //EmrDataT.ID
	long nDataGroupID; //TES 2/9/2007 - PLID 24761 - EmrDataT.EmrDataGroupID
	CString strName;
	BOOL bIsLabel;
	BOOL bIsSelected; //Ignored if bIsLabel is true.
	CString strLongForm;
	long nActionsType; //-1 = none, 1 = has EMR items or Mint Items, 2 = has other actions but not EMR items or Mint Items

	// (j.jones 2011-04-28 14:39) - PLID 43122 - track if this list item is floated,
	// which will require that we bold the text
	BOOL bIsFloated;

	//TES 1/23/2008 - PLID 28673 - This represents a multipopup dialog that was spawned by this list element in this
	// session, which can be restored by passing this pointer into it.  As of the current checkin, this should never
	// be set, but that's only temporary.
	DetailPopup* pDetailPopup;

	//TES 3/17/2011 - PLID 41108 - Added godtGlassesOrderDataType and nGlassesOrderDataID
	GlassesOrderDataType godtGlassesOrderDataType;
	long nGlassesOrderDataID;

	// (z.manning 2011-11-07 17:03) - PLID 46309
	CString strSpawnedItemsSeparator;

	// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
	long nParentLabelID;

	///

	static const CString defaultLongForm; // "<Data>" // prevents allocation of string for "<Data>" for each list element by referencing the existing CString

	// (a.walling 2012-10-12 15:05) - PLID 53165 - Added default constructor
	ListElement()
		: nID(-1)
		, nDataGroupID(-1)
		, bIsLabel(FALSE)
		, bIsSelected(FALSE)
		, strLongForm(defaultLongForm)
		, nActionsType(-1)
		, pDetailPopup(NULL)
		, godtGlassesOrderDataType(godtInvalid)
		, nGlassesOrderDataID(-1)
		// (j.jones 2011-04-28 14:39) - PLID 43122 - track if this list item is floated,
		// which will require that we bold the text
		, bIsFloated(FALSE)
		, nParentLabelID(-1)
	{}
};

// (a.walling 2012-12-03 12:50) - PLID 53983 - EmrListState provides a quick way to access elements by ID, as well as get child elements for hide/show
struct EmrListState
{
	EmrListState()
	{}

	EmrListState(CEMNDetail* pDetail)
	{
		Load(pDetail);
	}

	std::map<long, ListElement> elements;
	std::multimap<long, long> children;
	std::set<long> selected;

	void Reset();
	
	void Load(CEMNDetail* pDetail);
};


// (z.manning 2011-10-11 09:16) - PLID 42061 - Simple struct for table dropdown item
struct TableDropdownItem
{
public:
	long nID;
	CString strText;
	BOOL bVisible;

protected:
	CMap<long,long,bool,bool> mapStampsIDs;

public:
	TableDropdownItem();
	TableDropdownItem(const TableDropdownItem &item);
	TableDropdownItem(const long nDropdownID, LPCTSTR strDropdownText, const BOOL bDropdownVisible);

	TableDropdownItem& operator=(const TableDropdownItem &source);

	CString GetComboText() const;

	void AddStamp(const long nStampID);
	// (a.walling 2013-07-24 11:18) - PLID 57698 - const
	BOOL HasStampID(const long nStampID) const;
};

// (z.manning 2011-10-11 09:19) - PLID 42061 - New class for an array of dropdown items
class CTableDropdownItemArray : public CArray<TableDropdownItem,TableDropdownItem&>
{
public:
	// (a.walling 2013-07-24 11:18) - PLID 57698 - const
	CString GetComboText(const long nStampID) const;
};


// (c.haag 2008-11-25 09:40) - PLID 31693 - We used to store only detail pointers in CEMNDetail::m_arLinkedDetails.
// Now we now store both the detail pointer and its position in the narrative state.
struct LinkedDetailStruct {
	CEMNDetail* pDetail;
	DWORD nStartPos; // The detail's position in the narrative state
	BOOL bIsNarrativeDuplicate; // (c.haag 2008-12-02 16:22) - PLID 32296 - Now we always cache duplicate narrative
								// details; but, we need to know which ones are the duplicates in case the caller doesn't
								// want them
	long nDataID; // If this is -1, then the merge field represents an EMR Item. If it's a positive number, then the merge field
				// actually represents a list option
};

// (z.manning 2010-02-17 11:38) - PLID 37412 - Struct to store info about detail image stamps
struct EmrDetailImageStamp
{
	long nID;
	long nLoadedFromID;	// (j.jones 2010-03-03 12:36) - PLID 37231
	long nStampID;
	long nOrderIndex;
	EMRSmartStampTableSpawnRule eRule;
	long x;
	long y;
	long m_nRefCnt;
	BOOL bUsedInTableData; // (z.manning 2011-01-27 14:42) - PLID 42335
	// (z.manning 2011-09-08 09:40) - PLID 45335 - Added fields needed for 3D stamps
	_variant_t var3DPosX;
	_variant_t var3DPosY;
	_variant_t var3DPosZ;
	_variant_t varNormalX;
	_variant_t varNormalY;
	_variant_t varNormalZ;
	short n3DHotSpotID;
	// (a.walling 2012-10-31 17:17) - PLID 53552 - LastSavedDetail - LastSavedStamps no longer necessary
	//EmrDetailImageStamp *pLastSavedImageStamp;

protected:
	// (z.manning 2013-03-12 17:00) - PLID 55016 - Member for the copied from detail image stamp
	EmrDetailImageStamp *pCopiedFromDetailStamp;

public:
	// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp
	EmrDetailImageStamp();

	EmrDetailImageStamp(EmrDetailImageStamp *pStampToCopy);

	// (z.manning 2010-02-23 15:15) - PLID 37412 - Constructor to load stamp from fields pointer
	EmrDetailImageStamp(ADODB::FieldsPtr pflds);

	// (z.manning 2010-02-18 14:38) - PLID 37404 - Added overloaded equality operator
	bool operator==(EmrDetailImageStamp& source);

	int AddRef();
	int Release();

	_variant_t GetXVar();
	_variant_t GetYVar();

	_variant_t Get3DHotSpotIDVar(); // (z.manning 2011-09-13 14:22) - PLID 45335

	BOOL Is3D(); // (z.manning 2011-09-13 18:59) - PLID 45335

	// (j.jones 2013-07-25 16:54) - PLID 57583 - the positioning is invalid 
	// if all positioning data (X/Y or X/Y/Z if 3D) is empty, or if neither
	// positioning dataset is complete (X with no Y, X/Y but no Z, etc.)
	bool HasInvalidPositioning();

	// (z.manning 2013-03-12 17:02) - PLID 55016
	void SetCopiedFromDetailStamp(EmrDetailImageStamp *pCopiedFrom);
	EmrDetailImageStamp* GetCopiedFromDetailStamp();

	// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
	void LogEmrObjectData(int nIndent);
};

//DRT 7/10/2007 - PLID 24105 - Removed Width.  The table widths are now saved outside of this, so that
//	destruction / reconstruction of the detail may occur without losing column widths.
struct TableColumn {
	long nID; //EmrDetailTableDataT.EmrDataID_Y
	CString strName;
	long nType;
	BYTE nSubType; // (z.manning 2010-02-16 14:29) - PLID 37230
	BOOL bIsGrouped;

	// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused

	// (z.manning, 05/23/2008) - PLID 30155 - Added Formula and DecimalPlaces
	CString m_strFormula;
	BYTE m_nDecimalPlaces;
	// (z.manning 2009-01-15 15:20) - PLID 32724 - Added InputMask
	CString m_strInputMask;
	// (c.haag 2010-02-24 15:33) - PLID 21301 - EmrDataT.AutoAlphabetizeDropdown
	BOOL m_bAutoAlphabetizeDropdown;
	// (z.manning 2010-04-13 16:05) - PLID 38175
	BOOL m_bIsLabel;
	// (z.manning 2010-07-29 15:02) - PLID 36150
	CString m_strLongForm;
	BOOL m_bHasDropdownElements; // (z.manning 2011-03-11) - PLID 42778
	BOOL m_bHasActiveDropdownElements; // (z.manning 2011-03-17 15:05) - PLID 42778
	EmrTableAutofillType m_eAutofillType; // (z.manning 2011-03-21 11:21) - PLID 30608
	long m_nFlags; // (z.manning 2011-05-26 14:46) - PLID 43865

	// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
	EEmrTableAutoNumberType m_etantAutoNumberType;
	CString m_strAutoNumberPrefix;

	//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
	GlassesOrderDataType m_GlassesOrderDataType;
	long m_nGlassesOrderDataID;

	// (z.manning 2011-09-19 14:43) - PLID 41954 - Dropdown separators
	CString m_strDropdownSeparator;
	CString m_strDropdownSeparatorFinal;

	// (z.manning 2011-11-07 10:45) - PLID 46309
	CString m_strSpawnedItemsSeparator;

	// (z.manning 2008-06-11 16:31) - PLID 30155 - This array keeps track of any values that need to be
	// part of the embedded combo dropdown contents for this column.
	// (z.manning 2011-10-11 09:43) - PLID 42061 - This now uses the new CTableDropdownItemArray class
	CTableDropdownItemArray aryAdditionalEmbeddedComboValues;

	//(e.lally 2011-12-08) PLID 46471 - Tracks whether or not this is the official column for marking a current medication (Rx) or allergy (Yes) as in use
	//	This only applies to the current medication and allergies details.
	BOOL m_bIsCurrentMedOrAllergyUsageCol;

	// (r.gonet 08/03/2012) - PLID 51948 - Stores which column this is in Wound Care Coding Calculation.
	//  Please note that there is no corresponding value for TableRow because only fields can be special to the calculation.
	EWoundCareDataType m_ewccWoundCareDataType;

protected:
	// (z.manning 2011-10-12 09:51) - PLID 45728 - We now support default selections on table dropdowns when making 
	// a new smart stamp table row. This map stores such selections.
	CMap<long,long,CArray<long,long>*,CArray<long,long>*> m_mapStampIDToDefaultTableDropdownIDs;
	
public:
	// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp
	TableColumn();
	~TableColumn();

	// (z.manning 2008-06-11 16:33) - PLID 30155 - Added assignment operator and copy constructor
	TableColumn& TableColumn::operator=(const TableColumn &source);

	TableColumn(const TableColumn &tc);

	BOOL IsCalculated();
	BOOL IsReadOnly(); // (z.manning 2010-04-19 16:11) - PLID 38228

	// (z.manning 2008-06-11 16:33) - PLID 30155 - To get the ID for a calculated embedded combo value
	// we make the array index negative then subtract 100000.
	long GetEmbeddedComboIDFromArrayIndex(const long nArrayIndex);

	long GetArrayIndexFromEmbeddedComboID(const long nEmbeddedComboID);

	// (z.manning 2008-06-11 16:34) - PLID 30155 - This will return the embedded combo ID for the specified value
	// or 0 if the value is not yet part of the array.
	long GetEmbeddedComboIDFor(const CString strValue);

	// (z.manning 2008-06-11 16:35) - PLID 30155 - This function is to add a value to the embedded combo
	// for this column.
	long AddEmbeddedComboValue(const CString strValue, const BOOL bVisible);

	// (z.manning 2008-06-25 14:48) - PLID 30155 - Returns true if the specified embedded combo ID
	// is one of the additional values we store here.
	BOOL IsAdditionalEmbeddedComboID(IN const long nEmbeddedComboID, OUT CString &strValue);

	// (z.manning 2011-03-17 15:20) - PLID 42618
	BOOL CanHaveDropdownElements();
	BOOL ShouldLoadDropdownElementsFromData();

	// (z.manning 2011-03-22 09:41) - PLID 30608
	BOOL AllowAutofill();

	// (z.manning 2011-05-26 17:39) - PLID 42131
	BOOL ShouldShowPlusSign();

	// (z.manning 2011-05-27 10:00) - PLID 42131
	BOOL HasTransformFormula();

	// (z.manning 2011-10-12 14:58) - PLID 45728
	void ClearStampDefaultDropdownInfo();
	void AddStampDefaultDropdownID(const long nStampID, const long nDropdownInfoID);
	CArray<long,long>* GetDefaultDropdownIDsForStampID(const long nSampID);
};

// (z.manning 2010-02-19 09:13) - PLID 37427 - Table rows are no longer necessarily tied to an EMRDataT
// record, so now we have a struct for the table row ID to keep track of all possible ways of identifying
// them.
struct TableRowID
{
	// (c.haag 2012-10-26) - PLID 53440 - Moved all smart stamp variables into a private section. There are
	// three kinds of ways a stamp can persist in TableRowID:
	//
	// - The table row doesn't have a stamp: (nEmrDetailImageStampID = -1, nStampID = -1, pDetailImageStamp = null)
	// - The table row has a stamp but we didn't create the stamp object in memory yet: (nEmrDetailImageStampID != -1, nStampID != -1, pDetailImageStamp = null)
	// - The table row has a stamp and the stamp object is in memory  (nEmrDetailImageStampID = -1, nStampID = -1, pDetailImageStamp != null)
	//
	// By having this structure micromanage it, we avoid issues with nEmrDetailImageStampID and nStampID going outt
	// of sync with pDetailImageStamp.
	//
private:
	long nEmrDetailImageStampID;
	EmrDetailImageStamp *pDetailImageStamp;
	//TES 3/17/2010 - PLID 37530 - Added, this is the global ID for this stamp.
	long nStampID;

public:
	long nDataID;
	//TES 3/17/2010 - PLID 37530 - Added, this represents the index of this stamp within its current detail, based on the stamp type
	// (i.e., is this the first or second AK).
	long nStampIndexInDetailByType;

	// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp
	TableRowID();

	~TableRowID();

	// (z.manning 2010-03-17 14:58) - For safety, force users to use the 4 long overload
	TableRowID(const long _nDataID, const long _nEmrDetailImageStampID, const long _nStampIndexInDetailByType);

	TableRowID(const long _nDataID, const long _nEmrDetailImageStampID, const long _nEmrDetailImageStampPointer, const long _nStampID, const long _nStampIndexInDetailByType);

	TableRowID(EmrDetailImageStamp *p, const long _nStampIndexInDetailByType);

	TableRowID& TableRowID::operator=(const TableRowID &source);

	bool operator==(TableRowID& source);

	bool operator!=(TableRowID& source);

	// (c.haag 2012-10-26) - PLID 53440 - This will assign the stamp object to this table row
	void SetDetailImageStamp(EmrDetailImageStamp *pStamp);
	// (c.haag 2012-10-26) - PLID 53440 - Returns the smart stamp object. This can be null despite GetDetailImageStamp()
	// not returning -1 in the event that the stamp object hasn't been created in memory yet.
	EmrDetailImageStamp* GetDetailImageStampObject() const;
	// (c.haag 2012-10-26) - PLID 53440 - This will get the detail image stamp ID
	long GetDetailImageStampID() const;
	// (c.haag 2012-10-26) - PLID 53440 - This will set the detail image stamp ID. If this table row contains a valid
	// EmrDetailImageStamp object, it will be assigned this new value.
	void SetDetailImageStampID(long ID);
	// (c.haag 2012-10-26) - PLID 53440 - This will return the image stamp ID (not the detail ID)
	long GetImageStampID() const;
	// (c.haag 2012-10-26) - PLID 53440 - This will set the image stamp ID. If this table row contains a valid
	// EmrDetailImageStamp object, it will be assigned this new value.
	void SetImageStampID(long ID);
	// (c.haag 2012-10-26) - PLID 53440 - This will reset the detail image stamp ID
	void SetNew();
	// (c.haag 2012-10-26) - PLID 53440 - This will return true if this table row is associated with an image stamp
	// whether by ID or by pointer.
	BOOL HasDetailImageStamp() const;

	// (a.walling 2011-08-11 15:49) - PLID 44987 - for ordered associated containers
	// (c.haag 2012-10-26) - PLID 53440 - We now use the property function to compare
	bool operator<(const TableRowID& r) const
	{	
		if (nDataID != r.nDataID) {
			return nDataID < r.nDataID;
		}
		if (GetDetailImageStampID() != r.GetDetailImageStampID()) {
			return GetDetailImageStampID() < r.GetDetailImageStampID();
		}
		if (pDetailImageStamp != r.pDetailImageStamp) {
			return pDetailImageStamp < r.pDetailImageStamp;
		}

		return false;
	}
};

struct TableRow
{
	// (z.manning 2010-02-18 10:03) - PLID 37427 - Table rows no longer must reference an EmrDataT record
	// as we now support dynamic rows for smart stamps. As such, I removed the old ID field and replaced
	// it with a struct to determine the "ID" of a table row;
	//long nID; //EmrDetailTableDataT.EmrDataID_X
	//TableRowID *m_pID;
	// (a.walling 2011-08-11 16:43) - PLID 45021 - This is not allocated on the heap any longer
	TableRowID m_ID;

	CString strName;
	long nGroupID; // (z.manning 2009-02-24 12:45) - PLID 33141

	// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused

	// (z.manning, 05/23/2008) - PLID 30155 - Added Formula and DecimalPlaces
	CString m_strFormula;
	BYTE m_nDecimalPlaces;
	// (z.manning 2010-04-13 16:04) - PLID 38175
	BOOL m_bIsLabel;
	long m_nFlags; // (z.manning 2011-05-26 14:47) - PLID 43865

	// (z.manning 2010-03-03 17:33) - PLID 37225 - Flag to determine if a row should be removed
	// during the next call to request state change on that table.
	BOOL m_bDeleteOnStateChange;

	//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
	GlassesOrderDataType m_GlassesOrderDataType;
	long m_nGlassesOrderDataID;

	//(e.lally 2011-12-08) PLID 46471 - Tracks whether or not this is a row on a current medication or allergy detail
	BOOL m_bIsCurrentMedOrAllergy;
	//(e.lally 2011-12-08) PLID 46471 - Tracks whether or not the official value for marking a current medication (Rx) or allergy (Yes) is check marked as in use
	//	This only applies to the current medication and allergies details so m_bIsCurrentMedOrAllergy should always be true if this is true.
	BOOL m_bIsCurrentMedOrAllergyChecked;

	// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp
	TableRow();

	// (z.manning 2010-02-24 17:20) - PLID 37532 - New constructor
	//TES 3/17/2010 - PLID 37530 - Added the global StampID, and the index of the stamp in this detail, based on the stamp type
	// (i.e., is this the first or second AK).
	TableRow(const long nDataGroupID, const long nDetailStampID, const long nStampID, const long nStampIndexInDetailByType);

	// (z.manning 2010-02-19 11:51) - PLID 37427 - Added assignment operator
	TableRow& TableRow::operator=(const TableRow &source);

	TableRow(const TableRow &trSource);

	BOOL IsCalculated();
	BOOL IsReadOnly(); // (z.manning 2010-04-19 16:11) - PLID 38228

	// (z.manning 2011-02-23 13:46) - PLID 42549
	BOOL IsSmartStampRow();

	// (z.manning 2011-05-26 17:39) - PLID 42131
	BOOL ShouldShowPlusSign();
};

struct TableElement {
	TableRow *m_pRow;
	TableColumn *m_pColumn;
	
	//If pColumn->nColumnType == LIST_TYPE_TEXT
	CString m_strValue;
	//If pColumn->nColumnType == LIST_TYPE_DROPDOWN
	//long m_nDropdownID; //EMRTableDropdownInfoT.ID
	// (c.haag 2008-01-11 17:11) - PLID 17936 - We now support multiple selections
	CArray<long,long> m_anDropdownIDs;
	//If pColumn->nColumnType = LIST_TYPE_CHECKBOX
	BOOL m_bChecked;
	//If pColumn->nColumnType == LIST_TYPE_LINKED, at least one will be valid.
	CEMNDetail *m_pLinkedDetail;
	CString m_strLinkedSentenceHtml;
	CString m_strLinkedSentenceNonHtml;
	// (c.haag 2007-08-13 09:40) - PLID 27049 - If we tried to assign a detail
	// to m_pLinkedDetail in LoadValueFromString, but no detail could be found
	// (probably because it didn't spawn yet), then we store the value here. It
	// will be used when CEMNDetail::TryRecoverMissingLinkedDetails is called.
	CString m_strMissingLinkedDetailID;
	bool m_bHtmlCached; //Did we successfully cache the HTML form?
	//TES 10/12/2012 - PLID 39000 - Is true if this is a calculated field, and one of the fields that goes into the calculation has numbers
	// in it that were not included in the calculation (for example, if you list a height as 5'6", then that would resolve to just 5, and the
	// 6 would be unused).
	BOOL m_bFormulaHasUnusedNumbers;

	void LoadValueFromString(const CString &strValue, OPTIONAL CEMN *pContainingEMN);
	CString GetValueAsString();
	// (a.walling 2007-10-19 17:55) - PLID 27820 - Flag for preview pane special case
	// (c.haag 2008-02-22 10:37) - PLID 29064 - Added optional connection pointer
	// (a.walling 2011-05-31 11:57) - PLID 42448 - pDetail cannot be NULL now for performance reasons, but may be in the future
	CString GetValueAsOutput(CEMNDetail* pDetail, bool bHtml, CStringArray &saTempFiles, bool bForPreview = FALSE, OPTIONAL IN ADODB::_Connection *lpCon = NULL);
	void LoadValueFromVariant(const _variant_t &varValue, CEMN *pContainingEMN);
	_variant_t GetValueAsVariant();

	// (a.walling 2009-03-09 10:13) - PLID 33409 - Determine if the value is empty (whether it should be saved to data or not)
	BOOL IsValueEmpty();

	// (c.haag 2007-03-16 11:50) - PLID 25242 - We now support getting EMN's passed through to GetSentence
	void ReloadSentence(OPTIONAL CEMN* pContainingEMN = NULL);

	// (c.haag 2008-01-11 17:09) - PLID 17936 - Returns TRUE if a dropdown value is selected
	BOOL IsDropdownValueSelected() const;

	// (c.haag 2008-01-11 17:13) - PLID 17936 - Parses a comma-delimited list of ID's into an array of long integers
	void ParseCommaDeliminatedText(CArray<long,long> &anIDs, CString str) const;

	// (c.haag 2008-01-11 17:28) - PLID 17936 - Copy operator
	void operator =(const TableElement &tSource);

	// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp
	TableElement();

	// (c.haag 2008-01-11 17:34) - PLID 17936 - Copy constructor
	TableElement(const TableElement& tSource);

	// (z.manning 2008-06-26 13:48) - PLID 30155 - Returns true if the element is calculated from a formula, false otherwise.
	BOOL IsCalculated();

	// (z.manning 2010-04-13 16:08) - PLID 38175 - Returns true if the element is in a row or column that's a label
	BOOL IsLabel();

	BOOL IsReadOnly(); // (z.manning 2010-04-19 16:11) - PLID 38228

	// (z.manning 2010-12-20 10:22) - PLID 41886
	void Clear();
	BOOL IsEmpty();
};

// (z.manning 2010-02-17 11:41) - PLID 37412 - Array of image stamp detail info
class CEmrDetailImageStampArray : public CArray<EmrDetailImageStamp*,EmrDetailImageStamp*>
{
protected:
	unsigned short m_nVersion;

public:
	// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp
	CEmrDetailImageStampArray();

	void Clear();

	// (z.manning 2010-02-23 12:14) - PLID 37412 - Need to be able to load the detail stamp array from a variant array.
	void LoadFromSafeArrayVariant(_variant_t varDetailStampData);

	_variant_t GetAsVariant();

	// (z.manning 2010-02-18 14:08) - PLID 37404 - Function to find detail stamp by ID
	EmrDetailImageStamp* FindByID(const long nID);

	// (z.manning 2010-02-22 12:02) - PLID 37230 - Given a table row, will attempt to find a 
	// corresponding detail image stamp based on the properies in the given row's ID.
	EmrDetailImageStamp* FindByTableRow(const TableRow *ptr);

	// (z.manning 2010-02-22 12:01) - PLID 37230 - Returns the max order index of all image detail
	// stamps plus one.
	long GetNextOrderIndex();

	// (z.manning 2010-02-22 12:00) - PLID 37230 - Returns true if an instance of an image stamp
	// with ID nStampID already exists in the given hotspot.
	BOOL DoesStampAlreadyExistInHotspot(const long nStampID, CEMRHotSpot *pHotSpot, EmrDetailImageStamp *pDetailStampToIgnore);

	// (z.manning 2010-02-24 10:42) - PLID 37225 - Given a text string, will attempt to find a 
	// corresponding detail image stamp.
	EmrDetailImageStamp* FindByTextString(const TextString ts);

	// (z.manning 2010-02-24 10:42) - PLID 37225 - Removes the specified detail image stamp
	void RemoveDetailStamp(EmrDetailImageStamp *pDetailStamp);

	// (z.manning 2010-02-24 11:54) - PLID 37225 - Returns the first detail with the same stamp ID
	EmrDetailImageStamp* FindReplacementDetailStamp(EmrDetailImageStamp *pDetailStampToReplace);
	
	// (z.manning 2011-01-20 12:01) - PLID 42338
	BOOL Contains(EmrDetailImageStamp *pDetailStamp);

	// (z.manning 2011-02-02 18:06) - PLID 42335
	long GetTotalByGlobalStampID(const long nGlobalStampID);
};

//TES 3/16/2012 - PLID 45127 - Added, pulls the logic out of CEmrDetailImageStampArray::FindByTextString,
// so that other places can compare EmrDetailImageStamps and TextStrings
bool DoStampsMatch(EmrDetailImageStamp *pDetailStamp, const TextString ts);

//DRT 7/10/2007 - PLID 24105 - For maintaining the current details table column widths.  We track on 
//	column name because a) that's how it worked before I implemented this method, and b) that presumably is
//	because IDs may not yet be saved, IDs may change, etc.  The name should remain consistent.

class CTableColumnWidths {
public:
	//Constructor
	CTableColumnWidths() {	}

	//Destructor
	// (a.walling 2010-03-08 09:46) - PLID 37640 - I'll let this slide
	~CTableColumnWidths() {
		ClearAll();
	}

	// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp
	void CopyFrom(CTableColumnWidths* pFrom);

	// (r.gonet 02/14/2013) - PLID 40017 - Compare two sets of table column widths for deep equality.
	//  Returns true if all the column widths are the same between the two sets. false otherwise.
	bool Equals(CTableColumnWidths *pOther) const;

	// (c.haag 2008-06-26 11:09) - PLID 27968 - Set the width of the first column
	// of a table; it's a non-data-entry column.
	void SetFirstColumnWidth(long nWidth);
	// (r.gonet 02/14/2013) - PLID 40017 - Sets the first column's width in the popped up table.
	void SetFirstPopupColumnWidth(long nWidth);

	// (c.haag 2008-06-26 11:38) - PLID 27968 - Returns the width of the first column
	// of a table; it's a non-data-entry column.
	long GetFirstColumnWidth() const;
	// (r.gonet 02/14/2013) - PLID 40017 - Returns the first column's width in the popped up table.
	// - bDefaultToNonPopupWidth - Returns the regular width in the non popped up version if the popped up column has no width defined.
	long GetFirstPopupColumnWidth(bool bDefaultToNonPopupWidth) const;

	//Change the width of an existing column.  Adds newly if it does not exist
	void SetWidthByName(CString strName, long nWidth);
	// (r.gonet 02/14/2013) - PLID 40017 - Sets a column's width in the popped up table.
	void SetPopupWidthByName(CString strName, long nWidth);

	//Retrieves the currently stored width of a given column
	long GetWidthByName(CString strName);
	// (r.gonet 02/14/2013) - PLID 40017 - Returns a column's width in the popped up table.
	// - bDefaultToNonPopupWidth - Returns the regular width in the non popped up version if the popped up column has no width defined.
	long GetPopupWidthByName(CString strName, bool bDefaultToNonPopupWidth);

	//Wipes out everything in the structure (erase the saved widths)
	void ClearAll();

protected:
//This struct should only be used inside this class
	struct TableColumnWidth {
		CString strColName;
		long nWidth;
		// (r.gonet 02/14/2013) - PLID 40017 - This column's width in a popped up table. -1 if there is no width defined.
		long nPopupWidth;

		// (r.gonet 02/14/2013) - PLID 40017 - Initialize the popped up column's width.
		TableColumnWidth() : 
			nWidth(-1), nPopupWidth(-1)
		{
		}
	};

	//Member array of all our widths
	CArray<TableColumnWidth*, TableColumnWidth*> m_aryWidths;

	//Helper to find a specific, rather than copying this for loop to a bunch
	//	of different functions.
	// (c.haag 2008-06-26 11:09) - PLID 27968 - Start at column 1 because column
	// 0 is the special reserved column that has no name. If we included it, then
	// we could have more than one qualifying column.
	TableColumnWidth* FindTableColumnByName(CString strName);

	//Add a new width to be tracked
	// (r.gonet 02/14/2013) - PLID 40017 - Added the width of the column in the popped up table.
	void AddWidth(CString strName, long nWidth, long nPopupWidth = -1);
};

struct TableSpawnInfo
{
	long nDropdownID;
	// (z.manning 2010-02-24 17:35) - PLID 37532 - We now have a TableRow here instead of data group ID
	TableRow tr;
	long nStampID;
	
	// (a.walling 2010-03-08 09:46) - PLID 37640 - moved implementations to .cpp
	TableSpawnInfo();

	TableSpawnInfo(const long _nDropdownID, TableRow _tr, const long _nStampID);
};

// (a.walling 2013-06-06 09:41) - PLID 57069 - Keeps track of all dependencies when generating an image's
// render output. If these items are equivalent, then the output should also be identical. We can use this
// to keep track of the last rendered image state so redundant generation can be avoided
struct CEmrItemAdvImageRenderState
{
	CEmrItemAdvImageRenderState();

	// (z.manning 2015-05-28 14:35) - PLID 66102 - Added max width
	CEmrItemAdvImageRenderState(CTextStringFilterPtr pFilter, const CEmrItemAdvImageState& ais, IN CRect rImageSize
		, IN CString strOrigBackgroundImageFilePath, IN eImageType eitOrigBackgroundImageType
		, bool bEnableEMRImageTextScaling, IN const long nMaxWidth);

	NxMD5 stateHash; // CEmrItemAdvImageState's m_hash via CreateFromSafeArrayVariant
	NxMD5 optionHash; // Currently just a hash of the CEMNDetail's TextStampFilter entries

	// other parameters to CEmrItemAdvImageState::Render
	CRect rImageSize;
	CString strOrigBackgroundImageFilePath;
	eImageType eitOrigBackgroundImageType;
	int nEnableTextScaling;
	// (z.manning 2015-05-28 14:39) - PLID 66102 - Added max width
	long nMaxWidth;

	friend bool operator==(const CEmrItemAdvImageRenderState& l, const CEmrItemAdvImageRenderState& r) 
	{
		if (l.rImageSize != r.rImageSize) {
			return false;
		}

		if (l.eitOrigBackgroundImageType != r.eitOrigBackgroundImageType || l.nEnableTextScaling != r.nEnableTextScaling) {
			return false;
		}

		if (0 != l.strOrigBackgroundImageFilePath.CompareNoCase(r.strOrigBackgroundImageFilePath)) {
			return false;
		}
		if (0 != memcmp(&l.stateHash.m_digest, &r.stateHash.m_digest, sizeof(l.stateHash.m_digest))) {
			return false;
		}
		if (0 != memcmp(&l.optionHash.m_digest, &r.optionHash.m_digest, sizeof(l.optionHash.m_digest))) {
			return false;
		}
		return true;
	}
};

// (z.manning 2011-01-20 09:53) - PLID 42338 - Added a class for an array of EMN details so that I could
// add member functions to it as needed.
class CEMNDetailArray : public CArray<CEMNDetail*,CEMNDetail*>
{
public:
	void AddRefAll(LPCTSTR strDescription);
	void ReleaseAll(LPCTSTR strDescription);

	BOOL Contains(CEMNDetail *pDetailToCheck);

	long GetStampIndexInDetailByType(EmrDetailImageStamp* pDetailStamp);

	void UpdateChildEMRTemplateDetailIDs(const long nID);
	void UpdateChildEMRDetailIDs(const long nID);

	BOOL AreSmartStampsEnabled();

	void SetSmartStampTableDetail(CEMNDetail *pSmartStampTable);

	CEMRHotSpot* GetHotSpotFromQuantityBasedSmartStamp(const long nStampID);

	void SetVisible(BOOL bVisible, BOOL bRedraw, BOOL bIsInitialLoad);

	CEMNDetail* GetDetailFromDetailImageStamp(EmrDetailImageStamp *pDetailImageStamp);
	CEMNDetail* GetDetailFromDetailStampID(const long nDetailStampID);

	CEMNDetail* GetDetailFromEmrInfoMasterID(const long nEmrInfoMasterID);

	// (z.manning 2011-01-26 12:12) - PLID 42336
	BOOL AtLeastOneVisibleDetail();

	// (z.manning 2011-01-27 15:48) - PLID 42335
	EmrDetailImageStamp* FindReplacementDetailStamp(EmrDetailImageStamp *pDetailStampToReplace);

	// (z.manning 2011-01-31 13:27) - PLID 42338
	void GetAllDetailImageStampsInOrder(OUT CEmrDetailImageStampArray *paryDetailImageStamps);

	// (z.manning 2011-01-31 15:26) - PLID 42338 - Moved this to CEMNDetail array class
	long GetNextOrderIndex();

	// (z.manning 2011-02-02 18:06) - PLID 42335
	long GetTotalByGlobalStampID(const long nGlobalStampID);

	// (z.manning 2011-02-15 09:49) - PLID 42446
	void SortWithParentDetailsFirst();

	// (z.manning 2011-03-01 15:59) - PLID 42335
	BOOL IsGlobalStampIDEverUsedInData(const long nStampID);
};


// (z.manning 2011-10-24 11:39) - PLID 46082 - Class for loading per-item stamp exclusions
class CEmrItemStampExclusions
{
protected:
	CMap<long,long,bool,bool> m_mapStampIDs;

public:
	void LoadFromData(const long nEmrInfoMasterID); // (z.manning 2011-10-25 14:55) - PLID 39401
	void LoadFromRecordset(ADODB::_RecordsetPtr prs);

	CEmrItemStampExclusions& operator=(const CEmrItemStampExclusions &source);

	void AddExclusion(const long nStampID);
	void RemoveAll();

	BOOL IsExcluded(const long nStampID);

	void FindDifferences(CEmrItemStampExclusions *pCompare, OUT CArray<long,long> *parynNewStampIDs, OUT CArray<long,long> *parynRemovedStampIDs);

	CString GetStampText();
};