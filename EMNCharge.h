#pragma once

class EMNCharge {
public:
	long nID;
	long nServiceID;
	// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category And category count used to determine if the column is editable
	long nCategoryID;
	long nCategoryCount;
	CString strDescription;	
	CString strMod1;
	CString strMod2;
	CString strMod3;
	CString strMod4;
	double dblQuantity;
	COleCurrency cyUnitCost;
	BOOL bChanged;
	// (z.manning 2009-02-23 12:54) - PLID 33141 - Replaced source detail pointer with source action info class
	SourceActionInfo sai;
	CString strSubCode;	//This field is not audited, so we can just updated it as we go.
	CString strCode; //The office visits need this, we don't want to keep re-pulling from data.
	//CString strDiagList;		//DRT 1/11/2007 - PLID 24177 - || delimited list of all diagnosis codes linked to this charge (Whichcodes)
	//TES 2/28/2014 - PLID 61080 - Renamed to aryDiagIndexes, this now contains indexes within the EMN's m_aryDiagCodes array
	CArray<long, long> aryDiagIndexes;	// (j.jones 2009-01-02 09:00) - PLID 32601 - we now track the IDs of DiagCodes linked to this charge (Whichcodes)
	CString strDiagCodeList;	// (j.jones 2009-01-02 09:16) - PLID 32601 - used for auditing

	// (j.jones 2007-08-30 10:15) - PLID 27221 - added a pending audit info array, for E/M Checklist purposes
	CArray<CPendingAuditInfo*, CPendingAuditInfo*> aryPendingEMAuditInfo;
	BOOL m_bCreatedOnNewEMN; //needed for the E/M audits

	// (j.jones 2008-06-04 16:17) - PLID 30255 - track the charge ID from a quote
	long nQuoteChargeID;

	// (j.jones 2011-03-28 14:45) - PLID 42575 - added Billable flag
	BOOL bBillable;

	// (j.jones 2008-07-22 10:13) - PLID 30792 - added an array to track problems
	// (c.haag 2009-05-16 12:03) - PLID 34310 - We now track problem links instead of problems.
	CArray<CEmrProblemLink*, CEmrProblemLink*> m_apEmrProblemLinks;

	// (z.manning 2009-08-18 09:54) - PLID 35207 - Added a pointer to an EMN that can be set to
	// use as the EMN for problem links in the assignment operator.
	CEMN *pEmnOverride;

	long nInsuredPartyID;	// (j.dinatale 2012-01-03 17:39) - PLID 39451 - need to keep track of the insured party for this

	EMNCharge() {
		nID = -1;
		nServiceID = -1;
		nCategoryID = -1;
		nCategoryCount = 0;
		dblQuantity = 0.0;
		cyUnitCost = COleCurrency(0,0);
		bChanged = FALSE;
		m_bCreatedOnNewEMN = FALSE;
		nQuoteChargeID = -1;	// (j.jones 2008-06-04 16:17) - PLID 30255
		pEmnOverride = NULL;
		bBillable = TRUE;	// (j.jones 2011-03-28 14:45) - PLID 42575 - added Billable flag
		nInsuredPartyID = -2;	// (j.dinatale 2012-01-03 17:36) - PLID 39451
	}

	// (j.jones 2008-07-22 11:41) - PLID 30792 - remove all problems
	~EMNCharge() {

		// (c.haag 2009-05-19 12:00) - PLID 34310 - Remove problem links
		for(int i = 0; i < m_apEmrProblemLinks.GetSize(); i++) {
			delete m_apEmrProblemLinks[i];
		}
		m_apEmrProblemLinks.RemoveAll();

		// (z.manning 2011-07-12 09:58) - PLID 44469 - Moved this here instead from the EMN's destructor
		// (j.jones 2007-08-30 10:15) - PLID 27221 - added a pending audit info array, for E/M Checklist purposes
		for(int j = aryPendingEMAuditInfo.GetSize() - 1; j >= 0; j--) {
			delete aryPendingEMAuditInfo.GetAt(j);
		}
		aryPendingEMAuditInfo.RemoveAll();
	}

	// (j.jones 2007-08-30 10:36) - PLID 27221 - this became required after I added aryPendingEMAuditInfo
	// (z.manning 2009-08-18 10:15) - PLID 35207 - Moved the body of this function to EMN.cpp
	void operator =(EMNCharge &cSource);

	//TES 6/15/2012 - PLID 50983 - Added, used to determine when charges have been modified.
	BOOL operator ==(EMNCharge &ecCompare);

	// (j.jones 2008-07-22 10:59) - PLID 30792 - returns true if there are any undeleted problems on the charge
	// (c.haag 2009-05-19 10:10) - PLID 34310 - Check problem links
	BOOL HasProblems()
	{
		try {

			for(int i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
				CEmrProblem *pProblem = m_apEmrProblemLinks.GetAt(i)->GetProblem();
				if(pProblem != NULL && !pProblem->m_bIsDeleted) {

					return TRUE;
				}
			}

		}NxCatchAll("Error in EMNCharge::HasProblems");

		return FALSE;
	}

	// (j.jones 2008-07-22 10:59) - PLID 30792 - returns true if there are only undeleted, closed problems on the charge
	// (c.haag 2009-05-19 10:10) - PLID 34310 - Check problem links
	BOOL HasOnlyClosedProblems()
	{
		try {

			BOOL bHasProblems = FALSE;
			BOOL bHasOnlyClosed = TRUE;

			for(int i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
				CEmrProblem *pProblem = m_apEmrProblemLinks.GetAt(i)->GetProblem();
				if(pProblem != NULL && !pProblem->m_bIsDeleted) {

					bHasProblems = TRUE;
					
					if(pProblem->m_nStatusID != 2) {
						bHasOnlyClosed = FALSE;
					}
				}
			}

			if(bHasProblems && bHasOnlyClosed) {
				return TRUE;
			}
			else {
				return FALSE;
			}

		}NxCatchAll("Error in EMNCharge::HasOnlyClosedProblems");

		return FALSE;
	}

	// (j.jones 2008-07-23 11:06) - PLID 30792 - returns true if any problems are marked as modified,
	// including deleted items
	// (c.haag 2009-05-19 10:10) - PLID 34310 - Check problem links
	// (z.manning 2009-05-22 11:15) - PLID 34297 - Changed to detect changed links as that's all we need now.
	BOOL HasChangedProblemLinks()
	{
		try
		{
			for(int i = 0; i < m_apEmrProblemLinks.GetSize(); i++)
			{
				CEmrProblemLink *pProblemLink = m_apEmrProblemLinks.GetAt(i);
				if(pProblemLink != NULL) {
					if(pProblemLink->GetID() == -1) {
						return TRUE;
					}
				}
			}

		}NxCatchAll("Error in EMNCharge::HasChangedProblemLinks");

		return FALSE;
	}

	// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
	void LogEmrObjectData(int nIndent, BOOL bModified, BOOL bDeleted);
};
