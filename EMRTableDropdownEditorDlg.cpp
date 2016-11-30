// EMRTableDropdownEditorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "EMRTableDropdownEditorDlg.h"
#include "EMRSelectProductDlg.h"
#include "EMNDetail.h"
#include "GlobalDrawingUtils.h"
#include "EmrActionDlg.h"
#include "InvVisionWebUtils.h"
#include "EmrTableDropdownStampSetupDlg.h"
#include "ClipboardToStringArray.h"
#include <map>
#include <set>
#include <foreach.h>
#include "EMRSelectMedicationsDlg.h"
#include "EMRCodeEditorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.gruber 2014-07-30 10:18) - PLID 62630 - Keyword colors
#define HAS_KEYWORD_OVERRIDE_COLOR  RGB(255,255,137)
#define HAS_NO_KEYWORD_OVERRIDE_COLOR   RGB(255,255,255)

// (j.gruber 2014-08-04 14:48) - PLID 62632 - check for a keyword override
#define KEYWORD_OVERRIDE_WARNING "This data has a custom keyword override setup.  Please review to confirm it is still accurate."

// (c.haag 2008-02-20 15:28) - PLID 28686 - Timer event ID for automatically invoking
// the "Add New Item" function
#define	IDT_AUTO_ADD_NEW			1000

using namespace ADODB;

enum EDropDownListColumn {
	ddlcID = 0,
	ddlcData,
	ddlcSortOrder,
	ddlcAction, // (z.manning 2009-02-10 12:45) - PLID 33026
	ddlcUseKeyword, // (j.gruber 2014-07-22 12:52) - PLID 62627 - Keyword field
	ddlcStampFilter, // (z.manning 2011-09-28 10:40) - PLID 45729
	ddlcInactive,
	ddlcGlassesOrderDataID,	//TES 3/15/2011 - PLID 42757
	ddlcCodes, // (j.gruber 2013-09-30 13:34) - PLID 58675
};

using namespace NXDATALISTLib;

// (c.haag 2008-01-16 10:06) - PLID 17936 - Selections for individual cells of table dropdown
// columns are stored in EMRDetailTableDataT.Data (nvarchar(2000)) as a comma-delimited set of
// numbers that represent EMRTableDropdownInfoT.ID values.
//
// There are several places in this class that need to search EMRDetailTableDataT.Data for a
// specific ID. To do this, we use this utility function to hide all the abstraction of the logic.
// 
// (c.haag 2008-02-22 15:42) - PLID 17936 - Now applies to EMRTemplateTableDefaultsT as well

// This is the SQL version
#define EMRTABLE_DATA_CONTAINS_ELEMENT(field,element) \
	FormatString("( (%s = '%d') OR (%s LIKE '%%,%d') OR (%s LIKE '%d,%%') OR (%s LIKE '%%,%d,%%') )" \
	, field, element, field, element, field, element, field, element)

// This is the string version
static void ParseTableDataValueIntoArray(CString strData, CArray<long,long>& a)
{
	while (!strData.IsEmpty())	{
		long comma = strData.Find(',');

		// If we have no comma, we must have 1 more value in the string
		if(comma != -1) {
			a.Add(atol(strData.Left(comma)));
			strData = strData.Right(strData.GetLength() - strData.Left(comma+1).GetLength());
		} else {
			a.Add(atol(strData));
			strData = "";
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CEMRTableDropdownEditorDlg dialog


CEMRTableDropdownEditorDlg::CEMRTableDropdownEditorDlg(CEmrItemEntryDlg* pParent)
	: CNxDialog(CEMRTableDropdownEditorDlg::IDD, pParent)
	, m_nNextSortOrder(1)
{
	//{{AFX_DATA_INIT(CEMRTableDropdownEditorDlg)
		m_strColumnName = "";
		m_bAddingMultipleDataElements = FALSE;
		m_bAllowEdit = TRUE;
	//}}AFX_DATA_INIT
	m_bMaintainCurrentTable = FALSE;
	m_bIsCurrentDetailTemplate = FALSE;
	// (c.haag 2008-01-22 10:04) - PLID 28686 - By default, the user must press
	// "Add New" to add a new entry.
	m_bAutoAddNew = FALSE;
	m_pdlgEmrItemEntry = pParent; // (z.manning 2009-02-10 13:10) - PLID 33026
	m_bAutoAlphabetizeDropDown = FALSE; // (a.vengrofski 2010-02-22 16:47) - PLID <37524> - By defualt the order is manual.
	m_GlassesOrderDataType = godtInvalid; //TES 3/15/2011 - PLID 42757
	m_nColumnType = LIST_TYPE_DROPDOWN; // (z.manning 2011-03-17 09:18) - PLID 42722
}

CEMRTableDropdownEditorDlg::~CEMRTableDropdownEditorDlg()
{
	
}


void CEMRTableDropdownEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRTableDropdownEditorDlg)
	DDX_Control(pDX, IDC_DATA_DOWN, m_btnDataDown);
	DDX_Control(pDX, IDC_DATA_UP, m_btnDataUp);
	DDX_Control(pDX, IDC_DROPDOWN_LIST_LABEL, m_nxstaticDropdownListLabel);
	DDX_Control(pDX, IDC_ADD_DATA_ITEM, m_btnAddDataItem);
	DDX_Control(pDX, IDC_ADD_MULTIPLE_DATA_ITEM, m_btnAddMultipleDataItem);
	DDX_Control(pDX, IDC_ADD_OTHER_DATA_ITEM, m_btnAddOtherDataItem);
	DDX_Control(pDX, IDC_DELETE_DATA_ITEM, m_btnDeleteDataItem);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_AUTO_ALPHABETIZE_DROPDOWN_DATA, m_btnAutoAlphabetizeDropDown);
	DDX_Control(pDX, IDC_PASTE_SETUP_DATA, m_btnPasteData);
	DDX_Control(pDX, IDC_TABLE_DROPDOWN_LIST_SPELLCHECK, m_btnSpellCheck); 
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRTableDropdownEditorDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRTableDropdownEditorDlg)
	ON_BN_CLICKED(IDC_ADD_DATA_ITEM, OnAddDataItem)
	ON_BN_CLICKED(IDC_ADD_MULTIPLE_DATA_ITEM, OnAddMultipleDataItem)
	ON_BN_CLICKED(IDC_ADD_OTHER_DATA_ITEM, OnAddOtherDataItem)
	ON_BN_CLICKED(IDC_DELETE_DATA_ITEM, OnDeleteDataItem)
	ON_BN_CLICKED(IDC_DATA_UP, OnDataUp)
	ON_BN_CLICKED(IDC_DATA_DOWN, OnDataDown)
	ON_WM_CONTEXTMENU()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_AUTO_ALPHABETIZE_DROPDOWN_DATA, &CEMRTableDropdownEditorDlg::OnBnClickedAutoAlphabetizeDropdownData)
	ON_BN_CLICKED(IDC_PASTE_SETUP_DATA, OnBnClickedPasteSetupData)
	ON_BN_CLICKED(IDC_TABLE_DROPDOWN_LIST_SPELLCHECK, OnBnClickedSpellCheck)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRTableDropdownEditorDlg message handlers

BOOL CEMRTableDropdownEditorDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		m_btnDataUp.AutoSet(NXB_UP);
		m_btnDataDown.AutoSet(NXB_DOWN);
		// (c.haag 2008-04-29 17:38) - PLID 29840 - NxIconify more buttons
		m_btnAddDataItem.AutoSet(NXB_NEW);
		m_btnAddMultipleDataItem.AutoSet(NXB_NEW);
		m_btnAddOtherDataItem.AutoSet(NXB_NEW);
		m_btnDeleteDataItem.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		// (b.spivey, June 25, 2013) - PLID 37100 - use spell check icon. 
		m_btnSpellCheck.AutoSet(NXB_SPELLCHECK); 
		// (d.thompson 2012-10-16) - PLID 53217
		m_btnPasteData.SetIcon(IDI_PASTE);

		CString str;
		str.Format("Dropdown contents for column %s:",m_strColumnName);
		SetDlgItemText(IDC_DROPDOWN_LIST_LABEL, str);

		m_List = BindNxDataListCtrl(this, IDC_EMR_COLUMN_LIST, GetRemoteData(), false);

		// (z.manning 2011-03-17 09:50) - PLID 42722 - If this is a text type table column, hide the actions column.
		if(m_nColumnType == LIST_TYPE_TEXT) {
			IColumnSettingsPtr pActionCol = m_List->GetColumn(ddlcAction);
			// (z.manning 2011-03-17 10:18) - PLID 42722 - I update the stored width before making the column invisible
			// because the other columns won't update their widths otherwise (probably a minor bug in the datalist).
			pActionCol->PutStoredWidth(0);
			pActionCol->PutColumnStyle(0);
			IColumnSettingsPtr pGlassesOrderCol = m_List->GetColumn(ddlcGlassesOrderDataID);
			pGlassesOrderCol->PutStoredWidth(0);
			pGlassesOrderCol->PutColumnStyle(0);
			IColumnSettingsPtr pStampFilterCol = m_List->GetColumn(ddlcStampFilter);
			pStampFilterCol->PutStoredWidth(0);
			pStampFilterCol->PutColumnStyle(0);
			// (j.gruber 2013-09-30 13:35) - PLID 58675
			IColumnSettingsPtr pCodeCol = m_List->GetColumn(ddlcCodes);
			pCodeCol->PutStoredWidth(0);
			pCodeCol->PutColumnStyle(0);

			// (j.gruber 2014-07-22 12:52) - PLID 62627 - Keyword field
			IColumnSettingsPtr pKeywordCol = m_List->GetColumn(ddlcUseKeyword);
			pKeywordCol->PutStoredWidth(0);
			pKeywordCol->PutColumnStyle(0);


			// (z.manning 2011-03-17 11:35) - PLID 42722 - Let's also explain the purpose of editing dropdown 
			// contents for text columns.
			SetDlgItemText(IDC_DROPDOWN_EDIT_INFO,
				"* You are editing dropdown contents for a text type table item. These values can be used to quickly insert "
				"common values in a table, however, you can still type in whatever you want into these table cells.");
		}		
		else
		{
			// (z.manning 2011-03-17 09:59) - PLID 42722 - Do not do this if it's a text type column

			//TES 3/15/2011 - PLID 42757 - Set up the Glasses Order Field column based on our type.
			IColumnSettingsPtr pCol = m_List->GetColumn(ddlcGlassesOrderDataID);
			switch(m_GlassesOrderDataType) {
				case godtDesign:
					//TES 5/18/2011 - PLID 42757 - Sort the list
					pCol->ComboSource = _bstr_t("SELECT -1, '' AS DesignName UNION SELECT ID, DesignName FROM GlassesCatalogDesignsT ORDER BY DesignName ASC");
					break;
				case godtMaterial:
					//TES 5/18/2011 - PLID 42757 - Sort the list
					pCol->ComboSource = _bstr_t("SELECT -1, '' AS MaterialName UNION SELECT ID, MaterialName FROM GlassesCatalogMaterialsT ORDER BY MaterialName ASC");
					break;
				case godtTreatment:
					//TES 5/18/2011 - PLID 42757 - Sort the list
					pCol->ComboSource = _bstr_t("SELECT -1, '' AS TreatmentName UNION SELECT ID, TreatmentName FROM GlassesCatalogTreatmentsT ORDER BY TreatmentName ASC");
					break;
				case godtInvalid:
				case godtRxNumber: //TES 3/15/2011 - PLID 42757 - Rx Number is allowed for dropdown types, but then the field is set
									// at the EmrDataT level, not the EmrTableDropdownInfoT level
					pCol->ColumnStyle = csVisible|csFixedWidth;
					pCol->StoredWidth = 0;
					break;
				default:
					ASSERT(FALSE);
					pCol->ColumnStyle = csVisible|csFixedWidth;
					pCol->StoredWidth = 0;
					break;
			}
		}

		// (a.walling 2014-06-30 10:21) - PLID 62497
		for(const auto& ddi : m_arypEMRDropDownList) {
			// (d.thompson 2012-10-17) - PLID 53217 - Refactored out duplicated code, use the new shared version
			CreateDatalistRowFromDDI(ddi.get());

			// (a.walling 2012-08-02 16:14) - PLID 43277 - Calc the next greatest sort order
			if (ddi->nSortOrder >= m_nNextSortOrder) {
				m_nNextSortOrder = ddi->nSortOrder + 1;
			}
		}

		m_List->ReadOnly = !m_bAllowEdit;

		m_List->CurSel = -1;

		if(m_nColumnType == LIST_TYPE_TEXT) {
			// (z.manning 2011-04-11 14:07) - PLID 42722 - For now at least, we force auto-alphabatize on on text columns
			// because that's all that the auto-complete control that the datalist uses allows.
			GetDlgItem(IDC_AUTO_ALPHABETIZE_DROPDOWN_DATA)->EnableWindow(FALSE);
			CheckDlgButton(IDC_AUTO_ALPHABETIZE_DROPDOWN_DATA, BST_CHECKED);

			// (z.manning 2011-09-22 17:29) - PLID 41954 - Final separators aren't applicable for text dropdown columns.
			GetDlgItem(IDC_EMR_DROPDOWN_SEPARATOR_FINAL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EMR_DROPDOWN_SEPARATOR_FINAL_LABEL)->ShowWindow(SW_HIDE);
		}
		else {
			// (a.vengrofski 2010-02-22 16:52) - PLID <37524> - Set the check to be what it is coming in.
			CheckDlgButton(IDC_AUTO_ALPHABETIZE_DROPDOWN_DATA, m_bAutoAlphabetizeDropDown ?  BST_CHECKED : BST_UNCHECKED);
		}

		OnBnClickedAutoAlphabetizeDropdownData();

		// (z.manning 2011-09-19 14:05) - PLID 41954 - Dropdown separators
		((CEdit*)GetDlgItem(IDC_EMR_DROPDOWN_SEPARATOR))->SetLimitText(20);
		((CEdit*)GetDlgItem(IDC_EMR_DROPDOWN_SEPARATOR_FINAL))->SetLimitText(20);
		SetDlgItemText(IDC_EMR_DROPDOWN_SEPARATOR, m_strDropdownSeparator);
		SetDlgItemText(IDC_EMR_DROPDOWN_SEPARATOR_FINAL, m_strDropdownSeparatorFinal);

		OnSelChangedEmrColumnList(-1);

		// (c.haag 2008-01-22 10:05) - PLID 28686 - If m_bAutoAddNew is TRUE, we must
		// automatically add a new row for the user
		// (c.haag 2008-02-20 15:24) - We used to set the focus here and post a message.
		// This had the strange side effect of the [New Item] edit box appearing below where
		// it was supposed to every time if the list had more entries in it than were visible
		// on the screen. We now use a timer instead. Unfortunately, instead of eradicating 
		// the problem, it just makes it happen much less frequently.
		if (m_bAutoAddNew) {
			SetTimer(IDT_AUTO_ADD_NEW, 100, NULL);
		}

#ifdef _DEBUG
		{
			std::set<long> sortOrders;
			// (a.walling 2014-06-30 10:21) - PLID 62497
			for(const auto& pItem : m_arypEMRDropDownList) {
				_ASSERTE(!sortOrders.count(pItem->nSortOrder));
				sortOrders.insert(pItem->nSortOrder);
			}
		}
#endif
	}
	NxCatchAll("Error in CEMRTableDropdownEditorDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEMRTableDropdownEditorDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRTableDropdownEditorDlg)
	ON_EVENT(CEMRTableDropdownEditorDlg, IDC_EMR_COLUMN_LIST, 10 /* EditingFinished */, OnEditingFinishedEmrColumnList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEMRTableDropdownEditorDlg, IDC_EMR_COLUMN_LIST, 2 /* SelChanged */, OnSelChangedEmrColumnList, VTS_I4)
	ON_EVENT(CEMRTableDropdownEditorDlg, IDC_EMR_COLUMN_LIST, 9 /* EditingFinishing */, OnEditingFinishingEmrColumnList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEMRTableDropdownEditorDlg, IDC_EMR_COLUMN_LIST, 6 /* RButtonDown */, OnRButtonDownEmrColumnList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CEMRTableDropdownEditorDlg, IDC_EMR_COLUMN_LIST, 19, CEMRTableDropdownEditorDlg::LeftClickEmrColumnList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CEMRTableDropdownEditorDlg::OnEditingFinishedEmrColumnList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		if(nRow == -1)
			return;

		long nSortOrder = m_List->GetValue(nRow,ddlcSortOrder);

		if(nCol == ddlcData) {

			// Get the data element we're working with
			// (a.walling 2014-06-30 10:21) - PLID 62497
			auto it = GetCurDataElementArrayIndex(nSortOrder);

			CEmrTableDropDownItem *pDDI = it->get();

			// We're dealing with the delcData column, so see if this was a new entry (we'll use this later)
			BOOL bWasNewEntry;
			if (VarLong(m_List->GetValue(nRow, ddlcID)) == -2) {
				bWasNewEntry = TRUE;
			} else {
				bWasNewEntry = FALSE;
			}
			// See if the user committed or canceled
			if (bCommit && !(bWasNewEntry && AsString(varNewValue) == AsString(varOldValue))) {
				// The user committed a change to a data cell
				pDDI->strData = AsString(varNewValue);

				// And if it was a new entry we have to update the id, and possibly create another new entry
				if (bWasNewEntry) {
					// It is a new entry, and the user just committed it, so change its ID to be the official 
					// sentinel value that means it's set, rather than the one saying the user hasn't committed 
					// her decision to even create this item yet.
					m_List->PutValue(nRow, ddlcID, (long)-1);
					ASSERT(pDDI->nID == -1);
					// (a.walling 2014-06-30 10:21) - PLID 62497
					m_arypEMRDropDownList.UpdateElementID(pDDI, -1);

					// And see if we're in an add-multiple loop
					if (m_bAddingMultipleDataElements) {
						// Go again!
						OnAddDataItem();
					}

					// (a.vengrofski 2010-02-24 10:28) - PLID <37524> - Sort the list when we are done
					if (IsDlgButtonChecked(IDC_AUTO_ALPHABETIZE_DROPDOWN_DATA)) {
						m_List->Sort();
					}
				} else {
					// It wasn't a new entry, so we just do nothing because everything is taken care of by the 
					// user in combination with the datalist default behavior
					
					// (j.gruber 2014-08-04 14:48) - PLID 62632 - check for a keyword override
					if (VarString(varOldValue) != VarString(varNewValue))
					{
						if (pDDI->bUseKeyword && !pDDI->strKeywordOverride.IsEmpty())
						{
							MsgBox(KEYWORD_OVERRIDE_WARNING);
						}
					}
				}
			} else {
				// User canceled
				if (bWasNewEntry) {
					// It was a new entry and the user canceled it, so delete it
					
					// Remove the entry from the array
					m_arypEMRDropDownList.erase(it);

					// Remove the row from the datalist
					m_List->RemoveRow(nRow);

					// And if we were in an "add-multple" loop then get out of it (because the user canceled)
					m_bAddingMultipleDataElements = FALSE;

					// Reflect the correct state to the surrounding controls
					OnSelChangedEmrColumnList(m_List->CurSel);

					// (a.vengrofski 2010-02-24 10:28) - PLID <37524> - Sort the list when we are done
					if (IsDlgButtonChecked(IDC_AUTO_ALPHABETIZE_DROPDOWN_DATA)) {
						m_List->Sort();
					}

				} else {
					// Wasn't a new entry so the cancel is already taken care of by the user in combination with 
					// the datalist default bahavior
				}
			}			
		}
		else if(nCol == ddlcInactive) {
			GetCurDataElementArrayIndex(nSortOrder)->get()->bInactive = VarBool(varNewValue, FALSE);
		}
		else if(nCol == ddlcGlassesOrderDataID) {
			//TES 3/15/2011 - PLID 42757 - Added GlassesOrderDataID
			//TES 5/17/2011 - PLID 42757 - Respect bCommit!
			if(bCommit) {
				long nNewID = VarLong(varNewValue,-1);
				GetCurDataElementArrayIndex(nSortOrder)->get()->nGlassesOrderDataID = nNewID;
				GetCurDataElementArrayIndex(nSortOrder)->get()->strGlassesOrderDataName = GetGlassesOrderRecordDescription(m_GlassesOrderDataType, nNewID, false);
			}
		}
		else if (nCol == ddlcUseKeyword)
		{
			if (bCommit)
			{
				GetCurDataElementArrayIndex(nSortOrder)->get()->bUseKeyword = VarBool(varNewValue, FALSE);				

				// (j.gruber 2014-07-30 13:17) - PLID 62630 - do we already have a keyword override
				IRowSettingsPtr pRow = m_List->GetRow(nRow);
				if (GetCurDataElementArrayIndex(nSortOrder)->get()->bUseKeyword && !GetCurDataElementArrayIndex(nSortOrder)->get()->strKeywordOverride.IsEmpty())
				{					
					pRow->PutCellBackColor(ddlcUseKeyword, HAS_KEYWORD_OVERRIDE_COLOR);
				}
				else {
					pRow->PutCellBackColor(ddlcUseKeyword, HAS_NO_KEYWORD_OVERRIDE_COLOR);				
				}
			}
		}

	}NxCatchAll("Error updating list.");	
}

