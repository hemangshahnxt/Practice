#pragma once

#include "EMNCharge.h"

// (z.manning 2011-07-11 15:56) - PLID 44469 - Created a class for EMN charge arrays
class CEMNChargeArray : public CArray<EMNCharge*,EMNCharge*>
{
public:
	void Clear()
	{
		for(int nChargeIndex = GetCount() - 1; nChargeIndex >= 0; nChargeIndex--) {
			delete GetAt(nChargeIndex);
			RemoveAt(nChargeIndex);
		}
	}

	void CopyNew(CEMNChargeArray *parypChargesToCopy, CEMN *pEmnOverride, CMap<EMNCharge*,EMNCharge*,EMNCharge*,EMNCharge*> *pmapNewChargeToSourceCharge)
	{
		for(int nChargeIndex = 0; nChargeIndex < parypChargesToCopy->GetSize(); nChargeIndex++)
		{
			EMNCharge *pCharge = new EMNCharge;
			// (z.manning 2009-08-18 10:12) - PLID 35207 - Set the new EMN as the override
			pCharge->pEmnOverride = pEmnOverride;
			*pCharge = *(parypChargesToCopy->GetAt(nChargeIndex));
			Add(pCharge);
			if(pmapNewChargeToSourceCharge != NULL) {
				pmapNewChargeToSourceCharge->SetAt(pCharge, parypChargesToCopy->GetAt(nChargeIndex));
			}
		}
	}

	void CopyNew(CEMNChargeArray *parypChargesToCopy, CEMN *pEmnOverride)
	{
		CopyNew(parypChargesToCopy, pEmnOverride, NULL);
	}

	EMNCharge* FindByServiceID(const long nServiceID)
	{
		for(int nChargeIndex = 0; nChargeIndex < GetCount(); nChargeIndex++)
		{
			EMNCharge *pCharge = GetAt(nChargeIndex);
			if(pCharge->nServiceID == nServiceID) {
				return pCharge;
			}
		}
		return NULL;
	}

	BOOL HasCharge(EMNCharge *pCharge)
	{
		for(int nChargeIndex = 0; nChargeIndex < GetCount(); nChargeIndex++)
		{
			EMNCharge *pTemp = GetAt(nChargeIndex);
			if(pTemp == pCharge) {
				return TRUE;
			}
		}
		return FALSE;
	}

	void RemoveCharge(EMNCharge *pCharge)
	{
		for(int nChargeIndex = 0; nChargeIndex < GetCount(); nChargeIndex++)
		{
			EMNCharge *pTemp = GetAt(nChargeIndex);
			if(pTemp == pCharge) {
				RemoveAt(nChargeIndex);
				return;
			}
		}
	}
};