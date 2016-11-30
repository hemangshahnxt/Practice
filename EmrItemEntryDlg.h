#if !defined(AFX_EMRITEMENTRYDLG_H__D36A114E_DDDB_413C_A90C_4D7A20B60997__INCLUDED_)
#define AFX_EMRITEMENTRYDLG_H__D36A114E_DDDB_413C_A90C_4D7A20B60997__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrItemEntryDlg.h : header file
//

#include "EmrUtils.h"
#include "EMRTableDropdownEditorDlg.h"
#include "GlobalDrawingUtils.h"
#include "EMRHotSpot.h"
#import "RichTextEditor.tlb"
#include "InvVisionWebUtils.h"
#include "EmrInfoCommonListCollection.h" // (c.haag 2011-03-14) - PLID 42813
#include "NxInkPictureImport.h"
#include "WoundCareCalculator.h" // (r.gonet 08/03/2012) - PLID 51735
#include "EMRTableCellCodes.h"

// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray

// (j.jones 2013-05-16 15:18) - PLID 56596 - replaced EMN.h with a forward declare
class CEMN;

static long g_nLastTableDropDownIndex;





// (c.haag 2008-01-22 09:26) - PLID 28686 - We can now customize this dialog's behavior. 
// If the behavior is eEmrItemEntryDlgBehavior_Normal, then the dialog will behave as
// it always has when someone right-clicks on an item to edit it on the fly, or edits an
// item from the administrator module. If the behavior is eEmrItemEntryDlgBehavior_AddNewDropdownColumnSelection,
// then the dialog will take the user to the edit dropdown dialog without them having to
// press any buttons.
typedef enum {
	eEmrItemEntryDlgBehavior_Normal,
	eEmrItemEntryDlgBehavior_AddNewDropdownColumnSelection,
	// (z.manning 2010-02-12 11:34) - PLID 37320 - Added another method here where we open the dialog
	// but do not display it and then save right away. Its initial purpose was for smart stamp tables
	// because we need to ensure certain columns exist.
	eEmrItemEntryDlgBehavior_OpenInvisibleAndSave,
	// (c.haag 2011-03-30) - PLID 43047 - Open straight into the common list configuration
	eEmrItemEntryDlgBehavior_EditCommonLists,
} EEmrItemEntryDlgBehavior;

//////////////////////////
// CEmrInfoDataElement is essentially an initializeable struct to store the properties of an emr info data element
class CEmrInfoDataElement
{
public:
	// (z.manning 2010-02-11 17:05) - PLID 37324 - Added ListSubType
	// (c.haag 2010-02-23 17:47) - PLID 37488 - AutoAlphabetizeDropDown
	// (j.gruber 2010-04-26 12:32) - PLID 38336 - BOLD Code
	// (z.manning 2010-08-11 13:12) - PLID 40074 - AutoNumber fields
	// (j.jones 2011-03-08 11:48) - PLID 42282 - added EMCodeCategoryID
	//TES 3/11/2011 - PLID 42757 - Added Glasses Order data
	// (z.manning 2011-03-21 09:39) - PLID 23662 - Added autofill type
	// (z.manning 2011-05-26 14:47) - PLID 43865 - Added flags param
	// (z.manning 2011-09-19 13:20) - PLID 41954 - Added dropdown separators
	// (z.manning 2011-11-07 10:38) - PLID 46309 - Added SpawnedItemsSeparator
	// (r.gonet 08/03/2012) - PLID 51735 - Added WoundCareDataType
	// (j.jones 2012-09-19 12:22) - PLID 52316 - added ParentLabelID
	// (j.gruber 2013-09-25 12:19) - PLID 58754 - added Data GroupID
	// (j.gruber 2013-10-01 14:03) - PLID 58674 - added EMR Codes
	// (j.gruber 2014-07-17 13:23) - PLID 62621 - added strKeywordOverride and bUseKeyword
	// (j.gruber 2014-12-05 15:37) - PLID 64361 - Load the new values for UseNameForKeyword
	CEmrInfoDataElement(long nID, const CString &strData, long nSortOrder, BOOL bDefault, BOOL bInactive, long nListType, BOOL bIsGrouped, const CString &strLongForm, BOOL bIsLabel, long nParentLabelID, CString strOldParentLabelName, CEmrInfoDataElement *peideParentLabelPtr,
		BOOL bUseEMCoding, long nEMCodeCategoryID, CString strFormula, BYTE nDecimalPlaces, CString strInputMask, BYTE nListSubType, BOOL bAutoAlphabetizeDropDown, CString strBoldCode, short nAutoNumberType, CString strAutoNumberPrefix, GlassesOrderDataType GlassesOrderDataType, long nGlassesOrderDataID, EmrTableAutofillType eAutofillType, long nFlags, LPCTSTR strDropdownSeparator, LPCTSTR strDropdownSeparatorFinal, LPCTSTR strSpawnedItemsSeparator, EWoundCareDataType ewccWoundCareDataType, long nDataGroupID, CString strKeywordOverride, BOOL bUseKeyword, BOOL bUseNameForKeyword)
		: m_nID(nID), m_strData(strData), m_nSortOrder(nSortOrder), m_bDefault(bDefault), m_bInactive(bInactive), m_nListType(nListType), m_bIsGrouped(bIsGrouped), m_strLongForm(strLongForm), m_bIsLabel(bIsLabel), m_nParentLabelID(nParentLabelID), m_strOldParentLabelName(strOldParentLabelName), m_peideParentLabelPtr(peideParentLabelPtr), m_bUseEMCoding(bUseEMCoding), m_nEMCodeCategoryID(nEMCodeCategoryID), m_strFormula(strFormula), m_nDecimalPlaces(nDecimalPlaces), m_strInputMask(strInputMask), m_nListSubType(nListSubType), m_bAutoAlphabetizeDropDown(bAutoAlphabetizeDropDown), m_strBoldCode(strBoldCode), m_nAutoNumberType(nAutoNumberType), m_strAutoNumberPrefix(strAutoNumberPrefix), m_GlassesOrderDataType(GlassesOrderDataType), m_nGlassesOrderDataID(nGlassesOrderDataID), m_eAutofillType(eAutofillType), m_nFlags(nFlags), m_strDropdownSeparator(strDropdownSeparator), m_strDropdownSeparatorFinal(strDropdownSeparatorFinal), m_strSpawnedItemsSeparator(strSpawnedItemsSeparator), m_ewccWoundCareDataType(ewccWoundCareDataType), m_nDataGroupID(nDataGroupID), m_strKeywordOverride(strKeywordOverride), m_bUseKeyword(bUseKeyword), m_bUseNameForKeyword(bUseNameForKeyword) { };
	CEmrInfoDataElement(const CEmrInfoDataElement &f) 
		: m_nID(f.m_nID), m_strData(f.m_strData), m_nSortOrder(f.m_nSortOrder), m_bDefault(f.m_bDefault), m_bInactive(f.m_bInactive), m_nListType(f.m_nListType), m_bIsGrouped(f.m_bIsGrouped), m_strLongForm(f.m_strLongForm), m_bIsLabel(f.m_bIsLabel), m_nParentLabelID(f.m_nParentLabelID), m_strOldParentLabelName(f.m_strOldParentLabelName), m_peideParentLabelPtr(f.m_peideParentLabelPtr),
		m_bUseEMCoding(f.m_bUseEMCoding), m_nEMCodeCategoryID(f.m_nEMCodeCategoryID), m_strFormula(f.m_strFormula), m_nDecimalPlaces(f.m_nDecimalPlaces), m_nVisibleIndex(f.m_nVisibleIndex), m_strInputMask(f.m_strInputMask), m_nListSubType(f.m_nListSubType), m_bAutoAlphabetizeDropDown(f.m_bAutoAlphabetizeDropDown), m_strBoldCode(f.m_strBoldCode), m_nAutoNumberType(f.m_nAutoNumberType), m_strAutoNumberPrefix(f.m_strAutoNumberPrefix), m_GlassesOrderDataType(f.m_GlassesOrderDataType), m_nGlassesOrderDataID(f.m_nGlassesOrderDataID), m_eAutofillType(f.m_eAutofillType), m_nFlags(f.m_nFlags), m_strDropdownSeparator(f.m_strDropdownSeparator), m_strDropdownSeparatorFinal(f.m_strDropdownSeparatorFinal), m_strSpawnedItemsSeparator(f.m_strSpawnedItemsSeparator), m_ewccWoundCareDataType(f.m_ewccWoundCareDataType), m_nDataGroupID(f.m_nDataGroupID), m_strKeywordOverride(f.m_strKeywordOverride), m_bUseKeyword(f.m_bUseKeyword), m_bUseNameForKeyword(f.m_bUseNameForKeyword)
	{
		m_arypEMRDropDownList.AppendCopy(f.m_arypEMRDropDownList);
		m_arypEMRDropDownDeleted.AppendCopy(f.m_arypEMRDropDownDeleted);
		// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - Needed for vs2008
		m_arActions.Append(f.m_arActions);
		// (j.gruber 2013-10-01 14:04) - PLID 58674
		m_aryCodes = f.m_aryCodes;
		//for(int i = 0; i < f.m_arActions.GetSize(); i++) m_arActions.Add(f.m_arActions[i]);
	};

public:
	long m_nID;
	CString m_strData;
	long m_nSortOrder;
	BOOL m_bDefault;
	BOOL m_bIsGrouped;
	BOOL m_bInactive;
	long m_nListType;
	BYTE m_nListSubType;  // (z.manning 2010-02-11 17:06) - PLID 34324
	BOOL m_bAutoAlphabetizeDropDown; // (c.haag 2010-02-23 17:48) - PLID 37488
	CString m_strBoldCode; // (j.gruber 2010-04-26 12:34) - PLID 38336	
	MFCArray<EmrAction> m_arActions;
	CString m_strLongForm;
	BOOL m_bIsLabel;
	// (j.jones 2012-09-19 12:22) - PLID 52316 - added ParentLabelID,
	// the m_strOldParentLabelName is only used for auditing
	long m_nParentLabelID;
	CString m_strOldParentLabelName;
	// (j.jones 2007-08-15 11:50) - PLID 27053 - added UseEMCoding
	BOOL m_bUseEMCoding;
	// (j.jones 2011-03-08 11:48) - PLID 42282 - added EMCodeCategoryID
	long m_nEMCodeCategoryID;
	// (z.manning, 05/22/2008) - PLID 16443 - Added Formula and DecimalPlaces
	CString m_strFormula;
	BYTE m_nDecimalPlaces;
	// (z.manning 2009-01-13 15:06) - PLID 32719 - Added InputMask
	CString m_strInputMask;
	CString m_strAutoNumberPrefix; // (z.manning 2010-08-11 13:19) - PLID 40074
	short m_nAutoNumberType; // (z.manning 2010-08-11 13:20) - PLID 40074
	//TES 3/11/2011 - PLID 42757 - Added Glasses Order data
	GlassesOrderDataType m_GlassesOrderDataType;
	long m_nGlassesOrderDataID;
	EmrTableAutofillType m_eAutofillType; // (z.manning 2011-03-21 09:43) - PLID 23662
	long m_nFlags; // (z.manning 2011-05-26 14:48) - PLID 43865
	// (z.manning 2011-09-19 13:22) - PLID 41954 - Dropdown separators
	CString m_strDropdownSeparator;
	CString m_strDropdownSeparatorFinal;
	// (z.manning 2011-11-07 10:39) - PLID 46309 - Added customizable separator for the spawned item field
	CString m_strSpawnedItemsSeparator;
	// (r.gonet 08/03/2012) - PLID 51735 - Marks this column as being special to the Wound Care Calculator
	EWoundCareDataType m_ewccWoundCareDataType;
	long m_nDataGroupID; // (j.gruber 2013-09-25 12:21) - PLID 58754

