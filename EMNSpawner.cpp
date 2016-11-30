#include <stdafx.h>
#include "EMNSpawner.h"
#include "EMNDetail.h"
#include "InvVisionWebUtils.h"
#include "WoundCareCalculator.h"
#include "NxCache.h"
#include "EmrItemAdvImageState.h"	// (j.armen 2014-07-21 16:32) - PLID 62836

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (c.haag 2007-08-06 13:09) - PLID 26977 - Added bIsTemplate for loading list and table data
// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
CEMNSpawner::CEMNSpawner(const MFCArray<EmrAction>& arActions, BOOL bIsTemplate)
{
	// (a.walling 2013-07-18 10:04) - PLID 57629 - Ensure NxCache is up to date
	Nx::Cache::Checkpoint();
	m_arActions.Copy(arActions);
	m_bIsTemplate = bIsTemplate;
}

CEMNSpawner::~CEMNSpawner()
{
	POSITION pos;

	// (c.haag 2007-08-06 11:33) - PLID 26954 - Clean up our EMR info map
	pos = m_mapInfoItems.GetStartPosition();
	while (pos != NULL) {
		CEMRInfoItem* pInfo;
		long nEMRInfoID;
		m_mapInfoItems.GetNextAssoc( pos, nEMRInfoID, pInfo );
		if (pInfo) delete pInfo;
	}
}

//////////////////////////////////////////////////////////////////////
// Query generation and processing functions
//////////////////////////////////////////////////////////////////////

// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
CSqlFragment CEMNSpawner::GenerateEmrInfoGeneralSql(const CArray<long, long>& anMasterIDs) const
{
	// (c.haag 2007-08-06 11:18) - PLID 26954 - This function generates a SQL
	// statement to pull general EmrInfoT fields in the preloading of EMR item data.
	// We have to include the Active EMR info ID as well because LoadContent may require
	// that we load from it.
	// (c.haag 2008-10-16 11:26) - PLID 31709 - Added TableRowsAsFields
	//TES 11/5/2008 - PLID 31926 - This needs to hard-code the system tables to be "RememberForPatient" 
	// (and NOT "RememberForEMR"), because CEMRItemEntryDlg hardcodes them that way even though the data doesn't always
	// reflect it.  This should have been done in the course of PLID 29416 back in June, but I managed to overlook this
	// query.
	//TES 2/21/2010 - PLID 37463 - Added SmartStampsLongForm and UseSmartStampsLongForm
	// (z.manning 2010-07-26 14:36) - PLID 39848 - Removed SmartStampsLongForm and UseSmartStampsLongForm
	//TES 3/17/2011 - PLID 41108 - Added HasGlassesOrderData and GlassesOrderLens
	// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
	//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
	return CSqlFragment("SELECT "
		"EmrInfoT.ID, EmrInfoT.SliderMin, EmrInfoT.SliderMax, EmrInfoT.SliderInc, EmrInfoT.BackgroundImageType, EmrInfoT.BackgroundImageFilePath, "
		"EmrInfoT.LongForm, EmrInfoT.DataFormat, EmrInfoT.DataSeparator, EmrInfoT.DataSeparatorFinal, EmrInfoT.DisableTableBorder, EmrInfoT.DataType, "
		// (c.haag 2007-08-06 14:21) - PLID 26977 - Include AutoAlphabetizeListData for lists
		"EmrInfoT.AutoAlphabetizeListData, "
		// (c.haag 2007-08-06 15:46) - PLID 26992 - Other fields necessary for CEMNDetail::LoadFromInfoOrMasterID
		"EmrInfoT.EmrInfoMasterID, EmrInfoT.Name, EmrInfoT.DataSubType, EmrInfoT.TableRowsAsFields, "
		// (j.jones 2010-02-11 15:24) - PLID 37318 - load ChildEMRInfoMasterID and SmartStampsEnabled
		"EmrInfoT.ChildEMRInfoMasterID, EmrInfoT.SmartStampsEnabled, "
		"(CASE WHEN EmrInfoT.ID IN (SELECT SourceID FROM EMRActionsT WHERE Deleted = 0 AND SourceType = {CONST}) THEN 1 ELSE 0 END) AS HasInfoActions, "
		// (c.haag 2007-08-07 10:08) - PLID 26998 - Fields for LoadEMRDetailStateDefault
		// (j.jones 2008-09-22 15:05) - PLID 31408 - added RememberForEMR
		"CASE WHEN EmrInfoT.DataSubType IN ({CONST}, {CONST}) THEN convert(bit,1) "
		"	ELSE EmrInfoT.RememberForPatient END AS RememberForPatient, "
		"CASE WHEN EmrInfoT.DataSubType IN ({CONST}, {CONST}) THEN convert(bit,0) "
		"	ELSE EmrInfoT.RememberForEMR END AS RememberForEMR, "
		"EmrInfoT.DefaultText, "
		"CASE WHEN EmrInfoT.ID IN (SELECT EMRInfoDefaultsT.EMRInfoID FROM EMRInfoDefaultsT INNER JOIN EMRDataT ON EMRInfoDefaultsT.EMRDataID = EMRDataT.ID WHERE EMRDataT.Inactive = 0 AND EMRDataT.IsLabel = 0) THEN 1 ELSE 0 END AS HasDefaultValue, "
		// (j.jones 2007-08-27 10:26) - PLID 27056 - added E/M coding data
		// (j.jones 2011-03-09 09:05) - PLID 42283 - added EMCodeUseTableCategories
		// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields, because they are now only calculated in the API, and not in Practice code
		//"EMRInfoMasterT.EMCodeCategoryID, EMRInfoMasterT.EMCodeUseTableCategories, EMRInfoMasterT.UseEMCoding AS Info_UseEMCoding, EMRInfoMasterT.EMCodingType, "
		// (a.walling 2008-06-30 13:40) - PLID 29271 - Preview Pane flags
		"EmrInfoT.PreviewFlags, "
		"EmrInfoT.HasGlassesOrderData, EmrInfoT.GlassesOrderLens, EmrInfoT.InfoFlags, EmrInfoT.HasContactLensData, "
		// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
		"EmrInfoT.UseWithWoundCareCoding "
		"FROM EmrInfoMasterT "
		"LEFT JOIN EmrInfoT ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID "
		"WHERE EmrInfoMasterT.ID IN ({INTARRAY})",
		eaoEmrItem, eistCurrentMedicationsTable, eistAllergiesTable, eistCurrentMedicationsTable, eistAllergiesTable,
		anMasterIDs);
}

// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic

void CEMNSpawner::ProcessEmrInfoGeneralRecords(_RecordsetPtr& prs)
{
	// (c.haag 2007-08-06 11:25) - PLID 26954 - This function process records
	// returned from a query which used GenerateEmrInfoGeneralSql
	FieldsPtr f = prs->Fields;
	while (!prs->eof) {
		CEMRInfoItem* pItem = NULL;
		const long nEMRInfoID = AdoFldLong(f, "ID");
		if (!m_mapInfoItems.Lookup(nEMRInfoID, pItem)) {
			if (NULL == (pItem = new CEMRInfoItem)) {
				ThrowNxException("Could not allocate data for CEMNSpawner::ProcessEmrInfoGeneralRecords!");
			}
			pItem->m_nID = nEMRInfoID;
			m_mapInfoItems[pItem->m_nID] = pItem;
			// Slider variables
			pItem->m_vSliderMin = f->Item["SliderMin"]->Value;
			pItem->m_vSliderMax = f->Item["SliderMax"]->Value;
			pItem->m_vSliderInc = f->Item["SliderInc"]->Value;
			// Image variables
			pItem->m_vBackgroundImageFilePath = f->Item["BackgroundImageFilePath"]->Value;
			pItem->m_vBackgroundImageType = f->Item["BackgroundImageType"]->Value;
			// Other variables
			pItem->m_vLongForm = f->Item["LongForm"]->Value;
			pItem->m_vDataFormat = f->Item["DataFormat"]->Value;
			pItem->m_vDataSeparator = f->Item["DataSeparator"]->Value;
			pItem->m_vDataSeparatorFinal = f->Item["DataSeparatorFinal"]->Value;
			pItem->m_vHasInfoActions = f->Item["HasInfoActions"]->Value;
			pItem->m_vDisableTableBorder = f->Item["DisableTableBorder"]->Value;
			pItem->m_vDataType = f->Item["DataType"]->Value;
			pItem->m_vTableRowsAsFields = f->Item["TableRowsAsFields"]->Value; // (c.haag 2008-10-16 11:26) - PLID 31709
			// (c.haag 2007-08-06 14:24) - PLID 26977 - AutoAlphabetizeListData
			pItem->m_vAutoAlphabetizeListData = f->Item["AutoAlphabetizeListData"]->Value;
			// (c.haag 2007-08-06 15:46) - PLID 26992 - Other fields necessary for CEMNDetail::LoadFromInfoOrMasterID
			pItem->m_nMasterID = AdoFldLong(f, "EmrInfoMasterID");
			pItem->m_vName = f->Item["Name"]->Value;
			pItem->m_vDataSubType = f->Item["DataSubType"]->Value;
			// (c.haag 2007-08-07 10:07) - PLID 26998 - Fields for LoadEMRDetailStateDefault
			pItem->m_vRememberForPatient = f->Item["RememberForPatient"]->Value;
			// (j.jones 2008-09-22 15:05) - PLID 31408 - added RememberForEMR
			pItem->m_vRememberForEMR = f->Item["RememberForEMR"]->Value;
			pItem->m_vDefaultText = f->Item["DefaultText"]->Value;
			pItem->m_vHasDefaultValue = f->Item["HasDefaultValue"]->Value;
			// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields,
			// because they are now only calculated in the API, and not in Practice code
			/*
			// (j.jones 2007-08-27 10:35) - PLID 27056 - load the E/M coding data
			pItem->m_vEMCodeCategoryID = f->Item["EMCodeCategoryID"]->Value;
			// (j.jones 2011-03-09 09:05) - PLID 42283 - added m_vEMCodeUseTableCategories
			pItem->m_vEMCodeUseTableCategories = f->Item["EMCodeUseTableCategories"]->Value;
			pItem->m_vUseEMCoding = f->Item["Info_UseEMCoding"]->Value;
			pItem->m_vEMCodingType = f->Item["EMCodingType"]->Value;
			*/
			// (a.walling 2008-06-30 13:41) - PLID 29271 - Preview Pane flags
			pItem->m_vPreviewFlags = f->Item["PreviewFlags"]->Value;
			// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
			pItem->m_vChildEMRInfoMasterID = f->Item["ChildEMRInfoMasterID"]->Value;
			pItem->m_vSmartStampsEnabled = f->Item["SmartStampsEnabled"]->Value;
			//TES 3/17/2011 - PLID 41108 - Added Glasses Order data
			pItem->m_vHasGlassesOrderData = f->Item["HasGlassesOrderData"]->Value;
			pItem->m_vGlassesOrderLens = f->Item["GlassesOrderLens"]->Value;
			// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
			pItem->m_vInfoFlags = f->Item["InfoFlags"]->Value;
			//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
			pItem->m_vHasContactLensData = f->Item["HasContactLensData"]->Value;
			// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
			pItem->m_vUseWithWoundCareCoding = f->Item["UseWithWoundCareCoding"]->Value;
			m_mapInfoItemsByMasterID[pItem->m_nMasterID] = pItem;
		}
		prs->MoveNext();
	}
}

// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic

// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
// (j.jones 2011-04-28 14:39) - PLID 43122 - added nProviderIDForFloatingData as a required parameter (send -1 if you don't have a provider)
CSqlFragment CEMNSpawner::GenerateEmrInfoListAndTableItemSql(const CArray<long, long>& anMasterIDs, const long nProviderIDForFloatingData) const
{
	// (j.jones 2011-04-28 14:39) - PLID 43122 - If the currently selected EMN provider has ProvidersT.FloatEMRData = 1,
	// load the data from EmrProviderFloatDataT. If we are sorting alphabetically, sort the "floated" items first, 
	// then the regular items. If not sorting alphabetically, sort the "floated" items in Count order DESC, and then
	// the regular items in their normal sort order.

	// (c.haag 2007-08-06 13:00) - PLID 26977 - This function generates a SQL
	// statement to pull single and multi-select list and table information for use with
	// CEMNDetail::LoadContent.
	// (z.manning, 05/23/2008) - PLID 30155 - Added EmrDataT.Formula and DecimalPlaces
	// (z.manning 2009-01-15 17:04) - PLID 32724 - Added InputMask
	// (z.manning 2010-02-16 14:40) - PLID 37230 - ListSubType
	// (c.haag 2010-02-24 14:17) - PLID 21301 - AutoAlphabetizeDropdown
	// (z.manning 2011-03-11) - PLID 42778 - Added HasDropdownElements
	//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
	// (z.manning 2011-03-21 11:28) - PLID 30608 - Added autofill type
	// (z.manning 2011-05-26 15:00) - PLID 43865 - Added DataFlags
	// (z.manning 2011-09-19 17:22) - PLID 41954 - Added DropdownSeparator
	// (z.manning 2011-11-07 10:49) - PLID 46309 - SpawnedItemsSeparator
	// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
	return CSqlFragment(
		"SELECT EmrInfoT.ID AS EMRInfoID, EmrDataT.ID, EmrDataT.EmrDataGroupID, EmrDataT.Data, EmrDataT.IsLabel, EmrDataT.ListType, EmrDataT.IsGrouped, "
		"EmrDataT.LongForm AS DataLongForm, EmrDataT.SortOrder, EmrDataT.ListSubType, EmrDataT.AutoAlphabetizeDropdown, "
		// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
		"EmrDataT.AutoNumberType, EmrDataT.AutoNumberPrefix, "
		"CASE WHEN {INT} = 1 THEN CASE WHEN EmrInfoT.DataType = 2 OR EmrInfoT.DataType = 3 THEN (SELECT Min(CASE WHEN DestType = 3 OR DestType = 9 THEN 1 ELSE 2 END) AS MinActionType FROM EMRActionsT WHERE SourceType = 4 AND SourceID = EmrDataT.ID AND Deleted = 0) ELSE NULL END ELSE NULL END AS ActionsType, "
		// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of this, and the join to emrdatagroupst
		"EmrDataT.Formula, EmrDataT.DecimalPlaces, EmrDataT.InputMask, EmrDataT.DataFlags "

		// (a.walling 2013-02-28 17:13) - PLID 55390 - Avoid massive index scans and spooling and hash merges by limiting to individual seeks
		// based on the ListType of the Data when trying to determine whether dropdowns exist for this. Previously joining on the ad-hoc 
		// select dumps (potentially millions) of rows to a temp worktable and hashmatches. This replaces that with multiple seeks, unfortunately 2 per text/dropdown
		// item, but still better than before.
		// (a.walling 2013-07-02 14:57) - PLID 57401 - Indexed view for EMRTableDropdownInfoT's total/inactive counts
		", CONVERT(bit, "
			"CASE WHEN EmrDataT.ListType = 3 THEN "
				"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID AND TotalCount > InactiveCount) THEN 1 ELSE 0 END "
			"WHEN EmrDataT.ListType = 4 THEN "
				"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID) THEN 1 ELSE 0 END "
			"ELSE 0 "
			"END "
		") AS HasActiveDropdownElements "
		", CONVERT(bit, "
			"CASE WHEN EmrDataT.ListType IN (3,4) THEN "
				"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID) THEN 1 ELSE 0 END "
			"ELSE 0 "
			"END "
		") AS HasDropdownElements "
		"	, EmrDataT.GlassesOrderDataType, EmrDataT.GlassesOrderDataID, EmrDataT.AutofillType, DropdownSeparator, DropdownSeparatorFinal, SpawnedItemsSeparator, ParentLabelID, \r\n"
		// (j.jones 2011-04-28 14:39) - PLID 43122 - track if this list item is floating
		"Convert(bit, CASE WHEN FloatedItemsQ.Count Is Not Null THEN 1 ELSE 0 END) AS IsFloated, "
		// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
		"EmrDataT.WoundCareDataType "
		"FROM EmrInfoT "
		"LEFT JOIN EMRDataT ON EmrInfoT.ID = EmrDataT.EmrInfoID "
		// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of the join to emrdatagroupst
		// (j.jones 2011-04-28 14:39) - PLID 43122 - load the floating data info
		// (a.walling 2013-03-21 11:23) - PLID 55810 - The EmrProviderFloatDataT join can be skipped entirely if the providerid being scanned for is set to -1
		"LEFT JOIN (SELECT EMRDataGroupID, Count "
		"	FROM EmrProviderFloatDataT "
		"	WHERE ProviderID = {INT} AND {INT} <> -1) AS FloatedItemsQ ON EMRDataT.EmrDataGroupID = FloatedItemsQ.EMRDataGroupID "
		"WHERE EmrInfoT.ID IN (SELECT ActiveEmrInfoID FROM EmrInfoMasterT WHERE ID IN ({INTARRAY})) AND (EMRDataT.Inactive Is Null OR EMRDataT.Inactive = 0) "
		// (c.haag 2007-09-25 11:59) - PLID 27507 - Ensure the list and table data are properly sorted, or else they will appear in a random order on the chart
		"ORDER BY (CASE WHEN FloatedItemsQ.Count Is Not Null AND EMRInfoT.DataType IN (2,3) "
		"	THEN (CASE WHEN EmrInfoT.AutoAlphabetizeListData = 0 THEN FloatedItemsQ.Count ELSE 1 END) "
		"	ELSE 0 END) DESC, "
		"	(CASE WHEN FloatedItemsQ.Count Is Not Null AND EMRInfoT.DataType IN (2,3) THEN -1 ELSE (CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND ListType <> 2 THEN EmrDataT.SortOrder ELSE -1 END ELSE EmrDataT.SortOrder END) END), "
		"	(CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND ListType <> 2 THEN '' ELSE EmrDataT.Data END ELSE '' END) ",
		m_bIsTemplate ? 1 : 0, nProviderIDForFloatingData, nProviderIDForFloatingData, anMasterIDs);
}

// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
CSqlFragment CEMNSpawner::GenerateEmrInfoHotSpotsSql(const CArray<long, long>& anMasterIDs) const
{
	//DRT 2/14/2008 - PLID 28603 - Generates SQL to pull image hotspot information.  Based off the list & table query above.  We only want
	//	images with hotspots, we can filter out any spot-free images.
	//TES 2/11/2010 - PLID 37298 - Added Anatomic Location info for hotspots
	// (z.manning 2011-07-25 12:58) - PLID 44649 - Added ImageHotSpotID
	return CSqlFragment(
		"SELECT EmrInfoT.ID AS EMRInfoID, EMRImageHotSpotsT.ID, EMRImageHotSpotsT.EMRSpotGroupID, EMRImageHotSpotsT.Data, "
		"EMRImageHotSpotsT.AnatomicLocationID AS HotSpotLocationID, LabAnatomyT.Description AS HotSpotLocation, "
		"EMRImageHotSpotsT.AnatomicQualifierID AS HotSpotQualifierID, AnatomyQualifiersT.Name AS HotSpotQualifier, "
		"EMRImageHotSpotsT.AnatomySide AS HotSpotSide, ImageHotSpotID "
		"FROM EMRInfoT "
		"INNER JOIN EMRImageHotSpotsT ON EMRInfoT.ID = EMRImageHotSpotsT.EMRInfoID "
		"LEFT JOIN LabAnatomyT ON EMRImageHotSpotsT.AnatomicLocationID = LabAnatomyT.ID "
		"LEFT JOIN AnatomyQualifiersT ON EMRImageHotSpotsT.AnatomicQualifierID = AnatomyQualifiersT.ID "
		"WHERE EMRInfoT.ID IN (SELECT ActiveEmrInfoID FROM EmrInfoMasterT WHERE ID IN ({INTARRAY})) "
		, anMasterIDs);
}

// (z.manning 2011-10-25 10:36) - PLID 39401
CSqlFragment CEMNSpawner::GenerateStampExclusionSql(const CArray<long,long> *parynMasterIDs) const
{
	return CSqlFragment(
		"SELECT EmrInfoMasterID, StampID \r\n"
		"FROM EmrInfoStampExclusionsT \r\n"
		"WHERE EmrInfoMasterID IN ({INTARRAY}) \r\n"
		, *parynMasterIDs);
}

