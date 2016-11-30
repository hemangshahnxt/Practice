#ifndef __EMNSPAWNER_H__
#define __EMNSPAWNER_H__

#pragma once

/*	(c.haag 2007-08-06 10:47) - PLID 26954 - The CEMNSpawner object preloads 
	spawning-related data in bulk so that we minimize the number of round trips 
	to the SQL server during spawning actions. Everything currently runs in a
	single thread, so there is no need for mutex support.


	(c.haag 2007-08-06 10:47) - Version 1 - Only used for spawning individual
	EMR items. We preload all data necessary for CEMNDetail::LoadContent.
*/

#include <afxtempl.h>
#include "EmrUtils.h"
#include "EmnDetailStructures.h"
#include "WoundCareCalculator.h"

enum GlassesOrderDataType;

class CEMNSpawner
{
private:
	// (c.haag 2007-08-06 11:28) - PLID 26954 - The array of spawning actions.
	// This is only assigned upon construction, and is used to find out what
	// we want to preload.
	// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
	MFCArray<EmrAction> m_arActions;

	// (c.haag 2007-08-06 13:09) - PLID 26977 - TRUE if the CEMN we are
	// interfacing with is a template. False if it's a patient chart.
	BOOL m_bIsTemplate;

public:
	// (c.haag 2007-08-06 13:09) - PLID 26977 - This structure describes a EmrDataT
	// record. This is used in CEMRInfoItem objects, primarily in calls to
	// CEMNDetail::LoadContent
	struct EmrDataItem
	{
		long m_nID;
		long m_nEMRDataGroupID;
		CString m_strData;
		BOOL m_bIsLabel;
		long m_nListType;
		BYTE m_nListSubType; // (z.manning 2010-02-16 14:40) - PLID 37230
		BOOL m_bAutoAlphabetizeDropdown; // (c.haag 2010-02-24 14:16) - PLID 21301 - EmrDataT.AutoAlphabetizeDropdown
		BOOL m_bIsGrouped;
		CString m_strLongForm;
		// The following are calculated variables used only in CEMNDetail::LoadContent
		long m_nActionsType;
		// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
		CString m_strFormula;
		BYTE m_nDecimalPlaces;
		// (z.manning 2009-01-15 15:31) - PLID 32724 - Added InputMask
		CString m_strInputMask;
		// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
		EEmrTableAutoNumberType m_etantAutoNumberType;
		CString m_strAutoNumberPrefix;
		BOOL m_bHasDropdownElements; // (z.manning 2011-03-11) - PLID 42778
		BOOL m_bHasActiveDropdownElements; // (z.manning 2011-03-17 15:06) - PLID 42778
		GlassesOrderDataType m_GlassesOrderDataType; //TES 3/17/2011 - PLID 41108
		long m_nGlassesOrderDataID; //TES 3/17/2011 - PLID 41108
		EmrTableAutofillType m_eAutofillType; // (z.manning 2011-03-21 11:22) - PLID 30608
		// (j.jones 2011-04-28 14:39) - PLID 43122 - track if this list item is floated,
		// which will require that we bold the text
		BOOL m_bIsFloated;
		long m_nFlags; // (z.manning 2011-05-26 14:47) - PLID 43865
		// (z.manning 2011-09-19 17:14) - PLID 41954 - Dropdown separators
		CString m_strDropdownSeparator;
		CString m_strDropdownSeparatorFinal;
		CString m_strSpawnedItemsSeparator; // (z.manning 2011-11-07 11:10) - PLID 46309
		// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType, which marks this data item as being special to the CWoundCareCalculator
		EWoundCareDataType m_ewccWoundCareDataType;
		// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
		long m_nParentLabelID;

		// (a.walling 2012-10-12 15:05) - PLID 53165 - Added default constructor
		EmrDataItem()
			: m_nID(-1)
			, m_nEMRDataGroupID(-1)
			, m_bIsLabel(FALSE)
			, m_nListType(0)
			, m_nListSubType(0)
			, m_bAutoAlphabetizeDropdown(FALSE)
			, m_bIsGrouped(FALSE)
			, m_nActionsType(0)
			, m_nDecimalPlaces(0)
			, m_etantAutoNumberType(etantInvalid)
			, m_bHasDropdownElements(FALSE)
			, m_bHasActiveDropdownElements(FALSE)
			, m_GlassesOrderDataType(godtInvalid)
			, m_nGlassesOrderDataID(-1)
			, m_eAutofillType(etatNone)
			, m_bIsFloated(FALSE)
			, m_nFlags(0)
			, m_ewccWoundCareDataType(wcdtNone)
			, m_nParentLabelID(-1)
		{}
	};

