// NexFormsImportNewProcedureSheet.cpp : implementation file
//

#include "stdafx.h"
#include "NexFormsImportNewProcedureSheet.h"
#include "NexFormsUtils.h"
#include "MultiSelectDlg.h"

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

// (z.manning, 06/27/2007) - The way we handle ladder IDs is using the actual value for ladder IDs
// in the existing database and using the negative values for IDs in the export file (and zero when
// there isn't a value). This procludes us from using values like -1, however, for special cases.
// So, this dummy value is for the multiple ladders option in the embedded ladder combo in the procedure
// datalist.
#define MULTIPLE_LADDERS_COMBO_ID -9999999

using namespace NXDATALIST2Lib;

enum EProcedureListColumns
{
	plcID = 0,
	plcImport,
	plcProcedure,
	plcExistingProcedure,
	plcMasterProcedure,
	plcProcedurePointer,
	plcLadder,
};

// (z.manning, 06/25/2007) - This function is here so we can use the qsort function for arrays of the 
// ExistingProcedure struct.
int CompareExistingProceduresByName(const void *pDataA, const void *pDataB)
{
	ExistingProcedure **ppProcedureA = (ExistingProcedure**)pDataA;
	ExistingProcedure **ppProcedureB = (ExistingProcedure**)pDataB;

	if(*ppProcedureA != NULL && *ppProcedureB != NULL)
	{
		CString strProcedureNameA = (*ppProcedureA)->strName;
		CString strProcedureNameB = (*ppProcedureB)->strName;
		// (z.manning, 06/25/2007) - Ignore case when comparing procedure names.
		return strProcedureNameA.CompareNoCase(strProcedureNameB);
	}
	else 
	{
		// (z.manning, 06/25/2007) - Uhh, we shouldn't have a null pointer.
		ASSERT(FALSE);
		return -1;
	}
}

// (z.manning, 06/25/2007) - This function is here so we can use the qsort function for arrays of the 
// ExistingLadder struct.
int CompareExistingLaddersByName(const void *pDataA, const void *pDataB)
{
	ExistingLadder **ppLadderA = (ExistingLadder**)pDataA;
	ExistingLadder **ppLadderB = (ExistingLadder**)pDataB;

	if(*ppLadderA != NULL && *ppLadderB != NULL)
	{
		CString strLadderNameA = (*ppLadderA)->strName;
		CString strLadderNameB = (*ppLadderB)->strName;
		// (z.manning, 06/25/2007) - Ignore case when comparing Ladder names.
		return strLadderNameA.CompareNoCase(strLadderNameB);
	}
	else 
	{
		// (z.manning, 06/25/2007) - Uhh, we shouldn't have a null pointer.
		ASSERT(FALSE);
		return -1;
	}
}


// (z.manning, 08/30/2007) - PLID 18359 - Created a new importer for NexForms.
/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportNewProcedureSheet dialog


CNexFormsImportNewProcedureSheet::CNexFormsImportNewProcedureSheet(CNexFormsImportWizardMasterDlg* pParent)
	: CNexFormsImportWizardSheet(CNexFormsImportNewProcedureSheet::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNexFormsImportNewProcedureSheet)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bInitialLoad = TRUE;
}


void CNexFormsImportNewProcedureSheet::DoDataExchange(CDataExchange* pDX)
{
	CNexFormsImportWizardSheet::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexFormsImportNewProcedureSheet)
	DDX_Control(pDX, IDC_SELECT_NO_PROCS, m_btnSelectNone);
	DDX_Control(pDX, IDC_SELECT_ALL_PROCS, m_btnSelectAll);
	DDX_Control(pDX, IDC_SELECT_DERM_ONLY, m_btnSelectDermOnly);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexFormsImportNewProcedureSheet, CNexFormsImportWizardSheet)
	//{{AFX_MSG_MAP(CNexFormsImportNewProcedureSheet)
	ON_BN_CLICKED(IDC_SELECT_ALL_PROCS, OnSelectAll)
	ON_BN_CLICKED(IDC_SELECT_NO_PROCS, OnSelectNone)
	ON_BN_CLICKED(IDC_SELECT_DERM_ONLY, OnSelectDermOnly)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportNewProcedureSheet message handlers

