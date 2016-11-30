#include "stdafx.h"
#include "Practice.h"
#include "EmrOmrMap.h"
#include "GlobalParamUtils.h"
#include "NxAPI.h" 

// (b.spivey, July 31, 2012) - PLID 51878 - Created. 

EmrOmrMap::EmrOmrMap(void)
{
	m_nFormID = -1;
	m_bModified = false; 
}

EmrOmrMap::~EmrOmrMap(void)
{
}

//We can initialize with a formID
EmrOmrMap::EmrOmrMap(long nFormID)
{
	m_nFormID = nFormID;
	m_bModified = false; 
	LoadEmrOmrMap(nFormID, ltsForm); 
}

//Saving the map is as simple as deleting and inserting into OMRFormDetailT
void EmrOmrMap::SaveEmrOmrMap()
{
	CSqlFragment sqlFrag; 
	sqlFrag += CSqlFragment("DELETE FROM OMRFormDetailT WHERE OMRFormID = {INT} ", 
		m_nFormID);

	//If the desccriptions changed, then we need to update the database.
	if (m_nOriginalTemplateID != m_nCurrentTemplateID) {
		sqlFrag += CSqlFragment("UPDATE OMRFormT SET EMRTemplateID = {INT} WHERE ID = {INT} ", 
			m_nCurrentTemplateID, m_nFormID); 
		m_nOriginalTemplateID = m_nCurrentTemplateID; 
	}

	for(int i = 0; i < m_aryDetails.GetSize(); i++) {
		EmrItemDetail tempItem = m_aryDetails.GetAt(i);
		long nDataGroupID = tempItem.DataGroupID; 
		long nOmrID = tempItem.OmrID; 

		//nOmrID < 0 is the same as null, don't save.
		if(nDataGroupID > -1 && nOmrID > -1) {
			sqlFrag += CSqlFragment("INSERT INTO OMRFormDetailT (OMRFormID, EmrDataGroupID, OMRID) "
				"VALUES ({INT}, {INT}, {INT}) ", m_nFormID, nDataGroupID, nOmrID); 
		}
	}

	ExecuteParamSql(sqlFrag); 
	m_bModified = false; 
}

//We can load from a form (established), or from a template (new).
void EmrOmrMap::LoadEmrOmrMap(long nLoadingID, LoadingTypeSelection ltsSel)
{
	m_aryDetails.RemoveAll(); 
	ADODB::_RecordsetPtr prs;
	if (ltsSel == ltsForm) {
		m_bModified = false; 
		prs = CreateParamRecordset(EmrOmrFormFrag(nLoadingID));
	}
	else if (ltsSel == ltsTemplate) {
		m_bModified = true; 
		prs = CreateParamRecordset(EmrOmrTemplateFrag(nLoadingID));
		m_nCurrentTemplateID = nLoadingID; 
	}

	CMap<long, long, long, long> mapStartingIndex;
	while(!prs->eof){
		EmrItemDetail tempDetail;

		tempDetail.ItemID = AdoFldLong(prs, "DetailID", -1);
		tempDetail.ItemName = AdoFldString(prs, "ItemName", "{No Item Name}");
		tempDetail.DataGroupID = AdoFldLong(prs, "DataGroupID", -1);
		tempDetail.SelectionText = AdoFldString(prs, "SelectionText", "{No Selection Text}");
		tempDetail.OmrID = AdoFldLong(prs, "OMRID", -1);
		// (b.spivey, August 31, 2012) - PLID 51878 - support data type.
		tempDetail.DataType = (EMNDataType)AdoFldByte(prs, "DataTypeID", -1); //tinyint/byte 
		// (b.spivey, March 01, 2013) - PLID 53955 - Map the startin indices. 
		tempDetail.TemplateDetailID = AdoFldLong(prs, "TemplateDetailID", -1); 
		long index = m_aryDetails.Add(tempDetail); 
		long nil = -1; 
		if(!mapStartingIndex.Lookup(tempDetail.TemplateDetailID, nil)) {
			mapStartingIndex.SetAt(tempDetail.TemplateDetailID, index); 
		}
		prs->MoveNext(); 
	}

	//Load description. 
	prs = CreateParamRecordset("SELECT * FROM OMRFormT WHERE ID = {INT} ", m_nFormID); 
	if (!prs->eof) {
		m_strCurrentDesc = AdoFldString(prs, "Description", ""); 

		m_strFileLocation = AdoFldString(prs, "FilePath", ""); 

		//Track the template, only change this if they load from template. 
		if (ltsSel == ltsForm) {
			m_nOriginalTemplateID = AdoFldLong(prs, "EMRTemplateID");
			m_nCurrentTemplateID = m_nOriginalTemplateID;
		}
	}
	else {
		m_strCurrentDesc = "";
		m_strFileLocation = "";
	}

	// (b.spivey, April 25, 2013) - PLID 56438 - We can safely assumet his template doesn't exist, 
	//		trying to load it would cause an API error. 
	if(m_aryDetails.GetCount() > 0) {
		SortDetailsArray(mapStartingIndex);
	}
}

