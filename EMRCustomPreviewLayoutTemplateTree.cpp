// EMRCustomPreviewLayoutTemplateTree.cpp : implementation file
//
// (c.haag 2013-01-16) - PLID 54737 - This is a pane used in the Html MDI frame
// that lists all the available template topics and details for adding to the layout.
//

#include "stdafx.h"
#include "EMRCustomPreviewLayoutTemplateTree.h"
#include "NxAPI.h"

using namespace NXDATALIST2Lib;

#define IDC_TEMPLATE_TREE				1002

#define BUILT_IN_COLLECTION_NAME	"Built-In Fields"
#define BUILT_IN_COLLECTION_ID		1L

// (c.haag 2013-03-19) - PLID 55697 - Letter Writing field support
#define LW_FIELD_COLLECTION_NAME	"Letter Writing Fields"
#define LW_FIELD_COLLECTION_ID		2L

// CEMRCustomPreviewLayoutTemplateTree

enum TreeCols
{
	tcData = 0L,
	tcName = 1L,
};

IMPLEMENT_DYNAMIC(CEMRCustomPreviewLayoutTemplateTree, CDockablePane)

CEMRCustomPreviewLayoutTemplateTree::CEMRCustomPreviewLayoutTemplateTree(long nEmrTemplateID, const CString& strEmrTemplateName)
{
	m_nEmrTemplateID = nEmrTemplateID;
	m_strEmrTemplateName = strEmrTemplateName;
	m_bShowHiddenItems = FALSE;
}

void CEMRCustomPreviewLayoutTemplateTree::SetShowHiddenItems(BOOL bShow)
{
	m_bShowHiddenItems = bShow;
	RefreshControl();
}

// Creates the tree control
void CEMRCustomPreviewLayoutTemplateTree::CreateControl()
{
	m_wndTree.CreateControl(_T("NXDATALIST2.NxDataListCtrl.1"), "", WS_CHILD, CRect(0,0,0,0), this, IDC_TEMPLATE_TREE);

	if (m_wndTree.GetSafeHwnd()) {
		m_wndTree.ModifyStyleEx(WS_EX_NOPARENTNOTIFY, 0);
	}
	
	m_Tree = m_wndTree.GetControlUnknown();
	m_Tree->IsComboBox = VARIANT_FALSE;
	m_Tree->AdoConnection = GetRemoteData();
	m_Tree->AllowSort = VARIANT_FALSE;
	m_Tree->InsertColumn(tcData, "", "", 0, csFixedWidth);
	m_Tree->InsertColumn(tcName, "", "", 100, csVisible|csWidthPercent);
	m_Tree->HeadersVisible = VARIANT_FALSE;
	m_Tree->HighlightVisible = VARIANT_TRUE;
	m_Tree->AllowMultiSelect = VARIANT_FALSE;

	m_wndTree.ShowWindow(SW_SHOW);
}

// Populates the tree control with data
void CEMRCustomPreviewLayoutTemplateTree::RefreshControl()
{
	if (IsWindow(m_wndTree.GetSafeHwnd()))
	{
		// Clear the tree
		ClearTree();

		// (c.haag 2013-03-13) - PLID 55611 - Add built-in fields to the tree
		AddBuiltInFieldsToTree();
		// (c.haag 2013-03-19) - PLID 55697 - Add letter writing fields to the tree
		AddLWFieldsToTree();

		// Get the template
		NexTech_Accessor::_EMRTemplatePtr pTemplate = GetAPI()->GetEMRTemplate(
					GetAPISubkey(), GetAPILoginToken(), _bstr_t(m_nEmrTemplateID));

		// Now start iterating through the template
		if (NULL != pTemplate->RootLevelTopics)
		{
			PopulateTreeRecurse(Nx::SafeArray<IUnknown *>(pTemplate->RootLevelTopics));
		}

		// Expand all the rows
		// (z.manning 2013-03-12 15:21) - PLID 55595 - Actually, we don't want to expand the rows by default
		//ExpandOrCollapseAllTopics(TRUE);
	}
}

// (c.haag 2013-03-13) - PLID 55611 - Adds built-in fields to the tree (such as Signature)
void CEMRCustomPreviewLayoutTemplateTree::AddBuiltInFieldsToTree()
{
	// (c.haag 2013-03-13) - PLID 55611 - Initial implementation
	OLE_COLOR clrBuiltInRows = (OLE_COLOR)RGB(0,0,255); // All built-in rows should be blue

	// Add the master row
	IRowSettingsPtr pMasterRow = m_Tree->GetNewRow();
	pMasterRow->Value[tcName] = BUILT_IN_COLLECTION_NAME;
	pMasterRow->Value[tcData] = BUILT_IN_COLLECTION_ID;
	pMasterRow->ForeColor = clrBuiltInRows;
	m_Tree->AddRowAtEnd(pMasterRow, NULL);

	// Now add all known built-in fields
	AddBuiltInFieldToTree(pMasterRow, clrBuiltInRows, "Signature");
}

