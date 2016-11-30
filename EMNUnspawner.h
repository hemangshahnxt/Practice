// EMNUnspawner.h: interface for the CEMNUnspawner class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMNUNSPAWNER_H__6834B91C_6E1E_44E4_B773_9E9C6BD06FB4__INCLUDED_)
#define AFX_EMNUNSPAWNER_H__6834B91C_6E1E_44E4_B773_9E9C6BD06FB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EMNDetailStructures.h"

// (j.jones 2013-05-16 13:58) - PLID 56596 - turned EMR/EMN/EMNDetail/EMRTopic into forward declares
class CEMR;
class CEMN;
class CEMNDetail;
class CEMRTopic;
class CEMRHotSpot;
enum EWoundCareCodingCondition;

// (a.walling 2011-08-11 16:43) - PLID 45021 - TableRow.m_pID is now TableRow.m_ID, which is not allocated on the heap.


//DRT 8/2/2007 - PLID 26914 - See overview in .cpp

class CDoomedEmrObject;

class CUnspawningSource {

private:
	ListElement le;
	// (z.manning 2011-07-25 12:24) - PLID 44649 - This is not a pointer to a hot spot
	CEMRHotSpot *pHotSpot;

	// (r.gonet 08/03/2012) - PLID 51949 - The source can be a Wound Care Coding Condition (WoundCareConditionT.ConditionID)
	EWoundCareCodingCondition wcccConditionID;

	EmrActionObject eActionSourceType;

public:
	CEMNDetail *pSourceDetail;

	// (z.manning 2009-03-04 11:11) - PLID 33072 - We now use TableDropdownSpawnInfo for table dropdown spawning
	TableSpawnInfo tdsi; // (z.manning 2009-02-23 16:01) - PLID 33138

	// (j.jones 2013-05-16 14:08) - PLID 56596 - moved the function code to the .cpp

	//Default constructor
	CUnspawningSource();

	//Use this constructor in most cases
	CUnspawningSource(CEMNDetail *pDtl, ListElement _le);

	// (z.manning, 01/23/2008) - PLID 28690 - Use this constructor 
	CUnspawningSource(CEMNDetail *pDtl, CEMRHotSpot *pSpot);

	// (z.manning 2009-02-23 15:59) - PLID 33138 - A consructor for table dropdown actions
	CUnspawningSource(CEMNDetail *pDtl, TableSpawnInfo _tdsi);

	// (r.gonet 08/03/2012) - PLID 51949 - Create a new unspawning source for
	//  a particular Wound Care Coding Condition
	CUnspawningSource(CEMNDetail *pDtl, EWoundCareCodingCondition _wcccConditionID);

	EmrActionObject GetActionSourceType();

	long GetSourceID();

	BOOL HasSameGroupID(CEMNDetail *pDetail);

	BOOL HasSameGroupID(CEMRTopic *pTopic);

	// (z.manning 2009-03-24 16:26) - PLID 33450 - Added an equality operator
	bool operator==(CUnspawningSource& usSource);
};



//Possible options when performing an unspawn
enum eUnspawningOption {
	//DRT 8/3/2007 - Added for PLID 26938.  When we change the type of a detail on the fly, we
	//	want to revoke all its data actions.  But the info actions remain, because the item 
	//	wasn't removed.
	eUnspawnIgnoreInfoActions = 0x01,
	//DRT 8/17/2007 - This was a misunderstanding on my part of how the old behavior worked.  On a template, 
	//	ONLY if you right click and remove a detail or topic should it wipe out the remembered positions.  If
	//	you are unspawning by unchecking a box, it should retain them.  This is all controlled by the
	//	FlagDetailsAsPendingDelete function, which sets a flag on the topic, that is later checked during
	//	the unspawn itself (CEMN::RevokeEmrAction_xxxxx).
	eUnspawnDoNotMarkPendingDelete = 0x02,
	// (c.haag 2008-08-04 15:15) - PLID 30942 - Silent option. It's possible for EMR problems to be removed during
	// the unspawning process. By default, we want to warn the user of the problems that will be removed. If the
	// silent option is turned on, then the warnings will be suppressed; but the unspawning will continue as if the
	// user had acknowledged the warnings.
	eUnspawnSuppressWarnings = 0x04,
};



class CEMNUnspawner  
{
public:
	CEMNUnspawner(CEMN* pEMN);
	virtual ~CEMNUnspawner();


