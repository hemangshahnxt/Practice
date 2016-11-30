
#pragma once
namespace Intervention
{
	enum InterventionCriteriaOperator;
}

// (j.gruber 2010-02-23 15:18) - PLID - Stored in Data, must not change
// (j.gruber 2010-02-25 09:27) - PLID 37537 - added letterwriting filters
// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
enum DecisionRuleCriterionType {
	drctAge = 1,
	drctLabResultName = 2,	
	drctLabsResultValue = 3,
	drctFilter = 4,
	drctProblemName = 5,
	drctMedicationName = 6,
	drctAllergyName = 7,	
	drctEmrItem = 8,
};

// (j.gruber 2010-02-24 09:26) - PLID 37510
// (c.haag 2010-09-21 11:35) - PLID 40612 - Removed dwLabIDs (which took in a list of added/modified LabResultID's). We now
// go through all labs for the patient. Replaced it with nPatientID.
//void TodoCreateForDecisionRules(ADODB::_Connection* lpCon, long nAssignTo, long nEnteredBy, long nPriority, CString strTaskType, long nPatientID);
// (c.haag 2010-09-21 11:35) - PLID 40612 - This is just like the larger overload but with the default parameters of
// GetCurrentUserID(), GetCurrentUserID(), ttpMedium, "".
//TES 10/31/2013 - PLID 59251 - Renamed, this no longer creates ToDos, but instead updates DecisionRuleInterventionsT, and outputs a list
// of newly added DecisionRuleInterventionT.IDs
void UpdateDecisionRules(ADODB::_Connection* lpCon, long nPatientID, OUT CDWordArray &arNewInterventions);

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
Intervention::InterventionCriteriaOperator GetDefaultOperator(DecisionRuleCriterionType drct);
// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
//TES 12/2/2013 - PLID 59532 - Added eitEmrInfoType
NXDATALIST2Lib::IFormatSettingsPtr GetOperatorFormatSettings(DecisionRuleCriterionType drct, long eitEmrInfoType);
// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
NXDATALIST2Lib::IFormatSettingsPtr GetValueFormatSettings(DecisionRuleCriterionType drct, long eitEmrInfoType, long nRecordID, Intervention::InterventionCriteriaOperator ico);
