#include "stdafx.h"
#include "practice.h"
#include "OMRUtils.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include <foreach.h>
#include "NxAPI.h"
#include "EmrOmrMap.h"

// (j.dinatale 2012-08-02 16:11) - PLID 51911 - created

NexTech_COM::INxXmlGeneratorPtr OMRUtils::ProcessXML(const CString &strNxXMLFilePath)
{
	NexTech_COM::INxXmlGeneratorPtr pNxXmlGen;
	pNxXmlGen.CreateInstance(_T("NexTech_COM.NxXmlGenerator"));

	// (j.dinatale 2012-09-19 09:02) - PLID 51911 - dont bother checking for null, we want a NULL pointer exception (experience) when the pointer is bad
	pNxXmlGen->ReadFromFile(_bstr_t(strNxXMLFilePath));

	return pNxXmlGen;

}

// (b.spivey, August 17, 2012) - PLID 51721 - This is how we parse output from Remark. 
void OMRUtils::RemarkOutputProcess(CString strPendingOMRPath, CString strRemarkXMLPath, DWORD dwBatchID, long nFormID)
{
	//We need pointers for the remark document and the xml generator. 
	MSXML2::IXMLDOMDocument2Ptr xmlRemark;
	xmlRemark.CreateInstance(__uuidof(MSXML2::DOMDocument60)); 

	//Variables for various NxXml values. 
	CString strRespondantID; 
	long nPatID = -1;
	long nTemplateID = -1; 

	//necessary for setting up the map and extracting data group IDs. 
	EmrOmrMap eomMap; 
	CMap<long, long, long, long> idMap; 

	eomMap.LoadEmrOmrMap(nFormID, EmrOmrMap::ltsForm); 

	//Get the map set up so we can find the data group ID based on OMR id. 
	for(int i = 0; i < eomMap.GetItemCount(); i++) {
		EmrOmrMap::EmrItemDetail eidItem = eomMap.GetItemAt(i); 
		idMap.SetAt(eidItem.OmrID, eidItem.DataGroupID); 
	}
	nTemplateID = eomMap.GetCurrentTemplateID(); 
	
	//This is the Remark output we need to traverse. 
	xmlRemark->load(_variant_t(strRemarkXMLPath));
	xmlRemark->setProperty("SelectionNamespaces", "xmlns:rs='urn:schemas-microsoft-com:rowset' "
													"xmlns:z='#RowsetSchema'");
	
	//All the data is stored in a <z:row> element, so we need to go line by line inside of it to get what we need. 
	MSXML2::IXMLDOMNodeListPtr xmlRemarkNodeList(xmlRemark->selectNodes(_bstr_t("//z:row"))); 
	for (int i = 0; i < xmlRemarkNodeList->Getlength(); i++) {

		// (b.spivey, September 20, 2012) - PLID 51721 - Reinit this every iteration of the loop. 
		NexTech_COM::INxXmlGeneratorPtr pNxXmlGen;
		pNxXmlGen.CreateInstance(_T("NexTech_COM.NxXmlGenerator"));

		//So the history can be set up with a note. 
		pNxXmlGen->SetOMRFormID(nFormID); 

		//Attributes of the <z:row> element are regions with values from the OMR. 
		MSXML2::IXMLDOMNodePtr xmlRemarkNode(xmlRemarkNodeList->Getitem(i));
		MSXML2::IXMLDOMNamedNodeMapPtr xmlRemarkNodeMap = xmlRemarkNode->Getattributes();
			
		//Gotta go attribute by attribute. 
		for (int i = 0; i < xmlRemarkNodeMap->Getlength(); i++) {
			MSXML2::IXMLDOMNodePtr xmlRemarkAttribute(xmlRemarkNodeMap->Getitem(i));

			//As of this time we only support PatientIDs and DataGroupIDs. 
			//Unfortunately a draw back to remark is every region needs a unique name, so when you run the program it'll take every
			//region that has a repeated name and add a number to it, so we need to look at only the part of the name that we're 
			//expecting not to change. 
			if (VarString(xmlRemarkAttribute->GetnodeName(), "").Left(9).CompareNoCase("PatientID") == 0) {
				_variant_t varVal = xmlRemarkAttribute->GetnodeValue();
				ADODB::_RecordsetPtr rs = CreateParamRecordset(
					"SELECT PersonID FROM PatientsT WHERE UserDefinedID = {INT}", AsLong(varVal)); 
				if (!rs->eof) {
					nPatID = AdoFldLong(rs->Fields, "PersonID", -1); //For the NxXML.
				}
				strRespondantID = AsString(varVal); //For file searching.
			}
			else if (VarString(xmlRemarkAttribute->GetnodeName(), "").Left(4).CompareNoCase("Item") == 0){
				_variant_t varVal = xmlRemarkAttribute->GetnodeValue();
				if (AsString(varVal).Left(1).CompareNoCase("(") == 0) {
					CString strValList = AsString(varVal);
					strValList = strValList.Trim("(").Trim(")");
					CStringArray strIDArray;
					SplitString(strValList, ",", &strIDArray); 
					for (int i = 0; i < strIDArray.GetSize(); i++) {
						long nOmrID = atoi(strIDArray.GetAt(i)); //To seach for the DataGroupID. 
						long nDGID; 
						//Only if we find this OMRID do we add it to the NxXml. 
						if (idMap.Lookup(nOmrID, nDGID)) {
							pNxXmlGen->AddValue(nDGID); 
						}
					}
				}
				else {
					long nOmrID = AsLong(varVal); //To seach for the DataGroupID. 
					long nDGID; 
					//Only if we find this OMRID do we add it to the NxXml. 
					if (idMap.Lookup(nOmrID, nDGID)) {
						pNxXmlGen->AddValue(nDGID); 
					}
				}
			}
		}

		//We need to find the jpgs in the pending OMR Path and link them to the NxXML. They will always be jpgs. 
		WIN32_FIND_DATA FindFileData;
		// (b.spivey, February 27, 2013) - PLID 54717 - We're looking for PDFs. 
		HANDLE hFind = FindFirstFile(strPendingOMRPath ^ "*.pdf", &FindFileData);

		if(hFind != INVALID_HANDLE_VALUE) {
			bool bContinue = true;

			CString strSearchName = "";
			strSearchName.Format("%s%s%lu", strRespondantID, eomMap.GetCurrentDesc(), dwBatchID);
			long nSearchNameLength = strSearchName.GetLength(); 
			while (bContinue) {
				//Same problem up above-- Remark is going to append datetimes to images that it creates with names already existing. 
				//So we only look for the respondant ID. In this case it can vary, so we account for that. 
				if((CString(FindFileData.cFileName).Left(nSearchNameLength).CompareNoCase(strSearchName) == 0)) {
					//Found the file, but we need the absolute path. 
					pNxXmlGen->AddFilePath(_bstr_t(strPendingOMRPath  ^ FindFileData.cFileName)); 
				}

				if(!FindNextFile(hFind, &FindFileData)) {
					bContinue = false; 
				}
			}
		}

		//Unfortunately we can't be sure we have the patientID until after the parsing so that's when we finally add it to the NxXML. 
		pNxXmlGen->Initialize(nPatID, nTemplateID); 

		CString strFileName;
		//Use the patient ID as the significant part of the name and append a generic text name after that. 
		strFileName.Format("%liPendingOMRScan.xml", nPatID);
		pNxXmlGen->WriteToFile(_bstr_t(FileUtils::GetUniqueFileName(strPendingOMRPath ^ strFileName)));	
	}
	
	//Nothing to return.
}