BOOL CNexFormsImportNewProcedureSheet::OnInitDialog() 
{
	try
	{

		CNexFormsImportWizardSheet::OnInitDialog();
		
		m_pdlProcedureList = BindNxDataList2Ctrl(this, IDC_NEXFORMS_PROCEDURE_LIST, GetRemoteData(), false);

		LoadExistingProcedures();

	}NxCatchAll("CNexFormsImportNewProcedureSheet::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNexFormsImportNewProcedureSheet::Load()
{
	m_pdlProcedureList->Clear();
	for(int i = 0; i < m_pdlgMaster->m_ImportInfo.m_arypProcedures.GetSize(); i++)
	{
		IRowSettingsPtr pRow = m_pdlProcedureList->GetNewRow();
		pRow->PutValue(plcID, m_pdlgMaster->m_ImportInfo.m_arypProcedures.GetAt(i)->nID);
		pRow->PutValue(plcImport, m_pdlgMaster->m_ImportInfo.m_arypProcedures.GetAt(i)->bImport ? COleVariant(VARIANT_TRUE,VT_BOOL) : COleVariant(VARIANT_FALSE,VT_BOOL));
		pRow->PutValue(plcProcedure, _bstr_t(m_pdlgMaster->m_ImportInfo.m_arypProcedures.GetAt(i)->strName));
		pRow->PutValue(plcExistingProcedure, m_pdlgMaster->m_ImportInfo.m_arypProcedures.GetAt(i)->nExistingProcedureID);
		pRow->PutValue(plcMasterProcedure, m_pdlgMaster->m_ImportInfo.m_arypProcedures.GetAt(i)->nMasterProcedureID);
		pRow->PutValue(plcProcedurePointer, (long)m_pdlgMaster->m_ImportInfo.m_arypProcedures.GetAt(i));
		m_pdlProcedureList->AddRowAtEnd(pRow, NULL);
		UpdateLadderCombo(pRow);
	}
	m_pdlProcedureList->Sort();

	if(m_bInitialLoad)
	{
		MatchProceduresWithExisting();

		// (z.manning, 06/27/2007) - Go through any ladders that aren't being imported and see if we have an already
		// existing ladder with the same name that the procedures can use.
		UpdateProcedureTrackingLadders();
	}

	RefreshMasterProcedureEmbeddedCombo();

	RefreshLadderEmbeddedCombo();

	m_bInitialLoad = FALSE;
}

void CNexFormsImportNewProcedureSheet::MatchProceduresWithExisting()
{
	// (z.manning, 06/21/2007) - Create a local copy of the existing procedures so we can
	// remove them as we find them
	for(IRowSettingsPtr pRow = m_pdlProcedureList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		CString strNewProcedure = VarString(pRow->GetValue(plcProcedure), "");
		NexFormsProcedure *proc = NULL;
		IRowSettingsPtr pRowToUse = NULL;
		ExistingProcedure existingProc;
		for(int i = 0; i < m_aryExistingProcedures.GetSize(); i++)
		{
			CString strExistingProcedure = m_aryExistingProcedures.GetAt(i).strName;
			if(IsSameProcedureName(strExistingProcedure,strNewProcedure))
			{
				proc = (NexFormsProcedure*)VarLong(pRow->GetValue(plcProcedurePointer));
				pRowToUse = pRow;
				existingProc = m_aryExistingProcedures.GetAt(i);
				if(strExistingProcedure.CompareNoCase(strNewProcedure) == 0) {
					// (z.manning, 06/21/2007) - We found an exact match, so break out of the inner loop 
					// and move to the next procedure in the datalist.
					break;
				}
				else {
					// (z.manning, 08/30/2007) - If we get here it most likely means that we found a  near match
					// (e.g. Botox vs. Botox ®). So let's go ahead and mark this procedure's name to be updated 
					// with the new one so that it will definitely match our content from now on.
					proc->bUpdateName = TRUE;
				}
			}
		}

		if(proc != NULL)
		{
			// (z.manning, 06/26/2007) - Update our current row's exiting procedure ID and master ID.
			proc->nExistingProcedureID = existingProc.nID;
			pRowToUse->PutValue(plcExistingProcedure, existingProc.nID);
			proc->nMasterProcedureID = existingProc.nMasterProcedureID;
			pRowToUse->PutValue(plcMasterProcedure, existingProc.nMasterProcedureID);

			// (z.manning, 06/26/2007) - Go through every procedure and see if any are using
			// this procedure as a master. If so, change their master procedure ID to the existing ID.
			if(existingProc.nMasterProcedureID == 0) {
				UpdateMasterProcedureIDs(VarLong(pRowToUse->GetValue(plcID)), existingProc.nID);
			}
			else {
				UpdateMasterProcedureIDs(VarLong(pRowToUse->GetValue(plcID)), 0);
				proc->arynLadderTemplateIDs.RemoveAll();
			}

			// (z.manning, 06/27/2007) - Also update the tracking ladders to the ones currently used
			// by the existing procedure.
			// (z.manning, 07/30/2007) - Only do this for "existing" imports
			if(m_pdlgMaster->m_ImportInfo.m_eImportType == nfitExisting || m_pdlgMaster->m_ImportInfo.m_eImportType == nfitNewNexFormsExistingTracking) {
				proc->arynLadderTemplateIDs.RemoveAll();
				// (z.manning, 08/01/2007) - Only assign ladder IDs if this is a master procedure.
				if(proc->nMasterProcedureID == 0) {
					proc->arynLadderTemplateIDs.Append(existingProc.arynLadderTemplateIDs);
				}
				UpdateLadderCombo(pRowToUse);
			}

			// (z.manning, 07/23/2007) - PLID 26774 - Check the NexForms version of the existing procedure
			// and if it's the same (or possibly but unexpectedly greater) than the version we'll be importing
			// then default this procedure to not selected.
			if(existingProc.nNexFormsVersion >= m_pdlgMaster->m_ImportInfo.m_nNexFormsVersion) {
				proc->bImport = FALSE;
				pRowToUse->PutValue(plcImport, _variant_t(VARIANT_FALSE, VT_BOOL));
			}
		}
	}
	m_pdlProcedureList->Sort();
}

void CNexFormsImportNewProcedureSheet::LoadExistingProcedures()
{
	// (z.manning, 07/23/2007) - PLID 26774 - Also load the NexForms version.
	ADODB::_RecordsetPtr prs = CreateRecordset(
		"SELECT ID, Name, MasterProcedureID, LadderTemplateID, NexFormsVersion "
		"FROM ProcedureT "
		"LEFT JOIN ProcedureLadderTemplateT ON ProcedureT.ID = ProcedureLadderTemplateT.ProcedureID "
		"ORDER BY Name ");

	m_aryExistingProcedures.RemoveAll();
	CString strComboSource = "0;{ Create New Procedure };";
	for(; !prs->eof; prs->MoveNext())
	{
		ExistingProcedure proc;
		long nID = AdoFldLong(prs, "ID");
		CString strName = AdoFldString(prs, "Name", "");

		proc.nID = nID;
		proc.strName = strName;
		proc.nMasterProcedureID = AdoFldLong(prs, "MasterProcedureID", 0);
		proc.nNexFormsVersion = AdoFldLong(prs, "NexFormsVersion", 0);

		while(!prs->eof && AdoFldLong(prs, "ID") == nID)
		{
			long nLadderID = AdoFldLong(prs, "LadderTemplateID", 0);
			if(nLadderID != 0) {
				proc.arynLadderTemplateIDs.Add(nLadderID);
			}
			prs->MoveNext();
		}
		prs->MovePrevious();

		m_aryExistingProcedures.Add(proc);

		// (z.manning, 12/14/2007) - PLID 28246 - Need to make sure the procedure name doesn't have semicolons
		// in it or else the embedded combo will be a mess.
		strName.Replace(';', '_');
		strComboSource += AsString(nID) + ";" + strName + ";";
	}

	m_pdlProcedureList->GetColumn(plcExistingProcedure)->PutComboSource(_bstr_t(strComboSource));
}

BEGIN_EVENTSINK_MAP(CNexFormsImportNewProcedureSheet, CNexFormsImportWizardSheet)
    //{{AFX_EVENTSINK_MAP(CNexFormsImportNewProcedureSheet)
	ON_EVENT(CNexFormsImportNewProcedureSheet, IDC_NEXFORMS_PROCEDURE_LIST, 10 /* EditingFinished */, OnEditingFinishedNexformsProcedureList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CNexFormsImportNewProcedureSheet, IDC_NEXFORMS_PROCEDURE_LIST, 6 /* RButtonDown */, OnRButtonDownNexformsProcedureList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CNexFormsImportNewProcedureSheet, IDC_NEXFORMS_PROCEDURE_LIST, 9 /* EditingFinishing */, OnEditingFinishingNexformsProcedureList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CNexFormsImportNewProcedureSheet, IDC_NEXFORMS_PROCEDURE_LIST, 8 /* EditingStarting */, OnEditingStartingNexformsProcedureList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CNexFormsImportNewProcedureSheet::OnEditingStartingNexformsProcedureList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		return;
	}

	switch(nCol)
	{
		case plcLadder:
			// (z.manning, 10/16/2007) - Don't even let them drop down the ladder list if it's not a master procedure.
			if(VarLong(pRow->GetValue(plcMasterProcedure)) != 0) {
				*pbContinue = FALSE;
			}
			break;
	}
}

void CNexFormsImportNewProcedureSheet::OnEditingFinishingNexformsProcedureList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(pvarNewValue->vt == VT_EMPTY) {
			return;
		}

		switch(nCol)
		{
			case plcLadder:
				// (z.manning, 06/27/2007) - Make sure they don't try to assign a ladder to a detail procedure.
				if(VarLong(*pvarNewValue) != 0 && VarLong(pRow->GetValue(plcMasterProcedure)) != 0)
				{
					MessageBox("You may not assign ladders to detail procedures");
					*pbCommit = FALSE;
				}
			break;
				
			case plcExistingProcedure:
			{
				// (z.manning, 07/03/2007) - Make sure an existing procedure isn't selected more than once.
				long nNewExistingProcedureID = VarLong(*pvarNewValue);
				
				// (z.manning, 06/04/2008) - PLID 29200 - If the name of the procedure to be imported has the same
				// name as an existing procedure, then their only option is to update the existing procedure.
				CString strProcedure = VarString(pRow->GetValue(plcProcedure));
				for(int nExistingProcIndex = 0; nExistingProcIndex < m_aryExistingProcedures.GetSize(); nExistingProcIndex++)
				{
					ExistingProcedure proc = m_aryExistingProcedures.GetAt(nExistingProcIndex);
					if(proc.nID != nNewExistingProcedureID && proc.strName.CompareNoCase(strProcedure) == 0) {
						MessageBox("There is already an existing procedure named '" + proc.strName + ".' You may not associate this procedure with any other existing procedure.");
						*pbCommit = FALSE;
						return;
					}
				}

				if(nNewExistingProcedureID != 0)
				{
					IRowSettingsPtr pFind = m_pdlProcedureList->FindByColumn(plcExistingProcedure, nNewExistingProcedureID, NULL, VARIANT_FALSE);
					if(pFind != NULL && pFind != pRow)
					{
						// (z.manning, 07/03/2007) - There's another row with this value.
						MessageBox(FormatString("The procedure '%s' is already updating this existing procedure. You may not update the same existing procedure more than once.", VarString(pFind->GetValue(plcProcedure))));
						*pbCommit = FALSE;
					}
				}
			}
			break;

			case plcMasterProcedure:
				if(VarLong(*pvarNewValue) != 0)
				{
					long nProcedureID;
					if(VarLong(pRow->GetValue(plcExistingProcedure)) != 0) {
						// (z.manning, 07/05/2007) - We're updating an existing procedure, so check that ID.
						nProcedureID = VarLong(pRow->GetValue(plcExistingProcedure));
					}
					else {
						// (z.manning, 07/05/2007) - New procedure, use that ID.
						nProcedureID = VarLong(pRow->GetValue(plcID));
					}
					// (z.manning, 07/05/2007) - Don't let them make the master procedure itself.
					if(nProcedureID == VarLong(*pvarNewValue)) {
						MessageBox("You can't make a procedure its own master procedure.");
						*pbCommit = FALSE;
					}

					// (z.manning, 07/05/2007) - Make sure they aren't making a procedure into a detail procedure
					// if it's already the master procedure for something else.
					for(IRowSettingsPtr pTemp = m_pdlProcedureList->GetFirstRow(); pTemp != NULL && *pbCommit; pTemp = pTemp->GetNextRow())
					{
						if(VarLong(pTemp->GetValue(plcMasterProcedure)) == nProcedureID)
						{
							MessageBox(FormatString("The procedure '%s' is the master procedure for other procedure(s), so you can't make it a detail procedure.", VarString(pRow->GetValue(plcProcedure),"")));
							*pbCommit = FALSE;
						}
					}
				}
			break;

		}

	}NxCatchAll("CNexFormsImportNewProcedureSheet::OnEditingFinishingNexformsProcedureList");
}

