#pragma once

#include "NxAPI.h" // Needed for NexTech_Accessor reference

namespace DiagQuickListUtils
{
	// (c.haag 2014-02-26) - PLID 60931 - Returns how codes should be displayed in the QuickList
	NexTech_Accessor::DiagDisplayType GetAPIDiagDisplayType();

	// (c.haag 2014-02-26) - PLID 60931 - Gets the diagnosis QuickList for the current user
	NexTech_Accessor::_DiagQuickListPtr GetDiagQuickListForCurrentUser();

	// (c.haag 2014-03-17) - PLID 60929 - Returns how codes should be displayed in the QuickList as an integer 
	int GetAPIDiagDisplayTypeInt();
}