// (j.dinatale 2012-08-09 16:59) - PLID 52060 - be able to commit NxXML via the API
long OMRUtils::CommitNxXMLToEMN(NexTech_COM::INxXmlGeneratorPtr pNxXml)
{
	if(!pNxXml)
		return -1;

	long nPatientID = pNxXml->GetPatientID();
	long nTemplateID = pNxXml->GetTemplateID();

	if(nPatientID <= 0 || nTemplateID <= 0){
		return -1;
	}

	long nUserDefinedID = GetExistingPatientUserDefinedID(nPatientID);
	if(nUserDefinedID <= 0)
		return -1;

	CString strUserDefined, strTemplateID, strFormName;
	strUserDefined.Format("%li", nUserDefinedID);
	strTemplateID.Format("%li", nTemplateID);
	
	// (b.spivey, February 27, 2013) - PLID 55090 - Get the OMR Form name 
	ADODB::_RecordsetPtr rsFormName = CreateParamRecordset("SELECT Description FROM OMRFormT WHERE ID = {INT} ", 
		pNxXml->GetOMRFormID());

	strFormName = "OMR Form Scan";
	// (b.spivey, February 27, 2013) - PLID 55090 - If we got something, put a value here. 
	if(!rsFormName->eof) {
		strFormName = AdoFldString(rsFormName->Fields, "Description", "OMR Form Scan"); 
	}

	// (b.spivey, February 27, 2013) - PLID 55090 - Use form name instead of an empty string.
	NexTech_Accessor::_CreateEMNFromTemplateResultPtr pCreateResults = GetAPI()->CreateEMNFromTemplate(GetAPISubkey(), GetAPILoginToken(),
		_bstr_t(strTemplateID), _bstr_t(strUserDefined), _bstr_t(), _bstr_t(strFormName));

	if(!pCreateResults || !pCreateResults->Header)
		return -1;

	CString strEMNID = (LPCTSTR)pCreateResults->Header->ID;
	long nEMNID = atoi(strEMNID);
	long nOMRFormID = pNxXml->GetOMRFormID();

	using namespace NexTech_Accessor;
	struct DataGroupInfo{
		public:
			long nDataGroupID;
			long nDataID;
			long nTemplateDetailID;
	};

	CMap<long, long, DataGroupInfo, DataGroupInfo> mapDataGoupIDtoInfo;
	// (b.spivey, April 24, 2013) - PLID 56198 - set this up ahead of time, it'll tell us what values to clear out. 
	CMap<long, long, _EMNDetailCommitPtr, _EMNDetailCommitPtr> mapDetailIDtoValues;
	ADODB::_RecordsetPtr rs = CreateParamRecordset(
		"SELECT DISTINCT "
		"	EMRDataT.EMRDataGroupID AS DataGroupID, "
		"	EMRDataT.ID AS DataID, "
		"	EMRDetailsT.ID AS TemplateDetailID "
		"FROM EMRDetailsT "
		"LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"LEFT JOIN EMRDataT ON EMRInfoT.ID = EMRDataT.EMRInfoID "
		"LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID "
		"LEFT JOIN OMRFormT ON EMRMasterT.TemplateID = OMRFormT.EMRTemplateID "
		"INNER JOIN OMRFormDetailT ON OMRFormT.ID = OMRFormDetailT.OMRFormID "
		"	AND OMRFormDetailT.EmrDataGroupID = EMRDataT.EmrDataGroupID "
		"WHERE EMRDetailsT.EMRID = {INT} AND OMRFormT.ID = {INT} "
		"ORDER BY EMRDetailsT.ID ",
		nEMNID, nOMRFormID);

	while(!rs->eof){
		DataGroupInfo dgInfo;
		dgInfo.nDataGroupID = AdoFldLong(rs, "DataGroupID", -1);
		dgInfo.nDataID = AdoFldLong(rs, "DataID", -1);
		dgInfo.nTemplateDetailID = AdoFldLong(rs, "TemplateDetailID", -1);
		// (b.spivey, April 24, 2013) - PLID 56198 - Clear these values are they are mapped, we'll replace them later. 
		_EMNDetailCommitPtr pDetail;
		if(!mapDetailIDtoValues.Lookup(dgInfo.nTemplateDetailID, pDetail)){
			pDetail.CreateInstance(__uuidof(EMNDetailCommit));

			CString strTemplateID;
			strTemplateID.Format("%li", dgInfo.nTemplateDetailID);
			pDetail->ID = (LPCTSTR)strTemplateID;
			pDetail->value = (LPCTSTR)"";

			mapDetailIDtoValues.SetAt(dgInfo.nTemplateDetailID, pDetail);
		}

		mapDataGoupIDtoInfo.SetAt(dgInfo.nDataGroupID, dgInfo);

		rs->MoveNext();
	}

	rs->Close();

	Nx::SafeArray<long> saryDataGroupIDs(pNxXml->GetDataGroupIDs());

	foreach(long nDataGroupID, saryDataGroupIDs)
	{
		DataGroupInfo dgInfo;
		if(mapDataGoupIDtoInfo.Lookup(nDataGroupID, dgInfo)){
			_EMNDetailCommitPtr pDetail;
			// (b.spivey, April 24, 2013) - PLID 56198 - Now assign the values. 
			if(mapDetailIDtoValues.Lookup(dgInfo.nTemplateDetailID, pDetail)){
				CString strToAdd;
				strToAdd.Format(",%li", dgInfo.nDataID);

				CString strValue = (LPCTSTR) pDetail->value;
				strValue += strToAdd;
				strValue.TrimLeft(",");
				pDetail->value = _bstr_t(strValue);
			} 
			else{
				//Just eat the message. 
			}
		}
	}

	Nx::SafeArray<IUnknown *> saryDetailPtrs;
	foreach(_EMNDetailCommitPtr pDet, mapDetailIDtoValues | boost::adaptors::map_values){
		saryDetailPtrs.Add(pDet);

		// (j.jones 2012-11-29 13:48) - PLID 53946 - request the detail
		GetAPI()->RequestEMNDetail(GetAPISubkey(), GetAPILoginToken(), pDet->ID);
	}

	// if we have anything, then we commit, otherwise its just a pointless trip.
	if(saryDetailPtrs.GetCount()){
		// (j.jones 2012-11-29 13:48) - PLID 53946 - this changed in the API and now requires four parameters,
		// but the final parameter, direction, is empty here
		GetAPI()->CommitMultipleEMNDetails(GetAPISubkey(), GetAPILoginToken(), saryDetailPtrs, "");
	}

	GetAPI()->ReleaseEMNAccess(GetAPISubkey(), GetAPILoginToken(), _bstr_t(strEMNID));

	//table check EMRMasterT, so we refresh all our EMN lists
	CClient::RefreshTable(NetUtils::EMRMasterT, nPatientID);

	return nEMNID;
}

