#include "stdafx.h"
#include "CachedData.h"
#include <NxAlgorithm.h> // Ensure boost will pick up our CString-aware hash_value() function
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (b.cardillo 2015-05-06 00:06) - PLID 65951 - Framework for loading and checking for the existence 
// of certain values efficiently.
// (b.cardillo 2015-05-07 08:48) - PLID 65951 (supplemental) - Just fixed some comments and 
// implemented an as-yet-unused function for completeness.
// (b.cardillo 2015-05-07 14:21) - PLID 65951 (supplemental) - Made it right-trim all strings to be 
// consistent with sql; also some efficiency and template improvements.
// (b.cardillo 2015-05-10 08:35) - PLID 65951 (supplemental) - Added support for multisets and further templated.
// (b.cardillo 2015-07-16 22:04) - PLID 65951 (supplemental) - Make case insensitive. If we ever 
// use this class for some purpose that demands case sensitivity, we can easily template it.


///////////
// First some local utilities
///////////
namespace {
	// Templated VarT() functions 
	template<typename T> inline T VarTypedForCaching(const VARIANT &val) {
		// No default implementation.
		// If you get a compile error here it means you are calling Var<T>() for something other 
		// than a known implementation below. You either need to add another implementation below 
		// or reconsider the calling code to call one of the existing functions.
	}

	// Explicit implementations. Notice the string-typed one trims right; that's so we have the 
	// same behavior as sql text comparison (default collation) when looking up values in the cache.
	template<> inline long VarTypedForCaching(const VARIANT &val) { return VarLong(val); }
	template<> inline CString VarTypedForCaching(const VARIANT &val) { return VarString(val).TrimRight(); }
	template<> inline CiString VarTypedForCaching(const VARIANT &val) { return VarString(val).TrimRight(); }

	// Templated insert of values to avoid repeating this little code snippet all over
	template<typename H>
	inline void InsertNonNullTrimRight(H &hash, const VARIANT &val) {
		if (val.vt != VT_NULL) {
			hash.insert(VarTypedForCaching<H::key_type>(val));
		} else {
			// Nothing matches NULL, so don't store an entry in the hash for VT_NULL values
		}
	}
	// (b.cardillo 2015-05-08 10:12) - PLID 65988 - Added support for maps (we already had sets)
	template<typename H>
	inline void InsertNonNullTrimRight(H &hash, const VARIANT &key, const VARIANT &val) {
		if (key.vt != VT_NULL) {
			hash.insert({ VarTypedForCaching<H::key_type>(key), VarTypedForCaching<H::mapped_type>(val) });
		} else {
			// Nothing matches NULL, so don't store an entry in the hash for VT_NULL values
		}
	}

	// Returns a reference to the existing value from a map for the given key, or, if the key 
	// doesn't exist in the map yet, inserts a new default value for that key and returns the 
	// reference to the value.
	template <class K, class T, class H, class P, class A>
	inline T &FindOrCreate(boost::unordered_map<K, T, H, P, A> &map, const typename boost::unordered_map<K, T, H, P, A>::key_type &key)
	{
		// See if there's already an element
		boost::unordered_map<K, T, H, P, A>::iterator iter = map.find(key);
		if (iter != map.end()) {
			// Found it, return a reference to the value of the entry we just found
			return iter->second;
		} else {
			// We don't have a value for this key yet, so create one and return a reference to it
			T val;
			return map.insert({ key, val }).first->second;
		}
	}

	// Returns a right-trimmed version of the given string. This never modifies the original string 
	// (obviously, since the original is const), yet thanks to CString's reference counting, it will 
	// only allocate a new copy of the string if the original had trailing whitespace. Otherwise, 
	// this will do nothing and just return a reference to the original.
	inline CString TrimRight(const CString &str)
	{
		CString ans(str);
		ans.TrimRight();
		return ans;
	}
	inline CiString TrimRight(const CiString &str)
	{
		CiString ans(str);
		ans.TrimRight();
		return ans;
	}

	// These two classes each encapsulate a "context" concept for caching of different sources of 
	// data. They implement an instance function called "Fill" that populates the hash. By having 
	// this concept, we can template callers to unify calling logic into templated functions.

	// Datalist-related implementation of the "context" concept. This takes the two context-
	// identifying parameters (datalist and column) and provides the logic for inserting the values 
	// from that column of that datalist into the hash.
	class CCachedDataContext_DatalistColumn 
	{
	public:
		CCachedDataContext_DatalistColumn(NXDATALIST2Lib::_DNxDataList *pDatalist, int nColumn)
			: m_pDatalist(pDatalist), m_nColumn(nColumn) { }