void CEMNSpawner::ProcessEmrInfoListAndTableItemRecords(ADODB::_RecordsetPtr& prs)
{
	// (c.haag 2007-08-06 13:01) - PLID 26977 - This function processes records
	// returned from a query which used GenerateEmrInfoListItemSql
	FieldsPtr f = prs->Fields;
	while (!prs->eof) {
		const long nEMRInfoID = AdoFldLong(f, "EMRInfoID");
		CEMRInfoItem* pInfoItem = m_mapInfoItems[nEMRInfoID];
		if (NULL == pInfoItem) {
			ThrowNxException("Failed to find EMR info item in CEMNSpawner::ProcessEmrInfoListAndTableItemRecords!");
		}
		// Fill in the EmrDataT records
		EmrDataItem data;
		_variant_t vID = f->Item["ID"]->Value;
		if (VT_NULL != vID.vt) {
			data.m_nID = VarLong(vID);
			data.m_nEMRDataGroupID = AdoFldLong(f, "EmrDataGroupID");
			data.m_strData = AdoFldString(f, "Data");
			data.m_bIsLabel = AdoFldBool(f, "IsLabel");
			data.m_nListType = AdoFldLong(f, "ListType");
			// (z.manning 2010-02-16 14:41) - PLID 37230 - ListSubType
			data.m_nListSubType = AdoFldByte(f, "ListSubType", lstDefault);
			data.m_bIsGrouped = AdoFldBool(f, "IsGrouped");
			data.m_strLongForm = AdoFldString(f, "DataLongForm");
			data.m_nActionsType = AdoFldLong(f, "ActionsType", -1);
			// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
			// (z.manning, 05/23/2008) - PLID 30155 - Added Formula and DecimalPlaces
			data.m_strFormula = AdoFldString(f, "Formula");
			data.m_nDecimalPlaces = AdoFldByte(f, "DecimalPlaces");
			// (z.manning 2009-01-15 15:33) - PLID 32724 - Added InputMask
			data.m_strInputMask = AdoFldString(f, "InputMask", "");
			// (c.haag 2010-02-24 14:17) - PLID 21301 - AutoAlphabetizeDropdown
			data.m_bAutoAlphabetizeDropdown = AdoFldBool(f, "AutoAlphabetizeDropdown");
			// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
			data.m_etantAutoNumberType = (EEmrTableAutoNumberType)AdoFldByte(f, "AutoNumberType", (BYTE)etantPerRow);
			data.m_strAutoNumberPrefix = AdoFldString(f, "AutoNumberPrefix", "");
			data.m_bHasDropdownElements = AdoFldBool(f, "HasDropdownElements", TRUE); // (z.manning 2011-03-11) - PLID 42778
			data.m_bHasActiveDropdownElements = AdoFldBool(f, "HasActiveDropdownElements", TRUE); // (z.manning 2011-03-11) - PLID 42778
			//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
			data.m_GlassesOrderDataType = (GlassesOrderDataType)AdoFldLong(f, "GlassesOrderDataType", (long)godtInvalid);
			data.m_nGlassesOrderDataID = AdoFldLong(f, "GlassesOrderDataID", -1);
			data.m_eAutofillType = (EmrTableAutofillType)AdoFldByte(f, "AutofillType", etatNone); // (z.manning 2011-03-22 10:30) - PLID 30608
			// (j.jones 2011-04-28 14:39) - PLID 43122 - track if this list item is floated,
			// which will require that we bold the text
			data.m_bIsFloated = AdoFldBool(f, "IsFloated", FALSE);
			data.m_nFlags = AdoFldLong(f, "DataFlags", 0); // (z.manning 2011-05-26 15:02) - PLID 43865
			data.m_strDropdownSeparator = AdoFldString(f, "DropdownSeparator", ", ");
			data.m_strDropdownSeparatorFinal = AdoFldString(f, "DropdownSeparatorFinal", ", ");
			data.m_strSpawnedItemsSeparator = AdoFldString(f, "SpawnedItemsSeparator", ", ");
			// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
			data.m_ewccWoundCareDataType = (EWoundCareDataType)AdoFldLong(f, "WoundCareDataType", wcdtNone);
			// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
			data.m_nParentLabelID = AdoFldLong(f, "ParentLabelID", -1);
			pInfoItem->m_arDataItems.Add(data);
		}
		prs->MoveNext();
	}
}

void CEMNSpawner::ProcessEmrInfoHotSpotItemRecords(ADODB::_RecordsetPtr& prs)
{
	//DRT 2/14/2008 - PLID 28603 - Process the records from GenerateEmrInfoHotSpotsSql and add them to our
	//	info item.
	FieldsPtr f = prs->Fields;
	while(!prs->eof) {
		const long nEMRInfoID = AdoFldLong(f, "EMRInfoID");
		CEMRInfoItem* pInfoItem = m_mapInfoItems[nEMRInfoID];
		if (NULL == pInfoItem) {
			ThrowNxException("Failed to find EMR info item in CEMNSpawner::ProcessEmrInfoHotSpotItemRecords!");
		}

		EmrImageHotSpotItem spot;
		spot.m_nID = AdoFldLong(f, "ID");
		spot.m_nEMRHotSpotGroupID = AdoFldLong(f, "EMRSpotGroupID", -1);
		spot.m_strData = AdoFldString(f, "Data", "");
		//TES 2/11/2010 - PLID 37298 - Added Anatomic Location info for hotspots
		spot.m_nAnatomicLocationID = AdoFldLong(f, "HotSpotLocationID", -1);
		spot.m_strAnatomicLocation = AdoFldString(f, "HotSpotLocation", "");
		spot.m_nAnatomicQualifierID = AdoFldLong(f, "HotSpotQualifierID", -1);
		spot.m_strAnatomicQualifier = AdoFldString(f, "HotSpotQualifier", "");
		spot.m_asSide = (AnatomySide)AdoFldLong(f, "HotSpotSide");
		spot.m_n3DHotSpotID = AdoFldShort(f, "ImageHotSpotID", -1);
		pInfoItem->m_aryHotSpotItems.Add(spot);

		prs->MoveNext();
	}
}