	// (z.manning, 05/30/2008) - PLID 16443 - Added visible index
	long m_nVisibleIndex;

	// (j.gruber 2013-10-01 14:04) - PLID 58476 - codes
	CEMRCodeArray m_aryCodes;

	CEmrTableDropDownItemArray m_arypEMRDropDownList;
	CEmrTableDropDownItemArray m_arypEMRDropDownDeleted;	

	// (c.haag 2011-03-16) - PLID 42813 - We must preserve the arbitrary value here so that Common List saving
	// can use it when saving items that correspond to newly created table rows.
	CString m_strArbitraryGeneratedXmlValue;

	// (j.jones 2012-09-19 12:22) - PLID 52316 - added ParentLabelPtr,
	// a peide pointer only used in loading/saving
	CEmrInfoDataElement *m_peideParentLabelPtr;

	CString m_strKeywordOverride; // j.gruber 7/10/2014 - PLID 62621 
	BOOL m_bUseKeyword;// j.gruber 7/10/2014 - PLID 62621 

	BOOL m_bUseNameForKeyword; // (j.gruber 2014-12-05 15:37) - PLID 64361 - Load the new values for UseNameForKeyword

public:

	// (z.manning 2011-05-26 16:25) - PLID 43865
	BOOL IsCalculated()
	{
		if(!m_strFormula.IsEmpty() && (m_nFlags & edfFormulaForTransform) == 0) {
			return TRUE;
		}
		return FALSE;
	}

	CString GenerateDropDownXml(IN OUT long &nActionIDIncrement)
	{
		CString str;
		// (z.manning 2009-02-12 10:17) - PLID 33029 - Dropdown items can now have actions so we must
		// pass an ID increment value for the action xml.
		// (a.walling 2014-06-30 10:21) - PLID 62497 - Use generic iteration
		for (const auto& pDropDown : m_arypEMRDropDownList) {
			g_nLastTableDropDownIndex++;
			str += pDropDown->GenerateXml(g_nLastTableDropDownIndex, nActionIDIncrement);
		}

		return str;
	}

	//Only used for new elements
	//DRT 1/17/2007 - PLID 24181 - I added the arbitrary ActionID value parameter.  Since it needs to be
	//	an INOUT type parameter that sends its value back, I cannot give it a default value.  The strArbitraryValue was
	//	never used in Practice without specifying the value of it, so I removed that default as well to keep things consistent.  If
	//	you wish to not use the strArbitraryValue field, just pass "NULL".  You have no choice in the ActionID field.  Note that the
	//	nArbitraryActionIDValue will be changed here and passed back so that multiple actions per list item can be handled.
	CString GenerateXml(LPCTSTR strArbitraryValue, long &nArbitraryActionIDValue)
	{
		// (c.haag 2011-03-16) - PLID 42813 - We must preserve the arbitrary value here so that Common List saving
		// can use it when saving items that correspond to newly created table rows.
		if (m_strArbitraryGeneratedXmlValue.IsEmpty()) 
		{
			// Currently not assigned
			m_strArbitraryGeneratedXmlValue = strArbitraryValue;
		}
		else if (m_strArbitraryGeneratedXmlValue.Compare(strArbitraryValue))
		{
			// Already assigned and different!
			ThrowNxException("GenerateXml called on a CEmrInfoDataElement with two different arbitrary values!");
		}

		//DRT 1/17/2008 - PLID 28602 - Moved the action generation to its own function.
		CString strActionXml = GenerateActionXml(&m_arActions, nArbitraryActionIDValue);

		CString strAutoNumberType = m_nAutoNumberType == etantInvalid ? "" : FormatString("AutoNumberType=\"%li\" ", m_nAutoNumberType);

		//TES 3/11/2011 - PLID 42757 - Added Glasses Order data
		CString strGlassesOrderDataType = m_GlassesOrderDataType == godtInvalid ? "" : FormatString("GlassesOrderDataType=\"%i\" ", m_GlassesOrderDataType);
		CString strGlassesOrderDataID = m_nGlassesOrderDataID == -1 ? "" : FormatString("GlassesOrderDataID=\"%li\" ", m_nGlassesOrderDataID);

		// (r.gonet 08/03/2012) - PLID 51735 - Added Wound Care Data Type
		CString strWoundCareDataType = m_ewccWoundCareDataType == wcdtNone ? "" : FormatString("WoundCareDataType=\"%li\" ", (long)m_ewccWoundCareDataType);

		// (z.manning 2011-04-13 15:11) - PLID 42722 - If this is text column we need to force auto-alphabatize dropdown to on.
		if(m_nListType == LIST_TYPE_TEXT) {
			m_bAutoAlphabetizeDropDown = TRUE;
		}

		// (z.manning, 05/22/2008) - PLID 16443 - Added Formula and DecimalPlaces
		// (z.manning 2009-01-15 14:57) - PLID 32724 - Added InputMask
		// (z.manning 2010-02-11 17:06) - PLID 37324 - Added ListSubType
		// (c.haag 2010-02-23 17:49) - PLID 37488 - Added AutoAlphabetizeDropDown
		// (j.gruber 2010-04-27 08:59) - PLID 38336 - BOLD Code
		// (z.manning 2010-08-11 13:23) - PLID 40074 - AutoNumber fields
		// (j.jones 2011-03-08 11:48) - PLID 42282 - added EMCodeCategoryID
		//TES 3/11/2011 - PLID 42757 - Added Glasses Order data
		// (z.manning 2011-03-21 09:43) - PLID 23662 - Added autofill type
		// (z.manning 2011-05-26 14:50) - PLID 43865 - Added flags
		// (z.manning 2011-09-19 13:22) - PLID 41954 - Dropdown separators
		// (z.manning 2011-11-07 10:40) - PLID 46309 - SpawnedItemsSeparator
		// (r.gonet 08/03/2012) - PLID 51735 - Added WoundCareDataType
		// (j.gruber 2013-09-25 12:21) - PLID 58754 - added data groupID
		// (j.gruber 2014-07-18 14:14) - PLID 62624 - Keyword
		// (j.gruber 2014-12-05 16:05) - PLID 64289 - UseNameForKeyword  - Saving
		return FormatString(
			"<D %s%s%sID=\"%li\" Data=\"%s\" SortOrder=\"%li\" Default=\"%li\" Inactive=\"%li\" ListType=\"%li\" IsGrouped=\"%li\" IsLabel=\"%li\" UseEMCoding=\"%li\" EMCodeCategoryID=\"%li\" Formula=\"%s\" DecimalPlaces=\"%li\" InputMask=\"%s\" ListSubType=\"%li\" AutoAlphabetizeDropDown=\"%li\" BOLDCode=\"%s\" %sAutoNumberPrefix=\"%s\" %s %s AutofillType=\"%li\" DataFlags=\"%li\" WoundCareDataType=\"%li\" DataGroupID=\"%li\" UseKeyword=\"%li\" KeywordOverride=\"%s\" UseNameForKeyword=\"%li\" >\r\n"
			"<LongForm>%s</LongForm>\r\n" // (a.walling 2011-07-01 16:14) - PLID 36090 - CDATA must be in an element, not an attribute
			"<DropdownSeparator>%s</DropdownSeparator> <DropdownSeparatorFinal>%s</DropdownSeparatorFinal> <SpawnedItemsSeparator>%s</SpawnedItemsSeparator>\r\n"
			"%s\r\n" // dropdown
			"%s\r\n" // action
			"%s\r\n" //Codes
			"</D>\r\n", 
			strArbitraryValue ? "ArbVal =\"" : "", 
			strArbitraryValue ? ConvertToQuotableXMLString(strArbitraryValue) : "", 
			strArbitraryValue ? "\" " : "", 
			m_nID, ConvertToQuotableXMLString(m_strData), m_nSortOrder, m_bDefault ? 1 : 0, m_bInactive ? 1 : 0, m_nListType, m_bIsGrouped ?  1 : 0, 			
			m_bIsLabel ? 1 : 0, m_bUseEMCoding ? 1 : 0, m_nEMCodeCategoryID, ConvertToQuotableXMLString(m_strFormula), m_nDecimalPlaces, ConvertToQuotableXMLString(m_strInputMask), m_nListSubType, m_bAutoAlphabetizeDropDown ? 1 : 0, ConvertToQuotableXMLString(m_strBoldCode), strAutoNumberType, ConvertToQuotableXMLString(m_strAutoNumberPrefix), strGlassesOrderDataType, strGlassesOrderDataID, m_eAutofillType, m_nFlags, m_ewccWoundCareDataType, m_nDataGroupID, m_bUseKeyword ? 1 : 0, ConvertToQuotableXMLString(m_strKeywordOverride), m_bUseNameForKeyword ? 1:0,
			ConvertToQuotableXMLCDATA(m_strLongForm),  // (a.walling 2011-07-01 16:14) - PLID 36090 - converts to a CDATA string for XML
			ConvertToQuotableXMLCDATA(m_strDropdownSeparator), ConvertToQuotableXMLCDATA(m_strDropdownSeparatorFinal), ConvertToQuotableXMLCDATA(m_strSpawnedItemsSeparator), 
			GenerateDropDownXml(nArbitraryActionIDValue),
			strActionXml, m_aryCodes.GenerateXML("DCD"));
	};
};