// (d.thompson 2012-10-17) - PLID 53217 - Refactored this code out for re-use
//This function will generate a new CEmrTableDropDownItem record and add it to the
//	member array automatically.  This function will maintain all defaults, and callers
//	can choose to change them afterwards if necessary.
//Note:  This will automatically increment the sort order
//Parameters:
//	strDataName:		What text name you would like the new element to have
//Return value:
//	A pointer to the newly generated CEmrTableDropDownItem.  Use this to make any modifications to the
//	default values for this element as needed.
CEmrTableDropDownItem* CEMRTableDropdownEditorDlg::GenerateNewDDI(CString strDataName)
{
	// (a.walling 2012-08-02 16:14) - PLID 43277 - Always add new items with the next greatest sort order
	long nSortOrder = m_nNextSortOrder++;

	CEmrTableDropDownItem *pDDI = new CEmrTableDropDownItem;
	pDDI->strData = strDataName;
	pDDI->nSortOrder = nSortOrder;
	pDDI->bInactive = FALSE;
	pDDI->nGlassesOrderDataID = -1; //TES 3/15/2011 - PLID 42757
	// (j.gruber 2014-07-22 15:38) - PLID 62627 - new keyword fields
	// (j.gruber 2014-08-05 11:03) - PLID 63164 - When you have the keyword checkbox checked for a drop down, default all new drop down items in that list to have a checked keyword.   Same for unchecked.
	pDDI->bUseKeyword = m_bDefaultKeyword;
	pDDI->strKeywordOverride = "";

	m_arypEMRDropDownList.Add(pDDI);

	return pDDI;
}

// (d.thompson 2012-10-17) - PLID 53217 - Refactored this code to avoid code duplication
//This function, given a DDI, will create a datalist row in the interface from that item.  
//NOTE:  At present, this can only be used to create new rows.  New rows will never have any stamps
//	or actions, thus are always at the 0 state.
//Parameters:
//	pDDI:				A pointer to the DDI structure containing information to be put in the datalist
//	nOverrideGivenID:	This supports legacy behavior for adding a new item.  See the code in OnEditingFinishedEmrColumnList
//		to get the big picture, but this allows you to intentionally set the datalist's ID value to something that does not
//		match the associated DDI element.
//Return Value:
//	The index of the row added to the list
long CEMRTableDropdownEditorDlg::CreateDatalistRowFromDDI(const CEmrTableDropDownItem *pDDI, long nOverrideGivenID /*= 0*/)
{
	IRowSettingsPtr pRow = m_List->GetRow(-1);

	if(nOverrideGivenID != 0) {
		pRow->PutValue(ddlcID, (long)nOverrideGivenID);
	}
	else {
		pRow->PutValue(ddlcID, (long)pDDI->nID);
	}
	pRow->PutValue(ddlcData, _bstr_t(pDDI->strData));
	pRow->PutValue(ddlcSortOrder, pDDI->nSortOrder);
	// (z.manning 2009-02-10 13:22) - PLID 33026 - Now support table dropdown element based actions
	pRow->PutValue(ddlcAction, (LPCTSTR)("<" + AsString((long)pDDI->aryActions.GetSize()) + " action(s)>"));
	// (z.manning 2011-09-28 10:41) - PLID 45729 - Added stamp filter column
	// (j.jones 2012-11-27 11:15) - PLID 53144 - the content of this column is now generated in GetStampFilterHyperlinkText
	pRow->PutValue(ddlcStampFilter, _bstr_t(pDDI->GetStampFilterHyperlinkText()));
	pRow->PutValue(ddlcInactive, pDDI->bInactive == FALSE ? g_cvarFalse : g_cvarTrue);
	_variant_t varGlassesOrderDataID = g_cvarNull;
	if(pDDI->nGlassesOrderDataID != -1) {
		varGlassesOrderDataID = (long)pDDI->nGlassesOrderDataID;
	}
	pRow->PutValue(ddlcGlassesOrderDataID, varGlassesOrderDataID); //TES 3/15/2011 - PLID 42757
	// (j.gruber 2013-09-30 13:49) - PLID 58675
	pRow->PutValue(ddlcCodes, (LPCTSTR)("<" + AsString((long)pDDI->aryCodes.GetSize()) + " code(s)>"));
	long nIndex = m_List->AddRow(pRow);

	// (j.gruber 2014-07-22 12:52) - PLID 62627 - Keyword fields
	pRow->PutValue(ddlcUseKeyword, pDDI->bUseKeyword ? g_cvarTrue : g_cvarFalse);
	
	// (j.gruber 2014-07-30 10:18) - PLID 62630 - color the sel
	if (pDDI->bUseKeyword  && !pDDI->strKeywordOverride.IsEmpty())
	{
		pRow->PutCellBackColor(ddlcUseKeyword, HAS_KEYWORD_OVERRIDE_COLOR);
	}	

	return nIndex;
}

// (d.thompson 2012-10-17) - PLID 53217 - Refactored code to see if we have a duplicate
//This will perform a case sensitive search the array of DDI values for the given string.  
//Parameters:
//	strToFind:			The string you want to look for
//	nSortOrderToIgnore:	Leave -1 if you don't want to ignore anything.  Otherwise, this will look for elements that
//		match the given sort order and NOT compare them against strToFind.
//Return Value:
//	true if the string is found in the list
//	false if the string is not found in the list
bool CEMRTableDropdownEditorDlg::CheckStringForDuplicates(CString strToFind, long nSortOrderToIgnore /*= -1*/)
{
	//Loop through the array of DDI items, not the datalist
	// (a.walling 2014-06-30 10:21) - PLID 62497
	for(const auto& pDDI : m_arypEMRDropDownList) {
		if(nSortOrderToIgnore >= 0 && pDDI->nSortOrder == nSortOrderToIgnore) {
			//We've been requested to ignore this element on comparison checks.  Do nothing!
		}
		else {
			//Sort ignoring is not in use, or this isn't the ignore element
			if(strToFind == pDDI->strData) {
				return true;
			}
			else {
				//No match, keep trying
			}
		}
	}

	//Nothing was found
	return false;
}

// (d.thompson 2012-10-17) - PLID 53217 - A general encapsulation of the above sequence of events
// (j.jones 2013-01-29 10:57) - PLID 54899 - added ability to add an action
bool CEMRTableDropdownEditorDlg::TryAddNewElementByNameWithDuplicateChecks(IN CString strNewName, OUT CString &strFailureReason,
																		   BOOL bAddAction /*= FALSE*/, EmrActionObject eaoDestType /*= (EmrActionObject)eaoCpt*/, long nDestID /*= -1*/)
{
	// (d.thompson 2012-10-17) - PLID 53217 - Refactored duplicate-checks into a single function
	bool bDuplicate = CheckStringForDuplicates(strNewName);
	if(bDuplicate) {
		strFailureReason.Format(" - '%s' is a duplicated name. Two list items cannot have the same name.", strNewName);
		return false;
	}

	// (d.thompson 2012-10-17) - PLID 53217 - Use shared functions, not copied code, refactored this all to simplify
	CEmrTableDropDownItem *pDDI = GenerateNewDDI(strNewName);

	// (j.jones 2013-01-29 10:57) - PLID 54899 - added ability to add an action
	if (bAddAction) {
		EmrAction ea;
		ea.nID = -1;
		ea.eaoSourceType = eaoEmrDataItem;
		ea.nSourceID = -1;
		ea.eaoDestType = eaoDestType;
		ea.nDestID = nDestID;
		ea.nSortOrder = 1;
		ea.bPopup = false;
		ea.bSpawnAsChild = false;
		pDDI->aryActions.Add(ea);
	}

	CreateDatalistRowFromDDI(pDDI);
	return true;
}