// (z.manning 2011-10-25 10:41) - PLID 39401
void CEMNSpawner::ProcessStampExclusionRecords(ADODB::_RecordsetPtr& prs)
{
	for(; !prs->eof; prs->MoveNext())
	{
		const long nEmrInfoMasterID = AdoFldLong(prs, "EmrInfoMasterID");
		CEMRInfoItem *pInfoItem = NULL;
		m_mapInfoItemsByMasterID.Lookup(nEmrInfoMasterID, pInfoItem);
		if(pInfoItem == NULL) {
			AfxThrowNxException("Failed to find info item (master ID = " + AsString(nEmrInfoMasterID) + ") in " + CString(__FUNCTION__));
		}

		const long nStampID = AdoFldLong(prs, "StampID");
		pInfoItem->m_StampExclusions.AddExclusion(nStampID);
	}
}

// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
CSqlFragment CEMNSpawner::GenerateEmrInfoListDefaultsSql(const CArray<long, long>& anMasterIDs) const
{
	// (c.haag 2007-08-07 10:31) - PLID 26998 - This function generates a SQL statement to pull
	// single and multi-select list default selections for use with LoadEMRDetailStateDefault.
	return CSqlFragment(
		"SELECT ActiveData.ID AS EmrDataID, EmrInfoT.ID AS EmrInfoID "
		"FROM EMRInfoT LEFT JOIN (EMRInfoDefaultsT INNER JOIN "
		"(SELECT * FROM EMRDataT WHERE EmrDataT.Inactive = 0 AND EmrDataT.IsLabel = 0) AS ActiveData "
		"ON EMRInfoDefaultsT.EMRDataID = ActiveData.ID) ON EMRInfoT.ID = EMRInfoDefaultsT.EMRInfoID "
		"WHERE EMRInfoT.ID IN (SELECT ActiveEmrInfoID FROM EmrInfoMasterT WHERE ID IN ({INTARRAY}))",
		anMasterIDs);
}

void CEMNSpawner::ProcessEmrInfoListDefaultRecords(ADODB::_RecordsetPtr& prs)
{
	// (c.haag 2007-08-07 10:33) - PLID 26998 - This function processes records from a query
	// whose text was generated by GenerateEmrInfoListDefaultsSql
	FieldsPtr f = prs->Fields;
	while (!prs->eof) {
		const long nEMRInfoID = AdoFldLong(f, "EmrInfoID");
		CEMRInfoItem* pInfoItem = m_mapInfoItems[nEMRInfoID];
		if (NULL == pInfoItem) {
			ThrowNxException("Failed to find EMR info item in CEMNSpawner::ProcessEmrInfoListDefaultRecords!");
		}
		pInfoItem->m_arDefaultListSelections.Add(AdoFldLong(f, "EmrDataID", -1));
		prs->MoveNext();
	}
}

//////////////////////////////////////////////////////////////////////
// Preload functions
//////////////////////////////////////////////////////////////////////

// (j.jones 2011-04-28 14:39) - PLID 43122 - added nProviderIDForFloatingData as a required parameter (send -1 if you don't have a provider)
void CEMNSpawner::PreloadEmrItemData(_Connection *lpCon, const long nProviderIDForFloatingData)
{
	// (c.haag 2007-08-06 10:47) - PLID 26954 - This function preloads data
	// related to spawning individual EMR items
	const int nActions = m_arActions.GetSize();
	_ConnectionPtr pConn;
	if(lpCon) pConn = lpCon;
	else pConn = GetRemoteData();
	CArray<long, long> anMasterIDs;
	BOOL bLoadEmrInfoData = FALSE;
	// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
	CSqlFragment sqlSql;
	_variant_t vrs;

	// Now build the SQL statement to read in all the data
	int i;
	for (i=0; i < nActions; i++) {
		if(m_arActions[i].eaoDestType == eaoEmrItem) {
			anMasterIDs.Add(m_arActions[i].nDestID);
		}
	}
	if (anMasterIDs.GetSize() > 0) {
		bLoadEmrInfoData = TRUE;
		sqlSql += GenerateEmrInfoGeneralSql(anMasterIDs);
		// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic
		// (c.haag 2007-08-06 13:13) - PLID 26977 - Include list and table data
		sqlSql += "\r\n";
		sqlSql += GenerateEmrInfoListAndTableItemSql(anMasterIDs, nProviderIDForFloatingData);
		// (c.haag 2007-08-07 10:41) - PLID 26998 - Include list defaults
		sqlSql += "\r\n";
		sqlSql += GenerateEmrInfoListDefaultsSql(anMasterIDs);
		//DRT 2/14/2008 - PLID 28603 - Added hotspots
		sqlSql += "\r\n";
		sqlSql += GenerateEmrInfoHotSpotsSql(anMasterIDs);
		// (z.manning 2011-10-25 10:38) - PLID 39401 - Added stamp exclusion data
		sqlSql += "\r\n";
		sqlSql += GenerateStampExclusionSql(&anMasterIDs);
	}

	// Query the data
	if (!sqlSql.IsEmpty()) {
		// (c.haag 2007-08-07 11:16) - PLID 26998 - Use a parameter recordset as
		// it is now the standard means of creating recordsets
		_RecordsetPtr prs = CreateParamRecordset(pConn, "{SQL}", sqlSql);

		// Process the queried records
		if (bLoadEmrInfoData) {
			// (c.haag 2007-08-06 13:15) - PLID 26954 - General data
			ProcessEmrInfoGeneralRecords(prs);
			// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic
			// (c.haag 2007-08-06 13:15) - PLID 26977 - List and table data
			prs = prs->NextRecordset(&vrs);
			ProcessEmrInfoListAndTableItemRecords(prs);
			// (c.haag 2007-08-07 10:41) - PLID 26998 - Process list defaults
			prs = prs->NextRecordset(&vrs);
			ProcessEmrInfoListDefaultRecords(prs);
			//DRT 2/14/2008 - PLID 28603 - Added hotspots
			prs = prs->NextRecordset(&vrs);
			ProcessEmrInfoHotSpotItemRecords(prs);
			// (z.manning 2011-10-25 10:40) - PLID 39401 - Stamp exclusions
			prs = prs->NextRecordset(NULL);
			ProcessStampExclusionRecords(prs);
		}
	}
}