// CEmrInfoDataElementArray is an array of pointers to CEmrInfoDataElement objects; automatically frees the objects on destruction

class CEmrInfoDataElementArray : public CArray<CEmrInfoDataElement *, CEmrInfoDataElement *> 
{
public:
	~CEmrInfoDataElementArray();

public:
	/*
	CEmrInfoDataElement *RemoveAt(long nIndex)
	{
		CEmrInfoDataElement *pAns = GetAt(nIndex);
		RemoveAt(nIndex, 1);
		return pAns;
	}
	*/
	// (a.walling 2010-03-09 14:13) - PLID 37640 - Moved to cpp
	void RemoveAllAndFreeEntries();
public:
	long FindDataElement(long nSearchForID, long nStartSearchAtIndex) const;
	long FindDataElement(CString strData) const;
	void AppendCopy(const CEmrInfoDataElementArray &aryeideCopyFrom);

	// (z.manning 2010-02-12 09:33) - PLID 37320
	BOOL DoesElementExist(const long nListType, const BYTE nListSubType);

	// (z.manning 2010-02-12 11:03) - PLID 37320
	void EnsureSubTypeDoesNotExist(const BYTE nSubType);

protected:
	void RemoveAll(); // Intentionally not implemented because it doesn't deallocate the memory.  Call RemoveAllAndFreeEntries() instead.	
};

// CAdminCurrentListHelpDlg - A simple modeless text dlg that explains basic purpose of current list vs. admin list
// (c.haag 2006-07-05 16:49) - PLID 19862 - We no longer let the user edit the current list
/*class CAdminCurrentListHelpDlg : public CDialog
{
public:
	BOOL Create(UINT nIDTemplate, CWnd* pParentWnd)
	{
		BOOL bCreate =  CDialog::Create(nIDTemplate, pParentWnd);
		ShowWindow(SW_HIDE);
		return bCreate;
	}
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
	{		
		if(nCtlColor == CTLCOLOR_STATIC) {
			pDC->SetBkMode(TRANSPARENT);
			return (HBRUSH)GetStockObject(WHITE_BRUSH);
		}
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	void OnOK()
	{
		ShowWindow(SW_HIDE);
	}
	void OnCancel()
	{
		ShowWindow(SW_HIDE);
	}
	DECLARE_MESSAGE_MAP()
};
//////////////////////////*/



/////////////////////////////////////////////////////////////////////////////
// CEmrItemEntryDlg dialog

// (a.walling 2008-02-07 10:55) - PLID 14982 - Added bSelectedOnTopic, only useful for editing popped up items
struct CurrentListItem {
	CurrentListItem() {
		bSelected = FALSE;
		bSelectedOnTopic = FALSE;
	}

	CString strData;
	BOOL bSelected;
	BOOL bSelectedOnTopic;
};

class CEmrItemEntryDlg : public CNxDialog
{
	// (b.cardillo 2007-01-11 11:24) - PLID 17929 - The advanced spell-check of this dialog is handled by a 
	// separate class, and that class needs to reference us so we declare it as a friend here.
	friend class CEmrItemEntryDlgSpellCheckHandler;

	//TES 3/14/2011 - PLID 42784 - Converted to datalist2.  Therefore, the spell checker now maintains a pointer to the row it's on,
	// rather than an index, which means we need to not touch the datalist while a spell check is going on.  These variables tell us 
	// when a spell check is happening, and, when it's complete, whether it made a change so that we can update the screen accordingly.
	bool m_bSpellCheckInProgress;
	bool m_bSpellCheckMadeChange;

// Construction
public:
	CEmrItemEntryDlg(CWnd* pParent);   // standard constructor
	~CEmrItemEntryDlg();
	CArray<CurrentListItem,CurrentListItem&> m_aryCurrentList;	// Stores the "Current List" data if applicable.  This list must
									    // be populated prior to this dialog's initialization in order
									    // for the "Admin/Current List tabs to be available.
	BOOL m_bMaintainCurrentList; // TRUE if we must maintain both current and admin lists for single & multi selects

	//determines if we are editing on the fly from a patient EMN or a template
	BOOL m_bIsEditingOnEMN;
	BOOL m_bIsEditingOnTemplate;

	// (a.walling 2008-01-18 13:18) - PLID 14982 - Flag to prevent changes to item type
	BOOL m_bPreventTypeChange;

	// (j.jones 2010-02-17 11:55) - PLID 37318 - prevents changing the linked smart stamp table
	BOOL m_bPreventSmartStampTableChange;

	CArray<CEMNDetail*,CEMNDetail*> m_apCurrentTableDetails; // If this is a table item, and we are editing this item from an EMR, this
															// array contains pointers to all of the details that have the same EmrInfoID
									// right-clicked on a detail and selected "Edit..." If there are 10 details, then there
									// will be 10 elements in the array.
	BOOL m_bMaintainCurrentTable; // TRUE if we must maintain the current table so that we can stop the user from changing
									// a column type if data exists for the current detail.

	BOOL m_bIsCurrentDetailTemplate; // TRUE if the detail we are maintaining is on a template

	//DRT 2/26/2008 - PLID 28603 - Maintain an array of all images if we came into this from editing on the fly.
	//true if we want to maintain the current image, not let them delete spots that are in use, etc.
	bool m_bMaintainCurrentImage;
	//Similar to m_apCurrentTableDetails above, but for images.  Used to keep the user from deleting hotspots that are
	//	already selected.
	CArray<CEMNDetail*, CEMNDetail*> m_aryCurrentImageDetails;

	// (z.manning 2010-02-09 15:50) - PLID 37228 - Allows caller to set the default data type
	EmrInfoType m_eitDefaultDataType;

	// (z.manning 2010-02-10 10:47) - PLID 37228 - True if the current item is a table tied
	// to a smart stamp image.
	BOOL m_bIsSmartStampTable;

	// (j.jones 2010-02-26 08:33) - PLID 37231 - we need to track the "remember" settings
	// for the SmartStamp Image parent, as the table cannot be different
	BOOL m_bSmartStampImage_RememberForPatient;
	BOOL m_bSmartStampImage_RememberForEMR;

	// (z.manning 2010-07-26 14:40) - PLID 39848 - These are no longer used
	//TES 2/18/2010 - PLID 37234 - Added a checkbox if we're using the special "Smart Stamp Format" for sentences.
	//BOOL m_bUseSmartStampFormat;
	//TES 2/18/2010 - PLID 37234 - Added a new sentence format used only be Smart Stamps
	//CString m_strSmartStampFormat;

