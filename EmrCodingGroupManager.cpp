#include "stdafx.h"
#include "EmrCodingGroupManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// (z.manning 2011-07-05 11:05) - PLID 44421 - Created classes to store info about EMR coding groups


CEmrCodingGroupDetail::CEmrCodingGroupDetail(const long nID, const long nCptID, const EEmrCodingQuantityTypes eType, const long nQuantityValue)
{
	m_nID = nID;
	m_nCptCodeID = nCptID;
	m_eType = eType;
	m_nQuantityValue = nQuantityValue;
}

CEmrCodingGroupDetail::~CEmrCodingGroupDetail(void)
{
}

long CEmrCodingGroupDetail::GetID()
{
	return m_nID;
}

long CEmrCodingGroupDetail::GetCptCodeID()
{
	return m_nCptCodeID;
}

EEmrCodingQuantityTypes CEmrCodingGroupDetail::GetType()
{
	return m_eType;
}

long CEmrCodingGroupDetail::GetQuantityValue()
{
	return m_nQuantityValue;
}

void CEmrCodingGroupDetail::UpdateQuantity(const EEmrCodingQuantityTypes eType, const long nQuantityValue)
{
	ExecuteParamSql(
		"UPDATE EmrCodingGroupDetailsT SET QuantityType = {INT}, QuantityValue = {INT} WHERE ID = {INT}"
		, eType, nQuantityValue, GetID());

	m_eType = eType;
	m_nQuantityValue = nQuantityValue;
}

// (z.manning 2011-07-11 16:15) - PLID 44469
long CEmrCodingGroupDetail::GetCptQuantityFromGroupQuantity(const long nGroupQuantity)
{
	if(m_eType == ecqtQuantity)
	{
		return m_nQuantityValue;
	}
	else if(m_eType == ecqtSubtractFromTotal)
	{
		long nCptQuantity = nGroupQuantity - m_nQuantityValue;
		if(nCptQuantity <= 0) {
			// (z.manning 2011-07-11 16:20) - PLID 44469 - Failsafe
			nCptQuantity = 1;
		}
		return nCptQuantity;
	}

	// (z.manning 2011-07-11 16:20) - PLID 44469 - Unhandled type
	ASSERT(FALSE);
	return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEmrCodingGroupDetailArray::CEmrCodingGroupDetailArray(void)
{
}

CEmrCodingGroupDetailArray::~CEmrCodingGroupDetailArray(void)
{
}

void CEmrCodingGroupDetailArray::Clear()
{
	for(int nDetailIndex = GetCount() - 1; nDetailIndex >= 0; nDetailIndex--) {
		ClearElementByIndex(nDetailIndex);
	}
}

void CEmrCodingGroupDetailArray::ClearElementByIndex(const int nIndex)
{
	if(nIndex >= 0 && nIndex < GetCount()) {
		delete GetAt(nIndex);
		RemoveAt(nIndex);
	}
}

CEmrCodingGroupDetail* CEmrCodingGroupDetailArray::FindByCptID(const long nCptID)
{
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEmrCodingGroupDetail *pCodingDetail = GetAt(nDetailIndex);
		if(pCodingDetail->GetCptCodeID() == nCptID) {
			return pCodingDetail;
		}
	}
	return NULL;
}

