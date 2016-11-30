#pragma once

#include "EmrUtils.h"

// (j.jones 2014-07-29 10:09) - PLID 63085 - forward declarations
// in the Accessor namespaces allow us to remove .h includes
namespace NexTech_Accessor
{
	enum NexGEMDiagCode_MatchStatus;
}

// implementation in EmrUtils.cpp

// (r.farnworth 2014-03-06 11:53) - PLID 60820 - Takes the match status from the API and maps it to the enum we have in Practice
NexGEMMatchType MapMatchStatus(NexTech_Accessor::NexGEMDiagCode_MatchStatus matchStatus);

// (b.savon 2014-03-06 09:24) - PLID 60824 - SPAWN - Set the colors and behavior of the ICD-10 columns for spawned diagnosis codes in the diag codes list in the <Codes> topic.
NexGEMMatchColor GetDiagnosisNexGEMMatchColor(long nDiagCodeID_ICD10, NexGEMMatchType nxgmMatchType);
CString GetNexGEMDisplayText(long nDiagCodeID_ICD10, const CString &strDiagCodeDescription_ICD10, NexGEMMatchType nxgmMatchType);