void CNexFormsImportNewProcedureSheet::OnEditingFinishedNexformsProcedureList(LPDISPATCH lpRow, short nCol, const _variant_t &varOldValue, const _variant_t &varNewValue, BOOL bCommit)
{
	try
	{
		if(!bCommit) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		// (z.manning, 06/26/2007) - If nothing changed, don't need to do anything.
		// (z.manning, 06/27/2007) - Except for ladders where we need to popup the multi-select
		// dialog even if that's what's already selected.
		if(varOldValue == varNewValue && nCol != plcLadder) {
			return;
		}

		NexFormsProcedure *proc = (NexFormsProcedure*)VarLong(pRow->GetValue(plcProcedurePointer));

		switch(nCol)
		{
			case plcExistingProcedure:
			{
				// (z.manning, 06/22/2007) - Update this procedure to use the selected existing procedure ID.
				long nNewExistingProcedureID = VarLong(varNewValue);
				proc->nExistingProcedureID = nNewExistingProcedureID;
				if(nNewExistingProcedureID > 0)
				{
					// (z.manning, 06/26/2007) - It's an already existing procedure. Find this procedure's master ID.
					for(int i = 0; i < m_aryExistingProcedures.GetSize(); i++)
					{
						// (z.manning, 06/26/2007) - First, go through every procedure and see if any are using
						// this procedure as a master. If so, change their master procedure ID to the existing ID.
						// But only if the master procedure is not an already existing procedure.
						if(VarLong(varOldValue) != 0 && VarLong(pRow->GetValue(plcMasterProcedure)) < 0) {
							UpdateMasterProcedureIDs(VarLong(varOldValue), nNewExistingProcedureID);
						}

						if(m_aryExistingProcedures.GetAt(i).nID == nNewExistingProcedureID) 
						{
							pRow->PutValue(plcMasterProcedure, m_aryExistingProcedures.GetAt(i).nMasterProcedureID);
							proc->nMasterProcedureID = m_aryExistingProcedures.GetAt(i).nMasterProcedureID;

							// (z.manning, 06/27/2007) - Also update the tracking ladders to the ones currently used
							// by the existing procedure (only if it's an existing tracking user import).
							if(m_pdlgMaster->m_ImportInfo.m_eImportType == nfitExisting || m_pdlgMaster->m_ImportInfo.m_eImportType == nfitNewNexFormsExistingTracking) {
								proc->arynLadderTemplateIDs.RemoveAll();
								// (z.manning, 08/01/2007) - Make sure it's a master procedure.
								if(proc->nMasterProcedureID == 0) {
									proc->arynLadderTemplateIDs.Append(m_aryExistingProcedures.GetAt(i).arynLadderTemplateIDs);
								}
							}
							// (z.manning, 08/02/2007) - No ladders for detail procedures.
							if(proc->nMasterProcedureID != 0) {
								proc->arynLadderTemplateIDs.RemoveAll();
							}

							// (z.manning, 07/23/2007) - Do the existing procedure and import procedure have different names?
							if(m_aryExistingProcedures.GetAt(i).strName.CompareNoCase(VarString(pRow->GetValue(plcProcedure))) != 0) {
								// (z.manning, 07/23/2007) - They are different. Let's ask the user if he/she wants to
								// update the procedure name as that will make future imports run better.
								CString strMessage = FormatString(
									"You have chosen to update a procedure with an existing procedure with a different name. "
									"To make future updates easier, it's recommended that you update this procedure's name "
									"with the new name. However, this will change the name of this procedure everywhere it's "
									"used in the program including appointment purposes, tracking ladders, etc.\r\n\r\n"
									"Would you like to rename '%s' to '%s'?"
									, m_aryExistingProcedures.GetAt(i).strName, proc->strName);
								if(MessageBox(strMessage, "Rename procedure?", MB_YESNO) == IDYES) {
									// (z.manning, 07/23/2007) - Ok, they want to update the procedure name.
									proc->bUpdateName = TRUE;
								}
								else {
									//TES 9/2/2011 - PLID 37838 - It's possible that bUpdateName has already been set to TRUE.  Since the user
									// just explicitly said that they don't want to rename this procedure, make sure it's set to FALSE.
									proc->bUpdateName = FALSE;
								}
							}

							break;
						}
					}
				}
				else
				{
					// (z.manning, 06/26/2007) - They want to create it as new. Let's just leave the master
					// procedure column alone.
					ASSERT(nNewExistingProcedureID == 0);
				}
		
				RefreshMasterProcedureEmbeddedCombo();
				UpdateLadderCombo(pRow);
			}
			break;

			case plcImport:
			{
				proc->bImport = VarBool(varNewValue);
			}
			break;

			case plcMasterProcedure:
			{
				proc->nMasterProcedureID = VarLong(varNewValue);
				RefreshMasterProcedureEmbeddedCombo();

				// (z.manning, 06/27/2007) - If they set a master procedure, then this procedure is now a detail
				// so not tracking ladders allowed.
				if(VarLong(varNewValue) != 0)
				{
					proc->arynLadderTemplateIDs.RemoveAll();
					UpdateLadderCombo(pRow);
				}
			}
			break;

			case plcLadder:
			{
				long nLadderID = VarLong(varNewValue);
				if(nLadderID == MULTIPLE_LADDERS_COMBO_ID)
				{
					// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
					CMultiSelectDlg dlg(this, "LadderTemplatesT");
					dlg.PreSelect(proc->arynLadderTemplateIDs);
					// (z.manning, 06/27/2007) - Don't show mutliple ladder or no ladder options in this dialog.
					CVariantArray aryvarIDsToSkip;
					aryvarIDsToSkip.Add(_variant_t((long)0));
					aryvarIDsToSkip.Add(_variant_t((long)MULTIPLE_LADDERS_COMBO_ID));
					if(IDOK == dlg.OpenWithDelimitedComboSource(m_pdlProcedureList->GetColumn(plcLadder)->GetComboSource(), aryvarIDsToSkip, "Select Ladders"))
					{
						// (z.manning, 06/27/2007) - They saved the dialog. Repopulate this procedure's ladder
						// ID array.
						CVariantArray aryvarLadderIDs;
						dlg.FillArrayWithIDs(aryvarLadderIDs);
						proc->arynLadderTemplateIDs.RemoveAll();
						for(int i = 0; i < aryvarLadderIDs.GetSize(); i++) {
							proc->arynLadderTemplateIDs.Add(AsLong(aryvarLadderIDs.GetAt(i)));
						}
					}
				}
				else if(nLadderID != 0)
				{
					proc->arynLadderTemplateIDs.RemoveAll();
					proc->arynLadderTemplateIDs.Add(nLadderID);
				}
				else
				{
					ASSERT(nLadderID == 0);
					proc->arynLadderTemplateIDs.RemoveAll();
				}
				UpdateLadderCombo(pRow);
			}
			break;
		}

	}NxCatchAll("CNexFormsImportNewProcedureSheet::OnEditingFinishedNexformsProcedureList");
}


