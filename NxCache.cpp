#include "StdAfx.h"

#include "NxCache.h"

#include <NxSystemUtilitiesLib/NxThread.h>
#include <NxDataUtilitiesLib/NxSqlite.h>

#include "ProgressDialog.h"

//#define NXCACHE_ALL_THE_THINGS

//#define NXCACHE_EXTRA

//#define NXCACHE_PERSISTENT

#ifdef _DEBUG
#define NXCACHE_PERSISTENT
#endif

// (a.walling 2013-07-18 09:30) - PLID 57624 - NxCache - common framework for managing the cache, include sqlite lib

// (a.walling 2013-07-26 13:53) - PLID 57629 - The maximum revision is now only stored when performing a full update, so it does not end up skipping anything
// Also added //savepoint support for nested transactions, and the NXCACHE_PERSISTENT define for the persistent db

// (a.walling 2013-08-06 13:05) - PLID 57886 - Real BIGINT / VT_I8 support in GlobalParamUtils now that we no longer support Windows 2000

namespace Nx
{

namespace Cache
{
	namespace detail
	{
		bool ShouldPopulateInitialCache();
		void PopulateInitialCache();
		void UpdateCache();

		class CacheConnection
			: public CNoTrackObject
		{
		public:
			CacheConnection()
			{}

			sqlite::Connection sql;

			// (a.walling 2013-07-18 09:58) - PLID 57627 - Cached lookup statement for EMRTableDropdownInfoT.ID->Data
			sqlite::Statement dataFromDropdownID;

			void Close()
			{
				dataFromDropdownID.Finalize();
				sql.Close();
			}
		};

		CThreadLocal<CacheConnection> tlsConnection;

		// (a.walling 2013-07-18 09:30) - PLID 57624 - Setup db options
		void InitConnection(sqlite::Connection& sql)
		{
#ifdef NXCACHE_PERSISTENT
			sql.Exec("PRAGMA page_size=4096");
			sql.Exec("PRAGMA secure_delete=OFF");
			sql.Exec("PRAGMA cache_size=32768");
			sql.Exec("PRAGMA journal_mode=WAL");
			sql.Exec("PRAGMA synchronous=NORMAL");
#else
			sql.Exec("PRAGMA page_size=4096");
			sql.Exec("PRAGMA secure_delete=OFF");
			sql.Exec("PRAGMA cache_size=32768");
			sql.Exec("PRAGMA journal_mode=MEMORY");
			sql.Exec("PRAGMA synchronous=OFF");
#endif
#pragma TODO("Better busy handling would be ideal, but unnecessary so much at the moment since we are not sharing or persisting the file")
			sqlite3_busy_timeout(sql, 360000);
		}

#ifdef NXCACHE_PERSISTENT
		namespace {
			CString MakeTimestamp(const COleDateTime& dt)
			{
				if (dt.m_dt == 0.0) {
					return "0";
				}

				SYSTEMTIME st = {0};
				dt.GetAsSystemTime(st);
				CString str;
				
				str.Format(
					"%04u%02u%02u"
					"T%02u%02u%02u"
					, st.wYear, st.wMonth, st.wDay
					, st.wHour, st.wMinute, st.wSecond
				);

				return str;
			}

			bool IsValidFileNameChar(TCHAR c)
			{
				if (c < 32) {
					return false;
				}

				switch (c) {
					case _T('<'):
					case _T('>'):
					case _T(':'):
					case _T('\"'):
					case _T('/'):
					case _T('\\'):
					case _T('|'):
					case _T('?'):
					case _T('*'):
						return false;
				}

				return true;
			}

			bool IsValidFileName(const TCHAR* sz)
			{
				if (!sz || !*sz) {
					return false;
				}
				while (IsValidFileNameChar(*sz)) {
					++sz;
				}

				return *sz == 0;
			}
		}

		CString GetCacheDatabaseFileName()
		{
			ADODB::_RecordsetPtr prs = CreateRecordsetStd(GetRemoteData(),
				"SELECT "
					"( "
						"SELECT MAX(restore_date) "
						"FROM msdb..restorehistory "
						"WHERE destination_database_name = DB_NAME() "
					") AS RestoreDate " 
					", "
					"( "
						"SELECT create_date "
						"FROM master.sys.databases "
						"WHERE database_id = DB_ID() "
					") AS CreateDate "
					", @@SERVERNAME AS ServerName "
				);

			CString strFile;
			strFile.Format(
				"NxCache%s_%s_%s_%s.db"
				, MakeValidFileName(GetSubRegistryKey())
				, MakeValidFileName(AdoFldString(prs, "ServerName", "Server"), '$')
				, MakeTimestamp(AdoFldDateTime(prs, "RestoreDate", g_cdtZero))
				, MakeTimestamp(AdoFldDateTime(prs, "CreateDate", g_cdtZero))
			);

			CString strBasePath = FileUtils::GetLocalAppDataFolderPath() / "NexTech";

			FileUtils::CreatePath(strBasePath);
			
			return strBasePath / strFile;
		}
#else
		CString GetCacheDatabaseFileName()
		{
			return GetPracPath(PracPath::SessionPath) / "NxCache.db";
		}
#endif
#pragma TODO("PLID 57769 - Move to shared utils: CString MakeValidFileName(CString strFileName, char invalidChar = '_')")			

		// (a.walling 2013-07-18 09:30) - PLID 57624 - Get or create db connection
		CacheConnection& GetCacheConnection()
		{
			CacheConnection* pConnection = tlsConnection.GetData();
			if (!pConnection->sql) {
				pConnection->sql.Open(GetCacheDatabaseFileName());
				sqlite::Connection& p = pConnection->sql;

				InitConnection(p);
			}

			return *pConnection;
		}
		
		void Shutdown()
		{		
			try {
				CacheConnection* pConnection = tlsConnection.GetDataNA();
				if (pConnection && pConnection->sql) {
					pConnection->sql.Close();
				}
#pragma TODO("Our current cache db is effectively a temp file, being in the session path. Should be cleaned up with the rest of the path. Obviously this will be moot once we are persisting / sharing cache.")
			} NxCatchAllIgnore();
		}

		///

		
		// (a.walling 2013-07-18 10:08) - PLID 57630 - Event set when thread is complete
		CEvent cacheComplete(FALSE, TRUE, NULL, NULL);
		bool cacheInit = false;
		bool cacheReady = false;

		///

		// (a.walling 2013-07-18 10:08) - PLID 57630 - Launch thread to prepopulate the cache
		void InitCache()
		{
			if (cacheReady) {
				return;
			}
			cacheInit = true;

			if (ShouldPopulateInitialCache()) {
				NxThread initThread = NxThread(&PopulateInitialCache);
				initThread->RunDetached();
			} else {
				cacheComplete.SetEvent();
				cacheReady = true;
			}
		}

		// (a.walling 2013-07-18 10:08) - PLID 57630 - Wait until the async cache population thread has completed
		bool WaitForCacheReady()
		{
			if (cacheReady) {
				return true;
			}
			if (!cacheInit) {
				return false;
			}

			int rc;

			static const DWORD uiDelay = 2000;

			rc = ::WaitForSingleObject(cacheComplete, uiDelay);
			if (rc == WAIT_TIMEOUT) {
				CProgressDialog progressDialog;

				progressDialog.Start(NULL, CProgressDialog::NoTime | CProgressDialog::NoMinimize | CProgressDialog::MarqueeProgress, 
					"NexTech Practice", "Please wait...");

				progressDialog.SetLine(1, "Optimizing NexEMR...");
				progressDialog.SetLine(2, "This may take a few minutes, but only needs to run once.");

				rc = ::WaitForSingleObject(cacheComplete, INFINITE);
			}

			if (rc == WAIT_OBJECT_0) {
				cacheReady = true;
				return true;
			} else {
				return false;
			}
		}
		
		// (a.walling 2013-07-18 10:04) - PLID 57629 - Get max revision
		template<typename T>
		__int64 GetIntegerProp(sqlite::Connection& sql, T name, __int64 val = 0LL)
		{
			sqlite::Statement getProp(sql,
				"select Value from Props where Name = ?"
			);

			getProp.Reset().Bind() << name;
			if (sqlite::Row row = getProp.Next()) {
				val = row[0];
			}
			getProp.Reset();

			return val;
		}
		
		template<typename T>
		void UpdateMaxProp(sqlite::Connection& sql, T name, __int64 val)
		{
			if (!val) {
				return;
			}

			sqlite::Statement updateMaxProp(sql,
				"update Props set Value = max(Value, ?) where Name = ?"
			);

			sqlite::Statement insertProp(sql,
				"insert or ignore into Props(Name, Value) values(?, ?)"
			);

			// (a.walling 2013-07-18 10:04) - PLID 57629 - Update max revision
			updateMaxProp.Reset().Bind()
				<< val
				<< name
			;

			updateMaxProp.Exec();

			if (!sql.Changes()) {
				insertProp.Reset().Bind()
					<< name
					<< val
				;

				insertProp.Exec();

				if (!sql.Changes()) {
					// if that was ignored, something else inserted, so we'll update once more
					updateMaxProp.Exec();
				}
			}
		}
	}

