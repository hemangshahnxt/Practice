#pragma once

#include <NxAlgorithm.h> // Ensure boost will pick up our CString-aware hash_value() function
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/foreach_fwd.hpp>

// (b.cardillo 2015-05-06 00:06) - PLID 65951 - Framework for loading and checking for the existence 
// of certain values efficiently.
// (b.cardillo 2015-05-07 08:48) - PLID 65951 (supplemental) - Just fixed some comments and 
// implemented an as-yet-unused function for completeness.
// (b.cardillo 2015-05-07 14:21) - PLID 65951 (supplemental) - Made it right-trim all strings to be 
// consistent with sql; also some efficiency and template improvements.
// (b.cardillo 2015-05-10 08:35) - PLID 65951 (supplemental) - Added support for multisets, made 
// comments better, and further templated.
// (b.cardillo 2015-07-16 22:04) - PLID 65951 (supplemental) - Make case insensitive. If we ever 
// use this class for some purpose that demands case sensitivity, we can easily template it.

// Construct an object of this class and simply call one of the provided Exists_X() functions. The 
// function will return true if the given value exists in the requested context, otherwise false.
// Subsequent calls to the same object checking for different values in the same context will be 
// much more efficient than doing the lookup yourself every time.
class CCachedData
{
	// Typedef for code clarity and to simplify things if we ever want to template this type.
	typedef CiString string_key_type;

public:
	CCachedData();

	// All our functions do the following:
	//   1. Choose the correct cache container for the given context (query, or datalist column)
	//   2. If a cache container does not yet exist for the context, create one and populate it with all 
	//      the values from that context.
	//   3. Now simply check the cache container to see if it contains the valToLookup value.
	//   4. Return the result depending on the kind of function (Exists, Count, etc.);
	// NOTE: We have implemented the kinds of functions currently needed, but there are other 
	// combinations we could easily add if needed. There are four operations and two contexts, so a 
	// total of 8 possible combinations (* currently implemented):
	//  - * Exists - Datalist
	//  - * Exists - Query
	//  - * Count - Datalist
	//  -   Count - Query
	//  -   GetValue - Datalist
	//  - * GetValue - Query
	//  - * Set - Datalist
	//  -   Set - Query
	// There also different datatypes. Right now we only needed CString and long, and only for some 
	// of the above combinations. Again, adding other types for any of the above combinations is 
	// trivial thanks to the templating. All you have to do is add the member variable and function, 
	// both of which will be obvious from the existing examples.
public:
	// >Context = Datalist column
	// >Operation = Exists
	// >Datatype = CString
	// Simply returns true or false, depending on whether the value exists at all in that column
	bool Exists_DatalistValueInColumn(NXDATALIST2Lib::_DNxDataList *pDatalist, int nColumn, const string_key_type &valToLookup);

	// >Context = Datalist column
	// >Operation = Count
	// >Datatype = CString
	// For each distinct value in the given column of the datalist, stores the number of rows with 
	// that have that value in that column. Returns that count for the given valToLookup.
	int Count_DatalistValueInColumn(NXDATALIST2Lib::_DNxDataList *pDatalist, int nColumn, const string_key_type &valToLookup);

	// >Context = Query
	// >Operation = Exists
	// >Datatype = CString or long
	// Simply returns true or false, depending on whether the value exists at all in that recordset.
	// The query must return exactly one column, and the values must be of the correct type for the 
	// given overload. For example, if you are calling the CString overload, then your sqlQuery 
	// better return a text column! It doesn't matter what the column is named.
	bool Exists_QueryValue(const CString &sqlQuery, const string_key_type &valToLookup);
	bool Exists_QueryValue(const CString &sqlQuery, const long &valToLookup);

	// (b.cardillo 2015-05-08 10:12) - PLID 65988 - Added support for maps (we already had sets)
	// >Context = Query
	// >Operation = GetValue
	// >Datatype = CString for key, long for value
	// Returns true or false, depending on whether the key exists at all in that recordset. If the 
	// key does exist, you get the associated value via the valToRetrieve parameter. The query must 
	// return exactly two columns (key, value), and the fields must be of the correct types. So for 
	// this case it is text for the 0th column and integer for the 1th column. NOTE: If the query 
	// returns multiple records with the same key, only the first one will be stored in the map.
	bool GetValue_Query(const CString &sqlQuery, const string_key_type &keyToLookup, long &valToRetrieve);

	// >Context = Datalist column
	// >Operation = Insert
	// >Datatype = CString
	// Initially populates the hashset just like the Exists operation, but then on each request it 
	// makes sure the given valToAdd value ends up in the hashset. Returns true if it set the value 
	// (because it wasn't already there), false if the value already existed (so wasn't added).
	bool Set_DatalistValueInColumn(NXDATALIST2Lib::_DNxDataList *pDatalist, int nColumn, const string_key_type &valToAdd);

protected:
	// Helper struct to store the hash of any specific kind and datatype and to provide type-safe 
	// lookups in that set. The lookups auto-populate the hash based on the context.
	template<typename H>
	struct CCacheContainer
	{
	public:
		H hash;
		bool isPopulated = false;
	public:
		// (b.cardillo 2015-05-10 08:35) - PLID 65951 - Added support for multisets
		template<typename C> int Count(C &context, const typename H::key_type &valToLookUp);
		// Set the value in the hashset. Returns true if inserted, false if already there.
		template<typename C> bool Set(C &context, const typename H::key_type &valToSet);
		// (b.cardillo 2015-05-08 10:12) - PLID 65988 - Added support for maps (we already had sets)
		template<typename C, typename T> bool LookUp(C &context, const typename H::key_type &keyToLookUp, T &valToRetrieve);
	};

protected:
	// Storage for our different contexts. It would have been nice to have just one m_queryCacheSets 
	// to handle all query-based contexts, but for type safety we have to have different members.
	boost::unordered_map<CString, CCacheContainer<boost::unordered_set<string_key_type>>> m_queryCacheSets_CString;
	boost::unordered_map<CString, CCacheContainer<boost::unordered_set<long>>> m_queryCacheSets_long;
	boost::unordered_map<IUnknownPtr, boost::unordered_map<int, CCacheContainer<boost::unordered_set<string_key_type>>>> m_datalistColumnSets;
	// (b.cardillo 2015-05-10 08:35) - PLID 65951 - Added support for multisets
	boost::unordered_map<IUnknownPtr, boost::unordered_map<int, CCacheContainer<boost::unordered_multiset<string_key_type>>>> m_datalistColumnMultiSets;
	// (b.cardillo 2015-05-08 10:12) - PLID 65988 - Added support for maps (we already had sets)
	boost::unordered_map<CString, CCacheContainer<boost::unordered_map<string_key_type, long>>> m_queryCacheMaps_CString_long;
};

