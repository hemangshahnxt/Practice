// (c.haag 2011-03-14) - PLID 41163 - Initial implementation
//
// EMR Info "Common Lists" are collections of items that allow for rapid data entry by the user. For example,
// the Current Medications EMR item can have several Common Lists of medications that appear at the top
// of the item as buttons. When a user presses one of those buttons, a multi-select window appears with
// the common list content in it. A user can then quickly make choices from that list rather than having to
// choose from the entire master medications list.
//
#pragma once
#include <afxtempl.h>

class CEmrInfoDataElement;
class CEmrInfoDataElementArray;

// (c.haag 2011-03-14) - PLID 41163 - This class represents a single item in an EMR Info common list.
// It can be a single medication or a single allergy.
class CEmrInfoCommonListItem
{
private:
	long m_nID;

	// The EMR Data element for this item. Though the EmrInfoCommonListItemsT table has EmrDataID's, we require
	// CEmrInfoDataElement objects because a user could add new rows to the EMR item and assign them to common
	// lists. Those new rows all have EmrDataID's of -1.
	//
	// In CEmrItemEntryDlg::Load, we initially populate the list with EMR Data ID's. However, as the user proceeds to
	// edit common lists, those data ID's get deprecated with CEmrInfoDataElement objects corresponding to
	// CEmrItemEntryDlg::m_aryCurDataElements. In the saving, we will check for the presence of an element first.
	// If it's present, we use that in the saving logic. If absent, we look to the EmrDataID.
	//
	CEmrInfoDataElement* m_pElement;

	// The EMR Data ID for this item. This is always a valid value unless the user added this item from the
	// EmrItemEntryDlg.
	long m_nEmrDataID;

	// The EMR data value for this item. We need this when loading patient charts or templates to populate the
	// multi-select list content when the user clicks on a common list button.
	CString m_strData;

public:
	// Constructors and operators
	CEmrInfoCommonListItem();
	CEmrInfoCommonListItem(CEmrInfoDataElement* pElement);
	CEmrInfoCommonListItem(const CEmrInfoCommonListItem& src);
	CEmrInfoCommonListItem(ADODB::FieldsPtr& f, BOOL bHasDataValue);
	void operator=(const CEmrInfoCommonListItem& src);

public:
	// Property functions
	long GetID() const;
	void SetID(long nID);
	CEmrInfoDataElement* GetEmrInfoDataElement() const;
	long GetEmrDataID() const;
	void SetEmrDataID(long nEmrDataID);
	CString GetData() const;
};

// (c.haag 2011-03-14) - PLID 41163 - This class represents a Common List. You see it in a system medications
// or allergies EMR item in the form of a button.
class CEmrInfoCommonList
{
private:
	long m_nID;
	CString m_strName; // Name
	COLORREF m_clr; // Color
	BOOL m_bInactive; // Inactive flag
	long m_nOrderID; // Ordinal
	BOOL m_bGroupOnPreviewPane; // (c.haag 2011-04-11) - PLID 43155 - GroupOnPreviewPane
	CArray<CEmrInfoCommonListItem,CEmrInfoCommonListItem&> m_aItems; // The array of all items in the list
	// Note: This would probably be better as a map for fast lookups, but we can implement that on a need-to basis.

public:
	// Constructors and operators
	CEmrInfoCommonList();
	CEmrInfoCommonList(const CEmrInfoCommonList& src);
	CEmrInfoCommonList(ADODB::FieldsPtr& f);
	void operator=(const CEmrInfoCommonList& src);

private:
	// Returns the index of an item in this list given the list item ID
	int GetItemIndex(long nID) const;

public:
	// Returns TRUE if this is a new list
	BOOL IsNew() const;

public:
	// Returns TRUE if an item exists with a certain EMR data element
	BOOL DoesExist(const CEmrInfoDataElement* pElement) const;
	// Returns TRUE if an item exists with a certain EMR data ID
	BOOL DoesExist(long nEmrDataID) const;

public:
	// Adds an item to this list. The second parameter is TRUE if EmrDataT.Data is present in the fields list.
	void AddItem(ADODB::FieldsPtr f, BOOL bHasDataValue);
	void AddItem(CEmrInfoDataElement* pElement);
	// Deletes an item from this list given an EMR Info Data Element
	void DeleteItem(CEmrInfoDataElement* pDoomedElement);

public:
	// Returns the number of items in this list
	int GetItemCount() const;

public:
	long GetID() const;
	void SetID(long nID);
	CString GetName() const;
	void SetName(const CString& strName);
	COLORREF GetColor() const;
	void SetColor(COLORREF clr);
	long GetOrderID() const;
	void SetOrderID(long nOrderID);
	BOOL IsInactive() const;
	void SetInactive(BOOL bInactive);
	// (c.haag 2011-04-11) - PLID 43155 - GroupOnPreviewPane
	BOOL GetGroupOnPreviewPane() const;
	void SetGroupOnPreviewPane(BOOL bValue);

public:
	// Returns an item in this list
	CEmrInfoCommonListItem GetItemByIndex(int nIndex) const;

public:
	// (c.haag 2011-03-15) - PLID 42821 - Be able to change the internal ID's in the event the EMR item was copied
	void UpdateIDs(const EMRInfoChangedCommonListItem eiccli);

public:
	// Remove all the items
	void Clear();
};