	///

	CSqlFragment MaxRevisionQuery(const char* table)
	{
		return CSqlFragment(
			"SELECT COALESCE(CONVERT(BIGINT, MAX(Revision)), CONVERT(BIGINT, 0)) AS MaxRevision "
			"FROM {CONST_STRING}; \r\n"
			, table
		);
	}

	///

	
	// (a.walling 2013-07-18 09:58) - PLID 57627 - Cache EMRTableDropdownInfoT.ID->Data mappings
	namespace EmrTableDropdownInfo
	{
		// (a.walling 2013-07-18 10:04) - PLID 57629 - Get max revision
		__int64 GetMaxDataRevision(sqlite::Connection& sql)
		{
			return detail::GetIntegerProp(sql, "EmrTableDropdownInfoT_Revision");
		}		

		CSqlFragment SourceQuery()
		{
			return CSqlFragment(
				"SELECT "
					  "ID"
					", EMRDataID"
					", Data"
					", SortOrder"
					", Inactive"
					", DropdownGroupID"
					", GlassesOrderDataID "
				"FROM EMRTableDropdownInfoT "
			);
		}

		// (a.walling 2015-04-27 09:31) - PLID 65744 - Further optimizations to queries in NxCache via EMRTableDropdownInfoJoinedV
		CSqlFragment ActiveQuery()
		{
			return CSqlFragment(
				"{SQL}"
				"{SQL}"
#ifndef NXCACHE_ALL_THE_THINGS
				"WHERE EMRTableDropdownInfoT.ID IN ( "
					"SELECT EMRTableDropdownInfoJoinedV.ID "
					"FROM EMRTableDropdownInfoJoinedV "
					"WHERE EMRTableDropdownInfoJoinedV.EMRInfoID IN ( "
						"SELECT ActiveEMRInfoID "
						"FROM EMRInfoMasterT "
					") "
				") "
#endif
				, MaxRevisionQuery("EMRTableDropdownInfoT")
				, SourceQuery()
			);
		}

		CSqlFragment ChangedQuery(sqlite::Connection& sql)
		{
			__int64 maxRevision = GetMaxDataRevision(sql);

			return CSqlFragment(
				"{SQL}"
				"{SQL}"
				"WHERE EMRTableDropdownInfoT.Revision > CONVERT(ROWVERSION, {BIGINT}) "
				, MaxRevisionQuery("EMRTableDropdownInfoT")
				, SourceQuery()
				, maxRevision
			);
		}

		// (a.walling 2013-07-23 20:49) - PLID 57685 - Load all data for an individual EmrInfoT.ID
		CSqlFragment ByEmrInfoQuery()
		{
			// (a.walling 2014-10-15 13:50) - PLID 63921 - EMRTableDropdownInfoJoinedV
			return CSqlFragment(
				"{SQL}"
				"WHERE EMRTableDropdownInfoT.ID IN ("
					"SELECT ID "
					"FROM EMRTableDropdownInfoJoinedV "
					"WHERE EMRInfoID = @EmrInfoID"
				")"
				, SourceQuery()
			);
		}

		// (a.walling 2013-07-18 09:58) - PLID 57627 - Inserts EMRTableDropdownInfoT.ID->Data mappings into the sqlite db in a transaction
		// (a.walling 2013-07-18 10:04) - PLID 57629 - Updates and returns max revision
		__int64 InsertData(ADODB::_Recordset* prs, sqlite::Connection& sql, __int64 maxRevision)
		{
			if (prs->eof) {
				detail::UpdateMaxProp(sql, "EmrTableDropdownInfoT_Revision", maxRevision);
				return 0LL;
			}
			
			CNxPerform nxp(__FUNCTION__);

			sqlite::Statement insertText(sql, 
				"insert or ignore into EmrTableDropdownInfoTextT(Data) values(?);"
			);
			sqlite::Statement insertInfoTextLookup(sql, 
				"insert or replace into EmrTableDropdownInfoT( "
					  "ID "
					", EmrDataID"
					", TextID "
					", SortOrder"
					", Inactive"
					", DropdownGroupID"
					", GlassesOrderDataID"
				") select ?, ?, EmrTableDropdownInfoTextT.rowid, ?, ?, ?, ? from EmrTableDropdownInfoTextT where Data = ?;"
			);
			sqlite::Statement insertInfoTextIdentity(sql, 
				"insert or replace into EmrTableDropdownInfoT( "
					  "ID "
					", EmrDataID"
					", TextID "
					", SortOrder"
					", Inactive"
					", DropdownGroupID"
					", GlassesOrderDataID"
				") values (?, ?, ?, ?, ?, ?, ?);"
			);

			sqlite::Statement updateMaxProp(sql,
				"update Props set Value = max(Value, ?) where Name = ?"
			);

			sqlite::Statement insertProp(sql,
				"insert or replace into Props(Name, Value) values(?, ?)"
			);

			
			int count = 0;

			//sqlite:://savepoint //savepoint(sql, "EmrTableDropdownInfo_Active");
			//savepoint.Begin();
			while (!prs->eof) {
				++count;

				long id = AdoFldLong(prs, "ID");
				long emrDataID = AdoFldLong(prs, "EmrDataID");
				long sortOrder = AdoFldLong(prs, "SortOrder");
				BOOL inactive = AdoFldBool(prs, "Inactive");
				long dropdownGroupID = AdoFldLong(prs, "DropdownGroupID");
				long glassesOrderID = AdoFldLong(prs, "GlassesOrderDataID", 0);
				CString data = AdoFldString(prs, "Data", "");

				prs->MoveNext();

				insertInfoTextLookup.Reset().Bind()
					<< id
					<< emrDataID
					<< sortOrder
					<< inactive
					<< dropdownGroupID
					<< glassesOrderID
					<< boost::cref(data)
				;

				insertInfoTextLookup.Exec();

				if (sql.Changes()) {
					continue;
				}

				insertText.Reset().Bind()
					<< boost::cref(data)
				;

				insertText.Exec();

				if (sql.Changes()) {
					insertInfoTextIdentity.Reset().Bind()
						<< id
						<< emrDataID
						<< sql.LastInsertRowID()
						<< sortOrder
						<< inactive
						<< dropdownGroupID
						<< glassesOrderID
					;
					insertInfoTextIdentity.Exec();
				} else {
					insertInfoTextLookup.Reset().Exec();
					ASSERT(sql.Changes());
				}
			}

			detail::UpdateMaxProp(sql, "EmrTableDropdownInfoT_Revision", maxRevision);

			//savepoint.Commit();
			

			nxp.Tick("%li records; max revision %I64i", count, maxRevision);

			return maxRevision;
		}
		
		// (a.walling 2013-07-18 10:08) - PLID 57630 - Asynchronously pre-populate cache with active items
		ADODB::_RecordsetPtr GatherActiveData(ADODB::_RecordsetPtr prs, sqlite::Connection& sql)
		{
			CNxPerform nxp(__FUNCTION__);

			__int64 maxRevision = AdoFldBigInt(prs, "MaxRevision");
			prs = prs->NextRecordset(NULL);

			InsertData(prs, sql, maxRevision);
			return prs;
		}
		
		// (a.walling 2013-07-18 10:04) - PLID 57629 - Gathers all data changed since last inserted revision
		ADODB::_RecordsetPtr GatherChangedData(ADODB::_RecordsetPtr prs, sqlite::Connection& sql)
		{
			__int64 maxRevision = AdoFldBigInt(prs, "MaxRevision");
			prs = prs->NextRecordset(NULL);

			InsertData(prs, sql, maxRevision);
			return prs;
		}

		// (a.walling 2013-07-18 09:58) - PLID 57627 - Cache EMRTableDropdownInfoT.ID->Data mappings for the entire EMRDataID containing the dropdown ID
		void GatherDataForDropdownID(ADODB::_Connection* pCon, sqlite::Connection& sql, long nID)
		{
			CNxPerform nxp(__FUNCTION__);

			ADODB::_RecordsetPtr prs = CreateParamRecordset(pCon, ADODB::adUseServer, ADODB::adOpenForwardOnly, ADODB::adLockReadOnly, ADODB::adCmdText, 
				"{SQL}"
				"WHERE EMRTableDropdownInfoT.EMRDataID IN ("
					"SELECT EMRDataID "
					"FROM EMRTableDropdownInfoT "
					"WHERE ID = {INT}"
				")"
				, SourceQuery()
				, nID
			);

			InsertData(prs, sql, 0LL);
		}