void CEMRTableDropdownEditorDlg::OnAddDataItem() 
{
	try {
		// (d.thompson 2012-10-17) - PLID 53217 - Refactored these into functions to avoid code duplication
		CEmrTableDropDownItem *pDDI = GenerateNewDDI("[New Item]");
		//Use -2 as an override for the ID value, the OnEditingFinishedEmrColumnList() function looks for that
		//	flag as "hey this is new", so that it can decide if you hit escape whether it should delete
		//	the row for [New Item]
		long nIndex = CreateDatalistRowFromDDI(pDDI, -2);

		if(nIndex != -1)
			m_List->StartEditing(nIndex, ddlcData);

	} NxCatchAll(__FUNCTION__);
}

void CEMRTableDropdownEditorDlg::OnAddMultipleDataItem() 
{
	m_bAddingMultipleDataElements = TRUE;
	OnAddDataItem();	
}

// (j.jones 2013-01-28 17:11) - PLID 54899 - renamed Add Product to Add Other
void CEMRTableDropdownEditorDlg::OnAddOtherDataItem() 
{
	try {

		//this button now opens a pop-out list

		enum {
			miAddProduct = 1,
			miAddMedications,
		};

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();

		//we will hide the inventory options if they don't have the license
		BOOL bHasInventoryLicense = g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent);

		mnu.InsertMenu(0, MF_BYPOSITION, miAddMedications, "Add Medications...");
		if(bHasInventoryLicense) {
			mnu.InsertMenu(1, MF_BYPOSITION, miAddProduct, "Add Product...");
		}

		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_ADD_OTHER_DATA_ITEM);
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			switch(mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, rc.right, rc.top, this, NULL)) {
				case miAddMedications:
					AddMedicationsToList();
					break;
				case miAddProduct:
					AddProductToList();
					break;
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-01-29 10:50) - PLID 54899 - moved product adding to this function
void CEMRTableDropdownEditorDlg::AddProductToList()
{
	try {
		// Pop up the selection dialog
		CEMRSelectProductDlg dlg(this);

		if (dlg.DoModal() == IDOK) {
			// The user hit OK
			if (dlg.m_nSelectedProductID == -1 && dlg.m_nSelectedCategoryID == -1) {
				// We know the user hit ok, so this should be impossible ("OK" but nothing selected)
				ASSERT(FALSE);
				return;
			} else {

				if (dlg.m_nSelectedProductID != -1) {
					// One product chosen
					// (d.thompson 2012-10-17) - PLID 53217 - Refactored duplicate-checks into a single function
					CString strFailureReason;
					// (j.jones 2013-01-29 11:00) - PLID 54899 - added ability to add an action
					if(!TryAddNewElementByNameWithDuplicateChecks(dlg.m_strProductName, strFailureReason, dlg.m_bAutoAddAction, eaoCpt, dlg.m_nSelectedProductID)) {
						AfxMessageBox("Failed to add product:\r\n" + strFailureReason);
						return;
					}
					else {
						//It was added successfully
					}

				} else if (dlg.m_nSelectedCategoryID != -1) {
					// A whole category of products

					// Get the set of products that we'er going to add
					_RecordsetPtr rs = CreateRecordset(
						"SELECT ServiceT.ID, ServiceT.Name FROM ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
						"WHERE Category = %li AND Active = 1", dlg.m_nSelectedCategoryID);
					// Loop through adding each product
					FieldsPtr pflds = rs->GetFields();
					FieldPtr fldID = pflds->GetItem("ID");
					FieldPtr fldName = pflds->GetItem("Name");
					// (d.thompson 2012-10-17) - PLID 53217 - Refactored, took the time to improve the warning process to not give
					//	potentially dozes of message boxes.
					bool bAnyFailed = false;
					CString strFailureText;
					while (!rs->eof) {
						// (d.thompson 2012-10-17) - PLID 53217 - Refactored duplicate-checks into a single function
						CString strFailureReason;
						// (j.jones 2013-01-29 11:00) - PLID 54899 - added ability to add an action
						if(!TryAddNewElementByNameWithDuplicateChecks(AdoFldString(fldName), strFailureReason, dlg.m_bAutoAddAction, eaoCpt, AdoFldLong(fldID))) {
							bAnyFailed = true;
							strFailureText += strFailureReason + "\r\n";

							//Continue iterating to the next element
							rs->MoveNext();
							continue;
						}
						else {
							//This element was added successfully
						}

						rs->MoveNext();
					}

					//Now that we've tried to add all the items, report any failure
					if(bAnyFailed) {
						AfxMessageBox("Some products could not be added:\r\n" + strFailureText);
					}
				}
			}
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-01-29 10:50) - PLID 54899 - added ability to add medications
void CEMRTableDropdownEditorDlg::AddMedicationsToList()
{
	try {

		// (j.jones 2013-03-05 10:01) - PLID 55376 - changed so this interface
		// has its own dedicated dialog
		CEMRSelectMedicationsDlg dlg(this);
		if(dlg.DoModal() == IDOK) {

			if(dlg.m_arySelectedMeds.GetSize() == 0) {
				return;
			}

			CString strFailureText;
			long nCountBad = 0;

			for(int i=0;i<dlg.m_arySelectedMeds.GetSize();i++) {
				IdName idNameElement = dlg.m_arySelectedMeds.GetAt(i);
				CString strFailureReason;
				if(!TryAddNewElementByNameWithDuplicateChecks(idNameElement.strName, strFailureReason, TRUE, eaoMedication, idNameElement.nID)) {

					//only add up to 20 entries
					if(nCountBad < 20) {
						if(!strFailureText.IsEmpty()) {
							strFailureText += "\n";
						}
						strFailureText += strFailureReason;
					}
					nCountBad++;
				}
			}

			if(nCountBad > 20) {
				//we would have only tracked 20 issues, if there were more, say so
				strFailureText += "\n<more...>";
			}

			if(nCountBad > 0) {
				CString strMessage;
				strMessage.Format("The following medications could not be added:\n\n"
					"%s", strFailureText);
				AfxMessageBox(strMessage);
			}
		}

	} NxCatchAll(__FUNCTION__);
}

void CEMRTableDropdownEditorDlg::OnDeleteDataItem() 
{
	try {
		long nCurSel = m_List->GetCurSel();

		if (nCurSel != sriNoRow) {
			// Get the index in the current array
			long nSortOrder = VarLong(m_List->GetValue(nCurSel, ddlcSortOrder));

			// (a.walling 2014-06-30 10:21) - PLID 62497 - now an iterator
			auto it = GetCurDataElementArrayIndex(nSortOrder);

			// Remove the entry at that index (i.e. move it to the "deleted" array)
			{
				// Validate the deletion
				{
					// (c.haag 2006-03-13 15:44) - PLID 19689 - The next two variables
					// are used in calculating if the EMR this dialog was indirectly opened from
					// is using the list entry we want to delete.
					BOOL bAvailableInCurEMR = FALSE;
					BOOL bInUseInCurEMR = FALSE;

					// (c.haag 2006-03-14 11:45) - PLID 19689 - The next two variables track what
					// details in the current table are used. This way, we are not warned twice for
					// the same item usage. For example, if you have a saved EMN that uses a table,
					// and you open it and go straight to editing the table, there are technically
					// two unique EMN's using it -- the one in data, and the one you have open. These
					// track what details are using the table in the open EMN so that we can ignore if
					// they are again used in the saved version. If the table is not used in the open
					// EMN, then its corresponding saved version will be checked to avoid data problems.
					CString strDetailsAvailable;
					CString strDetailsInUse;

					// We use this id in a few places
					long nDataElementID = VarLong(m_List->GetValue(nCurSel, ddlcID));

					// Make sure we're allowed to delete
					if (nDataElementID != -1) {

						// (c.haag 2006-03-13 15:09) - PLID 19689 - Check the current EMR too!
						for (int i=0; i < m_apCurrentTableDetails.GetSize(); i++) {
							CString str = VarString(m_apCurrentTableDetails[i]->GetState(), "");
							CEmrTableStateIterator etsi(str);
							long X,Y,nEmrDetailImageStampID,nEmrDetailImageStampPointer,nStampID;
							CString strData;
							// (c.haag 2007-08-25 11:42) - PLID 27112 - Use the table state iterator class
							// (z.manning 2010-02-18 09:23) - PLID 37427 - Added nEmrDetailImageStampID
							// (z.manning 2011-03-02 15:09) - PLID 42335 - Added nStampID
							while(etsi.ReadNextElement(X,Y,strData,nEmrDetailImageStampID,nEmrDetailImageStampPointer,nStampID)) {
								strData.TrimRight();
								CArray<long,long> anDataElements;
								ParseTableDataValueIntoArray(strData, anDataElements);

								if (Y == m_nDataID) {
									CString strDetail;
									bAvailableInCurEMR = TRUE;
									strDetail.Format("%d,", (m_bIsCurrentDetailTemplate) ? m_apCurrentTableDetails[i]->m_nEMRTemplateDetailID : m_apCurrentTableDetails[i]->m_nEMRDetailID);
									strDetailsAvailable += strDetail;
									// (c.haag 2008-01-16 10:34) - PLID 17936 - strData may be a comma-delimited string,
									// so we can't just use a simple comparison anymore
									if (ExistsInArray(nDataElementID, anDataElements)) {
										bInUseInCurEMR = TRUE;
										strDetailsInUse += strDetail;
									}
								}
							}
						}
						strDetailsAvailable.TrimRight(",");
						strDetailsInUse.TrimRight(",");
						if (strDetailsAvailable.IsEmpty()) strDetailsAvailable = "-1";
						if (strDetailsInUse.IsEmpty()) strDetailsInUse = "-1";
						BOOL bInUse = ReturnsRecords("SELECT ID "
							"FROM EMRDetailTableDataT WHERE EmrDataID_Y = %li AND %s %s",
							m_nDataID,
							// (c.haag 2008-01-16 10:16) - PLID 17936 - Replaced "Data = {nDataElementID}" with a macro
							EMRTABLE_DATA_CONTAINS_ELEMENT("Data", nDataElementID),
							(m_bIsCurrentDetailTemplate) ? "" : (CString("AND EMRDetailID NOT IN (") + strDetailsInUse + ")")
							);
						if (bInUse || (bInUseInCurEMR && !m_bIsCurrentDetailTemplate)) {
							if (m_bMaintainCurrentTable) {
								MessageBox("This list item has been selected on existing EMRs, and cannot be deleted. If you unselected this list item in the open EMN, please go back and click on 'Save Changes', then try again.\n\n"
									"You may consider marking this item Inactive instead.", NULL, MB_ICONEXCLAMATION|MB_OK);
							} else {
								MessageBox("This list item has been selected on existing EMRs, and cannot be deleted.\n\n"
									"You may consider marking this item Inactive instead.", NULL, MB_ICONEXCLAMATION|MB_OK);
							}
							return;
						}
						// (c.haag 2006-03-13 15:09) - PLID 19689 - Check templates too!
						bInUse = ReturnsRecords("SELECT ID "
							"FROM EMRTemplateTableDefaultsT WHERE EmrDataID_Y = %li AND %s %s ",
							m_nDataID,
							// (c.haag 2008-02-22 16:09) - PLID 17936 - Replaced "Data = {nDataElementID}" with a macro
							EMRTABLE_DATA_CONTAINS_ELEMENT("Data", nDataElementID),
							(m_bIsCurrentDetailTemplate) ? (CString("AND EMRTemplateDetailID NOT IN (") + strDetailsInUse + ")") : ""
							);
						if (bInUse || (bInUseInCurEMR && m_bIsCurrentDetailTemplate)) {
							if (m_bMaintainCurrentTable) {
								MessageBox("This list item has been selected on existing EMRs, and cannot be deleted. If you unselected this list item in the open template, please go back and click on 'Save Changes', then try again.\n\n"
									"You may consider marking this item Inactive instead.", NULL, MB_ICONEXCLAMATION|MB_OK);
							} else {
								MessageBox("This list item has been selected on existing EMR templates, and cannot be deleted.\n\n"
									"You may consider marking this item Inactive instead.", NULL, MB_ICONEXCLAMATION|MB_OK);
							}
							return;
						}
					}
					// We're allowed to delete, so confirm it with the user
					{
						// See how many places it's used
						long nEMNCount, nEMNMintCount;
						if (nDataElementID != -1) {
							// See how much it's in use
							//DRT 6/25/2007 - PLID 26432 - Fixed the "ID NOT IN..." sections to properly specify what IDs are being compared.
							_RecordsetPtr prs = CreateRecordset("SELECT "
								"(SELECT COUNT(DISTINCT EMRID) FROM EMRDetailsT "
								" WHERE Deleted = 0 AND EmrInfoID = (SELECT EmrInfoID FROM EmrDataT WHERE ID = %li) %s) AS EMNCount, "
								"(SELECT COUNT(DISTINCT TemplateID) FROM EMRTemplateDetailsT "
								" INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID "
								" WHERE ActiveEmrInfoID = (SELECT EmrInfoID FROM EmrDataT WHERE ID = %li) %s "
								" AND TemplateID IN (SELECT ID FROM EMRTemplateT WHERE Deleted = 0)) AS EMNMintCount ",
								m_nDataID, (m_bIsCurrentDetailTemplate) ? "" : (CString("AND EMRDetailsT.ID NOT IN (") + strDetailsAvailable + ")"),
								m_nDataID, (m_bIsCurrentDetailTemplate) ? (CString("AND EMRTemplateDetailsT.ID NOT IN (") + strDetailsAvailable + ")") : ""
								);
							if (!prs->eof) {
								FieldsPtr pflds = prs->GetFields();
								nEMNCount = AdoFldLong(pflds, "EMNCount");
								nEMNMintCount = AdoFldLong(pflds, "EMNMintCount");
							} else {
								// This should be impossible
								ASSERT(FALSE);
								nEMNCount = nEMNMintCount = -1;
							}
						} else {
							// It's not even saved to data, so it can't possibly be in use
							nEMNCount = nEMNMintCount = 0;
						}
						// (c.haag 2006-03-13 15:42) - PLID 19689 - Include the current EMR
						if (bAvailableInCurEMR && (nEMNCount != -1 && nEMNMintCount != -1)) {
							if (m_bIsCurrentDetailTemplate) nEMNMintCount++;
							else nEMNCount++;
						}
						// Give the appropriate warning
						CString strPrelimWarning;
						if (nEMNCount != 0 || nEMNMintCount != 0) {
							strPrelimWarning.Format(
								"This list item is available for selection on %s EMNs and %s EMN Templates.  "
								"Deleting this item will remove it from all existing EMNs and EMN Templates.  This CANNOT be undone.\n\n",
								nEMNCount == -1 ? "an unknown number of" : AsString(nEMNCount), 
								nEMNMintCount == -1 ? "an unknown number of" : AsString(nEMNMintCount));
						} else {
							// It's not in use so give no preliminary warning
							strPrelimWarning.Empty();
						}
						
						CString strMsg, strName;
						
						strName = VarString(m_List->GetValue(nCurSel, ddlcData));

						strMsg.Format(
							"%s"
							"Are you sure you want to completely delete the '%s' list item?\n\n"
							"NOTE: If you do not want to completely remove this list item but would like to make it unavailable "
							"for future EMNs and EMN Templates, you may make it inactive.  To do so, click 'No' and then right-click "
							"on the data element and choose the 'Inactivate' option.", 
							strPrelimWarning, strName);
						if (MessageBox(strMsg, NULL, (strPrelimWarning.IsEmpty() ? MB_ICONQUESTION : MB_ICONWARNING)|MB_YESNO) != IDYES) {
							return;
						}
					}
				}
				// Delete the row from the datalist
				
				m_List->RemoveRow(nCurSel);
				// (a.walling 2014-06-30 10:21) - PLID 62497
				// Copy this element from one array to the other
				m_arypEMRDropDownDeleted.Add(new CEmrTableDropDownItem(*it->get()));

				m_arypEMRDropDownList.erase(it);

				UpdateSortOrders();

				// (c.haag 2007-04-12 12:06) - PLID 16050 - Reset the row selection
				// and disable the sort buttons
				m_List->CurSel = -1;
				OnSelChangedEmrColumnList(m_List->CurSel);
			}
		}
	} NxCatchAll("CEMRTableDropdownEditorDlg::OnDeleteDataItem");
}

void CEMRTableDropdownEditorDlg::OnDataUp() 
{
	try {
		// Get the current selection
		long nCurSel = m_List->GetCurSel();
		// Make sure there IS a selection
		if (nCurSel == sriNoRow) {
			return;
		}

		// If we're at the first row we can't shift HIGHER, so we have to be BEYOND the first row
		if (nCurSel > 0) {
			SwapDataElements(nCurSel - 1, nCurSel);
		}
	} NxCatchAll("CEmrItemEntryDlg::OnDataUp");
}

void CEMRTableDropdownEditorDlg::OnDataDown() 
{
	try {
		// Get the current selection
		long nCurSel = m_List->GetCurSel();
		// Make sure there IS a selection
		if (nCurSel == sriNoRow) {
			return;
		}

		// If we're at the last row we can't shift LOWER, so we have to be BEFORE the last row
		if (nCurSel < (m_List->GetRowCount() - 1)) {
			SwapDataElements(nCurSel, nCurSel + 1);
		}
	} NxCatchAll("CEmrItemEntryDlg::OnDataDown");
}

void CEMRTableDropdownEditorDlg::SwapDataElements(long nDatalistIndex1, long nDatalistIndex2)
{
	long nSortOrder1 = m_List->GetValue(nDatalistIndex1, ddlcSortOrder);
	long nSortOrder2 = m_List->GetValue(nDatalistIndex2, ddlcSortOrder);
	// (a.walling 2014-06-30 10:21) - PLID 62497
	auto it1 = GetCurDataElementArrayIndex(nSortOrder1);
	auto it2 = GetCurDataElementArrayIndex(nSortOrder2);
	// Swap the sort orders in the array, and in the datalist
	{
		// Swap the sort orders in the array
		{
			// Get the pointers
			CEmrTableDropDownItem *pDDI1 = it1->get();
			CEmrTableDropDownItem *pDDI2 = it2->get();
			// Swap the sort order values themselves
			pDDI1->nSortOrder = nSortOrder2;
			pDDI2->nSortOrder = nSortOrder1;
			// Swap the order of the elements in the array
			// (a.walling 2014-06-30 10:21) - PLID 62497 - move to swap
			std::unique_ptr<CEmrTableDropDownItem> temp = std::move(*it1);
			*it1 = std::move(*it2);
			*it2 = std::move(temp);
		}
		// Swap the sort orders in the on-screen datalist
		{
			// Swap the sort order values themselves
			m_List->PutValue(nDatalistIndex1, ddlcSortOrder, nSortOrder2);
			m_List->PutValue(nDatalistIndex2, ddlcSortOrder, nSortOrder1);
			// And re-sort
			m_List->Sort();
		}

		// Now that we've changed the order of things, refresh the buttons
		OnSelChangedEmrColumnList(m_List->CurSel);
	}
}

// (a.walling 2014-06-30 10:21) - PLID 62497 - Now returns an iterator
std::vector<std::unique_ptr<CEmrTableDropDownItem>>::iterator CEMRTableDropdownEditorDlg::GetCurDataElementArrayIndex(long nSortOrderNumber)
{
	for (auto it = m_arypEMRDropDownList.GetData().begin(); it != m_arypEMRDropDownList.GetData().end(); ++it) {
		if(nSortOrderNumber == (*it)->nSortOrder) {
			return it;
		}
	}

	// If we made it here we couldn't find it!  This represents a flaw somewhere in our synchronization 
	// between the on-screen datalist and the in-memory array of data elements
	ASSERT(FALSE);
	ThrowNxException("Could not find array index for sort order number %li!", nSortOrderNumber);
}

void CEMRTableDropdownEditorDlg::OnOK()
{
	//warn if there is nothing in the list
	// (z.manning 2011-03-17 16:07) - PLID 42722 - Only warn if this is a dropdown type column.
	if(m_List->GetRowCount() == 0 && m_nColumnType == LIST_TYPE_DROPDOWN) {
		if(IDNO == MessageBox("Your dropdown list is currently empty. If you try to use this table on an EMR, you will be unable to enter any data in this column.\n"
			"Are you sure you wish to save this empty list?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}
	}

	//warn if there are no active columns
	BOOL bActive = FALSE;
	for(int i=0;i<m_List->GetRowCount();i++) {
		if(!VarBool(m_List->GetValue(i,ddlcInactive),FALSE))
			bActive = TRUE;
	}

	// (z.manning 2011-03-17 16:07) - PLID 42722 - Only warn if this is a dropdown type column.
	if(m_List->GetRowCount() > 0 && !bActive && m_nColumnType == LIST_TYPE_DROPDOWN) {
		if(IDNO == MessageBox("Your dropdown list has no active entries. If you try to use this table on an EMR, you will be unable to enter any data in this column.\n"
			"Are you sure you wish to save the list in this state?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}
	}

	// (z.manning 2011-09-19 14:11) - PLID 41954 - Store the dropdown separators
	GetDlgItemText(IDC_EMR_DROPDOWN_SEPARATOR, m_strDropdownSeparator);
	GetDlgItemText(IDC_EMR_DROPDOWN_SEPARATOR_FINAL, m_strDropdownSeparatorFinal);

	// (a.walling 2012-08-02 16:14) - PLID 43277 - items are re-numbered as appropriate starting at 1 with no gaps
	UpdateSortOrders();

	/* (j.jones 2004-12-28 17:11) - the saving function now safely handles these characters
	//verify the contents are safe
	CString strBad;

	for(i=0;i<m_List->GetRowCount();i++) {
		CString strData = VarString(m_List->GetValue(i,ddlcData),"");
		if(strData.Find("<") != -1 || strData.Find(">") != -1 || strData.Find("\"") != -1) {
			strBad += strData;
			strBad += "\n";
		}
	}

	if(!strBad.IsEmpty()) {
		strBad = "The following items have invalid names:\n\n" + strBad;
		strBad += "\nItems cannot have the characters '>', '<', or '\"'";

		AfxMessageBox(strBad);
		return;
	}
	*/
	
	CDialog::OnOK();
}

void CEMRTableDropdownEditorDlg::OnSelChangedEmrColumnList(long nNewSel) 
{
	try{

		BOOL bEnable = nNewSel != -1;
		// (a.vengrofski 2010-02-22 13:57) - PLID <37524> - Modified to check the Autoalphabetize checkbox
		GetDlgItem(IDC_DELETE_DATA_ITEM)->EnableWindow(bEnable);
		if (!IsDlgButtonChecked(IDC_AUTO_ALPHABETIZE_DROPDOWN_DATA)) 
		{//Manual ordering
			GetDlgItem(IDC_DATA_UP)->EnableWindow(bEnable ? nNewSel != 0 : FALSE);
			GetDlgItem(IDC_DATA_DOWN)->EnableWindow(bEnable ? nNewSel != m_List->GetRowCount() - 1: FALSE);
		}else
		{//Autoalphabetize
			GetDlgItem(IDC_DATA_UP)->EnableWindow(FALSE);
			GetDlgItem(IDC_DATA_DOWN)->EnableWindow(FALSE);
		}
	} NxCatchAll("Error in CEMRTableDropdownEditorDlg::OnSelChangedEmrColumnList");
}

void CEMRTableDropdownEditorDlg::OnEditingFinishingEmrColumnList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		if(nRow == -1)
			return;
		// (b.spivey, June 27, 2013) - PLID 37100 - One function that does it all, now. 
		//	Look how much cleaner this interface event handler is!
		if(nCol == ddlcData) {
			bool bValid = ValidateListDataElement(nRow, strUserEntered, _variant_t(varOldValue)); 
			if(!bValid) {				
				*pbCommit = FALSE;
				return;
			}
		
		}
		else if(nCol == ddlcInactive) {

			//do nothing
		}

	}NxCatchAll("Error updating list.");
}

// (b.spivey, June 28, 2013) - PLID 37100 - One function to do several validations. 
bool CEMRTableDropdownEditorDlg::ValidateListDataElement(long nRowIndex, CString strUserEnteredData, CString strOldValue) 
{

	long nSortOrder = m_List->GetValue(nRowIndex,ddlcSortOrder);

	// (d.thompson 2012-10-17) - PLID 53217 - Refactored duplicate-checks into a single function
	bool bDuplicate = CheckStringForDuplicates(strUserEnteredData, nSortOrder);
	if(bDuplicate) {				
		AfxMessageBox("Another list item has the same name. Two list items cannot have the same name.");
		return false;
	}
	// (b.spivey, June 27, 2013) - PLID 37100 - New validate function specifically to find this value in use. 
	bool bInUseCanRename = ValidateListDataInUse(nRowIndex, strUserEnteredData, strOldValue); 

	//This means that it's in use and/or can be renamed. If we can't rename, we don't!
	if (!bInUseCanRename) {
		return false;
	}

	return true; 

}

// (b.spivey, June 27, 2013) - PLID 37100 - Validate function that can be used in other places. 
bool CEMRTableDropdownEditorDlg::ValidateListDataInUse(long nRowIndex, CString strUserEnteredData, CString strOldValue)
{
	// We use this id in a few places
	long nDataElementID = VarLong(m_List->GetValue(nRowIndex, ddlcID));
	// Make sure we're allowed to delete
	if (nDataElementID != -1) {
		BOOL bInUseInCurEMR = FALSE;
		// (c.haag 2006-03-13 15:09) - PLID 19689 - Check the current EMR too!
		for (int i=0; i < m_apCurrentTableDetails.GetSize(); i++) {
			CString str = VarString(m_apCurrentTableDetails[i]->GetState(), "");
			CEmrTableStateIterator etsi(str);
			long X, Y, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID;
			CString strData;
			// (c.haag 2007-08-25 11:44) - PLID 27112 - Use the table state iterator class
			// (z.manning 2010-02-18 09:45) - PLID 37427 - Added EmrDetailImageStampID
			while(etsi.ReadNextElement(X, Y, strData, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID)) {
				strData.TrimRight();
				CArray<long,long> anDataElements;
				ParseTableDataValueIntoArray(strData, anDataElements);

				if ((Y == m_nDataID) &&
					// (c.haag 2008-01-16 10:34) - PLID 17936 - strData may be a comma-delimited string,
					// so we can't just use a simple comparison anymore
					ExistsInArray(nDataElementID, anDataElements)) {
					bInUseInCurEMR = TRUE;
				}
			}
		}

		// (b.spivey, June 27, 2013) - PLID 37100 - Parameterized!
		BOOL bInUse = ReturnsRecordsParam("SELECT ID "
			// (c.haag 2008-01-16 10:16) - PLID 17936 - Replaced "Data = {nDataElementID}" with a macro
			"FROM EMRDetailTableDataT WHERE EmrDataID_Y = {INT} AND {SQL} ",
			m_nDataID, CSqlFragment(EMRTABLE_DATA_CONTAINS_ELEMENT("Data", nDataElementID)));
		if ((bInUse || (bInUseInCurEMR && !m_bIsCurrentDetailTemplate)) && strUserEnteredData.Compare(strOldValue) != 0) {
			if(IDNO == MessageBox("This list item has been selected on existing EMRs. It should not be renamed unless you are fixing a typo.\n"
				"Are you sure you wish to use this new name?", NULL, MB_ICONEXCLAMATION|MB_YESNO)) {
				return false; 
			}
		}

		// (c.haag 2006-03-13 15:09) - PLID 19689 - Check templates too!
		// (b.spivey, June 27, 2013) - PLID 37100 - Parameterized!
		bInUse = ReturnsRecordsParam("SELECT ID "
			"FROM EMRTemplateTableDefaultsT WHERE EmrDataID_Y = {INT} AND {SQL} ",
			m_nDataID, 
			// (c.haag 2008-02-22 16:09) - PLID 17936 - Replaced "Data = {nDataElementID}" with a macro
			CSqlFragment(EMRTABLE_DATA_CONTAINS_ELEMENT("Data", nDataElementID))
			);
		if ((bInUse || (bInUseInCurEMR && m_bIsCurrentDetailTemplate)) && strUserEnteredData.Compare(strOldValue) != 0) {
			if(IDNO == MessageBox("This list item has been selected on existing EMR templates. It should not be renamed unless you are fixing a typo.\n"
				"Are you sure you wish to use this new name?", NULL, MB_ICONEXCLAMATION|MB_YESNO)) {
				return false;
			}
		}
	}

	return true; 
}


void CEMRTableDropdownEditorDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	try {
		// Handle the context menu
		if (pWnd->GetSafeHwnd() && pWnd->GetSafeHwnd() == GetDlgItem(IDC_EMR_COLUMN_LIST)->GetSafeHwnd()) {

			// (j.gruber 2014-07-25 13:40) - PLID 62628 - use enum
			enum eListMenuItem {
				lmiDelete = 1,
				lmiActivate,
				lmiInactivate,
				lmiOverrideKeyword,
				lmiCheckAllKeywords,// (j.gruber 2014-07-25 13:41) - PLID 62629 - When editing multi-select lists, single-select lists, tables, and table dropdowns , add a right click and “Check All” and “Uncheck All” for the “Keyword” column to quickly check or uncheck all options.
				lmiUncheckAllKeywords,// (j.gruber 2014-07-25 13:41) - PLID 62629 - When editing multi-select lists, single-select lists, tables, and table dropdowns , add a right click and “Check All” and “Uncheck All” for the “Keyword” column to quickly check or uncheck all options.
			};

			// The context menu for the data element list is based on the current selection
			long nCurSel = m_List->GetCurSel();
			if (nCurSel != sriNoRow) {
				// Build the menu for the current row
				CMenu mnu;
				mnu.CreatePopupMenu();
				mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, lmiDelete, "&Delete");
				if (VarBool(m_List->GetValue(nCurSel, ddlcInactive))) {
					mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, lmiActivate, "&Activate");
				} else {
					mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, lmiInactivate, "&Inactivate");
				}

				// (j.gruber 2014-07-25 13:40) - PLID 62628 - new menu option				
				auto it = GetCurDataElementArrayIndex(VarLong(m_List->GetValue(nCurSel, ddlcSortOrder)));
				mnu.AppendMenu(MF_SEPARATOR | MF_BYPOSITION);
				if (it->get()->bUseKeyword)
				{
					mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, lmiOverrideKeyword, "Edit Search Keywords");
				}
				else {
					mnu.AppendMenu(MF_DISABLED | MF_STRING | MF_BYPOSITION, lmiOverrideKeyword, "Edit Search Keywords");
				}
				
				// (j.gruber 2014-07-25 14:41) - PLID 62629 - When editing multi-select lists, single-select lists, tables, and table dropdowns , add a right click and “Check All” and “Uncheck All” for the “Keyword” column to quickly check or uncheck all options.
				mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, lmiCheckAllKeywords, "Check All Keywords");
				mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, lmiUncheckAllKeywords, "Uncheck All Keywords");

				// Pop up the menu and gather the immediate response
				CPoint pt = CalcContextMenuPos(pWnd, point);
				long nMenuResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x, pt.y, this);
				switch (nMenuResult) {
				case lmiDelete: // delete
					OnDeleteDataItem();
					break;
				case lmiActivate: // activate
					{
						// (a.walling 2014-06-30 10:21) - PLID 62497
						auto it = GetCurDataElementArrayIndex(VarLong(m_List->GetValue(nCurSel, ddlcSortOrder)));
						// Update the datalist on screen
						m_List->PutValue(nCurSel, ddlcInactive, _variant_t(VARIANT_FALSE, VT_BOOL));
						// Update the array in memory
						it->get()->bInactive = FALSE;
					}
					break;
				case lmiInactivate: // inactivate
					{
						// (a.walling 2014-06-30 10:21) - PLID 62497
						auto it = GetCurDataElementArrayIndex(VarLong(m_List->GetValue(nCurSel, ddlcSortOrder)));
						// Update the datalist on screen
						m_List->PutValue(nCurSel, ddlcInactive, _variant_t(VARIANT_TRUE, VT_BOOL));
						// Update the array in memory
						it->get()->bInactive = TRUE;
					}
					break;
				case lmiOverrideKeyword:
				{				
					OnKeywordOverride(m_List->GetRow(nCurSel));
				}
					break;
					// (j.gruber 2014-07-25 13:41) - PLID 62629 - When editing multi-select lists, single-select lists, tables, and table dropdowns , add a right click and “Check All” and “Uncheck All” for the “Keyword” column to quickly check or uncheck all options.
				case lmiCheckAllKeywords:
				{
					long p = m_List->GetRowEnum(0);
					while (p) {
						IDispatchPtr lpDisp;
						m_List->GetNextRowEnum(&p, &lpDisp);
						if (IRowSettingsPtr pRowCheck = lpDisp) {
							pRowCheck->PutValue(ddlcUseKeyword, g_cvarTrue);

							auto it = GetCurDataElementArrayIndex(VarLong(pRowCheck->GetValue(ddlcSortOrder)));
							it->get()->bUseKeyword = TRUE;

						}
					}
				}

				break;
				// (j.gruber 2014-07-25 13:41) - PLID 62629 - When editing multi-select lists, single-select lists, tables, and table dropdowns , add a right click and “Check All” and “Uncheck All” for the “Keyword” column to quickly check or uncheck all options.
				case lmiUncheckAllKeywords:
				{
					long p = m_List->GetRowEnum(0);
					while (p) {
						IDispatchPtr lpDisp;
						m_List->GetNextRowEnum(&p, &lpDisp);
						if (IRowSettingsPtr pRowCheck = lpDisp) {
							pRowCheck->PutValue(ddlcUseKeyword, g_cvarFalse);

							auto it = GetCurDataElementArrayIndex(VarLong(pRowCheck->GetValue(ddlcSortOrder)));
							it->get()->bUseKeyword = FALSE;
							
						}
					}					
				}

					break;
				case 0:
					// The user canceled, do nothing
					break;
				default:
					// Unexpected response!
					ASSERT(FALSE);
					ThrowNxException("Unexpected return value %li from context menu!", nMenuResult);
				}
				mnu.DestroyMenu();
			}
		}
	} NxCatchAll("CEMRTableDropdownEditorDlg::OnContextMenu");
}

