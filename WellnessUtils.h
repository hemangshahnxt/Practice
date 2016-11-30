//TES 5/20/2009 - PLID 34302 - Created, stores enums and utility functions
#pragma once

// (b.cardillo 2009-07-09 09:30) - PLID 34369 - Split data-related functionality out of WellnessUtils so that the 
// business logic can be shared with other projects (such as NxWeb.dll)
enum WellnessTemplateCriterionType;
enum WellnessTemplateCriteriaOperator;
enum EmrInfoType;

// (j.gruber 2009-06-01 08:40) - PLID 34401 - define for previewing
#define ID_PREVIEW_REPORT 45487

//TES 5/20/2009 - PLID 34302 - Returns format settings which will generate the appropriate 
// embedded dropdown list of operators for the given type.
NXDATALIST2Lib::IFormatSettingsPtr GetOperatorFormatSettings(WellnessTemplateCriterionType wtct);

//TES 5/20/2009 - PLID 34302 - Similarly for the value, which will be different based on the type of criteria, as
// well as the EmrInfoType, if any.
//TES 6/2/2009 - PLID 34302 - Added a parameter for the operator, multiselects are hyperlinked if they have a transitive
// operator, but not if they have an intransitive operator.
//TES 6/8/2009 - PLID 34509 - Renamed EmrInfoMasterID to RecordID
NXDATALIST2Lib::IFormatSettingsPtr GetValueFormatSettings(WellnessTemplateCriterionType wtct, EmrInfoType eit, long nRecordID, WellnessTemplateCriteriaOperator wtco);

//TES 5/22/2009 - PLID 34302 - Returns the default operator for a given type of criterion.
WellnessTemplateCriteriaOperator GetDefaultOperator(WellnessTemplateCriterionType wtct);