		// (a.walling 2013-07-18 09:58) - PLID 57627 - Return the Data string for a given DropdownID; will query data on new snapshot connection if not found
		CString GetDataFromDropdownID(long nID)
		{
			// (a.walling 2013-07-18 10:08) - PLID 57630 - Ensure cache is ready
			detail::WaitForCacheReady();
			detail::CacheConnection& connection = detail::GetCacheConnection();
			sqlite::Connection& sql = connection.sql;

			sqlite::Statement& stmt = connection.dataFromDropdownID;

			// (a.walling 2013-07-18 09:58) - PLID 57627 - Create lookup statement if necessary
			if (!stmt) {
				stmt.Prepare(sql,
					"select EmrTableDropdownInfoTextT.Data "
					"from EmrTableDropdownInfoT "
					"inner join EmrTableDropdownInfoTextT on EmrTableDropdownInfoT.TextID = EmrTableDropdownInfoTextT.rowid "
					"where EmrTableDropdownInfoT.ID = ? "
				);
			}

			stmt.Reset().Bind() 
				<< nID
			;

			sqlite::Row row = stmt.Next();
			if (!row) {
				stmt.Reset();
				// (a.walling 2013-07-18 09:58) - PLID 57627 - Not found, so query db
				ADODB::_ConnectionPtr pCon = GetRemoteDataSnapshot();
				CIncreaseCommandTimeout noTimeout(pCon, 0);
				GatherDataForDropdownID(pCon, sql, nID);
				row = stmt.Next();
			}

			if (!row) {
				stmt.Reset();
				// (a.walling 2013-07-18 09:58) - PLID 57627 - Not found, this is an error case
				ASSERT(FALSE);
				ThrowNxException("Failed to find dropdown data for ID %li", nID);
			}
			
			CString str = row[0].Text();
			stmt.Reset();
			return str;
		}

		// (a.walling 2013-07-23 21:36) - PLID 57686 - Simply return the DropdownGroupID for a given EMRTableDropdownInfoT.ID
		long GetDropdownGroupIDFromDropdownID(long nID)
		{
			sqlite::Connection& sql = GetConnection();

			sqlite::Statement dataGroupIDFromDropdownID(sql,
				"select EmrTableDropdownInfoT.DropdownGroupID "
				"from EmrTableDropdownInfoT "
				"where EmrTableDropdownInfoT.ID = ? "
			);

			dataGroupIDFromDropdownID.Reset().Bind() 
				<< nID
			;

			sqlite::Row row = dataGroupIDFromDropdownID.Next();
			if (!row) {
				dataGroupIDFromDropdownID.Reset();
				// (a.walling 2013-07-18 09:58) - PLID 57627 - Not found, so query db
				ADODB::_ConnectionPtr pCon = GetRemoteDataSnapshot();
				CIncreaseCommandTimeout noTimeout(pCon, 0);
				GatherDataForDropdownID(pCon, sql, nID);
				row = dataGroupIDFromDropdownID.Next();
			}

			if (!row) {
				dataGroupIDFromDropdownID.Reset();
				// for consistency, caller will handle errors
				return -1;
			}

			long groupID = row[0];

			dataGroupIDFromDropdownID.Reset();

			return groupID;
		}
	}

	///

#ifdef NXCACHE_EXTRA
	// (a.walling 2013-07-22 17:32) - PLID 57670 - NxCache - EmrImageStampsT
	namespace EmrImageStamps
	{
		// (a.walling 2013-07-18 10:04) - PLID 57629 - Get max revision
		__int64 GetMaxDataRevision(sqlite::Connection& sql)
		{
			return detail::GetIntegerProp(sql, "EmrImageStampsT_Revision");
		}

		CSqlFragment SourceQuery()
		{
			return CSqlFragment(
				"SELECT "
					  "ID"
					", StampText"
					", TypeName"
					", Color"
					", Description"
					", Inactive"
					", SmartStampTableSpawnRule"
					", ShowDot"
					", Image"
					", CategoryID "
				"FROM EmrImageStampsT "
			);
		}

		CSqlFragment ActiveQuery()
		{
			return CSqlFragment(
				"{SQL}"
				"{SQL}"
				//"WHERE EmrImageStampsT.Inactive = 0"
				, MaxRevisionQuery("EmrImageStampsT")
				, SourceQuery()
			);
		}

		CSqlFragment ChangedQuery(sqlite::Connection& sql)
		{
			__int64 maxRevision = GetMaxDataRevision(sql);

			return CSqlFragment(
				"{SQL}"
				"{SQL}"
				"WHERE EmrImageStampsT.Revision > CONVERT(ROWVERSION, {BIGINT}) "
				, MaxRevisionQuery("EmrImageStampsT")
				, SourceQuery()
				, maxRevision
			);
		}
		
		__int64 InsertData(ADODB::_Recordset* prs, sqlite::Connection& sql, __int64 maxRevision)
		{
			if (prs->eof) {
				detail::UpdateMaxProp(sql, "EmrImageStampsT_Revision", maxRevision);
				return 0LL;
			}
			
			CNxPerform nxp(__FUNCTION__);

			sqlite::Statement insertStamp(sql, 
				"insert or replace into EmrImageStampsT( "
					  "ID"
					", StampText"
					", TypeName"
					", Color"
					", Description"
					", Inactive"
					", SmartStampTableSpawnRule"
					", ShowDot"
					", Image"
					", CategoryID"
				") values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"
			);

			
			int count = 0;

			//sqlite:://savepoint //savepoint(sql, "EmrImageStamps_Active");
			//savepoint.Begin();
			while (!prs->eof) {
				++count;

				long id = AdoFldLong(prs, "ID");
				CString text = AdoFldString(prs, "StampText");
				CString typeName = AdoFldString(prs, "TypeName");
				long color = AdoFldLong(prs, "Color");
				CString description = AdoFldString(prs, "Description");
				BOOL inactive = AdoFldBool(prs, "Inactive");
				long tableSpawnRule = AdoFldLong(prs, "SmartStampTableSpawnRule");
				BOOL showDot = AdoFldBool(prs, "ShowDot");
				_variant_t varImage = prs->Collect["Image"];
				_variant_t varCategory = prs->Collect["CategoryID"];

				prs->MoveNext();

				insertStamp.Reset().Bind()
					<< id
					<< boost::cref(text)
					<< boost::cref(typeName)
					<< color
					<< boost::cref(description)
					<< inactive
					<< tableSpawnRule
					<< showDot
					<< varImage
					<< varCategory
				;

				insertStamp.Exec();
			}

			detail::UpdateMaxProp(sql, "EmrImageStampsT_Revision", maxRevision);

			//savepoint.Commit();
			

			nxp.Tick("%li records; max revision %I64i", count, maxRevision);

			return maxRevision;
		}
		
		// (a.walling 2013-07-18 10:08) - PLID 57630 - Asynchronously pre-populate cache with active items
		ADODB::_RecordsetPtr GatherActiveData(ADODB::_RecordsetPtr prs, sqlite::Connection& sql)
		{
			CNxPerform nxp(__FUNCTION__);

			__int64 maxRevision = AdoFldBigInt(prs, "MaxRevision");
			prs = prs->NextRecordset(NULL);

			InsertData(prs, sql, maxRevision);
			return prs;
		}
		
		// (a.walling 2013-07-18 10:04) - PLID 57629 - Gathers all data changed since last inserted revision
		ADODB::_RecordsetPtr GatherChangedData(ADODB::_RecordsetPtr prs, sqlite::Connection& sql)
		{
			__int64 maxRevision = AdoFldBigInt(prs, "MaxRevision");
			prs = prs->NextRecordset(NULL);

			InsertData(prs, sql, maxRevision);
			return prs;
		}

		void GatherDataForStampID(ADODB::_Connection* pCon, sqlite::Connection& sql, long nID)
		{
			CNxPerform nxp(__FUNCTION__);

			ADODB::_RecordsetPtr prs = CreateParamRecordset(pCon, ADODB::adUseServer, ADODB::adOpenForwardOnly, ADODB::adLockReadOnly, ADODB::adCmdText, 
				"{SQL}"
				"WHERE EmrImageStampsT.ID = {INT}"
				, SourceQuery()
				, nID
			);

			InsertData(prs, sql);
		}
	}
#endif

	///

	// (a.walling 2013-07-22 17:30) - PLID 57669 - NxCache - EmrTableDropdownStampFilterT
	namespace EmrTableDropdownStampFilter
	{
		// (a.walling 2013-07-18 10:04) - PLID 57629 - Get max revision
		__int64 GetMaxDataRevision(sqlite::Connection& sql)
		{
			return detail::GetIntegerProp(sql, "EmrTableDropdownStampFilterT_Revision");
		}

		CSqlFragment SourceQuery()
		{
			return CSqlFragment(
				"SELECT "
					  "EmrTableDropdownInfoID"
					", StampID "
				"FROM EmrTableDropdownStampFilterT "
			);
		}

