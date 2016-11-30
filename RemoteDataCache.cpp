#include "stdafx.h"
#include "RemoteDataCache.h"

// (a.walling 2010-06-23 11:19) - PLID 39321 - Need a unified place to cache commonly accessed information.

CRemoteDataCache* g_pRemoteDataCache = NULL;

CRemoteDataCache& GetRemoteDataCache()
{
	if (!g_pRemoteDataCache) {
		g_pRemoteDataCache = new CRemoteDataCache();
	}

	return *g_pRemoteDataCache;
}

void FreeRemoteDataCache()
{
	delete g_pRemoteDataCache;
	g_pRemoteDataCache = NULL;
}

CRemoteDataCache::CRemoteDataCache(void)
	: m_tcLocations(NetUtils::LocationsT)
	, m_bLoadedInitialLocations(false)
{
}

CRemoteDataCache::~CRemoteDataCache(void)
{
}

void CRemoteDataCache::Reset(void)
{
	m_bLoadedInitialLocations = false;
}

CString CRemoteDataCache::GetLocationName(long nLocationID, bool bForceRefresh)
{
	EnsureLocations(bForceRefresh);

	LocationName lnLocationName;
	// (b.spivey, March 26, 2012) - PLID 47521 - have to get the location name out of the struct now. 
	if (!m_mapLocationNames.Lookup(nLocationID, lnLocationName)) {
		if (!bForceRefresh) {
			return GetLocationName(nLocationID, true);
		} else {
			ThrowNxException("A location with an ID of %li was not found!", nLocationID);
		}
	}

	return lnLocationName.strLocName;
}

// (b.spivey, March 28, 2012) - PLID 47521 - Added a function to get the location abbreviation. It should always exist. 
CString CRemoteDataCache::GetLocationAbbreviation(long nLocationID, bool bForceRefresh) 
{
	EnsureLocations(bForceRefresh); 

	LocationName lnLocationAbbrev; 

	if(!m_mapLocationNames.Lookup(nLocationID, lnLocationAbbrev)) {
		if (!bForceRefresh) {
			return GetLocationAbbreviation(nLocationID, true); 
		} 
		else {
			ThrowNxException("A location with an ID of %li was not found!", nLocationID);
		}
	}

	return lnLocationAbbrev.strLocAbbrev; 
}

void CRemoteDataCache::EnsureLocations(bool bForceRefresh)
{
	if (m_tcLocations.Changed() || !m_bLoadedInitialLocations || bForceRefresh) {
		m_bLoadedInitialLocations = true;

		// (b.spivey, March 28, 2012) - PLID 47521 - We hold LocationAbbreviation as a property of locations. 
		ADODB::_RecordsetPtr prs = CreateRecordset("SELECT ID, Name, LocationAbbreviation FROM LocationsT");

		m_mapLocationNames.RemoveAll();

		while (!prs->eof) {
			//Use the struct since there are two properties we want to access quickly. 
			LocationName tempLocName;
			tempLocName.strLocName = AdoFldString(prs, "Name");
			tempLocName.strLocAbbrev = AdoFldString(prs, "LocationAbbreviation", "");
			m_mapLocationNames[AdoFldLong(prs, "ID")] = tempLocName; 
			prs->MoveNext();
		}
	}
}