	protected:
		NXDATALIST2Lib::_DNxDataListPtr m_pDatalist;
		int m_nColumn;

	public:
		// Our one job: populate the hash
		template<typename H>
		void Fill(H &hash) const {
			// Core logic of this class: loop through the datalist, add the values from the column 
			// of interest to the given hash
			for (NXDATALIST2Lib::IRowSettingsPtr pRow = this->m_pDatalist->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
				Insert(hash, pRow);
			}
		}
	protected:
		// We're the only one who knows how insert our kind of entry into a hash. For now we only 
		// implement inserting into a set (or multi-set). We could add maps later if needed.
		template<typename H>
		inline void Insert(H &hash, NXDATALIST2Lib::IRowSettingsPtr pRow) const
		{
			InsertNonNullTrimRight(hash, pRow->GetValue(this->m_nColumn));
		}
	};

	// Query-related implementation of the "context" concept. This takes the one context-
	// identifying parameter (the sql query) and provides the Fill_Impl() logic for running that 
	// query and looping through that results, inserting the value from that field into the hash.
	class CCachedDataContext_Query
	{
	public:
		CCachedDataContext_Query(const CString &sqlPopulateCache)
			: m_sqlPopulateCache(sqlPopulateCache) { }

	protected:
		const CString &m_sqlPopulateCache;

	public:
		// Our one job: populate the hash
		template<typename H>
		void Fill(H &hash) {
			// Core logic of this class: run the recordset, loop through the results, add the values 
			// to the hash
			for (_RecordsetPtr prs = CreateRecordsetStd(this->m_sqlPopulateCache); !prs->eof; prs->MoveNext()) {
				Insert(hash, prs);
			}
		}
	protected:
		// We're the only one who knows how insert our kind of entry into a hash.
		template<typename H>
		static void Insert(H &hash, _RecordsetPtr prs)
		{
			InsertNonNullTrimRight(hash, prs->GetCollect((long)0));
		}
		// (b.cardillo 2015-05-08 10:12) - PLID 65988 - Added support for maps (we already had sets)
		template<typename K, typename T>
		static void Insert(boost::unordered_map<K, T> &hash, _RecordsetPtr prs)
		{
			InsertNonNullTrimRight(hash, prs->GetCollect((long)0), prs->GetCollect((long)1));
		}
	};
}


//////////
// CCachedData implementation
//////////

CCachedData::CCachedData()
{

}

// For hashsets or hashmultisets of any type key for any context
template<typename H>
template<typename C>
int CCachedData::CCacheContainer<H>::Count(C &context, const typename H::key_type &valToLookUp)
{
	// Make sure our hash set is populated
	if (!this->isPopulated) {
		context.Fill(this->hash);
		this->isPopulated = true;
	}

	// Look up the value the caller wants to check
	return this->hash.count(valToLookUp);
}

// For hashsets or hashmultisets of any type key for any context
template<typename H>
template<typename C>
bool CCachedData::CCacheContainer<H>::Set(C &context, const typename H::key_type &valToSet)
{
	// Make sure our hash set is populated
	if (!this->isPopulated) {
		context.Fill(this->hash);
		this->isPopulated = true;
	}

	// Insert the value the caller wants to insert, and return true if we inserted it, false if it 
	// was already there so we didn't insert it
	return this->hash.insert(valToSet).second;
}

// (b.cardillo 2015-05-08 10:12) - PLID 65988 - Added support for maps (we already had sets)
// For hashmaps or hashmultimaps (returns first entry) of any type key/value for any context
template<typename H>
template<typename C, typename T>
bool CCachedData::CCacheContainer<H>::LookUp(C &context, const typename H::key_type &keyToLookUp, T &valToRetrieve)
{
	// Make sure our hash set is populated
	if (!this->isPopulated) {
		context.Fill(this->hash);
		this->isPopulated = true;
	}

	// Look up the value the caller wants to check
	auto iter = this->hash.find(keyToLookUp);
	if (iter != this->hash.end()) {
		// Found it, return true and a copy of the value of the entry we just found
		valToRetrieve = iter->second;
		return true;
	} else {
		// Wasn't found
		return false;
	}
}



// Hash any COM smart pointer using its IUnknown pointer to ensure object identity. (An object might 
// have different pointer values for different COM interfaces, but for any given object you'll always 
// get the same IUnknown pointer.)
template<typename ComPtrType>
std::size_t hash_value(const _com_ptr_t<ComPtrType>& p)
{
	return std::hash_value(IUnknownPtr(p).GetInterfacePtr());
}