		// (a.walling 2015-04-27 09:31) - PLID 65744 - Further optimizations to queries in NxCache via EMRTableDropdownInfoJoinedV
		CSqlFragment ActiveQuery()
		{
			return CSqlFragment(
				"{SQL}"
				"{SQL}"
#ifndef NXCACHE_ALL_THE_THINGS
				"WHERE EmrTableDropdownStampFilterT.EmrTableDropdownInfoID IN ("
					"SELECT EMRTableDropdownInfoJoinedV.ID "
					"FROM EMRTableDropdownInfoJoinedV "
					"WHERE EMRTableDropdownInfoJoinedV.EMRInfoID IN ( "
						"SELECT ActiveEMRInfoID "
						"FROM EMRInfoMasterT "
					") "
				") "
#endif
				"ORDER BY EmrTableDropdownStampFilterT.EmrTableDropdownInfoID, EmrTableDropdownStampFilterT.StampID"
				, MaxRevisionQuery("EmrTableDropdownStampFilterT")
				, SourceQuery()
			);
		}

		CSqlFragment ChangedQuery(sqlite::Connection& sql)
		{
			__int64 maxRevision = GetMaxDataRevision(sql);

			return CSqlFragment(
				"{SQL}"
				"{SQL}"
				"WHERE EmrTableDropdownStampFilterT.EmrTableDropdownInfoID IN ("
					"SELECT EmrTableDropdownInfoID "
					"FROM EmrTableDropdownStampFilterT "
					"WHERE Revision > CONVERT(ROWVERSION, {BIGINT}) "
				") "
				"ORDER BY EmrTableDropdownStampFilterT.EmrTableDropdownInfoID, EmrTableDropdownStampFilterT.StampID"
				, MaxRevisionQuery("EmrTableDropdownStampFilterT")
				, SourceQuery()
				, maxRevision
			);
		}

		// (a.walling 2013-07-23 20:49) - PLID 57685 - Load all data for an individual EmrInfoT.ID
		CSqlFragment ByEmrInfoQuery()
		{
			// (a.walling 2014-10-15 13:50) - PLID 63921 - EMRTableDropdownInfoJoinedV
			return CSqlFragment(
				"{SQL}"
				"WHERE EmrTableDropdownStampFilterT.EmrTableDropdownInfoID IN ("
					"SELECT ID "
					"FROM EMRTableDropdownInfoJoinedV "
					"WHERE EMRInfoID = @EmrInfoID "
				")"
				, SourceQuery()
			);
		}
		
		__int64 InsertData(ADODB::_Recordset* prs, sqlite::Connection& sql, __int64 maxRevision)
		{
			if (prs->eof) {
				detail::UpdateMaxProp(sql, "EmrTableDropdownStampFilterT_Revision", maxRevision);
				return 0LL;
			}
			
			CNxPerform nxp(__FUNCTION__);

			sqlite::Statement insertStampFilter(sql, 
				"insert or replace into EmrTableDropdownStampFilterT( "
					  "EmrTableDropdownInfoID"
					", Stamps"
				") values (?, ?);"
			);

			

			int dropdownCount = 0;
			int stampCount = 0;

			// keeping values in a blob of N integers rather than N extra rows
			std::vector<long> stamps;
			stamps.reserve(2048);

			long curDropdownID = -1;
			long dropdownID = -1;
			
			//sqlite:://savepoint //savepoint(sql, "EmrTableDropdownStampFilter_Active");
			//savepoint.Begin();
			while (!prs->eof) {
				++dropdownCount;
				long dropdownID = AdoFldLong(prs, "EmrTableDropdownInfoID");
				stamps.clear();

				do {						
					stamps.push_back(AdoFldLong(prs, "StampID"));
					++stampCount;
					prs->MoveNext();
					if (prs->eof) {
						break;
					}
					curDropdownID = AdoFldLong(prs, "EmrTableDropdownInfoID");
					if (curDropdownID != dropdownID) {
						break;
					}
				} while (!prs->eof);

				// (a.walling 2014-12-04 09:00) - boost 1.57 - no cref of &&
				auto stampsBlob = sqlite::Blob<long>(stamps);
				insertStampFilter.Reset().Bind()
					<< dropdownID
					<< boost::cref(stampsBlob)
				;
				insertStampFilter.Exec();
			}

			detail::UpdateMaxProp(sql, "EmrTableDropdownStampFilterT_Revision", maxRevision);

			//savepoint.Commit();
			

			nxp.Tick("%li records, %li stamps, max revision %I64i", dropdownCount, stampCount, maxRevision);

			return maxRevision;
		}
		
		// (a.walling 2013-07-18 10:08) - PLID 57630 - Asynchronously pre-populate cache with active items
		ADODB::_RecordsetPtr GatherActiveData(ADODB::_RecordsetPtr prs, sqlite::Connection& sql)
		{
			CNxPerform nxp(__FUNCTION__);

			__int64 maxRevision = AdoFldBigInt(prs, "MaxRevision");
			prs = prs->NextRecordset(NULL);

			InsertData(prs, sql, maxRevision);
			return prs;
		}
		
		// (a.walling 2013-07-18 10:04) - PLID 57629 - Gathers all data changed since last inserted revision
		ADODB::_RecordsetPtr GatherChangedData(ADODB::_RecordsetPtr prs, sqlite::Connection& sql)
		{
			__int64 maxRevision = AdoFldBigInt(prs, "MaxRevision");
			prs = prs->NextRecordset(NULL);

			InsertData(prs, sql, maxRevision);
			return prs;
		}

		void GatherDataForDropdownID(ADODB::_Connection* pCon, sqlite::Connection& sql, long nID)
		{
			CNxPerform nxp(__FUNCTION__);

			ADODB::_RecordsetPtr prs = CreateParamRecordset(pCon, ADODB::adUseServer, ADODB::adOpenForwardOnly, ADODB::adLockReadOnly, ADODB::adCmdText, 
				"{SQL}"
				"WHERE EmrTableDropdownStampFilterT.EmrTableDropdownInfoID = {INT}"
				"ORDER BY EmrTableDropdownStampFilterT.EmrTableDropdownInfoID, EmrTableDropdownStampFilterT.StampID"
				, SourceQuery()
				, nID
			);

			InsertData(prs, sql, 0LL);
		}
	}

	///

	// (a.walling 2013-07-22 17:30) - PLID 57669 - NxCache - EmrTableDropdownStampDefaultsT
	namespace EmrTableDropdownStampDefaults
	{
		// (a.walling 2013-07-18 10:04) - PLID 57629 - Get max revision
		__int64 GetMaxDataRevision(sqlite::Connection& sql)
		{
			return detail::GetIntegerProp(sql, "EmrTableDropdownStampDefaultsT_Revision");
		}

		CSqlFragment SourceQuery()
		{
			return CSqlFragment(
				"SELECT "
					  "EmrTableDropdownInfoID"
					", StampID "
				"FROM EmrTableDropdownStampDefaultsT "
			);
		}

		// (a.walling 2015-04-27 09:31) - PLID 65744 - Further optimizations to queries in NxCache via EMRTableDropdownInfoJoinedV
		CSqlFragment ActiveQuery()
		{	
			return CSqlFragment(
				"{SQL}"
				"{SQL}"
#ifndef NXCACHE_ALL_THE_THINGS
				"WHERE EmrTableDropdownStampDefaultsT.EmrTableDropdownInfoID IN ("
					"SELECT EMRTableDropdownInfoJoinedV.ID "
					"FROM EMRTableDropdownInfoJoinedV "
					"WHERE EMRTableDropdownInfoJoinedV.EMRInfoID IN ( "
						"SELECT ActiveEMRInfoID "
						"FROM EMRInfoMasterT "
					") "
				") "
#endif
				"ORDER BY EmrTableDropdownStampDefaultsT.EmrTableDropdownInfoID, EmrTableDropdownStampDefaultsT.StampID"
				, MaxRevisionQuery("EmrTableDropdownStampDefaultsT")
				, SourceQuery()
			);
		}

		CSqlFragment ChangedQuery(sqlite::Connection& sql)
		{
			__int64 maxRevision = GetMaxDataRevision(sql);

			return CSqlFragment(
				"{SQL}"
				"{SQL}"
				"WHERE EmrTableDropdownStampDefaultsT.EmrTableDropdownInfoID IN ("
					"SELECT EmrTableDropdownInfoID "
					"FROM EmrTableDropdownStampDefaultsT "
					"WHERE Revision > CONVERT(ROWVERSION, {BIGINT}) "
				") "
				"ORDER BY EmrTableDropdownStampDefaultsT.EmrTableDropdownInfoID, EmrTableDropdownStampDefaultsT.StampID"
				, MaxRevisionQuery("EmrTableDropdownStampDefaultsT")
				, SourceQuery()
				, maxRevision
			);
		}

