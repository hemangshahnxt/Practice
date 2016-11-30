// EmrCommonListSetupDlg.cpp : implementation file
// (c.haag 2011-03-17) - PLID 42889 - Initial implementation. Unless otherwise commented, everything
// in this class falls on this item.

#include "stdafx.h"
#include "Practice.h"
#include "EmrCommonListSetupDlg.h"
#include "EmrCommonListItemsSetupDlg.h"
#include "EmrItemEntryDlg.h"

using namespace NXDATALIST2Lib;

enum ListColumns {
	lcID = 0,
	lcOrderID,
	lcName,
	lcItems,
	lcColor,
	lcGroupOnPreviewPane, // (c.haag 2011-04-11) - PLID 43234
	lcInactive,
};

// CEmrCommonListSetupDlg dialog

IMPLEMENT_DYNAMIC(CEmrCommonListSetupDlg, CNxDialog)

CEmrCommonListSetupDlg::CEmrCommonListSetupDlg(
	const CEmrInfoCommonListCollection& srcData, 
	CEmrInfoDataElementArray* pAvailableEmrDataElements,
	CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrCommonListSetupDlg::IDD, pParent)
{
	// Load the initial data
	m_data = srcData;
	m_pAvailableEmrDataElements = pAvailableEmrDataElements;
	
	// Build a map of inactive EMR Data ID's
	for (int i=0; i < m_pAvailableEmrDataElements->GetSize(); i++)
	{
		CEmrInfoDataElement* p = m_pAvailableEmrDataElements->GetAt(i);
		if (p->m_bInactive && p->m_nID > 0)
		{
			m_mapInactiveEmrDataIDs.SetAt(p->m_nID, p);
		}
	}
}

CEmrCommonListSetupDlg::~CEmrCommonListSetupDlg()
{
}

// Returns the resultant changes (test for an IDOK return value before calling this)
const CEmrInfoCommonListCollection& CEmrCommonListSetupDlg::GetResult() const
{
	return m_data;
}

// Returns the formatted text for the items column of a given row
_bstr_t CEmrCommonListSetupDlg::GetItemsColumnText(const CEmrInfoCommonList& list)
{
	CString strItems;
	const int nItems = list.GetItemCount();

	// Get the count of items in this list, not including inactive ones
	int nActiveItems = 0;
	for (int i=0; i < nItems; i++)
	{
		CEmrInfoCommonListItem item = list.GetItemByIndex(i);
		const CEmrInfoDataElement* pElement = item.GetEmrInfoDataElement();
		if (NULL != pElement)
		{
			if (!pElement->m_bInactive) {
				nActiveItems++;
			}
		}
		else {
			const long nEmrDataID = item.GetEmrDataID();
			CEmrInfoDataElement* pDummy;
			if (!m_mapInactiveEmrDataIDs.Lookup(nEmrDataID, pDummy))
			{
				nActiveItems++;
			}
		}
	}

	switch (nActiveItems)
	{
	case 0:
		strItems.Format("< No Items >");
		break;
	case 1:
		strItems.Format("< 1 Item >");
		break;
	default:
		strItems.Format("< %d Items >", nActiveItems);
		break;
	}

	// If there's at least one inactive item, throw in "Active" because the list technically has
	// more items than there are visible.
	if (nItems != nActiveItems) {
		strItems.Replace("Item", "Active Item");
	}

	return _bstr_t(strItems);
}

