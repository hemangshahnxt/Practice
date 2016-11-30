#pragma once
#include <NxDataUtilitiesLib/NxSafeArray.h>

class EmrOmrMap
{
public:
	EmrOmrMap(void);
	~EmrOmrMap(void);
	
	EmrOmrMap(long nFormID); 

	//selections 
	enum LoadingTypeSelection {
		ltsForm = 0, 
		ltsTemplate, 
	};

	// (b.spivey, August 28, 2012) - PLID 51878 - Data type uses the actual values in the database. 
	enum EMNDataType {
		edtSingleSel = 2, 
		edtMultiSel = 3,
	};


	//Structure to hold the info we need for the dialogs. 
	struct EmrItemDetail {
		long ItemID;
		CString ItemName;
		CString SelectionText;
		long DataGroupID;
		long OmrID; 
		long TemplateDetailID; 
		EMNDataType DataType; 
	};

	//CSqlFragment for the OMR Form. 
	CSqlFragment EmrOmrFormFrag(long nFormID);
	//CSqlFragment for the EMR Template. 
	CSqlFragment EmrOmrTemplateFrag(long nTemplateID); 

	//Given a loading ID and the type of loading (ltsForm vs ltsTemplate) we're doing, we can fill a map object.
	void LoadEmrOmrMap(long nLoadingID, LoadingTypeSelection ltsSel);
	//Save the edited form to the database. 
	void SaveEmrOmrMap();

	// (b.spivey, September 06, 2012) - PLID 52456 - Get/Set functions and values. 
	//Given an EmrDataGroupID, we can update its OMR mapping. 
	void ModifyOmrMapValue(long nDataGroupID, long nOmrID, bool bDelete = false); 
	//Given an EMRTemplateID and Description, we can create a new form.
	long CreateNewMap(long nTemplateID, CString strDescription);
	//Given a FormID, we delete a form. 
	void DeleteEmrOmrMap(); 
	//Update form description 
	void SetDescription(CString strDescription);
	//Get the modified status
	bool GetModifiedStatus(); 
	//get/set file location
	void SetFileLocation(CString strFileLocation);
	CString GetFileLocation(); 
	//No Original Desc. anymore. 
	//Get Current Description 
	CString GetCurrentDesc(); 
	//Get Current Template ID 
	long GetCurrentTemplateID();
	//Get the current Form ID
	long GetCurrentFormID(); 
	//Clean Object. 
	void ClearMap();
	// (b.spivey, February 28, 2013) - PLID 53955 - functions to sort the array based off output from the API.
	void SortDetailsArray(CMap<long, long, long, long>& mapStartingIndex);
	void SortArrayRecurse(Nx::SafeArray<IUnknown *> ptrTopics, CArray<EmrItemDetail, EmrItemDetail>& arySortDetails,
		CMap<long, long, long, long>& mapStartingIndex);
	

	//Traversing the array.
	EmrItemDetail GetItemAt(long nIndex); 
	//Size of array. 
	long GetItemCount(); 


	
private:

	//Form and template IDs. 
	long m_nFormID; 

	//Current description.
	CString m_strCurrentDesc; 

	//Current omr form template file path 
	CString m_strFileLocation;

	//Original and current templates. 
	long m_nOriginalTemplateID;
	long m_nCurrentTemplateID; 
	
	//Modified? Need to save. 
	bool m_bModified; 
	
	//Holds all the items.
	CArray<EmrItemDetail, EmrItemDetail> m_aryDetails; 
};