		// (a.walling 2013-07-23 20:49) - PLID 57685 - Load all data for an individual EmrInfoT.ID
		CSqlFragment ByEmrInfoQuery()
		{
			// (a.walling 2014-10-15 13:50) - PLID 63921 - EMRTableDropdownInfoJoinedV
			return CSqlFragment(
				"{SQL}"
				"WHERE EmrTableDropdownStampDefaultsT.EmrTableDropdownInfoID IN ("
					"SELECT ID "
					"FROM EMRTableDropdownInfoJoinedV "
					"WHERE EMRInfoID = @EmrInfoID "
				")"
				, SourceQuery()
			);
		}
		
		__int64 InsertData(ADODB::_Recordset* prs, sqlite::Connection& sql, __int64 maxRevision)
		{
			if (prs->eof) {
				detail::UpdateMaxProp(sql, "EmrTableDropdownStampDefaultsT_Revision", maxRevision);
				return 0LL;
			}
			
			CNxPerform nxp(__FUNCTION__);

			sqlite::Statement insertStampFilter(sql, 
				"insert or replace into EmrTableDropdownStampDefaultsT( "
					  "EmrTableDropdownInfoID"
					", Stamps"
				") values (?, ?);"
			);

			

			int dropdownCount = 0;
			int stampCount = 0;

			// keeping values in a blob of N integers rather than N extra rows
			std::vector<long> stamps;
			stamps.reserve(2048);

			long curDropdownID = -1;
			long dropdownID = -1;
			
			//sqlite:://savepoint //savepoint(sql, "EmrTableDropdownStampDefaults_Active");
			//savepoint.Begin();
			while (!prs->eof) {
				++dropdownCount;
				long dropdownID = AdoFldLong(prs, "EmrTableDropdownInfoID");
				stamps.clear();

				do {						
					stamps.push_back(AdoFldLong(prs, "StampID"));
					++stampCount;
					prs->MoveNext();
					if (prs->eof) {
						break;
					}
					curDropdownID = AdoFldLong(prs, "EmrTableDropdownInfoID");
					if (curDropdownID != dropdownID) {
						break;
					}
				} while (!prs->eof);

				// (a.walling 2014-12-04 09:00) - boost 1.57 - no cref of &&
				auto stampsBlob = sqlite::Blob<long>(stamps);
				insertStampFilter.Reset().Bind()
					<< dropdownID
					<< boost::cref(stampsBlob)
				;
				insertStampFilter.Exec();
			}

			detail::UpdateMaxProp(sql, "EmrTableDropdownStampDefaultsT_Revision", maxRevision);

			//savepoint.Commit();
			

			nxp.Tick("%li records, %li stamps, max revision %I64i", dropdownCount, stampCount, maxRevision);

			return maxRevision;
		}
		
		// (a.walling 2013-07-18 10:08) - PLID 57630 - Asynchronously pre-populate cache with active items
		ADODB::_RecordsetPtr GatherActiveData(ADODB::_RecordsetPtr prs, sqlite::Connection& sql)
		{
			CNxPerform nxp(__FUNCTION__);

			__int64 maxRevision = AdoFldBigInt(prs, "MaxRevision");
			prs = prs->NextRecordset(NULL);

			InsertData(prs, sql, maxRevision);
			return prs;
		}
		
		// (a.walling 2013-07-18 10:04) - PLID 57629 - Gathers all data changed since last inserted revision
		ADODB::_RecordsetPtr GatherChangedData(ADODB::_RecordsetPtr prs, sqlite::Connection& sql)
		{
			__int64 maxRevision = AdoFldBigInt(prs, "MaxRevision");
			prs = prs->NextRecordset(NULL);

			InsertData(prs, sql, maxRevision);
			return prs;
		}

		void GatherDataForDropdownID(ADODB::_Connection* pCon, sqlite::Connection& sql, long nID)
		{
			CNxPerform nxp(__FUNCTION__);

			ADODB::_RecordsetPtr prs = CreateParamRecordset(pCon, ADODB::adUseServer, ADODB::adOpenForwardOnly, ADODB::adLockReadOnly, ADODB::adCmdText, 
				"{SQL}"
				"WHERE EmrTableDropdownStampDefaultsT.EmrTableDropdownInfoID = {INT}"
				"ORDER BY EmrTableDropdownStampDefaultsT.EmrTableDropdownInfoID, EmrTableDropdownStampDefaultsT.StampID"
				, SourceQuery()
				, nID
			);

			InsertData(prs, sql, 0LL);
		}
	}

	///
		
	// (a.walling 2013-07-22 17:30) - PLID 57669 - NxCache - EmrProviderFloatTableDropdownT
	namespace EmrProviderFloatTableDropdown
	{
		// (a.walling 2013-07-18 10:04) - PLID 57629 - Get max revision
		__int64 GetMaxDataRevision(sqlite::Connection& sql)
		{
			return detail::GetIntegerProp(sql, "EmrProviderFloatTableDropdownT_Revision");
		}

		CSqlFragment SourceQuery()
		{
			return CSqlFragment(
				"SELECT "
					  "ProviderID"
					", EmrTableDropdownGroupID"
					", Count "
				"FROM EmrProviderFloatTableDropdownT "
			);
		}
		
		CSqlFragment ActiveQuery()
		{
			return CSqlFragment(
				"{SQL}"
				"{SQL}"
				, MaxRevisionQuery("EmrProviderFloatTableDropdownT")
				, SourceQuery()
			);
		}

		CSqlFragment ChangedQuery(sqlite::Connection& sql)
		{
			__int64 maxRevision = GetMaxDataRevision(sql);

			return CSqlFragment(
				"{SQL}"
				"{SQL}"
				"WHERE EmrProviderFloatTableDropdownT.Revision > CONVERT(ROWVERSION, {BIGINT}) "
				, MaxRevisionQuery("EmrProviderFloatTableDropdownT")
				, SourceQuery()
				, maxRevision
			);
		}
		
		__int64 InsertData(ADODB::_Recordset* prs, sqlite::Connection& sql, __int64 maxRevision)
		{
			if (prs->eof) {
				detail::UpdateMaxProp(sql, "EmrProviderFloatTableDropdownT_Revision", maxRevision);
				return 0LL;
			}
			
			CNxPerform nxp(__FUNCTION__);

			sqlite::Statement insertFloat(sql, 
				"insert or replace into EmrProviderFloatTableDropdownT( "
					  "EmrTableDropdownGroupID"
					", ProviderID"
					", Count"
				") values (?, ?, ?);"
			);

			

			int count = 0;
			
			//sqlite:://savepoint //savepoint(sql, "EmrProviderFloatTableDropdown_Active");
			//savepoint.Begin();
			while (!prs->eof) {
				++count;

				insertFloat.Reset().Bind()
					<< AdoFldLong(prs, "EmrTableDropdownGroupID")
					<< AdoFldLong(prs, "ProviderID")
					<< AdoFldLong(prs, "Count")
				;

				prs->MoveNext();

				insertFloat.Exec();
			}

			detail::UpdateMaxProp(sql, "EmrProviderFloatTableDropdownT_Revision", maxRevision);

			//savepoint.Commit();
			

			nxp.Tick("%li records, max revision %I64i", count, maxRevision);

			return maxRevision;
		}
		
		// (a.walling 2013-07-18 10:08) - PLID 57630 - Asynchronously pre-populate cache with active items
		ADODB::_RecordsetPtr GatherActiveData(ADODB::_RecordsetPtr prs, sqlite::Connection& sql)
		{
			CNxPerform nxp(__FUNCTION__);

			__int64 maxRevision = AdoFldBigInt(prs, "MaxRevision");
			prs = prs->NextRecordset(NULL);

			InsertData(prs, sql, maxRevision);
			return prs;
		}
		
		// (a.walling 2013-07-18 10:04) - PLID 57629 - Gathers all data changed since last inserted revision
		ADODB::_RecordsetPtr GatherChangedData(ADODB::_RecordsetPtr prs, sqlite::Connection& sql)
		{
			__int64 maxRevision = AdoFldBigInt(prs, "MaxRevision");
			prs = prs->NextRecordset(NULL);

			InsertData(prs, sql, maxRevision);
			return prs;
		}
	}

	///

	// (a.walling 2013-07-22 17:30) - PLID 57671 - NxCache - EmrInfoT
	namespace EmrInfo
	{
		// packed well
		struct Object
		{
			long		PreviewFlags;
			long		DataCodeID;
			long		ChildEmrInfoMasterID;
			long		GlassesOrderLens;

			long		InfoFlags;
			char		DataFormat;
			char		BackgroundImageType;

			double		InputDate;
			double		ModifiedDate;
			double		SliderMin;
			double		SliderMax;

			double		SliderInc;

			// using bitfields for 1 bit / bool
			bool		OnePerEmn					:1;
			bool		RememberForPatient			:1;
			bool		AutoAlphabetizeListData		:1;
			bool		DisableTableBorder			:1;
			bool		RememberForEMR				:1;
			bool		TableRowsAsFields			:1;
			bool		SmartStampsEnabled			:1;
			bool		HasGlassesOrderData			:1;
			bool		HasContactLensData			:1;
			bool		UseWithWoundCareCoding		:1;
		};

		
		// (a.walling 2013-07-18 10:04) - PLID 57629 - Get max revision
		__int64 GetMaxDataRevision(sqlite::Connection& sql)
		{
			return detail::GetIntegerProp(sql, "EmrInfoT_Revision");
		}

