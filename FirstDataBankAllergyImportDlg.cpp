// FirstDataBankAllergyImportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "FirstDataBankAllergyImportDlg.h"
#include "FirstDataBankUtils.h"
#include "NxAPI.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include <foreach.h>
#include "PatientsRc.h"

// (b.savon 2012-07-24 11:42) - PLID 51734 - Created

// CFirstDataBankAllergyImportDlg dialog

enum SearchListColumns {
	slcConceptID = 0,
	slcConceptTypeID,
	slcImport,
	slcAllergy,
	slcAllergyStatus,
};

IMPLEMENT_DYNAMIC(CFirstDataBankAllergyImportDlg, CNxDialog)

CFirstDataBankAllergyImportDlg::CFirstDataBankAllergyImportDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFirstDataBankAllergyImportDlg::IDD, pParent)
{
	m_bShouldRequery = FALSE;
}

CFirstDataBankAllergyImportDlg::~CFirstDataBankAllergyImportDlg()
{
}

void CFirstDataBankAllergyImportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDIMPORT, m_btnImport);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_NXC_BACK, m_nxcBack);
	DDX_Control(pDX, ID_FDB_ALLERGY_SEARCH, m_btnSearch);
}


BEGIN_MESSAGE_MAP(CFirstDataBankAllergyImportDlg, CNxDialog)
	ON_BN_CLICKED(IDIMPORT, &CFirstDataBankAllergyImportDlg::OnBnClickedImport)
	ON_BN_CLICKED(IDCANCEL, &CFirstDataBankAllergyImportDlg::OnBnClickedCancel)
	ON_BN_CLICKED(ID_FDB_ALLERGY_SEARCH, &CFirstDataBankAllergyImportDlg::OnBnClickedFdbAllergySearch)
END_MESSAGE_MAP()


// CFirstDataBankAllergyImportDlg message handlers

BOOL CFirstDataBankAllergyImportDlg::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog();

		//make sure we are connected to FDB
		if (!FirstDataBank::EnsureDatabase(this, true)) {
			OnCancel();
			return TRUE;
		}

		m_btnImport.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnSearch.AutoSet(NXB_INSPECT);

		m_nxcBack.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		m_pSearchList = BindNxDataList2Ctrl(IDC_NXDL_ALLERGY_LIST, false);

		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pSearchList->GetColumn(slcImport);
		if (pCol) {
			pCol->PutColumnStyle(NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csEditable);
		}


	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CFirstDataBankAllergyImportDlg::LoadFirstDataBankAllergyMasterList()
{
	CString strSearch;

	GetDlgItemText(IDC_EDT_SEARCH, strSearch);

	CWaitCursor cwait;

	if( strSearch.GetLength() < 3 ){
		MsgBox("Please enter at least 3 characters of the allergy and try again.");
		return;
	}

	m_pSearchList->Clear();

	// (j.jones 2016-01-20 15:15) - PLID 68011 - filter out non-FDB drugs
	NexTech_Accessor::_NullableBoolPtr pIncludeFDBAllergiesOnly(__uuidof(NexTech_Accessor::NullableBool));
	pIncludeFDBAllergiesOnly->SetBool(VARIANT_TRUE);
	
	// Call the API to do the Allergy Search.  We must covert the SAFEARRAY to a CArray before we can do anything useful with it.
	NexTech_Accessor::_FDBAllergyArrayPtr searchResults = GetAPI()->AllergySearch(GetAPISubkey(), GetAPILoginToken(), _bstr_t(strSearch), pIncludeFDBAllergiesOnly);

	//	If our search produced no results, tell the user and bail
	if (searchResults->FDBAllergies == NULL){
		MsgBox("No allergies that start with '%s' could be found.  Please check the spelling and try your search again.", strSearch);
		return;
	}	

	Nx::SafeArray<IUnknown *> saryAllergies(searchResults->FDBAllergies);

	if( saryAllergies.GetCount() == 0 ){
		MsgBox("No allergies that start with '%s' could be found.  Please check the spelling and try your search again.", strSearch);
		return;
	}

	// Run through our array and add the allergies found to the list
	foreach(NexTech_Accessor::_FDBAllergyPtr allergy, saryAllergies){
		// (b.savon 2013-01-29 18:14) - PLID 54920
		if( allergy->ConceptID != -1 && allergy->ConceptTypeID != -1 ){ // Only do if this is an FDB allergy
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSearchList->GetNewRow();
			pRow->PutValue(slcConceptID, allergy->ConceptID);
			pRow->PutValue(slcConceptTypeID, allergy->ConceptTypeID);
			pRow->PutValue(slcImport, g_cvarFalse);
			pRow->PutValue(slcAllergy, allergy->Name);
			pRow->PutValue(slcAllergyStatus, allergy->AlreadyInData == VARIANT_TRUE ? _bstr_t("Already Imported") : _bstr_t("Available To Import"));

			if( allergy->AlreadyInData == VARIANT_TRUE ){
				// Let's visually let the user know these have been imported already.  We'll piggy back the 'discontinue' color to do this.
				pRow->PutBackColor(RGB(222, 225, 231));
			}

			m_pSearchList->AddRowAtEnd(pRow, NULL);
		}
		else {
			// (j.jones 2016-01-20 16:19) - PLID 68011 - all returned allergies ought to be FDB only now,
			// if not, something in the API has failed
			ASSERT(FALSE);
		}
	}
}

