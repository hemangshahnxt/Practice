#include "stdafx.h"
#include "EMR.h"
#include "EMRTopic.h"
#include "EmrEditorDlg.h"
#include "ShowProgressFeedbackDlg.h"
#include "AuditTrail.h"
#include "EMNSpawner.h"
#include "EMRItemAdvMultiPopupDlg.h"
#include "EmrUtils.h"
#include "NewCropBrowserDlg.h"
#include "DecisionRuleUtils.h"
#include "PicContainerDlg.h"	// (j.dinatale 2012-07-12 14:19) - PLID 51481
#include "WoundCareCalculator.h" // (r.gonet 08/03/2012) - PLID 51948
#include "DontShowDlg.h"
#include "EmrCodingGroupManager.h"
#include "EMNDetail.h"
#include "EMNDetailStructures.h"
#include <boost/foreach.hpp>
#include "EMRHotSpot.h"
#include "WriteTokenInfo.h"
#include "NxAutoQuantum.h"
#include "DiagQuickListUtils.h"
#include "EMN.h"

// (a.walling 2011-08-11 16:43) - PLID 45021 - TableRow.m_pID is now TableRow.m_ID, which is not allocated on the heap.

// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray

//
//
// (c.haag 2007-04-24 09:17) - PLID 25881 - All "preloader" code related to
// LoadEMRTopic being called during the initial load of an EMN is now done in the
// CEMNLoader object.
// 
//
#include "EMNLoader.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (j.jones 2012-08-22 09:25) - PLID 50486 - added types for m_nDefaultChargeInsuredPartyID,
// if it is not a legitimate insured party ID it should be one of these values
namespace DefaultChargeInsuredPartyIDType
{
	enum Type
	{
		Unknown = -3,
		NoDefault = -2,
		PatientResp = -1,
	};
}

using namespace ADODB;
//Utility function called from the main thread.
void LoadEMRTopic(EMRTopicLoadInfo *pLoadInfo, ADODB::_Connection *lpCon, CEMR *pParentEmr)
{
	//
	// (c.haag 2007-07-19 09:29) - PLID 26744 - This function has been instrumented with catch-throws
	// for more precise exception handling (meaning easier debugging)
	//
	_ConnectionPtr pConn;
	try {
		pConn = lpCon;
		if (NULL == pLoadInfo) {
			ThrowNxException("The pLoadInfo value is invalid!"); 
		}
	} NxCatchAllThrowThread("Error in LoadEMRTopic() - Initialization");

	// (c.haag 2007-04-24 09:14) - PLID 25881 - We now make the loader object do all the work
	// (c.haag 2007-02-27 10:08) - PLID 24949 - If pLoadInfo is not null, then
	// we pull detail data from it rather than opening a recordset
	CEMNLoader* pLoader = pLoadInfo->m_pLoader;
	if (NULL != pLoader) {
		// (c.haag 2007-07-19 10:09) - PLID 26744 - The function being called has its
		// own granular try/catch/throw instrumentation
		pLoader->LoadEMRTopic(pLoadInfo, lpCon, pParentEmr);

		try {
			// (c.haag 2007-08-25 11:58) - PLID 25881 - The following code used to be in CEMNLoader::LoadEMRTopic,
			// but when the main thread gets the NXM_TOPIC_LOAD_COMPLETE_BY_EMN_LOADER message, it can delete the loader.
			// (c.haag 2007-08-25 11:54) - PLID 25881, 25853 - Release a reference now that we're done with the topic.
			// The final release will be done in CEMN::PostInitialLoad() because the CEMN object owns a reference too.
			pLoader->Release();

			//We're all done!
			pLoadInfo->m_bCompletelyLoaded = TRUE;
			//Let the topic do any post-load processing.
			if(pLoadInfo->hWnd) {
				if(::IsWindow(pLoadInfo->hWnd)) {
					// (c.haag 2007-05-08 16:16) - PLID 25941 - Instead of posting NXM_TOPIC_LOAD_COMPLETE
					// with a topic ID, just post a NXM_TOPIC_LOAD_COMPLETE_BY_EMN_LOADER message with the
					// topic object itself. This way, we don't need to have the main thread go fishing for
					// the topic object given its ID.
					CEMRTopic* pTopic = NULL;
					// (c.haag 2007-07-03 09:57) - PLID 26523 - Take exclusive ownership of topics until this is done
					CHoldEMNLoaderMutex mh(pLoader->GetTopicsMutex());
					if(pLoadInfo->bLoadFromTemplate) {
						// (c.haag 2007-07-24 14:35) - PLID 26334 - Do additional NULL checks
						CEMNLoader::CPreloadedTemplateTopic* pPreloadedTopic = pLoader->GetPreloadedTemplateTopicByID(pLoadInfo->nID);
						if (pPreloadedTopic) pTopic = pPreloadedTopic->m_pTopic;
					} else {
						CEMNLoader::CPreloadedTopic* pPreloadedTopic = pLoader->GetPreloadedTopicByID(pLoadInfo->nID);
						if (pPreloadedTopic) pTopic = pPreloadedTopic->m_pTopic;
					}
					mh.Release(); // We're done with the topics
					if (NULL != pTopic) {
						::PostMessage(pLoadInfo->hWnd, NXM_TOPIC_LOAD_COMPLETE_BY_EMN_LOADER, (WPARAM)pTopic, (LPARAM)0);
					} else {
						ASSERT(FALSE);
						ThrowNxException("Could not find loaded topic %d for post-load processing!", pLoadInfo->nID);
					}
				}
			}
			return;

		} NxCatchAllThrowThread("Error in LoadEMRTopic() - Post-preloader completion");
	}

	//The topic structure is much like the old tab structure but can either have
	//details or other topics underneath it. So to load a topic, we need to use
	//this CEMRTopic::LoadFromID() function recursively

	//load this topic's "topic-level" data
	_RecordsetPtr rsTopic;
	if(pLoadInfo->bLoadFromTemplate) {
		try {
			//DRT 9/25/2007 - PLID 27515 - Added SourceActionSourceID & SourceActionSourceDataGroupID
			// (z.manning, 01/23/2008) - PLID 28690 - Added EmrImageHotSpotGroupID
			// (z.manning 2009-02-13 10:21) - PLID 33070 - SourceActionSourceTableDropdownGroupID
			// (z.manning 2009-03-05 11:40) - PLID 33338 - SourceDataGroupID
			//TES 2/16/2010 - PLID 37298 - Added SourceActionName info for HotSpots
			//TES 3/17/2010 - PLID 37530 - Added SourceStampID and SourceStampIndex
			//TES 3/18/2010 - PLID 37530 - Added SourceActionName info for Smart Stamps
			// (a.walling 2010-04-02 17:44) - PLID 38059
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			rsTopic = CreateParamRecordset(pConn, "SELECT EMRTemplateTopicsT.Name, SourceEMRTemplateTopicsT.Name AS SourceName, "
				"EMRTemplateTopicsT.ID AS EMRTemplateTopicID, EMRTemplateTopicsT.SourceTemplateTopicID, "
				"EMRTemplateTopicsT.ShowIfEmpty, SourceEMRTemplateTopicsT.ShowIfEmpty AS SourceShowIfEmpty, "
				"EMRTemplateTopicsT.TemplateID, EMRTemplateTopicsT.OrderIndex AS TemplateOrderIndex, "
				"EMRTemplateTopicsT.OrderIndex AS OrderIndex, EMRTemplateTopicsT.EMRParentTemplateTopicID, "
				"EMRTemplateTopicsT.HideOnEMN, "
				"CASE WHEN EmrActionsT.SourceType = {CONST} THEN EmrInfoQ.Name WHEN EmrActionsT.SourceType = {CONST} THEN EmrDataQ.Data WHEN EmrActionsT.SourceType = {CONST} THEN EmrProcedureQ.Name WHEN EmrActionsT.SourceType = {CONST} THEN EmrImageHotSpotQ.AnatomicLocation "
				// (a.walling 2010-04-02 17:27) - PLID 38059 - Gather the action name for table dropdown spawns
				" WHEN EmrActionsT.SourceType = {CONST} THEN EmrImageStampsT.StampText + ' - ' + convert(nvarchar(50),EMRTemplateTopicsT.SourceStampIndex) WHEN EmrActionsT.SourceType = 13 THEN EmrTableDropdownQ.DropdownData + ' - ' + EmrTableDropdownRowQ.Data + ' - ' + EmrTableDropdownQ.DropdownTableColumn ELSE '' END AS SourceActionName, "
				"EmrActionsT.DestType AS SourceActionDestType, EmrActionsT.ID AS SourceActionID, "
				"EMRActionsT.SourceID AS SourceActionSourceID, EMRDataQ.EMRDataGroupID AS SourceActionSourceDataGroupID, {INT} AS SourceDetailID, "
				"SourceEMRTemplateTopicsT.TemplateID AS SourceTemplateID, " // (a.walling 2007-03-21 09:45) - PLID 25301 - Include source template
				// (a.walling 2008-06-30 13:09) - PLID 29271 - Preview Pane flags
				"EmrImageHotSpotQ.EmrSpotGroupID, EMRTemplateTopicsT.PreviewFlags, EmrTableDropdownQ.DropdownGroupID AS SourceActionSourceTableDropdownGroupID, "
				"EMRTemplateTopicsT.SourceDataGroupID, EmrActionsT.SourceType, NULL AS SourceDetailImageStampID, "
				"EMRTemplateTopicsT.SourceStampID, EMRTemplateTopicsT.SourceStampIndex "
				"FROM EMRTemplateTopicsT "
				"LEFT JOIN EMRTemplateTopicsT AS SourceEMRTemplateTopicsT ON EMRTemplateTopicsT.SourceTemplateTopicID = SourceEMRTemplateTopicsT.ID "
				"LEFT JOIN EmrActionsT ON EmrActionsT.ID = {INT} "
				// (a.walling 2010-04-06 08:16) - PLID 38061 - Filtered these on their appropriate source types
				"LEFT JOIN (SELECT ID, Name FROM EmrInfoT) AS EmrInfoQ ON EmrInfoQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 3  "
				"LEFT JOIN (SELECT ID, Data, EMRDataGroupID FROM EmrDataT) AS EmrDataQ ON EmrDataQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 4  "
				"LEFT JOIN (SELECT ID, Name FROM ProcedureT) AS EmrProcedureQ ON EmrProcedureQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 5  "
				"LEFT JOIN (SELECT EmrImageHotSpotsT.ID, EmrImageHotSpotsT.EmrSpotGroupID, "
				// (z.manning 2010-04-30 15:29) - PLID 37553 - We now have a view to pull anatomic location
				"EmrHotSpotAnatomicLocationQ.AnatomicLocation "
				"FROM EmrImageHotSpotsT  "
				"LEFT JOIN EmrHotSpotAnatomicLocationQ ON EmrImageHotSpotsT.ID = EmrHotSpotAnatomicLocationQ.EmrHotSpotID) AS EmrImageHotSpotQ ON EmrImageHotSpotQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = {CONST}  "
				// (a.walling 2010-04-02 16:32) - PLID 38059
				"LEFT JOIN (SELECT EmrTableDropdownInfoT.ID, EmrTableDropdownInfoT.DropdownGroupID, EmrTableDropdownInfoT.Data AS DropdownData, EMRDataTableColumnQ.Data AS DropdownTableColumn "
					"FROM EmrTableDropdownInfoT "
					"INNER JOIN EMRDataT EMRDataTableColumnQ ON EMRTableDropdownInfoT.EMRDataID = EMRDataTableColumnQ.ID "
					"AND EMRDataTableColumnQ.ListType IN (3,4) " // (a.walling 2013-02-28 17:35) - PLID 55391 - This eliminates thousands of seeks!
					") AS EmrTableDropdownQ ON EmrTableDropdownQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = {CONST} "
				// (a.walling 2010-04-02 16:42) - PLID 38059
				"LEFT JOIN EMRTemplateDetailsT SourceEMRDetailQ ON EMRTemplateTopicsT.SourceDetailID = SourceEMRDetailQ.ID "
				"LEFT JOIN EMRInfoMasterT SourceEMRInfoMasterQ ON SourceEMRDetailQ.EMRInfoMasterID = SourceEMRInfoMasterQ.ID "
				"LEFT JOIN EMRDataT EmrTableDropdownRowQ ON EMRTemplateTopicsT.SourceDataGroupID = EMRTableDropdownRowQ.EmrDataGroupID AND SourceEMRInfoMasterQ.ActiveEMRInfoID = EmrTableDropdownRowQ.EMRInfoID AND EmrActionsT.SourceType = 13 "

				"LEFT JOIN EmrImageStampsT ON EmrTemplateTopicsT.SourceStampID = EmrImageStampsT.ID "
				"WHERE EMRTemplateTopicsT.ID = {INT}",
				eaoEmrItem, eaoEmrDataItem, eaoProcedure, eaoEmrImageHotSpot, eaoSmartStamp,
				pLoadInfo->m_saiOverride.nSourceDetailID,
				pLoadInfo->m_saiOverride.nSourceActionID,
				eaoEmrImageHotSpot, eaoEmrTableDropDownItem,
				pLoadInfo->nID);
		} NxCatchAllThrowThread("Error in LoadEMRTopic() - Querying template topic data");
	}
	else {
		try {
			//DRT 9/25/2007 - PLID 27515 - Added SourceActionSourceID & SourceActionSourceDataGroupID
			// (z.manning, 01/23/2008) - PLID 28690 - Added EmrImageHotSpotGroupID
			// (z.manning 2009-02-13 10:24) - PLID 33070 - SourceActionSourceTableDropdownGroupID
			// (z.manning 2009-03-05 11:40) - PLID 33338 - SourceDataGroupID
			//TES 2/16/2010 - PLID 37298 - Added SourceActionName info for HotSpots
			// (z.manning 2010-02-25 10:33) - PLID 37532 - SourceDetailImageStampID
			//TES 3/17/2010 - PLID 37530 - Added SourceStampID and SourceStampIndex
			//TES 3/18/2010 - PLID 37530 - Added SourceActionName info for Smart Stamps
			// (a.walling 2010-04-02 17:18) - PLID 38059
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			rsTopic = CreateParamRecordset(pConn, "SELECT EmrTopicsT.Name, EmrTemplateTopicToTopicLinkT.EMRTemplateTopicID, "
				"EmrTopicsT.SourceActionID, EMRActionsT.SourceID AS SourceActionSourceID, EMRDataQ.EMRDataGroupID AS SourceActionSourceDataGroupID, "
				"EmrTopicsT.SourceDetailID, EmrTemplateTopicsT.SourceTemplateTopicID, EmrTopicsT.ShowIfEmpty, "
				"EmrTemplateTopicsT.TemplateID, EmrTemplateTopicsT.OrderIndex AS TemplateOrderIndex, EMRTopicsT.OrderIndex, "
				"CASE WHEN EmrActionsT.SourceType = {CONST} THEN EmrInfoQ.Name WHEN EmrActionsT.SourceType = {CONST} THEN EmrDataQ.Data WHEN EmrActionsT.SourceType = {CONST} THEN EmrProcedureQ.Name WHEN EmrActionsT.SourceType = {CONST} THEN EmrImageHotSpotQ.AnatomicLocation "
				// (a.walling 2010-04-02 17:19) - PLID 38059 - Gather the action name for table dropdown spawns
				" WHEN EmrActionsT.SourceType = {CONST} THEN EmrDetailImageStampsQ.StampText + ' - ' + convert(nvarchar(50),EmrDetailImageStampsQ.IndexByStamp) WHEN EmrActionsT.SourceType = 13 THEN EmrTableDropdownQ.DropdownData + ' - ' + EmrTableDropdownRowQ.Data + ' - ' + EmrTableDropdownQ.DropdownTableColumn ELSE '' END AS SourceActionName, "
				"EmrActionsT.DestType AS SourceActionDestType, SourceEMRTemplateTopicsT.TemplateID AS SourceTemplateID, EMRTemplateTopicsT.SourceTemplateTopicID,  "
				// (a.walling 2008-06-30 13:09) - PLID 29271 - Preview Pane flags
				"EmrImageHotSpotQ.EmrSpotGroupID, EMRTopicsT.PreviewFlags, EmrTableDropdownInfoT.DropdownGroupID AS SourceActionSourceTableDropdownGroupID, "
				"EMRTopicsT.SourceDataGroupID, EmrActionsT.SourceType, EmrTopicsT.SourceDetailImageStampID, "
				"NULL AS SourceStampID, NULL AS SourceStampIndex "
				"FROM EMRTopicsT "
				"LEFT JOIN EmrTemplateTopicToTopicLinkT ON EmrTopicsT.ID = EmrTemplateTopicToTopicLinkT.EmrTopicID "
				"LEFT JOIN EmrTemplateTopicsT ON EmrTemplateTopicToTopicLinkT.EmrTemplateTopicID = EmrTemplateTopicsT.ID "
				"LEFT JOIN EMRTemplateTopicsT AS SourceEMRTemplateTopicsT ON EMRTemplateTopicsT.SourceTemplateTopicID = SourceEMRTemplateTopicsT.ID " // (a.walling 2007-03-21 15:52) - PLID 25301 - include source template
				"LEFT JOIN EmrActionsT ON EmrActionsT.ID = EmrTopicsT.SourceActionID "
				// (a.walling 2010-04-06 08:16) - PLID 38061 - Filtered these on their appropriate source types
				"LEFT JOIN (SELECT ID, Name FROM EmrInfoT) AS EmrInfoQ ON EmrInfoQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 3  "
				"LEFT JOIN (SELECT ID, Data, EMRDataGroupID FROM EmrDataT) AS EmrDataQ ON EmrDataQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 4  "
				"LEFT JOIN (SELECT ID, Name FROM ProcedureT) AS EmrProcedureQ ON EmrProcedureQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 5  "
				"LEFT JOIN (SELECT EmrImageHotSpotsT.ID, EmrImageHotSpotsT.EmrSpotGroupID, "
				// (z.manning 2010-04-30 15:30) - PLID 37553 - We now pull anatomic location from a view
				"EmrHotSpotAnatomicLocationQ.AnatomicLocation "
				"FROM EmrImageHotSpotsT "
				"LEFT JOIN EmrHotSpotAnatomicLocationQ ON EmrImageHotSpotsT.ID = EmrHotSpotAnatomicLocationQ.EmrHotSpotID) AS EmrImageHotSpotQ ON EmrImageHotSpotQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = {CONST}  "
				// (a.walling 2010-04-02 16:32) - PLID 38059
				"LEFT JOIN (SELECT EmrTableDropdownInfoT.ID, EmrTableDropdownInfoT.DropdownGroupID, EmrTableDropdownInfoT.Data AS DropdownData, EMRDataTableColumnQ.Data AS DropdownTableColumn "
					"FROM EmrTableDropdownInfoT "
					"INNER JOIN EMRDataT EMRDataTableColumnQ ON EMRTableDropdownInfoT.EMRDataID = EMRDataTableColumnQ.ID "
					"AND EMRDataTableColumnQ.ListType IN (3,4) " // (a.walling 2013-02-28 17:35) - PLID 55391 - This eliminates thousands of seeks!
					") AS EmrTableDropdownQ ON EmrTableDropdownQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = {CONST} "
				// (a.walling 2010-04-02 16:42) - PLID 38059
				"LEFT JOIN EMRDetailsT SourceEMRDetailQ ON EMRTopicsT.SourceDetailID = SourceEMRDetailQ.ID "
				"LEFT JOIN EMRDataT EmrTableDropdownRowQ ON EMRTopicsT.SourceDataGroupID = EMRTableDropdownRowQ.EmrDataGroupID AND SourceEMRDetailQ.EMRInfoID = EmrTableDropdownRowQ.EMRInfoID AND EmrActionsT.SourceType = 13 "
				"LEFT JOIN (SELECT EmrDetailImageStampsT.ID, EmrImageStampsT.StampText, "
				" (SELECT Count(*) FROM EmrDetailImageStampsT OtherStamps WHERE OtherStamps.EmrDetailID = EmrDetailImageStampsT.EmrDetailID "
				" AND OtherStamps.EmrImageStampID = EmrDetailImageStampsT.EmrImageStampID "
				" AND OtherStamps.OrderIndex < EmrDetailImageStampsT.OrderIndex) + 1 AS IndexByStamp "
				" FROM EmrDetailImageStampsT INNER JOIN EmrImageStampsT ON EmrDetailImageStampsT.EmrImageStampID = EmrImageStampsT.ID "
				") EmrDetailImageStampsQ ON EmrTopicsT.SourceDetailImageStampID = EmrDetailImageStampsQ.ID "
				"WHERE EmrTopicsT.ID = {INT}", 
				eaoEmrItem, eaoEmrDataItem, eaoProcedure, eaoEmrImageHotSpot, eaoSmartStamp, eaoEmrImageHotSpot, eaoEmrTableDropDownItem,
				pLoadInfo->nID);
		} NxCatchAllThrowThread("Error in LoadEMRTopic() - Querying topic data");
	}	

	try {
		if(!rsTopic->eof) {
			if(pLoadInfo->bLoadFromTemplate) {
				if(!pLoadInfo->bIsNewTopic) {
					pLoadInfo->m_nOriginalTemplateTopicID = AdoFldLong(rsTopic, "SourceTemplateTopicID", -1);
					pLoadInfo->m_nOriginalTemplateID = AdoFldLong(rsTopic, "SourceTemplateID", -1); // (a.walling 2007-03-21 09:50) - PLID 25275 - set the source template id
				}
				else {
					pLoadInfo->m_nOriginalTemplateTopicID = pLoadInfo->nID; // this is the topic id
					pLoadInfo->m_nOriginalTemplateID = pLoadInfo->m_nTemplateID; // set the original template to the one that that id is on
				}
			}
			else {
				// (a.walling 2007-03-21 15:54) - PLID 25301
				// this used to just set -1, but that was causing issues.
				pLoadInfo->m_nOriginalTemplateTopicID = AdoFldLong(rsTopic, "SourceTemplateTopicID", -1);
				pLoadInfo->m_nOriginalTemplateID = AdoFldLong(rsTopic, "SourceTemplateID", -1);
			}

			if(pLoadInfo->m_saiOverride.nSourceActionID != -1) {
				pLoadInfo->m_sai = pLoadInfo->m_saiOverride;
				//DRT 9/25/2007 - PLID 27515 - Added SourceAction SourceID and SourceDataGroupID
				pLoadInfo->m_nSourceActionSourceID = pLoadInfo->nOverrideSourceActionSourceID;
				pLoadInfo->m_nSourceActionSourceDataGroupID = pLoadInfo->nOverrideSourceActionSourceDataGroupID;
				pLoadInfo->m_nSourceActionSourceHotSpotGroupID = pLoadInfo->nOverrideSourceActionSourceHotSpotGroupID;
				// (z.manning 2009-02-13 10:26) - PLID 33070
				pLoadInfo->m_nSourceActionSourceTableDropdownGroupID = pLoadInfo->nOverrideSourceActionSourceTableDropdownGroupID;
			}
			else {
				//TES 3/17/2010 - PLID 37530 - Pass in SourceStampID and SourceStampIndex
				pLoadInfo->m_sai = SourceActionInfo((EmrActionObject)AdoFldLong(rsTopic,"SourceType",eaoInvalid), AdoFldLong(rsTopic,"SourceActionID",-1), -1, &TableRow(AdoFldLong(rsTopic,"SourceDataGroupID",-1), AdoFldLong(rsTopic,"SourceDetailImageStampID",-1), AdoFldLong(rsTopic, "SourceStampID",-1), AdoFldLong(rsTopic, "SourceStampIndex",-1)));
				//DRT 9/25/2007 - PLID 27515 - Added SourceAction SourceID and SourceDataGroupID
				pLoadInfo->m_nSourceActionSourceID = AdoFldLong(rsTopic, "SourceActionSourceID", -1);
				pLoadInfo->m_nSourceActionSourceDataGroupID = AdoFldLong(rsTopic, "SourceActionSourceDataGroupID", -1);
				pLoadInfo->m_nSourceActionSourceHotSpotGroupID = AdoFldLong(rsTopic, "EmrSpotGroupID", -1);
				pLoadInfo->m_nSourceActionSourceTableDropdownGroupID = AdoFldLong(rsTopic, "SourceActionSourceTableDropdownGroupID", -1);
			}

			// (j.jones 2007-01-12 09:02) - PLID 24027 - supported SourceDetailID
			// this is different from SourceActionID in that SourceActionID would be the same
			// whether on a template or on a patient EMN, but SourceDetailID would be different
			// on a template than on an EMN
			if(pLoadInfo->bLoadFromTemplate) {
				if(!pLoadInfo->bIsNewTopic && pLoadInfo->bLoadToTemplate) {
					pLoadInfo->m_sai.nSourceDetailID = AdoFldLong(rsTopic, "SourceDetailID", -1);
				}
				else {
					pLoadInfo->m_sai.nSourceDetailID = pLoadInfo->m_saiOverride.nSourceDetailID;
				}
			}
			else {
				if(pLoadInfo->m_saiOverride.nSourceDetailID != -1) {
					pLoadInfo->m_sai.nSourceDetailID = pLoadInfo->m_saiOverride.nSourceDetailID;
				}
				else {
					pLoadInfo->m_sai.nSourceDetailID = AdoFldLong(rsTopic, "SourceDetailID", -1);
				}
			}
			
			pLoadInfo->m_sai.pSourceDetail = pLoadInfo->m_saiOverride.pSourceDetail;

			pLoadInfo->m_strSourceActionName = AdoFldString(rsTopic, "SourceActionName", "");

			pLoadInfo->m_SourceActionDestType = (EmrActionObject)AdoFldLong(rsTopic, "SourceActionDestType", -1);
			
			CString strName = AdoFldString(rsTopic, "Name","");
			BOOL bShowIfEmpty = AdoFldBool(rsTopic, "ShowIfEmpty");
			if(pLoadInfo->bLoadFromTemplate && pLoadInfo->m_nOriginalTemplateTopicID != -1 
				&& pLoadInfo->m_sai.nSourceActionID != -1) {
				//We're loading from a template topic, which was spawned.  The Name and ShowIfEmpty values need to come from
				//the original topic, if possible.
				_variant_t varSourceName = rsTopic->Fields->GetItem("SourceName")->Value;
				if(varSourceName.vt == VT_BSTR) strName = VarString(varSourceName);
				
				_variant_t varSourceShowIfEmpty = rsTopic->Fields->GetItem("SourceShowIfEmpty")->Value;
				if(varSourceName.vt == VT_BOOL) bShowIfEmpty = VarBool(varSourceShowIfEmpty);
			}
			pLoadInfo->m_strName = strName;

			pLoadInfo->m_bShowIfEmpty = bShowIfEmpty;
			
			pLoadInfo->m_nTemplateTopicID = AdoFldLong(rsTopic, "EMRTemplateTopicID",-1);
			
			pLoadInfo->m_nTemplateID = AdoFldLong(rsTopic, "TemplateID", -1);
			
			pLoadInfo->m_nTemplateTopicOrderIndex = AdoFldLong(rsTopic, "TemplateOrderIndex", -1);
			
			if(pLoadInfo->bLoadToTemplate) {
				pLoadInfo->m_bHideOnEMN = AdoFldBool(rsTopic, "HideOnEMN", FALSE);
			}
			else {
				pLoadInfo->m_bHideOnEMN = FALSE;
			}

			// (a.walling 2008-06-30 13:31) - PLID 29271 - Preview Pane flags
			pLoadInfo->m_nPreviewFlags = AdoFldLong(rsTopic, "PreviewFlags", 0);

		}
		rsTopic->Close();
	} NxCatchAllThrowThread("Error in LoadEMRTopic() - Reading queried topic data");

	//now load the subtopics for this topic, which in turn will load its subtopics or EMNDetails
	_RecordsetPtr rsSubTopics;
	if(pLoadInfo->bLoadFromTemplate) {
		try {
			//TES 2/16/2006 - If we're loading onto an EMN, don't load the HideOnEMNs topic.
			CSqlFragment sqlHide = pLoadInfo->bLoadToTemplate ? CSqlFragment("1=1") : CSqlFragment("HideOnEMN = 0");

			//TES 3/23/2006 - OK, the below code was complicated and slightly wrong.  If bLoadHiddenTopics is true, then we want
			//all subtopics, regardless of their SourceActionID.  Otherwise, we only want subtopics with no SourceActionID (they
			//will be added by a ProcessEMRAction call if necessary), unless we ourselves are being spawned by an action, in which
			//case we also want subtopics spawned by the same action.
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			CSqlFragment sqlSpawned;
			if(pLoadInfo->bLoadHiddenTopics) {
				//We want everything.
				sqlSpawned.Create("1=1");
			}
			else {
				//Get the ones that are just NULL, or the same as our action, if we have one.
				if(pLoadInfo->m_saiOverride.nSourceActionID != -1) {
					sqlSpawned.Create("(SourceActionID = {INT} OR SourceActionID Is Null)", pLoadInfo->m_saiOverride.nSourceActionID);
				}
				else {
					sqlSpawned.Create("SourceActionID Is Null");
				}

				// (a.walling 2007-11-27 13:24) - PLID 28194 - We should be filtering on the sourcedetail as well as the sourceaction
				if (sqlSpawned) {
					sqlSpawned += " AND ";
				}

				if (pLoadInfo->m_saiOverride.nSourceDetailID != -1) {
					sqlSpawned += CSqlFragment(" (SourceDetailID = {INT} OR SourceDetailID Is Null) ", pLoadInfo->m_saiOverride.nSourceDetailID);
				} else {
					sqlSpawned += " SourceDetailID Is Null ";
				}
			}
			
			//Use bLoadHiddenTopics
			//CString strSpawned = bLoadHiddenTopics ? "1=1" : "SourceTemplateTopicID Is Null";
			//TES 2/27/2006 - If we're being spawned as the result of an action, don't load subtopics that were the result of a 
			//different action (if that action has already been processed, then the topic was loaded somewhere else anyway, if it
			//hasn't, then the topic should stay hidden).
			//TES 3/17/2006 - However, DO load subtopics that were entered directly by the user (i.e., no SourceActionID).
			//CString strSourceAction = "1=1";
			//if(nSourceActionID != -1) {
			//	strSourceAction.Format("(SourceActionID = %li OR SourceActionID Is Null)", nSourceActionID);
			//}

			//DRT 9/25/2007 - PLID 27515 - Added SourceActionSourceID and SourceActionSourceDataGroupID and necessary joins
			// (z.manning, 01/23/2008) - PLID 28690 - Added SourceActionSourceHotSpotGroupID
			// (z.manning 2009-02-13 10:29) - PLID 33070 - SourceActionSourceTableDropdownGroupID
			// (z.manning 2009-03-05 11:49) - PLID 33338 - SourceDataGroupID
			// (z.manning 2010-02-25 11:13) - PLID 37532 - SourceDetailImageStampID
			//TES 3/17/2010 - PLID 37530 - Added SourceStampID and SourceStampIndex
			//TES 4/15/2010 - PLID 24692 - Added the template ID.
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			rsSubTopics = CreateParamRecordset(pConn, "SELECT EMRTemplateTopicsT.ID, SourceActionID, SourceDetailID, "
				"EmrActionsT.SourceID AS SourceActionSourceID, EMRDataT.EmrDataGroupID AS SourceActionSourceDataGroupID, "
				"EmrSpotGroupID, DropdownGroupID AS SourceActionSourceTableDropdownGroupID, SourceDataGroupID, NULL AS SourceDetailImageStampID, SourceType, "
				"EMRTemplateTopicsT.SourceStampID, EMRTemplateTopicsT.SourceStampIndex, EMRTemplateTopicsT.TemplateID "
				"FROM EMRTemplateTopicsT "
				"LEFT JOIN EmrActionsT ON EMRTemplateTopicsT.SourceActionID = EmrActionsT.ID "
				// (j.jones 2010-09-22 09:04) - PLID 29039 - ensured we force a join only on data item actions
				"LEFT JOIN EMRDataT ON EMRActionsT.SourceID = EMRDataT.ID AND EmrActionsT.SourceType = {CONST} "
				"LEFT JOIN EmrImageHotSpotsT ON EmrActionsT.SourceID = EmrImageHotSpotsT.ID AND EmrActionsT.SourceType = {CONST} "
				"LEFT JOIN EmrTableDropdownInfoT ON EmrActionsT.SourceID = EmrTableDropdownInfoT.ID AND EmrActionsT.SourceType = {CONST} "
				"WHERE EMRParentTemplateTopicID = {INT} AND {SQL} AND {SQL} ORDER BY OrderIndex", 
				eaoEmrDataItem, eaoEmrImageHotSpot, eaoEmrTableDropDownItem, pLoadInfo->nID, sqlSpawned, sqlHide);

			while(!rsSubTopics->eof) {
				//Load this topic.
				long nTopicID = AdoFldLong(rsSubTopics, "ID");
				//TES 4/15/2010 - PLID 24692 - If we're on a template, we may already have an entry for this ID, otherwise, create a new one.
				TopicPositionEntry *tpe = NULL;
				if(pLoadInfo->bLoadToTemplate) {
					long nTemplateID = AdoFldLong(rsSubTopics, "TemplateID");
					//TES 8/24/2010 - PLID 24692 - In cases of bad data, GetEMNByID() may return NULL.
					CEMN *pTemplate = pParentEmr->GetEMNByID(nTemplateID);
					if(pTemplate) {
						tpe = pTemplate->GetTopicPositionEntryByID(nTopicID);
					}
					if(tpe == NULL) {
						tpe = new TopicPositionEntry;
						//TES 5/3/2010 - PLID 24692 - Assign the ID.
						tpe->nTopicID = nTopicID;
					}
				}
				else {
					tpe = new TopicPositionEntry;
				}
				CEMRTopic *pTopic = new CEMRTopic(pLoadInfo->bLoadToTemplate, FALSE, tpe);
				long nSourceActionID = -1;
				long nSourceDetailID = -1;
				long nSourceDataGroupID = -1;
				long nSourceDetailImageStampID = -1;
				EmrActionObject eaoActionType = eaoInvalid;
				//DRT 9/25/2007 - PLID 27515 - Added Source action parameters
				// (z.manning 2009-02-13 10:32) - PLID 33070 - SourceActionSourceTableDropdownGroupID
				//TES 3/17/2010 - PLID 37530 - Added SourceStampID and SourceStampIndex
				long nSourceActionSourceID = -1, nSourceActionSourceDataGroupID = -1, nSourceActionSourceHotSpotGroupID = -1, nSourceActionSourceTableDropdownGroupID = -1,
					nSourceStampID = -1, nSourceStampIndex = -1;
				if(pLoadInfo->bIsNewTopic && pLoadInfo->m_saiOverride.nSourceActionID != -1) {
					nSourceActionID = pLoadInfo->m_saiOverride.nSourceActionID;
					nSourceDataGroupID = pLoadInfo->m_saiOverride.GetDataGroupID();
					nSourceDetailImageStampID = pLoadInfo->m_saiOverride.GetDetailStampID();
					nSourceStampID = pLoadInfo->m_saiOverride.GetStampID();
					nSourceStampIndex = pLoadInfo->m_saiOverride.GetStampIndexInDetailByType();
					eaoActionType = pLoadInfo->m_saiOverride.eaoSourceType;
					nSourceActionSourceID = pLoadInfo->nOverrideSourceActionSourceID;
					nSourceActionSourceDataGroupID = pLoadInfo->nOverrideSourceActionSourceDataGroupID;
					nSourceActionSourceHotSpotGroupID = pLoadInfo->nOverrideSourceActionSourceHotSpotGroupID;
					nSourceActionSourceTableDropdownGroupID = pLoadInfo->nOverrideSourceActionSourceTableDropdownGroupID;
				}
				else {
					nSourceActionID = AdoFldLong(rsSubTopics, "SourceActionID",-1);
					nSourceDataGroupID = AdoFldLong(rsSubTopics, "SourceDataGroupID", -1);
					nSourceDetailImageStampID = AdoFldLong(rsSubTopics, "SourceDetailImageStampID", -1);
					nSourceStampID = AdoFldLong(rsSubTopics, "SourceStampID", -1);
					nSourceStampIndex = AdoFldLong(rsSubTopics, "SourceStampIndex", -1);
					eaoActionType = (EmrActionObject)AdoFldLong(rsSubTopics, "SourceType", eaoInvalid);
					nSourceActionSourceID = AdoFldLong(rsSubTopics, "SourceActionSourceID",-1);
					nSourceActionSourceDataGroupID = AdoFldLong(rsSubTopics, "SourceActionSourceDataGroupID",-1);
					nSourceActionSourceHotSpotGroupID = AdoFldLong(rsSubTopics, "EmrSpotGroupID", -1);
					nSourceActionSourceTableDropdownGroupID = AdoFldLong(rsSubTopics, "SourceActionSourceTableDropdownGroupID", -1);
				}

				// (j.jones 2007-01-22 17:16) - PLID 24366 - use the override for
				// source detail if loading to a patient EMN
				if((pLoadInfo->bIsNewTopic || !pLoadInfo->bLoadToTemplate)
					&& pLoadInfo->m_saiOverride.nSourceDetailID != -1) {
					nSourceDetailID = pLoadInfo->m_saiOverride.nSourceDetailID;
				}
				else {
					nSourceDetailID = AdoFldLong(rsSubTopics, "SourceDetailID",-1);
				}

				// (j.jones 2007-01-22 17:16) - PLID 24366 - use the override for
				// source detail if loading to a patient EMN
				CEMNDetail *pSourceDetail = NULL;
				SourceActionInfo sai;
				if((pLoadInfo->bIsNewTopic || !pLoadInfo->bLoadToTemplate)
					&& pLoadInfo->m_saiOverride.pSourceDetail) {
					pSourceDetail = pLoadInfo->m_saiOverride.pSourceDetail;

					// (j.jones 2007-01-22 17:31) - PLID 24027 - when this happens,
					// ensure our source detail ID is not the template detail ID
					nSourceDetailID = (pLoadInfo->bLoadToTemplate ? pLoadInfo->m_saiOverride.pSourceDetail->m_nEMRTemplateDetailID : pLoadInfo->m_saiOverride.pSourceDetail->m_nEMRDetailID);
					//TES 3/31/2010 - PLID 38002 - Construct the SourceActionInfo
					sai = SourceActionInfo(eaoActionType, nSourceActionID, pSourceDetail, &TableRow(nSourceDataGroupID,nSourceDetailImageStampID, nSourceStampID, nSourceStampIndex));
				}
				else {
					//TES 3/31/2010 - PLID 38002 - We still want to reflect the fact that we have a source detail, so use 
					// the SourceActionInfo constructor that takes an ID instead of a pointer.
					sai = SourceActionInfo(eaoActionType, nSourceActionID, nSourceDetailID, &TableRow(nSourceDataGroupID,nSourceDetailImageStampID, nSourceStampID, nSourceStampIndex));
				}

				//DRT 9/25/2007 - PLID 27515 - Added extra source action parameters
				// (z.manning 2009-02-13 10:33) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
				// (z.manning 2009-03-05 09:14) - PLID 33338 - Use the new source action info class
				//TES 3/17/2010 - PLID 37530 - Pass in SourceStampID and SourceStampIndex
				//TES 3/31/2010 - PLID 38002 - Moved the SourceActionInfo contruction into the branch just above here.
				//SourceActionInfo sai(eaoActionType, nSourceActionID, pSourceDetail, &TableRow(nSourceDataGroupID,nSourceDetailImageStampID, nSourceStampID, nSourceStampIndex));
				pTopic->LoadFromTemplateTopicID(nTopicID, pLoadInfo->bIsNewTopic, pLoadInfo->hWnd, sai, pLoadInfo->bLoadHiddenTopics, pLoadInfo->nPatientID, pConn, pParentEmr, NULL, nSourceActionSourceID, nSourceActionSourceDataGroupID, nSourceActionSourceHotSpotGroupID, nSourceActionSourceTableDropdownGroupID);
				pLoadInfo->m_aryEMRTopics.Add(pTopic);
				rsSubTopics->MoveNext();
			}
			rsSubTopics->Close();
		} NxCatchAllThrowThread("Error in LoadEMRTopic() - Loading template subtopics");
	}
	else {
		try {
			//TES 4/15/2010 - PLID 24692 - Added the EmrID.
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			rsSubTopics = CreateParamRecordset(pConn, "SELECT ID, EmrID FROM EMRTopicsT WHERE Deleted = 0 AND EMRParentTopicID = {INT} ORDER BY OrderIndex", pLoadInfo->nID);
			while(!rsSubTopics->eof) {
				long nTopicID = AdoFldLong(rsSubTopics, "ID");
				long nEmnID = AdoFldLong(rsSubTopics, "EmrID");
				//TES 4/15/2010 - PLID 24692 - The EMN should already have an entry for this item.
				TopicPositionEntry *tpe = pParentEmr->GetEMNByID(nEmnID)->GetTopicPositionEntryByID(nTopicID);
				CEMRTopic *pTopic = new CEMRTopic(FALSE, FALSE, tpe);
				pTopic->LoadFromTopicID(nTopicID, pLoadInfo->hWnd, pConn, pParentEmr, NULL);
				pLoadInfo->m_aryEMRTopics.Add(pTopic);
				rsSubTopics->MoveNext();
			}
			rsSubTopics->Close();
		} NxCatchAllThrowThread("Error in LoadEMRTopic() - Loading subtopics");
	}

	//now load the details for this topic
	//Keep track of what we need to figure our completion status.
	int nEmptyCount = 0;
	int nNonEmptyCount = 0;
	// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
	BOOL bHasUnfilledRequiredDetail = FALSE;
	BOOL bHasDetails = FALSE;

	// (j.jones 2007-07-20 14:56) - PLID 26742 - I went to optimize this and then found these
	// IDs are never even used in this function! If they ever are used, just uncomments this and
	// ensure it pulls from the EMR object instead of data.
	/*
	long nCurrentMedicationsInfoID = -2;
	long nAllergiesInfoID = -2;
	try {

		if(pParentEmr) {
			nCurrentMedicationsInfoID = pParentEmr->GetCurrentMedicationsInfoID();
			nAllergiesInfoID = pParentEmr->GetCurrentAllergiesInfoID();
		}
		else {
			//why is this NULL?
			ASSERT(FALSE);
		}
		
	} NxCatchAllThrowThread("Error in LoadEMRTopic() - Reading system EMR Info ID's");
	*/

	if(pLoadInfo->bLoadFromTemplate) {
		try {
			long nTopicToLoadFrom = pLoadInfo->nID;
			if(pLoadInfo->m_nOriginalTemplateTopicID != -1 && pLoadInfo->m_sai.nSourceActionID!= -1) {
				// (j.jones 2006-02-24 17:36) - PLID 19473 - if a spawned, remembered topic,
				//load the details from the original topic, not anything saved in the remembered topic
				nTopicToLoadFrom = pLoadInfo->m_nOriginalTemplateTopicID;
			}
			//now load the details for this topic
			// (c.haag 2007-02-22 12:41) - PLID 24881 - We now load detail data here in addition to just the ID. This way
			// we won't have to run a query for each individual detail later on.
			//_RecordsetPtr rsDetails = CreateRecordset(pConn, "SELECT EMRTemplateDetailsT.ID "
			//	"FROM EMRTemplateDetailsT INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID "
			//	"LEFT JOIN EMRInfoT ON EmrInfoMasterT.ActiveEMRInfoID = EmrInfoT.ID "
			//	"WHERE EMRTemplateDetailsT.EMRTemplateTopicID = %li %s"
			//	"ORDER BY EMRInfoT.Name", nTopicToLoadFrom, pLoadInfo->bLoadToTemplate?"":"AND EmrTemplateDetailsT.SourceActionID Is Null ");
			// (j.jones 2007-07-18 13:03) - PLID 26730 - load whether or not the info item has Info actions,
			// which it usually does not, such that we don't have to search for them later
			// (j.jones 2007-08-01 14:12) - PLID 26905 - added more data to this query that we can pass into
			// LoadEMRDetailStateDefault() later on
			//DRT 8/2/2007 - PLID 26919 - Added SourceActionSourceID
			//DRT 8/14/2007 - PLID 27067 - Added SourceActionSourceDataGroupID
			// (z.manning, 01/23/2008) - PLID 28690 - Added SourceActionSourceHotSpotGroupID
			//DRT 2/14/2008 - PLID 28698 - Added an optimization to check if any hotspot actions exist.
			//TES 6/5/2008 - PLID 29416 - The system tables are hardcoded to display as "Remember"ing, but the data doesn't
			// have that flag set, so override the data in that case.
			// (c.haag 2008-06-16 12:34) - PLID 30319 - We now calculate the detail names considering text macros
			// (j.jones 2008-09-22 15:07) - PLID 31408 - supported RememberForEMR, which is always disabled when allergy/current meds
			// (z.manning 2009-02-13 10:33) - PLID 33070 - SourceActionSourceTableDropdownGroupID
			// (z.manning 2009-03-11 10:18) - PLID 33338 - Added SourceDataGroupID and source type
			//TES 2/25/2010 - PLID 37535 - Added UseSmartStampsLongForm
			// (z.manning 2010-07-26 14:38) - PLID 39848 - Removed UseSmartStampsLongForm
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			//TES 3/17/2011 - PLID 41108 - Added HasGlassesOrderData and GlassesOrderLens
			// (z.manning 2011-10-06 15:14) - PLID 45842 - Added print data
			// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
			//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
			// (j.jones 2010-06-21 12:22) - PLID 37981 - generic tables never remember their values
			// (c.haag 2008-10-16 12:24) - PLID 31709 - TableRowsAsFields
			// (j.jones 2010-02-11 15:24) - PLID 37318 - load ChildEMRInfoMasterID, SmartStampsEnabled, and ChildEMRTemplateDetailID
			// (z.manning 2011-01-25 15:09) - PLID 42336 - Removed parent detail IDs
			// (j.jones 2007-08-27 10:26) - PLID 27056 - added E/M coding data
			// (j.jones 2011-03-09 09:05) - PLID 42283 - added EMCodeUseTableCategories and Data_EMCodeCategoryID				
			// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields, because they are now only calculated in the API, and not in Practice code
			//"EMRInfoMasterT.EMCodeCategoryID, EMRInfoMasterT.EMCodeUseTableCategories, EMRInfoMasterT.UseEMCoding AS Info_UseEMCoding, EMRInfoMasterT.EMCodingType, "
			// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of this, and the join to emrdatagroupst
			// (a.walling 2008-06-30 13:32) - PLID 29271 - Preview Pane flags
			//TES 3/17/2010 - PLID 37530 - Added SourceStampID and SourceStampIndex
			// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
			// (j.jones 2010-09-22 09:04) - PLID 29039 - ensured we force a join only on data item actions
			// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of the join to emrdatagroupst
			// (j.armen 2014-09-10 11:04) - PLID 63026 - Added InkPenSizePercent and InkPenColor
			_RecordsetPtr rsDetails = CreateParamRecordset(pConn, R"(
SELECT
	EMRTemplateDetailsT.ID AS TemplateDetailID,
	CASE WHEN EmrInfoT.ID = {CONST} THEN EMRTemplateDetailsT.MacroName ELSE EmrInfoT.Name END AS Name,
	EmrInfoT.DataType, EmrInfoT.DataSubType, TemplateID, x, y, Width, Height, EmrInfoT.ID AS EmrInfoID, MergeOverride, SaveTableColumnWidths, 
	EMRTemplateDetailsT.SourceActionID, SourceDetailID, EmrTemplateDetailsT.EmrInfoMasterID,
	(CASE WHEN EmrInfoT.ID IN (SELECT SourceID FROM EMRActionsT WHERE Deleted = 0 AND SourceType = 3) THEN 1 ELSE 0 END) AS HasInfoActions,
	EmrInfoT.BackgroundImageFilePath AS Info_BackgroundImageFilePath, EmrInfoT.BackgroundImageType AS Info_BackgroundImageType, EmrInfoT.DefaultText AS Info_DefaultText,
	CASE WHEN EmrInfoT.DataSubType IN ({CONST}, {CONST}) THEN convert(bit,1) WHEN EmrInfoT.DataSubType = {CONST} THEN Convert(bit,0)
	ELSE EmrInfoT.RememberForPatient END AS Info_RememberForPatient,
	CASE WHEN EmrInfoT.DataSubType IN ({CONST}, {CONST}, {CONST}) THEN convert(bit,0) ELSE EmrInfoT.RememberForEMR END AS Info_RememberForEMR,
	EmrInfoT.SliderMin AS Info_SliderMin, EmrInfoT.SliderMax AS Info_SliderMax, EmrInfoT.SliderInc AS Info_SliderInc, EmrInfoT.TableRowsAsFields,
	EmrInfoT.ChildEMRInfoMasterID, EmrInfoT.SmartStampsEnabled, EMRTemplateDetailsT.ChildEMRTemplateDetailID, NULL AS ChildEMRDetailID,
	CASE WHEN EmrInfoT.ID IN (SELECT EMRInfoDefaultsT.EMRInfoID FROM EMRInfoDefaultsT INNER JOIN EMRDataT ON EMRInfoDefaultsT.EMRDataID = EMRDataT.ID WHERE EMRDataT.Inactive = 0 AND EMRDataT.IsLabel = 0) THEN 1 ELSE 0 END AS Info_HasDefaultValue,
	CASE WHEN EmrTemplateDetailsT.ID IN (SELECT EMRTemplateDetailID FROM EMRTemplateSelectT) THEN 1 ELSE 0 END AS Template_HasDefaultValue,
	EmrTemplateDetailsT.DefaultText, EmrTemplateDetailsT.SliderValue, 1 AS HasSliderValue, EMRActionsT.SourceID AS SourceActionSourceID, EMRDataT.EMRDataGroupID AS SourceActionSourceDataGroupID,
	EmrSpotGroupID,
	CASE WHEN EmrTemplateDetailsT.ID IN (SELECT EMRDetailID FROM EMRHotSpotTemplateSelectT) THEN 1 ELSE 0 END AS HasTemplateHotSpots,
	EMRTemplateDetailsT.PreviewFlags, DropdownGroupID AS SourceActionSourceTableDropdownGroupID,
	EMRTemplateDetailsT.SourceDataGroupID, NULL AS SourceDetailImageStampID, EmrActionsT.SourceType,
	EMRTemplateDetailsT.SourceStampID, EMRTemplateDetailsT.SourceStampIndex,
	EMRTemplateDetailsT.InkPenColor, EMRTemplateDetailsT.InkPenSizePercent,
	EmrInfoT.HasGlassesOrderData, EmrInfoT.GlassesOrderLens, EMRTemplateDetailsT.PrintData, EmrInfoT.InfoFlags, EmrInfoT.HasContactLensData,
	EmrInfoT.UseWithWoundCareCoding
FROM EMRTemplateDetailsT
INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID
LEFT JOIN EMRInfoT ON EmrInfoMasterT.ActiveEMRInfoID = EmrInfoT.ID
LEFT JOIN EMRActionsT ON EMRTemplateDetailsT.SourceActionID = EMRActionsT.ID
LEFT JOIN EMRDataT ON EMRActionsT.SourceID = EMRDataT.ID AND EmrActionsT.SourceType = {CONST}
LEFT JOIN EmrImageHotSpotsT ON EmrActionsT.SourceID = EmrImageHotSpotsT.ID AND EmrActionsT.SourceType = {CONST}
LEFT JOIN EmrTableDropdownInfoT ON EmrActionsT.SourceID = EmrTableDropdownInfoT.ID AND EmrActionsT.SourceType = {CONST}
WHERE EMRTemplateDetailsT.EMRTemplateTopicID = {INT} {CONST_STR}
ORDER BY (CASE WHEN EmrInfoT.ID = {CONST} THEN EMRTemplateDetailsT.MacroName ELSE EmrInfoT.Name END)
)"
				, EMR_BUILT_IN_INFO__TEXT_MACRO, eistCurrentMedicationsTable, eistAllergiesTable, eistGenericTable,
				eistCurrentMedicationsTable, eistAllergiesTable, eistGenericTable,
				eaoEmrDataItem, eaoEmrImageHotSpot, eaoEmrTableDropDownItem,
				nTopicToLoadFrom, pLoadInfo->bLoadToTemplate?"":"AND EmrTemplateDetailsT.SourceActionID IS NULL ",
				EMR_BUILT_IN_INFO__TEXT_MACRO
				);
				
			while(!rsDetails->eof) {
				bHasDetails = TRUE;

				// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
				CEMNDetail *pDetail = CEMNDetail::CreateDetail(NULL, "LoadEMRTopic detail from template recordset", FALSE);
				// (c.haag 2007-02-22 12:43) - PLID 24881 - Pass the recordset into the detail rather than making it pull records itself
				//pDetail->LoadFromTemplateDetailID(AdoFldLong(rsDetails, "ID"), pLoadInfo->bLoadToTemplate, pLoadInfo->bIsNewTopic, NULL, pLoadInfo->nPatientID, pConn);
				// (j.jones 2007-04-12 14:22) - PLID 25604 - bIsInitialLoad is TRUE for this function
				// (j.jones 2007-07-24 17:05) - PLID 26742 - added parameter for pParentEmr
				// (j.jones 2008-09-23 09:42) - PLID 31408 - send the EMRGroupID
				// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
				pDetail->LoadFromTemplateDetailRecordset(rsDetails, FALSE, pLoadInfo->bLoadToTemplate, pLoadInfo->bIsNewTopic, TRUE, pLoadInfo->nPatientID, pLoadInfo->nEMRGroupID, pConn, TRUE, pParentEmr);
				pLoadInfo->m_aryEMNDetails.Add(pDetail);

				if(pDetail->GetVisible()) {
					if(pDetail->IsStateSet(NULL)) {
						nNonEmptyCount++;
					}
					else {
						nEmptyCount++;
						// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
						if (!bHasUnfilledRequiredDetail && pDetail->IsRequired()) {
							bHasUnfilledRequiredDetail = TRUE;
						}
					}
				}

				rsDetails->MoveNext();
			}
			rsDetails->Close();
		} NxCatchAllThrowThread("Error in LoadEMRTopic() - Reading template detail data");
	}
	else {
		try {
			CString strSql;
			// (c.haag 2007-02-22 15:40) - PLID 24889 - We now include all of the detail information
			// in this one query so that we won't have to do it once for each detail later
			//_RecordsetPtr rsDetails = CreateRecordset(pConn, "SELECT EMRDetailsT.ID "
			//	"FROM EMRDetailsT LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EmrInfoT.ID "
			//	"WHERE EMRDetailsT.EMRTopicID = %li AND EMRDetailsT.Deleted = 0 "
			//	"ORDER BY EMRInfoT.Name", pLoadInfo->nID);
			// (j.jones 2007-07-18 13:26) - PLID 26730 - load whether or not the info item has Info actions,
			// which it usually does not, such that we don't have to search for them later
			// (j.jones 2007-08-02 10:25) - PLID 26912 - added extra fields to be passed into LoadEMRDetailState
			//DRT 8/2/2007 - PLID 26919 - Added SourceActionSourceID.
			//DRT 8/14/2007 - PLID 27067 - Added SourceActionSourceDataGroupID
			// (c.haag 2008-06-16 12:35) - PLID 30319 - Factor in built-in text macro names
			// (z.manning 2008-10-08 11:07) - PLID 31613 - EmrDetailsT.LabID
			// (z.manning 2009-02-13 10:37) - PLID 33070 - SourceActionSourceTableDropdownGroupID
			// (z.manning 2009-03-11 10:11) - PLID 33338 - Added SourceDataGroupID and SourceType
			// (z.manning 2010-02-25 11:16) - PLID 37532 - SourceDetailImageStampID
			//TES 3/17/2010 - PLID 37530 - Added SourceStampID and SourceStampIndex
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			//TES 3/17/2011 - PLID 41108 - Added HasGlassesOrderData and GlassesOrderLens
			// (z.manning 2011-10-05 16:59) - PLID 45842 - Added PrintData
			// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
			//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
			_RecordsetPtr rsDetails = CreateParamRecordset(pConn, "SELECT EMRDetailsT.ID AS DetailID, "
				"CASE WHEN EmrInfoT.ID = " + AsString((long)EMR_BUILT_IN_INFO__TEXT_MACRO) + " THEN EMRDetailsT.MacroName ELSE EmrInfoT.Name END AS Name, "
				"EmrInfoT.DataType, EmrInfoT.DataSubType, EMRDetailsT.EmrInfoID, x, y, Width, Height, MergeOverride, SourceTemplateID, SaveTableColumnWidths, SourceActionID, SourceDetailID, EmrInfoT.DataFormat, EmrInfoT.DisableTableBorder, EmrInfoT.EmrInfoMasterID, EMRTemplateDetailToDetailLinkT.EMRTemplateDetailID, "
				"(CASE WHEN EmrInfoT.ID IN (SELECT SourceID FROM EMRActionsT WHERE Deleted = 0 AND SourceType = 3) THEN 1 ELSE 0 END) AS HasInfoActions, "
				"EMRDetailsT.Text, EMRDetailsT.SliderValue, EMRDetailsT.InkData, EMRDetailsT.InkImagePathOverride, EMRDetailsT.InkImageTypeOverride, EMRDetailsT.ImageTextData, EmrDetailsT.PrintData, "
				// (r.gonet 05/31/2011) - PLID 43896 - Put back when we want to save and restore zoom and pan offsets.
				//"EMRDetailsT.ZoomLevel, EMRDetailsT.OffsetX, EMRDetailsT.OffsetY, "
				"CASE WHEN EMRDetailsT.ID IN (SELECT EMRDetailID FROM EmrSelectT) THEN 1 ELSE 0 END AS Detail_HasListSelections, EMRActionsT.SourceID AS SourceActionSourceID, "
				// (z.manning 2010-02-23 14:38) - PLID 37412 - Load whether or not detail has detail image stamps
				"CASE WHEN EmrDetailsT.ID IN (SELECT EmrDetailID FROM EmrDetailImageStampsT WHERE EmrDetailImageStampsT.Deleted = 0) THEN CONVERT(bit, 1) ELSE CONVERT(bit, 0) END AS Detail_HasDetailImageStamps, "
				"EMRDataT.EMRDataGroupID AS SourceActionSourceDataGroupID, "
				// (j.jones 2007-08-27 10:26) - PLID 27056 - added E/M coding data
				// (j.jones 2011-03-09 09:05) - PLID 42283 - added EMCodeUseTableCategories and Data_EMCodeCategoryID
				// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields, because they are now only calculated in the API, and not in Practice code
				//"EMRInfoMasterT.EMCodeCategoryID, EMRInfoMasterT.EMCodeUseTableCategories, EMRInfoMasterT.UseEMCoding AS Info_UseEMCoding, EMRInfoMasterT.EMCodingType, "
				// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of this, and the join to emrdatagroupst
				"EmrSpotGroupID, "
				// (a.walling 2008-06-30 13:32) - PLID 29271 - Preview Pane flags
				"EMRDetailsT.PreviewFlags, EmrDetailsT.LabID, DropdownGroupID AS SourceActionSourceTableDropdownGroupID, "
				"EmrDetailsT.SourceDataGroupID, EmrDetailsT.SourceDetailImageStampID, EmrActionsT.SourceType, "
				"NULL AS SourceStampID, NULL AS SourceStampIndex, "
				"EmrInfoT.HasGlassesOrderData, EmrInfoT.GlassesOrderLens, EmrInfoT.InfoFlags, EmrInfoT.HasContactLensData, "
				// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
				"EmrInfoT.UseWithWoundCareCoding "
				"FROM EMRDetailsT "
				"LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EmrInfoT.ID "
				"LEFT JOIN EMRInfoMasterT ON EMRInfoT.EMRInfoMasterID = EMRInfoMasterT.ID "
				"LEFT JOIN EMRTemplateDetailToDetailLinkT ON EmrDetailsT.ID = EMRTemplateDetailToDetailLinkT.EMRDetailID "
				"LEFT JOIN EMRActionsT ON EMRDetailsT.SourceActionID = EMRActionsT.ID "
				// (j.jones 2010-09-22 09:04) - PLID 29039 - ensured we force a join only on data item actions
				"LEFT JOIN EMRDataT ON EMRActionsT.SourceID = EMRDataT.ID AND EmrActionsT.SourceType = {CONST} "
				// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of the join to emrdatagroupst
				"LEFT JOIN EmrImageHotSpotsT ON EmrActionsT.SourceID = EmrImageHotSpotsT.ID AND EmrActionsT.SourceType = {CONST} "
				"LEFT JOIN EmrTableDropdownInfoT ON EmrActionsT.SourceID = EmrTableDropdownInfoT.ID AND EmrActionsT.SourceType = {CONST} "
				"WHERE EMRDetailsT.EMRTopicID = {INT} AND EMRDetailsT.Deleted = 0 "
				"ORDER BY (CASE WHEN EmrInfoT.ID = " + AsString((long)EMR_BUILT_IN_INFO__TEXT_MACRO) + " THEN EMRDetailsT.MacroName ELSE EmrInfoT.Name END) "
				, eaoEmrDataItem, eaoEmrImageHotSpot, eaoEmrTableDropDownItem, pLoadInfo->nID);

			while(!rsDetails->eof) {
				bHasDetails = TRUE;

				// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
				CEMNDetail *pDetail = CEMNDetail::CreateDetail(NULL, "LoadEMRTopic detail from recordset", FALSE);
				// (c.haag 2007-02-22 15:42) - PLID 24889 - We now use LoadFromDetailRecordset
				//pDetail->LoadFromDetailID(AdoFldLong(rsDetails, "ID"), NULL, pConn);
				pDetail->LoadFromDetailRecordset(rsDetails, FALSE, NULL, pConn);
				pLoadInfo->m_aryEMNDetails.Add(pDetail);
				
				if(pDetail->GetVisible()) {
					if(pDetail->IsStateSet(NULL)) {
						nNonEmptyCount++;
					}
					else {
						nEmptyCount++;
						// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
						if (!bHasUnfilledRequiredDetail && pDetail->IsRequired()) {
							bHasUnfilledRequiredDetail = TRUE;
						}
					}
				}

				rsDetails->MoveNext();
			}
			rsDetails->Close();
		} NxCatchAllThrowThread("Error in LoadEMRTopic() - Reading detail data");
	}

	try {
		//Now, we also know our completion status.
		// (z.manning, 04/07/2008) - PLID 29495 - Added new status for topics without any details
		if(!bHasDetails) {
			pLoadInfo->m_CompletionStatus = etcsBlank;
		}
		else if(nNonEmptyCount && !nEmptyCount) {
			pLoadInfo->m_CompletionStatus = etcsComplete;
		}
		else if (nNonEmptyCount && bHasUnfilledRequiredDetail) {
			// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
			pLoadInfo->m_CompletionStatus = etcsRequired;
		}
		else if(nNonEmptyCount) {
			pLoadInfo->m_CompletionStatus = etcsPartiallyComplete;
		}
		else {
			pLoadInfo->m_CompletionStatus = etcsIncomplete;
		}

		//We're all done!
		pLoadInfo->m_bCompletelyLoaded = TRUE;
		//Let the topic do any post-load processing.
		if(pLoadInfo->hWnd) {
			if(::IsWindow(pLoadInfo->hWnd))
				::PostMessage(pLoadInfo->hWnd, NXM_TOPIC_LOAD_COMPLETE, (WPARAM)pLoadInfo->nID, (LPARAM)pLoadInfo->bLoadFromTemplate);
		}

	} NxCatchAllThrowThread("Error in LoadEMRTopic() - Completion");
}

// (j.jones 2013-05-08 10:28) - PLID 56596 - moved the constructor and destructor to the .cpp,
// and filled the m_aryEMNDetails reference
EMRTopicLoadInfo::EMRTopicLoadInfo() :
	m_aryEMNDetails(*(new CEMNDetailArray())) {

	nID = -1;
	bLoadFromTemplate = FALSE;
	bIsNewTopic = FALSE;
	//DRT 9/25/2007 - PLID 27515 - These match up with the SourceActionID
	nOverrideSourceActionSourceID = -1;
	nOverrideSourceActionSourceDataGroupID = -1;
	nOverrideSourceActionSourceHotSpotGroupID = -1;
	nOverrideSourceActionSourceTableDropdownGroupID = -1; // (z.manning 2009-02-13 09:17) - PLID 33070
	bLoadHiddenTopics = FALSE;
	bLoadSubTopics = TRUE; // (c.haag 2007-08-08 11:16) - PLID 27014 -  Load subtopics by default
	hWnd = NULL;

	m_nTemplateTopicID = -1;
	m_nTemplateID = -1;
	m_nTemplateTopicOrderIndex = -1;
	m_nOriginalTemplateTopicID = -1;
	m_nOriginalTemplateID = -1; // (a.walling 2007-03-21 09:49) - PLID 25301
	//DRT 9/25/2007 - PLID 27515 - These match up with the SourceActionID
	m_nSourceActionSourceID = -1;
	m_nSourceActionSourceDataGroupID = -1;
	m_nSourceActionSourceTableDropdownGroupID = -1; // (z.manning 2009-02-13 09:18) - PLID 33070
	m_nSourceActionSourceHotSpotGroupID = -1;
	m_SourceActionDestType = eaoInvalid;
	m_bShowIfEmpty = FALSE;
	m_bHideOnEMN = FALSE;
	m_CompletionStatus = etcsBlank; // (z.manning, 04/07/2008) - PLID 29495 - Added new status for blank topics

	m_bCompletelyLoaded = FALSE;

	m_bDetailsCopied = FALSE;
	m_bTopicsCopied = FALSE;

	nPatientID = -1;
	nEMRGroupID = -1;

	m_pLoader = NULL;
}

EMRTopicLoadInfo::~EMRTopicLoadInfo() {
	try {
		
		// (j.jones 2013-05-08 10:26) - PLID 56596 - needs to destroy the m_aryEMNDetails reference.
		// It is never null. It has to have been filled in the constructor.
		delete &m_aryEMNDetails;

	}NxCatchAll(__FUNCTION__);
}

CEMR::CLoadTopicsThread::CLoadTopicsThread(CEMR* pEMR)
	: CWinThread(CEMR::CLoadTopicsThread::ThreadProc, NULL)
	, m_wndMessageWindow(m_LoadTopicsInfo)
	, m_eventWindowReady(FALSE, TRUE)
{
	m_pThreadParams = this;
	m_LoadTopicsInfo.pEmr = pEMR;
	m_bAutoDelete = FALSE;
}

CEMR::CLoadTopicsThread::~CLoadTopicsThread()
{
}

LRESULT CEMR::CLoadTopicsThread::PostMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (this == NULL) {
		ThrowNxException("CLoadTopicsThread does not exist");
	}

	// (a.walling 2010-07-23 10:48) - PLID 39834 - Wait for the initialization of the thread's message pump and message-only window
	UINT nRet = WaitForSingleObject(m_eventWindowReady, INFINITE);
	if (WAIT_OBJECT_0 != nRet) {
		ThrowNxException("Error communicating with CLoadTopicsThread: 0x%08x, 0x%08x", nRet, GetLastError());
	}

	HWND hwndMessageWindow = m_wndMessageWindow.GetSafeHwnd();

	if (!::IsWindow(hwndMessageWindow)) {
		return (LRESULT)-1;
	}

	return ::PostMessage(hwndMessageWindow, msg, wParam, lParam);
}

UINT CEMR::CLoadTopicsThread::ThreadProc(LPVOID pParam)
{
	CEMR::CLoadTopicsThread* pThread = (CEMR::CLoadTopicsThread*)pParam;

	UINT result = (UINT)-1;
	try {
		CoInitialize(NULL);
		result = pThread->RunThread();
	} NxCatchAllThread("Error in CEMR::CLoadTopicsThread::ThreadProc!");

	CoUninitialize();
	return result;
}

// (a.walling 2010-07-23 10:48) - PLID 39834 - We use a message-only window now to avoid thread messages being lost within modal loops 
// that we do not control. This way the message will always post to the appropriate window procedure.

CEMR::CLoadTopicsThread::CLoadTopicsMessageWindow::CLoadTopicsMessageWindow(LoadAllTopicsInfo& loadTopicsInfo)
	: m_LoadTopicsInfo(loadTopicsInfo)
	, m_bLoadingTopics(false)
{
}

BEGIN_MESSAGE_MAP(CEMR::CLoadTopicsThread::CLoadTopicsMessageWindow, CWnd)
	ON_MESSAGE(NXM_LOAD_TOPICS, OnLoadTopics)
	ON_MESSAGE(NXM_LOAD_NEXT_TOPIC, OnLoadNextTopic)
	ON_MESSAGE(NXM_LOAD_TOPIC_IMMEDIATE, OnLoadTopicImmediate)
	ON_MESSAGE(NXM_FORCIBLY_CLOSE, OnForciblyClose)
END_MESSAGE_MAP()

LRESULT CEMR::CLoadTopicsThread::CLoadTopicsMessageWindow::OnLoadTopics(WPARAM wParam, LPARAM lParam)
{
	try {
		if(!m_bLoadingTopics) {
			//There are topics available to load, and we're not already loading them.  
			//Post a message to ourselves to load the first one, and remember that we are now actively loading.
			m_bLoadingTopics = true;
			// (a.walling 2010-07-23 11:31) - PLID 39834 - No more thread messages
			//PostThreadMessage(GetCurrentThreadId(), NXM_LOAD_NEXT_TOPIC, NULL, NULL);
			PostMessage(NXM_LOAD_NEXT_TOPIC, NULL, NULL);
		}
	}NxCatchAllThread(__FUNCTION__);

	return 0;
}

LRESULT CEMR::CLoadTopicsThread::CLoadTopicsMessageWindow::OnLoadNextTopic(WPARAM wParam, LPARAM lParam)
{
	try {
		//Find the first topic that hasn't been loaded yet.
		EMRTopicLoadInfo *pTopicInfo = NULL;
		m_LoadTopicsInfo.mtxTopics.Lock();
		for(int i = 0; i < m_LoadTopicsInfo.arTopics.GetSize() && !pTopicInfo; i++) {
			if(!m_LoadTopicsInfo.arTopics[i]->m_bCompletelyLoaded) {
				pTopicInfo = m_LoadTopicsInfo.arTopics[i];
			}
		}
		m_LoadTopicsInfo.mtxTopics.Unlock();

		if(pTopicInfo) {
			//Now load it.
			try {
				LoadEMRTopic(pTopicInfo, m_pConn, m_LoadTopicsInfo.pEmr);
				//Now, tell ourselves to load the next one.
				// (a.walling 2010-07-23 11:31) - PLID 39834 - No more thread messages
				//PostThreadMessage(GetCurrentThreadId(), NXM_LOAD_NEXT_TOPIC, NULL, NULL);
				PostMessage(NXM_LOAD_NEXT_TOPIC, NULL, NULL);
				// (c.haag 2007-07-19 09:27) - PLID 26744 - Silently catch the exception. An
				// error message was already posted to the main thread by this point
			}NxCatchAllCallIgnore({/*Don't try to load this topic again*/pTopicInfo->m_bCompletelyLoaded = TRUE;});
		}
		else {
			//We've loaded all the topics we know of.
			m_bLoadingTopics = false;
		}
	}NxCatchAllThread(__FUNCTION__);

	return 0;
}

LRESULT CEMR::CLoadTopicsThread::CLoadTopicsMessageWindow::OnLoadTopicImmediate(WPARAM wParam, LPARAM lParam)
{
	try {
		//Somebody has an immediate need for a specific topic.
		//long nTopicID = (long)msg.wParam;

		HANDLE hEventToSignal = (HANDLE)lParam;
		/*EMRTopicLoadInfo *pInfo = NULL;
		m_LoadTopicsInfo.mtxTopics.Lock();
		for(int i = 0; i < m_LoadTopicsInfo.arTopics.GetSize() && !pInfo; i++) {
			if(m_LoadTopicsInfo.arTopics[i]->nID == nTopicID) {
				pInfo = m_LoadTopicsInfo.arTopics[i];
			}
		}
		m_LoadTopicsInfo.mtxTopics.Unlock();*/
		
		EMRTopicLoadInfo *pInfo = (EMRTopicLoadInfo*)wParam;			
		pInfo->m_mtxCanDelete.Lock();
		if(!pInfo->m_bCompletelyLoaded) {
			try {
				LoadEMRTopic(pInfo, m_pConn, m_LoadTopicsInfo.pEmr);
				// (c.haag 2007-07-19 09:27) - PLID 26744 - Silently catch the exception. An
				// error message was already posted to the main thread by this point
			}NxCatchAllCallIgnore({/*Don't try to load this topic again*/pInfo->m_bCompletelyLoaded = TRUE;});
		}
		pInfo->m_mtxCanDelete.Unlock();
		//They asked us to signal an event once we're done.
		SetEvent(hEventToSignal);
	}NxCatchAllThread(__FUNCTION__);

	return 0;
}

// (a.walling 2010-07-23 10:48) - PLID 39834 - Requested to close, so call PostQuitMessage to request termination of this thread's message loop.
LRESULT CEMR::CLoadTopicsThread::CLoadTopicsMessageWindow::OnForciblyClose(WPARAM wParam, LPARAM lParam)
{
	try {
		::PostQuitMessage(wParam);
	}NxCatchAllThread(__FUNCTION__);

	return 0;
}

UINT CEMR::CLoadTopicsThread::RunThread()
{	
	// (a.walling 2010-07-23 10:48) - PLID 39834 - Create our message-only window for communication and message processing.
	m_wndMessageWindow.CreateEx(0, ::AfxRegisterWndClass(NULL), "CLoadTopicsMessageWindow", 0, 0, 0, 0, 0, HWND_MESSAGE, 0, NULL);
	
	// (a.walling 2010-07-23 17:23) - PLID 39385 - Dont' call GetRemoteData from a thread!
	m_wndMessageWindow.m_pConn = GetThreadRemoteData(acDoNotAffirm); // (a.walling 2013-03-11 15:10) - PLID 55572 - will be new connection, don't bother affirming

	// (a.walling 2010-07-23 10:48) - PLID 39834 - Notify anyone waiting that our window has been created and our message pump is about to begin.
	m_eventWindowReady.SetEvent();

	MSG msg;
	//
	// Run the main message pump
	//
	BOOL bRet;
	while((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{ 
		if (bRet == -1)
		{
			break;
		}
		else
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}

		// (c.haag 2007-05-21 13:18) - PLID 26084 - If the shutdown event was set, we must abort
		if (WAIT_TIMEOUT != WaitForSingleObject(m_LoadTopicsInfo.pEmr->GetShutdownEvent(), 0)) {
			break;
		}
	}

	m_wndMessageWindow.DestroyWindow();

	/*
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		// (c.haag 2008-10-16 17:11) - PLID 31709 - This can actually become true while debugging. No
		// other message loop checks for this that I can find, and it causes premature termination
		// of this thread and random errors in the main thread if it's doing work. So, I'm taking this out.
		// It would really be nice if this were the reason for the funky issues debugging EMR loads. I have
		// not found exactly what a message value of 0 means, and it's best we not do any handling specific
		// to it unless we know anyway.

		// (a.walling 2010-07-23 11:28) - PLID 39834 - FYI message 0 is WM_NULL, which is sent usually by 
		// SendMessageTimeout or similar function to determine the responsiveness of a thread's message queue

		//if (msg.message == 0)
			//break;
	}
	*/

	// cleanup

	m_LoadTopicsInfo.mtxTopics.Lock();
	for(int i = 0; i < m_LoadTopicsInfo.arTopics.GetSize(); i++) {
		if(!m_LoadTopicsInfo.arTopics[i]->m_bDetailsCopied) {
			//Nobody's taken over the details, we need to clean them up ourselves.
			// (c.haag 2007-05-22 12:10) - PLID 26095 - Now that we use reference
			// counting with details, there's no need to do any deallocation here.
			// Each detail will either be deallocated within a topic's destructor
			// or the preloader's destructor.
			/*for(int nDetail = 0; nDetail < m_LoadTopicsInfo.arTopics[i]->m_aryEMNDetails.GetSize(); nDetail++) {
				delete m_LoadTopicsInfo.arTopics[i]->m_aryEMNDetails[nDetail];
			}*/
			m_LoadTopicsInfo.arTopics[i]->m_aryEMNDetails.RemoveAll();
		}
		if(!m_LoadTopicsInfo.arTopics[i]->m_bTopicsCopied) {
			//Nobody's taken over the topics, we need to clean them up ourselves.
			for(int nDetail = 0; nDetail < m_LoadTopicsInfo.arTopics[i]->m_aryEMRTopics.GetSize(); nDetail++) {
				delete m_LoadTopicsInfo.arTopics[i]->m_aryEMRTopics[nDetail];
			}
			m_LoadTopicsInfo.arTopics[i]->m_aryEMRTopics.RemoveAll();
		}
		delete m_LoadTopicsInfo.arTopics[i];
	}
	m_LoadTopicsInfo.arTopics.RemoveAll();
	m_LoadTopicsInfo.mtxTopics.Unlock();

	return 0;
};

// (a.walling 2010-10-18 17:07) - PLID 40260 - Allow using a default connection
CEMR::CEMR(BOOL bIsTemplate /* = FALSE */, ADODB::_ConnectionPtr pDefaultConnection /* = NULL */)
	: m_pDefaultConnection(pDefaultConnection)
{
	m_nEMRID = -1;
	m_nPicID = -1;
	m_nPatientID = -1;
	m_dtPatientBirthDate.SetStatus(COleDateTime::invalid);
	m_bIsTemplate = bIsTemplate;
	m_pInterface = NULL;
	m_nSpawnLocks = 0;
	m_bIgnoreActions = FALSE;
	m_bOwnChildren = TRUE;
	m_bIsUnsaved = FALSE;
	m_pActionProgressDlg = NULL;
	m_nProgressMax = 0;
	m_nCurrentProgress = 0;
	m_nStatus = 0;
	m_nLastSavedStatus = m_nStatus;
	m_nUnlockSpawnsRefCount = 0;
	m_nProcessEmrActionsDepth = 0;
	m_bIgnoreReadOnlyEMNs = FALSE; // (a.walling 2008-07-07 13:00) - PLID 30513
	m_bLastSavedIgnoreReadOnlyEMNs = FALSE;
	
	// (j.jones 2012-08-22 09:12) - PLID 50486 - m_nDefaultChargeInsuredPartyID must default
	// to Unknown (-3) to indicate we haven't calculated the default value yet
	m_nDefaultChargeInsuredPartyID = (long)DefaultChargeInsuredPartyIDType::Unknown;

	// (j.jones 2012-10-25 17:18) - PLID 53322
	m_bHadSavedAllergies = FALSE;
	// (j.jones 2012-10-29 08:58) - PLID 53324
	m_bHadSavedCurrentMeds = FALSE;

	// (j.jones 2007-02-06 13:46) - PLID 24509 - initialize the start time
	m_bIsTrackingTime = FALSE;
	m_dtStartEditingTime.SetStatus(COleDateTime::invalid);
	m_bChangesMadeThisSession = FALSE;

	// (j.jones 2007-09-17 17:32) - PLID 27396 - initialize the server time offset
	m_dtServerTimeOffset.SetStatus(COleDateTimeSpan::invalid);

	// (j.jones 2007-07-20 14:44) - PLID 26742 - cached these IDs per EMR,
	// default to -2 such that if they become -1, we know we tried to find
	// the IDs, and found none
	m_nCurrentMedicationsInfoID = -2;
	m_nCurrentAllergiesInfoID = -2;
	m_nCurrentGenericTableInfoID = -2;

	// (c.haag 2007-05-21 13:16) - PLID 26084 - Create the event used for tracking when this
	// object is being destroyed
	if (NULL == (m_hevShuttingDown = CreateEvent(NULL, TRUE, FALSE, NULL))) {
		ThrowNxException("Could not create shutdown event for the EMR object!  Last error code: %i", GetLastError());
	}

	//TES 6/5/2006 - Start up our thread.
	//m_pLoadTopicsInfo = new LoadAllTopicsInfo;
	//m_pLoadTopicsInfo->pEmr = this;
	//m_pLoadTopicsThread = AfxBeginThread(LoadAllTopicsMessageThread, m_pLoadTopicsInfo, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	//if(!m_pLoadTopicsThread) {
	//	AfxThrowNxException("Could not create LoadAllTopicsMessageThread!  Last error code: %i", GetLastError());
	//}

	// (a.walling 2010-07-23 11:53) - PLID 39834 - Start up the thread
	m_pLoadTopicsThread = new CLoadTopicsThread(this);
	if(!m_pLoadTopicsThread->CreateThread(CREATE_SUSPENDED)) {
		delete m_pLoadTopicsThread;
		m_pLoadTopicsThread = NULL;
		AfxThrowNxException("Could not create CLoadTopicsThread!  Last error code: %i", GetLastError());
	}

	// (a.walling 2010-07-23 11:56) - PLID 39834 - I think this is an oversight. We probably want to have lower priority
	// than the UI thread, but lowest is way too far for something that is actually handling loading the EMR!
	// so I'm changing to THREAD_PRIORITY_BELOW_NORMAL instead.
	//m_pLoadTopicsThread->SetThreadPriority(THREAD_PRIORITY_LOWEST);
	m_pLoadTopicsThread->SetThreadPriority(THREAD_PRIORITY_BELOW_NORMAL);
	m_pLoadTopicsThread->ResumeThread();
}

CEMR::~CEMR()
{
	// (z.manning 2011-03-04 18:22) - PLID 42682 - Clear any pending actions we may have had
	ClearAllPendingActions();

	// (c.haag 2007-05-21 13:27) - PLID 26084 - We now kill the topics thread first.
	// This ensures that anything we delete further down will not disrupt thread processing.
	if(m_pLoadTopicsThread) {
		// Get the exit code
		DWORD dwExitCode = 0;
		::GetExitCodeThread(m_pLoadTopicsThread->m_hThread, &dwExitCode);
		// See if the thread is still active
		if (dwExitCode == STILL_ACTIVE) {
			// (c.haag 2007-05-21 13:13) - PLID 26084 - Don't let the thread fend for itself.
			// Instead, set an event, animate the thread message pump so that it undoubtedly catches
			// the event, and wait for the thread to finish
			// (a.walling 2010-07-23 12:25) - PLID 39834 - First post to the message window so it will close the thread 
			m_pLoadTopicsThread->PostMessage(NXM_FORCIBLY_CLOSE, 0, 0);
			SetEvent(m_hevShuttingDown);
			m_pLoadTopicsThread->PostThreadMessage(WM_QUIT, 0, 0);
			// (a.walling 2010-07-23 12:00) - PLID 39834 - Is INFINITE a little too much to prevent a memory leak?
			// let's wait 60 seconds and throw an exception so we can be notified.
			if (WAIT_OBJECT_0 == WaitForSingleObject(m_pLoadTopicsThread->m_hThread, 60000)) {
				delete m_pLoadTopicsThread;
			} else {
				// it's not closing. Log/except.
				try {
					ThrowNxException("Thread shutdown timed out");
				} NxCatchAllThread("Error shutting down load topics thread");
			}
			// The thread is still going so post a quit message to it and let it delete itself
			//m_pLoadTopicsThread->m_bAutoDelete = TRUE;
			//PostThreadMessage(m_pLoadTopicsThread->m_nThreadID, WM_QUIT, 0, 0);
		} else {
			// The thread is finished, so just delete it
			delete m_pLoadTopicsThread;
		}
		m_pLoadTopicsThread = NULL;
		//m_pLoadTopicsInfo = NULL;
	}

	// (c.haag 2007-05-21 13:15) - PLID 26084 - Close down the shutting down event
	CloseHandle(m_hevShuttingDown);

	// (a.walling 2009-10-13 12:03) - PLID 36024 - Clear any dangling 'referenced' details
	for(int i = 0; i < m_arypReferencedEMNDetails.GetSize(); i++) {
		m_arypReferencedEMNDetails.GetAt(i)->__Release("~CEMR Dangling reference");
	}

	if(m_bOwnChildren) {
		for(int i = 0; i < m_arypEMNs.GetSize(); i++) {
			delete (CEMN*)m_arypEMNs.GetAt(i);
		}
	}
	for(int i=0; i < m_aryDeletedEMNs.GetSize(); i++) {
		delete m_aryDeletedEMNs[i];
	}

	// (j.jones 2008-07-22 11:41) - PLID 30789 - remove all problems
	// (c.haag 2009-05-19 13:19) - PLID 34277 - Now we remove problem links
	for(i = 0; i < m_apEmrProblemLinks.GetSize(); i++) {
		delete m_apEmrProblemLinks[i];
	}
	m_apEmrProblemLinks.RemoveAll();

	if(m_pActionProgressDlg) {
		delete m_pActionProgressDlg;
		m_pActionProgressDlg = NULL;
	}

	// (c.haag 2009-05-19 09:25) - PLID 34277 - Delete all problems
	for (i=0; i < m_apEmrProblemHeap.GetSize(); i++) {
		// (c.haag 2009-05-22 16:10) - PLID 34277 - Do not just flat out delete the
		// problem. Simply release it. We could be in a precarious case where the EMR
		// problem list has finished loading from memory, and this destructor is called
		// before the last saved details in EMR topics were deallocated. They may hold
		// a reference, still.
		//
		// The rest of the time, however, this will be the final release.
		//
		m_apEmrProblemHeap[i]->Release();
	}

	/*if(m_pLoadTopicsInfo) {
		m_pLoadTopicsInfo->mtxTopics.Lock();
		for(int i = 0; i < m_pLoadTopicsInfo->arTopics.GetSize(); i++) {
			delete m_pLoadTopicsInfo->arTopics[i];
		}
		m_pLoadTopicsInfo->arTopics.RemoveAll();
		m_pLoadTopicsInfo->mtxTopics.Unlock();
		delete m_pLoadTopicsInfo;
		m_pLoadTopicsInfo = NULL;
	}*/
}
	
// (a.walling 2010-10-18 17:07) - PLID 40260 - Allow using a default connection
ADODB::_ConnectionPtr CEMR::GetRemoteData()
{
	if (m_pDefaultConnection) {
		return m_pDefaultConnection;
	} else {
		return ::GetRemoteData();
	}
}

// (c.haag 2009-05-20 10:56) - PLID 34277 - This function must be used when creating a new CEMR problem
// object from within an EMR object (except in special cases where EMNs don't have EMRs, like when loading
// a patient's problem warning from the pt. module)
// (z.manning 2009-05-27 10:11) - PLID 34297 - Added patient ID
// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
// (s.tullis 2015-02-23 15:44) - PLID 64723 
// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added bDoNotShowOnProblemPrompt. True to have the problem show in the prompt when we switch patients
// or go to the EMR tab. False to not show in the prompt.
CEmrProblem* CEMR::AllocateEmrProblem(long nID, long nPatientID, CString strDescription, COleDateTime dtEnteredDate, COleDateTime dtModifiedDate, COleDateTime dtOnsetDate,
		long nStatusID, long nDiagCodeID_ICD9, long nDiagCodeID_ICD10, long nChronicityID, BOOL bIsModified, long nCodeID, BOOL bDoNotShowOnCCDA, BOOL bDoNotShowOnProblemPrompt)
{
	CEmrProblem* p = (nID > 0) ? GetEmrProblem(nID) : NULL;
	if (NULL == p) {
		// (r.gonet 2015-03-09 18:21) - PLID 65008 - Pass DoNotShowOnProblemPrompt.
		p = new CEmrProblem(nID, nPatientID, strDescription, dtEnteredDate, dtModifiedDate, dtOnsetDate, nStatusID, nDiagCodeID_ICD9, nDiagCodeID_ICD10, nChronicityID, FALSE, nCodeID,bDoNotShowOnCCDA,
			bDoNotShowOnProblemPrompt);
		m_apEmrProblemHeap.Add(p);
		// At this point in time, the problem has one reference; and it belongs to
		// CEMR. Even when the caller releases the problem, the CEMR still holds a
		// reference to it. We want this so that the final release can take place in
		// the CEMR destructor.
	}
	// Always add a reference to the problem.
	p->AddRef();
	return p;
}

// (c.haag 2009-07-09 11:08) - PLID 34829 - Overload for getting fields from an existing problem
CEmrProblem* CEMR::AllocateEmrProblem(CEmrProblem* pProblem)
{
	CEmrProblem* p = (pProblem->m_nID > 0) ? GetEmrProblem(pProblem->m_nID) : NULL;
	if (NULL == p) {
		p = new CEmrProblem(pProblem);
		m_apEmrProblemHeap.Add(p);
		// At this point in time, the problem has one reference; and it belongs to
		// CEMR. Even when the caller releases the problem, the CEMR still holds a
		// reference to it. We want this so that the final release can take place in
		// the CEMR destructor.
	}
	// Always add a reference to the problem.
	p->AddRef();
	return p;
}

// (c.haag 2009-05-20 10:56) - PLID 34277 - This function must be used when creating a new CEMR problem
// object from within an EMR object (except in special cases where EMNs don't have EMRs, like when loading
// a patient's problem warning from the pt. module)
CEmrProblem* CEMR::AllocateEmrProblem(ADODB::FieldsPtr& f)
{
	const long nProblemID = AdoFldLong(f, "ID");
	CEmrProblem* p = GetEmrProblem(nProblemID);
	if (NULL == p) {
		p = new CEmrProblem(f);
		m_apEmrProblemHeap.Add(p);
		// At this point in time, the problem has one reference; and it belongs to
		// CEMR. Even when the caller releases the problem, the CEMR still holds a
		// reference to it. We want this so that the final release can take place in
		// the CEMR destructor.
	}
	// Always add a reference to the problem.
	p->AddRef();
	return p;
}

// (c.haag 2009-05-20 10:58) - PLID 34277 - Returns an EMR problem given an ID. This does NOT reference count.
CEmrProblem* CEMR::GetEmrProblem(long nID)
{
	for (int i=0; i < m_apEmrProblemHeap.GetSize(); i++) {
		if (m_apEmrProblemHeap[i]->m_nID == nID) {
			return m_apEmrProblemHeap[i];
		}
	}
	return NULL;
}

// (z.manning 2016-04-12 09:27) - NX-100140 - Gets the IDs for any problems that have been deleted
std::set<long> CEMR::GetDeletedEmrProblemIDs()
{
	CArray<CEmrProblem*, CEmrProblem*> arypDeletedProblems;
	FindAllEmrProblemsToDelete(arypDeletedProblems, TRUE);
	std::set<long> setDeletedProblemIDs;
	foreach (CEmrProblem* pProblem, arypDeletedProblems)
	{
		if (pProblem->m_nID != -1) {
			setDeletedProblemIDs.insert(pProblem->m_nID);
		}
	}

	return setDeletedProblemIDs;
}

// (c.haag 2014-07-17) - PLID 54905 - This fragment of LoadFromID is now contained in a string that can be used in other places if necessary
CString CEMR::AppendLoadSubQuery_rsChargesWhichCodes() const
{
	// (j.jones 2008-06-04 16:33) - PLID 30255 - added QuoteChargeID
	// (j.jones 2011-03-28 14:45) - PLID 42575 - added Billable flag
	// (j.dinatale 2012-01-05 11:36) - PLID 39451 - added insured party id
	//DRT 1/16/2007 - PLID 24177 - We now need to load the diagnosis code linking for the charges above.
	// (j.jones 2009-01-02 09:03) - PLID 32601 - added DiagCodeID
	//TES 2/28/2014 - PLID 61080 - Added ICD-10 fields
	// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
	return R"(  SELECT EMRChargesT.EMRID, EMRChargesT.ID, EMRChargesT.ServiceID, EMRChargesT.Description, EMRChargesT.Category, COALESCE(CptCategoryCountQ.CPTCategoryCount, 0 ) AS CategoryCount,
		  EMRChargesT.CPTModifier1, EMRChargesT.CPTModifier2, EMRChargesT.CPTModifier3, EMRChargesT.CPTModifier4, 
		  EMRChargesT.Quantity, EMRChargesT.UnitCost, EMRChargesT.SourceActionID, EMRChargesT.SourceDetailID, 
		  CPTCodeT.Code, CPTCodeT.SubCode, CPTCodeT.Billable, 
		  QuotedChargesQ.ChargeID AS QuoteChargeID, EMRActionsT.SourceType, EMRChargesT.SourceDataGroupID, EMRChargesT.SourceDetailImageStampID, 
		  EMRChargeRespT.EMRChargeID AS RespChargeID, EMRChargeRespT.InsuredPartyID AS InsuredPartyID
		  FROM EMRChargesT 
		  LEFT JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID 
		  LEFT JOIN EmrActionsT ON EmrChargesT.SourceActionID = EmrActionsT.ID 
		  LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID 
		  LEFT JOIN ( Select ServiceID, COUNT( DISTINCT ServiceMultiCategoryT.CategoryID ) as CPTCategoryCount 
				FROM ServiceMultiCategoryT 
				Group BY ServiceID ) CptCategoryCountQ On  CptCategoryCountQ.ServiceID = EMRChargesT.ServiceID
		  LEFT JOIN (SELECT Min(EMRQuotedChargesT.ChargeID) AS ChargeID, EMRQuotedChargesT.EMRChargeID 
				FROM EMRQuotedChargesT 
				INNER JOIN LineItemT ON EMRQuotedChargesT.ChargeID = LineItemT.ID 
				WHERE LineItemT.Deleted = 0 
				GROUP BY EMRQuotedChargesT.EMRChargeID) AS QuotedChargesQ ON EMRChargesT.ID = QuotedChargesQ.EMRChargeID 
		  WHERE EMRChargesT.Deleted = 0 
		  AND EMRChargesT.EMRID IN {SQL} 
		  ORDER BY EMRChargesT.EMRID;

		  SELECT EMRChargesT.EMRID, EMRChargesT.ID, EMRChargesToDiagCodesT.DiagCodeID, DiagCodes.CodeNumber, 
		  EMRChargesToDiagCodesT.DiagCodeID_ICD10, DiagCodes_10.CodeNumber AS ICD10CodeNumber 
		  FROM EMRChargesToDiagCodesT LEFT JOIN DiagCodes ON EMRChargesToDiagCodesT.DiagCodeID = DiagCodes.ID 
		  LEFT JOIN DiagCodes DiagCodes_10 ON EMRChargesToDiagCodesT.DiagCodeID_ICD10 = DiagCodes_10.ID 
		  LEFT JOIN EMRChargesT ON EMRChargesToDiagCodesT.ChargeID = EMRChargesT.ID 
		  WHERE EMRChargesT.Deleted = 0 AND EMRChargesT.EMRID IN {SQL} ORDER BY EMRChargesT.EMRID, DiagCodes.CodeNumber;
)";
}

//DRT 7/27/2007 - PLID 26836 - Accept a parameter for the EMN that will be displayed when loading.  This allows
//	the preloader to properly load all non-displayed EMNs in the background.  This parameter will be ignored if
//	nID is -1 or if bIsTemplate is TRUE.  If the value is -2, the loading will choose
//	which EMN is to be displayed, and will change the nEMNIDToBeDisplayed variable to be
//	that selection.
// (a.walling 2008-06-09 17:26) - PLID 22049 - Added param to reload a single EMN/Template
void CEMR::LoadFromID(long nID, BOOL bIsTemplate, long &nEMNIDToBeDisplayed, long nEMNIDToReload)
{
	BOOL bReload = nEMNIDToReload != -1;
	m_bIsTemplate = bIsTemplate;
	if(m_bIsTemplate) {
		//Just hold the actions until we're fully loaded.
		LockSpawning();
	}
	else {
		//Ignore actions altogether, they were already processed when the EMN was originally saved.
		m_bIgnoreActions = TRUE;
	}
	
	if(m_bIsTemplate) {
		//if a template, there is no EMR record, the passed in ID is an EMNTemplateID

		m_nEMRID = -1;

		CEMN *pEMN = new CEMN(this);
		SourceActionInfo saiBlank;
		pEMN->LoadFromTemplateID(nID, TRUE, saiBlank);
		// (a.walling 2010-08-17 13:52) - PLID 38006 - Use AddEMN_Internal
		AddEMN_Internal(pEMN);
	}
	else {
		//normal patient EMR

		if (!bReload)
			m_nEMRID = nID;

		// (a.walling 2008-06-09 17:29) - PLID 22049 - Set up the string to grab the data we want
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		// (j.armen 2014-06-02 10:05) - PLID 62042 - sqlLoadIDs - Better query plans can be generated
		//	by inner joining table variables rather than using an IN clause.
		//	Modified the sqlLoadIDs so that it can be used in a FROM or INNER JOIN
		CSqlFragment sqlFindEmnIDs, sqlLoadIDs;
		if(!bReload)
		{
			// (z.manning 2011-05-20 10:20) - PLID 33114 - We need to not load any EMNs where a patient does not
			// have permission to view the linked chart.
			CSqlFragment sqlChartFilter = GetEmrChartPermissionFilter();
			CSqlFragment sqlChartJoin;
			// (z.manning 2011-05-20 10:24) - PLID 33114 - We may not need to filter at all so let's only join the EMN-chart
			// linking table if we actually need it.
			if(!sqlChartFilter.IsEmpty()) {
				sqlChartJoin = CSqlFragment("LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID \r\n");
			}
			sqlFindEmnIDs = CSqlFragment(
				"DECLARE @EmnIDs TABLE (ID int NOT NULL PRIMARY KEY) \r\n"
				"INSERT INTO @EmnIDs (ID) \r\n"
				"SELECT ID FROM EmrMasterT \r\n"
				"{SQL}"
				"WHERE EmrMasterT.Deleted = 0 AND EmrMasterT.EmrGroupID = @nEMRGroupID {SQL} \r\n"
				, sqlChartJoin, sqlChartFilter);
			sqlLoadIDs = CSqlFragment("(SELECT DISTINCT ID FROM @EmnIDs)");
		}
		else {
			sqlLoadIDs = CSqlFragment("(SELECT {INT} AS ID)", nEMNIDToReload);
		}

		// (j.jones 2012-07-31 15:17) - PLID 51750 - the sort is now defined by sqlEMNSortOrder, default is to ORDER BY EmrMasterT.ID
		// (j.jones 2013-07-02 08:53) - PLID 57271 - The sort no longer changes in SQL based on the EMRLoadEMNsSortOrder preference,
		// instead we always sort by EMN ID ascending. Later in this function, AddEMNSorted_Internal will sort the EMNs by that preference.
		// All queries below have been altered to sort by EMN ID, in many cases this reduces the need to join on EMRMasterT.

		// (c.haag 2007-05-01 11:00) - PLID 25853 - We now have two different queries
		// for pulling topics. For patient charts, we pull all topics in the EMN. For
		// templates or creating new charts from templates, we do it the old way.
		//DRT 9/13/2007 - PLID 27384 - See comments on the main query below, I added EMRID and changed the filters.
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		CSqlFragment sqlTopicSql;

		// rsPreloadTopics
		//DRT 9/25/2007 - PLID 27515 - Added SourceActionSourceID, SourceActionSourceDataGroupID
		// (z.manning 2009-02-13 10:38) - PLID 33070 - SourceActionSourceTableDropdownGroupID
		// (z.manning 2009-02-24 12:15) - PLID 33141 - Added source action type
		//TES 2/16/2010 - PLID 37298 - Added SourceActionName info for HotSpots
		// (z.manning 2010-02-25 10:01) - PLID 37532 - SourceDetailImageStampID
		//TES 3/17/2010 - PLID 37530 - Added SourceStampID and SourceStampIndex
		//TES 3/18/2010 - PLID 37530 - Added SourceActionName info for Smart Stamps
		// (a.walling 2010-04-02 16:16) - PLID 38059
		// (j.jones 2012-07-31 15:17) - PLID 51750 - the sort is now defined by sqlEMNSortOrder, default is to ORDER BY EmrMasterT.ID
		// (j.jones 2013-07-02 08:56) - PLID 57271 - the sort is now always by EMR ID, so I removed the EMRMasterT join
		sqlTopicSql.Create(
		"  SELECT EMRTopicsT.EMRID, EmrTopicsT.ID, EmrTopicsT.Name, EmrTopicsT.EmrParentTopicID, EmrTemplateTopicToTopicLinkT.EMRTemplateTopicID, EmrTopicsT.SourceActionID, "
		"  EMRActionsT.SourceID AS SourceActionSourceID, EMRDataQ.EMRDataGroupID AS SourceActionSourceDataGroupID, EmrTopicsT.SourceDetailID,  "
		"  EmrTemplateTopicsT.SourceTemplateTopicID, EmrTopicsT.ShowIfEmpty, EmrTemplateTopicsT.TemplateID, "
		"  EmrTemplateTopicsT.OrderIndex AS TemplateOrderIndex, EMRTopicsT.OrderIndex,  "
		"  CASE WHEN EmrActionsT.SourceType = 3 THEN EmrInfoQ.Name WHEN EmrActionsT.SourceType = 4 THEN EmrDataQ.Data WHEN EmrActionsT.SourceType = 5 THEN EmrProcedureQ.Name WHEN EmrActionsT.SourceType = 10 THEN EmrImageHotSpotsQ.AnatomicLocation "
		// (a.walling 2010-04-02 17:19) - PLID 38059 - Gather the action name for table dropdown spawns
		"   WHEN EmrActionsT.SourceType = 14 THEN EmrDetailImageStampsQ.StampText + ' - ' + convert(nvarchar(50),EmrDetailImageStampsQ.IndexByStamp) WHEN EmrActionsT.SourceType = 13 THEN EmrTableDropdownQ.DropdownData + ' - ' + EmrTableDropdownRowQ.Data + ' - ' + EmrTableDropdownQ.DropdownTableColumn ELSE '' END AS SourceActionName, "
		"  EmrActionsT.DestType AS SourceActionDestType, SourceEMRTemplateTopicsT.TemplateID AS SourceTemplateID, "
		// (a.walling 2008-06-30 13:08) - PLID 29271 - Preview Pane flags
		"  EmrSpotGroupID, EMRTopicsT.PreviewFlags, EmrTableDropdownQ.DropdownGroupID AS SourceActionSourceTableDropdownGroupID, "
		"  SourceType, EmrTopicsT.SourceDataGroupID, EmrTopicsT.SourceDetailImageStampID, "
		"  NULL AS SourceStampID, NULL AS SourceStampIndex "
		"  FROM EMRTopicsT "
		"  LEFT JOIN EmrTemplateTopicToTopicLinkT ON EmrTopicsT.ID = EmrTemplateTopicToTopicLinkT.EmrTopicID "
		"  LEFT JOIN EmrTemplateTopicsT ON EmrTemplateTopicToTopicLinkT.EmrTemplateTopicID = EmrTemplateTopicsT.ID "
		"  LEFT JOIN EMRTemplateTopicsT AS SourceEMRTemplateTopicsT ON EMRTemplateTopicsT.SourceTemplateTopicID = SourceEMRTemplateTopicsT.ID "
		"  LEFT JOIN EmrActionsT ON EmrActionsT.ID = EmrTopicsT.SourceActionID "
		// (a.walling 2010-04-06 08:16) - PLID 38061 - Filtered these on their appropriate source types
		"  LEFT JOIN (SELECT ID, Name FROM EmrInfoT) AS EmrInfoQ ON EmrInfoQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 3  "
		"  LEFT JOIN (SELECT ID, Data, EMRDataGroupID FROM EmrDataT) AS EmrDataQ ON EmrDataQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 4 "
		"  LEFT JOIN (SELECT ID, Name FROM ProcedureT) AS EmrProcedureQ ON EmrProcedureQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = 5 "
		"  LEFT JOIN (SELECT EmrImageHotSpotsT.ID, EmrImageHotSpotsT.EmrSpotGroupID, "
			"EmrHotSpotAnatomicLocationQ.AnatomicLocation "
			"FROM EmrImageHotSpotsT "
			"LEFT JOIN EmrHotSpotAnatomicLocationQ ON EmrImageHotSpotsT.ID = EmrHotSpotAnatomicLocationQ.EmrHotSpotID) AS EmrImageHotSpotsQ ON EmrImageHotSpotsQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = {CONST} "
		// (a.walling 2010-04-02 16:32) - PLID 38059
		"  LEFT JOIN (SELECT EmrTableDropdownInfoT.ID, EmrTableDropdownInfoT.DropdownGroupID, EmrTableDropdownInfoT.Data AS DropdownData, EMRDataTableColumnQ.Data AS DropdownTableColumn "
			"FROM EmrTableDropdownInfoT "
			"INNER JOIN EMRDataT EMRDataTableColumnQ ON EMRTableDropdownInfoT.EMRDataID = EMRDataTableColumnQ.ID "
			"AND EMRDataTableColumnQ.ListType IN (3,4) " // (a.walling 2013-02-28 17:35) - PLID 55391 - This eliminates thousands of seeks!
			") AS EmrTableDropdownQ ON EmrTableDropdownQ.ID = EmrActionsT.SourceID AND EmrActionsT.SourceType = {CONST} "
		// (a.walling 2010-04-02 16:42) - PLID 38059
		"  LEFT JOIN EMRDetailsT SourceEMRDetailQ ON EMRTopicsT.SourceDetailID = SourceEMRDetailQ.ID "
		"  LEFT JOIN EMRDataT EmrTableDropdownRowQ ON EMRTopicsT.SourceDataGroupID = EMRTableDropdownRowQ.EmrDataGroupID AND SourceEMRDetailQ.EMRInfoID = EmrTableDropdownRowQ.EMRInfoID AND EmrActionsT.SourceType = 13 "
		"  LEFT JOIN (SELECT EmrDetailImageStampsT.ID, EmrImageStampsT.StampText, "
			" (SELECT Count(*) FROM EmrDetailImageStampsT OtherStamps WHERE OtherStamps.EmrDetailID = EmrDetailImageStampsT.EmrDetailID "
			" AND OtherStamps.EmrImageStampID = EmrDetailImageStampsT.EmrImageStampID "
			" AND OtherStamps.OrderIndex < EmrDetailImageStampsT.OrderIndex) + 1 AS IndexByStamp "
			" FROM EmrDetailImageStampsT INNER JOIN EmrImageStampsT ON EmrDetailImageStampsT.EmrImageStampID = EmrImageStampsT.ID "
			") EmrDetailImageStampsQ ON EmrTopicsT.SourceDetailImageStampID = EmrDetailImageStampsQ.ID "
		"  WHERE EmrTopicsT.EMRID IN {SQL} AND EmrTopicsT.Deleted = 0"
		"  ORDER BY EmrTopicsT.EMRID, EmrTopicsT.OrderIndex; \r\n"
		, eaoEmrImageHotSpot, eaoEmrTableDropDownItem, sqlLoadIDs);
		
		// (b.cardillo 2006-06-01 15:18) - PLID 20861 - Instead of making tons of roundtrips to 
		// the server, just collect ALL the recordsets here in one shot.  It takes a fraction of 
		// the time to begin with, and it scales better because no matter how many EMNs we have 
		// we still only make ONE roundtrip to the server to get everything we need.
		// (b.cardillo 2006-06-01 16:21) - NOTE: this sql statement is extremely similar to the 
		// corresponding one in CEMN::LoadFromEmnID(), except this is for all EMNs on the EMR 
		// we're trying to load, while that one is just for its one EMN.  But changes made to 
		// the sql statement there should probably be ported to here, and vice versa.
		//DRT 7/27/2007 - PLID 26837 - Parameterized this query.
		//DRT 9/13/2007 - PLID 27384 - Cursors are bad.  The query here was doing... SELECT EMRMasterT.ID WHERE EMRGroupID = <nEMRID>, 
		//	then opening a cursor, looping over all EMNs, and running 10 queries for each one.  If instead we just run 10 queries total,
		//	include the EMRMasterT.ID in that query, then this whole thing runs on average about 400 ms.  Previously it took anywhere from
		//	300 ms (1 EMN) to 7200 ms (first time load, caching the execution plan), to 5200 ms (17 EMNs in an EMR).  So, I've taken this
		//	query, removed the cursor, and it now runs 12 total queries.  (1)  Constant 1 record -- EMR info.  (2)  Constant recordset -- ID of 
		//	all the EMNs in the EMR.  (3) through (12) The queries that pull out the data, with the EMN ID as the first field.  Each query
		//	will contain the data for ALL EMNs in the current EMR.  All ID fields are aliased as EMRID.
		//	
		// (j.armen 2014-07-23 16:03) - PLID 63026 - Declarations
		CSqlFragment sqlDeclare(R"(
DECLARE @EMR_BUILT_IN_INFO__TEXT_MACRO INT SET @EMR_BUILT_IN_INFO__TEXT_MACRO = {CONST}
DECLARE @eistCurrentMedicationsTable INT SET @eistCurrentMedicationsTable = {CONST}
DECLARE @eistAllergiesTable INT SET @eistAllergiesTable = {CONST}
DECLARE @eistGenericTable INT SET @eistGenericTable = {CONST}
DECLARE @eaoEmrDataItem INT SET @eaoEmrDataItem = {CONST}
DECLARE @eaoEmrImageHotSpot INT SET @eaoEmrImageHotSpot = {CONST}
DECLARE @eaoEmrTableDropDownItem INT SET @eaoEmrTableDropDownItem = {CONST}
)",
			EMR_BUILT_IN_INFO__TEXT_MACRO,
			eistCurrentMedicationsTable,
			eistAllergiesTable,
			eistGenericTable,
			eaoEmrDataItem,
			eaoEmrImageHotSpot,
			eaoEmrTableDropDownItem);

		// (z.manning 2009-02-24 12:15) - PLID 33141 - Added source action type and SourceDataGroupID to all applicable objects (i.e. anything that can be spawned)
		// (z.manning 2010-02-25 10:00) - PLID 37532 - SourceDetailImageStampID
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		// (c.haag 2014-03-17) - PLID 60929 - Added userID and diagDisplayPref
		CSqlFragment sqlLoadSql(
			"SET NOCOUNT ON \r\n"
			//DRT 9/17/2007 - PLID 27384 - Makes it easier to change things later.
			"DECLARE @nEMRGroupID INT;\r\nSET @nEMRGroupID = {INT};\r\n"
			"DECLARE @userID INT; SET @userID = {INT}; \r\n"
			"DECLARE @diagDisplayPref INT; SET @diagDisplayPref = {INT}; \r\n"
			"{SQL}" // (z.manning 2011-05-20 10:26) - PLID 33114 - sqlFindEmnIDs
			"SET NOCOUNT ON \r\n"
			// rsEmrInfo
			"SELECT PatientID, Description, Status FROM EmrGroupsT WHERE ID = @nEMRGroupID;\r\n "
			// rsEMNIDs - All the EMN IDs that are in this EMR and not deleted
			// (a.walling 2011-12-19 10:13) - PLID 47089 - This was implicitly sorting on EmrMasterT.ID due to the primary key, but there are
			// situations in which SQL might not necessary return the results in that order; hence, we need to explicitly sort on that field.
			// (j.jones 2012-07-31 15:17) - PLID 51750 - the sort is now defined by sqlEMNSortOrder, default is to ORDER BY EmrMasterT.ID
			// (j.jones 2013-07-02 08:56) - PLID 57271 - the sort is now always by EMR ID, so I removed the EMRMasterT join
			"SELECT ID FROM EmrMasterT WHERE EmrMasterT.Deleted = 0 AND EmrMasterT.EmrGroupID = @nEMRGroupID AND EmrMasterT.ID IN {SQL} ORDER BY EmrMasterT.ID; \r\n"
			//rsProblems
			"{SQL} \r\n "
			   // rsEMN
			   // (z.manning, 04/11/2007) - PLID 25569 - Added category ID and chart ID.
			   // (z.manning, 05/07/2007) - PLID 25731 - Added chart and category names.
			   // (z.manning, 05/07/2007) - PLID 25925 - Added location name.
			   // (j.jones 2007-06-14 11:44) - PLID 26276 - Added Completion Status
			   // (j.jones 2007-08-24 09:09) - PLID 27054 - added Visit Type
			   // (j.jones 2007-09-18 08:49) - PLID 27396 - added TotalTimeInSeconds
			   // (a.walling 2008-05-29 14:38) - PLID 22049 - added Revision
			   // (a.walling 2008-07-01 15:05) - PLID 30586 - Added location logo
			   // (a.walling 2010-10-29 10:33) - PLID 31435 - Added logo width
			   // (d.thompson 2009-05-27) - PLID 29909 - Added ConfidentialInfo
			   // (z.manning 2009-11-19 09:14) - PLID 35810 - 
			   // (z.manning 2010-02-25 11:11) - PLID 37532 - SourceDetailImageStampID
			   // (z.manning 2011-05-18 13:19) - PLID 43756 - Changed LocationID to EMRMasterT.LocationID
			   // (j.jones 2011-07-05 11:46) - PLID 43603 - added StatusName
			   // (a.walling 2012-06-07 08:48) - PLID 50920 - Dates - Modified, Created
			   // (j.jones 2012-07-312013 15:17) - PLID 51750 - the sort is now defined by sqlEMNSortOrder, default is to ORDER BY EmrMasterT.ID
			   // (j.jones 2013-02-27 15:47) - PLID 55351 - Ignore all time slips with a null end time, which means they are uncommitted.
			   // (j.jones 2013-07-02 08:56) - PLID 57271 - the sort is now always by EMR ID, so I removed the EMRMasterT join
			   //TES 1/17/2014 - PLID 60397 - Added EmrTemplateT.HideTitleOnPreview
			   // (b.eyers 2016-02-22) - PLID 68321 - new fields discharge status, admission time, discharge time
			"  SELECT EMRMasterT.ID AS EMRID, EMRMasterT.LocationID, Date, EMRMasterT.InputDate, EMRMasterT.ModifiedDate, PatientAge, PatientGender, "
			"  Status, EmrMasterT.Description, AdditionalNotes, TemplateID, Status, EMRStatusListT.Name AS StatusName, EMRCollectionID, SourceActionID, SourceDetailID, "
			"  PatientFirst, PatientMiddle, PatientLast, EmnTabChartsLinkT.EmnTabChartID, EmnTabCategoriesLinkT.EmnTabCategoryID, "
			"  EmnTabChartsT.Description AS ChartName, EmnTabCategoriesT.Description AS CategoryName, LocationsT.Name AS LocationName, "
			"  LocationsT.LogoImagePath AS LocationLogo, LocationsT.LogoWidth AS LocationLogoWidth, "
			"  EMRMasterT.CompletionStatus, EMRMasterT.VisitTypeID, EMRVisitTypesT.Name AS VisitTypeName, "
			"  (SELECT Coalesce(Sum(DATEDIFF(second, StartTime, EndTime)),0) FROM EMRMasterSlipT WHERE EMRID = EMRMasterT.ID AND EndTime Is Not Null) AS TotalTimeInSeconds, "
			"  EmrMasterT.Revision, SourceType, SourceDataGroupID, ConfidentialInfo, PatientCreatedStatus, SourceDetailImageStampID, "
			"  EMRMasterT.AppointmentID, " // (a.walling 2013-01-16 13:04) - PLID 54650 - Appointment linked with this EMN
			"  EmrTemplateT.HideTitleOnPreview, "
			"  EMRMasterT.DischargeStatusID, DischargeStatusT.Code AS DischargeStatusCode, DischargeStatusT.Description AS DischargeStatusDesc, "
			"  EMRMasterT.AdmissionTime, EMRMasterT.DischargeTime "
			"  FROM EMRMasterT "
			"  LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
			"  LEFT JOIN EmnTabCategoriesLinkT ON EmrMasterT.ID = EmnTabCategoriesLinkT.EmnID "
			"  LEFT JOIN EmnTabChartsT ON EmnTabChartsLinkT.EmnTabChartID = EmnTabChartsT.ID "
			"  LEFT JOIN EmnTabCategoriesT ON EmnTabCategoriesLinkT.EmnTabCategoryID = EmnTabCategoriesT.ID "
			"  LEFT JOIN LocationsT ON EmrMasterT.LocationID = LocationsT.ID "
			"  LEFT JOIN EMRVisitTypesT ON EMRMasterT.VisitTypeID = EMRVisitTypesT.ID "
			"  LEFT JOIN EmrActionsT ON EmrMasterT.SourceActionID = EmrActionsT.ID "
			"  LEFT JOIN EMRStatusListT ON EMRMasterT.Status = EMRStatusListT.ID "
			"  LEFT JOIN EmrTemplateT ON EmrMasterT.TemplateID = EmrTemplateT.ID "
			"  LEFT JOIN DischargeStatusT ON EMRMasterT.DischargeStatusID = DischargeStatusT.ID "
			"  WHERE EmrMasterT.ID IN {SQL} ORDER BY EmrMasterT.ID; \r\n"

			// rsProcs
			// (j.armen 2014-05-30 16:57) - PLID 62042 - INNER JOIN to ProceduresT
			R"(
SELECT EmrProcedureT.EMRID, EmrProcedureT.ProcedureID, EmrProcedureT.SourceActionID,
	EmrProcedureT.SourceDetailID, ProcedureT.Name, EmrActionsT.SourceType,
	EmrProcedureT.SourceDataGroupID, EmrProcedureT.SourceDetailImageStampID
FROM {SQL} IDs
INNER JOIN EmrProcedureT ON EmrProcedureT.EMRID = IDs.ID
INNER JOIN ProcedureT ON EmrProcedureT.ProcedureID = ProcedureT.ID
LEFT JOIN EmrActionsT ON EmrProcedureT.SourceActionID = EmrActionsT.ID
WHERE EmrProcedureT.Deleted = 0
ORDER BY EmrProcedureT.EMRID;

)"
			   // rsDiags
			   //TES 2/28/2014 - PLID 61046 - Added ICD-10 fields
			   // (s.dhole 2014-03-06 12:44) - PLID 60825 Added EmrDiagCodesT.NexGEMMatchType 
			   // (c.haag 2014-03-17) - PLID 60929 - Added QuickListID
			"  SELECT EMRDiagCodesT.ID, EMRDiagCodesT.EMRID, EMRDiagCodesT.DiagCodeID, EMRDiagCodesT.SourceActionID, EMRDiagCodesT.SourceDetailID, "
			"  DiagCodes.CodeNumber, DiagCodes.CodeDesc, EMRDiagCodesT.OrderIndex, EMRActionsT.SourceType, EMRDiagCodesT.SourceDataGroupID, EMRDiagCodesT.SourceDetailImageStampID, "
			"  EmrDiagCodesT.DiagCodeID_ICD10, DiagCodes_10.CodeNumber AS ICD10CodeNumber, DiagCodes_10.CodeDesc AS ICD10CodeDesc, "
			"  EmrDiagCodesT.NexGEMMatchType, GCQI.QuickListID "
			"  FROM EMRDiagCodesT "
			"  CROSS APPLY dbo.GetQuickListIDForCodes(@userID, @diagDisplayPref, EMRDiagCodesT.DiagCodeID, EMRDiagCodesT.DiagCodeID_ICD10) GCQI "
			"  LEFT JOIN DiagCodes ON EMRDiagCodesT.DiagCodeID = DiagCodes.ID "
			"  LEFT JOIN DiagCodes DiagCodes_10 ON EmrDiagCodesT.DiagCodeID_ICD10 = DiagCodes_10.ID "
			"  LEFT JOIN EmrActionsT ON EmrDiagCodesT.SourceActionID = EmrActionsT.ID "
			"  WHERE EMRDiagCodesT.Deleted = 0 AND "
			"  EMRID IN {SQL} ORDER BY EMRDiagCodesT.EMRID; \r\n"
			   // rsCharges
			   // (c.haag 2014-07-17) - PLID 54905- This fragment is now contained in a string that can be used in other places if necessary
			   + AppendLoadSubQuery_rsChargesWhichCodes() +
			   // rsMedications
			"  SELECT EMRMedicationsT.EMRID, PatientMedications.ID, PatientMedications.MedicationID, "
			// (c.haag 2007-02-02 17:58) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
			// (j.jones 2008-05-20 10:03) - PLID 30079 - added additional fields that the prescription editor uses
			//TES 2/10/2009 - PLID 33034 - Renamed Description to PatientExplanation, PillsPerBottle to Quantity
			//TES 2/12/2009 - PLID 33034 - Added Strength and Dosage Form
			//TES 3/31/2009 - PLID 33750 - Strength and Dosage Form now pull from DrugList
			//TES 2/17/2009 - PLID 33140 - Added more SureScripts fields
			// (d.thompson 2009-04-02) - PLID 33571 - Added strength unit
			// (j.jones 2010-05-07 11:02) - PLID 36062 - added EnglishDescription
			// (s.dhole 2013-03-07 15:48) - PLID 55509 load Strength form PatientMedications table
			"  PatientMedications.PatientExplanation, PatientMedications.EnglishDescription, RefillsAllowed, Quantity,"
			" CASE WHEN PatientMedications.QuantityUnitID IS NOT NULL THEN DrugQuantityUnitQ.NAME  ELSE  PatientMedications.Unit END AS Unit, "
			"  EMRMedicationsT.SourceActionID, EMRMedicationsT.SourceDetailID, EMRDataT.Data AS DrugName, "
			"  PatientMedications.ProviderID, PatientMedications.LocationID, PatientMedications.PharmacyID, PatientMedications.PrescriptionDate, " 
			"  PatientMedications.Strength, DrugDosageFormsT.Name AS DosageForm, StrengthUnitT.Name AS StrengthUnit, "
			"  PatientMedications.DaysSupply, PatientMedications.NoteToPharmacist, PatientMedications.AllowSubstitutions, "
			"  PatientMedications.PriorAuthorization, PatientMedications.PriorAuthorizationIsSample, EMRActionsT.SourceType, EMRMedicationsT.SourceDataGroupID, EMRMedicationsT.SourceDetailImageStampID, "
			// (j.jones 2009-04-01 15:59) - PLID 33736 - added NewCropGUID and Discontinued
			// (a.walling 2009-04-22 14:32) - PLID 33948- EPrescribe
			//TES 5/11/2009 - PLID 28519 - Added SampleExpirationDate
			// (a.walling 2009-07-01 13:39) - PLID 34052 - added AgentID, SupervisorID
			//TES 8/3/2009 - PLID 35008 - Added DEASchedule
			// (j.jones 2012-10-29 16:06) - PLID 53259 - added QueueStatus
			// (b.savon 2013-01-16 16:12) - PLID 54656 - Removed AgentID
			"  NewCropGUID, Discontinued, EPrescribe, SampleExpirationDate, SupervisorID, DrugList.DEASchedule, PatientMedications.QueueStatus ,"
			// (s.dhole 2013-03-07 15:48) - PLID 55509 load Strength form PatientMedications table
			" PatientMedications.StrengthUnitID , PatientMedications.DosageFormID ,PatientMedications.QuantityUnitID, "
			// (j.fouts 2013-04-23 14:55) - PLID 55101 - Added Dosage Unit, Quantity, Route, and Frequency
			" PatientMedications.DosageUnitID, PatientMedications.DosageRouteID, PatientMedications.DosageQuantity, PatientMedications.DosageFrequency "
			"  FROM PatientMedications INNER JOIN EmrMedicationsT ON PatientMedications.ID = EmrMedicationsT.MedicationID "
			"  LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
			"  LEFT JOIN DrugDosageFormsT ON PatientMedications.DosageFormID = DrugDosageFormsT.ID "
			// (s.dhole 2013-03-07 15:48) - PLID 55509 Load StrengthUnitID INT, DosageFormID ,QuantityUnitID from PatientMedications
			"  LEFT JOIN DrugStrengthUnitsT AS StrengthUnitT ON PatientMedications.StrengthUnitID = StrengthUnitT.ID "
			"  LEFT JOIN DrugStrengthUnitsT AS DrugQuantityUnitQ ON PatientMedications.QuantityUnitID = DrugQuantityUnitQ.ID "
			"  LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"  LEFT JOIN EmrActionsT ON EmrMedicationsT.SourceActionID = EmrActionsT.ID "
			"  WHERE EMRMedicationsT.Deleted = 0 AND EmrMedicationsT.EmrID IN {SQL} ORDER BY EMRMedicationsT.EMRID; \r\n"
			//TES 3/26/2009 - PLID 33262 - We now need to load the diagnosis code linking for the medications above.
			"  SELECT EMRMedicationsT.EMRID, EMRMedicationsT.MedicationID AS ID, PatientMedicationDiagCodesT.DiagCodeID, DiagCodes.CodeNumber, "
			"  DiagCodes.CodeDesc, PatientMedicationDiagCodesT.SortOrder "
			"  FROM PatientMedicationDiagCodesT LEFT JOIN DiagCodes ON PatientMedicationDiagCodesT.DiagCodeID = DiagCodes.ID "
			"  LEFT JOIN EMRMedicationsT ON PatientMedicationDiagCodesT.PatientMedicationID = EMRMedicationsT.MedicationID "
			"  WHERE EMRMedicationsT.Deleted = 0 AND EMRMedicationsT.EMRID IN {SQL} "
			"  ORDER BY EMRMedicationsT.EMRID, PatientMedicationDiagCodesT.SortOrder; \r\n"
				// rsProviders
			// (j.jones 2011-04-28 14:39) - PLID 43122 - added FloatEMRData
			"  SELECT EmrProvidersT.EMRID, EmrProvidersT.ProviderID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS ProviderName, "
			"  ProvidersT.FloatEMRData "
			"  FROM EmrProvidersT "
			"  INNER JOIN PersonT ON EmrProvidersT.ProviderID = PersonT.ID "
			"  INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
			"  WHERE EmrProvidersT.Deleted = 0 AND EmrProvidersT.EmrID IN {SQL} ORDER BY EmrProvidersT.EMRID; \r\n"
			// rsSecondaryProviders
			// (j.jones 2011-04-28 14:39) - PLID 43122 - added FloatEMRData
			"  SELECT EmrSecondaryProvidersT.EMRID, EmrSecondaryProvidersT.ProviderID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS ProviderName, "
			"  ProvidersT.FloatEMRData "
			"  FROM EmrSecondaryProvidersT "
			"  INNER JOIN PersonT ON EmrSecondaryProvidersT.ProviderID = PersonT.ID "
			"  INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
			"  WHERE EmrSecondaryProvidersT.Deleted = 0 AND EmrSecondaryProvidersT.EmrID IN {SQL} ORDER BY EmrSecondaryProvidersT.EMRID; \r\n"
			// (d.lange 2011-03-23 11:35) - PLID 42136 - rsTechnicianInfo
			"  SELECT EmrTechniciansT.EmrID, EmrTechniciansT.PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS TechnicianName "
			"  FROM EmrTechniciansT INNER JOIN PersonT ON EmrTechniciansT.PersonID = PersonT.ID "
			"  WHERE EmrTechniciansT.Deleted = 0 AND EmrTechniciansT.EmrID IN {SQL} ORDER BY EmrTechniciansT.EmrID; \r\n"
			// (j.gruber 2009-05-07 17:27) - PLID 33688 - other providers
			// (j.jones 2011-04-28 14:39) - PLID 43122 - added FloatEMRData
			"  SELECT EmrOtherProvidersT.EMRID, EmrOtherProvidersT.ProviderID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS ProviderName, "
			"  ProvTypeID, ProviderTypesT.Description, ProvidersT.FloatEMRData "
			"  FROM EmrOtherProvidersT "
			"  INNER JOIN PersonT ON EmrOtherProvidersT.ProviderID = PersonT.ID "
			"  INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
			"  LEFT JOIN ProviderTypesT ON EmrOtherProvidersT.ProvTypeID = ProviderTypesT.ID "
			"  WHERE EmrOtherProvidersT.Deleted = 0 AND EmrOtherProvidersT.EmrID IN {SQL} ORDER BY EmrOtherProvidersT.EMRID; \r\n"
			// (z.manning 2008-10-06 15:40) - PLID 21094 - rsLabInfo
			// (z.manning 2008-10-09 12:54) - PLID 31628 - Added LabProcedureID
			//TES 11/10/2009 - PLID 36260 - Replaced AnatomySide with AnatomyQualifierID
			//TES 12/8/2009 - PLID 36512 - Restored AnatomySide
			"  SELECT EmrDetailsT.EmrID, LabsT.ID AS LabID, LabsT.SourceActionID, LabsT.SourceDetailID \r\n"
			"	, LabProcedureID, LabsT.AnatomySide, AnatomyQualifiersT.Name AS LocationQualifier, ClinicalData, LabAnatomyT.Description AS AnatomicLocation \r\n"
			"	, Type, ToBeOrdered, SourceType, LabsT.SourceDataGroupID, LabsT.SourceDetailImageStampID \r\n"
			"  FROM LabsT \r\n"
			"  INNER JOIN EmrDetailsT ON LabsT.SourceDetailID = EmrDetailsT.ID \r\n"
			"  LEFT JOIN LabAnatomyT ON LabAnatomyT.ID = LabsT.AnatomyID \r\n"
			"  LEFT JOIN AnatomyQualifiersT ON AnatomyQualifiersT.ID = LabsT.AnatomyQualifierID \r\n"
			"  LEFT JOIN EmrActionsT ON LabsT.SourceActionID = EmrActionsT.ID \r\n"
			"  WHERE EmrDetailsT.EmrID IN {SQL} AND LabsT.Deleted = 0 \r\n"
			"  ORDER BY EmrDetailsT.EmrID; \r\n"
			//TES 4/15/2010 - PLID 24692 - rsTopicPositions - Load the topic positions separately from the topics list; 
			// it is the EMN's job to maintain that list, not the EMRTopic's job.
			"SELECT EmrTopicsT.ID, EmrTopicsT.EMRParentTopicID, EmrTopicsT.EMRID FROM EmrTopicsT "
			"WHERE EmrTopicsT.EmrID IN {SQL} AND EmrTopicsT.Deleted = 0 "
			"ORDER BY EmrTopicsT.EMRID, EmrTopicsT.EMRParentTopicID ASC, EmrTopicsT.OrderIndex ASC \r\n"
			// (c.haag 2007-02-26 17:54) - PLID 24949 - rsPreloadDetails
			// (c.haag 2007-04-27 08:34) - PLID 25774 - Pulled additional fields necessary for quickly processing EMR actions
			// (c.haag 2008-06-16 12:32) - PLID 30319 - Use new name calculation for text macros
			// (z.manning 2008-10-08 11:08) - PLID 31613 - EmrDetailsT.LabID
			// (c.haag 2008-10-16 11:42) - PLID 31709 - TableRowsAsFields
			//TES 2/21/2010 - PLID 37463 - SmartStampsLongForm and UseSmartStampsLongForm
			//TES 3/17/2011 - PLID 41108 - Added HasGlassesOrderData and GlassesOrderLens
			// (z.manning 2011-10-05 16:59) - PLID 45842 - Added PrintData
			// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
			//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
			// (j.jones 2010-02-11 15:24) - PLID 37318 - load ChildEMRInfoMasterID, SmartStampsEnabled, and ChildEMRDetailID
			// (z.manning 2011-01-25 15:09) - PLID 42336 - Removed parent detail IDs
			// (a.walling 2013-03-18 09:30) - PLID 55724 - Load AutoAlphabetizeListData for EMRInfoT records in the CEMNLoader
			// (a.walling 2013-03-27 10:05) - PLID 55900 - CEMNLoader needs to load the revision of EMRInfoT records
			//TES 6/5/2008 - PLID 29416 - The system tables are hardcoded to display as "Remember"ing, but the data doesn't
			// have that flag set, so override the data in that case.
			// (j.jones 2008-09-22 15:07) - PLID 31408 - supported RememberForEMR, which is always disabled when allergy/current meds
			// (j.jones 2010-06-21 12:22) - PLID 37981 - generic tables never remember their values
			// (j.jones 2007-07-18 13:26) - PLID 26730 - load whether or not the info item has Info actions,
			// which it usually does not, such that we don't have to search for them later
			// (a.walling 2008-06-30 13:05) - PLID 29271 - Preview Pane flags
			// (c.haag 2007-04-24 12:48) - PLID 25768 - Additional fields for the preload
			// (r.gonet 05/31/2011) - PLID 43896 - Put back when we want to save and restore zoom and pan offsets.
			//"  ZoomLevel, OffsetX, OffsetY, "
			// (j.jones 2007-08-02 10:50) - PLID 26912 - loaded whether or not the detail has list items selected
			// (z.manning 2010-02-23 14:38) - PLID 37412 - Load whether or not detail has detail image stamps
			// (c.haag 2007-05-02 08:23) - PLID 25870 - Load reconstructed detail data
			//DRT 8/2/2007 - PLID 26919 - Added SourceActionSourceID
			//DRT 8/14/2007 - PLID 27067 - Added SourceActionSourceDataGroupID
			// (z.manning, 01/23/2008) - PLID Added hot spot group ID
			// (z.manning 2009-02-13 10:40) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
			// (j.jones 2007-08-27 10:26) - PLID 27056 - added E/M coding data
			// (j.jones 2011-03-09 09:05) - PLID 42283 - added EMCodeUseTableCategories and Data_EMCodeCategoryID
			// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields, because they are now only calculated in the API, and not in Practice code
			//"  EMRInfoMasterT.EMCodeCategoryID, EMRInfoMasterT.EMCodeUseTableCategories, EMRInfoMasterT.UseEMCoding AS Info_UseEMCoding, EMRInfoMasterT.EMCodingType, "
			// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of this, and the join to emrdatagroupst
			// (a.walling 2008-06-30 13:05) - PLID 29271 - Preview Pane flags
			//TES 2/25/2010 - PLID 37535 - UseSmartStampsLongForm
			//TES 3/17/2010 - PLID 37530 - Added SourceStampID and SourceStampIndex
			// (z.manning 2010-07-26 14:39) - PLID 39848 - Removed UseSmartStampsLongForm
			// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
			// (j.jones 2010-09-22 09:04) - PLID 29039 - ensured we force a join only on data item actions
			// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of the join to emrdatagroupst
			// (c.haag 2007-06-27 12:34) - PLID 26473 - Don't load details that belong to deleted topics
			// (j.armen 2014-07-23 16:03) - PLID 63026 - Cleaned up query for readability
			// (j.armen 2014-07-23 16:03) - PLID 63026 - Added Ink Pen Color and Ink Pen Size
			R"(
SELECT
	-- EMRDetailsT
	D.EMRID, D.ID AS DetailID, D.EMRTopicID, D.EmrInfoID, D.x, D.y, D.Width, D.Height, D.MergeOverride,
	D.ChildEMRDetailID, NULL AS ChildEMRTemplateDetailID, D.SourceTemplateID, D.SaveTableColumnWidths,
	D.SourceActionID, D.SourceDetailID, D.Text, D.InkData, D.InkImagePathOverride, D.InkImageTypeOverride,
	D.ImageTextData, D.PrintData, D.SliderValue, D.PreviewFlags, D.LabID, D.InkPenColor, D.InkPenSizePercent,
	D.SourceDataGroupID, D.SourceDetailImageStampID, NULL AS SourceStampID, NULL AS SourceStampIndex,

	CASE WHEN D.ID IN (SELECT EMRDetailID FROM EmrSelectT) THEN 1 ELSE 0 END AS Detail_HasListSelections,

	CASE
		WHEN D.ID IN (
			SELECT EmrDetailID
			FROM EmrDetailImageStampsT
			WHERE Deleted = 0
		) THEN CONVERT(BIT, 1)
		ELSE CONVERT(BIT, 0)
	END AS Detail_HasDetailImageStamps,

	-- EMRInfoT
	I.DataType, I.DataSubType, I.TableRowsAsFields, I.ChildEMRInfoMasterID, I.SmartStampsEnabled,
	I.AutoAlphabetizeListData AS Info_AutoAlphabetizeListData, I.Revision AS Info_Revision, 
	I.BackgroundImageFilePath AS Info_BackgroundImageFilePath, I.BackgroundImageType AS Info_BackgroundImageType,
	I.SliderMin AS Info_SliderMin, I.SliderMax AS Info_SliderMax, I.SliderInc AS Info_SliderInc,
	I.LongForm AS Info_LongForm, I.DataSeparator AS Info_DataSeparator, I.DataSeparatorFinal AS Info_DataSeparatorFinal,
	I.DefaultText AS Info_DefaultText, I.DisableTableBorder AS Info_DisableTableBorder, I.DataFormat,
	I.DisableTableBorder, I.EmrInfoMasterID, I.PreviewFlags AS Info_PreviewFlags,
	I.HasGlassesOrderData, I.GlassesOrderLens, I.InfoFlags, I.HasContactLensData, I.UseWithWoundCareCoding,

	CASE WHEN I.ID = @EMR_BUILT_IN_INFO__TEXT_MACRO THEN D.MacroName ELSE I.Name END AS Name,

	CASE
		WHEN I.DataSubType IN (@eistCurrentMedicationsTable, @eistAllergiesTable) THEN CONVERT(BIT,1)
		WHEN I.DataSubType = @eistGenericTable THEN CONVERT(BIT,0)
		ELSE I.RememberForPatient
	END AS Info_RememberForPatient,

	CASE
		WHEN I.DataSubType IN (@eistCurrentMedicationsTable, @eistAllergiesTable, @eistGenericTable) THEN CONVERT(BIT,0)
		ELSE I.RememberForEMR
	END AS Info_RememberForEMR,

	CASE
		WHEN I.ID IN (
			SELECT SourceID
			FROM EMRActionsT
			WHERE Deleted = 0 AND SourceType = 3
		) THEN 1
		ELSE 0
	END AS HasInfoActions,

	-- EMRTemplateDetailToDetailLinkT
	TD2DL.EMRTemplateDetailID,

	-- ReconstructedEMRDetailsT
	RD.ReviewStatus,

	-- EMRActionsT
	A.SourceID AS SourceActionSourceID, A.SourceType,

	-- EMRDataT
	Data.EMRDataGroupID AS SourceActionSourceDataGroupID,

	-- EmrImageHotSpotsT
	HS.EmrSpotGroupID,

	-- EmrTableDropdownInfoT
	DI.DropdownGroupID AS SourceActionSourceTableDropdownGroupID

FROM EMRDetailsT D
INNER JOIN EMRTopicsT ON EMRTopicsT.ID = D.EMRTopicID
LEFT JOIN ReconstructedEMRDetailsT RD ON RD.EMRDetailID = D.ID
LEFT JOIN EMRInfoT I ON D.EMRInfoID = I.ID
LEFT JOIN EMRInfoMasterT IM ON I.EMRInfoMasterID = IM.ID
LEFT JOIN EMRTemplateDetailToDetailLinkT TD2DL ON D.ID = TD2DL.EMRDetailID
LEFT JOIN EMRActionsT A ON D.SourceActionID = A.ID
LEFT JOIN EMRDataT Data ON A.SourceID = Data.ID AND A.SourceType = @eaoEmrDataItem
LEFT JOIN EmrImageHotSpotsT HS ON A.SourceID = HS.ID AND A.SourceType = @eaoEmrImageHotSpot
LEFT JOIN EmrTableDropdownInfoT DI ON A.SourceID = DI.ID AND A.SourceType = @eaoEmrTableDropDownItem
WHERE D.EMRID IN {SQL}
	AND D.Deleted = 0
	AND EMRTopicsT.Deleted = 0
ORDER BY D.EMRID,
	CASE WHEN I.ID = @EMR_BUILT_IN_INFO__TEXT_MACRO THEN D.MacroName ELSE I.Name END

)"
			// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic
			// (z.manning 2011-04-06 15:16) - PLID 43140
			// prsDetailListOrderInfo
			"{SQL} "
			"\r\n"
			// (c.haag 2007-04-30 13:28) - PLID 25853 - We now have a variable SQL string for topics
			"{SQL}\r\n "
			// (z.manning 2010-02-17 15:33) - PLID 37412 - Added query to load detail image stamp data
			// (z.manning 2011-01-27 16:27) - PLID 42335 - Added UsedInTableData
			// (z.manning 2011-09-08 10:21) - PLID 45335 - Added 3D fields
			// rsDetailImageStamps
			"SELECT EmrDetailImageStampsT.ID, EmrDetailImageStampsT.EmrDetailID, EmrDetailImageStampsT.EmrImageStampID, \r\n"
			"	EmrDetailImageStampsT.OrderIndex, EmrDetailImageStampsT.SmartStampTableSpawnRule, "
			"	EmrDetailImageStampsT.XPos, EmrDetailImageStampsT.YPos, EmrDetailsT.EmrID \r\n"
			"	, CONVERT(bit, CASE WHEN EmrDetailTableDataQ.EmrDetailImageStampID IS NULL THEN 0 ELSE 1 END) AS UsedInTableData \r\n"
			"	, XPos3D, YPos3D, ZPos3D, XNormal, YNormal, ZNormal, HotSpot3D \r\n"
			"FROM EmrDetailImageStampsT \r\n"
			"INNER JOIN EmrDetailsT ON EmrDetailImageStampsT.EmrDetailID = EmrDetailsT.ID \r\n"
			"INNER JOIN EmrTopicsT ON EmrDetailsT.EmrTopicID = EmrTopicsT.ID \r\n"
			"LEFT JOIN (SELECT DISTINCT EmrDetailTableDataT.EmrDetailImageStampID FROM EmrDetailTableDataT WHERE EmrDetailTableDataT.EmrDetailImageStampID IS NOT NULL) EmrDetailTableDataQ ON EmrDetailImageStampsT.ID = EmrDetailTableDataQ.EmrDetailImageStampID \r\n"
			"WHERE EMRDetailsT.EMRID IN ({SQL}) AND EMRDetailsT.Deleted = 0 AND EMRTopicsT.Deleted = 0 AND EmrDetailImageStampsT.Deleted = 0 \r\n"
			"ORDER BY EMRDetailsT.EMRID, EmrDetailImageStampsT.OrderIndex \r\n"
			// (c.haag 2011-03-18) - PLID 42895 - Loading Common Lists for EMR items
			// (c.haag 2011-04-11) - PLID 43155 - Added GroupOnPreviewPane
			"SELECT EmrInfoCommonListT.ID, EmrInfoCommonListT.EmrInfoID, EmrInfoCommonListT.Name, \r\n"
			"EmrInfoCommonListT.Color, EmrInfoCommonListT.OrderID, EmrInfoCommonListT.Inactive,  \r\n"
			"EmrInfoCommonListT.GroupOnPreviewPane \r\n"
			"FROM EmrInfoCommonListT \r\n"
			"WHERE EmrInfoCommonListT.Inactive = 0 \r\n"
			"AND EmrInfoCommonListT.EmrInfoID IN \r\n"
			"(SELECT EmrInfoID FROM EMRDetailsT \r\n"
			"WHERE EMRDetailsT.EMRID IN {SQL} AND EMRDetailsT.Deleted = 0 "
			"AND EmrDetailsT.EmrTopicID IN (SELECT ID FROM EMRTopicsT WHERE Deleted = 0)) \r\n "
			"ORDER BY EmrInfoCommonListT.OrderID "
			// (c.haag 2011-03-18) - PLID 42895 - Loading Common List items for EMR items
			"SELECT EmrInfoCommonListItemsT.ID, EmrInfoCommonListItemsT.ListID, EmrInfoCommonListItemsT.EmrDataID, "
			"EmrInfoCommonListT.EmrInfoID, EmrDataT.Data \r\n"
			"FROM EmrInfoCommonListT "
			"INNER JOIN EmrInfoCommonListItemsT ON EmrInfoCommonListItemsT.ListID = EmrInfoCommonListT.ID \r\n"
			"INNER JOIN EmrDataT ON EmrDataT.ID = EmrInfoCommonListItemsT.EmrDataID \r\n"
			"WHERE EmrInfoCommonListT.Inactive = 0 AND EmrDataT.Inactive = 0 \r\n"
			"AND EmrInfoCommonListT.EmrInfoID IN \r\n"
			"(SELECT EmrInfoID FROM EMRDetailsT \r\n"
			"WHERE EMRDetailsT.EMRID IN {SQL} AND EMRDetailsT.Deleted = 0  "
			"AND EmrDetailsT.EmrTopicID IN (SELECT ID FROM EMRTopicsT WHERE Deleted = 0)) \r\n "
			"\r\n"
			// (z.manning 2011-07-07 12:34) - PLID 44469 - Load EMN coding group data
			"SELECT EmnCodingGroupLinkT.EmnID, EmnCodingGroupLinkT.EmrCodingGroupID, EmnCodingGroupLinkT.GroupQuantity \r\n"
			"FROM EmnCodingGroupLinkT \r\n"
			"WHERE EmnCodingGroupLinkT.EmnID IN {SQL} \r\n"
			"ORDER BY EmnCodingGroupLinkT.EmnID \r\n"
			"\r\n",
			m_nEMRID,
			GetCurrentUserID(),
			DiagQuickListUtils::GetAPIDiagDisplayTypeInt(),
			sqlFindEmnIDs,
			// rsEMNIDs
			sqlLoadIDs,
			// (c.haag 2008-07-18 15:35) - PLID 30784 - Include EMR problems
			GetEMNProblemSql(sqlLoadIDs),
			// rsEMN
			sqlLoadIDs,
			// rsProcs
			sqlLoadIDs,
			// rsDiags
			sqlLoadIDs,
			// rsCharges
			sqlLoadIDs,
			//charge/diag linking
			sqlLoadIDs,
			// rsMedications
			sqlLoadIDs,
			// rsMedication / diag linking
			sqlLoadIDs,
			// rsProviders
			sqlLoadIDs,
			// rsSecondaryProviders
			sqlLoadIDs,
			// rsTechnicianInfo
			sqlLoadIDs,
			//other providers
			sqlLoadIDs,
			// rsLabInfo
			sqlLoadIDs,
			// rsTopicPositions
			sqlLoadIDs, 
			// rsPreloadDetails
			sqlLoadIDs,
			// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic
			// prsDetailListOrderInfo
			GetEmrDetailListOrderSql(sqlLoadIDs),
			// variable SQL string for topics
			sqlTopicSql,
			//rsDetailImageStamps
			sqlLoadIDs,
			//Common Lists & Items
			sqlLoadIDs, sqlLoadIDs, // (c.haag 2011-03-18) - PLID 42895
			/* EmnCodingGroupLinkT.EmnID */sqlLoadIDs
			);

			_RecordsetPtr prsNextRecordset = CreateParamRecordset("{SQL}{SQL}", sqlDeclare, sqlLoadSql);


		//Load info about the EMR.
		_RecordsetPtr rsEmrInfo = prsNextRecordset;
		if(rsEmrInfo->eof) {
			AfxThrowNxException("CEMR::LoadFromID called with invalid ID!");
		}
		if (!bReload) {
			m_nPatientID = AdoFldLong(rsEmrInfo, "PatientID");
			m_strDescription = AdoFldString(rsEmrInfo, "Description");
			m_nStatus = AdoFldLong(rsEmrInfo, "Status");

			//store the "last saved" values
			m_nLastSavedStatus = m_nStatus;
			m_strLastSavedDescription = m_strDescription;
		}

		//DRT 9/13/2007 - PLID 27384 - The 2nd query is now a list of all the EMN IDs in the EMR.
		prsNextRecordset = prsNextRecordset->NextRecordset(NULL);
		_RecordsetPtr prsEMNIDs = prsNextRecordset;


		//DRT 9/13/2007 - PLID 27384 - We will now pass all our recordsets into a structure.  We get these all once
		//	ahead of time.  We will then pass this structure into CEMN::LoadFromEmnRecordsets.  Each recordset is
		//	sorted by EMN ID, so with 1 traversal of the recordset, we will eventually get through all the records.
		LoadEMNRecordsets_Objects lero;
		lero.prsProblemInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		lero.prsEMNInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		lero.prsProcInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		lero.prsDiagInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		lero.prsChargeInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		lero.prsWhichCodesInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		lero.prsMedicationInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		//TES 3/26/2009 - PLID 33262 - Medications also have diagnosis codes now.
		lero.prsMedDiagCodesInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		lero.prsProviderInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		lero.prsSecProviderInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		// (d.lange 2011-03-23 11:27) - PLID 42136 - added for Assistant/Technician
		lero.prsTechnicianInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		// (j.gruber 2009-05-07 17:22) - PLID 33688 - other providers
		lero.prsOtherProviderInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		// (z.manning 2008-10-06 15:26) - PLID 21094 - Labs
		lero.prsLabInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		//TES 4/15/2010 - PLID 24692 - rsTopicPositions
		lero.prsTopicPositionInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		lero.prsDetailInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic
		// (z.manning 2011-04-06 15:12) - PLID 43140 - Detail list sort order
		lero.prsDetailListOrderInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		lero.prsTopicInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		// (z.manning 2010-02-17 16:17) - PLID 37412 - detail image stamps
		lero.prsDetailImageStampInfo = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		// (c.haag 2011-03-18) - PLID 42895 - Loading Common Lists for EMR items
		lero.prsCommonLists = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		lero.prsCommonListItems = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));
		// (z.manning 2011-07-07 12:43) - PLID 44469
		lero.prsEmrCodingGroupData = (prsNextRecordset = prsNextRecordset->NextRecordset(NULL));

		//Load all our EMNs.
		// (b.cardillo 2006-06-01 15:13) - PLID 20861 - We already have all our results so loop 
		// through them one set of seven at a time (notice in the above big sql statement there 
		// are seven recordsets for each EMNID - one that just gives us the ID, and the other 
		// six that we pass into LoadFromEmnRecordsets() to be consumed by it).
		//DRT 9/13/2007 - PLID 27384 - Loop over our recordset of EMN IDs
		//while (prsNextRecordset) {
		while(!prsEMNIDs->eof) {
			CEMN *pEMN = new CEMN(this);
			// Eat the first recordset in order to get the EMNID
			long nEMNID = AdoFldLong(prsEMNIDs, "ID");
			//DRT 9/13/2007 - PLID 27384 - We now get this pointer above and keep it
			//prsNextRecordset = prsNextRecordset->NextRecordset(NULL);

			// Now pass the remaining six into LoadFromEmnID() to be consumed by it
			//DRT 7/27/2007 - PLID 26836 - We now have a parameter that tells us which EMN is going
			//	to be displayed to the user.  We will use that parameter to decide which EMNs can
			//	be loaded in the background and which need to use the smaller unit loading.  The one
			//	that is first displayed will load the first topic until it's visible, then load everything
			//	else in the background.  All others will always run in the background.
			BOOL bLoadInBackground = TRUE;
			if(nEMNID == nEMNIDToBeDisplayed || nEMNIDToBeDisplayed == -2) {
				//This EMN is the one that will be displayed to the user, so don't load it in the background.
				//If the one to be displayed is -2, this means we don't know yet, so we should display the first one.
				//-1 means that none of the loaded EMNs should be set as active.
				bLoadInBackground = FALSE;
				if(nEMNIDToBeDisplayed == -2)
					nEMNIDToBeDisplayed = nEMNID;
			}

			//DRT 9/13/2007 - PLID 27384 - This function no longer returns our position in the recordsets
			/*prsNextRecordset = */
			//pEMN->LoadFromEmnRecordsets(nEMNID, prsEMNData1, bLoadInBackground);
			pEMN->LoadFromEmnRecordsets(nEMNID, &lero, bLoadInBackground);

			// Finished loading this guy, add it to our array

			// (a.walling 2010-08-17 13:52) - PLID 38006 - Use AddEMN_Internal
			// (j.jones 2013-07-01 17:48) - PLID 57271 - we now use AddEMNSorted_Internal,
			// which adds identical to AddEMN_Internal except it will sort based
			// on the EMRLoadEMNsSortOrder preference
			long nSortOrder = GetRemotePropertyInt("EMRLoadEMNsSortOrder", 0, 0, GetCurrentUserName(), true);
			//0 - sort by order created (default: sorts by EMNID), 1 - sort by oldest date first, 2 - sort by newest date first
			AddEMNSorted_Internal(pEMN, nSortOrder);

			if (bReload) {
				pEMN->SetSaved(TRUE);
			}
			// Now we loop back up.  If LoadFromEmnRecordsets() consumed the last of our 
			// recordsets, then prsNextRecordset will now be NULL so we'll automatically break 
			// out of this loop.
			//DRT 9/13/2007 - PLID 27384 - Loop around the recordset of EMN IDs
			prsEMNIDs->MoveNext();
		}

		//DRT 9/17/2007 - PLID 27384 - When we have finished loading all, all the recordsets should be at the EOF point.  
		//	If they are not, then we've left something stranded and not loaded.  If you hit this error, then make sure
		//	the order of each piece of the main load query is in the same order as the EMN IDs are loaded.  
		//	**** If you hit this code, SOMETHING has not correctly loaded.  The rest of the EMR is not safe to use.
		if(!lero.AllAreEOF()) {
			ASSERT(FALSE);
			AfxThrowNxException("Failed to properly load all EMR data.  Some elements were not loaded.  Please try to restart Practice.");
		}

		if (!bReload) {
			//Load the patient information
			//DRT 8/28/2007 - PLID 27207 - Parameterized.
			_RecordsetPtr rsPatInfo = CreateParamRecordset("SELECT PersonT.BirthDate FROM PersonT WHERE PersonT.ID = {INT}", m_nPatientID);
			if(rsPatInfo->eof || rsPatInfo->Fields->Item["BirthDate"]->Value.vt == VT_NULL){
				//Set the patient birthdate to be invalid
				m_dtPatientBirthDate.SetStatus(COleDateTime::invalid);
			}
			else{
				m_dtPatientBirthDate = AdoFldDateTime(rsPatInfo, "BirthDate");
			}
			rsPatInfo->Close();
		}
	}
	if(m_bIsTemplate) {
		//Process all the actions we missed.
		UnlockSpawning();
	}
	else {
		//From now on, process actions.
		m_bIgnoreActions = FALSE;
	}

	if(nID != -1) {
		//We have finished loading, our EMNs may think that they've been modified, but they haven't.
		SetSaved(TRUE);
	}
}

// (j.jones 2007-08-02 11:42) - PLID 26915 - added ability to pass in a connection
void CEMR::LoadFromEmn(CEMN *pEMN, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	m_bIsTemplate = pEMN->IsTemplate();
	if(!pEMN->IsTemplate()) {
		//Load info about the EMR.
		//DRT 8/28/2007 - PLID 27207 - Parameterized.
		_RecordsetPtr rsEmrInfo = CreateParamRecordset(pCon, "SELECT ID, PatientID, Status, Description FROM EmrGroupsT "
			"WHERE ID = (SELECT EmrGroupID FROM EmrMasterT WHERE ID = {INT})", pEMN->GetID());
		if(rsEmrInfo->eof) {
			AfxThrowNxException("CEMR::LoadFromID called with invalid ID!");
		}
		m_nEMRID = AdoFldLong(rsEmrInfo, "ID");
		m_nPatientID = AdoFldLong(rsEmrInfo, "PatientID");
		m_strDescription = AdoFldString(rsEmrInfo, "Description");
		m_nStatus = AdoFldLong(rsEmrInfo, "Status");
		
		//store the "last saved" values
		m_nLastSavedStatus = m_nStatus;
		m_strLastSavedDescription = m_strDescription;
		
		rsEmrInfo->Close();
	}
	else {
		//Nothing to load.
		m_nEMRID = -1;
		m_nPatientID = -1;
	}
	ASSERT(!m_arypEMNs.GetSize());
	// (a.walling 2010-08-17 13:52) - PLID 38006 - Use AddEMN_Internal
	AddEMN_Internal(pEMN);
	m_bOwnChildren = FALSE;
	m_bIsUnsaved = FALSE;
}

//TES 11/22/2010 - PLID 41582 - Added nPicID (-1 for templates and the various places that create temporary EMRs)
void CEMR::CreateNew(long nPatientID, BOOL bIsTemplate, long nPicID /*= -1*/)
{
	m_nEMRID = -1;
	m_nPicID = nPicID;
	m_bIsTemplate = bIsTemplate;
	m_nPatientID = nPatientID;

	m_nStatus = 0;
	m_nLastSavedStatus = m_nStatus;

	//Load the patient information


	// (j.jones 2007-07-17 08:59) - PLID 26702 - don't bother trying to get a birthdate if the patient ID is -1
	if(m_nPatientID != -1) {

		// (j.jones 2007-07-17 09:26) - PLID 26702 - GetExistingPatientBirthDate will pull the birthdate
		// from the patient toolbar (by patient ID), and if the patient is not in the toolbar,
		// only then will it check the database. Returns COleDateTime::invalid if null.
		m_dtPatientBirthDate = GetExistingPatientBirthDate(m_nPatientID);

		/*
		_RecordsetPtr rsPatInfo = CreateRecordset("SELECT PersonT.BirthDate FROM PersonT WHERE PersonT.ID = %li", m_nPatientID);
		if(rsPatInfo->eof || rsPatInfo->Fields->Item["BirthDate"]->Value.vt == VT_NULL){
			//Set the patient birthdate to be invalid
			m_dtPatientBirthDate.SetStatus(COleDateTime::invalid);
		}
		else{
			m_dtPatientBirthDate = AdoFldDateTime(rsPatInfo, "BirthDate");
		}
		rsPatInfo->Close();
		*/
	}
	else {
		//Set the patient birthdate to be invalid
		m_dtPatientBirthDate.SetStatus(COleDateTime::invalid);
	}

	m_bIsUnsaved = TRUE;
}

// (a.walling 2012-03-23 15:27) - PLID 49187 - moved to inline// (a.walling 2008-06-27 15:03) - PLID 30482
// throws errors to caller
void CEMR::OverrideID(long nNewID)
{
	if (nNewID == m_nEMRID)
		return;

	long nOldID = m_nEMRID;

	_RecordsetPtr prs = CreateParamRecordset("SELECT Description, Status FROM EmrGroupsT WHERE ID = {INT}", nNewID);
	if (!prs->eof) {
		m_strDescription = AdoFldString(prs, "Description");
		m_nStatus = AdoFldLong(prs, "Status");

		//store the "last saved" values
		m_nLastSavedStatus = m_nStatus;
		m_strLastSavedDescription = m_strDescription;
	}

	m_nEMRID = nNewID;

	for (int i = 0; i < GetEMNCount(); i++) {
		CEMN* pEMN = GetEMN(i);
		if (pEMN) {
			BOOL bSetParentEMRChanged = TRUE;
			if (!pEMN->IsWritable()) {
				CWriteTokenInfo wtInfo;
				if (!pEMN->RequestWriteToken(wtInfo)) {
					// (a.walling 2008-06-27 17:12) - PLID 30482 - Failsafe. If we cannot acquire write access,
					// we need to update the EmrGroupID manually. This MUST occur or the EMN can be in limbo.
					// The possible consequence is that, if someone else already has this EMN open, they may
					// be unable to save their changes because the revision has changed. However, that is unlikely,
					// since these EMNs will probably be newly created. This entire situation is rare enough anyway.					
					ExecuteParamSql("UPDATE EmrMasterT SET EmrGroupID = {INT} WHERE EmrGroupID <> {INT} AND ID = {INT}",
						nNewID, nNewID, GetID());
					bSetParentEMRChanged = FALSE;
				}
			}
			if (bSetParentEMRChanged) {
				pEMN->SetParentEMRChanged();
			}
		}
	}

	m_bIsUnsaved = TRUE;
}

// (z.manning, 04/12/2007) - PLID 25600 - Removed optional parameter for category ID.
// (j.jones 2007-07-17 09:45) - PLID 26702 - added pSourceEMN, for when we create dummy EMNs
// for popups, we can query the source EMN for key data
// (z.manning 2009-03-04 15:03) - PLID 33338 - Use the new source action info class
CEMN* CEMR::AddEMNFromTemplate(long nTemplateID, SourceActionInfo &sai, CEMN *pSourceEMN, long nAppointmentID)
{
	LockSpawning();
	CEMN *pEMN = new CEMN(this);
	// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking
	if (-1 != nAppointmentID) {
		pEMN->SetAppointment(EMNAppointment(nAppointmentID));
	}

	pEMN->LoadFromTemplateID(nTemplateID, m_bIsTemplate, sai, FALSE, pSourceEMN);

#pragma TODO("PLID 55082 - This sets the EMN date to the last appointment date before now, but does so after loading so LoadDefaultProviderIDs will still use the current date. Is this intentional or not?")
	// (j.jones 2009-09-24 10:13) - PLID 29718 - added preference to default to the last appt. date
	COleDateTime dtToUse = COleDateTime::GetCurrentTime();
	if(GetRemotePropertyInt("EMNUseLastApptDate", 0, 0, "<None>", true) == 1) {
		COleDateTime dtLast = GetLastPatientAppointmentDate(GetPatientID());
		//make sure the appt. isn't before the patient's birthdate
		COleDateTime dtBirthDate = GetPatientBirthDate();		
		if(dtLast.GetStatus() != COleDateTime::invalid
			&& (dtBirthDate.GetStatus() == COleDateTime::invalid || dtLast >= dtBirthDate)) {
			dtToUse = dtLast;
		}
	}

	pEMN->SetEMNDate(dtToUse);

	// (a.walling 2010-08-17 13:52) - PLID 38006 - Use AddEMN_Internal
	AddEMN_Internal(pEMN);
	//(e.lally 2006-03-20) PLID 19754 - I moved the unlock spawning call to after this send message because
		//other messages can be sent while unlocking the spawning that rely on knowing the EMN was added.
	if(GetInterface()) {
		GetInterface()->SendMessage(NXM_EMN_ADDED, (WPARAM)pEMN);
	}
	UnlockSpawning();
	return pEMN;
}

CEMN* CEMR::AddNewTemplateEMN(long nCollectionID)
{
	LockSpawning();
	CEMN *pEMN = new CEMN(this);
	pEMN->CreateNewTemplateEMN(nCollectionID);
	//TES 2/20/2007 - PLID 24750 - This EMN is now finished loading (since it didn't have to load at all).
	pEMN->SetLoaded();
	// (a.walling 2010-08-17 13:52) - PLID 38006 - Use AddEMN_Internal
	AddEMN_Internal(pEMN);
	//(e.lally 2006-03-20) PLID 19754 - I moved the unlock spawning call to after this send message because
		//other messages can be sent while unlocking the spawning that rely on knowing the EMN was added.
	if(GetInterface()) {
		GetInterface()->SendMessage(NXM_EMN_ADDED, (WPARAM)pEMN);
	}
	UnlockSpawning();
	return pEMN;
}

// (a.walling 2010-04-02 11:56) - PLID 37923 - An EMN will notify the EMR when its initial load is complete
void CEMR::PostInitialEMNLoadComplete(CEMN* pEMN)
{
	// pEMN is not really used now, but might be in the future

	for (int i = 0; i < m_arypEMNs.GetSize(); i++) {
		if (m_arypEMNs[i]->IsLoading()) {
			return;
		}
	}

	// if we get here, all of our EMNs have completed loading.
	
	// ensure all the spawning EMN fields
	for (int i = 0; i < m_arypEMNs.GetSize(); i++) {
		m_arypEMNs[i]->EnsureSpawningEMNTextMergeField();
	}
	
	// and update any narratives that reference this
	for (int i = 0; i < m_arypEMNs.GetSize(); i++) {
		CEMN* pEMN = m_arypEMNs[i];

		int nTotalDetailCount = pEMN->GetTotalDetailCount();
		for (int j = 0; j < nTotalDetailCount; j++) {
			CEMNDetail* pDetail = pEMN->GetDetail(j);
			
			if(pDetail->m_EMRInfoType == eitNarrative) {
				// (a.walling 2009-11-19 14:55) - PLID 36369 - Only update if this narrative may actually have this detail
				// (a.walling 2010-04-01 10:44) - PLID 38013 - Consolidating some constant text strings
				if (pDetail->NarrativeMayHaveDetailField(NFIELD_EMN_SPAWNING_TEXT)) {
					pDetail->UpdateNarrativeFields();
				}
			}
		}
	}

	// (a.walling 2010-04-13 13:19) - PLID 37150 - Notify our interface that everything is now entirely loaded
	CWnd* pInterfaceWnd = GetInterface();
	if (::IsWindow(pInterfaceWnd->GetSafeHwnd())) {
		pInterfaceWnd->PostMessage(NXM_EMR_LOADED);
	}
}

void CEMR::AddEMN(CEMN *pEMN)
{
	// (a.walling 2010-08-17 13:52) - PLID 38006 - Use AddEMN_Internal
	AddEMN_Internal(pEMN);
	if(GetInterface()) {
		pEMN->SetInterface(GetInterface());
		GetInterface()->SendMessage(NXM_EMN_ADDED, (WPARAM)pEMN);
	}
}

// (a.walling 2010-08-17 13:51) - PLID 38006 - Add an EMN without notifying
void CEMR::AddEMN_Internal(CEMN* pEMN)
{
	_ASSERTE(pEMN != NULL); // we should never be adding an EMN that is NULL!
	m_arypEMNs.Add(pEMN);
}

// (j.jones 2013-07-01 17:48) - PLID 57271 - identical to AddEMN_Internal except
// that it sorts based on the EMRLoadEMNsSortOrder preference
void CEMR::AddEMNSorted_Internal(CEMN* pEMN, long nEMRLoadEMNsSortOrder)
{
	_ASSERTE(pEMN != NULL); // we should never be adding an EMN that is NULL!

	if(nEMRLoadEMNsSortOrder == 1) {
		//1 - sort by date ascending, EMNs with the same dates go by ID ascending
		//EMRMasterT.Date ASC, EMRMasterT.ID ASC
		for(int i=0;i<m_arypEMNs.GetSize();i++) {
			CEMN *pCompare = (CEMN*)m_arypEMNs.GetAt(i);
			if(pCompare != NULL && pEMN != NULL) {
				//if the current EMN index has a date newer than our new EMN,
				//or they are the same date and the current EMN ID is higher,
				//insert our new EMN before the current index
				if(pCompare->GetEMNDate() > pEMN->GetEMNDate()
					|| ((pCompare->GetEMNDate() == pEMN->GetEMNDate()) && pCompare->GetID() > pEMN->GetID())) {
					
					//insert here
					m_arypEMNs.InsertAt(i, pEMN);
					return;
				}
			}
		}
		//if still here, just add to the end
		m_arypEMNs.Add(pEMN);
		return;
	}
	else if(nEMRLoadEMNsSortOrder == 2) {
		//2 - sort by date descending, EMNs with the same dates go by ID ascending, keeping the creation order ascending on that date
		//EMRMasterT.Date DESC, EMRMasterT.ID ASC

		for(int i=0;i<m_arypEMNs.GetSize();i++) {
			CEMN *pCompare = (CEMN*)m_arypEMNs.GetAt(i);
			if(pCompare != NULL && pEMN != NULL) {
				//if the current EMN index has a date older than our new EMN,
				//or they are the same date and the current EMN ID is higher,
				//insert our new EMN before the current index
				if(pCompare->GetEMNDate() < pEMN->GetEMNDate()
					|| ((pCompare->GetEMNDate() == pEMN->GetEMNDate()) && pCompare->GetID() > pEMN->GetID())) {
					
					//insert here
					m_arypEMNs.InsertAt(i, pEMN);
					return;
				}
			}
		}
		//if still here, just add to the end
		m_arypEMNs.Add(pEMN);
		return;
	}
	else {
		//0 - sort by order created (default: sorts by EMNID, the caller would have done this)
		m_arypEMNs.Add(pEMN);
		return;
	}
}

int CEMR::GetEMNCount()
{
	return m_arypEMNs.GetSize();
}

CEMN* CEMR::GetEMN(int nIndex)
{
	if(nIndex < 0 || nIndex >= m_arypEMNs.GetSize()) {
		AfxThrowNxException("CEMR::GetEMN() called with invalid index!");
	}
	return m_arypEMNs[nIndex];
}

CEMN* CEMR::GetEMNByID(long nID)
{
	for(int i = 0; i < m_arypEMNs.GetSize(); i++) {
		if(m_arypEMNs[i]->GetID() == nID) return m_arypEMNs[i];
	}
	return NULL;
}

CEMN* CEMR::GetEMNByPointer(long nPointer)
{
	CEMN* pEMN = (CEMN*)nPointer;
	for(int i = 0; i < m_arypEMNs.GetSize(); i++) {
		if(m_arypEMNs[i] == pEMN) return m_arypEMNs[i];
	}
	return NULL;
}

// (a.walling 2012-10-04 12:41) - PLID 52878 - Verify that the given emr object pointer is a child

CEMN* CEMR::VerifyPointer(CEMN* pEMN)
{
	foreach (CEMN* p, GetAllEMNs()) {
		if (p == pEMN) {
			return p;
		}
	}
	return NULL;
}

CEMRTopic* CEMR::VerifyPointer(CEMRTopic* pTopic)
{
	foreach (CEMN* p, GetAllEMNs()) {
		if (p && p->VerifyPointer(pTopic)) {
			return pTopic;
		}
	}
	return NULL;
}

CEMNDetail* CEMR::VerifyPointer(CEMNDetail* pDetail)
{
	foreach (CEMN* p, GetAllEMNs()) {
		if (p && p->VerifyPointer(pDetail)) {
			return pDetail;
		}
	}
	return NULL;
}

long CEMR::GetPatientID()
{
	return m_nPatientID;
}

COleDateTime CEMR::GetPatientBirthDate()
{
	return m_dtPatientBirthDate;
}

BOOL CEMR::GetIsTemplate()
{
	return m_bIsTemplate;
}

//used when an item is changed from the administrator,
//for all instances of that item in the entire EMR
void CEMR::MarkItemChanged(long nEMRInfoID)
{
	//go through each EMN and call its MarkItemChanged function
	for(int i=0;i<m_arypEMNs.GetSize();i++) {
		m_arypEMNs.GetAt(i)->MarkItemChanged(nEMRInfoID);
	}
}

//refreshes all details currently loaded across the entire EMR
// (j.jones 2007-07-26 09:23) - PLID 24686 - this is a horrible idea that should never occur
/*
void CEMR::RefreshAllItems()
{
	CWaitCursor wc;

	//go through each EMN and call its RefreshAllItems function
	for(int i=0;i<m_arypEMNs.GetSize();i++) {
		//(e.lally 2007-02-09) PLID 24436 - Blindly telling EMNs to refresh
			//all items is a bad idea (IMO), but it's even worse for locked EMNs
			//because it is possible to give locked topics the appearance of being
			//modified. See also z.manning's comments for Refresh Content.
		if(m_arypEMNs.GetAt(i)->GetStatus() != 2) {
			m_arypEMNs.GetAt(i)->RefreshAllItems();
		}
	}
}
*/

// (j.jones 2007-07-26 09:10) - PLID 24686 - converted RefreshContent into two functions,
// accepting an InfoID or a MasterID
void CEMR::RefreshContentByInfoID(long nEMRInfoID, BOOL bSyncContentAndState /*= FALSE*/)
{
	// (c.haag 2006-03-31 10:13) - All this function does is set a flag that we
	// need to reload content for all of our EMN's. For example, if you call
	// RefreshContent on an invisible EMN, it will know to call LoadContent 
	// for its details when one of its topics is made visible. If no topic is
	// ever made visible, there's no point in loading content anyway because
	// LoadContent just synchronizes a detail with pracdata.

	// (c.haag 2004-07-08 13:30) - PLID 13373 - We now force the caller to decide
	// if bSetRegionAndInvalidate is set in RepositionDetailsInTab(). The reason is
	// that setting it to true will reset the positions of some controls. In most
	// cases we never want to do this. In 7600, we will revisit this and see the
	// best way to handle content positioning and sizing.

	// (c.haag 2004-05-28 15:54) - PLID 12289 - The new way of doing this is to
	// set a dirty content flag for all the EMNDetail items, and have 
	// RepositionDetailsInTab examine the flag and deal with it accordingly.

	//go through each EMN and call its RefreshContent function
	for(int i=0;i<m_arypEMNs.GetSize();i++) {
		// (z.manning, 01/29/2007) - PLID 24459 - We do not want to call RefreshContent on locked EMNs
		// because if LoadContent (which is used quite liberally) ever gets called on a detail on a locked
		// EMN, it will set that detail's topic as unsaved. Also, the cotent of a locked EMN should not
		// ever change anyway, so refreshing it is unneccesary.
		if(m_arypEMNs.GetAt(i)->GetStatus() != 2) {
			m_arypEMNs.GetAt(i)->RefreshContentByInfoID(nEMRInfoID, bSyncContentAndState);
		}
	}
}

// (j.jones 2007-07-26 09:10) - PLID 24686 - converted RefreshContent into two functions,
// accepting an InfoID or a MasterID
// (c.haag 2008-06-12 22:16) - PLID 27831 - Added papMasterDetails so that the caller may
// optionally get a list of all the details with a matching master ID
void CEMR::RefreshContentByInfoMasterID(long nEMRInfoMasterID, BOOL bSyncContentAndState /*= FALSE*/,
										CArray<CEMNDetail*,CEMNDetail*>* papMasterDetails /*= NULL */)
{
	// (c.haag 2006-03-31 10:13) - All this function does is set a flag that we
	// need to reload content for all of our EMN's. For example, if you call
	// RefreshContent on an invisible EMN, it will know to call LoadContent 
	// for its details when one of its topics is made visible. If no topic is
	// ever made visible, there's no point in loading content anyway because
	// LoadContent just synchronizes a detail with pracdata.

	// (c.haag 2004-07-08 13:30) - PLID 13373 - We now force the caller to decide
	// if bSetRegionAndInvalidate is set in RepositionDetailsInTab(). The reason is
	// that setting it to true will reset the positions of some controls. In most
	// cases we never want to do this. In 7600, we will revisit this and see the
	// best way to handle content positioning and sizing.

	// (c.haag 2004-05-28 15:54) - PLID 12289 - The new way of doing this is to
	// set a dirty content flag for all the EMNDetail items, and have 
	// RepositionDetailsInTab examine the flag and deal with it accordingly.

	//go through each EMN and call its RefreshContent function
	for(int i=0;i<m_arypEMNs.GetSize();i++) {
		// (z.manning, 01/29/2007) - PLID 24459 - We do not want to call RefreshContent on locked EMNs
		// because if LoadContent (which is used quite liberally) ever gets called on a detail on a locked
		// EMN, it will set that detail's topic as unsaved. Also, the cotent of a locked EMN should not
		// ever change anyway, so refreshing it is unneccesary.
		if(m_arypEMNs.GetAt(i)->GetStatus() != 2) {
			m_arypEMNs.GetAt(i)->RefreshContentByInfoMasterID(nEMRInfoMasterID, bSyncContentAndState, papMasterDetails);
		}
	}
}

// (a.wetta 2007-04-09 13:30) - PLID 25532 - This function refreshes the content
// all all EMR items of a certain type.
// (a.walling 2008-12-19 09:21) - PLID 29800 - This was only used for images, and only to refresh the custom stamps, which was causing the content
// to be reloaded. This is all unnecessary, and the custom stamps is entirely UI. So let's just do what we need to do, and refresh the custom stamps,
// rather than flag as needed to reload content. This is all controlled by the new bRefreshCustomStampsOnly param. I could have renamed the function
// entirely, but I can see how this might come in handy in the future.
void CEMR::RefreshContentByType(EmrInfoType eitItemType, BOOL bSyncContentAndState, BOOL bRefreshCustomStampsOnly)
{
	// (c.haag 2006-03-31 10:13) - All this function does is set a flag that we
	// need to reload content for all of our EMN's. For example, if you call
	// RefreshContent on an invisible EMN, it will know to call LoadContent 
	// for its details when one of its topics is made visible. If no topic is
	// ever made visible, there's no point in loading content anyway because
	// LoadContent just synchronizes a detail with pracdata.

	// (c.haag 2004-07-08 13:30) - PLID 13373 - We now force the caller to decide
	// if bSetRegionAndInvalidate is set in RepositionDetailsInTab(). The reason is
	// that setting it to true will reset the positions of some controls. In most
	// cases we never want to do this. In 7600, we will revisit this and see the
	// best way to handle content positioning and sizing.

	// (c.haag 2004-05-28 15:54) - PLID 12289 - The new way of doing this is to
	// set a dirty content flag for all the EMNDetail items, and have 
	// RepositionDetailsInTab examine the flag and deal with it accordingly.

	//go through each EMN and call its RefreshContent function
	for(int i=0;i<m_arypEMNs.GetSize();i++) {
		// (z.manning, 01/29/2007) - PLID 24459 - We do not want to call RefreshContent on locked EMNs
		// because if LoadContent (which is used quite liberally) ever gets called on a detail on a locked
		// EMN, it will set that detail's topic as unsaved. Also, the cotent of a locked EMN should not
		// ever change anyway, so refreshing it is unneccesary.
		if(m_arypEMNs.GetAt(i)->GetStatus() != 2) {
			m_arypEMNs.GetAt(i)->RefreshContentByType(eitItemType, bSyncContentAndState, bRefreshCustomStampsOnly);
		}
	}
}

//if editing a locked item creates a new copy, and we have unsaved items
//using the old info item, make them use the new info item
void CEMR::UpdateInfoID(long nOldEMRInfoID, long nNewEMRInfoID, EMRInfoChangedIDMap* pChangedIDMap)
{
	//go through each EMN and call its UpdateInfoID function
	for(int i=0;i<m_arypEMNs.GetSize();i++) {
		m_arypEMNs.GetAt(i)->UpdateInfoID(nOldEMRInfoID, nNewEMRInfoID, pChangedIDMap);
	}
}

//DRT 2/24/2006 - PLID 19465 - This function is ONLY for use in generating the string to save.  This function should NOT
//	be changing any member variables under the assumption that the save succeeded.  All code that needs to be updated after
//	the save succeeds should be placed in PostSaveUpdate()
// (j.jones 2007-01-11 14:28) - PLID 24027 - tracked strPostSaveSql, for sql statements to occur after the main save
// (c.haag 2007-06-20 12:41) - PLID 26397 - We now store saved objects in a map for fast lookups.
// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
// (j.jones 2012-09-27 15:11) - PLID 52820 - now we track a flag if something that contributed to drug interactions has changed,
// such as new or deleted prescriptions, or new or deleted diagnosis codes
// (a.walling 2014-01-30 00:00) - PLID 60542 - Quantize
Nx::Quantum::Batch CEMR::GenerateSaveString(long &nAuditTransactionID, Nx::Quantum::Batch& strPostSaveSql, CStringArray &arystrErrors, IN OUT CMapPtrToPtr& mapSavedObjects, CDWordArray &arynAffectedDetailIDs, OUT BOOL &bDrugInteractionsChanged, BOOL bSaveRecordOnly)
{
	Nx::Quantum::Batch strSaveString;

	nAuditTransactionID = -1;

	// (j.jones 2012-01-31 17:16) - PLID 47878 - this save might be the first notification
	// that the EMR is being changed, if so then we need to know that we've made changes
	if(IsEMRUnsaved()) {
		m_bChangesMadeThisSession = TRUE;
	}

	//first save the EMR data
	CString strDescription = m_strDescription;
	long nStatus = m_nStatus;

	if(!m_bIsTemplate)
	{
		// (z.manning 2009-05-21 14:44) - PLID 34297 - Problems are no longer associated with a single object
		// but rather can be linked to as many objects as the user desires. As a result, ALL problems are saved
		// at the EMR level now.
		CArray<CEmrProblem*,CEmrProblem*> arypProblems;
		GetAllProblems(arypProblems, TRUE);
		for(int nProblemIndex = 0; nProblemIndex < arypProblems.GetSize(); nProblemIndex++) {
			CEmrProblem *pProblem = arypProblems.GetAt(nProblemIndex);

			// (j.jones 2009-06-05 14:43) - PLID 34301 - if we are creating the problem,
			// verify we have valid problem links for it
			CArray<CEmrProblemLink*,CEmrProblemLink*> arypProblemLinks;

			ASSERT(pProblem->m_bDoNotShowOnCCDA > -1);
			if(pProblem->m_nID == -1 && !pProblem->m_bIsDeleted) {

				GetAllProblemLinks(arypProblemLinks, pProblem, FALSE);

				if(arypProblemLinks.GetSize() == 0) {
					//this can happen if we created a problem for a new detail, and removed that detail
					//prior to saving, without also linking anything else to the problem
					pProblem->m_bIsDeleted = TRUE;
				}
			}

			BOOL bWasDeleted = pProblem->m_bIsDeleted;

			// (c.haag 2009-05-28 13:14) - PLID 34277 - Pass in the interface window as the last parameter
			SaveProblem(strSaveString, nAuditTransactionID, pProblem, mapSavedObjects, GetPatientID()
				, GetExistingPatientName(GetPatientID()), GetInterface(), GetDeletedEmrProblemIDs());

			// (j.jones 2009-06-18 16:54) - PLID 34301 - It is possible (albeit rare) for the SaveProblem function to
			// abort saving and flag the problem as deleted. If so, let's cleanly mark all its links as deleted, even
			// though the saving code should be smart enough to handle that case.
			if(!bWasDeleted && pProblem->m_bIsDeleted) {
				//don't reload if we already did earlier
				if(arypProblemLinks.GetSize() == 0) {
					GetAllProblemLinks(arypProblemLinks, pProblem, FALSE);
				}
				for(int i=0; i<arypProblemLinks.GetSize(); i++) {
					CEmrProblemLink *pLink = arypProblemLinks.GetAt(i);
					pLink->SetDeleted();
				}
			}
		}
	}

	if(m_nEMRID == -1) {
		//save new

		if(m_bIsTemplate) {

			//This is the EMRGroupsT level, we don't need to do anything here for a template
		}
		else {

			//will we be creating this here? This is the EMRGroupsT level...

			//new patient EMR record			
			// (j.armen 2013-05-07 16:59) - PLID 56587 - EMRGroupsT.ID is now an identity
			AddStatementToSqlBatch(strSaveString, 
				"INSERT INTO EMRGroupsT (PatientID, Description, Status, InputDate) \r\n"
				"VALUES (%li, '%s', %li, GetDate()) \r\n"
				"SELECT @nNewObjectID = SCOPE_IDENTITY() \r\n",
				m_nPatientID, _Q(strDescription), nStatus);

			// (j.armen 2013-05-08 12:21) - PLID 56587 - We now have to add the new object after it is created
			AddNewEMRObjectToSqlBatch(strSaveString, esotEMR, (long)this, mapSavedObjects);

			//TES 11/22/2010 - PLID 41582 - If we have a PicID, then update it now.
			if(m_nPicID != -1) {
				AddStatementToSqlBatch(strSaveString, "UPDATE PicT SET EmrGroupID = @nNewObjectID WHERE ID = %li", m_nPicID);
			}

			// (j.jones 2008-07-22 15:06) - PLID 30789 - save all our problems
			// (z.manning 2009-05-22 09:49) - PLID 34297 - We now just have links to problems
			AddStatementToSqlBatch(strSaveString, "SET @nNewObjectIDForProblems = @nNewObjectID");
			SaveProblemLinkArray(strSaveString, m_apEmrProblemLinks, "@nNewObjectIDForProblems", mapSavedObjects
				, nAuditTransactionID, m_nPatientID, GetExistingPatientName(m_nPatientID));
		}
	}
	else {

		if(m_bIsUnsaved) {
			//update existing

			if(m_bIsTemplate) {

				//This is the EMRGroupsT level, we don't need to do anything here for a template
			}
			else {
				//update existing patient EMR record

				AddStatementToSqlBatch(strSaveString, "UPDATE EMRGroupsT SET Description = '%s', Status = %li "
					"WHERE ID = %li",_Q(strDescription), nStatus, m_nEMRID);

				// (j.jones 2008-07-22 15:06) - PLID 30789 - save all our problems
				// (z.manning 2009-05-22 09:55) - PLID 34297 - Just save problem links
				SaveProblemLinkArray(strSaveString, m_apEmrProblemLinks, AsString(m_nEMRID), mapSavedObjects
					, nAuditTransactionID, m_nPatientID, GetExistingPatientName(m_nPatientID));

				//Auditing

				//grab the patient name from the last EMN or from the current name
				CString strPatientName;
				CEMN *pMostRecentEMN = NULL;
				if(m_arypEMNs.GetSize() > 0) {
					pMostRecentEMN = m_arypEMNs.GetAt(m_arypEMNs.GetSize() - 1);
				}
				if(pMostRecentEMN) {
					strPatientName = pMostRecentEMN->GetPatientNameLast() + ", " + pMostRecentEMN->GetPatientNameFirst() + " " + pMostRecentEMN->GetPatientNameMiddle();
				}
				else {
					strPatientName = GetExistingPatientName(m_nPatientID);
				}

				//Status
				if(m_nLastSavedStatus != nStatus) {
					AuditEventPriorities aep = aepMedium;
					CString strOldStatus;
					switch(m_nLastSavedStatus) {
					case 0: 
						strOldStatus = "Open";
						break;
					case 1: 
						strOldStatus = "Closed";
						break;
					}
					CString strNewStatus;
					switch(nStatus) {
					case 0:
						strNewStatus = "Open";
						break;
					case 1:
						strNewStatus = "Closed";
						break;
					}
					if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(GetPatientID(), strPatientName, nAuditTransactionID, aeiEMRStatus, m_nEMRID, strOldStatus, strNewStatus, aep, aetChanged);
				}

				//Description
				if(m_strLastSavedDescription != m_strDescription) {
					if(nAuditTransactionID == -1) nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(GetPatientID(), strPatientName, nAuditTransactionID, aeiEMRDescription, m_nEMRID, m_strLastSavedDescription, m_strDescription, aepMedium, aetChanged);
				}
			}
		}
	}

	//now save all EMNs

	m_aryPendingDeleteEMNs.RemoveAll();
	for(int i=m_aryDeletedEMNs.GetSize() - 1; i >= 0; i--) {
		// (a.walling 2008-05-22 16:43) - PLID 22049
		if(m_aryDeletedEMNs.GetAt(i)->IsWritable()) {
			// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
			strSaveString += m_aryDeletedEMNs.GetAt(i)->GenerateDeleteString(nAuditTransactionID, arystrErrors, arynAffectedDetailIDs);
			// (a.walling 2008-08-13 13:53) - PLID 22049 - Don't delete anything until we are in PostSaveUpdate!!
			//delete m_aryDeletedEMNs.GetAt(i);
			//m_aryDeletedEMNs.RemoveAt(i);
			m_aryPendingDeleteEMNs.Add(m_aryDeletedEMNs.GetAt(i));
		} else if (!SafeGetIgnoreReadOnly()) { // (a.walling 2008-07-07 13:03) - PLID 30513
			arystrErrors.Add(FormatString("The EMN '%s' will not be deleted because write access has not been acquired", m_aryDeletedEMNs.GetAt(i)->GetDescription()));
		}
	}
	// (a.walling 2008-08-13 13:53) - PLID 22049 - Don't delete anything until we are in PostSaveUpdate!!
	//m_aryDeletedEMNs.RemoveAll();

	//TODO: eventually we need to set it up so we only have certain EMNs open for editing,
	//and this we would only save those EMNs
	//if saving the record only, then do not save any child EMNs
	if(!bSaveRecordOnly) {
		// (a.walling 2010-07-30 13:03) - PLID 38006 - We need to ensure these EMNs are saved in order of dependency.
		CArray<CEMN*, CEMN*> orderedEMNs;
		{
			CArray<CEMN*, CEMN*> allEMNs;

			allEMNs.Copy(m_arypEMNs);
			
			// first get all EMNs that are saved or have no dependencies
			for(int i=0;i<allEMNs.GetSize();i++) {
				CEMN* pEMN = allEMNs.GetAt(i);

				if (pEMN->GetID() != -1 || pEMN->GetSourceActionInfo().pSourceDetail == NULL) {
					orderedEMNs.Add(pEMN);
					allEMNs.RemoveAt(i);
					i--;
				}
			}

			while (!allEMNs.IsEmpty()) {
				int nFound = 0;
				for(int i=0;i<allEMNs.GetSize();i++) {
					bool bFound = false;

					CEMN* pEMN = allEMNs.GetAt(i);
					CEMN* pDependentEMN = pEMN->GetSourceActionInfo().pSourceDetail->GetParentEMN();

					if (pDependentEMN == NULL) {
						bFound = true;
					} else {
						for (int j = 0; !bFound && j < orderedEMNs.GetSize(); j++) {
							if (pDependentEMN == orderedEMNs.GetAt(j)) {
								bFound = true;
							}
						}
					}
					
					if (bFound) {
						nFound++;			
						orderedEMNs.Add(pEMN);		
						allEMNs.RemoveAt(i);
						i--;
					}
				}

				if (nFound == 0 && !allEMNs.IsEmpty()) {
					for (int i = 0; i < allEMNs.GetSize(); i++) {
						orderedEMNs.Add(allEMNs.GetAt(i));	
						allEMNs.RemoveAt(i);
						i--;
					}
				}
			}
		}

		for(int i=0;i<orderedEMNs.GetSize();i++) {
			CEMN* pEMN = orderedEMNs.GetAt(i);

			// (a.walling 2008-05-22 16:41) - PLID 22049
			if(orderedEMNs.GetAt(i)->IsWritable()) {
				// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
				strSaveString += pEMN->GenerateSaveString(m_nEMRID, nAuditTransactionID, strPostSaveSql, arystrErrors, mapSavedObjects, arynAffectedDetailIDs, bDrugInteractionsChanged, FALSE, FALSE);
			} else if (orderedEMNs.GetAt(i)->IsUnsaved() && !SafeGetIgnoreReadOnly()) { // (a.walling 2008-07-07 13:04) - PLID 30513
				arystrErrors.Add(FormatString("The EMN '%s' will not be saved because write access has not been acquired", pEMN->GetDescription()));
			}
		}
	}

	//We have now had our save string generated.
	mapSavedObjects[this] = this;

	//the calling function is responsible for saving the strSaveString 
	//AND calling CommitAuditTransaction if nAuditTransactionID != -1
	return strSaveString;
}

// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
void CEMR::LogEmrObjectData(int nIndent, BOOL bForceDeletedFlagTrue)
{
	BOOL bDeleted = bForceDeletedFlagTrue;

	// Log this object
	::LogEmrObjectData(nIndent, m_nEMRID, this, esotEMR, (m_nEMRID == -1), m_bIsUnsaved, bDeleted, m_strDescription,
		"m_nStatus = %d  m_bIsTemplate = %d"
		, m_nStatus
		, m_bIsTemplate);

	// Log problems and problem links
	for (auto l : m_apEmrProblemLinks)
	{
		if (nullptr != l)
		{
			CEmrProblem* p = l->GetProblem();
			if (nullptr != p)
			{
				p->LogEmrObjectData(nIndent + 1);
			}
			if (nullptr != l)
			{
				l->LogEmrObjectData(nIndent + 1);
			}
		}
	}

	// Log EMNs: (int nIndent, BOOL bDeleted, BOOL bInEMRPendingDeleteEMNsAry)
	for (auto e : m_arypEMNs)
	{
		e->LogEmrObjectData(nIndent + 1, FALSE, FALSE);
	}
	for (auto e : m_aryPendingDeleteEMNs)
	{
		e->LogEmrObjectData(nIndent + 1, TRUE, TRUE);
	}
	for (auto e : m_aryDeletedEMNs)
	{
		e->LogEmrObjectData(nIndent + 1, TRUE, FALSE);
	}
}

//DRT 2/24/2006 - PLID 19465 - This is a notification function that lets us know a save has just taken place.  This function
//	should contain any code that updates members, etc based on the successful save, not GenerateSaveString()
// (a.walling 2007-10-18 16:33) - PLID 27664 - Added array to gather all topics affected in the PostSaveUpdate cascade.
void CEMR::PostSaveUpdate(CShowProgressFeedbackDlg* pProgressDlg, BOOL bSaveRecordOnly /*= FALSE*/, CArray<CEMRTopic*, CEMRTopic*> *parTopicsAffected /*= NULL*/)
{
	///////////////////////////////////
	//Do all of your EMR updating here.  Keep in mind that PropagateIDs has not yet been called, so
	//	you may check for -1 for a new template, if necessary.

	//At this point, we've saved all our fields.
	m_nLastSavedStatus = m_nStatus;
	m_strLastSavedDescription = m_strDescription;

	// (a.walling 2008-07-07 14:15) - PLID 30513 - The ignore flag will expire after the first save
	if (m_bLastSavedIgnoreReadOnlyEMNs) {
		m_bLastSavedIgnoreReadOnlyEMNs = m_bIgnoreReadOnlyEMNs;
		m_bIgnoreReadOnlyEMNs = FALSE;
	} else {
		m_bLastSavedIgnoreReadOnlyEMNs = m_bIgnoreReadOnlyEMNs;
	}

	// (z.manning 2009-05-22 11:56) - PLID 34332 - Update problem links
	for(int nProblemLinkIndex = 0; nProblemLinkIndex < m_apEmrProblemLinks.GetSize(); nProblemLinkIndex++)
	{
		CEmrProblemLink *pProblemLink = m_apEmrProblemLinks.GetAt(nProblemLinkIndex);
		if(pProblemLink != NULL && (pProblemLink->GetIsDeleted() || (pProblemLink->GetProblem() != NULL && pProblemLink->GetProblem()->m_bIsDeleted))) {
			m_apEmrProblemLinks.RemoveAt(nProblemLinkIndex--);
			delete pProblemLink;
		}
	}

	//End all EMR updating here.
	/////////////////////////////

	// (a.walling 2008-08-13 13:56) - PLID 22049 - Now that we have succeeded, actually do the cleanup for deleted EMNs

	// (a.walling 2010-08-24 15:27) - PLID 37923 - Delay deleting the EMN until after iterating through and clearing the source info
	CArray<CEMN*> arFinallyDeletedEMNs;

	for (int j = m_aryDeletedEMNs.GetSize() - 1; j >= 0; j--) {
		for (int i = 0; i < m_aryPendingDeleteEMNs.GetSize(); i++) {
			if (m_aryDeletedEMNs.GetAt(j) == m_aryPendingDeleteEMNs.GetAt(i)) {

				// (a.walling 2010-03-31 14:08) - PLID 38008 - We need to ensure no other EMN's source action info is using a pointer to a deleted detail
				// or has an ID that is now deleted
				for (int x = 0; x < m_arypEMNs.GetSize(); x++) {
					SourceActionInfo sai = m_arypEMNs[x]->GetSourceActionInfo();
					if (!sai.IsBlank() &&
						(
						(sai.pSourceDetail != NULL && m_aryDeletedEMNs.GetAt(j)->GetDetailByPointer((long)sai.pSourceDetail) != NULL) ||
						(sai.nSourceDetailID != -1 && m_aryDeletedEMNs.GetAt(j)->GetDetailByID(sai.nSourceDetailID) != NULL)
						))
					{
						m_arypEMNs[x]->ClearSourceActionInfo();
					}
				}
				//delete m_aryDeletedEMNs.GetAt(j);
				arFinallyDeletedEMNs.Add(m_aryDeletedEMNs.GetAt(j));
				m_aryDeletedEMNs.RemoveAt(j);
				m_aryPendingDeleteEMNs.RemoveAt(i);
				break;
			}
		}
	}
	m_aryPendingDeleteEMNs.RemoveAll();

	for (int i = 0; i < arFinallyDeletedEMNs.GetSize(); i++) {
		delete arFinallyDeletedEMNs[i];
	}

	//This should be similar to GenerateSaveString - If we call an EMR PostSaveUpdate(), we need to call the same function for
	//	all of our children.
	if(!bSaveRecordOnly) {
		for(int i=0;i<m_arypEMNs.GetSize();i++) {			
			// (a.walling 2008-06-09 13:09) - PLID 22049 - Don't do so if they are not writable
			if (m_arypEMNs.GetAt(i)->IsWritable()) {
				// (a.walling 2007-10-18 16:36) - PLID 27664 - Pass the array to gather all affected topics
				m_arypEMNs.GetAt(i)->PostSaveUpdate(pProgressDlg, FALSE, FALSE, parTopicsAffected);
			}
		}
	}
}

// (z.manning 2009-06-15 17:05) - PLID 34332 - Moved the post save update logic for problems to 
// its own function
void CEMR::PostSaveUpdateProblems()
{
	// (c.haag 2008-08-25 10:09) - PLID 30779 - Flag all problems as unmodified. Also need
	// to delete problems that were flagged as deleted from memory.
	// (z.manning 2009-05-22 12:04) - PLID 34332 - Use the master heap problem array
	// (z.manning 2009-05-22 12:39) - PLID 34332 - This needs to be AFTER we've called 
	// PostSaveUpdate for everything so that all problem links have been updated.
	for(int nProb = 0; nProb < m_apEmrProblemHeap.GetSize(); nProb++) {
		CEmrProblem *pProblem = m_apEmrProblemHeap.GetAt(nProb);
		if(pProblem != NULL && pProblem->m_bIsDeleted) {
			m_apEmrProblemHeap.RemoveAt(nProb--);
			int nRefCount = pProblem->Release();
			// (c.haag 2009-07-10 15:48) - PLID 34760 - This can fire if a DetailPopup object
			// still owns the detail holding a reference to this problem. This is scheduled to
			// be properly addressed in a future release when we have more time.
			// (c.haag 2009-07-16 14:24) - PLID 34760 - This can also fire if you have a patient
			// EMR with no topics (except for More Info of course), and then you make a charge
			// linked with an existing problem for an existing lab, then save the EMR; close, open,
			// delete the charge and save again. The extra reference is kept by the CEMNLoader,
			// which is not deleted when the initial load finishes; but instead, when the EMR is
			// deleted. (EMR's with topics delete their CEMNLoader right when the initial load
			// finishes).
			// (c.haag 2009-08-03 09:14) - PLID 34760 - This assertion is now deprecated. It
			// acted as a catch for bugs that would have been introduced by new problem list 
			// code implemented in the 9300 scope. It can still fire on the circumstances above;
			// but it's just legacy behavior that doesn't actually cause any problems. This should
			// no longer be necessary. Furthermore, there's nothing to worry about with memory
			// leaks since the CEMR object will delete any problems that still exist in memory.
			//_ASSERTE(nRefCount == 0);
		}
		else {
			pProblem->m_bIsModified = FALSE;
				
			pProblem->m_strLastDescription = pProblem->m_strDescription;
			pProblem->m_nLastStatusID = pProblem->m_nStatusID;
			pProblem->m_dtLastOnsetDate = pProblem->m_dtOnsetDate;
			
			// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
			// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
			pProblem->m_nLastChronicityID = pProblem->m_nChronicityID;

			// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
			pProblem->m_nLastDiagICD9CodeID = pProblem->m_nDiagICD9CodeID;
			pProblem->m_nLastDiagICD10CodeID = pProblem->m_nDiagICD10CodeID;

			// (b.spivey, October 22, 2013) - PLID 58677 - added codeID
			pProblem->m_nLastCodeID = pProblem->m_nCodeID; 
			
			// (s.tullis 2015-02-23 15:44) - PLID 64723 
			pProblem->m_bLastDoNotShowOnCCDA = pProblem->m_bDoNotShowOnCCDA;
			// (r.gonet 2015-03-09 18:21) - PLID 65008 - Remember the original value of  DoNotShowOnProblemPrompt so we can audit later.
			pProblem->m_bLastDoNotShowOnProblemPrompt = pProblem->m_bDoNotShowOnProblemPrompt;
		}
	}
}

// (j.jones 2012-10-03 15:47) - PLID 36220 - if current meds. change, we will update bDrugInteractionsChanged
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
void CEMR::PostSaveUpdateCurrentMedications(CShowProgressFeedbackDlg* pProgressDlg, CEMNDetail *pCurrentMedsDetail, OUT BOOL &bDrugInteractionsChanged, IN OUT CDWordArray &arNewCDSInterventions)
{
	//
	// (c.haag 2007-01-25 10:21) - PLID 24396 - Now that the EMR has been saved, we need
	// to check if the patient Current Medications as defined by the most recent EMN is
	// different from the patient current medications list. If they are different, we must
	// give the user the opportunity to change the patient current medications list to be 
	// consistent with the EMR.
	//
	if (m_bIsTemplate) {
		ASSERT(FALSE); // We should never get here!
		return;
	}
	if (GetPatientID() <= 0) {
		ASSERT(FALSE); // We should never get here!
		return;
	}

	try {
		//DRT 8/15/2007 - PLID 26527 - The previous behavior was for SaveEMRObject to get a pointer to the EMR which contained the
		//	current medications detail that was last updated.  It then called this function on that EMR.  This function ran a query
		//	to lookup the last EMN ID and Detail ID of the current meds item saved (looking at last modified date).  But really, 
		//	we already have a fill list of details that are being saved back in SaveEMRObject, so we can just grab the EMNDetail*
		//	at that point and pass it in here.  This removes the recordset, and fixes a potential issue where you could get an EMNID
		//	but lack a detail ID, which resulted in some undefined behavior.  That behavior is now defined -- if you have no detail, 
		//	we quit with no warning/message.
		//Note on slight behavior change.  First, note that by design of the medications item, if it exists more than once on a single EMN, 
		//	all copies of the medication item will update themselves to match.  You cannot have 1 EMN with 2 "Current Medications" which are
		//	not identical.  There are 3 save types that affect this function.  By topic -- in this case, you can only ever have 1 detail, 
		//	so the previous sort on modified date has no bearing.  By EMN -- you can still only ever have 1 detail, so the sort on modified
		//	date has no bearing.  By EMR.  In this case, if you previously had 2 EMNs, each with a medication, then it was more or less
		//	arbitrary which one was the "last saved".  The behavior remains the same (arbitrary), though it will more often than not be
		//	the first 'Current Medication' item in your EMR, from top to bottom.  It may not be the same detail that was previously chosen.
		//Also note that if only 1 medication changes on an EMR which has multiple, that detail is the one that should prompt to update, 
		//	regardless of its order.
		if(pCurrentMedsDetail == NULL) {
			//This should not happen
			ASSERT(FALSE);
			return;
		}

		//Still need the EMN pointer below, so get it from the detail
		CEMN *pEMN = pCurrentMedsDetail->m_pParentTopic->GetParentEMN();
		if(pEMN == NULL) {
			//This should be impossible, but we must fail if so
			ASSERT(FALSE);
			return;
		}

		// (j.jones 2013-02-12 15:23) - PLID 55139 - do not sync current meds. unless using the NewCrop license
		CLicense::EPrescribingType eRxType = g_pLicense->HasEPrescribing(CLicense::cflrSilent);
		if(eRxType != CLicense::eptNewCrop) {
			//give a warning about this
			CString strERxName = "e-Prescribing";
			if(eRxType == CLicense::eptNone) {
				//call this "Medications" if no E-Rx license of any sort
				strERxName = "Medications";
			}
			CString strWarning;
			strWarning.Format("Changes to the Current Medications table will not update the patient's current medications in the Medications tab.\n\n"
				"To update the patient's Current Medications, click on the %s button in the toolbar to access this list.", strERxName);

			DontShowMeAgain((CWnd*)GetInterface(), strWarning, "EMRCurrentMedsSyncingDisabled");
			return;
		}

		// (j.jones 2011-05-06 11:25) - PLID 43527 - see if the Sig column exists, as we may
		// skip some things if it doesn't
		TableColumn *ptcSig = pCurrentMedsDetail->GetColumnByListSubType(lstCurrentMedicationSig);

		//
		// Now pull the medication ID's of the EMN by the detail we found
		//
		CArray<long,long> anEMNCurMedIDs;
		CArray<long,long> anEMNCurMedDataIDs;
		CStringArray astrDetailDrugListNames;
		CMap<long, long, CString, LPCTSTR> mapEMNCurMedDataIDsToSig;
		// (c.haag 2007-08-18 12:08) - PLID 27113 - Populate the arrays through the detail
		// (j.jones 2011-05-04 10:26) - PLID 43527 - added mapEMNCurMedDataIDsToSig, which maps a DataID
		// to the Sig entered in the table
		pCurrentMedsDetail->PopulateCurrentMedicationIDArrays(anEMNCurMedDataIDs, anEMNCurMedIDs, astrDetailDrugListNames, mapEMNCurMedDataIDsToSig);

		if(ptcSig == NULL && mapEMNCurMedDataIDsToSig.GetSize() > 0) {
			//we should not have filled the map if there is no Sig column
			ASSERT(FALSE);

			//but if we did, clear the map, we don't want to save the Sig
			mapEMNCurMedDataIDsToSig.RemoveAll();
		}

		//
		// Now pull the medication ID's for the patient
		//
		CArray<long,long> anPtCurMedIDs;
		CArray<long,long> anPtCurMedDataIDs;
		CMap<long, long, CString, LPCTSTR> mapPtCurMedDataIDsToSig;
		CStringArray astrPtDrugListNames;
		BOOL bCurHasNoMedsStatus = FALSE;

		// (c.haag 2007-02-02 18:00) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
		//DRT 8/28/2007 - PLID 27207 - Parameterized.
		// (c.haag 2009-06-12 10:24) - PLID 34611 - Filter out discontinued current medications
		// (j.jones 2012-10-17 14:08) - PLID 51713 - Added HasNoMeds
		_RecordsetPtr prsPt = CreateParamRecordset("SELECT HasNoMeds FROM PatientsT WHERE PersonID = {INT}\r\n"
			""
			"SELECT MedicationID, EMRDataID, EMRDataT.Data AS Name, CurrentPatientMedsT.Sig "
			"FROM CurrentPatientMedsT "
			"LEFT JOIN DrugList ON DrugList.ID = CurrentPatientMedsT.MedicationID "
			"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"WHERE PatientID = {INT} AND Discontinued = 0 "
			"ORDER BY Name ",
			GetPatientID(), GetPatientID());
		if(!prsPt->eof) {
			bCurHasNoMedsStatus = VarBool(prsPt->Fields->Item["HasNoMeds"]->Value, FALSE);
		}
		prsPt = prsPt->NextRecordset(NULL);
		while (!prsPt->eof) {
			anPtCurMedIDs.Add(AdoFldLong(prsPt, "MedicationID"));
			long nDataID = AdoFldLong(prsPt, "EMRDataID");
			anPtCurMedDataIDs.Add(nDataID);
			astrPtDrugListNames.Add(AdoFldString(prsPt, "Name"));
			// (j.jones 2011-05-04 10:26) - PLID 43527 - added mapPtCurMedDataIDsToSig, which maps a DataID
			// to the Sig entered in the medications tab
			mapPtCurMedDataIDsToSig.SetAt(nDataID, AdoFldString(prsPt, "Sig", ""));
			prsPt->MoveNext();
		}
		prsPt->Close();

		// (j.jones 2013-01-14 09:43) - PLID 54462 - If NewCrop is in use, and medication reconciliation is enabled,
		// and the option to skip updating the blue current meds. table with reconciled meds. is also enabled,
		// then any prescriptions on this EMN should have been reconcilied to the current medications list.
		// Remove those meds. from each list so that the blue table can never update current medications that are
		// also prescriptions on this EMN.
		BOOL bReconcileMeds = (GetRemotePropertyInt("ReconcileNewRxWithCurMeds", 0, 0, "<None>", true) == 1);
		BOOL bSkipEMRTable = (GetRemotePropertyInt("ReconcileNewRxWithCurMeds_SkipEMRTable", 1, 0, "<None>", true) == 1);

		if(bReconcileMeds && bSkipEMRTable && pEMN->GetID() != -1 && pEMN->GetMedicationCount() > 0) {
			//compare directly to the database, not what is loaded in memory,
			//in part because the patient meds. is also looking directly at data,
			//and in part because we need both the MedID and EMRDataID, and we do
			//not track the DataID in memory
			_RecordsetPtr prsEMNRx = CreateParamRecordset("SELECT DrugList.ID, DrugList.EMRDataID, EMRDataT.Data "
				"FROM EMRMedicationsT "
				"INNER JOIN PatientMedications ON EMRMedicationsT.MedicationID = PatientMedications.ID "
				"INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
				"INNER JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
				"WHERE EMRMedicationsT.Deleted = 0 AND PatientMedications.Deleted = 0 "
				"AND EMRMedicationsT.EMRID = {INT}", pEMN->GetID());
			while(!prsEMNRx->eof) {
				long nMedID = VarLong(prsEMNRx->Fields->Item["ID"]->Value);
				long nEMRDataID = VarLong(prsEMNRx->Fields->Item["EMRDataID"]->Value);
				CString strMedName = VarString(prsEMNRx->Fields->Item["Data"]->Value);
				
				//remove this drug from all four lists
				int iFoundMedID = -1;
				for(int i=0;i<anEMNCurMedIDs.GetSize() && iFoundMedID == -1; i++) {
					if(anEMNCurMedIDs.GetAt(i) == nMedID) {
						anEMNCurMedIDs.RemoveAt(i);
						iFoundMedID = i;
					}
				}
				int iFoundDataID = -1;
				for(i=0;i<anEMNCurMedDataIDs.GetSize() && iFoundDataID == -1; i++) {
					if(anEMNCurMedDataIDs.GetAt(i) == nEMRDataID) {
						anEMNCurMedDataIDs.RemoveAt(i);
						mapEMNCurMedDataIDsToSig.RemoveKey(nEMRDataID);
						iFoundDataID = i;
					}
				}
				int iFoundName = -1;
				for(i=0;i<astrDetailDrugListNames.GetSize() && iFoundName == -1; i++) {
					if(astrDetailDrugListNames.GetAt(i) == strMedName) {
						astrDetailDrugListNames.RemoveAt(i);
						iFoundName = i;
					}
				}

				//these three indexes should have been the same
				if(iFoundMedID != -1) {
					ASSERT((iFoundMedID == iFoundDataID) && (iFoundDataID == iFoundName));
				}

				iFoundMedID = -1;
				for(int i=0;i<anPtCurMedIDs.GetSize() && iFoundMedID == -1; i++) {
					if(anPtCurMedIDs.GetAt(i) == nMedID) {
						anPtCurMedIDs.RemoveAt(i);
						iFoundMedID = i;
					}
				}
				iFoundDataID = -1;
				for(i=0;i<anPtCurMedDataIDs.GetSize() && iFoundDataID == -1; i++) {
					if(anPtCurMedDataIDs.GetAt(i) == nEMRDataID) {
						anPtCurMedDataIDs.RemoveAt(i);
						mapPtCurMedDataIDsToSig.RemoveKey(nEMRDataID);
						iFoundDataID = i;
					}
				}
				iFoundName = -1;
				for(i=0;i<astrPtDrugListNames.GetSize() && iFoundName == -1; i++) {
					if(astrPtDrugListNames.GetAt(i) == strMedName) {
						astrPtDrugListNames.RemoveAt(i);
						iFoundName = i;
					}
				}

				//these three indexes should have been the same
				if(iFoundMedID != -1) {
					ASSERT((iFoundMedID == iFoundDataID) && (iFoundDataID == iFoundName));
				}

				prsEMNRx->MoveNext();
			}
			prsEMNRx->Close();
		}

		//
		// (c.haag 2007-01-25 10:37) - PLID 24396 - Compare the arrays. If anything is
		// different in either one, then we need to do something about the patient meds
		//
		BOOL bNeedToUpdatePtMeds = (AreArrayContentsMatched(anEMNCurMedIDs, anPtCurMedIDs)) ? FALSE : TRUE;

		// (j.jones 2012-10-29 09:18) - PLID 53324 - Track that we tried to save medications, and could have updated
		// the patient account. We may change nothing, and may not actually change the account. But track that we
		// tried, such that we can prompt later to confirm that a blank medication list was confirmed to be intentional.
		m_bHadSavedCurrentMeds = TRUE;

		// (j.jones 2011-05-06 10:28) - PLID 43527 - if the meds are matched, but the Sig is different,
		// we still need to update (only if we have the Sig column)
		if(!bNeedToUpdatePtMeds && ptcSig != NULL) {
			for(int i=0; i < anEMNCurMedDataIDs.GetSize() && !bNeedToUpdatePtMeds; i++) {
				long nDataID = (long)anEMNCurMedDataIDs.GetAt(i);
				CString strEMNSig = "";
				if(mapEMNCurMedDataIDsToSig.Lookup(nDataID, strEMNSig)) {
					//only if we have a result (could be blank) should we
					//try to compare further
					CString strPatSig = "";
					if(mapPtCurMedDataIDsToSig.Lookup(nDataID, strPatSig)) {
						//only if we have a result (could be blank) should we
						//try to compare further
						if(strEMNSig != strPatSig) {
							//the Sigs are different (case sensitive), so we must change it
							bNeedToUpdatePtMeds = TRUE;
						}
					}
				}
			}
		}

		//
		// At last, we finally know whether the patient medications in EMR differ
		// from those for the patient. If they are the same, there is nothing more
		// to do.
		//
		if (!bNeedToUpdatePtMeds)
			return;

		//
		// If we get here, then we need to do something about the patient meds. Give
		// the user a list of all the EMN medications, and a list of the patient medications,
		// and let them decide whether to automatically update the patient medications or not.

		// (j.jones 2007-08-24 16:11) - PLID 27012 - We decided that this is NOT optional.
		// Simply tell them we WILL be updating the current medications, and make it a
		// DontShowMeAgain box so they don't always have to see this information.
		// (j.jones 2009-08-27 10:45) - PLID 35366 - this is now a non-optional, non-hideable messagebox

		CString strEMNName = pEMN->GetDescription();
		if (strEMNName.IsEmpty()) strEMNName = "<no name>";
		CString strMsg = 
			"The Current Medications selected in the most recently modified EMN differ from those in the patient's Medications tab:\n\n"
			"Medications in the EMN \"" + strEMNName + "\":\n\n";
		if (0 == astrDetailDrugListNames.GetSize()) {
			strMsg += "<none>\n";
		} else {
			for (int i=0; i < astrDetailDrugListNames.GetSize(); i++) {
				if (i > 8) {
					strMsg += "<more...>\n";
					break;
				} else {
					// (j.jones 2011-05-06 10:32) - PLID 43527, show the Sig, if we have one that is non-empty
					CString strSig;
					mapEMNCurMedDataIDsToSig.Lookup((long)anEMNCurMedDataIDs.GetAt(i), strSig);
					if(!strSig.IsEmpty() && ptcSig != NULL) {
						strMsg += (astrDetailDrugListNames[i] + ": " + strSig + "\n");
					}
					else {
						strMsg += astrDetailDrugListNames[i] + "\n";
					}
				}
			}
		}

		strMsg += "\nMedications selected from the Patient Medications tab:\n\n";
		if (0 == astrPtDrugListNames.GetSize()) {
			strMsg += "<none>\n";
		} else {
			for (int i=0; i < astrPtDrugListNames.GetSize(); i++) {
				if (i > 8) {
					strMsg += "<more...>\n";
					break;
				} else {
					// (j.jones 2011-05-06 10:32) - PLID 43527, show the Sig, if we have one that is non-empty
					CString strSig;
					mapPtCurMedDataIDsToSig.Lookup((long)anPtCurMedDataIDs.GetAt(i), strSig);
					if(!strSig.IsEmpty() && ptcSig != NULL) {
						strMsg += (astrPtDrugListNames[i] + ": " + strSig + "\n");
					}
					else {
						strMsg += astrPtDrugListNames[i] + "\n";
					}
				}
			}
		}
		strMsg += "\n\nPractice will update the medication list in the Patient Medications tab with those in this EMR.";

		if (pProgressDlg) pProgressDlg->ShowProgress(SW_HIDE);
		// (c.haag 2007-02-14 09:42) - PLID 24396 - Don't use MsgBox because Practice will
		// try to format medication names with % signs in them
		// (j.jones 2007-08-24 17:01) - PLID 27012 - changed into a DontShowMeAgain box,
		// and converted to Control Text to not have & accelerators
		// (a.walling 2009-03-09 09:48) - PLID 33403 - This can cause a hang in the CFrameWnd::NotifyFloatingWindows GetWindow loop
		// in some rare occasions; we can avoid this by explicitly setting a parent window. If the interface window is NULL,
		// then it behaves as it did previously (but this is rare enough that it is unlikely to cause a problem)
		// (j.jones 2009-08-27 10:45) - PLID 35366 - this is now a non-optional, non-hideable messagebox
		MessageBox(GetActiveWindow(), strMsg, "Updating Patient Medications", MB_ICONINFORMATION|MB_OK);

		// (a.walling 2007-09-07 09:41) - PLID 24371 - Show but do not activate
		if (pProgressDlg) pProgressDlg->ShowProgress(SW_SHOWNA);

		//
		// If we get here, it's time to update the patient medications list. Do it one by one.
		// There is no auditing for CurrentPatientMedsT, hence, no audit transaction ID used here
		//
		long nAuditTransactionID = -1;		
		try {
			// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
			CSqlTransaction trans("Synchronizing Patient Medications With EMR");
			trans.Begin();

			// (c.haag 2007-10-22 08:55) - PLID 27827 - Audit all changes
			// (c.haag 2011-01-25) - PLID 42222 - Pass in a map that maps current medication data ID's
			// to NewCrop GUID's.
			long nPatientID = GetPatientID();
			// (j.jones 2012-10-17 13:46) - PLID 51713 - Added bCurHasNoMedsStatus, the current value of PatientsT.HasNoMeds.
			// This function will clear that status if it adds a current medication.
			// It will also set bCurHasNoMedsStatus to FALSE if it changes the value from TRUE to FALSE.
			CString strSql = GenerateCurrentMedsPropagationSaveString(anEMNCurMedDataIDs, mapEMNCurMedDataIDsToSig,
				anPtCurMedDataIDs, mapPtCurMedDataIDsToSig, nPatientID, nAuditTransactionID, m_mapNewEMRDataIDToNewCropGUID, bCurHasNoMedsStatus);

			// (c.haag 2007-10-22 08:57) - PLID 27827 - Make sure the query is not empty when
			// we try to run it
			if (!strSql.IsEmpty()) {
				// (j.jones 2010-04-20 09:50) - PLID 30852 - converted to a proper batch execute
				ExecuteSqlBatch(strSql);

				if(nAuditTransactionID != -1) {
					CommitAuditTransaction(nAuditTransactionID);
				}
				// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
				trans.Commit();

				// (j.jones 2012-10-29 09:20) - PLID 53324 - This function will check whether a blue Current
				// Medications table was on the EMN, and was saved with no meds. If so, and PatientsT.HasNoMeds
				// is false, the user will be prompted to fill this setting.
				CEmrTreeWnd *pTreeWnd = GetInterface();
				if(pTreeWnd) {
					pTreeWnd->CheckPromptPatientHasNoCurrentMeds(FALSE);
				}

				// (c.haag 2010-09-29 12:58) - PLID 40610 - Create todo alarms for decisions because
				// we may have added or updated them for the patient. There's not a guarantee that we
				// did; but if it did happen, it won't cause any problems worse than popping up a todo alarm.
				// This check is better than nothing and doesn't add a round trip to the database.
				if (-1 != strSql.Find("INSERT INTO CurrentPatientMedsT") ||
					-1 != strSql.Find("UPDATE CurrentPatientMedsT")
				) 
				{
					//TES 10/31/2013 - PLID 59251 - We now just pass any new interventions back to our caller
					UpdateDecisionRules(GetRemoteData(), nPatientID, arNewCDSInterventions);
				}

				// Now fire the table checker
				// (j.jones 2008-11-25 11:42) - PLID 32183 - make sure we send the patient ID
				CClient::RefreshTable(NetUtils::CurrentPatientMedsT, nPatientID);

				// (c.haag 2011-01-25) - PLID 42222 - We're done with the map.
				m_mapNewEMRDataIDToNewCropGUID.RemoveAll();

				// (j.jones 2012-10-03 15:47) - PLID 36220 - set the drug interaction flag to reflect
				// that something changed - in this case, current meds.
				bDrugInteractionsChanged = TRUE;

				// All done!
				return;
			}
			else {
				// (c.haag 2007-10-22 08:58) - If the query is empty, there's nothing to do
				// but undo all transactions

				// (c.haag 2011-01-25) - PLID 42222 - Ensure the map is empty.
				m_mapNewEMRDataIDToNewCropGUID.RemoveAll();
			}
		}
		NxCatchAll("Could not synchronize patient medications with EMR");
		if(nAuditTransactionID != -1) { RollbackAuditTransaction(nAuditTransactionID); }
	}
	NxCatchAll("Error in CEMR::PostSaveUpdateCurrentMedications()");
}

// (j.jones 2012-10-03 15:47) - PLID 36220 - if allergies change, we will update bDrugInteractionsChanged
void CEMR::PostSaveUpdateAllergies(CShowProgressFeedbackDlg* pProgressDlg, CEMNDetail *pCurrentAllergiesDetail, OUT BOOL &bDrugInteractionsChanged)
{
	//
	// (c.haag 2007-04-06 09:03) - PLID 25525 - Now that the EMR has been saved, we need
	// to check if the patient Allergies as defined by the most recent EMN is different
	// from the patient allergy list. If they are different, we must give the user the 
	// opportunity to change the patient allergy list to be consistent with the EMN.
	// (c.haag 2007-08-15 17:58) - PLID 25525 - Don found a far superior implementation
	// which is faster and more robust. He did the equivalent change for Current Medications
	// in PLID 26527, and I am mimicing his changes here. Simply stated: We are now given the
	// Allergies detail we would otherwise be searching for.
	// (c.haag 2007-08-15 18:04) - PLID 25525 - We now take in the official detail so we
	// don't have to query data
	//
	if (m_bIsTemplate) {
		ASSERT(FALSE); // We should never get here!
		return;
	}
	if (GetPatientID() <= 0) {
		ASSERT(FALSE); // We should never get here!
		return;
	}

	try {
		if (NULL == pCurrentAllergiesDetail) {
			// (c.haag 2007-08-15 18:00) - We should never get here
			ASSERT(FALSE);
			return;
		}

		// (c.haag 2007-08-15 18:00) - Get the EMN object
		CEMN *pEMN = pCurrentAllergiesDetail->m_pParentTopic->GetParentEMN();
		if(pEMN == NULL) {
			//This should be impossible, but we must fail if so
			ASSERT(FALSE);
			return;
		}

		//
		// Now pull the allergy ID's of the EMN by the detail we found
		//
		CArray<long,long> anEMNAllergyIDs;
		CArray<long,long> anEMNAllergyDataIDs;
		CStringArray astrDetailAllergyNames;
		// (c.haag 2007-08-18 12:34) - PLID 27114 - Populate the arrays through the detail
		pCurrentAllergiesDetail->PopulateAllergyIDArrays(anEMNAllergyIDs, anEMNAllergyDataIDs, astrDetailAllergyNames);

		//
		// Now pull the allergy ID's for the patient
		//
		CArray<long,long> anPtAllergyIDs;
		CArray<long,long> anPtAllergyDataIDs;
		CStringArray astrPtAllergyNames;
		BOOL bCurHasNoAllergiesStatus = FALSE;
		//DRT 8/28/2007 - PLID 27207 - Parameterized.
		// (c.haag 2009-06-12 10:24) - PLID 34611 - Filter out discontinued allergies
		// (j.jones 2012-10-17 09:32) - PLID 53179 - Added HasNoAllergies
		_RecordsetPtr prsPt = CreateParamRecordset("SELECT HasNoAllergies FROM PatientsT WHERE PersonID = {INT}\r\n"
			""
			"SELECT AllergyID, EMRDataID, EMRDataT.Data AS Name FROM PatientAllergyT "
			"LEFT JOIN AllergyT ON AllergyT.ID = PatientAllergyT.AllergyID "
			"LEFT JOIN EMRDataT ON AllergyT.EMRDataID = EMRDataT.ID "
			"WHERE PersonID = {INT} AND Discontinued = 0 "
			"ORDER BY Name\r\n",
			GetPatientID(), GetPatientID());
		if(!prsPt->eof) {
			bCurHasNoAllergiesStatus = VarBool(prsPt->Fields->Item["HasNoAllergies"]->Value, FALSE);
		}
		prsPt = prsPt->NextRecordset(NULL);
		while (!prsPt->eof) {
			anPtAllergyIDs.Add(AdoFldLong(prsPt, "AllergyID"));
			anPtAllergyDataIDs.Add(AdoFldLong(prsPt, "EMRDataID"));
			astrPtAllergyNames.Add(AdoFldString(prsPt, "Name"));
			prsPt->MoveNext();
		}
		prsPt->Close();

		//
		// (c.haag 2007-04-06 09:09) - PLID 25525 - Compare the arrays. If anything is
		// different in either one, then we need to do something about the patient allergies
		//
		BOOL bNeedToUpdatePtAllergies = (AreArrayContentsMatched(anEMNAllergyIDs, anPtAllergyIDs)) ? FALSE : TRUE;

		// (j.jones 2012-10-25 17:18) - PLID 53322 - Track that we tried to save allergies, and could have updated
		// the patient account. We may change nothing, and may not actually change the account. But track that we
		// tried, such that we can prompt later to confirm that a blank allergy list was confirmed to be intentional.
		m_bHadSavedAllergies = TRUE;

		//
		// At last, we finally know whether the patient allergies in EMR differ
		// from those for the patient. If they are the same, there is nothing more
		// to do.
		//
		if (!bNeedToUpdatePtAllergies)
			return;

		//
		// If we get here, then we need to do something about the patient allergies. Give the
		// user a list of all the EMN allergies and a list of the patient allergies, and let
		// them decide whether to automatically update the patient allergies or not

		// (j.jones 2007-08-24 16:11) - PLID 27012 - We decided that this is NOT optional.
		// Simply tell them we WILL be updating the patient allergies, and make it a
		// DontShowMeAgain box so they don't always have to see this information.
		// (j.jones 2009-08-27 10:45) - PLID 35366 - this is now a non-optional, non-hideable messagebox

		CString strEMNName = pEMN->GetDescription();
		if (strEMNName.IsEmpty()) strEMNName = "<no name>";
		CString strMsg = 
			"The allergies selected in the most recently modified EMN differ from those in the patient's Medications tab:\n\n"
			"Allergies in the EMN \"" + strEMNName + "\":\n\n";
		if (0 == astrDetailAllergyNames.GetSize()) {
			strMsg += "<none>\n";
		} else {
			for (int i=0; i < astrDetailAllergyNames.GetSize(); i++) {
				if (i > 8) {
					strMsg += "<more...>\n";
					break;
				} else {
					strMsg += astrDetailAllergyNames[i] + "\n";

				}
			}
		}

		strMsg += "\nAllergies selected from the Patient Medications tab:\n\n";
		if (0 == astrPtAllergyNames.GetSize()) {
			strMsg += "<none>\n";
		} else {
			for (int i=0; i < astrPtAllergyNames.GetSize(); i++) {
				if (i > 8) {
					strMsg += "<more...>\n";
					break;
				} else {
					strMsg += astrPtAllergyNames[i] + "\n";
				}
			}
		}
		strMsg += "\n\nPractice will update the allergy list in the Patient Medications tab with those in this EMR.";

		if (pProgressDlg) pProgressDlg->ShowProgress(SW_HIDE);
		
		// (j.jones 2007-08-24 17:01) - PLID 27012 - changed into a DontShowMeAgain box
		// and converted to Control Text to not have & accelerators
		// (a.walling 2009-03-09 09:48) - PLID 33403 - This can cause a hang in the CFrameWnd::NotifyFloatingWindows GetWindow loop
		// in some rare occasions; we can avoid this by explicitly setting a parent window. If the interface window is NULL,
		// then it behaves as it did previously (but this is rare enough that it is unlikely to cause a problem)
		// (j.jones 2009-08-27 10:45) - PLID 35366 - this is now a non-optional, non-hideable messagebox
		MessageBox(GetActiveWindow(), strMsg, "Updating Patient Allergies", MB_ICONINFORMATION|MB_OK);
		
		// (a.walling 2007-09-07 09:42) - PLID 24371 - Show but do not activate
		if (pProgressDlg) pProgressDlg->ShowProgress(SW_SHOWNA);
		
		//
		// If we get here, it's time to update the patient allergy list. Do it one by one.
		// There is no auditing for PatientAllergyT, hence, no audit transaction ID used here
		//
		long nAuditTransactionID = -1;
		try {
			// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
			CSqlTransaction trans("Synchronizing Patient Allergies With EMR");
			trans.Begin();
			// (c.haag 2007-10-19 13:21) - PLID 27822 - Audit all changes
			long nPatientID = GetPatientID();
			// (j.jones 2012-10-17 09:32) - PLID 53179 - Added bCurHasNoAllergiesStatus, the current value of PatientsT.HasNoAllergies.
			// This function will clear that status if it adds an allergy. It will also set bCurHasNoAllergiesStatus to FALSE if it
			// changes the value from TRUE to FALSE.
			CDWordArray aryAllergyIngredToImport, aryAllergyIngredToDelete;
			CDWordArray& aryIngredImportInOut = aryAllergyIngredToImport;
			CDWordArray& aryIngredDeleteInOut = aryAllergyIngredToDelete;
			CString strSql = GenerateAllergyPropagationSaveString(anEMNAllergyDataIDs,
				anPtAllergyDataIDs, nPatientID, nAuditTransactionID, bCurHasNoAllergiesStatus, aryIngredImportInOut, aryIngredDeleteInOut);

			// (c.haag 2007-10-19 16:05) - PLID 27822 - Make sure the query is not empty when we run it
			if (!strSql.IsEmpty()) {

				// (j.jones 2010-04-20 09:50) - PLID 30852 - converted to a proper batch execute
				ExecuteSqlBatch(strSql);

				if(nAuditTransactionID != -1) {
					CommitAuditTransaction(nAuditTransactionID);
				}
				trans.Commit();

				// (j.jones 2012-10-26 15:43) - PLID 53322 - This function will check whether a blue Allergies
				// table was on the EMN, and was saved with no allergies. If so, and PatientsT.HasNoAllergies
				// is false, the user will be prompted to fill this setting.
				CEmrTreeWnd *pTreeWnd = GetInterface();
				if(pTreeWnd) {
					pTreeWnd->CheckPromptPatientHasNoAllergies(FALSE);
				}

				// (j.luckoski 2012-11-20 10:31) - PLID 53825 - If the arrays are not empty, call their corresponding functions
				if(aryAllergyIngredToImport.GetSize() > 0) {
					ImportAllergyIngredientsFromFDB(aryAllergyIngredToImport);
				}
				if (aryAllergyIngredToDelete.GetSize() > 0) {
					DeleteAllergyIngredientsFromFDB(aryAllergyIngredToDelete, nPatientID);
				}

				// (j.jones 2008-11-25 14:32) - PLID 28508 - If we changed something, the allergy review
				// status will have been cleared. Check and see if this user has permission to mark the allergy
				// list as reviewed, and if so, ask if they want to mark it reviewed now.
				BOOL bHasAllergyReviewPerms = GetCurrentUserPermissions(bioPatientAllergyReview) & SPT___W________ANDPASS;
				if(bHasAllergyReviewPerms && MessageBox(GetActiveWindow(), "This patient's allergy information has changed.\n"
					"Do you wish to certify that you have reviewed the allergy list with the patient?\n\n"
					"If not, you can always review the allergy list later in their Medications Tab.", "Practice", MB_ICONQUESTION|MB_YESNO) == IDYES) {
					//now check the permission, it may need a password
					if(CheckCurrentUserPermissions(bioPatientAllergyReview, sptWrite)) {
						 UpdatePatientAllergyReviewStatus(nPatientID, TRUE);
					}
				}

				// Now fire the table checker
				// (j.jones 2008-11-25 11:42) - PLID 32183 - make sure we send the patient ID
				CClient::RefreshTable(NetUtils::PatientAllergyT, nPatientID);

				// (j.jones 2012-10-03 15:47) - PLID 36220 - set the drug interaction flag to reflect
				// that something changed - in this case, allergies
				bDrugInteractionsChanged = TRUE;

				// All done!
				return;
			}
			else {
				// (c.haag 2007-10-19 16:06) - PLID 27822 - If the query was empty, there's nothing
				// to commit, so leave the try/catch and commit the rollbacks
			}
		}
		NxCatchAll("Could not synchronize patient allergies with EMR");
		if(nAuditTransactionID != -1) { RollbackAuditTransaction(nAuditTransactionID); }
	}
	NxCatchAll("Error in CEMR::PostSaveUpdateAllergies()");

}

// (c.haag 2012-10-17) - PLID 52863 - This must be called after PropagateNewID for all ID's so that we can assign ID values to EMRTodosT.
void CEMR::UpdateUnsavedTodos()
{
	for (int i = 0; i < m_arypEMNs.GetSize(); i++) {
		m_arypEMNs[i]->UpdateUnsavedTodos();
	}
}

// (z.manning 2010-03-11 14:56) - PLID 37571 - Will reassasign the source detail stamp pointer to the given
// new pointer for any object in this EMN that could have potentially been spawned.
// (a.walling 2010-03-31 14:46) - PLID 38009 - Pass in the source EMN; if we are not the source EMN, we do not need to recurse into topics
void CEMR::UpdateSourceDetailStampPointers(EmrDetailImageStamp *pDetailStampOld, EmrDetailImageStamp *pDetailStampNew, CEMN* pSourceEMN)
{
	for (int i = 0; i < m_arypEMNs.GetSize(); i++) {
		m_arypEMNs[i]->UpdateSourceDetailStampPointers(pDetailStampOld, pDetailStampNew, pSourceEMN);
	}
}

// (c.haag 2010-08-05 09:39) - PLID 39984 - This function will go through every EMN in arEMNs and see if 
// any of their source detail pointers are pointing to a detail in another EMR. Each one that is found will be 
// updated to its local counterpart.
void CEMR::UpdateEMNSourceDetailsFromCopy(const CArray<CEMN*,CEMN*>& arEMNs)
{
	for (int i = 0; i < arEMNs.GetSize(); i++) 
	{
		CEMN* pEMN = arEMNs[i];
		if (NULL != pEMN) {
			SourceActionInfo sai = pEMN->GetSourceActionInfo();
			if(NULL != sai.pSourceDetail) 
			{
				//we have a pointer but no detail ID, so try to find the parent detail
				BOOL bFound = FALSE;
				for (int k=0; k < GetEMNCount() && !bFound; k++) {
					CEMN* pEMNToTest = GetEMN(k);
					if (NULL != pEMNToTest && pEMN != pEMNToTest) {
						long nDetails = pEMNToTest->GetTotalDetailCount();
						for(int j=0; j<nDetails && !bFound; j++) {
							//we have to compare to every detail in the EMN
							CEMNDetail *pDetailToCompare = pEMNToTest->GetDetail(j);
							if(pDetailToCompare) {
								//and we are temporarily storing a pointer to the copied-from detail,
								//which is what our sourcedetail currently links to
								CEMNDetail *pCopiedFromDetail = pDetailToCompare->GetCopiedFromDetail();
								if(pCopiedFromDetail == sai.pSourceDetail) {
									//good, we found our source detail, now reassign our pointer
									sai.pSourceDetail = pDetailToCompare;
									//ensure our source detail ID is -1 now
									sai.nSourceDetailID = -1;
									pEMN->SetSourceActionInfo(sai);
									bFound = TRUE;
								}
							}
							else {
								// No detail to compare with
							}
						} // for(int j=0; j<nDetails && !bFound; j++) {
					}
					else {
						// The EMN is not valid for testing
					}
				} // for (int k=0; k < GetEMNCount() && !bFound; k++) {

				if (!bFound) 
				{
					// If none was found, then we have an unexpected condition. Leave the pointer
					// alone and defer to legacy behavior.
				}
			}
			else {
				// The EMN Source detail is NULL; nothing to do here
			}
		}
		else {
			// The EMN is NULL; nothing to do here
		}
	} // for (int i = 0; i < m_arypEMNs.GetSize(); i++) 
}

//DRT 7/13/2007 - PLID 26671 - Get the full procedure structure, not just the ID
void CEMR::GetProcedures(CArray<EMNProcedure, EMNProcedure> &arProcedureIDs)
{
	CArray<EMNProcedure, EMNProcedure> arEmnProcs;
	for(int i = 0; i < GetEMNCount(); i++) {
		arEmnProcs.RemoveAll();
		GetEMN(i)->GetProcedures(arEmnProcs);
		for(int j = 0; j < arEmnProcs.GetSize(); j++) {
			bool bFound = false;
			for(int k = 0; k < arProcedureIDs.GetSize() && !bFound; k++) {
				//Compare on ID to avoid adding duplicate procedures
				if(arEmnProcs[j].nID == arProcedureIDs[k].nID) bFound = true;
			}
			if(!bFound) {
				arProcedureIDs.Add(arEmnProcs[j]);
			}
		}
	}
}

void CEMR::SetDescription(const CString& strDescription)
{
	//if (m_strDescription != strDescription.Left(255))
	//	m_bIsUnsaved = TRUE;
	if (0 != strncmp(m_strDescription, strDescription, 255)) {
		m_bIsUnsaved = TRUE;
	}
	m_strDescription = strDescription;
	if (m_strDescription.GetLength() > 255) {
		m_strDescription.Truncate(255);
	}
}

// (c.haag 2008-08-05 11:37) - PLID 30799 - Added bIsUnspawning. We don't allow manual removal
// of EMN's with problems if the user doesn't have permission to do so. However, if the EMN is
// removed via unspawning, then we don't do the permissions check.
BOOL CEMR::RemoveEMN(CEMN *pEMN, BOOL bIsEMNUnspawning /* = FALSE */)
{
	if(pEMN->GetStatus() == 2) {
		//This EMN is locked!
		return FALSE;
	}
	// (c.haag 2006-07-10 16:47) - PLID 19977 - We can't remove EMN's if they have details
	// marked as problems
	// (c.haag 2008-07-24 09:38) - PLID 30826 - New way of determining whether the user can
	// delete the EMN by virtue of existing EMR problems
	else if (!bIsEMNUnspawning && !CanCurrentUserDeleteEmrProblems() && pEMN->DoesEmnOrChildrenHaveSavedProblems()) {
		// This EMN has one or more problems!
		// (c.haag 2009-09-09 11:37) - PLID 31209 - Tell the user they can't manually remove the EMN
		MsgBox(MB_OK | MB_ICONSTOP, "You may not remove this EMN because it is associated with one or more saved problems.");
		return FALSE;
	} else if (!pEMN->IsWritable()) {
		// (a.walling 2008-06-25 16:03) - PLID 30515 - EMN is not writable! Attempt to acquire write token once.
		CWriteTokenInfo wtInfo;
		if (!pEMN->RequestWriteToken(wtInfo, FALSE)) {
			return FALSE;
		}
	}

	for(int i = m_arypEMNs.GetSize() - 1; i >= 0 ; i--) {
		if(m_arypEMNs[i] == pEMN) {
			m_arypEMNs.RemoveAt(i);
			m_aryDeletedEMNs.Add(pEMN);
			return TRUE;
		}
	}

	return FALSE;
}

//DRT 3/6/2006 - PLID 19571 - To make deleting a little easier from the outside, 
//	allow us to do a deletion by ID.  If the ID given doesn't exist, this function
//	will do nothing.
BOOL CEMR::RemoveEMNByID(long nEMNID)
{
	for(int i = 0; i < m_arypEMNs.GetSize(); i++) {
		CEMN* pEMN = m_arypEMNs.GetAt(i);
		if(pEMN->GetID() == nEMNID) {
			return RemoveEMN(pEMN);
		}
	}

	return FALSE;
}

void CEMR::ProcessEMRInfoActions(CEMNDetail *pSourceDetail, BOOL bIsInitialLoad, CEMNLoader* pLoader /* = NULL */)
{
	// (c.haag 2007-04-25 11:59) - PLID 25774 - Added support for pulling the action
	// list from a CEMNLoader object. If pLoader is not null, then we'll draw the action
	// list from there. Otherwise, we will allocate a tempoary action list and have
	// LoadActionInfo populate it
	try {

		// (j.jones 2007-07-18 13:21) - PLID 26730 - check whether or not the info item has Info actions,
		// which it usually does not, such that we don't have to search for them
		if(pSourceDetail->GetHasInfoActionsStatus() == ehiasHasNoInfoItems) {
			return;
		}

		//if undetermined, ASSERT, because there is no reason it should not be known
		if(pSourceDetail->GetHasInfoActionsStatus() == ehiasUndetermined) {
			ASSERT(FALSE);
		}

		//Get the actions to process.
		MFCArray<EmrAction>* parActions = NULL;
		// (c.haag 2007-07-03 10:49) - PLID 26523 - We always allocate actions now
		if (NULL == (parActions = new MFCArray<EmrAction>)) {
			ThrowNxException("Failed to allocate action array!");
		}
		if (NULL != pLoader) {
			// (c.haag 2007-07-03 10:46) - PLID 26523 - Temporarily take exclusive ownership
			// of EMR info objects
			CHoldEMNLoaderMutex mh(pLoader->GetEMRInfoMutex());
			MFCArray<EmrAction>* parLoaderActions = pLoader->GetEMRInfoActions(pSourceDetail);
			if (NULL != parLoaderActions) {
				for (int i=0; i < parLoaderActions->GetSize(); i++) {
					parActions->Add( parLoaderActions->GetAt(i) );
				}
			}
		} else {
			LoadActionInfo(CSqlFragment("SourceType = {INT} AND SourceID = {INT} AND Deleted = 0", eaoEmrItem, pSourceDetail->m_nEMRInfoID), *parActions);
		}

		ProcessEmrActions(*parActions, pSourceDetail, NULL, bIsInitialLoad);
		delete parActions;

	}NxCatchAll("Error in CEMR::ProcessEmrInfoActions()");
}

void CEMR::ProcessEMRDataActions(long nEMRDataID, CEMNDetail *pSourceDetail, BOOL bIsInitialLoad, CEMNLoader* pLoader /* = NULL */)
{
	// (j.jones 2013-01-09 16:44) - PLID 54541 - this now just calls the function for multiple, for just one detail
	CArray<long, long> aryEMRDataIDs;
	aryEMRDataIDs.Add(nEMRDataID);
	ProcessEMRDataActions(aryEMRDataIDs, pSourceDetail, bIsInitialLoad, pLoader);
}

// (j.jones 2013-01-09 16:44) - PLID 54541 - added an ability to process actions for more than one EMRDataID at a time,
// provided they are from the same detail (such as committing a popped up multi-select list)
void CEMR::ProcessEMRDataActions(CArray<long, long> &aryEMRDataIDs, CEMNDetail *pSourceDetail, BOOL bIsInitialLoad, CEMNLoader* pLoader /* = NULL */)
{
	// (j.jones 2013-01-09 16:44) - PLID 54541 - This function is mostly the original code for processing actions for one detail,
	// I just modified the content to support multiple.

	if(aryEMRDataIDs.GetSize() == 0) {
		//no data to load
		return;
	}

	// (c.haag 2007-04-25 12:04) - PLID 25774 - Added support for pulling the action
	// list from a CEMNLoader object. If pLoader is not null, then we'll draw the action
	// list from there. Otherwise, we will allocate a tempoary action list and have
	// LoadActionInfo populate it
	try {
		//Get the actions to process.
		MFCArray<EmrAction>* parActions = NULL;
		// (c.haag 2007-07-03 10:58) - PLID 26523 - We always allocate now
		if (NULL == (parActions = new MFCArray<EmrAction>)) {
			ThrowNxException("Failed to allocate action array!");
		}
		if (NULL != pLoader) {
			// (c.haag 2007-07-03 10:58) - PLID 26523 - Temporarily take exclusive ownership
			// of EMR data actions
			CHoldEMNLoaderMutex mh(pLoader->GetEMRDataActionsMutex());
			// (j.jones 2013-01-09 16:49) - PLID 54541 - this function now fills parActions for all requested data IDs
			pLoader->GetEMRDataActions(aryEMRDataIDs, parActions);
		} else {
			// (j.jones 2013-01-09 16:43) - PLID 54541 - this now supports multiple data IDs
			LoadActionInfo(CSqlFragment("SourceType = {INT} AND SourceID IN ({INTARRAY}) AND Deleted = 0", eaoEmrDataItem, aryEMRDataIDs), *parActions);
		}

		// (c.haag 2008-06-17 10:21) - PLID 17842 - We now pass in "result" which will be populated with a list of emr items and topics
		// that are spawned. We will use these results to look for table details with table elements that have missing linked details
		CProcessEmrActionsResult result;
		//TES 2/15/2010 - PLID 37375 - Added a parameter for the source HotSpot, NULL in this case since this is a data action.
		ProcessEmrActions(*parActions, pSourceDetail, NULL, bIsInitialLoad, NULL, NULL, (GetIsTemplate()) ? NULL : &result);
		delete parActions;

		// Now that the spawning is done, go through the spawn results and gather all the details into a single array
		if (!GetIsTemplate()) {
			CArray<CEMNDetail*,CEMNDetail*> apTopicDetails;
			const int nTopics = result.m_apNewTopics.GetSize();
			int i,j;
			for (i=0; i < nTopics; i++) {
				CEMRTopic* pTopic = result.m_apNewTopics[i];
				if (NULL != pTopic) {
					// Don't use CEMRTopic::GenerateEMNDetailArray because it also includes subtopics; but the
					// subtopics would already be in the topic array
					const int nDetails = pTopic->GetEMNDetailCount();
					for (j=0; j < nDetails; j++) {
						apTopicDetails.Add(pTopic->GetDetailByIndex(j));
					}
				}
			} // for (i=0; i < nTopics; i++) {

			// Now find and try to fix missing linked details
			DetectAndTryRepairMissingLinkedDetails(result.m_apNewDetails, FALSE);
			DetectAndTryRepairMissingLinkedDetails(apTopicDetails, TRUE);

		} // if (!GetIsTemplate()) {


	}NxCatchAll("Error in CEMR::ProcessEmrDataActions()");
}

// (z.manning, 01/22/2008) - PLID 28690 - Process hot spot actions
//TES 2/15/2010 - PLID 37375 - Changed to take a CEMRHotSpot, rather than an ID.
// (z.manning 2011-07-25 12:15) - PLID 44649 - Changed the hot spot param to a pointer
void CEMR::ProcessEmrImageHotSpotActions(CEMRHotSpot *pSpot, CEMNDetail *pSourceDetail, BOOL bIsInitialLoad, CEMNLoader* pLoader /* = NULL */)
{
	MFCArray<EmrAction> *paryActions = new MFCArray<EmrAction>;
	if(paryActions == NULL) {
		ThrowNxException("CEMR::ProcessEmrImageHotSpotActions - Failed to allocate action array");
	}

	if(pLoader != NULL) {
		//DRT 1/24/2008 - PLID 28690 - Pull all the hotspot actions out of the loader.
		CHoldEMNLoaderMutex mh(pLoader->GetImageHotSpotActionsMutex());
		MFCArray<EmrAction> *paryHotSpotActions = pLoader->GetEMRImageHotSpotActions(pSourceDetail, pSpot->GetID());
		if(paryHotSpotActions != NULL) {
			//copy to this array
			for(int i = 0; i < paryHotSpotActions->GetSize(); i++) {
				paryActions->Add(paryHotSpotActions->GetAt(i));
			}
		}
	}
	else {
		LoadActionInfo(CSqlFragment("SourceType = {INT} AND SourceID = {INT} AND Deleted = 0", eaoEmrImageHotSpot, pSpot->GetID()), *paryActions);
	}

	//TES 2/15/2010 - PLID 37375 - Pass in the source CEMRHotSpot
	ProcessEmrActions(*paryActions, pSourceDetail, NULL, bIsInitialLoad, pSpot);
	delete paryActions;
	paryActions = NULL;
}

// (r.gonet 08/03/2012) - PLID 51949 - Perform the actions associated with Wound Care Coding conditions being met.
// - aryWoundCareConditions is an array of all the conditions that have been met during Wound Care Coding Calculation
// - pSourceEmn is the EMN that owns the table being used as the Wound Care Coding Calculator
// - pSourceDetail is the table being used as the Wound Care Coding Calculator
void CEMR::ProcessEmrWoundCalculatorActions(CArray<CWoundCareMetCondition,CWoundCareMetCondition> &aryWoundCareConditions, CEMN* pSourceEmn, CEMNDetail *pSourceDetail, BOOL bIsInitialLoad, CEMNLoader *pLoader /* = NULL */)
{
	if(aryWoundCareConditions.GetSize() == 0) {
		// (r.gonet 08/03/2012) - PLID 51949 - No conditions were met, so perform no actions.
		return;
	}

	// (r.gonet 08/03/2012) - PLID 51949 - Convert the array of conditions to an array of longs representing the condition ids.
	//  Necessary to pass into loading SQL.
	CArray<long,long> arynWoundCareConditionIds;
	for(int nConditionIndex = 0; nConditionIndex < aryWoundCareConditions.GetSize(); nConditionIndex++) {
		arynWoundCareConditionIds.Add((long)aryWoundCareConditions[nConditionIndex].m_wcccCondition);
	}

	// (r.gonet 08/03/2012) - PLID 51949 - Load the non-deleted Wound Care actions that are for these conditions.
	MFCArray<EmrAction> arActions;
	LoadActionInfo(CSqlFragment("SourceType = {INT} AND SourceID IN ({INTARRAY}) AND Deleted = 0", eaoWoundCareCodingCondition, arynWoundCareConditionIds), arActions);
	
	// (r.gonet 08/03/2012) - PLID 51949 - Some of our conditions may have been met multiple times.
	//  We need to perform the associated actions each time the condition is met.
	for(int i = 0; i < arActions.GetSize(); i++) {
		long nTimes = 0;
		for(int j = 0; j < aryWoundCareConditions.GetSize(); j++) {
			if(arActions[i].nSourceID == (long)aryWoundCareConditions[j].m_wcccCondition) {
				nTimes = aryWoundCareConditions[j].m_nTimesMet;
			} else {
				// This action is for another wound condition
			}
		}

		if(nTimes > 0) {
			arActions[i].dblDefaultQuantity *= nTimes;
		} else {
			// This shouldn't be possible
			ThrowNxException("%s : Unexpected quantity of 0 when performing Wound Care Coding actions.", __FUNCTION__);
		}
	}

	// (r.gonet 08/03/2012) - PLID 51949 - Process the actions.
	ProcessEmrActions(arActions, pSourceDetail, NULL, bIsInitialLoad, NULL, pSourceEmn);
}

// (z.manning 2009-02-16 12:06) - PLID 33072 - Process table dropdown item actions
void CEMR::ProcessEmrTableDropdownItemActions(CArray<long,long> &arynDropdownIDs, CEMNDetail *pSourceDetail, TableElement SourceTableElement, BOOL bIsInitialLoad, CEMNLoader* pLoader /* = NULL */)
{
	if(arynDropdownIDs.GetSize() <= 0) {
		return;
	}

	for(int nDropdownIndex = 0; nDropdownIndex < arynDropdownIDs.GetSize(); nDropdownIndex++)
	{
		long nDropdownID = arynDropdownIDs.GetAt(nDropdownIndex);
		if(nDropdownID > 0) {
			long nRowDataGroupID = -1;
			TableRow tr;
			if(SourceTableElement.m_pRow != NULL) {
				tr = *SourceTableElement.m_pRow;
			}
			else {
				ASSERT(FALSE);
			}
			TableSpawnInfo tdsi(nDropdownID, tr, -1);
			ProcessEmrTableDropdownItemActions(tdsi, pSourceDetail, bIsInitialLoad, pLoader);
		}
	}
}

// (z.manning 2009-02-16 12:06) - PLID 33072 - Process table dropdown item actions
void CEMR::ProcessEmrTableDropdownItemActions(TableSpawnInfo tdsi, CEMNDetail *pSourceDetail, BOOL bIsInitialLoad, CEMNLoader *pLoader /* = NULL */)
{
	// (z.manning 2009-03-04 11:21) - PLID 33072 - This should never be called with dropdown IDs that
	// are less then or equal to 0 because that means that either nothing is selected for the dropdown
	// or some weird non-data based value is (e.g. formula calculation). It wont' cause any problems
	// but it will likely lead to unnecessary data accesses.
	ASSERT(tdsi.nDropdownID > 0);

	MFCArray<EmrAction> *paryActions = new MFCArray<EmrAction>;
	if(paryActions == NULL) {
		ThrowNxException("CEMR::ProcessEmrTableDropdownItemActions - Failed to allocate action array");
	}

	if(pLoader != NULL) {
		CHoldEMNLoaderMutex mh(pLoader->GetTableDropdownItemActionsMutex());
		long nDropdownID = tdsi.nDropdownID;
		MFCArray<EmrAction> *paryTableDropdownItemActions = pLoader->GetEMRTableDropdownItemActions(pSourceDetail, nDropdownID);
		if(paryTableDropdownItemActions != NULL) {
			for(int nActionIndex = 0; nActionIndex < paryTableDropdownItemActions->GetSize(); nActionIndex++) {
				paryActions->Add(paryTableDropdownItemActions->GetAt(nActionIndex));
			}
		}
	}
	else {
		LoadActionInfo(CSqlFragment("SourceType = {INT} AND SourceID = {INT} AND Deleted = 0", eaoEmrTableDropDownItem, tdsi.nDropdownID), *paryActions);
	}

	ProcessEmrActions(*paryActions, pSourceDetail, &tdsi.tr, bIsInitialLoad);
	delete paryActions;
	paryActions = NULL;
}

// (z.manning 2010-03-01 15:06) - PLID 37571 - Smart stamp spawning
void CEMR::ProcessEmrSmartStampImageActions(TableRow *pSourceTableRow, CEMNDetail *pSourceDetail, BOOL bIsInitialLoad, CEMNLoader *pLoader /* = NULL */)
{
	MFCArray<EmrAction> *paryActions = new MFCArray<EmrAction>;
	if(paryActions == NULL) {
		ThrowNxException("CEMR::ProcessEmrSmartStampImageActions - Failed to allocate action array");
	}

	// (c.haag 2012-10-26) - PLID 53440 - Use the new getter functions
	long nStampID = pSourceTableRow->m_ID.GetImageStampID();
	GetMainFrame()->GetEmrActionsByStampID(nStampID, FALSE, *paryActions);

	// (z.manning 2011-06-29 11:33) - PLID 37959 - We now support per-anatomic location actions for stamp based actions.
	// Call this function to remove any that do not apply.
	RemoveActionsBasedOnAnatomicLocation(paryActions, pSourceDetail, pSourceTableRow->m_ID.GetDetailImageStampObject());

	ProcessEmrActions(*paryActions, pSourceDetail, pSourceTableRow, bIsInitialLoad);
	delete paryActions;
	paryActions = NULL;
}

// (z.manning, 01/30/2007) - PLID 24468 - Load the actions for the given procedure and then process them.
void CEMR::ProcessEmrProcedureActions(CArray<long,long> &aryProcedureIDs, CEMN* pSourceEmn, BOOL bIsInitialLoad)
{
	try {

		if(aryProcedureIDs.GetSize() <= 0) {
			return;
		}

		MFCArray<EmrAction> arActions;
		LoadActionInfo(CSqlFragment("SourceType = {INT} AND SourceID IN ({INTARRAY}) AND Deleted = 0", eaoProcedure, aryProcedureIDs), arActions);

		//TES 2/15/2010 - PLID 37375 - Added parameter for the source HotSpot, obviously it's null in this case.
		ProcessEmrActions(arActions, NULL, NULL, bIsInitialLoad, NULL, pSourceEmn);

	}NxCatchAll("CEMN::ProcessEmrProcedureActions");
}

// (z.manning, 01/30/2007) - PLID 24468
void CEMR::ProcessEmrProcedureActions(CArray<EMNProcedure*,EMNProcedure*> &aryProcedures, CEMN* pSourceEmn, BOOL bIsInitialLoad)
{
	CArray<long,long> aryProcedureIDs;
	for(int i = 0; i < aryProcedures.GetSize(); i++) {
		aryProcedureIDs.Add(aryProcedures.GetAt(i)->nID);
	}

	ProcessEmrProcedureActions(aryProcedureIDs, pSourceEmn, bIsInitialLoad);
}

// (z.manning, 01/31/2007) - Added an option parameter for the source EMN since not all actions (e.g. procedure
// based actions) have a source detail.
// (c.haag 2008-06-17 10:12) - PLID 17842 - Added an optional parameter to allow the caller to track what was spawned
//TES 2/15/2010 - PLID 37375 - Added optional parameter for the source HotSpot
// (z.manning 2010-02-24 17:16) - PLID 37532 - Replaced nSourceDataGroupID with ptrSourceTableRow
void CEMR::ProcessEmrActions(MFCArray<EmrAction> &arActions, CEMNDetail *pSourceDetail, TableRow *ptrSourceTableRow, BOOL bIsInitialLoad, CEMRHotSpot *pSourceSpot /*= NULL*/, CEMN* pSourceEmn /* = NULL */,
							 CProcessEmrActionsResult* pEmrActionsResult /*= NULL*/)
{
	try {
		// (a.walling 2007-10-18 14:16) - PLID 25548 - Increase the depth
		IncreaseEmrActionsDepth();

		// (j.jones 2013-01-09 11:29) - PLID 45446 - if the actions result is null, create one
		if(pEmrActionsResult == NULL) {
			CProcessEmrActionsResult result;
			pEmrActionsResult = &result;
		}

		//If we are ignoring actions, or don't have any to process, do nothing.
		if(m_bIgnoreActions || !arActions.GetSize()) {
			// (a.walling 2007-10-18 14:16) - PLID 25548 - Decrease the depth
			DecreaseEmrActionsDepth();
			return;
		}

		//If spawning is locked, we still want to process the actions, but we don't need to go through all the progress
		//bar nonsense.
		if(m_nSpawnLocks) {
			CArray<SkippedTopic,SkippedTopic&> arTmp;
			for(int i = 0; i < arActions.GetSize(); i++) {
				EmrAction ea = arActions[i];
				// (z.manning 2009-02-23 12:54) - PLID 33141 - Replaced source detail pointer with source action info class
				//TES 2/16/2010 - PLID 37375 - Include the source HotSpot
				SourceActionInfo sai(&ea, pSourceDetail, pSourceSpot, ptrSourceTableRow);
				ProcessEmrAction(ea, sai, arTmp, bIsInitialLoad, NULL, pSourceEmn, NULL, pEmrActionsResult);
			}

			// (j.jones 2013-01-09 11:32) - PLID 45446 - potentially open medication reconciliation,
			// but that is unlikely because the new actions should still be pending at this point
			if(pEmrActionsResult != NULL && pEmrActionsResult->m_aryNewPrescriptionIDs.GetSize() > 0) {

				CEMN *pEMN = NULL;
				if(pSourceDetail) {
					pEMN = pSourceDetail->GetParentEMN();
				}
				if(pEMN == NULL) {
					pEMN = pSourceEmn;
				}

				ReconcileCurrentMedicationsWithSpawnedPrescriptions(pEMN, pEmrActionsResult->m_aryNewPrescriptionIDs);
			}

			ASSERT(!arTmp.GetSize());
			// (a.walling 2007-10-18 14:16) - PLID 25548 - Decrease the depth
			DecreaseEmrActionsDepth();
			return;
		}

		// (z.manning, 01/31/2007) - If we weren't passed a source EMN, use the source detail.
		if(pSourceEmn == NULL) {
			if(pSourceDetail) {
				pSourceEmn = pSourceDetail->m_pParentTopic->GetParentEMN();
			}
			else {
				ASSERT(FALSE);
			}
		}

		if(pSourceEmn) {
			//TES 1/23/2008 - PLID 24157 - Renamed.
			pSourceEmn->LockHandlingDetailChanges();
		}

		//TES 4/7/2006 - Keep the user informed on the progress.  Here's how this is going to work:
		//we will assign each type of action a certain block of the progress bar (50 units for eaoMintItems or eaoMint,
		//10 units for everything else, plus an extra 50 units for the skipped topics).  Then for each action, we will set up 
		//our CActionProgressInfo to constrain to that range, and pass it into ProcessEmrAction, which will be responsible 
		//for updating the progress on whatever logical range is most convenient for it.
		int nTotalProgressUnits = 50; //For skipped topics

		// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
		int i = 0;
		for(i = 0; i < arActions.GetSize(); i++) {
			if(arActions[i].eaoDestType == eaoMintItems || arActions[i].eaoDestType == eaoMint) nTotalProgressUnits += 50;
			else nTotalProgressUnits += 10;
		}

		BOOL bMustDestroyProgressDlg = FALSE;
		if(!m_pActionProgressDlg) {
			// (a.walling 2007-09-07 08:58) - PLID 24371 - wait at least a bit over a tenth of a second before displaying the
				// progress window. Also send a valid parent window.
			//TES 1/11/2008 - PLID 24157 - If our popup dialog is open, use it as the parent, otherwise the progress bar
			// will draw behind it.
			CEMRItemAdvMultiPopupDlg *pDlg = GetOpenMultiPopupDlg();
			HWND hwndParent = NULL;
			if(pDlg) {
				hwndParent = pDlg->GetSafeHwnd();
			}
			else if(GetInterface()) {
				hwndParent = GetInterface()->GetSafeHwnd();
			}
			m_pActionProgressDlg = new CShowProgressFeedbackDlg(150, TRUE, FALSE, FALSE, hwndParent);
			bMustDestroyProgressDlg = TRUE;
			m_pActionProgressDlg->SetCaption("Processing actions...");
			m_nCurrentProgress = 0;
			m_nProgressMax = nTotalProgressUnits;
		}
		else {
			//We need to insert ourself into the progress bar.
			m_nProgressMax += nTotalProgressUnits;
			m_pActionProgressDlg->SetProgress(0, m_nProgressMax, m_nCurrentProgress);
		}

		CProgressParameter pp;
		pp.Init(m_pActionProgressDlg, 0, m_nProgressMax);
		
		//TES 1/24/2006 - Process the eaoMintItems actions first, as other actions (specifically eaoEmrItem), may depend
		//on the topics that are spawned by that action.
		//TES 2/20/2006 - Track any topics that get skipped, try to put them in once they're done.
		CArray<SkippedTopic,SkippedTopic&> arSkippedTopics;
		for(i = 0; i < arActions.GetSize(); i++) {
			if(arActions[i].eaoDestType == eaoMintItems) {
				// (c.haag 2007-08-29 17:37) - PLID 27234 - Init first, SetSubRange after.
				//We have to keep re-initializing the ProgressParameter, in case m_nProgressMax changes
				pp.Init(m_pActionProgressDlg, 0, m_nProgressMax);
				pp.SetSubRange(m_nCurrentProgress, m_nCurrentProgress+50);
				EmrAction ea = arActions[i];
				// (z.manning 2009-02-23 12:54) - PLID 33141 - Replaced source detail pointer with source action info class
				//TES 2/16/2010 - PLID 37375 - Include the source HotSpot
				SourceActionInfo sai(&ea, pSourceDetail, pSourceSpot, ptrSourceTableRow);
				ProcessEmrAction(ea, sai, arSkippedTopics, bIsInitialLoad, &pp, pSourceEmn, NULL, pEmrActionsResult);
				m_nCurrentProgress += 50;
				m_pActionProgressDlg->SetProgress(0, m_nProgressMax, m_nCurrentProgress);
				arActions.RemoveAt(i);
				i--;
			}
		}
		//Now, put the skipped topics in.
		int nProgressAfterSkipped = m_nCurrentProgress+50;
		int nSkippedTopicCount = arSkippedTopics.GetSize();
		if(nSkippedTopicCount) {
			// (a.walling 2007-10-26 09:17) - PLID 25548 - Keep the new topics here so we can bulk add them into the preview
			CArray<CEMRTopic*, CEMRTopic*> arNewSkippedTopics;
			for(i = 0; i < nSkippedTopicCount; i++) {
				m_nCurrentProgress += 50/nSkippedTopicCount;
				m_pActionProgressDlg->SetProgress(0, m_nProgressMax, m_nCurrentProgress);
				if(pSourceDetail) {
					CEMRTopic* pNewSkippedTopic = pSourceDetail->m_pParentTopic->GetParentEMN()->ProcessSkippedTopic(pSourceDetail, arSkippedTopics[i]);
					// (a.walling 2007-10-26 09:14) - PLID 25548 - Now add to the array
					if (pNewSkippedTopic) {
						arNewSkippedTopics.Add(pNewSkippedTopic);
					}
				}
			}

			// (a.walling 2007-10-26 09:18) - PLID 25548 - Send the add preview message
			if (pSourceDetail->m_pParentTopic->GetParentEMN()->GetInterface()) {
				pSourceDetail->m_pParentTopic->GetParentEMN()->GetInterface()->SendMessage(NXM_EMR_TOPIC_ADD_PREVIEW, (WPARAM)&arNewSkippedTopics, (LPARAM)NULL);
			}
		}
		else {
			m_nCurrentProgress += 50;
			m_pActionProgressDlg->SetProgress(0, m_nProgressMax, m_nCurrentProgress);
		}

		// (j.jones 2011-04-28 14:39) - PLID 43122 - We need the provider ID to calculate floating data,
		// try to get it from our source object. If it's a hotspot or table row, the caller should have
		// given us a source detail to go with it.
		long nProviderIDForFloatingData = -1;
		if(pSourceDetail != NULL) {
			nProviderIDForFloatingData = pSourceDetail->GetProviderIDForFloatingData();
		}
		else if(pSourceEmn != NULL) {
			nProviderIDForFloatingData = pSourceEmn->GetProviderIDForFloatingData();
		}

		//Now, process the rest of the actions.
		// (c.haag 2007-08-06 10:24) - PLID 26954 - Create the CEMNSpawner object for preloading
		// spawning-related data in bulk.
		// (c.haag 2007-08-06 13:13) - PLID 26977 - Added m_bIsTemplate
		CEMNSpawner spawner(arActions, m_bIsTemplate);
		spawner.PreloadAllData(GetRemoteData(), nProviderIDForFloatingData);
		for(i = 0; i < arActions.GetSize(); i++) {
			int nProgressUnits = arActions[i].eaoDestType == eaoMint ? 50 : 10;
			// (c.haag 2007-08-29 17:37) - PLID 27234 - Init first, SetSubRange after.
			pp.Init(m_pActionProgressDlg, 0, m_nProgressMax);
			pp.SetSubRange(m_nCurrentProgress, m_nCurrentProgress+nProgressUnits);
			EmrAction ea = arActions[i];
			// (z.manning 2009-02-23 12:54) - PLID 33141 - Replaced source detail pointer with source action info class
			//TES 2/16/2010 - PLID 37375 - Include the source HotSpot
			SourceActionInfo sai(&ea, pSourceDetail, pSourceSpot, ptrSourceTableRow);
			ProcessEmrAction(ea, sai, arSkippedTopics, bIsInitialLoad, &pp, pSourceEmn, &spawner, pEmrActionsResult);
			m_nCurrentProgress += nProgressUnits;
			m_pActionProgressDlg->SetProgress(0, m_nProgressMax, m_nCurrentProgress);
		}

		if(bMustDestroyProgressDlg) {
			delete m_pActionProgressDlg;
			m_pActionProgressDlg = NULL;
			m_nProgressMax = 0;
			m_nCurrentProgress = 0;
		}

		// (z.manning, 07/11/2006) - PLID 19291 - Rather than calling UpdateNarrative for every single
		// action that gets processed/revoked, we now prevent UpdateNarrative from doing anything, instead
		// just reloading all narratives once the actions have all finished processing.
		if(pSourceEmn) {
			//TES 1/23/2008 - PLID 24157 - Renamed.
			pSourceEmn->UnlockHandlingDetailChanges();
		}

		// (j.jones 2013-01-09 11:32) - PLID 45446 - potentially open medication reconciliation
		if(pEmrActionsResult != NULL && pEmrActionsResult->m_aryNewPrescriptionIDs.GetSize() > 0) {
			
			
			CEMN *pEMN = NULL;
			if(pSourceDetail) {
				pEMN = pSourceDetail->GetParentEMN();
			}
			if(pEMN == NULL) {
				pEMN = pSourceEmn;
			}

			ReconcileCurrentMedicationsWithSpawnedPrescriptions(pEMN, pEmrActionsResult->m_aryNewPrescriptionIDs);
		}

	}NxCatchAll("Error in CEMR::ProcessEmrActions()");

	// (a.walling 2007-10-18 14:16) - PLID 25548 - Decrease the depth
	DecreaseEmrActionsDepth();
}

// (j.jones 2013-01-09 11:48) - PLID 45446 - opens medication reconciliation as a result of spawning multiple prescriptions
void CEMR::ReconcileCurrentMedicationsWithSpawnedPrescriptions(CEMN *pEMN, CArray<long, long> &aryNewPrescriptionIDs)
{
	CEmrTreeWnd *pTree = GetInterface();
	if(pTree != NULL && pEMN != NULL && !pEMN->IsLoading()) {

		//open medication reconciliation
		if(!m_bIsTemplate) {
			pTree->ReconcileCurrentMedicationsWithNewPrescriptions(pEMN, aryNewPrescriptionIDs);

			//if reconciliation changed the current meds table, it would have forced a save
		}

		// (j.jones 2012-10-22 17:40) - PLID 52819 - must reload medications, if we added one
		pEMN->ReloadMedicationsFromData(TRUE);

		// (j.jones 2012-11-13 10:10) - PLID 52869 - changed to be a posted message
		if(!m_bIsTemplate) {
			// (z.manning 2013-09-17 15:36) - PLID 58450 - New function for this
			pEMN->CheckSaveEMNForDrugInteractions(FALSE);
		}
	}
}

// (a.walling 2007-10-26 09:05) - PLID 25548 - Helper functions to increase and decrease the EmrActions depth
void CEMR::IncreaseEmrActionsDepth()
{
	m_nProcessEmrActionsDepth++;
}

// (a.walling 2007-10-26 09:05) - PLID 25548 - Helper functions to increase and decrease the EmrActions depth
// and call ProcessEmrActionsComplete when 0
void CEMR::DecreaseEmrActionsDepth()
{
	m_nProcessEmrActionsDepth--;

	if (m_nProcessEmrActionsDepth == 0) {
		ProcessEmrActionsComplete();
	}
}

// (a.walling 2007-10-18 12:01) - PLID 25548 - this is called at the end of every ProcessEmrActions function. If the depth is zero,
// it sends a message to the treewnd that the actions are completely processed.
void CEMR::ProcessEmrActionsComplete()
{
	try {
		// (a.walling 2007-10-18 15:54) - PLID 25548 - Don't bother sending when not on a template
		if (!GetIsTemplate()) {
			for(int i=0; i<m_arypEMNs.GetSize(); i++) {
				CEMN *pEMN = (CEMN*)m_arypEMNs.GetAt(i);
				// TES 1/15/2008 - PLID 24157 - Give our EMN a chance to do anything it needs to do after all
				// spawning is complete (like popup all the popped-up details, for instance).
				pEMN->ProcessEmrActionsComplete();

				// (a.walling 2007-10-15 12:09) - PLID 25548
				// Send our message to the interface that all spawning is now complete
				if(GetInterface() != NULL && GetInterface()->GetSafeHwnd() != NULL) {
					GetInterface()->SendMessage(NXM_EMR_SPAWNING_COMPLETE, (WPARAM)pEMN);
				}
			}
		}
	} NxCatchAllSilentCallThrow(ASSERT(FALSE));
}

// (z.manning, 01/31/2007) - Added an option parameter for the source EMN since not all actions (e.g. procedure
// based actions) have a source detail.
// (c.haag 2007-08-06 10:31) - PLID 26954 - Added an optional parameter for preloaded spawning-related data
// (c.haag 2008-06-17 10:12) - PLID 17842 - Added an optional parameter to allow the caller to track what was spawned
// (z.manning 2009-02-23 12:54) - PLID 33141 - Replaced source detail pointer with source action info class
void CEMR::ProcessEmrAction(EmrAction ea, SourceActionInfo &sai, CArray<SkippedTopic,SkippedTopic&> &arSkippedTopics, BOOL bIsInitialLoad, CProgressParameter *pProgress /*= NULL*/, CEMN* pSourceEmn /* = NULL */,
							CEMNSpawner* pSpawner /*= NULL */, CProcessEmrActionsResult* pEmrActionsResult /*= NULL*/)
{
	if(m_bIgnoreActions) return;

	if(IsActionLocked(ea)) {
		//This would be an infinite loop.
		return;
	}

	if(m_nSpawnLocks) {
		PendingAction *pa = new PendingAction;
		pa->ea = ea;
		pa->sai = sai;
		pa->bIsInitialLoad = bIsInitialLoad;
		pa->bProcessed = FALSE;
		if(pSourceEmn == NULL) {
			// (z.manning 2011-03-04 14:56) - PLID 42682 - If they didn't pass in a source EMN try using the one
			// from the source detail.
			if(pa->sai.pSourceDetail != NULL) {
				pa->pSourceEmn = pa->sai.pSourceDetail->GetParentEMN();
			}
		}
		else {
			pa->pSourceEmn = pSourceEmn;
		}
		if(bIsInitialLoad && pa->ea.bPopup) {
			//revoke the popup status if we originally did not wish to popup the item
			pa->ea.bPopup = FALSE;
		}
		m_arPendingActions.Add(pa);
		return;
	}
	//Don't let this action be processed again as a result of itself.
	LockAction(ea);

	// (z.manning, 01/31/2007) - PLID 24468 - If we weren't passed a source EMN, find it from the source detail
	if(pSourceEmn == NULL) {
		if(sai.pSourceDetail) {
			pSourceEmn = sai.pSourceDetail->m_pParentTopic->GetParentEMN();
		}
		else {
			ASSERT(FALSE);
		}
	}

	// (c.haag 2007-07-03 12:13) - I'm tired of "Error processing EMR Action." errors thrown by
	// Windows with no information as to where they were thrown from. I'm adding special exception
	// catching to help pin things down
	try {
		switch(ea.eaoDestType) {
			//These just go down to the EMN.
		case eaoCpt:
		// (b.savon 2014-07-14 11:20) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
		//case eaoDiag:
		case eaoDiagnosis:
		case eaoEmrItem:
		case eaoProcedure:
		case eaoMedication:
		case eaoMintItems:
		case eaoTodo: // (c.haag 2008-06-24 10:08) - PLID 17244
		case eaoLab: // (z.manning 2008-10-02) - PLID 21094
			try {
				pSourceEmn->ProcessEmrAction(ea, sai, arSkippedTopics, bIsInitialLoad, pProgress, pSpawner, pEmrActionsResult);
			} NxCatchAllSilentCallThrow(ASSERT(FALSE));
			break;
		case eaoMint:
			try {
				if(!m_bIsTemplate) {
					//Check if it's a one-per-EMR EMN.
					BOOL bAddOnce = VarBool(GetTableField("EmrTemplateT","AddOnce","ID",ea.nDestID));
					//TES 7/28/2006 - PLID 21677 - We also need to check whether this template has already been spawned
					//by this action on this EMR.
					for(int i = 0; i < m_arypEMNs.GetSize(); i++) {
						if(m_arypEMNs[i]->GetTemplateID() == ea.nDestID) {
							//Is this add-once?
							if(bAddOnce) {
								//Yup, so don't process.
								// (a.walling 2007-09-26 11:34) - PLID 27503 - Unlock this action
								UnlockAction(ea);
								return;
							}
							else {
								//Is this an infinite loop?
								// (a.walling 2010-03-23 13:49) - PLID 37853 - This would prevent spawning the EMN if it was from the same action
								// on a different actual detail, for example. However there is still the trouble with an infinite loop, if spawning
								// a template causes one template to spawn which causes another, etc. To avoid this, we determine if we are being
								// spawned during the initial load of the EMN, ie an automatic 'remembered' value, and if so we prevent duplicates
								// if an unsaved EMN from the save action already exists.
								if (pSourceEmn) {
									// OK, we have the EMN that is spawning this mini-template.
									if (pSourceEmn->IsLoading() && pSourceEmn->GetID() == -1) {
										// we are being automatically spawned by an unsaved EMN.
										// If this is the case, we do not want to spawn if another unsaved EMN exists spawned from the same action.

										if(m_arypEMNs[i]->GetSourceActionInfo().nSourceActionID == ea.nID && m_arypEMNs[i]->GetID() == -1) {
											//Yup, so don't process.
											// (a.walling 2007-09-26 11:34) - PLID 27503 - Unlock this action
											UnlockAction(ea);
											return;
										}

									} else {
										// we are being spawned by some other event. Just ensure it's not the exact same source.
										// (z.manning 2011-11-02 16:50) - PLID 45993 - Added param to HasSameSource to check action ID to maintain same behavior
										if (sai.HasSameSource(m_arypEMNs[i]->GetSourceActionInfo(), TRUE)) {
											//Yup, so don't process.
											// (a.walling 2007-09-26 11:34) - PLID 27503 - Unlock this action
											UnlockAction(ea);
											return;
										}
									}
								} else {
									// (a.walling 2010-03-23 13:49) - PLID 37853 - Revert to class if this occurs I assume
									ASSERT(FALSE);
									if(m_arypEMNs[i]->GetSourceActionInfo().nSourceActionID == ea.nID) {
										//Yup, so don't process.
										// (a.walling 2007-09-26 11:34) - PLID 27503 - Unlock this action
										UnlockAction(ea);
										return;
									}
								}
							}

						}
					}
					//Add to memory object					
					// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking
					// (a.walling 2013-02-08 12:05) - PLID 55080 - Pass in the linked appointment
					long nInheritedAppointmentID = -1;
					if (pSourceEmn) {
						nInheritedAppointmentID = pSourceEmn->GetAppointment().nID;
					}
					CEMN *pNewEMN = AddEMNFromTemplate(ea.nDestID, sai, NULL, nInheritedAppointmentID);
					// (c.haag 2008-07-21 14:13) - PLID 30725 - Now that the EMN has been created, we're ready to spawn any problems associated
					// with the action
					ProcessEmrProblemActions(ea, sai, pNewEMN);

					//AddEMNFromTemplate will add to the interface
					/*Add to interface
					if(GetInterface()) {
						GetInterface()->SendMessage(NXM_EMN_ADDED, (WPARAM)pNewEMN);
					}
					*/
				}
			} NxCatchAllSilentCallThrow(ASSERT(FALSE));
			break;
		default:
			//Wha??
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll("Error processing EMR Action.");

	//We're done processing this action, so this action is now free to be processed again.
	UnlockAction(ea);

	// (j.jones 2006-10-13 09:18) - PLID 22461 - moved office visit incrementing to the save process
	/*
	if(m_nSpawnLocks == 0) {
		// (j.jones 2005-05-19 10:07) - once we are done spawning, warn
		// if any of the office visit codes were upgraded
		if(!m_bIsTemplate) {
			if(GetInterface()) {
				GetInterface()->SendMessage(NXM_PROCESS_EMR_OFFICE_VISITS);
			}
		}
	}
	*/
}

	// (c.haag 2008-07-21 14:21) - PLID 30725 - Processes problem actions which correspond to an action
	// that created a new EMN.
// (z.manning 2009-02-23 12:54) - PLID 33141 - Replaced source detail pointer with source action info class
void CEMR::ProcessEmrProblemActions(const EmrAction& ea, SourceActionInfo &sai, CEMN* pNewEMN)
{
	// (a.walling 2014-07-01 15:28) - PLID 62697
	for (const auto& epa : ea.aProblemActions) {
		ProcessEmrProblemAction(ea, epa, sai, pNewEMN);
	}
}

// (c.haag 2008-07-21 14:22) - PLID 30725 - This function will spawn a single problem onto an EMR object
// (z.manning 2009-02-23 12:54) - PLID 33141 - Replaced source detail pointer with source action info class
void CEMR::ProcessEmrProblemAction(const EmrAction& ea, const EmrProblemAction& epa, SourceActionInfo &sai,
								   CEMN* pNewEMN)
{
	EMRProblemRegardingTypes eprtType = eprtInvalid;
	EmrActionObject eaoType = (epa.bSpawnToSourceItem) ? ea.eaoSourceType : ea.eaoDestType;
	CEMN* pSourceEMN = (sai.pSourceDetail && sai.pSourceDetail->m_pParentTopic) ? sai.pSourceDetail->m_pParentTopic->GetParentEMN() : NULL;

	// The problem action tells us what we're spawning to. Now given the type of object, fetch the object
	// and add to it.
	switch (eaoType) {
	case eaoMint: // The problem will be bound to a spawned EMN.
		// (z.manning 2016-04-11 14:02) - NX-100140 - Make sure we want to spawn this problem
		if (!ShouldSpawnEmrProblemForPatient(GetPatientID(), epa.nID, GetDeletedEmrProblemIDs())) {
			return;
		}
		ProcessEmrProblemAction_eaoMint(epa, pNewEMN);
		break;
	default: // Defer to the source EMN
		if (NULL != pSourceEMN) {
			pSourceEMN->ProcessEmrProblemAction(ea, epa, sai, NULL);
		}
		else {
			ASSERT(FALSE);
			ThrowNxException("Error in ProcessEmrProblemAction: Could not defer action to source EMN");
		}
		break;
	}
}

// (c.haag 2008-07-21 14:27) - PLID 30725 - This function will spawn a problem onto an EMN
void CEMR::ProcessEmrProblemAction_eaoMint(const EmrProblemAction& epa, CEMN* pDestEMN)
{
	// (s.tullis 2015-02-23 15:44) - PLID 64723 
	// (c.haag 2009-05-29 14:21) - PLID 34293 - Use the new problem link structure
	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs, but both are -1 here
	// (r.gonet 2015-03-09 18:21) - PLID 65008 - Pass DoNotShowOnProblemPrompt.
	CEmrProblem* pNewProblem = AllocateEmrProblem(-1, GetPatientID(),
		epa.strDescription, COleDateTime::GetCurrentTime(), COleDateTime::GetCurrentTime(), dtInvalid,
		epa.nStatus, -1, -1, -1, FALSE, -1, epa.bDoNotShowOnCCDA, epa.bDoNotShowOnProblemPrompt);
	pNewProblem->m_nEmrProblemActionID = epa.nID;
	CEmrProblemLink* pNewLink = new CEmrProblemLink(pNewProblem, -1, eprtEmrEMN, -1, -1);
	pNewProblem->Release(); // Release this function's reference to the problem. The EMR and link will retain references
	pNewLink->UpdatePointersWithEMN(pDestEMN);

	// (c.haag 2009-06-04 16:00) - PLID 34293 - Flag the EMR as modified
	SetUnsaved();

	// Add this problem to the EMN
	pDestEMN->m_apEmrProblemLinks.Add(pNewLink);
}

/*	DRT 8/3/2007 - PLID 26937 - This function should no longer be needed.  If you think you need it, 
		look at the CEMNUnspawner class instead.
void CEMR::RevokeEMRInfoActions(CEMNDetail *pSourceDetail)
{
	try {

		// (j.jones 2007-07-27 10:26) - PLID 26845 - check whether or not the info item has Info actions,
		// which it usually does not, such that we don't have to search for them
		if(pSourceDetail->GetHasInfoActionsStatus() == ehiasHasNoInfoItems) {
			return;
		}

		//if undetermined, ASSERT, because there is no reason it should not be known
		if(pSourceDetail->GetHasInfoActionsStatus() == ehiasUndetermined) {
			ASSERT(FALSE);
		}

		//Note that we include deleted actions
		MFCArray<EmrAction> arActionsToRevoke;

		//TES 12/5/2006 - PLID 23724 - We can just compare on EmrInfoMasterID now.
		// (j.jones 2006-09-21 15:04) - PLID 22611 - load the actions for this EMRInfo item
		// AND for all previous versions of this EMRInfo item
		//CString strAllPreviousInfoIDs = GeneratePastEMRInfoIDs(pSourceDetail->m_nEMRInfoID);

		// (a.wetta 2007-04-12 12:54) - PLID 25302 - Also make sure that deleted actions are not included
		LoadActionInfo(FormatString("SourceType = %i AND SourceID IN (SELECT ID FROM EmrInfoT WHERE EmrInfoMasterID = %li) AND Deleted = 0", eaoEmrItem, pSourceDetail->m_nEMRInfoMasterID), arActionsToRevoke);
		RevokeEmrActions(arActionsToRevoke, pSourceDetail);

	}NxCatchAll("Error in CEMR::RevokeEMRInfoActions()");
}
*/

/*	DRT 8/3/2007 - PLID 26937 - This function should no longer be needed.  If you think you need it, 
		look at the CEMNUnspawner class instead.
void CEMR::RevokeEMRDataActions(long nEMRDataID, CEMNDetail *pSourceDetail)
{
	try {

		//Note that we include deleted actions here.
		MFCArray<EmrAction> arActionsToRevoke;

		//TES 12/6/2006 - PLID 23766 - We can just compare on EmrDataGroupID now
		// (j.jones 2006-09-21 15:04) - PLID 22611 - load the actions for this EMRData item
		// AND for all previous versions of this EMRData item
		//CString strAllPreviousDataIDs = GeneratePastEMRDataIDs(nEMRDataID);

		// (a.wetta 2007-04-12 12:54) - PLID 25302 - Also make sure that deleted actions are not included
		LoadActionInfo(FormatString("SourceType = %i AND SourceID IN (SELECT ID FROM EmrDataT WHERE EmrDataGroupID = (SELECT EmrDataGroupID FROM EmrDataT SourceItem WHERE SourceItem.ID = %li)) AND Deleted = 0", eaoEmrDataItem, nEMRDataID), arActionsToRevoke);
		RevokeEmrActions(arActionsToRevoke, pSourceDetail);

	}NxCatchAll("Error in CEMR::RevokeEMRDataActions()");
}
*/

// (z.manning, 01/31/2007) - PLID 24468 - Load all actions for given procedures.
void CEMR::RevokeEMRProcedureActions(CArray<long,long> &aryProcedureIDs, CEMN* pSourceEmn)
{
	try {

		if(aryProcedureIDs.GetSize() <= 0) {
			return;
		}

		//Note that we include deleted actions
		MFCArray<EmrAction> arActionsToRevoke;

		// (a.wetta 2007-04-12 12:54) - PLID 25302 - Also make sure that deleted actions are not included
		LoadActionInfo(CSqlFragment("SourceType = {INT} AND SourceID IN ({INTARRAY}) AND Deleted = 0", eaoProcedure, aryProcedureIDs), arActionsToRevoke);
		SourceActionInfo sai;
		RevokeEmrActions(arActionsToRevoke, sai, pSourceEmn);

	}NxCatchAll("Error in CEMR::RevokeEMRProcedureActions()");
}

// (j.jones 2007-04-12 16:16) - PLID 25618 - removes any pending actions we may have
void CEMR::DeletePendingEMRActions(EmrAction ea, CEMNDetail *pSourceDetail)
{
	try {

		// (j.jones 2007-04-12 16:17) - PLID 25618 - See if our action is in the
		// locked, pending array of actions to process. If so, remove it from the array
		// (ignore the m_arLockedActionIDs array)

		for(int i = m_arPendingActions.GetSize() - 1; i >= 0; i--) {
			PendingAction *pa = m_arPendingActions.GetAt(i);
			if(pa->sai.pSourceDetail == pSourceDetail &&
				pa->ea.nID == ea.nID) {
				//it's our action, so let's get rid of it
				delete pa;
				m_arPendingActions.RemoveAt(i);
			}
		}


	}NxCatchAll("Error in CEMR::DeletePendingEMRActions()");
}

// (z.manning, 01/31/2007) - Added an option parameter for the source EMN since not all actions (e.g. procedure
// based actions) have a source detail.
// (z.manning 2009-02-24 09:32) - PLID 33138 - Changed pSourceDetail to source action info
void CEMR::RevokeEmrActions(MFCArray<EmrAction> &arActions, SourceActionInfo &sai, CEMN* pSourceEmn /* = NULL */)
{
	const int nActions = arActions.GetSize();
	int i;

	//TES 9/12/2006 - PLID 22042 - Don't do anything if there aren't any actions to revoke.
	if(!nActions) {
		return;
	}

	// (z.manning, 01/31/2007) - PLID 24468 - If we don't have a source EMN, find it based off the source detail.
	if(pSourceEmn == NULL) {
		if(sai.pSourceDetail != NULL) {
			pSourceEmn = sai.pSourceDetail->m_pParentTopic->GetParentEMN();
		}
		else {
			ASSERT(FALSE);
		}
	}

	// (c.haag 2008-08-07 10:51) - PLID 30979 - Gather lists of individual actions and source details 
	// (a.walling 2014-07-14 16:32) - PLID 62812 - use MFCArray

	MFCArray<CActionAndSource> aryCptActions;
	MFCArray<CActionAndSource> aryDiagActions;
	MFCArray<CActionAndSource> aryMedicationActions;
	MFCArray<CActionAndSource> aryProcedureActions;
	MFCArray<CActionAndSource> aryEmrItemActions;
	MFCArray<CActionAndSource> aryMintItemsActions;
	MFCArray<CActionAndSource> aryMintActions;
	MFCArray<CActionAndSource> aryTodoActions;
	MFCArray<CActionAndSource> aryLabActions; // (z.manning 2008-10-08 17:13) - PLID 31628
	for (i=0; i < nActions; i++) {
		CActionAndSource aas(arActions[i], sai);
		switch(arActions[i].eaoDestType) {
		case eaoCpt: aryCptActions.Add(aas); break;
		// (b.savon 2014-07-14 11:20) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide		
		//case eaoDiag: aryDiagActions.Add(aas); break;
		case eaoDiagnosis: aryDiagActions.Add(aas); break;
		case eaoMedication: aryMedicationActions.Add(aas); break;
		case eaoProcedure: aryProcedureActions.Add(aas); break;
		case eaoEmrItem: aryEmrItemActions.Add(aas); break;
		case eaoMintItems: aryMintItemsActions.Add(aas); break;
		case eaoMint: aryMintActions.Add(aas); break;
		case eaoTodo: aryTodoActions.Add(aas); break;
		case eaoLab: aryLabActions.Add(aas); break; // (z.manning 2008-10-08 17:13) - PLID 31628
		default:
			ASSERT(FALSE); // Not supported
			break;
		}
	}

	// (c.haag 2008-08-07 09:26) - PLID 30979 - Gather a list of all the objects that will be deleted
	// as a result of the revoking before we actually do any revoking.
	CDoomedEmrObjectAry aryDoomedCpts;
	CDoomedEmrObjectAry aryDoomedDiags;
	CDoomedEmrObjectAry aryDoomedMedications;
	CDoomedEmrObjectAry aryDoomedProcedures;
	CDoomedEmrObjectAry aryDoomedItems;
	CDoomedEmrObjectAry aryDoomedTopics;
	CDoomedEmrObjectAry aryDoomedEMNs;
	CDoomedEmrObjectAry aryDoomedTodos;
	CDoomedEmrObjectAry aryDoomedLabs; // (z.manning 2008-10-08 16:51) - PLID 31628
	
	// (j.jones 2011-07-11 10:56) - PLID 38366 - track coding groups that will be affected
	CEmrCodingGroupArray aryCodingGroupsToChange;
	CMap<CEmrCodingGroup*, CEmrCodingGroup*, long, long> mapCodingGroupQuantitiesToSubtract;

	if(pSourceEmn && aryCptActions.GetSize() > 0) {
		// (j.jones 2011-07-11 10:56) - PLID 38366 - track coding groups that will be affected
		pSourceEmn->GetEmrObjectsToRevoke_eaoCpt(&aryCptActions, aryDoomedCpts, aryCodingGroupsToChange, mapCodingGroupQuantitiesToSubtract);
	}
	if(pSourceEmn && aryDiagActions.GetSize() > 0) {
		pSourceEmn->GetEmrObjectsToRevoke_eaoDiag(&aryDiagActions, aryDoomedDiags);
	}
	if(pSourceEmn && aryMedicationActions.GetSize() > 0) {
		pSourceEmn->GetEmrObjectsToRevoke_eaoMedication(&aryMedicationActions, aryDoomedMedications);
	}
	if(pSourceEmn && aryProcedureActions.GetSize() > 0) {
		pSourceEmn->GetEmrObjectsToRevoke_eaoProcedure(&aryProcedureActions, aryDoomedProcedures);
	}
	if(pSourceEmn && aryEmrItemActions.GetSize() > 0) {
		pSourceEmn->GetEmrObjectsToRevoke_eaoEmrItem(&aryEmrItemActions, aryDoomedItems);
	}
	if(pSourceEmn && aryMintItemsActions.GetSize() > 0) {
		pSourceEmn->GetEmrObjectsToRevoke_eaoMintItems(&aryMintItemsActions, aryDoomedTopics);
	}
	if(aryMintActions.GetSize() > 0) {
		//Special:  Mint's are handled at the EMR level, not the EMN level.
		GetEmrObjectsToRevoke_eaoMint(&aryMintActions, aryDoomedEMNs);
	}
	if(pSourceEmn && aryTodoActions.GetSize() > 0) {
		pSourceEmn->GetEmrObjectsToRevoke_eaoTodo(&aryTodoActions, aryDoomedTodos);
	}
	if(pSourceEmn != NULL && aryLabActions.GetSize() > 0) { // (z.manning 2008-10-08 17:02) - PLID 31628
		pSourceEmn->GetEmrObjectsToRevoke_eaoLab(&aryLabActions, aryDoomedLabs);
	}

	// (c.haag 2008-08-07 11:23) - PLID 30799 - Gather a list of problems that will
	// be removed by iterating through the list of doomed objects
	// (c.haag 2009-05-29 16:26) - PLID 34293 - Gather problem links, not problems
	CArray<CEmrProblemLink*,CEmrProblemLink*> aryProblemLinks;
	CDoomedEmrObjectAry aryAllDoomedObjects;
	if (!m_bIsTemplate) {
		aryAllDoomedObjects.Append(aryDoomedCpts);
		aryAllDoomedObjects.Append(aryDoomedDiags);
		aryAllDoomedObjects.Append(aryDoomedMedications);
		aryAllDoomedObjects.Append(aryDoomedProcedures);
		aryAllDoomedObjects.Append(aryDoomedItems);
		aryAllDoomedObjects.Append(aryDoomedTopics);
		aryAllDoomedObjects.Append(aryDoomedEMNs);
		aryAllDoomedObjects.Append(aryDoomedTodos);
		aryAllDoomedObjects.Append(aryDoomedLabs); // (z.manning 2008-10-08 17:03) - PLID 31628
		// Now populate the problems array with all the problems that would be removed by the act of unspawning
		// (c.haag 2009-05-29 16:26) - PLID 34293 - Gather problem links, not problems
		FillUnspawnedProblemLinksArray(aryAllDoomedObjects, aryProblemLinks);
	}

	// (c.haag 2008-08-07 11:33) - PLID 30942 - Warn of problems that will be deleted by virtue
	// of EMR objects being unspawned
	if (aryProblemLinks.GetSize() > 0) {
		WarnOfUnspawningProblemLinks(aryProblemLinks);
	}

	// (c.haag 2008-08-07 11:23) - PLID 30799 - We will need to notify all affected EMN's
	// to update their problem flag icons. Build a map of the EMN interface windows.
	// (c.haag 2009-05-29 14:24) - PLID 34293 - Use new problem linking structure
	CMap<HWND,HWND,BOOL,BOOL> mapProblemWarnHwnds;
	if (aryProblemLinks.GetSize() > 0) {

		// Gather a unique list of EMN's by iterating through our problem array
		for (int iProb=0; iProb < aryProblemLinks.GetSize(); iProb++) {
			CEmrProblemLink* pLink = aryProblemLinks[iProb];
			if (NULL != pLink->GetEMN() &&
				NULL != pLink->GetEMN()->GetInterface() &&
				IsWindow(pLink->GetEMN()->GetInterface()->GetSafeHwnd())
				)
			{
				mapProblemWarnHwnds.SetAt(pLink->GetEMN()->GetInterface()->GetSafeHwnd(), TRUE);
			}
		} // for (int iProb=0; iProb < aryProblems.GetSize(); iProb++) {
	}

	// (z.manning, 07/11/2006) - PLID 19291 - Rather than calling UpdateNarrative for every single
	// action that gets processed/revoked, we now prevent UpdateNarrative from doing anything, instead
	// just reloading all narratives once the actions have all finished processing.
	if(pSourceEmn) {
		//TES 1/23/2008 - PLID 24157 - Renamed.
		pSourceEmn->LockHandlingDetailChanges();
	}

	if(aryDoomedCpts.GetSize() > 0 || aryCodingGroupsToChange.GetSize() > 0) {
		try {
			// (j.jones 2011-07-11 10:56) - PLID 38366 - pass in the coding groups that will be affected
			pSourceEmn->RevokeEmrActions_eaoCpt(aryDoomedCpts, aryCodingGroupsToChange, mapCodingGroupQuantitiesToSubtract);
		} NxCatchAll("Error in CEMR::RevokeEmrActions() : eaoCpt");
	}
	if(aryDoomedDiags.GetSize() > 0) {
		try {
			pSourceEmn->RevokeEmrActions_eaoDiag(aryDoomedDiags);
		} NxCatchAll("Error in CEMR::RevokeEmrActions() : eaoDiag");
	}
	if(aryDoomedMedications.GetSize() > 0) {
		try {
			pSourceEmn->RevokeEmrActions_eaoMedication(aryDoomedMedications);
		} NxCatchAll("Error in CEMR::RevokeEmrActions() : eaoMedication");
	}
	if(aryDoomedProcedures.GetSize() > 0) {
		try {
			pSourceEmn->RevokeEmrActions_eaoProcedure(aryDoomedProcedures);
		} NxCatchAll("Error in CEMR::RevokeEmrActions() : eaoProcedure");
	}
	if(aryDoomedItems.GetSize() > 0) {
		try {
			pSourceEmn->RevokeEmrActions_eaoEmrItem(aryDoomedItems);
		} NxCatchAll("Error in CEMR::RevokeEmrActions() : eaoEmrItem");
	}
	if(aryDoomedTopics.GetSize() > 0) {
		try {
			pSourceEmn->RevokeEmrActions_eaoMintItems(aryDoomedTopics);
		} NxCatchAll("Error in CEMR::RevokeEmrActions() : eaoMintItems");
	}
	if(aryDoomedEMNs.GetSize() > 0) {
		//Special:  Mint's are handled at the EMR level, not the EMN level.
		try {
			RevokeEmrActions_eaoMint(aryDoomedEMNs);
		} NxCatchAll("Error in CEMR::RevokeEmrActions() : eaoMint");
	}
	if(aryDoomedTodos.GetSize() > 0) { // (c.haag 2008-06-24 10:55) - PLID 17244
		try {
			pSourceEmn->RevokeEmrActions_eaoTodo(aryDoomedTodos);
		} NxCatchAll("Error in CEMR::RevokeEmrActions() : eaoTodo");
	}
	if(aryDoomedLabs.GetSize() > 0) { // (z.manning 2008-10-08 17:03) - PLID 31628
		try {
			pSourceEmn->RevokeEmrActions_eaoLab(aryDoomedLabs);
		} NxCatchAll("CEMR::RevokeEmrActions : eaoLab");
	}

	if(pSourceEmn) {
		//TES 1/23/2008 - PLID 24157 - Renamed.
		pSourceEmn->UnlockHandlingDetailChanges();
	}

	// (c.haag 2008-08-07 11:24) - PLID 30799 - Now we need to notify all affected EMN's
	// to update their problem flag icons
	// (c.haag 2009-05-29 16:26) - PLID 34293 - Now problem links
	if (aryProblemLinks.GetSize() > 0) {

		// Now go through the map and update EMN's
		POSITION pos = mapProblemWarnHwnds.GetStartPosition();
		BOOL bDummy;
		HWND hWnd;
		while (NULL != pos) {
			mapProblemWarnHwnds.GetNextAssoc(pos, hWnd, bDummy);
			if (IsWindow(hWnd)) {
				::PostMessage(hWnd, NXM_EMR_PROBLEM_CHANGED, 0, 0);
			}
		}

	} // if (aryProblems.GetSize() > 0) {
}

// (z.manning, 01/31/2007) - Added an option parameter for the source EMN since not all actions (e.g. procedure
// based actions) have a source detail.
// (c.haag 2008-08-07 09:50) - PLID 30979 - This function has been deprecated
//void CEMR::RevokeEmrAction(EmrAction ea, CEMNDetail *pSourceDetail, CEMN* pSourceEmn /* = NULL */)
/*
{
	try {
		//DRT 8/7/2007 - PLID 27003 - Action locking moved to each individual revoking function

		// (z.manning, 01/31/2007) - PLID 24468 - If we don't have a source EMN, find it based off the source detail.
		if(pSourceEmn == NULL) {
			if(pSourceDetail) {
				pSourceEmn = pSourceDetail->m_pParentTopic->GetParentEMN();
			}
			else {
				ASSERT(FALSE);
			}
		}

	switch(ea.eaoDestType) {
		case eaoCpt:
		case eaoDiag:
		case eaoMedication:
		case eaoProcedure:
		case eaoEmrItem:
		case eaoMintItems:
		case eaoTodo: // (c.haag 2008-06-24 10:08) - PLID 17244
			{
				//Pass to EMN.

				// (j.jones 2007-04-12 16:30) - PLID 25618 - this will revoke any locked spawns, so
				// no need to do it here
				pSourceEmn->RevokeEmrAction(ea, pSourceDetail);
			}
			break;
		case eaoMint:
			{
				//DRT 8/7/2007 - PLID 27003 - This now calls the direct RevokeEmrActions_eaoMint function.
				// (c.haag 2008-08-01 16:26) - PLID 30897 - We now do the act of finding EMN objects and
				// removing them in two separate functions
				CActionAndSource aas;
				aas.ea = ea;
				aas.pSourceDetail = pSourceDetail;
				MFCArray<CActionAndSource> aryActionsToRevoke;
				aryActionsToRevoke.Add(aas);

				CDoomedEmrObjectAry aryDoomedObjects;
				GetEmrObjectsToRevoke_eaoMint(&aryActionsToRevoke, aryDoomedObjects);
				RevokeEmrActions_eaoMint(aryDoomedObjects);
			}
			break;
		}
		
	}NxCatchAll("Error in CEMR::RevokeEMRAction()");
}*/

// (c.haag 2008-08-01 16:27) - PLID 30897 - This function gathers a list of all EMN objects that will be removed
// from an EMR as a result of one or more unspawning actions, and stores the EMN object pointer and action-pertinent 
// information in adeoDoomedObjects. That array will later be used in the actual removing of those EMN's.
// (a.walling 2014-07-14 16:32) - PLID 62812 - use MFCArray
void CEMR::GetEmrObjectsToRevoke_eaoMint(MFCArray<CActionAndSource> *paryActionsToRevoke, CDoomedEmrObjectAry& adeoDoomedObjects)
{
	//CDoomedEmrObjectAry adeoLocalDoomedObjects;
	CArray<CEMN*, CEMN*> aryDoomedEMNs;
	MFCArray<CActionAndSource> localActions;
	const long nActionCnt = paryActionsToRevoke->GetSize();

	for (int i = 0; i < nActionCnt; i++) {
		CActionAndSource aas = paryActionsToRevoke->GetAt(i);
		EmrAction ea = aas.ea;
		CEMNDetail *pSourceDetail = aas.sai.pSourceDetail;

		// Confirm it's a mint action
		if(ea.eaoDestType != eaoMint) {
			AfxThrowNxException("Invalid operation when attempting to gather EMNs for action revoking");
		}

		// (j.jones 2007-04-12 16:31) - PLID 25618 - revoke any locked spawns (note: different
		// from locked actions, confusing as that is)
		DeletePendingEMRActions(ea, pSourceDetail);

		for(int i = m_arypEMNs.GetSize() - 1; i >= 0; i--) {
			CEMN *pEMN = m_arypEMNs[i];

			// (j.jones 2007-01-12 15:10) - PLID 24027 - added SourceDetailID support
			// (z.manning 2009-03-04 15:49) - PLID 33338 - Use the utility function for this logic
			if(IsSourceActionInfoOk(aas.sai, pEMN->GetSourceActionInfo(), ea, pEMN->GetTemplateID())) {
				//TES 10/6/2006 - PLID 22437 - Don't remove this EMN if it is locked!
				if(pEMN->IsLockedAndSaved()) {
					continue;
				}

				// (a.walling 2007-09-26 09:51) - PLID 27503 - we will be removing this EMN
				// (a.walling 2007-10-25 17:55) - PLID 27503 - Don't add duplicates
				// (a.walling 2010-03-31 16:48) - PLID 38007 - We now keep track of all the EMNs here and create the DoomedObject array later
				int i = 0;
				for (; i < aryDoomedEMNs.GetSize(); i++) {
					if (aryDoomedEMNs[i] == pEMN)
						break;
				}
				if (i == aryDoomedEMNs.GetSize()) {
					aryDoomedEMNs.Add(pEMN);
					localActions.Add(aas);
				}
			}
		}
	} // for (int i = 0; i < nActionCnt; i++) {
	
	CArray<CEMN*, CEMN*> aryChosenDoomedEMNs;
	aryChosenDoomedEMNs.Copy(aryDoomedEMNs);

	for (int i = aryChosenDoomedEMNs.GetCount() - 1; i >= 0; i--) {		
		// (a.walling 2010-03-31 14:19) - PLID 38010 - Don't remove this EMN if it has E-Prescribing jibber jabber
		if(aryChosenDoomedEMNs[i]->HasEPrescriptions()) {
			aryChosenDoomedEMNs.RemoveAt(i);
		}
	}

	if (GetInterface() != NULL && m_bIsTemplate == FALSE && aryDoomedEMNs.GetSize() > 0) {
		// (a.walling 2007-09-25 09:10) - PLID 27503 - Only continue removing if the message handler
		// gives us the go ahead (with the appropriate return value of 0xDEAD)
		LRESULT nResult = GetInterface()->SendMessage(NXM_QUERY_UNSPAWN_EMNS, (WPARAM)(&aryChosenDoomedEMNs));

		// (a.walling 2010-03-31 15:42) - PLID 38007 - We may still have to perform some other actions here.
		/*
		if (nResult != (LRESULT)0xDEAD) {
			// The user does not approve of the unspawning
			return;
		}
		*/
	}

	// (a.walling 2010-03-31 15:38) - PLID 38007 - Now let us create our final array of doomed objects. Previously the DoomedObject array
	// was never touched; it was either unspawn all or unspawn nothing, regardless of the contents of aryDoomedEMNs.
	
	ASSERT(aryDoomedEMNs.GetSize() == localActions.GetSize());

	for (int i = 0; i < aryDoomedEMNs.GetSize(); i++) {
		// first off see if we can find it in the chosen array
		bool bFound = false;
		for (int j = 0; j < aryChosenDoomedEMNs.GetSize() && !bFound; j++) {
			if (aryDoomedEMNs[i] == aryChosenDoomedEMNs[j]) {
				bFound = true;
			}
		}
		
		CDoomedEmrObject deo;
		deo.m_aas = localActions[i];
		deo.SetEMN(aryDoomedEMNs[i]);
		// (a.walling 2010-03-31 15:45) - PLID 38008 - If we did not find it in the chosen array, then it is skipped.
		deo.SetIsSkipped(!bFound);
		adeoDoomedObjects.Add(deo);
	}

	// Go ahead with the unspawning. Copy the local array of doomed objects to the output array.
	// (a.walling 2010-03-31 15:45) - PLID 38007 - We just add to this array directly above now
	//adeoDoomedObjects.Append(adeoLocalDoomedObjects);
}

// (c.haag 2008-08-01 16:02) - PLID 30897 - This function no longer searches for objects to delete. Those
// objects are now passed into this function in adeoDoomedObjects
void CEMR::RevokeEmrActions_eaoMint(const CDoomedEmrObjectAry& adeoDoomedObjects)
{
	long nActionCnt = adeoDoomedObjects.GetSize();
	for(int i = 0; i < nActionCnt; i++) {
		CDoomedEmrObject deo = adeoDoomedObjects.GetAt(i);

		// (a.walling 2010-03-31 15:49) - PLID 38008 - Skip if we are told to
		if (deo.GetIsSkipped()) {
			continue;
		}
		CActionAndSource aas = deo.m_aas;
		EmrAction ea = aas.ea;
		CEMNDetail *pSourceDetail = aas.sai.pSourceDetail;
		CEMN* pEMN = deo.GetEMN();

		//First of all, this detail can't unspawn itself.
		if(pSourceDetail) {
			pSourceDetail->SetAllowUnspawn(FALSE);
		}

		//DRT 8/7/2007 - Moved action locking here from CEMR::RevokeEmrAction
		if(IsActionLocked(ea)) {
			//This would be an infinite loop.
			return;
		}
		LockAction(ea);

		//Confirm that every action is indeed a mint items action
		if(ea.eaoDestType != eaoMint) {
			AfxThrowNxException("Attempted to revoke a non-mint action via mint procedure.");
		}

		if(!m_bIsTemplate) {

			//TES 10/6/2006 - PLID 22437 - Don't remove this EMN if it is locked!
			if(pEMN->IsLockedAndSaved()) {
				continue;
			}

			//Tell the interface that we're about to remove it.
			if(GetInterface()) {
				// (a.walling 2007-09-25 09:10) - PLID 27503 - Only continue removing if the message handler
				// gives us the go ahead (with the appropriate return value of 0xDEAD)
				// (a.walling 2007-09-26 09:57) - PLID 27503 - We just ask once at the beginning instead
				GetInterface()->SendMessage(NXM_PRE_DELETE_EMN, (WPARAM)pEMN);
				// (z.manning, 10/08/2007) - PLID 27662 - Also pre-delete any procedures so we are
				// prompted to remove any deleted procedures from the PIC as well.
				for(int nProcIndex = 0; nProcIndex < pEMN->GetProcedureCount(); nProcIndex++) {
					GetInterface()->SendMessage(NXM_PRE_DELETE_EMN_PROCEDURE, (WPARAM)pEMN->GetProcedure(nProcIndex)->nID, (LPARAM)pEMN);
				}
			}

			// (c.haag 2008-07-21 17:40) - PLID 30799 - Unspawn any problems for this action which are bound to the source item
			if (NULL != pSourceDetail && NULL != pSourceDetail->m_pParentTopic) {
				pSourceDetail->m_pParentTopic->GetParentEMN()->RevokeEmrProblemAction(ea, pSourceDetail);
			}

			//Remove it.
			// (c.haag 2008-08-05 11:39) - PLID 30799 - The second parameter is TRUE, which means we are
			// removing the EMN in response to a spawning action. Therefore, permissions will not be checked.
			if(!RemoveEMN(pEMN, TRUE)) {
				//TODO - Does anything happen if this fails?
				// (a.walling 2007-09-25 10:04) - PLID 27503 - We should ASSERT if we fail; any failure
				// conditions should have been handled previous to the NXM_PRE_DELETE_EMN message
				ASSERT(FALSE);
			}

			if(GetInterface() != NULL) {
				// (z.manning, 10/08/2007) - PLID 27662 - Tell the interface about any procedures we've deleted
				// as a result of removing this EMN so that the EMR description gets updated properly.
				for(int nProcIndex = 0; nProcIndex < pEMN->GetProcedureCount(); nProcIndex++) {
					GetInterface()->SendMessage(NXM_POST_DELETE_EMN_PROCEDURE, (WPARAM)pEMN->GetProcedure(nProcIndex)->nID, (LPARAM)pEMN);
				}

				//TES 6/5/2008 - PLID 30196 - Tell our interface that the EMN has been deleted.
				GetInterface()->SendMessage(NXM_POST_DELETE_EMN, (WPARAM)pEMN);
			}
		}


		//We're done, the detail can be unspawned now.
		if(pSourceDetail) {
			pSourceDetail->SetAllowUnspawn(TRUE);
		}

		//We're done processing this action, so this action is now free to be processed again.
		UnlockAction(ea);

	} // for(int i = 0; i < nActionCnt; i++) {

	// (a.walling 2010-03-31 15:49) - PLID 38008 - Regardless of the 'skipped' state, we need to clear out the Source Action Info
	for(int i = 0; i < nActionCnt; i++) {
		CDoomedEmrObject deo = adeoDoomedObjects.GetAt(i);

		CEMN* pEMN = deo.GetEMN();
		pEMN->ClearSourceActionInfo();
	}
}

// (a.walling 2011-10-20 14:23) - PLID 46075 - Clean up interaction with external interfaces
void CEMR::SetInterface(CEmrTreeWnd* pEmrTreeWnd)
{
	m_pInterface = pEmrTreeWnd;
}

void CEMR::LockSpawning()
{
	ASSERT(m_nSpawnLocks >= 0);
	m_nSpawnLocks++;
}

void CEMR::UnlockSpawning()
{
	ASSERT(m_nSpawnLocks >= 1);
	m_nSpawnLocks--;

	// (a.walling 2007-10-26 09:08) - PLID 25548 - Increase the spawn depth, since we will process actions here
	IncreaseEmrActionsDepth();

	if(m_nSpawnLocks == 0) {
		// (c.haag 2008-06-26 12:41) - PLID 27549 - This function now contains all the code
		ProcessPendingActions(m_arPendingActions);
	}

	// (j.jones 2007-03-13 17:37) - PLID 25193 - once all unlocks are done,
	// see if any of our EMNs are pending completion, and if so, complete them
	// (z.manning 2008-12-12 14:36) - PLID 32427 - Not so fast-- we may also have locks on handling
	// detail changes in which case we should not call PostInitialLoad on this EMN yet. I added
	// NeedToCallPostInitialLoad to handle the logic for calling PostInitialLoad only if ready.
	for(int i=0; i<m_arypEMNs.GetSize(); i++) {
		CEMN *pEMN = (CEMN*)m_arypEMNs.GetAt(i);
		if(pEMN->NeedToCallPostInitialLoad()) {
			pEMN->PostInitialLoad();
		}
	}

	// (a.walling 2007-10-26 09:10) - PLID 25548 - Decrease our spawn depth, which will call ProcessEmrActionsComplete if necessary
	DecreaseEmrActionsDepth();
}

// (z.manning 2011-03-04 16:27) - PLID 42682
void CEMR::ClearAllPendingActions()
{
	for(int nPendingActionIndex = m_arPendingActions.GetSize() - 1; nPendingActionIndex >= 0; nPendingActionIndex--)
	{
		PendingAction *pa = m_arPendingActions.GetAt(nPendingActionIndex);
		delete pa;
	}
	m_arPendingActions.RemoveAll();
}

// (c.haag 2008-06-26 12:46) - PLID 27549 - This constitutes the bulk of code that existed in CEMR::UnlockSpawning.
// We pass in a reference to an array of pending actions, and optionally a pointer to a CEMNLoader object if we
// happen to be spawning mint items such that the source detail itself is a spawned mint item; it's necessary for
// proper topic positioning.
void CEMR::ProcessPendingActions(CArray<PendingAction*,PendingAction*>& arPendingActions,
								 CEMNLoader* pEMNSpawnedMintLoader /* = NULL */)
{
	// (j.jones 2007-03-13 17:22) - PLID 25193 - track that we are now spawning
	// the locked actions (a reference count)
	m_nUnlockSpawnsRefCount++;

	//We're done!  Process everything.
	//Just for extra safety, let's make a local copy and use that.
	//DRT 8/29/2007 - PLID 27225 - We now have the capability to pass in an array of actions, per type, to do the 
	//	spawning.  That can be more efficient than processing 1 action at a time.
	//This has to be broken up per source EMN to properly call ProcessEmrAction().  So we're going to need to 
	//	map <source EMN> -> <all 7 types of actions>, for each source EMN.
	CMap<CEMN*, CEMN*, CArray<PendingAction*,PendingAction*>*, CArray<PendingAction*,PendingAction*>*> mapEMNToCptAction;
	CMap<CEMN*, CEMN*, CArray<PendingAction*,PendingAction*>*, CArray<PendingAction*,PendingAction*>*> mapEMNToDiagAction;
	CMap<CEMN*, CEMN*, CArray<PendingAction*,PendingAction*>*, CArray<PendingAction*,PendingAction*>*> mapEMNToMedAction;
	CMap<CEMN*, CEMN*, CArray<PendingAction*,PendingAction*>*, CArray<PendingAction*,PendingAction*>*> mapEMNToProcAction;
	CMap<CEMN*, CEMN*, CArray<PendingAction*,PendingAction*>*, CArray<PendingAction*,PendingAction*>*> mapEMNToEmrItemAction;
	CMap<CEMN*, CEMN*, CArray<PendingAction*,PendingAction*>*, CArray<PendingAction*,PendingAction*>*> mapEMNToMintItemAction;
	CMap<CEMN*, CEMN*, CArray<PendingAction*,PendingAction*>*, CArray<PendingAction*,PendingAction*>*> mapEMNToTodoAction; // (c.haag 2008-06-24 16:23) - PLID 17244 - Todo actions
	CMap<CEMN*, CEMN*, CArray<PendingAction*,PendingAction*>*, CArray<PendingAction*,PendingAction*>*> mapEMNToLabAction; // (z.manning 2008-10-27 17:04) - PLID 21094 - Lab actions
	//Mints are handled by the EMR and just need a list of pending actions, the source EMN doesn't matter.
	CArray<PendingAction*,PendingAction*> aryPendingActions_eaoMint;

	// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
	int i = 0;

	for(i = 0; i < arPendingActions.GetSize(); i++)
	{
		PendingAction *pa = arPendingActions[i];

		// (z.manning 2011-03-04 18:20) - PLID 42682 - If we've already processed this action then just skip over it.
		if(pa->bProcessed) {
			continue;
		}

		CEMN *pSourceEmn = pa->pSourceEmn;

		//Pulled out of CEMR::ProcessEmrAction
		// (z.manning, 01/31/2007) - PLID 24468 - If we weren't passed a source EMN, find it from the source detail
		if(pSourceEmn == NULL) {
			if(pa->sai.pSourceDetail && pa->sai.pSourceDetail->m_pParentTopic) {
				pSourceEmn = pa->sai.pSourceDetail->m_pParentTopic->GetParentEMN();
			}
			else {
				ASSERT(FALSE);
			}
		}

		//Previously we called CEMR::ProcessEMRAction with these, which just returns if the action is locked.  Now we'll just
		//	skip adding it to an array if so.  Also check for the "ignore all actions" variable.  It behaved the same and just
		//	threw out any actions that came along.
		if(IsActionLocked(pa->ea) || m_bIgnoreActions) {
			//It is locked or we're ignoring!  This could be an infinite loop, and if it's locked, it means we're already spawning it, so just quit.
		}
		else {
			switch(pa->ea.eaoDestType) {
			case eaoCpt:
				{
					//Find the array of cpt actions used by this source EMN
					CArray<PendingAction*,PendingAction*> *pary;
					if(!mapEMNToCptAction.Lookup(pSourceEmn, pary)) {
						//array not found, we must allocate one
						pary = new CArray<PendingAction*,PendingAction*>;
						mapEMNToCptAction.SetAt(pSourceEmn, pary);
					}
					pary->Add(pa);
				}
				break;
			// (b.savon 2014-07-14 11:21) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
			//case eaoDiag:
			case eaoDiagnosis:
				{
					//Find the array of cpt actions used by this source EMN
					CArray<PendingAction*,PendingAction*> *pary;
					if(!mapEMNToDiagAction.Lookup(pSourceEmn, pary)) {
						//array not found, we must allocate one
						pary = new CArray<PendingAction*,PendingAction*>;
						mapEMNToDiagAction.SetAt(pSourceEmn, pary);
					}
					pary->Add(pa);
				}
				break;
			case eaoEmrItem:
				{
					//Find the array of cpt actions used by this source EMN
					CArray<PendingAction*,PendingAction*> *pary;
					if(!mapEMNToEmrItemAction.Lookup(pSourceEmn, pary)) {
						//array not found, we must allocate one
						pary = new CArray<PendingAction*,PendingAction*>;
						mapEMNToEmrItemAction.SetAt(pSourceEmn, pary);
					}
					pary->Add(pa);
				}
				break;
			case eaoProcedure:
				{
					//Find the array of cpt actions used by this source EMN
					CArray<PendingAction*,PendingAction*> *pary;
					if(!mapEMNToProcAction.Lookup(pSourceEmn, pary)) {
						//array not found, we must allocate one
						pary = new CArray<PendingAction*,PendingAction*>;
						mapEMNToProcAction.SetAt(pSourceEmn, pary);
					}
					pary->Add(pa);
				}
				break;
			case eaoMedication:
				{
					//Find the array of cpt actions used by this source EMN
					CArray<PendingAction*,PendingAction*> *pary;
					if(!mapEMNToMedAction.Lookup(pSourceEmn, pary)) {
						//array not found, we must allocate one
						pary = new CArray<PendingAction*,PendingAction*>;
						mapEMNToMedAction.SetAt(pSourceEmn, pary);
					}
					pary->Add(pa);
				}
				break;
			case eaoMintItems:
				{
					//Find the array of cpt actions used by this source EMN
					CArray<PendingAction*,PendingAction*> *pary;
					if(!mapEMNToMintItemAction.Lookup(pSourceEmn, pary)) {
						//array not found, we must allocate one
						pary = new CArray<PendingAction*,PendingAction*>;
						mapEMNToMintItemAction.SetAt(pSourceEmn, pary);
					}
					pary->Add(pa);
				}
				break;
			case eaoMint:
				{
					//Mints are handled by the EMR, not EMN, and there's no real optimizations at this point.  Just call it right now.
					aryPendingActions_eaoMint.Add(pa);
				}
				break;
			case eaoTodo: // (c.haag 2008-06-24 16:23) - PLID 17244 - Todo actions
				{
					//Find the array of cpt actions used by this source EMN
					CArray<PendingAction*,PendingAction*> *pary;
					if(!mapEMNToTodoAction.Lookup(pSourceEmn, pary)) {
						//array not found, we must allocate one
						pary = new CArray<PendingAction*,PendingAction*>;
						mapEMNToTodoAction.SetAt(pSourceEmn, pary);
					}
					pary->Add(pa);
				}
				break;

			case eaoLab: // (z.manning 2008-10-27 17:05) - PLID 21094 - Lab actions
				{
					CArray<PendingAction*,PendingAction*> *pary;
					if(!mapEMNToLabAction.Lookup(pSourceEmn, pary)) {
						pary = new CArray<PendingAction*,PendingAction*>;
						mapEMNToLabAction.SetAt(pSourceEmn, pary);
					}
					pary->Add(pa);
				}
				break;

			default:
				//Shouldn't ever happen
				ASSERT(FALSE);
				break;
			}
		}
	}
	
	// (z.manning 2011-03-04 16:24) - PLID 42682 - Do not clear out all pending actions anymore. We need to keep them
	// in memory in case we need to update them (such as their SourceActionInfo) during the potential chained spawning
	// that may occur during an inital load. Instead, flag them as processed.
	for(int nPendingActionIndex = 0; nPendingActionIndex < arPendingActions.GetSize(); nPendingActionIndex++)
	{
		PendingAction *pa = arPendingActions.GetAt(nPendingActionIndex);
		pa->bProcessed = TRUE;
	}

	//DRT 8/29/2007 - PLID 27225 - Now that we have our actions split into type per source EMN, we can just call each type individually.
	//	Note that now we will process the skipped topics after all are handled, instead of after each one.  This is how
	//	the rest of the program behaves.
	//We will do these by spawning all of each type for each EMN... so all cpt actions, then all diag... etc
	//Previously CEMR::ProcessEmrAction() called LockAction() and UnlockAction() around each individual action.  We will continue that
	//	tradition by moving those calls to the ProcessEmrAction_eao<Type> functions.  This does mean that actions may be locked multiple
	//	times, but the existing code seems to be setup to handle that.

	//1)  CPT Actions
	POSITION pos = mapEMNToCptAction.GetStartPosition();
	while(pos) {
		CEMN *pSourceEMN = NULL;
		CArray<PendingAction*,PendingAction*> *pary = NULL;

		//This will get us, for the given source EMN, all pending actions.
		mapEMNToCptAction.GetNextAssoc(pos, pSourceEMN, pary);

		//We then must translate the array of pending actions into an ActionAndSource array.  While doing that, save the initial load state.  
		//	For some reason it's saved per-action, but there's no way you'd get here with some actions from the initial load and some not.
		BOOL bIsInitialLoad = FALSE;
		MFCArray<CActionAndSource> aryActionsToSpawn;
		for(int i = 0; i < pary->GetSize(); i++) {
			PendingAction *pa = pary->GetAt(i);
			CActionAndSource aas;
			aas.ea = pa->ea;
			aas.sai = pa->sai;
			aryActionsToSpawn.Add(aas);

			bIsInitialLoad = pa->bIsInitialLoad;
		}

		//Now we've got our arrays straightened out, pass it on to actually process
		pSourceEMN->ProcessEmrAction_eaoCpt(&aryActionsToSpawn, bIsInitialLoad, NULL, NULL);

		// (c.haag 2008-08-14 12:47) - PLID 30725 - Now that the CPT codes have all been spawned, process problem actions
		if (!m_bIsTemplate) {
			for(int i = 0; i < pary->GetSize(); i++) {
				PendingAction *pa = pary->GetAt(i);
				pSourceEMN->ProcessEmrProblemActions(pa->ea, pa->sai, NULL);
			}
		}

		//DRT 9/11/2007 - We no longer need the array
		delete pary;
	}

	//2)  Diag Actions
	pos = mapEMNToDiagAction.GetStartPosition();
	while(pos) {
		CEMN *pSourceEMN = NULL;
		CArray<PendingAction*,PendingAction*> *pary = NULL;

		//This will get us, for the given source EMN, all pending actions.
		mapEMNToDiagAction.GetNextAssoc(pos, pSourceEMN, pary);

		//We then must translate the array of pending actions into an ActionAndSource array.  While doing that, save the initial load state.  
		//	For some reason it's saved per-action, but there's no way you'd get here with some actions from the initial load and some not.
		BOOL bIsInitialLoad = FALSE;
		MFCArray<CActionAndSource> aryActionsToSpawn;
		for(int i = 0; i < pary->GetSize(); i++) {
			PendingAction *pa = pary->GetAt(i);
			CActionAndSource aas;
			aas.ea = pa->ea;
			aas.sai = pa->sai;
			aryActionsToSpawn.Add(aas);

			bIsInitialLoad = pa->bIsInitialLoad;
		}

		//Now we've got our arrays straightened out, pass it on to actually process
		pSourceEMN->ProcessEmrAction_eaoDiagnosis(&aryActionsToSpawn, bIsInitialLoad, NULL, NULL);

		// (c.haag 2008-08-14 12:47) - PLID 30725 - Now that the diagnosis codes have all been spawned, process problem actions
		if (!m_bIsTemplate) {
			for(int i = 0; i < pary->GetSize(); i++) {
				PendingAction *pa = pary->GetAt(i);
				pSourceEMN->ProcessEmrProblemActions(pa->ea, pa->sai, NULL);
			}
		}

		//DRT 9/11/2007 - We no longer need the array
		delete pary;
	}

	//3)  Medication Actions
	pos = mapEMNToMedAction.GetStartPosition();
	while(pos) {
		CEMN *pSourceEMN = NULL;
		CArray<PendingAction*,PendingAction*> *pary = NULL;

		//This will get us, for the given source EMN, all pending actions.
		mapEMNToMedAction.GetNextAssoc(pos, pSourceEMN, pary);

		//We then must translate the array of pending actions into an ActionAndSource array.  While doing that, save the initial load state.  
		//	For some reason it's saved per-action, but there's no way you'd get here with some actions from the initial load and some not.
		BOOL bIsInitialLoad = FALSE;
		MFCArray<CActionAndSource> aryActionsToSpawn;
		for(int i = 0; i < pary->GetSize(); i++) {
			PendingAction *pa = pary->GetAt(i);
			CActionAndSource aas;
			aas.ea = pa->ea;
			aas.sai = pa->sai;
			aryActionsToSpawn.Add(aas);

			bIsInitialLoad = pa->bIsInitialLoad;
		}

		//Now we've got our arrays straightened out, pass it on to actually process
		pSourceEMN->ProcessEmrAction_eaoMedication(&aryActionsToSpawn, bIsInitialLoad, NULL, NULL);

		// (c.haag 2008-08-14 12:47) - PLID 30725 - Now that the medications have all been spawned, process problem actions
		if (!m_bIsTemplate) {
			for(int i = 0; i < pary->GetSize(); i++) {
				PendingAction *pa = pary->GetAt(i);
				pSourceEMN->ProcessEmrProblemActions(pa->ea, pa->sai, NULL);
			}
		}

		//DRT 9/11/2007 - We no longer need the array
		delete pary;
	}

	//4)  Procedure Actions
	pos = mapEMNToProcAction.GetStartPosition();
	while(pos) {
		CEMN *pSourceEMN = NULL;
		CArray<PendingAction*,PendingAction*> *pary = NULL;

		//This will get us, for the given source EMN, all pending actions.
		mapEMNToProcAction.GetNextAssoc(pos, pSourceEMN, pary);

		//We then must translate the array of pending actions into an ActionAndSource array.  While doing that, save the initial load state.  
		//	For some reason it's saved per-action, but there's no way you'd get here with some actions from the initial load and some not.
		BOOL bIsInitialLoad = FALSE;
		MFCArray<CActionAndSource> aryActionsToSpawn;
		for(int i = 0; i < pary->GetSize(); i++) {
			PendingAction *pa = pary->GetAt(i);
			CActionAndSource aas;
			aas.ea = pa->ea;
			aas.sai = pa->sai;
			aryActionsToSpawn.Add(aas);

			bIsInitialLoad = pa->bIsInitialLoad;
		}

		//Now we've got our arrays straightened out, pass it on to actually process
		pSourceEMN->ProcessEmrAction_eaoProcedure(&aryActionsToSpawn, bIsInitialLoad, NULL, NULL);

		// (c.haag 2008-08-14 14:15) - PLID 30725 - Now process problem actions
		if (!m_bIsTemplate) {
			for(int i = 0; i < pary->GetSize(); i++) {
				PendingAction *pa = pary->GetAt(i);
				pSourceEMN->ProcessEmrProblemActions(pa->ea, pa->sai, NULL);
			}
		}

		//DRT 9/11/2007 - We no longer need the array
		delete pary;
	}

	//5)  Emr Item Actions
	pos = mapEMNToEmrItemAction.GetStartPosition();
	while(pos) {
		CEMN *pSourceEMN = NULL;
		CArray<PendingAction*,PendingAction*> *pary = NULL;

		//This will get us, for the given source EMN, all pending actions.
		mapEMNToEmrItemAction.GetNextAssoc(pos, pSourceEMN, pary);

		// (c.haag 2008-08-14 14:02) - PLID 30725 - We also need to process problem actions. Because
		// those depend on newly created EMR Items given to us by ProcessEmrAction_eaoEmrItem, we need 
		// to put the process function inside the loop.

		//We then must translate the array of pending actions into an ActionAndSource array.  While doing that, save the initial load state.  
		//	For some reason it's saved per-action, but there's no way you'd get here with some actions from the initial load and some not.
		BOOL bIsInitialLoad = FALSE;
		for(int i = 0; i < pary->GetSize(); i++) {
			MFCArray<CActionAndSource> aryActionsToSpawn;
			PendingAction *pa = pary->GetAt(i);
			CActionAndSource aas;
			aas.ea = pa->ea;
			aas.sai = pa->sai;
			aryActionsToSpawn.Add(aas);
			bIsInitialLoad = pa->bIsInitialLoad;
			
			CEMNDetail* pNewDetail = NULL;
			pSourceEMN->ProcessEmrAction_eaoEmrItem(&aryActionsToSpawn, bIsInitialLoad, NULL, NULL, &pNewDetail);
			if (NULL != pNewDetail) {
				pSourceEMN->ProcessEmrProblemActions(pa->ea, pa->sai, pNewDetail);
			}
		}

		//DRT 9/11/2007 - We no longer need the array
		delete pary;
	}

	//6)  Mint Item Actions
	CArray<SkippedTopic,SkippedTopic&> arSkippedTopics;
	pos = mapEMNToMintItemAction.GetStartPosition();
	while(pos) {
		CEMN *pSourceEMN = NULL;
		CArray<PendingAction*,PendingAction*> *pary = NULL;

		//This will get us, for the given source EMN, all pending actions.
		mapEMNToMintItemAction.GetNextAssoc(pos, pSourceEMN, pary);

		//We then must translate the array of pending actions into an ActionAndSource array.  While doing that, save the initial load state.  
		//	For some reason it's saved per-action, but there's no way you'd get here with some actions from the initial load and some not.
		BOOL bIsInitialLoad = FALSE;
		MFCArray<CActionAndSource> aryActionsToSpawn;
		for(int i = 0; i < pary->GetSize(); i++) {
			PendingAction *pa = pary->GetAt(i);
			CActionAndSource aas;
			aas.ea = pa->ea;
			aas.sai = pa->sai;
			aryActionsToSpawn.Add(aas);

			bIsInitialLoad = pa->bIsInitialLoad;
		}

		//Now we've got our arrays straightened out, pass it on to actually process
		pSourceEMN->ProcessEmrAction_eaoMintItems(&aryActionsToSpawn, arSkippedTopics, bIsInitialLoad, NULL, NULL, NULL, pEMNSpawnedMintLoader);

		// (c.haag 2008-08-14 14:15) - PLID 30725 - Now process problem actions
		if (!m_bIsTemplate) {
			for(int i = 0; i < pary->GetSize(); i++) {
				PendingAction *pa = pary->GetAt(i);
				pSourceEMN->ProcessEmrProblemActions(pa->ea, pa->sai, NULL);
			}
		}

		//DRT 9/11/2007 - We no longer need the array
		delete pary;
	}

	//7) (c.haag 2008-06-24 16:22) - PLID 17244 - Todo actions
	pos = mapEMNToTodoAction.GetStartPosition();
	while(pos) {
		CEMN *pSourceEMN = NULL;
		CArray<PendingAction*,PendingAction*> *pary = NULL;

		//This will get us, for the given source EMN, all pending actions.
		mapEMNToTodoAction.GetNextAssoc(pos, pSourceEMN, pary);

		//We then must translate the array of pending actions into an ActionAndSource array.  While doing that, save the initial load state.  
		//	For some reason it's saved per-action, but there's no way you'd get here with some actions from the initial load and some not.
		BOOL bIsInitialLoad = FALSE;
		MFCArray<CActionAndSource> aryActionsToSpawn;
		for(int i = 0; i < pary->GetSize(); i++) {
			PendingAction *pa = pary->GetAt(i);
			CActionAndSource aas;
			aas.ea = pa->ea;
			aas.sai = pa->sai;
			aryActionsToSpawn.Add(aas);

			bIsInitialLoad = pa->bIsInitialLoad;
		}

		delete pary;

		//Now we've got our arrays straightened out, pass it on to actually process
		pSourceEMN->ProcessEmrAction_eaoTodo(&aryActionsToSpawn, bIsInitialLoad, NULL, NULL);
	}

	//8) (z.manning 2008-10-27 17:07) - PLID21094 - Lab actions
	pos = mapEMNToLabAction.GetStartPosition();
	while(pos)
	{
		CEMN *pSourceEMN = NULL;
		CArray<PendingAction*,PendingAction*> *pary = NULL;

		mapEMNToLabAction.GetNextAssoc(pos, pSourceEMN, pary);

		BOOL bIsInitialLoad = FALSE;
		MFCArray<CActionAndSource> aryActionsToSpawn;
		for(int nPendingActionIndex = 0; nPendingActionIndex < pary->GetSize(); nPendingActionIndex++)
		{
			PendingAction *pa = pary->GetAt(nPendingActionIndex);
			CActionAndSource aas;
			aas.ea = pa->ea;
			aas.sai = pa->sai;
			aryActionsToSpawn.Add(aas);

			bIsInitialLoad = pa->bIsInitialLoad;
		}

		pSourceEMN->ProcessEmrAction_eaoLab(&aryActionsToSpawn, bIsInitialLoad, NULL);

		delete pary;
	}

	// (a.walling 2007-10-26 09:20) - PLID 25548 - Collect any new topics
	CArray<CEMRTopic*, CEMRTopic*> arNewSkippedTopics;
	//Now process any skipped topics that came as a result of the above mint item spawning
	for(int j = 0; j < arSkippedTopics.GetSize(); j++) {
		// (z.manning, 01/31/2007) - PLID 24468 - If we don't have a source detail, then it should not be possible
		// to spawn something that results in skipping a topic.
		CEMNDetail *pSourceDetail = arSkippedTopics.GetAt(j).sai.pSourceDetail;
		if(pSourceDetail) {
			CEMRTopic* pNewSkippedTopic = pSourceDetail->m_pParentTopic->GetParentEMN()->ProcessSkippedTopic(pSourceDetail, arSkippedTopics[j]);
			if (pNewSkippedTopic) {
				arNewSkippedTopics.Add(pNewSkippedTopic);
			}
		}
		else {
			ASSERT(arSkippedTopics.GetSize() == 0);
		}
	}
	// (a.walling 2007-10-26 09:21) - PLID 25548 - Now tell the interface to update the preview
	if (arNewSkippedTopics.GetSize() > 0 && GetInterface() != NULL) {
		GetInterface()->SendMessage(NXM_EMR_TOPIC_ADD_PREVIEW, (WPARAM)&arNewSkippedTopics, (LPARAM)NULL);
	}
	arNewSkippedTopics.RemoveAll();

	//7)  Mint Item Actions
	for(i = 0; i < aryPendingActions_eaoMint.GetSize(); i++) {
		PendingAction *pa = aryPendingActions_eaoMint.GetAt(i);
		ProcessEmrAction(pa->ea, pa->sai, arSkippedTopics, pa->bIsInitialLoad, NULL, pa->pSourceEmn);
	}



	/*DRT 8/29/2007 - PLID 27225 - This was the old behavior, spawning 1 action at a time
	for(i = 0; i < arPendingActions.GetSize(); i++) {
		//TES 2/20/2006 - Process any skipped topics.
		CArray<SkippedTopic,SkippedTopic&> arSkippedTopics;
		ProcessEmrAction(arPendingActions[i].ea, arPendingActions[i].pSourceDetail, arSkippedTopics, arPendingActions[i].bIsInitialLoad, NULL, arPendingActions[i].pSourceEmn);
		// (z.manning, 01/31/2007) - PLID 24468 - If we don't have a source detail, then it should not be possible
		// to spawn something that results in skipping a topic.
		if(arPendingActions[i].pSourceDetail) {
			for(int j = 0; j < arSkippedTopics.GetSize(); j++) {
				arPendingActions[i].pSourceDetail->m_pParentTopic->GetParentEMN()->ProcessSkippedTopic(arPendingActions[i].pSourceDetail, arSkippedTopics[j]);
			}
		}
		else {
			ASSERT(arSkippedTopics.GetSize() == 0);
		}
	}
	*/

	//TES 1/24/2007 - PLID 24377 - We also need to apply any EMR links that are pending.
	//Again, we'll use a local copy.
	/*
	CArray<CEMNDetail*,CEMNDetail*> arPendingDetails;
	for(i = 0; i < m_arDetailsToBeLinked.GetSize(); i++) {
		arPendingDetails.Add(m_arDetailsToBeLinked[i]);
	}

	ApplyEmrLinksToBatch(arPendingDetails);
	*/

	ApplyEmrLinksToBatch(m_arDetailsToBeLinked);
	
	//m_arDetailsToBeLinked.RemoveAll();
	// (a.walling 2009-10-12 18:03) - PLID 36024 - Maintaining reference counts on details to be linked
	ClearDetailsToBeLinked();

	// (j.jones 2006-10-13 09:18) - PLID 22461 - moved office visit incrementing to the save process
	/*
	// (j.jones 2005-05-19 10:07) - once we are done spawning, warn
	// if any of the office visit codes were upgraded
	if(!m_bIsTemplate) {
		if(GetInterface()) {
			GetInterface()->SendMessage(NXM_PROCESS_EMR_OFFICE_VISITS);
		}
	}
	*/

	// (j.jones 2007-03-13 17:22) - PLID 25193 - track that we are finished
	// spawning the locked actions (decrement the ref. count)
	m_nUnlockSpawnsRefCount--;
}

// (a.walling 2009-10-12 18:02) - PLID 36024 - Maintaining reference counts on details to be linked
void CEMR::ClearDetailsToBeLinked()
{
	for (int i = 0; i < m_arDetailsToBeLinked.GetSize(); i++) {
		m_arDetailsToBeLinked.GetAt(i)->__Release("clear emr links");
	}

	m_arDetailsToBeLinked.RemoveAll();
}

// (c.haag 2008-06-26 12:11) - PLID 27549 - Called from ProcessEmrAction_eaoMintItems to search for
// any spawned multi-select details that were checked by default, and themselves spawn mint items;
// and then spawn those mint items using the proper topic hierarchy.
void CEMR::DetectAndSpawnChildMintItems(CEMNLoader* pEMNSpawnedMintLoader,
										CArray<CEMRTopic*,CEMRTopic*>& apNewTopics)
{
	// Search the EMR for pending actions that were invoked from a multi-select detail
	// within apNewTopics
	CArray<PendingAction*,PendingAction*> arPendingMintActions;
	CArray<long, long> arnPendingActionIndices;
	int i;
	for(i = 0; i < m_arPendingActions.GetSize(); i++) {
		PendingAction *pa = m_arPendingActions[i];
		// (a.walling 2010-05-17 16:23) - PLID 38707 - Needs to handle other sources as well
		if (eaoMintItems == pa->ea.eaoDestType && (eaoEmrDataItem == pa->ea.eaoSourceType || eaoEmrTableDropDownItem == pa->ea.eaoSourceType || eaoSmartStamp == pa->ea.eaoSourceType || eaoEmrImageHotSpot == pa->ea.eaoSourceType))
		{
			CEMNDetail* pSrcDetail = pa->sai.pSourceDetail;
			BOOL bSourceDetailFound = FALSE;
			for (int nTopic=0; nTopic < apNewTopics.GetSize() && !bSourceDetailFound; nTopic++) {
				CEMRTopic* pTopic = apNewTopics[nTopic];
				const int nDetailCount = pTopic->GetEMNDetailCount();
				for (int nDetail=0; nDetail < nDetailCount && !bSourceDetailFound; nDetail++) {

					if (pTopic->GetDetailByIndex(nDetail) == pSrcDetail) {
						// If we get here, we know that the action was spawned
						// from this detail.
						arPendingMintActions.Add(pa);
						arnPendingActionIndices.Add(i);
						bSourceDetailFound = TRUE;
					}

				} // for (int nDetail=0; nDetail < nDetailCount; nDetail++) {
			} // for (int nTopic=0; nTopic < apNewTopics.GetSize(); nTopic++) {
		}
	} // for(i = 0; i < m_arPendingActions.GetSize(); i++) {

	// When we get here, aPendingMintActions contains a list of all actions we want
	// to spawn right away using the information in pEMNSpawnedMintLoader to probably
	// build the spawning topic hierarchy
	if (0 == arPendingMintActions.GetSize()) {
		return; // Nothing to do
	}

	// This body of code effectively calls UnlockActions for a subset of pending actions
	// without actually unlocking anything or permanently changing any reference counts.
	IncreaseEmrActionsDepth();
	ProcessPendingActions(arPendingMintActions, pEMNSpawnedMintLoader);
	DecreaseEmrActionsDepth();

	for (i=arnPendingActionIndices.GetSize() - 1; i >= 0; i--) {
		int nIndex = arnPendingActionIndices[i];
		delete m_arPendingActions.GetAt(nIndex);
		m_arPendingActions.RemoveAt(nIndex);
	}
}

void CEMR::AddEMNDetailReference(CEMNDetail *pDetail)
{
	try {
		if (!IsEMNDetailReferenced(pDetail)) {	
			// (a.walling 2009-10-12 17:45) - PLID 36024
			pDetail->__AddRef("AddEMNDetailReference");

			m_arypReferencedEMNDetails.Add(pDetail);	
		}
	}NxCatchAll("Error in CEMR::AddEMNDetailReference()");
}

void CEMR::RemoveEMNDetailReference(CEMNDetail *pDetail)
{
	try {
		for (int i = 0; i < m_arypReferencedEMNDetails.GetSize(); i++) {
			if (pDetail == m_arypReferencedEMNDetails.GetAt(i)) {
				// We've found the detail, so now let's see if it's pending to be deleted

				// (a.walling 2009-10-12 16:05) - PLID 36024 - We have our own reference now, too.
				m_arypReferencedEMNDetails.GetAt(i)->__Release("RemoveEMNDetailReference");

				if (IsEMNDetailPendingForDeletion(pDetail)) {
					// This detail needs to be deleted, so let's delete it
					// (c.haag 2007-05-24 17:21) - PLID 26095 - Use Release() in case the
					// preloader is using it
					//m_arypReferencedEMNDetails.GetAt(i)->Release();
					// (a.walling 2009-10-12 16:05) - PLID 36024
					CEMNDetail* pTargetDetail = m_arypReferencedEMNDetails.GetAt(i);
					pTargetDetail->__Release("RemoveEMNDetailReference was pending");
					RemoveEMNDetailFromPendingDeletion(pTargetDetail);
				}

				// Remove the detail from the referenced details array
				m_arypReferencedEMNDetails.RemoveAt(i);
				i--;
			}
		}
	}NxCatchAll("Error in CEMR::RemoveEMNDetailReference()");
}

BOOL CEMR::IsEMNDetailReferenced(CEMNDetail *pDetail)
{
	try {
		for (int i = 0; i < m_arypReferencedEMNDetails.GetSize(); i++) {
			if(pDetail == m_arypReferencedEMNDetails.GetAt(i)) {
				return TRUE;
			}
		}
	}NxCatchAll("Error in CEMR::IsEMNDetailReferenced()");
	return FALSE;
}

void CEMR::DeleteEMNDetail(CEMNDetail *pDetail)
{
	try {

		// (j.jones 2009-06-11 09:13) - PLID 34301 - the detail object isn't always "deleted" yet in this function,
		// simply an attempt is made to release it, but we have to remove is problems now, so do that first
		for(int nProblemLinkIndex = 0; nProblemLinkIndex < pDetail->m_apEmrProblemLinks.GetSize(); nProblemLinkIndex++) {
			CEmrProblemLink *pProblemLink = pDetail->m_apEmrProblemLinks.GetAt(nProblemLinkIndex);
			if(pProblemLink != NULL) {
				pDetail->m_apEmrProblemLinks.RemoveAt(nProblemLinkIndex--);
				delete pProblemLink;
			}
		}

		if (IsEMNDetailReferenced(pDetail)) {
			// We can't delete the detail yet because it is referenced, so let's add it to the pending array
			// if it isn't already in it
			if (!IsEMNDetailPendingForDeletion(pDetail)) {
				m_arypPendingDetailsToDelete.Add(pDetail);
			}
		}
		else {
			// This detail isn't even referenced, so let's just go ahead and delete it now
			// (c.haag 2007-05-24 17:18) - PLID 26095 - Release it because the preloader
			// may use it
			//pDetail->Release();
			// (a.walling 2009-10-12 16:05) - PLID 36024
			pDetail->__Release("DeleteEMNDetail");
		}
	}NxCatchAll("Error in CEMR::DeleteEMNDetail()");
}

BOOL CEMR::IsEMNDetailPendingForDeletion(CEMNDetail *pDetail)
{
	try {
		for (int i = 0; i < m_arypPendingDetailsToDelete.GetSize(); i++) {
			if(pDetail == m_arypPendingDetailsToDelete.GetAt(i)) {
				return TRUE;
			}
		}
	}NxCatchAll("Error in CEMR::IsEMNDetailPendingForDeletion()");
	return FALSE;
}

// (a.walling 2009-10-12 18:07) - PLID 36024 - Maintaining reference counts on details pending deletion
void CEMR::RemoveEMNDetailFromPendingDeletion(CEMNDetail *pDetail)
{
	try {
		for (int i = 0; i < m_arypPendingDetailsToDelete.GetSize(); i++) {
			if(pDetail == m_arypPendingDetailsToDelete.GetAt(i)) {
				m_arypPendingDetailsToDelete.RemoveAt(i);
				return;
			}
		}
	}NxCatchAll("Error in CEMR::RemoveEMNDetailFromPendingDeletion()");
}

void CEMR::LockAction(const EmrAction &ea)
{
	m_arLockedActionIDs.Add(ea.nID);
}

void CEMR::UnlockAction(const EmrAction &ea)
{
	for(int i = 0; i < m_arLockedActionIDs.GetSize(); i++) {
		if(ea.nID == m_arLockedActionIDs[i]) {
			m_arLockedActionIDs.RemoveAt(i);
			i--;
		}
	}
}

BOOL CEMR::IsActionLocked(const EmrAction &ea)
{
	for(int i = 0; i < m_arLockedActionIDs.GetSize(); i++) {
		if(ea.nID == m_arLockedActionIDs[i]) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CEMR::PropagateNewID(long nID, EmrSaveObjectType esotSaveType, long nObjectPtr, long &nAuditTransactionID, CEMN** pFoundOnEMN)
{
	//test to see if the nObjectPtr and esotSaveType match this object,
	//if so, assign the ID and return TRUE,
	//otherwise, propagate into the children of this item

	*pFoundOnEMN = NULL;

	//is it this object?
	if(esotSaveType == esotEMR && (CEMR*)nObjectPtr == this) {
		//we have a winner!
		// (c.haag 2007-06-19 17:40) - PLID 26388 - Fail if an ID already exists
		if (m_nEMRID != -1) {
			ASSERT(FALSE); ThrowNxException("Called CEMR::PropagateNewID on an EMR with an existing ID! (Current = %d nID = %d)", m_nEMRID, nID);
		}
		m_nEMRID = nID;

		// (j.jones 2008-07-29 17:33) - PLID 30880 - if any problems are on this EMR, update their regarding IDs
		// (z.manning 2009-05-22 14:51) - PLID 34332 - Handle problem links.
		for(int i = 0; i < m_apEmrProblemLinks.GetSize(); i++) {
			CEmrProblemLink *pProblemLink = m_apEmrProblemLinks.GetAt(i);
			if(pProblemLink != NULL && pProblemLink->GetType() == eprtEmrEMR) {
				pProblemLink->SetRegardingID(nID);
			}
		}

		return TRUE;
	}
	else {
		//check all EMNs
		BOOL bFound = FALSE;
		for(int i = 0; i < m_arypEMNs.GetSize() && !bFound; i++) {
			if(m_arypEMNs.GetAt(i)->PropagateNewID(nID, esotSaveType, nObjectPtr, nAuditTransactionID)) {
				bFound = TRUE;
				*pFoundOnEMN = m_arypEMNs.GetAt(i);
			}
		}

		if(esotSaveType == esotProblem)
		{
			// (z.manning 2009-05-22 14:43) - PLID 28554 - Update the master list of problems associated
			// with this EMR.
			for(int i = 0; i < m_apEmrProblemHeap.GetSize(); i++)
			{
				CEmrProblem *pProblem = m_apEmrProblemHeap.GetAt(i);
				if(pProblem == (CEmrProblem*)nObjectPtr)
				{
					// (c.haag 2007-06-19 17:38) - PLID 26388 - Fail if an ID already exists
					if(pProblem->m_nID > 0) {
						ThrowNxException("Called CEMR::PropagateNewID on an existing problem for a EMR! (Problem ID: %li)", pProblem->m_nID);
					}

					pProblem->m_nID = nID;
					// (z.manning, 09/06/2006) - PLID 22400 - Need to audit this here so we have a problem ID
					// rather than auditing it with a -1 record ID which would make it impossible to track.
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					// (z.manning 2009-05-27 17:20) - PLID 34340 - Moved the audit logic to a member function
					// of CEmrProblem.
					pProblem->AuditNew(nAuditTransactionID);
					bFound = TRUE;
				}
			}
		}
		else if(esotSaveType == esotProblemLink)
		{
			// (z.manning 2009-05-22 15:14) - PLID 34332 - Need to handle problem links
			for(int nProblemLinkIndex = 0; nProblemLinkIndex < m_apEmrProblemLinks.GetSize(); nProblemLinkIndex++)
			{
				CEmrProblemLink *pProblemLink = m_apEmrProblemLinks.GetAt(nProblemLinkIndex);
				if(pProblemLink == (CEmrProblemLink*)nObjectPtr)
				{
					if(pProblemLink->GetID() > 0) {
						ThrowNxException("Called CEMR::PropagateNewID on an existing problem link for a EMR! (Problem link ID: %li)", pProblemLink->GetID());
					}

					pProblemLink->SetID(nID);
					pProblemLink->Audit(aeiEMNProblemLinkCreated, nAuditTransactionID, GetExistingPatientName(GetPatientID()));
					bFound = TRUE;
				}
			}
		}

		return bFound;
	}
}

// (a.walling 2010-03-31 11:23) - PLID 38006 - Now we need to ensure any cross-EMN IDs are updated
void CEMR::PropagateCrossEMNIDs()
{
	try {		
		for(int i=0; i < m_arypEMNs.GetSize(); i++) {
			CEMN* pEMN = m_arypEMNs[i];
			_ASSERTE(pEMN != NULL); // an EMN should never exist in the array that is NULL!
			if (pEMN) {
				pEMN->PropagateCrossEMNIDs();
			}
		}
	} NxCatchAll("Error in CEMR::PropagateCrossEMNIDs()");
}

void CEMR::SetSaved(BOOL bIsPostLoad /*= FALSE*/)
{
	// (j.jones 2007-02-06 13:47) - PLID 24509 - for the purposes of time tracking,
	// anytime we save a change, track that a change was indeed made
	if(!bIsPostLoad && IsEMRUnsaved()) {
		m_bChangesMadeThisSession = TRUE;
	}

	m_bIsUnsaved = FALSE;

	for(int i = 0; i < m_arypEMNs.GetSize(); i++) {
		// (a.walling 2008-06-09 13:09) - PLID 22049 - Don't do so if they are not writable
		if (bIsPostLoad || m_arypEMNs[i]->IsWritable()) {
			m_arypEMNs[i]->SetSaved(bIsPostLoad);
		}
	}
}

// (j.jones 2008-07-22 16:05) - PLID 30789 - added SetUnsaved
void CEMR::SetUnsaved()
{
	m_bIsUnsaved = TRUE;
}

// (j.jones 2012-01-31 16:21) - PLID 47878 - added a bIsPostLoad parameter
void CEMR::SetSavedNoPropagate(BOOL bIsPostLoad /*= FALSE*/)
{
	// (j.jones 2012-01-31 15:33) - PLID 47878 - for the purposes of time tracking,
	// anytime we save a change, track that a change was indeed made
	if(!bIsPostLoad && IsEMRUnsaved()) {
		m_bChangesMadeThisSession = TRUE;
	}

	m_bIsUnsaved = FALSE;
}

//TES 5/20/2008 - PLID 27905 - This is sometimes called when procedures are about to be deleted; if so, pass in the EMN
// they're being deleted from and this will only return true if the procedure is on some other EMN.
BOOL CEMR::IsProcedureInEMR(long nProcedureID, CEMN* pExcludingEmn /*= NULL*/)
{
	for(int i = 0; i < m_arypEMNs.GetSize(); i++) {
		CEMN *pEMN = m_arypEMNs.GetAt(i);
		//TES 5/20/2008 - PLID 27905 - Ignore the EMN we were asked to exclude.
		if(pEMN != pExcludingEmn) {
			for(int j=0; j<pEMN->GetProcedureCount(); j++) {
				if(pEMN->GetProcedure(j)->nID == nProcedureID)
					return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CEMR::IsEMRUnsaved()
{
	//see if the description or status changed

	// (j.dinatale 2012-07-13 11:30) - PLID 51481 - check for deleted EMNs and unsaved EMNs first
	//TES 6/27/2006 - PLID 21236 - If any EMNs have been deleted, then we are unsaved.
	if(m_aryDeletedEMNs.GetSize() > 0) return TRUE;

	for(int i = 0; i < m_arypEMNs.GetSize(); i++) {
		if(m_arypEMNs[i]->IsUnsaved())
			return TRUE;
	}

	if(!m_bIsTemplate) {
		// (j.dinatale 2012-07-13 09:49) - PLID 51481 - if the EMN ID is -1 and there are no EMNs, intended to show the PIC, 
		//	and we havent deleted anything or have any unsaved EMNS... we can just say we are not unsaved...
		if(GetInterface() && GetInterface()->GetPicContainer()){
			bool bOpenedForPic = GetInterface()->GetPicContainer()->OpenedForPic();
			if(bOpenedForPic && m_nEMRID == -1 && !m_arypEMNs.GetSize())
				return FALSE;
		}

		if(m_bIsUnsaved)
			return TRUE;
	}

	return FALSE;
}

// (j.jones 2011-07-15 13:45) - PLID 42111 - takes in an image file name (could be a path),
// and returns TRUE if any Image detail on this EMR references it
BOOL CEMR::IsImageFileInUse(const CString strFileName)
{
	for(int i = 0; i < m_arypEMNs.GetSize(); i++) {
		if(m_arypEMNs[i]->IsImageFileInUse(strFileName))
			return TRUE;
	}

	return FALSE;
}

// (z.manning 2010-08-20 09:42) - PLID 40190 - Added connection pointer parameter
void CEMR::LoadTopic(ADODB::_Connection *lpCon, EMRTopicLoadInfo *pEtli)
{
	// (a.walling 2010-07-23 12:36) - PLID 39834
	LoadAllTopicsInfo* pLoadAllTopicsInfo = GetLoadAllTopicsInfo();

	if (!pLoadAllTopicsInfo) {
		ThrowNxException("GetLoadAllTopicsInfo returned NULL");
	}

	if(pEtli->hWnd) {
		//Grab our array.
		pLoadAllTopicsInfo->mtxTopics.Lock();
		//Add to the array.
		pLoadAllTopicsInfo->arTopics.Add(pEtli);
		//Let go of the array.
		pLoadAllTopicsInfo->mtxTopics.Unlock();
		//Tell our thread to load (if it isn't already).
		// (a.walling 2010-07-23 12:31) - PLID 39834 - No more thread messages. Use our special PostMessage to post to the thread's message window.
		m_pLoadTopicsThread->PostMessage(NXM_LOAD_TOPICS, NULL, NULL);
	}
	else {
		// (z.manning 2010-08-20 09:42) - PLID 40190 - Do not call GetRemoteData here is this function can be called
		// from a non-main thread
		LoadEMRTopic(pEtli, lpCon, this);
		//Remember that we need to clean this up.
		pLoadAllTopicsInfo->mtxTopics.Lock();
		pLoadAllTopicsInfo->arTopics.Add(pEtli);
		pLoadAllTopicsInfo->mtxTopics.Unlock();
	}
}

void CEMR::LoadTopicImmediate(EMRTopicLoadInfo *pTopic)
{
	//TES 2/21/2007 - PLID 24864 - If the topic is already completely loaded, we're done, don't bother with all the
	// thread overhead.
	if(pTopic->m_bCompletelyLoaded)
		return;

	EMRLOGINDENT(1,"CEMR::LoadTopicImmediate called for and processing topic ID %s", (pTopic) ? AsString(pTopic->nID) : "(null)"); // (c.haag 2010-05-19 9:04) - PLID 38759

	ASSERT(pTopic);
	//Create an event to wait on.
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	//Ask our thread to load the topic ASAP.
	// (a.walling 2010-07-23 12:31) - PLID 39834 - No more thread messages. Use our special PostMessage to post to the thread's message window.
	m_pLoadTopicsThread->PostMessage(NXM_LOAD_TOPIC_IMMEDIATE, (WPARAM)pTopic, (LPARAM)hEvent);
	//Now wait until it signals our event, meaning that the topic is loaded.
	WaitForSingleObject(hEvent, INFINITE);
	CloseHandle(hEvent);
	//The topic is now loaded, so we're done.

	EMRLOGINDENT(-1,"CEMR::LoadTopicImmediate END"); // (c.haag 2010-05-19 9:04) - PLID 38759
}

void CEMR::AuditCloseUnsaved()
{
	try {
		// (m.hancock 2006-10-06 17:01) - PLID 22302 - Audit closing an unsaved EMN.  We need to loop through all EMN's,
		// check if they're unsaved, and tell that EMN to audit that it is being closed without saving.
		for(int i = 0; i < m_arypEMNs.GetSize(); i++) {
			if(m_arypEMNs[i]->IsUnsaved()) {
				//Tell the EMN to audit that it is being closed without saving
				m_arypEMNs[i]->AuditCloseUnsaved();
			}
		}

		// (m.hancock 2006-11-28 11:31) - PLID 22302 - Audit closing an unsaved EMN.  We also need to audit if the EMN
		// was deleted, but we're now cancelling that delete by closing the EMR without saving.
		for(int j = 0; j < m_aryDeletedEMNs.GetSize(); j++) {
			//Tell the EMN to audit that it is being closed without saving
			m_aryDeletedEMNs[j]->AuditCloseUnsavedDeleted();
		}

	}NxCatchAll("Error in CEMR::AuditCloseUnsaved()");
}

//TES 1/23/2007 - PLID 24377 - Used when processing EMR Links, this function returns TRUE if any one of the EmrDataT.IDs
// in the array is selected on any detail in the EMR, otherwise it returns FALSE.
BOOL CEMR::IsAnyItemChecked(const CArray<long,long> &arDataIDs, const CArray<CEMNDetail*,CEMNDetail*> &arDetailsToIgnore)
{
	//Check each of our EMNs
	for(int i = 0; i < m_arypEMNs.GetSize(); i++) {
		if(m_arypEMNs[i]->IsAnyItemChecked(arDataIDs, arDetailsToIgnore)) return TRUE;
	}
	return FALSE;
}

// (c.haag 2011-05-19) - PLID 43696 - Populates a map with all EmrDataID's that correspond to checked-off single-select
// and multi-select list items. All details in mapDetailsToIgnore are ignored during the search.
void CEMR::GetAllCheckedItems(CMap<long,long,BOOL,BOOL>& mapDataIDs, const CMap<CEMNDetail*,CEMNDetail*,BOOL,BOOL> &mapDetailsToIgnore)
{
	for(int i = 0; i < m_arypEMNs.GetSize(); i++) {
		m_arypEMNs[i]->GetAllCheckedItems(mapDataIDs, mapDetailsToIgnore);
	}
}

//TES 1/24/2007 - PLID 24377 - This function finds any details on this EMR that are linked to the given detail, and 
// applies those links to the detail as appropriate.
// This function will only have an effect on multi-select lists, and will only check items, never uncheck them.	
void CEMR::ApplyEmrLinks(CEMNDetail *pDetail)
{
	//If spawning is locked, then we must be in the middle of some mass-adding of details.  So, just add this to our list
	// of details to be processed once the spawning is done.
	if(m_nSpawnLocks) {
		// (a.walling 2009-10-12 17:45) - PLID 36024
		pDetail->__AddRef("ApplyEmrLinks spawnlocked");

		m_arDetailsToBeLinked.Add(pDetail);
		return;
	} else {
		//Let's process this detail!
		CArray<CEMNDetail*,CEMNDetail*> arDetail;
		arDetail.Add(pDetail);
		ApplyEmrLinksToBatch(arDetail);
	}
}

struct DetailChange
{
	CEMNDetail *pDetail;
	CString strNewState;
};

struct EmrLinkIDDataIDGroup
{
	long nLinkID;
	long nDataID;
	long nDataGroupID;
};

BOOL PopulateUnselectedElementArray(CEMNDetail* pDetail, CArray<long,long>& arUnselectedListElementIDs)
{
	// (c.haag 2007-03-29 15:49) - PLID 25415 - This function fills arUnselectedListElementIDs with
	// values. It's called three times in CEMR::ApplyEmrLinksToBatch.
	BOOL bIsPertinent = FALSE;
	const int nListElements = pDetail->GetListElementCount();
	for(int n = 0; n < nListElements; n++) {
		ListElement le = pDetail->GetListElement(n);
		if(!le.bIsSelected && !le.bIsLabel) {
			// (z.manning, 02/02/2007) - 24558 - We used to do everything within this loop, however,
			// that meant creating a recordset once for per list item. Let's collect all the data IDs first.
			arUnselectedListElementIDs.Add(le.nID);
			bIsPertinent = TRUE;
		}
	}
	return bIsPertinent;
}

// This function is called after spawning, after an EMN or detail are added, and possibly other times.
// Its purpose is to leverage our "Linked EMR Items" functionality where a client can "link" elements
// in one list detail (lets call it "A") with elements in another list detail (lets call that "B"). When linked,
// the effect is this: If the linked element in list A exists on an EMN and is checked off, and you add
// list B to *another* EMN in the same EMR, then the linked element in list B should be checked off.
// The operative word is *another*; don't let this throw you off when you test. "Linked EMR Items" 
// are configured in the Administrator Module => NexEMR Tab from one of the bottom buttons.
void CEMR::ApplyEmrLinksToBatch(const CArray<CEMNDetail*,CEMNDetail*> &arDetails)
{
	// (c.haag 2007-03-29 08:30) - PLID 25406 - There is nothing we can do if the array is empty
	if (0 == arDetails.GetSize())
		return;

	//TES 1/24/2007 - PLID 24377 - Go through the batch, calculate the new states of all the items that are changed,
	// then go through at the end and apply them (so that links in this batch won't affect each other, all will be calculated
	// before any changes are applied).
	// (c.haag 2007-03-29 14:26) - PLID 25415 - This function has been optimized for speed and minimal querying
	CArray<DetailChange,DetailChange&> arChangedDetails;
	CArray<long,long> arUnselectedListElementIDs;
	CMap<long, long, CArray<CEMNDetail*,CEMNDetail*>*, CArray<CEMNDetail*,CEMNDetail*>*> mapDetailsToCheck;
	CArray<CEMNDetail*,CEMNDetail*> arPertinentDetails;
	const int nInDetails = arDetails.GetSize();
	CArray<long,long> anEmrInfoIDs;
	int nDetailsInMap = 0;
	int i;

	// (c.haag 2007-06-06 15:47) - PLID 26240 - If possible, use the CEMNLoader object to avoid
	// multiple accesses. To ensure everything behaves predictably, we must ensure that every
	// detail has the same valid loader, or else it may not work.
	CEMNLoader* pEMNLoader = NULL;
	if (NULL != arDetails[0]->m_pParentTopic) {
		if (NULL != arDetails[0]->m_pParentTopic->GetParentEMN()) {
			pEMNLoader = arDetails[0]->m_pParentTopic->GetParentEMN()->GetEMNLoader();
		}
	}
	for(i = 1; i < arDetails.GetSize() && NULL != pEMNLoader; i++) {
		if (NULL == arDetails[i]->m_pParentTopic) {
			pEMNLoader = NULL;
		} else if (NULL == arDetails[i]->m_pParentTopic->GetParentEMN()) {
			pEMNLoader = NULL;
		} else if (pEMNLoader != arDetails[i]->m_pParentTopic->GetParentEMN()->GetEMNLoader()) {
			pEMNLoader = NULL;
		}
	}


	for(i = 0; i < arDetails.GetSize(); i++) {
		CEMNDetail *pDetail = arDetails[i];
		if(pDetail->m_EMRInfoType == eitMultiList) {

			// (c.haag 2007-03-29 14:26) - PLID 25415 - If we get here, this is a multi-select
			// detail. We need to find out whether it's possible for the detail to add at least
			// one element arUnselectedListElementIDs. If its content was already loaded, we can
			// just do it right now.
			//
			if (!pDetail->GetNeedContentReload()) {
				BOOL bIsPertinent = PopulateUnselectedElementArray(pDetail, arUnselectedListElementIDs);
				if (bIsPertinent) {
					arPertinentDetails.Add(pDetail);
				}
			} 
			// If the content has not been loaded yet, put it in a special map, and we'll figure
			// out whether to add it to arPertinentDetails later
			else {
				CArray<CEMNDetail*,CEMNDetail*>* parNodeDetails = NULL;
				if (!mapDetailsToCheck.Lookup(pDetail->m_nEMRInfoID, parNodeDetails)) {
					mapDetailsToCheck[pDetail->m_nEMRInfoID] = parNodeDetails = new CArray<CEMNDetail*,CEMNDetail*>;
					anEmrInfoIDs.Add(pDetail->m_nEMRInfoID);
				}
				parNodeDetails->Add(pDetail);
				nDetailsInMap++;
			}
		}
	}

	// (c.haag 2007-03-29 14:29) - PLID 24515 - Now we need to figure out which elements in the map
	// belong in the pertinent array.
	//
	// If the map is empty, we'll do nothing.
	// If the map has exactly one detail, just run the above logic on it (will cost one query)
	// If the map has two or more details, run a query to narrow down the possible info ID's
	//		(will cost at least one query)
	//
	if (nDetailsInMap == 1) {
		// The map has only one detail, so run with it
		POSITION pos = mapDetailsToCheck.GetStartPosition();
		CArray<CEMNDetail*,CEMNDetail*>* parNodeDetails = NULL;
		long nEMRInfoID;
		mapDetailsToCheck.GetNextAssoc(pos, nEMRInfoID, parNodeDetails);
		CEMNDetail* pDetail = parNodeDetails->GetAt(0);
		BOOL bIsPertinent = PopulateUnselectedElementArray(pDetail, arUnselectedListElementIDs);
		if (bIsPertinent) {
			arPertinentDetails.Add(pDetail);
		}
		delete parNodeDetails; // Clean up the one and only allocated array

	} else if (nDetailsInMap > 1) {
		// The map has multiple details, so run a query to narrow down the possible list by EMR Info ID
		CArray<CEMNDetail*,CEMNDetail*>* parNodeDetails = NULL;
		long nEMRInfoID;

		// (c.haag 2007-06-06 15:50) - PLID 26240 - If the preloader object is available, use it
		if (NULL != pEMNLoader) {
			// (c.haag 2007-07-03 11:12) - PLID 26523 - Claim exclusive access to linked data items in the CEMNLoader
			CHoldEMNLoaderMutex mh(pEMNLoader->GetEMRLinkedDataItemsMutex());
			pEMNLoader->EnsureEmrItemLinkedDataTArray(GetRemoteData());
			const long nCount = pEMNLoader->GetEmrLinkedDataItemCount();
			for (long j=0; j < nCount; j++) {
				const CEMNLoader::EmrLinkedDataItem item = pEMNLoader->GetEmrLinkedDataItem(j);
				if (IsIDInArray(item.m_nEmrInfoID, anEmrInfoIDs)) {

					///// Legacy code (should always match loop below) /////
					mapDetailsToCheck.Lookup(item.m_nEmrInfoID, parNodeDetails);
					const int nDetails = parNodeDetails->GetSize();
					for (i=0; i < nDetails; i++) {
						CEMNDetail* pDetail = parNodeDetails->GetAt(i);
						BOOL bIsPertinent = PopulateUnselectedElementArray(pDetail, arUnselectedListElementIDs);
						if (bIsPertinent) {
							arPertinentDetails.Add(pDetail);
						}
					}
					///// Legacy code (should always match loop below) /////
				}
			}

		} else {
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			_RecordsetPtr prsInfoIDs = CreateParamRecordset("SELECT EmrInfoID FROM EMRDataT WHERE ID IN (SELECT EMRDataID FROM EmrItemLinkedDataT) "
				"AND EmrInfoID IN ({INTARRAY})", anEmrInfoIDs);
			while (!prsInfoIDs->eof) {
				mapDetailsToCheck.Lookup(AdoFldLong(prsInfoIDs, "EmrInfoID"), parNodeDetails);
				const int nDetails = parNodeDetails->GetSize();
				for (i=0; i < nDetails; i++) {
					CEMNDetail* pDetail = parNodeDetails->GetAt(i);
					BOOL bIsPertinent = PopulateUnselectedElementArray(pDetail, arUnselectedListElementIDs);
					if (bIsPertinent) {
						arPertinentDetails.Add(pDetail);
					}
				}
				prsInfoIDs->MoveNext();
			}
		}

		// All done. Clean up the map
		POSITION pos = mapDetailsToCheck.GetStartPosition();
		while (NULL != pos) {
			mapDetailsToCheck.GetNextAssoc(pos, nEMRInfoID, parNodeDetails);
			delete parNodeDetails;
		}
	}

	// (c.haag 2007-03-29 14:48) - PLID 24515 - If the unselected list is empty, there's ntohing
	// else we can do
	if (0 == arUnselectedListElementIDs.GetSize()) {
		return;
	}

	// (z.manning, 02/02/2007) - PLID 24558 - Ok, we have all the data IDs, now let's get all the links for these
	// data items is one query.
	// (z.manning, 02/02/2007) - PLID 24558 - We need to keep track of the link groups.
	CArray<EmrLinkIDDataIDGroup,EmrLinkIDDataIDGroup&> arLinkIDDataIDGroups;

	// (c.haag 2007-06-06 16:07) - PLID 26240 - Look out Zack, we can do this without a query now!
	if (NULL != pEMNLoader) {
		// (c.haag 2007-07-03 11:12) - PLID 26523 - Claim exclusive access to linked data items in the CEMNLoader
		CHoldEMNLoaderMutex mh(pEMNLoader->GetEMRLinkedDataItemsMutex());
		pEMNLoader->EnsureEmrItemLinkedDataTArray(GetRemoteData());
		const long nCount = pEMNLoader->GetEmrLinkedDataItemCount();
		long j;

		// (c.haag 2008-03-28 13:08) - PLID 29428 - The filter below wants all the data ID's such that
		// they correspond to an EmrLinkID that itself corresponds to a data ID in arUnselectedListElementIDs.
		// This is what we do:
		//
		// 1. Go through the preloader array once finding elements with EmrLinkID's that correspond to data ID's
		// in arUnselectedListElementIDs
		//
		// 2. Go through the preloader array a second time finding elements with EmrLinkID's that match 
		// the first search.
		// 
		CMap<long,long,BOOL,BOOL> mapLinkIDsOfInterest;
		
		// Loop 1 (described above)
		for (j=0; j < nCount; j++) {
			const CEMNLoader::EmrLinkedDataItem item = pEMNLoader->GetEmrLinkedDataItem(j);
			if (IsIDInArray(item.m_nEmrDataID, arUnselectedListElementIDs)) {
				mapLinkIDsOfInterest.SetAt(item.m_nEmrLinkID, TRUE);
			}
		} // for (j=0; j < nCount; j++) {

		// Loop 2 (described above)
		if (!mapLinkIDsOfInterest.IsEmpty()) {
			for (j=0; j < nCount; j++) {
				const CEMNLoader::EmrLinkedDataItem item = pEMNLoader->GetEmrLinkedDataItem(j);
				BOOL bValue = FALSE;
				if (mapLinkIDsOfInterest.Lookup(item.m_nEmrLinkID, bValue) && bValue) {
					///// Legacy code (should always match loop below) /////
					EmrLinkIDDataIDGroup group;
					group.nLinkID = item.m_nEmrLinkID;
					group.nDataID = item.m_nEmrDataID;
					group.nDataGroupID = item.m_nEmrDataGroupID;
					arLinkIDDataIDGroups.Add(group);
					///// Legacy code (should always match loop below) /////
				}
			} // for (j=0; j < nCount; j++) {
		} // if (!mapLinkIDsOfInterest.IsEmpty()) {

	} else {

		// (j.jones 2008-10-01 16:21) - PLID 31377 - This recordset has to use a temp table
		// if there are over 11,000 IDs, but can time out once the IDs are in the thousands.
		// What we should do is run a simple IN clause if we have 100 IDs or less, and
		// create a temp table if we have more than 100 IDs.
		// And yes, the choice of 100 is arbitrary, simply because in most cases it will be
		// well under 100. Therefore most of the time we just run one recordset, but in
		// extreme cases we use a temp table.

		CString strInClause, strTempTableName;

		if(arUnselectedListElementIDs.GetSize() <= 100) {
			//just use a normal IN clause, which causes less SQL accesses
			strInClause = ArrayAsString(arUnselectedListElementIDs, false);
		}
		else {
			//if we have over 100 IDs, it is better to use a temp table

			CStringArray aryFieldNames;
			CStringArray aryFieldTypes;
			CString strIn;

			CStringArray saXMLStatements;

			strIn = "<ROOT>";
			int i = 0;
			for(i=0; i<arUnselectedListElementIDs.GetSize(); i++) {
				CString str;
				str.Format("<P ID=\"%li\"/>", (long)arUnselectedListElementIDs.GetAt(i));			
				strIn += str;
				if(strIn.GetLength() > 2000) {
					//End this statement.
					strIn += "</ROOT>";
					saXMLStatements.Add(strIn);
					//Start a new one.
					strIn = "<ROOT>";
				}
			}

			if (!strIn.IsEmpty()) {
				//now add the trailer
				strIn += "</ROOT>";
				saXMLStatements.Add(strIn);
			}

			aryFieldNames.Add("ID");
			aryFieldTypes.Add("int");

			CString strSqlBatch = "";

			//now create a temp table
			strTempTableName = CreateTempTableFromXML(aryFieldNames, aryFieldTypes, saXMLStatements.GetAt(0), &strSqlBatch);
			for(i=1; i<saXMLStatements.GetSize(); i++) {
				AppendToTempTableFromXML(aryFieldNames, aryFieldTypes, saXMLStatements.GetAt(i), strTempTableName, &strSqlBatch);
			}

			if(strSqlBatch.IsEmpty()) {
				//shouldn't be possible
				ThrowNxException("ApplyEmrLinksToBatch attempted to create an empty temp table!");
			}

			//rather than calling ExecuteSqlBatch, call ExecuteSqlStd,
			//so we can override the nMaxRecordsAffected number, as this code
			//can update thousands of temp-table records. Set 100k as the upper limit,
			//it seems fair to warn if some monstrous EMR has that many list items.
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(100000);
			long nRecordsAffected = 0;
			ExecuteSqlStd("BEGIN TRAN \r\n" + strSqlBatch + "COMMIT TRAN \r\n", &nRecordsAffected, adCmdText);

			//now that we have our temp table, we can run our query using that		
			strInClause.Format("SELECT ID FROM %s", strTempTableName);
		}

		// (j.jones 2008-10-01 17:56) - PLID 31377 - this query is unchanged, it only now takes an 
		// IN clause of not more than 100 IDs, or a temp table name
		_RecordsetPtr rsLinks = CreateRecordset(
			"SELECT EmrLinkID, EmrDataID, EmrDataGroupID "
			"FROM EmrItemLinkedDataT INNER JOIN EmrDataT ON EmrItemLinkedDataT.EmrDataID = EmrDataT.ID "
			"WHERE EmrLinkID IN (SELECT EmrLinkID FROM EmrItemLinkedDataT WHERE EmrDataID IN (%s)) "
			"ORDER BY EmrLinkID ", strInClause);

		while(!rsLinks->eof) {
			EmrLinkIDDataIDGroup group;
			group.nLinkID = AdoFldLong(rsLinks, "EmrLinkID");
			group.nDataID = AdoFldLong(rsLinks, "EmrDataID");
			group.nDataGroupID = AdoFldLong(rsLinks, "EmrDataGroupID");
			arLinkIDDataIDGroups.Add(group);
			rsLinks->MoveNext();
		}
		rsLinks->Close();

		//If we added a temp table, drop it now, we don't need it anymore.
		//If an exception had been thrown, it's no big deal, as the table
		//would auto-drop when practice closed. Since this code is
		//self-contained though, we can safely drop it ourselves.
		if(!strTempTableName.IsEmpty()) {
			//can't parameterize this statement
			ExecuteSql("DROP TABLE %s", strTempTableName);
		}
	}

	// (c.haag 2007-03-29 15:23) - PLID 25415 - If this array is empty, there's nothing else we can do
	if (0 == arLinkIDDataIDGroups.GetSize())
		return;

	// (c.haag 2011-05-19) - PLID 43696 - In past implementations, we would go through every unchecked
	// element in every pertinent detail, see if a counterpart checkbox in the same EMR group is checked off
	// somewhere in the EMR, and if it is, then check off the element. The act of searching for checked counterpart 
	// checkboxes involved going through the ENTIRE EMR searching for them every time. If a detail we were 
	// searching didn't have its content loaded from data yet, it would be loaded despite the fact we could get
	// the same information from the detail state in memory!
	//
	// To expedite this process, we now only go through the EMR just once to get a map of all checked off 
	// single-select and multi-select element EmrDataID's.
	//
	CMap<CEMNDetail*,CEMNDetail*,BOOL,BOOL> mapDetails; // This has identical content to arDetails, but in a map form
	CMap<long,long,BOOL,BOOL> mapCheckedDataIDs; // Map of all EmrDataIDs that correspond to checked list elements in the entire EMR
	// Populate mapDetails with the content from arDetails for fast lookups
	{
		for (int j=0; j < arDetails.GetSize(); j++)	{
			mapDetails.SetAt(arDetails[j], TRUE);
		}
	}
	// Get all the checked items in memory (this goes through the whole EMR)
	GetAllCheckedItems(mapCheckedDataIDs, mapDetails);

	// (c.haag 2011-05-19) - PLID 43696 - Skip the work if nothing in the whole EMR is checked
	if (mapCheckedDataIDs.GetSize() > 0)
	{
		for(int j = 0; j < arPertinentDetails.GetSize(); j++) {
			CEMNDetail *pDetail = arPertinentDetails[j];
			//TES 1/23/2007 - PLID 24377 - If we're on a multi-select list, go through all our unchecked lists, and find out if
			// any data elements that they are linked to are already checked on this EMR.
			// (c.haag 2007-03-29 14:29) - PLID 25415 - We now draw from an array where each element
			// is guaranteed to be a multi-select list
			//if(pDetail->m_EMRInfoType == eitMultiList)
			{
				BOOL bStateChanged = FALSE;
				CString strNewState = "";
				//Make sure that our array of elements has been generated.
				for(int k = 0; k < pDetail->GetListElementCount(); k++) {
					ListElement le = pDetail->GetListElement(k);
					if(!le.bIsSelected && !le.bIsLabel) {
						//Load all of the data elements that are linked to this one in an array.
						//_RecordsetPtr rsLinks = CreateRecordset("SELECT EmrDataID FROM EmrItemLinkedDataT WHERE EmrLinkID IN "
						//	" (SELECT EmrLinkID FROM EmrItemLinkedDataT WHERE EmrDataID = %li)", le.nID);
						// (z.manning, 02/02/2007) - PLID 24558 - We've already loaded all the possible link groups.
						// So let's go through them and find which groups contain this data ID.
						CArray<long,long> arLinkIDs;
						for(int l = 0; l < arLinkIDDataIDGroups.GetSize(); l++) {
							if(arLinkIDDataIDGroups.GetAt(l).nDataID == le.nID) {
								arLinkIDs.Add(arLinkIDDataIDGroups.GetAt(l).nLinkID);
							}
						}
						// (z.manning, 02/02/2007) - PLID 24558 - Ok, we know what link groups we want, now go through
						// each of those groups and get the data IDs.
						CArray<long,long> arLinkedDataIDs;
						for(int m = 0; m < arLinkIDs.GetSize(); m++) {
							for(int n = 0; n < arLinkIDDataIDGroups.GetSize(); n++) {
								if(arLinkIDs.GetAt(m) == arLinkIDDataIDGroups.GetAt(n).nLinkID
									//TES 2/9/2007 - PLID 24671 - Links don't apply to themselves, so make sure that the two
									// things we're linking don't have the same EmrDataGroupID
									&& le.nDataGroupID != arLinkIDDataIDGroups.GetAt(n).nDataGroupID) {
									arLinkedDataIDs.Add(arLinkIDDataIDGroups.GetAt(n).nDataID);
								}
							}
						}

						if(arLinkedDataIDs.GetSize() > 0) {//Don't bother checking if there are no links.

							// (c.haag 2011-05-19) - PLID 43696 - Optimized way of searching for data ID's that correspond to
							// checked boxes anywhere in the EMR.
							for (int m=0; m < arLinkedDataIDs.GetSize(); m++) 
							{
								BOOL bDummy;
								if (mapCheckedDataIDs.Lookup(arLinkedDataIDs[m], bDummy))
								{
									//An item that we're linked to is checked, so we need to be checked.  Add to our new state.
									strNewState += AsString(le.nID) + "; ";
									//We've now changed our state, so we'll need to update the detail
									bStateChanged = TRUE;
									break;
								}
							}
							//TES 2/6/2007 - PLID 24377 - Tell this function to ignore any details in this batch.
							/*if(IsAnyItemChecked(arLinkedDataIDs, arDetails)) 
							{
								//An item that we're linked to is checked, so we need to be checked.  Add to our new state.
								strNewState += AsString(le.nID) + "; ";
								//We've now changed our state, so we'll need to update the detail
								bStateChanged = TRUE;
							}*/
						}
					}else if(le.bIsSelected) {
						//This will be part of our new state (if we end up changing anything).
						strNewState += AsString(le.nID) + "; ";
					}
				}

				if(bStateChanged) {
					ASSERT(strNewState.GetLength() > 2);//We couldn't have gotten here without something being checked.
					//Trim the last "; "
					strNewState = strNewState.Left(strNewState.GetLength()-2);
					
					//Add it to our list.
					DetailChange dc;
					dc.strNewState = strNewState;
					dc.pDetail = pDetail;
					arChangedDetails.Add(dc);
				}

			}
		}
	} // if (mapCheckedDataIDs.GetSize() > 0)

	//Now, go through all the details which changed, and apply the changes.
	for(int nIndex = 0; nIndex < arChangedDetails.GetSize(); nIndex++) {
		arChangedDetails[nIndex].pDetail->RequestStateChange(_bstr_t(arChangedDetails[nIndex].strNewState));
	}
}

// (z.manning, 02/01/2007) - PLID 24524 - Will show/hide the action progress bar if there is one.
void CEMR::ShowActionProgressBar(UINT nShow)
{
	if(m_pActionProgressDlg) {
		m_pActionProgressDlg->ShowProgress(nShow);
	}
}

// (j.jones 2007-02-06 13:48) - PLID 24509 - allow to begin/end the StartEditingTime
void CEMR::TryStartTrackingTime()
{
	//if not tracking, set the current time, otherwise do nothing
	if(!m_bIsTemplate && !m_bIsTrackingTime) {
		m_bIsTrackingTime = TRUE;
		//get the time from the server
		_RecordsetPtr rs = CreateRecordset("SELECT GetDate() AS Date");
		if(!rs->eof) {
			m_dtStartEditingTime = AdoFldDateTime(rs, "Date");
		}
		rs->Close();
	}
}

void CEMR::TryStopTrackingTime()
{
	//if tracking, save the time, and stop tracking
	if(!m_bIsTemplate && m_bIsTrackingTime) {

		//ensure we have a valid start time
		if(m_dtStartEditingTime.GetStatus() == COleDateTime::invalid) {
			ASSERT(FALSE);
			return;
		}
		// (c.haag 2010-01-18 09:53) - PLID 26036 - Ensure we have a valid server time offset
		if (m_dtServerTimeOffset.GetStatus() == COleDateTimeSpan::invalid)
		{
			ASSERT(FALSE);
			return;
		}

		//only save the time if a change was made during this session
		
		// (j.jones 2012-01-31 15:28) - PLID 47878 - call HasChangesMadeThisSession(),
		// which will also check if we changed EMNs, we may have not changed the EMR itself
		// since it has very little data of its own
		if(HasChangesMadeThisSession() && m_nEMRID != -1) {

			//save the time
			// (c.haag 2010-01-14 12:24) - PLID 26036 - We used to store the time from m_dtStartEditingTime to GETDATE().
			// Now what we do instead is construct several slips based on the times that the EMR clock was paused and resumed.
			const COleDateTime dtBegin = m_dtStartEditingTime; // Absolute start of the timeline in SERVER time
			const COleDateTime dtNow = COleDateTime::GetCurrentTime() + m_dtServerTimeOffset; // Absolute end of the timeline in SERVER TIME

			COleDateTime dtStart = dtBegin;
			COleDateTime dtEnd;
			int i;

			if (m_adtServerPauseStartTimes.GetSize() != m_adtServerPauseEndTimes.GetSize()) {
				ThrowNxException("CEMR::TryStopTrackingTime failed due to inconsistent pause time array sizes!");
			}

			// Start the batch and create the slip ID variable
			// (a.walling 2010-11-01 10:36) - PLID 40965 - Parameterize this, it is filling up the cache!
			// Should have an identity column as well, but oh well.
			// (j.jones 2013-02-26 15:52) - PLID 55338 - finally an identity
			CParamSqlBatch batch;
			// Do for each set of paused times. Note that we audit despite what the start and end times are. In theory, you could rapid-fire minimize-maximize.
			// Consequently, you'd just get a lot of time slips because you were being silly.
			for (i=0; i < m_adtServerPauseStartTimes.GetSize(); i++) {
				dtEnd = m_adtServerPauseStartTimes[i];
				batch.Add("INSERT INTO EMRGroupsSlipT (EMRGroupID, UserID, StartTime, EndTime) "
					"VALUES ({INT}, {INT}, {OLEDATETIME}, {OLEDATETIME})"
					, m_nEMRID
					, GetCurrentUserID()
					, dtStart
					, dtEnd
				);
				dtStart = m_adtServerPauseEndTimes[i];
			}
			// Now try to add the final slip
			if (COleDateTime::valid == dtStart.GetStatus()) {
				dtEnd = dtNow;
				batch.Add("INSERT INTO EMRGroupsSlipT (EMRGroupID, UserID, StartTime, EndTime) "
					"VALUES ({INT}, {INT}, {OLEDATETIME}, {OLEDATETIME})"
					, m_nEMRID
					, GetCurrentUserID()
					, dtStart
					, dtEnd
				);
			}
			else {
				// If we get here, we saved the EMR while it was still paused (it can be done by quickly maximizing or
				// restoring the EMR and clicking Save). So, there's no "final slip" to save as the amount of time the
				// window was restored (assuming any) is negligible.
			}
			batch.Execute(GetRemoteData());
		}
		
		//now reset our tracking variables
		m_bIsTrackingTime = FALSE;
		m_dtStartEditingTime.SetStatus(COleDateTime::invalid);
		m_bChangesMadeThisSession = FALSE;
		// (c.haag 2010-01-14 12:23) - PLID 26036
		m_adtServerPauseStartTimes.RemoveAll();
		m_adtServerPauseEndTimes.RemoveAll();
	}
}

COleDateTime CEMR::GetStartEditingTime()
{
	return m_dtStartEditingTime;
}

// (j.jones 2007-09-17 17:35) - PLID 27396 - update the server offset
void CEMR::UpdateServerTimeOffset(COleDateTimeSpan dtOffset)
{
	m_dtServerTimeOffset = dtOffset;
}

// (j.jones 2007-09-17 17:35) - PLID 27396 - return the server offset
COleDateTimeSpan CEMR::GetServerTimeOffset()
{
	return m_dtServerTimeOffset;
}

// (j.jones 2007-03-13 17:22) - PLID 25193 - lets the caller know if we have any pending actions,
// or if we are currently processing those pending actions
BOOL CEMR::GetHasFinishedSpawning()
{
	BOOL bHasFinishedSpawning = TRUE;

	// (z.manning 2011-03-04 18:14) - PLID 42682 - We now keeping pending actions in memory so we
	// used the processed flag to determine if they've all been processed.
	for(int i = 0; i < m_arPendingActions.GetSize(); i++)
	{
		PendingAction *pa = m_arPendingActions.GetAt(i);
		if(!pa->bProcessed) {
			bHasFinishedSpawning = FALSE;
			break;
		}
	}

	if(m_nUnlockSpawnsRefCount > 0)
		bHasFinishedSpawning = FALSE;

	return bHasFinishedSpawning;
}

// (z.manning 2008-12-12 14:06) - PLID 32427 - Returns true if spawning is unlocked, false if it's locked
BOOL CEMR::IsSpawningUnlocked()
{
	return (m_nSpawnLocks == 0 && m_nUnlockSpawnsRefCount == 0);
}

// (j.jones 2007-07-20 14:55) - PLID 26742 - grab the cached value from the EMR
long CEMR::GetCurrentMedicationsInfoID(ADODB::_Connection *lpCon /*= NULL*/)
{
	_ConnectionPtr pConn;
	if(lpCon) pConn = lpCon;
	else pConn = GetRemoteData();

	if(m_nCurrentMedicationsInfoID == -2) { //-2 is uncached
		m_nCurrentMedicationsInfoID = GetActiveCurrentMedicationsInfoID(pConn);
	}
	else if(m_bIsTemplate && GetInterface()) {
		//if a non-template, we get tablecheckers that directly alter the tracked Info ID
		//but if a template, we can't interpret the tablecheckers that give us the ID, only
		//whether the tablechecker changed, which means on a template, if the current Info ID
		//did change, we have to query the data
		CEmrTreeWnd *pTree = GetInterface();
		if(pTree->m_CurrentMedicationsInfoChecker.Changed()) {
			m_nCurrentMedicationsInfoID = GetActiveCurrentMedicationsInfoID(pConn);
		}
	}

	return m_nCurrentMedicationsInfoID;
}

long CEMR::GetCurrentAllergiesInfoID(ADODB::_Connection *lpCon /*= NULL*/)
{
	_ConnectionPtr pConn;
	if(lpCon) pConn = lpCon;
	else pConn = GetRemoteData();

	if(m_nCurrentAllergiesInfoID == -2) { //-2 is uncached
		m_nCurrentAllergiesInfoID = GetActiveAllergiesInfoID(pConn);
	}
	else if(m_bIsTemplate && GetInterface()) {
		//if a non-template, we get tablecheckers that directly alter the tracked Info ID
		//but if a template, we can't interpret the tablecheckers that give us the ID, only
		//whether the tablechecker changed, which means on a template, if the current Info ID
		//did change, we have to query the data
		CEmrTreeWnd *pTree = GetInterface();
		if(pTree->m_CurrentAllergiesInfoChecker.Changed()) {
			m_nCurrentAllergiesInfoID = GetActiveAllergiesInfoID(pConn);
		}
	}

	return m_nCurrentAllergiesInfoID;
}

// (j.jones 2010-06-21 15:21) - PLID 37981 - supported generic tables
long CEMR::GetCurrentGenericTableInfoID(ADODB::_Connection *lpCon /*= NULL*/)
{
	_ConnectionPtr pConn;
	if(lpCon) pConn = lpCon;
	else pConn = GetRemoteData();

	if(m_nCurrentGenericTableInfoID == -2) { //-2 is uncached
		m_nCurrentGenericTableInfoID = GetActiveGenericTableInfoID(pConn);
	}
	else if(m_bIsTemplate && GetInterface()) {
		//if a non-template, we get tablecheckers that directly alter the tracked Info ID
		//but if a template, we can't interpret the tablecheckers that give us the ID, only
		//whether the tablechecker changed, which means on a template, if the current Info ID
		//did change, we have to query the data
		CEmrTreeWnd *pTree = GetInterface();
		if(pTree->m_CurrentGenericTableInfoChecker.Changed()) {
			m_nCurrentGenericTableInfoID = GetActiveGenericTableInfoID(pConn);
		}
	}

	return m_nCurrentGenericTableInfoID;
}


// (j.jones 2007-07-23 10:54) - PLID 26742 - be able to update the cached value from an external source
void CEMR::SetCurrentMedicationsInfoID(long nNewInfoID)
{
	m_nCurrentMedicationsInfoID = nNewInfoID;
}

void CEMR::SetCurrentAllergiesInfoID(long nNewInfoID)
{
	m_nCurrentAllergiesInfoID = nNewInfoID;
}

// (j.jones 2010-06-21 15:21) - PLID 37981 - supported generic tables
void CEMR::SetCurrentGenericTableInfoID(long nNewInfoID)
{
	m_nCurrentGenericTableInfoID = nNewInfoID;
}

// (z.manning, 08/15/2007) - PLID 26809 - Added an accessor for the load topics info object.
LoadAllTopicsInfo* CEMR::GetLoadAllTopicsInfo()
{
	// (a.walling 2010-07-23 12:00) - PLID 39834 - Get the info from the thread
	return m_pLoadTopicsThread ? &(m_pLoadTopicsThread->m_LoadTopicsInfo) : NULL;
}

CEMRItemAdvMultiPopupDlg* CEMR::GetOpenMultiPopupDlg()
{
	//TES 1/16/2008 - PLID 24157 - There shouldn't be more than one popup dialog open at a time.  So loop through all
	// our EMNs until we find an open one, and return it.  If we don't find any, we will return NULL.
	CEMRItemAdvMultiPopupDlg *pDlg = NULL;
	for(int i = 0; i < m_arypEMNs.GetSize(); i++) {
		pDlg = m_arypEMNs[i]->GetOpenMultiPopupDlg();
		if(pDlg) return pDlg;
	}
	return pDlg;
}

// (a.walling 2008-06-09 17:40) - PLID 22049 - Remove the EMN from the list but do not perform any specific actions
// returns the index of the found EMN, -1 if failed
long CEMR::RemoveEMNByPointerRaw(CEMN *pEMN)
{
	for (int i = 0; i < m_arypEMNs.GetSize(); i++) {
		if (pEMN == m_arypEMNs[i]) {
			m_arypEMNs.RemoveAt(i);
			return i;
		}
	}

	return -1;
}

// (a.walling 2008-06-10 09:21) - PLID 22049 - Reorder an EMN in the array
void CEMR::ReorderEMN(CEMN* pEMN, long nDesiredIndex)
{
	if (nDesiredIndex >= m_arypEMNs.GetSize()) {
		return;
	}

	long nActualIndex = -1;

	for (int i = 0; i < m_arypEMNs.GetSize(); i++) {
		if (pEMN == m_arypEMNs[i]) {
			// we have found it!
			nActualIndex = i;
			break;
		}
	}

	if (nActualIndex != nDesiredIndex && nActualIndex >= 0 && nDesiredIndex >= 0) {
		m_arypEMNs.RemoveAt(nActualIndex);
		// (a.walling 2010-08-17 13:54) - PLID 38006 - Verify pEMN
		_ASSERTE(pEMN != NULL); // we should never be adding an EMN that is NULL!
		m_arypEMNs.InsertAt(nDesiredIndex, pEMN);
	}
}

// (a.walling 2008-07-07 12:58) - PLID 30513 - Sets flag to ignore readonly modified EMNs
void CEMR::SetIgnoreReadOnly(BOOL bIgnore)
{
	m_bIgnoreReadOnlyEMNs = bIgnore;
}


// (c.haag 2008-07-09 13:15) - PLID 30648 - Appends apTodos with this EMR's list of todos
// created prior to their corresponding entities having been saved
void CEMR::GenerateCreatedTodosWhileUnsavedList(CArray<EMNTodo*,EMNTodo*>& apTodos)
{
	if (!m_bIsTemplate) {
		for (int i=0; i < m_arypEMNs.GetSize(); i++) {
			CEMN* pEMN = m_arypEMNs[i];
			pEMN->GenerateCreatedTodosWhileUnsavedList(apTodos);
		}
	}
}

// (c.haag 2008-07-09 13:15) - PLID 30648 - Appends apTodos with this EMR's list of todos
// deleted prior to their corresponding entities having been saved
void CEMR::GenerateDeletedTodosWhileUnsavedList(CArray<EMNTodo*,EMNTodo*>& apTodos)
{
	if (!m_bIsTemplate) {
		for (int i=0; i < m_arypEMNs.GetSize(); i++) {
			CEMN* pEMN = m_arypEMNs[i];
			pEMN->GenerateDeletedTodosWhileUnsavedList(apTodos);
		}
	}
}

// (j.jones 2008-07-21 17:28) - PLID 30729 - add all of this EMR's problems
// and all of the EMNs' problems to the passed-in array
// (c.haag 2008-08-14 12:05) - PLID 30820 - Added bIncludeDeletedProblems
// (c.haag 2009-05-19 12:20) - PLID 34277 - Use the new problem linking structure. Use EnsureProblemInArray
// to ensure no array content is duplicated.
void CEMR::GetAllProblems(CArray<CEmrProblem*, CEmrProblem*> &aryProblems, BOOL bIncludeDeletedProblems /* = FALSE */)
{
	try {

		//add problems from this EMR
		int i = 0;
		for(i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
			CEmrProblem *pProblem = m_apEmrProblemLinks.GetAt(i)->GetProblem();
			if(pProblem != NULL && (!pProblem->m_bIsDeleted || bIncludeDeletedProblems)) {
				ASSERT(pProblem->m_bDoNotShowOnCCDA > -1);
				EnsureProblemInArray(aryProblems, pProblem);
			}
		}

		//add problems from the EMNs
		for(i = 0; i < m_arypEMNs.GetSize(); i++) {
			CEMN *pEMN = (CEMN*)m_arypEMNs.GetAt(i);
			if(pEMN) {
				//get the EMN's problems
				pEMN->GetAllProblems(aryProblems, bIncludeDeletedProblems);
			}
		}

		// (c.haag 2009-06-01 09:46) - PLID 34277 - We also need to traverse deleted EMN's.
		// If a user deletes an EMN, then its problem links are deleted (well, flagged as deleted
		// until the data is committed). However, the EMN is also moved into m_aryDeletedEMNs. If
		// the problems are exclusively for that EMN, then this function will never find them unless
		// it traverses m_aryDeletedEMNs.
		if(bIncludeDeletedProblems) {
			for(i = 0; i < m_aryDeletedEMNs.GetSize(); i++) {
				CEMN *pEMN = (CEMN*)m_aryDeletedEMNs.GetAt(i);
				if(pEMN) {
					//get the EMN's problems
					pEMN->GetAllProblems(aryProblems, bIncludeDeletedProblems);
				}
			}
		}

	}NxCatchAll("Error in CEMR::GetAllProblems");
}

// (j.jones 2008-07-22 08:48) - PLID 30789 - returns true if there are any undeleted problems on the EMR
// (c.haag 2009-05-19 12:26) - PLID 34277 - Use the new problem linking structure
BOOL CEMR::HasProblems()
{
	try {

		for(int i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
			CEmrProblem *pProblem = m_apEmrProblemLinks.GetAt(i)->GetProblem();
			if(pProblem != NULL && !pProblem->m_bIsDeleted && !m_apEmrProblemLinks.GetAt(i)->IsDeleted()) {

				return TRUE;
			}
		}

	}NxCatchAll("Error in CEMR::HasProblems");

	return FALSE;
}

// (j.jones 2008-07-22 08:48) - PLID 30789 - returns true if there are only undeleted, closed problems on the EMR
// (c.haag 2009-05-19 12:26) - PLID 34277 - Use the new problem linking structure
BOOL CEMR::HasOnlyClosedProblems()
{
	try {

		BOOL bHasProblems = FALSE;
		BOOL bHasOnlyClosed = TRUE;

		for(int i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
			CEmrProblem *pProblem = m_apEmrProblemLinks.GetAt(i)->GetProblem();
			if(pProblem != NULL && !pProblem->m_bIsDeleted) {

				bHasProblems = TRUE;
				
				if(pProblem->m_nStatusID != 2) {
					bHasOnlyClosed = FALSE;
				}
			}
		}

		if(bHasProblems && bHasOnlyClosed) {
			return TRUE;
		}
		else {
			return FALSE;
		}

	}NxCatchAll("Error in CEMR::HasOnlyClosedProblems");

	return FALSE;
}

// (c.haag 2008-07-23 12:16) - PLID 30820 - Populate apProblems with a list of all deleted problems for this object and
// all its children. If a child or related EMR object is deleted, all its problems are considered deleted as well.
// (j.jones 2009-05-29 09:46) - PLID 34301 - returns links now, not problems
void CEMR::GetAllDeletedEmrProblemLinks(CArray<CEmrProblemLink*,CEmrProblemLink*>& apProblemLinks, BOOL bIncludeThisObject)
{
	int i;

	// Deleted EMN's
	for (i=0; i < m_aryDeletedEMNs.GetSize(); i++) {
		m_aryDeletedEMNs[i]->GetAllDeletedEmrProblemLinks(apProblemLinks, TRUE);
	}

	// Existing EMN's
	for (i=0; i < m_arypEMNs.GetSize(); i++) {
		m_arypEMNs[i]->GetAllDeletedEmrProblemLinks(apProblemLinks, FALSE);
	}

	CArray<CEmrProblemLink*,CEmrProblemLink*> apTmp;
	GetAllProblemLinks(apTmp, NULL, TRUE);
	// If the following is true, it means that this EMR is going to be deleted, so include all
	// the existing problems for presently undeleted objects.
	if (bIncludeThisObject) {
		for (i=0; i < apTmp.GetSize(); i++) {
			CEmrProblemLink *pLink = apTmp[i];
			// Don't check the deleted flag of the problem link. It's possible for a problem's
			// deleted flag to be set even if only its bound EMR object is deleted.
			// (j.jones 2009-05-29 09:46) - PLID 34301 - returns links now
			// also we need to include -1 ID links that we created but then deleted,
			// so we can reconcile against the problems in memory
			if(pLink != NULL /*&& pLink->GetID() > 0*/) {
				EnsureProblemLinkInArray(apProblemLinks, pLink);
			}
		}
	}
	// If we get here, this EMR itself is not going to be deleted. So, go through apTmp and just
	// check for deleted problems
	else {
		for (i=0; i < apTmp.GetSize(); i++) {
			CEmrProblemLink *pLink = apTmp[i];
			// (j.jones 2009-05-29 09:46) - PLID 34301 - returns links now
			// also we need to include -1 ID links that we created but then deleted,
			// so we can reconcile against the problems in memory
			if(pLink != NULL && pLink->GetIsDeleted() /*&& pLink->GetID() > 0*/) {
				EnsureProblemLinkInArray(apProblemLinks, pLink);
			}
		}
	}
}

// (j.jones 2008-07-23 16:54) - PLID 30789 - returns true if any problems are marked as modified,
// including deleted items
// (c.haag 2009-05-19 12:26) - PLID 34277 - Use the new problem linking structure
BOOL CEMR::HasChangedProblems()
{
	try {

		for(int i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
			CEmrProblem *pProblem = m_apEmrProblemLinks.GetAt(i)->GetProblem();
			if(pProblem != NULL) {

				if(pProblem->m_bIsModified) {
					return TRUE;
				}
			}
		}

	}NxCatchAll("Error in CEMR::HasChangedProblems");

	return FALSE;
}

// (c.haag 2008-07-24 09:49) - PLID 30826 - Returns TRUE if there is at least one saved problem for this EMR or any of
// its children. This does not check deleted EMR objects.
BOOL CEMR::DoesEmrOrChildrenHaveSavedProblems()
{
	CArray<CEmrProblem*,CEmrProblem*> apProblems;
	GetAllProblems(apProblems);
	const int nProblems = apProblems.GetSize();
	for (int i=0; i < nProblems; i++) {
		CEmrProblem* p = apProblems[i];
		if (p && !p->m_bIsDeleted && p->m_nID > 0) {
			return TRUE;
		}
	}
	return FALSE;
}

// (z.manning 2009-05-21 12:11) - PLID 34297 - Returns true if there is at least one unsaved problem for this EMR.
BOOL CEMR::DoesEmrOrChildrenHaveUnsavedProblems()
{
	CArray<CEmrProblem*,CEmrProblem*> arypProblems;
	GetAllProblems(arypProblems, TRUE);
	const int nProblems = arypProblems.GetSize();
	for(int nProblemIndex = 0; nProblemIndex < nProblems; nProblemIndex++) {
		CEmrProblem* pProblem = arypProblems.GetAt(nProblemIndex);
		if(pProblem != NULL) {
			if((pProblem->m_nID == -1 || pProblem->m_bIsModified) && !pProblem->m_bIsDeleted) {
				return TRUE;
			}
			else if(pProblem->m_nID != -1 && pProblem->m_bIsDeleted) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

// (z.manning 2009-06-30 14:45) - PLID 34340
BOOL CEMR::DoesEmrOrChildrenHaveUnsavedProblemLinks()
{
	CArray<CEmrProblemLink*,CEmrProblemLink*> arypProblemLinks;
	GetAllProblemLinks(arypProblemLinks, NULL, TRUE);
	for(int nProblemLinkIndex = 0; nProblemLinkIndex < arypProblemLinks.GetSize(); nProblemLinkIndex++)
	{
		CEmrProblemLink *pProblemLink = arypProblemLinks.GetAt(nProblemLinkIndex);
		if(pProblemLink != NULL) {
			if(pProblemLink->GetID() == -1) {
				// (z.manning 2009-06-30 14:50) - PLID 34340 - New problem, so it's unsaved
				return TRUE;
			}
			if(pProblemLink->GetID() != -1 && pProblemLink->GetIsDeleted()) {
				// (z.manning 2009-06-30 14:51) - PLID 34340 - Existing problem that was deleted
				return TRUE;
			}
		}
	}

	return FALSE;
}

// (a.walling 2013-07-23 21:36) - PLID 57686 - CEMR::m_mapDropdownIDToDropdownGropupID (sic) no longer necessary; we already have this in Nx::Cache. 
// (j.jones 2009-05-21 15:58) - PLID 34325 - recurses through all children and returns all problem links within the entire EMR,
// or just the problem links that reference pFilterProblem, if pFilterProblem is not NULL
void CEMR::GetAllProblemLinks(CArray<CEmrProblemLink*, CEmrProblemLink*> &aryProblemLinks, CEmrProblem *pFilterProblem /*= NULL*/, BOOL bIncludeDeletedLinks /*= FALSE*/)
{
	//throw exceptions to the caller

	//add problem links from this EMR
	int i = 0;
	for(i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
		CEmrProblemLink *pProblemLink = m_apEmrProblemLinks.GetAt(i);
		CEmrProblem *pProblem = pProblemLink->GetProblem();
		if(pProblem != NULL && (pFilterProblem == NULL || pFilterProblem == pProblem)
			&& (!pProblem->m_bIsDeleted || bIncludeDeletedLinks)) {

			//add if it doesn't already exist
			BOOL bFound = FALSE;
			for(int p=0; p<aryProblemLinks.GetSize() && !bFound; p++) {
				if(aryProblemLinks.GetAt(p) == pProblemLink) {
					bFound = TRUE;
				}
			}

			if(!bFound) {
				aryProblemLinks.Add(pProblemLink);
			}
		}
	}

	//add problem links from the EMNs
	for(i = 0; i < m_arypEMNs.GetSize(); i++) {
		CEMN *pEMN = (CEMN*)m_arypEMNs.GetAt(i);
		if(pEMN) {
			//get the EMN's problem links
			pEMN->GetAllProblemLinks(aryProblemLinks, pFilterProblem, bIncludeDeletedLinks);
		}
	}

	//add problem links from the deleted EMNs
	if(bIncludeDeletedLinks) {
		for(i = 0; i < m_aryDeletedEMNs.GetSize(); i++) {
			CEMN *pEMN = (CEMN*)m_aryDeletedEMNs.GetAt(i);
			if(pEMN) {
				//get the EMN's problem links
				pEMN->GetAllProblemLinks(aryProblemLinks, pFilterProblem, bIncludeDeletedLinks);
			}
		}
	}
}
// (z.manning 2016-04-12 10:20) - NX-100140 - Added version that doesn't take in the current problems and links
// since the EMR already has them all.
void CEMR::FindAllEmrProblemsToDelete(CArray<CEmrProblem*, CEmrProblem*>& apDoomedProblems, BOOL bProblemsInDataOnly)
{
	CArray<CEmrProblem*, CEmrProblem*> arypProblems;
	GetAllProblems(arypProblems, TRUE);
	CArray<CEmrProblemLink*, CEmrProblemLink*> arypDeletedProblemLinks;
	GetAllDeletedEmrProblemLinks(arypDeletedProblemLinks, TRUE);

	FindEmrProblemsToDelete(arypProblems, arypDeletedProblemLinks, apDoomedProblems, bProblemsInDataOnly);
}

// (c.haag 2009-06-08 09:23) - PLID 34398 - This function populates apDoomedProblems with a list
// of all EMR problems that are either deleted, or are not associated with any undeleted problem links.
// Josh authored all the code; I simply moved it into a utility function
void CEMR::FindEmrProblemsToDelete(const CArray<CEmrProblem*,CEmrProblem*>& apProblems,								   
								   const CArray<CEmrProblemLink*,CEmrProblemLink*>& apProblemLinksToDelete,
								   CArray<CEmrProblem*,CEmrProblem*>& apDoomedProblems,
								   BOOL bProblemsInDataOnly)
{
	int i=0;
	for(i=0; i<apProblems.GetSize(); i++) {
		CEmrProblem *pProblem = apProblems.GetAt(i);
		if(pProblem) {
			//for each problem we have in memory, find out if it is marked deleted,
			//or will now be deleted because we've removed the last link

			if(pProblem->m_bIsDeleted && (!bProblemsInDataOnly || pProblem->m_nID > 0)) {
				//simple deletion
				apDoomedProblems.Add(pProblem);
			}
			else {

				//now the fun part, if the problem is not deleted, see if we have
				//now removed its last link... which means we have to check data to
				//see if other links exist, but we don't need to check at all unless
				//we at least removed one link

				BOOL bRemovedOne = FALSE;
				for (int j=0; j < apProblemLinksToDelete.GetSize() && !bRemovedOne; j++) {
					if(apProblemLinksToDelete.GetAt(j)->GetProblem() == pProblem) {
						bRemovedOne = TRUE;
					}
				}

				if(bRemovedOne) {

					CArray<CEmrProblemLink*,CEmrProblemLink*> apAllLinks;
					CArray<CEmrProblemLink*,CEmrProblemLink*> apLinksFromData;
					PopulateProblemLinkArrays(pProblem, this, apAllLinks, apLinksFromData);
					
					BOOL bHasValidLink = FALSE;

					int j=0;
					for (j=0; j < apAllLinks.GetSize() && !bHasValidLink; j++) {

						CEmrProblemLink* pLink = apAllLinks[j];
						if(!pLink->GetIsDeleted()) {
							//is it in our list of links that will be deleted?
							BOOL bFound = FALSE;
							for (int k=0; k < apProblemLinksToDelete.GetSize() && !bFound; k++) {
								if(pLink == apProblemLinksToDelete.GetAt(k)
									|| pLink->GetID() == apProblemLinksToDelete.GetAt(k)->GetID()) {
									bFound = TRUE;
								}
							}
							
							if(!bFound) {
								//if not found, it means this link is still valid,
								//which means in turn that this problem will not need to be deleted
								bHasValidLink = TRUE;
							}
						}
					}

					//now, if there still is no valid link, we will be deleting this problem
					if(!bHasValidLink && (!bProblemsInDataOnly || pProblem->m_nID > 0)) {
						apDoomedProblems.Add(pProblem);
					}

					//clean up what we loaded, if anything
					for (j=0; j < apLinksFromData.GetSize(); j++) {
						delete apLinksFromData[j];
					}
				}
			}
		}
	}
}

// (c.haag 2010-01-14 11:28) - PLID 26036 - Called to "pause" the clock timer that tracks how long
// someone has had the chart open
void CEMR::PauseClock(const COleDateTime& dtServerPauseTime)
{
	// The m_adtServerPause[Start][End]Times array tracks all the different times that the clock was paused.
	// When we "pause" the clock here, we simply add an entry to both arrays. These arrays are ultimately
	// used for saving time slips.
	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	m_adtServerPauseStartTimes.Add(COleDateTime(dtServerPauseTime));
	m_adtServerPauseEndTimes.Add(dtInvalid);
}

// (c.haag 2010-01-14 11:28) - PLID 26036 - Called to "resume" the clock timer that tracks how long
// someone has had the chart open
void CEMR::ResumeClock(const COleDateTime& dtServerResumeTime)
{
	// The m_adtServerPause[Start][End]Times array tracks all the different times that the clock was paused.
	// When we "resume" the clock here, we fill in the latest EndTime entry with this moment in time. These 
	// arrays are ultimately used for saving time slips.
	if (0 == m_adtServerPauseStartTimes.GetSize() || 0 == m_adtServerPauseEndTimes.GetSize()) {
		ThrowNxException("CEMR::ResumeClock failed to resume EMR clock because PauseClock was never called!");
	}
	else if (m_adtServerPauseStartTimes.GetSize() != m_adtServerPauseEndTimes.GetSize()) {
		ThrowNxException("CEMR::ResumeClock failed to resume EMR clock due to inconsistent pause time array sizes!");
	}
	else {
		m_adtServerPauseEndTimes[ m_adtServerPauseEndTimes.GetSize() - 1 ] = dtServerResumeTime;
	}
}

// (z.manning 2011-03-04 14:40) - PLID 42682
void CEMR::UpdatePendingActionSourceDetailStampID(const long nOldDetailStampID, const long nNewDetailStampID, CEMN *pEmn)
{
	for(int nPendingActionIndex = 0; nPendingActionIndex < m_arPendingActions.GetSize(); nPendingActionIndex++)
	{
		PendingAction *pa = m_arPendingActions.GetAt(nPendingActionIndex);
		if(pa->pSourceEmn == pEmn) {
			SourceActionInfo sai = pa->sai;
			if(sai.GetDetailStampID() == nOldDetailStampID) {
				sai.SetDetailStampID(nNewDetailStampID);
				pa->sai = sai;
			}
		}
	}
}

// (z.manning 2011-03-04 14:40) - PLID 42682
void CEMR::UpdatePendingActionSourceDetailPointers(EmrDetailImageStamp *pOldDetailStamp, EmrDetailImageStamp *pNewDetailStamp, CEMN *pEmn)
{
	for(int nPendingActionIndex = 0; nPendingActionIndex < m_arPendingActions.GetSize(); nPendingActionIndex++)
	{
		PendingAction *pa = m_arPendingActions.GetAt(nPendingActionIndex);
		if(pa->pSourceEmn == pEmn) {
			SourceActionInfo sai = pa->sai;
			if(sai.GetDetailStampPointer() == pOldDetailStamp) {
				sai.SetDetailStampPointer(pNewDetailStamp);
				pa->sai = sai;
			}
		}
	}
}

// (z.manning 2011-09-22 14:32) - PLID 45623\
// (s.dhole 2013-06-21 16:58) - PLID 55964 new class
void CEMR::AddReconciledMedChange(CReconcileMedicationsDlg::MergeMedication *pChange)
{
	m_mapNewEMRDataIDToNewCropGUID.SetAt(pChange->nEMRDataID, pChange->strNewCropGUID);
}

// (j.jones 2012-01-31 15:13) - PLID 47878 - HasChangesMadeThisSession will return TRUE
// if m_bChangesMadeThisSession is true for the EMR *OR* for any EMN on the EMR.
BOOL CEMR::HasChangesMadeThisSession()
{
	if(m_bChangesMadeThisSession) {
		return TRUE;
	}

	//anything that set m_bChangesMadeThisSession to TRUE on the EMN
	//should have also set it to true on the parent EMR, but this code
	//will catch a mismatch if one exists
	for(int i = 0; i < m_arypEMNs.GetSize(); i++) {
		if(m_arypEMNs[i]->HasChangesMadeThisSession()) {
			return TRUE;
		}
	}

	return FALSE;
}

// (j.jones 2012-01-31 15:30) - PLID 47878 - added ability to set m_bChangesMadeThisSession from a caller
void CEMR::SetChangesMadeThisSession(BOOL bChangesMade)
{
	m_bChangesMadeThisSession = bChangesMade;
}

// (j.dinatale 2012-07-13 12:25) - PLID 51481 - need this for determining if save should be called
bool CEMR::HasValidInfo()
{
	if(m_aryDeletedEMNs.GetSize() > 0) return TRUE;

	if(!m_bIsTemplate) {
		if(m_nEMRID == -1 && !m_arypEMNs.GetSize())
			return FALSE;
	}

	return TRUE;
}

// (j.jones 2012-08-22 09:08) - PLID 50486 - If m_nDefaultChargeInsuredPartyID is Unknown (-3)
// then this will check the EMNChargeRespDefault preference and fill m_nDefaultChargeInsuredPartyID.
// Once filled, this function will always return m_nDefaultChargeInsuredPartyID.
long CEMR::GetDefaultChargeInsuredPartyID()
{
	try {

		//if the default is Unknown (-3), it means we haven't checked the preference yet
		if(m_nDefaultChargeInsuredPartyID == (long)DefaultChargeInsuredPartyIDType::Unknown) {
			long nEMNChargeRespDefault = GetRemotePropertyInt("EMNChargeRespDefault", -1, 0, GetCurrentUserName(), true);
						
			if(nEMNChargeRespDefault == 1) {
				//1 means primary resp., which means primary medical OR primary vision. 
				//If they have neither, choose patient.
				//If they have both, we need to default to NO resp.

				_RecordsetPtr rs = CreateParamRecordset("SELECT Min(InsuredPartyT.PersonID) AS InsuredPartyID, Count(*) AS CountPrimary "
					"FROM InsuredPartyT "
					"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"WHERE PatientID = {INT} AND RespTypeT.CategoryPlacement = 1", m_nPatientID);
				
				//default to patient if this returns nothing (or NULL)
				m_nDefaultChargeInsuredPartyID = (long)DefaultChargeInsuredPartyIDType::PatientResp;
				if(!rs->eof) {
					m_nDefaultChargeInsuredPartyID = VarLong(rs->Fields->Item["InsuredPartyID"]->Value, (long)DefaultChargeInsuredPartyIDType::PatientResp);
					long nCount = VarLong(rs->Fields->Item["CountPrimary"]->Value, 0);
					if(nCount > 1) {
						//there is more than one primary insurance, so set to no default resp. (-2)
						m_nDefaultChargeInsuredPartyID = (long)DefaultChargeInsuredPartyIDType::NoDefault;
					}
				}
				rs->Close();
			}
			else if(nEMNChargeRespDefault == 0) {
				//0 means patient resp, return PatientResp (-1)
				m_nDefaultChargeInsuredPartyID = (long)DefaultChargeInsuredPartyIDType::PatientResp;
			}
			else {
				//-1 means no responsibility, return NoDefault (-2)
				m_nDefaultChargeInsuredPartyID = (long)DefaultChargeInsuredPartyIDType::NoDefault;
			}
		}

		return m_nDefaultChargeInsuredPartyID;

	}NxCatchAll(__FUNCTION__);

	//if we had an exception, return NoDefault (-2)
	return (long)DefaultChargeInsuredPartyIDType::NoDefault;
}

// (j.jones 2012-10-25 17:18) - PLID 53322 - returns m_bHadSavedAllergies
BOOL CEMR::GetHadSavedAllergies()
{
	return m_bHadSavedAllergies;
}

// (j.jones 2012-10-29 08:55) - PLID 53324 - returns m_bHadSavedCurrentMeds
BOOL CEMR::GetHadSavedCurrentMeds()
{
	return m_bHadSavedCurrentMeds;
}