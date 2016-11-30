// NexFormsImportWordTemplatesSheet.cpp : implementation file
//

#include "stdafx.h"
#include "NexFormsImportWordTemplatesSheet.h"
#include "FileUtils.h"
#include "MergeEngine.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "GlobalStringUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/********************************************************************************/
/*																				*/
/* For documentation on the NexForms importer/exporter, see:					*/
/*																				*/
/*		http://192.168.1.2/developers/doku.php?id=nexforms_importer_exporter	*/
/*																				*/
/********************************************************************************/

using namespace NXDATALIST2Lib;

enum CategoryTreeColumns
{
	ctcCheckbox = 0,
	ctcName,
	ctcPointer,
};

enum WordTemplateColumns
{
	wtcCheckbox = 0,
	wtcName,
	wtcPointer,
	wtcType,
};

enum PacketColumns
{
	pcCheckbox = 0,
	pcName,
	pcPointer,
};

enum ExistingPacketColumns
{
	epcName = 0,
};

enum GroupFilterColumns
{
	gfcName = 0,
};


// (z.manning, 08/30/2007) - PLID 18359 - Created a new importer for NexForms.
/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportWordTemplatesSheet dialog


CNexFormsImportWordTemplatesSheet::CNexFormsImportWordTemplatesSheet(CNexFormsImportWizardMasterDlg* pParent)
	: CNexFormsImportWizardSheet(CNexFormsImportWordTemplatesSheet::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNexFormsImportWordTemplatesSheet)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bInitialLoad = TRUE;
}

CNexFormsImportWordTemplatesSheet::~CNexFormsImportWordTemplatesSheet()
{
	for(int i = 0; i < m_arySelectedCategories.GetSize(); i++)
	{
		if(m_arySelectedCategories.GetAt(i) != NULL) {
			delete m_arySelectedCategories.GetAt(i);
		}
	}
	m_arySelectedCategories.RemoveAll();
}

