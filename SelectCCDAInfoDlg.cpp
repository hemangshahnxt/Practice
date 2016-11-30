// SelectClinicalSummaryInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SelectCCDAInfoDlg.h"
#include "EMRRc.h"
#include "NxAPIUtils.h"
#include "CCDAUtils.h"

// (a.walling 2014-05-09 10:20) - PLID 61788 - CSelectCCDAInfoDlg formerly CSelectClinicalSummaryInfoDlg

// (j.gruber 2013-12-09 10:01) - PLID 59420 - created for
// CSelectCCDAInfoDlg dialog

enum SelectListColumn
{
	slcID = 0,
	slcExport,
	slcDescription,
};

IMPLEMENT_DYNAMIC(CSelectCCDAInfoDlg, CNxDialog)

// (r.gonet 04/22/2014) - PLID 61805 - Added the PICID to the constructor since the Clinical Summary needs to be assoaciated with the PIC.
CSelectCCDAInfoDlg::CSelectCCDAInfoDlg(CCDAType ccdaType, long nPICID, long nEMNID, long nPersonID, CWnd* pParent)
	: CNxDialog(CSelectCCDAInfoDlg::IDD, pParent)
	, m_nPICID(nPICID)
	, m_nEMNID(nEMNID)
	, m_nPersonID(nPersonID)
	, m_ccdaType(ccdaType)
{
	m_pOptions.CreateInstance(__uuidof(NexTech_Accessor::CCDAOptions));

	// (a.walling 2014-05-09 10:20) - PLID 61788 - Get the UserDefinedID up front
	if (-1 != m_nPersonID) {
		m_nUserDefinedID = GetExistingPatientUserDefinedID(m_nPersonID);
	}
}

CSelectCCDAInfoDlg::~CSelectCCDAInfoDlg()
{
}

void CSelectCCDAInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSelectCCDAInfoDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CSelectCCDAInfoDlg::OnBnClickedOk)	
END_MESSAGE_MAP()


bool CSelectCCDAInfoDlg::IsSummaryOfCare() const
{
	return ctSummaryOfCare == m_ccdaType;
}

bool CSelectCCDAInfoDlg::IsClinicalSummary() const
{
	return ctClinicalSummary == m_ccdaType;
}

// CSelectCCDAInfoDlg message handlers
BOOL CSelectCCDAInfoDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		// (a.walling 2014-05-09 10:20) - PLID 61788 - for now, must be one or the other.
		_ASSERTE((IsSummaryOfCare() || IsClinicalSummary()) && !(IsSummaryOfCare() && IsClinicalSummary()));

		if (IsSummaryOfCare()) {
			SetWindowText("Select Summary of Care Information");
		}

		m_pSelectList = BindNxDataList2Ctrl(IDC_CCDA_SELECTION_LIST, false);

		SetDlgItemText(IDC_ST_INSTRUCTION, "Please uncheck any data you would not like shown and then choose the format you would like to generate.");

		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.gruber 2014-05-06 13:44) - PLID 91925 - make the XML checkbox enabled by default
		CheckDlgButton(IDC_GENERATE_XML, TRUE);

		LoadList();

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

// (a.walling 2014-05-09 10:20) - PLID 61788 - Generate an appropriate patient filter
IUnknownPtr CSelectCCDAInfoDlg::MakePatientFilter()
{
	NexTech_Accessor::_CCDAPatientFilterPtr patFilter;
	patFilter.CreateInstance(__uuidof(NexTech_Accessor::CCDAPatientFilter));

	Nx::SafeArray<BSTR> aryEmnIDs;
	if (IsSummaryOfCare() || -1 == m_nEMNID) {
		//for now, we are going to use every emn for this person, in the future maybe we'll let them pick
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT EMRMasterT.ID "
			" FROM EMRMasterT "
			" WHERE DELETED = 0 AND PatientID = {INT}", m_nPersonID);
		while (!prs->eof)
		{
			long nEMNID = AdoFldLong(prs->Fields, "ID");
			aryEmnIDs.Add(_bstr_t(AsString(nEMNID)));

			prs->MoveNext();
		}

		ASSERT(m_nUserDefinedID != -1 && m_nPersonID != -1);
	}
	else {
		aryEmnIDs.Add(_bstr_t(AsString(m_nEMNID)));
	}

	if (aryEmnIDs) {
		patFilter->EmnIDs = aryEmnIDs;
	}
	
	patFilter->patientID = (LPCTSTR)AsString(m_nUserDefinedID);

	return patFilter;
}

