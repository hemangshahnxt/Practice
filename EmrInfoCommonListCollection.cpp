// (c.haag 2011-03-14) - PLID 41163 - Initial implementation
//
// EMR Info "Common Lists" are collections of items that allow for rapid data entry by the user. For example,
// the Current Medications EMR item can have several Common Lists of medications that appear at the top
// of the item as buttons. When a user presses one of those buttons, a multi-select window appears with
// the common list content in it. A user can then quickly make choices from that list rather than having to
// choose from the entire master medications list.
//

#include "stdafx.h"
#include "EmrInfoCommonListCollection.h"
#include "EmrItemEntryDlg.h"

using namespace ADODB;

///////////////////////////////////////////////////////////////////////////////
// CEmrInfoCommonListItem implementation

CEmrInfoCommonListItem::CEmrInfoCommonListItem()
{
	m_nID = -1;
	m_nEmrDataID = -1;
	m_pElement = NULL;
}

CEmrInfoCommonListItem::CEmrInfoCommonListItem(CEmrInfoDataElement* pElement)
{
	m_nID = -1;
	m_nEmrDataID = -1;
	m_pElement = pElement;
}

CEmrInfoCommonListItem::CEmrInfoCommonListItem(const CEmrInfoCommonListItem& src)
{
	*this = src;
}

CEmrInfoCommonListItem::CEmrInfoCommonListItem(ADODB::FieldsPtr& f, BOOL bHasDataValue)
{
	// None of these fields are nullable
	m_nID = AdoFldLong(f, "ID");
	m_nEmrDataID = AdoFldLong(f, "EmrDataID");
	m_pElement = NULL;
	if (bHasDataValue) {
		m_strData = AdoFldString(f, "Data");
	}
}

void CEmrInfoCommonListItem::operator=(const CEmrInfoCommonListItem& src)
{
	m_nID = src.m_nID;
	m_nEmrDataID = src.m_nEmrDataID;
	m_pElement = src.m_pElement;
	m_strData = src.m_strData;
}

long CEmrInfoCommonListItem::GetID() const
{
	return m_nID;
}

void CEmrInfoCommonListItem::SetID(long nID)
{
	m_nID = nID;
}

CEmrInfoDataElement* CEmrInfoCommonListItem::GetEmrInfoDataElement() const
{
	return m_pElement;
}

long CEmrInfoCommonListItem::GetEmrDataID() const
{
	return m_nEmrDataID;
}

void CEmrInfoCommonListItem::SetEmrDataID(long nEmrDataID)
{
	// This should never be called if we have an element pointer
	if (NULL != m_pElement) {
		ThrowNxException("Called CEmrInfoCommonListItem::SetEmrDataID for an item with a non-null element pointer!");
	}
	m_nEmrDataID = nEmrDataID;
}

CString CEmrInfoCommonListItem::GetData() const
{
	return m_strData;
}

///////////////////////////////////////////////////////////////////////////////
// CEmrInfoCommonList implementation

CEmrInfoCommonList::CEmrInfoCommonList()
{
	m_nID = -1;
	m_strName = "";
	m_clr = 0;
	m_bInactive = FALSE;
	m_nOrderID = 0;
	// (c.haag 2011-04-11) - PLID 43155 - GroupOnPreviewPane
	m_bGroupOnPreviewPane = TRUE;
}

CEmrInfoCommonList::CEmrInfoCommonList(const CEmrInfoCommonList& src)
{
	*this = src;
}

CEmrInfoCommonList::CEmrInfoCommonList(FieldsPtr& f)
{
	// All fields are non-nullable
	m_nID = AdoFldLong(f, "ID");
	m_strName = AdoFldString(f, "Name");
	m_clr = (COLORREF)AdoFldLong(f, "Color");
	m_bInactive = AdoFldBool(f, "Inactive");
	m_nOrderID = AdoFldLong(f, "OrderID");	
	m_bGroupOnPreviewPane = AdoFldBool(f, "GroupOnPreviewPane"); // (c.haag 2011-04-11) - PLID 43155 - GroupOnPreviewPane
}

