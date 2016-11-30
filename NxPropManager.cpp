#include "stdafx.h"
#include "NxPropManager.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (c.haag 2015-10-15) - PLID 67358 - Now inherits from a class in the Practice shared library

// Returns the property value in the form of a variant
COleVariant CNxPropManager::GetProperty(LPCTSTR strPropName, PropertyType nPropType, COleVariant *varDefault /* = 0 */, int nPropNum /* = 0 */, bool bAutoCreate /*= true*/)
{
	// Since we used to pull this from the ConfigT on the local machine, 
	// we now pull it from the ConfigRT giving our machine name as the user
	// (j.armen 2011-10-24 14:18) - PLID 46139 - GetPracPath is using ConfigRT
	CString strUserParam = GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT);
	return GetRemoteProperty(strPropName, nPropType, varDefault, nPropNum, strUserParam, bAutoCreate);
}

// Sets the property value in the form of a variant
// A return value of zero indicates success, otherwise an error code
long CNxPropManager::SetProperty(LPCTSTR strPropName, PropertyType nPropType, COleVariant varNewValue, int nPropNum /* = 0 */)
{
	// Since we used to pull this from the ConfigT on the local machine, 
	// we now pull it from the ConfigRT giving our machine name as the user
	// (j.armen 2011-10-24 14:19) - PLID 46139 - GetPracPath is using ConfigRT
	CString strUserParam = GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT);
	return SetRemoteProperty(strPropName, nPropType, varNewValue, nPropNum, strUserParam);
}

CString CNxPropManager::GetPropertyText(LPCTSTR strPropName, LPCTSTR strDefault /* = 0 */, int nPropNum /* = 0 */, bool bAutoCreate /*= true*/)
{
	COleVariant tmpVar;
	
	// Pass pointer to default variant only if a default value was passed
	if (strDefault) {
		COleVariant varDefault(strDefault);
		tmpVar = GetProperty(strPropName, propText, &varDefault, nPropNum, bAutoCreate);
	} else {
		tmpVar = GetProperty(strPropName, propText, 0, nPropNum, bAutoCreate);
	}
	
	// If null was returned, return a null-like value, otherwise, return the value
	if (tmpVar.vt == VT_NULL || tmpVar.vt == VT_EMPTY) {
		return CString("");
	} else {
		return (LPCTSTR)_bstr_t(tmpVar);
	}
}

CString CNxPropManager::GetPropertyMemo(LPCTSTR strPropName, LPCTSTR strDefault /* = 0 */, int nPropNum /* = 0 */, bool bAutoCreate /*= true*/)
{
	COleVariant tmpVar;
	
	// Pass pointer to default variant only if a default value was passed
	if (strDefault) {
		COleVariant varDefault(strDefault);
		tmpVar = GetProperty(strPropName, propMemo, &varDefault, nPropNum, bAutoCreate);
	} else {
		tmpVar = GetProperty(strPropName, propMemo, 0, nPropNum, bAutoCreate);
	}
	
	// If null was returned, return a null-like value, otherwise, return the value
	if (tmpVar.vt == VT_NULL || tmpVar.vt == VT_EMPTY) {
		return CString("");
	}
	else {
		return (LPCTSTR)_bstr_t(tmpVar);
	}
}

long CNxPropManager::GetPropertyInt(LPCTSTR strPropName, long nDefault /* = LONG_MIN + 1 */, int nPropNum /* = 0 */, bool bAutoCreate /*= true*/)
{
	COleVariant tmpVar;
	
	// Pass pointer to default variant only if a default value was passed
	if (nDefault != LONG_MIN + 1) {
		COleVariant varDefault(nDefault, VT_I4);
		tmpVar = GetProperty(strPropName, propNumber, &varDefault, nPropNum, bAutoCreate);
	} else {
		tmpVar = GetProperty(strPropName, propNumber, 0, nPropNum, bAutoCreate);
	}
	
	// If null was returned, return a null-like value, otherwise, return the value
	if (tmpVar.vt == VT_NULL || tmpVar.vt == VT_EMPTY) {
		return 0L;
	}
	else {
		return tmpVar.lVal;
	}
}

float CNxPropManager::GetPropertyFloat(LPCTSTR strPropName, float *pfltDefault /* = 0 */, int nPropNum /* = 0 */, bool bAutoCreate /*= true*/)
{
	COleVariant tmpVar;
	
	// Pass pointer to default variant only if a default value was passed
	if (pfltDefault) {
		COleVariant varDefault(*pfltDefault);
		tmpVar = GetProperty(strPropName, propFloat, &varDefault, nPropNum, bAutoCreate);
	} else {
		tmpVar = GetProperty(strPropName, propFloat, 0, nPropNum, bAutoCreate);
	}
	
	// If null was returned, return a null-like value, otherwise, return the value
	if (tmpVar.vt == VT_NULL || tmpVar.vt == VT_EMPTY) {
		return 0.0;
	}
	else {
		return tmpVar.fltVal;
	}
}

COleDateTime CNxPropManager::GetPropertyDateTime(LPCTSTR strPropName, COleDateTime *pdtDefault /* = 0 */, int nPropNum /* = 0 */, bool bAutoCreate /*= true*/)
{
	COleVariant tmpVar;
	
	// Pass pointer to default variant only if a default value was passed
	if (pdtDefault) {
		COleVariant varDefault(*pdtDefault);
		tmpVar = GetProperty(strPropName, propDateTime, &varDefault, nPropNum, bAutoCreate);
	} else {
		tmpVar = GetProperty(strPropName, propDateTime, 0, nPropNum, bAutoCreate);
	}
	
	// If null was returned, return a null-like value, otherwise, return the value
	COleDateTime Ans;
	if (tmpVar.vt == VT_NULL || tmpVar.vt == VT_EMPTY) {
		Ans.SetStatus(COleDateTime::invalid);
	}
	else {
		Ans = tmpVar.date;
	}
	return Ans;
}

long CNxPropManager::SetPropertyText(LPCTSTR strPropName, LPCTSTR strNewValue, int nPropNum /* = 0 */)
{
	return SetProperty(strPropName, propText, COleVariant(strNewValue, VT_BSTR), nPropNum);
}

long CNxPropManager::SetPropertyMemo(LPCTSTR strPropName, LPCTSTR strNewValue, int nPropNum /* = 0 */)
{
	return SetProperty(strPropName, propMemo, COleVariant(strNewValue, VT_BSTR), nPropNum);
}

long CNxPropManager::SetPropertyInt(LPCTSTR strPropName, long nNewValue, int nPropNum /* = 0 */)
{
	return SetProperty(strPropName, propNumber, COleVariant(nNewValue, VT_I4), nPropNum);
}

long CNxPropManager::SetPropertyFloat(LPCTSTR strPropName, float fltNewValue, int nPropNum /* = 0 */)
{
	return SetProperty(strPropName, propFloat, COleVariant(fltNewValue), nPropNum);
}

long CNxPropManager::SetPropertyDateTime(LPCTSTR strPropName, COleDateTime dtNewValue, int nPropNum /* = 0 */)
{
	return SetProperty(strPropName, propDateTime, COleVariant(dtNewValue), nPropNum);
}
