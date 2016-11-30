// (c.haag 2010-02-04 13:36) - PLID 37221 - Wrapper class for explicitly accessing SingleDay Reservation objects

#include "stdafx.h"
#include "Reservation.h"

// (c.haag 2010-04-30 11:22) - PLID 38379 - Track all instantiated IReservationPtr objects
class CInstantiatedResPtr
{
public:
	CReservation* m_pRes;
};
CArray<CInstantiatedResPtr*,CInstantiatedResPtr*> g_apInstantiatedResPtrs;

// (c.haag 2010-08-14 14:29) - PLID 40086 - Access to the global array is now thread-safe
CCriticalSection l_csInstantiatedResPtrs;

#pragma region Reservation Objects

	// (c.haag 2010-04-30 12:03) - PLID 38379 - One-time initialization. Add this reservation to the global array.
	void CReservation::Init()
	{
		CSingleLock cs(&l_csInstantiatedResPtrs, TRUE); // (c.haag 2010-08-14 14:29) - PLID 40086
		CInstantiatedResPtr* p = new CInstantiatedResPtr;
		p->m_pRes = this;
		g_apInstantiatedResPtrs.Add(p);
	}

	// Destruction. Remove this reservation from the global array.
	CReservation::~CReservation()
	{
		CSingleLock cs(&l_csInstantiatedResPtrs, TRUE); // (c.haag 2010-08-14 14:29) - PLID 40086
		for (int i=0; i < g_apInstantiatedResPtrs.GetSize(); i++) {
			if (g_apInstantiatedResPtrs[i]->m_pRes == this) {
				delete g_apInstantiatedResPtrs[i];
				g_apInstantiatedResPtrs.RemoveAt(i);
				return;
			}
		}
	}

	// (c.haag 2010-07-12 17:47) - PLID 39614 - Logs and traces output
	void CReservation::LogAndTrace(const CString& strOut)
	{
		Log(strOut);
		OutputDebugString(strOut + "\n");
	}

	// (c.haag 2010-07-12 17:47) - PLID 39614 - This will delete a reservation and log if a function
	// that isn't the caller also holds a reference to it.
	// (j.gruber 2011-02-17 14:54) - PLID 42425 - added option not to resolve
	HRESULT CReservation::DeleteRes(const CString& strCaller, BOOL bResolve /*=TRUE*/) 
	{
		CArray<CInstantiatedResPtr*,CInstantiatedResPtr*> apCandidates;
		int i;

		// Look in our global array of instantiated reservations and see if any meet these prerequisites:
		// 1. The item has an identical reservation or template item ID
		// 2. The item is owned by a function other than the caller
		CSingleLock cs(&l_csInstantiatedResPtrs, TRUE); // (c.haag 2010-08-14 14:29) - PLID 40086
		for (i=0; i < g_apInstantiatedResPtrs.GetSize(); i++) {
			CInstantiatedResPtr* p = g_apInstantiatedResPtrs[i];
			if (!p->m_pRes->IsNull() && p->m_pRes->GetOwner() != strCaller) 
			{
				if (GetReservationID() > -1 && p->m_pRes->GetReservationID() == GetReservationID()) 
				{
					apCandidates.Add( p );
				}
				else if (GetTemplateItemID() > -1 && p->m_pRes->GetTemplateItemID() == GetTemplateItemID()) 
				{
					apCandidates.Add( p );
				}
			}
		}

		// (c.haag 2010-08-14 14:29) - PLID 40086 - Keep the lock in place until we're finished with our candidate
		// array. We don't want ~CReservation to somehow get called on an object inside apCandidates (which 
		// should be treated as an active subset of g_apInstantiatedResPtrs)

		// If we found anything, trace and log it.
		if (apCandidates.GetSize() > 0) {
			CString strOut;
			LogAndTrace("");
			LogAndTrace("----------------------------------------------------------------------------------------");
			LogAndTrace(FormatString("WarnOfOpenReservations [%s -> %s] called with %d open reservations:", strCaller, __FUNCTION__, apCandidates.GetSize()));
			LogAndTrace("----------------------------------------------------------------------------------------");

			for (i=0; i < apCandidates.GetSize(); i++) {
				CInstantiatedResPtr* p = apCandidates[i];
				LogAndTrace(FormatString("(%d): Reservation ID = %d. Owner = %s",
					i+1, p->m_pRes->GetReservationID(), p->m_pRes->GetOwner()));
				Log("");
			}
			LogAndTrace("----------------------------------------------------------------------------------------");

			// If you get this assertion, it means some other place in Practice holds a reference to this reservation, and deleting it
			// now will result in that place being left with a "dangling reservation". Even if this is harmless, it still needs addressed.
			ASSERT(FALSE);
#ifdef _DEBUG
			// In debug mode, throw an exception.
			ThrowNxException("Practice attempted to delete a reservation while it was in use by another function!");
#endif
		}
		// (j.gruber 2011-02-17 14:55) - PLID 42425
		if (bResolve) {
			return m_pRes->DeleteRes(); 
		}
		else {
			return m_pRes->DeleteWithoutResolve(); 
		}
	}

