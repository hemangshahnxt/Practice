// NexFormsImportProcedureColumnsSheet.cpp : implementation file
//

#include "stdafx.h"
#include "NexFormsImportProcedureColumnsSheet.h"

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

enum ProcedureContentColumns
{
	pccID = 0,
	pccPointer,
	pccProcedure,
	pccUpdateName,
	pccMedicalTerm,
	pccCustom1,
	pccCustom2,
	pccCustom3,
	pccCustom4,
	pccCustom5,
	pccCustom6,
	pccAnesthesia,
	pccPrepTime,
};

enum NexFormsContentColumns
{
	nccID = 0,
	nccPointer,
	nccProcedure,
	nccMiniDesc,
	nccPreOp,
	nccTheDayOf,
	nccPostOp,
	nccRecovery,
	nccProcDetails, // (z.manning, 09/04/2007) - PLID 27286 - Renamed field in data to ProcDetails.
	nccRisks,
	nccAlternatives,
	nccComplications,
	nccSpecialDiet,
	nccShowering,
	nccBandages,
	nccConsent,
	nccAltConsent,
	nccHospitalStay,
};


// (z.manning, 08/30/2007) - PLID 18359 - Created a new importer for NexForms.
/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportProcedureColumnsSheet dialog


CNexFormsImportProcedureColumnsSheet::CNexFormsImportProcedureColumnsSheet(CNexFormsImportWizardMasterDlg* pParent)
	: CNexFormsImportWizardSheet(CNexFormsImportProcedureColumnsSheet::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNexFormsImportProcedureColumnsSheet)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNexFormsImportProcedureColumnsSheet::DoDataExchange(CDataExchange* pDX)
{
	CNexFormsImportWizardSheet::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexFormsImportProcedureColumnsSheet)
	DDX_Control(pDX, IDC_SELECT_NOTHING, m_btnSelectNothing);
	DDX_Control(pDX, IDC_SELECT_EVERYTHING, m_btnSelectEverything);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexFormsImportProcedureColumnsSheet, CNexFormsImportWizardSheet)
	//{{AFX_MSG_MAP(CNexFormsImportProcedureColumnsSheet)
	ON_BN_CLICKED(IDC_SELECT_EVERYTHING, OnSelectEverything)
	ON_BN_CLICKED(IDC_SELECT_NOTHING, OnSelectNothing)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CNexFormsImportProcedureColumnsSheet, CNexFormsImportWizardSheet)
    //{{AFX_EVENTSINK_MAP(CNexFormsImportProcedureColumnsSheet)
	ON_EVENT(CNexFormsImportProcedureColumnsSheet, IDC_NEXFORMS_CONTENT, 17 /* ColumnClicking */, OnColumnClickingNexformsContent, VTS_I2 VTS_PBOOL)
	ON_EVENT(CNexFormsImportProcedureColumnsSheet, IDC_PROCEDURE_CONTENT, 17 /* ColumnClicking */, OnColumnClickingProcedureContent, VTS_I2 VTS_PBOOL)
	ON_EVENT(CNexFormsImportProcedureColumnsSheet, IDC_NEXFORMS_CONTENT, 10 /* EditingFinished */, OnEditingFinishedNexformsContent, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CNexFormsImportProcedureColumnsSheet, IDC_PROCEDURE_CONTENT, 10 /* EditingFinished */, OnEditingFinishedProcedureContent, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportProcedureColumnsSheet message handlers

BOOL CNexFormsImportProcedureColumnsSheet::OnInitDialog() 
{
	try
	{
		CNexFormsImportWizardSheet::OnInitDialog();

		m_pdlProcedureContent = BindNxDataList2Ctrl(this, IDC_PROCEDURE_CONTENT, GetRemoteData(), false);
		m_pdlNexFormsContent = BindNxDataList2Ctrl(this, IDC_NEXFORMS_CONTENT, GetRemoteData(), false);

		for(int nCol = 0; nCol < m_pdlProcedureContent->GetColumnCount(); nCol++)
		{
			// (z.manning, 10/10/2007) - PLID 27706 - We now store the custom field names in a map.
			CString strUserFriendlyName;
			if(m_pdlgMaster->m_mapCustomFieldNames.Lookup(m_pdlProcedureContent->GetColumn(nCol)->GetColumnTitle(), strUserFriendlyName)) {
				m_pdlProcedureContent->GetColumn(nCol)->PutColumnTitle(_bstr_t(strUserFriendlyName));
			}
		}
	
	}NxCatchAll("CNexFormsImportProcedureColumnsSheet::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNexFormsImportProcedureColumnsSheet::Load()
{
	m_pdlNexFormsContent->Clear();
	m_pdlProcedureContent->Clear();

	// (z.manning, 07/16/2007) - The procedure post update thread determines which fields we want to import
	// for each procedure by default, so wait for it to finish. It may take a while to finish, so if the 
	// user went through the previous portion of the wizard real quick, we'll likely have to wait here.
	if(m_pdlgMaster->m_ImportInfo.m_pPostProcedureThread != NULL) {
		if(WaitForSingleObject(m_pdlgMaster->m_ImportInfo.m_pPostProcedureThread->m_hThread, 120000) == WAIT_TIMEOUT) {
			// (z.manning, 07/16/2007) - It didn't finish, for some odd reason, but that's not the end of the
			// world. All that will happen is that fields that my not have any data will be checked by default.
			ASSERT(FALSE);
			// (z.manning, 10/25/2007) - To avoid potentially odd things happening, we need to end the thread.
			SetEvent(m_pdlgMaster->m_ImportInfo.m_hevDestroying);
			if(WaitForSingleObject(m_pdlgMaster->m_ImportInfo.m_pPostProcedureThread->m_hThread, 5000) == WAIT_TIMEOUT) {
				// (z.manning, 10/25/2007) - This should not ever happen, but just in case, let's kill the thread.
				ASSERT(FALSE);
				TerminateThread(m_pdlgMaster->m_ImportInfo.m_pPostProcedureThread->m_hThread, 0);
			}
		}
	}

	for(int i = 0; i < m_pdlgMaster->m_ImportInfo.m_arypProcedures.GetSize(); i++)
	{
		NexFormsProcedure *procedure = m_pdlgMaster->m_ImportInfo.m_arypProcedures.GetAt(i);
		// (z.manning, 06/28/2007) - Only load procedures that we'll be importing. Duh.
		// Also, only ones we're updating.
		if(procedure->bImport && procedure->nExistingProcedureID != 0)
		{
			_variant_t varTrue(VARIANT_TRUE, VT_BOOL), varFalse(VARIANT_FALSE, VT_BOOL);

			IRowSettingsPtr pProcedureRow = m_pdlProcedureContent->GetNewRow();
			pProcedureRow->PutValue(pccID, procedure->nID);
			pProcedureRow->PutValue(pccPointer, (long)procedure);
			pProcedureRow->PutValue(pccProcedure, _bstr_t(procedure->strName));
			pProcedureRow->PutValue(pccUpdateName, procedure->bUpdateName ? varTrue : varFalse);
			pProcedureRow->PutValue(pccMedicalTerm, procedure->bImportOfficialName ? varTrue : varFalse);
			pProcedureRow->PutValue(pccCustom1, procedure->bImportCustom1 ? varTrue : varFalse);
			pProcedureRow->PutValue(pccCustom2, procedure->bImportCustom2 ? varTrue : varFalse);
			pProcedureRow->PutValue(pccCustom3, procedure->bImportCustom3 ? varTrue : varFalse);
			pProcedureRow->PutValue(pccCustom4, procedure->bImportCustom4 ? varTrue : varFalse);
			pProcedureRow->PutValue(pccCustom5, procedure->bImportCustom5 ? varTrue : varFalse);
			pProcedureRow->PutValue(pccCustom6, procedure->bImportCustom6 ? varTrue : varFalse);
			pProcedureRow->PutValue(pccAnesthesia, procedure->bImportAnesthesia ? varTrue : varFalse);
			pProcedureRow->PutValue(pccPrepTime, procedure->bImportArrivalPrepMinutes ? varTrue : varFalse);
			m_pdlProcedureContent->AddRowSorted(pProcedureRow, NULL);
			HandleProcedureContentChange(pProcedureRow);

			IRowSettingsPtr pNexFormsRow = m_pdlNexFormsContent->GetNewRow();
			pNexFormsRow->PutValue(nccID, procedure->nID);
			pNexFormsRow->PutValue(nccPointer, (long)procedure);
			pNexFormsRow->PutValue(nccProcedure, _bstr_t(procedure->strName));
			pNexFormsRow->PutValue(nccMiniDesc, procedure->bImportMiniDescription ? varTrue : varFalse);
			pNexFormsRow->PutValue(nccPreOp, procedure->bImportPreOp ? varTrue : varFalse);
			pNexFormsRow->PutValue(nccTheDayOf, procedure->bImportTheDayOf ? varTrue : varFalse);
			pNexFormsRow->PutValue(nccPostOp, procedure->bImportPostOp ? varTrue : varFalse);
			pNexFormsRow->PutValue(nccRecovery, procedure->bImportRecovery ? varTrue : varFalse);
			pNexFormsRow->PutValue(nccProcDetails, procedure->bImportProcDetails ? varTrue : varFalse);
			pNexFormsRow->PutValue(nccRisks, procedure->bImportRisks ? varTrue : varFalse);
			pNexFormsRow->PutValue(nccAlternatives, procedure->bImportAlternatives ? varTrue : varFalse);
			pNexFormsRow->PutValue(nccComplications, procedure->bImportComplications ? varTrue : varFalse);
			pNexFormsRow->PutValue(nccSpecialDiet, procedure->bImportSpecialDiet ? varTrue : varFalse);
			pNexFormsRow->PutValue(nccShowering, procedure->bImportShowering ? varTrue : varFalse);
			pNexFormsRow->PutValue(nccBandages, procedure->bImportBandages ? varTrue : varFalse);
			pNexFormsRow->PutValue(nccConsent, procedure->bImportConsent ? varTrue : varFalse);
			pNexFormsRow->PutValue(nccAltConsent, procedure->bImportAltConsent ? varTrue : varFalse);
			pNexFormsRow->PutValue(nccHospitalStay, procedure->bImportHospitalStay ? varTrue : varFalse);
			m_pdlNexFormsContent->AddRowSorted(pNexFormsRow, NULL);
			HandleNexFormsContentChange(pNexFormsRow);
		}
	}

	// (z.manning, 07/10/2007) - If we have no procedures to update, then this sheet is pretty pointless,
	// so we may as well skip it. (We can only do this under the assumption that this is not the last
	// sheet.)
	if(m_pdlNexFormsContent->GetRowCount() == 0 && m_pdlProcedureContent->GetRowCount() == 0) {
		m_bSkipSheet = TRUE;
	}
	else {
		m_bSkipSheet = FALSE;
	}
}

void CNexFormsImportProcedureColumnsSheet::OnColumnClickingNexformsContent(short nCol, BOOL FAR* bAllowSort) 
{
	try
	{
		*bAllowSort = FALSE;
		HandleColumnClick(m_pdlNexFormsContent, nCol);

		for(IRowSettingsPtr pRow = m_pdlNexFormsContent->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			HandleNexFormsContentChange(pRow);
		}

	}NxCatchAll("CNexFormsImportProcedureColumnsSheet::OnColumnClickingNexformsContent");
}

void CNexFormsImportProcedureColumnsSheet::OnColumnClickingProcedureContent(short nCol, BOOL FAR* bAllowSort) 
{
	try
	{
		*bAllowSort = FALSE;
		HandleColumnClick(m_pdlProcedureContent, nCol);

		for(IRowSettingsPtr pRow = m_pdlProcedureContent->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			HandleProcedureContentChange(pRow);
		}

	}NxCatchAll("CNexFormsImportProcedureColumnsSheet::OnColumnClickingProcedureContent");
}

void CNexFormsImportProcedureColumnsSheet::HandleColumnClick(_DNxDataListPtr pdl, short nCol)
{
	// (z.manning, 06/28/2007) - Just take the opposite value of the first row's check box
	// and assign that value to every row.

	if(pdl->GetRowCount() == 0) {
		return;
	}

	BOOL bIsFirstRowChecked = VarBool(pdl->GetFirstRow()->GetValue(nCol));

	for(IRowSettingsPtr pRow = pdl->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		pRow->PutValue(nCol, bIsFirstRowChecked ? _variant_t(VARIANT_FALSE,VT_BOOL) : _variant_t(VARIANT_TRUE,VT_BOOL));
	}
}


void CNexFormsImportProcedureColumnsSheet::OnEditingFinishedNexformsContent(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		HandleNexFormsContentChange(pRow);

	}NxCatchAll("CNexFormsImportProcedureColumnsSheet::OnEditingFinishedNexformsContent");
}

void CNexFormsImportProcedureColumnsSheet::OnEditingFinishedProcedureContent(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		HandleProcedureContentChange(pRow);

	}NxCatchAll("CNexFormsImportProcedureColumnsSheet::OnEditingFinishedProcedureContent");
}

void CNexFormsImportProcedureColumnsSheet::HandleProcedureContentChange(IRowSettingsPtr pRow)
{
	NexFormsProcedure *procedure = (NexFormsProcedure*)VarLong(pRow->GetValue(pccPointer));

	// (z.manning, 06/28/2007) - Update memory for the given procedure based on current checkbox values.
	procedure->bUpdateName = VarBool(pRow->GetValue(pccUpdateName));
	procedure->bImportOfficialName = VarBool(pRow->GetValue(pccMedicalTerm));
	procedure->bImportCustom1 = VarBool(pRow->GetValue(pccCustom1));
	procedure->bImportCustom2 = VarBool(pRow->GetValue(pccCustom2));
	procedure->bImportCustom3 = VarBool(pRow->GetValue(pccCustom3));
	procedure->bImportCustom4 = VarBool(pRow->GetValue(pccCustom4));
	procedure->bImportCustom5 = VarBool(pRow->GetValue(pccCustom5));
	procedure->bImportCustom6 = VarBool(pRow->GetValue(pccCustom6));
	procedure->bImportAnesthesia = VarBool(pRow->GetValue(pccAnesthesia));
	procedure->bImportArrivalPrepMinutes = VarBool(pRow->GetValue(pccPrepTime));
}

void CNexFormsImportProcedureColumnsSheet::HandleNexFormsContentChange(IRowSettingsPtr pRow)
{
	NexFormsProcedure *procedure = (NexFormsProcedure*)VarLong(pRow->GetValue(nccPointer));

	// (z.manning, 06/28/2007) - Update memory for the given procedure based on current checkbox values.
	procedure->bImportMiniDescription = VarBool(pRow->GetValue(nccMiniDesc));
	procedure->bImportPreOp = VarBool(pRow->GetValue(nccPreOp));
	procedure->bImportTheDayOf = VarBool(pRow->GetValue(nccTheDayOf));
	procedure->bImportPostOp = VarBool(pRow->GetValue(nccPostOp));
	procedure->bImportRecovery = VarBool(pRow->GetValue(nccRecovery));
	procedure->bImportProcDetails = VarBool(pRow->GetValue(nccProcDetails));
	procedure->bImportRisks = VarBool(pRow->GetValue(nccRisks));
	procedure->bImportAlternatives = VarBool(pRow->GetValue(nccAlternatives));
	procedure->bImportComplications = VarBool(pRow->GetValue(nccComplications));
	procedure->bImportSpecialDiet = VarBool(pRow->GetValue(nccSpecialDiet));
	procedure->bImportShowering = VarBool(pRow->GetValue(nccShowering));
	procedure->bImportBandages = VarBool(pRow->GetValue(nccBandages));
	procedure->bImportConsent = VarBool(pRow->GetValue(nccConsent));
	procedure->bImportAltConsent = VarBool(pRow->GetValue(nccAltConsent));
	procedure->bImportHospitalStay = VarBool(pRow->GetValue(nccHospitalStay));
}

void CNexFormsImportProcedureColumnsSheet::OnSelectEverything() 
{
	try
	{
		SelectAll(TRUE);

	}NxCatchAll("CNexFormsImportProcedureColumnsSheet::OnSelectEverything");
}

void CNexFormsImportProcedureColumnsSheet::OnSelectNothing() 
{
	try
	{
		SelectAll(FALSE);

	}NxCatchAll("CNexFormsImportProcedureColumnsSheet::OnSelectNothing");
}

void CNexFormsImportProcedureColumnsSheet::SelectAll(BOOL bSelect)
{
	// (z.manning, 07/02/2007) - Go through every row and every column and set all checkboxes to
	// the given value.
	ASSERT(m_pdlProcedureContent->GetRowCount() == m_pdlNexFormsContent->GetRowCount());
	IRowSettingsPtr pProcedureRow = m_pdlProcedureContent->GetFirstRow();
	IRowSettingsPtr pNexFormsRow = m_pdlNexFormsContent->GetFirstRow();
	while(pProcedureRow != NULL && pNexFormsRow != NULL)
	{
		for(int i = 0; i < m_pdlProcedureContent->GetColumnCount(); i++)
		{
			// (z.manning, 07/02/2007) - If it's a checkbox, set it to to the specified value.
			if(m_pdlProcedureContent->GetColumn(i)->GetFieldType() == cftBoolCheckbox)
			{
				pProcedureRow->PutValue(i, bSelect ? _variant_t(VARIANT_TRUE,VT_BOOL) : _variant_t(VARIANT_FALSE,VT_BOOL));
			}
		}
		HandleProcedureContentChange(pProcedureRow);

		for(i = 0; i < m_pdlNexFormsContent->GetColumnCount(); i++)
		{
			// (z.manning, 07/02/2007) - If it's a checkbox, set it to to the specified value.
			if(m_pdlNexFormsContent->GetColumn(i)->GetFieldType() == cftBoolCheckbox)
			{
				pNexFormsRow->PutValue(i, bSelect ? _variant_t(VARIANT_TRUE,VT_BOOL) : _variant_t(VARIANT_FALSE,VT_BOOL));
			}
		}
		HandleNexFormsContentChange(pNexFormsRow);

		pProcedureRow = pProcedureRow->GetNextRow();
		pNexFormsRow = pNexFormsRow->GetNextRow();
	}
}

BOOL CNexFormsImportProcedureColumnsSheet::Validate()
{
	//TES 7/25/2007 - PLID 26685 - At this point, we need to check whether or not we should update their
	// pre-existing content fields to use the Tahoma typeface.  If we've ever done so before, we don't need
	// to again.
	// (z.manning, 10/11/2007) - PLID 27719 - If this is an existing NexForms user, prompt them to see if they
	// want to update the font of the NexForms content for any existing procedures we'll be updating.
	if(m_pdlgMaster->m_ImportInfo.m_eImportType == nfitExisting) 
	{
		// (z.manning, 10/11/2007) - PLID 27719 - Go through and get all of the procedure IDs for any existing
		// procedures we'll be updating.
		CArray<long,long> arynExistingProcedureIDs;
		for(int i = 0; i < m_pdlgMaster->m_ImportInfo.m_arypProcedures.GetSize(); i++) {
			NexFormsProcedure *procedure = m_pdlgMaster->m_ImportInfo.m_arypProcedures.GetAt(i);
			if(procedure->bImport && procedure->nExistingProcedureID > 0) {
				arynExistingProcedureIDs.Add(procedure->nExistingProcedureID);
			}
		}

		if(arynExistingProcedureIDs.GetSize() > 0)
		{
			// (z.manning, 10/11/2007) - PLID 27719 - As long as they are updating at least one existing procedure
			// then prompt them to see if they want to update the font for every existing procedure that we're importing.
			if(ReturnsRecords("SELECT TOP 1 ID FROM ProcedureT WHERE ProcedureT.ID IN (%s) AND COALESCE(NexFormsFontVersion, 0) < %i", ArrayAsString(arynExistingProcedureIDs,false), NEXFORMS_FONT_VERSION))
			{
				CString strMessage = "The NexForms content you are importing is designed to work best with content that uses the 'Tahoma' font.  "
					"It is highly recommended that you update any pre-existing content to use that font.\r\n\r\n"
					"Would you like to update the font for all of the procedures that match a procedure that you are importing?\r\n\r\n"
					"If you say 'Yes', all your existing NexForms content fields for procedures that you are importing will be modified to use the Tahoma font.  Only the formatting of the text will be affected, the text itself will be unchanged.\r\n\r\n"
					"If you say 'No', your existing content will not be modified, which may cause formatting issues if that content is merged to one of the Word templates being imported.";
				if(IDYES == MessageBox(strMessage, "Update Font?", MB_YESNO)) {
					m_pdlgMaster->m_ImportInfo.m_bUpdateExistingProceduresFont = TRUE;
				}
				else {
					m_pdlgMaster->m_ImportInfo.m_bUpdateExistingProceduresFont = FALSE;
				}
			}
		}
	}

	return TRUE;
}