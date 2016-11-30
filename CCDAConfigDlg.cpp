// CCDAConfigDlg.cpp : implementation file
//

// (j.gruber 2013-10-17 09:36) - PLID 59062
#include "stdafx.h"
#include "Practice.h"
#include "CCDAConfigDlg.h"

#include "CCDAAutoExportDlg.h"

using namespace ADODB;

#define ID_REMOVE_ITEM        52346

#define PARENT_ROW_COLOR RGB(211, 219, 253)
#define CHILD_ROW_COLOR RGB(235,239,253)

// (a.walling 2014-04-24 12:00) - VS2013 - map::insert was using explicitly qualified make_pair template, failing due to rvalue refs

enum DocumentListColumns
{
	dlcID = 0,
	dlcName,
};

enum MajorSectionColumns
{
	mscID = 0,
	mscName,
};

enum SectionColumnList
{
	slcID = 0,
	slcName,
};

enum FieldColumnsList
{
	flcID = 0,
	flcName,
	flcDescription,
};

enum TreeColumnList
{

	tclID=0,
	tclParentID,
	tclName,
	tclDescription,
};

enum EMRInfoColumn
{
	eicID=0,
	eicName,
	eicDataType,
	eicDataTypeName,
	eicIsFlipped,
};


//these are only the ones that we can define
enum ConfigListColumns
{
	clcItemID=0,
	clcIsFlipped,
	clcItemDataType,
	clcItemName,
	clcGenerationType,
};

#define MAX_CONFIG_DEF_COLS clcGenerationType
// CCCDAConfigDlg dialog

IMPLEMENT_DYNAMIC(CCCDAConfigDlg, CNxDialog)

CCCDAConfigDlg::CCCDAConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCCDAConfigDlg::IDD, pParent)
{

}

CCCDAConfigDlg::~CCCDAConfigDlg()
{
	
}

void CCCDAConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCCDAConfigDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CCCDAConfigDlg::OnBnClickedOk)	
	ON_COMMAND(ID_REMOVE_ITEM, OnRemoveItem)
	ON_BN_CLICKED(IDC_CCDA_AUTO_EXPORT_BTN, &CCCDAConfigDlg::OnBnClickedCcdaAutoExportBtn)
END_MESSAGE_MAP()


// CCCDAConfigDlg message handlers



void CCCDAConfigDlg::OnBnClickedOk()
{
	try {

		//save the last one we have
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTreeList->CurSel;
		if (pRow) 
		{
			CCDASectionPtr pSection = FindTreeSection(pRow);
			if (!SaveFields(pSection))
			{
				return;
			}
		}			

		CCDAXmlDocument xmlSaveDocument;
		CString strOutput = xmlSaveDocument.GenerateSaveText(m_mapDocuments);

		//write it to the data
		SetRemotePropertyMemo("CCDAConfigurationXML", strOutput, 0, "<None>");

		CNxDialog::OnOK();
		
	}NxCatchAll(__FUNCTION__);
	
}


BOOL CCCDAConfigDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		//bind our controls
		m_pTreeList = BindNxDataList2Ctrl(IDC_CCDA_TREE_LIST, false);
		m_pFieldDataList = BindNxDataList2Ctrl(IDC_FIELD_DATA_LIST, false);	
		m_pEMRInfoList = BindNxDataList2Ctrl(IDC_EMR_INFO_LIST, true);	
		m_pConfigList = BindNxDataList2Ctrl(IDC_OTHER_LIST, false);		
		m_pDocumentList = BindNxDataList2Ctrl(IDC_CCDA_DOCUMENT_LIST, false);

		// (j.jones 2015-02-17 10:44) - PLID 64880 - clear the last item warning
		m_strLastRemovedItemWarning = "";
		
		AutoSet(IDOK, NXB_OK);
		AutoSet(IDCANCEL, NXB_CANCEL);
		AutoSet(IDC_CCDA_AUTO_EXPORT_BTN, NXB_MODIFY);

		LoadXML();

		SetDlgItemText(IDC_ST_INSTRUCTIONS, "Choose the CCDA document you'd like to configure in the list to the left, "
			"then use the upper tree list to choose the sections in the document. "
			"Then, choose the corresponding EMR item for that section from the drop down list "
			"and configure the fields for the selected section in the bottom list.");

		//load the documents map
		CCDADocumentMap::iterator itDoc;
		for (itDoc = m_mapDocuments.begin(); itDoc != m_mapDocuments.end(); itDoc++)
		{
			CString strDocName = itDoc->first;
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDocumentList->GetNewRow();

			pRow->PutValue(dlcID, _variant_t(strDocName));
			pRow->PutValue(dlcName, _variant_t(strDocName));

			m_pDocumentList->AddRowAtEnd(pRow, NULL);			
		}		
		//set the selection to our first item
		m_pDocumentList->CurSel = m_pDocumentList->GetFirstRow();
		SelChosenCcdaDocumentList(m_pDocumentList->GetFirstRow());

		//disable the fields list until they pick a section		
		m_pConfigList->Enabled = false;
		m_pEMRInfoList->Enabled = false;
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CCCDAConfigDlg::LoadTree(NXDATALIST2Lib::IRowSettingsPtr pParentRow, CCDASectionMap map)
{
	CString strParentID;
	if (pParentRow) {
		strParentID = VarString(pParentRow->GetValue(tclID));
	}

	CCDASectionIterator it;
	for(it = map.begin(); it != map.end(); it++) 
	{
		CCDASectionPtr section = it->second;

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTreeList->GetNewRow();
		pRow->PutValue(tclID, _variant_t(section->strName));
		pRow->PutValue(tclParentID, _variant_t(strParentID));
		pRow->PutValue(tclName, _variant_t(section->strName));
		pRow->PutValue(tclDescription, _variant_t(section->strDescription));
		if (section->isRequired)
		{
			pRow->PutForeColor(RGB(255,0,0));
		}
		if (pParentRow) {
			//its a child row use a lighter color
			pRow->PutBackColor(CHILD_ROW_COLOR);
		}
		else {
			pRow->PutBackColor(PARENT_ROW_COLOR);
		}

		m_pTreeList->AddRowAtEnd(pRow, pParentRow);

		//now do the children
		if (section->innerSection.size() > 0) {
			LoadTree(pRow, section->innerSection);
		}

		
	}

}

MSXML2::IXMLDOMNodePtr CCCDAConfigDlg::FindNodeFromList(MSXML2::IXMLDOMNodeListPtr xmlList, CString strNameToFind)
{
	for (int i=0; i < xmlList->length; i++)
	{
		MSXML2::IXMLDOMNodePtr item = xmlList->Getitem(i);
		CString strName = GetTextFromXPath(item, "@name");
		if (strName.CompareNoCase(strNameToFind) == 0)
		{
			return item;
		}
	}

	return NULL;
}