void CNexFormsImportNewProcedureSheet::SelectHighlightedRows(BOOL bSelect)
{
	// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
	IRowSettingsPtr pCurSelRow = m_pdlProcedureList->GetFirstSelRow();
	while (pCurSelRow != NULL) 
	{
		IRowSettingsPtr pRow = pCurSelRow;
		pCurSelRow = pCurSelRow->GetNextSelRow();

		pRow->PutValue(plcImport, bSelect ? COleVariant(VARIANT_TRUE,VT_BOOL) : COleVariant(VARIANT_FALSE,VT_BOOL));
		NexFormsProcedure *proc = (NexFormsProcedure*)VarLong(pRow->GetValue(plcProcedurePointer));
		proc->bImport = bSelect;
	}
}


void CNexFormsImportNewProcedureSheet::OnRButtonDownNexformsProcedureList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		// (z.manning, 06/22/2007) - If we're right clicking on a row that isn't already selected, make 
		// it the current selection.
		if(pRow != NULL && !pRow->IsHighlighted()) {
			m_pdlProcedureList->PutCurSel(pRow);
		}
		if(m_pdlProcedureList->GetCurSel() == NULL) {
			return;
		}

		CMenu mnu;
		mnu.CreatePopupMenu();

		enum MenuOptions
		{
			moCheck = 1,
			moUncheck = 2,
		};

		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, moCheck, "Check");
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, moUncheck, "Uncheck");

		CPoint pt(x, y);
		CWnd *pWnd = GetDlgItem(IDC_NEXFORMS_PROCEDURE_LIST);
		if(pWnd != NULL) {
			pWnd->ClientToScreen(&pt);
		}

		int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
		switch(nResult)
		{
			case moCheck:
				SelectHighlightedRows(TRUE);
				break;

			case moUncheck:
				SelectHighlightedRows(FALSE);
				break;
		}

	}NxCatchAll("CNexFormsImportNewProcedureSheet::OnRButtonDownNexformsProcedureList");
}

