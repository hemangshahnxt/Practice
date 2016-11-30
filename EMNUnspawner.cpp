// EMNUnspawner.cpp: implementation of the CEMNUnspawner class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EMNUnspawner.h"
#include "EMRProblemDeleteDlg.h"
#include "EMN.h"
#include "EmrTreeWnd.h"
#include "EmrCodingGroupManager.h"
#include "EMR.h"
#include "EMNDetail.h"
#include "EmrTopic.h"
#include "EmrHotSpot.h"
#include "WoundCareCalculator.h" // (r.gonet 08/03/2012) - PLID 51949

#include "NxCache.h"

// (a.walling 2011-08-11 16:43) - PLID 45021 - TableRow.m_pID is now TableRow.m_ID, which is not allocated on the heap.

// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/*****************************************************************************************************************************
	DRT 8/2/2007 - PLID 26914 - This was created to be an overall unspawning utility, that can be easily used anywhere, and 
	runs that the fastest possible speed for revoking actions.  It also consolidates a number of tasks that we used to run
	in 20 different places.
	Note that procedure actions (source eaoProcedure) are not covered by this utility at this time.  See 
	CEMR::RevokeProcedureActions()
*****************************************************************************************************************************/

// (j.jones 2013-05-16 14:08) - PLID 56596 - moved the CUnspawningSource class functions to the .cpp from the .h
//Default constructor
CUnspawningSource::CUnspawningSource() {
	pSourceDetail = NULL;
}

//Use this constructor in most cases
CUnspawningSource::CUnspawningSource(CEMNDetail *pDtl, ListElement _le) {
	pSourceDetail = pDtl;
	le = _le;
	eActionSourceType = eaoEmrDataItem;
}

// (z.manning, 01/23/2008) - PLID 28690 - Use this constructor 
CUnspawningSource::CUnspawningSource(CEMNDetail *pDtl, CEMRHotSpot *pSpot) {
	pSourceDetail = pDtl;
	pHotSpot = pSpot;
	eActionSourceType = eaoEmrImageHotSpot;
}

// (z.manning 2009-02-23 15:59) - PLID 33138 - A consructor for table dropdown actions
CUnspawningSource::CUnspawningSource(CEMNDetail *pDtl, TableSpawnInfo _tdsi) {
	pSourceDetail = pDtl;
	tdsi = _tdsi;
	if(tdsi.nDropdownID != -1) {
		eActionSourceType = eaoEmrTableDropDownItem;
	}
	else {
		// (c.haag 2012-10-26) - PLID 53440 - Use HasDetailImageStamp
		ASSERT(tdsi.tr.m_ID.HasDetailImageStamp());
		eActionSourceType = eaoSmartStamp;
	}
}

// (r.gonet 08/03/2012) - PLID 51949 - Create a new unspawning source for
//  a particular Wound Care Coding Condition
CUnspawningSource::CUnspawningSource(CEMNDetail *pDtl, EWoundCareCodingCondition _wcccConditionID) {
	pSourceDetail = pDtl;
	wcccConditionID = _wcccConditionID;
	eActionSourceType = eaoWoundCareCodingCondition;
}

EmrActionObject CUnspawningSource::GetActionSourceType()
{
	return eActionSourceType;
}

long CUnspawningSource::GetSourceID()
{
	switch(eActionSourceType)
	{
		case eaoEmrDataItem:
			return le.nID;
			break;

		case eaoEmrImageHotSpot:
			return pHotSpot->GetID();
			break;

		case eaoEmrTableDropDownItem:
			// (z.manning 2009-02-23 16:00) - PLID 33138
			return tdsi.nDropdownID;
			break;

		case eaoSmartStamp:
			// (z.manning 2010-03-02 15:39) - PLID 37571
			return tdsi.nStampID;
			break;

		// (r.gonet 08/03/2012) - PLID 51949 - The source is a Wound Care Coding Condition
		case eaoWoundCareCodingCondition:
			return wcccConditionID;
			break;

		default:
			AfxThrowNxException("CUnspawningSource::GetSourceID - Invalid action source type (%i)", eActionSourceType);
			break;
	}
	return -1;
}

BOOL CUnspawningSource::HasSameGroupID(CEMNDetail *pDetail)
{
	switch(eActionSourceType)
	{
		case eaoEmrDataItem:
			return (le.nDataGroupID == pDetail->GetSourceActionSourceDataGroupID());
			break;

		case eaoEmrImageHotSpot:
			return (pHotSpot->GetGroupID() == pDetail->GetSourceActionSourceHotSpotGroupID());
			break;

		case eaoEmrTableDropDownItem:
			// (z.manning 2009-02-23 16:00) - PLID 33138
			if(pSourceDetail != NULL) {
				if(pSourceDetail->m_pParentTopic != NULL) {
					if(pSourceDetail->m_pParentTopic->GetParentEMN() != NULL) {
						CEMR *pEMR = pSourceDetail->m_pParentTopic->GetParentEMN()->GetParentEMR();
						if(pEMR != NULL) {
							// (a.walling 2013-07-23 21:34) - PLID 57686 - Use Nx::Cache or die
							const long nDropdownGroupID = Nx::Cache::EmrTableDropdownInfo::GetDropdownGroupIDFromDropdownID(tdsi.nDropdownID);
							if(nDropdownGroupID == -1) {
								ASSERT(FALSE);
								return FALSE;
							}
							return (nDropdownGroupID == pDetail->GetSourceActionSourceTableDropdownGroupID());
						}
					}
				}
			}
			ASSERT(FALSE);
			break;

		case eaoSmartStamp:
			// (z.manning 2010-03-02 15:46) - PLID 37571 - No group IDs for smart stamps
			return TRUE;
			break;

		case eaoWoundCareCodingCondition:
			// (r.gonet 08/03/2012) - PLID 51949 - similar for Wound Care Calculator, no group ids
			return TRUE;
			break;

		default:
			AfxThrowNxException("CUnspawningSource::HasSameGroupID (detail) - Invalid action source type (%i)", eActionSourceType);
			break;
	}

	return FALSE;
}

BOOL CUnspawningSource::HasSameGroupID(CEMRTopic *pTopic)
{
	switch(eActionSourceType)
	{
		case eaoEmrDataItem:
			return (le.nDataGroupID == pTopic->GetSourceActionSourceDataGroupID());
			break;

		case eaoEmrImageHotSpot:
			return (pHotSpot->GetGroupID() == pTopic->GetSourceActionSourceHotSpotGroupID());
			break;

		case eaoEmrTableDropDownItem:
			// (z.manning 2009-02-23 16:00) - PLID 33138
			if(pSourceDetail != NULL) {
				if(pSourceDetail->m_pParentTopic != NULL) {
					if(pSourceDetail->m_pParentTopic->GetParentEMN() != NULL) {
						CEMR *pEMR = pSourceDetail->m_pParentTopic->GetParentEMN()->GetParentEMR();
						if(pEMR != NULL) {
							// (a.walling 2013-07-23 21:34) - PLID 57686 - Use Nx::Cache or die
							const long nDropdownGroupID = Nx::Cache::EmrTableDropdownInfo::GetDropdownGroupIDFromDropdownID(tdsi.nDropdownID);
							if(nDropdownGroupID == -1) {
								ASSERT(FALSE);
								return FALSE;
							}
							return (nDropdownGroupID == pTopic->GetSourceActionSourceTableDropdownGroupID());
						}
					}
				}
			}
			ASSERT(FALSE);
			break;

		case eaoSmartStamp:
			// (z.manning 2010-03-02 15:46) - PLID 37571 - No group IDs for smart stamps
			return TRUE;
			break;

		case eaoWoundCareCodingCondition:
			// (r.gonet 08/03/2012) - PLID 51949 - similar for Wound Care Calculator, no group ids
			return TRUE;
			break;

		default:
			AfxThrowNxException("CUnspawningSource::HasSameGroupID (topic) - Invalid action source type (%i)", eActionSourceType);
			break;
	}

	return FALSE;
}