	BOOL m_bWarnBold; // (j.gruber 2010-06-02 15:21) - PLID 38336
	// (j.jones 2008-09-22 13:02) - PLID 31476 - added m_checkRememberPerEMR
// Dialog Data
	//{{AFX_DATA(CEmrItemEntryDlg)
	enum { IDD = IDD_EMR_ITEM_ENTRY_DLG };
	NxButton	m_checkRememberPerEMR;
	NxButton	m_btnHideTitle;
	NxButton	m_btnHideItem;
	NxButton	m_btnSelText;
	NxButton	m_btnSelList;
	NxButton	m_btnSelMultiList;
	NxButton	m_btnSelImage;
	NxButton	m_btnSelTable;
	NxButton	m_btnSelSlider;
	NxButton	m_btnSelNarrative;
	NxButton	m_btnFmtText;
	NxButton	m_btnFmtList;
	NxButton	m_btnFmtBullet;
	NxButton	m_btnFmtNum;
	NxButton	m_checkRememberPerPatient;
	NxButton	m_btnOnePerEMN;
	NxButton	m_btnDisableTableBorder;
	NxButton	m_btnUseEMCoding;
	NxButton	m_btnInactiveElements;
	NxButton	m_btnAlphabetizeList;
	NxButton	m_btnRowsAsFields;
	NxButton	m_btnInactiveInfo;
	NxButton	m_btnPromptImage;
	NxButton	m_btnSelectedImage;
	NxButton	m_btnSelected3DImage;
	CNxIconButton	m_btnInsertSentenceField;
	// (j.jones 2013-01-28 11:57) - PLID 54731 - renamed Add Product to Add Other
	CNxIconButton	m_btnAddOther;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnEditEMCategories;
	CNxIconButton	m_btnEditCategories;
	CNxIconButton	m_btnEditDropdownContents;
	CNxIconButton	m_btnPreviewTable;
	CNxIconButton	m_btnDeleteDataItem;
	CNxIconButton	m_btnAddMultiDataItem;
	CNxIconButton	m_btnAddDataItem;
	CNxIconButton	m_btnInsertField;
	CNxIconButton	m_btnAction;
	CNxIconButton	m_ctrlModifyHotSpots;
	CNxIconButton	m_btnEditEmrDataCodes;
	// (d.thompson 2012-10-16) - PLID 53184
	CNxIconButton	m_btnPasteRows;
	CNxIconButton	m_btnPasteCols;
	CNxColor	m_nxclrLeft;
	CNxColor	m_nxclrRightTop;
	CNxColor	m_nxclrRightBottom;
	CNxEdit	m_nxeditItemName;
	CNxEdit	m_nxeditSentence;
	CNxEdit	m_nxeditDataFormatSeparator;
	CNxEdit	m_nxeditDataFormatSeparatorFinal;
	CNxEdit	m_nxeditDefaultText;
	CNxEdit	m_nxeditDataLongform;
	CNxEdit	m_nxeditEmrMinimum;
	CNxEdit	m_nxeditEmrIncrement;
	CNxEdit	m_nxeditEmrMaximum;
	CNxEdit		m_nxeditDataUnit;
	CNxStatic	m_nxstaticEmrCategoryCaption;
	CNxStatic	m_nxstaticDataTypeLabel;
	CNxStatic	m_nxstaticSentenceFormatCaption;
	CNxStatic	m_nxstaticDataFormatLabel;
	CNxStatic	m_nxstaticDataFormatSeparatorLabel;
	CNxStatic	m_nxstaticDataFormatSeparatorFinalLabel;
	CNxStatic	m_nxstaticDataLabel;
	CNxStatic	m_nxstaticDefaultTextCaption;
	CNxStatic	m_nxstaticRowLabel;
	CNxStatic	m_nxstaticColumnLabel;
	CNxStatic	m_nxstaticDataLongformLabel;
	CNxStatic	m_nxstaticEmrEmCategoryCaption;
	CNxStatic	m_nxstaticEmrEmCodeTypeCaption;
	CNxStatic	m_nxstaticEmrMinLabel;
	CNxStatic	m_nxstaticEmrIncLabel;
	CNxStatic	m_nxstaticEmrMaxLabel;
	CNxStatic	m_nxstaticEmrImageFrame;
	CNxStatic	m_nxstaticDataCodeLabel;
	CNxStatic	m_nxstaticDataUnitLabel;
	CNxIconButton	m_btnEditTableCalculatedField;
	CNxIconButton	m_btnEditTableAutofill;
	NxButton	m_btnEnableSmartStamps;
	CNxIconButton	m_btnEditSmartStampTable;
	CNxIconButton	m_btnNewSmartStampTable;
	NxButton m_btnAssociateWithGlassesOrder;
	CNxIconButton m_btnEditCommonLists;
	CNxIconButton m_btnSelectStamps;
	NxButton m_btnAssociateWithGlassesOrderOrCL;
	CNxLabel m_nxlGOCLToggle;
	// (r.gonet 08/03/2012) - PLID 51735
	NxButton m_btnUseWithWoundCareCoding;
	// (j.gruber 2013-12-11 13:16) - PLID 58816 - button to open table cell codes dialog
	CNxIconButton m_btnTableCellCodes;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrItemEntryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	//TES 12/24/2006 - PLID 23833 - I want this dialog to be called with the EmrInfoMasterT.ID, not the EmrInfoT.ID.  Since I'm 
	// changing the meaning of this function, I wanted to also change the name, to make sure I caught everywhere that needed changing.
	// (c.haag 2008-01-22 09:24) - PLID 28686 - We now allow the caller to define the dialog behavior
	// (see the definition of the enumeration for details)
	//virtual int DoModal(long nID);
	int OpenWithMasterID(long nInfoMasterID, EEmrItemEntryDlgBehavior behavior = eEmrItemEntryDlgBehavior_Normal, LPVOID pBehaviorData = NULL);
	
// For getting properties back out after the user has clicked OK
public:
	long GetID();
	long GetInfoMasterID();
	CString GetName();
	long GetDataType();
	// (a.walling 2007-03-12 09:42) - PLID 19884
	BOOL GetInactive(); // Return whether this item is inactive

	BOOL m_bListDataWasAdded; // TRUE if any new data was added to a single/mutli select list
	BOOL m_bWillAddToTopic;	// TRUE if we are adding this item to a topic immediately after creating it

	//returns m_pChangedIDMap
	EMRInfoChangedIDMap* GetChangedIDMap();

	// (z.manning 2011-04-28 14:58) - PLID 37604 - This is now a detail pointer instead of a detail ID
	CEMNDetail *m_pCalledFromDetail; //used to determine if we are editing an EMN detail

protected:
	long GetCalledFromDetailID(); // (z.manning 2011-04-28 15:03) - PLID 37604

// Implementation
protected:
	// The official ID that this item is represented by in data (EMRInfoT.ID)
	long m_nID;

	// (a.walling 2009-07-01 15:59) - PLID 34759 - Keep track of the 'original' info ID before reassigned
	long m_nOriginalInfoID;

	//TES 12/6/2006 - PLID 23724 - The corresponding EmrInfoMasterID
	//NOTE: This ID is now the "official" ID for this dialog, although ultimately the EmrInfoT.ID and EmrInfoMasterT.ID fields 
	// can be calculated from each other.
	long m_nEmrInfoMasterID;

	// Given a valid m_nID, this pulls the properties out of the database, places the values into 
	// variables, and then reflects the variables on screen.
	void Load();
	// Takes the local state member variables and fills the on-screen controls based on them.
	void ReflectState();

	// (z.manning 2011-07-22 10:04) - PLID 44676
	BOOL LoadCurrent3DImage();

	// (j.gruber 2007-08-09 14:39) - PLID 26973 - determines whether a medication is in use
	BOOL IsMedicationInUse(long nMedicationID, CString &strCounts);