void CEmrInfoCommonList::operator=(const CEmrInfoCommonList& src)
{
	m_nID = src.m_nID;
	m_strName = src.m_strName;
	m_clr = src.m_clr;
	m_bInactive = src.m_bInactive;
	m_nOrderID = src.m_nOrderID;	
	m_bGroupOnPreviewPane = src.m_bGroupOnPreviewPane; // (c.haag 2011-04-11) - PLID 43155 - GroupOnPreviewPane
	int nItems = src.m_aItems.GetSize();
	m_aItems.RemoveAll();
	for (int i=0; i < nItems; i++)
	{
		CEmrInfoCommonListItem item = src.m_aItems[i];
		m_aItems.Add(item);
	}
}

int CEmrInfoCommonList::GetItemCount() const
{
	return m_aItems.GetSize();
}

// Returns the index of an item in this list given the list item ID
int CEmrInfoCommonList::GetItemIndex(long nID) const
{
	for (int i=0; i < m_aItems.GetSize(); i++)
	{
		if (m_aItems[i].GetID() == nID) {
			return i;
		}
	}
	ThrowNxException("Could not find EMR Info common list item ID %d!", nID);
}

// Returns TRUE if this is a new list
BOOL CEmrInfoCommonList::IsNew() const
{
	return (m_nID < 0) ? TRUE : FALSE;
}