void CCCDAConfigDlg::LoadXML()
{
	//Load our XML into our Structures
	MSXML2::IXMLDOMDocument2Ptr xmlDocumentConfig(__uuidof(MSXML2::DOMDocument60));	
	CString strFile = GetSharedPath() ^ "CCDAConfig.xml";

	if (VARIANT_TRUE != xmlDocumentConfig->load((LPCTSTR)strFile)) {
		CString strReason = "(unknown)";
		if(xmlDocumentConfig && xmlDocumentConfig->parseError) {
			strReason.Format("%li - %s %li, %li, %li", xmlDocumentConfig->parseError->errorCode, (LPCTSTR)xmlDocumentConfig->parseError->reason, xmlDocumentConfig->parseError->line, xmlDocumentConfig->parseError->linepos, xmlDocumentConfig->parseError->filepos);
		}
		ThrowNxException("Error In CCCDAConfig::LoadXML - The document '%s' could not be loaded; please verify this file exists and is a valid XML file.  Reason:  %s", strFile, strReason);
	}

	//we also may have an XML with our selected values
	MSXML2::IXMLDOMDocument2Ptr xmlDocumentSavedValues(__uuidof(MSXML2::DOMDocument60));
	CString strXMLSavedValues = GetRemotePropertyMemo("CCDAConfigurationXML", "", 0, "<None>");
	if (!strXMLSavedValues.IsEmpty()) {
		if (VARIANT_TRUE != xmlDocumentSavedValues->loadXML((LPCTSTR)strXMLSavedValues)) {
			CString strReason = "(unknown)";
			if(xmlDocumentSavedValues && xmlDocumentSavedValues->parseError) {
				strReason.Format("%li - %s %li, %li, %li", xmlDocumentSavedValues->parseError->errorCode, (LPCTSTR)xmlDocumentSavedValues->parseError->reason, xmlDocumentSavedValues->parseError->line, xmlDocumentSavedValues->parseError->linepos, xmlDocumentSavedValues->parseError->filepos);
			}
			ThrowNxException("Error In CCCDAConfig::LoadXML(2) - The saved values xml could not be loaded. Reason:  %s", strReason);
		}
	}

	//we now support multiple documents
	MSXML2::IXMLDOMNodeListPtr xmlDocsConfig = xmlDocumentConfig->selectNodes("//Documents/Document");	
	MSXML2::IXMLDOMNodeListPtr xmlDocsSaved;
	if (xmlDocumentSavedValues)
	{
		xmlDocsSaved= xmlDocumentSavedValues->selectNodes("//Documents/Document");
	}
	for (long t = 0; t < xmlDocsConfig->length; t++)
	{

		MSXML2::IXMLDOMNodePtr xmlDocConfig = xmlDocsConfig->item[t];
		MSXML2::IXMLDOMNodePtr xmlDocSaved;
		if (xmlDocsSaved)
		{
			xmlDocSaved = FindNodeFromList(xmlDocsSaved, GetTextFromXPath(xmlDocConfig, "@name"));
		}

		//make a map for our document
		//TES 2/5/2014 - PLID 60671 - Documents now have uid and ContentTimeStamp attributes along with their name
		CCDASectionMap mapSection;
		CString strDocName = GetTextFromXPath(xmlDocConfig, "@name");
		CString strDocUID = GetTextFromXPath(xmlDocConfig, "@uid");
		CString strTimestamp = GetTextFromXPath(xmlDocConfig, "@ContentTimeStamp");
		COleDateTime dtContentTimeStamp;
		dtContentTimeStamp.ParseDateTime(strTimestamp);

		//now do the major sections of this document
		MSXML2::IXMLDOMNodeListPtr xmlMajorSectionsConfig = xmlDocConfig->selectNodes("MajorSection");	
	
		MSXML2::IXMLDOMNodeListPtr xmlMajorSectionsSaved;
		if (xmlDocSaved) {
			xmlMajorSectionsSaved = xmlDocSaved->selectNodes("MajorSection");
		}

		for (long i = 0; i < xmlMajorSectionsConfig->length; i++) {
			MSXML2::IXMLDOMNodePtr xmlMajorSectionConfig = xmlMajorSectionsConfig->item[i];
			MSXML2::IXMLDOMNodePtr xmlMajorSectionSaved;
			if (xmlMajorSectionsSaved) {			
				xmlMajorSectionSaved = FindNodeFromList(xmlMajorSectionsSaved, GetTextFromXPath(xmlMajorSectionConfig, "@name"));
			}

			// (j.jones 2015-02-17 10:33) - PLID 64880 - send the document name
			CCDASectionPtr section = LoadSection(strDocName, xmlMajorSectionConfig, xmlMajorSectionSaved, true);
			
			//add our section to the array
			mapSection.insert(std::make_pair(section->strName, section));
			
		}
		//TES 2/5/2014 - PLID 60671 - Documents now have uid and ContentTimeStamp attributes along with their name
		CCDADocumentContents docContents;
		docContents.strUID = strDocUID;
		docContents.dtContentTimeStamp = dtContentTimeStamp;
		docContents.sectionMap = mapSection;
		m_mapDocuments.insert(std::make_pair(strDocName, docContents));
		
	}
}

BOOL CCCDAConfigDlg::ConfirmItemType(long nInfoMasterID, long nSavedType, CString &strItemName)
{
	ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT EMRInfoMasterT.ID AS MasterID, "			
			" EMRInfoT.Name, EMRInfoT.DataType "
			" FROM  "
			" EMRInfoMasterT INNER JOIN EMRInfoT ON EMRInfoMasterT.ActiveEMRInfoID = EMRInfoT.ID "
			" WHERE EMRInfoMasterT.ID = {INT} "
			, nInfoMasterID);
	//there is only one
	if (!rs->eof)
	{
		long nCurrentDataType = (long)AdoFldByte(rs->Fields, "DataType");
		strItemName = AdoFldString(rs->Fields, "Name");

		if (nCurrentDataType != nSavedType)
		{
			return FALSE;
		}
		else {
			return TRUE;
		}

	}
	return FALSE;			
}

// (j.jones 2015-02-17 10:27) - PLID 64880 - added section name
void CCCDAConfigDlg::LoadValues(CString strSectionName, CCDAFieldPtr pField, MSXML2::IXMLDOMNodePtr xmlFieldSaved)
{
	CString str = (LPCTSTR)xmlFieldSaved->Getxml();
	TRACE(str + "\r\n");
	MSXML2::IXMLDOMNodeListPtr xmlValuesSaved = xmlFieldSaved->selectNodes("Value");		

	for (long i=0; i < xmlValuesSaved->length; i++) {
		MSXML2::IXMLDOMNodePtr xmlValueSaved = xmlValuesSaved->item[i];

		CCDAValuePtr pValue(new CCDAValue);
		pValue->type = (CCDAValueType)atoi(GetTextFromXPath(xmlValueSaved, "@type"));
		CString strID = GetTextFromXPath(xmlValueSaved, "@ID");
		CString strXID = GetTextFromXPath(xmlValueSaved, "@X_ID");
		CString strYID = GetTextFromXPath(xmlValueSaved, "@Y_ID");
		long nSavedType = atoi(GetTextFromXPath(xmlValueSaved, "@ItemType"));
		long nInfoMasterID = atoi(GetTextFromXPath(xmlValueSaved, "@InfoMasterID"));
		pValue->bIsFlipped = !!atoi(GetTextFromXPath(xmlValueSaved, "@IsFlipped"));
		CCDAGenerationType genType = (CCDAGenerationType)atoi(GetTextFromXPath(xmlValueSaved, "@GenerationType"));		
		if (pValue->bIsFlipped) {
			if (genType == cgtColumn){
				genType = cgtRow;
			}
			else if (genType == cgtRow)
			{
				genType = cgtColumn;
			}

			//flip the types too
			if (pValue->type == cvtEMRDataCode_X)
			{
				pValue->type = cvtEMRDataCode_Y;
			}
			else if (pValue->type == cvtEMRDataCode_Y)
			{
				pValue->type = cvtEMRDataCode_X;
			}
		}
		pValue->nGenTypeID = genType;
		if (ConfirmItemType(nInfoMasterID, nSavedType, pValue->strInfoName))
		{
			pValue->nDataType = nSavedType;
		}
		else {
			// (j.jones 2015-02-17 09:27) - PLID 64880 - improved this message to tell us exactly what changed
			CString strWarn;
			_RecordsetPtr rs = CreateParamRecordset("SELECT Name, DataType FROM EMRInfoT "
				"INNER JOIN EMRInfoMasterT ON EMRInfoT.ID = EMRInfoMasterT.ActiveEMRInfoID "
				"WHERE EMRInfoMasterT.ID = {INT}", nInfoMasterID);
			if (!rs->eof) {
				//report the current item's name and data type
				CString strName = AdoFldString(rs->Fields, "Name");
				
				EmrInfoType eNewDataType = (EmrInfoType)AdoFldByte(rs->Fields, "DataType");
				CString strNewDataType = GetDataTypeName(eNewDataType);

				EmrInfoType eOldDataType = (EmrInfoType)nSavedType;
				CString strOldDataType = GetDataTypeName(eOldDataType);

				strWarn.Format("The previously configured item '%s' has changed types from %s to %s.\n\nThis item was used in the %s section, and will be taken out of the configuration.", strName, strOldDataType, strNewDataType, strSectionName);
			}
			else {
				//the master ID no longer exists? how is that possible?
				ASSERT(FALSE);
				//report the cached item name
				strWarn.Format("The previously configured item '%s' has changed types (ie: table to multi-select list).\n\nThis item was used in the %s section, and will be taken out of the configuration.", pValue->strInfoName, strSectionName);
			}

			// (j.jones 2015-02-17 10:44) - PLID 64880 - Compare against the last item warning.
			// The way the xml is loaded, the item is tracked multiple times per section, so
			// you would get the same identical warning twice in a row. Stop it.
			if (m_strLastRemovedItemWarning != strWarn) {
				MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION | MB_OK);
				m_strLastRemovedItemWarning = strWarn;
			}

			//ensure we do not leak memory
			pValue.reset();

			// (j.jones 2015-02-17 09:27) - PLID 64880 - continue looping, do not exit this function
			continue;
		}

		switch (pValue->type)
		{		
			case cvtNone:
				ASSERT(FALSE);
				pValue->nID = -999;
			break;

			case cvtEMRInfo:
			case cvtEMRData:
			case cvtCode:
			case cvtEMRDataCode:
			case cvtEMRDataDropDownCode:
			case cvtEMRDataCode_X:
			case cvtEMRDataCode_Y:
				pValue->nID = atoi(strID);
			break;

			case cvtEMNDate:			
				pValue->nID = -1;			
			break;
				
			case cvtCell:
			case cvtEMRCellCode:
				pValue->X_ID = atoi(strXID);
				pValue->Y_ID = atoi(strYID);				
			break;

			default:
				ThrowNxException("Error in CCDAConfigDlg::LoadValue - Unknown value type");				
			break;
		}

		//we have to get additional information about our item

		pField->mapSelections.insert(std::make_pair(nInfoMasterID, pValue));
	}
}

