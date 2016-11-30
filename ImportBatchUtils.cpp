
#include "stdafx.h"
#include "ImportBatchUtils.h"
#include "ImportUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// (b.savon 2015-03-20 15:10) - PLID 65153 - Add Race batch statement
void AddRaceStatementToBatch(CString &strSqlBatch, CString strFields, CString strValues)
{
	if (!strFields.IsEmpty()){
		//Trim our trailing spaces and comma
		strFields.TrimRight();
		strFields.TrimRight(",");
		strValues.TrimRight();
		strValues.TrimRight(",");

		AddStatementToSqlBatch(strSqlBatch,
			R"(
SET @RaceID = %s
INSERT INTO PersonRaceT (PersonID, %s)
VALUES(@PersonID, @RaceID)
							
						)", strValues, strFields
		);
	}
}

// (b.savon 2015-03-23 11:55) - PLID 65158 - Add Referral Source Name batch statement for MultiReferralT
void AddMultiReferalSourceToBatch(CString &strSqlBatch, CString strFields, CString strValues)
{
	if (!strFields.IsEmpty()){
		//Trim our trailing spaces and comma
		strFields.TrimRight();
		strFields.TrimRight(",");
		strValues.TrimRight();
		strValues.TrimRight(",");

		AddStatementToSqlBatch(strSqlBatch,
			R"(
SET @ReferralSourceID = %s
INSERT INTO MultiReferralsT (%s, PatientID, Date)
VALUES(@ReferralSourceID, @PersonID, GETDATE())
							
						)", strValues, strFields
		);
	}
}

// (b.savon 2015-03-25 08:22) - PLID 65144 - Add Custom Fields query batch statement
void AddCustomFieldsToBatch(CString &strSqlBatch, CString strQuery)
{
	if (!strQuery.IsEmpty()){
		AddStatementToSqlBatch(strSqlBatch, "%s", strQuery);
	}
}