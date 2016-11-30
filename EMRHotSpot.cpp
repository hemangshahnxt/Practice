// EMRHotSpot.cpp: implementation of the CEMRHotSpot class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EMRHotSpot.h"
#include "GlobalLabUtils.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//DRT 1/18/2008 - PLID 28602 - Created

CEMRHotSpot::CEMRHotSpot()
{
	m_nOriginalAnatomicLocationID = -1;
	m_nAnatomicLocationID = -1;
	m_nOriginalAnatomicQualifierID = -1;
	m_nAnatomicQualifierID = -1;
	m_OriginalSide = asNone;
	m_Side = asNone;
	m_n3DHotSpotID = -1;
}

CEMRHotSpot::~CEMRHotSpot()
{

}

//DRT 1/21/2008 - PLID 28603 - Copy constructor
CEMRHotSpot::CEMRHotSpot(CEMRHotSpot &sSource)
{
	*this = sSource;
}

//DRT 1/21/2008 - PLID 28603 - = operator
void CEMRHotSpot::operator =(CEMRHotSpot &sSource)
{
	CHotSpot::operator =(sSource);

	int i = 0;
	m_aryOriginalActions.RemoveAll();
	for(i = 0; i < sSource.m_aryOriginalActions.GetSize(); i++) {
		m_aryOriginalActions.Add( sSource.m_aryOriginalActions.GetAt(i) );
	}

	m_aryCurrentActions.RemoveAll();
	for(i = 0; i < sSource.m_aryCurrentActions.GetSize(); i++) {
		m_aryCurrentActions.Add( sSource.m_aryCurrentActions.GetAt(i) );
	}

	//TES 2/9/2010 - PLID 37223 - Added Anatomic Location, Qualifier, and Side
	m_nAnatomicLocationID = sSource.m_nAnatomicLocationID;
	m_nOriginalAnatomicLocationID = sSource.m_nOriginalAnatomicLocationID;
	m_nAnatomicQualifierID = sSource.m_nAnatomicQualifierID;
	m_nOriginalAnatomicQualifierID = sSource.m_nOriginalAnatomicQualifierID;
	m_OriginalSide = sSource.m_OriginalSide;
	m_Side = sSource.m_Side;
	//TES 2/16/2010 - PLID 37223 - Forgot to include the names.
	m_strAnatomicLocation = sSource.m_strAnatomicLocation;
	m_strOriginalAnatomicLocation = sSource.m_strAnatomicLocation;
	m_strAnatomicQualifier = sSource.m_strAnatomicQualifier;
	m_strOriginalAnatomicQualifier = sSource.m_strOriginalAnatomicQualifier;

	m_n3DHotSpotID = sSource.m_n3DHotSpotID; // (z.manning 2011-07-22 16:05) - PLID 44676
}


//Original actions go in both arrays to start.
void CEMRHotSpot::AddOriginalAction(EmrAction ea)
{
	m_aryOriginalActions.Add(ea);
	m_aryCurrentActions.Add(ea);
}

// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray

MFCArray<EmrAction>* CEMRHotSpot::GetCurrentActionArray()
{
	return &m_aryCurrentActions;
}

MFCArray<EmrAction>* CEMRHotSpot::GetOriginalActionArray()
{
	return &m_aryOriginalActions;
}

//TES 2/9/2010 - PLID 37223 - Accessors for Anatomic Location, Qualifier, and Side
//TES 2/10/2010 - PLID 37296 - Added names, for auditing.
void CEMRHotSpot::SetOriginalAnatomicLocation(long nAnatomicLocationID, const CString &strAnatomicLocation)
{
	m_nOriginalAnatomicLocationID = nAnatomicLocationID;
	m_nAnatomicLocationID = nAnatomicLocationID;

	// (a.walling 2014-07-02 11:53) - PLID 62696 - Always use blank if -1
	m_strAnatomicLocation = (nAnatomicLocationID != -1) ? strAnatomicLocation : "";
	m_strOriginalAnatomicLocation = m_strAnatomicLocation;
}