// (b.spivey, February 28, 2013) - PLID 53955 - Start the sort process. 
void EmrOmrMap::SortDetailsArray(CMap<long, long, long, long>& mapStartingIndex) 
{
	NexTech_Accessor::_EMRTemplatePtr pTemplate = GetAPI()->GetEMRTemplate(
			GetAPISubkey(), GetAPILoginToken(),  _bstr_t(m_nCurrentTemplateID));

	//If we have some root level topics, we can start sorting this. 
	if (NULL != pTemplate->RootLevelTopics) {
		CArray<EmrItemDetail, EmrItemDetail> arySortDetails; 
		arySortDetails.Copy(m_aryDetails); 
		m_aryDetails.RemoveAll(); 

		SortArrayRecurse(Nx::SafeArray<IUnknown *>(pTemplate->RootLevelTopics), arySortDetails, mapStartingIndex); 

	}

	return;
}

// (b.spivey, March 04, 2013) - PLID 53955 - Recursively call this function to sort all the items. 
void EmrOmrMap::SortArrayRecurse(Nx::SafeArray<IUnknown *> ptrTopics, CArray<EmrItemDetail, EmrItemDetail>& arySortDetails,
								 CMap<long, long, long, long>& mapStartingIndex)
{
	//For every topic.
	foreach(NexTech_Accessor::_EMRTemplateTopicPtr pTopic, ptrTopics)
	{
		//Check for details. 
		if(pTopic->details != NULL) {
			Nx::SafeArray<IUnknown *> ptrDetails = Nx::SafeArray<IUnknown *>(pTopic->details);
			//For every detail
			foreach(NexTech_Accessor::_EMRTemplateDetailPtr pDetail, ptrDetails)
			{
				//We see if it's in the map. 
				long index = 0; 
				long nDetailID = atoi((LPCTSTR)(pDetail->ID));
				CString strDetailName = (LPCTSTR)pDetail->Name;
				if(mapStartingIndex.Lookup(nDetailID, index)) {
					//And if it's in the map we start adding the selections until the detail ID changes.
					while (index < arySortDetails.GetSize() && nDetailID == arySortDetails.GetAt(index).TemplateDetailID) {
						m_aryDetails.Add(arySortDetails.GetAt(index)); 
						index++; 
					}
				}
			}
		}

		//If we have children topics, call it again. 
		if(pTopic->ChildTopics != NULL) {
			SortArrayRecurse(Nx::SafeArray<IUnknown *>(pTopic->ChildTopics), arySortDetails, mapStartingIndex);
		}
	}

}