void CSelectCCDAInfoDlg::LoadList()
{

	//the first thing we need to do is get the content

	//get out outputOptions
	NexTech_Accessor::_CCDAOutputOptionsPtr outputOptions(__uuidof(NexTech_Accessor::CCDAOutputOptions));
	outputOptions->IncludeXMLFile = false;
	outputOptions->IncludePDFFile = false;
	outputOptions->IncludeContentSections = true;


	CString strEMNID = AsString(m_nEMNID);
	CString strUserDefinedID = AsString(m_nUserDefinedID);

	NexTech_Accessor::_CCDAGenerationResultPtr ccdaResult;
	

	// (j.jones 2014-01-10 16:20) - PLID 60274 - GenerateClinicalSummary can return an API soap fault
	// when a message box (such as when the CCDA is not set up) is desired instead of an exception.
	// We need to look for the special error, and handle it. If it is not handled, then throw it
	// as a normal exception.
	try {
		// (a.walling 2014-05-09 10:20) - PLID 61788 - Filter is used only for the Summary of Care at this point
		if (IsSummaryOfCare()) {
			NexTech_Accessor::_CCDAPatientFilterPtr patFilter = MakePatientFilter();
			ccdaResult = GetAPI()->GenerateSummaryofCare(GetAPISubkey(), GetAPILoginToken(), patFilter, outputOptions);
		} else {
			ccdaResult = GetAPI()->GenerateClinicalSummary(GetAPISubkey(), GetAPILoginToken(), _bstr_t(strEMNID), _bstr_t(strUserDefinedID), outputOptions);
		}
	}catch (_com_error &e) {
		//this function will turn expected soap exceptions into messageboxes,
		//return TRUE if it did, FALSE if the error still needs handled
		if(!ProcessAPIComError_AsMessageBox(GetSafeHwnd(), "Practice", e)) {
			throw e;
		}

		return;
	}

	Nx::SafeArray<IUnknown *> sarySections(ccdaResult->SectionContent);

	if (sarySections.GetCount() > 0)
	{
		foreach(NexTech_Accessor::_CCDAIncludedSectionPtr pSection, sarySections)
		{
			CString strSectionName = (LPCTSTR)pSection->SectionName;
			CString strSectionLOINC = (LPCTSTR)pSection->SectionLOINC;

			SetSection(strSectionLOINC, true);

			//add our section to the list
			NXDATALIST2Lib::IRowSettingsPtr pSectionRow = m_pSelectList->GetNewRow();
			pSectionRow->PutValue(slcExport, g_cvarTrue);
			pSectionRow->PutValue(slcDescription, _variant_t(strSectionName));
			pSectionRow->PutValue(slcID, _variant_t(strSectionLOINC));
			pSectionRow->PutBackColor(RGB(179,208,250));
			m_pSelectList->AddRowAtEnd(pSectionRow, NULL);

			//now get the data
			Nx::SafeArray<IUnknown *> saryData(pSection->SectionData);
			foreach(NexTech_Accessor::_CCDAIncludedDataPtr pData, saryData)
			{
				CString strDataID = (LPCTSTR)pData->DataID;
				CString strDescription = (LPCTSTR)pData->DataDescription;

				NXDATALIST2Lib::IRowSettingsPtr pRowData = m_pSelectList->GetNewRow();
				pRowData->PutValue(slcExport, g_cvarTrue);
				pRowData->PutValue(slcDescription, _variant_t(strDescription));
				pRowData->PutValue(slcID, _variant_t(strDataID));
				m_pSelectList->AddRowAtEnd(pRowData, pSectionRow);
			}
		}
	}

	//loop through the list and expand the columns
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelectList->GetFirstRow();
	while(pRow)
	{
		pRow->Expanded = true;
		pRow = pRow->GetNextRow();
	}

}

