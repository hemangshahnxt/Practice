
#pragma once

#include "SharedEmrUtils.h"

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS

namespace Intervention
{
	//TES 5/20/2009 - PLID 34302 - Stored in data, MUST NOT CHANGE
	// (j.gruber 2010-02-25 09:27) - PLID 37537 add IN operators
	enum InterventionCriteriaOperator {
		icoEqual = 1,
		icoNotEqual = 2,
		icoGreaterThan = 3,
		icoLessThan = 4,
		icoGreaterThanOrEqual = 5,
		icoLessThanOrEqual = 6,
		icoContains = 7,
		icoDoesNotContain = 8,
		icoFilledIn = 9,
		icoNotFilledIn = 10,
		icoExists = 11,
		icoDoesNotExist = 12,
		icoIsIn = 13,		//CDS ONLY
		icoIsNotIn = 14,	//CDS ONLY
	};

	enum InterventionListColumns
	{
		ilcID = 0,
		ilcName = 1,
		ilcToDoMessage = 2,
	};

	enum AvailableCriteriaListColumns
	{
		aclcName = 0,
		aclcTypeID = 1,
		aclcType = 2,
		aclcRecordID = 3, //TES 6/8/2009 - PLID 34509 - Renamed
		aclcEmrInfoType = 4,
	};

	enum CriteriaListColumns
	{
		clcID = 0,
		clcName = 1,
		clcType = 2,
		clcTypeID = 3,
		clcRecordID = 4, //TES 6/8/2009 - PLID 34509 - Renamed
		clcOperator = 5,
		clcValue = 6, //TES 5/22/2009 - PLID 34302 - The actual Value in data
		clcDisplayValue = 7, //TES 5/22/2009 - PLID 34302 - The visible representation of the value, 
							//same as clcValue unless the row is a Multi-Select EMR item.
		clcLastXDays = 8,
		clcEmrInfoType = 9,
	};
}