#include "stdafx.h"
#include "DecisionRuleUtils.h"
#include "DateTimeUtils.h"
#include "GlobalParamUtils.h"
#include "filter.h"
#include "groups.h"
#include "InterventionUtils.h"

using namespace ADODB;
using namespace Intervention;

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
// (a.wilson 2014-09-04 10:56) - PLID 63535 - Changed Key variable to prevent saving address of string but actual string data.
namespace
{
	CMap<CString, LPCTSTR, NXDATALIST2Lib::IFormatSettingsPtr, NXDATALIST2Lib::IFormatSettingsPtr> g_mapComboSourceToOperatorFormatSettings;
	CMap<CString, LPCTSTR, NXDATALIST2Lib::IFormatSettingsPtr, NXDATALIST2Lib::IFormatSettingsPtr> g_mapComboSourceToValueFormatSettings;
}

// (j.gruber 2010-02-24 09:28) - PLID 37510
// (c.haag 2010-09-21 11:35) - PLID 40612 - Removed dwLabIDs (which took in a list of added/modified LabResultID's). We now
// go through all labs for the patient. Replaced it with nPatientID.
//TES 10/31/2013 - PLID 59251 - Renamed, this no longer creates ToDos, but instead updates DecisionRuleInterventionsT, and outputs a list
// of newly added DecisionRuleInterventionT.IDs
void UpdateDecisionRules(ADODB::_Connection* lpCon, long nPatientID, IN OUT CDWordArray &arNewInterventions)
{
	try {

		// (j.gruber 2010-02-25 16:07) - PLID 37510 - if the don't have emr, don't bother checking
		// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
		if (!g_pLicense->HasEMR(CLicense::cflrSilent)) {
			return;
		}

		_ConnectionPtr pCon(lpCon);
		// (c.haag 2010-09-21 11:35) - PLID 40612 - No sense in running the big query if we have no decision rules.
		//TES 11/27/2013 - PLID 59848 - Check Inactive flag
		if (!ReturnsRecords(pCon, "SELECT TOP 1 ID FROM DecisionRulesT WHERE Inactive = 0")) {
			return;
		}

		// (j.armen 2014-01-27 16:23) - PLID 60490 - Create a single param batch
		CParamSqlBatch sqlBatch;
		sqlBatch.Declare("SET NOCOUNT ON");
		sqlBatch.Declare("DECLARE @TempDecisionRuleFiltersT TABLE(CriterionID INT PRIMARY KEY, Filter NVARCHAR(MAX))");

		// (j.armen 2014-01-27 16:23) - PLID 60490 - store our XML document of rules to follow
		CString strRulesXml;
		//now loop through our criterion and see if there are any filters
		_RecordsetPtr rsFilters = CreateParamRecordset(pCon, "SELECT DecisionRulesCriterionT.ID, FiltersT.Name, FiltersT.ID as FilterID, Filter FROM "
			" (SELECT ID, Value FROM DecisionRulesCriterionT WHERE Type = 4) DecisionRulesCriterionT "
			" LEFT JOIN FiltersT on CONVERT(int, DecisionRulesCriterionT.Value) = FiltersT.ID ");
		while (!rsFilters->eof) {

			long nID = AdoFldLong(rsFilters, "ID");
			long nFilterID = AdoFldLong(rsFilters, "FilterID");
			CString strFilter = AdoFldString(rsFilters, "Filter");
			CString strName  = AdoFldString(rsFilters, "Name");
			CString strFilterWhere, strFilterFrom;

			//we only support person filters
			if(!CFilter::ConvertFilterStringToClause(nFilterID, strFilter, fboPerson, &strFilterWhere, &strFilterFrom)) {
				//something went wrong, fail
				MsgBox("Report could not be generated because it uses an invalid filter. \nNo Clinical Decision Support Rules will be checked for this lab.");
				return;
			}

			strFilter.Format(" (Select PersonT.ID AS PersonID FROM %s WHERE %s GROUP BY PersonT.ID)", strFilterFrom, strFilterWhere); 
			if (strFilter.GetLength() > 4000) {
				//too big, kick them out
				MsgBox("The filter '%s' is too large to use for Clinical Decision Support Rules, please select another filter.  \nNo Clinical Decision Support Rules will be checked for this lab.", strName);
				return;
			}

			// (j.armen 2014-01-27 16:23) - PLID 60490 - Instead of adding to the sql batch, add to our xml document
			strRulesXml += FormatString("<row ID='%li' Filter='%s'/>\r\n", nID, XMLEncode(strFilter));

			rsFilters->MoveNext();
		}

		// (j.armen 2014-01-27 16:23) - PLID 60490 - Load the rules into our table variable
		// (j.armen 2014-02-14 13:09) - PLID 60841 - Added a root node
		sqlBatch.Declare("DECLARE @RulesXML XML");
		sqlBatch.Add("SET @RulesXML = {STRING}", FormatString("<root>%s</root>", strRulesXml));
		sqlBatch.Add("INSERT INTO @TempDecisionRuleFiltersT (CriterionID, Filter)\r\n"
			"SELECT\r\n"
			"	c.value('@ID', 'INT'),\r\n"
			"	c.value('@Filter', 'NVARCHAR(MAX)')\r\n"
			"FROM @RulesXML.nodes('root/row') T(c)");

		// (c.haag 2010-09-23 16:05) - PLID 40663 - We now union the lab results query on a NULL value so that
		// we can test rules on patients who have no labs. The consequence of the NULL is the presence of a NULL
		// lab result name and value; those values are specially handled.
		//TES 10/31/2013 - PLID 59251 - Updated to add records to DecisionRuleInterventionsT, rather than ToDoList
		//TES 11/13/2013 - PLID 59481 - This would end up throwing exceptions when comparing lab results with non-numeric results, I fixed it to cast
		// the result as a number before comparing (just looking at the first characters until finding something that isn't a number or '.')
		//TES 11/14/2013 - PLID 59503 - EMR Problem List criteria were actually checking diagnosis codes, not EMR problems. I changed it to look at EMR problems
		//TES 11/13/2013 - PLID 59481 - Fixed issue that was causing lab results to look at results for all patients
		//TES 11/15/2013 - PLID 59525 - Added Contains and Does Not Contain operators for result name, medication, allergy, and problem
		//TES 11/15/2013 - PLID 59526 - Fixed bug where medications were only checking prescribed meds and not current meds
		//TES 11/15/2013 - PLID 59532 - It wasn't actually checking EMR Items up until now, I implemented that
		//TES 11/21/2013 - PLID 59680 - In certain cases, absence of data was satisfying any criteria (i.e., a patient with no allergies 
		// was satisfying all allergy-based criteria)
		//TES 11/21/2013 - PLID 59684 - Don't regenerate interventions for the same rule for the same patient, even if the previous intervention was acknowledged.
		//TES 11/22/2013 - PLID 59680 - Fixed NULL birthdates triggering Age criteria
		//TES 11/27/2013 - PLID 59848 - Filter out Inactive rules
		//TES 12/2/2013 - PLID 59532 - Treat text-type EMR Items as text rather than numbers
		//TES 12/2/2013 - PLID 59532 - For list-type items, check the actual selections rather than trying to convert their value
		// to a number
		//TES 12/3/2013 - PLID 59532 - Fixed issue with trying to convert textual EMR Item criterion values to numbers
		//TES 12/17/2013 - PLID 59532 - Supported the Is Filled In / Is Not Filled In operators
		//TES 1/14/2014 - PLID 60081 - Made it so that if a criterion is comparing an EMR text item, and both the value of the item
		// and the criterion value are completely numeric, then the comparison will be numeric rather than textual
		// (j.armen 2014-01-27 16:23) - PLID 60490 - Broke this into several statements with the param batch
		sqlBatch.Declare("DECLARE @nPatientID INT, @dtBirthDate DATETIME, @nPtAge INT, @rules XML");
		sqlBatch.Add("SET @nPatientID = {INT}", nPatientID);

		sqlBatch.Add("SET @dtBirthDate = (SELECT BirthDate FROM PersonT WHERE ID = @nPatientID)");

		sqlBatch.Add("SET @nPtAge = (SELECT (YEAR(GETDATE()) - YEAR(@dtBirthDate) - \r\n"
			"	CASE WHEN MONTH(@dtBirthDate) > MONTH(GETDATE()) THEN 1 \r\n"
			"	WHEN MONTH(@dtBirthDate) < MONTH(GETDATE()) THEN 0 \r\n"
			"	WHEN DAY(@dtBirthDate) > DAY(GETDATE()) THEN 1 \r\n"
			"	ELSE 0 END)) \r\n");

		sqlBatch.Add(
			"SET @rules = \r\n"
			"(SELECT DecisionRulesT.ID AS '@ID', \r\n"
				"(SELECT ' OR ' + \r\n"
				"CASE DecisionRulesCriterionT.[Type] \r\n"
					"WHEN 1 THEN CASE WHEN @nptAge Is Null THEN 'Null' ELSE CONVERT(nVarchar, @nptAge) END \r\n"
					"WHEN 2 THEN CASE WHEN LabResultsT.Name Is Null THEN 'Null' ELSE '''' + REPLACE(LabResultsT.Name, '''', '''''') + '''' END \r\n"
					"WHEN 3 THEN CASE WHEN COALESCE(LabResultsT.Value,'') = '' THEN 'Null' ELSE CASE WHEN patindex('%[^0-9.]%', LabResultsT.Value) = 0 THEN LabResultsT.Value WHEN patindex('%[^0-9.]%', LabResultsT.Value) = 1 THEN 'Null' ELSE Left(LabResultsT.Value, patindex('%[^0-9.]%',LabResultsT.Value)-1) END END\r\n"
					"WHEN 4 THEN  CONVERT(nVarchar, PersonT.ID) \r\n"
					"WHEN 5 THEN CASE WHEN EmrProblemsT.Description Is Null THEN 'Null' ELSE '''' + REPLACE(EmrProblemsT.Description, '''', '''''') + '''' END \r\n"
					"WHEN 6 THEN CASE WHEN DrugDataT.Data Is Null THEN 'Null' ELSE '''' + REPLACE(DrugDataT.Data, '''', '''''') + '''' END \r\n"
					"WHEN 7 THEN CASE WHEN AllergyDataT.Data Is Null THEN 'Null' ELSE '''' + REPLACE(AllergyDataT.Data, '''', '''''') + '''' END \r\n"
					"WHEN 8 THEN CASE WHEN COALESCE(EmrDetailValuesQ.Value,'') = '' THEN 'Null' ELSE "
						"CASE WHEN EmrDetailValuesQ.DataType = 1 THEN CASE WHEN patindex('%[^0-9.]%', EmrDetailValuesQ.Value) = 0 AND patindex('%[^0-9.]%', DecisionRulesCriterionT.Value) = 0 THEN \r\n"
						"	CASE WHEN Len(EmrDetailValuesQ.Value) = 0 THEN \r\n"
						"		'Null'\r\n"
						"			ELSE\r\n"
						"				EmrDetailValuesQ.Value \r\n"
						"			END\r\n"
						"		ELSE\r\n"
						"			'''' + REPLACE(EmrDetailValuesQ.Value, '''', '''''') + ''''\r\n"
						"			END ELSE \r\n"
							"CASE WHEN patindex('%[^0-9.]%', EmrDetailValuesQ.Value) = 0 THEN EmrDetailValuesQ.Value WHEN patindex('%[^0-9.]%', EmrDetailValuesQ.Value) = 1 THEN 'Null' ELSE Left(EmrDetailValuesQ.Value, patindex('%[^0-9.]%',EmrDetailValuesQ.Value)-1) END "
						"END "
					"END\r\n"
				"END \r\n"
				"+ \r\n"
				"CASE DecisionRulesCriterionT.Operator \r\n"
					"WHEN 1 THEN ' = ' \r\n"
					"WHEN 2 THEN ' <> ' \r\n"
					"WHEN 3 THEN ' > ' \r\n"
					"WHEN 4 THEN ' < ' \r\n"
					"WHEN 5 THEN ' >= ' \r\n"
					"WHEN 6 THEN ' <= ' \r\n"
					"WHEN 7 THEN ' LIKE ' \r\n"
					"WHEN 8 THEN ' NOT LIKE ' \r\n"
					"WHEN 9 THEN ' Is Not Null ' \r\n"
					"WHEN 10 THEN ' Is Null ' \r\n"
					"WHEN 13 THEN ' IN ' \r\n"
					"WHEN 14 THEN ' NOT IN ' \r\n"
				"END \r\n"
				"+ \r\n"
				"CASE DecisionRulesCriterionT.[Type] \r\n"
					"WHEN 1 THEN CONVERT(nVarChar, CONVERT(int, DecisionRulesCriterionT.Value)) \r\n"
					"WHEN 2 THEN '''' + CASE WHEN DecisionRulesCriterionT.Operator IN (7,8) THEN '%' ELSE '' END + REPLACE(DecisionRulesCriterionT.Value, '''', '''''') + CASE WHEN DecisionRulesCriterionT.Operator IN (7,8) THEN '%' ELSE '' END + '''' \r\n"
					"WHEN 3 THEN CONVERT(nVarChar, CONVERT(float, DecisionRulesCriterionT.Value)) \r\n"
					"WHEN 4 THEN \r\n"
						"CASE DecisionRulesCriterionT.Operator \r\n"
							"WHEN 13 THEN '(' + (SELECT CONVERT(nVarChar(4000), Filter) FROM @TempDecisionRuleFiltersT WHERE CriterionID = DecisionRulesCriterionT.ID)  + ')' \r\n"
							"WHEN 14 THEN '(' + (SELECT CONVERT(nVarChar(4000), Filter) FROM @TempDecisionRuleFiltersT WHERE CriterionID = DecisionRulesCriterionT.ID)  + ')' \r\n"
						"END \r\n"
					"WHEN 5 THEN '''' + CASE WHEN DecisionRulesCriterionT.Operator IN (7,8) THEN '%' ELSE '' END + REPLACE(DecisionRulesCriterionT.Value, '''', '''''') + CASE WHEN DecisionRulesCriterionT.Operator IN (7,8) THEN '%' ELSE '' END + '''' \r\n"
					"WHEN 6 THEN '''' + CASE WHEN DecisionRulesCriterionT.Operator IN (7,8) THEN '%' ELSE '' END + REPLACE(DecisionRulesCriterionT.Value, '''', '''''') + CASE WHEN DecisionRulesCriterionT.Operator IN (7,8) THEN '%' ELSE '' END + '''' \r\n"
					"WHEN 7 THEN '''' + CASE WHEN DecisionRulesCriterionT.Operator IN (7,8) THEN '%' ELSE '' END + REPLACE(DecisionRulesCriterionT.Value, '''', '''''') + CASE WHEN DecisionRulesCriterionT.Operator IN (7,8) THEN '%' ELSE '' END + '''' \r\n"
					"WHEN 8 THEN \r\n"
					"	CASE WHEN DecisionRulesCriterionT.Operator IN (9,10) THEN \r\n"
					"		'' \r\n"
					"	ELSE \r\n"
					"	CASE WHEN EmrDetailValuesQ.DataType = 1 THEN \r\n"
					"		CASE WHEN DecisionRulesCriterionT.Operator IN (7,8) THEN\r\n"
					"			'''' + '%' + REPLACE(DecisionRulesCriterionT.Value, '''', '''''') + '%' + ''''\r\n"
					"		ELSE\r\n"
					"			CASE WHEN patindex('%[^0-9.]%', EmrDetailValuesQ.Value) = 0 AND patindex('%[^0-9.]%', DecisionRulesCriterionT.Value) = 0 THEN \r\n"
					"				CASE WHEN Len(DecisionRulesCriterionT.Value) = 0 THEN\r\n"
					"					'Null'\r\n"
					"				ELSE\r\n"
					"					DecisionRulesCriterionT.Value \r\n"
					"				END\r\n"
					"			ELSE\r\n"
					"				'''' + REPLACE(DecisionRulesCriterionT.Value, '''', '''''') + ''''\r\n"
					"			END\r\n"
					"		END\r\n"
					"	ELSE \r\n"
					"		CASE WHEN patindex('%[^0-9.]%', DecisionRulesCriterionT.Value) = 0 THEN \r\n"
					"			CASE WHEN Len(DecisionRulesCriterionT.Value) = 0 THEN \r\n"
					"				'Null' \r\n"
					"			ELSE \r\n"
					"				DecisionRulesCriterionT.Value \r\n"
					"			END \r\n"
					"		WHEN patindex('%[^0-9.]%', DecisionRulesCriterionT.Value) = 1 THEN \r\n"
					"			'Null' \r\n"
					"		ELSE \r\n"
					"			Left(DecisionRulesCriterionT.Value, patindex('%[^0-9.]%',DecisionRulesCriterionT.Value)-1) \r\n"
					"		END \r\n"
					"	END \r\n"
					"END\r\n"
				"END \r\n"
				"AS Condition \r\n"
				"FROM PersonT \r\n"
				"LEFT JOIN LabsT ON LabsT.PatientID = PersonT.ID \r\n"
				"LEFT JOIN LabResultsT \r\n"
					"ON LabResultsT.LabID = LabsT.ID AND DecisionRulesCriterionT.[Type] IN (2,3) \r\n"
					"LEFT JOIN (SELECT PatientID, MedicationID FROM PatientMedications UNION SELECT PatientID, MedicationID FROM CurrentPatientMedsT) AS PatientMedications \r\n"
					"ON PatientMedications.PatientID = PersonT.ID AND DecisionRulesCriterionT.[Type] = 6 \r\n"
				"LEFT JOIN DrugList \r\n"
					"ON DrugList.ID = PatientMedications.MedicationID \r\n"
				"LEFT JOIN EmrDataT DrugDataT \r\n"
					"ON DrugDataT.ID = DrugList.EmrDataID \r\n"
				"LEFT JOIN PatientAllergyT \r\n"
					"ON PatientAllergyT.PersonID = PersonT.ID AND DecisionRulesCriterionT.[Type] = 7 \r\n"
				"LEFT JOIN AllergyT \r\n"
					"ON AllergyT.ID = PatientAllergyT.AllergyID \r\n"
				"LEFT JOIN EmrDataT AllergyDataT \r\n"
					"ON AllergyDataT.ID = AllergyT.EmrDataID \r\n"
				"LEFT JOIN EmrProblemsT \r\n"
					"ON EmrProblemsT.PatientID = PersonT.ID AND DecisionRulesCriterionT.[Type] = 5 \r\n"
				"LEFT JOIN (SELECT EmrMasterT.PatientID, Convert(nvarchar(2000), EmrDetailsT.Text) AS Value, EmrInfoT.EmrInfoMasterID, \r\n"
				"EmrInfoT.DataType "
					"FROM EmrDetailsT INNER JOIN EmrMasterT ON EmrDetailsT.EmrID = EmrMasterT.ID \r\n"
					"INNER JOIN EmrInfoT ON EmrDetailsT.EmrInfoID = EmrInfoT.ID \r\n"
					"WHERE EmrMasterT.Deleted = 0 AND EmrDetailsT.Deleted = 0 AND EmrInfoT.DataType = 1 \r\n"
				"UNION SELECT EmrMasterT.PatientID, Convert(nvarchar(2000),EmrDataT.EmrDataGroupID) AS Value, EmrInfoT.EmrInfoMasterID, \r\n"
				"EmrInfoT.DataType "
					"FROM EmrDetailsT INNER JOIN EmrMasterT ON EmrDetailsT.EmrID = EmrMasterT.ID \r\n"
					"INNER JOIN EmrInfoT ON EmrDetailsT.EmrInfoID = EmrInfoT.ID \r\n"
					"INNER JOIN EmrSelectT ON EmrSelectT.EmrDetailID = EmrDetailsT.ID \r\n"
					"INNER JOIN EmrDataT ON EmrSelectT.EmrDataID = EmrDataT.ID \r\n"
					"WHERE EmrMasterT.Deleted = 0 AND EmrDetailsT.Deleted = 0 AND EmrInfoT.DataType IN (2,3) \r\n"
				"UNION SELECT EmrMasterT.PatientID, Convert(nvarchar(2000), EmrDetailsT.SliderValue) AS Value, EmrInfoT.EmrInfoMasterID, \r\n"
				"EmrInfoT.DataType "
					"FROM EmrDetailsT INNER JOIN EmrMasterT ON EmrDetailsT.EmrID = EmrMasterT.ID \r\n"
					"INNER JOIN EmrInfoT ON EmrDetailsT.EmrInfoID = EmrInfoT.ID \r\n"
					"WHERE EmrMasterT.Deleted = 0 AND EmrDetailsT.Deleted = 0 AND EmrInfoT.DataType = 5 \r\n"
				") AS EmrDetailValuesQ "
					"ON EmrDetailValuesQ.PatientID = PersonT.ID AND EmrDetailValuesQ.EmrInfoMasterID = DecisionRulesCriterionT.RecordID \r\n"
					"AND DecisionRulesCriterionT.[Type] = 8 \r\n"
				"WHERE \r\n"
					"PersonT.ID = @nPatientID \r\n"
					/*"AND (LabsT.ID IS NOT NULL OR DecisionRulesCriterionT.[Type] NOT IN (2,3)) \r\n"
					"AND (EmrProblemsT.PatientID IS NOT NULL OR DecisionRulesCriterionT.[Type] <> 5) \r\n"
					"AND (PatientMedications.PatientID IS NOT NULL OR DecisionRulesCriterionT.[Type] <> 6) \r\n"
					"AND (PatientAllergyT.ID IS NOT NULL OR DecisionRulesCriterionT.[Type] <> 7) \r\n"*/
				"FOR XML PATH('Criterion'), ROOT('Rule'), TYPE) AS Criterion \r\n"
				"FROM (SELECT * FROM DecisionRulesT WHERE Inactive = 0) AS DecisionRulesT \r\n"
			"INNER JOIN DecisionRulesCriterionT \r\n"
				"ON DecisionRulesCriterionT.RuleID = DecisionRulesT.ID \r\n"
			"FOR XML PATH('Rule'), ROOT('Root'), TYPE) \r\n");

		sqlBatch.Declare("DECLARE @qry NVARCHAR(MAX)");
		sqlBatch.Add("SET @qry = \r\n"
			"(SELECT STUFF( \r\n"
			"(SELECT DISTINCT \r\n"
				"' UNION ' + ISNULL( \r\n"
				"'SELECT ' + \r\n"
				"CAST(DecisionRulesT.ID AS NVARCHAR) + ' AS ID, ''' + REPLACE(DecisionRulesT.ToDoMessage, '''', '''''') + ''' AS Message WHERE ' + \r\n"
				"STUFF(CAST( \r\n"
				"(SELECT ' AND (' + STUFF(InnerRules.[Rule].value('.', 'NVARCHAR(MAX)'), 1, 4, '') + ')' \r\n"
				"FROM @rules.nodes('Root/Rule') InnerRules([Rule]) \r\n"
				"WHERE InnerRules.[Rule].value('@ID', 'INT') = Rules.[Rule].value('@ID', 'INT') \r\n"
				"FOR XML PATH(''), TYPE).value('.', 'NVARCHAR(MAX)') AS NVARCHAR(MAX)), 1, 5, ''), 'SELECT 0 AS ID, '''' AS Message WHERE 1=0') AS Query \r\n"
			"FROM @rules.nodes('Root/Rule') Rules([Rule]) \r\n"
			"INNER JOIN (SELECT * FROM DecisionRulesT WHERE Inactive = 0) AS DecisionRulesT ON DecisionRulesT.ID = Rules.[Rule].value('@ID', 'INT') \r\n"
			"FOR XML PATH(''), TYPE).value('.', 'NVARCHAR(MAX)'), 1, 7, '')) \r\n");


		sqlBatch.Declare("BEGIN TRAN");
		sqlBatch.Declare("DECLARE @ActiveRulesT TABLE (ID INT, [Message] NVARCHAR(2000))");
		sqlBatch.Declare("DECLARE @AddedInterventionT TABLE (ID INT)");
		sqlBatch.Add("INSERT INTO @ActiveRulesT (ID, [Message]) \r\n"
			"EXEC (@qry) \r\n");
		
		sqlBatch.Add(
			"INSERT INTO DecisionRuleInterventionsT (DecisionRuleID, PatientID) \r\n"
			"	OUTPUT INSERTED.ID INTO @AddedInterventionT \r\n"
			"SELECT ID, @nPatientID FROM @ActiveRulesT ActiveQ \r\n"
			"WHERE NOT EXISTS (SELECT ID FROM DecisionRuleInterventionsT WHERE DecisionRuleID = ActiveQ.ID AND PatientID = @nPatientID) \r\n");
		
		sqlBatch.Declare("SET NOCOUNT OFF");

		sqlBatch.Add("SELECT ID FROM @AddedInterventionT");

		sqlBatch.Declare("COMMIT TRAN");

		_RecordsetPtr rsNewInterventions = sqlBatch.CreateRecordsetNoTransaction(GetRemoteData());
		//TES 10/31/2013 - PLID 59251 - Output the new intervention IDs (we add to the array, it may already have interventions in it if we're in the
		// middle of some kind of batch save.
		while (!rsNewInterventions->eof) {
			arNewInterventions.Add(AdoFldLong(rsNewInterventions, "ID"));
			rsNewInterventions->MoveNext();
		}
		
	}NxCatchAllCall(__FUNCTION__, MsgBox("An Error occurred checking Clinical Decision Support Rules. No Clinical Decision Support Rules have checked."););
}

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
InterventionCriteriaOperator GetDefaultOperator(DecisionRuleCriterionType drct)
{
	switch(drct) {
		case drctAge:
		case drctLabsResultValue:
			return icoGreaterThanOrEqual;
			break;
		case drctLabResultName:	
		case drctProblemName:
		case drctMedicationName:
		case drctAllergyName:
		case drctEmrItem:
			return icoEqual;
			break;		
		case drctFilter:  // (j.gruber 2010-02-25 09:28) - PLID 37537
			return icoIsIn;
			break;

	}
	ASSERT(FALSE);
	return icoEqual;
}

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
//TES 12/2/2013 - PLID 59532 - Added eitEmrInfoType
NXDATALIST2Lib::IFormatSettingsPtr GetOperatorFormatSettings(DecisionRuleCriterionType drct, long eitEmrInfoType)
{
	try {

		CString strOperatorComboSource;

		switch(drct) {
			case drctAge:
			case drctLabsResultValue:
				strOperatorComboSource.Format(
					"%i;Is Greater Than Or Equal To (>=);"
					"%i;Is Less Than Or Equal To (<=);"
					"%i;Is Equal To (=);"
					"%i;Is Not Equal To (<>);",					
					icoGreaterThanOrEqual, 
					icoLessThanOrEqual,
					icoEqual, icoNotEqual);
				break;
			case drctLabResultName:
			case drctProblemName:
			case drctMedicationName:
			case drctAllergyName:
				//TES 11/15/2013 - PLID 59525 - Added Contains and Does Not Contain operators for result name, medication, allergy, and problem
				strOperatorComboSource.Format("%i;Is Equal To (=);"
					"%i;Is Not Equal To (<>);"
					"%i;Contains;"
					"%i;Does Not Contain;",					
					icoEqual, icoNotEqual,
					icoContains, icoDoesNotContain);
				break;	
			case drctFilter: // (j.gruber 2010-02-25 09:28) - PLID 37537
				strOperatorComboSource.Format("%i;Is In;"
					"%i;Is NOT In;",					
					icoIsIn, icoIsNotIn);
				//TES 11/21/2013 - PLID 59506 - Was missing this break, causing the wrong operators to show.
				break;
			case drctEmrItem:
				//TES 12/2/2013 - PLID 59532 - Don't give the greater/less than operators for list-type items
				//TES 12/19/2013 - PLID 59532 - Took out the Exists/Does Not Exist operators, they were never actually implemented
				//TES 12/26/2013 - PLID 59532 - Took out the Is Not Filled In operator
				switch(eitEmrInfoType) {
					case eitText:
					case eitSlider:
					strOperatorComboSource.Format("%i;Is Equal To (=);"
						"%i;Is Not Equal To (<>);"
						"%i;Is Greater Than (>);"
						"%i;Is Less Than (<);"
						"%i;Is Greater Than Or Equal To (>=);"
						"%i;Is Less Than Or Equal To (<=);"
						"%i;Contains;"
						"%i;Does Not Contain;"
						"%i;Is Filled In;",
						icoEqual, icoNotEqual, icoGreaterThan, icoLessThan, icoGreaterThanOrEqual, 
						icoLessThanOrEqual, icoContains, icoDoesNotContain, icoFilledIn);
					break;
					case eitSingleList:
					case eitMultiList:
					default:
					strOperatorComboSource.Format("%i;Is Equal To (=);"
						"%i;Is Not Equal To (<>);"
						"%i;Is Filled In;",
						icoEqual, icoNotEqual, 
						icoFilledIn);
					break;
				}
			break;

		}

		
		
		NXDATALIST2Lib::IFormatSettingsPtr pfs(__uuidof(NXDATALIST2Lib::FormatSettings));

		if(g_mapComboSourceToOperatorFormatSettings.Lookup((LPCTSTR)strOperatorComboSource, pfs)) {
			//excellent, get out of here
			return pfs;
		}
		else {
			//create the format settings, add to the map, and return it
			pfs->PutDataType(VT_I2);
			pfs->PutFieldType(NXDATALIST2Lib::cftComboSimple);
			pfs->PutEditable(VARIANT_TRUE);
			pfs->PutConnection(_variant_t((LPDISPATCH)GetRemoteData())); //we're going to let this combo use Practice's connection
			pfs->PutComboSource(_bstr_t(strOperatorComboSource));

			g_mapComboSourceToOperatorFormatSettings.SetAt((LPCTSTR)strOperatorComboSource, pfs);

			return pfs;
		}

	}NxCatchAll(__FUNCTION__);

	return NULL;
}

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
NXDATALIST2Lib::IFormatSettingsPtr GetValueFormatSettings(DecisionRuleCriterionType drct, long eitEmrInfoType, long nRecordID, InterventionCriteriaOperator ico)
{
	try {

		CString strValueComboSource;
		bool bUseCombo = true, bUseHyperlink = false;
		short vt = VT_BSTR;

		switch(drct) {
			case drctAge:
			case drctLabsResultValue:			
				bUseCombo = false;
				vt = VT_R8;
				break;
			case drctLabResultName:	
			case drctProblemName:
			case drctMedicationName:
			case drctAllergyName:
				bUseCombo = false;
				vt = VT_BSTR;				
			break;
			case drctFilter: // (j.gruber 2010-02-25 09:29) - PLID 37537
				bUseCombo = true;
				vt = VT_I4;
				strValueComboSource.Format("SELECT ID, Name FROM FiltersT WHERE Type = %li ORDER BY Name", fboPerson);
			break;		
			case drctEmrItem:
				switch(eitEmrInfoType) {
					case eitText:
					case eitImage:
					case eitNarrative:
					case eitTable:
						bUseCombo = false;
						break;
					case eitSlider:
						bUseCombo = false;
						vt = VT_R8;
						break;
					case eitSingleList:
					//TES 12/2/2013 - PLID 59532 - Use the same format options for multi-list items, we no longer allow selecting
					// multiple list options here.
					case eitMultiList:
						// (j.gruber 2009-07-02 16:27) - PLID 34350 - take out inactives and labels
						// (j.jones 2009-07-15 17:56) - PLID 34916 - ensure we filter on list items only, incase any table columns exist
						strValueComboSource.Format("SELECT EmrDataT.EmrDataGroupID, EmrDataT.Data, CASE WHEN EMRDataT.Inactive = 0 AND EMRDataT.IsLabel = 0 then 1 else 0 END AS IsVisible "
							"FROM EmrDataT INNER JOIN EmrInfoMasterT ON EmrDataT.EmrInfoID = EmrInfoMasterT.ActiveEmrInfoID "
							"WHERE EmrInfoMasterT.ID = %li AND EMRDataT.ListType = 1", nRecordID);
						break;
				}
				break;
		}

		
		
		NXDATALIST2Lib::IFormatSettingsPtr pfs(__uuidof(NXDATALIST2Lib::FormatSettings));
		CString strValueFormatKey;
		strValueFormatKey.Format("%li|%li|%li|%s", vt, bUseCombo?1:0, bUseHyperlink?1:0, strValueComboSource);
		if(g_mapComboSourceToValueFormatSettings.Lookup((LPCTSTR)strValueFormatKey, pfs)) {
			return pfs;
		}
		else {
			//create the format settings, add to the map, and return it
			pfs->PutDataType(vt);
			if(bUseCombo) {
				pfs->PutFieldType(NXDATALIST2Lib::cftComboSimple);
			}	
			else if(bUseHyperlink) {
				pfs->PutFieldType(NXDATALIST2Lib::cftTextSingleLineLink);
			}
			else {
				pfs->PutFieldType(NXDATALIST2Lib::cftTextSingleLine);
			}
			pfs->PutEditable(VARIANT_TRUE);
			pfs->PutConnection(_variant_t((LPDISPATCH)GetRemoteData())); //we're going to let this combo use Practice's connection
			pfs->PutComboSource(_bstr_t(strValueComboSource));
			g_mapComboSourceToValueFormatSettings.SetAt((LPCTSTR)strValueFormatKey, pfs);
			return pfs;
		}

	}NxCatchAll(__FUNCTION__);

	return NULL;
}