void CFirstDataBankAllergyImportDlg::OnBnClickedImport()
{
	try{

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSearchList->GetFirstRow();
			
		CWaitCursor cwait;

		//	Go through our list and pick off the ones the user has selected to import.
		CArray<NexTech_Accessor::_FDBAllergyImportInputPtr, NexTech_Accessor::_FDBAllergyImportInputPtr> aryAllergies;
		while (pRow) {

			if (VarBool(pRow->GetValue(slcImport), FALSE)) {
				NexTech_Accessor::_FDBAllergyImportInputPtr allergy(__uuidof(NexTech_Accessor::FDBAllergyImportInput));
				
				allergy->AllergyName = _bstr_t(pRow->GetValue(slcAllergy));
				allergy->ConceptID = VarLong(pRow->GetValue(slcConceptID));
				allergy->ConceptTypeID = VarLong(pRow->GetValue(slcConceptTypeID));
				aryAllergies.Add(allergy);
			}
			pRow = pRow->GetNextRow();
		}

		// If we have nothing to selected to import, tell the user and bail
		if( aryAllergies.GetCount() <= 0 ){
			MsgBox("There were no allergies chosen to be imported.  Please select allergies to import.");
			return;
		}

		//	Create our SAFEARRAY to be passed to the AllergyImport function in the API
		Nx::SafeArray<IUnknown *> saryAllergies = Nx::SafeArray<IUnknown *>::From(aryAllergies);

		//	Call the API to import the allergies and then convert the handed back SAFEARRAY to a CArray so we can do something useful with it.
		NexTech_Accessor::_FDBAllergyImportResultsArrayPtr importResults = GetAPI()->AllergyImport(GetAPISubkey(), GetAPILoginToken(), saryAllergies);

		//	If for some reason we get nothing back (although, this should never happen), tell the user and bail.
		if( importResults->FDBAllergyImportResults == NULL ){
			MsgBox("There were no allergies to import.");
			return;
		}

		Nx::SafeArray<IUnknown *> saryAllergyResults(importResults->FDBAllergyImportResults);

		//	Prepare our results message so the user knows which imported succesfully (or not)
		CString strSuccess = "The following allergies have imported successfully:\r\n";
		CString strFailure = "\r\nThe following allergies are already imported:\r\n";
		foreach(NexTech_Accessor::_FDBAllergyImportOutputPtr pAllergyResult, saryAllergyResults)
		{
			if( pAllergyResult->Success == VARIANT_TRUE ){
				strSuccess += CString((LPCTSTR)pAllergyResult->AllergyName) + "\r\n";
				m_bShouldRequery = TRUE;
			}else{
				strFailure += CString((LPCTSTR)pAllergyResult->AllergyName) + "\r\n";
			}
		}

		CString strResults;
		if( strSuccess != "The following allergies have imported successfully:\r\n" ){
			strResults += strSuccess;
		}

		if( strFailure != "\r\nThe following allergies are already imported:\r\n" ){
			strResults += strFailure;
		}

		if( !strResults.IsEmpty() ){
			MsgBox("%s", strResults);
		}

	}NxCatchAll(__FUNCTION__);
	
	CNxDialog::OnOK();
}

void CFirstDataBankAllergyImportDlg::OnOK()
{
	try{

	}NxCatchAll(__FUNCTION__);
}

void CFirstDataBankAllergyImportDlg::OnBnClickedCancel()
{
	try{

		CNxDialog::OnCancel();

	}NxCatchAll(__FUNCTION__);
}

void CFirstDataBankAllergyImportDlg::OnBnClickedFdbAllergySearch()
{
	try{
		
		LoadFirstDataBankAllergyMasterList();

	}NxCatchAll(__FUNCTION__);
}