void CEMRTableDropdownEditorDlg::OnRButtonDownEmrColumnList(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_List->CurSel = nRow;
}

void CEMRTableDropdownEditorDlg::OnTimer(UINT nIDEvent)
{
	// (c.haag 2008-02-20 15:25) - PLID 28686 - Intercept auto-"Add New" timer events
	try {
		CDialog::OnTimer(nIDEvent);

		if (IDT_AUTO_ADD_NEW == nIDEvent)
		{
			KillTimer(nIDEvent);

			// This makes it so when the user enters a new data item, and presses the <Enter> key,
			// then it won't fire the OnOK message.
			GetDlgItem(IDC_EMR_COLUMN_LIST)->SetFocus();
			
			// Now act as if the user pressed the Add button
			PostMessage(WM_COMMAND, IDC_ADD_DATA_ITEM);
		}
	}
	NxCatchAll("Error in CEMRTableDropdownEditorDlg::OnTimer");
}

// (z.manning 2009-02-10 12:53) - PLID 33026
void CEMRTableDropdownEditorDlg::LeftClickEmrColumnList(long nRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		if(nRow == -1) {
			return;
		}

		IRowSettingsPtr pRow = m_List->GetRow(nRow);
		if(pRow == NULL) {
			return;
		}

		// (a.walling 2014-06-30 10:21) - PLID 62497
		auto it = GetCurDataElementArrayIndex(VarLong(pRow->GetValue(ddlcSortOrder)));
		CEmrTableDropDownItem* pddi = it->get();

		switch(nCol)
		{
			case ddlcAction:
			{
				CWaitCursor wc;
				// Prep the actions dialog
				CEmrActionDlg dlg(this);
				if(m_pdlgEmrItemEntry->GetCurrentEMN() != NULL) {
					dlg.SetCurrentEMN(m_pdlgEmrItemEntry->GetCurrentEMN());
				}
				dlg.m_SourceType = eaoEmrTableDropDownItem;
				dlg.m_nSourceID = CEmrActionDlg::bisidNotBoundToData;
				dlg.m_strSourceObjectName = pddi->strData;
				dlg.m_nOriginatingID = m_pdlgEmrItemEntry->GetID();
				for(int nActionIndex = 0; nActionIndex < pddi->aryActions.GetSize(); nActionIndex++) {
					dlg.m_arActions.Add(pddi->aryActions.GetAt(nActionIndex));
				}

				// Pop up the dialog
				if (dlg.DoModal() == IDOK) {
					// The user clicked OK, so write the selected actions back to our data element's array
					pddi->aryActions.RemoveAll();
					for(nActionIndex = 0; nActionIndex < dlg.m_arActions.GetSize(); nActionIndex++) {
						pddi->aryActions.Add(dlg.m_arActions.GetAt(nActionIndex));
					}
					pRow->PutValue(ddlcAction, (LPCTSTR)("<" + AsString((long)pddi->aryActions.GetSize()) + " action(s)>"));
				}
			}
			break;

			case ddlcStampFilter:
			{
				// (z.manning 2011-09-28 10:43) - PLID 45729
				CEmrTableDropdownStampSetupDlg dlg(pddi, this);

				// (j.jones 2012-11-28 10:15) - PLID 53144 - pass in our array of all dropdowns,
				// the dialog will use this to confirm if a stamp is filtered in any other dropdown
				dlg.m_arypEMRDropDownList = &m_arypEMRDropDownList;

				if(dlg.DoModal() == IDOK) {
					// (z.manning 2011-09-28 14:45) - PLID 45729 - If they made changes the dialog would have saved them to the
					// dropdown item pointer (pddi).
					// (j.jones 2012-11-27 11:15) - PLID 53144 - the content of this column is now generated in GetStampFilterHyperlinkText
					pRow->PutValue(ddlcStampFilter, _bstr_t(pddi->GetStampFilterHyperlinkText()));
				}
			}
			break;
			// (j.gruber 2013-09-30 13:53) - PLID 58675 - codes
			case ddlcCodes: {				
					CEMRCodeArray aryOrig;
					aryOrig = pddi->aryCodes;
					CEMRCodeArray pTmp;
					pTmp = pddi->aryCodes;
					CEMRCodeEditorDlg dlgCodes(&pTmp);

					if (dlgCodes.DoModal() == IDOK) 
					{
						//did our code array change
						if (aryOrig.IsDifferent(&pTmp)) 
						{
							//remove our current array values
							pddi->aryCodes.RemoveAll();
							//add the new codes
							pddi->aryCodes = pTmp;
							//set our boolean so we know it changed
							pddi->bCodesChanged = TRUE;
						}

						//update the link
						pRow->PutValue(ddlcCodes, (LPCTSTR)("<" + AsString((long)pddi->aryCodes.GetSize()) + " code(s)>"));

					}
				}
			break;
		}

	}NxCatchAll("CEMRTableDropdownEditorDlg::LeftClickEmrColumnList");
}



