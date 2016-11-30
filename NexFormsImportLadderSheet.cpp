// NexFormsImportLadderSheet.cpp : implementation file
//

#include "stdafx.h"
#include "letterwriting.h"
#include "NexFormsImportLadderSheet.h"
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

// (z.manning, 06/25/2007) - This enum applies to ALL 3 datalists on this dialog. So if you change
// the number of columns in any of the datalists, make sure you keep all 3 the same.
enum LadderListColumns
{
	llcID = 0,
	llcName,
	llcLadderPointer,
};

/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportLadderSheet dialog


// (z.manning, 08/30/2007) - PLID 18359 - Created a new importer for NexForms.

CNexFormsImportLadderSheet::CNexFormsImportLadderSheet(CNexFormsImportWizardMasterDlg *pParent)
	: CNexFormsImportWizardSheet(CNexFormsImportLadderSheet::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNexFormsImportLadderSheet)	
	//}}AFX_DATA_INIT
}


void CNexFormsImportLadderSheet::DoDataExchange(CDataExchange* pDX)
{
	CNexFormsImportWizardSheet::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexFormsImportLadderSheet)	
	DDX_Control(pDX, IDC_REMOVE_ALL_LADDERS, m_btnRemoveAllLadders);
	DDX_Control(pDX, IDC_REMOVE_LADDER, m_btnRemoveLadder);
	DDX_Control(pDX, IDC_ADD_ALL_LADDERS, m_btnAddAllLadders);
	DDX_Control(pDX, IDC_ADD_LADDER, m_btnAddLadder);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexFormsImportLadderSheet, CNexFormsImportWizardSheet)
	//{{AFX_MSG_MAP(CNexFormsImportLadderSheet)
	ON_BN_CLICKED(IDC_ADD_LADDER, OnAddLadder)
	ON_BN_CLICKED(IDC_ADD_ALL_LADDERS, OnAddAllLadders)
	ON_BN_CLICKED(IDC_REMOVE_LADDER, OnRemoveLadder)
	ON_BN_CLICKED(IDC_REMOVE_ALL_LADDERS, OnRemoveAllLadders)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CNexFormsImportLadderSheet, CNexFormsImportWizardSheet)
    //{{AFX_EVENTSINK_MAP(CNexFormsImportLadderSheet)
	ON_EVENT(CNexFormsImportLadderSheet, IDC_AVAILABLE_LADDERS, 3 /* DblClickCell */, OnDblClickCellAvailableLadders, VTS_DISPATCH VTS_I2)
	ON_EVENT(CNexFormsImportLadderSheet, IDC_LADDERS_TO_IMPORT, 3 /* DblClickCell */, OnDblClickCellLaddersToImport, VTS_DISPATCH VTS_I2)
	ON_EVENT(CNexFormsImportLadderSheet, IDC_AVAILABLE_LADDERS, 6 /* RButtonDown */, OnRButtonDownAvailableLadders, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CNexFormsImportLadderSheet, IDC_LADDERS_TO_IMPORT, 6 /* RButtonDown */, OnRButtonDownLaddersToImport, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CNexFormsImportLadderSheet, IDC_AVAILABLE_LADDERS, 9 /* EditingFinishing */, OnEditingFinishingAvailableLadders, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CNexFormsImportLadderSheet, IDC_AVAILABLE_LADDERS, 10 /* EditingFinished */, OnEditingFinishedAvailableLadders, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CNexFormsImportLadderSheet, IDC_LADDERS_TO_IMPORT, 9 /* EditingFinishing */, OnEditingFinishingLaddersToImport, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CNexFormsImportLadderSheet, IDC_LADDERS_TO_IMPORT, 10 /* EditingFinished */, OnEditingFinishedLaddersToImport, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CNexFormsImportLadderSheet, IDC_EXISTING_LADDERS, 18 /* RequeryFinished */, OnRequeryFinishedExistingLadders, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportLadderSheet message handlers

BOOL CNexFormsImportLadderSheet::OnInitDialog() 
{
	try
	{
		CNexFormsImportWizardSheet::OnInitDialog();

		m_btnAddLadder.AutoSet(NXB_RIGHT);
		m_btnAddAllLadders.AutoSet(NXB_RRIGHT);
		m_btnRemoveLadder.AutoSet(NXB_LEFT);
		m_btnRemoveAllLadders.AutoSet(NXB_LLEFT);

		m_pdlExistingLadders = BindNxDataList2Ctrl(this, IDC_EXISTING_LADDERS, GetRemoteData(), true);
		m_pdlAvailableLadders = BindNxDataList2Ctrl(this, IDC_AVAILABLE_LADDERS, GetRemoteData(), false);
		m_pdlLaddersToImport = BindNxDataList2Ctrl(this, IDC_LADDERS_TO_IMPORT, GetRemoteData(), false);

	}NxCatchAll("CNexFormsImportLadderSheet::OnInitDialog");	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNexFormsImportLadderSheet::Load()
{
	// (z.manning, 06/25/2007) - If we have no records in either list, then we assume this is the first load.
	BOOL bInitialLoad = FALSE;
	if(m_pdlAvailableLadders->GetRowCount() == 0 && m_pdlLaddersToImport->GetRowCount() == 0)
	{
		bInitialLoad = TRUE;
	}
	else
	{
		// (z.manning, 06/26/2007) - Clear the lists since we're about to reload everything.
		m_pdlAvailableLadders->Clear();
		m_pdlLaddersToImport->Clear();
	}

	for(int i = 0; i < m_pdlgMaster->m_ImportInfo.m_arypLadders.GetSize(); i++)
	{
		// (z.manning, 06/26/2007) - For each ladder from the content file, find out if it's been set to import
		// and then add it to the appropriate list.
		IRowSettingsPtr pRow;
		BOOL bImport = m_pdlgMaster->m_ImportInfo.m_arypLadders.GetAt(i)->bImport;
		if(bImport)
		{
			pRow = m_pdlLaddersToImport->GetNewRow();
		}
		else
		{
			pRow = m_pdlAvailableLadders->GetNewRow();
		}
		pRow->PutValue(llcID, m_pdlgMaster->m_ImportInfo.m_arypLadders.GetAt(i)->nID);
		pRow->PutValue(llcName, _bstr_t(m_pdlgMaster->m_ImportInfo.m_arypLadders.GetAt(i)->strName));
		pRow->PutValue(llcLadderPointer, (long)m_pdlgMaster->m_ImportInfo.m_arypLadders.GetAt(i));
		if(bImport)
		{
			m_pdlLaddersToImport->AddRowSorted(pRow, NULL);
		}
		else
		{
			m_pdlAvailableLadders->AddRowSorted(pRow, NULL);
		}
	}

	// (z.manning, 06/26/2007) - If this is the first load, then let's select certain ladders by
	// default based on what ladders they already have.
	if(bInitialLoad)
	{
		FillDefaultLaddersToImport();
	}
}

void CNexFormsImportLadderSheet::FillDefaultLaddersToImport()
{
	// (z.manning, 06/25/2007) - Find all ladders that don't match the name of a current ladder and 
	// put those in the ladders to import list.
	ASSERT(m_pdlLaddersToImport->GetRowCount() == 0);
	IRowSettingsPtr pRow = m_pdlAvailableLadders->GetFirstRow();
	while(pRow != NULL)
	{
		IRowSettingsPtr pNextRow = pRow->GetNextRow();
		IRowSettingsPtr pExistingLadderRow = m_pdlExistingLadders->FindByColumn(llcName, pRow->GetValue(llcName), NULL, VARIANT_FALSE);
		if(NULL == pExistingLadderRow)
		{
			// (z.manning, 06/25/2007) - Ok, we don't already have a ladder with this name, let's move
			// it to the ladders to import list.
			MoveRowFromAvailableListToImportList(pRow);
		}
		else
		{
			// (z.manning, 08/30/2007) - Only allow overwriting ladders if it's safe to delete the existing one.
			// (i.e. it's not used anywhere).
			if(PhaseTracking::CanDeleteLadderTemplate(VarLong(pExistingLadderRow->GetValue(llcID)), TRUE, this)) {
				MoveRowFromAvailableListToImportList(pRow);
			}
		}
		pRow = pNextRow;
	}
}

void CNexFormsImportLadderSheet::MoveRowFromAvailableListToImportList(IRowSettingsPtr pRow)
{
	// (z.manning, 06/25/2007) - Add a copy of this row to the import list.
	IRowSettingsPtr pNewRow = m_pdlLaddersToImport->GetNewRow();
	pNewRow->PutValue(llcID, pRow->GetValue(llcID));
	pNewRow->PutValue(llcName, pRow->GetValue(llcName));
	pNewRow->PutValue(llcLadderPointer, pRow->GetValue(llcLadderPointer));
	m_pdlLaddersToImport->AddRowSorted(pNewRow, NULL);

	// (z.manning, 06/26/2007) - Let the object know we're importing it.
	NexFormsLadder *ladder = (NexFormsLadder*)VarLong(pNewRow->GetValue(llcLadderPointer));
	ladder->bImport = TRUE;

	// (z.manning, 06/25/2007) - Then remove it from the available list.
	m_pdlAvailableLadders->RemoveRow(pRow);
}

void CNexFormsImportLadderSheet::MoveRowFromImportListToAvailableList(IRowSettingsPtr pRow)
{
	// (z.manning, 06/25/2007) - Add a copy of this row to the import list.
	IRowSettingsPtr pNewRow = m_pdlAvailableLadders->GetNewRow();
	pNewRow->PutValue(llcID, pRow->GetValue(llcID));
	pNewRow->PutValue(llcName, pRow->GetValue(llcName));
	pNewRow->PutValue(llcLadderPointer, pRow->GetValue(llcLadderPointer));
	m_pdlAvailableLadders->AddRowSorted(pNewRow, NULL);

	// (z.manning, 06/26/2007) - Let the object know we're NOT importing it.
	NexFormsLadder *ladder = (NexFormsLadder*)VarLong(pNewRow->GetValue(llcLadderPointer));
	ladder->bImport = FALSE;

	// (z.manning, 06/25/2007) - Then remove it from the available list.
	m_pdlLaddersToImport->RemoveRow(pRow);
}

BOOL CNexFormsImportLadderSheet::Validate()
{
	CStringArray arystrLaddersToOverwrite;
	// (a.walling 2007-11-06 17:35) - PLID 27998 - VS2008 - for() loops
	IRowSettingsPtr pRow = NULL;
	for(pRow = m_pdlLaddersToImport->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		NexFormsLadder *ladder = (NexFormsLadder*)VarLong(pRow->GetValue(llcLadderPointer));

		CString strCurrentLadder = VarString(pRow->GetValue(llcName),"");
		strCurrentLadder.TrimLeft();
		strCurrentLadder.TrimRight();

		// (z.manning, 06/26/2007) - Do not allow them to import a ladder with the same name as one they already have.
		for(IRowSettingsPtr pExistingRow = m_pdlExistingLadders->GetFirstRow(); pExistingRow != NULL; pExistingRow = pExistingRow->GetNextRow())
		{
			CString str = VarString(pExistingRow->GetValue(llcName),"");
			str.TrimLeft();
			str.TrimRight();
			if(strCurrentLadder.CompareNoCase(str) == 0)
			{
				// (z.manning, 08/30/2007) - As long as this existing ladder has never been used, we can overwrite it.
				if(PhaseTracking::CanDeleteLadderTemplate(VarLong(pExistingRow->GetValue(llcID)), TRUE, this)) {
					// (z.manning, 08/30/2007) - Ok, it's safe to delete the existing ladder of the same name,
					// but don't do it quite yet because who knows if we'll end up committing the import or not.
					// Instead, let's just set a flag so that when we do import, we know we need to delete a ladder
					// with the same name.
					ladder->bDeleteExistingLadder = TRUE;
					arystrLaddersToOverwrite.Add(ladder->strName);
				}
				else {
					MessageBox( FormatString("You already have a ladder named '%s.' Please rename the new ladder or move it "
						"out of the list of ladders to import.", VarString(pRow->GetValue(llcName),"")) );
					return FALSE;
				}
			}
		}

		// (z.manning, 06/26/2007) - Also, make sure they can't import multiple rows with the same name.
		for(IRowSettingsPtr pRow2 = m_pdlLaddersToImport->GetFirstRow(); pRow2 != NULL; pRow2 = pRow2->GetNextRow())
		{
			CString str = VarString(pRow2->GetValue(llcName),"");
			str.TrimLeft();
			str.TrimRight();
			if(pRow2 != pRow && strCurrentLadder.CompareNoCase(str) == 0)
			{
				MessageBox( FormatString("You may not import more than one ladder named '%s.'", VarString(pRow->GetValue(llcName),"")) );
				return FALSE;
			}
		}

		// (z.manning, 07/24/2007) - Go through each step and check its actions to make sure we force any
		// dependencies to be imported as well.
		UpdateDependentActions(ladder, TRUE);
	}

	// (z.manning, 08/30/2007) - If we're going to be overwriting any ladders, warn the user.
	if(arystrLaddersToOverwrite.GetSize() > 0)
	{
		CString strMessage = "The following ladder(s) already exist in your system, but since they are not "
			"tied to any patient records you may overwrite them if you want.\r\n\r\n";
		for(int i = 0; i < arystrLaddersToOverwrite.GetSize(); i++) {
			strMessage += "  -- " + arystrLaddersToOverwrite.GetAt(i) + "\r\n";
		}
		strMessage += "\r\nAre you sure you want to overwrite these ladder(s)?";
		if(IDYES != MessageBox(strMessage, "Overwrite ladders?", MB_YESNO)) {
			return FALSE;
		}
	}

	for(pRow = m_pdlAvailableLadders->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		// (z.manning, 07/27/2007) - For all ladders we aren't importing, go through and mark
		// any actions as not needed.
		NexFormsLadder *ladder = (NexFormsLadder*)VarLong(pRow->GetValue(llcLadderPointer));
		UpdateDependentActions(ladder, FALSE);
	}

	return TRUE;
}

void CNexFormsImportLadderSheet::OnAddLadder() 
{
	try
	{
		IRowSettingsPtr pRow = m_pdlAvailableLadders->GetCurSel();
		if(pRow == NULL) {
			return;
		}

		MoveRowFromAvailableListToImportList(pRow);

	}NxCatchAll("CNexFormsImportLadderSheet::OnAddLadder");
}

void CNexFormsImportLadderSheet::OnAddAllLadders() 
{
	try
	{
		IRowSettingsPtr pRow = m_pdlAvailableLadders->GetFirstRow(); 
		while(pRow != NULL)
		{
			IRowSettingsPtr pNextRow = pRow->GetNextRow();
			MoveRowFromAvailableListToImportList(pRow);
			pRow = pNextRow;
		}

	}NxCatchAll("CNexFormsImportLadderSheet::OnAddAllLadders");
}

void CNexFormsImportLadderSheet::OnRemoveLadder() 
{
	try
	{
		IRowSettingsPtr pRow = m_pdlLaddersToImport->GetCurSel();
		if(pRow == NULL) {
			return;
		}

		MoveRowFromImportListToAvailableList(pRow);

	}NxCatchAll("CNexFormsImportLadderSheet::OnRemoveLadder");	
}

void CNexFormsImportLadderSheet::OnRemoveAllLadders() 
{
	try
	{
		IRowSettingsPtr pRow = m_pdlLaddersToImport->GetFirstRow(); 
		while(pRow != NULL)
		{
			IRowSettingsPtr pNextRow = pRow->GetNextRow();
			MoveRowFromImportListToAvailableList(pRow);
			pRow = pNextRow;
		}

	}NxCatchAll("CNexFormsImportLadderSheet::OnRemoveAllLadders");	
}

void CNexFormsImportLadderSheet::OnDblClickCellAvailableLadders(LPDISPATCH lpRow, short nColIndex) 
{
	try
	{
		MoveRowFromAvailableListToImportList(lpRow);

	}NxCatchAll("CNexFormsImportLadderSheet::OnDblClickCellAvailableLadders");
}

void CNexFormsImportLadderSheet::OnDblClickCellLaddersToImport(LPDISPATCH lpRow, short nColIndex) 
{
	try
	{
		MoveRowFromImportListToAvailableList(lpRow);
	
	}NxCatchAll("CNexFormsImportLadderSheet::OnDblClickCellLaddersToImport");
}


void CNexFormsImportLadderSheet::OnRButtonDownAvailableLadders(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try
	{
		m_pdlAvailableLadders->PutCurSel(IRowSettingsPtr(lpRow));
		PopupContextMenu(m_pdlAvailableLadders, lpRow);

	}NxCatchAll("CNexFormsImportLadderSheet::OnRButtonDownAvailableLadders");
}

void CNexFormsImportLadderSheet::OnRButtonDownLaddersToImport(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try
	{
		m_pdlLaddersToImport->PutCurSel(IRowSettingsPtr(lpRow));
		PopupContextMenu(m_pdlLaddersToImport, lpRow);

	}NxCatchAll("CNexFormsImportLadderSheet::OnRButtonDownLaddersToImport");
}

void CNexFormsImportLadderSheet::OnEditingFinishingAvailableLadders(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol)
		{
			case llcName:
				// (z.manning, 06/26/2007) - Don't allow blank strings.
				CString strNewValue = VarString(*pvarNewValue, "");
				strNewValue.TrimLeft();
				strNewValue.TrimRight();
				if(strNewValue.IsEmpty()) {
					MessageBox("Blank ladder names are not allowed.");
					*pbContinue = FALSE;
				}
				else if(strNewValue.GetLength() >= 50) {
					MessageBox("The ladder name you entered is too long. Please shorten it.");
					*pbContinue = FALSE;
				}
				break;
		}

	}NxCatchAll("CNexFormsImportLadderSheet::OnEditingFinishingAvailableLadders");
}

