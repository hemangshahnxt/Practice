// PPCLink.h: interface for the CPPCLink class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PPCLINK_H__824A2E06_6CB2_42F4_BF9B_4E75FFF6C1B4__INCLUDED_)
#define AFX_PPCLINK_H__824A2E06_6CB2_42F4_BF9B_4E75FFF6C1B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// (z.manning 2009-10-27 17:40) - PLID 36066 - I removed some of the unnecessary functions from here.

// PPCAddAppt will add an appointment to the PPC. If the
// appointment is already there, it will be modified.
HRESULT PPCAddAppt(long nResID);

// PPCModifyAppt will modify an appointment in the PPC. If
// the appointmnet is not there, an exception will be thrown.
HRESULT PPCModifyAppt(long nResID);

// PPCDeleteAppt will delete an appointment from the PPC.
HRESULT PPCDeleteAppt(long nResID);

// This function will modify an appointment in this computer's Outlook
// database only.
HRESULT PPCRefreshAppt(long nResID);

// This function will modify a set of appointments in this computer's Outlook
// database only.
HRESULT PPCRefreshAppts(long nPatientID);

//This function will flag a contact as modified, so it will be included in the next sync.
HRESULT PPCModifyContact(long nPersonID);

#endif // !defined(AFX_PPCLINK_H__824A2E06_6CB2_42F4_BF9B_4E75FFF6C1B4__INCLUDED_)