//TES 2/9/2010 - PLID 37223 - Accessors for Anatomic Location, Qualifier, and Side
//TES 2/10/2010 - PLID 37296 - Added names, for auditing.
void CEMRHotSpot::SetAnatomicLocation(long nAnatomicLocationID, const CString &strAnatomicLocation)
{
	m_nAnatomicLocationID = nAnatomicLocationID;
	// (a.walling 2014-07-02 11:53) - PLID 62696 - Always use blank if -1
	m_strAnatomicLocation = (nAnatomicLocationID != -1) ? strAnatomicLocation : "";
}

//TES 2/9/2010 - PLID 37223 - Accessors for Anatomic Location, Qualifier, and Side
//TES 2/10/2010 - PLID 37296 - Added names, for auditing.
void CEMRHotSpot::SetOriginalAnatomicQualifier(long nAnatomicQualifierID, const CString &strAnatomicQualifier)
{
	m_nOriginalAnatomicQualifierID = nAnatomicQualifierID;
	m_nAnatomicQualifierID = nAnatomicQualifierID;

	// (a.walling 2014-07-02 11:53) - PLID 62696 - Always use blank if -1
	m_strAnatomicQualifier = (nAnatomicQualifierID != -1) ? strAnatomicQualifier : "";
	m_strOriginalAnatomicQualifier = m_strAnatomicQualifier;
}

//TES 2/9/2010 - PLID 37223 - Accessors for Anatomic Location, Qualifier, and Side
//TES 2/10/2010 - PLID 37296 - Added names, for auditing.
void CEMRHotSpot::SetAnatomicQualifier(long nAnatomicQualifierID, const CString &strAnatomicQualifier)
{
	m_nAnatomicQualifierID = nAnatomicQualifierID;
	// (a.walling 2014-07-02 11:53) - PLID 62696 - Always use blank if -1
	m_strAnatomicQualifier = (nAnatomicQualifierID != -1) ? strAnatomicQualifier : "";
}

//TES 2/9/2010 - PLID 37223 - Accessors for Anatomic Location, Qualifier, and Side
long CEMRHotSpot::GetOriginalAnatomicLocationID()
{
	return m_nOriginalAnatomicLocationID;
}

//TES 2/9/2010 - PLID 37223 - Accessors for Anatomic Location, Qualifier, and Side
long CEMRHotSpot::GetAnatomicLocationID()
{
	return m_nAnatomicLocationID;
}

//TES 2/9/2010 - PLID 37223 - Accessors for Anatomic Location, Qualifier, and Side
long CEMRHotSpot::GetOriginalAnatomicQualifierID()
{
	return m_nOriginalAnatomicQualifierID;
}

//TES 2/9/2010 - PLID 37223 - Accessors for Anatomic Location, Qualifier, and Side
long CEMRHotSpot::GetAnatomicQualifierID()
{
	return m_nAnatomicQualifierID;
}

//TES 2/9/2010 - PLID 37223 - Accessors for Anatomic Location, Qualifier, and Side
void CEMRHotSpot::SetOriginalSide(AnatomySide asSide)
{
	m_OriginalSide = asSide;
	m_Side = asSide;
}

//TES 2/9/2010 - PLID 37223 - Accessors for Anatomic Location, Qualifier, and Side
void CEMRHotSpot::SetSide(AnatomySide asSide)
{
	m_Side = asSide;
}

//TES 2/9/2010 - PLID 37223 - Accessors for Anatomic Location, Qualifier, and Side
AnatomySide CEMRHotSpot::GetOriginalSide()
{
	return m_OriginalSide;
}

//TES 2/9/2010 - PLID 37223 - Accessors for Anatomic Location, Qualifier, and Side
AnatomySide CEMRHotSpot::GetSide()
{
	return m_Side;
}

//TES 2/10/2010 - PLID 37296 - Added names, for auditing.
CString CEMRHotSpot::GetOriginalAnatomicLocationName()
{
	return m_strOriginalAnatomicLocation;
}