void CNexFormsImportNewProcedureSheet::RefreshMasterProcedureEmbeddedCombo()
{
	// (z.manning, 06/25/2007) - The should include not only all master procedures that exist
	// in the database already, but also any ones that are currently selected to be imported.
	// Let's put them in an array first so we can sort them before setting the embedded combo.
	CArray<ExistingProcedure*,ExistingProcedure*> aryTempProcedures;

	// (z.manning, 06/25/2007) - Get all existing master procedures.
	for(int i = 0; i < m_aryExistingProcedures.GetSize(); i++)
	{
		if(m_aryExistingProcedures.GetAt(i).nMasterProcedureID == 0) 
		{
			ExistingProcedure *proc = new ExistingProcedure;
			proc->nID = m_aryExistingProcedures.GetAt(i).nID;
			proc->strName = m_aryExistingProcedures.GetAt(i).strName;
			aryTempProcedures.Add(proc);
		}
	}

	// (z.manning, 06/25/2007) - Now load all procedures that we'll be importing new that are currently
	// set to be master procedures.
	for(IRowSettingsPtr pRow = m_pdlProcedureList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		if(VarLong(pRow->GetValue(plcExistingProcedure)) == 0 && // (z.manning, 06/25/2007) - It's being imported new
			VarLong(pRow->GetValue(plcMasterProcedure)) == 0) // (z.manning, 06/25/2007) - It's a master procedure
		{
			// (z.manning, 06/25/2007) - Note: we use positive values for current DB IDs and negative values
			// for export IDs to ensure we don't mix them up.
			ExistingProcedure *proc = new ExistingProcedure;
			proc->nID = VarLong(pRow->GetValue(plcID));
			proc->strName = VarString(pRow->GetValue(plcProcedure), "");
			aryTempProcedures.Add(proc);
		}
	}

	// (z.manning, 06/25/2007) - Sort the array
	qsort(aryTempProcedures.GetData(), aryTempProcedures.GetSize(), sizeof(ExistingProcedure*), CompareExistingProceduresByName);

	// (z.manning, 06/26/2007) - Now loop through the array to build the combo source.
	CString strComboSource = "0;{ Is Master Procedure };";
	for(i = 0; i < aryTempProcedures.GetSize(); i++)
	{
		// (z.manning, 12/14/2007) - PLID 28246 - Need to make sure the procedure name doesn't have semicolons
		// in it or else the embedded combo will be a mess.
		CString strProcedureName = aryTempProcedures.GetAt(i)->strName;
		strProcedureName.Replace(';', '_');
		strComboSource += AsString(aryTempProcedures.GetAt(i)->nID) + ";" + strProcedureName + ";";
		delete aryTempProcedures.GetAt(i);
	}
	m_pdlProcedureList->GetColumn(plcMasterProcedure)->PutComboSource(_bstr_t(strComboSource));
}