		CSqlFragment SourceQuery()
		{
			return CSqlFragment(
				"SELECT "
					"  ID "
					", Name "
					", DataType "
					", LongForm "
					", BackgroundImageFilePath "
					", BackgroundImageType "
					", DataFormat "
					", DataSeparator "
					", DefaultText "
					", RememberForPatient "
					", SliderMin "
					", SliderMax "
					", SliderInc "
					", DataSeparatorFinal "
					", OnePerEmn "
					", AutoAlphabetizeListData "
					", DisableTableBorder "
					", InputDate "
					", EmrInfoMasterID "
					", DataSubType "
					", PreviewFlags "
					", RememberForEMR "
					", TableRowsAsFields "
					", DataCodeID "
					", DataUnit "
					", ChildEmrInfoMasterID "
					", SmartStampsEnabled "
					", HasGlassesOrderData "
					", GlassesOrderLens "
					", InfoFlags "
					", HasContactLensData "
					", UseWithWoundCareCoding "
					", ModifiedDate "
				"FROM EmrInfoT "
			);
		}

		CSqlFragment ActiveQuery()
		{			
			return CSqlFragment(
				"{SQL}"
				"{SQL}"
#ifndef NXCACHE_ALL_THE_THINGS
				"WHERE EMRInfoT.ID IN ("
					"SELECT ActiveEMRInfoID "
					"FROM EMRInfoMasterT "
				")"
#endif
				, MaxRevisionQuery("EMRInfoT")
				, SourceQuery()
			);
		}

		CSqlFragment ChangedQuery(sqlite::Connection& sql)
		{
			__int64 maxRevision = GetMaxDataRevision(sql);

			return CSqlFragment(
				"{SQL}"
				"{SQL}"
				"WHERE EMRInfoT.Revision > CONVERT(ROWVERSION, {BIGINT}) "
				, MaxRevisionQuery("EMRInfoT")
				, SourceQuery()
				, maxRevision
			);
		}

		// (a.walling 2013-07-23 20:49) - PLID 57685 - Load all data for an individual EmrInfoT.ID
		CSqlFragment ByEmrInfoQuery()
		{
			return CSqlFragment(
				"{SQL}"
				"WHERE EMRInfoT.ID = @EmrInfoID"
				, SourceQuery()
			);
		}
		
		__int64 InsertData(ADODB::_Recordset* prs, sqlite::Connection& sql, __int64 maxRevision)
		{
			if (prs->eof) {
				detail::UpdateMaxProp(sql, "EmrInfoT_Revision", maxRevision);
				return 0LL;
			}
			
			CNxPerform nxp(__FUNCTION__);

			sqlite::Statement insertInfo(sql, 
				"insert or replace into EmrInfoT( "
					  "ID "
					", Name "
					", EmrInfoMasterID "
					", DataType "
					", DataSubType "
					", LongForm "
					", BackgroundImageFilePath "
					", DataSeparator "
					", DefaultText "
					", DataSeparatorFinal "
					", DataUnit "
					", Object "
				") values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"
			);

			

			int count = 0;
			
			//sqlite:://savepoint //savepoint(sql, "EmrInfo_Active");
			//savepoint.Begin();
			while (!prs->eof) {
				++count;

				Object o;

				o.DataFormat		= (char)AdoFldLong(prs, "DataFormat", 0);
				o.PreviewFlags		= AdoFldLong(prs, "PreviewFlags", 0);
				o.DataCodeID		= AdoFldLong(prs, "DataCodeID", -1);
				o.ChildEmrInfoMasterID = AdoFldLong(prs, "ChildEmrInfoMasterID", -1);
				o.GlassesOrderLens	= AdoFldLong(prs, "GlassesOrderLens", -1);
				o.InfoFlags			= AdoFldLong(prs, "InfoFlags", 0);

				o.InputDate			= AdoFldDateTime(prs, "InputDate", g_cdtNull).m_dt;
				o.ModifiedDate		= AdoFldDateTime(prs, "ModifiedDate", g_cdtNull).m_dt;
				o.SliderMin			= AdoFldDouble(prs, "SliderMin", 0.0);
				o.SliderMax			= AdoFldDouble(prs, "SliderMax", 0.0);
				o.SliderInc			= AdoFldDouble(prs, "SliderInc", 0.0);

				o.BackgroundImageType = (char)AdoFldLong(prs, "BackgroundImageType", -1);

				o.OnePerEmn			 = !!AdoFldBool(prs, "OnePerEmn", false);
				o.RememberForPatient = !!AdoFldBool(prs, "RememberForPatient", false);
				o.AutoAlphabetizeListData = !!AdoFldBool(prs, "AutoAlphabetizeListData", false);
				o.DisableTableBorder = !!AdoFldBool(prs, "DisableTableBorder", false);
				o.RememberForEMR	 = !!AdoFldBool(prs, "RememberForEMR", false);
				o.TableRowsAsFields	 = !!AdoFldBool(prs, "TableRowsAsFields", false);
				o.SmartStampsEnabled = !!AdoFldBool(prs, "SmartStampsEnabled", false);
				o.HasGlassesOrderData = !!AdoFldBool(prs, "HasGlassesOrderData", false);
				o.HasContactLensData = !!AdoFldBool(prs, "HasContactLensData", false);
				o.UseWithWoundCareCoding = !!AdoFldBool(prs, "UseWithWoundCareCoding", false);
			
				// (a.walling 2014-12-04 09:00) - boost 1.57 - no cref of &&	
				auto oBlob = sqlite::Blob<Object>(o);

				insertInfo.Reset().Bind()
					<< AdoFldLong(prs, "ID")
					<< prs->Collect["Name"]
					<< AdoFldLong(prs, "EmrInfoMasterID")
					<< AdoFldByte(prs, "DataType")
					<< AdoFldByte(prs, "DataSubType")
					<< prs->Collect["LongForm"]
					<< prs->Collect["BackgroundImageFilePath"]
					<< prs->Collect["DataSeparator"]
					<< prs->Collect["DefaultText"]
					<< prs->Collect["DataSeparatorFinal"]
					<< prs->Collect["DataUnit"]
					<< boost::cref(oBlob)
				;

				prs->MoveNext();

				insertInfo.Exec();
			}

			detail::UpdateMaxProp(sql, "EmrInfoT_Revision", maxRevision);

			//savepoint.Commit();
			

			nxp.Tick("%li records, max revision %I64i", count, maxRevision);

			return maxRevision;
		}
		
		// (a.walling 2013-07-18 10:08) - PLID 57630 - Asynchronously pre-populate cache with active items
		ADODB::_RecordsetPtr GatherActiveData(ADODB::_RecordsetPtr prs, sqlite::Connection& sql)
		{
			CNxPerform nxp(__FUNCTION__);

			__int64 maxRevision = AdoFldBigInt(prs, "MaxRevision");
			prs = prs->NextRecordset(NULL);

			InsertData(prs, sql, maxRevision);
			return prs;
		}
		
		// (a.walling 2013-07-18 10:04) - PLID 57629 - Gathers all data changed since last inserted revision
		ADODB::_RecordsetPtr GatherChangedData(ADODB::_RecordsetPtr prs, sqlite::Connection& sql)
		{
			__int64 maxRevision = AdoFldBigInt(prs, "MaxRevision");
			prs = prs->NextRecordset(NULL);

			InsertData(prs, sql, maxRevision);
			return prs;
		}
	}

	///

#ifdef NXCACHE_EXTRA
	// (a.walling 2013-07-22 17:30) - PLID 57670 - NxCache - EmrTemplateT
	namespace EmrTemplate
	{		
		// (a.walling 2013-07-18 10:04) - PLID 57629 - Get max revision
		__int64 GetMaxDataRevision(sqlite::Connection& sql)
		{
			return detail::GetIntegerProp(sql, "EmrTemplateT_Revision");
		}

#pragma TODO("PLID 60397 - AddHideTitleOnPreview to this if necessary")
		CSqlFragment SourceQuery()
		{
			return CSqlFragment(
				"SELECT "
					  "ID "
					", CollectionID "
					", Name "
					", AddOnce "
					", Deleted "
					", LastVersion "
					", CreatedDate "
					", CreatedLogin "
					", ModifiedDate "
					", ModifiedLogin "
					", EmnTabChartID "
					", EmnTabCategoryID "
					", VisitTypeID "
					", IsUniversal "
				"FROM EMRTemplateT "
			);
		}

		CSqlFragment ActiveQuery()
		{
			return CSqlFragment(
				"{SQL}"
				"{SQL}"
				, MaxRevisionQuery("EMRTemplateT")
				, SourceQuery()
			);
		}