	//when copying a locked info item, we will store our new IDs in member arrays,
	//but not apply them to this dialog until the copy is committed.
	//The IDs are still needed to be read by the calling code.
	//All the information is stored in this pointer (the struct is defined in EmrUtils.h)
	EMRInfoChangedIDMap *m_pChangedIDMap;

// Each of these variables stores exactly one piece of information about the emr info item.
protected:
	CString m_strName; // the name of the emr item; raw.
	long m_nDataType; // 1=text; 2=single-sel; 3=multi-sel; 4=image; 5=slider; 6=narrative; 7=table;
	EmrInfoSubType m_DataSubType; // (c.haag 2007-01-29 15:22) -	PLID 24423 - 1=Current Medications
	BOOL m_bRememberForPatient; // TRUE/FALSE; translated to 1/0 for storage in the bit field.
	BOOL m_bRememberForEMR;	// (j.jones 2008-09-22 13:03) - PLID 31476 - added m_bRememberForEMR
	CString m_strLongForm; // the text for the merge field for this item; raw.
	long m_nDataFormat; // 0=text; 1=bulletlist; 2=numlist; raw.
	CString m_strDataSeparator; // separator used when m_nDataFormat is 0 (text); raw.
	CString m_strDataSeparatorFinal; // final separator used when m_nDataFormat is 0 (text); raw.	
	long m_nEmrDataCodeID; // (a.walling 2009-05-28 15:09) - PLID 34389
	CString m_strEmrDataCodeUnit; // (a.walling 2009-05-28 15:09) - PLID 34389
	BOOL m_bImageSpecified; // TRUE if this is an anatomical diagram, otherwise FALSE; translated.
	CString m_strImagePath; // Only used if m_bImageSpecified is TRUE; raw.
	CString m_strDefaultText; // The default text optionally used for text type items; raw.
	double m_dblSliderMin; // slider minimum value; raw.
	double m_dblSliderMax; // slider maximum value; raw.
	double m_dblSliderInc; // slider increment; raw.
	BOOL m_bOnePerEmn; // TRUE if this item appears once per emn	
	BOOL m_bAutoAlphabetizeListData; // TRUE if multi and single-select lists are auto alphabetized
	BOOL m_bInactive; // TRUE if the item is inactive
	BOOL m_bDisableTableBorder; // TRUE if the item is a table and the border should be disabled when merged
	// (a.walling 2008-06-30 14:48) - PLID 30570 - Preview Pane flags
	DWORD m_nPreviewFlags; // set of flags for preview pane behaviour
	BOOL m_bTableRowsAsFields; // (c.haag 2008-10-21 15:40) - PLID 31708
	long m_nInfoFlags; // (z.manning 2011-11-15 15:45) - PLID 46485

	EEmrItemEntryDlgBehavior m_Behavior; // (c.haag 2008-01-22 09:26) - PLID 28686 - Dialog behavior (see enumeration definition for details)
	LPVOID m_pBehaviorData; // (c.haag 2008-01-22 09:39) - Data related to the behavior (ex. when opening to edit a dropdown combo, this would refer to the column)

	// (j.jones 2007-08-14 16:58) - PLID 27052 - added E/M Category
	long m_nEMCodeCategoryID;
	// (j.jones 2011-03-08 11:48) - PLID 42282 - added m_eEMCodeUseTableCategories
	EMCodeUseTableCategories m_eEMCodeUseTableCategories;

	// (j.jones 2011-03-08 11:48) - PLID 42282 - If bShow is true,
	// the E/M category dropdown will show the options for configuring
	// categories per row or column. If false, these options will be removed
	void EnsureEMCodeUseTableCategoriesListOptions(BOOL bShow);

	// (j.jones 2007-08-14 16:58) - PLID 27053 - added E/M Code Type
	BOOL m_bUseEMCoding;
	EMCodingTypes m_emctEMCodingType;

	CDWordArray m_arynCategoryIDs; // Array of EMR category IDs that this emr info item belongs to; raw, but stored in a separate table.
	MFCArray<EmrAction> m_arActions; // Array of CEmrAction objects that are spawned when this emr info item is added to an EMN; complex raw, stored in a separate table.
	CEmrInfoDataElementArray m_aryDataElements; // Array of CEmrInfoDataElement objects associated with this emr info item; complex raw, stored in a separate table.
	CEmrInfoDataElementArray m_aryColumnDataElements; // Array of CEmrInfoDataElement column objects associated with this emr info item; complex raw, stored in a separate table.

	// (c.haag 2006-03-07 10:15) - PLID 19581 - An array of columns whose types were
	// changed. We need to retain these so that when we save the item and change the
	// column, we can clear out all the data elements for all existing tables on EMR's
	CArray<CEmrInfoDataElement*,CEmrInfoDataElement*> m_aryChangedCols;

	//TES 12/12/2006 - PLID 22321 - We were writing any actions that needed revoking directly to the EMN; however, it was possible
	// that actions would get put on the EMN, but then never actually deleted.  So now we store them internally until we're sure
	// that the action deletion has been committed to data, then copy the whole thing to the EMN (and then only if it's a template).
	MFCArray<EmrAction> m_arActionsToRevoke; // Actions that need to be revoked probably because they were deleted.

	//DRT 1/15/2008 - PLID 28602 - This is the original array of hotspots when the dialog was loaded.  This is updated when saved.
	CEMRHotSpotArray m_aryLastSavedHotSpots;

	BOOL m_bHasGlassesOrderData; //TES 3/11/2011 - PLID 42757
	GlassesOrderLens m_GlassesOrderLens; //TES 3/11/2011 - PLID 42757
	BOOL m_bHasContactLensData; //TES 4/6/2012 - PLID 49367
	BOOL m_bSavedHasContactLensData; //TES 4/6/2012 - PLID 49367

	// (r.gonet 08/03/2012) - PLID 51735 - Marks this table item as being able to be used with Wound Care Coding Calculation
	BOOL m_bUseWithWoundCareCoding;

	CEmrInfoCommonListCollection m_CommonListCollection; // (c.haag 2011-03-14) - PLID 42813 - Current collection of common lists
	CEmrInfoCommonListCollection m_SavedCommonListCollection; // (c.haag 2011-03-14) - PLID 42813 - Saved collection of common lists from data

	// (z.manning 2011-10-24 11:50) - PLID 46082 - Variables to store stamp exclusion data
	CEmrItemStampExclusions m_StampExclusions;
	CEmrItemStampExclusions m_SavedStampExclusions;

	// (j.gruber 2013-09-20 14:07) - PLID 58676
	CEMRTableCellCodes m_TableCellCodes;
	CEMRTableCellCodes m_SavedCellCodes;	
	

	//TES 6/27/2012 - PLID 51241 - Track the current data type, so that we can detect the case when the user clicks on the already-selected
	// radio button.
	long m_nCurrentlySelectedDataType;

// Some variables to access the on-screen controls
protected:
	// (c.haag 2006-07-06 09:36) - PLID 19862 - We no longer let the user edit the current list
	//CAdminCurrentListHelpDlg m_dlgAdminCurrentListHelp;
	NXDATALISTLib::_DNxDataListPtr m_pdlUsedOnTemplateList;
	CNxSliderCtrl m_Slider;
	CNxIconButton m_btnDataDown;
	CNxIconButton m_btnDataUp;
	CNxIconButton m_btnColumnDataDown;
	CNxIconButton m_btnColumnDataUp;
	CMirrorImageButton m_btnEmrImage;
	// (b.cardillo 2007-01-23 14:47) - PLID 24388 - Changed the spellcheck button to an NxIconButton so we could include the icon.
	CNxIconButton m_btnSpellCheck;

	
	// (a.walling 2009-05-28 13:32) - PLID 34389
	NXDATALIST2Lib::_DNxDataListPtr m_dlEmrDataCode;
	enum EDataCodeListColumns {
		lcID = 0,
		lcCode,
		lcDescription,
		lcDefaultUnit,
		lcVital,
	};

	//used when we are saving the current item as a new item
	//TES 12/7/2006 - This doesn't appear to be called anywhere.
	//void ResetItemToNewState();
	//used when we are saving the current item as a new item
	BOOL PrepareToReassignEmrInfoItem(long nOldInfoID, long &nNewInfoID);
	BOOL ReassignEmrInfoItem(long nOldInfoID, long nNewInfoID);

	// (c.haag 2008-08-18 11:26) - PLID 30724 - Percolates ID changes from PrepareToReassignEmrInfoItem (legacy code)
	void UpdateChangedInfoActionIDs();
	void UpdateChangedDataActionIDs();
	void UpdateChangedHotspotActionIDs();
	// (z.manning 2009-02-12 14:45) - PLID 33058
	void UpdateChangedDropdownActionIDs(EMRInfoChangedDropdown *pChangedDropdown, CEmrTableDropDownItem *pddi);

	// (c.haag 2008-08-18 11:50) - PLID 30724 - When PrepareToReassignEmrInfoItem is called, this function will percolate
	// the newly generated problem action ID's to the given action 
	void UpdateChangedProblemActionIDs(EmrAction& ea, const CArray<EMRInfoChangedProblemAction,EMRInfoChangedProblemAction&>& aChangedIDs);

	// (c.haag 2011-03-15) - PLID 42821 - Update the internal ID's of the custom lists and their details
	void UpdateChangedCommonListIDs();

	void ClearChangedIDMap();

	// (c.haag 2006-07-06 09:36) - PLID 19862 - We no longer let the user edit the current list
	//_DNxTabPtr m_pSelectListTab;
	enum ESelectListTabs {
		sltAdminList = 0,
		sltCurrentList = 1,
	};

	NXDATALISTLib::_DNxDataListPtr m_pdlCategoryCombo;
	enum ECategoryListSpecialRowValue {
		clsrvMultiCategory = -1,
		clsrvNoCategory = -2,
	};
	