// Repopulate the visible list. This only happens on initialization.
void CEmrCommonListSetupDlg::UpdateView()
{
	m_CollectionList->Clear();
	
	// Go through all the lists we got in m_data and create datalist rows for them. Inactive
	// common lists should appear at the bottom because of how they're ordered in m_data.
	int nLists = m_data.GetListCount();
	for (int i=0; i < nLists; i++)
	{
		CEmrInfoCommonList list = m_data.GetListByIndex(i);
		IRowSettingsPtr pRow = m_CollectionList->GetNewRow();

		pRow->Value[lcID] = list.GetID();
		pRow->Value[lcOrderID] = list.GetOrderID();
		pRow->Value[lcName] = _bstr_t(list.GetName());
		pRow->Value[lcItems] = GetItemsColumnText(list);
		pRow->Value[lcColor] = (long)list.GetColor();
		pRow->Value[lcGroupOnPreviewPane] = list.GetGroupOnPreviewPane() ? g_cvarTrue : g_cvarFalse; // (c.haag 2011-04-11) - PLID 43234
		pRow->Value[lcInactive] = list.IsInactive() ? g_cvarTrue : g_cvarFalse;
		UpdateRowColors(pRow);

		m_CollectionList->AddRowSorted(pRow, NULL);
	}
}

// Update the colors of a row given its data content
void CEmrCommonListSetupDlg::UpdateRowColors(IRowSettingsPtr pRow)
{
	BOOL bInactive = VarBool(pRow->Value[lcInactive]);
	COLORREF clr = (bInactive) ? RGB(128,128,128) : (COLORREF)VarLong(pRow->Value[lcColor]);
	pRow->PutForeColor((OLE_COLOR)clr);
	pRow->PutCellBackColor(lcColor, (OLE_COLOR)clr);
}

// Update the button enabled/disabled states
void CEmrCommonListSetupDlg::UpdateButtonStates()
{
	IRowSettingsPtr pRow = m_CollectionList->CurSel;
	if (NULL == pRow)
	{
		m_btnDelete.EnableWindow(FALSE);
		m_btnUp.EnableWindow(FALSE);
		m_btnDown.EnableWindow(FALSE);
	}
	else {
		m_btnDelete.EnableWindow(TRUE);
		if (NULL == pRow->GetNextRow()) {
			m_btnDown.EnableWindow(FALSE);
		} else {
			m_btnDown.EnableWindow(TRUE);
		}
		if (NULL == pRow->GetPreviousRow()) {
			m_btnUp.EnableWindow(FALSE);
		} else {
			m_btnUp.EnableWindow(TRUE);
		}
	}
}

// Ensure that it's safe to save the data and close the window
BOOL CEmrCommonListSetupDlg::Validate()
{
	int nLists = m_data.GetListCount();
	for (int i=0; i < nLists; i++)
	{
		CEmrInfoCommonList list = m_data.GetListByIndex(i);
		if (m_data.DoesListExist(list.GetName(), list.GetID())) {
			AfxMessageBox(FormatString("The name '%s' appears more than once. Please correct this.", list.GetName()));
			return FALSE;
		}
		else if (0 == list.GetItemCount()) {
			AfxMessageBox(FormatString("The list '%s' has no items in it. Please correct this or delete the list from your set.", list.GetName()));
			return FALSE;				
		}
	}
	return TRUE;
}

// Take the datalist content and assign it to the common lists
void CEmrCommonListSetupDlg::ApplyFormDataToList()
{
	// Go through the list and synchronize datalist values with the list content. We also fill in the ordinals here.
	IRowSettingsPtr pRow = m_CollectionList->GetFirstRow();
	BOOL bInactiveRowsExist = FALSE;
	long nRow = 0;
	while (NULL != pRow)
	{
		const long nListID = VarLong(pRow->GetValue(lcID));
		CEmrInfoCommonList list = m_data.GetListByID(nListID);
		list.SetName( VarString(pRow->GetValue(lcName)) );
		// The items field is updated on the spot when the user changes them; do nothing for it here
		list.SetColor( (COLORREF)VarLong(pRow->GetValue(lcColor)) );
		list.SetGroupOnPreviewPane( VarBool(pRow->GetValue(lcGroupOnPreviewPane)) );
		list.SetInactive( VarBool(pRow->GetValue(lcInactive)) );

		// Update the ordinal. Whenever an item is inactivated, we want to scoot it to the bottom,
		// so ignore inactive items for now.
		if (FALSE == list.IsInactive()) {
			list.SetOrderID(++nRow);
		} else {
			bInactiveRowsExist = TRUE;
		}

		m_data.SetListByID(nListID, list);
		pRow = pRow->GetNextRow();
	}

	// Now go through the list once more assigning new ordinals to the inactive items. It doesn't
	// really matter what order they are in because they're inactive.
	if (bInactiveRowsExist)
	{
		pRow = m_CollectionList->GetFirstRow();
		while (NULL != pRow)
		{
			const long nListID = VarLong(pRow->GetValue(lcID));
			CEmrInfoCommonList list = m_data.GetListByID(nListID);
			if (list.IsInactive()) {
				list.SetOrderID(++nRow);
				m_data.SetListByID(nListID, list);
			}
			pRow = pRow->GetNextRow();
		}
	}
}