// (z.manning 2009-03-24 16:26) - PLID 33450 - Added an equality operator
bool CUnspawningSource::operator==(CUnspawningSource& usSource)
{
	// (z.manning 2009-03-24 16:56) - PLID 33450 - We also need to compare row group ID
	// if this is a table dropdown spawning source.
	if(GetActionSourceType() == eaoEmrTableDropDownItem) {
		bool bGroupIDOk = false;
		if(tdsi.tr.nGroupID != -1 && tdsi.tr.nGroupID == usSource.tdsi.tr.nGroupID) {
			bGroupIDOk = true;
		}
		bool bDetailStampOk = false;
		if(tdsi.tr.m_ID == usSource.tdsi.tr.m_ID) {
			bDetailStampOk = true;
		}
		if(!bGroupIDOk && !bDetailStampOk) {
			return false;
		}
	}

	// (z.manning 2010-03-02 15:47) - PLID 37571 - Check smart stamp source
	if(GetActionSourceType() == eaoSmartStamp) {
		if(tdsi.tr.m_ID != usSource.tdsi.tr.m_ID) {
			return false;
		}
	}

	return (GetActionSourceType() == usSource.GetActionSourceType() &&
		GetSourceID() == usSource.GetSourceID() &&
		pSourceDetail == usSource.pSourceDetail);
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEMNUnspawner::CEMNUnspawner(CEMN* pEMN)
{
	if(pEMN == NULL)
		AfxThrowNxException("NULL EMN provided when attempting to unspawn actions.");

	m_pEMN = pEMN;
	m_nOptions = 0;

	//Ensure all topics are loaded
	m_pEMN->EnsureCompletelyLoaded();
}

CEMNUnspawner::~CEMNUnspawner()
{

}

//////////////////////////////////////////////////////////////////////
//Exposed functions
//////////////////////////////////////////////////////////////////////


//By EMN -- I had originally planned a function to remove by EMN.  But we actually don't do any revoking (except procedures).  If anything in
//	your EMN had spawned a new EMN, that new EMN is left in the EMR.  We then send messages to the PIC to let it know which procedures
//	were in that EMN that were removed.  Therefore, there is no functionality for the EMNUnspawner to handle when removing an entire EMN.


//Parameters:
//	paryTopics - An array of topics that are going to be removed from the given CEMN.
//
//This function will accept a list of topics which are to be removed from an EMN.  Those topics will be traversed, and all the actions on the EMNDetails
//	in those topics (recursively) will be revoked.  This function just pulls all the details off the topics (and any subtopics below it) that will be 
//	removed and calls RemoveActionsByDetails().
//Note that all the details actually being removed should be removed AFTER the unspawning is complete, so that the search
//	can properly look at all elements.  Neither the CEMRTopic object nor any of the root CEMNDetail objects will be removed, it is left to the caller to do that.
void CEMNUnspawner::RemoveActionsByTopics(IN CArray<CEMRTopic*, CEMRTopic*> *paryTopics)
{
	//Get all the details on these topics into a temporary array.  Remember that our member array of details only includes those that will be unspawned, 
	//	not the "beginning" details that will be removed by RemoveTopic.
	CArray<CEMNDetail*, CEMNDetail*> aryDetails;

	//Get a pointer to all the details on these topics
	for(int i = 0; i < paryTopics->GetSize(); i++) {
		CEMRTopic *pTopic = paryTopics->GetAt(i);
		if(pTopic) {
			//Get all the details off this topic and any subtopics underneath it
			RecursivelyGetDetailsFromTopic(pTopic, &aryDetails);
		}
		else {
			//NULL value in our array, this should not be valid.  Please check
			//	the values you have passed in.
			ASSERT(FALSE);
		}
	}

	//
	//At this point we have an array of all details which are going to be removed.  This is the exact same thing as the user 
	//	loading this info and calling RemoveActionsByDetails().  The removal of a topic causes nothing special to happen,
	//	which is why we can just send it on to the other function.
	RemoveActionsByDetails(&aryDetails);
}

//Parameters:
//	paryDetails - An array of details that are going to be removed from the given CEMN
//
//This function will accept a list of details which are about to be removed from an EMN.
//Note that all the details actually being removed should be removed AFTER the unspawning is complete, so that the search
//	can properly look at all elements.  The root CEMNDetail objects will not be removed by this function, it is left to the caller to do that.
void CEMNUnspawner::RemoveActionsByDetails(IN CArray<CEMNDetail*, CEMNDetail*> *paryDetails)
{
	//When revoking by details, the details themselves are being entirely removed.  Therefore we DO want to mark our details as pending delete, 
	//	and everything spawned by them the same.  All remembered positions will be lost for these details.


	//
	//1)  All of the details passed in are going to be removed.  When unspawning on a template, we need
	//	to be aware of the state of the source.  So we'll flag all of these as "pending delete"
	FlagDetailsAsPendingDelete(paryDetails);


	//
	//2)  We need to concern ourselves with EMR Info Actions.  These are the ones that are spawned when you add
	//	a detail to an EMN.  These are pretty rarely used as of right now.
	CArray<CEMNDetail*, CEMNDetail*> aryDetailsWithInfoSpawns;
	FillInfoSpawnArrayFromDetailArray(&aryDetailsWithInfoSpawns, paryDetails);


	//
	//3)  We now have an array of info actions that will be revoked.  We next want to get a list of all the data actions
	//	that are going to be revoked.  On a template, we look at all the list items, because we keep their spawned actions
	//	in memory for fast access.  On patient EMNs, we only look at the list elements that are actually selected.  This whole
	//	section applies only to single & multi select items, nothing else can cause spawning.  We use the CUnspawningSource class
	//	for this, because we need to know both the source detail and the list element information to accurately lookup
	//	anything that might be spawned.
	CArray<CUnspawningSource, CUnspawningSource&> arySources;
	FillSourcesArrayFromDetails(&arySources, paryDetails);


	//
	//4)  Invoke the manager for unspawning.  This function will recursively traverse the EMR looking for any details that will be unspawned
	//	by our sources, create new sources, and then do the actual work of revoking.
	RecursivelyUnspawn(&arySources, &aryDetailsWithInfoSpawns);
}

void CEMNUnspawner::RemoveActionsByDataElements(IN CArray<ListElement, ListElement&> *paryListElements, IN CEMNDetail *pSourceDetail)
{
	//When revoking by action, the user does not wish any actual details on a template to be removed.  Therefore we do NOT want to mark 
	//	our details as pending delete.  This will cause the actual revoking code to just hide them, and the Save() functionality will
	//	then mark their position for future use.
	SetOption(eUnspawnDoNotMarkPendingDelete, true);

	//
	//1)  We have a list of elements that need their actions revoked, and a source detail -- turn these into unspawning sources
	CArray<CUnspawningSource, CUnspawningSource&> arySources;
	long nElementSize = paryListElements->GetSize();
	for(int i = 0; i < nElementSize; i++) {
		CUnspawningSource us(pSourceDetail, paryListElements->GetAt(i));
		arySources.Add(us);
	}


	//
	//2)  We don't have any details at this point that are being revoked, but we may, so make an array to hold them
	CArray<CEMNDetail*, CEMNDetail*> aryDetailsWithInfoSpawns;


	//
	//3)  Invoke the manager for unspawning.  This function will recursively traverse the EMR looking for any details that will be unspawned
	//	by our sources, create new sources, and then do the actual work of revoking.
	RecursivelyUnspawn(&arySources, &aryDetailsWithInfoSpawns);
}

// (z.manning 2011-07-25 12:10) - PLID 44649 - Changed type to CEMRHotSpotArray
void CEMNUnspawner::RemoveActionsByImageHotSpots(IN CEMRHotSpotArray *paryHotSpots, IN CEMNDetail *pSourceDetail)
{
	//When revoking by action, the user does not wish any actual details on a template to be removed.  Therefore we do NOT want to mark 
	//	our details as pending delete.  This will cause the actual revoking code to just hide them, and the Save() functionality will
	//	then mark their position for future use.
	SetOption(eUnspawnDoNotMarkPendingDelete, true);

	//
	//1)  We have a list of elements that need their actions revoked, and a source detail -- turn these into unspawning sources
	CArray<CUnspawningSource, CUnspawningSource&> arySources;
	long nSize = paryHotSpots->GetSize();
	for(int i = 0; i < nSize; i++) {
		CUnspawningSource us(pSourceDetail, paryHotSpots->GetAt(i));
		arySources.Add(us);
	}

	//
	//2)  We don't have any details at this point that are being revoked, but we may, so make an array to hold them
	CArray<CEMNDetail*, CEMNDetail*> aryDetailsWithInfoSpawns;

	//
	//3)  Invoke the manager for unspawning.  This function will recursively traverse the EMR looking for any details that will be unspawned
	//	by our sources, create new sources, and then do the actual work of revoking.
	RecursivelyUnspawn(&arySources, &aryDetailsWithInfoSpawns);
}

// (z.manning 2010-03-02 16:30) - PLID 37571
void CEMNUnspawner::RemoveActionsBySmartStamps(CArray<TableSpawnInfo,TableSpawnInfo&> *parynTableSpawnInfo, CEMNDetail *pSourceDetail)
{
	// (z.manning 2010-03-02 16:32) - PLID 37571 - Since smart stamp images have a linked detail, we
	// utilize the same logic as table dropdown spawning.
	RemoveActionsByTableDropdownItems(parynTableSpawnInfo, pSourceDetail);
}

// (z.manning 2009-02-23 16:02) - PLID 33138 - For table dropdown item based spawning
void CEMNUnspawner::RemoveActionsByTableDropdownItems(IN CArray<TableSpawnInfo,TableSpawnInfo&> *parynDropdownItems, IN CEMNDetail *pSourceDetail)
{
	//When revoking by action, the user does not wish any actual details on a template to be removed.  Therefore we do NOT want to mark 
	//	our details as pending delete.  This will cause the actual revoking code to just hide them, and the Save() functionality will
	//	then mark their position for future use.
	SetOption(eUnspawnDoNotMarkPendingDelete, true);

	//
	//1)  We have a list of elements that need their actions revoked, and a source detail -- turn these into unspawning sources
	CArray<CUnspawningSource, CUnspawningSource&> arySources;
	long nSize = parynDropdownItems->GetSize();
	for(int i = 0; i < nSize; i++) {
		// (z.manning 2009-03-04 11:12) - PLID 33072 - We now use TableDropdownSpawnInfo for table dropdown spawning
		TableSpawnInfo tdsi = parynDropdownItems->GetAt(i);
		CUnspawningSource us(pSourceDetail, tdsi);
		arySources.Add(us);
	}

	//
	//2)  We don't have any details at this point that are being revoked, but we may, so make an array to hold them
	CArray<CEMNDetail*, CEMNDetail*> aryDetailsWithInfoSpawns;

	//
	//3)  Invoke the manager for unspawning.  This function will recursively traverse the EMR looking for any details that will be unspawned
	//	by our sources, create new sources, and then do the actual work of revoking.
	RecursivelyUnspawn(&arySources, &aryDetailsWithInfoSpawns);
}

// (r.gonet 08/03/2012) - PLID 51949 - Removes actions that were added by the CWoundCareCalculator
// - pSourceDetail is the EMR table that is being used for Wound Care Coding.
void CEMNUnspawner::RemoveActionsByWoundCareCalculator(IN CEMNDetail *pSourceDetail)
{
	if(!pSourceDetail) {
		ThrowNxException("%s : The argument pSourceDetail is NULL.", __FUNCTION__);
	}

	//When revoking by action, the user does not wish any actual details on a template to be removed.  Therefore we do NOT want to mark 
	//	our details as pending delete.  This will cause the actual revoking code to just hide them, and the Save() functionality will
	//	then mark their position for future use.
	SetOption(eUnspawnDoNotMarkPendingDelete, true);

	//
	//1)  We have a source detail, which should be a table, and we need to try and unspawn all conditional actions added by the Wound
	//    Care Calculator for this item.
	CArray<CUnspawningSource, CUnspawningSource&> arySources;
	for(int i = 1; i < wcccEndPlaceholder; i++) {
		CUnspawningSource us(pSourceDetail, (EWoundCareCodingCondition)i);
		arySources.Add(us);
	}

	//
	//2)  We don't have any details at this point that are being revoked, but we may, so make an array to hold them
	CArray<CEMNDetail*, CEMNDetail*> aryDetailsWithInfoSpawns;

	//
	//3)  Invoke the manager for unspawning.  This function will recursively traverse the EMR looking for any details that will be unspawned
	//	by our sources, create new sources, and then do the actual work of revoking. Include deleted here because versioned wound care conditions don't make sense.
	RecursivelyUnspawn(&arySources, &aryDetailsWithInfoSpawns);
}

void CEMNUnspawner::SetOption(eUnspawningOption euo, bool bOn)
{
	//Add the option
	if(bOn) {
		m_nOptions |= euo;
	}
	else {
		//Remove the option
		m_nOptions &= ~euo;
	}
}

//////////////////////////////////////////////////////////////////////
//Protected recursion helper functions
//////////////////////////////////////////////////////////////////////

//Parameters:
//	pTopic - The CEMRTopic we want to get details from
//	paryDetails - The array we should add all details to the end of
//
//Given a CEMRTopic, recursively gets all the details off that topic and its own subtopics
void CEMNUnspawner::RecursivelyGetDetailsFromTopic(IN CEMRTopic *pTopic, OUT CArray<CEMNDetail*, CEMNDetail*> *paryDetails)
{
	//First, get all the details off this topic.
	{
		long nDetails = pTopic->GetEMNDetailCount();
		for(int i = 0; i < nDetails; i++) {
			paryDetails->Add(pTopic->GetDetailByIndex(i));
		}
	}

	//Second, we need to get all the details off any subtopics (and their subtopics, etc)
	{
		long nSubTopics = pTopic->GetSubTopicCount();
		for(int i = 0; i < nSubTopics; i++) {
			RecursivelyGetDetailsFromTopic(pTopic->GetSubTopic(i), paryDetails);
		}
	}
}

//DRT 9/27/2007 - PLID 27515 - See the comments in RecursivelyGather where this is called to explain the reasoning behind it.
//	The goal is to search the details parents, and all ancestors of that parent to see if any
//	of them were spawned by our unspawning source.  If so, we return true, otherwise false.
bool CEMNUnspawner::HierarchicalParentTopicSpawnedByThisSource(IN CEMNDetail *pDetailToCheck, IN CUnspawningSource *pus)
{
	CEMRTopic *pTopic = pDetailToCheck->m_pParentTopic;
	while(pTopic) {
		//See if this topic was spawned by the unspawning source.  This code is very similar to that 
		//	in the first loop of RecursivelyGather.

		//Evaluate this particular topic.  Does it have the same source detail?  Does it have the same source action ID?  If so, 
		//	then this is a topic which will be revoked by our CUnspawningSource pair.  
		//DRT 8/10/2007 - Apparently the pSourceDetail pointer of a CEMNDetail object is only valid when it's a new spawn on
		//	an EMN.  If you load a saved one, it is not filled in.  But on new ones, the Detail ID is probably -1.  So we need
		//	to test both cases here.
		bool bIsSourceOK = false;
		CEMNDetail *pCurrentSourceDetail = pTopic->GetSourceDetail();
		if(pCurrentSourceDetail) {
			//We have a current source detail, so compare that to our unspawning source
			if(pCurrentSourceDetail == pus->pSourceDetail) {
				//We have a match!
				bIsSourceOK = true;
			}
		}
		else {
			//No source detail, try the IDs instead
			//DRT 8/17/2007 - PLID 26914 - Apparently GetID() is only valid for patient EMNs!  If we're on a template, 
			//	we need to look at the template detail ID, otherwise the emn detail id.  I'm going to skip using GetID()
			//	at all because it's confusing and fairly new and I no longer trust it.
			// (z.manning 2010-03-11 13:22) - PLID 37571 - Do not include details if the source detail ID is -1
			if(pTopic->GetSourceDetailID() != -1 && pTopic->GetSourceDetailID() == (m_pEMN->IsTemplate() ? pus->pSourceDetail->m_nEMRTemplateDetailID : pus->pSourceDetail->m_nEMRDetailID)) {
				bIsSourceOK = true;
			}
		}

		// (a.walling 2010-04-05 18:10) - PLID 38060
		bool bIsSourceTableRowOK = false;
		{
			SourceActionInfo saiTopic = pTopic->GetSourceActionInfo();
			if (saiTopic.IsBlank() || saiTopic.eaoSourceType != eaoEmrTableDropDownItem) {
				bIsSourceTableRowOK = true;
			} else {
				if ( (saiTopic.GetTableRow() == NULL || saiTopic.GetTableRow()->m_ID.nDataID == -1) && 
					(pus->tdsi.tr.m_ID.nDataID == -1) ) {
					bIsSourceTableRowOK = true;
				} else if (saiTopic.GetTableRow()->m_ID == pus->tdsi.tr.m_ID) {
					bIsSourceTableRowOK = true;
				}
			}
		}

		//If the source detail has been found, we need to make sure it's the same action that caused the spawning.
		if(bIsSourceOK && bIsSourceTableRowOK && pTopic->GetSourceActionSourceID() == pus->GetSourceID()) {
			//We have a full match!
			return true;
		}
		//We need to check historically against the group.  If the source detail matches, check that the group ID of
		//	the list element is the same as the source data group that spawned our current topic.
		else if(bIsSourceOK && bIsSourceTableRowOK && pus->HasSameGroupID(pTopic)) {
			//DRT 8/14/2007 - I added the source action's data group ID.  This is basically what the
			//	IsSourceDetailEquivalent stuff did -- by looking up in queries later.  We can do it
			//	now in the load, and just compare.  If the source detail matches, and the EMRDataGroupID
			//	matches, then we know this is the same list element, just from a different version of 
			//	the Info Item.
			return true;
		}

		//Keep moving up the chain until we have no further ancestors
		pTopic = pTopic->GetParentTopic();
	}

	return false;
}


//Parameters:
//	parySources - An array of CUnspawningSource objects.  This is both a list that we compare against while searching the EMN for things
//		being unspawned, and a list that add to once we've found those new details.
//	paryDetailsWithInfoSpawns - An array of CEMNDetail pointers which have an EMR Info Action spawning from them.
//
//Given the 2 arrays, recursively traverses the array of sources, looking to match the source to any existing EMN detail.  This will find which
//	details are going to be removed once our revoke is performed.  We then take the list of all newly found items, pull out their list elements
//	as sources, and recursively continue to search for new unspawns until none are found.
void CEMNUnspawner::RecursivelyGather(IN OUT CArray<CUnspawningSource, CUnspawningSource&> *parySources, IN OUT CArray<CEMNDetail*, CEMNDetail*> *paryDetailsWithInfoSpawns)
{
	CArray<CEMNDetail*, CEMNDetail*> aryTempFoundDetails;

	//
	//1)  Loop through everything in our array of sources
	long nSourceSize = parySources->GetSize();
	for(int nSourceIdx = 0; nSourceIdx < nSourceSize; nSourceIdx++) {
		//Look for anything that our source will spawn.  We are only looking at EMNDetails that are spawned in this case
		//	(which should be all eaoEMRItem and eaoMintItems actions).  We aren't concerned with anything else like 
		//	cpt codes, etc, because they cannot spawn other items.
		CUnspawningSource us = parySources->GetAt(nSourceIdx);

		//Loop through every EMNDetail on the EMN (spawned details can go to any other topic, but must remain within the current EMN)
		long nDetailSize = m_pEMN->GetTotalDetailCount();
		for(int nDetailIdx = 0; nDetailIdx < nDetailSize ; nDetailIdx++) {
			CEMNDetail *pDetail = m_pEMN->GetDetail(nDetailIdx);

			//Evaluate this particular detail.  Does it have the same source detail?  Does it have the same source action ID?  If so, 
			//	then this is a detail which will be revoked by our CUnspawningSource pair.  We want to add this detail to an array for
			//	processing.
			//DRT 8/10/2007 - Apparently the pSourceDetail pointer of a CEMNDetail object is only valid when it's a new spawn on
			//	an EMN.  If you load a saved one, it is not filled in.  But on new ones, the Detail ID is probably -1.  So we need
			//	to test both cases here.

			bool bIsSourceOK = false;
			CEMNDetail *pCurrentSourceDetail = pDetail->GetSourceDetail();
			if(pCurrentSourceDetail) {
				//We have a current source detail, so compare using that.
				if(pDetail->GetSourceDetail() == us.pSourceDetail)
					bIsSourceOK = true;
			}
			else {
				//No source detail, try the IDs instead
				//DRT 8/17/2007 - PLID 26914 - Apparently GetID() is only valid for patient EMNs!  If we're on a template, 
				//	we need to look at the template detail ID, otherwise the emn detail id.  I'm going to skip using GetID()
				//	at all because it's confusing and fairly new and I no longer trust it.
				// (z.manning 2010-03-11 13:22) - PLID 37571 - Do not include topics if the source detail ID is -1
				if(pDetail->GetSourceDetailID() != -1 && pDetail->GetSourceDetailID() == (m_pEMN->IsTemplate() ? us.pSourceDetail->m_nEMRTemplateDetailID : us.pSourceDetail->m_nEMRDetailID))
					bIsSourceOK = true;
			}

			if(bIsSourceOK && pDetail->GetSourceActionSourceID() == us.GetSourceID()) {
				//We have a match!  Add this detail to our temporary array
				aryTempFoundDetails.Add(pDetail);
			}
			else if(bIsSourceOK && us.HasSameGroupID(pDetail)) {
				//DRT 8/14/2007 - I added the source action's data group ID.  This is basically what the
				//	IsSourceDetailEquivalent stuff did -- by looking up in queries later.  We can do it
				//	now in the load, and just compare.  If the source detail matches, and the EMRDataGroupID
				//	matches, then we know this is the same list element, just from a different version of 
				//	the Info Item.
				aryTempFoundDetails.Add(pDetail);
			}
			else {
				//This detail does not have the same source detail and source action, it is safe... for now

				//DRT 9/25/2007 - Note that I consider this very close to a bug.  We need to account for mint items here as well.  
				//	When a mint item is spawned, the details don't have the source detail / source action set, the topics do!  Then 
				//	all the mint item is revoked, it doesn't revoke the details spawned, it revokes the topics.  This mean that
				//	(a) If you take a detail off a mint-spawned topic and put it elsewhere, it will not be revoked, and (b) if you
				//	manually move a detail onto (or add a new detail to) a mint-spawned topic, when you revoke that mint-items action, it will
				//	delete those details, even though they weren't spawned.  I've added PLID 27514 to reconsider this behavior.  I think
				//	that if we properly flagged the spawned details as having the correct SourceDetailID / SourceActionID when they
				//	are spawned as part of a mint-items action, that we could let the Unspawner work without this block of code, 
				//	then go back afterward and remove any unused topics.  This would be safer than blindly removing all details on those topics.
				//If we come back later and fix the spawning to properly mark details with their source, I think we can just
				//	remove this block of code entirely.
				{
					if(HierarchicalParentTopicSpawnedByThisSource(pDetail, &us)) {
						//If any topic above this detail was spawned by this source, and we're revoking this source, then
						//	this topic is about to be destroyed, taking out this detail with it.  Add it to the list of 
						//	details to check for revoking.
						//DRT 10/16/2007 - PLID 27515 - We need to double check that we aren't re-adding our own source!  This could
						//	happen if a detail spawned a mint items, then you moved the spawning item to that mint items, then remove
						//	the spawn.
						if(pDetail != us.pSourceDetail) {
							aryTempFoundDetails.Add(pDetail);
						}
					}
				}

			}
		}
	}


	//
	//2)  Everything found in aryTempFoundDetails is going to be unspawned.  Therefore we need to mark them as pending deletion.
	FlagDetailsAsPendingDelete(&aryTempFoundDetails);


	//
	//3)  We now have a temporary array of more details that are going to be revoked.  We're basically in the same state that
	//	we were when we reached RemoveActionsByDetails()
	
	//3a)  Since those will be removed, we need to see if they have any info spawns to add to our list.
	FillInfoSpawnArrayFromDetailArray(paryDetailsWithInfoSpawns, &aryTempFoundDetails);

	//3b)  Check our new details for any unspawning sources.  We only want to fill in a list of new sources to gather to, so 
	//	we avoid an infinite loop of looking at the same sources.
	CArray<CUnspawningSource, CUnspawningSource&> aryNewSources;
	FillSourcesArrayFromDetails(&aryNewSources, &aryTempFoundDetails);

	// (z.manning 2009-03-24 16:32) - PLID 33450 - Before we recursively gather sources based on the
	// newly found sources, make sure our newly found sources are not duplicates of sources we've 
	// previously gathered (which can happen with crazy recursive spawning setups).
	for(int nExistingSourceIndex = 0; nExistingSourceIndex < parySources->GetSize(); nExistingSourceIndex++)
	{
		CUnspawningSource usExisting = parySources->GetAt(nExistingSourceIndex);
		for(int nNewSourceIndex = 0; nNewSourceIndex < aryNewSources.GetSize(); nNewSourceIndex++) {
			CUnspawningSource usNew = aryNewSources.GetAt(nNewSourceIndex);
			if(usNew == usExisting) {
				aryNewSources.RemoveAt(nNewSourceIndex);
				nNewSourceIndex--;
			}
		}
	}

	//3c)  Again, Recursively gather our sources.  Only do this if we found something in the above.  This will
	//	be the control on the recursion.
	long nNewFound = aryNewSources.GetSize();
	if(nNewFound > 0) {
		//We found something in the above, so we must continue to recurse
		RecursivelyGather(&aryNewSources, paryDetailsWithInfoSpawns);

		//Now that we have finished gathering, copy all our "new" sources onto our source array.  Make sure that
		//	we are using the latest size of the "new source" array, the recursion may have gained us some
		//	new sources.
		for(int i = 0; i < aryNewSources.GetSize(); i++) {

			// (c.haag 2009-03-18 16:35) - PLID 33449 - Only attempt to add this source if it's not already in
			// parysources. This use to happen if item A spawns item B, item B spawns item C, and the topic 
			// containing all three is deleted. The result was multiple unspawning from the same source, causing
			// attempts at deleting objects twice and causing instability.
			EnsureExists(parySources, aryNewSources.GetAt(i));
		}
	}
	else {
		//Nothing new was found, we can end our recursion.
	}
}

//////////////////////////////////////////////////////////////////////
//General Helper Functions
//////////////////////////////////////////////////////////////////////

//Parameters:
//	parySources - An array of sources that are being unspawned.
//	paryDetailsWithInfoSpawns - An array of details that have info spawns that are being removed.
//
//Handles managing the bulk of the unspawning code.  This function should be called after a public function manipulates the
//	given details/topics/EMNs/whatever into a set of unspawning sources.  Both of the source lists may be added to as we
//	recursively traverse the EMR looking for details that will be unspawned.
void CEMNUnspawner::RecursivelyUnspawn(IN OUT CArray<CUnspawningSource, CUnspawningSource&> *parySources, IN OUT CArray<CEMNDetail*, CEMNDetail*> *paryDetailsWithInfoSpawns)
{
	//
	//1)  Recursion.  We have an array of source elements that are going to cause some kind of revoking.  What they are going to revoke
	//	is already on our EMR.  So to save data accesses, we can lookup the result of all the spawns in our arySources array, 
	//	add those details to our array of details that are going to actually be revoked, and recurse from there.  The 2 lists above are
	//	in their "initial" state, meaning that any details and list elements in them come from the details we were explicitly asked to 
	//	revoke.  We need to recurse through those elements and find details spawned that will be revoked, and update both lists accordingly.
	RecursivelyGather(parySources, paryDetailsWithInfoSpawns);


	//
	//2)  At this point, we now have one array which is all the details that spawn info actions that need to be revoked, and one array of 
	//	all data elements which have actions that need to be revoked.  We can now call LoadActionInfo.  That will give us a list of all
	//	the actions which are going to be revoked.
	MFCArray<EmrAction> aryAllActions;
	FillEMRActionArray(parySources, paryDetailsWithInfoSpawns, &aryAllActions);


	//
	//3)  We've now got all our actions, all our source details that spawn info actions, and our source actions for data elements.  We are
	//	ready to finally revoke the actions!
	ProcessRevoking(parySources, paryDetailsWithInfoSpawns, &aryAllActions);
}

//Parameters
//	paryDetails - An array of details that are going to be removed from the current EMN.
//
//Given an array of details, it goes to the topic of each detail and flags it as pending deletion.  This is necessary
//	for the CEMN::RevokeEmrAction to know if the source of any detail is gone, and to cleanup appropriately.
void CEMNUnspawner::FlagDetailsAsPendingDelete(IN const CArray<CEMNDetail*, CEMNDetail*> *paryDetails)
{
	//See comments above this enum for description.  This option is only followed on templates, it has no bearing on patient EMNs.
	if(( m_nOptions & eUnspawnDoNotMarkPendingDelete) && m_pEMN->IsTemplate()) {
		//Do not mark anything as pending deletion
		return;
	}

	long nCnt = paryDetails->GetSize();
	for(int i = 0; i < nCnt; i++) {
		CEMNDetail *pDetail = paryDetails->GetAt(i);

		//Get the parent topic, this is where the pending deletion info is held
		CEMRTopic *pTopic = pDetail->m_pParentTopic;
		if(pTopic) {
			pTopic->AddDetailToPendingDeletion(pDetail);
		}
		else {
			//There is no topic for this detail.  This should never happen on a fully loaded EMN
			ASSERT(FALSE);
		}
	}
}


//Parameters:
//	paryDetailsWithInfoSpawns - Array that will be filled with CEMNDetail pointers
//	paryDetails - The set of details that we are searching for info spawns
//
//Given an array of details, look for any of them which cause Info Actions to be spawned, and add them to the 
//	output array.
void CEMNUnspawner::FillInfoSpawnArrayFromDetailArray(OUT CArray<CEMNDetail*, CEMNDetail*> *paryDetailsWithInfoSpawns, IN const CArray<CEMNDetail*, CEMNDetail*> *paryDetails)
{
	//If the user has specified the option to not touch the info actions, we'll just skip over.  That will leave us with an array
	//	of no items, and LoadActionInfo will ignore that source type entirely.
	//Note:  We do this here instead of in the FillEmrActionArray function because it's possible a user might want to allow some details to be 
	//	handled with their info actions, but not others.
	if(m_nOptions & eUnspawnIgnoreInfoActions) {
		return;
	}

	long nSize = paryDetails->GetSize();
	for(int i = 0; i < nSize; i++) {
		CEMNDetail *pDetail = paryDetails->GetAt(i);
		if(pDetail) {
			//Check to see if this detail actually has an info items
			if(pDetail->GetHasInfoActionsStatus() != ehiasHasNoInfoItems) {
				paryDetailsWithInfoSpawns->Add(pDetail);
			}
		}
		else {
			//We lack the EMNDetail object passed in.  Please check your calling code to see that
			//	you aren't giving invalid parameters
			ASSERT(FALSE);
		}
	}
}

//Parameters
//	parySources - An array of spawnable sources that are possible to have caused some kind of spawn on the current EMN or EMN template.
//	paryDetails - An array of CEMNDetail pointers which we want to pull any source data from.
//
//Given an array of details, looks through all list elements in those details (only single/multi select, all others are ignored) and checks to
//	see, whether you're on a template or patient EMN, whether any of those list elements have caused something to be spawned.
void CEMNUnspawner::FillSourcesArrayFromDetails(OUT CArray<CUnspawningSource, CUnspawningSource&> *parySources, IN CArray<CEMNDetail*, CEMNDetail*> *paryDetails)
{
	BOOL bOnTemplate = m_pEMN->IsTemplate();

	long nSize = paryDetails->GetSize();
	for(int i = 0; i < nSize; i++) {
		CEMNDetail *pDetail = paryDetails->GetAt(i);
		if(pDetail) {
			//Check the detail type.  You can only spawn things from single and multi select lists
			if(pDetail->m_EMRInfoType == eitSingleList || pDetail->m_EMRInfoType == eitMultiList) {
				//We have the detail, look through all its list elements
				long nListElements = pDetail->GetListElementCount();
				for(int nListIdx = 0; nListIdx < nListElements; nListIdx++) {
					ListElement le = pDetail->GetListElement(nListIdx);
					if(bOnTemplate) {
						//We are currently on a template.  This means:
						//	- All non-emr item / non-mint items spawns are ignored on templates.  We need only look for
						//	nActionsType of 1.
						//	- We need to get all other list elements regardless of their selection status.
						if(le.nActionsType == 1) {
							//This particular list item spawns either an EMR Item or a Mint Items action
							CUnspawningSource us(pDetail, le);
							parySources->Add(us);
						}
						else {
							//This list element spawns either nothing, or a medication/cpt/diag/procedure/mint, which are completely ignored
							//	on all templates, so we can ignore the list element here.
						}
					}
					else {
						//We are currently on a patient EMN.  This means
						// - We only need get the list elements which are actually selected.
						// - EMNs worry about all types of spawning, so we need check the nActionsType for anything not -1
						//Apparently ListElement::nActionsType is not valid on patient EMNs, or there is another bug afflicting it.  Regardless, 
						//	things that cause spawning in patient EMNs are coming up -1.  This unfortunately means that we are adding things to our
						//	spawning source list that may not actually be causing anything to spawn.  They'll come up as IDs in the call to 
						//	LoadActionInfo(), but no actions will result.
						//This list element can spawn something
						if(le.bIsSelected) {
							//Selected, on a patient EMN, and it can spawn something.  We'll need to revoke it.
							CUnspawningSource us(pDetail, le);
							parySources->Add(us);
						}
						else {
							//This list element is not selected, and we're on a patient EMN, so we do not need to worry about
							//	what it might spawn.
						}
					}
				}
			}
			// (z.manning, 02/20/2008) - PLID 28690 - Handle image hot spot actions.
			else if(pDetail->m_EMRInfoType == eitImage)
			{
				// (z.manning 2010-03-10 16:50) - PLID 37571 - Also look through image stamps
				// (z.manning 2011-11-16 09:40) - PLID 46496 - We need to do this for all images, not just smart stamp images.
				//if(pDetail->GetSmartStampTableDetail() != NULL)
				{
					for(int nDetailStampIndex = 0; nDetailStampIndex < pDetail->GetImageStampCount(); nDetailStampIndex++) {
						EmrDetailImageStamp *pDetailStamp = pDetail->GetImageStampByIndex(nDetailStampIndex);
						MFCArray<EmrAction> aryActions;
						GetMainFrame()->GetEmrActionsByStampID(pDetailStamp->nStampID, TRUE, aryActions);
						if(aryActions.GetSize() > 0) {
							TableRow *ptr = NULL;
							if(pDetail->GetSmartStampTableDetail() != NULL) {
								ptr = pDetail->GetSmartStampTableDetail()->GetRowByDetailStamp(pDetailStamp);
							}
							if(ptr == NULL) {
								TableRow tr;
								tr.m_ID = TableRowID(pDetailStamp, pDetail->GetStampIndexInDetailByType(pDetailStamp));
								ptr = &tr;
							}
							if(ptr != NULL) {
								TableSpawnInfo tdsi(-1, *ptr, pDetailStamp->nStampID);
								CUnspawningSource us(pDetail, tdsi);
								parySources->Add(us);
							}
						}
					}
				}

				//We have the detail, look through all its hot spots
				CEMRHotSpotArray *paryHotSpots = pDetail->GetHotSpotArray();
				for(int nHotSpotIndex = 0; nHotSpotIndex < paryHotSpots->GetSize(); nHotSpotIndex++) {
					CEMRHotSpot *pSpot = paryHotSpots->GetAt(nHotSpotIndex);
					if(bOnTemplate) {
						//We are currently on a template.
						//TODO: One day we should add an nActionsType to the CEMRHotSpot class (similar to the one
						// in the list element class) so we don't unnecessarily add hot spots with no relevant
						// actions here.
						CUnspawningSource us(pDetail, pSpot);
						parySources->Add(us);
					}
					else {
						//We are currently on a patient EMN.  This means
						// - We only need get the hot spots which are actually selected.
						if(pSpot->GetSelected()) {
							//Selected, on a patient EMN, and it can spawn something.  We'll need to revoke it.
							CUnspawningSource us(pDetail, pSpot);
							parySources->Add(us);
						}
						else {
							//This hot spot is not selected, and we're on a patient EMN, so we do not need to worry about
							//	what it might spawn.
						}
					}
				}
			}
			// (z.manning 2009-02-24 09:55) - PLID 33138 - EMR table dropdown item actions
			else if(pDetail->m_EMRInfoType == eitTable)
			{
				for(int nTableElementIndex = 0; nTableElementIndex < pDetail->GetTableElementCount(); nTableElementIndex++)
				{
					TableElement te;
					pDetail->GetTableElementByIndex(nTableElementIndex, te);
					if(te.m_pColumn->nType == LIST_TYPE_DROPDOWN) {
						for(int nDropdownIndex = 0; nDropdownIndex < te.m_anDropdownIDs.GetSize(); nDropdownIndex++) {
							long nDropdownID = te.m_anDropdownIDs.GetAt(nDropdownIndex);
							if(nDropdownID > 0) {
								TableSpawnInfo tdsi(nDropdownID, *te.m_pRow, -1);
								CUnspawningSource us(pDetail, tdsi);
								parySources->Add(us);
							}
						}
					}
				}
				// (r.gonet 08/03/2012) - PLID 51949 - This table could have been used in a Wound Care Coding Calculation,
				//  which can perform actions with a source of WoundCareCalculator.
				if(pDetail->m_bUseWithWoundCareCoding) {
					// (r.gonet 08/03/2012) - PLID 51949 - This is a reset of all conditions, so add them all.
					for(int i = 1; i < (int)wcccEndPlaceholder; i++) {
						// (r.gonet 08/03/2012) - PLID 51949 - The source is a hard-coded condition that can be met when doing Wound Care Coding.
						CUnspawningSource us(pDetail, (EWoundCareCodingCondition)i);
						parySources->Add(us);	
					}
				}
			}
			else {
				//This detail is not a single select list, nor is it a multi select list.  Therefore it can't possibly spawn anything,
				//	and we can ignore it safely.
				// (z.manning, 02/20/2008) - PLID 28690 - It's also NOT an image.
				// (z.manning 2009-02-24 11:09) - PLID 33138 - Also not a table
			}
		}
		else {
			//Given an invalid CEMNDetail pointer.  Please check that the calling code is giving us
			//	legitimate details.
			ASSERT(FALSE);
		}
	}
}

//Parameters
//	parySources - An array of spawnable sources that are known to have caused some kind of spawn on the current EMN or EMN template.
//	paryDetailsWithInfoSpawns - An array of CEMNDetail pointers which are known to have caused some kind of EMR Info Action spawn on the current EMN or EMN template
//
//Given the arrays of data, this function generates the appropriate WHERE clause and calls LoadActionInfo (which generates a recordset).  For speed purposes, 
//	this function should be called as absolutely few times as possible.
void CEMNUnspawner::FillEMRActionArray(IN CArray<CUnspawningSource, CUnspawningSource&> *parySources, IN CArray<CEMNDetail*, CEMNDetail*> *paryDetailsWithInfoSpawns, OUT MFCArray<EmrAction> *paryActions)
{
	//Our eventual WHERE clause for LoadActionInfo
	CSqlFragment sqlWhere;

	//
	//1)  Generate all the actions that will be revoked that are EMR Info Actions.
	//	Copied this code out of CEMR::RevokeEMRInfoActions()
	long nDetailCnt = paryDetailsWithInfoSpawns->GetSize();
	if(nDetailCnt > 0) {
		//We only need generate the list if we actually have some details that spawn info actions (most of the time we do not)
		CString strIDList;
		for(int i = 0; i < nDetailCnt; i++) {
			CEMNDetail *pDetail = paryDetailsWithInfoSpawns->GetAt(i);
			CString str;
			str.Format("%li,", pDetail->m_nEMRInfoMasterID);
			strIDList += str;
		}
		strIDList.TrimRight(",");
		sqlWhere = CSqlFragment("SourceType = {CONST} AND SourceID IN (SELECT ID FROM EmrInfoT WHERE EmrInfoMasterID IN ({INTSTRING})) AND Deleted = 0", eaoEmrItem, strIDList);
	}


	//
	//2)  Generate all the actions that will be revoked from emr data actions
	//	Copied from CEMR::RevokeEMRDataActions()
	long nSourceCount = parySources->GetSize();
	if(nSourceCount > 0) {
		//If we had any sources, then generate a list to run the LoadActionInfo on
		CString strIDList;
		for(int i = 0; i < nSourceCount; i++) {
			CUnspawningSource us = parySources->GetAt(i);
			if(us.GetActionSourceType() == eaoEmrDataItem) {
				CString str;
				str.Format("%li,", us.GetSourceID());		//This is the DataID of the element
				strIDList += str;
			}
		}
		strIDList.TrimRight(",");


		if(!strIDList.IsEmpty()) {
			//This WHERE clause will give us a historical list of all actions which match this source, because it may well
			//	have changed since the items were originally saved (common on templates)
			CSqlFragment sqlAddtlWhere("SourceType = {CONST} AND SourceID IN (SELECT ID FROM EmrDataT WHERE EmrDataGroupID IN "
				"(SELECT EmrDataGroupID FROM EmrDataT SourceItem WHERE SourceItem.ID IN ({INTSTRING}))) AND Deleted = 0", eaoEmrDataItem, strIDList);

			if(!sqlWhere) {
				sqlWhere = sqlAddtlWhere;
			} else {
				sqlWhere = CSqlFragment("({SQL}) OR ({SQL})", sqlWhere, sqlAddtlWhere);
			}
		}

		// (z.manning, 01/23/2008) - PLID 28690 - Now handle hot spot based actions.
		//If we had any sources, then generate a list to run the LoadActionInfo on
		strIDList.Empty();
		for(i = 0; i < nSourceCount; i++) {
			CUnspawningSource us = parySources->GetAt(i);
			if(us.GetActionSourceType() == eaoEmrImageHotSpot) {
				CString str;
				str.Format("%li,", us.GetSourceID());		//This is the DataID of the element
				strIDList += str;
			}
		}
		strIDList.TrimRight(",");

		if(!strIDList.IsEmpty()) {
			//This WHERE clause will give us a historical list of all actions which match this source, because it may well
			//	have changed since the items were originally saved (common on templates)
			CSqlFragment sqlAddtlWhere("SourceType = {CONST} AND SourceID IN (SELECT ID FROM EmrImageHotSpotsT WHERE EmrSpotGroupID IN "
				"(SELECT EmrSpotGroupID FROM EmrImageHotSpotsT SourceItem WHERE SourceItem.ID IN ({INTSTRING}))) AND Deleted = 0", eaoEmrImageHotSpot, strIDList);

			if(!sqlWhere)
				sqlWhere = sqlAddtlWhere;
			else
				sqlWhere = CSqlFragment("({SQL}) OR ({SQL})", sqlWhere, sqlAddtlWhere);
		}

		// (z.manning 2009-02-23 16:14) - PLID 33138 - EMR table dropdown item based actions
		//If we had any sources, then generate a list to run the LoadActionInfo on
		strIDList.Empty();
		for(i = 0; i < nSourceCount; i++) {
			CUnspawningSource us = parySources->GetAt(i);
			if(us.GetActionSourceType() == eaoEmrTableDropDownItem) {
				CString str;
				str.Format("%li,", us.GetSourceID());
				strIDList += str;
			}
		}
		strIDList.TrimRight(",");

		if(!strIDList.IsEmpty()) {
			//This WHERE clause will give us a historical list of all actions which match this source, because it may well
			//	have changed since the items were originally saved (common on templates)
			CSqlFragment sqlAddtlWhere(
				"SourceType = {CONST} AND SourceID IN (SELECT ID FROM EmrTableDropdownInfoT WHERE DropdownGroupID IN "
				"(SELECT DropdownGroupID FROM EmrTableDropdownInfoT SourceItem WHERE SourceItem.ID IN ({INTSTRING}))) AND Deleted = 0"
				, eaoEmrTableDropDownItem, strIDList);

			if(!sqlWhere)
				sqlWhere = sqlAddtlWhere;
			else
				sqlWhere = CSqlFragment("({SQL}) OR ({SQL})", sqlWhere, sqlAddtlWhere);
		}

		// (z.manning 2010-03-02 17:01) - PLDI 37571 - Get the where clause for any smart stamp actions
		CString strStampIDs;
		for(i = 0; i < nSourceCount; i++) {
			CUnspawningSource us = parySources->GetAt(i);
			if(us.GetActionSourceType() == eaoSmartStamp) {
				strStampIDs += AsString(us.GetSourceID()) + ',';
			}
		}
		strStampIDs.TrimRight(',');
		if(!strStampIDs.IsEmpty()) {
			// (r.gonet 09/12/2012) - PLID 51949 - Allow deleted actions if the flag is set.
			CSqlFragment sqlAddtlWhere(
				"SourceType = {CONST} AND SourceID IN ({INTSTRING}) AND Deleted = 0"
				, eaoSmartStamp, strStampIDs);
			
			if(!sqlWhere)
				sqlWhere = sqlAddtlWhere;
			else
				sqlWhere = CSqlFragment("({SQL}) OR ({SQL})", sqlWhere, sqlAddtlWhere);
		}

		// (r.gonet 08/03/2012) - PLID 51949 - Also get all actions that reference a Wound Care Coding condition.
		CString strWoundCareConditionIDs;
		for(i = 0; i < nSourceCount; i++) {
			CUnspawningSource us = parySources->GetAt(i);
			if(us.GetActionSourceType() == eaoWoundCareCodingCondition) {
				strWoundCareConditionIDs += AsString(us.GetSourceID()) + ',';
			} else {
				// (r.gonet 08/03/2012) - PLID 51949 - Not a wound care action, so skip it.
			}
		}
		strWoundCareConditionIDs.TrimRight(',');
		if(!strWoundCareConditionIDs.IsEmpty()) {
			// (r.gonet 09/12/2012) - PLID 51949 - Allow deleted actions.
			CSqlFragment sqlAddtlWhere(
				"SourceType = {CONST} AND SourceID IN ({INTSTRING})"
				, eaoWoundCareCodingCondition, strWoundCareConditionIDs);

			if(!sqlWhere) {
				sqlWhere = sqlAddtlWhere;
			} else {
				sqlWhere = CSqlFragment("({SQL}) OR ({SQL})", sqlWhere, sqlAddtlWhere);
			}
		} else {
			// (r.gonet 08/03/2012) - PLID 51949 - There are no actions for Wound Care Conditions
		}
	}

	if(sqlWhere) {
		LoadActionInfo(sqlWhere, *paryActions, GetRemoteData());
	}
}

//Parameters
//	parySources - All of the UnspawningSource objects that have been found for our unspawning
//	paryDetailsWithInfoSpawns - All of the CEMNDetail pointers that cause an info action spawn.
//	paryAllActions - All actions loaded from the above arrays using FillEMRActionArray
//
//Given the 3 arrays of input data, process those into appropriate arrays for fastest execution, and call CEMR::RevokeEmrActions() with them.  This
//	function will clear out the arrays of all details, sources, and actions that match up to a valid unspawn.
void CEMNUnspawner::ProcessRevoking(IN CArray<CUnspawningSource, CUnspawningSource&> *parySources, IN OUT CArray<CEMNDetail*, CEMNDetail*> *paryDetailsWithInfoSpawns, IN OUT MFCArray<EmrAction> *paryAllActions)
{
	if(paryAllActions->GetSize() > 0) {
		//Here's how this will work.  For each action that we want to unspawn, we need to know its source detail for
		//	the RevokeEMRAction() function.  Unfortunately, the way LoadActionInfo works, we cannot get that at the same
		//	time.  So right now we've got an array of actions that need revoked, an array of details that spawn info type
		//	actions, and an array of source detail / list element pairs.  The revoking function wants all the actions for
		//	a specific source detail.  That means that we need to search for all the UnspawningSources with that source, 
		//	as well as all 
		//
		//So we need to do several loops to get everything.  We will build a map that links up our possible source details
		//	to every action that came from that source detail.
		//DRT 8/7/2007 - PLID 27003 - Instead of the map, we now keep an array of all actions, per dest type.  This is easier to
		//	mass-feed into the revoking functions.
		// (a.walling 2014-07-14 16:32) - PLID 62812 - use MFCArray

		MFCArray<CActionAndSource> aryCptActions;
		MFCArray<CActionAndSource> aryDiagActions;
		MFCArray<CActionAndSource> aryMedicationActions;
		MFCArray<CActionAndSource> aryProcedureActions;
		MFCArray<CActionAndSource> aryEmrItemActions;
		MFCArray<CActionAndSource> aryMintItemsActions;
		MFCArray<CActionAndSource> aryMintActions;
		MFCArray<CActionAndSource> aryTodoActions; // (c.haag 2008-06-24 10:55) - PLID 17244
		MFCArray<CActionAndSource> aryLabActions; // (z.manning 2008-10-08 17:11) - PLID 31628

		//
		//First, loop over all possible unspawning sources, in reverse because we'll be removing any that we use
		long nSourceCount = parySources->GetSize();
		for(int nSourceIdx = nSourceCount - 1; nSourceIdx >= 0; nSourceIdx--) {
			//Get our source information
			CUnspawningSource us = parySources->GetAt(nSourceIdx);
			long nSourceActionID = us.GetSourceID();

			//
			//Look through the entire remaining action array for all actions which were spawned from this source & action ID.  Search
			//	in reverse because we remove any matches.
			long nActionCnt = paryAllActions->GetSize();
			for(int nActionIdx = nActionCnt - 1; nActionIdx >= 0; nActionIdx--) {
				EmrAction ea = paryAllActions->GetAt(nActionIdx);

				//Does this action have a source type of a data item and a source action ID that matches?
				if(ea.eaoSourceType == us.GetActionSourceType() && ea.nSourceID == nSourceActionID) {
					//This action was spawned from us!  Save it into the appropriate array for revoking
					SourceActionInfo sai(&ea, us.pSourceDetail, &us.tdsi.tr);
					CActionAndSource aas(ea, sai);

					switch(ea.eaoDestType) {
					case eaoCpt:
						aryCptActions.Add(aas);
						break;
					// (b.savon 2014-07-14 11:21) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
					//case eaoDiag:
					case eaoDiagnosis:
						aryDiagActions.Add(aas);
						break;
					case eaoMedication:
						aryMedicationActions.Add(aas);
						break;
					case eaoProcedure:
						aryProcedureActions.Add(aas);
						break;
					case eaoEmrItem:
						aryEmrItemActions.Add(aas);
						break;
					case eaoMintItems:
						aryMintItemsActions.Add(aas);
						break;
					case eaoMint:
						aryMintActions.Add(aas);
						break;
					case eaoTodo: // (c.haag 2008-06-24 10:55) - PLID 17244
						aryTodoActions.Add(aas);
						break;
					case eaoLab: // (z.manning 2008-10-08 17:11) - PLID 31628
						aryLabActions.Add(aas);
						break;
					default:
						//bad dest type
						ASSERT(FALSE);
						break;
					}

					//Remove from the "all actions" array.  This will speed up future looping, and we've
					//	got the action copied over to the map array anyways.
					//DRT 9/28/2007 - PLID 26914 - Ahh, unfortunately we cannot remove the actions.  It is possible
					//	that a single action is spawned multiple times, by identical copied details (silly, but
					//	possible).  So we can't remove these, this action might actually get applied to 2 or more
					//	source details.
					//paryAllActions->RemoveAt(nActionIdx);
				}
				else {
					//This particular action does not match.  Continue on our merry way
				}
			}

			//Remove this spawning source from the array, we no longer need to look at it.
			parySources->RemoveAt(nSourceIdx);
		}

		//
		//At this point we have looked through all the unspawning sources, and setup an array of all actions spawned from each
		//	source CEMNDetail.  We now need to run a quick pass through the array of info spawning details and add them to the map
		//	as well.
		long nInfoCount = paryDetailsWithInfoSpawns->GetSize();
		for(int nInfoIdx = nInfoCount - 1; nInfoIdx >= 0; nInfoIdx--) {
			//Get the detail info
			CEMNDetail *pSourceDetail = paryDetailsWithInfoSpawns->GetAt(nInfoIdx);

			//
			//Look through the entire remaining action array for all actions which were spawned from this source & action ID.  Search
			//	in reverse because we remove any matches.
			long nActionCnt = paryAllActions->GetSize();
			for(int nActionIdx = nActionCnt - 1; nActionIdx >= 0; nActionIdx--) {
				EmrAction ea = paryAllActions->GetAt(nActionIdx);

				//Does this action have a source type of a data item and a source action ID that matches?  The action source
				//	is the EMR Info ID (not the EMR Info Master ID)
				if(ea.eaoSourceType == eaoEmrItem && ea.nSourceID == pSourceDetail->m_nEMRInfoID) {
					//This action was spawned from us!  Save it into the appropriate array for revoking
					SourceActionInfo sai(&ea, pSourceDetail);
					CActionAndSource aas(ea, sai);

					switch(ea.eaoDestType) {
					case eaoCpt:
						aryCptActions.Add(aas);
						break;
					// (b.savon 2014-07-14 11:21) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
					//case eaoDiag:
					case eaoDiagnosis:
						aryDiagActions.Add(aas);
						break;
					case eaoMedication:
						aryMedicationActions.Add(aas);
						break;
					case eaoProcedure:
						aryProcedureActions.Add(aas);
						break;
					case eaoEmrItem:
						aryEmrItemActions.Add(aas);
						break;
					case eaoMintItems:
						aryMintItemsActions.Add(aas);
						break;
					case eaoMint:
						aryMintActions.Add(aas);
						break;
					case eaoTodo: // (c.haag 2008-06-24 10:55) - PLID 17244
						aryTodoActions.Add(aas);
						break;
					case eaoLab: // (z.manning 2008-10-08 17:12) - PLID 31628
						aryLabActions.Add(aas);
						break;
					default:
						//bad dest type
						ASSERT(FALSE);
						break;
					}

					//Remove from the "all actions" array.  This will speed up future looping, and we've
					//	got the action copied over to the map array anyways.
					//DRT 9/28/2007 - PLID 26914 - Ahh, unfortunately we cannot remove the actions.  It is possible
					//	that a single action is spawned multiple times, by identical copied details (silly, but
					//	possible).  So we can't remove these, this action might actually get applied to 2 or more
					//	source details.
					//paryAllActions->RemoveAt(nActionIdx);
				}
				else {
					//This particular action does not match.  Continue on our merry way
				}
			}

			//Remove this info spawning detail from the array, we no longer need to look at it.
			paryDetailsWithInfoSpawns->RemoveAt(nInfoIdx);
		}


		//
		//At this point, our map should contain all details which have done any spawning, and an array of all the actions that
		//	came from that detail, whether they are info actions or data actions.  We group in this manner because the 
		//	current RevokeEmrActions function requires the source detail for each action being revoked.

		//If we did everything correctly, then our array of all actions, our array of info spawning details, and our array of
		//	unspawning sources should all be empty, because we should have gotten a match.  If we aren't empty here...
		//	then LoadActionInfo must not have found an action for one of the elements or details which are flagged in memory
		//	as spawning an action.
		ASSERT(paryDetailsWithInfoSpawns->GetSize() == 0);
		//We cannot actually detect when a patient EMN is going to spawn something (ListElement::nActionsType is 
		//	not valid on patient EMNs, only templates).  Therefore there may well be potential source actions detected that don't 
		//	actually spawn anything.  But on a template, we always should be empty
		if(m_pEMN->IsTemplate()) {
			ASSERT(parySources->GetSize() == 0);
		}
		//Upon further review, the action array actually may not be cleared.  We load a historical list of all actions that match
		//	our source.  If an item has been used & changed many times, there may be 5, 10, or even more actions that match!  In the
		//	above array splitting, we pull out the ones that match, so anything leftover here is just unnecessary, they're potential
		//	historic actions that didn't pan out.
		//DRT 9/28/2007 - PLID 26914 - We don't remove any actions anymore, because they might be used by multiple details.  So this
		//	assertion can definitely never be brought back.
		//ASSERT(paryAllActions->GetSize() == 0);

		// (c.haag 2008-07-31 10:00) - PLID 30897 - Now gather a list of all the Emr object pointers that will be involved in the unspawning
		CDoomedEmrObjectAry aryDoomedCpts;
		CDoomedEmrObjectAry aryDoomedDiags;
		CDoomedEmrObjectAry aryDoomedMedications;
		CDoomedEmrObjectAry aryDoomedProcedures;
		CDoomedEmrObjectAry aryDoomedItems;
		CDoomedEmrObjectAry aryDoomedTopics;
		CDoomedEmrObjectAry aryDoomedEMNs;
		CDoomedEmrObjectAry aryDoomedTodos;
		CDoomedEmrObjectAry aryDoomedLabs; // (z.manning 2008-10-08 17:00) - PLID 31628

		// (j.jones 2011-07-11 10:56) - PLID 38366 - track coding groups that will be affected
		CEmrCodingGroupArray aryCodingGroupsToChange;
		CMap<CEmrCodingGroup*, CEmrCodingGroup*, long, long> mapCodingGroupQuantitiesToSubtract;

		if(aryCptActions.GetSize() > 0) {
			// (j.jones 2011-07-11 10:56) - PLID 38366 - track coding groups that will be affected
			m_pEMN->GetEmrObjectsToRevoke_eaoCpt(&aryCptActions, aryDoomedCpts, aryCodingGroupsToChange, mapCodingGroupQuantitiesToSubtract);
		}
		if(aryDiagActions.GetSize() > 0) {
			m_pEMN->GetEmrObjectsToRevoke_eaoDiag(&aryDiagActions, aryDoomedDiags);
		}
		if(aryMedicationActions.GetSize() > 0) {
			m_pEMN->GetEmrObjectsToRevoke_eaoMedication(&aryMedicationActions, aryDoomedMedications);
		}
		if(aryProcedureActions.GetSize() > 0) {
			m_pEMN->GetEmrObjectsToRevoke_eaoProcedure(&aryProcedureActions, aryDoomedProcedures);
		}
		if(aryEmrItemActions.GetSize() > 0) {
			m_pEMN->GetEmrObjectsToRevoke_eaoEmrItem(&aryEmrItemActions, aryDoomedItems);
		}
		if(aryMintItemsActions.GetSize() > 0) {
			m_pEMN->GetEmrObjectsToRevoke_eaoMintItems(&aryMintItemsActions, aryDoomedTopics);
		}
		if(aryMintActions.GetSize() > 0) {
			//Special:  Mint's are handled at the EMR level, not the EMN level.
			m_pEMN->GetParentEMR()->GetEmrObjectsToRevoke_eaoMint(&aryMintActions, aryDoomedEMNs);
		}
		if(aryTodoActions.GetSize() > 0) { // (c.haag 2008-06-24 10:55) - PLID 17244
			m_pEMN->GetEmrObjectsToRevoke_eaoTodo(&aryTodoActions, aryDoomedTodos);
		}
		if(aryLabActions.GetSize() > 0) { // (z.manning 2008-10-08 17:01) - PLID 31628
			m_pEMN->GetEmrObjectsToRevoke_eaoLab(&aryLabActions, aryDoomedLabs);
		}

		// (c.haag 2008-08-05 15:48) - PLID 30799 - Gather a list of problems that will
		// be removed by iterating through the list of doomed objects
		// (c.haag 2009-05-29 16:32) - PLID 34398 - We now manage problem links
		CArray<CEmrProblemLink*,CEmrProblemLink*> aryProblemLinks;
		CDoomedEmrObjectAry aryAllDoomedObjects;
		aryAllDoomedObjects.Append(aryDoomedCpts);
		aryAllDoomedObjects.Append(aryDoomedDiags);
		aryAllDoomedObjects.Append(aryDoomedMedications);
		aryAllDoomedObjects.Append(aryDoomedProcedures);
		aryAllDoomedObjects.Append(aryDoomedItems);
		aryAllDoomedObjects.Append(aryDoomedTopics);
		aryAllDoomedObjects.Append(aryDoomedEMNs);
		aryAllDoomedObjects.Append(aryDoomedTodos);
		aryAllDoomedObjects.Append(aryDoomedLabs); // (z.manning 2008-10-08 17:02) - PLID 31628
		// Now populate the problems array with all the problems that would be removed by the act of unspawning
		FillUnspawnedProblemLinksArray(aryAllDoomedObjects, aryProblemLinks);

		// (c.haag 2008-08-04 15:17) - PLID 30942 - By this point in time, we know exactly what we're going to unspawn. Some
		// of the EMR objects may be bound to problems; in those cases, we want to warn the user that those problems are being
		// removed.
		if (!(m_nOptions & eUnspawnSuppressWarnings)) {
			
			// If any problems are being removed, display a warning for the user
			// (c.haag 2009-05-29 16:33) - PLID 34398 - Check based on links not problems
			if (aryProblemLinks.GetSize() > 0) {
				WarnOfUnspawningProblemLinks(aryProblemLinks);
			}
		}

		// (c.haag 2008-08-05 15:39) - PLID 30799 - We will need to notify all affected EMN's
		// to update their problem flag icons. Build a map of the EMN interface windows.
		// (c.haag 2009-05-29 16:33) - PLID 34398 - Check based on links not problems
		CMap<HWND,HWND,BOOL,BOOL> mapProblemWarnHwnds;
		if (aryProblemLinks.GetSize() > 0) {

			// Gather a unique list of EMN's by iterating through our problem array
			for (int iProb=0; iProb < aryProblemLinks.GetSize(); iProb++) {
				CEmrProblemLink* pLink = aryProblemLinks[iProb];
				if (NULL != pLink->GetEMN() &&
					NULL != pLink->GetEMN()->GetInterface() &&
					IsWindow(pLink->GetEMN()->GetInterface()->GetSafeHwnd())
					)
				{
					mapProblemWarnHwnds.SetAt(pLink->GetEMN()->GetInterface()->GetSafeHwnd(), TRUE);
				}
			} // for (int iProb=0; iProb < aryProblems.GetSize(); iProb++) {
		}


		//DRT 8/8/2007 - We want to lock narrative & linked table updating until all the actions have been processed.
		//TES 1/23/2008 - PLID 24157 - Renamed.
		m_pEMN->LockHandlingDetailChanges();

		//
		//And now, finally, we can revoke some actions!
		//DRT 8/7/2007 - PLID 27003 - Phase 2.  Instead of a map joining source details & all their actions, we have arrays of
		//	action dest types.  It is more efficient to batch process this way, so we can handle any recordset needed stuff
		//	all at once for the entire batch of dest types.
		// (c.haag 2008-08-01 16:50) - PLID 30897 - Use the doomed arrays for the revoking
		if(aryDoomedCpts.GetSize() > 0 || aryCodingGroupsToChange.GetSize() > 0) {
			// (j.jones 2011-07-11 10:56) - PLID 38366 - pass in the coding groups that will be affected
			m_pEMN->RevokeEmrActions_eaoCpt(aryDoomedCpts, aryCodingGroupsToChange, mapCodingGroupQuantitiesToSubtract);
		}
		if(aryDoomedDiags.GetSize() > 0) {
			m_pEMN->RevokeEmrActions_eaoDiag(aryDoomedDiags);
		}
		if(aryDoomedMedications.GetSize() > 0) {
			m_pEMN->RevokeEmrActions_eaoMedication(aryDoomedMedications);
		}
		if(aryDoomedProcedures.GetSize() > 0) {
			m_pEMN->RevokeEmrActions_eaoProcedure(aryDoomedProcedures);
		}
		if(aryDoomedItems.GetSize() > 0) {
			m_pEMN->RevokeEmrActions_eaoEmrItem(aryDoomedItems);
		}
		if(aryDoomedTopics.GetSize() > 0) {
			m_pEMN->RevokeEmrActions_eaoMintItems(aryDoomedTopics);
		}
		if(aryDoomedEMNs.GetSize() > 0) {
			//Special:  Mint's are handled at the EMR level, not the EMN level.
			m_pEMN->GetParentEMR()->RevokeEmrActions_eaoMint(aryDoomedEMNs);
		}
		if(aryDoomedTodos.GetSize() > 0) { // (c.haag 2008-06-24 10:55) - PLID 17244
			m_pEMN->RevokeEmrActions_eaoTodo(aryDoomedTodos);
		}
		// (z.manning 2008-10-08 16:49) - PLID 31613 - Revoke labs
		if(aryDoomedLabs.GetSize() > 0) {
			m_pEMN->RevokeEmrActions_eaoLab(aryDoomedLabs);
		}

		//DRT 8/8/2007 - Unlock updating -- if noone else has locked it, this will perform the needed actions.
		//TES 1/23/2008 - PLID 24157 - Renamed.
		m_pEMN->UnlockHandlingDetailChanges();

		// (c.haag 2008-08-05 15:39) - PLID 30799 - Now we need to notify all affected EMN's
		// to update their problem flag icons
		// (c.haag 2009-05-29 16:36) - PLID 34398 - Check the problem link array
		if (aryProblemLinks.GetSize() > 0) {

			// (c.haag 2009-07-09 13:07) - PLID 34293 - Go through all problems in aryProblemLinks,
			// and set them to be deleted if they have no live links
			CEMR* pEMR = m_pEMN->GetParentEMR();
			if (NULL != pEMR) {
				CArray<CEmrProblem*,CEmrProblem*> apProblems;
				CArray<CEmrProblem*,CEmrProblem*> apDoomedProblems;
				pEMR->GetAllProblems(apProblems);			
				pEMR->FindEmrProblemsToDelete(apProblems, aryProblemLinks, apDoomedProblems, FALSE);
				for (int i=0; i < apDoomedProblems.GetSize(); i++) {
					apDoomedProblems[i]->m_bIsDeleted = TRUE;					
				}
			}			

			// Now go through the map and update EMN's
			POSITION pos = mapProblemWarnHwnds.GetStartPosition();
			BOOL bDummy;
			HWND hWnd;
			while (NULL != pos) {
				mapProblemWarnHwnds.GetNextAssoc(pos, hWnd, bDummy);
				if (IsWindow(hWnd)) {
					::PostMessage(hWnd, NXM_EMR_PROBLEM_CHANGED, 0, 0);
				}
			}

		} // if (aryProblems.GetSize() > 0) {

	} // if(paryAllActions->GetSize() > 0) {
}

// (c.haag 2009-03-18 16:37) - PLID 33449 - This function ensures an element exists in an unspawning source array
void CEMNUnspawner::EnsureExists(CArray<CUnspawningSource, CUnspawningSource&> *parySources, CUnspawningSource& us)
{
	if (NULL == parySources) {
		ASSERT(FALSE);
		return;
	}
	const int nSources = parySources->GetSize();
	for (int i=0; i < nSources; i++) {
		CUnspawningSource& usAry = parySources->GetAt(i);
		// (z.manning 2009-03-24 16:31) - PLID 33450 - Use the == operator for CUnspawningSource
		if (us == usAry)
		{
			// If we get here, we have a match, so quit the function now
			return;
		}
	}
	// If we get here, there are no matches, so add the element
	parySources->Add(us);
}