void CNexFormsImportWordTemplatesSheet::DoDataExchange(CDataExchange* pDX)
{
	CNexFormsImportWizardSheet::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexFormsImportWordTemplatesSheet)
	DDX_Control(pDX, IDC_UNSELECT_ALL_TEMPLATES, m_btnUnselectAllTemplates);
	DDX_Control(pDX, IDC_UNSELECT_ALL_PACKETS, m_btnUnselectAllPackets);
	DDX_Control(pDX, IDC_SELECT_ALL_TEMPLATES, m_btnSelectAllTemplates);
	DDX_Control(pDX, IDC_SELECT_ALL_PACKETS, m_btnSelectAllPackets);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexFormsImportWordTemplatesSheet, CNexFormsImportWizardSheet)
	//{{AFX_MSG_MAP(CNexFormsImportWordTemplatesSheet)
	ON_BN_CLICKED(IDC_SELECT_ALL_PACKETS, OnSelectAllPackets)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_PACKETS, OnUnselectAllPackets)
	ON_BN_CLICKED(IDC_SELECT_ALL_TEMPLATES, OnSelectAllTemplates)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_TEMPLATES, OnUnselectAllTemplates)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CNexFormsImportWordTemplatesSheet, CNexFormsImportWizardSheet)
    //{{AFX_EVENTSINK_MAP(CNexFormsImportWordTemplatesSheet)
	ON_EVENT(CNexFormsImportWordTemplatesSheet, IDC_WORD_TEMPLATE_CATEGORY_TREE, 10 /* EditingFinished */, OnEditingFinishedWordTemplateCategoryTree, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CNexFormsImportWordTemplatesSheet, IDC_PACKET_LIST, 10 /* EditingFinished */, OnEditingFinishedPacketList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CNexFormsImportWordTemplatesSheet, IDC_PACKET_LIST, 6 /* RButtonDown */, OnRButtonDownPacketList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CNexFormsImportWordTemplatesSheet, IDC_PACKET_LIST, 9 /* EditingFinishing */, OnEditingFinishingPacketList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CNexFormsImportWordTemplatesSheet, IDC_GROUP_FILTER, 2 /* SelChanged */, OnSelChangedGroupFilter, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CNexFormsImportWordTemplatesSheet, IDC_WORD_TEMPLATE_LIST, 6 /* RButtonDown */, OnRButtonDownWordTemplateList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CNexFormsImportWordTemplatesSheet, IDC_WORD_TEMPLATE_LIST, 10 /* EditingFinished */, OnEditingFinishedWordTemplateList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportWordTemplatesSheet message handlers

BOOL CNexFormsImportWordTemplatesSheet::OnInitDialog() 
{
	try
	{
		CNexFormsImportWizardSheet::OnInitDialog();

		m_pdlCategoryTree = BindNxDataList2Ctrl(this, IDC_WORD_TEMPLATE_CATEGORY_TREE, GetRemoteData(), false);
		m_pdlWordTemplates = BindNxDataList2Ctrl(this, IDC_WORD_TEMPLATE_LIST, GetRemoteData(), false);
		m_pdlPackets = BindNxDataList2Ctrl(this, IDC_PACKET_LIST, GetRemoteData(), false);
		m_pdlGroupFilter = BindNxDataList2Ctrl(this, IDC_GROUP_FILTER, GetRemoteData(), false);

		// (z.manning, 07/09/2007) - The existing packets datalist is actually not visible on this sheet.
		// Its purpose is so we can safely asynchrounsly load all existing packet names so we don't create
		// a new packet with a duplicate name.
		m_pdlExistingPackets = BindNxDataList2Ctrl(this, IDC_EXISTING_PACKETS, GetRemoteData(), true);

	}NxCatchAll("CNexFormsImportWordTemplatesSheet::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNexFormsImportWordTemplatesSheet::Load()
{
	m_pdlWordTemplates->Clear();
	m_pdlPackets->Clear();
	for(int i = 0; i < m_pdlgMaster->m_ImportInfo.m_arypWordTemplates.GetSize(); i++)
	{
		NexFormsWordTemplate *word = m_pdlgMaster->m_ImportInfo.m_arypWordTemplates.GetAt(i);

		// (z.manning, 07/06/2007) - Add the template to the word template list.
		IRowSettingsPtr pTemplateRow = m_pdlWordTemplates->GetNewRow();
		pTemplateRow->PutValue(wtcCheckbox, word->bImport ? _variant_t(VARIANT_TRUE,VT_BOOL) : _variant_t(VARIANT_FALSE,VT_BOOL));
		pTemplateRow->PutValue(wtcName, _bstr_t(word->strFilename));
		pTemplateRow->PutValue(wtcPointer, (long)word);
		pTemplateRow->PutValue(wtcType, _bstr_t(word->strType));
		UpdateRowVisibility(pTemplateRow);
		m_pdlWordTemplates->AddRowSorted(pTemplateRow, NULL);
		if(word->bAlreadyExists) {
			pTemplateRow->PutCellForeColor(wtcName, RGB(255,0,0));
		}

		// (z.manning, 07/06/2007) - Populate the group filter (plastic, derm, etc.)
		if(NULL == m_pdlGroupFilter->FindByColumn(gfcName, _bstr_t(word->strGroup), NULL, VARIANT_FALSE))
		{
			IRowSettingsPtr pGroupFilterRow = m_pdlGroupFilter->GetNewRow();
			pGroupFilterRow->PutValue(gfcName, _bstr_t(word->strGroup));
			m_pdlGroupFilter->AddRowSorted(pGroupFilterRow, NULL);
		}

		// (z.manning, 07/05/2007) - If it's not there already, add the category to the array.
		BOOL bFoundType = FALSE;
		BOOL bFoundCategory = FALSE;
		for(int j = 0; j < m_arySelectedCategories.GetSize() && (!bFoundCategory || !bFoundType); j++)
		{
			// (z.manning, 07/05/2007) - See if this matches a type already in the array
			if(m_arySelectedCategories.GetAt(j)->strName.CompareNoCase(word->strType) == 0
				&& m_arySelectedCategories.GetAt(j)->strParent.IsEmpty())
			{
				bFoundType = TRUE;
			}

			// (z.manning, 07/05/2007) - See if this matches a category already in the array
			if(m_arySelectedCategories.GetAt(j)->strName.CompareNoCase(word->strCategory) == 0
				&& !m_arySelectedCategories.GetAt(j)->strParent.IsEmpty())
			{
				bFoundCategory = TRUE;
			}
		}
		if(!bFoundType) {
			// (z.manning, 07/05/2007) - This type isn't in the array yet, so add it.
			ASSERT(!word->strType.IsEmpty());
			SelectedCategory *sc = new SelectedCategory;
			sc->strName = word->strType;
			sc->strParent = "";
			m_arySelectedCategories.Add(sc);
		}
		if(!bFoundCategory && !word->strCategory.IsEmpty()) {
			// (z.manning, 07/05/2007) - This category isn't in the array yet, so add it.
			SelectedCategory *sc = new SelectedCategory;
			sc->strName = word->strCategory;
			sc->strParent = word->strType;
			m_arySelectedCategories.Add(sc);
		}
	}

	// (z.manning, 07/05/2007) - Go through all categories and now add them to the datalist.
	m_pdlCategoryTree->Clear();
	for(i = 0; i < m_arySelectedCategories.GetSize(); i++)
	{
		IRowSettingsPtr pRow = m_pdlCategoryTree->GetNewRow();
		pRow->PutValue(ctcName, _bstr_t(m_arySelectedCategories.GetAt(i)->strName));
		pRow->PutValue(ctcCheckbox, m_arySelectedCategories.GetAt(i)->bSelected ? _variant_t(VARIANT_TRUE,VT_BOOL) : _variant_t(VARIANT_FALSE,VT_BOOL));
		pRow->PutValue(ctcPointer, (long)m_arySelectedCategories.GetAt(i));
		IRowSettingsPtr pParentRow = NULL;
		if(!m_arySelectedCategories.GetAt(i)->strParent.IsEmpty())
		{
			pParentRow = m_pdlCategoryTree->FindByColumn(ctcName, _bstr_t(m_arySelectedCategories.GetAt(i)->strParent), NULL, VARIANT_FALSE);
			// (z.manning, 07/05/2007) - We should have gotten a row because the parent rows are added to
			// the array before the children above.
			if(pParentRow != NULL) {
				pParentRow->PutExpanded(VARIANT_TRUE);
			}
			else {
				ASSERT(FALSE);
			}
		}
		m_pdlCategoryTree->AddRowSorted(pRow, pParentRow);
	}

	// (z.manning, 07/09/2007) - Load packets.
	for(i = 0; i < m_pdlgMaster->m_ImportInfo.m_arypPackets.GetSize(); i++)
	{
		NexFormsPacket *packet = m_pdlgMaster->m_ImportInfo.m_arypPackets.GetAt(i);
		IRowSettingsPtr pRow = m_pdlPackets->GetNewRow();
		pRow->PutValue(pcCheckbox, packet->bImport ? _variant_t(VARIANT_TRUE,VT_BOOL) : _variant_t(VARIANT_FALSE,VT_BOOL));
		pRow->PutValue(pcName, _bstr_t(packet->strName));
		pRow->PutValue(pcPointer, (long)packet);
		m_pdlPackets->AddRowSorted(pRow, NULL);
	}

	// (z.manning, 07/17/2007) - Select the correct group option.
	CString strGroup = GetRemotePropertyText("NexFormsImportTemplateGroup", "{ All }", 0, "<None>", true);
	if(NULL == m_pdlGroupFilter->SetSelByColumn(gfcName, _bstr_t(strGroup))) {
		// (z.manning, 07/17/2007) - We should have found the selection. If we get here, someone
		// may have been messing with the folder names where the NexForms templates are stored internally.
		// Or we may be missing the word templates altogether.
		ASSERT(FALSE);
	}
	HandleGroupChange();

	if(m_bInitialLoad) {
		SelectNonExistingTemplateNames();
		SelectNonExistingPackets();
	}

	m_bInitialLoad = FALSE;
}

void CNexFormsImportWordTemplatesSheet::OnEditingFinishedWordTemplateCategoryTree(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		ASSERT(pRow != NULL);

		SelectedCategory *sc = (SelectedCategory*)VarLong(pRow->GetValue(ctcPointer));

		switch(nCol)
		{
			case ctcCheckbox:
				sc->bSelected = VarBool(varNewValue);
				if(pRow->GetParentRow() == NULL)
				{
					// (z.manning, 07/05/2007) - If this is a parent row, let's update the kiddies as well.
					for(IRowSettingsPtr pChild = pRow->GetFirstChildRow(); pChild != NULL; pChild = pChild->GetNextRow())
					{
						pChild->PutValue(ctcCheckbox, varNewValue);
						SelectedCategory *scChild = (SelectedCategory*)VarLong(pChild->GetValue(ctcPointer));
						scChild->bSelected = VarBool(varNewValue);
					}
					ShowTemplateByType(sc->strName, VarBool(varNewValue));
				}
				else
				{
					// (z.manning, 07/05/2007) - If it's a child row and it's now selected, ensure the parent
					// row is also selected.
					if(VarBool(varNewValue)) {
						pRow->GetParentRow()->PutValue(ctcCheckbox, varNewValue);
					}
					ShowTemplateByCategory(sc->strName, VarBool(varNewValue));
				}
				break;
		}

	}NxCatchAll("CNexFormsImportWordTemplatesSheet::OnEditingFinishedWordTemplateCategoryTree");
}

void CNexFormsImportWordTemplatesSheet::SelectNonExistingTemplateNames()
{
	// (z.manning, 07/06/2007) - Go through each template in the list and see if a file with
	// the same name already exists on the server.  If so, unselect it.
	for(IRowSettingsPtr pRow = m_pdlWordTemplates->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		NexFormsWordTemplate *word = (NexFormsWordTemplate*)VarLong(pRow->GetValue(wtcPointer));
		
		if(FileUtils::DoesFileOrDirExist(GetTemplatePath(word->strType, word->strFilename)))
		{
			word->bImport = FALSE;
			word->bAlreadyExists = TRUE;
			pRow->PutValue(wtcCheckbox, _variant_t(VARIANT_FALSE, VT_BOOL));
			// (z.manning, 07/09/2007) - Let's color these rows red as well.
			pRow->PutCellForeColor(wtcName, RGB(255,0,0));
		}
		else
		{
			word->bImport = TRUE;
			word->bAlreadyExists = FALSE;
			pRow->PutValue(wtcCheckbox, _variant_t(VARIANT_TRUE, VT_BOOL));
		}
	}
}

void CNexFormsImportWordTemplatesSheet::ShowTemplateByCategory(CString strCategory, BOOL bShow)
{
	// (z.manning, 07/06/2007) - Go through every row in the word template list and if it
	// matches the given category name, then show/hide it.
	for(IRowSettingsPtr pRow = m_pdlWordTemplates->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		NexFormsWordTemplate *word = (NexFormsWordTemplate*)VarLong(pRow->GetValue(wtcPointer));
		if(word->strCategory == strCategory) {
			word->bImportCategory = bShow;
		}
		UpdateRowVisibility(pRow);
	}
}

void CNexFormsImportWordTemplatesSheet::ShowTemplateByType(CString strType, BOOL bShow)
{
	// (z.manning, 07/06/2007) - Go through every row in the word template list and if it
	// matches the given type, then show/hide it.
	for(IRowSettingsPtr pRow = m_pdlWordTemplates->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		NexFormsWordTemplate *word = (NexFormsWordTemplate*)VarLong(pRow->GetValue(wtcPointer));
		if(word->strType == strType) {
			word->bImportCategory = bShow;
		}
		UpdateRowVisibility(pRow);
	}
}

void CNexFormsImportWordTemplatesSheet::OnSelectAllPackets() 
{
	try
	{
		SelectAllPackets(TRUE);

	}NxCatchAll("CNexFormsImportWordTemplatesSheet::OnSelectAllPackets");
}

void CNexFormsImportWordTemplatesSheet::OnUnselectAllPackets() 
{
	try
	{
		SelectAllPackets(FALSE);

	}NxCatchAll("CNexFormsImportWordTemplatesSheet::OnUnselectAllPackets");
}

void CNexFormsImportWordTemplatesSheet::OnSelectAllTemplates() 
{
	try
	{
		SelectAllWordTemplates(TRUE);

	}NxCatchAll("CNexFormsImportWordTemplatesSheet::OnSelectAllTemplates");
}

void CNexFormsImportWordTemplatesSheet::OnUnselectAllTemplates() 
{
	try
	{
		SelectAllWordTemplates(FALSE);

	}NxCatchAll("CNexFormsImportWordTemplatesSheet::OnUnselectAllTemplates");
}

void CNexFormsImportWordTemplatesSheet::SelectAllWordTemplates(BOOL bSelect)
{
	for(IRowSettingsPtr pRow = m_pdlWordTemplates->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		pRow->PutValue(wtcCheckbox, bSelect ? _variant_t(VARIANT_TRUE,VT_BOOL) : _variant_t(VARIANT_FALSE,VT_BOOL));
		NexFormsWordTemplate *word = (NexFormsWordTemplate*)VarLong(pRow->GetValue(wtcPointer));
		word->bImport = bSelect;
	}
}

void CNexFormsImportWordTemplatesSheet::SelectAllPackets(BOOL bSelect)
{
	for(IRowSettingsPtr pRow = m_pdlPackets->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		pRow->PutValue(pcCheckbox, bSelect ? _variant_t(VARIANT_TRUE,VT_BOOL) : _variant_t(VARIANT_FALSE,VT_BOOL));
		NexFormsPacket *packet = (NexFormsPacket*)VarLong(pRow->GetValue(pcPointer));
		packet->bImport = bSelect;
	}
}

BOOL CNexFormsImportWordTemplatesSheet::DoesPacketExistInList(LPDISPATCH lpDatalist, short nCol, CString strPacketName, LPDISPATCH lpRowToIgnore)
{
	_DNxDataListPtr pdl(lpDatalist);
	IRowSettingsPtr pRowToIgnore(lpRowToIgnore);

	for(IRowSettingsPtr pRow = pdl->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		CString strPacketNameToCompare = VarString(pRow->GetValue(nCol));
		if(strPacketNameToCompare.CompareNoCase(strPacketName) == 0 && pRow != pRowToIgnore) {
			return TRUE;
		}
	}

	return FALSE;
}

void CNexFormsImportWordTemplatesSheet::SelectNonExistingPackets()
{
	// (z.manning, 07/09/2007) - Go through every packet available for import and compare it against
	// all existing packets and then select all that don't already match an existing one.
	for(IRowSettingsPtr pImportRow = m_pdlPackets->GetFirstRow(); pImportRow != NULL; pImportRow = pImportRow->GetNextRow())
	{
		CString strPacket = VarString(pImportRow->GetValue(pcName));
		BOOL bPacketExists = DoesPacketExistInList(m_pdlExistingPackets, epcName, strPacket, NULL);
		pImportRow->PutValue(pcCheckbox, bPacketExists ? _variant_t(VARIANT_FALSE,VT_BOOL) : _variant_t(VARIANT_TRUE,VT_BOOL));
		NexFormsPacket *packet = (NexFormsPacket*)VarLong(pImportRow->GetValue(pcPointer));
		packet->bImport = !bPacketExists;
	}
}

void CNexFormsImportWordTemplatesSheet::OnEditingFinishingPacketList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		NexFormsPacket *packet = (NexFormsPacket*)VarLong(pRow->GetValue(pcPointer));

		switch(nCol)
		{
			case pcName:
			{
				CString strNewName = VarString(*pvarNewValue);
				strNewName.TrimLeft();
				strNewName.TrimRight();
				if(strNewName.IsEmpty()) {
					MessageBox("You may not have blank packet names.");
					*pbContinue = FALSE;
				}
				else if(DoesPacketExistInList(m_pdlPackets, pcName, strNewName, pRow)) {
					MessageBox(FormatString("There's already a packet named '%s.'", strNewName));
					*pbContinue = FALSE;
				}
				else if(strNewName.GetLength() >= 50) {
					MessageBox("The packet name is too long. Please shorten it.");
					*pbContinue = FALSE;
				}
			}
			break;
		}

	}NxCatchAll("CNexFormsImportWordTemplatesSheet::OnEditingFinishingPacketList");
}