void CNexFormsImportNewProcedureSheet::RefreshLadderEmbeddedCombo()
{
	// (z.manning, 06/26/2007) - Store all ladders in an array temporarily so we can sort them.
	CArray<ExistingLadder*,ExistingLadder*> aryLadders;

	// (z.manning, 06/26/2007) - Load all known existing procedures.
	for(int i = 0; i < m_pdlgMaster->m_ImportInfo.m_aryExistingLadders.GetSize(); i++)
	{
		ExistingLadder *ladder = new ExistingLadder;
		ladder->nID = m_pdlgMaster->m_ImportInfo.m_aryExistingLadders.GetAt(i)->nID;
		ladder->strName = m_pdlgMaster->m_ImportInfo.m_aryExistingLadders.GetAt(i)->strName;
		aryLadders.Add(ladder);
	}

	// (z.manning, 06/26/2007) - Load all new procedures.
	for(i = 0; i < m_pdlgMaster->m_ImportInfo.m_arypLadders.GetSize(); i++)
	{
		if(m_pdlgMaster->m_ImportInfo.m_arypLadders.GetAt(i)->bImport)
		{
			ExistingLadder *ladder = new ExistingLadder;
			ladder->nID = m_pdlgMaster->m_ImportInfo.m_arypLadders.GetAt(i)->nID;
			CString strLadderName = m_pdlgMaster->m_ImportInfo.m_arypLadders.GetAt(i)->strName;
			ladder->strName = strLadderName;
			aryLadders.Add(ladder);

			// (z.manning, 09/10/2007) - If we are going to be deleting an existing ladder with the same
			// name, then remove it from the embedded combo as well.
			if(m_pdlgMaster->m_ImportInfo.m_arypLadders.GetAt(i)->bDeleteExistingLadder) {
				for(int j = 0; j < aryLadders.GetSize(); j++) {
					if(aryLadders.GetAt(j)->nID > 0 && aryLadders.GetAt(j)->strName.CompareNoCase(strLadderName) == 0) {
						delete aryLadders.GetAt(j);
						aryLadders.RemoveAt(j);
						break;
					}
				}
			}
		}
	}

	// (z.manning, 06/26/2007) - Sort the ladders.
	qsort(aryLadders.GetData(), aryLadders.GetSize(), sizeof(ExistingLadder*), CompareExistingLaddersByName);

	// (z.manning, 06/26/2007) - Now build the combo source.
	CString strComboSource = FormatString("0;;%li;{ Multiple Ladders };", MULTIPLE_LADDERS_COMBO_ID);
	for(i = 0; i < aryLadders.GetSize(); i++)
	{
		strComboSource += AsString(aryLadders.GetAt(i)->nID) + ";" + aryLadders.GetAt(i)->strName + ";";
		delete aryLadders.GetAt(i);
	}
	m_pdlProcedureList->GetColumn(plcLadder)->PutComboSource(_bstr_t(strComboSource));
}