// (c.haag 2013-03-19) - PLID 55697 - Adds Letter Writing fields to the tree
void CEMRCustomPreviewLayoutTemplateTree::AddLWFieldsToTree()
{
	OLE_COLOR clrLWRows = (OLE_COLOR)RGB(96,64,0); // Tinted yellow like the module is

	// Add the master row
	IRowSettingsPtr pMasterRow = m_Tree->GetNewRow();
	pMasterRow->Value[tcName] = LW_FIELD_COLLECTION_NAME;
	pMasterRow->Value[tcData] = LW_FIELD_COLLECTION_ID;
	pMasterRow->ForeColor = clrLWRows;
	m_Tree->AddRowAtEnd(pMasterRow, NULL);

	// Now add all known letter writing fields
	CStringSortedArrayNoCase astrFields;
	::LoadEMRLetterWritingMergeFieldList(astrFields);
	const int nFields = astrFields.GetCount();
	for (int i=0; i < nFields; i++)
	{
		IRowSettingsPtr pRow = m_Tree->GetNewRow();
		pRow->Value[tcName] = (LPCTSTR)astrFields[i];
		pRow->ForeColor = clrLWRows;
		m_Tree->AddRowAtEnd(pRow, pMasterRow);		
	}
}

// (c.haag 2013-03-13) - PLID 55611 - Adds a built-in field to the tree (such as Signature)
void CEMRCustomPreviewLayoutTemplateTree::AddBuiltInFieldToTree(IRowSettingsPtr pMasterRow, OLE_COLOR clr, const CString& strField)
{
	IRowSettingsPtr pRow = m_Tree->GetNewRow();
	pRow->Value[tcName] = (LPCTSTR)strField;
	pRow->ForeColor = clr;
	m_Tree->AddRowAtEnd(pRow, pMasterRow);
}

// (z.manning 2013-03-12 15:31) - PLID 55595
void CEMRCustomPreviewLayoutTemplateTree::ExpandOrCollapseAllTopics(BOOL bExpand)
{
	for (IRowSettingsPtr pTopicRow = m_Tree->GetFirstRow(); NULL != pTopicRow; pTopicRow = pTopicRow->GetNextRow())
	{
		pTopicRow->Expanded = bExpand ? VARIANT_TRUE : VARIANT_FALSE;
	}
}

// Clears the tree
void CEMRCustomPreviewLayoutTemplateTree::ClearTree()
{
	// Release all topic objects
	for (IRowSettingsPtr pTopicRow = m_Tree->GetFirstRow(); NULL != pTopicRow; pTopicRow = pTopicRow->GetNextRow())
	{
		if (VT_EMPTY != pTopicRow->Value[tcData].vt && VT_NULL != pTopicRow->Value[tcData].vt && VT_I4 != pTopicRow->Value[tcData].vt)
		{
			LPDISPATCH p = (LPDISPATCH)pTopicRow->Value[tcData];
			p->Release();
		}
		for (IRowSettingsPtr pDetailRow = pTopicRow->GetFirstChildRow(); NULL != pDetailRow; pDetailRow = pDetailRow->GetNextRow())
		{
			if (VT_EMPTY != pDetailRow->Value[tcData].vt && VT_NULL != pDetailRow->Value[tcData].vt && VT_I4 != pTopicRow->Value[tcData].vt)
			{
				LPDISPATCH p = (LPDISPATCH)pDetailRow->Value[tcData];
				p->Release();
			}
		}
	}

	m_Tree->Clear();
}

