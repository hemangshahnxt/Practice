#pragma once

// (z.manning 2011-07-05 11:05) - PLID 44421 - Created classes to store info about EMR coding groups


// (z.manning 2011-07-05 11:22) - PLID 44421 - Saved to data, don't change existing values.
enum EEmrCodingQuantityTypes
{
	ecqtQuantity = 1,
	ecqtSubtractFromTotal = 2,
};

class CEmrCodingGroupDetail
{
public:
	CEmrCodingGroupDetail(const long nID, const long nCptID, const EEmrCodingQuantityTypes eType, const long nQuantityValue);
	~CEmrCodingGroupDetail(void);

	long GetID();
	long GetCptCodeID();
	EEmrCodingQuantityTypes GetType();
	long GetQuantityValue();

	void UpdateQuantity(const EEmrCodingQuantityTypes eType, const long nQuantityValue);

	// (z.manning 2011-07-11 16:15) - PLID 44469
	long GetCptQuantityFromGroupQuantity(const long nGroupQuantity);

protected:
	long m_nID;
	long m_nCptCodeID;
	EEmrCodingQuantityTypes m_eType;
	long m_nQuantityValue;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CEmrCodingGroupDetailArray : public CArray<CEmrCodingGroupDetail*,CEmrCodingGroupDetail*>
{
public:
	CEmrCodingGroupDetailArray(void);
	~CEmrCodingGroupDetailArray(void);

	void Clear();
	void ClearElementByIndex(const int nIndex);

	CEmrCodingGroupDetail* FindByCptID(const long nCptID);

	void DeleteDetail(CEmrCodingGroupDetail *pCodingDetailToDelete);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CEmrCodingRange
{
public:
	CEmrCodingRange(const long nRangeID, const long nMinQuantity);
	~CEmrCodingRange(void);

	void Clear();

	CEmrCodingGroupDetail* CreateNewDetail(const long nCptID);
	CEmrCodingGroupDetail* AddDetail(const long nID, const long nCptID, const EEmrCodingQuantityTypes eType, const long nQuantityValue);
	void DeleteDetail(CEmrCodingGroupDetail *pCodingDetail);

	long GetID();
	long GetMinQuantity();

	int GetDetailCount();
	CEmrCodingGroupDetail* GetDetailByIndex(const int nIndex);

	CEmrCodingGroupDetail* FindByCptID(const long nCptID);

protected:
	long m_nID;
	long m_nMinQuantity;
	CEmrCodingGroupDetailArray m_arypDetails;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CEmrCodingRangeArray: public CArray<CEmrCodingRange*,CEmrCodingRange*>
{
public:
	CEmrCodingRangeArray(void);
	~CEmrCodingRangeArray(void);

	void ClearAllRanges();
	void ClearElementByIndex(const int nIndex);

	void GetAllCptCodeIDs(OUT CArray<long,long> *parynCptCodeIDs);

	CEmrCodingRange* FindByMinQuantity(const long nMinQuantity);

	void AddSorted(CEmrCodingRange* pNewCodingRange);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CEmrCodingGroup
{
public:
	CEmrCodingGroup(const long nGroupID, LPCTSTR strName);
	~CEmrCodingGroup(void);

	int GetRangeCount();
	CEmrCodingRange* GetRangeByIndex(const int nIndex);
	
	CEmrCodingRange* CreateNewRange(const long nMinQuantity);
	void DeleteRange(const long nRangeID);
	void RenameGroup(LPCTSTR strNewName);

	CEmrCodingRange* AddRange(const long nRangeID, const long nMinQuantity);

	void Clear();

	long GetID();
	CString GetName();
	void SetName(LPCTSTR strName);

	CEmrCodingRange* FindByMinQuantity(const long nMinQuantity);

	// (j.jones 2011-07-08 15:44) - PLID 38366 - FindByCurQuantity
	// will return the range that should be used for a given quantity,
	// where nCurQuantity >= the range's nMinQuantity, but less than
	// the nMinQuantity of the next range.
	CEmrCodingRange* FindByCurQuantity(const long nCurQuantity);

	void GetAllCptCodeIDs(OUT CArray<long,long> *parynCptCodeIDs);
	
	// (j.jones 2011-07-08 11:58) - PLID 38366 - HasCptCodeID returns TRUE
	// if the given service ID exists in this group
	BOOL HasCptCodeID(long nServiceID);

protected:
	long m_nID;
	CString m_strName;
	CEmrCodingRangeArray m_arypRanges;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CEmrCodingGroupArray : public CArray<CEmrCodingGroup*,CEmrCodingGroup*>
{
public:
	void ReloadAllGroups();

	CEmrCodingGroup* CreateNewGroup(LPCTSTR strName);
	void DeleteGroup(const long nGroupID);

	void ClearAll();
	void ClearElementByIndex(const int nIndex);
	
	BOOL DoesNameAlreadyExist(const CString &strName);
	BOOL IsNameValid(const CString &strName, CWnd *pwndParent);

	// (j.jones 2011-07-11 15:12) - PLID 44509 - given a group ID, return its name
	CString GetGroupNameByID(const long nGroupID);

	// (z.manning 2011-07-12 16:18) - PLID 44469
	CEmrCodingGroup* GetCodingGroupByCptID(const long nServiceID);

	// (z.manning 2011-07-14 10:15) - PLID 44469
	BOOL Contains(const CEmrCodingGroup *pCodingGroup);
};