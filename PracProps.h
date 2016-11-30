class CDaoException;

#ifndef Practice_Properties_h
#define Practice_Properties_h

#pragma once



#include "GlobalUtils.h"
#include "MiscSystemUtils.h"
#include "NxAdo.h"


// (a.walling 2011-09-07 18:01) - PLID 45448 - NxAdo unification - Practice changes


// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Snapshot and thread stuff now in NxAdo///////////////////////////////ADO///////////////////////////////////



///////////////////////////////
// For working with the Connection itself

const CString &GetSqlServerName();

inline CString GetPassword() {
	return GetSecretDatabasePassword();
}
const CString &GetNetworkLibraryString();
long GetConnectionTimeout();
long GetCommandTimeout();

// (a.walling 2010-06-02 07:49) - PLID 31316 - Get (and check) the data provider string (eg Provider=SQLOLEDB)
const CString &GetDataProviderString();

// (a.walling 2010-06-02 07:49) - PLID 31316 - Standard database connection string
// will use PracData[_subkey] if no override database passed in.
// (a.walling 2010-07-28 13:08) - PLID 39871 - Suffix to be appended to the application name
// (a.walling 2010-07-28 13:06) - PLID 39871 - We now use the application name (and the application name suffix passed in)
// in order to explicitly differentiate connections for connection pooling purposes.
CString GetStandardConnectionString(bool bIncludePassword = true, const CString& strDatabase = "", const CString& strApplicationNameSuffix = "");

bool EnsureSqlServer(BOOL& bRunTroubleshooter);
BOOL EnsureNxServer(BOOL& bRunTroubleshooter);
BOOL EnsureLicenseServer(BOOL& bRunTroubleshooter);

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - lots of stuff moved to NxAdo
// eg ConnectionHealth handling the EnsureRemoteData etc and NxAdo handling remote server time

////////////////////////////////

COleVariant GetProperty(LPCTSTR strPropName, PropertyType nPropType, COleVariant *varDefault = 0, int nPropNum = 0, bool bAutoCreate = true);
long SetProperty(LPCTSTR strPropName, PropertyType nPropType, COleVariant varNewValue, int nPropNum = 0);

//Used by the Array functions below
void ParseCommaDeliminatedText(CArray<int,int> &ary, const char *p);
CString GetCommaDeliminatedText(CArray<int,int> &ary);
void ParseCommaDeliminatedText(CStringArray &ary, const CString &strText);
CString GetCommaDeliminatedText(CStringArray &ary);
// (j.jones 2011-02-11 12:29) - PLID 35180 - I added CDWord array versions as well
void ParseCommaDeliminatedText(CDWordArray &ary, const char *p);
CString GetCommaDeliminatedText(CDWordArray &ary);
// (b.spivey, January 7th, 2015) PLID 64397 - For longs. 
void ParseCommaDeliminatedText(CArray<long, long> &ary, const char *p);
CString GetCommaDeliminatedText(CArray<long, long> &ary);

// Get properties
void GetRemotePropertyArray(LPCSTR strPropName, CArray<int,int> &ary, int nPropNum  = 0 , LPCTSTR strUsername  = NULL );
void SetRemotePropertyArray(LPCSTR strPropName, CArray<int,int> &ary, int nPropNum  = 0 , LPCTSTR strUsername  = NULL );
void GetRemotePropertyCStringArrayMemo(LPCSTR strPropName, CStringArray &ary, int nPropNum  = 0 , LPCTSTR strUsername  = NULL );
void SetRemotePropertyCStringArrayMemo(LPCSTR strPropName, CStringArray &ary, int nPropNum  = 0 , LPCTSTR strUsername  = NULL );
CString GetPropertyText(LPCTSTR strPropName, LPCTSTR strDefault = 0, int nPropNum = 0, bool bAutoCreate = true);
CString GetPropertyMemo(LPCTSTR strPropName, LPCTSTR strDefault = 0, int nPropNum = 0, bool bAutoCreate = true);
long GetPropertyInt(LPCTSTR strPropName, long nDefault = LONG_MIN + 1, int nPropNum = 0, bool bAutoCreate = true);
float GetPropertyFloat(LPCTSTR strPropName, float *pfltDefault = 0, int nPropNum = 0, bool bAutoCreate = true);
COleDateTime GetPropertyDateTime(LPCTSTR strPropName, COleDateTime *pdtDefault = 0, int nPropNum = 0, bool bAutoCreate = true);