void CNexFormsImportLadderSheet::OnEditingFinishedAvailableLadders(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL || !bCommit) {
			return;
		}

		switch(nCol)
		{
			case llcName:
				// (z.manning, 06/26/2007) - Update this ladder's name in memory.
				NexFormsLadder *ladder = (NexFormsLadder*)VarLong(pRow->GetValue(llcLadderPointer));
				if(ladder != NULL) {
					ladder->strName = Trim(VarString(varNewValue));
				}
				else {
					// (z.manning, 10/16/2007) - Every row should have an associated ladder. If you hit
					// this assertion, there's likely a bug somewhere else.
					ASSERT(FALSE);
				}
				break;
		}

	}NxCatchAll("CNexFormsImportLadderSheet::OnEditingFinishedAvailableLadders");
}

void CNexFormsImportLadderSheet::OnEditingFinishingLaddersToImport(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try
	{
		// (z.manning, 06/26/2007) - Same behavior as the available ladders list.
		OnEditingFinishingAvailableLadders(lpRow, nCol, varOldValue, strUserEntered, pvarNewValue, pbCommit, pbContinue);

	}NxCatchAll("CNexFormsImportLadderSheet::OnEditingFinishingLaddersToImport");
}

void CNexFormsImportLadderSheet::OnEditingFinishedLaddersToImport(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try
	{
		// (z.manning, 06/26/2007) - Same behavior as the available ladders list.
		OnEditingFinishedAvailableLadders(lpRow, nCol, varOldValue, varNewValue, bCommit);

	}NxCatchAll("CNexFormsImportLadderSheet::OnEditingFinishedLaddersToImport");
}