void CSelectCCDAInfoDlg::SetSection(CString strLoinc, bool bIncluded)
{
	if (strLoinc == "42348-3")
	{
		m_pOptions->includeAdvanceDirectives = bIncluded;
	}
	if (strLoinc == "48765-2")
	{
		m_pOptions->includeAllergies = bIncluded;
	}

	if (strLoinc == "46240-8")
	{
		m_pOptions->includeEncounters= bIncluded;
	}
	if (strLoinc == "10157-6")
	{
		m_pOptions->includeFamilyHistory = bIncluded;
	}
	if (strLoinc == "47420-5")
	{
		m_pOptions->includeFunctionalStatus = bIncluded;
	}
	if (strLoinc == "11369-6")
	{
		m_pOptions->includeImmunizations = bIncluded;
	}
	if (strLoinc == "69730-0")
	{
		m_pOptions->includeInstructions = bIncluded;
	}
	if (strLoinc == "46264-8")
	{
		m_pOptions->includeMedicalEquipment = bIncluded;
	}

	if (strLoinc == "10160-0")
	{
		m_pOptions->includeMedications = bIncluded;
	}

	if (strLoinc == "29549-3")
	{
		m_pOptions->includeMedicationsAdministered = bIncluded;
	}

	if (strLoinc == "48768-6")
	{
		m_pOptions->includePayers = bIncluded;
	}

	if (strLoinc == "18776-5")
	{
		m_pOptions->includePlanOfCare = bIncluded;
	}

	if (strLoinc == "11450-4")
	{
		m_pOptions->includeProblems = bIncluded;
	}

	if (strLoinc == "47519-4")
	{
		m_pOptions->IncludeProcedures = bIncluded;
	}

	if (strLoinc == "42349-1")
	{
		m_pOptions->includeReasonForReferral = bIncluded;
	}

	if (strLoinc == "29299-5")
	{
		m_pOptions->includeReasonForVisit = bIncluded;
	}

	if (strLoinc == "30954-2")
	{
		m_pOptions->includeResults = bIncluded;
	}
	
	if (strLoinc == "29762-2")
	{
		m_pOptions->includeSocialHistory = bIncluded;
	}

	if (strLoinc == "8716-3")
	{
		m_pOptions->includeVitalSigns = bIncluded;
	}
}

void CSelectCCDAInfoDlg::GetSectionExclusions(CArray<NexTech_Accessor::_CCDAExclusionPtr, NexTech_Accessor::_CCDAExclusionPtr> &aryExclusions)
{
	
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelectList->GetFirstRow();
	while (pRow)
	{
		bool IsExcluded = !VarBool(pRow->GetValue(slcExport));
		CString strLoinc = VarString(pRow->GetValue(slcID));
		if (IsExcluded)
		{			
			//since we are in a section, lets set the option
			SetSection(strLoinc, false);

			//since we know the whole section is excluded, we don't have to look any further
		}
		else {

			SetSection(strLoinc, true);

			//see if any of the children are excluded
			NXDATALIST2Lib::IRowSettingsPtr pChildRow = pRow->GetFirstChildRow();			
			while (pChildRow)
			{
				bool bIsChildExcluded = !VarBool(pChildRow->GetValue(slcExport));
				if (bIsChildExcluded)
				{
					NexTech_Accessor::_CCDAExclusionPtr pExclusion(_uuidof(NexTech_Accessor::CCDAExclusion));
					pExclusion->DataID = _bstr_t(VarString(pChildRow->GetValue(slcID)));
					pExclusion->SectionLOINC = _bstr_t(strLoinc);
					aryExclusions.Add(pExclusion);
				}
				pChildRow = pChildRow->GetNextRow();
			
			}
		}
	
		pRow = pRow->GetNextRow();
	}


}

