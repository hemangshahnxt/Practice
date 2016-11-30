#pragma once

#include <sqlite/sqlite3.h>
#include <NxDataUtilitiesLib/NxSqlite.h>

// (a.walling 2013-07-18 09:30) - PLID 57624 - NxCache - common framework for managing the cache, include sqlite lib

namespace Nx
{

namespace Cache
{
	// Initialize sqlite and set options
	void InitSystem();
	// (a.walling 2013-07-18 09:30) - PLID 57624 - Access database, ensure schema, initialize cache
	void InitDatabase();
	// (a.walling 2013-07-18 09:30) - PLID 57624 - Close database and global connection
	void Shutdown();

	// (a.walling 2013-07-18 10:04) - PLID 57629 - Update changed cached data
	void Checkpoint();

	///
	
	// (a.walling 2013-07-18 09:30) - PLID 57624 - Thread-local connections
	sqlite::Connection& GetConnection();

	///

	namespace Emr
	{
		// (a.walling 2013-07-23 20:49) - PLID 57685 - Load all data for an individual EmrInfoT.ID
		void EnsureEmrInfo(long nID);
	}

	// (a.walling 2013-07-18 09:58) - PLID 57627 - Cache EMRTableDropdownInfoT.ID->Data mappings
	namespace EmrTableDropdownInfo
	{
		// throw if nID not found
		CString GetDataFromDropdownID(long nID);

		// (a.walling 2013-07-23 21:36) - PLID 57686 - Simply return the DropdownGroupID for a given EMRTableDropdownInfoT.ID
		// returns -1 if not found
		long GetDropdownGroupIDFromDropdownID(long nID);
	}
}

}