		CSqlFragment ChangedQuery(sqlite::Connection& sql)
		{
			__int64 maxRevision = GetMaxDataRevision(sql);

			return CSqlFragment(
				"{SQL}"
				"{SQL}"
				"WHERE EMRTemplateT.Revision > CONVERT(ROWVERSION, {BIGINT}) "
				, MaxRevisionQuery("EMRTemplateT")
				, SourceQuery()
				, maxRevision
			);
		}
		
		__int64 InsertData(ADODB::_Recordset* prs, sqlite::Connection& sql, __int64 maxRevision)
		{
			if (prs->eof) {
				detail::UpdateMaxProp(sql, "EmrTemplateT_Revision", maxRevision);
				return 0LL;
			}
			
			CNxPerform nxp(__FUNCTION__);

#pragma TODO("PLID 60397 - AddHideTitleOnPreview to this if necessary")
			sqlite::Statement insertTemplate(sql, 
				"insert or replace into EmrTemplateT( "
					"  ID "
					", CollectionID "
					", Name "
					", AddOnce "
					", Deleted "
					", LastVersion "
					", CreatedDate "
					", CreatedLogin "
					", ModifiedDate "
					", ModifiedLogin "
					", EmnTabChartID "
					", EmnTabCategoryID "
					", VisitTypeID "
					", IsUniversal"
				") values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"
			);

			

			int count = 0;
			
			//sqlite:://savepoint //savepoint(sql, "EmrTemplate_Active");
			//savepoint.Begin();
			while (!prs->eof) {
				++count;
				
#pragma TODO("PLID 60397 - AddHideTitleOnPreview to this if necessary")
				insertTemplate.Reset().Bind()
					<< AdoFldLong(prs, "ID")
					<< prs->Collect["CollectionID"]
					<< prs->Collect["Name"]
					<< AdoFldBool(prs, "AddOnce")
					<< AdoFldBool(prs, "Deleted")
					<< prs->Collect["LastVersion"]
					<< prs->Collect["CreatedDate"]
					<< prs->Collect["CreatedLogin"]
					<< prs->Collect["ModifiedDate"]
					<< prs->Collect["ModifiedLogin"]
					<< prs->Collect["EmnTabChartID"]
					<< prs->Collect["EmnTabCategoryID"]
					<< prs->Collect["VisitTypeID"]
					<< AdoFldBool(prs, "IsUniversal")
				;

				prs->MoveNext();

				insertTemplate.Exec();
			}

			detail::UpdateMaxProp(sql, "EmrTemplateT_Revision", maxRevision);

			//savepoint.Commit();
			

			nxp.Tick("%li records, max revision %I64i", count, maxRevision);

			return maxRevision;
		}
		
		// (a.walling 2013-07-18 10:08) - PLID 57630 - Asynchronously pre-populate cache with active items
		ADODB::_RecordsetPtr GatherActiveData(ADODB::_RecordsetPtr prs, sqlite::Connection& sql)
		{
			CNxPerform nxp(__FUNCTION__);

			__int64 maxRevision = AdoFldBigInt(prs, "MaxRevision");
			prs = prs->NextRecordset(NULL);

			InsertData(prs, sql, maxRevision);
			return prs;
		}
		
		// (a.walling 2013-07-18 10:04) - PLID 57629 - Gathers all data changed since last inserted revision
		ADODB::_RecordsetPtr GatherChangedData(ADODB::_RecordsetPtr prs, sqlite::Connection& sql)
		{
			__int64 maxRevision = AdoFldBigInt(prs, "MaxRevision");
			prs = prs->NextRecordset(NULL);

			InsertData(prs, sql, maxRevision);
			return prs;
		}
	}
#endif

	///

	namespace detail
	{
		// (a.walling 2013-07-18 10:08) - PLID 57630 - pre-populate cache
#ifdef NXCACHE_PERSISTENT
		bool ShouldPopulateInitialCache()
		{
			if(g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
				return false;
			}

			{
				sqlite::Connection& sql = GetCacheConnection().sql;

				sqlite::Statement checkExisting(sql,
					"select count(*) from Props where Name like '%\\_Revision' escape '\\'"
				);
				
				sqlite::Row row = checkExisting.Next();
				int count = row[0];
				if (count > 0) {
					return false;
				}
			}

			return true;
		}
#else
		bool ShouldPopulateInitialCache()
		{
			if(g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
				return false;
			}

			return true;
		}
#endif

		// (a.walling 2013-07-18 10:08) - PLID 57630 - pre-populate cache
		void PopulateInitialCache()
		{
			try {
				CNxPerform nxp(__FUNCTION__);

				ADODB::_ConnectionPtr pCon = GetRemoteDataSnapshot();
				CIncreaseCommandTimeout noTimeout(pCon, 0);

				sqlite::Connection& sql = GetCacheConnection().sql;

				sqlite::Transaction trans(sql);
				trans.Begin();

				ADODB::_RecordsetPtr prs = CreateParamRecordset(pCon, ADODB::adUseServer, ADODB::adOpenForwardOnly, ADODB::adLockReadOnly, ADODB::adCmdText, 
					"BEGIN TRANSACTION\r\n"
					"SET NOCOUNT ON\r\n"
					"{SQL}\r\n"
					"{SQL}\r\n"
					"{SQL}\r\n"
					"{SQL}\r\n"
					"{SQL}\r\n"
#ifdef NXCACHE_EXTRA
					"{SQL}\r\n"
					"{SQL}\r\n"
#endif
					"SET NOCOUNT OFF\r\n"
					"COMMIT TRANSACTION\r\n"
					, EmrTableDropdownInfo::ActiveQuery()
					, EmrTableDropdownStampFilter::ActiveQuery()
					, EmrTableDropdownStampDefaults::ActiveQuery()
					, EmrProviderFloatTableDropdown::ActiveQuery()
					, EmrInfo::ActiveQuery()
#ifdef NXCACHE_EXTRA
					, EmrImageStamps::ActiveQuery()
					, EmrTemplate::ActiveQuery()
#endif
				);

				prs = EmrTableDropdownInfo::GatherActiveData(prs, sql)
					->NextRecordset(NULL);
				prs = EmrTableDropdownStampFilter::GatherActiveData(prs, sql)
					->NextRecordset(NULL);
				prs = EmrTableDropdownStampDefaults::GatherActiveData(prs, sql)
					->NextRecordset(NULL);
				prs = EmrProviderFloatTableDropdown::GatherActiveData(prs, sql)
					->NextRecordset(NULL);
				prs = EmrInfo::GatherActiveData(prs, sql)
#ifdef NXCACHE_EXTRA
					->NextRecordset(NULL);
				prs = EmrImageStamps::GatherActiveData(prs, sql)
					->NextRecordset(NULL);
				prs = EmrTemplate::GatherActiveData(prs, sql)
#endif
				;

				trans.Commit();

				sql.Checkpoint();

			} NxCatchAll(__FUNCTION__);

			cacheComplete.SetEvent();
			cacheReady = true;
		}

		CSqlFragment GetUpdateQuery(sqlite::Connection& sql)
		{
			return CSqlFragment(
				"{SQL}\r\n"
				"{SQL}\r\n"
				"{SQL}\r\n"
				"{SQL}\r\n"
				"{SQL}\r\n"
#ifdef NXCACHE_EXTRA
				"{SQL}\r\n"
				"{SQL}\r\n"
#endif
				, EmrTableDropdownInfo::ChangedQuery(sql)
				, EmrTableDropdownStampFilter::ChangedQuery(sql)
				, EmrTableDropdownStampDefaults::ChangedQuery(sql)
				, EmrProviderFloatTableDropdown::ChangedQuery(sql)
				, EmrInfo::ChangedQuery(sql)
#ifdef NXCACHE_EXTRA
				, EmrImageStamps::ChangedQuery(sql)
				, EmrTemplate::ChangedQuery(sql)
#endif
			);
		}

		ADODB::_RecordsetPtr ProcessUpdates(ADODB::_RecordsetPtr prs, sqlite::Connection& sql)
		{
			prs = EmrTableDropdownInfo::GatherChangedData(prs, sql)
				->NextRecordset(NULL);
			prs = EmrTableDropdownStampFilter::GatherChangedData(prs, sql)
				->NextRecordset(NULL);
			prs = EmrTableDropdownStampDefaults::GatherChangedData(prs, sql)
				->NextRecordset(NULL);
			prs = EmrProviderFloatTableDropdown::GatherChangedData(prs, sql)
				->NextRecordset(NULL);
			prs = EmrInfo::GatherChangedData(prs, sql)
				->NextRecordset(NULL);
#ifdef NXCACHE_EXTRA
			prs = EmrImageStamps::GatherChangedData(prs, sql)
				->NextRecordset(NULL);
			prs = EmrTemplate::GatherChangedData(prs, sql)
				->NextRecordset(NULL);
#endif
			return prs;
		}