// (a.vengrofski 2010-02-22 09:52) - PLID <37524> - Added this function to handle the sorting.
void CEMRTableDropdownEditorDlg::OnBnClickedAutoAlphabetizeDropdownData()
{
	try{
		if (IsDlgButtonChecked(IDC_AUTO_ALPHABETIZE_DROPDOWN_DATA)) 
		{//Alphabetize the list
			m_List->GetColumn(ddlcSortOrder)->SortPriority = -1;
			m_List->GetColumn(ddlcData)->SortPriority = 0;
			m_List->GetColumn(ddlcData)->SortAscending = TRUE;
			m_bAutoAlphabetizeDropDown = TRUE;
		}
		else 
		{//Set to the manual sorting
			m_List->GetColumn(ddlcData)->SortPriority = -1;
			m_List->GetColumn(ddlcSortOrder)->SortPriority = 0;
			m_List->GetColumn(ddlcSortOrder)->SortAscending = TRUE;
			m_bAutoAlphabetizeDropDown = FALSE;
		}
		m_List->Sort();//Sorts the list, this need to be called or nothing will happen on screen.
		OnSelChangedEmrColumnList(m_List->CurSel);// (a.vengrofski 2010-02-22 14:09) - PLID <37524> - Was best to modify the behavior and then call it here.
	} NxCatchAll("Error in CEMRTableDropdownEditorDlg::OnBnClickedAutoAlphabetizeDropdownData");
}