void CNexFormsImportWordTemplatesSheet::OnEditingFinishedPacketList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL || !bCommit) {
			return;
		}

		NexFormsPacket *packet = (NexFormsPacket*)VarLong(pRow->GetValue(pcPointer));

		switch(nCol)
		{
			case pcCheckbox:
				packet->bImport = VarBool(varNewValue);
				break;

			case pcName:
				packet->strName = Trim(VarString(varNewValue));
				break;
		}

	}NxCatchAll("CNexFormsImportWordTemplatesSheet::OnEditingFinishedPacketList");
}

void CNexFormsImportWordTemplatesSheet::OnRButtonDownPacketList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);

		CMenu mnu;
		mnu.CreatePopupMenu();

		enum PacketListMenuOptions
		{
			plmoRename = 1,
		};

		if(pRow != NULL) {
			m_pdlPackets->PutCurSel(pRow);
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, plmoRename, "Rename");
		}

		CPoint pt(x, y);
		CWnd *pWnd = GetDlgItem(IDC_PACKET_LIST);
		if(pWnd != NULL) {
			pWnd->ClientToScreen(&pt);
		}

		int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, pt.x, pt.y, this, NULL);

		switch(nResult)
		{
			case plmoRename:
				m_pdlPackets->StartEditing(pRow, pcName);
				break;
		}

	}NxCatchAll("CNexFormsImportWordTemplatesSheet::OnRButtonDownPacketList");
}