	//TES 3/14/2011 - PLID 42784 - Converted to datalist2
	NXDATALIST2Lib::_DNxDataListPtr m_pdlDataElementList;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlColumnDataElementList;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlCurrentDataElementList;
	enum EDataElementListColumn {
		delcID = 0,
		delcData,		
		delcDefault,
		delcAction,
		delcUseKeyWord,// (j.gruber 7/10/2014) - plid 62621
		delcBOLDCode, // (j.gruber 2010-04-26 12:56) - PLID 38336
		delcSortOrder,
		delcInactive,
		delcIsLabel,
		// (j.jones 2012-09-18 16:33) - PLID 52316 - added ParentLabelID
		delcParentLabelID,
		// (j.jones 2007-08-15 11:50) - PLID 27053 - added UseEMCoding
		delcUseEMCoding,
		// (j.jones 2011-03-08 11:48) - PLID 42282 - added EMCodeCategoryID
		delcEMCodeCategoryID,
		//TES 3/11/2011 - PLID 42757 - Added Glasses Order data
		delcGlassesOrderDataType,
		delcGlassesOrderDataID,
		delcCodes, // (j.gruber 2013-10-02 11:36) - PLID 58674
		
	};
	enum EColumnDataElementListColumn {
		cdelcID = 0,
		cdelcData,
		cdelcType,
		cdelcUseKeyword, // (j.gruber 7/10/2014) - plid 62621
		cdelcSortOrder,
		cdelcIsGrouped,
		cdelcInactive,
		cdelcIsLabel, // (z.manning 2010-04-13 10:06) - PLID 29301
		// (j.jones 2007-08-15 11:50) - PLID 27053 - added UseEMCoding
		cdelcUseEMCoding,
		cdelcSubType, // (z.manning 2010-02-11 17:21) - PLID 37324
		// (j.jones 2011-03-08 11:48) - PLID 42282 - added EMCodeCategoryID
		cdelcEMCodeCategoryID,
		//TES 3/11/2011 - PLID 42757 - Added Glasses Order data
		cdelcGlassesOrderDataType,
		cdelcGlassesOrderDataID,
		// (r.gonet 08/03/2012) - PLID 51735 - Added Wound Care Data Type
		cdelcWoundCareDataType,
		cdelcCodes, // (j.gruber 2013-10-21 09:10) - PLID 59101 - added codes to columns and rows
		
	};

	RICHTEXTEDITORLib::_DRichTextEditorPtr m_RichEditCtrl;

	// (j.jones 2007-08-14 08:42) - PLID 27052 - added E/M Categories
	NXDATALIST2Lib::_DNxDataListPtr m_pdlEMCategoryCombo;

	// (z.manning 2011-07-21 17:28) - PLID 44676 - Added 3D control
	Nx3DLib::_DNx3DPtr m_p3DImage;

	// (j.jones 2007-08-14 11:52) - PLID 27053 - added E/M calculation options
	NXDATALIST2Lib::_DNxDataListPtr m_pdlEMCodeTypeCombo;
	void ReflectEMCodeInterface(); //enables/disables E/M controls, and rebuilds the E/M code type combo based on the data type
	CString GetEMCodeTypeName(EMCodingTypes emctCodeType); //returns the name associated with each code type
	void AddEMCodeRow(EMCodingTypes emctCodeType); //adds an E/M code type row to the code type combo

	BOOL NeedTableRowEMColumn(); //determine if we require the E/M column for a table row
	BOOL NeedTableColumnEMColumn(); //determine if we require the E/M column for a table column
	// (j.jones 2011-03-08 11:48) - PLID 42282 - supported E/M categories per item
	BOOL NeedTableRowEMCodeCategoryColumn();
	BOOL NeedTableColumnEMCodeCategoryColumn();

	// (j.jones 2011-03-08 11:48) - PLID 42282 - this will force the embedded E/M category
	// combos to requery for the row & column lists
	void RequeryTableEMCategoryComboColumns();