// (c.haag 2011-03-14) - PLID 41163 - This is the master class for loading, maintaining, and saving
// common lists for a specific Emr Info item.
class CEmrInfoCommonListCollection
{
private:
	// The array of all custom buttons
	CArray<CEmrInfoCommonList,CEmrInfoCommonList&> m_aLists; // The array of all custom lists
	CArray<CEmrInfoCommonList,CEmrInfoCommonList&> m_aDeletedLists; // The array of all deleted custom lists. We need to keep all the info
																								// for auditing purposes and remapping.

private:
	// The next new list ID (always a negative number)
	long m_nNextNewListID;

public:
	// Constructors and operators
	CEmrInfoCommonListCollection();
	CEmrInfoCommonListCollection(const CEmrInfoCommonListCollection& src);
	void operator=(const CEmrInfoCommonListCollection& src);

private:
	// Returns the ordinal of a custom list given its ID
	int GetListArrayIndexFromID(long nID) const;

public:
	// Clears all content in this object
	void Clear();
	// Loads lists and list items for a given EMR Info ID
	void Load(ADODB::_ConnectionPtr& pCon, long nEmrInfoID);
	// Loads a single custom list. This will not populate the list items
	void LoadList(ADODB::FieldsPtr& f);
	// Loads custom lists from a recordset. This will not populate the list items
	void LoadLists(ADODB::_RecordsetPtr& prs);
	// Loads a single custom list item. The second parameter is TRUE if EmrDataT.Data is present in the fields list.
	void LoadListItem(ADODB::FieldsPtr& f, BOOL bHasDataValue);
	// Loads custom list items from a recordset
	void LoadListItems(ADODB::_RecordsetPtr& prs);
	// Deletes a list
	void DeleteList(long nID);
	// Deletes an item from all lists given an element object
	void DeleteItems(CEmrInfoDataElement* pDoomedElement);
	// Creates an empty list
	CEmrInfoCommonList CreateNewList();

public:
	// (c.haag 2011-03-15) - PLID 42821 - Be able to change the internal ID's in the event the EMR item was copied
	void UpdateIDs(EMRInfoChangedIDMap* pChangedMap);

public:
	// Returns the count of common lists
	int GetListCount() const;
	// Returns the count of deleted lists
	int GetDeletedListCount() const;
	// Returns a list given a zero-based index used as an iterator from 0 to GetListCount
	CEmrInfoCommonList GetListByIndex(int nIndex) const;
	// Returns a deleted list given a zero-based index used as an iterator from 0 to GetDeletedListCount
	CEmrInfoCommonList GetDeletedListByIndex(int nIndex) const;
	// Returns a list given an ID
	CEmrInfoCommonList GetListByID(int nID) const;
	// Changes list content given a list ID
	void SetListByID(int nID, const CEmrInfoCommonList& srcData);
	// Returns TRUE if a list with a given name exists
	BOOL DoesListExist(const CString& strName, long nIDToExclude) const;
};