BOOL CNexFormsImportWordTemplatesSheet::Validate()
{
	// (a.walling 2007-11-06 17:35) - PLID 27998 - VS2008 - for() loops
	IRowSettingsPtr pRow = NULL;
	// (z.manning, 07/25/2007) - Are there any packets that we aren't importing that we need to?
	for(pRow = m_pdlPackets->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		NexFormsPacket *packet = (NexFormsPacket*)VarLong(pRow->GetValue(pcPointer));
		if(!packet->bImport)
		{
			// (z.manning, 07/24/2007) - The packet is not set to be imported. Does it need to be?
			if(packet->bUsedInTrackingLadder && !DoesPacketExistInList(m_pdlExistingPackets, epcName, packet->strName, NULL)) {
				int nResult = MessageBox(FormatString(
					"The packet '%s' is used on tracking ladder(s) that are being imported. You must import this packet "
					"in order to continue.\r\n\r\nWould you like to import the '%s' packet?", packet->strName, packet->strName),
					"Import packet?", MB_YESNO);
				if(nResult == IDYES) {
					packet->bImport = TRUE;
					pRow->PutValue(pcCheckbox, _variant_t(VARIANT_TRUE, VT_BOOL));
				}
				else {
					return FALSE;
				}
			}
		}
	}

	CArray<IRowSettings*,IRowSettings*> arypNeededTemplateRows;
	for(pRow = m_pdlPackets->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		NexFormsPacket *packet = (NexFormsPacket*)VarLong(pRow->GetValue(pcPointer));
		if(packet->bImport)
		{
			// (z.manning, 07/09/2007) - Don't allow an existing packet name to be duplicated.
			if(DoesPacketExistInList(m_pdlExistingPackets, epcName, VarString(pRow->GetValue(pcName)), NULL)) {
				MessageBox(FormatString("You already have an existing packet named '%s.' Please rename or unselect it.",VarString(pRow->GetValue(pcName))));
				return FALSE;
			}

			// (z.manning, 07/16/2007) - If we're importing a packet, make sure we're either importing all of its word
			// templates or that they already exist.
			for(int i = 0; i < packet->m_arypComponents.GetSize(); i++)
			{
				NexFormsPacketComponent *component = packet->m_arypComponents.GetAt(i);

				// (z.manning, 07/16/2007) - See if the file already exists.
				if(!FileUtils::DoesFileOrDirExist(GetSharedPath() ^ component->strPath))
				{
					// (z.manning, 07/16/2007) - Now check the import list if it doesn't exist.
					for(IRowSettingsPtr pTemplateRow = m_pdlWordTemplates->FindAbsoluteFirstRow(VARIANT_FALSE); pTemplateRow != NULL; pTemplateRow = m_pdlWordTemplates->FindAbsoluteNextRow(pTemplateRow, VARIANT_FALSE))
					{
						NexFormsWordTemplate *word = (NexFormsWordTemplate*)VarLong(pTemplateRow->GetValue(wtcPointer));
						if(word->strFilename.CompareNoCase(component->strTemplateName) == 0) {
							if(word->ShouldImport()) {
								// (z.manning, 07/16/2007) - We found it and we're importing it. Good.
							}
							else {
								// (z.manning, 07/16/2007) - We found it but we're not importing it, add it to the list
								// and we handle these a bit later.
								arypNeededTemplateRows.Add(pTemplateRow);
							}
							break;
						}

						// (z.manning, 07/16/2007) - If we hit this assertion, it means there's a packet that 
						// references a word template that is not part of the content. Shouldn't ever happen,
						// but if it does, it almost certainly means something is messed up in the content.
						ASSERT(pTemplateRow != m_pdlWordTemplates->FindAbsoluteLastRow(VARIANT_FALSE));
					}
				}
			}
		}
	}

	// (z.manning, 07/24/2007) - Go through every word template word and make sure we're not missing any
	// that are necessary for other things (such as ladders).
	CStringArray arystrTemplatesToBeOverwritten;
	for(IRowSettingsPtr pTemplateRow = m_pdlWordTemplates->FindAbsoluteFirstRow(VARIANT_FALSE); pTemplateRow != NULL; pTemplateRow = m_pdlWordTemplates->FindAbsoluteNextRow(pTemplateRow, VARIANT_FALSE))
	{
		NexFormsWordTemplate *word = (NexFormsWordTemplate*)VarLong(pTemplateRow->GetValue(wtcPointer));
		if(!word->ShouldImport() && !word->bAlreadyExists && word->bUsedInTrackingLadder) {
			arypNeededTemplateRows.Add(pTemplateRow);
		}

		// (z.manning, 07/31/2007) - Also find any templates that already exist that are marked to be imported.
		if(word->bAlreadyExists && word->ShouldImport()) {
			// (z.manning 2009-10-22 12:44) - PLID 36033 - If this is manadatory template for import then
			// don't warn about it-- we'll just silently import it later.
			if(!MustImportWordTemplate(word->strFilename)) {
				arystrTemplatesToBeOverwritten.Add(word->strFilename);
			}
		}
	}

	// (z.manning, 07/16/2007) - Did we find any word templates that weren't imported that we need?
	if(arypNeededTemplateRows.GetSize() > 0)
	{
		CString strMessage = "The following Word template(s) are part of packets or tracking ladders you are importing, but are not set "
			"to be imported themselves. Would you like to import these Word templates?\r\n";
		// (z.manning, 07/16/2007) - Prepare the message box.
		for(int i = 0; i < arypNeededTemplateRows.GetSize(); i++) {
			IRowSettingsPtr pTemplateRow = arypNeededTemplateRows.GetAt(i);
			strMessage += FormatString("\r\n  -- %s", VarString(pTemplateRow->GetValue(wtcName), ""));
		}
		if(IDYES == MessageBox(strMessage, "Import templates?", MB_YESNO)) {
			// (z.manning, 07/16/2007) - They wan't to import them, so reflect that in the list.
			for(i = 0; i < arypNeededTemplateRows.GetSize(); i++) {
				IRowSettingsPtr pTemplateRow = arypNeededTemplateRows.GetAt(i);
				NexFormsWordTemplate *word = (NexFormsWordTemplate*)VarLong(pTemplateRow->GetValue(wtcPointer));
				word->bImport = TRUE;
				word->bImportCategory = TRUE;
				word->bImportGroup = TRUE;
				pTemplateRow->PutValue(wtcCheckbox, _variant_t(VARIANT_TRUE, VT_BOOL));
			}
		}
		else {
			return FALSE;
		}
	}

	// (z.manning, 07/31/2007) - If we're going to be overwriting any templates, warn the user.
	if(arystrTemplatesToBeOverwritten.GetSize() > 0)
	{
		CString strMessage;
		// (z.manning, 07/31/2007) - If they are overwriting a lot of templates, just give a generic warning
		// as we don't want to message box going off the screen.
		if(arystrTemplatesToBeOverwritten.GetSize() > 30) {
			strMessage = FormatString("If you continue, %li Word templates will be overwritten. Are you sure you want "
				"to continue with the import?", arystrTemplatesToBeOverwritten.GetSize());
		}
		else {
			strMessage = "The following Word template(s) already exist and will be overwritten during the import:\r\n";
			for(int i = 0; i < arystrTemplatesToBeOverwritten.GetSize(); i++) {
				strMessage += "\r\n-- " + arystrTemplatesToBeOverwritten.GetAt(i);
			}
			strMessage += "\r\n\r\nAre you sure you want to continue with the import?";

		}
		
		if(MessageBox(strMessage, "Overwrite templates?", MB_YESNO) != IDYES) {
			return FALSE;
		}
	}

	return TRUE;
}