	//TES 3/11/2011 - PLID 42757
	NXDATALIST2Lib::_DNxDataListPtr m_pdlGlassesOrderLens;
	enum GlassesOrderLensColumn {
		golcCode = 0,
		golcDescription = 1,
	};
	//TES 3/11/2011 - PLID 42757 - Show/Hide Glasses-Order-related fields
	void ReflectGlassesOrderCheck();
	//TES 3/11/2011 - PLID 42757 - Update the format of a GlassesOrderDataID field based on the selected GlassesOrderDataType
	void ReflectGlassesOrderDataType(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bIsColumn);
	//TES 3/14/2011 - PLID 42757 - Update the format of a GlassesOrderDataType field based on the selected column type
	void ReflectColumnType(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (r.gonet 08/03/2012) - PLID 51735 - Reflects the WoundCareCoding checkbox by adding or hiding the WoundCareDataType column
	void ReflectUseWithWoundCareCodingCheck();

// These are member variables meant to store the CURRENT STATE of certain fields that can't be 
// completely stored in the on-screen controls.  These are VERY DIFFERENT from the corresponding 
// member variables above, as those store the ORIGINAL STATE.
protected:
	MFCArray<EmrAction> m_arCurActions; // Array of CEmrAction objects that are currently selected to be spawned by this emr info item
	CString m_strCurImagePath; // String that keeps track of the relative path of the currently selected image (if there is one)
	CDWordArray m_arynCurCategoryIDs; // Array of category IDs that are currently selected for this emr info item
	CEmrInfoDataElementArray m_aryCurDataElements; // Array of data elements currently showing in the list (this is the authority but the on-screen list is kept in sync with it)
	CEmrInfoDataElementArray m_aryCurColumnDataElements; // Array of data elements currently showing in the column list (this is the authority but the on-screen list is kept in sync with it)
	CEmrInfoDataElementArray m_aryCurDeletedDataElements; // Array of deleted data elements
	CEMN* m_pCurrentEMN; // The currently active EMR, if applicable.

public:
	// (z.manning 2009-02-10 15:02) - PLID 33026 - Added an accesor the current EMN
	CEMN* GetCurrentEMN();

	//DRT 1/15/2008 - PLID 28602 - These variables are used for interaction with the popup CEMRImageHotSpotSetupDlg.  Use these in conjunction with the
	//	m_aryLastSavedHotSpots to get a full picture of the current state of hotspots.
	CEMRHotSpotArray m_aryRemovedHotSpots;		//These had to come from m_aryLastSaved... and should not be deleted in memory.
	CEMRHotSpotArray m_aryChangedHotSpots;		//These had to come from m_aryLastSaved... and should not be deleted in memory.
	CEMRHotSpotArray m_aryNewHotSpots;			//These are new and will need cleaned up in memory.

	// (z.manning 2011-07-25 09:54) - PLID 44676
	void GetCurrentHotSpots(CEMRHotSpotArray *parypCurrentHotSpots);
	CEMRHotSpot* GetHotSpotFrom3DHotSpotID(const short n3DHotSpotID);
	void OpenAnatomicLocationEditorForHotSpot(CEMRHotSpot *pHotSpot);

// Some internal functions for user-interaction purposes (such as disabling controls and so forth when appropriate)
protected:
	void ReflectDataElementSelection(); // Called to show/hide or enable/disable controls surrounding the data element list
	
	// Helper function for ReflectState called to refill the data element datalist based on m_aryCurDataElements 
	// and m_aryCurDeletedDataElements; nSetSelDataElementID can be either a valid data element ID, or can be one 
	// of the special values described by the ESpecialDataElementIdentifier enum.
	void ReflectDataElementList(long nSetSelDataElementID);
	enum ESpecialDataElementIdentifier {
		sdeidFirstInList = -1, // If there are any elements after the refill, leave the first one selected
		sdeidClearSelection = -2, // Leave NO elements selected
		sdeidPriorSelection = -3, // Whatever element was selected before, try to leave that one selected (if the previous one is no longer available, set the selection to the next in the sort order)
	};

	// Given a sort order index this searches the m_aryCurDataElements array to find the matching entry, and returns the index of the entry
	long GetCurDataElementArrayIndex(long nSortOrderNumber, BOOL bUseColumnList);
	// (z.manning 2010-02-11 16:10) - PLID 37320 - Same as above but returns the actual data element
	CEmrInfoDataElement* GetCurDataElementBySortOrder(long nSortOrder, BOOL bUseColumnList);

	// Helper function to simply switch the sort orders of any two data elements (as referenced by their 
	// location in the on-screen datalist); this will result in the sort order values changing in the on-
	// screen datalist and in the member variable array m_aryCurDataElements, and also for the positions 
	// of the two rows to change in both the on-screen datalist and that same member variable array.
	//TES 3/14/2011 - PLID 42784 - Converted to datalist2
	void SwapDataElements(NXDATALIST2Lib::IRowSettingsPtr pRow1, NXDATALIST2Lib::IRowSettingsPtr pRow2);

	// (z.manning, 05/29/2008) - PLID 16443 - Returns the visible index (i.e. ignore inactive) of the
	// current row or column list or -1 of the specified sort order doesn't exist or is inactive.
	long GetCurVisibleIndex(const long nSortOrder, BOOL bColumnList);

	// (z.manning, 05/30/2008) - PLID 16443 - Will go through all columns and update the visible index
	// for each element.
	// (z.manning 2011-05-04 15:39) - PLID 43560 - Added bForceUpdate
	void UpdateVisibleIndices(BOOL bForceUpdate = FALSE);

	// (z.manning 2011-05-04 14:57) - PLID 43560 - Added a flag for whether or not the item has any formulas
	BOOL m_bHasFormulas;

	// (z.manning, 05/30/2008) - PLID 16443 - This function will go through all calculated field columns
	// and update references to each column in all calculated field formulas for the item.
	void UpdateFormulaReferences(CEmrInfoDataElement *peideIgnore = NULL);

	void OpenCalculatedFieldEditor(CEmrInfoDataElement *peide);

	// (z.manning 2009-01-13 15:11) - PLID 32719 - Added function to open the input mask editor
	void OpenInputMaskEditor(CEmrInfoDataElement *peide);

	BOOL AtLeastOneActiveRowAndColumn();

	//TES 3/14/2011 - PLID 42784 - Converted to datalist2
	NXDATALIST2Lib::IRowSettingsPtr CreateDatalistRowFromDataElement(const CEmrInfoDataElement *peide);

	BOOL m_bAddingMultipleDataElements;

	// (j.jones 2009-08-13 15:48) - PLID 31730 - track the sort order of the last multiply-added row
	long m_nLastMultiplyAddedItemSortOrder;

	// (j.jones 2013-01-29 08:51) - PLID 54731 - Added bSilent to not warn about duplicate names.
	// This now returns true if it succeeded, false if it didn't add due to being a duplicate name.
	BOOL AddDataElement(const CString &strDataText, BOOL bColumnList, BOOL bAddAction = FALSE, EmrActionObject eaoDestType = (EmrActionObject)eaoCpt, long nDestID = -1, BOOL bSilent = FALSE);
	// (z.manning 2010-02-12 10:44) - PLID 37320 - Added overload that takes CEmrInfoDataElement
	void AddDataElement(CEmrInfoDataElement *peide, BOOL bColumnList);

	// (z.manning 2010-02-12 12:39) - PLID 37320 - Function to increment the sort order by 1 for all
	// rows or cols with a sort order greater than or equal to nSortOrder.
	void IncrementSortOrderGreaterThanOrEqualTo(const long nSortOrder, BOOL bColumnList);

	//TES 3/14/2011 - PLID 42784 - Converted to datalist2
	void DoActivateDataElement(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void DoInactivateDataElement(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void DoActivateColumnDataElement(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void DoInactivateColumnDataElement(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// Returns true if this item can be set to one-per-emn. This will return false if there
	// are any existing EMN's or EMN templates which would automatically violate that rule.
	BOOL CanSetOnePerEmn();

	long FindInCurrentList(CString strData); // Returns the index of strData in the current list (-1 if not found)

	// (c.haag 2008-07-18 10:16) - PLID 30724 - Returns the string used when auditing new or deleted problem actions
	CString GetProblemActionAuditString(const EmrAction& ea, const EmrProblemAction& epa);

public:
	// Validate() checks each area of the screen and warns the user if anything is unacceptable.  If 
	// everything is saveable then it silently returns TRUE, otherwise gives the informative message 
	// to the user and returns FALSE.
	BOOL Validate();
	// Apply() determines what on screen is different from when it was loaded, and only saves those 
	// changes. Returns true on success
	BOOL Apply();
	// (z.manning 2010-02-12 12:17) - PLID 37320 - Function to combine the the validate and save logic
	BOOL ValidateSaveAndClose();

	// This function returns an array of the EmrDataID's and data for elements in the current list
	// It's designed to be used by other classes once this dlg has been closed and applied.
	void GetCurrentList(CArray<long,long>* naryIDs, CStringArray* straryData, CStringArray* straryLabels, CStringArray* straryLongForms, CArray<long,long>* naryActionsType, CArray<BOOL,BOOL>* naryInactive);
	CString GetLabelText();
	CString GetLongForm();
	long GetDataFormat();
	CString GetDataSeparator();
	CString GetDataSeparatorFinal();
	// (a.walling 2008-06-30 17:24) - PLID 30570
	inline DWORD GetPreviewFlags() {return m_nPreviewFlags;};
	void SetCurrentEMN(CEMN* pEMN);

	// (z.manning 2008-06-09 17:20) - PLID 30145 - Added public function to preview the table
	void PreviewTable(CEmrInfoDataElementArray *paryRows, CEmrInfoDataElementArray *paryColumns);

	// (z.manning 2010-03-22 14:13) - PLID 37228
	long GetSmartStampTableMasterID();

protected:
	int CalcDataTypeByCurSelection() const;

	// (c.haag 2007-03-05 10:02) - PLID 23943 - Added support for auditing
	BOOL AddStatementToSqlBatch_EmrActionChanges(IN OUT CString &strSqlBatch, long nSourceType, const CString &strSourceID, const MFCArray<EmrAction> &arOriginalActions, const MFCArray<EmrAction> &arCurrentActions, long &nAuditTransactionID);
	// (z.manning 2009-02-12 09:43) - PLID 33029 - Added a parameter for the original info data element
	BOOL AddStatementToSqlBatch_EmrTableDropDownChanges(IN OUT CString &strSqlBatch, const CString &strSourceID, const CEmrInfoDataElement &pOrigDataElement, const CEmrInfoDataElement &pDataElement, long &nAuditTransactionID);

	//TES 4/30/2008 - PLID 29748 - This function checks whether the given data ID is used by (checked on) any EMNs.  
	// It returns whether it's in use by the currently edited item, as well as whether it's in use on the current topic.
	void CheckDataItemUse(long nDataElementID, BOOL bIsColumn, OUT BOOL &bInUseByCurrent, OUT BOOL &bInUseByTopic);
	//TES 4/30/2008 - PLID 29748 - An override that additionally returns more information about how the item is used.
	void CheckDataItemUse(long nDataElementID, BOOL bIsColumn, OUT BOOL &bInUseByCurrent, OUT BOOL &bInUseByTopic,
		OUT long &nEMNCount, OUT long &nEMNMintCount, OUT CString &strDetailsUsed, OUT CString &strDetailsFilled);
	// (z.manning 2010-04-19 11:30) - PLID 29301 - Will check data to see if an EMR data item is in use
	BOOL CheckDataItemUseInDataForLabel(const long nDataID);

protected:

	void OnDeleteDataItem(BOOL bColumnList);

	// (j.jones 2009-08-13 09:52) - PLID 31730 - added ability to insert at a certain list position
	void OnAddDataItem(BOOL bColumnList, long nInsertWithSortOrder = -1);

	// (j.jones 2009-08-13 15:50) - PLID 31730 - added ability to insert at a certain list position
	void OnAddMultipleDataItem(BOOL bColumnList, long nInsertWithSortOrder = -1);
	void OnAddInvDataItem(BOOL bColumnList);
	// (j.jones 2013-01-28 12:17) - PLID 54731 - added ability to add medications
	void OnAddMedicationDataItem(BOOL bColumnList, BOOL bAddAsAction = FALSE);

	void ReflectDataType();

	// (z.manning 2011-11-17 17:10) - PLID 46485
	void UpdateDontSpawnRememberedValuesCheck();

protected:
	BOOL IsSystemCurrentMedicationsItem() const;
	// (c.haag 2007-04-03 09:05) - PLID 25468 - Return TRUE if this is the system
	// allergies item that we're editing
	BOOL IsSystemAllergiesItem() const;

protected:
	BOOL GetAutoAlphabetizeRowData() const;
	BOOL GetAutoAlphabetizeColumnData() const;

protected:
	// (c.haag 2007-03-06 12:04) - PLID 23943 - Calculates text used for auditing given a source type and source ID
	// (c.haag 2015-07-07) - PLID 65572 - We now allow the caller to pass in objects so that lookups are not required
	CString GetAuditSourceTypeText(EmrActionObject type, const CString& strSourceID,
		const CEmrInfoDataElement* peideSourceEmrDataItem = NULL , const CEmrTableDropDownItem* pddiSourceTableDropdownItem = NULL) const;
	// (c.haag 2007-03-06 12:04) - PLID 23943 - Calculates text used for auditing given a dest destination and destination ID
	CString GetAuditDestTypeText(EmrActionObject type, long nDestID) const;

	//DRT 2/18/2008 - PLID 28602
	bool CheckForAndPromptToClearHotSpots();

	// (a.walling 2008-10-22 12:15) - PLID 31794 - Returns SQL to update template detail preview flags
	CString UpdatePreviewFlagsOnTemplates(DWORD dwFlagsToAdd, DWORD dwFlagsToRemove);

protected:
	// (z.manning 2010-02-09 16:53) - PLID 37228 - Variables for the smart stamp feature
	BOOL m_bEnableSmartStamps;
	_variant_t m_varSmartStampTableMasterID;
	_variant_t m_varSmartStampTableMasterIDForRequeryFinished;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlSmartStampTable;
	enum ESmartStampTableColumns {
		sstcMasterID = 0,
		sstcName,
	};

	int OpenItemEntryDialogForSmartStampTable(long nEmrInfoMasterID, EEmrItemEntryDlgBehavior eBehavior = eEmrItemEntryDlgBehavior_Normal);

	// (j.jones 2010-02-26 09:43) - PLID 37231 - used to determine if we are allowed
	// to save a link to a SmartStamp Table, based on the 'remember' settings
	BOOL CompareSmartStampRememberSettings(long nSmartStampTableMasterID, CString &strImageText, CString &strTableText);

	// (z.manning 2010-07-29 09:49) - PLID 36150
	void EditDataElementSentenceFormat(CEmrInfoDataElement *peide, BOOL bIsColumnElement);

	// (z.manning 2010-08-11 12:08) - PLID 40074
	void EditAutoNumberSettings(CEmrInfoDataElement *peide);

	// (z.manning 2011-03-18 15:07) - PLID 23622
	void EditTableAutofillSettings();

	//TES 6/25/2011 - PLID 38230 - Moved the code for when the data type changes out of the message handler and into this function.
	void HandleDataTypeSelection();

	// (z.manning 2011-09-27 15:52) - PLID 44676
	void Reset3DImageRadioIfNecessary();
	// (d.thompson 2012-10-16) - PLID 53184
	void PasteSetupData(BOOL bInTableColumns);

protected:
	// (z.manning, 05/22/2008) - PLID 16443 - Added button handler for editing calculated field formula
	// (j.jones 2008-09-23 08:49) - PLID 31476 - added handlers for the "Remember This Item's Value" checkboxes
	// Generated message map functions
	//{{AFX_MSG(CEmrItemEntryDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDataTypeSelected();
	afx_msg void OnDataFormatSelected();
	afx_msg void OnImageTypeSelected();
	afx_msg void OnPromptForImage();
	afx_msg void OnInactivate();
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnEditEmrCategories();
	afx_msg void OnSelChangingEmrCategories(long FAR* nNewSel);
	afx_msg void OnSelChosenEmrCategories(long nRow);
	afx_msg void OnInsertEmrField();
	afx_msg void OnEmrImage();
	afx_msg void OnItemAction();
	afx_msg void OnShowInactiveElements();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnDataUp();
	afx_msg void OnDataDown();
	afx_msg void OnColumnDataUp();
	afx_msg void OnColumnDataDown();
	afx_msg void OnBtnDeleteDataItem();
	afx_msg void OnBtnAddDataItem();
	afx_msg void OnBtnAddMultipleDataItem();
	// (j.jones 2013-01-28 11:57) - PLID 54731 - renamed Add Product to Add Other
	afx_msg void OnBtnAddOtherDataItem();
	afx_msg void OnBtnPreviewTable();
	afx_msg void OnBtnEditEmrDataCodes();
	afx_msg void OnSelChosenListEmrDataCodes(LPDISPATCH lpRow);
	afx_msg void OnBtnEditTableDropdownData();
	afx_msg void OnBtnEditTableAutofill();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnKillfocusDataLongform();
	afx_msg void OnChangeDataLongform();
	afx_msg void OnInsertEmrDataField();
	afx_msg void OnSelectTabSelectList(short nNewTab, short nOldTab);
	afx_msg void OnAutoAlphabetizeListData();
	afx_msg void OnChangeColumnSortFinishedEmrCategories(short nOldSortCol, BOOL bOldSortAscending, short nNewSortCol, BOOL bNewSortAscending);
	afx_msg void OnKillfocusEmrIncrement();
	afx_msg void OnSpellCheckBtn();
	afx_msg void OnEditEmrEmCategories();
	afx_msg void OnSelChosenEmrEmCategories(LPDISPATCH lpRow);
	afx_msg void OnCheckUseForEmCoding();
	afx_msg void OnSelChosenEmrEmCodeType(LPDISPATCH lpRow);
	afx_msg void OnModifyHotspots();
	afx_msg void OnBtnEditTableCalculatedField();
	afx_msg void OnRememberForPatient();
	afx_msg void OnRememberForEmr();
	afx_msg void OnTableRowsAsFields();
	afx_msg void OnEnableSmartStamps(); // (z.manning 2010-02-09 13:38) - PLID 37228
	afx_msg void OnEditSmartStampTable(); // (z.manning 2010-02-09 13:38) - PLID 37228
	afx_msg void OnNewSmartStampTable(); // (z.manning 2010-02-09 13:38) - PLID 37228
	afx_msg void OnTimer(UINT nIDEvent);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:

	// (j.jones 2012-09-20 12:58) - PLID 52316 - will update the ParentLabelID column's
	// dropdown list with all current labels
	void RecalculateAndApplyParentLabelComboSql();
	// (j.jones 2012-09-21 11:11) - PLID 52316 - given a pointer that was formerly a valid
	// label, this function will find any rows using this label as its parent, and clear their parents
	void TryRemoveParentLabelReferences(CEmrInfoDataElement *peideLabelToRemove);

	// (a.wilson 2012-06-29 17:40) - PLID 51306 - function to check whether or not the dataid of the row or column are tied to graphing.
	CString IsDataIDAssignedInGraphing(long nDataID);
	// (a.wilson 2012-07-16 11:01) - PLID 49704 - function to check if the current item is assigned to a line in a graph.
	CString IsItemIDAssignedInGraphing(long nItemID);
	// (j.gruber 2009-11-18 14:29) - PLID 35945 - added functions
	void SelChangingListEmrdatacodes(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	// (j.gruber 2009-12-22 10:36) - PLID 35771 - changed the name of the function to include reports
	// (j.gruber 2010-01-18 10:09) - PLID 36929 - changed function name back
	BOOL CheckTOPSCodeAgainstType(CString strCode, CString strAdditionalMessage);
	void RevertToSavedDataType();
	BOOL IsTOPSCode(CString strCode);	
	// (j.gruber 2010-01-18 10:09) - PLID 36929 - removed report and thus codes
	//BOOL IsReportCode(CString strCode);	
	void RequeryFinishedSmartStampTableCombo(short nFlags);
	// (j.jones 2010-02-26 09:28) - PLID 37231 - added OnSelChosenSmartStampTableCombo
	void OnSelChosenSmartStampTableCombo(LPDISPATCH lpRow);
	// (z.manning 2010-03-02 08:23) - PLID 37320
	CString EnsureUniqueDataName(const CString &strDataName, BOOL bColumnList);
	// (j.gruber 2010-04-27 11:52) - PLID 38336 - needed for bold codes
	BOOL IsBOLDCode(CString strCode); // (j.gruber 2010-04-27 16:52) - PLID 38377
	BOOL CheckBOLDCodeAgainstType(CString strCode, CString strAdditionalMessage);

	// (j.gruber 2014-07-25 13:54) - PLID 62629 - Just checks if any columns have a checkbox type
	BOOL HasCheckAllType(); 	
	void Check_UncheckAllKeywords(BOOL bCheck, BOOL bIsColumnList);
	// (j.gruber 2014-08-06 10:21) - PLID 63188 - if you check the keyword column for a drop down and none of the underlying list elements are already checked, check them all.  
	void HandleKeywordDropDownContents(CEmrInfoDataElement *pElement, BOOL bChecked);
	BOOL CanDropDownListBeChanged(CEmrInfoDataElement *pElement, BOOL bChecked);

	// (c.haag 2015-07-07) - PLID 65572 - Audits all the EMR problem actions for this EMR item. Only used when the item is being created.
	void AuditNewItemEmrProblemActions(long& nAuditTransactionID, const CString& strName);
	// (c.haag 2015-07-07) - PLID 65572 - Audits all the EMR problem actions for a single EMR action. Only used when the item is being created.
	void AuditNewItemEmrProblemActions(long& nAuditTransactionID, const CString& strName,
		const CEmrInfoDataElement* peideSourceEmrDataItem, const CEmrTableDropDownItem* pddiSourceTableDropdownItem,
		const EmrAction& ea);

	afx_msg void OnBnClickedCancelEmrItemEntry(); // (z.manning 2010-05-07 09:50) - PLID 28759
	afx_msg void OnAssociateWithGlassesOrder();
	afx_msg void OnEditCommonLists();
	void OnSelChangedEmrDataList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void OnRButtonDownEmrDataList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnEditingFinishedEmrDataList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void OnLeftClickEmrDataList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnEditingFinishingEmrDataList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void EditingStartingEmrDataList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void OnEditingStartingEmrColumnList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void OnSelChangedEmrColumnList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void OnRButtonDownEmrColumnList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnEditingFinishingEmrColumnList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void OnEditingFinishedEmrColumnList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnBnClickedUseSelected3dimage();
	//TES 3/9/2012 - PLID 48779 - There's no HotspotRightClicked any more, we handle that in OnContextMenu() now
	afx_msg void OnBnClickedEmrItemSelectStamps(); // (z.manning 2011-10-24 11:28) - PLID 46082
	afx_msg void OnAssociateWithGlassesOrderOrCL();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	// (r.gonet 08/03/2012) - PLID 51735
	afx_msg void OnUseWithWoundCareCoding();
	void OnPasteSetupData();
	void OnPasteSetupDataColumns();

	// (j.gruber 2014-07-21 13:40) - PLID 62625 - KeywordOverride Handler
	// (j.gruber 2014-07-28 11:43) - PLID 62630 - Changed to use a row instead of an EMRDataElement
	void OnKeywordOverride(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bUseColumnList);
public:
	afx_msg void OnBnClickedBtnEditTableCellCodes();
	void LeftClickEmrColumnList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRITEMENTRYDLG_H__D36A114E_DDDB_413C_A90C_4D7A20B60997__INCLUDED_)