// (a.walling 2012-08-02 16:14) - PLID 43277 - items are re-numbered as appropriate starting at 1 with no gaps
void CEMRTableDropdownEditorDlg::UpdateSortOrders()
{
	using namespace std;

	map<long, long> oldToNew; // only active items
	
	{
		// sort array by sort order
		multimap<long, CEmrTableDropDownItem*> sortOrders;

		// note that multimap is not guaranteed to order duplicate keys in a stable order; however all msvc implementations do. in C++11 the standard
		// specifies explicitly that it inserts at upper_bound.
		// (a.walling 2014-06-30 10:21) - PLID 62497
		for (const auto& pItem : m_arypEMRDropDownList) {
			sortOrders.insert(make_pair(pItem->nSortOrder, pItem.get()));
		}

		// iterate entries in sorted order, mapping old sort order to new sort order starting from 1
		long nCurSortOrder = 1;
		foreach (CEmrTableDropDownItem* pItem, sortOrders | boost::adaptors::map_values) {
			ASSERT(!oldToNew.count(pItem->nSortOrder));
			oldToNew.insert(make_pair(pItem->nSortOrder, nCurSortOrder));
			pItem->nSortOrder = nCurSortOrder;
			nCurSortOrder++;
		}
	}

	// update UI
	if (m_List->GetRowCount() <= 0) {
		return;
	}

	long p = m_List->GetRowEnum(0);
	while (p) {
		IDispatchPtr lpDisp;
		m_List->GetNextRowEnum(&p, &lpDisp);		
		if (IRowSettingsPtr pRow = lpDisp) {
			ASSERT(oldToNew.count(VarLong(pRow->Value[ddlcSortOrder], -1)));
			pRow->Value[ddlcSortOrder] = oldToNew[pRow->Value[ddlcSortOrder]];
		}
	}
}