// Set properties
long SetPropertyText(LPCTSTR strPropName, LPCTSTR strNewValue, int nPropNum = 0);
long SetPropertyMemo(LPCTSTR strPropName, LPCTSTR strNewValue, int nPropNum = 0);
long SetPropertyInt(LPCTSTR strPropName, long nNewValue, int nPropNum = 0);
long SetPropertyFloat(LPCTSTR strPropName, float fltNewValue, int nPropNum = 0);
long SetPropertyDateTime(LPCTSTR strPropName, COleDateTime dtNewValue, int nPropNum = 0);

// Remote properties
COleVariant GetRemoteProperty(LPCTSTR strPropName, PropertyType nPropType, COleVariant *varDefault = 0, int nPropNum = 0, LPCTSTR strUsername = NULL, bool bAutoCreate = true);
long SetRemoteProperty(LPCTSTR strPropName, PropertyType nPropType, COleVariant varNewValue, int nPropNum = 0, LPCTSTR strUsername = NULL);

// Get properties
CString GetRemotePropertyText(LPCTSTR strPropName, LPCTSTR strDefault = 0, int nPropNum = 0, LPCTSTR strUsername = NULL, bool bAutoCreate = true);
//TES 10/15/2008 - PLID 31646 - Need an overload that takes a connection pointer.
CString GetRemotePropertyText(ADODB::_Connection* lpCon, LPCTSTR strPropName, LPCTSTR strDefault = 0, int nPropNum = 0, LPCTSTR strUsername = NULL, bool bAutoCreate = true);
CString GetRemotePropertyMemo(LPCTSTR strPropName, LPCTSTR strDefault = 0, int nPropNum = 0, LPCTSTR strUsername = NULL, bool bAutoCreate = true);
long GetRemotePropertyInt(LPCTSTR strPropName, long nDefault = LONG_MIN + 1, int nPropNum = 0, LPCTSTR strUsername = NULL, bool bAutoCreate = true);
//TES 10/15/2008 - PLID 31646 - Need an overload that takes a connection pointer.
long GetRemotePropertyInt(ADODB::_Connection* lpCon, LPCTSTR strPropName, long nDefault = LONG_MIN + 1, int nPropNum = 0, LPCTSTR strUsername = NULL, bool bAutoCreate = true);
float GetRemotePropertyFloat(LPCTSTR strPropName, float *pfltDefault = 0, int nPropNum = 0, LPCTSTR strUsername = NULL, bool bAutoCreate = true);
COleDateTime GetRemotePropertyDateTime(LPCTSTR strPropName, const COleDateTime *pdtDefault = 0, int nPropNum = 0, LPCTSTR strUsername = NULL, bool bAutoCreate = true);
_variant_t GetRemotePropertyImage(LPCTSTR strPropName, int nPropNum, LPCTSTR strUsername, bool bAutoCreate);

// Set properties
long SetRemotePropertyText(LPCTSTR strPropName, LPCTSTR strNewValue, int nPropNum = 0, LPCTSTR strUsername = NULL);
long SetRemotePropertyMemo(LPCTSTR strPropName, LPCTSTR strNewValue, int nPropNum = 0, LPCTSTR strUsername = NULL);
long SetRemotePropertyInt(LPCTSTR strPropName, long nNewValue, int nPropNum = 0, LPCTSTR strUsername = NULL);
long SetRemotePropertyFloat(LPCTSTR strPropName, float fltNewValue, int nPropNum = 0, LPCTSTR strUsername = NULL);
long SetRemotePropertyDateTime(LPCTSTR strPropName, COleDateTime dtNewValue, int nPropNum = 0, LPCTSTR strUsername = NULL);
long SetRemotePropertyImage(LPCTSTR strPropName, _variant_t varNewValue, int nPropNum, LPCTSTR strUsername);

// Cache functions
void InitializeRemotePropertyCache(CWnd* pWndTimer);
void DestroyRemotePropertyCache();
void FlushRemotePropertyCache();

#endif
