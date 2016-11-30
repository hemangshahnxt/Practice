#pragma once

#include "Administratorrc.h"
#include "NxXMLDocument.h"
// CCCDAConfigDlg dialog
// (j.gruber 2013-10-17 09:36) - PLID 59062




struct EMRData{

	EMRData()
	{
		nID = -1;
		nSortOrder = -1;
		nListType = -1;
	}

	long nID;
	CString strName;
	long nSortOrder;
	long nListType;
};


struct EMRInfoTable
{
	EMRInfoTable()
	{
		nMasterID = -1;
		bIsFlipped = FALSE;
		bShowCode = FALSE;
		rows.empty();
		columns.empty();
	}
	long nMasterID;
	BOOL bIsFlipped;
	BOOL bShowCode;
	std::vector<EMRData> rows;
	std::vector<EMRData> columns;
};


enum CCDAValueType
{
	cvtNone,
	cvtEMRInfo,
	cvtEMNDate,
	cvtCell,
	cvtEMRData,
	cvtCode,
	cvtEMRDataCode,
	cvtEMRDataDropDownCode,
	cvtEMRCellCode,
	cvtEMRDataCode_X,
	cvtEMRDataCode_Y,

};

enum CCDAGenerationType
{
	cgtNoSelection = -1,
	cgtItem = 1,
	cgtSelection,
	cgtRow,
	cgtColumn,	
};

struct CCDAValue
{

	CCDAValue()
	{
		type = cvtNone;
		nID = -1;
		X_ID = -1;
		Y_ID = -1;
		nDataType = -1;
		nGenTypeID = cgtNoSelection;
		bIsFlipped = false;
	}

	CCDAValueType type;
	long nID;
	long X_ID;
	long Y_ID;
	long nDataType;
	CString strInfoName;
	CCDAGenerationType nGenTypeID;
	bool bIsFlipped;
};

typedef boost::shared_ptr<CCDAValue> CCDAValuePtr;
typedef std::map<long, CCDAValuePtr> CCDAValueMap;

struct CCDAField{

	CCDAField()
	{
		isRequired = FALSE;
		mapSelections.empty();
	}

	CString strName;
	bool isRequired;
	CString strDescription;
	CString strLimit;
	CString strType;
	CString strCodeSystem;

	//stored the configuration values
	CCDAValueMap mapSelections;
};



typedef boost::shared_ptr<CCDAField> CCDAFieldPtr;
typedef std::map<CString, CCDAFieldPtr> CCDAFieldMap;
typedef CCDAFieldMap::iterator CCDAFieldIterator;
typedef CCDAFieldMap::const_iterator CCDAConstFieldIterator;


class CCDASection;
typedef boost::shared_ptr<CCDASection> CCDASectionPtr;
typedef std::map<CString, CCDASectionPtr> CCDASectionMap;
typedef CCDASectionMap::iterator CCDASectionIterator;
typedef CCDASectionMap::const_iterator CCDAConstSectionIterator;
//TES 2/5/2014 - PLID 60671 - Documents now have uid and ContentTimeStamp attributes along with their name, so we will map the name to this struct instead
// of just the sectionMap
struct CCDADocumentContents {
	CCDASectionMap sectionMap;
	CString strUID;
	COleDateTime dtContentTimeStamp;
};

typedef std::map<CString, CCDADocumentContents> CCDADocumentMap;

class CCDASection
{
public:
	CCDASection() 
	{ 
		isMajorSection = false;
		isRequired = false;		
		fields.empty();
		innerSection.empty();
	}	
	
	bool isMajorSection;
	bool isRequired;
	CString strName;
	CString strDescription;
	CCDAFieldMap fields;
	CCDASectionMap innerSection;		
};

class CCCDAConfigDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CCCDAConfigDlg)