// (d.thompson 2012-10-16) - PLID 53217
void CEMRTableDropdownEditorDlg::OnBnClickedPasteSetupData()
{
	try {
		PasteSetupData();
	} NxCatchAll(__FUNCTION__);
}

// (b.spivey, June 26, 2013) - PLID 37100 - Event handler for spell checking. 
void CEMRTableDropdownEditorDlg::OnBnClickedSpellCheck() 
{

	try {
		//Start from the top. 
		long nRowIndex = m_List->GetTopRowIndex(); 

		//If no row, then we can't do anything. And since the spell checker takes 5 seconds to load lets just not bother. 
		if (nRowIndex == sriNoRow) { return; }

		IRowSettingsPtr pRow = m_List->GetRow(nRowIndex); 
		WSPELLLib::_DWSpellPtr pwsSpellChecker = CreateSpellExObject(this);

		//While we have a row. 
		while (pRow) {
			//Grab these right quick. 
			CString strPreCheck = VarString(pRow->GetValue(ddlcData), "");
			pwsSpellChecker->Text = _bstr_t(strPreCheck); 

			//The spell checker is doing things. The answer is a value that tells us what happened. All we care about is -17 and 0. 
			long nAns = pwsSpellChecker->Start(); 
			//Something happened. 
			if (nAns == 0) {
				//Get the new value.
				CString strCorrected = VarString(pwsSpellChecker->Text);
				//If these are equal, this was perfectly spelled. Or it wasn't and the user just doesn't care. 
				if (strPreCheck.Compare(strCorrected) != 0) {
					//Put the corrected value in here. 
					if (ValidateListDataElement(pRow->GetIndex(), strCorrected, strPreCheck)) {
						
						long nSortOrder = m_List->GetValue(nRowIndex, ddlcSortOrder);
						// (a.walling 2014-06-30 10:21) - PLID 62497
						auto it = GetCurDataElementArrayIndex(nSortOrder);
						CEmrTableDropDownItem *pDDI = it->get();	
						pDDI->strData = strCorrected;
						pRow->PutValue(ddlcData, _variant_t(strCorrected)); 
					}
				}
			}
			//user cancelled. 
			else if (nAns == -17) {
				break; 
			}
			
			//Interate. 
			nRowIndex++;
			//new row. 
			pRow = m_List->GetRow(nRowIndex); 
		}
		
	}NxCatchAll(__FUNCTION__);

}


// (d.thompson 2012-10-16) - PLID 53217 - Paste data from the clipboard into a datalist.
void CEMRTableDropdownEditorDlg::PasteSetupData()
{
	//Give them a warning, as the icon may be a touch unclear, and if they just click it at random, who knows what may result.
	int nResult = DontShowMeAgain(this, "This button will paste the current contents of your clipboard into the item list.  Each item should be separated by a newline.\r\n\r\nDo you wish to paste now?", 
		"EMRTableDropdownPasteWarning", "Warning", FALSE, TRUE);

	//For some reason, "don't show me again" forces it to forever return IDOK.  So treat that as approval.
	if(nResult != IDYES && nResult != IDOK) {
		return;
	}

	CClipboardToStringArray data(cpoAll);

	bool bAnyFailed = false;
	CString strFailureText;
	for(int i = 0; i < data.m_aryItems.GetSize(); i++) {
		CString strData = data.m_aryItems.GetAt(i);

		// (d.thompson 2012-10-17) - PLID 53217 - Refactored duplicate-checks into a single function
		CString strFailureReason;
		if(!TryAddNewElementByNameWithDuplicateChecks(strData, strFailureReason)) {
			bAnyFailed = true;
			strFailureText += strFailureReason + "\r\n";
		}
		else {
			//This element was added successfully
		}
	}

	//Did any fail?  If so, alert the user
	if(bAnyFailed) {
		AfxMessageBox("Some pasted items could not be added:\r\n" + strFailureText);
	}
}

// (j.gruber 2014-07-25 13:40) - PLID 62628 - KeywordOverride Handler
// (j.gruber 2014-07-30 10:18) - PLID 62630 - changed to row
void CEMRTableDropdownEditorDlg::OnKeywordOverride(NXDATALISTLib::IRowSettingsPtr pRow)
{
	try {
		
		auto it = GetCurDataElementArrayIndex(VarLong(pRow->GetValue(ddlcSortOrder)));
		CEmrTableDropDownItem *pDDI = it->get();
		CString strResult = pDDI->strKeywordOverride;
		CString strResultOrig = strResult;
		if (InputBoxLimited(this, "Edit Search Keywords", strResult, "", 150, false, false, "Cancel"))
		{
			//set the varible back to what it was
			if (strResultOrig != strResult)
			{
				pDDI->strKeywordOverride = strResult;

				// (j.gruber 2014-07-30 10:18) - PLID 62630 - color the row
				if (!pDDI->strKeywordOverride.IsEmpty())
				{
					pRow->PutCellBackColor(ddlcUseKeyword, HAS_KEYWORD_OVERRIDE_COLOR);
				}
				else {
					pRow->PutCellBackColor(ddlcUseKeyword, HAS_NO_KEYWORD_OVERRIDE_COLOR);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);

}