void CNexFormsImportLadderSheet::PopupContextMenu(LPDISPATCH lpDatalist, LPDISPATCH lpRow)
{
	_DNxDataListPtr pdl(lpDatalist);
	if(pdl == NULL) {
		return;
	}

	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		return;
	}

	enum ELadderPopupMenuOptions
	{
		lpmoRename = 1,
	};

	CMenu mnu;
	mnu.CreatePopupMenu();
	mnu.InsertMenu(0, MF_BYPOSITION, lpmoRename, "Rename");
	
	CPoint pt;
	GetCursorPos(&pt);
	int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, pt.x, pt.y, this);

	switch(nResult)
	{
		case lpmoRename:
			pdl->StartEditing(pRow, llcName);
			break;
	}
}

void CNexFormsImportLadderSheet::OnRequeryFinishedExistingLadders(short nFlags) 
{
	try
	{
		UpdateMasterExistingProcedureArray();

	}NxCatchAll("CNexFormsImportLadderSheet::OnRequeryFinishedExistingLadders");
}

void CNexFormsImportLadderSheet::UpdateMasterExistingProcedureArray()
{
	m_pdlgMaster->m_ImportInfo.CleanUpExistingLadders();

	for(IRowSettingsPtr pRow = m_pdlExistingLadders->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		ExistingLadder *ladder = new ExistingLadder;
		ladder->nID = VarLong(pRow->GetValue(llcID));
		ladder->strName = VarString(pRow->GetValue(llcName), "");
		m_pdlgMaster->m_ImportInfo.m_aryExistingLadders.Add(ladder);
		pRow->PutValue(llcLadderPointer, (long)ladder);
	}
}