// Populates the tree control with data
void CEMRCustomPreviewLayoutTemplateTree::PopulateTreeRecurse(Nx::SafeArray<IUnknown *> saTopics)
{
	foreach(NexTech_Accessor::_EMRTemplateTopicPtr pTopic, saTopics)
	{
		// Add the topic to the tree
		if (m_bShowHiddenItems || !(pTopic->PreviewFlags & epfHideItem))
		{
			IRowSettingsPtr pTopicRow = m_Tree->GetNewRow();
			BOOL bTopicHidden = (pTopic->PreviewFlags & epfHideItem);
			pTopic->AddRef();
			pTopicRow->Value[tcData] = (LPUNKNOWN)pTopic;

			// We should indicate if a topic was spawned.
			CString strTopicName = (LPCTSTR)pTopic->Name;
			CString strSourceActionName = (LPCTSTR)pTopic->SourceActionName;
			if (strSourceActionName.GetLength() > 0) {
				pTopicRow->Value[tcName] = (LPCTSTR)FormatString("%s (%s)", strTopicName, strSourceActionName);
			} else {
				pTopicRow->Value[tcName] = pTopic->Name;
			}
			
			if (m_bShowHiddenItems && bTopicHidden) {
				pTopicRow->ForeColor = RGB(128,128,128);
			}
			// Note how we always add the topic on the root branch. This is for two reasons: Easier navigation
			// (no horizontal scrolling), and because that's how they are presented in a standard print preview.
			m_Tree->AddRowAtEnd(pTopicRow, NULL);

			// Now add the topic details
			if (NULL != pTopic->details)
			{
				Nx::SafeArray<IUnknown *> saDetails = Nx::SafeArray<IUnknown *>(pTopic->details);
				foreach(NexTech_Accessor::_EMRTemplateDetailPtr pDetail, saDetails)
				{
					BOOL bDetailHidden = bTopicHidden || (pDetail->PreviewFlags & epfHideItem);
					if (m_bShowHiddenItems || !(pDetail->PreviewFlags & epfHideItem))
					{
						IRowSettingsPtr pDetailRow = m_Tree->GetNewRow();
						pDetail->AddRef();
						pDetailRow->Value[tcData] = (LPUNKNOWN)pDetail;

						// We should indicate if a detail was spawned
						CString strDetailName = (LPCTSTR)pDetail->Name;
						CString strSourceActionName = (LPCTSTR)pDetail->SourceActionName;
						if (strSourceActionName.GetLength() > 0) 
						{
							pDetailRow->Value[tcName] = (LPCTSTR)FormatString("%s (%s)", strDetailName, strSourceActionName);
						}
						else {
							pDetailRow->Value[tcName] = pDetail->Name;
						}

						if (m_bShowHiddenItems && bDetailHidden) {
							pDetailRow->ForeColor = RGB(128,128,128);
						}
						m_Tree->AddRowAtEnd(pDetailRow, pTopicRow);
					}
				}
			}
		}

		// We must go deeper (recurse into child topics)
		if (NULL != pTopic->ChildTopics)
		{
			PopulateTreeRecurse(Nx::SafeArray<IUnknown *>(pTopic->ChildTopics));
		}
	}
}

// (c.haag 2013-03-13) - PLID 55611 - Returns TRUE if the row is a built-in field
BOOL CEMRCustomPreviewLayoutTemplateTree::IsBuiltInField(IRowSettingsPtr pRow)
{
	IRowSettingsPtr pParentRow = pRow->GetParentRow();
	if (NULL == pParentRow) return FALSE;
	_variant_t v = pParentRow->Value[tcData];
	return (VT_I4 == v.vt && VarLong(v) == BUILT_IN_COLLECTION_ID);
}

// (c.haag 2013-03-19) - PLID 55697 - Returns TRUE if the row is a letter writing field
BOOL CEMRCustomPreviewLayoutTemplateTree::IsLWField(IRowSettingsPtr pRow)
{
	IRowSettingsPtr pParentRow = pRow->GetParentRow();
	if (NULL == pParentRow) return FALSE;
	_variant_t v = pParentRow->Value[tcData];
	return (VT_I4 == v.vt && VarLong(v) == LW_FIELD_COLLECTION_ID);
}

BEGIN_MESSAGE_MAP(CEMRCustomPreviewLayoutTemplateTree, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CEMRCustomPreviewLayoutTemplateTree message handlers

int CEMRCustomPreviewLayoutTemplateTree::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	try {
		if (__super::OnCreate(lpCreateStruct) == -1)
			return -1;

		// Create the datalist tree 
		CreateControl();
		// Now fill it with data (excluding hidden items by default)
		RefreshControl();

	} NxCatchAll(__FUNCTION__);

	return 0;
}

void CEMRCustomPreviewLayoutTemplateTree::OnSize(UINT nType, int cx, int cy)
{
	try {
		__super::OnSize(nType, cx, cy);
		
		// Ensure the control is as large as the pane
		if (IsWindow(m_wndTree.GetSafeHwnd()))
		{
			CRect rcClient;
			GetClientRect(&rcClient);
			m_wndTree.MoveWindow(rcClient);
		}
		
	} NxCatchAll(__FUNCTION__);
}