// (j.jones 2015-02-17 10:27) - PLID 64880 - added section name
CCDASectionPtr CCCDAConfigDlg::LoadSection(CString strSectionName, MSXML2::IXMLDOMNodePtr xmlSectionConfig, MSXML2::IXMLDOMNodePtr xmlSectionSaved, bool bisMajor)
{
	//Declare our section
	CCDASectionPtr pSection(new CCDASection);

	//first load this sections attributes
	CString strIsReq = GetTextFromXPath(xmlSectionConfig, "@isRequired");
	if (strIsReq.CompareNoCase("true") == 0) {
		pSection->isRequired = true;
	}
	else {
		pSection->isRequired = false;
	}

	pSection->isMajorSection = bisMajor;
	pSection->strDescription = GetTextFromXPath(xmlSectionConfig, "@Description");
	pSection->strName = GetTextFromXPath(xmlSectionConfig, "@name");

	//now setup the fields that are in the section
	MSXML2::IXMLDOMNodeListPtr xmlFieldsConfig = xmlSectionConfig->selectNodes("Field");
	MSXML2::IXMLDOMNodeListPtr xmlFieldsSaved;
	if (xmlSectionSaved) {
		xmlFieldsSaved = xmlSectionSaved->selectNodes("Field");
	}

	for (long i=0; i < xmlFieldsConfig->length; i++) {
		MSXML2::IXMLDOMNodePtr xmlFieldConfig = xmlFieldsConfig->item[i];
		
		MSXML2::IXMLDOMNodePtr xmlFieldSaved;
		if (xmlFieldsSaved) {
			xmlFieldSaved = FindNodeFromList(xmlFieldsSaved, GetTextFromXPath(xmlFieldConfig, "@name"));			
		}

		CCDAFieldPtr pField(new CCDAField);
		pField->strName = GetTextFromXPath(xmlFieldConfig, "@name");
		pField->strDescription = GetTextFromXPath(xmlFieldConfig, "@Description");
		CString strIsRequired = GetTextFromXPath(xmlFieldConfig, "@isRequired");
		if (strIsRequired.CompareNoCase("true") == 0) { 
			pField->isRequired = true;
		}
		else {
			pField->isRequired = false;
		}
		pField->strLimit = GetTextFromXPath(xmlFieldConfig, "@limit");
		pField->strType = GetTextFromXPath(xmlFieldConfig, "@type");
		pField->strCodeSystem = GetTextFromXPath(xmlFieldConfig, "@CodeSystem");	

		//now load the values
		
		if (xmlFieldSaved) {
			// (j.jones 2015-02-17 10:33) - PLID 64880 - concatenate the section names
			LoadValues(FormatString("%s -> %s", strSectionName, pSection->strName), pField, xmlFieldSaved);
		}	

		pSection->fields.insert(std::make_pair(pField->strName, pField));
	}

	//now look for sections
	MSXML2::IXMLDOMNodeListPtr xmlInnerSectionsConfig = xmlSectionConfig->selectNodes("Section");
	MSXML2::IXMLDOMNodeListPtr xmlInnerSectionsSaved;
	if (xmlSectionSaved)
	{
		xmlInnerSectionsSaved = xmlSectionSaved->selectNodes("Section");				
	}

	for (long i=0; i < xmlInnerSectionsConfig->length; i++) {
		MSXML2::IXMLDOMNodePtr xmlInnerSectionConfig = xmlInnerSectionsConfig->item[i];
		MSXML2::IXMLDOMNodePtr xmlInnerSectionSaved;
		if (xmlInnerSectionsSaved)
		{
			xmlInnerSectionSaved = FindNodeFromList(xmlInnerSectionsSaved, GetTextFromXPath(xmlInnerSectionConfig, "@name"));
		}

		//load the section
		// (j.jones 2015-02-17 10:33) - PLID 64880 - concatenate the section names
		CCDASectionPtr pInnerSection = LoadSection(FormatString("%s -> %s", strSectionName, pSection->strName), xmlInnerSectionConfig, xmlInnerSectionSaved, false);
		
		pSection->innerSection.insert(std::make_pair(pInnerSection->strName, pInnerSection));
	}

	return pSection;
}