void CNexFormsImportWordTemplatesSheet::OnSelChangedGroupFilter(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try
	{
		IRowSettingsPtr pNewSel(lpNewSel);
		if(pNewSel == NULL) {
			return;
		}

		// (z.manning, 07/17/2007) - Remember this value.
		CString strGroup = VarString(pNewSel->GetValue(gfcName));
		SetRemotePropertyText("NexFormsImportTemplateGroup", strGroup, 0, "<None>");
		HandleGroupChange();

	}NxCatchAll("CNexFormsImportWordTemplatesSheet::OnSelChangedGroupFilter");
}

void CNexFormsImportWordTemplatesSheet::UpdateRowVisibility(IRowSettingsPtr pRow)
{
	NexFormsWordTemplate *word = (NexFormsWordTemplate*)VarLong(pRow->GetValue(wtcPointer));
	if(MustImportWordTemplate(word->strFilename)) {
		// (z.manning 2009-10-22 12:14) - PLID 36033 - This template MUST be imported so don't
		// even show it in the list so we can just silently import it in the background later on.
		pRow->PutVisible(VARIANT_FALSE);
	}
	else {
		pRow->PutVisible((word->bImportCategory && word->bImportGroup) ? _variant_t(VARIANT_TRUE,VT_BOOL) : _variant_t(VARIANT_FALSE,VT_BOOL));
	}
}