//TES 2/10/2010 - PLID 37296 - Added names, for auditing.
CString CEMRHotSpot::GetAnatomicLocationName()
{
	return m_strAnatomicLocation;
}

//TES 2/10/2010 - PLID 37296 - Added names, for auditing.
CString CEMRHotSpot::GetOriginalAnatomicQualifierName()
{
	return m_strOriginalAnatomicQualifier;
}

//TES 2/10/2010 - PLID 37296 - Added names, for auditing.
CString CEMRHotSpot::GetAnatomicQualifierName()
{
	return m_strAnatomicQualifier;
}

// (z.manning 2010-03-10 09:58) - PLID 37225 - Returns the full anatomic location text
CString CEMRHotSpot::GetFullAnatomicLocation()
{
	return ::FormatAnatomicLocation(m_strAnatomicLocation, m_strAnatomicQualifier, m_Side);
}

// (z.manning 2010-03-16 12:37) - PLID 37493
BOOL CEMRHotSpot::DoAnatomicLocationsMatch(CEMRHotSpot *pHotSpotToCompare)
{
	return (
		this->m_nAnatomicLocationID == pHotSpotToCompare->m_nAnatomicLocationID &&
		this->m_nAnatomicQualifierID == pHotSpotToCompare->m_nAnatomicQualifierID &&
		this->m_Side == pHotSpotToCompare->m_Side
		);
}

// (z.manning 2011-07-22 16:06) - PLID 44676
short CEMRHotSpot::Get3DHotSpotID()
{
	return m_n3DHotSpotID;
}

// (z.manning 2011-07-22 16:06) - PLID 44676
void CEMRHotSpot::Set3DHotSpotID(const short n3DHotSpotID)
{
	m_n3DHotSpotID = n3DHotSpotID;
}

// (z.manning 2011-09-27 14:14) - PLID 44676
BOOL CEMRHotSpot::Is3DHotSpot()
{
	if(m_n3DHotSpotID == -1) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CEMRHotSpotArray::Contains(CEMRHotSpot *pHotSpot)
{
	for(int nHotSpotIndex = 0; nHotSpotIndex < GetCount(); nHotSpotIndex++)
	{
		if(pHotSpot == GetAt(nHotSpotIndex)) {
			return TRUE;
		}
	}

	return FALSE;
}

void CEMRHotSpotArray::Clear()
{
	for(int nHotSpotIndex = GetCount() - 1; nHotSpotIndex >= 0; nHotSpotIndex--) {
		ClearByIndex(nHotSpotIndex);
	}
}

void CEMRHotSpotArray::ClearByIndex(const int nIndex)
{
	delete GetAt(nIndex);
	RemoveAt(nIndex);
}

CEMRHotSpot* CEMRHotSpotArray::FindBy3DHotSpotID(const short n3DHotSpotID)
{
	for(int nHotSpotIndex = 0; nHotSpotIndex < GetCount(); nHotSpotIndex++)
	{
		CEMRHotSpot *pHotSpot = GetAt(nHotSpotIndex);
		if(pHotSpot->Get3DHotSpotID() == n3DHotSpotID) {
			return pHotSpot;
		}
	}

	return NULL;
}

void CEMRHotSpotArray::CopyFromArray(CEMRHotSpotArray &arySource)
{
	this->Clear();
	for(int nHotSpotIndex = 0; nHotSpotIndex < arySource.GetCount(); nHotSpotIndex++)
	{
		CEMRHotSpot *pSourceHotSpot = arySource.GetAt(nHotSpotIndex);
		CEMRHotSpot *pNewHotSpot = new CEMRHotSpot;
		*pNewHotSpot = *pSourceHotSpot;
		Add(pNewHotSpot);
	}
}

BOOL CEMRHotSpotArray::Is3DHotSpotArray()
{
	if(GetCount() > 0 && GetAt(0)->Is3DHotSpot()) {
		return TRUE;
	}

	return FALSE;
}