namespace {
	// Convenience function PickDatalistCacheSet() to make sure the pointer isn't null and then 
	// cleanly retrieve the correct CCacheContainer object based on that datalist and column.
	template <class K, class T, class H, class P, class A>
	inline typename boost::unordered_map<K, T, H, P, A>::mapped_type::mapped_type &PickDatalistCacheSet(boost::unordered_map<K, T, H, P, A> &map, NXDATALIST2Lib::_DNxDataList *pDatalist, int nColumn)
	{
		if (pDatalist == NULL) {
			// It would never make sense to call this with a NULL datalist pointer because it would mean 
			// you don't have a control you're searching. If there were ever such calling code, it is 
			// surely a flaw in that code. But if not, then the caller can just check for NULL and not 
			// call us in that case.
			ASSERT(FALSE);
			ThrowNxException("CCachedData::Exists_DatalistValueInColumn() cannot search a NULL datalist.");
		}
		// The inner FindOrCreate() finds the map associated with the DATALIST, and then the outer 
		// FindOrCreate() looks in that map to find the CCacheContainer associated with the given COLUMN.
		return FindOrCreate(FindOrCreate(map, IUnknownPtr(pDatalist)), nColumn);
	}
}

bool CCachedData::Exists_DatalistValueInColumn(NXDATALIST2Lib::_DNxDataList *pDatalist, int nColumn, const string_key_type &valToLookup)
{
	// Call the standard Count() on the appropriate CCacheContainer. The appropriate CCacheContainer 
	// is the one that's associated with the requested COLUMN of the requested DATALIST.
	return PickDatalistCacheSet(m_datalistColumnSets, pDatalist, nColumn)
		.Count(CCachedDataContext_DatalistColumn(pDatalist, nColumn), TrimRight(valToLookup)) ? true : false;
}

int CCachedData::Count_DatalistValueInColumn(NXDATALIST2Lib::_DNxDataList *pDatalist, int nColumn, const string_key_type &valToLookup)
{
	// Call the standard Count() on the appropriate CCacheContainer. The appropriate CCacheContainer 
	// is the one that's associated with the requested COLUMN of the requested DATALIST.
	return PickDatalistCacheSet(m_datalistColumnMultiSets, pDatalist, nColumn)
		.Count(CCachedDataContext_DatalistColumn(pDatalist, nColumn), TrimRight(valToLookup));
}

bool CCachedData::Set_DatalistValueInColumn(NXDATALIST2Lib::_DNxDataList *pDatalist, int nColumn, const string_key_type &valToSet)
{
	// Call the standard Set() on the appropriate CCacheContainer. The appropriate CCacheContainer 
	// is the one that's associated with the requested COLUMN of the requested DATALIST.
	return PickDatalistCacheSet(m_datalistColumnSets, pDatalist, nColumn)
		.Set(CCachedDataContext_DatalistColumn(pDatalist, nColumn), TrimRight(valToSet));
}

bool CCachedData::Exists_QueryValue(const CString &sqlQuery, const string_key_type &valToLookup)
{
	// Call the standard Count() on the appropriate CCacheContainer. The appropriate CCacheContainer 
	// is the one of type T that's associated with the requested query string.
	return FindOrCreate(this->m_queryCacheSets_CString, sqlQuery)
		.Count(CCachedDataContext_Query(sqlQuery), TrimRight(valToLookup)) ? true : false;
}

bool CCachedData::Exists_QueryValue(const CString &sqlQuery, const long &valToLookup)
{
	// Call the standard Count() on the appropriate CCacheContainer. The appropriate CCacheContainer 
	// is the one of type T that's associated with the requested query string.
	return FindOrCreate(this->m_queryCacheSets_long, sqlQuery)
		.Count(CCachedDataContext_Query(sqlQuery), valToLookup) ? true : false;
}

// (b.cardillo 2015-05-08 10:12) - PLID 65988 - Added support for maps (we already had sets)
bool CCachedData::GetValue_Query(const CString &sqlQuery, const string_key_type &keyToLookup, long &valToRetrieve)
{
	// Call the standard LookUp() on the appropriate CCacheContainer. The appropriate CCacheContainer 
	// is the one of type T that's associated with the requested query string.
	return FindOrCreate(this->m_queryCacheMaps_CString_long, sqlQuery)
		.LookUp(CCachedDataContext_Query(sqlQuery), TrimRight(keyToLookup), valToRetrieve);
}