// Change list button text color
void CEmrCommonListSetupDlg::PromptForButtonTextColor(LPDISPATCH lpRow)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		return;
	}

	long nCurColor = VarLong(pRow->GetValue(lcColor));
	CColorDialog dlg(nCurColor, CC_ANYCOLOR|CC_RGBINIT);
	if(dlg.DoModal() == IDOK) {
		pRow->PutValue(lcColor, (long)dlg.m_cc.rgbResult);
		UpdateRowColors(pRow);
	}
}

// Swaps two datalist rows
void CEmrCommonListSetupDlg::SwapRows(IRowSettingsPtr p1, IRowSettingsPtr p2)
{
	// Swap the content of each row
	const short nCols = m_CollectionList->GetColumnCount();
	for (short i=0; i < nCols; i++)
	{
		_variant_t vTemp = p1->Value[i];
		p1->Value[i] = p2->Value[i];
		p2->Value[i] = vTemp;
	}
	UpdateRowColors(p1);
	UpdateRowColors(p2);
}

void CEmrCommonListSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_ADD_COMMON_LIST, m_btnAdd);
	DDX_Control(pDX, IDC_BTN_DELETE_COMMON_LIST, m_btnDelete);
	DDX_Control(pDX, IDC_COMMON_LIST_UP, m_btnUp);
	DDX_Control(pDX, IDC_COMMON_LIST_DOWN, m_btnDown);
}