// Returns TRUE if an item exists with a certain EMR data element
BOOL CEmrInfoCommonList::DoesExist(const CEmrInfoDataElement* pElement) const
{
	for (int i=0; i < m_aItems.GetSize(); i++)
	{
		// There's two possibilities for this item:
		// 1. The item was modified by the user in CEmrItemEntryDlg and therefore has a CEmrInfoDataElement object
		// 2. The item was not modified by the user and still retains an original EMR Data ID back from CEmrItemEntryDlg::Load
		const CEmrInfoDataElement* pItemElement = m_aItems[i].GetEmrInfoDataElement();
		if (NULL != pItemElement)
		{
			// Possibility 1 is true. Test against the element; they should both be from CEmrInfoDataElement::m_aryCurDataElements
			if (pItemElement == pElement) {
				return TRUE;
			}
		}
		else 
		{
			// Possibility 2 is true. Just test against the EMR Data ID
			if (m_aItems[i].GetEmrDataID() == pElement->m_nID)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

// Returns TRUE if an item exists with a certain EMR data ID
BOOL CEmrInfoCommonList::DoesExist(long nEmrDataID) const
{
	for (int i=0; i < m_aItems.GetSize(); i++)
	{
		// There's two possibilities for this item:
		// 1. The item was modified by the user in CEmrItemEntryDlg and therefore has a CEmrInfoDataElement object
		// 2. The item was not modified by the user and still retains an original EMR Data ID back from CEmrItemEntryDlg::Load
		const CEmrInfoDataElement* pItemElement = m_aItems[i].GetEmrInfoDataElement();
		if (NULL != pItemElement)
		{
			// Possibility 1 is true. Test the element's Emr Data ID
			if (pItemElement->m_nID == nEmrDataID) {
				return TRUE;
			}
		}
		else 
		{
			// Possibility 2 is true. Just test against the EMR Data ID
			if (m_aItems[i].GetEmrDataID() == nEmrDataID)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

// Adds an item to this list. The second parameter is TRUE if EmrDataT.Data is present in the fields list.
void CEmrInfoCommonList::AddItem(FieldsPtr f, BOOL bHasDataValue)
{
	CEmrInfoCommonListItem item(f, bHasDataValue);
	m_aItems.Add(item);
}

// Adds an item to this list
void CEmrInfoCommonList::AddItem(CEmrInfoDataElement* pElement)
{
	CEmrInfoCommonListItem item(pElement);
	m_aItems.Add(item);
}

// Deletes an item from this list given an EMR Info Data Element
void CEmrInfoCommonList::DeleteItem(CEmrInfoDataElement* pDoomedElement)
{
	for (int i=0; i < m_aItems.GetSize(); i++)
	{
		// There's two possibilities for this item:
		// 1. The item was modified by the user in CEmrItemEntryDlg and therefore has a CEmrInfoDataElement object
		// 2. The item was not modified by the user and still retains an original EMR Data ID back from CEmrItemEntryDlg::Load
		const CEmrInfoDataElement* pItemElement = m_aItems[i].GetEmrInfoDataElement();
		if (NULL != pItemElement)
		{
			// Possibility 1 is true. Test the element
			if (pItemElement == pDoomedElement) {
				// Found it; delete it.
				m_aItems.RemoveAt(i);
				// A list can never have more than one of an Emr Data ID, so quit now.
				return;
			}
		}
		else if (-1 != pDoomedElement->m_nID)
		{
			// Possibility 2 is true. Just test against the EMR Data ID
			if (m_aItems[i].GetEmrDataID() == pDoomedElement->m_nID)
			{
				// Found it; delete it.
				m_aItems.RemoveAt(i);
				// A list can never have more than one of an Emr Data ID, so quit now.
				return;
			}
		}
	}
}

long CEmrInfoCommonList::GetID() const
{
	return m_nID;
}

void CEmrInfoCommonList::SetID(long nID)
{
	m_nID = nID;
}

CString CEmrInfoCommonList::GetName() const
{
	return m_strName;
}

void CEmrInfoCommonList::SetName(const CString& strName)
{
	m_strName = strName;
}

COLORREF CEmrInfoCommonList::GetColor() const
{
	return m_clr;
}

void CEmrInfoCommonList::SetColor(COLORREF clr)
{
	m_clr = clr;
}

long CEmrInfoCommonList::GetOrderID() const
{
	return m_nOrderID;
}

void CEmrInfoCommonList::SetOrderID(long nOrderID)
{
	m_nOrderID = nOrderID;
}

BOOL CEmrInfoCommonList::IsInactive() const
{
	return m_bInactive;
}

void CEmrInfoCommonList::SetInactive(BOOL bInactive)
{
	m_bInactive = bInactive;
}

// (c.haag 2011-04-11) - PLID 43155 - GroupOnPreviewPane
BOOL CEmrInfoCommonList::GetGroupOnPreviewPane() const
{
	return m_bGroupOnPreviewPane;
}

// (c.haag 2011-04-11) - PLID 43155 - GroupOnPreviewPane
void CEmrInfoCommonList::SetGroupOnPreviewPane(BOOL bValue)
{
	m_bGroupOnPreviewPane = bValue;
}

// Returns an item in this list
CEmrInfoCommonListItem CEmrInfoCommonList::GetItemByIndex(int nIndex) const
{
	return m_aItems[nIndex];
}

// (c.haag 2011-03-15) - PLID 42821 - Be able to change the internal ID's in the event the EMR item was copied
void CEmrInfoCommonList::UpdateIDs(const EMRInfoChangedCommonListItem eiccli)
{
	// This list may have been modified by the user, in which case there can be items
	// with -1 ID's. So, we have to brute force through all the list and update ID's
	// accordingly.
	int nItems = m_aItems.GetSize();
	for (int i=0; i < nItems; i++)
	{
		if (m_aItems[i].GetID() > 0 && m_aItems[i].GetID() == eiccli.nOldID)
		{
			m_aItems[i].SetID(eiccli.nNewID);
		}
		if (m_aItems[i].GetEmrDataID() == eiccli.nOldDataID) 
		{
			m_aItems[i].SetEmrDataID(eiccli.nNewDataID);
		}
	}
}

// Remove all the items
void CEmrInfoCommonList::Clear()
{
	m_aItems.RemoveAll();
}

///////////////////////////////////////////////////////////////////////////////
// CEmrInfoCommonListCollection implementation

CEmrInfoCommonListCollection::CEmrInfoCommonListCollection()
{
	m_nNextNewListID = -1;
}

CEmrInfoCommonListCollection::CEmrInfoCommonListCollection(const CEmrInfoCommonListCollection& src)
{
	*this = src;
}

void CEmrInfoCommonListCollection::operator=(const CEmrInfoCommonListCollection& src)
{
	Clear();
	int nLists = src.m_aLists.GetSize();
	int i;
	for (i=0; i < nLists; i++)
	{
		CEmrInfoCommonList list = src.m_aLists[i];
		m_aLists.Add(list);
	}
	nLists = src.m_aDeletedLists.GetSize();
	for (i=0; i < nLists; i++)
	{
		CEmrInfoCommonList list = src.m_aDeletedLists[i];
		m_aDeletedLists.Add(list);
	}
	m_nNextNewListID = src.m_nNextNewListID;
}

// Returns the ordinal of a custom list given its ID
int CEmrInfoCommonListCollection::GetListArrayIndexFromID(long nID) const
{
	for (int i=0; i < m_aLists.GetSize(); i++) {
		if (m_aLists[i].GetID() == nID) {
			return i;
		}
	}
	ThrowNxException("Could not find EMR Info common list ID %d!", nID);
}

// Clears all content in this object
void CEmrInfoCommonListCollection::Clear()
{
	m_aLists.RemoveAll();
}

// Loads lists and list items for a given EMR Info ID. (Note: The only place where inactive items
// are loaded is EmrItemEntryDlg)
void CEmrInfoCommonListCollection::Load(ADODB::_ConnectionPtr& pCon, long nEmrInfoID)
{
	Clear();
	// (c.haag 2011-04-11) - PLID 43155 - GroupOnPreviewPane
	_RecordsetPtr prs = CreateParamRecordset(pCon, 
			"SELECT EmrInfoCommonListT.ID, EmrInfoCommonListT.EmrInfoID, EmrInfoCommonListT.Name, \r\n"
			"EmrInfoCommonListT.Color, EmrInfoCommonListT.OrderID, EmrInfoCommonListT.Inactive, \r\n"
			"EmrInfoCommonListT.GroupOnPreviewPane \r\n"
			"FROM EmrInfoCommonListT \r\n"
			"WHERE EmrInfoID = {INT} AND Inactive = 0 \r\n"
			"ORDER BY EmrInfoCommonListT.OrderID "
			"SELECT EmrInfoCommonListItemsT.ID, EmrInfoCommonListItemsT.ListID, EmrInfoCommonListItemsT.EmrDataID, "
			"EmrInfoCommonListT.EmrInfoID, EmrDataT.Data \r\n"
			"FROM EmrInfoCommonListT \r\n"
			"INNER JOIN EmrInfoCommonListItemsT ON EmrInfoCommonListItemsT.ListID = EmrInfoCommonListT.ID \r\n"
			"INNER JOIN EmrDataT ON EmrDataT.ID = EmrInfoCommonListItemsT.EmrDataID "
			"WHERE EmrInfoCommonListT.EmrInfoID = {INT} AND EmrInfoCommonListT.Inactive = 0 AND EmrDataT.Inactive = 0\r\n"
			,nEmrInfoID
			,nEmrInfoID);
	LoadLists(prs);
	prs = prs->NextRecordset(NULL);
	LoadListItems(prs);
}

// Loads a single custom list. This will not populate the list items
void CEmrInfoCommonListCollection::LoadList(FieldsPtr& f)
{
	CEmrInfoCommonList list(f);
	m_aLists.Add(list);
}

// Loads custom lists from a recordset. This will not populate the list items
void CEmrInfoCommonListCollection::LoadLists(_RecordsetPtr& prs)
{
	FieldsPtr f = prs->Fields;
	while (!prs->eof) {
		LoadList(f);
		prs->MoveNext();
	}
}

// Loads a single custom list item. The second parameter is TRUE if EmrDataT.Data is present in the fields list.
void CEmrInfoCommonListCollection::LoadListItem(FieldsPtr& f, BOOL bHasDataValue)
{
	const long nListID = AdoFldLong(f, "ListID");
	int nListIndex = GetListArrayIndexFromID(nListID);
	m_aLists[nListIndex].AddItem(f, bHasDataValue);
}

// Loads custom list items from a recordset
void CEmrInfoCommonListCollection::LoadListItems(_RecordsetPtr& prs)
{
	FieldsPtr f = prs->Fields;
	long nFields = f->Count;
	BOOL bHasDataValue = FALSE; // TRUE if EmrDataT.Data is present in the fields list.
	for (long i=0; i < nFields && !bHasDataValue; i++) 
	{
		CString strFieldName =  (LPCTSTR)f->Item[i]->Name;
		if (strFieldName == "Data")
		{
			bHasDataValue = TRUE;
		}
	}
	while (!prs->eof)
	{
		LoadListItem(f, bHasDataValue);
		prs->MoveNext();
	}
}

// Deletes a list
void CEmrInfoCommonListCollection::DeleteList(long nID)
{
	int nOrdinal = GetListArrayIndexFromID(nID);
	CEmrInfoCommonList doomedList = m_aLists[nOrdinal];
	m_aLists.RemoveAt(nOrdinal);
	if (nID > 0) {
		// Only add lists that exist in data into the deleted lists array
		m_aDeletedLists.Add(doomedList);
	}
}

// Deletes an item from all lists given an element object
void CEmrInfoCommonListCollection::DeleteItems(CEmrInfoDataElement* pDoomedElement)
{
	for (int i=0; i < m_aLists.GetSize(); i++)
	{
		m_aLists[i].DeleteItem(pDoomedElement);
		// If deleting this element renders the list empty, then delete the list too. Even new
		// lists have unique ID's (-1, -2, -3...) so GetID() should be perfectly valid here.
		if (0 == m_aLists[i].GetItemCount()) {
			DeleteList(m_aLists[i--].GetID());
		}
	}
}

// Creates an empty list
CEmrInfoCommonList CEmrInfoCommonListCollection::CreateNewList()
{
	CEmrInfoCommonList newList;
	newList.SetID( m_nNextNewListID-- );
	newList.SetName("[New List]");
	// Don't set the Order ID; it should be generated when the caller finalizes changes to the collection
	m_aLists.Add(newList);
	return newList;
}

// (c.haag 2011-03-15) - PLID 42821 - Be able to change the internal ID's in the event the EMR item was copied
void CEmrInfoCommonListCollection::UpdateIDs(EMRInfoChangedIDMap* pChangedMap)
{
	int i,j;

	// Change list ID's (EmrInfoCommonListT.ID)
	for (i=0; i < pChangedMap->aryChangedCommonListIDs.GetSize(); i++)
	{
		const EMRInfoChangedCommonList eiccl = pChangedMap->aryChangedCommonListIDs[i];
		// Ensure the ID is updated in the lists
		for (j=0; j < m_aLists.GetSize(); j++) 
		{
			CEmrInfoCommonList l = m_aLists[j];
			if (l.GetID() == eiccl.nOldID) {
				l.SetID(eiccl.nNewID);
				m_aLists[j] = l;
				break;
			}
		}
		// Ensure the ID is updated in the deleted lists
		for (j=0; j < m_aDeletedLists.GetSize(); j++) 
		{
			CEmrInfoCommonList l = m_aDeletedLists[j];
			if (l.GetID() == eiccl.nOldID)
			{
				l.SetID(eiccl.nNewID);
				m_aDeletedLists[j] = l;
				break;
			}
		}
	}

	// Change list item ID and EmrDataID's
	for (i=0; i < pChangedMap->aryChangedCommonListDetailIDs.GetSize(); i++)
	{
		const EMRInfoChangedCommonListItem eiccli = pChangedMap->aryChangedCommonListDetailIDs[i];

		// Ensure the items are updated in the lists
		for (j=0; j < m_aLists.GetSize(); j++) 
		{
			CEmrInfoCommonList l = m_aLists[j];
			if (l.GetID() == eiccli.nNewListID) {
				l.UpdateIDs(eiccli);
				m_aLists[j] = l;
				break;
			}
		}
		// Ensure the items are updated in the deleted lists.
		for (j=0; j < m_aDeletedLists.GetSize(); j++) 
		{
			CEmrInfoCommonList l = m_aDeletedLists[j];
			if (l.GetID() == eiccli.nNewListID) {
				l.UpdateIDs(eiccli);
				m_aDeletedLists[j] = l;
				break;
			}
		}
	}
}

// Returns the count of common lists
int CEmrInfoCommonListCollection::GetListCount() const
{
	return m_aLists.GetSize();
}

// Returns the count of deleted lists
int CEmrInfoCommonListCollection::GetDeletedListCount() const
{
	return m_aDeletedLists.GetSize();
}

// Returns a list given a zero-based index used as an iterator from 0 to GetListCount
CEmrInfoCommonList CEmrInfoCommonListCollection::GetListByIndex(int nIndex) const
{
	return m_aLists[nIndex];
}

// Returns a deleted list given a zero-based index used as an iterator from 0 to GetDeletedListCount
CEmrInfoCommonList CEmrInfoCommonListCollection::GetDeletedListByIndex(int nIndex) const
{
	return m_aDeletedLists[nIndex];
}

// Returns a list given an ID
CEmrInfoCommonList CEmrInfoCommonListCollection::GetListByID(int nID) const
{
	return m_aLists[GetListArrayIndexFromID(nID)];
}

// Updates a list given an ID
void CEmrInfoCommonListCollection::SetListByID(int nID, const CEmrInfoCommonList& srcData)
{
	m_aLists[GetListArrayIndexFromID(nID)] = srcData;
}

// Returns TRUE if a list with a given name exists
BOOL CEmrInfoCommonListCollection::DoesListExist(const CString& strName, long nIDToExclude) const
{
	for (int i=0; i < m_aLists.GetSize(); i++)
	{
		if (nIDToExclude != m_aLists[i].GetID() &&
			!strName.CompareNoCase( m_aLists[i].GetName() )) {
			return TRUE;
		}
	}
	return FALSE;
}