public:


	CCCDAConfigDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCCDAConfigDlg();

	NXDATALIST2Lib::_DNxDataListPtr m_pTreeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pFieldDataList;
	NXDATALIST2Lib::_DNxDataListPtr m_pConfigList;
	NXDATALIST2Lib::_DNxDataListPtr m_pEMRInfoList;
	NXDATALIST2Lib::_DNxDataListPtr m_pDocumentList;

	std::map<CString, CString> m_mapCodes;
	std::map<long, EMRInfoTable> m_mapEMRInfoTables;	

	CCDADocumentMap m_mapDocuments;
	//CCDASectionMap m_mapMajorSections;

	/*Configuration XML Functions*/
	void LoadXML();

	// (j.jones 2015-02-17 10:27) - PLID 64880 - added section name
	CCDASectionPtr LoadSection(CString strSectionName, MSXML2::IXMLDOMNodePtr xmlSectionConfig, MSXML2::IXMLDOMNodePtr xmlSectionSaved, bool bisMajor);

	// (j.jones 2015-02-17 10:27) - PLID 64880 - added section name
	void LoadValues(CString strSectionName, CCDAFieldPtr pField, MSXML2::IXMLDOMNodePtr xmlFieldSaved);

	// (j.jones 2015-02-17 10:44) - PLID 64880 - cache the last item warning
	CString m_strLastRemovedItemWarning;

	MSXML2::IXMLDOMNodePtr FindNodeFromList(MSXML2::IXMLDOMNodeListPtr xmlList, CString strNameToFind);

	void HandleItemType(long nItemID, CString strItemName, long nItemType, bool bIsFlipped);
	

	/*Datalist Functions*/
	void LoadTree(NXDATALIST2Lib::IRowSettingsPtr pParentRow, CCDASectionMap map);
	CCDASectionPtr FindTreeSection(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void GetTreePath(NXDATALIST2Lib::IRowSettingsPtr pRow, CStringArray &aryPath);
	NXDATALIST2Lib::EColumnFieldType GetTypeFromField(CCDAFieldPtr field);
	BOOL SaveFields(CCDASectionPtr pOldSection);
	void LoadFields(CCDASectionPtr pSection);
	NXDATALIST2Lib::IRowSettingsPtr GetSectionRow(CCDASectionPtr pSection, BOOL bAddColumns);		
	

	/*map Functions*/
	CString GetSourceFromField(CCDAFieldPtr pField, long nItemDataType, long nItemID, CCDAGenerationType nGenTypeID);
	void ClearSelections(CCDASectionPtr pOldSection);
	void RemoveItemFromSection(CCDASectionPtr pSection, long nInfoMasterID);

	/*Utility Functions*/
	CString GetCodeListFromSystem(CString strCodeSystem);
	BOOL ValidateFields(CCDASectionPtr section);	
	CCDAValueType GetValueType(CCDAFieldPtr field, NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::IColumnSettingsPtr pCol);
	short GetColumnFromField(CCDAFieldPtr pField);
	_variant_t GetValueFromType(CCDAValuePtr pValue);
	//long GetItemValue(CCDAValueMap mapSelections, long nItemID);
	NXDATALIST2Lib::IFormatSettingsPtr GetGenerationTypeFormatOverride(long nItemDataType);
	NXDATALIST2Lib::IFormatSettingsPtr GetFieldFormatOverride(CCDAFieldPtr pField, long nItemDataType, long nItemID, CCDAGenerationType nGenTypeID);
	void SetComboSourceForFields(CCDASectionPtr pSection, NXDATALIST2Lib::IRowSettingsPtr pRow);
	CString GetTableList(CCDAFieldPtr pField, long nItemID, CCDAGenerationType nGenType);
	BOOL IsCodeField(CCDAFieldPtr pField);
	BOOL IsDateField(CCDAFieldPtr pField);
	CCDAValueType GetValueTypeFromCell(BOOL bIsCodeField, BOOL bIsDateField, _variant_t varValue);
	CCDAValueType GetValueTypeFromSelection(BOOL bIsCodeField, BOOL bIsDateField, _variant_t varValue, CCDAGenerationType genType);
	CCDAValueType GetValueTypeFromValue(BOOL bIsCodeField, BOOL bIsDateField, _variant_t varValue);
	BOOL ConfirmItemType(long nInfoMasterID, long nSavedType, CString &strItemName);


	MSXML2::IXMLDOMNodePtr CCCDAConfigDlg::AddSectionToXML(CCDASectionMap mapSections);


// Dialog Data
	enum { IDD = IDD_CONFIGURE_CCDA_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	DECLARE_EVENTSINK_MAP()
	/*void SelChangingMajorSectionList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenMajorSectionList(LPDISPATCH lpRow);
	void SelChangingSectionList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenSectionList(LPDISPATCH lpRow);
	void SelChangingFieldList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenFieldList(LPDISPATCH lpRow);*/
	void SelChangedTreeList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	//afx_msg void OnBnClickedSwitchType();
	void SelChosenEmrInfoList(LPDISPATCH lpRow);
	void EditingFinishedOtherList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void RButtonUpOtherList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRemoveItem();
	void SelChosenCcdaDocumentList(LPDISPATCH lpRow);
	void SelChangingCcdaDocumentList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingCcdaTreeList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void EditingFinishingOtherList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	afx_msg void OnBnClickedCcdaAutoExportBtn();
};


class CCDAXmlDocument : public NxXmlDocument
{
public:
	CCDAXmlDocument(){}
	~CCDAXmlDocument(){}
protected:
	virtual void CreateRootNode()
	{
		Root = CreateNode("Documents");
	}
private:
	MSXML2::IXMLDOMNodePtr AddSectionToXML(CCDASectionPtr pSection)
	{
		MSXML2::IXMLDOMNodePtr xmlSection;
		
		//create our section
		if (pSection->isMajorSection)
		{
			xmlSection = CreateNode("MajorSection");
		}
		else {
			xmlSection = CreateNode("Section");
		}
		
		//put the section name, that's the only thing we need for the save string
		xmlSection->attributes->setNamedItem(CreateAttribute("name", _variant_t(pSection->strName)));		
		
		//are there any fields in this section?
		CCDAFieldIterator itField;
		for (itField = pSection->fields.begin(); itField != pSection->fields.end(); itField++)
		{
			//add our field nodes
			CCDAFieldPtr pField = itField->second;
			MSXML2::IXMLDOMNodePtr xmlField = CreateNode("Field");
			xmlField->attributes->setNamedItem(CreateAttribute("name", _variant_t(pField->strName)));			

			//now add our values
			CCDAValueMap::iterator itValue;
			for (itValue = pField->mapSelections.begin(); itValue != pField->mapSelections.end(); itValue++)
			{
				CCDAValuePtr pValue = itValue->second;
				MSXML2::IXMLDOMNodePtr xmlValue = CreateNode("Value");

				//we need to add all our information about the value

				//we have to flip the EMRDataCode types if the table is flipped
				if (pValue->bIsFlipped) {
					if (pValue->type == cvtEMRDataCode_X)
					{
						pValue->type = cvtEMRDataCode_Y;
					}
					else if (pValue->type == cvtEMRDataCode_Y)
					{
						pValue->type = cvtEMRDataCode_X;
					}
				}
				xmlValue->attributes->setNamedItem(CreateAttribute("type", pValue->type));
				xmlValue->attributes->setNamedItem(CreateAttribute("InfoMasterID", itValue->first));
				xmlValue->attributes->setNamedItem(CreateAttribute("ItemType", pValue->nDataType));
				xmlValue->attributes->setNamedItem(CreateAttribute("IsFlipped", pValue->bIsFlipped));
				switch (pValue->type)
				{		
					case cvtNone:
						//this should not exist
						ThrowNxException("Error in AddSectionToXML - none value selected");
					break;

					case cvtEMRInfo:
					case cvtEMRData:
					case cvtCode:
					case cvtEMRDataCode:
					case cvtEMRDataDropDownCode:
					case cvtEMRDataCode_X:
					case cvtEMRDataCode_Y:
						xmlValue->attributes->setNamedItem(CreateAttribute("ID", pValue->nID));
					break;
					case cvtEMNDate:					
						//none needed for this
					break;
						
					case cvtCell:
					case cvtEMRCellCode:
						xmlValue->attributes->setNamedItem(CreateAttribute("X_ID", pValue->X_ID));
						xmlValue->attributes->setNamedItem(CreateAttribute("Y_ID", pValue->Y_ID));
					break;				

					default:
						ThrowNxException("Error in AddSectionToXML - unknown type selected");
					break;
					
				}

				if (pValue->bIsFlipped)
				{
					if (pValue->nGenTypeID == cgtRow)
					{
						xmlValue->attributes->setNamedItem(CreateAttribute("GenerationType", cgtColumn));						
					}
					else if (pValue->nGenTypeID == cgtColumn)
					{
						xmlValue->attributes->setNamedItem(CreateAttribute("GenerationType", cgtRow));
					}
					else {								
						xmlValue->attributes->setNamedItem(CreateAttribute("GenerationType", pValue->nGenTypeID));
					}
				}
				else {
					xmlValue->attributes->setNamedItem(CreateAttribute("GenerationType", pValue->nGenTypeID));
				}

				CString str = (LPCTSTR)xmlValue->Gettext();

				//add this node to the field
				xmlField->appendChild(xmlValue);
				
			}
			
			//now add our field node to the section
			xmlSection->appendChild(xmlField);
		}

		//loop through our map and write a new XML field with our saved values in it
		CCDASectionIterator itSection;
		for(itSection = pSection->innerSection.begin(); itSection != pSection->innerSection.end(); itSection++)
		{
			CCDASectionPtr pInnerSection = itSection->second;
			xmlSection->appendChild(AddSectionToXML(pInnerSection));		
		}
	
		//MsgBox(xmlSection->Getxml());
		return xmlSection;
	}
public:
	CString GenerateSaveText(CCDADocumentMap documentMap)
	{			
		InitializeDocument();
		CCDADocumentMap::iterator itDoc;
		for (itDoc = documentMap.begin(); itDoc != documentMap.end(); itDoc++)
		{
			CString strDocName = itDoc->first;
			CCDADocumentContents docContents = itDoc->second;
			
			//TES 2/5/2014 - PLID 60671 - Documents now have uid and ContentTimeStamp attributes along with their name
			MSXML2::IXMLDOMNodePtr pDoc = Root->appendChild(CreateNode("Document"));
			pDoc->attributes->setNamedItem(CreateAttribute("name", _variant_t(strDocName)));
			if(docContents.strUID.IsEmpty()) {
				docContents.strUID = NewUUID();
			}
			pDoc->attributes->setNamedItem(CreateAttribute("uid", _variant_t(docContents.strUID)));
			//TES 2/5/2014 - PLID 60671 - ContentTimeStamp won't be present unless this document was imported with NexEmrImporter
			if(docContents.dtContentTimeStamp.GetStatus() == COleDateTime::valid) {
				pDoc->attributes->setNamedItem(CreateAttribute("ContentTimeStamp", _variant_t(docContents.dtContentTimeStamp, VT_DATE)));
			}
		
			CCDASectionIterator itSection;
			for (itSection = docContents.sectionMap.begin(); itSection != docContents.sectionMap.end(); itSection++) 
			{
				CCDASectionPtr pSection = itSection->second;
				pDoc->appendChild(AddSectionToXML(pSection));
			}				
		}
			Finalize();

		CString strOutput = GetText();
//#ifdef _DEBUG 
//		MsgBox(strOutput);
//#endif

		return strOutput;
	}
};