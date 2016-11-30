// UnitedLinkPatient.h: interface for the CUnitedLinkPatient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNITEDLINKPATIENT_H__6EC5121F_9D5D_4781_88D5_20AA996D93DD__INCLUDED_)
#define AFX_UNITEDLINKPATIENT_H__6EC5121F_9D5D_4781_88D5_20AA996D93DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GenericLinkPatient.h"


// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - Need to specify namespace
// using namespace ADODB;
class CUnitedLinkPatient : public CGenericLinkPatient  
{
public:
	CUnitedLinkPatient(CWnd* pParent, ADODB::_ConnectionPtr& pConPractice, ADODB::_ConnectionPtr& pConRemote, CString strRemoteDataPath, CString strRemotePassword = "image_access");
	virtual ~CUnitedLinkPatient();

	// Returns non-zero on failure
	DWORD AddToRemoteDatabase(DWORD dwPracticeID, ADODB::_RecordsetPtr &prsPractice, DWORD& dwRemoteID);
	DWORD AddToLocalDatabase(DWORD dwRemoteID, ADODB::_RecordsetPtr &prsRemote, DWORD& dwPracticeID, DWORD& dwUserDefinedID);

	DWORD LinkExistingToRemote(DWORD dwPracticeID, DWORD dwRemoteID);
	DWORD LinkRemoteToExisting(DWORD dwRemoteID, DWORD dwPracticeID);

	DWORD UpdateRemote(DWORD dwPracticeID, ADODB::_RecordsetPtr &prsPractice);
	DWORD UpdateLocal(DWORD dwPracticeID, ADODB::_RecordsetPtr &prsRemote);

	DWORD Import(DWORD dwRemoteID, DWORD& dwPracticeID, DWORD& dwUserDefinedID, BOOL& bStop, EGenericLinkPersonAction& action);
};

#endif // !defined(AFX_UNITEDLINKPATIENT_H__6EC5121F_9D5D_4781_88D5_20AA996D93DD__INCLUDED_)