#pragma endregion

#pragma region SingleDay Objects

	// (c.haag 2010-03-26 15:16) - PLID 37332 - Destruction. No need to do anything in this
	// check-in; but we will likely change in later
	CSingleDay::~CSingleDay()
	{
	}

	// Returns TRUE if a reservation exists. Call this wherever GetReservation() is called such
	// that the result is only compared with NULL and discarded.
	BOOL CSingleDay::HasReservation(long Day, long Index)
	{
		return (NULL != m_pCtrl->GetReservation(Day, Index)) ? TRUE : FALSE;
	}

	// Just like GetReservation but returns an LPDISPATCH; an unencapsulated reservation object.
	LPDISPATCH CSingleDay::GetReservationRaw(long Day, long Index)
	{
		return (LPDISPATCH)m_pCtrl->GetReservation(Day, Index);
	}

	// (c.haag 2010-04-30 11:32) - PLID 38379 - Logs and traces output
	void CSingleDay::LogAndTrace(const CString& strOut)
	{
		Log(strOut);
		OutputDebugString(strOut + "\n");
	}

	// (c.haag 2010-04-30 11:32) - PLID 38379 - Checks to see if Practice owns a reference to
	// any reservations. This is called in functions where we expect Practice to have none.
	void CSingleDay::WarnOfOpenReservations(const CString& strGrandCaller, const CString& strCaller)
	{
		CArray<CInstantiatedResPtr*,CInstantiatedResPtr*> apCandidates;
		int i;

		// Look in our global array of instantiated reservations and see which ones are not null and belong
		// to this singleday control. If we find any, then those are "candidate" reservations that will get
		// disconnected if something like a Clear or a Shift happens. These are the ones to report.
		CSingleLock cs(&l_csInstantiatedResPtrs, TRUE); // (c.haag 2010-08-14 14:29) - PLID 40086
		for (i=0; i < g_apInstantiatedResPtrs.GetSize(); i++) {
			CInstantiatedResPtr* p = g_apInstantiatedResPtrs[i];
			if (!p->m_pRes->IsNull()) {
				if (p->m_pRes->GetSingleDayCtrl() == this->m_pCtrl) {
					apCandidates.Add( g_apInstantiatedResPtrs[i] );
				}
			}
		}

		// (c.haag 2010-08-14 14:29) - PLID 40086 - Keep the lock in place until we're finished with our candidate
		// array. We don't want ~CReservation to somehow get called on an object inside apCandidates (which 
		// should be treated as an active subset of g_apInstantiatedResPtrs)

		// If we found anything, trace and log it.
		if (apCandidates.GetSize() > 0) {
			CString strOut;
			LogAndTrace("");
			LogAndTrace("----------------------------------------------------------------------------------------");
			LogAndTrace(FormatString("WarnOfOpenReservations [%s -> %s] called with %d open reservations:", strGrandCaller, strCaller, apCandidates.GetSize()));
			LogAndTrace("----------------------------------------------------------------------------------------");

			for (i=0; i < apCandidates.GetSize(); i++) {
				CInstantiatedResPtr* p = apCandidates[i];
				LogAndTrace(FormatString("(%d): Reservation ID = %d. Owner = %s",
					i+1, p->m_pRes->GetReservationID(), p->m_pRes->GetOwner()));
				Log("");
			}
			LogAndTrace("----------------------------------------------------------------------------------------");

			// (c.haag 2010-05-06) - PLID 38379 - If you get this assertion, it means Practice has a reference for at least one
			// reservation, and it tried to do something that could result in the reservation getting deleted or disconnected from
			// the SingleDay control. Even if this is harmless, it still needs addressed.
			ASSERT(FALSE);
#ifdef _DEBUG
			// In debug mode, throw an exception.
			ThrowNxException("Practice attempted to improperly update the SingleDay control with at least one reservation open!");
#endif
		}
	}

	// (c.haag 2010-04-30 11:32) - PLID 38379 - Special handling for when Practice owns a reference to any reservations
	HRESULT CSingleDay::Clear(const CString& strCaller)
	{
		WarnOfOpenReservations(strCaller, "Clear");
		return m_pCtrl->Clear();
	}

	// (c.haag 2010-04-30 11:32) - PLID 38379 - Special handling for when Practice owns a reference to any reservations
	void CSingleDay::PutDayTotalCount(const CString& strCaller, long n)
	{
		WarnOfOpenReservations(strCaller, "PutDayTotalCount");
		m_pCtrl->PutDayTotalCount(n);
	}

	// (c.haag 2010-05-04 3:08) - PLID 38379 - Special handling for when Practice owns a reference to any reservations
	HRESULT CSingleDay::Shift(const CString& strCaller, short nNumDays)
	{
		WarnOfOpenReservations(strCaller, "Shift");
		return m_pCtrl->Shift(nNumDays);
	}

#pragma endregion