	//Exposed functions.  See comments in .cpp for use.
	void RemoveActionsByTopics(IN CArray<CEMRTopic*, CEMRTopic*> *paryTopics);
	void RemoveActionsByDetails(IN CArray<CEMNDetail*, CEMNDetail*> *paryDetails);
	void RemoveActionsByDataElements(IN CArray<ListElement, ListElement&> *paryListElements, IN CEMNDetail *pSourceDetail);
	// (z.manning, 01/23/2008) - PLID 28690 - Added the ability to revoke EMR image hot spot actions.
	// (z.manning 2011-07-25 12:10) - PLID 44649 - Changed type to CEMRHotSpotArray
	void RemoveActionsByImageHotSpots(IN CEMRHotSpotArray *paryHotSpots, IN CEMNDetail *pSourceDetail);
	// (z.manning 2009-02-23 16:02) - PLID 33138 - For table dropdown item based spawning
	void RemoveActionsByTableDropdownItems(IN CArray<TableSpawnInfo,TableSpawnInfo&> *parynDropdownItems, IN CEMNDetail *pSourceDetail);
	// (r.gonet 08/03/2012) - PLID 51949 - Removes actions performed from Wound Care Coding Conditions
	void RemoveActionsByWoundCareCalculator(IN CEMNDetail *pSourceDetail);
	// (z.manning 2010-03-02 16:28) - PLID 37571 - For smart stamp spawning
	void RemoveActionsBySmartStamps(IN CArray<TableSpawnInfo,TableSpawnInfo&> *parynTableSpawnInfo, IN CEMNDetail *pSourceDetail);

	//Flags to be followed while performing the revoking
	void SetOption(eUnspawningOption euo, bool bOn);

	//DRT 9/27/2007 - PLID 27515 - Given a detail and an unspawning source, see if any of the parent topics (looking
	//	up the chain to the top) were spawned by this unspawning source.
	bool HierarchicalParentTopicSpawnedByThisSource(IN CEMNDetail *pDetail, IN CUnspawningSource *pus);


protected:

	//////////////////////////////////
	//		Member variables		//
	//////////////////////////////////

	//There must be 1 EMN for this unspawning to happen
	CEMN *m_pEMN;

	//Any options that we have chosen
	long m_nOptions;


	//////////////////////////////////////
	//		Recursion Helper Functions	//
	//////////////////////////////////////

	void RecursivelyGetDetailsFromTopic(IN CEMRTopic *pTopic, OUT CArray<CEMNDetail*, CEMNDetail*> *paryDetails);
	void RecursivelyGather(IN OUT CArray<CUnspawningSource, CUnspawningSource&> *parySources, IN OUT CArray<CEMNDetail*, CEMNDetail*> *paryDetailsWithInfoSpawns);


	//////////////////////////////////////
	//		General Helper Functions	//
	//////////////////////////////////////

	// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray

	void RecursivelyUnspawn(IN OUT CArray<CUnspawningSource, CUnspawningSource&> *parySources, IN OUT CArray<CEMNDetail*, CEMNDetail*> *paryDetailsWithInfoSpawns);
	void FlagDetailsAsPendingDelete(IN const CArray<CEMNDetail*, CEMNDetail*> *paryDetails);
	void FillInfoSpawnArrayFromDetailArray(OUT CArray<CEMNDetail*, CEMNDetail*> *paryDetailsWithInfoSpawns, IN const CArray<CEMNDetail*, CEMNDetail*> *paryDetails);
	void FillSourcesArrayFromDetails(OUT CArray<CUnspawningSource, CUnspawningSource&> *parySources, IN CArray<CEMNDetail*, CEMNDetail*> *paryDetails);
	void FillEMRActionArray(IN CArray<CUnspawningSource, CUnspawningSource&> *parySources, IN CArray<CEMNDetail*, CEMNDetail*> *paryDetailsWithInfoSpawns, OUT MFCArray<EmrAction> *paryActions);
	void ProcessRevoking(IN CArray<CUnspawningSource, CUnspawningSource&> *parySources, IN OUT CArray<CEMNDetail*, CEMNDetail*> *paryDetailsWithInfoSpawns, IN OUT MFCArray<EmrAction> *paryAllActions);
	// (c.haag 2009-03-18 16:37) - PLID 33449 - This function ensures an element exists in an unspawning source array
	void EnsureExists(CArray<CUnspawningSource, CUnspawningSource&> *parySources, CUnspawningSource& us);
};

#endif // !defined(AFX_EMNUNSPAWNER_H__6834B91C_6E1E_44E4_B773_9E9C6BD06FB4__INCLUDED_)