// (j.jones 2011-04-28 14:39) - PLID 43122 - added nProviderIDForFloatingData as a required parameter (send -1 if you don't have a provider)
void CEMNSpawner::PreloadAllData(ADODB::_Connection *lpCon, const long nProviderIDForFloatingData)
{
	// (c.haag 2007-08-06 10:48) - PLID 26954 - This function preloads data
	// for all supported actions
	PreloadEmrItemData(lpCon, nProviderIDForFloatingData);
}

//////////////////////////////////////////////////////////////////////
// Preloaded data access functions
//////////////////////////////////////////////////////////////////////

CEMNSpawner::CEMRInfoItem* CEMNSpawner::GetEmrInfoItem(long nEmrInfoID) const
{
	// (c.haag 2007-08-06 11:41) - PLID 26954 - Returns an EMR info item
	// (c.haag 2007-08-06 15:50) - PLID 26992 - This can return NULL. Calling code
	// should be aware of this and handle such cases accordingly.
	CEMRInfoItem* pItem = NULL;
	m_mapInfoItems.Lookup(nEmrInfoID, pItem);
	return pItem;
}

CEMNSpawner::CEMRInfoItem* CEMNSpawner::GetEmrInfoItemByMasterID(long nEmrInfoMasterID) const
{
	// (c.haag 2007-08-06 15:47) - PLID 26992 - Returns an EMR info item
	// given the Master ID. We currently only support loading Active
	// info items, so this should work.
	CEMRInfoItem* pItem = NULL;
	m_mapInfoItemsByMasterID.Lookup(nEmrInfoMasterID, pItem);
	return pItem;
}

//////////////////////////////////////////////////////////////////////
// State access functions
//////////////////////////////////////////////////////////////////////