void CNexFormsImportNewProcedureSheet::UpdateMasterProcedureIDs(long nCurrentMasterProcedureID, long nNewMasterProcedureID)
{
	// (z.manning, 06/26/2007) - First, go through every procedure and see if any are using
	// this procedure as a master. If so, change their master procedure ID to the existing ID.
	for(IRowSettingsPtr pRow = m_pdlProcedureList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		if(VarLong(pRow->GetValue(plcMasterProcedure)) == nCurrentMasterProcedureID)
		{
			NexFormsProcedure *proc = (NexFormsProcedure*)VarLong(pRow->GetValue(plcProcedurePointer));
			pRow->PutValue(plcMasterProcedure, nNewMasterProcedureID);
			proc->nMasterProcedureID = nNewMasterProcedureID;
			// (z.manning, 08/01/2007) - If it's now a detail procedure, make sure it doesn't have any ladders.
			if(nNewMasterProcedureID != 0) {
				proc->arynLadderTemplateIDs.RemoveAll();
			}
			UpdateLadderCombo(pRow);
		}
	}
}

void CNexFormsImportNewProcedureSheet::UpdateLadderCombo(IRowSettingsPtr pRow)
{
	NexFormsProcedure *procedure = (NexFormsProcedure*)VarLong(pRow->GetValue(plcProcedurePointer));

	int nLadderCount = procedure->arynLadderTemplateIDs.GetSize();
	if(nLadderCount == 0)
	{
		pRow->PutValue(plcLadder, (long)0);

	}
	else if(nLadderCount == 1)
	{
		pRow->PutValue(plcLadder, procedure->arynLadderTemplateIDs.GetAt(0));
	}
	else
	{
		pRow->PutValue(plcLadder, (long)MULTIPLE_LADDERS_COMBO_ID);
	}

	// (z.manning, 08/01/2007) - For details procedures, color the cell gray to denote they can't select anything.
	if(procedure->nMasterProcedureID != 0) {
		ASSERT(nLadderCount == 0);
		pRow->PutCellBackColor(plcLadder, RGB(175,175,175));
	}
	else {
		pRow->PutCellBackColor(plcLadder, RGB(255,255,255));
	}
}

void CNexFormsImportNewProcedureSheet::UpdateProcedureTrackingLadders()
{
	// (z.manning, 06/27/2007)
	// - Go through each ladder we're NOT importing
	// - See if we can match it to an already existing ladder
	// - If so, find all procedures that use this ladder ID and replace the 'new' ID with the 'existing' ID
	for(int nNewLadderIndex = 0; nNewLadderIndex < m_pdlgMaster->m_ImportInfo.m_arypLadders.GetSize(); nNewLadderIndex++)
	{
		if(!m_pdlgMaster->m_ImportInfo.m_arypLadders.GetAt(nNewLadderIndex)->bImport)
		{
			CString strNewName = m_pdlgMaster->m_ImportInfo.m_arypLadders.GetAt(nNewLadderIndex)->strName;
			long nNewLadderID = m_pdlgMaster->m_ImportInfo.m_arypLadders.GetAt(nNewLadderIndex)->nID;
			for(int nExistingLadderIndex = 0; nExistingLadderIndex < m_pdlgMaster->m_ImportInfo.m_aryExistingLadders.GetSize(); nExistingLadderIndex++)
			{
				CString strExistingName = m_pdlgMaster->m_ImportInfo.m_aryExistingLadders.GetAt(nExistingLadderIndex)->strName;
				long nExistingLadderID = m_pdlgMaster->m_ImportInfo.m_aryExistingLadders.GetAt(nExistingLadderIndex)->nID;
				if(strExistingName.CompareNoCase(strNewName) == 0)
				{
					for(int nProcIndex = 0; nProcIndex < m_pdlgMaster->m_ImportInfo.m_arypProcedures.GetSize(); nProcIndex++)
					{
						NexFormsProcedure *procedure = m_pdlgMaster->m_ImportInfo.m_arypProcedures.GetAt(nProcIndex);
						for(int nLadderIndex = 0; nLadderIndex < procedure->arynLadderTemplateIDs.GetSize(); nLadderIndex++)
						{
							ASSERT(procedure->nMasterProcedureID == 0);
							if(nNewLadderID == procedure->arynLadderTemplateIDs.GetAt(nLadderIndex))
							{
								procedure->arynLadderTemplateIDs.SetAt(nLadderIndex, nExistingLadderID);
							}
						}
					}
				}
			}
		}
	}

	for(IRowSettingsPtr pRow = m_pdlProcedureList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		UpdateLadderCombo(pRow);
	}
}

