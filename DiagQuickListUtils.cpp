#include "stdafx.h"
#include "DiagQuickListUtils.h"
#include "DiagSearchUtils.h"
#include "NxAPI.h"

namespace DiagQuickListUtils
{
	// (c.haag 2014-02-26) - PLID 60931 - Returns how codes should be displayed in the QuickList
	NexTech_Accessor::DiagDisplayType GetAPIDiagDisplayType()
	{
		switch (DiagSearchUtils::GetPreferenceSearchStyle())
		{
		case eICD9_10_Crosswalk:
			return NexTech_Accessor::DiagDisplayType_Crosswalk;
		case eManagedICD9_Search:
			return NexTech_Accessor::DiagDisplayType_ICD9;
		case eManagedICD10_Search:
			return NexTech_Accessor::DiagDisplayType_ICD10;
		default:
			ASSERT(FALSE);
			return NexTech_Accessor::DiagDisplayType_Crosswalk;
		}
	}

	// (c.haag 2014-03-17) - PLID 60929 - Returns how codes should be displayed in the QuickList as an integer 
	int GetAPIDiagDisplayTypeInt()
	{
		NexTech_Accessor::DiagDisplayType displayType = GetAPIDiagDisplayType();
		switch (displayType)
		{
		case NexTech_Accessor::DiagDisplayType_ICD9:
				return 0;
		case NexTech_Accessor::DiagDisplayType_Crosswalk:
				return 1;
		case NexTech_Accessor::DiagDisplayType_ICD10:
				return 2;
		default:
			ASSERT(FALSE);
			return 0;
		}
	}

	// (c.haag 2014-02-26) - PLID 60931 - Gets the diagnosis QuickList for the current user
	NexTech_Accessor::_DiagQuickListPtr GetDiagQuickListForCurrentUser()
	{
		// (r.farnworth 2014-07-14 12:37) - PLID 62816 - Added alwaysIncludeStandalone
		// (r.farnworth 2014-07-21 17:04) - PLID 62816 - Flag needs to be VARIANT_TRUE
		NexTech_Accessor::_NullableBoolPtr pNullBool(__uuidof(NexTech_Accessor::NullableBool));
		pNullBool->SetBool(VARIANT_TRUE);
		return GetAPI()->GetDiagQuickListForCurrentUser(GetAPISubkey(), GetAPILoginToken(), GetAPIDiagDisplayType(), pNullBool);
	}
}