BEGIN_EVENTSINK_MAP(CCCDAConfigDlg, CNxDialog)
ON_EVENT(CCCDAConfigDlg, IDC_CCDA_TREE_LIST, 2, CCCDAConfigDlg::SelChangedTreeList, VTS_DISPATCH VTS_DISPATCH)
ON_EVENT(CCCDAConfigDlg, IDC_EMR_INFO_LIST, 16, CCCDAConfigDlg::SelChosenEmrInfoList, VTS_DISPATCH)
ON_EVENT(CCCDAConfigDlg, IDC_OTHER_LIST, 10, CCCDAConfigDlg::EditingFinishedOtherList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
ON_EVENT(CCCDAConfigDlg, IDC_OTHER_LIST, 7, CCCDAConfigDlg::RButtonUpOtherList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
ON_EVENT(CCCDAConfigDlg, IDC_CCDA_DOCUMENT_LIST, 16, CCCDAConfigDlg::SelChosenCcdaDocumentList, VTS_DISPATCH)
ON_EVENT(CCCDAConfigDlg, IDC_CCDA_DOCUMENT_LIST, 1, CCCDAConfigDlg::SelChangingCcdaDocumentList, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CCCDAConfigDlg, IDC_CCDA_TREE_LIST, 1, CCCDAConfigDlg::SelChangingCcdaTreeList, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CCCDAConfigDlg, IDC_OTHER_LIST, 9, CCCDAConfigDlg::EditingFinishingOtherList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

void CCCDAConfigDlg::GetTreePath(NXDATALIST2Lib::IRowSettingsPtr pRow, CStringArray &aryReturn)
{
	//we need to keep track of all the parents
	CStringArray aryPath;
	
	//first add the current row's ID
	aryPath.Add(VarString(pRow->GetValue(tclID)));
	

	NXDATALIST2Lib::IRowSettingsPtr pParentRow = pRow->GetParentRow();
	while (pParentRow)
	{
		aryPath.Add(VarString(pParentRow->GetValue(tclID)));
		pParentRow = pParentRow->GetParentRow();
	}
	
	//now reverse it so its the correct way
	for(int i= aryPath.GetSize() - 1; i >= 0; i--)
	{
		aryReturn.Add(aryPath.GetAt(i));
	}
	
}

CCDASectionPtr CCCDAConfigDlg::FindTreeSection(NXDATALIST2Lib::IRowSettingsPtr pRow)
{

	//what document are we on?
	NXDATALIST2Lib::IRowSettingsPtr pDocRow = m_pDocumentList->CurSel;
	if (pDocRow)
	{
		CString strDocName = VarString(pDocRow->GetValue(dlcID));
		CCDADocumentMap::iterator itDoc;
		itDoc = m_mapDocuments.find(strDocName);

		if (itDoc != m_mapDocuments.end())
		{
			//TES 2/5/2014 - PLID 60671 - mapSection is now one component of docContents
			CCDADocumentContents docContents = itDoc->second;
			CCDASectionMap mapSection = docContents.sectionMap;
	
			CStringArray aryPath;
			GetTreePath(pRow, aryPath);

			CCDASectionMap mapCurrent = mapSection;	
			CCDASectionPtr curSection(new CCDASection);
			for(int i=0; i < aryPath.GetSize(); i++)
			{
				CString strToFind = aryPath.GetAt(i);

				CCDASectionIterator it;
				it = mapCurrent.find(strToFind);

				if (it != mapCurrent.end()) 
				{
					curSection = it->second;
					mapCurrent = curSection->innerSection;
				}
			}

			return curSection;
		}
	}

	if (pRow)
	{
		CString rowName = VarString(pRow->GetValue(tclName));
		ThrowNxException("Error in CCCDAConfigDlg::FindTreeSection - Could not find section: " + rowName); 
	}
	else {
			ThrowNxException("Error in CCCDAConfigDlg::FindTreeSection - Invalid Row Sent");
	}
}

CCDAValueType CCCDAConfigDlg::GetValueTypeFromValue(BOOL bIsCodeField, BOOL bIsDateField, _variant_t varValue)
{
	CString strValue = AsString(varValue);

	long nResult = strValue.Find("C");
	if (bIsCodeField && nResult != -1)
	{
		//we need to know if its a selected code, or an item code (if its a multi or single select list, the placement of the C tells us)
		if (nResult == 0) {
			//this is a code field and we chose a code
			return cvtCode;
		}
		else {
			return cvtEMRDataCode;
		}
	}
	else if (bIsDateField) {
		ASSERT(varValue.vt == VT_I4 || varValue.vt == VT_BSTR);
		if (
			((varValue.vt == VT_I4) && varValue.lVal == -1)
		 || ((varValue.vt == VT_BSTR) && VarString(varValue) == "-1")
		){			
			//the chose a date
			return cvtEMNDate;
		}
		else {
			return cvtEMRInfo;
		}

	}
	else {
		//this is our item value
		return cvtEMRInfo;
	}

}

//this is only called when we'd be returning the EMRData options
CCDAValueType CCCDAConfigDlg::GetValueTypeFromSelection(BOOL bIsCodeField, BOOL bIsDateField, _variant_t varValue, CCDAGenerationType genType)
{
	CString strValue = AsString(varValue);
	if (bIsCodeField && strValue.Find("DC") != -1)
	{
		//this is a code field and we chose a drop down code
		return cvtEMRDataDropDownCode;
	}

	// (j.gruber 2014-01-27 14:28) - PLID 60199 - support explicit codes for tables
	long nCIndex = strValue.Find("C");

	if (bIsCodeField && nCIndex != -1)
	{
		//is it =1C because if so, we are going based on generation type
		if (strValue == "-1C")
		{
			if (genType == cgtRow)
			{
				//we want the row Code
				return cvtEMRDataCode_X;
			}
			else if (genType == cgtColumn)
			{
				//we want the column code
				return cvtEMRDataCode_Y;
			}
			else {
				//this shouldn't be possible
				ThrowNxException("Error in GetValueTypeFromSelection - Unknown generation type selection");
			}
		}
		else {
			//reverse the above
			// (j.gruber 2014-01-27 14:30) - PLID 60199 - check to see where the C is
			if (nCIndex == 0) {
				//its a code
				return cvtCode;
			}
			else {
				if (genType == cgtRow)
				{
					//we want the col Code
					return cvtEMRDataCode_Y;
				}
				else if (genType == cgtColumn)
				{
					//we want the row code
					return cvtEMRDataCode_X;
				}
				else {
					//this is a code field and we chose a code
					return cvtEMRDataCode;
				}
			}
		}		
			
	}
	else if (bIsDateField) {
		ASSERT(varValue.vt == VT_I4 || varValue.vt == VT_BSTR);
		if (
			((varValue.vt == VT_I4) && varValue.lVal == -1)
		 || ((varValue.vt == VT_BSTR) && VarString(varValue) == "-1")
		){			
			//the chose a date
			return cvtEMNDate;
		}
		else {
			return cvtEMRData;
		}
	}
	else {
		//this is our item value
		return cvtEMRData;
	}

}


CCDAValueType CCCDAConfigDlg::GetValueTypeFromCell(BOOL bIsCodeField, BOOL bIsDateField, _variant_t varValue)
{	
	if (bIsDateField) {
		ASSERT(varValue.vt == VT_I4 || varValue.vt == VT_BSTR);
		if (
			((varValue.vt == VT_I4) && varValue.lVal == -1)
		 || ((varValue.vt == VT_BSTR) && VarString(varValue) == "-1")
		){			
			//the chose a date
			return cvtEMNDate;
		}
		else {
			return cvtCell;
		}
	}
	else {
		ASSERT(varValue.vt == VT_BSTR);
		//we know we have a cell value here, so parse it out into the row and column
		CString strValue = VarString(varValue);
		long nResult = strValue.Find("_");
		if (nResult == -1) {
			//they may have picked a code
			if (bIsCodeField) {
				// (j.gruber 2014-01-29 16:02) - PLID 60199 - this would return false exactly when it shouldn't be
				if (strValue.Find("C") != -1) {
					//yep!
					return cvtCode;
				}
			}
		}
		CString strRow = strValue.Left(nResult);
		CString strCol = strValue.Right(strValue.GetLength() - (nResult + 1));
		long nHasCode = strRow.Find("C");

		if (bIsCodeField && nHasCode != -1)
		{
			//this is a code field and we chose a code
			return cvtEMRCellCode;
		}
		else {
			//this is our item value
			return cvtCell;
		}
	}

}

CCDAValueType CCCDAConfigDlg::GetValueType(CCDAFieldPtr pField, NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::IColumnSettingsPtr pCol)
{

	//check the datatype
	long nDataType = VarLong(pRow->GetValue(clcItemDataType));
	//get the generation type
	CCDAGenerationType type = (CCDAGenerationType) VarLong(pRow->GetValue(clcGenerationType));
	_variant_t varValue = pRow->GetValue(GetColumnFromField(pField));

	BOOL bIsCodeField = IsCodeField(pField);
	BOOL bIsDateField = IsDateField(pField);
				
	switch (nDataType)
	{
		case eitText:
		case eitSingleList:			
		case eitSlider:
			
			//our options here are they if its a code field, they could choose either a code, or the item value,
			//if its a date field, they could choose EMN Date or the item value
			return GetValueTypeFromValue(bIsCodeField, bIsDateField, varValue);
			
			
		break;

		case eitMultiList:

			//our options here, are if they chose the item, we treat it like a single select,
			//if they choose selection, they could choose selection code if it is a code field

			if (type == cgtItem)
			{
				return GetValueTypeFromValue(bIsCodeField, bIsDateField, varValue);
			}
			else {
				return GetValueTypeFromSelection(bIsCodeField, bIsDateField, varValue, type);
			}
		break;
		case eitTable:
			if (type == cgtItem)
			{
				//we have to figure out their selection
				return GetValueTypeFromCell(bIsCodeField, bIsDateField, varValue);
					
			}
			else {
				//its an Data
				return GetValueTypeFromSelection(bIsCodeField, bIsDateField, varValue, type);
			}
		break;	
	}
	

	return cvtEMRInfo;
		
}

void CCCDAConfigDlg::ClearSelections(CCDASectionPtr oldSection)
{
	//loop through our fields
	CCDAFieldIterator itField;
	for(itField = oldSection->fields.begin(); itField != oldSection->fields.end(); itField++)
	{
		CCDAFieldPtr field = itField->second;
		field->mapSelections.clear();
	}
}

short CCCDAConfigDlg::GetColumnFromField(CCDAFieldPtr pField)
{
	//loop through the columns and pull out the column index that goes with our field
	for(int i=MAX_CONFIG_DEF_COLS+1; i < m_pConfigList->GetColumnCount(); i++) {
		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pConfigList->GetColumn(i);
		CString strTitle = (LPCTSTR)pCol->GetColumnTitle();

		if (strTitle == pField->strName)
		{
			return i;
		}
	}
	return -1;	
}

_variant_t CCCDAConfigDlg::GetValueFromType(CCDAValuePtr pValue)
{

	switch (pValue->type)
	{		
		case cvtNone:
			return -999;
		break;

		case cvtEMRInfo:
		case cvtEMRData:
			return pValue->nID;
		case cvtEMNDate:
			return -1;
		break;
			
		case cvtCell:
			return _variant_t(AsString(pValue->X_ID) + "_" + AsString(pValue->Y_ID));
		break;

		case cvtCode:
			return _variant_t("C" + AsString(pValue->nID));
		break;
		case cvtEMRDataCode:			
		case cvtEMRDataCode_X:
		case cvtEMRDataCode_Y:
			return _variant_t(AsString(pValue->nID) + "C");
		break;
		case cvtEMRDataDropDownCode:
			return _variant_t(AsString(pValue->nID) + "DC");
		break;
		case cvtEMRCellCode:
			return _variant_t(AsString(pValue->X_ID) + "C_" + AsString(pValue->Y_ID) + "C");
		break;

		default:
			return pValue->nID;
		break;
	}

	
	
}

/*Given a section of the map, load the fields for the section*/
void CCCDAConfigDlg::LoadFields(CCDASectionPtr pSection)
{

	std::map<long, NXDATALIST2Lib::IRowSettingsPtr> mapRows;

	//loop through our selections and add each row, per item we have
	CCDAFieldIterator itField;
	for (itField = pSection->fields.begin(); itField != pSection->fields.end(); itField++)
	{
		CCDAFieldPtr pField = itField->second;

		//now the selections
		CCDAValueMap::iterator itValue;
		for (itValue = pField->mapSelections.begin(); itValue != pField->mapSelections.end(); itValue++)
		{
			long nInfoID = itValue->first;
			CCDAValuePtr pValue = itValue->second;

			//see if we have a row for this value yet
			std::map<long, NXDATALIST2Lib::IRowSettingsPtr>::iterator itRows;
			itRows = mapRows.find(nInfoID);
			if (itRows != mapRows.end())
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow = itRows->second;

				//we already have a row, find our column and fill it
				pRow->PutRefCellFormatOverride(GetColumnFromField(pField), GetFieldFormatOverride(pField, pValue->nDataType, nInfoID, pValue->nGenTypeID));
				pRow->PutValue(GetColumnFromField(pField), GetValueFromType(pValue));
			}
			else {
				//we don't have a row yet, we need to make one
				NXDATALIST2Lib::IRowSettingsPtr pRow = GetSectionRow(pSection, (mapRows.size() == 0));
				//fill the values we know
				pRow->PutValue(clcItemID, nInfoID);
				pRow->PutValue(clcIsFlipped, pValue->bIsFlipped);
				pRow->PutValue(clcItemDataType, pValue->nDataType);
				pRow->PutValue(clcItemName, _variant_t(pValue->strInfoName));
				pRow->PutRefCellFormatOverride(clcGenerationType, GetGenerationTypeFormatOverride(pValue->nDataType));
				pRow->PutValue(clcGenerationType, pValue->nGenTypeID);
				pRow->PutRefCellFormatOverride(GetColumnFromField(pField), GetFieldFormatOverride(pField, pValue->nDataType, nInfoID, pValue->nGenTypeID));						

				//now fill our cell that we care about
				pRow->PutValue(GetColumnFromField(pField), GetValueFromType(pValue));

				//insert our row
				mapRows.insert(std::make_pair(nInfoID, pRow));
			}
			
		}
	}

	//now loop through our map and add the rows to the datalist
	std::map<long, NXDATALIST2Lib::IRowSettingsPtr>::iterator itRows;
	for (itRows = mapRows.begin(); itRows != mapRows.end(); itRows++)
	{
		m_pConfigList->AddRowAtEnd(itRows->second, NULL);	
	}

}

/*long CCCDAConfigDlg::GetItemValue(CCDAValueMap mapSelections, long nItemID)
{
	CCDAValueMap::iterator itVal;
	itVal = mapSelections.find(nItemID);

	if (itVal != mapSelections.end())
	{
		//we found one, now get the type
		CCDAValuePtr pValue = itVal->second;
		switch (pValue->type)
		{
			case cvtEMRInfo:						
			case cvtEMRData:
			case cvtCode:
				return pValue->nID;
			case cvtEMNDate:
				return -1;
			case cvtCell:
				break;
		}
		return -999;
	}
}*/

NXDATALIST2Lib::IRowSettingsPtr CCCDAConfigDlg::GetSectionRow(CCDASectionPtr pSection, BOOL bAddColumns)
{
	//add our columns if there aren't any for this section yet
	if (bAddColumns) 
	{

		//first, add columns for the id, datatype, and name
		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pConfigList->GetColumn(m_pConfigList->InsertColumn(clcItemID, _T(""), _bstr_t("ItemID"), 0, NXDATALIST2Lib::csFixedWidth));
		pCol->PutFieldType(NXDATALIST2Lib::cftTextSingleLine);

		pCol = m_pConfigList->GetColumn(m_pConfigList->InsertColumn(clcIsFlipped, _T(""), _bstr_t("Is Flipped"), 0, NXDATALIST2Lib::csFixedWidth));
		pCol->PutFieldType(NXDATALIST2Lib::cftTextSingleLine);
		
		pCol = m_pConfigList->GetColumn(m_pConfigList->InsertColumn(clcItemDataType, _T(""), _bstr_t("Data Type"), 0, NXDATALIST2Lib::csFixedWidth));
		pCol->PutFieldType(NXDATALIST2Lib::cftTextSingleLine);
		
		pCol = m_pConfigList->GetColumn(m_pConfigList->InsertColumn(clcItemName, _T(""), _bstr_t("Item Name"), 150, NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csWidthData));
		pCol->PutFieldType(NXDATALIST2Lib::cftTextSingleLine);

		pCol = m_pConfigList->GetColumn(m_pConfigList->InsertColumn(clcGenerationType, _T(""), _bstr_t("Generate 1 Section Per"), 150, NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csWidthData));		

		int i = MAX_CONFIG_DEF_COLS + 1;	
		long nValueCol = -1;
		CCDAFieldIterator itField;
		for (itField = pSection->fields.begin(); itField != pSection->fields.end(); itField++)
		{
			CCDAFieldPtr pField = itField->second;
								
			pCol = m_pConfigList->GetColumn(m_pConfigList->InsertColumn(i, _T(""), _bstr_t(pField->strName), 150, NXDATALIST2Lib::csVisible));					
			NXDATALIST2Lib::EColumnFieldType colType = GetTypeFromField(pField);
			pCol->PutFieldType(colType);
			if (colType == NXDATALIST2Lib::cftComboSimple) {
				pCol->PutColumnStyle(NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csWidthData|NXDATALIST2Lib::csEditable);				
				//pCol->PutComboSource(_bstr_t(GetSourceFromField(pField, nDataType)));
			}	
			else {
				//since we already know that this section can only have one value to fill in, this must be the value, since its not a code or date
				pCol->PutColumnStyle(NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csWidthData);				
				//ASSERT(nValueCol == -1);
				nValueCol = i;
			}	
		}
	}
	//now make a new row
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConfigList->GetNewRow();
	if (pRow)
	{	
		/*if (nValueCol != -1) {
			pRow->PutValue(nValueCol, _variant_t("<Value of Item>"));
		}*/		
		return pRow;
	}
	
	//if we get here, throw an error
	ThrowNxException("Error in GetSectionRow - Could not get valid row");

}

BOOL CCCDAConfigDlg::ValidateFields(CCDASectionPtr pOldSection)
{
	// we are just going to check that they didn't pick the same field for 2 different values and that they filled everything in
	//loop through the config rows
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConfigList->GetFirstRow();

	while (pRow) {
		long nItemID = VarLong(pRow->GetValue(clcItemID));		

		//first make sure nothing is empty
		//start at column 2 beause we know the first column is the ItemID, second is the name, third is datatype		
		for (int i = 3; i < m_pConfigList->GetColumnCount(); i++) {
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pConfigList->GetColumn(i);

			_variant_t varValue = pRow->GetValue(pCol->GetIndex());

			if (varValue.vt == VT_EMPTY)
			{
				MsgBox("Please fill in values for each field");
				return FALSE;
			}
		}

		//now make sure we didn't reuse a column
		std::map<CString, CString> mapValues;
		for (int i = 3; i < m_pConfigList->GetColumnCount(); i++) {

			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pConfigList->GetColumn(i);

			//find the field for this column
			CString strTitle = (LPCSTR)pCol->GetColumnTitle();
			CCDAFieldIterator itField = pOldSection->fields.find(strTitle);
			if (itField != pOldSection->fields.end()) {
				
				CCDAFieldPtr pField = itField->second;				

				CString strValCombo = "";
				CCDAValueType type = GetValueType(pField, pRow, pCol);
				strValCombo += AsString((long)type) + "_";
				strValCombo += AsString(VarLong(pRow->GetValue(clcGenerationType))) + "_";
						
				_variant_t varValue = pRow->GetValue(pCol->GetIndex());
				switch (type)
				{
					case cvtNone:
					break;
					case cvtEMRInfo:					
						strValCombo += AsString(nItemID) + "_";
					break;
					case cvtEMNDate:					
						strValCombo += "-1_";
					break;
					case cvtCell:
					case cvtEMRCellCode:
						{
							CString strCellID = VarString(varValue);
							strValCombo += strCellID + "_";							
						}
					break;
					case cvtCode:
						{
						//strip out the "C"
						CString strCode = AsString(varValue);						
						strValCombo += strCode + "_";						
						}
					break;
					case cvtEMRData:
					case cvtEMRDataDropDownCode:
					case cvtEMRDataCode:
					case cvtEMRDataCode_X:
					case cvtEMRDataCode_Y:
						strValCombo += AsString(varValue) + "_";						
					break;
				}
				
				std::map<CString, CString>::iterator it = mapValues.find(strValCombo);
				if (it != mapValues.end())
				{
					//its already been used
					MsgBox("Fields in the same section must be unique.");
					return FALSE;
				}
				mapValues.insert(std::make_pair(strValCombo, strValCombo));
				//pOldSection->fields[pField->strName] = pField;
				//oldSection.fields.insert(std::make_pair(field.strName, field));
			}			
		}
		pRow = pRow->GetNextRow();
	}	

	//if we got here, we are good
	return TRUE;
}

BOOL CCCDAConfigDlg::SaveFields(CCDASectionPtr pOldSection)
{
	if (!ValidateFields(pOldSection))
	{
		return FALSE;
	}

	//first, clear the section selections since we are going to over-write them
	ClearSelections(pOldSection);

	//loop through the config rows
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConfigList->GetFirstRow();

	while (pRow) {
		long nItemID = VarLong(pRow->GetValue(clcItemID));		

		//loop through the columns
		//start at column 2 beause we know the first column is the ItemID, second is the name, third is datatype		
		for (int i = 3; i < m_pConfigList->GetColumnCount(); i++) {
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pConfigList->GetColumn(i);
			//find the field for this column
			CString strTitle = (LPCSTR)pCol->GetColumnTitle();
			CCDAFieldIterator itField = pOldSection->fields.find(strTitle);
			if (itField != pOldSection->fields.end()) {
				
				CCDAFieldPtr pField = itField->second;				

				CCDAValuePtr pValue(new CCDAValue);
				//found the field, now save the new values
				pValue->strInfoName = VarString(pRow->GetValue(clcItemName));
				pValue->nDataType = VarLong(pRow->GetValue(clcItemDataType));				
				pValue->nGenTypeID = (CCDAGenerationType) VarLong(pRow->GetValue(clcGenerationType));
				pValue->type = GetValueType(pField, pRow, pCol);
				pValue->bIsFlipped = !!VarBool(pRow->GetValue(clcIsFlipped));
				//if its a code, then get the ID From the field, otherwise either the type of the infoID is all we need
				_variant_t varValue = pRow->GetValue(pCol->GetIndex());
				switch (pValue->type)
				{
					case cvtNone:
					break;
					case cvtEMRInfo:					
						pValue->nID = nItemID;
					break;
					case cvtEMNDate:					
						pValue->nID = -1;
					break;
					case cvtCell:
					case cvtEMRCellCode:
						{
							CString strCellID = VarString(varValue);
							long nResult = strCellID.Find("_");
							pValue->X_ID = atoi(strCellID.Left(nResult));
							pValue->Y_ID = atoi(strCellID.Right(strCellID.GetLength() - (nResult + 1)));
						}
					break;
					case cvtCode:
						{
						//strip out the "C"
						CString strCode = AsString(varValue);						
						strCode.Replace("C", "");
						pValue->nID = atoi(strCode);
						}
					break;
					case cvtEMRData:
					case cvtEMRDataDropDownCode:
					case cvtEMRDataCode:
					case cvtEMRDataCode_X:
					case cvtEMRDataCode_Y:
						pValue->nID = atoi(AsString(varValue));			
					break;
				}
				
				pField->mapSelections.insert(std::make_pair(nItemID, pValue));
				//pOldSection->fields[pField->strName] = pField;
				//oldSection.fields.insert(std::make_pair(field.strName, field));
			}			
		}
		pRow = pRow->GetNextRow();
	}	
	return TRUE;
}

void CCCDAConfigDlg::SelChangedTreeList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {

		//save our previous selections
		NXDATALIST2Lib::IRowSettingsPtr pOldRow(lpOldSel);
		if (pOldRow) {				
			CCDASectionPtr pOldSection = FindTreeSection(pOldRow);
			if (!SaveFields(pOldSection))
			{
				m_pTreeList->CurSel = pOldRow;
				return;
			}

			//clear the list
			m_pConfigList->Clear();
			//remove the columns also
			long nCount = m_pConfigList->GetColumnCount();
			for (short nCol = m_pConfigList->GetColumnCount() - 1; nCol >= 0; nCol--) {
				m_pConfigList->RemoveColumn(nCol);
			}

			ASSERT(m_pConfigList->GetColumnCount() == 0);
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);

		if (pRow) 
		{				
			CCDASectionPtr pSection = FindTreeSection(pRow);	
			//grey out based on if we have fields, not major section, some major sections have fields and no subsections
			if (pSection->fields.size() == 0)
			{
				GetDlgItem(IDC_EMR_INFO_LIST)->EnableWindow(FALSE);
				GetDlgItem(IDC_OTHER_LIST)->EnableWindow(FALSE);
			}
			else {
				GetDlgItem(IDC_EMR_INFO_LIST)->EnableWindow(TRUE);
				GetDlgItem(IDC_OTHER_LIST)->EnableWindow(TRUE);
			}

			if (pSection->fields.size() > 0) {

				m_pFieldDataList->Clear();
				//fill the fields list
				CCDAFieldIterator itField;
				for(itField = pSection->fields.begin(); itField != pSection->fields.end(); itField++)
				{
					CCDAFieldPtr pField = itField->second;
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldDataList->GetNewRow();
					if (pRow)
					{
						pRow->PutValue(flcID, _variant_t(pField->strName));
						pRow->PutValue(flcName, _variant_t(pField->strName));
						pRow->PutValue(flcDescription, _variant_t(pField->strDescription));
						m_pFieldDataList->AddRowAtEnd(pRow, NULL);
					}
				}
				
				//load the section
				LoadFields(pSection);
			}
			else {
				//grey out the fields lists				
				m_pEMRInfoList->Enabled = false;
				m_pConfigList->Enabled = false;
			}
		}
		else {
			//grey out the fields lists				
			m_pEMRInfoList->Enabled = false;
			m_pConfigList->Enabled = false;
		}
		
	}NxCatchAll(__FUNCTION__);
	
}


CString CCCDAConfigDlg::GetCodeListFromSystem(CString strCodeSystem)
{
	//first check our map to see if we have already gotten this system
	std::map<CString, CString>::iterator it;
	it = m_mapCodes.find(strCodeSystem);

	if (it != m_mapCodes.end())
	{
		CString strCodes = it->second;		
		return strCodes;
	}

	//if we got here, that means our list wasn't in the map, so lets get it from the data
	ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT ID, Code, Vocab, Name FROM CodesT WHERE vocab = {STRING}", strCodeSystem);
	CString strCodes;
	CStringArray *paryCodes = new CStringArray();
	while (!rs->eof)
	{	
		CString strCodeID = AsString(AdoFldLong(rs->Fields, "ID"));
		CString strCode = AdoFldString(rs->Fields, "Code");
		CString strName = AdoFldString(rs->Fields, "Name");
		CString strIndivCode = "C" + strCodeID + ";" + strCode + "-" + strName + ";";
		strCodes += strIndivCode;
		paryCodes->Add(strIndivCode);
		rs->MoveNext();
	}

	//add our array to the map
	m_mapCodes.insert(std::make_pair(strCodeSystem, strCodes));

	return strCodes;
}

NXDATALIST2Lib::EColumnFieldType CCCDAConfigDlg::GetTypeFromField(CCDAFieldPtr pField)
{
	
	if (IsDateField(pField))
	{		
		return NXDATALIST2Lib::cftComboSimple;
	}
	if (IsCodeField(pField)) {
		return NXDATALIST2Lib::cftComboSimple;
	}
	else {
		return NXDATALIST2Lib::cftTextSingleLine;
	}
}

BOOL CCCDAConfigDlg::IsDateField(CCDAFieldPtr pField)
{
	if (pField->strName.CompareNoCase("effectiveTime") == 0)
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CCCDAConfigDlg::IsCodeField(CCDAFieldPtr pField)
{
	CString strType = pField->strType;

	strType.MakeLower();

	if (strType == "code" 
		|| strType.Find("code") != -1		
		)
	{
		return TRUE;
	}

	return FALSE;

}

CString CCCDAConfigDlg::GetTableList(CCDAFieldPtr pField, long nItemID, CCDAGenerationType nGenType)
{
	//we shouldn't get here if they haven't chosed anything
	ASSERT(nGenType != cgtNoSelection);

	EMRInfoTable table;
	CString strComboString;

	// (j.gruber 2014-01-21 14:33) - PLID 60414 - only show drop down codes on flipped tables if they are all drop downs
	BOOL bCheckTypes = FALSE;

	//see if this item is in our map
	std::map<long, EMRInfoTable>::iterator itTable;
	itTable = m_mapEMRInfoTables.find(nItemID);
	if (itTable != m_mapEMRInfoTables.end())
	{
		//its here
		table = itTable->second;
	}

	else {
	
		//load from data
		BOOL bIsFlipped = FALSE;		
		
		//run our recordset to get the information
		ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT EMRInfoMasterT.ID AS MasterID, "
			" EMRInfoT.TableRowsAsFields as IsFlipped, "
			" EMRDataT.EMRDataGroupID as DataID, EMRDataT.Data, EMRDataT.ListType, EMRDataT.SortOrder "
			" FROM  "
			" EMRInfoMasterT INNER JOIN EMRInfoT ON EMRInfoMasterT.ActiveEMRInfoID = EMRInfoT.ID "
			" INNER JOIN EMRDataT ON EMRInfoT.ID = EMRDataT.EMRInfoID "
			" WHERE EMRDataT.Inactive = 0 AND EMRDataT.IsLabel = 0 "
			" AND EMRInfoMasterT.ID = {INT} "
			" ORDER BY SortOrder ASC", nItemID);
		while (!rs->eof)
		{
			bIsFlipped = AdoFldBool(rs->Fields, "IsFlipped");

			long nListType = AdoFldLong(rs->Fields, "ListType");

			EMRData data;
			data.nID = AdoFldLong(rs->Fields, "DataID");
			data.strName = AdoFldString(rs->Fields, "Data");
			data.nSortOrder = AdoFldLong(rs->Fields, "SortOrder");
			data.nListType = nListType;

			if (bIsFlipped && 
				nListType == LIST_TYPE_DROPDOWN)
			{
				bCheckTypes = TRUE;
			}
				
			
			if (nListType == LIST_TYPE_ROW)
			{
				table.rows.push_back(data);			
			}
			else {
				table.columns.push_back(data);
			}

			rs->MoveNext();
		}

		table.bIsFlipped = bIsFlipped;
		table.nMasterID = nItemID;

		//add this to our map
		m_mapEMRInfoTables.insert(std::make_pair(nItemID, table));
	}

	// (j.gruber 2014-01-21 14:33) - PLID 60414 - check te types if we have to
	if (bCheckTypes)
	{
		//run through the columns and see if they are all drop downs, because we know at least one is
		std::vector<EMRData>::iterator itCheck;
		BOOL bAllColumns = TRUE;
		for (itCheck = table.columns.begin(); itCheck != table.columns.end(); itCheck++)
		{
			if (itCheck->nListType != LIST_TYPE_DROPDOWN)
			{
				bAllColumns = FALSE;
			}
		}
		if (bAllColumns)
		{
			table.bShowCode = TRUE;
		}
	}

	switch (nGenType) 
	{
		case cgtRow:
		case cgtColumn:
		{
			//now that we have our data loaded, we can set the values
			std::vector<EMRData> dataToShow;
			CString strSpecifier;
			CString strItemType;
			if (nGenType == cgtRow)
			{		
				strSpecifier = "Col: ";
				strItemType = "Row";

				//are we flipped?
				if (!table.bIsFlipped){
					//we are flipped, show the rows as columns
					dataToShow = table.columns;			
				}
				else {
					dataToShow = table.rows;			
				}
			}
			else if (nGenType == cgtColumn)
			{
				strSpecifier = "Row: ";
				strItemType = "Column";
				if (table.bIsFlipped){
					//we are flipped, show the rows as columns
					dataToShow = table.columns;
				}
				else {
					dataToShow = table.rows;
				}
			}

			//make our combo string			
			std::vector<EMRData>::iterator it;
			BOOL  bXYCodeUsed = FALSE;
			for (it = dataToShow.begin(); it != dataToShow.end(); it++)
			{	
				EMRData data = *it;				

				CString strTmp;
				// (j.gruber 2014-01-30 12:24) - PLID 60535 - if this is a code field, don't let them choose values
				if (! IsCodeField(pField)) {				
					strTmp.Format("%li;%s%s;", data.nID, strSpecifier, data.strName);
					strComboString += strTmp;
				}
				else {								
					strTmp.Format("%liC;%s%s Code;", data.nID, strSpecifier, data.strName);
					strComboString += strTmp;

					//allow to have set the same code for what we are on
					if (bXYCodeUsed == FALSE) {
						strTmp.Format("-1C;%s Code;", strItemType);
						strComboString += strTmp;
						bXYCodeUsed = TRUE;
					}
					
				}

				if (IsCodeField(pField)
					&& (data.nListType == LIST_TYPE_DROPDOWN
					|| table.bShowCode)
					)
				{

					//add the drop down code also
					strTmp.Format("%liDC;%s%s Drop Down Code;", data.nID, strSpecifier, data.strName);
					strComboString += strTmp;
				}

			}
		}
		break;
		case cgtItem:
			//loop through our rows and columns and let them choose a cell
			std::vector<EMRData>::iterator itRows;
			for (itRows = table.rows.begin(); itRows != table.rows.end(); itRows++)
			{
				EMRData row = *itRows;

				std::vector<EMRData>::iterator itCols;
				for (itCols = table.columns.begin(); itCols != table.columns.end(); itCols++)
				{
					EMRData col = *itCols;

					CString strTmp;
					// (j.gruber 2014-01-30 12:25) - PLID 60535 - only allow code selections for code fields
					if (! IsCodeField(pField)) {
						if (!table.bIsFlipped) {
							strTmp.Format("%li_%li;Row: %s/Col: %s;", row.nID, col.nID, row.strName, col.strName);
						}
						else {
							//we are going to keep the ID as row_col, but change the name
							strTmp.Format("%li_%li;Row: %s/Col: %s;", row.nID, col.nID, col.strName, row.strName);
						}
						strComboString += strTmp;
					}
					else {									

						//add the cell code also	
						CString strTmp;
						if (!table.bIsFlipped) {
							strTmp.Format("%liC_%liC;Row: %s/Col: %s Cell Code;", row.nID, col.nID, row.strName, col.strName);
						}
						else {
							//we are going to keep the ID as row_col, but change the name
							strTmp.Format("%liC_%liC;Row: %s/Col: %s Cell Code;", row.nID, col.nID, col.strName, row.strName);
						}
						strComboString += strTmp;
					}
				}
			}				

				


			

		break;
	}

	return strComboString;

}

CString CCCDAConfigDlg::GetSourceFromField(CCDAFieldPtr pField, long nItemDataType, long nItemID, CCDAGenerationType nGenTypeID)
{
	CString strComboSource; 

	if (nItemDataType == eitMultiList)		
	{		
		// (j.gruber 2014-01-30 12:25) - PLID 60535 - only allow code values for code fields
		if (!IsCodeField(pField)) {
			strComboSource += AsString(nItemID) + ";<Item Selection>;";
		}
		else {	
			// (j.gruber 2014-02-03 13:56) - PLID 60616 - check the generation type
			//only allow the selection code if they want to generate one section per checkbox
			//if they want to concatenate the checkbox then they can only pick one code
			if (nGenTypeID == cgtSelection) 
			{
				strComboSource += AsString(nItemID) + "C;<Selection Code>;";
			}
		}
	}
	else if (nItemDataType == eitTable)
	{	
		if (nGenTypeID == cgtNoSelection) {
			//they haven't picked a GenType yet. so just tell them they need to do that first
			strComboSource += "-4;<Choose Section Type For Expanded Selection>;";
		}
		else {
			strComboSource += GetTableList(pField, nItemID, nGenTypeID);
		}
	}
	else {
		// (j.gruber 2014-01-30 12:26) - PLID 60535 - only allow code selections for coded fields
		if (! IsCodeField(pField)) {
			strComboSource += AsString(nItemID) + ";<Item Value>;";
		}		 
			
		if (IsCodeField(pField)
			&& nItemDataType == eitSingleList)
		{
			strComboSource += AsString(nItemID) + "C;<Selection Code>;";
		}
	}

	if (IsDateField(pField))
	{
		//add EMN Date to the sections
		strComboSource += "-1;<EMN Date>;";
	}
	// (j.gruber 2014-01-08 13:53) - take out ability to straight add codes for tables
	// (j.gruber 2014-01-28 13:26) - PLID 60199 - put it back in
	else if (IsCodeField(pField) /*&& nItemDataType != eitTable*/)
	{
		strComboSource += GetCodeListFromSystem(pField->strCodeSystem);
	}
	else
	{
		// this would only be valid for tables and would show rows or columns
	}
	
	return strComboSource;
	
}


NXDATALIST2Lib::IFormatSettingsPtr CCCDAConfigDlg::GetFieldFormatOverride(CCDAFieldPtr pField, long nItemDataType, long nItemID, CCDAGenerationType nGenTypeID)
{
	NXDATALIST2Lib::IFormatSettingsPtr pfs(__uuidof(NXDATALIST2Lib::FormatSettings));
	
	pfs->PutDataType(VT_BSTR);
	pfs->PutFieldType(NXDATALIST2Lib::cftComboSimple);					
	pfs->PutEditable(VARIANT_TRUE);
	pfs->PutConnection(_variant_t((LPDISPATCH)NULL)); 
	pfs->PutComboSource(_bstr_t(GetSourceFromField(pField, nItemDataType, nItemID, nGenTypeID)));			
	return pfs;	
}
NXDATALIST2Lib::IFormatSettingsPtr CCCDAConfigDlg::GetGenerationTypeFormatOverride(long nItemDataType)
{
	NXDATALIST2Lib::IFormatSettingsPtr pfs(__uuidof(NXDATALIST2Lib::FormatSettings));
	CString strCombo;
	switch (nItemDataType)
	{
		case eitMultiList:
			pfs->PutDataType(VT_I4);
			pfs->PutFieldType(NXDATALIST2Lib::cftComboSimple);					
			pfs->PutEditable(VARIANT_TRUE);
			pfs->PutConnection(_variant_t((LPDISPATCH)NULL)); 			
			strCombo.Format("%li;Item;%li;Selection;", cgtItem, cgtSelection);
			pfs->PutComboSource(_bstr_t(strCombo));		
		break;

		case eitTable:			
			pfs->PutDataType(VT_I4);
			pfs->PutFieldType(NXDATALIST2Lib::cftComboSimple);					
			pfs->PutEditable(VARIANT_TRUE);
			pfs->PutConnection(_variant_t((LPDISPATCH)NULL)); 			
			// (j.gruber 2014-01-08 12:57) - take out the ability to choose Item from table for the time being
			// (j.gruber 2014-01-16 15:15) - PLID 60193 - put it back in
			strCombo.Format("%li;Item;%li;Row;%li;Column", cgtItem, cgtRow, cgtColumn);
			pfs->PutComboSource(_bstr_t(strCombo));					
		break;

		default:	
			pfs->PutDataType(VT_I4);
			pfs->PutFieldType(NXDATALIST2Lib::cftComboSimple);					
			pfs->PutEditable(VARIANT_FALSE);			
			pfs->PutConnection(_variant_t((LPDISPATCH)NULL)); 			
			strCombo.Format("%li;Item;", cgtItem);
			pfs->PutComboSource(_bstr_t(strCombo));					
		break;
	}
	return pfs;
}
		
void CCCDAConfigDlg::SetComboSourceForFields(CCDASectionPtr pSection, NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	//loop through the fields and set the combo source for each column accordingly
	CCDAFieldIterator itField;
	for(itField = pSection->fields.begin(); itField != pSection->fields.end(); itField++)
	{
		CCDAFieldPtr pField = itField->second;
		long nDataType = VarLong(pRow->GetValue(clcItemDataType));
		long nItemID = VarLong(pRow->GetValue(clcItemID));
		_variant_t varGenType = pRow->GetValue(clcGenerationType);
		CCDAGenerationType nGenTypeID = cgtNoSelection;
		if (varGenType.vt != VT_EMPTY) {
			nGenTypeID = (CCDAGenerationType)VarLong(pRow->GetValue(clcGenerationType), cgtNoSelection);
		}
		pRow->PutRefCellFormatOverride(GetColumnFromField(pField), GetFieldFormatOverride(pField, nDataType,nItemID, nGenTypeID));
	}

}


void CCCDAConfigDlg::HandleItemType(long nItemID, CString strItemName, long nItemType, bool bIsFlipped)
{

	//first, lets find our section
	NXDATALIST2Lib::IRowSettingsPtr pRowSection = m_pTreeList->CurSel;
	CCDASectionPtr pSection(new CCDASection);
	if (pRowSection) {		
		pSection = FindTreeSection(pRowSection);
	}
	else {
		ThrowNxException("Error in CCCDAConfigDlg::HandleItemType - no current section selection");
	}

	
	//make sure we only have one text field
		
	//get our row for this item			
	NXDATALIST2Lib::IRowSettingsPtr pRow = GetSectionRow(pSection, (m_pConfigList->GetRowCount() == 0));

	//add the values we already know
	pRow->PutValue(clcItemID, nItemID);
	pRow->PutValue(clcIsFlipped, bIsFlipped);
	pRow->PutValue(clcItemDataType, nItemType);
	pRow->PutValue(clcItemName, _variant_t(strItemName));
	//setup our Generation Column
	if (nItemType != eitTable && nItemType != eitMultiList) {
		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pConfigList->GetColumn(clcGenerationType);			
		if (pCol)
		{
			pCol->PutFieldType(NXDATALIST2Lib::cftTextSingleLine);
			pRow->PutValue(clcGenerationType, cgtItem);
		}
	}
	pRow->PutRefCellFormatOverride(clcGenerationType, GetGenerationTypeFormatOverride(nItemType));
	SetComboSourceForFields(pSection, pRow);		

	//now add the row to the datalist
	m_pConfigList->AddRowAtEnd(pRow, NULL);

}
void CCCDAConfigDlg::SelChosenEmrInfoList(LPDISPATCH lpRow)
{
	try {		

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow)
		{
			// (j.gruber 2014-01-20 14:48) - PLID 60405 - make sure that we don't already have this item in the list
			long nItemID = VarLong(pRow->GetValue(eicID));

			NXDATALIST2Lib::IRowSettingsPtr pCheckRow = m_pConfigList->GetFirstRow();
			while (pCheckRow)
			{

				long nCheckItemID = VarLong(pCheckRow->GetValue(clcItemID));
				if (nItemID == nCheckItemID)
				{
					//we already selected this item for this section, tell them they cannot do that
					MsgBox("This EMR Item has already been selected in the configuration for this section.  You cannot have the same EMR item selected more than once for a given section.  You may want to choose a different value for the 'Generate 1 Section Per' column.");
					return;
				}
				pCheckRow = pCheckRow->GetNextRow();
			}


			//get the type of item this is
			long nDataType = (long)VarShort(pRow->GetValue(eicDataType));
			CString strItemName = VarString(pRow->GetValue(eicName));			
			bool bIsFlipped = !!VarBool(pRow->GetValue(eicIsFlipped));
			HandleItemType(nItemID, strItemName, nDataType, bIsFlipped);			
		}

	}NxCatchAll(__FUNCTION__);
}