void CNexFormsImportNewProcedureSheet::OnSelectAll() 
{
	try
	{
		SelectAllRows(TRUE);

	}NxCatchAll("CNexFormsImportNewProcedureSheet::OnSelectAll");
}

void CNexFormsImportNewProcedureSheet::OnSelectNone() 
{
	try
	{
		SelectAllRows(FALSE);

	}NxCatchAll("CNexFormsImportNewProcedureSheet::OnSelectNone");
}

void CNexFormsImportNewProcedureSheet::OnSelectDermOnly()
{
	try
	{
		for(IRowSettingsPtr pRow = m_pdlProcedureList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			NexFormsProcedure *proc = (NexFormsProcedure*)VarLong(pRow->GetValue(plcProcedurePointer));
			BOOL bSelect = IsDermProcedure(proc->strName);
			pRow->PutValue(plcImport, bSelect ? COleVariant(VARIANT_TRUE,VT_BOOL) : COleVariant(VARIANT_FALSE,VT_BOOL));
			proc->bImport = bSelect;
		}

	}NxCatchAll("CNexFormsImportNewProcedureSheet::OnSelectDermOnly");
}

void CNexFormsImportNewProcedureSheet::SelectAllRows(BOOL bSelect)
{
	for(IRowSettingsPtr pRow = m_pdlProcedureList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		pRow->PutValue(plcImport, bSelect ? COleVariant(VARIANT_TRUE,VT_BOOL) : COleVariant(VARIANT_FALSE,VT_BOOL));
		NexFormsProcedure *proc = (NexFormsProcedure*)VarLong(pRow->GetValue(plcProcedurePointer));
		proc->bImport = bSelect;
	}
}

BOOL CNexFormsImportNewProcedureSheet::Validate()
{
	CArray<IRowSettings*,IRowSettings*> arypNeededMasterProcedureRows;
	for(IRowSettingsPtr pRow = m_pdlProcedureList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		if(VarBool(pRow->GetValue(plcImport)))
		{
			// (z.manning, 07/03/2007) - We assume that the export file does not have bad data, including
			// procedures with dupicate names. We do, however, need to make sure that any procedures that
			// are being imported new do not have the same name.
			if(VarLong(pRow->GetValue(plcExistingProcedure), 0) == 0)
			{
				for(int i = 0; i < m_aryExistingProcedures.GetSize(); i++)
				{
					if(0 == VarString(pRow->GetValue(plcProcedure),"").CompareNoCase(m_aryExistingProcedures.GetAt(i).strName))
					{
						MessageBox(FormatString("You already have a procedure named '%s.' You must either update the existing procedure or not import the new procedure.", m_aryExistingProcedures.GetAt(i).strName));
						return FALSE;
					}
				}
			}

			// (z.manning, 07/03/2007) - Make sure that if a detail procedure has a master procedure that doesn't
			// exist yet, that it's at least marked to be imported.
			long nMasterProcedureID = VarLong(pRow->GetValue(plcMasterProcedure), 0);
			if(nMasterProcedureID < 0)
			{
				// (z.manning, 07/03/2007) - Ok, it's a new procedure, make sure we're importing it.
				IRowSettingsPtr pMasterRow = m_pdlProcedureList->FindByColumn(plcID, nMasterProcedureID, NULL, VARIANT_FALSE);
				if(pMasterRow != NULL) {
					if(!VarBool(pMasterRow->GetValue(plcImport))) {
						// (z.manning, 07/03/2007) - We're not importing it, add it to the array and we'll handle it
						// later.
						BOOL bFound = FALSE;
						for(int i = 0; i < arypNeededMasterProcedureRows.GetSize() && !bFound; i++) {
							if(arypNeededMasterProcedureRows.GetAt(i) == pMasterRow) {
								bFound = TRUE;
							}
						}
						if(!bFound) {
							arypNeededMasterProcedureRows.Add(pMasterRow);
						}
					}
				}
			}
		}
	}

	// (z.manning, 07/03/2007) - If we have procedures set to use master procedures that aren't marked to import
	// ask the user if they want to import them. If yes, good, if not, then we can't continue.
	if(arypNeededMasterProcedureRows.GetSize() > 0)
	{
		CString strMsg = "The following procedures are set to the master for detail procedures you're importing:\r\n";
		for(int i = 0; i < arypNeededMasterProcedureRows.GetSize(); i++) {
			strMsg += FormatString("-- %s\r\n", VarString(arypNeededMasterProcedureRows.GetAt(i)->GetValue(plcProcedure),""));
		}
		strMsg += "\r\nYou must import these procedures to continue. Would you like to import these procedures?";
		if(IDYES != MessageBox(strMsg,NULL,MB_YESNO)) {
			return FALSE;
		}
		for(i = 0; i < arypNeededMasterProcedureRows.GetSize(); i++) {
			// (z.manning, 07/03/2007) - If we get this far, it means they want to import them.
			IRowSettingsPtr pTemp = arypNeededMasterProcedureRows.GetAt(i);
			pTemp->PutValue(plcImport, _variant_t(VARIANT_TRUE, VT_BOOL));
			NexFormsProcedure *proc = (NexFormsProcedure*)VarLong(pTemp->GetValue(plcProcedurePointer));
			proc->bImport = TRUE;
		}
	}


	return TRUE;
}