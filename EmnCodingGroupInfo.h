#pragma once

// (z.manning 2011-07-07 12:58) - PLID 44469 - Simply class to track a link between coding group data for an EMN
class CEmnCodingGroupInfo
{
public:
	
	long m_nEmrCodingGroupID;
	long m_nGroupQuantity;

	// (j.jones 2011-07-11 14:25) - PLID 44509 - added booleans to track if this group is new or modified,
	// as well as m_nOldQuantity, used for auditing purposes
	BOOL m_bIsNew;
	BOOL m_bIsModified;
	long m_nOldQuantity;

	CEmnCodingGroupInfo::CEmnCodingGroupInfo()
	{
		m_nEmrCodingGroupID = -1;
		m_nGroupQuantity = -1;
		
		// (j.jones 2011-07-11 14:25) - PLID 44509 - added booleans to track if this group is new or modified,
		// as well as m_nOldQuantity, used for auditing purposes
		m_bIsNew = TRUE;
		m_bIsModified = TRUE;
		m_nOldQuantity = -1;
	}
};