BEGIN_MESSAGE_MAP(CEmrCommonListSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_ADD_COMMON_LIST, OnBtnAddCommonList)
	ON_BN_CLICKED(IDC_BTN_DELETE_COMMON_LIST, OnBtnDeleteCommonList)
	ON_BN_CLICKED(IDC_COMMON_LIST_UP, OnBtnUp)
	ON_BN_CLICKED(IDC_COMMON_LIST_DOWN, OnBtnDown)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEmrCommonListSetupDlg, CNxDialog)
	ON_EVENT(CEmrCommonListSetupDlg, IDC_COMMON_LISTS, 6, OnRButtonDownCollectionList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrCommonListSetupDlg, IDC_COMMON_LISTS, 9, OnEditingFinishingCollectionList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEmrCommonListSetupDlg, IDC_COMMON_LISTS, 19, LeftClickCollectionList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrCommonListSetupDlg, IDC_COMMON_LISTS, 2, OnSelChangedCollectionList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CEmrCommonListSetupDlg, IDC_COMMON_LISTS, 10, OnEditingFinishedCollectionList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

// CEmrCommonListSetupDlg message handlers

BOOL CEmrCommonListSetupDlg::OnInitDialog() 
{		
	try 
	{
		CNxDialog::OnInitDialog();

		// Initialize controls
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnUp.AutoSet(NXB_UP);
		m_btnDown.AutoSet(NXB_DOWN);
		m_CollectionList = BindNxDataList2Ctrl(IDC_COMMON_LISTS, false);

		// Populate the list
		UpdateView();

		// Ensure the buttons are properly enabled/disabled
		UpdateButtonStates();
	}
	NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEmrCommonListSetupDlg::OnBtnAddCommonList()
{
	try {
		CEmrInfoCommonList list = m_data.CreateNewList();
		IRowSettingsPtr pRow = m_CollectionList->GetNewRow();
		pRow->Value[lcID] = list.GetID();
		// Don't fill in the order ID column; we only needed it in the initial load
		pRow->Value[lcName] = _bstr_t(list.GetName());
		pRow->Value[lcItems] = _bstr_t("< No Items >");
		pRow->Value[lcColor] = (long)list.GetColor();
		pRow->PutForeColor((OLE_COLOR)list.GetColor());
		pRow->PutCellBackColor(lcColor, (OLE_COLOR)list.GetColor());
		pRow->Value[lcGroupOnPreviewPane] = list.GetGroupOnPreviewPane() ? g_cvarTrue : g_cvarFalse; // (c.haag 2011-04-11) - PLID 43234
		pRow->Value[lcInactive] = list.IsInactive() ? g_cvarTrue : g_cvarFalse;
		m_CollectionList->AddRowBefore( pRow, m_CollectionList->GetFirstRow() );
		UpdateRowColors(pRow);
		m_CollectionList->CurSel = pRow;
		m_CollectionList->StartEditing(pRow, lcName);
		UpdateButtonStates();
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrCommonListSetupDlg::OnBtnDeleteCommonList()
{
	try {
		IRowSettingsPtr pRow(m_CollectionList->CurSel);
		long nListID = VarLong(pRow->Value[lcID]);
		CString strName = VarString(pRow->Value[lcName]);
		if (IDYES == AfxMessageBox(FormatString("Are you sure you wish to delete the '%s' list?", strName), MB_YESNO | MB_ICONQUESTION))
		{
			m_data.DeleteList(nListID);
			m_CollectionList->RemoveRow(pRow);
			UpdateButtonStates();
		}
	}
	NxCatchAll(__FUNCTION__);
}

// This is the button the user presses to move the current row up. We accomplish this by swapping
// row content and setting the current selection to the target row.
void CEmrCommonListSetupDlg::OnBtnUp()
{
	try {
		IRowSettingsPtr p1 = m_CollectionList->CurSel;
		if (NULL != p1)
		{
			IRowSettingsPtr p2 = p1->GetPreviousRow();
			if (NULL != p2)
			{
				SwapRows(p1,p2);
				m_CollectionList->CurSel = p2;
				UpdateButtonStates();
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// This is the button the user presses to move the current row down. We accomplish this by swapping
// row content and setting the current selection to the target row.
void CEmrCommonListSetupDlg::OnBtnDown()
{
	try {
		IRowSettingsPtr p1 = m_CollectionList->CurSel;
		if (NULL != p1)
		{
			IRowSettingsPtr p2 = p1->GetNextRow();
			if (NULL != p2)
			{
				SwapRows(p1,p2);
				m_CollectionList->CurSel = p2;
				UpdateButtonStates();
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrCommonListSetupDlg::OnOK()
{
	try {
		// Take the content from the datalist and apply it to the common lists
		ApplyFormDataToList();
		if (!Validate()) {
			return;
		}		

		CDialog::OnOK();
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrCommonListSetupDlg::OnRButtonDownCollectionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_CollectionList->CurSel = pRow;
		// Because we changed the current selection, we need to update the up/down button states
		UpdateButtonStates();

		enum {
			eChangeColor = 1,
			eDeleteList = 2,
			eEditList = 3,
		};

		// Create the menu
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eChangeColor, "&Change Color");
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eEditList, "&Edit List");
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eDeleteList, "&Delete List");

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if(nRet == eChangeColor) {
			PromptForButtonTextColor(pRow);
		}
		else if(nRet == eDeleteList) {
			OnBtnDeleteCommonList();
		}
		else if(nRet == eEditList) {
			LeftClickCollectionList(lpRow, lcItems, x, y, 0);
		}

	}NxCatchAll(__FUNCTION__);
}

void CEmrCommonListSetupDlg::OnEditingFinishingCollectionList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol == lcName) {

			CString strName = CString(strUserEntered);
			strName.TrimLeft();
			strName.TrimRight();

			if(strName.IsEmpty()) {
				AfxMessageBox("The list name cannot be blank.");
				*pbCommit = FALSE;
				return;
			}
			else if(strName.GetLength() > 255) {
				AfxMessageBox("The list name cannot be longer than 255 characters.");
				*pbCommit = FALSE;
				return;
			}
			// Duplicate name checking happens on pre-save validation
			_variant_t varNew(strName);
			*pvarNewValue = varNew.Detach();
		}
	}NxCatchAll(__FUNCTION__);
}

void CEmrCommonListSetupDlg::LeftClickCollectionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol)
		{
			case lcItems:
			{
				// Open the items list for editing
				long nListID = VarLong(pRow->GetValue(lcID));
				CEmrInfoCommonList list = m_data.GetListByID(nListID);
				CEmrCommonListItemsSetupDlg dlg(list, m_pAvailableEmrDataElements, this);
				// Preserve a list of inactive items. Once they dismiss the dialog, they will no longer
				// be part of the results. We have to add them back in.
				CArray<CEmrInfoDataElement*,CEmrInfoDataElement*> arInactiveItems;
				CStringArray astrInactiveItems;
				int i;
				for (i=0; i < list.GetItemCount(); i++)
				{
					CEmrInfoCommonListItem item = list.GetItemByIndex(i);
					CEmrInfoDataElement* pElement = item.GetEmrInfoDataElement();
					if (NULL != pElement)
					{
						if (pElement->m_bInactive) {
							arInactiveItems.Add(pElement);
							astrInactiveItems.Add(pElement->m_strData);
						}
					}
					else {
						const long nEmrDataID = item.GetEmrDataID();
						CEmrInfoDataElement* p;
						if (m_mapInactiveEmrDataIDs.Lookup(nEmrDataID, p))
						{
							arInactiveItems.Add(p);
							astrInactiveItems.Add(p->m_strData);
						}
					}
				}

				// If every item in the list is inactive, let the user know. This should be an exceedingly
				// rare case. If some items are inactive, I don't think there's a pressing need to show this
				// every time the client goes into it; it could be more annoying than not.
				if (arInactiveItems.GetSize() > 0 && arInactiveItems.GetSize() == list.GetItemCount())
				{
					CString strWarning = "This list contains one or more inactive items which will not be displayed in the selection window:\r\n\r\n";
					for (i=0; i < astrInactiveItems.GetSize(); i++)
					{
						if (i >= 20) {
							strWarning += "<more...>\r\n";
							break;
						}
						else {
							strWarning += astrInactiveItems[i] + "\r\n";
						}
					}
					strWarning += "\r\nIf you do not intend to reactivate any of the items above, you should consider deleting and then re-creating this common list.\r\n";
					AfxMessageBox(strWarning);
				}
				
				if (IDOK == dlg.DoModal())
				{
					list = dlg.GetResult();
					// Now re-add the inactive items
					for (i=0; i < arInactiveItems.GetSize(); i++) {
						list.AddItem(arInactiveItems[i]);
					}
					pRow->Value[lcItems] = GetItemsColumnText(list);
					m_data.SetListByID(nListID, list);
				}
			}
			break;

			case lcColor:
				// Change the color
				PromptForButtonTextColor(pRow);
				break;
		}

	}
	NxCatchAll(__FUNCTION__);
}

void CEmrCommonListSetupDlg::OnSelChangedCollectionList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try
	{
		// Make sure the up / down arrow enabled states are correct
		UpdateButtonStates();
	}
	NxCatchAll(__FUNCTION__);
}


void CEmrCommonListSetupDlg::OnEditingFinishedCollectionList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	try
	{
		// Make sure the colors are up to date (could have become inactive for example)
		IRowSettingsPtr pRow(lpRow);
		UpdateRowColors(pRow);
	}
	NxCatchAll(__FUNCTION__);
}
