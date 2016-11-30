//TES 5/20/2009 - PLID 34302 - Created, stores enums and utility functions
#include "stdafx.h"
#include "WellnessUtils.h"
#include "WellnessDataUtils.h"
#include "EmrUtils.h"
#include "Groups.h"

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
// (a.wilson 2014-09-05 11:39) - PLID 63535 - key value should never be LPCTSTR as that would save the address of the string as the key.
namespace
{
	CMap<CString, LPCTSTR, NXDATALIST2Lib::IFormatSettingsPtr, NXDATALIST2Lib::IFormatSettingsPtr> g_mapComboSourceToOperatorFormatSettings;
	CMap<CString, LPCTSTR, NXDATALIST2Lib::IFormatSettingsPtr, NXDATALIST2Lib::IFormatSettingsPtr> g_mapComboSourceToValueFormatSettings;
}

//TES 5/22/2009 - PLID 34302 - Generates the appropriate list of operators for a given type of criterion.
// Copied originally from CEMRAnalysisConfigDlg::GetOperatorFormatSettings().
NXDATALIST2Lib::IFormatSettingsPtr GetOperatorFormatSettings(WellnessTemplateCriterionType wtct)
{
	try {

		CString strOperatorComboSource;

		switch(wtct) {
			case wtctAge:
				strOperatorComboSource.Format(
					"%i;Is Greater Than Or Equal To (>=);"
					"%i;Is Less Than Or Equal To (<=);"
					"%i;Is Filled In;"
					"%i;Is Not Filled In;",
					wtcoGreaterThanOrEqual, 
					wtcoLessThanOrEqual, wtcoFilledIn, wtcoNotFilledIn);
				break;
			case wtctGender:
				strOperatorComboSource.Format("%i;Is Equal To (=);"
					"%i;Is Filled In;"
					"%i;Is Not Filled In;",
					wtcoEqual, wtcoFilledIn, wtcoNotFilledIn);
				break;
			case wtctEmrItem:
				strOperatorComboSource.Format("%i;Is Equal To (=);"
					"%i;Is Not Equal To (<>);"
					"%i;Is Greater Than (>);"
					"%i;Is Less Than (<);"
					"%i;Is Greater Than Or Equal To (>=);"
					"%i;Is Less Than Or Equal To (<=);"
					"%i;Contains;"
					"%i;Does Not Contain;"
					"%i;Is Filled In;"
					"%i;Is Not Filled In;"
					"%i;Exists;"
					"%i;Does Not Exist;",
					wtcoEqual, wtcoNotEqual, wtcoGreaterThan, wtcoLessThan, wtcoGreaterThanOrEqual, 
					wtcoLessThanOrEqual, wtcoContains, wtcoDoesNotContain, wtcoFilledIn, wtcoNotFilledIn,
					wtcoExists, wtcoDoesNotExist);
				break;
			case wtctEmrProblemList:
				strOperatorComboSource.Format("%i;Contains;"
					"%i;Does Not Contain;",
					wtcoContains, wtcoDoesNotContain);
				break;
			//TES 6/8/2009 - PLID 34509 - Added support for immunization
			case wtctImmunization:
				strOperatorComboSource.Format("%i;Exists;"
					"%i;Does Not Exist;",
					wtcoExists, wtcoDoesNotExist);
				break;
			case wtctFilter:
				strOperatorComboSource.Format("%i;Is In;"
					"%i;Is NOT In;",
					wtcoIsIn, wtcoIsNotIn);
				break;
			case wtctLabResultName:
				strOperatorComboSource.Format("%i;Is Equal To (=);"
					"%i;Is Not Equal To (<>);",
					wtcoEqual, wtcoNotEqual);
				break;
			case wtctLabResultValue:
				strOperatorComboSource.Format("%i;Is Equal To (=);"
					"%i;Is Not Equal To (<>);"
					"%i;Is Greater Than (>);"
					"%i;Is Less Than (<);"
					"%i;Is Greater Than Or Equal To (>=);"
					"%i;Is Less Than Or Equal To (<=);",
					wtcoEqual, wtcoNotEqual, wtcoGreaterThan, wtcoLessThan, wtcoGreaterThanOrEqual, 
					wtcoLessThanOrEqual);
				break;
		}

		
		// (j.jones 2009-04-09 15:09) - PLID 33847 - is this already in our map?
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

	}NxCatchAll("Error in WellnessUtils::GetOperatorFormatSettings");

	return NULL;
}

