#pragma once

#include "nxado.h"
#include "NxAdoLib\NxAdoConnection.h"

// (c.haag 2015-01-22) - PLID 64646 - The "report snapshot" connection is basically a snapshot connection that tries
// to use MARS (Multiple Active Result Sets) in a SQL Server Native Client. This is currently used for reports that use 
// server-side cursors through manual ConfigRT changes.
//
// The premise is that there can be so many records returned from the server using server-side cursors, that the server
// process must return those records to the client before it can query for additional records. Subsequent queries would
// result in a new server process being spawned and not having access to any #temp tables used in the original server
// process. This will always lead to exception cases if the user is doing certain kinds of report filtering.
//
// MARS is only supported for SQL Server Native Client. You can overide the provider by writing a value to the
// GetRegistryBase() + "ReportSnapshot\\DataProvider" registry key. If that provider is not a flavor of SQLNCLI then
// MARS will not be used.
// 

CNxAdoConnection& GetThreadConnectionReportSnapshot();

inline CNxAdoConnection& GetRemoteConnectionReportSnapshot()
{
	return GetThreadConnectionReportSnapshot();
}

CNxAdoConnection& GetRemoteConnectionReportSnapshot();

inline void EnsureRemoteDataReportSnapshot(EnumAffirmConnection nAffirmConnection = acAffirmNow)
{
	GetRemoteConnectionReportSnapshot().EnsureRemoteData(nAffirmConnection);
}

inline ADODB::_ConnectionPtr GetRemoteDataReportSnapshot(EnumAffirmConnection nAffirmConnection = acAffirmDelay)
{
	return GetRemoteConnectionReportSnapshot().GetRemoteData(nAffirmConnection);
}

// (a.walling 2012-02-09 17:13) - PLID 48116 - Isolate CNxAdoConnections per-thread
inline ADODB::_ConnectionPtr GetThreadRemoteDataReportSnapshot(EnumAffirmConnection nAffirmConnection = acAffirmDelay)
{
	return GetRemoteDataReportSnapshot(nAffirmConnection);
}

inline bool EnsureNotRemoteDataReportSnapshot()
{
	return GetRemoteConnectionReportSnapshot().EnsureNotRemoteData();
}

class CNxAdoThreadReportSnapshotConnection
	: public CNxAdoConnection
{
public:
	CNxAdoThreadReportSnapshotConnection();

	// (c.haag 2015-01-22) - PLID 64646 - We need to allow inherited classes to manipulate connection strings
	virtual ConnectionInstance& Init(NxAdo::Connection& r);
};