void CEMRCustomPreviewLayoutTemplateTree::OnDestroy()
{
	try {
		ClearTree();
		__super::OnDestroy();
	} NxCatchAll(__FUNCTION__);
}

BOOL CEMRCustomPreviewLayoutTemplateTree::OnShowControlBarMenu(CPoint point)
{
	// Don't show the pane's default context menu
	return FALSE;
}

BEGIN_EVENTSINK_MAP(CEMRCustomPreviewLayoutTemplateTree, CDockablePane)
    //{{AFX_EVENTSINK_MAP(CEMRCustomPreviewLayoutTemplateTree)
	ON_EVENT(CEMRCustomPreviewLayoutTemplateTree, IDC_TEMPLATE_TREE, 12 /* DragBegin */, OnDragBeginTree, VTS_PBOOL VTS_DISPATCH VTS_I2 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

// Generate HTML from a tree row. It should either be a topic or a detail; and we will get
// the appropriate text depending on what it is
CString CEMRCustomPreviewLayoutTemplateTree::GenerateHtml(IRowSettingsPtr pRow)
{
	// (c.haag 2013-01-21) - PLID 54738 - Initial implementation
	CString strResult;

	// (c.haag 2013-03-13) - PLID 55611 - Handle built-in fields
	if (IsBuiltInField(pRow))
	{
		// Bring over the label and the value. The user can always remove the label if they don't want it.
		CString fieldNameHtml = (LPCSTR)GetAPI()->FormatBuiltInNameFieldForEMRCustomPreviewLayout((LPCTSTR)VarString(pRow->Value[tcName]));
		CString fieldValueHtml = (LPCSTR)GetAPI()->FormatBuiltInDataFieldForEMRCustomPreviewLayout((LPCTSTR)VarString(pRow->Value[tcName]));
		strResult = FormatString("<p>%s&#160;%s</p>", fieldNameHtml, fieldValueHtml);
	}
	// (c.haag 2013-03-19) - PLID 55697 - Bring over LW fields
	else if (IsLWField(pRow))
	{
		CString fieldValueHtml = (LPCSTR)GetAPI()->FormatLetterWritingDataFieldForEMRCustomPreviewLayout((LPCTSTR)VarString(pRow->Value[tcName]));
		strResult = FormatString("<p>%s</p>", fieldValueHtml);
	}
	// (c.haag 2013-03-13) - PLID 55611 - Handle null and empty rows
	else if (VT_EMPTY == pRow->Value[tcData].vt || VT_NULL == pRow->Value[tcData].vt || VT_I4 == pRow->Value[tcData].vt)
	{
		strResult.Empty(); // Ensure this is empty
	}
	else
	{
		NexTech_Accessor::_EMRTemplateTopicPtr pTopic((LPDISPATCH)pRow->Value[tcData]);
		NexTech_Accessor::_EMRTemplateDetailPtr pDetail((LPDISPATCH)pRow->Value[tcData]);
		if (NULL != pTopic)
		{
			// We only support bringing over the topic name. Although it would make sense to check the preview
			// flags to see whether the name should be visible, we don't do it because we always expect a drag
			// begin operation to mean that the user does want the topic name on the layout despite their previous
			// preference.
			strResult = (LPCSTR)GetAPI()->FormatTopicNameFieldForEMRCustomPreviewLayout(pTopic);
		}
		else if (NULL != pDetail)
		{
			// (c.haag 2012-02-21) - PLID 55293 - We now put every field in the same paragraph
			CString detailNameHtml = "";
			// Always bring over the data, even if the user elected not to print the data by default, because we 
			// always expect a drag begin operation to mean that the user does intend for this despite their
			// previous preference.
			CString detailDataHtml = (LPCSTR)GetAPI()->FormatDetailDataFieldForEMRCustomPreviewLayout(pDetail);

			// See if we should bring over the title
			if (!(pDetail->PreviewFlags & epfHideTitle))
			{
				detailNameHtml = CString((LPCSTR)GetAPI()->FormatDetailNameFieldForEMRCustomPreviewLayout(pDetail)) + "&#160;"; // Trailing space
			}

			// (j.armen 2013-02-01 10:30) - PLID 54686 - Use P tags instead of BR tags
			strResult = FormatString("<p>%s%s</p>", detailNameHtml, detailDataHtml);
		}
	}
	return strResult;
}

// This function is abridged from http://support.microsoft.com/kb/274308 . It is used to generate a
// packet which can be used in an Html drag-and-drop.

CString GenerateHtmlClipboardData(const CString& strHtml) 
{
    // Create temporary buffer for HTML header...
    char *buf = new char [400 + strHtml.GetLength()];

    // Create a template string for the HTML header...
    strcpy(buf,
        "Version:0.9\r\n"
        "StartHTML:00000000\r\n"
        "EndHTML:00000000\r\n"
        "StartFragment:00000000\r\n"
        "EndFragment:00000000\r\n"
        "<html><body>\r\n"
        "<!--StartFragment -->\r\n");

    // Append the HTML...
    strcat(buf, strHtml);
    strcat(buf, "\r\n");
    // Finish up the HTML format...
    strcat(buf,
        "<!--EndFragment-->\r\n"
        "</body>\r\n"
        "</html>");

    // Now go back, calculate all the lengths, and write out the
    // necessary header information. Note, wsprintf() truncates the
    // string when you overwrite it so you follow up with code to replace
    // the 0 appended at the end with a '\r'...
    char *ptr = strstr(buf, "StartHTML");
    wsprintf(ptr+10, "%08u", strstr(buf, "<html>") - buf);
    *(ptr+10+8) = '\r';

    ptr = strstr(buf, "EndHTML");
    wsprintf(ptr+8, "%08u", strlen(buf));
    *(ptr+8+8) = '\r';

    ptr = strstr(buf, "StartFragment");
    wsprintf(ptr+14, "%08u", strstr(buf, "<!--StartFrag") - buf + strlen("<!--StartFragment -->\r\n"));
    *(ptr+14+8) = '\r';

    ptr = strstr(buf, "EndFragment");
    wsprintf(ptr+12, "%08u", strstr(buf, "<!--EndFrag") - buf);
    *(ptr+12+8) = '\r';

	CString strResult = buf;
	delete[] buf;
	return strResult;
}

void CEMRCustomPreviewLayoutTemplateTree::OnDragBeginTree(BOOL FAR* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags)
{
	// (c.haag 2013-01-21) - PLID 54738 - Initial implementation
	try {
		// Build HTML consisting of all the selected rows
		CArray<IRowSettingsPtr,IRowSettingsPtr&> aSelectedRows;
		for (IRowSettingsPtr pTopicRow = m_Tree->GetFirstRow(); NULL != pTopicRow; pTopicRow = pTopicRow->GetNextRow())
		{
			if (VARIANT_FALSE != pTopicRow->Selected) {
				aSelectedRows.Add(pTopicRow);
			}
			for (IRowSettingsPtr pDetailRow = pTopicRow->GetFirstChildRow(); NULL != pDetailRow; pDetailRow = pDetailRow->GetNextRow())
			{
				if (VARIANT_FALSE != pDetailRow->Selected) {
					aSelectedRows.Add(pDetailRow);
				}
			}
		}

		// Generate the HTML to paste into the target. We currently don't support a drag with multiple rows; but when we do,
		// we will need to append breaks to spread out the results.
		CString strHtml;
		for (int i=0; i < aSelectedRows.GetCount(); i++)
		{
			CString str = GenerateHtml(aSelectedRows[i]);
			if (!str.IsEmpty())
			{
				if (!strHtml.IsEmpty()) {
					strHtml += "<br/><br/>";
				}
				strHtml += str;
			}
		}

		if (!strHtml.IsEmpty())
		{
			// Generate the packet for the drag-and-drop
			CString strData = GenerateHtmlClipboardData(strHtml);

			// use CacheData or CacheGlobalData to define the data to be moved/copied
			// In this example, the cell text of the source item is copied into a global
			// storage area
			m_dataSource.Empty();
			HGLOBAL hGlobal = ::GlobalAlloc(GHND, (strData.GetLength()+1)*sizeof(TCHAR));
			if (!hGlobal)
				return;
			LPTSTR lpStr = (LPTSTR) GlobalLock(hGlobal);
			lstrcpy(lpStr, (LPCTSTR) strData);// copy the cell text into the global area
			GlobalUnlock(hGlobal);
			int cfid = RegisterClipboardFormat("HTML Format");
			m_dataSource.CacheGlobalData(cfid, hGlobal);

			// Initiate the OLE drag & drop, OLE mechanisms now take over (using COleDropSource
			// and COleDataSource classes defined here) and COleDropTarget in the target control/window
			m_dataSource.DoDragDrop( DROPEFFECT_COPY|DROPEFFECT_MOVE, NULL, &m_dropSource);
		}

	} NxCatchAll(__FUNCTION__);
}