void CNexFormsImportWordTemplatesSheet::HandleGroupChange()
{
	IRowSettingsPtr pGroupRow = m_pdlGroupFilter->GetCurSel();
	CString strSelectedGroup;
	if(pGroupRow != NULL) {
		strSelectedGroup = VarString(pGroupRow->GetValue(gfcName), "");
	}

	for(IRowSettingsPtr pTemplateRow = m_pdlWordTemplates->GetFirstRow(); pTemplateRow != NULL; pTemplateRow = pTemplateRow->GetNextRow())
	{
		NexFormsWordTemplate *word = (NexFormsWordTemplate*)VarLong(pTemplateRow->GetValue(wtcPointer));

		// (z.manning, 07/17/2007) - The group "{ All }" means we don't want to filter out any templates.
		// So, if we have that selection, no selection, or a matching selecting, then we want to set this
		// row to be visible (as long as nothing else causes it to hide).
		if(pGroupRow == NULL || strSelectedGroup == "{ All }" || word->strGroup == "{ All }" 
			|| strSelectedGroup.CompareNoCase(word->strGroup) == 0) 
		{
			word->bImportGroup = TRUE;
		}
		else {
			word->bImportGroup = FALSE;
		}
		UpdateRowVisibility(pTemplateRow);
	}
}