void CEmrCodingGroupDetailArray::DeleteDetail(CEmrCodingGroupDetail *pCodingDetailToDelete)
{
	for(int nDetailIndex = 0; nDetailIndex < GetCount(); nDetailIndex++)
	{
		CEmrCodingGroupDetail *pTemp = GetAt(nDetailIndex);
		if(pTemp == pCodingDetailToDelete)
		{
			ExecuteParamSql("DELETE FROM EmrCodingGroupDetailsT WHERE ID = {INT}", pCodingDetailToDelete->GetID());
			ClearElementByIndex(nDetailIndex);
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEmrCodingRange::CEmrCodingRange(const long nRangeID, const long nMinQuantity)
{
	m_nID = nRangeID;
	m_nMinQuantity = nMinQuantity;
}

CEmrCodingRange::~CEmrCodingRange(void)
{
}

void CEmrCodingRange::Clear()
{
	m_arypDetails.Clear();
}

CEmrCodingGroupDetail* CEmrCodingRange::AddDetail(const long nID, const long nCptID, const EEmrCodingQuantityTypes eType, const long nQuantityValue)
{
	CEmrCodingGroupDetail *pCodingDetail = new CEmrCodingGroupDetail(nID, nCptID, eType, nQuantityValue);
	m_arypDetails.Add(pCodingDetail);
	return pCodingDetail;
}

CEmrCodingGroupDetail* CEmrCodingRange::CreateNewDetail(const long nCptID)
{
	const EEmrCodingQuantityTypes eDefaultType = ecqtSubtractFromTotal;
	const long nDefaultQuantityValue = 0;
	ADODB::_RecordsetPtr prsInsert = CreateParamRecordset(
		"SET NOCOUNT ON \r\n"
		"DECLARE @nNewDetailID int \r\n"
		"INSERT INTO EmrCodingGroupDetailsT (EmrCodingGroupRangeID, CptCodeID, QuantityType, QuantityValue) \r\n"
		"VALUES ({INT}, {INT}, {INT}, {INT}) \r\n"
		"SET @nNewDetailID = CONVERT(int, SCOPE_IDENTITY()) \r\n"
		"SET NOCOUNT OFF \r\n"
		"SELECT @nNewDetailID AS NewDetailID \r\n"
		, GetID(), nCptID, eDefaultType, nDefaultQuantityValue);
	
	long nNewDetailID = AdoFldLong(prsInsert, "NewDetailID");
	CEmrCodingGroupDetail *pNewCodingDetail = AddDetail(nNewDetailID, nCptID, eDefaultType, nDefaultQuantityValue);
	return pNewCodingDetail;
}

void CEmrCodingRange::DeleteDetail(CEmrCodingGroupDetail *pCodingDetail)
{
	m_arypDetails.DeleteDetail(pCodingDetail);
}

long CEmrCodingRange::GetID()
{
	return m_nID;
}

long CEmrCodingRange::GetMinQuantity()
{
	return m_nMinQuantity;
}

int CEmrCodingRange::GetDetailCount()
{
	return m_arypDetails.GetCount();
}

CEmrCodingGroupDetail* CEmrCodingRange::GetDetailByIndex(const int nIndex)
{
	return m_arypDetails.GetAt(nIndex);
}

CEmrCodingGroupDetail* CEmrCodingRange::FindByCptID(const long nCptID)
{
	return m_arypDetails.FindByCptID(nCptID);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEmrCodingRangeArray::CEmrCodingRangeArray(void)
{
}

CEmrCodingRangeArray::~CEmrCodingRangeArray(void)
{
}

void CEmrCodingRangeArray::ClearAllRanges()
{
	for(int nRangeIndex = GetCount() - 1; nRangeIndex >= 0; nRangeIndex--) {
		ClearElementByIndex(nRangeIndex);
	}
}

void CEmrCodingRangeArray::ClearElementByIndex(const int nIndex)
{
	if(nIndex >= 0 && nIndex < GetCount()) {
		CEmrCodingRange *pRange = GetAt(nIndex);
		pRange->Clear();
		delete pRange;
		RemoveAt(nIndex);
	}
}

void CEmrCodingRangeArray::GetAllCptCodeIDs(OUT CArray<long,long> *parynCptCodeIDs)
{
	for(int nRangeIndex = 0; nRangeIndex < GetCount(); nRangeIndex++)
	{
		CEmrCodingRange *pCodingRange = GetAt(nRangeIndex);
		for(int nDetailIndex = 0; nDetailIndex < pCodingRange->GetDetailCount(); nDetailIndex++)
		{
			CEmrCodingGroupDetail *pCodingDetail = pCodingRange->GetDetailByIndex(nDetailIndex);
			const long nCptCodeID = pCodingDetail->GetCptCodeID();
			if(!IsIDInArray(nCptCodeID, *parynCptCodeIDs)) {
				parynCptCodeIDs->Add(nCptCodeID);
			}
		}
	}
}

void CEmrCodingRangeArray::AddSorted(CEmrCodingRange* pNewCodingRange)
{
	// (z.manning 2011-07-06 10:08) - We need to keep this in order with each group range ordered by quantity
	for(int nRangeIndex = 0; nRangeIndex < GetCount(); nRangeIndex++)
	{
		CEmrCodingRange *pTemp = GetAt(nRangeIndex);
		if(pNewCodingRange->GetMinQuantity() < pTemp->GetMinQuantity()) {
			InsertAt(nRangeIndex, pNewCodingRange);
			return;
		}
	}

	// (z.manning 2011-07-06 10:19) - This must be the first range for this group so add it at the end
	Add(pNewCodingRange);
}

CEmrCodingRange* CEmrCodingRangeArray::FindByMinQuantity(const long nMinQuantity)
{
	for(int nRangeIndex = 0; nRangeIndex < GetCount(); nRangeIndex++)
	{
		CEmrCodingRange *pCodingRange = GetAt(nRangeIndex);
		if(pCodingRange->GetMinQuantity() == nMinQuantity) {
			return pCodingRange;
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEmrCodingGroup::CEmrCodingGroup(const long nGroupID, LPCTSTR strName)
{
	m_nID = nGroupID;
	m_strName = strName;
}

CEmrCodingGroup::~CEmrCodingGroup(void)
{
}

long CEmrCodingGroup::GetID()
{
	return m_nID;
}

CString CEmrCodingGroup::GetName()
{
	return m_strName;
}

CEmrCodingRange* CEmrCodingGroup::CreateNewRange(const long nMinQuantity)
{
	ADODB::_RecordsetPtr prsInsert = CreateParamRecordset(
		"SET NOCOUNT ON \r\n"
		"DECLARE @nNewRangeID int \r\n"
		"INSERT INTO EmrCodingGroupRangesT (EmrCodingGroupID, MinQuantity) VALUES ({INT}, {INT}) \r\n"
		"SET @nNewRangeID = CONVERT(int, SCOPE_IDENTITY()) \r\n"
		"SET NOCOUNT OFF \r\n"
		"SELECT @nNewRangeID AS NewRangeID \r\n"
		, GetID(), nMinQuantity);
	long nNewRangeID = AdoFldLong(prsInsert, "NewRangeID");
	CEmrCodingRange *pNewCodingRange = new CEmrCodingRange(nNewRangeID, nMinQuantity);
	m_arypRanges.AddSorted(pNewCodingRange);
	return pNewCodingRange;
}

void CEmrCodingGroup::DeleteRange(const long nRangeID)
{
	for(int nRangeIndex = m_arypRanges.GetCount() - 1; nRangeIndex >= 0; nRangeIndex--)
	{
		CEmrCodingRange *pCodingRange = m_arypRanges.GetAt(nRangeIndex);
		if(pCodingRange->GetID() == nRangeID) {
			m_arypRanges.ClearElementByIndex(nRangeIndex);
		}
	}

	ExecuteParamSql(
		"DECLARE @nRangeID int \r\n"
		"SET @nRangeID = {INT} \r\n"
		"\r\n"
		"DELETE EmrCodingGroupDetailsT \r\n"
		"FROM EmrCodingGroupRangesT \r\n"
		"INNER JOIN EmrCodingGroupDetailsT ON EmrCodingGroupRangesT.ID = EmrCodingGroupDetailsT.EmrCodingGroupRangeID \r\n"
		"WHERE EmrCodingGroupRangesT.ID = @nRangeID \r\n"
		"\r\n"
		"DELETE EmrCodingGroupRangesT \r\n"
		"FROM EmrCodingGroupRangesT \r\n"
		"WHERE EmrCodingGroupRangesT.ID = @nRangeID \r\n"
		, nRangeID);
}

void CEmrCodingGroup::RenameGroup(LPCTSTR strNewName)
{
	ExecuteParamSql("UPDATE EmrCodingGroupsT SET Name = {STRING} WHERE ID = {INT}", strNewName, GetID());
	m_strName = strNewName;
}

void CEmrCodingGroup::Clear()
{
	m_arypRanges.ClearAllRanges();
}

CEmrCodingRange* CEmrCodingGroup::AddRange(const long nRangeID, const long nMinQuantity)
{
	CEmrCodingRange *pCodingRange = new CEmrCodingRange(nRangeID, nMinQuantity);
	m_arypRanges.AddSorted(pCodingRange);
	return pCodingRange;
}

CEmrCodingRange* CEmrCodingGroup::FindByMinQuantity(const long nMinQuantity)
{
	return m_arypRanges.FindByMinQuantity(nMinQuantity);
}

// (j.jones 2011-07-08 15:44) - PLID 38366 - FindByCurQuantity
// will return the range that should be used for a given quantity,
// where nCurQuantity >= the range's nMinQuantity, but less than
// the nMinQuantity of the next range.
CEmrCodingRange* CEmrCodingGroup::FindByCurQuantity(const long nCurQuantity)
{
	//this would only be zero if we are removing the last code in the group
	if(nCurQuantity <= 0) {
		return NULL;
	}

	CEmrCodingRange *pCodingRangeToUse = NULL;
	long nMaxQuantityFound = -1;

	for(int nRangeIndex = 0; nRangeIndex < m_arypRanges.GetCount(); nRangeIndex++)
	{
		CEmrCodingRange *pCodingRangeToCheck = m_arypRanges.GetAt(nRangeIndex);

		long nMinQuanity = pCodingRangeToCheck->GetMinQuantity();
		if(nMinQuanity <= nCurQuantity
			&& nMinQuanity > nMaxQuantityFound) {

			//this is the best range we have found so far
			pCodingRangeToUse = pCodingRangeToCheck;
			nMaxQuantityFound = nMinQuanity;
		}
	}

	return pCodingRangeToUse;
}

void CEmrCodingGroup::GetAllCptCodeIDs(OUT CArray<long,long> *parynCptCodeIDs)
{
	m_arypRanges.GetAllCptCodeIDs(parynCptCodeIDs);
}

// (j.jones 2011-07-08 11:58) - PLID 38366 - HasCptCodeID returns TRUE
// if the given service ID exists in this group
BOOL CEmrCodingGroup::HasCptCodeID(long nServiceID)
{
	for(int nRangeIndex = 0; nRangeIndex < m_arypRanges.GetCount(); nRangeIndex++)
	{
		CEmrCodingRange *pCodingRange = m_arypRanges.GetAt(nRangeIndex);
		for(int nDetailIndex = 0; nDetailIndex < pCodingRange->GetDetailCount(); nDetailIndex++)
		{
			CEmrCodingGroupDetail *pCodingDetail = pCodingRange->GetDetailByIndex(nDetailIndex);
			const long nCptCodeID = pCodingDetail->GetCptCodeID();
			if(nCptCodeID == nServiceID) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

int CEmrCodingGroup::GetRangeCount()
{
	return m_arypRanges.GetCount();
}

CEmrCodingRange* CEmrCodingGroup::GetRangeByIndex(const int nIndex)
{
	return m_arypRanges.GetAt(nIndex);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEmrCodingGroup* CEmrCodingGroupArray::CreateNewGroup(LPCTSTR strName)
{
	const long nDefaultMinQuantity = 1;
	ADODB::_RecordsetPtr prsInsert = CreateParamRecordset(
		"SET NOCOUNT ON \r\n"
		"DECLARE @nNewGroupID int, @nNewRangeID int \r\n"
		"INSERT INTO EmrCodingGroupsT (Name) VALUES ({STRING}) \r\n"
		"SET @nNewGroupID = CONVERT(int, SCOPE_IDENTITY()) \r\n"
		"INSERT INTO EmrCodingGroupRangesT (EmrCodingGroupID, MinQuantity) VALUES (@nNewGroupID, {INT}) \r\n"
		"SET @nNewRangeID = CONVERT(int, SCOPE_IDENTITY()) \r\n"
		"SET NOCOUNT OFF \r\n"
		"SELECT @nNewGroupID AS NewGroupID, @nNewRangeID AS NewRangeID \r\n"
		, strName, nDefaultMinQuantity);
	long nNewGroupID = AdoFldLong(prsInsert, "NewGroupID");
	long nNewRangeID = AdoFldLong(prsInsert, "NewRangeID");
	CEmrCodingGroup *pNewCodingGroup = new CEmrCodingGroup(nNewGroupID, strName);
	Add(pNewCodingGroup);
	pNewCodingGroup->AddRange(nNewRangeID, nDefaultMinQuantity);
	return pNewCodingGroup;
}

void CEmrCodingGroupArray::ReloadAllGroups()
{
	ClearAll();

	ADODB::_RecordsetPtr prs = CreateParamRecordset(
		"SELECT EmrCodingGroupsT.ID AS GroupID, EmrCodingGroupRangesT.ID AS RangeID, Name, MinQuantity, \r\n"
		"	EmrCodingGroupDetailsT.ID AS DetailID, CptCodeID, QuantityType, QuantityValue \r\n"
		"FROM EmrCodingGroupsT \r\n"
		"INNER JOIN EmrCodingGroupRangesT ON EmrCodingGroupsT.ID = EmrCodingGroupRangesT.EmrCodingGroupID \r\n"
		"LEFT JOIN EmrCodingGroupDetailsT ON EmrCodingGroupRangesT.ID = EmrCodingGroupDetailsT.EmrCodingGroupRangeID \r\n"
		"ORDER BY EmrCodingGroupsT.ID, EmrCodingGroupRangesT.ID, MinQuantity \r\n"
		);
	for(; !prs->eof; prs->MoveNext())
	{
		const long nGroupID = AdoFldLong(prs, "GroupID");
		CEmrCodingGroup *pGroup = new CEmrCodingGroup(nGroupID, AdoFldString(prs,"Name"));
		Add(pGroup);

		do
		{
			_variant_t varRangeID = prs->GetFields()->GetItem("RangeID")->GetValue();
			if(varRangeID.vt != VT_NULL)
			{
				const long nRangeID = VarLong(varRangeID);
				const long nMinQuantity = AdoFldLong(prs, "MinQuantity");
				CEmrCodingRange *pCodingRange = pGroup->AddRange(nRangeID, nMinQuantity);

				do
				{
					_variant_t varDetailID = prs->GetFields()->GetItem("DetailID")->GetValue();
					if(varDetailID.vt != VT_NULL)
					{
						const long nDetailID = VarLong(varDetailID);
						const long nCptID = AdoFldLong(prs, "CptCodeID");
						const BYTE nQuantityType = AdoFldByte(prs, "QuantityType");
						const long nQuantityValue = AdoFldLong(prs, "QuantityValue");
						pCodingRange->AddDetail(nDetailID, nCptID, (EEmrCodingQuantityTypes)nQuantityType, nQuantityValue);
					}

					prs->MoveNext();

				}while(!prs->eof && nRangeID == AdoFldLong(prs, "RangeID"));
				// (z.manning 2011-07-05 12:12) - We looped until we found a new group so move back one now so that
				// we process this group the next time through the loop.
				prs->MovePrevious();
			}

			prs->MoveNext();

		}while(!prs->eof && nGroupID == AdoFldLong(prs, "GroupID"));
		// (z.manning 2011-07-05 12:12) - We looped until we found a new group so move back one now so that
		// we process this group the next time through the loop.
		prs->MovePrevious();
	}
}

void CEmrCodingGroupArray::DeleteGroup(const long nGroupID)
{
	for(int nGroupIndex = GetCount() - 1; nGroupIndex >= 0; nGroupIndex--)
	{
		CEmrCodingGroup *pCodingGroup = GetAt(nGroupIndex);
		if(pCodingGroup->GetID() == nGroupID) {
			ClearElementByIndex(nGroupIndex);
		}
	}

	ExecuteParamSql(
		"DECLARE @nGroupID int \r\n"
		"SET @nGroupID = {INT} \r\n"
		"\r\n"
		"DELETE EmrCodingGroupDetailsT \r\n"
		"FROM EmrCodingGroupsT \r\n"
		"INNER JOIN EmrCodingGroupRangesT ON EmrCodingGroupsT.ID = EmrCodingGroupRangesT.EmrCodingGroupID \r\n"
		"INNER JOIN EmrCodingGroupDetailsT ON EmrCodingGroupRangesT.ID = EmrCodingGroupDetailsT.EmrCodingGroupRangeID \r\n"
		"WHERE EmrCodingGroupsT.ID = @nGroupID \r\n"
		"\r\n"
		"DELETE EmrCodingGroupRangesT \r\n"
		"FROM EmrCodingGroupsT \r\n"
		"INNER JOIN EmrCodingGroupRangesT ON EmrCodingGroupsT.ID = EmrCodingGroupRangesT.EmrCodingGroupID \r\n"
		"WHERE EmrCodingGroupsT.ID = @nGroupID \r\n"
		"\r\n"
		"DELETE EmrCodingGroupsT \r\n"
		"FROM EmrCodingGroupsT \r\n"
		"WHERE EmrCodingGroupsT.ID = @nGroupID \r\n"
		, nGroupID);
}

BOOL CEmrCodingGroupArray::DoesNameAlreadyExist(const CString &strName)
{
	for(int nGroupIndex = 0; nGroupIndex < GetCount(); nGroupIndex++)
	{
		CEmrCodingGroup *pCodingGroup = GetAt(nGroupIndex);
		if(pCodingGroup->GetName().CompareNoCase(strName) == 0) {
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CEmrCodingGroupArray::IsNameValid(const CString &strName, CWnd *pwndParent)
{
	if(strName.SpanExcluding(" ").IsEmpty()) {
		pwndParent->MessageBox("Blank group names are not allowed.", NULL, MB_OK|MB_ICONERROR);
		return FALSE;
	}

	if(DoesNameAlreadyExist(strName)) {
		pwndParent->MessageBox("There is already a group with that name.", NULL, MB_OK|MB_ICONERROR);
		return FALSE;
	}

	return TRUE;
}

// (j.jones 2011-07-11 15:12) - PLID 44509 - given a group ID, return its name
CString CEmrCodingGroupArray::GetGroupNameByID(const long nGroupID)
{
	for(int nGroupIndex = 0; nGroupIndex < GetCount(); nGroupIndex++)
	{
		CEmrCodingGroup *pCodingGroup = GetAt(nGroupIndex);
		if(pCodingGroup->GetID() == nGroupID) {
			return pCodingGroup->GetName();
		}
	}

	return "";
}

void CEmrCodingGroupArray::ClearElementByIndex(const int nIndex)
{
	if(nIndex >= 0 && nIndex < GetCount()) {
		CEmrCodingGroup *pGroup = GetAt(nIndex);
		pGroup->Clear();
		delete pGroup;
		RemoveAt(nIndex);
	}
}

void CEmrCodingGroupArray::ClearAll()
{
	for(int nGroupIndex = GetCount() - 1; nGroupIndex >= 0; nGroupIndex--) {
		ClearElementByIndex(nGroupIndex);
	}
}

// (z.manning 2011-07-12 16:18) - PLID 44469
CEmrCodingGroup* CEmrCodingGroupArray::GetCodingGroupByCptID(const long nServiceID)
{
	for(int nGroupIndex = 0; nGroupIndex < GetCount(); nGroupIndex++)
	{
		CEmrCodingGroup *pCodingGroup = GetAt(nGroupIndex);
		if(pCodingGroup->HasCptCodeID(nServiceID)) {
			return pCodingGroup;
		}
	}

	return NULL;
}

// (z.manning 2011-07-14 10:15) - PLID 44469
BOOL CEmrCodingGroupArray::Contains(const CEmrCodingGroup *pCodingGroup)
{
	for(int nGroupIndex = 0; nGroupIndex < GetCount(); nGroupIndex++)
	{
		if(GetAt(nGroupIndex) == pCodingGroup) {
			return TRUE;
		}
	}

	return FALSE;
}