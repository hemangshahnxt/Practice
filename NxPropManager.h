#pragma once

#include "SharedPropertyUtils.h"

// (c.haag 2004-11-26 12:42) - CNxPropManager is a wrapper class for getting and
// setting properties in ConfigRT. It uses a "memory table" and communication
// with NxServer to effectively cache data for super fast property acquisitions.
// The logic is as follows:
//
// Getting remote properties: First we see if the property is in our cache.
// If it is not, then we get it from data, then add it to our cache, then return
// it to the user. If it is in our cache already, we just get it from there and
// return it to the user. Our cache is updated any time we get ConfigRT table
// checker messages, assuming the calling program uses OnConfigRTPacket.
// In the event caching is disabled, we always do it from the data.
//
// Setting remote properties: We always write properties to both data and cache.
//
// (c.haag 2015-10-15) - PLID 67358 - Now inherits from a class in the Practice shared library

class CNxPropManager : public CSharedPropManager
{
public:
	// Get properties
	COleVariant GetProperty(LPCTSTR strPropName, PropertyType nPropType, COleVariant *varDefault = 0, int nPropNum = 0, bool bAutoCreate = true);
	CString GetPropertyText(LPCTSTR strPropName, LPCTSTR strDefault = 0, int nPropNum = 0, bool bAutoCreate = true);
	CString GetPropertyMemo(LPCTSTR strPropName, LPCTSTR strDefault = 0, int nPropNum = 0, bool bAutoCreate = true);
	long GetPropertyInt(LPCTSTR strPropName, long nDefault = LONG_MIN + 1, int nPropNum = 0, bool bAutoCreate = true);
	float GetPropertyFloat(LPCTSTR strPropName, float *pfltDefault = 0, int nPropNum = 0, bool bAutoCreate = true);
	COleDateTime GetPropertyDateTime(LPCTSTR strPropName, COleDateTime *pdtDefault = 0, int nPropNum = 0, bool bAutoCreate = true);

	// Set properties
	long SetProperty(LPCTSTR strPropName, PropertyType nPropType, COleVariant varNewValue, int nPropNum = 0);
	long SetPropertyText(LPCTSTR strPropName, LPCTSTR strNewValue, int nPropNum = 0);
	long SetPropertyMemo(LPCTSTR strPropName, LPCTSTR strNewValue, int nPropNum = 0);
	long SetPropertyInt(LPCTSTR strPropName, long nNewValue, int nPropNum = 0);
	long SetPropertyFloat(LPCTSTR strPropName, float fltNewValue, int nPropNum = 0);
	long SetPropertyDateTime(LPCTSTR strPropName, COleDateTime dtNewValue, int nPropNum = 0);
};