		// (a.walling 2013-07-18 10:04) - PLID 57629 - Update changed cached data
		void UpdateCache()
		{
			try {
				CNxPerform nxp(__FUNCTION__);

				sqlite::Connection& sql = GetConnection();
				ADODB::_ConnectionPtr pCon = GetRemoteDataSnapshot();
				CIncreaseCommandTimeout noTimeout(pCon, 0);

				sqlite::Transaction trans(sql);
				trans.Begin();
				
				ADODB::_RecordsetPtr prs = CreateParamRecordset(pCon, ADODB::adUseServer, ADODB::adOpenForwardOnly, ADODB::adLockReadOnly, ADODB::adCmdText, 
					"BEGIN TRANSACTION\r\n"
					"SET NOCOUNT ON\r\n"
					"{SQL}\r\n"
					"SET NOCOUNT OFF\r\n"
					"COMMIT TRANSACTION\r\n"
					, GetUpdateQuery(sql)
				);

				prs = ProcessUpdates(prs, sql);
				
				trans.Commit();

				sql.Checkpoint();

			} NxCatchAll(__FUNCTION__);
		}		

		// (a.walling 2013-07-23 20:49) - PLID 57685 - Load all data for an individual EmrInfoT.ID
		void LoadEmrInfo(long nID)
		{
			try {
				CNxPerform nxp(__FUNCTION__);

				sqlite::Connection& sql = GetConnection();
				ADODB::_ConnectionPtr pCon = GetRemoteDataSnapshot();
				CIncreaseCommandTimeout noTimeout(pCon, 0);

				sqlite::Transaction trans(sql);
				trans.Begin();
				
				ADODB::_RecordsetPtr prs = CreateParamRecordset(pCon, ADODB::adUseServer, ADODB::adOpenForwardOnly, ADODB::adLockReadOnly, ADODB::adCmdText, 
					"BEGIN TRANSACTION\r\n"
					"SET NOCOUNT ON\r\n"
					"DECLARE @EmrInfoID INT; SET @EmrInfoID = {INT};\r\n"
					"{SQL}\r\n"
					"{SQL}\r\n"
					"{SQL}\r\n"
					"{SQL}\r\n"
					"SET NOCOUNT OFF\r\n"
					"COMMIT TRANSACTION\r\n"
					, nID
					, EmrTableDropdownInfo::ByEmrInfoQuery()
					, EmrTableDropdownStampFilter::ByEmrInfoQuery()
					, EmrTableDropdownStampDefaults::ByEmrInfoQuery()
					, EmrInfo::ByEmrInfoQuery()
				);

				EmrTableDropdownInfo::InsertData(prs, sql, 0LL);
				prs = prs->NextRecordset(NULL);
				EmrTableDropdownStampFilter::InsertData(prs, sql, 0LL);
				prs = prs->NextRecordset(NULL);
				EmrTableDropdownStampDefaults::InsertData(prs, sql, 0LL);
				prs = prs->NextRecordset(NULL);
				EmrInfo::InsertData(prs, sql, 0LL);
				
				trans.Commit();
			} NxCatchAll(__FUNCTION__);
		}

		void InitSchema(sqlite::Connection& sql)
		{		
			sqlite::Transaction trans(sql);
			trans.Begin();

			// (a.walling 2013-07-18 10:04) - PLID 57629 - Generic Name-Value map
			sql.Exec(
				"create table if not exists Props( "
					  "Name not null primary key"
					", Value not null"
				") "
			);
			
			// (a.walling 2013-07-18 09:58) - PLID 57627 - Schema for EMRTableDropdownInfoT.ID->Data mappings
			sql.Exec(
				"create table if not exists EmrTableDropdownInfoT( "
					  "ID integer not null primary key"
				    ", EmrDataID integer not null"
					", TextID integer not null" // references EmrTableDropdownInfoTextT.rowid
					", SortOrder integer not null"
					", Inactive integer not null"
					", DropdownGroupID integer not null"
					", GlassesOrderDataID integer not null"
					", unique (EmrDataID, ID)"
				") "
			);

			// (a.walling 2013-07-18 09:58) - PLID 57627 - Since EMRTableDropdownInfoT.Data is often duplicated, store that in a separate entity table
			sql.Exec(
				"create table if not exists EmrTableDropdownInfoTextT( "
					"Data text not null unique"
				") "
			);

			//

#ifdef NXCACHE_EXTRA
			sql.Exec(
				"create table if not exists EmrImageStampsT( "
					  "ID integer not null primary key"
					", StampText text not null"
					", TypeName text not null"
					", Color integer not null"
					", Description text not null"
					", Inactive integer not null"
					", SmartStampTableSpawnRule integer not null"
					", ShowDot integer not null"
					", Image blob"
					", CategoryID integer"
				") "
			);
#endif

			//

			sql.Exec(
				"create table if not exists EmrTableDropdownStampFilterT( "
					  "EmrTableDropdownInfoID integer not null primary key"
					", Stamps blob"
				") "
			);

			//

			sql.Exec(
				"create table if not exists EmrTableDropdownStampDefaultsT( "
					  "EmrTableDropdownInfoID integer not null primary key"
					", Stamps blob"
				") "
			);

			//

			sql.Exec(
				"create table if not exists EmrProviderFloatTableDropdownT( "
					  "EmrTableDropdownGroupID integer not null"
					", ProviderID integer not null"
					", Count integer not null"
					", unique(EmrTableDropdownGroupID, ProviderID)"
				") "
			);

			//

			sql.Exec(
				"create table if not exists EmrInfoT( "
					  "ID integer not null primary key "
					", Name text not null "
					", EmrInfoMasterID integer not null "
					", DataType integer not null "
					", DataSubType integer not null "
					", LongForm text not null "
					", BackgroundImageFilePath text "
					", DataSeparator text "
					", DefaultText text "
					", DataSeparatorFinal text "
					", DataUnit text not null "
					", Object blob not null "
					", unique(EmrInfoMasterID, ID) "
				");"
			);

			//

#ifdef NXCACHE_EXTRA
#pragma TODO("PLID 60397 - AddHideTitleOnPreview to this if necessary")
			sql.Exec(
				"create table if not exists EmrTemplateT( "
					  "ID integer not null primary key "
					", CollectionID integer "
					", Name text "
					", AddOnce integer not null "
					", Deleted integer not null "
					", LastVersion integer "
					", CreatedDate real "
					", CreatedLogin text not null "
					", ModifiedDate real "
					", ModifiedLogin text not null "
					", EmnTabChartID integer "
					", EmnTabCategoryID integer "
					", VisitTypeID integer "
					", IsUniversal integer not null "
				");"
			);
#endif

			trans.Commit();
		}
	}

	///

	namespace Emr
	{		
		// (a.walling 2013-07-23 20:49) - PLID 57685 - Load all data for an individual EmrInfoT.ID
		void EnsureEmrInfo(long nID)
		{
			if (-1 == nID) {
				return;
			}

			sqlite::Connection& sql = GetConnection();

			{
				sqlite::Statement check(sql,
					"select null from EmrInfoT where ID = ?"
				);

				check.Reset().Bind() << nID;

				if (sqlite::Row row = check.Next()) {
					return;
				}
			}

			detail::LoadEmrInfo(nID);
		}
	}

	///

	void SqliteErrorLogCallback(void *pData, int rc, const char* sz)
	{
		LogDetail("sqlite: error %li - %s", rc, sz);
	}

	// (a.walling 2013-07-18 09:30) - PLID 57624 - Initialize sqlite and set options
	void InitSystem()
	{
		sqlite3_initialize();
		sqlite3_config(SQLITE_CONFIG_LOG, &SqliteErrorLogCallback, NULL);
		sqlite3_enable_shared_cache(1);
		sqlite3_config(SQLITE_CONFIG_MMAP_SIZE, 0x1000000LL, 0x10000000LL);
		sqlite3_config(SQLITE_CONFIG_URI, true);
	}

	void InitDatabase()
	{
		sqlite::Connection& sql = detail::GetCacheConnection().sql;

		detail::InitSchema(sql);
		detail::InitCache();
	}

	void Shutdown()
	{
		// (a.walling 2013-07-18 10:08) - PLID 57630 - Ensure cache is ready
		detail::WaitForCacheReady();
		detail::Shutdown();
	}

	// (a.walling 2013-07-18 10:04) - PLID 57629 - Update changed cached data
	void Checkpoint()
	{
		try {
			// (a.walling 2013-07-18 10:08) - PLID 57630 - Ensure cache is ready
			detail::WaitForCacheReady();
			detail::UpdateCache();
		} NxCatchAll(__FUNCTION__);
	}
	
	// (a.walling 2013-07-18 09:30) - PLID 57624 - Get thread connection
	sqlite::Connection& GetConnection()
	{
		// (a.walling 2013-07-18 10:08) - PLID 57630 - Ensure cache is ready
		detail::WaitForCacheReady();
		return detail::GetCacheConnection().sql;
	}
}

}
