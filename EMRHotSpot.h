// EMRHotSpot.h: interface for the CEMRHotSpot class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMRHOTSPOT_H__C3530749_C234_4317_B408_D0E73D3E0000__INCLUDED_)
#define AFX_EMRHOTSPOT_H__C3530749_C234_4317_B408_D0E73D3E0000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//DRT 1/18/2008 - PLID 28602 - This class is derived from CHotSpot and contains some extra stuff
//	that can't be shared to other projects.
#include "HotSpot.h"
#include "EMRUtils.h"

enum AnatomySide;

// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray

class CEMRHotSpot : public CHotSpot
{
public:
	CEMRHotSpot();
	//DRT 1/21/2008 - PLID 28603
	CEMRHotSpot(CEMRHotSpot &sSource);
	virtual ~CEMRHotSpot();
	//DRT 1/21/2008 - PLID 28603
	void operator =(CEMRHotSpot &sSource);

	//Load information for actions
	void AddOriginalAction(EmrAction ea);

	//TES 2/9/2010 - PLID 37223 - Accessors for Anatomic Location, Qualifier, and Side
	//TES 2/10/2010 - PLID 37296 - Added names, for auditing.
	void SetOriginalAnatomicLocation(long nAnatomicLocationID, const CString &strAnatomicLocation);
	void SetAnatomicLocation(long nAnatomicLocationID, const CString &strAnatomicLocation);
	void SetOriginalAnatomicQualifier(long nAnatomicQualifierID, const CString &strAnatomicQualifier);
	void SetAnatomicQualifier(long nAnatomicQualifierID, const CString &strAnatomicQualifier);
	long GetOriginalAnatomicLocationID();
	long GetAnatomicLocationID();
	long GetOriginalAnatomicQualifierID();
	long GetAnatomicQualifierID();
	void SetOriginalSide(AnatomySide asSide);
	void SetSide(AnatomySide asSide);
	AnatomySide GetOriginalSide();
	AnatomySide GetSide();

	CString GetOriginalAnatomicLocationName();
	CString GetAnatomicLocationName();
	CString GetOriginalAnatomicQualifierName();
	CString GetAnatomicQualifierName();

	// (z.manning 2010-03-10 09:58) - PLID 37225 - Returns the full anatomic location text
	CString GetFullAnatomicLocation();
	
	//Get pointers to the action arrays
	MFCArray<EmrAction>* GetCurrentActionArray();
	MFCArray<EmrAction>* GetOriginalActionArray();

	// (z.manning 2010-03-16 12:37) - PLID 37493
	BOOL DoAnatomicLocationsMatch(CEMRHotSpot *pHotSpotToCompare);

	// (z.manning 2011-07-22 16:06) - PLID 44676
	short Get3DHotSpotID();
	void Set3DHotSpotID(const short n3DHotSpotID);

	BOOL Is3DHotSpot(); // (z.manning 2011-09-27 14:13) - PLID 44676

private:
	//Actions.  Keep the loaded copy & the current copy so we can find out what changed
	MFCArray<EmrAction> m_aryOriginalActions;
	MFCArray<EmrAction> m_aryCurrentActions;

	//TES 2/9/2010 - PLID 37223 - Added Anatomic Location, Qualifier, and Side
	long m_nAnatomicLocationID, m_nOriginalAnatomicLocationID;
	long m_nAnatomicQualifierID,  m_nOriginalAnatomicQualifierID;
	AnatomySide m_Side, m_OriginalSide;

	//TES 2/10/2010 - PLID 37296 - Added names, for auditing.
	CString m_strOriginalAnatomicLocation, m_strAnatomicLocation;
	CString m_strOriginalAnatomicQualifier, m_strAnatomicQualifier;

	// (z.manning 2011-07-22 16:03) - PLID 44676 - Used to keep track of the hot spot ID specific to hot spots
	// on the Nx3D control;
	short m_n3DHotSpotID;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// (z.manning 2011-07-22 17:03) - PLID 44676 - Created a class for EMR hot spot arrays
class CEMRHotSpotArray : public CArray<CEMRHotSpot*,CEMRHotSpot*>
{
public:
	BOOL Contains(CEMRHotSpot* pHotSpot);

	CEMRHotSpot* FindBy3DHotSpotID(const short n3DHotSpotID);

	void CopyFromArray(CEMRHotSpotArray &arySource);

	BOOL Is3DHotSpotArray();

	void Clear();
	void ClearByIndex(const int nIndex);
};

#endif // !defined(AFX_EMRHOTSPOT_H__C3530749_C234_4317_B408_D0E73D3E0000__INCLUDED_)
