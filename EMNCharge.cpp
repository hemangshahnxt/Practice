#include "stdafx.h"
#include "EMNCharge.h"
#include "EMN.h"

// (z.manning 2009-08-18 10:15) - PLID 35207 - Moved the body of this function to EMN.cpp
void EMNCharge::operator =(EMNCharge &cSource)
{
	nID = cSource.nID;
	nServiceID = cSource.nServiceID;
	strDescription = cSource.strDescription;
	strMod1 = cSource.strMod1;
	strMod2 = cSource.strMod2;
	strMod3 = cSource.strMod3;
	strMod4 = cSource.strMod4;
	dblQuantity = cSource.dblQuantity;
	cyUnitCost = cSource.cyUnitCost;
	bChanged = cSource.bChanged;
	sai = cSource.sai;
	strSubCode = cSource.strSubCode;
	strCode = cSource.strCode;
	// (j.jones 2012-01-26 12:12) - PLID 47700 - this needs to copy nInsuredPartyID
	nInsuredPartyID = cSource.nInsuredPartyID;
	// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
	nCategoryID = cSource.nCategoryID;
	nCategoryCount = cSource.nCategoryCount;
	// (j.jones 2009-01-02 09:01) - PLID 32601 - renamed to aryDiagIDs, and added strDiagCodeList
	//TES 2/28/2014 - PLID 61080 - Renamed to aryDiagIndexes
	int i=0;
	for(i=0; i < cSource.aryDiagIndexes.GetSize(); i++) {
		aryDiagIndexes.Add(cSource.aryDiagIndexes.GetAt(i));
	}		
	strDiagCodeList = cSource.strDiagCodeList;

	nQuoteChargeID = cSource.nQuoteChargeID;	// (j.jones 2008-06-04 16:17) - PLID 30255

	// (j.jones 2011-03-28 14:45) - PLID 42575 - added Billable flag
	bBillable = cSource.bBillable;
	
	
	for(i=0; i < cSource.aryPendingEMAuditInfo.GetSize(); i++) {
		CPendingAuditInfo* pOldInfo = (CPendingAuditInfo*)(cSource.aryPendingEMAuditInfo.GetAt(i));
		CPendingAuditInfo* pNewInfo = new CPendingAuditInfo(pOldInfo);
		aryPendingEMAuditInfo.Add(pNewInfo);
	}

	// (j.jones 2008-07-22 10:17) - PLID 30792 - copy the problems too
	// (c.haag 2009-05-19 10:03) - PLID 34310 - We now copy problem links
	for(i = 0; i < cSource.m_apEmrProblemLinks.GetSize(); i++) {
		// (z.manning 2009-08-18 10:19) - PLID 35207 - When copying problem links, check and see if
		// we have another EMN pointer to use to associate with the problem links and if so, use that.
		CEMR *pOwningEmr = NULL;
		if(pEmnOverride != NULL) {
			pOwningEmr = pEmnOverride->GetParentEMR();
		}
		else {
			pEmnOverride = cSource.m_apEmrProblemLinks[i]->GetEMN();
		}
		CEmrProblemLink *pNewLink = new CEmrProblemLink(cSource.m_apEmrProblemLinks[i], pOwningEmr);
		pNewLink->UpdatePointersWithCharge(pEmnOverride, this);
		m_apEmrProblemLinks.Add(pNewLink);
	}
}

//TES 6/15/2012 - PLID 50983 - Added, used to determine when charges have been modified.
BOOL EMNCharge::operator == (EMNCharge &ecCompare)
{
	if(nID != ecCompare.nID) {
		return FALSE;
	}
	if(nServiceID != ecCompare.nServiceID) {
		return FALSE;
	}
	if(strDescription != ecCompare.strDescription) {
		return FALSE;
	}
	if(strMod1 != ecCompare.strMod1) {
		return FALSE;
	}
	if(strMod2 != ecCompare.strMod2) {
		return FALSE;
	}
	if(strMod3 != ecCompare.strMod3) {
		return FALSE;
	}
	if(strMod4 != ecCompare.strMod4) {
		return FALSE;
	}
	if(dblQuantity != ecCompare.dblQuantity) {
		return FALSE;
	}
	if(cyUnitCost != ecCompare.cyUnitCost) {
		return FALSE;
	}
	//TES 6/15/2012 - PLID 50983 - sai.HasSameSource() will return FALSE if they're both blank
	if(sai.nSourceActionID != -1 || ecCompare.sai.nSourceActionID != -1 || sai.nSourceDetailID != -1 || ecCompare.sai.nSourceDetailID != -1) {
		if(!sai.HasSameSource(ecCompare.sai)) {
			return FALSE;
		}
	}
	if(strSubCode != ecCompare.strSubCode) {
		return FALSE;
	}
	if(strCode != ecCompare.strCode) {
		return FALSE;
	}
	// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
	if (nCategoryID != ecCompare.nCategoryID){
		return FALSE;
	}
	//TES 2/28/2014 - PLID 61080 - Just compare strDiagCodeList, that gives a full comparison of whichcodes
	/*if(aryDiagIndexes.GetCount() != ecCompare.aryDiagIndexes.GetCount()) {
		return FALSE;
	}
	for(int nDiag = 0; nDiag < aryDiagIndexes.GetCount(); nDiag++) {
		bool bFound = false;
		long nDiagID = aryDiagIDs[nDiag];
		for(int nDiag2 = 0; nDiag2 < ecCompare.aryDiagIndexes.GetCount() && !bFound; nDiag2++) {
			if(ecCompare.aryDiagIDs[nDiag2] == nDiagID) bFound = true;
		}
		if(!bFound) {
			return FALSE;
		}
	}*/
	if(strDiagCodeList != ecCompare.strDiagCodeList) {
		return FALSE;
	}
	
	
	if(nQuoteChargeID != ecCompare.nQuoteChargeID) {
		return FALSE;
	}
	if(bBillable != ecCompare.bBillable) {
		return FALSE;
	}

	//TES 6/15/2012 - PLID 50983 - We don't compare the problem links here, there's already a HasChangedProblemLinks() function

	if(nInsuredPartyID != ecCompare.nInsuredPartyID) {
		return FALSE;
	}

	return TRUE;
}

// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
void EMNCharge::LogEmrObjectData(int nIndent, BOOL bModified, BOOL bDeleted)
{
	// Log this object
	::LogEmrObjectData(nIndent, nID, this, esotCharge, (nID == -1), bModified, bDeleted, strDescription,
		"nServiceID = %d  dblQuantity = %f  cyUnitCost = %s  sourceActionID = %d  sourceDetailID = %d  sourceDataGroupID = %d  sourceDetailImageStampID = %d"
		, nServiceID
		, dblQuantity
		, FormatCurrencyForInterface(cyUnitCost)
		, sai.nSourceActionID
		, sai.GetSourceDetailID()
		, sai.GetDataGroupID()
		, sai.GetDetailStampID()
	);

	// Log problems and problem links
	for (auto l : m_apEmrProblemLinks)
	{
		if (nullptr != l)
		{
			CEmrProblem* p = l->GetProblem();
			if (nullptr != p)
			{
				p->LogEmrObjectData(nIndent + 1);
			}
			if (nullptr != l)
			{
				l->LogEmrObjectData(nIndent + 1);
			}
		}
	}
}