//TES 5/22/2009 - PLID 34302 - Formats the Value cell based on information about the criterion.
// Copied originally from CEMRAnalysisConfigDlg::GetOperatorFormatSettings().
//TES 6/8/2009 - PLID 34509 - Renamed EmrInfoMasterID to RecordID
NXDATALIST2Lib::IFormatSettingsPtr GetValueFormatSettings(WellnessTemplateCriterionType wtct, EmrInfoType eit, long nRecordID, WellnessTemplateCriteriaOperator wtco)
{
	try {

		CString strValueComboSource;
		bool bUseCombo = true, bUseHyperlink = false;
		short vt = VT_BSTR;

		switch(wtct) {
			case wtctAge:
				bUseCombo = false;
				vt = VT_I4;
				break;
			case wtctGender:
				strValueComboSource.Format("1;Male;"
					"2;Female;");
				vt = VT_I4;
				break;
			case wtctEmrItem:
				switch(eit) {
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
						// (j.gruber 2009-07-02 16:27) - PLID 34350 - take out inactives and labels
						// (j.jones 2009-07-15 17:56) - PLID 34916 - ensure we filter on list items only, incase any table columns exist
						strValueComboSource.Format("SELECT EmrDataT.EmrDataGroupID, EmrDataT.Data, CASE WHEN EMRDataT.Inactive = 0 AND EMRDataT.IsLabel = 0 then 1 else 0 END AS IsVisible "
							"FROM EmrDataT INNER JOIN EmrInfoMasterT ON EmrDataT.EmrInfoID = EmrInfoMasterT.ActiveEmrInfoID "
							"WHERE EmrInfoMasterT.ID = %li AND EMRDataT.ListType = 1", nRecordID);
						break;
					case eitMultiList:
						bUseCombo = false;
						strValueComboSource = "Hyperlink";
						//TES 6/2/2009 - PLID 34302 - Only use the hyperlink if we have a transitive operator.
						if(wtco == wtcoFilledIn || wtco == wtcoNotFilledIn || wtco == wtcoExists || wtco == wtcoDoesNotExist) {
							bUseHyperlink = false;
						}
						else {
							bUseHyperlink = true;
						}
						break;
				}
				break;
			case wtctEmrProblemList:
				vt = VT_BSTR;
				bUseCombo = false;
				break;
			//TES 6/8/2009 - PLID 34509 - Added support for immunization
			case wtctImmunization:
				vt = VT_BSTR;
				bUseCombo = false;
				break;
			case wtctFilter:
				bUseCombo = true;
				vt = VT_I4;
				strValueComboSource.Format("SELECT ID, Name FROM FiltersT WHERE Type = %li", fboPerson);
				break;
			case wtctLabResultName:
				vt = VT_BSTR;
				bUseCombo = false;
				break;
			case wtctLabResultValue:
				vt = VT_BSTR;
				bUseCombo = false;
				break;
		}

		
		// (j.jones 2009-04-09 15:09) - PLID 33847 - is this already in our map?
		NXDATALIST2Lib::IFormatSettingsPtr pfs(__uuidof(NXDATALIST2Lib::FormatSettings));
		CString strValueFormatKey;
		strValueFormatKey.Format("%i|%i|%i|%s", vt, bUseCombo?1:0, bUseHyperlink?1:0, strValueComboSource);
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

	}NxCatchAll("Error in WellnessUtils::GetValueFormatSettings");

	return NULL;
}

//TES 5/22/2009 - PLID 34302 - Returns the default operator for a given type of criterion.
WellnessTemplateCriteriaOperator GetDefaultOperator(WellnessTemplateCriterionType wtct)
{
	switch(wtct) {
		case wtctAge:
		case wtctLabResultValue:
			return wtcoGreaterThanOrEqual;
			break;
		case wtctGender:
		case wtctEmrItem:
		case wtctLabResultName:
			return wtcoEqual;
			break;
		case wtctEmrProblemList:
			return wtcoContains;
			break;
		//TES 6/8/2009 - PLID 34509 - Added support for immunization
		case wtctImmunization:
			return wtcoExists;
			break;
		case wtctFilter:
			return wtcoIsIn;
			break;
	}
	ASSERT(FALSE);
	return wtcoEqual;
}

// (b.cardillo 2009-07-09 09:30) - PLID 34369 - Split data-related functionality out of WellnessUtils so that the 
// business logic can be shared with other projects (such as NxWeb.dll)
