#include "StdAfx.h"
#include "ReportAdo.h"
#include <NxSystemUtilitiesLib\NxConnectionUtils.h>

// (c.haag 2015-01-22) - PLID 64646 - The "report snapshot" connection is basically a snapshot connection that tries
// to use MARS (Multiple Active Result Sets) in a SQL Server Native Client. This is only used for reports that use server-side
// cursors.
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

CString GetSQLServerNativeClientProviderName()
{
	LPCTSTR szNames[] = {
		"SQLNCLI11", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SQL Server\\SQLNCLI11\\CurrentVersion",
		"SQLNCLI10", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SQL Server\\SQLNCLI10\\CurrentVersion",
		"SQLNCLI", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SQL Native Client\\CurrentVersion",
		"SQLNCLI11", "HKEY_CLASSES_ROOT\\SQLNCLI11",
		"SQLNCLI10", "HKEY_CLASSES_ROOT\\SQLNCLI10",
		"SQLNCLI", "HKEY_CLASSES_ROOT\\SQLNCLI"
	};

	CString strProviderName;
	const int nameCount = sizeof(szNames) / sizeof(LPCTSTR);
	for (int i = 0; i < nameCount; i += 2)
	{
		if (NxRegUtils::DoesKeyExist(szNames[i + 1]))
		{
			strProviderName = szNames[i];
			break;
		}
	}

	return strProviderName;
}

CThreadLocal<CNxAdoThreadReportSnapshotConnection> tlsGlobalReportSnapshotConnections;

CNxAdoConnection& GetThreadConnectionReportSnapshot()
{
	return *tlsGlobalReportSnapshotConnections.GetData();
}

CNxAdoThreadReportSnapshotConnection::CNxAdoThreadReportSnapshotConnection()
{
	// (a.walling 2012-02-09 17:13) - PLID 48116 - snapshot connections are created based off of the thread's connection, or the globalTemplateConnection if not, via GetThreadConnection
	Init(GetThreadConnection()).EnableSnapshot();
}

// (c.haag 2015-01-22) - PLID 64646 - We need to manipulate the connection string
NxAdo::ConnectionInstance& CNxAdoThreadReportSnapshotConnection::Init(NxAdo::Connection& r)
{
	if (this == &r) return *this;

	// Reset and destroy any existing connection; the ADODB::Connection instance will be recreated
	Reset();

	// (c.haag 2015-01-22) - PLID 64646 - Generate our own connection string here. The difference between this
	// and the standard connection string is we try to use the SQLNCLI provider and enable a MARS Connection.
	CString strConn;
	CString strNetworkLibrary = r.GetNetworkLibrary();	
	CString strProviderOverride = NxRegUtils::ReadString(_T(GetRegistryBase() + "ReportSnapshot\\DataProvider"), "");
	CString strProvider = strProviderOverride.IsEmpty() ? GetSQLServerNativeClientProviderName() : strProviderOverride;
	
	if (strProvider.IsEmpty())
	{
		// We did not detect any SQL Server Native Client provider and no default was specified. 
		// Defer to the incoming connection's string.
		Log("CNxAdoThreadReportSnapshotConnection::Init failed to detect a SQL Server Native Client, and no override was provided! The function will not attempt to enable Multiple Active Result Sets.");
		SetConnectionString(r.GetConnectionString());
	}
	// (b.savon 2016-05-18 13:02) - NX-100670 - Check the flag to use Windows Auth for SQL connections
	else if (NxConnectionUtils::UseSqlIntegratedSecurity(GetSubRegistryKey())) {
		// Ensure we set the data type compatibility.
		CString strProviderOptions = (1 != strProvider.Find("SQLNCLI")) ? "DataTypeCompatibility=80;MARS Connection=True;" : "";
		strConn.Format(
			"Provider=%s;"								// provider (and possibly other options)
			"%s"
			"Integrated Security=SSPI;"
			"Persist Security Info=True;"
			"Initial Catalog=%s;"				// database
			"Data Source=%s;"					// server
			"%s"								// network library
			, strProvider
			, strProviderOptions
			, r.GetDatabase()
			, r.GetDataSource()
			, (strNetworkLibrary.GetLength() > 0) ? (strNetworkLibrary + ";") : ""
		);

		SetConnectionString(strConn);
	}
	else
	{
		// Ensure we set the data type compatibility.
		CString strProviderOptions = (1 != strProvider.Find("SQLNCLI")) ? "DataTypeCompatibility=80;MARS Connection=True;" : "";
		strConn.Format(
			"Provider=%s;"								// provider (and possibly other options)
			"%s"
			"Password=%s;"
			"Persist Security Info=True;"
			"User ID=%s;"
			"Initial Catalog=%s;"				// database
			"Data Source=%s;"					// server
			"%s"								// network library
			, strProvider
			, strProviderOptions
			, r.GetPassword()
			, r.GetUserID()
			, r.GetDatabase()
			, r.GetDataSource()
			, (strNetworkLibrary.GetLength() > 0) ? (strNetworkLibrary + ";") : ""
			);

		SetConnectionString(strConn);
	}

	SetConnectionTimeout(r.GetConnectionTimeout());
	SetCommandTimeout(r.GetCommandTimeout());
	return *this;
}