// (z.manning, 07/19/2007) - PLID 26746 - Added right click handler for the template list.
void CNexFormsImportWordTemplatesSheet::OnRButtonDownWordTemplateList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);

		CMenu mnu;
		mnu.CreatePopupMenu();

		enum MenuOptions
		{
			moPreview = 1,
		};

		if(lpRow != NULL) {
			m_pdlWordTemplates->PutCurSel(pRow);
			// (z.manning, 07/19/2007) - PLID 26746 - Added an option to preview Word templates.
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, moPreview, "Preview");
		}

		CPoint pt(x, y);
		CWnd *pWnd = GetDlgItem(IDC_WORD_TEMPLATE_LIST);
		if(pWnd != NULL) {
			pWnd->ClientToScreen(&pt);
		}

		int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, pt.x, pt.y, this, NULL);

		switch(nResult)
		{
			case moPreview:
				OpenWordTemplateFromRow(lpRow);
				break;
		}

	}NxCatchAll("CNexFormsImportWordTemplatesSheet::OnRButtonDownWordTemplateList");
}

// (z.manning, 07/19/2007) - PLID 26746 - Opens a word template from the given row from the word template list.
void CNexFormsImportWordTemplatesSheet::OpenWordTemplateFromRow(LPDISPATCH lpRow)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		return;
	}

	CWaitCursor wc;

	NexFormsWordTemplate *word = (NexFormsWordTemplate*)VarLong(pRow->GetValue(wtcPointer));
	
	// (z.manning, 07/19/2007) - PLID 26746 - Make sure they have permission to view templates.
	if (!CheckCurrentUserPermissions(bioLWEditTemplate, sptView)) {
		return;
	}

	// (z.manning, 07/19/2007) - PLID 26746 - Make sure Word is installed.
	if (!GetWPManager()->CheckWordProcessorInstalled()) {
		return;
	}

	CString strMergeInfoPath;
	// (z.manning 2016-02-12 13:57) - PLID 68230 - Use the base word processor app class
	std::shared_ptr<CGenericWordProcessorApp> pApp = GetWPManager()->GetAppInstance();
	if (nullptr == pApp) return; // (c.haag 2016-06-01 11:48) - NX-100320 - If it's null then it's not supported and an exception was not thrown
	pApp->EnsureValid();

	// (z.manning, 07/19/2007) - PLID 26746 - Same flags that PIC container uses.
	long nFlags = BMS_HIDE_ALL_DATA	| BMS_DEFAULT | /*BMS_HIDE_PRACTICE_INFO |*/
		/*BMS_HIDE_PERSON_INFO | BMS_HIDE_DATE_INFO | BMS_HIDE_PRESCRIPTION_INFO |*/
		/*BMS_HIDE_CUSTOM_INFO | BMS_HIDE_INSURANCE_INFO |*/ BMS_HIDE_BILL_INFO /*|*/
		/*BMS_HIDE_PROCEDURE_INFO | BMS_HIDE_DOCTOR_INFO*/;

	strMergeInfoPath = CMergeEngine::CreateBlankMergeInfo(nFlags, NULL, NULL);
	if(!strMergeInfoPath.IsEmpty())
	{
		// (z.manning, 07/19/2007) - PLID 26746 - Open the template
		// (z.manning 2016-02-18 09:18) - PLID 68366 - This now returns a word processor document object
		// (c.haag 2016-02-23) - PLID 68416 - We no longer catch Word-specific exceptions here. Those are now managed deep within the WordProcessor application object
		// (c.haag 2016-04-22 10:40) - NX-100275 - OpenTemplate no longer returns a document. We never did anything with it except throw an exception if it were null
		// anyway, and now OpenTemplate does that for us
		pApp->OpenTemplate(word->strTempFileFullPath, strMergeInfoPath);
					
		// We can't delete the merge info text file right now because it is in use, but 
		// it's a temp file so mark it to be deleted after the next reboot
		DeleteFileWhenPossible(strMergeInfoPath);
		strMergeInfoPath.Empty();
	}
	else {
		AfxThrowNxException("Failed to create merge info");
	}

	if (!strMergeInfoPath.IsEmpty()) {
		// This means the file wasn't used and/or it wasn't marked for deletion at startup, so delete it now
		DeleteFile(strMergeInfoPath);
	}
}

void CNexFormsImportWordTemplatesSheet::OnEditingFinishedWordTemplateList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		NexFormsWordTemplate *word = (NexFormsWordTemplate*)VarLong(pRow->GetValue(wtcPointer));

		switch(nCol)
		{
			case pcCheckbox:
				word->bImport = VarBool(varNewValue);
				break;
		}

	}NxCatchAll("CNexFormsImportWordTemplatesSheet::OnEditingFinishedWordTemplateList");
}