// (j.dinatale 2012-08-21 16:52) - PLID 52284 - attach files to history
bool OMRUtils::AttachFilesFromNxXML(NexTech_COM::INxXmlGeneratorPtr pNxXml, long nEMNID)
{
	// ensure we have a valid NxXML object and a good EMN ID
	if(!pNxXml || nEMNID <= 0)
		return false;

	// check if we have a valid patient ID
	long nPatientID = pNxXml->GetPatientID();
	if(nPatientID <= 0){
		return false;
	}

	// get a list of files we wish to import
	Nx::SafeArray<BSTR> saryFiles(pNxXml->GetFiles());
	FileFolder ff;
	bool bAtLeastOneFail = false;	// (j.dinatale 2012-09-19 09:56) - PLID 52284 - need to see if all our files exist as well
	foreach(_bstr_t strPath, saryFiles){
		bool bWillFail = !FileUtils::DoesFileOrDirExist(strPath);
		if(!bWillFail){
			CString strFilePath = (LPCSTR)strPath;
			ff.saFileNames.Add(strFilePath);
		}

		bAtLeastOneFail = (bAtLeastOneFail || bWillFail);
	}

	// no files to import
	if(!ff.saFileNames.GetCount())
		return !bAtLeastOneFail;

	long nFormID = pNxXml->GetOMRFormID();
	CString strFormName;

	// get the form name
	// get the pic ID for the pic we wish to attach our files to
	long nPicID = -1;
	ADODB::_RecordsetPtr rsFormInfo = CreateParamRecordset(
		"SELECT Description FROM OMRFormT WHERE ID = {INT}; "
		"SELECT PicT.ID AS PicID FROM EMRMasterT "
		"LEFT JOIN EMRGroupsT ON EMRMasterT.EMRGroupID = EMRGroupsT.ID "
		"LEFT JOIN PicT ON EMRGroupsT.ID = PicT.EmrGroupID "
		"WHERE EMRMasterT.ID = {INT} ", nFormID, nEMNID);

	// form name
	if(rsFormInfo && !rsFormInfo->eof){
		strFormName = AdoFldString(rsFormInfo, "Description", "");
	}

	// next recordset
	if(rsFormInfo){
		rsFormInfo = rsFormInfo->NextRecordset(NULL);
	}

	// pic ID
	if(rsFormInfo && !rsFormInfo->eof){
		nPicID = AdoFldLong(rsFormInfo, "PicID", -1);
	}

	// fire away, capt'n
	// (j.fouts 2013-02-27 17:00) - PLID 54715 - Import to the their default category
	long nDefaultCategoryID = GetRemotePropertyInt("OMRDefaultImportCategory", -1);
	if(!ImportAndAttachFolder(ff, nPatientID, NULL, nDefaultCategoryID, "", nPicID, FALSE, _bstr_t(strFormName), NULL, COleDateTime::GetCurrentTime(), NULL, true)){
		return false;
	}

	return !bAtLeastOneFail;
}