void CCCDAConfigDlg::EditingFinishedOtherList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try
	{
		if (bCommit) {
			if (nCol == clcGenerationType)
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				if (pRow)
				{
					//get our section
					NXDATALIST2Lib::IRowSettingsPtr pTreeRow = m_pTreeList->CurSel;
					if (pTreeRow) {
						CCDASectionPtr pSection = FindTreeSection(pTreeRow);

						//we need to clear out their existing selections					
						if (varOldValue.vt == VT_I4) {
							long nInfoMasterID = VarLong(pRow->GetValue(clcItemID));
							RemoveItemFromSection(pSection, nInfoMasterID);

							//clear the fields
							for(int i=MAX_CONFIG_DEF_COLS+1; i < m_pConfigList->GetColumnCount(); i++) {							
								pRow->PutValue(i, g_cvarEmpty);
							}							
						}
						SetComboSourceForFields(pSection, pRow);
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CCCDAConfigDlg::RButtonUpOtherList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		CMenu Popup;
		Popup.m_hMenu = CreatePopupMenu();		
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {		
			m_pConfigList->CurSel = pRow;
			Popup.InsertMenu(3, MF_BYPOSITION, ID_REMOVE_ITEM, "Remove");
		}		

		CPoint pt;
		pt.x = x;
		pt.y = y;
		CWnd* pwnd = GetDlgItem(IDC_OTHER_LIST);
		if (pwnd != NULL) {
			pwnd->ClientToScreen(&pt);
			Popup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}
	}NxCatchAll(__FUNCTION__);
}

void CCCDAConfigDlg::RemoveItemFromSection(CCDASectionPtr pSection, long nInfoMasterID)
{
	//loop through our section fields and remove any values from this infoID
	CCDAFieldIterator itField;
	for (itField = pSection->fields.begin(); itField != pSection->fields.end(); itField++)
	{
		CCDAFieldPtr pField = itField->second;

		CCDAValueMap::iterator itValue;
		itValue = pField->mapSelections.find(nInfoMasterID);
		if (itValue != pField->mapSelections.end())
		{
			pField->mapSelections.erase(itValue);
		}
	}
}

void CCCDAConfigDlg::OnRemoveItem()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConfigList->CurSel;

		if (pRow) 
		{
			//we have to remove it, first get the values we need out of it
			long nInfoMasterID = VarLong(pRow->GetValue(clcItemID));

			NXDATALIST2Lib::IRowSettingsPtr pSectionRow = m_pTreeList->CurSel;
			if (pSectionRow) {
				CCDASectionPtr pSection = FindTreeSection(pSectionRow);
				RemoveItemFromSection(pSection, nInfoMasterID);
				m_pConfigList->RemoveRow(pRow);
			}
		}

		//if there are no more fields left, clear the columms also
		if (m_pConfigList->GetRowCount() == 0) 
		{
			long nCount = m_pConfigList->GetColumnCount();
			for (short nCol = m_pConfigList->GetColumnCount() - 1; nCol >= 0; nCol--) {
				m_pConfigList->RemoveColumn(nCol);
			}
		}
	}NxCatchAll(__FUNCTION__);

}
void CCCDAConfigDlg::SelChosenCcdaDocumentList(LPDISPATCH lpRow)
{
	try{

		//we have already saved by the time we are here

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow)
		{
			CString strDocName = VarString(pRow->GetValue(dlcID));

			//look it up in our map
			CCDADocumentMap::iterator itDoc;
			itDoc = m_mapDocuments.find(strDocName);

			if (itDoc != m_mapDocuments.end())
			{
				//load the section list
				//TES 2/5/2014 - PLID 60671 - sectionMap is now just one component of docContents
				CCDADocumentContents docContents = itDoc->second;
				CCDASectionMap sectionMap = docContents.sectionMap;
				
				//clear our the configuration
				m_pConfigList->Clear();

				//and the fields
				m_pTreeList->Clear();

				//column too	
				long nCount = m_pConfigList->GetColumnCount();
				for (short nCol = m_pConfigList->GetColumnCount() - 1; nCol >= 0; nCol--) {
					m_pConfigList->RemoveColumn(nCol);
				}

				LoadTree(NULL, sectionMap);		
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CCCDAConfigDlg::SelChangingCcdaDocumentList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try
	{
		if (*lppNewSel == NULL) {			
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	
		//save our fields
		NXDATALIST2Lib::IRowSettingsPtr pOldRow(lpOldSel);
		if (pOldRow) {				
			CCDASectionPtr pOldSection = FindTreeSection(pOldRow);
			if (!SaveFields(pOldSection))
			{
				SafeSetCOMPointer(lppNewSel, lpOldSel);
			}

		}

	}NxCatchAll(__FUNCTION__);
}

void CCCDAConfigDlg::SelChangingCcdaTreeList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try{
		
		NXDATALIST2Lib::IRowSettingsPtr pOldRow(lpOldSel);
		if (pOldRow) {				
			CCDASectionPtr pOldSection = FindTreeSection(pOldRow);
			if (!ValidateFields(pOldSection))
			{
				SafeSetCOMPointer(lppNewSel, lpOldSel);
			}

		}

	}NxCatchAll(__FUNCTION__);
}

void CCCDAConfigDlg::EditingFinishingOtherList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try
	{		
		if (*pbCommit) {
			if (nCol == clcGenerationType)
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				if (pRow)
				{	
					if (varOldValue.vt == VT_I4) {
						if (IDNO == MsgBox(MB_YESNO, "Switching the generation type will clear out any field selections for this item, are you sure you wish to continue?"))
						{						
							*pbCommit = FALSE;
						}	
					}								
				}
			}	
		}
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2015-10-28 11:46) - PLID 67425 - Configuration for CCDA export locations
void CCCDAConfigDlg::OnBnClickedCcdaAutoExportBtn()
{
	try {

		CCCDAAutoExportDlg dlg(this);
		dlg.DoModal();

	} NxCatchAll(__FUNCTION__);
}
