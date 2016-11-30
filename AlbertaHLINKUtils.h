//AlbertaHLINKUtils.h
//

#ifndef ALBERTAHLINKUTILS_H
#define ALBERTAHLINKUTILS_H

#pragma once


// (j.jones 2010-11-03 15:04) - PLID 39620 - created
bool UseAlbertaHLINK();

// (j.jones 2014-07-28 13:06) - PLID 62947 - enums for special Alberta billing
// statuses, these are not stored in data
enum AlbertaBillingStatus {
	eEntered = 1,
	ePending,
	ePosted,
	ePartiallyPaid,
	eRejected,
	//any additions to this enumeration requires code to change in
	//EnsureAlbertaBillingStatuses and CreateAlbertaBillingStatuses
};

// (j.jones 2014-07-28 13:06) - PLID 62947 - hardcoded names for each enum
#define ALBERTA_ENTERED_STATUS_NAME			"Entered"
#define ALBERTA_PENDING_STATUS_NAME			"Pending"
#define ALBERTA_POSTED_STATUS_NAME			"Posted"
#define ALBERTA_PARTIALLY_PAID_STATUS_NAME	"Partially Paid"
#define ALBERTA_REJECTED_STATUS_NAME		"Rejected"

// (j.jones 2014-07-28 13:08) - PLID 62947 - static global map of AlbertaBillingStatus enums
// to BillStatusT record IDs
static boost::container::flat_map<AlbertaBillingStatus, int> g_mapAlbertaBillingStatusToID;

// (j.jones 2014-07-28 13:10) - PLID 62947 - Ensures g_mapAlbertaBillingStatusToID is filled
// with the correct BillStatusT.IDs, will auto-create all required statuses if they do not exist.
BOOL EnsureAlbertaBillingStatuses();

// (j.jones 2014-07-28 13:47) - PLID 62947 - creates Alberta statuses in data, as needed
void CreateAlbertaBillingStatuses();

// (j.jones 2014-07-28 13:12) - PLID 62947 - given a status enum, will return the proper BillStatusT.ID
// that represents that status
long GetAlbertaBillStatusID(AlbertaBillingStatus eStatus);

//TES 9/24/2014 - PLID 62782 - Added
CString GetAlbertaBillStatusName(AlbertaBillingStatus eStatus);

CString GetAlbertaHLINKHealthTitle();

void ImportAlbertaCodes();

// (j.jones 2011-10-10 17:02) - PLID 44941 - Enumerated action types.
// These are stored in data and cannot be changed!
enum AlbertaHLINKModifierActionType
{
	amatUnknown = -1,
	amatReplaceBase = 0,
	amatIncreaseBy = 1,
	amatForEachCallPayBaseAt = 2,
	amatForEachCallIncreaseBy = 3,
	amatReduceBaseTo = 4,
	amatIncreaseBaseBy = 5,
	amatIncreaseBaseTo = 6,
	amatMAX = 7,
	amatReduceBaseBy = 8,
	amatReplace = 9,
};

// (j.jones 2011-10-10 17:58) - PLID 44941 - changed the return value
AlbertaHLINKModifierActionType EnumerateActionType(CString strActionType);

// (d.singleton 2011-10-27 15:58) - PLID below function is to check the line of text to see if its the title of the next chapter and to skip that line
BOOL IsChapterTitle(CString strTrimedString);

#endif	//ALBERTAHLINKUTILS_H