//Seperate function for the EmrOmrForm fragment. 
CSqlFragment EmrOmrMap::EmrOmrFormFrag(long nFormID) 
{
	// (b.spivey, October 25, 2012) - PLID 53435 - Filter on ListType = 1, means it's from a list. Order by SortOrder, not ID. 
	// (b.spivey, November 30, 2012) - PLID 53435 - Inactive selections too.
	m_nFormID = nFormID; 
	return CSqlFragment(
		 "SELECT Template.ID AS TemplateID, Template.Name AS TemplateName, Info.ID AS DetailID, Info.Name AS ItemName, "
		 "	Data.Data AS SelectionText, Data.EmrDataGroupID AS DataGroupID, "
		 "	OMR.OmrID, Info.DataType AS DataTypeID, TemplateDetail.ID  AS TemplateDetailID "
		 "FROM OMRFormT OMRForm " 
		 "INNER JOIN EMRTemplateT Template ON OMRForm.EMRTemplateID = Template.ID "
		 "INNER JOIN EMRTemplateDetailsT TemplateDetail ON Template.ID = TemplateDetail.TemplateID "
		 "INNER JOIN EMRInfoMasterT InfoMaster ON TemplateDetail.EmrInfoMasterID = InfoMaster.ID "
		 "INNER JOIN EMRInfoT Info ON InfoMaster.ActiveEmrInfoID = Info.ID "
		 "INNER JOIN EMRDataT Data ON Info.ID = Data.EMRInfoID "
		 "LEFT JOIN OMRFormDetailT OMR ON OMR.EmrDataGroupID = Data.EmrDataGroupID AND OMR.OMRFormID = OMRForm.ID "
		 "WHERE Info.DataType IN (2,3) "
		 "	AND OMRForm.ID = {INT} "
		 "	AND Data.IsLabel <> 1 "
		 "	AND Data.ListType = 1 "
		 "	AND Data.Inactive <> 1 "
		 "ORDER BY Template.ID, Info.ID, "
		// (b.spivey, March 08, 2013) - PLID 53955 - Added an additional clause to handle Auto-Alphabetize 
		 "(CASE WHEN Info.AutoAlphabetizeListData = 1 "
		 "			THEN "
		 "			CASE WHEN DataSubType IN (1,2) "
		 "				THEN Data.SortOrder "
		 "				ELSE -1 END "
		 "			ELSE Data.SortOrder "
		 "		END), "
		 "		"
		 "		(CASE WHEN Info.AutoAlphabetizeListData = 1 "
		 "				THEN "
		 "				CASE WHEN DataSubType IN (1,2) "
		 "					THEN '' "
		 "					ELSE CAST(Data.Data AS VARCHAR(2000)) END "
		 "			ELSE '' "
		 "		END) ", m_nFormID); 
}

//Seperate function for the EmrOmrTemplate fragment. 
CSqlFragment EmrOmrMap::EmrOmrTemplateFrag(long nTemplateID)
{
	// (b.spivey, October 25, 2012) - PLID 53435 - Filter on ListType = 1, means it's from a list. Order by SortOrder, not ID. 
	// (b.spivey, November 30, 2012) - PLID 53435 - Inactive selections too.
	return CSqlFragment(
		 "SELECT Template.ID AS TemplateID, Template.Name AS TemplateName, Info.ID AS DetailID, Info.Name AS ItemName, "
		 "	Data.Data AS SelectionText, Data.EmrDataGroupID AS DataGroupID, "
		 "	NULL AS OmrID, Info.DataType AS DataTypeID, TemplateDetail.ID  AS TemplateDetailID "
		 "FROM EMRTemplateT Template "
		 "INNER JOIN EMRTemplateDetailsT TemplateDetail ON Template.ID = TemplateDetail.TemplateID "
		 "INNER JOIN EMRInfoMasterT InfoMaster ON TemplateDetail.EmrInfoMasterID = InfoMaster.ID "
		 "INNER JOIN EMRInfoT Info ON InfoMaster.ActiveEmrInfoID = Info.ID "
		 "INNER JOIN EMRDataT Data ON Info.ID = Data.EMRInfoID "
		 "WHERE Info.DataType IN (2,3) "
		 "	AND Template.ID  = {INT} "
		 "	AND Data.IsLabel <> 1 "
		 "	AND Data.ListType = 1 "
		 "	AND Data.Inactive <> 1 "
		 "ORDER BY Template.ID, Info.ID, "
		// (b.spivey, March 08, 2013) - PLID 53955 - Added an additional clause to handle Auto-Alphabetize 
		 "(CASE WHEN Info.AutoAlphabetizeListData = 1 "
		 "			THEN "
		 "			CASE WHEN DataSubType IN (1,2) "
		 "				THEN Data.SortOrder "
		 "				ELSE -1 END "
		 "			ELSE Data.SortOrder "
		 "		END), "
		 "		"
		 "		(CASE WHEN Info.AutoAlphabetizeListData = 1 "
		 "				THEN "
		 "				CASE WHEN DataSubType IN (1,2) "
		 "					THEN '' "
		 "					ELSE CAST(Data.Data AS VARCHAR(2000)) END "
		 "			ELSE '' "
		 "		END) ", nTemplateID);  
}

