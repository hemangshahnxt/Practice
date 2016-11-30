#pragma once

// (a.walling 2010-06-23 11:19) - PLID 39321 - Need a unified place to cache commonly accessed information.

class CRemoteDataCache
{
public:
	CRemoteDataCache(void);
	~CRemoteDataCache(void);
	
	void Reset();
// (b.spivey, March 23, 2012) - PLID 47521 - location name struct.
	struct LocationName {
		CString strLocName;
		CString strLocAbbrev; 
	};
// Locations
public:
	CString GetLocationName(long nLocationID, bool bForceRefresh = false);
	// (b.spivey, March 26, 2012) - PLID 47521 - make it work like the GetLocationFunction.
	CString GetLocationAbbreviation(long nLocationID, bool bForceRefresh = false); 

protected:
	CTableChecker m_tcLocations;
	bool m_bLoadedInitialLocations;
	void EnsureLocations(bool bForceRefresh);
	CMap<long, long, LocationName, LocationName &> m_mapLocationNames;
};

CRemoteDataCache& GetRemoteDataCache();

void FreeRemoteDataCache(); // frees the object entirely