	//DRT 2/14/2008 - PLID 28603 - This structure describes an EMRImageHotSpotsT record.  Used to load
	//	the hotspots for CEMNDetail::LoadContent
	struct EmrImageHotSpotItem
	{
		long m_nID;
		long m_nEMRHotSpotGroupID;
		CString m_strData;
		//TES 2/10/2010 - PLID 37298 - Added Anatomic Location information
		long m_nAnatomicLocationID;
		CString m_strAnatomicLocation;
		long m_nAnatomicQualifierID;
		CString m_strAnatomicQualifier;
		AnatomySide m_asSide;
		short m_n3DHotSpotID; // (z.manning 2011-07-25 12:55) - PLID 44649
	};


	// (c.haag 2007-08-06 11:31) - PLID 26954 - This is a store of EMR info items.
	// This information is accessed in CEMNDetail::LoadContent
	class CEMRInfoItem
	{
	public:
		long m_nID;
		// (c.haag 2007-08-06 15:43) - PLID 26992 - EmrInfoMasterID
		long m_nMasterID;
	public:
		// Slider variables
		_variant_t m_vSliderMin;
		_variant_t m_vSliderMax;
		_variant_t m_vSliderInc;
	public:
		// Image variables
		_variant_t m_vBackgroundImageFilePath;
		_variant_t m_vBackgroundImageType;
	public:
		// Other variables
		_variant_t m_vLongForm;
		_variant_t m_vDataFormat;
		_variant_t m_vDataSeparator;
		_variant_t m_vDataSeparatorFinal;
		_variant_t m_vHasInfoActions;
		_variant_t m_vDisableTableBorder;
		_variant_t m_vDataType;
		_variant_t m_vTableRowsAsFields; // (c.haag 2008-10-16 11:26) - PLID 31709
		// (c.haag 2007-08-06 14:20) - PLID 26977 - For list sorting
		_variant_t m_vAutoAlphabetizeListData;
		// (c.haag 2007-08-06 15:44) - PLID 26992 - Name and sub type
		_variant_t m_vName;
		_variant_t m_vDataSubType;
		// (c.haag 2007-08-07 10:12) - PLID 26998 - For LoadEMRDetailStateDefault
		_variant_t m_vRememberForPatient;
		_variant_t m_vRememberForEMR;	// (j.jones 2008-09-22 15:07) - PLID 31408 - added
		_variant_t m_vDefaultText;
		_variant_t m_vHasDefaultValue;

		// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields,
		// because they are now only calculated in the API, and not in Practice code
		/*
		// (j.jones 2007-08-27 10:35) - PLID 27056 - added the E/M coding data
		_variant_t m_vEMCodeCategoryID;
		// (j.jones 2011-03-09 09:05) - PLID 42283 - added m_vEMCodeUseTableCategories
		_variant_t m_vEMCodeUseTableCategories;
		_variant_t m_vUseEMCoding;
		_variant_t m_vEMCodingType;
		*/

		// (a.walling 2008-06-30 13:40) - PLID 29271 - Preview Pane flags
		_variant_t m_vPreviewFlags;

		// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
		_variant_t m_vChildEMRInfoMasterID;
		_variant_t m_vSmartStampsEnabled;

		//TES 3/17/2011 - PLID 41108 - Added Glasses Order data
		_variant_t m_vHasGlassesOrderData;
		_variant_t m_vGlassesOrderLens;
		//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
		_variant_t m_vHasContactLensData;
		// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding, which marks this table item as being special to the CWoundCareCalculator
		_variant_t m_vUseWithWoundCareCoding;

		_variant_t m_vInfoFlags; // (z.manning 2011-11-15 17:04) - PLID 38130
	public:
		// (c.haag 2007-08-06 12:51) - PLID 26977 - This is an array of all the EmrDataT
		// records connected to this item. This is only used in CEMNDetail::LoadContent 
		// at the time of this comment
		CArray<EmrDataItem,EmrDataItem&> m_arDataItems;
		//DRT 2/14/2008 - PLID 28603 - Added hotspots
		CArray<EmrImageHotSpotItem, EmrImageHotSpotItem&> m_aryHotSpotItems;
		// (c.haag 2007-08-07 10:37) - PLID 26998 - This is an array of all default
		// selections for list items. Each entry is an ID in EmrDataT.
		CArray<long,long> m_arDefaultListSelections;

		CEmrItemStampExclusions m_StampExclusions; // (z.manning 2011-10-25 10:45) - PLID 39401

		// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic
	};

private:
	CMap<long,long,CEMRInfoItem*,CEMRInfoItem*> m_mapInfoItems;

	// (c.haag 2007-08-06 15:49) - PLID 26992 - Map by EmrInfoMasterID.
	// No duplicates are expected or allowed.
	CMap<long,long,CEMRInfoItem*,CEMRInfoItem*> m_mapInfoItemsByMasterID;

public:
	// (c.haag 2007-08-06 13:09) - PLID 26977 - Added bIsTemplate for loading list data
	CEMNSpawner(const MFCArray<EmrAction>& arActions, BOOL bIsTemplate);
	~CEMNSpawner();

//////////////////////////////////////////////////////////////////////
// Query generation and processing functions
//////////////////////////////////////////////////////////////////////
private:
	// (c.haag 2007-08-06 11:18) - PLID 26954 - This function generates a SQL
	// statement to pull general EmrInfoT fields in the preloading of EMR item data.
	// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
	CSqlFragment GenerateEmrInfoGeneralSql(const CArray<long, long>& anMasterIDs) const;

	// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic
	// (c.haag 2007-08-06 11:25) - PLID 26954 - This function process records
	// returned from a query which used GenerateEmrInfoGeneralSql
	void ProcessEmrInfoGeneralRecords(ADODB::_RecordsetPtr& prs);

	// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic

	// (c.haag 2007-08-06 13:00) - PLID 26977 - This function generates a SQL
	// statement to pull single and multi-select list and table information for use with
	// CEMNDetail::LoadContent.
	// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
	// (j.jones 2011-04-28 14:39) - PLID 43122 - added nProviderIDForFloatingData as a required parameter (send -1 if you don't have a provider)
	CSqlFragment GenerateEmrInfoListAndTableItemSql(const CArray<long, long>& anMasterIDs, const long nProviderIDForFloatingData) const;

	//DRT 2/14/2008 - PLID 28603 - Generates SQL for loading images with hotspots
	// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
	CSqlFragment GenerateEmrInfoHotSpotsSql(const CArray<long, long>& anMasterIDs) const;

	// (z.manning 2011-10-25 10:36) - PLID 39401
	CSqlFragment GenerateStampExclusionSql(const CArray<long,long> *parynMasterIDs) const;

	// (c.haag 2007-08-06 13:01) - PLID 26977 - This function processes records
	// returned from a query which used GenerateEmrInfoListAndTableItemSql
	void ProcessEmrInfoListAndTableItemRecords(ADODB::_RecordsetPtr& prs);

	//DRT 2/14/2008 - PLID 28603 - Process hotspots into our info item
	void ProcessEmrInfoHotSpotItemRecords(ADODB::_RecordsetPtr& prs);

	// (z.manning 2011-10-25 10:41) - PLID 39401
	void ProcessStampExclusionRecords(ADODB::_RecordsetPtr& prs);

	// (c.haag 2007-08-07 10:31) - PLID 26998 - This function generates a SQL statement to pull
	// single and multi-select list default selections for use with LoadEMRDetailStateDefault.
	// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
	CSqlFragment GenerateEmrInfoListDefaultsSql(const CArray<long, long>& anMasterIDs) const;

	// (c.haag 2007-08-07 10:33) - PLID 26998 - This function processes records from a query
	// whose text was generated by GenerateEmrInfoListDefaultsSql
	void ProcessEmrInfoListDefaultRecords(ADODB::_RecordsetPtr& prs);

//////////////////////////////////////////////////////////////////////
// Preload functions
//////////////////////////////////////////////////////////////////////
private:
	// (c.haag 2007-08-06 10:47) - PLID 26954 - This function preloads data
	// related to spawning individual EMR items
	// (j.jones 2011-04-28 14:39) - PLID 43122 - added nProviderIDForFloatingData as a required parameter (send -1 if you don't have a provider)
	void PreloadEmrItemData(ADODB::_Connection *lpCon, const long nProviderIDForFloatingData);

public:
	// (c.haag 2007-08-06 10:48) - PLID 26954 - This function preloads data
	// for all supported actions
	// (j.jones 2011-04-28 14:39) - PLID 43122 - added nProviderIDForFloatingData as a required parameter (send -1 if you don't have a provider)
	void PreloadAllData(ADODB::_Connection *lpCon, const long nProviderIDForFloatingData);

//////////////////////////////////////////////////////////////////////
// Preloaded data access functions
//////////////////////////////////////////////////////////////////////
public:
	// (c.haag 2007-08-06 11:41) - PLID 26954 - Returns an EMR info item
	CEMRInfoItem* GetEmrInfoItem(long nEmrInfoID) const;

	// (c.haag 2007-08-06 15:47) - PLID 26992 - Returns an EMR info item
	// given the Master ID. We currently only support loading Active
	// info items, so this should work.
	CEMRInfoItem* GetEmrInfoItemByMasterID(long nEmrInfoMasterID) const;

//////////////////////////////////////////////////////////////////////
// State access functions
//////////////////////////////////////////////////////////////////////
public:
	// (c.haag 2007-08-06 17:39) - PLID 26998 - Loads a detail's default state
	// using information preloaded into this object
	// (j.jones 2008-09-22 15:26) - PLID 31408 - added nEMRGroupID
	// (z.manning 2011-11-16 12:21) - PLID 38130 - Removed default parameters and added an output parameter for 
	// the remembered detail ID from which we loaded the state.
	_variant_t LoadEMRDetailStateDefault(long nEmrInfoID, long nPatientID, long nEMRGroupID, IN ADODB::_Connection *lpCon, OUT long &nRememberedDetailID);
};

#endif