// (j.jones 2008-09-22 15:26) - PLID 31408 - added nEMRGroupID
// (z.manning 2011-11-16 12:21) - PLID 38130 - Removed default parameters and added an output parameter for 
// the remembered detail ID from which we loaded the state.
_variant_t CEMNSpawner::LoadEMRDetailStateDefault(long nEmrInfoID, long nPatientID, long nEMRGroupID, IN ADODB::_Connection *lpCon, OUT long &nRememberedDetailID)
{
	// (c.haag 2007-08-06 17:39) - PLID 26998 - Loads a detail's default state
	// using information preloaded into this object. I copied over code from the
	// existing function in EmrUtils.cpp, so there are a lot of existing comments
	// which were added before today. We still require running a query for items 
	// with remembered values for patients.
	CEMRInfoItem* pInfo = GetEmrInfoItem(nEmrInfoID);
	if (NULL == pInfo) {
		// If the item doesn't exist, fall back to the old way of loading
		// (j.jones 2008-09-22 15:27) - PLID 31408 - send nEMRGroupID
		// (z.manning 2011-11-16 12:56) - PLID 38130 - Pass in parameter for remembered detail ID
		return ::LoadEMRDetailStateDefault(NULL, nEmrInfoID, nPatientID, nEMRGroupID, -1, lpCon, nRememberedDetailID);
	}

	_variant_t varNull;
	varNull.vt = VT_NULL;
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();	

	EmrInfoType nDataType = (EmrInfoType)VarByte(pInfo->m_vDataType);
	EmrInfoSubType eDataSubType = (EmrInfoSubType)VarByte(pInfo->m_vDataSubType);
	
	//TES 10/27/2004: If this is one of those items that should be remembered per patient, and if this patient already
	//has this item, then use that.
	// (c.haag 2007-01-25 08:13) - PLID 24416 - The patient ID may be -1 if this is a
	// template or there's simply no patient...in which case there is no reason to run
	// the rsExistingDetail query

	// (j.jones 2008-09-22 15:06) - PLID 31408 - now we support remembering items per patient, or per EMR,
	// the latter will only pull items from other EMNs on the same EMR

	if((nPatientID > 0 && VarBool(pInfo->m_vRememberForPatient)) || (nEMRGroupID != -1 && VarBool(pInfo->m_vRememberForEMR)))
	{
		//TES 12/5/2006 - PLID 23724 - We can just compare on EmrInfoMasterID now.
		// (j.jones 2006-08-22 11:38) - PLID 22157 - we need to check all previous versions of this Info item
		//CString strAllPreviousInfoIDs = GeneratePastEMRInfoIDs(nEmrInfoID);

		// (a.wetta 2007-03-06 10:38) - PLID 25008 - We want to find the last use of this detail for when it had the same data type as it has
		// now.  If the last data type is not the same as its current data type, the value would be invalid.

		// (j.jones 2007-08-02 10:25) - PLID 26912 - added extra fields to be passed into LoadEMRDetailState
		// (z.manning 2011-02-24 17:25) - PLID 42579 - I moved the logic here to its own function.
		_variant_t varState;
		// (z.manning 2011-11-16 12:56) - PLID 38130 - Pass in parameter for remembered detail ID
		if(TryLoadDetailStateFromExistingByInfoID(nEmrInfoID, nDataType, eDataSubType, nPatientID, nEMRGroupID, VarBool(pInfo->m_vRememberForEMR), pCon, varState, nRememberedDetailID)) {
			return varState;
		}
	}
	
	switch(nDataType) {
	case eitText:
		{
			//TES 6/23/2004: Text types now have a default.
			CString strDefault = VarString(pInfo->m_vDefaultText, "");
			if(strDefault == "") {
				return LoadEMRDetailStateBlank(nDataType);
			}
			else {
				return _bstr_t(strDefault);
			}
		}
		break;
	case eitSingleList:
		{
			// (j.jones 2007-08-01 14:37) - PLID 26905 - ActiveData.ID is no longer
			// in the rsEmrInfo recordset, instead we have Info_HasDefaultValue which tells us
			// whether a default value exists. Since spawning non-template details with
			// default values isn't too common, this isn't a huge loss.

			long nHasDefaultValue = VarLong(pInfo->m_vHasDefaultValue, 0);
			if(nHasDefaultValue == 0)
				return LoadEMRDetailStateBlank(nDataType);

			//otherwise, if we have a default value, we have to load it

			const int nSelections = pInfo->m_arDefaultListSelections.GetSize();
			if (0 == nSelections) {
				return LoadEMRDetailStateBlank(nDataType);
			} else {
				// (c.haag 2007-08-07 10:44) - PLID 26998 - I just want to point out that the
				// legacy code also did not care which, if there were multiple default selections,
				// caused by bad data, is returned.
				//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t (although it would
				// appear that this function is expected to return one of type VT_BSTR, not VT_I4, so we'll keep the
				// AsString() in there). 
				return _variant_t(AsString(pInfo->m_arDefaultListSelections[0]));
			}
		}
		break;
	case eitMultiList:
		// (j.jones 2004-10-22 14:47) - Load all the defaults into a semi-colon delimited list
		{
			CString strDefault = "";

			// (j.jones 2007-08-01 14:37) - PLID 26905 - ActiveData.ID is no longer
			// in the rsEmrInfo recordset, instead we have Info_HasDefaultValue which tells us
			// whether a default value exists. Since spawning non-template details with
			// default values isn't too common, this isn't a huge loss.

			const long nHasDefaultValue = VarLong(pInfo->m_vHasDefaultValue, 0);
			if(nHasDefaultValue == 0)
				return LoadEMRDetailStateBlank(nDataType);

			const int nSelections = pInfo->m_arDefaultListSelections.GetSize();
			if (0 == nSelections) {
				return LoadEMRDetailStateBlank(nDataType);
			} else {
				for (int i=0; i < nSelections; i++) {
					const long nDefault = pInfo->m_arDefaultListSelections[i];
					if(nDefault == -1)
						return LoadEMRDetailStateBlank(nDataType);
					else {
						strDefault += AsString(nDefault);
						strDefault += "; ";
					}
				}
				strDefault.TrimRight("; ");
				//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
				return _variant_t(strDefault);
			}
		}
		break;
	case eitImage: {
		// Image type info items don't currently have default values.
		//return LoadEMRDetailStateBlank(nDataType);
		//
		// (c.haag 2006-11-09 17:47) - PLID 23365 - Whenever we pull BackgroundImageType, we could
		// get a null value. If so, it means we pulled it from an EmrInfo item where the user must
		// choose the image to assign to the detail. In that event, we need to assign "blank default
		// values" to the image state. In our program, this means clearing the path and assigning the
		// image type of itDiagram.

		// (c.haag 2007-02-09 15:40) - PLID 23365 - The previous comment is wrong. The simple story
		// is that EmrInfoT.BackgroundImageType is NULL if there is no default image for the info
		// item. When the detail is added, it should have no image unless the detail itself is assigned
		// one in its InkImage*Override fields. The user will have to pick an image.
		//
		CEmrItemAdvImageState ais;
		if (itUndefined == (ais.m_eitImageTypeOverride = (eImageType)VarLong(pInfo->m_vBackgroundImageType, -1))) {
			ais.m_strImagePathOverride.Empty();
		} else {
			ais.m_strImagePathOverride = VarString(pInfo->m_vBackgroundImageFilePath, "");
		}
		return ais.AsSafeArrayVariant();

	   } break;
	case eitSlider:
		//Sliders don't have defaults yet (though they probably should)
		return LoadEMRDetailStateBlank(nDataType);
	case eitNarrative:
		{
			//TES 6/23/2004: Text types now have a default.
			CString strDefault = VarString(pInfo->m_vDefaultText, "");
			if(strDefault == "") {
				return LoadEMRDetailStateBlank(nDataType);
			}
			else {
				return _bstr_t(strDefault);
			}
		}
		break;
	case eitTable:
		// (j.jones 2007-08-01 16:56) - PLID 26905 - returned if not an EMRTemplateDetail
		return "";
	default:
		ASSERT(FALSE);
		return varNull;
		break;
	}
}