// (z.manning, 07/27/2007) - For a given ladder, go through all actions and find anything else we may
// be importing later and mark as either needed or not needed.
void CNexFormsImportLadderSheet::UpdateDependentActions(NexFormsLadder *ladder, BOOL bNeeded)
{
	for(int nStepIndex = 0; nStepIndex < ladder->arypSteps.GetSize(); nStepIndex++)
	{
		for(int nActionIndex = 0; nActionIndex < ladder->arypSteps.GetAt(nStepIndex)->aryvarActions.GetSize(); nActionIndex++)
		{
			switch(ladder->arypSteps.GetAt(nStepIndex)->nAction)
			{
				// (z.manning, 07/24/2007) - For packets, find the packet name and compare it against packets
				// available for import and when we find a match, signify that in that packet.
				case PhaseTracking::PA_WritePacket:
				{
					CString strAction = VarString(ladder->arypSteps.GetAt(nStepIndex)->aryvarActions.GetAt(nActionIndex));
					for(int nPacketIndex = 0; nPacketIndex < m_pdlgMaster->m_ImportInfo.m_arypPackets.GetSize(); nPacketIndex++)
					{
						NexFormsPacket *packet = m_pdlgMaster->m_ImportInfo.m_arypPackets.GetAt(nPacketIndex);
						if(strAction.CompareNoCase(packet->strName) == 0) {
							packet->bUsedInTrackingLadder = bNeeded;
							break;
						}
					}
					ASSERT(nPacketIndex != m_pdlgMaster->m_ImportInfo.m_arypPackets.GetSize());
				}
				break;

				// (z.manning, 07/24/2007) - For word templates, the exporter gives us the relative file path.
				// So let's compare that against our word template list and when we find a match, mark the
				// word template as being used on a ladder.
				case PhaseTracking::PA_WriteTemplate:
				{
					CString strAction = VarString(ladder->arypSteps.GetAt(nStepIndex)->aryvarActions.GetAt(nActionIndex));
					for(int nTemplateIndex = 0; nTemplateIndex < m_pdlgMaster->m_ImportInfo.m_arypWordTemplates.GetSize(); nTemplateIndex++)
					{
						NexFormsWordTemplate *word = m_pdlgMaster->m_ImportInfo.m_arypWordTemplates.GetAt(nTemplateIndex);
						if(strAction.CompareNoCase("\\Templates" ^ word->strType ^ word->strFilename) == 0) {
							word->bUsedInTrackingLadder = bNeeded;
							break;
						}
					}
					ASSERT(nTemplateIndex != m_pdlgMaster->m_ImportInfo.m_arypWordTemplates.GetSize());
				}
				break;
			}
		}
	}
}