// (j.dinatale 2012-08-22 11:14) - PLID 52256 - be able to clean up our NxXML files, including the attached documents
bool OMRUtils::DeleteNxXMLFiles(const CString &strNxXMLPath, NexTech_COM::INxXmlGeneratorPtr pNxXml, bool bSkipAttachments /*= false*/ )
{
	bool bFailed = false;

	if(pNxXml && !bSkipAttachments){
		Nx::SafeArray<BSTR> saryFiles(pNxXml->GetFiles());
		foreach(_bstr_t strPath, saryFiles){
			CString strFilePath = (LPCSTR)strPath;

			if(FileUtils::DoesFileOrDirExist(strFilePath)){
				// (b.spivey, February 28, 2013) - PLID 54717 - Delete when possible, because of the switch in 
				//		controls this may take a moment or two. 
				bFailed = (DeleteFileWhenPossible(strFilePath) || bFailed);
			}
		}
	}

	if(FileUtils::DoesFileOrDirExist(strNxXMLPath)){
		// (b.spivey, February 28, 2013) - PLID 54717 - Delete when possible, because of the switch in 
		//		controls this may take a moment or two. 
		bFailed = (DeleteFileWhenPossible(strNxXMLPath) || bFailed);
	}

	return !bFailed;
}

// (b.savon 2013-02-28 12:40) - PLID 54714 - Validation utility function.  Takes in the data type, right now all that is
// supported are single and multi-select items.  Takes in the count selected in the item.  If more than 1 item is selected
// on a single select list, then set the warning flag.  Returns the color for the parent row:
// Green -> Valid, Red -> Error, Orange -> Empty
OLE_COLOR OMRUtils::GetOMRItemValidation(long nEMRDataType, long nCountSelected, bool &bWarning)
{
	switch( nEMRDataType ){
		case EmrOmrMap::edtSingleSel:
			{
				if( nCountSelected == 0 ){
					return OMR_EMPTY;
				}else if( nCountSelected == 1 ){
					return OMR_SUCCESS;
				}else if ( nCountSelected > 1 ){
					bWarning = true;
					return OMR_ERROR;
				}
			}
			break;
		case EmrOmrMap::edtMultiSel:
			{
				if( nCountSelected == 0 ){
					return OMR_EMPTY;
				}else if( nCountSelected > 0 ){
					return OMR_SUCCESS;
				}
			}
			break;
		default:
			//Ride it out, return the error.
			break;
	}

	return OMR_ERROR;
}