void CSelectCCDAInfoDlg::OnBnClickedOk()
{
	try {
		GenerateDocument();
	} NxCatchAll(__FUNCTION__);
}

void CSelectCCDAInfoDlg::GenerateDocument()
{
	try {
		//make sure they checked a box
		if (!IsDlgButtonChecked(IDC_GENERATE_XML) && !IsDlgButtonChecked(IDC_GENERATE_PDF))
		{
			MsgBox("Please select the format you would like to generate");
			return;
		}
		
		CWaitCursor cur;
				
		CArray<NexTech_Accessor::_CCDAExclusionPtr, NexTech_Accessor::_CCDAExclusionPtr> aryExclusions;
		
		GetSectionExclusions(aryExclusions);

		NexTech_Accessor::_CCDAExclusionPtr pExclusion(_uuidof(NexTech_Accessor::CCDAExclusion));

		//turn it into a safe array		
		Nx::SafeArray<IUnknown *> saryExclusions = Nx::SafeArray<IUnknown *>::From(aryExclusions);
		m_pOptions->exclusions = saryExclusions;

		//get our output options		
		NexTech_Accessor::_CCDAOutputOptionsPtr pOutput(__uuidof(NexTech_Accessor::CCDAOutputOptions));

		if (IsDlgButtonChecked(IDC_GENERATE_XML))
		{
			pOutput->IncludeXMLFile = true;
		}

		if (IsDlgButtonChecked(IDC_GENERATE_PDF))
		{
			pOutput->IncludePDFFile = true;
		}
		
		m_pOptions->outputOptions = pOutput;

		// (a.walling 2014-05-09 10:20) - PLID 61788 - Get appropriate document name and set validation standard
		if (IsClinicalSummary()) {
			m_pOptions->configDocumentName = "Clinical Summary";
			m_pOptions->ValidationStandard = NexTech_Accessor::CCDAValidationStandard_cvsClinicalSummary;
		}
		else  if (IsSummaryOfCare()) {
			m_pOptions->configDocumentName = "Ambulatory Summary of Care";
			m_pOptions->ValidationStandard = NexTech_Accessor::CCDAValidationStandard_cvsSummaryofCare;
		}
		else {
			ASSERT(FALSE);
		}

		NexTech_Accessor::_CCDAPatientFilterPtr patFilter = MakePatientFilter();
		m_pOptions->patientFilter = patFilter;
		
		//now generate again
		NexTech_Accessor::_CCDAGenerationResultPtr ccdaResult;

		// (j.jones 2014-01-10 16:20) - PLID 60274 - GenerateCCDA can return an API soap fault
		// when a message box (such as when the CCDA is not set up) is desired instead of an exception.
		// We need to look for the special error, and handle it. If it is not handled, then throw it
		// as a normal exception.
		try {
			ccdaResult = GetAPI()->GenerateCCDA(GetAPISubkey(), GetAPILoginToken(), m_pOptions);
		} catch (_com_error &e) {
			//this function will turn expected soap exceptions into messageboxes,
			//return TRUE if it did, FALSE if the error still needs handled
			if(!ProcessAPIComError_AsMessageBox(GetSafeHwnd(), "Practice", e)) {
				throw e;
			}

			return;
		}

		CString strDesc;
		if (IsClinicalSummary()) {
			strDesc = "Generated Clinical Summary";
		}
		else if (IsSummaryOfCare()) {
			strDesc = "Generated Summary of Care";
		}
		else {
			ASSERT(FALSE);
			strDesc = "Generated CCDA Document";
		}

		if (IsDlgButtonChecked(IDC_GENERATE_XML))
		{
			// (d.singleton 2013-11-15 11:03) - PLID 59513 - need to insert the CCDAType when generating a CCDA
			// (b.savon 2014-04-30 10:32) - PLID 61792 - User Story 10 - Auxiliary CCDA Items - Requirement 100150 - Pass the EMNID and PatientID
			// (a.walling 2014-05-13 14:43) - PLID 61788 - Attach to history with appropriate description
			// to be consistent with older behavior, we do not try to get more information from the clinical summary output for the description.
			// (r.gonet 04/22/2014) - PLID 61805 - Pass along the PICID so it gets associated with the MailSent record.
			CCDAUtils::AttachToHistory(ccdaResult->XMLFile, strDesc, "xml", "", SELECTION_CCDA, m_ccdaType, m_nPICID, m_nEMNID, m_nPersonID, IsClinicalSummary() ? false : true);
		}

		if (IsDlgButtonChecked(IDC_GENERATE_PDF))
		{
			// (d.singleton 2013-11-15 11:03) - PLID 59513 - need to insert the CCDAType when generating a CCDA
			// (j.gruber 2013-11-21 16:08) - PLID 59513 - this should be none, since its just a PDF
			// (b.savon 2014-04-30 10:32) - PLID 61792 - User Story 10 - Auxiliary CCDA Items - Requirement 100150 - Pass the EMNID and PatientID
			// (a.walling 2014-05-13 14:43) - PLID 61788 - Attach to history with appropriate description
			// Note this description differs from the previous since it needs the XML to determine the description.
			// (r.gonet 04/22/2014) - PLID 61805 - Pass along the PICID so it gets associated with the MailSent record.
#pragma TODO("PLID 62346 - Do we need the XML always so we have a consistent description for the PDF?")
			long nMailID = CCDAUtils::AttachToHistory(ccdaResult->PDFFile, strDesc, "pdf", "- Human Readable", SELECTION_FILE, ctNone, m_nPICID, m_nEMNID, m_nPersonID, false);

			// (a.walling 2015-10-28 13:16) - PLID 67424 - Attempt to copy the CCDA to export paths if necessary
			GetAPI()->CopyToCCDAExportPaths(GetAPISubkey(), GetAPILoginToken(), (const char*)FormatString("%li", nMailID));
		}

		MessageBox(strDesc + " successfully exported to patient history.", NULL, MB_OK | MB_ICONINFORMATION);

		OnOK();

	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CSelectCCDAInfoDlg, CNxDialog)
	ON_EVENT(CSelectCCDAInfoDlg, IDC_CCDA_SELECTION_LIST, 10, CSelectCCDAInfoDlg::EditingFinishedCcdaSelectionList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

void CSelectCCDAInfoDlg::ChangeAllChildren(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bChecked)
{
	if (pRow)
	{
		NXDATALIST2Lib::IRowSettingsPtr pChildRow = pRow->GetFirstChildRow();
		while (pChildRow)
		{
			if (bChecked) 
			{
				pChildRow->PutValue(slcExport, g_cvarTrue);
			}
			else {
				pChildRow->PutValue(slcExport, g_cvarFalse);
			}
			pChildRow = pChildRow->GetNextRow();
		}
	}

}


BOOL CSelectCCDAInfoDlg::AllChildrenUnchecked(NXDATALIST2Lib::IRowSettingsPtr pParentRow)
{
	if (pParentRow)
	{
		NXDATALIST2Lib::IRowSettingsPtr pChildRow = pParentRow->GetFirstChildRow();
		while (pChildRow)
		{
			if (VarBool(pChildRow->GetValue(slcExport)))
			{
				return FALSE;
			}
			pChildRow = pChildRow->GetNextRow();
		}
		return TRUE;
	}
	return FALSE;
}

void CSelectCCDAInfoDlg::EditingFinishedCcdaSelectionList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{

		if (bCommit){
			if (nCol == slcExport)
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				if (pRow)
				{
					BOOL bChecked = VarBool(varNewValue);
					ChangeAllChildren(pRow, bChecked);
					
					//check if this is a child row
					NXDATALIST2Lib::IRowSettingsPtr pParentRow = pRow->GetParentRow();
					if (VarBool(varNewValue) == FALSE)
					{
						if (pParentRow)
						{
							if (AllChildrenUnchecked(pParentRow))
							{
								MsgBox("All the elements of this section have been unchecked, the whole section will therefore be unchecked");
								pParentRow->PutValue(slcExport, varNewValue);
							}
						}
					}
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}