//Clean up the object after it gets deleted. 
void EmrOmrMap::ClearMap() 
{
	//Set everything to defaults. 
	m_nFormID = -1; 
	m_strCurrentDesc = "";
	m_strFileLocation = "";
	m_nOriginalTemplateID = -1;
	m_nCurrentTemplateID = -1;
	m_bModified = false; 
	m_aryDetails.RemoveAll(); 
}
//Returns the new form ID. 
long EmrOmrMap::CreateNewMap(long nTemplateID, CString strDescription) 
{
	// (b.spivey, August 31, 2012) - PLID 51878 - Bug because I didn't include column names I'm inserting 
	//		into and then the data structure changed.
	ADODB::_RecordsetPtr prs = CreateParamRecordset("SET NOCOUNT ON "
		"DECLARE @FormID INT "
		"INSERT INTO OMRFormT (EMRTemplateID, Description) VALUES ({INT}, {STRING}) "
		"SET @FormID = SCOPE_IDENTITY() "
		"SET NOCOUNT OFF " 
		"SELECT * FROM OMRFormT WHERE ID = @FormID ", nTemplateID, strDescription);

	ClearMap(); 
	m_nFormID = AdoFldLong(prs, "ID", -1); 
	LoadEmrOmrMap(m_nFormID, ltsForm);  
	return m_nFormID; 
}

// (b.spivey, September 06, 2012) - PLID 52456 - Modify the OMR-EMR ids. 
//Modifying the value -- we need to find the datagroupID and give it the OmrID. 
void EmrOmrMap::ModifyOmrMapValue(long nDataGroupID, long nOmrID, bool bDelete /* = false */) 
{
	
	for(int i = 0; i < m_aryDetails.GetSize(); i++) 
	{
		EmrItemDetail tempDetail = m_aryDetails.GetAt(i);

		if (tempDetail.DataGroupID == nDataGroupID) {
			if (bDelete) {
				// (b.spivey, September 19, 2012) - PLID 52456 - This is simulating "delete." 
				//	 We can't remove these because it will allow duplicates later. 
				m_aryDetails.GetAt(i).OmrID = -1; 
			}
			else {
				m_aryDetails.GetAt(i).OmrID = nOmrID; 
			}
			m_bModified = true; 
			break;
		}
	}
} 

//Get the item at the given index. 
EmrOmrMap::EmrItemDetail EmrOmrMap::GetItemAt(long nIndex)
{
	return m_aryDetails.GetAt(nIndex); 
}
//Get item count. 
long EmrOmrMap::GetItemCount() 
{
	return m_aryDetails.GetCount(); 
}

//Delete a form. 
void EmrOmrMap::DeleteEmrOmrMap() 
{
	ExecuteParamSql(
	"BEGIN TRAN "
	"DECLARE @FormID INT " 
	"SET @FormID = {INT} "
	"DELETE FROM OMRFormDetailT WHERE OMRFormID = @FormID "
	"DELETE FROM OMRFormT WHERE ID = @FormID "
	"COMMIT TRAN ", m_nFormID); 

	//This should get called every time. 
	ClearMap(); 
}

//Get modified status. 
bool EmrOmrMap::GetModifiedStatus() 
{
	return m_bModified;
}

// (b.spivey, September 06, 2012) - PLID 52456 - Update description. 
//Update description. 
void EmrOmrMap::SetDescription(CString strDescription) 
{
	if (m_strCurrentDesc.Compare(strDescription)) {
		//Update instantly, it's just a name. 
		ExecuteParamSql("UPDATE OMRFormT SET Description = {STRING} WHERE ID = {INT} ", 
			strDescription, m_nFormID);

		m_strCurrentDesc = strDescription; 
	}
}

// (b.spivey, September 06, 2012) - PLID 52456 - Get original description
// (b.spivey, September 13, 2012) - PLID 52456 - Only one Desc. variable. 

// (b.spivey, September 06, 2012) - PLID 52456 - Get current description.
//Get Current Description 
CString EmrOmrMap::GetCurrentDesc() 
{
	return m_strCurrentDesc; 
}

// (b.spivey, September 06, 2012) - PLID 52456 - Get current template. 
//current template
long EmrOmrMap::GetCurrentTemplateID()
{
	return m_nCurrentTemplateID; 
}

// (b.spivey, September 06, 2012) - PLID 52456 - Get current form ID
//current form
long EmrOmrMap::GetCurrentFormID()
{
	return m_nFormID;
}

// (b.spivey, September 06, 2012) - PLID 52456 - set current form location. 
//get/set current omr file location.
void EmrOmrMap::SetFileLocation(CString strFileLocation) 
{
	ExecuteParamSql("UPDATE OmrFormT SET FilePath = {STRING} WHERE ID = {INT}", strFileLocation, m_nFormID);
	m_strFileLocation = strFileLocation; 
	return;
}

// (b.spivey, September 06, 2012) - PLID 52456 - Get current form location. 
CString EmrOmrMap::GetFileLocation() 
{
	return m_strFileLocation;
}