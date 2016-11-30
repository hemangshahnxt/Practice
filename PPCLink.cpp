// PPCLink.cpp: implementation of the CPPCLink class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PPCLink.h"
//#include "nxlinksecurity.h"
//#include "nxoutlooklink.h"
#include "NxOutlookUtils.h"
#include <SharedScheduleUtils.h>
using namespace ADODB;

//#pragma warning( push )
//#pragma warning( disable : 4146 )
//#import "MSO9.DLL"
//#import "MSOUTL9.OLB"
//#pragma warning( pop )

// Pocket PC link thread messages (constrict to the scope of this
// source file only)
#define PPC_MSG_ENSURE_LINK				WM_USER + 1000
#define PPC_MSG_ADD_APPT				WM_USER + 1001
#define PPC_MSG_MODIFY_APPT				WM_USER + 1002
#define PPC_MSG_DELETE_APPT				WM_USER + 1003
#define PPC_MSG_REFRESH_APPT			WM_USER + 1004
#define PPC_MSG_REFRESH_MULTI_APPT		WM_USER + 1005
#define PPC_MSG_SYNCHRONIZE				WM_USER + 1006


extern const CString &GetNetworkLibraryString();
extern CString GetPassword();
extern long GetConnectionTimeout();
extern long GetCommandTimeout();

static long g_nProfileID = -1;
static DWORD g_dwPPCThreadID = 0;
static HANDLE g_hPPCThread = NULL;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// (z.manning 2009-10-27 17:40) - PLID 36066 - I removed some of the unnecessary functions from here.

HRESULT PPCAddAppt(long nResID)
{
	return PPCModifyAppt(nResID);
}

HRESULT PPCModifyAppt(long nResID)
{
	// (z.manning 2009-10-27 17:55) - PLID 36066 - Also check NexSync license count
	if(g_pLicense->GetPPCCountAllowed() == 0 && g_pLicense->GetNexSyncCountAllowed() == 0) {
		return S_FALSE;
	}

	// (z.manning 2010-07-14 16:04) - PLID 39422 - Moved to shared library
	SetAppointmentToSync(GetRemoteData(), nResID);
	
	return S_OK;
}

HRESULT PPCDeleteAppt(long nResID)
{
	return PPCModifyAppt(nResID);
}

HRESULT PPCRefreshAppts(long nPatientID)
{
	// (c.haag 2005-08-09 09:13) - The purpose of this function is to flag all appointments
	// which belong to a patient as modified because the patient's demographics, which appear
	// on the appointment, have been changed.

	// (c.haag 2006-04-12 11:17) - PLID 20094 - New and improved!
	//
	// (z.manning 2009-10-27 17:55) - PLID 36066 - Also check NexSync license count
	if(g_pLicense->GetPPCCountAllowed() == 0 && g_pLicense->GetNexSyncCountAllowed() == 0) {
		return S_FALSE;
	}

	// (z.manning 2010-07-14 16:04) - PLID 39422 - Moved to shared library
	SetAppointmentsToSyncByPatient(GetRemoteData(), nPatientID);

	return S_OK;
}

HRESULT PPCRefreshAppt(long nResID)
{
	// (c.haag 2005-08-09 09:19) - This is just like PPCRefreshAppts, but for one appointment
	return PPCModifyAppt(nResID);
}

HRESULT PPCModifyContact(long nPersonID)
{
	if (!g_pLicense->GetPPCCountAllowed())
		return S_FALSE;

	SetContactToSync(GetRemoteData(), nPersonID);
	
	return S_OK;
}